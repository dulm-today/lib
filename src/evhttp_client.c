#include <time.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <event2/event.h>
#include <event2/util.h>
#include <event2/thread.h>
#include <list.h>
#include <string.h>
#include "evhttp_client.h"

#define _EVHTTP_CLIENT_MAX_CACHE		3

enum conn_status
{
	CONN_CLOSE = -1,
	CONN_RUNING = 0,
	CONN_FINISH,
};

typedef struct _evhttp_client
{
	struct event_base 	*evbase;
	struct event 		*timer_event;
	CURLM 				*multi;
	void				*lock;
  	int 				still_running;
	int					free_num;
	int					max_free_num;
	list_head			conn_list;
	list_head			free_list;
}_evhttp_client;

typedef struct _conn_info
{
	curl_socket_t 	sockfd;
	CURL			*easy;
	
	struct event	*ev;
	struct _evhttp_client	*evh;
	
	void				*url_data;
	evh_client_url_cb	url_cb;

	int				status;
    char 			error[1024];
	list_node		conn_node;
}_conn_info;

evh_client_log_cb log_cb = NULL;

void *_evthreadimpl_lock_alloc(unsigned locktype);
void _evthreadimpl_lock_free(void *lock, unsigned locktype);
int _evthreadimpl_lock_lock(unsigned mode, void *lock);
int _evthreadimpl_lock_unlock(unsigned mode, void *lock);

static void evh_easy_destory(struct _conn_info* conn);
static void evh_easy_cleanup_nolock(struct _evhttp_client* evh);
static void evh_curl_check_complete(struct _evhttp_client *evh);

#define EV_LOCK_ALLOC(x)	((x) = _evthreadimpl_lock_alloc(EVTHREAD_LOCKTYPE_RECURSIVE))
#define EV_LOCK_FREE(x)		(_evthreadimpl_lock_free(x, EVTHREAD_LOCKTYPE_RECURSIVE))
#define EV_LOCK_LOCK(x)		(_evthreadimpl_lock_lock(0, x))
#define EV_LOCK_UNLOCK(x)	(_evthreadimpl_lock_unlock(0, x))

static void _LOG(int severity, const char * fmt, ...)
{
	char buf[1024];
	va_list ap;

	if (NULL == log_cb)
		return;
	
	va_start(ap, fmt);
	
	if (fmt != NULL)
		evutil_vsnprintf(buf, sizeof(buf), fmt, ap);
	else
		buf[0] = '\0';

	va_end(ap);

	if (log_cb)
		log_cb(severity, buf);
}

static void evh_libevent_call_curl(int fd, int action, struct _evhttp_client* evh)
{
	CURLMcode rc;

	EV_LOCK_LOCK(evh->lock);
	
	rc = curl_multi_socket_action(evh->multi, fd, action, &evh->still_running);
	if (CURLM_OK != rc){
		_LOG(EVENT_LOG_ERR, "%s: curl_multi_socket_action return %d", __FUNCTION__, rc);

		// TODO: error deal
	}

	evh_curl_check_complete(evh);
	
	if (evh->still_running <= 0) {
		_LOG(EVENT_LOG_DEBUG, "all transfer done, kill timeout");
		
		if (evtimer_pending(evh->timer_event, NULL)) {
			evtimer_del(evh->timer_event);
		}
	}

	EV_LOCK_UNLOCK(evh->lock);
}

/* Called by libevent when we get action on a multi socket */
static void evh_libevent_event_cb(int fd, short kind, void *userp)
{
	struct _evhttp_client *evh = (struct _evhttp_client*)userp;

	int action = (kind & EV_READ  ? CURL_CSELECT_IN  : 0) 
				|(kind & EV_WRITE ? CURL_CSELECT_OUT : 0);

	evh_libevent_call_curl(fd, action, evh);
}

/* Called by libevent when our timeout expires */
static void evh_libevent_timer_cb(int fd, short kind, void *userp)
{
  	struct _evhttp_client *evh = (struct _evhttp_client*)userp;
  
	(void)fd;
	(void)kind;

	evh_libevent_call_curl(CURL_SOCKET_TIMEOUT, 0, evh);
}

