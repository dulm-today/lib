#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <assert.h>
#include <limits.h>
#include <math.h>
#include "vadef.h"

#if defined(__GNUC__)
	#define LONGLONG	long long
	#define LONGDOUBLE	long double
#elif defined(_MSC_VER)	
	#define LONGLONG	__int64
	#define LONGDOUBLE	long double
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

#define PRNT_DEFAULT_SIZE	(INT_MAX)
#define PRNT_BUF_SIZE		(512)

#define PRNT_PRE_SIZE		(16)
#define PRNT_SUF_SIZE		(16)
#define PRNT_FMT_SIZE		(16)
#define PRNT_ARFMT_SIZE		(PRNT_FMT_SIZE*4)

#define PRNT_ARRAY_CH		('w')

enum{
	PRNT_ARRAY	= 1,
};

enum {
	TYPE_char   = 1 ,
	TYPE_short		,
	TYPE_int		,	/* char, short, int, */
	TYPE_long		,	/* 64 bit */
	TYPE_float		,
	TYPE_double		,	/* double */
	TYPE_ldouble	,	/* long double */
	TYPE_ptr		,	/* ptr */
};

static int _s_type_size[8] = {
	sizeof(char),
	sizeof(short),
	sizeof(int),
	sizeof(LONGLONG),
	sizeof(float),
	sizeof(double),
	sizeof(LONGDOUBLE),
	sizeof(void*)
};

#define TYPESIZE(t)	(((t)>=TYPE_char && (t)<=TYPE_ptr) ? _s_type_size[(t)-1] : -1 )

typedef int (*OUTPUTFN)(void** s, int cnt, const char* fmt, ...);
typedef int (*OUTPUTFN_V)(void** s, int cnt, const char* fmt, va_list ap);

static int _outputf(void** s, int cnt, const char* fmt, ...)
{
	int ret;
	FILE* f = *(FILE**)s;
	va_list ap;

	va_start(ap, fmt);
	ret = vfprintf(f, fmt, ap);
	va_end(ap);

	return ret;
}

static int _outputvf(void** s, int cnt, const char* fmt, va_list ap)
{
	FILE* f = *(FILE**)s;

	return vfprintf(f, fmt, ap);
}

static int _outputvs_safe(char* buf, int cnt, const char* fmt, va_list ap)
{
	int ret;
	
#if defined(_MSC_VER) && _MSC_VER >= 1500
	ret = vsnprintf_s(buf, cnt, _TRUNCATE, fmt, ap);
#elif defined(_MSC_VER)
	ret = _vsnprintf(buf, cnt, fmt, ap);
#else
	ret = vsnprintf(buf, cnt, fmt, ap);
#endif
	if (cnt > 0){
		if (ret < 0)
			buf[cnt-1] = '\0';
		else if (ret >= cnt)
			ret = -1;
	}

	return ret;
}

static int _outputs(void** s, int cnt, const char* fmt, ...)
{
	int ret;
	char** p = (char**)s;
	va_list ap;
 
	va_start(ap, fmt);
	ret = _outputvs_safe(*p, cnt, fmt, ap);
	va_end(ap);

	if (ret < 0 || ret >= cnt)
		return -1;
	*p += ret;

	return ret;
}

static int _outputvs(void** s, int cnt, const char* fmt, va_list ap)
{
	int ret;
	char** p = (char**)s;
	
	ret = _outputvs_safe(*p, cnt, fmt, ap);
	if (ret < 0 || ret >= cnt)
		return -1;
	*p += ret;

	return ret;
}


