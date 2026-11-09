#include "StdAfx.h"
#include <stdlib.h>
#include <stdio.h>
#include "../eterBase/Debug.h"
#include "../eterBase/Timer.h"
#include "../eterLib/ResourceManager.h"
#include "../eterPack/EterPackManager.h"
#include "../eterLib/Camera.h"
#include "../eterLib/StateManager.h"

#include "../GameLib/StdAfx.h"
#include "../GameLib/MapManager.h"
#include "../GameLib/FlyingObjectManager.h"

#include "SpeedTreeConfig.h"
#include "SpeedTreeForestDirectX8.h"
#include "SpeedTreeWrapper.h"
#include "VertexShaders.h"

#include "../UserInterface/PythonBackground.h"


unsigned int                    CSpeedTreeWrapper::m_unNumWrappersActive = 0;
LPDIRECT3DVERTEXSHADER9 CSpeedTreeWrapper::ms_dwBranchVertexShader = 0;
LPDIRECT3DVERTEXSHADER9 CSpeedTreeWrapper::ms_dwLeafVertexShader = 0;
bool CSpeedTreeWrapper::ms_bSelfShadowOn = true;

#define AGBR2ARGB(dwColor) (dwColor & 0xff00ff00) + ((dwColor & 0x00ff0000) >> 16) + ((dwColor & 0x000000ff) << 16)
CSpeedTreeWrapper::CSpeedTreeWrapper() :
	m_pSpeedTree(new CSpeedTreeRT),
	m_bIsInstance(false),
	m_pInstanceOf(NULL),
	m_pGeometryCache(NULL),
	m_usNumLeafLods(0),
	m_pBranchIndexCounts(NULL),
	m_pBranchIndexBuffer(NULL),
	m_pBranchVertexBuffer(NULL),
	m_pFrondIndexCounts(NULL),
	m_pFrondIndexBuffers(NULL),
	m_pFrondVertexBuffer(NULL),
	m_pLeafVertexBuffer(NULL),
	m_pLeavesUpdatedByCpu(NULL),
	m_unBranchVertexCount(0),
	m_unFrondVertexCount(0),
	m_pTextureInfo(NULL)
{
	// set initial position
	m_afPos[0] = m_afPos[1] = m_afPos[2] = 0.0f;

	m_pSpeedTree->SetWindStrength(1.0f);
	m_pSpeedTree->SetLocalMatrices(0, 4);
}

void CSpeedTreeWrapper::SetVertexShaders(LPDIRECT3DVERTEXSHADER9 dwBranchVertexShader, LPDIRECT3DVERTEXSHADER9 dwLeafVertexShader)
{
	ms_dwBranchVertexShader = dwBranchVertexShader;
	ms_dwLeafVertexShader = dwLeafVertexShader;
}

void CSpeedTreeWrapper::OnRenderPCBlocker()
{
	// removed
}

void CSpeedTreeWrapper::OnRender()
{
	if (ms_dwBranchVertexShader == 0)
	{
		ms_dwBranchVertexShader = LoadBranchShader(ms_lpd3dDevice);
		//LogBox("Vertex Shader not assigned. You must call CSpeedTreeWrapper::SetVertexShader for this");
	}

	if (ms_dwLeafVertexShader == 0)
	{
		ms_dwLeafVertexShader = LoadLeafShader(ms_lpd3dDevice);
		//LogBox("Vertex Shader not assigned. You must call CSpeedTreeWrapper::SetVertexShader for this");
	}

	CSpeedTreeForestDirectX8::Instance().UpdateSystem(ELTimer_GetMSec() / 1000.0f);

	// �ϳ��� ������ �� ���� LOD ������� ����
	m_pSpeedTree->SetLodLevel(1.0f);
	//Advance();

	CSpeedTreeForestDirectX8::Instance().UpdateCompundMatrix(CCameraManager::Instance().GetCurrentCamera()->GetEye(), ms_matView, ms_matProj);

	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	STATEMANAGER.SaveRenderState(D3DRS_LIGHTING, FALSE);
	STATEMANAGER.SaveRenderState(D3DRS_COLORVERTEX, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	#if defined(__BL_FOG_FIX__)
	STATEMANAGER.SaveRenderState(D3DRS_FOGENABLE, CPythonBackground::Instance().GetFogMode());
#else
	STATEMANAGER.SaveRenderState(D3DRS_FOGENABLE, TRUE);
#endif

	// choose fixed function pipeline or custom shader for fronds and branches
	STATEMANAGER.SetVertexShader(ms_dwBranchVertexShader);

	SetupBranchForTreeType();
	RenderBranches();

	STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());
	STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	SetupFrondForTreeType();
	RenderFronds();

	STATEMANAGER.SetVertexShader(ms_dwLeafVertexShader);

	SetupLeafForTreeType();
	RenderLeaves();
	EndLeafForTreeType();

	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
	STATEMANAGER.SetRenderState(D3DRS_COLORVERTEX, FALSE);
	RenderBillboards();

	STATEMANAGER.RestoreRenderState(D3DRS_LIGHTING);
	STATEMANAGER.RestoreRenderState(D3DRS_COLORVERTEX);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
	STATEMANAGER.RestoreRenderState(D3DRS_FOGENABLE);
}

UINT CSpeedTreeWrapper::GetCollisionObjectCount()
{
	assert(m_pSpeedTree);
	return m_pSpeedTree->GetCollisionObjectCount();
}

void CSpeedTreeWrapper::GetCollisionObject(UINT nIndex, CSpeedTreeRT::ECollisionObjectType& eType, float* pPosition, float* pDimensions)
{
	assert(m_pSpeedTree);
	m_pSpeedTree->GetCollisionObject(nIndex, eType, pPosition, pDimensions);
}

void CSpeedTreeWrapper::OnUpdateCollisionData(const CStaticCollisionDataVector*)
{
	D3DXMATRIX mat;
	D3DXMatrixTranslation(&mat, m_afPos[0], m_afPos[1], m_afPos[2]);
	for (UINT i = 0; i < GetCollisionObjectCount(); ++i)
	{
		CSpeedTreeRT::ECollisionObjectType ObjectType;
		CStaticCollisionData CollisionData;

		GetCollisionObject(i, ObjectType, (float*)&CollisionData.v3Position, CollisionData.fDimensions);

		if (ObjectType == CSpeedTreeRT::CO_BOX)
			continue;

		switch (ObjectType)
		{
		case CSpeedTreeRT::CO_SPHERE:
			CollisionData.dwType = COLLISION_TYPE_SPHERE;
			CollisionData.fDimensions[0] = CollisionData.fDimensions[0];
			break;

		case CSpeedTreeRT::CO_CYLINDER:
			CollisionData.dwType = COLLISION_TYPE_CYLINDER;
			CollisionData.fDimensions[0] = CollisionData.fDimensions[0];
			CollisionData.fDimensions[1] = CollisionData.fDimensions[1];
			break;
		}
		AddCollision(&CollisionData, &mat);
	}
}

bool CSpeedTreeWrapper::GetBoundingSphere(D3DXVECTOR3& v3Center, float& fRadius)
{
	float fX, fY, fZ;
	fX = m_afBoundingBox[3] - m_afBoundingBox[0];
	fY = m_afBoundingBox[4] - m_afBoundingBox[1];
	fZ = m_afBoundingBox[5] - m_afBoundingBox[2];
	v3Center.x = 0.0f;
	v3Center.y = 0.0f;
	v3Center.z = fZ * 0.5f;
	fRadius = sqrtf(fX * fX + fY * fY + fZ * fZ) * 0.5f * 0.9f;
	D3DXVECTOR3 vec = m_pSpeedTree->GetTreePosition();
	v3Center += vec;
	return true;
}

