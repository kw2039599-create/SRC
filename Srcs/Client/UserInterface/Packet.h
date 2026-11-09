#pragma once

#include "Locale.h"
#include "../gamelib/RaceData.h"
#include "../gamelib/ItemData.h"

typedef BYTE TPacketHeader;

enum
{
	/////////////////////////////////////////////////
	// To Server
	// HEADER_BLANK is the not use(for future use)
	HEADER_CG_LOGIN								= 1,
	HEADER_CG_ATTACK							= 2,
	HEADER_CG_CHAT								= 3,
	HEADER_CG_PLAYER_CREATE						= 4,
	HEADER_CG_PLAYER_DESTROY					= 5,
	HEADER_CG_PLAYER_SELECT						= 6,
	HEADER_CG_CHARACTER_MOVE					= 7,
	HEADER_CG_SYNC_POSITION  					= 8,
	HEADER_CG_DIRECT_ENTER						= 9,
	HEADER_CG_ENTERGAME							= 10,
	HEADER_CG_ITEM_USE							= 11,
	HEADER_CG_ITEM_DROP							= 12,
	HEADER_CG_ITEM_MOVE							= 13,
	HEADER_CG_ITEM_PICKUP						= 15,
	HEADER_CG_QUICKSLOT_ADD                     = 16,
	HEADER_CG_QUICKSLOT_DEL                     = 17,
	HEADER_CG_QUICKSLOT_SWAP                    = 18,
	HEADER_CG_WHISPER							= 19,
	HEADER_CG_ITEM_DROP2                        = 20,
#ifdef ENABLE_CHEST_OPEN_RENEWAL
	HEADER_CG_CHEST_OPEN			= 22,
#endif
	//HEADER_BLANK21								= 21,
	//HEADER_BLANK22								= 22,
	//HEADER_BLANK22								= 23,
	//HEADER_BLANK24								= 24,
	//HEADER_BLANK25								= 25,
	HEADER_CG_ON_CLICK							= 26,
	HEADER_CG_EXCHANGE							= 27,
    HEADER_CG_CHARACTER_POSITION                = 28,
    HEADER_CG_SCRIPT_ANSWER						= 29,
	HEADER_CG_QUEST_INPUT_STRING				= 30,
    HEADER_CG_QUEST_CONFIRM                     = 31,
	//HEADER_BLANK32								= 32,
	//HEADER_BLANK33								= 33,
	//HEADER_BLANK34								= 34,
	//HEADER_BLANK35								= 35,
	//HEADER_BLANK36								= 36,
	//HEADER_BLANK37								= 37,
	//HEADER_BLANK38								= 38,
	//HEADER_BLANK39								= 39,
	//HEADER_BLANK40								= 40,
	HEADER_CG_PVP								= 41,
	//HEADER_BLANK42								= 42,
	//HEADER_BLANK43								= 43,
	//HEADER_BLANK44								= 44,
	//HEADER_BLANK45								= 45,
	//HEADER_BLANK46								= 46,
	//HEADER_BLANK47								= 47,
	//HEADER_BLANK48								= 48,
	//HEADER_BLANK49								= 49,
    HEADER_CG_SHOP								= 50,
	HEADER_CG_FLY_TARGETING						= 51,
	HEADER_CG_USE_SKILL							= 52,
    HEADER_CG_ADD_FLY_TARGETING                 = 53,
	HEADER_CG_SHOOT								= 54,
	HEADER_CG_MYSHOP                            = 55,
#ifdef ENABLE_GEM_SYSTEM
	HEADER_CG_GEM_SHOP							= 56,
#endif
#ifdef ENABLE_OFFLINESHOP_SYSTEM
	HEADER_CG_OFFLINE_SHOP						= 57,
	HEADER_CG_MY_OFFLINE_SHOP					= 58,
#endif
	//HEADER_BLANK56								= 56,
	//HEADER_BLANK57								= 57,
	//HEADER_BLANK58								= 58,
	//HEADER_BLANK59								= 59,
	HEADER_CG_ITEM_USE_TO_ITEM					= 60,
    HEADER_CG_TARGET                            = 61,
	//HEADER_BLANK62								= 62,
	//HEADER_BLANK63								= 63,
	//HEADER_BLANK64								= 64,
	HEADER_CG_WARP								= 65,
    HEADER_CG_SCRIPT_BUTTON						= 66,
    HEADER_CG_MESSENGER                         = 67,
	//HEADER_BLANK68								= 68,
    HEADER_CG_MALL_CHECKOUT                     = 69,
    HEADER_CG_SAFEBOX_CHECKIN                   = 70,
    HEADER_CG_SAFEBOX_CHECKOUT                  = 71,
    HEADER_CG_PARTY_INVITE                      = 72,
    HEADER_CG_PARTY_INVITE_ANSWER               = 73,
    HEADER_CG_PARTY_REMOVE                      = 74,
    HEADER_CG_PARTY_SET_STATE                   = 75,
    HEADER_CG_PARTY_USE_SKILL                   = 76,
    HEADER_CG_SAFEBOX_ITEM_MOVE                 = 77,
	HEADER_CG_PARTY_PARAMETER                   = 78,
	//HEADER_BLANK68								= 79,
	HEADER_CG_GUILD								= 80,
	HEADER_CG_ANSWER_MAKE_GUILD					= 81,
    HEADER_CG_FISHING                           = 82,
    HEADER_CG_GIVE_ITEM                         = 83,
	//HEADER_BLANK84								= 84,
	//HEADER_BLANK85								= 85,
	//HEADER_BLANK86								= 86,
	//HEADER_BLANK87								= 87,
	//HEADER_BLANK88								= 88,
	//HEADER_BLANK89								= 89,
    HEADER_CG_EMPIRE                            = 90,
	//HEADER_BLANK91								= 91,
	//HEADER_BLANK92								= 92,
	//HEADER_BLANK93								= 93,
	//HEADER_BLANK94								= 94,
	//HEADER_BLANK95								= 95,
    HEADER_CG_REFINE                            = 96,
	//HEADER_BLANK97								= 97,
	//HEADER_BLANK98								= 98,
	//HEADER_BLANK99								= 99,

	HEADER_CG_MARK_LOGIN						= 100,
	HEADER_CG_MARK_CRCLIST						= 101,
	HEADER_CG_MARK_UPLOAD						= 102,
	HEADER_CG_MARK_IDXLIST						= 104,

	HEADER_CG_CRC_REPORT						= 103,

	HEADER_CG_HACK								= 105,
    HEADER_CG_CHANGE_NAME                       = 106,
    HEADER_CG_LOGIN2                            = 109,
	HEADER_CG_DUNGEON							= 110,
	HEADER_CG_LOGIN3							= 111,
	HEADER_CG_GUILD_SYMBOL_UPLOAD				= 112,
	HEADER_CG_GUILD_SYMBOL_CRC					= 113,
	HEADER_CG_SCRIPT_SELECT_ITEM				= 114,
	HEADER_CG_LOGIN4							= 115,
#ifdef ENABLE_IKASHOP_RENEWAL
	HEADER_CG_NEW_OFFLINESHOP					= 119,
#endif
#ifdef ENABLE_SORT_INVEN
	SORT_INVEN									= 140,
#endif
#ifdef NEW_PET_SYSTEM
	HEADER_CG_PetSetName						= 147,
#endif
#ifdef ENABLE_SKILLBOOK_COMB_SYSTEM
	HEADER_CG_SKILLBOOK_COMB = 151,
#endif
#ifdef ENABLE_SWITCHBOT
	HEADER_CG_SWITCHBOT							= 171,
#endif
	HEADER_CG_DRAGON_SOUL_REFINE				= 205,
	HEADER_CG_STATE_CHECKER						= 206,
#if defined(ENABLE_VOICE_CHAT)
	HEADER_CG_VOICE_CHAT = 207,
#endif
#if defined(__BL_67_ATTR__)
	HEADER_CG_67_ATTR 						= 208,
	HEADER_CG_CLOSE_67_ATTR 				= 209,
#endif
#ifdef ELEMENT_SPELL_WORLDARD
	HEADER_CG_ELEMENTS_SPELL 					= 213,
#endif
#if defined(__BL_MAILBOX__)
	HEADER_CG_MAILBOX_WRITE = 219,
	HEADER_CG_MAILBOX_WRITE_CONFIRM = 220,
	HEADER_CG_MAILBOX_PROCESS = 221,
#endif
#if defined(ENABLE_NPC_LOCATION_HELPER)
	HEADER_CG_NPC_LOCATION_HELPER = 223,
#endif
#if defined(__BL_MOVE_COSTUME_ATTR__)
	HEADER_CG_ITEM_COMBINATION					= 225,
	HEADER_CG_ITEM_COMBINATION_CANCEL			= 226,
#endif
#ifdef BOT_KILLER
	HEADER_CG_BOT_KILLER						= 227,
#endif
#if defined(__BL_TRANSMUTATION__)
	HEADER_CG_CHANGE_LOOK = 228,
#endif
#ifdef ENABLE_CUBE_RENEWAL_WORLDARD
	HEADER_CG_CUBE_RENEWAL					= 229,
#endif
#ifdef ENABLE_SHOP_SEARCH_SYSTEM
	HEADER_CG_SHOP_SEARCH = 230,
#endif
#if defined(__BL_OFFICIAL_LOOT_FILTER__)
	HEADER_CG_LOOT_FILTER = 231,
#endif
#if defined(__MINI_GAME_CATCH_KING__)
	HEADER_CG_MINI_GAME_CATCH_KING = 232,
#endif
#ifdef ENABLE_MINI_GAME_YUTNORI
	HEADER_CG_MINI_GAME_YUT_NORI = 233,
#endif
#ifdef ENABLE_TAROT
	HEADER_CG_MINIGAME_BNW			= 234,
#endif
#if defined(ENABLE_PARTY_MATCH)
	HEADER_CG_PARTY_MATCH						= 235,
#endif
#ifdef ENABLE_AURA_SYSTEM
	HEADER_CG_AURA								= 236,
#endif
#if defined(WJ_COMBAT_ZONE)
	HEADER_CG_COMBAT_ZONE_REQUEST_ACTION		= 237,
#endif
#ifdef ENABLE_CAPTCHA
	HEADER_CG_CAPTCHA							= 237,
#endif
#ifdef ENABLE_CSHIELD
	HEADER_CG_CSHIELD							= 238,
	HEADER_CG_CSHIELD_DUMMY1					= 239, // header 239 is reserved for cshield.dll and can not be changed
	HEADER_CG_CSHIELD_DUMMY2					= 240, // header 240 is reserved for cshield.dll and can not be changed
#endif

	HEADER_CG_KEY_AGREEMENT						= 0xfb, // _IMPROVED_PACKET_ENCRYPTION_
	HEADER_CG_TIME_SYNC							= 0xfc,
	HEADER_CG_CLIENT_VERSION2					= 0xf1,
	HEADER_CG_PONG								= 0xfe,
    HEADER_CG_HANDSHAKE                         = 0xff,

	/////////////////////////////////////////////////
	// From Server
	HEADER_GC_CHARACTER_ADD						= 1,
	HEADER_GC_CHARACTER_DEL						= 2,
	HEADER_GC_CHARACTER_MOVE					= 3,
	HEADER_GC_CHAT								= 4,
	HEADER_GC_SYNC_POSITION 					= 5,
	HEADER_GC_LOGIN_SUCCESS3					= 6,
	HEADER_GC_LOGIN_FAILURE						= 7,
	HEADER_GC_PLAYER_CREATE_SUCCESS				= 8,
	HEADER_GC_PLAYER_CREATE_FAILURE				= 9,
	HEADER_GC_PLAYER_DELETE_SUCCESS				= 10,
	HEADER_GC_PLAYER_DELETE_WRONG_SOCIAL_ID		= 11,
	// 12
	HEADER_GC_STUN								= 13,
	HEADER_GC_DEAD								= 14,

	HEADER_GC_MAIN_CHARACTER					= 15,
	HEADER_GC_PLAYER_POINTS						= 16,
	HEADER_GC_PLAYER_POINT_CHANGE				= 17,
	HEADER_GC_CHANGE_SPEED						= 18,
	HEADER_GC_CHARACTER_UPDATE                  = 19,
	HEADER_GC_ITEM_SET							= 20,
	HEADER_GC_ITEM_SET2							= 21,
	HEADER_GC_ITEM_USE							= 22,
	HEADER_GC_ITEM_DROP							= 23,
	HEADER_GC_ITEM_UPDATE						= 25,
	HEADER_GC_ITEM_GROUND_ADD					= 26,
	HEADER_GC_ITEM_GROUND_DEL					= 27,
    HEADER_GC_QUICKSLOT_ADD                     = 28,
    HEADER_GC_QUICKSLOT_DEL                     = 29,
    HEADER_GC_QUICKSLOT_SWAP                    = 30,
	HEADER_GC_ITEM_OWNERSHIP					= 31,
	HEADER_GC_LOGIN_SUCCESS4					= 32,
	HEADER_GC_ITEM_UNBIND_TIME					= 33,
	HEADER_GC_WHISPER							= 34,
	HEADER_GC_ALERT								= 35,

	HEADER_GC_MOTION							= 36,

	HEADER_GC_SHOP							    = 38,
	HEADER_GC_SHOP_SIGN							= 39,

	HEADER_GC_DUEL_START						= 40,
	HEADER_GC_PVP								= 41,
	HEADER_GC_EXCHANGE							= 42,
    HEADER_GC_CHARACTER_POSITION                = 43,

	HEADER_GC_PING								= 44,

	HEADER_GC_SCRIPT							= 45,
    HEADER_GC_QUEST_CONFIRM                     = 46,
#ifdef ENABLE_OFFLINESHOP_SYSTEM
	HEADER_GC_OFFLINE_SHOP						= 47,
	HEADER_GC_OFFLINE_SHOP_SIGN					= 48,
#endif

#ifdef ENABLE_EVENT_MANAGER
	HEADER_GC_EVENT_MANAGER						= 50,
#endif

	HEADER_GC_MOUNT								= 61,
	HEADER_GC_OWNERSHIP                         = 62,
    HEADER_GC_TARGET                            = 63,
	HEADER_GC_WARP								= 65,
	HEADER_GC_ADD_FLY_TARGETING                 = 69,

	HEADER_GC_CREATE_FLY						= 70,
	HEADER_GC_FLY_TARGETING						= 71,
	HEADER_GC_SKILL_LEVEL						= 72,
	HEADER_GC_SKILL_COOLTIME_END				= 73,
    HEADER_GC_MESSENGER                         = 74,
	HEADER_GC_GUILD								= 75,
	HEADER_GC_SKILL_LEVEL_NEW					= 76,

    HEADER_GC_PARTY_INVITE                      = 77,
    HEADER_GC_PARTY_ADD                         = 78,
    HEADER_GC_PARTY_UPDATE                      = 79,
    HEADER_GC_PARTY_REMOVE                      = 80,

    HEADER_GC_QUEST_INFO                        = 81,
    HEADER_GC_REQUEST_MAKE_GUILD                = 82,
	HEADER_GC_PARTY_PARAMETER                   = 83,

    HEADER_GC_SAFEBOX_MONEY_CHANGE              = 84,
    HEADER_GC_SAFEBOX_SET                       = 85,
    HEADER_GC_SAFEBOX_DEL                       = 86,
    HEADER_GC_SAFEBOX_WRONG_PASSWORD            = 87,
    HEADER_GC_SAFEBOX_SIZE                      = 88,

    HEADER_GC_FISHING                           = 89,

    HEADER_GC_EMPIRE                            = 90,

    HEADER_GC_PARTY_LINK                        = 91,
    HEADER_GC_PARTY_UNLINK                      = 92,
#ifdef ENABLE_ITEMSHOP
	HEADER_GC_ITEMSHOP = 93,
#endif
    HEADER_GC_REFINE_INFORMATION                = 95,

	HEADER_GC_OBSERVER_ADD						= 96,
	HEADER_GC_OBSERVER_REMOVE					= 97,
	HEADER_GC_OBSERVER_MOVE						= 98,
	HEADER_GC_VIEW_EQUIP                        = 99,

	HEADER_GC_MARK_BLOCK						= 100,
	HEADER_GC_MARK_DIFF_DATA                    = 101,
	HEADER_GC_MARK_IDXLIST						= 102,

    HEADER_GC_TIME                              = 106,
    HEADER_GC_CHANGE_NAME                       = 107,

	HEADER_GC_DUNGEON							= 110,
	HEADER_GC_WALK_MODE							= 111,
	HEADER_GC_CHANGE_SKILL_GROUP				= 112,

	// SUPPORT_BGM
	HEADER_GC_MAIN_CHARACTER2_EMPIRE			= 113,
	// END_OF_SUPPORT_BGM

    HEADER_GC_SEPCIAL_EFFECT                    = 114,
	HEADER_GC_NPC_POSITION						= 115,

    HEADER_GC_LOGIN_KEY                         = 118,
    HEADER_GC_REFINE_INFORMATION_NEW            = 119,
    HEADER_GC_CHARACTER_ADD2                    = 120,
    HEADER_GC_CHANNEL                           = 121,

    HEADER_GC_MALL_OPEN                         = 122,
	HEADER_GC_TARGET_UPDATE                     = 123,
	HEADER_GC_TARGET_DELETE                     = 124,
	HEADER_GC_TARGET_CREATE_NEW                 = 125,

	HEADER_GC_AFFECT_ADD                        = 126,
	HEADER_GC_AFFECT_REMOVE                     = 127,

    HEADER_GC_MALL_SET                          = 128,
    HEADER_GC_MALL_DEL                          = 129,
	HEADER_GC_LAND_LIST                         = 130,
	HEADER_GC_LOVER_INFO						= 131,
	HEADER_GC_LOVE_POINT_UPDATE					= 132,
	HEADER_GC_GUILD_SYMBOL_DATA					= 133,
    HEADER_GC_DIG_MOTION                        = 134,

	HEADER_GC_DAMAGE_INFO						= 135,
	HEADER_GC_CHAR_ADDITIONAL_INFO				= 136,

	// SUPPORT_BGM
	HEADER_GC_MAIN_CHARACTER3_BGM				= 137,
	HEADER_GC_MAIN_CHARACTER4_BGM_VOL			= 138,
	// END_OF_SUPPORT_BGM
    HEADER_GC_AUTH_SUCCESS                      = 150,
    HEADER_GC_PANAMA_PACK						= 151,

	//HYBRID CRYPT
	HEADER_GC_HYBRIDCRYPT_KEYS					= 152,
	HEADER_GC_HYBRIDCRYPT_SDB					= 153, // SDB means Supplmentary Data Blocks
	//HYBRID CRYPT
#ifdef ENABLE_FISHING_TIMER
	HEADER_GC_FISHING_TIMER						= 155,
#endif
#ifdef ENABLE_SHOP_SEARCH_SYSTEM
	HEADER_GC_SHOPSEARCH_SET = 156,
#endif
#ifdef ENABLE_GEM_SYSTEM
	HEADER_GC_GEM_SHOP_OPEN						= 157,
	HEADER_GC_GEM_SHOP_REFRESH					= 158,
#endif
#ifdef ENABLE_GUILD_TOKEN_AUTH
	HEADER_GC_GUILD_TOKEN						= 161,
#endif
#ifdef ENABLE_SWITCHBOT
	HEADER_GC_SWITCHBOT							= 171,
#endif
#if defined(ENABLE_VOICE_CHAT)
	HEADER_GC_VOICE_CHAT = 182,
#endif

