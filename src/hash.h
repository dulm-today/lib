#ifndef __HASH_H__
#define __HASH_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C"{
#endif

// BKDR Hash mul
unsigned int BKDRHash(const void *str, size_t size);

// AP Hash judge in cycle, 
unsigned int APHash(const void *str, size_t size);

// DJB Hash shift, add
unsigned int DJBHash(const void *str, size_t size);

// JS Hash 
unsigned int JSHash(const void *str, size_t size);

// RS Hash 
unsigned int RSHash(const void *str, size_t size);

// SDBM Hash
unsigned int SDBMHash(const void *str, size_t size);

// P. J. Weinberger Hash 
unsigned int PJWHash(const void *str, size_t size);

// ELF Hash 
unsigned int ELFHash(const void *str, size_t size);


// DJBHash macro
#define _HASH(h, x)				(h+(h<<5)+x)
#define _HASH4(h, x)			_HASH ( _HASH (\
									_HASH( _HASH(h, (x)[0] ), (x)[1]),\
												(x)[2]), (x)[3] )

#define _HASH8(h, x)			_HASH4 ( _HASH4 (h, x), &(x)[4] )
#define _HASH16(h, x)			_HASH8 ( _HASH8 (h, x), &(x)[8] )
#define _HASH32(h, x)			_HASH16( _HASH16(h, x), &(x)[16])

#define HASH4(x)				_HASH4 (5381ul, x)
#define HASH8(x)				_HASH8 (5381ul, x)
#define HASH16(x)				_HASH16(5381ul, x)
#define HASH32(x)				_HASH32(5381ul, x)

#ifdef __cplusplus
}
#endif

#endif // __COMMON_HASH_H__