/*
* 
* Info:  get the normal format and va_list
*  
**/
static int _donormalformat(const char* format, va_list* ap, int* flag)
{
	const char* ptr = format;
	int short_width = 0, wide_width = 0;

	while (*ptr != '\0')
	{
		if (*ptr != '%')
			ptr++;
		else
		{
			switch (*(ptr+1))
			{
				case '\0':
					ptr++;
					continue;
				case '%':
					ptr += 2;
					continue;
				case PRNT_ARRAY_CH:
					*flag = PRNT_ARRAY;
					return ptr-format;
				default:
					ptr++;
					break;
			}
			
			while (strchr("-+ #0", *ptr))
				ptr++;

			if (*ptr == '*'){
				va_arg(*ap,int);
				ptr++;
			}
			else
				while (isdigit(*ptr))
					ptr++;

			if (*ptr == '.')
			{
				ptr++;
				if (*ptr == '*'){
					va_arg(*ap,int);
					ptr++;
				}
				else
					while (isdigit(*ptr))
						ptr++;
			}

			while (strchr("hlL", *ptr))
			{
				switch (*ptr)
				{
					case 'h':
						short_width = 1;
						break;
					case 'l':
						wide_width++;
						break;
					case 'L':
						wide_width = 2;
						break;
					default:
						assert(0);
				}
				ptr++;
			}

			switch (*ptr)
			{
				case 'd':
				case 'i':
				case 'o':
				case 'u':
				case 'x':
				case 'X':
				case 'c':
					{
						if (short_width){
							va_arg(*ap, int);
							ptr++;
						}
						else{
							switch (wide_width)
							{
								case 0:
									va_arg(*ap, int);
									ptr++;
									break;
								case 1:
									va_arg(*ap, long);
									ptr++;
									break;
								case 2:
									va_arg(*ap, LONGLONG);
									ptr++;
									break;
								default:
									assert(0);
							}
						}
					}
					break;
				case 'f':
				case 'e':
				case 'E':
				case 'g':
				case 'G':
					{
						if (wide_width == 0){
							va_arg(*ap, double);
							ptr++;
						}
						else{
							va_arg(*ap, LONGDOUBLE);
							ptr++;
						}
					}
					break;
				case 's':
					va_arg(*ap, void*);
					ptr++;
					break;
				case 'p':
					va_arg(*ap, char*);
					ptr++;
					break;
				default:
					break;
			}/* End of switch (*ptr) */
		}
	}

	return ptr-format;
}


/*
*
* Info   : deal the array format, get the data type, prefix, suffix and the output format 
* 
* return : > 0 ok; <= 0 error 
* 		
*  format: 
*  	eg: %w%c[%02x]%w
**/
static int _doarrayformat(const char* format, int* flag, char* pre, char* suf, char* fmt)
{
	const char* p1 = format+2;
	const char* p2;
	int type = 0;
	int wide_width = 0;

	/* get type */
	p2 = strchr(p1, '%');
	if (NULL == p2)
		return -1;
	
	p1 = p2+1;
	while(strchr("l", *p1))
	{
		wide_width++;
		p1++;
	}

	switch(*p1++)
	{
		case 'c':
			type = TYPE_char;
			break;
		case 'h':
			type = TYPE_short;
			break;
		case 'd':
			if (wide_width)
				type = TYPE_long;
			else
				type = TYPE_int;
			break;
		case 'f':
			if (1 == wide_width)
				type = TYPE_double;
			else if( 2 == wide_width)
				type = TYPE_ldouble;
			else
				type = TYPE_float;
			break;
		case 'p':
			type = TYPE_ptr;
			break;
		default:
			return -1;
	}

	/* get prefix */
	p2 = p1;
	while (*p2 != '\0'){
		if (*p2 != '%')
			p2++;
		else if (*(p2+1) == '%')
			p2+=2;
		else
			break;
	}

	if (*p2 == '\0')
		return -1;

	memcpy(pre, p1, p2-p1);
	pre[p2-p1] = '\0';

	/* get format */
	p1 = p2++;
	while (*p2 != '\0' && *p2 != '%')
	{
		switch( *p2 )
		{
			case '*':
				return -1;
			case 'd':
			case 'i':
			case 'o':
			case 'u':
			case 'x':
			case 'X':
			case 'f':
			case 'e':
			case 'E':
			case 'g':
			case 'G':
			case 'c':
			case 's':
			case 'S':
			case 'p':
				break;
			default:
				p2++;
				continue;
		}
		break;
	}

	if (*p2 == '\0' || *p2 == '%')
		return -1;

	memcpy(fmt, p1, p2-p1+1);
	fmt[p2-p1+1] = '\0';

	p1 = ++p2;

	/* get suffix */
	while (*p2 != '\0'){
		if (*p2 != '%')
			p2++;
		else if (*(p2+1) == '%')
			p2+=2;
		else
			break;
	}
	if (*p2 == '\0')
		return -1;
	if (*(p2+1) != PRNT_ARRAY_CH)
		return -1;

	memcpy(suf, p1, p2-p1);
	suf[p2-p1] = '\0';
	
	*flag = type;

	return ((p2+2)-format);
}

