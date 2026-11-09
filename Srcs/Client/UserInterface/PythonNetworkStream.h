#pragma once

#include "../eterLib/FuncObject.h"
#include "../eterlib/NetStream.h"
#include "../eterLib/NetPacketHeaderMap.h"

#include "InsultChecker.h"

#include "packet.h"
#ifdef ENABLE_SWITCHBOT
#include "PythonSwitchbot.h"
#endif

class CInstanceBase;
class CNetworkActorManager;
struct SNetworkActorData;
struct SNetworkUpdateActorData;

class CPythonNetworkStream : public CNetworkStream, public CSingleton<CPythonNetworkStream>
{
	public:
		enum
		{
			SERVER_COMMAND_LOG_OUT = 0,
			SERVER_COMMAND_RETURN_TO_SELECT_CHARACTER = 1,
			SERVER_COMMAND_QUIT = 2,

			MAX_ACCOUNT_PLAYER
		};

		enum
		{
			ERROR_NONE,
			ERROR_UNKNOWN,
			ERROR_CONNECT_MARK_SERVER,
			ERROR_LOAD_MARK,
			ERROR_MARK_WIDTH,
			ERROR_MARK_HEIGHT,

			// MARK_BUG_FIX
			ERROR_MARK_UPLOAD_NEED_RECONNECT,
			ERROR_MARK_CHECK_NEED_RECONNECT,
			// END_OF_MARK_BUG_FIX
		};

		enum
		{
			ACCOUNT_CHARACTER_SLOT_ID,
			ACCOUNT_CHARACTER_SLOT_NAME,
			ACCOUNT_CHARACTER_SLOT_RACE,
			ACCOUNT_CHARACTER_SLOT_LEVEL,
			ACCOUNT_CHARACTER_SLOT_STR,
			ACCOUNT_CHARACTER_SLOT_DEX,
			ACCOUNT_CHARACTER_SLOT_HTH,
			ACCOUNT_CHARACTER_SLOT_INT,
			ACCOUNT_CHARACTER_SLOT_PLAYTIME,
			ACCOUNT_CHARACTER_SLOT_FORM,
			ACCOUNT_CHARACTER_SLOT_ADDR,
			ACCOUNT_CHARACTER_SLOT_PORT,
			ACCOUNT_CHARACTER_SLOT_GUILD_ID,
			ACCOUNT_CHARACTER_SLOT_GUILD_NAME,
			ACCOUNT_CHARACTER_SLOT_CHANGE_NAME_FLAG,
			ACCOUNT_CHARACTER_SLOT_HAIR,
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
			ACCOUNT_CHARACTER_SLOT_ACCE,
#endif
#ifdef ENABLE_CONQUEROR_LEVEL
			ACCOUNT_CHARACTER_SLOT_CONQUEROR_LEVEL,
			ACCOUNT_CHARACTER_SLOT_SUNGMA_ST,
			ACCOUNT_CHARACTER_SLOT_SUNGMA_HP,
			ACCOUNT_CHARACTER_SLOT_SUNGMA_MOVE,
			ACCOUNT_CHARACTER_SLOT_SUNGMA_INMUNE,
#endif
#ifdef ENABLE_AURA_SYSTEM
			ACCOUNT_CHARACTER_SLOT_AURA,
#endif
		};

		enum
		{
			PHASE_WINDOW_LOGO,
			PHASE_WINDOW_LOGIN,
			PHASE_WINDOW_SELECT,
			PHASE_WINDOW_CREATE,
			PHASE_WINDOW_LOAD,
			PHASE_WINDOW_GAME,
			PHASE_WINDOW_EMPIRE,
			PHASE_WINDOW_NUM,
		};

	public:
		CPythonNetworkStream();
		virtual ~CPythonNetworkStream();

		bool SendSpecial(int nLen, void * pvBuf);

		void StartGame();
		void Warp(LONG lGlobalX, LONG lGlobalY);

		void NotifyHack(const char* c_szMsg);
		void SetWaitFlag();

		#ifdef ENABLE_DISCORD_RPC
		void Discord_Start();
		void Discord_Close();
		void Discord_Update(const bool ingame);
		#endif

		void SendEmoticon(UINT eEmoticon);

		void ExitApplication();
		void ExitGame();
		void LogOutGame();
		void AbsoluteExitGame();
		void AbsoluteExitApplication();

		void EnableChatInsultFilter(bool isEnable);
		bool IsChatInsultIn(const char* c_szMsg);
		bool IsInsultIn(const char* c_szMsg);

		DWORD GetGuildID();

		UINT UploadMark(const char* c_szImageFileName);
		UINT UploadSymbol(const char* c_szImageFileName);

		bool LoadInsultList(const char* c_szInsultListFileName);
		bool LoadConvertTable(DWORD dwEmpireID, const char* c_szFileName);

		UINT		GetAccountCharacterSlotDatau(UINT iSlot, UINT eType);
		const char* GetAccountCharacterSlotDataz(UINT iSlot, UINT eType);

		// SUPPORT_BGM
		const char*		GetFieldMusicFileName();
		float			GetFieldMusicVolume();
		// END_OF_SUPPORT_BGM

		bool IsSelectedEmpire();

		void ToggleGameDebugInfo();

#if defined(__BL_PICK_FILTER__)
		void OpenPickUpWindow();
#endif
		void SetMarkServer(const char* c_szAddr, UINT uPort);
		void ConnectLoginServer(const char* c_szAddr, UINT uPort);
		void ConnectGameServer(UINT iChrSlot);

		void SetLoginInfo(const char* c_szID, const char* c_szPassword);
		void SetLoginKey(DWORD dwLoginKey);
		void ClearLoginInfo( void );

