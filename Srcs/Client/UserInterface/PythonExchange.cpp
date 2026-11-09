#include "stdafx.h"
#include "PythonExchange.h"

#ifdef ELEMENT_SPELL_WORLDARD

//ELEMENT VICTIM
void CPythonExchange::SetItemElementToTarget(int pos, BYTE value_grade_element, BYTE type_element)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.item_grade_element[pos] = value_grade_element;
	m_victim.item_element_type_bonus[pos] = type_element;
}
void CPythonExchange::SetItemElementAttackToTarget(int pos, DWORD attack_element_index, DWORD attack_element)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.item_attack_element[pos][attack_element_index] = attack_element;
}

void CPythonExchange::SetItemElementValueToTarget(int pos, DWORD elements_value_bonus_index, short elements_value_bonus)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.item_elements_value_bonus[pos][elements_value_bonus_index] = elements_value_bonus;
}

BYTE CPythonExchange::GetItemElementGradeFromTarget(BYTE pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_victim.item_grade_element[pos];
}

DWORD CPythonExchange::GetItemElementAttackFromTarget(BYTE pos, DWORD attack_element_index)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;
	
	return m_victim.item_attack_element[pos][attack_element_index];
}

BYTE CPythonExchange::GetItemElementTypeFromTarget(BYTE pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;
	
	return m_victim.item_element_type_bonus[pos];
}

short CPythonExchange::GetItemElementValueFromTarget(BYTE pos, DWORD elements_value_bonus_index)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;
	
	return m_victim.item_elements_value_bonus[pos][elements_value_bonus_index];
}


//ELEMENT SELF
void CPythonExchange::SetItemElementToSelf(int pos, BYTE value_grade_element, BYTE type_element)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.item_grade_element[pos] = value_grade_element;
	m_self.item_element_type_bonus[pos] = type_element;
}
void CPythonExchange::SetItemElementAttackToSelf(int pos, DWORD attack_element_index, DWORD attack_element)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.item_attack_element[pos][attack_element_index] = attack_element;
}

void CPythonExchange::SetItemElementValueToSelf(int pos, DWORD elements_value_bonus_index, short elements_value_bonus)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.item_elements_value_bonus[pos][elements_value_bonus_index] = elements_value_bonus;
}


BYTE CPythonExchange::GetItemElementGradeFromSelf(BYTE pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_self.item_grade_element[pos];
}

DWORD CPythonExchange::GetItemElementAttackFromSelf(BYTE pos, DWORD attack_element_index)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;
	
	return m_self.item_attack_element[pos][attack_element_index];
}

BYTE CPythonExchange::GetItemElementTypeFromSelf(BYTE pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;
	
	return m_self.item_element_type_bonus[pos];
}

short CPythonExchange::GetItemElementValueFromSelf(BYTE pos, DWORD elements_value_bonus_index)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;
	
	return m_self.item_elements_value_bonus[pos][elements_value_bonus_index];
}

#endif

void CPythonExchange::SetSelfName(const char *name)
{
	strncpy(m_self.name, name, CHARACTER_NAME_MAX_LEN);
}

void CPythonExchange::SetTargetName(const char *name)
{
	strncpy(m_victim.name, name, CHARACTER_NAME_MAX_LEN);
}

char * CPythonExchange::GetNameFromSelf()
{
	return m_self.name;
}

char * CPythonExchange::GetNameFromTarget()
{
	return m_victim.name;
}

#ifdef ENABLE_LEVEL_IN_TRADE
void CPythonExchange::SetSelfLevel(DWORD level)
{
	m_self.level = level;
}

void CPythonExchange::SetTargetLevel(DWORD level)
{
	m_victim.level = level;
}

DWORD CPythonExchange::GetLevelFromSelf()
{
	return m_self.level;
}

DWORD CPythonExchange::GetLevelFromTarget()
{
	return m_victim.level;
}
#endif

void CPythonExchange::SetElkToTarget(DWORD	elk)
{
	m_victim.elk = elk;
}

void CPythonExchange::SetElkToSelf(DWORD elk)
{
	m_self.elk = elk;
}

DWORD CPythonExchange::GetElkFromTarget()
{
	return m_victim.elk;
}

DWORD CPythonExchange::GetElkFromSelf()
{
	return m_self.elk;
}

void CPythonExchange::SetItemToTarget(DWORD pos, DWORD vnum, BYTE count)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.item_vnum[pos] = vnum;
	m_victim.item_count[pos] = count;
}

void CPythonExchange::SetItemToSelf(DWORD pos, DWORD vnum, BYTE count)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.item_vnum[pos] = vnum;
	m_self.item_count[pos] = count;
}

void CPythonExchange::SetItemMetinSocketToTarget(int pos, int imetinpos, DWORD vnum)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.item_metin[pos][imetinpos] = vnum;
}

void CPythonExchange::SetItemMetinSocketToSelf(int pos, int imetinpos, DWORD vnum)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.item_metin[pos][imetinpos] = vnum;
}

void CPythonExchange::SetItemAttributeToTarget(int pos, int iattrpos, BYTE byType, short sValue)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.item_attr[pos][iattrpos].bType = byType;
	m_victim.item_attr[pos][iattrpos].sValue = sValue;
}

