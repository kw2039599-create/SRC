#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "Packet.h"

#include "PythonGuild.h"
#include "PythonCharacterManager.h"
#include "PythonPlayer.h"
#include "PythonBackground.h"
#include "PythonMiniMap.h"
#include "PythonTextTail.h"
#include "PythonItem.h"
#include "PythonChat.h"
#include "PythonShop.h"
#include "PythonExchange.h"
#include "PythonQuest.h"
#include "PythonEventManager.h"
#include "PythonMessenger.h"
#include "PythonApplication.h"
#if defined(WJ_COMBAT_ZONE)
	#include "PythonCombatZone.h"
#endif
#if defined(ENABLE_MESSENGER_RENEWAL)
#include "PythonCommunity.h"
#endif
#include "../EterPack/EterPackManager.h"
#include "../gamelib/ItemManager.h"
#ifdef ENABLE_CSHIELD
#include "CShield.h"
#include "CShieldCheckIntegrity.h"
#endif
#include "AbstractApplication.h"
#include "AbstractCharacterManager.h"
#include "InstanceBase.h"
#ifdef ENABLE_GEM_SYSTEM
#include "PythonPlayer.h"
#endif
#ifdef ENABLE_CUBE_RENEWAL_WORLDARD
#include "PythonCubeRenewal.h"
#endif
#if defined(ENABLE_NPC_LOCATION_HELPER)
#	include "PythonNPCLocationHelper.h"
#endif
#include "ProcessCRC.h"
#include <string.h>

BOOL gs_bEmpireLanuageEnable = TRUE;

void CPythonNetworkStream::__RefreshAlignmentWindow()
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshAlignment", Py_BuildValue("()"));
}

void CPythonNetworkStream::__RefreshTargetBoardByVID(DWORD dwVID)
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshTargetBoardByVID", Py_BuildValue("(i)", dwVID));
}

void CPythonNetworkStream::__RefreshTargetBoardByName(const char * c_szName)
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshTargetBoardByName", Py_BuildValue("(s)", c_szName));
}

void CPythonNetworkStream::__RefreshTargetBoard()
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshTargetBoard", Py_BuildValue("()"));
}

void CPythonNetworkStream::__RefreshGuildWindowGradePage()
{
	m_isRefreshGuildWndGradePage=true;
}

void CPythonNetworkStream::__RefreshGuildWindowSkillPage()
{
	m_isRefreshGuildWndSkillPage=true;
}

void CPythonNetworkStream::__RefreshGuildWindowMemberPageGradeComboBox()
{
	m_isRefreshGuildWndMemberPageGradeComboBox=true;
}

void CPythonNetworkStream::__RefreshGuildWindowMemberPage()
{
	m_isRefreshGuildWndMemberPage=true;
}

void CPythonNetworkStream::__RefreshGuildWindowBoardPage()
{
	m_isRefreshGuildWndBoardPage=true;
}

void CPythonNetworkStream::__RefreshGuildWindowInfoPage()
{
	m_isRefreshGuildWndInfoPage=true;
}

void CPythonNetworkStream::__RefreshMessengerWindow()
{
	m_isRefreshMessengerWnd=true;
}

void CPythonNetworkStream::__RefreshSafeboxWindow()
{
	m_isRefreshSafeboxWnd=true;
}

void CPythonNetworkStream::__RefreshMallWindow()
{
	m_isRefreshMallWnd=true;
}

void CPythonNetworkStream::__RefreshSkillWindow()
{
	m_isRefreshSkillWnd=true;
}

void CPythonNetworkStream::__RefreshExchangeWindow()
{
	m_isRefreshExchangeWnd=true;
}

void CPythonNetworkStream::__RefreshStatus()
{
	m_isRefreshStatus=true;
}

void CPythonNetworkStream::__RefreshCharacterWindow()
{
	m_isRefreshCharacterWnd=true;
}

void CPythonNetworkStream::__RefreshInventoryWindow()
{
	m_isRefreshInventoryWnd=true;
}

#ifdef ENABLE_SHOP_SEARCH_SYSTEM
void CPythonNetworkStream::__RefreshShopSearchWindow()
{
	m_isRefreshShopSearchWnd = true;
}
#endif

void CPythonNetworkStream::__RefreshEquipmentWindow()
{
	m_isRefreshEquipmentWnd=true;
}

void CPythonNetworkStream::__SetGuildID(DWORD id)
{
	if (m_dwGuildID != id)
	{
		m_dwGuildID = id;
		IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();

		for (int i = 0; i < PLAYER_PER_ACCOUNT4; ++i)
			if (!strncmp(m_akSimplePlayerInfo[i].szName, rkPlayer.GetName(), CHARACTER_NAME_MAX_LEN))
			{
				m_adwGuildID[i] = id;

				std::string  guildName;
				if (CPythonGuild::Instance().GetGuildName(id, &guildName))
				{
					m_astrGuildName[i] = guildName;
				}
				else
				{
					m_astrGuildName[i] = "";
				}
			}
	}
}

#ifdef ENABLE_GUILD_TOKEN_AUTH
void CPythonNetworkStream::__SetGuildToken(uint64_t token)
{
	m_dwGuildToken = token;
}
#endif

struct PERF_PacketInfo
{
	DWORD dwCount;
	DWORD dwTime;

	PERF_PacketInfo()
	{
		dwCount=0;
		dwTime=0;
	}
};

#ifdef __PERFORMANCE_CHECK__

class PERF_PacketTimeAnalyzer
{
	public:
		~PERF_PacketTimeAnalyzer()
		{
			FILE* fp=fopen("perf_dispatch_packet_result.txt", "w");

			for (std::map<DWORD, PERF_PacketInfo>::iterator i=m_kMap_kPacketInfo.begin(); i!=m_kMap_kPacketInfo.end(); ++i)
			{
				if (i->second.dwTime>0)
					fprintf(fp, "header %d: count %d, time %d, tpc %d\n", i->first, i->second.dwCount, i->second.dwTime, i->second.dwTime/i->second.dwCount);
			}
			fclose(fp);
		}

	public:
		std::map<DWORD, PERF_PacketInfo> m_kMap_kPacketInfo;
};

PERF_PacketTimeAnalyzer gs_kPacketTimeAnalyzer;

#endif

