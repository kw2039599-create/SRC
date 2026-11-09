#include "stdafx.h"
#ifndef DISABLE_JTX_DYNAMIC_LIGHT
#include "../EterLib/StdAfx.h"
#include "../EterLib/StateManager.h"
#include "../EterPack/EterPackManager.h"
#include "PostProcessingEffect.h"
#include <xmllite.h>
#include <shlwapi.h>
#include <comdef.h>
#include "PythonSystem.h"

#pragma comment(lib, "xmllite.lib")
#pragma comment(lib, "shlwapi.lib")

CPostProcessingEffect::CPostProcessingEffect() : m_pPixelShader(nullptr), m_pConstants(nullptr), m_pVertexShader(nullptr) {}

CPostProcessingEffect::CPostProcessingEffect(const CPostProcessingEffect& c_kOld)
    : m_pPixelShader(nullptr), m_pConstants(nullptr), m_pVertexShader(nullptr), m_strName(c_kOld.m_strName), m_shaderSource(c_kOld.m_shaderSource), m_entryPoint(c_kOld.m_entryPoint) {
    SetShader(c_kOld.m_pPixelShader, c_kOld.m_pConstants);
}

CPostProcessingEffect::CPostProcessingEffect(CPostProcessingEffect&& r_kOld)
    : m_pConstants(r_kOld.m_pConstants), m_pPixelShader(r_kOld.m_pPixelShader), m_pVertexShader(r_kOld.m_pVertexShader), m_strName(std::move(r_kOld.m_strName)), m_shaderSource(std::move(r_kOld.m_shaderSource)), m_entryPoint(std::move(r_kOld.m_entryPoint)) {
    r_kOld.m_pPixelShader = nullptr;
    r_kOld.m_pConstants = nullptr;
    r_kOld.m_pVertexShader = nullptr;
}

CPostProcessingEffect::~CPostProcessingEffect() {
    SetShader(nullptr, nullptr);
}

CPostProcessingEffect& CPostProcessingEffect::operator=(const CPostProcessingEffect& c_kOld) {
    if (&c_kOld == this) {
        return *this;
    }
    m_strName = c_kOld.m_strName;
    m_shaderSource = c_kOld.m_shaderSource;
    m_entryPoint = c_kOld.m_entryPoint;
    SetShader(c_kOld.m_pPixelShader, c_kOld.m_pConstants);
    return *this;
}

CPostProcessingEffect& CPostProcessingEffect::operator=(CPostProcessingEffect&& r_kOld) {
    if (&r_kOld == this) {
        return *this;
    }
    if (m_pPixelShader) {
        m_pPixelShader->Release();
    }
    if (m_pConstants) {
        m_pConstants->Release();
    }
    if (m_pVertexShader) {
        m_pVertexShader->Release();
    }
    m_strName = std::move(r_kOld.m_strName);
    m_shaderSource = std::move(r_kOld.m_shaderSource);
    m_entryPoint = std::move(r_kOld.m_entryPoint);
    m_pPixelShader = r_kOld.m_pPixelShader;
    m_pConstants = r_kOld.m_pConstants;
    m_pVertexShader = r_kOld.m_pVertexShader;
    r_kOld.m_pPixelShader = nullptr;
    r_kOld.m_pConstants = nullptr;
    r_kOld.m_pVertexShader = nullptr;
    return *this;
}

bool CPostProcessingEffect::operator==(const char* c_pszName) const {
    return !_stricmp(m_strName.c_str(), c_pszName);
}

bool CPostProcessingEffect::operator==(const CPostProcessingEffect& c_kOld) const {
    return !_stricmp(m_strName.c_str(), c_kOld.GetName());
}

void CPostProcessingEffect::SetName(const char* c_pszName) {
    m_strName = c_pszName;
}

const char* CPostProcessingEffect::GetName() const {
    return m_strName.c_str();
}

std::string ConvertWCharToString(const wchar_t* wchar) {
    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wchar, -1, nullptr, 0, nullptr, nullptr);
    std::string result(bufferSize, 0);
    WideCharToMultiByte(CP_UTF8, 0, wchar, -1, &result[0], bufferSize, nullptr, nullptr);
    return result;
}

