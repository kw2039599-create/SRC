#include "StdAfx.h"
#include "PythonNetworkStream.h"
//#include "PythonNetworkDatagram.h"
#include "AccountConnector.h"
#include "PythonGuild.h"
#include "Test.h"

#include "AbstractPlayer.h"

static std::string gs_stServerInfo;
extern BOOL gs_bEmpireLanuageEnable;
std::list<std::string> g_kList_strCommand;

PyObject* netGetBettingGuildWarValue(PyObject* poSelf, PyObject* poArgs)
{
	char* szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	return Py_BuildValue("i", rkNetStream.EXPORT_GetBettingGuildWarValue(szName));
}

PyObject* netSetServerInfo(PyObject* poSelf, PyObject* poArgs)
{
	char* szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	gs_stServerInfo=szFileName;
	return Py_BuildNone();
}

PyObject* netGetServerInfo(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("s", gs_stServerInfo.c_str());
}

PyObject* netPreserveServerCommand(PyObject* poSelf, PyObject* poArgs)
{
	char* szLine;
	if (!PyTuple_GetString(poArgs, 0, &szLine))
		return Py_BuildException();

	g_kList_strCommand.push_back(szLine);

	return Py_BuildNone();
}

PyObject* netGetPreservedServerCommand(PyObject* poSelf, PyObject* poArgs)
{
	if (g_kList_strCommand.empty())
		return Py_BuildValue("s", "");

	std::string strCommand = g_kList_strCommand.front();
	g_kList_strCommand.pop_front();

	return Py_BuildValue("s", strCommand.c_str());
}

PyObject* netStartGame(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.StartGame();

	return Py_BuildNone();
}

PyObject* netIsTest(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", __IS_TEST_SERVER_MODE__);
}

PyObject* netWarp(PyObject* poSelf, PyObject* poArgs)
{
	int nX;
	if (!PyTuple_GetInteger(poArgs, 0, &nX))
		return Py_BuildException();

	int nY;
	if (!PyTuple_GetInteger(poArgs, 1, &nY))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.Warp(nX, nY);

	return Py_BuildNone();
}

PyObject* netLoadInsultList(PyObject* poSelf, PyObject* poArgs)
{
	char* szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	return Py_BuildValue("i", rkNetStream.LoadInsultList(szFileName));
}

PyObject* netUploadMark(PyObject* poSelf, PyObject* poArgs)
{
	char* szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	return Py_BuildValue("i", rkNetStream.UploadMark(szFileName));
}

PyObject* netUploadSymbol(PyObject* poSelf, PyObject* poArgs)
{
	char* szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	return Py_BuildValue("i", rkNetStream.UploadSymbol(szFileName));
}

PyObject* netGetGuildID(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	return Py_BuildValue("i", rkNetStream.GetGuildID());
}

PyObject* netGetEmpireID(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	return Py_BuildValue("i", rkNetStream.GetEmpireID());
}

PyObject* netGetMainActorVID(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	return Py_BuildValue("i", rkNetStream.GetMainActorVID());
}

PyObject* netGetMainActorRace(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	return Py_BuildValue("i", rkNetStream.GetMainActorRace());
}

PyObject* netGetMainActorEmpire(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	return Py_BuildValue("i", rkNetStream.GetMainActorEmpire());
}

PyObject* netGetMainActorSkillGroup(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	return Py_BuildValue("i", rkNetStream.GetMainActorSkillGroup());
}

PyObject* netIsSelectedEmpire(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	return Py_BuildValue("i", rkNetStream.IsSelectedEmpire());
}

PyObject* netGetAccountCharacterSlotDataInteger(PyObject* poSelf, PyObject* poArgs)
{
	int nIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &nIndex))
		return Py_BuildException();

	int nType;
	if (!PyTuple_GetInteger(poArgs, 1, &nType))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	UINT uResult=rkNetStream.GetAccountCharacterSlotDatau(nIndex, nType);
	return Py_BuildValue("i", uResult);
}

PyObject* netGetAccountCharacterSlotDataString(PyObject* poSelf, PyObject* poArgs)
{
	int nIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &nIndex))
		return Py_BuildException();

	int nType;
	if (!PyTuple_GetInteger(poArgs, 1, &nType))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	return Py_BuildValue("s", rkNetStream.GetAccountCharacterSlotDataz(nIndex, nType));
}

// SUPPORT_BGM
PyObject* netGetFieldMusicFileName(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	return Py_BuildValue("s", rkNetStream.GetFieldMusicFileName());
}

PyObject* netGetFieldMusicVolume(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	return Py_BuildValue("f", rkNetStream.GetFieldMusicVolume());
}
// END_OF_SUPPORT_BGM

PyObject* netSetPhaseWindow(PyObject* poSelf, PyObject* poArgs)
{
	int ePhaseWnd;
	if (!PyTuple_GetInteger(poArgs, 0, &ePhaseWnd))
		return Py_BuildException();

	PyObject* poPhaseWnd;
	if (!PyTuple_GetObject(poArgs, 1, &poPhaseWnd))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SetPhaseWindow(ePhaseWnd, poPhaseWnd);
	return Py_BuildNone();
}

PyObject* netClearPhaseWindow(PyObject* poSelf, PyObject* poArgs)
{
	int ePhaseWnd;
	if (!PyTuple_GetInteger(poArgs, 0, &ePhaseWnd))
		return Py_BuildException();

	PyObject* poPhaseWnd;
	if (!PyTuple_GetObject(poArgs, 1, &poPhaseWnd))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.ClearPhaseWindow(ePhaseWnd, poPhaseWnd);
	return Py_BuildNone();
}

PyObject* netSetServerCommandParserWindow(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* poPhaseWnd;
	if (!PyTuple_GetObject(poArgs, 0, &poPhaseWnd))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SetServerCommandParserWindow(poPhaseWnd);
	return Py_BuildNone();
}

PyObject* netSetAccountConnectorHandler(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* poPhaseWnd;
	if (!PyTuple_GetObject(poArgs, 0, &poPhaseWnd))
		return Py_BuildException();

	CAccountConnector & rkAccountConnector = CAccountConnector::Instance();
	rkAccountConnector.SetHandler(poPhaseWnd);
	return Py_BuildNone();
}

PyObject* netSetHandler(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* poHandler;

	if (!PyTuple_GetObject(poArgs, 0, &poHandler))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SetHandler(poHandler);
	return Py_BuildNone();
}

PyObject* netSetTCPRecvBufferSize(PyObject* poSelf, PyObject* poArgs)
{
	int bufSize;
	if (!PyTuple_GetInteger(poArgs, 0, &bufSize))
		return Py_BuildException();

	if (bufSize < (2 * 1024 * 1024))
		bufSize = (2 * 1024 * 1024);

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SetRecvBufferSize(bufSize);
	return Py_BuildNone();
}

PyObject* netSetTCPSendBufferSize(PyObject* poSelf, PyObject* poArgs)
{
	int bufSize;
	if (!PyTuple_GetInteger(poArgs, 0, &bufSize))
		return Py_BuildException();

	if (bufSize < (256 * 1024))
		bufSize = (256 * 1024);

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SetSendBufferSize(bufSize);
	return Py_BuildNone();
}

PyObject* netSetUDPRecvBufferSize(PyObject* poSelf, PyObject* poArgs)
{
	int bufSize;
	if (!PyTuple_GetInteger(poArgs, 0, &bufSize))
		return Py_BuildException();

	//CPythonNetworkDatagram::Instance().SetRecvBufferSize(bufSize);
	return Py_BuildNone();
}

PyObject* netSetMarkServer(PyObject* poSelf, PyObject* poArgs)
{
	char* szAddr;
	if (!PyTuple_GetString(poArgs, 0, &szAddr))
		return Py_BuildException();

	int port;
	if (!PyTuple_GetInteger(poArgs, 1, &port))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SetMarkServer(szAddr, port);
	return Py_BuildNone();
}

PyObject* netConnectTCP(PyObject* poSelf, PyObject* poArgs)
{
	char* szAddr;
	if (!PyTuple_GetString(poArgs, 0, &szAddr))
		return Py_BuildException();

	int port;
	if (!PyTuple_GetInteger(poArgs, 1, &port))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.ConnectLoginServer(szAddr, port);
	return Py_BuildNone();
}

PyObject* netConnectUDP(PyObject* poSelf, PyObject* poArgs)
{
	char * c_szIP;
	if (!PyTuple_GetString(poArgs, 0, &c_szIP))
		return Py_BuildException();
	int iPort;
	if (!PyTuple_GetInteger(poArgs, 1, &iPort))
		return Py_BuildException();

	//CPythonNetworkDatagram::Instance().SetConnection(c_szIP, iPort);
	return Py_BuildNone();
}

PyObject* netConnectToAccountServer(PyObject* poSelf, PyObject* poArgs)
{
	char* addr;
	if (!PyTuple_GetString(poArgs, 0, &addr))
		return Py_BuildException();

	int port;
	if (!PyTuple_GetInteger(poArgs, 1, &port))
		return Py_BuildException();

	char* account_addr;
	if (!PyTuple_GetString(poArgs, 2, &account_addr))
		return Py_BuildException();

	int account_port;
	if (!PyTuple_GetInteger(poArgs, 3, &account_port))
		return Py_BuildException();

	CAccountConnector & rkAccountConnector = CAccountConnector::Instance();
	rkAccountConnector.Connect(addr, port, account_addr, account_port);
	return Py_BuildNone();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

PyObject* netSetLoginInfo(PyObject* poSelf, PyObject* poArgs)
{
	char* szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	char* szPwd;
	if (!PyTuple_GetString(poArgs, 1, &szPwd))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	CAccountConnector & rkAccountConnector = CAccountConnector::Instance();
	rkNetStream.SetLoginInfo(szName, szPwd);
	rkAccountConnector.SetLoginInfo(szName, szPwd);
	return Py_BuildNone();
}

PyObject* netSetOfflinePhase(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SetOffLinePhase();
	return Py_BuildNone();
}

PyObject* netSendSelectEmpirePacket(PyObject* poSelf, PyObject* poArgs)
{
	int iEmpireIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iEmpireIndex))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendSelectEmpirePacket(iEmpireIndex);
	return Py_BuildNone();
}

PyObject* netSendLoginPacket(PyObject* poSelf, PyObject* poArgs)
{
	char* szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	char* szPwd;
	if (!PyTuple_GetString(poArgs, 1, &szPwd))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendLoginPacket(szName, szPwd);
	return Py_BuildNone();
}

PyObject* netDirectEnter(PyObject* poSelf, PyObject* poArgs)
{
	int nChrSlot;
	if (!PyTuple_GetInteger(poArgs, 0, &nChrSlot))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.ConnectGameServer(nChrSlot);
	return Py_BuildNone();
}

PyObject* netSendSelectCharacterPacket(PyObject* poSelf, PyObject* poArgs)
{
	int Index;
	if (!PyTuple_GetInteger(poArgs, 0, &Index))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendSelectCharacterPacket((BYTE) Index);
	return Py_BuildNone();
}

PyObject* netSendChangeNamePacket(PyObject* poSelf, PyObject* poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();
	char* szName;
	if (!PyTuple_GetString(poArgs, 1, &szName))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendChangeNamePacket((BYTE)iIndex, szName);
	return Py_BuildNone();
}

PyObject* netEnableChatInsultFilter(PyObject* poSelf, PyObject* poArgs)
{
	int nEnable;
	if (!PyTuple_GetInteger(poArgs, 0, &nEnable))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.EnableChatInsultFilter(nEnable ? true : false);
	return Py_BuildNone();
}

PyObject* netIsChatInsultIn(PyObject* poSelf, PyObject* poArgs)
{
	char* szMsg;
	if (!PyTuple_GetString(poArgs, 0, &szMsg))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	return Py_BuildValue("i", rkNetStream.IsChatInsultIn(szMsg));
}

PyObject* netIsInsultIn(PyObject* poSelf, PyObject* poArgs)
{
	char* szMsg;
	if (!PyTuple_GetString(poArgs, 0, &szMsg))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	return Py_BuildValue("i", rkNetStream.IsInsultIn(szMsg));
}

PyObject* netSendWhisperPacket(PyObject* poSelf, PyObject* poArgs)
{
	char* szName;
	char* szLine;

	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	if (!PyTuple_GetString(poArgs, 1, &szLine))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendWhisperPacket(szName, szLine);
	return Py_BuildNone();
}

PyObject* netSendMobileMessagePacket(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildNone();
}

PyObject* netSendCharacterPositionPacket(PyObject* poSelf, PyObject* poArgs)
{
	int iPosition;
	if (!PyTuple_GetInteger(poArgs, 0, &iPosition))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendCharacterPositionPacket((BYTE) iPosition);
	return Py_BuildNone();
}

PyObject* netSendChatPacket(PyObject* poSelf, PyObject* poArgs)
{
	char* szLine;
	if (!PyTuple_GetString(poArgs, 0, &szLine))
		return Py_BuildException();
	int iType;
	if (!PyTuple_GetInteger(poArgs, 1, &iType))
	{
		iType = CHAT_TYPE_TALKING;
	}

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendChatPacket(szLine, iType);
	return Py_BuildNone();
}

PyObject* netSendEmoticon(PyObject* poSelf, PyObject* poArgs)
{
	int eEmoticon;
	if (!PyTuple_GetInteger(poArgs, 0, &eEmoticon))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendEmoticon(eEmoticon);
	return Py_BuildNone();
}

PyObject* netSendCreateCharacterPacket(PyObject* poSelf, PyObject* poArgs)
{
	int index;
	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	char* name;
	if (!PyTuple_GetString(poArgs, 1, &name))
		return Py_BuildException();

	int job;
	if (!PyTuple_GetInteger(poArgs, 2, &job))
		return Py_BuildException();

	int shape;
	if (!PyTuple_GetInteger(poArgs, 3, &shape))
		return Py_BuildException();

	int stat1;
	if (!PyTuple_GetInteger(poArgs, 4, &stat1))
		return Py_BuildException();
	int stat2;
	if (!PyTuple_GetInteger(poArgs, 5, &stat2))
		return Py_BuildException();
	int stat3;
	if (!PyTuple_GetInteger(poArgs, 6, &stat3))
		return Py_BuildException();
	int stat4;
	if (!PyTuple_GetInteger(poArgs, 7, &stat4))
		return Py_BuildException();

	if (index<0 && index>3)
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendCreateCharacterPacket((BYTE) index, name, (BYTE) job, (BYTE) shape, stat1, stat2, stat3, stat4);
	return Py_BuildNone();
}

