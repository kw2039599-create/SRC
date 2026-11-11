#include "StdAfx.h"
#include "EffectInstance.h"
#include "ParticleSystemInstance.h"
#include "SimpleLightInstance.h"

#include "../eterBase/Stl.h"
#include "../eterLib/StateManager.h"
#include "../MilesLib/SoundManager.h"
#include "../eterLib/Camera.h"

#ifdef __ANDROID__
static inline bool IsAndroidEffectInCameraFrustum(const D3DXMATRIX& rMatGlobal, const D3DXVECTOR3& v3SphereOffset, float fSphereRadius, float fExtraPad)
{
	CCamera* pCam = CCameraManager::Instance().GetCurrentCamera();
	if (!pCam)
		return true;

	const float px = rMatGlobal._41 + v3SphereOffset.x;
	const float py = rMatGlobal._42 + v3SphereOffset.y;
	const float pz = rMatGlobal._43 + v3SphereOffset.z;

	// Never cull unpositioned effects at the origin
	if (fabsf(px) < 1.0f && fabsf(py) < 1.0f && fabsf(pz) < 1.0f)
		return true;

	const D3DXVECTOR3& v3Eye = pCam->GetEye();
	const D3DXVECTOR3& v3Target = pCam->GetTarget();

	static D3DXVECTOR3 s_lastEye(0.0f, 0.0f, 0.0f);
	static D3DXVECTOR3 s_lastTarget(0.0f, 0.0f, 0.0f);
	static D3DXVECTOR3 s_F(0.0f, 1.0f, 0.0f);
	static D3DXVECTOR3 s_R(1.0f, 0.0f, 0.0f);
	static D3DXVECTOR3 s_U(0.0f, 0.0f, 1.0f);

	if (v3Eye != s_lastEye || v3Target != s_lastTarget)
	{
		s_lastEye = v3Eye;
		s_lastTarget = v3Target;
		float fx = v3Target.x - v3Eye.x;
		float fy = v3Target.y - v3Eye.y;
		float fz = v3Target.z - v3Eye.z;
		float flen = sqrtf(fx * fx + fy * fy + fz * fz);
		if (flen > 1e-3f) { fx /= flen; fy /= flen; fz /= flen; }
		s_F = D3DXVECTOR3(fx, fy, fz);

		float rlen = sqrtf(fy * fy + fx * fx);
		if (rlen > 1e-3f)
			s_R = D3DXVECTOR3(fy / rlen, -fx / rlen, 0.0f);
		else
			s_R = D3DXVECTOR3(1.0f, 0.0f, 0.0f);

		s_U = D3DXVECTOR3(
			s_R.y * s_F.z,
			-s_R.x * s_F.z,
			s_R.x * s_F.y - s_R.y * s_F.x);
	}

	const float pad = ((fSphereRadius > 260.0f) ? fSphereRadius : 260.0f) + fExtraPad;

	// Always keep effects near the camera target (main player) active
	const float tdx = px - v3Target.x;
	const float tdy = py - v3Target.y;
	if ((tdx * tdx + tdy * tdy) <= (550.0f * 550.0f))
		return true;

	const float vx = px - v3Eye.x;
	const float vy = py - v3Eye.y;
	const float vz = pz - v3Eye.z;

	const float zCam = vx * s_F.x + vy * s_F.y + vz * s_F.z;
	if (zCam < -pad)
		return false;

	const float xCam = fabsf(vx * s_R.x + vy * s_R.y);
	if (xCam > (zCam * 1.35f + pad))
		return false;

	const float yCam = fabsf(vx * s_U.x + vy * s_U.y + vz * s_U.z);
	if (yCam > (zCam * 0.95f + pad))
		return false;

	return true;
}
#endif
CDynamicPool<CEffectInstance>	CEffectInstance::ms_kPool;
int CEffectInstance::ms_iRenderingEffectCount = 0;

bool CEffectInstance::LessRenderOrder(CEffectInstance* pkEftInst)
{
	return (m_pkEftData<pkEftInst->m_pkEftData);
}

void CEffectInstance::ResetRenderingEffectCount()
{
	ms_iRenderingEffectCount = 0;
}

