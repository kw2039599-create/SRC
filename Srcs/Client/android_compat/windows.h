#pragma once

#if !defined(__ANDROID__) && !defined(__APPLE__)
  #error "This windows.h is only for Android and iOS!"
#endif

#if defined(__APPLE__)
  #ifndef ANDROID
    #define ANDROID 1
  #endif
  #ifndef __ANDROID__
    #define __ANDROID__ 1
  #endif
#endif

// OLE Automation header guards for MSCTF / IME support on Android
#ifndef __OAIDL_H__
#define __OAIDL_H__
#endif

#ifndef __OCIDL_H__
#define __OCIDL_H__
#endif

#ifndef __OBJBASE_H__
#define __OBJBASE_H__
#endif

// Define DECLSPEC_UUID using Clang's cross-platform attribute for __uuidof support on Android
#undef DECLSPEC_UUID
#define DECLSPEC_UUID(x) __declspec(uuid(x))

#ifndef DECLSPEC_NOVTABLE
#define DECLSPEC_NOVTABLE
#endif

#undef MIDL_INTERFACE
#define MIDL_INTERFACE(x) struct __declspec(uuid(x))

// Minimum stub for Windows types required by the engine.

#include <stdint.h>
#include <stddef.h>
#include <stdint.h>
#include <stddef.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WIN32_LEAN_AND_MEAN
#ifndef _WIN32
#define _WIN32
#endif

typedef int BOOL;
typedef int32_t LONG;
typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef uint8_t BYTE;
typedef void* HANDLE;
typedef void* LPVOID;
typedef void VOID;
typedef void* PVOID;
typedef const void* LPCVOID;
typedef uint32_t UINT;
typedef int32_t INT;
typedef float FLOAT;
typedef void* HWND;
typedef void* HDC;
typedef void* HGLRC;
typedef void* HMODULE;
typedef void* HINSTANCE;
typedef long LRESULT;
typedef long HRESULT;

#define TRUE 1
#define FALSE 0

#define S_OK 0
#define S_FALSE 1
#define E_FAIL -1
#define E_NOTIMPL -2
#define E_OUTOFMEMORY -3
#define E_INVALIDARG -4
#define E_POINTER -5
#define E_HANDLE -6
#define E_ABORT -7
#define E_ACCESSDENIED -8
#define E_PENDING -9
#define E_NOINTERFACE -10

#define WINAPI
#define CALLBACK
#define __stdcall
#define STDMETHODCALLTYPE

typedef struct _RECT {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
} RECT, *PRECT;

typedef struct _POINT {
  LONG x;
  LONG y;
} POINT, *PPOINT, *LPPOINT;

typedef struct _SIZE {
  LONG cx;
  LONG cy;
} SIZE, *PSIZE;

// D3D types for GrpOpenGL.h
typedef struct _D3DXIMAGE_INFO {
    UINT Width;
    UINT Height;
    UINT Depth;
    UINT MipLevels;
    uint32_t Format;
    uint32_t ResourceType;
    uint32_t ImageFileFormat;
} D3DXIMAGE_INFO;

typedef struct _PALETTEENTRY {
    BYTE peRed;
    BYTE peGreen;
    BYTE peBlue;
    BYTE peFlags;
} PALETTEENTRY;

typedef struct _D3DVIEWPORT8 {
    DWORD       X;
    DWORD       Y;
    DWORD       Width;
    DWORD       Height;
    float       MinZ;
    float       MaxZ;
} D3DVIEWPORT8;

typedef uint32_t D3DCOLOR;
typedef uint32_t D3DFORMAT;
typedef uint32_t D3DPOOL;

#define D3DX_DEFAULT            ((UINT)-1)
#define D3DFMT_A8R8G8B8         21
#define D3DPOOL_MANAGED         1

typedef struct IDirect3DDevice8 *LPDIRECT3DDEVICE8;
typedef struct IDirect3DTexture8 *LPDIRECT3DTEXTURE8;

#ifdef __cplusplus
}
#endif
#ifndef CONST
#define CONST const
#endif
#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif
#ifndef OPTIONAL
#define OPTIONAL
#endif
#ifndef FAR
#define FAR
#endif
#ifndef NEAR
#define NEAR
#endif
#ifndef PASCAL
#define PASCAL
#endif
#ifndef CDECL
#define CDECL
#endif

typedef struct _CRITICAL_SECTION {
    pthread_mutex_t mutex;
} CRITICAL_SECTION, *PCRITICAL_SECTION, *LPCRITICAL_SECTION;

static inline void InitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection) {
    if (!lpCriticalSection) return;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&lpCriticalSection->mutex, &attr);
    pthread_mutexattr_destroy(&attr);
}

static inline void DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection) {
    if (!lpCriticalSection) return;
    pthread_mutex_destroy(&lpCriticalSection->mutex);
}

static inline void EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection) {
    if (!lpCriticalSection) return;
    pthread_mutex_lock(&lpCriticalSection->mutex);
}

static inline void LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection) {
    if (!lpCriticalSection) return;
    pthread_mutex_unlock(&lpCriticalSection->mutex);
}

static inline BOOL TryEnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection) {
    if (!lpCriticalSection) return FALSE;
    return (pthread_mutex_trylock(&lpCriticalSection->mutex) == 0) ? TRUE : FALSE;
}

typedef void* HGDIOBJ;
typedef void* HBITMAP;
typedef void* HFONT;
typedef void* HBRUSH;
typedef void* HPEN;
typedef void* HICON;
typedef void* HCURSOR;
typedef void* HMENU;
typedef void* HRGN;
typedef uintptr_t WPARAM;
typedef intptr_t LPARAM;
typedef DWORD COLORREF;

#ifndef RGB
#define RGB(r,g,b) ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
#endif

#ifndef BI_RGB
#define BI_RGB 0L
#endif
#ifndef DIB_RGB_COLORS
#define DIB_RGB_COLORS 0
#endif

typedef struct tagRGBQUAD {
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
    BYTE rgbReserved;
} RGBQUAD;

typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG  biWidth;
    LONG  biHeight;
    WORD  biPlanes;
    WORD  biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG  biXPelsPerMeter;
    LONG  biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER, *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD          bmiColors[1];
} BITMAPINFO, *LPBITMAPINFO, *PBITMAPINFO;

#ifndef LF_FACESIZE
#define LF_FACESIZE 32
#endif

typedef struct tagLOGFONTA {
    LONG lfHeight;
    LONG lfWidth;
    LONG lfEscapement;
    LONG lfOrientation;
    LONG lfWeight;
    BYTE lfItalic;
    BYTE lfUnderline;
    BYTE lfStrikeOut;
    BYTE lfCharSet;
    BYTE lfOutPrecision;
    BYTE lfClipPrecision;
    BYTE lfQuality;
    BYTE lfPitchAndFamily;
    char lfFaceName[LF_FACESIZE];
} LOGFONTA, LOGFONT, *PLOGFONT, *LPLOGFONT;

typedef struct tagTEXTMETRICA {
    LONG tmHeight;
    LONG tmAscent;
    LONG tmDescent;
    LONG tmInternalLeading;
    LONG tmExternalLeading;
    LONG tmAveCharWidth;
    LONG tmMaxCharWidth;
    LONG tmWeight;
    LONG tmOverhang;
    LONG tmDigitizedAspectX;
    LONG tmDigitizedAspectY;
    BYTE tmFirstChar;
    BYTE tmLastChar;
    BYTE tmDefaultChar;
    BYTE tmBreakChar;
    BYTE tmItalic;
    BYTE tmUnderlined;
    BYTE tmStruckOut;
    BYTE tmPitchAndFamily;
    BYTE tmCharSet;
} TEXTMETRICA, TEXTMETRIC, *PTEXTMETRIC, *LPTEXTMETRIC;

#include <string.h>
#include <wchar.h>

#ifndef CP_ACP
#define CP_ACP 0
#endif
#ifndef CP_OEMCP
#define CP_OEMCP 1
#endif
#ifndef CP_UTF7
#define CP_UTF7 65000
#endif
#ifndef CP_UTF8
#define CP_UTF8 65001
#endif

#ifndef MB_PRECOMPOSED
#define MB_PRECOMPOSED 0x00000001
#endif
#ifndef MB_COMPOSITE
#define MB_COMPOSITE 0x00000002
#endif
#ifndef MB_USEGLYPHCHARS
#define MB_USEGLYPHCHARS 0x00000004
#endif
#ifndef MB_ERR_INVALID_CHARS
#define MB_ERR_INVALID_CHARS 0x00000008
#endif

#ifndef WC_COMPOSITECHECK
#define WC_COMPOSITECHECK 0x00000200
#endif
#ifndef WC_NO_BEST_FIT_CHARS
#define WC_NO_BEST_FIT_CHARS 0x00000400
#endif

typedef char CHAR;
typedef char* LPSTR;
typedef const char* LPCSTR;
typedef wchar_t WCHAR;
typedef WCHAR* LPWSTR;
typedef const WCHAR* LPCWSTR;
typedef BOOL* LPBOOL;
typedef DWORD* LPDWORD;

static inline int MultiByteToWideChar(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar) {
    (void)CodePage;
    (void)dwFlags;
    static const uint16_t s_cp1256[128] = {
        0x20AC,0x067E,0x201A,0x0192,0x201E,0x2026,0x2020,0x2021,0x02C6,0x2030,0x0679,0x2039,0x0152,0x0686,0x0698,0x0688,
        0x06AF,0x2018,0x2019,0x201C,0x201D,0x2022,0x2013,0x2014,0x06A9,0x2122,0x0691,0x203A,0x0153,0x200C,0x200D,0x06BA,
        0x00A0,0x060C,0x00A2,0x00A3,0x00A4,0x00A5,0x00A6,0x00A7,0x00A8,0x00A9,0x06BE,0x00AB,0x00AC,0x00AD,0x00AE,0x00AF,
        0x00B0,0x00B1,0x00B2,0x00B3,0x00B4,0x00B5,0x00B6,0x00B7,0x00B8,0x00B9,0x061B,0x00BB,0x00BC,0x00BD,0x00BE,0x061F,
        0x06C1,0x0621,0x0622,0x0623,0x0624,0x0625,0x0626,0x0627,0x0628,0x0629,0x062A,0x062B,0x062C,0x062D,0x062E,0x062F,
        0x0630,0x0631,0x0632,0x0633,0x0634,0x0635,0x0636,0x00D7,0x0637,0x0638,0x0639,0x063A,0x0640,0x0641,0x0642,0x0643,
        0x00E0,0x0644,0x00E2,0x0645,0x0646,0x0647,0x0648,0x00E7,0x00E8,0x00E9,0x00EA,0x00EB,0x0649,0x064A,0x00EE,0x00EF,
        0x064B,0x064C,0x064D,0x064E,0x00F4,0x064F,0x0650,0x00F7,0x0651,0x00F9,0x0652,0x00FB,0x00FC,0x200E,0x200F,0x06D2
    };
    if (!lpMultiByteStr) return 0;
    int len = (cbMultiByte < 0) ? (int)(strlen(lpMultiByteStr) + 1) : cbMultiByte;
    if (cchWideChar == 0 || !lpWideCharStr) return len;
    int copyLen = (len < cchWideChar) ? len : cchWideChar;
    for (int i = 0; i < copyLen; ++i) {
        unsigned char c = (unsigned char)lpMultiByteStr[i];
        lpWideCharStr[i] = (c >= 0x80) ? (WCHAR)s_cp1256[c - 0x80] : (WCHAR)c;
    }
    return copyLen;
}

static inline int WideCharToMultiByte(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar) {
    (void)CodePage;
    (void)dwFlags;
    (void)lpDefaultChar;
    if (lpUsedDefaultChar) *lpUsedDefaultChar = FALSE;
    if (!lpWideCharStr) return 0;
    int len = (cchWideChar < 0) ? (int)(wcslen(lpWideCharStr) + 1) : cchWideChar;
    if (cbMultiByte == 0 || !lpMultiByteStr) return len;
    int copyLen = (len < cbMultiByte) ? len : cbMultiByte;
    for (int i = 0; i < copyLen; ++i) {
        lpMultiByteStr[i] = (char)(lpWideCharStr[i] & 0xFF);
    }
    return copyLen;
}

// Win32 character navigation APIs for EterLib/parser.cpp
static inline LPSTR CharNextExA(DWORD CodePage, LPCSTR lpCurrentChar, DWORD dwFlags) {
    (void)dwFlags;
    if (!lpCurrentChar || !*lpCurrentChar)
        return (LPSTR)lpCurrentChar;

    unsigned char c = (unsigned char)*lpCurrentChar;
    int len = 1;

    if (CodePage == 65001 || CodePage == CP_UTF8) {
        if ((c & 0x80) == 0x00) len = 1;
        else if ((c & 0xE0) == 0xC0) len = 2;
        else if ((c & 0xF0) == 0xE0) len = 3;
        else if ((c & 0xF8) == 0xF0) len = 4;
    } else if (CodePage == 949 || CodePage == 936 || CodePage == 932 || CodePage == 950) {
        if ((c >= 0x81 && c <= 0xFE) && lpCurrentChar[1] != '\0') {
            len = 2;
        }
    }

    return (LPSTR)(lpCurrentChar + len);
}

static inline LPSTR CharPrevExA(DWORD CodePage, LPCSTR lpStart, LPCSTR lpCurrentChar, DWORD dwFlags) {
    (void)dwFlags;
    if (!lpStart || !lpCurrentChar || lpCurrentChar <= lpStart)
        return (LPSTR)lpStart;

    LPCSTR cur = lpStart;
    LPCSTR prev = lpStart;
    while (cur < lpCurrentChar) {
        prev = cur;
        LPCSTR next = CharNextExA(CodePage, cur, dwFlags);
        if (next <= cur || next > lpCurrentChar)
            break;
        cur = next;
    }
    return (LPSTR)prev;
}

#ifndef CharNext
#define CharNext(s) CharNextExA(CP_ACP, (s), 0)
#define CharPrev(start, cur) CharPrevExA(CP_ACP, (start), (cur), 0)
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

typedef int SOCKET;
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr* PSOCKADDR;
typedef struct sockaddr* LPSOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr_in* PSOCKADDR_IN;
typedef struct sockaddr_in* LPSOCKADDR_IN;
typedef struct in_addr IN_ADDR;
typedef struct in_addr* PIN_ADDR;
typedef struct in_addr* LPIN_ADDR;
typedef struct hostent HOSTENT;
typedef struct hostent* LPHOSTENT;

typedef struct timeval TIMEVAL;
typedef struct timeval* PTIMEVAL;
typedef struct timeval* LPTIMEVAL;

#ifdef __cplusplus
static inline ssize_t recvfrom(int fd, void *buf, size_t len, int flags, struct sockaddr *src_addr, int *addrlen) {
    socklen_t slen = addrlen ? (socklen_t)*addrlen : 0;
    ssize_t ret = ::recvfrom(fd, buf, len, flags, src_addr, addrlen ? &slen : nullptr);
    if (addrlen) *addrlen = (int)slen;
    return ret;
}
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET ((SOCKET)(~0))
#endif
#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif

#ifdef B0
#undef B0
#endif

#include <strings.h>
#include <unistd.h>

#ifndef _strnicmp
#define _strnicmp strncasecmp
#endif
#ifndef strnicmp
#define strnicmp strncasecmp
#endif
#ifndef _stricmp
#define _stricmp strcasecmp
#endif
#ifndef stricmp
#define stricmp strcasecmp
#endif

#ifndef ZeroMemory
#define ZeroMemory(Destination, Length) memset((Destination), 0, (Length))
#endif

#ifndef ERROR_SUCCESS
#define ERROR_SUCCESS 0L
#endif
#ifndef ERROR_PARTIAL_COPY
#define ERROR_PARTIAL_COPY 299L
#endif

static inline DWORD GetCurrentProcessId(void) {
    return (DWORD)getpid();
}

static inline HANDLE GetCurrentProcess(void) {
    return (HANDLE)-1;
}

static inline bool __IsPersistentPackFd(int fd);

static inline BOOL CloseHandle(HANDLE hObject) {
    if (hObject == ((HANDLE)-1) || !hObject || hObject == (HANDLE)-1 || hObject == (HANDLE)-2) 
        return FALSE;
    
    uintptr_t uval = (uintptr_t)hObject;
    // File-mapping handle created by CreateFileMapping: do not close underlying fd!
    if (uval >= 500000 && uval < 1000000) {
        return TRUE;
    }
    if (uval >= 1000 && uval < 500000) {
        int fd = (int)(uval - 1000);
        if (__IsPersistentPackFd(fd)) {
            return TRUE;
        }
        return (close(fd) == 0) ? TRUE : FALSE;
    }
    return TRUE;
}

static inline DWORD GetLastError(void) {
    return (DWORD)errno;
}

static inline void SetLastError(DWORD dwErrCode) {
    errno = (int)dwErrCode;
}

#ifdef __cplusplus
template <typename TSizePtr>
static inline BOOL ReadProcessMemory(HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, size_t nSize, TSizePtr lpNumberOfBytesRead) {
    (void)hProcess;
    (void)lpBaseAddress;
    (void)lpBuffer;
    (void)nSize;
    (void)lpNumberOfBytesRead;
    return FALSE;
}
#endif

typedef intptr_t INT_PTR, *PINT_PTR;
typedef uintptr_t UINT_PTR, *PUINT_PTR;
typedef intptr_t LONG_PTR, *PLONG_PTR;
typedef uintptr_t ULONG_PTR, *PULONG_PTR;
typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;
typedef ULONG_PTR SIZE_T, *PSIZE_T;
typedef LONG_PTR SSIZE_T, *PSSIZE_T;

#include <stdlib.h>

typedef void* PVOID;
typedef const void* PCVOID;
typedef WCHAR* PWSTR;
typedef const WCHAR* PCWSTR;
typedef CHAR* PSTR;
typedef const CHAR* PCSTR;
typedef unsigned long long ULONGLONG;
typedef long long LONGLONG;
typedef uint32_t ULONG;
typedef ULONG* PULONG;
typedef BYTE* PBYTE;
typedef BYTE* LPBYTE;
typedef WORD* PWORD;
typedef WORD* LPWORD;
typedef DWORD* PDWORD;
typedef DWORD* LPDWORD;
typedef void* HLOCAL;
typedef void* HGLOBAL;

typedef struct _IO_COUNTERS {
    ULONGLONG ReadOperationCount;
    ULONGLONG WriteOperationCount;
    ULONGLONG OtherOperationCount;
    ULONGLONG ReadTransferCount;
    ULONGLONG WriteTransferCount;
    ULONGLONG OtherTransferCount;
} IO_COUNTERS, *PIO_COUNTERS;

#ifndef LMEM_FIXED
#define LMEM_FIXED 0x0000
#endif
#ifndef LMEM_ZEROINIT
#define LMEM_ZEROINIT 0x0040
#endif
#ifndef LPTR
#define LPTR (LMEM_FIXED | LMEM_ZEROINIT)
#endif

#ifndef ERROR_INVALID_PARAMETER
#define ERROR_INVALID_PARAMETER 87L
#endif

static inline HLOCAL LocalAlloc(UINT uFlags, SIZE_T uBytes) {
    if (uFlags & LMEM_ZEROINIT) {
        return calloc(1, uBytes);
    }
    return malloc(uBytes);
}

static inline HLOCAL LocalFree(HLOCAL hMem) {
    if (hMem) free(hMem);
    return NULL;
}

static inline HMODULE GetModuleHandleA(LPCSTR lpModuleName) {
    (void)lpModuleName;
    return (HMODULE)0x400000;
}

static inline HMODULE GetModuleHandleW(LPCWSTR lpModuleName) {
    (void)lpModuleName;
    return (HMODULE)0x400000;
}

#ifndef GetModuleHandle
#define GetModuleHandle GetModuleHandleA
#endif

struct _EXCEPTION_POINTERS;
typedef LONG (*PTOP_LEVEL_EXCEPTION_FILTER)(struct _EXCEPTION_POINTERS *ExceptionInfo);
typedef PTOP_LEVEL_EXCEPTION_FILTER LPTOP_LEVEL_EXCEPTION_FILTER;

static inline LPTOP_LEVEL_EXCEPTION_FILTER SetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter) {
    (void)lpTopLevelExceptionFilter;
    return NULL;
}

typedef struct _SYSTEM_INFO {
    union {
        DWORD dwOemId;
        struct {
            WORD wProcessorArchitecture;
            WORD wReserved;
        };
    };
    DWORD     dwPageSize;
    LPVOID    lpMinimumApplicationAddress;
    LPVOID    lpMaximumApplicationAddress;
    DWORD_PTR dwActiveProcessorMask;
    DWORD     dwNumberOfProcessors;
    DWORD     dwProcessorType;
    DWORD     dwAllocationGranularity;
    WORD      wProcessorLevel;
    WORD      wProcessorRevision;
} SYSTEM_INFO, *LPSYSTEM_INFO;

static inline void GetSystemInfo(LPSYSTEM_INFO lpSystemInfo) {
    if (lpSystemInfo) {
        memset(lpSystemInfo, 0, sizeof(SYSTEM_INFO));
        lpSystemInfo->dwPageSize = 4096;
        lpSystemInfo->dwNumberOfProcessors = 4;
        lpSystemInfo->dwAllocationGranularity = 65536;
    }
}

#ifndef PROCESS_TERMINATE
#define PROCESS_TERMINATE         0x0001
#define PROCESS_CREATE_THREAD     0x0002
#define PROCESS_VM_OPERATION      0x0008
#define PROCESS_VM_READ           0x0010
#define PROCESS_VM_WRITE          0x0020
#define PROCESS_DUP_HANDLE        0x0040
#define PROCESS_QUERY_INFORMATION 0x0400
#define PROCESS_ALL_ACCESS        0x1F0FFF
#endif

static inline HANDLE OpenProcess(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId) {
    (void)dwDesiredAccess;
    (void)bInheritHandle;
    (void)dwProcessId;
    return NULL;
}

static inline void Sleep(DWORD dwMilliseconds) {
    usleep((useconds_t)dwMilliseconds * 1000);
}

#ifndef INFINITE
#define INFINITE 0xFFFFFFFF
#endif
#ifndef WAIT_OBJECT_0
#define WAIT_OBJECT_0 0x00000000L
#endif
#ifndef WAIT_TIMEOUT
#define WAIT_TIMEOUT 258L
#endif
#ifndef WAIT_FAILED
#define WAIT_FAILED ((DWORD)0xFFFFFFFF)
#endif

