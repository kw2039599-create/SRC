#include "StdAfx.h"
#include "../eterLib/StateManager.h"
#include "../eterLib/GrpSubImage.h"
#include "../eterlib/Camera.h"
#include "../EterPack/EterPackManager.h"

#include "PythonMiniMap.h"
#include "PythonBackground.h"
#include "PythonCharacterManager.h"
#include "PythonGuild.h"

#include "AbstractPlayer.h"
#if defined(__BL_PARTY_POSITION__)
#include "PythonPlayer.h"
#endif
#if defined(ENABLE_NPC_LOCATION_HELPER)
#	include "../EffectLib/EffectManager.h"
#	include "../GameLib/GameUtil.h"
#	include "PythonNetworkStream.h"
#endif

#include "../eterPythonLib/PythonWindowManager.h"

void CPythonMiniMap::AddObserver(DWORD dwVID, float fSrcX, float fSrcY)
{
	std::map<DWORD, SObserver>::iterator f=m_kMap_dwVID_kObserver.find(dwVID);
	if (m_kMap_dwVID_kObserver.end()==f)
	{
		SObserver kObserver;
		kObserver.dwSrcTime=ELTimer_GetMSec();
		kObserver.dwDstTime=kObserver.dwSrcTime+1000;
		kObserver.fSrcX=fSrcX;
		kObserver.fSrcY=fSrcY;
		kObserver.fDstX=fSrcX;
		kObserver.fDstY=fSrcY;
		kObserver.fCurX=fSrcX;
		kObserver.fCurY=fSrcY;
		m_kMap_dwVID_kObserver.insert(std::map<DWORD, SObserver>::value_type(dwVID, kObserver));
	}
	else
	{
		SObserver& rkObserver=f->second;
		rkObserver.dwSrcTime=ELTimer_GetMSec();
		rkObserver.dwDstTime=rkObserver.dwSrcTime+1000;
		rkObserver.fSrcX=fSrcX;
		rkObserver.fSrcY=fSrcY;
		rkObserver.fDstX=fSrcX;
		rkObserver.fDstY=fSrcY;
		rkObserver.fCurX=fSrcX;
		rkObserver.fCurY=fSrcY;
	}
}

void CPythonMiniMap::MoveObserver(DWORD dwVID, float fDstX, float fDstY)
{
	std::map<DWORD, SObserver>::iterator f=m_kMap_dwVID_kObserver.find(dwVID);
	if (m_kMap_dwVID_kObserver.end()==f)
		return;

	SObserver& rkObserver=f->second;
	rkObserver.dwSrcTime=ELTimer_GetMSec();
	rkObserver.dwDstTime=rkObserver.dwSrcTime+1000;
	rkObserver.fSrcX=rkObserver.fCurX;
	rkObserver.fSrcY=rkObserver.fCurY;
	rkObserver.fDstX=fDstX;
	rkObserver.fDstY=fDstY;
}

void CPythonMiniMap::RemoveObserver(DWORD dwVID)
{
	m_kMap_dwVID_kObserver.erase(dwVID);
}

void CPythonMiniMap::SetCenterPosition(float fCenterX, float fCenterY)
{
	m_fCenterX = fCenterX;
	m_fCenterY = fCenterY;

	CMapOutdoor& rkMap = CPythonBackground::Instance().GetMapOutdoorRef();
	for (BYTE byTerrainNum = 0; byTerrainNum < AROUND_AREA_NUM; ++byTerrainNum)
	{
		m_lpMiniMapTexture[byTerrainNum] = NULL;
		CTerrain * pTerrain;
		if (rkMap.GetTerrainPointer(byTerrainNum, &pTerrain))
			m_lpMiniMapTexture[byTerrainNum] = pTerrain->GetMiniMapTexture();
	}

	const TOutdoorMapCoordinate & rOutdoorMapCoord = rkMap.GetCurCoordinate();

	m_fCenterCellX = (m_fCenterX - (float)(rOutdoorMapCoord.m_sTerrainCoordX * CTerrainImpl::TERRAIN_XSIZE)) / (float)(CTerrainImpl::CELLSCALE);
	m_fCenterCellY = (m_fCenterY - (float)(rOutdoorMapCoord.m_sTerrainCoordY * CTerrainImpl::TERRAIN_YSIZE)) / (float)(CTerrainImpl::CELLSCALE);

	__SetPosition();
}

void CPythonMiniMap::Update(float fCenterX, float fCenterY)
{
	CPythonBackground& rkBG=CPythonBackground::Instance();
	if (!rkBG.IsMapOutdoor())
		return;

	if (m_fCenterX != fCenterX || m_fCenterY != fCenterY )
		SetCenterPosition(fCenterX, fCenterY);

	m_OtherPCPositionVector.clear();
	m_PartyPCPositionVector.clear();
	m_NPCPositionVector.clear();
	m_MonsterPositionVector.clear();
	m_WarpPositionVector.clear();

	float fooCellScale = 1.0f / ((float) CTerrainImpl::CELLSCALE);

	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();

	CInstanceBase* pkInstMain=rkChrMgr.GetMainInstancePtr();
	if (!pkInstMain)
		return;

#if defined(__BL_PARTY_POSITION__)
	DWORD dwPID = 0;
	const float c_fMiniMapWindowRadius = 54.0f;
	if (pkInstMain->IsPartyMember())
	{
		CPythonPlayer::Instance().PartyMemberVIDToPID(pkInstMain->GetVirtualID(), &dwPID);

		if (CPythonPlayer::Instance().IsPartyPositionExist(dwPID))
		{
			const size_t iPartyMemberCount = CPythonPlayer::Instance().GetPartyMemberCount();
			if (iPartyMemberCount > 0)
			{
				for (size_t i = 0; i < iPartyMemberCount; ++i)
				{
					const DWORD dwPartyMemberPID = CPythonPlayer::Instance().GetPartyMemberPIDByIndex(i);
					if (dwPID != dwPartyMemberPID)
					{
						D3DXVECTOR2 v2Position = { 0.0f, 0.0f };
						CPythonPlayer::Instance().GetPartyMemberPosition(dwPartyMemberPID, &v2Position);
						if (v2Position.x != 0.0 && v2Position.y != 0.0 && v2Position.x >= 0.0 && m_fAtlasMaxX >= v2Position.x && v2Position.y >= 0.0 && m_fAtlasMaxY >= v2Position.y)
						{
							float fDistanceFromCenterX = (v2Position.x - m_fCenterX) * fooCellScale * m_fScale;
							float fDistanceFromCenterY = (v2Position.y - m_fCenterY) * fooCellScale * m_fScale;

							float fDistanceFromCenter = sqrtf(fDistanceFromCenterX * fDistanceFromCenterX + fDistanceFromCenterY * fDistanceFromCenterY);
							if (c_fMiniMapWindowRadius <= fDistanceFromCenter)
							{
								float fRadianX = acosf(fDistanceFromCenterX / fDistanceFromCenter);
								float fRadianY = asinf(fDistanceFromCenterY / fDistanceFromCenter);
								fDistanceFromCenterX = c_fMiniMapWindowRadius * cosf(fRadianX);
								fDistanceFromCenterY = c_fMiniMapWindowRadius * sinf(fRadianY);
							}

							TMarkPosition aMarkPosition;

							aMarkPosition.m_fX = (m_fWidth - (float)m_WhiteMark.GetWidth()) / 2.0f + fDistanceFromCenterX + m_fScreenX;
							aMarkPosition.m_fY = (m_fHeight - (float)m_WhiteMark.GetHeight()) / 2.0f + fDistanceFromCenterY + m_fScreenY;

							CPythonPlayer::Instance().UpdatePartyMemberMinimapPosition(dwPartyMemberPID, aMarkPosition.m_fX + 1.5f, aMarkPosition.m_fY + 1.5f);
							m_PartyPCPositionVector.push_back(aMarkPosition);
						}
					}
				}
			}
		}
	}
#endif

	CPythonCharacterManager::CharacterIterator i;
	for(i = rkChrMgr.CharacterInstanceBegin(); i!=rkChrMgr.CharacterInstanceEnd(); ++i)
	{
		CInstanceBase* pkInstEach=*i;

		TPixelPosition kInstancePosition;
		pkInstEach->NEW_GetPixelPosition(&kInstancePosition);
		float fDistanceFromCenterX = (kInstancePosition.x - m_fCenterX) * fooCellScale * m_fScale;
		float fDistanceFromCenterY = (kInstancePosition.y - m_fCenterY) * fooCellScale * m_fScale;
		if (fabs(fDistanceFromCenterX) >= m_fMiniMapRadius || fabs(fDistanceFromCenterY) >= m_fMiniMapRadius)
			continue;

#ifndef ENABLE_OPENGL
		float fDistanceFromCenter = sqrtf(fDistanceFromCenterX * fDistanceFromCenterX + fDistanceFromCenterY * fDistanceFromCenterY );
		if ( fDistanceFromCenter >= m_fMiniMapRadius )
			continue;
#endif

		TMarkPosition aMarkPosition;

		if (pkInstEach->IsPC() && !pkInstEach->IsInvisibility())
		{
			if (pkInstEach == CPythonCharacterManager::Instance().GetMainInstancePtr())
				continue;

			aMarkPosition.m_fX = ( m_fWidth - (float)m_WhiteMark.GetWidth() ) / 2.0f + fDistanceFromCenterX + m_fScreenX;
			aMarkPosition.m_fY = ( m_fHeight - (float)m_WhiteMark.GetHeight() ) / 2.0f + fDistanceFromCenterY + m_fScreenY;
			aMarkPosition.m_eNameColor=pkInstEach->GetNameColorIndex();
#if defined(__BL_PARTY_POSITION__)
			if (aMarkPosition.m_eNameColor == CInstanceBase::NAMECOLOR_PARTY)
			{
				if (pkInstMain->IsPartyMember())
				{
					DWORD dwPartyMemberPID = 0;
					if (CPythonPlayer::Instance().PartyMemberVIDToPID(pkInstEach->GetVirtualID(), &dwPartyMemberPID))
					{
						if (CPythonPlayer::Instance().IsPartyPositionExist(dwPID))
						{
							CPythonPlayer::Instance().UpdatePartyMemberPosition(dwPartyMemberPID, kInstancePosition.x, kInstancePosition.y);
						}
						else
						{
							CPythonPlayer::Instance().UpdatePartyMemberPosition(dwPartyMemberPID, 0, 0);
							m_PartyPCPositionVector.push_back(aMarkPosition);
						}
					}
				}
				else
					m_PartyPCPositionVector.push_back(aMarkPosition);
			}
			else
				m_OtherPCPositionVector.push_back(aMarkPosition);
#else
			if (aMarkPosition.m_eNameColor == CInstanceBase::NAMECOLOR_PARTY)
				m_PartyPCPositionVector.push_back(aMarkPosition);
			else
				m_OtherPCPositionVector.push_back(aMarkPosition);
#endif
		}
#if defined(ENABLE_EVENT_BANNER_FLAG)
		else if (pkInstEach->IsNPC() && !pkInstEach->IsBannerFlag())
#else
		else if (pkInstEach->IsNPC())
#endif
		{
			aMarkPosition.m_fX = ( m_fWidth - (float)m_WhiteMark.GetWidth() ) / 2.0f + fDistanceFromCenterX + m_fScreenX;
			aMarkPosition.m_fY = ( m_fHeight - (float)m_WhiteMark.GetHeight() ) / 2.0f + fDistanceFromCenterY + m_fScreenY;

			m_NPCPositionVector.push_back(aMarkPosition);
		}
		else if (pkInstEach->IsEnemy())
		{
			aMarkPosition.m_fX = ( m_fWidth - (float)m_WhiteMark.GetWidth() ) / 2.0f + fDistanceFromCenterX + m_fScreenX;
			aMarkPosition.m_fY = ( m_fHeight - (float)m_WhiteMark.GetHeight() ) / 2.0f + fDistanceFromCenterY + m_fScreenY;

			m_MonsterPositionVector.push_back(aMarkPosition);
		}
		else if (pkInstEach->IsWarp())
		{
			aMarkPosition.m_fX = ( m_fWidth - (float)m_WhiteMark.GetWidth() ) / 2.0f + fDistanceFromCenterX + m_fScreenX;
			aMarkPosition.m_fY = ( m_fHeight - (float)m_WhiteMark.GetHeight() ) / 2.0f + fDistanceFromCenterY + m_fScreenY;

			m_WarpPositionVector.push_back(aMarkPosition);
		}
	}

	{
		DWORD dwCurTime=ELTimer_GetMSec();

		std::map<DWORD, SObserver>::iterator i;
		for (i=m_kMap_dwVID_kObserver.begin(); i!=m_kMap_dwVID_kObserver.end(); ++i)
		{
			SObserver& rkObserver=i->second;

			float fPos=float(dwCurTime-rkObserver.dwSrcTime)/float(rkObserver.dwDstTime-rkObserver.dwSrcTime);
			if (fPos<0.0f) fPos=0.0f;
			else if (fPos>1.0f) fPos=1.0f;

			rkObserver.fCurX=(rkObserver.fDstX-rkObserver.fSrcX)*fPos+rkObserver.fSrcX;
			rkObserver.fCurY=(rkObserver.fDstY-rkObserver.fSrcY)*fPos+rkObserver.fSrcY;

			TPixelPosition kInstancePosition;
			kInstancePosition.x=rkObserver.fCurX;
			kInstancePosition.y=rkObserver.fCurY;
			kInstancePosition.z=0.0f;

			float fDistanceFromCenterX = (kInstancePosition.x - m_fCenterX) * fooCellScale * m_fScale;
			float fDistanceFromCenterY = (kInstancePosition.y - m_fCenterY) * fooCellScale * m_fScale;
			if (fabs(fDistanceFromCenterX) >= m_fMiniMapRadius || fabs(fDistanceFromCenterY) >= m_fMiniMapRadius)
				continue;
#ifndef ENABLE_OPENGL
			float fDistanceFromCenter = sqrtf(fDistanceFromCenterX * fDistanceFromCenterX + fDistanceFromCenterY * fDistanceFromCenterY );
			if ( fDistanceFromCenter >= m_fMiniMapRadius )
				continue;
#endif
			TMarkPosition aMarkPosition;
			aMarkPosition.m_fX = ( m_fWidth - (float)m_WhiteMark.GetWidth() ) / 2.0f + fDistanceFromCenterX + m_fScreenX;
			aMarkPosition.m_fY = ( m_fHeight - (float)m_WhiteMark.GetHeight() ) / 2.0f + fDistanceFromCenterY + m_fScreenY;
			aMarkPosition.m_eNameColor=CInstanceBase::NAMECOLOR_PARTY;
			m_PartyPCPositionVector.push_back(aMarkPosition);
		}
	}

	{
		TAtlasMarkInfoVector::iterator itor = m_AtlasWayPointInfoVector.begin();
		for (; itor != m_AtlasWayPointInfoVector.end(); ++itor)
		{
			TAtlasMarkInfo & rAtlasMarkInfo = *itor;

			if (TYPE_TARGET != rAtlasMarkInfo.m_byType)
				continue;

			if (0 != rAtlasMarkInfo.m_dwChrVID)
			{
				CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(rAtlasMarkInfo.m_dwChrVID);
				if (pInstance)
				{
					TPixelPosition kPixelPosition;
					pInstance->NEW_GetPixelPosition(&kPixelPosition);
					__UpdateWayPoint(&rAtlasMarkInfo, kPixelPosition.x, kPixelPosition.y);
				}
			}

#if !defined(__BL_PARTY_POSITION__)
			const float c_fMiniMapWindowRadius = 55.0f;
#endif

			float fDistanceFromCenterX = (rAtlasMarkInfo.m_fX - m_fCenterX) * fooCellScale * m_fScale;
			float fDistanceFromCenterY = (rAtlasMarkInfo.m_fY - m_fCenterY) * fooCellScale * m_fScale;
			float fDistanceFromCenter = sqrtf(fDistanceFromCenterX * fDistanceFromCenterX + fDistanceFromCenterY * fDistanceFromCenterY );

			if (fDistanceFromCenter >= c_fMiniMapWindowRadius)
			{
				float fRadianX = acosf(fDistanceFromCenterX / fDistanceFromCenter);
				float fRadianY = asinf(fDistanceFromCenterY / fDistanceFromCenter);
#if defined(__BL_PARTY_POSITION__)
				fDistanceFromCenterX = c_fMiniMapWindowRadius * cosf(fRadianX);
				fDistanceFromCenterY = c_fMiniMapWindowRadius * sinf(fRadianY);
#else
				fDistanceFromCenterX = 55.0f * cosf(fRadianX);
				fDistanceFromCenterY = 55.0f * sinf(fRadianY);
#endif
				rAtlasMarkInfo.m_fMiniMapX = ( m_fWidth - (float)m_WhiteMark.GetWidth() ) / 2.0f + fDistanceFromCenterX + m_fScreenX + 2.0f;
				rAtlasMarkInfo.m_fMiniMapY = ( m_fHeight - (float)m_WhiteMark.GetHeight() ) / 2.0f + fDistanceFromCenterY + m_fScreenY + 2.0f;
			}
			else
			{
				rAtlasMarkInfo.m_fMiniMapX = ( m_fWidth - (float)m_WhiteMark.GetWidth() ) / 2.0f + fDistanceFromCenterX + m_fScreenX;
				rAtlasMarkInfo.m_fMiniMapY = ( m_fHeight - (float)m_WhiteMark.GetHeight() ) / 2.0f + fDistanceFromCenterY + m_fScreenY;
#if defined(ENABLE_NPC_LOCATION_HELPER)
				__UpdateNPCLocationDirectionEffect();
#endif
			}
		}
	}
}

