#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "option.h"

#ifdef __cplusplus
extern "C"{
#endif


typedef struct{
	int			init;
	size_t		size;
	op_reg_t*	reg;
	log_cb_v	log;
	char		file[];
}op_handle_t;


static int op_reg_cmp(const void* left, const void* right)
{
	return strcmp( ((op_reg_t*)left)->name, ((op_reg_t*)right)->name);
}

static char* option_fill(char* p, const char* value, int len)
{
	if (p)
		free(p);

	p = (char*)malloc(len + sizeof(char)*2);
	if (!p)
		return p;

	memcpy(p, value, len);
	p[len] = '\0';

	return p;
}

static int option_search(op_handle_t* op_h, const char* name_start, const char* name_end)
{
#if 0
	int iret = 0;
	int left = 0;
	int right = op_h->size;
	int middle = 0;

	while (left < right)
	{
		middle = (left + right) / 2;

		iret = strncmp(op_h->reg[middle].name, name_start, name_end - name_start);
		
		if (iret < 0){
			left = middle + 1;
		}
		else if (iret > 0){
			right = middle;
		}
		else{
			return middle;
		}
	}

	return -1;
#else
	int iret = 0;
	int index = 0;
	int size = op_h->size;
	
	for (;index < size; ++index)
	{
		if (!op_h->reg[index].name)
			continue;
		
		iret = strncmp(op_h->reg[index].name, name_start, name_end - name_start);

		if (0 == iret)
			return index;
	}

	return -1;
#endif
}

static int option_parse_bool(const char* str)
{
	const char* t = "true";

	while(*str && isspace(*str))
		++str;

	if (!*str || !isalnum(*str))
		return 0;

	if (isdigit(*str))
		return (atoi(str) ? 1 : 0);
	
	while(*t && *str){
		if (*t++ != tolower(*str++))
			return 0;
	}

	if (!*t)
		return 1;

	return 0;
}

static int option_parse_combo(op_handle_t* op_h, op_reg_t* op, const char* str, size_t len)
{
	size_t index;
	op_combo_default_t* combo_default = (op_combo_default_t*)op->pdefault;
	op_combo_t*	combo;

	if (isdigit(*str))
		return atoi(str);
	
	for (index = 0; index < combo_default->size; ++index)
	{
		combo = &combo_default->combos[index];

		if (0 == strncmp(combo->name, str, len))
			break;
	}

	if (index >= combo_default->size){
		LOG_CB_V(op_h->log, LOG_WARN, "combo %.*s not match", len, str);
		return -1;
	}

	return combo->value;
}

static int option_parse(op_handle_t* op_h, size_t index, const char* value_start, size_t len)
{
	op_reg_t* 	op;
	op_combo_default_t* combo_default;
	char buffer[128];

	op = &op_h->reg[index];
	buffer[sizeof(buffer)-1] = '\0';
	
	switch(op->type)
	{
		case OP_INT:
			strncpy(buffer, value_start, MAX(len, sizeof(buffer)-1));
			*(int*)op->pvalue = atoi(buffer);
			break;
		case OP_FLOAT:
			strncpy(buffer, value_start, MAX(len, sizeof(buffer)-1));
			*(double*)op->pvalue = atof(buffer);
			break;
		case OP_BOOL:
			strncpy(buffer, value_start, MAX(len, sizeof(buffer)-1));
			*(int*)op->pvalue = option_parse_bool(buffer);
			break;
		case OP_STRING:
			*((char**)op->pvalue) = option_fill(*(char**)op->pvalue, value_start, len);
			break;
		case OP_COMBO:
			combo_default = (op_combo_default_t*)op->pdefault;
			*(int*)op->pvalue = option_parse_combo(op_h, op, value_start, len);
			break;
		default:
			LOG_CB_V(op_h->log, LOG_WARN, "%s.type[%d] unknow", op->name, op->type);
			break;
	}
	return 0;
}

static const char* option_string(op_handle_t* op_h, size_t index, char* buffer)
{
	op_reg_t* 			op = &op_h->reg[index];
	op_combo_default_t* combo_default;
	op_combo_t*			combo;
	int					select;
	const char*			p = buffer;
	
	buffer[0] = '\0';
	
	switch(op->type)
	{
		case OP_INT:
			sprintf(buffer, "%d", *(int*)op->pvalue);
			break;
		case OP_FLOAT:
			sprintf(buffer, "%f", *(double*)op->pvalue);
			break;
		case OP_BOOL:
			sprintf(buffer, "%s", *(int*)op->pvalue ? "true" : "false");
			break;
		case OP_STRING:
			p = *(char**)op->pvalue;
			break;
		case OP_COMBO:
			select = *(int*)op->pvalue;
			combo_default = (op_combo_default_t*)op->pdefault;

			p = "no_select";
			for (index = 0; index < combo_default->size; ++index){
				combo = &combo_default->combos[index];
				if (combo->value == select){
					p = combo->name;
					break;
				}
			}
			break;
		default:
			LOG_CB_V(op_h->log, LOG_WARN, "%s.type[%d] unknow", op->name, op->type);
			p = "unknow_type";
			break;
	}

	return p;
}

static int option_line(op_handle_t* op_h, char* buffer)
{
	int index = -1;
	char* name_start = NULL;
	char* name_end = NULL;
	char* value_start = NULL;
	char* value_end = NULL;
	
	int len = strlen(buffer);
	char* ptr = buffer;
	char* pend = ptr + len;
	
	while (ptr < pend && *ptr)
	{
		switch(*ptr)
		{
			case '#':
				// comment
				if (NULL == value_end && NULL != value_start)
					value_end = ptr;
				else if (NULL == name_end && NULL != name_start)
					name_end = ptr;
				
				ptr = pend;
				break;
			case '=':
				if (NULL == name_start)
					ptr = pend;	// syntax error
				else if (NULL == name_end)
					name_end = ptr;
				break;
			case ' ':
			case '\t':
			case '\n':
				if (NULL == name_end && NULL != name_start)
					name_end = ptr;
				else if (NULL == value_end && NULL != value_start)
					value_end = ptr;

				if (*ptr == '\n')
					ptr = pend;
				break;
			default:
				if (NULL == name_start)
					name_start = ptr;
				else if (NULL != name_end && NULL == value_start)
					value_start = ptr;
				break;
		}
		ptr++;
	}

	if (NULL == value_start)
		// no assignment, return
		return 0;

	if (NULL == value_end)
		 value_end = pend;

	index = option_search(op_h, name_start, name_end);
	if (index < 0)
	{
		LOG_CB_V(op_h->log, LOG_ERR, "option_search [%.*s] fail", name_end - name_start, name_start );
		return -1;
	}

	return option_parse(op_h, index, value_start, value_end-value_start);
}

static int option_read(op_handle_t* op_h)
{
	FILE* fd = NULL;
	char* file = op_h->file;
	char  buffer[512] = {0};

	fd = fopen(file, "r+");
	if (fd == NULL)
	{
		LOG_CB_V (op_h->log, LOG_ERR, "fopen [%s] fail", file );
		return -1;
	}

	while (!feof(fd))
	{
		fgets(buffer, 512, fd);

		option_line(op_h, buffer);
		buffer[0] = '\0';
	}

	fclose(fd);
	return 0;
}

static int option_write(op_handle_t* op_h)
{
	int index = 0;
	int size = op_h->size;
	char* file = op_h->file;
	op_reg_t* op = NULL;
	char buffer[128];
	
	FILE* fd = NULL;

	fd = fopen(file, "w");
	if (fd == NULL)
	{
		LOG_CB_V(op_h->log, LOG_ERR, "fopen [%s] > error", file);

		return -1;
	}

	for (;index < size; ++index)
	{
		op = &op_h->reg[index];

		fprintf(fd, "%s = %s\n", op->name, option_string(op_h, index, buffer));
	}

	fclose(fd);
	
	return 0;
}

static void option_print(op_handle_t* op_h)
{
	int index = 0;
	int size = op_h->size;
	op_reg_t* op = NULL;
	char buffer[128];

	for (;index < size; ++index)
	{
		op = &op_h->reg[index];

		LOG_CB_V(op_h->log, LOG_DEBUG, "%s = %s", op->name, option_string(op_h, index, buffer));
	}
}

static void option_default(op_handle_t* op_h)
{
	size_t		index;
	op_reg_t* 	op;
	const char* str;
	op_combo_default_t* combo_default;
	
	for (index = 0;index < op_h->size; ++index)
	{
		op = &op_h->reg[index];

		switch(op->type)
		{
			case OP_INT:
				*(int*)op->pvalue = (NULL != op->pdefault ? atoi((const char*)op->pdefault) : 0);
				break;
			case OP_FLOAT:
				*(double*)op->pvalue = (NULL != op->pdefault ? atof((const char*)op->pdefault) : 0.0);
				break;
			case OP_BOOL:
				*(int*)op->pvalue = (NULL != op->pdefault ? option_parse_bool((const char*)op->pdefault) : 0);
				break;
			case OP_STRING:
				str = (char*)op->pdefault;
				if (str)
					*(char**)op->pvalue = option_fill(NULL, str, strlen(str));
				else
					*(char**)op->pvalue = NULL;
				break;
			case OP_COMBO:
				combo_default = (op_combo_default_t*)op->pdefault;
				*(int*)op->pvalue = (NULL != combo_default ? combo_default->select : 0);
				break;
			default:
				LOG_CB_V(op_h->log, LOG_WARN, "%s.type[%d] unknow", op->name, op->type);
				break;
		}
	}
}

/* init */
op_handle option_init(const char* file, op_reg_t* reg, size_t size, log_cb_v cb)
{
	op_handle_t* op_h = NULL;

	if (NULL == file || NULL == reg || 0 == size)
		return NULL;

	op_h = (op_handle_t*)malloc(sizeof(op_handle_t) + strlen(file) + 1);
	if (NULL == op_h){
		LOG_CB_V(cb, LOG_ERR, "malloc fail");
		return NULL;
	}

	op_h->init = 0;
	op_h->reg = reg;
	op_h->log = cb;
	op_h->size = size;
	strcpy(op_h->file, file);

	option_default(op_h);

#if 0
	// ∂‘op_reg≈≈–Ú
	qsort(reg, size, sizeof(op_reg_t), op_reg_cmp);
#endif

	if (0 != option_read(op_h))
	{	
		option_write(op_h);
	}

	option_print(op_h);
	
	return op_h;
}

void option_release(op_handle handle)
{
	op_handle_t* op_h = (op_handle_t*)handle;

	if (op_h){
		option_flush(handle);
		free(op_h);
	}
}


/* flush the option data to file */
int	option_flush(op_handle handle)
{
	op_handle_t* op_h = (op_handle_t*)handle;

	if (op_h)
		return option_write(op_h);

	return -1;
}


/* set option(int) */
int	option_set_int(int* op, int value)
{
	*op = value;

	return value;
}

/* set option(double) */
double option_set_float(double* op, double value)
{
	*op = value;

	return value;
}

/* set option(string)*/
char* option_set_string(char** op, const char* value, int len)
{
	*op = option_fill(*op, value, len);

	return *op;
}

#ifdef __cplusplus
}
#endif
