#pragma once

#ifndef __ANDROID__
  #error "This shlwapi.h is only for Android!"
#endif

#ifndef __SHLWAPI_H__
#define __SHLWAPI_H__

#include <windows.h>
#include <unknwn.h>

// COM stream interfaces required by SHLWAPI
struct ISequentialStream : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE Read(void* pv, ULONG cb, ULONG* pcbRead) = 0;
    virtual HRESULT STDMETHODCALLTYPE Write(const void* pv, ULONG cb, ULONG* pcbWritten) = 0;
};

struct IStream : public ISequentialStream {
    virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize) = 0;
    virtual HRESULT STDMETHODCALLTYPE CopyTo(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead, ULARGE_INTEGER* pcbWritten) = 0;
    virtual HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags) = 0;
    virtual HRESULT STDMETHODCALLTYPE Revert(void) = 0;
    virtual HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockFlags) = 0;
    virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockFlags) = 0;
    virtual HRESULT STDMETHODCALLTYPE Stat(void* pstatstg, DWORD grfStatFlag) = 0;
    virtual HRESULT STDMETHODCALLTYPE Clone(IStream** ppstm) = 0;
};

static inline HRESULT SHCreateStreamOnFileA(LPCSTR pszFile, DWORD grfMode, IStream** ppstm) {
    (void)pszFile; 
    (void)grfMode;
    if (ppstm) *ppstm = NULL;
    return E_FAIL;
}

static inline IStream* SHCreateMemStream(const BYTE* pInit, UINT cbInit) {
    // Basic memory stream implementation wrapper for Android stub
    class CCtxMemoryStream : public IStream {
    private:
        LONG m_lRef;
        BYTE* m_pBuffer;
        UINT m_cbSize;
        UINT m_cbSeek;
    public:
        CCtxMemoryStream(const BYTE* pInit, UINT cbInit) : m_lRef(1), m_cbSize(cbInit), m_cbSeek(0) {
            m_pBuffer = (BYTE*)malloc(cbInit ? cbInit : 1);
            if (pInit && cbInit) {
                memcpy(m_pBuffer, pInit, cbInit);
            }
        }
        virtual ~CCtxMemoryStream() {
            if (m_pBuffer) free(m_pBuffer);
        }
        virtual HRESULT STDMETHODCALLTYPE QueryInterface(const GUID& riid, void** ppvObject) override {
            if (!ppvObject) return E_POINTER;
            *ppvObject = this;
            AddRef();
            return S_OK;
        }
        virtual ULONG STDMETHODCALLTYPE AddRef(void) override { return ++m_lRef; }
        virtual ULONG STDMETHODCALLTYPE Release(void) override {
            if (--m_lRef == 0) { delete this; return 0; }
            return m_lRef;
        }
        virtual HRESULT STDMETHODCALLTYPE Read(void* pv, ULONG cb, ULONG* pcbRead) override {
            if (!pv) return E_POINTER;
            UINT bytesLeft = (m_cbSeek < m_cbSize) ? (m_cbSize - m_cbSeek) : 0;
            UINT bytesToRead = (cb < bytesLeft) ? cb : bytesLeft;
            if (bytesToRead > 0) {
                memcpy(pv, m_pBuffer + m_cbSeek, bytesToRead);
                m_cbSeek += bytesToRead;
            }
            if (pcbRead) *pcbRead = bytesToRead;
            return S_OK;
        }
        virtual HRESULT STDMETHODCALLTYPE Write(const void* pv, ULONG cb, ULONG* pcbWritten) override {
            (void)pv; (void)cb; (void)pcbWritten; return E_NOTIMPL;
        }
        virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition) override {
            int target = m_cbSeek;
            if (dwOrigin == 0) target = (int)dlibMove.QuadPart;
            else if (dwOrigin == 1) target += (int)dlibMove.QuadPart;
            else if (dwOrigin == 2) target = m_cbSize + (int)dlibMove.QuadPart;
            if (target < 0) target = 0;
            if ((UINT)target > m_cbSize) target = m_cbSize;
            m_cbSeek = (UINT)target;
            if (plibNewPosition) plibNewPosition->QuadPart = m_cbSeek;
            return S_OK;
        }
        virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize) override { (void)libNewSize; return E_NOTIMPL; }
        virtual HRESULT STDMETHODCALLTYPE CopyTo(IStream*, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*) override { return E_NOTIMPL; }
        virtual HRESULT STDMETHODCALLTYPE Commit(DWORD) override { return S_OK; }
        virtual HRESULT STDMETHODCALLTYPE Revert(void) override { return E_NOTIMPL; }
        virtual HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override { return E_NOTIMPL; }
        virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override { return E_NOTIMPL; }
        virtual HRESULT STDMETHODCALLTYPE Stat(void*, DWORD) override { return E_NOTIMPL; }
        virtual HRESULT STDMETHODCALLTYPE Clone(IStream**) override { return E_NOTIMPL; }
    };
    return new CCtxMemoryStream(pInit, cbInit);
}

#endif // __SHLWAPI_H__