void CPythonMiniMap::Render(float fScreenX, float fScreenY)
{
	CPythonBackground& rkBG=CPythonBackground::Instance();
	if (!rkBG.IsMapOutdoor())
		return;

	if (!m_bShow)
		return;

	if (!rkBG.IsMapReady())
		return;

	if (m_fScreenX != fScreenX || m_fScreenY != fScreenY)
	{
		m_fScreenX = fScreenX;
		m_fScreenY = fScreenY;
		__SetPosition();
	}

#ifdef DIRECTX9
	STATEMANAGER.SaveSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
	STATEMANAGER.SaveSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	STATEMANAGER.SaveSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	STATEMANAGER.SaveSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	STATEMANAGER.SaveSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
#else
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_ANISOTROPIC);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_ANISOTROPIC);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_ANISOTROPIC);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
#endif

	STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
	STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
#ifdef DIRECTX9
	STATEMANAGER.SaveSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	STATEMANAGER.SaveSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
#else
	STATEMANAGER.SaveTextureStageState(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	STATEMANAGER.SaveTextureStageState(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
#endif

	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	STATEMANAGER.SaveTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SaveTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	STATEMANAGER.SaveTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SaveTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SaveTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	STATEMANAGER.SaveTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	STATEMANAGER.SaveRenderState(D3DRS_TEXTUREFACTOR, 0xFF000000);

	STATEMANAGER.SetTexture(1, m_MiniMapFilterGraphicImageInstance.GetTexturePointer()->GetD3DTexture());
	STATEMANAGER.SetTransform(D3DTS_TEXTURE1, &m_matMiniMapCover);

#ifdef DIRECTX9
	STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);
#else
	STATEMANAGER.SetVertexShader(D3DFVF_XYZ | D3DFVF_TEX1);
#endif
	STATEMANAGER.SetStreamSource(0, m_VertexBuffer.GetD3DVertexBuffer(), 20);
	STATEMANAGER.SetIndices(m_IndexBuffer.GetD3DIndexBuffer(), 0);
	STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matWorld);

	for (BYTE byTerrainNum = 0; byTerrainNum < AROUND_AREA_NUM; ++byTerrainNum)
	{
		LPDIRECT3DTEXTURE8 pMiniMapTexture = m_lpMiniMapTexture[byTerrainNum];
		STATEMANAGER.SetTexture(0, pMiniMapTexture);
		if (pMiniMapTexture)
		{
			CStateManager& rkSttMgr=CStateManager::Instance();
			rkSttMgr.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, byTerrainNum * 4, 4, byTerrainNum * 6, 2);
		}
		else
		{
			STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
			STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, byTerrainNum * 4, 4, byTerrainNum * 6, 2);
			STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		}
	}

	STATEMANAGER.RestoreRenderState(D3DRS_TEXTUREFACTOR);

	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_ALPHAARG2);
	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_ALPHAARG1);
	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_ALPHAOP);
	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_COLORARG1);
	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_COLORARG2);
	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_COLOROP);

	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG2);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG1);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAOP);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG2);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);

#ifdef DIRECTX9
	STATEMANAGER.RestoreSamplerState(0, D3DSAMP_ADDRESSU);
	STATEMANAGER.RestoreSamplerState(0, D3DSAMP_ADDRESSV);
#else
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ADDRESSU);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ADDRESSV);
#endif
	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXCOORDINDEX);
	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS);
#ifdef DIRECTX9
	STATEMANAGER.RestoreSamplerState(1, D3DSAMP_ADDRESSU);
	STATEMANAGER.RestoreSamplerState(1, D3DSAMP_ADDRESSV);
#else
	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_ADDRESSU);
	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_ADDRESSV);
#endif

	SetDiffuseOperation();
	STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matIdentity);

	STATEMANAGER.SaveRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);

	TInstancePositionVectorIterator aIterator;

	if (m_fScale >= 2.0f)
	{
		// Monster
		STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_MOB));//m_MarkTypeToColorMap[TYPE_MONSTER]);
		aIterator = m_MonsterPositionVector.begin();
		while (aIterator != m_MonsterPositionVector.end())
		{
			TMarkPosition & rPosition = *aIterator;
			m_WhiteMark.SetPosition(rPosition.m_fX, rPosition.m_fY);
			m_WhiteMark.Render();
			++aIterator;
		}

		// Other PC
		aIterator = m_OtherPCPositionVector.begin();
		while (aIterator != m_OtherPCPositionVector.end())
		{
			TMarkPosition & rPosition = *aIterator;
			STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, CInstanceBase::GetIndexedNameColor(rPosition.m_eNameColor));
			m_WhiteMark.SetPosition(rPosition.m_fX, rPosition.m_fY);
			m_WhiteMark.Render();
			++aIterator;
		}
#if !defined(__BL_PARTY_POSITION__)
		// Party PC
		if (!m_PartyPCPositionVector.empty())
		{
			float v = (1+sinf(CTimer::Instance().GetCurrentSecond()*6))/5+0.6;
			D3DXCOLOR c(CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_PARTY));//(m_MarkTypeToColorMap[TYPE_PARTY]);
			D3DXCOLOR d(v,v,v,1);
			D3DXColorModulate(&c,&c,&d);
			STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, (DWORD)c);
			aIterator = m_PartyPCPositionVector.begin();
			while (aIterator != m_PartyPCPositionVector.end())
			{
				TMarkPosition & rPosition = *aIterator;
				m_WhiteMark.SetPosition(rPosition.m_fX, rPosition.m_fY);
				m_WhiteMark.Render();
				++aIterator;
			}
		}
#endif
	}

#if defined(__BL_PARTY_POSITION__)
	// Party PC
	if (!m_PartyPCPositionVector.empty())
	{
		float v = (1 + sinf(CTimer::Instance().GetCurrentSecond() * 6)) / 5 + 0.8;
		D3DXCOLOR c(CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_PARTY));//(m_MarkTypeToColorMap[TYPE_PARTY]);
		D3DXCOLOR d(v, v, v, 1);
		D3DXColorModulate(&c, &c, &d);
		STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, (DWORD)c);
		aIterator = m_PartyPCPositionVector.begin();
		while (aIterator != m_PartyPCPositionVector.end())
		{
			TMarkPosition& rPosition = *aIterator;
			m_WhiteMark.SetPosition(rPosition.m_fX, rPosition.m_fY);
			m_WhiteMark.Render();
			++aIterator;
		}
	}
#endif

	// NPC
	STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_NPC));
	aIterator = m_NPCPositionVector.begin();
	while (aIterator != m_NPCPositionVector.end())
	{
		TMarkPosition & rPosition = *aIterator;
		m_WhiteMark.SetPosition(rPosition.m_fX, rPosition.m_fY);
		m_WhiteMark.Render();
		++aIterator;
	}

	// Warp
	STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_WARP));
	aIterator = m_WarpPositionVector.begin();
	while (aIterator != m_WarpPositionVector.end())
	{
		TMarkPosition & rPosition = *aIterator;
		m_WhiteMark.SetPosition(rPosition.m_fX, rPosition.m_fY);
		m_WhiteMark.Render();
		++aIterator;
	}

	STATEMANAGER.RestoreRenderState(D3DRS_TEXTUREFACTOR);

	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG2);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG1);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAOP);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG2);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);

#ifdef DIRECTX9
	STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MIPFILTER);
	STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MINFILTER);
	STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MAGFILTER);

	STATEMANAGER.SaveSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	STATEMANAGER.SaveSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
#else
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_MIPFILTER);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_MINFILTER);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_MAGFILTER);

	STATEMANAGER.SaveTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_ANISOTROPIC);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_ANISOTROPIC);
#endif

	CInstanceBase * pkInst = CPythonCharacterManager::Instance().GetMainInstancePtr();

	if (pkInst)
	{
		float fRotation;
		fRotation = (540.0f - pkInst->GetRotation());
		while(fRotation > 360.0f)
			fRotation -= 360.0f;
		while(fRotation < 0.0f)
			fRotation += 360.0f;

		m_PlayerMark.SetRotation(fRotation);
		m_PlayerMark.Render();
	}

	// Target
	{
		TAtlasMarkInfoVector::iterator itor = m_AtlasWayPointInfoVector.begin();
		for (; itor != m_AtlasWayPointInfoVector.end(); ++itor)
		{
			TAtlasMarkInfo & rAtlasMarkInfo = *itor;

			if (TYPE_TARGET != rAtlasMarkInfo.m_byType)
				continue;
			if (rAtlasMarkInfo.m_fMiniMapX <= 0.0f)
				continue;
			if (rAtlasMarkInfo.m_fMiniMapY <= 0.0f)
				continue;

			__RenderTargetMark(rAtlasMarkInfo.m_fMiniMapX, rAtlasMarkInfo.m_fMiniMapY);
		}
	}

	CCamera* pkCmrCur=CCameraManager::Instance().GetCurrentCamera();

	if (pkCmrCur)
	{
		m_MiniMapCameraraphicImageInstance.SetRotation(pkCmrCur->GetRoll());
		m_MiniMapCameraraphicImageInstance.Render();
	}
#ifdef DIRECTX9
	STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MINFILTER);
	STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MAGFILTER);
#else
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_MINFILTER);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_MAGFILTER);
#endif
}

void CPythonMiniMap::SetScale(float fScale)
{
	if (fScale >= 4.0f)
		fScale = 4.0f;
	if (fScale <= 0.5f)
		fScale = 0.5f;
	m_fScale = fScale;

	__SetPosition();
}

void CPythonMiniMap::ScaleUp()
{
	m_fScale *= 2.0f;
	if (m_fScale >= 4.0f)
		m_fScale = 4.0f;
	__SetPosition();
}

void CPythonMiniMap::ScaleDown()
{
	m_fScale *= 0.5f;
	if (m_fScale <= 0.5f)
		m_fScale = 0.5f;
	__SetPosition();
}

void CPythonMiniMap::SetMiniMapSize(float fWidth, float fHeight)
{
	m_fWidth = fWidth;
	m_fHeight = fHeight;
}

#pragma pack(push)
#pragma pack(1)
typedef struct _MINIMAPVERTEX
{
    float x, y, z;          // position
    float u, v;       // normal
} MINIMAPVERTEX, *LPMINIMAPVERTEX;
#pragma pack(pop)

bool CPythonMiniMap::Create()
{
	const std::string strImageRoot = "D:/ymir work/ui/";
	#ifdef ENABLE_OPENGL
	const std::string strImageFilter = "mark/m_minimap_mask.tga";
#else
	const std::string strImageFilter = strImageRoot + "minimap_image_filter.dds";
#endif
	const std::string strImageCamera = strImageRoot + "minimap_camera.dds";
#ifdef ENABLE_MINIMAP_WHITEMARK_CIRCLE
	const std::string strPlayerMark = strImageRoot + "minimap/whitemark_circle.tga";
	const std::string strWhiteMark = strImageRoot + "minimap/whitemark_circle.tga";
#else
	const std::string strPlayerMark = strImageRoot + "minimap/playermark.sub";
	const std::string strWhiteMark = strImageRoot + "minimap/whitemark.sub";
#endif

	CGraphicImage * pImage = (CGraphicImage *) CResourceManager::Instance().GetResourcePointer(strImageFilter.c_str());
	m_MiniMapFilterGraphicImageInstance.SetImagePointer(pImage);
	pImage = (CGraphicImage *) CResourceManager::Instance().GetResourcePointer(strImageCamera.c_str());
	m_MiniMapCameraraphicImageInstance.SetImagePointer(pImage);

	m_matMiniMapCover._11 = 1.0f / ((float)m_MiniMapFilterGraphicImageInstance.GetWidth());
	m_matMiniMapCover._22 = 1.0f / ((float)m_MiniMapFilterGraphicImageInstance.GetHeight());
	m_matMiniMapCover._33 = 0.0f;

	CGraphicSubImage * pSubImage = (CGraphicSubImage *) CResourceManager::Instance().GetResourcePointer(strPlayerMark.c_str());
	m_PlayerMark.SetImagePointer(pSubImage);

	pSubImage = (CGraphicSubImage *) CResourceManager::Instance().GetResourcePointer(strWhiteMark.c_str());
	m_WhiteMark.SetImagePointer(pSubImage);

	char buf[256];
	for (int i = 0; i < MINI_WAYPOINT_IMAGE_COUNT; ++i)
	{
		sprintf(buf, "%sminimap/mini_waypoint%02d.sub", strImageRoot.c_str(), i+1);
		m_MiniWayPointGraphicImageInstances[i].SetImagePointer((CGraphicSubImage *) CResourceManager::Instance().GetResourcePointer(buf));
		m_MiniWayPointGraphicImageInstances[i].SetRenderingMode(CGraphicExpandedImageInstance::RENDERING_MODE_SCREEN);
	}
	for (int j = 0; j < WAYPOINT_IMAGE_COUNT; ++j)
	{
		sprintf(buf, "%sminimap/waypoint%02d.sub", strImageRoot.c_str(), j+1);
		m_WayPointGraphicImageInstances[j].SetImagePointer((CGraphicSubImage *) CResourceManager::Instance().GetResourcePointer(buf));
		m_WayPointGraphicImageInstances[j].SetRenderingMode(CGraphicExpandedImageInstance::RENDERING_MODE_SCREEN);
	}
	for (int k = 0; k < TARGET_MARK_IMAGE_COUNT; ++k)
	{
		sprintf(buf, "%sminimap/targetmark%02d.sub", strImageRoot.c_str(), k+1);
		m_TargetMarkGraphicImageInstances[k].SetImagePointer((CGraphicSubImage *) CResourceManager::Instance().GetResourcePointer(buf));
		m_TargetMarkGraphicImageInstances[k].SetRenderingMode(CGraphicExpandedImageInstance::RENDERING_MODE_SCREEN);
	}

	m_GuildAreaFlagImageInstance.SetImagePointer((CGraphicSubImage *) CResourceManager::Instance().GetResourcePointer("d:/ymir work/ui/minimap/GuildArea01.sub"));
#if defined(ENABLE_NPC_LOCATION_HELPER)
	for (int iMyPos = 0; iMyPos < ATLAS_HELPER_MY_POSITION_IMAGE_COUNT; ++iMyPos)
	{
		sprintf(buf, "d:/ymir work/ui/game/npc_location_helper/my_position_%02d.sub", iMyPos + 1);
		m_AtlasHelperMyPositionMarks[iMyPos].SetImagePointer(
			(CGraphicSubImage*)CResourceManager::Instance().GetResourcePointer(buf));
		m_AtlasHelperMyPositionMarks[iMyPos].SetRenderingMode(CGraphicExpandedImageInstance::RENDERING_MODE_SCREEN);
		m_AtlasHelperMyPositionMarks[iMyPos].SetDiffuseColor(0.55f, 0.85f, 1.0f, 1.0f);
		m_AtlasHelperMyPositionMarks[iMyPos].SetOrigin(0.0f, 0.0f);
	}
#endif
#pragma pack(push)
#pragma pack(1)
	LPMINIMAPVERTEX		lpMiniMapVertex;
	LPMINIMAPVERTEX		lpOrigMiniMapVertex;
#pragma pack(pop)

	if (!m_VertexBuffer.Create(36, D3DFVF_XYZ | D3DFVF_TEX1, D3DUSAGE_WRITEONLY, D3DPOOL_MANAGED) )
	{
		return false;
	}

	if (m_VertexBuffer.Lock((void **) &lpOrigMiniMapVertex))
	{
		char * pchMiniMapVertex = (char *)lpOrigMiniMapVertex;
		memset(pchMiniMapVertex, 0, sizeof(char) * 720);
		lpMiniMapVertex = (LPMINIMAPVERTEX) pchMiniMapVertex;

		for (int iY = -3; iY <= 1; ++iY)
		{
			if (0 == iY%2)
				continue;
			float fY = 0.5f * ((float)iY);
			for (int iX = -3; iX <= 1; ++iX)
			{
				if (0 == iX%2)
					continue;
				float fX = 0.5f * ((float)iX);
				lpMiniMapVertex = (LPMINIMAPVERTEX) pchMiniMapVertex;
				lpMiniMapVertex->x = fX;
				lpMiniMapVertex->y = fY;
				lpMiniMapVertex->z = 0.0f;
				lpMiniMapVertex->u = 0.0f;
				lpMiniMapVertex->v = 0.0f;
				pchMiniMapVertex += 20;
				lpMiniMapVertex = (LPMINIMAPVERTEX) pchMiniMapVertex;
				lpMiniMapVertex->x = fX;
				lpMiniMapVertex->y = fY + 1.0f;
				lpMiniMapVertex->z = 0.0f;
				lpMiniMapVertex->u = 0.0f;
				lpMiniMapVertex->v = 1.0f;
				pchMiniMapVertex += 20;
				lpMiniMapVertex = (LPMINIMAPVERTEX) pchMiniMapVertex;
				lpMiniMapVertex->x = fX + 1.0f;
				lpMiniMapVertex->y = fY;
				lpMiniMapVertex->z = 0.0f;
				lpMiniMapVertex->u = 1.0f;
				lpMiniMapVertex->v = 0.0f;
				pchMiniMapVertex += 20;
				lpMiniMapVertex = (LPMINIMAPVERTEX) pchMiniMapVertex;
				lpMiniMapVertex->x = fX + 1.0f;
				lpMiniMapVertex->y = fY + 1.0f;
				lpMiniMapVertex->z = 0.0f;
				lpMiniMapVertex->u = 1.0f;
				lpMiniMapVertex->v = 1.0f;
				pchMiniMapVertex += 20;
			}
		}

		m_VertexBuffer.Unlock();
	}

	if (!m_IndexBuffer.Create(54, D3DFMT_INDEX16))
	{
		return false;
	}

	WORD pwIndices[54] =
	{
		0, 1, 2, 2, 1, 3,
		4, 5, 6, 6, 5, 7,
		8, 9, 10, 10, 9, 11,

		12, 13, 14, 14, 13, 15,
		16, 17, 18, 18, 17, 19,
		20, 21, 22, 22, 21, 23,

		24, 25, 26, 26, 25, 27,
		28, 29, 30, 30, 29, 31,
		32, 33, 34, 34, 33, 35
	};

	void * pIndices;

	if (m_IndexBuffer.Lock(&pIndices))
	{
		memcpy(pIndices, pwIndices, 54 * sizeof(WORD));
		m_IndexBuffer.Unlock();
	}

	return true;
}