		void SetHandler(PyObject* poHandler);
		void SetPhaseWindow(UINT ePhaseWnd, PyObject* poPhaseWnd);
		void ClearPhaseWindow(UINT ePhaseWnd, PyObject* poPhaseWnd);
		void SetServerCommandParserWindow(PyObject* poPhaseWnd);

		bool SendSyncPositionElementPacket(DWORD dwVictimVID, DWORD dwVictimX, DWORD dwVictimY);

		bool SendAttackPacket(UINT uMotAttack, DWORD dwVIDVictim);
		bool SendCharacterStatePacket(const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg);
		bool SendUseSkillPacket(DWORD dwSkillIndex, DWORD dwTargetVID=0);
		bool SendTargetPacket(DWORD dwVID);

		// OLDCODE:
		bool SendCharacterStartWalkingPacket(float fRotation, long lx, long ly);
		bool SendCharacterEndWalkingPacket(float fRotation, long lx, long ly);
		bool SendCharacterCheckWalkingPacket(float fRotation, long lx, long ly);

		bool SendCharacterPositionPacket(BYTE iPosition);
#ifdef ENABLE_SORT_INVEN
		bool SortInven(BYTE window_type, BYTE option);
#endif
		bool SendItemUsePacket(TItemPos pos);
#ifdef ENABLE_CHEST_OPEN_RENEWAL
		bool SendOpenChestPacket(TItemPos pos, WORD wOpenCount);
#endif
		bool SendItemUseToItemPacket(TItemPos source_pos, TItemPos target_pos);
		bool SendItemDropPacket(TItemPos pos, DWORD elk
			#if defined(ENABLE_CHEQUE_SYSTEM) && !defined(DISABLE_CHEQUE_DROP)
			, DWORD cheque = 0
			#endif
		);
		bool SendItemDropPacketNew(TItemPos pos, DWORD elk, DWORD count
			#if defined(ENABLE_CHEQUE_SYSTEM) && !defined(DISABLE_CHEQUE_DROP)
			, DWORD cheque = 0
			#endif
		);
		bool SendItemMovePacket(TItemPos pos, TItemPos change_pos, BYTE num);
#if defined(ENABLE_PARTY_MATCH)
		bool PartyMatch(BYTE subheader, DWORD targetPID, int mapIndex, const char* szDesc);
		bool RecvPartyMatch();
#endif
		bool SendItemPickUpPacket(DWORD vid);

		bool SendQuickSlotAddPacket(BYTE wpos, BYTE type, BYTE pos);
		bool SendQuickSlotDelPacket(BYTE wpos);
		bool SendQuickSlotMovePacket(BYTE wpos, BYTE change_pos);

		bool SendPointResetPacket();

		// Shop
		bool SendShopEndPacket();
		bool SendShopBuyPacket(BYTE byCount);
		bool SendShopSellPacket(BYTE bySlot);
#ifdef ENABLE_SPECIAL_STORAGE
		bool SendShopSellPacketNew(BYTE bySlot, BYTE byCount, BYTE byType);
#else
		bool SendShopSellPacketNew(BYTE bySlot, BYTE byCount);
#endif
#ifdef ENABLE_GEM_SYSTEM
		bool SendGemShopBuy(BYTE bPos);
		bool SendGemShopAdd(BYTE bPos);
		bool SendGemShopRefresh();
#endif
#ifdef ENABLE_FISH_EVENT
		bool SendFishBoxUse(BYTE bWindow, WORD wCell);
		bool SendFishShapeAdd(BYTE bPos);
#endif
#ifdef ENABLE_OFFLINESHOP_SYSTEM
	// Offline Shop
		bool SendOfflineShopEndPacket();
		bool SendOfflineShopBuyPacket(DWORD vid, BYTE pos);
		bool SendAddOfflineShopItem(TItemPos bDisplayPos, BYTE bPos, long long lPrice);
		bool SendRemoveOfflineShopItem(BYTE bPos);
		bool SendOpenShopSlot(BYTE bPos);
		bool SendGetBackItems();
		bool SendAddTime();
		bool SendDestroyOfflineShop();
		bool SendTakeOfflineMoney();
		bool SendOfflineShopChangeTitle(const char* title);
		bool SendOfflineShopCheck();
		bool SendOfflineShopButton();
		bool SendOfflineShopRemoveLogs();
		bool SendOfflineOpenWithVID(DWORD vid);
		bool SendOfflineShopChangeDecoration(const char* sign, DWORD vnum, DWORD type);
#endif
#ifdef ENABLE_DS_SET
		bool	RecvDSTablePacket();
#endif
		// Exchange
		bool SendExchangeStartPacket(DWORD vid);
		bool SendExchangeItemAddPacket(TItemPos ItemPos, BYTE byDisplayPos);
		bool SendExchangeElkAddPacket(DWORD elk);
#ifdef ENABLE_CHEQUE_SYSTEM
		bool SendExchangeChequeAddPacket(DWORD elk);
#endif
		bool SendExchangeItemDelPacket(BYTE pos);
#ifdef ENABLE_EVENT_MANAGER
		bool RecvEventManager();
#endif
		bool SendExchangeAcceptPacket();
		bool SendExchangeExitPacket();

		// Quest
		bool SendScriptAnswerPacket(int iAnswer);
		bool SendScriptButtonPacket(unsigned int iIndex);
		bool SendAnswerMakeGuildPacket(const char * c_szName);
		bool SendQuestInputStringPacket(const char * c_szString);
		bool SendQuestConfirmPacket(BYTE byAnswer, DWORD dwPID);

