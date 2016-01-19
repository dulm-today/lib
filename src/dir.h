#ifndef __DIR_H__
#define __DIR_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef UNICODE
#define CHAR_DEF	char
#else
#define CHAR_DEF	wchar_t
#endif


typedef int (*read_dir_callback)(const CHAR_DEF* path, const CHAR_DEF* name, int dir, void* data);

/*
	find all in the dir. use read_dir_callback to get what is found.
	if read_dir_callback return 0 then continue, else will break.
	
	return:
		0 :		Ok
		other:	Error
*/
int read_dir(const CHAR_DEF* dir, read_dir_callback cb, void* data);



void* read_dir_init(const CHAR_DEF* dir);
int   read_dir_next(void* h, CHAR_DEF* name, int size, int* bdir);
void  read_dir_close(void* h);


/*
	get the absulate path. parameter size contain null characters.
	
	return:
		< 0 :			parameter is wrong.
		0   :			system call is error.
		>= size :		dest is too small, return the size of space.
		other :			the number of characters in the dest.
*/
int absulate_path(const CHAR_DEF* src, CHAR_DEF* dest, size_t size);



#ifdef __cplusplus
}
#endif

#endif // __DIR_H__