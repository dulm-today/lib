#ifndef PRINTPLUS_H
#define PRINTPLUS_H

#include <stdio.h>
#include <stdarg.h>

/*
* Add print format type
*     1. array print
*        eg: %w%c[%02x]%w
*            "%w...%w" : the flag of this type
*            "%c"      : the data type of the array is char(or 1 byte)
*            "[" & "]" : the prefix and the subfix to print with every element of the array
*            "%02x"    : the print format of the data
*/

#ifdef __cplusplus
extern "C" {
#endif

int snprintfplus(char* buf, int size, const char* format, ...);
int vsnprintfplus(char* buf, int size, const char* format, va_list ap);
int fprintfplus(FILE* f, const char* format, ...);
int vfprintfplus(FILE* f, const char* format, va_list ap);


#ifdef __cplusplus
}
#endif


#endif // PRINTPLUS_H