PyObject* netSendDestroyCharacterPacket(PyObject* poSelf, PyObject* poArgs)
{
	int index;
	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	char * szPrivateCode;
	if (!PyTuple_GetString(poArgs, 1, &szPrivateCode))
		return Py_BuildException();

	if (index<0 && index>3)
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendDestroyCharacterPacket((BYTE) index, szPrivateCode);
	return Py_BuildNone();
}

PyObject* netSendEnterGamePacket(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendEnterGame();
	return Py_BuildNone();
}

PyObject* netOnClickPacket(PyObject* poSelf, PyObject* poArgs)
{
	int index;
	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendOnClickPacket(index);

	return Py_BuildNone();
}

PyObject* netSendItemUsePacket(PyObject* poSelf, PyObject* poArgs)
{
	TItemPos Cell;
	switch (PyTuple_Size(poArgs))
	{
	case 1:
		if (!PyTuple_GetInteger(poArgs, 0, &Cell.cell))
			return Py_BuildException();
		break;
	case 2:
		if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
			return Py_BuildException();
		break;
	default:
		return Py_BuildException();
	}

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendItemUsePacket(Cell);
	return Py_BuildNone();
}

#ifdef ENABLE_CHEST_OPEN_RENEWAL
PyObject* netSendOpenChestPacket(PyObject* poSelf, PyObject* poArgs)
{
	TItemPos Cell;
	int iOpenCount;

	switch (PyTuple_Size(poArgs))
	{
		case 2: // Legacy fallback: (cell, count)
			if (!PyTuple_GetInteger(poArgs, 0, &Cell.cell))
				return Py_BuildException();
			if (!PyTuple_GetInteger(poArgs, 1, &iOpenCount))
				return Py_BuildException();
			break;
		case 3: // Secure mapping: (window_type, cell, count)
			if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
				return Py_BuildException();
			if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
				return Py_BuildException();
			if (!PyTuple_GetInteger(poArgs, 2, &iOpenCount))
				return Py_BuildException();
			break;
		default:
			return Py_BuildException();
	}

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendOpenChestPacket(Cell, iOpenCount);
	return Py_BuildNone();
}
#endif

PyObject* netSendItemUseToItemPacket(PyObject* poSelf, PyObject* poArgs)
{
	TItemPos SourceCell;
	TItemPos TargetCell;
	switch (PyTuple_Size(poArgs))
	{
	case 2:
		if (!PyTuple_GetInteger(poArgs, 0, &SourceCell.cell))
			return Py_BuildException();

		if (!PyTuple_GetInteger(poArgs, 1, &TargetCell.cell))
			return Py_BuildException();
		break;
	case 4:
		if (!PyTuple_GetByte(poArgs, 0, &SourceCell.window_type))
			return Py_BuildException();

		if (!PyTuple_GetInteger(poArgs, 1, &SourceCell.cell))
			return Py_BuildException();

		if (!PyTuple_GetByte(poArgs, 2, &TargetCell.window_type))
			return Py_BuildException();

		if (!PyTuple_GetInteger(poArgs, 3, &TargetCell.cell))
			return Py_BuildException();
		break;
	default:
		return Py_BuildException();
	}
	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendItemUseToItemPacket(SourceCell, TargetCell);
	return Py_BuildNone();
}

PyObject* netSendItemDropPacket(PyObject* poSelf, PyObject* poArgs)
{
	TItemPos Cell;
	switch (PyTuple_Size(poArgs))
	{
	case 1:
		if (!PyTuple_GetInteger(poArgs, 0, &Cell.cell))
			return Py_BuildException();
		break;
	case 2:
		if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
			return Py_BuildException();
		break;
	default:
		return Py_BuildException();
	}

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendItemDropPacket(Cell, 0);
	return Py_BuildNone();
}

PyObject* netSendItemDropPacketNew(PyObject* poSelf, PyObject* poArgs)
{
	TItemPos Cell;
	int count;
	switch (PyTuple_Size(poArgs))
	{
	case 2:
		if (!PyTuple_GetInteger(poArgs, 0, &Cell.cell))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &count))
			return Py_BuildException();

		break;
	case 3:
		if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 2, &count))
			return Py_BuildException();

		break;
	default:
		return Py_BuildException();
	}
	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendItemDropPacketNew(Cell, 0, count);
	return Py_BuildNone();
}

PyObject* netSendElkDropPacket(PyObject* poSelf, PyObject* poArgs)
{
	int iElk;
	if (!PyTuple_GetInteger(poArgs, 0, &iElk))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendItemDropPacket(TItemPos(RESERVED_WINDOW, 0), (DWORD) iElk);
	return Py_BuildNone();
}

PyObject* netSendGoldDropPacketNew(PyObject* poSelf, PyObject* poArgs)
{
	int iElk;
	if (!PyTuple_GetInteger(poArgs, 0, &iElk))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendItemDropPacketNew(TItemPos (RESERVED_WINDOW, 0), (DWORD) iElk, 0);
	return Py_BuildNone();
}

#if defined(ENABLE_CHEQUE_SYSTEM) && !defined(DISABLE_CHEQUE_DROP)
PyObject* netSendChequeDropPacketNew(PyObject* poSelf, PyObject* poArgs)
{
	int Cheque;
	if (!PyTuple_GetInteger(poArgs, 0, &Cheque))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendItemDropPacketNew(TItemPos(RESERVED_WINDOW, 0), 0, 0, (DWORD)Cheque);
	return Py_BuildNone();
}
#endif

PyObject* netSendItemMovePacket(PyObject* poSelf, PyObject* poArgs)
{
	TItemPos Cell;
	TItemPos ChangeCell;
	int num;

	switch (PyTuple_Size(poArgs))
	{
	case 3:
		if (!PyTuple_GetInteger(poArgs, 0, &Cell.cell))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &ChangeCell.cell))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 2, &num))
			return Py_BuildException();
		break;
	case 5:
		{
			if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
				return Py_BuildException();
			if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
				return Py_BuildException();
			if (!PyTuple_GetByte(poArgs, 2, &ChangeCell.window_type))
				return Py_BuildException();
			if (!PyTuple_GetInteger(poArgs, 3, &ChangeCell.cell))
				return Py_BuildException();
			if (!PyTuple_GetInteger(poArgs, 4, &num))
				return Py_BuildException();
		}
		break;
	default:
		return Py_BuildException();
	}

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendItemMovePacket(Cell, ChangeCell, (BYTE) num);
	return Py_BuildNone();
}

PyObject* netSendItemPickUpPacket(PyObject* poSelf, PyObject* poArgs)
{
	int vid;
	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendItemPickUpPacket(vid);
	return Py_BuildNone();
}

#ifdef ENABLE_SORT_INVEN
PyObject* netSortInven(PyObject* poSelf, PyObject* poArgs)
{
	int window_type;
	int option;

	// Grab the first argument (window type)
	if (!PyTuple_GetInteger(poArgs, 0, &window_type))
		return Py_BuildException();
	
	// Grab the second argument (sort option)
	if (!PyTuple_GetInteger(poArgs, 1, &option))
		return Py_BuildException();
	
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SortInven(window_type, option); // Pass both to the network stream
	
	return Py_BuildNone();
}
#endif

PyObject* netSendGiveItemPacket(PyObject* poSelf, PyObject* poArgs)
{
	int iTargetVID;
	TItemPos Cell;
	int iItemCount;
	switch (PyTuple_Size(poArgs))
	{
	case 3:
		if (!PyTuple_GetInteger(poArgs, 0, &iTargetVID))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 2, &iItemCount))
			return Py_BuildException();
		break;
	case 4:
		if (!PyTuple_GetInteger(poArgs, 0, &iTargetVID))
			return Py_BuildException();
		if (!PyTuple_GetByte(poArgs, 1, &Cell.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 2, &Cell.cell))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 3, &iItemCount))
			return Py_BuildException();
		break;
	default:
		break;
	}

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendGiveItemPacket(iTargetVID, Cell, iItemCount);
	return Py_BuildNone();
}

PyObject* netSendShopEndPacket(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendShopEndPacket();
	return Py_BuildNone();
}

PyObject* netSendShopBuyPacket(PyObject* poSelf, PyObject* poArgs)
{
	int iCount;
	if (!PyTuple_GetInteger(poArgs, 0, &iCount))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendShopBuyPacket(iCount);
	return Py_BuildNone();
}

PyObject* netSendShopSellPacket(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotNumber;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotNumber))
		return Py_BuildException();
	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendShopSellPacket(iSlotNumber);
	return Py_BuildNone();
}

PyObject* netSendShopSellPacketNew(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotNumber;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotNumber))
		return Py_BuildException();
	int iCount;
	if (!PyTuple_GetInteger(poArgs, 1, &iCount))
		return Py_BuildException();
#ifdef ENABLE_SPECIAL_STORAGE
	int iType;
	if (!PyTuple_GetInteger(poArgs, 2, &iType))
		return Py_BuildException();
#endif
	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
#ifdef ENABLE_SPECIAL_STORAGE
	rkNetStream.SendShopSellPacketNew(iSlotNumber, iCount, iType);
#else
	rkNetStream.SendShopSellPacketNew(iSlotNumber, iCount);
#endif
	return Py_BuildNone();
}

PyObject* netSendExchangeStartPacket(PyObject* poSelf, PyObject* poArgs)
{
	int vid;
	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendExchangeStartPacket(vid);
	return Py_BuildNone();
}

PyObject* netSendExchangeElkAddPacket(PyObject* poSelf, PyObject* poArgs)
{
	int iElk;
	if (!PyTuple_GetInteger(poArgs, 0, &iElk))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendExchangeElkAddPacket(iElk);
	return Py_BuildNone();
}

#if defined(ENABLE_PARTY_MATCH)
#include "PythonPlayer.h"
PyObject* netSendPartyMatch(PyObject* poSelf, PyObject* poArgs)
{
	int subheader;
	if (!PyTuple_GetInteger(poArgs, 0, &subheader))
		return Py_BuildException();

	int targetPID;
	if (!PyTuple_GetInteger(poArgs, 1, &targetPID))
		return Py_BuildException();

	int mapIndex;
	if (!PyTuple_GetInteger(poArgs, 2, &mapIndex))
		return Py_BuildException();

	char* szDesc = NULL; // FIXED: Initialize as NULL instead of ""
	
	// Safely grab the 4th argument (Description) if Python sends it
	if (PyTuple_Size(poArgs) > 3)
	{
		PyTuple_GetString(poArgs, 3, &szDesc);
	}

	// FIXED: If szDesc is NULL, pass an empty string literal safely
	CPythonNetworkStream::Instance().PartyMatch(subheader, targetPID, mapIndex, szDesc ? szDesc : "");
	
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_CHEQUE_SYSTEM
PyObject* netSendExchangeChequeAddPacket(PyObject* poSelf, PyObject* poArgs)
{
	int cheque;
	if (!PyTuple_GetInteger(poArgs, 0, &cheque))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendExchangeChequeAddPacket(cheque);
	return Py_BuildNone();
}
#endif

PyObject* netSendExchangeItemAddPacket(PyObject* poSelf, PyObject* poArgs)
{
	BYTE bWindowType;
	if (!PyTuple_GetInteger(poArgs, 0, &bWindowType))
		return Py_BuildException();
	WORD wSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &wSlotIndex))
		return Py_BuildException();
	int iDisplaySlotIndex;
	if (!PyTuple_GetInteger(poArgs, 2, &iDisplaySlotIndex))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendExchangeItemAddPacket(TItemPos(bWindowType, wSlotIndex), iDisplaySlotIndex);
	return Py_BuildNone();
}

PyObject* netSendExchangeItemDelPacket(PyObject* poSelf, PyObject* poArgs)
{
	int pos;
	if (!PyTuple_GetInteger(poArgs, 0, &pos))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendExchangeItemDelPacket((BYTE) pos);
	return Py_BuildNone();
}

PyObject* netSendExchangeAcceptPacket(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendExchangeAcceptPacket();
	return Py_BuildNone();
}

PyObject* netSendExchangeExitPacket(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SendExchangeExitPacket();
	return Py_BuildNone();
}

PyObject* netExitApplication(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.ExitApplication();
	return Py_BuildNone();
}

PyObject* netExitGame(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.ExitGame();
	return Py_BuildNone();
}

PyObject* netLogOutGame(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.LogOutGame();
	return Py_BuildNone();
}

PyObject* netDisconnect(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.SetOffLinePhase();
	rkNetStream.Disconnect();

	return Py_BuildNone();
}

#if defined(__BL_67_ATTR__)
PyObject* netSend67AttrPacket(PyObject* poSelf, PyObject* poArgs)
{
	int iMaterialCount;
	if (!PyTuple_GetInteger(poArgs, 0, &iMaterialCount))
		return Py_BuildException();

	int iSupportCount;
	if (!PyTuple_GetInteger(poArgs, 1, &iSupportCount))
		return Py_BuildException();

	int iSupportPos;
	if (!PyTuple_GetInteger(poArgs, 2, &iSupportPos))
		return Py_BuildException();

	int iItemPos;
	if (!PyTuple_GetInteger(poArgs, 3, &iItemPos))
		return Py_BuildException();

	CPythonNetworkStream::Instance().Send67AttrPacket(iMaterialCount, iSupportCount, iSupportPos, iItemPos);
	return Py_BuildNone();
}

PyObject* netSend67AttrClosePacket(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().Send67AttrClosePacket();
	return Py_BuildNone();
}
#endif

PyObject* netIsConnect(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	return Py_BuildValue("i", rkNetStream.IsOnline());
}

PyObject* netToggleGameDebugInfo(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.ToggleGameDebugInfo();
	return Py_BuildNone();
}