		// Event
		bool SendOnClickPacket(DWORD vid);
#ifdef ENABLE_MINI_GAME_CATCH_KING
		bool SendMiniGameCatchKingStart(int ibetNumber);
		bool SendMiniGameCatchKingDeckCardClick();
		bool SendMiniGameCatchKingFieldCardClick(int icardNumber);
		bool SendMiniGameCatchKingReward();
		bool RecvMiniGameCatchKingPacket();
		bool SendOpenCatchKing();
#endif
		// Fly
		bool SendFlyTargetingPacket(DWORD dwTargetVID, const TPixelPosition& kPPosTarget);
		bool SendAddFlyTargetingPacket(DWORD dwTargetVID, const TPixelPosition& kPPosTarget);
		bool SendShootPacket(UINT uSkill);

		// Command
		bool ClientCommand(const char * c_szCommand);
		void ServerCommand(const char * c_szCommand);

		// Emoticon
		void RegisterEmoticonString(const char * pcEmoticonString);

		// Party
		bool SendPartyInvitePacket(DWORD dwVID);
		bool SendPartyInviteAnswerPacket(DWORD dwLeaderVID, BYTE byAccept);
		bool SendPartyRemovePacket(DWORD dwPID);
		bool SendPartySetStatePacket(DWORD dwVID, BYTE byState, BYTE byFlag);
		bool SendPartyUseSkillPacket(BYTE bySkillIndex, DWORD dwVID);
		bool SendPartyParameterPacket(BYTE byDistributeMode);

		// SafeBox
		bool SendSafeBoxMoneyPacket(BYTE byState, DWORD dwMoney);
		bool SendSafeBoxCheckinPacket(TItemPos InventoryPos, BYTE bySafeBoxPos);
		bool SendSafeBoxCheckoutPacket(BYTE bySafeBoxPos, TItemPos InventoryPos);
		bool SendSafeBoxItemMovePacket(BYTE bySourcePos, BYTE byTargetPos, BYTE byCount);
#ifdef BOT_KILLER
		bool SendBotKillerAnswerPacket(WORD wX, WORD wY, WORD wMouseDistance, DWORD dwSessionKey);
#endif

		// Mall
		bool SendMallCheckoutPacket(BYTE byMallPos, TItemPos InventoryPos);

		// Guild
		bool SendGuildAddMemberPacket(DWORD dwVID);
		bool SendGuildRemoveMemberPacket(DWORD dwPID);
		bool SendGuildChangeGradeNamePacket(BYTE byGradeNumber, const char * c_szName);
		bool SendGuildChangeGradeAuthorityPacket(BYTE byGradeNumber, BYTE byAuthority);
		bool SendGuildOfferPacket(DWORD dwExperience);
		bool SendGuildPostCommentPacket(const char * c_szMessage);
		bool SendGuildDeleteCommentPacket(DWORD dwIndex);
		bool SendGuildRefreshCommentsPacket(DWORD dwHighestIndex);
		bool SendGuildChangeMemberGradePacket(DWORD dwPID, BYTE byGrade);
		bool SendGuildUseSkillPacket(DWORD dwSkillID, DWORD dwTargetVID);
		bool SendGuildChangeMemberGeneralPacket(DWORD dwPID, BYTE byFlag);
		bool SendGuildInvitePacket(DWORD dwVID);
		bool SendGuildInviteAnswerPacket(DWORD dwGuildID, BYTE byAnswer);
		bool SendGuildChargeGSPPacket(DWORD dwMoney);
		bool SendGuildDepositMoneyPacket(DWORD dwMoney);
		bool SendGuildWithdrawMoneyPacket(DWORD dwMoney);
#ifdef NEW_PET_SYSTEM
		bool PetSetNamePacket(const char * petname);
#endif
#if defined(__BL_TRANSMUTATION__)
		bool SendChangeLookCheckInPacket(const TItemPos& pos, const BYTE bSlotType);
		bool SendChangeLookCheckOutPacket(const BYTE bSlotType);
		bool SendChangeLookCheckInFreeYangItemPacket(const TItemPos& pos);
		bool SendChangeLookCheckOutFreeYangItemPacket();
		bool SendChangeLookAcceptPacket();
		bool SendChangeLookCancelPacket();
		bool RecvChangeLookItemSetPacket();
		bool RecvChangeLookItemDelPacket();
		bool RecvChangeLookFreeItemSetPacket();
		bool RecvChangeLookFreeItemDelPacket();
#endif
		// Mall
		bool RecvMallOpenPacket();
		bool RecvMallItemSetPacket();
		bool RecvMallItemDelPacket();
#ifdef ENABLE_IKASHOP_RENEWAL
		bool RecvOfflineshopPacket();

		bool RecvOfflineshopShopOpen();
		bool RecvOfflineshopShopOpenOwner();
		bool RecvOfflineshopShopOpenOwnerNoShop();
		bool RecvOfflineshopShopExpiredGuesting();
		bool RecvOfflineshopShopRemoveItem(bool owner);

		bool RecvOfflineshopShopGuestEditItem();
		bool RecvOfflineshopShopOwnerEditItem();


		bool RecvOfflineshopShopFilterResult();
		bool RecvOfflineshopOfferList();
		bool RecvOfflineshopShopSafeboxRefresh();
		bool RecvOfflineshopShopSafeboxRemoveItem();
		bool RecvOfflineshopShopSafeboxAddItem();

		bool RecvOfflineshopAuctionList();
		bool RecvOfflineshopOpenMyAuction();
		bool RecvOfflineshopOpenMyAuctionNoAuction();
		bool RecvOfflineshopOpenAuction();

		bool RecvOfflineshopPopupMessage();

		bool RecvOfflineshopBoardCounters();
#ifdef EXTEND_IKASHOP_PRO
		bool RecvOfflineshopNotificationList();
#endif
#ifdef EXTEND_IKASHOP_ULTIMATE
		bool RecvOfflineshopPriceAverageResponse();
#endif
#ifdef ENABLE_IKASHOP_ENTITIES
		bool RecvOfflineshopInsertEntity();
		bool RecvOfflineshopRemoveEntity();