void CPythonMiniMap::__SetPosition()
{
	m_fMiniMapRadius = fMIN(6400.0f / ((float) CTerrainImpl::CELLSCALE) * m_fScale, 64.0f);

	m_matWorld._11 = m_fWidth * m_fScale;
	m_matWorld._22 = m_fHeight * m_fScale;
	m_matWorld._41 = (1.0f + m_fScale) * m_fWidth * 0.5f - m_fCenterCellX * m_fScale + m_fScreenX;
	m_matWorld._42 = (1.0f + m_fScale) * m_fHeight * 0.5f - m_fCenterCellY * m_fScale + m_fScreenY;

	if (!m_MiniMapFilterGraphicImageInstance.IsEmpty())
	{
		m_matMiniMapCover._41 = -(m_fScreenX) / ((float)m_MiniMapFilterGraphicImageInstance.GetWidth());
		m_matMiniMapCover._42 = -(m_fScreenY) / ((float)m_MiniMapFilterGraphicImageInstance.GetHeight());
	}

	if (!m_PlayerMark.IsEmpty())
		m_PlayerMark.SetPosition( ( m_fWidth - (float)m_PlayerMark.GetWidth() ) / 2.0f + m_fScreenX,
		( m_fHeight - (float)m_PlayerMark.GetHeight() ) / 2.0f  + m_fScreenY );

	if (!m_MiniMapCameraraphicImageInstance.IsEmpty())
		m_MiniMapCameraraphicImageInstance.SetPosition( ( m_fWidth - (float)m_MiniMapCameraraphicImageInstance.GetWidth() ) / 2.0f + m_fScreenX,
		( m_fHeight - (float)m_MiniMapCameraraphicImageInstance.GetHeight() ) / 2.0f  + m_fScreenY );
}

//////////////////////////////////////////////////////////////////////////
// Atlas

void CPythonMiniMap::ClearAtlasMarkInfo()
{
	m_AtlasNPCInfoVector.clear();
	m_AtlasWarpInfoVector.clear();
}

void CPythonMiniMap::RegisterAtlasMark(BYTE byType, const char * c_szName, long lx, long ly)
{
	TAtlasMarkInfo aAtlasMarkInfo;

	aAtlasMarkInfo.m_fX = float(lx);
	aAtlasMarkInfo.m_fY = float(ly);
	aAtlasMarkInfo.m_strText = c_szName;

	aAtlasMarkInfo.m_fScreenX = aAtlasMarkInfo.m_fX / m_fAtlasMaxX * m_fAtlasImageSizeX - (float)m_WhiteMark.GetWidth() / 2.0f;
	aAtlasMarkInfo.m_fScreenY = aAtlasMarkInfo.m_fY / m_fAtlasMaxY * m_fAtlasImageSizeY - (float)m_WhiteMark.GetHeight() / 2.0f;

	switch(byType)
	{
		case CActorInstance::TYPE_NPC:
			aAtlasMarkInfo.m_byType = TYPE_NPC;
			m_AtlasNPCInfoVector.push_back(aAtlasMarkInfo);
			break;
		case CActorInstance::TYPE_WARP:
			aAtlasMarkInfo.m_byType = TYPE_WARP;
			{
				int iPos = aAtlasMarkInfo.m_strText.find(" ");
				if (iPos >= 0)
					aAtlasMarkInfo.m_strText[iPos]=0;

			}
			m_AtlasWarpInfoVector.push_back(aAtlasMarkInfo);
			break;
	}
}

void CPythonMiniMap::ClearGuildArea()
{
	m_GuildAreaInfoVector.clear();
}

void CPythonMiniMap::RegisterGuildArea(DWORD dwID, DWORD dwGuildID, long x, long y, long width, long height)
{
	TGuildAreaInfo kGuildAreaInfo;
	kGuildAreaInfo.dwGuildID = dwGuildID;
	kGuildAreaInfo.lx = x;
	kGuildAreaInfo.ly = y;
	kGuildAreaInfo.lwidth = width;
	kGuildAreaInfo.lheight = height;
	m_GuildAreaInfoVector.push_back(kGuildAreaInfo);
}

DWORD CPythonMiniMap::GetGuildAreaID(DWORD x, DWORD y)
{
	TGuildAreaInfoVectorIterator itor = m_GuildAreaInfoVector.begin();
	for (; itor != m_GuildAreaInfoVector.end(); ++itor)
	{
		TGuildAreaInfo & rAreaInfo = *itor;

		if (x >= rAreaInfo.lx)
		if (y >= rAreaInfo.ly)
		if (x <= rAreaInfo.lx + rAreaInfo.lwidth)
		if (y <= rAreaInfo.ly + rAreaInfo.lheight)
		{
			return rAreaInfo.dwGuildID;
		}
	}

	return 0xffffffff;
}

void CPythonMiniMap::CreateTarget(int iID, const char * c_szName)
{
	AddWayPoint(TYPE_TARGET, iID, 0.0f, 0.0f, c_szName);
}

void CPythonMiniMap::UpdateTarget(int iID, int ix, int iy)
{
	TAtlasMarkInfo * pkInfo;
	if (!__GetWayPoint(iID, &pkInfo))
		return;

	if (0 != pkInfo->m_dwChrVID)
	{
		if (CPythonCharacterManager::Instance().GetInstancePtr(pkInfo->m_dwChrVID))
			return;
	}

	if (ix < m_dwAtlasBaseX)
		return;
	if (iy < m_dwAtlasBaseY)
		return;
	if (ix > m_dwAtlasBaseX+DWORD(m_fAtlasMaxX))
		return;
	if (iy > m_dwAtlasBaseY+DWORD(m_fAtlasMaxY))
		return;

	__UpdateWayPoint(pkInfo, ix-int(m_dwAtlasBaseX), iy-int(m_dwAtlasBaseY));
}

void CPythonMiniMap::CreateTarget(int iID, const char * c_szName, DWORD dwVID)
{
	AddWayPoint(TYPE_TARGET, iID, 0.0f, 0.0f, c_szName, dwVID);
}

void CPythonMiniMap::DeleteTarget(int iID)
{
	RemoveWayPoint(iID);
}

#define ENABLE_NEW_ATLAS_MARK_INFO
#ifdef ENABLE_NEW_ATLAS_MARK_INFO
	#include "PythonNonPlayer.h"
#endif
void CPythonMiniMap::__LoadAtlasMarkInfo()
{
	ClearAtlasMarkInfo();
	ClearGuildArea();

	CPythonBackground & rkBG=CPythonBackground::Instance();
	if (!rkBG.IsMapOutdoor())
		return;

	CMapOutdoor & rkMap=rkBG.GetMapOutdoorRef();

	char szAtlasMarkInfoFileName[256]; 
	std::string strMapName = rkMap.GetName();

#if defined(ENABLE_NPC_LOCATION_HELPER)
	if (!m_strAtlasHelperDisplayMapName.empty())
		strMapName = m_strAtlasHelperDisplayMapName;
#endif

	CTokenVectorMap stTokenVectorMap;

	// --- FIX: Strictly use the STANDARD path. C++ will fail if it tries the helper path! ---
	_snprintf(szAtlasMarkInfoFileName, sizeof(szAtlasMarkInfoFileName), "%s/map/%s_point.txt", LocaleService_GetLocalePath(), strMapName.c_str());
	if (!LoadMultipleTextData(szAtlasMarkInfoFileName, stTokenVectorMap))
	{
		Tracef(" CPythonMiniMap::__LoadAtlasMarkInfo File Load %s ERROR\n", szAtlasMarkInfoFileName);
		return;
	}

	const std::string strType[TYPE_COUNT] = { "OPC", "OPCPVP", "OPCPVPSELF", "NPC", "MONSTER", "WARP", "WAYPOINT" };

	for (DWORD i = 0; i < stTokenVectorMap.size(); ++i)
	{
		char szMarkInfoName[32+1];
		_snprintf(szMarkInfoName, sizeof(szMarkInfoName), "%d", i);

		if (stTokenVectorMap.end() == stTokenVectorMap.find(szMarkInfoName))
			continue;

		const CTokenVector & rVector = stTokenVectorMap[szMarkInfoName];

		TAtlasMarkInfo aAtlasMarkInfo;
#ifdef ENABLE_NEW_ATLAS_MARK_INFO
		if (rVector.size() == 3)
		{
			const std::string & c_rstrType = strType[3]; 
			const std::string & c_rstrPositionX = rVector[0].c_str();
			const std::string & c_rstrPositionY = rVector[1].c_str();
			const std::string & c_rstrText = rVector[2].c_str();
			int iVNum = atoi(c_rstrText.c_str());

			aAtlasMarkInfo.m_fX = atof(c_rstrPositionX.c_str());
			aAtlasMarkInfo.m_fY = atof(c_rstrPositionY.c_str());
			aAtlasMarkInfo.m_strText = CPythonNonPlayer::Instance().GetMonsterName(iVNum);
		}
		else
#endif
		{
			const std::string & c_rstrType = rVector[0].c_str();
			const std::string & c_rstrPositionX = rVector[1].c_str();
			const std::string & c_rstrPositionY = rVector[2].c_str();
			const std::string & c_rstrText = rVector[3].c_str();
			for (int j = 0; j < TYPE_COUNT; ++j)
			{
				if (0 == c_rstrType.compare(strType[j]))
					aAtlasMarkInfo.m_byType = (BYTE)j;
			}

			aAtlasMarkInfo.m_fX = atof(c_rstrPositionX.c_str());
			aAtlasMarkInfo.m_fY = atof(c_rstrPositionY.c_str());
			aAtlasMarkInfo.m_strText = c_rstrText;
		}

		aAtlasMarkInfo.m_fScreenX = aAtlasMarkInfo.m_fX / m_fAtlasMaxX * m_fAtlasImageSizeX - (float)m_WhiteMark.GetWidth() / 2.0f;
		aAtlasMarkInfo.m_fScreenY = aAtlasMarkInfo.m_fY / m_fAtlasMaxY * m_fAtlasImageSizeY - (float)m_WhiteMark.GetHeight() / 2.0f;

		switch (aAtlasMarkInfo.m_byType)
		{
			case TYPE_NPC:
				m_AtlasNPCInfoVector.push_back(aAtlasMarkInfo);
				break;
			case TYPE_WARP:
				m_AtlasWarpInfoVector.push_back(aAtlasMarkInfo);
				break;
		}
	}
}

#if defined(ENABLE_NPC_LOCATION_HELPER)
namespace
{
void BuildAtlasSubFileName(char* atlasFileName, size_t cbAtlasFileName, const char* pszMapName, bool bNpcHelperResize)
{
	if (!atlasFileName || cbAtlasFileName == 0 || !pszMapName)
		return;

	atlasFileName[0] = '\0';

	if (bNpcHelperResize)
	{
		snprintf(atlasFileName, cbAtlasFileName, "d:/ymir work/ui/atlas_resize/%s/atlas.sub", pszMapName);
		if (CEterPackManager::Instance().isExist(atlasFileName))
			return;
	}

	snprintf(atlasFileName, cbAtlasFileName, "%s/atlas.sub", pszMapName);
	if (CEterPackManager::Instance().isExist(atlasFileName))
		return;

	snprintf(atlasFileName, cbAtlasFileName, "d:/ymir work/ui/atlas/%s/atlas.sub", pszMapName);
}
}
#endif

bool CPythonMiniMap::LoadAtlas()
{
	CPythonBackground& rkBG=CPythonBackground::Instance();
	if (!rkBG.IsMapOutdoor())
		return false;

	CMapOutdoor& rkMap=rkBG.GetMapOutdoorRef();
	const char* playerMarkFileName = "d:/ymir work/ui/minimap/playermark.sub";
	char atlasFileName[1024+1];

#if defined(ENABLE_NPC_LOCATION_HELPER)
	BuildAtlasSubFileName(atlasFileName, sizeof(atlasFileName), rkMap.GetName().c_str(), m_bAtlasNpcHelperView);
#else
	snprintf(atlasFileName, sizeof(atlasFileName), "%s/atlas.sub", rkMap.GetName().c_str());
	if (!CEterPackManager::Instance().isExist(atlasFileName))
	{
		snprintf(atlasFileName, sizeof(atlasFileName), "d:/ymir work/ui/atlas/%s/atlas.sub", rkMap.GetName().c_str());
	}
#endif

	m_AtlasImageInstance.Destroy();
	m_AtlasPlayerMark.Destroy();
	CGraphicImage* pkGrpImgAtlas = (CGraphicImage *) CResourceManager::Instance().GetResourcePointer(atlasFileName);
	if (pkGrpImgAtlas)
	{
		m_AtlasImageInstance.SetImagePointer(pkGrpImgAtlas);
		if (pkGrpImgAtlas->IsEmpty())
			m_bAtlas=false;
		else
			m_bAtlas=true;
	}
	else
	{
		m_bAtlas = false;
	}
	m_AtlasPlayerMark.SetImagePointer((CGraphicSubImage *) CResourceManager::Instance().GetResourcePointer(playerMarkFileName));

	short sTerrainCountX, sTerrainCountY;
	rkMap.GetBaseXY(&m_dwAtlasBaseX, &m_dwAtlasBaseY);
	rkMap.GetTerrainCount(&sTerrainCountX, &sTerrainCountY);
	m_fAtlasMaxX = (float) sTerrainCountX * CTerrainImpl::TERRAIN_XSIZE;
	m_fAtlasMaxY = (float) sTerrainCountY * CTerrainImpl::TERRAIN_YSIZE;

	m_fAtlasImageSizeX = (float) m_AtlasImageInstance.GetWidth();
	m_fAtlasImageSizeY = (float) m_AtlasImageInstance.GetHeight();

#if defined(ENABLE_NPC_LOCATION_HELPER)
	m_strAtlasHelperDisplayMapName = rkMap.GetName();
#endif

	__LoadAtlasMarkInfo();

	if (m_bShowAtlas)
		OpenAtlasWindow();

	return true;
}

#if defined(ENABLE_NPC_LOCATION_HELPER)
bool CPythonMiniMap::LoadAtlasForMapName(const char* pszMapName)
{
	if (!pszMapName || '\0' == pszMapName[0])
		return false;

	const char* playerMarkFileName = "d:/ymir work/ui/minimap/playermark.sub";

	char atlasFileName[1024 + 1];
	BuildAtlasSubFileName(atlasFileName, sizeof(atlasFileName), pszMapName, m_bAtlasNpcHelperView);

	m_AtlasImageInstance.Destroy();
	m_AtlasPlayerMark.Destroy();

	CGraphicImage* pkGrpImgAtlas = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer(atlasFileName);
	if (pkGrpImgAtlas)
	{
		m_AtlasImageInstance.SetImagePointer(pkGrpImgAtlas);

		if (pkGrpImgAtlas->IsEmpty())
			m_bAtlas = false;
		else
			m_bAtlas = true;
	}
	else
	{
		m_bAtlas = false;
	}

	m_AtlasPlayerMark.SetImagePointer((CGraphicSubImage*)CResourceManager::Instance().GetResourcePointer(playerMarkFileName));
	m_AtlasPlayerMark.SetOrigin();

	CPythonBackground& rkBG = CPythonBackground::Instance();
	DWORD dwBaseX = 0;
	DWORD dwBaseY = 0;
	float fMaxX = 0.0f;
	float fMaxY = 0.0f;
	bool bHaveExtents = false;

	if (!bHaveExtents && rkBG.IsMapOutdoor())
	{
		CMapOutdoor& rkMap = rkBG.GetMapOutdoorRef();
		if (0 == _stricmp(rkMap.GetName().c_str(), pszMapName))
		{
			short sTerrainCountX = 0;
			short sTerrainCountY = 0;
			rkMap.GetBaseXY(&dwBaseX, &dwBaseY);
			rkMap.GetTerrainCount(&sTerrainCountX, &sTerrainCountY);
			fMaxX = static_cast<float>(sTerrainCountX) * static_cast<float>(CTerrainImpl::TERRAIN_XSIZE);
			fMaxY = static_cast<float>(sTerrainCountY) * static_cast<float>(CTerrainImpl::TERRAIN_YSIZE);
			bHaveExtents = (fMaxX > 0.0f && fMaxY > 0.0f);
		}
	}

	if (!bHaveExtents)
	{
		// --- FIX: Scale the map extents back up to Centimeters ---
		// 1 pixel on the atlas = 1 meter in-game (100cm).
		fMaxX = static_cast<float>(m_AtlasImageInstance.GetWidth()) * 100.0f;
		fMaxY = static_cast<float>(m_AtlasImageInstance.GetHeight()) * 100.0f;
		
		if (fMaxX <= 0.0f) fMaxX = 1.0f;
		if (fMaxY <= 0.0f) fMaxY = 1.0f;
	}

	m_dwAtlasBaseX = dwBaseX;
	m_dwAtlasBaseY = dwBaseY;
	m_fAtlasMaxX = fMaxX;
	m_fAtlasMaxY = fMaxY;

	m_fAtlasImageSizeX = static_cast<float>(m_AtlasImageInstance.GetWidth());
	m_fAtlasImageSizeY = static_cast<float>(m_AtlasImageInstance.GetHeight());

	m_strAtlasHelperDisplayMapName = pszMapName;

	__LoadAtlasMarkInfo();

	m_fAtlasHelperScale = 1.0f;
	m_fAtlasRenderPosX = 0.0f;
	m_fAtlasRenderPosY = 0.0f;
	__ClampAtlasRenderPos();

	if (m_bShowAtlas)
		OpenAtlasWindow();

	return m_bAtlas;
}
#endif