PyObject* netRegisterEmoticonString(PyObject* poSelf, PyObject* poArgs)
{
	char * pcEmoticonString;
	if (!PyTuple_GetString(poArgs, 0, &pcEmoticonString))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream=CPythonNetworkStream::Instance();
	rkNetStream.RegisterEmoticonString(pcEmoticonString);
	return Py_BuildNone();
}

#ifdef ENABLE_MESSENGER_BLOCK
PyObject* netSendMessengerAddBlockByVIDPacket(PyObject* poSelf, PyObject* poArgs)
{
	int vid;
	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendMessengerAddBlockByVIDPacket(vid);
	
	return Py_BuildNone();
}

PyObject* netSendMessengerAddBlockByNamePacket(PyObject* poSelf, PyObject* poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendMessengerAddBlockByNamePacket(szName);
	
	return Py_BuildNone();
}

PyObject* netSendMessengerBlockRemovePacket(PyObject* poSelf, PyObject* poArgs)
{
	char * szKey;
	if (!PyTuple_GetString(poArgs, 0, &szKey))
		return Py_BuildException();
	char * szName;
	if (!PyTuple_GetString(poArgs, 1, &szName))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendMessengerRemoveBlockPacket(szKey, szName);
	
	return Py_BuildNone();
}
#endif

PyObject* netSendMessengerAddByVIDPacket(PyObject* poSelf, PyObject* poArgs)
{
	int vid;
	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendMessengerAddByVIDPacket(vid);

	return Py_BuildNone();
}

PyObject* netSendMessengerAddByNamePacket(PyObject* poSelf, PyObject* poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendMessengerAddByNamePacket(szName);

	return Py_BuildNone();
}

PyObject* netSendMessengerRemovePacket(PyObject* poSelf, PyObject* poArgs)
{
	char * szKey;
	if (!PyTuple_GetString(poArgs, 0, &szKey))
		return Py_BuildException();
	char * szName;
	if (!PyTuple_GetString(poArgs, 1, &szName))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendMessengerRemovePacket(szKey, szName);

	return Py_BuildNone();
}

PyObject* netSendPartyInvitePacket(PyObject* poSelf, PyObject* poArgs)
{
	int vid;
	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendPartyInvitePacket(vid);

	return Py_BuildNone();
}

PyObject* netSendPartyInviteAnswerPacket(PyObject* poSelf, PyObject* poArgs)
{
	int vid;
	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();
	int answer;
	if (!PyTuple_GetInteger(poArgs, 1, &answer))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendPartyInviteAnswerPacket(vid, answer);

	return Py_BuildNone();
}

PyObject* netSendPartyExitPacket(PyObject* poSelf, PyObject* poArgs)
{
	IAbstractPlayer& rPlayer=IAbstractPlayer::GetSingleton();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();

	DWORD dwVID = rPlayer.GetMainCharacterIndex();
	DWORD dwPID;
	if (rPlayer.PartyMemberVIDToPID(dwVID, &dwPID))
		rns.SendPartyRemovePacket(dwPID);

	return Py_BuildNone();
}

PyObject* netSendPartyRemovePacketPID(PyObject* poSelf, PyObject* poArgs)
{
	int pid;
	if (!PyTuple_GetInteger(poArgs, 0, &pid))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendPartyRemovePacket(pid);

	return Py_BuildNone();
}

PyObject* netSendPartyRemovePacketVID(PyObject* poSelf, PyObject* poArgs)
{
	int vid;
	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();

	IAbstractPlayer& rPlayer=IAbstractPlayer::GetSingleton();
	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	DWORD dwPID;
	if (rPlayer.PartyMemberVIDToPID(vid, &dwPID))
		rns.SendPartyRemovePacket(dwPID);

	return Py_BuildNone();
}

PyObject* netSendPartySetStatePacket(PyObject* poSelf, PyObject* poArgs)
{
	int iVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVID))
		return Py_BuildException();
	int iState;
	if (!PyTuple_GetInteger(poArgs, 1, &iState))
		return Py_BuildException();
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 2, &iFlag))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendPartySetStatePacket(iVID, iState, iFlag);

	return Py_BuildNone();
}

PyObject* netSendPartyUseSkillPacket(PyObject* poSelf, PyObject* poArgs)
{
	int iSkillIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillIndex))
		return Py_BuildException();
	int iVID;
	if (!PyTuple_GetInteger(poArgs, 1, &iVID))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendPartyUseSkillPacket(iSkillIndex, iVID);

	return Py_BuildNone();
}

PyObject* netSendPartyParameterPacket(PyObject* poSelf, PyObject* poArgs)
{
	int iMode;
	if (!PyTuple_GetInteger(poArgs, 0, &iMode))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendPartyParameterPacket(iMode);

	return Py_BuildNone();
}

PyObject* netSendSafeboxSaveMoneyPacket(PyObject* poSelf, PyObject* poArgs)
{
	int iMoney;
	if (!PyTuple_GetInteger(poArgs, 0, &iMoney))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendSafeBoxMoneyPacket(SAFEBOX_MONEY_STATE_SAVE, iMoney);

	return Py_BuildNone();
}

PyObject* netSendSafeboxWithdrawMoneyPacket(PyObject* poSelf, PyObject* poArgs)
{
	int iMoney;
	if (!PyTuple_GetInteger(poArgs, 0, &iMoney))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendSafeBoxMoneyPacket(SAFEBOX_MONEY_STATE_WITHDRAW, iMoney);

	return Py_BuildNone();
}

PyObject* netSendSafeboxCheckinPacket(PyObject* poSelf, PyObject* poArgs)
{
	TItemPos InventoryPos;
	int iSafeBoxPos;

	switch (PyTuple_Size(poArgs))
	{
	case 2:
		InventoryPos.window_type = INVENTORY;
		if (!PyTuple_GetInteger(poArgs, 0, &InventoryPos.cell))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &iSafeBoxPos))
			return Py_BuildException();
		break;
	case 3:
		if (!PyTuple_GetInteger(poArgs, 0, &InventoryPos.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &InventoryPos.cell))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 2, &iSafeBoxPos))
			return Py_BuildException();
		break;

	}

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendSafeBoxCheckinPacket(InventoryPos, iSafeBoxPos);

	return Py_BuildNone();
}

PyObject* netSendSafeboxCheckoutPacket(PyObject* poSelf, PyObject* poArgs)
{
	int iSafeBoxPos;
	TItemPos InventoryPos;

	switch (PyTuple_Size(poArgs))
	{
	case 2:
		if (!PyTuple_GetInteger(poArgs, 0, &iSafeBoxPos))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &InventoryPos.cell))
			return Py_BuildException();
		break;
	case 3:
		if (!PyTuple_GetInteger(poArgs, 0, &iSafeBoxPos))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &InventoryPos.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 2, &InventoryPos.cell))
			return Py_BuildException();
		break;
	default:
		break;
	}

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendSafeBoxCheckoutPacket(iSafeBoxPos, InventoryPos);

	return Py_BuildNone();
}

PyObject* netSendSafeboxItemMovePacket(PyObject* poSelf, PyObject* poArgs)
{
	int iSourcePos;
	if (!PyTuple_GetInteger(poArgs, 0, &iSourcePos))
		return Py_BuildException();
	int iTargetPos;
	if (!PyTuple_GetInteger(poArgs, 1, &iTargetPos))
		return Py_BuildException();
	int iCount;
	if (!PyTuple_GetInteger(poArgs, 2, &iCount))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendSafeBoxItemMovePacket(iSourcePos, iTargetPos, iCount);

	return Py_BuildNone();
}

PyObject* netSendMallCheckoutPacket(PyObject* poSelf, PyObject* poArgs)
{
	int iMallPos;
	TItemPos InventoryPos;

	switch (PyTuple_Size(poArgs))
	{
	case 2:
		if (!PyTuple_GetInteger(poArgs, 0, &iMallPos))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &InventoryPos.cell))
			return Py_BuildException();
		break;
	case 3:
		if (!PyTuple_GetInteger(poArgs, 0, &iMallPos))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &InventoryPos.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 2, &InventoryPos.cell))
			return Py_BuildException();
		break;
	default:
		return Py_BuildException();
	}
	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendMallCheckoutPacket(iMallPos, InventoryPos);

	return Py_BuildNone();
}

#ifdef BOT_KILLER
PyObject* netSendBotKillerAnswer(PyObject* poSelf, PyObject* poArgs)
{
	int iX;
	if (!PyTuple_GetInteger(poArgs, 0, &iX)) return Py_BuildException();

	int iY;
	if (!PyTuple_GetInteger(poArgs, 1, &iY)) return Py_BuildException();

	int iDistance;
	if (!PyTuple_GetInteger(poArgs, 2, &iDistance)) return Py_BuildException();

	int iSessionKey;
	if (!PyTuple_GetInteger(poArgs, 3, &iSessionKey)) return Py_BuildException();

	CPythonNetworkStream::Instance().SendBotKillerAnswerPacket((WORD)iX, (WORD)iY, (WORD)iDistance, (DWORD)iSessionKey);
	return Py_BuildNone();
}
#endif

PyObject* netSendAnswerMakeGuildPacket(PyObject* poSelf, PyObject* poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendAnswerMakeGuildPacket(szName);

	return Py_BuildNone();
}

PyObject* netSendQuestInputStringPacket(PyObject* poSelf, PyObject* poArgs)
{
	char * szString;
	if (!PyTuple_GetString(poArgs, 0, &szString))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendQuestInputStringPacket(szString);

	return Py_BuildNone();
}

PyObject* netSendQuestConfirmPacket(PyObject* poSelf, PyObject* poArgs)
{
	int iAnswer;
	if (!PyTuple_GetInteger(poArgs, 0, &iAnswer))
		return Py_BuildException();
	int iPID;
	if (!PyTuple_GetInteger(poArgs, 1, &iPID))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendQuestConfirmPacket(iAnswer, iPID);

	return Py_BuildNone();
}

PyObject* netSendGuildAddMemberPacket(PyObject* poSelf, PyObject* poArgs)
{
	int iVID;
	if (!PyTuple_GetInteger(poArgs, 0, &iVID))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendGuildAddMemberPacket(iVID);

	return Py_BuildNone();
}

PyObject* netSendGuildRemoveMemberPacket(PyObject* poSelf, PyObject* poArgs)
{
	char * szKey;
	if (!PyTuple_GetString(poArgs, 0, &szKey))
		return Py_BuildException();

	CPythonGuild::TGuildMemberData * pGuildMemberData;
	if (!CPythonGuild::Instance().GetMemberDataPtrByName(szKey, &pGuildMemberData))
	{
		TraceError("netSendGuildRemoveMemberPacket(szKey=%s) - Can't Find Guild Member\n", szKey);
		return Py_BuildNone();
	}

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendGuildRemoveMemberPacket(pGuildMemberData->dwPID);

	return Py_BuildNone();
}

PyObject* netSendGuildChangeGradeNamePacket(PyObject* poSelf, PyObject* poArgs)
{
	int iGradeNumber;
	if (!PyTuple_GetInteger(poArgs, 0, &iGradeNumber))
		return Py_BuildException();
	char * szGradeName;
	if (!PyTuple_GetString(poArgs, 1, &szGradeName))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendGuildChangeGradeNamePacket(iGradeNumber, szGradeName);

	return Py_BuildNone();
}

PyObject* netSendGuildChangeGradeAuthorityPacket(PyObject* poSelf, PyObject* poArgs)
{
	int iGradeNumber;
	if (!PyTuple_GetInteger(poArgs, 0, &iGradeNumber))
		return Py_BuildException();
	int iAuthority;
	if (!PyTuple_GetInteger(poArgs, 1, &iAuthority))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendGuildChangeGradeAuthorityPacket(iGradeNumber, iAuthority);

	return Py_BuildNone();
}

PyObject* netSendGuildOfferPacket(PyObject* poSelf, PyObject* poArgs)
{
	int iExperience;
	if (!PyTuple_GetInteger(poArgs, 0, &iExperience))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendGuildOfferPacket(iExperience);

	return Py_BuildNone();
}

PyObject* netSnedGuildPostCommentPacket(PyObject* poSelf, PyObject* poArgs)
{
	char * szComment;
	if (!PyTuple_GetString(poArgs, 0, &szComment))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendGuildPostCommentPacket(szComment);

	return Py_BuildNone();
}

PyObject* netSnedGuildDeleteCommentPacket(PyObject* poSelf, PyObject* poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendGuildDeleteCommentPacket(iIndex);

	return Py_BuildNone();
}

PyObject* netSendGuildRefreshCommentsPacket(PyObject* poSelf, PyObject* poArgs)
{
	int iHightestIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iHightestIndex))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendGuildRefreshCommentsPacket(iHightestIndex);

	return Py_BuildNone();
}

PyObject* netSendGuildChangeMemberGradePacket(PyObject* poSelf, PyObject* poArgs)
{
	int iPID;
	if (!PyTuple_GetInteger(poArgs, 0, &iPID))
		return Py_BuildException();
	int iGradeNumber;
	if (!PyTuple_GetInteger(poArgs, 1, &iGradeNumber))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendGuildChangeMemberGradePacket(iPID, iGradeNumber);

	return Py_BuildNone();
}

PyObject* netSendGuildUseSkillPacket(PyObject* poSelf, PyObject* poArgs)
{
	int iSkillID;
	if (!PyTuple_GetInteger(poArgs, 0, &iSkillID))
		return Py_BuildException();
	int iTargetVID;
	if (!PyTuple_GetInteger(poArgs, 1, &iTargetVID))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendGuildUseSkillPacket(iSkillID, iTargetVID);

	return Py_BuildNone();
}

PyObject* netSendGuildChangeMemberGeneralPacket(PyObject* poSelf, PyObject* poArgs)
{
	int iPID;
	if (!PyTuple_GetInteger(poArgs, 0, &iPID))
		return Py_BuildException();
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 1, &iFlag))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendGuildChangeMemberGeneralPacket(iPID, iFlag);

	return Py_BuildNone();
}

