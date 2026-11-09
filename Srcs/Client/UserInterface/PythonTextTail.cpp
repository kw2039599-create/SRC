#include "stdafx.h"
#include "InstanceBase.h"
#include "resource.h"
#include "PythonTextTail.h"
#include "PythonCharacterManager.h"
#include "PythonGuild.h"
#include "Locale.h"
#include "MarkManager.h"
#include <algorithm>
#include <filesystem>

const D3DXCOLOR c_TextTail_Player_Color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
const D3DXCOLOR c_TextTail_Monster_Color = D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f);
const D3DXCOLOR c_TextTail_Item_Color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
const D3DXCOLOR c_TextTail_Chat_Color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
const D3DXCOLOR c_TextTail_Info_Color = D3DXCOLOR(1.0f, 0.785f, 0.785f, 1.0f);
const D3DXCOLOR c_TextTail_Guild_Name_Color = 0xFFEFD3FF;
const float c_TextTail_Name_Position = -10.0f;
const float c_fxMarkPosition = 1.5f;
const float c_fyGuildNamePosition = 15.0f;
const float c_fyMarkPosition = 15.0f + 11.0f;
BOOL bPKTitleEnable = TRUE;

// TEXTTAIL_LIVINGTIME_CONTROL
long gs_TextTail_LivingTime = 5000;

long TextTail_GetLivingTime()
{
	assert(gs_TextTail_LivingTime>1000);
	return gs_TextTail_LivingTime;
}

void TextTail_SetLivingTime(long livingTime)
{
	gs_TextTail_LivingTime = livingTime;
}
// END_OF_TEXTTAIL_LIVINGTIME_CONTROL

CGraphicText * ms_pFont = NULL;

void CPythonTextTail::GetInfo(std::string* pstInfo)
{
	char szInfo[256];
	sprintf(szInfo, "TextTail: ChatTail %zu, ChrTail (Map %zu, List %zu), ItemTail (Map %zu, List %zu), Pool %zu",
		m_ChatTailMap.size(),
		m_CharacterTextTailMap.size(), m_CharacterTextTailList.size(),
		m_ItemTextTailMap.size(), m_ItemTextTailList.size(),
		m_TextTailPool.GetCapacity());

	pstInfo->append(szInfo);
}

void CPythonTextTail::UpdateAllTextTail()
{
	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();
	if (pInstance)
	{
		TPixelPosition pixelPos;
		pInstance->NEW_GetPixelPosition(&pixelPos);

		for (TTextTailMap::iterator itorMap = m_CharacterTextTailMap.begin(); itorMap != m_CharacterTextTailMap.end(); ++itorMap)
		{
			UpdateDistance(pixelPos, itorMap->second);
		}

		for (TTextTailMap::iterator itorMap = m_ItemTextTailMap.begin(); itorMap != m_ItemTextTailMap.end(); ++itorMap)
		{
			UpdateDistance(pixelPos, itorMap->second);
		}

		for (TChatTailMap::iterator itorChat = m_ChatTailMap.begin(); itorChat != m_ChatTailMap.end(); ++itorChat)
		{
			UpdateDistance(pixelPos, itorChat->second);

			if (itorChat->second->bNameFlag)
			{
				DWORD dwVID = itorChat->first;
				ShowCharacterTextTail(dwVID);
			}
		}
	}
}

void CPythonTextTail::UpdateShowingTextTail()
{
	CPythonGraphic::Instance().Identity();

	for (size_t i = 0; i < m_ItemTextTailList.size(); ++i)
	{
		UpdateTextTail(m_ItemTextTailList[i]);
	}

	for (TChatTailMap::iterator itorChat = m_ChatTailMap.begin(); itorChat != m_ChatTailMap.end(); ++itorChat)
	{
		UpdateTextTail(itorChat->second);
	}

	const bool bHasChatTails = !m_ChatTailMap.empty();

	for (size_t i = 0; i < m_CharacterTextTailList.size(); ++i)
	{
		TTextTail * pTextTail = m_CharacterTextTailList[i];
		UpdateTextTail(pTextTail);

		if (bHasChatTails)
		{
			TChatTailMap::iterator itor = m_ChatTailMap.find(pTextTail->dwVirtualID);
			if (m_ChatTailMap.end() != itor)
			{
				TTextTail * pChatTail = itor->second;
				if (pChatTail->bNameFlag)
				{
					pTextTail->y = pChatTail->y - 17.0f;
				}
			}
		}
	}
}

#include "../EterLib/Camera.h"

void CPythonTextTail::UpdateTextTail(TTextTail * pTextTail)
{
	if (!pTextTail->pOwner)
		return;

	const D3DXVECTOR3 & c_rv3Position = pTextTail->pOwner->GetPosition();
	const D3DXVECTOR3 v3WorldPos(c_rv3Position.x, c_rv3Position.y, c_rv3Position.z + pTextTail->fHeight);

	// Cull any TextTail whose 3D anchor is behind the camera plane (fixes inverted rear projection)
	if (CCamera* pCurCam = CCameraManager::Instance().GetCurrentCamera())
	{
		const D3DXVECTOR3 v3ToPoint = v3WorldPos - pCurCam->GetEye();
		if (D3DXVec3Dot(&v3ToPoint, &pCurCam->GetView()) <= 15.0f)
		{
			pTextTail->x = -10000.0f;
			pTextTail->y = -10000.0f;
			pTextTail->z = 1.0f;
			return;
		}
	}

	CPythonGraphic & rpyGraphic = CPythonGraphic::Instance();

	float fx, fy, fz;
	rpyGraphic.ProjectPosition(v3WorldPos.x,
							   v3WorldPos.y,
							   v3WorldPos.z,
							   &fx,
							   &fy,
							   &fz);

	if (fz < 0.0f || fz > 1.0f)
	{
		pTextTail->x = -10000.0f;
		pTextTail->y = -10000.0f;
		pTextTail->z = 1.0f;
		return;
	}

	fx = floorf(fx);
	fy = floorf(fy);

	if (pTextTail->fDistanceFromPlayer < 1300.0f)
	{
		fz = 0.0f;
	}
	else
	{
		fz = fz * rpyGraphic.GetOrthoDepth() * -1.0f + 10.0f;
	}

	pTextTail->x = fx;
	pTextTail->y = fy;
	pTextTail->z = fz;
}

