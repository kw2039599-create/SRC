#pragma once

#ifndef __ANDROID__
  #error "This unknwn.h is only for Android!"
#endif

#include <windows.h>
#include <rpc.h>
#include <rpcndr.h>

#ifndef __UNKNWN_H__
#define __UNKNWN_H__

#ifndef __IUnknown_INTERFACE_DEFINED__
#define __IUnknown_INTERFACE_DEFINED__

#ifndef _GUID_DEFINED
#define _GUID_DEFINED
typedef struct _GUID {
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
} GUID, IID, CLSID;
#else
#ifndef IID
typedef struct _GUID IID;
#endif
#ifndef CLSID
typedef struct _GUID CLSID;
#endif
#endif

typedef struct IUnknown IUnknown;

typedef struct I1UnknownVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IUnknown *This, const IID *riid, void **ppvObject);
    ULONG (STDMETHODCALLTYPE *AddRef)(IUnknown *This);
    ULONG (STDMETHODCALLTYPE *Release)(IUnknown *This);
} I1UnknownVtbl;

struct IUnknown {
    const I1UnknownVtbl *lpVtbl;
};

#endif // __IUnknown_INTERFACE_DEFINED__

// COM & IME type stubs (LANGID, REFCLSID, REFIID)
#ifndef LANGID
typedef WORD LANGID;
#endif

#ifndef _REFCLSID_DEFINED
#define _REFCLSID_DEFINED
#ifdef __cplusplus
typedef const GUID& REFCLSID;
typedef const GUID& REFIID;
#else
typedef const GUID* REFCLSID;
typedef const GUID* REFIID;
#endif
#endif

#endif // __UNKNWN_H__