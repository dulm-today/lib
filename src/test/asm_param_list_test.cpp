#include "test_core.h"
#include "stdio.h"
#include "stdarg.h"

int asm_param_func(const char* fmt, ...)
{
	va_list ap;
	
	fprintf(stderr, "asm_param_func: ");

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	return 0;
}


int asm_param_caller(const char* fmt, int count)
{
	int i;
	const char* param[5] = {"aaa", "bbb", "ccc", "ddd", "eee"};

	for (i = 0; i < count && i < 5; ++i){
		__asm{
			lea ebx, param
			mov edx, i
			mov eax, [ebx+edx*4]
			push eax
		}
	}

	asm_param_func(fmt);

	for (i = 0; i < count && i < 5; ++i){
		__asm{
			pop eax
		}
	}
	return 0;
}


int _asm_param_list()
{
	int error = 0;

	asm_param_caller("\n", 0);
	asm_param_caller("%s\n", 1);
	asm_param_caller("%s %s\n", 2);
	asm_param_caller("%s %s %s\n", 3);
	asm_param_caller("%s %s %s %s\n", 4);
	asm_param_caller("%s %s %s %s %s\n", 5);
	asm_param_caller("%s %s %s %s %s %s\n", 6);

	return error;
}

Test asm_param_list_test("asm_param_list", _asm_param_list, 1);

