#ifndef __LIBDEF_H_INCLUDE
#define __LIBDEF_H_INCLUDE


#ifdef _MSC_VER
#	define __FUNC__			__FUNCTION__
#	define __FUNCLONG__		__FUNCSIG__
#elif defined(__GNUC__)
#	define __FUNC__			__FUNCTION__
#	define __FUNCLONG__		__PRETTY_FUNCTION__
#endif

#ifndef MAX
#define MAX(x,y)	((x) >= (y) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x,y)	((x) < (y) ? (x) : (y))
#endif

#define _VALUE(x)	__VALUE(x)
#define __VALUE(x)	x

#ifdef _WIN32
#define PATH_CH		'\\'
#define PATH_STR	"\\"
#else
#define PATH_CH		'/'
#define PATH_STR	"/"
#endif

#endif // __LIBDEF_H_INCLUDE