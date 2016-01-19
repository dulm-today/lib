#include <stdio.h>
#include "test_core.h"
#include "bitcount.h"
#include "stdlib.h"

int _bitcount(unsigned int n){
	int count = 0;

	while(n){
		count += n & 0x01;
		n >>= 1;
	}

	return count;
}

#define BITCOUNT_CHECK(func, n, c1, c2)	\
	do{\
		fprintf(stderr, "%s: %8d  bits  %d-%d  [%s]\n",\
				#func, n, c1, c2, c1 == c2 ? "OK" : "ERROR");\
		error += c1 == c2 ? 0 : 1;\
	}while(0)

int bitcount8_test()
{
	int error = 0;
	int c1, c2;
	
	for (int i = 0; i < 256; ++i){
		c1 = bitcount8(i);
		c2 = _bitcount(i);
		BITCOUNT_CHECK(bitcount8, i, c1, c2);
	}
	return error;
}

int bitcount16_test()
{
	int error = 0;
	int c1, c2;
	
	for (int i = 0; i < 256; ++i){
		unsigned short n = (rand() << 8) | (rand() & 0x00ff);
		c1 = bitcount16(n);
		c2 = _bitcount(n);
		BITCOUNT_CHECK(bitcount16, n, c1, c2);
	}
	return error;
}

int bitcount32_test()
{
	int error = 0;
	int c1, c2;
	
	for (int i = 0; i < 256; ++i){
		unsigned int n = (rand() << 24) 
					   | ((rand()& 0x00ff) << 16 )
					   | ((rand()& 0x00ff) << 8 )
					   | (rand()& 0x00ff);
		c1 = bitcount32(n);
		c2 = _bitcount(n);
		BITCOUNT_CHECK(bitcount32, n, c1, c2);
	}
	return error;
}

int bitcount_test()
{
	int error = 0;

	if (bitcount8_test())
		error++;

	if (bitcount16_test())
		error++;

	if (bitcount32_test())
		error++;

	return 0;
}

Test bitcount_item("bitcount", bitcount_test, 3);
