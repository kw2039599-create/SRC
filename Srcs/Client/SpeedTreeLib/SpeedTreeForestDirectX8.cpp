/*
* CSpeedTreeForestDirectX8 Class
*
* (c) 2003 IDV, Inc.
*
* This class is provided to illustrate one way to incorporate
* SpeedTreeRT into an OpenGL application.  All of the SpeedTreeRT
* calls that must be made on a per tree basis are done by this class.
* Calls that apply to all trees (i.e. static SpeedTreeRT functions)
* are made in the functions in main.cpp.
*
*
* *** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
*
* This software is supplied under the terms of a license agreement or
* nondisclosure agreement with Interactive Data Visualization and may
* not be copied or disclosed except in accordance with the terms of
* that agreement.
*
* Copyright (c) 2001-2003 IDV, Inc.
* All Rights Reserved.
*
* IDV, Inc.
* 1233 Washington St. Suite 610
* Columbia, SC 29201
* Voice: (803) 799-1699
* Fax: (803) 931-0320
* Web: http://www.idvinc.com
*/

#include "StdAfx.h"

#include <stdio.h>

#include <MicrosoftDirectX/d3d9.h>
#include <MicrosoftDirectX/d3dx9.h>
#include <MicrosoftDirectX/d3d9types.h>


#include "../EterBase/Timer.h"
#include "../Eterlib/StateManager.h"
#include "../Eterlib/Camera.h"

#include "SpeedTreeForestDirectX8.h"
#include "SpeedTreeConfig.h"
#include "VertexShaders.h"

///////////////////////////////////////////////////////////////////////
// CSpeedTreeForestDirectX8::CSpeedTreeForestDirectX8

CSpeedTreeForestDirectX8::CSpeedTreeForestDirectX8() : m_pDx(NULL), m_dwBranchVertexShader(0), m_dwLeafVertexShader(0) {}

///////////////////////////////////////////////////////////////////////
// CSpeedTreeForestDirectX8::~CSpeedTreeForestDirectX8

CSpeedTreeForestDirectX8::~CSpeedTreeForestDirectX8() = default;

///////////////////////////////////////////////////////////////////////
// CSpeedTreeForestDirectX8::InitVertexShaders

bool CSpeedTreeForestDirectX8::InitVertexShaders(void)
{
	NANOBEGIN
		// load the vertex shaders
		if (!m_dwBranchVertexShader)
			m_dwBranchVertexShader = LoadBranchShader(m_pDx);

	if (!m_dwLeafVertexShader)
		m_dwLeafVertexShader = LoadLeafShader(m_pDx);

	if (m_dwBranchVertexShader && m_dwLeafVertexShader)
	{
		CSpeedTreeWrapper::SetVertexShaders(m_dwBranchVertexShader, m_dwLeafVertexShader);
		return true;
	}

	NANOEND
		return false;
}


