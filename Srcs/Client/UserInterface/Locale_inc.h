#pragma once

//MOBILE
#if defined(__ANDROID__) || defined(__APPLE__)
#ifndef ENABLE_OPENGL
#define ENABLE_OPENGL
#endif
#ifndef ENABLE_MOBILE_CONTROLS
#define ENABLE_MOBILE_CONTROLS
#endif
#endif
//
//////////////////////////////////////////////////////////////////////////
// ### Default Ymir Macros ###
#define ENABLE_HIDE_COSTUME_SYSTEM
#define LOCALE_SERVICE_EUROPE
#define ENABLE_COSTUME_SYSTEM
#define ENABLE_ENERGY_SYSTEM
#define ENABLE_DRAGON_SOUL_SYSTEM
#define ENABLE_NEW_EQUIPMENT_SYSTEM
#define __BL_ENVIRONMENT_EFFECT_OPTION__
//#define __BL_OFFICIAL_LOOT_FILTER__
#define __BL_PICK_FILTER__
#define ENABLE_OFFLINESHOP_SYSTEM
#ifdef ENABLE_OFFLINESHOP_SYSTEM
	#define ENABLE_SHOP_SEARCH_SYSTEM
#endif
//#define ENABLE_PREMIUM_LOOT_FILTER
// ### Default Ymir Macros ###
#define ENABLE_EVENT_BANNER_FLAG
#define __BL_TRANSMUTATION__
#define ENABLE_CHANGELOOK_SYSTEM
#define /* @author: Owsap */ ENABLE_GAME_OPTION_ESCAPE // Game Option (Escape)

#define NEW_PET_SYSTEM
#define ENABLE_PET_ATTR_DETERMINE					//Tr Yeni Pet Güncellemesi
#define ENABLE_MELEY_LAIR_DUNGEON
#ifdef ENABLE_MELEY_LAIR_DUNGEON
	#define MELEY_LAIR_DUNGEON_STATUE 6118
#endif

#define ENABLE_SKILLBOOK_COMB_SYSTEM // Skill book combination
#define ENABLE_CHATTING_WINDOW_RENEWAL

#define __BL_MOVE_COSTUME_ATTR__			// Move Costume Attributes
//////////////////////////////////////////////////////////////////////////
//#define ENABLE_NEW_GAMEOPTION
#define __BL_FOG_FIX__
#define ENABLE_OKEY_EVENT_SYSTEM

#define ENABLE_EVENT_MANAGER

#define ENABLE_FOV_OPTION
#define ENABLE_SORT_INVEN
#define ENABLE_DS_SET
#define WJ_ENABLE_TRADABLE_ICON

#define __BL_MAILBOX__
#define ENABLE_ITEMSHOP
//#define ENABLE_VIEW_TARGET_PLAYER_HP
//#define ENABLE_VIEW_TARGET_DECIMAL_HP

#define __ENABLE_SHAMAN_ATTACK_FIX__
#define /* @author: Owsap */ ENABLE_DS_CHANGE_ATTR // Dragon Soul Change Attribute
#define __AUTO_CHAT__
//////////////////////////////////////////////////////////////////////////
// ### New From LocaleInc ###
#define ENABLE_PACK_GET_CHECK
//#define ENABLE_CANSEEHIDDENTHING_FOR_GM
#define ENABLE_PROTOSTRUCT_AUTODETECT
#define ENABLE_PLAYER_PER_ACCOUNT5
#define ENABLE_LEVEL_IN_TRADE
#define ENABLE_DICE_SYSTEM
#define ENABLE_EXTEND_INVEN_SYSTEM
#define ENABLE_LVL115_ARMOR_EFFECT
#define VERSION_162_ENABLED
#define ENABLE_SLOT_WINDOW_EX
#define ENABLE_TEXT_LEVEL_REFRESH
#define ENABLE_USE_COSTUME_ATTR
#define ENABLE_DISCORD_RPC
#define ENABLE_PET_SYSTEM_EX
#define ENABLE_LOCALE_EX
//#define ENABLE_NO_DSS_QUALIFICATION
//#define ENABLE_NO_SELL_PRICE_DIVIDED_BY_5
#define ENABLE_PENDANT_SYSTEM
#define ENABLE_GLOVE_SYSTEM
#define ENABLE_MOVE_CHANNEL
#define ENABLE_QUIVER_SYSTEM
#define ENABLE_RACE_HEIGHT
#define ENABLE_ELEMENTAL_TARGET
#define ENABLE_INGAME_CONSOLE
#define ENABLE_4TH_AFF_SKILL_DESC
#define ENABLE_LOCALE_COMMON
#define ENABLE_GUILD_TOKEN_AUTH
#define ENABLE_DS_GRADE_MYTH
#define ENABLE_CONQUEROR_UI