typedef struct _SECURITY_ATTRIBUTES {
    DWORD  nLength;
    LPVOID lpSecurityDescriptor;
    BOOL   bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

#ifdef __cplusplus
struct __AndroidSyncObject {
    int magic; // 0x45564E54 = Event, 0x53454D41 = Semaphore, 0x54485244 = Thread
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    BOOL manualReset;
    BOOL signaled;
    LONG count;
    pthread_t thread;
    bool joined;
};

static inline __AndroidSyncObject* __AllocSyncObj(int magic) {
    __AndroidSyncObject* obj = (__AndroidSyncObject*)calloc(1, sizeof(__AndroidSyncObject));
    if (!obj) return nullptr;
    obj->magic = magic;
    pthread_mutex_init(&obj->mutex, NULL);
    pthread_cond_init(&obj->cond, NULL);
    return obj;
}

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE ((HANDLE)-1)
#endif

static inline bool __IsSyncObj(HANDLE h) {
    if (!h || h == (HANDLE)-1 || h == (HANDLE)-2 || (uintptr_t)h < 1000000u) return false;
    int m = ((__AndroidSyncObject*)h)->magic;
    return (m == 0x45564E54 || m == 0x53454D41 || m == 0x54485244);
}
#endif

static inline HANDLE CreateEventA(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCSTR lpName) {
    (void)lpEventAttributes; (void)lpName;
#ifdef __cplusplus
    __AndroidSyncObject* ev = __AllocSyncObj(0x45564E54);
    if (!ev) return (HANDLE)1;
    ev->manualReset = bManualReset;
    ev->signaled = bInitialState;
    return (HANDLE)ev;
#else
    (void)bManualReset; (void)bInitialState;
    return (HANDLE)1;
#endif
}

#ifndef CreateEvent
#define CreateEvent CreateEventA
#endif

static inline BOOL SetEvent(HANDLE hEvent) {
#ifdef __cplusplus
    if (__IsSyncObj(hEvent)) {
        __AndroidSyncObject* ev = (__AndroidSyncObject*)hEvent;
        pthread_mutex_lock(&ev->mutex);
        ev->signaled = TRUE;
        if (ev->manualReset) pthread_cond_broadcast(&ev->cond);
        else pthread_cond_signal(&ev->cond);
        pthread_mutex_unlock(&ev->mutex);
    }
#else
    (void)hEvent;
#endif
    return TRUE;
}

static inline BOOL ResetEvent(HANDLE hEvent) {
#ifdef __cplusplus
    if (__IsSyncObj(hEvent)) {
        __AndroidSyncObject* ev = (__AndroidSyncObject*)hEvent;
        pthread_mutex_lock(&ev->mutex);
        ev->signaled = FALSE;
        pthread_mutex_unlock(&ev->mutex);
    }
#else
    (void)hEvent;
#endif
    return TRUE;
}

static inline DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds) {
#ifdef __cplusplus
    if (!__IsSyncObj(hHandle)) return WAIT_OBJECT_0;
    __AndroidSyncObject* obj = (__AndroidSyncObject*)hHandle;
    if (obj->magic == 0x54485244) {
        if (!obj->joined && obj->thread) {
            obj->joined = true;
            pthread_join(obj->thread, NULL);
        }
        return WAIT_OBJECT_0;
    }
    pthread_mutex_lock(&obj->mutex);
    struct timespec ts;
    if (dwMilliseconds != INFINITE) {
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += dwMilliseconds / 1000;
        ts.tv_nsec += (dwMilliseconds % 1000) * 1000000L;
        if (ts.tv_nsec >= 1000000000L) {
            ts.tv_sec += 1;
            ts.tv_nsec -= 1000000000L;
        }
    }
    DWORD ret = WAIT_OBJECT_0;
    if (obj->magic == 0x45564E54) {
        while (!obj->signaled) {
            if (dwMilliseconds == 0) { ret = WAIT_TIMEOUT; break; }
            if (dwMilliseconds == INFINITE) {
                pthread_cond_wait(&obj->cond, &obj->mutex);
            } else if (pthread_cond_timedwait(&obj->cond, &obj->mutex, &ts) == ETIMEDOUT) {
                ret = WAIT_TIMEOUT;
                break;
            }
        }
        if (ret == WAIT_OBJECT_0 && !obj->manualReset) obj->signaled = FALSE;
    } else if (obj->magic == 0x53454D41) {
        while (obj->count <= 0) {
            if (dwMilliseconds == 0) { ret = WAIT_TIMEOUT; break; }
            if (dwMilliseconds == INFINITE) {
                pthread_cond_wait(&obj->cond, &obj->mutex);
            } else if (pthread_cond_timedwait(&obj->cond, &obj->mutex, &ts) == ETIMEDOUT) {
                ret = WAIT_TIMEOUT;
                break;
            }
        }
        if (ret == WAIT_OBJECT_0 && obj->count > 0) --obj->count;
    }
    pthread_mutex_unlock(&obj->mutex);
    return ret;
#else
    (void)hHandle; (void)dwMilliseconds;
    return WAIT_OBJECT_0;
#endif
}

#ifndef THREAD_PRIORITY_LOWEST
#define THREAD_PRIORITY_IDLE          (-15)
#define THREAD_PRIORITY_LOWEST        (-2)
#define THREAD_PRIORITY_BELOW_NORMAL  (-1)
#define THREAD_PRIORITY_NORMAL        0
#define THREAD_PRIORITY_ABOVE_NORMAL  1
#define THREAD_PRIORITY_HIGHEST       2
#define THREAD_PRIORITY_TIME_CRITICAL 15
#endif

typedef unsigned (__stdcall *PTHREAD_START) (void *);

static inline uintptr_t _beginthreadex(
    void *security,
    unsigned stack_size,
    PTHREAD_START start_address,
    void *arglist,
    unsigned initflag,
    unsigned *thrdaddr
) {
    (void)security; (void)stack_size; (void)initflag;
    pthread_t thread;
    int result = pthread_create(&thread, NULL, (void* (*)(void*))(start_address), arglist);
    if (result != 0) return 0;
    if (thrdaddr) *thrdaddr = (unsigned)(uintptr_t)thread;
#ifdef __cplusplus
    __AndroidSyncObject* tobj = __AllocSyncObj(0x54485244);
    if (tobj) {
        tobj->thread = thread;
        tobj->joined = false;
        return (uintptr_t)tobj;
    }
#endif
    return (uintptr_t)thread;
}

static inline uintptr_t _beginthread(void (*start_address)(void *), unsigned stack_size, void *arglist) {
    (void)start_address;
    (void)stack_size;
    (void)arglist;
    return (uintptr_t)1;
}

static inline void _endthread(void) {}

static inline BOOL SetThreadPriority(HANDLE hThread, int nPriority) {
    (void)hThread;
    (void)nPriority;
    return TRUE;
}

static inline HANDLE GetCurrentThread(void) {
    return (HANDLE)-2;
}

static inline HANDLE CreateSemaphoreA(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes, LONG lInitialCount, LONG lMaximumCount, LPCSTR lpName) {
    (void)lpSemaphoreAttributes; (void)lMaximumCount; (void)lpName;
#ifdef __cplusplus
    __AndroidSyncObject* sem = __AllocSyncObj(0x53454D41);
    if (!sem) return (HANDLE)1;
    sem->count = lInitialCount;
    return (HANDLE)sem;
#else
    (void)lInitialCount;
    return (HANDLE)1;
#endif
}
#ifndef CreateSemaphore
#define CreateSemaphore CreateSemaphoreA
#endif

static inline BOOL ReleaseSemaphore(HANDLE hSemaphore, LONG lReleaseCount, LONG* lpPreviousCount) {
#ifdef __cplusplus
    if (__IsSyncObj(hSemaphore)) {
        __AndroidSyncObject* sem = (__AndroidSyncObject*)hSemaphore;
        pthread_mutex_lock(&sem->mutex);
        if (lpPreviousCount) *lpPreviousCount = sem->count;
        sem->count += (lReleaseCount > 0 ? lReleaseCount : 1);
        pthread_cond_broadcast(&sem->cond);
        pthread_mutex_unlock(&sem->mutex);
        return TRUE;
    }
#else
    (void)hSemaphore; (void)lReleaseCount;
#endif
    if (lpPreviousCount) *lpPreviousCount = 0;
    return TRUE;
}

// Pointer aliases for common Win32 structs
typedef POINT* LPPOINT;
typedef const POINT* LPCPOINT;
typedef RECT* LPRECT;
typedef const RECT* LPCRECT;

// Common Win32 primitive pointer aliases
typedef const void* LPCVOID;
typedef char TCHAR;
typedef char* LPTSTR;
typedef const char* LPCTSTR;

#ifndef TEXT
#define TEXT(quote) quote
#endif
#ifndef _T
#define _T(quote) quote
#endif

// Calling conventions
#ifndef WINAPI
#define WINAPI
#endif
#ifndef CALLBACK
#define CALLBACK
#endif
#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef __stdcall
#define __stdcall
#endif
#ifndef __cdecl
#define __cdecl
#endif
#ifndef __fastcall
#define __fastcall
#endif
#ifndef __in
#define __in
#endif
#ifndef __out
#define __out
#endif
#ifndef __inout
#define __inout
#endif

// Winsock compatibility macros & functions
#ifndef closesocket
#define closesocket close
#endif
#ifndef ioctlsocket
#define ioctlsocket ioctl
#endif
#ifndef SD_RECEIVE
#define SD_RECEIVE SHUT_RD
#define SD_SEND    SHUT_WR
#define SD_BOTH    SHUT_RDWR
#endif
#ifndef WSAEWOULDBLOCK
#define WSAEWOULDBLOCK EWOULDBLOCK
#define WSAEINPROGRESS EINPROGRESS
#define WSAECONNRESET  ECONNRESET
#define WSAENETRESET   ENETRESET
#define WSAENOTCONN    ENOTCONN
#define WSAETIMEDOUT   ETIMEDOUT
#endif

#ifndef WSADESCRIPTION_LEN
#define WSADESCRIPTION_LEN 256
#define WSASYS_STATUS_LEN  128
typedef struct WSAData {
    WORD wVersion;
    WORD wHighVersion;
    char szDescription[WSADESCRIPTION_LEN + 1];
    char szSystemStatus[WSASYS_STATUS_LEN + 1];
    unsigned short iMaxSockets;
    unsigned short iMaxUdpDg;
    char* lpVendorInfo;
} WSADATA, *LPWSADATA;

static inline int WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData) {
    (void)wVersionRequested;
    if (lpWSAData) memset(lpWSAData, 0, sizeof(WSADATA));
    return 0;
}
static inline int WSACleanup(void) { return 0; }
static inline int WSAGetLastError(void) {
    int err = errno;
    if (err == EINPROGRESS || err == EALREADY || err == EAGAIN)
        return WSAEWOULDBLOCK;
    return err;
}
static inline void WSASetLastError(int iError) { errno = iError; }
#endif

// Virtual memory flags & stubs
#ifndef MEM_COMMIT
#define MEM_COMMIT  0x00001000
#define MEM_RESERVE 0x00002000
#define MEM_RELEASE 0x00008000
#define MEM_FREE    0x00010000
#define MEM_PRIVATE 0x00020000
#define MEM_Mapped  0x00040000
#define MEM_IMAGE   0x01000000
#endif

#ifndef PAGE_NOACCESS
#define PAGE_NOACCESS          0x01
#define PAGE_READONLY          0x02
#define PAGE_READWRITE         0x04
#define PAGE_WRITECOPY         0x08
#define PAGE_EXECUTE           0x10
#define PAGE_EXECUTE_READ      0x20
#define PAGE_EXECUTE_READWRITE 0x40
#define PAGE_GUARD             0x100
#define PAGE_NOCACHE           0x200
#endif

// File & Directory constants
#ifndef INVALID_FILE_SIZE
#define INVALID_FILE_SIZE ((DWORD)0xFFFFFFFF)
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#endif

#ifndef FILE_ATTRIBUTE_READONLY
#define FILE_ATTRIBUTE_READONLY  0x00000001
#define FILE_ATTRIBUTE_HIDDEN    0x00000002
#define FILE_ATTRIBUTE_SYSTEM    0x00000004
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010
#define FILE_ATTRIBUTE_ARCHIVE   0x00000020
#define FILE_ATTRIBUTE_NORMAL    0x00000080
#endif

#ifndef GENERIC_READ
#define GENERIC_READ    0x80000000L
#define GENERIC_WRITE   0x40000000L
#define GENERIC_EXECUTE 0x20000000L
#define GENERIC_ALL     0x10000000L
#endif

#ifndef FILE_SHARE_READ
#define FILE_SHARE_READ   0x00000001
#define FILE_SHARE_WRITE  0x00000002
#define FILE_SHARE_DELETE 0x00000004
#endif

#ifndef CREATE_NEW
#define CREATE_NEW        1
#define CREATE_ALWAYS     2
#define OPEN_EXISTING     3
#define OPEN_ALWAYS       4
#define TRUNCATE_EXISTING 5
#endif

#ifndef FILE_BEGIN
#define FILE_BEGIN   0
#define FILE_CURRENT 1
#define FILE_END     2
#endif

// MessageBox flags
#ifndef MB_OK
#define MB_OK              0x00000000L
#define MB_OKCANCEL        0x00000001L
#define MB_ABORTRETRYIGNORE 0x00000002L
#define MB_YESNOCANCEL     0x00000003L
#define MB_YESNO           0x00000004L
#define MB_RETRYCANCEL     0x00000005L
#define MB_ICONHAND        0x00000010L
#define MB_ICONQUESTION    0x00000020L
#define MB_ICONEXCLAMATION 0x00000030L
#define MB_ICONASTERISK    0x00000040L
#define MB_ICONWARNING     MB_ICONEXCLAMATION
#define MB_ICONERROR       MB_ICONHAND
#define MB_ICONINFORMATION MB_ICONASTERISK
#define MB_ICONSTOP        MB_ICONHAND
#endif

#ifndef IDOK
#define IDOK     1
#define IDCANCEL 2
#define IDABORT  3
#define IDRETRY  4
#define IDIGNORE 5
#define IDYES    6
#define IDNO     7
#endif

// ShowWindow commands
#ifndef SW_HIDE
#define SW_HIDE            0
#define SW_SHOWNORMAL      1
#define SW_NORMAL          1
#define SW_SHOWMINIMIZED   2
#define SW_SHOWMAXIMIZED   3
#define SW_MAXIMIZE        3
#define SW_SHOWNOACTIVATE  4
#define SW_SHOW            5
#define SW_MINIMIZE        6
#define SW_SHOWMINNOACTIVE 7
#define SW_SHOWNA          8
#define SW_RESTORE         9
#define SW_SHOWDEFAULT     10
#endif

// Common Virtual Key codes
#ifndef VK_LBUTTON
#define VK_LBUTTON  0x01
#define VK_RBUTTON  0x02
#define VK_CANCEL   0x03
#define VK_MBUTTON  0x04
#define VK_BACK     0x08
#define VK_TAB      0x09
#define VK_CLEAR    0x0C
#define VK_RETURN   0x0D
#define VK_SHIFT    0x10
#define VK_CONTROL  0x11
#define VK_MENU     0x12
#define VK_PAUSE    0x13
#define VK_CAPITAL  0x14
#define VK_ESCAPE   0x1B
#define VK_SPACE    0x20
#define VK_PRIOR    0x21
#define VK_NEXT     0x22
#define VK_END      0x23
#define VK_HOME     0x24
#define VK_LEFT     0x25
#define VK_UP       0x26
#define VK_RIGHT    0x27
#define VK_DOWN     0x28
#define VK_SELECT   0x29
#define VK_PRINT    0x2A
#define VK_SNAPSHOT 0x2C
#define VK_INSERT   0x2D
#define VK_DELETE   0x2E
#define VK_HELP     0x2F
#define VK_LWIN     0x5B
#define VK_RWIN     0x5C
#define VK_NUMPAD0  0x60
#define VK_NUMPAD1  0x61
#define VK_NUMPAD2  0x62
#define VK_NUMPAD3  0x63
#define VK_NUMPAD4  0x64
#define VK_NUMPAD5  0x65
#define VK_NUMPAD6  0x66
#define VK_NUMPAD7  0x67
#define VK_NUMPAD8  0x68
#define VK_NUMPAD9  0x69
#define VK_MULTIPLY 0x6A
#define VK_ADD      0x6B
#define VK_SEPARATOR 0x6C
#define VK_SUBTRACT 0x6D
#define VK_DECIMAL  0x6E
#define VK_DIVIDE   0x6F
#define VK_F1       0x70
#define VK_F2       0x71
#define VK_F3       0x72
#define VK_F4       0x73
#define VK_F5       0x74
#define VK_F6       0x75
#define VK_F7       0x76
#define VK_F8       0x77
#define VK_F9       0x78
#define VK_F10      0x79
#define VK_F11      0x7A
#define VK_F12      0x7B
#define VK_NUMLOCK  0x90
#define VK_SCROLL   0x91
#define VK_LSHIFT   0xA0
#define VK_RSHIFT   0xA1
#define VK_LCONTROL 0xA2
#define VK_RCONTROL 0xA3
#define VK_LMENU    0xA4
#define VK_RMENU    0xA5
#endif

#include <time.h>
#include <sys/time.h>

// Additional pointer aliases for RECT and SIZE
typedef const RECT* PCRECT;
typedef SIZE* LPSIZE;
typedef const SIZE* PCSIZE;
typedef const SIZE* LPCSIZE;

// Standard Win32 bit-manipulation macros
#ifndef LOWORD
#define LOWORD(l) ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#endif
#ifndef HIWORD
#define HIWORD(l) ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
#endif
#ifndef LOBYTE
#define LOBYTE(w) ((BYTE)(((DWORD_PTR)(w)) & 0xff))
#endif
#ifndef HIBYTE
#define HIBYTE(w) ((BYTE)((((DWORD_PTR)(w)) >> 8) & 0xff))
#endif
#ifndef MAKELONG
#define MAKELONG(a, b) ((LONG)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))
#endif
#ifndef MAKEWORD
#define MAKEWORD(a, b) ((WORD)(((BYTE)(((DWORD_PTR)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD_PTR)(b)) & 0xff))) << 8))
#endif
#ifndef MAKELPARAM
#define MAKELPARAM(l, h) ((LPARAM)(DWORD)MAKELONG(l, h))
#endif
#ifndef MAKEWPARAM
#define MAKEWPARAM(l, h) ((WPARAM)(DWORD)MAKELONG(l, h))
#endif
#ifndef MAKELRESULT
#define MAKELRESULT(l, h) ((LRESULT)(DWORD)MAKELONG(l, h))
#endif
#ifndef MAKEINTRESOURCEA
#define MAKEINTRESOURCEA(i) ((LPSTR)((ULONG_PTR)((WORD)(i))))
#define MAKEINTRESOURCE  MAKEINTRESOURCEA
#endif

// 64-bit integer unions & timing structs
typedef union _LARGE_INTEGER {
    struct {
        DWORD LowPart;
        LONG  HighPart;
    };
    struct {
        DWORD LowPart;
        LONG  HighPart;
    } u;
    LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef union _ULARGE_INTEGER {
    struct {
        DWORD LowPart;
        DWORD HighPart;
    };
    struct {
        DWORD LowPart;
        DWORD HighPart;
    } u;
    ULONGLONG QuadPart;
} ULARGE_INTEGER, *PULARGE_INTEGER;

typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;

typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

// Win32 GetTickCount / GetCurrentTime compatibility stub
static inline DWORD GetTickCount(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (DWORD)((ts.tv_sec * 1000ULL) + (ts.tv_nsec / 1000000ULL));
}

#ifndef GetCurrentTime
#define GetCurrentTime GetTickCount
#endif

static inline ULONGLONG GetTickCount64(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ULONGLONG)((ts.tv_sec * 1000ULL) + (ts.tv_nsec / 1000000ULL));
}

static inline BOOL QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount) {
    if (lpPerformanceCount) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        lpPerformanceCount->QuadPart = (LONGLONG)ts.tv_sec * 1000000000LL + ts.tv_nsec;
    }
    return TRUE;
}

static inline BOOL QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency) {
    if (lpFrequency) {
        lpFrequency->QuadPart = 1000000000LL;
    }
    return TRUE;
}

static inline void GetLocalTime(LPSYSTEMTIME lpSystemTime) {
    if (!lpSystemTime) return;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm tm_info;
    localtime_r(&tv.tv_sec, &tm_info);
    lpSystemTime->wYear = (WORD)(tm_info.tm_year + 1900);
    lpSystemTime->wMonth = (WORD)(tm_info.tm_mon + 1);
    lpSystemTime->wDayOfWeek = (WORD)tm_info.tm_wday;
    lpSystemTime->wDay = (WORD)tm_info.tm_mday;
    lpSystemTime->wHour = (WORD)tm_info.tm_hour;
    lpSystemTime->wMinute = (WORD)tm_info.tm_min;
    lpSystemTime->wSecond = (WORD)tm_info.tm_sec;
    lpSystemTime->wMilliseconds = (WORD)(tv.tv_usec / 1000);
}

static inline void GetSystemTime(LPSYSTEMTIME lpSystemTime) {
    GetLocalTime(lpSystemTime);
}

// Memory & Process Query structs
typedef struct _MEMORY_BASIC_INFORMATION {
    PVOID  BaseAddress;
    PVOID  AllocationBase;
    DWORD  AllocationProtect;
    SIZE_T RegionSize;
    DWORD  State;
    DWORD  Protect;
    DWORD  Type;
} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;

static inline SIZE_T VirtualQuery(LPCVOID lpAddress, PMEMORY_BASIC_INFORMATION lpBuffer, SIZE_T dwLength) {
    (void)lpAddress;
    if (lpBuffer && dwLength >= sizeof(MEMORY_BASIC_INFORMATION)) {
        memset(lpBuffer, 0, sizeof(MEMORY_BASIC_INFORMATION));
        return sizeof(MEMORY_BASIC_INFORMATION);
    }
    return 0;
}

static inline LPVOID VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect) {
    (void)lpAddress; (void)flAllocationType; (void)flProtect;
    return calloc(1, dwSize);
}

static inline BOOL VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType) {
    (void)dwSize; (void)dwFreeType;
    if (lpAddress) free(lpAddress);
    return TRUE;
}

static inline BOOL VirtualProtect(LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect) {
    (void)lpAddress; (void)dwSize; (void)flNewProtect;
    if (lpflOldProtect) *lpflOldProtect = PAGE_READWRITE;
    return TRUE;
}

// Dynamic library & handle helpers
typedef int (*FARPROC)(void);

static inline HMODULE LoadLibraryA(LPCSTR lpLibFileName) {
    (void)lpLibFileName;
    return NULL;
}
static inline HMODULE LoadLibraryW(LPCWSTR lpLibFileName) {
    (void)lpLibFileName;
    return NULL;
}
#ifndef LoadLibrary
#define LoadLibrary LoadLibraryA
#endif

static inline BOOL FreeLibrary(HMODULE hLibModule) {
    (void)hLibModule;
    return TRUE;
}

static inline FARPROC GetProcAddress(HMODULE hModule, LPCSTR lpProcName) {
    (void)hModule; (void)lpProcName;
    return NULL;
}

static inline BOOL DuplicateHandle(HANDLE hSourceProcessHandle, HANDLE hSourceHandle, HANDLE hTargetProcessHandle, HANDLE* lpTargetHandle, DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwOptions) {
    (void)hSourceProcessHandle; (void)hTargetProcessHandle; (void)dwDesiredAccess; (void)bInheritHandle; (void)dwOptions;
    if (lpTargetHandle) *lpTargetHandle = hSourceHandle;
    return TRUE;
}

#ifndef THREAD_QUERY_INFORMATION
#define THREAD_QUERY_INFORMATION 0x0040
#endif

// Window Message struct & common WM_* constants
typedef struct tagMSG {
    HWND   hwnd;
    UINT   message;
    WPARAM wParam;
    LPARAM lParam;
    DWORD  time;
    POINT  pt;
} MSG, *PMSG, *LPMSG;

typedef LRESULT (CALLBACK *WNDPROC)(HWND, UINT, WPARAM, LPARAM);

typedef struct tagWNDCLASSA {
    UINT      style;
    WNDPROC   lpfnWndProc;
    int       cbClsExtra;
    int       cbWndExtra;
    HINSTANCE hInstance;
    HICON     hIcon;
    HCURSOR   hCursor;
    HBRUSH    hbrBackground;
    LPCSTR    lpszMenuName;
    LPCSTR    lpszClassName;
} WNDCLASSA, WNDCLASS, *PWNDCLASSA, *LPWNDCLASSA;

typedef struct tagWNDCLASSEXA {
    UINT      cbSize;
    UINT      style;
    WNDPROC   lpfnWndProc;
    int       cbClsExtra;
    int       cbWndExtra;
    HINSTANCE hInstance;
    HICON     hIcon;
    HCURSOR   hCursor;
    HBRUSH    hbrBackground;
    LPCSTR    lpszMenuName;
    LPCSTR    lpszClassName;
    HICON     hIconSm;
} WNDCLASSEXA, WNDCLASSEX, *PWNDCLASSEXA, *LPWNDCLASSEXA;

#ifndef WM_NULL
#define WM_NULL             0x0000
#define WM_CREATE           0x0001
#define WM_DESTROY          0x0002
#define WM_MOVE             0x0003
#define WM_SIZE             0x0005
#define WM_ACTIVATE         0x0006
#define WM_SETFOCUS         0x0007
#define WM_KILLFOCUS        0x0008
#define WM_ENABLE           0x000A
#define WM_PAINT            0x000F
#define WM_CLOSE            0x0010
#define WM_QUIT             0x0012
#define WM_ERASEBKGND       0x0014
#define WM_SHOWWINDOW       0x0018
#define WM_ACTIVATEAPP      0x001C
#define WM_SETCURSOR        0x0020
#define WM_MOUSEACTIVATE    0x0021
#define WM_WINDOWPOSCHANGING 0x0046
#define WM_WINDOWPOSCHANGED 0x0047
#define WM_NCCREATE         0x0081
#define WM_NCDESTROY        0x0082
#define WM_NCHITTEST        0x0084
#define WM_KEYDOWN          0x0100
#define WM_KEYUP            0x0101
#define WM_CHAR             0x0102
#define WM_SYSKEYDOWN       0x0104
#define WM_SYSKEYUP         0x0105
#define WM_SYSCHAR          0x0106
#define WM_IME_STARTCOMPOSITION 0x010D
#define WM_IME_ENDCOMPOSITION   0x010E
#define WM_IME_COMPOSITION      0x010F
#define WM_COMMAND          0x0111
#define WM_SYSCOMMAND       0x0112
#define WM_TIMER            0x0113
#define WM_MOUSEMOVE        0x0200
#define WM_LBUTTONDOWN      0x0201
#define WM_LBUTTONUP        0x0202
#define WM_LBUTTONDBLCLK    0x0203
#define WM_RBUTTONDOWN      0x0204
#define WM_RBUTTONUP        0x0205
#define WM_RBUTTONDBLCLK    0x0206
#define WM_MBUTTONDOWN      0x0207
#define WM_MBUTTONUP        0x0208
#define WM_MBUTTONDBLCLK    0x0209
#define WM_MOUSEWHEEL       0x020A
#define WM_IME_SETCONTEXT   0x0281
#define WM_IME_NOTIFY       0x0282
#define WM_IME_CHAR         0x0286
#define WM_USER             0x0400
#define WM_APP              0x8000
#endif

