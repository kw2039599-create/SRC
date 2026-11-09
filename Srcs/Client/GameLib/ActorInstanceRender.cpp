#include "StdAfx.h"
#include "../eterLib/StateManager.h"
#include "../eterBase/Timer.h"
#include "../GameLib/MapManager.h"

#include "ActorInstance.h"
#include "../UserInterface/PythonBackground.h"
#include "../EterLib/Camera.h"

bool CActorInstance::ms_isDirLine=false;

bool CActorInstance::IsDirLine()
{
	return ms_isDirLine;
}

void CActorInstance::ShowDirectionLine(bool isVisible)
{
	ms_isDirLine=isVisible;
}

void CActorInstance::SetMaterialColor(DWORD dwColor)
{
	if (m_pkHorse)
		m_pkHorse->SetMaterialColor(dwColor);

	m_dwMtrlColor&=0xff000000;
	m_dwMtrlColor|=(dwColor&0x00ffffff);
}

void CActorInstance::SetMaterialAlpha(DWORD dwAlpha)
{
	m_dwMtrlAlpha=dwAlpha;
}

float shiness2 = 15.0f;
float timer = 0.0f;

void CActorInstance::OnRender()
{
	auto* FlyManager = CFlyingManager::InstancePtr();
	auto* MapManager = FlyManager ? FlyManager->GetMapManagerPtr() : nullptr;

	const bool bUseShadowShader = (Frustum::Instance().graphicsLevel > 0 && m_fAlphaValue >= 0.99f &&
		MapManager && MapManager->IsMapOutdoor() && MapManager->IsMapReady());

	D3DMATERIAL9 kMtrl;
	STATEMANAGER.GetMaterial(&kMtrl);
	kMtrl.Diffuse = D3DXCOLOR(m_dwMtrlColor);

	if (!bUseShadowShader)
	{
		STATEMANAGER.SetMaterial(&kMtrl);
	}

	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	if (bUseShadowShader)
	{
		auto& OutDoor = MapManager->GetMapOutdoorRef();

		STATEMANAGER.SetVertexShader(OutDoor.m_lpShadowMapVertexShaderMain);
		STATEMANAGER.SetPixelShader(OutDoor.m_lpShadowMapPixelShaderMain);
		STATEMANAGER.SaveRenderState(D3DRS_FOGENABLE, FALSE);

#ifndef __ANDROID__
		// Per-actor World Matrix (Register c0..c3) - used by DirectX HLSL vertex shader
		D3DXMATRIX worldMat;
		STATEMANAGER.GetTransform(D3DTS_WORLD, &worldMat);
		D3DXMatrixTranspose(&worldMat, &worldMat);
		ms_lpd3dDevice->SetVertexShaderConstantF(0, (const float*)(&worldMat), 4);

		// Per-actor Material & Alpha Constants
		const float alphaVec[4] = { m_fAlphaValue, 0.0f, 0.0f, 0.0f };
		ms_lpd3dDevice->SetPixelShaderConstantF(83, alphaVec, 1);
		ms_lpd3dDevice->SetPixelShaderConstantF(0, (const float*)&kMtrl.Diffuse, 1);
		ms_lpd3dDevice->SetPixelShaderConstantF(7, (const float*)&kMtrl.Ambient, 1);
#else
		static float s_lastAlphaValue = -1.0f;
		if (m_fAlphaValue != s_lastAlphaValue)
		{
			s_lastAlphaValue = m_fAlphaValue;
			const float alphaVec[4] = { m_fAlphaValue, 0.0f, 0.0f, 0.0f };
			ms_lpd3dDevice->SetPixelShaderConstantF(83, alphaVec, 1);
		}
		static DWORD s_lastMtrlColor = 0xDEADBEEFu;
		if (m_dwMtrlColor != s_lastMtrlColor)
		{
			s_lastMtrlColor = m_dwMtrlColor;
			ms_lpd3dDevice->SetPixelShaderConstantF(0, (const float*)&kMtrl.Diffuse, 1);
		}
		static D3DCOLORVALUE s_lastAmbient = { -1.0f, -1.0f, -1.0f, -1.0f };
		if (memcmp(&kMtrl.Ambient, &s_lastAmbient, sizeof(D3DCOLORVALUE)) != 0)
		{
			s_lastAmbient = kMtrl.Ambient;
			ms_lpd3dDevice->SetPixelShaderConstantF(7, (const float*)&kMtrl.Ambient, 1);
		}
#endif

		// Cache scene/pass-invariant constants per View matrix & millisecond tick
		static DWORD s_dwLastFrameTick = 0;
		static D3DXMATRIX s_lastViewMat;
		static D3DXMATRIX s_cachedViewProjMat;

		D3DXMATRIX viewMat;
		STATEMANAGER.GetTransform(D3DTS_VIEW, &viewMat);
		const DWORD dwCurTick = CTimer::Instance().GetCurrentMillisecond();

		// Recompute ViewProj and global lighting only when the pass/frame changes
		if (dwCurTick != s_dwLastFrameTick || memcmp(&viewMat, &s_lastViewMat, sizeof(D3DXMATRIX)) != 0)
		{
			s_lastViewMat = viewMat;

#ifndef __ANDROID__
			D3DXMATRIX projMat;
			STATEMANAGER.GetTransform(D3DTS_PROJECTION, &projMat);
			D3DXMatrixMultiplyTranspose(&s_cachedViewProjMat, &viewMat, &projMat);
#else
			ms_lpd3dDevice->SetVertexShaderConstantF(8, (const float*)(&OutDoor.m_MatLightViewProj), 4);
			STATEMANAGER.SetVertexShaderConstant(16, (const float*)(&OutDoor.m_MatLightViewProj2), 4);
#endif

			if (dwCurTick != s_dwLastFrameTick)
			{
				s_dwLastFrameTick = dwCurTick;
				timer += 0.0000060f;
				if (timer > 10.0f)
					timer = 0.0f;

				if (Frustum::Instance().selectRainFrustun != 0 && Frustum::Instance().isThunderTime)
				{
					shiness2 += 3.0f;
					if (shiness2 > 156.0f)
						shiness2 = 156.0f;
				}
				else
				{
					shiness2 = 10.0f;
				}
			}

			D3DLIGHT9 light;
			STATEMANAGER.GetLight(0, &light);

			D3DVECTOR cameraPos = CCameraManager::instance().GetCurrentCamera()->GetEye();
			D3DCOLOR fogColor;
			STATEMANAGER.GetRenderState(D3DRS_FOGCOLOR, &fogColor);
			D3DXCOLOR fogColorX(fogColor);

			DWORD dwFogStart = 0, dwFogEnd = 0;
			STATEMANAGER.GetRenderState(D3DRS_FOGSTART, &dwFogStart);
			STATEMANAGER.GetRenderState(D3DRS_FOGEND, &dwFogEnd);

			float shiness = 4.0f;
			float specR = 0.98f, specG = 0.89f, specB = 0.63f;

			if (Frustum::Instance().selectRainFrustun != 0)
			{
				if (Frustum::Instance().isThunderTime)
				{
					shiness = shiness2;
					specR = 1.0f; specG = 1.0f; specB = 1.0f;
				}
				else
				{
					shiness = 89.0f;
					specR = 0.2f; specG = 0.2f; specB = 0.2f;
				}
			}

			float shadowOpacity = 70.0f;
			const int rainFrustum = Frustum::Instance().selectRainFrustun;
			if (rainFrustum == 16 || rainFrustum == 25 || rainFrustum == 8)
				shadowOpacity = 80.0f;

			// Pack c1..c6 and c8..c15 into safe 16-byte aligned float4 registers
			const float c1_to_c6[6][4] = {
				{ OutDoor.current_v3SunPosition.x, OutDoor.current_v3SunPosition.y, OutDoor.current_v3SunPosition.z / 1.5f, 0.0f }, // c1
				{ light.Diffuse.r, light.Diffuse.g, light.Diffuse.b, light.Diffuse.a },                                              // c2
				{ 0.6f, 0.6f, 0.6f, 0.0f },                                                                                          // c3
				{ cameraPos.x, cameraPos.y, cameraPos.z, 0.0f },                                                                     // c4
				{ fogColorX.r, fogColorX.g, fogColorX.b, fogColorX.a },                                                              // c5
				{ *(float*)&dwFogStart, *(float*)&dwFogEnd, 0.0f, 0.0f }                                                             // c6
			};
			ms_lpd3dDevice->SetPixelShaderConstantF(1, &c1_to_c6[0][0], 6);

			const float c8_to_c15[8][4] = {
				{ shiness, 0.0f, 0.0f, 0.0f },                                   // c8
				{ specR, specG, specB, 0.0f },                                   // c9
				{ shadowOpacity, 0.0f, 0.0f, 0.0f },                             // c10
				{ (float)Frustum::Instance().realtimeSpecular, 0.0f, 0.0f, 0.0f }, // c11
				{ 1.0f, 0.0f, 0.0f, 0.0f },                                      // c12 (specularPower)
				{ timer, 0.0f, 0.0f, 0.0f },                                     // c13
				{ 0.0f, 0.0f, 0.0f, 0.0f },                                      // c14
				{ 4.0f, 0.0f, 0.0f, 0.0f }                                       // c15 (reducePointLightIntensity)
			};
			ms_lpd3dDevice->SetPixelShaderConstantF(8, &c8_to_c15[0][0], 8);
		}

#ifndef __ANDROID__
		ms_lpd3dDevice->SetVertexShaderConstantF(4, (const float*)(&s_cachedViewProjMat), 4);
#endif
		ms_lpd3dDevice->SetVertexShaderConstantF(8, (const float*)(&OutDoor.m_MatLightViewProj), 4);
		STATEMANAGER.SetVertexShaderConstant(16, (const float*)(&OutDoor.m_MatLightViewProj2), 4);

		if (OutDoor.m_lpCharacterShadowMapTexture)
			STATEMANAGER.SetTexture(2, OutDoor.m_lpCharacterShadowMapTexture);
		if (OutDoor.m_lpCharacterShadowMapTexture4)
			STATEMANAGER.SetTexture(3, OutDoor.m_lpCharacterShadowMapTexture4);
	}

	switch (m_iRenderMode)
	{
	case RENDER_MODE_NORMAL:
		BeginDiffuseRender();
		RenderWithOneTexture();
		EndDiffuseRender();
		BeginOpacityRender();
		BlendRenderWithOneTexture();
		EndOpacityRender();
		break;
	case RENDER_MODE_BLEND:
		if (m_fAlphaValue == 1.0f)
		{
			BeginDiffuseRender();
			RenderWithOneTexture();
			EndDiffuseRender();
			BeginOpacityRender();
			BlendRenderWithOneTexture();
			EndOpacityRender();
		}
		else if (m_fAlphaValue > 0.0f)
		{
			BeginBlendRender();
			RenderWithOneTexture();
			BlendRenderWithOneTexture();
			EndBlendRender();
		}
		break;
	case RENDER_MODE_ADD:
		BeginAddRender();
		RenderWithOneTexture();
		BlendRenderWithOneTexture();
		EndAddRender();
		break;
	case RENDER_MODE_MODULATE:
		BeginModulateRender();
		RenderWithOneTexture();
		BlendRenderWithOneTexture();
		EndModulateRender();
		break;
	}

	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);

	if (bUseShadowShader)
	{
		STATEMANAGER.SetVertexShader(nullptr);
		STATEMANAGER.SetPixelShader(nullptr);
		STATEMANAGER.RestoreRenderState(D3DRS_FOGENABLE);
	}
	else
	{
		kMtrl.Diffuse = D3DXCOLOR(0xffffffff);
		STATEMANAGER.SetMaterial(&kMtrl);
	}

	if (ms_isDirLine)
	{
		D3DXVECTOR3 kD3DVt3Cur(m_x, m_y, m_z);

		D3DXVECTOR3 kD3DVt3LookDir(0.0f, -1.0f, 0.0f);
		D3DXMATRIX kD3DMatLook;
		D3DXMatrixRotationZ(&kD3DMatLook, D3DXToRadian(GetRotation()));
		D3DXVec3TransformCoord(&kD3DVt3LookDir, &kD3DVt3LookDir, &kD3DMatLook);
		D3DXVec3Scale(&kD3DVt3LookDir, &kD3DVt3LookDir, 200.0f);
		D3DXVec3Add(&kD3DVt3LookDir, &kD3DVt3LookDir, &kD3DVt3Cur);

		D3DXVECTOR3 kD3DVt3AdvDir(0.0f, -1.0f, 0.0f);
		D3DXMATRIX kD3DMatAdv;
		D3DXMatrixRotationZ(&kD3DMatAdv, D3DXToRadian(GetAdvancingRotation()));
		D3DXVec3TransformCoord(&kD3DVt3AdvDir, &kD3DVt3AdvDir, &kD3DMatAdv);
		D3DXVec3Scale(&kD3DVt3AdvDir, &kD3DVt3AdvDir, 200.0f);
		D3DXVec3Add(&kD3DVt3AdvDir, &kD3DVt3AdvDir, &kD3DVt3Cur);

		static CScreen s_kScreen;

		STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
		STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		STATEMANAGER.SaveRenderState(D3DRS_ZENABLE, FALSE);
		STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

		s_kScreen.SetDiffuseColor(1.0f, 1.0f, 0.0f);
		s_kScreen.RenderLine3d(kD3DVt3Cur.x, kD3DVt3Cur.y, kD3DVt3Cur.z, kD3DVt3AdvDir.x, kD3DVt3AdvDir.y, kD3DVt3AdvDir.z);

		s_kScreen.SetDiffuseColor(0.0f, 1.0f, 1.0f);
		s_kScreen.RenderLine3d(kD3DVt3Cur.x, kD3DVt3Cur.y, kD3DVt3Cur.z, kD3DVt3LookDir.x, kD3DVt3LookDir.y, kD3DVt3LookDir.z);

		STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);
		STATEMANAGER.RestoreRenderState(D3DRS_ZENABLE);

		STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
		STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);
		STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAOP);
		STATEMANAGER.RestoreFVF();
	}
}

