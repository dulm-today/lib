#include "stdio.h"
#include "printf_safe.h"
#include "test_core.h"
#include "log.h"
#include <locale.h>

static LOG_V_MODULE(log_v, "printf_safe_log_v", g_global_log_output, 0);
static LOG_MODULE(log, "printf_safe_log", g_global_log_output, 0);

char large_str[2001] = {0};

void large_str_init()
{
	int i;

	for (i = 0; i < 2000;){
		sprintf(&large_str[i], "helloworld");
		i += 10;
	}
}

#define PRINT_CHECK(fn, buf, size, use, ret, fmt, ...)	\
	len = fn(buf, use, fmt, __VA_ARGS__);\
	fprintf(stderr, "[%-5s]%s %d: %d(%d) %s \n", len == ret ? "OK" : "ERROR",\
					#fn, use, len, ret, buf);\
	error += (len == ret ? 0 : 1); 

#define PRINT_CHECKW(fn, buf, size, use, ret, fmt, ...)	\
	len = fn(buf, use, fmt, __VA_ARGS__);\
	fprintf(stderr, "[%-5s]%s %d: %d(%d) %ls \n", len == ret ? "OK" : "ERROR",\
					#fn, use, len, ret, buf);\
	error += (len == ret ? 0 : 1); 


int printf_safe_print_test()
{
	int len = 0, error = 0;
	char buffer[12];
	wchar_t buffer2[12];

	LOG_CB_V(log_v, LOG_DEBUG, "");
	LOG_CB_V(log_v, LOG_DEBUG, "%.*s", 100, large_str);
	LOG_CB_V(log_v, LOG_DEBUG, "%s", large_str);

	LOG_CB(log, LOG_DEBUG, "");
	//__LOG_CB(log, LOG_DEBUG, large_str);

	PRINT_CHECK(snprintf_safe, buffer, 12, 1, -1, "helloworld");
	PRINT_CHECK(snprintf_safe, buffer, 12, 2, -1, "helloworld");
	PRINT_CHECK(snprintf_safe, buffer, 12, 10, -1, "helloworld");
	PRINT_CHECK(snprintf_safe, buffer, 12, 11, 10, "helloworld");
	PRINT_CHECK(snprintf_safe, buffer, 12, 12, 10, "helloworld");

	PRINT_CHECKW(snwprintf_safe, buffer2, 12, 1, -1, L"helloworld");
	PRINT_CHECKW(snwprintf_safe, buffer2, 12, 2, -1, L"helloworld");
	PRINT_CHECKW(snwprintf_safe, buffer2, 12, 10, -1, L"helloworld");
	PRINT_CHECKW(snwprintf_safe, buffer2, 12, 11, 10, L"helloworld");
	PRINT_CHECKW(snwprintf_safe, buffer2, 12, 12, 10, L"helloworld");

	return error > 0 ? 1 : 0;
}

#define PRINT_CALC_CHECK(fn, buf, fmt, ...)	\
	len1 = fn(NULL, 0, fmt, __VA_ARGS__);\
	len2 = fn(buf, sizeof(buf)/sizeof(buf[0]), fmt, __VA_ARGS__);\
	fprintf(stderr, "[%-5s]%s %s: %d(%d)[%s]\n", len1 == len2 ? "OK" : "ERROR",\
			#fn, fmt, len1, len2, buf);\
	error += (len1 == len2 ? 0 : 1);

#define PRINT_CALC_CHECKW(fn, buf, fmt, ...)	\
	len1 = fn(NULL, 0, fmt, __VA_ARGS__);\
	len2 = fn(buf, sizeof(buf)/sizeof(buf[0]), fmt, __VA_ARGS__);\
	fprintf(stderr, "[%-5s]%s %ls: %d(%d)[%S]\n", len1 == len2 ? "OK" : "ERROR",\
			#fn, fmt, len1, len2, buf);\
	error += (len1 == len2 ? 0 : 1);

int printf_safe_calc_test()
{
	int len1, len2, error = 0;
	char buffer[1024];
	wchar_t buffer2[1024];
	long l = 100;
	__int64 i64 = 1000;
	double f = 10.1111;
	wchar_t wc = L'好';
	char    ch = 'a';
	wchar_t wstr[] = L"好孩子";
	char    str[] = "天天向上";

	PRINT_CALC_CHECK(snprintf_safe, buffer, "");
	PRINT_CALC_CHECK(snprintf_safe, buffer, "%-5.d  %05d  %05ld %+#5I32d %+05Id %5I64d", 1000, 1000, l, l, -l, i64);
	PRINT_CALC_CHECK(snprintf_safe, buffer, "%5.10f %-*.*f", f, 5, 10, f);
	PRINT_CALC_CHECK(snprintf_safe, buffer, "%c %wc %hc %C", ch, wc, ch, wc);
	PRINT_CALC_CHECK(snprintf_safe, buffer, "%s %S %hS %ls", str, wstr, str, wstr);
	PRINT_CALC_CHECK(snprintf_safe, buffer, "好好学习", str, wstr, str, wstr);

	fprintf(stderr, "\n");

	PRINT_CALC_CHECKW(snwprintf_safe, buffer2, L"");
	PRINT_CALC_CHECKW(snwprintf_safe, buffer2, L"%-5.d  %05d  %05ld %+#5I32d %+05Id %5I64d", 1000, 1000, l, l, -l, i64);
	PRINT_CALC_CHECKW(snwprintf_safe, buffer2, L"%5.10f %-*.*f", f, 5, 10, f);
	PRINT_CALC_CHECKW(snwprintf_safe, buffer2, L"%c %wc %hc %C", ch, wc, ch, wc);
	PRINT_CALC_CHECKW(snwprintf_safe, buffer2, L"%s %S %hs %lS", wstr, str, str, wstr);
	PRINT_CALC_CHECKW(snwprintf_safe, buffer2, L"好好学习", str, wstr, str, wstr);

	return error > 0 ? 1 : 0;
}

int printf_safe_test()
{
	int error = 0;
	

	large_str_init();
	
	error += printf_safe_print_test();
	fprintf(stderr, "\n");
	error += printf_safe_calc_test();
	
	return error;
}

Test printf_safe_item("printf_safe", printf_safe_test, 2);