	HEADER_GC_SPECIFIC_EFFECT					= 208,
	HEADER_GC_DRAGON_SOUL_REFINE				= 209,
	HEADER_GC_RESPOND_CHANNELSTATUS				= 210,
#ifdef ENABLE_DS_SET
	HEADER_GC_DS_TABLE							= 212,
#endif
#ifdef ELEMENT_SPELL_WORLDARD
	HEADER_GC_ELEMENTS_SPELL 					= 213,
#endif
#ifdef ENABLE_IKASHOP_RENEWAL
	HEADER_GC_NEW_OFFLINESHOP					= 214,
#endif
#if defined(__BL_KILL_BAR__)
	HEADER_GC_KILLBAR 							= 218,
#endif
#if defined(__BL_67_ATTR__)
	HEADER_GC_OPEN_67_ATTR 						= 219,
#endif
#if defined(__BL_MAILBOX__)
	HEADER_GC_MAILBOX_PROCESS = 220,
	HEADER_GC_MAILBOX = 221,
	HEADER_GC_MAILBOX_ADD_DATA = 222,
	HEADER_GC_MAILBOX_ALL = 223,
	HEADER_GC_MAILBOX_UNREAD = 224,
#endif
#ifdef ENABLE_CUBE_RENEWAL_WORLDARD
	HEADER_GC_CUBE_RENEWAL						= 225,
#endif
#if defined(__BL_TRANSMUTATION__)
	HEADER_GC_CHANGE_LOOK_SET = 228,
	HEADER_GC_CHANGE_LOOK_DEL = 229,
	HEADER_GC_CHANGE_LOOK_FREE_SET = 230,
	HEADER_GC_CHANGE_LOOK_FREE_DEL = 231,
#endif
#ifdef ENABLE_12ZI
	HEADER_GC_SEPCIAL_ZODIAC_EFFECT = 232,
#endif
#if defined(ENABLE_NPC_LOCATION_HELPER)
	HEADER_GC_NPC_LOCATION_HELPER = 233,
#endif
#if defined(__MINI_GAME_CATCH_KING__)
	HEADER_GC_MINI_GAME_CATCH_KING = 234,
#endif
#ifdef ENABLE_MINI_GAME_YUTNORI
	HEADER_GC_MINI_GAME_YUT_NORI 				= 235,
#endif
#ifdef ENABLE_TAROT
	HEADER_GC_MINIGAME_BNW			= 236,
#endif
#if defined(ENABLE_PARTY_MATCH)
	HEADER_GC_PARTY_MATCH = 237,
#endif
#ifdef ENABLE_AURA_SYSTEM
	HEADER_GC_AURA								= 238,
#endif
#if defined(WJ_COMBAT_ZONE)
	HEADER_GC_RANKING_COMBAT_ZONE				= 239, 
	HEADER_GC_SEND_COMBAT_ZONE					= 240,
#endif
//#ifdef ENABLE_CAPTCHA
//	HEADER_GC_CAPTCHA							= 237,
//#endif
//#ifdef ENABLE_CSHIELD
//	HEADER_GC_CSHIELD							= 238,
//	HEADER_GC_CSHIELD_STOP_PLAYER				= 239,
//#endif
//#if defined(__BL_OFFICIAL_LOOT_FILTER__)
//	HEADER_GC_LOOT_FILTER = 240,
//#endif
#ifdef BOT_KILLER
	HEADER_GC_BOT_KILLER						= 241,
#endif
#ifdef ENABLE_CONQUEROR_LEVEL
	HEADER_GC_SUNGMA_ATTR_UPDATE = 139,
#endif
	HEADER_GC_KEY_AGREEMENT_COMPLETED			= 0xfa, // _IMPROVED_PACKET_ENCRYPTION_

	HEADER_GC_KEY_AGREEMENT						= 0xfb, // _IMPROVED_PACKET_ENCRYPTION_
	HEADER_GC_HANDSHAKE_OK						= 0xfc, // 252
	HEADER_GC_PHASE								= 0xfd,	// 253
    HEADER_GC_BINDUDP                           = 0xfe, // 254
    HEADER_GC_HANDSHAKE                         = 0xff, // 255
};

enum
{
	ID_MAX_NUM = 30,
	PASS_MAX_NUM = 16,
	CHAT_MAX_NUM = 128,
	PATH_NODE_MAX_NUM = 64,
	SHOP_SIGN_MAX_LEN = 45,

	PLAYER_PER_ACCOUNT3 = 3,
#ifndef ENABLE_PLAYER_PER_ACCOUNT5
	PLAYER_PER_ACCOUNT4 = 4,
#else
	PLAYER_PER_ACCOUNT4 = 5,
	PLAYER_PER_ACCOUNT5 = 5,
#endif

	PLAYER_ITEM_SLOT_MAX_NUM = 20,

	QUICKSLOT_MAX_LINE = 4,
	QUICKSLOT_MAX_COUNT_PER_LINE = 8,
	QUICKSLOT_MAX_COUNT = QUICKSLOT_MAX_LINE * QUICKSLOT_MAX_COUNT_PER_LINE,

	QUICKSLOT_MAX_NUM = 36,

	SHOP_HOST_ITEM_MAX_NUM = 40,
#ifdef ENABLE_OFFLINESHOP_SYSTEM
	OFFLINE_SHOP_HOST_ITEM_MAX_NUM = 80,
#endif

	METIN_SOCKET_COUNT = 6,

	PARTY_AFFECT_SLOT_MAX_NUM = 7,

	GUILD_GRADE_NAME_MAX_LEN = 8,
	GUILD_NAME_MAX_LEN = 12,
	GUILD_GRADE_COUNT = 15,
	GULID_COMMENT_MAX_LEN = 50,

	MARK_CRC_NUM = 8*8,
	MARK_DATA_SIZE = 16*12,
	SYMBOL_DATA_SIZE = 128*256,
	QUEST_INPUT_STRING_MAX_NUM = 64,

	PRIVATE_CODE_LENGTH = 8,

	REFINE_MATERIAL_MAX_NUM = 5,

	WEAR_MAX_NUM = CItemData::WEAR_MAX_NUM,

	SHOP_TAB_NAME_MAX = 32,
	SHOP_TAB_COUNT_MAX = 3,
};

#pragma pack(push)
#pragma pack(1)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Mark
#ifdef ENABLE_OFFLINESHOP_SYSTEM
typedef struct packet_offline_shop_item
{
	DWORD		vnum;
	long long	price;
	int			count;
	BYTE		display_pos;
	LONG		alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
	BYTE		status;
	char		szBuyerName[CHARACTER_NAME_MAX_LEN+1];
#if defined(__BL_TRANSMUTATION__)
	DWORD		dwTransmutationVnum;
#endif
#ifdef ELEMENT_SPELL_WORLDARD
	BYTE 	    grade_element;
	DWORD 	 	attack_element[MAX_ELEMENTS_SPELL];
 	BYTE 		element_type_bonus;
 	short 		elements_value_bonus[MAX_ELEMENTS_SPELL];
#endif
#ifdef ENABLE_NEW_SOCKET_VALUE
	LONG alMinMaxValue[ITEM_MIN_MAX_VALUE_MAX_NUM];
#endif
	DWORD		owner_id;
} TOfflineShopItemData;
typedef struct ShopLog {
	char	name[CHARACTER_NAME_MAX_LEN + 1];
	char	date[25];
	DWORD	itemVnum;
	BYTE	itemCount;
	long long	price;
}TShopLog;
typedef struct SShopDecoration {
	char	sign[SHOP_SIGN_MAX_LEN + 1];
	DWORD	owner_id;
	BYTE	vnum;
	BYTE	type;
}TShopDecoration;
typedef struct SPacketGCShopSign
{
	BYTE        bHeader;
	DWORD       dwVID;
	char        szSign[SHOP_SIGN_MAX_LEN + 1];
} TPacketGCShopSign;
typedef struct SOfflineShopItemTable
{
	DWORD		vnum;
	int			count;
	TItemPos	pos;
	long long		price;
	BYTE		display_pos;
} TOfflineShopItemTable;
typedef struct SOfflineShopAddItem
{
	TItemPos	bDisplayPos;
	BYTE		bPos;
	long long		lPrice;
} TOfflineShopAddItem;
typedef struct SPacketCGMyOfflineShop
{
	BYTE		bHeader;
	char		szSign[SHOP_SIGN_MAX_LEN + 1];
	BYTE		bCount;
	DWORD		shopVnum;
	BYTE		shopTitle;
} TPacketCGMyOfflineShop;
typedef struct packet_offline_shop_start
{
	DWORD	owner_vid;
	TOfflineShopItemData items[OFFLINE_SHOP_HOST_ITEM_MAX_NUM];
	DWORD	m_dwDisplayedCount;
	char			title[SHOP_SIGN_MAX_LEN + 1];
	long long	price;
	DWORD	m_dwRealWatcherCount;
	ShopLog			log[OFFLINE_SHOP_HOST_ITEM_MAX_NUM];
	long long		flag;
	int					time;
	DWORD			type;
	bool			IsOwner;
} TPacketGCOfflineShopStart;
typedef struct packet_offline_shop_update_item
{
	BYTE						pos;
	TOfflineShopItemData item;
	DWORD			m_dwDisplayedCount;
	DWORD			m_dwRealWatcherCount;
	char			title[SHOP_SIGN_MAX_LEN + 1];
	long long	price;
	ShopLog			log;
	long long		flag;
	int					time;
	DWORD			type;
} TPacketGCOfflineShopUpdateItem;
#endif

typedef struct command_mark_login
{
    BYTE    header;
    DWORD   handle;
    DWORD   random_key;
} TPacketCGMarkLogin;

typedef struct command_mark_upload
{
    BYTE    header;
    DWORD   gid;
    BYTE    image[16*12*4];
#ifdef ENABLE_GUILD_TOKEN_AUTH
    uint64_t	token;
#endif
} TPacketCGMarkUpload;

typedef struct command_mark_idxlist
{
    BYTE    header;
} TPacketCGMarkIDXList;

typedef struct command_mark_crclist
{
    BYTE    header;
    BYTE    imgIdx;
    DWORD   crclist[80];
} TPacketCGMarkCRCList;

typedef struct packet_mark_idxlist
{
    BYTE    header;
	DWORD	bufSize;
    WORD    count;
} TPacketGCMarkIDXList;

typedef struct packet_mark_block
{
    BYTE    header;
    DWORD   bufSize;
	BYTE	imgIdx;
    DWORD   count;
} TPacketGCMarkBlock;

typedef struct command_symbol_upload
{
	BYTE	header;
	WORD	size;
	DWORD	handle;
#ifdef ENABLE_GUILD_TOKEN_AUTH
	uint64_t	token;
#endif
} TPacketCGSymbolUpload;

typedef struct command_symbol_crc
{
	BYTE	header;
	DWORD	dwGuildID;
	DWORD	dwCRC;
	DWORD	dwSize;
} TPacketCGSymbolCRC;

typedef struct packet_symbol_data
{
    BYTE header;
    WORD size;
    DWORD guild_id;
} TPacketGCGuildSymbolData;

//
//
//
typedef struct packet_observer_add
{
	BYTE	header;
	DWORD	vid;
	WORD	x;
	WORD	y;
} TPacketGCObserverAdd;

typedef struct packet_observer_move
{
	BYTE	header;
	DWORD	vid;
	WORD	x;
	WORD	y;
} TPacketGCObserverMove;

typedef struct packet_observer_remove
{
	BYTE	header;
	DWORD	vid;
} TPacketGCObserverRemove;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// To Server

typedef struct command_checkin
{
	BYTE header;
	char name[ID_MAX_NUM+1];
	char pwd[PASS_MAX_NUM+1];
} TPacketCGCheckin;

typedef struct command_login
{
    BYTE header;
    char name[ID_MAX_NUM + 1];
    char pwd[PASS_MAX_NUM + 1];
} TPacketCGLogin;

typedef struct command_login2
{
	BYTE	header;
	char	name[ID_MAX_NUM + 1];
	DWORD	login_key;
    DWORD	adwClientKey[4];
} TPacketCGLogin2;

typedef struct command_login3
{
    BYTE	header;
    char	name[ID_MAX_NUM + 1];
    char	pwd[PASS_MAX_NUM + 1];
    DWORD	adwClientKey[4];
#if defined(ENABLE_CSHIELD) || defined(ENABLE_CSHIELD_BYPASS)
	char	cshieldKey[64 + 1];
	char	cshieldHwid[128 + 1];
#endif
} TPacketCGLogin3;

#ifdef NEW_PET_SYSTEM
typedef struct packet_RequestPetName
{
	BYTE byHeader;
	char petname[13];
} TPacketCGRequestPetName;
#endif

typedef struct command_direct_enter
{
    BYTE        bHeader;
    char        login[ID_MAX_NUM + 1];
    char        passwd[PASS_MAX_NUM + 1];
    BYTE        index;
} TPacketCGDirectEnter;

typedef struct command_player_select
{
	BYTE	header;
	BYTE	player_index;
} TPacketCGSelectCharacter;

typedef struct command_attack
{
	BYTE	header;
	BYTE	bType;
	DWORD	dwVictimVID;
	BYTE	bCRCMagicCubeProcPiece;
	BYTE	bCRCMagicCubeFilePiece;
} TPacketCGAttack;

typedef struct command_chat
{
	BYTE	header;
	WORD	length;
	BYTE	type;
} TPacketCGChat;

typedef struct command_whisper
{
    BYTE        bHeader;
    WORD        wSize;
    char        szNameTo[CHARACTER_NAME_MAX_LEN + 1];
} TPacketCGWhisper;

enum EBattleMode
{
	BATTLEMODE_ATTACK = 0,
	BATTLEMODE_DEFENSE = 1,
};

typedef struct command_EnterFrontGame
{
	BYTE header;
} TPacketCGEnterFrontGame;

typedef struct command_item_use
{
	BYTE header;
	TItemPos pos;
} TPacketCGItemUse;

#ifdef ENABLE_CHEST_OPEN_RENEWAL
typedef struct OpenChestPacket
{
	BYTE header;
	TItemPos pos;
	WORD wOpenCount;
} TPacketCGOpenChest;
#endif

typedef struct command_item_use_to_item
{
	BYTE header;
	TItemPos source_pos;
	TItemPos target_pos;
} TPacketCGItemUseToItem;

typedef struct command_item_drop
{
	BYTE  header;
	TItemPos pos;
	DWORD elk;
#if defined(ENABLE_CHEQUE_SYSTEM) && !defined(DISABLE_CHEQUE_DROP)
	DWORD cheque;
#endif
} TPacketCGItemDrop;

typedef struct command_item_drop2
{
    BYTE        header;
    TItemPos	pos;
    DWORD       gold;
#if defined(ENABLE_CHEQUE_SYSTEM) && !defined(DISABLE_CHEQUE_DROP)
	DWORD		cheque;
#endif
    BYTE        count;
} TPacketCGItemDrop2;

typedef struct command_item_move
{
	BYTE header;
	TItemPos pos;
	TItemPos change_pos;
	BYTE num;
} TPacketCGItemMove;

#if defined(ENABLE_PARTY_MATCH)
typedef struct partymatchCG
{
	BYTE		Header;
	BYTE		SubHeader;
	WORD		dummy_padding;
	DWORD		targetPID;
	int			mapIndex;
	char		szDesc[52]; // NEW: 52 bytes alignment
} TPacketCGPartyMatch;

typedef struct partymatchGC
{
	BYTE		Header;
	BYTE		SubHeader;
	WORD		dummy_padding;
	DWORD		targetPID;
	int			mapIndex;
	char		szName[CHARACTER_NAME_MAX_LEN + 1];
	BYTE		current_count;
	BYTE		max_count;
	BYTE		msg;
	BYTE		job;
	BYTE		empire;
	BYTE		is_my_party; 
	BYTE		is_online;
	BYTE		channel; // NEW: Channel Category
	BYTE		dummy_padding2[1]; // Adjusted padding for alignment
	int			level;
	int			conqueror_level;
	char		szDesc[52]; 
} TPacketGCPartyMatch;
#endif

#ifdef ENABLE_SORT_INVEN
typedef struct sort_inven
{
	BYTE	header;
	BYTE	window_type; // Added
	BYTE 	option;
} TPacketCGSortInven;
#endif

typedef struct command_item_pickup
{
	BYTE header;
	DWORD vid;
} TPacketCGItemPickUp;

typedef struct command_quickslot_add
{
    BYTE        header;
    BYTE        pos;
	TQuickSlot	slot;
}TPacketCGQuickSlotAdd;

#ifdef ENABLE_GEM_SYSTEM

typedef struct command_gem_shop
{
	BYTE header;
	BYTE subheader;
	BYTE pos;
} TPacketCGGemShop;

typedef struct command_gem_shop_open
{
	BYTE	header;
	int	nextRefreshTime;
	TGemShopItem shopItems[9];
} TPacketGCGemShopOpen;

typedef struct command_gem_shop_refresh
{
	BYTE	header;
	int	nextRefreshTime;
	TGemShopItem shopItems[9];
} TPacketGCGemShopRefresh;


enum
{
	GEM_SHOP_SUBHEADER_CG_BUY,
	GEM_SHOP_SUBHEADER_CG_ADD,
	GEM_SHOP_SUBHEADER_CG_REFRESH,
};
#endif


typedef struct command_quickslot_del
{
    BYTE        header;
    BYTE        pos;
}TPacketCGQuickSlotDel;

typedef struct command_quickslot_swap
{
    BYTE        header;
    BYTE        pos;
    BYTE        change_pos;
}TPacketCGQuickSlotSwap;

typedef struct command_on_click
{
	BYTE		header;
	DWORD		vid;
} TPacketCGOnClick;

enum
{
    SHOP_SUBHEADER_CG_END,
	SHOP_SUBHEADER_CG_BUY,
	SHOP_SUBHEADER_CG_SELL,
#ifdef ENABLE_OFFLINESHOP_SYSTEM
	SHOP_SUBHEADER_CG_ADD_ITEM,
	SHOP_SUBHEADER_CG_REMOVE_ITEM,
	SHOP_SUBHEADER_CG_DESTROY_OFFLINE_SHOP,
	SHOP_SUBHEADER_CG_CHECK,
	SHOP_SUBHEADER_CG_BUTTON,
	SHOP_SUBHEADER_CG_CHANGE_TITLE,
	SHOP_SUBHEADER_CG_TAKE_MONEY,
	SHOP_SUBHEADER_CG_LOG_REMOVE,
	SHOP_SUBHEADER_CG_CHANGE_DECORATION,
	SHOP_SUBHEADER_CG_OPEN_SLOT,
	SHOP_SUBHEADER_CG_GET_ITEM,
	SHOP_SUBHEADER_CG_ADD_TIME,
	SHOP_SUBHEADER_CG_OPEN_WITH_VID,
#endif
	SHOP_SUBHEADER_CG_SELL2,
};

typedef struct command_shop
{
	BYTE        header;
	BYTE		subheader;
#ifdef ENABLE_OFFLINESHOP_SYSTEM
	char		title[50 + 1];
	DWORD		vid;
	BYTE		pos;
#endif
} TPacketCGShop;

#if defined(__BL_TRANSMUTATION__)
enum class ECG_TRANSMUTATION_SHEADER: BYTE
{
	ITEM_CHECK_IN,
	ITEM_CHECK_OUT,
	FREE_ITEM_CHECK_IN,
	FREE_ITEM_CHECK_OUT,
	ACCEPT,
	CANCEL
};

typedef struct packet_set_transmutation
{
	BYTE		header;
	WORD		wCell;
	BYTE		bSlotType;
} TPacketGCTransmutationItemSet;

typedef struct packet_transmutation_del
{
	BYTE        header;
	WORD        wCell;
	BYTE        bSlotType;
} TPacketGCTransmutationDel;

typedef struct command_transmutation
{
	command_transmutation(const BYTE _Sub) :
		header(HEADER_CG_CHANGE_LOOK),
		subheader(_Sub),
		slot_type(255)
	{}
	BYTE		header;
	BYTE		subheader;
	BYTE		slot_type;
	TItemPos	pos;
} TPacketCGTransmutation;
#endif

enum
{
	EXCHANGE_SUBHEADER_CG_START,			// arg1 == vid of target character
	EXCHANGE_SUBHEADER_CG_ITEM_ADD,			// arg1 == position of item
	EXCHANGE_SUBHEADER_CG_ITEM_DEL,			// arg1 == position of item
	EXCHANGE_SUBHEADER_CG_ELK_ADD,			// arg1 == amount of elk
	EXCHANGE_SUBHEADER_CG_ACCEPT,			// arg1 == not used
	EXCHANGE_SUBHEADER_CG_CANCEL,			// arg1 == not used
#ifdef ENABLE_CHEQUE_SYSTEM
	EXCHANGE_SUBHEADER_CG_CHEQUE_ADD,
#endif
};