#define ENABLE_NEW_EVENT_STRUCT
#ifdef ENABLE_NEW_EVENT_STRUCT
#define USE_NEW_EVENT_TEXT_AUTO_Y
#endif

#define WJ_SHOW_MOB_INFO
#ifdef WJ_SHOW_MOB_INFO
#define ENABLE_SHOW_MOBAIFLAG
#define ENABLE_SHOW_MOBLEVEL
#define WJ_SHOW_MOB_INFO_EX
#endif
// ### New From LocaleInc ###
//////////////////////////////////////////////////////////////////////////
//#define ENABLE_CSHIELD
#if defined(ENABLE_CSHIELD) && !defined(_DEBUG)
	#ifdef ENABLE_CSHIELD
		#define SERVERS {"159.195.73.182:30015"} // you can add multiple cores to be more failsave (auth is not recommended)
		#define ENABLE_CAPTCHA
		#define HIDE_CAPTCHA // hides client window when captcha appears
		#define ENABLE_REGISTRY_HWID_BAN // sets a registry key when the user is HWID banned (can not be undone via webpanel)
		#define HIDE_STRINGS // hide strings in the client
	#endif
//#else
//	#define ENABLE_CSHIELD_BYPASS
#endif
//////////////////////////////////////////////////////////////////////////
// ### From GameLib ###
#define ENABLE_WOLFMAN_CHARACTER
#ifdef ENABLE_WOLFMAN_CHARACTER
//#define DISABLE_WOLFMAN_ON_CREATE
#endif
#define ENABLE_MAGIC_REDUCTION_SYSTEM

#define ENABLE_MOUNT_COSTUME_SYSTEM
#define ENABLE_WEAPON_COSTUME_SYSTEM
// ### From GameLib ###
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// ### New System Defines - Extended Version ###

// if is define ENABLE_ACCE_COSTUME_SYSTEM the players can use shoulder sash
#define ENABLE_ACCE_COSTUME_SYSTEM
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
// #define USE_ACCE_ABSORB_WITH_NO_NEGATIVE_BONUS
#endif

// if you want use SetMouseWheelScrollEvent or you want use mouse wheel to move the scrollbar
#define ENABLE_MOUSEWHEEL_EVENT

//if you want to see highlighted a new item when dropped or when exchanged

#define ENABLE_HIGHLIGHT_NEW_ITEM

// it shows emojis in the textlines
#define ENABLE_EMOJI_SYSTEM

// effects while hidden won't show up
#define __ENABLE_STEALTH_FIX__

// circle dots in minimap instead of squares
//#define ENABLE_MINIMAP_WHITEMARK_CIRCLE
#define ENABLE_MINIMAP_TELEPORT_CLICK // click on minimap as gm to warp directly

// enable the won system as a currency
#define ENABLE_CHEQUE_SYSTEM
#ifdef ENABLE_CHEQUE_SYSTEM
#define DISABLE_CHEQUE_DROP
#define ENABLE_WON_EXCHANGE_WINDOW
#endif

// for debug: print received packets
// #define ENABLE_PRINT_RECV_PACKET_DEBUG

