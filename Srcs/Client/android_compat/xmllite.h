#pragma once

#ifndef __ANDROID__
  #error "This xmllite.h is only for Android!"
#endif

#ifndef __XMLLITE_H__
#define __XMLLITE_H__

#include <windows.h>

typedef enum EarliestXmlReaderProperty {
    XmlReaderProperty_MultiLanguage = 0,
    XmlReaderProperty_ConformanceLevel = 1
} XmlReaderProperty;

typedef enum EarliestXmlNodeType {
    XmlNodeType_None = 0,
    XmlNodeType_Element = 1,
    XmlNodeType_Attribute = 2,
    XmlNodeType_Text = 3,
    XmlNodeType_EndElement = 15
} XmlNodeType;

static const GUID IID_IXmlReader = { 0x7279fc81, 0x0b8d, 0x4f16, { 0xb0, 0xba, 0xda, 0x2b, 0x2f, 0x9f, 0x24, 0xfe } };
static const GUID IID_IXmlWriter = { 0x7279fc88, 0x0b8d, 0x4f16, { 0xb0, 0xba, 0xda, 0x2b, 0x2f, 0x9f, 0x24, 0xfe } };

struct DECLSPEC_UUID("7279FC81-0B8D-4F16-B0BA-DA2B2F9F24FE") IXmlReader : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE SetInput(void* pInput) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetAttribute(LPCWSTR *ppWsName, LPCWSTR *ppWsValue) = 0;
    virtual HRESULT STDMETHODCALLTYPE MoveToFirstAttribute(void) = 0;
    virtual HRESULT STDMETHODCALLTYPE MoveToNextAttribute(void) = 0;
    virtual HRESULT STDMETHODCALLTYPE MoveToAttributeByName(LPCWSTR pwszLocalName, LPCWSTR pwszNamespaceUri) = 0;
    virtual HRESULT STDMETHODCALLTYPE Read(XmlNodeType *pNodeType) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetLocalName(LPCWSTR *ppWsName, UINT *pCchName) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetValue(LPCWSTR *ppWsValue, UINT *pCchValue) = 0;
};

struct DECLSPEC_UUID("7279FC88-0B8D-4F16-B0BA-DA2B2F9F24FE") IXmlWriter : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE SetOutput(void* pOutput) = 0;
    virtual HRESULT STDMETHODCALLTYPE WriteStartDocument(int standalone) = 0;
    virtual HRESULT STDMETHODCALLTYPE WriteEndDocument(void) = 0;
    virtual HRESULT STDMETHODCALLTYPE WriteStartElement(LPCWSTR pwszPrefix, LPCWSTR pwszLocalName, LPCWSTR pwszNamespaceUri) = 0;
    virtual HRESULT STDMETHODCALLTYPE WriteEndElement(void) = 0;
    virtual HRESULT STDMETHODCALLTYPE WriteString(LPCWSTR pwszText) = 0;
    virtual HRESULT STDMETHODCALLTYPE Flush(void) = 0;
};

class CCtxXmlReader : public IXmlReader {
private:
    LONG m_lRef;
public:
    CCtxXmlReader() : m_lRef(1) {}
    virtual ~CCtxXmlReader() {}

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(const GUID& riid, void** ppvObject) override {
        if (!ppvObject) return E_POINTER;
        *ppvObject = this;
        AddRef();
        return S_OK;
    }
    virtual ULONG STDMETHODCALLTYPE AddRef(void) override {
        return ++m_lRef;
    }
    virtual ULONG STDMETHODCALLTYPE Release(void) override {
        if (--m_lRef == 0) {
            delete this;
            return 0;
        }
        return m_lRef;
    }

