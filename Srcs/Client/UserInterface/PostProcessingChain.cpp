#include "stdafx.h"
#ifndef DISABLE_JTX_DYNAMIC_LIGHT
#include "../EterLib/StdAfx.h"
#include "../EterLib/StateManager.h"
#include "../EterPack/EterPackManager.h"
#include "PostProcessingChain.h"
#include "xmllite.h"

typedef struct SRTTVertex
{
	D3DXVECTOR4 p;
	D3DXVECTOR2 t;
} TRTTVertex;

CPostProcessingChain::CPostProcessingChain() : m_lpSourceTexture(nullptr), m_lpLastPass(nullptr), m_lpTarget(nullptr), m_lpBackupTargetSurface(nullptr), m_lpSourceTextureSurface(nullptr), m_lpTargetSurface(nullptr), m_lpDepthStencilSurface(nullptr), m_lpDepthStencilBackup(nullptr), m_lpScreenQuad(nullptr), m_lpLastPassSurface(nullptr)
{
}
CPostProcessingChain::~CPostProcessingChain()
{
	ReleaseResources();
}

void CPostProcessingChain::ReleaseResources()
{
	if (m_lpSourceTextureSurface)
	{
		m_lpSourceTextureSurface->Release();
		m_lpSourceTextureSurface = nullptr;
	}
	if (m_lpSourceTexture)
	{
		m_lpSourceTexture->Release();
		m_lpSourceTexture = nullptr;
	}
	if (m_lpLastPassSurface)
	{
		m_lpLastPassSurface->Release();
		m_lpLastPassSurface = nullptr;
	}
	if (m_lpLastPass)
	{
		m_lpLastPass->Release();
		m_lpLastPass = nullptr;
	}
	if (m_lpTarget)
	{
		m_lpTarget->Release();
		m_lpTarget = nullptr;
	}
	if (m_lpTargetSurface)
	{
		m_lpTargetSurface->Release();
		m_lpTargetSurface = nullptr;
	}
	if (m_lpDepthStencilSurface)
	{
		m_lpDepthStencilSurface->Release();
		m_lpDepthStencilSurface = nullptr;
	}
	if (m_lpScreenQuad)
	{
		m_lpScreenQuad->Release();
		m_lpScreenQuad = nullptr;
	}

	RemoveAllEffects();
}
bool CPostProcessingChain::CreateResources()
{
	ReleaseResources();

	D3DSURFACE_DESC kDesc;
	IDirect3DSurface9* pSurface;
	STATEMANAGER.GetDevice()->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pSurface);
	pSurface->GetDesc(&kDesc);
	pSurface->Release();

	if (FAILED(STATEMANAGER.GetDevice()->CreateTexture(kDesc.Width, kDesc.Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &m_lpSourceTexture, nullptr)))
	{
		TraceError("CPostProcessingChain unable to create render target");
		return false;
	}
	m_lpSourceTexture->GetSurfaceLevel(0, &m_lpSourceTextureSurface);
	if (FAILED(STATEMANAGER.GetDevice()->CreateTexture(kDesc.Width, kDesc.Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &m_lpLastPass, nullptr)))
	{
		TraceError("CPostProcessingChain unable to create pass render target");
		return false;
	}
	m_lpLastPass->GetSurfaceLevel(0, &m_lpLastPassSurface);
	if (FAILED(STATEMANAGER.GetDevice()->CreateTexture(kDesc.Width, kDesc.Height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F, D3DPOOL_DEFAULT, &m_lpTarget, nullptr)))
	{
		TraceError("CPostProcessingChain unable to create final render target");
		return false;
	}
	m_lpTarget->GetSurfaceLevel(0, &m_lpTargetSurface);

	STATEMANAGER.GetDevice()->GetDepthStencilSurface(&pSurface);
	pSurface->GetDesc(&kDesc);
	pSurface->Release();
	if (FAILED(STATEMANAGER.GetDevice()->CreateDepthStencilSurface(kDesc.Width, kDesc.Height, D3DFMT_D16, kDesc.MultiSampleType, kDesc.MultiSampleQuality, FALSE, &m_lpDepthStencilSurface, nullptr)))
	{
		TraceError("CPostProcessingChain unable to create depth surface");
		return false;
	}

	if (FAILED(STATEMANAGER.GetDevice()->CreateVertexBuffer(sizeof(TRTTVertex) * 6, 0, D3DFVF_XYZRHW | D3DFVF_TEX1, D3DPOOL_MANAGED, &m_lpScreenQuad, nullptr)))
	{
		TraceError("CPostProcessingChain unable to create screen quad");
		return false;
	}

	float fWidth = static_cast<float>(kDesc.Width) - 0.5f;
	float fHeight = static_cast<float>(kDesc.Height) - 0.5f;

	TRTTVertex* pVertices;
	m_lpScreenQuad->Lock(0, 0, reinterpret_cast<void**>(&pVertices), 0);
	pVertices[0].p = D3DXVECTOR4(-0.5f, -0.5f, 0.0f, 1.0f);
	pVertices[0].t = D3DXVECTOR2(0.0f, 0.0f);
	pVertices[1].p = D3DXVECTOR4(-0.5f, fHeight, 0.0f, 1.0f);
	pVertices[1].t = D3DXVECTOR2(0.0f, 1.0f);
	pVertices[2].p = D3DXVECTOR4(fWidth, fHeight, 0.0f, 1.0f);
	pVertices[2].t = D3DXVECTOR2(1.0f, 1.0f);
	pVertices[3].p = D3DXVECTOR4(-0.5f, -0.5f, 0.0f, 1.0f);
	pVertices[3].t = D3DXVECTOR2(0.0f, 0.0f);
	pVertices[4].p = D3DXVECTOR4(fWidth, fHeight, 0.0f, 1.0f);
	pVertices[4].t = D3DXVECTOR2(1.0f, 1.0f);
	pVertices[5].p = D3DXVECTOR4(fWidth, -0.5f, 0.0f, 1.0f);
	pVertices[5].t = D3DXVECTOR2(1.0f, 0.0f);
	m_lpScreenQuad->Unlock();

	return true;
}