// Game Phase ---------------------------------------------------------------------------
void CPythonNetworkStream::GamePhase()
{
#ifdef ENABLE_CSHIELD
	if (cshieldClientCode != 0)
	{
		__SendHack(cshieldClientCode, cshieldWin, cshieldLanguage, cshieldReport);

		cshieldClientCode = 0;
		cshieldWin = 0;
		cshieldLanguage = "";
		cshieldReport = "";
	}
#else
	if (!m_kQue_stHack.empty())
	{
		__SendHack(m_kQue_stHack.front().c_str());
		m_kQue_stHack.pop_front();
	}
#endif

	TPacketHeader header = 0;
	bool ret = true;

#ifdef __PERFORMANCE_CHECK__
	DWORD timeBeginDispatch=timeGetTime();

	static std::map<DWORD, PERF_PacketInfo> kMap_kPacketInfo;
	kMap_kPacketInfo.clear();
#endif

	const DWORD MAX_RECV_COUNT = 128;
	const DWORD SAFE_RECV_BUFSIZE = 262144; // 256 KB threshold (inside 4 MB recv buffer)
	DWORD dwRecvCount = 0;

	while (ret)
	{
		if (dwRecvCount++ >= MAX_RECV_COUNT - 1 && GetRecvBufferSize() < SAFE_RECV_BUFSIZE
			&& m_strPhase == "Game")
			break;

		if (!CheckPacket(&header))
			break;

#ifdef __PERFORMANCE_CHECK__
		DWORD timeBeginPacket=timeGetTime();
#endif

#if defined(_DEBUG) && defined(ENABLE_PRINT_RECV_PACKET_DEBUG)
		Tracenf("RECV HEADER : %u , phase %s ", header, m_strPhase.c_str());
#endif

		switch (header)
		{
			case HEADER_GC_OBSERVER_ADD:
				ret = RecvObserverAddPacket();
				break;
			case HEADER_GC_OBSERVER_REMOVE:
				ret = RecvObserverRemovePacket();
				break;
			case HEADER_GC_OBSERVER_MOVE:
				ret = RecvObserverMovePacket();
				break;
			case HEADER_GC_WARP:
				ret = RecvWarpPacket();
				return;
				break;

			case HEADER_GC_PHASE:
				ret = RecvPhasePacket();
#ifdef HIDE_CAPTCHA
				if (!CPythonApplication::Instance().IsWindowVisisble())
					CPythonApplication::Instance().SetWindowInvisible(false);
#endif
				return;
				break;

			case HEADER_GC_PVP:
				ret = RecvPVPPacket();
				break;

			case HEADER_GC_DUEL_START:
				ret = RecvDuelStartPacket();
				break;

			case HEADER_GC_CHARACTER_ADD:
 				ret = RecvCharacterAppendPacket();
				break;

			case HEADER_GC_CHAR_ADDITIONAL_INFO:
				ret = RecvCharacterAdditionalInfo();
				break;

			case HEADER_GC_CHARACTER_ADD2:
				ret = RecvCharacterAppendPacketNew();
				break;

			case HEADER_GC_CHARACTER_UPDATE:
				ret = RecvCharacterUpdatePacket();
				break;

			case HEADER_GC_CHARACTER_DEL:
				ret = RecvCharacterDeletePacket();
				break;

			case HEADER_GC_CHAT:
				ret = RecvChatPacket();
				break;

			case HEADER_GC_SYNC_POSITION:
				ret = RecvSyncPositionPacket();
				break;

			case HEADER_GC_OWNERSHIP:
				ret = RecvOwnerShipPacket();
				break;

			case HEADER_GC_WHISPER:
				ret = RecvWhisperPacket();
				break;

			case HEADER_GC_CHARACTER_MOVE:
				ret = RecvCharacterMovePacket();
				break;

			// Position
			case HEADER_GC_CHARACTER_POSITION:
				ret = RecvCharacterPositionPacket();
				break;

			// Battle Packet
			case HEADER_GC_STUN:
				ret = RecvStunPacket();
				break;

			case HEADER_GC_DEAD:
				ret = RecvDeadPacket();
				break;

#if defined(__BL_OFFICIAL_LOOT_FILTER__)
			case HEADER_GC_LOOT_FILTER:
				ret = RecvLootFilterPacket();
				break;
#endif

			case HEADER_GC_PLAYER_POINT_CHANGE:
				ret = RecvPointChange();
				break;

			// item packet.
			case HEADER_GC_ITEM_SET:
				ret = RecvItemSetPacket();
				break;

			case HEADER_GC_ITEM_SET2:
				ret = RecvItemSetPacket2();
				break;

			case HEADER_GC_ITEM_USE:
				ret = RecvItemUsePacket();
				break;

			case HEADER_GC_ITEM_UPDATE:
				ret = RecvItemUpdatePacket();
				break;

			case HEADER_GC_ITEM_GROUND_ADD:
				ret = RecvItemGroundAddPacket();
				break;

			case HEADER_GC_ITEM_GROUND_DEL:
				ret = RecvItemGroundDelPacket();
				break;

			case HEADER_GC_ITEM_OWNERSHIP:
				ret = RecvItemOwnership();
				break;

			case HEADER_GC_QUICKSLOT_ADD:
				ret = RecvQuickSlotAddPacket();
				break;

			case HEADER_GC_QUICKSLOT_DEL:
				ret = RecvQuickSlotDelPacket();
				break;

			case HEADER_GC_QUICKSLOT_SWAP:
				ret = RecvQuickSlotMovePacket();
				break;

			case HEADER_GC_MOTION:
				ret = RecvMotionPacket();
				break;
#if defined(__BL_67_ATTR__)
			case HEADER_GC_OPEN_67_ATTR:
				ret = Recv67AttrOpenPacket();
				break;
#endif
#if defined(__BL_TRANSMUTATION__)
			case HEADER_GC_CHANGE_LOOK_SET:
				ret = RecvChangeLookItemSetPacket();
				break;

			case HEADER_GC_CHANGE_LOOK_DEL:
				ret = RecvChangeLookItemDelPacket();
				break;

			case HEADER_GC_CHANGE_LOOK_FREE_SET:
				ret = RecvChangeLookFreeItemSetPacket();
				break;

			case HEADER_GC_CHANGE_LOOK_FREE_DEL:
				ret = RecvChangeLookFreeItemDelPacket();
				break;
#endif

#if defined(__BL_MAILBOX__)
			case HEADER_GC_MAILBOX_PROCESS:
				ret = RecvMailboxProcess();
				break;

			case HEADER_GC_MAILBOX:
				ret = RecvMailbox();
				break;

			case HEADER_GC_MAILBOX_ADD_DATA:
				ret = RecvMailboxAddData();
				break;

			case HEADER_GC_MAILBOX_ALL:
				ret = RecvMailboxAll();
				break;

			case HEADER_GC_MAILBOX_UNREAD:
				ret = RecvMailboxUnread();
				break;
#endif

			case HEADER_GC_SHOP:
				ret = RecvShopPacket();
				break;

#if defined(__BL_KILL_BAR__)
			case HEADER_GC_KILLBAR:
				ret = RecvKillBar();
				break;
#endif

			case HEADER_GC_SHOP_SIGN:
				ret = RecvShopSignPacket();
				break;

			case HEADER_GC_EXCHANGE:
				ret = RecvExchangePacket();
				break;

#if defined(ENABLE_PARTY_MATCH)
			case HEADER_GC_PARTY_MATCH:
				ret = RecvPartyMatch();
				break;
#endif

			case HEADER_GC_QUEST_INFO:
				ret = RecvQuestInfoPacket();
				break;

			case HEADER_GC_REQUEST_MAKE_GUILD:
				ret = RecvRequestMakeGuild();
				break;

			case HEADER_GC_PING:
				ret = RecvPingPacket();
				break;

			case HEADER_GC_SCRIPT:
				ret = RecvScriptPacket();
				break;

			case HEADER_GC_QUEST_CONFIRM:
				ret = RecvQuestConfirmPacket();
				break;

			case HEADER_GC_TARGET:
				ret = RecvTargetPacket();
				break;

			case HEADER_GC_DAMAGE_INFO:
				ret = RecvDamageInfoPacket();
				break;
#if defined(WJ_COMBAT_ZONE)
			case HEADER_GC_RANKING_COMBAT_ZONE:
				ret = __RecvCombatZoneRankingPacket();
				break;
				
			case HEADER_GC_SEND_COMBAT_ZONE:
				ret = __RecvCombatZonePacket();
				break;		
#endif

			case HEADER_GC_MOUNT:
				ret = RecvMountPacket();
				break;

			case HEADER_GC_CHANGE_SPEED:
				ret = RecvChangeSpeedPacket();
				break;

			case HEADER_GC_PLAYER_POINTS:
				ret = __RecvPlayerPoints();
				break;

			case HEADER_GC_CREATE_FLY:
				ret = RecvCreateFlyPacket();
				break;

			case HEADER_GC_FLY_TARGETING:
				ret = RecvFlyTargetingPacket();
				break;

			case HEADER_GC_ADD_FLY_TARGETING:
				ret = RecvAddFlyTargetingPacket();
				break;

			case HEADER_GC_SKILL_LEVEL:
				ret = RecvSkillLevel();
				break;

			case HEADER_GC_SKILL_LEVEL_NEW:
				ret = RecvSkillLevelNew();
				break;

			case HEADER_GC_MESSENGER:
				ret = RecvMessenger();
				break;

			case HEADER_GC_GUILD:
				ret = RecvGuild();
				break;

			case HEADER_GC_PARTY_INVITE:
				ret = RecvPartyInvite();
				break;

			case HEADER_GC_PARTY_ADD:
				ret = RecvPartyAdd();
				break;

			case HEADER_GC_PARTY_UPDATE:
				ret = RecvPartyUpdate();
				break;

			case HEADER_GC_PARTY_REMOVE:
				ret = RecvPartyRemove();
				break;

			case HEADER_GC_PARTY_LINK:
				ret = RecvPartyLink();
				break;

			case HEADER_GC_PARTY_UNLINK:
				ret = RecvPartyUnlink();
				break;

			case HEADER_GC_PARTY_PARAMETER:
				ret = RecvPartyParameter();
				break;

			case HEADER_GC_SAFEBOX_SET:
				ret = RecvSafeBoxSetPacket();
				break;

			case HEADER_GC_SAFEBOX_DEL:
				ret = RecvSafeBoxDelPacket();
				break;

			case HEADER_GC_SAFEBOX_WRONG_PASSWORD:
				ret = RecvSafeBoxWrongPasswordPacket();
				break;

			case HEADER_GC_SAFEBOX_SIZE:
				ret = RecvSafeBoxSizePacket();
				break;

			case HEADER_GC_SAFEBOX_MONEY_CHANGE:
				ret = RecvSafeBoxMoneyChangePacket();
				break;

			case HEADER_GC_FISHING:
				ret = RecvFishing();
				break;

			case HEADER_GC_DUNGEON:
				ret = RecvDungeon();
				break;

			case HEADER_GC_TIME:
				ret = RecvTimePacket();
				break;

			case HEADER_GC_WALK_MODE:
				ret = RecvWalkModePacket();
				break;

			case HEADER_GC_CHANGE_SKILL_GROUP:
				ret = RecvChangeSkillGroupPacket();
				break;

			case HEADER_GC_REFINE_INFORMATION:
				ret = RecvRefineInformationPacket();
				break;

			case HEADER_GC_REFINE_INFORMATION_NEW:
				ret = RecvRefineInformationPacketNew();
				break;

			case HEADER_GC_SEPCIAL_EFFECT:
				ret = RecvSpecialEffect();
				break;

			case HEADER_GC_NPC_POSITION:
				ret = RecvNPCList();
				break;

			case HEADER_GC_CHANNEL:
				ret = RecvChannelPacket();
				break;

			#ifdef ENABLE_GUILD_TOKEN_AUTH
			case HEADER_GC_GUILD_TOKEN:
				ret = RecvGuildTokenPacket();
				break;
			#endif

			case HEADER_GC_VIEW_EQUIP:
				ret = RecvViewEquipPacket();
				break;

			case HEADER_GC_LAND_LIST:
				ret = RecvLandPacket();
				break;

			//case HEADER_GC_TARGET_CREATE:
			//	ret = RecvTargetCreatePacket();
			//	break;

			case HEADER_GC_TARGET_CREATE_NEW:
				ret = RecvTargetCreatePacketNew();
				break;

			case HEADER_GC_TARGET_UPDATE:
				ret = RecvTargetUpdatePacket();
				break;

			case HEADER_GC_TARGET_DELETE:
				ret = RecvTargetDeletePacket();
				break;

			case HEADER_GC_AFFECT_ADD:
				ret = RecvAffectAddPacket();
				break;

			case HEADER_GC_AFFECT_REMOVE:
				ret = RecvAffectRemovePacket();
				break;

			case HEADER_GC_MALL_OPEN:
				ret = RecvMallOpenPacket();
				break;

			case HEADER_GC_MALL_SET:
				ret = RecvMallItemSetPacket();
				break;

			case HEADER_GC_MALL_DEL:
				ret = RecvMallItemDelPacket();
				break;

#ifdef ENABLE_CONQUEROR_LEVEL
			case HEADER_GC_SUNGMA_ATTR_UPDATE:
				ret = RecvSungmaAttrUpdatePacket();
				break;
#endif

			case HEADER_GC_LOVER_INFO:
				ret = RecvLoverInfoPacket();
				break;

			case HEADER_GC_LOVE_POINT_UPDATE:
				ret = RecvLovePointUpdatePacket();
				break;

			case HEADER_GC_DIG_MOTION:
				ret = RecvDigMotionPacket();
				break;

			case HEADER_GC_HANDSHAKE:
				RecvHandshakePacket();
				break;

			case HEADER_GC_HANDSHAKE_OK:
				RecvHandshakeOKPacket();
				break;

#ifdef ENABLE_ITEMSHOP
			case HEADER_GC_ITEMSHOP:
				ret = RecvItemShop();
				break;
#endif

			case HEADER_GC_HYBRIDCRYPT_KEYS:
				RecvHybridCryptKeyPacket();
				return;
				break;

			case HEADER_GC_HYBRIDCRYPT_SDB:
				RecvHybridCryptSDBPacket();
				return;
				break;

#ifdef _IMPROVED_PACKET_ENCRYPTION_
			case HEADER_GC_KEY_AGREEMENT:
				RecvKeyAgreementPacket();
				return;
				break;

			case HEADER_GC_KEY_AGREEMENT_COMPLETED:
				RecvKeyAgreementCompletedPacket();
				return;
				break;
#endif

			case HEADER_GC_SPECIFIC_EFFECT:
				ret = RecvSpecificEffect();
				break;

			case HEADER_GC_DRAGON_SOUL_REFINE:
				ret = RecvDragonSoulRefine();
				break;
#ifdef ENABLE_CUBE_RENEWAL_WORLDARD
			case HEADER_GC_CUBE_RENEWAL:
				ret = RecvCubeRenewalPacket();
				break;
#endif
#ifdef BOT_KILLER
			case HEADER_GC_BOT_KILLER:
				ret = RecvBotKillerPacket();
				break;
#endif
#ifdef ENABLE_GEM_SYSTEM
			case HEADER_GC_GEM_SHOP_OPEN:
				ret = RecvGemShopOpen();
				break;
				
			case HEADER_GC_GEM_SHOP_REFRESH:
				ret = RecvGemShopRefresh();
				break;
#endif
#ifdef ELEMENT_SPELL_WORLDARD
			case HEADER_GC_ELEMENTS_SPELL:
				ret = RecvElementsSpellPacket();
				break;	
#endif

#ifdef ENABLE_FISH_EVENT
			case HEADER_GC_FISH_EVENT_INFO:
				ret = RecvFishEventInfo();
				break;
#endif

#ifdef ENABLE_SHOP_SEARCH_SYSTEM
			case HEADER_GC_SHOPSEARCH_SET:
				ret = RecvShopSearchSet();
				break;
#endif

#ifdef ENABLE_DS_SET
			case HEADER_GC_DS_TABLE:
				ret = RecvDSTablePacket();
				break;
#endif

#ifdef ENABLE_OFFLINESHOP_SYSTEM
			case HEADER_GC_OFFLINE_SHOP:
				ret = RecvOfflineShopPacket();
				break;

			case HEADER_GC_OFFLINE_SHOP_SIGN:
				ret = RecvOfflineShopSignPacket();
				break;
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
			case HEADER_GC_ACCE:
				ret = RecvAccePacket();
				break;
#endif
#ifdef ENABLE_IKASHOP_RENEWAL
			case HEADER_GC_NEW_OFFLINESHOP:
				ret = RecvOfflineshopPacket();
				break;
#endif
#if defined(ENABLE_VOICE_CHAT)
			case HEADER_GC_VOICE_CHAT:
				ret = RecvVoiceChatPacket();
				break;
#endif
#ifdef ENABLE_MINI_GAME_CATCH_KING
			case HEADER_GC_MINI_GAME_CATCH_KING:
				ret = RecvMiniGameCatchKingPacket();
				break;
#endif
#ifdef ENABLE_MINI_GAME_YUTNORI
			case HEADER_GC_MINI_GAME_YUT_NORI:
				ret = RecvMiniGameYutNoriPacket();
				break;
#endif
#ifdef ENABLE_TAROT
			case HEADER_GC_MINIGAME_BNW:
				ret = RecvMinigameBnw();
				break;
#endif
#ifdef ENABLE_CSHIELD
			case HEADER_GC_CSHIELD:
				ret = RecvCShieldPacket();
				break;

			case HEADER_GC_CSHIELD_STOP_PLAYER:
				ret = RecvCShieldStopPlayerPacket();
				break;
#endif
#ifdef ENABLE_CAPTCHA
			case HEADER_GC_CAPTCHA:
				ret = RecvCaptchaPacket();
				break;
#endif
#ifdef ENABLE_EVENT_MANAGER
			case HEADER_GC_EVENT_MANAGER:
				ret = RecvEventManager();
				break;
#endif
#ifdef ENABLE_12ZI
			case HEADER_GC_SEPCIAL_ZODIAC_EFFECT:
				ret = RecvSpecialZodiacEffect();
				break;
#endif
#ifdef ENABLE_FISHING_TIMER
			case HEADER_GC_FISHING_TIMER:
				ret = RecvFishingTimer();
				break;
#endif
#if defined(ENABLE_NPC_LOCATION_HELPER)
			case HEADER_GC_NPC_LOCATION_HELPER:
				ret = RecvNPCLocationHelperPacket();
				break;
#endif
#ifdef ENABLE_SWITCHBOT
			case HEADER_GC_SWITCHBOT:
				ret = RecvSwitchbotPacket();
				break;
#endif
#ifdef ENABLE_AURA_SYSTEM
			case HEADER_GC_AURA:
				ret = RecvAuraPacket();
				break;
#endif
			default:
				ret = RecvDefaultPacket(header);
				break;

		}
#ifdef __PERFORMANCE_CHECK__
		DWORD timeEndPacket=timeGetTime();

		{
			PERF_PacketInfo& rkPacketInfo=kMap_kPacketInfo[header];
			rkPacketInfo.dwCount++;
			rkPacketInfo.dwTime+=timeEndPacket-timeBeginPacket;
		}

		{
			PERF_PacketInfo& rkPacketInfo=gs_kPacketTimeAnalyzer.m_kMap_kPacketInfo[header];
			rkPacketInfo.dwCount++;
			rkPacketInfo.dwTime+=timeEndPacket-timeBeginPacket;
		}
#endif
	}

#ifdef __PERFORMANCE_CHECK__
	DWORD timeEndDispatch=timeGetTime();

	if (timeEndDispatch-timeBeginDispatch>2)
	{
		static FILE* fp=fopen("perf_dispatch_packet.txt", "w");

		fprintf(fp, "delay %d\n", timeEndDispatch-timeBeginDispatch);
		for (std::map<DWORD, PERF_PacketInfo>::iterator i=kMap_kPacketInfo.begin(); i!=kMap_kPacketInfo.end(); ++i)
		{
			if (i->second.dwTime>0)
				fprintf(fp, "header %d: count %d, time %d\n", i->first, i->second.dwCount, i->second.dwTime);
		}
		fputs("=====================================================\n", fp);
		fflush(fp);
	}
#endif

	if (!ret)
		RecvErrorPacket(header);

	static DWORD s_nextRefreshTime = ELTimer_GetMSec();

	DWORD curTime = ELTimer_GetMSec();
	if (s_nextRefreshTime > curTime)
		return;

	if (m_isRefreshCharacterWnd)
	{
		m_isRefreshCharacterWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshCharacter", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshEquipmentWnd)
	{
		m_isRefreshEquipmentWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshEquipment", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshInventoryWnd)
	{
		m_isRefreshInventoryWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshInventory", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshExchangeWnd)
	{
		m_isRefreshExchangeWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshExchange", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshSkillWnd)
	{
		m_isRefreshSkillWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSkill", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshSafeboxWnd)
	{
		m_isRefreshSafeboxWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSafebox", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshMallWnd)
	{
		m_isRefreshMallWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshMall", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshStatus)
	{
		m_isRefreshStatus=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshStatus", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshMessengerWnd)
	{
		m_isRefreshMessengerWnd=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshMessenger", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshGuildWndInfoPage)
	{
		m_isRefreshGuildWndInfoPage=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildInfoPage", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshGuildWndBoardPage)
	{
		m_isRefreshGuildWndBoardPage=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildBoardPage", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshGuildWndMemberPage)
	{
		m_isRefreshGuildWndMemberPage=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildMemberPage", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshGuildWndMemberPageGradeComboBox)
	{
		m_isRefreshGuildWndMemberPageGradeComboBox=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildMemberPageGradeComboBox", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshGuildWndSkillPage)
	{
		m_isRefreshGuildWndSkillPage=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildSkillPage", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}

	if (m_isRefreshGuildWndGradePage)
	{
		m_isRefreshGuildWndGradePage=false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildGradePage", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 300;
	}
#ifdef ENABLE_SHOP_SEARCH_SYSTEM
	if (m_isRefreshShopSearchWnd)
	{
		m_isRefreshShopSearchWnd = false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshShopSearch", Py_BuildValue("()"));
		s_nextRefreshTime = curTime + 500;
	}
#endif
}

void CPythonNetworkStream::__InitializeGamePhase()
{
	__ServerTimeSync_Initialize();

	m_isRefreshStatus=false;
	m_isRefreshCharacterWnd=false;
	m_isRefreshEquipmentWnd=false;
	m_isRefreshInventoryWnd=false;
	m_isRefreshExchangeWnd=false;
	m_isRefreshSkillWnd=false;
	m_isRefreshSafeboxWnd=false;
	m_isRefreshMallWnd=false;
	m_isRefreshMessengerWnd=false;
	m_isRefreshGuildWndInfoPage=false;
	m_isRefreshGuildWndBoardPage=false;
	m_isRefreshGuildWndMemberPage=false;
	m_isRefreshGuildWndMemberPageGradeComboBox=false;
	m_isRefreshGuildWndSkillPage=false;
	m_isRefreshGuildWndGradePage=false;

	m_EmoticonStringVector.clear();

	m_pInstTarget = NULL;
}

void CPythonNetworkStream::Warp(LONG lGlobalX, LONG lGlobalY)
{
	CPythonBackground& rkBgMgr=CPythonBackground::Instance();
	rkBgMgr.Destroy();
	rkBgMgr.Create();
	rkBgMgr.Warp(lGlobalX, lGlobalY);
	//rkBgMgr.SetShadowLevel(CPythonBackground::SHADOW_ALL);
	rkBgMgr.RefreshShadowLevel();

	LONG lLocalX = lGlobalX;
	LONG lLocalY = lGlobalY;
	__GlobalPositionToLocalPosition(lLocalX, lLocalY);
	float fHeight = CPythonBackground::Instance().GetHeight(float(lLocalX), float(lLocalY));

	IAbstractApplication& rkApp=IAbstractApplication::GetSingleton();
	rkApp.SetCenterPosition(float(lLocalX), float(lLocalY), fHeight);

	__ShowMapName(lLocalX, lLocalY);
}

void CPythonNetworkStream::__ShowMapName(LONG lLocalX, LONG lLocalY)
{
	const std::string & c_rstrMapFileName = CPythonBackground::Instance().GetWarpMapName();
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ShowMapName", Py_BuildValue("(sii)", c_rstrMapFileName.c_str(), lLocalX, lLocalY));
}

void CPythonNetworkStream::__LeaveGamePhase()
{
	CInstanceBase::ClearPVPKeySystem();

	__ClearNetworkActorManager();

	m_bComboSkillFlag = FALSE;
#if defined(ENABLE_VOICE_CHAT)
	VoiceChat::Instance().Destroy();
#endif

	IAbstractCharacterManager& rkChrMgr=IAbstractCharacterManager::GetSingleton();
	rkChrMgr.Destroy();

	CPythonItem& rkItemMgr=CPythonItem::Instance();
	rkItemMgr.Destroy();
#ifdef ENABLE_IKASHOP_RENEWAL
#ifdef ENABLE_IKASHOP_ENTITIES
	CPythonIkarusShop::instance().DeleteEntities();
#endif
#endif
}

void CPythonNetworkStream::SetGamePhase()
{
	if ("Game"!=m_strPhase)
		m_phaseLeaveFunc.Run();

	Tracen("");
	Tracen("## Network - Game Phase ##");
	Tracen("");


	m_strPhase = "Game";

	m_dwChangingPhaseTime = ELTimer_GetMSec();
	m_phaseProcessFunc.Set(this, &CPythonNetworkStream::GamePhase);
	m_phaseLeaveFunc.Set(this, &CPythonNetworkStream::__LeaveGamePhase);

#if defined(ENABLE_VOICE_CHAT)
	VoiceChat::Instance().Initialize();
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ReloadVoiceChatVolumes", Py_BuildValue("()"));
#endif

	IAbstractPlayer & rkPlayer = IAbstractPlayer::GetSingleton();
	rkPlayer.SetMainCharacterIndex(GetMainActorVID());

	__RefreshStatus();
}

bool CPythonNetworkStream::RecvObserverAddPacket()
{
	TPacketGCObserverAdd kObserverAddPacket;
	if (!Recv(sizeof(kObserverAddPacket), &kObserverAddPacket))
		return false;

	CPythonMiniMap::Instance().AddObserver(
		kObserverAddPacket.vid,
		kObserverAddPacket.x*100.0f,
		kObserverAddPacket.y*100.0f);

	return true;
}

bool CPythonNetworkStream::RecvObserverRemovePacket()
{
	TPacketGCObserverAdd kObserverRemovePacket;
	if (!Recv(sizeof(kObserverRemovePacket), &kObserverRemovePacket))
		return false;

	CPythonMiniMap::Instance().RemoveObserver(
		kObserverRemovePacket.vid
	);

	return true;
}

bool CPythonNetworkStream::RecvObserverMovePacket()
{
	TPacketGCObserverMove kObserverMovePacket;
	if (!Recv(sizeof(kObserverMovePacket), &kObserverMovePacket))
		return false;

	CPythonMiniMap::Instance().MoveObserver(
		kObserverMovePacket.vid,
		kObserverMovePacket.x*100.0f,
		kObserverMovePacket.y*100.0f);

	return true;
}

bool CPythonNetworkStream::RecvWarpPacket()
{
	TPacketGCWarp kWarpPacket;

	if (!Recv(sizeof(kWarpPacket), &kWarpPacket))
		return false;

	__DirectEnterMode_Set(m_dwSelectedCharacterIndex);

	CNetworkStream::Connect((DWORD)kWarpPacket.lAddr, kWarpPacket.wPort);

	return true;
}

bool CPythonNetworkStream::RecvDuelStartPacket()
{
	TPacketGCDuelStart kDuelStartPacket;
	if (!Recv(sizeof(kDuelStartPacket), &kDuelStartPacket))
		return false;

	DWORD count = (kDuelStartPacket.wSize - sizeof(kDuelStartPacket))/sizeof(DWORD);

	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();

	CInstanceBase* pkInstMain=rkChrMgr.GetMainInstancePtr();
	if (!pkInstMain)
	{
		TraceError("CPythonNetworkStream::RecvDuelStartPacket - MainCharacter is NULL");
		return false;
	}
	DWORD dwVIDSrc = pkInstMain->GetVirtualID();
	DWORD dwVIDDest;

	for ( DWORD i = 0; i < count; i++)
	{
		Recv(sizeof(dwVIDDest),&dwVIDDest);
		CInstanceBase::InsertDUELKey(dwVIDSrc,dwVIDDest);
	}

	if(count == 0)
		pkInstMain->SetDuelMode(CInstanceBase::DUEL_CANNOTATTACK);
	else
		pkInstMain->SetDuelMode(CInstanceBase::DUEL_START);

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoard", Py_BuildValue("()"));

	rkChrMgr.RefreshAllPCTextTail();

	return true;
}

bool CPythonNetworkStream::RecvPVPPacket()
{
	TPacketGCPVP kPVPPacket;
	if (!Recv(sizeof(kPVPPacket), &kPVPPacket))
		return false;

	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();
	CPythonPlayer & rkPlayer = CPythonPlayer::Instance();

	switch (kPVPPacket.bMode)
	{
		case PVP_MODE_AGREE:
			rkChrMgr.RemovePVPKey(kPVPPacket.dwVIDSrc, kPVPPacket.dwVIDDst);

			if (rkPlayer.IsMainCharacterIndex(kPVPPacket.dwVIDDst))
				rkPlayer.RememberChallengeInstance(kPVPPacket.dwVIDSrc);

			if (rkPlayer.IsMainCharacterIndex(kPVPPacket.dwVIDSrc))
				rkPlayer.RememberCantFightInstance(kPVPPacket.dwVIDDst);
			break;
		case PVP_MODE_REVENGE:
		{
			rkChrMgr.RemovePVPKey(kPVPPacket.dwVIDSrc, kPVPPacket.dwVIDDst);

			DWORD dwKiller = kPVPPacket.dwVIDSrc;
			DWORD dwVictim = kPVPPacket.dwVIDDst;

			if (rkPlayer.IsMainCharacterIndex(dwVictim))
				rkPlayer.RememberRevengeInstance(dwKiller);

			if (rkPlayer.IsMainCharacterIndex(dwKiller))
				rkPlayer.RememberCantFightInstance(dwVictim);
			break;
		}

		case PVP_MODE_FIGHT:
			rkChrMgr.InsertPVPKey(kPVPPacket.dwVIDSrc, kPVPPacket.dwVIDDst);
			rkPlayer.ForgetInstance(kPVPPacket.dwVIDSrc);
			rkPlayer.ForgetInstance(kPVPPacket.dwVIDDst);
			break;
		case PVP_MODE_NONE:
			rkChrMgr.RemovePVPKey(kPVPPacket.dwVIDSrc, kPVPPacket.dwVIDDst);
			rkPlayer.ForgetInstance(kPVPPacket.dwVIDSrc);
			rkPlayer.ForgetInstance(kPVPPacket.dwVIDDst);
			break;
	}

	__RefreshTargetBoardByVID(kPVPPacket.dwVIDSrc);
	__RefreshTargetBoardByVID(kPVPPacket.dwVIDDst);

	return true;
}

// DELETEME
/*
void CPythonNetworkStream::__SendWarpPacket()
{
	TPacketCGWarp kWarpPacket;
	kWarpPacket.bHeader=HEADER_GC_WARP;
	if (!Send(sizeof(kWarpPacket), &kWarpPacket))
	{
		return;
	}
}
*/
#ifdef ENABLE_CSHIELD
bool CPythonNetworkStream::__SendHack(const DWORD errorcode, const BYTE win, std::string language, std::string report)
{
	TPacketCGHack kPacketHack;
	kPacketHack.bHeader = HEADER_CG_HACK;
	kPacketHack.errorcode = errorcode;
	kPacketHack.win = win;
	strncpy(kPacketHack.language, language.c_str(), sizeof(kPacketHack.language) - 1);
	kPacketHack.language[3] = '\0';
	strncpy(kPacketHack.report, report.c_str(), sizeof(kPacketHack.report) - 1);
	kPacketHack.report[1024] = '\0';

	if (!Send(sizeof(kPacketHack), &kPacketHack))
		return false;

	return SendSequence();
}
#else
void CPythonNetworkStream::NotifyHack(const char* c_szMsg)
{
	if (!m_kQue_stHack.empty())
		if (c_szMsg==m_kQue_stHack.back())
			return;

	m_kQue_stHack.push_back(c_szMsg);	
}

bool CPythonNetworkStream::__SendHack(const char* c_szMsg)
{
	Tracen(c_szMsg);
	
	TPacketCGHack kPacketHack;
	kPacketHack.bHeader=HEADER_CG_HACK;
	strncpy(kPacketHack.szBuf, c_szMsg, sizeof(kPacketHack.szBuf)-1);

	if (!Send(sizeof(kPacketHack), &kPacketHack))
		return false;

	return SendSequence();
}
#endif

bool CPythonNetworkStream::SendMessengerAddByVIDPacket(DWORD vid)
{
	TPacketCGMessenger packet;
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_ADD_BY_VID;
	if (!Send(sizeof(packet), &packet))
		return false;
	if (!Send(sizeof(vid), &vid))
		return false;
	return SendSequence();
}

#ifdef NEW_PET_SYSTEM
bool CPythonNetworkStream::PetSetNamePacket(const char * petname)
{
	TPacketCGRequestPetName PetSetName;
	PetSetName.byHeader = HEADER_CG_PetSetName;
	strncpy(PetSetName.petname, petname, 12);
	PetSetName.petname[12] = '\0';

	if (!Send(sizeof(PetSetName), &PetSetName))
		return false;

	return SendSequence();
}
#endif

bool CPythonNetworkStream::SendMessengerAddByNamePacket(const char * c_szName)
{
	TPacketCGMessenger packet;
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_ADD_BY_NAME;
	if (!Send(sizeof(packet), &packet))
		return false;
	char szName[CHARACTER_NAME_MAX_LEN];
	strncpy(szName, c_szName, CHARACTER_NAME_MAX_LEN-1);
	szName[CHARACTER_NAME_MAX_LEN-1] = '\0';

	if (!Send(sizeof(szName), &szName))
		return false;
	Tracef(" SendMessengerAddByNamePacket : %s\n", c_szName);
	return SendSequence();
}

bool CPythonNetworkStream::SendMessengerRemovePacket(const char * c_szKey, const char * c_szName)
{
	TPacketCGMessenger packet;
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_REMOVE;
	if (!Send(sizeof(packet), &packet))
		return false;
	char szKey[CHARACTER_NAME_MAX_LEN];
	strncpy(szKey, c_szKey, CHARACTER_NAME_MAX_LEN-1);
	if (!Send(sizeof(szKey), &szKey))
		return false;
	__RefreshTargetBoardByName(c_szName);
	return SendSequence();
}

#if defined(ENABLE_MESSENGER_RENEWAL)
void CPythonNetworkStream::RequestMessengerRefresh()
{
	__RefreshMessengerWindow();
}

bool CPythonNetworkStream::SendMessengerSetConnectionStatePacket(BYTE bConnectionState)
{
	if (bConnectionState > MESSENGER_CONNECTION_STATE_DISCONNECT)
		return false;

	TPacketCGMessenger Packet;
	Packet.header = HEADER_CG_MESSENGER;
	Packet.subheader = MESSENGER_SUBHEADER_CG_SET_CONNECTION_STATE;
	if (!Send(sizeof(Packet), &Packet))
		return false;

	TPacketCGMessengerSetConnectionState StatePacket;
	StatePacket.bConnectionState = bConnectionState;
	if (!Send(sizeof(StatePacket), &StatePacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendMessengerSetStatusMessagePacket(const char* c_szMessage)
{
	if (!c_szMessage || !*c_szMessage)
		return false;

	size_t len = strlen(c_szMessage);
	if (len == 0 || len > MESSENGER_STATUS_MESSAGE_MAX_LEN)
		return false;

	TPacketCGMessenger Packet;
	Packet.header = HEADER_CG_MESSENGER;
	Packet.subheader = MESSENGER_SUBHEADER_CG_SET_STATUS_MESSAGE;
	if (!Send(sizeof(Packet), &Packet))
		return false;

	TPacketCGMessengerSetStatusMessage StatusPacket;
	memset(&StatusPacket, 0, sizeof(StatusPacket));
	strncpy(StatusPacket.szStatusMessage, c_szMessage, MESSENGER_STATUS_MESSAGE_MAX_LEN);
	StatusPacket.szStatusMessage[MESSENGER_STATUS_MESSAGE_MAX_LEN] = '\0';
	if (!Send(sizeof(StatusPacket), &StatusPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendMessengerDeleteStatusMessagePacket()
{
	TPacketCGMessenger Packet;
	Packet.header = HEADER_CG_MESSENGER;
	Packet.subheader = MESSENGER_SUBHEADER_CG_DELETE_STATUS_MESSAGE;
	if (!Send(sizeof(Packet), &Packet))
		return false;

	return SendSequence();
}
#endif

#ifdef ENABLE_MESSENGER_BLOCK
bool CPythonNetworkStream::SendMessengerAddBlockByVIDPacket(DWORD vid)
{
	TPacketCGMessenger packet;
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_ADD_BLOCK_BY_VID;
	if (!Send(sizeof(packet), &packet))
		return false;
	if (!Send(sizeof(vid), &vid))
		return false;
	return SendSequence();
}

bool CPythonNetworkStream::SendMessengerAddBlockByNamePacket(const char * c_szName)
{
	TPacketCGMessenger packet;
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_ADD_BLOCK_BY_NAME;
	if (!Send(sizeof(packet), &packet))
		return false;
	char szName[CHARACTER_NAME_MAX_LEN];
	strncpy(szName, c_szName, CHARACTER_NAME_MAX_LEN-1);
	szName[CHARACTER_NAME_MAX_LEN-1] = '\0'; // #720: ??? ?? ?? ?? ????? ?? ??

	if (!Send(sizeof(szName), &szName))
		return false;
	Tracef(" SendMessengerAddBlockByNamePacket : %s\n", c_szName);
	return SendSequence();
}

bool CPythonNetworkStream::SendMessengerRemoveBlockPacket(const char * c_szKey, const char * c_szName)
{
	TPacketCGMessenger packet;
	packet.header = HEADER_CG_MESSENGER;
	packet.subheader = MESSENGER_SUBHEADER_CG_REMOVE_BLOCK;
	if (!Send(sizeof(packet), &packet))
		return false;
	char szKey[CHARACTER_NAME_MAX_LEN];
	strncpy(szKey, c_szKey, CHARACTER_NAME_MAX_LEN-1);
	if (!Send(sizeof(szKey), &szKey))
		return false;
	__RefreshTargetBoardByName(c_szName);
	return SendSequence();
}
#endif

bool CPythonNetworkStream::SendCharacterStatePacket(const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg)
{
	if (!__CanActMainInstance())
		return true;

#if defined(ENABLE_CSHIELD) && !defined(__AUTO_HUNT__)
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	CInstanceBase* pkInstMain = rkChrMgr.GetMainInstancePtr();
	if (pkInstMain)
	{
		if (!pkInstMain->IsGoing() && pkInstMain->__CanSkipCollision())
		{
			cshieldClientCode = 45;
			CShieldClose();
			return true;
		}
	}
#endif

	if (fDstRot < 0.0f)
		fDstRot = 360 + fDstRot;
	else if (fDstRot > 360.0f)
		fDstRot = fmodf(fDstRot, 360.0f);

	TPacketCGMove kStatePacket;
	kStatePacket.bHeader = HEADER_CG_CHARACTER_MOVE;
	kStatePacket.bFunc = eFunc;
	kStatePacket.bArg = uArg;
	kStatePacket.bRot = fDstRot/5.0f;
	kStatePacket.lX = long(c_rkPPosDst.x);
	kStatePacket.lY = long(c_rkPPosDst.y);

	kStatePacket.dwTime = ELTimer_GetServerMSec();

	assert(kStatePacket.lX >= 0 && kStatePacket.lX < 204800);

	__LocalPositionToGlobalPosition(kStatePacket.lX, kStatePacket.lY);

	if (!Send(sizeof(kStatePacket), &kStatePacket))
	{
		Tracenf("CPythonNetworkStream::SendCharacterStatePacket(dwCmdTime=%u, fDstPos=(%f, %f), fDstRot=%f, eFunc=%d uArg=%d) - PACKET SEND ERROR",
			kStatePacket.dwTime,
			float(kStatePacket.lX),
			float(kStatePacket.lY),
			fDstRot,
			kStatePacket.bFunc,
			kStatePacket.bArg);
		return false;
	}
	return SendSequence();
}

bool CPythonNetworkStream::SendUseSkillPacket(DWORD dwSkillIndex, DWORD dwTargetVID)
{
	TPacketCGUseSkill UseSkillPacket;
	UseSkillPacket.bHeader = HEADER_CG_USE_SKILL;
	UseSkillPacket.dwVnum = dwSkillIndex;
	UseSkillPacket.dwTargetVID = dwTargetVID;
	if (!Send(sizeof(TPacketCGUseSkill), &UseSkillPacket))
	{
		Tracen("CPythonNetworkStream::SendUseSkillPacket - SEND PACKET ERROR");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendChatPacket(const char * c_szChat, BYTE byType)
{
	if (strlen(c_szChat) == 0)
		return true;

	if (strlen(c_szChat) >= 512)
		return true;

	if (c_szChat[0] == '/')
	{
		if (1 == strlen(c_szChat))
		{
			if (!m_strLastCommand.empty())
				c_szChat = m_strLastCommand.c_str();
		}
		else
		{
			m_strLastCommand = c_szChat;
		}
	}

	if (ClientCommand(c_szChat))
		return true;

	int iTextLen = strlen(c_szChat) + 1;
	TPacketCGChat ChatPacket;
	ChatPacket.header = HEADER_CG_CHAT;
	ChatPacket.length = sizeof(ChatPacket) + iTextLen;
	ChatPacket.type = byType;

	if (!Send(sizeof(ChatPacket), &ChatPacket))
		return false;

	if (!Send(iTextLen, c_szChat))
		return false;

	return SendSequence();
}

//////////////////////////////////////////////////////////////////////////
// Emoticon
void CPythonNetworkStream::RegisterEmoticonString(const char * pcEmoticonString)
{
	if (m_EmoticonStringVector.size() >= CInstanceBase::EMOTICON_NUM)
	{
		TraceError("Can't register emoticon string... vector is full (size:%d)", m_EmoticonStringVector.size() );
		return;
	}
	m_EmoticonStringVector.push_back(pcEmoticonString);
}

bool CPythonNetworkStream::ParseEmoticon(const char * pChatMsg, DWORD * pdwEmoticon)
{
	for (DWORD dwEmoticonIndex = 0; dwEmoticonIndex < m_EmoticonStringVector.size() ; ++dwEmoticonIndex)
	{
		if (strlen(pChatMsg) > m_EmoticonStringVector[dwEmoticonIndex].size())
			continue;

		const char * pcFind = strstr(pChatMsg, m_EmoticonStringVector[dwEmoticonIndex].c_str());

		if (pcFind != pChatMsg)
			continue;

		*pdwEmoticon = dwEmoticonIndex;

		return true;
	}

	return false;
}
// Emoticon
//////////////////////////////////////////////////////////////////////////

void CPythonNetworkStream::__ConvertEmpireText(DWORD dwEmpireID, char* szText)
{
	if (dwEmpireID<1 || dwEmpireID>3)
		return;

	UINT uHanPos;

	STextConvertTable& rkTextConvTable=m_aTextConvTable[dwEmpireID-1];

	BYTE* pbText=(BYTE*)szText;
	while (*pbText)
	{
		if (*pbText & 0x80)
		{
			if (pbText[0]>=0xb0 && pbText[0]<=0xc8 && pbText[1]>=0xa1 && pbText[1]<=0xfe)
			{
				uHanPos=(pbText[0]-0xb0)*(0xfe-0xa1+1)+(pbText[1]-0xa1);
				pbText[0]=rkTextConvTable.aacHan[uHanPos][0];
				pbText[1]=rkTextConvTable.aacHan[uHanPos][1];
			}
			pbText+=2;
		}
		else
		{
			if (*pbText>='a' && *pbText<='z')
			{
				*pbText=rkTextConvTable.acLower[*pbText-'a'];
			}
			else if (*pbText>='A' && *pbText<='Z')
			{
				*pbText=rkTextConvTable.acUpper[*pbText-'A'];
			}
			pbText++;
		}
	}
}

bool CPythonNetworkStream::RecvChatPacket()
{
	TPacketGCChat kChat;
    char buf[1024 + 1];
	char line[1024 + 1];

	if (!Recv(sizeof(kChat), &kChat))
		return false;

	UINT uChatSize=kChat.size - sizeof(kChat);

	if (!Recv(uChatSize, buf))
		return false;

	buf[uChatSize]='\0';

	if (GetDefaultCodePage() == 1256)
	{
		char * p = strchr(buf, ':');
		if (p && p[1] == ' ')
			p[1] = 0x08;
	}

	if (kChat.type >= CHAT_TYPE_MAX_NUM)
		return true;

	if (CHAT_TYPE_COMMAND == kChat.type)
	{
		ServerCommand(buf);
		return true;
	}

	if (kChat.dwVID != 0)
	{
		CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
		CInstanceBase * pkInstChatter = rkChrMgr.GetInstancePtr(kChat.dwVID);
		if (NULL == pkInstChatter)
			return true;

		switch (kChat.type)
		{
		case CHAT_TYPE_TALKING:
		case CHAT_TYPE_PARTY:
		case CHAT_TYPE_GUILD:
		case CHAT_TYPE_SHOUT:
		case CHAT_TYPE_WHISPER:
			{
				char * p = strchr(buf, ':');

				if (p)
					p += 2;
				else
					p = buf;

				DWORD dwEmoticon;

				if (ParseEmoticon(p, &dwEmoticon))
				{
					pkInstChatter->SetEmoticon(dwEmoticon);
					return true;
				}
				else
				{
					if (gs_bEmpireLanuageEnable)
					{
						CInstanceBase* pkInstMain=rkChrMgr.GetMainInstancePtr();
						if (pkInstMain)
							if (!pkInstMain->IsSameEmpire(*pkInstChatter))
								__ConvertEmpireText(pkInstChatter->GetEmpireID(), p);
					}

					if (m_isEnableChatInsultFilter)
					{
						if (false == pkInstChatter->IsNPC() && false == pkInstChatter->IsEnemy())
						{
							__FilterInsult(p, strlen(p));
						}
					}

					_snprintf(line, sizeof(line), "%s", p);
				}
			}
			break;
		case CHAT_TYPE_COMMAND:
		case CHAT_TYPE_INFO:
		case CHAT_TYPE_NOTICE:
		case CHAT_TYPE_BIG_NOTICE:
		// case CHAT_TYPE_UNK_10:
#ifdef ENABLE_DICE_SYSTEM
		case CHAT_TYPE_DICE_INFO:
#endif
#if defined(ENABLE_CHATTING_WINDOW_RENEWAL)
		case CHAT_TYPE_EXP_INFO:
		case CHAT_TYPE_ITEM_INFO:
		case CHAT_TYPE_MONEY_INFO:
#endif
#ifdef ENABLE_12ZI
		case CHAT_TYPE_MISSION:
		case CHAT_TYPE_SUB_MISSION:
		case CHAT_TYPE_CLEAR_MISSION:
#endif
		case CHAT_TYPE_BOSS_KILL:
		case CHAT_TYPE_INFO_QUEST:
		case CHAT_TYPE_STONE_KILL:
		case CHAT_TYPE_MAX_NUM:
		default:
			_snprintf(line, sizeof(line), "%s", buf);
			break;
		}

		if (CHAT_TYPE_SHOUT != kChat.type)
		{
			CPythonTextTail::Instance().RegisterChatTail(kChat.dwVID, line);
		}

#ifdef ENABLE_12ZI
		if (pkInstChatter->IsPC() && (CHAT_TYPE_MISSION != kChat.type && CHAT_TYPE_SUB_MISSION != kChat.type && CHAT_TYPE_CLEAR_MISSION != kChat.type))
			CPythonChat::Instance().AppendChat(kChat.type, buf);
#else
		if (pkInstChatter->IsPC())
			CPythonChat::Instance().AppendChat(kChat.type, buf);
#endif
	}
	else
	{
		if (CHAT_TYPE_NOTICE == kChat.type)
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_SetTipMessage", Py_BuildValue("(s)", buf));
		}
		else if (CHAT_TYPE_BIG_NOTICE == kChat.type)
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_SetBigMessage", Py_BuildValue("(s)", buf));
		}
		else if (CHAT_TYPE_SHOUT == kChat.type)
		{
			char * p = strchr(buf, ':');

			if (p)
			{
				if (m_isEnableChatInsultFilter)
					__FilterInsult(p, strlen(p));
			}
		}
#ifdef ENABLE_12ZI
		else if (CHAT_TYPE_MISSION == kChat.type)
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_SetMissionMessage", Py_BuildValue("(s)", buf));
			return true;
		}
		else if (CHAT_TYPE_SUB_MISSION == kChat.type)
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_SetSubMissionMessage", Py_BuildValue("(s)", buf));
			return true;
		}
		else if (CHAT_TYPE_CLEAR_MISSION == kChat.type)
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_CleanMissionMessage", Py_BuildValue("()"));
			return true;
		}
#endif

#ifdef ENABLE_MESSENGER_BLOCK
	// Intercept and filter global shouts locally to save server CPU
	if (kChat.type == CHAT_TYPE_SHOUT)
	{
		const char* pNameStart = strstr(buf, "|Hmsg:");
		if (pNameStart)
		{
			pNameStart += 6; // Move past the "|Hmsg:" tag
			const char* pNameEnd = strchr(pNameStart, '|');
			if (pNameEnd)
			{
				std::string senderName(pNameStart, pNameEnd - pNameStart);
				
				// If the sender is in our local RAM block map, drop the packet silently
				if (CPythonMessenger::Instance().IsBlockByName(senderName.c_str()))
					return true; 
			}
		}
	}
#endif

		CPythonChat::Instance().AppendChat(kChat.type, buf);

	}
	return true;
}

bool CPythonNetworkStream::RecvWhisperPacket()
{
	TPacketGCWhisper whisperPacket;
    char buf[512 + 1];

	if (!Recv(sizeof(whisperPacket), &whisperPacket))
		return false;

	assert(whisperPacket.wSize - sizeof(whisperPacket) < 512);

	if (!Recv(whisperPacket.wSize - sizeof(whisperPacket), &buf))
		return false;

	buf[whisperPacket.wSize - sizeof(whisperPacket)] = '\0';

	static char line[256];
	if (CPythonChat::WHISPER_TYPE_CHAT == whisperPacket.bType || CPythonChat::WHISPER_TYPE_GM == whisperPacket.bType)
	{
		_snprintf(line, sizeof(line), "%s : %s", whisperPacket.szNameFrom, buf);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnRecvWhisper", Py_BuildValue("(iss)", (int) whisperPacket.bType, whisperPacket.szNameFrom, line));
	}
	else if (CPythonChat::WHISPER_TYPE_SYSTEM == whisperPacket.bType || CPythonChat::WHISPER_TYPE_ERROR == whisperPacket.bType)
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnRecvWhisperSystemMessage", Py_BuildValue("(iss)", (int) whisperPacket.bType, whisperPacket.szNameFrom, buf));
	}
	else
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnRecvWhisperError", Py_BuildValue("(iss)", (int) whisperPacket.bType, whisperPacket.szNameFrom, buf));
	}

	return true;
}

bool CPythonNetworkStream::SendWhisperPacket(const char * name, const char * c_szChat)
{
	if (strlen(c_szChat) >= 255)
		return true;

	int iTextLen = strlen(c_szChat) + 1;
	TPacketCGWhisper WhisperPacket;
	WhisperPacket.bHeader = HEADER_CG_WHISPER;
	WhisperPacket.wSize = sizeof(WhisperPacket) + iTextLen;

	strncpy(WhisperPacket.szNameTo, name, sizeof(WhisperPacket.szNameTo) - 1);

	if (!Send(sizeof(WhisperPacket), &WhisperPacket))
		return false;

	if (!Send(iTextLen, c_szChat))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvPointChange()
{
	TPacketGCPointChange PointChange;

	if (!Recv(sizeof(TPacketGCPointChange), &PointChange))
	{
		Tracen("Recv Point Change Packet Error");
		return false;
	}

	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	rkChrMgr.ShowPointEffect(PointChange.Type, PointChange.dwVID);

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();

	if (pInstance && PointChange.dwVID == pInstance->GetVirtualID())
	{
		CPythonPlayer & rkPlayer = CPythonPlayer::Instance();
		rkPlayer.SetStatus(PointChange.Type, PointChange.value);

// === Client-Side Chat Log ===
		if (PointChange.amount > 0)
		{
			char szMsg[256];
			bool bLog = false;
			BYTE bChatType = CHAT_TYPE_INFO;

			if (PointChange.Type == POINT_EXP)
			{
				sprintf(szMsg, " áÞÏ ÍÕáÊ Úáì %lld ÎÈÑÉ. ", (long long)PointChange.amount);

				bChatType = CHAT_TYPE_EXP_INFO;

				bLog = true;
			}
#ifdef ENABLE_CONQUEROR_LEVEL
			else if (PointChange.Type == POINT_CONQUEROR_EXP)
			{
				sprintf(szMsg, " áÞÏ ÍÕáÊ Úáì %lld ÎÈÑÉ ÈØá. ", (long long)PointChange.amount);

				bChatType = CHAT_TYPE_EXP_INFO;

				bLog = true;
			}
#endif
			else if (PointChange.Type == POINT_GOLD)
			{
				sprintf(szMsg, " áÞÏ ÍÕáÊ Úáì %lld íÇäÛ. ", (long long)PointChange.amount);

				bChatType = CHAT_TYPE_MONEY_INFO;

				bLog = true;
			}

			if (bLog)
			{
				CPythonChat::Instance().AppendChat(bChatType, szMsg);
			}
		}
		// ============================

		switch (PointChange.Type)
		{
			case POINT_STAT_RESET_COUNT:
				__RefreshStatus();
				break;
			case POINT_LEVEL:
			case POINT_ST:
			case POINT_DX:
			case POINT_HT:
			case POINT_IQ:
#ifdef ENABLE_CONQUEROR_LEVEL
			case POINT_CONQUEROR_LEVEL:
			case POINT_SUNGMA_STR:
			case POINT_SUNGMA_HP:
			case POINT_SUNGMA_MOVE:
			case POINT_SUNGMA_INMUNE:			
#endif
				__RefreshStatus();
				__RefreshSkillWindow();
				break;
			case POINT_SKILL:
			case POINT_SUB_SKILL:
			case POINT_HORSE_SKILL:
				__RefreshSkillWindow();
				break;
			case POINT_ENERGY:
				if (PointChange.value == 0)
				{
					rkPlayer.SetStatus(POINT_ENERGY_END_TIME, 0);
				}
				__RefreshStatus();
				break;
			default:
				__RefreshStatus();
				break;
		}

		if (POINT_GOLD == PointChange.Type)
		{
			if (PointChange.amount > 0)
			{
				//PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnPickMoney", Py_BuildValue("(i)", PointChange.amount));
			}
		}

#ifdef ENABLE_CHEQUE_SYSTEM
		else if (POINT_CHEQUE == PointChange.Type)
		{
			if (PointChange.amount > 0)
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnPickCheque", Py_BuildValue("(i)", PointChange.amount));
		}
#endif
#ifdef ENABLE_GEM_SYSTEM
		else if (POINT_GEM == PointChange.Type)
		{
			if (PointChange.amount > 0)
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnPickGem", Py_BuildValue("(i)", PointChange.amount));
			}
		}
#endif
	}
#ifdef ENABLE_TEXT_LEVEL_REFRESH
	else
	{
		// the /advance command will provide no global refresh! it sends the pointchange only to the specific player and not all
		pInstance = CPythonCharacterManager::Instance().GetInstancePtr(PointChange.dwVID);
		if (pInstance && PointChange.Type == POINT_LEVEL)
		{
			pInstance->SetLevel(PointChange.value);
			pInstance->UpdateTextTailLevel(PointChange.value);
		}
	}
#endif

	return true;
}

bool CPythonNetworkStream::RecvStunPacket()
{
	TPacketGCStun StunPacket;

	if (!Recv(sizeof(StunPacket), &StunPacket))
	{
		Tracen("CPythonNetworkStream::RecvStunPacket Error");
		return false;
	}

	//Tracef("RecvStunPacket %d\n", StunPacket.vid);

	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	CInstanceBase * pkInstSel = rkChrMgr.GetInstancePtr(StunPacket.vid);

	if (pkInstSel)
	{
		if (CPythonCharacterManager::Instance().GetMainInstancePtr()==pkInstSel)
			pkInstSel->Die();
		else
			pkInstSel->Stun();
	}

	return true;
}

bool CPythonNetworkStream::RecvDeadPacket()
{
	TPacketGCDead DeadPacket;
	if (!Recv(sizeof(DeadPacket), &DeadPacket))
	{
		Tracen("CPythonNetworkStream::RecvDeadPacket Error");
		return false;
	}

	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	CInstanceBase * pkChrInstSel = rkChrMgr.GetInstancePtr(DeadPacket.vid);
	if (pkChrInstSel)
	{
		CInstanceBase* pkInstMain=rkChrMgr.GetMainInstancePtr();
		if (pkInstMain==pkChrInstSel)
		{
			Tracenf("On MainActor");
			if (false == pkInstMain->GetDuelMode())
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnGameOver", Py_BuildValue("()"));
			}
			CPythonPlayer::Instance().NotifyDeadMainCharacter();
		}

		pkChrInstSel->Die();
	}

	return true;
}

bool CPythonNetworkStream::SendCharacterPositionPacket(BYTE iPosition)
{
	TPacketCGPosition PositionPacket;

	PositionPacket.header = HEADER_CG_CHARACTER_POSITION;
	PositionPacket.position = iPosition;

	if (!Send(sizeof(TPacketCGPosition), &PositionPacket))
	{
		Tracen("Send Character Position Packet Error");
		return false;
	}

	return SendSequence();
}

#if defined(__BL_67_ATTR__)
bool CPythonNetworkStream::Send67AttrPacket(int iMaterialCount, int iSupportCount, int iSupportPos, int iItemPos)
{
	TPacketCG67Attr p =
	{
		static_cast<BYTE>(HEADER_CG_67_ATTR),
		static_cast<BYTE>(iMaterialCount),
		static_cast<BYTE>(iSupportCount),
		static_cast<short>(iSupportPos),
		static_cast<short>(iItemPos),
	};

	if (!Send(sizeof(p), &p))
	{
		Tracef("Send67AttrPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::Send67AttrClosePacket()
{
	TPacket67AttrOpenClose p;
	p.bHeader = HEADER_CG_CLOSE_67_ATTR;

	if (!Send(sizeof(p), &p))
	{
		Tracef("Send67AttrClosePacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::Recv67AttrOpenPacket()
{
	TPacket67AttrOpenClose p;

	if (!Recv(sizeof(p), &p))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenAttr67AddDlg", Py_BuildValue("()"));

	return true;
}
#endif

bool CPythonNetworkStream::SendOnClickPacket(DWORD vid)
{
	TPacketCGOnClick OnClickPacket;
	OnClickPacket.header	= HEADER_CG_ON_CLICK;
	OnClickPacket.vid		= vid;

	if (!Send(sizeof(OnClickPacket), &OnClickPacket))
	{
		Tracen("Send On_Click Packet Error");
		return false;
	}

	Tracef("SendOnClickPacket\n");
	return SendSequence();
}

bool CPythonNetworkStream::RecvCharacterPositionPacket()
{
	TPacketGCPosition PositionPacket;

	if (!Recv(sizeof(TPacketGCPosition), &PositionPacket))
		return false;

	CInstanceBase * pChrInstance = CPythonCharacterManager::Instance().GetInstancePtr(PositionPacket.vid);

	if (!pChrInstance)
		return true;

	//pChrInstance->UpdatePosition(PositionPacket.position);

	return true;
}

#if defined(__BL_KILL_BAR__)
bool CPythonNetworkStream::RecvKillBar()
{
	TPacketGCKillBar p;

	if (!Recv(sizeof(p), &p))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AddKillInfo", 
		Py_BuildValue("(ssiii)", p.szKiller, p.szVictim, p.bKillerRace, p.bVictimRace, p.bKillerWeaponType));

	return true;
}
#endif

#if defined(__BL_MOVE_COSTUME_ATTR__)
bool CPythonNetworkStream::SendItemCombinationPacket(const int iMediumIndex, const int iBaseIndex, const int iMaterialIndex)
{
	TPacketCGItemCombination p;
	p.Header = HEADER_CG_ITEM_COMBINATION;
	p.MediumIndex = (short)iMediumIndex;
	p.BaseIndex = (short)iBaseIndex;
	p.MaterialIndex = (short)iMaterialIndex;

	if (!Send(sizeof(p), &p))
	{
		Tracef("SendItemCombinationPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendItemCombinationPacketCancel()
{
	TPacketCGItemCombinationCancel p;
	p.Header = HEADER_CG_ITEM_COMBINATION_CANCEL;

	if (!Send(sizeof(p), &p))
	{
		Tracef("SendItemCombinationPacketCancel Error\n");
		return false;
	}

	return SendSequence();
}
#endif

bool CPythonNetworkStream::RecvMotionPacket()
{
	TPacketGCMotion MotionPacket;

	if (!Recv(sizeof(TPacketGCMotion), &MotionPacket))
		return false;

	CInstanceBase * pMainInstance = CPythonCharacterManager::Instance().GetInstancePtr(MotionPacket.vid);
	CInstanceBase * pVictimInstance = NULL;

	if (0 != MotionPacket.victim_vid)
		pVictimInstance = CPythonCharacterManager::Instance().GetInstancePtr(MotionPacket.victim_vid);

	if (!pMainInstance)
		return true;

	return true;
}

#if defined(__BL_MAILBOX__)
#include "PythonMail.h"

bool CPythonNetworkStream::RecvMailboxProcess()
{
	TPacketMailboxProcess p;

	if (!Recv(sizeof(p), &p))
		return false;

	switch (p.bSubHeader)
	{
	case CPythonMail::EMAILBOX_GC::MAILBOX_GC_CLOSE:
		CPythonMail::Instance().Destroy();
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MailBoxProcess", Py_BuildValue("(ii)", p.bSubHeader, p.bArg1));
		break;
	case CPythonMail::EMAILBOX_GC::MAILBOX_GC_OPEN:
	case CPythonMail::EMAILBOX_GC::MAILBOX_GC_POST_WRITE_CONFIRM:
	case CPythonMail::EMAILBOX_GC::MAILBOX_GC_POST_WRITE:
	case CPythonMail::EMAILBOX_GC::MAILBOX_GC_POST_ALL_DELETE:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MailBoxProcess", Py_BuildValue("(ii)", p.bSubHeader, p.bArg1));
		break;
	case CPythonMail::EMAILBOX_GC::MAILBOX_GC_ADD_DATA:
	case CPythonMail::EMAILBOX_GC::MAILBOX_GC_POST_DELETE:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MailBoxProcess", Py_BuildValue("(i(ii))", p.bSubHeader, p.bArg1, p.bArg2));
		break;
	case CPythonMail::EMAILBOX_GC::MAILBOX_GC_POST_GET_ITEMS:
		CPythonMail::Instance().ResetAddData(p.bArg1);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MailBoxProcess", Py_BuildValue("(i(ii))", p.bSubHeader, p.bArg1, p.bArg2));
		break;
	default:
		TraceError("CPythonNetworkStream::RecvMailboxProcess: Unknown subheader: %d\n", p.bSubHeader);
		break;
	}

	return true;
}

bool CPythonNetworkStream::RecvMailbox()
{
	TPacketGCMailBox p;
	if (!Recv(sizeof(p), &p))
		return false;

	CPythonMail::Instance().Destroy();
	
	unsigned int iPacketSize = (p.wSize - sizeof(TPacketGCMailBox));
	for (; iPacketSize > 0; iPacketSize -= sizeof(TPacketGCMailBoxMessage))
	{
		TPacketGCMailBoxMessage Message;
		if (!Recv(sizeof(Message), &Message))
			return false;

		CPythonMail::Instance().AddMail(new CPythonMail::SMailBox(Message.SendTime, Message.DeleteTime, Message.szTitle, Message.bIsGMPost, Message.bIsItemExist, Message.bIsConfirm));
	}

	return true;
}

bool CPythonNetworkStream::RecvMailboxAddData()
{
	TPacketGCMailBoxAddData p;

	if (!Recv(sizeof(p), &p))
		return false;

	CPythonMail::SMailBox* _Data = CPythonMail::Instance().GetMail(p.Index);
	if (_Data == nullptr)
	{
		Tracef("RecvMailboxAddData Error: SMailBox is null.\n");
		return true;
	}

	CPythonMail::SMailBoxAddData*& _AddData = _Data->AddData;
	if (_AddData != nullptr)
	{
		Tracef("RecvMailboxAddData Error: SMailBoxAddData is not null.\n");
		return true;
	}

	_Data->bIsConfirm = true;
	_AddData = new CPythonMail::SMailBoxAddData(p.szFrom, p.szMessage, p.iYang, p.iWon, p.ItemVnum, p.ItemCount, p.alSockets, p.aAttr
#if defined(__BL_TRANSMUTATION__)
		, p.dwTransmutationVnum
#endif
#if defined(ELEMENT_SPELL_WORLDARD)
		, p.grade_element, p.element_type_bonus, p.attack_element, p.elements_value_bonus
#endif
#ifdef ENABLE_NEW_SOCKET_VALUE
		, p.alMinMaxValue
#endif
	);
	return true;
}

bool CPythonNetworkStream::RecvMailboxAll()
{
	TPacketGCMailBox p;
	if (!Recv(sizeof(p), &p))
		return false;

	PyObject* list = PyList_New(0);

	unsigned int iPacketSize = (p.wSize - sizeof(TPacketGCMailBox));
	for (; iPacketSize > 0; iPacketSize -= sizeof(TPacketGCMailboxProcessAll))
	{
		TPacketGCMailboxProcessAll Index;
		if (!Recv(sizeof(Index), &Index))
			return false;

		CPythonMail::Instance().ResetAddData(Index.Index);
		PyList_Append(list, Py_BuildValue("i", Index.Index));
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MailBoxProcess", Py_BuildValue("i(iO)",
		CPythonMail::EMAILBOX_GC::MAILBOX_GC_POST_ALL_GET_ITEMS, CPythonMail::EMAILBOX_POST_ALL_GET_ITEMS::POST_ALL_GET_ITEMS_OK, list));

	Py_DECREF(list);
	return true;
}

bool CPythonNetworkStream::RecvMailboxUnread()
{
	TMailBoxRespondUnreadData p;

	if (!Recv(sizeof(p), &p))
		return false;

	const bool bFlash = p.bItemMessageCount > 0;
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MailBoxProcess", Py_BuildValue("i(iiiii)", CPythonMail::EMAILBOX_GC::MAILBOX_GC_UNREAD_DATA, 
		bFlash, (p.bItemMessageCount + p.bCommonMessageCount), p.bItemMessageCount, p.bCommonMessageCount, p.bGMVisible));

	return true;
}

bool CPythonNetworkStream::SendPostWriteConfirm(const char* szName)
{
	TPacketCGMailboxWriteConfirm p;
	p.bHeader = HEADER_CG_MAILBOX_WRITE_CONFIRM;
	std::strcpy(p.szName, szName);

	if (!Send(sizeof(p), &p))
	{
		Tracef("SendPostWriteConfirm Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendMailBoxClose()
{
	TPacketMailboxProcess p;
	p.bHeader = HEADER_CG_MAILBOX_PROCESS;
	p.bSubHeader = CPythonMail::EMAILBOX_CG::MAILBOX_CG_CLOSE;

	if (!Send(sizeof(p), &p))
	{
		Tracef("SendMailBoxClose Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendPostDelete(const BYTE Index)
{
	TPacketMailboxProcess p;
	p.bHeader = HEADER_CG_MAILBOX_PROCESS;
	p.bSubHeader = CPythonMail::EMAILBOX_CG::MAILBOX_CG_DELETE;
	p.bArg1 = Index;

	if (!Send(sizeof(p), &p))
	{
		Tracef("SendPostDelete Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendPostAllDelete()
{
	TPacketMailboxProcess p;
	p.bHeader = HEADER_CG_MAILBOX_PROCESS;
	p.bSubHeader = CPythonMail::EMAILBOX_CG::MAILBOX_CG_ALL_DELETE;

	if (!Send(sizeof(p), &p))
	{
		Tracef("SendPostAllDelete Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendPostGetItems(const BYTE Index)
{
	TPacketMailboxProcess p;
	p.bHeader = HEADER_CG_MAILBOX_PROCESS;
	p.bSubHeader = CPythonMail::EMAILBOX_CG::MAILBOX_CG_GET_ITEMS;
	p.bArg1 = Index;

	if (!Send(sizeof(p), &p))
	{
		Tracef("SendPostGetItems Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendPostAllGetItems()
{
	TPacketMailboxProcess p;
	p.bHeader = HEADER_CG_MAILBOX_PROCESS;
	p.bSubHeader = CPythonMail::EMAILBOX_CG::MAILBOX_CG_ALL_GET_ITEMS;

	if (!Send(sizeof(p), &p))
	{
		Tracef("SendPostAllGetItems Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RequestPostAddData(const BYTE ButtonIndex, const BYTE DataIndex)
{
	TPacketMailboxProcess p;
	p.bHeader = HEADER_CG_MAILBOX_PROCESS;
	p.bSubHeader = CPythonMail::EMAILBOX_CG::MAILBOX_CG_ADD_DATA;
	p.bArg1 = ButtonIndex;
	p.bArg2 = DataIndex;

	if (!Send(sizeof(p), &p))
	{
		Tracef("RequestPostAddData Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendPostWrite(const char* szName, const char* szTitle, const char* szMessage, const TItemPos& pos, const int iYang, const int iWon)
{
	TPacketCGMailboxWrite p;
	p.bHeader = HEADER_CG_MAILBOX_WRITE;
	std::strcpy(p.szName, szName);
	std::strcpy(p.szTitle, szTitle);
	std::strcpy(p.szMessage, szMessage);
	p.pos = pos;
	p.iYang = iYang;
	p.iWon = iWon;

	if (!Send(sizeof(p), &p))
	{
		Tracef("SendPostWrite Error\n");
		return false;
	}

	return SendSequence();
}
#endif

bool CPythonNetworkStream::RecvShopPacket()
{
	std::vector<char> vecBuffer;
	vecBuffer.clear();

    TPacketGCShop  packet_shop;
	if (!Recv(sizeof(packet_shop), &packet_shop))
		return false;

	int iSize = packet_shop.size - sizeof(packet_shop);
	if (iSize > 0)
	{
		vecBuffer.resize(iSize);
		if (!Recv(iSize, &vecBuffer[0]))
			return false;
	}

	switch (packet_shop.subheader)
	{
		case SHOP_SUBHEADER_GC_START:
			{
				CPythonShop::Instance().Clear();

				DWORD dwVID = *(DWORD *)&vecBuffer[0];

				TPacketGCShopStart * pShopStartPacket = (TPacketGCShopStart *)&vecBuffer[4];
				for (BYTE iItemIndex = 0; iItemIndex < SHOP_HOST_ITEM_MAX_NUM; ++iItemIndex)
				{
					CPythonShop::Instance().SetItemData(iItemIndex, pShopStartPacket->items[iItemIndex]);
				}
#ifdef ENABLE_12ZI
				CPythonShop::Instance().SetLimitedItemShop(pShopStartPacket->islimiteditemshop);
#endif
#if defined(WJ_COMBAT_ZONE)
				CPythonShop::instance().SetTabCoinType(0, SHOP_COIN_TYPE_GOLD);
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartShop", Py_BuildValue("(iiii)", dwVID, 0, 0, 0));
#else
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartShop", Py_BuildValue("(i)", dwVID));	
#endif
			}
			break;

		case SHOP_SUBHEADER_GC_START_EX:
			{
				CPythonShop::Instance().Clear();

				TPacketGCShopStartEx * pShopStartPacket = (TPacketGCShopStartEx *)&vecBuffer[0];
				size_t read_point = sizeof(TPacketGCShopStartEx);

				DWORD dwVID = pShopStartPacket->owner_vid;
				BYTE shop_tab_count = pShopStartPacket->shop_tab_count;
#if defined(WJ_COMBAT_ZONE)
				DWORD dwPoints = pShopStartPacket->points;	
				DWORD dwCurLimit = pShopStartPacket->curLimit;	
				DWORD dwMaxLimit = pShopStartPacket->maxLimit;	
#endif
				CPythonShop::instance().SetTabCount(shop_tab_count);

				for (size_t i = 0; i < shop_tab_count; i++)
				{
					TPacketGCShopStartEx::TSubPacketShopTab* pPackTab = (TPacketGCShopStartEx::TSubPacketShopTab*)&vecBuffer[read_point];
					read_point += sizeof(TPacketGCShopStartEx::TSubPacketShopTab);

					CPythonShop::instance().SetTabCoinType(i, pPackTab->coin_type);
					CPythonShop::instance().SetTabName(i, pPackTab->name);

					struct packet_shop_item* item = &pPackTab->items[0];

					for (BYTE j = 0; j < SHOP_HOST_ITEM_MAX_NUM; j++)
					{
						TShopItemData* itemData = (item + j);
						CPythonShop::Instance().SetItemData(i, j, *itemData);
					}
				}

#if defined(WJ_COMBAT_ZONE)
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartShop", Py_BuildValue("(iiii)", dwVID, dwPoints, dwCurLimit, dwMaxLimit));
#else
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartShop", Py_BuildValue("(i)", dwVID));					
#endif
			}
			break;

		case SHOP_SUBHEADER_GC_END:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "EndShop", Py_BuildValue("()"));
			break;

		case SHOP_SUBHEADER_GC_UPDATE_ITEM:
			{
				TPacketGCShopUpdateItem * pShopUpdateItemPacket = (TPacketGCShopUpdateItem *)&vecBuffer[0];
				CPythonShop::Instance().SetItemData(pShopUpdateItemPacket->pos, pShopUpdateItemPacket->item);
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshShop", Py_BuildValue("()"));
			}
			break;

		case SHOP_SUBHEADER_GC_UPDATE_PRICE:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetShopSellingPrice", Py_BuildValue("(i)", *(int *)&vecBuffer[0]));
			break;

		case SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_MONEY"));
			break;
#ifdef ENABLE_MULTISHOP
		case SHOP_SUBHEADER_GC_NOT_ENOUGH_ITEM:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_ITEM"));
			break;
#endif
		case SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY_EX:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_MONEY_EX"));
			break;
#if defined(WJ_COMBAT_ZONE)
		case SHOP_SUBHEADER_GC_NOT_ENOUGH_POINTS:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_POINTS"));
			break;
			
		case SHOP_SUBHEADER_GC_MAX_LIMIT_POINTS:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "MAX_LIMIT_POINTS"));
			break;
			
		case SHOP_SUBHEADER_GC_OVERFLOW_LIMIT_POINTS:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "OVERFLOW_LIMIT_POINTS"));
			break;
#endif
#ifdef ENABLE_CHEQUE_SYSTEM
		case SHOP_SUBHEADER_GC_NOT_ENOUGH_CHEQUE:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_CHEQUE"));
			break;

		case SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY_CHEQUE:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_MONEY_CHEQUE"));
			break;
#endif
#ifdef ENABLE_12ZI
		case SHOP_SUBHEADER_GC_LIMITED_PURCHASE_OVER:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "LIMITED_PURCHASE_OVER"));
			break;
#endif
		case SHOP_SUBHEADER_GC_SOLDOUT:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "SOLDOUT"));
			break;

		case SHOP_SUBHEADER_GC_INVENTORY_FULL:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "INVENTORY_FULL"));
			break;

		case SHOP_SUBHEADER_GC_INVALID_POS:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "INVALID_POS"));
			break;

		default:
			TraceError("CPythonNetworkStream::RecvShopPacket: Unknown subheader\n");
			break;
	}

	return true;
}

bool CPythonNetworkStream::RecvExchangePacket()
{
	TPacketGCExchange exchange_packet;

	if (!Recv(sizeof(exchange_packet), &exchange_packet))
		return false;

	switch (exchange_packet.subheader)
	{
		case EXCHANGE_SUBHEADER_GC_START:
			CPythonExchange::Instance().Clear();
			CPythonExchange::Instance().Start();
			CPythonExchange::Instance().SetSelfName(CPythonPlayer::Instance().GetName());
#ifdef ENABLE_LEVEL_IN_TRADE
			//CPythonExchange::Instance().SetSelfLevel(CPythonPlayer::Instance().GetLevel());
			CPythonExchange::Instance().SetSelfLevel(CPythonPlayer::Instance().GetStatus(POINT_LEVEL));
#endif

			{
				CInstanceBase * pCharacterInstance = CPythonCharacterManager::Instance().GetInstancePtr(exchange_packet.arg1);

				if (pCharacterInstance)
				{
					CPythonExchange::Instance().SetTargetName(pCharacterInstance->GetNameString());
#ifdef ENABLE_LEVEL_IN_TRADE
					CPythonExchange::Instance().SetTargetLevel(pCharacterInstance->GetLevel());
#endif
				}
			}

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartExchange", Py_BuildValue("()"));
			break;

		case EXCHANGE_SUBHEADER_GC_ITEM_ADD:
			if (exchange_packet.is_me)
			{
				int iSlotIndex = exchange_packet.arg2.cell;
				CPythonExchange::Instance().SetItemToSelf(iSlotIndex, exchange_packet.arg1, (WORD) exchange_packet.arg3);
#ifdef ELEMENT_SPELL_WORLDARD
				CPythonExchange::Instance().SetItemElementToSelf(iSlotIndex,exchange_packet.grade_element,exchange_packet.element_type_bonus);

				for (int i = 0; i < MAX_ELEMENTS_SPELL; ++i)
				{
					CPythonExchange::Instance().SetItemElementAttackToSelf(iSlotIndex,i,exchange_packet.attack_element[i]);
					CPythonExchange::Instance().SetItemElementValueToSelf(iSlotIndex,i,exchange_packet.elements_value_bonus[i]);
				}
#endif
				for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
					CPythonExchange::Instance().SetItemMetinSocketToSelf(iSlotIndex, i, exchange_packet.alValues[i]);
				for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
					CPythonExchange::Instance().SetItemAttributeToSelf(iSlotIndex, j, exchange_packet.aAttr[j].bType, exchange_packet.aAttr[j].sValue);
#ifdef ENABLE_NEW_SOCKET_VALUE
				for (int k = 0; k < ITEM_MIN_MAX_VALUE_MAX_NUM; ++k)
					CPythonExchange::Instance().SetItemMinMaxValueToSelf(iSlotIndex, k, exchange_packet.alMinMaxValue[k]);
#endif
#if defined(__BL_TRANSMUTATION__)
				CPythonExchange::Instance().SetItemChangeLookVnumToSelf(iSlotIndex, exchange_packet.dwTransmutationVnum);
#endif

#ifdef WJ_ENABLE_TRADABLE_ICON
				PyObject *args = PyTuple_New(3);
				PyTuple_SetItem(args, 0, PyInt_FromLong(exchange_packet.arg4.window_type));
				PyTuple_SetItem(args, 1, PyInt_FromLong(exchange_packet.arg4.cell));
				PyTuple_SetItem(args, 2, PyInt_FromLong(iSlotIndex));
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AddItemToExchange", args);
#endif
			}
			else
			{
				int iSlotIndex = exchange_packet.arg2.cell;
				CPythonExchange::Instance().SetItemToTarget(iSlotIndex, exchange_packet.arg1, (WORD)exchange_packet.arg3);
#ifdef ELEMENT_SPELL_WORLDARD
				CPythonExchange::Instance().SetItemElementToTarget(iSlotIndex,exchange_packet.grade_element,exchange_packet.element_type_bonus);

				for (int i = 0; i < MAX_ELEMENTS_SPELL; ++i)
				{
					CPythonExchange::Instance().SetItemElementAttackToTarget(iSlotIndex,i,exchange_packet.attack_element[i]);
					CPythonExchange::Instance().SetItemElementValueToTarget(iSlotIndex,i,exchange_packet.elements_value_bonus[i]);
				}
#endif
				for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
					CPythonExchange::Instance().SetItemMetinSocketToTarget(iSlotIndex, i, exchange_packet.alValues[i]);
				for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
					CPythonExchange::Instance().SetItemAttributeToTarget(iSlotIndex, j, exchange_packet.aAttr[j].bType, exchange_packet.aAttr[j].sValue);
#ifdef ENABLE_NEW_SOCKET_VALUE
				for (int k = 0; k < ITEM_MIN_MAX_VALUE_MAX_NUM; ++k)
					CPythonExchange::Instance().SetItemMinMaxValueToTarget(iSlotIndex, k, exchange_packet.alMinMaxValue[k]);
#endif
#if defined(__BL_TRANSMUTATION__)
				CPythonExchange::Instance().SetItemChangeLookVnumToTarget(iSlotIndex, exchange_packet.dwTransmutationVnum);
#endif
			}

			__RefreshExchangeWindow();
			__RefreshInventoryWindow();
			break;

		case EXCHANGE_SUBHEADER_GC_ITEM_DEL:
			if (exchange_packet.is_me)
			{
				CPythonExchange::Instance().DelItemOfSelf((BYTE) exchange_packet.arg1);
			}
			else
			{
				CPythonExchange::Instance().DelItemOfTarget((BYTE) exchange_packet.arg1);
			}
			__RefreshExchangeWindow();
			__RefreshInventoryWindow();
			break;

		case EXCHANGE_SUBHEADER_GC_ELK_ADD:
			if (exchange_packet.is_me)
				CPythonExchange::Instance().SetElkToSelf(exchange_packet.arg1);
			else
				CPythonExchange::Instance().SetElkToTarget(exchange_packet.arg1);

			__RefreshExchangeWindow();
			break;

#ifdef ENABLE_CHEQUE_SYSTEM
		case EXCHANGE_SUBHEADER_GC_CHEQUE_ADD:
			if (exchange_packet.is_me)
				CPythonExchange::Instance().SetChequeToSelf(exchange_packet.arg1);
			else
				CPythonExchange::Instance().SetChequeToTarget(exchange_packet.arg1);

			__RefreshExchangeWindow();
			break;
#endif

		case EXCHANGE_SUBHEADER_GC_ACCEPT:
			if (exchange_packet.is_me)
			{
				CPythonExchange::Instance().SetAcceptToSelf((BYTE) exchange_packet.arg1);
			}
			else
			{
				CPythonExchange::Instance().SetAcceptToTarget((BYTE) exchange_packet.arg1);
			}
			__RefreshExchangeWindow();
			break;

		case EXCHANGE_SUBHEADER_GC_END:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "EndExchange", Py_BuildValue("()"));
			__RefreshInventoryWindow();
			CPythonExchange::Instance().End();
			break;

		case EXCHANGE_SUBHEADER_GC_ALREADY:
			Tracef("trade_already");
			break;

		case EXCHANGE_SUBHEADER_GC_LESS_ELK:
			Tracef("trade_less_elk");
			break;

#ifdef ENABLE_CHEQUE_SYSTEM
		case EXCHANGE_SUBHEADER_GC_LESS_CHEQUE:
			Tracef("trade_less_cheque");
			break;
#endif
	};

	return true;
}

bool CPythonNetworkStream::RecvQuestInfoPacket()
{
	TPacketGCQuestInfo QuestInfo;

	if (!Peek(sizeof(TPacketGCQuestInfo), &QuestInfo))
	{
		Tracen("Recv Quest Info Packet Error #1");
		return false;
	}

	if (!PeekNoFetch(QuestInfo.size))
	{
		Tracen("Recv Quest Info Packet Error #2");
		return false;
	}

	RecvNoFetch(sizeof(TPacketGCQuestInfo));

	const BYTE & c_rFlag = QuestInfo.flag;

	enum
	{
		QUEST_PACKET_TYPE_NONE,
		QUEST_PACKET_TYPE_BEGIN,
		QUEST_PACKET_TYPE_UPDATE,
		QUEST_PACKET_TYPE_END,
	};

	BYTE byQuestPacketType = QUEST_PACKET_TYPE_NONE;

	if (0 != (c_rFlag & QUEST_SEND_IS_BEGIN))
	{
		BYTE isBegin;
		if (!Recv(sizeof(isBegin), &isBegin))
			return false;

		if (isBegin)
			byQuestPacketType = QUEST_PACKET_TYPE_BEGIN;
		else
			byQuestPacketType = QUEST_PACKET_TYPE_END;
	}
	else
	{
		byQuestPacketType = QUEST_PACKET_TYPE_UPDATE;
	}

	// Recv Data Start
	char szTitle[30 + 1] = "";
	char szClockName[16 + 1] = "";
	int iClockValue = 0;
	char szCounterName[16 + 1] = "";
	int iCounterValue = 0;
	char szIconFileName[24 + 1] = "";

	if (0 != (c_rFlag & QUEST_SEND_TITLE))
	{
		if (!Recv(sizeof(szTitle), &szTitle))
			return false;

		szTitle[30]='\0';
	}
	if (0 != (c_rFlag & QUEST_SEND_CLOCK_NAME))
	{
		if (!Recv(sizeof(szClockName), &szClockName))
			return false;

		szClockName[16]='\0';
	}
	if (0 != (c_rFlag & QUEST_SEND_CLOCK_VALUE))
	{
		if (!Recv(sizeof(iClockValue), &iClockValue))
			return false;
	}
	if (0 != (c_rFlag & QUEST_SEND_COUNTER_NAME))
	{
		if (!Recv(sizeof(szCounterName), &szCounterName))
			return false;

		szCounterName[16]='\0';
	}
	if (0 != (c_rFlag & QUEST_SEND_COUNTER_VALUE))
	{
		if (!Recv(sizeof(iCounterValue), &iCounterValue))
			return false;
	}
	if (0 != (c_rFlag & QUEST_SEND_ICON_FILE))
	{
		if (!Recv(sizeof(szIconFileName), &szIconFileName))
			return false;

		szIconFileName[24]='\0';
	}
	// Recv Data End

	CPythonQuest& rkQuest=CPythonQuest::Instance();

	// Process Start
	if (QUEST_PACKET_TYPE_END == byQuestPacketType)
	{
		rkQuest.DeleteQuestInstance(QuestInfo.index);
#if defined(ENABLE_QUEST_RENEWAL)
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "DeleteQuest", Py_BuildValue("(ii)", QuestInfo.type, QuestInfo.index));
#endif
	}
	else if (QUEST_PACKET_TYPE_UPDATE == byQuestPacketType)
	{
		if (!rkQuest.IsQuest(QuestInfo.index))
		{
#if defined(ENABLE_QUEST_RENEWAL)
			rkQuest.MakeQuest(QuestInfo.index, QuestInfo.type, QuestInfo.is_confirmed);
#else
			rkQuest.MakeQuest(QuestInfo.index);
#endif
		}

		if (strlen(szTitle) > 0)
			rkQuest.SetQuestTitle(QuestInfo.index, szTitle);
		if (strlen(szClockName) > 0)
			rkQuest.SetQuestClockName(QuestInfo.index, szClockName);
		if (strlen(szCounterName) > 0)
			rkQuest.SetQuestCounterName(QuestInfo.index, szCounterName);
		if (strlen(szIconFileName) > 0)
			rkQuest.SetQuestIconFileName(QuestInfo.index, szIconFileName);

		if (c_rFlag & QUEST_SEND_CLOCK_VALUE)
			rkQuest.SetQuestClockValue(QuestInfo.index, iClockValue);
		if (c_rFlag & QUEST_SEND_COUNTER_VALUE)
			rkQuest.SetQuestCounterValue(QuestInfo.index, iCounterValue);
	}
	else if (QUEST_PACKET_TYPE_BEGIN == byQuestPacketType)
	{
		CPythonQuest::SQuestInstance QuestInstance;
		QuestInstance.dwIndex = QuestInfo.index;
		QuestInstance.strTitle = szTitle;
		QuestInstance.strClockName = szClockName;
		QuestInstance.iClockValue = iClockValue;
		QuestInstance.strCounterName = szCounterName;
		QuestInstance.iCounterValue = iCounterValue;
		QuestInstance.strIconFileName = szIconFileName;
		CPythonQuest::Instance().RegisterQuestInstance(QuestInstance);
	}
	// Process Start End
#if defined(ENABLE_QUEST_RENEWAL)
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshQuest", Py_BuildValue("(ii)", QuestInfo.type, QuestInfo.index));
#else
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshQuest", Py_BuildValue("()"));
#endif
	return true;
}

bool CPythonNetworkStream::RecvQuestConfirmPacket()
{
	TPacketGCQuestConfirm kQuestConfirmPacket;
	if (!Recv(sizeof(kQuestConfirmPacket), &kQuestConfirmPacket))
	{
		Tracen("RecvQuestConfirmPacket Error");
		return false;
	}

	PyObject * poArg = Py_BuildValue("(sii)", kQuestConfirmPacket.msg, kQuestConfirmPacket.timeout, kQuestConfirmPacket.requestPID);
 	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_OnQuestConfirm", poArg);
	return true;
}

bool CPythonNetworkStream::RecvRequestMakeGuild()
{
	TPacketGCBlank blank;
	if (!Recv(sizeof(blank), &blank))
	{
		Tracen("RecvRequestMakeGuild Packet Error");
		return false;
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AskGuildName", Py_BuildValue("()"));

	return true;
}

void CPythonNetworkStream::ToggleGameDebugInfo()
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ToggleDebugInfo", Py_BuildValue("()"));
}

#if defined(__BL_PICK_FILTER__)
void CPythonNetworkStream::OpenPickUpWindow()
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenPickUpWindow", Py_BuildValue("()"));
}
#endif

bool CPythonNetworkStream::SendExchangeStartPacket(DWORD vid)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange	packet;

	packet.header		= HEADER_CG_EXCHANGE;
	packet.subheader	= EXCHANGE_SUBHEADER_CG_START;
	packet.arg1			= vid;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_start_packet Error\n");
		return false;
	}

	Tracef("send_trade_start_packet   vid %d \n", vid);
	return SendSequence();
}

bool CPythonNetworkStream::SendExchangeElkAddPacket(DWORD elk)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange	packet;

	packet.header		= HEADER_CG_EXCHANGE;
	packet.subheader	= EXCHANGE_SUBHEADER_CG_ELK_ADD;
	packet.arg1			= elk;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_elk_add_packet Error\n");
		return false;
	}

	return SendSequence();
}

#ifdef ENABLE_CHEQUE_SYSTEM
bool CPythonNetworkStream::SendExchangeChequeAddPacket(DWORD elk)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange	packet;

	packet.header = HEADER_CG_EXCHANGE;
	packet.subheader = EXCHANGE_SUBHEADER_CG_CHEQUE_ADD;
	packet.arg1 = elk;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_cheque_add_packet Error\n");
		return false;
	}

	return SendSequence();
}
#endif

bool CPythonNetworkStream::SendExchangeItemAddPacket(TItemPos ItemPos, BYTE byDisplayPos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange	packet;

	packet.header		= HEADER_CG_EXCHANGE;
	packet.subheader	= EXCHANGE_SUBHEADER_CG_ITEM_ADD;
	packet.Pos			= ItemPos;
	packet.arg2			= byDisplayPos;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_item_add_packet Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendExchangeItemDelPacket(BYTE pos)
{
	assert(!"Can't be called function - CPythonNetworkStream::SendExchangeItemDelPacket");
	return true;

	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange	packet;

	packet.header		= HEADER_CG_EXCHANGE;
	packet.subheader	= EXCHANGE_SUBHEADER_CG_ITEM_DEL;
	packet.arg1			= pos;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_item_del_packet Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendExchangeAcceptPacket()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange	packet;

	packet.header		= HEADER_CG_EXCHANGE;
	packet.subheader	= EXCHANGE_SUBHEADER_CG_ACCEPT;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_accept_packet Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendExchangeExitPacket()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGExchange	packet;

	packet.header		= HEADER_CG_EXCHANGE;
	packet.subheader	= EXCHANGE_SUBHEADER_CG_CANCEL;

	if (!Send(sizeof(packet), &packet))
	{
		Tracef("send_trade_exit_packet Error\n");
		return false;
	}

	return SendSequence();
}

#ifdef ELEMENT_SPELL_WORLDARD
bool CPythonNetworkStream::RecvElementsSpellPacket()
{
	TPacketGCElementsSpell ElementsSpellPacket;

	if (!Recv(sizeof(ElementsSpellPacket), &ElementsSpellPacket))
		return false;

	switch (ElementsSpellPacket.subheader)
	{

		case ELEMENT_SPELL_SUB_HEADER_OPEN:
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ELEMENTS_SPELL_OPEN", Py_BuildValue("(ibii)",ElementsSpellPacket.pos, ElementsSpellPacket.func ,ElementsSpellPacket.cost, ElementsSpellPacket.grade_add));
		}

		break;

		case ELEMENT_SPELL_SUB_HEADER_OPEN_CHANGE:
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ELEMENTS_SPELL_OPEN_CHANGE", Py_BuildValue("(ii)",ElementsSpellPacket.pos, ElementsSpellPacket.cost));
		}

		break;

	}
	
	return true;
}

bool CPythonNetworkStream::SendElementsSpellClose()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGElementsSpell packet;
	
	packet.header 		= HEADER_CG_ELEMENTS_SPELL;
	packet.subheader	= ELEMENTS_SPELL_SUB_HEADER_CLOSE;

	if (!Send(sizeof(TPacketCGElementsSpell), &packet))
	{
		Tracef("SendElementsSpellClose Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendElementsSpellAdd(int pos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGElementsSpell packet;
	
	packet.header 		= HEADER_CG_ELEMENTS_SPELL;
	packet.subheader	= ELEMENTS_SPELL_SUB_HEADER_ADD_BONUS;
	packet.pos 			= pos;

	if (!Send(sizeof(TPacketCGElementsSpell), &packet))
	{
		Tracef("SendElementsSpellAdd Error\n");
		return false;
	}

	return SendSequence();
}


bool CPythonNetworkStream::SendElementsSpellChange(int pos, int type_select)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGElementsSpell packet;
	
	packet.header 		= HEADER_CG_ELEMENTS_SPELL;
	packet.subheader	= ELEMENTS_SPELL_SUB_HEADER_CHANGE_BONUS;
	packet.pos 			= pos;
	packet.type_select 	= type_select;

	if (!Send(sizeof(TPacketCGElementsSpell), &packet))
	{
		Tracef("SendElementsSpellChange Error\n");
		return false;
	}

	return SendSequence();
}


#endif

bool CPythonNetworkStream::SendPointResetPacket()
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartPointReset", Py_BuildValue("()"));
	return true;
}

bool CPythonNetworkStream::__IsPlayerAttacking()
{
	CPythonCharacterManager& rkChrMgr=CPythonCharacterManager::Instance();
	CInstanceBase* pkInstMain=rkChrMgr.GetMainInstancePtr();
	if (!pkInstMain)
		return false;

	if (!pkInstMain->IsAttacking())
		return false;

	return true;
}

bool CPythonNetworkStream::RecvScriptPacket()
{
	TPacketGCScript ScriptPacket;

	if (!Recv(sizeof(TPacketGCScript), &ScriptPacket))
	{
		TraceError("RecvScriptPacket_RecvError");
		return false;
	}

	if (ScriptPacket.size < sizeof(TPacketGCScript))
	{
		TraceError("RecvScriptPacket_SizeError");
		return false;
	}

	ScriptPacket.size -= sizeof(TPacketGCScript);

	static string str;
	str = "";
	str.resize(ScriptPacket.size+1);

	if (!Recv(ScriptPacket.size, &str[0]))
		return false;

	str[str.size()-1] = '\0';

	int iIndex = CPythonEventManager::Instance().RegisterEventSetFromString(str);

	if (-1 != iIndex)
	{
		CPythonEventManager::Instance().SetVisibleLineCount(iIndex, 30);
		CPythonNetworkStream::Instance().OnScriptEventStart(ScriptPacket.skin,iIndex);
	}

	return true;
}

bool CPythonNetworkStream::SendScriptAnswerPacket(int iAnswer)
{
	TPacketCGScriptAnswer ScriptAnswer;

	ScriptAnswer.header = HEADER_CG_SCRIPT_ANSWER;
	ScriptAnswer.answer = (BYTE) iAnswer;
	if (!Send(sizeof(TPacketCGScriptAnswer), &ScriptAnswer))
	{
		Tracen("Send Script Answer Packet Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendScriptButtonPacket(unsigned int iIndex)
{
	TPacketCGScriptButton ScriptButton;

	ScriptButton.header = HEADER_CG_SCRIPT_BUTTON;
	ScriptButton.idx = iIndex;
	if (!Send(sizeof(TPacketCGScriptButton), &ScriptButton))
	{
		Tracen("Send Script Button Packet Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendAnswerMakeGuildPacket(const char * c_szName)
{
	TPacketCGAnswerMakeGuild Packet;

	Packet.header = HEADER_CG_ANSWER_MAKE_GUILD;
	strncpy(Packet.guild_name, c_szName, GUILD_NAME_MAX_LEN);
	Packet.guild_name[GUILD_NAME_MAX_LEN] = '\0';

	if (!Send(sizeof(Packet), &Packet))
	{
		Tracen("SendAnswerMakeGuild Packet Error");
		return false;
	}

// 	Tracef(" SendAnswerMakeGuildPacket : %s", c_szName);
	return SendSequence();
}

bool CPythonNetworkStream::SendQuestInputStringPacket(const char * c_szString)
{
	TPacketCGQuestInputString Packet;
	Packet.bHeader = HEADER_CG_QUEST_INPUT_STRING;
	strncpy(Packet.szString, c_szString, QUEST_INPUT_STRING_MAX_NUM);

	if (!Send(sizeof(Packet), &Packet))
	{
		Tracen("SendQuestInputStringPacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendQuestConfirmPacket(BYTE byAnswer, DWORD dwPID)
{
	TPacketCGQuestConfirm kPacket;
	kPacket.header = HEADER_CG_QUEST_CONFIRM;
	kPacket.answer = byAnswer;
	kPacket.requestPID = dwPID;

	if (!Send(sizeof(kPacket), &kPacket))
	{
		Tracen("SendQuestConfirmPacket Error");
		return false;
	}

	Tracenf(" SendQuestConfirmPacket : %d, %d", byAnswer, dwPID);
	return SendSequence();
}

bool CPythonNetworkStream::RecvSkillCoolTimeEnd()
{
	TPacketGCSkillCoolTimeEnd kPacketSkillCoolTimeEnd;
	if (!Recv(sizeof(kPacketSkillCoolTimeEnd), &kPacketSkillCoolTimeEnd))
	{
		Tracen("CPythonNetworkStream::RecvSkillCoolTimeEnd - RecvError");
		return false;
	}

	CPythonPlayer::Instance().EndSkillCoolTime(kPacketSkillCoolTimeEnd.bSkill);

	return true;
}

bool CPythonNetworkStream::RecvSkillLevel()
{
	assert(!"CPythonNetworkStream::RecvSkillLevel - Don't use this function");
	TPacketGCSkillLevel packet;
	if (!Recv(sizeof(TPacketGCSkillLevel), &packet))
	{
		Tracen("CPythonNetworkStream::RecvSkillLevel - RecvError");
		return false;
	}

	DWORD dwSlotIndex;

	CPythonPlayer& rkPlayer=CPythonPlayer::Instance();
	for (int i = 0; i < SKILL_MAX_NUM; ++i)
	{
		if (rkPlayer.GetSkillSlotIndex(i, &dwSlotIndex))
			rkPlayer.SetSkillLevel(dwSlotIndex, packet.abSkillLevels[i]);
	}

	__RefreshSkillWindow();
	__RefreshStatus();
	Tracef(" >> RecvSkillLevel\n");
	return true;
}

bool CPythonNetworkStream::RecvSkillLevelNew()
{
	TPacketGCSkillLevelNew packet;
	if (!Recv(sizeof(TPacketGCSkillLevelNew), &packet))
	{
		Tracen("CPythonNetworkStream::RecvSkillLevelNew - RecvError");
		return false;
	}
	
	CPythonPlayer& rkPlayer=CPythonPlayer::Instance();
#ifdef ENABLE_7AND8TH_SKILLS
	BYTE SKILL_ANTI_PALBANG = 221;
	BYTE SKILL_ANTI_BYEURAK = 228;
#ifdef ENABLE_WOLFMAN_CHARACTER
	BYTE SKILL_ANTI_SALPOONG = 229;
#endif
	BYTE SKILL_HELP_PALBANG = 236;
	BYTE SKILL_HELP_BYEURAK = 243;
#ifdef ENABLE_WOLFMAN_CHARACTER
	BYTE SKILL_HELP_SALPOONG = 244;
#endif
	
	rkPlayer.SetSkill(7, 0);
	rkPlayer.SetSkill(8, 0);
#endif
	for (int i = 0; i < SKILL_MAX_NUM; ++i)
	{
		TPlayerSkill & rPlayerSkill = packet.skills[i];
#ifdef ENABLE_7AND8TH_SKILLS
#ifdef ENABLE_WOLFMAN_CHARACTER
		if (i >= SKILL_ANTI_PALBANG && i <= SKILL_ANTI_SALPOONG)
#else
		if (i >= SKILL_ANTI_PALBANG && i <= SKILL_ANTI_BYEURAK)
#endif
		{
			if (rPlayerSkill.bLevel >= 1)
			{
				rkPlayer.SetSkill(7, i);
				rkPlayer.SetSkillLevel_(i, rPlayerSkill.bMasterType, rPlayerSkill.bLevel);
			}
		}
#ifdef ENABLE_WOLFMAN_CHARACTER
		else if (i >= SKILL_HELP_PALBANG && i <= SKILL_HELP_SALPOONG)
#else
		else if (i >= SKILL_HELP_PALBANG && i <= SKILL_HELP_BYEURAK)
#endif
		{
			if (rPlayerSkill.bLevel >= 1)
			{
				rkPlayer.SetSkill(8, i);
				rkPlayer.SetSkillLevel_(i, rPlayerSkill.bMasterType, rPlayerSkill.bLevel);
			}
		}
		else
			rkPlayer.SetSkillLevel_(i, rPlayerSkill.bMasterType, rPlayerSkill.bLevel);
#else
		rkPlayer.SetSkillLevel_(i, rPlayerSkill.bMasterType, rPlayerSkill.bLevel);
#endif
	}
	
	__RefreshSkillWindow();
	__RefreshStatus();
	return true;
}

bool CPythonNetworkStream::RecvDamageInfoPacket()
{
	TPacketGCDamageInfo DamageInfoPacket;

	if (!Recv(sizeof(TPacketGCDamageInfo), &DamageInfoPacket))
	{
		Tracen("Recv Target Packet Error");
		return false;
	}

	CInstanceBase * pInstTarget = CPythonCharacterManager::Instance().GetInstancePtr(DamageInfoPacket.dwVID);
	
	if (pInstTarget)
	{
		bool bSelf = (pInstTarget == CPythonCharacterManager::Instance().GetMainInstancePtr());
		bool bTarget = (pInstTarget == m_pInstTarget);

#ifdef TARGET_DAMAGE
		// Only render if the damaged entity is our player or our active target
		if (bSelf || bTarget)
		{
			if (DamageInfoPacket.damage >= 0)
				pInstTarget->AddDamageEffect(DamageInfoPacket.damage, DamageInfoPacket.flag, bSelf, bTarget);
			else
				TraceError("Damage is equal or below 0.");
		}
#else
		if (DamageInfoPacket.damage >= 0)
			pInstTarget->AddDamageEffect(DamageInfoPacket.damage, DamageInfoPacket.flag, bSelf, bTarget);
		else
			TraceError("Damage is equal or below 0.");
#endif
	}

	return true;
}
bool CPythonNetworkStream::RecvTargetPacket()
{
	TPacketGCTarget TargetPacket;

	if (!Recv(sizeof(TPacketGCTarget), &TargetPacket))
	{
		Tracen("Recv Target Packet Error");
		return false;
	}
#if defined(ENABLE_SHIP_DEFENSE)
	if (TargetPacket.bAlliance)
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetHPAllianceTargetBoard", Py_BuildValue("(iLL)", TargetPacket.dwVID, TargetPacket.iAllianceMinHP, TargetPacket.iAllianceMaxHP));
		return true;
	}
#endif
	CInstanceBase * pInstPlayer = CPythonCharacterManager::Instance().GetMainInstancePtr();
	CInstanceBase * pInstTarget = CPythonCharacterManager::Instance().GetInstancePtr(TargetPacket.dwVID);
	if (pInstPlayer && pInstTarget)
	{
		if (!pInstTarget->IsDead())
		{
			#if defined(WJ_COMBAT_ZONE_HIDE_INFO_USER)
			if (pInstPlayer->IsCombatZoneMap() || pInstTarget->IsCombatZoneMap())
			{
				m_pInstTarget = pInstTarget;
				return true;
			}
			#endif
			if (pInstTarget->IsPC() || pInstTarget->IsBuilding())
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoardIfDifferent", Py_BuildValue("(i)", TargetPacket.dwVID));
			else if (pInstPlayer->CanViewTargetHP(*pInstTarget))
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetHPTargetBoard", Py_BuildValue("(ii)", TargetPacket.dwVID, TargetPacket.bHPPercent));
			else
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoard", Py_BuildValue("()"));

			m_pInstTarget = pInstTarget;
		}
	}
	else
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "CloseTargetBoard", Py_BuildValue("()"));
	}

	return true;
}

bool CPythonNetworkStream::RecvMountPacket()
{
	TPacketGCMount MountPacket;

	if (!Recv(sizeof(TPacketGCMount), &MountPacket))
	{
		Tracen("Recv Mount Packet Error");
		return false;
	}

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(MountPacket.vid);

	if (pInstance)
	{
		// Mount
		if (0 != MountPacket.mount_vid)
		{
//			pInstance->Ride(MountPacket.pos, MountPacket.mount_vid);
		}
		// Unmount
		else
		{
//			pInstance->Unride(MountPacket.pos, MountPacket.x, MountPacket.y);
		}
	}

	if (CPythonPlayer::Instance().IsMainCharacterIndex(MountPacket.vid))
	{
//		CPythonPlayer::Instance().SetRidingVehicleIndex(MountPacket.mount_vid);
	}

	return true;
}

bool CPythonNetworkStream::RecvChangeSpeedPacket()
{
	TPacketGCChangeSpeed SpeedPacket;

	if (!Recv(sizeof(TPacketGCChangeSpeed), &SpeedPacket))
	{
		Tracen("Recv Speed Packet Error");
		return false;
	}

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(SpeedPacket.vid);

	if (!pInstance)
		return true;

//	pInstance->SetWalkSpeed(SpeedPacket.walking_speed);
//	pInstance->SetRunSpeed(SpeedPacket.running_speed);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Recv

bool CPythonNetworkStream::SendAttackPacket(UINT uMotAttack, DWORD dwVIDVictim)
{
	if (!__CanActMainInstance())
		return true;

#ifdef ATTACK_TIME_LOG
	static DWORD prevTime = timeGetTime();
	DWORD curTime = timeGetTime();
	TraceError("TIME: %.4f(%.4f) ATTACK_PACKET: %d TARGET: %d", curTime/1000.0f, (curTime-prevTime)/1000.0f, uMotAttack, dwVIDVictim);
	prevTime = curTime;
#endif
#ifdef ENABLE_CSHIELD
	if (!__IsPlayerAttacking())
		return true;
#endif
	TPacketCGAttack kPacketAtk;

	kPacketAtk.header = HEADER_CG_ATTACK;
	kPacketAtk.bType = uMotAttack;
	kPacketAtk.dwVictimVID = dwVIDVictim;

	if (!SendSpecial(sizeof(kPacketAtk), &kPacketAtk))
	{
		Tracen("Send Battle Attack Packet Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendSpecial(int nLen, void * pvBuf)
{
	BYTE bHeader = *(BYTE *) pvBuf;

	switch (bHeader)
	{
		case HEADER_CG_ATTACK:
			{
				TPacketCGAttack * pkPacketAtk = (TPacketCGAttack *) pvBuf;
				pkPacketAtk->bCRCMagicCubeProcPiece = GetProcessCRCMagicCubePiece();
				pkPacketAtk->bCRCMagicCubeFilePiece = GetProcessCRCMagicCubePiece();
				return Send(nLen, pvBuf);
			}
			break;
	}

	return Send(nLen, pvBuf);
}

bool CPythonNetworkStream::RecvAddFlyTargetingPacket()
{
	TPacketGCFlyTargeting kPacket;
	if (!Recv(sizeof(kPacket), &kPacket))
		return false;

	__GlobalPositionToLocalPosition(kPacket.lX, kPacket.lY);

	Tracef("VID [%d] Added to target settings\n",kPacket.dwShooterVID);

	CPythonCharacterManager & rpcm = CPythonCharacterManager::Instance();

	CInstanceBase * pShooter = rpcm.GetInstancePtr(kPacket.dwShooterVID);

	if (!pShooter)
	{
#ifndef _DEBUG
		TraceError("CPythonNetworkStream::RecvFlyTargetingPacket() - dwShooterVID[%d] NOT EXIST", kPacket.dwShooterVID);
#endif
		return true;
	}

	CInstanceBase * pTarget = rpcm.GetInstancePtr(kPacket.dwTargetVID);

	if (kPacket.dwTargetVID && pTarget)
	{
		pShooter->GetGraphicThingInstancePtr()->AddFlyTarget(pTarget->GetGraphicThingInstancePtr());
	}
	else
	{
		float h = CPythonBackground::Instance().GetHeight(kPacket.lX,kPacket.lY) + 60.0f; // TEMPORARY HEIGHT
		pShooter->GetGraphicThingInstancePtr()->AddFlyTarget(D3DXVECTOR3(kPacket.lX,kPacket.lY,h));
		//pShooter->GetGraphicThingInstancePtr()->SetFlyTarget(kPacket.kPPosTarget.x,kPacket.kPPosTarget.y,);
	}

	return true;
}

bool CPythonNetworkStream::RecvFlyTargetingPacket()
{
	TPacketGCFlyTargeting kPacket;
	if (!Recv(sizeof(kPacket), &kPacket))
		return false;

	__GlobalPositionToLocalPosition(kPacket.lX, kPacket.lY);

	//Tracef("CPythonNetworkStream::RecvFlyTargetingPacket - VID [%d]\n",kPacket.dwShooterVID);

	CPythonCharacterManager & rpcm = CPythonCharacterManager::Instance();

	CInstanceBase * pShooter = rpcm.GetInstancePtr(kPacket.dwShooterVID);

	if (!pShooter)
	{
#ifdef _DEBUG
		TraceError("CPythonNetworkStream::RecvFlyTargetingPacket() - dwShooterVID[%d] NOT EXIST", kPacket.dwShooterVID);
#endif
		return true;
	}

	CInstanceBase * pTarget = rpcm.GetInstancePtr(kPacket.dwTargetVID);

	if (kPacket.dwTargetVID && pTarget)
	{
		pShooter->GetGraphicThingInstancePtr()->SetFlyTarget(pTarget->GetGraphicThingInstancePtr());
	}
	else
	{
		float h = CPythonBackground::Instance().GetHeight(kPacket.lX, kPacket.lY) + 60.0f; // TEMPORARY HEIGHT
		pShooter->GetGraphicThingInstancePtr()->SetFlyTarget(D3DXVECTOR3(kPacket.lX,kPacket.lY,h));
		//pShooter->GetGraphicThingInstancePtr()->SetFlyTarget(kPacket.kPPosTarget.x,kPacket.kPPosTarget.y,);
	}

	return true;
}

bool CPythonNetworkStream::SendShootPacket(UINT uSkill)
{
	TPacketCGShoot kPacketShoot;
	kPacketShoot.bHeader=HEADER_CG_SHOOT;
	kPacketShoot.bType=uSkill;

	if (!Send(sizeof(kPacketShoot), &kPacketShoot))
	{
		Tracen("SendShootPacket Error");
		return false;
	}

	return SendSequence();
}


#if defined(ENABLE_PARTY_MATCH)
bool CPythonNetworkStream::PartyMatch(BYTE subheader, DWORD targetPID, int mapIndex, const char* szDesc)
{
	TPacketCGPartyMatch packet;
	memset(&packet, 0, sizeof(packet)); 

	packet.Header = HEADER_CG_PARTY_MATCH;
	packet.SubHeader = subheader;
	packet.targetPID = targetPID;
	packet.mapIndex = mapIndex;
	
	if (szDesc)
	{
		// Use strncpy on the client side since strlcpy is a server-side macro
		strncpy(packet.szDesc, szDesc, sizeof(packet.szDesc) - 1);
		packet.szDesc[sizeof(packet.szDesc) - 1] = '\0';
	}
	
	if (!Send(sizeof(packet), &packet))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvPartyMatch()
{
	TPacketGCPartyMatch Packet;
	if (!Recv(sizeof(Packet), &Packet)) return false;

	Packet.szName[CHARACTER_NAME_MAX_LEN] = '\0'; 
	Packet.szDesc[51] = '\0'; // Guarantee safety

	switch (Packet.SubHeader)
	{
		case CPythonPlayer::PARTY_MATCH_SUBHEADER_GC_LIST_START:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "PartyMatchClear", Py_BuildValue("()"));
			break;
		case CPythonPlayer::PARTY_MATCH_SUBHEADER_GC_LIST_ITEM:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "PartyMatchAdd", Py_BuildValue("(iisiiiiiiiiis)", Packet.targetPID, Packet.mapIndex, Packet.szName, Packet.current_count, Packet.max_count, Packet.job, Packet.empire, Packet.level, Packet.conqueror_level, Packet.is_my_party, Packet.is_online, Packet.channel, Packet.szDesc));
			break;
		case CPythonPlayer::PARTY_MATCH_SUBHEADER_GC_ASK_JOIN:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "PartyMatchAskJoin", Py_BuildValue("(isi)", Packet.targetPID, Packet.szName, Packet.current_count));
			break;
		case CPythonPlayer::PARTY_MATCH_SUBHEADER_GC_MESSAGE:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "PartyMatchMessage", Py_BuildValue("(i)", Packet.msg));
			break;
	}
	return true;
}

#endif

bool CPythonNetworkStream::SendAddFlyTargetingPacket(DWORD dwTargetVID, const TPixelPosition & kPPosTarget)
{
	TPacketCGFlyTargeting packet;

	//CPythonCharacterManager & rpcm = CPythonCharacterManager::Instance();

	packet.bHeader	= HEADER_CG_ADD_FLY_TARGETING;
	packet.dwTargetVID = dwTargetVID;
	packet.lX = kPPosTarget.x;
	packet.lY = kPPosTarget.y;

	__LocalPositionToGlobalPosition(packet.lX, packet.lY);

	if (!Send(sizeof(packet), &packet))
	{
		Tracen("Send FlyTargeting Packet Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendFlyTargetingPacket(DWORD dwTargetVID, const TPixelPosition & kPPosTarget)
{
	TPacketCGFlyTargeting packet;

	//CPythonCharacterManager & rpcm = CPythonCharacterManager::Instance();

	packet.bHeader	= HEADER_CG_FLY_TARGETING;
	packet.dwTargetVID = dwTargetVID;
	packet.lX = kPPosTarget.x;
	packet.lY = kPPosTarget.y;

	__LocalPositionToGlobalPosition(packet.lX, packet.lY);

	if (!Send(sizeof(packet), &packet))
	{
		Tracen("Send FlyTargeting Packet Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RecvCreateFlyPacket()
{
	TPacketGCCreateFly kPacket;
	if (!Recv(sizeof(TPacketGCCreateFly), &kPacket))
		return false;

	CFlyingManager& rkFlyMgr = CFlyingManager::Instance();
	CPythonCharacterManager & rkChrMgr = CPythonCharacterManager::Instance();

	CInstanceBase * pkStartInst = rkChrMgr.GetInstancePtr(kPacket.dwStartVID);
	CInstanceBase * pkEndInst = rkChrMgr.GetInstancePtr(kPacket.dwEndVID);
	if (!pkStartInst || !pkEndInst)
		return true;

	rkFlyMgr.CreateIndexedFly(kPacket.bType, pkStartInst->GetGraphicThingInstancePtr(), pkEndInst->GetGraphicThingInstancePtr());

	return true;
}

bool CPythonNetworkStream::SendTargetPacket(DWORD dwVID)
{
	TPacketCGTarget packet;
	packet.header = HEADER_CG_TARGET;
	packet.dwVID = dwVID;

	if (!Send(sizeof(packet), &packet))
	{
		Tracen("Send Target Packet Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendSyncPositionElementPacket(DWORD dwVictimVID, DWORD dwVictimX, DWORD dwVictimY)
{
	TPacketCGSyncPositionElement kSyncPos;
	kSyncPos.dwVID=dwVictimVID;
	kSyncPos.lX=dwVictimX;
	kSyncPos.lY=dwVictimY;

	__LocalPositionToGlobalPosition(kSyncPos.lX, kSyncPos.lY);

	if (!Send(sizeof(kSyncPos), &kSyncPos))
	{
		Tracen("CPythonNetworkStream::SendSyncPositionElementPacket - ERROR");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::RecvMessenger()
{
	TPacketGCMessenger p;
	if (!Recv(sizeof(p), &p))
		return false;

	int iSize = p.size - sizeof(p);
	char char_name[24 + 1];

#if defined(ENABLE_MESSENGER_RENEWAL)
	if (p.subheader == MESSENGER_SUBHEADER_GC_MY_STATUS_MESSAGE)
	{
		if (iSize < static_cast<int>(sizeof(TPacketGCMessengerMyStatusMessage)))
			return false;

		TPacketGCMessengerMyStatusMessage StatusPacket;
		if (!Recv(sizeof(StatusPacket), &StatusPacket))
			return false;

		StatusPacket.szStatusMessage[MESSENGER_STATUS_MESSAGE_MAX_LEN] = '\0';
		CPythonCommunity::Instance().NotifyMyStatusMessage(StatusPacket.szStatusMessage);
		return true;
	}

	if (p.subheader == MESSENGER_SUBHEADER_GC_STATUS_MESSAGE)
	{
		while (iSize >= static_cast<int>(sizeof(TPacketGCMessengerStatusMessage)))
		{
			TPacketGCMessengerStatusMessage StatusPacket;
			if (!Recv(sizeof(StatusPacket), &StatusPacket))
				return false;

			StatusPacket.szName[CHARACTER_NAME_MAX_LEN] = '\0';
			StatusPacket.szStatusMessage[MESSENGER_STATUS_MESSAGE_MAX_LEN] = '\0';

			iSize -= sizeof(StatusPacket);
			CPythonCommunity::Instance().OnFriendStatusMessage(
				StatusPacket.szName, StatusPacket.szStatusMessage);
		}
		return true;
	}
#endif

	switch (p.subheader)
	{
		case MESSENGER_SUBHEADER_GC_LIST:
		{
#if defined(ENABLE_MESSENGER_RENEWAL)
			TPacketGCMessengerList ListPacket;
			while (iSize >= static_cast<int>(sizeof(TPacketGCMessengerList)))
			{
				if (!Recv(sizeof(TPacketGCMessengerList), &ListPacket))
					return false;

				ListPacket.szName[CHARACTER_NAME_MAX_LEN] = '\0';
				ListPacket.szStatusMessage[MESSENGER_STATUS_MESSAGE_MAX_LEN] = '\0';

				if (ListPacket.bConnected & MESSENGER_CONNECTED_STATE_ONLINE)
				{
					CPythonMessenger::Instance().OnFriendLogin(ListPacket.szName);
					CPythonCommunity::Instance().OnFriendRenewalDetails(
						ListPacket.szName,
						ListPacket.bLevel,
						ListPacket.bIsConqueror,
						ListPacket.bChannel,
						ListPacket.lMapIndex);
				}
				else
				{
					CPythonMessenger::Instance().OnFriendLogout(ListPacket.szName);
				}

				CPythonCommunity::Instance().OnFriendConnectionState(
					ListPacket.szName, ListPacket.bConnectionState);
				CPythonCommunity::Instance().OnFriendStatusMessage(
					ListPacket.szName, ListPacket.szStatusMessage);

				iSize -= sizeof(TPacketGCMessengerList);
			}
#else
			TPacketGCMessengerListOnline on;
			while(iSize)
			{
				if (!Recv(sizeof(TPacketGCMessengerListOffline),&on))
					return false;

				if (!Recv(on.length, char_name))
					return false;

				char_name[on.length] = 0;

				if (on.connected & MESSENGER_CONNECTED_STATE_ONLINE)
					CPythonMessenger::Instance().OnFriendLogin(char_name);
				else
					CPythonMessenger::Instance().OnFriendLogout(char_name);

				iSize -= sizeof(TPacketGCMessengerListOffline);
				iSize -= on.length;
			}
#endif
			break;
		}
		#ifdef ENABLE_MESSENGER_BLOCK
		case MESSENGER_SUBHEADER_GC_BLOCK_LIST:
		{
			TPacketGCMessengerBlockListOnline onn;
			while(iSize)
			{
				if (!Recv(sizeof(TPacketGCMessengerBlockListOffline),&onn))
					return false;

				if (!Recv(onn.length, char_name))
					return false;

				char_name[onn.length] = 0;

				if (onn.connected & MESSENGER_CONNECTED_STATE_ONLINE)
					CPythonMessenger::Instance().OnBlockLogin(char_name);
				else
					CPythonMessenger::Instance().OnBlockLogout(char_name);

				iSize -= sizeof(TPacketGCMessengerBlockListOffline);
				iSize -= onn.length;
			}
			break;
		}
		
		case MESSENGER_SUBHEADER_GC_BLOCK_LOGIN:
		{
			TPacketGCMessengerLogin pp;
			if (!Recv(sizeof(pp),&pp))
				return false;
			if (!Recv(pp.length, char_name))
				return false;
			char_name[pp.length] = 0;
			CPythonMessenger::Instance().OnBlockLogin(char_name);
			__RefreshTargetBoardByName(char_name);
			break;
		}

		case MESSENGER_SUBHEADER_GC_BLOCK_LOGOUT:
		{
			TPacketGCMessengerLogout logout2;
			if (!Recv(sizeof(logout2),&logout2))
				return false;
			if (!Recv(logout2.length, char_name))
				return false;
			char_name[logout2.length] = 0;
			CPythonMessenger::Instance().OnBlockLogout(char_name);
			break;
		}
		#endif
		case MESSENGER_SUBHEADER_GC_LOGIN:
		{
#if defined(ENABLE_MESSENGER_RENEWAL)
			TPacketGCMessengerList ListPacket;
			if (!Recv(sizeof(TPacketGCMessengerList), &ListPacket))
				return false;

			ListPacket.szName[CHARACTER_NAME_MAX_LEN] = '\0';
			ListPacket.szStatusMessage[MESSENGER_STATUS_MESSAGE_MAX_LEN] = '\0';

			CPythonMessenger::Instance().OnFriendLogin(ListPacket.szName);
			if (ListPacket.bConnected & MESSENGER_CONNECTED_STATE_ONLINE)
			{
				CPythonCommunity::Instance().OnFriendRenewalDetails(
					ListPacket.szName,
					ListPacket.bLevel,
					ListPacket.bIsConqueror,
					ListPacket.bChannel,
					ListPacket.lMapIndex);
			}
			CPythonCommunity::Instance().OnFriendConnectionState(
				ListPacket.szName, ListPacket.bConnectionState);
			CPythonCommunity::Instance().OnFriendStatusMessage(
				ListPacket.szName, ListPacket.szStatusMessage);
			__RefreshTargetBoardByName(ListPacket.szName);
#else
			TPacketGCMessengerLogin p;
			if (!Recv(sizeof(p),&p))
				return false;
			if (!Recv(p.length, char_name))
				return false;
			char_name[p.length] = 0;
			CPythonMessenger::Instance().OnFriendLogin(char_name);
			__RefreshTargetBoardByName(char_name);
#endif
			break;
		}

		case MESSENGER_SUBHEADER_GC_LOGOUT:
		{
			TPacketGCMessengerLogout logout;
			if (!Recv(sizeof(logout),&logout))
				return false;
			if (!Recv(logout.length, char_name))
				return false;
			char_name[logout.length] = 0;
			CPythonMessenger::Instance().OnFriendLogout(char_name);
#if defined(ENABLE_MESSENGER_RENEWAL)
			CPythonCommunity::Instance().OnFriendConnectionState(char_name, MESSENGER_CONNECTION_STATE_DISCONNECT);
#endif
			break;
		}

#if defined(ENABLE_MESSENGER_RENEWAL)
		case MESSENGER_SUBHEADER_GC_CONNECTION_STATE:
		{
			TPacketGCMessengerList ListPacket;
			if (!Recv(sizeof(TPacketGCMessengerList), &ListPacket))
				return false;

			ListPacket.szName[CHARACTER_NAME_MAX_LEN] = '\0';
			ListPacket.szStatusMessage[MESSENGER_STATUS_MESSAGE_MAX_LEN] = '\0';

			if (ListPacket.bConnected & MESSENGER_CONNECTED_STATE_ONLINE)
			{
				CPythonMessenger::Instance().OnFriendLogin(ListPacket.szName);
				CPythonCommunity::Instance().OnFriendRenewalDetails(
					ListPacket.szName,
					ListPacket.bLevel,
					ListPacket.bIsConqueror,
					ListPacket.bChannel,
					ListPacket.lMapIndex);
			}
			else
			{
				CPythonMessenger::Instance().OnFriendLogout(ListPacket.szName);
			}

			CPythonCommunity::Instance().OnFriendConnectionState(
				ListPacket.szName, ListPacket.bConnectionState);
			CPythonCommunity::Instance().OnFriendStatusMessage(
				ListPacket.szName, ListPacket.szStatusMessage);
			break;
		}
#endif
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
// Party

bool CPythonNetworkStream::SendPartyInvitePacket(DWORD dwVID)
{
	TPacketCGPartyInvite kPartyInvitePacket;
	kPartyInvitePacket.header = HEADER_CG_PARTY_INVITE;
	kPartyInvitePacket.vid = dwVID;

	if (!Send(sizeof(kPartyInvitePacket), &kPartyInvitePacket))
	{
		Tracenf("CPythonNetworkStream::SendPartyInvitePacket [%ud] - PACKET SEND ERROR", dwVID);
		return false;
	}

	Tracef(" << SendPartyInvitePacket : %d\n", dwVID);
	return SendSequence();
}

bool CPythonNetworkStream::SendPartyInviteAnswerPacket(DWORD dwLeaderVID, BYTE byAnswer)
{
	TPacketCGPartyInviteAnswer kPartyInviteAnswerPacket;
	kPartyInviteAnswerPacket.header = HEADER_CG_PARTY_INVITE_ANSWER;
	kPartyInviteAnswerPacket.leader_pid = dwLeaderVID;
	kPartyInviteAnswerPacket.accept = byAnswer;

	if (!Send(sizeof(kPartyInviteAnswerPacket), &kPartyInviteAnswerPacket))
	{
		Tracenf("CPythonNetworkStream::SendPartyInviteAnswerPacket [%ud %ud] - PACKET SEND ERROR", dwLeaderVID, byAnswer);
		return false;
	}

	Tracef(" << SendPartyInviteAnswerPacket : %d, %d\n", dwLeaderVID, byAnswer);
	return SendSequence();
}

bool CPythonNetworkStream::SendPartyRemovePacket(DWORD dwPID)
{
	TPacketCGPartyRemove kPartyInviteRemove;
	kPartyInviteRemove.header = HEADER_CG_PARTY_REMOVE;
	kPartyInviteRemove.pid = dwPID;

	if (!Send(sizeof(kPartyInviteRemove), &kPartyInviteRemove))
	{
		Tracenf("CPythonNetworkStream::SendPartyRemovePacket [%ud] - PACKET SEND ERROR", dwPID);
		return false;
	}

	Tracef(" << SendPartyRemovePacket : %d\n", dwPID);
	return SendSequence();
}

bool CPythonNetworkStream::SendPartySetStatePacket(DWORD dwVID, BYTE byState, BYTE byFlag)
{
	TPacketCGPartySetState kPartySetState;
	kPartySetState.byHeader = HEADER_CG_PARTY_SET_STATE;
	kPartySetState.dwVID = dwVID;
	kPartySetState.byState = byState;
	kPartySetState.byFlag = byFlag;

	if (!Send(sizeof(kPartySetState), &kPartySetState))
	{
		Tracenf("CPythonNetworkStream::SendPartySetStatePacket(%ud, %ud) - PACKET SEND ERROR", dwVID, byState);
		return false;
	}

	Tracef(" << SendPartySetStatePacket : %d, %d, %d\n", dwVID, byState, byFlag);
	return SendSequence();
}

bool CPythonNetworkStream::SendPartyUseSkillPacket(BYTE bySkillIndex, DWORD dwVID)
{
	TPacketCGPartyUseSkill kPartyUseSkill;
	kPartyUseSkill.byHeader = HEADER_CG_PARTY_USE_SKILL;
	kPartyUseSkill.bySkillIndex = bySkillIndex;
	kPartyUseSkill.dwTargetVID = dwVID;

	if (!Send(sizeof(kPartyUseSkill), &kPartyUseSkill))
	{
		Tracenf("CPythonNetworkStream::SendPartyUseSkillPacket(%ud, %ud) - PACKET SEND ERROR", bySkillIndex, dwVID);
		return false;
	}

	Tracef(" << SendPartyUseSkillPacket : %d, %d\n", bySkillIndex, dwVID);
	return SendSequence();
}

bool CPythonNetworkStream::SendPartyParameterPacket(BYTE byDistributeMode)
{
	TPacketCGPartyParameter kPartyParameter;
	kPartyParameter.bHeader = HEADER_CG_PARTY_PARAMETER;
	kPartyParameter.bDistributeMode = byDistributeMode;

	if (!Send(sizeof(kPartyParameter), &kPartyParameter))
	{
		Tracenf("CPythonNetworkStream::SendPartyParameterPacket(%d) - PACKET SEND ERROR", byDistributeMode);
		return false;
	}

	Tracef(" << SendPartyParameterPacket : %d\n", byDistributeMode);
	return SendSequence();
}

bool CPythonNetworkStream::RecvPartyInvite()
{
	TPacketGCPartyInvite kPartyInvitePacket;
	if (!Recv(sizeof(kPartyInvitePacket), &kPartyInvitePacket))
		return false;

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(kPartyInvitePacket.leader_pid);
	if (!pInstance)
	{
		TraceError(" CPythonNetworkStream::RecvPartyInvite - Failed to find leader instance [%d]\n", kPartyInvitePacket.leader_pid);
		return true;
	}

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RecvPartyInviteQuestion", Py_BuildValue("(is)", kPartyInvitePacket.leader_pid, pInstance->GetNameString()));
	Tracef(" >> RecvPartyInvite : %d, %s\n", kPartyInvitePacket.leader_pid, pInstance->GetNameString());

	return true;
}

bool CPythonNetworkStream::RecvPartyAdd()
{
	TPacketGCPartyAdd kPartyAddPacket;
	if (!Recv(sizeof(kPartyAddPacket), &kPartyAddPacket))
		return false;

	CPythonPlayer::Instance().AppendPartyMember(kPartyAddPacket.pid, kPartyAddPacket.name);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AddPartyMember", Py_BuildValue("(is)", kPartyAddPacket.pid, kPartyAddPacket.name));
	Tracef(" >> RecvPartyAdd : %d, %s\n", kPartyAddPacket.pid, kPartyAddPacket.name);

	return true;
}

bool CPythonNetworkStream::RecvPartyUpdate()
{
	TPacketGCPartyUpdate kPartyUpdatePacket;
	if (!Recv(sizeof(kPartyUpdatePacket), &kPartyUpdatePacket))
		return false;

	CPythonPlayer::TPartyMemberInfo * pPartyMemberInfo;
	if (!CPythonPlayer::Instance().GetPartyMemberPtr(kPartyUpdatePacket.pid, &pPartyMemberInfo))
		return true;

	BYTE byOldState = pPartyMemberInfo->byState;

	CPythonPlayer::Instance().UpdatePartyMemberInfo(kPartyUpdatePacket.pid, kPartyUpdatePacket.state, kPartyUpdatePacket.percent_hp);
	for (int i = 0; i < PARTY_AFFECT_SLOT_MAX_NUM; ++i)
	{
		CPythonPlayer::Instance().UpdatePartyMemberAffect(kPartyUpdatePacket.pid, i, kPartyUpdatePacket.affects[i]);
	}

#if defined(__BL_PARTY_POSITION__)
	if (kPartyUpdatePacket.x || kPartyUpdatePacket.y)
		__GlobalPositionToLocalPosition(kPartyUpdatePacket.x, kPartyUpdatePacket.y);

	CPythonPlayer::Instance().UpdatePartyMemberPosition(kPartyUpdatePacket.pid, kPartyUpdatePacket.x, kPartyUpdatePacket.y);
#endif

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "UpdatePartyMemberInfo", Py_BuildValue("(i)", kPartyUpdatePacket.pid));

	DWORD dwVID;
	if (CPythonPlayer::Instance().PartyMemberPIDToVID(kPartyUpdatePacket.pid, &dwVID))
	if (byOldState != kPartyUpdatePacket.state)
	{
		__RefreshTargetBoardByVID(dwVID);
	}

// 	Tracef(" >> RecvPartyUpdate : %d, %d, %d\n", kPartyUpdatePacket.pid, kPartyUpdatePacket.state, kPartyUpdatePacket.percent_hp);

	return true;
}

bool CPythonNetworkStream::RecvPartyRemove()
{
	TPacketGCPartyRemove kPartyRemovePacket;
	if (!Recv(sizeof(kPartyRemovePacket), &kPartyRemovePacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RemovePartyMember", Py_BuildValue("(i)", kPartyRemovePacket.pid));
	Tracef(" >> RecvPartyRemove : %d\n", kPartyRemovePacket.pid);

	return true;
}

bool CPythonNetworkStream::RecvPartyLink()
{
	TPacketGCPartyLink kPartyLinkPacket;
	if (!Recv(sizeof(kPartyLinkPacket), &kPartyLinkPacket))
		return false;

	CPythonPlayer::Instance().LinkPartyMember(kPartyLinkPacket.pid, kPartyLinkPacket.vid);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "LinkPartyMember", Py_BuildValue("(ii)", kPartyLinkPacket.pid, kPartyLinkPacket.vid));
	Tracef(" >> RecvPartyLink : %d, %d\n", kPartyLinkPacket.pid, kPartyLinkPacket.vid);

	return true;
}

bool CPythonNetworkStream::RecvPartyUnlink()
{
	TPacketGCPartyUnlink kPartyUnlinkPacket;
	if (!Recv(sizeof(kPartyUnlinkPacket), &kPartyUnlinkPacket))
		return false;

	CPythonPlayer::Instance().UnlinkPartyMember(kPartyUnlinkPacket.pid);

	if (CPythonPlayer::Instance().IsMainCharacterIndex(kPartyUnlinkPacket.vid))
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "UnlinkAllPartyMember", Py_BuildValue("()"));
	}
	else
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "UnlinkPartyMember", Py_BuildValue("(i)", kPartyUnlinkPacket.pid));
	}

	Tracef(" >> RecvPartyUnlink : %d, %d\n", kPartyUnlinkPacket.pid, kPartyUnlinkPacket.vid);

	return true;
}

bool CPythonNetworkStream::RecvPartyParameter()
{
	TPacketGCPartyParameter kPartyParameterPacket;
	if (!Recv(sizeof(kPartyParameterPacket), &kPartyParameterPacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ChangePartyParameter", Py_BuildValue("(i)", kPartyParameterPacket.bDistributeMode));
	Tracef(" >> RecvPartyParameter : %d\n", kPartyParameterPacket.bDistributeMode);

	return true;
}

// Party
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Guild

bool CPythonNetworkStream::SendGuildAddMemberPacket(DWORD dwVID)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_ADD_MEMBER;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(dwVID), &dwVID))
		return false;

	Tracef(" SendGuildAddMemberPacket\n", dwVID);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildRemoveMemberPacket(DWORD dwPID)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_REMOVE_MEMBER;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(dwPID), &dwPID))
		return false;

	Tracef(" SendGuildRemoveMemberPacket %d\n", dwPID);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildChangeGradeNamePacket(BYTE byGradeNumber, const char * c_szName)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_CHANGE_GRADE_NAME;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(byGradeNumber), &byGradeNumber))
		return false;

	char szName[GUILD_GRADE_NAME_MAX_LEN+1];
	strncpy(szName, c_szName, GUILD_GRADE_NAME_MAX_LEN);
	szName[GUILD_GRADE_NAME_MAX_LEN] = '\0';

	if (!Send(sizeof(szName), &szName))
		return false;

	Tracef(" SendGuildChangeGradeNamePacket %d, %s\n", byGradeNumber, c_szName);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildChangeGradeAuthorityPacket(BYTE byGradeNumber, BYTE byAuthority)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_CHANGE_GRADE_AUTHORITY;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(byGradeNumber), &byGradeNumber))
		return false;
	if (!Send(sizeof(byAuthority), &byAuthority))
		return false;

	Tracef(" SendGuildChangeGradeAuthorityPacket %d, %d\n", byGradeNumber, byAuthority);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildOfferPacket(DWORD dwExperience)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_OFFER;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(dwExperience), &dwExperience))
		return false;

	Tracef(" SendGuildOfferPacket %d\n", dwExperience);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildPostCommentPacket(const char * c_szMessage)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_POST_COMMENT;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	BYTE bySize = BYTE(strlen(c_szMessage)) + 1;
	if (!Send(sizeof(bySize), &bySize))
		return false;
	if (!Send(bySize, c_szMessage))
		return false;

	Tracef(" SendGuildPostCommentPacket %d, %s\n", bySize, c_szMessage);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildDeleteCommentPacket(DWORD dwIndex)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_DELETE_COMMENT;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwIndex), &dwIndex))
		return false;

	Tracef(" SendGuildDeleteCommentPacket %d\n", dwIndex);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildRefreshCommentsPacket(DWORD dwHighestIndex)
{
	static DWORD s_LastTime = timeGetTime() - 1001;

	if (timeGetTime() - s_LastTime < 1000)
		return true;
	s_LastTime = timeGetTime();

	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_REFRESH_COMMENT;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	Tracef(" SendGuildRefreshCommentPacket %d\n", dwHighestIndex);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildChangeMemberGradePacket(DWORD dwPID, BYTE byGrade)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_CHANGE_MEMBER_GRADE;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwPID), &dwPID))
		return false;
	if (!Send(sizeof(byGrade), &byGrade))
		return false;

	Tracef(" SendGuildChangeMemberGradePacket %d, %d\n", dwPID, byGrade);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildUseSkillPacket(DWORD dwSkillID, DWORD dwTargetVID)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_USE_SKILL;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwSkillID), &dwSkillID))
		return false;
	if (!Send(sizeof(dwTargetVID), &dwTargetVID))
		return false;

	Tracef(" SendGuildUseSkillPacket %d, %d\n", dwSkillID, dwTargetVID);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildChangeMemberGeneralPacket(DWORD dwPID, BYTE byFlag)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_CHANGE_MEMBER_GENERAL;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwPID), &dwPID))
		return false;
	if (!Send(sizeof(byFlag), &byFlag))
		return false;

	Tracef(" SendGuildChangeMemberGeneralFlagPacket %d, %d\n", dwPID, byFlag);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildInviteAnswerPacket(DWORD dwGuildID, BYTE byAnswer)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_GUILD_INVITE_ANSWER;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwGuildID), &dwGuildID))
		return false;
	if (!Send(sizeof(byAnswer), &byAnswer))
		return false;

	Tracef(" SendGuildInviteAnswerPacket %d, %d\n", dwGuildID, byAnswer);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildChargeGSPPacket(DWORD dwMoney)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_CHARGE_GSP;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;

	if (!Send(sizeof(dwMoney), &dwMoney))
		return false;

	Tracef(" SendGuildChargeGSPPacket %d\n", dwMoney);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildDepositMoneyPacket(DWORD dwMoney)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_DEPOSIT_MONEY;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(dwMoney), &dwMoney))
		return false;

	Tracef(" SendGuildDepositMoneyPacket %d\n", dwMoney);
	return SendSequence();
}

bool CPythonNetworkStream::SendGuildWithdrawMoneyPacket(DWORD dwMoney)
{
	TPacketCGGuild GuildPacket;
	GuildPacket.byHeader = HEADER_CG_GUILD;
	GuildPacket.bySubHeader = GUILD_SUBHEADER_CG_WITHDRAW_MONEY;
	if (!Send(sizeof(GuildPacket), &GuildPacket))
		return false;
	if (!Send(sizeof(dwMoney), &dwMoney))
		return false;

	Tracef(" SendGuildWithdrawMoneyPacket %d\n", dwMoney);
	return SendSequence();
}

bool CPythonNetworkStream::RecvGuild()
{
    TPacketGCGuild GuildPacket;
	if (!Recv(sizeof(GuildPacket), &GuildPacket))
		return false;

	switch(GuildPacket.subheader)
	{
		case GUILD_SUBHEADER_GC_LOGIN:
		{
			DWORD dwPID;
			if (!Recv(sizeof(DWORD), &dwPID))
				return false;

			// Messenger
			CPythonGuild::TGuildMemberData * pGuildMemberData;
			if (CPythonGuild::Instance().GetMemberDataPtrByPID(dwPID, &pGuildMemberData))
				if (0 != pGuildMemberData->strName.compare(CPythonPlayer::Instance().GetName()))
					CPythonMessenger::Instance().LoginGuildMember(pGuildMemberData->strName.c_str());

			//Tracef(" <Login> %d\n", dwPID);
			break;
		}
		case GUILD_SUBHEADER_GC_LOGOUT:
		{
			DWORD dwPID;
			if (!Recv(sizeof(DWORD), &dwPID))
				return false;

			// Messenger
			CPythonGuild::TGuildMemberData * pGuildMemberData;
			if (CPythonGuild::Instance().GetMemberDataPtrByPID(dwPID, &pGuildMemberData))
				if (0 != pGuildMemberData->strName.compare(CPythonPlayer::Instance().GetName()))
					CPythonMessenger::Instance().LogoutGuildMember(pGuildMemberData->strName.c_str());

			//Tracef(" <Logout> %d\n", dwPID);
			break;
		}
		case GUILD_SUBHEADER_GC_REMOVE:
		{
			DWORD dwPID;
			if (!Recv(sizeof(dwPID), &dwPID))
				return false;

			if (CPythonGuild::Instance().IsMainPlayer(dwPID))
			{
				CPythonGuild::Instance().Destroy();
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "DeleteGuild", Py_BuildValue("()"));
				CPythonMessenger::Instance().RemoveAllGuildMember();
				__SetGuildID(0);
				#ifdef ENABLE_GUILD_TOKEN_AUTH
				__SetGuildToken(0);
				#endif
				__RefreshMessengerWindow();
				__RefreshTargetBoard();
				__RefreshCharacterWindow();
			}
			else
			{
				// Get Member Name
				std::string strMemberName = "";
				CPythonGuild::TGuildMemberData * pData;
				if (CPythonGuild::Instance().GetMemberDataPtrByPID(dwPID, &pData))
				{
					strMemberName = pData->strName;
					CPythonMessenger::Instance().RemoveGuildMember(pData->strName.c_str());
				}

				CPythonGuild::Instance().RemoveMember(dwPID);

				// Refresh
				__RefreshTargetBoardByName(strMemberName.c_str());
				__RefreshGuildWindowMemberPage();
			}

			Tracef(" <Remove> %d\n", dwPID);
			break;
		}
		case GUILD_SUBHEADER_GC_LIST:
		{
			int iPacketSize = int(GuildPacket.size) - sizeof(GuildPacket);

			for (; iPacketSize > 0;)
			{
				TPacketGCGuildSubMember memberPacket;
				if (!Recv(sizeof(memberPacket), &memberPacket))
					return false;

				char szName[CHARACTER_NAME_MAX_LEN+1] = "";
				if (memberPacket.byNameFlag)
				{
					if (!Recv(sizeof(szName), &szName))
						return false;

					iPacketSize -= CHARACTER_NAME_MAX_LEN+1;
				}
				else
				{
					CPythonGuild::TGuildMemberData * pMemberData;
					if (CPythonGuild::Instance().GetMemberDataPtrByPID(memberPacket.pid, &pMemberData))
					{
						strncpy(szName, pMemberData->strName.c_str(), CHARACTER_NAME_MAX_LEN);
					}
				}

				//Tracef(" <List> %d : %s, %d (%d, %d, %d)\n", memberPacket.pid, szName, memberPacket.byGrade, memberPacket.byJob, memberPacket.byLevel, memberPacket.dwOffer);

				CPythonGuild::SGuildMemberData GuildMemberData;
				GuildMemberData.dwPID = memberPacket.pid;
				GuildMemberData.byGrade = memberPacket.byGrade;
				GuildMemberData.strName = szName;
				GuildMemberData.byJob = memberPacket.byJob;
				GuildMemberData.byLevel = memberPacket.byLevel;
				GuildMemberData.dwOffer = memberPacket.dwOffer;
				GuildMemberData.byGeneralFlag = memberPacket.byIsGeneral;
				CPythonGuild::Instance().RegisterMember(GuildMemberData);

				// Messenger
				if (strcmp(szName, CPythonPlayer::Instance().GetName()))
					CPythonMessenger::Instance().AppendGuildMember(szName);

				__RefreshTargetBoardByName(szName);

				iPacketSize -= sizeof(memberPacket);
			}

			__RefreshGuildWindowInfoPage();
			__RefreshGuildWindowMemberPage();
			__RefreshMessengerWindow();
			__RefreshCharacterWindow();
			break;
		}
		case GUILD_SUBHEADER_GC_GRADE:
		{
			BYTE byCount;
			if (!Recv(sizeof(byCount), &byCount))
				return false;

			for (BYTE i = 0; i < byCount; ++ i)
			{
				BYTE byIndex;
				if (!Recv(sizeof(byCount), &byIndex))
					return false;
				TPacketGCGuildSubGrade GradePacket;
				if (!Recv(sizeof(GradePacket), &GradePacket))
					return false;

				CPythonGuild::Instance().SetGradeData(byIndex, CPythonGuild::SGuildGradeData(GradePacket.auth_flag, GradePacket.grade_name));
				//Tracef(" <Grade> [%d/%d] : %s, %d\n", byIndex, byCount, GradePacket.grade_name, GradePacket.auth_flag);
			}
			__RefreshGuildWindowGradePage();
			__RefreshGuildWindowMemberPageGradeComboBox();
			break;
		}
		case GUILD_SUBHEADER_GC_GRADE_NAME:
		{
			BYTE byGradeNumber;
			if (!Recv(sizeof(byGradeNumber), &byGradeNumber))
				return false;

			char szGradeName[GUILD_GRADE_NAME_MAX_LEN+1] = "";
			if (!Recv(sizeof(szGradeName), &szGradeName))
				return false;

			CPythonGuild::Instance().SetGradeName(byGradeNumber, szGradeName);
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildGrade", Py_BuildValue("()"));

			Tracef(" <Change Grade Name> %d, %s\n", byGradeNumber, szGradeName);
			__RefreshGuildWindowGradePage();
			__RefreshGuildWindowMemberPageGradeComboBox();
			break;
		}
		case GUILD_SUBHEADER_GC_GRADE_AUTH:
		{
			BYTE byGradeNumber;
			if (!Recv(sizeof(byGradeNumber), &byGradeNumber))
				return false;
			BYTE byAuthorityFlag;
			if (!Recv(sizeof(byAuthorityFlag), &byAuthorityFlag))
				return false;

			CPythonGuild::Instance().SetGradeAuthority(byGradeNumber, byAuthorityFlag);
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGuildGrade", Py_BuildValue("()"));

			Tracef(" <Change Grade Authority> %d, %d\n", byGradeNumber, byAuthorityFlag);
			__RefreshGuildWindowGradePage();
			break;
		}
		case GUILD_SUBHEADER_GC_INFO:
		{
			TPacketGCGuildInfo GuildInfo;
			if (!Recv(sizeof(GuildInfo), &GuildInfo))
				return false;

			CPythonGuild::Instance().EnableGuild();
			CPythonGuild::TGuildInfo & rGuildInfo = CPythonGuild::Instance().GetGuildInfoRef();
			strncpy(rGuildInfo.szGuildName, GuildInfo.name, GUILD_NAME_MAX_LEN);
			rGuildInfo.szGuildName[GUILD_NAME_MAX_LEN] = '\0';

			rGuildInfo.dwGuildID = GuildInfo.guild_id;
			rGuildInfo.dwMasterPID = GuildInfo.master_pid;
			rGuildInfo.dwGuildLevel = GuildInfo.level;
			rGuildInfo.dwCurrentExperience = GuildInfo.exp;
			rGuildInfo.dwCurrentMemberCount = GuildInfo.member_count;
			rGuildInfo.dwMaxMemberCount = GuildInfo.max_member_count;
			rGuildInfo.dwGuildMoney = GuildInfo.gold;
			rGuildInfo.bHasLand = GuildInfo.hasLand;

			//Tracef(" <Info> %s, %d, %d : %d\n", GuildInfo.name, GuildInfo.master_pid, GuildInfo.level, rGuildInfo.bHasLand);
			__RefreshGuildWindowInfoPage();
			break;
		}
		case GUILD_SUBHEADER_GC_COMMENTS:
		{
			BYTE byCount;
			if (!Recv(sizeof(byCount), &byCount))
				return false;

			CPythonGuild::Instance().ClearComment();
			//Tracef(" >>> Comments Count : %d\n", byCount);

			for (BYTE i = 0; i < byCount; ++i)
			{
				DWORD dwCommentID;
				if (!Recv(sizeof(dwCommentID), &dwCommentID))
					return false;

				char szName[CHARACTER_NAME_MAX_LEN+1] = "";
				if (!Recv(sizeof(szName), &szName))
					return false;

				char szComment[GULID_COMMENT_MAX_LEN+1] = "";
				if (!Recv(sizeof(szComment), &szComment))
					return false;

				//Tracef(" [Comment-%d] : %s, %s\n", dwCommentID, szName, szComment);
				CPythonGuild::Instance().RegisterComment(dwCommentID, szName, szComment);
			}

			__RefreshGuildWindowBoardPage();
			break;
		}
		case GUILD_SUBHEADER_GC_CHANGE_EXP:
		{
			BYTE byLevel;
			if (!Recv(sizeof(byLevel), &byLevel))
				return false;
			DWORD dwEXP;
			if (!Recv(sizeof(dwEXP), &dwEXP))
				return false;
			CPythonGuild::Instance().SetGuildEXP(byLevel, dwEXP);
			Tracef(" <ChangeEXP> %d, %d\n", byLevel, dwEXP);
			__RefreshGuildWindowInfoPage();
			break;
		}
		case GUILD_SUBHEADER_GC_CHANGE_MEMBER_GRADE:
		{
			DWORD dwPID;
			if (!Recv(sizeof(dwPID), &dwPID))
				return false;
			BYTE byGrade;
			if (!Recv(sizeof(byGrade), &byGrade))
				return false;
			CPythonGuild::Instance().ChangeGuildMemberGrade(dwPID, byGrade);
			Tracef(" <ChangeMemberGrade> %d, %d\n", dwPID, byGrade);
			__RefreshGuildWindowMemberPage();
			break;
		}
		case GUILD_SUBHEADER_GC_SKILL_INFO:
		{
			CPythonGuild::TGuildSkillData & rSkillData = CPythonGuild::Instance().GetGuildSkillDataRef();
			if (!Recv(sizeof(rSkillData.bySkillPoint), &rSkillData.bySkillPoint))
				return false;
			if (!Recv(sizeof(rSkillData.bySkillLevel), rSkillData.bySkillLevel))
				return false;
			if (!Recv(sizeof(rSkillData.wGuildPoint), &rSkillData.wGuildPoint))
				return false;
			if (!Recv(sizeof(rSkillData.wMaxGuildPoint), &rSkillData.wMaxGuildPoint))
				return false;

			Tracef(" <SkillInfo> %d / %d, %d\n", rSkillData.bySkillPoint, rSkillData.wGuildPoint, rSkillData.wMaxGuildPoint);
			__RefreshGuildWindowSkillPage();
			break;
		}
		case GUILD_SUBHEADER_GC_CHANGE_MEMBER_GENERAL:
		{
			DWORD dwPID;
			if (!Recv(sizeof(dwPID), &dwPID))
				return false;
			BYTE byFlag;
			if (!Recv(sizeof(byFlag), &byFlag))
				return false;

			CPythonGuild::Instance().ChangeGuildMemberGeneralFlag(dwPID, byFlag);
			Tracef(" <ChangeMemberGeneralFlag> %d, %d\n", dwPID, byFlag);
			__RefreshGuildWindowMemberPage();
			break;
		}
		case GUILD_SUBHEADER_GC_GUILD_INVITE:
		{
			DWORD dwGuildID;
			if (!Recv(sizeof(dwGuildID), &dwGuildID))
				return false;
			char szGuildName[GUILD_NAME_MAX_LEN+1];
			if (!Recv(GUILD_NAME_MAX_LEN, &szGuildName))
				return false;

			szGuildName[GUILD_NAME_MAX_LEN] = 0;

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RecvGuildInviteQuestion", Py_BuildValue("(is)", dwGuildID, szGuildName));
			Tracef(" <Guild Invite> %d, %s\n", dwGuildID, szGuildName);
			break;
		}
		case GUILD_SUBHEADER_GC_WAR:
		{
			TPacketGCGuildWar kGuildWar;
			if (!Recv(sizeof(kGuildWar), &kGuildWar))
				return false;

			switch (kGuildWar.bWarState)
			{
				case GUILD_WAR_SEND_DECLARE:
					Tracef(" >> GUILD_SUBHEADER_GC_WAR : GUILD_WAR_SEND_DECLARE\n");
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
						"BINARY_GuildWar_OnSendDeclare",
						Py_BuildValue("(i)", kGuildWar.dwGuildOpp)
					);
					break;
				case GUILD_WAR_RECV_DECLARE:
					Tracef(" >> GUILD_SUBHEADER_GC_WAR : GUILD_WAR_RECV_DECLARE\n");
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
						"BINARY_GuildWar_OnRecvDeclare",
						Py_BuildValue("(ii)", kGuildWar.dwGuildOpp, kGuildWar.bType)
					);
					break;
				case GUILD_WAR_ON_WAR:
					Tracef(" >> GUILD_SUBHEADER_GC_WAR : GUILD_WAR_ON_WAR : %d, %d\n", kGuildWar.dwGuildSelf, kGuildWar.dwGuildOpp);
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
						"BINARY_GuildWar_OnStart",
						Py_BuildValue("(ii)", kGuildWar.dwGuildSelf, kGuildWar.dwGuildOpp)
					);
					CPythonGuild::Instance().StartGuildWar(kGuildWar.dwGuildOpp);
					break;
				case GUILD_WAR_END:
					Tracef(" >> GUILD_SUBHEADER_GC_WAR : GUILD_WAR_END\n");
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
						"BINARY_GuildWar_OnEnd",
						Py_BuildValue("(ii)", kGuildWar.dwGuildSelf, kGuildWar.dwGuildOpp)
					);
					CPythonGuild::Instance().EndGuildWar(kGuildWar.dwGuildOpp);
					break;
			}
			break;
		}
		case GUILD_SUBHEADER_GC_GUILD_NAME:
		{
			DWORD dwID;
			char szGuildName[GUILD_NAME_MAX_LEN+1];

			int iPacketSize = int(GuildPacket.size) - sizeof(GuildPacket);

			int nItemSize = sizeof(dwID) + GUILD_NAME_MAX_LEN;

			assert(iPacketSize%nItemSize==0 && "GUILD_SUBHEADER_GC_GUILD_NAME");

			for (; iPacketSize > 0;)
			{
				if (!Recv(sizeof(dwID), &dwID))
					return false;

				if (!Recv(GUILD_NAME_MAX_LEN, &szGuildName))
					return false;

				szGuildName[GUILD_NAME_MAX_LEN] = 0;

				//Tracef(" >> GulidName [%d : %s]\n", dwID, szGuildName);
				CPythonGuild::Instance().RegisterGuildName(dwID, szGuildName);
				iPacketSize -= nItemSize;
			}
			break;
		}
		case GUILD_SUBHEADER_GC_GUILD_WAR_LIST:
		{
			DWORD dwSrcGuildID;
			DWORD dwDstGuildID;

			int iPacketSize = int(GuildPacket.size) - sizeof(GuildPacket);
			int nItemSize = sizeof(dwSrcGuildID) + sizeof(dwDstGuildID);

			assert(iPacketSize%nItemSize==0 && "GUILD_SUBHEADER_GC_GUILD_WAR_LIST");

			for (; iPacketSize > 0;)
			{
				if (!Recv(sizeof(dwSrcGuildID), &dwSrcGuildID))
					return false;

				if (!Recv(sizeof(dwDstGuildID), &dwDstGuildID))
					return false;

				Tracef(" >> GulidWarList [%d vs %d]\n", dwSrcGuildID, dwDstGuildID);
				CInstanceBase::InsertGVGKey(dwSrcGuildID, dwDstGuildID);
				CPythonCharacterManager::Instance().ChangeGVG(dwSrcGuildID, dwDstGuildID);
				iPacketSize -= nItemSize;
			}
			break;
		}
		case GUILD_SUBHEADER_GC_GUILD_WAR_END_LIST:
		{
			DWORD dwSrcGuildID;
			DWORD dwDstGuildID;

			int iPacketSize = int(GuildPacket.size) - sizeof(GuildPacket);
			int nItemSize = sizeof(dwSrcGuildID) + sizeof(dwDstGuildID);

			assert(iPacketSize%nItemSize==0 && "GUILD_SUBHEADER_GC_GUILD_WAR_END_LIST");

			for (; iPacketSize > 0;)
			{
				if (!Recv(sizeof(dwSrcGuildID), &dwSrcGuildID))
					return false;

				if (!Recv(sizeof(dwDstGuildID), &dwDstGuildID))
					return false;

				Tracef(" >> GulidWarEndList [%d vs %d]\n", dwSrcGuildID, dwDstGuildID);
				CInstanceBase::RemoveGVGKey(dwSrcGuildID, dwDstGuildID);
				CPythonCharacterManager::Instance().ChangeGVG(dwSrcGuildID, dwDstGuildID);
				iPacketSize -= nItemSize;
			}
			break;
		}
		case GUILD_SUBHEADER_GC_WAR_POINT:
		{
			TPacketGuildWarPoint GuildWarPoint;
			if (!Recv(sizeof(GuildWarPoint), &GuildWarPoint))
				return false;

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
				"BINARY_GuildWar_OnRecvPoint",
				Py_BuildValue("(iii)", GuildWarPoint.dwGainGuildID, GuildWarPoint.dwOpponentGuildID, GuildWarPoint.lPoint)
			);
			break;
		}
		case GUILD_SUBHEADER_GC_MONEY_CHANGE:
		{
			DWORD dwMoney;
			if (!Recv(sizeof(dwMoney), &dwMoney))
				return false;

			CPythonGuild::Instance().SetGuildMoney(dwMoney);

			__RefreshGuildWindowInfoPage();
			Tracef(" >> Guild Money Change : %d\n", dwMoney);
			break;
		}
#if defined(ENABLE_MESSENGER_RENEWAL) && defined(ENABLE_COMMUNITY_GUILD_RENEWAL)
		case GUILD_SUBHEADER_GC_MEMBER_LOCATION:
		{
			TPacketGCGuildMemberLocation loc;
			if (!Recv(sizeof(loc), &loc))
				return false;

			CPythonCommunity::Instance().OnGuildMemberLocation(loc.dwPID, loc.bChannel, loc.lMapIndex);
			break;
		}
#endif
	}

	return true;
}

// Guild
//////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// Fishing

bool CPythonNetworkStream::SendFishingPacket(int iRotation)
{
	BYTE byHeader = HEADER_CG_FISHING;
	if (!Send(sizeof(byHeader), &byHeader))
		return false;
	BYTE byPacketRotation = iRotation / 5;
	if (!Send(sizeof(BYTE), &byPacketRotation))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendGiveItemPacket(DWORD dwTargetVID, TItemPos ItemPos, int iItemCount)
{
	TPacketCGGiveItem GiveItemPacket;
	GiveItemPacket.byHeader = HEADER_CG_GIVE_ITEM;
	GiveItemPacket.dwTargetVID = dwTargetVID;
	GiveItemPacket.ItemPos = ItemPos;
	GiveItemPacket.byItemCount = iItemCount;

	if (!Send(sizeof(GiveItemPacket), &GiveItemPacket))
		return false;

	return SendSequence();
}

#if defined(WJ_COMBAT_ZONE)
bool CPythonNetworkStream::__RecvCombatZoneRankingPacket()
{
	TPacketGCCombatZoneRankingData p;
	if (!Recv(sizeof(TPacketGCCombatZoneRankingData), &p))
	{
		Tracen("Recv PacketGCCombatZoneRankingData Error");
		return false;
	}

	CPythonCombatZone::instance()->Initialize(p);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_CombatZone_Manager", Py_BuildValue("(s)", "RegisterRank"));
	return true;
}

bool CPythonNetworkStream::SendCombatZoneRequestActionPacket(int iAction, int iValue)
{
	TPacketCGCombatZoneRequestAction p;
	p.header = HEADER_CG_COMBAT_ZONE_REQUEST_ACTION;
	p.action = iAction;
	p.value	 = iValue;

	if (!Send(sizeof(p), &p))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::__RecvCombatZonePacket()
{
	TPacketGCSendCombatZone p;

	if (!Recv(sizeof(TPacketGCSendCombatZone), &p))
	{
		Tracen("Recv CombatZone Packet Error");
		return false;
	}

	switch (p.sub_header)
	{
		/*
		case COMBAT_ZONE_SUB_HEADER_ADD_LEAVING_TARGET:
			CPythonBackground::Instance().CreateCombatZoneTargetsLeaving(p.m_pInfoData[0], p.m_pInfoData[1]);
			break;
		
		case COMBAT_ZONE_SUB_HEADER_REMOVE_LEAVING_TARGET:
			CPythonBackground::Instance().DeleteCombatZoneTargetsLeaving(0);
			break;
		*/
			
		case COMBAT_ZONE_SUB_HEADER_FLASH_ON_MINIMAP:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_CombatZone_Manager", Py_BuildValue("(s)", "StartFlashing"));
			break;		
			
		case COMBAT_ZONE_SUB_HEADER_OPEN_RANKING:
		{
			DWORD dwPoints = p.m_pInfoData[0];
			DWORD dwTimeRemaining = p.m_pInfoData[1];
			DWORD dwCurMobsKills = p.m_pInfoData[2];
			DWORD dwMaxMobsKills = p.m_pInfoData[3];
			
			CPythonCombatZone::instance()->SendDataDays(p.m_pDataDays, p.isRunning);
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_CombatZone_Manager", Py_BuildValue("(siiii)", "OpenWindow", dwPoints, dwTimeRemaining, dwCurMobsKills, dwMaxMobsKills));
		}
		break;	

		case COMBAT_ZONE_SUB_HEADER_REFRESH_SHOP:
		{
			DWORD dwPoints = p.m_pInfoData[0];
			DWORD dwCurLimit = p.m_pInfoData[1];
			DWORD dwMaxLimit = p.m_pInfoData[2];
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_CombatZone_Manager", Py_BuildValue("(siii)", "RefreshShop", dwPoints, dwCurLimit, dwMaxLimit));
		}
		break;	
		default:
			return false;
	}
	return true;
}
#endif

bool CPythonNetworkStream::RecvFishing()
{
	TPacketGCFishing FishingPacket;
	if (!Recv(sizeof(FishingPacket), &FishingPacket))
		return false;

	CInstanceBase * pFishingInstance = NULL;
	if (FISHING_SUBHEADER_GC_FISH != FishingPacket.subheader)
	{
		pFishingInstance = CPythonCharacterManager::Instance().GetInstancePtr(FishingPacket.info);
		if (!pFishingInstance)
			return true;
	}

	switch (FishingPacket.subheader)
	{
		case FISHING_SUBHEADER_GC_START:
			pFishingInstance->StartFishing(float(FishingPacket.dir) * 5.0f);
			break;
		case FISHING_SUBHEADER_GC_STOP:
			if (pFishingInstance->IsFishing())
				pFishingInstance->StopFishing();
			break;
		case FISHING_SUBHEADER_GC_REACT:
			if (pFishingInstance->IsFishing())
			{
				pFishingInstance->SetFishEmoticon(); // Fish Emoticon
				pFishingInstance->ReactFishing();
			}
			break;
		case FISHING_SUBHEADER_GC_SUCCESS:
			pFishingInstance->CatchSuccess();
			break;
		case FISHING_SUBHEADER_GC_FAIL:
			pFishingInstance->CatchFail();
			if (pFishingInstance == CPythonCharacterManager::Instance().GetMainInstancePtr())
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFishingFailure", Py_BuildValue("()"));
			}
			break;
		case FISHING_SUBHEADER_GC_FISH:
		{
			DWORD dwFishID = FishingPacket.info;

			if (0 == FishingPacket.info)
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFishingNotifyUnknown", Py_BuildValue("()"));
				return true;
			}

			CItemData * pItemData;
			if (!CItemManager::Instance().GetItemDataPointer(dwFishID, &pItemData))
				return true;

			CInstanceBase * pMainInstance = CPythonCharacterManager::Instance().GetMainInstancePtr();
			if (!pMainInstance)
				return true;

			if (pMainInstance->IsFishing())
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFishingNotify", Py_BuildValue("(is)", CItemData::ITEM_TYPE_FISH == pItemData->GetType(), pItemData->GetName()));
			}
			else
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnFishingSuccess", Py_BuildValue("(is)", CItemData::ITEM_TYPE_FISH == pItemData->GetType(), pItemData->GetName()));
			}
			break;
		}
	}

	return true;
}
// Fishing
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// Dungeon
bool CPythonNetworkStream::RecvDungeon()
{
	TPacketGCDungeon DungeonPacket;
	if (!Recv(sizeof(DungeonPacket), &DungeonPacket))
		return false;

	switch (DungeonPacket.subheader)
	{
		case DUNGEON_SUBHEADER_GC_TIME_ATTACK_START:
		{
			break;
		}
		case DUNGEON_SUBHEADER_GC_DESTINATION_POSITION:
		{
			unsigned long ulx, uly;
			if (!Recv(sizeof(ulx), &ulx))
				return false;
			if (!Recv(sizeof(uly), &uly))
				return false;

			CPythonPlayer::Instance().SetDungeonDestinationPosition(ulx, uly);
			break;
		}
	}

	return true;
}
// Dungeon
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// MyShop
bool CPythonNetworkStream::SendBuildPrivateShopPacket(const char * c_szName, const std::vector<TShopItemTable> & c_rSellingItemStock)
{
	TPacketCGMyShop packet;
	packet.bHeader = HEADER_CG_MYSHOP;
	strncpy(packet.szSign, c_szName, SHOP_SIGN_MAX_LEN);
	packet.bCount = c_rSellingItemStock.size();
	if (!Send(sizeof(packet), &packet))
		return false;

	for (std::vector<TShopItemTable>::const_iterator itor = c_rSellingItemStock.begin(); itor < c_rSellingItemStock.end(); ++itor)
	{
		const TShopItemTable & c_rItem = *itor;
		if (!Send(sizeof(c_rItem), &c_rItem))
			return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RecvShopSignPacket()
{
	TPacketGCShopSign p;
	if (!Recv(sizeof(TPacketGCShopSign), &p))
		return false;

	CPythonPlayer& rkPlayer=CPythonPlayer::Instance();

	if (0 == strlen(p.szSign))
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
			"BINARY_PrivateShop_Disappear",
			Py_BuildValue("(i)", p.dwVID)
		);

		if (rkPlayer.IsMainCharacterIndex(p.dwVID))
			rkPlayer.ClosePrivateShop();
	}
	else
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
			"BINARY_PrivateShop_Appear",
			Py_BuildValue("(is)", p.dwVID, p.szSign)
		);

		if (rkPlayer.IsMainCharacterIndex(p.dwVID))
			rkPlayer.OpenPrivateShop();
	}

	return true;
}
/////////////////////////////////////////////////////////////////////////

