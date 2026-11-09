#include "StdAfx.h"
#include "StateManager.h"

//#define StateManager_Assert(a) if (!(a)) puts("assert"#a)
#define StateManager_Assert(a) assert(a)

struct SLightData
{
	enum
	{
		LIGHT_NUM = 8,
	};
	D3DLIGHT8 m_akD3DLight[LIGHT_NUM];
} m_kLightData;

void CStateManager::SetLight(DWORD index, CONST D3DLIGHT8* pLight)
{
	assert(index<SLightData::LIGHT_NUM);
	m_kLightData.m_akD3DLight[index]=*pLight;

	m_lpD3DDev->SetLight(index, pLight);
}

void CStateManager::GetLight(DWORD index, D3DLIGHT8* pLight)
{
	assert(index<8);
	*pLight=m_kLightData.m_akD3DLight[index];
}

bool CStateManager::BeginScene()
{
	m_bScene=true;

	D3DXMATRIX m4Proj;
	D3DXMATRIX m4View;
	D3DXMATRIX m4World;
	GetTransform(D3DTS_WORLD, &m4World);
	GetTransform(D3DTS_PROJECTION, &m4Proj);
	GetTransform(D3DTS_VIEW, &m4View);
	SetTransform(D3DTS_WORLD, &m4World);
	SetTransform(D3DTS_PROJECTION, &m4Proj);
	SetTransform(D3DTS_VIEW, &m4View);

	if (FAILED(m_lpD3DDev->BeginScene()))
		return false;
	return true;
}

void CStateManager::EndScene()
{
	m_lpD3DDev->EndScene();
	m_bScene=false;
}

CStateManager::CStateManager(LPDIRECT3DDEVICE8 lpDevice) : m_lpD3DDev(NULL)
{
	m_bScene = false;
	m_dwBestMinFilter = D3DTEXF_LINEAR;
	m_dwBestMagFilter = D3DTEXF_LINEAR;
	SetDevice(lpDevice);
}

CStateManager::~CStateManager()
{
	if (m_lpD3DDev)
	{
		m_lpD3DDev->Release();
		m_lpD3DDev = NULL;
	}
}

void CStateManager::SetDevice(LPDIRECT3DDEVICE8 lpDevice)
{
	StateManager_Assert(lpDevice);
	lpDevice->AddRef();

	if (m_lpD3DDev)
	{
		m_lpD3DDev->Release();
		m_lpD3DDev = NULL;
	}

	m_lpD3DDev = lpDevice;

	SetDefaultState();
}