PyObject* netSendGuildInviteAnswerPacket(PyObject* poSelf, PyObject* poArgs)
{
	int iGuildID;
	if (!PyTuple_GetInteger(poArgs, 0, &iGuildID))
		return Py_BuildException();
	int iAnswer;
	if (!PyTuple_GetInteger(poArgs, 1, &iAnswer))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendGuildInviteAnswerPacket(iGuildID, iAnswer);

	return Py_BuildNone();
}

PyObject* netSendGuildChargeGSPPacket(PyObject* poSelf, PyObject* poArgs)
{
	int iGSP;
	if (!PyTuple_GetInteger(poArgs, 0, &iGSP))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendGuildChargeGSPPacket(iGSP);

	return Py_BuildNone();
}

PyObject* netSendGuildDepositMoneyPacket(PyObject* poSelf, PyObject* poArgs)
{
	int iGSP;
	if (!PyTuple_GetInteger(poArgs, 0, &iGSP))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendGuildDepositMoneyPacket(iGSP);

	return Py_BuildNone();
}

PyObject* netSendGuildWithdrawMoneyPacket(PyObject* poSelf, PyObject* poArgs)
{
	int iGSP;
	if (!PyTuple_GetInteger(poArgs, 0, &iGSP))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendGuildWithdrawMoneyPacket(iGSP);

	return Py_BuildNone();
}

PyObject* netSendRequestRefineInfoPacket(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();

//	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
//	rns.SendRequestRefineInfoPacket(iSlotIndex);
	assert(!"netSendRequestRefineInfoPacket - This function is not used anymore");

	return Py_BuildNone();
}

PyObject* netSendRefinePacket(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BuildException();
	int iType;
	if (!PyTuple_GetInteger(poArgs, 1, &iType))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendRefinePacket(iSlotIndex, iType);

	return Py_BuildNone();
}

PyObject* netSendSelectItemPacket(PyObject* poSelf, PyObject* poArgs)
{
	int iItemPos;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemPos))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.SendSelectItemPacket(iItemPos);

	return Py_BuildNone();
}

PyObject* netSetPacketSequenceMode(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	CAccountConnector & rkAccountConnector = CAccountConnector::Instance();
	rns.SetPacketSequenceMode(true);
	rkAccountConnector.SetPacketSequenceMode(true);

	return Py_BuildNone();
}

PyObject* netSetEmpireLanguageMode(PyObject* poSelf, PyObject* poArgs)
{
	int iMode;
	if (!PyTuple_GetInteger(poArgs, 0, &iMode))
		return Py_BuildException();

	//CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	gs_bEmpireLanuageEnable = iMode;

	return Py_BuildNone();
}

PyObject* netSetSkillGroupFake(PyObject* poSelf, PyObject* poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	CPythonNetworkStream& rns=CPythonNetworkStream::Instance();
	rns.__TEST_SetSkillGroupFake(iIndex);

	return Py_BuildNone();
}

PyObject* netRegisterErrorLog(PyObject* poSelf, PyObject* poArgs)
{
	char * szLog;
	if (!PyTuple_GetString(poArgs, 0, &szLog))
		return Py_BuildException();

	return Py_BuildNone();
}

#if defined(__BL_OFFICIAL_LOOT_FILTER__)
#include "PythonItem.h"

PyObject* netSendLootingSettings(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* poDict;
	if (!PyTuple_GetObject(poArgs, 0, &poDict))
		return Py_BuildValue("b", false);

	BYTE bSettings[ELootFilter::LOOT_FILTER_SETTINGS_MAX];
	memset(bSettings, 0, sizeof(bSettings));

	//WEAPON BEGIN
	{
		PyObject* poTopic = PyDict_GetItemString(poDict, "weapon");
		if (!poTopic)
			return Py_BuildValue("b", false);

		PyObject* poOnOff = PyDict_GetItemString(poTopic, "onoff");
		if (poOnOff)
			bSettings[ELootFilter::WEAPON_ON_OFF] = PyLong_AsLong(poOnOff) != 0;

		PyObject* poRefineMin = PyDict_GetItemString(poTopic, "refine_min");
		if (poRefineMin)
			bSettings[ELootFilter::WEAPON_REFINE_MIN] = PyLong_AsLong(poRefineMin);

		PyObject* poRefineMax = PyDict_GetItemString(poTopic, "refine_max");
		if (poRefineMax)
			bSettings[ELootFilter::WEAPON_REFINE_MAX] = PyLong_AsLong(poRefineMax);

		PyObject* poWearingLevelMin = PyDict_GetItemString(poTopic, "wearing_level_min");
		if (poWearingLevelMin)
			bSettings[ELootFilter::WEAPON_WEARING_LEVEL_MIN] = PyLong_AsLong(poWearingLevelMin);

		PyObject* poWearingLevelMax = PyDict_GetItemString(poTopic, "wearing_level_max");
		if (poWearingLevelMax)
			bSettings[ELootFilter::WEAPON_WEARING_LEVEL_MAX] = PyLong_AsLong(poWearingLevelMax);

		PyObject* poSelectData = PyDict_GetItemString(poTopic, "select_data");
		if (poSelectData)
		{
			if (PyList_Size(poSelectData) != ELootFilter::WEAPON_SELECT_DATA_MAX)
				return Py_BuildValue("b", false);

			for (int i = 0; i < ELootFilter::WEAPON_SELECT_DATA_MAX; ++i)
			{
				PyObject* poSelectDataItem = PyList_GetItem(poSelectData, i);
				if (!poSelectDataItem)
					return Py_BuildValue("b", false);

				bSettings[ELootFilter::WEAPON_SELECT_DATA_JOB_WARRIOR + i] = PyLong_AsLong(poSelectDataItem) != 0;
			}
		}
	}
	//WEAPON END

	//ARMOR BEGIN
	{
		PyObject* poTopic = PyDict_GetItemString(poDict, "armor");
		if (!poTopic)
			return Py_BuildValue("b", false);

		PyObject* poOnOff = PyDict_GetItemString(poTopic, "onoff");
		if (poOnOff)
			bSettings[ELootFilter::ARMOR_ON_OFF] = PyLong_AsLong(poOnOff) != 0;

		PyObject* poRefineMin = PyDict_GetItemString(poTopic, "refine_min");
		if (poRefineMin)
			bSettings[ELootFilter::ARMOR_REFINE_MIN] = PyLong_AsLong(poRefineMin);

		PyObject* poRefineMax = PyDict_GetItemString(poTopic, "refine_max");
		if (poRefineMax)
			bSettings[ELootFilter::ARMOR_REFINE_MAX] = PyLong_AsLong(poRefineMax);

		PyObject* poWearingLevelMin = PyDict_GetItemString(poTopic, "wearing_level_min");
		if (poWearingLevelMin)
			bSettings[ELootFilter::ARMOR_WEARING_LEVEL_MIN] = PyLong_AsLong(poWearingLevelMin);

		PyObject* poWearingLevelMax = PyDict_GetItemString(poTopic, "wearing_level_max");
		if (poWearingLevelMax)
			bSettings[ELootFilter::ARMOR_WEARING_LEVEL_MAX] = PyLong_AsLong(poWearingLevelMax);

		PyObject* poSelectData = PyDict_GetItemString(poTopic, "select_data");
		if (poSelectData)
		{
			if (PyList_Size(poSelectData) != ELootFilter::ARMOR_SELECT_DATA_MAX)
				return Py_BuildValue("b", false);

			for (int i = 0; i < ELootFilter::ARMOR_SELECT_DATA_MAX; ++i)
			{
				PyObject* poSelectDataItem = PyList_GetItem(poSelectData, i);
				if (!poSelectDataItem)
					return Py_BuildValue("b", false);

				bSettings[ELootFilter::ARMOR_SELECT_DATA_JOB_WARRIOR + i] = PyLong_AsLong(poSelectDataItem) != 0;
			}
		}
	}
	//ARMOR END

	//HEAD BEGIN
	{
		PyObject* poTopic = PyDict_GetItemString(poDict, "head");
		if (!poTopic)
			return Py_BuildValue("b", false);

		PyObject* poOnOff = PyDict_GetItemString(poTopic, "onoff");
		if (poOnOff)
			bSettings[ELootFilter::HEAD_ON_OFF] = PyLong_AsLong(poOnOff) != 0;

		PyObject* poRefineMin = PyDict_GetItemString(poTopic, "refine_min");
		if (poRefineMin)
			bSettings[ELootFilter::HEAD_REFINE_MIN] = PyLong_AsLong(poRefineMin);

		PyObject* poRefineMax = PyDict_GetItemString(poTopic, "refine_max");
		if (poRefineMax)
			bSettings[ELootFilter::HEAD_REFINE_MAX] = PyLong_AsLong(poRefineMax);

		PyObject* poWearingLevelMin = PyDict_GetItemString(poTopic, "wearing_level_min");
		if (poWearingLevelMin)
			bSettings[ELootFilter::HEAD_WEARING_LEVEL_MIN] = PyLong_AsLong(poWearingLevelMin);

		PyObject* poWearingLevelMax = PyDict_GetItemString(poTopic, "wearing_level_max");
		if (poWearingLevelMax)
			bSettings[ELootFilter::HEAD_WEARING_LEVEL_MAX] = PyLong_AsLong(poWearingLevelMax);

		PyObject* poSelectData = PyDict_GetItemString(poTopic, "select_data");
		if (poSelectData)
		{
			if (PyList_Size(poSelectData) != ELootFilter::HEAD_SELECT_DATA_MAX)
				return Py_BuildValue("b", false);

			for (int i = 0; i < ELootFilter::HEAD_SELECT_DATA_MAX; ++i)
			{
				PyObject* poSelectDataItem = PyList_GetItem(poSelectData, i);
				if (!poSelectDataItem)
					return Py_BuildValue("b", false);

				bSettings[ELootFilter::HEAD_SELECT_DATA_JOB_WARRIOR + i] = PyLong_AsLong(poSelectDataItem) != 0;
			}
		}
	}
	//HEAD END

	//COMMON BEGIN
	{
		PyObject* poTopic = PyDict_GetItemString(poDict, "common");
		if (!poTopic)
			return Py_BuildValue("b", false);

		PyObject* poOnOff = PyDict_GetItemString(poTopic, "onoff");
		if (poOnOff)
			bSettings[ELootFilter::COMMON_ON_OFF] = PyLong_AsLong(poOnOff) != 0;

		PyObject* poRefineMin = PyDict_GetItemString(poTopic, "refine_min");
		if (poRefineMin)
			bSettings[ELootFilter::COMMON_REFINE_MIN] = PyLong_AsLong(poRefineMin);

		PyObject* poRefineMax = PyDict_GetItemString(poTopic, "refine_max");
		if (poRefineMax)
			bSettings[ELootFilter::COMMON_REFINE_MAX] = PyLong_AsLong(poRefineMax);

		PyObject* poWearingLevelMin = PyDict_GetItemString(poTopic, "wearing_level_min");
		if (poWearingLevelMin)
			bSettings[ELootFilter::COMMON_WEARING_LEVEL_MIN] = PyLong_AsLong(poWearingLevelMin);

		PyObject* poWearingLevelMax = PyDict_GetItemString(poTopic, "wearing_level_max");
		if (poWearingLevelMax)
			bSettings[ELootFilter::COMMON_WEARING_LEVEL_MAX] = PyLong_AsLong(poWearingLevelMax);

		PyObject* poSelectData = PyDict_GetItemString(poTopic, "select_data");
		if (poSelectData)
		{
			if (PyList_Size(poSelectData) != ELootFilter::COMMON_SELECT_DATA_MAX)
				return Py_BuildValue("b", false);

			for (int i = 0; i < ELootFilter::COMMON_SELECT_DATA_MAX; ++i)
			{
				PyObject* poSelectDataItem = PyList_GetItem(poSelectData, i);
				if (!poSelectDataItem)
					return Py_BuildValue("b", false);

				bSettings[ELootFilter::COMMON_SELECT_DATA_SHOE + i] = PyLong_AsLong(poSelectDataItem) != 0;
			}
		}
	}
	//COMMON END

	//COSTUME BEGIN
	{
		PyObject* poTopic = PyDict_GetItemString(poDict, "costume");
		if (!poTopic)
			return Py_BuildValue("b", false);

		PyObject* poOnOff = PyDict_GetItemString(poTopic, "onoff");
		if (poOnOff)
			bSettings[ELootFilter::COSTUME_ON_OFF] = PyLong_AsLong(poOnOff) != 0;

		PyObject* poSelectData = PyDict_GetItemString(poTopic, "select_data");
		if (poSelectData)
		{
			if (PyList_Size(poSelectData) != ELootFilter::COSTUME_SELECT_DATA_MAX)
				return Py_BuildValue("b", false);

			for (int i = 0; i < ELootFilter::COSTUME_SELECT_DATA_MAX; ++i)
			{
				PyObject* poSelectDataItem = PyList_GetItem(poSelectData, i);
				if (!poSelectDataItem)
					return Py_BuildValue("b", false);

				bSettings[ELootFilter::COSTUME_SELECT_DATA_WEAPON + i] = PyLong_AsLong(poSelectDataItem) != 0;
			}
		}
	}
	//COSTUME END

	//DS BEGIN
	{
		PyObject* poTopic = PyDict_GetItemString(poDict, "ds");
		if (!poTopic)
			return Py_BuildValue("b", false);

		PyObject* poOnOff = PyDict_GetItemString(poTopic, "onoff");
		if (poOnOff)
			bSettings[ELootFilter::DS_ON_OFF] = PyLong_AsLong(poOnOff) != 0;

		PyObject* poSelectData = PyDict_GetItemString(poTopic, "select_data");
		if (poSelectData)
		{
			if (PyList_Size(poSelectData) != ELootFilter::DS_SELECT_DATA_MAX)
				return Py_BuildValue("b", false);

			for (int i = 0; i < ELootFilter::DS_SELECT_DATA_MAX; ++i)
			{
				PyObject* poSelectDataItem = PyList_GetItem(poSelectData, i);
				if (!poSelectDataItem)
					return Py_BuildValue("b", false);

				bSettings[ELootFilter::DS_SELECT_DATA_DS + i] = PyLong_AsLong(poSelectDataItem) != 0;
			}
		}
	}
	//DS END

	//UNIQUE BEGIN
	{
		PyObject* poTopic = PyDict_GetItemString(poDict, "unique");
		if (!poTopic)
			return Py_BuildValue("b", false);

		PyObject* poOnOff = PyDict_GetItemString(poTopic, "onoff");
		if (poOnOff)
			bSettings[ELootFilter::UNIQUE_ON_OFF] = PyLong_AsLong(poOnOff) != 0;

		PyObject* poSelectData = PyDict_GetItemString(poTopic, "select_data");
		if (poSelectData)
		{
			if (PyList_Size(poSelectData) != ELootFilter::UNIQUE_SELECT_DATA_MAX)
				return Py_BuildValue("b", false);

			for (int i = 0; i < ELootFilter::UNIQUE_SELECT_DATA_MAX; ++i)
			{
				PyObject* poSelectDataItem = PyList_GetItem(poSelectData, i);
				if (!poSelectDataItem)
					return Py_BuildValue("b", false);

				bSettings[ELootFilter::UNIQUE_SELECT_DATA_ABILITY + i] = PyLong_AsLong(poSelectDataItem) != 0;
			}
		}
	}
	//UNIQUE END

	//REFINE BEGIN
	{
		PyObject* poTopic = PyDict_GetItemString(poDict, "refine");
		if (!poTopic)
			return Py_BuildValue("b", false);

		PyObject* poOnOff = PyDict_GetItemString(poTopic, "onoff");
		if (poOnOff)
			bSettings[ELootFilter::REFINE_ON_OFF] = PyLong_AsLong(poOnOff) != 0;

		PyObject* poSelectData = PyDict_GetItemString(poTopic, "select_data");
		if (poSelectData)
		{
			if (PyList_Size(poSelectData) != ELootFilter::REFINE_SELECT_DATA_MAX)
				return Py_BuildValue("b", false);

			for (int i = 0; i < ELootFilter::REFINE_SELECT_DATA_MAX; ++i)
			{
				PyObject* poSelectDataItem = PyList_GetItem(poSelectData, i);
				if (!poSelectDataItem)
					return Py_BuildValue("b", false);

				bSettings[ELootFilter::REFINE_SELECT_DATA_MATERIAL + i] = PyLong_AsLong(poSelectDataItem) != 0;
			}
		}
	}
	//REFINE END

	//POTION BEGIN
	{
		PyObject* poTopic = PyDict_GetItemString(poDict, "potion");
		if (!poTopic)
			return Py_BuildValue("b", false);

		PyObject* poOnOff = PyDict_GetItemString(poTopic, "onoff");
		if (poOnOff)
			bSettings[ELootFilter::POTION_ON_OFF] = PyLong_AsLong(poOnOff) != 0;

		PyObject* poSelectData = PyDict_GetItemString(poTopic, "select_data");
		if (poSelectData)
		{
			if (PyList_Size(poSelectData) != ELootFilter::POTION_SELECT_DATA_MAX)
				return Py_BuildValue("b", false);

			for (int i = 0; i < ELootFilter::POTION_SELECT_DATA_MAX; ++i)
			{
				PyObject* poSelectDataItem = PyList_GetItem(poSelectData, i);
				if (!poSelectDataItem)
					return Py_BuildValue("b", false);

				bSettings[ELootFilter::POTION_SELECT_DATA_ABILITY + i] = PyLong_AsLong(poSelectDataItem) != 0;
			}
		}
	}
	//POTION END

	//FISH_MINING BEGIN
	{
		PyObject* poTopic = PyDict_GetItemString(poDict, "fish_mining");
		if (!poTopic)
			return Py_BuildValue("b", false);

		PyObject* poOnOff = PyDict_GetItemString(poTopic, "onoff");
		if (poOnOff)
			bSettings[ELootFilter::FISH_MINING_ON_OFF] = PyLong_AsLong(poOnOff) != 0;

		PyObject* poSelectData = PyDict_GetItemString(poTopic, "select_data");
		if (poSelectData)
		{
			if (PyList_Size(poSelectData) != ELootFilter::FISH_MINING_SELECT_DATA_MAX)
				return Py_BuildValue("b", false);

			for (int i = 0; i < ELootFilter::FISH_MINING_SELECT_DATA_MAX; ++i)
			{
				PyObject* poSelectDataItem = PyList_GetItem(poSelectData, i);
				if (!poSelectDataItem)
					return Py_BuildValue("b", false);

				bSettings[ELootFilter::FISH_MINING_SELECT_DATA_FOOD + i] = PyLong_AsLong(poSelectDataItem) != 0;
			}
		}
	}
	//FISH_MINING END

	//MOUNT_PET BEGIN
	{
		PyObject* poTopic = PyDict_GetItemString(poDict, "mount_pet");
		if (!poTopic)
			return Py_BuildValue("b", false);

		PyObject* poOnOff = PyDict_GetItemString(poTopic, "onoff");
		if (poOnOff)
			bSettings[ELootFilter::MOUNT_PET_ON_OFF] = PyLong_AsLong(poOnOff) != 0;

		PyObject* poSelectData = PyDict_GetItemString(poTopic, "select_data");
		if (poSelectData)
		{
			if (PyList_Size(poSelectData) != ELootFilter::MOUNT_PET_SELECT_DATA_MAX)
				return Py_BuildValue("b", false);

			for (int i = 0; i < ELootFilter::MOUNT_PET_SELECT_DATA_MAX; ++i)
			{
				PyObject* poSelectDataItem = PyList_GetItem(poSelectData, i);
				if (!poSelectDataItem)
					return Py_BuildValue("b", false);

				bSettings[ELootFilter::MOUNT_PET_SELECT_DATA_CHARGED_PET + i] = PyLong_AsLong(poSelectDataItem) != 0;
			}
		}
	}
	//MOUNT_PET END

	//SKILL_BOOK BEGIN
	{
		PyObject* poTopic = PyDict_GetItemString(poDict, "skill_book");
		if (!poTopic)
			return Py_BuildValue("b", false);

		PyObject* poOnOff = PyDict_GetItemString(poTopic, "onoff");
		if (poOnOff)
			bSettings[ELootFilter::SKILL_BOOK_ON_OFF] = PyLong_AsLong(poOnOff) != 0;

		PyObject* poSelectData = PyDict_GetItemString(poTopic, "select_data");
		if (poSelectData)
		{
			if (PyList_Size(poSelectData) != ELootFilter::SKILL_BOOK_SELECT_DATA_MAX)
				return Py_BuildValue("b", false);

			for (int i = 0; i < ELootFilter::SKILL_BOOK_SELECT_DATA_MAX; ++i)
			{
				PyObject* poSelectDataItem = PyList_GetItem(poSelectData, i);
				if (!poSelectDataItem)
					return Py_BuildValue("b", false);

				bSettings[ELootFilter::SKILL_BOOK_SELECT_DATA_JOB_WARRIOR + i] = PyLong_AsLong(poSelectDataItem) != 0;
			}
		}
	}
	//SKILL_BOOK END

	//ETC BEGIN
	{
		PyObject* poTopic = PyDict_GetItemString(poDict, "etc");
		if (!poTopic)
			return Py_BuildValue("b", false);

		PyObject* poOnOff = PyDict_GetItemString(poTopic, "onoff");
		if (poOnOff)
			bSettings[ELootFilter::ETC_ON_OFF] = PyLong_AsLong(poOnOff) != 0;

		PyObject* poSelectData = PyDict_GetItemString(poTopic, "select_data");
		if (poSelectData)
		{
			if (PyList_Size(poSelectData) != ELootFilter::ETC_SELECT_DATA_MAX)
				return Py_BuildValue("b", false);

			for (int i = 0; i < ELootFilter::ETC_SELECT_DATA_MAX; ++i)
			{
				PyObject* poSelectDataItem = PyList_GetItem(poSelectData, i);
				if (!poSelectDataItem)
					return Py_BuildValue("b", false);

				bSettings[ELootFilter::ETC_SELECT_DATA_GIFTBOX + i] = PyLong_AsLong(poSelectDataItem) != 0;
			}
		}
	}
	//ETC END

	//EVENT BEGIN
	{
		PyObject* poTopic = PyDict_GetItemString(poDict, "event");
		if (!poTopic)
			return Py_BuildValue("b", false);

		PyObject* poOnOff = PyDict_GetItemString(poTopic, "onoff");
		if (poOnOff)
			bSettings[ELootFilter::EVENT_ON_OFF] = PyLong_AsLong(poOnOff) != 0;
	}
	//EVENT END

	CPythonItem::Instance().ClearLootFilteredItems();
	CPythonNetworkStream::Instance().SendLootFilterPacket(bSettings);
	return Py_BuildValue("b", true);
}
#endif