void CSpeedTreeWrapper::CalculateBBox()
{
	float fX, fY, fZ;

	fX = m_afBoundingBox[3] - m_afBoundingBox[0];
	fY = m_afBoundingBox[4] - m_afBoundingBox[1];
	fZ = m_afBoundingBox[5] - m_afBoundingBox[2];

	m_v3BBoxMin.x = -fX / 2.0f;
	m_v3BBoxMin.y = -fY / 2.0f;
	m_v3BBoxMin.z = 0.0f;
	m_v3BBoxMax.x = fX / 2.0f;
	m_v3BBoxMax.y = fY / 2.0f;
	m_v3BBoxMax.z = fZ;

	m_v4TBBox[0] = D3DXVECTOR4(m_v3BBoxMin.x, m_v3BBoxMin.y, m_v3BBoxMin.z, 1.0f);
	m_v4TBBox[1] = D3DXVECTOR4(m_v3BBoxMin.x, m_v3BBoxMax.y, m_v3BBoxMin.z, 1.0f);
	m_v4TBBox[2] = D3DXVECTOR4(m_v3BBoxMax.x, m_v3BBoxMin.y, m_v3BBoxMin.z, 1.0f);
	m_v4TBBox[3] = D3DXVECTOR4(m_v3BBoxMax.x, m_v3BBoxMax.y, m_v3BBoxMin.z, 1.0f);
	m_v4TBBox[4] = D3DXVECTOR4(m_v3BBoxMin.x, m_v3BBoxMin.y, m_v3BBoxMax.z, 1.0f);
	m_v4TBBox[5] = D3DXVECTOR4(m_v3BBoxMin.x, m_v3BBoxMax.y, m_v3BBoxMax.z, 1.0f);
	m_v4TBBox[6] = D3DXVECTOR4(m_v3BBoxMax.x, m_v3BBoxMin.y, m_v3BBoxMax.z, 1.0f);
	m_v4TBBox[7] = D3DXVECTOR4(m_v3BBoxMax.x, m_v3BBoxMax.y, m_v3BBoxMax.z, 1.0f);

	const D3DXMATRIX& c_rmatTransform = GetTransform();

	for (DWORD i = 0; i < 8; ++i)
	{
		D3DXVec4Transform(&m_v4TBBox[i], &m_v4TBBox[i], &c_rmatTransform);
		if (0 == i)
		{
			m_v3TBBoxMin.x = m_v4TBBox[i].x;
			m_v3TBBoxMin.y = m_v4TBBox[i].y;
			m_v3TBBoxMin.z = m_v4TBBox[i].z;
			m_v3TBBoxMax.x = m_v4TBBox[i].x;
			m_v3TBBoxMax.y = m_v4TBBox[i].y;
			m_v3TBBoxMax.z = m_v4TBBox[i].z;
		}
		else
		{
			if (m_v3TBBoxMin.x > m_v4TBBox[i].x)
				m_v3TBBoxMin.x = m_v4TBBox[i].x;
			if (m_v3TBBoxMax.x < m_v4TBBox[i].x)
				m_v3TBBoxMax.x = m_v4TBBox[i].x;
			if (m_v3TBBoxMin.y > m_v4TBBox[i].y)
				m_v3TBBoxMin.y = m_v4TBBox[i].y;
			if (m_v3TBBoxMax.y < m_v4TBBox[i].y)
				m_v3TBBoxMax.y = m_v4TBBox[i].y;
			if (m_v3TBBoxMin.z > m_v4TBBox[i].z)
				m_v3TBBoxMin.z = m_v4TBBox[i].z;
			if (m_v3TBBoxMax.z < m_v4TBBox[i].z)
				m_v3TBBoxMax.z = m_v4TBBox[i].z;
		}
	}
}

CSpeedTreeWrapper::~CSpeedTreeWrapper()
{
	if (!m_bIsInstance)
	{
		if (m_unBranchVertexCount > 0)
		{
			SAFE_RELEASE(m_pBranchVertexBuffer);
			SAFE_RELEASE(m_pBranchIndexBuffer);
			SAFE_DELETE_ARRAY(m_pBranchIndexCounts);
		}

		if (m_unFrondVertexCount > 0)
		{
			SAFE_RELEASE(m_pFrondVertexBuffer);
			SAFE_RELEASE(m_pFrondIndexBuffers);
			SAFE_DELETE_ARRAY(m_pFrondIndexCounts);
		}

		for (unsigned short i = 0; i < m_usNumLeafLods; ++i)
		{
			m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_LeafGeometry, -1, -1, i);

			if (m_pGeometryCache->m_sLeaves0.m_usLeafCount > 0)
				SAFE_RELEASE(m_pLeafVertexBuffer[i]);
		}

		SAFE_DELETE_ARRAY(m_pLeavesUpdatedByCpu);
		SAFE_DELETE_ARRAY(m_pLeafVertexBuffer);

		SAFE_DELETE(m_pTextureInfo);
		SAFE_DELETE(m_pGeometryCache);
	}
	SAFE_DELETE(m_pSpeedTree);

	--m_unNumWrappersActive;


	Clear();
}

bool CSpeedTreeWrapper::LoadTree(const char* pszSptFile, const BYTE* c_pbBlock = NULL, unsigned int uiBlockSize = 0, unsigned int nSeed, float fSize, float fSizeVariance)
{
	bool bSuccess = false;

#ifdef WRAPPER_FLIP_T_TEXCOORD
	m_pSpeedTree->SetTextureFlip(true);
#endif

	if (!m_pSpeedTree->LoadTree(c_pbBlock, uiBlockSize))
	{
		if (!m_pSpeedTree->LoadTree(pszSptFile))
		{
			TraceError("SpeedTreeRT Error: %s", CSpeedTreeRT::GetCurrentError());
			return false;
		}
	}

#ifdef JTX_TREE_DYNAMIC_LIGHT
	m_pSpeedTree->SetBranchLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);
	m_pSpeedTree->SetLeafLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);
	m_pSpeedTree->SetFrondLightingMethod(CSpeedTreeRT::LIGHT_DYNAMIC);
#else
	m_pSpeedTree->SetBranchLightingMethod(CSpeedTreeRT::LIGHT_STATIC);
	m_pSpeedTree->SetLeafLightingMethod(CSpeedTreeRT::LIGHT_STATIC);
	m_pSpeedTree->SetFrondLightingMethod(CSpeedTreeRT::LIGHT_STATIC);
#endif

#ifdef JTX_GPU_TREE_WIND
	m_pSpeedTree->SetBranchWindMethod(CSpeedTreeRT::WIND_GPU);
	m_pSpeedTree->SetLeafWindMethod(CSpeedTreeRT::WIND_GPU);
	m_pSpeedTree->SetFrondWindMethod(CSpeedTreeRT::WIND_GPU);
#else
	m_pSpeedTree->SetBranchWindMethod(CSpeedTreeRT::WIND_NONE);
	m_pSpeedTree->SetLeafWindMethod(CSpeedTreeRT::WIND_NONE);
	m_pSpeedTree->SetFrondWindMethod(CSpeedTreeRT::WIND_NONE);
