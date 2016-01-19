#include "test_core.h"
#include "stdio.h"
#include "struct_offset.h"

#define ST_SIZE_SHOW(type)	\
	fprintf(stderr, "sizeof %-16s = %d\n", #type, st_type_size_get(type))
	
#define ST_ALIGN_SHOW(type)	\
	fprintf(stderr, "offset %-16s = %d\n", #type, st_type_align_get(type))

int struct_offset_show()
{
	fprintf(stderr, "pack = %d\n", st_pack_get());

	ST_SIZE_SHOW(st_char);
	ST_SIZE_SHOW(st_short);
	ST_SIZE_SHOW(st_int);
	ST_SIZE_SHOW(st_long);
	ST_SIZE_SHOW(st_longlong);
	ST_SIZE_SHOW(st_bool);
	ST_SIZE_SHOW(st_float);
	ST_SIZE_SHOW(st_double);
	ST_SIZE_SHOW(st_longdouble);

	ST_ALIGN_SHOW(st_char);
	ST_ALIGN_SHOW(st_short);
	ST_ALIGN_SHOW(st_int);
	ST_ALIGN_SHOW(st_long);
	ST_ALIGN_SHOW(st_longlong);
	ST_ALIGN_SHOW(st_bool);
	ST_ALIGN_SHOW(st_float);
	ST_ALIGN_SHOW(st_double);
	ST_ALIGN_SHOW(st_longdouble);
	
	return 0;
}

#define ST_CALC_CHECK(calc, type, member, mem_type)	\
	do{\
		int __ptr1 = (int)&((type*)0)->member;\
		int __ptr2 = st_calc_type(calc, mem_type);\
		fprintf(stderr, "%s: %s.%-16s  %d-%d  [%s]\n", \
				__FUNCTION__, #type, #member,\
				__ptr1, __ptr2, __ptr1 == __ptr2 ? "OK" : "ERROR");\
		error += __ptr2 == __ptr2 ? 0 : 1;\
	}while(0)

#define ST_CALC_SIZE(calc, type)	\
	do{\
		int __size1 = sizeof(type);\
		int __size2 = (calc)->size;\
		fprintf(stderr, "%s: %s pack %d sizeof %d-%d  [%s]\n", \
			__FUNCTION__, #type, (calc)->pack, __size1, __size2,\
			__size1 == __size2 ? "OK" : "ERROR");\
		error += __size1 == __size2 ? 0 : 1;\
	}while(0)

#define ST_CALC_CHILD(calc, type, member, calc_child)	\
	do{\
		int __ptr1 = (int)&((type*)0)->member;\
		int __ptr2 = st_calc_child(calc, calc_child);\
		fprintf(stderr, "%s: %s.%-16s  %d-%d  [%s]\n", \
				__FUNCTION__, #type, #member,\
				__ptr1, __ptr2, __ptr1 == __ptr2 ? "OK" : "ERROR");\
		error += __ptr2 == __ptr2 ? 0 : 1;\
	}while(0)

int struct_offset_calc_test()
{
	int ptr = 0;
	int size = 0;
	int error = 0;
	
	struct A{
		char 	a;
		short	b;
		int		c;
		double	d;
		char	e;
	};

	struct BA{
		char 		a;
		struct 	A	aa;
		char		b;
		void*		p;
	};

	st_calc calcA;
	st_calc calcBA;

	st_calc_begin(&calcA);
	ST_CALC_CHECK(&calcA, struct A, a, st_char);
	ST_CALC_CHECK(&calcA, struct A, b, st_short);
	ST_CALC_CHECK(&calcA, struct A, c, st_int);
	ST_CALC_CHECK(&calcA, struct A, d, st_double);
	ST_CALC_CHECK(&calcA, struct A, e, st_char);
	size = st_calc_end(&calcA);
	ST_CALC_SIZE(&calcA, struct A);

	st_calc_begin(&calcBA);
	ST_CALC_CHECK(&calcBA, struct BA, a, st_char);
	ST_CALC_CHILD(&calcBA, struct BA, aa, &calcA);
	ST_CALC_CHECK(&calcBA, struct BA, b, st_char);
	ST_CALC_CHECK(&calcBA, struct BA, p, st_ptr);
	size = st_calc_end(&calcBA);
	ST_CALC_SIZE(&calcBA, struct BA);

	return error;
}

int struct_offset_calc_test2()
{
	int ptr = 0;
	int size = 0;
	int error = 0;
	
#pragma pack(push)	
#pragma pack(3)	
	struct A{
		char 	a;
		short	b;
		int		c;
		double	d;
		char	e;
	};

	struct BA{
		char 		a;
		struct 	A	aa;
		char		b;
		void*		p;
	};
#pragma pack(pop)

	st_calc calcA;
	st_calc calcBA;

	st_pack_set(3);
	st_calc_begin(&calcA);
	ST_CALC_CHECK(&calcA, struct A, a, st_char);
	ST_CALC_CHECK(&calcA, struct A, b, st_short);
	ST_CALC_CHECK(&calcA, struct A, c, st_int);
	ST_CALC_CHECK(&calcA, struct A, d, st_double);
	ST_CALC_CHECK(&calcA, struct A, e, st_char);
	size = st_calc_end(&calcA);
	ST_CALC_SIZE(&calcA, struct A);

	st_calc_begin(&calcBA);
	ST_CALC_CHECK(&calcBA, struct BA, a, st_char);
	ST_CALC_CHILD(&calcBA, struct BA, aa, &calcA);
	ST_CALC_CHECK(&calcBA, struct BA, b, st_char);
	ST_CALC_CHECK(&calcBA, struct BA, p, st_ptr);
	size = st_calc_end(&calcBA);
	ST_CALC_SIZE(&calcBA, struct BA);
	st_pack_reset();
	
	return error;
}

int struct_offset_test()
{
	int error = 0;
	
	if (struct_offset_show())
		error++;

	if (struct_offset_calc_test())
		error++;
	
	if (struct_offset_calc_test2())
		error++;
	
	return error;
}

Test struct_offset_item("struct_offset", struct_offset_test, 3);