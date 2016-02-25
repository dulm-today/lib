#include "before_after_main.h"
#include <stdio.h>

void before_main_void ()
{
	printf("%s\n", __FUNCTION__);
}
int  before_main_int()
{
	printf("%s\n", __FUNCTION__);
	return 0;
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

RUN_BEFORE_MAIN(before_main_int);
RUN_BEFORE_MAIN(before_main_void);
RUN_BEFORE_AFTER_MAIN(before_after_main);
RUN_AFTER_MAIN(after_main);

int main ()
{
	printf("main\n");
	atexit(after_exit);
	return 0;
}