// Rect helpers
static inline BOOL SetRect(LPRECT lprc, int xLeft, int yTop, int xRight, int yBottom) {
    if (!lprc) return FALSE;
    lprc->left = xLeft; lprc->top = yTop; lprc->right = xRight; lprc->bottom = yBottom;
    return TRUE;
}

static inline BOOL PtInRect(const RECT *lprc, POINT pt) {
    if (!lprc) return FALSE;
    return (pt.x >= lprc->left && pt.x < lprc->right && pt.y >= lprc->top && pt.y < lprc->bottom) ? TRUE : FALSE;
}

static inline BOOL OffsetRect(LPRECT lprc, int dx, int dy) {
    if (!lprc) return FALSE;
    lprc->left += dx; lprc->right += dx; lprc->top += dy; lprc->bottom += dy;
    return TRUE;
}

static inline BOOL IntersectRect(LPRECT lprcDst, const RECT *lprcSrc1, const RECT *lprcSrc2) {
    if (!lprcDst || !lprcSrc1 || !lprcSrc2) return FALSE;
    lprcDst->left   = (lprcSrc1->left > lprcSrc2->left) ? lprcSrc1->left : lprcSrc2->left;
    lprcDst->top    = (lprcSrc1->top > lprcSrc2->top) ? lprcSrc1->top : lprcSrc2->top;
    lprcDst->right  = (lprcSrc1->right < lprcSrc2->right) ? lprcSrc1->right : lprcSrc2->right;
    lprcDst->bottom = (lprcSrc1->bottom < lprcSrc2->bottom) ? lprcSrc1->bottom : lprcSrc2->bottom;
    if (lprcDst->left >= lprcDst->right || lprcDst->top >= lprcDst->bottom) {
        memset(lprcDst, 0, sizeof(RECT));
        return FALSE;
    }
    return TRUE;
}

#include <stdio.h>

// Common Window / Input / Debug stubs
static inline int MessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) {
    (void)hWnd; (void)uType;
    fprintf(stderr, "\n[MessageBox] %s:\n%s\n", lpCaption ? lpCaption : "Info", lpText ? lpText : "");
    fflush(stderr);
    return IDOK;
}
#ifndef MessageBox
#define MessageBox MessageBoxA
#endif

static inline void OutputDebugStringA(LPCSTR lpOutputString) { (void)lpOutputString; }
static inline void OutputDebugStringW(LPCWSTR lpOutputString) { (void)lpOutputString; }
#ifndef OutputDebugString
#define OutputDebugString OutputDebugStringA
#endif

static inline BOOL IsBadReadPtr(const void* lp, UINT_PTR ucb) { (void)ucb; return (lp == NULL) ? TRUE : FALSE; }
static inline BOOL IsBadWritePtr(LPVOID lp, UINT_PTR ucb) { (void)ucb; return (lp == NULL) ? TRUE : FALSE; }
static inline BOOL IsBadStringPtrA(LPCSTR lpsz, UINT_PTR ucchMax) { (void)ucchMax; return (lpsz == NULL) ? TRUE : FALSE; }
#ifndef IsBadStringPtr
#define IsBadStringPtr IsBadStringPtrA
#endif

typedef int16_t SHORT;
typedef SHORT* PSHORT;
typedef uint16_t USHORT;
typedef USHORT* PUSHORT;
typedef uint8_t UCHAR;
typedef UCHAR* PUCHAR;

static inline SHORT GetAsyncKeyState(int vKey) { (void)vKey; return 0; }
static inline SHORT GetKeyState(int nVirtKey) { (void)nVirtKey; return 0; }
extern "C" int g_AndroidMouseX;
extern "C" int g_AndroidMouseY;
static inline BOOL GetCursorPos(LPPOINT lpPoint) { if (lpPoint) { lpPoint->x = g_AndroidMouseX; lpPoint->y = g_AndroidMouseY; } return TRUE; }
static inline BOOL SetCursorPos(int X, int Y) { g_AndroidMouseX = X; g_AndroidMouseY = Y; return TRUE; }
static inline BOOL ScreenToClient(HWND hWnd, LPPOINT lpPoint) { (void)hWnd; (void)lpPoint; return TRUE; }
static inline BOOL ClientToScreen(HWND hWnd, LPPOINT lpPoint) { (void)hWnd; (void)lpPoint; return TRUE; }
extern "C" int g_AndroidScreenWidth;
extern "C" int g_AndroidScreenHeight;
static inline BOOL GetClientRect(HWND hWnd, LPRECT lpRect) {
    (void)hWnd;
    if (lpRect) {
        lpRect->left = 0;
        lpRect->top = 0;
        lpRect->right = (g_AndroidScreenWidth > 0) ? g_AndroidScreenWidth : 1280;
        lpRect->bottom = (g_AndroidScreenHeight > 0) ? g_AndroidScreenHeight : 720;
    }
    return TRUE;
}
static inline BOOL GetWindowRect(HWND hWnd, LPRECT lpRect) { return GetClientRect(hWnd, lpRect); }

#ifndef sys_err
#define sys_err TraceError
#endif

#ifndef sys_log
#define sys_log(...) ((void)0)
#endif

#ifdef __cplusplus
#include <algorithm>
#include <type_traits>

using std::min;
using std::max;

template <typename T1, typename T2,
          typename std::enable_if<!std::is_same<T1, T2>::value, int>::type = 0>
inline constexpr typename std::common_type<T1, T2>::type min(const T1& a, const T2& b) {
    return (a < b) ? a : b;
}

template <typename T1, typename T2,
          typename std::enable_if<!std::is_same<T1, T2>::value, int>::type = 0>
inline constexpr typename std::common_type<T1, T2>::type max(const T1& a, const T2& b) {
    return (a > b) ? a : b;
}
#endif

#include <sys/stat.h>

#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#ifndef _MAX_PATH
#define _MAX_PATH 260
#define _MAX_DRIVE 3
#define _MAX_DIR 256
#define _MAX_FNAME 256
#define _MAX_EXT 256
#endif

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE ((HANDLE)-1)
#endif
#ifndef ERROR_FILE_NOT_FOUND
#define ERROR_FILE_NOT_FOUND 2L
#endif
#ifndef ERROR_PATH_NOT_FOUND
#define ERROR_PATH_NOT_FOUND 3L
#endif
#ifndef ERROR_ALREADY_EXISTS
#define ERROR_ALREADY_EXISTS 183L
#endif

static inline BOOL CreateDirectoryA(LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
    (void)lpSecurityAttributes;
    if (!lpPathName || !*lpPathName) return FALSE;
    char normPath[1024];
    strncpy(normPath, lpPathName, sizeof(normPath) - 1);
    normPath[sizeof(normPath) - 1] = '\0';
    for (char* p = normPath; *p; ++p) {
        if (*p == '\\') *p = '/';
    }
    if (mkdir(normPath, 0777) == 0) return TRUE;
    if (errno == EEXIST) errno = ERROR_ALREADY_EXISTS;
    return FALSE;
}
#ifndef CreateDirectory
#define CreateDirectory CreateDirectoryA
#endif

static inline BOOL RemoveDirectoryA(LPCSTR lpPathName) {
    if (!lpPathName) return FALSE;
    return (rmdir(lpPathName) == 0) ? TRUE : FALSE;
}
#ifndef RemoveDirectory
#define RemoveDirectory RemoveDirectoryA
#endif

static inline BOOL DeleteFileA(LPCSTR lpFileName) {
    if (!lpFileName) return FALSE;
    return (unlink(lpFileName) == 0) ? TRUE : FALSE;
}
#ifndef DeleteFile
#define DeleteFile DeleteFileA
#endif

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

static inline BOOL CopyFileA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, BOOL bFailIfExists) {
    if (!lpExistingFileName || !lpNewFileName) return FALSE;
    if (bFailIfExists && access(lpNewFileName, F_OK) != -1) return FALSE;
    
    int fdSrc = open(lpExistingFileName, O_RDONLY);
    if (fdSrc < 0) return FALSE;
    
    int fdDst = open(lpNewFileName, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fdDst < 0) {
        close(fdSrc);
        return FALSE;
    }
    
    char buf[4096];
    ssize_t n;
    BOOL success = TRUE;
    while ((n = read(fdSrc, buf, sizeof(buf))) > 0) {
        if (write(fdDst, buf, n) != n) {
            success = FALSE;
            break;
        }
    }
    if (n < 0) success = FALSE;
    
    close(fdSrc);
    close(fdDst);
    return success;
}

#ifndef CopyFile
#define CopyFile CopyFileA
#endif

static inline BOOL MoveFileA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName) {
    if (!lpExistingFileName || !lpNewFileName) return FALSE;
    return (rename(lpExistingFileName, lpNewFileName) == 0) ? TRUE : FALSE;
}
#ifndef MoveFile
#define MoveFile MoveFileA
#endif

static inline DWORD GetFileAttributesA(LPCSTR lpFileName) {
    if (!lpFileName || !*lpFileName) return INVALID_FILE_ATTRIBUTES;
    char normPath[1024];
    strncpy(normPath, lpFileName, sizeof(normPath) - 1);
    normPath[sizeof(normPath) - 1] = '\0';
    for (char* p = normPath; *p; ++p) {
        if (*p == '\\') *p = '/';
    }
    struct stat st;
    if (stat(normPath, &st) != 0) return INVALID_FILE_ATTRIBUTES;
    return S_ISDIR(st.st_mode) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
}
#ifndef GetFileAttributes
#define GetFileAttributes GetFileAttributesA
#endif

static inline BOOL SetFileAttributesA(LPCSTR lpFileName, DWORD dwFileAttributes) {
    (void)lpFileName; (void)dwFileAttributes;
    return TRUE;
}
#ifndef SetFileAttributes
#define SetFileAttributes SetFileAttributesA
#endif

static inline DWORD GetCurrentDirectoryA(DWORD nBufferLength, LPSTR lpBuffer) {
    if (!lpBuffer || nBufferLength == 0) return 0;
    if (getcwd(lpBuffer, (size_t)nBufferLength) != NULL) {
        return (DWORD)strlen(lpBuffer);
    }
    return 0;
}
#ifndef GetCurrentDirectory
#define GetCurrentDirectory GetCurrentDirectoryA
#endif

static inline BOOL SetCurrentDirectoryA(LPCSTR lpPathName) {
    if (!lpPathName) return FALSE;
    return (chdir(lpPathName) == 0) ? TRUE : FALSE;
}
#ifndef SetCurrentDirectory
#define SetCurrentDirectory SetCurrentDirectoryA
#endif

// GDI Stock Objects
#ifndef WHITE_BRUSH
#define WHITE_BRUSH         0
#define LTGRAY_BRUSH        1
#define GRAY_BRUSH          2
#define DKGRAY_BRUSH        3
#define BLACK_BRUSH         4
#define NULL_BRUSH          5
#define HOLLOW_BRUSH        NULL_BRUSH
#define WHITE_PEN           6
#define BLACK_PEN           7
#define NULL_PEN            8
#define OEM_FIXED_FONT      10
#define ANSI_FIXED_FONT     11
#define ANSI_VAR_FONT       12
#define SYSTEM_FONT         13
#define DEVICE_DEFAULT_FONT 14
#define DEFAULT_PALETTE     15
#define SYSTEM_FIXED_FONT   16
#define DEFAULT_GUI_FONT    17
#define DC_BRUSH            18
#define DC_PEN              19
#endif

static inline HGDIOBJ GetStockObject(int i) {
    return (HGDIOBJ)(intptr_t)(i + 1);
}

static inline HGDIOBJ SelectObject(HDC hdc, HGDIOBJ h) {
    (void)hdc;
    return h;
}

static inline BOOL DeleteObject(HGDIOBJ ho) {
    (void)ho;
    return TRUE;
}

// Window Styles (WS_*)
#ifndef WS_OVERLAPPED
#define WS_OVERLAPPED       0x00000000L
#define WS_POPUP            0x80000000L
#define WS_CHILD            0x40000000L
#define WS_MINIMIZE         0x20000000L
#define WS_VISIBLE          0x10000000L
#define WS_DISABLED         0x08000000L
#define WS_CLIPSIBLINGS     0x04000000L
#define WS_CLIPCHILDREN     0x02000000L
#define WS_MAXIMIZE         0x01000000L
#define WS_CAPTION          0x00C00000L
#define WS_BORDER           0x00800000L
#define WS_DLGFRAME         0x00400000L
#define WS_VSCROLL          0x00200000L
#define WS_HSCROLL          0x00100000L
#define WS_SYSMENU          0x00080000L
#define WS_THICKFRAME       0x00040000L
#define WS_GROUP            0x00020000L
#define WS_TABSTOP          0x00010000L
#define WS_MINIMIZEBOX      0x00020000L
#define WS_MAXIMIZEBOX      0x00010000L
#define WS_TILED            WS_OVERLAPPED
#define WS_ICONIC           WS_MINIMIZE
#define WS_SIZEBOX          WS_THICKFRAME
#define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)
#define WS_TILEDWINDOW      WS_OVERLAPPEDWINDOW
#define WS_POPUPWINDOW      (WS_POPUP | WS_BORDER | WS_SYSMENU)
#define WS_CHILDWINDOW      (WS_CHILD)
#endif

// Extended Window Styles (WS_EX_*)
#ifndef WS_EX_DLGMODALFRAME
#define WS_EX_DLGMODALFRAME  0x00000001L
#define WS_EX_NOPARENTNOTIFY 0x00000004L
#define WS_EX_TOPMOST        0x00000008L
#define WS_EX_ACCEPTFILES    0x00000010L
#define WS_EX_TRANSPARENT    0x00000020L
#define WS_EX_MDICHILD       0x00000040L
#define WS_EX_TOOLWINDOW     0x00000080L
#define WS_EX_WINDOWEDGE     0x00000100L
#define WS_EX_CLIENTEDGE     0x00000200L
#define WS_EX_CONTEXTHELP    0x00000400L
#define WS_EX_RIGHT          0x00001000L
#define WS_EX_LEFT           0x00000000L
#define WS_EX_RTLREADING     0x00002000L
#define WS_EX_LTRREADING     0x00000000L
#define WS_EX_LEFTSCROLLBAR  0x00004000L
#define WS_EX_RIGHTSCROLLBAR 0x00000000L
#define WS_EX_CONTROLPARENT  0x00010000L
#define WS_EX_STATICEDGE     0x00020000L
#define WS_EX_APPWINDOW      0x00040000L
#define WS_EX_OVERLAPPEDWINDOW (WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE)
#define WS_EX_PALETTEWINDOW  (WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST)
#define WS_EX_LAYERED        0x00080000L
#endif

// Class Styles (CS_*)
#ifndef CS_VREDRAW
#define CS_VREDRAW          0x0001
#define CS_HREDRAW          0x0002
#define CS_DBLCLKS          0x0008
#define CS_OWNDC            0x0020
#define CS_CLASSDC          0x0040
#define CS_PARENTDC         0x0080
#define CS_NOCLOSE          0x0200
#define CS_SAVEBITS         0x0800
#define CS_BYTEALIGNCLIENT  0x1000
#define CS_BYTEALIGNWINDOW  0x2000
#define CS_GLOBALCLASS      0x4000
#endif

// Window field offsets for GetWindowLong / SetWindowLong
#ifndef GWL_WNDPROC
#define GWL_WNDPROC         (-4)
#define GWL_HINSTANCE       (-6)
#define GWL_HWNDPARENT      (-8)
#define GWL_STYLE           (-16)
#define GWL_EXSTYLE         (-20)
#define GWL_USERDATA        (-21)
#define GWL_ID              (-12)
#define GWLP_WNDPROC        (-4)
#define GWLP_HINSTANCE      (-6)
#define GWLP_HWNDPARENT     (-8)
#define GWLP_USERDATA       (-21)
#define GWLP_ID             (-12)
#endif

// SetWindowPos flags & constants
#ifndef SWP_NOSIZE
#define SWP_NOSIZE          0x0001
#define SWP_NOMOVE          0x0002
#define SWP_NOZORDER        0x0004
#define SWP_NOREDRAW        0x0008
#define SWP_NOACTIVATE      0x0010
#define SWP_FRAMECHANGED    0x0020
#define SWP_SHOWWINDOW      0x0040
#define SWP_HIDEWINDOW      0x0080
#define SWP_NOCOPYBITS      0x0100
#define SWP_NOOWNERZORDER   0x0200
#define SWP_NOSENDCHANGING  0x0400
#endif

#ifndef HWND_TOP
#define HWND_TOP        ((HWND)0)
#define HWND_BOTTOM     ((HWND)1)
#define HWND_TOPMOST    ((HWND)-1)
#define HWND_NOTOPMOST  ((HWND)-2)
#endif

#ifndef CW_USEDEFAULT
#define CW_USEDEFAULT   ((int)0x80000000)
#endif

// System Metrics & Standard Cursors/Icons
#ifndef SM_CXSCREEN
#define SM_CXSCREEN 0
#define SM_CYSCREEN 1
#endif

#ifndef IDC_ARROW
#define IDC_ARROW       MAKEINTRESOURCEA(32512)
#define IDC_IBEAM       MAKEINTRESOURCEA(32513)
#define IDC_WAIT        MAKEINTRESOURCEA(32514)
#define IDC_CROSS       MAKEINTRESOURCEA(32515)
#define IDC_UPARROW     MAKEINTRESOURCEA(32516)
#define IDC_SIZEALL     MAKEINTRESOURCEA(32646)
#define IDC_NO          MAKEINTRESOURCEA(32648)
#define IDC_HAND        MAKEINTRESOURCEA(32649)
#define IDI_APPLICATION MAKEINTRESOURCEA(32512)
#endif

#ifndef PM_NOREMOVE
#define PM_NOREMOVE 0x0000
#define PM_REMOVE   0x0001
#endif

typedef WORD ATOM;

// Window & Message Management Stubs
static inline ATOM RegisterClassA(const WNDCLASSA *lpWndClass) { (void)lpWndClass; return 1; }
static inline ATOM RegisterClassExA(const WNDCLASSEXA *lpwcx) { (void)lpwcx; return 1; }
static inline BOOL UnregisterClassA(LPCSTR lpClassName, HINSTANCE hInstance) { (void)lpClassName; (void)hInstance; return TRUE; }
#ifndef RegisterClass
#define RegisterClass RegisterClassA
#define RegisterClassEx RegisterClassExA
#define UnregisterClass UnregisterClassA
#endif

static inline HWND CreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
    (void)dwExStyle; (void)lpClassName; (void)lpWindowName; (void)dwStyle;
    (void)X; (void)Y; (void)nWidth; (void)nHeight; (void)hWndParent; (void)hMenu; (void)hInstance; (void)lpParam;
    return (HWND)1;
}
#ifndef CreateWindowEx
#define CreateWindowEx CreateWindowExA
#endif
#ifndef CreateWindowA
#define CreateWindowA(lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam) \
    CreateWindowExA(0L, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam)
#define CreateWindow CreateWindowA
#endif

static inline BOOL DestroyWindow(HWND hWnd) { (void)hWnd; return TRUE; }
static inline BOOL ShowWindow(HWND hWnd, int nCmdShow) { (void)hWnd; (void)nCmdShow; return TRUE; }
static inline BOOL UpdateWindow(HWND hWnd) { (void)hWnd; return TRUE; }
static inline BOOL MoveWindow(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint) {
    (void)hWnd; (void)X; (void)Y; (void)nWidth; (void)nHeight; (void)bRepaint;
    return TRUE;
}
static inline BOOL SetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags) {
    (void)hWnd; (void)hWndInsertAfter; (void)X; (void)Y; (void)cx; (void)cy; (void)uFlags;
    return TRUE;
}
static inline BOOL AdjustWindowRect(LPRECT lpRect, DWORD dwStyle, BOOL bMenu) {
    (void)lpRect; (void)dwStyle; (void)bMenu;
    return TRUE;
}
static inline BOOL AdjustWindowRectEx(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle) {
    (void)lpRect; (void)dwStyle; (void)bMenu; (void)dwExStyle;
    return TRUE;
}

static inline BOOL SetWindowTextA(HWND hWnd, LPCSTR lpString) { (void)hWnd; (void)lpString; return TRUE; }
static inline int GetWindowTextA(HWND hWnd, LPSTR lpString, int nMaxCount) {
    (void)hWnd;
    if (lpString && nMaxCount > 0) lpString[0] = '\0';
    return 0;
}
#ifndef SetWindowText
#define SetWindowText SetWindowTextA
#define GetWindowText GetWindowTextA
#endif

static inline LONG GetWindowLongA(HWND hWnd, int nIndex) { (void)hWnd; (void)nIndex; return 0; }
static inline LONG SetWindowLongA(HWND hWnd, int nIndex, LONG dwNewLong) { (void)hWnd; (void)nIndex; return dwNewLong; }
static inline LONG_PTR GetWindowLongPtrA(HWND hWnd, int nIndex) { (void)hWnd; (void)nIndex; return 0; }
static inline LONG_PTR SetWindowLongPtrA(HWND hWnd, int nIndex, LONG_PTR dwNewLong) { (void)hWnd; (void)nIndex; return dwNewLong; }
#ifndef GetWindowLong
#define GetWindowLong GetWindowLongA
#define SetWindowLong SetWindowLongA
#define GetWindowLongPtr GetWindowLongPtrA
#define SetWindowLongPtr SetWindowLongPtrA
#endif

static inline LRESULT DefWindowProcA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    (void)hWnd; (void)Msg; (void)wParam; (void)lParam;
    return 0;
}
#ifndef DefWindowProc
#define DefWindowProc DefWindowProcA
#endif

static inline void PostQuitMessage(int nExitCode) { (void)nExitCode; }
static inline BOOL PeekMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg) {
    (void)lpMsg; (void)hWnd; (void)wMsgFilterMin; (void)wMsgFilterMax; (void)wRemoveMsg;
    return FALSE;
}
static inline BOOL GetMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax) {
    (void)lpMsg; (void)hWnd; (void)wMsgFilterMin; (void)wMsgFilterMax;
    return FALSE;
}
static inline BOOL TranslateMessage(const MSG *lpMsg) { (void)lpMsg; return TRUE; }
static inline LRESULT DispatchMessageA(const MSG *lpMsg) { (void)lpMsg; return 0; }
static inline LRESULT SendMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    (void)hWnd; (void)Msg; (void)wParam; (void)lParam;
    return 0;
}
static inline BOOL PostMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    (void)hWnd; (void)Msg; (void)wParam; (void)lParam;
    return TRUE;
}
#ifndef PeekMessage
#define PeekMessage PeekMessageA
#define GetMessage GetMessageA
#define DispatchMessage DispatchMessageA
#define SendMessage SendMessageA
#define PostMessage PostMessageA
#endif

static inline HICON LoadIconA(HINSTANCE hInstance, LPCSTR lpIconName) { (void)hInstance; (void)lpIconName; return (HICON)1; }
static inline HCURSOR LoadCursorA(HINSTANCE hInstance, LPCSTR lpCursorName) { (void)hInstance; (void)lpCursorName; return (HCURSOR)1; }
static inline HCURSOR SetCursor(HCURSOR hCursor) { return hCursor; }
static inline int ShowCursor(BOOL bShow) { return bShow ? 0 : -1; }
static inline BOOL DestroyCursor(HCURSOR hCursor) { (void)hCursor; return TRUE; }
static inline BOOL DestroyIcon(HICON hIcon) { (void)hIcon; return TRUE; }
#ifndef LoadIcon
#define LoadIcon LoadIconA
#define LoadCursor LoadCursorA
#endif

// Win32 LoadImage constants
#ifndef IMAGE_BITMAP
#define IMAGE_BITMAP        0
#define IMAGE_ICON          1
#define IMAGE_CURSOR        2
#define IMAGE_ENHMETAFILE   3
#endif