void CActorInstance::BeginDiffuseRender()
{
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2,	D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2,	D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP,	D3DTOP_MODULATE);

	STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

void CActorInstance::EndDiffuseRender()
{
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
}

void CActorInstance::BeginOpacityRender()
{
	STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHAREF, 0);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);

	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2,	D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP,	D3DTOP_MODULATE);
}

void CActorInstance::EndOpacityRender()
{
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAREF);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);
}

void CActorInstance::BeginBlendRender()
{
	STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2,	D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, D3DXCOLOR(1.0f, 1.0f, 1.0f, m_fAlphaValue));
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
}

void CActorInstance::EndBlendRender()
{
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
	STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
}

void CActorInstance::BeginAddRender()
{
	STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, m_AddColor);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2,	D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2,	D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP,	D3DTOP_MODULATE);

	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1,	D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2,	D3DTA_TFACTOR);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP,	D3DTOP_ADD);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP,	D3DTOP_DISABLE);

	STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

void CActorInstance::EndAddRender()
{
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
}

void CActorInstance::RestoreRenderMode()
{
	m_iRenderMode = RENDER_MODE_NORMAL;
	if (m_kBlendAlpha.m_isBlending)
	{
		m_kBlendAlpha.m_iOldRenderMode = m_iRenderMode;
	}
}

