#include <stdlib.h>
#include <locale.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "character_transcoding.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
size_t conv_mbs2wcs(wchar_t** dest, size_t* size, 
				const char* src, size_t n, int cp_src)
{
	int rc;
	size_t	utf16_size;
	wchar_t* utf16 = NULL;
	
	if (-1 == cp_src)
		return 0;

	if (NULL == dest || NULL == size){
		//not convert, calculate the buffer size
		rc = MultiByteToWideChar(cp_src, 0, src, n, NULL, 0);
	}else if (NULL == *dest || 0 == *size){
		//no buffer to obtain the wide str, need malloc
		rc = MultiByteToWideChar(cp_src, 0, src, n, NULL, 0);
		if (rc <= 0)
			return -1;
		utf16_size = rc;
		utf16 = (wchar_t*)malloc(sizeof(wchar_t)*rc);
		if (NULL == utf16)
			return -1;
		rc = MultiByteToWideChar(cp_src, 0, src, n, utf16, utf16_size);
		if (rc <= 0){
			free(utf16);
			return -1;
		}

		*dest = utf16;
		*size = utf16_size;
	}else{
		//use the dest buffer
		rc = MultiByteToWideChar(cp_src, 0, src, n, *dest, *size);
	}
	
	return rc;
}

size_t conv_wcs2mbs(char** dest, size_t* size, int cp_dest,
				const wchar_t* src, size_t n)
{
	int rc;
	size_t	mbs_size;
	char* mbs;
	
	if (-1 == cp_dest)
		return 0;

	if (NULL == dest || NULL == size){
		//not convert, calculate the buffer size
		rc = WideCharToMultiByte(cp_dest, 0, src, n, NULL, 0, NULL, NULL);
	}else if (NULL == *dest || 0 == *size){
		//no buffer to obtain the wide str, need malloc
		rc = WideCharToMultiByte(cp_dest, 0, src, n, NULL, 0, NULL, NULL);
		if (rc <= 0)
			return -1;
		mbs_size = rc;
		mbs = (char*)malloc(sizeof(char)*rc);
		if (NULL == mbs)
			return -1;
		rc = WideCharToMultiByte(cp_dest, 0, src, n, 
							mbs, mbs_size, NULL, NULL);
		if (rc <= 0){
			free(mbs);
			return -1;
		}
		*dest = mbs;
		*size = mbs_size;
	}else{
		//use the dest buffer
		rc = WideCharToMultiByte(cp_dest, 0, src, n, 
							*dest, *size, NULL, NULL);
	}
	
	return rc;
}

size_t convert(void* dest, size_t* size, int cp_dest, 
				const void* src, size_t n, int cp_src)
{
	int rc;
	wchar_t* wcs = NULL;
	size_t	 wcs_size = 0;
	size_t	 wcs_len;

	if (-1 == cp_src)
		// src is unicode
		return conv_wcs2mbs((char**)dest, size, cp_dest, 
					(wchar_t*)src, n);

	if (-1 == cp_dest)
		// dest is unicode
		return conv_mbs2wcs((wchar_t**)dest, size, 
					(const char*)src, n, cp_src);

	rc = conv_mbs2wcs(&wcs, &wcs_size, (const char*)src, n, cp_src);
	if (rc <= 0)
		return rc;

	wcs_len = rc;
	rc = conv_wcs2mbs((char**)dest, size, cp_dest, 
							wcs, wcs_len);

	free(wcs);
	return rc;
}

#else// !_WIN32

size_t conv_wcstombs(char** dest, size_t* size, const wchar_t* src, size_t n)
{
	size_t rc;
	
	if (NULL == dest || NULL == size){
		rc = wcstombs(NULL, src, n);
	}
	if (NULL == *dest || 0 == *size){
		char* cs;
		size_t cs_size = wcstombs(NULL, src, n);
		if (-1 == cs_size)
			return -1;

		cs = (char*)malloc((cs_size+1)*sizeof(char));
		if (NULL == cs)
			return -1;

		rc = wcstombs(cs, src, n);
		if (-1 == rc){
			free(cs);
			return -1
		}
		*dest = cs;
		*size = cs_size+1;
	}
	else{
		rc = wcstombs(*dest, src, n);
	}
		
	return rc;
}

size_t conv_mbstowcs(wchar_t** dest, size_t* size, const char* src, size_t n)
{
	size_t rc;
	
	if (NULL == dest || NULL == size){
		rc = mbstowcs(NULL, src, n);
	}
	if (NULL == *dest || 0 == *size){
		wchar_t* cs;
		size_t cs_size = mbstowcs(NULL, src, n);
		if (-1 == cs_size)
			return -1;

		cs = (char*)malloc((cs_size+1)*sizeof(wchar_t));
		if (NULL == cs)
			return -1;

		rc = mbstowcs(cs, src, n);
		if (-1 == rc){
			free(cs);
			return -1
		}
		*dest = cs;
		*size = cs_size+1;
	}
	else{
		rc = mbstowcs(*dest, src, n);
	}
		
	return rc;
}

#endif //!_WIN32

#ifdef __cplusplus
}
#endif


/* cplusplus */
#ifdef __cplusplus
using namespace std;

#ifdef _WIN32
exstring conv_wcs2mbs(const exwstring& src, size_t cnchar, int cp_dest)
{
	exstring dest;

	if (-1 == cnchar)
		cnchar = src.length();
	
	size_t cndest = conv_wcs2mbs(NULL, NULL, cp_dest, src.c_str(), cnchar);
	if (-1 == cndest)
		return dest;

	char* ptr = dest.getbuffer(cndest);
	if (NULL != ptr){
		conv_wcs2mbs(&ptr, &cndest, cp_dest, src.c_str(), cnchar);
	}
	dest.releasebuffer(cndest);

	return dest;
}

exwstring conv_mbs2wcs(const exstring& src, size_t cnchar, int cp_src)
{
	exwstring dest;

	if (-1 == cnchar)
		cnchar = src.length();
	
	size_t cndest = conv_mbs2wcs(NULL, NULL, src.c_str(), cnchar, cp_src);
	if (-1 == cndest)
		return dest;

	wchar_t* ptr = dest.getbuffer(cndest);
	if (NULL != ptr){
		conv_mbs2wcs(&ptr, &cndest, src.c_str(), cnchar, cp_src);
	}
	dest.releasebuffer(cndest);

	return dest;
}

std::exstring convert(const std::exstring& src, size_t cnchar, int cp_src, int cp_dest)
{
	std::exstring dest("");

	if (-1 == cp_dest)
		return std::move(dest);
	
	if (-1 == cnchar)
		cnchar = src.length();
	
	size_t cndest = convert(NULL, NULL, cp_dest, src.c_str(), cnchar, cp_src);
	if (-1 == cndest)
		return dest;

	void* ptr = dest.getbuffer(cndest);
	if (NULL != ptr){
		convert(&ptr, &cndest, cp_dest, src.c_str(), cnchar, cp_src);
	}
	dest.releasebuffer(cndest);

	return dest;
}
#endif // _WIN32
#endif // cplusplus