#endif

	m_pSpeedTree->SetNumLeafRockingGroups(1);

	if (fSize >= 0.0f && fSizeVariance >= 0.0f)
		m_pSpeedTree->SetTreeSize(fSize, fSizeVariance);

	if (m_pSpeedTree->Compute(NULL, nSeed))
	{
		m_pSpeedTree->GetBoundingBox(m_afBoundingBox);
		m_pSpeedTree->SetLeafRockingState(true);
		CSpeedTreeRT::SetDropToBillboard(true);
		m_cBranchMaterial.Set(m_pSpeedTree->GetBranchMaterial());
		m_cFrondMaterial.Set(m_pSpeedTree->GetFrondMaterial());
		m_cLeafMaterial.Set(m_pSpeedTree->GetLeafMaterial());
		float fHeight = m_afBoundingBox[5] - m_afBoundingBox[2];
		m_pTextureInfo = new CSpeedTreeRT::STextures;
		m_pSpeedTree->GetTextures(*m_pTextureInfo);
#ifdef __ANDROID__
		std::string strSptPath = pszSptFile ? pszSptFile : "";
		CFileNameHelper::StringPath(strSptPath);
		std::string strTreeDir = CFileNameHelper::GetPath(strSptPath);
		auto LoadTreeTex = [&](const char* szRawName, CGraphicImageInstance& rImg) {
			if (!szRawName || !*szRawName) return;
			std::string stem(szRawName);
			size_t slash = stem.find_last_of("/\\");
			if (slash != std::string::npos)
				stem = stem.substr(slash + 1);
			size_t dot = stem.find_last_of('.');
			if (dot != std::string::npos)
				stem = stem.substr(0, dot);
			for (char& c : stem)
				c = (char)tolower((unsigned char)c);
			if (stem.empty()) return;

			std::string base = stem + ".dds";
			std::string candidates[4] = {
				strTreeDir + base,
				"d:/ymir work/tree/" + base,
				"d:/ymir work/zone/b/tree/1/" + base,
				"d:/ymir work/zone/b/tree/c/" + base
			};
			for (int i = 0; i < 4; ++i) {
				if (CEterPackManager::Instance().isExist(candidates[i].c_str())) {
					LoadTexture(candidates[i].c_str(), rImg);
					return;
				}
			}
			LoadTexture(candidates[0].c_str(), rImg);
		};
		LoadTreeTex(m_pTextureInfo->m_pBranchTextureFilename, m_BranchImageInstance);

		if (m_pTextureInfo->m_pSelfShadowFilename != NULL)
			LoadTreeTex(m_pTextureInfo->m_pSelfShadowFilename, m_ShadowImageInstance);

		if (m_pTextureInfo->m_pCompositeFilename)
			LoadTreeTex(m_pTextureInfo->m_pCompositeFilename, m_CompositeImageInstance);
#else
		LoadTexture((CFileNameHelper::GetPath(std::string(pszSptFile)) + CFileNameHelper::NoExtension(std::string(m_pTextureInfo->m_pBranchTextureFilename)) + ".dds").c_str(), m_BranchImageInstance);

		if (m_pTextureInfo->m_pSelfShadowFilename != NULL)
			LoadTexture((CFileNameHelper::GetPath(std::string(pszSptFile)) + CFileNameHelper::NoExtension(std::string(m_pTextureInfo->m_pSelfShadowFilename)) + ".dds").c_str(), m_ShadowImageInstance);

		if (m_pTextureInfo->m_pCompositeFilename)
			LoadTexture((CFileNameHelper::GetPath(std::string(pszSptFile)) + CFileNameHelper::NoExtension(std::string(m_pTextureInfo->m_pCompositeFilename)) + ".dds").c_str(), m_CompositeImageInstance);
#endif
		SetupBuffers();
		bSuccess = true;
	}
	else
		fprintf(stderr, "\nFatal Error, cannot compute tree [%s]\n\n", CSpeedTreeRT::GetCurrentError());

	return bSuccess;
}

void CSpeedTreeWrapper::SetupBuffers(void)
{
	if (m_pGeometryCache == NULL)
		m_pGeometryCache = new CSpeedTreeRT::SGeometry;
	m_pSpeedTree->GetGeometry(*m_pGeometryCache);
	SetupBranchBuffers();
	SetupFrondBuffers();
	SetupLeafBuffers();
}

void CSpeedTreeWrapper::SetupBranchBuffers(void)
{
	// reference to branch structure
	CSpeedTreeRT::SGeometry::SIndexed* pBranches = &(m_pGeometryCache->m_sBranches);
	m_unBranchVertexCount = pBranches->m_usVertexCount; // we asked for a contiguous strip

	// check if this tree has branches
	if (m_unBranchVertexCount > 1)
	{
		// create the vertex buffer for storing branch vertices
		SFVFBranchVertex* pVertexBuffer = NULL;

#ifndef WRAPPER_USE_CPU_WIND
		ms_lpd3dDevice->CreateVertexBuffer(m_unBranchVertexCount * sizeof(SFVFBranchVertex), D3DUSAGE_WRITEONLY, D3DFVF_SPEEDTREE_BRANCH_VERTEX, D3DPOOL_MANAGED, &m_pBranchVertexBuffer, nullptr);
		// fill the vertex buffer by interleaving SpeedTree data
		m_pBranchVertexBuffer->Lock(0, 0, (void**)(&pVertexBuffer), 0);
#else
		ms_lpd3dDevice->CreateVertexBuffer(m_unBranchVertexCount * sizeof(SFVFBranchVertex), D3DUSAGE_DYNAMIC, D3DFVF_SPEEDTREE_BRANCH_VERTEX, D3DPOOL_SYSTEMMEM, &m_pBranchVertexBuffer);
		// fill the vertex buffer by interleaving SpeedTree data
		m_pBranchVertexBuffer->Lock(0, 0, reinterpret_cast<BYTE**>(&pVertexBuffer), D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
#endif
		{
			for (UINT i = 0; i < m_unBranchVertexCount; ++i)
			{
				// position
				memcpy(&pVertexBuffer->m_vPosition, &(pBranches->m_pCoords[i * 3]), 3 * sizeof(float));

				// normal or color
#ifdef JTX_TREE_DYNAMIC_LIGHT
				memcpy(&pVertexBuffer->m_vNormal, &(pBranches->m_pNormals[i * 3]), 3 * sizeof(float));
#else
				pVertexBuffer->m_dwDiffuseColor = pBranches->m_pColors ? pBranches->m_pColors[i] : 0xFFFFFFFF;
#endif

				// texcoords for layer 0
				pVertexBuffer->m_fTexCoords[0] = pBranches->m_pTexCoords0[i * 2];
				pVertexBuffer->m_fTexCoords[1] = pBranches->m_pTexCoords0[i * 2 + 1];

				// texcoords for layer 1 (if enabled)
#ifdef WRAPPER_RENDER_SELF_SHADOWS

#endif

				// extra data for gpu wind
#ifdef JTX_GPU_TREE_WIND
				pVertexBuffer->m_fWindIndex = 4.0f * pBranches->m_pWindMatrixIndices[i];
				pVertexBuffer->m_fWindWeight = pBranches->m_pWindWeights[i];
#endif

				++pVertexBuffer;
			}
			m_pBranchVertexBuffer->Unlock();
		}

		// create and fill the index counts for each LOD
		UINT unNumLodLevels = m_pSpeedTree->GetNumBranchLodLevels();
		m_pBranchIndexCounts = new unsigned short[unNumLodLevels];
		for (UINT i = 0; i < unNumLodLevels; ++i)
		{
			// force update for particular LOD
			m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry, i);

			// check if this LOD has branches
			if (pBranches->m_usNumStrips > 0)
				m_pBranchIndexCounts[i] = pBranches->m_pStripLengths[0];
			else
				m_pBranchIndexCounts[i] = 0;
		}
		// set back to highest LOD
		m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry, 0);

		// the first LOD level contains the most indices of all the levels, so
		// we use its size to allocate the index buffer
		ms_lpd3dDevice->CreateIndexBuffer(m_pBranchIndexCounts[0] * sizeof(unsigned short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pBranchIndexBuffer, nullptr);

		// fill the index buffer
		unsigned short* pIndexBuffer = NULL;
		m_pBranchIndexBuffer->Lock(0, 0, (void**)(&pIndexBuffer), 0);
		memcpy(pIndexBuffer, pBranches->m_pStrips[0], pBranches->m_pStripLengths[0] * sizeof(unsigned short));
		m_pBranchIndexBuffer->Unlock();
	}
}

