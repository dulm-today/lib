#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "vadef.h"
#include "printf_safe.h"



#if defined(__GNUC__)
	#define LONGLONG	long long
	#define LONGDOUBLE	long double
#elif defined(_MSC_VER)	
	#define LONGLONG	__int64
	#define LONGDOUBLE	long double
#endif


#define OUTPUT_SIZE	 64
#define MIN(x,y)	((x) <= (y) ? (x) : (y))
#define MAX(x,y)	((x) >= (y) ? (x) : (y))

static int _outputvs_calc(const char* fmt, va_list ap)
{
	size_t size = 0;
	char _bf[OUTPUT_SIZE];
	char _fm[OUTPUT_SIZE];
	const char *p = fmt, *start;
	char* end;
	int rc, err = 0, wide, prec, type_size, wide_char;
	va_list copy;

	while(*p){
		if (*p != '%'){
			size++; p++;
		}
		else {
			switch(*(p+1)){
				case 0:
					size++; p++;
					continue;
				case '%':
					size++; p += 2;
					continue;
				default:
					break;
			}
			start = p++;	// start -> %
			va_copy(copy, ap);
			
			// flag
			while(strchr("-+ #0", *p))
				p++;

			// wide
			if (*p == '*'){
				wide = va_arg(ap, int);
				p++;
			} else {
				wide = strtol(p, &end, 10);
				p = end;
			}

			// precision
			if (*p == '.'){
				p++;
				if (*p == '*'){
					prec = va_arg(ap, int);
					p++;
				} else {
					prec = strtol(p, &end, 10);
					p = end;
				}
			}
			else {
				prec = INT_MAX;
			}

			// prefix
			type_size = 0; wide_char = -1;
			while (strchr("hlLIw", *p)){
				switch(*p++){
					case 'h':
						type_size = 0; wide_char = 0; break;
					case 'l':
						type_size++; wide_char = 1; break;
					case 'L':
						type_size = 2; break;
					case 'I':
						if (isdigit(*p)){
							if (*p == '3' && *(p+1) == '2')
								type_size = 0;
							else if (*p == '6' && *(p+1) == '4')
								type_size = 2;
							else
								assert(0);
							p += 2;
						} else if (sizeof(size_t) == 4){
							type_size = 0;
						} else {
							type_size = 2;
						}
						break;
					case 'w':
						type_size = 0; wide_char = 1; break;
					default:
						assert(0);
				}
			}

			rc = -2;
			// data type
			switch (*p){
				case 'd':
				case 'i':
				case 'o':
				case 'u':
				case 'x':
				case 'X':
					switch(type_size){
						case 0:
							va_arg(ap, int); break;
						case 1:
							va_arg(ap, long); break;
						case 2:
							va_arg(ap, LONGLONG); break;
						default:
							assert(0);
					}
					break;
				case 'c':
				case 'C':
					va_arg(ap, int); 
					break;
				case 'a':
				case 'A':
				case 'f':
				case 'e':
				case 'E':
				case 'g':
				case 'G':
					if (type_size){
						va_arg(ap, LONGDOUBLE);
					} else {
						va_arg(ap, double);
					}
					break;
				case 'n':
					va_arg(ap, void*);
					rc = 0;
					break;
				case 'p':
					va_arg(ap, void*);
					break;
				case 's':
					if (wide_char > 0){
						rc = wcstombs(NULL, va_arg(ap, wchar_t*), 0);
					} else {
						rc = strlen(va_arg(ap, char*));
					}
					break;
				case 'S':
					// wide-char
					if (wide_char == 0)
						rc = strlen(va_arg(ap, char*));
					else
						rc = wcstombs(NULL, va_arg(ap, wchar_t*), 0);
					break;
				case 'Z':
					assert(0);
				default:
					rc = p+1-start;
					break;
			}
			if (-2 == rc){
				if (wide >= OUTPUT_SIZE) {
					rc = MAX(rc, wide);
					rc = MIN(rc, prec);
				} else {
					assert(p+1-start <= sizeof(_fm)/sizeof(_fm[0])-1);
					strncpy(_fm, start, p+1-start);
					_fm[p+1-start] = 0;
					rc = vsnprintf_safe(_bf, OUTPUT_SIZE, _fm, copy);
					if (rc < 0)
						err = 1;
				}
			}
			else if (-1 == rc) {
				err = 1;
			}
			else {
				rc = MAX(rc, wide);
				rc = MIN(rc, prec);
			}
			va_end(copy);
			size += rc;
			p++;
			if (err)
				return -1;
		}
	}
	
	return size;
}

