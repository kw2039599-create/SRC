#pragma once
#include "../GameLib/ItemData.h"

struct SAffects
{
	enum
	{
		AFFECT_MAX_NUM = 32,
	};

	SAffects() : dwAffects(0) {}
	SAffects(const DWORD & c_rAffects)
	{
		__SetAffects(c_rAffects);
	}
	int operator = (const DWORD & c_rAffects)
	{
		__SetAffects(c_rAffects);
	}

	BOOL IsAffect(BYTE byIndex)
	{
		return dwAffects & (1 << byIndex);
	}

	void __SetAffects(const DWORD & c_rAffects)
	{
		dwAffects = c_rAffects;
	}

	DWORD dwAffects;
};

extern std::string g_strGuildSymbolPathName;

constexpr DWORD c_Name_Max_Length = 64;
constexpr DWORD c_FileName_Max_Length = 128;
constexpr DWORD c_Short_Name_Max_Length = 32;

constexpr DWORD c_Inventory_Page_Column = 5;
constexpr DWORD c_Inventory_Page_Row = 9;
constexpr DWORD c_Inventory_Page_Size = c_Inventory_Page_Column*c_Inventory_Page_Row; // x*y

constexpr DWORD c_Inventory_Page_Count = 5;

constexpr DWORD c_ItemSlot_Count = c_Inventory_Page_Size * c_Inventory_Page_Count;
constexpr DWORD c_Equipment_Count = 12;

constexpr DWORD c_Equipment_Start = c_ItemSlot_Count;

constexpr DWORD c_Equipment_Body	= c_Equipment_Start + CItemData::WEAR_BODY;
constexpr DWORD c_Equipment_Head	= c_Equipment_Start + CItemData::WEAR_HEAD;
constexpr DWORD c_Equipment_Shoes	= c_Equipment_Start + CItemData::WEAR_FOOTS;
constexpr DWORD c_Equipment_Wrist	= c_Equipment_Start + CItemData::WEAR_WRIST;
constexpr DWORD c_Equipment_Weapon	= c_Equipment_Start + CItemData::WEAR_WEAPON;
constexpr DWORD c_Equipment_Neck	= c_Equipment_Start + CItemData::WEAR_NECK;
constexpr DWORD c_Equipment_Ear		= c_Equipment_Start + CItemData::WEAR_EAR;
constexpr DWORD c_Equipment_Unique1	= c_Equipment_Start + CItemData::WEAR_UNIQUE1;
constexpr DWORD c_Equipment_Unique2	= c_Equipment_Start + CItemData::WEAR_UNIQUE2;
constexpr DWORD c_Equipment_Arrow	= c_Equipment_Start + CItemData::WEAR_ARROW;
constexpr DWORD c_Equipment_Shield	= c_Equipment_Start + CItemData::WEAR_SHIELD;

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
constexpr DWORD c_New_Equipment_Start = c_Equipment_Start + CItemData::WEAR_BELT;
constexpr DWORD c_New_Equipment_Count = 1;
constexpr DWORD c_Equipment_Belt  = c_Equipment_Start + CItemData::WEAR_BELT;
#endif
#ifdef ENABLE_PENDANT_SYSTEM
constexpr DWORD c_Equipment_Pendant  = c_Equipment_Start + CItemData::WEAR_PENDANT;
#endif
#ifdef ENABLE_GLOVE_SYSTEM
constexpr DWORD c_Equipment_Glove  = c_Equipment_Start + CItemData::WEAR_GLOVE;
#endif

#ifdef WJ_ENABLE_TRADABLE_ICON
enum ETopWindowTypes
{
	ON_TOP_WND_NONE,
	ON_TOP_WND_SHOP,
	ON_TOP_WND_EXCHANGE,
	ON_TOP_WND_SAFEBOX,
	ON_TOP_WND_PRIVATE_SHOP,
	ON_TOP_WND_ITEM_COMB,
	ON_TOP_WND_PET_FEED,
#if defined(__BL_MAILBOX__)
	ON_TOP_WND_MAILBOX,
#endif
#ifdef ENABLE_FISH_EVENT
	ON_TOP_WND_FISH_EVENT,
#endif
#if defined(__BL_67_ATTR__)
	ON_TOP_WND_ATTR_67,
#endif
	ON_TOP_WND_MAX,
};
#endif