void CSpeedTreeWrapper::SetupFrondBuffers(void)
{
	// reference to frond structure
	CSpeedTreeRT::SGeometry::SIndexed* pFronds = &(m_pGeometryCache->m_sFronds);
	m_unFrondVertexCount = pFronds->m_usVertexCount; // we asked for a contiguous strip

	// check if tree has fronds
	if (m_unFrondVertexCount > 1)
	{
		// create the vertex buffer for storing frond vertices
		SFVFBranchVertex* pVertexBuffer = NULL;
#ifndef WRAPPER_USE_CPU_WIND
		ms_lpd3dDevice->CreateVertexBuffer(m_unFrondVertexCount * sizeof(SFVFBranchVertex), D3DUSAGE_WRITEONLY, D3DFVF_SPEEDTREE_BRANCH_VERTEX, D3DPOOL_MANAGED, &m_pFrondVertexBuffer, nullptr);
		// fill the vertex buffer by interleaving SpeedTree data
		m_pFrondVertexBuffer->Lock(0, 0, (void**)(&pVertexBuffer), 0);
#else
		ms_lpd3dDevice->CreateVertexBuffer(m_unFrondVertexCount * sizeof(SFVFBranchVertex), D3DUSAGE_DYNAMIC, D3DFVF_SPEEDTREE_BRANCH_VERTEX, D3DPOOL_SYSTEMMEM, &m_pFrondVertexBuffer);
		// fill the vertex buffer by interleaving SpeedTree data
		m_pFrondVertexBuffer->Lock(0, 0, reinterpret_cast<BYTE**>(&pVertexBuffer), D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
#endif
		for (UINT i = 0; i < m_unFrondVertexCount; ++i)
		{
			// position
			memcpy(&pVertexBuffer->m_vPosition, &(pFronds->m_pCoords[i * 3]), 3 * sizeof(float));

			// normal or color
#ifdef JTX_TREE_DYNAMIC_LIGHT
			memcpy(&pVertexBuffer->m_vNormal, &(pFronds->m_pNormals[i * 3]), 3 * sizeof(float));
#else
			pVertexBuffer->m_dwDiffuseColor = pFronds->m_pColors ? pFronds->m_pColors[i] : 0xFFFFFFFF;
#endif

			// texcoords for layer 0
			pVertexBuffer->m_fTexCoords[0] = pFronds->m_pTexCoords0[i * 2];
			pVertexBuffer->m_fTexCoords[1] = pFronds->m_pTexCoords0[i * 2 + 1];

			// texcoords for layer 1 (if enabled)
#ifdef WRAPPER_RENDER_SELF_SHADOWS

#endif

			// extra data for gpu wind
#ifdef JTX_GPU_TREE_WIND
			pVertexBuffer->m_fWindIndex = 4.0f * pFronds->m_pWindMatrixIndices[i];
			pVertexBuffer->m_fWindWeight = pFronds->m_pWindWeights[i];
#endif

			++pVertexBuffer;
		}
		m_pFrondVertexBuffer->Unlock();

		// create and fill the index counts for each LOD
		UINT unNumLodLevels = m_pSpeedTree->GetNumFrondLodLevels();
		m_pFrondIndexCounts = new unsigned short[unNumLodLevels];
		for (WORD j = 0; j < unNumLodLevels; ++j)
		{
			// force update for this LOD
			m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry, -1, j);

			// check if this LOD has fronds
			if (pFronds->m_usNumStrips > 0)
				m_pFrondIndexCounts[j] = pFronds->m_pStripLengths[0];
			else
				m_pFrondIndexCounts[j] = 0;
		}
		// go back to highest LOD
		m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry, -1, 0);

		// the first LOD level contains the most indices of all the levels, so
		// we use its size to allocate the index buffer
		ms_lpd3dDevice->CreateIndexBuffer(m_pFrondIndexCounts[0] * sizeof(unsigned short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pFrondIndexBuffers, nullptr);

		// fill the index buffer
		unsigned short* pIndexBuffer = NULL;
		m_pFrondIndexBuffers->Lock(0, 0, (void**)(&pIndexBuffer), 0);
		memcpy(pIndexBuffer, pFronds->m_pStrips[0], pFronds->m_pStripLengths[0] * sizeof(unsigned short));
		m_pFrondIndexBuffers->Unlock();
	}
}

