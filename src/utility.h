#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <stddef.h>
#ifdef __cplusplus
extern "C"{
#endif

void write_u16(void* out, unsigned short u16);
void write_u32(void* out, unsigned int u32);

void read_u16(void* in, unsigned short* pu16);
void read_u32(void* in, unsigned int* pu32);

#ifdef __cplusplus
}
#endif

#endif //__UTILITY_H__