bool CPythonNetworkStream::RecvTimePacket()
{
	TPacketGCTime TimePacket;
	if (!Recv(sizeof(TimePacket), &TimePacket))
		return false;

	IAbstractApplication& rkApp=IAbstractApplication::GetSingleton();
	rkApp.SetServerTime(TimePacket.time);

	return true;
}

bool CPythonNetworkStream::RecvWalkModePacket()
{
	TPacketGCWalkMode WalkModePacket;
	if (!Recv(sizeof(WalkModePacket), &WalkModePacket))
		return false;

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(WalkModePacket.vid);
	if (pInstance)
	{
		if (WALKMODE_RUN == WalkModePacket.mode)
		{
			pInstance->SetRunMode();
		}
		else
		{
			pInstance->SetWalkMode();
		}
	}

	return true;
}

bool CPythonNetworkStream::RecvChangeSkillGroupPacket()
{
	TPacketGCChangeSkillGroup ChangeSkillGroup;
	if (!Recv(sizeof(ChangeSkillGroup), &ChangeSkillGroup))
		return false;

	m_dwMainActorSkillGroup = ChangeSkillGroup.skill_group;

	CPythonPlayer::Instance().NEW_ClearSkillData();
	__RefreshCharacterWindow();
	return true;
}

void CPythonNetworkStream::__TEST_SetSkillGroupFake(int iIndex)
{
	m_dwMainActorSkillGroup = DWORD(iIndex);

	CPythonPlayer::Instance().NEW_ClearSkillData();
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshCharacter", Py_BuildValue("()"));
}

