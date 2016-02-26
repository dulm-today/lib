#include "before_after_main.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _MSC_VER
#include <stdarg.h>
#include <windows.h>
#define printf(fmt, ...)	_my_printf(fmt, __VA_ARGS__)

void _my_printf(const char* fmt, ...)
{
	char buffer[1024];

	va_list ap;
	va_start(ap, fmt);
	_vsnprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

	OutputDebugString(buffer);
}
#endif

void before_main_void ()
{
	printf("%s\n", __FUNCTION__);
}
void  before_main_int()
{
	printf("%s\n", __FUNCTION__);
}

void before_after_main()
{
	printf("%s\n", __FUNCTION__);
}

void after_main()
{
	printf("%s\n", __FUNCTION__);
}

void after_exit()
{
	printf("%s\n", __FUNCTION__);
}

//RUN_BEFORE_MAIN((void(*)(void))before_main_int);
RUN_BEFORE_MAIN(before_main_void);
RUN_BEFORE_AFTER_MAIN(before_after_main);
#ifdef __GNUC__
RUN_BEFORE_MAIN(before_main_int);
#endif
RUN_AFTER_MAIN(after_main);

int main ()
{
	printf("main\n");
	atexit(after_exit);
	return 0;
}