void CPythonTextTail::ArrangeTextTail()
{
	DWORD dwTime = CTimer::Instance().GetCurrentMillisecond();

	const size_t itemCount = m_ItemTextTailList.size();
	if (itemCount > 0)
	{
		static std::vector<TTextTail*> s_xOverlapCandidates;

		for (size_t i = 0; i < itemCount; ++i)
		{
			TTextTail * pInsertTextTail = m_ItemTextTailList[i];

			s_xOverlapCandidates.clear();
			const float x1Source = pInsertTextTail->x + pInsertTextTail->xStart;
			const float x2Source = pInsertTextTail->x + pInsertTextTail->xEnd;

			for (size_t j = 0; j < itemCount; ++j)
			{
				if (i == j)
					continue;

				TTextTail * pCompare = m_ItemTextTailList[j];
				if (x1Source <= pCompare->x + pCompare->xEnd && x2Source >= pCompare->x + pCompare->xStart)
				{
					s_xOverlapCandidates.push_back(pCompare);
				}
			}

			if (!s_xOverlapCandidates.empty())
			{
				const int yTemp = 5;
				int LimitCount = 0;
				const size_t candCount = s_xOverlapCandidates.size();

				for (size_t k = 0; k < candCount;)
				{
					if (LimitCount >= 20)
						break;

					TTextTail * pCompareTextTail = s_xOverlapCandidates[k];
					const float y1Source = pInsertTextTail->y + pInsertTextTail->yStart;
					const float y2Source = pInsertTextTail->y + pInsertTextTail->yEnd;

					if (y1Source <= pCompareTextTail->y + pCompareTextTail->yEnd &&
						y2Source >= pCompareTextTail->y + pCompareTextTail->yStart)
					{
						pInsertTextTail->y = (pCompareTextTail->y + pCompareTextTail->yEnd + yTemp);
						k = 0;
						++LimitCount;
						continue;
					}

					++k;
				}
			}

			if (pInsertTextTail->pOwnerTextInstance)
			{
				pInsertTextTail->pOwnerTextInstance->SetPosition(pInsertTextTail->x, pInsertTextTail->y, pInsertTextTail->z);
				pInsertTextTail->pOwnerTextInstance->Update();

				pInsertTextTail->pTextInstance->SetColor(pInsertTextTail->Color.r, pInsertTextTail->Color.g, pInsertTextTail->Color.b);
				pInsertTextTail->pTextInstance->SetPosition(pInsertTextTail->x, pInsertTextTail->y + 15.0f, pInsertTextTail->z);
				pInsertTextTail->pTextInstance->Update();
			}
			else
			{
				pInsertTextTail->pTextInstance->SetColor(pInsertTextTail->Color.r, pInsertTextTail->Color.g, pInsertTextTail->Color.b);
				pInsertTextTail->pTextInstance->SetPosition(pInsertTextTail->x, pInsertTextTail->y, pInsertTextTail->z);
				pInsertTextTail->pTextInstance->Update();
			}
		}
	}

	const bool bIsArabic = (GetDefaultCodePage() == CP_ARABIC);
	const size_t chrCount = m_CharacterTextTailList.size();

	for (size_t i = 0; i < chrCount; ++i)
	{
		TTextTail * pTextTail = m_CharacterTextTailList[i];

		float fxAdd = 0.0f;

		CGraphicMarkInstance * pMarkInstance = pTextTail->pMarkInstance;
		CGraphicTextInstance * pGuildNameInstance = pTextTail->pGuildNameTextInstance;
#if defined(WJ_COMBAT_ZONE)
		CGraphicImageInstance * pCombatZoneRank = pTextTail->pCombatZoneRankInstance;
#endif
		if (pMarkInstance && pGuildNameInstance)
		{
			int iWidth, iHeight;
			int iImageHalfSize = pMarkInstance->GetWidth()/2 + c_fxMarkPosition;
			pGuildNameInstance->GetTextSize(&iWidth, &iHeight);

			pMarkInstance->SetPosition(pTextTail->x - iWidth/2 - iImageHalfSize, pTextTail->y - c_fyMarkPosition);
			pGuildNameInstance->SetPosition(pTextTail->x + iImageHalfSize, pTextTail->y - c_fyGuildNamePosition, pTextTail->z);
			pGuildNameInstance->Update();
		}

		int iNameWidth, iNameHeight;
		pTextTail->pTextInstance->GetTextSize(&iNameWidth, &iNameHeight);
#if defined(WJ_COMBAT_ZONE)
		if (pCombatZoneRank)
		{
		#if defined(__NEW_TITLE__) || defined(ENABLE_TITLE_SYSTEM)
			if (pGuildNameInstance && pMrVegaS_Titan2)
			{
				pCombatZoneRank->SetPosition (pTextTail->x - pCombatZoneRank->GetWidth()/2, pTextTail->y - (c_fyMarkPosition * 4) + 3.0f);
			}
			else if (pGuildNameInstance && !pMrVegaS_Titan2)
			{
				pCombatZoneRank->SetPosition (pTextTail->x - pCombatZoneRank->GetWidth()/2, pTextTail->y - (c_fyMarkPosition * 4) + 15.0f);
			}
			else if (pMrVegaS_Titan2 && !pGuildNameInstance)
			{
				pCombatZoneRank->SetPosition (pTextTail->x - pCombatZoneRank->GetWidth()/2, pTextTail->y - (c_fyMarkPosition * 4) + 15.0f);
			}
			else
			{
				pCombatZoneRank->SetPosition (pTextTail->x - pCombatZoneRank->GetWidth()/2, pTextTail->y - (c_fyMarkPosition * 4) + 30.0f);
			}
		#else
			if (pGuildNameInstance)
			{
				pCombatZoneRank->SetPosition (pTextTail->x - pCombatZoneRank->GetWidth()/2, pTextTail->y - (c_fyMarkPosition * 4) + 15.0f);
			}
			else
			{
				pCombatZoneRank->SetPosition (pTextTail->x - pCombatZoneRank->GetWidth()/2, pTextTail->y - (c_fyMarkPosition * 4) + 30.0f);
			}
		#endif
		}
#endif
		CGraphicTextInstance * pTitle = pTextTail->pTitleTextInstance;
		if (pTitle)
		{
			int iTitleWidth, iTitleHeight;
			pTitle->GetTextSize(&iTitleWidth, &iTitleHeight);

			fxAdd = 8.0f;

			if (bIsArabic)
				pTitle->SetPosition(pTextTail->x - (iNameWidth / 2) - iTitleWidth - 4.0f, pTextTail->y, pTextTail->z);
			else
				pTitle->SetPosition(pTextTail->x - (iNameWidth / 2) + 4.0f, pTextTail->y, pTextTail->z);

			pTitle->Update();

			CGraphicTextInstance * pLevel = pTextTail->pLevelTextInstance;
			if (pLevel)
			{
				int iLevelWidth, iLevelHeight;
				pLevel->GetTextSize(&iLevelWidth, &iLevelHeight);

				if (bIsArabic)
					pLevel->SetPosition(pTextTail->x - (iNameWidth / 2) - iLevelWidth - iTitleWidth - 8.0f, pTextTail->y, pTextTail->z);
				else
					pLevel->SetPosition(pTextTail->x - (iNameWidth / 2) - iTitleWidth, pTextTail->y, pTextTail->z);

				pLevel->Update();
			}
		}
		else
		{
			fxAdd = 4.0f;

			CGraphicTextInstance * pLevel = pTextTail->pLevelTextInstance;
			if (pLevel)
			{
				int iLevelWidth, iLevelHeight;
				pLevel->GetTextSize(&iLevelWidth, &iLevelHeight);

				if (bIsArabic)
					pLevel->SetPosition(pTextTail->x - (iNameWidth / 2) - iLevelWidth - 4.0f, pTextTail->y, pTextTail->z);
				else
					pLevel->SetPosition(pTextTail->x - (iNameWidth / 2) - 2.0f, pTextTail->y, pTextTail->z);

				pLevel->Update();
			}
		}

		pTextTail->pTextInstance->SetColor(pTextTail->Color.r, pTextTail->Color.g, pTextTail->Color.b);
		pTextTail->pTextInstance->SetPosition(pTextTail->x + fxAdd, pTextTail->y, pTextTail->z);
		pTextTail->pTextInstance->Update();

#ifdef ENABLE_AFK_MODE_SYSTEM
		if (pTextTail->pAfkTextInstance)
		{
			pTextTail->pAfkTextInstance->SetPosition(pTextTail->x, pTextTail->y + 13.0f, pTextTail->z);
			pTextTail->pAfkTextInstance->Update();
		}
#endif
	}

	for (TChatTailMap::iterator itorChat = m_ChatTailMap.begin(); itorChat != m_ChatTailMap.end();)
	{
		TTextTail * pTextTail = itorChat->second;

		if (pTextTail->LivingTime < dwTime)
		{
			DeleteTextTail(pTextTail);
			itorChat = m_ChatTailMap.erase(itorChat);
			continue;
		}
		else
			++itorChat;

		pTextTail->pTextInstance->SetColor(pTextTail->Color);
		pTextTail->pTextInstance->SetPosition(pTextTail->x, pTextTail->y, pTextTail->z);
		pTextTail->pTextInstance->Update();
	}
}

