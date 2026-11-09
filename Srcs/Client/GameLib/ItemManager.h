#pragma once

#include "ItemData.h"
#include "../UserInterface/Locale_inc.h"
#if defined(__BL_CHEST_DROP_INFO__)
#include <unordered_map>
#endif
#ifdef ENABLE_CSHIELD
#include "CShield.h"
#endif

class CItemManager : public CSingleton<CItemManager>
{
	public:
		enum EItemDescCol
		{
			ITEMDESC_COL_VNUM,
			ITEMDESC_COL_NAME,
			ITEMDESC_COL_DESC,
			ITEMDESC_COL_SUMM,
			ITEMDESC_COL_NUM,
		};

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		enum EItemScaleCol
		{
			ITEMSCALE_COL_VNUM,
			ITEMSCALE_COL_JOB,
			ITEMSCALE_COL_SEX,
			ITEMSCALE_COL_SCALE_X,
			ITEMSCALE_COL_SCALE_Y,
			ITEMSCALE_COL_SCALE_Z,
			ITEMSCALE_COL_PARTICLE_SCALE,
		};
#endif

	public:
//#ifdef ENABLE_CSHIELD
//		typedef std::map<DWORD, EncryptedPtr<CItemData>> TItemMap;
//#else
		typedef std::map<DWORD, CItemData*> TItemMap;
//#endif
		typedef std::map<std::string, CItemData*> TItemNameMap;

	public:
		CItemManager();
		virtual ~CItemManager();

		void			Destroy();

		BOOL			SelectItemData(DWORD dwIndex);
		CItemData *		GetSelectedItemDataPointer();
		std::map<DWORD, CItemData*>			GetItemsVector(){ return m_ItemMap; }
		BOOL			GetItemDataPointer(DWORD dwItemID, CItemData ** ppItemData);

		/////
		bool			LoadItemDesc(const char* c_szFileName);
		bool			LoadItemList(const char* c_szFileName);
		bool			LoadItemTable(const char* c_szFileName);
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		bool			LoadItemScale(const char* szItemScale);
#endif

		CItemData *		MakeItemData(DWORD dwIndex);
#if defined(__BL_CHEST_DROP_INFO__)
		struct SDropItemInfo
		{
			DWORD	dwDropVnum;
			int		iCount;
		};

		using							TChestDropItemInfoVec = std::vector<SDropItemInfo>;
		using							TChestDropItemInfoMap = std::unordered_map<DWORD, TChestDropItemInfoVec>;

		bool							LoadChestDropInfo(const char* c_szFileName);
		
		TChestDropItemInfoVec*			GetItemDropInfoVec(const DWORD dwVnum);
		TChestDropItemInfoVec*			GetBaseItemDropInfoVec(const DWORD dwVnum);
#endif
#ifdef ENABLE_IKASHOP_RENEWAL
		void			GetItemsNameMap(std::map<DWORD, std::string>& inMap);
#endif

	protected:
		TItemMap m_ItemMap;
#ifdef ENABLE_CSHIELD
		std::vector<EncryptedPtr<CItemData>> m_vec_ItemRange;
		EncryptedPtr<CItemData> m_pSelectedItemData;
#else
		std::vector<CItemData*> m_vec_ItemRange;
		CItemData* m_pSelectedItemData;
#endif
#if defined(__BL_CHEST_DROP_INFO__)
		TChestDropItemInfoMap m_ItemDropInfoMap;
		TChestDropItemInfoMap m_BaseItemDropInfoMap;
#endif
#ifdef ENABLE_AURA_SYSTEM
	public:
		enum EAuraScaleCol
		{
			AURA_SCALE_COL_VNUM,
			AURA_SCALE_COL_JOB,
			AURA_SCALE_COL_SEX,
			AURA_SCALE_COL_MESH_SCALE_X,
			AURA_SCALE_COL_MESH_SCALE_Y,
			AURA_SCALE_COL_MESH_SCALE_Z,
			AURA_SCALE_COL_PARTICLE_SCALE,
			AURA_SCALE_COL_NUM,
		};

		bool	LoadAuraScale(const char* c_szFileName);
#endif
};
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