void CStateManager::SetBestFiltering(DWORD dwStage)
{
#ifdef DIRECTX9
	SetSamplerState(dwStage, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
	SetSamplerState(dwStage, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
	SetSamplerState(dwStage, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);
#else
	SetTextureStageState(dwStage, D3DTSS_MINFILTER, m_dwBestMinFilter);
	SetTextureStageState(dwStage, D3DTSS_MAGFILTER, m_dwBestMagFilter);
	SetTextureStageState(dwStage, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
#endif
}

void CStateManager::Restore()
{
	int i, j;

	m_bForce = true;

	for (i = 0; i < STATEMANAGER_MAX_RENDERSTATES; ++i)
		SetRenderState(D3DRENDERSTATETYPE(i), m_CurrentState.m_RenderStates[i]);

	for (i = 0; i < STATEMANAGER_MAX_STAGES; ++i)
		for (j = 0; j < STATEMANAGER_MAX_TEXTURESTATES; ++j)
			SetTextureStageState(i, D3DTEXTURESTAGESTATETYPE(j), m_CurrentState.m_TextureStates[i][j]);

	for (i = 0; i < STATEMANAGER_MAX_STAGES; ++i)
		SetTexture(i, m_CurrentState.m_Textures[i]);

	m_bForce = false;
}

void CStateManager::SetDefaultState()
{
	// @fixme039 BEGIN (moved from SetDevice)
	D3DCAPS8 d3dCaps;
	m_lpD3DDev->GetDeviceCaps(&d3dCaps);

	if (d3dCaps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC)
		m_dwBestMagFilter = D3DTEXF_ANISOTROPIC;
	else
		m_dwBestMagFilter = D3DTEXF_LINEAR;

	if (d3dCaps.TextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC)
		m_dwBestMinFilter = D3DTEXF_ANISOTROPIC;
	else
		m_dwBestMinFilter = D3DTEXF_LINEAR;

	DWORD dwMax = d3dCaps.MaxAnisotropy;
	dwMax = dwMax < 4 ? dwMax : 4;

	for (int i = 0; i < 8; ++i)
#ifdef DIRECTX9
		m_lpD3DDev->SetSamplerState(i, D3DSAMP_MAXANISOTROPY, 16);
#else
		m_lpD3DDev->SetTextureStageState(i, D3DTSS_MAXANISOTROPY, dwMax);
#endif

	m_CurrentState.ResetState();
	m_CopyState.ResetState();
	m_ChipState.ResetState();

	m_bScene = false;
	m_bForce = true;

	D3DXMATRIX Identity;
	D3DXMatrixIdentity(&Identity);

	SetTransform(D3DTS_WORLD, &Identity);
	SetTransform(D3DTS_VIEW, &Identity);
	SetTransform(D3DTS_PROJECTION, &Identity);

	D3DMATERIAL8 DefaultMat;
	ZeroMemory(&DefaultMat, sizeof(D3DMATERIAL8));

	DefaultMat.Diffuse.r = 1.0f;
	DefaultMat.Diffuse.g = 1.0f;
	DefaultMat.Diffuse.b = 1.0f;
	DefaultMat.Diffuse.a = 1.0f;
	DefaultMat.Ambient.r = 1.0f;
	DefaultMat.Ambient.g = 1.0f;
	DefaultMat.Ambient.b = 1.0f;
	DefaultMat.Ambient.a = 1.0f;
	DefaultMat.Emissive.r = 0.0f;
	DefaultMat.Emissive.g = 0.0f;
	DefaultMat.Emissive.b = 0.0f;
	DefaultMat.Emissive.a = 0.0f;
	DefaultMat.Specular.r = 0.0f;
	DefaultMat.Specular.g = 0.0f;
	DefaultMat.Specular.b = 0.0f;
	DefaultMat.Specular.a = 0.0f;
	DefaultMat.Power = 0.0f;

	SetMaterial(&DefaultMat);

	SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
	SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);
	SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
	SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);

#ifndef DIRECTX9
	SetRenderState(D3DRS_LINEPATTERN, 0xFFFFFFFF);
	SetRenderState(D3DRS_ZVISIBLE, FALSE);
	SetRenderState(D3DRS_EDGEANTIALIAS, FALSE);
	SetRenderState(D3DRS_ZBIAS, 0);
	SetRenderState(D3DRS_SOFTWAREVERTEXPROCESSING, FALSE);
#endif
	SetRenderState(D3DRS_LASTPIXEL, FALSE);
	SetRenderState(D3DRS_ALPHAREF, 1);
	SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
	SetRenderState(D3DRS_FOGSTART, 0);
	SetRenderState(D3DRS_FOGEND, 0);
	SetRenderState(D3DRS_FOGDENSITY, 0);
	SetRenderState(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);
	SetRenderState(D3DRS_AMBIENT, 0x00000000);
	SetRenderState(D3DRS_LOCALVIEWER, FALSE);
	SetRenderState(D3DRS_NORMALIZENORMALS, FALSE);
	SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
	SetRenderState(D3DRS_CLIPPLANEENABLE, 0);
#ifdef DIRECTX9
	m_lpD3DDev->SetSoftwareVertexProcessing(false);
#endif
	SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);
	SetRenderState(D3DRS_MULTISAMPLEMASK, 0xFFFFFFFF);
	SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);
	SetRenderState(D3DRS_COLORWRITEENABLE, 0xFFFFFFFF);
	SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	SetRenderState(D3DRS_FOGENABLE, FALSE);
	SetRenderState(D3DRS_FOGCOLOR, 0xFF000000);
	SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
	SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
	SetRenderState(D3DRS_RANGEFOGENABLE, FALSE);
	SetRenderState(D3DRS_ZENABLE, TRUE);
	SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	SetRenderState(D3DRS_DITHERENABLE, TRUE);
	SetRenderState(D3DRS_STENCILENABLE, FALSE);
	SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	SetRenderState(D3DRS_CLIPPING, TRUE);
	SetRenderState(D3DRS_LIGHTING, FALSE);
	SetRenderState(D3DRS_SPECULARENABLE, FALSE);
	SetRenderState(D3DRS_COLORVERTEX, FALSE);
	SetRenderState(D3DRS_WRAP0, 0);
	SetRenderState(D3DRS_WRAP1, 0);
	SetRenderState(D3DRS_WRAP2, 0);
	SetRenderState(D3DRS_WRAP3, 0);
	SetRenderState(D3DRS_WRAP4, 0);
	SetRenderState(D3DRS_WRAP5, 0);
	SetRenderState(D3DRS_WRAP6, 0);
	SetRenderState(D3DRS_WRAP7, 0);

	SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
	SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
	SetTextureStageState(2, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	SetTextureStageState(2, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	SetTextureStageState(2, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	SetTextureStageState(2, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	SetTextureStageState(3, D3DTSS_COLOROP, D3DTOP_DISABLE);
	SetTextureStageState(3, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	SetTextureStageState(3, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	SetTextureStageState(3, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	SetTextureStageState(3, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	SetTextureStageState(3, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	SetTextureStageState(4, D3DTSS_COLOROP, D3DTOP_DISABLE);
	SetTextureStageState(4, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	SetTextureStageState(4, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	SetTextureStageState(4, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	SetTextureStageState(4, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	SetTextureStageState(4, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	SetTextureStageState(5, D3DTSS_COLOROP, D3DTOP_DISABLE);
	SetTextureStageState(5, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	SetTextureStageState(5, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	SetTextureStageState(5, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	SetTextureStageState(5, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	SetTextureStageState(5, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	SetTextureStageState(6, D3DTSS_COLOROP, D3DTOP_DISABLE);
	SetTextureStageState(6, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	SetTextureStageState(6, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	SetTextureStageState(6, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	SetTextureStageState(6, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	SetTextureStageState(6, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	SetTextureStageState(7, D3DTSS_COLOROP, D3DTOP_DISABLE);
	SetTextureStageState(7, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	SetTextureStageState(7, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	SetTextureStageState(7, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	SetTextureStageState(7, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	SetTextureStageState(7, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
	SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
	SetTextureStageState(2, D3DTSS_TEXCOORDINDEX, 2);
	SetTextureStageState(3, D3DTSS_TEXCOORDINDEX, 3);
	SetTextureStageState(4, D3DTSS_TEXCOORDINDEX, 4);
	SetTextureStageState(5, D3DTSS_TEXCOORDINDEX, 5);
	SetTextureStageState(6, D3DTSS_TEXCOORDINDEX, 6);
	SetTextureStageState(7, D3DTSS_TEXCOORDINDEX, 7);

#ifdef DIRECTX9
	SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	SetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	SetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	SetSamplerState(2, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	SetSamplerState(3, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	SetSamplerState(3, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	SetSamplerState(3, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	SetSamplerState(4, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	SetSamplerState(4, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	SetSamplerState(4, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	SetSamplerState(5, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	SetSamplerState(5, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	SetSamplerState(5, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	SetSamplerState(6, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	SetSamplerState(6, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	SetSamplerState(6, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	SetSamplerState(7, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	SetSamplerState(7, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	SetSamplerState(7, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	SetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	SetSamplerState(3, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	SetSamplerState(3, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	SetSamplerState(4, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	SetSamplerState(4, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	SetSamplerState(5, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	SetSamplerState(5, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	SetSamplerState(6, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	SetSamplerState(6, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	SetSamplerState(7, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	SetSamplerState(7, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
#else
	SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
	SetTextureStageState(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	SetTextureStageState(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
	SetTextureStageState(2, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	SetTextureStageState(2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	SetTextureStageState(2, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
	SetTextureStageState(3, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	SetTextureStageState(3, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	SetTextureStageState(3, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
	SetTextureStageState(4, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	SetTextureStageState(4, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	SetTextureStageState(4, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
	SetTextureStageState(5, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	SetTextureStageState(5, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	SetTextureStageState(5, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
	SetTextureStageState(6, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	SetTextureStageState(6, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	SetTextureStageState(6, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
	SetTextureStageState(7, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	SetTextureStageState(7, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
	SetTextureStageState(7, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
	SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
	SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
	SetTextureStageState(1, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
	SetTextureStageState(1, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
	SetTextureStageState(2, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
	SetTextureStageState(2, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
	SetTextureStageState(3, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
	SetTextureStageState(3, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
	SetTextureStageState(4, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
	SetTextureStageState(4, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
	SetTextureStageState(5, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
	SetTextureStageState(5, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
	SetTextureStageState(6, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
	SetTextureStageState(6, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
	SetTextureStageState(7, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
	SetTextureStageState(7, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
#endif

	SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
	SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
	SetTextureStageState(2, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
	SetTextureStageState(3, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
	SetTextureStageState(4, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
	SetTextureStageState(5, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
	SetTextureStageState(6, D3DTSS_TEXTURETRANSFORMFLAGS, 0);
	SetTextureStageState(7, D3DTSS_TEXTURETRANSFORMFLAGS, 0);

	SetTexture(0, NULL);
	SetTexture(1, NULL);
	SetTexture(2, NULL);
	SetTexture(3, NULL);
	SetTexture(4, NULL);
	SetTexture(5, NULL);
	SetTexture(6, NULL);
	SetTexture(7, NULL);

	SetPixelShader(0);
#ifdef DIRECTX9
	SetFVF(D3DFVF_XYZ);
#else
	SetVertexShader(D3DFVF_XYZ);
#endif

	D3DXVECTOR4 av4Null[STATEMANAGER_MAX_VCONSTANTS];
	memset(av4Null, 0, sizeof(av4Null));
	SetVertexShaderConstant(0, av4Null, STATEMANAGER_MAX_VCONSTANTS);
	SetPixelShaderConstant(0, av4Null, STATEMANAGER_MAX_PCONSTANTS);

	m_bForce = false;

#ifdef _DEBUG
	int i, j;
	for (i = 0; i < STATEMANAGER_MAX_RENDERSTATES; i++)
		m_bRenderStateSavingFlag[i] = FALSE;

	for (j = 0; j < STATEMANAGER_MAX_TRANSFORMSTATES; j++)
		m_bTransformSavingFlag[j] = FALSE;

	for (j = 0; j < STATEMANAGER_MAX_STAGES; ++j)
		for (i = 0; i < STATEMANAGER_MAX_TEXTURESTATES; ++i)
			m_bTextureStageStateSavingFlag[j][i] = FALSE;
#endif _DEBUG
}

// Material
void CStateManager::SaveMaterial()
{
	m_CopyState.m_D3DMaterial = m_CurrentState.m_D3DMaterial;
}

void CStateManager::SaveMaterial(const D3DMATERIAL8 * pMaterial)
{
	// Check that we have set this up before, if not, the default is this.
	m_CopyState.m_D3DMaterial = m_CurrentState.m_D3DMaterial;
	SetMaterial(pMaterial);
}

void CStateManager::RestoreMaterial()
{
	SetMaterial(&m_CopyState.m_D3DMaterial);
}

void CStateManager::SetMaterial(const D3DMATERIAL8 * pMaterial)
{
	m_lpD3DDev->SetMaterial(pMaterial);
	m_CurrentState.m_D3DMaterial = *pMaterial;
}
void CStateManager::GetMaterial(D3DMATERIAL8 * pMaterial)
{
	// Set the renderstate and remember it.
	*pMaterial = m_CurrentState.m_D3DMaterial;
}

// Renderstates
DWORD CStateManager::GetRenderState(D3DRENDERSTATETYPE Type)
{
	return m_CurrentState.m_RenderStates[Type];
}

void CStateManager::SaveRenderState(D3DRENDERSTATETYPE Type, DWORD dwValue)
{
#ifdef _DEBUG
	if (m_bRenderStateSavingFlag[Type])
	{
		Tracef(" CStateManager::SaveRenderState - This render state is already saved [%d, %d]\n", Type, dwValue);
		StateManager_Assert(!" This render state is already saved!");
	}
	m_bRenderStateSavingFlag[Type] = TRUE;
#endif _DEBUG

	// Check that we have set this up before, if not, the default is this.
	m_CopyState.m_RenderStates[Type] = m_CurrentState.m_RenderStates[Type];
	SetRenderState(Type, dwValue);
}

void CStateManager::RestoreRenderState(D3DRENDERSTATETYPE Type)
{
#ifdef _DEBUG
	if (!m_bRenderStateSavingFlag[Type])
	{
		Tracef(" CStateManager::SaveRenderState - This render state was not saved [%d, %d]\n", Type);
		StateManager_Assert(!" This render state was not saved!");
	}
	m_bRenderStateSavingFlag[Type] = FALSE;
#endif _DEBUG

	SetRenderState(Type, m_CopyState.m_RenderStates[Type]);
}

void CStateManager::SetRenderState(D3DRENDERSTATETYPE Type, DWORD Value)
{

	if (m_CurrentState.m_RenderStates[Type] == Value)
		return;

	m_lpD3DDev->SetRenderState(Type, Value);
	m_CurrentState.m_RenderStates[Type] = Value;
}

void CStateManager::GetRenderState(D3DRENDERSTATETYPE Type, DWORD * pdwValue)
{
	*pdwValue = m_CurrentState.m_RenderStates[Type];
}

// Textures
void CStateManager::SaveTexture(DWORD dwStage, LPDIRECT3DBASETEXTURE8 pTexture)
{
	// Check that we have set this up before, if not, the default is this.
	m_CopyState.m_Textures[dwStage] = m_CurrentState.m_Textures[dwStage];
	SetTexture(dwStage, pTexture);
}

void CStateManager::RestoreTexture(DWORD dwStage)
{
	SetTexture(dwStage, m_CopyState.m_Textures[dwStage]);
}

void CStateManager::SetTexture(DWORD dwStage, LPDIRECT3DBASETEXTURE8 pTexture)
{
	if (pTexture == m_CurrentState.m_Textures[dwStage])
		return;

	m_lpD3DDev->SetTexture(dwStage, pTexture);
	m_CurrentState.m_Textures[dwStage] = pTexture;
}

void CStateManager::GetTexture(DWORD dwStage, LPDIRECT3DBASETEXTURE8 * ppTexture)
{
	*ppTexture = m_CurrentState.m_Textures[dwStage];
}

// Texture stage states
void CStateManager::SaveTextureStageState(DWORD dwStage,D3DTEXTURESTAGESTATETYPE Type, DWORD dwValue)
{
	// Check that we have set this up before, if not, the default is this.
#ifdef _DEBUG
	if (m_bTextureStageStateSavingFlag[dwStage][Type])
	{
		Tracef(" CStateManager::SaveTextureStageState - This texture stage state is already saved [%d, %d]\n", dwStage, Type);
		StateManager_Assert(!" This texture stage state is already saved!");
	}
	m_bTextureStageStateSavingFlag[dwStage][Type] = TRUE;
#endif _DEBUG
	m_CopyState.m_TextureStates[dwStage][Type] = m_CurrentState.m_TextureStates[dwStage][Type];
	SetTextureStageState(dwStage, Type, dwValue);
}

void CStateManager::RestoreTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE Type)
{
#ifdef _DEBUG
	if (!m_bTextureStageStateSavingFlag[dwStage][Type])
	{
		Tracef(" CStateManager::RestoreTextureStageState - This texture stage state was not saved [%d, %d]\n", dwStage, Type);
		StateManager_Assert(!" This texture stage state was not saved!");
	}
	m_bTextureStageStateSavingFlag[dwStage][Type] = FALSE;
#endif _DEBUG
	SetTextureStageState(dwStage, Type, m_CopyState.m_TextureStates[dwStage][Type]);
}

void CStateManager::SetTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE Type, DWORD dwValue)
{
	if (m_CurrentState.m_TextureStates[dwStage][Type] == dwValue)
		return;

	m_lpD3DDev->SetTextureStageState(dwStage, Type, dwValue);
	m_CurrentState.m_TextureStates[dwStage][Type] = dwValue;
}

void CStateManager::GetTextureStageState(DWORD dwStage, D3DTEXTURESTAGESTATETYPE Type, DWORD * pdwValue)
{
	*pdwValue = m_CurrentState.m_TextureStates[dwStage][Type];
}

#ifdef DIRECTX9
void CStateManager::SaveFVF(DWORD dwFVF)
{
	m_CopyState.m_dwFVF = m_CurrentState.m_dwFVF;
	SetFVF(dwFVF);
}

void CStateManager::RestoreFVF()
{
	SetFVF(m_CopyState.m_dwFVF);
}

void CStateManager::SetFVF(DWORD dwFVF)
{
	if (m_CurrentState.m_dwFVF != dwFVF)
	{
		m_lpD3DDev->SetFVF(dwFVF);
		m_CurrentState.m_dwFVF = dwFVF;
	}
}

void CStateManager::GetFVF(DWORD* dwFVF)
{
	*dwFVF = m_CurrentState.m_dwFVF;
}
#endif

void CStateManager::PushRenderTarget()
{
	LPDIRECT3DSURFACE9 depthSurface;
	m_lpD3DDev->GetDepthStencilSurface(&depthSurface);
	m_renderTargetStack.push_back(depthSurface);

	LPDIRECT3DSURFACE9 renderTarget;
	m_lpD3DDev->GetRenderTarget(0, &renderTarget);
	m_renderTargetStack.push_back(renderTarget);
}

void CStateManager::PopRenderTarget()
{
	assert(m_renderTargetStack.size() >= 2);

	// Restore the render target
	LPDIRECT3DSURFACE9 renderTarget = m_renderTargetStack.back();
	m_lpD3DDev->SetRenderTarget(0, renderTarget);
	renderTarget->Release();
	m_renderTargetStack.pop_back();

	// Restore the depth stencil surface
	LPDIRECT3DSURFACE9 depthSurface = m_renderTargetStack.back();
	m_lpD3DDev->SetDepthStencilSurface(depthSurface);
	depthSurface->Release();
	m_renderTargetStack.pop_back();
}

void CStateManager::ReleaseRenderTargets()
{
	for (auto& surface : m_renderTargetStack)
	{
		if (surface)
		{
			surface->Release();
			surface = nullptr;
		}
	}
	m_renderTargetStack.clear();
}

void CStateManager::EnableClipPlane(int planeId, float A, float B, float C, float D)
{
	// 1. Transform to the view space
	D3DXPLANE planeWorld(A, B, C, D);
	D3DXMATRIX viewMat, viewMatInv, viewMatInvT;
	GetTransform(D3DTS_VIEW, &viewMat);
	D3DXMatrixInverse(&viewMatInv, nullptr, &viewMat);
	D3DXMatrixTranspose(&viewMatInvT, &viewMatInv);

	D3DXPLANE planeView;
	D3DXPlaneTransform(&planeView, &planeWorld, &viewMatInvT);

	// 2. Transform to the clip space
	D3DXMATRIX projMat;
	GetTransform(D3DTS_PROJECTION, &projMat);

	float plane[4] = {
		planeView.a / projMat._11,
		planeView.b / projMat._22,
		planeView.d / projMat._43,
		planeView.d * projMat._33 / projMat._43 - planeView.c
	};
	m_lpD3DDev->SetClipPlane(planeId, plane);
	m_userClipPlanes |= 1 << planeId;
	m_lpD3DDev->SetRenderState(D3DRS_CLIPPLANEENABLE, m_userClipPlanes);
	m_CurrentState.m_RenderStates[D3DRS_CLIPPLANEENABLE] = m_userClipPlanes;
}

void CStateManager::DisableClipPlane(int planeId)
{
	m_userClipPlanes &= !(1 << planeId);
	m_lpD3DDev->SetRenderState(D3DRS_CLIPPLANEENABLE, m_userClipPlanes);
	m_CurrentState.m_RenderStates[D3DRS_CLIPPLANEENABLE] = m_userClipPlanes;
}

//Converting2DirectX9
#ifdef DIRECTX9
void CStateManager::SaveSamplerState(DWORD dwStage, D3DSAMPLERSTATETYPE Type, DWORD dwValue)
{
	// Check that we have set this up before, if not, the default is this.
#ifdef _DEBUG
	if (m_bSamplerStateSavingFlag[dwStage][Type])
	{
	}
	m_bSamplerStateSavingFlag[dwStage][Type] = TRUE;
#endif _DEBUG
	m_CopyState.m_SamplerStates[dwStage][Type] = m_CurrentState.m_SamplerStates[dwStage][Type];
	SetSamplerState(dwStage, Type, dwValue);
}

void CStateManager::RestoreSamplerState(DWORD dwStage, D3DSAMPLERSTATETYPE Type)
{
#ifdef _DEBUG
	if (!m_bSamplerStateSavingFlag[dwStage][Type])
	{
	}
	m_bSamplerStateSavingFlag[dwStage][Type] = FALSE;
#endif _DEBUG
	SetSamplerState(dwStage, Type, m_CopyState.m_SamplerStates[dwStage][Type]);
}

void CStateManager::SetSamplerState(DWORD dwStage, D3DSAMPLERSTATETYPE Type, DWORD dwValue)
{
	if (m_CurrentState.m_SamplerStates[dwStage][Type] == dwValue)
		return;

	m_lpD3DDev->SetSamplerState(dwStage, Type, dwValue);

	m_CurrentState.m_SamplerStates[dwStage][Type] = dwValue;
}

void CStateManager::GetSamplerState(DWORD dwStage, D3DSAMPLERSTATETYPE Type, DWORD* pdwValue)
{
	*pdwValue = m_CurrentState.m_SamplerStates[dwStage][Type];
}
#endif

// Vertex Shader
#ifdef DIRECTX9
void CStateManager::SaveVertexShader(LPDIRECT3DVERTEXSHADER9 dwShader)
{
	m_CopyState.m_dwVertexShader = m_CurrentState.m_dwVertexShader;
	SetVertexShader(dwShader);
}

void CStateManager::SetVertexShader(LPDIRECT3DVERTEXSHADER9 dwShader)
{
	if (m_CurrentState.m_dwVertexShader != dwShader)
	{
		m_CurrentState.m_dwVertexShader = dwShader;
		m_lpD3DDev->SetVertexShader(dwShader);
	}
}

void CStateManager::RestoreVertexShader()
{
	SetVertexShader(m_CopyState.m_dwVertexShader);
}

void CStateManager::GetVertexShader(LPDIRECT3DVERTEXSHADER9* pdwShader)
{
	*pdwShader = m_CurrentState.m_dwVertexShader;
}
#else
void CStateManager::SaveVertexShader(DWORD dwShader)
{
	m_CopyState.m_dwVertexShader = m_CurrentState.m_dwVertexShader;
	SetVertexShader(dwShader);
}

void CStateManager::RestoreVertexShader()
{
	SetVertexShader(m_CopyState.m_dwVertexShader);
}

void CStateManager::SetVertexShader(DWORD dwShader)
{
	if (m_CurrentState.m_dwVertexShader == dwShader)
		return;

	m_lpD3DDev->SetVertexShader(dwShader);
	m_CurrentState.m_dwVertexShader = dwShader;
}

void CStateManager::GetVertexShader(DWORD* pdwShader)
{
	*pdwShader = m_CurrentState.m_dwVertexShader;
}
#endif

// Pixel Shader
#ifdef DIRECTX9
void CStateManager::SaveVertexDeclaration(LPDIRECT3DVERTEXDECLARATION9 dwShader)
{
	m_CopyState.m_dwVertexDeclaration = m_CurrentState.m_dwVertexDeclaration;
	SetVertexDeclaration(dwShader);
}

void CStateManager::RestoreVertexDeclaration()
{
	SetVertexDeclaration(m_CopyState.m_dwVertexDeclaration);
}

void CStateManager::SetVertexDeclaration(LPDIRECT3DVERTEXDECLARATION9 dwShader)
{
	if (m_CurrentState.m_dwVertexDeclaration == dwShader)
		return;

	m_lpD3DDev->SetVertexDeclaration(dwShader);
	m_CurrentState.m_dwVertexDeclaration = dwShader;
}

void CStateManager::GetVertexDeclaration(LPDIRECT3DVERTEXDECLARATION9* pdwShader)
{
	*pdwShader = m_CurrentState.m_dwVertexDeclaration;
}

// Pixel Shader
void CStateManager::SavePixelShader(LPDIRECT3DPIXELSHADER9 dwShader)
{
	m_CopyState.m_dwPixelShader = m_CurrentState.m_dwPixelShader;
	SetPixelShader(dwShader);
}

void CStateManager::SetPixelShader(LPDIRECT3DPIXELSHADER9 dwShader)
{
	if (m_CurrentState.m_dwPixelShader != dwShader)
	{
		m_CurrentState.m_dwPixelShader = dwShader;
		m_lpD3DDev->SetPixelShader(dwShader);
	}
}

void CStateManager::RestorePixelShader()
{
	SetPixelShader(m_CopyState.m_dwPixelShader);
}

void CStateManager::GetPixelShader(LPDIRECT3DPIXELSHADER9* pdwShader)
{
	*pdwShader = m_CurrentState.m_dwPixelShader;
}
#else
void CStateManager::SavePixelShader(DWORD dwShader)
{
	m_CopyState.m_dwPixelShader = m_CurrentState.m_dwPixelShader;
	SetPixelShader(dwShader);
}

void CStateManager::RestorePixelShader()
{
	SetPixelShader(m_CopyState.m_dwPixelShader);
}

void CStateManager::SetPixelShader(DWORD dwShader)
{
	if (m_CurrentState.m_dwPixelShader == dwShader)
		return;

	m_lpD3DDev->SetPixelShader(dwShader);
	m_CurrentState.m_dwPixelShader = dwShader;
}

void CStateManager::GetPixelShader(DWORD* pdwShader)
{
	*pdwShader = m_CurrentState.m_dwPixelShader;
}
#endif

// *** These states are cached, but not protected from multiple sends of the same value.
// Transform
void CStateManager::SaveTransform(D3DTRANSFORMSTATETYPE Type, const D3DMATRIX* pMatrix)
{
#ifdef _DEBUG
	if (m_bTransformSavingFlag[Type])
	{
		Tracef(" CStateManager::SaveTransform - This transform is already saved [%d]\n", Type);
		StateManager_Assert(!" This trasform is already saved!");
	}
	m_bTransformSavingFlag[Type] = TRUE;
#endif _DEBUG

	m_CopyState.m_Matrices[Type] = m_CurrentState.m_Matrices[Type];
	SetTransform(Type, (D3DXMATRIX *)pMatrix);
}

void CStateManager::RestoreTransform(D3DTRANSFORMSTATETYPE Type)
{
#ifdef _DEBUG
	if (!m_bTransformSavingFlag[Type])
	{
		Tracef(" CStateManager::RestoreTransform - This transform was not saved [%d]\n", Type);
		StateManager_Assert(!" This render state was not saved!");
	}
	m_bTransformSavingFlag[Type] = FALSE;
#endif _DEBUG

	SetTransform(Type, &m_CopyState.m_Matrices[Type]);
}

// Don't cache-check the transform.  To much to do
void CStateManager::SetTransform(D3DTRANSFORMSTATETYPE Type, const D3DMATRIX* pMatrix)
{
	if (m_bScene)
	{
		m_lpD3DDev->SetTransform(Type, pMatrix);
	}
	else
	{
#ifndef __ANDROID__
		assert(D3DTS_VIEW==Type || D3DTS_PROJECTION==Type || D3DTS_WORLD==Type);
#endif
	}

	m_CurrentState.m_Matrices[Type] = *pMatrix;
}

void CStateManager::GetTransform(D3DTRANSFORMSTATETYPE Type, D3DMATRIX * pMatrix)
{
	*pMatrix = m_CurrentState.m_Matrices[Type];
}

// SetVertexShaderConstant
void CStateManager::SaveVertexShaderConstant(DWORD dwRegister,CONST void* pConstantData,DWORD dwConstantCount)
{
	for (DWORD i = 0; i < dwConstantCount; i++)
	{
		if ((dwRegister + i) < STATEMANAGER_MAX_VCONSTANTS)
			m_CopyState.m_VertexShaderConstants[dwRegister + i] = m_CurrentState.m_VertexShaderConstants[dwRegister + i];
	}

	SetVertexShaderConstant(dwRegister, pConstantData, dwConstantCount);
}

void CStateManager::RestoreVertexShaderConstant(DWORD dwRegister, DWORD dwConstantCount)
{
	if (dwRegister + dwConstantCount <= STATEMANAGER_MAX_VCONSTANTS)
		SetVertexShaderConstant(dwRegister, &m_CopyState.m_VertexShaderConstants[dwRegister], dwConstantCount);
}

void CStateManager::SetVertexShaderConstant(DWORD dwRegister,CONST void* pConstantData,DWORD dwConstantCount)
{
#ifdef DIRECTX9
	m_lpD3DDev->SetVertexShaderConstantF(dwRegister, (const float*)pConstantData, dwConstantCount);
#else
	m_lpD3DDev->SetVertexShaderConstant(dwRegister, pConstantData, dwConstantCount);
#endif

	if ((dwRegister + dwConstantCount) <= STATEMANAGER_MAX_VCONSTANTS)
	{
		memcpy(&m_CurrentState.m_VertexShaderConstants[dwRegister], pConstantData, dwConstantCount * sizeof(D3DXVECTOR4));
	}
}

// SetPixelShaderConstant
void CStateManager::SavePixelShaderConstant(DWORD dwRegister,CONST void* pConstantData,DWORD dwConstantCount)
{
	for (DWORD i = 0; i < dwConstantCount; i++)
	{
		if ((dwRegister + i) < STATEMANAGER_MAX_PCONSTANTS)
			m_CopyState.m_PixelShaderConstants[dwRegister + i] = m_CurrentState.m_PixelShaderConstants[dwRegister + i];
	}

	SetPixelShaderConstant(dwRegister, pConstantData, dwConstantCount);
}

void CStateManager::RestorePixelShaderConstant(DWORD dwRegister, DWORD dwConstantCount)
{
	if (dwRegister + dwConstantCount <= STATEMANAGER_MAX_PCONSTANTS)
		SetPixelShaderConstant(dwRegister, &m_CopyState.m_PixelShaderConstants[dwRegister], dwConstantCount);
}

void CStateManager::SetPixelShaderConstant(DWORD dwRegister,CONST void* pConstantData,DWORD dwConstantCount)
{
#ifdef DIRECTX9
	m_lpD3DDev->SetPixelShaderConstantF(dwRegister, (const float*)pConstantData, dwConstantCount);
#else
	m_lpD3DDev->SetPixelShaderConstant(dwRegister, pConstantData, dwConstantCount);
#endif

	// Guard against out-of-bounds array corruption when PS 3.0 shaders write to registers >= STATEMANAGER_MAX_PCONSTANTS
	if ((dwRegister + dwConstantCount) <= STATEMANAGER_MAX_PCONSTANTS)
	{
		memcpy(&m_CurrentState.m_PixelShaderConstants[dwRegister], pConstantData, dwConstantCount * sizeof(D3DXVECTOR4));
	}
}

void CStateManager::SaveStreamSource(UINT StreamNumber, LPDIRECT3DVERTEXBUFFER8 pStreamData,UINT Stride)
{
	// Check that we have set this up before, if not, the default is this.
	m_CopyState.m_StreamData[StreamNumber] = m_CurrentState.m_StreamData[StreamNumber];
	SetStreamSource(StreamNumber, pStreamData, Stride);
}

void CStateManager::RestoreStreamSource(UINT StreamNumber)
{
	SetStreamSource(StreamNumber,
					m_CopyState.m_StreamData[StreamNumber].m_lpStreamData,
					m_CopyState.m_StreamData[StreamNumber].m_Stride);
}

void CStateManager::SetStreamSource(UINT StreamNumber, LPDIRECT3DVERTEXBUFFER8 pStreamData, UINT Stride)
{
	CStreamData kStreamData(pStreamData, Stride);
	if (m_CurrentState.m_StreamData[StreamNumber] == kStreamData)
		return;

#ifdef DIRECTX9
	m_lpD3DDev->SetStreamSource(StreamNumber, pStreamData, 0, Stride);
#else
	m_lpD3DDev->SetStreamSource(StreamNumber, pStreamData, Stride);
#endif
	m_CurrentState.m_StreamData[StreamNumber] = kStreamData;
}

void CStateManager::SaveIndices(LPDIRECT3DINDEXBUFFER8 pIndexData, UINT BaseVertexIndex)
{
	m_CopyState.m_IndexData = m_CurrentState.m_IndexData;
	SetIndices(pIndexData, BaseVertexIndex);
}

void CStateManager::RestoreIndices()
{
	SetIndices(m_CopyState.m_IndexData.m_lpIndexData, m_CopyState.m_IndexData.m_BaseVertexIndex);
}

void CStateManager::SetIndices(LPDIRECT3DINDEXBUFFER8 pIndexData, UINT BaseVertexIndex)
{
	CIndexData kIndexData(pIndexData, BaseVertexIndex);

	if (m_CurrentState.m_IndexData == kIndexData)
		return;

#ifdef DIRECTX9
	m_lpD3DDev->SetIndices(pIndexData);
#else
	m_lpD3DDev->SetIndices(pIndexData, BaseVertexIndex);
#endif
	m_CurrentState.m_IndexData = kIndexData;
}

HRESULT CStateManager::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	return (m_lpD3DDev->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount));
}

HRESULT CStateManager::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, const void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	m_CurrentState.m_StreamData[0] = NULL;
	return (m_lpD3DDev->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride));
}

#ifdef DIRECTX9
HRESULT CStateManager::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT minIndex, UINT NumVertices, UINT startIndex, UINT primCount, INT baseVertexIndex)
{
	return (m_lpD3DDev->DrawIndexedPrimitive(PrimitiveType, baseVertexIndex, minIndex, NumVertices, startIndex, primCount));
}
#else
HRESULT CStateManager::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT minIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{

	return (m_lpD3DDev->DrawIndexedPrimitive(PrimitiveType, minIndex, NumVertices, startIndex, primCount));
}
#endif

HRESULT CStateManager::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertexIndices, UINT PrimitiveCount, CONST void * pIndexData, D3DFORMAT IndexDataFormat, CONST void * pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	m_CurrentState.m_IndexData = NULL;
	m_CurrentState.m_StreamData[0] = NULL;
	return (m_lpD3DDev->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertexIndices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride));
}
//martysama0134's fb0d8d5d0566583187a94f9a823b2075