bool CPostProcessingChain::BeginScene()
{
	if (!m_lpSourceTexture || !m_lpLastPass || !m_lpTarget)
	{
		if (!CreateResources())
		{
			return false;
		}
	}

	STATEMANAGER.GetDevice()->GetRenderTarget(0, &m_lpBackupTargetSurface);
	STATEMANAGER.GetDevice()->SetRenderTarget(0, m_lpSourceTextureSurface);
	STATEMANAGER.GetDevice()->GetDepthStencilSurface(&m_lpDepthStencilBackup);

	// Clear the render target and the depth stencil
	STATEMANAGER.GetDevice()->Clear(0, nullptr, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	return true;
}

bool CPostProcessingChain::EndScene()
{
	// Restore the original render target
	STATEMANAGER.GetDevice()->SetRenderTarget(0, m_lpBackupTargetSurface);
	// Restore the original depth stencil surface
	STATEMANAGER.GetDevice()->SetDepthStencilSurface(m_lpDepthStencilBackup);

	// Release the depth stencil backup surface
	if (m_lpDepthStencilBackup)
	{
		m_lpDepthStencilBackup->Release();
		m_lpDepthStencilBackup = nullptr;
	}
	// Release the backup render target surface
	if (m_lpBackupTargetSurface)
	{
		m_lpBackupTargetSurface->Release();
		m_lpBackupTargetSurface = nullptr;
	}

	// Set the fixed-function vertex format
	STATEMANAGER.GetDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);
	STATEMANAGER.GetDevice()->SetPixelShader(nullptr);
	STATEMANAGER.GetDevice()->SetVertexShader(nullptr);
	STATEMANAGER.RestoreRenderState(D3DRS_ZENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_FOGENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
	return true;
}





bool CPostProcessingChain::Render()
{
	LPDIRECT3DDEVICE9 device = STATEMANAGER.GetDevice();

	IDirect3DStateBlock9* pStateBlock = nullptr;

	if (FAILED(device->CreateStateBlock(D3DSBT_ALL, &pStateBlock)))
	{
		pStateBlock = nullptr;
	}

	// Set render target
	device->SetRenderTarget(0, m_lpLastPassSurface);

	// Save render states
	STATEMANAGER.SaveRenderState(D3DRS_ZENABLE, FALSE);
	STATEMANAGER.SaveRenderState(D3DRS_FOGENABLE, FALSE);


	// Copy source texture to last pass surface
	device->Clear(
		0,
		nullptr,
		D3DCLEAR_TARGET,
		D3DCOLOR_XRGB(0, 0, 0),
		1.0f,
		0
	);

	device->SetTexture(0, m_lpSourceTexture);
	device->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
	device->SetStreamSource(
		0,
		m_lpScreenQuad,
		0,
		sizeof(TRTTVertex)
	);

	device->DrawPrimitive(
		D3DPT_TRIANGLELIST,
		0,
		2
	);


	IDirect3DSurface9* pCurrentTarget = m_lpTargetSurface;

	device->SetTexture(1, m_lpSourceTexture);


	for (auto& effect : m_kEffects)
	{
		D3DSURFACE_DESC kDesc;
		pCurrentTarget->GetDesc(&kDesc);

		D3DXVECTOR4 vScreenSize(
			(float)kDesc.Width,
			(float)kDesc.Height,
			0.0f,
			0.0f
		);

		effect.GetConstants()->SetFloatArray(
			device,
			"ScreenSize",
			reinterpret_cast<float*>(&vScreenSize),
			4
		);


		if (effect.Apply())
		{
			device->SetRenderTarget(0, pCurrentTarget);

			device->Clear(
				0,
				nullptr,
				D3DCLEAR_TARGET,
				D3DCOLOR_XRGB(0,0,0),
				1.0f,
				0
			);


			device->SetTexture(
				0,
				pCurrentTarget == m_lpLastPassSurface ?
				m_lpTarget :
				m_lpLastPass
			);


			device->SetStreamSource(
				0,
				m_lpScreenQuad,
				0,
				sizeof(TRTTVertex)
			);

			device->DrawPrimitive(
				D3DPT_TRIANGLELIST,
				0,
				2
			);


			pCurrentTarget =
				(pCurrentTarget == m_lpLastPassSurface)
				?
				m_lpTargetSurface
				:
				m_lpLastPassSurface;
		}
	}


	device->SetTexture(1, nullptr);


	// Restore normal backbuffer
	device->SetRenderTarget(0, m_lpBackupTargetSurface);


	device->SetTexture(
		0,
		pCurrentTarget == m_lpLastPassSurface ?
		m_lpTarget :
		m_lpLastPass
	);


	device->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);

	device->SetStreamSource(
		0,
		m_lpScreenQuad,
		0,
		sizeof(TRTTVertex)
	);


	device->DrawPrimitive(
		D3DPT_TRIANGLELIST,
		0,
		2
	);


	// Restore states
	STATEMANAGER.RestoreRenderState(D3DRS_ZENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_FOGENABLE);


	if (pStateBlock)
	{
		pStateBlock->Apply();
		pStateBlock->Release();
	}


	return true;
}

