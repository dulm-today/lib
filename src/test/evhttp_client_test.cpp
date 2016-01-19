#include "evhttp_client.h"
#include "event2/event.h"
#include "event2/thread.h"
#include <Windows.h>

#pragma comment(lib,"ws2_32.lib")

void log_cb(int level, const char* module, const char* msg)
{
	char buffer[1024];
	const char* strlevel[] = {"DEBUG", "MSG", "WARN", "ERROR"};

	sprintf(buffer, "(%-6s) %-16s  %s\n", strlevel[level], module, msg);

	OutputDebugString(buffer);
}

void evhttp_log_cb(int level, const char* msg)
{
	log_cb(level, "evhttp_client", msg);
}

void libevent_log_cb(int level, const char* msg)
{
	log_cb(level, "libevent", msg);
}

void test_log_cb(int level, const char* msg)
{
	
}

void url_cb(enum url_status status, 
				const void* result, void* usrdata)
{
	char buffer[1024];

	switch(status)
	{
	
	case URL_STATUS_FINISH:
		{
			const url_cb_finish* fin = (const url_cb_finish*)result;
			sprintf(buffer, "FINISH result[%d]", fin->result);
			log_cb(EVENT_LOG_DEBUG, "url_cb", buffer);
		}
		break;
	case URL_STATUS_DATA:
		{
			const url_cb_data* da = (const url_cb_data*)result;
			sprintf(buffer, "DATA [%d*%d=%d][%.*s]", da->size, da->nmemb, da->size*da->nmemb,
				512, (const char*)da->ptr);
			log_cb(EVENT_LOG_DEBUG, "url_cb", buffer);
		}
		break;
	case URL_STATUS_PROGRESS:
		{
			const url_cb_prog* prog = (const url_cb_prog*)result;
			sprintf(buffer, "Progress: download[%f/%f] upload[%f/%f]", 
				prog->dnow, prog->dtotal, prog->unow, prog->utotal);
			log_cb(EVENT_LOG_DEBUG, "url_cb", buffer);
		}
		break;
	case URL_STATUS_ERROR:
	case URL_STATUS_FAIL:
	default:
		sprintf(buffer, "status[%d]", status);
		log_cb(EVENT_LOG_DEBUG, "url_cb", buffer);
		break;

	}
}

struct event_base* ev_base = NULL;
evh_client evhttp = NULL;	
struct event* ev_timer = NULL;

static void timer_cb(int fd, short kind, void *userp)
{ 
	struct timeval time;
	time.tv_sec = 0;
	time.tv_usec = 10*1000;
	evtimer_add(ev_timer, &time);

	evh_client_get(evhttp, "www.baidu.com", url_cb, NULL);
}

int main(int argc, const char** argv)
{
#ifdef WIN32
	WSADATA WSAData;
	WSAStartup(0x101, &WSAData);
#endif

	evthread_use_windows_threads();
	//event_enable_debug_mode();
	//event_set_log_callback(libevent_log_cb);

	ev_base = event_base_new();

#if 0	
	ev_timer = evtimer_new(ev_base, timer_cb, NULL);
	struct timeval time;
	time.tv_sec = 0;
	time.tv_usec = 10*1000;
	evtimer_add(ev_timer, &time);
#endif

	evhttp = evh_client_init(ev_base);

	evh_client_setopt(evhttp, EVHTTP_CLIENT_DEBUG, evhttp_log_cb);

	evh_client_get(evhttp, "www.baidu.com", url_cb, NULL);
	evh_client_get(evhttp, "www.163.com", url_cb, NULL);

	event_base_dispatch(ev_base);

	evh_client_release(evhttp);

	return 0;
}
