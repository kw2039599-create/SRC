// PythonBackground.cpp: implementation of the CPythonBackground class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../eterlib/CullingManager.h"
#include "../eterlib/Camera.h"
#include "../eterPack/EterPackManager.h"
#include "../gamelib/MapOutDoor.h"
#include "../gamelib/PropertyLoader.h"

#include "PythonBackground.h"
#include "PythonCharacterManager.h"
#include "PythonNetworkStream.h"
#include "PythonMiniMap.h"
#include "PythonSystem.h"

std::string g_strEffectName = "d:/ymir work/effect/etc/direction/direction_land.mse";
std::string g_strEffectShopSearch = "d:/ymir work/effect/etc/direction/direction_land2.mse";
#if defined(WJ_COMBAT_ZONE)
std::string g_strEffectTargetLeaving = "d:/ymir work/effect/etc/direction/direction_land_dragonroom.mse";
#endif
DWORD CPythonBackground::GetRenderShadowTime()
{
	return m_dwRenderShadowTime;
}

bool CPythonBackground::SetVisiblePart(int eMapOutDoorPart, bool isVisible)
{
	if (!m_pkMap)
		return false;

	CMapOutdoor& rkMap=GetMapOutdoorRef();
	rkMap.SetVisiblePart(eMapOutDoorPart, isVisible);
	return true;
}

void CPythonBackground::EnableTerrainOnlyForHeight()
{
	if (!m_pkMap)
		return;

	CMapOutdoor& rkMap=GetMapOutdoorRef();
	rkMap.EnableTerrainOnlyForHeight(TRUE);
}

bool CPythonBackground::SetSplatLimit(int iSplatNum)
{
	if (!m_pkMap)
		return false;

	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.SetSplatLimit(iSplatNum);
	return true;
}

void CPythonBackground::CreateCharacterShadowTexture()
{
	if (!m_pkMap)
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();
	if (Frustum::Instance().graphicsLevel == 0)
		rkMap.CreateCharacterShadowTextureOriginal();
	else
		rkMap.CreateCharacterShadowTexture();
}

void CPythonBackground::ReleaseCharacterShadowTexture()
{
	if (!m_pkMap)
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.ReleaseCharacterShadowTexture();
	rkMap.ReleaseCharacterShadowTextureOriginal();
}

void CPythonBackground::RefreshShadowLevel()
{
	SetShadowLevel(CPythonSystem::Instance().GetShadowLevel());
	SetGraphicsLevel(CPythonSystem::Instance().GetGraphicsLevel());
	if (CPythonSystem::Instance().GetGraphicsLevel() == 0)
	{
		SetTerrainShadow(0);
		SetTreeShadow(0);
		SetSpecularLevel(0);
		SetBloodLevel(0);
	}
	else
	{
		SetTerrainShadow(CPythonSystem::Instance().GetTerrainShadow());
		SetTreeShadow(CPythonSystem::Instance().GetTreeShadow());
		SetRainLevel(CPythonSystem::Instance().GetRainLevel());
		SetSpecularLevel(CPythonSystem::Instance().GetSpecularLevel());
		SetBloodLevel(CPythonSystem::Instance().GetBloodLevel());
	}

	SetGrassType(CPythonSystem::Instance().GetGrassType());
	//CPythonSystem::Instance().SetSharpnessPower(CPythonSystem::Instance().GetSharpnessPower());
}

void CPythonBackground::RefreshGraphicsLevel()
{
	SetGraphicsLevel(CPythonSystem::Instance().GetGraphicsLevel());
}

// JTX SETTINGS
// JTX SETTINGS
bool CPythonBackground::SetGraphicsLevel(int eLevel)
{
	if (!m_pkMap)
		return false;

#if defined(ENABLE_OPENGL) && !defined(__ANDROID__)
	eLevel = GRAPHICS_OFF;
#endif

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	m_eGraphicsLevel = eLevel;

	switch (m_eGraphicsLevel)
	{
	case GRAPHICS_OFF:
		Frustum::Instance().graphicsLevel = 0;
		Frustum::Instance().dynamicLight = 0;
		Frustum::Instance().shadowType = 3;
		Frustum::Instance().graphicsWaterLevel = 0;
		Frustum::Instance().shadowRender = 0;
		Frustum::Instance().terrainDepthLevel = 0;
		Frustum::Instance().increaseShadowRangeFrustum = 1.0f;
		rkMap.recreate = true;
		SetShadowQualityLevel(0);
		SetTreeShadow(0);
		SetSpecularLevel(0);
		SetBloodLevel(0);
		SetShadowLevel(CPythonSystem::Instance().GetShadowLevel());
		break;

	case GRAPHICS_MEDIUM:
		Frustum::Instance().graphicsLevel = 2;
		Frustum::Instance().dynamicLight = 3;
		Frustum::Instance().shadowType = 2;
		Frustum::Instance().graphicsWaterLevel = 0;
		Frustum::Instance().shadowRender = 1;
		Frustum::Instance().increaseShadowRangeFrustum = 1.0f;
		Frustum::Instance().terrainDepthLevel = 1;
		rkMap.recreate = true;
		SetShadowQualityLevel(1);
		SetTreeShadow(1);
		SetSpecularLevel(1);
		SetBloodLevel(1);
		break;

	case GRAPHICS_HIGH:
	case GRAPHICS_ULTRA:
		// Mode 3: Old Graphics Quality (4096 shadows, shadowRender 3) + Water Reflections ONLY
		Frustum::Instance().graphicsLevel = 3;
		Frustum::Instance().dynamicLight = 3;
		Frustum::Instance().shadowType = 2;
		Frustum::Instance().graphicsWaterLevel = 2;
		Frustum::Instance().shadowRender = 3;
		Frustum::Instance().increaseShadowRangeFrustum = 10.0f;
		Frustum::Instance().terrainDepthLevel = 0; // Old ground (no new depth/normal maps)
		rkMap.recreate = true;
		SetShadowQualityLevel(3);
		SetTreeShadow(3);
		SetSpecularLevel(1);
		SetBloodLevel(1);
		break;

	case GRAPHICS_HIGH_ULTRA:
		// Mode 4: New High-Quality Ground (terrainDepthLevel = 1), Puddles, Grass & Expanded Shadows
		Frustum::Instance().graphicsLevel = 4;
		Frustum::Instance().dynamicLight = 3;
		Frustum::Instance().shadowType = 2;
		Frustum::Instance().graphicsWaterLevel = 2;
		Frustum::Instance().shadowRender = 3;
		Frustum::Instance().increaseShadowRangeFrustum = 10.0f;
		Frustum::Instance().terrainDepthLevel = 1; // New high-quality ground maps
		rkMap.recreate = true;
		SetShadowQualityLevel(3);
		SetTreeShadow(3);
		SetSpecularLevel(1);
		SetBloodLevel(1);
		break;
	}

	return true;
}

bool CPythonBackground::SetShadowQualityLevel(int eLevel)
{
	if (!m_pkMap)
		return false;

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	m_eShadowQualityLevel = eLevel;

	if (Frustum::Instance().graphicsLevel == 0)
	{
		rkMap.CreateCharacterShadowTextureOriginal();
		return true;
	}

	switch (eLevel)
	{
	case SHADOW_QUALITY_BAD:
		rkMap.SetShadowTextureSize(1024);
		rkMap.CreateCharacterShadowTexture();
		break;
	case SHADOW_QUALITY_AVERAGE:
		rkMap.SetShadowTextureSize(1280);
		rkMap.CreateCharacterShadowTexture();
		break;
	case SHADOW_QUALITY_GOOD:
		rkMap.SetShadowTextureSize(2048);
		rkMap.CreateCharacterShadowTexture();
		break;
	case SHADOW_QUALITY_ULTRA:
		rkMap.SetShadowTextureSize(4096);
		rkMap.CreateCharacterShadowTexture();
		break;
	}

	return true;
}

bool CPythonBackground::SetTerrainShadow(int eLevel)
{
	if (!m_pkMap)
		return false;

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	m_eTerrainShadow = eLevel;

	switch (eLevel)
	{
	case TERRAIN_SHADOW_off:
		break;
	case TERRAIN_SHADOW_on:
		break;
	}

	return true;
}

//SPECULAR JTX
bool CPythonBackground::SetSpecularLevel(int eLevel)
{
	m_eSpecularLevel = eLevel;

	switch (m_eSpecularLevel)
	{
	case SPECULAR_off:
		Frustum::Instance().realtimeSpecular = 0;
		break;

	case SPECULAR_on:
		Frustum::Instance().realtimeSpecular = 1;
		break;
	}

	return true;
}

