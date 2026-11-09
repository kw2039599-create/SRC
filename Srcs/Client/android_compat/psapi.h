#pragma once

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _MODULEINFO {
    LPVOID lpBaseOfDll;
    DWORD  SizeOfImage;
    LPVOID EntryPoint;
} MODULEINFO, *LPMODULEINFO;

inline BOOL GetModuleInformation(HANDLE hProcess, HMODULE hModule, LPMODULEINFO lpmodinfo, DWORD cb) {
    if (lpmodinfo) {
        lpmodinfo->lpBaseOfDll = (LPVOID)hModule;
        lpmodinfo->SizeOfImage = 0x100000;
        lpmodinfo->EntryPoint = NULL;
    }
    return TRUE;
}

inline BOOL EnumProcessModules(HANDLE hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded) {
    if (lphModule && cb >= sizeof(HMODULE)) {
        *lphModule = (HMODULE)0x400000;
        if (lpcbNeeded) *lpcbNeeded = sizeof(HMODULE);
        return TRUE;
    }
    return FALSE;
}

inline DWORD GetModuleBaseNameA(HANDLE hProcess, HMODULE hModule, LPSTR lpBaseName, DWORD nSize) {
    if (lpBaseName && nSize > 0) {
        strncpy(lpBaseName, "metin2_mobile.so", nSize);
        return (DWORD)strlen(lpBaseName);
    }
    return 0;
}

#define GetModuleBaseName GetModuleBaseNameA

#ifdef __cplusplus
}
#endif