#ifndef LR_DEFAULTCOLOR
#define LR_DEFAULTCOLOR     0x00000000
#define LR_MONOCHROME       0x00000001
#define LR_COLOR            0x00000002
#define LR_COPYRETURNORG    0x00000004
#define LR_COPYDELETEORG    0x00000008
#define LR_LOADFROMFILE     0x00000010
#define LR_LOADTRANSPARENT  0x00000020
#define LR_DEFAULTSIZE      0x00000040
#define LR_VGACOLOR         0x00000080
#define LR_LOADMAP3DCOLORS  0x00001000
#define LR_CREATEDIBSECTION 0x00002000
#define LR_COPYFROMRESOURCE 0x00004000
#define LR_SHARED           0x00008000
#endif

static inline HANDLE LoadImageA(HINSTANCE hInst, LPCSTR name, UINT type, int cx, int cy, UINT fuLoad) {
    (void)hInst; (void)name; (void)type; (void)cx; (void)cy; (void)fuLoad;
    return (HANDLE)1;
}

#ifndef LoadImage
#define LoadImage LoadImageA
#endif

static inline HDC GetDC(HWND hWnd) { (void)hWnd; return (HDC)1; }
static inline int ReleaseDC(HWND hWnd, HDC hDC) { (void)hWnd; (void)hDC; return 1; }
static inline BOOL InvalidateRect(HWND hWnd, const RECT *lpRect, BOOL bErase) { (void)hWnd; (void)lpRect; (void)bErase; return TRUE; }
static inline BOOL ValidateRect(HWND hWnd, const RECT *lpRect) { (void)hWnd; (void)lpRect; return TRUE; }

static inline HWND SetCapture(HWND hWnd) { return hWnd; }
static inline BOOL ReleaseCapture(void) { return TRUE; }
static inline HWND GetCapture(void) { return (HWND)1; }
static inline HWND SetFocus(HWND hWnd) { return hWnd; }
static inline HWND GetFocus(void) { return (HWND)1; }
static inline HWND GetActiveWindow(void) { return (HWND)1; }
static inline HWND GetForegroundWindow(void) { return (HWND)1; }
static inline BOOL SetForegroundWindow(HWND hWnd) { (void)hWnd; return TRUE; }
static inline BOOL IsWindow(HWND hWnd) { return (hWnd != NULL) ? TRUE : FALSE; }
static inline BOOL IsWindowVisible(HWND hWnd) { (void)hWnd; return TRUE; }
static inline BOOL IsIconic(HWND hWnd) { (void)hWnd; return FALSE; }
static inline int GetSystemMetrics(int nIndex) {
    if (nIndex == SM_CXSCREEN || nIndex == 16) return (g_AndroidScreenWidth > 0) ? g_AndroidScreenWidth : 1280;
    if (nIndex == SM_CYSCREEN || nIndex == 17) return (g_AndroidScreenHeight > 0) ? g_AndroidScreenHeight : 720;
    return 0;
}

// Window Placement & Monitor stubs (used by MiniMode)
#ifndef WPF_SETMINPOSITION
#define WPF_SETMINPOSITION          0x0001
#define WPF_RESTORETOMAXIMIZED      0x0002
#define WPF_ASYNCWINDOWPLACEMENT    0x0004
#endif

typedef struct tagWINDOWPLACEMENT {
    UINT  length;
    UINT  flags;
    UINT  showCmd;
    POINT ptMinPosition;
    POINT ptMaxPosition;
    RECT  rcNormalPosition;
} WINDOWPLACEMENT, *PWINDOWPLACEMENT, *LPWINDOWPLACEMENT;

static inline BOOL GetWindowPlacement(HWND hWnd, WINDOWPLACEMENT *lpwndpl) {
    (void)hWnd;
    if (lpwndpl) {
        memset(lpwndpl, 0, sizeof(WINDOWPLACEMENT));
        lpwndpl->length = sizeof(WINDOWPLACEMENT);
        lpwndpl->showCmd = SW_SHOWNORMAL;
        lpwndpl->rcNormalPosition.right = 1280;
        lpwndpl->rcNormalPosition.bottom = 720;
    }
    return TRUE;
}

static inline BOOL SetWindowPlacement(HWND hWnd, const WINDOWPLACEMENT *lpwndpl) {
    (void)hWnd;
    (void)lpwndpl;
    return TRUE;
}

typedef void* HMONITOR;

#ifndef MONITOR_DEFAULTTONULL
#define MONITOR_DEFAULTTONULL       0x00000000
#define MONITOR_DEFAULTTOPRIMARY    0x00000001
#define MONITOR_DEFAULTTONEAREST    0x00000002
#endif

typedef struct tagMONITORINFO {
    DWORD cbSize;
    RECT  rcMonitor;
    RECT  rcWork;
    DWORD dwFlags;
} MONITORINFO, *LPMONITORINFO;

static inline HMONITOR MonitorFromWindow(HWND hwnd, DWORD dwFlags) {
    (void)hwnd; (void)dwFlags;
    return (HMONITOR)1;
}

static inline BOOL GetMonitorInfoA(HMONITOR hMonitor, LPMONITORINFO lpmi) {
    (void)hMonitor;
    if (lpmi) {
        lpmi->rcMonitor.left = 0; lpmi->rcMonitor.top = 0;
        lpmi->rcMonitor.right = 1280; lpmi->rcMonitor.bottom = 720;
        lpmi->rcWork = lpmi->rcMonitor;
        lpmi->dwFlags = 1;
    }
    return TRUE;
}
#ifndef GetMonitorInfo
#define GetMonitorInfo GetMonitorInfoA
#endif

#ifndef SPI_GETWORKAREA
#define SPI_GETWORKAREA 0x0030
#endif

static inline BOOL SystemParametersInfoA(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni) {
    (void)uiParam; (void)fWinIni;
    if (uiAction == SPI_GETWORKAREA && pvParam) {
        RECT* rc = (RECT*)pvParam;
        rc->left = 0; rc->top = 0; rc->right = 1280; rc->bottom = 720;
    }
    return TRUE;
}
#ifndef SystemParametersInfo
#define SystemParametersInfo SystemParametersInfoA
#endif

// Multimedia Timer APIs (mmsystem.h equivalents)
#include <time.h>

#ifndef TIMERR_NOERROR
#define TIMERR_NOERROR 0
#endif

typedef UINT MMRESULT;

typedef struct timecaps_tag {
    UINT wPeriodMin;
    UINT wPeriodMax;
} TIMECAPS, *PTIMECAPS, *LPTIMECAPS;

static inline DWORD timeGetTime(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (DWORD)((uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL);
}

static inline MMRESULT timeBeginPeriod(UINT uPeriod) {
    (void)uPeriod;
    return TIMERR_NOERROR;
}

static inline MMRESULT timeEndPeriod(UINT uPeriod) {
    (void)uPeriod;
    return TIMERR_NOERROR;
}

static inline MMRESULT timeGetDevCaps(LPTIMECAPS ptc, UINT cbtc) {
    if (ptc && cbtc >= sizeof(TIMECAPS)) {
        ptc->wPeriodMin = 1;
        ptc->wPeriodMax = 1000000;
    }
    return TIMERR_NOERROR;
}

// MSVC CRT string & formatting aliases
#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>
#include <math.h>

#ifndef _vsnprintf
#define _vsnprintf vsnprintf
#endif

#ifndef _snprintf
#define _snprintf snprintf
#endif

#ifndef _vsnwprintf
#define _vsnwprintf vswprintf
#endif

#ifndef _snwprintf
#define _snwprintf swprintf
#endif

#ifndef _stricmp
#define _stricmp strcasecmp
#endif

#ifndef _strnicmp
#define _strnicmp strncasecmp
#endif

#ifndef strnicmp
#define strnicmp strncasecmp
#endif

#ifndef stricmp
#define stricmp strcasecmp
#endif

#ifndef _strdup
#define _strdup strdup
#endif

#ifndef _atoi64
#define _atoi64 atoll
#endif

#ifndef _strtoi64
#define _strtoi64 strtoll
#endif

#ifndef _strtoui64
#define _strtoui64 strtoull
#endif

#ifndef _isnan
#define _isnan isnan
#endif

#ifndef _finite
#define _finite isfinite
#endif

// MSVC _TRUNCATE, _countof, and safe strncpy_s helpers
#ifndef _TRUNCATE
#define _TRUNCATE ((size_t)-1)
#endif

#ifndef _countof
#ifdef __cplusplus
template <typename T, size_t N>
constexpr size_t _android_countof(T (&)[N]) noexcept { return N; }
#define _countof(arr) _android_countof(arr)
#else
#define _countof(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif
#endif

#ifdef __cplusplus
static inline int _android_strncpy_s(char* dst, size_t dstSize, const char* src, size_t count) {
    if (!dst || dstSize == 0) return -1;
    if (!src) { dst[0] = '\0'; return 0; }
    size_t maxCopy = (count == _TRUNCATE || count >= dstSize) ? (dstSize - 1) : count;
    size_t i = 0;
    for (; i < maxCopy && src[i] != '\0'; ++i) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
    return 0;
}

template <size_t N>
static inline int _android_strncpy_s(char (&dst)[N], const char* src, size_t count) {
    return _android_strncpy_s(dst, N, src, count);
}

#undef strncpy_s
#define strncpy_s _android_strncpy_s
#endif
// Windows P* pointer typedefs
typedef CHAR*           PCHAR;
typedef CHAR*           PCH;
typedef const CHAR*     PCCH;
typedef WCHAR*          PWCHAR;
typedef WCHAR*          PWCH;
typedef const WCHAR*    PCWCH;
typedef BOOL*           PBOOL;
typedef INT*            PINT;
typedef UINT*           PUINT;
typedef LONG*           PLONG;
typedef FLOAT*          PFLOAT;

#ifndef GMEM_FIXED
#define GMEM_FIXED          0x0000
#define GMEM_MOVEABLE       0x0002
#define GMEM_ZEROINIT       0x0040
#define GPTR                (GMEM_FIXED | GMEM_ZEROINIT)
#endif

static inline HGLOBAL GlobalAlloc(UINT uFlags, size_t dwBytes) {
    void* ptr = (uFlags & GMEM_ZEROINIT) ? calloc(1, dwBytes) : malloc(dwBytes);
    return (HGLOBAL)ptr;
}

static inline HGLOBAL GlobalFree(HGLOBAL hMem) {
    if (hMem) free(hMem);
    return NULL;
}

static inline LPVOID GlobalLock(HGLOBAL hMem) {
    return (LPVOID)hMem;
}

static inline BOOL GlobalUnlock(HGLOBAL hMem) {
    (void)hMem;
    return TRUE;
}

// COM Initialization Stubs for Android
#ifndef COINIT_APARTMENTTHREADED
#define COINIT_APARTMENTTHREADED    0x2
#define COINIT_MULTITHREADED        0x0
#define COINIT_DISABLE_OLE1DDE      0x4
#define COINIT_SPEED_OVER_MEMORY    0x8
#endif

static inline HRESULT CoInitialize(LPVOID pvReserved) {
    (void)pvReserved;
    return S_OK;
}

static inline HRESULT CoInitializeEx(LPVOID pvReserved, DWORD dwCoInit) {
    (void)pvReserved;
    (void)dwCoInit;
    return S_OK;
}

static inline void CoUninitialize(void) {}

// MSVC interface keyword extension for COM headers (DIMM.h)
#ifndef interface
#define interface struct
#endif

// MSVC file operation aliases
#ifndef _unlink
#define _unlink unlink
#endif

#ifndef _rmdir
#define _rmdir rmdir
#endif

// Wide character logfont & EXTERN_C helper stubs
#ifndef _LOGFONTW_DEFINED
#define _LOGFONTW_DEFINED
typedef struct tagLOGFONTW {
    LONG lfHeight;
    LONG lfWidth;
    LONG lfEscapement;
    LONG lfOrientation;
    LONG lfWeight;
    BYTE lfItalic;
    BYTE lfUnderline;
    BYTE lfStrikeOut;
    BYTE lfCharSet;
    BYTE lfOutPrecision;
    BYTE lfClipPrecision;
    BYTE lfQuality;
    BYTE lfPitchAndFamily;
    WCHAR lfFaceName[LF_FACESIZE];
} LOGFONTW, *PLOGFONTW, *LPLOGFONTW;
#endif

#ifndef EXTERN_C
#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif
#endif

// MSVC UNREFERENCED_PARAMETER macro stub
#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P) (void)(P)
#endif

#ifdef __ANDROID__
#include <mss.h>

// Overload wrappers for Miles Sound System ADPCM & ASI decompression under 64-bit Android NDK
inline int AIL_decompress_ADPCM(AILSOUNDINFO const FAR* info, void** dest, unsigned int* dest_len) {
    unsigned long temp_len = dest_len ? *dest_len : 0;
    int result = AIL_decompress_ADPCM(info, dest, &temp_len);
    if (dest_len) *dest_len = (unsigned int)temp_len;
    return result;
}

// Overload wrapper for unsigned int* (ULONG*)
inline int AIL_decompress_ASI(void const FAR* src, unsigned int src_len, const char* filename, void** dest, unsigned int* dest_len, void* unk) {
    (void)filename;
    (void)unk;
    unsigned long temp_src_len = src_len;
    unsigned long temp_dest_len = dest_len ? (unsigned long)*dest_len : 0;
    int result = ::AIL_decompress_ASI(src, temp_src_len, NULL, dest, &temp_dest_len, (AILLENGTHYCB)NULL);
    if (dest_len) *dest_len = (unsigned int)temp_dest_len;
    return result;
}

// Overload wrapper for unsigned long*
inline int AIL_decompress_ASI(void const FAR* src, unsigned int src_len, const char* filename, void** dest, unsigned long* dest_len, void* unk) {
    (void)filename;
    (void)unk;
    unsigned long temp_src_len = src_len;
    unsigned long temp_dest_len = dest_len ? *dest_len : 0;
    int result = ::AIL_decompress_ASI(src, temp_src_len, NULL, dest, &temp_dest_len, (AILLENGTHYCB)NULL);
    if (dest_len) *dest_len = temp_dest_len;
    return result;
}
#endif


// OS Version info structures & GetVersionEx stubs
#ifndef _OSVERSIONINFOA_DEFINED
#define _OSVERSIONINFOA_DEFINED

typedef struct tagOSVERSIONINFOA {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    CHAR  szCSDVersion[128];
} OSVERSIONINFOA, *POSVERSIONINFOA, *LPOSVERSIONINFOA;

typedef struct tagOSVERSIONINFOW {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    WCHAR szCSDVersion[128];
} OSVERSIONINFOW, *POSVERSIONINFOW, *LPOSVERSIONINFOW;

#ifndef OSVERSIONINFO
#define OSVERSIONINFO OSVERSIONINFOA
#endif

static inline BOOL GetVersionExA(LPOSVERSIONINFOA lpVersionInformation) {
    if (!lpVersionInformation) return FALSE;
    lpVersionInformation->dwMajorVersion = 6;
    lpVersionInformation->dwMinorVersion = 1;
    lpVersionInformation->dwBuildNumber = 7601;
    lpVersionInformation->dwPlatformId = 2;
    lpVersionInformation->szCSDVersion[0] = '\0';
    return TRUE;
}

static inline BOOL GetVersionExW(LPOSVERSIONINFOW lpVersionInformation) {
    if (!lpVersionInformation) return FALSE;
    lpVersionInformation->dwMajorVersion = 6;
    lpVersionInformation->dwMinorVersion = 1;
    lpVersionInformation->dwBuildNumber = 7601;
    lpVersionInformation->dwPlatformId = 2;
    lpVersionInformation->szCSDVersion[0] = L'\0';
    return TRUE;
}

#ifndef GetVersionEx
#define GetVersionEx GetVersionExA
#endif

#endif // _OSVERSIONINFOA_DEFINED

// DirectShow GUID Stubs for Android
#ifndef __DirectShow_GUIDs_Defined__
#define __DirectShow_GUIDs_Defined__

#ifndef _GUID_DEFINED
#define _GUID_DEFINED
typedef struct _GUID {
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
} GUID;
#endif

#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef GUID CLSID;
typedef GUID IID;
#endif

// COM Reference GUID typedefs
#ifndef _REFGUID_DEFINED
#define _REFGUID_DEFINED
#ifdef __cplusplus
typedef const GUID &REFGUID;
typedef const GUID &REFCLSID;
typedef const GUID &REFIID;
#else
typedef const GUID *REFGUID;
typedef const GUID *REFCLSID;
typedef const GUID *REFIID;
#endif
#endif

#ifndef _BSTR_DEFINED
#define _BSTR_DEFINED
typedef WCHAR* BSTR;
#endif

#ifndef __IUnknown_INTERFACE_DEFINED__
#define __IUnknown_INTERFACE_DEFINED__
struct IUnknown {
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(const GUID& riid, void** ppvObject) = 0;
    virtual ULONG STDMETHODCALLTYPE AddRef(void) = 0;
    virtual ULONG STDMETHODCALLTYPE Release(void) = 0;
};
#endif

// Map __uuidof to interface IID constants for Clang on Android
#ifndef __uuidof
#define __uuidof(T) IID_##T
#endif

// Text Services Framework IID Definitions
EXTERN_C inline const GUID IID_ITfThreadMgr                      = { 0xaa80e801, 0x2021, 0x11d2, { 0x93, 0xe0, 0x00, 0x60, 0xb0, 0x67, 0xb8, 0x6e } };
EXTERN_C inline const GUID IID_ITfThreadMgrEx                    = { 0x3e90ade3, 0x7594, 0x4cb0, { 0xbb, 0x58, 0x69, 0x62, 0x8f, 0x5f, 0x45, 0x8c } };
EXTERN_C inline const GUID IID_ITfSource                         = { 0x4ea48a35, 0x60ae, 0x446f, { 0x8f, 0xd6, 0xe6, 0xa8, 0xd8, 0x24, 0x59, 0xf7 } };
EXTERN_C inline const GUID IID_ITfUIElementSink                  = { 0xea1ea136, 0x19df, 0x11d7, { 0xa6, 0xd2, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c } };
EXTERN_C inline const GUID IID_ITfInputProcessorProfileActivationSink = { 0x71c6e74e, 0x0f28, 0x11d8, { 0xa8, 0x2a, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c } };
EXTERN_C inline const GUID IID_ITfCompartmentEventSink           = { 0x743abd5f, 0xf26d, 0x48df, { 0x8c, 0xc5, 0x23, 0x84, 0x92, 0x41, 0x9b, 0x64 } };
EXTERN_C inline const GUID IID_ITfReadingInformationUIElement    = { 0xea1ea139, 0x19df, 0x11d7, { 0xa6, 0xd2, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c } };
EXTERN_C inline const GUID IID_ITfCandidateListUIElement         = { 0xea1ea138, 0x19df, 0x11d7, { 0xa6, 0xd2, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c } };
EXTERN_C inline const GUID IID_ITfUIElementMgr                   = { 0xea1ea135, 0x19df, 0x11d7, { 0xa6, 0xd2, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c } };
EXTERN_C inline const GUID IID_ITfInputProcessorProfiles         = { 0x1F02B6C5, 0x7842, 0x4EE6, { 0x8A, 0x0B, 0x9A, 0x24, 0x18, 0x3A, 0x95, 0xCA } };
EXTERN_C inline const GUID IID_ITfInputProcessorProfileMgr      = { 0x71c6e74c, 0x0f28, 0x11d8, { 0xa8, 0x2a, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c } };
EXTERN_C inline const GUID IID_ITfCompartmentMgr                 = { 0x7dcf57ac, 0x18ad, 0x438b, { 0x82, 0x4d, 0x97, 0x9b, 0xff, 0xb7, 0x4b, 0x7c } };

// Win32 Primary Language Identifiers for IME / Locales
#ifndef LANG_NEUTRAL
#define LANG_NEUTRAL        0x00
#define LANG_INVARIANT      0x7f
#define LANG_ARABIC         0x01
#define LANG_BULGARIAN      0x02
#define LANG_CATALAN        0x03
#define LANG_CHINESE        0x04
#define LANG_CZECH          0x05
#define LANG_DANISH         0x06
#define LANG_GERMAN         0x07
#define LANG_GREEK          0x08
#define LANG_ENGLISH        0x09
#define LANG_SPANISH        0x0a
#define LANG_FINNISH        0x0b
#define LANG_FRENCH         0x0c
#define LANG_HEBREW         0x0d
#define LANG_HUNGARIAN      0x0e
#define LANG_ICELANDIC      0x0f
#define LANG_ITALIAN        0x10
#define LANG_JAPANESE       0x11
#define LANG_KOREAN         0x12
#define LANG_DUTCH          0x13
#define LANG_NORWEGIAN      0x14
#define LANG_POLISH         0x15
#define LANG_PORTUGUESE     0x16
#define LANG_ROMANIAN       0x18
#define LANG_RUSSIAN        0x19
#define LANG_CROATIAN       0x1a
#define LANG_SLOVAK         0x1b
#define LANG_SWEDISH        0x1d
#define LANG_THAI           0x1e
#define LANG_TURKISH        0x1f
#define LANG_UKRAINIAN      0x22
#define LANG_BELARUSIAN     0x23
#define LANG_SLOVENIAN      0x24
#define LANG_ESTONIAN       0x25
#define LANG_LATVIAN        0x26
#define LANG_LITHUANIAN     0x27
#define LANG_VIETNAMESE     0x2a
#define LANG_MACEDONIAN     0x2f
#endif

#ifndef PRIMARYLANGID
#define PRIMARYLANGID(lgid) ((WORD)(lgid) & 0x3ff)
#define SUBLANGID(lgid)     ((WORD)(lgid) >> 10)
#define MAKELANGID(p, s)    ((((WORD)(s)) << 10) | (WORD)(p))
#define MAKELCID(lgid, srt) (((DWORD)((WORD)(srt)) << 16) | (DWORD)((WORD)(lgid)))
#define SORT_DEFAULT        0x0
#endif

// Clipboard format & Data Object stubs for OLE / TSF
typedef WORD CLIPFORMAT;

typedef struct tagFORMATETC {
    CLIPFORMAT cfFormat;
    void *ptd;
    DWORD dwAspect;
    LONG lindex;
    DWORD tymed;
} FORMATETC, *LPFORMATETC;

#ifdef __cplusplus
struct IDataObject : public IUnknown {};
#endif
#ifndef __IEnumGUID_INTERFACE_DEFINED__
#define __IEnumGUID_INTERFACE_DEFINED__
struct IEnumGUID : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE Next(ULONG celt, GUID *rgelt, ULONG *pceltFetched) = 0;
    virtual HRESULT STDMETHODCALLTYPE Skip(ULONG celt) = 0;
    virtual HRESULT STDMETHODCALLTYPE Reset(void) = 0;
    virtual HRESULT STDMETHODCALLTYPE Clone(IEnumGUID **ppenum) = 0;
};
#endif
// OLE Automation VARIANT definition for msctf.h & COM interfaces
#ifndef _VARIANT_DEFINED
#define _VARIANT_DEFINED
typedef short VARIANT_BOOL;

typedef struct tagVARIANT {
    WORD vt;
    WORD wReserved1;
    WORD wReserved2;
    WORD wReserved3;
    union {
        LONGLONG llVal;
        LONG lVal;
        BYTE bVal;
        SHORT iVal;
        FLOAT fltVal;
        double dblVal;
        VARIANT_BOOL boolVal;
        LONG scode;
        BSTR bstrVal;
        struct IUnknown *punkVal;
        struct IDispatch *pdispVal;
        void *parray;
        void *byref;
    };
} VARIANT, *LPVARIANT, VARIANTARG, *LPVARIANTARG;
#endif

// DirectShow & Media Control Interface Stubs for Android
#ifndef __DIRECTSHOW_INTERFACES_DEFINED__
#define __DIRECTSHOW_INTERFACES_DEFINED__

typedef LONGLONG REFERENCE_TIME;
struct IMediaClock;
struct IEnumPins;
struct IPin;
struct IBaseFilter;
struct IGraphBuilder;
typedef WCHAR OLECHAR;
typedef WCHAR* LPOLESTR;
typedef const WCHAR* LPCOLESTR;
struct IBindCtx;
struct IMoniker;
struct IStream;
struct IStorage;
typedef WCHAR* BSTR;

#ifndef __IUnknown_INTERFACE_DEFINED__
#define __IUnknown_INTERFACE_DEFINED__
struct IUnknown {
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(const GUID& riid, void** ppvObject) = 0;
    virtual ULONG STDMETHODCALLTYPE AddRef(void) = 0;
    virtual ULONG STDMETHODCALLTYPE Release(void) = 0;
};
#endif

#ifndef __STREAM_TYPE_DEFINED__
#define __STREAM_TYPE_DEFINED__
typedef enum tagSTREAM_TYPE {
    STREAMTYPE_READ = 0,
    STREAMTYPE_WRITE = 1,
    STREAMTYPE_TRANSFORM = 2
} STREAM_TYPE;
typedef LONGLONG STREAM_TIME;
typedef GUID MSPID;
typedef MSPID* PMSPID;
typedef const MSPID* PCMSPID;
#if defined(__cplusplus)
typedef const GUID &REFMSPID;
#else
typedef const GUID *REFMSPID;
#endif
#endif