//BLOOD JTX
bool CPythonBackground::SetBloodLevel(int eLevel)
{
	m_eBloodLevel = eLevel;

	switch (m_eBloodLevel)
	{
	case BLOOD_off:
		Frustum::Instance().bloodLevel = 0;
		break;

	case BLOOD_on:
		Frustum::Instance().bloodLevel = 1;
		break;
	}

	return true;
}

bool CPythonBackground::SetTreeShadow(int eLevel)
{
	if (!m_pkMap)
		return false;

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	m_eTreeShadow = eLevel;

	switch (eLevel)
	{
	case TREE_SHADOW_on:
		Frustum::Instance().treeShadow = 0;
		break;

	case TREE_SHADOW_off:
		Frustum::Instance().treeShadow = 1;
		break;

	case TREE_SHADOW_high:
		Frustum::Instance().treeShadow = 2;
		break;

	case TREE_SHADOW_all:
		Frustum::Instance().treeShadow = 3;
		break;
	}

	return true;
}

bool CPythonBackground::SetGrassType(int eLevel)
{
	if (!m_pkMap)
		return false;

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	m_eGrassType = eLevel;

	switch (eLevel)
	{
	case GRASS_0:
		Frustum::Instance().grassType = 0;
		break;

	case GRASS_1:
		Frustum::Instance().grassType = 1;
		break;

	case GRASS_2:
		Frustum::Instance().grassType = 2;
		break;

	case GRASS_3:
		Frustum::Instance().grassType = 3;
		break;
	}

	return true;
}

//RAIN
//RAIN
bool CPythonBackground::SetRainLevel(int eLevel)
{
	if (!m_pkMap)
		return false;

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	m_eRainLevel = eLevel;
	CPythonSystem::Instance().SetRainLevel(eLevel);
	CPythonSystem::Instance().SetRainModeOption(eLevel > 0);

	switch (m_eRainLevel)
	{
	case RAIN_0:
		DisableSnowEnvironment();
		Frustum::Instance().selectRainFrustun = 0;
		Frustum::Instance().windSpeed = 0.5f;
		CSpeedTreeForestDirectX8::Instance().SetWindStrength(0.5f);
		break;

	case RAIN_1:
		EnableSnowEnvironment();
		Frustum::Instance().windSpeed = 0.6f;
		CSpeedTreeForestDirectX8::Instance().SetWindStrength(0.6f);
		Frustum::Instance().selectRainFrustun = 8;
		break;

	case RAIN_2:
		EnableSnowEnvironment();
		Frustum::Instance().windSpeed = 0.8f;
		CSpeedTreeForestDirectX8::Instance().SetWindStrength(0.8f);
		Frustum::Instance().selectRainFrustun = 16;
		break;

	case RAIN_3:
		EnableSnowEnvironment();
		Frustum::Instance().windSpeed = 0.95f;
		CSpeedTreeForestDirectX8::Instance().SetWindStrength(0.95f);
		Frustum::Instance().selectRainFrustun = 25;
		break;

	case RAIN_4:
		break;
	}

	return true;
}

bool CPythonBackground::SetShadowLevel(int eLevel)
{
	if (!m_pkMap)
		return false;

	if (m_eShadowLevel == eLevel)
		return true;

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	m_eShadowLevel = eLevel;

#if defined(ENABLE_OPENGL) && !defined(__ANDROID__)
	rkMap.SetDrawShadow(true);
	rkMap.SetDrawCharacterShadow(true);
	rkMap.SetShadowTextureSize(512);
	return true;
#endif

	switch (m_eShadowLevel)
	{
		case SHADOW_NONE:
			rkMap.SetDrawShadow(false);
			rkMap.SetShadowTextureSize(2048);
			break;

		case SHADOW_GROUND:
			rkMap.SetDrawShadow(true);
			rkMap.SetDrawCharacterShadow(false);
			rkMap.SetShadowTextureSize(2048);
			break;

		case SHADOW_GROUND_AND_SOLO:
			rkMap.SetDrawShadow(true);
			rkMap.SetDrawCharacterShadow(true);
			rkMap.SetShadowTextureSize(2048);
			break;

		case SHADOW_ALL:
			rkMap.SetDrawShadow(true);
			rkMap.SetDrawCharacterShadow(true);
			rkMap.SetShadowTextureSize(2048);
			break;

		case SHADOW_ALL_HIGH:
			rkMap.SetDrawShadow(true);
			rkMap.SetDrawCharacterShadow(true);
			rkMap.SetShadowTextureSize(2048);
			break;

		case SHADOW_ALL_MAX:
			rkMap.SetDrawShadow(true);
			rkMap.SetDrawCharacterShadow(true);
			rkMap.SetShadowTextureSize(2048);
			break;
	}

	return true;
}

void CPythonBackground::SelectViewDistanceNum(int eNum)
{
	if (!m_pkMap)
		return;

	CMapOutdoor& rkMap=GetMapOutdoorRef();

	if (!mc_pcurEnvironmentData)
	{
		TraceError("CPythonBackground::SelectViewDistanceNum(int eNum=%d) mc_pcurEnvironmentData is NULL", eNum);
		return;
	}

	m_eViewDistanceNum = eNum;

	TEnvironmentData * env = ((TEnvironmentData *) mc_pcurEnvironmentData);

	if (env->bReserve)
	{
		env->m_fFogNearDistance = m_ViewDistanceSet[m_eViewDistanceNum].m_fFogStart;
		env->m_fFogFarDistance = m_ViewDistanceSet[m_eViewDistanceNum].m_fFogEnd;
		env->v3SkyBoxScale = m_ViewDistanceSet[m_eViewDistanceNum].m_v3SkyBoxScale;
		rkMap.SetEnvironmentSkyBox();
	}
}

void CPythonBackground::SetViewDistanceSet(int eNum, float fFarClip)
{
	if (!m_pkMap)
		return;

	m_ViewDistanceSet[eNum].m_fFogStart		= fFarClip * 0.5f;//0.3333333f;
	m_ViewDistanceSet[eNum].m_fFogEnd		= fFarClip * 0.7f;//0.6666667f;

	float fSkyBoxScale						= fFarClip * 0.6f;//0.5773502f;
	m_ViewDistanceSet[eNum].m_v3SkyBoxScale	= D3DXVECTOR3(fSkyBoxScale, fSkyBoxScale, fSkyBoxScale);
	m_ViewDistanceSet[eNum].m_fFarClip		= fFarClip;

	if (eNum == m_eViewDistanceNum)
		SelectViewDistanceNum(eNum);
}

float CPythonBackground::GetFarClip()
{
	if (!m_pkMap)
		return 50000.0f;

	if (m_ViewDistanceSet[m_eViewDistanceNum].m_fFarClip==0.0f)
	{
		TraceError("CPythonBackground::GetFarClip m_eViewDistanceNum=%d", m_eViewDistanceNum);
		m_ViewDistanceSet[m_eViewDistanceNum].m_fFarClip=25600.0f;
	}

	return m_ViewDistanceSet[m_eViewDistanceNum].m_fFarClip;
}

