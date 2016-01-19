#ifndef _CALLSTACK_WIN32_H_
#define _CALLSTACK_WIN32_H_

#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif

void CallStackPrint(FILE* fd);

#ifdef __cplusplus
}
#endif

#endif // _CALLSTACK_WIN32_H_