void CPythonMiniMap::__GlobalPositionToAtlasPosition(long lx, long ly, float * pfx, float * pfy)
{
	*pfx = lx / m_fAtlasMaxX * m_fAtlasImageSizeX;
	*pfy = ly / m_fAtlasMaxY * m_fAtlasImageSizeY;
}

void CPythonMiniMap::UpdateAtlas()
{
	CInstanceBase * pkInst = CPythonCharacterManager::Instance().GetMainInstancePtr();

	if (pkInst && !m_AtlasPlayerMark.IsEmpty())
	{
		TPixelPosition kInstPos;
		pkInst->NEW_GetPixelPosition(&kInstPos);

		float fRotation;
		fRotation = (540.0f - pkInst->GetRotation());
		while (fRotation > 360.0f)
			fRotation -= 360.0f;
		while (fRotation < 0.0f)
			fRotation += 360.0f;

#if defined(ENABLE_NPC_LOCATION_HELPER)
		if (m_bAtlasNpcHelperView && __IsAtlasHelperShowingCurrentMap())
		{
			float fPlayerX = 0.0f;
			float fPlayerY = 0.0f;
			__MapPosToHelperAtlasImagePos(kInstPos.x, kInstPos.y, &fPlayerX, &fPlayerY);
			m_fAtlasHelperPlayerScreenX = fPlayerX;
			m_fAtlasHelperPlayerScreenY = fPlayerY;
			m_fAtlasHelperPlayerRotation = fRotation;
			__SetAtlasHelperExpandedCenter(m_AtlasPlayerMark, fPlayerX, fPlayerY, true);
		}
		else
#endif
		if (m_fAtlasMaxX > 0.0f && m_fAtlasMaxY > 0.0f)
		{
			m_AtlasPlayerMark.SetPosition(kInstPos.x / m_fAtlasMaxX * m_fAtlasImageSizeX - (float)m_AtlasPlayerMark.GetWidth() / 2.0f,
				kInstPos.y / m_fAtlasMaxY * m_fAtlasImageSizeY - (float)m_AtlasPlayerMark.GetHeight() / 2.0f);
		}
		m_AtlasPlayerMark.SetRotation(fRotation);
#if defined(ENABLE_NPC_LOCATION_HELPER)
		__UpdateNPCLocationDirectionEffect();
#endif
	}

	{
		TGuildAreaInfoVectorIterator itor = m_GuildAreaInfoVector.begin();
		for (; itor != m_GuildAreaInfoVector.end(); ++itor)
		{
			TGuildAreaInfo & rInfo = *itor;
			__GlobalPositionToAtlasPosition(rInfo.lx, rInfo.ly, &rInfo.fsxRender, &rInfo.fsyRender);
			__GlobalPositionToAtlasPosition(rInfo.lx+rInfo.lwidth, rInfo.ly+rInfo.lheight, &rInfo.fexRender, &rInfo.feyRender);
		}
	}
}

void CPythonMiniMap::RenderAtlas(float fScreenX, float fScreenY)
{
	if (!m_bShowAtlas)
		return;

	if (m_fAtlasScreenX != fScreenX || m_fAtlasScreenY != fScreenY)
	{
		m_matWorldAtlas._41 = fScreenX;
		m_matWorldAtlas._42 = fScreenY;
		m_fAtlasScreenX = fScreenX;
		m_fAtlasScreenY = fScreenY;
	}

#if defined(ENABLE_NPC_LOCATION_HELPER)
	if (m_bAtlasNpcHelperView)
	{
		__RenderAtlasForNpcLocationHelper(fScreenX, fScreenY);
		return;
	}
#endif

	STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matWorldAtlas);
#ifdef DIRECTX9
	STATEMANAGER.SaveSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	STATEMANAGER.SaveSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
#else
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_MINFILTER);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_MAGFILTER);
#endif
	m_AtlasImageInstance.Render();

	STATEMANAGER.SaveRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

	STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_NPC));
	m_AtlasMarkInfoVectorIterator = m_AtlasNPCInfoVector.begin();
	while (m_AtlasMarkInfoVectorIterator != m_AtlasNPCInfoVector.end())
	{
		TAtlasMarkInfo & rAtlasMarkInfo = *m_AtlasMarkInfoVectorIterator;
		m_WhiteMark.SetPosition(rAtlasMarkInfo.m_fScreenX, rAtlasMarkInfo.m_fScreenY);
		m_WhiteMark.Render();
		++m_AtlasMarkInfoVectorIterator;
	}

	STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_WARP));
	m_AtlasMarkInfoVectorIterator = m_AtlasWarpInfoVector.begin();
	while (m_AtlasMarkInfoVectorIterator != m_AtlasWarpInfoVector.end())
	{
		TAtlasMarkInfo & rAtlasMarkInfo = *m_AtlasMarkInfoVectorIterator;
		m_WhiteMark.SetPosition(rAtlasMarkInfo.m_fScreenX, rAtlasMarkInfo.m_fScreenY);
		m_WhiteMark.Render();
		++m_AtlasMarkInfoVectorIterator;
	}

#if defined(__BL_PARTY_POSITION__)
	float v = (1 + sinf(CTimer::Instance().GetCurrentSecond() * 6)) / 5 + 0.8;
	D3DXCOLOR c(CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_PARTY));
	D3DXCOLOR d(v, v, v, 1);
	D3DXColorModulate(&c, &c, &d);

	STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, (DWORD)c);

	CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetMainInstancePtr();
	if (pkInst && pkInst->IsPartyMember())
	{
		const size_t iPartyMemberCount = CPythonPlayer::Instance().GetPartyMemberCount();
		if (iPartyMemberCount > 0)
		{
			DWORD dwPID = 0;
			CPythonPlayer::Instance().PartyMemberVIDToPID(pkInst->GetVirtualID(), &dwPID);

			for (size_t i = 0; i < iPartyMemberCount; ++i)
			{
				const DWORD dwPartyMemberPID = CPythonPlayer::Instance().GetPartyMemberPIDByIndex(i);
				if (dwPID != dwPartyMemberPID)
				{
					D3DXVECTOR2 v2Position = { 0.0f, 0.0f };
					CPythonPlayer::Instance().GetPartyMemberPosition(dwPartyMemberPID, &v2Position);
					if (v2Position.x != 0.0 && v2Position.y != 0.0 && v2Position.x >= 0.0 && m_fAtlasMaxX >= v2Position.x && v2Position.y >= 0.0 && m_fAtlasMaxY >= v2Position.y)
					{
						const float fX = v2Position.x / m_fAtlasMaxX * m_fAtlasImageSizeX;
						const float fY = v2Position.y / m_fAtlasMaxY * m_fAtlasImageSizeY;
						m_WhiteMark.SetPosition(fX, fY);
						m_WhiteMark.Render();
					}
				}
			}
		}
	}
#endif

#ifdef DIRECTX9
	STATEMANAGER.SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	STATEMANAGER.SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
#else
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_ANISOTROPIC);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_ANISOTROPIC);
#endif
	STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_WAYPOINT));
	m_AtlasMarkInfoVectorIterator = m_AtlasWayPointInfoVector.begin();
	for (; m_AtlasMarkInfoVectorIterator != m_AtlasWayPointInfoVector.end(); ++m_AtlasMarkInfoVectorIterator)
	{
		TAtlasMarkInfo & rAtlasMarkInfo = *m_AtlasMarkInfoVectorIterator;

		if (rAtlasMarkInfo.m_fScreenX <= 0.0f)
			continue;
		if (rAtlasMarkInfo.m_fScreenY <= 0.0f)
			continue;

		if (TYPE_TARGET == rAtlasMarkInfo.m_byType)
		{
			__RenderMiniWayPointMark(rAtlasMarkInfo.m_fScreenX, rAtlasMarkInfo.m_fScreenY);
		}
		else
		{
			__RenderWayPointMark(rAtlasMarkInfo.m_fScreenX, rAtlasMarkInfo.m_fScreenY);
		}
	}

	STATEMANAGER.RestoreRenderState(D3DRS_TEXTUREFACTOR);

	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG2);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);

	if ((ELTimer_GetMSec() / 500) % 2)
		m_AtlasPlayerMark.Render();

#ifdef DIRECTX9
	STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MINFILTER);
	STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MAGFILTER);
#else
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_MINFILTER);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_MAGFILTER);
#endif
	STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matIdentity);

	{
		TGuildAreaInfoVectorIterator itor = m_GuildAreaInfoVector.begin();
		for (; itor != m_GuildAreaInfoVector.end(); ++itor)
		{
			TGuildAreaInfo & rInfo = *itor;

			m_GuildAreaFlagImageInstance.SetPosition(fScreenX+(rInfo.fsxRender+rInfo.fexRender)/2.0f - m_GuildAreaFlagImageInstance.GetWidth()/2,
													 fScreenY+(rInfo.fsyRender+rInfo.feyRender)/2.0f - m_GuildAreaFlagImageInstance.GetHeight()/2);
			m_GuildAreaFlagImageInstance.Render();

//			CScreen::RenderBar2d(fScreenX+rInfo.fsxRender,
//								 fScreenY+rInfo.fsyRender,
//								 fScreenX+rInfo.fexRender,
//								 fScreenY+rInfo.feyRender);
		}
	}
}

bool CPythonMiniMap::GetPickedInstanceInfo(float fScreenX, float fScreenY, std::string & rReturnName, float * pReturnPosX, float * pReturnPosY, DWORD * pdwTextColor)
{
	float fDistanceFromMiniMapCenterX = fScreenX - m_fScreenX - m_fWidth * 0.5f;
	float fDistanceFromMiniMapCenterY = fScreenY - m_fScreenY - m_fHeight * 0.5f;

#if defined(__BL_PARTY_POSITION__)
	if (sqrtf(fDistanceFromMiniMapCenterX * fDistanceFromMiniMapCenterX + fDistanceFromMiniMapCenterY * fDistanceFromMiniMapCenterY) > 55.0f)
#else
	if (sqrtf(fDistanceFromMiniMapCenterX * fDistanceFromMiniMapCenterX + fDistanceFromMiniMapCenterY * fDistanceFromMiniMapCenterY) > m_fMiniMapRadius)
#endif
		return false;

	float fRealX = m_fCenterX + fDistanceFromMiniMapCenterX / m_fScale * ((float) CTerrainImpl::CELLSCALE);
	float fRealY = m_fCenterY + fDistanceFromMiniMapCenterY / m_fScale * ((float) CTerrainImpl::CELLSCALE);

	CInstanceBase * pkInst = CPythonCharacterManager::Instance().GetMainInstancePtr();

	if (pkInst)
	{
		TPixelPosition kInstPos;
		pkInst->NEW_GetPixelPosition(&kInstPos);

		if (fabs(kInstPos.x - fRealX) < ((float) CTerrainImpl::CELLSCALE) * 6.0f / m_fScale &&
			fabs(kInstPos.y - fRealY) < ((float) CTerrainImpl::CELLSCALE) * 6.0f / m_fScale)
		{
			#if defined(WJ_COMBAT_ZONE) && defined(WJ_COMBAT_ZONE_HIDE_INFO_USER)
			rReturnName = (pkInst->IsCombatZoneMap()) ? "" : pkInst->GetNameString();
			#else
			rReturnName = pkInst->GetNameString();				
			#endif
			*pReturnPosX = kInstPos.x;
			*pReturnPosY = kInstPos.y;
			*pdwTextColor = pkInst->GetNameColor();
			return true;
		}
	}

#if !defined(__BL_PARTY_POSITION__)
	if (m_fScale < 1.0f)
		return false;
#endif

	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	CPythonCharacterManager::CharacterIterator i;
	for(i = rkChrMgr.CharacterInstanceBegin(); i!=rkChrMgr.CharacterInstanceEnd(); ++i)
	{
		CInstanceBase* pkInstEach=*i;
		if (pkInstEach->IsInvisibility())
			continue;
		if (m_fScale < 2.0f && (pkInstEach->IsEnemy() || pkInstEach->IsPC()))
			continue;
		TPixelPosition kInstancePosition;
		pkInstEach->NEW_GetPixelPosition(&kInstancePosition);

		if (fabs(kInstancePosition.x - fRealX) < ((float) CTerrainImpl::CELLSCALE) * 3.0f / m_fScale &&
			fabs(kInstancePosition.y - fRealY) < ((float) CTerrainImpl::CELLSCALE) * 3.0f / m_fScale)
		{
			#if defined(WJ_COMBAT_ZONE) && defined(WJ_COMBAT_ZONE_HIDE_INFO_USER)
			rReturnName = (pkInstEach->IsCombatZoneMap()) ? "" : pkInstEach->GetNameString();
			#else
			rReturnName = pkInstEach->GetNameString();				
			#endif
			*pReturnPosX = kInstancePosition.x;
			*pReturnPosY = kInstancePosition.y;
			*pdwTextColor = pkInstEach->GetNameColor();
			return true;
		}
	}

#if defined(__BL_PARTY_POSITION__)
	if (pkInst && pkInst->IsPartyMember())
	{
		const size_t iPartyMemberCount = CPythonPlayer::Instance().GetPartyMemberCount();
		if (iPartyMemberCount > 0)
		{
			DWORD dwPID = 0;
			CPythonPlayer::Instance().PartyMemberVIDToPID(pkInst->GetVirtualID(), &dwPID);

			for (size_t i = 0; i < iPartyMemberCount; ++i)
			{
				const DWORD dwPartyMemberPID = CPythonPlayer::Instance().GetPartyMemberPIDByIndex(i);
				if (dwPID != dwPartyMemberPID)
				{
					D3DXVECTOR2 v2Position = { 0.0f, 0.0f };
					CPythonPlayer::Instance().GetPartyMemberMinimapPosition(dwPartyMemberPID, &v2Position);

					if (fabs(v2Position.x - fScreenX) < 3.0f && fabs(v2Position.y - fScreenY) < 3.0f)
					{
						rReturnName = CPythonPlayer::Instance().GetPartyMemberName(dwPartyMemberPID);
						CPythonPlayer::Instance().GetPartyMemberPosition(dwPartyMemberPID, &v2Position);
						*pReturnPosX = v2Position.x;
						*pReturnPosY = v2Position.y;
						*pdwTextColor = CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_PARTY);
						return true;
					}
				}
			}
		}
	}
#endif
	return false;
}

