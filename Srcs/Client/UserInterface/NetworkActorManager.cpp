#include "StdAfx.h"
#include "NetworkActorManager.h"
#include "PythonCharacterManager.h"
#include "PythonItem.h"

#include "AbstractPlayer.h"
#ifdef ENABLE_CSHIELD
#include "CShield.h"
#endif

void SNetworkActorData::UpdatePosition()
{
	DWORD dwClientCurTime=ELTimer_GetMSec();
	DWORD dwElapsedTime=dwClientCurTime-m_dwClientSrcTime;

	if (dwElapsedTime<m_dwDuration)
	{
		float fRate=float(dwElapsedTime)/float(m_dwDuration);
		m_lCurX=LONG((m_lDstX-m_lSrcX)*fRate+m_lSrcX);
		m_lCurY=LONG((m_lDstY-m_lSrcY)*fRate+m_lSrcY);
	}
	else
	{
		m_lCurX=m_lDstX;
		m_lCurY=m_lDstY;
	}
}

void SNetworkActorData::SetDstPosition(DWORD dwServerTime, LONG lDstX, LONG lDstY, DWORD dwDuration)
{
	m_lSrcX=m_lCurX;
	m_lSrcY=m_lCurY;
	m_lDstX=lDstX;
	m_lDstY=lDstY;

	m_dwDuration=dwDuration;
	m_dwServerSrcTime=dwServerTime;
	m_dwClientSrcTime=ELTimer_GetMSec();
}

void SNetworkActorData::SetPosition(LONG lPosX, LONG lPosY)
{
	m_lDstX=m_lSrcX=m_lCurX=lPosX;
	m_lDstY=m_lSrcY=m_lCurY=lPosY;
}

// NETWORK_ACTOR_DATA_COPY
SNetworkActorData::SNetworkActorData(const SNetworkActorData& src)
{
	__copy__(src);
}

void SNetworkActorData::operator=(const SNetworkActorData& src)
{
	__copy__(src);
}

void SNetworkActorData::__copy__(const SNetworkActorData& src)
{
	m_bType = src.m_bType;
	m_dwVID = src.m_dwVID;
	m_dwStateFlags = src.m_dwStateFlags;
	m_dwEmpireID = src.m_dwEmpireID;
	m_dwRace = src.m_dwRace;
	m_dwMovSpd = src.m_dwMovSpd;
	m_dwAtkSpd = src.m_dwAtkSpd;
	m_fRot = src.m_fRot;
	m_lCurX = src.m_lCurX;
	m_lCurY = src.m_lCurY;
	m_lSrcX = src.m_lSrcX;
	m_lSrcY = src.m_lSrcY;
	m_lDstX = src.m_lDstX;
	m_lDstY = src.m_lDstY;
	m_kAffectFlags.CopyInstance(src.m_kAffectFlags);

	m_dwServerSrcTime = src.m_dwServerSrcTime;
	m_dwClientSrcTime = src.m_dwClientSrcTime;
	m_dwDuration = src.m_dwDuration;

	m_dwArmor = src.m_dwArmor;
	m_dwWeapon = src.m_dwWeapon;
	m_dwHair = src.m_dwHair;
#ifdef ELEMENT_SPELL_WORLDARD
	m_dwElementsEffect = src.m_dwElementsEffect;
#endif
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	m_dwAcce = src.m_dwAcce;
#endif
#ifdef ENABLE_QUIVER_SYSTEM
	m_dwArrow = src.m_dwArrow;
#endif

	m_dwOwnerVID = src.m_dwOwnerVID;

	m_sAlignment = src.m_sAlignment;
	m_byPKMode = src.m_byPKMode;
	m_dwMountVnum = src.m_dwMountVnum;

	m_dwGuildID = src.m_dwGuildID;
	m_dwLevel = src.m_dwLevel;
#ifdef ENABLE_CONQUEROR_LEVEL
	m_dwConquerorLevel = src.m_dwConquerorLevel;
#endif
#ifdef ENABLE_AURA_SYSTEM
	m_dwAura = src.m_dwAura;
#endif
	m_stName = src.m_stName;
#if defined(WJ_COMBAT_ZONE)
	combat_zone_rank = src.combat_zone_rank;
	combat_zone_points = src.combat_zone_points;
#endif
}
// END_OF_NETWORK_ACTOR_DATA_COPY

