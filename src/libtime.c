#include <stddef.h>
#include "libtime.h"

#ifdef _WIN32
#include <windows.h>
#elif defined(linux)
#include <time.h>
#include <sys/time.h>
#endif

static TimeVal _time_start = {0};

#ifdef _WIN32

static LARGE_INTEGER	_frequency = {0};
static LARGE_INTEGER    _start_counter = {0};

void time_base(TimeVal* time)
{
	LARGE_INTEGER	fc;
	QueryPerformanceCounter(&fc);

	fc.QuadPart -= _start_counter.QuadPart;
	time->tv_sec = (long int)(fc.QuadPart / _frequency.QuadPart);
	time->tv_usec = (long int)((fc.QuadPart * 1000*1000) % _frequency.QuadPart);
}

void time_init()
{
	QueryPerformanceFrequency(&_frequency);
	QueryPerformanceCounter(&_start_counter);
	time_base(&_time_start);
}

void time_mono_get(TimeVal* time)
{
	time_base(time);
}

void time_date_get(struct time_date* t)
{
	SYSTEMTIME time;
	GetLocalTime(&time);

	t->_year 	= time.wYear;
	t->_month 	= time.wMonth;
	t->_day		= time.wDay;
	t->_hour	= time.wHour;
	t->_min		= time.wMinute;
	t->_sec		= time.wSecond;
	t->_ms		= time.wMilliseconds;
}
#else // not _WIN32
void time_init()
{
	gettimeofday(&_time_start, NULL);
}

void time_mono_get(TimeVal* time)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	time->tv_sec = ts->tv_sec;
	time->tv_usec = ts->tv_nsec / 1000;
}

void time_date_get(struct time_date* t)
{
	struct timeval  time;
	struct tm*		ptm;
	gettimeofday(&time, NULL);
	ptm = localtime(&time.tv_sec);
	if (ptm){
		t->_year 	= ptm->tm_year+1900;
		t->_month 	= ptm->tm_mon;
		t->_day		= ptm->tm_mday;
		t->_hour	= ptm->tm_hour;
		t->_min		= ptm->tm_min;
		t->_sec		= ptm->tm_sec;
		t->_ms		= time.tv_usec / 1000;
	}
}
#endif // _WIN32

int  time_compare(TimeVal* t1, TimeVal* t2)
{
	long usec;
	long sec = (long)(t1->tv_sec - t2->tv_sec);
	if (sec) return sec;
	
	usec = t1->tv_usec - t2->tv_usec;
	return usec;
}

void time_sub(TimeVal* t1, TimeVal* t2, TimeVal* ret)
{
	ret->tv_sec = t1->tv_sec - t2->tv_sec;
	ret->tv_usec = t1->tv_usec - t2->tv_usec;
	if (ret->tv_usec < 0){
		ret->tv_sec--;
		ret->tv_usec += 1000000;
	}
}

