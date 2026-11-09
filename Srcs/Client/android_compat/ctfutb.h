#pragma once
#ifndef __CTFUTB_H__
#define __CTFUTB_H__

#include "windows.h"

// Forward declarations for Text Services Framework Language Bar interfaces
struct ITfLangBarItem;
struct ITfLangBarItemMgr;
struct ITfLangBarEventSink;
struct ITfLangBarItemSink;

struct ITfLangBarMgr : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE AdviseEventSink(void *pSink, HWND hwnd, DWORD dwFlags, DWORD *pdwCookie) = 0;
    virtual HRESULT STDMETHODCALLTYPE UnadviseEventSink(DWORD dwCookie) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetConfig(void **ppConfig) = 0;
    virtual HRESULT STDMETHODCALLTYPE Add(struct ITfLangBarItem *pItem) = 0;
    virtual HRESULT STDMETHODCALLTYPE Remove(struct ITfLangBarItem *pItem) = 0;
    virtual HRESULT STDMETHODCALLTYPE ShowFloating(DWORD dwFlags) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetShowFloatingStatus(DWORD *pdwFlags) = 0;
};

#endif // __CTFUTB_H__