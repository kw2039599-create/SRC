#pragma once

#ifndef __ANDROID__
  #error "This ole2.h is only for Android!"
#endif

#ifndef __OLE2_H__
#define __OLE2_H__

#include <windows.h>
#include <unknwn.h>

#ifdef __cplusplus
extern "C" {
#endif

// Basic OLE/COM stubs for Android DirectShow compatibility
inline HRESULT OleInitialize(LPVOID pvReserved) { return 0; }
inline void    OleUninitialize(void) {}

#ifdef __cplusplus
}
#endif

#endif // __OLE2_H__