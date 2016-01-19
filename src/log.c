#include "log.h"
#include <ctype.h>
#include "libtime.h"
#include "vadef.h"


// log format
#define LOG_FMT_DATA		0x0001
#define LOG_FMT_TIME		0x0002
#define LOG_FMT_LEVEL		0x0004
#define LOG_FMT_FILE		0x0008
#define LOG_FMT_LINE		0x0010
#define LOG_FMT_FUNC		0x0020
#define LOG_FMT_MODULE		0x0040
#define LOG_FMT_FMT			0x1000

#define LOG_FORMAT_LENGTH		256
#define LOG_FORMAT_SIZE			16


#define LOG_FORMAT			"(%-5level%)%time% %-24module% %-32file%(%-5line%) "
#define LOG_FORMAT_TYPE		{{LOG_FMT_LEVEL, "(%-5s)", 0},\
							 {LOG_FMT_MODULE,"%-16s ", 0},\
							 {LOG_FMT_FUNC,  "%-24s | ", 0},\
							 {LOG_FMT_DATA,  ""}}

#define LOG_FORMAT_TIME		"yyyy-mm-dd HH:MM:ss SSS"
#define LOG_FORMAT_TIME_LEN	32

struct _log_fmt
{
	int	 _type;
	char _fmt[32];
	char _replace[4];
};

int  _log_level = 0;
int  _log_format_has_time = 0;
char _str_log_format[LOG_FORMAT_LENGTH] = LOG_FORMAT;
struct _log_fmt _log_format_type[LOG_FORMAT_SIZE] = LOG_FORMAT_TYPE;


struct _log_fmt _log_format_table[] = {
	{LOG_FMT_DATA,  "data", ""},
	{LOG_FMT_TIME,  "time", "s"},
	{LOG_FMT_LEVEL, "level","s"},
	{LOG_FMT_FILE,  "file", "s"},
	{LOG_FMT_LINE,  "line", "d"},
	{LOG_FMT_FUNC,  "func", "s"},
	{LOG_FMT_MODULE,"module", "s"},
};

char* _str_log_level[LOG_LEVEL_MAX+1] = {"DEBUG", "MSG", "WARN", "ERROR", "D_ERR", "?????"};


static void log_format_deal();

int	log_level()
{
	return _log_level;
}

void log_level_set(int level)
{
	_log_level = level;
}

const char* log_format()
{
	return &_str_log_format[0];
}

void log_format_set(const char* fmt)
{
	strncpy(_str_log_format, fmt, LOG_FORMAT_LENGTH);
	
	log_format_deal();
}


static int  log_filter(int l, int filter)
{
	if (filter < 0)
		return LOG_NONE;
	if (l >= LOG_LEVEL_MAX)
		return LOG_LEVEL_MAX;
	if ((l & 0x0003) >= (filter & 0x0003))
		return l & 0x0003;
	if ((l & 0x000c) >= (filter & 0x000c))
		return l & 0x000c;
	return LOG_NONE;
}

static struct _log_fmt* log_format_fmt(char* p, int len)
{
	struct _log_fmt* fmt = &_log_format_table[0];
	int i, size = sizeof(_log_format_table)/sizeof(_log_format_table[0]);
	int cmp = len >= sizeof(fmt->_fmt) ? sizeof(fmt->_fmt) : len;
	for (i = 0; i < size; ++i){
		if (0 == memcmp(fmt[i]._fmt, p, cmp))
			return &fmt[i];
	}
	return NULL;
}

static int log_format_add(int* index, int* total, const char* p, int len, int type)
{
	struct _log_fmt* fmt = &_log_format_type[0];
	int size = sizeof(fmt->_fmt)-1;
	int cp = 0;

	// check the type
	while(len > 0 && *index < LOG_FORMAT_SIZE)
	{
		if (0 == fmt[*index]._type){
			fmt[*index]._type = type;
			*total = 0;
		}
		
		if (LOG_FMT_FMT == type){
			cp = ((*total+len) > size) ? (size-*total) : len;
			if (0 == cp){
				*index++;
				continue;
			}
		}
		else if (fmt[*index]._type == type){
			// this the same type, and more than one add, always safe
			cp = len;
		}
		else {
			(*index)++;
			continue;
		}

		memcpy(&fmt[*index]._fmt[*total], p, cp);
		len -= cp;
		p += cp;
		*total += cp;

		if (len)
			*index++;
	}
	
	return len;
}