static int _outputvws_calc(const wchar_t* fmt, va_list ap)
{
	size_t size = 0;
	wchar_t _bf[OUTPUT_SIZE];
	wchar_t _fm[OUTPUT_SIZE];
	const wchar_t *p = fmt, *start;
	wchar_t* end;
	int rc, err = 0, wide, prec, type_size, wide_char;
	va_list copy;

	while(*p){
		if (*p != L'%'){
			size++; p++;
		}
		else {
			switch(*(p+1)){
				case 0:
					size++; p++;
					continue;
				case L'%':
					size++; p += 2;
					continue;
				default:
					break;
			}
			start = p++;	// start -> %
			va_copy(copy, ap);
			
			// flag
			while(wcschr(L"-+ #0", *p))
				p++;

			// wide
			if (*p == L'*'){
				wide = va_arg(ap, int);
				p++;
			} else {
				wide = wcstol(p, &end, 10);
				p = end;
			}

			// precision
			if (*p == L'.'){
				p++;
				if (*p == L'*'){
					prec = va_arg(ap, int);
					p++;
				} else {
					prec = wcstol(p, &end, 10);
					p = end;
				}
			}
			else {
				prec = INT_MAX;
			}

			// prefix
			type_size = 0; wide_char = -1;
			while (wcschr(L"hlLIw", *p)){
				switch(*p++){
					case L'h':
						type_size = 0; wide_char = 0; break;
					case L'l':
						type_size++; wide_char = 1; break;
					case L'L':
						type_size = 2; break;
					case L'I':
						if (iswdigit(*p)){
							if (*p == L'3' && *(p+1) == L'2')
								type_size = 0;
							else if (*p == L'6' && *(p+1) == L'4')
								type_size = 2;
							else
								assert(0);
							p += 2;
						} else if (sizeof(size_t) == 4){
							type_size = 0;
						} else {
							type_size = 2;
						}
						break;
					case L'w':
						type_size = 0; wide_char = 1; break;
					default:
						assert(0);
				}
			}

			rc = -2;
			// data type
			switch (*p){
				case L'd':
				case L'i':
				case L'o':
				case L'u':
				case L'x':
				case L'X':
					switch(type_size){
						case 0:
							va_arg(ap, int); break;
						case 1:
							va_arg(ap, long); break;
						case 2:
							va_arg(ap, LONGLONG); break;
						default:
							assert(0);
					}
					break;
				case L'c':
				case L'C':
					va_arg(ap, int); 
					break;
				case L'a':
				case L'A':
				case L'f':
				case L'e':
				case L'E':
				case L'g':
				case L'G':
					if (type_size){
						va_arg(ap, LONGDOUBLE);
					} else {
						va_arg(ap, double);
					}
					break;
				case L'n':
					va_arg(ap, void*);
					rc = 0;
					break;
				case L'p':
					va_arg(ap, void*);
					break;
				case L's':
					if (-1 == wide_char || wide_char){
						rc = wcslen(va_arg(ap, wchar_t*));
					} else {
						rc = mbstowcs(NULL, va_arg(ap, char*), 0);
					}
					break;
				case L'S':
					// multi-char
					if (wide_char <= 0)
						rc = mbstowcs(NULL, va_arg(ap, char*), 0);
					else
						rc = wcslen(va_arg(ap, wchar_t*));
					break;
				case L'Z':
					assert(0);
				default:
					rc = p+1-start;
					break;
			}
			if (-2 == rc){
				if (wide >= OUTPUT_SIZE) {
					rc = MAX(rc, wide);
					rc = MIN(rc, prec);
				} else {
					assert(p+1-start <= sizeof(_fm)/sizeof(_fm[0])-1);
					wcsncpy(_fm, start, p+1-start);
					_fm[p+1-start] = 0;
					rc = vsnwprintf_safe(_bf, OUTPUT_SIZE, _fm, copy);
					if (rc < 0)
						err = 1;
				}
			}
			else if (-1 == rc) {
				err = 1;
			}
			else {
				rc = MAX(rc, wide);
				rc = MIN(rc, prec);
			}
			va_end(copy);
			size += rc;
			p++;
			if (err)
				return -1;
		}
	}
	
	return size;
}