int CEffectInstance::GetRenderingEffectCount()
{
	return ms_iRenderingEffectCount;
}

CEffectInstance* CEffectInstance::New()
{
	CEffectInstance* pkEftInst=ms_kPool.Alloc();
	return pkEftInst;
}

void CEffectInstance::Delete(CEffectInstance* pkEftInst)
{
	pkEftInst->Clear();
	ms_kPool.Free(pkEftInst);
}

void CEffectInstance::DestroySystem()
{
	ms_kPool.Destroy();

	CParticleSystemInstance::DestroySystem();
	CEffectMeshInstance::DestroySystem();
	CLightInstance::DestroySystem();
}

void CEffectInstance::UpdateSound()
{
	if (m_pSoundInstanceVector)
	{
		CSoundManager& rkSndMgr=CSoundManager::Instance();
		rkSndMgr.UpdateSoundInstance(m_matGlobal._41, m_matGlobal._42, m_matGlobal._43, m_dwFrame, m_pSoundInstanceVector);
	}
	++m_dwFrame;
}

struct FEffectUpdator
{
	BOOL isAlive;
	float fElapsedTime;
	FEffectUpdator(float fElapsedTime)
		: isAlive(FALSE), fElapsedTime(fElapsedTime)
	{
	}
	void operator () (CEffectElementBaseInstance * pInstance)
	{
		if (pInstance->Update(fElapsedTime))
			isAlive = TRUE;
	}
};

struct FEffectFrustum
{
	bool ignoreFrustum;
	FEffectFrustum(bool igno)
		: ignoreFrustum(igno)
	{
	}
	void operator () (CEffectElementBaseInstance * pInstance)
	{
		pInstance->SetIgnoreFrustum(ignoreFrustum);
	}
};

void CEffectInstance::OnUpdate()
{
#ifndef __ANDROID__
	Transform();
#endif

#ifdef WORLD_EDITOR
	const float fElapsedTime = CTimer::Instance().GetElapsedSecond();
#else
	const float fCurTime = CTimer::Instance().GetCurrentSecond();
	float fElapsedTime = fCurTime - m_fLastTime;
#ifdef __ANDROID__
	if (!m_ignoreFrustum && m_dwFrame > 2)
	{
		if (!IsAndroidEffectInCameraFrustum(m_matGlobal, m_v3BoundingSpherePosition, m_fBoundingSphereRadius, 150.0f))
		{
			// Off-screen effect: only step simulation every 8th frame so one-shot effects still expire
			// without burning CPU on 200 off-screen players' shining/aura particles every frame
			if ((m_dwFrame & 7) != 0)
				return;
		}
	}
#endif
	m_fLastTime = fCurTime;
	if (fElapsedTime < 0.0f)
		fElapsedTime = 0.0f;
#ifdef __ANDROID__
	else if (fElapsedTime > 0.10f)
		fElapsedTime = 0.10f;
#else
	else if (fElapsedTime > 0.05f)
		fElapsedTime = 0.05f;
#endif
#endif

	BOOL bAlive = FALSE;
	for (size_t i = 0, n = m_ParticleInstanceVector.size(); i < n; ++i)
	{
		if (m_ParticleInstanceVector[i]->Update(fElapsedTime))
			bAlive = TRUE;
	}
	for (size_t i = 0, n = m_MeshInstanceVector.size(); i < n; ++i)
	{
		if (m_MeshInstanceVector[i]->Update(fElapsedTime))
			bAlive = TRUE;
	}
	for (size_t i = 0, n = m_LightInstanceVector.size(); i < n; ++i)
	{
		if (m_LightInstanceVector[i]->Update(fElapsedTime))
			bAlive = TRUE;
	}
	m_isAlive = bAlive;
}

void CEffectInstance::OnRender()
{
	if (m_fBoundingSphereRadius > 0.0f && !m_ignoreFrustum && !isShow())
		return;

	if (m_ParticleInstanceVector.empty() && m_MeshInstanceVector.empty())
		return;

#ifdef __ANDROID__
	if (!m_ignoreFrustum && !IsAndroidEffectInCameraFrustum(m_matGlobal, m_v3BoundingSpherePosition, m_fBoundingSphereRadius, 250.0f))
		return;
#endif

#ifdef DIRECTX9
	STATEMANAGER.SaveSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	STATEMANAGER.SaveSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
#else
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
#endif
	STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	STATEMANAGER.SaveRenderState(D3DRS_ZWRITEENABLE, FALSE);

	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);