static void log_format_deal()
{
	char* p = &_str_log_format[0];
	char* start, *head = NULL, *name = NULL;
	int count = 0, total = 0;
	struct _log_fmt* fmt = &_log_format_type[0];
	struct _log_fmt* f;
	int max = sizeof(fmt->_fmt)-1;
	int type_before = 0;

	memset(fmt, 0, sizeof(_log_format_type));
	_log_format_has_time = 0;
	
	start = p;
	while(*p && count < LOG_FORMAT_SIZE){
		if (*p == '%')
			*p = '$';
		if (*p == '$'){
			if (NULL == head){
				log_format_add(&count, &total, start, p-start, LOG_FMT_FMT);
				start = p;
				head = p++;
				name = NULL;
				continue;
			}
			if (NULL == name){
				// empty format, look as fmt
				log_format_add(&count, &total, "$$", 2, LOG_FMT_FMT);
				log_format_add(&count, &total, start+1, p-start-1, LOG_FMT_FMT);
				p = start = p+1;
				head = NULL;
				continue;
			}
			f = log_format_fmt(name, p-name);
			if (NULL == f || name-start+strlen(f->_replace) > max){
				// format type not find, look as fmt
				log_format_add(&count, &total, "$$", 2, LOG_FMT_FMT);
				log_format_add(&count, &total, start+1, p-start-1, LOG_FMT_FMT);
				log_format_add(&count, &total, "$$", 2, LOG_FMT_FMT);
				p = start = p+1;
				head = NULL;
				continue;
			}
			if (f->_type == LOG_FMT_TIME){
				_log_format_has_time = 1;
			}
			if (type_before == f->_type){
				count++; total = 0;
			}
			type_before = f->_type;
			log_format_add(&count, &total, "%", 1, f->_type);
			log_format_add(&count, &total, start+1, name-start-1, f->_type);
			log_format_add(&count, &total, f->_replace, strlen(f->_replace), f->_type);

			p = start = p+1;
			head = NULL;
			continue;
		}
		else if (NULL != head && NULL == name && isalpha(*p)){
			name = p;
		}
		p++;
	}

	if (start < p && count < LOG_FORMAT_SIZE){
		log_format_add(&count, &total, start, p-start, LOG_FMT_FMT);
	}
}

static void log_format_time(char* out, int size)
{
	struct time_date time;
	
	time_date_get(&time);
	snprintf_safe(out, size, "%04d%2d%2d %2d:%2d:%2d %3d", 
			time._year, time._month, time._day,
			time._hour, time._min, time._sec, time._ms);
}

void _log_gen(log_output out, int l, int filter, const char* file, int line, 
				const char* func, const char* module, const char* fmt, va_list ap)
{
	int rc, i = 0, pos = 0;
	char buffer[LOG_BUFFER_SIZE];
	char time[LOG_FORMAT_TIME_LEN];
	int size = sizeof(buffer)-1;
	struct _log_fmt* format = &_log_format_type[0];
	va_list cp, bp;

	time[0] = 0;
	l = log_filter(l, filter);
	if (LOG_NONE == l)
		return;
	l = log_filter(l, _log_level);
	if (LOG_NONE == l)
		return;

	// copy the paramet list info
	va_copy(cp, ap);

	// time
	if (_log_format_has_time)
		log_format_time(time, LOG_FORMAT_TIME_LEN);
	
	// fmt
	for (i = 0; i < LOG_FORMAT_SIZE 
				&& format[i]._type; ++i)
	{
		switch(format[i]._type)
		{
			case LOG_FMT_DATA:
				va_copy(bp, cp);
				rc = vsnprintf_safe(&buffer[pos], size-pos, fmt, bp);
				va_end(bp);
				break;
			case LOG_FMT_TIME:
				rc = snprintf_safe(&buffer[pos], size-pos, format[i]._fmt, time);
				break;
			case LOG_FMT_LEVEL:
				rc = snprintf_safe(&buffer[pos], size-pos, format[i]._fmt, _str_log_level[l]);
				break;
			case LOG_FMT_FILE:
				rc = snprintf_safe(&buffer[pos], size-pos, format[i]._fmt, file);
				break;
			case LOG_FMT_LINE:
				rc = snprintf_safe(&buffer[pos], size-pos, format[i]._fmt, line);
				break;
			case LOG_FMT_FUNC:
				rc = snprintf_safe(&buffer[pos], size-pos, format[i]._fmt, func);
				break;
			case LOG_FMT_MODULE:
				rc = snprintf_safe(&buffer[pos], size-pos, format[i]._fmt, module);
				break;
			case LOG_FMT_FMT:
				rc = snprintf_safe(&buffer[pos], size-pos, format[i]._fmt);
				break;
			default:
				continue;
		}
		if (rc < 0){
			pos = size-1-3;
			buffer[pos++] = '.';
			buffer[pos++] = '.';
			buffer[pos++] = '.';
			break;
		}
		pos += rc;
	}
	va_end(ap);

	buffer[pos] = '\n';
	buffer[pos+1] = 0;

	out(buffer);
}

const char* _basename(const char* file)
{
	const char* _x = strrchr(file, PATH_CH);

	return (_x ? _x+1 : _x);
}	