		void SendIkarusShopOnClickShopEntity(DWORD dwPickedShopVID);
#ifdef EXTEND_IKASHOP_PRO
		void SendIkarusShopMoveShopEntity();
#endif

		bool RecvOfflineshopSearchResultDelete();
#endif
#ifdef ENABLE_IKASHOP_RENEWAL_TITEL
		bool RecvOfflineshopTitle();
		bool RecvOfflineshopTitleChange();
#endif



		void SendIkarusShopCreateShop();
		void SendIkarusShopForceCloseShop();

		void SendIkarusShopRequestShopList();

		void SendIkarusShopOpenShop(DWORD dwOwnerID);
		void SendIkarusShopOpenShopOwner();

		void SendIkarusShopBuyItem(DWORD dwOwnerID, DWORD dwItemID, bool isSearch, long long TotalPriceSeen); //patch seen price check

		void SendIkarusShopAddItem(ikashop::TShopItemInfo& itemInfo);
		void SendIkarusShopRemoveItem(DWORD dwItemID);
		void SendOfflineShopEditItem(DWORD dwItemID, const ikashop::TPriceInfo& price);

		void SendIkarusShopFilterRequest(const ikashop::TFilterInfo& filter);
		void SendIkarusShopSearchFillRequest();

		void SendIkarusShopOfferCreate(const ikashop::TOfferInfo& offer);
		void SendIkarusShopOfferAccept(DWORD dwOfferID);
		void SendIkarusShopOfferCancel(DWORD dwOfferID, DWORD dwOwnerID);
		void SendIkarusShopOfferListRequest();

		void SendIkarusShopSafeboxOpen();
		void SendIkarusShopSafeboxGetItem(DWORD dwItemID);
		void SendIkarusShopSafeboxGetValutes();
		void SendIkarusShopSafeboxClose();

		//AUCTION
		void SendIkarusShopAuctionListRequest();
		void SendIkarusShopAuctionOpen(DWORD dwOwnerID);
		void SendIkarusShopAuctionAddOffer(DWORD dwOwnerID, const ikashop::TPriceInfo& price);
		void SendIkarusShopAuctionExitFrom(DWORD dwOwnerID);
		void SendIkarusShopAuctionCreate(const TItemPos& pos, const ikashop::TPriceInfo& price);
		void SendIkarusShopMyAuctionOpen();
		void SendIkarusShopMyAuctionClose();
		void SendIkarusShopMyAuctionCancel();
#ifdef ENABLE_IKASHOP_RENEWAL_TITEL
		void SendIkarusShopTitleChange(const char* szTitle, BYTE bTitleType);
#endif
		void SendIkarusMyShopCloseBoard();
		void SendIkarusOffersListClose();
		void SendIkarusCloseShopGuestBoard();
#ifdef EXTEND_IKASHOP_PRO
		void SendIkarusNotificationListRequest();
		void SendIkarusNotificationListClose();
#endif
#ifdef EXTEND_IKASHOP_ULTIMATE
		void SendPriceAverageRequest(DWORD vnum, DWORD count);
		void SendShopDecorationUsage(int index);
		void SendShopMoveItem(int srcPos, int destPos);
#endif
#endif

		// Lover
#ifdef ENABLE_CONQUEROR_LEVEL
		bool RecvSungmaAttrUpdatePacket();
#endif
		bool RecvLoverInfoPacket();
		bool RecvLovePointUpdatePacket();

		// Dig
		bool RecvDigMotionPacket();
#if defined(__BL_KILL_BAR__)
		bool RecvKillBar();
#endif
		// Fishing
		bool SendFishingPacket(int iRotation);
		bool SendGiveItemPacket(DWORD dwTargetVID, TItemPos ItemPos, int iItemCount);
#ifdef ENABLE_OFFLINESHOP_SYSTEM
	// Offline Shop
		bool SendBuildOfflineShopPacket(const char* c_szName, const std::vector<TOfflineShopItemTable>& c_rSellingItemStock, DWORD shopVnum, BYTE shopTitle);
#endif

		// Private Shop
		bool SendBuildPrivateShopPacket(const char * c_szName, const std::vector<TShopItemTable> & c_rSellingItemStock);

		// Refine
		bool SendRefinePacket(BYTE byPos, BYTE byType);
		bool SendSelectItemPacket(DWORD dwItemPos);
#if defined(__BL_OFFICIAL_LOOT_FILTER__)
		bool SendLootFilterPacket(BYTE(&settings)[ELootFilter::LOOT_FILTER_SETTINGS_MAX]);
		bool RecvLootFilterPacket();
#endif
#if defined(__BL_MAILBOX__)
		bool RecvMailboxProcess();
		bool RecvMailbox();
		bool RecvMailboxAddData();
		bool RecvMailboxAll();
		bool RecvMailboxUnread();
		bool SendPostDelete(const BYTE Index);
		bool SendPostGetItems(const BYTE Index);
		bool SendPostWriteConfirm(const char* szName);
		bool SendPostWrite(const char* szName, const char* szTitle, const char* szMessage, const TItemPos& pos, const int iYang, const int iWon);
		bool SendMailBoxClose();
		bool SendPostAllDelete();
		bool SendPostAllGetItems();
		bool RequestPostAddData(const BYTE ButtonIndex, const BYTE DataIndex);
#endif
#if defined(__BL_MOVE_COSTUME_ATTR__)
		bool SendItemCombinationPacket(const int iMediumIndex, const int iBaseIndex, const int iMaterialIndex);
		bool SendItemCombinationPacketCancel();
#endif
#if defined(__BL_67_ATTR__)
		bool Send67AttrPacket(int iMaterialCount, int iSupportCount, int iSupportPos, int iItemPos);
		bool Send67AttrClosePacket();
		bool Recv67AttrOpenPacket();
#endif
		// Client Version
		bool SendClientVersionPacket();