SNetworkActorData::SNetworkActorData()
{
	SetPosition(0, 0);

	m_bType=0;
	m_dwVID=0;
	m_dwStateFlags=0;
	m_dwRace=0;
	m_dwMovSpd=0;
	m_dwAtkSpd=0;
	m_fRot=0.0f;
	m_dwArmor=0;
	m_dwWeapon=0;
	m_dwHair=0;
#ifdef ELEMENT_SPELL_WORLDARD
	m_dwElementsEffect = 0;
#endif
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	m_dwAcce = 0;
#endif
#ifdef ENABLE_QUIVER_SYSTEM
	m_dwArrow=0;
#endif

	m_dwEmpireID=0;
	m_dwOwnerVID=0;

	m_dwDuration=0;
	m_dwClientSrcTime=0;
	m_dwServerSrcTime=0;

	m_sAlignment=0;
	m_byPKMode=0;
	m_dwMountVnum=0;
#ifdef ENABLE_CONQUEROR_LEVEL
	m_dwConquerorLevel=0;
#endif
#ifdef ENABLE_AURA_SYSTEM
	m_dwAura=0;
#endif
	m_stName="";
#if defined(WJ_COMBAT_ZONE)
	combat_zone_rank = 0;
#endif

	m_kAffectFlags.Clear();
}

////////////////////////////////////////////////////////////////////////////////

CNetworkActorManager::CNetworkActorManager()
{
	m_dwMainVID=0;
}

CNetworkActorManager::~CNetworkActorManager()
{
}

void CNetworkActorManager::Destroy()
{
	m_kNetActorDict.clear();

	m_dwMainVID=0;
	m_lMainPosX=0;
	m_lMainPosY=0;
}

void CNetworkActorManager::SetMainActorVID(DWORD dwVID)
{
	m_dwMainVID=dwVID;
	m_lMainPosX=0;
	m_lMainPosY=0;

	m_kNetActorDict.clear();
}

void CNetworkActorManager::Update()
{
	__OLD_Update();
}

void CNetworkActorManager::__OLD_Update()
{
	__UpdateMainActor();

	CPythonCharacterManager& rkChrMgr = __GetCharacterManager();

	for (std::map<DWORD, SNetworkActorData>::iterator i = m_kNetActorDict.begin(); i != m_kNetActorDict.end(); ++i)
	{
		SNetworkActorData& rkNetActorData = i->second;
		rkNetActorData.UpdatePosition();

		CInstanceBase * pkInstFind = rkChrMgr.GetInstancePtr(rkNetActorData.m_dwVID);
		if (!pkInstFind)
		{
			if (__IsVisibleActor(rkNetActorData))
				__AppendCharacterManagerActor(rkNetActorData);
		}
	}
}

CInstanceBase* CNetworkActorManager::__FindActor(SNetworkActorData& rkNetActorData, LONG lDstX, LONG lDstY)
{
	CPythonCharacterManager& rkChrMgr=__GetCharacterManager();
	CInstanceBase * pkInstFind = rkChrMgr.GetInstancePtr(rkNetActorData.m_dwVID);
	if (!pkInstFind)
	{
		if (__IsVisiblePos(lDstX, lDstY))
			return __AppendCharacterManagerActor(rkNetActorData);

		return NULL;
	}

	return pkInstFind;
}

CInstanceBase* CNetworkActorManager::__FindActor(SNetworkActorData& rkNetActorData)
{
	CPythonCharacterManager& rkChrMgr=__GetCharacterManager();
	CInstanceBase * pkInstFind = rkChrMgr.GetInstancePtr(rkNetActorData.m_dwVID);
	if (!pkInstFind)
	{
		if (__IsVisibleActor(rkNetActorData))
			return __AppendCharacterManagerActor(rkNetActorData);

		return NULL;
	}

	return pkInstFind;
}

void CNetworkActorManager::__RemoveAllGroundItems()
{
	CPythonItem& rkItemMgr=CPythonItem::Instance();
	rkItemMgr.DeleteAllItems();
}

void CNetworkActorManager::__RemoveAllActors()
{
	m_kNetActorDict.clear();

	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();
	rkChrMgr.DeleteAllInstances();
}

