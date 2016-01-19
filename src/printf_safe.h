#ifndef __PRINTF_SAFE_H__
#define __PRINTF_SAFE_H__
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif


/*
  if buf == NULL || size == 0
  	return the character counte will to print.
  else if needsize > size
    buf[size-1] = 0
    return -1
  else
    buf[count] = 0
    return character counte printed;

    size is the character num of buf
*/

int snprintf_safe(char* buf, int size, const char* format, ...);
int vsnprintf_safe(char* buf, int size, const char* format, va_list ap);

int snwprintf_safe(wchar_t* buf, int size, const wchar_t* format, ...);
int vsnwprintf_safe(wchar_t* buf, int size, const wchar_t* format, va_list ap);

#ifdef __cplusplus
}
#endif

#endif // __PRINTF_SAFE_H__
