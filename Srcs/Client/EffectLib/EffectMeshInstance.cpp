#include "StdAfx.h"
#include "../eterLib/StateManager.h"
#include "../eterLib/ResourceManager.h"
#include "EffectMeshInstance.h"
#include "../eterlib/GrpMath.h"

CDynamicPool<CEffectMeshInstance>		CEffectMeshInstance::ms_kPool;

void CEffectMeshInstance::DestroySystem()
{
	ms_kPool.Destroy();
}

CEffectMeshInstance* CEffectMeshInstance::New()
{
	return ms_kPool.Alloc();
}

void CEffectMeshInstance::Delete(CEffectMeshInstance* pkMeshInstance)
{
	pkMeshInstance->Destroy();
	ms_kPool.Free(pkMeshInstance);
}

BOOL CEffectMeshInstance::isActive()
{
	if (!CEffectElementBaseInstance::isActive())
		return FALSE;

	if (!m_MeshFrameController.isActive())
		return FALSE;

	const int iCurrentFrame = m_MeshFrameController.GetCurrentFrame();
	const size_t uTexCount = m_TextureInstanceVector.size();
	for (size_t j = 0; j < uTexCount; ++j)
	{
		if (m_TextureInstanceVector[j].TextureFrameController.isActive(iCurrentFrame))
			return TRUE;
	}

	return FALSE;
}

bool CEffectMeshInstance::OnUpdate(float fElapsedTime)
{
	if (!isActive())
		return false;

	if (m_MeshFrameController.isActive())
		m_MeshFrameController.Update(fElapsedTime);

	const int iCurrentFrame = m_MeshFrameController.GetCurrentFrame();
	const size_t uTexCount = m_TextureInstanceVector.size();
	for (size_t j = 0; j < uTexCount; ++j)
	{
		if (m_TextureInstanceVector[j].TextureFrameController.isActive(iCurrentFrame))
			m_TextureInstanceVector[j].TextureFrameController.Update(fElapsedTime);
	}

	return true;
}