void CPythonBackground::GetDistanceSetInfo(int * peNum, float * pfStart, float * pfEnd, float * pfFarClip)
{
	if (!m_pkMap)
	{
		*peNum = 4;
		*pfStart= 10000.0f;
		*pfEnd= 15000.0f;
		*pfFarClip = 50000.0f;
		return;
	}
	*peNum = m_eViewDistanceNum;
	*pfStart = m_ViewDistanceSet[m_eViewDistanceNum].m_fFogStart;
	*pfEnd= m_ViewDistanceSet[m_eViewDistanceNum].m_fFogEnd;
	*pfFarClip = m_ViewDistanceSet[m_eViewDistanceNum].m_fFarClip;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPythonBackground::CPythonBackground()
{
	m_dwRenderShadowTime=0;
	m_eViewDistanceNum=0;
	m_eViewDistanceNum=0;
	m_eViewDistanceNum=0;
	m_eShadowLevel=SHADOW_NONE;
	m_dwBaseX=0;
	m_dwBaseY=0;
	m_strMapName="";
	m_iDayMode = DAY_MODE_LIGHT;
	m_iXMasTreeGrade = 0;
	m_bVisibleGuildArea = FALSE;
#if defined(__BL_ENVIRONMENT_EFFECT_OPTION__)
	m_bXMasShowEvent = false;
	m_bNightMode = true;
	m_bSnowMode = true;
	m_bSnowTextureMode = true;
#endif
	SetViewDistanceSet(4, 25600.0f);
	SetViewDistanceSet(3, 25600.0f);
	SetViewDistanceSet(2, 25600.0f);
	SetViewDistanceSet(1, 25600.0f);
	SetViewDistanceSet(0, 25600.0f);
	Initialize();
}

CPythonBackground::~CPythonBackground()
{
	Tracen("CPythonBackground Clear");
}

#define ENABLE_ATLASINFO_FROM_ROOT
void CPythonBackground::Initialize()
{
#ifdef ENABLE_ATLASINFO_FROM_ROOT
	std::string stAtlasInfoFileName("AtlasInfo.txt");
#else
	std::string stAtlasInfoFileName (LocaleService_GetLocalePath());
	stAtlasInfoFileName += "/AtlasInfo.txt";
#endif
	SetAtlasInfoFileName(stAtlasInfoFileName.c_str());
	CMapManager::Initialize();
}

void CPythonBackground::__CreateProperty()
{
	if (CEterPackManager::SEARCH_FILE_FIRST == CEterPackManager::Instance().GetSearchMode() &&
		_access("property", 0) == 0)
	{
		m_PropertyManager.Initialize(NULL);

		CPropertyLoader PropertyLoader;
		PropertyLoader.SetPropertyManager(&m_PropertyManager);
		PropertyLoader.Create("*.*", "Property");
	}
	else
	{
		m_PropertyManager.Initialize("pack/data020");
	}
}

//////////////////////////////////////////////////////////////////////
// Normal Functions
//////////////////////////////////////////////////////////////////////

bool CPythonBackground::GetPickingPoint(D3DXVECTOR3 * v3IntersectPt)
{
	CMapOutdoor& rkMap=GetMapOutdoorRef();
	return rkMap.GetPickingPoint(v3IntersectPt);
}

bool CPythonBackground::GetPickingPointWithRay(const CRay & rRay, D3DXVECTOR3 * v3IntersectPt)
{
	CMapOutdoor& rkMap=GetMapOutdoorRef();
	return rkMap.GetPickingPointWithRay(rRay, v3IntersectPt);
}

bool CPythonBackground::GetPickingPointWithRayOnlyTerrain(const CRay & rRay, D3DXVECTOR3 * v3IntersectPt)
{
	CMapOutdoor& rkMap=GetMapOutdoorRef();
	return rkMap.GetPickingPointWithRayOnlyTerrain(rRay, v3IntersectPt);
}

BOOL CPythonBackground::GetLightDirection(D3DXVECTOR3 & rv3LightDirection)
{
	if (!mc_pcurEnvironmentData)
		return FALSE;

	rv3LightDirection.x = mc_pcurEnvironmentData->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction.x;
	rv3LightDirection.y = mc_pcurEnvironmentData->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction.y;
	rv3LightDirection.z = mc_pcurEnvironmentData->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction.z;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
void CPythonBackground::Destroy()
{
	CMapManager::Destroy();
	m_SnowEnvironment.Destroy();
	m_bVisibleGuildArea = FALSE;
}

void CPythonBackground::Create()
{
	static int s_isCreateProperty=false;

	if (!s_isCreateProperty)
	{
		s_isCreateProperty=true;
		__CreateProperty();
	}

	CMapManager::Create();

	m_SnowEnvironment.Create();
}

struct FGetPortalID
{
	float m_fRequestX, m_fRequestY;
	std::set<int> m_kSet_iPortalID;
	FGetPortalID(float fRequestX, float fRequestY)
	{
		m_fRequestX=fRequestX;
		m_fRequestY=fRequestY;
	}
	void operator () (CGraphicObjectInstance * pObject)
	{
		for (int i = 0; i < PORTAL_ID_MAX_NUM; ++i)
		{
			int iID = pObject->GetPortal(i);
			if (0 == iID)
				break;

			m_kSet_iPortalID.insert(iID);
		}
	}
};

void CPythonBackground::Update(float fCenterX, float fCenterY, float fCenterZ)
{
	if (!IsMapReady())
		return;
// --- [FIX] GLOBAL PERMANENT SHADE OVERRIDE ---
	if (mc_pcurEnvironmentData)
	{
		TEnvironmentData * env = ((TEnvironmentData *) mc_pcurEnvironmentData);
#if defined(__BL_FOG_FIX__)
		bool bFogMode = CPythonSystem::Instance().GetFogMode();
		if (!m_strMapName.compare("metin2_map_defensewave"))
			bFogMode = true;

		if (!bFogMode)
		{
			env->bFogEnable = FALSE;
			env->m_fFogNearDistance = 500000.0f;
			env->m_fFogFarDistance  = 1000000.0f;
		}
		else
#endif
		{
			env->bFogEnable = TRUE;
			env->m_fFogNearDistance = 2000.0f;
			env->m_fFogFarDistance  = 45000.0f;
		}

		CSpeedTreeForestDirectX8::Instance().SetFog(
			env->m_fFogNearDistance,
			env->m_fFogFarDistance
		);
	}
	// ---------------------------------------------
#ifdef __PERFORMANCE_CHECKER__
	DWORD t1=ELTimer_GetMSec();
#endif
	UpdateMap(fCenterX, fCenterY, fCenterZ);
#ifdef __PERFORMANCE_CHECKER__
	DWORD t2=ELTimer_GetMSec();
#endif
	UpdateAroundAmbience(fCenterX, fCenterY, fCenterZ);
#ifdef __PERFORMANCE_CHECKER__
	DWORD t3=ELTimer_GetMSec();
#endif
	m_SnowEnvironment.Update(D3DXVECTOR3(fCenterX, -fCenterY, fCenterZ));
#if defined(WJ_COMBAT_ZONE)
	{
		std::map<DWORD, DWORD>::iterator itor = m_kMapCombatZone_dwTargetID_dwChrID.begin();
		for (; itor != m_kMapCombatZone_dwTargetID_dwChrID.end(); ++itor)
		{
			DWORD dwTargetID = itor->first;
			DWORD dwChrID = itor->second;

			CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwChrID);

			if (!pInstance)
				continue;

			TPixelPosition kPixelPosition;
			pInstance->NEW_GetPixelPosition(&kPixelPosition);

			CreateSpecialEffect(dwTargetID,
								+kPixelPosition.x,
								-kPixelPosition.y,
								+kPixelPosition.z,
								g_strEffectTargetLeaving.c_str());
		}
	}
#endif

#ifdef __PERFORMANCE_CHECKER__
	{
		static FILE* fp=fopen("perf_bg_update.txt", "w");
		if (t3-t1>5)
		{
			fprintf(fp, "BG.Total %d (Time %f)\n", t3-t1, ELTimer_GetMSec()/1000.0f);
			fprintf(fp, "BG.UpdateMap %d\n", t2-t1);
			fprintf(fp, "BG.UpdateAmb %d\n", t3-t2);
			fflush(fp);
		}
	}
#endif

	// Portal Process
	CMapOutdoor& rkMap=GetMapOutdoorRef();
	if (rkMap.IsEnablePortal())
	{
		CCullingManager & rkCullingMgr = CCullingManager::Instance();
		FGetPortalID kGetPortalID(fCenterX, -fCenterY);

		Vector3d aVector3d;
		aVector3d.Set(fCenterX, -fCenterY, fCenterZ);

		Vector3d toTop;
		toTop.Set(0, 0, 25000.0f);

		rkCullingMgr.ForInRay(aVector3d, toTop, &kGetPortalID);

		std::set<int>::iterator itor = kGetPortalID.m_kSet_iPortalID.begin();
		if (!__IsSame(kGetPortalID.m_kSet_iPortalID, m_kSet_iShowingPortalID))
		{
			ClearPortal();
			std::set<int>::iterator itor=kGetPortalID.m_kSet_iPortalID.begin();
			for (; itor!=kGetPortalID.m_kSet_iPortalID.end(); ++itor)
			{
				AddShowingPortalID(*itor);
			}
			RefreshPortal();

			m_kSet_iShowingPortalID = kGetPortalID.m_kSet_iPortalID;
		}
	}

	std::map<DWORD, DWORD>::iterator itor2 = m_kMap_dwTargetID_dwChrID2.begin();
		for (; itor2 != m_kMap_dwTargetID_dwChrID2.end(); ++itor2)
		{
			DWORD dwTargetID2 = itor2->first;
			DWORD dwChrID2 = itor2->second;
			CInstanceBase * pInstance2 = CPythonCharacterManager::Instance().GetInstancePtr(dwChrID2);

			if (!pInstance2)
				continue;

			TPixelPosition kPixelPosition2;
			pInstance2->NEW_GetPixelPosition(&kPixelPosition2);

			CreateSpecialEffect(dwTargetID2,
				+kPixelPosition2.x,
				-kPixelPosition2.y,
				+kPixelPosition2.z,
				g_strEffectShopSearch.c_str());
		}

	// Target Effect Process
	{
		std::map<DWORD, DWORD>::iterator itor = m_kMap_dwTargetID_dwChrID.begin();
		for (; itor != m_kMap_dwTargetID_dwChrID.end(); ++itor)
		{
			DWORD dwTargetID = itor->first;
			DWORD dwChrID = itor->second;

			CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwChrID);

			if (!pInstance)
				continue;

			TPixelPosition kPixelPosition;
			pInstance->NEW_GetPixelPosition(&kPixelPosition);

			CreateSpecialEffect(dwTargetID,
								+kPixelPosition.x,
								-kPixelPosition.y,
								+kPixelPosition.z,
								g_strEffectName.c_str());
		}
	}

	// Reserve Target Effect
	{
		std::map<DWORD, SReserveTargetEffect>::iterator itor = m_kMap_dwID_kReserveTargetEffect.begin();
		for (; itor != m_kMap_dwID_kReserveTargetEffect.end();)
		{
			DWORD dwID = itor->first;
			SReserveTargetEffect & rReserveTargetEffect = itor->second;

			float ilx = float(rReserveTargetEffect.ilx);
			float ily = float(rReserveTargetEffect.ily);

			float fHeight = rkMap.GetHeight(ilx, ily);
			if (0.0f == fHeight)
			{
				++itor;
				continue;
			}

			CreateSpecialEffect(dwID, ilx, ily, fHeight, g_strEffectName.c_str());

			itor = m_kMap_dwID_kReserveTargetEffect.erase(itor);
		}
	}
}

