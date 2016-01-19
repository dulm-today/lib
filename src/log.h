#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "libconfig.h"
#include "libdef.h"
#include "printf_safe.h"

// log level
#define LOG_NONE	-1
#define LOG_DEBUG	0
#define LOG_MSG		1
#define LOG_WARN	2
#define LOG_ERR		3
#define LOG_DEBUG_ERR 	4
#define LOG_LEVEL_MAX	5


#ifndef LOG_BUFFER_SIZE
#define LOG_BUFFER_SIZE		1024
#endif


#ifndef __SHORTFILE__
#define __SHORTFILE__		_basename(__FILE__)
#endif

#define __LOG_CB(cb, level, file, line, func, msg)	\
	do{\
		log_cb __cb__ = cb;\
		if (__cb__ != NULL){\
			cb(level, file, line, func, msg);\
		}\
	}while(0)

#define __LOG_CB_V(cb, level, file, line, func, fmt, ...)	\
	do{\
		log_cb_v __cb__ = cb;\
		if (__cb__ != NULL){\
			cb(level, file, line, func, fmt, __VA_ARGS__);\
		}\
	}while(0)

#define LOG_CB(cb, l, msg)			\
			__LOG_CB(cb, l, __SHORTFILE__, __LINE__, __FUNC__, msg)
			
#define LOG_CB_V(cb, l, fmt, ...) 	\
			__LOG_CB_V(cb, l, __SHORTFILE__, __LINE__, __FUNC__, fmt, __VA_ARGS__)

#ifndef NDEBUG
#define DEBUG_CB	LOG_CB
#define DEBUG_CB_V	LOG_CB_V
#else
#define DEBUG_CB(cb, l, m)	
#define DEBUG_CB_V(cb, l, fmt, ...)
#endif

typedef void (*log_output)(const char* fmt);

// chose use c++ or c
#if defined(USE_C_LOG) || !defined(__cplusplus)
typedef void (*log_cb)(int level, const char* file, int line, 
						const char* func, const char* msg);
typedef void (*log_cb_v)(int level, const char* file, int line, 
						const char* func, const char* fmt, ...);

#define LOG_MODULE(name, module, output, filter)	\
			void name(int l, const char* file, int line, \
					const char* func, const char* msg)\
			{\
				va_list ap;\
				va_start(ap, msg);\
				_log_gen(output, l, filter, file, line, func, module, msg, ap);\
				va_end(ap);\
			}\

#define LOG_V_MODULE(name, module, output, filter)	\
			void name(int l, const char* file, int line, \
					const char* func, const char* fmt, ...)\
			{\
				va_list ap;\
				va_start(ap, fmt);\
				_log_gen(output, l, filter, file, line, func, module, fmt, ap);\
				va_end(ap);\
			}\
			
	


#else // use c++ log
class LogPointer;
typedef LogPointer log_cb;
typedef LogPointer log_cb_v;

#define LOG_MODULE(name, module, output, filter)	\
			LogPointer name(new Log(module, output, filter))

#define LOG_V_MODULE	LOG_MODULE
#endif


// C and some use in C++
#ifdef __cplusplus
extern "C" {
#endif

int	log_level();
void log_level_set(int level);

const char* log_format();
void log_format_set(const char* fmt);

void _log_gen(log_output out, int l, int filter, const char* file, int line, 
				const char* func, const char* module, const char* fmt, va_list ap);

const char* _basename(const char* file);

#ifdef UNICODE
extern void(*g_global_log_output)(const wchar_t* msg);
#else
extern void(*g_global_log_output)(const char* msg);
#endif

#ifdef __cplusplus
}
#endif


// C++
#ifdef __cplusplus

class Log
{
public:
	Log(const char* module, log_output out, int level = 0)
		: _module(NULL), 
		_output(out),
		_level(level)
	{
		if (module)
			_module = _strdup(module);
	}
	
	~Log()
	{
		if (_module)
			free(_module);
	}

	void operator()(int l, const char* file, int line, const char* func,
					const char* fmt, va_list ap)
	{
		_log_gen(_output, l, _level, file, line, func, _module, fmt, ap);
	}

	void operator()(int l, const char* file, int line, const char* func, 
					const char* fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		(*this)(l, file, line, func, fmt, ap);
		va_end(ap);
	}

private:
	char* 		_module;
	log_output  _output;
	int			_level;
};

class LogPointer
{
public:
	LogPointer(Log* log = NULL) : _log(log) {}
	LogPointer(const LogPointer& right) : _log(right._log){}

	void operator()(int level, const char* file, int line, const char* func, 
					const char* fmt, ...)
	{
		if (_log){
			va_list ap;
			va_start(ap, fmt);
			(*_log)(level, file, line, func, fmt, ap);
			va_end(ap);
		}
	}

	bool operator!()const{
		return (NULL == _log ? 1 : 0);
	}

	bool operator!=(const LogPointer& right){
		return (right._log != _log ? 1 : 0);
	}
	
private:
	Log* _log;
};

#endif



#endif //__LOG_H__