bool CPythonNetworkStream::SendRefinePacket(BYTE byPos, BYTE byType)
{
	TPacketCGRefine kRefinePacket;
	kRefinePacket.header = HEADER_CG_REFINE;
	kRefinePacket.pos = byPos;
	kRefinePacket.type = byType;

	if (!Send(sizeof(kRefinePacket), &kRefinePacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendSelectItemPacket(DWORD dwItemPos)
{
	TPacketCGScriptSelectItem kScriptSelectItem;
	kScriptSelectItem.header = HEADER_CG_SCRIPT_SELECT_ITEM;
	kScriptSelectItem.selection = dwItemPos;

	if (!Send(sizeof(kScriptSelectItem), &kScriptSelectItem))
		return false;

	return SendSequence();
}

#if defined(__BL_OFFICIAL_LOOT_FILTER__)
bool CPythonNetworkStream::SendLootFilterPacket(BYTE(&settings)[ELootFilter::LOOT_FILTER_SETTINGS_MAX])
{
	TPacketCGLootFilter p;
	p.header = HEADER_CG_LOOT_FILTER;
	memcpy(p.settings, settings, sizeof(p.settings));

	if (!Send(sizeof(p), &p))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvLootFilterPacket()
{
	TPacketGCLootFilter p;
	if (!Recv(sizeof(p), &p))
		return false;

	if (p.enable)
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "LootingSystemProcess", Py_BuildValue("()"));
	else
		CPythonItem::Instance().InsertLootFilteredItem(p.vid);

	return true;
}
#endif

bool CPythonNetworkStream::RecvRefineInformationPacket()
{
	TPacketGCRefineInformation kRefineInfoPacket;
	if (!Recv(sizeof(kRefineInfoPacket), &kRefineInfoPacket))
		return false;

	TRefineTable & rkRefineTable = kRefineInfoPacket.refine_table;
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
		"OpenRefineDialog",
		Py_BuildValue("(iiii)",
			kRefineInfoPacket.pos,
			kRefineInfoPacket.refine_table.result_vnum,
			rkRefineTable.cost,
			rkRefineTable.prob));

	for (int i = 0; i < rkRefineTable.material_count; ++i)
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AppendMaterialToRefineDialog", Py_BuildValue("(ii)", rkRefineTable.materials[i].vnum, rkRefineTable.materials[i].count));
	}

#ifdef _DEBUG
	Tracef(" >> RecvRefineInformationPacket(pos=%d, result_vnum=%d, cost=%d, prob=%d)\n",
														kRefineInfoPacket.pos,
														kRefineInfoPacket.refine_table.result_vnum,
														rkRefineTable.cost,
														rkRefineTable.prob);
#endif

	return true;
}

bool CPythonNetworkStream::RecvRefineInformationPacketNew()
{
	TPacketGCRefineInformationNew kRefineInfoPacket;
	if (!Recv(sizeof(kRefineInfoPacket), &kRefineInfoPacket))
		return false;

	TRefineTable & rkRefineTable = kRefineInfoPacket.refine_table;
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME],
		"OpenRefineDialog",
		Py_BuildValue("(iiiii)",
			kRefineInfoPacket.pos,
			kRefineInfoPacket.refine_table.result_vnum,
			rkRefineTable.cost,
			rkRefineTable.prob,
			kRefineInfoPacket.type)
		);

	for (int i = 0; i < rkRefineTable.material_count; ++i)
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AppendMaterialToRefineDialog", Py_BuildValue("(ii)", rkRefineTable.materials[i].vnum, rkRefineTable.materials[i].count));
	}