bool CPostProcessingEffect::CreateShader(CMappedFile& rkDescriptor) {
    char* pszBuffer = new char[rkDescriptor.Size() + 1];
    rkDescriptor.Read(pszBuffer, rkDescriptor.Size());
    pszBuffer[rkDescriptor.Size()] = 0;

    HRESULT hr;
    IStream* pStream = SHCreateMemStream((const BYTE*)pszBuffer, rkDescriptor.Size());
    if (!pStream) {
        delete[] pszBuffer;
        return false;
    }

    IXmlReader* pReader = nullptr;
    hr = CreateXmlReader(IID_IXmlReader, (void**)&pReader, nullptr);
    if (FAILED(hr)) {
        pStream->Release();
        delete[] pszBuffer;
        return false;
    }

    hr = pReader->SetInput(pStream);
    if (FAILED(hr)) {
        pReader->Release();
        pStream->Release();
        delete[] pszBuffer;
        return false;
    }

    XmlNodeType nodeType;
    const wchar_t* pwszLocalName;

    while (S_OK == pReader->Read(&nodeType)) {
        if (nodeType == XmlNodeType_Element) {
            pReader->GetLocalName(&pwszLocalName, nullptr);
            if (wcscmp(pwszLocalName, L"Effect") == 0) {
                const wchar_t* pwszValue;
                if (S_OK == pReader->MoveToAttributeByName(L"Name", nullptr)) {
                    pReader->GetValue(&pwszValue, nullptr);
                    m_strName = ConvertWCharToString(pwszValue);
                }
                if (S_OK == pReader->MoveToAttributeByName(L"Source", nullptr)) {
                    pReader->GetValue(&pwszValue, nullptr);
                    m_shaderSource = ConvertWCharToString(pwszValue);
                }
                if (S_OK == pReader->MoveToAttributeByName(L"Entry", nullptr)) {
                    pReader->GetValue(&pwszValue, nullptr);
                    m_entryPoint = ConvertWCharToString(pwszValue);
                }
            }
        }
    }

    pReader->Release();
    pStream->Release();
    delete[] pszBuffer;

    CMappedFile kFile;
    LPCVOID pData;
    if (!CEterPackManager::Instance().Get(kFile, m_shaderSource.c_str(), &pData)) {
        TraceError("CPostProcessingEffect unable to load effect %s", m_shaderSource.c_str());
        return false;
    }

    std::string shaderCode;
    shaderCode.assign(static_cast<const char*>(pData), kFile.Size());

    LPD3DXBUFFER pCode;
    LPD3DXBUFFER pError = nullptr;
    if (FAILED(D3DXCompileShader(shaderCode.c_str(), shaderCode.length(), nullptr, nullptr, m_entryPoint.c_str(), "ps_3_0", 0, &pCode, &pError, &m_pConstants))) {
        char* pszError = new char[pError->GetBufferSize() + 1];
        memcpy(pszError, pError->GetBufferPointer(), pError->GetBufferSize());
        pszError[pError->GetBufferSize()] = 0;
        pError->Release();
        TraceError("CPostProcessingEffect unable to compile effect %s: %s", m_shaderSource.c_str(), pszError);
        delete[] pszError;
        return false;
    }
    if (pError) {
        pError->Release();
    }
    if (FAILED(STATEMANAGER.GetDevice()->CreatePixelShader(reinterpret_cast<DWORD*>(pCode->GetBufferPointer()), &m_pPixelShader))) {
        TraceError("CPostProcessingEffect unable to create effect instance %s", m_shaderSource.c_str());
        pCode->Release();
        m_pConstants->Release();
        return false;
    }
    pCode->Release();
    return true;
}

void CPostProcessingEffect::SetShader(IDirect3DPixelShader9* lpShader, LPD3DXCONSTANTTABLE lpConstants) {
    if (m_pPixelShader) {
        m_pPixelShader->Release();
    }
    if (m_pConstants) {
        m_pConstants->Release();
    }
    m_pPixelShader = lpShader;
    m_pConstants = lpConstants;
    if (m_pPixelShader) {
        m_pPixelShader->AddRef();
    }
    if (m_pConstants) {
        m_pConstants->AddRef();
    }
}

IDirect3DPixelShader9* CPostProcessingEffect::GetShader() {
    return m_pPixelShader;
}

LPD3DXCONSTANTTABLE CPostProcessingEffect::GetConstants() {
    return m_pConstants;
}


bool CPostProcessingEffect::Apply() {
    IDirect3DDevice9* device = STATEMANAGER.GetDevice();
    if (!device) {
        TraceError("Direct3D device is NULL.");
        return false;
    }

    device->SetPixelShader(m_pPixelShader);

    return true;
}
#endif