    virtual HRESULT STDMETHODCALLTYPE SetInput(void* pInput) override { (void)pInput; return S_OK; }
    virtual HRESULT STDMETHODCALLTYPE GetAttribute(LPCWSTR *ppWsName, LPCWSTR *ppWsValue) override { 
        if (ppWsName) *ppWsName = L""; 
        if (ppWsValue) *ppWsValue = L""; 
        return E_FAIL; 
    }
    virtual HRESULT STDMETHODCALLTYPE MoveToFirstAttribute(void) override { return E_FAIL; }
    virtual HRESULT STDMETHODCALLTYPE MoveToNextAttribute(void) override { return E_FAIL; }
    virtual HRESULT STDMETHODCALLTYPE MoveToAttributeByName(LPCWSTR pwszLocalName, LPCWSTR pwszNamespaceUri) override {
    (void)pwszLocalName; (void)pwszNamespaceUri;
    return S_FALSE; // Return S_FALSE or S_OK if attribute not strictly mandatory for stub fallback
}
    virtual HRESULT STDMETHODCALLTYPE Read(XmlNodeType *pNodeType) override { 
        if (pNodeType) *pNodeType = XmlNodeType_None; 
        return S_FALSE; 
    }
    virtual HRESULT STDMETHODCALLTYPE GetLocalName(LPCWSTR *ppWsName, UINT *pCchName) override { 
        if (ppWsName) *ppWsName = L""; 
        if (pCchName) *pCchName = 0; 
        return S_OK; 
    }
    virtual HRESULT STDMETHODCALLTYPE GetValue(LPCWSTR *ppWsValue, UINT *pCchValue) override { 
        if (ppWsValue) *ppWsValue = L""; 
        if (pCchValue) *pCchValue = 0; 
        return S_OK; 
    }
};

class CCtxXmlWriter : public IXmlWriter {
private:
    LONG m_lRef;
public:
    CCtxXmlWriter() : m_lRef(1) {}
    virtual ~CCtxXmlWriter() {}

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(const GUID& riid, void** ppvObject) override {
        if (!ppvObject) return E_POINTER;
        *ppvObject = this;
        AddRef();
        return S_OK;
    }
    virtual ULONG STDMETHODCALLTYPE AddRef(void) override {
        return ++m_lRef;
    }
    virtual ULONG STDMETHODCALLTYPE Release(void) override {
        if (--m_lRef == 0) {
            delete this;
            return 0;
        }
        return m_lRef;
    }

    virtual HRESULT STDMETHODCALLTYPE SetOutput(void* pOutput) override { (void)pOutput; return S_OK; }
    virtual HRESULT STDMETHODCALLTYPE WriteStartDocument(int standalone) override { (void)standalone; return S_OK; }
    virtual HRESULT STDMETHODCALLTYPE WriteEndDocument(void) override { return S_OK; }
    virtual HRESULT STDMETHODCALLTYPE WriteStartElement(LPCWSTR pwszPrefix, LPCWSTR pwszLocalName, LPCWSTR pwszNamespaceUri) override {
        (void)pwszPrefix; (void)pwszLocalName; (void)pwszNamespaceUri; return S_OK;
    }
    virtual HRESULT STDMETHODCALLTYPE WriteEndElement(void) override { return S_OK; }
    virtual HRESULT STDMETHODCALLTYPE WriteString(LPCWSTR pwszText) override { (void)pwszText; return S_OK; }
    virtual HRESULT STDMETHODCALLTYPE Flush(void) override { return S_OK; }
};

inline HRESULT CreateXmlReader(const GUID& riid, void** ppvObject, void* pMalloc) {
    (void)riid;
    (void)pMalloc;
    if (!ppvObject) return E_POINTER;
    *ppvObject = new CCtxXmlReader();
    return S_OK;
}

inline HRESULT CreateXmlWriter(const GUID& riid, void** ppvObject, void* pMalloc) {
    (void)riid;
    (void)pMalloc;
    if (!ppvObject) return E_POINTER;
    *ppvObject = new CCtxXmlWriter();
    return S_OK;
}

#endif // __XMLLITE_H__