typedef struct command_exchange
{
	BYTE		header;
	BYTE		subheader;
	DWORD		arg1;
	BYTE		arg2;
	TItemPos	Pos;
} TPacketCGExchange;

typedef struct command_position
{
    BYTE        header;
    BYTE        position;
} TPacketCGPosition;

typedef struct command_script_answer
{
    BYTE        header;
	BYTE		answer;
} TPacketCGScriptAnswer;

typedef struct command_script_button
{
    BYTE        header;
	unsigned int			idx;
} TPacketCGScriptButton;

typedef struct command_target
{
    BYTE        header;
    DWORD       dwVID;
} TPacketCGTarget;

typedef struct command_move
{
	BYTE		bHeader;
	BYTE		bFunc;
	BYTE		bArg;
	BYTE		bRot;
	LONG		lX;
	LONG		lY;
	DWORD		dwTime;
} TPacketCGMove;

typedef struct command_sync_position_element
{
    DWORD       dwVID;
    LONG        lX;
    LONG        lY;
} TPacketCGSyncPositionElement;

typedef struct command_sync_position
{
    BYTE        bHeader;
	WORD		wSize;
} TPacketCGSyncPosition;

typedef struct command_fly_targeting
{
	BYTE		bHeader;
	DWORD		dwTargetVID;
	LONG		lX;
	LONG		lY;
} TPacketCGFlyTargeting;

typedef struct packet_fly_targeting
{
    BYTE        bHeader;
	DWORD		dwShooterVID;
	DWORD		dwTargetVID;
	LONG		lX;
	LONG		lY;
} TPacketGCFlyTargeting;

typedef struct packet_shoot
{
    BYTE		bHeader;
    BYTE		bType;
} TPacketCGShoot;

typedef struct command_warp
{
	BYTE			bHeader;
} TPacketCGWarp;

enum
{
#ifdef ENABLE_MESSENGER_BLOCK
	MESSENGER_SUBHEADER_GC_BLOCK_LIST,
	MESSENGER_SUBHEADER_GC_BLOCK_LOGIN,
	MESSENGER_SUBHEADER_GC_BLOCK_LOGOUT,
	MESSENGER_SUBHEADER_GC_BLOCK_INVITE,//not used
#endif	
	MESSENGER_SUBHEADER_GC_LIST,
	MESSENGER_SUBHEADER_GC_LOGIN,
	MESSENGER_SUBHEADER_GC_LOGOUT,
	MESSENGER_SUBHEADER_GC_INVITE,
#if defined(ENABLE_MESSENGER_RENEWAL)
	MESSENGER_SUBHEADER_GC_CONNECTION_STATE,
	MESSENGER_SUBHEADER_GC_MY_STATUS_MESSAGE,
	MESSENGER_SUBHEADER_GC_STATUS_MESSAGE,
#endif
};

typedef struct packet_messenger
{
    BYTE header;
    WORD size;
    BYTE subheader;
} TPacketGCMessenger;

typedef struct packet_messenger_list_offline
{
    BYTE connected; // always 0
	BYTE length;
} TPacketGCMessengerListOffline;

#ifdef ENABLE_MESSENGER_BLOCK
typedef struct packet_messenger_block_list_offline
{
	BYTE connected; // always 0
	BYTE length;
} TPacketGCMessengerBlockListOffline;

typedef struct packet_messenger_block_list_online
{
	BYTE connected; // always 1
	BYTE length;
} TPacketGCMessengerBlockListOnline;
#endif

enum
{
	MESSENGER_CONNECTED_STATE_OFFLINE,
	MESSENGER_CONNECTED_STATE_ONLINE,
};

typedef struct packet_messenger_list_online
{
    BYTE connected;
	BYTE length;
	//BYTE length_char_name;
} TPacketGCMessengerListOnline;

#if defined(ENABLE_MESSENGER_RENEWAL)
enum EMessengerConnectionState
{
	MESSENGER_CONNECTION_STATE_CONNECT = 0,
	MESSENGER_CONNECTION_STATE_LEFT_SEAT = 1,
	MESSENGER_CONNECTION_STATE_AUTO_HUNT = 2,
	MESSENGER_CONNECTION_STATE_SHOP_OPEN = 3,
	MESSENGER_CONNECTION_STATE_DISCONNECT = 4,
};

enum
{
	MESSENGER_STATUS_MESSAGE_MAX_LEN = 50,
};

typedef struct packet_messenger_list
{
	packet_messenger_list()
		: bConnected(MESSENGER_CONNECTED_STATE_OFFLINE)
		, szName{}
		, bLevel(0)
		, bIsConqueror(0)
		, bChannel(0)
		, lMapIndex(0)
		, bConnectionState(MESSENGER_CONNECTION_STATE_CONNECT)
		, szStatusMessage{}
	{}

	BYTE bConnected;
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	BYTE bLevel;
	BYTE bIsConqueror;
	BYTE bChannel;
	LONG lMapIndex;
	BYTE bConnectionState;
	char szStatusMessage[MESSENGER_STATUS_MESSAGE_MAX_LEN + 1];
} TPacketGCMessengerList;

typedef struct command_messenger_set_connection_state
{
	BYTE bConnectionState;
} TPacketCGMessengerSetConnectionState;

typedef struct packet_messenger_my_status_message
{
	char szStatusMessage[MESSENGER_STATUS_MESSAGE_MAX_LEN + 1];
} TPacketGCMessengerMyStatusMessage;

typedef struct packet_messenger_status_message
{
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	char szStatusMessage[MESSENGER_STATUS_MESSAGE_MAX_LEN + 1];
} TPacketGCMessengerStatusMessage;

typedef struct command_messenger_set_status_message
{
	char szStatusMessage[MESSENGER_STATUS_MESSAGE_MAX_LEN + 1];
} TPacketCGMessengerSetStatusMessage;
#endif

typedef struct packet_messenger_login
{
	//BYTE length_login;
	//BYTE length_char_name;
	BYTE length;
} TPacketGCMessengerLogin;

typedef struct packet_messenger_logout
{
	BYTE length;
} TPacketGCMessengerLogout;

enum
{
#ifdef ENABLE_MESSENGER_BLOCK
	MESSENGER_SUBHEADER_CG_ADD_BLOCK_BY_VID,
    MESSENGER_SUBHEADER_CG_ADD_BLOCK_BY_NAME,
    MESSENGER_SUBHEADER_CG_REMOVE_BLOCK,
#endif
    MESSENGER_SUBHEADER_CG_ADD_BY_VID,
    MESSENGER_SUBHEADER_CG_ADD_BY_NAME,
    MESSENGER_SUBHEADER_CG_REMOVE,
	MESSENGER_SUBHEADER_CG_INVITE_ANSWER,
#if defined(ENABLE_MESSENGER_RENEWAL)
	MESSENGER_SUBHEADER_CG_SET_CONNECTION_STATE,
	MESSENGER_SUBHEADER_CG_SET_STATUS_MESSAGE,
	MESSENGER_SUBHEADER_CG_DELETE_STATUS_MESSAGE,
#endif
};

typedef struct command_messenger
{
    BYTE header;
    BYTE subheader;
} TPacketCGMessenger;

typedef struct command_messenger_remove
{
	BYTE length;
} TPacketCGMessengerRemove;

enum
{
	SAFEBOX_MONEY_STATE_SAVE,
	SAFEBOX_MONEY_STATE_WITHDRAW,
};

typedef struct command_safebox_money
{
    BYTE        bHeader;
    BYTE        bState;
    DWORD       dwMoney;
} TPacketCGSafeboxMoney;

typedef struct command_safebox_checkout
{
    BYTE        bHeader;
    BYTE        bSafePos;
    TItemPos	ItemPos;
} TPacketCGSafeboxCheckout;

typedef struct command_safebox_checkin
{
    BYTE        bHeader;
    BYTE        bSafePos;
    TItemPos	ItemPos;
} TPacketCGSafeboxCheckin;

typedef struct command_mall_checkout
{
    BYTE        bHeader;
    BYTE        bMallPos;
    TItemPos	ItemPos;
} TPacketCGMallCheckout;

///////////////////////////////////////////////////////////////////////////////////
// Party

typedef struct command_use_skill
{
    BYTE                bHeader;
    DWORD               dwVnum;
	DWORD				dwTargetVID;
} TPacketCGUseSkill;

typedef struct command_party_invite
{
    BYTE header;
    DWORD vid;
} TPacketCGPartyInvite;

typedef struct command_party_invite_answer
{
    BYTE header;
    DWORD leader_pid;
    BYTE accept;
} TPacketCGPartyInviteAnswer;

typedef struct command_party_remove
{
    BYTE header;
    DWORD pid;
} TPacketCGPartyRemove;

typedef struct command_party_set_state
{
    BYTE byHeader;
    DWORD dwVID;
	BYTE byState;
    BYTE byFlag;
} TPacketCGPartySetState;

typedef struct packet_party_link
{
    BYTE header;
    DWORD pid;
    DWORD vid;
} TPacketGCPartyLink;

typedef struct packet_party_unlink
{
    BYTE header;
    DWORD pid;
	DWORD vid;
} TPacketGCPartyUnlink;

typedef struct command_party_use_skill
{
    BYTE byHeader;
	BYTE bySkillIndex;
    DWORD dwTargetVID;
} TPacketCGPartyUseSkill;

enum
{
	GUILD_SUBHEADER_CG_ADD_MEMBER,
	GUILD_SUBHEADER_CG_REMOVE_MEMBER,
	GUILD_SUBHEADER_CG_CHANGE_GRADE_NAME,
	GUILD_SUBHEADER_CG_CHANGE_GRADE_AUTHORITY,
	GUILD_SUBHEADER_CG_OFFER,
	GUILD_SUBHEADER_CG_POST_COMMENT,
	GUILD_SUBHEADER_CG_DELETE_COMMENT,
	GUILD_SUBHEADER_CG_REFRESH_COMMENT,
	GUILD_SUBHEADER_CG_CHANGE_MEMBER_GRADE,
	GUILD_SUBHEADER_CG_USE_SKILL,
	GUILD_SUBHEADER_CG_CHANGE_MEMBER_GENERAL,
	GUILD_SUBHEADER_CG_GUILD_INVITE_ANSWER,
	GUILD_SUBHEADER_CG_CHARGE_GSP,
	GUILD_SUBHEADER_CG_DEPOSIT_MONEY,
	GUILD_SUBHEADER_CG_WITHDRAW_MONEY,
};

typedef struct command_guild
{
    BYTE byHeader;
	BYTE bySubHeader;
} TPacketCGGuild;

typedef struct command_guild_answer_make_guild
{
	BYTE header;
	char guild_name[GUILD_NAME_MAX_LEN+1];
} TPacketCGAnswerMakeGuild;

typedef struct command_give_item
{
	BYTE byHeader;
	DWORD dwTargetVID;
	TItemPos ItemPos;
	BYTE byItemCount;
} TPacketCGGiveItem;

typedef struct SPacketCGHack
{
    BYTE        bHeader;
#ifdef ENABLE_CSHIELD
	DWORD		errorcode;
	BYTE		win;
	char		language[3 + 1];
	char		report[1024 + 1];
#else
    char        szBuf[255 + 1];
#endif
} TPacketCGHack;

typedef struct command_dungeon
{
	BYTE		bHeader;
	WORD		size;
} TPacketCGDungeon;

// Private Shop
typedef struct SShopItemTable
{
    DWORD		vnum;
    BYTE		count;
#ifdef ENABLE_MULTISHOP
	DWORD		wPriceVnum;
	DWORD		wPrice;
#endif
    TItemPos	pos;
    DWORD		price;
#ifdef ENABLE_CHEQUE_SYSTEM
	DWORD		cheque;
#endif
    BYTE		display_pos;
} TShopItemTable;

typedef struct SPacketCGMyShop
{
    BYTE        bHeader;
    char        szSign[SHOP_SIGN_MAX_LEN + 1];
    BYTE        bCount;	// count of TShopItemTable, max 39
} TPacketCGMyShop;

typedef struct SPacketCGRefine
{
	BYTE		header;
	BYTE		pos;
	BYTE		type;
} TPacketCGRefine;

#if defined(__BL_OFFICIAL_LOOT_FILTER__)
typedef struct SPacketCGLootFilter
{
	BYTE		header;
	BYTE		settings[ELootFilter::LOOT_FILTER_SETTINGS_MAX];
} TPacketCGLootFilter;

typedef struct SPacketGCLootFilter
{
	BYTE		header;
	bool		enable;
	DWORD		vid;
} TPacketGCLootFilter;
#endif

typedef struct SPacketCGChangeName
{
    BYTE header;
    BYTE index;
    char name[CHARACTER_NAME_MAX_LEN+1];
} TPacketCGChangeName;

typedef struct command_client_version2
{
	BYTE header;
	char filename[32+1];
	char timestamp[32+1];
} TPacketCGClientVersion2;

typedef struct command_crc_report
{
	BYTE header;
	BYTE byPackMode;
	DWORD dwBinaryCRC32;
	DWORD dwProcessCRC32;
	DWORD dwRootPackCRC32;
} TPacketCGCRCReport;

enum EPartyExpDistributionType
{
    PARTY_EXP_DISTRIBUTION_NON_PARITY,
    PARTY_EXP_DISTRIBUTION_PARITY,
};

typedef struct command_party_parameter
{
    BYTE        bHeader;
    BYTE        bDistributeMode;
} TPacketCGPartyParameter;

typedef struct command_quest_input_string
{
    BYTE        bHeader;
    char		szString[QUEST_INPUT_STRING_MAX_NUM+1];
} TPacketCGQuestInputString;

typedef struct command_quest_confirm
{
    BYTE header;
    BYTE answer;
    DWORD requestPID;
} TPacketCGQuestConfirm;

typedef struct command_script_select_item
{
    BYTE header;
    DWORD selection;
} TPacketCGScriptSelectItem;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// From Server
enum EPhase
{
    PHASE_CLOSE,
    PHASE_HANDSHAKE,
    PHASE_LOGIN,
    PHASE_SELECT,
    PHASE_LOADING,
    PHASE_GAME,
    PHASE_DEAD,

	PHASE_DBCLIENT_CONNECTING,
    PHASE_DBCLIENT,
    PHASE_P2P,
    PHASE_AUTH,
};

typedef struct packet_phase
{
    BYTE        header;
    BYTE        phase;
} TPacketGCPhase;

typedef struct packet_blank
{
	BYTE		header;
} TPacketGCBlank;

typedef struct packet_blank_dynamic
{
	BYTE		header;
	WORD		size;
} TPacketGCBlankDynamic;

typedef struct packet_header_handshake
{
	BYTE		header;
	DWORD		dwHandshake;
	DWORD		dwTime;
	LONG		lDelta;
} TPacketGCHandshake;

typedef struct packet_header_bindudp
{
	BYTE		header;
	DWORD		addr;
	WORD		port;
} TPacketGCBindUDP;

typedef struct packet_header_dynamic_size
{
	BYTE		header;
	WORD		size;
} TDynamicSizePacketHeader;

#ifdef ENABLE_LARGE_DYNAMIC_PACKETS
typedef struct packet_header_large_dynamic_size
{
	BYTE		header;
	int			size;
} TLargeDynamicSizePacketHeader;
#endif

typedef struct SSimplePlayerInformation
{
    DWORD               dwID;
    char                szName[CHARACTER_NAME_MAX_LEN + 1];
    BYTE                byJob;
    BYTE                byLevel;
    DWORD               dwPlayMinutes;
    BYTE                byST, byHT, byDX, byIQ;
    DWORD               wMainPart; // @fixme502
    BYTE                bChangeName;
    DWORD				wHairPart; // @fixme502
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	DWORD				wAccePart;
#endif
    BYTE                bDummy[4];
#ifdef __ANDROID__
	LONG				x, y;
#else
	LONG				x, y;
#endif
	LONG				lAddr;
	WORD				wPort;
	BYTE				bySkillGroup;
#ifdef ENABLE_CONQUEROR_LEVEL
	BYTE				byConquerorLevel;
	BYTE				bySungmaST, bySungmaHP, bySungmaMove, bySungmaInmune;
#endif
} TSimplePlayerInformation;

typedef struct packet_login_success3
{
	BYTE						header;
	TSimplePlayerInformation	akSimplePlayerInformation[PLAYER_PER_ACCOUNT3];
    DWORD						guild_id[PLAYER_PER_ACCOUNT3];
    char						guild_name[PLAYER_PER_ACCOUNT3][GUILD_NAME_MAX_LEN+1];
	DWORD handle;
	DWORD random_key;
} TPacketGCLoginSuccess3;

typedef struct packet_login_success4
{
	BYTE						header;
	TSimplePlayerInformation	akSimplePlayerInformation[PLAYER_PER_ACCOUNT4];
    DWORD						guild_id[PLAYER_PER_ACCOUNT4];
    char						guild_name[PLAYER_PER_ACCOUNT4][GUILD_NAME_MAX_LEN+1];
	DWORD handle;
	DWORD random_key;
} TPacketGCLoginSuccess4;

#ifdef ENABLE_PLAYER_PER_ACCOUNT5
typedef struct packet_login_success5
{
	BYTE						header;
	TSimplePlayerInformation	akSimplePlayerInformation[PLAYER_PER_ACCOUNT5];
    DWORD						guild_id[PLAYER_PER_ACCOUNT5];
    char						guild_name[PLAYER_PER_ACCOUNT5][GUILD_NAME_MAX_LEN+1];
	DWORD handle;
	DWORD random_key;
} TPacketGCLoginSuccess5;
#endif

enum { LOGIN_STATUS_MAX_LEN = 8 };
typedef struct packet_login_failure
{
	BYTE	header;
	char	szStatus[LOGIN_STATUS_MAX_LEN + 1];
} TPacketGCLoginFailure;

typedef struct command_player_create
{
	BYTE        header;
	BYTE        index;
	char        name[CHARACTER_NAME_MAX_LEN + 1];
	WORD        job;
	BYTE		shape;
	BYTE		CON;
	BYTE		INT;
	BYTE		STR;
	BYTE		DEX;
} TPacketCGCreateCharacter;

typedef struct command_player_create_success
{
    BYTE						header;
    BYTE						bAccountCharacterSlot;
    TSimplePlayerInformation	kSimplePlayerInfomation;
} TPacketGCPlayerCreateSuccess;

typedef struct command_create_failure
{
	BYTE	header;
	BYTE	bType;
} TPacketGCCreateFailure;

typedef struct command_player_delete
{
	BYTE        header;
	BYTE        index;
	char		szPrivateCode[PRIVATE_CODE_LENGTH];
} TPacketCGDestroyCharacter;

typedef struct packet_player_delete_success
{
	BYTE        header;
	BYTE        account_index;
} TPacketGCDestroyCharacterSuccess;

