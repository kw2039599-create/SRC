#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "PythonItem.h"
#include "PythonShop.h"
#include "PythonExchange.h"
#include "PythonSafeBox.h"
#include "PythonCharacterManager.h"
#ifdef ENABLE_GEM_SYSTEM
#include "PythonPlayer.h"
#endif
#include "AbstractPlayer.h"

//////////////////////////////////////////////////////////////////////////
// SafeBox
#ifdef ENABLE_OFFLINESHOP_SYSTEM
bool CPythonNetworkStream::SendOfflineShopEndPacket()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGShop packet_shop;
	packet_shop.header = HEADER_CG_OFFLINE_SHOP;
	packet_shop.subheader = SHOP_SUBHEADER_CG_END;

	if (!Send(sizeof(packet_shop), &packet_shop))
	{
		Tracef("SendOfflineShopEndPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendOfflineShopBuyPacket(DWORD vid, BYTE bPos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_OFFLINE_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_BUY;
	PacketShop.vid = vid;
	PacketShop.pos = bPos;
	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendOfflineShopBuyPacket Error\n");
		return false;
	}
	//TraceError("SendOfflineShopBuyPacket vid=%d sub=%d",vid,bPos);
	return SendSequence();
}

bool CPythonNetworkStream::SendAddOfflineShopItem(TItemPos bDisplayPos, BYTE bPos, long long lPrice)
{
	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_OFFLINE_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_ADD_ITEM;

	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendAddOfflineShopItem Error\n");
		return false;
	}

	TOfflineShopAddItem pTable;

	pTable.bDisplayPos = bDisplayPos;
	pTable.bPos = bPos;
	pTable.lPrice = lPrice;
	if (!Send(sizeof(TOfflineShopAddItem), &pTable))
	{
		Tracef("SendAddOfflineShopItem Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendRemoveOfflineShopItem(BYTE bPos)
{
	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_OFFLINE_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_REMOVE_ITEM;

	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendRemoveOfflineShopItem Error\n");
		return false;
	}

	if (!Send(sizeof(BYTE), &bPos))
	{
		Tracef("SendRemoveOfflineShopItem Packet Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendOpenShopSlot(BYTE bPos)
{
	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_OFFLINE_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_OPEN_SLOT;

	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendOpenShopSlot Error\n");
		return false;
	}

	if (!Send(sizeof(BYTE), &bPos))
	{
		Tracef("SendOpenShopSlot Packet Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendGetBackItems()
{
	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_OFFLINE_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_GET_ITEM;
	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendOpenShopSlot Error\n");
		return false;
	}
	return SendSequence();
}

bool CPythonNetworkStream::SendAddTime()
{
	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_OFFLINE_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_ADD_TIME;
	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendOpenShopSlot Error\n");
		return false;
	}
	return SendSequence();
}

bool CPythonNetworkStream::SendDestroyOfflineShop()
{
	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_OFFLINE_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_DESTROY_OFFLINE_SHOP;
	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendDestroyOfflineShop Packet Error\n");
		return false;
	}
	return SendSequence();
}

bool CPythonNetworkStream::SendTakeOfflineMoney()
{
	static DWORD s_LastTime = timeGetTime() - 5001;
	if (timeGetTime() - s_LastTime < 5000)
		return true;
	s_LastTime = timeGetTime();
	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_OFFLINE_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_TAKE_MONEY;
	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendRefreshOfflineShopMoney Packet Error\n");
		return false;
	}
	return SendSequence();
}

bool CPythonNetworkStream::SendOfflineShopChangeTitle(const char* title)
{
	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_OFFLINE_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_CHANGE_TITLE;
	strcpy(PacketShop.title, title);
	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendOfflineShopChangeTitle Packet Error\n");
		return false;
	}
	return SendSequence();
}

bool CPythonNetworkStream::SendOfflineShopCheck()
{
	//static DWORD s_LastTime = timeGetTime() - 5001;
	//if (timeGetTime() - s_LastTime < 5000)
		//	return true;
	//s_LastTime = timeGetTime();
	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_OFFLINE_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_CHECK;
	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendOfflineShopCheck Packet error\n");
		return false;
	}
	return SendSequence();
}

bool CPythonNetworkStream::SendOfflineShopButton()
{
	//static DWORD s_LastTime = timeGetTime() - 5001;
	//if (timeGetTime() - s_LastTime < 5000)
	//	return true;
	//s_LastTime = timeGetTime();
	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_OFFLINE_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_BUTTON;
	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendOfflineShopCheck Packet error\n");
		return false;
	}
	return SendSequence();
}

bool CPythonNetworkStream::SendOfflineShopRemoveLogs()
{
	//static DWORD s_LastTime = timeGetTime() - 5001;
	//if (timeGetTime() - s_LastTime < 5000)
	//	return true;
	//s_LastTime = timeGetTime();
	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_OFFLINE_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_LOG_REMOVE;
	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendOfflineShopCheck Packet error\n");
		return false;
	}
	return SendSequence();
}

bool CPythonNetworkStream::SendOfflineOpenWithVID(DWORD vid)
{
	//static DWORD s_LastTime = timeGetTime() - 5001;
	//if (timeGetTime() - s_LastTime < 5000)
	//	return true;
	//s_LastTime = timeGetTime();
	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_OFFLINE_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_OPEN_WITH_VID;
	PacketShop.vid = vid;
	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendOfflineOpenWithVID Packet error\n");
		return false;
	}
	return SendSequence();
}

bool CPythonNetworkStream::SendOfflineShopChangeDecoration(const char* sign, DWORD vnum, DWORD type)
{
	//static DWORD s_LastTime = timeGetTime() - 5001;
	//if (timeGetTime() - s_LastTime < 5000)
	//	return true;
	//s_LastTime = timeGetTime();
	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_OFFLINE_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_CHANGE_DECORATION;
	if (!Send(sizeof(TPacketCGShop), &PacketShop))
	{
		Tracef("SendOfflineShopChangeDecoration Packet error\n");
		return false;
	}

	TShopDecoration p;
	snprintf(p.sign, sizeof(p.sign), "%s", sign);
	p.vnum = vnum;
	p.type = type;
	if (!Send(sizeof(TShopDecoration), &p))
	{
		Tracef("SendOfflineShopChangeDecoration Packet Error\n");
		return false;
	}

	return SendSequence();
}
#endif

bool CPythonNetworkStream::SendSafeBoxMoneyPacket(BYTE byState, DWORD dwMoney)
{
	assert(!"CPythonNetworkStream::SendSafeBoxMoneyPacket - Don't use this function");
	return false;
}

