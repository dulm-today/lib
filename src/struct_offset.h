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

int st_pack_set(int n);
void st_pack_reset();
int st_pack_get();
int st_pack_default_get();
int st_type_size_get(enum st_type type);
int st_type_align_get(enum st_type type);


typedef struct{
	int	pack;
	int offset;
	int size;
}st_calc, *pst_calc;

void st_calc_begin(pst_calc calc);

// return the offset of the type
int st_calc_type(pst_calc calc, enum st_type type);

// return the offset of the child struct
int st_calc_child(pst_calc calc, pst_calc child);

// return the size of the struct
int	st_calc_end(pst_calc calc);

#ifdef __cplusplus
}
#endif

#endif // __STRUCT_OFFSET_H__