struct IStreamSample;
struct IMultiMediaStream;

#ifndef __IMediaStream_INTERFACE_DEFINED__
#define __IMediaStream_INTERFACE_DEFINED__
struct IMediaStream : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetMultiMediaStream(IMultiMediaStream **ppMultiMediaStream) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetInformation(MSPID *pPurposeId, STREAM_TYPE *pType) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetSameFormat(IMediaStream *pStreamThatHasDesiredFormat, DWORD dwFlags) = 0;
    virtual HRESULT STDMETHODCALLTYPE AllocateSample(DWORD dwFlags, IStreamSample **ppSample) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateSharedSample(IStreamSample *pExistingSample, DWORD dwFlags, IStreamSample **ppNewSample) = 0;
    virtual HRESULT STDMETHODCALLTYPE SendEndOfStream(DWORD dwFlags) = 0;
};
#endif

#ifndef __IMultiMediaStream_INTERFACE_DEFINED__
#define __IMultiMediaStream_INTERFACE_DEFINED__
struct IMultiMediaStream : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetInformation(DWORD *pdwFlags, STREAM_TYPE *pStreamType) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetMediaStream(const GUID &idPurpose, IMediaStream **ppMediaStream) = 0;
    virtual HRESULT STDMETHODCALLTYPE EnumMediaStreams(long Index, IMediaStream **ppMediaStream) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetState(int *pCurrentState) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetState(int NewState) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetTime(STREAM_TIME *pCurrentTime) = 0;
    virtual HRESULT STDMETHODCALLTYPE Seek(STREAM_TIME SeekTime) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetEndOfStreamEventHandle(HANDLE *phEvent) = 0;
};
#endif

// Full stub for IDispatch so derived interfaces like IDirectShowStream can inherit from it
struct IDispatch : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT *pctinfo) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT iTInfo, DWORD lcid, void **ppTInfo) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames(const GUID &riid, LPOLESTR *rgszNames, UINT cNames, DWORD lcid, long *rgDispId) = 0;
    virtual HRESULT STDMETHODCALLTYPE Invoke(long dispIdMember, const GUID &riid, DWORD lcid, WORD wFlags, void *pDispParams, void *pVarResult, void *pExcepInfo, UINT *puArgErr) = 0;
};

// Define FILTER_INFO struct/typedef before interface declarations use it
typedef struct _FILTER_INFO {
    WCHAR achName[128];
    void* pGraph;
} FILTER_INFO;

// DirectShow Media Type definition for Android
typedef struct _AM_MEDIA_TYPE {
    GUID     majortype;
    GUID     subtype;
    BOOL     bFixedSizeSamples;
    BOOL     bTemporalCompression;
    ULONG    lSampleSize;
    GUID     formattype;
    IUnknown *pUnk;
    ULONG    cbFormat;
    BYTE     *pbFormat;
} AM_MEDIA_TYPE;


struct IBaseFilter : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetClassID(GUID *pClassID) = 0;
    virtual HRESULT STDMETHODCALLTYPE Stop(void) = 0;
    virtual HRESULT STDMETHODCALLTYPE Pause(void) = 0;
    virtual HRESULT STDMETHODCALLTYPE Run(REFERENCE_TIME tStart) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetState(DWORD dwMilliSecsTimeout, int *pState) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetSyncSource(IMediaClock *pClock) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetSyncSource(IMediaClock **pClock) = 0;
    virtual HRESULT STDMETHODCALLTYPE EnumPins(IEnumPins **ppEnum) = 0;
    virtual HRESULT STDMETHODCALLTYPE FindPin(LPCWSTR ID, IPin **ppPin) = 0;
    virtual HRESULT STDMETHODCALLTYPE QueryFilterInfo(FILTER_INFO *pInfo) = 0;
    virtual HRESULT STDMETHODCALLTYPE JoinFilterGraph(void *pGraph, LPCWSTR pName) = 0;
    virtual HRESULT STDMETHODCALLTYPE QueryVendorInfo(LPWSTR *pVendorInfo) = 0;
};

struct IGraphBuilder : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE AddFilter(IBaseFilter *pFilter, LPCWSTR pName) = 0;
    virtual HRESULT STDMETHODCALLTYPE RemoveFilter(IBaseFilter *pFilter) = 0;
    virtual HRESULT STDMETHODCALLTYPE EnumFilters(void **ppEnum) = 0;
    virtual HRESULT STDMETHODCALLTYPE FindFilterByName(LPCWSTR pName, IBaseFilter **ppFilter) = 0;
    virtual HRESULT STDMETHODCALLTYPE Connect(IPin *ppinOut, IPin *ppinIn) = 0;
    virtual HRESULT STDMETHODCALLTYPE Render(IPin *ppinOut) = 0;
    virtual HRESULT STDMETHODCALLTYPE RenderFile(LPCWSTR lpcwstrFile, LPCWSTR lpcwstrPlayList) = 0;
    virtual HRESULT STDMETHODCALLTYPE AddSourceFilter(LPCWSTR lpcwstrFileName, LPCWSTR lpcwstrFilterName, IBaseFilter **ppFilter) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetLogFile(DWORD_PTR hFile) = 0;
    virtual HRESULT STDMETHODCALLTYPE Abort(void) = 0;
    virtual HRESULT STDMETHODCALLTYPE ShouldInjectClock(void) = 0;
};

typedef LONG_PTR OAHWND;
typedef unsigned char byte;

struct ISampleGrabber : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE SetOneShot(BOOL bOneShot) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetMediaType(const void *pType) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType(void *pType) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetBufferSamples(BOOL bBufferThem) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer(LONG *pBufferSize, LONG *pBuffer) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetCurrentSample(void *ppSample) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetCallback(void *pCallback, long WhichMethodToCallback) = 0;
};

struct IMediaControl : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE Run(void) = 0;
    virtual HRESULT STDMETHODCALLTYPE Pause(void) = 0;
    virtual HRESULT STDMETHODCALLTYPE Stop(void) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetState(LONG msTimeout, LONGLONG *pState) = 0;
    virtual HRESULT STDMETHODCALLTYPE RenderFile(BSTR strFilename) = 0;
    virtual HRESULT STDMETHODCALLTYPE AddSourceFilter(BSTR strFilename, IDispatch **ppUnk) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_FilterCollection(IDispatch **ppUnk) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_RegFilterCollection(IDispatch **ppUnk) = 0;
    virtual HRESULT STDMETHODCALLTYPE StopWhenReady(void) = 0;
};

struct IMediaEventEx : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetEventHandle(LONG_PTR *hEvent) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetEvent(long *lEventCode, long *lParam1, long *lParam2, long msTimeout) = 0;
    virtual HRESULT STDMETHODCALLTYPE FreeEventParams(long lEventCode, long lParam1, long lParam2) = 0;
    virtual HRESULT STDMETHODCALLTYPE RegisterWindowMessage(LONG Msg) = 0;
    virtual HRESULT STDMETHODCALLTYPE NotifyWindowHandle(LONG_PTR hwnd, LONG Msg, LONG_PTR lInstanceData) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetNotifyWindow(OAHWND hwnd, long lMsg, LONG_PTR lInstanceData) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetNotifyFlags(LONG lFlags) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetNotifyFlags(LONG *lplFlags) = 0;
};

struct IVideoWindow : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE put_Caption(BSTR strCaption) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_Caption(BSTR *strCaption) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_WindowStyle(long lStyle) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_WindowStyle(long *lStyle) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_WindowOwner(LONG_PTR Owner) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_WindowOwner(LONG_PTR *Owner) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_MessageDrain(LONG_PTR Drain) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_MessageDrain(LONG_PTR *Drain) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_BorderColor(long *Color) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_BorderColor(long Color) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_FullScreenMode(long *FullScreenMode) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_FullScreenMode(long FullScreenMode) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetWindowPosition(long Left, long Top, long Width, long Height) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetWindowPosition(long *pLeft, long *pTop, long *pWidth, long *pHeight) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetWindowForeground(long Focus) = 0;
    virtual HRESULT STDMETHODCALLTYPE NotifyOwnerMessage(LONG_PTR hwnd, long uMsg, LONG_PTR wParam, LONG_PTR lParam) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetDefaultWindowPosition(void) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetCurrentImage(long *pBufferSize, long *pDIBImage) = 0;
    virtual HRESULT STDMETHODCALLTYPE is_Visible(long *pVisible) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_Visible(long Visible) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_Owner(LONG_PTR Owner) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_Owner(LONG_PTR *Owner) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_Left(long Left) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_Left(long *pLeft) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_Width(long Width) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_Width(long *pWidth) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_Top(long Top) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_Top(long *pTop) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_Height(long Height) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_Height(long *pHeight) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_WindowStates(long WindowState) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_WindowStates(long *WindowState) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetPaletteEntries(long StartIndex, long Entries, long *pPalette) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetCurrentActualImage(long *pBufferSize, long *pDIBImage) = 0;
    virtual HRESULT STDMETHODCALLTYPE RealizePalette(void) = 0;
};

// 1. DEFINE_GUID & EXTERN_GUID macros for DirectX/DirectShow headers (amstream.h, etc.)
#ifndef DEFINE_GUID
#ifdef __cplusplus
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    EXTERN_C inline const GUID name = { l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }
#else
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    const GUID name = { l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }
#endif
#endif

#ifndef EXTERN_GUID
#define EXTERN_GUID DEFINE_GUID
#endif

// 2. Allocator properties & Video Info structs for amstream.h and PythonApplicationLogo.cpp
#ifndef _ALLOCATOR_PROPERTIES_DEFINED
#define _ALLOCATOR_PROPERTIES_DEFINED
typedef struct _AllocatorProperties {
    long cBuffers;
    long cbBuffer;
    long cbAlign;
    long cbPrefix;
} ALLOCATOR_PROPERTIES;
#endif

#ifndef _VIDEOINFOHEADER_DEFINED
#define _VIDEOINFOHEADER_DEFINED
typedef struct tagVIDEOINFOHEADER {
    RECT             rcSource;
    RECT             rcTarget;
    DWORD            dwBitRate;
    DWORD            dwBitErrorRate;
    REFERENCE_TIME   AvgTimePerFrame;
    BITMAPINFOHEADER bmiHeader;
} VIDEOINFOHEADER;
#endif

// 3. Complete IBasicVideo & IBasicAudio stubs with methods used by MovieMan / Logo player
struct IBasicVideo : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE get_SourceWidth(long *pWidth) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_SourceHeight(long *pHeight) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetDefaultSourcePosition(void) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetDestinationPosition(long Left, long Top, long Width, long Height) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetVideoSize(long *pWidth, long *pHeight) = 0;
};

struct IBasicAudio : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE put_Volume(long lVolume) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_Volume(long *plVolume) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_Balance(long lBalance) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_Balance(long *plBalance) = 0;
};

// 4. Stream sample & DirectShow playback constants used by MovieMan.cpp & PythonApplicationLogo.cpp
#ifndef STREAMSTATE_STOP
#define STREAMSTATE_STOP 0
#define STREAMSTATE_RUN  1
#endif

#ifndef COMPSTAT_NOUPDATEOK
#define COMPSTAT_NOUPDATEOK 0x00000001
#define COMPSTAT_WAIT       0x00000002
#define COMPSTAT_ABORT      0x00000004
#endif

#ifndef EC_COMPLETE
#define EC_COMPLETE   0x01
#define EC_USERABORT  0x02
#define EC_ERRORABORT 0x03
#endif

#ifndef OATRUE
#define OATRUE  (-1)
#define OAFALSE (0)
#endif

struct IStreamSample : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetMediaStream(IMediaStream **ppMediaStream) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetSampleTimes(STREAM_TIME *pStart, STREAM_TIME *pEnd, STREAM_TIME *pCurrent) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetSampleTimes(const STREAM_TIME *pStart, const STREAM_TIME *pEnd) = 0;
    virtual HRESULT STDMETHODCALLTYPE Update(DWORD dwFlags, HANDLE hEvent, void *pfnAPC, DWORD_PTR dwAPCData) = 0;
    virtual HRESULT STDMETHODCALLTYPE CompletionStatus(DWORD dwFlags, DWORD dwMilliseconds) = 0;
};

#endif // __DIRECTSHOW_INTERFACES_DEFINED__

// 5. All DirectShow / ActiveMovie GUIDs & CLSIDs needed by MovieMan.cpp and PythonApplicationLogo.cpp
#ifndef CLSCTX_INPROC_SERVER
#define CLSCTX_INPROC_SERVER 0x1
#define CLSCTX_INPROC_HANDLER 0x2
#define CLSCTX_LOCAL_SERVER 0x4
#define CLSCTX_ALL (CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER)
#endif

EXTERN_C inline const GUID CLSID_FilterGraph          = { 0xe436ebb3, 0x524f, 0x11ce, { 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70 } };
EXTERN_C inline const GUID CLSID_SampleGrabber        = { 0xc1f400a0, 0x3f08, 0x11d3, { 0x9f, 0x0b, 0x00, 0x60, 0x08, 0x03, 0x9e, 0x37 } };
EXTERN_C inline const GUID CLSID_NullRenderer         = { 0xc1f400a4, 0x3f08, 0x11d3, { 0x9f, 0x0b, 0x00, 0x60, 0x08, 0x03, 0x9e, 0x37 } };

EXTERN_C inline const GUID IID_IUnknown               = { 0x00000000, 0x0000, 0x0000, { 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
EXTERN_C inline const GUID IID_IGraphBuilder          = { 0x56a868a9, 0x0ad4, 0x11ce, { 0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70 } };
EXTERN_C inline const GUID IID_ISampleGrabber         = { 0x6b652fff, 0x11fe, 0x4fce, { 0x92, 0xad, 0x02, 0x66, 0xb5, 0xd7, 0xc7, 0x8f } };
EXTERN_C inline const GUID IID_IBaseFilter            = { 0x56a86895, 0x0ad4, 0x11ce, { 0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70 } };
EXTERN_C inline const GUID IID_IMediaControl          = { 0x56a868b1, 0x0ad4, 0x11ce, { 0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70 } };
EXTERN_C inline const GUID IID_IMediaEvent            = { 0x56a868b6, 0x0ad4, 0x11ce, { 0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70 } };
EXTERN_C inline const GUID IID_IMediaEventEx          = { 0x56a868c0, 0x0ad4, 0x11ce, { 0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70 } };
EXTERN_C inline const GUID IID_IVideoWindow           = { 0x56a868b4, 0x0ad4, 0x11ce, { 0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70 } };
EXTERN_C inline const GUID IID_IBasicVideo            = { 0x56a868b5, 0x0ad4, 0x11ce, { 0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70 } };
EXTERN_C inline const GUID IID_IBasicAudio            = { 0x56a868b3, 0x0ad4, 0x11ce, { 0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70 } };
EXTERN_C inline const GUID IID_IDirectDrawMediaStream = { 0xf4104fce, 0x9a70, 0x11d0, { 0x8f, 0xde, 0x00, 0xc0, 0x4f, 0xd9, 0x18, 0x9d } };
EXTERN_C inline const GUID IID_IAudioMediaStream      = { 0xf7537560, 0xa3be, 0x11d0, { 0x82, 0x12, 0x00, 0xc0, 0x4f, 0xc3, 0x2c, 0x45 } };

EXTERN_C inline const GUID MSPID_PrimaryVideo         = { 0xa35ff56a, 0x9fda, 0x11d0, { 0x8f, 0xdf, 0x00, 0xc0, 0x4f, 0xd9, 0x18, 0x9d } };
EXTERN_C inline const GUID MSPID_PrimaryAudio         = { 0xa35ff56b, 0x9fda, 0x11d0, { 0x8f, 0xdf, 0x00, 0xc0, 0x4f, 0xd9, 0x18, 0x9d } };

EXTERN_C inline const GUID MEDIATYPE_Video            = { 0x73646976, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };
EXTERN_C inline const GUID MEDIATYPE_Audio            = { 0x73647561, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };
EXTERN_C inline const GUID MEDIASUBTYPE_RGB24         = { 0xe436eb7d, 0x524f, 0x11ce, { 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70 } };
EXTERN_C inline const GUID MEDIASUBTYPE_RGB32         = { 0xe436eb7e, 0x524f, 0x11ce, { 0x9f, 0x53, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70 } };
EXTERN_C inline const GUID FORMAT_VideoInfo           = { 0x05589f80, 0xc356, 0x11ce, { 0xbf, 0x01, 0x00, 0xaa, 0x00, 0x55, 0x59, 0x5a } };

static inline HRESULT CoCreateInstance(const GUID& rclsid, LPVOID pUnkOuter, DWORD dwClsContext, const GUID& riid, LPVOID* ppv) {
    (void)rclsid; (void)pUnkOuter; (void)dwClsContext; (void)riid;
    if (ppv) *ppv = NULL;
    return E_NOINTERFACE;
}

#endif // __DirectShow_GUIDs_Defined__

// 6. Common Win32 File-Finding, CRT, Timer, Clipboard, Shell & System stubs
#ifndef _unlink
#define _unlink unlink
#endif
#ifndef _rmdir
#define _rmdir rmdir
#endif
#ifndef _access
#define _access access
#endif

#ifdef __ANDROID__
#include <cctype>
#include <ctype.h>
#include <cstring>
#include <cerrno>
using std::tolower;
using std::toupper;

inline int memcpy_s(void* dest, size_t destSize, const void* src, size_t count) {
    if (!dest || !src) return EINVAL;
    if (destSize < count) {
        std::memset(dest, 0, destSize);
        return ERANGE;
    }
    std::memcpy(dest, src, count);
    return 0;
}
#endif

#ifndef _WIN32_FIND_DATA_DEFINED
#define _WIN32_FIND_DATA_DEFINED
typedef struct tagWIN32_FIND_DATAA {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD dwReserved0;
    DWORD dwReserved1;
    CHAR  cFileName[MAX_PATH];
    CHAR  cAlternateFileName[14];
} WIN32_FIND_DATAA, *PWIN32_FIND_DATAA, *LPWIN32_FIND_DATAA;

typedef struct tagWIN32_FIND_DATAW {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD dwReserved0;
    DWORD dwReserved1;
    WCHAR cFileName[MAX_PATH];
    WCHAR cAlternateFileName[14];
} WIN32_FIND_DATAW, *PWIN32_FIND_DATAW, *LPWIN32_FIND_DATAW;

#ifndef WIN32_FIND_DATA
#define WIN32_FIND_DATA WIN32_FIND_DATAA
#define LPWIN32_FIND_DATA LPWIN32_FIND_DATAA
#endif
#endif

#ifndef CF_TEXT
#define CF_TEXT 1
#define CF_UNICODETEXT 13
#endif

static inline BOOL OpenClipboard(HWND hWndNewOwner) { (void)hWndNewOwner; return FALSE; }
static inline BOOL CloseClipboard(void) { return TRUE; }
static inline BOOL EmptyClipboard(void) { return TRUE; }
static inline HANDLE GetClipboardData(UINT uFormat) { (void)uFormat; return NULL; }
static inline HANDLE SetClipboardData(UINT uFormat, HANDLE hMem) { (void)uFormat; return hMem; }

static inline UINT_PTR SetTimer(HWND hWnd, UINT_PTR nIDEvent, UINT uElapse, void* lpTimerFunc) {
    (void)hWnd; (void)uElapse; (void)lpTimerFunc;
    return nIDEvent ? nIDEvent : 1;
}
static inline BOOL KillTimer(HWND hWnd, UINT_PTR uIDEvent) { (void)hWnd; (void)uIDEvent; return TRUE; }
static inline UINT GetDoubleClickTime(void) { return 500; }
static inline UINT GetCaretBlinkTime(void) { return 500; }

static inline DWORD GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize) {
    (void)hModule;
    if (lpFilename && nSize > 0) lpFilename[0] = '\0';
    return 0;
}
#ifndef GetModuleFileName
#define GetModuleFileName GetModuleFileNameA
#endif

static inline LPSTR GetCommandLineA(void) { return (LPSTR)""; }
#ifndef GetCommandLine
#define GetCommandLine GetCommandLineA
#endif

static inline HANDLE FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData) {
    (void)lpFileName; (void)lpFindFileData;
    return INVALID_HANDLE_VALUE;
}
static inline BOOL FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData) {
    (void)hFindFile; (void)lpFindFileData;
    return FALSE;
}
static inline BOOL FindClose(HANDLE hFindFile) { (void)hFindFile; return TRUE; }
#ifndef FindFirstFile
#define FindFirstFile FindFirstFileA
#define FindNextFile  FindNextFileA
#endif

static inline HINSTANCE ShellExecuteA(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd) {
    (void)hwnd; (void)lpOperation; (void)lpFile; (void)lpParameters; (void)lpDirectory; (void)nShowCmd;
    return (HINSTANCE)33;
}
#ifndef ShellExecute
#define ShellExecute ShellExecuteA
#endif

static inline BOOL GetVolumeInformationA(LPCSTR lpRootPathName, LPSTR lpVolumeNameBuffer, DWORD nVolumeNameSize, LPDWORD lpVolumeSerialNumber, LPDWORD lpMaximumComponentLength, LPDWORD lpFileSystemFlags, LPSTR lpFileSystemNameBuffer, DWORD nFileSystemNameSize) {
    (void)lpRootPathName;
    if (lpVolumeNameBuffer && nVolumeNameSize > 0) lpVolumeNameBuffer[0] = '\0';
    if (lpVolumeSerialNumber) *lpVolumeSerialNumber = 0x12345678;
    if (lpMaximumComponentLength) *lpMaximumComponentLength = 255;
    if (lpFileSystemFlags) *lpFileSystemFlags = 0;
    if (lpFileSystemNameBuffer && nFileSystemNameSize > 0) lpFileSystemNameBuffer[0] = '\0';
    return TRUE;
}
#ifndef GetVolumeInformation
#define GetVolumeInformation GetVolumeInformationA
#endif

typedef struct _MEMORYSTATUS {
    DWORD  dwLength;
    DWORD  dwMemoryLoad;
    SIZE_T dwTotalPhys;
    SIZE_T dwAvailPhys;
    SIZE_T dwTotalPageFile;
    SIZE_T dwAvailPageFile;
    SIZE_T dwTotalVirtual;
    SIZE_T dwAvailVirtual;
} MEMORYSTATUS, *LPMEMORYSTATUS;

static inline void GlobalMemoryStatus(LPMEMORYSTATUS lpBuffer) {
    if (lpBuffer) {
        memset(lpBuffer, 0, sizeof(MEMORYSTATUS));
        lpBuffer->dwLength = sizeof(MEMORYSTATUS);
        lpBuffer->dwTotalPhys = 1024ULL * 1024ULL * 1024ULL;
        lpBuffer->dwAvailPhys = 512ULL * 1024ULL * 1024ULL;
    }
}

// SAL annotation compatibility stubs for DirectXMath / Direct3D
#ifndef _Use_decl_annotations_
#define _Use_decl_annotations_
#endif

#ifndef _Analysis_assume_
#define _Analysis_assume_(expr) ((void)0)
#endif

// Legacy MSVC __min and __max macros used by SpeedTreeLib and EterLib
#ifndef __min
#define __min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef __max
#define __max(a,b) (((a) > (b)) ? (a) : (b))
#endif

// MSVC x86 floating-point control stubs (_controlfp / _control87) for ARM64 Android
#ifndef _MCW_PC
#define _MCW_EM         0x0008001F
#define _MCW_IC         0x00040000
#define _MCW_RC         0x00000300
#define _MCW_PC         0x00030000
#define _MCW_DN         0x03000000

#define _PC_24          0x00020000
#define _PC_53          0x00010000
#define _PC_64          0x00000000

#define _RC_NEAR        0x00000000
#define _RC_DOWN        0x00000100
#define _RC_UP          0x00000200
#define _RC_CHOP        0x00000300

#define _EM_INVALID     0x00000010
#define _EM_DENORMAL    0x00080000
#define _EM_ZERODIVIDE  0x00000008
#define _EM_OVERFLOW    0x00000004
#define _EM_UNDERFLOW   0x00000002
#define _EM_INEXACT     0x00000001
#endif

static inline unsigned int _controlfp(unsigned int new_val, unsigned int mask) {
    (void)new_val;
    (void)mask;
    return 0;
}

static inline unsigned int _control87(unsigned int new_val, unsigned int mask) {
    (void)new_val;
    (void)mask;
    return 0;
}

static inline unsigned int _clearfp(void) {
    return 0;
}

static inline unsigned int _statusfp(void) {
    return 0;
}

static inline void _fpreset(void) {}

