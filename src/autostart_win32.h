#ifndef __AUTOSTART_WIN32_H__
#define __AUTOSTART_WIN32_H__

#include <tchar.h>

#ifdef __cplusplus
extern "C" {
#endif 

/*
	Registe a key to run the program after the system start.
	The parameter "name" is the name of the key, the parameter "cmd" is
	the value of the key. the "cmd" is like "C:\\Windows\\aa.exe -s 3600"
	
	return:
		0 		:		Ok
		other	:		ErrorCode
*/
int WinAutoStartOn(const TCHAR* name, const TCHAR* cmd);


/*
	Delete the key that registe by WinAutoStartOn with "name".
	
	return:
		0		:		Ok
		other	:		ErrorCode
*/
int WinAutoStartOff(const TCHAR* name);


#ifdef __cplusplus
}
#endif 

#endif // __AUTOSTART_WIN32_H__

