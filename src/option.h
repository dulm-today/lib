#ifndef __OPTION_H__
#define __OPTION_H__

#include "log.h"

#ifdef __cplusplus
extern "C" {
#endif


enum OP_TYPE
{
	OP_INT = 1,	// int
	OP_FLOAT,	// double
	OP_BOOL,	// int
	OP_STRING,	// char*
	OP_COMBO	// int
};


typedef struct
{
	int			value;
	const char* name;
}op_combo_t;

typedef struct
{
	size_t		select;
	size_t		size;
	op_combo_t*	combos;
}op_combo_default_t;

typedef struct
{
	const char* 	name;
	enum OP_TYPE	type;
	void*			pvalue;
	const void*		pdefault;
}op_reg_t;

typedef void* op_handle;

/* init */
op_handle option_init(const char* file, op_reg_t* reg, size_t size, log_cb_v cb);

/* release */
void option_release(op_handle handle);


/* flush the option data to file */
int option_flush(op_handle handle);

/* set option(int) */
int option_set_int(int* op, int value);

/* set option(double) */
double option_set_float(double* op, double value);

/* set option(string)*/
char* option_set_string(char** op, const char* value, int len);

#ifdef __cplusplus
}
#endif

#endif // __OPTION_H__