/* Clean up the SockInfo structure */
static void evh_sock_event_remove(CURL *e, struct _evhttp_client* evh, struct _conn_info *conn)
{
	if (NULL != conn->ev)
		event_free(conn->ev);
	
	conn->ev = NULL;
	conn->sockfd = -1;;
}


/* Assign information to a SockInfo structure */
static void evh_sock_event_reset(int act, struct _evhttp_client* evh, 
					struct _conn_info* conninfo)
{
	int kind = (act & CURL_POLL_IN  ? EV_READ : 0)
	         | (act & CURL_POLL_OUT ? EV_WRITE: 0)
	         | EV_PERSIST;

#if 1
	if (NULL != conninfo->ev)
		event_free(conninfo->ev);
	conninfo->ev = event_new(evh->evbase, conninfo->sockfd, kind, evh_libevent_event_cb, evh);
#else
	if (NULL == conninfo->ev)
		conninfo->ev = event_new(evh->evbase, conninfo->sockfd, kind, evh_libevent_event_cb, evh);
	else
		event_assign(conninfo->ev, evh->evbase, conninfo->sockfd, kind, evh_libevent_event_cb, evh);
#endif

	event_add(conninfo->ev, NULL);
}

static struct _conn_info* 
	evh_sock_bind(CURL* e, curl_socket_t s, struct _evhttp_client* evh)
{
	list_node* node = NULL;
	struct _conn_info* conninfo = NULL;
	
	while(NULL != (node = list_for_each(&evh->conn_list, node)))
	{
		struct _conn_info* conn = container_of(node, struct _conn_info, conn_node);
		if (conn->easy == e){
			conninfo = conn;
			break;
		}
	}

	if (NULL == conninfo){
		_LOG(EVENT_LOG_ERR, "%s connect not find CURL:%p socket:%d", 
				__FUNCTION__, e, s);
		return NULL;
	}

	conninfo->sockfd = s;
	curl_multi_assign(evh->multi, s, conninfo);

	_LOG(EVENT_LOG_DEBUG, "%s CURL[%p] socket[%d] bind", e, s);
	
	return conninfo;
}

/* CURLMOPT_SOCKETFUNCTION */
static int evh_curl_sock_cb(CURL *e, curl_socket_t s, int what, void *cbp, void *sockp)
{
	struct _evhttp_client *evh = (struct _evhttp_client*) cbp;
	struct _conn_info *conninfo = (struct _conn_info*) sockp;
	static const char* whatstr[] = {"NONE", "IN", "OUT", "INOUT", "REMOVE"};

	_LOG(EVENT_LOG_DEBUG, "sock_cb: socket %d %s, %p %p", s, whatstr[what], cbp, sockp);

	if (NULL == conninfo){
		conninfo = evh_sock_bind(e, s, evh);
		if (NULL == conninfo)
			return 0;
	}
	
	switch(what)
	{
		case CURL_POLL_NONE:
		case CURL_POLL_IN:
		case CURL_POLL_OUT:
		case CURL_POLL_INOUT:
			evh_sock_event_reset(what, evh, conninfo);
			break;
		case CURL_POLL_REMOVE:
			evh_sock_event_remove(e, evh, conninfo);
			break;
		default:
			break;
	}

	return 0;
}

/* CURLOPT_WRITEFUNCTION */
static size_t evh_curl_write_cb(void *ptr, size_t size, size_t nmemb, void *userp)
{
	struct _conn_info *conn = (struct _conn_info*) userp;
	size_t realsize = size * nmemb;
	url_cb_data cb_data;
	
	if (NULL == conn)
		return realsize;
	
	//_LOG(EVENT_LOG_DEBUG, "%s: %d * %d", __FUNCTION__, size, nmemb);

	if (conn->url_cb){
		cb_data.ptr = ptr;
		cb_data.size = size;
		cb_data.nmemb = nmemb;

		conn->url_cb(URL_STATUS_DATA, &cb_data, conn->url_data);
	}

	return realsize;
}


