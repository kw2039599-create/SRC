#pragma once

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#ifndef CHAR
typedef char CHAR;
#endif

typedef uintptr_t ULONG_PTR;

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE ((HANDLE)-1)
#endif

#define MAX_MODULE_NAME32 255

#define TH32CS_SNAPHEAPLIST 0x00000001
#define TH32CS_SNAPPROCESS  0x00000002
#define TH32CS_SNAPTHREAD   0x00000004
#define TH32CS_SNAPMODULE   0x00000008
#define TH32CS_SNAPMODULE32 0x00000010
#define TH32CS_SNAPALL      (TH32CS_SNAPHEAPLIST | TH32CS_SNAPPROCESS | TH32CS_SNAPTHREAD | TH32CS_SNAPMODULE)
#define TH32CS_INHERIT      0x80000000

typedef struct tagPROCESSENTRY32 {
    DWORD     dwSize;
    DWORD     cntUsage;
    DWORD     th32ProcessID;
    ULONG_PTR th32DefaultHeapID;
    DWORD     th32ModuleID;
    DWORD     cntThreads;
    DWORD     th32ParentProcessID;
    LONG      pcPriClassBase;
    DWORD     dwFlags;
    CHAR      szExeFile[MAX_PATH];
} PROCESSENTRY32, *PPROCESSENTRY32, *LPPROCESSENTRY32;

typedef struct tagMODULEENTRY32 {
    DWORD   dwSize;
    DWORD   th32ModuleID;
    DWORD   th32ProcessID;
    DWORD   GlblcntUsage;
    DWORD   ProccntUsage;
    BYTE  * modBaseAddr;
    DWORD   modBaseSize;
    HMODULE hModule;
    char    szModule[MAX_MODULE_NAME32 + 1];
    char    szExePath[MAX_PATH];
} MODULEENTRY32, *PMODULEENTRY32, *LPMODULEENTRY32;

static inline HANDLE CreateToolhelp32Snapshot(DWORD dwFlags, DWORD th32ProcessID) {
    (void)dwFlags;
    (void)th32ProcessID;
    return INVALID_HANDLE_VALUE;
}

static inline BOOL Process32First(HANDLE hSnapshot, LPPROCESSENTRY32 lppe) {
    (void)hSnapshot;
    (void)lppe;
    return FALSE;
}

static inline BOOL Process32Next(HANDLE hSnapshot, LPPROCESSENTRY32 lppe) {
    (void)hSnapshot;
    (void)lppe;
    return FALSE;
}

static inline BOOL Module32First(HANDLE hSnapshot, LPMODULEENTRY32 lpme) {
    (void)hSnapshot;
    (void)lpme;
    return FALSE;
}

static inline BOOL Module32Next(HANDLE hSnapshot, LPMODULEENTRY32 lpme) {
    (void)hSnapshot;
    (void)lpme;
    return FALSE;
}

#ifdef __cplusplus
}
#endif