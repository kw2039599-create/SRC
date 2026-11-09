#pragma once

#ifndef __ANDROID__
  #error "This shlobj.h is only for Android!"
#endif

#include <windows.h>

#ifndef __SHLOBJ_H__
#define __SHLOBJ_H__

#ifndef CSIDL_PERSONAL
#define CSIDL_PERSONAL              0x0005
#define CSIDL_APPDATA               0x001a
#define CSIDL_LOCAL_APPDATA         0x001c
#define CSIDL_COMMON_APPDATA        0x0023
#endif

static inline BOOL SHGetSpecialFolderPathA(HWND hwnd, LPSTR pszPath, int csidl, BOOL fCreate) {
    (void)hwnd; (void)csidl; (void)fCreate;
    if (pszPath) {
        strcpy(pszPath, "/sdcard");
    }
    return TRUE;
}

#ifndef SHGetSpecialFolderPath
#define SHGetSpecialFolderPath SHGetSpecialFolderPathA
#endif

#endif // __SHLOBJ_H__