void CNetworkActorManager::__RemoveDynamicActors()
{
	//copy it
	std::vector<DWORD> dwCharacterVIDList;

	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	for(CPythonCharacterManager::CharacterIterator i = rkChrMgr.CharacterInstanceBegin(); i!=rkChrMgr.CharacterInstanceEnd(); ++i)
	{
		dwCharacterVIDList.push_back( (*i)->GetVirtualID() );
	}

	for( int i = 0; i < dwCharacterVIDList.size(); ++i )
	{
		CInstanceBase*  pkInstEach = rkChrMgr.GetInstancePtr( dwCharacterVIDList[i] );
		if( !pkInstEach )
			continue;

		CActorInstance* rkActorEach=pkInstEach->GetGraphicThingInstancePtr();
		if (rkActorEach->IsPC() || rkActorEach->IsNPC() || rkActorEach->IsEnemy() || rkActorEach->IsStone()) // @fixme034 (missing IsStone())
		{
			rkChrMgr.DeleteInstance(dwCharacterVIDList[i]);
			auto it = m_kNetActorDict.find(dwCharacterVIDList[i]);
			if (it != m_kNetActorDict.end())
				m_kNetActorDict.erase(it);
		}
	}

	rkChrMgr.DestroyDeadInstanceList();
}

void CNetworkActorManager::__UpdateMainActor()
{
	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();
	CInstanceBase* pkInstMain=rkChrMgr.GetMainInstancePtr();
	if (!pkInstMain)
		return;

	TPixelPosition kPPosMain;
	pkInstMain->NEW_GetPixelPosition(&kPPosMain);

	m_lMainPosX=LONG(kPPosMain.x);
	m_lMainPosY=LONG(kPPosMain.y);
}

extern bool IsWall(unsigned race);

bool CNetworkActorManager::__IsVisibleActor(const SNetworkActorData& c_rkNetActorData)
{
	if (__IsMainActorVID(c_rkNetActorData.m_dwVID))
		return true;

	if (c_rkNetActorData.m_kAffectFlags.IsSet(CInstanceBase::AFFECT_SHOW_ALWAYS))
		return true;

	if (__IsVisiblePos(c_rkNetActorData.m_lCurX, c_rkNetActorData.m_lCurY))
		return true;

	if (IsWall(c_rkNetActorData.m_dwRace))
		return true;

 	return false;
}

bool CNetworkActorManager::__IsVisiblePos(LONG lPosX, LONG lPosY)
{
	const int64_t dx = static_cast<int64_t>(lPosX) - static_cast<int64_t>(m_lMainPosX);
	const int64_t dy = static_cast<int64_t>(lPosY) - static_cast<int64_t>(m_lMainPosY);
	const int64_t distSq = dx * dx + dy * dy;

	extern int CHAR_STAGE_VIEW_BOUND;
	const int64_t bound = static_cast<int64_t>(CHAR_STAGE_VIEW_BOUND);
	return (distSq < (bound * bound));
}

bool CNetworkActorManager::__IsMainActorVID(DWORD dwVID)
{
	if (dwVID == m_dwMainVID)
		return true;

	return false;
}

CPythonCharacterManager& CNetworkActorManager::__GetCharacterManager()
{
	return CPythonCharacterManager::Instance();
}

void CNetworkActorManager::__RemoveCharacterManagerActor(SNetworkActorData& rkNetActorData)
{
	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();

	if (__IsMainActorVID(rkNetActorData.m_dwVID))
	{
		rkChrMgr.DeleteInstance(rkNetActorData.m_dwVID);
	}
	else
	{
		rkChrMgr.DeleteInstanceByFade(rkNetActorData.m_dwVID);
	}
}

