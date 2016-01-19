#include <windows.h>
#include <tchar.h>
#include <string.h>
#include <stdio.h>
#include "dir.h"
#include "libdef.h"
#include "printf_safe.h"

int read_dir(const TCHAR* dir, read_dir_callback cb, void* data)
{
	WIN32_FIND_DATA wfd;
	HANDLE hFind = NULL;
	TCHAR strpath[MAX_PATH];
	
	if (NULL == dir || NULL == cb)
		return -1;
	
	snprintf_safe(strpath, MAX_PATH, "%s%s%s", dir, _T(PATH_STR), _T("*.*"));

	hFind = FindFirstFile(strpath, &wfd);
	if (hFind == INVALID_HANDLE_VALUE)
		return -1;

	do{
		if (cb(dir, wfd.cFileName, wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY, data)){
			// Do not need continue;
			break;
		}
	}while(FindNextFile(hFind, &wfd));
	
	FindClose(hFind);
	return 0;
}

typedef struct
{
	HANDLE			_handle;
	WIN32_FIND_DATA _wfd;
	int				_first;
}read_dir_t;

void* read_dir_init(const CHAR_DEF* dir)
{	
	TCHAR strpath[MAX_PATH];
	read_dir_t* ptr = (read_dir_t*)malloc(sizeof(read_dir_t));
	if (NULL == ptr)
		return NULL;

	snprintf_safe(strpath, MAX_PATH, "%s%s%s", dir, _T(PATH_STR), _T("*.*"));
	
	ptr->_first = 1;
	ptr->_handle = FindFirstFile(strpath, &ptr->_wfd);
	if (INVALID_HANDLE_VALUE == ptr->_handle){
		read_dir_close(ptr);
		return NULL;
	}

	return ptr;
}

int   read_dir_next(void* h, CHAR_DEF* name, int size, int* bdir)
{
	read_dir_t* ptr = (read_dir_t*)h;
	
	if (NULL == h || NULL == name || NULL == bdir)
		return -1;
	if (!ptr->_first && !FindNextFile(ptr->_handle, &ptr->_wfd)){
		return -1;
	}
	else {
		ptr->_first = 0;
	}

	strncpy(name, ptr->_wfd.cFileName, size);
	*bdir = ptr->_wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
	return 0;
}

void  read_dir_close(void* h)
{
	read_dir_t* ptr = (read_dir_t*)h;
	
	if (NULL == h)
		return;
	if (INVALID_HANDLE_VALUE != ptr->_handle)
		FindClose(ptr->_handle);
	free(ptr);
}

int absulate_path(const TCHAR* src, TCHAR* dest, size_t size)
{
	TCHAR* p;
	
	if (NULL == src || NULL == dest)
		return -1;
		
	return GetFullPathName(src, size, dest, &p);
}