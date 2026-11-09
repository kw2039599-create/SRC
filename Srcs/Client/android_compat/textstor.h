#pragma once
#ifndef __TEXTSTOR_H__
#define __TEXTSTOR_H__

#include "windows.h"

// Basic Text Store constants and types for Text Services Framework (TSF) stubs
#ifndef TS_E_INVALIDARG
#define TS_E_INVALIDARG         E_INVALIDARG
#define TS_E_INVALIDPOS         0x80040200
#define TS_E_NOLOCK             0x80040201
#define TS_E_NOOBJECT           0x80040202
#define TS_E_NOSERVICE          0x80040203
#define TS_E_NOINTERFACE        E_NOINTERFACE
#define TS_E_NOSELECTION        0x80040204
#define TS_E_NOLAYOUT           0x80040205
#define TS_E_INVALIDPOINT       0x80040206
#define TS_E_SYNCHRONOUS        0x80040207
#define TS_E_READONLY           0x80040208
#define TS_E_FORMAT             0x80040209
#endif

typedef DWORD TsViewCookie;

typedef struct TS_STATUS {
    DWORD dwDynamicFlags;
    DWORD dwStaticFlags;
} TS_STATUS;

typedef struct TS_TEXTCHANGE {
    LONG acpStart;
    LONG acpOldEnd;
    LONG acpNewEnd;
} TS_TEXTCHANGE;

struct ITextStoreACP;
struct ITextStoreACPSink;

#endif // __TEXTSTOR_H__