#ifndef __CRC_H__
#define __CRC_H__

#ifdef __cplusplus
extern "C"{
#endif


unsigned int crc32(const void* data, unsigned int size );


/***
 * crc16 - compute the CRC-16 for the data buffer
 * @crc:	previous CRC value
 * @buffer:	data pointer
 * @len:	number of bytes in the buffer
 *
 * Returns the updated CRC value.
 */
 unsigned short crc16( unsigned short crc, const unsigned char* buffer, unsigned int len );


/***
 * crc7 - update the CRC7 for the data buffer
 * @crc:     previous CRC7 value
 * @buffer:  data pointer
 * @len:     number of bytes in the buffer
 * Context: any
 *
 * Returns the updated CRC7 value.
 */
unsigned char crc7( unsigned char crc, const unsigned  char *buffer, unsigned int len );


#ifdef __cplusplus
}
#endif

#endif //__CRC_H__
