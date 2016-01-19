#include <windows.h>
#include <tchar.h>
#include <string.h>


int WinAutoStartOn(const TCHAR* name, const TCHAR* cmd)
{
	HKEY hKey;
	LSTATUS res;
	LPCTSTR lpRun = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run");

	if (NULL == name || NULL == cmd)
		return -1;

	res = RegOpenKeyEx(HKEY_CURRENT_USER, lpRun, 0, KEY_WRITE, &hKey);
	if (res != ERROR_SUCCESS)
		return res;

	res = RegSetValueEx(hKey, name, 0, REG_SZ, (const BYTE*)cmd, _tcslen(cmd)*sizeof(TCHAR));
	RegCloseKey(hKey);

	return (res == ERROR_SUCCESS ? 0 : res);
}


int WinAutoStartOff(const TCHAR* name)
{
	HKEY hKey;
	LSTATUS res;
	LPCTSTR lpRun = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run");

	if (NULL == name)
		return -1;

	res = RegOpenKeyEx(HKEY_CURRENT_USER, lpRun, 0, KEY_WRITE, &hKey);
	if (res != ERROR_SUCCESS)
		return res;

	res = RegDeleteValue(hKey, name);
	RegCloseKey(hKey);

	return (res == ERROR_SUCCESS ? 0 : res);
}