#ifndef __CHARACTER_TRANSCODING_H__
#define __CHARACTER_TRANSCODING_H__


#include <stddef.h>
#ifdef _WIN32
#include <windows.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
size_t conv_mbs2wcs(wchar_t** dest, size_t* size, 
					const char* src, size_t n, int cp_src);

size_t conv_wcs2mbs(char** dest, size_t* size, int cp_dest,
				const wchar_t* src, size_t n);

size_t convert(void* dest, size_t* size, int cp_dest, 
				const void* src, size_t n, int cp_src);
#endif

size_t conv_wcstombs(char** dest, size_t* size, const wchar_t* src, size_t n);
size_t conv_mbstowcs(wchar_t** dest, size_t* size, const char* src, size_t n);

#ifdef __cplusplus
}
#endif


/* cplusplus */
#if defined(__cplusplus) && defined(_WIN32)
#include "tstring.h"

std::exstring conv_wcs2mbs(const std::exwstring& src, size_t cnchar, int cp_dest);
std::exwstring conv_mbs2wcs(const std::exstring& src, size_t cnchar, int cp_src);
std::exstring convert(const std::exstring& src, size_t cnchar, int cp_src, int cp_dest);
#endif

#endif //__CHARACTER_TRANSCODING_H__