void CPythonTextTail::Render()
{
	STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);

	const size_t chrCount = m_CharacterTextTailList.size();

	if (chrCount > 0)
	{
		CGraphicTextInstance::BeginBatchRender();

		for (size_t i = 0; i < chrCount; ++i)
		{
			TTextTail * pTextTail = m_CharacterTextTailList[i];
			if (pTextTail->x <= -5000.0f)
				continue;

			pTextTail->pTextInstance->Render();

			if (pTextTail->pMarkInstance && pTextTail->pGuildNameTextInstance)
				pTextTail->pGuildNameTextInstance->Render();

			if (pTextTail->pTitleTextInstance)
				pTextTail->pTitleTextInstance->Render();

			if (pTextTail->pLevelTextInstance)
				pTextTail->pLevelTextInstance->Render();

#ifdef ENABLE_AFK_MODE_SYSTEM
			if (pTextTail->pAfkTextInstance)
				pTextTail->pAfkTextInstance->Render();
#endif
		}

		CGraphicTextInstance::EndBatchRender();

		for (size_t i = 0; i < chrCount; ++i)
		{
			TTextTail * pTextTail = m_CharacterTextTailList[i];
			if (pTextTail->x <= -5000.0f)
				continue;

			if (pTextTail->pMarkInstance && pTextTail->pGuildNameTextInstance)
				pTextTail->pMarkInstance->Render();

#if defined(WJ_COMBAT_ZONE)
			if (pTextTail->pCombatZoneRankInstance)
				pTextTail->pCombatZoneRankInstance->Render();
#endif
		}
	}

	const size_t itemCount = m_ItemTextTailList.size();
	if (itemCount > 0)
	{
		for (size_t i = 0; i < itemCount; ++i)
		{
			if (m_ItemTextTailList[i]->x <= -5000.0f)
				continue;
			RenderTextTailBox(m_ItemTextTailList[i]);
		}

		CGraphicTextInstance::BeginBatchRender();
		for (size_t i = 0; i < itemCount; ++i)
		{
			TTextTail * pTextTail = m_ItemTextTailList[i];
			if (pTextTail->x <= -5000.0f)
				continue;

			pTextTail->pTextInstance->Render();
			if (pTextTail->pOwnerTextInstance)
				pTextTail->pOwnerTextInstance->Render();
		}
		CGraphicTextInstance::EndBatchRender();
	}

	if (!m_ChatTailMap.empty())
	{
		CGraphicTextInstance::BeginBatchRender();
		for (TChatTailMap::iterator itorChat = m_ChatTailMap.begin(); itorChat != m_ChatTailMap.end(); ++itorChat)
		{
			TTextTail * pTextTail = itorChat->second;
			if (pTextTail->pOwner->isShow() && pTextTail->x > -5000.0f)
				RenderTextTailName(pTextTail);
		}
		CGraphicTextInstance::EndBatchRender();
	}
}