bool CPythonNetworkStream::SendSafeBoxCheckinPacket(TItemPos InventoryPos, BYTE bySafeBoxPos)
{
	__PlayInventoryItemDropSound(InventoryPos);

	TPacketCGSafeboxCheckin kSafeboxCheckin;
	kSafeboxCheckin.bHeader = HEADER_CG_SAFEBOX_CHECKIN;
	kSafeboxCheckin.ItemPos = InventoryPos;
	kSafeboxCheckin.bSafePos = bySafeBoxPos;
	if (!Send(kSafeboxCheckin))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendSafeBoxCheckoutPacket(BYTE bySafeBoxPos, TItemPos InventoryPos)
{
	__PlaySafeBoxItemDropSound(bySafeBoxPos);

	TPacketCGSafeboxCheckout kSafeboxCheckout;
	kSafeboxCheckout.bHeader = HEADER_CG_SAFEBOX_CHECKOUT;
	kSafeboxCheckout.bSafePos = bySafeBoxPos;
	kSafeboxCheckout.ItemPos = InventoryPos;
	if (!Send(kSafeboxCheckout))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::SendSafeBoxItemMovePacket(BYTE bySourcePos, BYTE byTargetPos, BYTE byCount)
{
	__PlaySafeBoxItemDropSound(bySourcePos);

	TPacketCGItemMove kItemMove;
	kItemMove.header = HEADER_CG_SAFEBOX_ITEM_MOVE;
	kItemMove.pos = TItemPos(INVENTORY, bySourcePos);
	kItemMove.num = byCount;
	kItemMove.change_pos = TItemPos(INVENTORY, byTargetPos);
	if (!Send(kItemMove))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvSafeBoxSetPacket()
{
	TPacketGCItemSet2 kItemSet;
	if (!AutoRecv(kItemSet))
		return false;

	TItemData kItemData;
	kItemData.vnum	= kItemSet.vnum;
	kItemData.count = kItemSet.count;
	kItemData.flags = kItemSet.flags;
	kItemData.anti_flags = kItemSet.anti_flags;
#ifdef ELEMENT_SPELL_WORLDARD
	kItemData.grade_element = kItemSet.grade_element;
	for (int i=0; i<MAX_ELEMENTS_SPELL; ++i){
		kItemData.attack_element[i] = kItemSet.attack_element[i];
		kItemData.elements_value_bonus[i] = kItemSet.elements_value_bonus[i];
	}
	kItemData.element_type_bonus = kItemSet.element_type_bonus;
#endif
	for (int isocket=0; isocket<ITEM_SOCKET_SLOT_MAX_NUM; ++isocket)
		kItemData.alSockets[isocket] = kItemSet.alSockets[isocket];
	for (int iattr=0; iattr<ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++iattr)
		kItemData.aAttr[iattr] = kItemSet.aAttr[iattr];
#if defined(__BL_TRANSMUTATION__)
	kItemData.dwTransmutationVnum = kItemSet.dwTransmutationVnum;
#endif
#ifdef ENABLE_SOULBIND_SYSTEM
	kItemData.sealbind = kItemSet.sealbind;
#endif
#ifdef ENABLE_NEW_SOCKET_VALUE
	for (int iMinMax = 0; iMinMax < ITEM_MIN_MAX_VALUE_MAX_NUM; ++iMinMax)
		kItemData.alMinMaxValue[iMinMax] = kItemSet.alMinMaxValue[iMinMax];
#endif
	CPythonSafeBox::Instance().SetItemData(kItemSet.Cell.cell, kItemData);

	__RefreshSafeboxWindow();

	return true;
}

bool CPythonNetworkStream::RecvSafeBoxDelPacket()
{
	TPacketGCItemDel kItemDel;
	if (!AutoRecv(kItemDel))
		return false;

	CPythonSafeBox::Instance().DelItemData(kItemDel.pos);

	__RefreshSafeboxWindow();

	return true;
}

bool CPythonNetworkStream::RecvSafeBoxWrongPasswordPacket()
{
	TPacketGCSafeboxWrongPassword kSafeboxWrongPassword;

	if (!AutoRecv(kSafeboxWrongPassword))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OnSafeBoxError", Py_BuildValue("()"));

	return true;
}

bool CPythonNetworkStream::RecvSafeBoxSizePacket()
{
	TPacketGCSafeboxSize kSafeBoxSize;
	if (!AutoRecv(kSafeBoxSize))
		return false;

	CPythonSafeBox::Instance().OpenSafeBox(kSafeBoxSize.bSize);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenSafeboxWindow", Py_BuildValue("(i)", kSafeBoxSize.bSize));

	return true;
}

bool CPythonNetworkStream::RecvSafeBoxMoneyChangePacket()
{
	TPacketGCSafeboxMoneyChange kMoneyChange;
	if (!AutoRecv(kMoneyChange))
		return false;

	CPythonSafeBox::Instance().SetMoney(kMoneyChange.dwMoney);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSafeboxMoney", Py_BuildValue("()"));

	return true;
}

// SafeBox
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Mall
bool CPythonNetworkStream::SendMallCheckoutPacket(BYTE byMallPos, TItemPos InventoryPos)
{
	__PlayMallItemDropSound(byMallPos);

	TPacketCGMallCheckout kMallCheckoutPacket;
	kMallCheckoutPacket.bHeader = HEADER_CG_MALL_CHECKOUT;
	kMallCheckoutPacket.bMallPos = byMallPos;
	kMallCheckoutPacket.ItemPos = InventoryPos;
	if (!Send(kMallCheckoutPacket))
		return false;

	return SendSequence();
}

bool CPythonNetworkStream::RecvMallOpenPacket()
{
	TPacketGCMallOpen kMallOpen;
	if (!AutoRecv(kMallOpen))
		return false;

	CPythonSafeBox::Instance().OpenMall(kMallOpen.bSize);
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenMallWindow", Py_BuildValue("(i)", kMallOpen.bSize));

	return true;
}
bool CPythonNetworkStream::RecvMallItemSetPacket()
{
	TPacketGCItemSet2 kItemSet;
	if (!AutoRecv(kItemSet))
		return false;

	TItemData kItemData;
	kItemData.vnum = kItemSet.vnum;
	kItemData.count = kItemSet.count;
	kItemData.flags = kItemSet.flags;
	kItemData.anti_flags = kItemSet.anti_flags;
#ifdef ELEMENT_SPELL_WORLDARD
	kItemData.grade_element = kItemSet.grade_element;
	for (int i=0; i<MAX_ELEMENTS_SPELL; ++i){
		kItemData.attack_element[i] = kItemSet.attack_element[i];
		kItemData.elements_value_bonus[i] = kItemSet.elements_value_bonus[i];
	}
	kItemData.element_type_bonus = kItemSet.element_type_bonus;
#endif
	for (int isocket=0; isocket<ITEM_SOCKET_SLOT_MAX_NUM; ++isocket)
		kItemData.alSockets[isocket] = kItemSet.alSockets[isocket];
	for (int iattr=0; iattr<ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++iattr)
		kItemData.aAttr[iattr] = kItemSet.aAttr[iattr];
#if defined(__BL_TRANSMUTATION__)
	kItemData.dwTransmutationVnum = kItemSet.dwTransmutationVnum;
#endif
#ifdef ENABLE_SOULBIND_SYSTEM
	kItemData.sealbind = kItemSet.sealbind;
#endif
#ifdef ENABLE_NEW_SOCKET_VALUE
	for (int iMinMax = 0; iMinMax < ITEM_MIN_MAX_VALUE_MAX_NUM; ++iMinMax)
		kItemData.alMinMaxValue[iMinMax] = kItemSet.alMinMaxValue[iMinMax];
#endif
	CPythonSafeBox::Instance().SetMallItemData(kItemSet.Cell.cell, kItemData);

	__RefreshMallWindow();

	return true;
}
bool CPythonNetworkStream::RecvMallItemDelPacket()
{
	TPacketGCItemDel kItemDel;
	if (!AutoRecv(kItemDel))
		return false;

	CPythonSafeBox::Instance().DelMallItemData(kItemDel.pos);

	__RefreshMallWindow();
	Tracef(" >> CPythonNetworkStream::RecvMallItemDelPacket\n");

	return true;
}
// Mall
//////////////////////////////////////////////////////////////////////////

// Item
// Recieve
bool CPythonNetworkStream::RecvItemSetPacket()
{
	TPacketGCItemSet packet_item_set;

	if (!AutoRecv(packet_item_set))
		return false;

	TItemData kItemData;
	kItemData.vnum	= packet_item_set.vnum;
	kItemData.count	= packet_item_set.count;
	kItemData.flags = 0;
#ifdef ELEMENT_SPELL_WORLDARD
	kItemData.grade_element = packet_item_set.grade_element;
	for (int i=0; i<MAX_ELEMENTS_SPELL; ++i){
		kItemData.attack_element[i] = packet_item_set.attack_element[i];
		kItemData.elements_value_bonus[i] = packet_item_set.elements_value_bonus[i];
	}
	kItemData.element_type_bonus = packet_item_set.element_type_bonus;
#endif
	for (int i=0; i<ITEM_SOCKET_SLOT_MAX_NUM; ++i)
		kItemData.alSockets[i]=packet_item_set.alSockets[i];
	for (int j=0; j<ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
		kItemData.aAttr[j]=packet_item_set.aAttr[j];
#if defined(__BL_TRANSMUTATION__)
	kItemData.dwTransmutationVnum = packet_item_set.dwTransmutationVnum;
#endif
#ifdef ENABLE_NEW_SOCKET_VALUE
	for (int l = 0; l < ITEM_MIN_MAX_VALUE_MAX_NUM; ++l)
		kItemData.alMinMaxValue[l] = packet_item_set.alMinMaxValue[l];
#endif
	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();

	rkPlayer.SetItemData(packet_item_set.Cell, kItemData);
#ifdef ENABLE_SWITCHBOT
	if (packet_item_set.Cell.window_type == SWITCHBOT)
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSwitchbotWindow", Py_BuildValue("()"));
		return true;
	}
#endif
	__RefreshInventoryWindow();
	return true;
}

bool CPythonNetworkStream::RecvItemSetPacket2()
{
	TPacketGCItemSet2 packet_item_set;

	if (!AutoRecv(packet_item_set))
		return false;

	TItemData kItemData;
	kItemData.vnum	= packet_item_set.vnum;
	kItemData.count	= packet_item_set.count;
	kItemData.flags = packet_item_set.flags;
	kItemData.anti_flags = packet_item_set.anti_flags;
#ifdef ELEMENT_SPELL_WORLDARD
	kItemData.grade_element = packet_item_set.grade_element;
	for (int i=0; i<MAX_ELEMENTS_SPELL; ++i){
		kItemData.attack_element[i] = packet_item_set.attack_element[i];
		kItemData.elements_value_bonus[i] = packet_item_set.elements_value_bonus[i];
	}
	kItemData.element_type_bonus = packet_item_set.element_type_bonus;
#endif
	for (int i=0; i<ITEM_SOCKET_SLOT_MAX_NUM; ++i)
		kItemData.alSockets[i]=packet_item_set.alSockets[i];
	for (int j=0; j<ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
		kItemData.aAttr[j]=packet_item_set.aAttr[j];
#if defined(__BL_TRANSMUTATION__)
	kItemData.dwTransmutationVnum = packet_item_set.dwTransmutationVnum;
#endif
#ifdef ENABLE_SOULBIND_SYSTEM
	kItemData.sealbind = packet_item_set.sealbind;
#endif
#ifdef ENABLE_NEW_SOCKET_VALUE
	for (int l = 0; l < ITEM_MIN_MAX_VALUE_MAX_NUM; ++l)
		kItemData.alMinMaxValue[l] = packet_item_set.alMinMaxValue[l];
#endif
	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	rkPlayer.SetItemData(packet_item_set.Cell, kItemData);
#ifdef ENABLE_SWITCHBOT
	if (packet_item_set.Cell.window_type == SWITCHBOT)
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshSwitchbotWindow", Py_BuildValue("()"));
		return true;
	}
#endif
	if (packet_item_set.highlight)
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_Highlight_Item", Py_BuildValue("(ii)", packet_item_set.Cell.window_type, packet_item_set.Cell.cell));

	__RefreshInventoryWindow();
	return true;
}

bool CPythonNetworkStream::RecvItemUsePacket()
{
	TPacketGCItemUse packet_item_use;

	if (!AutoRecv(packet_item_use))
		return false;

	__RefreshInventoryWindow();
	return true;
}

bool CPythonNetworkStream::RecvItemUpdatePacket()
{
	TPacketGCItemUpdate packet_item_update;

	if (!AutoRecv(packet_item_update))
		return false;

	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	rkPlayer.SetItemCount(packet_item_update.Cell, packet_item_update.count);
#ifdef ELEMENT_SPELL_WORLDARD
	rkPlayer.SetElement(packet_item_update.Cell, packet_item_update.grade_element, packet_item_update.element_type_bonus);
	for (int i = 0; i < MAX_ELEMENTS_SPELL; ++i){
		rkPlayer.SetElementAttack(packet_item_update.Cell, i, packet_item_update.attack_element[i]);
		rkPlayer.SetElementValue(packet_item_update.Cell, i, packet_item_update.elements_value_bonus[i]);
	}
#endif
	for (int i = 0; i < ITEM_SOCKET_SLOT_MAX_NUM; ++i)
		rkPlayer.SetItemMetinSocket(packet_item_update.Cell, i, packet_item_update.alSockets[i]);
	for (int j = 0; j < ITEM_ATTRIBUTE_SLOT_MAX_NUM; ++j)
		rkPlayer.SetItemAttribute(packet_item_update.Cell, j, packet_item_update.aAttr[j].bType, packet_item_update.aAttr[j].sValue);
#if defined(__BL_TRANSMUTATION__)
	rkPlayer.SetItemTransmutationVnum(packet_item_update.Cell, packet_item_update.dwTransmutationVnum);
#endif
#ifdef ENABLE_SOULBIND_SYSTEM
	rkPlayer.SetItemUnbindTime(packet_item_update.Cell, packet_item_update.sealbind);
#endif
#ifdef ENABLE_NEW_SOCKET_VALUE
	for (int k = 0; k < ITEM_MIN_MAX_VALUE_MAX_NUM; ++k)
		rkPlayer.SetItemMinMaxValue(packet_item_update.Cell, k, packet_item_update.alMinMaxValue[k]);
#endif

	__RefreshInventoryWindow();
	return true;
}

bool CPythonNetworkStream::RecvItemGroundAddPacket()
{
	TPacketGCItemGroundAdd packet_item_ground_add;

	if (!AutoRecv(packet_item_ground_add))
		return false;

	__GlobalPositionToLocalPosition(packet_item_ground_add.lX, packet_item_ground_add.lY);

	CPythonItem::Instance().CreateItem(packet_item_ground_add.dwVID,
									   packet_item_ground_add.dwVnum,
									   packet_item_ground_add.lX,
									   packet_item_ground_add.lY,
									   packet_item_ground_add.lZ);
	return true;
}

bool CPythonNetworkStream::RecvItemOwnership()
{
	TPacketGCItemOwnership p;

	if (!AutoRecv(p))
		return false;

	CPythonItem::Instance().SetOwnership(p.dwVID, p.szName);
	return true;
}

bool CPythonNetworkStream::RecvItemGroundDelPacket()
{
	TPacketGCItemGroundDel	packet_item_ground_del;

	if (!AutoRecv(packet_item_ground_del))
		return false;

	CPythonItem::Instance().DeleteItem(packet_item_ground_del.vid);
	return true;
}

bool CPythonNetworkStream::RecvQuickSlotAddPacket()
{
	TPacketGCQuickSlotAdd packet_quick_slot_add;

	if (!AutoRecv(packet_quick_slot_add))
		return false;

	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	rkPlayer.AddQuickSlot(packet_quick_slot_add.pos, packet_quick_slot_add.slot.Type, packet_quick_slot_add.slot.Position);

	__RefreshInventoryWindow();

	return true;
}

bool CPythonNetworkStream::RecvQuickSlotDelPacket()
{
	TPacketGCQuickSlotDel packet_quick_slot_del;

	if (!AutoRecv(packet_quick_slot_del))
		return false;

	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	rkPlayer.DeleteQuickSlot(packet_quick_slot_del.pos);

	__RefreshInventoryWindow();

	return true;
}

bool CPythonNetworkStream::RecvQuickSlotMovePacket()
{
	TPacketGCQuickSlotSwap packet_quick_slot_swap;

	if (!AutoRecv(packet_quick_slot_swap))
		return false;

	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	rkPlayer.MoveQuickSlot(packet_quick_slot_swap.pos, packet_quick_slot_swap.change_pos);

	__RefreshInventoryWindow();

	return true;
}

bool CPythonNetworkStream::SendShopEndPacket()
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGShop packet_shop;
	packet_shop.header = HEADER_CG_SHOP;
	packet_shop.subheader = SHOP_SUBHEADER_CG_END;

	if (!Send(packet_shop))
	{
		Tracef("SendShopEndPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendShopBuyPacket(BYTE bPos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_BUY;

	if (!Send(PacketShop))
	{
		Tracef("SendShopBuyPacket Error\n");
		return false;
	}

	BYTE bCount=1;
	if (!Send(bCount))
	{
		Tracef("SendShopBuyPacket Error\n");
		return false;
	}

	if (!Send(bPos))
	{
		Tracef("SendShopBuyPacket Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendShopSellPacket(BYTE bySlot)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_SELL;

	if (!Send(PacketShop))
	{
		Tracef("SendShopSellPacket Error\n");
		return false;
	}
	if (!Send(bySlot))
	{
		Tracef("SendShopAddSellPacket Error\n");
		return false;
	}

	return SendSequence();
}

#ifdef ENABLE_SPECIAL_STORAGE
bool CPythonNetworkStream::SendShopSellPacketNew(BYTE bySlot, BYTE byCount, BYTE byType)
#else
bool CPythonNetworkStream::SendShopSellPacketNew(BYTE bySlot, BYTE byCount)
#endif
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGShop PacketShop;
	PacketShop.header = HEADER_CG_SHOP;
	PacketShop.subheader = SHOP_SUBHEADER_CG_SELL2;

	if (!Send(PacketShop))
	{
		Tracef("SendShopSellPacket Error\n");
		return false;
	}
	if (!Send(bySlot))
	{
		Tracef("SendShopAddSellPacket Error\n");
		return false;
	}
	if (!Send(byCount))
	{
		Tracef("SendShopAddSellPacket Error\n");
		return false;
	}

#ifdef ENABLE_SPECIAL_STORAGE
		if (!Send(sizeof(BYTE), &byType))
		{
			Tracef("SendShopAddSellPacket Error\n");
			return false;
		}
#endif

	Tracef(" SendShopSellPacketNew(bySlot=%d, byCount=%d)\n", bySlot, byCount);

	return SendSequence();
}

// Send
bool CPythonNetworkStream::SendItemUsePacket(TItemPos pos)
{
	if (!__CanActMainInstance())
		return true;

	if (__IsEquipItemInSlot(pos))
	{
		if (CPythonExchange::Instance().isTrading())
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AppendNotifyMessage", Py_BuildValue("(s)", "CANNOT_EQUIP_EXCHANGE"));
			return true;
		}

		if (CPythonShop::Instance().IsOpen())
		{
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AppendNotifyMessage", Py_BuildValue("(s)", "CANNOT_EQUIP_SHOP"));
			return true;
		}

		if (__IsPlayerAttacking())
			return true;
	}

	__PlayInventoryItemUseSound(pos);

	TPacketCGItemUse itemUsePacket;
	itemUsePacket.header = HEADER_CG_ITEM_USE;
	itemUsePacket.pos = pos;

	if (!Send(itemUsePacket))
	{
		Tracen("SendItemUsePacket Error");
		return false;
	}

	return SendSequence();
}

#ifdef ENABLE_SORT_INVEN
bool CPythonNetworkStream::SortInven(BYTE window_type, BYTE option)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGSortInven p;
	p.header = SORT_INVEN;
	p.window_type = window_type; // <-- Changed 'packet' to 'p'
	p.option = option;

	if (!Send(sizeof(p), &p)) {
		Tracen("Error sort inven packet");
		return false;
	}

	return SendSequence();
}
#endif


#ifdef ENABLE_CHEST_OPEN_RENEWAL
bool CPythonNetworkStream::SendOpenChestPacket(TItemPos pos, WORD wOpenCount)
{
	if (!__CanActMainInstance())
		return true;
	__PlayInventoryItemUseSound(pos);

	TPacketCGOpenChest chestUsePacket;
	chestUsePacket.header = HEADER_CG_CHEST_OPEN;
	chestUsePacket.pos = pos;
	chestUsePacket.wOpenCount = wOpenCount;

	if (!Send(sizeof(TPacketCGOpenChest), &chestUsePacket))
	{
		Tracen("SendOpenChestPacket Error");
		return false;
	}

	return SendSequence();
}
#endif

bool CPythonNetworkStream::SendItemUseToItemPacket(TItemPos source_pos, TItemPos target_pos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGItemUseToItem itemUseToItemPacket;
	itemUseToItemPacket.header = HEADER_CG_ITEM_USE_TO_ITEM;
	itemUseToItemPacket.source_pos = source_pos;
	itemUseToItemPacket.target_pos = target_pos;

	if (!Send(itemUseToItemPacket))
	{
		Tracen("SendItemUseToItemPacket Error");
		return false;
	}

#ifdef _DEBUG
	Tracef(" << SendItemUseToItemPacket(src=%d, dst=%d)\n", source_pos, target_pos);
#endif

	return SendSequence();
}

bool CPythonNetworkStream::SendItemDropPacket(TItemPos pos, DWORD elk
	#if defined(ENABLE_CHEQUE_SYSTEM) && !defined(DISABLE_CHEQUE_DROP)
	, DWORD cheque
	#endif
)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGItemDrop itemDropPacket;
	itemDropPacket.header = HEADER_CG_ITEM_DROP;
	itemDropPacket.pos = pos;
	itemDropPacket.elk = elk;
#if defined(ENABLE_CHEQUE_SYSTEM) && !defined(DISABLE_CHEQUE_DROP)
	itemDropPacket.cheque = cheque;
#endif

	if (!Send(itemDropPacket))
	{
		Tracen("SendItemDropPacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendItemDropPacketNew(TItemPos pos, DWORD elk, DWORD count
#if defined(ENABLE_CHEQUE_SYSTEM) && !defined(DISABLE_CHEQUE_DROP)
	, DWORD cheque
#endif
)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGItemDrop2 itemDropPacket;
	itemDropPacket.header = HEADER_CG_ITEM_DROP2;
	itemDropPacket.pos = pos;
	itemDropPacket.gold = elk;
	itemDropPacket.count = count;
#if defined(ENABLE_CHEQUE_SYSTEM) && !defined(DISABLE_CHEQUE_DROP)
	itemDropPacket.cheque = cheque;
#endif

	if (!Send(itemDropPacket))
	{
		Tracen("SendItemDropPacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::__IsEquipItemInSlot(TItemPos uSlotPos)
{
	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	return rkPlayer.IsEquipItemInSlot(uSlotPos);
}

void CPythonNetworkStream::__PlayInventoryItemUseSound(TItemPos uSlotPos)
{
	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	DWORD dwItemID=rkPlayer.GetItemIndex(uSlotPos);

	CPythonItem& rkItem=CPythonItem::Instance();
	rkItem.PlayUseSound(dwItemID);
}

void CPythonNetworkStream::__PlayInventoryItemDropSound(TItemPos uSlotPos)
{
	IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
	DWORD dwItemID=rkPlayer.GetItemIndex(uSlotPos);

	CPythonItem& rkItem=CPythonItem::Instance();
	rkItem.PlayDropSound(dwItemID);
}

void CPythonNetworkStream::__PlaySafeBoxItemDropSound(UINT uSlotPos)
{
	DWORD dwItemID;
	CPythonSafeBox& rkSafeBox=CPythonSafeBox::Instance();
	if (!rkSafeBox.GetSlotItemID(uSlotPos, &dwItemID))
		return;

	CPythonItem& rkItem=CPythonItem::Instance();
	rkItem.PlayDropSound(dwItemID);
}

void CPythonNetworkStream::__PlayMallItemDropSound(UINT uSlotPos)
{
	DWORD dwItemID;
	CPythonSafeBox& rkSafeBox=CPythonSafeBox::Instance();
	if (!rkSafeBox.GetSlotMallItemID(uSlotPos, &dwItemID))
		return;

	CPythonItem& rkItem=CPythonItem::Instance();
	rkItem.PlayDropSound(dwItemID);
}

bool CPythonNetworkStream::SendItemMovePacket(TItemPos pos, TItemPos change_pos, BYTE num)
{
	if (!__CanActMainInstance())
		return true;

	if (__IsEquipItemInSlot(pos))
	{
		if (CPythonExchange::Instance().isTrading())
		{
			if (pos.IsEquipCell() || change_pos.IsEquipCell())
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AppendNotifyMessage", Py_BuildValue("(s)", "CANNOT_EQUIP_EXCHANGE"));
				return true;
			}
		}

		if (CPythonShop::Instance().IsOpen())
		{
			if (pos.IsEquipCell() || change_pos.IsEquipCell())
			{
				PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_AppendNotifyMessage", Py_BuildValue("(s)", "CANNOT_EQUIP_SHOP"));
				return true;
			}
		}

		if (__IsPlayerAttacking())
			return true;
	}

	__PlayInventoryItemDropSound(pos);

	TPacketCGItemMove	itemMovePacket;
	itemMovePacket.header = HEADER_CG_ITEM_MOVE;
	itemMovePacket.pos = pos;
	itemMovePacket.change_pos = change_pos;
	itemMovePacket.num = num;

	if (!Send(itemMovePacket))
	{
		Tracen("SendItemMovePacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendItemPickUpPacket(DWORD vid)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGItemPickUp	itemPickUpPacket;
	itemPickUpPacket.header = HEADER_CG_ITEM_PICKUP;
	itemPickUpPacket.vid = vid;

	if (!Send(itemPickUpPacket))
	{
		Tracen("SendItemPickUpPacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendQuickSlotAddPacket(BYTE wpos, BYTE type, BYTE pos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGQuickSlotAdd quickSlotAddPacket;

	quickSlotAddPacket.header		= HEADER_CG_QUICKSLOT_ADD;
	quickSlotAddPacket.pos			= wpos;
	quickSlotAddPacket.slot.Type	= type;
	quickSlotAddPacket.slot.Position = pos;

	if (!Send(quickSlotAddPacket))
	{
		Tracen("SendQuickSlotAddPacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendQuickSlotDelPacket(BYTE pos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGQuickSlotDel quickSlotDelPacket;

	quickSlotDelPacket.header = HEADER_CG_QUICKSLOT_DEL;
	quickSlotDelPacket.pos = pos;

	if (!Send(quickSlotDelPacket))
	{
		Tracen("SendQuickSlotDelPacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendQuickSlotMovePacket(BYTE pos, BYTE change_pos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGQuickSlotSwap quickSlotSwapPacket;

	quickSlotSwapPacket.header = HEADER_CG_QUICKSLOT_SWAP;
	quickSlotSwapPacket.pos = pos;
	quickSlotSwapPacket.change_pos = change_pos;

	if (!Send(quickSlotSwapPacket))
	{
		Tracen("SendQuickSlotSwapPacket Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RecvSpecialEffect()
{
	TPacketGCSpecialEffect kSpecialEffect;
	if (!AutoRecv(kSpecialEffect))
		return false;

	DWORD effect = -1;
	bool bPlayPotionSound = false;
	bool bAttachEffect = true;
	switch (kSpecialEffect.type)
	{
		case SE_HPUP_RED:
			effect = CInstanceBase::EFFECT_HPUP_RED;
			bPlayPotionSound = true;
			break;
		case SE_SPUP_BLUE:
			effect = CInstanceBase::EFFECT_SPUP_BLUE;
			bPlayPotionSound = true;
			break;
		case SE_SPEEDUP_GREEN:
			effect = CInstanceBase::EFFECT_SPEEDUP_GREEN;
			bPlayPotionSound = true;
			break;
		case SE_DXUP_PURPLE:
			effect = CInstanceBase::EFFECT_DXUP_PURPLE;
			bPlayPotionSound = true;
			break;
		case SE_CRITICAL:
			effect = CInstanceBase::EFFECT_CRITICAL;
			break;
		case SE_PENETRATE:
			effect = CInstanceBase::EFFECT_PENETRATE;
			break;
		case SE_BLOCK:
			effect = CInstanceBase::EFFECT_BLOCK;
			break;
		case SE_DODGE:
			effect = CInstanceBase::EFFECT_DODGE;
			break;
		case SE_CHINA_FIREWORK:
			effect = CInstanceBase::EFFECT_FIRECRACKER;
			bAttachEffect = false;
			break;
		case SE_SPIN_TOP:
			effect = CInstanceBase::EFFECT_SPIN_TOP;
			bAttachEffect = false;
			break;
		case SE_SUCCESS :
			effect = CInstanceBase::EFFECT_SUCCESS ;
			bAttachEffect = false ;
			break ;
		case SE_FAIL :
			effect = CInstanceBase::EFFECT_FAIL ;
			break ;
		case SE_FR_SUCCESS:
			effect = CInstanceBase::EFFECT_FR_SUCCESS;
			bAttachEffect = false ;
			break;
		case SE_LEVELUP_ON_14_FOR_GERMANY:
			effect = CInstanceBase::EFFECT_LEVELUP_ON_14_FOR_GERMANY;
			bAttachEffect = false ;
			break;
		case SE_LEVELUP_UNDER_15_FOR_GERMANY:
			effect = CInstanceBase::EFFECT_LEVELUP_UNDER_15_FOR_GERMANY;
			bAttachEffect = false ;
			break;
		case SE_PERCENT_DAMAGE1:
			effect = CInstanceBase::EFFECT_PERCENT_DAMAGE1;
			break;
		case SE_PERCENT_DAMAGE2:
			effect = CInstanceBase::EFFECT_PERCENT_DAMAGE2;
			break;
		case SE_PERCENT_DAMAGE3:
			effect = CInstanceBase::EFFECT_PERCENT_DAMAGE3;
			break;
		case SE_AUTO_HPUP:
			effect = CInstanceBase::EFFECT_AUTO_HPUP;
			break;
		case SE_AUTO_SPUP:
			effect = CInstanceBase::EFFECT_AUTO_SPUP;
			break;
		case SE_EQUIP_RAMADAN_RING:
			effect = CInstanceBase::EFFECT_RAMADAN_RING_EQUIP;
			break;
		case SE_EQUIP_HALLOWEEN_CANDY:
			effect = CInstanceBase::EFFECT_HALLOWEEN_CANDY_EQUIP;
			break;
		case SE_EQUIP_HAPPINESS_RING:
 			effect = CInstanceBase::EFFECT_HAPPINESS_RING_EQUIP;
			break;
		case SE_EQUIP_LOVE_PENDANT:
			effect = CInstanceBase::EFFECT_LOVE_PENDANT_EQUIP;
			break;
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
		case SE_EFFECT_ACCE_SUCESS_ABSORB:
			effect = CInstanceBase::EFFECT_ACCE_SUCESS_ABSORB;
			break;
		case SE_EFFECT_ACCE_EQUIP:
			effect = CInstanceBase::EFFECT_ACCE_EQUIP;
			break;
#endif
#if defined(ENABLE_VOICE_CHAT)
		case SE_VOICE_CHAT:
			effect = CInstanceBase::EFFECT_VOICE_CHAT;
			break;
#endif
#ifdef VERSION_162_ENABLED
		case SE_EFFECT_HEALER:
			effect = CInstanceBase::EFFECT_HEALER;
			break;
#endif
#ifdef ENABLE_12ZI
		case SE_SKILL_DAMAGE_ZONE:
			effect = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE;
			break;
		case SE_SKILL_SAFE_ZONE:
			effect = CInstanceBase::EFFECT_SKILL_SAFE_ZONE;
			break;

		case SE_METEOR:
			effect = CInstanceBase::EFFECT_METEOR;
			break;
		case SE_BEAD_RAIN:
			effect = CInstanceBase::EFFECT_BEAD_RAIN;
			break;
		case SE_FALL_ROCK:
			effect = CInstanceBase::EFFECT_FALL_ROCK;
			break;
		case SE_ARROW_RAIN:
			effect = CInstanceBase::EFFECT_ARROW_RAIN;
			break;
		case SE_HORSE_DROP:
			effect = CInstanceBase::EFFECT_HORSE_DROP;
			break;
		case SE_EGG_DROP:
			effect = CInstanceBase::EFFECT_EGG_DROP;
			break;
		case SE_DEAPO_BOOM:
			effect = CInstanceBase::EFFECT_DEAPO_BOOM;
			break;

		case SE_SKILL_DAMAGE_ZONE_BIG:
			effect = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_BIG;
			break;
		case SE_SKILL_DAMAGE_ZONE_MIDDLE:
			effect = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_MIDDLE;
			break;
		case SE_SKILL_DAMAGE_ZONE_SMALL:
			effect = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_SMALL;
			break;


		case SE_SKILL_SAFE_ZONE_BIG:
			effect = CInstanceBase::EFFECT_SKILL_SAFE_ZONE_BIG;
			break;
		case SE_SKILL_SAFE_ZONE_MIDDLE:
			effect = CInstanceBase::EFFECT_SKILL_SAFE_ZONE_MIDDLE;
			break;
		case SE_SKILL_SAFE_ZONE_SMALL:
			effect = CInstanceBase::EFFECT_SKILL_SAFE_ZONE_SMALL;
			break;

#endif
#ifdef ENABLE_QUEEN_NETHIS
		case SE_EFFECT_SNAKE_REGEN:
			effect = CInstanceBase::EFFECT_SNAKE_REGEN;
			bAttachEffect = false;
			break;
#endif
#if defined(WJ_COMBAT_ZONE)
		case SE_COMBAT_ZONE_POTION:
			effect = CInstanceBase::EFFECT_COMBAT_ZONE_POTION;
			break;	
#endif

		case SE_AGGREGATE_MONSTER_EFFECT:
			effect = CInstanceBase::EFFECT_AGGREGATE_MONSTER;
			break;

		default:
			TraceError("%d is not a special effect number. TPacketGCSpecialEffect",kSpecialEffect.type);
			break;
	}

	if (bPlayPotionSound)
	{
		IAbstractPlayer& rkPlayer=IAbstractPlayer::GetSingleton();
		if(rkPlayer.IsMainCharacterIndex(kSpecialEffect.vid))
		{
			CPythonItem& rkItem=CPythonItem::Instance();
			rkItem.PlayUsePotionSound();
		}
	}

	if (-1 != effect)
	{
		CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(kSpecialEffect.vid);
		if (pInstance)
		{
			if(bAttachEffect)
				pInstance->AttachSpecialEffect(effect);
			else
				pInstance->CreateSpecialEffect(effect);
		}
	}

	return true;
}

#ifdef ENABLE_12ZI
bool CPythonNetworkStream::RecvSpecialZodiacEffect()
{
	TPacketGCSpecialZodiacEffect kSpecialZodiacEffect;
	if (!Recv(sizeof(kSpecialZodiacEffect), &kSpecialZodiacEffect))
		return false;

	uint32_t effect = -1;
	switch (kSpecialZodiacEffect.type)
	{
		case SE_SKILL_DAMAGE_ZONE:
			effect = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE;
			break;
		case SE_SKILL_SAFE_ZONE:
			effect = CInstanceBase::EFFECT_SKILL_SAFE_ZONE;
			break;

		case SE_METEOR:
			effect = CInstanceBase::EFFECT_METEOR;
			break;
		case SE_BEAD_RAIN:
			effect = CInstanceBase::EFFECT_BEAD_RAIN;
			break;
		case SE_FALL_ROCK:
			effect = CInstanceBase::EFFECT_FALL_ROCK;
			break;
		case SE_ARROW_RAIN:
			effect = CInstanceBase::EFFECT_ARROW_RAIN;
			break;
		case SE_HORSE_DROP:
			effect = CInstanceBase::EFFECT_HORSE_DROP;
			break;
		case SE_EGG_DROP:
			effect = CInstanceBase::EFFECT_EGG_DROP;
			break;

		case SE_SKILL_DAMAGE_ZONE_BIG:
			effect = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_BIG;
			break;
		case SE_SKILL_DAMAGE_ZONE_MIDDLE:
			effect = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_MIDDLE;
			break;
		case SE_SKILL_DAMAGE_ZONE_SMALL:
			effect = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_SMALL;
			break;

		case SE_SKILL_SAFE_ZONE_BIG:
			effect = CInstanceBase::EFFECT_SKILL_SAFE_ZONE_BIG;
			break;
		case SE_SKILL_SAFE_ZONE_MIDDLE:
			effect = CInstanceBase::EFFECT_SKILL_SAFE_ZONE_MIDDLE;
			break;
		case SE_SKILL_SAFE_ZONE_SMALL:
			effect = CInstanceBase::EFFECT_SKILL_SAFE_ZONE_SMALL;
			break;

		default:
			TraceError("TPacketGCSpecialZodiacEffect.type::Cannot packet:: %d", kSpecialZodiacEffect.type);
			break;
	}

	if (-1 != effect)
	{
		CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(kSpecialZodiacEffect.vid);
		if (pInstance)
		{
			pInstance->AttachSpecialZodiacEffect(effect, kSpecialZodiacEffect.x, kSpecialZodiacEffect.y);
		}
	}

	uint32_t effect2 = -1;
	switch (kSpecialZodiacEffect.type2)
	{
		case SE_SKILL_DAMAGE_ZONE:
			effect2 = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE;
			break;
		case SE_SKILL_SAFE_ZONE:
			effect2 = CInstanceBase::EFFECT_SKILL_SAFE_ZONE;
			break;

		case SE_METEOR:
			effect2 = CInstanceBase::EFFECT_METEOR;
			break;
		case SE_BEAD_RAIN:
			effect2 = CInstanceBase::EFFECT_BEAD_RAIN;
			break;
		case SE_FALL_ROCK:
			effect2 = CInstanceBase::EFFECT_FALL_ROCK;
			break;
		case SE_ARROW_RAIN:
			effect2 = CInstanceBase::EFFECT_ARROW_RAIN;
			break;
		case SE_HORSE_DROP:
			effect2 = CInstanceBase::EFFECT_HORSE_DROP;
			break;
		case SE_EGG_DROP:
			effect2 = CInstanceBase::EFFECT_EGG_DROP;
			break;

		case SE_SKILL_DAMAGE_ZONE_BIG:
			effect2 = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_BIG;
			break;
		case SE_SKILL_DAMAGE_ZONE_MIDDLE:
			effect2 = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_MIDDLE;
			break;
		case SE_SKILL_DAMAGE_ZONE_SMALL:
			effect2 = CInstanceBase::EFFECT_SKILL_DAMAGE_ZONE_SMALL;
			break;

		case SE_SKILL_SAFE_ZONE_BIG:
			effect2 = CInstanceBase::EFFECT_SKILL_SAFE_ZONE_BIG;
			break;
		case SE_SKILL_SAFE_ZONE_MIDDLE:
			effect2 = CInstanceBase::EFFECT_SKILL_SAFE_ZONE_MIDDLE;
			break;
		case SE_SKILL_SAFE_ZONE_SMALL:
			effect2 = CInstanceBase::EFFECT_SKILL_SAFE_ZONE_SMALL;
			break;
	}

	if (-1 != effect2)
	{
		CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(kSpecialZodiacEffect.vid);
		if (pInstance)
		{
			pInstance->AttachSpecialZodiacEffect(effect2, kSpecialZodiacEffect.x, kSpecialZodiacEffect.y);
		}
	}

	return true;
}
#endif

bool CPythonNetworkStream::RecvSpecificEffect()
{
	TPacketGCSpecificEffect kSpecificEffect;
	if (!AutoRecv(kSpecificEffect))
		return false;

	CInstanceBase * pInstance = CPythonCharacterManager::Instance().GetInstancePtr(kSpecificEffect.vid);
	//EFFECT_TEMP
	if (pInstance)
	{
		CInstanceBase::RegisterEffect(CInstanceBase::EFFECT_TEMP, "", kSpecificEffect.effect_file, false);
		pInstance->AttachSpecialEffect(CInstanceBase::EFFECT_TEMP);
	}

	return true;
}

bool CPythonNetworkStream::RecvDragonSoulRefine()
{
	TPacketGCDragonSoulRefine kDragonSoul;

	if (!AutoRecv(kDragonSoul))
		return false;

	switch (kDragonSoul.bSubType)
	{
#if defined(ENABLE_DS_CHANGE_ATTR)
	case DS_SUB_HEADER_OPEN:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_DragonSoulRefineWindow_Open", Py_BuildValue("(i)", DS_REFINE_TYPE_GRADE));
		break;

	case DS_SUB_HEADER_OPEN_CHANGE_ATTR:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_DragonSoulRefineWindow_Open", Py_BuildValue("(i)", DS_REFINE_TYPE_CHANGE_ATTR));
		break;
#else
	case DS_SUB_HEADER_OPEN:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_DragonSoulRefineWindow_Open", Py_BuildValue("()"));
		break;
#endif

	case DS_SUB_HEADER_REFINE_FAIL:
	case DS_SUB_HEADER_REFINE_FAIL_MAX_REFINE:
	case DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL:
	case DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MONEY:
	case DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL:
	case DS_SUB_HEADER_REFINE_FAIL_TOO_MUCH_MATERIAL:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_DragonSoulRefineWindow_RefineFail", Py_BuildValue("(iii)",
			kDragonSoul.bSubType, kDragonSoul.Pos.window_type, kDragonSoul.Pos.cell));
		break;
	case DS_SUB_HEADER_REFINE_SUCCEED:
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "BINARY_DragonSoulRefineWindow_RefineSucceed",
				Py_BuildValue("(ii)", kDragonSoul.Pos.window_type, kDragonSoul.Pos.cell));
		break;
	}

	return true;
}

#ifdef ENABLE_DS_SET
bool CPythonNetworkStream::RecvDSTablePacket() {
	TPacketDSTable p;
	if (!Recv(sizeof(TPacketDSTable), &p))
		return false;
	
	return CPythonItem::Instance().SetDSTable(p);
}
#endif

#ifdef ENABLE_FISH_EVENT
bool CPythonNetworkStream::SendFishBoxUse(BYTE bWindow, WORD wCell)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGFishEvent packetFishEvent;
	packetFishEvent.bHeader = HEADER_CG_FISH_EVENT_SEND;
	packetFishEvent.bSubheader = FISH_EVENT_SUBHEADER_BOX_USE;

	if (!Send(sizeof(TPacketCGFishEvent), &packetFishEvent))
	{
		Tracef("SendFishEventPacket Error\n");
		return false;
	}
	
	if (!Send(sizeof(bWindow), &bWindow))
	{
		Tracef("SendFishBoxUse Error\n");
		return false;
	}
	
	if (!Send(sizeof(wCell), &wCell))
	{
		Tracef("SendFishBoxUse Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::SendFishShapeAdd(BYTE bPos)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGFishEvent packetFishEvent;
	packetFishEvent.bHeader = HEADER_CG_FISH_EVENT_SEND;
	packetFishEvent.bSubheader = FISH_EVENT_SUBHEADER_SHAPE_ADD;

	if (!Send(sizeof(TPacketCGFishEvent), &packetFishEvent))
	{
		Tracef("SendFishEventPacket Error\n");
		return false;
	}
	
	if (!Send(sizeof(BYTE), &bPos))
	{
		Tracef("SendFishShapeAdd Error\n");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RecvFishEventInfo()
{
	TPacketGCFishEventInfo fishEventPacket;

	if (!Recv(sizeof(fishEventPacket), &fishEventPacket))
		return false;

	switch (fishEventPacket.bSubheader)
	{
		case FISH_EVENT_SUBHEADER_BOX_USE:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameFishUse", Py_BuildValue("(ii)", fishEventPacket.dwFirstArg, fishEventPacket.dwSecondArg));
			break;
			
		case FISH_EVENT_SUBHEADER_SHAPE_ADD:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameFishAdd", Py_BuildValue("(ii)", fishEventPacket.dwFirstArg, fishEventPacket.dwSecondArg));
			break;
			
		case FISH_EVENT_SUBHEADER_GC_REWARD:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameFishReward", Py_BuildValue("(i)", fishEventPacket.dwFirstArg));
			break;
			
		case FISH_EVENT_SUBHEADER_GC_ENABLE:
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "MiniGameFishEvent", Py_BuildValue("(ii)", fishEventPacket.dwFirstArg, fishEventPacket.dwSecondArg));
			break;
	};

	return true;
}
#endif
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
#ifdef ENABLE_GEM_SYSTEM
bool CPythonNetworkStream::RecvGemShopOpen()
{
	TPacketGCGemShopOpen kPacket;
	if (!Recv(sizeof(kPacket), &kPacket))
		return false;
	
	CPythonPlayer::Instance().ClearGemShopItemVector();
	for (int i = 0; i < 9; ++i)
	{
		TGemShopItem GemItem;
		GemItem.slotIndex = kPacket.shopItems[i].slotIndex;
		GemItem.status = kPacket.shopItems[i].status;
		
		GemItem.dwVnum = kPacket.shopItems[i].dwVnum;
		GemItem.bCount = kPacket.shopItems[i].bCount;
		GemItem.dwPrice = kPacket.shopItems[i].dwPrice;

		CPythonPlayer::Instance().SetGemShopItemData(kPacket.shopItems[i].slotIndex, GemItem);
	}
	
	CPythonPlayer::Instance().SetGemShopRefreshTime(kPacket.nextRefreshTime);
	
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "OpenGemShop", Py_BuildValue("()"));

	return true;
}

bool CPythonNetworkStream::RecvGemShopRefresh()
{
	TPacketGCGemShopRefresh kPacket;
	if (!Recv(sizeof(kPacket), &kPacket))
		return false;
	
	CPythonPlayer::Instance().ClearGemShopItemVector();
	for (int i = 0; i < 9; ++i)
	{
		TGemShopItem GemItem;
		GemItem.slotIndex = kPacket.shopItems[i].slotIndex;
		GemItem.status = kPacket.shopItems[i].status;
		
		GemItem.dwVnum = kPacket.shopItems[i].dwVnum;
		GemItem.bCount = kPacket.shopItems[i].bCount;
		GemItem.dwPrice = kPacket.shopItems[i].dwPrice;

		CPythonPlayer::Instance().SetGemShopItemData(kPacket.shopItems[i].slotIndex, GemItem);
	}
	
	CPythonPlayer::Instance().SetGemShopRefreshTime(kPacket.nextRefreshTime);
	
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RefreshGemShop", Py_BuildValue("()"));

	return true;
}


bool CPythonNetworkStream::SendGemShopBuy(BYTE bPos)
{
	if (!__CanActMainInstance())
		return true;
	
	TPacketCGGemShop PacketGemShop;
	PacketGemShop.header = HEADER_CG_GEM_SHOP;
	PacketGemShop.subheader = GEM_SHOP_SUBHEADER_CG_BUY;
	PacketGemShop.pos = bPos;
	
	if (!Send(sizeof(TPacketCGGemShop), &PacketGemShop))
		return false;
	
	return SendSequence();
}

bool CPythonNetworkStream::SendGemShopAdd(BYTE bPos)
{
	if (!__CanActMainInstance())
		return true;
	
	TPacketCGGemShop PacketGemShop;
	PacketGemShop.header = HEADER_CG_GEM_SHOP;
	PacketGemShop.subheader = GEM_SHOP_SUBHEADER_CG_ADD;
	PacketGemShop.pos = bPos;
	
	if (!Send(sizeof(TPacketCGGemShop), &PacketGemShop))
		return false;
	
	return SendSequence();
}

bool CPythonNetworkStream::SendGemShopRefresh()
{
	if (!__CanActMainInstance())
		return true;
	
	TPacketCGGemShop PacketGemShop;
	PacketGemShop.header = HEADER_CG_GEM_SHOP;
	PacketGemShop.subheader = GEM_SHOP_SUBHEADER_CG_REFRESH;
	PacketGemShop.pos = 0; // Padding to maintain the strict 3-byte size
	
	if (!Send(sizeof(TPacketCGGemShop), &PacketGemShop))
		return false;
	
	return SendSequence();
}
#endif