enum EDragonSoulDeckType
{
	DS_DECK_1,
	DS_DECK_2,
	DS_DECK_MAX_NUM = 2,
};

enum EDragonSoulGradeTypes
{
	DRAGON_SOUL_GRADE_NORMAL,
	DRAGON_SOUL_GRADE_BRILLIANT,
	DRAGON_SOUL_GRADE_RARE,
	DRAGON_SOUL_GRADE_ANCIENT,
	DRAGON_SOUL_GRADE_LEGENDARY,
#ifdef ENABLE_DS_GRADE_MYTH
	DRAGON_SOUL_GRADE_MYTH,
#endif
	DRAGON_SOUL_GRADE_MAX,
};

enum EDragonSoulStepTypes
{
	DRAGON_SOUL_STEP_LOWEST,
	DRAGON_SOUL_STEP_LOW,
	DRAGON_SOUL_STEP_MID,
	DRAGON_SOUL_STEP_HIGH,
	DRAGON_SOUL_STEP_HIGHEST,
	DRAGON_SOUL_STEP_MAX,
};

#if defined(ENABLE_SKILLBOOK_COMB_SYSTEM)
const DWORD c_SkillBook_Comb_Size = 10;
enum ESkillBookComb
{
	SKILLBOOK_COMB_SLOT_MAX = c_SkillBook_Comb_Size
};
#endif

#ifdef ENABLE_COSTUME_SYSTEM
	const DWORD c_Costume_Slot_Start	= c_Equipment_Start + CItemData::WEAR_COSTUME_BODY;
	const DWORD	c_Costume_Slot_Body		= c_Costume_Slot_Start + CItemData::COSTUME_BODY;
	const DWORD	c_Costume_Slot_Hair		= c_Costume_Slot_Start + CItemData::COSTUME_HAIR;
#ifdef ENABLE_MOUNT_COSTUME_SYSTEM
	const DWORD	c_Costume_Slot_Mount	= c_Costume_Slot_Start + CItemData::COSTUME_MOUNT;
#endif
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	const DWORD	c_Costume_Slot_Acce		= c_Costume_Slot_Start + CItemData::COSTUME_ACCE;
#endif
#ifdef ENABLE_AURA_SYSTEM
	const DWORD c_Costume_Slot_Aura		= c_Equipment_Start + CItemData::WEAR_COSTUME_AURA;
#endif

#if defined(ENABLE_WEAPON_COSTUME_SYSTEM) || defined(ENABLE_ACCE_COSTUME_SYSTEM)
	const DWORD c_Costume_Slot_Count	= 4;
#elif defined(ENABLE_MOUNT_COSTUME_SYSTEM)
	const DWORD c_Costume_Slot_Count	= 3;
#else
	const DWORD c_Costume_Slot_Count	= 2;
#endif

	const DWORD c_Costume_Slot_End		= c_Costume_Slot_Start + c_Costume_Slot_Count;

#ifdef ENABLE_WEAPON_COSTUME_SYSTEM
	const DWORD	c_Costume_Slot_Weapon	= c_Equipment_Start + CItemData::WEAR_COSTUME_WEAPON; // c_Costume_Slot_End + 1;
#endif

#endif

const DWORD c_Wear_Max = CItemData::WEAR_MAX_NUM;
const DWORD c_DragonSoul_Equip_Start = c_ItemSlot_Count + c_Wear_Max;
const DWORD c_DragonSoul_Equip_Slot_Max = 7;
const DWORD c_DragonSoul_Equip_End = c_DragonSoul_Equip_Start + c_DragonSoul_Equip_Slot_Max * DS_DECK_MAX_NUM;