bool CPythonMiniMap::GetAtlasInfo(float fScreenX, float fScreenY, std::string & rReturnString, float * pReturnPosX, float * pReturnPosY, DWORD * pdwTextColor, DWORD * pdwGuildID)
{
#if defined(ENABLE_NPC_LOCATION_HELPER)
	if (m_bAtlasNpcHelperView)
	{
		const float fScale = (m_fAtlasHelperScale > 0.0f) ? m_fAtlasHelperScale : 1.0f;
		const float fLocalX = fScreenX - m_fAtlasScreenX;
		const float fLocalY = fScreenY - m_fAtlasScreenY;

		float fMarkPickHalfW = m_WhiteMark.IsEmpty() ? 6.0f : ((float)m_WhiteMark.GetWidth() * 0.5f * fScale + 1.0f);
		float fMarkPickHalfH = m_WhiteMark.IsEmpty() ? 6.0f : ((float)m_WhiteMark.GetHeight() * 0.5f * fScale + 1.0f);

		float fMiniWayPointPickHalfW = fMarkPickHalfW;
		float fMiniWayPointPickHalfH = fMarkPickHalfH;
		if (!m_MiniWayPointGraphicImageInstances[0].IsEmpty())
		{
			fMiniWayPointPickHalfW = (float)m_MiniWayPointGraphicImageInstances[0].GetWidth() * 0.5f * fScale + 1.0f;
			fMiniWayPointPickHalfH = (float)m_MiniWayPointGraphicImageInstances[0].GetHeight() * 0.5f * fScale + 1.0f;
		}

		bool bBestFound = false;
		float fBestDistSq = FLT_MAX;
		std::string strBestName;
		float fBestPosX = 0.0f;
		float fBestPosY = 0.0f;
		DWORD dwBestColor = 0;
		DWORD dwBestGuildID = 0;

		auto ConsiderHelperMark = [&](float fMapX, float fMapY, const std::string& strName, DWORD dwColor, float fHalfW, float fHalfH, DWORD dwGuildID_ = 0)
		{
			if (strName.empty()) return;
			float fMarkSX = 0.0f, fMarkSY = 0.0f;
			__CalcAtlasHelperScreenPos(fMapX, fMapY, &fMarkSX, &fMarkSY);
			const float fDx = fLocalX - fMarkSX;
			const float fDy = fLocalY - fMarkSY;
			if (fabsf(fDx) > fHalfW || fabsf(fDy) > fHalfH) return;
			const float fDistSq = fDx * fDx + fDy * fDy;
			if (fDistSq >= fBestDistSq) return;
			fBestDistSq = fDistSq;
			bBestFound = true;
			strBestName = strName;
			fBestPosX = fMapX;
			fBestPosY = fMapY;
			dwBestColor = dwColor;
			dwBestGuildID = dwGuildID_;
		};

		CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetMainInstancePtr();
		if (pkInst)
		{
			TPixelPosition kInstPos;
			pkInst->NEW_GetPixelPosition(&kInstPos);
			ConsiderHelperMark(kInstPos.x, kInstPos.y, pkInst->GetNameString(), pkInst->GetNameColor(), fMarkPickHalfW, fMarkPickHalfH);
		}

		m_AtlasMarkInfoVectorIterator = m_AtlasNPCInfoVector.begin();
		while (m_AtlasMarkInfoVectorIterator != m_AtlasNPCInfoVector.end())
		{
			const TAtlasMarkInfo& rAtlasMarkInfo = *m_AtlasMarkInfoVectorIterator;
			ConsiderHelperMark(rAtlasMarkInfo.m_fX, rAtlasMarkInfo.m_fY, rAtlasMarkInfo.m_strText, CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_NPC), fMarkPickHalfW, fMarkPickHalfH);
			++m_AtlasMarkInfoVectorIterator;
		}

		m_AtlasMarkInfoVectorIterator = m_AtlasWarpInfoVector.begin();
		while (m_AtlasMarkInfoVectorIterator != m_AtlasWarpInfoVector.end())
		{
			const TAtlasMarkInfo& rAtlasMarkInfo = *m_AtlasMarkInfoVectorIterator;
			ConsiderHelperMark(rAtlasMarkInfo.m_fX, rAtlasMarkInfo.m_fY, rAtlasMarkInfo.m_strText, CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_WARP), fMarkPickHalfW, fMarkPickHalfH);
			++m_AtlasMarkInfoVectorIterator;
		}

		m_AtlasMarkInfoVectorIterator = m_AtlasWayPointInfoVector.begin();
		while (m_AtlasMarkInfoVectorIterator != m_AtlasWayPointInfoVector.end())
		{
			const TAtlasMarkInfo& rAtlasMarkInfo = *m_AtlasMarkInfoVectorIterator;
			const float fHalfW = (TYPE_TARGET == rAtlasMarkInfo.m_byType) ? fMiniWayPointPickHalfW : fMarkPickHalfW;
			const float fHalfH = (TYPE_TARGET == rAtlasMarkInfo.m_byType) ? fMiniWayPointPickHalfH : fMarkPickHalfH;
			ConsiderHelperMark(rAtlasMarkInfo.m_fX, rAtlasMarkInfo.m_fY, rAtlasMarkInfo.m_strText, CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_WAYPOINT), fHalfW, fHalfH);
			++m_AtlasMarkInfoVectorIterator;
		}

#if defined(__BL_PARTY_POSITION__)
		if (pkInst && pkInst->IsPartyMember())
		{
			const size_t iPartyMemberCount = CPythonPlayer::Instance().GetPartyMemberCount();
			if (iPartyMemberCount > 0)
			{
				DWORD dwPID = 0;
				CPythonPlayer::Instance().PartyMemberVIDToPID(pkInst->GetVirtualID(), &dwPID);

				for (size_t i = 0; i < iPartyMemberCount; ++i)
				{
					const DWORD dwPartyMemberPID = CPythonPlayer::Instance().GetPartyMemberPIDByIndex(i);
					if (dwPID == dwPartyMemberPID) continue;

					D3DXVECTOR2 v2Position = { 0.0f, 0.0f };
					CPythonPlayer::Instance().GetPartyMemberPosition(dwPartyMemberPID, &v2Position);

					if (v2Position.x == 0.0f && v2Position.y == 0.0f) continue;

					ConsiderHelperMark(v2Position.x, v2Position.y, CPythonPlayer::Instance().GetPartyMemberName(dwPartyMemberPID), CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_PARTY), fMarkPickHalfW, fMarkPickHalfH);
				}
			}
		}
#endif

		TGuildAreaInfoVector::iterator itor = m_GuildAreaInfoVector.begin();
		for (; itor != m_GuildAreaInfoVector.end(); ++itor)
		{
			TGuildAreaInfo& rInfo = *itor;

			float fSX0 = 0.0f, fSY0 = 0.0f, fSX1 = 0.0f, fSY1 = 0.0f;
			__CalcAtlasHelperScreenPos((float)rInfo.lx, (float)rInfo.ly, &fSX0, &fSY0);
			__CalcAtlasHelperScreenPos((float)(rInfo.lx + rInfo.lwidth), (float)(rInfo.ly + rInfo.lheight), &fSX1, &fSY1);

			const float fMinX = (fSX0 < fSX1) ? fSX0 : fSX1;
			const float fMaxX = (fSX0 > fSX1) ? fSX0 : fSX1;
			const float fMinY = (fSY0 < fSY1) ? fSY0 : fSY1;
			const float fMaxY = (fSY0 > fSY1) ? fSY0 : fSY1;

			if (fLocalX < fMinX || fLocalX > fMaxX || fLocalY < fMinY || fLocalY > fMaxY) continue;

			const float fCenterX = (fMinX + fMaxX) * 0.5f;
			const float fCenterY = (fMinY + fMaxY) * 0.5f;
			const float fDx = fLocalX - fCenterX;
			const float fDy = fLocalY - fCenterY;
			const float fDistSq = fDx * fDx + fDy * fDy;
			if (fDistSq >= fBestDistSq) continue;

			std::string strGuildName;
			DWORD dwGuildID = 0;
			if (CPythonGuild::Instance().GetGuildName(rInfo.dwGuildID, &strGuildName))
				dwGuildID = rInfo.dwGuildID;
			else
				strGuildName = "empty_guild_area";

			fBestDistSq = fDistSq;
			bBestFound = true;
			strBestName = strGuildName;
			fBestPosX = (float)(rInfo.lx + rInfo.lwidth / 2);
			fBestPosY = (float)(rInfo.ly + rInfo.lheight / 2);
			dwBestColor = CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_PARTY);
			dwBestGuildID = dwGuildID;
		}

		if (bBestFound)
		{
			rReturnString = strBestName;
			*pReturnPosX = fBestPosX;
			*pReturnPosY = fBestPosY;
			*pdwTextColor = dwBestColor;
			*pdwGuildID = dwBestGuildID;
			return true;
		}
		return false;
	}
#endif
	float fRealX = (fScreenX - m_fAtlasScreenX) * (m_fAtlasMaxX / m_fAtlasImageSizeX);
	float fRealY = (fScreenY - m_fAtlasScreenY) * (m_fAtlasMaxY / m_fAtlasImageSizeY);

#ifdef ENABLE_MINIMAP_TELEPORT_CLICK
	*pReturnPosX = fRealX;
	*pReturnPosY = fRealY;
#endif

	//((float) CTerrainImpl::CELLSCALE) * 10.0f
	float fCheckWidth = (m_fAtlasMaxX / m_fAtlasImageSizeX) * 5.0f;
	float fCheckHeight = (m_fAtlasMaxY / m_fAtlasImageSizeY) * 5.0f;

	CInstanceBase * pkInst = CPythonCharacterManager::Instance().GetMainInstancePtr();

	if (pkInst)
	{
		TPixelPosition kInstPos;
		pkInst->NEW_GetPixelPosition(&kInstPos);

		if (kInstPos.x-fCheckWidth<fRealX && kInstPos.x+fCheckWidth>fRealX &&
			kInstPos.y-fCheckHeight<fRealY && kInstPos.y+fCheckHeight>fRealY)
		{
			#if defined(WJ_COMBAT_ZONE) && defined(WJ_COMBAT_ZONE_HIDE_INFO_USER)
			rReturnString = (pkInst->IsCombatZoneMap()) ? "" : pkInst->GetNameString();
			#else
			rReturnString = pkInst->GetNameString();				
			#endif
			*pReturnPosX = kInstPos.x;
			*pReturnPosY = kInstPos.y;
			*pdwTextColor = pkInst->GetNameColor();
			return true;
		}
	}

	m_AtlasMarkInfoVectorIterator = m_AtlasNPCInfoVector.begin();
	while (m_AtlasMarkInfoVectorIterator != m_AtlasNPCInfoVector.end())
	{
		TAtlasMarkInfo & rAtlasMarkInfo = *m_AtlasMarkInfoVectorIterator;

		if (rAtlasMarkInfo.m_fX-fCheckWidth/2<fRealX && rAtlasMarkInfo.m_fX+fCheckWidth>fRealX &&
			rAtlasMarkInfo.m_fY-fCheckWidth/2<fRealY && rAtlasMarkInfo.m_fY+fCheckHeight>fRealY)
		{
			rReturnString = rAtlasMarkInfo.m_strText;
			*pReturnPosX = rAtlasMarkInfo.m_fX;
			*pReturnPosY = rAtlasMarkInfo.m_fY;
			*pdwTextColor = CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_NPC);//m_MarkTypeToColorMap[rAtlasMarkInfo.m_byType];
			return true;
		}
		++m_AtlasMarkInfoVectorIterator;
	}

	m_AtlasMarkInfoVectorIterator = m_AtlasWarpInfoVector.begin();
	while (m_AtlasMarkInfoVectorIterator != m_AtlasWarpInfoVector.end())
	{
		TAtlasMarkInfo & rAtlasMarkInfo = *m_AtlasMarkInfoVectorIterator;
		if (rAtlasMarkInfo.m_fX-fCheckWidth/2<fRealX && rAtlasMarkInfo.m_fX+fCheckWidth>fRealX &&
			rAtlasMarkInfo.m_fY-fCheckWidth/2<fRealY && rAtlasMarkInfo.m_fY+fCheckHeight>fRealY)
		{
			rReturnString = rAtlasMarkInfo.m_strText;
			*pReturnPosX = rAtlasMarkInfo.m_fX;
			*pReturnPosY = rAtlasMarkInfo.m_fY;
			*pdwTextColor = CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_WARP);//m_MarkTypeToColorMap[rAtlasMarkInfo.m_byType];
			return true;
		}
		++m_AtlasMarkInfoVectorIterator;
	}

	m_AtlasMarkInfoVectorIterator = m_AtlasWayPointInfoVector.begin();
	while (m_AtlasMarkInfoVectorIterator != m_AtlasWayPointInfoVector.end())
	{
		TAtlasMarkInfo & rAtlasMarkInfo = *m_AtlasMarkInfoVectorIterator;
		if (rAtlasMarkInfo.m_fScreenX > 0.0f)
		if (rAtlasMarkInfo.m_fScreenY > 0.0f)
		if (rAtlasMarkInfo.m_fX-fCheckWidth/2<fRealX && rAtlasMarkInfo.m_fX+fCheckWidth>fRealX &&
			rAtlasMarkInfo.m_fY-fCheckWidth/2<fRealY && rAtlasMarkInfo.m_fY+fCheckHeight>fRealY)
		{
			rReturnString = rAtlasMarkInfo.m_strText;
			*pReturnPosX = rAtlasMarkInfo.m_fX;
			*pReturnPosY = rAtlasMarkInfo.m_fY;
			*pdwTextColor = CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_WAYPOINT);//m_MarkTypeToColorMap[rAtlasMarkInfo.m_byType];
			return true;
		}
		++m_AtlasMarkInfoVectorIterator;
	}

	TGuildAreaInfoVector::iterator itor = m_GuildAreaInfoVector.begin();
	for (; itor!=m_GuildAreaInfoVector.end(); ++itor)
	{
		TGuildAreaInfo & rInfo = *itor;
		if (fScreenX - m_fAtlasScreenX >= rInfo.fsxRender)
		if (fScreenY - m_fAtlasScreenY >= rInfo.fsyRender)
		if (fScreenX - m_fAtlasScreenX <= rInfo.fexRender)
		if (fScreenY - m_fAtlasScreenY <= rInfo.feyRender)
		{
			if (CPythonGuild::Instance().GetGuildName(rInfo.dwGuildID, &rReturnString))
			{
				*pdwGuildID = rInfo.dwGuildID;
			}
			else
			{
				rReturnString = "empty_guild_area";
			}

			*pReturnPosX = rInfo.lx + rInfo.lwidth/2;
			*pReturnPosY = rInfo.ly + rInfo.lheight/2;
			*pdwTextColor = CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_PARTY);
			return true;
		}
	}

#if defined(__BL_PARTY_POSITION__)
	if (pkInst && pkInst->IsPartyMember())
	{
		const size_t iPartyMemberCount = CPythonPlayer::Instance().GetPartyMemberCount();
		if (iPartyMemberCount > 0)
		{
			DWORD dwPID = 0;
			CPythonPlayer::Instance().PartyMemberVIDToPID(pkInst->GetVirtualID(), &dwPID);

			for (size_t i = 0; i < iPartyMemberCount; ++i)
			{
				const DWORD dwPartyMemberPID = CPythonPlayer::Instance().GetPartyMemberPIDByIndex(i);
				if (dwPID != dwPartyMemberPID)
				{
					D3DXVECTOR2 v2Position = { 0.0f, 0.0f };
					CPythonPlayer::Instance().GetPartyMemberPosition(dwPartyMemberPID, &v2Position);

					if (v2Position.x != 0.0f || v2Position.y != 0.0f)
					{
						if (fCheckWidth > fabs(v2Position.x - fRealX) && fCheckHeight > fabs(v2Position.y - fRealY))
						{
							rReturnString = CPythonPlayer::Instance().GetPartyMemberName(dwPartyMemberPID);
							*pReturnPosX = v2Position.x;
							*pReturnPosY = v2Position.y;
							*pdwTextColor = CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_PARTY);
							return true;
						}
					}
				}
			}
		}
	}
#endif

	return false;
}

bool CPythonMiniMap::GetAtlasSize(float * pfSizeX, float * pfSizeY)
{
	CPythonBackground& rkBG=CPythonBackground::Instance();
	if (!rkBG.IsMapOutdoor())
		return false;

	*pfSizeX = m_fAtlasImageSizeX;
	*pfSizeY = m_fAtlasImageSizeY;

	return true;
}

// Atlas
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// WayPoint
void CPythonMiniMap::AddWayPoint(BYTE byType, DWORD dwID, float fX, float fY, std::string strText, DWORD dwChrVID)
{
	m_AtlasMarkInfoVectorIterator = m_AtlasWayPointInfoVector.begin();
	while (m_AtlasMarkInfoVectorIterator != m_AtlasWayPointInfoVector.end())
	{
		TAtlasMarkInfo & rAtlasMarkInfo = *m_AtlasMarkInfoVectorIterator;
		if (rAtlasMarkInfo.m_dwID == dwID)
			return;
		++m_AtlasMarkInfoVectorIterator;
	}

	TAtlasMarkInfo aAtlasMarkInfo;
	aAtlasMarkInfo.m_byType = byType;
	aAtlasMarkInfo.m_dwID = dwID;
	aAtlasMarkInfo.m_fX = fX;
	aAtlasMarkInfo.m_fY = fY;
	aAtlasMarkInfo.m_fScreenX = 0.0f;
	aAtlasMarkInfo.m_fScreenY = 0.0f;
	aAtlasMarkInfo.m_fMiniMapX = 0.0f;
	aAtlasMarkInfo.m_fMiniMapY = 0.0f;
	aAtlasMarkInfo.m_strText = strText;
	aAtlasMarkInfo.m_dwChrVID = dwChrVID;
	__UpdateWayPoint(&aAtlasMarkInfo, fX, fY);
	m_AtlasWayPointInfoVector.push_back(aAtlasMarkInfo);
}

void CPythonMiniMap::RemoveWayPoint(DWORD dwID)
{
	m_AtlasMarkInfoVectorIterator = m_AtlasWayPointInfoVector.begin();
	while (m_AtlasMarkInfoVectorIterator != m_AtlasWayPointInfoVector.end())
	{
		TAtlasMarkInfo & rAtlasMarkInfo = *m_AtlasMarkInfoVectorIterator;
		if (rAtlasMarkInfo.m_dwID == dwID)
		{
			m_AtlasMarkInfoVectorIterator = m_AtlasWayPointInfoVector.erase(m_AtlasMarkInfoVectorIterator);
			return;
		}
		++m_AtlasMarkInfoVectorIterator;
	}
}