// 7. Win32 GDI Painting & Raster Operation stubs for MovieMan.cpp
#ifndef SRCCOPY
#define SRCCOPY     0x00CC0020
#define BLACKNESS   0x00000042
#define WHITENESS   0x00FF0062
#endif

typedef struct tagPAINTSTRUCT {
    HDC  hdc;
    BOOL fErase;
    RECT rcPaint;
    BOOL fRestore;
    BOOL fIncUpdate;
    BYTE rgbReserved[32];
} PAINTSTRUCT, *PPAINTSTRUCT, *LPPAINTSTRUCT;

static inline HDC BeginPaint(HWND hWnd, LPPAINTSTRUCT lpPaint) {
    (void)hWnd;
    if (lpPaint) {
        memset(lpPaint, 0, sizeof(PAINTSTRUCT));
        lpPaint->hdc = (HDC)1;
    }
    return (HDC)1;
}

static inline BOOL EndPaint(HWND hWnd, const PAINTSTRUCT *lpPaint) {
    (void)hWnd; (void)lpPaint;
    return TRUE;
}

static inline HBRUSH CreateSolidBrush(COLORREF color) {
    (void)color;
    return (HBRUSH)1;
}

static inline int FillRect(HDC hDC, const RECT *lprc, HBRUSH hbr) {
    (void)hDC; (void)lprc; (void)hbr;
    return 1;
}

static inline BOOL BitBlt(HDC hdc, int x, int y, int cx, int cy, HDC hdcSrc, int x1, int y1, DWORD rop) {
    (void)hdc; (void)x; (void)y; (void)cx; (void)cy; (void)hdcSrc; (void)x1; (void)y1; (void)rop;
    return TRUE;
}

static inline BOOL StretchBlt(HDC hdcDest, int xDest, int yDest, int wDest, int hDest, HDC hdcSrc, int xSrc, int ySrc, int wSrc, int hSrc, DWORD rop) {
    (void)hdcDest; (void)xDest; (void)yDest; (void)wDest; (void)hDest;
    (void)hdcSrc; (void)xSrc; (void)ySrc; (void)wSrc; (void)hSrc; (void)rop;
    return TRUE;
}

// 8. Legacy DirectDraw (ddraw.h) stubs for MovieMan.cpp
#ifndef DDSCL_NORMAL
#define DDSCL_NORMAL            0x00000008
#define DDSCL_FULLSCREEN        0x00000001
#define DDSCL_EXCLUSIVE         0x00000010
#endif

#ifndef DDSD_CAPS
#define DDSD_CAPS               0x00000001
#define DDSD_HEIGHT             0x00000002
#define DDSD_WIDTH              0x00000004
#define DDSD_PITCH              0x00000008
#define DDSD_PIXELFORMAT        0x00001000
#endif

#ifndef DDSCAPS_PRIMARYSURFACE
#define DDSCAPS_PRIMARYSURFACE  0x00000200
#define DDSCAPS_OFFSCREENPLAIN  0x00000040
#define DDSCAPS_SYSTEMMEMORY    0x00000800
#define DDSCAPS_VIDEOMEMORY     0x00004000
#endif

#ifndef DDBLT_WAIT
#define DDBLT_COLORFILL         0x00000400
#define DDBLT_WAIT              0x01000000
#endif

typedef struct _DDSCAPS {
    DWORD dwCaps;
} DDSCAPS, *LPDDSCAPS;

typedef struct _DDPIXELFORMAT {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwFourCC;
    DWORD dwRGBBitCount;
    DWORD dwRBitMask;
    DWORD dwGBitMask;
    DWORD dwBBitMask;
    DWORD dwRGBAlphaBitMask;
} DDPIXELFORMAT, *LPDDPIXELFORMAT;

typedef struct _DDSURFACEDESC {
    DWORD         dwSize;
    DWORD         dwFlags;
    DWORD         dwHeight;
    DWORD         dwWidth;
    LONG          lPitch;
    DWORD         dwBackBufferCount;
    DWORD         dwRefreshRate;
    DWORD         dwAlphaBitDepth;
    DWORD         dwReserved;
    LPVOID        lpSurface;
    DDSCAPS       ddsCaps;
    DDPIXELFORMAT ddpfPixelFormat;
} DDSURFACEDESC, *LPDDSURFACEDESC;

typedef struct _DDBLTFX {
    DWORD dwSize;
    DWORD dwDDFX;
    DWORD dwROP;
    DWORD dwDDROP;
    DWORD dwRotationAngle;
    DWORD dwZBufferOpCode;
    DWORD dwZBufferLow;
    DWORD dwZBufferHigh;
    DWORD dwZBufferBaseDest;
    DWORD dwZDestConstBitDepth;
    DWORD dwFillColor;
} DDBLTFX, *LPDDBLTFX;

struct IDirectDrawClipper : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE SetHWnd(DWORD dwFlags, HWND hWnd) = 0;
};
typedef IDirectDrawClipper* LPDIRECTDRAWCLIPPER;

struct IDirectDrawSurface : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE Blt(LPRECT lpDestRect, IDirectDrawSurface *lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx) = 0;
    virtual HRESULT STDMETHODCALLTYPE BltFast(DWORD dwX, DWORD dwY, IDirectDrawSurface *lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwTrans) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetDC(HDC *lphDC) = 0;
    virtual HRESULT STDMETHODCALLTYPE ReleaseDC(HDC hDC) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetSurfaceDesc(LPDDSURFACEDESC lpDDSurfaceDesc) = 0;
    virtual HRESULT STDMETHODCALLTYPE Lock(LPRECT lpDestRect, LPDDSURFACEDESC lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent) = 0;
    virtual HRESULT STDMETHODCALLTYPE Unlock(LPVOID lpSurfaceData) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetClipper(LPDIRECTDRAWCLIPPER lpDDClipper) = 0;
    virtual HRESULT STDMETHODCALLTYPE Restore(void) = 0;
    virtual HRESULT STDMETHODCALLTYPE IsLost(void) = 0;
};
typedef IDirectDrawSurface* LPDIRECTDRAWSURFACE;

struct IDirectDraw : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE SetCooperativeLevel(HWND hWnd, DWORD dwFlags) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateSurface(LPDDSURFACEDESC lpDDSurfaceDesc, LPDIRECTDRAWSURFACE *lplpDDSurface, IUnknown *pUnkOuter) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateClipper(DWORD dwFlags, LPDIRECTDRAWCLIPPER *lplpDDClipper, IUnknown *pUnkOuter) = 0;
};
typedef IDirectDraw* LPDIRECTDRAW;

static inline HRESULT DirectDrawCreate(GUID *lpGUID, LPDIRECTDRAW *lplpDD, IUnknown *pUnkOuter) {
    (void)lpGUID; (void)pUnkOuter;
    if (lplpDD) *lplpDD = NULL;
    return E_NOINTERFACE;
}

// windowsx.h Window Style & Menu helpers for PythonApplicationEvent.cpp
#ifndef GetWindowStyle
#define GetWindowStyle(hwnd)    ((DWORD)GetWindowLong(hwnd, GWL_STYLE))
#endif

#ifndef GetWindowExStyle
#define GetWindowExStyle(hwnd)  ((DWORD)GetWindowLong(hwnd, GWL_EXSTYLE))
#endif

static inline HMENU GetMenu(HWND hWnd) {
    (void)hWnd;
    return NULL;
}

// Complete DirectInput (dinput.h) Scancodes & Stubs for PythonApplicationEvent.cpp and Input modules
#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif

#ifndef DIK_ESCAPE
#define DIK_ESCAPE          0x01
#define DIK_1               0x02
#define DIK_2               0x03
#define DIK_3               0x04
#define DIK_4               0x05
#define DIK_5               0x06
#define DIK_6               0x07
#define DIK_7               0x08
#define DIK_8               0x09
#define DIK_9               0x0A
#define DIK_0               0x0B
#define DIK_MINUS           0x0C
#define DIK_EQUALS          0x0D
#define DIK_BACK            0x0E
#define DIK_TAB             0x0F
#define DIK_Q               0x10
#define DIK_W               0x11
#define DIK_E               0x12
#define DIK_R               0x13
#define DIK_T               0x14
#define DIK_Y               0x15
#define DIK_U               0x16
#define DIK_I               0x17
#define DIK_O               0x18
#define DIK_P               0x19
#define DIK_LBRACKET        0x1A
#define DIK_RBRACKET        0x1B
#define DIK_RETURN          0x1C
#define DIK_LCONTROL        0x1D
#define DIK_A               0x1E
#define DIK_S               0x1F
#define DIK_D               0x20
#define DIK_F               0x21
#define DIK_G               0x22
#define DIK_H               0x23
#define DIK_J               0x24
#define DIK_K               0x25
#define DIK_L               0x26
#define DIK_SEMICOLON       0x27
#define DIK_APOSTROPHE      0x28
#define DIK_GRAVE           0x29
#define DIK_LSHIFT          0x2A
#define DIK_BACKSLASH       0x2B
#define DIK_Z               0x2C
#define DIK_X               0x2D
#define DIK_C               0x2E
#define DIK_V               0x2F
#define DIK_B               0x30
#define DIK_N               0x31
#define DIK_M               0x32
#define DIK_COMMA           0x33
#define DIK_PERIOD          0x34
#define DIK_SLASH           0x35
#define DIK_RSHIFT          0x36
#define DIK_MULTIPLY        0x37
#define DIK_LMENU           0x38
#define DIK_SPACE           0x39
#define DIK_CAPITAL         0x3A
#define DIK_F1              0x3B
#define DIK_F2              0x3C
#define DIK_F3              0x3D
#define DIK_F4              0x3E
#define DIK_F5              0x3F
#define DIK_F6              0x40
#define DIK_F7              0x41
#define DIK_F8              0x42
#define DIK_F9              0x43
#define DIK_F10             0x44
#define DIK_NUMLOCK         0x45
#define DIK_SCROLL          0x46
#define DIK_NUMPAD7         0x47
#define DIK_NUMPAD8         0x48
#define DIK_NUMPAD9         0x49
#define DIK_SUBTRACT        0x4A
#define DIK_NUMPAD4         0x4B
#define DIK_NUMPAD5         0x4C
#define DIK_NUMPAD6         0x4D
#define DIK_ADD             0x4E
#define DIK_NUMPAD1         0x4F
#define DIK_NUMPAD2         0x50
#define DIK_NUMPAD3         0x51
#define DIK_NUMPAD0         0x52
#define DIK_DECIMAL         0x53
#define DIK_F11             0x57
#define DIK_F12             0x58
#define DIK_NUMPADENTER     0x9C
#define DIK_RCONTROL        0x9D
#define DIK_NUMPADCOMMA     0xB3
#define DIK_DIVIDE          0xB5
#define DIK_SYSRQ           0xB7
#define DIK_RMENU           0xB8
#define DIK_PAUSE           0xC5
#define DIK_HOME            0xC7
#define DIK_UP              0xC8
#define DIK_PGUP            0xC9
#define DIK_LEFT            0xCB
#define DIK_RIGHT           0xCD
#define DIK_END             0xCF
#define DIK_DOWN            0xD0
#define DIK_PGDN            0xD1
#define DIK_INSERT          0xD2
#define DIK_DELETE          0xD3
#define DIK_LWIN            0xDB
#define DIK_RWIN            0xDC
#define DIK_APPS            0xDD

// DirectInput alternate key aliases used in Metin2 PythonApplicationModule
#define DIK_BACKSPACE       DIK_BACK
#define DIK_NUMPADSTAR      DIK_MULTIPLY
#define DIK_LALT            DIK_LMENU
#define DIK_CAPSLOCK        DIK_CAPITAL
#define DIK_NUMPADMINUS     DIK_SUBTRACT
#define DIK_NUMPADPLUS      DIK_ADD
#define DIK_NUMPADPERIOD    DIK_DECIMAL
#define DIK_NUMPADSLASH     DIK_DIVIDE
#define DIK_RALT            DIK_RMENU
#define DIK_UPARROW         DIK_UP
#define DIK_PGUP            0xC9
#define DIK_LEFTARROW       DIK_LEFT
#define DIK_RIGHTARROW      DIK_RIGHT
#define DIK_DOWNARROW       DIK_DOWN
#define DIK_PGDN            0xD1
#define DIK_PRIOR           DIK_PGUP
#define DIK_NEXT            DIK_PGDN
#endif

// DirectInput8 data structures (IDirectInput8 & LPDIRECTINPUT8 are provided by GrpOpenGL.h)
#ifndef DI_OK
#define DI_OK                           S_OK
#define DIERR_INPUTLOST                 0x8007001EL
#define DIERR_NOTACQUIRED               0x8007000CL
#define DIDFT_BUTTON                    0x0000000C
#define DIDFT_AXIS                      0x00000003
#define DIDFT_ANYINSTANCE               0x00FFFF00
#endif

typedef struct DIDEVICEOBJECTDATA {
    DWORD       dwOfs;
    DWORD       dwData;
    DWORD       dwTimeStamp;
    DWORD       dwSequence;
    UINT_PTR    uAppData;
} DIDEVICEOBJECTDATA, *LPDIDEVICEOBJECTDATA;

typedef struct _DIMOUSESTATE {
    LONG    lX;
    LONG    lY;
    LONG    lZ;
    BYTE    rgbButtons[4];
} DIMOUSESTATE, *LPDIMOUSESTATE;

typedef struct _DIMOUSESTATE2 {
    LONG    lX;
    LONG    lY;
    LONG    lZ;
    BYTE    rgbButtons[8];
} DIMOUSESTATE2, *LPDIMOUSESTATE2;

typedef struct _DIPROPHEADER {
    DWORD   dwSize;
    DWORD   dwHeaderSize;
    DWORD   dwObj;
    DWORD   dwHow;
} DIPROPHEADER, *LPDIPROPHEADER;
typedef const DIPROPHEADER* LPCDIPROPHEADER;

typedef struct _DIPROPDWORD {
    DIPROPHEADER diph;
    DWORD        dwData;
} DIPROPDWORD, *LPDIPROPDWORD;

#ifndef DIPH_DEVICE
#define DIPH_DEVICE     0
#define DIPROP_BUFFERSIZE ((const void*)(size_t)1)
#endif

// Complete PythonApplication.cpp Win32 Compatibility Block

// 1. FindWindowA / FindWindow (Line 1425)
static inline HWND FindWindowA(LPCSTR lpClassName, LPCSTR lpWindowName) {
    (void)lpClassName;
    (void)lpWindowName;
    return NULL;
}
static inline HWND FindWindowW(LPCWSTR lpClassName, LPCWSTR lpWindowName) {
    (void)lpClassName;
    (void)lpWindowName;
    return NULL;
}
#ifndef FindWindow
#define FindWindow FindWindowA
#endif

// 2. STICKYKEYS & SystemParametersInfo flags (Lines 1564-1570, 1732-1736)
#ifndef SPI_GETSTICKYKEYS
#define SPI_GETSTICKYKEYS 0x003A
#define SPI_SETSTICKYKEYS 0x003B
#endif

#ifndef SKF_AVAILABLE
#define SKF_STICKYKEYSON  0x00000001
#define SKF_AVAILABLE     0x00000002
#define SKF_HOTKEYACTIVE  0x00000004
#define SKF_CONFIRMHOTKEY 0x00000008
#define SKF_HOTKEYSOUND   0x00000010
#define SKF_INDICATOR     0x00000020
#define SKF_AUDIBLEFEEDBACK 0x00000040
#define SKF_TRISTATE      0x00000080
#define SKF_TWOKEYSOFF    0x00000100
#endif

typedef struct tagSTICKYKEYS {
    DWORD cbSize;
    DWORD dwFlags;
} STICKYKEYS, *LPSTICKYKEYS;

// 3. Alphanumeric Virtual Key Codes (Line 547: VK_Z)
#ifndef VK_0
#define VK_0 0x30
#define VK_1 0x31
#define VK_2 0x32
#define VK_3 0x33
#define VK_4 0x34
#define VK_5 0x35
#define VK_6 0x36
#define VK_7 0x37
#define VK_8 0x38
#define VK_9 0x39
#define VK_A 0x41
#define VK_B 0x42
#define VK_C 0x43
#define VK_D 0x44
#define VK_E 0x45
#define VK_F 0x46
#define VK_G 0x47
#define VK_H 0x48
#define VK_I 0x49
#define VK_J 0x4A
#define VK_K 0x4B
#define VK_L 0x4C
#define VK_M 0x4D
#define VK_N 0x4E
#define VK_O 0x4F
#define VK_P 0x50
#define VK_Q 0x51
#define VK_R 0x52
#define VK_S 0x53
#define VK_T 0x54
#define VK_U 0x55
#define VK_V 0x56
#define VK_W 0x57
#define VK_X 0x58
#define VK_Y 0x59
#define VK_Z 0x5A
#endif

// 4. BITMAP, GetObject, Layered Window & Splash constants (Lines 53-84)
#ifndef _BITMAP_DEFINED
#define _BITMAP_DEFINED
typedef struct tagBITMAP {
    LONG   bmType;
    LONG   bmWidth;
    LONG   bmHeight;
    LONG   bmWidthBytes;
    WORD   bmPlanes;
    WORD   bmBitsPixel;
    LPVOID bmBits;
} BITMAP, *PBITMAP, *NPBITMAP, *LPBITMAP;
#endif

static inline int GetObjectA(HANDLE h, int c, LPVOID pv) {
    (void)h;
    if (pv && c > 0) memset(pv, 0, (size_t)c);
    return c;
}
#ifndef GetObject
#define GetObject GetObjectA
#endif

#ifndef SS_BITMAP
#define SS_BITMAP      0x0000000EL
#define STM_SETIMAGE   0x0172
#endif

#ifndef LWA_COLORKEY
#define LWA_COLORKEY   0x00000001
#define LWA_ALPHA      0x00000002
#endif

static inline BOOL SetLayeredWindowAttributes(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags) {
    (void)hwnd; (void)crKey; (void)bAlpha; (void)dwFlags;
    return TRUE;
}

// 5. Window Display Affinity (Lines 202-216)
#ifndef WDA_NONE
#define WDA_NONE       0x00000000
#define WDA_MONITOR    0x00000001
#endif

static inline BOOL SetWindowDisplayAffinity(HWND hWnd, DWORD dwAffinity) {
    (void)hWnd; (void)dwAffinity;
    return TRUE;
}

static inline BOOL GetWindowDisplayAffinity(HWND hWnd, DWORD *pdwAffinity) {
    (void)hWnd;
    if (pdwAffinity) *pdwAffinity = WDA_NONE;
    return TRUE;
}

// _alloca compatibility & Full DirectInput Multimedia / Extended Scancodes for PythonApplicationModule.cpp
#include <alloca.h>
#ifndef _alloca
#define _alloca alloca
#endif

#ifndef DIK_NEXTTRACK
#define DIK_F13             0x64
#define DIK_F14             0x65
#define DIK_F15             0x66
#define DIK_KANA            0x70
#define DIK_ABNT_C1         0x73
#define DIK_CONVERT         0x79
#define DIK_NOCONVERT       0x7B
#define DIK_YEN             0x7D
#define DIK_ABNT_C2         0x7E
#define DIK_NUMPADEQUALS    0x8D
#define DIK_PREVTRACK       0x90
#define DIK_CIRCUMFLEX      0x90
#define DIK_AT              0x91
#define DIK_COLON           0x92
#define DIK_UNDERLINE       0x93
#define DIK_KANJI           0x94
#define DIK_STOP            0x95
#define DIK_AX              0x96
#define DIK_UNLABELED       0x97
#define DIK_NEXTTRACK       0x99
#define DIK_MUTE            0xA0
#define DIK_CALCULATOR      0xA1
#define DIK_PLAYPAUSE       0xA2
#define DIK_MEDIASTOP       0xA4
#define DIK_VOLUMEDOWN      0xAE
#define DIK_VOLUMEUP        0xB0
#define DIK_WEBHOME         0xB2
#define DIK_POWER           0xDE
#define DIK_SLEEP           0xDF
#define DIK_WAKE            0xE3
#define DIK_WEBSEARCH       0xE5
#define DIK_WEBFAVORITES    0xE6
#define DIK_WEBREFRESH      0xE7
#define DIK_WEBSTOP         0xE8
#define DIK_WEBFORWARD      0xE9
#define DIK_WEBBACK         0xEA
#define DIK_MYCOMPUTER      0xEB
#define DIK_MAIL            0xEC
#define DIK_MEDIASELECT     0xED
#endif

// Complete PythonApplicationProcedure.cpp & PythonSystem Display/Window Procedure Block

// 1. DEVMODE & Display Settings (Lines 28-45, plus EnumDisplaySettings for PythonSystem.cpp)
#ifndef CCHDEVICENAME
#define CCHDEVICENAME 32
#define CCHFORMNAME   32
#endif

#ifndef _DEVMODEA_DEFINED
#define _DEVMODEA_DEFINED
typedef struct _devicemodeA {
    BYTE  dmDeviceName[CCHDEVICENAME];
    WORD  dmSpecVersion;
    WORD  dmDriverVersion;
    WORD  dmSize;
    WORD  dmDriverExtra;
    DWORD dmFields;
    union {
        struct {
            short dmOrientation;
            short dmPaperSize;
            short dmPaperLength;
            short dmPaperWidth;
            short dmScale;
            short dmCopies;
            short dmDefaultSource;
            short dmPrintQuality;
        };
        POINT dmPosition;
    };
    short dmColor;
    short dmDuplex;
    short dmYResolution;
    short dmTTOption;
    short dmCollate;
    BYTE  dmFormName[CCHFORMNAME];
    WORD  dmLogPixels;
    DWORD dmBitsPerPel;
    DWORD dmPelsWidth;
    DWORD dmPelsHeight;
    DWORD dmDisplayFlags;
    DWORD dmDisplayFrequency;
} DEVMODEA, *PDEVMODEA, *NPDEVMODEA, *LPDEVMODEA;
typedef DEVMODEA DEVMODE;
typedef PDEVMODEA PDEVMODE;
typedef LPDEVMODEA LPDEVMODE;
#endif

#ifndef DM_BITSPERPEL
#define DM_BITSPERPEL       0x00040000L
#define DM_PELSWIDTH        0x00080000L
#define DM_PELSHEIGHT       0x00100000L
#define DM_DISPLAYFLAGS     0x00200000L
#define DM_DISPLAYFREQUENCY 0x00400000L
#endif

#ifndef CDS_FULLSCREEN
#define CDS_UPDATEREGISTRY  0x00000001
#define CDS_TEST            0x00000002
#define CDS_FULLSCREEN      0x00000004
#endif

#ifndef DISP_CHANGE_SUCCESSFUL
#define DISP_CHANGE_SUCCESSFUL  0
#define DISP_CHANGE_RESTART     1
#define DISP_CHANGE_FAILED     (-1)
#define DISP_CHANGE_BADMODE    (-2)
#endif

#ifndef ENUM_CURRENT_SETTINGS
#define ENUM_CURRENT_SETTINGS  ((DWORD)-1)
#define ENUM_REGISTRY_SETTINGS ((DWORD)-2)
#endif

static inline LONG ChangeDisplaySettingsA(DEVMODEA *lpDevMode, DWORD dwFlags) {
    (void)lpDevMode;
    (void)dwFlags;
    return DISP_CHANGE_SUCCESSFUL;
}
#ifndef ChangeDisplaySettings
#define ChangeDisplaySettings ChangeDisplaySettingsA
#endif

static inline BOOL EnumDisplaySettingsA(LPCSTR lpszDeviceName, DWORD iModeNum, DEVMODEA *lpDevMode) {
    (void)lpszDeviceName;
    if (!lpDevMode) return FALSE;
    if (iModeNum == ENUM_CURRENT_SETTINGS || iModeNum == 0) {
        memset(lpDevMode, 0, sizeof(DEVMODEA));
        lpDevMode->dmSize = sizeof(DEVMODEA);
        lpDevMode->dmBitsPerPel = 32;
        lpDevMode->dmPelsWidth = 1280;
        lpDevMode->dmPelsHeight = 720;
        lpDevMode->dmDisplayFrequency = 60;
        return TRUE;
    }
    return FALSE;
}
#ifndef EnumDisplaySettings
#define EnumDisplaySettings EnumDisplaySettingsA
#endif

// 2. Window Procedure Messages, System Commands, IME, Mouse Wheel & Sizing Constants
#ifndef WM_CANCELMODE
#define WM_CANCELMODE       0x001F
#endif
#ifndef WM_INPUTLANGCHANGE
#define WM_INPUTLANGCHANGE  0x0051
#endif
#ifndef WM_ENTERSIZEMOVE
#define WM_ENTERSIZEMOVE    0x0231
#define WM_EXITSIZEMOVE     0x0232
#endif