////////////////////////////////////////////////////////////////////////////
/* JTX | MR WOLF  --->  WORK START
⠀⠀⠀⠀⠀⣀⣠⣤⣤⣤⣤⣄⣀⠀⠀⠀⠀⠀
⠀⠀⢀⣴⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣦⡀⠀⠀                   _  __
⠀⣴⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠟⢿⣿⣷⡀⠀		            | |/ _|
⣸⣿⣿⣿⣿⣿⣿⣿⣿⣿⠟⠁⠀⣴⢿⣿⣧⠀	  __      _____ | | |_
⣿⣿⣿⣿⣿⡿⠛⣩⠍⠀⠀⠀⠐⠉⢠⣿⣿⡇    \ \ /\ / / _ \| |  _|
⣿⡿⠛⠋⠉⠀⠀⠀⠀⠀⠀⠀⠀⢠⣿⣿⣿⣿     \ V  V / (_) | | |
⢹⣿⣤⠄⠀⠀⠀⠀⠀⠀⠀⠀⢠⣿⣿⣿⣿⡏      \_/\_/ \___/|_|_|
⠀⠻⡏⠀⠀⠀⠀⠀⠀⠀⠀⠀⢿⣿⣿⣿⠟⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢻⠟⠁⠀⠀⠀
*/

#ifndef ENABLE_OPENGL
	#define DIRECTX9
	#define JTX_GPU_LEAF_PLACEMENT
	#define JTX_GPU_TREE_WIND
	#define JTX_TREE_DYNAMIC_LIGHT
#endif

#ifdef DIRECTX9
#define D3DLIGHT8					D3DLIGHT9
#define D3DMATERIAL8				D3DMATERIAL9
#define IDirect3DVertexBuffer8		IDirect3DVertexBuffer9
#define LPDIRECT3DTEXTURE8			LPDIRECT3DTEXTURE9
#define LPDIRECT3DSURFACE8			LPDIRECT3DSURFACE9
#define D3DVIEWPORT8				D3DVIEWPORT9
#define LPDIRECT3DDEVICE8			LPDIRECT3DDEVICE9
#define LPDIRECT3DVERTEXBUFFER8		LPDIRECT3DVERTEXBUFFER9
#define LPDIRECT3DINDEXBUFFER8		LPDIRECT3DINDEXBUFFER9
#define DXLOCKTYPE					(void**)
#define D3DVERTEXELEMENT8			D3DVERTEXELEMENT9
#define LPDIRECT3DBASETEXTURE8		LPDIRECT3DBASETEXTURE9
#define LPDIRECT3DPIXELSHADER8		LPDIRECT3DPIXELSHADER9
#define D3DADAPTER_IDENTIFIER8		D3DADAPTER_IDENTIFIER9
#define IDirect3D8					IDirect3D9
#define IDirect3DDevice8			IDirect3DDevice9
#define D3DCAPS8					D3DCAPS9
#define LPDIRECT3D8					LPDIRECT3D9
#define D3DCAPS2_CANRENDERWINDOWED	DDCAPS2_CANRENDERWINDOWED
#define IDirect3DTexture8			IDirect3DTexture9
#define Direct3DCreate8				Direct3DCreate9
#define IDirect3DSurface8			IDirect3DSurface9
#else
#define DXLOCKTYPE					(BYTE**)
#endif

#define ENABLE_IKASHOP_RENEWAL
#define ENABLE_IKASHOP_ENTITIES
#define EXTEND_IKASHOP_PRO
#define EXTEND_IKASHOP_ULTIMATE
#define ENABLE_LARGE_DYNAMIC_PACKETS
#define ENABLE_IKASHOP_RENEWAL_TITEL
// JTX | MR WOLF  --->  WORK END
///////////////////////////////////////////////////////////////////////////

