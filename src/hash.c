#include "hash.h"


// BKDR Hash 
unsigned int BKDRHash(const void *p, size_t size)
{
	unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
	unsigned int hash = 0;
	const char* str = (const char*)p;
   	const char* end = str + size;
	while (str < end)
	{
		hash = hash * seed + (*str++);
	}
 
	return (hash & 0x7FFFFFFF);
}

// AP Hash 
unsigned int APHash(const void *p, size_t size)
{
	unsigned int hash = 0;
	const char* str = (const char*)p;
	size_t i;

	for (i=0; i < size; i++)
	{
		if ((i & 1) == 0)
		{
			hash ^= ((hash << 7) ^ (*str++) ^ (hash >> 3));
		}
		else
		{
			hash ^= (~((hash << 11) ^ (*str++) ^ (hash >> 5)));
		}
	}
 
	return (hash & 0x7FFFFFFF);
}


 
// DJB Hash 
unsigned int DJBHash(const void *p, size_t size)
{
	unsigned int hash = 5381;
	const char* str = (const char*)p;
    const char* end = str + size;
	
	while (str < end)
	{
		hash += (hash << 5) + (*str++);
	}
 
	return (hash & 0x7FFFFFFF);
}

// JS Hash 
unsigned int JSHash(const void *p, size_t size)
{
	unsigned int hash = 1315423911;
	const char* str = (const char*)p;
    const char* end = str + size;
	while (str < end)
	{
		hash ^= ((hash << 5) + (*str++) + (hash >> 2));
	}
 
	return (hash & 0x7FFFFFFF);
}

// RS Hash 
unsigned int RSHash(const void *p, size_t size)
{
	unsigned int b = 378551;
	unsigned int a = 63689;
	unsigned int hash = 0;
	const char* str = (const char*)p;
    const char* end = str + size;
	while (str < end)
	{
		hash = hash * a + (*str++);
		a *= b;
	}
 
	return (hash & 0x7FFFFFFF);
}

unsigned int SDBMHash(const void *p, size_t size)
{
	unsigned int hash = 0;
	const char* str = (const char*)p;
    const char* end = str + size;
	
	while (str < end)
	{
		// equivalent to: hash = 65599*hash + (*str++);
		hash = (*str++) + (hash << 6) + (hash << 16) - hash;
	}
 
	return (hash & 0x7FFFFFFF);
}

// P. J. Weinberger Hash 
unsigned int PJWHash(const void *p, size_t size)
{
	unsigned int BitsInUnignedInt = (unsigned int)(sizeof(unsigned int) * 8);
	unsigned int ThreeQuarters	= (unsigned int)((BitsInUnignedInt  * 3) / 4);
	unsigned int OneEighth = (unsigned int)(BitsInUnignedInt / 8);
	unsigned int HighBits = (unsigned int)(0xFFFFFFFF) << (BitsInUnignedInt - OneEighth);
	unsigned int hash	= 0;
	unsigned int test	= 0;
	const char* str = (const char*)p;
    const char* end = str + size;
	
	while (str < end)
	{
		hash = (hash << OneEighth) + (*str++);
		if ((test = hash & HighBits) != 0)
		{
			hash = ((hash ^ (test >> ThreeQuarters)) & (~HighBits));
		}
	}
 
	return (hash & 0x7FFFFFFF);
}

// ELF Hash 
unsigned int ELFHash(const void *p, size_t size)
{
	unsigned int hash = 0;
	unsigned int x	= 0;
	const char* str = (const char*)p;
    const char* end = str + size;
	
	while (str < end)
	{
		hash = (hash << 4) + (*str++);
		if ((x = hash & 0xF0000000L) != 0)
		{
			hash ^= (x >> 24);
			hash &= ~x;
		}
	}
 
	return (hash & 0x7FFFFFFF);
}