bool CSpeedTreeForestDirectX8::SetRenderingDevice(LPDIRECT3DDEVICE9 lpDevice)
{
	m_pDx = lpDevice;

#ifndef ENABLE_OPENGL
	if (!InitVertexShaders())
		return false;
#else
	InitVertexShaders();
#endif

	const float c_afLightPosition[4] = { -0.707f, -0.300f, 0.707f, 0.0f };
	const float	c_afLightAmbient[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	const float	c_afLightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const float	c_afLightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float afLight1[] =
	{
		c_afLightPosition[0], c_afLightPosition[1], c_afLightPosition[2], // pos
		c_afLightDiffuse[0], c_afLightDiffuse[1], c_afLightDiffuse[2], // diffuse
		c_afLightAmbient[0], c_afLightAmbient[1], c_afLightAmbient[2], // ambient
		c_afLightSpecular[0], c_afLightSpecular[1], c_afLightSpecular[2], // specular
		c_afLightPosition[3], // directional flag
		1.0f, 0.0f, 0.0f // attenuation (constant, linear, quadratic)
	};

	CSpeedTreeRT::SetNumWindMatrices(c_nNumWindMatrices);

	CSpeedTreeRT::SetLightAttributes(0, afLight1);
	CSpeedTreeRT::SetLightState(0, true);
#ifdef ENABLE_OPENGL
	const float c_afDefaultCamPos[3] = { 0.0f, 0.0f, 0.0f };
	const float c_afDefaultCamDir[3] = { 0.0f, 1.0f, 0.0f };
	CSpeedTreeRT::SetCamera(c_afDefaultCamPos, c_afDefaultCamDir);
#endif
	return true;
}

///////////////////////////////////////////////////////////////////////
// CSpeedTreeForestDirectX8::UploadWindMatrix

void CSpeedTreeForestDirectX8::UploadWindMatrix(UINT uiLocation, const float* pMatrix) const
{
	STATEMANAGER.SetVertexShaderConstant(uiLocation, pMatrix, 4);
}

void CSpeedTreeForestDirectX8::UpdateCompundMatrix(const D3DXVECTOR3& c_rEyeVec, const D3DXMATRIX& c_rmatView, const D3DXMATRIX& c_rmatProj)
{
	// setup composite matrix for shader
	D3DXMATRIX matBlend;
	D3DXMatrixIdentity(&matBlend);

	D3DXMATRIX matBlendShader;
	D3DXMatrixMultiply(&matBlendShader, &c_rmatView, &c_rmatProj);

	float afDirection[3];
	afDirection[0] = matBlendShader.m[0][2];
	afDirection[1] = matBlendShader.m[1][2];
	afDirection[2] = matBlendShader.m[2][2];
	CSpeedTreeRT::SetCamera(c_rEyeVec, afDirection);

	D3DXMatrixTranspose(&matBlendShader, &matBlendShader);
	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_CompoundMatrix, &matBlendShader, 4);
}

///////////////////////////////////////////////////////////////////////
// CSpeedTreeForestDirectX8::Render

void CSpeedTreeForestDirectX8::RenderReflection(unsigned long ulRenderBitVector)
{
	if (m_pMainTreeMap.empty())
		return;

	DWORD dwLightState = STATEMANAGER.GetRenderState(D3DRS_LIGHTING);
	DWORD dwColorVertexState = STATEMANAGER.GetRenderState(D3DRS_COLORVERTEX);
	DWORD dwFogVertexMode = STATEMANAGER.GetRenderState(D3DRS_FOGVERTEXMODE);

	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
	STATEMANAGER.SetRenderState(D3DRS_COLORVERTEX, TRUE);

	UINT uiCount;
	TTreeMap::const_iterator itor = m_pMainTreeMap.begin();

	if (ulRenderBitVector & Forest_RenderToShadow)
	{
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	}
	else
	{
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, TRUE);
		STATEMANAGER.SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
		STATEMANAGER.SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
		STATEMANAGER.SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);

		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	}

	STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_CW);

	// set up fog if it is enabled

	STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);


	if (ulRenderBitVector & Forest_RenderLeaves)
	{
		STATEMANAGER.SetFVF(D3DFVF_SPEEDTREE_LEAF_VERTEX);
		STATEMANAGER.SetVertexShader(m_dwLeafVertexShader);

		if (STATEMANAGER.GetRenderState(D3DRS_FOGENABLE)) {}
		if (ulRenderBitVector & Forest_RenderToShadow || ulRenderBitVector & Forest_RenderToMiniMap)
		{
			STATEMANAGER.SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL);
			STATEMANAGER.SaveRenderState(D3DRS_ALPHAREF, 0x00000000);
		}

		itor = m_pMainTreeMap.begin();

		while (itor != m_pMainTreeMap.end())
		{
			SpeedTreeWrapperPtr pMainTree = (itor++)->second;
			std::list<CSpeedTreeForest::SpeedTreeWrapperWeakPtr> ppInstances = pMainTree->GetInstances(uiCount);

			pMainTree->SetupLeafForTreeType();

			for (auto it : ppInstances)
			{
				if (it.expired())
					continue;

				auto pInstance = it.lock();
				if (!pInstance->isShow())
					continue;


				pInstance->RenderLeaves();
			}
		}

		if (ulRenderBitVector & Forest_RenderToShadow || ulRenderBitVector & Forest_RenderToMiniMap)
		{
			STATEMANAGER.SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
			STATEMANAGER.RestoreRenderState(D3DRS_ALPHAREF);
		}
	}

	if (ulRenderBitVector & Forest_RenderFronds)
	{
#ifdef __ANDROID__
		STATEMANAGER.SetFVF(D3DFVF_SPEEDTREE_BRANCH_VERTEX);
#endif
		itor = m_pMainTreeMap.begin();

		while (itor != m_pMainTreeMap.end())
		{
			SpeedTreeWrapperPtr pMainTree = (itor++)->second;
			std::list<CSpeedTreeForest::SpeedTreeWrapperWeakPtr> ppInstances = pMainTree->GetInstances(uiCount);

			pMainTree->SetupFrondForTreeType();

			for (auto it : ppInstances)
			{
				if (it.expired())
					continue;

				auto pInstance = it.lock();
				if (!pInstance->isShow())
					continue;

				pInstance->RenderFronds();
			}
		}
	}

	STATEMANAGER.SetFVF(D3DFVF_SPEEDTREE_BRANCH_VERTEX);