void CSpeedTreeWrapper::SetupLeafBuffers(void)
{
	// set up constants
	const short anVertexIndices[6] = { 0, 1, 2, 0, 2, 3 };
	//const int nNumLeafMaps = m_pTextureInfo->m_uiLeafTextureCount;

	// set up the leaf counts for each LOD
	m_usNumLeafLods = m_pSpeedTree->GetNumLeafLodLevels();

	// create array of vertex buffers (one for each LOD)
	m_pLeafVertexBuffer = new LPDIRECT3DVERTEXBUFFER9[m_usNumLeafLods];

	// create array of bools for CPU updating (so we don't update for each instance)
	m_pLeavesUpdatedByCpu = new bool[m_usNumLeafLods];

	// cycle through LODs
	for (UINT unLod = 0; unLod < m_usNumLeafLods; ++unLod)
	{
		m_pLeavesUpdatedByCpu[unLod] = false;
		m_pLeafVertexBuffer[unLod] = NULL;

		// if this LOD has no leaves, skip it
		unsigned short usLeafCount = m_pGeometryCache->m_sLeaves0.m_usLeafCount;

		if (usLeafCount < 1)
			continue;

		SFVFLeafVertex* pVertexBuffer = NULL;
		// create the vertex buffer for storing leaf vertices
#ifndef WRAPPER_USE_CPU_LEAF_PLACEMENT
		ms_lpd3dDevice->CreateVertexBuffer(usLeafCount * 6 * sizeof(SFVFLeafVertex), D3DUSAGE_WRITEONLY, D3DFVF_SPEEDTREE_LEAF_VERTEX, D3DPOOL_MANAGED, &m_pLeafVertexBuffer[unLod], nullptr);
		// fill the vertex buffer by interleaving SpeedTree data
		m_pLeafVertexBuffer[unLod]->Lock(0, 0, (void**)(&pVertexBuffer), 0);
#else
		ms_lpd3dDevice->CreateVertexBuffer(usLeafCount * 6 * sizeof(SFVFLeafVertex), D3DUSAGE_DYNAMIC, D3DFVF_SPEEDTREE_LEAF_VERTEX, D3DPOOL_SYSTEMMEM, &m_pLeafVertexBuffer[unLod], nullptr);
		// fill the vertex buffer by interleaving SpeedTree data
		m_pLeafVertexBuffer[unLod]->Lock(0, 0, DXLOCKTYPE(&pVertexBuffer), D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
#endif
		SFVFLeafVertex* pVertex = pVertexBuffer;
		for (UINT unLeaf = 0; unLeaf < usLeafCount; ++unLeaf)
		{
			const CSpeedTreeRT::SGeometry::SLeaf* pLeaf = &(m_pGeometryCache->m_sLeaves0);
			for (UINT unVert = 0; unVert < 6; ++unVert)  // 6 verts == 2 triangles
			{
				// position
				memcpy(pVertex->m_vPosition, &(pLeaf->m_pCenterCoords[unLeaf * 3]), 3 * sizeof(float));

#ifdef JTX_TREE_DYNAMIC_LIGHT
				// normal
				memcpy(&pVertex->m_vNormal, &(pLeaf->m_pNormals[unLeaf * 3]), 3 * sizeof(float));
#else
				// color
				pVertex->m_dwDiffuseColor = pLeaf->m_pColors ? pLeaf->m_pColors[unLeaf] : 0xFFFFFFFF;
#endif

				// tex coord
				memcpy(pVertex->m_fTexCoords, &(pLeaf->m_pLeafMapTexCoords[unLeaf][anVertexIndices[unVert] * 2]), 2 * sizeof(float));

				// wind weights
#ifdef JTX_GPU_TREE_WIND
				pVertex->m_fWindIndex = 4.0f * pLeaf->m_pWindMatrixIndices[unLeaf];
				pVertex->m_fWindWeight = pLeaf->m_pWindWeights[unLeaf];
#endif

				// GPU placement data
#ifdef JTX_GPU_LEAF_PLACEMENT
				pVertex->m_fLeafPlacementIndex = c_nVertexShader_LeafTables + pLeaf->m_pLeafClusterIndices[unLeaf] * 4.0f + anVertexIndices[unVert];
				pVertex->m_fLeafScalarValue = m_pSpeedTree->GetLeafLodSizeAdjustments()[unLod];
#endif

				++pVertex;
			}
		}
		m_pLeafVertexBuffer[unLod]->Unlock();
	}
}

void CSpeedTreeWrapper::Advance(void) {}

CSpeedTreeWrapper::SpeedTreeWrapperPtr CSpeedTreeWrapper::MakeInstance()
{
	std::shared_ptr<CSpeedTreeWrapper> pInstance = std::make_shared<CSpeedTreeWrapper>();

	pInstance->m_pSpeedTree->~CSpeedTreeRT();

	// make an instance of this object's SpeedTree
	pInstance->m_bIsInstance = true;
	pInstance->m_pSpeedTree = m_pSpeedTree->MakeInstance();
	if (pInstance->m_pSpeedTree)
	{
		// use the same materials
		pInstance->m_cBranchMaterial = m_cBranchMaterial;
		pInstance->m_cLeafMaterial = m_cLeafMaterial;
		pInstance->m_cFrondMaterial = m_cFrondMaterial;
		pInstance->m_CompositeImageInstance.SetImagePointer(m_CompositeImageInstance.GetGraphicImagePointer());
		pInstance->m_BranchImageInstance.SetImagePointer(m_BranchImageInstance.GetGraphicImagePointer());
		if (!m_ShadowImageInstance.IsEmpty())
			pInstance->m_ShadowImageInstance.SetImagePointer(m_ShadowImageInstance.GetGraphicImagePointer());
		pInstance->m_pTextureInfo = m_pTextureInfo;

		// use the same geometry cache
		pInstance->m_pGeometryCache = m_pGeometryCache;

		// use the same buffers
		pInstance->m_pBranchIndexBuffer = m_pBranchIndexBuffer;
		pInstance->m_pBranchIndexCounts = m_pBranchIndexCounts;
		pInstance->m_pBranchVertexBuffer = m_pBranchVertexBuffer;
		pInstance->m_unBranchVertexCount = m_unBranchVertexCount;
		pInstance->m_pFrondIndexBuffers = m_pFrondIndexBuffers;
		pInstance->m_pFrondIndexCounts = m_pFrondIndexCounts;
		pInstance->m_pFrondVertexBuffer = m_pFrondVertexBuffer;
		pInstance->m_unFrondVertexCount = m_unFrondVertexCount;
		pInstance->m_pLeafVertexBuffer = m_pLeafVertexBuffer;
		pInstance->m_usNumLeafLods = m_usNumLeafLods;
		pInstance->m_pLeavesUpdatedByCpu = m_pLeavesUpdatedByCpu;

		// new stuff
		memcpy(pInstance->m_afPos, m_afPos, 3 * sizeof(float));
		memcpy(pInstance->m_afBoundingBox, m_afBoundingBox, 6 * sizeof(float));
		pInstance->m_pInstanceOf = shared_from_this();
		m_vInstances.push_back(pInstance);
	}
	else
	{
		fprintf(stderr, "SpeedTreeRT Error: %s\n", m_pSpeedTree->GetCurrentError());
		pInstance.reset();
	}

	return pInstance;
}

std::list<CSpeedTreeWrapper::SpeedTreeWrapperWeakPtr> CSpeedTreeWrapper::GetInstances(UINT& nCount)
{
	std::list<SpeedTreeWrapperWeakPtr> kResult;

	nCount = m_vInstances.size();
	if (nCount)
	{
		for (auto it : m_vInstances)
		{
			kResult.push_back(it);
		}
	}

	return kResult;
}

void CSpeedTreeWrapper::DeleteInstance(SpeedTreeWrapperPtr pInstance)
{
	std::vector<SpeedTreeWrapperPtr>::iterator itor = m_vInstances.begin();

	while (itor != m_vInstances.end())
	{
		if (*itor == pInstance)
		{
			itor = m_vInstances.erase(itor);
		}
		else
		{
			++itor;
		}
	}
}

D3DMATERIAL9 currentMaterial;
void CSpeedTreeWrapper::SetupBranchForTreeType(void)
{
	STATEMANAGER.GetMaterial(&currentMaterial);
#ifdef JTX_TREE_DYNAMIC_LIGHT
	// set lighting material
	STATEMANAGER.SetMaterial(m_cBranchMaterial.Get());
	SetShaderConstants(m_pSpeedTree->GetBranchMaterial());
#endif

	// update the branch geometry for CPU wind
#ifdef WRAPPER_USE_CPU_WIND
	m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry);

	if (m_pGeometryCache->m_sBranches.m_usNumStrips > 0)
	{
		// update the vertex array
		SFVFBranchVertex* pVertexBuffer = NULL;
		m_pBranchVertexBuffer->Lock(0, 0, reinterpret_cast<BYTE**>(&pVertexBuffer), D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
		for (UINT i = 0; i < m_unBranchVertexCount; ++i)
		{
			memcpy(&(pVertexBuffer[i].m_vPosition), &(m_pGeometryCache->m_sBranches.m_pCoords[i * 3]), 3 * sizeof(float));
		}
		m_pBranchVertexBuffer->Unlock();
	}
#endif

	LPDIRECT3DTEXTURE9 lpd3dTexture;

	// set texture map
	if ((lpd3dTexture = m_BranchImageInstance.GetTextureReference().GetD3DTexture()))
		STATEMANAGER.SetTexture(0, lpd3dTexture);

	// bind shadow texture
#ifdef WRAPPER_RENDER_SELF_SHADOWS
	STATEMANAGER.SetTexture(1, NULL);
#endif

	if (m_pGeometryCache->m_sBranches.m_usVertexCount > 0)
	{
#ifdef __ANDROID__
		STATEMANAGER.SetFVF(D3DFVF_SPEEDTREE_BRANCH_VERTEX);
#endif
		// activate the branch vertex buffer
		STATEMANAGER.SetStreamSource(0, m_pBranchVertexBuffer, sizeof(SFVFBranchVertex));
		// set the index buffer
		STATEMANAGER.SetIndices(m_pBranchIndexBuffer, 0);
	}

	STATEMANAGER.SetMaterial(&currentMaterial);
}

float timertree = 0.0f;
void CSpeedTreeWrapper::RenderBranches(void) const
{
#ifdef __ANDROID__
	if (false)
#else
	if (Frustum::Instance().graphicsLevel >= 2)
#endif
	{
		m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry);

		if (m_pGeometryCache->m_fBranchAlphaTestValue > 0.0f)
		{
			PositionTreeBranch();

			timertree += 0.000000060f;

			if (timertree > 10)
				timertree = 0.0f;

			auto* FlyManager = CFlyingManager::InstancePtr();
			auto* MapManager = FlyManager->GetMapManagerPtr();
			auto& OutDoor = MapManager->GetMapOutdoorRef();

			STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_fBranchAlphaTestValue));
			if (m_pBranchIndexCounts &&
				m_pGeometryCache->m_sBranches.m_nDiscreteLodLevel > -1 &&
				m_pBranchIndexCounts[m_pGeometryCache->m_sBranches.m_nDiscreteLodLevel] > 0)
			{
				if (Frustum::Instance().shadowType == 2)
				{
					if (FlyManager) {
						if (MapManager && MapManager->IsMapOutdoor() && MapManager->IsMapReady())
						{
							//HRESULT hr = D3DXSaveTextureToFile("depth.png", D3DXIFF_PNG, m_lpCharacterShadowMapTexture, nullptr);
							CCamera* pCurrentCamera = CCameraManager::Instance().GetCurrentCamera();
							STATEMANAGER.SetVertexShader(OutDoor.m_lpShadowMapVertexShaderTree);
							STATEMANAGER.SetPixelShader(OutDoor.m_lpShadowMapPixelShaderTree);


							// Vertex shader constants ----
							// Transformation matrix
							D3DXMATRIX worldMat;
							//STATEMANAGER.GetTransform(D3DTS_WORLD, &worldMat);
							ms_lpd3dDevice->GetTransform(D3DTS_WORLD, &worldMat);
							D3DXMatrixTranspose(&worldMat, &worldMat);
							//D3DXMatrixIdentity(&worldMat);
							ms_lpd3dDevice->SetVertexShaderConstantF(0, (const float*)(&worldMat), 4);
							// View-Projection
							D3DXMATRIX viewProjMat, viewMat, projMat;
							ms_lpd3dDevice->GetTransform(D3DTS_VIEW, &viewMat);
							ms_lpd3dDevice->GetTransform(D3DTS_PROJECTION, &projMat);
							D3DXMatrixMultiplyTranspose(&viewProjMat, &viewMat, &projMat);
							ms_lpd3dDevice->SetVertexShaderConstantF(4, (const float*)(&viewProjMat), 4);
							// Light space
							ms_lpd3dDevice->SetVertexShaderConstantF(8, (const float*)(&OutDoor.m_MatLightViewProj), 4);

							STATEMANAGER.SetVertexShaderConstant(16, (const float*)(&OutDoor.m_MatLightViewProj2), 4);
							STATEMANAGER.SetVertexShaderConstant(64, &timertree, 1);

							// Light ViewProj matrix (TODO)


							// Pixel shader constants ----
							//D3DXVECTOR4 diffuseMatColor = { 1.0f, 1.0f, 1.0f, 1.0f };
							D3DMATERIAL9 material;
							STATEMANAGER.GetMaterial(&material);
							D3DLIGHT9 light;
							STATEMANAGER.GetLight(0, &light);
							STATEMANAGER.SetPixelShaderConstant(0, &material.Diffuse, 1);
							STATEMANAGER.SetPixelShaderConstant(1, &OutDoor.current_v3SunPosition, 1);
							STATEMANAGER.SetPixelShaderConstant(2, &light.Diffuse, 1);

							D3DXVECTOR3 AmbientLight = { 0.4f, 0.4f, 0.4f };

							STATEMANAGER.SetPixelShaderConstant(3, &AmbientLight, 1);
							// Fog settings
							D3DVECTOR cameraPos = CCameraManager::instance().GetCurrentCamera()->GetEye();
							STATEMANAGER.SetPixelShaderConstant(4, &cameraPos, 1);
							D3DCOLOR fogColor;
							STATEMANAGER.GetRenderState(D3DRS_FOGCOLOR, &fogColor);
							D3DXCOLOR fogColorX(fogColor);
							STATEMANAGER.SetPixelShaderConstant(5, (float*)fogColorX, 1);
							D3DVECTOR fogProps;
							STATEMANAGER.GetRenderState(D3DRS_FOGSTART, (DWORD*)&fogProps.x);
							STATEMANAGER.GetRenderState(D3DRS_FOGEND, (DWORD*)&fogProps.y);
							STATEMANAGER.SetPixelShaderConstant(6, &fogProps, 1);

							STATEMANAGER.SetPixelShaderConstant(7, &material.Ambient, 1);
							float shiness;
							D3DXVECTOR3 specularLight;
							float reducePointLightIntensity = 4.0f;


							shiness = 169.0f;
							specularLight = { 0.98f, 0.89f, 0.63f };


							STATEMANAGER.SetPixelShaderConstant(8, &shiness, 1);
							STATEMANAGER.SetPixelShaderConstant(9, &specularLight, 1);

							float shadowOpacity = 80.0f;


							STATEMANAGER.SetPixelShaderConstant(10, (int*)&shadowOpacity, 1);

							float specularLightEnabled = 1.0f;
							STATEMANAGER.SetPixelShaderConstant(11, &specularLightEnabled, 1);



							float specularPower = 1.0f;
							STATEMANAGER.SetPixelShaderConstant(12, &specularPower, 1);

							STATEMANAGER.SetPixelShaderConstant(13, &timertree, 1);

							float enableLights;

							if (Frustum::Instance().ambianceEffectsLevel == 0)
								enableLights = 0.0f;
							if (Frustum::Instance().ambianceEffectsLevel == 1)
								enableLights = 1.0f;

							STATEMANAGER.SetPixelShaderConstant(15, &reducePointLightIntensity, 1);

							if (OutDoor.m_lpCharacterShadowMapTexture)
								STATEMANAGER.SetTexture(2, OutDoor.m_lpCharacterShadowMapTexture);
							if (OutDoor.m_lpCharacterShadowMapTexture4)
								STATEMANAGER.SetTexture(3, OutDoor.m_lpCharacterShadowMapTexture4);
						}
					}
				}

				ms_faceCount += m_pBranchIndexCounts[m_pGeometryCache->m_sBranches.m_nDiscreteLodLevel] - 2;
				STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, m_pGeometryCache->m_sBranches.m_usVertexCount, 0, m_pBranchIndexCounts[m_pGeometryCache->m_sBranches.m_nDiscreteLodLevel] - 2);

				if (Frustum::Instance().shadowType == 2)
				{
					if (FlyManager) {
						if (MapManager && MapManager->IsMapOutdoor() && MapManager->IsMapReady())
						{
							STATEMANAGER.SetVertexShader(ms_dwBranchVertexShader);
							STATEMANAGER.SetPixelShader(nullptr);
							ms_lpd3dDevice->SetVertexShader(ms_dwBranchVertexShader);
							ms_lpd3dDevice->SetPixelShader(nullptr);
						}
					}
				}

			}
		}
	}
	else
	{
		m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry);

		if (m_pGeometryCache->m_fBranchAlphaTestValue)
		{
			PositionTree();

			// set alpha test value
			STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_fBranchAlphaTestValue));

			// render if this LOD has branches
			if (m_pBranchIndexCounts &&
				m_pBranchIndexCounts[m_pGeometryCache->m_sBranches.m_nDiscreteLodLevel] > 0)
			{
				ms_faceCount += m_pBranchIndexCounts[m_pGeometryCache->m_sBranches.m_nDiscreteLodLevel] - 2;
				STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, m_pGeometryCache->m_sBranches.m_usVertexCount, 0, m_pBranchIndexCounts[m_pGeometryCache->m_sBranches.m_nDiscreteLodLevel] - 2);
			}
		}
	}

}