bool CPythonBackground::__IsSame(std::set<int> & rleft, std::set<int> & rright)
{
	std::set<int>::iterator itor_l;
	std::set<int>::iterator itor_r;

	for (itor_l=rleft.begin(); itor_l!=rleft.end(); ++itor_l)
	{
		if (rright.end() == rright.find(*itor_l))
			return false;
	}

	for (itor_r=rright.begin(); itor_r!=rright.end(); ++itor_r)
	{
		if (rleft.end() == rleft.find(*itor_r))
			return false;
	}

	return true;
}

void CPythonBackground::Render()
{
	if (!IsMapReady())
		return;

	m_SnowEnvironment.Deform();

	CMapOutdoor& rkMap=GetMapOutdoorRef();
	rkMap.Render();
	if (m_bVisibleGuildArea)
		rkMap.RenderMarkedArea();
}

void CPythonBackground::RenderSnow()
{
	m_SnowEnvironment.Render();
}

void CPythonBackground::RenderPCBlocker()
{
	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap=GetMapOutdoorRef();
	rkMap.RenderPCBlocker();
}

void CPythonBackground::RenderCollision()
{
	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap=GetMapOutdoorRef();
	rkMap.RenderCollision();
}

void CPythonBackground::RenderShadow()
{
	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	if (Frustum::Instance().graphicsLevel > 0)
	{
		if (Frustum::Instance().treeShadow >= 1)
		{
			rkMap.RenderArea2();
			rkMap.__RenderTerrain_RenderHardwareTransformPatch2();
		}
	}
}

void CPythonBackground::RenderCharacterShadowToTexture()
{
	extern bool GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW;
	if (GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW)
		return;

	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	rkMap.SkyUtil();
#ifndef __ANDROID__
	CSpeedTreeForestDirectX8::Instance().Render3(Forest_RenderAll);
#endif
}

void CPythonBackground::RenderCharacterShadowToTexture2()
{
	extern bool GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW;
	if (GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW)
		return;

	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	if (Frustum::Instance().shadowType != 3)
	{
		if (Frustum::Instance().isIndoor == true)
		{
			bool canRender = rkMap.BeginRenderCharacterShadowToTexture2();
			if (canRender)
			{
				rkMap.RenderDungeon();
				rkMap.EndRenderCharacterShadowToTexture2();
			}
		}
	}
}

void CPythonBackground::RenderCharacterShadowToTextureOriginal()
{
	extern bool GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW;
	if (GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW)
		return;

	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	bool canRender = rkMap.BeginRenderCharacterShadowToTextureOriginal();
	if (canRender)
	{
		D3DXMATRIX matWorldIdent;
		D3DXMatrixIdentity(&matWorldIdent);
		STATEMANAGER.SetTransform(D3DTS_WORLD, &matWorldIdent);

		CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
		rkChrMgr.RenderShadowAllInstances();
		rkMap.EndRenderCharacterShadowToTextureOriginal();
	}
}

void CPythonBackground::GetUnderwaterCamera()
{
	if (!m_pkMap)
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	if (!rkMap.IsWaterVisible())
		return;

	CCamera* camera = CCameraManager::Instance().GetCurrentCamera();
	D3DXVECTOR3 cameraEye = camera->GetEye();

	bool underwaterSound = false;
	bool surfaceSound = false;

	if ((cameraEye.z < Frustum::Instance().waterLevel - 5.0f) && Frustum::Instance().checkUnderwaterPosition > 0.5f)
	{
		Frustum::Instance().isUnderwater = 1.0f;
		underwaterSound = true;
		surfaceSound = false;
	}
	else
	{
		Frustum::Instance().isUnderwater = 0.0f;
		underwaterSound = false;
		surfaceSound = true;
	}

	static bool isFirstTime = true;

	if (surfaceSound && !Frustum::Instance().isPlayingSurfaceSound)
	{
		if (!isFirstTime)
		{
			CSoundManager::Instance().StopAllSound3D();
			CSoundManager::Instance().PlayMusic3D(2, cameraEye.x, cameraEye.y, cameraEye.z, "data/sound/waterOut.mp3");
			CSoundManager::Instance().PlayMusic3D(2, cameraEye.x, cameraEye.y, cameraEye.z, "data/sound/waterOut.mp3");
			Frustum::Instance().underwaterCameraTransition = 5.0f;
		}

		Frustum::Instance().isPlayingSurfaceSound = true;
		Frustum::Instance().isPlayingUnderwaterSound = false;
		isFirstTime = false;
	}

	if (underwaterSound && !Frustum::Instance().isPlayingUnderwaterSound)
	{
		CSoundManager::Instance().PlayMusic3D(2, cameraEye.x, cameraEye.y, cameraEye.z, "data/sound/underwater.mp3");
		CSoundManager::Instance().PlayMusic3D(2, cameraEye.x, cameraEye.y, cameraEye.z, "data/sound/underwater.mp3");
		CSoundManager::Instance().PlayMusic3D(2, cameraEye.x, cameraEye.y, cameraEye.z, "data/sound/waterSubmerge.mp3");
		CSoundManager::Instance().PlayMusic3D(2, cameraEye.x, cameraEye.y, cameraEye.z, "data/sound/waterSubmerge.mp3");
		Frustum::Instance().isPlayingUnderwaterSound = true;
		Frustum::Instance().isPlayingSurfaceSound = false;
	}
}

