#pragma once

#ifndef __ANDROID__
  #error "This comdef.h is only for Android!"
#endif

#include <windows.h>
#include <xmllite.h>
#include <string>

class _com_error {
public:
    _com_error(HRESULT hr = S_OK) : m_hr(hr) {}
    HRESULT Error() const noexcept { return m_hr; }
    const char* ErrorMessage() const noexcept { return "COM stub error (Android)"; }
private:
    HRESULT m_hr;
};

class _bstr_t {
public:
    _bstr_t() {}
    _bstr_t(const char* s) : m_str(s ? s : "") {
        if (s) {
            while (*s) m_wstr.push_back((WCHAR)(unsigned char)(*s++));
        }
    }
    _bstr_t(const WCHAR* ws) : m_wstr(ws ? ws : L"") {
        if (ws) {
            while (*ws) m_str.push_back((char)(*ws++ & 0xFF));
        }
    }
    operator const char*() const noexcept { return m_str.c_str(); }
    operator const WCHAR*() const noexcept { return m_wstr.c_str(); }
    const char* c_str() const noexcept { return m_str.c_str(); }
    unsigned int length() const noexcept { return (unsigned int)m_str.length(); }
private:
    std::string m_str;
    std::wstring m_wstr;
};

template <typename Interface>
class _com_ptr_t {
public:
    _com_ptr_t() : m_p(NULL) {}
    _com_ptr_t(Interface* p) : m_p(p) { if (m_p) m_p->AddRef(); }
    _com_ptr_t(const _com_ptr_t& other) : m_p(other.m_p) { if (m_p) m_p->AddRef(); }
    ~_com_ptr_t() { Release(); }

    _com_ptr_t& operator=(Interface* p) {
        if (m_p != p) {
            if (p) p->AddRef();
            if (m_p) m_p->Release();
            m_p = p;
        }
        return *this;
    }

    _com_ptr_t& operator=(const _com_ptr_t& other) {
        return (*this = other.m_p);
    }

    void Release() {
        if (m_p) {
            m_p->Release();
            m_p = NULL;
        }
    }

    Interface* GetInterfacePtr() const { return m_p; }
    Interface* operator->() const { return m_p; }
    Interface& operator*() const { return *m_p; }
    Interface** operator&() { Release(); return &m_p; }
    operator Interface*() const { return m_p; }
    explicit operator bool() const { return m_p != NULL; }

private:
    Interface* m_p;
};

#ifndef _COM_SMARTPTR_TYPEDEF
#define _COM_SMARTPTR_TYPEDEF(Interface, IID) typedef _com_ptr_t<Interface> Interface##Ptr
#endif

_COM_SMARTPTR_TYPEDEF(IUnknown, __uuidof(IUnknown));
_COM_SMARTPTR_TYPEDEF(ISequentialStream, __uuidof(ISequentialStream));
_COM_SMARTPTR_TYPEDEF(IStream, __uuidof(IStream));
_COM_SMARTPTR_TYPEDEF(IXmlReader, __uuidof(IXmlReader));
_COM_SMARTPTR_TYPEDEF(IXmlWriter, __uuidof(IXmlWriter));