#ifdef DIRECTX9
	STATEMANAGER.SaveFVF(D3DFVF_XYZ | D3DFVF_TEX1);
#else
	STATEMANAGER.SetVertexShader(D3DFVF_XYZ | D3DFVF_TEX1);
#endif
	for (size_t i = 0, n = m_ParticleInstanceVector.size(); i < n; ++i)
		m_ParticleInstanceVector[i]->Render();
	for (size_t i = 0, n = m_MeshInstanceVector.size(); i < n; ++i)
		m_MeshInstanceVector[i]->Render();

#ifdef DIRECTX9
	STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MINFILTER);
	STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MAGFILTER);
#else
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_MINFILTER);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_MAGFILTER);
#endif
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
	STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
	STATEMANAGER.RestoreRenderState(D3DRS_ZWRITEENABLE);

	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG2);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG1);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG2);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAOP);
#ifdef DIRECTX9
	STATEMANAGER.RestoreFVF();
#endif

	++ms_iRenderingEffectCount;
}

void CEffectInstance::SetGlobalMatrix(const D3DXMATRIX & c_rmatGlobal)
{
	const bool bPosMoved = (
		fabsf(m_matGlobal._41 - c_rmatGlobal._41) > 5.0f ||
		fabsf(m_matGlobal._42 - c_rmatGlobal._42) > 5.0f ||
		fabsf(m_matGlobal._43 - c_rmatGlobal._43) > 5.0f);

	m_matGlobal = c_rmatGlobal;

	if (bPosMoved && m_fBoundingSphereRadius > 0.0f)
		UpdateBoundingSphere();
}

BOOL CEffectInstance::isAlive()
{
	return m_isAlive;
}

void CEffectInstance::SetActive()
{
	m_fLastTime = CTimer::Instance().GetCurrentSecond();
	std::for_each(
		m_ParticleInstanceVector.begin(),
		m_ParticleInstanceVector.end(),
		std::void_mem_fun(&CEffectElementBaseInstance::SetActive));
	std::for_each(
		m_MeshInstanceVector.begin(),
		m_MeshInstanceVector.end(),
		std::void_mem_fun(&CEffectElementBaseInstance::SetActive));
	std::for_each(
		m_LightInstanceVector.begin(),
		m_LightInstanceVector.end(),
		std::void_mem_fun(&CEffectElementBaseInstance::SetActive));
}

void CEffectInstance::SetDeactive()
{
	std::for_each(
		m_ParticleInstanceVector.begin(),
		m_ParticleInstanceVector.end(),
		std::void_mem_fun(&CEffectElementBaseInstance::SetDeactive));
	std::for_each(
		m_MeshInstanceVector.begin(),
		m_MeshInstanceVector.end(),
		std::void_mem_fun(&CEffectElementBaseInstance::SetDeactive));
	std::for_each(
		m_LightInstanceVector.begin(),
		m_LightInstanceVector.end(),
		std::void_mem_fun(&CEffectElementBaseInstance::SetDeactive));
}

void CEffectInstance::__SetParticleData(CParticleSystemData * pData)
{
	CParticleSystemInstance * pInstance = CParticleSystemInstance::New();
	pInstance->SetDataPointer(pData);
	pInstance->SetLocalMatrixPointer(&m_matGlobal);

	pInstance->SetParticleScale(GetParticleScale());
	m_ParticleInstanceVector.push_back(pInstance);
}
void CEffectInstance::__SetMeshData(CEffectMeshScript * pMesh)
{
	CEffectMeshInstance * pMeshInstance = CEffectMeshInstance::New();
	pMeshInstance->SetDataPointer(pMesh);
	pMeshInstance->SetLocalMatrixPointer(&m_matGlobal);

	pMeshInstance->SetMeshScale(GetMeshScale());
	m_MeshInstanceVector.push_back(pMeshInstance);
}