#ifdef _DEBUG
	Tracef(" >> RecvRefineInformationPacketNew(pos=%d, result_vnum=%d, cost=%d, prob=%d, type=%d)\n",
														kRefineInfoPacket.pos,
														kRefineInfoPacket.refine_table.result_vnum,
														rkRefineTable.cost,
														rkRefineTable.prob,
														kRefineInfoPacket.type);
#endif

	return true;
}

bool CPythonNetworkStream::RecvNPCList()
{
	TPacketGCNPCPosition kNPCPosition;
	if (!Recv(sizeof(kNPCPosition), &kNPCPosition))
		return false;

	assert(int(kNPCPosition.size)-sizeof(kNPCPosition) == kNPCPosition.count*sizeof(TNPCPosition) && "HEADER_GC_NPC_POSITION");

	CPythonMiniMap::Instance().ClearAtlasMarkInfo();

	for (int i = 0; i < kNPCPosition.count; ++i)
	{
		TNPCPosition NPCPosition;
		if (!Recv(sizeof(TNPCPosition), &NPCPosition))
			return false;

		CPythonMiniMap::Instance().RegisterAtlasMark(NPCPosition.bType, NPCPosition.name, NPCPosition.x, NPCPosition.y);
	}

	return true;
}

bool CPythonNetworkStream::__SendCRCReportPacket()
{
	/*
	DWORD dwProcessCRC = 0;
	DWORD dwFileCRC = 0;
	CFilename exeFileName;
	//LPCVOID c_pvBaseAddress = NULL;

	GetExeCRC(dwProcessCRC, dwFileCRC);

	CFilename strRootPackFileName = CEterPackManager::Instance().GetRootPacketFileName();
	strRootPackFileName.ChangeDosPath();

	TPacketCGCRCReport kReportPacket;

	kReportPacket.header = HEADER_CG_CRC_REPORT;
	kReportPacket.byPackMode = CEterPackManager::Instance().GetSearchMode();
	kReportPacket.dwBinaryCRC32 = dwFileCRC;
	kReportPacket.dwProcessCRC32 = dwProcessCRC;
	kReportPacket.dwRootPackCRC32 = GetFileCRC32(strRootPackFileName.c_str());

	if (!Send(sizeof(kReportPacket), &kReportPacket))
		Tracef("SendClientReportPacket Error");

	return SendSequence();
	*/
	return true;
}