#ifndef __ANDROID__
	STATEMANAGER.SetVertexShader(m_dwBranchVertexShader);
#endif

	if (ulRenderBitVector & Forest_RenderBranches)
	{
#ifdef __ANDROID__
		STATEMANAGER.SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
#endif
		itor = m_pMainTreeMap.begin();

		while (itor != m_pMainTreeMap.end())
		{
			SpeedTreeWrapperPtr pMainTree = (itor++)->second;
			std::list<CSpeedTreeForest::SpeedTreeWrapperWeakPtr> ppInstances = pMainTree->GetInstances(uiCount);

			pMainTree->SetupBranchForTreeType();

			for (auto it : ppInstances)
			{
				if (it.expired())
					continue;

				auto pInstance = it.lock();
				if (!pInstance->isShow())
					continue;

#ifdef __ANDROID__
				static int s_iFirstTreePosLog = 0;
				if (++s_iFirstTreePosLog <= 3) {
					const float* tp = pInstance->GetPosition();
					D3DXVECTOR3 eye = CCameraManager::Instance().GetCurrentCamera()->GetEye();
					Tracenf("[TreePos] inst=(%.1f, %.1f, %.1f) eye=(%.1f, %.1f, %.1f)",
						tp[0], tp[1], tp[2], eye.x, eye.y, eye.z);
				}
#endif
				pInstance->RenderBranches();
			}
		}
#ifdef __ANDROID__
		STATEMANAGER.SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
#endif
	}

	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, dwLightState);
	STATEMANAGER.SetRenderState(D3DRS_COLORVERTEX, dwColorVertexState);
	STATEMANAGER.SetRenderState(D3DRS_FOGVERTEXMODE, dwFogVertexMode);

	if (!(ulRenderBitVector & Forest_RenderToShadow))
	{
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	}

	STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
}