const DWORD c_DragonSoul_Equip_Reserved_Count = c_DragonSoul_Equip_Slot_Max * 3;

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
	const DWORD c_Belt_Inventory_Slot_Start = c_DragonSoul_Equip_End + c_DragonSoul_Equip_Reserved_Count;
	const DWORD c_Belt_Inventory_Width = 4;
	const DWORD c_Belt_Inventory_Height= 4;
	const DWORD c_Belt_Inventory_Slot_Count = c_Belt_Inventory_Width * c_Belt_Inventory_Height;
	const DWORD c_Belt_Inventory_Slot_End = c_Belt_Inventory_Slot_Start + c_Belt_Inventory_Slot_Count;

	const DWORD c_Inventory_Count	= c_Belt_Inventory_Slot_End;
#else
	const DWORD c_Inventory_Count	= c_DragonSoul_Equip_End;
#endif

const DWORD c_DragonSoul_Inventory_Start = 0;
const DWORD c_DragonSoul_Inventory_Box_Size = 32;
const DWORD c_DragonSoul_Inventory_Count = CItemData::DS_SLOT_NUM_TYPES * DRAGON_SOUL_GRADE_MAX * c_DragonSoul_Inventory_Box_Size;
const DWORD c_DragonSoul_Inventory_End = c_DragonSoul_Inventory_Start + c_DragonSoul_Inventory_Count;

#ifdef ENABLE_SPECIAL_STORAGE
const DWORD c_Special_Inventory_Page_Size = 5*9;
const DWORD c_Special_Inventory_Page_Count = 4;
const DWORD c_Special_ItemSlot_Count = c_Special_Inventory_Page_Size * c_Special_Inventory_Page_Count;
#endif

#if defined(__BL_OFFICIAL_LOOT_FILTER__)
enum ELootFilter
{
	WEAPON_SELECT_DATA_MAX = 13, // Changed from 5 to 13
	ARMOR_SELECT_DATA_MAX = 5,
	HEAD_SELECT_DATA_MAX = 5,
	COMMON_SELECT_DATA_MAX = 10,
	COSTUME_SELECT_DATA_MAX = 6,
	DS_SELECT_DATA_MAX = 2,
	UNIQUE_SELECT_DATA_MAX = 2,
	REFINE_SELECT_DATA_MAX = 3,
	POTION_SELECT_DATA_MAX = 4,  // Changed from 3 to 4
	FISH_MINING_SELECT_DATA_MAX = 3,
	MOUNT_PET_SELECT_DATA_MAX = 4,
	SKILL_BOOK_SELECT_DATA_MAX = 6,
	ETC_SELECT_DATA_MAX = 8,
	EVENT_SELECT_DATA_MAX = 0, 

	WEAPON_ON_OFF = 0,
	WEAPON_REFINE_MIN,
	WEAPON_REFINE_MAX,
	WEAPON_WEARING_LEVEL_MIN,
	WEAPON_WEARING_LEVEL_MAX,
	WEAPON_SELECT_DATA_JOB_WARRIOR,
	WEAPON_SELECT_DATA_JOB_SURA,
	WEAPON_SELECT_DATA_JOB_ASSASSIN,
	WEAPON_SELECT_DATA_JOB_SHAMAN,
	WEAPON_SELECT_DATA_JOB_LYCAN,
	WEAPON_SELECT_DATA_TYPE_SWORD,
	WEAPON_SELECT_DATA_TYPE_DAGGER,
	WEAPON_SELECT_DATA_TYPE_BOW,
	WEAPON_SELECT_DATA_TYPE_TWO_HANDED,
	WEAPON_SELECT_DATA_TYPE_BELL,
	WEAPON_SELECT_DATA_TYPE_FAN,
	WEAPON_SELECT_DATA_TYPE_ARROW,
	WEAPON_SELECT_DATA_TYPE_CLAW,
	ARMOR_ON_OFF,
	ARMOR_REFINE_MIN,
	ARMOR_REFINE_MAX,
	ARMOR_WEARING_LEVEL_MIN,
	ARMOR_WEARING_LEVEL_MAX,
	ARMOR_SELECT_DATA_JOB_WARRIOR,
	ARMOR_SELECT_DATA_JOB_SURA,
	ARMOR_SELECT_DATA_JOB_ASSASSIN,
	ARMOR_SELECT_DATA_JOB_SHAMAN,
	ARMOR_SELECT_DATA_JOB_LYCAN,

