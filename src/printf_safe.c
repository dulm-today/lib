#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


static int _outputvs_safe(char* buf, int cnt, const char* fmt, va_list ap)
{
	int ret;

#if defined(_MSC_VER)
	if (NULL == buf || 0 == cnt)
		return _vscprintf(fmt, ap);
	else
	#if _MSC_VER >= 1500
	ret = vsnprintf_s(buf, cnt, _TRUNCATE, fmt, ap);
	#else
	ret = _vsnprintf(buf, cnt, fmt, ap);
	#endif
#else
	ret = vsnprintf(buf, cnt, fmt, ap);
#endif

	if (NULL != buf && cnt > 0){
		if (ret < 0)
			buf[cnt-1] = 0;
		else if (ret >= cnt)
			ret = -1;
	}

	return ret;
}

static int _outputvws_safe(wchar_t* buf, int cnt, const wchar_t* fmt, va_list ap)
{
	int ret;

#if defined(_MSC_VER)
	if (NULL == buf || 0 == cnt)
		return _vscwprintf(fmt, ap);
	#if _MSC_VER >= 1500
	ret = _vsnwprintf_s(buf, cnt, _TRUNCATE, fmt, ap);
	#else
	ret = _vsnwprintf(buf, cnt, fmt, ap);
	#endif
#else
	ret = vsnwprintf(buf, cnt, fmt, ap);
#endif
	
	if (NULL != buf && cnt > 0){
		if (ret < 0)
			buf[cnt-1] = 0;
		else if (ret >= cnt)
			ret = -1;
	}

	return ret;
}


int snprintf_safe(char* buf, int size, const char* format, ...)
{
	int len;
	va_list ap;
	va_start(ap, format);
	len = _outputvs_safe(buf, size, format, ap);
	va_end(ap);
	
	return len;
}

int vsnprintf_safe(char* buf, int size, const char* format, va_list ap)
{
	return _outputvs_safe(buf, size, format, ap);
}


int snwprintf_safe(wchar_t* buf, int size, const wchar_t* format, ...)
{
	int len;
	va_list ap;
	va_start(ap, format);
	len = _outputvws_safe(buf, size, format, ap);
	va_end(ap);
	
	return len;
}

int vsnwprintf_safe(wchar_t* buf, int size, const wchar_t* format, va_list ap)
{
	return _outputvws_safe(buf, size, format, ap);
}
