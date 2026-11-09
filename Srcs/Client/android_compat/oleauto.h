#pragma once
#ifndef __OLEAUTO_H__
#define __OLEAUTO_H__

#include "windows.h"

// COM Method implementation macros
#ifndef STDMETHODIMP
#define STDMETHODCALLTYPE
#define STDMETHODIMP            HRESULT STDMETHODCALLTYPE
#define STDMETHODIMP_(type)     type STDMETHODCALLTYPE
#define STDMETHOD(method)       virtual HRESULT STDMETHODCALLTYPE method
#define STDMETHOD_(type,method) virtual type STDMETHODCALLTYPE method
#define STDAPI                  HRESULT
#define STDAPI_(type)           type
#endif

// Win32 Platform IDs
#ifndef VER_PLATFORM_WIN32_NT
#define VER_PLATFORM_WIN32s         0
#define VER_PLATFORM_WIN32_WINDOWS  1
#define VER_PLATFORM_WIN32_NT       2
#endif

// IMM32 Composition String Action flags
#ifndef SCS_SETSTR
#define SCS_SETSTR                  0x0009
#define SCS_CHANGEATTR              0x0011
#define SCS_CHANGECLAUSE            0x0022
#define SCS_SETRECONVERTSTRING      0x00010000
#define SCS_QUERYRECONVERTSTRING    0x00020000
#endif

// Sublanguage Identifiers
#ifndef SUBLANG_CHINESE_SIMPLIFIED
#define SUBLANG_NEUTRAL             0x00
#define SUBLANG_DEFAULT             0x01
#define SUBLANG_SYS_DEFAULT         0x02
#define SUBLANG_CHINESE_TRADITIONAL 0x01
#define SUBLANG_CHINESE_SIMPLIFIED  0x02
#define SUBLANG_CHINESE_HONGKONG    0x03
#define SUBLANG_CHINESE_SINGAPORE   0x04
#define SUBLANG_CHINESE_MACAU       0x05
#define SUBLANG_ENGLISH_US          0x01
#define SUBLANG_ENGLISH_UK          0x02
#endif

// System Directory stub
static inline UINT GetSystemDirectoryA(LPSTR lpBuffer, UINT uSize) {
    if (!lpBuffer || uSize == 0) return 0;
    const char* sysDir = "/system";
    strncpy(lpBuffer, sysDir, uSize - 1);
    lpBuffer[uSize - 1] = '\0';
    return (UINT)strlen(lpBuffer);
}
#ifndef GetSystemDirectory
#define GetSystemDirectory GetSystemDirectoryA
#endif

// Wide character string stepping
static inline LPWSTR CharNextW(LPCWSTR lpCurrentChar) {
    if (!lpCurrentChar || !*lpCurrentChar) return (LPWSTR)lpCurrentChar;
    return (LPWSTR)(lpCurrentChar + 1);
}

#ifdef __cplusplus
extern "C" {
#endif

// BSTR allocation & manipulation stubs
static inline BSTR SysAllocString(const OLECHAR *psz) {
    if (!psz) return NULL;
    size_t len = wcslen(psz);
    OLECHAR *bstr = (OLECHAR*)malloc((len + 1) * sizeof(OLECHAR));
    if (bstr) wcscpy(bstr, psz);
    return bstr;
}

static inline BSTR SysAllocStringLen(const OLECHAR *strIn, UINT ui) {
    if (!strIn) return NULL;
    OLECHAR *bstr = (OLECHAR*)malloc((ui + 1) * sizeof(OLECHAR));
    if (bstr) {
        wcsncpy(bstr, strIn, ui);
        bstr[ui] = L'\0';
    }
    return bstr;
}

static inline void SysFreeString(BSTR bstr) {
    if (bstr) free(bstr);
}

static inline UINT SysStringLen(BSTR pbstr) {
    return pbstr ? (UINT)wcslen(pbstr) : 0;
}

static inline void VariantInit(VARIANTARG *pvarg) {
    if (pvarg) memset(pvarg, 0, sizeof(VARIANTARG));
}

static inline HRESULT VariantClear(VARIANTARG *pvarg) {
    if (!pvarg) return E_INVALIDARG;
    if (pvarg->vt == 8 && pvarg->bstrVal) {
        SysFreeString(pvarg->bstrVal);
    }
    memset(pvarg, 0, sizeof(VARIANTARG));
    return S_OK;
}

static inline HRESULT VariantCopy(VARIANTARG *pvargDest, const VARIANTARG *pvargSrc) {
    if (!pvargDest || !pvargSrc) return E_INVALIDARG;
    memcpy(pvargDest, pvargSrc, sizeof(VARIANTARG));
    if (pvargSrc->vt == 8 && pvargSrc->bstrVal) {
        pvargDest->bstrVal = SysAllocString(pvargSrc->bstrVal);
    }
    return S_OK;
}

#ifdef __cplusplus
}
#endif

#endif // __OLEAUTO_H__