CInstanceBase* CNetworkActorManager::__AppendCharacterManagerActor(SNetworkActorData& rkNetActorData)
{
	Lognf(0, "CNetworkActorManager::__AppendCharacterManagerActor(%d, %s)", rkNetActorData.m_dwVID, rkNetActorData.m_stName.c_str());

	DWORD dwVID=rkNetActorData.m_dwVID;

	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();

	CInstanceBase::SCreateData kCreateData;
	kCreateData.m_bType=rkNetActorData.m_bType;
	kCreateData.m_dwLevel=rkNetActorData.m_dwLevel;
#ifdef ENABLE_CONQUEROR_LEVEL
	kCreateData.m_dwConquerorLevel=rkNetActorData.m_dwConquerorLevel;
#endif
	kCreateData.m_dwGuildID=rkNetActorData.m_dwGuildID;
	kCreateData.m_dwEmpireID=rkNetActorData.m_dwEmpireID;
#ifdef ENABLE_CSHIELD
	kCreateData.m_dwVID=eData(&rkNetActorData.m_dwVID, sizeof(DWORD));
#else
	kCreateData.m_dwVID=rkNetActorData.m_dwVID;
#endif
	kCreateData.m_dwMountVnum=rkNetActorData.m_dwMountVnum;
	kCreateData.m_dwRace=rkNetActorData.m_dwRace;
	kCreateData.m_fRot=rkNetActorData.m_fRot;
#ifdef ENABLE_CSHIELD
 	kCreateData.m_lPosX=eData(&rkNetActorData.m_lCurX, sizeof(LONG));
#else
	kCreateData.m_lPosX=rkNetActorData.m_lCurX;
#endif
	kCreateData.m_lPosY=rkNetActorData.m_lCurY;
	kCreateData.m_stName=rkNetActorData.m_stName;
#if defined(WJ_COMBAT_ZONE)
	kCreateData.combat_zone_rank = rkNetActorData.combat_zone_rank;
	kCreateData.combat_zone_points = rkNetActorData.combat_zone_points;
#endif
	kCreateData.m_dwStateFlags=rkNetActorData.m_dwStateFlags;
	kCreateData.m_dwMovSpd=rkNetActorData.m_dwMovSpd;
	kCreateData.m_dwAtkSpd=rkNetActorData.m_dwAtkSpd;
	kCreateData.m_sAlignment=rkNetActorData.m_sAlignment;
	kCreateData.m_byPKMode=rkNetActorData.m_byPKMode;
	kCreateData.m_kAffectFlags=rkNetActorData.m_kAffectFlags;
	kCreateData.m_dwArmor=rkNetActorData.m_dwArmor;
	kCreateData.m_dwWeapon=rkNetActorData.m_dwWeapon;
	kCreateData.m_dwHair=rkNetActorData.m_dwHair;
#ifdef ELEMENT_SPELL_WORLDARD
	kCreateData.m_dwElementsEffect = rkNetActorData.m_dwElementsEffect;
#endif
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	kCreateData.m_dwAcce = rkNetActorData.m_dwAcce;
#endif
#ifdef ENABLE_QUIVER_SYSTEM
	kCreateData.m_dwArrow=rkNetActorData.m_dwArrow;
#endif
#ifdef ENABLE_AURA_SYSTEM
	kCreateData.m_dwAura=rkNetActorData.m_dwAura;
#endif
	kCreateData.m_isMain=__IsMainActorVID(dwVID);

	CInstanceBase* pOldInstance = rkChrMgr.GetInstancePtr(dwVID);
	if (pOldInstance)
	{
		TPixelPosition oldPos;
		pOldInstance->GetGraphicThingInstancePtr()->GetPixelPosition( &oldPos );

		BOOL bIsMountingHorse = pOldInstance->IsMountingHorse();
		if( (bIsMountingHorse && kCreateData.m_dwMountVnum == 0) ||
			(!bIsMountingHorse && kCreateData.m_dwMountVnum != 0) )
		{
			//Override Pos
#ifdef ENABLE_CSHIELD
			LONG posX = static_cast<LONG>(oldPos.x);
			kCreateData.m_lPosX = eData(&posX, sizeof(LONG));
#else
			kCreateData.m_lPosX = oldPos.x;
#endif
			kCreateData.m_lPosY = -oldPos.y;
		}

		rkChrMgr.DeleteInstance(dwVID);
	}

	CInstanceBase* pNewInstance = rkChrMgr.CreateInstance(kCreateData);
	if (!pNewInstance)
		return NULL;

	if (kCreateData.m_isMain)
	{
		IAbstractPlayer & rkPlayer = IAbstractPlayer::GetSingleton();
		rkPlayer.SetMainCharacterIndex(dwVID);

		m_lMainPosX=rkNetActorData.m_lCurX;
		m_lMainPosY=rkNetActorData.m_lCurY;
	}

	DWORD dwClientCurTime=ELTimer_GetMSec();
	DWORD dwElapsedTime=dwClientCurTime-rkNetActorData.m_dwClientSrcTime;

	if (dwElapsedTime<rkNetActorData.m_dwDuration)
	{
		TPixelPosition kPPosDst;
		kPPosDst.x=float(rkNetActorData.m_lDstX);
		kPPosDst.y=float(rkNetActorData.m_lDstY);
		kPPosDst.z=0;
		pNewInstance->PushTCPState(rkNetActorData.m_dwServerSrcTime+dwElapsedTime, kPPosDst, rkNetActorData.m_fRot, CInstanceBase::FUNC_MOVE, 0);
	}
	return pNewInstance;
}

