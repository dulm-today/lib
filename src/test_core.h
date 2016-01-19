#ifndef __TEST_CORE_H__
#define __TEST_CORE_H__

#include <stdio.h>

/* return number of error */
typedef int (*test_main)();

class Test
{
public:
	Test(const char* name, test_main fn, int total);
	
};

#endif // __TEST_CORE_H__