CPostProcessingEffect CPostProcessingChain::AddEffect(const char* c_pszFileName)
{
	CPostProcessingEffect kEffect;
	CMappedFile kFile;
	LPCVOID pData;
	if (!CEterPackManager::Instance().Get(kFile, c_pszFileName, &pData))
	{
		return kEffect;
	}
	if (!kEffect.CreateShader(kFile))
	{
		return CPostProcessingEffect();
	}
	AddEffect(kEffect);
	return kEffect;
}


bool CPostProcessingChain::AddEffect(const CPostProcessingEffect& c_kEffect)
{
	m_kEffects.push_back(c_kEffect);
	return true;
}
bool CPostProcessingChain::RemoveEffect(const CPostProcessingEffect& c_kEffect)
{
	bool bResult = false;
	for (std::list<CPostProcessingEffect>::iterator it = m_kEffects.begin(); it != m_kEffects.end(); ++it)
	{
		if ((*it) == c_kEffect)
		{
			m_kEffects.erase(it--);
			bResult = true;
		}
	}
	return bResult;
}

bool CPostProcessingChain::RemoveAllEffects()
{
	m_kEffects.clear();
	return true;
}

std::list<CPostProcessingEffect>& CPostProcessingChain::GetEffects()
{
	return m_kEffects;
}
#endif