void CEffectInstance::__SetLightData(CLightData* pData)
{
	CLightInstance * pInstance = CLightInstance::New();
	pInstance->SetDataPointer(pData);
	pInstance->SetLocalMatrixPointer(&m_matGlobal);

	m_LightInstanceVector.push_back(pInstance);
}

void CEffectInstance::SetEffectDataPointer(CEffectData * pEffectData)
{
	m_isAlive = true;

	m_pkEftData = pEffectData;

	m_fLastTime = CTimer::Instance().GetCurrentSecond();
	m_fBoundingSphereRadius = pEffectData->GetBoundingSphereRadius();
	m_v3BoundingSpherePosition = pEffectData->GetBoundingSpherePosition();

#ifdef __ANDROID__
	Show();
#else
	if (m_fBoundingSphereRadius > 0.0f)
		CGraphicObjectInstance::RegisterBoundingSphere();
#endif
	DWORD i;

	for (i = 0; i < pEffectData->GetParticleCount(); ++i)
	{
		CParticleSystemData * pParticle = pEffectData->GetParticlePointer(i);

		__SetParticleData(pParticle);
	}

	for (i = 0; i < pEffectData->GetMeshCount(); ++i)
	{
		CEffectMeshScript * pMesh = pEffectData->GetMeshPointer(i);

		__SetMeshData(pMesh);
	}

	for (i = 0; i < pEffectData->GetLightCount(); ++i)
	{
		CLightData * pLight = pEffectData->GetLightPointer(i);

		__SetLightData(pLight);
	}

	m_pSoundInstanceVector = pEffectData->GetSoundInstanceVector();
#ifdef __ANDROID__
	m_fLastTime = CTimer::Instance().GetCurrentSecond() - 0.016f;
#endif
}

bool CEffectInstance::GetBoundingSphere(D3DXVECTOR3 & v3Center, float & fRadius)
{
	v3Center.x = m_matGlobal._41 + m_v3BoundingSpherePosition.x;
	v3Center.y = m_matGlobal._42 + m_v3BoundingSpherePosition.y;
	v3Center.z = m_matGlobal._43 + m_v3BoundingSpherePosition.z;
	fRadius = m_fBoundingSphereRadius;
	return true;
}

void CEffectInstance::Clear()
{
	if (!m_ParticleInstanceVector.empty())
	{
		std::for_each(m_ParticleInstanceVector.begin(), m_ParticleInstanceVector.end(), CParticleSystemInstance::Delete);
		m_ParticleInstanceVector.clear();
	}

	if (!m_MeshInstanceVector.empty())
	{
		std::for_each(m_MeshInstanceVector.begin(), m_MeshInstanceVector.end(), CEffectMeshInstance::Delete);
		m_MeshInstanceVector.clear();
	}

	if (!m_LightInstanceVector.empty())
	{
		std::for_each(m_LightInstanceVector.begin(), m_LightInstanceVector.end(), CLightInstance::Delete);
		m_LightInstanceVector.clear();
	}

#ifdef __ANDROID__
	Show();
#endif
	__Initialize();
}

void CEffectInstance::__Initialize()
{
	m_isAlive = FALSE;
	m_dwFrame = 0;
	m_ignoreFrustum = false;
	m_pSoundInstanceVector = NULL;
	m_fBoundingSphereRadius = 0.0f;
	m_v3BoundingSpherePosition.x = m_v3BoundingSpherePosition.y = m_v3BoundingSpherePosition.z = 0.0f;

	m_fParticleScale = 1.0f;
	m_v3MeshScale.x = m_v3MeshScale.y = m_v3MeshScale.z = 1.0f;

	m_pkEftData=NULL;
#ifdef __ENABLE_STEALTH_FIX__
	ReleaseAlwaysHidden();
#endif
	D3DXMatrixIdentity(&m_matGlobal);
}

CEffectInstance::CEffectInstance()
{
	__Initialize();
}
CEffectInstance::~CEffectInstance()
{
	assert(m_ParticleInstanceVector.empty());
	assert(m_MeshInstanceVector.empty());
	assert(m_LightInstanceVector.empty());
}
//martysama0134's fb0d8d5d0566583187a94f9a823b2075