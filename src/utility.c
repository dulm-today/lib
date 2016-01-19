#include "utility.h"

// use big end
void write_u16(void* out, unsigned short u16)
{
	unsigned char* p = (unsigned char*)out;
	*p++ = (u16 >> 8) & 0xFF;
	*p = u16 & 0xFF;
}

void write_u32(void* out, unsigned int u32)
{
	unsigned char* p = (unsigned char*)out;
	*p++ = (u32 >> 24) & 0xFF;
	*p++ = (u32 >> 16) & 0xFF;
	*p++ = (u32 >> 8) & 0xFF;
	*p = u32 & 0xFF;
}

void read_u16(void* in, unsigned short* pu16)
{
	unsigned char* p = (unsigned char*)in;

	*pu16 = *p++ << 8;
	*pu16 |= *p; 
}

void read_u32(void* in, unsigned int* pu32)
{
	unsigned char* p = (unsigned char*)in;

	*pu32 = *p++ << 24;
	*pu32 |= *p++ << 16;
	*pu32 |= *p++ << 8;
	*pu32 |= *p;
}
