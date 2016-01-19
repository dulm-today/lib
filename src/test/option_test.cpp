#include "option.h"
#include "log.h"
#include "test_core.h"
#include <stdio.h>
#include <stdarg.h>

int 		op_int;
double 		op_double;
int			op_bool;
char*		op_string;
int			op_combo;


op_combo_t g_combo[] = 
{
	{1, "one"},
	{2, "two"},
	{3, "three"},
	{10, "ten"},
	{0, "zero"}
};


op_combo_default_t g_combo_default = {
	1,
	sizeof(g_combo)/sizeof(g_combo[0]),
	g_combo
};

op_reg_t g_reg[] = 
{
	{"int", 	OP_INT, 	&op_int, 	"999"},
	{"double", 	OP_FLOAT, 	&op_double, "0.99"},
	{"bool",	OP_BOOL,	&op_bool, 	"true"},
	{"string",	OP_STRING,	&op_string, "123456789"},
	{"combo",	OP_COMBO,	&op_combo,	&g_combo_default}
};

static LOG_V_MODULE(test_option_log_cb_v, "option_test", g_global_log_output, 0);

static int test_option_cb()
{
	op_handle op = option_init("option.txt", g_reg, sizeof(g_reg)/sizeof(g_reg[0]), test_option_log_cb_v);

	if (op == NULL){
		fprintf(stderr, "option_init fail\n");
		return 1;
	}
	
#if 0
	option_set_int(&op_int, 5);
	option_set_int(&op_bool, 0);
	option_set_int(&op_combo, 2);
	option_set_float(&op_double, 1.111);
	option_set_string(&op_string, "dulm", 4);
#endif

	option_release(op);

	return 0;
}

Test test_option("option", test_option_cb, 1);