// ### New System Defines - Extended Version ###
//////////////////////////////////////////////////////////////////////////
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
#define ENABLE_VOICE_CHAT
#define ENABLE_7AND8TH_SKILLS
#define ENABLE_12ZI
#define /* @author: Owsap */ ENABLE_SHIP_DEFENSE // Ship Defense (Hydra Dungeon)
#define ENABLE_MINI_GAME
#define ENABLE_FISH_EVENT
#define ENABLE_MULTISHOP
#define ENABLE_FISHING_TIMER
//#define __BL_KILL_BAR__
#define ENABLE_TWEENY_LIBRARY
#define ENABLE_MESSENGER_BLOCK
#define ENABLE_ANTI_EXP
#define __BACK_DUNGEON__
#define __BL_67_ATTR__
//#define BONUS_PCT
#define ENABLE_CONQUEROR_LEVEL
#define ENABLE_QUEEN_NETHIS
#define ENABLE_ATTR_6TH_7TH_EXTEND
#define __BL_PARTY_POSITION__
#define ELEMENT_SPELL_WORLDARD
#define ENABLE_AFK_MODE_SYSTEM
#define __BL_CLIP_MASK__
#define __BL_MOUSE_WHEEL_TOP_WINDOW__
#define /* @author: Owsap */ ENABLE_QUEST_RENEWAL // Quest Page Renewal
#define ENABLE_GEM_SYSTEM
#define ENABLE_SOULBIND_SYSTEM
#define ENABLE_SHOP_NAMES
#define TARGET_DAMAGE
#define BOT_KILLER
#define ENABLE_RANDOM_STATS_SYSTEM
#define ENABLE_CUBE_RENEWAL_WORLDARD
#define ENABLE_NEW_SOCKET_VALUE

//#define ATTACK_GUARD
#ifndef ENABLE_OPENGL
#define ANTI_SPLASH
#endif
#define TIMER_SYNC
#define ENABLE_RENDER_TARGET
//#define __BL_OFFICIAL_LOOT_FILTER__
//#if defined(__BL_OFFICIAL_LOOT_FILTER__)
//#	define ENABLE_PREMIUM_LOOT_FILTER // Enable Premium Usage of the Loot Filter System
//#endif

#define ENABLE_NPC_LOCATION_HELPER							
#define ENABLE_SWITCHBOT
//#if defined(ENABLE_GUILD_DRAGONLAIR_SYSTEM)
	//#define ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM			
//#endif

#define ENABLE_SPECIAL_STORAGE
#define ENABLE_MINI_GAME_CATCH_KING
#define __MINI_GAME_CATCH_KING__
#define ENABLE_MINI_GAME_YUTNORI

#define ENABLE_STANDING_MOUNT
#ifdef ENABLE_STANDING_MOUNT
	// Returns true if the vnum matches any of your custom standing mounts
	#define IS_SURFBOARD(vnum) ((vnum) == 40002 || (vnum) == 40003 || (vnum) == 40004 || (vnum) == 40005 || (vnum) == 40006 || (vnum) == 40007)
#endif

#define ENABLE_TAROT
#define __BL_CHEST_DROP_INFO__
#define ENABLE_CHEST_OPEN_RENEWAL

#define FIX_MOVEMENT

#define ENABLE_PARTY_MATCH

#define ENABLE_AURA_SYSTEM

#define WJ_COMBAT_ZONE
#define WJ_COMBAT_ZONE_HIDE_INFO_USER // Disable if you want for people can see names of players, align, level in map, target, minimap name etc.
#define ENABLE_MINI_WINDOW_SYSTEM // Mini Window & Always on Top System

#define ENABLE_MESSENGER_RENEWAL							// Resmi Gelişmiş Mesaj Sistemi
#if defined(ENABLE_MESSENGER_RENEWAL)
	#define ENABLE_COMMUNITY_GUILD_RENEWAL					// Community lonca sekmesi uye listesi / konum
#endif

#ifdef ENABLE_OPENGL
#include "../EterLib/GrpOpenGL.h"
#endif