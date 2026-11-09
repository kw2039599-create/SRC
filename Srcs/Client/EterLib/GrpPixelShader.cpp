#include "StdAfx.h"
#include "GrpPixelShader.h"
#include "GrpD3DXBuffer.h"
#include "StateManager.h"

CPixelShader::CPixelShader()
{
	Initialize();
}

CPixelShader::~CPixelShader()
{
	Destroy();
}

void CPixelShader::Initialize()
{
	m_handle = 0;
}

void CPixelShader::Destroy()
{
	if (m_handle)
	{
#ifdef DIRECTX9
#else
		if (ms_lpd3dDevice)
			ms_lpd3dDevice->DeletePixelShader(m_handle);
#endif
		m_handle = 0;
	}
}

#ifdef DIRECTX9
bool CPixelShader::CreateFromDiskFile(const char* c_szFileName)
{
	Destroy();

	LPD3DXBUFFER lpd3dxShaderBuffer;
	LPD3DXBUFFER lpd3dxErrorBuffer;

	if (FAILED(D3DXAssembleShaderFromFileA(c_szFileName, 0, NULL, 0, &lpd3dxShaderBuffer, &lpd3dxErrorBuffer)))
		return false;

	if (FAILED(ms_lpd3dDevice->CreatePixelShader((const DWORD*)lpd3dxShaderBuffer->GetBufferPointer(), &m_handle)))
		return false;

	return true;
}
#else
bool CPixelShader::CreateFromDiskFile(const char* c_szFileName)
{
	Destroy();

	LPD3DXBUFFER lpd3dxShaderBuffer;
	LPD3DXBUFFER lpd3dxErrorBuffer;

	if (FAILED(
		D3DXAssembleShaderFromFile(c_szFileName, 0, NULL, &lpd3dxShaderBuffer, &lpd3dxErrorBuffer)
	))
		return false;

	CDirect3DXBuffer shaderBuffer(lpd3dxShaderBuffer);
	CDirect3DXBuffer errorBuffer(lpd3dxErrorBuffer);

	if (FAILED(ms_lpd3dDevice->CreatePixelShader((DWORD*)shaderBuffer.GetPointer(), &m_handle)))
		return false;

	return true;
}
#endif

void CPixelShader::Set()
{
#ifdef DIRECTX9
	STATEMANAGER.SetPixelShader(m_handle);

#else
	STATEMANAGER.SetPixelShader(m_handle);

#endif
}
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