	HEAD_ON_OFF,
	HEAD_REFINE_MIN,
	HEAD_REFINE_MAX,
	HEAD_WEARING_LEVEL_MIN,
	HEAD_WEARING_LEVEL_MAX,
	HEAD_SELECT_DATA_JOB_WARRIOR,
	HEAD_SELECT_DATA_JOB_SURA,
	HEAD_SELECT_DATA_JOB_ASSASSIN,
	HEAD_SELECT_DATA_JOB_SHAMAN,
	HEAD_SELECT_DATA_JOB_LYCAN,

	COMMON_ON_OFF,
	COMMON_REFINE_MIN,
	COMMON_REFINE_MAX,
	COMMON_WEARING_LEVEL_MIN,
	COMMON_WEARING_LEVEL_MAX,
	COMMON_SELECT_DATA_SHOE,
	COMMON_SELECT_DATA_BELT,
	COMMON_SELECT_DATA_BRACELET,
	COMMON_SELECT_DATA_NECKLACE,
	COMMON_SELECT_DATA_EARRING,
	COMMON_SELECT_DATA_SHIELD,
	COMMON_SELECT_DATA_GLOVE,
	COMMON_SELECT_DATA_TALISMAN,
	COMMON_SELECT_DATA_FISHING_ROD,
	COMMON_SELECT_DATA_PICKAXE,

	COSTUME_ON_OFF,
	COSTUME_SELECT_DATA_WEAPON,
	COSTUME_SELECT_DATA_ARMOR,
	COSTUME_SELECT_DATA_HAIR,
	COSTUME_SELECT_DATA_ACCE,
	COSTUME_SELECT_DATA_AURA,
	COSTUME_SELECT_DATA_ETC,

	DS_ON_OFF,
	DS_SELECT_DATA_DS,
	DS_SELECT_DATA_ETC,

	UNIQUE_ON_OFF,
	UNIQUE_SELECT_DATA_ABILITY,
	UNIQUE_SELECT_DATA_ETC,

	REFINE_ON_OFF,
	REFINE_SELECT_DATA_MATERIAL,
	REFINE_SELECT_DATA_STONE,
	REFINE_SELECT_DATA_ETC,

	POTION_ON_OFF,
	POTION_SELECT_DATA_ABILITY,
	POTION_SELECT_DATA_HAIRDYE,
	POTION_SELECT_DATA_REGULAR,
	POTION_SELECT_DATA_ETC,

	FISH_MINING_ON_OFF,
	FISH_MINING_SELECT_DATA_FOOD,
	FISH_MINING_SELECT_DATA_STONE,
	FISH_MINING_SELECT_DATA_ETC,

	MOUNT_PET_ON_OFF,
	MOUNT_PET_SELECT_DATA_CHARGED_PET,
	MOUNT_PET_SELECT_DATA_MOUNT,
	MOUNT_PET_SELECT_DATA_FREE_PET,
	MOUNT_PET_SELECT_DATA_EGG,

	SKILL_BOOK_ON_OFF,
	SKILL_BOOK_SELECT_DATA_JOB_WARRIOR,
	SKILL_BOOK_SELECT_DATA_JOB_SURA,
	SKILL_BOOK_SELECT_DATA_JOB_ASSASSIN,
	SKILL_BOOK_SELECT_DATA_JOB_SHAMAN,
	SKILL_BOOK_SELECT_DATA_JOB_LYCAN,
	SKILL_BOOK_SELECT_DATA_JOB_PUBLIC,