bool CPythonMiniMap::__GetWayPoint(DWORD dwID, TAtlasMarkInfo ** ppkInfo)
{
	TAtlasMarkInfoVectorIterator itor = m_AtlasWayPointInfoVector.begin();
	for (; itor != m_AtlasWayPointInfoVector.end(); ++itor)
	{
		TAtlasMarkInfo & rInfo = *itor;
		if (dwID == rInfo.m_dwID)
		{
			*ppkInfo = &rInfo;
			return true;
		}
	}

	return false;
}

void CPythonMiniMap::__UpdateWayPoint(TAtlasMarkInfo * pkInfo, int ix, int iy)
{
	pkInfo->m_fX = float(ix);
	pkInfo->m_fY = float(iy);

#if defined(ENABLE_NPC_LOCATION_HELPER)
	if (m_bAtlasNpcHelperView)
	{
		__MapPosToHelperAtlasImagePos(pkInfo->m_fX, pkInfo->m_fY, &pkInfo->m_fScreenX, &pkInfo->m_fScreenY);
		return;
	}
#endif

	pkInfo->m_fScreenX = pkInfo->m_fX / m_fAtlasMaxX * m_fAtlasImageSizeX;
	pkInfo->m_fScreenY = pkInfo->m_fY / m_fAtlasMaxY * m_fAtlasImageSizeY;
}

// WayPoint
//////////////////////////////////////////////////////////////////////////

void CPythonMiniMap::__RenderWayPointMark(int ixCenter, int iyCenter)
{
	int iNum = (ELTimer_GetMSec() / 67) % WAYPOINT_IMAGE_COUNT;

	CGraphicImageInstance & rInstance = m_WayPointGraphicImageInstances[iNum];
	rInstance.SetPosition(ixCenter - rInstance.GetWidth()/2, iyCenter - rInstance.GetHeight()/2);
	rInstance.Render();
}

void CPythonMiniMap::__RenderMiniWayPointMark(int ixCenter, int iyCenter)
{
	int iNum = (ELTimer_GetMSec() / 67) % MINI_WAYPOINT_IMAGE_COUNT;

	CGraphicImageInstance & rInstance = m_MiniWayPointGraphicImageInstances[iNum];
	rInstance.SetPosition(ixCenter - rInstance.GetWidth()/2, iyCenter - rInstance.GetHeight()/2);
	rInstance.Render();
}

void CPythonMiniMap::__RenderTargetMark(int ixCenter, int iyCenter)
{
	int iNum = (ELTimer_GetMSec() / 80) % TARGET_MARK_IMAGE_COUNT;

	CGraphicImageInstance & rInstance = m_TargetMarkGraphicImageInstances[iNum];
	rInstance.SetPosition(ixCenter - rInstance.GetWidth()/2, iyCenter - rInstance.GetHeight()/2);
	rInstance.Render();
}

void CPythonMiniMap::AddSignalPoint(float fX, float fY)
{
	static unsigned int g_id = 255;

	TSignalPoint sp;
	sp.id = g_id;
	sp.v2Pos.x = fX;
	sp.v2Pos.y = fY;

	m_SignalPointVector.push_back(sp);

	AddWayPoint(TYPE_WAYPOINT, g_id, fX, fY, "");

	g_id++;
}

void CPythonMiniMap::ClearAllSignalPoint()
{
	std::vector<TSignalPoint>::iterator it;
	for(it = m_SignalPointVector.begin();it!=m_SignalPointVector.end();++it)
	{
		RemoveWayPoint(it->id);
	}
	m_SignalPointVector.clear();
}

void CPythonMiniMap::RegisterAtlasWindow(PyObject* poHandler)
{
	m_poHandler = poHandler;
}

void CPythonMiniMap::UnregisterAtlasWindow()
{
	m_poHandler = 0;
}

void CPythonMiniMap::OpenAtlasWindow()
{
	if (m_poHandler)
	{
		PyCallClassMemberFunc(m_poHandler,"Show", Py_BuildValue("()"));
	}
}

void CPythonMiniMap::SetAtlasCenterPosition(int x, int y)
{
	if (m_poHandler)
	{
		//int sw = UI::CWindowManager::Instance().GetScreenWidth();
		//int sh = UI::CWindowManager::Instance().GetScreenHeight();
		//PyCallClassMemberFunc(m_poHandler,"SetPosition", Py_BuildValue("(ii)",sw/2+x,sh/2+y));
		PyCallClassMemberFunc(m_poHandler,"SetCenterPositionAdjust", Py_BuildValue("(ii)",x,y));
	}
}

bool CPythonMiniMap::IsAtlas()
{
	return m_bAtlas;
}

void CPythonMiniMap::ShowAtlas()
{
	m_bShowAtlas=true;
}

void CPythonMiniMap::HideAtlas()
{
	m_bShowAtlas=false;
}

bool CPythonMiniMap::CanShowAtlas()
{
	return m_bShowAtlas;
}

bool CPythonMiniMap::CanShow()
{
	return m_bShow;
}

void CPythonMiniMap::Show()
{
	m_bShow=true;
}

void CPythonMiniMap::Hide()
{
	m_bShow=false;
}

void CPythonMiniMap::__Initialize()
{
	m_poHandler = 0;

	SetMiniMapSize(128.0f, 128.0f);

	m_fScale = 2.0f;

	m_fCenterX = m_fWidth * 0.5f;
	m_fCenterY = m_fHeight * 0.5f;

	m_fScreenX = 0.0f;
	m_fScreenY = 0.0f;

	m_fAtlasScreenX = 0.0f;
	m_fAtlasScreenY = 0.0f;

	m_dwAtlasBaseX = 0;
	m_dwAtlasBaseY = 0;

	m_fAtlasMaxX = 0.0f;
	m_fAtlasMaxY = 0.0f;

	m_fAtlasImageSizeX = 0.0f;
	m_fAtlasImageSizeY = 0.0f;

	m_bAtlas = false;

	m_bShow = false;
	m_bShowAtlas = false;
#if defined(ENABLE_NPC_LOCATION_HELPER)
	m_bAtlasNpcHelperView = false;
	m_strAtlasHelperDisplayMapName.clear();
	m_fAtlasHelperScale = 1.0f;
	m_fAtlasHelperViewportW = static_cast<float>(SQUARED_ATLAS_SIZE);
	m_fAtlasHelperViewportH = static_cast<float>(SQUARED_ATLAS_SIZE);
	m_fAtlasRenderPosX = 0.0f;
	m_fAtlasRenderPosY = 0.0f;
	m_fAtlasHelperPlayerScreenX = 0.0f;
	m_fAtlasHelperPlayerScreenY = 0.0f;
	m_fAtlasHelperPlayerRotation = 0.0f;
	m_bNPCSelected = false;
	m_lSelectedMapIndex = 0;
	m_dwSelectedNPCVnum = 0;
	m_lSelectedNPCX = 0;
	m_lSelectedNPCY = 0;
	m_dwNPCLocationDirectionEffectID = 0;
	m_dwNPCLocationDirectionEffectStartMS = 0;
	m_iNPCLocationDirectionEffectIndex = -1;
#endif
	D3DXMatrixIdentity(&m_matIdentity);
	D3DXMatrixIdentity(&m_matWorld);
	D3DXMatrixIdentity(&m_matMiniMapCover);
	D3DXMatrixIdentity(&m_matWorldAtlas);
}

void CPythonMiniMap::Destroy()
{
	ClearAllSignalPoint();
	m_poHandler = 0;

	m_VertexBuffer.Destroy();
	m_IndexBuffer.Destroy();

	m_PlayerMark.Destroy();

	m_MiniMapFilterGraphicImageInstance.Destroy();
	m_MiniMapCameraraphicImageInstance.Destroy();

	m_AtlasWayPointInfoVector.clear();
	m_AtlasImageInstance.Destroy();
	m_AtlasPlayerMark.Destroy();
	m_WhiteMark.Destroy();

	for (int i = 0; i < MINI_WAYPOINT_IMAGE_COUNT; ++i)
		m_MiniWayPointGraphicImageInstances[i].Destroy();
	for (int j = 0; j < WAYPOINT_IMAGE_COUNT; ++j)
		m_WayPointGraphicImageInstances[j].Destroy();
	for (int k = 0; k < TARGET_MARK_IMAGE_COUNT; ++k)
		m_TargetMarkGraphicImageInstances[k].Destroy();

	m_GuildAreaFlagImageInstance.Destroy();
#if defined(ENABLE_NPC_LOCATION_HELPER)
	for (int iMyPos = 0; iMyPos < ATLAS_HELPER_MY_POSITION_IMAGE_COUNT; ++iMyPos)
		m_AtlasHelperMyPositionMarks[iMyPos].Destroy();
#endif
	__Initialize();
}

CPythonMiniMap::CPythonMiniMap()
{
	__Initialize();
}

CPythonMiniMap::~CPythonMiniMap()
{
	Destroy();
}

#if defined(ENABLE_NPC_LOCATION_HELPER)
namespace
{
	bool AtlasHelperMarkCulled(float fMarkX, float fMarkY, float fHalfW, float fHalfH, const RECT* pClip)
	{
		if (!pClip) return false;
		if (fMarkX + fHalfW <= (float)pClip->left) return true;
		if (fMarkY + fHalfH <= (float)pClip->top) return true;
		if (fMarkX - fHalfW >= (float)pClip->right) return true;
		if (fMarkY - fHalfH >= (float)pClip->bottom) return true;
		return false;
	}

	bool AtlasHelperMarkFullyInside(float fMarkX, float fMarkY, float fHalfW, float fHalfH, const RECT* pClip)
	{
		if (!pClip) return true;
		if (fMarkX - fHalfW < (float)pClip->left) return false;
		if (fMarkY - fHalfH < (float)pClip->top) return false;
		if (fMarkX + fHalfW > (float)pClip->right) return false;
		if (fMarkY + fHalfH > (float)pClip->bottom) return false;
		return true;
	}

	float AtlasHelperMarkRotatedHalfExtent(float fHalfW, float fHalfH, float fRotationDeg)
	{
		const float fRad = D3DXToRadian(fRotationDeg);
		const float c = fabsf(cosf(fRad));
		const float s = fabsf(sinf(fRad));
		return fHalfW * c + fHalfH * s;
	}

	const float HELPER_ATLAS_LEGACY_TEX_W = 204.0f;
	const float HELPER_ATLAS_LEGACY_TEX_H = 255.0f;

	const float HELPER_ATLAS_SCALE_LEVELS[] = { 1.0f, 1.5f, 2.0f, 2.5f, 3.0f };
	const int HELPER_ATLAS_SCALE_LEVEL_COUNT = sizeof(HELPER_ATLAS_SCALE_LEVELS) / sizeof(HELPER_ATLAS_SCALE_LEVELS[0]);

	int FindAtlasHelperScaleLevelIndex(float fScale)
	{
		int iBest = 0;
		float fBestDiff = fabsf(HELPER_ATLAS_SCALE_LEVELS[0] - fScale);
		for (int i = 1; i < HELPER_ATLAS_SCALE_LEVEL_COUNT; ++i)
		{
			const float fDiff = fabsf(HELPER_ATLAS_SCALE_LEVELS[i] - fScale);
			if (fDiff < fBestDiff)
			{
				fBestDiff = fDiff;
				iBest = i;
			}
		}
		return iBest;
	}

	const char* c_szNPCLocationDirectionEffect = "d:/ymir work/effect/etc/npc_location/npc_location_direction.mse";
	const float c_fNPCLocationDirectionArriveRadius = 300.0f;
	const DWORD c_dwNPCLocationDirectionGraceMS = 400;

	float __GetNPCLocationDirectionGroundZ(const TPixelPosition& kPlayerPos)
	{
		CPythonBackground& rkBG = CPythonBackground::Instance();
		if (rkBG.IsMapOutdoor()) return rkBG.GetHeight(kPlayerPos.x, kPlayerPos.y);
		return kPlayerPos.z;
	}

	void __BuildNPCLocationDirectionEffectMatrix(const TPixelPosition& kPlayerPos, float fTargetX, float fTargetY, D3DXMATRIX* pMat)
	{
		float fAngle = GetDegreeFromPosition2(kPlayerPos.x, kPlayerPos.y, fTargetX, fTargetY);
		fAngle = fmodf(540.0f - fAngle, 360.0f);
		D3DXMatrixRotationYawPitchRoll(pMat, 0.0f, 0.0f, D3DXToRadian(fAngle));
		pMat->_41 = kPlayerPos.x;
		pMat->_42 = -kPlayerPos.y;
		pMat->_43 = __GetNPCLocationDirectionGroundZ(kPlayerPos);
	}
}

void CPythonMiniMap::__RenderAtlasForNpcLocationHelper(float fScreenX, float fScreenY)
{
	D3DXMATRIX matFinalAtlas, matAtlasHelper;
	D3DXMatrixScaling(&matAtlasHelper, m_fAtlasHelperScale, m_fAtlasHelperScale, 1.0f);
	matAtlasHelper._41 = -m_fAtlasRenderPosX;
	matAtlasHelper._42 = -m_fAtlasRenderPosY;
	D3DXMatrixMultiply(&matFinalAtlas, &matAtlasHelper, &m_matWorldAtlas);
	STATEMANAGER.SetTransform(D3DTS_WORLD, &matFinalAtlas);

#ifdef DIRECTX9
	STATEMANAGER.SaveSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	STATEMANAGER.SaveSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	if (m_AtlasImageInstance.IsEmpty())
	{
		STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MINFILTER);
		STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MAGFILTER);
		return;
	}
#else
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_POINT);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_POINT);
	if (m_AtlasImageInstance.IsEmpty())
	{
		STATEMANAGER.RestoreTextureStageState(0, D3DTSS_MINFILTER);
		STATEMANAGER.RestoreTextureStageState(0, D3DTSS_MAGFILTER);
		return;
	}
#endif

	const float fScale = (m_fAtlasHelperScale > 0.0f) ? m_fAtlasHelperScale : 1.0f;
	RECT rcHelperMarkClip;
	RECT* pHelperMarkClip = NULL;

#if defined(ENABLE_CLIP_MASK)
	{
		RECT rcMask;
		rcMask.left = static_cast<LONG>(m_fAtlasRenderPosX / fScale);
		rcMask.top = static_cast<LONG>(m_fAtlasRenderPosY / fScale);
		rcMask.right = static_cast<LONG>((m_fAtlasRenderPosX + __GetAtlasHelperViewportW()) / fScale);
		rcMask.bottom = static_cast<LONG>((m_fAtlasRenderPosY + __GetAtlasHelperViewportH()) / fScale);
		m_AtlasImageInstance.Render(&rcMask);
		rcHelperMarkClip = rcMask;
		pHelperMarkClip = &rcHelperMarkClip;
	}
#else
	m_AtlasImageInstance.Render();
#endif

	STATEMANAGER.SetTransform(D3DTS_WORLD, &matFinalAtlas);
	STATEMANAGER.SaveRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_NPC));

	if (!m_WhiteMark.IsEmpty())
	{
		const float fHalfW = (float)m_WhiteMark.GetWidth() * 0.5f;
		const float fHalfH = (float)m_WhiteMark.GetHeight() * 0.5f;

		m_AtlasMarkInfoVectorIterator = m_AtlasNPCInfoVector.begin();
		while (m_AtlasMarkInfoVectorIterator != m_AtlasNPCInfoVector.end())
		{
			TAtlasMarkInfo& rAtlasMarkInfo = *m_AtlasMarkInfoVectorIterator;
			if (m_bNPCSelected)
			{
				if (!__IsAtlasMarkAtSelectedNpc(rAtlasMarkInfo)) { ++m_AtlasMarkInfoVectorIterator; continue; }
				++m_AtlasMarkInfoVectorIterator; continue;
			}
			float fMarkX = 0.0f, fMarkY = 0.0f;
			__MapPosToHelperAtlasImagePos(rAtlasMarkInfo.m_fX, rAtlasMarkInfo.m_fY, &fMarkX, &fMarkY);
			if (!AtlasHelperMarkCulled(fMarkX, fMarkY, fHalfW, fHalfH, pHelperMarkClip))
			{
				m_WhiteMark.SetPosition(fMarkX - fHalfW, fMarkY - fHalfH);
#if defined(ENABLE_CLIP_MASK)
				m_WhiteMark.Render(pHelperMarkClip);
#else
				m_WhiteMark.Render();
#endif
			}
			++m_AtlasMarkInfoVectorIterator;
		}
	}

	STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_WARP));
	if (!m_WhiteMark.IsEmpty() && !m_bNPCSelected)
	{
		const float fHalfW = (float)m_WhiteMark.GetWidth() * 0.5f;
		const float fHalfH = (float)m_WhiteMark.GetHeight() * 0.5f;
		m_AtlasMarkInfoVectorIterator = m_AtlasWarpInfoVector.begin();
		while (m_AtlasMarkInfoVectorIterator != m_AtlasWarpInfoVector.end())
		{
			TAtlasMarkInfo& rAtlasMarkInfo = *m_AtlasMarkInfoVectorIterator;
			float fMarkX = 0.0f, fMarkY = 0.0f;
			__MapPosToHelperAtlasImagePos(rAtlasMarkInfo.m_fX, rAtlasMarkInfo.m_fY, &fMarkX, &fMarkY);

			if (!AtlasHelperMarkCulled(fMarkX, fMarkY, fHalfW, fHalfH, pHelperMarkClip))
			{
				m_WhiteMark.SetPosition(fMarkX - fHalfW, fMarkY - fHalfH);
#if defined(ENABLE_CLIP_MASK)
				m_WhiteMark.Render(pHelperMarkClip);
#else
				m_WhiteMark.Render();
#endif
			}
			++m_AtlasMarkInfoVectorIterator;
		}
	}