void CPythonBackground::RenderWaterReflection()
{
	if (!m_pkMap)
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	if (!rkMap.IsWaterVisible() || !Frustum::Instance().m_lpWaterReflectionTexture)
		return;

	// Use existing MainInstance instead of constructing/destroying a full CInstanceBase on the stack every frame
	if (CInstanceBase* pkMainInst = CPythonCharacterManager::Instance().GetMainInstancePtr())
		pkMainInst->GetUnderwaterCameraPosition();

	rkMap.SetWaterReflectionRenderTarget();
	CCamera* camera = CCameraManager::Instance().GetCurrentCamera();
	D3DXVECTOR3 cameraEye = camera->GetEye();
	D3DXVECTOR3 cameraTarget = camera->GetTarget();
	D3DXVECTOR3 cameraUp = camera->GetUp();

	static float closestWaterHeight = 0.0f;
	static D3DXVECTOR3 s_lastWaterCheckPos(FLT_MAX, FLT_MAX, FLT_MAX);

	const float playerX = Frustum::Instance().characterPosition.x;
	const float playerY = Frustum::Instance().characterPosition.y;
	const float playerZ = Frustum::Instance().characterPosition.z;

	const float moveDx = playerX - s_lastWaterCheckPos.x;
	const float moveDy = playerY - s_lastWaterCheckPos.y;

	// Only rescan waterPoints when the player moves > 50 units, and use squared distance (no std::sqrt)
	if ((moveDx * moveDx + moveDy * moveDy) > (50.0f * 50.0f))
	{
		s_lastWaterCheckPos = D3DXVECTOR3(playerX, playerY, playerZ);
		float minDistanceSq = 1e20f;
		const auto& waterPoints = Frustum::Instance().waterPoints;

		for (const auto& point : waterPoints)
		{
			float waterX, waterY, waterZ;
			std::tie(waterX, waterY, waterZ) = point;

			const float dx = waterX - playerX;
			const float dy = waterY - playerY;
			const float dz = waterZ - playerZ;
			const float distSq = dx * dx + dy * dy + dz * dz;
			if (distSq < minDistanceSq)
			{
				minDistanceSq = distSq;
				closestWaterHeight = waterZ;
			}
		}
	}

	Frustum::Instance().waterLevel = closestWaterHeight;

	D3DXVECTOR3 reflectionEye(cameraEye.x, cameraEye.y, Frustum::Instance().waterLevel * 2.0f - cameraEye.z);
	D3DXVECTOR3 reflectionTarget(cameraTarget.x, cameraTarget.y, Frustum::Instance().waterLevel * 2.0f - cameraTarget.z);
	D3DXVECTOR3 reflectionUp(cameraUp.x, cameraUp.y, -cameraUp.z);

	camera->SetViewParams(reflectionEye, reflectionTarget, reflectionUp);

	D3DXMATRIX matLightProj;
	STATEMANAGER.SaveTransform(D3DTS_VIEW, &(camera->GetViewMatrix()));
	STATEMANAGER.EnableClipPlane(0, 0.0, 0.0, 1.0, -Frustum::Instance().waterLevel);
	STATEMANAGER.GetTransform(D3DTS_PROJECTION, &matLightProj);

	CSpeedTreeForestDirectX8::Instance().UpdateCompundMatrix2(cameraEye, camera->GetViewMatrix(), matLightProj);

	{
		RenderSky();
		RenderCloud();
		CSpeedTreeForestDirectX8::Instance().RenderReflection();
		rkMap.RenderArea();
		rkMap.RenderTerrainReflect();

		SetCharacterDirLight();
		CPythonCharacterManager::Instance().Render();
	}

	{
		STATEMANAGER.PopRenderTarget();
		camera->SetViewParams(cameraEye, cameraTarget, cameraUp);
		STATEMANAGER.RestoreTransform(D3DTS_VIEW);
		STATEMANAGER.DisableClipPlane(0);
		CSpeedTreeForestDirectX8::Instance().UpdateCompundMatrix(cameraEye, camera->GetViewMatrix(), matLightProj);
		SetBackgroundDirLight();
	}
}

void CPythonBackground::RenderGrass()
{
	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.RenderGrass();
}

void CPythonBackground::RenderPuddleReflection()
{
	if (!m_pkMap || !Frustum::Instance().m_lpPuddleReflectionTexture)
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	rkMap.SetPuddleReflectionRenderTarget();
	CCamera* camera = CCameraManager::Instance().GetCurrentCamera();
	D3DXVECTOR3 cameraEye = camera->GetEye();
	D3DXVECTOR3 cameraTarget = camera->GetTarget();
	D3DXVECTOR3 cameraUp = camera->GetUp();

	Frustum::Instance().puddleLevel = Frustum::Instance().characterPosition.z;

	D3DXVECTOR3 reflectionEye(cameraEye.x, cameraEye.y, Frustum::Instance().puddleLevel * 2.0f - cameraEye.z);
	D3DXVECTOR3 reflectionTarget(cameraTarget.x, cameraTarget.y, Frustum::Instance().puddleLevel * 2.0f - cameraTarget.z);
	D3DXVECTOR3 reflectionUp(cameraUp.x, cameraUp.y, -cameraUp.z);

	camera->SetViewParams(reflectionEye, reflectionTarget, reflectionUp);

	D3DXMATRIX matLightProj;
	STATEMANAGER.SaveTransform(D3DTS_VIEW, &(camera->GetViewMatrix()));
	STATEMANAGER.EnableClipPlane(0, 0.0, 0.0, 1.0, -Frustum::Instance().puddleLevel);
	STATEMANAGER.GetTransform(D3DTS_PROJECTION, &matLightProj);

	CSpeedTreeForestDirectX8::Instance().UpdateCompundMatrix2(cameraEye, camera->GetViewMatrix(), matLightProj);

	{
		RenderSky();
		RenderCloud();
		CSpeedTreeForestDirectX8::Instance().RenderReflection();
		rkMap.RenderArea();

		SetCharacterDirLight();
		CPythonCharacterManager::Instance().Render();
	}

	{
		STATEMANAGER.PopRenderTarget();
		camera->SetViewParams(cameraEye, cameraTarget, cameraUp);
		STATEMANAGER.RestoreTransform(D3DTS_VIEW);
		STATEMANAGER.DisableClipPlane(0);
		CSpeedTreeForestDirectX8::Instance().UpdateCompundMatrix(cameraEye, camera->GetViewMatrix(), matLightProj);
		SetBackgroundDirLight();
	}
}

void CPythonBackground::RenderCharacterShadowToTexture3()
{
	extern bool GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW;
	if (GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW)
		return;

	if (!IsMapReady())
		return;

	if (Frustum::Instance().isIndoor)
		return;

#ifdef __ANDROID__
	static int s_iAndroidCascade3Frame = 0;
	if ((++s_iAndroidCascade3Frame & 1) != 0)
		return;
#endif

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	if (Frustum::Instance().shadowType != 3)
	{
		bool canRender = rkMap.BeginRenderCharacterShadowToTexture3();
		if (canRender)
		{
			D3DXMATRIX matWorldIdent;
			D3DXMatrixIdentity(&matWorldIdent);
			STATEMANAGER.SetTransform(D3DTS_WORLD, &matWorldIdent);

			CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();

			if (Frustum::Instance().shadowType != 2)
			{
				if (Frustum::Instance().treeShadow == 0)
					rkChrMgr.RenderShadowMainInstance();
				else if (Frustum::Instance().treeShadow >= 1)
					rkChrMgr.RenderShadowAllInstances();
			}

			if (Frustum::Instance().treeShadow >= 1)
			{
				rkMap.RenderArea2();
#ifndef __ANDROID__
				rkMap.__RenderTerrain_RenderHardwareTransformPatch2();
#endif
			}

#ifdef __ANDROID__
			if (Frustum::Instance().treeShadow >= 1)
#else
			if (Frustum::Instance().treeShadow == 3)
#endif
			{
				ms_lpd3dDevice->SetPixelShader(rkMap.m_lpShadowMapPixelShaderDepthTree);
				CSpeedTreeForestDirectX8::Instance().Render2(Forest_RenderAll);
			}

			STATEMANAGER.SetTransform(D3DTS_WORLD, &matWorldIdent);
			rkMap.EndRenderCharacterShadowToTexture3();
		}
	}
}

void CPythonBackground::RenderCharacterShadowToTexture4()
{
	extern bool GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW;
	if (GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW)
		return;

	if (!IsMapReady())
		return;

	if (Frustum::Instance().isIndoor)
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();

	if (Frustum::Instance().shadowType != 3)
	{
		if (Frustum::Instance().shadowType == 2)
		{
			bool canRender = rkMap.BeginRenderCharacterShadowToTexture4();
			if (canRender)
			{
				D3DXMATRIX matWorldIdent;
				D3DXMatrixIdentity(&matWorldIdent);
				STATEMANAGER.SetTransform(D3DTS_WORLD, &matWorldIdent);

				CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();

				if (Frustum::Instance().treeShadow == 0)
				{
					rkChrMgr.RenderShadowMainInstance();
				}

				if (Frustum::Instance().treeShadow >= 1)
				{
					rkChrMgr.RenderShadowAllInstances();
					rkMap.RenderArea2();
#ifndef __ANDROID__
					rkMap.__RenderTerrain_RenderHardwareTransformPatch2();
#endif
				}

#ifdef __ANDROID__
				if (Frustum::Instance().treeShadow >= 1)
#else
				if (Frustum::Instance().treeShadow == 3)
#endif
				{
					ms_lpd3dDevice->SetPixelShader(rkMap.m_lpShadowMapPixelShaderDepthTree);
					CSpeedTreeForestDirectX8::Instance().Render2(Forest_RenderAll);
				}

				STATEMANAGER.SetTransform(D3DTS_WORLD, &matWorldIdent);
				rkMap.EndRenderCharacterShadowToTexture4();
			}
		}
		else
		{
			if (Frustum::Instance().treeShadow == 3)
			{
				bool canRender = rkMap.BeginRenderCharacterShadowToTexture4();
				if (canRender)
				{
					CSpeedTreeForestDirectX8::Instance().Render3(Forest_RenderAll);
					rkMap.EndRenderCharacterShadowToTexture4();
				}
			}
		}
	}
}

