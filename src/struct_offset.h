#ifndef __STRUCT_OFFSET_H__
#define __STRUCT_OFFSET_H__

#ifdef __cplusplus
extern "C" {
#endif

enum st_type
{
	st_type_start = 0,
		
	st_char = 0,
	st_short,
	st_int,
	st_long,
	st_longlong,
	st_bool,
	st_float,
	st_double,
	st_longdouble,
	st_ptr,
	st_int32,
	st_int64,

	st_type_end
};

int st_pack_default_get();
int st_type_size_get(enum st_type type);
int st_type_align_get(enum st_type type);


typedef struct{
	int	pack;
	int offset;
	int size;
	int nosizearray;
}st_calc, *pst_calc;

void st_calc_begin(pst_calc calc, int pack = 0);

// return the offset of the type. if num > 1, then it is array; if num == 0, it must be the last one
int st_calc_type(pst_calc calc, enum st_type type, int num = 1);

// return the offset of the child struct
int st_calc_child(pst_calc calc, pst_calc child, int num = 1);

// return the size of the struct
int	st_calc_end(pst_calc calc);

#ifdef __cplusplus
}
#endif

#endif // __STRUCT_OFFSET_H__