void CSpeedTreeWrapper::RenderBranchesShadow(void) const
{
	m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BranchGeometry);

	if (m_pGeometryCache->m_fBranchAlphaTestValue > 0.0f)
	{
		PositionTree();
		STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_fBranchAlphaTestValue));
		if (m_pBranchIndexCounts &&
			m_pGeometryCache->m_sBranches.m_nDiscreteLodLevel > -1 &&
			m_pBranchIndexCounts[m_pGeometryCache->m_sBranches.m_nDiscreteLodLevel] > 0)
		{
			ms_faceCount += m_pBranchIndexCounts[m_pGeometryCache->m_sBranches.m_nDiscreteLodLevel] - 2;
			STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, m_pGeometryCache->m_sBranches.m_usVertexCount, 0, m_pBranchIndexCounts[m_pGeometryCache->m_sBranches.m_nDiscreteLodLevel] - 2);
		}
	}
}

void CSpeedTreeWrapper::SetupFrondForTreeType(void)
{
	if (Frustum::Instance().getLeaves < 9.9)
	{
#if defined(__BL_FOG_FIX__)
		BOOL bFogEnable = CPythonBackground::Instance().GetFogMode() ? TRUE : FALSE;
#else
		BOOL bFogEnable = TRUE;
#endif
		STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, bFogEnable);
		ms_lpd3dDevice->SetRenderState(D3DRS_FOGENABLE, bFogEnable);

		float fFogStart = 0.0f, fFogEnd = 0.0f;
		STATEMANAGER.GetRenderState(D3DRS_FOGSTART, (DWORD*)&fFogStart);
		STATEMANAGER.GetRenderState(D3DRS_FOGEND, (DWORD*)&fFogEnd);
		if (fFogEnd > fFogStart)
		{
			D3DXVECTOR4 v4Fog(fFogStart, fFogEnd, 1.0f / (fFogEnd - fFogStart), 0.0f);
			STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_Fog, (const float*)&v4Fog, 1);
		}

		// update the frond geometry for CPU wind