inline float Interpolate(float fStart, float fEnd, float fPercent)
{
	return fStart + (fEnd - fStart) * fPercent;
}
struct CollisionChecker
{
	bool isBlocked;
	CInstanceBase* pInstance;
	CollisionChecker(CInstanceBase* pInstance) : pInstance(pInstance), isBlocked(false) {}
	void operator () (CGraphicObjectInstance* pOpponent)
	{
		if (isBlocked)
			return;

		if (!pOpponent)
			return;

		if (pInstance->IsBlockObject(*pOpponent))
			isBlocked=true;
	}
};

struct CollisionAdjustChecker
{
	bool isBlocked;
	CInstanceBase* pInstance;
	CollisionAdjustChecker(CInstanceBase* pInstance) : pInstance(pInstance), isBlocked(false) {}
	void operator () (CGraphicObjectInstance* pOpponent)
	{
		if (!pOpponent)
			return;

		if (pInstance->AvoidObject(*pOpponent))
			isBlocked=true;
	}
};

bool CPythonBackground::CheckAdvancing(CInstanceBase * pInstance)
{
	if (!IsMapReady())
		return true;

	Vector3d center;
	float radius;
	pInstance->GetGraphicThingInstanceRef().GetBoundingSphere(center,radius);

	CCullingManager & rkCullingMgr = CCullingManager::Instance();

	CollisionAdjustChecker kCollisionAdjustChecker(pInstance);
	rkCullingMgr.ForInRange(center, radius, &kCollisionAdjustChecker);
	if (kCollisionAdjustChecker.isBlocked)
	{
		CollisionChecker kCollisionChecker(pInstance);
		rkCullingMgr.ForInRange(center, radius, &kCollisionChecker);
		if (kCollisionChecker.isBlocked)
		{
			pInstance->BlockMovement();
			return true;
		}
		else
		{
			pInstance->NEW_MoveToDestPixelPositionDirection(pInstance->NEW_GetDstPixelPositionRef());
		}
		return false;
	}
	return false;
}

void CPythonBackground::RenderSky()
{
	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap=GetMapOutdoorRef();
	rkMap.RenderSky();
}

void CPythonBackground::RenderCloud()
{
	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap=GetMapOutdoorRef();
	rkMap.RenderCloud();
}

void CPythonBackground::RenderWater()
{
	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap=GetMapOutdoorRef();
	rkMap.RenderWater();
}

void CPythonBackground::RenderEffect()
{
	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap=GetMapOutdoorRef();
	rkMap.RenderEffect();
}

void CPythonBackground::RenderBeforeLensFlare()
{
	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap=GetMapOutdoorRef();
	rkMap.RenderBeforeLensFlare();
}

void CPythonBackground::RenderAfterLensFlare()
{
	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap=GetMapOutdoorRef();
	rkMap.RenderAfterLensFlare();
}

void CPythonBackground::ClearGuildArea()
{
	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap=GetMapOutdoorRef();
	rkMap.ClearGuildArea();
}

void CPythonBackground::RegisterGuildArea(int isx, int isy, int iex, int iey)
{
	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap=GetMapOutdoorRef();
	rkMap.RegisterGuildArea(isx, isy, iex, iey);
}

void CPythonBackground::SetCharacterDirLight()
{
	if (!IsMapReady())
		return;

	if (!mc_pcurEnvironmentData)
		return;

	STATEMANAGER.SetLight(0, &mc_pcurEnvironmentData->DirLights[ENV_DIRLIGHT_CHARACTER]);
}

void CPythonBackground::SetBackgroundDirLight()
{
	if (!IsMapReady())
		return;
	if (!mc_pcurEnvironmentData)
		return;

	STATEMANAGER.SetLight(0, &mc_pcurEnvironmentData->DirLights[ENV_DIRLIGHT_BACKGROUND]);
}

void CPythonBackground::GlobalPositionToLocalPosition(LONG& rGlobalX, LONG& rGlobalY)
{
	rGlobalX-=m_dwBaseX;
	rGlobalY-=m_dwBaseY;
}

void CPythonBackground::LocalPositionToGlobalPosition(LONG& rLocalX, LONG& rLocalY)
{
	rLocalX+=m_dwBaseX;
	rLocalY+=m_dwBaseY;
}

void CPythonBackground::RegisterDungeonMapName(const char * c_szMapName)
{
	m_kSet_strDungeonMapName.insert(c_szMapName);
}

CPythonBackground::TMapInfo* CPythonBackground::GlobalPositionToMapInfo(DWORD dwGlobalX, DWORD dwGlobalY)
{
	TMapInfoVector::iterator f = std::find_if(m_kVct_kMapInfo.begin(), m_kVct_kMapInfo.end(), FFindWarpMapName(dwGlobalX, dwGlobalY));
	if (f == m_kVct_kMapInfo.end())
		return NULL;

	return &(*f);
}

void CPythonBackground::Warp(DWORD dwX, DWORD dwY)
{
	TMapInfo* pkMapInfo = GlobalPositionToMapInfo(dwX, dwY);
	if (!pkMapInfo)
	{
		TraceError("NOT_FOUND_GLOBAL_POSITION(%d, %d)", dwX, dwY);
		return;
	}

	TMapInfo & rMapInfo = *pkMapInfo;
	assert( (dwX >= rMapInfo.m_dwBaseX) && (dwY >= rMapInfo.m_dwBaseY) );

	Frustum::Instance().grassTexturesInitialized = false;
	Frustum::Instance().grassMatrices.clear();
	Frustum::Instance().grassShadowMatrices.clear();
	Frustum::Instance().lastGrassPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	Frustum::Instance().lastGrassShadowPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	Frustum::Instance().waterPoints.clear();

	if (!LoadMap(rMapInfo.m_strName, float(dwX - rMapInfo.m_dwBaseX), float(dwY - rMapInfo.m_dwBaseY), 0))
	{
		// LOAD_MAP_ERROR_HANDLING
		PostQuitMessage(0);
		// END_OF_LOAD_MAP_ERROR_HANDLING
		return;
	}

	RefreshShadowLevel();
	//RefreshGraphicsLevel();

	CPythonMiniMap::Instance().LoadAtlas();

	m_dwBaseX=rMapInfo.m_dwBaseX;
	m_dwBaseY=rMapInfo.m_dwBaseY;

	m_strMapName = rMapInfo.m_strName;

	SetXMaxTree(m_iXMasTreeGrade);

	if (m_kSet_strDungeonMapName.end() != m_kSet_strDungeonMapName.find(m_strMapName))
	{
		EnableTerrainOnlyForHeight();

		CMapOutdoor& rkMap=GetMapOutdoorRef();
		rkMap.EnablePortal(TRUE);
	}

	m_kSet_iShowingPortalID.clear();
	m_kMap_dwTargetID_dwChrID.clear();
#if defined(WJ_COMBAT_ZONE)
	m_kMapCombatZone_dwTargetID_dwChrID.clear();
#endif
	m_kMap_dwTargetID_dwChrID2.clear();
	m_kMap_dwID_kReserveTargetEffect.clear();
#if defined(__BL_FOG_FIX__)
	bool bFogMode = CPythonSystem::Instance().GetFogMode();

	if (!m_strMapName.compare("metin2_map_defensewave"))
		bFogMode = true;

	SetFogMode(bFogMode);
#endif

#if defined(__BL_ENVIRONMENT_EFFECT_OPTION__)
	if (IsSnowMap(m_strMapName))
	{
		// FIX: We completely removed GetXMasShowEvent(). 
		// Now it ONLY checks if the player's UI button is ON!
		if (GetSnowModeOption() || CPythonSystem::Instance().GetRainModeOption())
			EnableSnowEnvironment();
		else
			DisableSnowEnvironment();
		
		// FIX: Textures now strictly obey the UI button!
		if (GetSnowTextureModeOption())
			SetSnowTexture(true);
		else
			SetSnowTexture(false);
	}
	else
	{
		if (CPythonSystem::Instance().GetRainModeOption())
			EnableSnowEnvironment();
		else
			DisableSnowEnvironment();
	}

	if (GetDayMode() != DAY_MODE_LIGHT && GetNightModeOption() && IsBoomMap(m_strMapName))
	{
		RegisterEnvironmentData(DAY_MODE_DARK, "d:/ymir work/environment/moonlight04.msenv");

		const TEnvironmentData* c_pEnvironmenData;
		if (GetEnvironmentData(DAY_MODE_DARK, &c_pEnvironmenData))
			ResetEnvironmentDataPtr(c_pEnvironmenData);
	}
#endif
}