void CPythonTextTail::RenderTextTailBox(TTextTail * pTextTail)
{
	CPythonGraphic::Instance().SetDiffuseColor(0.0f, 0.0f, 0.0f, 1.0f);
	CPythonGraphic::Instance().RenderBox2d(pTextTail->x + pTextTail->xStart,
										   pTextTail->y + pTextTail->yStart,
										   pTextTail->x + pTextTail->xEnd,
										   pTextTail->y + pTextTail->yEnd,
										   pTextTail->z);

	CPythonGraphic::Instance().SetDiffuseColor(0.0f, 0.0f, 0.0f, 0.3f);
	CPythonGraphic::Instance().RenderBar2d(pTextTail->x + pTextTail->xStart,
										   pTextTail->y + pTextTail->yStart,
										   pTextTail->x + pTextTail->xEnd,
										   pTextTail->y + pTextTail->yEnd,
										   pTextTail->z);
}

void CPythonTextTail::RenderTextTailName(TTextTail * pTextTail)
{
	pTextTail->pTextInstance->Render();
}

void CPythonTextTail::HideAllTextTail()
{
	for (size_t i = 0; i < m_CharacterTextTailList.size(); ++i)
		m_CharacterTextTailList[i]->bIsShown = false;

	for (size_t i = 0; i < m_ItemTextTailList.size(); ++i)
		m_ItemTextTailList[i]->bIsShown = false;

	m_CharacterTextTailList.clear();
	m_ItemTextTailList.clear();
}

void CPythonTextTail::UpdateDistance(const TPixelPosition & c_rCenterPosition, TTextTail * pTextTail)
{
	if (!pTextTail->pOwner || !pTextTail->pOwner->isShow())
	{
		pTextTail->fDistanceFromPlayer = 99999.0f;
		return;
	}

	const D3DXVECTOR3 & c_rv3Position = pTextTail->pOwner->GetPosition();
	const float dx = c_rv3Position.x - c_rCenterPosition.x;
	const float dy = -c_rv3Position.y - c_rCenterPosition.y;
	const float fDistSq = dx * dx + dy * dy;

	// Avoid sqrtf if beyond the 3500.0f display cutoff
	if (fDistSq >= (3500.0f * 3500.0f))
	{
		pTextTail->fDistanceFromPlayer = 3500.0f;
		return;
	}

	pTextTail->fDistanceFromPlayer = sqrtf(fDistSq);
}

void CPythonTextTail::ShowAllTextTail()
{
	for (TTextTailMap::iterator itor = m_CharacterTextTailMap.begin(); itor != m_CharacterTextTailMap.end(); ++itor)
	{
		TTextTail * pTextTail = itor->second;
		if (pTextTail->fDistanceFromPlayer < 3500.0f)
			__ShowCharacterTextTail(pTextTail);
	}
	for (TTextTailMap::iterator itor = m_ItemTextTailMap.begin(); itor != m_ItemTextTailMap.end(); ++itor)
	{
		TTextTail * pTextTail = itor->second;
		if (pTextTail->fDistanceFromPlayer < 3500.0f)
			__ShowItemTextTail(pTextTail);
	}
}

void CPythonTextTail::__ShowCharacterTextTail(TTextTail * pTextTail)
{
	if (pTextTail->bIsShown)
		return;

	if (!pTextTail->pOwner || !pTextTail->pOwner->isShow())
		return;

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(pTextTail->dwVirtualID);
	if (!pInstance)
		return;

	// Hide the main player's own nameplate when in First-Person camera mode (camera distance < 50.0f)
	if (pInstance == CPythonCharacterManager::Instance().GetMainInstancePtr())
	{
		if (CCamera* pCurCam = CCameraManager::Instance().GetCurrentCamera())
		{
			if (pCurCam->GetDistance() < 50.0f)
				return;
		}
	}

	if (pInstance->IsGuildWall())
		return;

#if defined(ENABLE_EVENT_BANNER_FLAG)
	if (pInstance->IsBannerFlag())
		return;
#endif

	if (pInstance->CanPickInstance())
	{
		pTextTail->bIsShown = true;
		m_CharacterTextTailList.push_back(pTextTail);
	}
}

void CPythonTextTail::ShowCharacterTextTail(DWORD VirtualID)
{
	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(VirtualID);

	if (m_CharacterTextTailMap.end() == itor)
		return;

	__ShowCharacterTextTail(itor->second);
}

void CPythonTextTail::__ShowItemTextTail(TTextTail * pTextTail)
{
	if (pTextTail->bIsShown)
		return;

	pTextTail->bIsShown = true;
	m_ItemTextTailList.push_back(pTextTail);
}

void CPythonTextTail::ShowItemTextTail(DWORD VirtualID)
{
	TTextTailMap::iterator itor = m_ItemTextTailMap.find(VirtualID);

	if (m_ItemTextTailMap.end() == itor)
		return;

	__ShowItemTextTail(itor->second);
}

bool CPythonTextTail::isIn(CPythonTextTail::TTextTail * pSource, CPythonTextTail::TTextTail * pTarget)
{
	float x1Source = pSource->x + pSource->xStart;
	float y1Source = pSource->y + pSource->yStart;
	float x2Source = pSource->x + pSource->xEnd;
	float y2Source = pSource->y + pSource->yEnd;
	float x1Target = pTarget->x + pTarget->xStart;
	float y1Target = pTarget->y + pTarget->yStart;
	float x2Target = pTarget->x + pTarget->xEnd;
	float y2Target = pTarget->y + pTarget->yEnd;

	if (x1Source <= x2Target && x2Source >= x1Target &&
	    y1Source <= y2Target && y2Source >= y1Target)
	{
		return true;
	}

	return false;
}

void CPythonTextTail::RegisterCharacterTextTail(DWORD dwGuildID, DWORD dwVirtualID, const D3DXCOLOR & c_rColor, float fAddHeight)
{
	CInstanceBase * pCharacterInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwVirtualID);

	if (!pCharacterInstance)
		return;

#if defined(WJ_COMBAT_ZONE)	&& defined(WJ_COMBAT_ZONE_HIDE_INFO_USER)
	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();
	CInstanceBase* pkInstMain = rkChrMgr.GetMainInstancePtr();

	if (pCharacterInstance->IsCombatZoneMap() && pCharacterInstance != pkInstMain)
		return;