#ifdef ENABLE_GEM_SYSTEM
PyObject* netSendGemShopBuy(PyObject* poSelf, PyObject* poArgs)
{
	int iItemPos;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemPos))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendGemShopBuy(iItemPos);

	return Py_BuildNone();
}
PyObject* netSendGemShopAdd(PyObject* poSelf, PyObject* poArgs)
{
	int iItemPos;
	if (!PyTuple_GetInteger(poArgs, 0, &iItemPos))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendGemShopAdd(iItemPos);

	return Py_BuildNone();
}
PyObject* netSendGemShopRefresh(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().SendGemShopRefresh();
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_FISH_EVENT
PyObject* netSendFishBoxUse(PyObject* poSelf, PyObject* poArgs)
{
	int iWindow;
	if (!PyTuple_GetInteger(poArgs, 0, &iWindow))
		return Py_BuildException();
	
	int iCell;
	if (!PyTuple_GetInteger(poArgs, 1, &iCell))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendFishBoxUse(iWindow, iCell);
	return Py_BuildNone();
}
PyObject* netSendFishShapeAdd(PyObject* poSelf, PyObject* poArgs)
{
	int iShapePos;
	if (!PyTuple_GetInteger(poArgs, 0, &iShapePos))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendFishShapeAdd(iShapePos);

	return Py_BuildNone();
}
#endif

#if defined(__BL_TRANSMUTATION__)
PyObject* netSendChangeLookCheckIn(PyObject* poSelf, PyObject* poArgs)
{
	TItemPos Cell;
	if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
		return Py_BuildException();

	BYTE bChangeLookSlotType;
	if (!PyTuple_GetByte(poArgs, 2, &bChangeLookSlotType))
		return Py_BuildException();

	if (bChangeLookSlotType >= static_cast<BYTE>(ETRANSMUTATIONSLOTTYPE::TRANSMUTATION_SLOT_MAX))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendChangeLookCheckInPacket(Cell, bChangeLookSlotType);

	return Py_BuildNone();
}

PyObject* netSendChangeLookCheckOut(PyObject* poSelf, PyObject* poArgs)
{
	BYTE bChangeLookSlotType;
	if (!PyTuple_GetByte(poArgs, 0, &bChangeLookSlotType))
		return Py_BuildException();
	
	CPythonNetworkStream::Instance().SendChangeLookCheckOutPacket(bChangeLookSlotType);
	
	return Py_BuildNone();
}

PyObject* netSendChangeLookCheckInFreeYangItem(PyObject* poSelf, PyObject* poArgs)
{
	TItemPos Cell;
	if (!PyTuple_GetByte(poArgs, 0, &Cell.window_type))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 1, &Cell.cell))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendChangeLookCheckInFreeYangItemPacket(Cell);

	return Py_BuildNone();
}

PyObject* netSendChangeLookCheckOutFreeYangItem(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().SendChangeLookCheckOutFreeYangItemPacket();
	
	return Py_BuildNone();
}

PyObject* netSendChangeLookAccept(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().SendChangeLookAcceptPacket();
	
	return Py_BuildNone();
}

PyObject* netSendChangeLookCancel(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().SendChangeLookCancelPacket();
	
	return Py_BuildNone();
}
#endif

#if defined(__BL_MAILBOX__)
PyObject* netSendPostWriteConfirm(PyObject* poSelf, PyObject* poArgs)
{
	char* szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendPostWriteConfirm(szName);
	return Py_BuildNone();
}

PyObject* netSendMailBoxClose(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().SendMailBoxClose();
	return Py_BuildNone();
}

PyObject* netSendPostAllDelete(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().SendPostAllDelete();
	return Py_BuildNone();
}

PyObject* netSendPostAllGetItems(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().SendPostAllGetItems();
	return Py_BuildNone();
}

PyObject* netSendPostDelete(PyObject* poSelf, PyObject* poArgs)
{
	BYTE index;
	if (!PyTuple_GetByte(poArgs, 0, &index))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendPostDelete(index);
	return Py_BuildNone();
}

PyObject* netSendPostGetItems(PyObject* poSelf, PyObject* poArgs)
{
	BYTE index;
	if (!PyTuple_GetByte(poArgs, 0, &index))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendPostGetItems(index);
	return Py_BuildNone();
}

PyObject* netSendPostWrite(PyObject* poSelf, PyObject* poArgs)
{
	BYTE Idx = 0;

	char* szName;
	if (!PyTuple_GetString(poArgs, Idx++, &szName))
		return Py_BuildException();

	char* szTitle;
	if (!PyTuple_GetString(poArgs, Idx++, &szTitle))
		return Py_BuildException();

	char* szMessage;
	if (!PyTuple_GetString(poArgs, Idx++, &szMessage))
		return Py_BuildException();

	TItemPos Cell;
	if (!PyTuple_GetByte(poArgs, Idx++, &Cell.window_type))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, Idx++, &Cell.cell))
		return Py_BuildException();

	int iYang;
	if (!PyTuple_GetInteger(poArgs, Idx++, &iYang))
		return Py_BuildException();

	int iWon;
	if (!PyTuple_GetInteger(poArgs, Idx++, &iWon))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendPostWrite(szName, szTitle, szMessage, Cell, iYang, iWon);
	return Py_BuildNone();
}