static int _doarrayprnt(OUTPUTFN _outfn, OUTPUTFN_V _outfnv, void** s, int size, 
				int type, const char* pre, const char* suf, const char* fmt,
				int array_size, void* array_ptr)
{
	int 	ret;
	char 	format[PRNT_ARFMT_SIZE];
	char*	ff = format;

	char*	p = (char*)array_ptr;
	int		inc = TYPESIZE(type);
	int		cnt = size;

	ret = _outputs((void**)&ff, PRNT_ARFMT_SIZE, "%s%s%s", pre, fmt, suf);
	if (ret < 0)
		return ret;

	while (array_size > 0 && cnt > 0)
	{

		switch(type)
		{
			case TYPE_char:
				ret = _outfn(s, cnt, format, *(char*)p);
				break;
			case TYPE_short:
				ret = _outfn(s, cnt, format, *(short*)p);
				break;
			case TYPE_int:
				ret = _outfn(s, cnt, format, *(int*)p);
				break;
			case TYPE_long:
				ret = _outfn(s, cnt, format, *(LONGLONG*)p);
				break;
			case TYPE_float:
				ret = _outfn(s, cnt, format, *(float*)p);
				break;
			case TYPE_double:
				ret = _outfn(s, cnt, format, *(double*)p);
				break;
			case TYPE_ldouble:
				ret = _outfn(s, cnt, format, *(LONGDOUBLE*)p);
				break;
			case TYPE_ptr:
				ret = _outfn(s, cnt, format, *(void**)p);
				break;
			default:
				assert(0);
		}
	
		if (ret < 0)
			return ret;

		p += inc;
		array_size--;
		cnt -= ret;
	}

	return size-cnt;
}

static int _output_helper(OUTPUTFN _outfn, OUTPUTFN_V _outfnv, void* s, int size, const char* format, va_list ap)
{
	int				ret = 0;
	int				pos = 0;
	unsigned int 	cnt = (unsigned int)size;
	const char*		pstr = format;

	va_list			cp;
	char 			_buf[PRNT_BUF_SIZE];
	char 			_pre[PRNT_PRE_SIZE];
	char 			_suf[PRNT_SUF_SIZE];
	char 			_fmt[PRNT_FMT_SIZE];
	int				flag;
	int 			type;
	int				array_size = 0;
	void*			array_ptr = NULL;

	while (*pstr != '\0' && cnt != 0)
	{
		flag = 0;

		/* get the normal format*/
		va_copy(cp,ap);

		flag = 0;
		/* pass the normal format */
		pos = _donormalformat(pstr, &ap, &flag);
		if (pos != 0)
		{
			assert(pos < PRNT_BUF_SIZE-1);

			memcpy(_buf, pstr, pos);
			_buf[pos] = '\0';
			
			ret = _outfnv(&s, cnt, _buf, cp);
			
			if (ret < 0){
				va_end(cp);
				return ret;
			}
			
			cnt -= ret;
		}
		va_end(cp);

		pstr += pos;

		/* deal the array print */
		switch (flag)
		{
			case 0:
				break;
			case PRNT_ARRAY:
				{
					ret = _doarrayformat(pstr, &type, _pre, _suf, _fmt);
					if (ret < 0)
						return ret;

					pstr += ret;
	
					array_size = abs(va_arg(ap, int));
					array_ptr = va_arg(ap, void*);
					ret = _doarrayprnt(_outfn, _outfnv, &s, cnt, type, 
								_pre, _suf, _fmt, array_size, array_ptr);
					
					if (ret < 0)
						return ret;

					cnt -= ret;
				}
				break;
			default:
				assert(0);
		}
	}

	return size-cnt;
}


/* string print format */

int snprintfplus(char* buf, int size, const char* format, ...)
{
	int ret;
	va_list ap;

	va_start(ap,format);
	ret = _output_helper(_outputs, _outputvs, buf, size, format, ap);
	va_end(ap);

	return ret;
}

int vsnprintfplus(char* buf, int size, const char* format, va_list ap)
{
	return _output_helper(_outputs, _outputvs, buf, size, format, ap);
}


/* file print format */
int fprintfplus(FILE* f, const char* format, ...)
{
	int ret;
	va_list ap;

	va_start(ap,format);
	ret = _output_helper(_outputf, _outputvf, f, PRNT_DEFAULT_SIZE, format, ap);
	va_end(ap);

	return ret;
}

int vfprintfplus(FILE* f, const char* format, va_list ap)
{
	return _output_helper(_outputf, _outputvf, f, PRNT_DEFAULT_SIZE, format, ap);
}

