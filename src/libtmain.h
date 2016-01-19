#ifndef __LIBTMAIN_H_INCLUDE
#define __LIBTMAIN_H_INCLUDE

#ifndef _tmain
#ifdef _UNICODE
#define _tmain wmain
#else
#define _tmain main
#endif
#endif // _tmain

#if !defined(__MSVCRT__) && defined(_UNICODE)
#include <stdlib.h>
int wmain(int argc, wchar_t *argv[]);

int main(int argc, char* argv[])
{
	int rc = EXIT_FAILURE, i;
	size_t len;
	wchar_t **_argv;
	
	_argv = (wchar_t**)calloc(argc, sizeof(wchar_t*));
	if (NULL == _argv) 
		return -1;
	for (i = 0; i < argc; ++i){
		len = mbstowcs(NULL, argv[i], 0);
		if (len == (size_t)-1) 
			goto end;
		_argv[i] = (wchar_t*)calloc(len+1, sizeof(wchar_t));
		if (NULL == _argv[i])
			goto end;
		mbstowcs(_argv[i], argv[i], len+1);
	}
		
	rc = wmain(argc, _argv);

end:	
	for (i = 0; i < argc; ++i){
		if (NULL != _argv[i])
			free(_argv[i]);
	}
	free(_argv);
	return rc;
}
#endif



#endif    //  __LIBTMAIN_H_INCLUDE