PyObject* netRequestPostAddData(PyObject* poSelf, PyObject* poArgs)
{
	BYTE ButtonIndex;
	if (!PyTuple_GetByte(poArgs, 0, &ButtonIndex))
		return Py_BuildException();

	BYTE DataIndex;
	if (!PyTuple_GetByte(poArgs, 1, &DataIndex))
		return Py_BuildException();

	CPythonNetworkStream::Instance().RequestPostAddData(ButtonIndex, DataIndex);
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_CAPTCHA
PyObject* netSendCaptcha(PyObject* poSelf, PyObject* poArgs)
{
	int captcha;
	if (!PyTuple_GetInteger(poArgs, 0, &captcha))
		return Py_BuildException();

	if (captcha < 100 || captcha > 999)
		return Py_BuildNone();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendCaptchaPacket(captcha);

	return Py_BuildNone();
}
#endif

#if defined(__BL_MOVE_COSTUME_ATTR__)
PyObject* netSendItemCombinationPacket(PyObject* poSelf, PyObject* poArgs)
{
	int iMediumIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iMediumIndex))
		return Py_BuildException();

	int iBaseIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iBaseIndex))
		return Py_BuildException();

	int iMaterialIndex;
	if (!PyTuple_GetInteger(poArgs, 2, &iMaterialIndex))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendItemCombinationPacket(iMediumIndex, iBaseIndex, iMaterialIndex);
	return Py_BuildNone();
}

PyObject* netSendItemCombinationPacketCancel(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().SendItemCombinationPacketCancel();
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_OFFLINESHOP_SYSTEM
PyObject* netSendOfflineShopEndPacket(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendOfflineShopEndPacket();
	return Py_BuildNone();
}

PyObject* netSendOfflineShopBuyPacket(PyObject* poSelf, PyObject* poArgs)
{
	int vid;
	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();
	int pos;
	if (!PyTuple_GetInteger(poArgs, 1, &pos))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendOfflineShopBuyPacket((DWORD)vid, pos);
	return Py_BuildNone();
}

PyObject* netSendAddOfflineShopItem(PyObject* poSelf, PyObject* poArgs)
{
	int bDisplayPos;
	int bPos;
	long long iPrice;
	BYTE bWindowType;

	if (!PyTuple_GetInteger(poArgs, 0, &bDisplayPos))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 1, &bPos))
		return Py_BuildException();
	if (!PyTuple_GetLongLong(poArgs, 2, &iPrice))
		return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 3, &bWindowType))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::instance();
	rkNetStream.SendAddOfflineShopItem(TItemPos(bWindowType, bDisplayPos), bPos, iPrice);
	return Py_BuildNone();
}

PyObject* netSendRemoveOfflineShopItem(PyObject* poSelf, PyObject* poArgs)
{
	BYTE bPos;
	if (!PyTuple_GetInteger(poArgs, 0, &bPos))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::instance();
	rkNetStream.SendRemoveOfflineShopItem(bPos);
	return Py_BuildNone();
}

PyObject* netSendDestroyOfflineShop(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::instance();
	rkNetStream.SendDestroyOfflineShop();
	return Py_BuildNone();
}

PyObject* netSendTakeOfflineMoney(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::instance();
	rkNetStream.SendTakeOfflineMoney();
	return Py_BuildNone();
}

PyObject* netSendOfflineShopChangeTitle(PyObject* poSelf, PyObject* poArgs)
{
	char* title;
	if (!PyTuple_GetString(poArgs, 0, &title))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::instance();
	rkNetStream.SendOfflineShopChangeTitle(title);
	return Py_BuildNone();
}

PyObject* netSendOfflineShopCheck(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::instance();
	rkNetStream.SendOfflineShopCheck();
	return Py_BuildNone();
}

PyObject* netSendOfflineShopButton(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::instance();
	rkNetStream.SendOfflineShopButton();
	return Py_BuildNone();
}

PyObject* netSendOfflineShopRemoveLogs(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::instance();
	rkNetStream.SendOfflineShopRemoveLogs();
	return Py_BuildNone();
}

PyObject* netSendOpenShopSlot(PyObject* poSelf, PyObject* poArgs)
{
	BYTE bPos;
	if (!PyTuple_GetInteger(poArgs, 0, &bPos))
		return Py_BuildException();
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::instance();
	rkNetStream.SendOpenShopSlot(bPos);
	return Py_BuildNone();
}

PyObject* netSendGetBackItems(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::instance();
	rkNetStream.SendGetBackItems();
	return Py_BuildNone();
}

PyObject* netSendAddTime(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::instance();
	rkNetStream.SendAddTime();
	return Py_BuildNone();
}

PyObject* netSendOpenWithVID(PyObject* poSelf, PyObject* poArgs)
{
	int vid;
	if (!PyTuple_GetInteger(poArgs, 0, &vid))
		return Py_BuildException();
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::instance();
	rkNetStream.SendOfflineOpenWithVID(vid);
	return Py_BuildNone();
}

#endif

#if defined(__BL_PICK_FILTER__)
PyObject* netOpenPickUpWindow(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.OpenPickUpWindow();

	return Py_BuildNone();
}
#endif

#ifdef ENABLE_SKILLBOOK_COMB_SYSTEM

PyObject* netSendSkillBookCombinationPacket(PyObject* poSelf, PyObject* poArgs)
{
	TItemPos CombItemPos[SKILLBOOK_COMB_SLOT_MAX];
	
	// FIX: Initialize the array with blank positions to prevent memory corruption
	// and stop the server from accidentally consuming items from random memory offsets.
	for (int i = 0; i < SKILLBOOK_COMB_SLOT_MAX; i++)
	{
		CombItemPos[i].window_type = 0; // RESERVED_WINDOW
		CombItemPos[i].cell = 0xFFFF;   // Safe Null Cell
	}

	PyObject* pDic;
	if (!PyTuple_GetObject(poArgs, 0, &pDic))
		return Py_BuildException();

	BYTE bAction;
	if (!PyTuple_GetByte(poArgs, 1, &bAction))
		return Py_BuildException();

	int pos = 0;
	PyObject* key, * value;
	int size = PyDict_Size(pDic);

	while (PyDict_Next(pDic, &pos, &key, &value))
	{
		int i = PyInt_AsLong(key);
		if (i > SKILLBOOK_COMB_SLOT_MAX)
			return Py_BuildException();

		if (!PyTuple_GetByte(value, 0, &CombItemPos[i].window_type)
			|| !PyTuple_GetInteger(value, 1, &CombItemPos[i].cell))
		{
			return Py_BuildException();
		}
	}

	CPythonNetworkStream& rns = CPythonNetworkStream::Instance();
	rns.SendSkillBookCombinationPacket(CombItemPos, bAction);

	return Py_BuildNone();
}
#endif

#ifdef ENABLE_SHOP_SEARCH_SYSTEM
PyObject* netSendPrivateShopSearchInfo(PyObject* poSelf, PyObject* poArgs)
{
	int ItemCat;
	if (!PyTuple_GetInteger(poArgs, 0, &ItemCat))
		return Py_BuildException();
	int SubCat;
	if (!PyTuple_GetInteger(poArgs, 1, &SubCat))
		return Py_BuildException();
	int minAvg;
	if (!PyTuple_GetInteger(poArgs, 2, &minAvg))
		return Py_BuildException();
	int maxAvg;
	if (!PyTuple_GetInteger(poArgs, 3, &maxAvg))
		return Py_BuildException();
	int minSkill;
	if (!PyTuple_GetInteger(poArgs, 4, &minSkill))
		return Py_BuildException();
	int maxSkill;
	if (!PyTuple_GetInteger(poArgs, 5, &maxSkill))
		return Py_BuildException();
	int minAbs;
	if (!PyTuple_GetInteger(poArgs, 6, &minAbs))
		return Py_BuildException();
	int maxAbs;
	if (!PyTuple_GetInteger(poArgs, 7, &maxAbs))
		return Py_BuildException();
	int minLevel;
	if (!PyTuple_GetInteger(poArgs, 8, &minLevel))
		return Py_BuildException();
	int maxLevel;
	if (!PyTuple_GetInteger(poArgs, 9, &maxLevel))
		return Py_BuildException();
	int minRefine;
	if (!PyTuple_GetInteger(poArgs, 10, &minRefine))
		return Py_BuildException();
	int maxRefine;
	if (!PyTuple_GetInteger(poArgs, 11, &maxRefine))
		return Py_BuildException();
	int enchant;
	if (!PyTuple_GetInteger(poArgs, 12, &enchant))
		return Py_BuildException();
	int alchemy;
	if (!PyTuple_GetInteger(poArgs, 13, &alchemy))
		return Py_BuildException();
	int sex;
	if (!PyTuple_GetInteger(poArgs, 14, &sex))
		return Py_BuildException();
	int character;
	if (!PyTuple_GetInteger(poArgs, 15, &character))
		return Py_BuildException();
	int value;
	if (!PyTuple_GetInteger(poArgs, 16, &value))
		return Py_BuildException();
	int itemvnum;
	if (!PyTuple_GetInteger(poArgs, 17, &itemvnum))
		return Py_BuildException();

	TPacketCGShopSearch p;
	p.header = HEADER_CG_SHOP_SEARCH;
	p.ItemCat = ItemCat;
	p.SubCat = SubCat;
	p.avg[0] = minAvg;
	p.avg[1] = maxAvg;
	p.skill[0] = minSkill;
	p.skill[1] = maxSkill;
	p.abs[0] = minAbs;
	p.abs[1] = maxAbs;
	p.level[0] = minLevel;
	p.level[1] = maxLevel;
	p.refine[0] = minRefine;
	p.refine[1] = maxRefine;
	p.enchant = enchant;
	p.alchemy = alchemy;
	p.sex = sex;
	p.character = character;
	p.Value = value;
	p.itemVnum = itemvnum;
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendPrivateShopSearchInfo(&p);
	return Py_BuildNone();
}
#endif

#ifdef ELEMENT_SPELL_WORLDARD

PyObject*  netSendElementsSpellClose(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendElementsSpellClose();
	return Py_BuildNone();
}

PyObject*  netSendElementsSpellAdd(PyObject* poSelf, PyObject* poArgs)
{

	int pos;
	if (!PyTuple_GetInteger(poArgs, 0, &pos))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendElementsSpellAdd(pos);
	return Py_BuildNone();
}

PyObject*  netSendElementsSpellChange(PyObject* poSelf, PyObject* poArgs)
{

	int pos;
	if (!PyTuple_GetInteger(poArgs, 0, &pos))
		return Py_BuildException();

	int type_select;
	if (!PyTuple_GetInteger(poArgs, 1, &type_select))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendElementsSpellChange(pos, type_select);
	return Py_BuildNone();
}

#endif

#ifdef ENABLE_MINI_GAME_CATCH_KING
PyObject* netSendMiniGameCatchKingStart(PyObject* poSelf, PyObject* poArgs)
{
	int ibetNumber;
	if (!PyTuple_GetInteger(poArgs, 0, &ibetNumber))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendMiniGameCatchKingStart(ibetNumber);
	return Py_BuildNone();
}

PyObject* netSendMiniGameCatchKingDeckCardClick(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().SendMiniGameCatchKingDeckCardClick();
	return Py_BuildNone();
}

PyObject* netSendMiniGameCatchKingFieldCardClick(PyObject* poSelf, PyObject* poArgs)
{
	int icardNumber;
	if (!PyTuple_GetInteger(poArgs, 0, &icardNumber))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendMiniGameCatchKingFieldCardClick(icardNumber);
	return Py_BuildNone();
}

PyObject* netSendMiniGameCatchKingReward(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().SendMiniGameCatchKingReward();
	return Py_BuildNone();
}

PyObject* netOpenCatchKing(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().SendOpenCatchKing();
	return Py_BuildNone();
}
#endif	// ENABLE_MINI_GAME_CATCH_KING