#if defined(__BL_ENVIRONMENT_EFFECT_OPTION__)
void CPythonBackground::SetXMasShowEvent(bool bMode)
{
	m_bXMasShowEvent = bMode;
}

bool CPythonBackground::GetXMasShowEvent() const
{
	return m_bXMasShowEvent;
}

bool CPythonBackground::IsBoomMap(const std::string& sMapName) const
{
	static const std::vector<std::string> vMapName
	{
		"season1/metin2_map_sungzi_flame_hill_01",
		"season1/metin2_map_sungzi_flame_hill_02",
		"season1/metin2_map_sungzi_flame_hill_03",
		"season1/metin2_map_sungzi_desert_01",
		"season1/metin2_map_sungzi_desert_hill_01",
		"season1/metin2_map_sungzi_desert_hill_02",
		"season1/metin2_map_sungzi_desert_hill_03",
		"season2/metin2_map_empirewar03",
		"metin2_map_devilsCatacomb",
		"metin2_map_Mt_Thunder",
		"metin2_map_n_flame_dungeon_01",
		"metin2_map_n_snow_dungeon_01",
		"metin2_map_spiderdungeon_01",
		"metin2_map_spiderdungeon_02",
		"metin2_map_spiderdungeon_03",
		"metin2_map_deviltower1",
		"metin2_map_dawnmist_dungeon_01",
		"metin2_map_Mt_Th_dungeon_01",
		"metin2_12zi_stage",
		"metin2_map_n_flame_dragon",
		"metin2_map_defensewave",
		"metin2_map_defensewave_port",
		"metin2_map_mists_of_island",
		"metin2_map_miniboss_01",
		"metin2_map_miniboss_02",
		"metin2_map_labyrinth",
		"metin2_map_boss_crack_skipia",
		"metin2_map_boss_crack_flame",
		"metin2_map_boss_crack_snow",
		"metin2_map_boss_crack_dawnmist",
		"metin2_map_boss_awaken_skipia",
		"metin2_map_boss_awaken_flame",
		"metin2_map_boss_awaken_snow",
		"metin2_map_boss_awaken_dawnmist",
		"metin2_map_elemental_01",
		"metin2_map_elemental_02",
		"metin2_map_elemental_03",
		"metin2_map_elemental_04",
		"metin2_map_smhdungeon_01",
		"metin2_map_smhdungeon_02",
		"metin2_map_snake_temple_02",
		"metin2_map_whitedragoncave_01",
		"metin2_map_whitedragoncave_02",
		"metin2_map_whitedragoncave_boss",
		"metin2_map_anglar_dungeon_01",
		"metin2_map_otherworld_01",
		"metin2_map_otherworld_02",
		"metin2_map_otherworld_03",
		"metin2_map_otherworld_04",
		"metin2_map_smhgate_a1",
		"metin2_map_smhgate_b1",
		"metin2_map_smhgate_c1",
		"metin2_map_smhgate_threeway",
		"metin2_map_smhgate_desert",
		"metin2_map_smhgate_flame",
		"metin2_map_smhgate_snow",
		"metin2_map_smhgate_devils",
		"metin2_map_smhgate_dawnmist",
	};

	return std::find(vMapName.begin(), vMapName.end(), sMapName) == vMapName.end();
}

bool CPythonBackground::IsSnowMap(const std::string& sMapName) const
{
	static const std::vector<std::string> vMapName
	{
		"metin2_map_n_flame_01",
		"metin2_map_n_desert_01",
		"season1/metin2_map_sungzi_flame_hill_01",
		"season1/metin2_map_sungzi_flame_hill_02",
		"season1/metin2_map_sungzi_flame_hill_03",
		"season1/metin2_map_sungzi_desert_01",
		"season1/metin2_map_sungzi_desert_hill_01",
		"season1/metin2_map_sungzi_desert_hill_02",
		"season1/metin2_map_sungzi_desert_hill_03",
		"season2/metin2_map_empirewar03",
		"metin2_map_devilsCatacomb",
		"metin2_map_Mt_Thunder",
		"metin2_map_n_flame_dungeon_01",
		"metin2_map_spiderdungeon_01",
		"metin2_map_spiderdungeon_02",
		"metin2_map_spiderdungeon_03",
		"metin2_map_deviltower1",
		"metin2_map_dawnmist_dungeon_01",
		"metin2_map_Mt_Th_dungeon_01",
		"metin2_12zi_stage",
		"metin2_map_n_flame_dragon",
		"metin2_map_defensewave",
		"metin2_map_defensewave_port",
		"metin2_map_mists_of_island",
		"metin2_map_miniboss_01",
		"metin2_map_miniboss_02",
		"metin2_map_labyrinth",
		"metin2_map_boss_crack_skipia",
		"metin2_map_boss_crack_flame",
		"metin2_map_boss_crack_snow",
		"metin2_map_boss_crack_dawnmist",
		"metin2_map_boss_awaken_skipia",
		"metin2_map_boss_awaken_flame",
		"metin2_map_boss_awaken_snow",
		"metin2_map_boss_awaken_dawnmist",
		"metin2_guild_pve",
		"metin2_map_elemental_01",
		"metin2_map_elemental_02",
		"metin2_map_elemental_03",
		"metin2_map_elemental_04",
		"metin2_map_battleroyale",
		"metin2_map_maze_dungeon1",
		"metin2_map_maze_dungeon2",
		"metin2_map_maze_dungeon3",
		"metin2_map_smhdungeon_01",
		"metin2_map_smhdungeon_02",
		"metin2_map_snake_temple_02",
		"metin2_map_whitedragoncave_01",
		"metin2_map_whitedragoncave_02",
		"metin2_map_whitedragoncave_boss",
		"metin2_map_anglar_dungeon_01",
		"metin2_map_otherworld_01",
		"metin2_map_otherworld_02",
		"metin2_map_otherworld_03",
		"metin2_map_otherworld_04",
		"metin2_map_secretdungeon_01",
		"metin2_map_smhgate_a1",
		"metin2_map_smhgate_b1",
		"metin2_map_smhgate_c1",
		"metin2_map_smhgate_threeway",
		"metin2_map_smhgate_desert",
		"metin2_map_smhgate_flame",
		"metin2_map_smhgate_snow",
		"metin2_map_smhgate_devils",
		"metin2_map_smhgate_dawnmist",
	};

	return std::find(vMapName.begin(), vMapName.end(), sMapName) == vMapName.end();
}

void CPythonBackground::SetNightModeOption(bool bMode)
{
	m_bNightMode = bMode;
}

bool CPythonBackground::GetNightModeOption() const
{
	return m_bNightMode;
}

void CPythonBackground::SetSnowModeOption(bool bMode)
{
	m_bSnowMode = bMode;
}

bool CPythonBackground::GetSnowModeOption() const
{
	return m_bSnowMode;
}

void CPythonBackground::SetSnowTextureModeOption(bool bMode)
{
	m_bSnowTextureMode = bMode;
}

bool CPythonBackground::GetSnowTextureModeOption() const
{
	return m_bSnowTextureMode;
}

int CPythonBackground::GetDayMode() const
{
	return m_iDayMode;
}
#endif

void CPythonBackground::VisibleGuildArea()
{
	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap=GetMapOutdoorRef();
	rkMap.VisibleMarkedArea();

	m_bVisibleGuildArea = TRUE;
}