		// CRC Report
		bool __SendCRCReportPacket();

		bool SendDragonSoulRefinePacket(BYTE bRefineType, TItemPos* pos);

		// Handshake
		bool RecvHandshakePacket();
		bool RecvHandshakeOKPacket();

		bool RecvHybridCryptKeyPacket();
		bool RecvHybridCryptSDBPacket();
#ifdef _IMPROVED_PACKET_ENCRYPTION_
		bool RecvKeyAgreementPacket();
		bool RecvKeyAgreementCompletedPacket();

#endif
		// ETC
		DWORD GetMainActorVID();
		DWORD GetMainActorRace();
		DWORD GetMainActorEmpire();
		DWORD GetMainActorSkillGroup();
		void SetEmpireID(DWORD dwEmpireID);
		DWORD GetEmpireID();
		void __TEST_SetSkillGroupFake(int iIndex);
#ifdef ENABLE_CUBE_RENEWAL_WORLDARD
		bool CubeRenewalMakeItem(int index_item, int count_item, int index_item_improve);
		bool CubeRenewalClose();
		bool RecvCubeRenewalPacket();
#endif
#ifdef ELEMENT_SPELL_WORLDARD
		bool 	RecvElementsSpellPacket();
		bool 	SendElementsSpellClose();
		bool 	SendElementsSpellAdd(int pos);
		bool 	SendElementsSpellChange(int pos, int type_select);
#endif
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		bool	SendAcceClosePacket();
		bool	SendAcceAddPacket(TItemPos tPos, BYTE bPos);
		bool	SendAcceRemovePacket(BYTE bPos);
		bool	SendAcceRefinePacket();
#endif
#ifdef ENABLE_MINI_GAME_YUTNORI
		bool SendMiniGameYutNori(BYTE bSubHeader, BYTE bSubArgument);
		bool RecvMiniGameYutNoriSetYut();
#endif
#ifdef ENABLE_TAROT
	public:
		bool SendMinigameBnwStart();
		bool SendMinigameBnwCardSelected(BYTE bCard);
		bool SendMinigameBnwFinished();

	protected:
		bool RecvMinigameBnw();
#endif
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	public:
		void SetOffLinePhase();
		void SetHandShakePhase();
		void SetLoginPhase();
		void SetSelectPhase();
		void SetLoadingPhase();
		void SetGamePhase();
		void ClosePhase();

		// Login Phase
		bool SendLoginPacket(const char * c_szName, const char * c_szPassword);
		bool SendLoginPacketNew(const char * c_szName, const char * c_szPassword);
		bool SendDirectEnterPacket(const char * c_szName, const char * c_szPassword, UINT uChrSlot);

		bool SendEnterGame();

		// Select Phase
		bool SendSelectEmpirePacket(DWORD dwEmpireID);
		bool SendSelectCharacterPacket(BYTE account_Index);
		bool SendChangeNamePacket(BYTE index, const char *name);
		bool SendCreateCharacterPacket(BYTE index, const char *name, BYTE job, BYTE shape, BYTE byStat1, BYTE byStat2, BYTE byStat3, BYTE byStat4);
		bool SendDestroyCharacterPacket(BYTE index, const char * szPrivateCode);

		// Main Game Phase
		bool SendC2CPacket(DWORD dwSize, void * pData);
		bool SendChatPacket(const char * c_szChat, BYTE byType = CHAT_TYPE_TALKING);
		bool SendWhisperPacket(const char * name, const char * c_szChat);
#if defined(WJ_COMBAT_ZONE)
		bool SendCombatZoneRequestActionPacket(int action, int value);
#endif
		bool SendMessengerAddByVIDPacket(DWORD vid);
		bool SendMessengerAddByNamePacket(const char * c_szName);
		bool SendMessengerRemovePacket(const char * c_szKey, const char * c_szName);
#if defined(ENABLE_MESSENGER_RENEWAL)
		void RequestMessengerRefresh();
		bool SendMessengerSetConnectionStatePacket(BYTE bConnectionState);
		bool SendMessengerSetStatusMessagePacket(const char* c_szMessage);
		bool SendMessengerDeleteStatusMessagePacket();
#endif
		#ifdef ENABLE_MESSENGER_BLOCK
		bool SendMessengerAddBlockByVIDPacket(DWORD vid);
		bool SendMessengerAddBlockByNamePacket(const char * c_szName);
		bool SendMessengerRemoveBlockPacket(const char * c_szKey, const char * c_szName);
		#endif
#ifdef ENABLE_SHOP_SEARCH_SYSTEM
		bool SendPrivateShopSearchInfo(TPacketCGShopSearch* p);
		bool RecvShopSearchSet();
#endif
#ifdef ENABLE_OFFLINESHOP_SYSTEM
		bool RecvOfflineShopPacket();
		bool RecvOfflineShopSignPacket();
#endif

	protected:
		bool OnProcess();
		void OffLinePhase();
		void HandShakePhase();
		void LoginPhase();
		void SelectPhase();
		void LoadingPhase();
		void GamePhase();

		bool __IsNotPing();

		void __DownloadMark();
		void __DownloadSymbol(const std::vector<DWORD> & c_rkVec_dwGuildID);

		void __PlayInventoryItemUseSound(TItemPos uSlotPos);
		void __PlayInventoryItemDropSound(TItemPos uSlotPos);
		//void __PlayShopItemDropSound(UINT uSlotPos);
		void __PlaySafeBoxItemDropSound(UINT uSlotPos);
		void __PlayMallItemDropSound(UINT uSlotPos);