#ifndef SC_KEYMENU
#define SC_SIZE             0xF000
#define SC_MOVE             0xF010
#define SC_MINIMIZE         0xF020
#define SC_MAXIMIZE         0xF030
#define SC_CLOSE            0xF060
#define SC_KEYMENU          0xF100
#define SC_RESTORE          0xF120
#define SC_SCREENSAVE       0xF140
#define SC_MONITORPOWER     0xF170
#endif

#ifndef WA_INACTIVE
#define WA_INACTIVE         0
#define WA_ACTIVE           1
#define WA_CLICKACTIVE      2
#endif

#ifndef ISC_SHOWUICOMPOSITIONWINDOW
#define ISC_SHOWUICOMPOSITIONWINDOW     0x80000000
#define ISC_SHOWUIGUIDELINE             0x40000000
#define ISC_SHOWUIALLCANDIDATEWINDOW    0x0000000F
#define ISC_SHOWUICANDIDATEWINDOW       0x00000001
#define ISC_SHOWUIALL                   0xC000000F
#endif

#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif
#ifndef GET_WHEEL_DELTA_WPARAM
#define GET_WHEEL_DELTA_WPARAM(wParam) ((short)HIWORD(wParam))
#endif

#ifndef SIZE_RESTORED
#define SIZE_RESTORED       0
#define SIZE_MINIMIZED      1
#define SIZE_MAXIMIZED      2
#define SIZE_MAXSHOW        3
#define SIZE_MAXHIDE        4
#endif

#ifndef HTCLIENT
#define HTERROR             (-2)
#define HTTRANSPARENT       (-1)
#define HTNOWHERE           0
#define HTCLIENT            1
#define HTCAPTION           2
#define HTSYSMENU           3
#define HTGROWBOX           4
#define HTMENU              5
#endif

// Legacy MSVC/POSIX case-insensitive string comparison aliases for PythonNetworkStreamCommand.cpp
#ifndef strcmpi
#define strcmpi strcasecmp
#endif

#ifndef _strcmpi
#define _strcmpi strcasecmp
#endif

#ifndef strncmpi
#define strncmpi strncasecmp
#endif

#ifndef _wcsicmp
#define _wcsicmp wcscasecmp
#endif

#ifndef wcsicmp
#define wcsicmp wcscasecmp
#endif

#ifndef _wcsnicmp
#define _wcsnicmp wcsncasecmp
#endif

#ifndef wcsnicmp
#define wcsnicmp wcsncasecmp
#endif

// Additional System Metrics constants for PythonSystem.cpp
#ifndef SM_CXFULLSCREEN
#define SM_CXFULLSCREEN 16
#define SM_CYFULLSCREEN 17
#endif

// MSVC safe file open (fopen_s) compatibility stub for msl/file_ptr.h and client modules
#include <stdio.h>
#include <errno.h>

#ifndef _ERRNO_T_DEFINED
#define _ERRNO_T_DEFINED
typedef int errno_t;
#endif

static inline errno_t fopen_s(FILE** pFile, const char* filename, const char* mode) {
    if (!pFile) return EINVAL;
    *pFile = nullptr;
    if (!filename || !mode) return EINVAL;
    *pFile = fopen(filename, mode);
    return (*pFile != nullptr) ? 0 : errno;
}

// MSVC POSIX I/O & Console compatibility stubs for fmt/format-inl.h
#include <stdio.h>
#include <unistd.h>

#ifndef _fileno
#define _fileno(f) fileno(f)
#endif

#ifndef _isatty
#define _isatty(fd) isatty(fd)
#endif

static inline intptr_t _get_osfhandle(int fd) {
    return (intptr_t)fd;
}

#if __SIZEOF_LONG__ == 4
typedef unsigned long __compat_console_dword;
#else
typedef unsigned int __compat_console_dword;
#endif

extern "C" inline int WriteConsoleW(void* hConsoleOutput, const void* lpBuffer, __compat_console_dword nNumberOfCharsToWrite, __compat_console_dword* lpNumberOfCharsWritten, void* lpReserved) {
    if (lpNumberOfCharsWritten) *lpNumberOfCharsWritten = nNumberOfCharsToWrite;
    return 1;
}

// Win32 Console stubs for EterBase/Debug.cpp
static inline BOOL AllocConsole(void) {
    return TRUE;
}

static inline BOOL FreeConsole(void) {
    return TRUE;
}

static inline BOOL SetConsoleTitleA(LPCSTR lpConsoleTitle) {
    (void)lpConsoleTitle;
    return TRUE;
}
#ifndef SetConsoleTitle
#define SetConsoleTitle SetConsoleTitleA
#endif

// Win32 GetFileSize disambiguation for EterBase/CRC32.cpp & FileBase.cpp
#ifdef GetFileSize
#undef GetFileSize
#endif

// Win32 File Mapping & Handle Emulation stubs for EterBase/CRC32.cpp & MappedFile.cpp
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef FILE_MAP_READ
#define FILE_MAP_READ 0x0004
#endif
#ifndef PAGE_READONLY
#define PAGE_READONLY 0x02
#endif

struct __AndroidPersistentPackEntry {
    char path[512];
    int fd;
    size_t fileSize;
    void* fullMmapBase;
    bool mmapAttempted;
};

static inline __AndroidPersistentPackEntry* __GetPersistentPackTable() {
    static __AndroidPersistentPackEntry s_packs[128];
    return s_packs;
}

static inline pthread_mutex_t* __GetAndroidMmapMutex() {
    static pthread_mutex_t s_mutex = PTHREAD_MUTEX_INITIALIZER;
    return &s_mutex;
}

static inline bool __IsPersistentPackFd(int fd) {
    if (fd < 0) return false;
    pthread_mutex_lock(__GetAndroidMmapMutex());
    __AndroidPersistentPackEntry* packs = __GetPersistentPackTable();
    bool found = false;
    for (int i = 0; i < 128; ++i) {
        if (packs[i].path[0] != '\0' && packs[i].fd == fd) {
            found = true;
            break;
        }
    }
    pthread_mutex_unlock(__GetAndroidMmapMutex());
    return found;
}

static inline bool __LookupPersistentPackByFd(int fd, size_t* outSize, void** outBase) {
    if (fd < 0) return false;
    pthread_mutex_lock(__GetAndroidMmapMutex());
    __AndroidPersistentPackEntry* packs = __GetPersistentPackTable();
    for (int i = 0; i < 128; ++i) {
        if (packs[i].path[0] != '\0' && packs[i].fd == fd) {
            if (!packs[i].mmapAttempted) {
                packs[i].mmapAttempted = true;
#if defined(__LP64__) || defined(_LP64) || (INTPTR_MAX == INT64_MAX)
                if (packs[i].fileSize > 0) {
                    void* base = mmap(NULL, packs[i].fileSize, PROT_READ, MAP_PRIVATE, fd, 0);
                    if (base != MAP_FAILED && base != NULL) {
                        packs[i].fullMmapBase = base;
                    }
                }
#endif
            }
            if (outSize) *outSize = packs[i].fileSize;
            if (outBase) *outBase = packs[i].fullMmapBase;
            pthread_mutex_unlock(__GetAndroidMmapMutex());
            return true;
        }
    }
    pthread_mutex_unlock(__GetAndroidMmapMutex());
    return false;
}

static inline DWORD WINAPI GetFileSize_Win32(HANDLE hFile, LPDWORD lpFileSizeHigh) {
    if (hFile == INVALID_HANDLE_VALUE || !hFile) return 0xFFFFFFFF;
    int fd = (int)(intptr_t)hFile - 1000;
    if (fd < 0) return 0xFFFFFFFF;

    size_t cachedSize = 0;
    if (__LookupPersistentPackByFd(fd, &cachedSize, NULL)) {
        if (lpFileSizeHigh) {
            *lpFileSizeHigh = (DWORD)(((uint64_t)cachedSize >> 32) & 0xFFFFFFFF);
        }
        return (DWORD)((uint64_t)cachedSize & 0xFFFFFFFF);
    }

    struct stat st;
    if (fstat(fd, &st) == 0) {
        if (lpFileSizeHigh) {
            *lpFileSizeHigh = (DWORD)(((uint64_t)st.st_size >> 32) & 0xFFFFFFFF);
        }
        return (DWORD)((uint64_t)st.st_size & 0xFFFFFFFF);
    }
    return 0xFFFFFFFF;
}
#define GetFileSize GetFileSize_Win32

static inline bool __IsPackArchivePath(const char* normPath) {
    if (!normPath) return false;
    size_t len = strlen(normPath);
    if (len >= 6 && strcasecmp(normPath + len - 6, ".mt010") == 0)
        return true;
    return false;
}

static inline HANDLE CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
    (void)dwShareMode; (void)lpSecurityAttributes; (void)dwFlagsAndAttributes; (void)hTemplateFile;
    if (!lpFileName || !*lpFileName) return INVALID_HANDLE_VALUE;

    char normPath[1024];
    strncpy(normPath, lpFileName, sizeof(normPath) - 1);
    normPath[sizeof(normPath) - 1] = '\0';
    for (char* p = normPath; *p; ++p) {
        if (*p == '\\') *p = '/';
    }

    const bool isReadOnly = ((dwDesiredAccess & GENERIC_WRITE) == 0) &&
                            (dwCreationDisposition == OPEN_EXISTING);
    const bool isPackFile = isReadOnly && __IsPackArchivePath(normPath);

    if (isPackFile) {
        pthread_mutex_lock(__GetAndroidMmapMutex());
        __AndroidPersistentPackEntry* packs = __GetPersistentPackTable();
        for (int i = 0; i < 128; ++i) {
            if (packs[i].path[0] != '\0' && strcmp(packs[i].path, normPath) == 0) {
                int cachedFd = packs[i].fd;
                pthread_mutex_unlock(__GetAndroidMmapMutex());
                return (HANDLE)(intptr_t)(cachedFd + 1000);
            }
        }
        pthread_mutex_unlock(__GetAndroidMmapMutex());
    }

    int flags = O_RDONLY;
    if (dwDesiredAccess & GENERIC_WRITE) {
        flags = O_RDWR;
        if (dwCreationDisposition == CREATE_ALWAYS || dwCreationDisposition == TRUNCATE_EXISTING)
            flags |= O_CREAT | O_TRUNC;
        else if (dwCreationDisposition == OPEN_ALWAYS || dwCreationDisposition == CREATE_NEW)
            flags |= O_CREAT;
    }

    int fd = open(normPath, flags, 0666);
    if (fd < 0) return INVALID_HANDLE_VALUE;

    if (isPackFile && strlen(normPath) < 512) {
        struct stat st;
        if (fstat(fd, &st) == 0 && st.st_size > 0) {
            pthread_mutex_lock(__GetAndroidMmapMutex());
            __AndroidPersistentPackEntry* packs = __GetPersistentPackTable();
            for (int i = 0; i < 128; ++i) {
                if (packs[i].path[0] == '\0') {
                    strncpy(packs[i].path, normPath, sizeof(packs[i].path) - 1);
                    packs[i].path[sizeof(packs[i].path) - 1] = '\0';
                    packs[i].fd = fd;
                    packs[i].fileSize = (size_t)st.st_size;
                    packs[i].fullMmapBase = NULL;
                    packs[i].mmapAttempted = false;
                    break;
                }
            }
            pthread_mutex_unlock(__GetAndroidMmapMutex());
        }
    }

    return (HANDLE)(intptr_t)(fd + 1000);
}
#ifndef CreateFile
#define CreateFile CreateFileA
#endif

static inline HANDLE CreateFileMapping(HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR lpName) {
    (void)lpFileMappingAttributes; (void)flProtect; (void)dwMaximumSizeHigh; (void)dwMaximumSizeLow; (void)lpName;
    if (hFile == INVALID_HANDLE_VALUE || !hFile) return NULL;
    uintptr_t uval = (uintptr_t)hFile;
    if (uval >= 1000 && uval < 500000) {
        return (HANDLE)(uval + 500000);
    }
    return hFile;
}

struct __AndroidMmapEntry {
    void* viewPtr;
    void* mmapBase;
    size_t mmapSize;
};

static inline __AndroidMmapEntry* __GetAndroidMmapTable() {
    static __AndroidMmapEntry s_table[16384];
    return s_table;
}

static inline LPVOID MapViewOfFile(HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap) {
    (void)dwDesiredAccess;
    if (hFileMappingObject == INVALID_HANDLE_VALUE || !hFileMappingObject) return NULL;
    uintptr_t uval = (uintptr_t)hFileMappingObject;
    int fd = -1;
    if (uval >= 500000 && uval < 1000000) fd = (int)(uval - 501000);
    else if (uval >= 1000 && uval < 500000) fd = (int)(uval - 1000);
    if (fd < 0) return NULL;

    off_t offset = ((off_t)dwFileOffsetHigh << 32) | (off_t)dwFileOffsetLow;

    size_t persistentSize = 0;
    void* persistentBase = NULL;
    if (__LookupPersistentPackByFd(fd, &persistentSize, &persistentBase)) {
        if (offset < 0 || (size_t)offset >= persistentSize) return NULL;
        if (persistentBase != NULL) {
            return (void*)((uint8_t*)persistentBase + (size_t)offset);
        }
    }

    size_t fileSize = persistentSize;
    if (fileSize == 0) {
        struct stat st;
        if (fstat(fd, &st) < 0 || st.st_size <= 0) return NULL;
        fileSize = (size_t)st.st_size;
    }

    if (offset < 0 || (size_t)offset >= fileSize) return NULL;

    size_t size = (dwNumberOfBytesToMap > 0) ? (size_t)dwNumberOfBytesToMap : (fileSize - (size_t)offset);
    if (size > (fileSize - (size_t)offset)) {
        size = fileSize - (size_t)offset;
    }

    // Align offset down to OS page size (4 KB on legacy Android, 16 KB on iOS & Android 15+)
    long sysPage = sysconf(_SC_PAGESIZE);
    size_t pageSize = (sysPage > 0) ? (size_t)sysPage : 4096;
    off_t alignedOffset = (offset / (off_t)pageSize) * (off_t)pageSize;
    size_t delta = (size_t)(offset - alignedOffset);
    size_t mapSize = size + delta;

    void* basePtr = mmap(NULL, mapSize, PROT_READ, MAP_PRIVATE, fd, alignedOffset);
    if (basePtr == MAP_FAILED) return NULL;

    void* viewPtr = (void*)((uint8_t*)basePtr + delta);

    pthread_mutex_lock(__GetAndroidMmapMutex());
    __AndroidMmapEntry* table = __GetAndroidMmapTable();
    bool stored = false;
    for (int i = 0; i < 16384; ++i) {
        if (table[i].viewPtr == NULL) {
            table[i].viewPtr = viewPtr;
            table[i].mmapBase = basePtr;
            table[i].mmapSize = mapSize;
            stored = true;
            break;
        }
    }
    pthread_mutex_unlock(__GetAndroidMmapMutex());

    if (!stored) {
        munmap(basePtr, mapSize);
        return NULL;
    }
    return viewPtr;
}

static inline BOOL UnmapViewOfFile(LPCVOID lpBaseAddress) {
    if (!lpBaseAddress) return FALSE;
    void* mmapBase = NULL;
    size_t mmapSize = 0;

    pthread_mutex_lock(__GetAndroidMmapMutex());
    __AndroidPersistentPackEntry* packs = __GetPersistentPackTable();
    for (int i = 0; i < 128; ++i) {
        if (packs[i].fullMmapBase != NULL) {
            const uint8_t* start = (const uint8_t*)packs[i].fullMmapBase;
            const uint8_t* end = start + packs[i].fileSize;
            if ((const uint8_t*)lpBaseAddress >= start && (const uint8_t*)lpBaseAddress < end) {
                pthread_mutex_unlock(__GetAndroidMmapMutex());
                return TRUE;
            }
        }
    }

    __AndroidMmapEntry* table = __GetAndroidMmapTable();
    for (int i = 0; i < 16384; ++i) {
        if (table[i].viewPtr == lpBaseAddress) {
            mmapBase = table[i].mmapBase;
            mmapSize = table[i].mmapSize;
            table[i].viewPtr = NULL;
            table[i].mmapBase = NULL;
            table[i].mmapSize = 0;
            break;
        }
    }
    pthread_mutex_unlock(__GetAndroidMmapMutex());

    if (mmapBase && mmapSize > 0) {
        munmap(mmapBase, mmapSize);
    }
    return TRUE;
}

// Win32 Clipboard & Memory utility stubs for EterBase/CPostIt.cpp
#ifndef _LPWORD_DEFINED
#define _LPWORD_DEFINED
typedef WORD* LPWORD;
#endif

#ifndef CopyMemory
#define CopyMemory(destination, source, length) memcpy((destination), (source), (length))
#endif

static inline UINT RegisterClipboardFormatA(LPCSTR lpszFormat) {
    (void)lpszFormat;
    return 0xC000; // Simulated custom clipboard format atom
}
#ifndef RegisterClipboardFormat
#define RegisterClipboardFormat RegisterClipboardFormatA
#endif


// Win32 File I/O Emulation stubs for EterBase/FileBase.cpp
#ifndef FILE_BEGIN
#define FILE_BEGIN   0
#define FILE_CURRENT 1
#define FILE_END     2
#endif

static inline DWORD SetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod) {
    if (hFile == INVALID_HANDLE_VALUE || !hFile) return (DWORD)-1;
    int fd = (int)(intptr_t)hFile - 1000;
    
    int whence = SEEK_SET;
    if (dwMoveMethod == FILE_CURRENT) whence = SEEK_CUR;
    else if (dwMoveMethod == FILE_END) whence = SEEK_END;
    
    off_t offset = (lpDistanceToMoveHigh) ? ((off_t)*lpDistanceToMoveHigh << 32) | (uint32_t)lDistanceToMove : (off_t)lDistanceToMove;
    off_t res = lseek(fd, offset, whence);
    return (res == (off_t)-1) ? (DWORD)-1 : (DWORD)(res & 0xFFFFFFFF);
}

static inline BOOL WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, void* lpOverlapped) {
    (void)lpOverlapped;
    if (hFile == INVALID_HANDLE_VALUE || !hFile) return FALSE;
    int fd = (int)(intptr_t)hFile - 1000;
    ssize_t written = write(fd, lpBuffer, nNumberOfBytesToWrite);
    if (written < 0) {
        if (lpNumberOfBytesWritten) *lpNumberOfBytesWritten = 0;
        return FALSE;
    }
    if (lpNumberOfBytesWritten) *lpNumberOfBytesWritten = (DWORD)written;
    return TRUE;
}

static inline BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, void* lpOverlapped) {
    (void)lpOverlapped;
    if (hFile == INVALID_HANDLE_VALUE || !hFile) return FALSE;
    int fd = (int)(intptr_t)hFile - 1000;
    ssize_t nread = read(fd, lpBuffer, nNumberOfBytesToRead);
    if (nread < 0) {
        if (lpNumberOfBytesRead) *lpNumberOfBytesRead = 0;
        return FALSE;
    }
    if (lpNumberOfBytesRead) *lpNumberOfBytesRead = (DWORD)nread;
    return TRUE;
}

// Complete DirectInput8 Interface Stubs for EterLib/Input.cpp
#ifndef __DIRECTINPUT8_STUBS_DEFINED__
#define __DIRECTINPUT8_STUBS_DEFINED__

struct IDirectInputDevice8;

struct IDirectInput8 : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE CreateDevice(const GUID&, struct IDirectInputDevice8**, LPVOID) = 0;
};
#ifndef LPDIRECTINPUT8_DEFINED
#define LPDIRECTINPUT8_DEFINED
typedef IDirectInput8* LPDIRECTINPUT8;
#endif

EXTERN_C inline const GUID IID_IDirectInput8 = { 0xbf798030, 0x483a, 0x4da2, { 0xaa, 0x99, 0x5d, 0x64, 0xed, 0x36, 0x97, 0x00 } };

struct IDirectInputDevice8 : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE SetDataFormat(const void*) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetCooperativeLevel(HWND, DWORD) = 0;
    virtual HRESULT STDMETHODCALLTYPE Acquire(void) = 0;
    virtual HRESULT STDMETHODCALLTYPE Unacquire(void) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetDeviceState(DWORD, void*) = 0;
};
#ifndef LPDIRECTINPUTDEVICE8_DEFINED
#define LPDIRECTINPUTDEVICE8_DEFINED
typedef IDirectInputDevice8* LPDIRECTINPUTDEVICE8;
#endif

static inline HRESULT DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, const GUID& riid, LPVOID* ppvOUT, LPVOID punkOuter) {
    (void)hinst; (void)dwVersion; (void)riid; (void)punkOuter;
    if (ppvOUT) *ppvOUT = NULL;
    return E_NOINTERFACE;
}

static const GUID GUID_SysKeyboard = { 0x6f1d2b61, 0xd5a0, 0x11cf, { 0xbf, 0x7d, 0x00, 0xaa, 0x00, 0x69, 0x46, 0xee } };
static const void* const c_dfDIKeyboard = (const void*)1;

#ifndef DISCL_BACKGROUND
#define DISCL_BACKGROUND 0x0008
#define DISCL_NONEXCLUSIVE 0x0002
#define DISCL_FOREGROUND 0x0004
#define DISCL_EXCLUSIVE 0x0001
#endif

#endif

// Win32 imagehlp.h stubs for EterBase/error.cpp under Android
#ifndef _IMAGEHLP_H
#define _IMAGEHLP_H

#ifndef DWORD64
typedef uint64_t DWORD64;
#endif
#ifndef ULONG64
typedef uint64_t ULONG64;
#endif
#ifndef PDWORD64
typedef DWORD64* PDWORD64;
#endif

typedef struct _IMAGEHLP_LINE64 {
    DWORD SizeOfStruct;
    PVOID Key;
    DWORD LineNumber;
    CHAR  FileName[256];
    DWORD64 Address;
} IMAGEHLP_LINE64, *PIMAGEHLP_LINE64;

typedef struct _SYMBOL_INFO {
    ULONG64 Address;
    ULONG   Size;
    ULONG   Flags;
    ULONG   Value;
    ULONG   NameLen;
    ULONG   MaxNameLen;
    CHAR    Name[1];
} SYMBOL_INFO, *PSYMBOL_INFO;

static inline BOOL SymInitialize(HANDLE hProcess, LPCSTR UserSearchPath, BOOL fInvadeProcess) {
    (void)hProcess; (void)UserSearchPath; (void)fInvadeProcess;
    return TRUE;
}

static inline BOOL SymCleanup(HANDLE hProcess) {
    (void)hProcess;
    return TRUE;
}

static inline BOOL SymFromAddr(HANDLE hProcess, DWORD64 Address, PDWORD64 Displacement, PSYMBOL_INFO Symbol) {
    (void)hProcess; (void)Address; (void)Displacement;
    if (Symbol) Symbol->Name[0] = '\0';
    return FALSE;
}

static inline BOOL SymGetLineFromAddr64(HANDLE hProcess, DWORD64 qwAddr, PDWORD pdwDisplacement, PIMAGEHLP_LINE64 Line64) {
    (void)hProcess; (void)qwAddr; (void)pdwDisplacement;
    if (Line64) Line64->FileName[0] = '\0';
    return FALSE;
}

static inline USHORT CaptureStackBackTrace(ULONG FramesToSkip, ULONG FramesToCapture, PVOID *BackTrace, PDWORD BackTraceHash) {
    (void)FramesToSkip; (void)FramesToCapture; (void)BackTrace; (void)BackTraceHash;
    return 0;
}
#endif

// Win32 OLE Automation headers stubs for Android (`oaidl.h`, `ocidl.h`)
#ifndef __OAIDL_H__
#define __OAIDL_H__
#endif

#ifndef __OCIDL_H__
#define __OCIDL_H__
#endif

// Win32 direct.h stubs for EterBase/Utils.cpp under Android
#ifndef _INC_DIRECT
#define _INC_DIRECT

#include <sys/stat.h>
#include <unistd.h>

#ifndef _mkdir
#define _mkdir(path) mkdir(path, 0777)
#endif

#ifndef _rmdir
#define _rmdir rmdir
#endif

#ifndef _chdir
#define _chdir chdir
#endif

#ifndef _getcwd
#define _getcwd getcwd
#endif

#endif

// Win32 GDI & DIB Section stubs for EterLib/GrpDIB.cpp under Android
static inline HDC CreateCompatibleDC(HDC hdc) {
    (void)hdc;
    return (HDC)1;
}

static inline BOOL DeleteDC(HDC hdc) {
    (void)hdc;
    return TRUE;
}