#if defined(__BL_PARTY_POSITION__)
	if (__IsAtlasHelperShowingCurrentMap())
	{
		float v = (1 + sinf(CTimer::Instance().GetCurrentSecond() * 6)) / 5 + 0.8;
		D3DXCOLOR c(CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_PARTY));
		D3DXCOLOR d(v, v, v, 1);
		D3DXColorModulate(&c, &c, &d);
		STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, (DWORD)c);

		CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetMainInstancePtr();
		if (pkInst && pkInst->IsPartyMember())
		{
			const size_t iPartyMemberCount = CPythonPlayer::Instance().GetPartyMemberCount();
			if (iPartyMemberCount > 0)
			{
				DWORD dwPID = 0;
				CPythonPlayer::Instance().PartyMemberVIDToPID(pkInst->GetVirtualID(), &dwPID);

				// Pre-calculate mark boundaries for clipping
				const float fHalfW = (float)m_WhiteMark.GetWidth() * 0.5f;
				const float fHalfH = (float)m_WhiteMark.GetHeight() * 0.5f;

				for (size_t i = 0; i < iPartyMemberCount; ++i)
				{
					const DWORD dwPartyMemberPID = CPythonPlayer::Instance().GetPartyMemberPIDByIndex(i);
					if (dwPID != dwPartyMemberPID)
					{
						D3DXVECTOR2 v2Position = { 0.0f, 0.0f };
						CPythonPlayer::Instance().GetPartyMemberPosition(dwPartyMemberPID, &v2Position);
						if (m_fAtlasMaxX > 0.0f && m_fAtlasMaxY > 0.0f && m_fAtlasImageSizeX > 0.0f && m_fAtlasImageSizeY > 0.0f && v2Position.x != 0.0 && v2Position.y != 0.0 && v2Position.x >= 0.0 && m_fAtlasMaxX >= v2Position.x && v2Position.y >= 0.0 && m_fAtlasMaxY >= v2Position.y)
						{
							// --- FIX: Properly convert to Helper Atlas Image Space ---
							float fMarkX = 0.0f, fMarkY = 0.0f;
							__MapPosToHelperAtlasImagePos(v2Position.x, v2Position.y, &fMarkX, &fMarkY);

							if (!AtlasHelperMarkCulled(fMarkX, fMarkY, fHalfW, fHalfH, pHelperMarkClip))
							{
								m_WhiteMark.SetPosition(fMarkX - fHalfW, fMarkY - fHalfH);
#if defined(ENABLE_CLIP_MASK)
								m_WhiteMark.Render(pHelperMarkClip);
#else
								m_WhiteMark.Render();
#endif
							}
						}
					}
				}
			}
		}
	}
#endif

	if (m_bNPCSelected || __IsAtlasHelperShowingCurrentMap())
	{
#ifdef DIRECTX9
		STATEMANAGER.SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		STATEMANAGER.SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
#else
		STATEMANAGER.SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
#endif
		STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, CInstanceBase::GetIndexedNameColor(CInstanceBase::NAMECOLOR_WAYPOINT));
		if (!m_bNPCSelected)
		{
			m_AtlasMarkInfoVectorIterator = m_AtlasWayPointInfoVector.begin();
			for (; m_AtlasMarkInfoVectorIterator != m_AtlasWayPointInfoVector.end(); ++m_AtlasMarkInfoVectorIterator)
			{
				TAtlasMarkInfo& rAtlasMarkInfo = *m_AtlasMarkInfoVectorIterator;
				float fMarkX = 0.0f, fMarkY = 0.0f;
				__MapPosToHelperAtlasImagePos(rAtlasMarkInfo.m_fX, rAtlasMarkInfo.m_fY, &fMarkX, &fMarkY);
				const int iMarkX = static_cast<int>(fMarkX);
				const int iMarkY = static_cast<int>(fMarkY);

				if (TYPE_TARGET == rAtlasMarkInfo.m_byType)
				{
#if defined(ENABLE_CLIP_MASK)
					__RenderWayPointMark(iMarkX, iMarkY, pHelperMarkClip);
					__RenderMiniWayPointMark(iMarkX, iMarkY, pHelperMarkClip);
#else
					__RenderWayPointMark(iMarkX, iMarkY);
					__RenderMiniWayPointMark(iMarkX, iMarkY);
#endif
				}
				else
				{
#if defined(ENABLE_CLIP_MASK)
					__RenderWayPointMark(iMarkX, iMarkY, pHelperMarkClip);
#else
					__RenderWayPointMark(iMarkX, iMarkY);
#endif
				}
			}
		}

		if (m_bNPCSelected)
			__RenderAtlasHelperSelectedNpcMark(pHelperMarkClip);

		if (__IsAtlasHelperShowingCurrentMap())
			__RenderAtlasHelperPlayerMark(pHelperMarkClip);
	}

	STATEMANAGER.RestoreRenderState(D3DRS_TEXTUREFACTOR);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG2);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLOROP);
#ifdef DIRECTX9
	STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MINFILTER);
	STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MAGFILTER);
#else
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_MINFILTER);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_MAGFILTER);
#endif
	STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matIdentity);
}

void CPythonMiniMap::__MapPosToHelperAtlasImagePos(float fMapX, float fMapY, float* pfImgX, float* pfImgY) const
{
	if (!pfImgX || !pfImgY || m_fAtlasMaxX <= 0.0f || m_fAtlasMaxY <= 0.0f) return;
	if (m_fAtlasImageSizeX <= 0.0f || m_fAtlasImageSizeY <= 0.0f) { *pfImgX = 0.0f; *pfImgY = 0.0f; return; }

	const float fLegacyX = fMapX / m_fAtlasMaxX * HELPER_ATLAS_LEGACY_TEX_W;
	const float fLegacyY = fMapY / m_fAtlasMaxY * HELPER_ATLAS_LEGACY_TEX_H;

	const float fScaleX = m_fAtlasImageSizeX / HELPER_ATLAS_LEGACY_TEX_W;
	const float fScaleY = m_fAtlasImageSizeY / HELPER_ATLAS_LEGACY_TEX_H;
	const float fContain = (fScaleX < fScaleY) ? fScaleX : fScaleY;

	const float fContentW = HELPER_ATLAS_LEGACY_TEX_W * fContain;
	const float fContentH = HELPER_ATLAS_LEGACY_TEX_H * fContain;
	const float fOffX = (m_fAtlasImageSizeX - fContentW) * 0.5f;
	const float fOffY = (m_fAtlasImageSizeY - fContentH) * 0.5f;

	*pfImgX = fLegacyX * fContain + fOffX;
	*pfImgY = fLegacyY * fContain + fOffY;
}

void CPythonMiniMap::__CalcAtlasHelperScreenPos(float fMapX, float fMapY, float* pfScreenX, float* pfScreenY) const
{
	if (!pfScreenX || !pfScreenY) return;
	float fImgX = 0.0f, fImgY = 0.0f;
	__MapPosToHelperAtlasImagePos(fMapX, fMapY, &fImgX, &fImgY);
	const float fScale = (m_fAtlasHelperScale > 0.0f) ? m_fAtlasHelperScale : 1.0f;
	*pfScreenX = fImgX * fScale - m_fAtlasRenderPosX;
	*pfScreenY = fImgY * fScale - m_fAtlasRenderPosY;
}

float CPythonMiniMap::__GetAtlasHelperViewportW() const { return (m_fAtlasHelperViewportW > 0.0f) ? m_fAtlasHelperViewportW : static_cast<float>(SQUARED_ATLAS_SIZE); }
float CPythonMiniMap::__GetAtlasHelperViewportH() const { return (m_fAtlasHelperViewportH > 0.0f) ? m_fAtlasHelperViewportH : static_cast<float>(SQUARED_ATLAS_SIZE); }

void CPythonMiniMap::__GetAtlasHelperPanExtents(float& fMinX, float& fMaxX, float& fMinY, float& fMaxY) const
{
	const float fViewportW = __GetAtlasHelperViewportW();
	const float fViewportH = __GetAtlasHelperViewportH();
	const float fScaledW = m_fAtlasImageSizeX * m_fAtlasHelperScale;
	const float fScaledH = m_fAtlasImageSizeY * m_fAtlasHelperScale;

	fMinX = 0.5f * (fScaledW - fViewportW); fMaxX = fMinX;
	fMinY = 0.5f * (fScaledH - fViewportH); fMaxY = fMinY;
	if (fScaledW > fViewportW) { fMinX = 0.0f; fMaxX = fScaledW - fViewportW; }
	if (fScaledH > fViewportH) { fMinY = 0.0f; fMaxY = fScaledH - fViewportH; }
}

bool CPythonMiniMap::__IsAtlasHelperMarkVisible(float fLocalX, float fLocalY, float fHalfW, float fHalfH) const
{
	const float fViewportW = __GetAtlasHelperViewportW();
	const float fViewportH = __GetAtlasHelperViewportH();
	if (fLocalX + fHalfW <= 0.0f || fLocalY + fHalfH <= 0.0f || fLocalX - fHalfW >= fViewportW || fLocalY - fHalfH >= fViewportH) return false;
	return true;
}

void CPythonMiniMap::__GetAtlasHelperMarkClipRect(RECT* pRect) const
{
	if (!pRect) return;
	pRect->left = 0; pRect->top = 0;
	pRect->right = static_cast<LONG>(__GetAtlasHelperViewportW());
	pRect->bottom = static_cast<LONG>(__GetAtlasHelperViewportH());
}

void CPythonMiniMap::__GetAtlasHelperImageClipRect(RECT* pRect) const
{
	if (!pRect) return;
	const float fScale = (m_fAtlasHelperScale > 0.0f) ? m_fAtlasHelperScale : 1.0f;
	pRect->left = static_cast<LONG>(m_fAtlasRenderPosX / fScale);
	pRect->top = static_cast<LONG>(m_fAtlasRenderPosY / fScale);
	pRect->right = static_cast<LONG>((m_fAtlasRenderPosX + __GetAtlasHelperViewportW()) / fScale);
	pRect->bottom = static_cast<LONG>((m_fAtlasRenderPosY + __GetAtlasHelperViewportH()) / fScale);
}

float CPythonMiniMap::__GetNextAtlasHelperScale(float fCurrentScale, bool bScaleUp) const
{
	if (bScaleUp && fCurrentScale < 1.0f - 0.001f) return 1.0f;
	const int iIdx = FindAtlasHelperScaleLevelIndex(fCurrentScale);
	if (bScaleUp) return (iIdx < HELPER_ATLAS_SCALE_LEVEL_COUNT - 1) ? HELPER_ATLAS_SCALE_LEVELS[iIdx + 1] : HELPER_ATLAS_SCALE_LEVELS[iIdx];
	return (iIdx > 0) ? HELPER_ATLAS_SCALE_LEVELS[iIdx - 1] : HELPER_ATLAS_SCALE_LEVELS[iIdx];
}

void CPythonMiniMap::__ApplyAtlasHelperScaleAtPlayerFocal(float fNewScale)
{
	if (fNewScale < 1.0f) fNewScale = 1.0f;
	if (fNewScale > 3.0f) fNewScale = 3.0f;
	const float fOldScale = m_fAtlasHelperScale;
	if (fabsf(fOldScale - fNewScale) < 0.001f) return;

	float fFocalScreenX = __GetAtlasHelperViewportW() * 0.5f;
	float fFocalScreenY = __GetAtlasHelperViewportH() * 0.5f;

	CInstanceBase* pkInst = CPythonCharacterManager::Instance().GetMainInstancePtr();
	if (pkInst)
	{
		TPixelPosition kInstPos;
		pkInst->NEW_GetPixelPosition(&kInstPos);
		__CalcAtlasHelperScreenPos(kInstPos.x, kInstPos.y, &fFocalScreenX, &fFocalScreenY);
	}

	if (fOldScale > 0.001f)
	{
		const float fRatio = fNewScale / fOldScale;
		m_fAtlasRenderPosX = (fFocalScreenX + m_fAtlasRenderPosX) * fRatio - fFocalScreenX;
		m_fAtlasRenderPosY = (fFocalScreenY + m_fAtlasRenderPosY) * fRatio - fFocalScreenY;
	}
	m_fAtlasHelperScale = fNewScale;
	__ClampAtlasRenderPos();
}

void CPythonMiniMap::GetAtlasRenderPos(float* pfX, float* pfY) const
{
	if (pfX) *pfX = m_fAtlasRenderPosX;
	if (pfY) *pfY = m_fAtlasRenderPosY;
}

void CPythonMiniMap::__SetAtlasHelperExpandedCenter(CGraphicExpandedImageInstance& rkImg, float fCenterX, float fCenterY, bool bUseRotationPivot) const
{
	if (rkImg.IsEmpty()) return;
	if (bUseRotationPivot)
	{
		rkImg.SetOrigin();
		rkImg.SetPosition(fCenterX - (float)rkImg.GetWidth() * 0.5f, fCenterY - (float)rkImg.GetHeight() * 0.5f);
		return;
	}
	rkImg.SetOrigin(0.0f, 0.0f);
	rkImg.SetPosition(fCenterX - (float)rkImg.GetWidth() * 0.5f + 0.5f, fCenterY - (float)rkImg.GetHeight() * 0.5f + 0.5f);
}

bool CPythonMiniMap::__IsAtlasHelperShowingCurrentMap() const
{
	if (m_strAtlasHelperDisplayMapName.empty()) return true;
	CPythonBackground& rkBG = CPythonBackground::Instance();
	if (!rkBG.IsMapOutdoor()) return false;
	return 0 == _stricmp(m_strAtlasHelperDisplayMapName.c_str(), rkBG.GetMapOutdoorRef().GetName().c_str());
}

bool CPythonMiniMap::__IsAtlasMarkAtSelectedNpc(const TAtlasMarkInfo& rInfo) const
{
	if (!m_bNPCSelected) return false;
	const float fDx = rInfo.m_fX - static_cast<float>(m_lSelectedNPCX);
	const float fDy = rInfo.m_fY - static_cast<float>(m_lSelectedNPCY);
	return ((fDx * fDx + fDy * fDy) <= (150.0f * 150.0f));
}

void CPythonMiniMap::CenterAtlasOnMapPos(float fMapX, float fMapY)
{
	float fImgX = 0.0f, fImgY = 0.0f;
	__MapPosToHelperAtlasImagePos(fMapX, fMapY, &fImgX, &fImgY);
	const float fScale = (m_fAtlasHelperScale > 0.001f) ? m_fAtlasHelperScale : 1.0f;
	m_fAtlasRenderPosX = fImgX * fScale - __GetAtlasHelperViewportW() * 0.5f;
	m_fAtlasRenderPosY = fImgY * fScale - __GetAtlasHelperViewportH() * 0.5f;
	__ClampAtlasRenderPos();
}

void CPythonMiniMap::__RenderAtlasHelperPlayerMark(RECT* pClipRect)
{
	if (!__IsAtlasHelperShowingCurrentMap()) return;

	RECT rcImageClip;
	const RECT* pImageClip = pClipRect;
	if (!pImageClip) { __GetAtlasHelperImageClipRect(&rcImageClip); pImageClip = &rcImageClip; }

	const float fCenterX = m_fAtlasHelperPlayerScreenX;
	const float fCenterY = m_fAtlasHelperPlayerScreenY;

	if (!m_bNPCSelected)
	{
		const int iFrame = static_cast<int>((ELTimer_GetMSec() / 120) % ATLAS_HELPER_MY_POSITION_IMAGE_COUNT);
		if (iFrame >= 0 && iFrame < ATLAS_HELPER_MY_POSITION_IMAGE_COUNT)
		{
			CGraphicExpandedImageInstance& rkCircle = m_AtlasHelperMyPositionMarks[iFrame];
			if (!rkCircle.IsEmpty())
			{
				const float fHalfW = (float)rkCircle.GetWidth() * 0.5f;
				const float fHalfH = (float)rkCircle.GetHeight() * 0.5f;
				rkCircle.SetOrigin(0.0f, 0.0f);
				rkCircle.SetPosition(fCenterX - fHalfW, fCenterY - fHalfH);
				if (!AtlasHelperMarkCulled(fCenterX, fCenterY, fHalfW, fHalfH, pImageClip))
				{
#if defined(ENABLE_CLIP_MASK)
					RECT rcClip = *pImageClip; rkCircle.Render(&rcClip);
#else
					rkCircle.Render();
#endif
				}
			}
		}
	}

	if (!m_AtlasPlayerMark.IsEmpty())
	{
		STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF);
		const float fPlayerMarkScale = static_cast<float>(ATLAS_HELPER_PLAYER_MARK_SCALE_PERCENT) / 100.0f;
		const float fMarkHalfW = (float)m_AtlasPlayerMark.GetWidth() * 0.5f * fPlayerMarkScale;
		const float fMarkHalfH = (float)m_AtlasPlayerMark.GetHeight() * 0.5f * fPlayerMarkScale;
		const float fRotHalfW = AtlasHelperMarkRotatedHalfExtent(fMarkHalfW, fMarkHalfH, m_fAtlasHelperPlayerRotation);
		const float fRotHalfH = AtlasHelperMarkRotatedHalfExtent(fMarkHalfW, fMarkHalfH, m_fAtlasHelperPlayerRotation + 90.0f);

		if (!AtlasHelperMarkCulled(fCenterX, fCenterY, fRotHalfW, fRotHalfH, pImageClip) && AtlasHelperMarkFullyInside(fCenterX, fCenterY, fRotHalfW, fRotHalfH, pImageClip))
		{
			m_AtlasPlayerMark.SetScale(fPlayerMarkScale, fPlayerMarkScale);
			__SetAtlasHelperExpandedCenter(m_AtlasPlayerMark, fCenterX, fCenterY, true);
			m_AtlasPlayerMark.Render();
			m_AtlasPlayerMark.SetScale(1.0f, 1.0f);
		}
	}
}