		bool __CanActMainInstance();

		enum REFRESH_WINDOW_TYPE
		{
			RefreshStatus = (1 << 0),
			RefreshAlignmentWindow = (1 << 1),
			RefreshCharacterWindow = (1 << 2),
			RefreshEquipmentWindow = (1 << 3),
			RefreshInventoryWindow = (1 << 4),
			RefreshExchangeWindow = (1 << 5),
			RefreshSkillWindow = (1 << 6),
			RefreshSafeboxWindow  = (1 << 7),
			RefreshMessengerWindow = (1 << 8),
			RefreshGuildWindowInfoPage = (1 << 9),
			RefreshGuildWindowBoardPage = (1 << 10),
			RefreshGuildWindowMemberPage = (1 << 11),
			RefreshGuildWindowMemberPageGradeComboBox = (1 << 12),
			RefreshGuildWindowSkillPage = (1 << 13),
			RefreshGuildWindowGradePage = (1 << 14),
			RefreshTargetBoard = (1 << 15),
			RefreshMallWindow = (1 << 16),
		};

		void __RefreshStatus();
		void __RefreshAlignmentWindow();
		void __RefreshCharacterWindow();
		void __RefreshEquipmentWindow();
		void __RefreshInventoryWindow();
		void __RefreshExchangeWindow();
		void __RefreshSkillWindow();
		void __RefreshSafeboxWindow();
		void __RefreshMessengerWindow();
		void __RefreshGuildWindowInfoPage();
		void __RefreshGuildWindowBoardPage();
		void __RefreshGuildWindowMemberPage();
		void __RefreshGuildWindowMemberPageGradeComboBox();
		void __RefreshGuildWindowSkillPage();
		void __RefreshGuildWindowGradePage();
		void __RefreshTargetBoardByVID(DWORD dwVID);
		void __RefreshTargetBoardByName(const char * c_szName);
		void __RefreshTargetBoard();
#ifdef ENABLE_SHOP_SEARCH_SYSTEM
		void __RefreshShopSearchWindow();
#endif
		void __RefreshMallWindow();

#ifdef ENABLE_CSHIELD
		bool __SendHack(const DWORD errorcode, const BYTE win, std::string language, std::string report);
#else
		bool __SendHack(const char* c_szMsg);
#endif

	protected:
		bool RecvObserverAddPacket();
		bool RecvObserverRemovePacket();
		bool RecvObserverMovePacket();

		// Common
		bool RecvErrorPacket(int header);
		bool RecvPingPacket();
		bool RecvDefaultPacket(int header);
		bool RecvPhasePacket();

		// Login Phase
		bool __RecvLoginSuccessPacket3();
		bool __RecvLoginSuccessPacket4();
		bool __RecvLoginFailurePacket();
		bool __RecvEmpirePacket();
		bool __RecvLoginKeyPacket();

		// Select Phase
		bool __RecvPlayerCreateSuccessPacket();
		bool __RecvPlayerCreateFailurePacket();
		bool __RecvPlayerDestroySuccessPacket();
		bool __RecvPlayerDestroyFailurePacket();
		bool __RecvPreserveItemPacket();
		bool __RecvPlayerPoints();
		bool __RecvChangeName();
#if defined(WJ_COMBAT_ZONE)
		bool __RecvCombatZoneRankingPacket();
		bool __RecvCombatZonePacket();
#endif
		// Loading Phase
		bool RecvMainCharacter();
		bool RecvMainCharacter2_EMPIRE();
		bool RecvMainCharacter3_BGM();
		bool RecvMainCharacter4_BGM_VOL();

		void __SetFieldMusicFileName(const char* musicName);
		void __SetFieldMusicFileInfo(const char* musicName, float vol);
		// END_OF_SUPPORT_BGM

#if defined(ENABLE_VOICE_CHAT)
		bool RecvVoiceChatPacket();
#endif

		// Main Game Phase
		bool RecvWarpPacket();
		bool RecvPVPPacket();
		bool RecvDuelStartPacket();
        bool RecvGlobalTimePacket();
		bool RecvCharacterAppendPacket();
		bool RecvCharacterAdditionalInfo();
		bool RecvCharacterAppendPacketNew();
		bool RecvCharacterUpdatePacket();
		bool RecvCharacterDeletePacket();
		bool RecvChatPacket();
		bool RecvOwnerShipPacket();
		bool RecvSyncPositionPacket();
		bool RecvWhisperPacket();
		bool RecvPointChange();						// Alarm to python
		bool RecvChangeSpeedPacket();

		bool RecvStunPacket();
		bool RecvDeadPacket();
		bool RecvCharacterMovePacket();

		bool RecvItemSetPacket();					// Alarm to python
		bool RecvItemSetPacket2();					// Alarm to python
		bool RecvItemUsePacket();					// Alarm to python
		bool RecvItemUpdatePacket();				// Alarm to python
		bool RecvItemGroundAddPacket();
		bool RecvItemGroundDelPacket();
		bool RecvItemOwnership();

		bool RecvQuickSlotAddPacket();				// Alarm to python
		bool RecvQuickSlotDelPacket();				// Alarm to python
		bool RecvQuickSlotMovePacket();				// Alarm to python

		bool RecvCharacterPositionPacket();
		bool RecvMotionPacket();

		bool RecvShopPacket();
		bool RecvShopSignPacket();
		bool RecvExchangePacket();

		// Quest
		bool RecvScriptPacket();
		bool RecvQuestInfoPacket();
		bool RecvQuestConfirmPacket();
		bool RecvRequestMakeGuild();

		// Skill
		bool RecvSkillLevel();
		bool RecvSkillLevelNew();
		bool RecvSkillCoolTimeEnd();