void CPythonExchange::SetItemAttributeToSelf(int pos, int iattrpos, BYTE byType, short sValue)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.item_attr[pos][iattrpos].bType = byType;
	m_self.item_attr[pos][iattrpos].sValue = sValue;
}

#if defined(__BL_TRANSMUTATION__)
void CPythonExchange::SetItemChangeLookVnumToSelf(const int pos, const DWORD blVnum)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.dwTransmutationVnum[pos] = blVnum;
}

void CPythonExchange::SetItemChangeLookVnumToTarget(const int pos, const DWORD blVnum)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.dwTransmutationVnum[pos] = blVnum;
}

DWORD CPythonExchange::GetItemChangeLookVnumToSelf(const BYTE pos) const
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_self.dwTransmutationVnum[pos];
}

DWORD CPythonExchange::GetItemChangeLookVnumToTarget(const BYTE pos) const
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_victim.dwTransmutationVnum[pos];
}
#endif

void CPythonExchange::DelItemOfTarget(BYTE pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.item_vnum[pos] = 0;
	m_victim.item_count[pos] = 0;
}

void CPythonExchange::DelItemOfSelf(BYTE pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.item_vnum[pos] = 0;
	m_self.item_count[pos] = 0;
}

DWORD CPythonExchange::GetItemVnumFromTarget(BYTE pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_victim.item_vnum[pos];
}

DWORD CPythonExchange::GetItemVnumFromSelf(BYTE pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_self.item_vnum[pos];
}

BYTE CPythonExchange::GetItemCountFromTarget(BYTE pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_victim.item_count[pos];
}

BYTE CPythonExchange::GetItemCountFromSelf(BYTE pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_self.item_count[pos];
}

DWORD CPythonExchange::GetItemMetinSocketFromTarget(BYTE pos, int iMetinSocketPos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_victim.item_metin[pos][iMetinSocketPos];
}

DWORD CPythonExchange::GetItemMetinSocketFromSelf(BYTE pos, int iMetinSocketPos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_self.item_metin[pos][iMetinSocketPos];
}

void CPythonExchange::GetItemAttributeFromTarget(BYTE pos, int iAttrPos, BYTE * pbyType, short * psValue)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	*pbyType = m_victim.item_attr[pos][iAttrPos].bType;
	*psValue = m_victim.item_attr[pos][iAttrPos].sValue;
}

void CPythonExchange::GetItemAttributeFromSelf(BYTE pos, int iAttrPos, BYTE * pbyType, short * psValue)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	*pbyType = m_self.item_attr[pos][iAttrPos].bType;
	*psValue = m_self.item_attr[pos][iAttrPos].sValue;
}

#ifdef ENABLE_NEW_SOCKET_VALUE
void CPythonExchange::SetItemMinMaxValueToTarget(int pos, int iMinMaxPos, long lValue)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.item_min_max_value[pos][iMinMaxPos] = lValue;
}

void CPythonExchange::SetItemMinMaxValueToSelf(int pos, int iMinMaxPos, long lValue)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.item_min_max_value[pos][iMinMaxPos] = lValue;
}

long CPythonExchange::GetItemMinMaxValueFromTarget(int pos, int iMinMaxPos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_victim.item_min_max_value[pos][iMinMaxPos];
}

long CPythonExchange::GetItemMinMaxValueFromSelf(int pos, int iMinMaxPos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_self.item_min_max_value[pos][iMinMaxPos];
}
#endif

void CPythonExchange::SetAcceptToTarget(BYTE Accept)
{
	m_victim.accept = Accept ? true : false;
}

void CPythonExchange::SetAcceptToSelf(BYTE Accept)
{
	m_self.accept = Accept ? true : false;
}

bool CPythonExchange::GetAcceptFromTarget()
{
	return m_victim.accept ? true : false;
}

bool CPythonExchange::GetAcceptFromSelf()
{
	return m_self.accept ? true : false;
}

bool CPythonExchange::GetElkMode()
{
	return m_elk_mode;
}

void CPythonExchange::SetElkMode(bool value)
{
	m_elk_mode = value;
}

void CPythonExchange::Start()
{
	m_isTrading = true;
}

void CPythonExchange::End()
{
	m_isTrading = false;
}

bool CPythonExchange::isTrading()
{
	return m_isTrading;
}

void CPythonExchange::Clear()
{
	memset(&m_self, 0, sizeof(m_self));
	memset(&m_victim, 0, sizeof(m_victim));
/*
	m_self.item_vnum[0] = 30;
	m_victim.item_vnum[0] = 30;
	m_victim.item_vnum[1] = 40;
	m_victim.item_vnum[2] = 50;
*/
}

CPythonExchange::CPythonExchange()
{
	Clear();
	m_isTrading = false;
	m_elk_mode = false;
}
CPythonExchange::~CPythonExchange()
{
}

#ifdef ENABLE_CHEQUE_SYSTEM
void CPythonExchange::SetChequeToTarget(DWORD cheque)
{
	m_victim.cheque = cheque;
}

void CPythonExchange::SetChequeToSelf(DWORD cheque)
{
	m_self.cheque = cheque;
}

DWORD CPythonExchange::GetChequeFromTarget()
{
	return m_victim.cheque;
}

DWORD CPythonExchange::GetChequeFromSelf()
{
	return m_self.cheque;
}
#endif
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
