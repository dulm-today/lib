#include "struct_offset.h"
#include <stddef.h>
#include <bitcount.h>

typedef long long longlong;
typedef long double longdouble;
typedef void* ptr;

#define MAX(x,y)	((x) >= (y) ? (x) : (y))
#define MIN(x,y)	((x) <= (y) ? (x) : (y))

#define ALIGN(x, a)	(((x)+(a)-1) & ~((a)-1))

#define ST_TYPE_SIZE_INIT(type)	st_type_size[st_##type] = sizeof(type)
#define ST_TYPE_ALIGN_INIT(type)	\
	do{\
		struct _align_##type{\
			char 	a;\
			type	b;\
		};\
		st_type_align[st_##type] = (size_t)(&((struct _align_##type*)0)->b);\
	}while(0)
	

static char st_inited = 0;

/* default pragma pack value, the max of the st_type_offset */
static unsigned char st_pack = 0;
static unsigned char st_pack_default = 0;

static unsigned char st_type_size[st_type_end] = {0xff};
static unsigned char st_type_align[st_type_end] = {0xff};

void st_init()
{
	int i;
	
	if (st_inited)
		return;

	// size init
	ST_TYPE_SIZE_INIT(char);
	ST_TYPE_SIZE_INIT(short);
	ST_TYPE_SIZE_INIT(int);
	ST_TYPE_SIZE_INIT(long);
	ST_TYPE_SIZE_INIT(longlong);
	ST_TYPE_SIZE_INIT(bool);
	ST_TYPE_SIZE_INIT(float);
	ST_TYPE_SIZE_INIT(double);
	ST_TYPE_SIZE_INIT(longdouble);
	ST_TYPE_SIZE_INIT(ptr);
	st_type_size[st_int32] = 4;
	st_type_size[st_int64] = 8;

	// offset init
	ST_TYPE_ALIGN_INIT(char);
	ST_TYPE_ALIGN_INIT(short);
	ST_TYPE_ALIGN_INIT(int);
	ST_TYPE_ALIGN_INIT(long);
	ST_TYPE_ALIGN_INIT(longlong);
	ST_TYPE_ALIGN_INIT(bool);
	ST_TYPE_ALIGN_INIT(float);
	ST_TYPE_ALIGN_INIT(double);
	ST_TYPE_ALIGN_INIT(longdouble);
	ST_TYPE_ALIGN_INIT(ptr);
	for (i = st_int; i <= st_longlong; ++i){
		if (4 == st_type_size[i])
			st_type_align[st_int32] = st_type_align[i];
		if (8 == st_type_size[i])
			st_type_align[st_int64] = st_type_align[i]; 
	}

	for (i = 0; i < st_type_end; ++i){
		st_pack_default = MAX(st_pack_default, st_type_align[i]);
	}
	st_pack = st_pack_default;

	st_inited = 1;
}

int st_pack_set(int n)
{
	int nn = n;
	st_init();
	
	if (n > 0){
		if (1 == bitcount32(n))
			st_pack = n;
	}

	return st_pack;
}

void st_pack_reset()
{
	st_init();
	st_pack = st_pack_default;
}

int st_pack_get()
{
	st_init();

	return st_pack;
}

int st_pack_default_get()
{
	st_init();

	return st_pack_default;
}

int st_type_size_get(enum st_type type)
{
	st_init();

	if (type >= st_type_start && type < st_type_end)
		return st_type_size[type];
	return -1;
}

int st_type_align_get(enum st_type type)
{
	st_init();
	if (type >= st_type_start && type < st_type_end)
		return st_type_align[type];
	return -1;
}


void st_calc_begin(pst_calc calc)
{
	st_init();
	if (calc){
		calc->pack = 0;
		calc->offset = 0;
		calc->size = 0;
	}
}

// return the offset of the type
int  st_calc_type(pst_calc calc, enum st_type type)
{
	int align, ptr;

	st_init();
	
	if (NULL == calc)
		return -1;
	
	align = st_type_align_get(type);
	if (align <= 0)
		return -1;

	align = MIN(align, st_pack);
	ptr = ALIGN(calc->offset, align);

	calc->offset = ptr + st_type_size_get(type);
	calc->pack = MAX(align, calc->pack);

	return ptr;
}

int st_calc_child(pst_calc calc, const pst_calc child)
{
	int ptr;
	
	if (NULL == calc || NULL == child 
		|| child->pack <= 0 || child->size <= 0)
		return -1;

	ptr = ALIGN(calc->offset, child->pack);
	calc->offset = ptr + child->size;
	calc->pack = MAX(calc->pack, child->pack);
	
	return ALIGN(ptr, child->pack);
}

// return the size of the struct
int	st_calc_end(pst_calc calc)
{	
	if (NULL == calc || calc->pack <= 0)
		return -1;

	calc->size = ALIGN(calc->offset, calc->pack);

	return calc->size;
}


