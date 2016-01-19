#ifndef __BITCOUNT_H__
#define __BITCOUNT_H__

#ifdef __cplusplus
extern "C" {
#endif

int bitcount8(unsigned char n);
int bitcount16(unsigned short n);
int bitcount32(unsigned int n);

#ifdef __cplusplus
}
#endif

#endif // __BITCOUNT_H__