bool CPythonNetworkStream::SendClientVersionPacket()
{
	std::string filename;

	GetExcutedFileName(filename);

	filename = CFileNameHelper::NoPath(filename);
	CFileNameHelper::ChangeDosPath(filename);

	TPacketCGClientVersion2 kVersionPacket;
	kVersionPacket.header = HEADER_CG_CLIENT_VERSION2;
	strncpy(kVersionPacket.filename, filename.c_str(), sizeof(kVersionPacket.filename)-1);
	strncpy(kVersionPacket.timestamp, "4227586879", sizeof(kVersionPacket.timestamp)-1);

	if (!Send(sizeof(kVersionPacket), &kVersionPacket))
		Tracef("SendClientReportPacket Error");

	return SendSequence();
}

bool CPythonNetworkStream::RecvAffectAddPacket()
{
	TPacketGCAffectAdd kAffectAdd;
	if (!Recv(sizeof(kAffectAdd), &kAffectAdd))
		return false;

	TPacketAffectElement & rkElement = kAffectAdd.elem;
	if (rkElement.bPointIdxApplyOn == POINT_ENERGY)
	{
		CPythonPlayer::instance().SetStatus (POINT_ENERGY_END_TIME, CPythonApplication::Instance().GetServerTimeStamp() + rkElement.lDuration);
		__RefreshStatus();
	}
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_NEW_AddAffect", Py_BuildValue("(iiii)", rkElement.dwType, rkElement.bPointIdxApplyOn, rkElement.lApplyValue, rkElement.lDuration));

	return true;
}

bool CPythonNetworkStream::RecvAffectRemovePacket()
{
	TPacketGCAffectRemove kAffectRemove;
	if (!Recv(sizeof(kAffectRemove), &kAffectRemove))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_NEW_RemoveAffect", Py_BuildValue("(ii)", kAffectRemove.dwType, kAffectRemove.bApplyOn));

	return true;
}