/* CURLOPT_PROGRESSFUNCTION */
static int evh_curl_prog_cb (void *p, double dltotal, double dlnow, double ult,
                    double uln)
{
	struct _conn_info *conn = (struct _conn_info *)p;
	url_cb_prog cb_data;

	if (NULL == conn)
		return 0;

	//_LOG(EVENT_LOG_DEBUG, "Progress: %d download(%f/%f) upload(%f/%f)", 
	//				conn->sockfd, dlnow, dltotal, uln, ult);

	if (conn->url_cb){
		cb_data.dtotal = dltotal;
		cb_data.dnow = dlnow;
		cb_data.utotal = ult;
		cb_data.unow = uln;

		conn->url_cb(URL_STATUS_PROGRESS, &cb_data, conn->url_data);
	}

	return 0;
}

/* Update the event timer after curl_multi library calls */
static int evh_curl_multi_timer_cb(CURLM *multi, long timeout_ms, struct _evhttp_client *evh)
{
	struct timeval timeout;
	(void)multi; /* unused */

	if (-1 == timeout_ms){
		evtimer_add(evh->timer_event, NULL);
	}
	else{
		timeout.tv_sec = timeout_ms/1000;
		timeout.tv_usec = (timeout_ms%1000)*1000;
		evtimer_add(evh->timer_event, &timeout);
	}

	_LOG(EVENT_LOG_DEBUG, "%s: set timer %ld ms", __FUNCTION__, timeout_ms);
	return 0;
}

/* Check for completed transfers, and remove their easy handles */
static void evh_curl_check_complete(struct _evhttp_client *evh)
{
	int msgs_left;
	char *eff_url;
	struct _conn_info *conn;
	CURLMsg *msg;
	CURL *easy;
	CURLcode res;

	_LOG(EVENT_LOG_DEBUG, "curl_still_runing: %d", evh->still_running);
	
	while (NULL != (msg = curl_multi_info_read(evh->multi, &msgs_left))) 
	{
		_LOG(EVENT_LOG_DEBUG, "conn[%p] msg[%d]", msg->easy_handle, msg->msg);
		
		if (msg->msg == CURLMSG_DONE) {
			easy = msg->easy_handle;
			res = msg->data.result;
			curl_easy_getinfo(easy, CURLINFO_PRIVATE, &conn);
			curl_easy_getinfo(easy, CURLINFO_EFFECTIVE_URL, &eff_url);
			
			_LOG(EVENT_LOG_DEBUG, "DONE: %s(%d) => (%d) %s", eff_url, conn->sockfd, res, conn->error);

			if (NULL != conn->url_cb)
			{
				url_cb_finish cb_data;
				cb_data.result = res;
				conn->url_cb(URL_STATUS_FINISH, &cb_data, conn->url_data);
			}
			
			evh_easy_destory(conn);
		}
	}
}

evh_client evh_client_init(struct event_base *evbase /*!= NULL*/)
{
	struct _evhttp_client* evh = NULL;
	
	if (NULL == evbase)
		return NULL;
	
	evh = (struct _evhttp_client*)malloc(sizeof(*evh));
	if (NULL == evh){
		_LOG(EVENT_LOG_ERR, "%s malloc fail", __FUNCTION__);
		return NULL;
	}
	
	memset(evh, 0, sizeof(*evh));

	EV_LOCK_ALLOC(evh->lock);
	evh->evbase = evbase;
	evh->max_free_num = _EVHTTP_CLIENT_MAX_CACHE;
	evh->multi = curl_multi_init();
	if (NULL == evh->multi){
		_LOG(EVENT_LOG_ERR, "%s curl_multi_init fail", __FUNCTION__);
		goto err;
	}

	evh->timer_event = evtimer_new(evh->evbase, evh_libevent_timer_cb, evh);
	if (NULL == evh->timer_event){
		_LOG(EVENT_LOG_ERR, "%s evtimer_new fail", __FUNCTION__);
		goto err;
	}

	/* setup the generic multi interface options we want */
	curl_multi_setopt(evh->multi, CURLMOPT_SOCKETFUNCTION, evh_curl_sock_cb);
	curl_multi_setopt(evh->multi, CURLMOPT_SOCKETDATA, evh);
	curl_multi_setopt(evh->multi, CURLMOPT_TIMERFUNCTION, evh_curl_multi_timer_cb);
	curl_multi_setopt(evh->multi, CURLMOPT_TIMERDATA, evh);

	return (evh_client)evh;

err:
	if (evh->timer_event)
		event_free(evh->timer_event);
	
	if (evh->multi)
		curl_multi_cleanup(evh->multi);

	if (evh->lock)
		EV_LOCK_FREE(evh->lock);

	free(evh);
	return NULL;
}

