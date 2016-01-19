#ifndef ___LIBTIME__H___
#define ___LIBTIME__H___

#include <time.h>
#ifdef __cplusplus
extern "C"{
#endif

struct time_date
{
	unsigned short	_year;
	unsigned short	_month;
	unsigned short	_day;
	unsigned short	_hour;
	unsigned short	_min;
	unsigned short	_sec;
	unsigned short	_ms;
};

typedef struct _file_time
{
	unsigned int 	low;
	unsigned int	high;
}filetime;

typedef struct _TimeVal
{
	time_t	tv_sec;
	long	tv_usec;
}TimeVal;


void time_init();
void time_mono_get(TimeVal* time);
int  time_compare(TimeVal* t1, TimeVal* t2);
void time_sub(TimeVal* t1, TimeVal* t2, TimeVal* ret);

void time_date_get(struct time_date* time);

#ifdef __cplusplus
}
#endif

#endif //___LIBTIME__H___