	ETC_ON_OFF,
	ETC_SELECT_DATA_GIFTBOX,
	ETC_SELECT_DATA_MATRIMONY,
	ETC_SELECT_DATA_SEAL,
	ETC_SELECT_DATA_PARTY,
	ETC_SELECT_DATA_POLYMORPH,
	ETC_SELECT_DATA_RECIPE,
	ETC_SELECT_DATA_WEAPON_ARROW,
	ETC_SELECT_DATA_ETC,

	EVENT_ON_OFF,

	LOOT_FILTER_SETTINGS_MAX
};
#endif

enum ESlotType
{
	SLOT_TYPE_NONE,
	SLOT_TYPE_INVENTORY,
	SLOT_TYPE_SKILL,
	SLOT_TYPE_EMOTION,
	SLOT_TYPE_SHOP,
	SLOT_TYPE_EXCHANGE_OWNER,
	SLOT_TYPE_EXCHANGE_TARGET,
	SLOT_TYPE_QUICK_SLOT,
	SLOT_TYPE_SAFEBOX,
	SLOT_TYPE_PRIVATE_SHOP,
	SLOT_TYPE_MALL,
	SLOT_TYPE_DRAGON_SOUL_INVENTORY,
#ifdef ENABLE_FISH_EVENT
	SLOT_TYPE_FISH_EVENT,
#endif
#ifdef ENABLE_IKASHOP_RENEWAL
#ifdef EXTEND_IKASHOP_ULTIMATE
	SLOT_TYPE_OFFLINESHOP,
#endif
#endif
#ifdef ENABLE_SWITCHBOT
	SLOT_TYPE_SWITCHBOT,
#endif
#ifdef ENABLE_SPECIAL_STORAGE
	SLOT_TYPE_UPGRADE_INVENTORY,
	SLOT_TYPE_BOOK_INVENTORY,
	SLOT_TYPE_STONE_INVENTORY,
	SLOT_TYPE_CHEST_INVENTORY,
#endif
#ifdef ENABLE_AURA_SYSTEM
	SLOT_TYPE_AURA,
#endif
#if defined(__BL_TRANSMUTATION__)
	SLOT_TYPE_CHANGE_LOOK,
#endif
	SLOT_TYPE_AUTOHUNT_SKILL,
	SLOT_TYPE_AUTOHUNT_ITEM,
	SLOT_TYPE_MAX,
};

#ifdef ENABLE_AURA_SYSTEM
const BYTE c_AuraMaxLevel = 250;

enum EAuraRefineInfoSlot
{
	AURA_REFINE_INFO_SLOT_CURRENT,
	AURA_REFINE_INFO_SLOT_NEXT,
	AURA_REFINE_INFO_SLOT_EVOLVED,
	AURA_REFINE_INFO_SLOT_MAX
};

enum EAuraWindowType
{
	AURA_WINDOW_TYPE_ABSORB,
	AURA_WINDOW_TYPE_GROWTH,
	AURA_WINDOW_TYPE_EVOLVE,
	AURA_WINDOW_TYPE_MAX,
};

enum EAuraSlotType
{
	AURA_SLOT_MAIN,
	AURA_SLOT_SUB,
	AURA_SLOT_RESULT,
	AURA_SLOT_MAX
};

enum EAuraRefineInfoType
{
	AURA_REFINE_INFO_STEP,
	AURA_REFINE_INFO_LEVEL_MIN,
	AURA_REFINE_INFO_LEVEL_MAX,
	AURA_REFINE_INFO_NEED_EXP,
	AURA_REFINE_INFO_MATERIAL_VNUM,
	AURA_REFINE_INFO_MATERIAL_COUNT,
	AURA_REFINE_INFO_NEED_GOLD,
	AURA_REFINE_INFO_EVOLVE_PCT,
	AURA_REFINE_INFO_MAX
};
#endif

