#include "test_core.h"
#include "callstack_win32.h"

void callstack_func1(int);
void callstack_func2(int);

void callstack_func1(int dep)
{
	if (0 == dep) {
		CallStackPrint(stderr);
	}
	else if ((--dep) % 2) {
		callstack_func1(dep);
	}
	else {
		callstack_func2(dep);
	}
}

void callstack_func2(int dep)
{
	if (0 == dep) {
		CallStackPrint(stderr);
	}
	else if ((--dep) % 2) {
		callstack_func1(dep);
	}
	else {
		callstack_func2(dep);
	}
}

int callstack_main()
{
	callstack_func1(11);

	return 0;
}

Test callstack_test("callstack", callstack_main, 1);