void CNetworkActorManager::AppendActor(const SNetworkActorData& c_rkNetActorData)
{
	if (__IsMainActorVID(c_rkNetActorData.m_dwVID))
	{
		bool bChangeMountStatus = false;

		CInstanceBase* pMainInstance = CPythonCharacterManager::Instance().GetInstancePtr(c_rkNetActorData.m_dwVID);
		if( pMainInstance )
		{
			BOOL bIsMountingHorse = pMainInstance->IsMountingHorse();
			bChangeMountStatus = (bIsMountingHorse && c_rkNetActorData.m_dwMountVnum == 0) || (!bIsMountingHorse && c_rkNetActorData.m_dwMountVnum != 0);
		}

		if (!bChangeMountStatus)
		{
			//__RemoveAllActors();
			__RemoveDynamicActors();
			__RemoveAllGroundItems();
		}

	}

	SNetworkActorData& rkNetActorData = m_kNetActorDict[c_rkNetActorData.m_dwVID];
	rkNetActorData = c_rkNetActorData;

	if (__IsVisibleActor(rkNetActorData))
	{
		if (!__AppendCharacterManagerActor(rkNetActorData))
			m_kNetActorDict.erase(c_rkNetActorData.m_dwVID);
	}
}

void CNetworkActorManager::RemoveActor(DWORD dwVID)
{
	std::map<DWORD, SNetworkActorData>::iterator f=m_kNetActorDict.find(dwVID);
	if (m_kNetActorDict.end()==f)
	{
#ifdef _DEBUG
		TraceError("CNetworkActorManager::RemoveActor(dwVID=%d) - NOT EXIST VID", dwVID);
#endif
		return;
	}

	SNetworkActorData& rkNetActorData=f->second;
	__RemoveCharacterManagerActor(rkNetActorData);

	m_kNetActorDict.erase(f);
}