		// Target
		bool RecvTargetPacket();
		bool RecvViewEquipPacket();
		bool RecvDamageInfoPacket();

		// Mount
		bool RecvMountPacket();

		// Fly
		bool RecvCreateFlyPacket();
		bool RecvFlyTargetingPacket();
		bool RecvAddFlyTargetingPacket();

		// Messenger
		bool RecvMessenger();

		// Guild
		bool RecvGuild();

		// Party
		bool RecvPartyInvite();
		bool RecvPartyAdd();
		bool RecvPartyUpdate();
		bool RecvPartyRemove();
		bool RecvPartyLink();
		bool RecvPartyUnlink();
		bool RecvPartyParameter();

		// SafeBox
		bool RecvSafeBoxSetPacket();
		bool RecvSafeBoxDelPacket();
		bool RecvSafeBoxWrongPasswordPacket();
		bool RecvSafeBoxSizePacket();
		bool RecvSafeBoxMoneyChangePacket();
#ifdef BOT_KILLER
		bool RecvBotKillerPacket();
#endif
#ifdef ENABLE_GEM_SYSTEM
		bool RecvGemShopOpen();
		bool RecvGemShopRefresh();
#endif
#ifdef ENABLE_FISH_EVENT
		bool RecvFishEventInfo();
#endif
#ifdef ENABLE_MINI_GAME_YUTNORI
		bool RecvMiniGameYutNoriPacket();
#endif
		// Fishing
		bool RecvFishing();
#ifdef ENABLE_ITEMSHOP
		bool RecvItemShop();

#endif
		// Dungeon
		bool RecvDungeon();

		// Time
		bool RecvTimePacket();

		// WalkMode
		bool RecvWalkModePacket();

		// ChangeSkillGroup
		bool RecvChangeSkillGroupPacket();

		// Refine
		bool RecvRefineInformationPacket();
		bool RecvRefineInformationPacketNew();

		// Use Potion
		bool RecvSpecialEffect();
#ifdef ENABLE_12ZI
		bool RecvSpecialZodiacEffect();
#endif
		bool RecvSpecificEffect();

		bool RecvDragonSoulRefine();

		// MiniMap Info
		bool RecvNPCList();
		bool RecvLandPacket();
		bool RecvTargetCreatePacket();
		bool RecvTargetCreatePacketNew();
		bool RecvTargetUpdatePacket();
		bool RecvTargetDeletePacket();

		// Affect
		bool RecvAffectAddPacket();
		bool RecvAffectRemovePacket();

		// Channel
		bool RecvChannelPacket();
#ifdef ENABLE_FISHING_TIMER
		bool RecvFishingTimer();
#endif
#ifdef ENABLE_GUILD_TOKEN_AUTH
		bool RecvGuildTokenPacket();
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		bool RecvAccePacket(bool bReturn = false);
#endif

	protected:
		bool ParseEmoticon(const char * pChatMsg, DWORD * pdwEmoticon);

		void OnConnectFailure();
		void OnScriptEventStart(int iSkin, int iIndex);

		void OnRemoteDisconnect();
		void OnDisconnect();

		void SetGameOnline();
		void SetGameOffline();
		BOOL IsGameOnline();

#ifdef ENABLE_SKILLBOOK_COMB_SYSTEM
	public:
		bool SendSkillBookCombinationPacket(TItemPos* pPos, BYTE bAction);
#endif
#if defined(ENABLE_NPC_LOCATION_HELPER)
		bool SendNPCLocationHelperPacket(BYTE bSubHeader, int lMapIndex, DWORD dwVnum, int lX, int lY);
		bool RecvNPCLocationHelperPacket();
#endif
	protected:
		bool CheckPacket(TPacketHeader * pRetHeader);

		void __InitializeGamePhase();
		void __InitializeMarkAuth();
		void __GlobalPositionToLocalPosition(LONG& rGlobalX, LONG& rGlobalY);
		void __LocalPositionToGlobalPosition(LONG& rLocalX, LONG& rLocalY);
#if defined(__ANDROID__) || defined(__LP64__) || defined(__aarch64__)
		inline void __GlobalPositionToLocalPosition(long& rGlobalX, long& rGlobalY) {
			LONG lx = (LONG)rGlobalX, ly = (LONG)rGlobalY;
			__GlobalPositionToLocalPosition(lx, ly);
			rGlobalX = lx; rGlobalY = ly;
		}
		inline void __LocalPositionToGlobalPosition(long& rLocalX, long& rLocalY) {
			LONG lx = (LONG)rLocalX, ly = (LONG)rLocalY;
			__LocalPositionToGlobalPosition(lx, ly);
			rLocalX = lx; rLocalY = ly;
		}
#endif

		bool __IsPlayerAttacking();
		bool __IsEquipItemInSlot(TItemPos Cell);

		void __ShowMapName(LONG lLocalX, LONG lLocalY);

		void __LeaveOfflinePhase() {}
		void __LeaveHandshakePhase() {}
		void __LeaveLoginPhase() {}
		void __LeaveSelectPhase() {}
		void __LeaveLoadingPhase() {}
		void __LeaveGamePhase();

		void __ClearNetworkActorManager();

		void __ClearSelectCharacterData();

		// DELETEME
		//void __SendWarpPacket();

		void __ConvertEmpireText(DWORD dwEmpireID, char* szText);

		void __RecvCharacterAppendPacket(SNetworkActorData * pkNetActorData);
		void __RecvCharacterUpdatePacket(SNetworkUpdateActorData * pkNetUpdateActorData);

		void __FilterInsult(char* szLine, UINT uLineLen);

		void __SetGuildID(DWORD id);
#ifdef ENABLE_GUILD_TOKEN_AUTH
		void __SetGuildToken(uint64_t token);
#endif

