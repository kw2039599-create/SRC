#pragma once

#include "GrpBase.h"

class CVertexShader : public CGraphicBase
{
public:
	CVertexShader();
	virtual ~CVertexShader();

	void Destroy();
	bool CreateFromDiskFile(const char* c_szFileName, const DWORD* c_pdwVertexDecl);

	void Set();

protected:
	void Initialize();

protected:
#ifdef DIRECTX9
	LPDIRECT3DVERTEXSHADER9 m_handle;
#else
	DWORD m_handle;
#endif
};
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
