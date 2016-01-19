#include <windows.h>
#include <WinInet.h>
#include <shlobj.h>
#include <tchar.h>

int wallpaper_set(LPCTSTR path, int style)
{
	HRESULT hr;
	IActiveDesktop* pIAD;
	
	CoInitialize(NULL);  
	hr = CoCreateInstance ( CLSID_ActiveDesktop,  NULL, CLSCTX_INPROC_SERVER,       
				IID_IActiveDesktop, (void**) &pIAD );
	if(!SUCCEEDED(hr)) 
		return FALSE;

#ifndef UNICODE
	WCHAR   wszWallpaper [2*MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, path, -1, wszWallpaper, MAX_PATH);
#else
	const WCHAR*  wszWallpaper = path;
#endif
	hr = pIAD->SetWallpaper(wszWallpaper, 0);
	if(!SUCCEEDED(hr)) 
		return FALSE;

	WALLPAPEROPT wpo;
	wpo.dwSize = sizeof(wpo);
	wpo.dwStyle = style;
	hr = pIAD->SetWallpaperOptions(&wpo, 0);
	if(!SUCCEEDED(hr)) 
		return FALSE;

	hr = pIAD->ApplyChanges(AD_APPLY_ALL);
	if(!SUCCEEDED(hr)) 
		return FALSE;

	pIAD->Release();
	CoUninitialize(); 
	return TRUE;
}