enum EWindows
{
	RESERVED_WINDOW,
	INVENTORY,
	EQUIPMENT,
	SAFEBOX,
	MALL,
	DRAGON_SOUL_INVENTORY,
	BELT_INVENTORY,
	GROUND,
#ifdef ENABLE_IKASHOP_RENEWAL
	IKASHOP_OFFLINESHOP,
	IKASHOP_SAFEBOX,
	IKASHOP_AUCTION,
#endif
#ifdef ENABLE_SWITCHBOT
	SWITCHBOT,
#endif
#ifdef ENABLE_SPECIAL_STORAGE
	UPGRADE_INVENTORY,
	BOOK_INVENTORY,
	STONE_INVENTORY,
	CHEST_INVENTORY,
#endif
#ifdef ENABLE_AURA_SYSTEM
	AURA_REFINE,
#endif
	WINDOW_TYPE_MAX,
};

#ifdef ENABLE_FISH_EVENT
enum EFishEventInfo
{
	FISH_EVENT_SHAPE_NONE,
	FISH_EVENT_SHAPE_1,
	FISH_EVENT_SHAPE_2,			
	FISH_EVENT_SHAPE_3,			
	FISH_EVENT_SHAPE_4,			
	FISH_EVENT_SHAPE_5,			
	FISH_EVENT_SHAPE_6,			
	FISH_EVENT_SHAPE_7,
	FISH_EVENT_SHAPE_MAX_NUM,
};
#endif

enum EDSInventoryMaxNum
{
	DS_INVENTORY_MAX_NUM = c_DragonSoul_Inventory_Count,
	DS_REFINE_WINDOW_MAX_NUM = 15,
};

#if defined(__BL_MOVE_COSTUME_ATTR__)
enum ECombSlotType
{
	COMB_WND_SLOT_MEDIUM,
	COMB_WND_SLOT_BASE,
	COMB_WND_SLOT_MATERIAL,
	COMB_WND_SLOT_RESULT,

	COMB_WND_SLOT_MAX
};
#endif

#if defined(__BL_TRANSMUTATION__)
enum class ETRANSMUTATIONTYPE : BYTE
{
	TRANSMUTATION_TYPE_MOUNT,
	TRANSMUTATION_TYPE_ITEM
};

enum class ETRANSMUTATIONSLOTTYPE : size_t
{
	TRANSMUTATION_SLOT_LEFT,
	TRANSMUTATION_SLOT_RIGHT,

	TRANSMUTATION_SLOT_MAX
};

enum class ETRANSMUTATIONSETTINGS : DWORD
{
	TRANSMUTATION_ITEM_PRICE = 50000000, // 50M
	TRANSMUTATION_MOUNT_PRICE = 30000000, // 30M
	TRANSMUTATION_TICKET_1 = 72326,
	TRANSMUTATION_TICKET_2 = 72341,
	TRANSMUTATION_CLEAR_SCROLL = 72325,
};
#endif

#if defined(ENABLE_DS_CHANGE_ATTR)
enum EDSRefineType
{
	DS_REFINE_TYPE_GRADE,
	DS_REFINE_TYPE_STEP,
	DS_REFINE_TYPE_STRENGTH,
	DS_REFINE_TYPE_CHANGE_ATTR,
	DS_REFINE_TYPE_MAX,
};
#endif

#pragma pack (push, 1)
#define WORD_MAX 0xffff

#ifdef ENABLE_SWITCHBOT
enum ESwitchbotValues
{
	SWITCHBOT_SLOT_COUNT = 5,
	SWITCHBOT_ALTERNATIVE_COUNT = 2,
	MAX_NORM_ATTR_NUM = 5,
};

enum EAttributeSet
{
	ATTRIBUTE_SET_WEAPON,
	ATTRIBUTE_SET_BODY,
	ATTRIBUTE_SET_WRIST,
	ATTRIBUTE_SET_FOOTS,
	ATTRIBUTE_SET_NECK,
	ATTRIBUTE_SET_HEAD,
	ATTRIBUTE_SET_SHIELD,
	ATTRIBUTE_SET_EAR,
	ATTRIBUTE_SET_MAX_NUM,
};

#endif

