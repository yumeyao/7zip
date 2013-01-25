#ifdef USE_ASM
__declspec(naked) HWND WINAPI FuncName(HWND hwndCaller, StrType pszFile, UINT uCommand, DWORD_PTR dwData)
{
	__asm {
	mov    eax, dword ptr [g_hmodHHCtrl]
	test   eax, eax
	jnz    CheckFunc
	cmp    eax, dword ptr [g_fTriedAndFailed]
	jnz    CheckFunc
	call   TryLoadingModule
	test   eax, eax
	jz     Fail
	mov    dword ptr [g_hmodHHCtrl], eax
CheckFunc:
	mov    ecx, dword ptr [mkFuncPtr(FuncName)]
	jecxz  GetFunc
	jmp    ecx
GetFunc:
	push   mkFuncHint(FuncName)
	push   eax
	call   dword ptr [GetProcAddress]
	test   eax, eax
	jz     Fail
	mov    dword ptr [mkFuncPtr(FuncName)], eax
	jmp    eax
Fail:
	or     dword ptr [g_fTriedAndFailed], -1
	ret    16
	}
}
#else
HWND WINAPI FuncName(HWND hwndCaller, StrType pszFile, UINT uCommand, DWORD_PTR dwData)
{
	HMODULE hMod = g_hmodHHCtrl;
	mkFuncPtrType(FuncName) pFunc;
	if (!hMod && g_fTriedAndFailed == FALSE) {
		if (hMod = TryLoadingModule())
			g_hmodHHCtrl = hMod;
		else
			goto Fail;
	}

	pFunc = mkFuncPtr(FuncName);
	if (pFunc) goto CallFunc;
	if (pFunc = (mkFuncPtrType(FuncName))GetProcAddress(g_hmodHHCtrl, (LPCSTR)mkFuncHint(FuncName))) {
		mkFuncPtr(FuncName) = pFunc;
CallFunc:
		return pFunc((HWND_)hwndCaller, pszFile, uCommand, dwData);
	}

Fail:
	g_fTriedAndFailed = ~FALSE;
	return NULL;
}
#endif
