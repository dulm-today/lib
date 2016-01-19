#include "test_core.h"
#include "dir.h"
#include <stdio.h>
#include <string>
#include <windows.h>

const char* depth_str(int depth){
	static char depth_str[MAX_PATH] = {0};

	char* ptr = &depth_str[0];

	for (int i = 0; i < depth; ++i){
		*ptr++ = ' ';
		*ptr++ = ' ';
	}
	*ptr = '\0';

	return &depth_str[0];
}

int read_dir_cb(const char* path, const char* name, int isdir, void* data)
{
	int depth = *(int*)data + 1;
	
	if (isdir && 0 != strcmp(name, ".") && 0 != strcmp(name, "..")){
		fprintf(stderr, "%s%s©·\n", depth_str(depth), name);
		std::string path(path);
		path += "\\";
		path += name;
		read_dir(path.c_str(), read_dir_cb, &depth);
	}
	else{
		fprintf(stderr, "%s%s\n", depth_str(depth), name);
	}
	
	return 0;
}

int dir_read_test()
{
	char dest[MAX_PATH];
	int size;
	int depth = 0;
	size = absulate_path("..", dest, MAX_PATH);
	
	fprintf(stderr, "read_dir: ..(%s)\n", dest);
	
	return read_dir("..", read_dir_cb, &depth);
}

int absulate_path_test()
{
	char dest[MAX_PATH];
	int size;
	
	size = absulate_path("..", dest, 0);
	if (size <= 0)
		return -1;
		
	size = absulate_path("..", dest, MAX_PATH);
	if (size <= 0)
		return -1;
	
	fprintf(stderr, "absulate_path: .. -> %s\n\n", dest);
	
	return 0;
}

int dir_test_cb()
{
	int error = 0;
	
	if (absulate_path_test())
		error++;
	
	if (dir_read_test())
		error++;

	return error;
}

Test dir_test("dir", dir_test_cb, 2);