	protected:
		TPacketGCHandshake m_HandshakeData;
		DWORD m_dwChangingPhaseTime;
		DWORD m_dwBindupRetryCount;
		DWORD m_dwMainActorVID;
		DWORD m_dwMainActorRace;
		DWORD m_dwMainActorEmpire;
		DWORD m_dwMainActorSkillGroup;
		BOOL m_isGameOnline;
		BOOL m_isStartGame;

		DWORD m_dwGuildID;
#ifdef ENABLE_GUILD_TOKEN_AUTH
		uint64_t m_dwGuildToken{};
#endif
		DWORD m_dwEmpireID;

		struct SServerTimeSync
		{
			DWORD m_dwChangeServerTime;
			DWORD m_dwChangeClientTime;
		} m_kServerTimeSync;

		void __ServerTimeSync_Initialize();
		//DWORD m_dwBaseServerTime;
		//DWORD m_dwBaseClientTime;

		DWORD m_dwLastGamePingTime;

		std::string	m_stID;
		std::string	m_stPassword;
		std::string	m_strLastCommand;
		std::string	m_strPhase;
		DWORD m_dwLoginKey;
		BOOL m_isWaitLoginKey;

		std::string m_stMarkIP;

		CFuncObject<CPythonNetworkStream>	m_phaseProcessFunc;
		CFuncObject<CPythonNetworkStream>	m_phaseLeaveFunc;

		PyObject*							m_poHandler;
		PyObject*							m_apoPhaseWnd[PHASE_WINDOW_NUM];
		PyObject*							m_poSerCommandParserWnd;

		TSimplePlayerInformation			m_akSimplePlayerInfo[PLAYER_PER_ACCOUNT4];
		DWORD								m_adwGuildID[PLAYER_PER_ACCOUNT4];
		std::string							m_astrGuildName[PLAYER_PER_ACCOUNT4];
		bool m_bSimplePlayerInfo;

		CRef<CNetworkActorManager>			m_rokNetActorMgr;

		bool m_isRefreshStatus;
		bool m_isRefreshCharacterWnd;
		bool m_isRefreshEquipmentWnd;
		bool m_isRefreshInventoryWnd;
		bool m_isRefreshExchangeWnd;
		bool m_isRefreshSkillWnd;
		bool m_isRefreshSafeboxWnd;
		bool m_isRefreshMallWnd;
		bool m_isRefreshMessengerWnd;
		bool m_isRefreshGuildWndInfoPage;
		bool m_isRefreshGuildWndBoardPage;
		bool m_isRefreshGuildWndMemberPage;
		bool m_isRefreshGuildWndMemberPageGradeComboBox;
		bool m_isRefreshGuildWndSkillPage;
		bool m_isRefreshGuildWndGradePage;
#ifdef ENABLE_SHOP_SEARCH_SYSTEM
		bool m_isRefreshShopSearchWnd;
#endif
		// Emoticon
		std::vector<std::string> m_EmoticonStringVector;

		struct STextConvertTable
		{
			char acUpper[26];
			char acLower[26];
			BYTE aacHan[5000][2];
		} m_aTextConvTable[3];

		struct SMarkAuth
		{
			CNetworkAddress m_kNetAddr;
			DWORD m_dwHandle;
			DWORD m_dwRandomKey;
		} m_kMarkAuth;

		DWORD m_dwSelectedCharacterIndex;

		CInsultChecker m_kInsultChecker;

		bool m_isEnableChatInsultFilter;
		bool m_bComboSkillFlag;

		std::deque<std::string> m_kQue_stHack;

	private:
		struct SDirectEnterMode
		{
			bool m_isSet;
			DWORD m_dwChrSlotIndex;
		} m_kDirectEnterMode;

		void __DirectEnterMode_Initialize();
		void __DirectEnterMode_Set(UINT uChrSlotIndex);
		bool __DirectEnterMode_IsSet();

	public:
		DWORD EXPORT_GetBettingGuildWarValue(const char* c_szValueName);

	private:
		struct SBettingGuildWar
		{
			DWORD m_dwBettingMoney;
			DWORD m_dwObserverCount;
		} m_kBettingGuildWar;

#ifdef ENABLE_CSHIELD
		EncryptedPtr<CInstanceBase> m_pInstTarget;
#else
		CInstanceBase* m_pInstTarget;
#endif

		void __BettingGuildWar_Initialize();
		void __BettingGuildWar_SetObserverCount(UINT uObserverCount);
		void __BettingGuildWar_SetBettingMoney(UINT uBettingMoney);
#ifdef ENABLE_SWITCHBOT
	public:
		bool RecvSwitchbotPacket();

		bool SendSwitchbotStartPacket(BYTE slot, std::vector<CPythonSwitchbot::TSwitchbotAttributeAlternativeTable> alternatives);
		bool SendSwitchbotStopPacket(BYTE slot);
#endif
#ifdef ENABLE_CSHIELD
	public:
		bool SendCShieldPacket(bool start, DWORD num);

	protected:
		bool RecvCShieldPacket();
		bool RecvCShieldStopPlayerPacket();
#endif
#ifdef ENABLE_CAPTCHA
	public:
		bool SendCaptchaPacket(short captchaResult);

	protected:
		bool RecvCaptchaPacket();
#endif
#ifdef ENABLE_AURA_SYSTEM
	protected:
		bool RecvAuraPacket();

	public:
		bool SendAuraRefineCheckIn(TItemPos InventoryCell, TItemPos AuraCell, BYTE byAuraRefineWindowType);
		bool SendAuraRefineCheckOut(TItemPos AuraCell, BYTE byAuraRefineWindowType);
		bool SendAuraRefineAccept(BYTE byAuraRefineWindowType);
		bool SendAuraRefineCancel();
#endif
};
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
