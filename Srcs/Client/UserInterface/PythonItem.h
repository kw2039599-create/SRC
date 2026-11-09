#pragma once

#include "../EterGrnLib/ThingInstance.h"
#ifdef ENABLE_DS_SET
#include "Packet.h"
#endif
#if defined(__BL_OFFICIAL_LOOT_FILTER__)
#include <unordered_set>
#endif

class CItemData;

class CPythonItem : public CSingleton<CPythonItem>
{
	public:
		enum
		{
			INVALID_ID = 0xffffffff,
		};

		enum
		{
			VNUM_MONEY = 1,
		};

		enum
		{
			USESOUND_NONE,
			USESOUND_DEFAULT,
			USESOUND_ARMOR,
			USESOUND_WEAPON,
			USESOUND_BOW,
			USESOUND_ACCESSORY,
			USESOUND_POTION,
			USESOUND_PORTAL,
			USESOUND_NUM,
		};

		enum
		{
			DROPSOUND_DEFAULT,
			DROPSOUND_ARMOR,
			DROPSOUND_WEAPON,
			DROPSOUND_BOW,
			DROPSOUND_ACCESSORY,
			DROPSOUND_NUM
		};

		enum EDropItemEffects
		{
			DROP_ITEM_EFFECT_NORMAL,
			DROP_ITEM_EFFECT_RARE,
			DROP_ITEM_EFFECT_EPIC,
			DROP_ITEM_EFFECT_LEGENARY,
			DROP_ITEM_EFFECT_NUM,
		};

		typedef struct SGroundItemInstance
		{
			DWORD					dwVirtualNumber;
			D3DXVECTOR3				v3EndPosition;

			D3DXVECTOR3				v3RotationAxis;
			D3DXQUATERNION			qEnd;
			D3DXVECTOR3				v3Center;
			CGraphicThingInstance	ThingInstance;
			DWORD					dwStartTime;
			DWORD					dwEndTime;

			DWORD					eDropSoundType;

			bool					bAnimEnded;
			bool Update();
			void Clear();

			DWORD					dwEffectInstanceIndex;
			std::string				stOwnership;
			DWORD					dwLastLootTime;

			static void	__PlayDropSound(DWORD eItemType, const D3DXVECTOR3& c_rv3Pos);
			static std::string		ms_astDropSoundFileName[DROPSOUND_NUM];

			SGroundItemInstance() {}
			virtual ~SGroundItemInstance() {}
		} TGroundItemInstance;

#ifdef ENABLE_CSHIELD
		typedef std::map<DWORD, EncryptedPtr<TGroundItemInstance>> TGroundItemInstanceMap;
#else
		typedef std::map<DWORD, TGroundItemInstance*> TGroundItemInstanceMap;
#endif
#ifdef ENABLE_DS_SET
		typedef struct SDSTable
		{
			float	fWeight;
			int		iApplyCount;
			int		iBasicApplyValue[255];
			int		iAditionalApplyValue[255];
		} TDSTable;
		
		typedef std::map<int, TDSTable>	TDSTableMap;
#endif

	public:
		CPythonItem(void);
		virtual ~CPythonItem(void);

		// Initialize
		void	Destroy();
		void	Create();

		void	PlayUseSound(DWORD dwItemID);
#if defined(__BL_PICK_FILTER__)
		std::vector<DWORD> GetCloseItemVector(const std::string& myName, const TPixelPosition& c_rPixelPosition) const;
#endif
		void	PlayDropSound(DWORD dwItemID);
		void	PlayUsePotionSound();

		void	SetUseSoundFileName(DWORD eItemType, const std::string& c_rstFileName);
		void	SetDropSoundFileName(DWORD eItemType, const std::string& c_rstFileName);

		void	GetInfo(std::string* pstInfo);

		void	DeleteAllItems();

		void	Render();
		void	Update(const POINT& c_rkPtMouse);
#if defined(__BL_TRANSMUTATION__)
		bool	CanAddChangeLookItem(const CItemData* item, const CItemData* other_item) const;
		bool	CanAddChangeLookFreeItem(const DWORD dwVnum) const;
		bool	IsChangeLookClearScrollItem(const DWORD dwVnum) const;
#endif

		void	CreateItem(DWORD dwVirtualID, DWORD dwVirtualNumber, float x, float y, float z, bool bDrop=true);
		void	DeleteItem(DWORD dwVirtualID);
		void	SetOwnership(DWORD dwVID, const char * c_pszName);
		bool	GetOwnership(DWORD dwVID, const char ** c_pszName);

		BOOL	GetGroundItemPosition(DWORD dwVirtualID, TPixelPosition * pPosition);

		bool	GetPickedItemID(DWORD* pdwPickedItemID);

		bool	GetCloseItem(const TPixelPosition & c_rPixelPosition, DWORD* pdwItemID, DWORD dwDistance=300);
		void	AutoLootPet(const TPixelPosition& c_rPixelPosition);
		std::vector<DWORD> GetCloseItemVector(const TPixelPosition& c_rPixelPosition, DWORD dwDistance);
		bool	GetCloseMoney(const TPixelPosition & c_rPixelPosition, DWORD* dwItemID, DWORD dwDistance=300);

		DWORD	GetVirtualNumberOfGroundItem(DWORD dwVID);

		void	BuildNoGradeNameData(int iType);
		DWORD	GetNoGradeNameDataCount();
		CItemData * GetNoGradeNameDataPtr(DWORD dwIndex);
#if defined(__BL_OFFICIAL_LOOT_FILTER__)
		bool	IsLootFilteredItem(DWORD dwVID) const;
		void	InsertLootFilteredItem(DWORD dwVID);
		void	EraseLootFilteredItem(DWORD dwVID);
		void	ClearLootFilteredItems();
#endif
	protected:
		void	__RegisterEffect(int iIndex, const char* szFile);
		int		__AttachEffect(DWORD dwVnum, BYTE byType, BYTE bySubType);
		DWORD	__Pick(const POINT& c_rkPtMouse);

		DWORD	__GetUseSoundType(const CItemData& c_rkItemData);
		DWORD	__GetDropSoundType(const CItemData& c_rkItemData);

	protected:
		TGroundItemInstanceMap				m_GroundItemInstanceMap;
		CDynamicPool<TGroundItemInstance>	m_GroundItemInstancePool;

		DWORD m_dwDropItemEffectID[DROP_ITEM_EFFECT_NUM];
		DWORD m_dwPickedItemID;

		int m_nMouseX;
		int m_nMouseY;

		std::string m_astUseSoundFileName[USESOUND_NUM];

#ifdef ENABLE_CSHIELD
		std::vector<EncryptedPtr<CItemData>> m_NoGradeNameItemData;
#else
		std::vector<CItemData*> m_NoGradeNameItemData;
#endif
#if defined(__BL_OFFICIAL_LOOT_FILTER__)
		std::unordered_set<DWORD> m_LootFilteredItemsSet;
#endif
#ifdef ENABLE_DS_SET
		TDSTableMap	m_DSTableMap;
	public:
		bool	SetDSTable(TPacketDSTable p);
		float	GetDSSetWeight(int iDSType, int iDSGrade);
		int		GetDSBasicApplyCount(int iDSType, int iDSGrade);
		int		GetDSBasicApplyValue(int iDSType, int iDSApplyType);
		int		GetDSAdditionalApplyValue(int iDSType, int iDSApplyType);
#endif
};
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