#if defined(__BL_KILL_BAR__)
typedef struct command_kill_bar
{
	BYTE	bHeader;
	BYTE	bKillerRace;
	BYTE	bKillerWeaponType;
	BYTE	bVictimRace;
	char	szKiller[CHARACTER_NAME_MAX_LEN + 1];
	char	szVictim[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGCKillBar;
#endif

enum
{
	ADD_CHARACTER_STATE_DEAD   = (1 << 0),
	ADD_CHARACTER_STATE_SPAWN  = (1 << 1),
	ADD_CHARACTER_STATE_GUNGON = (1 << 2),
	ADD_CHARACTER_STATE_KILLER = (1 << 3),
	ADD_CHARACTER_STATE_PARTY  = (1 << 4),
};

enum EPKModes
{
	PK_MODE_PEACE,
	PK_MODE_REVENGE,
	PK_MODE_FREE,
	PK_MODE_PROTECT,
	PK_MODE_GUILD,
	PK_MODE_MAX_NUM,
};

enum ECharacterEquipmentPart
{
	CHR_EQUIPPART_ARMOR,
	CHR_EQUIPPART_WEAPON,
	CHR_EQUIPPART_HEAD,
	CHR_EQUIPPART_HAIR,
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	CHR_EQUIPPART_ACCE,
#endif
#ifdef ENABLE_AURA_SYSTEM
	CHR_EQUIPPART_AURA,
#endif
	CHR_EQUIPPART_NUM,
};

typedef struct packet_char_additional_info
{
	BYTE    header;
	DWORD   dwVID;
	char    name[CHARACTER_NAME_MAX_LEN + 1];
	DWORD   awPart[CHR_EQUIPPART_NUM]; // @fixme502
#ifdef ELEMENT_SPELL_WORLDARD
	BYTE 	dwElementsEffect;
#endif	
	BYTE	bEmpire;
	DWORD   dwGuildID;
	DWORD   dwLevel;
	short   sAlignment;
	BYTE    bPKMode;
	DWORD   dwMountVnum;
#ifdef ENABLE_CONQUEROR_LEVEL
	DWORD	dwConquerorLevel;
#endif
#ifdef ENABLE_QUIVER_SYSTEM
	DWORD	dwArrow;
#endif
#if defined(WJ_COMBAT_ZONE)
	BYTE	combat_zone_rank;
#endif
} TPacketGCCharacterAdditionalInfo;

typedef struct packet_add_char
{
    BYTE        header;

    DWORD       dwVID;

    float       angle;
    LONG        x;
    LONG        y;
    LONG        z;

	BYTE		bType;
    DWORD       wRaceNum; // @fixme501
    BYTE        bMovingSpeed;
    BYTE        bAttackSpeed;

    BYTE        bStateFlag;
    DWORD       dwAffectFlag[2];
} TPacketGCCharacterAdd;

typedef struct packet_add_char2
{
    BYTE        header;

    DWORD       dwVID;

    char        name[CHARACTER_NAME_MAX_LEN + 1];

    float       angle;
    LONG        x;
    LONG        y;
    LONG        z;

	BYTE		bType;
    DWORD       wRaceNum; // @fixme501
    DWORD       awPart[CHR_EQUIPPART_NUM]; // @fixme502
#ifdef ELEMENT_SPELL_WORLDARD
	BYTE 	dwElementsEffect;
#endif
    BYTE        bMovingSpeed;
    BYTE        bAttackSpeed;

    BYTE        bStateFlag;
    DWORD       dwAffectFlag[2];
    BYTE        bEmpire;

    DWORD       dwGuild;
    short       sAlignment;

	BYTE		bPKMode;
	DWORD		dwMountVnum;
#ifdef ENABLE_CONQUEROR_LEVEL
	DWORD		dwConquerorLevel;
#endif
#ifdef ENABLE_QUIVER_SYSTEM
	DWORD		dwArrow;
#endif
} TPacketGCCharacterAdd2;

typedef struct packet_update_char
{
    BYTE        header;
    DWORD       dwVID;

    DWORD		awPart[CHR_EQUIPPART_NUM]; // @fixme502
#ifdef ELEMENT_SPELL_WORLDARD
	BYTE 	dwElementsEffect;
#endif
    BYTE        bMovingSpeed;
	BYTE		bAttackSpeed;

    BYTE        bStateFlag;
    DWORD       dwAffectFlag[2];

	DWORD		dwGuildID;
    short       sAlignment;

#ifdef NEW_PET_SYSTEM
	DWORD   dwLevel;
#endif
	BYTE		bPKMode;
	DWORD		dwMountVnum;
#ifdef ENABLE_CONQUEROR_LEVEL
	DWORD		dwConquerorLevel;
#endif
#ifdef ENABLE_QUIVER_SYSTEM
	DWORD		dwArrow;
#endif
#if defined(WJ_COMBAT_ZONE)
	DWORD		combat_zone_points;
#endif
} TPacketGCCharacterUpdate;

typedef struct packet_del_char
{
	BYTE	header;
    DWORD	dwVID;
} TPacketGCCharacterDelete;

typedef struct packet_GlobalTime
{
	BYTE	header;
	float	GlobalTime;
} TPacketGCGlobalTime;

enum EChatType
{
	CHAT_TYPE_TALKING,
	CHAT_TYPE_INFO,
	CHAT_TYPE_NOTICE,
	CHAT_TYPE_PARTY,
	CHAT_TYPE_GUILD,
	CHAT_TYPE_COMMAND,
	CHAT_TYPE_SHOUT,
	CHAT_TYPE_WHISPER,
	CHAT_TYPE_BIG_NOTICE,
	CHAT_TYPE_MONARCH_NOTICE,
#ifdef ENABLE_DICE_SYSTEM
	CHAT_TYPE_DICE_INFO, //11
#endif
#if defined(ENABLE_CHATTING_WINDOW_RENEWAL)
	CHAT_TYPE_EXP_INFO,
	CHAT_TYPE_ITEM_INFO,
	CHAT_TYPE_MONEY_INFO,
#endif
#ifdef ENABLE_12ZI
	CHAT_TYPE_MISSION,
	CHAT_TYPE_SUB_MISSION,
	CHAT_TYPE_CLEAR_MISSION,
#endif
	CHAT_TYPE_BOSS_KILL,
	CHAT_TYPE_INFO_QUEST,
	CHAT_TYPE_STONE_KILL,
	CHAT_TYPE_MAX_NUM,
};

typedef struct packet_chatting
{
	BYTE	header;
	WORD	size;
	BYTE	type;
	DWORD	dwVID;
	BYTE	bEmpire;
} TPacketGCChat;

typedef struct packet_whisper
{
    BYTE        bHeader;
    WORD        wSize;
    BYTE        bType;
    char        szNameFrom[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGCWhisper;

typedef struct packet_stun
{
	BYTE		header;
	DWORD		vid;
} TPacketGCStun;

#if defined(WJ_COMBAT_ZONE)
enum EPacketCGCombatZoneSubHeaderType
{
	COMBAT_ZONE_SUB_HEADER_NONE,
	COMBAT_ZONE_SUB_HEADER_ADD_LEAVING_TARGET,
	COMBAT_ZONE_SUB_HEADER_REMOVE_LEAVING_TARGET,	
	COMBAT_ZONE_SUB_HEADER_FLASH_ON_MINIMAP,
	COMBAT_ZONE_SUB_HEADER_OPEN_RANKING,
	COMBAT_ZONE_SUB_HEADER_REFRESH_SHOP,
};

typedef struct SPacketCGCombatZoneRequestAction
{
	BYTE	header;
	int		action;
	int		value;
} TPacketCGCombatZoneRequestAction;

typedef struct SPacketGCCombatZoneRanking
{
	int		rank;
	char	name[12 + 1];
	int		empire;
	int		points;
} TPacketGCCombatZoneRanking;

typedef struct SPacketGCCombatZoneRankingData
{
	BYTE header;
	TPacketGCCombatZoneRanking rankingData[11];
} TPacketGCCombatZoneRankingData;

typedef struct SPacketGCSendCombatZone
{
	BYTE	header;
	DWORD	sub_header;
	DWORD	m_pInfoData[4];
	DWORD	m_pDataDays[7][8];
	bool	isRunning;
} TPacketGCSendCombatZone;
#endif

typedef struct packet_dead
{
	BYTE		header;
	DWORD		vid;
} TPacketGCDead;

typedef struct packet_main_character
{
    BYTE        header;
    DWORD       dwVID;
	DWORD		wRaceNum; // @fixme501
    char        szName[CHARACTER_NAME_MAX_LEN + 1];
    LONG        lX, lY, lZ;
	BYTE		bySkillGroup;
} TPacketGCMainCharacter;

// SUPPORT_BGM
typedef struct packet_main_character2_empire
{
    BYTE        header;
    DWORD       dwVID;
	DWORD		wRaceNum; // @fixme501
    char        szName[CHARACTER_NAME_MAX_LEN + 1];
    LONG        lX, lY, lZ;
	BYTE		byEmpire;
	BYTE		bySkillGroup;
} TPacketGCMainCharacter2_EMPIRE;

typedef struct packet_main_character3_bgm
{
	enum
	{
		MUSIC_NAME_MAX_LEN = 24,
	};
    BYTE        header;
    DWORD       dwVID;
	DWORD		wRaceNum; // @fixme501
    char        szUserName[CHARACTER_NAME_MAX_LEN + 1];
	char        szBGMName[MUSIC_NAME_MAX_LEN + 1];
    LONG        lX, lY, lZ;
	BYTE		byEmpire;
	BYTE		bySkillGroup;
} TPacketGCMainCharacter3_BGM;

typedef struct packet_main_character4_bgm_vol
{
	enum
	{
		MUSIC_NAME_MAX_LEN = 24,
	};
    BYTE        header;
    DWORD       dwVID;
	DWORD		wRaceNum; // @fixme501
    char        szUserName[CHARACTER_NAME_MAX_LEN + 1];
	char        szBGMName[MUSIC_NAME_MAX_LEN + 1];
	float		fBGMVol;
    LONG        lX, lY, lZ;
	BYTE		byEmpire;
	BYTE		bySkillGroup;
} TPacketGCMainCharacter4_BGM_VOL;
// END_OF_SUPPORT_BGM

enum eFlyTypes
{
	FLY_NONE,
	FLY_EXP,
	FLY_HP_MEDIUM,
	FLY_HP_BIG,
	FLY_SP_SMALL,
	FLY_SP_MEDIUM,
	FLY_SP_BIG,
	FLY_FIREWORK1,
	FLY_FIREWORK2,
	FLY_FIREWORK3,
	FLY_FIREWORK4,
	FLY_FIREWORK5,
	FLY_FIREWORK6,
	FLY_FIREWORK_XMAS,
	FLY_CHAIN_LIGHTNING,
	FLY_HP_SMALL,
	FLY_SKILL_MUYEONG,
#ifdef ENABLE_QUIVER_SYSTEM
	FLY_QUIVER_ATTACK_NORMAL, // 17 ItemProto Value0
#endif
#ifdef ENABLE_CONQUEROR_LEVEL
	FLY_CONQUEROR_EXP,
#endif
};

enum EPointTypes
{
    POINT_NONE,						// 0
    POINT_LEVEL,					// 1
    POINT_VOICE,					// 2
    POINT_EXP,						// 3
    POINT_NEXT_EXP,					// 4
    POINT_HP,						// 5
    POINT_MAX_HP,					// 6
    POINT_SP,						// 7
    POINT_MAX_SP,					// 8
    POINT_STAMINA,
    POINT_MAX_STAMINA,

    POINT_GOLD,						// 11
    POINT_ST,
    POINT_HT,
    POINT_DX,
    POINT_IQ,
    POINT_ATT_POWER,
    POINT_ATT_SPEED,
    POINT_EVADE_RATE,
    POINT_MOV_SPEED,
    POINT_DEF_GRADE,
	POINT_CASTING_SPEED,
	POINT_MAGIC_ATT_GRADE,
    POINT_MAGIC_DEF_GRADE,
    POINT_EMPIRE_POINT,
    POINT_LEVEL_STEP,
    POINT_STAT,
	POINT_SUB_SKILL,
	POINT_SKILL,
	POINT_MIN_ATK,
	POINT_MAX_ATK,
    POINT_PLAYTIME,
    POINT_HP_REGEN,
    POINT_SP_REGEN,

    POINT_BOW_DISTANCE,

    POINT_HP_RECOVERY,
    POINT_SP_RECOVERY,

    POINT_POISON_PCT,
    POINT_STUN_PCT,
    POINT_SLOW_PCT,
    POINT_CRITICAL_PCT,
    POINT_PENETRATE_PCT,
    POINT_CURSE_PCT,

    POINT_ATTBONUS_HUMAN,
    POINT_ATTBONUS_ANIMAL,
    POINT_ATTBONUS_ORC,
    POINT_ATTBONUS_MILGYO,
    POINT_ATTBONUS_UNDEAD,
    POINT_ATTBONUS_DEVIL,
    POINT_ATTBONUS_INSECT,
    POINT_ATTBONUS_FIRE,
    POINT_ATTBONUS_ICE,
    POINT_ATTBONUS_DESERT,
	POINT_ATTBONUS_MONSTER,			// 53
	POINT_ATTBONUS_WARRIOR,			// 54
	POINT_ATTBONUS_ASSASSIN,		// 55
	POINT_ATTBONUS_SURA,			// 56
	POINT_ATTBONUS_SHAMAN,			// 57
	POINT_ATTBONUS_TREE,			// 58

	POINT_RESIST_WARRIOR,			// 59
	POINT_RESIST_ASSASSIN,			// 60
	POINT_RESIST_SURA,				// 61
	POINT_RESIST_SHAMAN,			// 62

    POINT_STEAL_HP,
    POINT_STEAL_SP,

    POINT_MANA_BURN_PCT,

    POINT_DAMAGE_SP_RECOVER,

    POINT_BLOCK,
    POINT_DODGE,

    POINT_RESIST_SWORD,				// 69
    POINT_RESIST_TWOHAND,			// 70
    POINT_RESIST_DAGGER,			// 71
    POINT_RESIST_BELL,				// 72
    POINT_RESIST_FAN,				// 73
    POINT_RESIST_BOW,
    POINT_RESIST_FIRE,
    POINT_RESIST_ELEC,
    POINT_RESIST_MAGIC,
    POINT_RESIST_WIND,

    POINT_REFLECT_MELEE,

    POINT_REFLECT_CURSE,
    POINT_POISON_REDUCE,

    POINT_KILL_SP_RECOVER,
    POINT_EXP_DOUBLE_BONUS,			// 83
    POINT_GOLD_DOUBLE_BONUS,		// 84
    POINT_ITEM_DROP_BONUS,			// 85

    POINT_POTION_BONUS,				// 86
    POINT_KILL_HP_RECOVERY,			// 87

    POINT_IMMUNE_STUN,				// 88
    POINT_IMMUNE_SLOW,				// 89
    POINT_IMMUNE_FALL,				// 90
    //////////////////

    POINT_PARTY_ATT_GRADE,			// 91
    POINT_PARTY_DEF_GRADE,			// 92

    POINT_ATT_BONUS,				// 93
    POINT_DEF_BONUS,				// 94

    POINT_ATT_GRADE_BONUS,			// 95
    POINT_DEF_GRADE_BONUS,			// 96
    POINT_MAGIC_ATT_GRADE_BONUS,	// 97
    POINT_MAGIC_DEF_GRADE_BONUS,	// 98

    POINT_RESIST_NORMAL_DAMAGE,		// 99

	POINT_STAT_RESET_COUNT = 112,
    POINT_HORSE_SKILL = 113,

	POINT_MALL_ATTBONUS,
	POINT_MALL_DEFBONUS,
	POINT_MALL_EXPBONUS,
	POINT_MALL_ITEMBONUS,
	POINT_MALL_GOLDBONUS,
    POINT_MAX_HP_PCT,
    POINT_MAX_SP_PCT,

	POINT_SKILL_DAMAGE_BONUS,
	POINT_NORMAL_HIT_DAMAGE_BONUS,

    POINT_SKILL_DEFEND_BONUS,
    POINT_NORMAL_HIT_DEFEND_BONUS,
    POINT_PC_BANG_EXP_BONUS,		// 125
	POINT_PC_BANG_DROP_BONUS,

	POINT_ENERGY = 128,

	POINT_ENERGY_END_TIME = 129,

	POINT_COSTUME_ATTR_BONUS = 130,
	POINT_MAGIC_ATT_BONUS_PER = 131,
	POINT_MELEE_MAGIC_ATT_BONUS_PER = 132,

	POINT_RESIST_ICE = 133,
	POINT_RESIST_EARTH = 134,
	POINT_RESIST_DARK = 135,

	POINT_RESIST_CRITICAL = 136,
	POINT_RESIST_PENETRATE = 137,

#ifdef ENABLE_WOLFMAN_CHARACTER
	POINT_BLEEDING_REDUCE = 138,
	POINT_BLEEDING_PCT = 139,

	POINT_ATTBONUS_WOLFMAN = 140,
	POINT_RESIST_WOLFMAN = 141,
	POINT_RESIST_CLAW = 142,
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	POINT_ACCEDRAIN_RATE = 143,
#endif
#ifdef ENABLE_MAGIC_REDUCTION_SYSTEM
	POINT_RESIST_MAGIC_REDUCTION = 144,
#endif

#ifdef ENABLE_CHEQUE_SYSTEM
	POINT_CHEQUE = 145,
#endif
	POINT_RESIST_MOUNT_FALL		= 146,
	POINT_RESIST_HUMAN			= 147,

	POINT_ENCHANT_ELECT			= 148,
	POINT_ENCHANT_FIRE			= 149,
	POINT_ENCHANT_ICE			= 150,
	POINT_ENCHANT_WIND			= 151,
	POINT_ENCHANT_EARTH			= 152,
	POINT_ENCHANT_DARK			= 153,

	POINT_ATTBONUS_CZ			= 154,
	POINT_ATTBONUS_SWORD		= 155,
	POINT_ATTBONUS_TWOHAND		= 156,
	POINT_ATTBONUS_DAGGER		= 157,
	POINT_ATTBONUS_BELL			= 158,
	POINT_ATTBONUS_FAN			= 159,
	POINT_ATTBONUS_BOW			= 160,
#ifdef ENABLE_WOLFMAN_CHARACTER
	POINT_ATTBONUS_CLAW			= 161,
#endif
#ifdef ENABLE_ATTR_6TH_7TH_EXTEND
	POINT_NORMAL_HIT_DEFEND_BONUS_BOSS_OR_MORE	= 162,
	POINT_SKILL_DEFEND_BONUS_BOSS_OR_MORE		= 163,
	POINT_NORMAL_HIT_DAMAGE_BONUS_BOSS_OR_MORE	= 164,
	POINT_SKILL_DAMAGE_BONUS_BOSS_OR_MORE		= 165,
	POINT_ATTBONUS_STONE						= 166,
	POINT_DAMAGE_HP_RECOVERY					= 167,
	POINT_DAMAGE_SP_RECOVERY					= 168,
	POINT_ALIGNMENT_DAMAGE_BONUS				= 169,
	POINT_NORMAL_DAMAGE_GUARD					= 170,
	POINT_MORE_THEN_HP90_DAMAGE_REDUCE			= 171,
	POINT_FIRST_ATTRIBUTE_BONUS					= 172,
	POINT_SECOND_ATTRIBUTE_BONUS				= 173,
	POINT_THIRD_ATTRIBUTE_BONUS					= 174,
	POINT_FOURTH_ATTRIBUTE_BONUS				= 175,
	POINT_FIFTH_ATTRIBUTE_BONUS					= 176,
	POINT_RESIST_FIST			= 177,
	POINT_HIT_PCT								= 178,
	POINT_RANDOM								= 179,
	POINT_ATTBONUS_PER_HUMAN					= 180,
	POINT_ATTBONUS_PER_ANIMAL					= 181,
	POINT_ATTBONUS_PER_ORC						= 182,
	POINT_ATTBONUS_PER_MILGYO					= 183,
	POINT_ATTBONUS_PER_UNDEAD					= 184,
	POINT_ATTBONUS_PER_DEVIL					= 185,
	POINT_ENCHANT_PER_ELECT						= 186,
	POINT_ENCHANT_PER_FIRE						= 187,
	POINT_ENCHANT_PER_ICE						= 188,
	POINT_ENCHANT_PER_WIND						= 189,
	POINT_ENCHANT_PER_EARTH						= 190,
	POINT_ENCHANT_PER_DARK						= 191,
	POINT_ATTBONUS_PER_CZ						= 192,
	POINT_ATTBONUS_PER_INSECT					= 193,
	POINT_ATTBONUS_PER_DESERT					= 194,
	POINT_ATTBONUS_PER_STONE					= 195,
	POINT_ATTBONUS_PER_MONSTER					= 196,
	POINT_RESIST_PER_HUMAN						= 197,
	POINT_RESIST_PER_ICE						= 198,
	POINT_RESIST_PER_DARK						= 199,
	POINT_RESIST_PER_EARTH						= 200,
	POINT_RESIST_PER_FIRE						= 201,
	POINT_RESIST_PER_ELEC						= 202,
	POINT_RESIST_PER_MAGIC						= 203,
	POINT_RESIST_PER_WIND						= 204,
	POINT_MONSTER_DEFEND_BONUS					= 205,
#endif
#ifdef ENABLE_CONQUEROR_LEVEL
	POINT_CONQUEROR_LEVEL = 206,
	POINT_CONQUEROR_LEVEL_STEP = 207,
	POINT_SUNGMA_STR = 208,
	POINT_SUNGMA_HP = 209,
	POINT_SUNGMA_MOVE = 210,
	POINT_SUNGMA_INMUNE = 211,
	POINT_CONQUEROR_POINT = 212,
	POINT_CONQUEROR_EXP = 213,
	POINT_CONQUEROR_NEXT_EXP = 214,
	POINT_SUNGMA_PER_STR						= 215,
	POINT_SUNGMA_PER_HP							= 216,
	POINT_SUNGMA_PER_MOVE						= 217,
	POINT_SUNGMA_PER_INMUNE						= 218,
	POINT_HIT_BUFF_ENCHANT_FIRE					= 219,
	POINT_HIT_BUFF_ENCHANT_ICE					= 220,
	POINT_HIT_BUFF_ENCHANT_ELEC					= 221,
	POINT_HIT_BUFF_ENCHANT_WIND					= 222,
	POINT_HIT_BUFF_ENCHANT_DARK					= 223,
	POINT_HIT_BUFF_ENCHANT_EARTH				= 224,
	POINT_HIT_BUFF_RESIST_FIRE					= 225,
	POINT_HIT_BUFF_RESIST_ICE					= 226,
	POINT_HIT_BUFF_RESIST_ELEC					= 227,
	POINT_HIT_BUFF_RESIST_WIND					= 228,
	POINT_HIT_BUFF_RESIST_DARK					= 229,
	POINT_HIT_BUFF_RESIST_EARTH					= 230,
	POINT_HIT_BUFF_SUNGMA_STR					= 231,
	POINT_HIT_BUFF_SUNGMA_MOVE					= 232,
	POINT_HIT_BUFF_SUNGMA_HP					= 233,
	POINT_HIT_BUFF_SUNGMA_INMUNE				= 234,
#ifdef ELEMENT_SPELL_WORLDARD
	POINT_ATT_ELEMENTS = 235,
#endif
#endif
#ifdef ENABLE_GEM_SYSTEM
	POINT_GEM 				= 236,
#endif
	POINT_MIN_WEP = 237,
	POINT_MAX_WEP,
	POINT_MIN_MAGIC_WEP,
	POINT_MAX_MAGIC_WEP,
	POINT_HIT_RATE,
	POINT_MAX,
};

typedef struct packet_points
{
    BYTE        header;
    LONG        points[POINT_MAX_NUM];
} TPacketGCPoints;

typedef struct packet_point_change
{
    int         header;

	DWORD		dwVID;
	BYTE		Type;

	LONG        amount;
    LONG        value;
} TPacketGCPointChange;

typedef struct packet_motion
{
	BYTE		header;
	DWORD		vid;
	DWORD		victim_vid;
	WORD		motion;
} TPacketGCMotion;

typedef struct packet_set_item
{
	BYTE		header;
	TItemPos	Cell;
	DWORD		vnum;
	BYTE		count;
	LONG		alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
    TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#if defined(__BL_TRANSMUTATION__)
	DWORD		dwTransmutationVnum;
#endif
#ifdef ELEMENT_SPELL_WORLDARD
	BYTE 	    grade_element;
	DWORD 	 	attack_element[MAX_ELEMENTS_SPELL];
 	BYTE 		element_type_bonus;
 	short 		elements_value_bonus[MAX_ELEMENTS_SPELL];
#endif
#ifdef ENABLE_NEW_SOCKET_VALUE
	LONG alMinMaxValue[ITEM_MIN_MAX_VALUE_MAX_NUM];
#endif
} TPacketGCItemSet;

typedef struct packet_set_item2
{
	BYTE		header;
	TItemPos	Cell;
	DWORD		vnum;
	BYTE		count;
	DWORD		flags;
	DWORD		anti_flags;
	bool		highlight;
	LONG		alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
    TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#if defined(__BL_TRANSMUTATION__)
	DWORD		dwTransmutationVnum;
#endif
#ifdef ELEMENT_SPELL_WORLDARD
	BYTE 	    grade_element;
	DWORD 	 	attack_element[MAX_ELEMENTS_SPELL];
 	BYTE 		element_type_bonus;
 	short 		elements_value_bonus[MAX_ELEMENTS_SPELL];
#endif
#ifdef ENABLE_NEW_SOCKET_VALUE
	LONG alMinMaxValue[ITEM_MIN_MAX_VALUE_MAX_NUM];
#endif
#ifdef ENABLE_SOULBIND_SYSTEM
	LONG		sealbind;
#endif
} TPacketGCItemSet2;

typedef struct packet_item_del
{
    BYTE        header;
    BYTE        pos;
} TPacketGCItemDel;

#if defined(__BL_MAILBOX__)
typedef struct packet_mailbox_process
{
	packet_mailbox_process()
		: bArg1(0), bArg2(0) {}
	BYTE							bHeader;
	BYTE							bSubHeader;
	BYTE							bArg1;
	BYTE							bArg2;
} TPacketMailboxProcess;

typedef struct SMailBoxRespondUnreadData
{
	BYTE							bHeader;
	BYTE							bItemMessageCount;
	BYTE							bCommonMessageCount;
	bool							bGMVisible;
} TMailBoxRespondUnreadData;

typedef struct packet_mailbox_process_all
{
	BYTE							Index;
} TPacketGCMailboxProcessAll;

typedef struct packet_mailbox_add_data
{
	BYTE							bHeader;
	BYTE							Index;
	char							szFrom[CHARACTER_NAME_MAX_LEN + 1];
	char							szMessage[100 + 1];
	int								iYang;
	int								iWon;
	DWORD							ItemVnum;
	DWORD							ItemCount;
	LONG							alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute			aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#if defined(__BL_TRANSMUTATION__)
	DWORD							dwTransmutationVnum;
#endif
#ifdef ELEMENT_SPELL_WORLDARD
	BYTE 	    grade_element;
	DWORD 	 	attack_element[MAX_ELEMENTS_SPELL];
 	BYTE 		element_type_bonus;
 	short 		elements_value_bonus[MAX_ELEMENTS_SPELL];
#endif
#ifdef ENABLE_NEW_SOCKET_VALUE
	LONG alMinMaxValue[ITEM_MIN_MAX_VALUE_MAX_NUM];
#endif
} TPacketGCMailBoxAddData;

typedef struct packet_mailbox_message
{
	LONG							SendTime;
	LONG							DeleteTime;
	char							szTitle[25 + 1];
	bool							bIsGMPost;
	bool							bIsItemExist;
	bool							bIsConfirm;
} TPacketGCMailBoxMessage;

typedef struct packet_mailbox
{
	BYTE							bHeader;
	WORD							wSize;
} TPacketGCMailBox;

typedef struct packet_mailbox_write
{
	BYTE							bHeader;
	char							szName[CHARACTER_NAME_MAX_LEN + 1];
	char							szTitle[25 + 1];
	char							szMessage[100 + 1];
	TItemPos						pos;
	int								iYang;
	int								iWon;
} TPacketCGMailboxWrite;

typedef struct packet_mailbox_write_confirm
{
	BYTE							bHeader;
	char							szName[CHARACTER_NAME_MAX_LEN + 1];
} TPacketCGMailboxWriteConfirm;
#endif

typedef struct packet_use_item
{
	BYTE		header;
	TItemPos	Cell;
	DWORD		ch_vid;
	DWORD		victim_vid;

	DWORD		vnum;
} TPacketGCItemUse;

typedef struct packet_update_item
{
	BYTE		header;
	TItemPos	Cell;
	BYTE		count;
	LONG		alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
    TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#if defined(__BL_TRANSMUTATION__)
	DWORD		dwTransmutationVnum;
#endif
#ifdef ELEMENT_SPELL_WORLDARD
	BYTE 	    grade_element;
	DWORD 	 	attack_element[MAX_ELEMENTS_SPELL];
 	BYTE 		element_type_bonus;
 	short 		elements_value_bonus[MAX_ELEMENTS_SPELL];
#endif
#ifdef ENABLE_NEW_SOCKET_VALUE
	LONG alMinMaxValue[ITEM_MIN_MAX_VALUE_MAX_NUM];
#endif
#ifdef ENABLE_SOULBIND_SYSTEM
	LONG		sealbind;
#endif
} TPacketGCItemUpdate;

typedef struct packet_ground_add_item
{
    BYTE        bHeader;
    LONG        lX;
	LONG		lY;
	LONG		lZ;

    DWORD       dwVID;
    DWORD       dwVnum;
} TPacketGCItemGroundAdd;

typedef struct packet_ground_del_item
{
	BYTE		header;
	DWORD		vid;
} TPacketGCItemGroundDel;

typedef struct packet_item_ownership
{
    BYTE        bHeader;
    DWORD       dwVID;
    char        szName[CHARACTER_NAME_MAX_LEN + 1];
} TPacketGCItemOwnership;

typedef struct packet_quickslot_add
{
    BYTE        header;
    BYTE        pos;
	TQuickSlot	slot;
} TPacketGCQuickSlotAdd;

typedef struct packet_quickslot_del
{
    BYTE        header;
    BYTE        pos;
} TPacketGCQuickSlotDel;

typedef struct packet_quickslot_swap
{
    BYTE        header;
    BYTE        pos;
    BYTE        change_pos;
} TPacketGCQuickSlotSwap;

typedef struct packet_shop_start
{
	struct packet_shop_item		items[SHOP_HOST_ITEM_MAX_NUM];
#ifdef ENABLE_12ZI
	bool	islimiteditemshop;
#endif
} TPacketGCShopStart;

typedef struct packet_shop_start_ex
{
	typedef struct sub_packet_shop_tab
	{
		char name[SHOP_TAB_NAME_MAX];
		BYTE coin_type;
		packet_shop_item items[SHOP_HOST_ITEM_MAX_NUM];
	} TSubPacketShopTab;
	DWORD owner_vid;
	BYTE shop_tab_count;
#if defined(WJ_COMBAT_ZONE)
	DWORD points;
	DWORD curLimit;
	DWORD maxLimit;
#endif
} TPacketGCShopStartEx;

typedef struct packet_shop_update_item
{
	BYTE						pos;
	struct packet_shop_item		item;
} TPacketGCShopUpdateItem;

#if defined(__BL_MOVE_COSTUME_ATTR__)
typedef struct packet_cg_item_combination
{
	BYTE	Header;
	short	MediumIndex;
	short	BaseIndex;
	short	MaterialIndex;
} TPacketCGItemCombination;

typedef struct packet_cg_item_combination_cancel
{
	BYTE	Header;
} TPacketCGItemCombinationCancel;
#endif

typedef struct packet_shop_update_price
{
	int iElkAmount;
} TPacketGCShopUpdatePrice;

enum EPacketShopSubHeaders
{
	SHOP_SUBHEADER_GC_START,
    SHOP_SUBHEADER_GC_END,
	SHOP_SUBHEADER_GC_UPDATE_ITEM,
	SHOP_SUBHEADER_GC_UPDATE_PRICE,
    SHOP_SUBHEADER_GC_OK,
    SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY,
#ifdef ENABLE_MULTISHOP
	SHOP_SUBHEADER_GC_NOT_ENOUGH_ITEM,
#endif
    SHOP_SUBHEADER_GC_SOLDOUT,
    SHOP_SUBHEADER_GC_INVENTORY_FULL,
    SHOP_SUBHEADER_GC_INVALID_POS,
	SHOP_SUBHEADER_GC_SOLD_OUT,
#ifdef ENABLE_OFFLINESHOP_SYSTEM
	SHOP_SUBHEADER_GC_REFRESH_MONEY,
	SHOP_SUBHEADER_GC_CHECK_RESULT,
	SHOP_SUBHEADER_GC_REFRESH_DISPLAY_COUNT,
	SHOP_SUBHEADER_GC_REALWATCHER_COUNT,
	SHOP_SUBHEADER_GC_REFRESH_COUNT,
	SHOP_SUBHEADER_GC_CHANGE_TITLE,
#endif
	SHOP_SUBHEADER_GC_START_EX,
	SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY_EX,
#ifdef ENABLE_CHEQUE_SYSTEM
	SHOP_SUBHEADER_GC_NOT_ENOUGH_CHEQUE,
	SHOP_SUBHEADER_GC_NOT_ENOUGH_MONEY_CHEQUE,
#endif
#ifdef ENABLE_12ZI
	SHOP_SUBHEADER_GC_LIMITED_PURCHASE_OVER,
#endif
#if defined(WJ_COMBAT_ZONE)
	SHOP_SUBHEADER_GC_NOT_ENOUGH_POINTS,
	SHOP_SUBHEADER_GC_MAX_LIMIT_POINTS,
	SHOP_SUBHEADER_GC_OVERFLOW_LIMIT_POINTS,
#endif
};

typedef struct packet_shop
{
	BYTE        header;
	WORD		size;
	BYTE        subheader;
} TPacketGCShop;

typedef struct packet_exchange
{
    BYTE        header;
    BYTE        subheader;
    BYTE        is_me;
    DWORD       arg1;
    TItemPos       arg2;
    DWORD       arg3;
#ifdef WJ_ENABLE_TRADABLE_ICON
    TItemPos       arg4;
#endif
	LONG		alValues[ITEM_SOCKET_SLOT_MAX_NUM];
    TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#if defined(__BL_TRANSMUTATION__)
	DWORD		dwTransmutationVnum;
#endif
#ifdef ELEMENT_SPELL_WORLDARD
	BYTE 	    grade_element;
	DWORD 	 	attack_element[MAX_ELEMENTS_SPELL];
 	BYTE 		element_type_bonus;
 	short 		elements_value_bonus[MAX_ELEMENTS_SPELL];
#endif
#ifdef ENABLE_NEW_SOCKET_VALUE
	LONG alMinMaxValue[ITEM_MIN_MAX_VALUE_MAX_NUM];
#endif
} TPacketGCExchange;

enum
{
    EXCHANGE_SUBHEADER_GC_START,			// arg1 == vid
    EXCHANGE_SUBHEADER_GC_ITEM_ADD,			// arg1 == vnum  arg2 == pos  arg3 == count
	EXCHANGE_SUBHEADER_GC_ITEM_DEL,			// arg1 == pos
    EXCHANGE_SUBHEADER_GC_ELK_ADD,			// arg1 == elk
    EXCHANGE_SUBHEADER_GC_ACCEPT,			// arg1 == accept
    EXCHANGE_SUBHEADER_GC_END,				// arg1 == not used
    EXCHANGE_SUBHEADER_GC_ALREADY,			// arg1 == not used
    EXCHANGE_SUBHEADER_GC_LESS_ELK,			// arg1 == not used
#ifdef ENABLE_CHEQUE_SYSTEM
	EXCHANGE_SUBHEADER_GC_CHEQUE_ADD,
	EXCHANGE_SUBHEADER_GC_LESS_CHEQUE,
#endif
};

typedef struct packet_position
{
    BYTE        header;
	DWORD		vid;
    BYTE        position;
} TPacketGCPosition;

typedef struct packet_ping
{
	BYTE		header;
} TPacketGCPing;

typedef struct packet_pong
{
	BYTE		bHeader;
} TPacketCGPong;

typedef struct packet_script
{
    BYTE		header;
    WORD        size;
	BYTE		skin;
    WORD        src_size;
} TPacketGCScript;

typedef struct packet_target
{
    BYTE        header;
    DWORD       dwVID;
#if defined(ENABLE_SHIP_DEFENSE)
	bool bAlliance;
	int64_t iAllianceMinHP, iAllianceMaxHP;
#endif
    BYTE        bHPPercent;
#ifdef ENABLE_VIEW_TARGET_DECIMAL_HP
	int		iMinHP;
	int		iMaxHP;
#endif
} TPacketGCTarget;

typedef struct packet_damage_info
{
	BYTE header;
	DWORD dwVID;
	BYTE flag;
	int  damage;
} TPacketGCDamageInfo;

typedef struct packet_mount
{
    BYTE        header;
    DWORD       vid;
    DWORD       mount_vid;
    BYTE        pos;
	DWORD		_x, _y;
} TPacketGCMount;

typedef struct packet_change_speed
{
	BYTE		header;
	DWORD		vid;
	WORD		moving_speed;
} TPacketGCChangeSpeed;

typedef struct packet_move
{
	BYTE		bHeader;
	BYTE		bFunc;
	BYTE		bArg;
	BYTE		bRot;
	DWORD		dwVID;
	LONG		lX;
	LONG		lY;
	DWORD		dwTime;
	DWORD		dwDuration;
} TPacketGCMove;

enum
{
	QUEST_SEND_IS_BEGIN         = 1 << 0,
    QUEST_SEND_TITLE            = 1 << 1,
    QUEST_SEND_CLOCK_NAME       = 1 << 2,
    QUEST_SEND_CLOCK_VALUE      = 1 << 3,
    QUEST_SEND_COUNTER_NAME     = 1 << 4,
    QUEST_SEND_COUNTER_VALUE    = 1 << 5,
	QUEST_SEND_ICON_FILE		= 1 << 6,
};

typedef struct packet_quest_info
{
	BYTE header;
	WORD size;
	WORD index;
	BYTE flag;
#if defined(ENABLE_QUEST_RENEWAL)
	BYTE type;
	bool is_confirmed;
#endif
} TPacketGCQuestInfo;

typedef struct packet_quest_confirm
{
    BYTE header;
    char msg[64+1];
    LONG timeout;
    DWORD requestPID;
} TPacketGCQuestConfirm;

typedef struct packet_attack
{
    BYTE        header;
    DWORD       dwVID;
    DWORD       dwVictimVID;
    BYTE        bType;
} TPacketGCAttack;

typedef struct packet_c2c
{
	BYTE		header;
	WORD		wSize;
} TPacketGCC2C;

typedef struct packetd_sync_position_element
{
    DWORD       dwVID;
    LONG        lX;
    LONG        lY;
} TPacketGCSyncPositionElement;

typedef struct packetd_sync_position
{
    BYTE        bHeader;
    WORD		wSize;
} TPacketGCSyncPosition;

typedef struct packet_ownership
{
    BYTE                bHeader;
    DWORD               dwOwnerVID;
    DWORD               dwVictimVID;
} TPacketGCOwnership;

#define	SKILL_MAX_NUM 255

typedef struct packet_skill_level
{
    BYTE        bHeader;
    BYTE        abSkillLevels[SKILL_MAX_NUM];
} TPacketGCSkillLevel;

typedef struct SPlayerSkill
{
	BYTE bMasterType;
	BYTE bLevel;
	LONG tNextRead;
} TPlayerSkill;

typedef struct packet_skill_level_new
{
	BYTE bHeader;
	TPlayerSkill skills[SKILL_MAX_NUM];
} TPacketGCSkillLevelNew;

// fly
typedef struct packet_fly
{
    BYTE        bHeader;
    BYTE        bType;
    DWORD       dwStartVID;
    DWORD       dwEndVID;
} TPacketGCCreateFly;

enum EPVPModes
{
	PVP_MODE_NONE,
    PVP_MODE_AGREE,
    PVP_MODE_FIGHT,
    PVP_MODE_REVENGE,
};

typedef struct packet_duel_start
{
    BYTE	header ;
    WORD	wSize ;
} TPacketGCDuelStart ;

typedef struct packet_pvp
{
	BYTE		header;
	DWORD		dwVIDSrc;
	DWORD		dwVIDDst;
	BYTE		bMode;
} TPacketGCPVP;

typedef struct packet_skill_cooltime_end
{
	BYTE		header;
	BYTE		bSkill;
} TPacketGCSkillCoolTimeEnd;

typedef struct packet_warp
{
	BYTE			bHeader;
	LONG			lX;
	LONG			lY;
	LONG			lAddr;
	WORD			wPort;
} TPacketGCWarp;

typedef struct packet_party_invite
{
    BYTE header;
    DWORD leader_pid;
} TPacketGCPartyInvite;

typedef struct packet_party_add
{
    BYTE header;
    DWORD pid;
    char name[CHARACTER_NAME_MAX_LEN+1];
} TPacketGCPartyAdd;

typedef struct packet_party_update
{
    BYTE header;
    DWORD pid;
    BYTE state;
    BYTE percent_hp;
    short affects[PARTY_AFFECT_SLOT_MAX_NUM];
#if defined(__BL_PARTY_POSITION__)
	LONG x; 
	LONG y;
#endif
} TPacketGCPartyUpdate;

typedef struct packet_party_remove
{
    BYTE header;
    DWORD pid;
} TPacketGCPartyRemove;

typedef TPacketCGSafeboxCheckout TPacketGCSafeboxCheckout;
typedef TPacketCGSafeboxCheckin TPacketGCSafeboxCheckin;

typedef struct packet_safebox_wrong_password
{
    BYTE        bHeader;
} TPacketGCSafeboxWrongPassword;

typedef struct packet_safebox_size
{
	BYTE bHeader;
	BYTE bSize;
} TPacketGCSafeboxSize;

typedef struct packet_safebox_money_change
{
    BYTE bHeader;
    DWORD dwMoney;
} TPacketGCSafeboxMoneyChange;

typedef struct command_empire
{
    BYTE        bHeader;
    BYTE        bEmpire;
} TPacketCGEmpire;

typedef struct packet_empire
{
    BYTE        bHeader;
    BYTE        bEmpire;
} TPacketGCEmpire;

enum
{
	FISHING_SUBHEADER_GC_START,
	FISHING_SUBHEADER_GC_STOP,
	FISHING_SUBHEADER_GC_REACT,
	FISHING_SUBHEADER_GC_SUCCESS,
	FISHING_SUBHEADER_GC_FAIL,
    FISHING_SUBHEADER_GC_FISH,
};

typedef struct packet_fishing
{
    BYTE header;
    BYTE subheader;
    DWORD info;
    BYTE dir;
} TPacketGCFishing;

typedef struct paryt_parameter
{
    BYTE        bHeader;
    BYTE        bDistributeMode;
} TPacketGCPartyParameter;

//////////////////////////////////////////////////////////////////////////
// Guild

enum
{
    GUILD_SUBHEADER_GC_LOGIN,
	GUILD_SUBHEADER_GC_LOGOUT,
	GUILD_SUBHEADER_GC_LIST,
	GUILD_SUBHEADER_GC_GRADE,
	GUILD_SUBHEADER_GC_ADD,
	GUILD_SUBHEADER_GC_REMOVE,
	GUILD_SUBHEADER_GC_GRADE_NAME,
	GUILD_SUBHEADER_GC_GRADE_AUTH,
	GUILD_SUBHEADER_GC_INFO,
	GUILD_SUBHEADER_GC_COMMENTS,
    GUILD_SUBHEADER_GC_CHANGE_EXP,
    GUILD_SUBHEADER_GC_CHANGE_MEMBER_GRADE,
	GUILD_SUBHEADER_GC_SKILL_INFO,
	GUILD_SUBHEADER_GC_CHANGE_MEMBER_GENERAL,
	GUILD_SUBHEADER_GC_GUILD_INVITE,
    GUILD_SUBHEADER_GC_WAR,
    GUILD_SUBHEADER_GC_GUILD_NAME,
    GUILD_SUBHEADER_GC_GUILD_WAR_LIST,
    GUILD_SUBHEADER_GC_GUILD_WAR_END_LIST,
    GUILD_SUBHEADER_GC_WAR_POINT,
	GUILD_SUBHEADER_GC_MONEY_CHANGE,
#if defined(ENABLE_COMMUNITY_GUILD_RENEWAL)
	GUILD_SUBHEADER_GC_MEMBER_LOCATION,
#endif
};

typedef struct packet_guild
{
    BYTE header;
    WORD size;
    BYTE subheader;
} TPacketGCGuild;

// SubHeader - Grade
enum
{
    GUILD_AUTH_ADD_MEMBER       = (1 << 0),
    GUILD_AUTH_REMOVE_MEMBER    = (1 << 1),
    GUILD_AUTH_NOTICE           = (1 << 2),
    GUILD_AUTH_SKILL            = (1 << 3),
};

typedef struct packet_guild_sub_grade
{
	char grade_name[GUILD_GRADE_NAME_MAX_LEN+1];
	BYTE auth_flag;
} TPacketGCGuildSubGrade;

typedef struct packet_guild_sub_member
{
	DWORD pid;
	BYTE byGrade;
	BYTE byIsGeneral;
	BYTE byJob;
	BYTE byLevel;
	DWORD dwOffer;
	BYTE byNameFlag;
// if NameFlag is TRUE, name is sent from server.
//	char szName[CHARACTER_ME_MAX_LEN+1];
} TPacketGCGuildSubMember;

typedef struct packet_guild_sub_info
{
    WORD member_count;
    WORD max_member_count;
	DWORD guild_id;
    DWORD master_pid;
    DWORD exp;
    BYTE level;
    char name[GUILD_NAME_MAX_LEN+1];
	DWORD gold;
	BYTE hasLand;
} TPacketGCGuildInfo;

#if defined(ENABLE_COMMUNITY_GUILD_RENEWAL)
typedef struct packet_guild_member_location
{
	DWORD dwPID;
	BYTE bChannel;
	LONG lMapIndex;
} TPacketGCGuildMemberLocation;
#endif

enum EGuildWarState
{
    GUILD_WAR_NONE,
    GUILD_WAR_SEND_DECLARE,
    GUILD_WAR_REFUSE,
    GUILD_WAR_RECV_DECLARE,
    GUILD_WAR_WAIT_START,
    GUILD_WAR_CANCEL,
    GUILD_WAR_ON_WAR,
    GUILD_WAR_END,

    GUILD_WAR_DURATION = 2*60*60,
};

typedef struct packet_guild_war
{
    DWORD       dwGuildSelf;
    DWORD       dwGuildOpp;
    BYTE        bType;
    BYTE        bWarState;
} TPacketGCGuildWar;

typedef struct SPacketGuildWarPoint
{
    DWORD dwGainGuildID;
    DWORD dwOpponentGuildID;
    LONG lPoint;
} TPacketGuildWarPoint;

// SubHeader - Dungeon
enum
{
	DUNGEON_SUBHEADER_GC_TIME_ATTACK_START = 0,
	DUNGEON_SUBHEADER_GC_DESTINATION_POSITION = 1,
};

typedef struct packet_dungeon
{
	BYTE		bHeader;
    WORD		size;
    BYTE		subheader;
} TPacketGCDungeon;

// Private Shop

typedef struct SPacketGCTime
{
    BYTE        bHeader;
    LONG      time;
} TPacketGCTime;

enum
{
    WALKMODE_RUN,
    WALKMODE_WALK,
};

typedef struct SPacketGCWalkMode
{
    BYTE        header;
    DWORD       vid;
    BYTE        mode;
} TPacketGCWalkMode;

typedef struct SPacketGCChangeSkillGroup
{
    BYTE        header;
    BYTE        skill_group;
} TPacketGCChangeSkillGroup;

struct TMaterial
{
    DWORD vnum;
    DWORD count;
};

typedef struct SRefineTable
{
    DWORD src_vnum;
    DWORD result_vnum;
    BYTE material_count;
    int cost;
    int prob;
    TMaterial materials[REFINE_MATERIAL_MAX_NUM];
} TRefineTable;

typedef struct SPacketGCRefineInformation
{
	BYTE			header;
	BYTE			pos;
	TRefineTable	refine_table;
} TPacketGCRefineInformation;

typedef struct SPacketGCRefineInformationNew
{
	BYTE			header;
	BYTE			type;
	BYTE			pos;
	TRefineTable	refine_table;
} TPacketGCRefineInformationNew;

enum SPECIAL_EFFECT
{
	SE_NONE,
	SE_HPUP_RED,
	SE_SPUP_BLUE,
	SE_SPEEDUP_GREEN,
	SE_DXUP_PURPLE,
	SE_CRITICAL,
	SE_PENETRATE,
	SE_BLOCK,
	SE_DODGE,
	SE_CHINA_FIREWORK,
	SE_SPIN_TOP,
	SE_SUCCESS,
	SE_FAIL,
	SE_FR_SUCCESS,
    SE_LEVELUP_ON_14_FOR_GERMANY,
    SE_LEVELUP_UNDER_15_FOR_GERMANY,
    SE_PERCENT_DAMAGE1,
    SE_PERCENT_DAMAGE2,
    SE_PERCENT_DAMAGE3,
	SE_AUTO_HPUP,
	SE_AUTO_SPUP,
	SE_EQUIP_RAMADAN_RING,
	SE_EQUIP_HALLOWEEN_CANDY,
	SE_EQUIP_HAPPINESS_RING,
	SE_EQUIP_LOVE_PENDANT,
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	SE_EFFECT_ACCE_SUCESS_ABSORB,
	SE_EFFECT_ACCE_EQUIP,
#endif
#if defined(ENABLE_VOICE_CHAT)
	SE_VOICE_CHAT,
#endif
#ifdef VERSION_162_ENABLED
	SE_EFFECT_HEALER,
#endif
#ifdef ENABLE_12ZI
	SE_SKILL_DAMAGE_ZONE,
	SE_SKILL_SAFE_ZONE,

	SE_METEOR,
	SE_BEAD_RAIN,
	SE_FALL_ROCK,
	SE_ARROW_RAIN,
	SE_HORSE_DROP,
	SE_EGG_DROP,
	SE_DEAPO_BOOM,

	SE_SKILL_DAMAGE_ZONE_BIG,
	SE_SKILL_DAMAGE_ZONE_MIDDLE,
	SE_SKILL_DAMAGE_ZONE_SMALL,

	SE_SKILL_SAFE_ZONE_BIG,
	SE_SKILL_SAFE_ZONE_MIDDLE,
	SE_SKILL_SAFE_ZONE_SMALL,
#endif
#ifdef ENABLE_QUEEN_NETHIS
	SE_EFFECT_SNAKE_REGEN,
#endif
#if defined(WJ_COMBAT_ZONE)
	SE_COMBAT_ZONE_POTION,
#endif
	SE_AGGREGATE_MONSTER_EFFECT,

};

typedef struct SPacketGCSpecialEffect
{
    BYTE header;
    BYTE type;
    DWORD vid;
} TPacketGCSpecialEffect;

#ifdef ENABLE_12ZI
typedef struct SPacketGCSpecialZodiacEffect
{
	BYTE header;
	BYTE type;
	BYTE type2;
	DWORD vid;
	LONG x;
	LONG y;
} TPacketGCSpecialZodiacEffect;
#endif

typedef struct SPacketGCNPCPosition
{
    BYTE header;
	WORD size;
    WORD count;
} TPacketGCNPCPosition;

struct TNPCPosition
{
    BYTE bType;
    char name[CHARACTER_NAME_MAX_LEN+1];
    LONG x;
    LONG y;
};

typedef struct SPacketGCChangeName
{
    BYTE header;
    DWORD pid;
    char name[CHARACTER_NAME_MAX_LEN+1];
} TPacketGCChangeName;

enum EBlockAction
{
    BLOCK_EXCHANGE              = (1 << 0),
    BLOCK_PARTY_INVITE          = (1 << 1),
    BLOCK_GUILD_INVITE          = (1 << 2),
    BLOCK_WHISPER               = (1 << 3),
    BLOCK_MESSENGER_INVITE      = (1 << 4),
    BLOCK_PARTY_REQUEST         = (1 << 5),
};

typedef struct packet_login_key
{
	BYTE	bHeader;
	DWORD	dwLoginKey;
} TPacketGCLoginKey;

typedef struct packet_auth_success
{
    BYTE        bHeader;
    DWORD       dwLoginKey;
    BYTE        bResult;
} TPacketGCAuthSuccess;

typedef struct packet_channel
{
    BYTE header;
    BYTE channel;
} TPacketGCChannel;

#ifdef ENABLE_GUILD_TOKEN_AUTH
struct TPacketGCGuildToken {
	uint8_t header;
	uint64_t token;
};
#endif

typedef struct SEquipmentItemSet
{
	DWORD   vnum;
	BYTE    count;
	LONG    alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
#if defined(__BL_TRANSMUTATION__)
	DWORD		dwTransmutationVnum;
#endif

} TEquipmentItemSet;

typedef struct pakcet_view_equip
{
    BYTE header;
	DWORD dwVID;
	TEquipmentItemSet equips[WEAR_MAX_NUM];
} TPacketGCViewEquip;

typedef struct SLandPacketElement
{
    DWORD       dwID;
    LONG        x, y;
    LONG        width, height;
    DWORD       dwGuildID;
} TLandPacketElement;

typedef struct packet_land_list
{
    BYTE        header;
    WORD        size;
} TPacketGCLandList;

typedef struct SPacketGCTargetCreate
{
    BYTE        bHeader;
    LONG        lID;
    char        szTargetName[32+1];
} TPacketGCTargetCreate;

enum
{
	CREATE_TARGET_TYPE_NONE,
	CREATE_TARGET_TYPE_LOCATION,
	CREATE_TARGET_TYPE_CHARACTER,
#if defined(WJ_COMBAT_ZONE)	
	CREATE_TARGET_TYPE_COMBAT_ZONE = 4,
#endif
};

typedef struct SPacketGCTargetCreateNew
{
	BYTE		bHeader;
	LONG		lID;
	char		szTargetName[32+1];
	DWORD		dwVID;
	BYTE		byType;
} TPacketGCTargetCreateNew;

typedef struct SPacketGCTargetUpdate
{
    BYTE        bHeader;
    LONG        lID;
    LONG        lX, lY;
} TPacketGCTargetUpdate;

typedef struct SPacketGCTargetDelete
{
    BYTE        bHeader;
    LONG        lID;
} TPacketGCTargetDelete;

#if defined(__BL_67_ATTR__)
typedef struct command_67_attr
{
	BYTE			bHeader;
	BYTE			bMaterialCount;
	BYTE			bSupportCount;
	short			sSupportPos;
	short			sItemPos;
} TPacketCG67Attr;

typedef struct command_67_attr_open_close
{
	BYTE			bHeader;
} TPacket67AttrOpenClose;
#endif

typedef struct SPacketAffectElement
{
    DWORD       dwType;
    BYTE        bPointIdxApplyOn;
    LONG        lApplyValue;
    DWORD       dwFlag;
    LONG        lDuration;
    LONG        lSPCost;
} TPacketAffectElement;

typedef struct SPacketGCAffectAdd
{
    BYTE bHeader;
    TPacketAffectElement elem;
} TPacketGCAffectAdd;

typedef struct SPacketGCAffectRemove
{
    BYTE bHeader;
    DWORD dwType;
    BYTE bApplyOn;
} TPacketGCAffectRemove;

typedef struct packet_mall_open
{
	BYTE bHeader;
	BYTE bSize;
} TPacketGCMallOpen;

#ifdef ENABLE_CONQUEROR_LEVEL
typedef struct packet_sungma_attr_update
{
	BYTE bHeader;
	BYTE bSTR;
	BYTE bHP;
	BYTE bMOVE;
	BYTE bINMUNE;
} TPacketGCSungmaAttrUpdate;
#endif

typedef struct packet_lover_info
{
	BYTE bHeader;
	char szName[CHARACTER_NAME_MAX_LEN + 1];
	BYTE byLovePoint;
} TPacketGCLoverInfo;

typedef struct packet_love_point_update
{
	BYTE bHeader;
	BYTE byLovePoint;
} TPacketGCLovePointUpdate;

typedef struct packet_dig_motion
{
    BYTE header;
    DWORD vid;
    DWORD target_vid;
	BYTE count;
} TPacketGCDigMotion;

typedef struct SPacketGCOnTime
{
    BYTE header;
    int ontime;     // sec
} TPacketGCOnTime;

typedef struct SPacketGCResetOnTime
{
    BYTE header;
} TPacketGCResetOnTime;

typedef struct SPacketGCPanamaPack
{
    BYTE    bHeader;
    char    szPackName[256];
    BYTE    abIV[32];
} TPacketGCPanamaPack;

typedef struct SPacketGCHybridCryptKeys
{
private:
	SPacketGCHybridCryptKeys() : m_pStream(NULL) {}

public:
	SPacketGCHybridCryptKeys(int iStreamSize) : iKeyStreamLen(iStreamSize)
	{
		m_pStream = new BYTE[iStreamSize];
	}
	~SPacketGCHybridCryptKeys()
	{
		if( m_pStream )
		{
			delete[] m_pStream;
			m_pStream = NULL;
		}
	}
	static int GetFixedHeaderSize()
	{
		return sizeof(BYTE)+sizeof(WORD)+sizeof(int);
	}

	BYTE	bHeader;
	WORD    wDynamicPacketSize;
	int		iKeyStreamLen;
	BYTE*	m_pStream;
} TPacketGCHybridCryptKeys;

typedef struct SPacketGCHybridSDB
{
private:
	SPacketGCHybridSDB() : m_pStream(NULL) {}

public:
	SPacketGCHybridSDB(int iStreamSize) : iSDBStreamLen(iStreamSize)
	{
		m_pStream = new BYTE[iStreamSize];
	}
	~SPacketGCHybridSDB()
	{
		delete[] m_pStream;
		m_pStream = NULL;
	}
	static int GetFixedHeaderSize()
	{
		return sizeof(BYTE)+sizeof(WORD)+sizeof(int);
	}

	BYTE	bHeader;
	WORD    wDynamicPacketSize;
	int		iSDBStreamLen;
	BYTE*	m_pStream;
} TPacketGCHybridSDB;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Client To Client

typedef struct packet_state
{
	BYTE			bHeader;
	BYTE			bFunc;
	BYTE			bArg;
	BYTE			bRot;
	DWORD			dwVID;
	DWORD			dwTime;
	TPixelPosition	kPPos;
} TPacketCCState;

// AUTOBAN
typedef struct packet_autoban_quiz
{
    BYTE bHeader;
	BYTE bDuration;
    BYTE bCaptcha[64*32];
    char szQuiz[256];
} TPacketGCAutoBanQuiz;
// END_OF_AUTOBAN

#ifdef _IMPROVED_PACKET_ENCRYPTION_
struct TPacketKeyAgreement
{
	static const int MAX_DATA_LEN = 256;
	BYTE bHeader;
	WORD wAgreedLength;
	WORD wDataLength;
	BYTE data[MAX_DATA_LEN];
};

struct TPacketKeyAgreementCompleted
{
	BYTE bHeader;
	BYTE data[3]; // dummy (not used)
};
#endif // _IMPROVED_PACKET_ENCRYPTION_

typedef struct SPacketGCSpecificEffect
{
	BYTE header;
	DWORD vid;
	char effect_file[128];
} TPacketGCSpecificEffect;

enum EDragonSoulRefineWindowRefineType
{
	DragonSoulRefineWindow_UPGRADE,
	DragonSoulRefineWindow_IMPROVEMENT,
	DragonSoulRefineWindow_REFINE,
};

enum EPacketCGDragonSoulSubHeaderType
{
	DS_SUB_HEADER_OPEN,
	DS_SUB_HEADER_CLOSE,
	DS_SUB_HEADER_DO_UPGRADE,
	DS_SUB_HEADER_DO_IMPROVEMENT,
	DS_SUB_HEADER_DO_REFINE,
	DS_SUB_HEADER_REFINE_FAIL,
	DS_SUB_HEADER_REFINE_FAIL_MAX_REFINE,
	DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL,
	DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MONEY,
	DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL,
	DS_SUB_HEADER_REFINE_FAIL_TOO_MUCH_MATERIAL,
	DS_SUB_HEADER_REFINE_SUCCEED,
#if defined(ENABLE_DS_CHANGE_ATTR)
	DS_SUB_HEADER_OPEN_CHANGE_ATTR,
	DS_SUB_HEADER_DO_CHANGE_ATTR,
#endif
};

typedef struct SPacketCGDragonSoulRefine
{
	SPacketCGDragonSoulRefine() : header (HEADER_CG_DRAGON_SOUL_REFINE)
	{}
	BYTE header;
	BYTE bSubType;
	TItemPos ItemGrid[DS_REFINE_WINDOW_MAX_NUM];
} TPacketCGDragonSoulRefine;

typedef struct SPacketGCDragonSoulRefine
{
	SPacketGCDragonSoulRefine() : header(HEADER_GC_DRAGON_SOUL_REFINE)
	{}
	BYTE header;
	BYTE bSubType;
	TItemPos Pos;
} TPacketGCDragonSoulRefine;

typedef struct SChannelStatus
{
	WORD nPort; // @fixme024 (max port limit 32767 to 65535)
	BYTE bStatus;
} TChannelStatus;

#ifdef ENABLE_DS_SET
typedef struct STPacketDSTable {
	BYTE	bHeader;
	int		iType, iApplyCount, iBasicApplyValue[255], iAditionalApplyValue[255];
	float	fWeight;
} TPacketDSTable;
#endif

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
enum EAcceInfo
{
	ACCE_ABSORPTION_SOCKET = 0,
	ACCE_ABSORBED_SOCKET = 1,
	ACCE_CLEAN_ATTR_VALUE0 = 7,
	ACCE_WINDOW_MAX_MATERIALS = 2,
	ACCE_REVERSAL_VNUM_1 = 39046,
	ACCE_REVERSAL_VNUM_2 = 90000,
};

enum
{
	HEADER_CG_ACCE = 211,
	HEADER_GC_ACCE = 215,
	ACCE_SUBHEADER_GC_OPEN = 0,
	ACCE_SUBHEADER_GC_CLOSE,
	ACCE_SUBHEADER_GC_ADDED,
	ACCE_SUBHEADER_GC_REMOVED,
	ACCE_SUBHEADER_CG_REFINED,
	ACCE_SUBHEADER_CG_CLOSE = 0,
	ACCE_SUBHEADER_CG_ADD,
	ACCE_SUBHEADER_CG_REMOVE,
	ACCE_SUBHEADER_CG_REFINE,
};

typedef struct SPacketAcce
{
	BYTE	header;
	BYTE	subheader;
	bool	bWindow;
	DWORD	dwPrice;
	BYTE	bPos;
	TItemPos	tPos;
	DWORD	dwItemVnum;
	DWORD	dwMinAbs;
	DWORD	dwMaxAbs;
} TPacketAcce;

typedef struct SAcceMaterial
{
	BYTE	bHere;
	WORD	wCell;
} TAcceMaterial;

typedef struct SAcceResult
{
	DWORD	dwItemVnum;
	DWORD	dwMinAbs;
	DWORD	dwMaxAbs;
} TAcceResult;
#endif
#ifdef ENABLE_IKASHOP_RENEWAL
//ACTIONS PACKETS
typedef struct
{
	BYTE header;
#ifdef ENABLE_LARGE_DYNAMIC_PACKETS
	int size;
#else
	WORD size;
#endif
	BYTE subheader;
} TPacketGCNewOfflineshop;

typedef struct {
	BYTE header;
	WORD size;
	BYTE subheader;
} TPacketCGNewOfflineShop;

namespace ikashop
{
	typedef struct SPlayerItem
	{
		DWORD id;
		BYTE window;
		WORD pos;
		DWORD count;
		DWORD vnum;
		LONG alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
		TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
		DWORD owner;
#if defined(__BL_TRANSMUTATION__)
		DWORD dwTransmutationVnum;
#endif
#ifdef ELEMENT_SPELL_WORLDARD
	BYTE 	    grade_element;
	DWORD 	 	attack_element[MAX_ELEMENTS_SPELL];
 	BYTE 		element_type_bonus;
 	short 		elements_value_bonus[MAX_ELEMENTS_SPELL];
#endif
#ifdef ENABLE_NEW_SOCKET_VALUE
	LONG alMinMaxValue[ITEM_MIN_MAX_VALUE_MAX_NUM];
#endif
#ifdef ENABLE_SOULBIND_SYSTEM
	LONG		sealbind;
#endif
	} TPlayerItem;

	//patch 08-03-2020
	enum class ExpirationType
	{
		EXPIRE_NONE,
		EXPIRE_REAL_TIME,
		EXPIRE_REAL_TIME_FIRST_USE,
	};

	struct TPriceInfo
	{
		long long yang = 0;
#ifdef ENABLE_CHEQUE_SYSTEM
		int cheque = 0;
#endif
		long long GetTotalYangAmount() const {
			return yang
#ifdef ENABLE_CHEQUE_SYSTEM
				+ static_cast<long long>(cheque) * YANG_PER_CHEQUE
#endif
				;
		}
	};

	struct TShopPlayerItem : public TPlayerItem
	{
		ExpirationType expiration;
	};

	struct TShopItem : public TShopPlayerItem
	{
		TPriceInfo price;
	};

	struct TOfferInfo
	{
		DWORD offerid;
		DWORD ownerid;
		DWORD itemid;
		DWORD buyerid;
		TPriceInfo price;
		bool notified;
		bool accepted;
		char buyername[CHARACTER_NAME_MAX_LEN + 1];
		int64_t datetime;
	};

#ifdef ENABLE_IKASHOP_ENTITIES
	struct TShopSpawn
	{
		int x;
		int y;
		int map;
	};
#endif

	//AUCTION
	struct TAuctionInfo : public TShopItem
	{
		char  ownername[CHARACTER_NAME_MAX_LEN + 1];
		DWORD duration;
#ifdef ENABLE_IKASHOP_ENTITIES
#ifdef EXTEND_IKASHOP_ULTIMATE
		TShopSpawn spawn;
#endif
#endif
	};


	struct TAuctionOfferInfo
	{
		TPriceInfo	price;
		DWORD ownerid;
		DWORD buyerid;
		char buyername[CHARACTER_NAME_MAX_LEN + 1];
		bool seenflag;
		int64_t datetime;
	};

	struct TValutesInfo
	{
		long long yang = 0;

		void operator +=(const TValutesInfo& r)
		{
			yang += r.yang;
		}

		void operator -=(const TValutesInfo& r)
		{
			yang -= r.yang;
		}
	};


	struct TShopInfo
	{
		DWORD ownerid;
		DWORD duration;
		char name[OFFLINE_SHOP_NAME_MAX_LEN];
		DWORD count;
#ifdef ENABLE_IKASHOP_ENTITIES
		TShopSpawn spawn;
#endif
#ifdef EXTEND_IKASHOP_ULTIMATE
		int decoration;
		int decoration_time;
		int lock_index;
#endif
#ifdef ENABLE_IKASHOP_RENEWAL_TITEL
		char title[IKASHOP_TITLE_MAX_LEN + 1];
		BYTE title_type;
#endif
#ifdef ENABLE_SHOP_NAMES
		DWORD dwGuildID;
		DWORD dwLevel;
		short sAlignment;
		BYTE bEmpire;
#ifdef ENABLE_CONQUEROR_LEVEL
		DWORD dwConquerorLevel;
#endif
#endif
	};

	struct TFilterInfo
	{
		BYTE type;
		BYTE subtype;
		char name[CItemData::ITEM_NAME_MAX_LEN];
		TPriceInfo pricestart;
		TPriceInfo pricend;
		int levelstart;
		int levelend;
		TPlayerItemAttribute attrs[ITEM_ATTRIBUTE_SLOT_NORM_NUM];
		int sashGrade;
		int alchemyGrade;
	};

	struct TShopItemInfo
	{
		TItemPos pos;
		int destpos;
		TPriceInfo price;
	};

#ifdef EXTEND_IKASHOP_ULTIMATE
	struct TShopItemView : public ikashop::TShopItem
	{
		long long priceavg;
	};
#endif

#ifdef EXTEND_IKASHOP_ULTIMATE
	struct TSearchResultInfo : public ikashop::TShopItemView
#else
	struct TSearchResultInfo : public ikashop::TShopItem
#endif
	{
		bool auction;
		char seller_name[OFFLINE_SHOP_NAME_MAX_LEN];
		int duration;
	};

	//AUCTION
	struct TAuctionListElement
	{
		TAuctionInfo auction;
		TPriceInfo actual_best;
		DWORD offercount;
#ifdef EXTEND_IKASHOP_ULTIMATE
		long long priceavg;
#endif
	};

	struct TOfferListElement : public TOfferInfo
	{
		TShopItem item;
		char shopname[OFFLINE_SHOP_NAME_MAX_LEN];
		bool incoming;
#ifdef EXTEND_IKASHOP_ULTIMATE
		long long priceavg;
#endif
	};

#ifdef EXTEND_IKASHOP_PRO
	enum class ENotificationType : uint8_t
	{
		SELLER_SOLD_ITEM,
		SELLER_NEW_OFFER_ON_ITEM,
		SELLER_SHOP_EXPIRED,
		SELLER_ITEM_EXPIRED,
		SELLER_AUCTION_NEW_OFFER,
		SELLER_AUCTION_EXPIRED,
		SELLER_AUCTION_SOLD,
		SELLER_OFFER_CANCELLED,
		BUYER_OFFER_DENIED,
		BUYER_OFFER_ACCEPTED,
		BUYER_OFFER_EXPIRED,
		BUYER_AUCTION_BUY,
		BUYER_AUCTION_RAISE,
		BUYER_AUCTION_LOST,
#ifdef EXTEND_IKASHOP_ULTIMATE
		SELLER_DECORATION_EXPIRED,
#endif
	};

	struct TNotificationInfo
	{
		DWORD id;
		ENotificationType type;
		DWORD what;
		char who[OFFLINE_SHOP_NAME_MAX_LEN];
		char format[OFFLINESHOP_NOTIFICATION_FORMAT_LEN];
		int64_t datetime;
		bool seenflag;
	};
#endif


	//GAME TO CLIENT
	enum eSubHeaderGC
	{
		SUBHEADER_GC_SHOP_OPEN,
		SUBHEADER_GC_SHOP_OPEN_OWNER,
		SUBHEADER_GC_SHOP_OPEN_OWNER_NO_SHOP,
		SUBHEADER_GC_SHOP_EXPIRED_GUESTING,
		SUBHEADER_GC_SHOP_REMOVE_ITEM_OWNER,
		SUBHEADER_GC_SHOP_REMOVE_ITEM_GUEST,
		SUBHEADER_GC_SHOP_EDIT_ITEM_OWNER,
		SUBHEADER_GC_SHOP_EDIT_ITEM_GUEST,

		SUBHEADER_GC_OFFER_LIST,

		SUBHEADER_GC_SHOP_FILTER_RESULT,
		SUBHEADER_GC_SHOP_SAFEBOX_REFRESH,
		SUBHEADER_GC_SHOP_SAFEBOX_REMOVE_ITEM,
		SUBHEADER_GC_SHOP_SAFEBOX_ADD_ITEM,

		//AUCTION
		SUBHEADER_GC_AUCTION_LIST,
		SUBHEADER_GC_OPEN_MY_AUCTION,
		SUBHEADER_GC_OPEN_MY_AUCTION_NO_AUCTION,
		SUBHEADER_GC_OPEN_AUCTION,

		SUBHEDAER_GC_POPUP_MESSAGE,

#ifdef ENABLE_IKASHOP_ENTITIES
		SUBHEADER_GC_INSERT_SHOP_ENTITY,
		SUBHEADER_GC_REMOVE_SHOP_ENTITY,
#endif
		SUBHEADER_GC_BOARD_COUNTERS,
#ifdef EXTEND_IKASHOP_PRO
		SUBHEADER_GC_NOTIFICATION_LIST,
#endif
#ifdef EXTEND_IKASHOP_ULTIMATE
		SUBHEADER_GC_PRICE_AVERAGE_RESPONSE,
#endif
#ifdef ENABLE_IKASHOP_RENEWAL_TITEL
		SUBHEADER_GC_IKASHOP_TITLE,
		SUBHEADER_GC_IKASHOP_TITLE_CHANGE,
#endif
		SUBHEADER_GC_SEARCH_RESULT_DELETE,
	};

	struct TSubPacketGCShopOpen
	{
		TShopInfo shop;
	};

	struct TSubPacketGCShopOpenOwner
	{
		TShopInfo shop;
	};

	struct TSubPacketGCShopExpiredGuesting
	{
		DWORD ownerid;
	};

	struct TSubPacketGCShopRemoveItem
	{
		DWORD itemid;
	};

	struct TSubPacketGCShopEditItem
	{
		DWORD itemid;
		TPriceInfo price;
	};

	struct TSubPacketGCShopFilterResult
	{
		DWORD count;
	};

	struct TSubPacketGCShopOfferList
	{
		DWORD offercount;
	};

	struct TSubPacketGCShopSafeboxRefresh
	{
		TValutesInfo valute;
		DWORD itemcount;
	};

	struct TSubPacketGCShopSafeboxRemoveItem
	{
		DWORD itemid;
	};

	struct TSubPacketGCShoppingSafeboxAddItem
	{
		TShopPlayerItem item;
	};

	//AUCTION
	struct TSubPacketGCAuctionList
	{
		DWORD count;
	};

	struct TSubPacketGCAuctionOpen
	{
		TAuctionInfo auction;
		DWORD offercount;
#ifdef EXTEND_IKASHOP_ULTIMATE
		long long priceavg;
#endif
	};

	struct TSubPacketGCPopupMessage
	{
		static constexpr inline auto MESSAGE_LEN = 80;
		char localeString[MESSAGE_LEN];
	};

#ifdef ENABLE_IKASHOP_ENTITIES
	struct TSubPacketGCInsertShopEntity
	{
		DWORD vid;
		char name[OFFLINE_SHOP_NAME_MAX_LEN];
		int type;
		int x, y, z;
#ifdef ENABLE_SHOP_NAMES
		DWORD dwGuildID;
		DWORD dwLevel;
		short sAlignment;
		BYTE bEmpire;
#ifdef ENABLE_CONQUEROR_LEVEL
		DWORD	dwConquerorLevel;
#endif
#endif
	};

	struct TSubPacketGCRemoveShopEntity
	{
		DWORD vid;
	};
#endif

	struct TSubPacketGCBoardCounters
	{
		int safebox;
		int offers;
		int auction;
#ifdef EXTEND_IKASHOP_PRO
		int notification;
#endif
	};

#ifdef EXTEND_IKASHOP_PRO
	struct TSubPacketGCNotificationList
	{
		DWORD count;
	};
#endif

	// CLIENT TO GAME
	enum eSubHeaderCG
	{
		SUBHEADER_CG_SHOP_CREATE_NEW,
		SUBHEADER_CG_SHOP_FORCE_CLOSE,
		SUBHEADER_CG_SHOP_REQUEST_SHOPLIST,
		SUBHEADER_CG_SHOP_OPEN,
		SUBHEADER_CG_SHOP_OPEN_OWNER,
		SUBHEADER_CG_SHOP_BUY_ITEM,

		SUBHEADER_CG_SHOP_ADD_ITEM,
		SUBHEADER_CG_SHOP_REMOVE_ITEM,
		SUBHEADER_CG_SHOP_EDIT_ITEM,

		SUBHEADER_CG_SHOP_FILTER_REQUEST,
		SUBHEADER_CG_SHOP_SEARCH_FILL_REQUEST,

		SUBHEADER_CG_SHOP_OFFER_CREATE,
		SUBHEADER_CG_SHOP_OFFER_ACCEPT,
		SUBHEADER_CG_SHOP_OFFER_CANCEL,
		SUBHEADER_CG_SHOP_REQUEST_OFFER_LIST,

		SUBHEADER_CG_SHOP_SAFEBOX_OPEN,
		SUBHEADER_CG_SHOP_SAFEBOX_GET_ITEM,
		SUBHEADER_CG_SHOP_SAFEBOX_GET_VALUTES,
		SUBHEADER_CG_SHOP_SAFEBOX_CLOSE,

		//AUCTION
		SUBHEADER_CG_AUCTION_LIST_REQUEST,
		SUBHEADER_CG_AUCTION_OPEN_REQUEST,
		SUBHEADER_CG_MY_AUCTION_OPEN_REQUEST,
		SUBHEADER_CG_MY_AUCTION_CLOSE,
		SUBHEADER_CG_MY_AUCTION_CANCEL,
		SUBHEADER_CG_CREATE_AUCTION,
		SUBHEADER_CG_AUCTION_ADD_OFFER,
		SUBHEADER_CG_EXIT_FROM_AUCTION,

		SUBHEADER_CG_CLOSE_MY_SHOP_BOARD,
		SUBHEADER_CG_CLOSE_OFFER_LIST,
		SUBHEADER_CG_CLOSE_SHOP_GUEST,
#ifdef ENABLE_IKASHOP_ENTITIES
		SUBHEADER_CG_CLICK_ENTITY,
#ifdef EXTEND_IKASHOP_PRO
		SUBHEADER_CG_MOVE_SHOP_ENTITY,
#endif
#endif
#ifdef EXTEND_IKASHOP_PRO
		SUBHEADER_CG_NOTIFICATION_LIST_REQUEST,
		SUBHEADER_CG_NOTIFICATION_LIST_CLOSE,
#endif
#ifdef EXTEND_IKASHOP_ULTIMATE
		SUBHEADER_CG_PRICE_AVERAGE_REQUEST,
		SUBHEADER_CG_SHOP_DECORATION_USE,
		SUBHEADER_CG_SHOP_MOVE_ITEM,
#endif
#ifdef ENABLE_IKASHOP_RENEWAL_TITEL
		SUBHEADER_CG_IKASHOP_TITLE_CHANGE,
#endif
	};

	struct TSubPacketCGShopOpen
	{
		DWORD ownerid;
	};

	struct TSubPacketCGAddItem
	{
		TItemPos pos;
		TPriceInfo price;
		int destpos;
	};

	struct TSubPacketCGRemoveItem
	{
		DWORD itemid;
	};

	struct TSubPacketCGEditItem
	{
		DWORD itemid;
		TPriceInfo price;
	};

	struct TSubPacketCGFilterRequest
	{
		TFilterInfo filter;
	};

	struct TSubPacketCGOfferCreate
	{
		TOfferInfo offer;
	};

	struct TSubPacketCGOfferAccept
	{
		DWORD offerid;
	};

	struct TSubPacketCGOfferCancel
	{
		DWORD offerid;
		DWORD ownerid;
	};

	struct TSubPacketCGShopSafeboxGetItem
	{
		DWORD itemid;
	};

	struct TSubPacketCGShopBuyItem
	{
		DWORD ownerid;
		DWORD itemid;
		bool searching;
		long long seenprice;
	};

	//AUCTION
	struct TSubPacketCGAuctionOpenRequest
	{
		DWORD ownerid;
	};

	struct TSubPacketCGAuctionCreate
	{
		TItemPos pos;
		TPriceInfo startprice;
	};

	struct TSubPacketCGAuctionAddOffer
	{
		DWORD ownerid;
		TPriceInfo price;
	};

	struct TSubPacketCGAuctionExitFrom
	{
		DWORD ownerid;
	};

#ifdef ENABLE_IKASHOP_ENTITIES
	struct TSubPacketCGShopClickEntity
	{
		DWORD vid;
#ifdef EXTEND_IKASHOP_ULTIMATE
		bool isAuction = false;
#endif
	};
#endif

#ifdef EXTEND_IKASHOP_ULTIMATE
	struct TSubPacketCGPriceAverageRequest
	{
		DWORD vnum;
		DWORD count;
	};

	struct TSubPacketCGShopMoveItem 
	{
		int srcPos;
		int destPos;
	};
#endif

#ifdef ENABLE_IKASHOP_RENEWAL_TITEL
	struct TSubPacketCGIkaShopTitleChange
	{
		char szTitle[IKASHOP_TITLE_MAX_LEN + 1];
		BYTE bTitleType;
	};
	struct TSubPacketGCIkaShopTitle
	{
		DWORD dwShopID;    // Entity VID for 3D position tracking
		DWORD dwOwnerID;   // Owner PID for opening shop
		char szTitle[IKASHOP_TITLE_MAX_LEN + 1];
		BYTE bTitleType;
	};
#endif

}

#endif

#if defined(ENABLE_VOICE_CHAT)
enum EVoiceChatTypes : uint8_t
{
	VOICE_CHAT_TYPE_NONE,
	VOICE_CHAT_TYPE_LOCAL,
	VOICE_CHAT_TYPE_PARTY,
	VOICE_CHAT_TYPE_GUILD,
	VOICE_CHAT_TYPE_MAX_NUM,
};

typedef struct SVoiceChatPacket
{
	uint8_t header;
	uint16_t size;
	uint16_t dataSize;
	uint16_t distance;
	uint8_t type;
	bool isEncoded;
	char name[CHARACTER_NAME_MAX_LEN + 1];
	uint32_t vid;
} TVoiceChatPacket;
#endif

#ifdef ENABLE_CSHIELD
typedef struct SPacketGCCShield
{
	BYTE	header;
	bool	start;
	DWORD	num;
} TPacketGCCShield;

typedef struct SPacketCGCShield
{
	BYTE	header;
	char	key[64 + 1];
	char	data[256 + 1];
} TPacketCGCShield;

typedef struct SPacketGCCShieldStopPlayer
{
	BYTE	header;
} TPacketGCCShieldStopPlayer;
#endif

#ifdef ENABLE_CAPTCHA
typedef struct SPacketGCCaptcha
{
	BYTE	header;
	char	captchaImage[4096];
} TPacketGCCaptcha;

typedef struct SPacketCGCaptcha
{
	BYTE	header;
	short	captchaResult;
} TPacketCGCaptcha;
#endif

#ifdef ENABLE_AURA_SYSTEM
enum EPacketGCAuraSubHeader
{
	AURA_SUBHEADER_GC_OPEN,
	AURA_SUBHEADER_GC_CLOSE,
	AURA_SUBHEADER_GC_SET_ITEM,
	AURA_SUBHEADER_GC_CLEAR_SLOT,
	AURA_SUBHEADER_GC_CLEAR_ALL,
	AURA_SUBHEADER_GC_CLEAR_RIGHT,
	AURA_SUBHEADER_GC_REFINE_INFO,
};

typedef struct SSubPacketGCAuraOpenClose
{
	BYTE	bAuraWindowType;
} TSubPacketGCAuraOpenClose;

typedef struct SSubPacketGCAuraSetItem
{
	TItemPos	Cell;
	TItemPos	AuraCell;
	TItemData	pItem;
} TSubPacketGCAuraSetItem;

typedef struct SSubPacketGCAuraClearSlot
{
	BYTE	bAuraSlotPos;
} TSubPacketGCAuraClearSlot;

typedef struct SSubPacketGCAuraRefineInfo
{
	BYTE	bAuraRefineInfoType;
	BYTE	bAuraRefineInfoLevel;
	BYTE	bAuraRefineInfoExpPercent;
} TSubPacketGCAuraRefineInfo;

enum EPacketCGAuraSubHeader
{
	AURA_SUBHEADER_CG_REFINE_CHECKIN,
	AURA_SUBHEADER_CG_REFINE_CHECKOUT,
	AURA_SUBHEADER_CG_REFINE_ACCEPT,
	AURA_SUBHEADER_CG_REFINE_CANCEL,
};

typedef struct SSubPacketCGAuraRefineCheckIn
{
	TItemPos	ItemCell;
	TItemPos	AuraCell;
	BYTE		byAuraRefineWindowType;
} TSubPacketCGAuraRefineCheckIn;

typedef struct SSubPacketCGAuraRefineCheckOut
{
	TItemPos	AuraCell;
	BYTE		byAuraRefineWindowType;
} TSubPacketCGAuraRefineCheckOut;

typedef struct SSubPacketCGAuraRefineAccept
{
	BYTE		byAuraRefineWindowType;
} TSubPacketCGAuraRefineAccept;

typedef struct SPacketGCAura
{
	SPacketGCAura() : bHeader(HEADER_GC_AURA) {}
	BYTE bHeader;
	WORD wSize;
	BYTE bSubHeader;
} TPacketGCAura;

typedef struct SPacketCGAura
{
	SPacketCGAura() : bHeader(HEADER_CG_AURA) {}
	BYTE bHeader;
	WORD wSize;
	BYTE bSubHeader;
} TPacketCGAura;
#endif

#ifdef ENABLE_ITEMSHOP
enum
{
	ITEMSHOP_LOAD,
	ITEMSHOP_LOG,
	ITEMSHOP_BUY,
	ITEMSHOP_DRAGONCOIN,
	ITEMSHOP_RELOAD,
	ITEMSHOP_LOG_ADD,
	ITEMSHOP_UPDATE_ITEM,
	ITEMSHOP_REDEEM_VOUCHER,
};
typedef struct SIShopData
{
	DWORD	id;
	DWORD	itemVnum;
	long long	itemPrice;
	int		topSellingIndex;
	BYTE	discount;
	int		offerTime;
	int		addedTime;
	long long	sellCount;
	int	week_limit;
	int	month_limit;
	int maxSellCount;
}TIShopData;
typedef struct SIShopLogData
{
	DWORD	accountID;
	char	playerName[CHARACTER_NAME_MAX_LEN + 1];
	char	buyDate[21];
	int		buyTime;
	char	ipAdress[16];
	DWORD	itemID;
	DWORD	itemVnum;
	int		itemCount;
	long long	itemPrice;
}TIShopLogData;
typedef struct SPacketGCItemShop
{
	BYTE	header;
	DWORD	size;
} TPacketGCItemShop;
#endif

#ifdef ENABLE_EVENT_MANAGER
typedef struct SPacketGCEventManager
{
	BYTE	header;
	DWORD	size;
} TPacketGCEventManager;
typedef struct event_struct_
{
	WORD	eventID;
	BYTE	eventIndex;
	int		startTime;
	int		endTime;
	BYTE	empireFlag;
	BYTE	channelFlag;
	DWORD	value[4];
	bool	eventStatus;
	bool	eventTypeOnlyStart;
	char	startTimeText[25];
	char	endTimeText[25];
}TEventManagerData;
enum
{
	EVENT_MANAGER_LOAD,
	EVENT_MANAGER_EVENT_STATUS,
	EVENT_MANAGER_REMOVE_EVENT,
	EVENT_MANAGER_UPDATE,

	BONUS_EVENT = 1,
	DOUBLE_BOSS_LOOT_EVENT = 2,
	DOUBLE_METIN_LOOT_EVENT = 3,
	DOUBLE_MISSION_BOOK_EVENT = 4,
	DUNGEON_COOLDOWN_EVENT = 5,
	DUNGEON_TICKET_LOOT_EVENT = 6,
	EMPIRE_WAR_EVENT = 7,
	MOONLIGHT_EVENT = 8,
	TOURNAMENT_EVENT = 9,
	WHELL_OF_FORTUNE_EVENT = 10,
	HALLOWEEN_EVENT = 11,
	NPC_SEARCH_EVENT = 12,
	EXP_EVENT = 13,
	ITEM_DROP_EVENT = 14,
	YANG_DROP_EVENT = 15,
	UPBOX_EVENT = 16,
	FOOTBALL_EVENT = 17,
	COSTUME_EVENT = 18,
	HAIR_EVENT = 19,
	OKEY_EVENT = 20,
	MOUNT1_DROP = 21,
	PET_EVENT = 22,
	HERB_EVENT = 23,
	KOR_EVENT = 24,
	DRAGON_EVENT = 25,
	CHOKO_EVENT = 26,
	SECRET_EVENT = 27,
	LOVE_EVENT = 28,
	FISH_EVENT = 29,
	SUPER_EVENT = 30,
	CHALL_EVENT = 31,
	KHAM_EVENT = 32,
	BNW_EVENT = 33,
	SHARDS_EVENT = 34,
	BLESSING_EVENT = 35,
};
#endif

#if defined(ENABLE_SKILLBOOK_COMB_SYSTEM)
typedef struct SPacketCGSkillBookCombination
{
	BYTE bHeader;
	BYTE bAction;
	TItemPos CombItemGrid[SKILLBOOK_COMB_SLOT_MAX];
} TPacketCGSkillBookCombination;
#endif

#ifdef ENABLE_SHOP_SEARCH_SYSTEM
typedef struct packet_priv_shop_item_set
{
	BYTE header;
	BYTE count;
	TOfflineShopItemData items[51];
} TPacketGCShopSearchItemSet;

typedef struct command_shop_search
{
	BYTE	header;
	int		ItemCat;
	BYTE	Value;
	DWORD	itemVnum;
	int		SubCat;
	BYTE	avg[2];
	BYTE	skill[2];
	BYTE	abs[2];
	BYTE	level[2];
	BYTE	refine[2];
	BYTE	sex;
	BYTE	enchant;
	BYTE	alchemy;
	BYTE	character;
} TPacketCGShopSearch;
#endif

#ifdef ENABLE_FISH_EVENT
enum
{
	HEADER_CG_FISH_EVENT_SEND = 216,
	HEADER_GC_FISH_EVENT_INFO = 217,
};

enum
{
	FISH_EVENT_SUBHEADER_BOX_USE,
	FISH_EVENT_SUBHEADER_SHAPE_ADD,
	FISH_EVENT_SUBHEADER_GC_REWARD,
	FISH_EVENT_SUBHEADER_GC_ENABLE,
};

typedef struct SPacketGCFishEvent {
	BYTE	bHeader;
	BYTE	bSubheader;
} TPacketCGFishEvent;

typedef struct SPacketGCFishEventInfo {
	BYTE	bHeader;
	BYTE	bSubheader;
	DWORD	dwFirstArg;
	DWORD	dwSecondArg;
} TPacketGCFishEventInfo;
#endif

#ifdef ENABLE_FISHING_TIMER
enum EFishingTimerTypes
{
	FISHING_TIMER_SUBHEADER_GC_RESET,
	FISHING_TIMER_SUBHEADER_GC_WAITING,
	FISHING_TIMER_SUBHEADER_GC_GET,
};

typedef struct SPacketGCFishingTimer
{
	BYTE header;
	BYTE subHeader;
	BYTE timer;
	DWORD vid;
} TPacketGCFishingTimer;
#endif

#if defined(ENABLE_NPC_LOCATION_HELPER)
enum ENPCLocationHelperCGSubHeader
{
	NPC_LOCATION_HELPER_CG_REQUEST_STATUS,
	NPC_LOCATION_HELPER_CG_WARP_TO_NPC,
	NPC_LOCATION_HELPER_CG_USE_TICKET,
	NPC_LOCATION_HELPER_CG_REQUEST_GUILD_LAND,
};

enum ENPCLocationHelperGCSubHeader
{
	NPC_LOCATION_HELPER_GC_STATUS,
	NPC_LOCATION_HELPER_GC_WARP_RESULT,
	NPC_LOCATION_HELPER_GC_GUILD_LAND,
};

typedef struct command_npc_location_helper_cg
{
	BYTE header;
	BYTE subheader;
	int mapIndex;
	DWORD vnum;
	int x;
	int y;
} TPacketCGNPCLocationHelper;

typedef struct command_npc_location_helper_gc
{
	BYTE header;
	BYTE subheader;
	BYTE result;
	BYTE active;
	DWORD cooldownRemain;
	int mapIndex;
	DWORD vnum;
	int x;
	int y;
} TPacketGCNPCLocationHelper;
#endif

#ifdef ENABLE_SWITCHBOT
enum ECGSwitchbotSubheader
{
	SUBHEADER_CG_SWITCHBOT_START,
	SUBHEADER_CG_SWITCHBOT_STOP,
};

struct TPacketCGSwitchbot
{
	BYTE header;
	int size;
	BYTE subheader;
	BYTE slot;
};

enum EGCSwitchbotSubheader
{
	SUBHEADER_GC_SWITCHBOT_UPDATE,
	SUBHEADER_GC_SWITCHBOT_UPDATE_ITEM,
	SUBHEADER_GC_SWITCHBOT_SEND_ATTRIBUTE_INFORMATION,
};

struct TPacketGCSwitchbot
{
	BYTE header;
	int size;
	BYTE subheader;
	BYTE slot;
};

struct TSwitchbotUpdateItem
{
	BYTE	slot;
	DWORD	vnum;
	BYTE	count;
	LONG	alSockets[ITEM_SOCKET_SLOT_MAX_NUM];
	TPlayerItemAttribute aAttr[ITEM_ATTRIBUTE_SLOT_MAX_NUM];
};
#endif

#ifdef ELEMENT_SPELL_WORLDARD
enum
{
	ELEMENT_SPELL_SUB_HEADER_OPEN,
	ELEMENT_SPELL_SUB_HEADER_OPEN_CHANGE,

	ELEMENTS_SPELL_SUB_HEADER_CLOSE,
	ELEMENTS_SPELL_SUB_HEADER_ADD_BONUS,
	ELEMENTS_SPELL_SUB_HEADER_CHANGE_BONUS,
};

typedef struct  SPacketCGElementsSpell
{
	BYTE header;
	BYTE subheader;
	int pos;
	BYTE type_select;
}TPacketCGElementsSpell;

typedef struct SPacketGCElementsSpell
{
	BYTE	header;
	BYTE    subheader;
	int		pos;
	bool 	func;
	int 	cost;
	BYTE 	grade_add;
} TPacketGCElementsSpell;
#endif

#ifdef ENABLE_CUBE_RENEWAL_WORLDARD

enum
{
	CUBE_RENEWAL_SUB_HEADER_OPEN_RECEIVE,
	CUBE_RENEWAL_SUB_HEADER_CLEAR_DATES_RECEIVE,
	CUBE_RENEWAL_SUB_HEADER_DATES_RECEIVE,
	CUBE_RENEWAL_SUB_HEADER_DATES_LOADING,

	CUBE_RENEWAL_SUB_HEADER_MAKE_ITEM,
	CUBE_RENEWAL_SUB_HEADER_CLOSE,
};


typedef struct  packet_send_cube_renewal
{
	BYTE header;
	BYTE subheader;
	DWORD index_item;
	DWORD count_item;
	DWORD index_item_improve;
}TPacketCGCubeRenewalSend;



typedef struct dates_cube_renewal
{
	DWORD npc_vnum;
	DWORD index;

	DWORD	vnum_reward;
	int		count_reward;

	bool 	item_reward_stackable;

	DWORD	vnum_material_1;
	int		count_material_1;

	DWORD	vnum_material_2;
	int		count_material_2;

	DWORD	vnum_material_3;
	int		count_material_3;

	DWORD	vnum_material_4;
	int		count_material_4;

	DWORD	vnum_material_5;
	int		count_material_5;

	int 	gold;
	int		gem;
	int 	percent;

	char 	category[100];
}TInfoDateCubeRenewal;

typedef struct packet_receive_cube_renewal
{
	packet_receive_cube_renewal(): header(HEADER_GC_CUBE_RENEWAL)
	{}

	BYTE header;
	BYTE subheader;
	TInfoDateCubeRenewal	date_cube_renewal;
}TPacketGCCubeRenewalReceive;
#endif

#if defined(__MINI_GAME_CATCH_KING__)
enum
{
	SUBHEADER_GC_CATCH_KING_EVENT_INFO,
	SUBHEADER_GC_CATCH_KING_START,
	SUBHEADER_GC_CATCH_KING_SET_CARD,
	SUBHEADER_GC_CATCH_KING_RESULT_FIELD,
	SUBHEADER_GC_CATCH_KING_SET_END_CARD,
	SUBHEADER_GC_CATCH_KING_REWARD
};

typedef struct SCatchKingCard
{
	SCatchKingCard() { bIndex = 0; bIsExposed = false; }
	SCatchKingCard(BYTE index, bool isExposed)
	{
		bIndex = index;
		bIsExposed = isExposed;
	}

	BYTE bIndex;
	bool bIsExposed;
} TCatchKingCard;

// SECURED: Strictly fixed size (3 bytes) to prevent TCP buffer desyncs.
typedef struct SPacketCGMiniGameCatchKing
{
	BYTE bHeader;
	BYTE bSubheader;
	BYTE bSubArgument;
} TPacketCGMiniGameCatchKing;

// DYNAMIC: wSize allows the server to safely append the varying payloads
typedef struct SPacketGCMiniGameCatchKing
{
	BYTE bHeader;
	WORD wSize;
	BYTE bSubheader;
} TPacketGCMiniGameCatchKing;

typedef struct SPacketGCMiniGameCatchKingResult
{
	DWORD dwPoints;
	BYTE bRowType;
	BYTE bCardPos;
	BYTE bCardValue;
	bool bKeepFieldCard;
	bool bDestroyHandCard;
	bool bGetReward;
	bool bIsFiveNearBy;
} TPacketGCMiniGameCatchKingResult;

typedef struct SPacketGCMiniGameCatchKingSetEndCard
{
	BYTE bCardPos;
	BYTE bCardValue;
} TPacketGCMiniGameCatchKingSetEndCard;
#endif

#ifdef ENABLE_TAROT
typedef struct SPacketCGMinigameBnw
{
	BYTE header;
	BYTE subheader;
} TPacketCGMinigameBnw;

enum
{
	MINIGAME_BNW_SUBHEADER_CG_START,
	MINIGAME_BNW_SUBHEADER_CG_SELECTED_CARD,
	MINIGAME_BNW_SUBHEADER_CG_FINISHED,
};

typedef struct SPacketGCMinigameBnw
{
	BYTE header;
	BYTE subheader;
} TPacketGCMinigameBnw;

enum
{
	MINIGAME_BNW_SUBHEADER_GC_START,
	MINIGAME_BNW_SUBHEADER_GC_DRAW_RESULT,
};

typedef struct SPacketGCMinigameBnwDrawResult
{
	BYTE result;
	BYTE playerPoints;
	BYTE enemyPoints;
} TPacketGCMinigameBnwDrawResult;
#endif

#ifdef BOT_KILLER
#pragma pack(push, 1)

typedef struct SPacketGCBotKiller
{
	BYTE	bHeader;
	DWORD	dwSessionKey;
	WORD	wTimeout;
	char	captchaImage[2110];
} TPacketGCBotKiller;

typedef struct command_bot_killer
{
	BYTE	bHeader;
	WORD	wX;
	WORD	wY;
	WORD	wMouseDistance;
	DWORD	dwSessionKey;
} TPacketCGBotKiller;

#ifdef ENABLE_MINI_GAME_YUTNORI
enum
{
	SUBHEADER_GC_START_GAME,
	SUBHEADER_GC_END_GAME,
	
	SUBHEADER_GC_THROW_YUT,
	SUBHEADER_GC_NEXT_TURN,
	SUBHEADER_GC_YUT_MOVE,
	SUBHEADER_GC_SHOW_AVAILABLE_AREA,
	SUBHEADER_GC_PUSH_CATCH_YUT,
	
	SUBHEADER_GC_SET_PROB,
	SUBHEADER_GC_SET_SCORE,
	SUBHEADER_GC_SET_REMAIN_COUNT,
};

typedef struct SPacketCGMiniGameYutNori {
	BYTE	bHeader;
	BYTE	bSubheader;
	BYTE	bSubArgument;
} TPacketCGMiniGameYutNori;

typedef struct SPacketGCMiniGameYutNori {
	BYTE	bHeader;
	WORD	wSize;
	BYTE	bSubheader;
} TPacketGCMiniGameYutNori;

typedef struct SPacketGCMiniGameYutNoriThrow {
	bool	bIsPc;
	char	cYut;
} TPacketGCMiniGameYutNoriThrow;

typedef struct SPacketGCMiniGameYutNoriNextTurn {
	bool	bPcTurn;
	BYTE	bState;
} TPacketGCMiniGameYutNoriNextTurn;

typedef struct SPacketGCMiniGameYutNoriMove {
	bool	bIsPc;
	BYTE	bUnitIndex;
	bool	bIsCatch;
	BYTE	bStartIndex;
	BYTE	bDestIndex;
} TPacketGCMiniGameYutNoriMove;

typedef struct SPacketGCMiniGameYutNoriAvailableArea {
	BYTE	bPlayerIndex;
	BYTE	bAvailableIndex;
} TPacketGCMiniGameYutNoriAvailableArea;

typedef struct SPacketGCMiniGameYutNoriPushCatchYut {
	bool	bIsPc;
	BYTE	bUnitIndex;
} TPacketGCMiniGameYutNoriPushCatchYut;
#endif

#pragma pack(pop)
#endif

#pragma pack(pop)
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
