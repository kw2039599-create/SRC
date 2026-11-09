#pragma once
#include "SpeedTreeForest.h"
#include "SpeedTreeMaterial.h"

class CSpeedTreeForestDirectX8 : public CSpeedTreeForest, public CGraphicBase
{
public:
	CSpeedTreeForestDirectX8();
	~CSpeedTreeForestDirectX8();

	void                        UploadWindMatrix(unsigned int uiLocation, const float* pMatrix) const;
	void						UpdateCompundMatrix(const D3DXVECTOR3& c_rEyeVec, const D3DXMATRIX& c_rmatView, const D3DXMATRIX& c_rmatProj);
	void						UpdateCompundMatrix2(const D3DXVECTOR3& c_rEyeVec, const D3DXMATRIX& c_rmatView, const D3DXMATRIX& c_rmatProj);

	void                        Render(unsigned long ulRenderBitVector = Forest_RenderAll);
	void                        RenderReflection(unsigned long ulRenderBitVector = Forest_RenderAll);
	void						Render2(unsigned long ulRenderBitVector);
	void						Render3(unsigned long ulRenderBitVector);
	bool						SetRenderingDevice(LPDIRECT3DDEVICE9 pDevice);

private:
	bool                        InitVertexShaders();

private:
	LPDIRECT3DDEVICE9 m_pDx;

	LPDIRECT3DVERTEXSHADER9     m_dwBranchVertexShader;
	LPDIRECT3DVERTEXSHADER9     m_dwLeafVertexShader;
};
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