#endif

	TTextTail * pTextTail = RegisterTextTail(dwVirtualID,
											 pCharacterInstance->GetNameString(),
											 pCharacterInstance->GetGraphicThingInstancePtr(),
											 pCharacterInstance->GetGraphicThingInstanceRef().GetHeight() + fAddHeight,
											 c_rColor);

	CGraphicTextInstance * pTextInstance = pTextTail->pTextInstance;
	pTextInstance->SetOutline(true);
	pTextInstance->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);

	pTextTail->pMarkInstance=NULL;
	pTextTail->pGuildNameTextInstance=NULL;
	pTextTail->pTitleTextInstance=NULL;
	pTextTail->pLevelTextInstance=NULL;
#if defined(WJ_COMBAT_ZONE)
	pTextTail->pCombatZoneRankInstance=NULL;
#endif

	if (0 != dwGuildID)
	{
		pTextTail->pMarkInstance = CGraphicMarkInstance::New();

		DWORD dwMarkID = CGuildMarkManager::Instance().GetMarkID(dwGuildID);
		bool bLoadedServerMark = false;

		if (dwMarkID != CGuildMarkManager::INVALID_MARK_ID)
		{
			std::string markImagePath;

			if (CGuildMarkManager::Instance().GetMarkImageFilename(dwMarkID / CGuildMarkImage::MARK_TOTAL_COUNT, markImagePath))
			{
				if (CResourceManager::Instance().IsFileExist(markImagePath.c_str()))
				{
					pTextTail->pMarkInstance->SetImageFileName(markImagePath.c_str());
					pTextTail->pMarkInstance->Load();
					pTextTail->pMarkInstance->SetIndex(dwMarkID % CGuildMarkImage::MARK_TOTAL_COUNT);
					bLoadedServerMark = true;
				}
			}
		}


		if (!bLoadedServerMark)
		{
			static std::vector<std::string> s_vecUploadMarks;
			static bool s_bScannedUploadFolder = false;

			if (!s_bScannedUploadFolder)
			{
				s_bScannedUploadFolder = true;
				std::error_code ec;
				if (std::filesystem::exists("upload", ec) && std::filesystem::is_directory("upload", ec))
				{
					for (const auto& entry : std::filesystem::directory_iterator("upload", ec))
					{
						if (entry.is_regular_file(ec))
						{
							s_vecUploadMarks.push_back(entry.path().generic_string());
						}
					}
				}
			}

			if (!s_vecUploadMarks.empty())
			{
				const size_t idx = (dwGuildID + 131u) % s_vecUploadMarks.size();
				pTextTail->pMarkInstance->SetImageFileName(s_vecUploadMarks[idx].c_str());
				pTextTail->pMarkInstance->Load();
				pTextTail->pMarkInstance->SetIndex(0);
			}
		}

		std::string strGuildName;
		if (!CPythonGuild::Instance().GetGuildName(dwGuildID, &strGuildName))
			strGuildName = "Noname";

		CGraphicTextInstance *& prGuildNameInstance = pTextTail->pGuildNameTextInstance;
		prGuildNameInstance = CGraphicTextInstance::New();
		prGuildNameInstance->SetTextPointer(ms_pFont);
		prGuildNameInstance->SetOutline(true);
		prGuildNameInstance->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
		prGuildNameInstance->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
		prGuildNameInstance->SetValue(strGuildName.c_str());
		prGuildNameInstance->SetColor(c_TextTail_Guild_Name_Color.r, c_TextTail_Guild_Name_Color.g, c_TextTail_Guild_Name_Color.b);
		prGuildNameInstance->Update();
	}

	m_CharacterTextTailMap.insert(TTextTailMap::value_type(dwVirtualID, pTextTail));
#if defined(WJ_COMBAT_ZONE)	
	if (pCharacterInstance->IsPC())
	{
		BYTE iCombatZoneRankID = pCharacterInstance->GetCombatZoneRank();
		switch (iCombatZoneRankID)
		{
			case 1:
			case 2:
			case 3:
			{
				pTextTail->pCombatZoneRankInstance = CGraphicImageInstance::New();
				char c_pszRank[256];
				sprintf(c_pszRank, "d:/ymir work/effect/etc/ranking_battle/ranker_%d.tga", iCombatZoneRankID);
				pTextTail->pCombatZoneRankInstance->SetImagePointer((CGraphicImage*)CResourceManager::Instance().GetResourcePointer(c_pszRank));
			}
			break;
		}
	}
#endif
}

void CPythonTextTail::RegisterItemTextTail(DWORD VirtualID, const char * c_szText, CGraphicObjectInstance * pOwner)
{
#ifdef __DEBUG
	char szName[256];
	spritnf(szName, "%s[%d]", c_szText, VirtualID);

	TTextTail * pTextTail = RegisterTextTail(VirtualID, c_szText, pOwner, c_TextTail_Name_Position, c_TextTail_Item_Color);
	m_ItemTextTailMap.insert(TTextTailMap::value_type(VirtualID, pTextTail));
#else
	TTextTail * pTextTail = RegisterTextTail(VirtualID, c_szText, pOwner, c_TextTail_Name_Position, c_TextTail_Item_Color);
	m_ItemTextTailMap.insert(TTextTailMap::value_type(VirtualID, pTextTail));
#endif
}