void CActorInstance::SetAddRenderMode()
{
	m_iRenderMode = RENDER_MODE_ADD;
	if (m_kBlendAlpha.m_isBlending)
	{
		m_kBlendAlpha.m_iOldRenderMode = m_iRenderMode;
	}
}

void CActorInstance::SetRenderMode(int iRenderMode)
{
	m_iRenderMode = iRenderMode;
	if (m_kBlendAlpha.m_isBlending)
	{
		m_kBlendAlpha.m_iOldRenderMode = iRenderMode;
	}
}

void CActorInstance::SetAddColor(const D3DXCOLOR & c_rColor)
{
	m_AddColor = c_rColor;
	m_AddColor.a = 1.0f;
}

void CActorInstance::BeginModulateRender()
{
	STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, m_AddColor);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2,	D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP,	D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1,	D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2,	D3DTA_DIFFUSE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP,	D3DTOP_MODULATE);

	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1,	D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2,	D3DTA_TFACTOR);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP,	D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP,	D3DTOP_DISABLE);

	STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

void CActorInstance::EndModulateRender()
{
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
}

void CActorInstance::SetModulateRenderMode()
{
	m_iRenderMode = RENDER_MODE_MODULATE;
	if (m_kBlendAlpha.m_isBlending)
	{
		m_kBlendAlpha.m_iOldRenderMode = m_iRenderMode;
	}
}