void CNetworkActorManager::UpdateActor(const SNetworkUpdateActorData& c_rkNetUpdateActorData)
{
	std::map<DWORD, SNetworkActorData>::iterator f=m_kNetActorDict.find(c_rkNetUpdateActorData.m_dwVID);
	if (m_kNetActorDict.end()==f)
	{
#ifdef _DEBUG
		TraceError("CNetworkActorManager::UpdateActor(dwVID=%d) - NOT EXIST VID", c_rkNetUpdateActorData.m_dwVID);
#endif
		return;
	}

	SNetworkActorData& rkNetActorData=f->second;

	CInstanceBase* pkInstFind=__FindActor(rkNetActorData);
	if (pkInstFind)
	{
		pkInstFind->ChangeArmor(c_rkNetUpdateActorData.m_dwArmor);
		pkInstFind->ChangeWeapon(c_rkNetUpdateActorData.m_dwWeapon);
		pkInstFind->ChangeHair(c_rkNetUpdateActorData.m_dwHair);
#ifdef ELEMENT_SPELL_WORLDARD
		pkInstFind->SetElementEffect(c_rkNetUpdateActorData.m_dwElementsEffect);
#endif
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		pkInstFind->ChangeAcce(c_rkNetUpdateActorData.m_dwAcce);
#endif
#ifdef ENABLE_QUIVER_SYSTEM
		pkInstFind->SetArrow(c_rkNetUpdateActorData.m_dwArrow);
#endif
		pkInstFind->ChangeGuild(c_rkNetUpdateActorData.m_dwGuildID);
		pkInstFind->SetAffectFlagContainer(c_rkNetUpdateActorData.m_kAffectFlags);
		pkInstFind->SetMoveSpeed(c_rkNetUpdateActorData.m_dwMovSpd);
		pkInstFind->SetAttackSpeed(c_rkNetUpdateActorData.m_dwAtkSpd);
		pkInstFind->SetAlignment(c_rkNetUpdateActorData.m_sAlignment);
#ifdef ENABLE_CONQUEROR_LEVEL
		if (c_rkNetUpdateActorData.m_dwConquerorLevel > 0)
			pkInstFind->SetConquerorLevelText(c_rkNetUpdateActorData.m_dwConquerorLevel);
#endif
#ifdef ENABLE_AURA_SYSTEM
		pkInstFind->ChangeAura(c_rkNetUpdateActorData.m_dwAura);
#endif
		pkInstFind->SetPKMode(c_rkNetUpdateActorData.m_byPKMode);
		pkInstFind->SetStateFlags(c_rkNetUpdateActorData.m_dwStateFlags);
#if defined(WJ_COMBAT_ZONE)
		pkInstFind->SetCombatZonePoints(c_rkNetUpdateActorData.combat_zone_points);
#endif
	}

	rkNetActorData.m_kAffectFlags=c_rkNetUpdateActorData.m_kAffectFlags;
#if defined(WJ_COMBAT_ZONE)
	rkNetActorData.combat_zone_points = c_rkNetUpdateActorData.combat_zone_points;
#endif
	rkNetActorData.m_dwGuildID=c_rkNetUpdateActorData.m_dwGuildID;
#ifdef ENABLE_CONQUEROR_LEVEL
	rkNetActorData.m_dwConquerorLevel=c_rkNetUpdateActorData.m_dwConquerorLevel;
#endif
	rkNetActorData.m_dwMovSpd=c_rkNetUpdateActorData.m_dwMovSpd;
	rkNetActorData.m_dwAtkSpd=c_rkNetUpdateActorData.m_dwAtkSpd;
	rkNetActorData.m_dwArmor=c_rkNetUpdateActorData.m_dwArmor;
	rkNetActorData.m_dwWeapon=c_rkNetUpdateActorData.m_dwWeapon;
	rkNetActorData.m_dwHair=c_rkNetUpdateActorData.m_dwHair;
#ifdef ELEMENT_SPELL_WORLDARD
	rkNetActorData.m_dwElementsEffect = c_rkNetUpdateActorData.m_dwElementsEffect;
#endif
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	rkNetActorData.m_dwAcce = c_rkNetUpdateActorData.m_dwAcce;
#endif
#ifdef ENABLE_QUIVER_SYSTEM
	rkNetActorData.m_dwArrow=c_rkNetUpdateActorData.m_dwArrow;
#endif
#ifdef ENABLE_AURA_SYSTEM
	rkNetActorData.m_dwAura=c_rkNetUpdateActorData.m_dwAura;
#endif
	rkNetActorData.m_sAlignment=c_rkNetUpdateActorData.m_sAlignment;
	rkNetActorData.m_byPKMode=c_rkNetUpdateActorData.m_byPKMode;
}