#ifdef WRAPPER_USE_CPU_WIND
		m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry);
		if (m_pGeometryCache->m_sFronds.m_usNumStrips > 0)
		{
			// update the vertex array
			SFVFBranchVertex* pVertexBuffer = NULL;
			m_pFrondVertexBuffer->Lock(0, 0, reinterpret_cast<BYTE**>(&pVertexBuffer), D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK);
			for (UINT i = 0; i < m_unFrondVertexCount; ++i)
			{
				memcpy(&(pVertexBuffer[i].m_vPosition), &(m_pGeometryCache->m_sFronds.m_pCoords[i * 3]), 3 * sizeof(float));
			}
			m_pFrondVertexBuffer->Unlock();
		}
#endif

		if (!m_CompositeImageInstance.IsEmpty())
			STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());

		// bind shadow texture
#ifdef WRAPPER_RENDER_SELF_SHADOWS
		STATEMANAGER.SetTexture(1, NULL);
#endif

		if (m_pGeometryCache->m_sFronds.m_usVertexCount > 0)
		{
#ifdef __ANDROID__
			STATEMANAGER.SetFVF(D3DFVF_SPEEDTREE_BRANCH_VERTEX);
#endif
			// activate the frond vertex buffer
			STATEMANAGER.SetStreamSource(0, m_pFrondVertexBuffer, sizeof(SFVFBranchVertex));
			// set the index buffer
			STATEMANAGER.SetIndices(m_pFrondIndexBuffers, 0);
		}
	}
}

void CSpeedTreeWrapper::RenderFronds(void) const
{
	if (Frustum::Instance().getLeaves < 9.9)
	{
		m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry);

		if (m_pGeometryCache->m_fFrondAlphaTestValue > 0.0f)
		{
			PositionTree();

			// set alpha test value
			STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_fFrondAlphaTestValue));

			// render if this LOD has fronds
			if (m_pFrondIndexCounts &&
				m_pFrondIndexCounts[m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel] > 0)
			{
				ms_faceCount += m_pFrondIndexCounts[m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel] - 2;
#ifdef __ANDROID__
				float fDiv = (Frustum::Instance().getLeaves >= 1.0f) ? Frustum::Instance().getLeaves : 1.0f;
				STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, m_pGeometryCache->m_sFronds.m_usVertexCount, 0, (UINT)((m_pFrondIndexCounts[m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel] - 2) / fDiv));
#else
				STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, m_pGeometryCache->m_sFronds.m_usVertexCount, 0, (m_pFrondIndexCounts[m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel] - 2) / Frustum::Instance().getLeaves);
#endif
			}
		}
	}
}

void CSpeedTreeWrapper::RenderFrondsShadow(void) const
{
	if (Frustum::Instance().getLeaves < 9.9)
	{
		m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_FrondGeometry);

		if (m_pGeometryCache->m_fFrondAlphaTestValue > 0.0f)
		{
			PositionTree();

			// set alpha test value
			STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_fFrondAlphaTestValue));

			// render if this LOD has fronds
			if (m_pFrondIndexCounts &&
				m_pFrondIndexCounts[m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel] > 0)
			{
				ms_faceCount += m_pFrondIndexCounts[m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel] - 2;
				STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, m_pGeometryCache->m_sFronds.m_usVertexCount, 0, (m_pFrondIndexCounts[m_pGeometryCache->m_sFronds.m_nDiscreteLodLevel] - 2) / Frustum::Instance().getLeaves);
			}
		}
	}
}

void CSpeedTreeWrapper::SetupLeafForTreeType(void)
{
	if (Frustum::Instance().getLeaves < 9.9)
	{
#if defined(__BL_FOG_FIX__)
		BOOL bFogEnable = CPythonBackground::Instance().GetFogMode() ? TRUE : FALSE;
#else
		BOOL bFogEnable = TRUE;
#endif
		STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, bFogEnable);
		ms_lpd3dDevice->SetRenderState(D3DRS_FOGENABLE, bFogEnable);

		float fFogStart = 0.0f, fFogEnd = 0.0f;
		STATEMANAGER.GetRenderState(D3DRS_FOGSTART, (DWORD*)&fFogStart);
		STATEMANAGER.GetRenderState(D3DRS_FOGEND, (DWORD*)&fFogEnd);
		if (fFogEnd > fFogStart)
		{
			D3DXVECTOR4 v4Fog(fFogStart, fFogEnd, 1.0f / (fFogEnd - fFogStart), 0.0f);
			STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_Fog, (const float*)&v4Fog, 1);
		}

		// pass leaf tables to shader
#ifdef JTX_GPU_LEAF_PLACEMENT
		UploadLeafTables(c_nVertexShader_LeafTables);
#endif

		if (!m_CompositeImageInstance.IsEmpty())
			STATEMANAGER.SetTexture(0, m_CompositeImageInstance.GetTextureReference().GetD3DTexture());

		// bind shadow texture
#ifdef WRAPPER_RENDER_SELF_SHADOWS
		STATEMANAGER.SetTexture(1, NULL);
#endif
	}
}

#ifdef JTX_GPU_LEAF_PLACEMENT
void CSpeedTreeWrapper::UploadLeafTables(UINT uiLocation) const
{
	// query leaf cluster table from RT
	UINT uiEntryCount = 0;
	const float* pTable = m_pSpeedTree->GetLeafBillboardTable(uiEntryCount);

	// upload for vertex shader use
	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_LeafTables, pTable, uiEntryCount / 4);
}
#endif

void CSpeedTreeWrapper::RenderLeaves(void) const
{
	// update leaf geometry
	m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_LeafGeometry);

	PositionTree();

	// render leaves for only one LOD level
	const CSpeedTreeRT::SGeometry::SLeaf* pLeaf = &m_pGeometryCache->m_sLeaves0;
	int unLod = pLeaf->m_nDiscreteLodLevel;

	if (unLod > -1 && pLeaf->m_bIsActive && pLeaf->m_usLeafCount > 0)
	{
#ifdef ENABLE_OPENGL
		if (m_pLeafVertexBuffer && m_pLeafVertexBuffer[unLod])
		{
			static const short anVertexIndices[6] = { 0, 1, 2, 0, 2, 3 };
			SFVFLeafVertex* pVertex = NULL;
			if (SUCCEEDED(m_pLeafVertexBuffer[unLod]->Lock(0, 0, (void**)&pVertex, 0)) && pVertex)
			{
				UINT uiEntryCount = 0;
				const float* pTable = m_pSpeedTree->GetLeafBillboardTable(uiEntryCount);
				const float* pLodAdjust = m_pSpeedTree->GetLeafLodSizeAdjustments();
				float fScalar = pLodAdjust ? pLodAdjust[unLod] : 1.0f;

				for (UINT unLeaf = 0; unLeaf < pLeaf->m_usLeafCount; ++unLeaf)
				{
					const float* pCenter = &(pLeaf->m_pCenterCoords[unLeaf * 3]);
					const float* pMapCoords = (pLeaf->m_pLeafMapCoords) ? pLeaf->m_pLeafMapCoords[unLeaf] : NULL;
					UINT clusterIdx = (pLeaf->m_pLeafClusterIndices) ? pLeaf->m_pLeafClusterIndices[unLeaf] : 0;
					DWORD dwColor = (pLeaf->m_pColors) ? pLeaf->m_pColors[unLeaf] : 0xFFFFFFFF;

					for (UINT unVert = 0; unVert < 6; ++unVert, ++pVertex)
					{
						int corner = anVertexIndices[unVert];
						if (pTable && (clusterIdx * 4 + corner) * 4 + 2 < uiEntryCount)
						{
							const float* pOffset = pTable + (clusterIdx * 4 + corner) * 4;
							pVertex->m_vPosition.x = pCenter[0] + pOffset[0] * fScalar;
							pVertex->m_vPosition.y = pCenter[1] + pOffset[1] * fScalar;
							pVertex->m_vPosition.z = pCenter[2] + pOffset[2] * fScalar;
						}
						else if (pMapCoords)
						{
							memcpy(pVertex->m_vPosition, &(pMapCoords[corner * 4]), 3 * sizeof(float));
						}
						pVertex->m_dwDiffuseColor = dwColor;
						if (pLeaf->m_pLeafMapTexCoords && pLeaf->m_pLeafMapTexCoords[unLeaf])
						{
							memcpy(pVertex->m_fTexCoords, &(pLeaf->m_pLeafMapTexCoords[unLeaf][corner * 2]), 2 * sizeof(float));
						}
					}
				}
				m_pLeafVertexBuffer[unLod]->Unlock();
			}
		}
#endif
		STATEMANAGER.SetStreamSource(0, m_pLeafVertexBuffer[unLod], sizeof(SFVFLeafVertex));
		STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(pLeaf->m_fAlphaTestValue));

		float fLeafDiv = (Frustum::Instance().getLeaves >= 1.0f) ? Frustum::Instance().getLeaves : 1.0f;
		UINT tenthLeafCount = (UINT)(pLeaf->m_usLeafCount / fLeafDiv);
		if (tenthLeafCount < 1 && pLeaf->m_usLeafCount > 0) tenthLeafCount = pLeaf->m_usLeafCount;
		ms_faceCount += tenthLeafCount * 2;

		STATEMANAGER.DrawPrimitive(D3DPT_TRIANGLELIST, 0, tenthLeafCount * 2);
	}
}