void CSpeedTreeForestDirectX8::Render(unsigned long ulRenderBitVector)
{
	if (m_pMainTreeMap.empty())
		return;

#ifdef __ANDROID__
	UpdateSystem(CTimer::Instance().GetCurrentSecond());
#endif

	if (!(ulRenderBitVector & Forest_RenderToShadow) && !(ulRenderBitVector & Forest_RenderToMiniMap))
		UpdateCompundMatrix(CCameraManager::Instance().GetCurrentCamera()->GetEye(), ms_matView, ms_matProj);

	DWORD dwLightState = STATEMANAGER.GetRenderState(D3DRS_LIGHTING);
	DWORD dwColorVertexState = STATEMANAGER.GetRenderState(D3DRS_COLORVERTEX);
	DWORD dwFogVertexMode = STATEMANAGER.GetRenderState(D3DRS_FOGVERTEXMODE);

	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
	STATEMANAGER.SetRenderState(D3DRS_COLORVERTEX, TRUE);

	UINT uiCount;
	TTreeMap::const_iterator itor = m_pMainTreeMap.begin();

	while (itor != m_pMainTreeMap.end())
	{
		SpeedTreeWrapperPtr pMainTree = (itor++)->second;
		std::list<CSpeedTreeForest::SpeedTreeWrapperWeakPtr> ppInstances = pMainTree->GetInstances(uiCount);

		for (auto it : ppInstances)
		{
			if (it.expired())
				continue;

			auto pInstance = it.lock();
			if (!pInstance->isShow())
				continue;


			pInstance->Advance();
		}
	}

	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_Light, m_afLighting, 3);
	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_Fog, m_afFog, 1);

	if (ulRenderBitVector & Forest_RenderToShadow)
	{
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	}
	else
	{
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		STATEMANAGER.SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
		STATEMANAGER.SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
		STATEMANAGER.SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);

		#ifdef ENABLE_OPENGL
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
#else
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
#endif
	}

	STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_CW);

	// set up fog if it is enabled
	if (STATEMANAGER.GetRenderState(D3DRS_FOGENABLE))
	{
#ifdef JTX_GPU_TREE_WIND
		STATEMANAGER.SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE); // GPU needs to work on all cards
#endif
	}

	STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);


	if (ulRenderBitVector & Forest_RenderLeaves)
	{
		STATEMANAGER.SetFVF(D3DFVF_SPEEDTREE_LEAF_VERTEX);
		STATEMANAGER.SetVertexShader(m_dwLeafVertexShader);

		if (STATEMANAGER.GetRenderState(D3DRS_FOGENABLE)) {}
		if (ulRenderBitVector & Forest_RenderToShadow || ulRenderBitVector & Forest_RenderToMiniMap)
		{
			STATEMANAGER.SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL);
			STATEMANAGER.SaveRenderState(D3DRS_ALPHAREF, 0x00000000);
		}

		itor = m_pMainTreeMap.begin();

		while (itor != m_pMainTreeMap.end())
		{
			SpeedTreeWrapperPtr pMainTree = (itor++)->second;
			std::list<CSpeedTreeForest::SpeedTreeWrapperWeakPtr> ppInstances = pMainTree->GetInstances(uiCount);

			pMainTree->SetupLeafForTreeType();

			for (auto it : ppInstances)
			{
				if (it.expired())
					continue;

				auto pInstance = it.lock();
				if (!pInstance->isShow())
					continue;


				pInstance->RenderLeaves();
			}
		}

		if (ulRenderBitVector & Forest_RenderToShadow || ulRenderBitVector & Forest_RenderToMiniMap)
		{
			STATEMANAGER.SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
			STATEMANAGER.RestoreRenderState(D3DRS_ALPHAREF);
		}
	}

	if (ulRenderBitVector & Forest_RenderFronds)
	{
#ifdef __ANDROID__
		STATEMANAGER.SetFVF(D3DFVF_SPEEDTREE_BRANCH_VERTEX);
#endif
		itor = m_pMainTreeMap.begin();

		while (itor != m_pMainTreeMap.end())
		{
			SpeedTreeWrapperPtr pMainTree = (itor++)->second;
			std::list<CSpeedTreeForest::SpeedTreeWrapperWeakPtr> ppInstances = pMainTree->GetInstances(uiCount);

			pMainTree->SetupFrondForTreeType();

			for (auto it : ppInstances)
			{
				if (it.expired())
					continue;

				auto pInstance = it.lock();
				if (!pInstance->isShow())
					continue;

				pInstance->RenderFronds();
			}
		}
	}

	STATEMANAGER.SetFVF(D3DFVF_SPEEDTREE_BRANCH_VERTEX);
#ifndef __ANDROID__
	STATEMANAGER.SetVertexShader(m_dwBranchVertexShader);
