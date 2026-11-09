#include "StdAfx.h"
#include "GrpVertexShader.h"
#include "GrpD3DXBuffer.h"
#include "StateManager.h"

CVertexShader::CVertexShader()
{
	Initialize();
}

CVertexShader::~CVertexShader()
{
	Destroy();
}

void CVertexShader::Initialize()
{
	m_handle = 0;
}

void CVertexShader::Destroy()
{
	if (m_handle)
	{
#ifdef DIRECTX9
		m_handle->Release();
#else
		if (ms_lpd3dDevice)
			ms_lpd3dDevice->DeleteVertexShader(m_handle);
#endif

		m_handle = 0;
	}
}

#ifdef DIRECTX9
bool CVertexShader::CreateFromDiskFile(const char* c_szFileName, const DWORD* c_pdwVertexDecl)
{
	Destroy();

	LPD3DXBUFFER lpd3dxShaderBuffer;
	LPD3DXBUFFER lpd3dxErrorBuffer;
	if (FAILED(D3DXAssembleShaderFromFile(c_szFileName, 0, NULL, 0, &lpd3dxShaderBuffer, &lpd3dxErrorBuffer)))
		return false;

	if (FAILED(ms_lpd3dDevice->CreateVertexShader((const DWORD*)lpd3dxShaderBuffer->GetBufferPointer(), &m_handle)))
		return false;

	return true;
}
#else
bool CVertexShader::CreateFromDiskFile(const char* c_szFileName, const DWORD* c_pdwVertexDecl)
{
	Destroy();

	LPD3DXBUFFER lpd3dxShaderBuffer;
	LPD3DXBUFFER lpd3dxErrorBuffer;

	if (FAILED(
		D3DXAssembleShaderFromFile(c_szFileName, 0, NULL, &lpd3dxShaderBuffer, &lpd3dxErrorBuffer)
	)) return false;

	CDirect3DXBuffer shaderBuffer(lpd3dxShaderBuffer);
	CDirect3DXBuffer errorBuffer(lpd3dxErrorBuffer);

	if (FAILED(
		ms_lpd3dDevice->CreateVertexShader(c_pdwVertexDecl, (DWORD*)shaderBuffer.GetPointer(), &m_handle, 0)
	))
		return false;

	return true;
}
#endif

void CVertexShader::Set()
{
	STATEMANAGER.SetVertexShader(m_handle);
}
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