typedef struct SItemPos
{
	BYTE window_type;
	WORD cell;
    SItemPos ()
    {
		window_type =     INVENTORY;
		cell = WORD_MAX;
    }
	SItemPos (BYTE _window_type, WORD _cell)
    {
        window_type = _window_type;
        cell = _cell;
    }

  //  int operator=(const int _cell)
  //  {
		//window_type = INVENTORY;
  //      cell = _cell;
  //      return cell;
  //  }
	bool IsValidCell()
	{
		switch (window_type)
		{
		case INVENTORY:
			return cell < c_Inventory_Count;
			break;
		case EQUIPMENT:
			return cell < c_DragonSoul_Equip_End;
			break;
		case DRAGON_SOUL_INVENTORY:
			return cell < (DS_INVENTORY_MAX_NUM);
			break;
#ifdef ENABLE_SWITCHBOT
		case SWITCHBOT:
			return cell < SWITCHBOT_SLOT_COUNT;
			break;
#endif
#ifdef ENABLE_SPECIAL_STORAGE
		case UPGRADE_INVENTORY:
			return cell < c_Special_ItemSlot_Count;
			break;
		case BOOK_INVENTORY:
			return cell < c_Special_ItemSlot_Count;
			break;
		case STONE_INVENTORY:
			return cell < c_Special_ItemSlot_Count;
			break;
		case CHEST_INVENTORY:
			return cell < c_Special_ItemSlot_Count;
			break;
#endif
		default:
			return false;
		}
	}
	bool IsEquipCell()
	{
		switch (window_type)
		{
		case INVENTORY:
		case EQUIPMENT:
			return (c_Equipment_Start + c_Wear_Max > cell) && (c_Equipment_Start <= cell);
			break;

		case BELT_INVENTORY:
		case DRAGON_SOUL_INVENTORY:
#ifdef ENABLE_SPECIAL_STORAGE
		case UPGRADE_INVENTORY:
		case BOOK_INVENTORY:
		case STONE_INVENTORY:
		case CHEST_INVENTORY:
#endif
			return false;
			break;

		default:
			return false;
		}
	}

#ifdef ENABLE_NEW_EQUIPMENT_SYSTEM
	bool IsBeltInventoryCell()
	{
		bool bResult = c_Belt_Inventory_Slot_Start <= cell && c_Belt_Inventory_Slot_End > cell;
		return bResult;
	}
#endif

	bool IsNPOS()
	{
		return (window_type == RESERVED_WINDOW && cell == WORD_MAX);
	}

	bool operator==(const struct SItemPos& rhs) const
	{
		return (window_type == rhs.window_type) && (cell == rhs.cell);
	}

	bool operator!=(const struct SItemPos& rhs) const
	{
		return (window_type != rhs.window_type) || (cell != rhs.cell);
	}

	bool operator<(const struct SItemPos& rhs) const
	{
		return (window_type < rhs.window_type) || ((window_type == rhs.window_type) && (cell < rhs.cell));
	}
} TItemPos;
const TItemPos NPOS(RESERVED_WINDOW, WORD_MAX);
#pragma pack(pop)

const DWORD c_QuickBar_Line_Count = 3;
const DWORD c_QuickBar_Slot_Count = 12;

const float c_Idle_WaitTime = 5.0f;

const int c_Monster_Race_Start_Number = 6;
const int c_Monster_Model_Start_Number = 20001;

const float c_fAttack_Delay_Time = 0.2f;
const float c_fHit_Delay_Time = 0.1f;
const float c_fCrash_Wave_Time = 0.2f;
const float c_fCrash_Wave_Distance = 3.0f;

const float c_fHeight_Step_Distance = 50.0f;

enum
{
	DISTANCE_TYPE_FOUR_WAY,
	DISTANCE_TYPE_EIGHT_WAY,
	DISTANCE_TYPE_ONE_WAY,
	DISTANCE_TYPE_MAX_NUM,
};

const float c_fMagic_Script_Version = 1.0f;
const float c_fSkill_Script_Version = 1.0f;
const float c_fMagicSoundInformation_Version = 1.0f;
const float c_fBattleCommand_Script_Version = 1.0f;
const float c_fEmotionCommand_Script_Version = 1.0f;
const float c_fActive_Script_Version = 1.0f;
const float c_fPassive_Script_Version = 1.0f;

