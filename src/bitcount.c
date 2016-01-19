#include "bitcount.h"

#ifdef BITCOUNT_TABLE
/* maybe slower, because of the cache line change */
static int bits_in_char [256] = {             
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4, 2,
    3, 3, 4, 3, 4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3,
    3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3,
    4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4,
    3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5,
    6, 6, 7, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4,
    4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5,
    6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 2, 3, 3, 4, 3, 4, 4, 5,
    3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 3,
    4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 4, 5, 5, 6, 5, 6, 6, 7, 5, 6,
	6, 7, 6, 7, 7, 8
};

int bitcount8(unsigned char n)
{
	return bits_in_char[(unsigned char)n];
}

int bitcount16(unsigned short n)
{
	int count;
	unsigned char* p = (unsigned char*)&n;

	count = bits_in_char[*p++];
	count += bits_in_char[*p];
	return count;
}

int bitcount32(unsigned int n)
{
	int count;
	unsigned char* p = (unsigned char*)&n;

	count = bits_in_char[*p++];
	count += bits_in_char[*p++];
	count += bits_in_char[*p++];
	count += bits_in_char[*p++];
	return count;
}
#else
int bitcount8(unsigned char n)
{
	unsigned char const MASK1 = 0x55;
	unsigned char const MASK2 = 0x33;
	unsigned char const MASK4 = 0x0f;

	n = (n & MASK1) + (n>>1 & MASK1);
	n = (n & MASK2) + (n>>2 & MASK2);
	n = (n & MASK4) + (n>>4 & MASK4);
	
	return n;
}

int bitcount16(unsigned short n)
{
	unsigned short const MASK1 = 0x5555;
	unsigned short const MASK2 = 0x3333;
	unsigned short const MASK4 = 0x0f0f;
	unsigned short const MASK8 = 0x00ff;

	n = (n & MASK1) + (n>>1 & MASK1);
	n = (n & MASK2) + (n>>2 & MASK2);
	n = (n & MASK4) + (n>>4 & MASK4);
	n = (n & MASK8) + (n>>8 & MASK8);
	
	return n;
}

int bitcount32(unsigned int n)
{
	unsigned int const MASK1 = 0x55555555;
	unsigned int const MASK2 = 0x33333333;
	unsigned int const MASK4 = 0x0f0f0f0f;
	unsigned int const MASK8 = 0x00ff00ff;
	unsigned int const MASK16 = 0x0000ffff;

	n = (n & MASK1) + (n>>1 & MASK1);
	n = (n & MASK2) + (n>>2 & MASK2);
	n = (n & MASK4) + (n>>4 & MASK4);
	n = (n & MASK8) + (n>>8 & MASK8);
	n = (n & MASK16) + (n>>16 & MASK16);
	
	return n;
}

#endif // BITCOUNT_TABLE