void CPythonTextTail::RegisterChatTail(DWORD VirtualID, const char * c_szChat)
{
	CInstanceBase * pCharacterInstance = CPythonCharacterManager::Instance().GetInstancePtr(VirtualID);

	if (!pCharacterInstance)
		return;

	TChatTailMap::iterator itor = m_ChatTailMap.find(VirtualID);

	if (m_ChatTailMap.end() != itor)
	{
		TTextTail * pTextTail = itor->second;

		pTextTail->pTextInstance->SetValue(c_szChat);
		pTextTail->pTextInstance->Update();
		pTextTail->Color = c_TextTail_Chat_Color;
		pTextTail->pTextInstance->SetColor(c_TextTail_Chat_Color);

		// TEXTTAIL_LIVINGTIME_CONTROL
		pTextTail->LivingTime = CTimer::Instance().GetCurrentMillisecond() + TextTail_GetLivingTime();
		// END_OF_TEXTTAIL_LIVINGTIME_CONTROL

		pTextTail->bNameFlag = TRUE;

		return;
	}

	auto baseHeight = pCharacterInstance->GetGraphicThingInstanceRef().GetHeight() + 10.0f;
#ifdef ENABLE_RACE_HEIGHT
	baseHeight += pCharacterInstance->GetBaseHeight();
#endif

	TTextTail * pTextTail = RegisterTextTail(VirtualID,
											 c_szChat,
											 pCharacterInstance->GetGraphicThingInstancePtr(),
											 baseHeight,
											 c_TextTail_Chat_Color);

	// TEXTTAIL_LIVINGTIME_CONTROL
	pTextTail->LivingTime = CTimer::Instance().GetCurrentMillisecond() + TextTail_GetLivingTime();
	// END_OF_TEXTTAIL_LIVINGTIME_CONTROL

	pTextTail->bNameFlag = TRUE;
	pTextTail->pTextInstance->SetOutline(true);
	pTextTail->pTextInstance->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
	m_ChatTailMap.emplace(VirtualID, pTextTail);
}

void CPythonTextTail::RegisterInfoTail(DWORD VirtualID, const char * c_szChat)
{
	CInstanceBase * pCharacterInstance = CPythonCharacterManager::Instance().GetInstancePtr(VirtualID);

	if (!pCharacterInstance)
		return;

	TChatTailMap::iterator itor = m_ChatTailMap.find(VirtualID);

	if (m_ChatTailMap.end() != itor)
	{
		TTextTail * pTextTail = itor->second;

		pTextTail->pTextInstance->SetValue(c_szChat);
		pTextTail->pTextInstance->Update();
		pTextTail->Color = c_TextTail_Info_Color;
		pTextTail->pTextInstance->SetColor(c_TextTail_Info_Color);

		// TEXTTAIL_LIVINGTIME_CONTROL
		pTextTail->LivingTime = CTimer::Instance().GetCurrentMillisecond() + TextTail_GetLivingTime();
		// END_OF_TEXTTAIL_LIVINGTIME_CONTROL

		pTextTail->bNameFlag = FALSE;

		return;
	}

	TTextTail * pTextTail = RegisterTextTail(VirtualID,
											 c_szChat,
											 pCharacterInstance->GetGraphicThingInstancePtr(),
											 pCharacterInstance->GetGraphicThingInstanceRef().GetHeight() + 10.0f,
											 c_TextTail_Info_Color);

	// TEXTTAIL_LIVINGTIME_CONTROL
	pTextTail->LivingTime = CTimer::Instance().GetCurrentMillisecond() + TextTail_GetLivingTime();
	// END_OF_TEXTTAIL_LIVINGTIME_CONTROL

	pTextTail->bNameFlag = FALSE;
	pTextTail->pTextInstance->SetOutline(true);
	pTextTail->pTextInstance->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
	m_ChatTailMap.insert(TTextTailMap::value_type(VirtualID, pTextTail));
}

bool CPythonTextTail::GetTextTailPosition(DWORD dwVID, float* px, float* py, float* pz)
{
	TTextTailMap::iterator itorCharacter = m_CharacterTextTailMap.find(dwVID);

	if (m_CharacterTextTailMap.end() == itorCharacter)
	{
		return false;
	}

	TTextTail * pTextTail = itorCharacter->second;
	*px=pTextTail->x;
	*py=pTextTail->y;
	*pz=pTextTail->z;

	return true;
}

bool CPythonTextTail::IsChatTextTail(DWORD dwVID)
{
	TChatTailMap::iterator itorChat = m_ChatTailMap.find(dwVID);

	if (m_ChatTailMap.end() == itorChat)
		return false;

	return true;
}

void CPythonTextTail::SetCharacterTextTailColor(DWORD VirtualID, const D3DXCOLOR & c_rColor)
{
	TTextTailMap::iterator itorCharacter = m_CharacterTextTailMap.find(VirtualID);

	if (m_CharacterTextTailMap.end() == itorCharacter)
		return;

	TTextTail * pTextTail = itorCharacter->second;
	pTextTail->pTextInstance->SetColor(c_rColor);
	pTextTail->Color = c_rColor;
}

void CPythonTextTail::SetItemTextTailOwner(DWORD dwVID, const char * c_szName)
{
	TTextTailMap::iterator itor = m_ItemTextTailMap.find(dwVID);
	if (m_ItemTextTailMap.end() == itor)
		return;

	TTextTail * pTextTail = itor->second;

	if (strlen(c_szName) > 0)
	{
		if (!pTextTail->pOwnerTextInstance)
		{
			pTextTail->pOwnerTextInstance = CGraphicTextInstance::New();
		}

		std::string strName = c_szName;
		static const string & strOwnership = ApplicationStringTable_GetString(IDS_POSSESSIVE_MORPHENE) == "" ? "'s" : ApplicationStringTable_GetString(IDS_POSSESSIVE_MORPHENE);
		strName += strOwnership;

		pTextTail->pOwnerTextInstance->SetTextPointer(ms_pFont);
		pTextTail->pOwnerTextInstance->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
		pTextTail->pOwnerTextInstance->SetValue(strName.c_str());
		pTextTail->pOwnerTextInstance->SetColor(1.0f, 1.0f, 0.0f);
		pTextTail->pOwnerTextInstance->Update();

		int xOwnerSize, yOwnerSize;
		pTextTail->pOwnerTextInstance->GetTextSize(&xOwnerSize, &yOwnerSize);
		pTextTail->yStart	= -2.0f;
		pTextTail->yEnd		+= float(yOwnerSize + 4);
		pTextTail->xStart	= fMIN(pTextTail->xStart, float(-xOwnerSize / 2 - 1));
		pTextTail->xEnd		= fMAX(pTextTail->xEnd, float(xOwnerSize / 2 + 1));
	}
	else
	{
		if (pTextTail->pOwnerTextInstance)
		{
			CGraphicTextInstance::Delete(pTextTail->pOwnerTextInstance);
			pTextTail->pOwnerTextInstance = NULL;
		}

		int xSize, ySize;
		pTextTail->pTextInstance->GetTextSize(&xSize, &ySize);
		pTextTail->xStart	= (float) (-xSize / 2 - 2);
		pTextTail->yStart	= -2.0f;
		pTextTail->xEnd		= (float) (xSize / 2 + 2);
		pTextTail->yEnd		= (float) ySize;
	}
}