// Used by PushMove
const float c_fWalkDistance = 175.0f;
const float c_fRunDistance = 310.0f;

#define FILE_MAX_LEN 128
#ifdef ELEMENT_SPELL_WORLDARD
enum
{
   MAX_ELEMENTS_SPELL = 3,
};
#endif
#ifdef ENABLE_NEW_SOCKET_VALUE
enum
{
	ITEM_MIN_MAX_VALUE_MAX_NUM = 6,
};
#endif
enum
{
	ITEM_SOCKET_SLOT_MAX_NUM = 3,
	// refactored attribute slot begin
	ITEM_ATTRIBUTE_SLOT_NORM_NUM	= 5,
	ITEM_ATTRIBUTE_SLOT_RARE_NUM	= 2,

	ITEM_ATTRIBUTE_SLOT_NORM_START	= 0,
	ITEM_ATTRIBUTE_SLOT_NORM_END	= ITEM_ATTRIBUTE_SLOT_NORM_START + ITEM_ATTRIBUTE_SLOT_NORM_NUM,

	ITEM_ATTRIBUTE_SLOT_RARE_START	= ITEM_ATTRIBUTE_SLOT_NORM_END,
	ITEM_ATTRIBUTE_SLOT_RARE_END	= ITEM_ATTRIBUTE_SLOT_RARE_START + ITEM_ATTRIBUTE_SLOT_RARE_NUM,

#ifdef ENABLE_RANDOM_STATS_SYSTEM
	ITEM_ATTRIBUTE_SLOT_MAX_NUM		= 10,
#else
	ITEM_ATTRIBUTE_SLOT_MAX_NUM		= ITEM_ATTRIBUTE_SLOT_RARE_END, // 7
#endif
	// refactored attribute slot end
};

#pragma pack(push)
#pragma pack(1)

typedef struct SQuickSlot
{
	BYTE Type;
	BYTE Position;
} TQuickSlot;

typedef struct TPlayerItemAttribute
{
    BYTE        bType;
    short       sValue;
} TPlayerItemAttribute;

typedef struct packet_item
{
    DWORD       vnum;
    BYTE        count;
	DWORD		flags;
	DWORD		anti_flags;
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
} TItemData;

typedef struct packet_shop_item
{
    DWORD       vnum;
    DWORD       price;
#ifdef ENABLE_CHEQUE_SYSTEM
	DWORD		cheque;
#endif
    BYTE        count;
#ifdef ENABLE_MULTISHOP
	DWORD		wPriceVnum;
	DWORD		wPrice;
#endif
	BYTE		display_pos;
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
#ifdef ENABLE_12ZI
	DWORD		getLimitedCount;
	DWORD		getLimitedPurchaseCount;
#endif
} TShopItemData;

#ifdef ENABLE_GEM_SYSTEM
typedef struct SGemShopItem
{
	BYTE	slotIndex;
	BYTE	status;
	
	DWORD	dwVnum;
	BYTE	bCount;
	DWORD	dwPrice;
} TGemShopItem;
#endif

#pragma pack(pop)

inline float GetSqrtDistance(int ix1, int iy1, int ix2, int iy2) // By sqrt
{
	float dx, dy;

	dx = float(ix1 - ix2);
	dy = float(iy1 - iy2);

	return sqrtf(dx*dx + dy*dy);
}

// DEFAULT_FONT
void DefaultFont_Startup();
void DefaultFont_Cleanup();
void DefaultFont_SetName(const char * c_szFontName);
CResource* DefaultFont_GetResource();
CResource* DefaultItalicFont_GetResource();
// END_OF_DEFAULT_FONT

void SetGuildSymbolPath(const char * c_szPathName);
const char * GetGuildSymbolFileName(DWORD dwGuildID);
BYTE SlotTypeToInvenType(BYTE bSlotType);
#ifdef ENABLE_AURA_SYSTEM
int* GetAuraRefineInfo(BYTE bLevel);
#endif
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
