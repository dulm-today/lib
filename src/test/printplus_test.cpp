#ifdef TEST
#include "printplus.h"
#include "test_core.h"
#include <math.h>
#include <time.h>
#include <string.h>
#ifndef M_PI
#define M_PI (3.1415926535897932385)
#endif

#define RESULT(x) do \
{ \
    int i = (x); \
    fprintf (stderr, "printed %d characters\n", i); \
    fflush(stdin); \
} while (0)

static int checkit (FILE* fd, const char * format, ...);

static int checkit (FILE* fd, const char* format, ...)
{
  int result;
  va_list ap;
  
  va_start(ap, format);
  result = vfprintfplus (fd, format, ap);
  va_end(ap);

  return result;
}

static int checkstr(FILE* fd, char* buffer, int size, const char* format, ...)
{
	int result;
	va_list ap;

	va_start(ap, format);
	result = vsnprintfplus(buffer, size, format, ap);
	va_end(ap);

	fprintf(fd, "%s", buffer );

	return result;
}

static clock_t s_start;

void timer_start()
{
	s_start = clock();
}

void timer_end()
{
	clock_t end = clock();

	fprintf(stderr, "timestart: %08d, timeend: %08d, time pass: %08d\n", s_start, end, end - s_start);
}

int printplus_main ()
{
  RESULT(checkit (stderr, "<%d>\n", 0x12345678));
  RESULT(fprintf (stderr, "<%d>\n", 0x12345678));

  RESULT(checkit (stderr, "<%200d>\n", 5));
  RESULT(fprintf (stderr, "<%200d>\n", 5));

  RESULT(checkit (stderr, "<%.300d>\n", 6));
  RESULT(fprintf (stderr, "<%.300d>\n", 6));

  RESULT(checkit (stderr, "<%100.150d>\n", 7));
  RESULT(fprintf (stderr, "<%100.150d>\n", 7));

  RESULT(checkit (stderr, "<%s>\n",
		  "jjjjjjjjjiiiiiiiiiiiiiiioooooooooooooooooppppppppppppaa\n\
777777777777777777333333333333366666666666622222222222777777777777733333"));
  RESULT(fprintf (stderr, "<%s>\n",
		 "jjjjjjjjjiiiiiiiiiiiiiiioooooooooooooooooppppppppppppaa\n\
777777777777777777333333333333366666666666622222222222777777777777733333"));

  RESULT(checkit (stderr, "<%f><%0+#f>%s%d%s>\n",
		  1.0, 1.0, "foo", 77, "asdjffffffffffffffiiiiiiiiiiixxxxx"));
  RESULT(fprintf (stderr, "<%f><%0+#f>%s%d%s>\n",
		 1.0, 1.0, "foo", 77, "asdjffffffffffffffiiiiiiiiiiixxxxx"));

  RESULT(checkit (stderr, "<%4f><%.4f><%%><%4.4f>\n", M_PI, M_PI, M_PI));
  RESULT(fprintf (stderr, "<%4f><%.4f><%%><%4.4f>\n", M_PI, M_PI, M_PI));

  RESULT(checkit (stderr, "<%*f><%.*f><%%><%*.*f>\n", 3, M_PI, 3, M_PI, 3, 3, M_PI));
  RESULT(fprintf (stderr, "<%*f><%.*f><%%><%*.*f>\n", 3, M_PI, 3, M_PI, 3, 3, M_PI));

  RESULT(checkit (stderr, "<%d><%i><%o><%u><%x><%X><%c>\n",
		  75, 75, 75, 75, 75, 75, 75));
  RESULT(fprintf (stderr, "<%d><%i><%o><%u><%x><%X><%c>\n",
		 75, 75, 75, 75, 75, 75, 75));

  RESULT(checkit (stderr, "<%d><%i><%o><%u><%x><%X><%c>\n",
		  75, 75, 75, 75, 75, 75, 75));
  RESULT(fprintf (stderr, "<%d><%i><%o><%u><%x><%X><%c>\n",
		 75, 75, 75, 75, 75, 75, 75));

  RESULT(checkit (stderr, "Testing (hd) short: <%d><%ld><%hd><%hd><%d>\n", 123, (long)234, 345, 123456789, 456));
  RESULT(fprintf (stderr, "Testing (hd) short: <%d><%ld><%hd><%hd><%d>\n", 123, (long)234, 345, 123456789, 456));

#if defined(__GNUC__) || defined (HAVE_LONG_LONG) || _MSC_VER >= 1500
  RESULT(checkit (stderr, "Testing (lld) long long: <%d><%lld><%d>\n", 123, 234234234234234234LL, 345));
  RESULT(fprintf (stderr, "Testing (lld) long long: <%d><%lld><%d>\n", 123, 234234234234234234LL, 345));
  RESULT(checkit (stderr, "Testing (Ld) long long: <%d><%Ld><%d>\n", 123, 234234234234234234LL, 345));
  RESULT(fprintf (stderr, "Testing (Ld) long long: <%d><%Ld><%d>\n", 123, 234234234234234234LL, 345));
#endif

#if defined(__GNUC__) || defined (HAVE_LONG_DOUBLE) || _MSC_VER >= 1500
  RESULT(checkit (stderr, "Testing (Lf) long double: <%.20f><%.20Lf><%0+#.20f>\n",
		  1.23456, 1.234567890123456789L, 1.23456));
  RESULT(fprintf (stderr, "Testing (Lf) long double: <%.20f><%.20Lf><%0+#.20f>\n",
		 1.23456, 1.234567890123456789L, 1.23456));
#endif

  fprintf(stderr, "\ncheck file array:\n" );

  {

  char* buffer = "hello word!";

  RESULT(checkit (stderr, "Testing (file) long double: <%w%c%c%w>\n", 5, buffer));
  RESULT(checkit (stderr, "Testing (file) long double: <%w%c%c%w>\n", strlen(buffer), buffer));

  RESULT(checkit (stderr, "Testing (file) long double: <%w%c[%%%c%%]%w>\n", 5, buffer));
  RESULT(checkit (stderr, "Testing (file) long double: <%w%c[%%%c%%]%w>\n", strlen(buffer), buffer));

  RESULT(checkit (stderr, "Testing (file) long double: <%w%c[%*.02x]%w>\n", 5, buffer));
  RESULT(checkit (stderr, "Testing (file) long double: <%w%c[%02x]%w>\n", strlen(buffer), buffer));

  }

   fprintf(stderr, "\ncheck string array:\n" );

  {
	char buffer[1024] = {0};
	char* array = "hello world!";

	RESULT(checkstr ( stderr, buffer, 5, "Testing (string) long double: <%w%c[%%%c%%]%w>\n", 5, array));
	RESULT(checkstr ( stderr, buffer, 5, "Testing (string) long double: <%w%c[%%%c%%]%w>\n", strlen(array), array));
	RESULT(checkstr ( stderr, buffer, 1024,"Testing (string) long double: <%w%c[%*.02x]%w>\n", 5, array));
	RESULT(checkstr ( stderr, buffer, 1024,"Testing (string) long double: <%w%c[%-4x]%w>\n", 5, array));
	RESULT(checkstr ( stderr, buffer, 1024,"Testing (string) long double: <%w%c[%02x]%w>\n", strlen(array), array));

	array = NULL;
  }


   fprintf( stderr, "\ncheck time normal:\n" );
   {
	   char buffer[1024];
	   int index = 0;
	   timer_start();
	  
	   while ( index++ < 1000000 )
	   {
#if defined(_MSC_VER) && _MSC_VER >= 1500
		   _snprintf_s(buffer, 1024, _TRUNCATE, "<%d><%08d><%f>\n", 10000, -1, 12345678.12345678);
#elif defined(_MSC_VER)
		   _snprintf(buffer, 1024,  "<%d><%08d><%f>\n", 10000, -1, 12345678.12345678);
#else
		   snprintf(buffer, 1024, "<%d><%08d><%f>\n", 10000, -1, 12345678.12345678);
#endif
	   }

	   timer_end();
   }

   fprintf( stderr, "\ncheck time array normal:\n" );
   {
	   char buffer[1024];
	   int index = 0;
	   timer_start();

	   while ( index++ < 1000000 )
	   {
		   snprintfplus(buffer, 1024, "<%d><%08d><%f>\n", 10000, -1, 12345678.12345678 );
	   }

	   timer_end();
   }

   fprintf( stderr, "\ncheck time array:\n" );
   {
	   char buffer[1024];
	   int index = 0;
	   timer_start();

	   while ( index++ < 1000000 )
	   {
		   snprintfplus(buffer, 1024, "<%w%c[%-2x]%w>\n", 10, "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x10\x11" );
	   }

	   timer_end();
   }

  fprintf( stderr, "sizeof(long) : %d\n", sizeof(long) );
#if defined(__GNUC__) || defined (HAVE_LONG_DOUBLE) || _MSC_VER >= 1500
  fprintf( stderr, "sizeof(long long) : %d\n", sizeof(long long) );
#endif
  fprintf( stderr, "sizeof(double) : %d\n", sizeof(double) );
#if defined(__GNUC__) || defined (HAVE_LONG_DOUBLE) || _MSC_VER >= 1500
  fprintf( stderr, "sizeof(long double) : %d\n", sizeof(long double) );
#endif

  return 0;
}

Test test_printplus("printplus", printplus_main, 1);

#endif /* TEST */