int evh_client_release(evh_client client)
{
	struct _evhttp_client* evh = (struct _evhttp_client*)client;
	list_node* node = NULL;

	if (NULL == evh)
		return -1;

	EV_LOCK_LOCK(evh->lock);

	if (evh->timer_event)
		event_free(evh->timer_event);

	evh->max_free_num = 0;
	
	while(NULL != (node = list_head_node(&evh->conn_list)))
	{
		struct _conn_info* conn = container_of(node, struct _conn_info, conn_node);

		evh_easy_destory(conn);
	}

	evh_easy_cleanup_nolock(evh);

	if (evh->multi)
		curl_multi_cleanup(evh->multi);

	EV_LOCK_FREE(evh->lock);
	
	free(evh);
	return 0;
}


int evh_client_setopt(evh_client client,enum set_opt opt, void* data)
{
	struct _evhttp_client* evh = (struct _evhttp_client*)client;

	if (NULL == evh)
		return -1;

	switch(opt)
	{
		case EVHTTP_CLIENT_DEBUG:
			log_cb = (evh_client_log_cb)data;
			break;
		case EVHTTP_CLIENT_MAX_CACHE:
			evh->max_free_num = *(int*)data;
			break;
		default:
			_LOG(EVENT_LOG_WARN, "%s opt unknow %d", __FUNCTION__, opt);
			return -1;
	}

	return 0;
}


static void evh_easy_cleanup_nolock(struct _evhttp_client* evh)
{
	list_node* node = NULL;

	while(evh->free_num > evh->max_free_num 
		&& NULL != (node = list_head_node(&evh->free_list)))
	{
		struct _conn_info* conn = container_of(node, struct _conn_info, conn_node);

		_LOG(EVENT_LOG_DEBUG, "close connect[%p]", conn);
		curl_easy_cleanup(conn->easy);
		list_del(&evh->free_list, node);
		evh->free_num--;
		free(conn);
	}
}

static void evh_easy_destory(struct _conn_info* conn)
{
	struct _evhttp_client *evh;
	
	if (NULL == conn)
		return;

	evh = conn->evh;

	EV_LOCK_LOCK(conn->evh->lock);
	list_del(&conn->evh->conn_list, &conn->conn_node);
	
	if (NULL != conn->ev)
		event_free(conn->ev);
	
	if (NULL != conn->easy){
		curl_multi_remove_handle(conn->evh->multi, conn->easy);
		curl_easy_reset(conn->easy);

		conn->ev = NULL;
		conn->sockfd = -1;
		conn->url_cb = NULL;
		conn->url_data = NULL;
		conn->status = 0;
		conn->error[0] = '\0';
	
		list_add_tail(&conn->evh->free_list, &conn->conn_node);
		conn->evh->free_num++;

		_LOG(EVENT_LOG_DEBUG, "move connect[%p] to cache", conn);
		
		evh_easy_cleanup_nolock(conn->evh);
	}
	else{
		free(conn);
	}
	
	EV_LOCK_UNLOCK(conn->evh->lock);
}


