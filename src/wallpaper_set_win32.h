#ifndef __WALLPAPER_SET_WIN32_H__
#define __WALLPAPER_SET_WIN32_H__

#include <WinSock2.h>
#include <windows.h>
#include <WinInet.h>
#include <shlobj.h>
#include <tchar.h>

/*
#define WPSTYLE_CENTER      0
#define WPSTYLE_TILE        1
#define WPSTYLE_STRETCH     2
#if (NTDDI_VERSION >= NTDDI_WIN7)
#define WPSTYLE_KEEPASPECT  3
#define WPSTYLE_CROPTOFIT   4
#define WPSTYLE_MAX         5
#else
#define WPSTYLE_MAX         3
#endif // NTDDI_WIN7
*/
int wallpaper_set(LPCTSTR path, int style);


#endif //__WALLPAPER_SET_WIN32_H__