void CPythonMiniMap::__RenderAtlasHelperSelectedNpcMark(RECT* pClipRect)
{
	if (!m_bNPCSelected) return;
	float fMarkX = 0.0f, fMarkY = 0.0f;
	__MapPosToHelperAtlasImagePos(static_cast<float>(m_lSelectedNPCX), static_cast<float>(m_lSelectedNPCY), &fMarkX, &fMarkY);
	const int iMarkX = static_cast<int>(fMarkX);
	const int iMarkY = static_cast<int>(fMarkY);

	STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF);
#if defined(ENABLE_CLIP_MASK)
	__RenderWayPointMark(iMarkX, iMarkY, pClipRect);
	__RenderMiniWayPointMark(iMarkX, iMarkY, pClipRect);
#else
	__RenderWayPointMark(iMarkX, iMarkY);
	__RenderMiniWayPointMark(iMarkX, iMarkY);
#endif

	const DWORD dwInCycle = ELTimer_GetMSec() % 5000;
	if (dwInCycle <= 4900 && !m_WhiteMark.IsEmpty())
	{
		const float fOrangeScale = static_cast<float>(ATLAS_HELPER_ORANGE_MARK_SCALE_PERCENT) / 100.0f;
		const float fHalfW = (float)m_WhiteMark.GetWidth() * 0.5f * fOrangeScale;
		const float fHalfH = (float)m_WhiteMark.GetHeight() * 0.5f * fOrangeScale;
		const float fOrangeOffsetX = fHalfW + 3.0f;

		STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_XRGB(255, 132, 0));
		m_WhiteMark.SetScale(fOrangeScale, fOrangeScale);
		m_WhiteMark.SetPosition(fMarkX + fOrangeOffsetX - fHalfW, fMarkY - fHalfH);

		if (!AtlasHelperMarkCulled(fMarkX + fOrangeOffsetX, fMarkY, fHalfW, fHalfH, pClipRect))
		{
#if defined(ENABLE_CLIP_MASK)
			if (pClipRect) m_WhiteMark.Render(pClipRect); else m_WhiteMark.Render();
#else
			m_WhiteMark.Render();
#endif
		}
		m_WhiteMark.SetScale(1.0f, 1.0f);
	}
	STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, 0xFFFFFFFF);
}

void CPythonMiniMap::__ClampAtlasRenderPos()
{
	float fMinX = 0.0f, fMaxX = 0.0f, fMinY = 0.0f, fMaxY = 0.0f;
	__GetAtlasHelperPanExtents(fMinX, fMaxX, fMinY, fMaxY);

	if (m_fAtlasRenderPosX < fMinX) m_fAtlasRenderPosX = fMinX;
	if (m_fAtlasRenderPosX > fMaxX) m_fAtlasRenderPosX = fMaxX;
	if (m_fAtlasRenderPosY < fMinY) m_fAtlasRenderPosY = fMinY;
	if (m_fAtlasRenderPosY > fMaxY) m_fAtlasRenderPosY = fMaxY;
}

void CPythonMiniMap::CenterAtlasRenderPos()
{
	float fMinX = 0.0f, fMaxX = 0.0f, fMinY = 0.0f, fMaxY = 0.0f;
	__GetAtlasHelperPanExtents(fMinX, fMaxX, fMinY, fMaxY);
	m_fAtlasRenderPosX = 0.5f * (fMinX + fMaxX);
	m_fAtlasRenderPosY = 0.5f * (fMinY + fMaxY);
}

void CPythonMiniMap::SetAtlasScale(float fScale)
{
	if (fScale < 0.58f) fScale = 0.58f;
	if (fScale > 3.0f) fScale = 3.0f;
	m_fAtlasHelperScale = fScale;
	__ClampAtlasRenderPos();
}

float CPythonMiniMap::GetAtlasScale() const { return m_fAtlasHelperScale; }

void CPythonMiniMap::AtlasScaleUp()
{
	if (!m_bAtlasNpcHelperView) return;
	__ApplyAtlasHelperScaleAtPlayerFocal(__GetNextAtlasHelperScale(m_fAtlasHelperScale, true));
}

void CPythonMiniMap::AtlasScaleDown()
{
	if (!m_bAtlasNpcHelperView) return;
	__ApplyAtlasHelperScaleAtPlayerFocal(__GetNextAtlasHelperScale(m_fAtlasHelperScale, false));
}

void CPythonMiniMap::ChangeAtlasRenderPos(float fX, float fY) { m_fAtlasRenderPosX = fX; m_fAtlasRenderPosY = fY; __ClampAtlasRenderPos(); }

void CPythonMiniMap::GetAtlasPanExtents(float* pfMinX, float* pfMaxX, float* pfMinY, float* pfMaxY) const
{
	float fMinX = 0.0f, fMaxX = 0.0f, fMinY = 0.0f, fMaxY = 0.0f;
	__GetAtlasHelperPanExtents(fMinX, fMaxX, fMinY, fMaxY);
	if (pfMinX) *pfMinX = fMinX;
	if (pfMaxX) *pfMaxX = fMaxX;
	if (pfMinY) *pfMinY = fMinY;
	if (pfMaxY) *pfMaxY = fMaxY;
}

void CPythonMiniMap::GetAtlasMaxPos(float* pfMaxX, float* pfMaxY) const { GetAtlasPanExtents(NULL, pfMaxX, NULL, pfMaxY); }

void CPythonMiniMap::SetAtlasHelperViewportSize(float fWidth, float fHeight)
{
	if (fWidth < static_cast<float>(SQUARED_ATLAS_SIZE)) fWidth = static_cast<float>(SQUARED_ATLAS_SIZE);
	if (fHeight < static_cast<float>(SQUARED_ATLAS_SIZE)) fHeight = static_cast<float>(SQUARED_ATLAS_SIZE);
	m_fAtlasHelperViewportW = fWidth;
	m_fAtlasHelperViewportH = fHeight;
	__ClampAtlasRenderPos();
}

void CPythonMiniMap::GetAtlasHelperViewportSize(float* pfWidth, float* pfHeight) const
{
	if (pfWidth) *pfWidth = __GetAtlasHelperViewportW();
	if (pfHeight) *pfHeight = __GetAtlasHelperViewportH();
}

void CPythonMiniMap::SetAtlasScreenOrigin(float fScreenX, float fScreenY)
{
	m_fAtlasScreenX = fScreenX;
	m_fAtlasScreenY = fScreenY;
	m_matWorldAtlas._41 = fScreenX;
	m_matWorldAtlas._42 = fScreenY;
}

void CPythonMiniMap::ConvertGlobalPosToAtlasPos(float fGlobalX, float fGlobalY, float* pfAtlasX, float* pfAtlasY) const
{
	__CalcAtlasHelperScreenPos(fGlobalX, fGlobalY, pfAtlasX, pfAtlasY);
}

float CPythonMiniMap::GetAtlasWidthHeightRatio() const
{
	if (m_fAtlasImageSizeY <= 0.0f) return 1.0f;
	return m_fAtlasImageSizeX / m_fAtlasImageSizeY;
}

void CPythonMiniMap::SetAtlasNpcHelperView(bool bNpcHelperView)
{
	if (m_bAtlasNpcHelperView == bNpcHelperView) return;
	m_bAtlasNpcHelperView = bNpcHelperView;

	CPythonBackground& rkBG = CPythonBackground::Instance();
	if (!rkBG.IsMapOutdoor()) return;

	CMapOutdoor& rkMap = rkBG.GetMapOutdoorRef();
	char atlasFileName[1024 + 1];
	BuildAtlasSubFileName(atlasFileName, sizeof(atlasFileName), rkMap.GetName().c_str(), m_bAtlasNpcHelperView);

	m_AtlasImageInstance.Destroy();
	CGraphicImage* pkGrpImgAtlas = (CGraphicImage*)CResourceManager::Instance().GetResourcePointer(atlasFileName);
	if (pkGrpImgAtlas)
	{
		m_AtlasImageInstance.SetImagePointer(pkGrpImgAtlas);
		m_bAtlas = !pkGrpImgAtlas->IsEmpty();
	}
	else
	{
		m_bAtlas = false;
	}

	m_fAtlasImageSizeX = (float)m_AtlasImageInstance.GetWidth();
	m_fAtlasImageSizeY = (float)m_AtlasImageInstance.GetHeight();

	float fMarkWidth = (float)m_WhiteMark.GetWidth() / 2.0f;
	float fMarkHeight = (float)m_WhiteMark.GetHeight() / 2.0f;

	for (size_t i = 0; i < m_AtlasNPCInfoVector.size(); ++i)
	{
		m_AtlasNPCInfoVector[i].m_fScreenX = m_AtlasNPCInfoVector[i].m_fX / m_fAtlasMaxX * m_fAtlasImageSizeX - fMarkWidth;
		m_AtlasNPCInfoVector[i].m_fScreenY = m_AtlasNPCInfoVector[i].m_fY / m_fAtlasMaxY * m_fAtlasImageSizeY - fMarkHeight;
	}
	for (size_t i = 0; i < m_AtlasWarpInfoVector.size(); ++i)
	{
		m_AtlasWarpInfoVector[i].m_fScreenX = m_AtlasWarpInfoVector[i].m_fX / m_fAtlasMaxX * m_fAtlasImageSizeX - fMarkWidth;
		m_AtlasWarpInfoVector[i].m_fScreenY = m_AtlasWarpInfoVector[i].m_fY / m_fAtlasMaxY * m_fAtlasImageSizeY - fMarkHeight;
	}
	// --- FIX: We must recalculate the flags/waypoints too! ---
	for (size_t i = 0; i < m_AtlasWayPointInfoVector.size(); ++i)
	{
		m_AtlasWayPointInfoVector[i].m_fScreenX = m_AtlasWayPointInfoVector[i].m_fX / m_fAtlasMaxX * m_fAtlasImageSizeX - fMarkWidth;
		m_AtlasWayPointInfoVector[i].m_fScreenY = m_AtlasWayPointInfoVector[i].m_fY / m_fAtlasMaxY * m_fAtlasImageSizeY - fMarkHeight;
	}

	m_fAtlasHelperScale = 1.0f;
	m_fAtlasRenderPosX = 0.0f;
	m_fAtlasRenderPosY = 0.0f;
	__ClampAtlasRenderPos();
}

void CPythonMiniMap::TickNPCLocationDirectionEffect() { __UpdateNPCLocationDirectionEffect(); }

void CPythonMiniMap::__ClearNPCLocationDirectionEffect()
{
	if (m_iNPCLocationDirectionEffectIndex >= 0)
	{
		CEffectManager::Instance().DestroyEffectInstance(static_cast<DWORD>(m_iNPCLocationDirectionEffectIndex));
		m_iNPCLocationDirectionEffectIndex = -1;
	}
}

bool CPythonMiniMap::__EnsureNPCLocationDirectionEffectRegistered()
{
	if (m_dwNPCLocationDirectionEffectID != 0) return true;
	DWORD dwID = 0;
	if (!CEffectManager::Instance().RegisterEffect2(c_szNPCLocationDirectionEffect, &dwID, true)) return false;
	m_dwNPCLocationDirectionEffectID = dwID;
	return (m_dwNPCLocationDirectionEffectID != 0);
}

void CPythonMiniMap::__SpawnNPCLocationDirectionEffect()
{
	__ClearNPCLocationDirectionEffect();
	if (!__EnsureNPCLocationDirectionEffectRegistered()) return;

	CInstanceBase* pkInstMain = CPythonCharacterManager::Instance().GetMainInstancePtr();
	if (!pkInstMain) return;

	TPixelPosition kPlayerPos;
	pkInstMain->NEW_GetPixelPosition(&kPlayerPos);

	const float fTargetX = static_cast<float>(m_lSelectedNPCX);
	const float fTargetY = static_cast<float>(m_lSelectedNPCY);

	D3DXMATRIX matEffect;
	__BuildNPCLocationDirectionEffectMatrix(kPlayerPos, fTargetX, fTargetY, &matEffect);

	const D3DXVECTOR3 v3Rot(0.0f, 0.0f, 0.0f);
	CEffectManager& rkEftMgr = CEffectManager::Instance();
	m_iNPCLocationDirectionEffectIndex = rkEftMgr.CreateEffect(
		m_dwNPCLocationDirectionEffectID,
		D3DXVECTOR3(matEffect._41, matEffect._42, matEffect._43),
		v3Rot);

	if (m_iNPCLocationDirectionEffectIndex < 0) return;
	if (!rkEftMgr.SelectEffectInstance(static_cast<DWORD>(m_iNPCLocationDirectionEffectIndex)))
	{
		__ClearNPCLocationDirectionEffect();
		return;
	}

	rkEftMgr.SetEffectInstanceGlobalMatrix(matEffect);
	m_dwNPCLocationDirectionEffectStartMS = ELTimer_GetMSec();
}

void CPythonMiniMap::__UpdateNPCLocationDirectionEffect()
{
	if (!m_bNPCSelected) return;
// Removed custom GetMapIndex check

	CInstanceBase* pkInstMain = CPythonCharacterManager::Instance().GetMainInstancePtr();
	if (!pkInstMain)
	{
		__ClearNPCLocationDirectionEffect();
		return;
	}

	TPixelPosition kPlayerPos;
	pkInstMain->NEW_GetPixelPosition(&kPlayerPos);

	const float fTargetX = static_cast<float>(m_lSelectedNPCX);
	const float fTargetY = static_cast<float>(m_lSelectedNPCY);
	const float fDx = kPlayerPos.x - fTargetX;
	const float fDy = kPlayerPos.y - fTargetY;

	if (ELTimer_GetMSec() - m_dwNPCLocationDirectionEffectStartMS >= c_dwNPCLocationDirectionGraceMS)
	{
		if ((fDx * fDx + fDy * fDy) <= (c_fNPCLocationDirectionArriveRadius * c_fNPCLocationDirectionArriveRadius))
		{
			UnselectNPC();
			return;
		}
	}

	if (m_iNPCLocationDirectionEffectIndex < 0)
	{
		__SpawnNPCLocationDirectionEffect();
		return;
	}

	CEffectManager& rkEftMgr = CEffectManager::Instance();
	if (!rkEftMgr.IsAliveEffect(static_cast<DWORD>(m_iNPCLocationDirectionEffectIndex)))
	{
		__ClearNPCLocationDirectionEffect();
		__SpawnNPCLocationDirectionEffect();
		return;
	}

	if (!rkEftMgr.SelectEffectInstance(static_cast<DWORD>(m_iNPCLocationDirectionEffectIndex)))
	{
		__ClearNPCLocationDirectionEffect();
		return;
	}

	D3DXMATRIX matEffect;
	__BuildNPCLocationDirectionEffectMatrix(kPlayerPos, fTargetX, fTargetY, &matEffect);
	rkEftMgr.SetEffectInstanceGlobalMatrix(matEffect);
}

void CPythonMiniMap::SelectNPC(long lMapIndex, DWORD dwVnum, long lX, long lY)
{
	RemoveWayPoint(NPC_LOCATION_HELPER_TARGET_WAYPOINT);

	m_bNPCSelected = true;
	m_lSelectedMapIndex = lMapIndex;
	m_dwSelectedNPCVnum = dwVnum;
	m_lSelectedNPCX = lX;
	m_lSelectedNPCY = lY;

	m_dwNPCLocationDirectionEffectStartMS = ELTimer_GetMSec();
	__SpawnNPCLocationDirectionEffect();
}

void CPythonMiniMap::UnselectNPC()
{
	RemoveWayPoint(NPC_LOCATION_HELPER_TARGET_WAYPOINT);
	m_bNPCSelected = false;
	m_lSelectedMapIndex = 0;
	m_dwSelectedNPCVnum = 0;
	m_lSelectedNPCX = 0;
	m_lSelectedNPCY = 0;
	__ClearNPCLocationDirectionEffect();
}

bool CPythonMiniMap::IsNPCSelected() const { return m_bNPCSelected; }

void CPythonMiniMap::GetSelectedNPCPos(long* plX, long* plY) const
{
	if (plX) *plX = m_lSelectedNPCX;
	if (plY) *plY = m_lSelectedNPCY;
}
#endif
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
