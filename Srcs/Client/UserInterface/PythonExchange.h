#pragma once

#include "Packet.h"

class CPythonExchange : public CSingleton<CPythonExchange>
{
	public:
		enum
		{
			EXCHANGE_ITEM_MAX_NUM = 12,
		};

		typedef struct trade
		{
			char					name[CHARACTER_NAME_MAX_LEN + 1];
#ifdef ENABLE_LEVEL_IN_TRADE
			DWORD					level;
#endif

			DWORD					item_vnum[EXCHANGE_ITEM_MAX_NUM];
			BYTE					item_count[EXCHANGE_ITEM_MAX_NUM];
			DWORD					item_metin[EXCHANGE_ITEM_MAX_NUM][ITEM_SOCKET_SLOT_MAX_NUM];
			TPlayerItemAttribute	item_attr[EXCHANGE_ITEM_MAX_NUM][ITEM_ATTRIBUTE_SLOT_MAX_NUM];

			BYTE					accept;
			DWORD					elk;
#if defined(__BL_TRANSMUTATION__)
			DWORD					dwTransmutationVnum[EXCHANGE_ITEM_MAX_NUM];
#endif
#ifdef ELEMENT_SPELL_WORLDARD
			BYTE 					item_grade_element[EXCHANGE_ITEM_MAX_NUM];
			DWORD 	 				item_attack_element[EXCHANGE_ITEM_MAX_NUM][MAX_ELEMENTS_SPELL];
		 	BYTE 					item_element_type_bonus[EXCHANGE_ITEM_MAX_NUM];
		 	short 					item_elements_value_bonus[EXCHANGE_ITEM_MAX_NUM][MAX_ELEMENTS_SPELL];
#endif
#ifdef ENABLE_NEW_SOCKET_VALUE
			long item_min_max_value[EXCHANGE_ITEM_MAX_NUM][ITEM_MIN_MAX_VALUE_MAX_NUM];
#endif
#ifdef ENABLE_CHEQUE_SYSTEM
			DWORD					cheque;
#endif
		} TExchangeData;

	public:
		CPythonExchange();
		virtual ~CPythonExchange();

		void			Clear();

		void			Start();
		void			End();
		bool			isTrading();

		// Interface

		void			SetSelfName(const char *name);
		void			SetTargetName(const char *name);

		char			*GetNameFromSelf();
		char			*GetNameFromTarget();

#ifdef ENABLE_LEVEL_IN_TRADE
		void			SetSelfLevel(DWORD level);
		void			SetTargetLevel(DWORD level);

		DWORD			GetLevelFromSelf();
		DWORD			GetLevelFromTarget();
#endif
#ifdef ELEMENT_SPELL_WORLDARD
		void 			SetItemElementToTarget(int pos, BYTE value_grade_element, BYTE type_element);
		void 			SetItemElementAttackToTarget(int pos, DWORD attack_element_index, DWORD attack_element);
		void 			SetItemElementValueToTarget(int pos, DWORD elements_value_bonus_index, short elements_value_bonus);
		BYTE 			GetItemElementGradeFromTarget(BYTE pos);
		DWORD 			GetItemElementAttackFromTarget(BYTE pos, DWORD attack_element_index);
		BYTE 			GetItemElementTypeFromTarget(BYTE pos);
		short 			GetItemElementValueFromTarget(BYTE pos, DWORD elements_value_bonus_index);
		void 			SetItemElementToSelf(int pos, BYTE value_grade_element, BYTE type_element);
		void 			SetItemElementAttackToSelf(int pos, DWORD attack_element_index, DWORD attack_element);
		void 			SetItemElementValueToSelf(int pos, DWORD elements_value_bonus_index, short elements_value_bonus);
		BYTE 			GetItemElementGradeFromSelf(BYTE pos);
		DWORD 			GetItemElementAttackFromSelf(BYTE pos, DWORD attack_element_index);
		BYTE 			GetItemElementTypeFromSelf(BYTE pos);
		short 			GetItemElementValueFromSelf(BYTE pos, DWORD elements_value_bonus_index);
#endif
		void			SetElkToTarget(DWORD elk);
		void			SetElkToSelf(DWORD elk);

		DWORD			GetElkFromTarget();
		DWORD			GetElkFromSelf();

		void			SetItemToTarget(DWORD pos, DWORD vnum, BYTE count);
		void			SetItemToSelf(DWORD pos, DWORD vnum, BYTE count);

		void			SetItemMetinSocketToTarget(int pos, int imetinpos, DWORD vnum);
		void			SetItemMetinSocketToSelf(int pos, int imetinpos, DWORD vnum);

		void			SetItemAttributeToTarget(int pos, int iattrpos, BYTE byType, short sValue);
		void			SetItemAttributeToSelf(int pos, int iattrpos, BYTE byType, short sValue);
#if defined(__BL_TRANSMUTATION__)
		void			SetItemChangeLookVnumToSelf(const int pos, const DWORD blVnum);
		void			SetItemChangeLookVnumToTarget(const int pos, const DWORD blVnum);

		DWORD			GetItemChangeLookVnumToSelf(const BYTE pos) const;
		DWORD			GetItemChangeLookVnumToTarget(const BYTE pos) const;
#endif

		void			DelItemOfTarget(BYTE pos);
		void			DelItemOfSelf(BYTE pos);

		DWORD			GetItemVnumFromTarget(BYTE pos);
		DWORD			GetItemVnumFromSelf(BYTE pos);

		BYTE			GetItemCountFromTarget(BYTE pos);
		BYTE			GetItemCountFromSelf(BYTE pos);

		DWORD			GetItemMetinSocketFromTarget(BYTE pos, int iMetinSocketPos);
		DWORD			GetItemMetinSocketFromSelf(BYTE pos, int iMetinSocketPos);
#ifdef ENABLE_NEW_SOCKET_VALUE
	void SetItemMinMaxValueToTarget(int pos, int iMinMaxPos, long lValue);
	void SetItemMinMaxValueToSelf(int pos, int iMinMaxPos, long lValue);

	long GetItemMinMaxValueFromTarget(int pos, int iMinMaxPos);
	long GetItemMinMaxValueFromSelf(int pos, int iMinMaxPos);
#endif

		void			GetItemAttributeFromTarget(BYTE pos, int iAttrPos, BYTE * pbyType, short * psValue);
		void			GetItemAttributeFromSelf(BYTE pos, int iAttrPos, BYTE * pbyType, short * psValue);

		void			SetAcceptToTarget(BYTE Accept);
		void			SetAcceptToSelf(BYTE Accept);

		bool			GetAcceptFromTarget();
		bool			GetAcceptFromSelf();

		bool			GetElkMode();
		void			SetElkMode(bool value);
#ifdef ENABLE_CHEQUE_SYSTEM
		void			SetChequeToTarget(DWORD cheque);
		void			SetChequeToSelf(DWORD cheque);

		DWORD			GetChequeFromTarget();
		DWORD			GetChequeFromSelf();
#endif

	protected:
		bool				m_isTrading;

		bool				m_elk_mode;
		TExchangeData		m_self;
		TExchangeData		m_victim;
};
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