void CEffectMeshInstance::OnRender()
{
	if (!isActive())
		return;

	CEffectMesh* pEffectMesh = m_roMesh.GetPointer();
	if (!pEffectMesh)
		return;

	const DWORD dwMeshCount = pEffectMesh->GetMeshCount();
	const int iCurMeshFrame = m_MeshFrameController.GetCurrentFrame();

	// Precompute constant 90.0f radian X-rotation matrix once
	static const D3DXMATRIX s_matRotX90 = []() {
		D3DXMATRIX m;
		D3DXMatrixRotationX(&m, 90.0f);
		return m;
	}();

	// Cache inverse view matrix once per View change instead of per effect sub-mesh
	static D3DXMATRIX s_lastViewMat;
	static D3DXMATRIX s_matInvView;
	const D3DXMATRIX& c_rViewMat = CScreen::GetViewMatrix();
	if (memcmp(&c_rViewMat, &s_lastViewMat, sizeof(D3DXMATRIX)) != 0)
	{
		s_lastViewMat = c_rViewMat;
		D3DXMatrixInverse(&s_matInvView, NULL, &c_rViewMat);
	}

	D3DXVECTOR3 BasePosition;
	m_pMeshScript->GetPosition(m_fLocalTime, BasePosition);

	for (DWORD i = 0; i < dwMeshCount; ++i)
	{
		assert(i < m_TextureInstanceVector.size());

		CFrameController& rTextureFrameController = m_TextureInstanceVector[i].TextureFrameController;
		if (!rTextureFrameController.isActive(iCurMeshFrame))
			continue;

		CEffectMesh::TEffectMeshData* pMeshData = pEffectMesh->GetMeshDataPointer(i);
		assert(iCurMeshFrame < (int)pMeshData->EffectFrameDataVector.size());
		CEffectMesh::TEffectFrameData& rFrameData = pMeshData->EffectFrameDataVector[iCurMeshFrame];

		if (!rFrameData.dwIndexCount)
			continue;

		const int iBillboardType = m_pMeshScript->GetBillboardType(i);

		D3DXMATRIX m_matWorld;

		switch (iBillboardType)
		{
			case MESH_BILLBOARD_TYPE_ALL:
				D3DXMatrixMultiply(&m_matWorld, &s_matRotX90, &s_matInvView);
				break;

			case MESH_BILLBOARD_TYPE_Y:
				D3DXMatrixIdentity(&m_matWorld);
				m_matWorld._11 = s_matInvView._11;
				m_matWorld._12 = s_matInvView._12;
				m_matWorld._21 = s_matInvView._21;
				m_matWorld._22 = s_matInvView._22;
				break;

			case MESH_BILLBOARD_TYPE_MOVE:
				{
					D3DXMatrixIdentity(&m_matWorld);
					D3DXVECTOR3 LastPosition;
#ifdef __ANDROID__
					const float fLookback = (m_fLocalTime > 0.016f) ? (m_fLocalTime - 0.016f) : 0.0f;
					m_pMeshScript->GetPosition(fLookback, LastPosition);
#else
					m_pMeshScript->GetPosition(m_fLocalTime - CTimer::Instance().GetElapsedSecond(), LastPosition);
#endif
					D3DXVECTOR3 Dir = BasePosition - LastPosition;
					if (D3DXVec3LengthSq(&Dir) > 0.001f)
					{
						D3DXVec3Normalize(&Dir, &Dir);
						D3DXQUATERNION q = SafeRotationNormalizedArc(D3DXVECTOR3(0.0f, -1.0f, 0.0f), Dir);
						D3DXMatrixRotationQuaternion(&m_matWorld, &q);
					}
				}
				break;

			default:
				D3DXMatrixIdentity(&m_matWorld);
				break;
		}

		if (!m_pMeshScript->isBlendingEnable(i))
		{
			STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		}
		else
		{
			STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, m_pMeshScript->GetBlendingSrcType(i));
			STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, m_pMeshScript->GetBlendingDestType(i));
		}

		m_matWorld._41 += BasePosition.x;
		m_matWorld._42 += BasePosition.y;
		m_matWorld._43 += BasePosition.z;

		// Direct column scaling: replaces D3DXMatrixTransformation + D3DXMatrixMultiply
		const float sx = m_v3MeshScale.x;
		const float sy = m_v3MeshScale.y;
		const float sz = m_v3MeshScale.z;
		m_matWorld._11 *= sx; m_matWorld._12 *= sy; m_matWorld._13 *= sz;
		m_matWorld._21 *= sx; m_matWorld._22 *= sy; m_matWorld._23 *= sz;
		m_matWorld._31 *= sx; m_matWorld._32 *= sy; m_matWorld._33 *= sz;
		m_matWorld._41 *= sx; m_matWorld._42 *= sy; m_matWorld._43 *= sz;

		D3DXMatrixMultiply(&m_matWorld, &m_matWorld, mc_pmatLocal);
		STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matWorld);

		BYTE byType;
		D3DXCOLOR Color(1.0f, 1.0f, 1.0f, 1.0f);
		if (m_pMeshScript->GetColorOperationType(i, &byType))
			STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, byType);
		m_pMeshScript->GetColorFactor(i, &Color);

		TTimeEventTableFloat* TableAlpha;
		float fAlpha = 1.0f;
		if (m_pMeshScript->GetTimeTableAlphaPointer(i, &TableAlpha) && !TableAlpha->empty())
			GetTimeEventBlendValue(m_fLocalTime, *TableAlpha, &fAlpha);

		const DWORD dwcurTextureFrame = rTextureFrameController.GetCurrentFrame();
		if (dwcurTextureFrame < m_TextureInstanceVector[i].TextureInstanceVector.size())
		{
			CGraphicImageInstance* pImageInstance = m_TextureInstanceVector[i].TextureInstanceVector[dwcurTextureFrame];
			STATEMANAGER.SetTexture(0, pImageInstance->GetTexturePointer()->GetD3DTexture());
		}

		Color.a = fAlpha * rFrameData.fVisibility;
		STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, DWORD(Color));
#ifdef DIRECTX9
		STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);
#else
		STATEMANAGER.SetVertexShader(D3DFVF_XYZ | D3DFVF_TEX1);