void CActorInstance::RenderCollisionData()
{
	static CScreen s_Screen;

	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	if (m_pAttributeInstance)
	{
		for (DWORD col=0; col < GetCollisionInstanceCount(); ++col)
		{
			CBaseCollisionInstance * pInstance = GetCollisionInstanceData(col);
			pInstance->Render();
		}
	}

	STATEMANAGER.SetRenderState(D3DRS_ZENABLE, FALSE);
	s_Screen.SetColorOperation();
	s_Screen.SetDiffuseColor(1.0f, 0.0f, 0.0f);
	TCollisionPointInstanceList::iterator itor;

	s_Screen.SetDiffuseColor(1.0f, (isShow())?1.0f:0.0f, 0.0f);
	D3DXVECTOR3 center;
	float r;
	GetBoundingSphere(center,r);
	s_Screen.RenderCircle3d(center.x,center.y,center.z,r);

	s_Screen.SetDiffuseColor(0.0f, 0.0f, 1.0f);
	itor = m_DefendingPointInstanceList.begin();
	for (; itor != m_DefendingPointInstanceList.end(); ++itor)
	{
		const TCollisionPointInstance & c_rInstance = *itor;
		for (DWORD i = 0; i < c_rInstance.SphereInstanceVector.size(); ++i)
		{
			const CDynamicSphereInstance & c_rSphereInstance = c_rInstance.SphereInstanceVector[i];
			s_Screen.RenderCircle3d(c_rSphereInstance.v3Position.x,
									c_rSphereInstance.v3Position.y,
									c_rSphereInstance.v3Position.z,
									c_rSphereInstance.fRadius);
		}
	}

	s_Screen.SetDiffuseColor(0.0f, 1.0f, 0.0f);
	itor = m_BodyPointInstanceList.begin();
	for (; itor != m_BodyPointInstanceList.end(); ++itor)
	{
		const TCollisionPointInstance & c_rInstance = *itor;
		for (DWORD i = 0; i < c_rInstance.SphereInstanceVector.size(); ++i)
		{
			const CDynamicSphereInstance & c_rSphereInstance = c_rInstance.SphereInstanceVector[i];
			s_Screen.RenderCircle3d(c_rSphereInstance.v3Position.x,
									c_rSphereInstance.v3Position.y,
									c_rSphereInstance.v3Position.z,
									c_rSphereInstance.fRadius);
		}
	}

	s_Screen.SetDiffuseColor(1.0f, 0.0f, 0.0f);
	{
		CDynamicSphereInstanceVector::iterator itor = m_kSplashArea.SphereInstanceVector.begin();
		for (; itor != m_kSplashArea.SphereInstanceVector.end(); ++itor)
		{
			const CDynamicSphereInstance & c_rInstance = *itor;
			s_Screen.RenderCircle3d(c_rInstance.v3Position.x,
									c_rInstance.v3Position.y,
									c_rInstance.v3Position.z,
									c_rInstance.fRadius);
		}
	}

	STATEMANAGER.SetRenderState(D3DRS_ZENABLE, TRUE);
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);
}

void CActorInstance::RenderToShadowMap()
{
	if (RENDER_MODE_BLEND == m_iRenderMode)
	if (GetAlphaValue() < 0.5f)
		return;

	CGraphicThingInstance::RenderToShadowMap();

	if (m_pkHorse)
		m_pkHorse->RenderToShadowMap();
}