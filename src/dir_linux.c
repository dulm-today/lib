#include "dir.h"
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#define PATH_CHAR 	'/'
#define PATH_STR	"/"

int read_dir(const char* dir, read_dir_callback cb, void* data)
{
	DIR              *pdir ; 
    struct dirent    *ent  ; 
	
	if (NULL == dir || NULL == cb)
		return -1;
		
	pdir = opendir(dir);
	if (NULL == pdir)
		return -1;
		
	while(NULL != (ent = readdir(pdir))){
		if (cb(dir, ent->d_name, ent->d_type & DT_DIR, data)){
			break;
		}
	}
	
	closedir(pdir);
	return 0;
}


typedef struct
{
	DIR*				_dir;
}read_dir_t;

void* read_dir_init(const CHAR_DEF* dir)
{	
	read_dir_t* ptr = (read_dir_t*)malloc(sizeof(read_dir_t));
	if (NULL == ptr)
		return NULL;

	ptr->_dir = opendir(dir);
	if (NULL == ptr->_dir){
		read_dir_close(ptr);
		return NULL;
	}

	return ptr;
}

int   read_dir_next(void* h, CHAR_DEF* name, int size, int* bdir)
{
	read_dir_t* ptr = (read_dir_t*)h;
	struct dirent    *ent  ;
	
	if (NULL == h || NULL == name || NULL == bdir)
		return -1;
	
	ent = readdir(ptr->_dir);
	if (NULL == ent)
		return -1;
	
	strncpy(name, ent->d_name, size);
	*bdir = ent->d_type & DT_DIR;
	return 0;
}

void  read_dir_close(void* h)
{
	read_dir_t* ptr = (read_dir_t*)h;
	
	if (NULL == h)
		return;
	if (NULL != ptr->_dir)
		closedir(ptr->_dir);
	free(ptr);
}

int absulate_path(const char* src, char* dest, size_t size)
{
	char* 	p;
	size_t	need;
	
	if (NULL == src || NULL == dest)
		return -1;
	
	if (NULL != dest && size >= PATH_MAX){
		p = real_path(src, dest);
		
		return (NULL == p ? 0 : strlen(dest)+1);
	}
	
	p = realpath(src, NULL);
	if (NULL == p)
		return 0;
		
	need = strlen(p) + 1;
	if (size >= need)
		memcpy(dest, p, need);
		
	free(p);
	return need;
}