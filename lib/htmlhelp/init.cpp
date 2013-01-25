/*
 * htmlhelp/init.c
 *
 * used to replace htmlhelp.lib in windows SDK to remove security_cookie
 *
 */

#include <windows.h>

static HMODULE g_hmodHHCtrl = NULL;
static BOOL g_fTriedAndFailed = FALSE;
struct HWND__;
typedef HWND__ *HWND_;
typedef HWND_ (WINAPI *pHtmlHelpA_t)(HWND_ hwndCaller, LPCSTR pszFile, UINT uCommand, DWORD_PTR dwData);
typedef HWND_ (WINAPI *pHtmlHelpW_t)(HWND_ hwndCaller, LPCWSTR pszFile, UINT uCommand, DWORD_PTR dwData);
pHtmlHelpA_t pHtmlHelpA = NULL;
pHtmlHelpW_t pHtmlHelpW = NULL;
#define HtmlHelpAHint 14
#define HtmlHelpWHint 15

__forceinline static LPSTR GetRegisteredLocation(LPSTR pszPath, LPSTR pszExpandedPath)
{
	LPSTR ret = NULL;
	HKEY hKey;
	DWORD cbData;
	DWORD dwType;
	if (ERROR_SUCCESS != RegOpenKeyExA(
			HKEY_CLASSES_ROOT,
			"CLSID\\{ADB880A6-D8FF-11CF-9377-00AA003B7A11}\\InprocServer32",
			0, KEY_READ, &hKey))
		return ret;
	cbData = MAX_PATH;
	if (ERROR_SUCCESS == RegQueryValueExA(hKey, NULL, NULL, &dwType, (LPBYTE)pszPath, &cbData)) {
		if (REG_EXPAND_SZ==dwType) {
			if (MAX_PATH == ExpandEnvironmentStringsA(pszPath, pszExpandedPath, MAX_PATH)) {
				ret = pszExpandedPath;
			}
		} else {
			ret = pszPath;
		}
	}
	RegCloseKey(hKey);
	return ret;
}

__declspec(noinline) static HMODULE WINAPI TryLoadingModule()
{
	CHAR szPath[MAX_PATH];
	CHAR szExpandedPath[MAX_PATH];
	HMODULE hMod;
	CHAR *pszPath;

	if (pszPath = GetRegisteredLocation(szPath, szExpandedPath)) {
		hMod = LoadLibraryA(pszPath);
		if (!hMod)
			goto LoadDefaultPath;
	} else {
LoadDefaultPath:
		hMod = LoadLibraryA("hhctrl.ocx");
	}
	return hMod;
}

#ifndef _WIN64
#define USE_ASM
#endif

#define _mkFuncPtr(FuncName) p##FuncName
#define _mkFuncPtrType(FuncName) p##FuncName##_t
#define _mkFuncHint(FuncName) FuncName##Hint
#define mkFuncPtr(FuncName) _mkFuncPtr(FuncName)
#define mkFuncPtrType(FuncName) _mkFuncPtrType(FuncName)
#define mkFuncHint(FuncName) _mkFuncHint(FuncName)

#define FuncName HtmlHelpA
#define StrType LPCSTR
extern "C"
#include "sub.cpp"

#undef FuncName
#undef StrType
#define FuncName HtmlHelpW
#define StrType LPCWSTR
extern "C"
#include "sub.cpp"