void CPythonTextTail::DeleteCharacterTextTail(DWORD VirtualID)
{
	TTextTailMap::iterator itorCharacter = m_CharacterTextTailMap.find(VirtualID);
	TTextTailMap::iterator itorChat = m_ChatTailMap.find(VirtualID);

	if (m_CharacterTextTailMap.end() != itorCharacter)
	{
		TTextTail * pTextTail = itorCharacter->second;
		if (pTextTail->bIsShown)
		{
			TTextTailList::iterator itList = std::find(m_CharacterTextTailList.begin(), m_CharacterTextTailList.end(), pTextTail);
			if (itList != m_CharacterTextTailList.end())
				m_CharacterTextTailList.erase(itList);
		}
		DeleteTextTail(pTextTail);
		m_CharacterTextTailMap.erase(itorCharacter);
	}
	else
	{
		Tracenf("CPythonTextTail::DeleteCharacterTextTail - Find VID[%d] Error", VirtualID);
	}

	if (m_ChatTailMap.end() != itorChat)
	{
		DeleteTextTail(itorChat->second);
		m_ChatTailMap.erase(itorChat);
	}
}

void CPythonTextTail::DeleteItemTextTail(DWORD VirtualID)
{
	TTextTailMap::iterator itor = m_ItemTextTailMap.find(VirtualID);

	if (m_ItemTextTailMap.end() == itor)
	{
		Tracef(" CPythonTextTail::DeleteItemTextTail - None Item Text Tail\n");
		return;
	}

	TTextTail * pTextTail = itor->second;
	if (pTextTail->bIsShown)
	{
		TTextTailList::iterator itList = std::find(m_ItemTextTailList.begin(), m_ItemTextTailList.end(), pTextTail);
		if (itList != m_ItemTextTailList.end())
			m_ItemTextTailList.erase(itList);
	}
	DeleteTextTail(pTextTail);
	m_ItemTextTailMap.erase(itor);
}

CPythonTextTail::TTextTail * CPythonTextTail::RegisterTextTail(DWORD dwVirtualID, const char * c_szText, CGraphicObjectInstance * pOwner, float fHeight, const D3DXCOLOR & c_rColor)
{
	TTextTail * pTextTail = m_TextTailPool.Alloc();

	pTextTail->dwVirtualID = dwVirtualID;
	pTextTail->pOwner = pOwner;
	pTextTail->pTextInstance = CGraphicTextInstance::New();
	pTextTail->pOwnerTextInstance = NULL;
	pTextTail->fHeight = fHeight;

	pTextTail->pTextInstance->SetTextPointer(ms_pFont);
	pTextTail->pTextInstance->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
	pTextTail->pTextInstance->SetValue(c_szText);
	pTextTail->pTextInstance->SetColor(c_rColor.r, c_rColor.g, c_rColor.b);
	pTextTail->pTextInstance->Update();

	int xSize, ySize;
	pTextTail->pTextInstance->GetTextSize(&xSize, &ySize);
	pTextTail->xStart				= (float) (-xSize / 2 - 2);
	pTextTail->yStart				= -2.0f;
	pTextTail->xEnd					= (float) (xSize / 2 + 2);
	pTextTail->yEnd					= (float) ySize;
	pTextTail->Color				= c_rColor;
	pTextTail->fDistanceFromPlayer	= 0.0f;
	pTextTail->x = -100.0f;
	pTextTail->y = -100.0f;
	pTextTail->z = 0.0f;
	pTextTail->bIsShown = false;

	pTextTail->pMarkInstance = NULL;
	pTextTail->pGuildNameTextInstance = NULL;
	pTextTail->pTitleTextInstance = NULL;
	pTextTail->pLevelTextInstance = NULL;
#ifdef ENABLE_AFK_MODE_SYSTEM
	pTextTail->pAfkTextInstance = NULL;
#endif
#if defined(WJ_COMBAT_ZONE)
	pTextTail->pCombatZoneRankInstance = NULL;
#endif
	return pTextTail;
}

void CPythonTextTail::DeleteTextTail(TTextTail * pTextTail)
{
	if (pTextTail->pTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pTextInstance);
		pTextTail->pTextInstance = NULL;
	}
	if (pTextTail->pOwnerTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pOwnerTextInstance);
		pTextTail->pOwnerTextInstance = NULL;
	}
	if (pTextTail->pMarkInstance)
	{
		CGraphicMarkInstance::Delete(pTextTail->pMarkInstance);
		pTextTail->pMarkInstance = NULL;
	}
	if (pTextTail->pGuildNameTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pGuildNameTextInstance);
		pTextTail->pGuildNameTextInstance = NULL;
	}
	if (pTextTail->pTitleTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pTitleTextInstance);
		pTextTail->pTitleTextInstance = NULL;
	}
	if (pTextTail->pLevelTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pLevelTextInstance);
		pTextTail->pLevelTextInstance = NULL;
	}
#ifdef ENABLE_AFK_MODE_SYSTEM
	if (pTextTail->pAfkTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pAfkTextInstance);
		pTextTail->pAfkTextInstance = NULL;
	}
#endif
#if defined(WJ_COMBAT_ZONE)
	if (pTextTail->pCombatZoneRankInstance)
	{
		CGraphicImageInstance::Delete(pTextTail->pCombatZoneRankInstance);
		pTextTail->pCombatZoneRankInstance = NULL;
	}
#endif

	m_TextTailPool.Free(pTextTail);
}

int CPythonTextTail::Pick(int ixMouse, int iyMouse)
{
	for (TTextTailMap::iterator itor = m_ItemTextTailMap.begin(); itor != m_ItemTextTailMap.end(); ++itor)
	{
		TTextTail * pTextTail = itor->second;

		if (ixMouse >= pTextTail->x + pTextTail->xStart && ixMouse <= pTextTail->x + pTextTail->xEnd &&
			iyMouse >= pTextTail->y + pTextTail->yStart && iyMouse <= pTextTail->y + pTextTail->yEnd)
		{
			SelectItemName(itor->first);
			return (itor->first);
		}
	}

	return -1;
}