static inline HBITMAP CreateDIBSection(HDC hdc, const BITMAPINFO *pbmi, UINT usage, VOID **ppvBits, HANDLE hSection, DWORD offset) {
    (void)hdc; (void)usage; (void)hSection; (void)offset;
    if (pbmi && ppvBits) {
        int size = pbmi->bmiHeader.biWidth * abs(pbmi->bmiHeader.biHeight) * 4;
        *ppvBits = calloc(1, size > 0 ? size : 1024);
    }
    return (HBITMAP)1;
}

static inline int SetDIBitsToDevice(HDC hdc, int xDest, int yDest, DWORD w, DWORD h, int xSrc, int ySrc, UINT startScan, UINT cLines, const VOID *lpBits, const BITMAPINFO *lpbmi, UINT colorUse) {
    (void)hdc; (void)xDest; (void)yDest; (void)w; (void)h; (void)xSrc; (void)ySrc; (void)startScan; (void)cLines; (void)lpBits; (void)lpbmi; (void)colorUse;
    return 1;
}

static inline BOOL SetTextColor(HDC hdc, COLORREF color) {
    (void)hdc; (void)color;
    return TRUE;
}

static inline BOOL SetBkColor(HDC hdc, COLORREF color) {
    (void)hdc; (void)color;
    return TRUE;
}

static inline BOOL TextOutA(HDC hdc, int nXStart, int nYStart, LPCSTR lpString, int cchString) {
    (void)hdc; (void)nXStart; (void)nYStart; (void)lpString; (void)cchString;
    return TRUE;
}
#ifndef TextOut
#define TextOut TextOutA
#endif

// Win32 Temp path & utility stubs for EterBase/Utils.cpp under Android
#ifndef _S_IWRITE
#define _S_IWRITE 0000200
#endif

#ifndef BOOLEAN
typedef int BOOLEAN;
#endif

static inline DWORD GetTempPathA(DWORD nBufferLength, LPSTR lpBuffer) {
    if (!lpBuffer || nBufferLength == 0) return 0;
    const char* tmpDir = getenv("TMPDIR");
    if (!tmpDir) tmpDir = "/data/local/tmp";
    snprintf(lpBuffer, nBufferLength, "%s/", tmpDir);
    return (DWORD)strlen(lpBuffer);
}
#ifndef GetTempPath
#define GetTempPath GetTempPathA
#endif

static inline UINT GetTempFileNameA(LPCSTR lpPathName, LPCSTR lpPrefixString, UINT uUnique, LPSTR lpTempFileName) {
    (void)uUnique;
    if (!lpPathName || !lpTempFileName) return 0;
    static int s_counter = 0;
    snprintf(lpTempFileName, MAX_PATH, "%s%s_%d.tmp", lpPathName, lpPrefixString ? lpPrefixString : "etb", ++s_counter);
    return 1;
}
#ifndef GetTempFileName
#define GetTempFileName GetTempFileNameA
#endif

static inline char* _ecvt(double value, int count, int* dec, int* sign) {
    static char buf[32];
    if (sign) *sign = (value < 0) ? 1 : 0;
    snprintf(buf, sizeof(buf), "%.*e", count > 0 ? count - 1 : 0, value);
    // Simple parse of exponent for dec
    if (dec) *dec = 1; 
    return buf;
}

// Win32 Minidump & Structured Exception Handling stubs for EterBase/error.cpp under Android
#ifndef _MINIDUMP_H_
#define _MINIDUMP_H_

typedef struct _EXCEPTION_RECORD {
    DWORD ExceptionCode;
    DWORD ExceptionFlags;
    struct _EXCEPTION_RECORD *ExceptionRecord;
    PVOID ExceptionAddress;
    DWORD NumberParameters;
    ULONG_PTR ExceptionInformation[15];
} EXCEPTION_RECORD, *PEXCEPTION_RECORD;

typedef struct _CONTEXT {
    DWORD ContextFlags;
} CONTEXT, *PCONTEXT;

typedef struct _EXCEPTION_POINTERS {
    PEXCEPTION_RECORD ExceptionRecord;
    PCONTEXT ContextRecord;
} EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;

typedef struct _MINIDUMP_EXCEPTION_INFORMATION {
    DWORD ThreadId;
    PEXCEPTION_POINTERS ExceptionPointers;
    BOOL ClientPointers;
} MINIDUMP_EXCEPTION_INFORMATION, *PMINIDUMP_EXCEPTION_INFORMATION;

typedef enum _MINIDUMP_TYPE {
    MiniDumpNormal = 0x00000000,
    MiniDumpWithDataSegs = 0x00000001,
    MiniDumpWithFullMemory = 0x00000002,
    MiniDumpWithHandleData = 0x00000004,
    MiniDumpFilterMemory = 0x00000008,
    MiniDumpScanMemory = 0x00000010,
    MiniDumpWithUnloadedModules = 0x00000020,
    MiniDumpWithIndirectlyReferencedMemory = 0x00000040,
    MiniDumpFilterModulePaths = 0x00000080,
    MiniDumpWithProcessThreadData = 0x00000100,
    MiniDumpWithPrivateReadWriteMemory = 0x00000200,
    MiniDumpWithoutOptionalData = 0x00000400,
    MiniDumpWithFullMemoryInfo = 0x00000800,
    MiniDumpWithThreadInfo = 0x00001000,
    MiniDumpWithCodeSegs = 0x00002000
} MINIDUMP_TYPE;

typedef BOOL (WINAPI *PMINIDUMP_WRITE_DUMP)(
    HANDLE hProcess,
    DWORD ProcessId,
    HANDLE hFile,
    MINIDUMP_TYPE DumpType,
    PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
    void* UserStreamParam,
    void* CallbackParam
);

// Declare MiniDumpWriteDump prototype for decltype(&MiniDumpWriteDump) inside error.cpp
#ifdef __cplusplus
extern "C" {
#endif
BOOL WINAPI MiniDumpWriteDump(
    HANDLE hProcess,
    DWORD ProcessId,
    HANDLE hFile,
    MINIDUMP_TYPE DumpType,
    PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
    void* UserStreamParam,
    void* CallbackParam
);
#ifdef __cplusplus
}
#endif

#ifndef EXCEPTION_EXECUTE_HANDLER
#define EXCEPTION_EXECUTE_HANDLER 1
#endif

static inline DWORD GetCurrentThreadId(void) {
    return (DWORD)(uintptr_t)pthread_self();
}

typedef BOOL (WINAPI *PMINIDUMP_WRITE_DUMP)(
    HANDLE hProcess,
    DWORD ProcessId,
    HANDLE hFile,
    MINIDUMP_TYPE DumpType,
    PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
    void* UserStreamParam,
    void* CallbackParam
);

static inline int SetBkMode(HDC hdc, int iBkMode) {
    (void)hdc; (void)iBkMode;
    return 1;
}

#endif

// Win32 Multithreading stubs for EterLib/Thread.cpp under Android
#ifndef _BEGINTHREADEX_DEFINED
#define _BEGINTHREADEX_DEFINED

#include <pthread.h>

#ifndef __stdcall
#define __stdcall
#endif

typedef unsigned (__stdcall *PTHREAD_START) (void *);


#ifndef THREAD_PRIORITY_NORMAL
#define THREAD_PRIORITY_NORMAL 0
#endif


#endif

// Window sizing and non-client message stubs for MSWindow.cpp under Android
#ifndef WM_GETMINMAXINFO
#define WM_GETMINMAXINFO     0x0024
#define WM_NCLBUTTONDOWN     0x00A1
#define WM_NCLBUTTONDBLCLK   0x00A3
#endif

#ifndef HTLEFT
#define HTERROR             (-2)
#define HTTRANSPARENT       (-1)
#define HTNOWHERE           0
#define HTCLIENT            1
#define HTCAPTION           2
#define HTSYSMENU           3
#define HTGROWBOX           4
#define HTMENU              5
#define HTHSCROLL           6
#define HTVSCROLL           7
#define HTMINBUTTON         8
#define HTMAXBUTTON         9
#define HTLEFT              10
#define HTRIGHT             11
#define HTTOP               12
#define HTTOPLEFT           13
#define HTTOPRIGHT          14
#define HTBOTTOM            15
#define HTBOTTOMLEFT        16
#define HTBOTTOMRIGHT       17
#endif

#ifndef _MINMAXINFO_DEFINED
#define _MINMAXINFO_DEFINED
typedef struct tagMINMAXINFO {
    POINT ptReserved;
    POINT ptMaxSize;
    POINT ptMaxPosition;
    POINT ptMinTrackSize;
    POINT ptMaxTrackSize;
} MINMAXINFO, *PMINMAXINFO, *LPMINMAXINFO;
#endif

// GDI Font weights & styles
#ifndef FW_NORMAL
#define FW_NORMAL         400
#define FW_BOLD           700
#endif

// GDI Font precision, clip precision, quality, and pitch flags
#ifndef OUT_DEFAULT_PRECIS
#define OUT_DEFAULT_PRECIS    0
#define CLIP_DEFAULT_PRECIS   0
#define ANTIALIASED_QUALITY   4
#define DEFAULT_PITCH         0
#endif

// GDI Character Set Constants (wingdi.h)
#ifndef SHIFTJIS_CHARSET
#define ANSI_CHARSET            0
#define DEFAULT_CHARSET         1
#define SYMBOL_CHARSET          2
#define SHIFTJIS_CHARSET        128
#define HANGEUL_CHARSET         129
#define HANGUL_CHARSET          129
#define GB2312_CHARSET          134
#define CHINESEBIG5_CHARSET     136
#define OEM_CHARSET             255
#define JOHAB_CHARSET           130
#define HEBREW_CHARSET          177
#define ARABIC_CHARSET          178
#define GREEK_CHARSET           161
#define TURKISH_CHARSET         162
#define VIETNAMESE_CHARSET      163
#define THAI_CHARSET            222
#define EASTEUROPE_CHARSET      238
#define RUSSIAN_CHARSET         204
#define MAC_CHARSET             77
#define BALTIC_CHARSET          186
#endif

// GDI Font Enumeration Types and Stubs
#ifndef _FONTENUMPROC_DEFINED
#define _FONTENUMPROC_DEFINED
typedef int (CALLBACK* FONTENUMPROC)(const LOGFONT*, const TEXTMETRIC*, DWORD, LPARAM);
#endif

typedef LOGFONTA* LPLOGFONTA;

static inline int EnumFontFamiliesExA(HDC hdc, LPLOGFONTA lpLogfont, FONTENUMPROC lpProc, LPARAM lParam, DWORD dwFlags) {
    (void)hdc; (void)lpLogfont; (void)lpProc; (void)lParam; (void)dwFlags;
    return 1; // Return non-zero to indicate continuation/success on stub
}

#ifndef EnumFontFamiliesEx
#define EnumFontFamiliesEx EnumFontFamiliesExA
#endif

// ABCFLOAT structure for font glyph metrics
typedef struct _ABCFLOAT {
    FLOAT abcfA;
    FLOAT abcfB;
    FLOAT abcfC;
} ABCFLOAT, *PABCFLOAT, *LPABCFLOAT;

// Win32 Font and Text Output Stubs for GrpFontTexture.cpp under Android
static inline HFONT CreateFontIndirectA(const LOGFONTA *lplf) { (void)lplf; return (HFONT)1; }
static inline HFONT CreateFontIndirectW(const LOGFONTW *lplf) { (void)lplf; return (HFONT)1; }
#ifndef CreateFontIndirect
#define CreateFontIndirect CreateFontIndirectA
#endif

static inline BOOL GetTextExtentPoint32W(HDC hdc, LPCWSTR lpString, int c, LPSIZE lpsz) {
    (void)hdc; (void)c;
    if (lpString && lpsz) {
        int len = 0;
        while (lpString[len]) len++;
        lpsz->cx = len * 8;
        lpsz->cy = 16;
    }
    return TRUE;
}

static inline BOOL TextOutW(HDC hdc, int nXStart, int nYStart, LPCWSTR lpString, int cchString) {
    (void)hdc; (void)nXStart; (void)nYStart; (void)lpString; (void)cchString;
    return TRUE;
}

static inline BOOL GetCharABCWidthsFloatW(HDC hdc, UINT iFirstChar, UINT iLastChar, LPABCFLOAT lpABCF) {
    (void)hdc; (void)iFirstChar; (void)iLastChar;
    if (lpABCF) {
        lpABCF->abcfA = 0.0f;
        lpABCF->abcfB = 10.0f;
        lpABCF->abcfC = 0.0f;
    }
    return TRUE;
}

// Win32 System Menu flags & stubs for EterLib/MSWindow.cpp under Android
#ifndef MF_STRING
#define MF_INSERT           0x00000000L
#define MF_CHANGE           0x00000080L
#define MF_APPEND           0x00000100L
#define MF_DELETE           0x00000200L
#define MF_REMOVE           0x00001000L
#define MF_BYCOMMAND        0x00000000L
#define MF_BYPOSITION       0x00000400L
#define MF_SEPARATOR        0x00000800L
#define MF_ENABLED          0x00000000L
#define MF_GRAYED           0x00000001L
#define MF_DISABLED         0x00000002L
#define MF_UNCHECKED        0x00000000L
#define MF_CHECKED          0x00000008L
#define MF_STRING           0x00000000L
#endif

static inline HMENU GetSystemMenu(HWND hWnd, BOOL bRevert) {
    (void)hWnd; (void)bRevert;
    return NULL;
}

static inline BOOL ModifyMenuA(HMENU hMnu, UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, LPCSTR lpNewItem) {
    (void)hMnu; (void)uPosition; (void)uFlags; (void)uIDNewItem; (void)lpNewItem;
    return TRUE;
}
#ifndef ModifyMenu
#define ModifyMenu ModifyMenuA
#endif

static inline BOOL AppendMenuA(HMENU hMenu, UINT uFlags, UINT_PTR uIDNewItem, LPCSTR lpNewItem) {
    (void)hMenu; (void)uFlags; (void)uIDNewItem; (void)lpNewItem;
    return TRUE;
}
#ifndef AppendMenu
#define AppendMenu AppendMenuA
#endif

// Win32 & MSVC CRT stubs for UserInterface/UserInterface.cpp under Android

#ifndef swprintf_s
#define swprintf_s swprintf
#endif

static inline int lstrlenA(LPCSTR lpString) {
    return lpString ? (int)strlen(lpString) : 0;
}
static inline int lstrlenW(LPCWSTR lpString) {
    return lpString ? (int)wcslen(lpString) : 0;
}
#ifndef lstrlen
#define lstrlen lstrlenA
#endif

static inline DWORD GetPrivateProfileStringA(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString, DWORD nSize, LPCSTR lpFileName) {
    (void)lpAppName; (void)lpKeyName; (void)lpFileName;
    if (lpReturnedString && nSize > 0) {
        if (lpDefault) {
            strncpy(lpReturnedString, lpDefault, nSize - 1);
            lpReturnedString[nSize - 1] = '\0';
            return (DWORD)strlen(lpReturnedString);
        }
        lpReturnedString[0] = '\0';
    }
    return 0;
}
#ifndef GetPrivateProfileString
#define GetPrivateProfileString GetPrivateProfileStringA
#endif

static inline int LoadStringA(HINSTANCE hInstance, UINT uID, LPSTR lpBuffer, int cchBufferMax) {
    (void)hInstance; (void)uID;
    if (lpBuffer && cchBufferMax > 0) {
        lpBuffer[0] = '\0';
    }
    return 0;
}
#ifndef LoadString
#define LoadString LoadStringA
#endif

static inline HANDLE CreateMutexA(LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCSTR lpName) {
    (void)lpMutexAttributes; (void)bInitialOwner; (void)lpName;
    return (HANDLE)1;
}
#ifndef CreateMutex
#define CreateMutex CreateMutexA
#endif

static inline BOOL ReleaseMutex(HANDLE hMutex) {
    (void)hMutex;
    return TRUE;
}

// Additional MessageBox flags for EterLib/GrpDevice.cpp under Android
#ifndef MB_TOPMOST
#define MB_APPLMODAL        0x00000000L
#define MB_SYSTEMMODAL      0x00001000L
#define MB_TASKMODAL        0x00002000L
#define MB_SETFOREGROUND    0x00010000L
#define MB_DEFAULT_DESKTOP_ONLY 0x00020000L
#define MB_TOPMOST          0x00040000L
#endif

// Win32 _beginthreadex & Semaphore stubs for EterLib/Thread.cpp & FileLoaderThread.cpp under Android
#ifndef _BEGINTHREADEX_DEFINED
#define _BEGINTHREADEX_DEFINED

static inline HANDLE CreateSemaphoreA(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes, LONG lInitialCount, LONG lMaximumCount, LPCSTR lpName) {
    (void)lpSemaphoreAttributes; (void)lInitialCount; (void)lMaximumCount; (void)lpName;
    return (HANDLE)1;
}
#ifndef CreateSemaphore
#define CreateSemaphore CreateSemaphoreA
#endif

static inline BOOL ReleaseSemaphore(HANDLE hSemaphore, LONG lReleaseCount, LONG* lpPreviousCount) {
    (void)hSemaphore; (void)lReleaseCount;
    if (lpPreviousCount) *lpPreviousCount = 0;
    return TRUE;
}

#endif

// GDI Background modes and ANSI GetTextExtentPoint32A for EterLib/TextBar.cpp
#ifndef TRANSPARENT
#define TRANSPARENT 1
#define OPAQUE      2
#endif

static inline BOOL GetTextExtentPoint32A(HDC hdc, LPCSTR lpString, int c, LPSIZE lpsz) {
    (void)hdc;
    if (lpString && lpsz) {
        int len = (c >= 0) ? c : (int)strlen(lpString);
        lpsz->cx = len * 8;
        lpsz->cy = 16;
    }
    return TRUE;
}
#ifndef GetTextExtentPoint32
#define GetTextExtentPoint32 GetTextExtentPoint32A
#endif

// ============================================================================
// IMM32 Composition Attributes (imm.h)
// ============================================================================
#ifndef ATTR_INPUT
#define ATTR_INPUT                  0x00
#define ATTR_TARGET_CONVERTED       0x01
#define ATTR_CONVERTED              0x02
#define ATTR_TARGET_NOTCONVERTED    0x03
#define ATTR_INPUT_ERROR            0x04
#define ATTR_FIXEDCONVERTED         0x05
#endif

// ============================================================================
// NLS String Comparison Flags & Function (winnls.h)
// ============================================================================
#ifndef NORM_IGNORECASE
#define NORM_IGNORECASE             0x00000001
#define NORM_IGNORENONSPACE         0x00000002
#define NORM_IGNORESYMBOLS          0x00000004
#endif

#ifndef CSTR_EQUAL
#define CSTR_LESS_THAN              1
#define CSTR_EQUAL                  2
#define CSTR_GREATER_THAN           3
#endif

static inline int CompareStringA(DWORD Locale, DWORD dwCmpFlags, LPCSTR lpString1, int cchCount1, LPCSTR lpString2, int cchCount2) {
    (void)Locale;
    if (!lpString1 || !lpString2) return 0;
    int res = 0;
    if (dwCmpFlags & NORM_IGNORECASE) {
        if (cchCount1 >= 0 && cchCount2 >= 0) {
            size_t minLen = (size_t)((cchCount1 < cchCount2) ? cchCount1 : cchCount2);
            res = strncasecmp(lpString1, lpString2, minLen);
            if (res == 0 && cchCount1 != cchCount2)
                res = (cchCount1 < cchCount2) ? -1 : 1;
        } else {
            res = strcasecmp(lpString1, lpString2);
        }
    } else {
        if (cchCount1 >= 0 && cchCount2 >= 0) {
            size_t minLen = (size_t)((cchCount1 < cchCount2) ? cchCount1 : cchCount2);
            res = strncmp(lpString1, lpString2, minLen);
            if (res == 0 && cchCount1 != cchCount2)
                res = (cchCount1 < cchCount2) ? -1 : 1;
        } else {
            res = strcmp(lpString1, lpString2);
        }
    }
    if (res < 0) return CSTR_LESS_THAN;
    if (res > 0) return CSTR_GREATER_THAN;
    return CSTR_EQUAL;
}
#ifndef CompareString
#define CompareString CompareStringA
#endif

// ============================================================================
// Version Information Stubs (winver.h)
// ============================================================================
#ifndef _VS_FIXEDFILEINFO_DEFINED
#define _VS_FIXEDFILEINFO_DEFINED
typedef struct tagVS_FIXEDFILEINFO {
    DWORD   dwSignature;
    DWORD   dwStrucVersion;
    DWORD   dwFileVersionMS;
    DWORD   dwFileVersionLS;
    DWORD   dwProductVersionMS;
    DWORD   dwProductVersionLS;
    DWORD   dwFileFlagsMask;
    DWORD   dwFileFlags;
    DWORD   dwFileOS;
    DWORD   dwFileType;
    DWORD   dwFileSubtype;
    DWORD   dwFileDateMS;
    DWORD   dwFileDateLS;
} VS_FIXEDFILEINFO;
#endif

static inline DWORD GetFileVersionInfoSizeA(LPCSTR lptstrFilename, LPDWORD lpdwHandle) {
    (void)lptstrFilename;
    if (lpdwHandle) *lpdwHandle = 0;
    return 0;
}
#ifndef GetFileVersionInfoSize
#define GetFileVersionInfoSize GetFileVersionInfoSizeA
#endif

static inline BOOL GetFileVersionInfoA(LPCSTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData) {
    (void)lptstrFilename; (void)dwHandle; (void)dwLen; (void)lpData;
    return FALSE;
}
#ifndef GetFileVersionInfo
#define GetFileVersionInfo GetFileVersionInfoA
#endif

static inline BOOL VerQueryValueA(LPCVOID pBlock, LPCSTR lpSubBlock, LPVOID *lplpBuffer, PUINT puLen) {
    (void)pBlock; (void)lpSubBlock;
    if (lplpBuffer) *lplpBuffer = NULL;
    if (puLen) *puLen = 0;
    return FALSE;
}
#ifndef VerQueryValue
#define VerQueryValue VerQueryValueA
#endif

// ============================================================================
// Windows Registry Stubs (winreg.h)
// ============================================================================
typedef void* HKEY;
typedef DWORD REGSAM;

#ifndef HKEY_CLASSES_ROOT
#define HKEY_CLASSES_ROOT   ((HKEY)0x80000000)
#define HKEY_CURRENT_USER   ((HKEY)0x80000001)
#define HKEY_LOCAL_MACHINE  ((HKEY)0x80000002)
#define HKEY_USERS          ((HKEY)0x80000003)
#endif

#ifndef KEY_READ
#define KEY_READ            0x20019
#endif

static inline LONG RegOpenKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, HKEY *phkResult) {
    (void)hKey; (void)lpSubKey; (void)ulOptions; (void)samDesired;
    if (phkResult) *phkResult = NULL;
    return 2L; // ERROR_FILE_NOT_FOUND on Android
}
static inline LONG RegQueryValueExA(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData) {
    (void)hKey; (void)lpValueName; (void)lpReserved; (void)lpType; (void)lpData; (void)lpcbData;
    return 2L; // ERROR_FILE_NOT_FOUND
}
static inline LONG RegCloseKey(HKEY hKey) {
    (void)hKey;
    return 0L; // ERROR_SUCCESS
}

// ============================================================================
// COM GUID Comparison & Variant Types (objbase.h / wtypes.h)
// ============================================================================
#ifdef __cplusplus
static inline int IsEqualGUID(const GUID& rguid1, const GUID& rguid2) {
    return !memcmp(&rguid1, &rguid2, sizeof(GUID));
}
static inline int IsEqualIID(const GUID& riid1, const GUID& riid2) {
    return IsEqualGUID(riid1, riid2);
}
#else
#define IsEqualGUID(rguid1, rguid2) (!memcmp(rguid1, rguid2, sizeof(GUID)))
#define IsEqualIID(riid1, riid2) IsEqualGUID(riid1, riid2)
#endif

#ifndef VT_I4
#define VT_EMPTY    0
#define VT_NULL     1
#define VT_I2       2
#define VT_I4       3
#define VT_R4       4
#define VT_R8       5
#define VT_BSTR     8
#define VT_DISPATCH 9
#define VT_ERROR    10
#define VT_BOOL     11
#define VT_VARIANT  12
#define VT_UNKNOWN  13
#endif

// Map LZO max compression to standard MiniLZO compression for Android
#ifndef lzo1x_999_compress
#define lzo1x_999_compress lzo1x_1_compress
#endif

// Missing Text Services Framework CLSID
EXTERN_C inline const GUID CLSID_TF_ThreadMgr = { 0x529a9e6b, 0x6587, 0x4f23, { 0xab, 0x9e, 0x9c, 0x7d, 0x68, 0x3e, 0x3c, 0x50 } };

// Suppress NDK -Werror=format-security across legacy client snprintf/sprintf calls
#pragma clang diagnostic ignored "-Wformat-security"