void CPythonBackground::DisableGuildArea()
{
	if (!IsMapReady())
		return;

	CMapOutdoor& rkMap=GetMapOutdoorRef();
	rkMap.DisableMarkedArea();

	m_bVisibleGuildArea = FALSE;
}

const char * CPythonBackground::GetWarpMapName()
{
	return m_strMapName.c_str();
}

void CPythonBackground::ChangeToDay()
{
	m_iDayMode = DAY_MODE_LIGHT;
}

void CPythonBackground::ChangeToNight()
{
	m_iDayMode = DAY_MODE_DARK;
}

void CPythonBackground::EnableSnowEnvironment()
{
	m_SnowEnvironment.Enable();
}

void CPythonBackground::DisableSnowEnvironment()
{
	m_SnowEnvironment.Disable();
}

const D3DXVECTOR3 c_v3TreePos = D3DXVECTOR3(76500.0f, -60900.0f, 20215.0f);

void CPythonBackground::SetXMaxTree(int iGrade)
{
	if (!m_pkMap)
		return;

	assert(iGrade >= 0 && iGrade <= 3);
	m_iXMasTreeGrade = iGrade;

	CMapOutdoor& rkMap=GetMapOutdoorRef();

	if ("map_n_snowm_01" != m_strMapName)
	{
		rkMap.XMasTree_Destroy();
		return;
	}

	if (0 == iGrade)
	{
		rkMap.XMasTree_Destroy();
		return;
	}

	//////////////////////////////////////////////////////////////////////

	iGrade -= 1;
	iGrade = max(iGrade, 0);
	iGrade = min(iGrade, 2);

	static std::string s_strTreeName[3] = {
		"d:/ymir work/tree/christmastree1.spt",
		"d:/ymir work/tree/christmastree2.spt",
		"d:/ymir work/tree/christmastree3.spt"
	};
	static std::string s_strEffectName[3] = {
		"d:/ymir work/effect/etc/christmas_tree/tree_1s.mse",
		"d:/ymir work/effect/etc/christmas_tree/tree_2s.mse",
		"d:/ymir work/effect/etc/christmas_tree/tree_3s.mse",
	};
	rkMap.XMasTree_Set(c_v3TreePos.x, c_v3TreePos.y, c_v3TreePos.z, s_strTreeName[iGrade].c_str(), s_strEffectName[iGrade].c_str());
}

#if defined(WJ_COMBAT_ZONE)
void CPythonBackground::CreateCombatZoneTargetsLeaving(DWORD dwID, DWORD dwChrVID)
{
	m_kMapCombatZone_dwTargetID_dwChrID.insert(std::make_pair(dwID, dwChrVID));
}

void CPythonBackground::CreateCombatZoneTargetsLeaving(DWORD dwID, long lx, long ly)
{
	if (m_kMapCombatZone_dwTargetID_dwChrID.end() != m_kMapCombatZone_dwTargetID_dwChrID.find(dwID))
		return;

	CMapOutdoor& rkMap=GetMapOutdoorRef();

	DWORD dwBaseX;
	DWORD dwBaseY;
	rkMap.GetBaseXY(&dwBaseX, &dwBaseY);

	int ilx = +(lx-int(dwBaseX));
	int ily = -(ly-int(dwBaseY));

	float fHeight = rkMap.GetHeight(float(ilx), float(ily));
	CreateSpecialEffect(dwID, ilx, ily, fHeight, g_strEffectTargetLeaving.c_str());
}
#endif

void CPythonBackground::CreateTargetEffect(DWORD dwID, DWORD dwChrVID)
{
	m_kMap_dwTargetID_dwChrID.emplace(dwID, dwChrVID);
}

void CPythonBackground::CreateTargetEffect(DWORD dwID, long lx, long ly)
{
	if (m_kMap_dwTargetID_dwChrID.end() != m_kMap_dwTargetID_dwChrID.find(dwID))
		return;

// --- FIX: Prevent static glitch! ---
	// If this ID is already being used by our custom Shop Target, abort so we don't spawn a vanilla quest arrow over it!
	if (m_kMap_dwTargetID_dwChrID2.end() != m_kMap_dwTargetID_dwChrID2.find(dwID))
		return;
	// -----------------------------------

	CMapOutdoor& rkMap=GetMapOutdoorRef();

	DWORD dwBaseX;
	DWORD dwBaseY;
	rkMap.GetBaseXY(&dwBaseX, &dwBaseY);

	int ilx = +(lx-int(dwBaseX));
	int ily = -(ly-int(dwBaseY));

	float fHeight = rkMap.GetHeight(float(ilx), float(ily));

	if (0.0f == fHeight)
	{
		SReserveTargetEffect ReserveTargetEffect;
		ReserveTargetEffect.ilx = ilx;
		ReserveTargetEffect.ily = ily;
		m_kMap_dwID_kReserveTargetEffect.emplace(dwID, ReserveTargetEffect);
		return;
	}

	CreateSpecialEffect(dwID, ilx, ily, fHeight, g_strEffectName.c_str());
}

void CPythonBackground::CreateTargetEffect2(DWORD dwID, DWORD dwChrVID)
{
	// Changed from insert(make_pair) to emplace
	m_kMap_dwTargetID_dwChrID2.emplace(dwID, dwChrVID);
}

void CPythonBackground::CreateTargetEffect2(DWORD dwID, long lx, long ly)
{
	if (m_kMap_dwTargetID_dwChrID2.end() != m_kMap_dwTargetID_dwChrID2.find(dwID))
		return;

	CMapOutdoor& rkMap = GetMapOutdoorRef();
	DWORD dwBaseX, dwBaseY;
	rkMap.GetBaseXY(&dwBaseX, &dwBaseY);

	int ilx = +(lx - int(dwBaseX));
	int ily = -(ly - int(dwBaseY));
	float fHeight = rkMap.GetHeight(float(ilx), float(ily));

	if (0.0f == fHeight)
	{
		SReserveTargetEffect ReserveTargetEffect;
		ReserveTargetEffect.ilx = ilx;
		ReserveTargetEffect.ily = ily;
		
		// Changed from insert(make_pair) to emplace
		m_kMap_dwID_kReserveTargetEffect.emplace(dwID, ReserveTargetEffect);
		return;
	}

	CreateSpecialEffect(dwID, ilx, ily, fHeight, g_strEffectShopSearch.c_str());
}

void CPythonBackground::DeleteTargetEffect(DWORD dwID)
{
	if (m_kMap_dwID_kReserveTargetEffect.end() != m_kMap_dwID_kReserveTargetEffect.find(dwID))
	{
		m_kMap_dwID_kReserveTargetEffect.erase(dwID);
	}
	
	if (m_kMap_dwTargetID_dwChrID.end() != m_kMap_dwTargetID_dwChrID.find(dwID))
	{
		m_kMap_dwTargetID_dwChrID.erase(dwID);
	}
#if defined(WJ_COMBAT_ZONE)
	if (m_kMapCombatZone_dwTargetID_dwChrID.end() != m_kMapCombatZone_dwTargetID_dwChrID.find(dwID))
	{
		m_kMapCombatZone_dwTargetID_dwChrID.erase(dwID);
	}
#endif

	if (m_kMap_dwTargetID_dwChrID2.end() != m_kMap_dwTargetID_dwChrID2.find(dwID))
	{
		m_kMap_dwTargetID_dwChrID2.erase(dwID);
	}

	DeleteSpecialEffect(dwID);
}

void CPythonBackground::CreateSpecialEffect(DWORD dwID, float fx, float fy, float fz, const char * c_szFileName)
{
	CMapOutdoor& rkMap=GetMapOutdoorRef();
	rkMap.SpecialEffect_Create(dwID, fx, fy, fz, c_szFileName);
}

void CPythonBackground::DeleteSpecialEffect(DWORD dwID)
{
	CMapOutdoor& rkMap=GetMapOutdoorRef();
	rkMap.SpecialEffect_Delete(dwID);
}

#ifdef ENABLE_SHOP_SEARCH_SYSTEM
void CPythonBackground::CreateSpecialShopPosEffect(DWORD dwID, float fx, float fy, float fz, const char * c_szFileName)
{
	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.SpecialShopPosEffect_Create(dwID, fx, fy, fz, c_szFileName);
}

void CPythonBackground::DeleteSpecialShopPosEffect()
{
	CMapOutdoor& rkMap = GetMapOutdoorRef();
	rkMap.SpecialShopPosEffect_Delete();
}
#endif
//martysama0134's fb0d8d5d0566583187a94f9a823b2075