#ifdef ENABLE_MINI_GAME_YUTNORI
PyObject* netSendMiniGameYutNori(PyObject* poSelf, PyObject* poArgs)
{
	int iSubHeader;
	if (!PyTuple_GetInteger(poArgs, 0, &iSubHeader))
		return Py_BuildException();
	
	int iSubArgument;
	if (!PyTuple_GetInteger(poArgs, 1, &iSubArgument))
		CPythonNetworkStream::Instance().SendMiniGameYutNori(iSubHeader, 0);
	else
		CPythonNetworkStream::Instance().SendMiniGameYutNori(iSubHeader, iSubArgument);
	
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_TAROT
PyObject* netMinigameBnwStart(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendMinigameBnwStart();
	return Py_BuildNone();
}

PyObject* netMinigameBnwCardSelected(PyObject* poSelf, PyObject* poArgs)
{
	BYTE bCard = 0;
	if (!PyTuple_GetByte(poArgs, 0, &bCard))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendMinigameBnwCardSelected(bCard);
	return Py_BuildNone();
}

PyObject* netMinigameBnwFinished(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendMinigameBnwFinished();
	return Py_BuildNone();
}
#endif

#ifdef ENABLE_AURA_SYSTEM
PyObject* netSendAuraRefineCheckIn(PyObject* poSelf, PyObject* poArgs)
{
	TItemPos InventoryPos;
	TItemPos AuraPos;
	BYTE bAuraRefineWindowType;
	switch (PyTuple_Size(poArgs))
	{
	case 3:
		if (!PyTuple_GetInteger(poArgs, 0, &InventoryPos.cell))
			return Py_BuildException();
		AuraPos.window_type = AURA_REFINE;
		if (!PyTuple_GetInteger(poArgs, 1, &AuraPos.cell))
			return Py_BuildException();
		if (!PyTuple_GetByte(poArgs, 2, &bAuraRefineWindowType))
			return Py_BuildException();
		break;
	case 5:
		if (!PyTuple_GetByte(poArgs, 0, &InventoryPos.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &InventoryPos.cell))
			return Py_BuildException();
		if (!PyTuple_GetByte(poArgs, 2, &AuraPos.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 3, &AuraPos.cell))
			return Py_BuildException();
		if (!PyTuple_GetByte(poArgs, 4, &bAuraRefineWindowType))
			return Py_BuildException();
		break;
	default:
		return Py_BuildException();
	}

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendAuraRefineCheckIn(InventoryPos, AuraPos, bAuraRefineWindowType);
	return Py_BuildNone();
}

PyObject* netSendAuraRefineCheckOut(PyObject* poSelf, PyObject* poArgs)
{
	TItemPos AuraPos;
	BYTE byAuraRefineWindowType;
	switch (PyTuple_Size(poArgs))
	{
	case 2:
		AuraPos.window_type = AURA_REFINE;
		if (!PyTuple_GetInteger(poArgs, 0, &AuraPos.cell))
			return Py_BuildException();
		if (!PyTuple_GetByte(poArgs, 1, &byAuraRefineWindowType))
			return Py_BuildException();
		break;
	case 3:
		if (!PyTuple_GetByte(poArgs, 0, &AuraPos.window_type))
			return Py_BuildException();
		if (!PyTuple_GetInteger(poArgs, 1, &AuraPos.cell))
			return Py_BuildException();
		if (!PyTuple_GetByte(poArgs, 2, &byAuraRefineWindowType))
			return Py_BuildException();
		break;
	default:
		return Py_BuildException();
	}

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendAuraRefineCheckOut(AuraPos, byAuraRefineWindowType);
	return Py_BuildNone();
}

PyObject* netSendAuraRefineAccept(PyObject* poSelf, PyObject* poArgs)
{
	BYTE byAuraRefineWindowType;
	if (!PyTuple_GetByte(poArgs, 0, &byAuraRefineWindowType))
		return Py_BuildException();

	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendAuraRefineAccept(byAuraRefineWindowType);
	return Py_BuildNone();
}

PyObject* netSendAuraRefineCancel(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendAuraRefineCancel();
	return Py_BuildNone();
}
#endif

void initnet()
{
	static PyMethodDef s_methods[] =
	{
		{ "GetBettingGuildWarValue",			netGetBettingGuildWarValue,				METH_VARARGS },
		{ "EnableChatInsultFilter",				netEnableChatInsultFilter,				METH_VARARGS },
		{ "SetServerInfo",						netSetServerInfo,						METH_VARARGS },
		{ "GetServerInfo",						netGetServerInfo,						METH_VARARGS },
		{ "PreserveServerCommand",				netPreserveServerCommand,				METH_VARARGS },
		{ "GetPreservedServerCommand",			netGetPreservedServerCommand,			METH_VARARGS },

		{ "StartGame",							netStartGame,							METH_VARARGS },
		{ "Warp",								netWarp,								METH_VARARGS },
		{ "IsTest",								netIsTest,								METH_VARARGS },
		{ "SetMarkServer",						netSetMarkServer,						METH_VARARGS },
		{ "IsChatInsultIn",						netIsChatInsultIn,						METH_VARARGS },
		{ "IsInsultIn",							netIsInsultIn,							METH_VARARGS },
		{ "LoadInsultList",						netLoadInsultList,						METH_VARARGS },
		{ "UploadMark",							netUploadMark,							METH_VARARGS },
		{ "UploadSymbol",						netUploadSymbol,						METH_VARARGS },
		{ "GetGuildID",							netGetGuildID,							METH_VARARGS },
		{ "GetEmpireID",						netGetEmpireID,							METH_VARARGS },
		{ "GetMainActorVID",					netGetMainActorVID,						METH_VARARGS },
		{ "GetMainActorRace",					netGetMainActorRace,					METH_VARARGS },
		{ "GetMainActorEmpire",					netGetMainActorEmpire,					METH_VARARGS },
		{ "GetMainActorSkillGroup",				netGetMainActorSkillGroup,				METH_VARARGS },
		{ "GetAccountCharacterSlotDataInteger",	netGetAccountCharacterSlotDataInteger,	METH_VARARGS },
		{ "GetAccountCharacterSlotDataString",	netGetAccountCharacterSlotDataString,	METH_VARARGS },

		// FIELD_MUSIC
		{ "GetFieldMusicFileName",				netGetFieldMusicFileName,				METH_VARARGS },
		{ "GetFieldMusicVolume",				netGetFieldMusicVolume,					METH_VARARGS },
		// END_OF_FIELD_MUSIC

		{ "ToggleGameDebugInfo",				netToggleGameDebugInfo,					METH_VARARGS },
#if defined(__BL_PICK_FILTER__)
		{ "OpenPickUpWindow",					netOpenPickUpWindow,					METH_VARARGS },
#endif
		{ "SetLoginInfo",						netSetLoginInfo,						METH_VARARGS },
		{ "SetPhaseWindow",						netSetPhaseWindow,						METH_VARARGS },
		{ "ClearPhaseWindow",					netClearPhaseWindow,					METH_VARARGS },
		{ "SetServerCommandParserWindow",		netSetServerCommandParserWindow,		METH_VARARGS },
		{ "SetAccountConnectorHandler",			netSetAccountConnectorHandler,			METH_VARARGS },
		{ "SetHandler",							netSetHandler,							METH_VARARGS },
		{ "SetTCPRecvBufferSize",				netSetTCPRecvBufferSize,				METH_VARARGS },
		{ "SetTCPSendBufferSize",				netSetTCPSendBufferSize,				METH_VARARGS },
		{ "SetUDPRecvBufferSize",				netSetUDPRecvBufferSize,				METH_VARARGS },
		{ "DirectEnter",						netDirectEnter,							METH_VARARGS },

		{ "LogOutGame",							netLogOutGame,							METH_VARARGS },
		{ "ExitGame",							netExitGame,							METH_VARARGS },
		{ "ExitApplication",					netExitApplication,						METH_VARARGS },
		{ "ConnectTCP",							netConnectTCP,							METH_VARARGS },
		{ "ConnectUDP",							netConnectUDP,							METH_VARARGS },
		{ "ConnectToAccountServer",				netConnectToAccountServer,				METH_VARARGS },

		{ "SendLoginPacket",					netSendLoginPacket,						METH_VARARGS },
		{ "SendSelectEmpirePacket",				netSendSelectEmpirePacket,				METH_VARARGS },
		{ "SendSelectCharacterPacket",			netSendSelectCharacterPacket,			METH_VARARGS },
		{ "SendChangeNamePacket",				netSendChangeNamePacket,				METH_VARARGS },
		{ "SendCreateCharacterPacket",			netSendCreateCharacterPacket,			METH_VARARGS },
		{ "SendDestroyCharacterPacket",			netSendDestroyCharacterPacket,			METH_VARARGS },
		{ "SendEnterGamePacket",				netSendEnterGamePacket,					METH_VARARGS },

		{ "SendItemUsePacket",					netSendItemUsePacket,					METH_VARARGS },
		{ "SendItemUseToItemPacket",			netSendItemUseToItemPacket,				METH_VARARGS },
#ifdef ENABLE_CHEST_OPEN_RENEWAL
		{ "SendOpenChestPacket",				netSendOpenChestPacket,					METH_VARARGS },
#endif
		{ "SendItemDropPacket",					netSendItemDropPacket,					METH_VARARGS },
		{ "SendItemDropPacketNew",				netSendItemDropPacketNew,				METH_VARARGS },
		{ "SendElkDropPacket",					netSendElkDropPacket,					METH_VARARGS },
		{ "SendGoldDropPacketNew",				netSendGoldDropPacketNew,				METH_VARARGS },
		{ "SendItemMovePacket",					netSendItemMovePacket,					METH_VARARGS },
		{ "SendItemPickUpPacket",				netSendItemPickUpPacket,				METH_VARARGS },
#ifdef ENABLE_SORT_INVEN
		{ "SortInven",							netSortInven,							METH_VARARGS },
#endif
		{ "SendGiveItemPacket",					netSendGiveItemPacket,					METH_VARARGS },

		{ "SetOfflinePhase",					netSetOfflinePhase,						METH_VARARGS },
		{ "Disconnect",							netDisconnect,							METH_VARARGS },
		{ "IsConnect",							netIsConnect,							METH_VARARGS },

		{ "SendChatPacket",						netSendChatPacket,						METH_VARARGS },
		{ "SendEmoticon",						netSendEmoticon,						METH_VARARGS },
		{ "SendWhisperPacket",					netSendWhisperPacket,					METH_VARARGS },
		{ "SendMobileMessagePacket",			netSendMobileMessagePacket,				METH_VARARGS },

		{ "SendCharacterPositionPacket",		netSendCharacterPositionPacket,			METH_VARARGS },

		{ "SendShopEndPacket",					netSendShopEndPacket,					METH_VARARGS },
		{ "SendShopBuyPacket",					netSendShopBuyPacket,					METH_VARARGS },
		{ "SendShopSellPacket",					netSendShopSellPacket,					METH_VARARGS },
		{ "SendShopSellPacketNew",				netSendShopSellPacketNew,				METH_VARARGS },

		{ "SendExchangeStartPacket",			netSendExchangeStartPacket,				METH_VARARGS },
#if defined(ENABLE_PARTY_MATCH)
		{ "SendPartyMatch",				netSendPartyMatch,				METH_VARARGS },
#endif
		{ "SendExchangeItemAddPacket",			netSendExchangeItemAddPacket,			METH_VARARGS },
		{ "SendExchangeItemDelPacket",			netSendExchangeItemDelPacket,			METH_VARARGS },
		{ "SendExchangeElkAddPacket",			netSendExchangeElkAddPacket,			METH_VARARGS },
		{ "SendExchangeAcceptPacket",			netSendExchangeAcceptPacket,			METH_VARARGS },
		{ "SendExchangeExitPacket",				netSendExchangeExitPacket,				METH_VARARGS },

		{ "SendOnClickPacket",					netOnClickPacket,						METH_VARARGS },

		// Emoticon String
		{ "RegisterEmoticonString",				netRegisterEmoticonString,				METH_VARARGS },

		// Messenger
		{ "SendMessengerAddByVIDPacket",		netSendMessengerAddByVIDPacket,			METH_VARARGS },
		{ "SendMessengerAddByNamePacket",		netSendMessengerAddByNamePacket,		METH_VARARGS },
		{ "SendMessengerRemovePacket",			netSendMessengerRemovePacket,			METH_VARARGS },
		#ifdef ENABLE_MESSENGER_BLOCK
		{ "SendMessengerAddBlockByVIDPacket",		netSendMessengerAddBlockByVIDPacket,			METH_VARARGS },
		{ "SendMessengerAddBlockByNamePacket",		netSendMessengerAddBlockByNamePacket,			METH_VARARGS },
		{ "SendMessengerRemoveBlockPacket",			netSendMessengerBlockRemovePacket,				METH_VARARGS },
		#endif
		// Party
		{ "SendPartyInvitePacket",				netSendPartyInvitePacket,				METH_VARARGS },
		{ "SendPartyInviteAnswerPacket",		netSendPartyInviteAnswerPacket,			METH_VARARGS },
		{ "SendPartyExitPacket",				netSendPartyExitPacket,					METH_VARARGS },
		{ "SendPartyRemovePacket",				netSendPartyRemovePacketPID,			METH_VARARGS },
		{ "SendPartyRemovePacketPID",			netSendPartyRemovePacketPID,			METH_VARARGS },
		{ "SendPartyRemovePacketVID",			netSendPartyRemovePacketVID,			METH_VARARGS },
		{ "SendPartySetStatePacket",			netSendPartySetStatePacket,				METH_VARARGS },
		{ "SendPartyUseSkillPacket",			netSendPartyUseSkillPacket,				METH_VARARGS },
		{ "SendPartyParameterPacket",			netSendPartyParameterPacket,			METH_VARARGS },

		// Safebox
		{ "SendSafeboxSaveMoneyPacket",			netSendSafeboxSaveMoneyPacket,			METH_VARARGS },
		{ "SendSafeboxWithdrawMoneyPacket",		netSendSafeboxWithdrawMoneyPacket,		METH_VARARGS },
		{ "SendSafeboxCheckinPacket",			netSendSafeboxCheckinPacket,			METH_VARARGS },
		{ "SendSafeboxCheckoutPacket",			netSendSafeboxCheckoutPacket,			METH_VARARGS },
		{ "SendSafeboxItemMovePacket",			netSendSafeboxItemMovePacket,			METH_VARARGS },

		// Mall
		{ "SendMallCheckoutPacket",				netSendMallCheckoutPacket,				METH_VARARGS },
#ifdef BOT_KILLER
		{ "SendBotKillerAnswer",				netSendBotKillerAnswer,					METH_VARARGS },
#endif
		// Guild
		{ "SendAnswerMakeGuildPacket",				netSendAnswerMakeGuildPacket,				METH_VARARGS },
		{ "SendQuestInputStringPacket",				netSendQuestInputStringPacket,				METH_VARARGS },
		{ "SendQuestConfirmPacket",					netSendQuestConfirmPacket,					METH_VARARGS },
		{ "SendGuildAddMemberPacket",				netSendGuildAddMemberPacket,				METH_VARARGS },
		{ "SendGuildRemoveMemberPacket",			netSendGuildRemoveMemberPacket,				METH_VARARGS },
		{ "SendGuildChangeGradeNamePacket",			netSendGuildChangeGradeNamePacket,			METH_VARARGS },
		{ "SendGuildChangeGradeAuthorityPacket",	netSendGuildChangeGradeAuthorityPacket,		METH_VARARGS },
		{ "SendGuildOfferPacket",					netSendGuildOfferPacket,					METH_VARARGS },
		{ "SendGuildPostCommentPacket",				netSnedGuildPostCommentPacket,				METH_VARARGS },
		{ "SendGuildDeleteCommentPacket",			netSnedGuildDeleteCommentPacket,			METH_VARARGS },
		{ "SendGuildRefreshCommentsPacket",			netSendGuildRefreshCommentsPacket,			METH_VARARGS },
		{ "SendGuildChangeMemberGradePacket",		netSendGuildChangeMemberGradePacket,		METH_VARARGS },
		{ "SendGuildUseSkillPacket",				netSendGuildUseSkillPacket,					METH_VARARGS },
		{ "SendGuildChangeMemberGeneralPacket",		netSendGuildChangeMemberGeneralPacket,		METH_VARARGS },
		{ "SendGuildInviteAnswerPacket",			netSendGuildInviteAnswerPacket,				METH_VARARGS },
		{ "SendGuildChargeGSPPacket",				netSendGuildChargeGSPPacket,				METH_VARARGS },
		{ "SendGuildDepositMoneyPacket",			netSendGuildDepositMoneyPacket,				METH_VARARGS },
		{ "SendGuildWithdrawMoneyPacket",			netSendGuildWithdrawMoneyPacket,			METH_VARARGS },

		// Refine
		{ "SendRequestRefineInfoPacket",			netSendRequestRefineInfoPacket,				METH_VARARGS },
		{ "SendRefinePacket",						netSendRefinePacket,						METH_VARARGS },
		{ "SendSelectItemPacket",					netSendSelectItemPacket,					METH_VARARGS },
#if defined(__BL_MAILBOX__)
		{ "SendPostDelete",							netSendPostDelete,							METH_VARARGS },
		{ "SendPostGetItems",						netSendPostGetItems,						METH_VARARGS },
		{ "SendPostWriteConfirm",					netSendPostWriteConfirm,					METH_VARARGS },
		{ "SendPostWrite",							netSendPostWrite,							METH_VARARGS },
		{ "SendMailBoxClose",						netSendMailBoxClose,						METH_VARARGS },
		{ "SendPostAllDelete",						netSendPostAllDelete,						METH_VARARGS },
		{ "SendPostAllGetItems",					netSendPostAllGetItems,						METH_VARARGS },
		{ "RequestPostAddData",						netRequestPostAddData,						METH_VARARGS },
#endif
#if defined(__BL_MOVE_COSTUME_ATTR__)
		{ "SendItemCombinationPacket",				netSendItemCombinationPacket,				METH_VARARGS },
		{ "SendItemCombinationPacketCancel",		netSendItemCombinationPacketCancel,			METH_VARARGS },
#endif
		// SYSTEM
		{ "SetPacketSequenceMode",					netSetPacketSequenceMode,					METH_VARARGS },
		{ "SetEmpireLanguageMode",					netSetEmpireLanguageMode,					METH_VARARGS },

		// For Test
		{ "SetSkillGroupFake",						netSetSkillGroupFake,						METH_VARARGS },

		// Log
		{ "RegisterErrorLog",						netRegisterErrorLog,						METH_VARARGS },
#if defined(__BL_OFFICIAL_LOOT_FILTER__)
		{ "SendLootingSettings",					netSendLootingSettings,						METH_VARARGS },
#endif
#ifdef ENABLE_GEM_SYSTEM
		{ "SendGemShopBuy",							netSendGemShopBuy,							METH_VARARGS },
		{ "SendGemShopAdd",							netSendGemShopAdd,							METH_VARARGS },
		{ "SendGemShopRefresh",						netSendGemShopRefresh,						METH_VARARGS },
#endif
#ifdef ELEMENT_SPELL_WORLDARD
		{ "ElementsSpellClose",						netSendElementsSpellClose,						METH_VARARGS },
		{ "ElementsSpellAdd",						netSendElementsSpellAdd,						METH_VARARGS },
		{ "ElementsSpellChange",					netSendElementsSpellChange,						METH_VARARGS },
#endif
#if defined(__BL_67_ATTR__)
		{ "Send67AttrPacket",						netSend67AttrPacket,						METH_VARARGS },
		{ "Send67AttrClosePacket",					netSend67AttrClosePacket,					METH_VARARGS },
#endif

#ifdef ENABLE_FISH_EVENT
		{ "SendUseFishBox",							netSendFishBoxUse,							METH_VARARGS },
		{ "SendAddFishBox",							netSendFishShapeAdd,						METH_VARARGS },
#endif

#if defined(__BL_TRANSMUTATION__)
		{ "SendChangeLookCheckIn",					netSendChangeLookCheckIn,					METH_VARARGS },
		{ "SendChangeLookCheckOut",					netSendChangeLookCheckOut,					METH_VARARGS },
		{ "SendChangeLookCheckInFreeYangItem",		netSendChangeLookCheckInFreeYangItem,		METH_VARARGS },
		{ "SendChangeLookCheckOutFreeYangItem",		netSendChangeLookCheckOutFreeYangItem,		METH_VARARGS },
		{ "SendChangeLookCheckOut",					netSendChangeLookCheckOut,					METH_VARARGS },
		{ "SendChangeLookAccept",					netSendChangeLookAccept,					METH_VARARGS },
		{ "SendChangeLookCanCle",					netSendChangeLookCancel,					METH_VARARGS },
#endif
#ifdef ENABLE_SHOP_SEARCH_SYSTEM
		{ "SendPrivateShopSearchInfo", netSendPrivateShopSearchInfo, METH_VARARGS },
#endif
#ifdef ENABLE_CHEQUE_SYSTEM
		{ "SendExchangeChequeAddPacket",			netSendExchangeChequeAddPacket,				METH_VARARGS },
#endif
#if defined(ENABLE_CHEQUE_SYSTEM) && !defined(DISABLE_CHEQUE_DROP)
		{ "SendGoldChequePacketNew",				netSendChequeDropPacketNew,					METH_VARARGS },
#endif
#ifdef ENABLE_CAPTCHA
		{ "SendCaptcha",							netSendCaptcha,								METH_VARARGS },
#endif

#ifdef ENABLE_OFFLINESHOP_SYSTEM
		{ "SendOfflineShopEndPacket", netSendOfflineShopEndPacket, METH_VARARGS },
		{ "SendOfflineShopBuyPacket", netSendOfflineShopBuyPacket, METH_VARARGS },
		{ "SendAddOfflineShopItem", netSendAddOfflineShopItem, METH_VARARGS },
		{ "SendRemoveOfflineShopItem", netSendRemoveOfflineShopItem, METH_VARARGS },
		{ "SendDestroyOfflineShop", netSendDestroyOfflineShop, METH_VARARGS },
		{ "SendTakeOfflineMoney", netSendTakeOfflineMoney, METH_VARARGS },
		{ "SendOfflineShopChangeTitle", netSendOfflineShopChangeTitle, METH_VARARGS },
		{ "SendOfflineShopCheck", netSendOfflineShopCheck, METH_VARARGS },
		{ "SendOfflineShopButton", netSendOfflineShopButton, METH_VARARGS },
		{ "SendOfflineShopRemoveLogs", netSendOfflineShopRemoveLogs, METH_VARARGS },
		{ "SendOpenShopSlot", netSendOpenShopSlot, METH_VARARGS },
		{ "SendGetBackItems", netSendGetBackItems, METH_VARARGS },
		{ "SendAddTime", netSendAddTime, METH_VARARGS },
		{ "SendOpenWithVID", netSendOpenWithVID, METH_VARARGS },
#endif

#if defined(ENABLE_SKILLBOOK_COMB_SYSTEM)
		{ "SendSkillBookCombinationPacket", netSendSkillBookCombinationPacket, METH_VARARGS },
#endif

#ifdef ENABLE_MINI_GAME_CATCH_KING
		{ "SendMiniGameCatchKingStart",				netSendMiniGameCatchKingStart,				METH_VARARGS },
		{ "SendMiniGameCatchKingDeckCardClick",		netSendMiniGameCatchKingDeckCardClick,		METH_VARARGS },
		{ "SendMiniGameCatchKingFieldCardClick",	netSendMiniGameCatchKingFieldCardClick,		METH_VARARGS },
		{ "SendMiniGameCatchKingReward",			netSendMiniGameCatchKingReward,				METH_VARARGS },
		{ "OpenCatchKing",							netOpenCatchKing,							METH_VARARGS },
#endif

#ifdef ENABLE_MINI_GAME_YUTNORI
		{ "SendMiniGameYutNori",					netSendMiniGameYutNori,						METH_VARARGS },
#endif
#ifdef ENABLE_TAROT
		{ "MinigameBnwStart",						netMinigameBnwStart,						METH_VARARGS },
		{ "MinigameBnwCardSelected",				netMinigameBnwCardSelected,					METH_VARARGS },
		{ "MinigameBnwFinished",					netMinigameBnwFinished,						METH_VARARGS },
#endif
#ifdef ENABLE_AURA_SYSTEM
		{ "SendAuraRefineCheckIn",					netSendAuraRefineCheckIn,					METH_VARARGS },
		{ "SendAuraRefineCheckOut",					netSendAuraRefineCheckOut,					METH_VARARGS },
		{ "SendAuraRefineAccept",					netSendAuraRefineAccept,					METH_VARARGS },
		{ "SendAuraRefineCancel",					netSendAuraRefineCancel,					METH_VARARGS },
#endif
		{ NULL,										NULL,										NULL },
	};

	PyObject* poModule = Py_InitModule("net", s_methods);

	PyModule_AddIntConstant(poModule, "ERROR_NONE", CPythonNetworkStream::ERROR_NONE);
	PyModule_AddIntConstant(poModule, "ERROR_CONNECT_MARK_SERVER", CPythonNetworkStream::ERROR_CONNECT_MARK_SERVER);
	PyModule_AddIntConstant(poModule, "ERROR_LOAD_MARK", CPythonNetworkStream::ERROR_LOAD_MARK);
	PyModule_AddIntConstant(poModule, "ERROR_MARK_WIDTH", CPythonNetworkStream::ERROR_MARK_WIDTH);
	PyModule_AddIntConstant(poModule, "ERROR_MARK_HEIGHT", CPythonNetworkStream::ERROR_MARK_HEIGHT);

	// MARK_BUG_FIX
	PyModule_AddIntConstant(poModule, "ERROR_MARK_UPLOAD_NEED_RECONNECT", CPythonNetworkStream::ERROR_MARK_UPLOAD_NEED_RECONNECT);
	PyModule_AddIntConstant(poModule, "ERROR_MARK_CHECK_NEED_RECONNECT", CPythonNetworkStream::ERROR_MARK_CHECK_NEED_RECONNECT);
	// END_OF_MARK_BUG_FIX

	PyModule_AddIntConstant(poModule, "PHASE_WINDOW_LOGIN", CPythonNetworkStream::PHASE_WINDOW_LOGIN);
	PyModule_AddIntConstant(poModule, "PHASE_WINDOW_SELECT", CPythonNetworkStream::PHASE_WINDOW_SELECT);
	PyModule_AddIntConstant(poModule, "PHASE_WINDOW_CREATE", CPythonNetworkStream::PHASE_WINDOW_CREATE);
	PyModule_AddIntConstant(poModule, "PHASE_WINDOW_LOAD", CPythonNetworkStream::PHASE_WINDOW_LOAD);
	PyModule_AddIntConstant(poModule, "PHASE_WINDOW_GAME", CPythonNetworkStream::PHASE_WINDOW_GAME);
	PyModule_AddIntConstant(poModule, "PHASE_WINDOW_EMPIRE", CPythonNetworkStream::PHASE_WINDOW_EMPIRE);
	PyModule_AddIntConstant(poModule, "PHASE_WINDOW_LOGO", CPythonNetworkStream::PHASE_WINDOW_LOGO);

	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_ID", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_ID);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_NAME", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_NAME);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_RACE", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_RACE);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_LEVEL", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_LEVEL);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_STR", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_STR);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_DEX", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_DEX);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_INT", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_INT);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_HTH", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_HTH);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_PLAYTIME", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_PLAYTIME);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_FORM", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_FORM);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_ADDR", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_ADDR);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_PORT", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_PORT);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_GUILD_ID", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_GUILD_ID);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_GUILD_NAME", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_GUILD_NAME);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_CHANGE_NAME_FLAG", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_CHANGE_NAME_FLAG);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_HAIR", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_HAIR);