static struct _conn_info* evh_easy_create(struct _evhttp_client *evh)
{
	struct _conn_info *conn;
	list_node* node = NULL;

	EV_LOCK_LOCK(evh->lock);
	node = list_head_node(&evh->free_list);
	if (NULL != node){
		list_del(&evh->free_list, node);
		evh->free_num--;
	}
	EV_LOCK_UNLOCK(evh->lock);
	
	if (NULL == node)
	{
		conn = malloc(sizeof(*conn));
		if (NULL == conn){
			_LOG(EVENT_LOG_ERR, "%s malloc fail", __FUNCTION__ );
			return NULL;
		}
		memset(conn, 0, sizeof(*conn));

		conn->easy = curl_easy_init();
		if (NULL == conn->easy){
			_LOG(EVENT_LOG_ERR, "%s curl_easy_init fail", __FUNCTION__);
			goto err;
		}
	}
	else{
		conn = container_of(node, struct _conn_info, conn_node);
	}

	curl_easy_setopt(conn->easy, CURLOPT_WRITEFUNCTION, evh_curl_write_cb);
	curl_easy_setopt(conn->easy, CURLOPT_WRITEDATA, conn);
	curl_easy_setopt(conn->easy, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(conn->easy, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(conn->easy, CURLOPT_ERRORBUFFER, conn->error);
	curl_easy_setopt(conn->easy, CURLOPT_PRIVATE, conn);
	curl_easy_setopt(conn->easy, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(conn->easy, CURLOPT_PROGRESSFUNCTION, evh_curl_prog_cb);
	curl_easy_setopt(conn->easy, CURLOPT_PROGRESSDATA, conn);

	return conn;
	
err:
	evh_easy_destory(conn);
	
	return NULL;
}

int evh_client_post(evh_client client, const char* url, 
				const char* post, evh_client_url_cb cb, void* data)
{
	CURLMcode rc;
	struct _evhttp_client* evh = (struct _evhttp_client*)client;
	struct _conn_info *conn;

	if (NULL == evh)
		return -1;

	conn = evh_easy_create(evh);
	if (NULL == conn)
		return -1;
	
	conn->evh = evh;
	conn->url_cb = cb;
	conn->url_data = data;
	
	curl_easy_setopt(conn->easy, CURLOPT_URL, url);
	curl_easy_setopt(conn->easy, CURLOPT_POST, 1L);
	curl_easy_setopt(conn->easy, CURLOPT_POSTFIELDS, post);
	
	rc = curl_multi_add_handle(evh->multi, conn->easy);
	if (CURLM_OK != rc){
		_LOG(EVENT_LOG_ERR, "%s curl_multi_add_handle fail", __FUNCTION__);
		goto err;
	}

	EV_LOCK_LOCK(evh->lock);
	list_add_tail(&evh->conn_list, &conn->conn_node);
	EV_LOCK_UNLOCK(evh->lock);

	return 0;

err:
	evh_easy_destory(conn);
	
	return -1;
}


int evh_client_get(evh_client client, const char* url, 
				evh_client_url_cb cb, void* data)
{
	CURLMcode rc;
	struct _evhttp_client* evh = (struct _evhttp_client*)client;
	struct _conn_info *conn;

	if (NULL == evh)
		return -1;

	conn = evh_easy_create(evh);
	if (NULL == conn)
		return -1;

	conn->evh = evh;
	conn->url_cb = cb;
	conn->url_data = data;
	
	curl_easy_setopt(conn->easy, CURLOPT_URL, url);

	rc = curl_multi_add_handle(evh->multi, conn->easy);
	if (CURLM_OK != rc){
		_LOG(EVENT_LOG_ERR, "%s curl_multi_add_handle fail", __FUNCTION__);
		goto err;
	}

	EV_LOCK_LOCK(evh->lock);
	list_add_tail(&evh->conn_list, &conn->conn_node);
	EV_LOCK_UNLOCK(evh->lock);

	return 0;

err:
	evh_easy_destory(conn);

	return -1;
}

