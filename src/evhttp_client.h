#ifndef __EVHTTP_CLIENT_H__
#define __EVHTTP_CLIENT_H__

#include "event2/event.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef void* evh_client;

enum set_opt
{
	EVHTTP_CLIENT_DEBUG = 0,	/* set log callback function 'evh_client_log_cb' */
	EVHTTP_CLIENT_MAX_CACHE,	/* set max cache connect, default 3, 'int' */
};

enum url_status
{
	URL_STATUS_ERROR = -1,
	URL_STATUS_FAIL,
	URL_STATUS_FINISH,
	URL_STATUS_DATA,
	URL_STATUS_PROGRESS,
};

typedef struct _url_cb_finish{
	int result;
}url_cb_finish;

typedef struct _url_cb_data{
	void*	ptr;
	size_t 	size; 
	size_t 	nmemb;
}url_cb_data;

typedef struct _url_cb_prog{
	double	dtotal;
	double	dnow;
	double	utotal;
	double	unow;
}url_cb_prog;


typedef void (*evh_client_log_cb)(int severity, const char* str);
typedef void (*evh_client_url_cb)(enum url_status status, 
				const void* result, void* usrdata);


/* create a http client */
evh_client evh_client_init(struct event_base *evbase);

/* release a http client */
int evh_client_release(evh_client client);


int evh_client_setopt(evh_client client, enum set_opt opt, void* data);


int evh_client_post(evh_client client, const char* url, 
			const char* post, evh_client_url_cb cb, void* data);


int evh_client_get(evh_client client, const char* url, 
			evh_client_url_cb cb, void* data);


#ifdef __cplusplus
}
#endif


#endif // __EVHTTP_CLIENT_H__