#endif
		STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLELIST,
								 rFrameData.dwIndexCount / 3,
								 rFrameData.PDTVertexVector.data(),
								 sizeof(TPTVertex));
	}
}

void CEffectMeshInstance::OnSetDataPointer(CEffectElementBase * pElement)
{
	CEffectMeshScript * pMesh = (CEffectMeshScript *)pElement;
	m_pMeshScript = pMesh;

	const char * c_szMeshFileName = pMesh->GetMeshFileName();

	m_pEffectMesh = (CEffectMesh *) CResourceManager::Instance().GetResourcePointer(c_szMeshFileName);

	if (!m_pEffectMesh)
		return;

	m_roMesh.SetPointer(m_pEffectMesh);

	m_MeshFrameController.Clear();
	m_MeshFrameController.SetMaxFrame(m_roMesh.GetPointer()->GetFrameCount());
	m_MeshFrameController.SetFrameTime(pMesh->GetMeshAnimationFrameDelay());
	m_MeshFrameController.SetLoopFlag(pMesh->isMeshAnimationLoop());
	m_MeshFrameController.SetLoopCount(pMesh->GetMeshAnimationLoopCount());
	m_MeshFrameController.SetStartFrame(0);

	m_TextureInstanceVector.clear();
	m_TextureInstanceVector.resize(m_pEffectMesh->GetMeshCount());
	for (DWORD j = 0; j < m_TextureInstanceVector.size(); ++j)
	{
		CEffectMeshScript::TMeshData * pMeshData;
		if (!m_pMeshScript->GetMeshDataPointer(j, &pMeshData))
			continue;

		CEffectMesh* pkEftMesh=m_roMesh.GetPointer();

		if (!pkEftMesh)
			continue;

		std::vector<CGraphicImage*>* pTextureVector = pkEftMesh->GetTextureVectorPointer(j);
		if (!pTextureVector)
			continue;

		std::vector<CGraphicImage*>& rTextureVector = *pTextureVector;

		CFrameController & rFrameController = m_TextureInstanceVector[j].TextureFrameController;
		rFrameController.Clear();
		rFrameController.SetMaxFrame(rTextureVector.size());
		rFrameController.SetFrameTime(pMeshData->fTextureAnimationFrameDelay);
		rFrameController.SetLoopFlag(pMeshData->bTextureAnimationLoopEnable);
		rFrameController.SetStartFrame(pMeshData->dwTextureAnimationStartFrame);

		std::vector<CGraphicImageInstance*> & rImageInstanceVector = m_TextureInstanceVector[j].TextureInstanceVector;
		rImageInstanceVector.clear();
		rImageInstanceVector.reserve(rTextureVector.size());
		for (std::vector<CGraphicImage*>::iterator itor = rTextureVector.begin(); itor != rTextureVector.end(); ++itor)
		{
			CGraphicImage * pImage = *itor;
			CGraphicImageInstance * pImageInstance = CGraphicImageInstance::ms_kPool.Alloc();
			pImageInstance->SetImagePointer(pImage);
			rImageInstanceVector.push_back(pImageInstance);
		}
	}
}

void CEffectMeshInstance_DeleteImageInstance(CGraphicImageInstance * pkInstance)
{
	CGraphicImageInstance::ms_kPool.Free(pkInstance);
}

void CEffectMeshInstance_DeleteTextureInstance(CEffectMeshInstance::TTextureInstance & rkInstance)
{
	std::vector<CGraphicImageInstance*> & rVector = rkInstance.TextureInstanceVector;
	for_each(rVector.begin(), rVector.end(), CEffectMeshInstance_DeleteImageInstance);
	rVector.clear();
}

void CEffectMeshInstance::OnInitialize()
{
}

void CEffectMeshInstance::OnDestroy()
{
	for_each(m_TextureInstanceVector.begin(), m_TextureInstanceVector.end(), CEffectMeshInstance_DeleteTextureInstance);
	m_TextureInstanceVector.clear();
	m_roMesh.SetPointer(NULL);
}

CEffectMeshInstance::CEffectMeshInstance()
{
	Initialize();
}

CEffectMeshInstance::~CEffectMeshInstance()
{
	Destroy();
}
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