bool CPythonNetworkStream::RecvChannelPacket()
{
	TPacketGCChannel kChannelPacket;
	if (!Recv(sizeof(kChannelPacket), &kChannelPacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_NEW_CurrentChannel", Py_BuildValue("(i)", kChannelPacket.channel));

	return true;
}

#ifdef ENABLE_GUILD_TOKEN_AUTH
bool CPythonNetworkStream::RecvGuildTokenPacket()
{
	TPacketGCGuildToken packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	__SetGuildToken(packet.token);
	return true;
}
#endif

bool CPythonNetworkStream::RecvViewEquipPacket()
{
	TPacketGCViewEquip kViewEquipPacket;
	if (!Recv(sizeof(kViewEquipPacket), &kViewEquipPacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenEquipmentDialog", Py_BuildValue("(i)", kViewEquipPacket.dwVID));

	for (int i = 0; i < WEAR_MAX_NUM; ++i)
	{
		TEquipmentItemSet & rItemSet = kViewEquipPacket.equips[i];
#if defined(__BL_TRANSMUTATION__)
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetEquipmentDialogItem", Py_BuildValue("(iiiii)", kViewEquipPacket.dwVID, i, rItemSet.vnum, rItemSet.count, rItemSet.dwTransmutationVnum));
#else
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetEquipmentDialogItem", Py_BuildValue("(iiii)", kViewEquipPacket.dwVID, i, rItemSet.vnum, rItemSet.count));
#endif

		for (int j = 0; j < ITEM_SOCKET_SLOT_MAX_NUM; ++j)
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetEquipmentDialogSocket", Py_BuildValue("(iiii)", kViewEquipPacket.dwVID, i, j, rItemSet.alSockets[j]));

		for (int k = 0; k < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++k)
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "SetEquipmentDialogAttr", Py_BuildValue("(iiiii)", kViewEquipPacket.dwVID, i, k, rItemSet.aAttr[k].bType, rItemSet.aAttr[k].sValue));
	}

	return true;
}

bool CPythonNetworkStream::RecvLandPacket()
{
	TPacketGCLandList kLandList;
	if (!Recv(sizeof(kLandList), &kLandList))
		return false;

	std::vector<DWORD> kVec_dwGuildID;

	CPythonMiniMap & rkMiniMap = CPythonMiniMap::Instance();
	CPythonBackground & rkBG = CPythonBackground::Instance();
	CInstanceBase * pMainInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();

	rkMiniMap.ClearGuildArea();
	rkBG.ClearGuildArea();

	int iPacketSize = (kLandList.size - sizeof(TPacketGCLandList));
	for (; iPacketSize > 0; iPacketSize-=sizeof(TLandPacketElement))
	{
		TLandPacketElement kElement;
		if (!Recv(sizeof(TLandPacketElement), &kElement))
			return false;

		rkMiniMap.RegisterGuildArea(kElement.dwID,
									kElement.dwGuildID,
									kElement.x,
									kElement.y,
									kElement.width,
									kElement.height);

		if (pMainInstance)
		if (kElement.dwGuildID == pMainInstance->GetGuildID())
		{
			rkBG.RegisterGuildArea(kElement.x,
								   kElement.y,
								   kElement.x+kElement.width,
								   kElement.y+kElement.height);
		}

		if (0 != kElement.dwGuildID)
			kVec_dwGuildID.push_back(kElement.dwGuildID);
	}
	// @fixme006
	if (kVec_dwGuildID.size()>0)
		__DownloadSymbol(kVec_dwGuildID);

	return true;
}

bool CPythonNetworkStream::RecvTargetCreatePacket()
{
	TPacketGCTargetCreate kTargetCreate;
	if (!Recv(sizeof(kTargetCreate), &kTargetCreate))
		return false;

	CPythonMiniMap & rkpyMiniMap = CPythonMiniMap::Instance();
	rkpyMiniMap.CreateTarget(kTargetCreate.lID, kTargetCreate.szTargetName);

//#ifdef _DEBUG
//	char szBuf[256+1];
//	CPythonChat::Instance().AppendChat(CHAT_TYPE_NOTICE, szBuf);
//	Tracef(" >> RecvTargetCreatePacket %d : %s\n", kTargetCreate.lID, kTargetCreate.szTargetName);
//#endif

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_OpenAtlasWindow", Py_BuildValue("()"));
	return true;
}

bool CPythonNetworkStream::RecvTargetCreatePacketNew()
{
	TPacketGCTargetCreateNew kTargetCreate;
	if (!Recv(sizeof(kTargetCreate), &kTargetCreate))
		return false;

	CPythonMiniMap & rkpyMiniMap = CPythonMiniMap::Instance();
	CPythonBackground & rkpyBG = CPythonBackground::Instance();
	if (CREATE_TARGET_TYPE_LOCATION == kTargetCreate.byType)
	{
		rkpyMiniMap.CreateTarget(kTargetCreate.lID, kTargetCreate.szTargetName);
	}
#if defined(WJ_COMBAT_ZONE)
	else if (CREATE_TARGET_TYPE_COMBAT_ZONE == kTargetCreate.byType)
	{
		rkpyBG.CreateCombatZoneTargetsLeaving(kTargetCreate.lID, kTargetCreate.dwVID);
		rkpyMiniMap.CreateTarget(kTargetCreate.lID,
			#if defined(WJ_COMBAT_ZONE_HIDE_INFO_USER)
			"" 
			#else
			kTargetCreate.szTargetName
			#endif
			,kTargetCreate.dwVID);
	}
#endif
	else if (3 == kTargetCreate.byType)
	{
		rkpyMiniMap.CreateTarget(kTargetCreate.lID, kTargetCreate.szTargetName, kTargetCreate.dwVID);
		rkpyBG.CreateTargetEffect2(kTargetCreate.lID, kTargetCreate.dwVID);
	}
	else
	{
		rkpyMiniMap.CreateTarget(kTargetCreate.lID, kTargetCreate.szTargetName, kTargetCreate.dwVID);
		rkpyBG.CreateTargetEffect(kTargetCreate.lID, kTargetCreate.dwVID);
	}

//#ifdef _DEBUG
//	char szBuf[256+1];
//	CPythonChat::Instance().AppendChat(CHAT_TYPE_NOTICE, szBuf);
//	Tracef(" >> RecvTargetCreatePacketNew %d : %d/%d\n", kTargetCreate.lID, kTargetCreate.byType, kTargetCreate.dwVID);
//#endif

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_OpenAtlasWindow", Py_BuildValue("()"));
	return true;
}

bool CPythonNetworkStream::RecvTargetUpdatePacket()
{
	TPacketGCTargetUpdate kTargetUpdate;
	if (!Recv(sizeof(kTargetUpdate), &kTargetUpdate))
		return false;

	CPythonMiniMap & rkpyMiniMap = CPythonMiniMap::Instance();
	rkpyMiniMap.UpdateTarget(kTargetUpdate.lID, kTargetUpdate.lX, kTargetUpdate.lY);

	CPythonBackground & rkpyBG = CPythonBackground::Instance();
	rkpyBG.CreateTargetEffect(kTargetUpdate.lID, kTargetUpdate.lX, kTargetUpdate.lY);

//#ifdef _DEBUG
//	char szBuf[256+1];
//	CPythonChat::Instance().AppendChat(CHAT_TYPE_NOTICE, szBuf);
//	Tracef(" >> RecvTargetUpdatePacket %d : %d, %d\n", kTargetUpdate.lID, kTargetUpdate.lX, kTargetUpdate.lY);
//#endif

	return true;
}

bool CPythonNetworkStream::RecvTargetDeletePacket()
{
	TPacketGCTargetDelete kTargetDelete;
	if (!Recv(sizeof(kTargetDelete), &kTargetDelete))
		return false;

	CPythonMiniMap & rkpyMiniMap = CPythonMiniMap::Instance();
	rkpyMiniMap.DeleteTarget(kTargetDelete.lID);

	CPythonBackground & rkpyBG = CPythonBackground::Instance();
	rkpyBG.DeleteTargetEffect(kTargetDelete.lID);

//#ifdef _DEBUG
//	Tracef(" >> RecvTargetDeletePacket %d\n", kTargetDelete.lID);
//#endif

	return true;
}

#ifdef ENABLE_CONQUEROR_LEVEL
bool CPythonNetworkStream::RecvSungmaAttrUpdatePacket()
{
	TPacketGCSungmaAttrUpdate kSungmaAttrUpdate;
	if (!Recv(sizeof(kSungmaAttrUpdate), &kSungmaAttrUpdate))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_SungMaAttr", Py_BuildValue("(iiii)", kSungmaAttrUpdate.bSTR, kSungmaAttrUpdate.bHP, kSungmaAttrUpdate.bMOVE, kSungmaAttrUpdate.bINMUNE));
#ifdef _DEBUG
	Tracef("RECV SUNGMA ATTR UPDATE : %d\n", kSungmaAttrUpdate.bSTR, kSungmaAttrUpdate.bHP, kSungmaAttrUpdate.bMOVE, kSungmaAttrUpdate.bINMUNE);
#endif
	return true;
}
#endif

bool CPythonNetworkStream::RecvLoverInfoPacket()
{
	TPacketGCLoverInfo kLoverInfo;
	if (!Recv(sizeof(kLoverInfo), &kLoverInfo))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_LoverInfo", Py_BuildValue("(si)", kLoverInfo.szName, kLoverInfo.byLovePoint));
#ifdef _DEBUG
	Tracef("RECV LOVER INFO : %s, %d\n", kLoverInfo.szName, kLoverInfo.byLovePoint);
#endif
	return true;
}

bool CPythonNetworkStream::RecvLovePointUpdatePacket()
{
	TPacketGCLovePointUpdate kLovePointUpdate;
	if (!Recv(sizeof(kLovePointUpdate), &kLovePointUpdate))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_UpdateLovePoint", Py_BuildValue("(i)", kLovePointUpdate.byLovePoint));
#ifdef _DEBUG
	Tracef("RECV LOVE POINT UPDATE : %d\n", kLovePointUpdate.byLovePoint);
#endif
	return true;
}

bool CPythonNetworkStream::RecvDigMotionPacket()
{
	TPacketGCDigMotion kDigMotion;
	if (!Recv(sizeof(kDigMotion), &kDigMotion))
		return false;

#ifdef _DEBUG
	Tracef(" Dig Motion [%d/%d]\n", kDigMotion.vid, kDigMotion.count);
#endif

	IAbstractCharacterManager& rkChrMgr=IAbstractCharacterManager::GetSingleton();
	CInstanceBase * pkInstMain = rkChrMgr.GetInstancePtr(kDigMotion.vid);
	CInstanceBase * pkInstTarget = rkChrMgr.GetInstancePtr(kDigMotion.target_vid);
	if (NULL == pkInstMain)
		return true;

	if (pkInstTarget)
		pkInstMain->NEW_LookAtDestInstance(*pkInstTarget);

	for (int i = 0; i < kDigMotion.count; ++i)
		pkInstMain->PushOnceMotion(CRaceMotionData::NAME_DIG);

	return true;
}

bool CPythonNetworkStream::SendDragonSoulRefinePacket(BYTE bRefineType, TItemPos* pos)
{
	TPacketCGDragonSoulRefine pk;
	pk.header = HEADER_CG_DRAGON_SOUL_REFINE;
	pk.bSubType = bRefineType;
	memcpy (pk.ItemGrid, pos, sizeof (TItemPos) * DS_REFINE_WINDOW_MAX_NUM);
	if (!Send(sizeof (pk), &pk))
	{
		return false;
	}
	return true;
}

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
bool CPythonNetworkStream::RecvAccePacket(bool bReturn)
{
	TPacketAcce sPacket;
	if (!Recv(sizeof(sPacket), &sPacket))
		return bReturn;

	bReturn = true;
	switch (sPacket.subheader)
	{
		case ACCE_SUBHEADER_GC_OPEN:
			CPythonAcce::Instance().Clear();
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActAcce", Py_BuildValue("(ib)", 1, sPacket.bWindow));
			break;

		case ACCE_SUBHEADER_GC_CLOSE:
			CPythonAcce::Instance().Clear();
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActAcce", Py_BuildValue("(ib)", 2, sPacket.bWindow));
			break;

		case ACCE_SUBHEADER_GC_ADDED:
			CPythonAcce::Instance().AddMaterial(sPacket.dwPrice, sPacket.bPos, sPacket.tPos);
			if (sPacket.bPos == 1)
			{
				CPythonAcce::Instance().AddResult(sPacket.dwItemVnum, sPacket.dwMinAbs, sPacket.dwMaxAbs);
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AlertAcce", Py_BuildValue("(b)", sPacket.bWindow));
			}

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActAcce", Py_BuildValue("(ib)", 3, sPacket.bWindow));
			break;

		case ACCE_SUBHEADER_GC_REMOVED:
			CPythonAcce::Instance().RemoveMaterial(sPacket.dwPrice, sPacket.bPos);
			if (sPacket.bPos == 0)
				CPythonAcce::Instance().RemoveMaterial(sPacket.dwPrice, 1);

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActAcce", Py_BuildValue("(ib)", 4, sPacket.bWindow));
			break;

		case ACCE_SUBHEADER_CG_REFINED:
			if (sPacket.dwMaxAbs == 0)
				CPythonAcce::Instance().RemoveMaterial(sPacket.dwPrice, 1);
			else
			{
				CPythonAcce::Instance().RemoveMaterial(sPacket.dwPrice, 0);
				CPythonAcce::Instance().RemoveMaterial(sPacket.dwPrice, 1);
			}

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ActAcce", Py_BuildValue("(ib)", 4, sPacket.bWindow));
			break;

		default:
			TraceError("CPythonNetworkStream::RecvAccePacket: unknown subheader %d\n.", sPacket.subheader);
			break;
	}

	return bReturn;
}

bool CPythonNetworkStream::SendAcceClosePacket()
{
	if (!__CanActMainInstance())
		return true;

	TItemPos tPos;
	tPos.window_type	= INVENTORY;
	tPos.cell			= 0;

	TPacketAcce sPacket;
	sPacket.header		= HEADER_CG_ACCE;
	sPacket.subheader	= ACCE_SUBHEADER_CG_CLOSE;
	sPacket.dwPrice		= 0;
	sPacket.bPos		= 0;
	sPacket.tPos		= tPos;
	sPacket.dwItemVnum	= 0;
	sPacket.dwMinAbs	= 0;
	sPacket.dwMaxAbs	= 0;

	if (!Send(sizeof(sPacket), &sPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendAcceAddPacket(TItemPos tPos, BYTE bPos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketAcce sPacket;
	sPacket.header		= HEADER_CG_ACCE;
	sPacket.subheader	= ACCE_SUBHEADER_CG_ADD;
	sPacket.dwPrice		= 0;
	sPacket.bPos		= bPos;
	sPacket.tPos		= tPos;
	sPacket.dwItemVnum	= 0;
	sPacket.dwMinAbs	= 0;
	sPacket.dwMaxAbs	= 0;

	if (!Send(sizeof(sPacket), &sPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendAcceRemovePacket(BYTE bPos)
{
	if (!__CanActMainInstance())
		return true;

	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;

	TPacketAcce sPacket;
	sPacket.header		= HEADER_CG_ACCE;
	sPacket.subheader	= ACCE_SUBHEADER_CG_REMOVE;
	sPacket.dwPrice		= 0;
	sPacket.bPos		= bPos;
	sPacket.tPos		= tPos;
	sPacket.dwItemVnum	= 0;
	sPacket.dwMinAbs	= 0;
	sPacket.dwMaxAbs	= 0;

	if (!Send(sizeof(sPacket), &sPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendAcceRefinePacket()
{
	if (!__CanActMainInstance())
		return true;

	TItemPos tPos;
	tPos.window_type = INVENTORY;
	tPos.cell = 0;

	TPacketAcce sPacket;
	sPacket.header		= HEADER_CG_ACCE;
	sPacket.subheader	= ACCE_SUBHEADER_CG_REFINE;
	sPacket.dwPrice		= 0;
	sPacket.bPos		= 0;
	sPacket.tPos		= tPos;
	sPacket.dwItemVnum	= 0;
	sPacket.dwMinAbs	= 0;
	sPacket.dwMaxAbs	= 0;

	if (!Send(sizeof(sPacket), &sPacket))
		return false;

	return SendSequence();
}
#endif

#ifdef ENABLE_DISCORD_RPC
#include "Discord.h"
#include <discord_rpc.h>

#ifdef _DEBUG
#pragma comment(lib, "discord_rpc_d.lib")
#else
#pragma comment(lib, "discord_rpc_r.lib")
#endif

static int64_t DiscordStartTime{};

void CPythonNetworkStream::Discord_Start()
{
	DiscordStartTime = time(0);
	DiscordEventHandlers handlers{};
	Discord_Initialize(Discord::DiscordClientID, &handlers, 1, nullptr);
	Discord_Update(false);
}

void CPythonNetworkStream::Discord_Update(const bool ingame)
{
	DiscordRichPresence discordPresence{};
	discordPresence.startTimestamp = DiscordStartTime;

#ifdef ENABLE_DISCORD_JOIN_URL
	discordPresence.buttonLabel = "Join with me!";
	discordPresence.buttonURL = "https://www.worldmetin2.com/";
#endif

	if (!ingame)
	{
		Discord_UpdatePresence(&discordPresence);
		return;
	}

	/*Name*/
	auto NameData = Discord::GetNameData();
	discordPresence.state = std::get<0>(NameData).c_str();
	discordPresence.details = std::get<1>(NameData).c_str();

	/*Race*/
	auto RaceData = Discord::GetRaceData();
	discordPresence.largeImageKey = std::get<0>(RaceData).c_str();
	discordPresence.largeImageText = std::get<1>(RaceData).c_str();

	/*Empire*/
	auto EmpireData = Discord::GetEmpireData();
	discordPresence.smallImageKey = std::get<0>(EmpireData).c_str();
	discordPresence.smallImageText = std::get<1>(EmpireData).c_str();

	Discord_UpdatePresence(&discordPresence);
}

void CPythonNetworkStream::Discord_Close()
{
	Discord_Shutdown();
}
#endif
//martysama0134's fb0d8d5d0566583187a94f9a823b2075

#if defined(ENABLE_VOICE_CHAT)
bool CPythonNetworkStream::RecvVoiceChatPacket()
{
	static auto& rkVoice = VoiceChat::Instance();
	TVoiceChatPacket packet{};

	if (!Recv(sizeof(packet), &packet))
	{
		TraceError("FAILED TO RECEIVE VOICE CHAT PACKET");
		return false;
	}

	const uint32_t bufSize = sizeof(int16_t) * std::max<uint32_t>(4800, rkVoice.GetSampleRate()) * std::max<uint8_t>(8, rkVoice.GetChannelCount());
	static std::vector<char> buf(bufSize, 0/*silence*/); // Default to enough for 48k frequency on 8 channels
	if (buf.size() < bufSize)
		buf.resize(bufSize); // Tf are you doing to fill this?

	if (!Recv(packet.dataSize, buf.data()))
	{
		TraceError("FAILED TO RECEIVE AUDIO DATA! size %u, vid %u, name %s, totalSize %u, recvBufSize %d(out %d, in %d, max %d)",
			packet.dataSize, packet.vid, packet.name, packet.size, GetRecvBufferSize(), m_recvBufOutputPos, m_recvBufInputPos, m_recvBufSize);
		return false;
	}

	if (auto inst = CPythonCharacterManager::Instance().GetInstancePtr(packet.vid))
	{
		inst->AttachSpecialEffect(CInstanceBase::EFFECT_VOICE_CHAT);
	}

	rkVoice.OnRecvVoiceChatPacket(packet, buf.data());
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_OnVoice",
		Py_BuildValue("(si)", packet.name, packet.type));

	return true;
}
#endif

#ifdef ENABLE_CSHIELD
bool CPythonNetworkStream::RecvCShieldPacket()
{
	TPacketGCCShield pCShieldPacket;

	if (!Recv(sizeof(TPacketGCCShield), &pCShieldPacket))
	{
		Tracen("Recv CShield Packet Error");
		return false;
	}

	SendCShieldPacket(pCShieldPacket.start, pCShieldPacket.num);
	return true;
}

bool CPythonNetworkStream::SendCShieldPacket(bool start, DWORD num)
{
	TPacketCGCShield pCShieldPacket;
	pCShieldPacket.header = HEADER_CG_CSHIELD;

	strncpy(pCShieldPacket.key, GenKey(start, num).c_str(), sizeof(pCShieldPacket.key) - 1);
	pCShieldPacket.key[64] = '\0';
	strncpy(pCShieldPacket.data, GenData().c_str(), sizeof(pCShieldPacket.data) - 1);
	pCShieldPacket.data[256] = '\0';

	if (!Send(sizeof(pCShieldPacket), &pCShieldPacket))
		return false;

	return true;
}

bool CPythonNetworkStream::RecvCShieldStopPlayerPacket()
{
	TPacketGCCShieldStopPlayer pCShieldStopPlayerPacket;

	if (!Recv(sizeof(TPacketGCCShieldStopPlayer), &pCShieldStopPlayerPacket))
	{
#if !defined(ENABLE_CSHIELD) || !defined(HIDE_STRINGS)
		Tracen("Recv CShieldStopPlayer Packet Error");
#endif
		return false;
	}

	CPythonPlayer::Instance().NEW_Stop();

	return true;
}
#endif
#ifdef ENABLE_CAPTCHA
bool CPythonNetworkStream::RecvCaptchaPacket()
{
	TPacketGCCaptcha pCaptchaPacket;

	if (!Recv(sizeof(TPacketGCCaptcha), &pCaptchaPacket))
	{
		Tracen("Recv Captcha Packet Error");
		return false;
	}

#ifdef HIDE_CAPTCHA
	if (strcmp(pCaptchaPacket.captchaImage, "1000") == 0)
		CPythonApplication::Instance().SetWindowInvisible(false);
	else
		CPythonApplication::Instance().SetWindowInvisible(true);
#endif

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_ShowCaptchaChallenge",
		Py_BuildValue("(s)", pCaptchaPacket.captchaImage));

	return true;
}

bool CPythonNetworkStream::SendCaptchaPacket(short captchaResult)
{
	TPacketCGCaptcha pCaptchaPacket;
	pCaptchaPacket.header = HEADER_CG_CAPTCHA;
	pCaptchaPacket.captchaResult = captchaResult;

	if (!Send(sizeof(pCaptchaPacket), &pCaptchaPacket))
		return false;

	return true;
}
#endif

#ifdef ENABLE_OFFLINESHOP_SYSTEM
bool CPythonNetworkStream::SendBuildOfflineShopPacket(const char* c_szName, const std::vector<TOfflineShopItemTable>& c_rSellingItemStock, DWORD shopVnum, BYTE shopTitle)
{
	TPacketCGMyOfflineShop packet;
	packet.bHeader = HEADER_CG_MY_OFFLINE_SHOP;
	strncpy(packet.szSign, c_szName, SHOP_SIGN_MAX_LEN);
	packet.bCount = c_rSellingItemStock.size();
	packet.shopVnum = shopVnum;
	packet.shopTitle = shopTitle;
	if (!Send(sizeof(packet), &packet))
		return false;

	for (std::vector<TOfflineShopItemTable>::const_iterator itor = c_rSellingItemStock.begin(); itor < c_rSellingItemStock.end(); ++itor)
	{
		const TOfflineShopItemTable& c_rItem = *itor;
		if (!Send(sizeof(c_rItem), &c_rItem))
			return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RecvOfflineShopSignPacket()
{
	TPacketGCShopSign p;
	if (!Recv(sizeof(TPacketGCShopSign), &p))
		return false;
	CInstanceBase* pInstance = CPythonCharacterManager::instance().GetInstancePtr(p.dwVID);
	if (pInstance)
	{
		if (pInstance->IsShop())
		{
			if (0 == strlen(p.szSign))
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_OfflineShop_Disappear", Py_BuildValue("(i)", p.dwVID));
			else
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_OfflineShop_Appear", Py_BuildValue("(is)", p.dwVID, p.szSign));
		}
	}
	return true;
}

bool CPythonNetworkStream::RecvOfflineShopPacket()
{
	TPacketGCShop  packet_shop;
	if (!Recv(sizeof(packet_shop), &packet_shop))
		return false;
	std::vector<char> vecBuffer;
	vecBuffer.clear();
	int iSize = packet_shop.size - sizeof(packet_shop);

	switch (packet_shop.subheader)
	{
	case SHOP_SUBHEADER_GC_REFRESH_MONEY:
	case SHOP_SUBHEADER_GC_REFRESH_DISPLAY_COUNT:
	case SHOP_SUBHEADER_GC_CHECK_RESULT:
	case SHOP_SUBHEADER_GC_REFRESH_COUNT:
	case SHOP_SUBHEADER_GC_REALWATCHER_COUNT:
	case SHOP_SUBHEADER_GC_CHANGE_TITLE:
		break;
	default:
		if (iSize > 0)
		{
			vecBuffer.resize(iSize);
			if (!Recv(iSize, &vecBuffer[0]))
				return false;
		}
	}

	switch (packet_shop.subheader)
	{
	case SHOP_SUBHEADER_GC_START:
	{
		CPythonShop::Instance().Clear();
		TPacketGCOfflineShopStart* pShopStartPacket = (TPacketGCOfflineShopStart*)vecBuffer.data();
		for (BYTE iItemIndex = 0; iItemIndex < OFFLINE_SHOP_HOST_ITEM_MAX_NUM; ++iItemIndex)
			CPythonShop::Instance().SetOfflineShopItemData(iItemIndex, pShopStartPacket->items[iItemIndex]);
		CPythonShop::Instance().SetShopDisplayedCount(pShopStartPacket->m_dwDisplayedCount);
		CPythonShop::Instance().SetRealWatcherCount(pShopStartPacket->m_dwRealWatcherCount);
		CPythonShop::Instance().SetCurrentOfflineShopMoney(pShopStartPacket->price);
		CPythonShop::Instance().SetShopSign(pShopStartPacket->title);
		CPythonShop::Instance().SetShopFlag(pShopStartPacket->flag);
		CPythonShop::Instance().SetShopType(pShopStartPacket->type);
		CPythonShop::Instance().SetShopTime(pShopStartPacket->time);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ClearOfflineShopLog", Py_BuildValue("()"));
		for (int logIndex = OFFLINE_SHOP_HOST_ITEM_MAX_NUM-1; logIndex >= 0; --logIndex)
			if (pShopStartPacket->log[logIndex].price != 0) {
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AppendShopLog", Py_BuildValue("(ssiiL)", pShopStartPacket->log[logIndex].name, pShopStartPacket->log[logIndex].date, pShopStartPacket->log[logIndex].itemVnum, pShopStartPacket->log[logIndex].itemCount, pShopStartPacket->log[logIndex].price));
			}

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartOfflineShop", Py_BuildValue("(ii)", pShopStartPacket->owner_vid, pShopStartPacket->IsOwner == true ? 1 : 0));
	}
	break;

	case SHOP_SUBHEADER_GC_END:
	{
		CPythonShop::instance().Clear();
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ClearOfflineShopLog", Py_BuildValue("()"));
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "EndOfflineShop", Py_BuildValue("()"));
	}
	break;

	case SHOP_SUBHEADER_GC_UPDATE_ITEM:
	{
		TPacketGCOfflineShopUpdateItem* pShopUpdateItemPacket = (TPacketGCOfflineShopUpdateItem*)vecBuffer.data();
		CPythonShop::Instance().SetShopDisplayedCount(pShopUpdateItemPacket->m_dwDisplayedCount);
		CPythonShop::Instance().SetRealWatcherCount(pShopUpdateItemPacket->m_dwRealWatcherCount);
		CPythonShop::Instance().SetShopFlag(pShopUpdateItemPacket->flag);
		CPythonShop::Instance().SetShopTime(pShopUpdateItemPacket->time);
		CPythonShop::Instance().SetShopSign(pShopUpdateItemPacket->title);
		CPythonShop::Instance().SetShopType(pShopUpdateItemPacket->type);
		CPythonShop::Instance().SetOfflineShopItemData(pShopUpdateItemPacket->pos, pShopUpdateItemPacket->item);
		CPythonShop::Instance().SetCurrentOfflineShopMoney(pShopUpdateItemPacket->price);
		if (pShopUpdateItemPacket->log.price != 0) {
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AppendShopLogFirst", Py_BuildValue("(ssiiL)", pShopUpdateItemPacket->log.name, pShopUpdateItemPacket->log.date, pShopUpdateItemPacket->log.itemVnum, pShopUpdateItemPacket->log.itemCount, pShopUpdateItemPacket->log.price));
		}
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshOfflineShop", Py_BuildValue("()"));
	}
	break;

	case SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "NOT_ENOUGH_MONEY"));
		break;

	case SHOP_SUBHEADER_GC_SOLD_OUT:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "SOLDOUT"));
		break;

	case SHOP_SUBHEADER_GC_OK:
		//PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "SOLDOUT"));
		break;

	case SHOP_SUBHEADER_GC_INVENTORY_FULL:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "INVENTORY_FULL"));
		break;

	case SHOP_SUBHEADER_GC_INVALID_POS:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnShopError", Py_BuildValue("(s)", "INVALID_POS"));
		break;

	case SHOP_SUBHEADER_GC_REFRESH_MONEY:
	{
		long long llMoney = 0;
		if (!Recv(sizeof(long long), &llMoney))
			return false;
		CPythonShop::Instance().SetCurrentOfflineShopMoney(llMoney);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshOfflineShop", Py_BuildValue("()"));
	}
	break;

	case SHOP_SUBHEADER_GC_CHECK_RESULT:
	{
		BYTE bHasOfflineShop = 0;
		if (!Recv(sizeof(BYTE), &bHasOfflineShop))
			return false;
		CPythonShop::instance().SetHasOfflineShop(bHasOfflineShop == 1 ? true : false);
	}
	break;

	case SHOP_SUBHEADER_GC_REFRESH_DISPLAY_COUNT:
	{
		DWORD displayCount = 0;
		if (!Recv(sizeof(DWORD), &displayCount))
			return false;
		CPythonShop::Instance().SetShopDisplayedCount(displayCount);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshOfflineShop", Py_BuildValue("()"));
	}
	break;
	case SHOP_SUBHEADER_GC_REALWATCHER_COUNT:
	{
		DWORD realWatcher = 0;
		if (!Recv(sizeof(DWORD), &realWatcher))
			return false;
		CPythonShop::Instance().SetRealWatcherCount(realWatcher);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshOfflineShop", Py_BuildValue("()"));
	}
	break;

	case SHOP_SUBHEADER_GC_REFRESH_COUNT:
	{
		DWORD count[2] = { 0,0 };
		if (!Recv(sizeof(count), &count))
			return false;
		CPythonShop::Instance().SetShopDisplayedCount(count[0]);
		CPythonShop::Instance().SetRealWatcherCount(count[1]);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshOfflineShop", Py_BuildValue("()"));
	}
	break;

	case SHOP_SUBHEADER_GC_CHANGE_TITLE:
	{
		char sign[SHOP_SIGN_MAX_LEN + 1];
		if (!Recv(sizeof(sign), &sign))
			return false;
		CPythonShop::Instance().SetShopSign(sign);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshOfflineShop", Py_BuildValue("()"));
	}
	break;

	default:
		TraceError("CPythonNetworkStream::RecvOfflineShopPacket: Unknown subheader %d\n", packet_shop.subheader);
		break;
	}

	return true;
}
#endif
#ifdef ENABLE_EVENT_MANAGER
bool CPythonNetworkStream::RecvEventManager()
{
	TPacketGCEventManager p;
	if (!Recv(sizeof(TPacketGCEventManager), &p))
		return false;

	BYTE subIndex;
	if (!Recv(sizeof(BYTE), &subIndex))
		return false;

	if (subIndex == EVENT_MANAGER_LOAD)
	{
		BYTE dayCount;
		if (!Recv(sizeof(BYTE), &dayCount))
			return false;
		int serverTime;
		if (!Recv(sizeof(int), &serverTime))
			return false;

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ClearEventManager", Py_BuildValue("()"));
		for (BYTE j = 0; j < dayCount; ++j)
		{
			BYTE dayIndex;
			if (!Recv(sizeof(BYTE), &dayIndex))
				return false;
			BYTE dayEventCount;
			if (!Recv(sizeof(BYTE), &dayEventCount))
				return false;
			if (dayEventCount > 0)
			{
				std::vector<TEventManagerData> m_vec;
				m_vec.resize(dayEventCount);
				if (!Recv(dayEventCount * sizeof(TEventManagerData), &m_vec[0]))
					return false;
				for(DWORD x=0;x<m_vec.size();++x)
				{
					const auto & eventPtr = m_vec[x];
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AppendEvent", Py_BuildValue("(iiissiiiiiiiii)", dayIndex, eventPtr.eventID, eventPtr.eventIndex, eventPtr.startTimeText, eventPtr.endTimeText, eventPtr.empireFlag, eventPtr.channelFlag, eventPtr.value[0], eventPtr.value[1], eventPtr.value[2], eventPtr.value[3], eventPtr.startTime - serverTime, eventPtr.endTime == 0 ? 0 : eventPtr.endTime - serverTime, eventPtr.eventStatus));
				}
			}
		}
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshEventManager", Py_BuildValue("()"));
	}
	else if (subIndex == EVENT_MANAGER_EVENT_STATUS)
	{
		WORD eventID;
		if (!Recv(sizeof(WORD), &eventID))
			return false;
		bool eventStatus;
		if (!Recv(sizeof(bool), &eventStatus))
			return false;
		int endTime;
		if (!Recv(sizeof(int), &endTime))
			return false;
		char endTimeText[25];
		if (!Recv(sizeof(endTimeText), &endTimeText))
			return false;
		int serverTime;
		if (!Recv(sizeof(int), &serverTime))
			return false;
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshEventStatus", Py_BuildValue("(iiis)", eventID, eventStatus, endTime == 0 ? 0 : endTime - serverTime, endTimeText));
	}
	return true;
}
#endif

#ifdef ENABLE_SKILLBOOK_COMB_SYSTEM
bool CPythonNetworkStream::SendSkillBookCombinationPacket(TItemPos* pPos, BYTE bAction)
{
	TPacketCGSkillBookCombination SkillBookCombCGPacket;
	SkillBookCombCGPacket.bHeader = HEADER_CG_SKILLBOOK_COMB;
	SkillBookCombCGPacket.bAction = bAction;
	memcpy(SkillBookCombCGPacket.CombItemGrid, pPos, sizeof(TItemPos) * SKILLBOOK_COMB_SLOT_MAX);

	if (!Send(sizeof(SkillBookCombCGPacket), &SkillBookCombCGPacket))
		return false;

	return SendSequence();
}
#endif

#if defined(__BL_TRANSMUTATION__)
#define TransmutationPacket(x) TPacketCGTransmutation p(static_cast<std::underlying_type_t<ECG_TRANSMUTATION_SHEADER>>(ECG_TRANSMUTATION_SHEADER::x))

bool CPythonNetworkStream::SendChangeLookCheckInPacket(const TItemPos& pos, const BYTE bSlotType)
{
	TransmutationPacket(ITEM_CHECK_IN);
	p.slot_type = bSlotType;
	p.pos = pos;

	if (!Send(sizeof(p), &p))
	{
		Tracef("CPythonNetworkStream::SendChangeLookCheckInPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendChangeLookCheckOutPacket(const BYTE bSlotType)
{
	TransmutationPacket(ITEM_CHECK_OUT);
	p.slot_type = bSlotType;

	if (!Send(sizeof(p), &p))
	{
		Tracef("CPythonNetworkStream::SendChangeLookCheckOutPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendChangeLookCheckInFreeYangItemPacket(const TItemPos& pos)
{
	TransmutationPacket(FREE_ITEM_CHECK_IN);
	p.pos = pos;

	if (!Send(sizeof(p), &p))
	{
		Tracef("CPythonNetworkStream::SendChangeLookCheckInFreeYangItemPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendChangeLookCheckOutFreeYangItemPacket()
{
	TransmutationPacket(FREE_ITEM_CHECK_OUT);

	if (!Send(sizeof(p), &p))
	{
		Tracef("CPythonNetworkStream::SendChangeLookCheckOutFreeYangItemPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendChangeLookAcceptPacket()
{
	TransmutationPacket(ACCEPT);

	if (!Send(sizeof(p), &p))
	{
		Tracef("CPythonNetworkStream::SendChangeLookAcceptPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendChangeLookCancelPacket()
{
	TransmutationPacket(CANCEL);

	if (!Send(sizeof(p), &p))
	{
		Tracef("CPythonNetworkStream::SendChangeLookCancelPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RecvChangeLookItemSetPacket()
{
	TPacketGCTransmutationItemSet kItemSet;
	if (!Recv(sizeof(kItemSet), &kItemSet))
		return false;

	if (kItemSet.bSlotType >= static_cast<BYTE>(ETRANSMUTATIONSLOTTYPE::TRANSMUTATION_SLOT_MAX))
		return true;

	CPythonPlayer::Instance().SetChangeLookItemData(kItemSet);

	__RefreshInventoryWindow();

	return true;
}

bool CPythonNetworkStream::RecvChangeLookItemDelPacket()
{
	TPacketGCTransmutationDel kItemDel;
	if (!Recv(sizeof(kItemDel), &kItemDel))
		return false;

	CPythonPlayer::Instance().DelChangeLookItemData(kItemDel.bSlotType, true);

	__RefreshInventoryWindow();
	Tracef(" >> CPythonNetworkStream::RecvChangeLookItemDelPacket\n");

	return true;
}

bool CPythonNetworkStream::RecvChangeLookFreeItemSetPacket()
{
	TPacketGCTransmutationItemSet kItemSet;
	if (!Recv(sizeof(kItemSet), &kItemSet))
		return false;

	CPythonPlayer::Instance().SetChangeLookFreeItemData(kItemSet);

	__RefreshInventoryWindow();

	return true;
}

bool CPythonNetworkStream::RecvChangeLookFreeItemDelPacket()
{
	TPacketGCTransmutationDel kItemDel;
	if (!Recv(sizeof(kItemDel), &kItemDel))
		return false;

	CPythonPlayer::Instance().DelChangeLookFreeItemData(true);

	__RefreshInventoryWindow();
	Tracef(" >> CPythonNetworkStream::RecvChangeLookFreeItemDelPacket\n");

	return true;
}
#endif

#ifdef ENABLE_SHOP_SEARCH_SYSTEM
bool CPythonNetworkStream::SendPrivateShopSearchInfo(TPacketCGShopSearch* p)
{
	if (!__CanActMainInstance())
		return true;
	if (!Send(sizeof(TPacketCGShopSearch), p))
	{
		#ifdef ENABLE_CONSOLE_LOG_WINDOW
		Tracen("SendPrivateShopSearchInfo Error");
		#endif
		return false;
	}

	return true;
}

bool CPythonNetworkStream::RecvShopSearchSet()
{
	TPacketGCShopSearchItemSet packet_item_set;
	if (!Recv(sizeof(TPacketGCShopSearchItemSet), &packet_item_set))
		return false;
	for (BYTE a = 0; a < packet_item_set.count; a++)
		CPythonPrivateShopSearch::Instance().AddItemData(packet_item_set.items[a]);
	__RefreshShopSearchWindow();
	return true;
}
#endif

#ifdef ENABLE_ITEMSHOP
bool CPythonNetworkStream::RecvItemShop()
{
	TPacketGCItemShop p;
	if (!Recv(sizeof(TPacketGCItemShop), &p))
		return false;
	
	BYTE subIndex;
	if (!Recv(sizeof(BYTE), &subIndex))
		return false;

	switch (subIndex)
	{
		case ITEMSHOP_DRAGONCOIN:
		{
			long long dragonCoin;
			if (!Recv(sizeof(long long), &dragonCoin))
				return false;
			bool isLogOpen;
			if (!Recv(sizeof(bool), &isLogOpen))
				return false;

			if (isLogOpen)
			{
				TIShopLogData logData;
				if (!Recv(sizeof(TIShopLogData), &logData))
					return false;
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopAppendLog", Py_BuildValue("(sissiiL)", logData.buyDate, logData.buyTime, logData.playerName, logData.ipAdress, logData.itemVnum, logData.itemCount, logData.itemPrice));
			}

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopSetDragonCoin", Py_BuildValue("(L)", dragonCoin));
		}
		break;
		
		case ITEMSHOP_LOG:
		{
			int logCount;
			if (!Recv(sizeof(int), &logCount))
				return false;
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopHideLoading", Py_BuildValue("()"));
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopLogClear", Py_BuildValue("()"));
			if (logCount)
			{
				std::vector<TIShopLogData> m_vec;
				m_vec.resize(logCount);
				if (!Recv(sizeof(TIShopLogData)*logCount, &m_vec[0]))
					return false;
				for (DWORD j = 0; j < m_vec.size(); ++j)
					PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopAppendLog", Py_BuildValue("(sissiiL)", m_vec[j].buyDate, m_vec[j].buyTime, m_vec[j].playerName, m_vec[j].ipAdress, m_vec[j].itemVnum, m_vec[j].itemCount, m_vec[j].itemPrice));
			}
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopPurchasesWindow", Py_BuildValue("()"));
		}
		break;
		
		case ITEMSHOP_UPDATE_ITEM:
		{
			TIShopData updateItem;
			if (!Recv(sizeof(TIShopData), &updateItem))
				return false;
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopUpdateItem", Py_BuildValue("(iiLiiiiLi)",
				updateItem.id, updateItem.itemVnum, updateItem.itemPrice, updateItem.discount, updateItem.offerTime, updateItem.topSellingIndex, updateItem.addedTime,
				updateItem.sellCount, updateItem.maxSellCount));
		}
		break;
		
case ITEMSHOP_LOAD:
		{
			long long dragonCoin;
			if (!Recv(sizeof(long long), &dragonCoin))
				return false;
			int updateTime;
			if (!Recv(sizeof(int), &updateTime))
				return false;
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopSetDragonCoin", Py_BuildValue("(L)", dragonCoin));

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopHideLoading", Py_BuildValue("()"));
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopClear", Py_BuildValue("(i)", updateTime));

			int categoryTotalSize;
			if (!Recv(sizeof(int), &categoryTotalSize))
				return false;

			if (categoryTotalSize == 9999)
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopOpenMainPage", Py_BuildValue("()"));
				return true;
			}

			for (DWORD j = 0; j < categoryTotalSize; ++j)
			{
				BYTE categoryIndex, categorySize;
				if (!Recv(sizeof(BYTE), &categoryIndex))
					return false;
				if (!Recv(sizeof(BYTE), &categorySize))
					return false;
				for (DWORD x = 0; x < categorySize; ++x)
				{
					BYTE categorySubIndex, categorySubSize;
					if (!Recv(sizeof(BYTE), &categorySubIndex))
						return false;
					if (!Recv(sizeof(BYTE), &categorySubSize))
						return false;

					std::vector<TIShopData> m_vec;
					if (categorySubSize)
					{
						m_vec.resize(categorySubSize);
						if (!Recv(sizeof(TIShopData) * categorySubSize, &m_vec[0]))
							return false;
						for (DWORD k = 0; k < m_vec.size(); ++k)
						{
							const TIShopData& shopData = m_vec[k];
							PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopAppendItem", Py_BuildValue("(iiiiLiiiiLi)",
								categoryIndex, categorySubIndex, shopData.id, shopData.itemVnum, shopData.itemPrice, shopData.discount, shopData.offerTime, shopData.topSellingIndex, shopData.addedTime,
								shopData.sellCount, shopData.maxSellCount));
						}
					}
				}
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "ItemShopOpenMainPage", Py_BuildValue("()"));
			}
		}
		break;
	}

	return true;
}
#endif
#ifdef ENABLE_FISHING_TIMER
bool CPythonNetworkStream::RecvFishingTimer()
{
	TPacketGCFishingTimer packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_GetFishingTimer", Py_BuildValue("(iii)", packet.vid, packet.subHeader, packet.timer));

	return true;
}
#endif

#ifdef BOT_KILLER
bool CPythonNetworkStream::RecvBotKillerPacket()
{
	TPacketGCBotKiller pack;

	if (!Recv(sizeof(TPacketGCBotKiller), &pack))
		return false;

	// Note the "s#" formatter! This safely passes the raw binary BMP data to Python.
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_BotKiller_Show", 
		Py_BuildValue("(s#ii)", pack.captchaImage, 2110, pack.wTimeout, pack.dwSessionKey));

	return true;
}

bool CPythonNetworkStream::SendBotKillerAnswerPacket(WORD wX, WORD wY, WORD wMouseDistance, DWORD dwSessionKey)
{
	TPacketCGBotKiller pack;
	pack.bHeader = HEADER_CG_BOT_KILLER;
	pack.wX = wX;
	pack.wY = wY;
	pack.wMouseDistance = wMouseDistance;
	pack.dwSessionKey = dwSessionKey; 

	if (!Send(sizeof(pack), &pack))
		return false;

	return SendSequence();
}
#endif
#ifdef ENABLE_CUBE_RENEWAL_WORLDARD

bool CPythonNetworkStream::CubeRenewalMakeItem(int index_item, int count_item, int index_item_improve)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGCubeRenewalSend	packet;

	packet.header		= HEADER_CG_CUBE_RENEWAL;
	packet.subheader	= CUBE_RENEWAL_SUB_HEADER_MAKE_ITEM;
	packet.index_item 	= index_item;
	packet.count_item   = count_item;
	packet.index_item_improve	= index_item_improve;

	if (!Send(sizeof(TPacketCGCubeRenewalSend), &packet))
	{
		Tracef("CPythonNetworkStream::CubeRenewalMakeItem Error\n");
		return false;
	}

	return true;
}
bool CPythonNetworkStream::CubeRenewalClose()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGCubeRenewalSend	packet;

	packet.header		= HEADER_CG_CUBE_RENEWAL;
	packet.subheader	= CUBE_RENEWAL_SUB_HEADER_CLOSE;

	if (!Send(sizeof(TPacketCGCubeRenewalSend), &packet))
	{
		Tracef("CPythonNetworkStream::CubeRenewalClose Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::RecvCubeRenewalPacket()
{

	TPacketGCCubeRenewalReceive CubeRenewalPacket;

	if (!Recv(sizeof(CubeRenewalPacket), &CubeRenewalPacket))
		return false;

	switch (CubeRenewalPacket.subheader)
	{

		case CUBE_RENEWAL_SUB_HEADER_OPEN_RECEIVE:
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_CUBE_RENEWAL_OPEN", Py_BuildValue("()"));
		}
		break;

		case CUBE_RENEWAL_SUB_HEADER_DATES_RECEIVE:
		{
			CPythonCubeRenewal::Instance().ReceiveList(CubeRenewalPacket.date_cube_renewal);
		}
		break;

		case CUBE_RENEWAL_SUB_HEADER_DATES_LOADING:
		{
			CPythonCubeRenewal::Instance().LoadingList();
		}
		break;

		case CUBE_RENEWAL_SUB_HEADER_CLEAR_DATES_RECEIVE:
		{
			CPythonCubeRenewal::Instance().ClearList();
		}
		break;
	}

	return true;
}

#endif

#if defined(ENABLE_NPC_LOCATION_HELPER)
bool CPythonNetworkStream::SendNPCLocationHelperPacket(BYTE bSubHeader, int lMapIndex, DWORD dwVnum, int lX, int lY)
{
	TPacketCGNPCLocationHelper packet;
	packet.header = HEADER_CG_NPC_LOCATION_HELPER;
	packet.subheader = bSubHeader;
	packet.mapIndex = lMapIndex;
	packet.vnum = dwVnum;
	packet.x = lX;
	packet.y = lY;

	if (!Send(sizeof(packet), &packet))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvNPCLocationHelperPacket()
{
	TPacketGCNPCLocationHelper packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	if (!m_apoPhaseWnd[PHASE_WINDOW_GAME])
		return true;

	if (!CPythonNPCLocationHelper::InstancePtr())
		return true;

	if (packet.subheader == NPC_LOCATION_HELPER_GC_STATUS)
	{
		CPythonNPCLocationHelper::Instance().SetStatus(packet);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshNPCLocationHelperStatus", Py_BuildValue("()"));
	}
	else if (packet.subheader == NPC_LOCATION_HELPER_GC_WARP_RESULT)
	{
		CPythonNPCLocationHelper::Instance().SetWarpResult(packet);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnNPCLocationHelperWarpResult", Py_BuildValue("(i)", packet.result));
	}
	else if (packet.subheader == NPC_LOCATION_HELPER_GC_GUILD_LAND)
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshNPCLocationHelperGuildLand", Py_BuildValue("(ii)", packet.mapIndex, packet.result));
	}

	return true;
}
#endif

#ifdef ENABLE_SWITCHBOT
bool CPythonNetworkStream::RecvSwitchbotPacket()
{
	TPacketGCSwitchbot pack;
	if (!Recv(sizeof(pack), &pack))
	{
		return false;
	}

	size_t packet_size = int(pack.size) - sizeof(TPacketGCSwitchbot);
	if (pack.subheader == SUBHEADER_GC_SWITCHBOT_UPDATE)
	{
		if (packet_size != sizeof(CPythonSwitchbot::TSwitchbotTable))
		{
			return false;
		}

		CPythonSwitchbot::TSwitchbotTable table;
		if (!Recv(sizeof(table), &table))
		{
			return false;
		}

		CPythonSwitchbot::Instance().Update(table);
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSwitchbotWindow", Py_BuildValue("()"));
	}
	else if (pack.subheader == SUBHEADER_GC_SWITCHBOT_UPDATE_ITEM)
	{
		if (packet_size != sizeof(TSwitchbotUpdateItem))
		{
			return false;
		}

		TSwitchbotUpdateItem update;
		if (!Recv(sizeof(update), &update))
		{
			return false;
		}

		TItemPos pos(SWITCHBOT, update.slot);

		IAbstractPlayer& rkPlayer = IAbstractPlayer::GetSingleton();
		rkPlayer.SetItemCount(pos, update.count);

		for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
		{
			rkPlayer.SetItemMetinSocket(pos, i, update.alSockets[i]);

		}

		for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
		{
			rkPlayer.SetItemAttribute(pos, j, update.aAttr[j].bType, update.aAttr[j].sValue);
		}

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSwitchbotItem", Py_BuildValue("(i)", update.slot));
		return true;
	}
	else if (pack.subheader == SUBHEADER_GC_SWITCHBOT_SEND_ATTRIBUTE_INFORMATION)
	{
		CPythonSwitchbot::Instance().ClearAttributeMap();

		size_t table_size = sizeof(CPythonSwitchbot::TSwitchbottAttributeTable);
		while (packet_size >= table_size)
		{
			const int test = sizeof(CPythonSwitchbot::TSwitchbottAttributeTable);

			CPythonSwitchbot::TSwitchbottAttributeTable table;
			if (!Recv(table_size, &table))
			{
				return false;
			}

			CPythonSwitchbot::Instance().AddAttributeToMap(table);
			packet_size -= table_size;
		}
	}

	return true;
}
bool CPythonNetworkStream::SendSwitchbotStartPacket(BYTE slot, std::vector<CPythonSwitchbot::TSwitchbotAttributeAlternativeTable> alternatives)
{
	TPacketCGSwitchbot pack;
	pack.header = HEADER_CG_SWITCHBOT;
	pack.subheader = SUBHEADER_CG_SWITCHBOT_START;
	pack.size = sizeof(TPacketCGSwitchbot) + sizeof(CPythonSwitchbot::TSwitchbotAttributeAlternativeTable) * SWITCHBOT_ALTERNATIVE_COUNT;
	pack.slot = slot;

	if (!Send(sizeof(pack), &pack))
	{
		return false;
	}

	for (const auto& it : alternatives)
	{
		if (!Send(sizeof(it), &it))
		{
			return false;
		}
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendSwitchbotStopPacket(BYTE slot)
{
	TPacketCGSwitchbot pack;
	pack.header = HEADER_CG_SWITCHBOT;
	pack.subheader = SUBHEADER_CG_SWITCHBOT_STOP;
	pack.size = sizeof(TPacketCGSwitchbot);
	pack.slot = slot;

	if (!Send(sizeof(pack), &pack))
	{
		return false;
	}

	return SendSequence();
}
#endif

#ifdef ENABLE_MINI_GAME_CATCH_KING
bool CPythonNetworkStream::SendMiniGameCatchKingStart(int ibetNumber)
{
	TPacketCGMiniGameCatchKing kMiniGamePacket;
	kMiniGamePacket.bHeader = HEADER_CG_MINI_GAME_CATCH_KING;
	kMiniGamePacket.bSubheader = 0; // SUBHEADER_CG_CATCH_KING_START
	kMiniGamePacket.bSubArgument = ibetNumber;

	if (!Send(sizeof(TPacketCGMiniGameCatchKing), &kMiniGamePacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendMiniGameCatchKingDeckCardClick()
{
	TPacketCGMiniGameCatchKing kMiniGamePacket;
	kMiniGamePacket.bHeader = HEADER_CG_MINI_GAME_CATCH_KING;
	kMiniGamePacket.bSubheader = 1; // SUBHEADER_CG_CATCH_KING_DECKCARD_CLICK
	kMiniGamePacket.bSubArgument = 0;

	if (!Send(sizeof(TPacketCGMiniGameCatchKing), &kMiniGamePacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendMiniGameCatchKingFieldCardClick(int icardNumber)
{
	TPacketCGMiniGameCatchKing kMiniGamePacket;
	kMiniGamePacket.bHeader = HEADER_CG_MINI_GAME_CATCH_KING;
	kMiniGamePacket.bSubheader = 2; // SUBHEADER_CG_CATCH_KING_FIELDCARD_CLICK
	kMiniGamePacket.bSubArgument = icardNumber;

	if (!Send(sizeof(TPacketCGMiniGameCatchKing), &kMiniGamePacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendMiniGameCatchKingReward()
{
	TPacketCGMiniGameCatchKing kMiniGamePacket;
	kMiniGamePacket.bHeader = HEADER_CG_MINI_GAME_CATCH_KING;
	kMiniGamePacket.bSubheader = 3; // SUBHEADER_CG_CATCH_KING_REWARD
	kMiniGamePacket.bSubArgument = 0;

	if (!Send(sizeof(TPacketCGMiniGameCatchKing), &kMiniGamePacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvMiniGameCatchKingPacket()
{
	TPacketGCMiniGameCatchKing kMiniGamePacket;
	if (!Recv(sizeof(TPacketGCMiniGameCatchKing), &kMiniGamePacket))
		return false;

	int iPacketSize = int(kMiniGamePacket.wSize) - sizeof(TPacketGCMiniGameCatchKing);
	if (iPacketSize <= 0)
		return true;

	// SECURITY FIX: Read exact bytes into a generic payload buffer.
	// This guarantees the TCP stream buffer advances correctly regardless of struct padding.
	char payload[256];
	if (iPacketSize > sizeof(payload))
		iPacketSize = sizeof(payload); // Prevent buffer overflow

	if (!Recv(iPacketSize, payload))
		return false;

	switch (kMiniGamePacket.bSubheader)
	{
		case 0: // SUBHEADER_GC_CATCH_KING_EVENT_INFO
		{
			bool bIsEnable = *(bool*)payload;
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameCatchKingEvent", Py_BuildValue("(b)", bIsEnable));
			break;
		}
		case 1: // SUBHEADER_GC_CATCH_KING_START
		{
			DWORD dwBigScore = *(DWORD*)payload;
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameCatchKingEventStart", Py_BuildValue("(i)", dwBigScore));
			break;
		}
		case 2: // SUBHEADER_GC_CATCH_KING_SET_CARD
		{
			BYTE bCardInHand = *(BYTE*)payload;
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameCatchKingSetHandCard", Py_BuildValue("(i)", bCardInHand));
			break;
		}
		case 3: // SUBHEADER_GC_CATCH_KING_RESULT_FIELD
		{
			// Safe explicit mapping using the packed struct from Packet.h
			TPacketGCMiniGameCatchKingResult* pRes = (TPacketGCMiniGameCatchKingResult*)payload;
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameCatchKingResultField", Py_BuildValue("(iiiibbbb)",
				pRes->dwPoints, pRes->bRowType, pRes->bCardPos, pRes->bCardValue,
				pRes->bKeepFieldCard, pRes->bDestroyHandCard, pRes->bGetReward, pRes->bIsFiveNearBy));
			break;
		}
		case 4: // SUBHEADER_GC_CATCH_KING_SET_END_CARD
		{
			// Safe explicit mapping using the packed struct from Packet.h
			TPacketGCMiniGameCatchKingSetEndCard* pEnd = (TPacketGCMiniGameCatchKingSetEndCard*)payload;
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameCatchKingSetEndCard", Py_BuildValue("(ii)", pEnd->bCardPos, pEnd->bCardValue));
			break;
		}
		case 5: // SUBHEADER_GC_CATCH_KING_REWARD
		{
			BYTE bReturnCode = *(BYTE*)payload;
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameCatchKingReward", Py_BuildValue("(i)", bReturnCode));
			break;
		}
	}

	return true;
}

bool CPythonNetworkStream::SendOpenCatchKing()
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "StartCatchKing", Py_BuildValue("()"));
	return true;
}
#endif

#ifdef ENABLE_MINI_GAME_YUTNORI
bool CPythonNetworkStream::SendMiniGameYutNori(BYTE bSubHeader, BYTE bSubArgument)
{
	if (!__CanActMainInstance())
		return true;
	
	TPacketCGMiniGameYutNori packet;
	packet.bHeader = HEADER_CG_MINI_GAME_YUT_NORI;
	packet.bSubheader = bSubHeader;
	packet.bSubArgument = bSubArgument;

	if (!Send(sizeof(TPacketCGMiniGameYutNori), &packet))
	{
		Tracef("SendMiniGameYutNori Send Packet Error\n");
		return false;
	}

	return true;
}

bool CPythonNetworkStream::RecvMiniGameYutNoriPacket()
{
	std::vector<char> vecBuffer;
	vecBuffer.clear();

    TPacketGCMiniGameYutNori packet;
	if (!Recv(sizeof(packet), &packet))
		return false;

	int iSize = packet.wSize - sizeof(packet);
	if (iSize > 0)
	{
		vecBuffer.resize(iSize);
		if (!Recv(iSize, &vecBuffer[0]))
			return false;
	}

	switch (packet.bSubheader)
	{		
		case SUBHEADER_GC_START_GAME:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameYutNoriProcess", Py_BuildValue("(i,())", 0));
			break;
			
		case SUBHEADER_GC_END_GAME:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameYutNoriProcess", Py_BuildValue("(i,())", 1));
			break;
			
		case SUBHEADER_GC_THROW_YUT:
			if (vecBuffer.size() >= sizeof(TPacketGCMiniGameYutNoriThrow))
			{
				TPacketGCMiniGameYutNoriThrow * packSecond = (TPacketGCMiniGameYutNoriThrow *)&vecBuffer[0];
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameYutNoriProcess", Py_BuildValue("(i,(bi))", 2, packSecond->bIsPc, packSecond->cYut));
			}
			break;
			
		case SUBHEADER_GC_NEXT_TURN:
			if (vecBuffer.size() >= sizeof(TPacketGCMiniGameYutNoriNextTurn))
			{
				TPacketGCMiniGameYutNoriNextTurn * packSecond = (TPacketGCMiniGameYutNoriNextTurn *)&vecBuffer[0];
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameYutNoriProcess", Py_BuildValue("(i,(bi))", 3, packSecond->bPcTurn, packSecond->bState));
			}
			break;
			
		case SUBHEADER_GC_YUT_MOVE:
			if (vecBuffer.size() >= sizeof(TPacketGCMiniGameYutNoriMove))
			{
				TPacketGCMiniGameYutNoriMove * packSecond = (TPacketGCMiniGameYutNoriMove *)&vecBuffer[0];
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameYutNoriProcess", Py_BuildValue("(i,(bibii))", 
					5, packSecond->bIsPc, packSecond->bUnitIndex, packSecond->bIsCatch, packSecond->bStartIndex, packSecond->bDestIndex));
			}
			break;
			
		case SUBHEADER_GC_SHOW_AVAILABLE_AREA:
			if (vecBuffer.size() >= sizeof(TPacketGCMiniGameYutNoriAvailableArea))
			{
				TPacketGCMiniGameYutNoriAvailableArea * packSecond = (TPacketGCMiniGameYutNoriAvailableArea *)&vecBuffer[0];
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameYutNoriProcess", Py_BuildValue("(i,(ii))", 
					6, packSecond->bPlayerIndex, packSecond->bAvailableIndex));
			}
			break;
			
		case SUBHEADER_GC_PUSH_CATCH_YUT:
			if (vecBuffer.size() >= sizeof(TPacketGCMiniGameYutNoriPushCatchYut))
			{
				TPacketGCMiniGameYutNoriPushCatchYut * packSecond = (TPacketGCMiniGameYutNoriPushCatchYut *)&vecBuffer[0];
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameYutNoriProcess", Py_BuildValue("(i,(ii))", 7, packSecond->bIsPc, packSecond->bUnitIndex));
			}
			break;
			
		case SUBHEADER_GC_SET_PROB:
			if (vecBuffer.size() >= sizeof(BYTE))
			{
				BYTE bProbIndex = *(BYTE *)&vecBuffer[0];
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameYutNoriProcess", Py_BuildValue("(ii)", 8, bProbIndex));
			}
			break;
			
		case SUBHEADER_GC_SET_SCORE:
			if (vecBuffer.size() >= sizeof(WORD))
			{
				WORD wScore = *(WORD *)&vecBuffer[0];
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameYutNoriProcess", Py_BuildValue("(ii)", 9, wScore));
			}
			break;
			
		case SUBHEADER_GC_SET_REMAIN_COUNT:
			if (vecBuffer.size() >= sizeof(BYTE))
			{
				BYTE bRemainCount = *(BYTE *)&vecBuffer[0];
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameYutNoriProcess", Py_BuildValue("(ii)", 10, bRemainCount));
			}
			break;
			
		default:
			TraceError("CPythonNetworkStream::RecvMiniGameYutNoriPacket: Unknown subheader - %d\n", packet.bSubheader);
			break;
	}

	return true;
}

bool CPythonNetworkStream::RecvMiniGameYutNoriSetYut()
{
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameYutNoriProcess", Py_BuildValue("(i,())", 4));
	return true;
}
#endif

#ifdef ENABLE_TAROT
bool CPythonNetworkStream::SendMinigameBnwStart()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGMinigameBnw packet;
	packet.header = HEADER_CG_MINIGAME_BNW;
	packet.subheader = MINIGAME_BNW_SUBHEADER_CG_START;

	if (!Send(sizeof(packet), &packet))
	{
		Tracen("SendMinigameBnwStart Error");
		return false;
	}
	return SendSequence();
}

bool CPythonNetworkStream::SendMinigameBnwCardSelected(BYTE bCard)
{
	if (!__CanActMainInstance())
		return true;

	if (bCard >= 9)
		return false;

	TPacketCGMinigameBnw packet;
	packet.header = HEADER_CG_MINIGAME_BNW;
	packet.subheader = MINIGAME_BNW_SUBHEADER_CG_SELECTED_CARD;

	if (!Send(sizeof(packet), &packet))
	{
		Tracen("SendMinigameBnwCardSelected Error");
		return false;
	}

	if (!Send(sizeof(BYTE), &bCard))
	{
		Tracen("SendMinigameBnwCardSelected Error2");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendMinigameBnwFinished()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGMinigameBnw packet;
	packet.header = HEADER_CG_MINIGAME_BNW;
	packet.subheader = MINIGAME_BNW_SUBHEADER_CG_FINISHED;

	if (!Send(sizeof(packet), &packet))
	{
		Tracen("SendMinigameBnwFinished Error");
		return false;
	}
	return SendSequence();
}

bool CPythonNetworkStream::RecvMinigameBnw()
{
	TPacketGCMinigameBnw p;
	if (!Recv(sizeof(p), &p))
		return false;

	switch (p.subheader)
	{
		case MINIGAME_BNW_SUBHEADER_GC_START:
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_MinigameBnwStart", Py_BuildValue("()"));
			break;
		}
		case MINIGAME_BNW_SUBHEADER_GC_DRAW_RESULT:
		{
			TPacketGCMinigameBnwDrawResult result;
			if (!Recv(sizeof(TPacketGCMinigameBnwDrawResult), &result))
				return false;

			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_MinigameBnwDrawResult", Py_BuildValue("(iii)", result.result, result.playerPoints, result.enemyPoints));
			break;
		}
		default:
			TraceError("CPythonNetworkStream::RecvMinigameBnw: Unknown subheader %d", p.subheader);
			return false;
	}
	return true;
}
#endif

#ifdef ENABLE_AURA_SYSTEM
bool CPythonNetworkStream::RecvAuraPacket()
{
	TPacketGCAura kAuraPacket;
	if (!Recv(sizeof(TPacketGCAura), &kAuraPacket))
	{
		Tracef("Aura Packet Error SubHeader %u\n", kAuraPacket.bSubHeader);
		return false;
	}

	int iPacketSize = int(kAuraPacket.wSize) - sizeof(TPacketGCAura);

	switch (kAuraPacket.bSubHeader)
	{
	case AURA_SUBHEADER_GC_OPEN:
	case AURA_SUBHEADER_GC_CLOSE:
	{
		if (iPacketSize > 0)
		{
			TSubPacketGCAuraOpenClose kSubPacket;
			assert(iPacketSize % sizeof(TSubPacketGCAuraOpenClose) == 0 && "AURA_SUBHEADER_GC_OPENCLOSE");
			if (!Recv(sizeof(TSubPacketGCAuraOpenClose), &kSubPacket))
				return false;

			if (kAuraPacket.bSubHeader == AURA_SUBHEADER_GC_OPEN)
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AuraWindowOpen", Py_BuildValue("(i)", kSubPacket.bAuraWindowType));

			else if (kAuraPacket.bSubHeader == AURA_SUBHEADER_GC_CLOSE)
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "AuraWindowClose", Py_BuildValue("()"));
				CPythonPlayer::instance().DelAuraItemData(AURA_SLOT_MAIN);
				CPythonPlayer::instance().DelAuraItemData(AURA_SLOT_SUB);
				CPythonPlayer::instance().DelAuraItemData(AURA_SLOT_RESULT);
			}

			CPythonPlayer::instance().SetAuraRefineWindowOpen(kSubPacket.bAuraWindowType);
		}
		else
			TraceError(" RecvAuraPacket Error 0x04100%u0F", kAuraPacket.bSubHeader);

		break;
	}
	case AURA_SUBHEADER_GC_SET_ITEM:
	{
		if (iPacketSize > 0)
		{
			TSubPacketGCAuraSetItem kSubPacket;
			assert(iPacketSize % sizeof(TSubPacketGCAuraSetItem) == 0 && "AURA_SUBHEADER_GC_SET_ITEM");
			if (!Recv(sizeof(TSubPacketGCAuraSetItem), &kSubPacket))
				return false;

			TItemData kItemData;
			kItemData.vnum = kSubPacket.pItem.vnum;
			kItemData.count = kSubPacket.pItem.count;
			for (int iSocket = 0; iSocket < ITEM_SOCKET_SLOT_MAX_NUM; ++iSocket)
				kItemData.alSockets[iSocket] = kSubPacket.pItem.alSockets[iSocket];
			for (int iAttr = 0; iAttr < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++iAttr)
				kItemData.aAttr[iAttr] = kSubPacket.pItem.aAttr[iAttr];

			if (kSubPacket.Cell.IsValidCell() && !kSubPacket.Cell.IsEquipCell())
				CPythonPlayer::instance().SetActivatedAuraSlot(BYTE(kSubPacket.AuraCell.cell), kSubPacket.Cell);

			CPythonPlayer::instance().SetAuraItemData(BYTE(kSubPacket.AuraCell.cell), kItemData);
		}
		else
			TraceError(" RecvAuraPacket Error 0x040%uBABE", kAuraPacket.bSubHeader);

		break;
	}
	case AURA_SUBHEADER_GC_CLEAR_SLOT:
	{
		if (iPacketSize > 0)
		{
			TSubPacketGCAuraClearSlot kSubPacket;
			assert(iPacketSize % sizeof(TSubPacketGCAuraClearSlot) == 0 && "AURA_SUBHEADER_GC_CLEAR_SLOT");
			if (!Recv(sizeof(TSubPacketGCAuraClearSlot), &kSubPacket))
				return false;

			CPythonPlayer::instance().DelAuraItemData(kSubPacket.bAuraSlotPos);
		}
		else
			TraceError(" RecvAuraPacket Error 0x04FF0%uAA", kAuraPacket.bSubHeader);

		break;
	}
	case AURA_SUBHEADER_GC_CLEAR_ALL:
	{
		CPythonPlayer::instance().DelAuraItemData(AURA_SLOT_MAIN);
		CPythonPlayer::instance().DelAuraItemData(AURA_SLOT_SUB);
		CPythonPlayer::instance().DelAuraItemData(AURA_SLOT_RESULT);
		break;
	}
	case AURA_SUBHEADER_GC_CLEAR_RIGHT:
	{
		if (iPacketSize == 0)
			CPythonPlayer::instance().DelAuraItemData(AURA_SLOT_SUB);
		else
			TraceError("invalid packet size %d", iPacketSize);
		break;
	}
	case AURA_SUBHEADER_GC_REFINE_INFO:
	{
		if (iPacketSize > 0)
		{
			for (size_t i = 0; iPacketSize > 0; ++i)
			{
				assert(iPacketSize % sizeof(TSubPacketGCAuraRefineInfo) == 0 && "AURA_SUBHEADER_GC_REFINE_INFO");
				TSubPacketGCAuraRefineInfo kSubPacket;
				if (!Recv(sizeof(TSubPacketGCAuraRefineInfo), &kSubPacket))
					return false;

				CPythonPlayer::instance().SetAuraRefineInfo(kSubPacket.bAuraRefineInfoType, kSubPacket.bAuraRefineInfoLevel, kSubPacket.bAuraRefineInfoExpPercent);
				iPacketSize -= sizeof(TSubPacketGCAuraRefineInfo);
			}

		}
		else
			TraceError(" RecvAuraPacket Error 0x04000%FF", kAuraPacket.bSubHeader);

		break;
	}
	}

	__RefreshInventoryWindow();

	return true;
}

bool CPythonNetworkStream::SendAuraRefineCheckIn(TItemPos InventoryCell, TItemPos AuraCell, BYTE byAuraRefineWindowType)
{
	__PlayInventoryItemDropSound(InventoryCell);

	TPacketCGAura kAuraPacket;
	kAuraPacket.wSize = sizeof(TPacketCGAura) + sizeof(TSubPacketCGAuraRefineCheckIn);
	kAuraPacket.bSubHeader = AURA_SUBHEADER_CG_REFINE_CHECKIN;

	TSubPacketCGAuraRefineCheckIn kAuraSubPacket;
	kAuraSubPacket.ItemCell = InventoryCell;
	kAuraSubPacket.AuraCell = AuraCell;
	kAuraSubPacket.byAuraRefineWindowType = byAuraRefineWindowType;

	if (!Send(sizeof(TPacketCGAura), &kAuraPacket))
		return false;

	if (!Send(sizeof(TSubPacketCGAuraRefineCheckIn), &kAuraSubPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendAuraRefineCheckOut(TItemPos AuraCell, BYTE byAuraRefineWindowType)
{
	TPacketCGAura kAuraPacket;
	kAuraPacket.wSize = sizeof(TPacketCGAura) + sizeof(TSubPacketCGAuraRefineCheckOut);
	kAuraPacket.bSubHeader = AURA_SUBHEADER_CG_REFINE_CHECKOUT;

	TSubPacketCGAuraRefineCheckOut kAuraSubPacket;
	kAuraSubPacket.AuraCell = AuraCell;
	kAuraSubPacket.byAuraRefineWindowType = byAuraRefineWindowType;

	if (!Send(sizeof(TPacketCGAura), &kAuraPacket))
		return false;

	if (!Send(sizeof(TSubPacketCGAuraRefineCheckOut), &kAuraSubPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendAuraRefineAccept(BYTE byAuraRefineWindowType)
{
	TPacketCGAura kAuraPacket;
	kAuraPacket.wSize = sizeof(TPacketCGAura) + sizeof(TSubPacketCGAuraRefineAccept);
	kAuraPacket.bSubHeader = AURA_SUBHEADER_CG_REFINE_ACCEPT;

	TSubPacketCGAuraRefineAccept kAuraSubPacket;
	kAuraSubPacket.byAuraRefineWindowType = byAuraRefineWindowType;

	if (!Send(sizeof(TPacketCGAura), &kAuraPacket))
		return false;

	if (!Send(sizeof(TSubPacketCGAuraRefineAccept), &kAuraSubPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendAuraRefineCancel()
{
	TPacketCGAura kAuraPacket;
	kAuraPacket.wSize = sizeof(TPacketCGAura);
	kAuraPacket.bSubHeader = AURA_SUBHEADER_CG_REFINE_CANCEL;

	if (!Send(sizeof(TPacketCGAura), &kAuraPacket))
		return false;

	return SendSequence();
}
#endif