#ifdef ENABLE_CSHIELD
void CNetworkActorManager::MoveActor(const SNetworkMoveActorData& c_rkNetMoveActorData)
{
	std::map<DWORD, SNetworkActorData>::iterator f=m_kNetActorDict.find(dData<DWORD>(c_rkNetMoveActorData.m_dwVID));
	if (m_kNetActorDict.end()==f)
	{
#ifdef _DEBUG
		TraceError("CNetworkActorManager::MoveActor(dwVID=%d) - NOT EXIST VID", dData<DWORD>(c_rkNetMoveActorData.m_dwVID));
#endif
		return;
	}

	SNetworkActorData& rkNetActorData=f->second;

	CInstanceBase* pkInstFind=__FindActor(rkNetActorData, dData<LONG>(c_rkNetMoveActorData.m_lPosX), c_rkNetMoveActorData.m_lPosY);
	if (pkInstFind)
	{
		TPixelPosition kPPosDst;
		kPPosDst.x=float(dData<LONG>(c_rkNetMoveActorData.m_lPosX));
		kPPosDst.y=float(c_rkNetMoveActorData.m_lPosY);
		kPPosDst.z=0.0f;

		pkInstFind->PushTCPState(c_rkNetMoveActorData.m_dwTime, kPPosDst, 
			c_rkNetMoveActorData.m_fRot, c_rkNetMoveActorData.m_dwFunc, c_rkNetMoveActorData.m_dwArg);	
	}

	rkNetActorData.SetDstPosition(c_rkNetMoveActorData.m_dwTime,
		dData<LONG>(c_rkNetMoveActorData.m_lPosX), c_rkNetMoveActorData.m_lPosY, c_rkNetMoveActorData.m_dwDuration);
	rkNetActorData.m_fRot=c_rkNetMoveActorData.m_fRot;		
}
#else
void CNetworkActorManager::MoveActor(const SNetworkMoveActorData& c_rkNetMoveActorData)
{
	std::map<DWORD, SNetworkActorData>::iterator f = m_kNetActorDict.find(c_rkNetMoveActorData.m_dwVID);
	if (m_kNetActorDict.end() == f)
	{
#ifdef _DEBUG
		TraceError("CNetworkActorManager::MoveActor(dwVID=%d) - NOT EXIST VID", c_rkNetMoveActorData.m_dwVID);
#endif
		return;
	}

	SNetworkActorData& rkNetActorData = f->second;

	CInstanceBase* pkInstFind = __FindActor(rkNetActorData, c_rkNetMoveActorData.m_lPosX, c_rkNetMoveActorData.m_lPosY);
	if (pkInstFind)
	{
		TPixelPosition kPPosDst;
		kPPosDst.x = float(c_rkNetMoveActorData.m_lPosX);
		kPPosDst.y = float(c_rkNetMoveActorData.m_lPosY);
		kPPosDst.z = 0.0f;

		pkInstFind->PushTCPState(c_rkNetMoveActorData.m_dwTime, kPPosDst,
			c_rkNetMoveActorData.m_fRot, c_rkNetMoveActorData.m_dwFunc, c_rkNetMoveActorData.m_dwArg);
	}

	rkNetActorData.SetDstPosition(c_rkNetMoveActorData.m_dwTime,
		c_rkNetMoveActorData.m_lPosX, c_rkNetMoveActorData.m_lPosY, c_rkNetMoveActorData.m_dwDuration);
	rkNetActorData.m_fRot = c_rkNetMoveActorData.m_fRot;
}
#endif

void CNetworkActorManager::SyncActor(DWORD dwVID, LONG lPosX, LONG lPosY)
{
	std::map<DWORD, SNetworkActorData>::iterator f=m_kNetActorDict.find(dwVID);
	if (m_kNetActorDict.end()==f)
	{
#ifdef _DEBUG
		TraceError("CNetworkActorManager::SyncActor(dwVID=%d) - NOT EXIST VID", dwVID);
#endif
		return;
	}

	SNetworkActorData& rkNetActorData=f->second;

	CInstanceBase* pkInstFind=__FindActor(rkNetActorData);
	if (pkInstFind)
	{
		pkInstFind->NEW_SyncPixelPosition(lPosX, lPosY);
	}

	rkNetActorData.SetPosition(lPosX, lPosY);
}

void CNetworkActorManager::SetActorOwner(DWORD dwOwnerVID, DWORD dwVictimVID)
{
	std::map<DWORD, SNetworkActorData>::iterator f=m_kNetActorDict.find(dwVictimVID);
	if (m_kNetActorDict.end()==f)
	{
#ifdef _DEBUG
		TraceError("CNetworkActorManager::SetActorOwner(dwOwnerVID=%d, dwVictimVID=%d) - NOT EXIST VID", dwOwnerVID, dwVictimVID);
#endif
		return;
	}

	SNetworkActorData& rkNetActorData=f->second;
	rkNetActorData.m_dwOwnerVID=dwOwnerVID;

	CInstanceBase* pkInstFind=__FindActor(rkNetActorData);
	if (pkInstFind)
	{
		pkInstFind->NEW_SetOwner(rkNetActorData.m_dwOwnerVID);
	}
}
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