#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_ACCE", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_ACCE);
#endif
#ifdef ENABLE_CONQUEROR_LEVEL
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_CONQUEROR_LEVEL", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_CONQUEROR_LEVEL);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_SUNGMA_ST", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_SUNGMA_ST);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_SUNGMA_HP", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_SUNGMA_HP);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_SUNGMA_MOVE", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_SUNGMA_MOVE);
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_SUNGMA_INMUNE", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_SUNGMA_INMUNE);
#endif
#ifdef ENABLE_AURA_SYSTEM
	PyModule_AddIntConstant(poModule, "ACCOUNT_CHARACTER_SLOT_AURA", CPythonNetworkStream::ACCOUNT_CHARACTER_SLOT_AURA);
#endif
	PyModule_AddIntConstant(poModule, "SERVER_COMMAND_LOG_OUT",	CPythonNetworkStream::SERVER_COMMAND_LOG_OUT);
	PyModule_AddIntConstant(poModule, "SERVER_COMMAND_RETURN_TO_SELECT_CHARACTER",	CPythonNetworkStream::SERVER_COMMAND_RETURN_TO_SELECT_CHARACTER);
	PyModule_AddIntConstant(poModule, "SERVER_COMMAND_QUIT",	CPythonNetworkStream::SERVER_COMMAND_QUIT);

	PyModule_AddIntConstant(poModule, "EMPIRE_A", 1);
	PyModule_AddIntConstant(poModule, "EMPIRE_B", 2);
	PyModule_AddIntConstant(poModule, "EMPIRE_C", 3);
	PyModule_AddIntConstant(poModule, "DS_SUB_HEADER_REFINE_FAIL", DS_SUB_HEADER_REFINE_FAIL);
	PyModule_AddIntConstant(poModule, "DS_SUB_HEADER_REFINE_FAIL_MAX_REFINE", DS_SUB_HEADER_REFINE_FAIL_MAX_REFINE);
	PyModule_AddIntConstant(poModule, "DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL", DS_SUB_HEADER_REFINE_FAIL_INVALID_MATERIAL);
	PyModule_AddIntConstant(poModule, "DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MONEY", DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MONEY);
	PyModule_AddIntConstant(poModule, "DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL", DS_SUB_HEADER_REFINE_FAIL_NOT_ENOUGH_MATERIAL);
	PyModule_AddIntConstant(poModule, "DS_SUB_HEADER_REFINE_FAIL_TOO_MUCH_MATERIAL", DS_SUB_HEADER_REFINE_FAIL_TOO_MUCH_MATERIAL);
	PyModule_AddIntConstant(poModule, "DS_SUB_HEADER_REFINE_SUCCEED", DS_SUB_HEADER_REFINE_SUCCEED);
}
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