void CSpeedTreeWrapper::RenderLeaves2(void) const
{
	m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_LeafGeometry);
	PositionTree();
}

void CSpeedTreeWrapper::SetupBranchForTreeTypeShadow(void)
{

	STATEMANAGER.SetTexture(0, Frustum::Instance().shadowTexture);
	//STATEMANAGER.SetTexture(0, NULL); //jtxfix

	if (m_pGeometryCache->m_sBranches.m_usVertexCount > 0)
	{
		// activate the branch vertex buffer
		STATEMANAGER.SetStreamSource(0, m_pBranchVertexBuffer, sizeof(SFVFBranchVertex));
		// set the index buffer
		STATEMANAGER.SetIndices(m_pBranchIndexBuffer, 0);
	}
}

void CSpeedTreeWrapper::EndLeafForTreeType(void) {}

void CSpeedTreeWrapper::RenderBillboards(void) const
{
#ifdef WRAPPER_BILLBOARD_MODE
	PositionTree();
	struct SBillboardVertex
	{
		float fX, fY, fZ;
		DWORD dColor;
		float fU, fV;
	};

	m_pSpeedTree->GetGeometry(*m_pGeometryCache, SpeedTree_BillboardGeometry);

	if (m_pGeometryCache->m_sBillboard0.m_bIsActive)
	{
		const float* pCoords = m_pGeometryCache->m_sBillboard0.m_pCoords;
		const float* pTexCoords = m_pGeometryCache->m_sBillboard0.m_pTexCoords;
		SBillboardVertex sVertex[4] =
		{
			{ pCoords[0], pCoords[1], pCoords[2], 0xffffff, pTexCoords[0], pTexCoords[1] },
			{ pCoords[3], pCoords[4], pCoords[5], 0xffffff, pTexCoords[2], pTexCoords[3] },
			{ pCoords[6], pCoords[7], pCoords[8], 0xffffff, pTexCoords[4], pTexCoords[5] },
			{ pCoords[9], pCoords[10], pCoords[11], 0xffffff, pTexCoords[6], pTexCoords[7] },
		};
		STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_sBillboard0.m_fAlphaTestValue));

		ms_faceCount += 2;
		STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, sVertex, sizeof(SBillboardVertex));
	}

	if (m_pGeometryCache->m_sBillboard1.m_bIsActive)
	{
		const float* pCoords = m_pGeometryCache->m_sBillboard1.m_pCoords;
		const float* pTexCoords = m_pGeometryCache->m_sBillboard1.m_pTexCoords;
		SBillboardVertex sVertex[4] =
		{
			{ pCoords[0], pCoords[1], pCoords[2], 0xffffff, pTexCoords[0], pTexCoords[1] },
			{ pCoords[3], pCoords[4], pCoords[5], 0xffffff, pTexCoords[2], pTexCoords[3] },
			{ pCoords[6], pCoords[7], pCoords[8], 0xffffff, pTexCoords[4], pTexCoords[5] },
			{ pCoords[9], pCoords[10], pCoords[11], 0xffffff, pTexCoords[6], pTexCoords[7] },
		};
		STATEMANAGER.SetRenderState(D3DRS_ALPHAREF, DWORD(m_pGeometryCache->m_sBillboard1.m_fAlphaTestValue));

		ms_faceCount += 2;
		STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, sVertex, sizeof(SBillboardVertex));
	}
#endif
}

void CSpeedTreeWrapper::CleanUpMemory(void)
{
	if (!m_bIsInstance)
		m_pSpeedTree->DeleteTransientData();
}

void CSpeedTreeWrapper::PositionTree(void) const
{
	D3DXVECTOR3 vecPosition = m_pSpeedTree->GetTreePosition();
	D3DXMATRIX matTranslation;
	D3DXMatrixIdentity(&matTranslation);
	D3DXMatrixTranslation(&matTranslation, vecPosition.x, vecPosition.y, vecPosition.z);
	STATEMANAGER.SetTransform(D3DTS_WORLD, &matTranslation);
	D3DXVECTOR4 vecConstant(vecPosition[0], vecPosition[1], vecPosition[2], 0.0f);
	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_TreePos, (float*)&vecConstant, 1);
}

void CSpeedTreeWrapper::PositionTreeBranch(void) const
{
	D3DXVECTOR3 vecPosition = m_pSpeedTree->GetTreePosition();
	D3DXMATRIX matTranslation;
	D3DXMatrixIdentity(&matTranslation);
	D3DXMatrixTranslation(&matTranslation, vecPosition.x, vecPosition.y, vecPosition.z);
	STATEMANAGER.SetTransform(D3DTS_WORLD, &matTranslation);

	if (Frustum::Instance().shadowType == 2)
	{
		D3DXMATRIX matShader;
		D3DXMatrixTranspose(&matShader, &matTranslation);
		STATEMANAGER.SetVertexShaderConstant(20, &matShader, 4);
	}
	else
	{
		D3DXVECTOR4 vecConstant(vecPosition[0], vecPosition[1], vecPosition[2], 0.0f);
		STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_TreePos, (float*)&vecConstant, 1);
	}
}

bool CSpeedTreeWrapper::LoadTexture(const char* pFilename, CGraphicImageInstance& rImage)
{
	CResource* pResource = CResourceManager::Instance().GetResourcePointer(pFilename);
	rImage.SetImagePointer(static_cast<CGraphicImage*>(pResource));

	if (rImage.IsEmpty())
		return false;

	return true;
}

void CSpeedTreeWrapper::SetShaderConstants(const float* pMaterial) const
{
	const float afUsefulConstants[] =
	{
		m_pSpeedTree->GetLeafLightingAdjustment(), 0.0f, 0.0f, 0.0f
	};
	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_LeafLightingAdjustment, afUsefulConstants, 1);

	const float afMaterial[] =
	{
		pMaterial[0], pMaterial[1], pMaterial[2], 1.0f,
		pMaterial[3], pMaterial[4], pMaterial[5], 1.0f
	};
	STATEMANAGER.SetVertexShaderConstant(c_nVertexShader_Material, afMaterial, 2);
}

void CSpeedTreeWrapper::SetPosition(float x, float y, float z)
{
	m_afPos[0] = x;
	m_afPos[1] = y;
	m_afPos[2] = z;

	m_pSpeedTree->SetTreePosition(x, y, z);
	CGraphicObjectInstance::SetPosition(x, y, z);
}

const float* CSpeedTreeWrapper::GetPosition()
{
	return m_afPos;
}

//martysama0134's fb0d8d5d0566583187a94f9a823b2075