#endif

	if (ulRenderBitVector & Forest_RenderBranches)
	{
		itor = m_pMainTreeMap.begin();

		while (itor != m_pMainTreeMap.end())
		{
			SpeedTreeWrapperPtr pMainTree = (itor++)->second;
			std::list<CSpeedTreeForest::SpeedTreeWrapperWeakPtr> ppInstances = pMainTree->GetInstances(uiCount);

			pMainTree->SetupBranchForTreeType();

			for (auto it : ppInstances)
			{
				if (it.expired())
					continue;

				auto pInstance = it.lock();
				if (!pInstance->isShow())
					continue;

#ifdef __ANDROID__
				static int s_iFirstTreePosLog = 0;
				if (++s_iFirstTreePosLog <= 3) {
					const float* tp = pInstance->GetPosition();
					D3DXVECTOR3 eye = CCameraManager::Instance().GetCurrentCamera()->GetEye();
					Tracenf("[TreePos] inst=(%.1f, %.1f, %.1f) eye=(%.1f, %.1f, %.1f)",
						tp[0], tp[1], tp[2], eye.x, eye.y, eye.z);
				}
#endif
				pInstance->RenderBranches();
			}
		}
#ifdef __ANDROID__
		STATEMANAGER.SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
#endif
	}

	STATEMANAGER.SetFVF(D3DFVF_SPEEDTREE_BRANCH_VERTEX);
	STATEMANAGER.SetVertexShader(m_dwBranchVertexShader);

	if (ulRenderBitVector & Forest_RenderBillboards)
	{
		STATEMANAGER.SetVertexShader((LPDIRECT3DVERTEXSHADER9)NULL); // Updated cast
		STATEMANAGER.SetFVF(D3DFVF_SPEEDTREE_BILLBOARD_VERTEX);
		STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
		STATEMANAGER.SetRenderState(D3DRS_COLORVERTEX, FALSE);

		itor = m_pMainTreeMap.begin();

		while (itor != m_pMainTreeMap.end())
		{
			SpeedTreeWrapperPtr pMainTree = (itor++)->second;
			std::list<CSpeedTreeForest::SpeedTreeWrapperWeakPtr> ppInstances = pMainTree->GetInstances(uiCount);

			pMainTree->SetupBranchForTreeType();

			for (auto it : ppInstances)
			{
				if (it.expired())
					continue;

				auto pInstance = it.lock();
				if (!pInstance->isShow())
					continue;


				pInstance->RenderBillboards();
			}
		}
	}

	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, dwLightState);
	STATEMANAGER.SetRenderState(D3DRS_COLORVERTEX, dwColorVertexState);
	STATEMANAGER.SetRenderState(D3DRS_FOGVERTEXMODE, dwFogVertexMode);

	if (!(ulRenderBitVector & Forest_RenderToShadow))
	{
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	}

	STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
}

void CSpeedTreeForestDirectX8::UpdateCompundMatrix2(const D3DXVECTOR3& c_rEyeVec, const D3DXMATRIX& c_rmatView, const D3DXMATRIX& c_rmatProj)
{
	// setup composite matrix for shader
	D3DXMATRIX matBlend;
	D3DXMatrixIdentity(&matBlend);

	D3DXMATRIX matBlendShader;
	D3DXMatrixMultiply(&matBlendShader, &c_rmatView, &c_rmatProj);

	float afDirection[3];
	//afDirection[0] = matBlendShader.m[0][2];
	//afDirection[1] = matBlendShader.m[1][2];
	//afDirection[2] = matBlendShader.m[2][2];
	//CSpeedTreeRT::SetCamera(c_rEyeVec, afDirection);

	D3DXMatrixTranspose(&matBlendShader, &matBlendShader);
	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_CompoundMatrix, &matBlendShader, 4);
}