void CPythonTextTail::SelectItemName(DWORD dwVirtualID)
{
	TTextTailMap::iterator itor = m_ItemTextTailMap.find(dwVirtualID);

	if (m_ItemTextTailMap.end() == itor)
		return;

	TTextTail * pTextTail = itor->second;
	pTextTail->pTextInstance->SetColor(0.1f, 0.9f, 0.1f);
}

void CPythonTextTail::AttachTitle(DWORD dwVID, const char * c_szName, const D3DXCOLOR & c_rColor)
{
	if (!bPKTitleEnable)
		return;

	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail * pTextTail = itor->second;

	CGraphicTextInstance *& prTitle = pTextTail->pTitleTextInstance;
	if (!prTitle)
	{
		prTitle = CGraphicTextInstance::New();
		prTitle->SetTextPointer(ms_pFont);
		prTitle->SetOutline(true);
		prTitle->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_RIGHT);
		prTitle->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
	}

	prTitle->SetValue(c_szName);
	prTitle->SetColor(c_rColor.r, c_rColor.g, c_rColor.b);
	prTitle->Update();
}

void CPythonTextTail::DetachTitle(DWORD dwVID)
{
	if (!bPKTitleEnable)
		return;

	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail * pTextTail = itor->second;

	if (pTextTail->pTitleTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pTitleTextInstance);
		pTextTail->pTitleTextInstance = NULL;
	}
}

void CPythonTextTail::EnablePKTitle(BOOL bFlag)
{
	bPKTitleEnable = bFlag;
}

void CPythonTextTail::AttachLevel(DWORD dwVID, const char * c_szText, const D3DXCOLOR & c_rColor)
{
	if (!bPKTitleEnable)
		return;

	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail * pTextTail = itor->second;

	CGraphicTextInstance *& prLevel = pTextTail->pLevelTextInstance;
	if (!prLevel)
	{
		prLevel = CGraphicTextInstance::New();
		prLevel->SetTextPointer(ms_pFont);
		prLevel->SetOutline(true);

		prLevel->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_RIGHT);
		prLevel->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
	}

	prLevel->SetValue(c_szText);
	prLevel->SetColor(c_rColor.r, c_rColor.g, c_rColor.b);
	prLevel->Update();
}

void CPythonTextTail::DetachLevel(DWORD dwVID)
{
	if (!bPKTitleEnable)
		return;

	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail * pTextTail = itor->second;

	if (pTextTail->pLevelTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pLevelTextInstance);
		pTextTail->pLevelTextInstance = NULL;
	}
}

#ifdef ENABLE_CONQUEROR_LEVEL
void CPythonTextTail::AttachConquerorLevel(DWORD dwVID, const char* c_szText)
{
	if (!bPKTitleEnable)
		return;

	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail* pTextTail = itor->second;

	CGraphicTextInstance*& prLevel = pTextTail->pLevelTextInstance;
	if (!prLevel)
	{
		prLevel = CGraphicTextInstance::New();
		prLevel->SetTextPointer(ms_pFont);
		prLevel->SetOutline(true);

		prLevel->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_RIGHT);
		prLevel->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
	}

	prLevel->SetValue(c_szText);
	prLevel->SetColor(140.0f / 255.0f, 200.0f / 255.0f, 255.0f / 255.0f, 1.0f);
	prLevel->SetOutLineColor(140.0f / 255.0f * 0.5f, 200.0f / 255.0f * 0.5f, 255.0f / 255.0f * 0.5f, 1.0f);
	prLevel->Update();
}
#endif

void CPythonTextTail::Initialize()
{
	CGraphicText* pkDefaultFont = static_cast<CGraphicText*>(DefaultFont_GetResource());
	if (!pkDefaultFont)
	{
		TraceError("CPythonTextTail::Initialize - CANNOT_FIND_DEFAULT_FONT");
		return;
	}

	ms_pFont = pkDefaultFont;

	m_CharacterTextTailList.reserve(256);
	m_ItemTextTailList.reserve(256);
}

void CPythonTextTail::Destroy()
{
	m_TextTailPool.Clear();
}

void CPythonTextTail::Clear()
{
	m_CharacterTextTailMap.clear();
	m_CharacterTextTailList.clear();
	m_ItemTextTailMap.clear();
	m_ItemTextTailList.clear();
	m_ChatTailMap.clear();

	m_TextTailPool.Clear();
}

CPythonTextTail::CPythonTextTail()
{
	Clear();
}

CPythonTextTail::~CPythonTextTail()
{
	Destroy();
}

#ifdef ENABLE_AFK_MODE_SYSTEM
void CPythonTextTail::AttachAfk(DWORD dwVID, const char * c_szText, const D3DXCOLOR & c_rColor)
{
	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail * pTextTail = itor->second;

	CGraphicTextInstance *& prAfk = pTextTail->pAfkTextInstance;
	if (!prAfk)
	{
		prAfk = CGraphicTextInstance::New();
		prAfk->SetTextPointer(ms_pFont);
		prAfk->SetOutline(true);
		prAfk->SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_CENTER);
		prAfk->SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_BOTTOM);
	}

	std::string strAfkText = c_szText;

	if (GetDefaultCodePage() == CP_ARABIC)
	{
		strAfkText = "\r" + strAfkText + "\r";
	}

	prAfk->SetValue(strAfkText.c_str());
	prAfk->SetColor(c_rColor.r, c_rColor.g, c_rColor.b);
	prAfk->Update();
}

void CPythonTextTail::DetachAfk(DWORD dwVID)
{
	TTextTailMap::iterator itor = m_CharacterTextTailMap.find(dwVID);
	if (m_CharacterTextTailMap.end() == itor)
		return;

	TTextTail * pTextTail = itor->second;

	if (pTextTail->pAfkTextInstance)
	{
		CGraphicTextInstance::Delete(pTextTail->pAfkTextInstance);
		pTextTail->pAfkTextInstance = NULL;
	}
}
#endif