#include "tstring.h"
#include "test_core.h"
#include "vadef.h"

static char buffer[1024];

int tstring_format_check(const char* fmt, ...){
	va_list ap;
	va_start(ap, fmt);
	
	std::tstring s;
	int size = vsnprintf_safe(buffer, 1024, fmt, ap);
	s.formatv(fmt, ap);
	va_end(ap);
	int rc = 0;
	
	if (size != s.length())
		rc++;
	if (strcmp(buffer, s.c_str()) != 0)
		rc++;

	fprintf(stderr, "[%s] %s [%s]\n", buffer, rc == 0 ? "==" : "!=", s.c_str());
	return rc == 0 ? 0 : 1;
}

#define TSTR_PROC_CHECK(s1, s2, fn)	\
	do{\
		std::tstring _s1(s1);\
		_s1.fn();\
		int _cmp = strcmp(_s1.c_str(), s2);\
		fprintf(stderr, "[%-5s]%s.%s -> %s(%s)\n", 0 == _cmp ? "OK" : "ERROR",\
			s1, #fn, _s1.c_str(), s2);\
		error += _cmp == 0 ? 0 : 1;\
	}while(0)
	
#define TSTR_PROC_CHECK1(s1, s2, fn, va1)	\
	do{\
		std::tstring _s1(s1);\
		_s1.fn(va1);\
		int _cmp = strcmp(_s1.c_str(), s2);\
		fprintf(stderr, "[%-5s]%s.%s(%s) -> %s(%s)\n", 0 == _cmp ? "OK" : "ERROR",\
			s1, #fn, va1, _s1.c_str(), s2);\
		error += _cmp == 0 ? 0 : 1;\
	}while(0)
	
#define TSTR_PROC_CHECK2(s1, s2, fn, va1, va2)	\
	do{\
		std::tstring _s1(s1);\
		_s1.fn(va1, va2);\
		int _cmp = strcmp(_s1.c_str(), s2);\
		fprintf(stderr, "[%-5s]%s.%s(%s,%s) -> %s(%s)\n", 0 == _cmp ? "OK" : "ERROR",\
			s1, #fn, va1, va2, _s1.c_str(), s2);\
		error += _cmp == 0 ? 0 : 1;\
	}while(0)

std::tstring tstring_add(const std::tstring& s1, const std::string& s2)
{
	return s1 + s2;
}

int tstring_length(const std::string& s)
{
	return s.length();
}

int tstring_test_main()
{
	int error = 0, n;

	// test format
	error += tstring_format_check("");
	error += tstring_format_check(" %d ", 1);
	error += tstring_format_check(" %d %s ", 2, "hello");
	error += tstring_format_check(" %0127d %ls ", 3, L"hello");
	error += tstring_format_check(" %-.10d %- *.*ls %-+####p  %62d %63d %64d ",
							3, 3, 4, L"hello", &error, 10, 11, 12, &n);

	TSTR_PROC_CHECK("AaBbCc", "aabbcc", tolower);
	TSTR_PROC_CHECK("AaBbCc", "AABBCC", toupper);

	TSTR_PROC_CHECK2("AaAbBBCccC", "AaAbCccC", replace, "bB", "b");
	TSTR_PROC_CHECK2("bBBBBBBBBB", "b", replace, "bB", "b");

	TSTR_PROC_CHECK1("AaAbBBCccC", "BBCccC", trimLeft, "AabdzD");
	TSTR_PROC_CHECK1("AaAbBBCccC", "AaAbBBCccC", trimLeft, "bBDzb");
	TSTR_PROC_CHECK1("AaAbBBCccC", "", trimLeft, "AaBbCc");

	TSTR_PROC_CHECK1("AaAbBBCccC", "AaA", trimRight, "CcBbz");
	TSTR_PROC_CHECK1("AaAbBBCccC", "AaAbBBCccC", trimRight, "AaBbDd");
	TSTR_PROC_CHECK1("AaAbBBCccC", "", trimRight, "AaCcDdBb");

	// test contructor
	std::string b1("b1b1b1");
	std::string b2("b2b2b2");
	std::tstring a;
	std::tstring a1("a1");
	std::tstring a2(10, '2');
	std::tstring a3("a3a3a3a3", 4);
	std::tstring a4(a3.begin() + 2, a3.end());
	std::tstring a5(std::string("a5"));
	std::tstring a6(b1);
	std::tstring a7(a);
	std::tstring a8(std::tstring("a8"));
	std::tstring a9(std::move(a2));
	std::tstring a10(std::move(b2));

	// test assign contructor
	b1 = "b1b1b1";
	b2 = "b2b2b2";
	char ch = ' ';
	a = "";
	a = ch;
	a = ' ';
	a = a1;
	a = std::move(a3);
	a = b1;
	a = std::move(b1);

	std::tstring sss;

	b1 = "b1";
	b2 = "b2";
	std::string b3("b3000000000000000000000000000000b3");

	sss += 'a';
	sss += "bb";
	sss += std::string("cc");
	sss += b1;
	sss += std::move(b3);
	sss += std::tstring("ts");
	sss += a8;
	sss += std::move(a8);

	std::tstring sss2;

	sss2 += std::move(b3);
	sss2 += (double)10.11;
	sss2 += true;

	sss.assign(b1);
	sss.assign(a8);
	sss.assign(std::move(a8));

	sss = "aaaaaaaaa";
	sss.replace(1, 3, "sssssss");

	sss = "aaaa";
	sss = std::tstring("bbb");
	sss = std::string("ccc");

	sss = std::tstring("ddd") + std::string("eee");

	sss = std::tstring("aaa") + 10.1;
	sss = std::string("bbb") + 10.1;
	sss = 10.1 + std::string("ccc");
	sss = 10.2 + std::tstring("ddd");
	sss = true + a10;
	sss = a10 + 111.11;
	sss = a10 + "aaa";
	sss = a10 + 'a';
	//sss = a10 + L"bbb";
	//sss = a10 + L'b';
	sss = a10 + a9;

	{
		std::exstring ex("aaa");
	}
	{
		std::exwstring exw(L"aaa");
	}
	
	std::exwstring exw(L"ºÃºÃÑ§Ï°");
	std::exstring ex(exw.c_str());
	std::exwstring exw2(ex.c_str());
	

	{
		std::tstring ts("ts");
		std::string  ss("ss");
		sss = tstring_add(ss, ts);
	}

	{
		sss = "100";
		std::tstring ts("101");
		std::string  ss("102");
		if (sss < ts) {
			fprintf(stderr, "%s < %s\n", sss.c_str(), ts.c_str());
		}

		if (sss >= "101") {
			fprintf(stderr, "%s < 101\n", sss.c_str());
		}
	}

	{
		std::hash<std::string> fn;
		size_t h = fn(sss);
		h = fn(b1);
	}

	tstring_length(sss); 

	return error;
}

Test tstring_test_item("tstring", tstring_test_main, 15);