void CSpeedTreeForestDirectX8::Render2(unsigned long ulRenderBitVector)
{
#ifdef __ANDROID__
	if (Frustum::Instance().getLeaves < 1.0f)
		Frustum::Instance().getLeaves = 1.0f;
	Render(ulRenderBitVector | Forest_RenderBranches | Forest_RenderFronds | Forest_RenderLeaves);
	return;
#endif
	if (m_pMainTreeMap.empty())
		return;


	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);


	TTreeMap::iterator itor;
	UINT uiCount;



	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_Light, m_afLighting, 3);
	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_Fog, m_afFog, 1);


	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);


	STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	// set up fog if it is enabled
	if (STATEMANAGER.GetRenderState(D3DRS_FOGENABLE))
	{
#ifdef WRAPPER_USE_GPU_WIND
		STATEMANAGER.SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_NONE); // GPU needs to work on all cards
#endif
	}

	itor = m_pMainTreeMap.begin();

	// Get the position of the character
	while (itor != m_pMainTreeMap.end())
	{

		SpeedTreeWrapperPtr pMainTree = (itor++)->second;
		std::list<CSpeedTreeForest::SpeedTreeWrapperWeakPtr> ppInstances = pMainTree->GetInstances(uiCount);


		STATEMANAGER.SetFVF(D3DFVF_SPEEDTREE_BRANCH_VERTEX);
		STATEMANAGER.SetVertexShader(m_dwBranchVertexShader);

		pMainTree->SetupBranchForTreeTypeShadow();

		for (auto it : ppInstances)
		{
			if (it.expired())
				continue;

			auto pInstance = it.lock();
			if (!pInstance->isShow())
				continue;


			pInstance->RenderBranchesShadow();
		}

		if (std::strcmp(Frustum::Instance().currentMapNameSave.c_str(), "metin2_map_dawnmist_dungeon_01") != 0)
		{
			pMainTree->SetupFrondForTreeType();


			for (auto it : ppInstances)
			{
				if (it.expired())
					continue;

				auto pInstance = it.lock();
				if (!pInstance->isShow())
					continue;


				pInstance->RenderFronds();
			}

		}

		STATEMANAGER.SetFVF(D3DFVF_SPEEDTREE_LEAF_VERTEX);
		STATEMANAGER.SetVertexShader(m_dwLeafVertexShader);

		if (!Forest_RenderFakeShadow)
		{
			pMainTree->SetupLeafForTreeType();
			for (auto it : ppInstances)
			{
				if (it.expired())
					continue;

				auto pInstance = it.lock();
				if (!pInstance->isShow())
					continue;


				pInstance->RenderLeaves();
			}
		}
	}

	// render billboards
	STATEMANAGER.SetVertexShader((LPDIRECT3DVERTEXSHADER9)NULL); // Updated cast
	//STATEMANAGER.SetFVF(D3DFVF_SPEEDTREE_BILLBOARD_VERTEX);

	// ����������� ���� TextureStage 1�� COLOROP�� ALPHAOP�� ����� ���� ������ �� �����
	// ����� ���´�. (�ȱ׷��� �˰� ���� ���ɼ���..)

	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
}

void CSpeedTreeForestDirectX8::Render3(unsigned long ulRenderBitVector)
{
	UpdateSystem(CTimer::Instance().GetCurrentSecond());

	if (m_pMainTreeMap.empty())
		return;


	TTreeMap::iterator itor;
	UINT uiCount;


	itor = m_pMainTreeMap.begin();

	// Get the position of the character
	while (itor != m_pMainTreeMap.end())
	{
		SpeedTreeWrapperPtr pMainTree = (itor++)->second;
		std::list<CSpeedTreeForest::SpeedTreeWrapperWeakPtr> ppInstances = pMainTree->GetInstances(uiCount);


		STATEMANAGER.SetFVF(D3DFVF_SPEEDTREE_LEAF_VERTEX);
		STATEMANAGER.SetVertexShader(m_dwLeafVertexShader);


		pMainTree->SetupLeafForTreeType();
		for (auto it : ppInstances)
		{
			if (it.expired())
				continue;

			auto pInstance = it.lock();
			if (!pInstance->isShow())
				continue;


			pInstance->RenderLeaves2();
		}
	}

	STATEMANAGER.SetVertexShader((LPDIRECT3DVERTEXSHADER9)NULL);
}
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
