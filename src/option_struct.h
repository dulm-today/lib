#ifndef __OPTION_STRUCT_H__
#define __OPTION_STRUCT_H__

enum OP2_TYPE
{
	OP2_I8	= 1,
	OP2_U8,
	OP2_I16,
	OP2_U16,
	OP2_I32,
	OP2_U32,
	OP2_INT,
	OP2_I64,
	OP2_U64,
	OP2_FLOAT,
	OP2_DOUBLE,
	OP2_COMBO,			// int
	OP2_ST,

	OP2_ARRAY = 0x010000,
};

// combo
typedef struct
{
	int			value;
	const char* name;
}op2_combo_t;

typedef struct
{
	size_t			select;
	size_t			size;
	op2_combo_t* 	combos;
}op2_combo_default_t;


// array


// option data output:  object ->  value_type   ->  format_type   ->  file
//             input :  file   ->  format_type  ->  value_type    ->  object

typedef struct
{
	const char* 	name;			// object name
	int				value_type;		// the type of pvalue
	OP2_TYPE		fmt_type;		// the type for output and input.
	void*			pvalue;			// object address
	void*			pvalue_desc; 	// 
	void*			pfmt_desc;		// 
	void*			pdefault;		// default value
}op2_reg_t;


typedef struct op2_blk
{
	const char* 	name;		// if null, no blk_name

	int				num;
	void*			reg_or_blk;		
}op2_blk_t;



typedef struct op2_reg_attr
{
	int		case;
	
}op2_reg_attr_t;

op2_reg_t reg[] = {
	{}
};

op2_blk_t blk_child[] = {
	{"b", sizeof(reg)/sizeof(reg[0]), &reg},
};

op2_blk_t blk[] = {
	{NULL, sizeof(reg)/sizeof(reg[0]), &reg},
	{"a", 1, blk_child},
};



struct cc {
	float c[10];
};

struct bb {
	int  	a;
	int 	b;

	struct cc c;
};

int 		aa = 0;
struct 	bb	b1;

op2_reg_t reg_b1[] = {
		{"a", OP2_INT, 0, NULL},
		{"b", OP2_INT, 4, NULL},
		{"c", OP2_ST,  8, NULL},
};

op2_reg_t father[] = 
{
	{"aa", OP2_I32, &aa, NULL},
	{"bb", OP2_ST,  &b1, reg_b1},
};


#endif //__OPTION_STRUCT_H__