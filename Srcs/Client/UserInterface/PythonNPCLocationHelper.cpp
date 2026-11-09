#include "stdafx.h"
#if defined(ENABLE_NPC_LOCATION_HELPER)
#include "PythonNPCLocationHelper.h"
#include "PythonNetworkStream.h"
#include "../EterPack/EterPackManager.h"

namespace
{
CPythonNPCLocationHelper gs_npcLocationHelper;
}

CPythonNPCLocationHelper::CPythonNPCLocationHelper()
{
	Clear();
}

void CPythonNPCLocationHelper::Clear()
{
	m_bActive = false;
	m_dwCooldownRemain = 0;
	m_bLastResult = 0;
}

void CPythonNPCLocationHelper::SetStatus(const TPacketGCNPCLocationHelper& packet)
{
	m_bActive = packet.active != 0;
	m_dwCooldownRemain = packet.cooldownRemain;
}

void CPythonNPCLocationHelper::SetWarpResult(const TPacketGCNPCLocationHelper& packet)
{
	SetStatus(packet);
	m_bLastResult = packet.result;
}

bool CPythonNPCLocationHelper::IsActive() const
{
	return m_bActive;
}

DWORD CPythonNPCLocationHelper::GetCooldownRemain() const
{
	return m_dwCooldownRemain;
}

BYTE CPythonNPCLocationHelper::GetLastResult() const
{
	return m_bLastResult;
}

PyObject* npcLocationHelperRequestStatus(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().SendNPCLocationHelperPacket(NPC_LOCATION_HELPER_CG_REQUEST_STATUS, 0, 0, 0, 0);
	return Py_BuildNone();
}

PyObject* npcLocationHelperWarpToNPC(PyObject* poSelf, PyObject* poArgs)
{
	int iMapIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iMapIndex))
		return Py_BuildException();

	int iVnum;
	if (!PyTuple_GetInteger(poArgs, 1, &iVnum))
		return Py_BuildException();

	int iX = 0;
	if (!PyTuple_GetInteger(poArgs, 2, &iX))
		return Py_BuildException();

	int iY = 0;
	if (!PyTuple_GetInteger(poArgs, 3, &iY))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendNPCLocationHelperPacket(NPC_LOCATION_HELPER_CG_WARP_TO_NPC, iMapIndex, iVnum, iX, iY);
	return Py_BuildNone();
}

PyObject* npcLocationHelperUseTicket(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().SendNPCLocationHelperPacket(NPC_LOCATION_HELPER_CG_USE_TICKET, 0, 0, 0, 0);
	return Py_BuildNone();
}

PyObject* npcLocationHelperRequestGuildLand(PyObject* poSelf, PyObject* poArgs)
{
	int iMapIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iMapIndex))
		return Py_BuildException();

	CPythonNetworkStream::Instance().SendNPCLocationHelperPacket(NPC_LOCATION_HELPER_CG_REQUEST_GUILD_LAND, iMapIndex, 0, 0, 0);
	return Py_BuildNone();
}

PyObject* npcLocationHelperIsActive(PyObject* poSelf, PyObject* poArgs)
{
	if (!CPythonNPCLocationHelper::InstancePtr())
		return Py_BuildValue("i", 0);
	return Py_BuildValue("i", CPythonNPCLocationHelper::Instance().IsActive() ? 1 : 0);
}

PyObject* npcLocationHelperGetCooldownRemain(PyObject* poSelf, PyObject* poArgs)
{
	if (!CPythonNPCLocationHelper::InstancePtr())
		return Py_BuildValue("i", 0);
	return Py_BuildValue("i", CPythonNPCLocationHelper::Instance().GetCooldownRemain());
}

PyObject* npcLocationHelperGetLastResult(PyObject* poSelf, PyObject* poArgs)
{
	if (!CPythonNPCLocationHelper::InstancePtr())
		return Py_BuildValue("i", 0);
	return Py_BuildValue("i", CPythonNPCLocationHelper::Instance().GetLastResult());
}

PyObject* npcLocationHelperReadTextFile(PyObject* poSelf, PyObject* poArgs)
{
	char* szFileName;
	if (!PyTuple_GetString(poArgs, 0, &szFileName))
		return Py_BuildException();

	CMappedFile file;
	LPCVOID pvData = NULL;
	if (!CEterPackManager::Instance().Get(file, szFileName, &pvData))
		return Py_BuildValue("[]");

	const char* pData = static_cast<const char*>(pvData);
	const DWORD dwSize = file.Size();
	PyObject* poList = PyList_New(0);
	std::string line;

	for (DWORD i = 0; i < dwSize; ++i)
	{
		const char ch = pData[i];
		if (ch == '\n')
		{
			PyObject* poLine = Py_BuildValue("s", line.c_str());
			PyList_Append(poList, poLine);
			Py_DECREF(poLine);
			line.clear();
		}
		else if (ch != '\r')
		{
			line.push_back(ch);
		}
	}

	if (!line.empty())
	{
		PyObject* poLine = Py_BuildValue("s", line.c_str());
		PyList_Append(poList, poLine);
		Py_DECREF(poLine);
	}

	return poList;
}

void initNPCLocationHelper()
{
	static PyMethodDef s_methods[] =
	{
		{ "RequestStatus", npcLocationHelperRequestStatus, METH_VARARGS },
		{ "WarpToNPC", npcLocationHelperWarpToNPC, METH_VARARGS },
		{ "UseTicket", npcLocationHelperUseTicket, METH_VARARGS },
		{ "RequestGuildLand", npcLocationHelperRequestGuildLand, METH_VARARGS },
		{ "IsActive", npcLocationHelperIsActive, METH_VARARGS },
		{ "GetCooldownRemain", npcLocationHelperGetCooldownRemain, METH_VARARGS },
		{ "GetLastResult", npcLocationHelperGetLastResult, METH_VARARGS },
		{ "ReadTextFile", npcLocationHelperReadTextFile, METH_VARARGS },
		{ NULL, NULL, NULL },
	};

	PyObject* poModule = Py_InitModule("npcLocationHelper", s_methods);
	PyModule_AddIntConstant(poModule, "CG_REQUEST_STATUS", NPC_LOCATION_HELPER_CG_REQUEST_STATUS);
	PyModule_AddIntConstant(poModule, "CG_WARP_TO_NPC", NPC_LOCATION_HELPER_CG_WARP_TO_NPC);
	PyModule_AddIntConstant(poModule, "CG_USE_TICKET", NPC_LOCATION_HELPER_CG_USE_TICKET);
	PyModule_AddIntConstant(poModule, "CG_REQUEST_GUILD_LAND", NPC_LOCATION_HELPER_CG_REQUEST_GUILD_LAND);
	PyModule_AddIntConstant(poModule, "GC_STATUS", NPC_LOCATION_HELPER_GC_STATUS);
	PyModule_AddIntConstant(poModule, "GC_WARP_RESULT", NPC_LOCATION_HELPER_GC_WARP_RESULT);
	PyModule_AddIntConstant(poModule, "GC_GUILD_LAND", NPC_LOCATION_HELPER_GC_GUILD_LAND);
	PyModule_AddIntConstant(poModule, "RESULT_OK", 0);
	PyModule_AddIntConstant(poModule, "RESULT_NOT_ACTIVE", 1);
	PyModule_AddIntConstant(poModule, "RESULT_CANNOT_USE", 2);
	PyModule_AddIntConstant(poModule, "RESULT_COOLDOWN", 3);
	PyModule_AddIntConstant(poModule, "RESULT_INVALID_TARGET", 4);
	PyModule_AddIntConstant(poModule, "RESULT_LEVEL_LIMIT", 5);
	PyModule_AddIntConstant(poModule, "RESULT_EMPIRE_LIMIT", 6);
	PyModule_AddIntConstant(poModule, "RESULT_NEED_ITEM", 7);
	PyModule_AddIntConstant(poModule, "RESULT_NEED_YANG", 8);
	PyModule_AddIntConstant(poModule, "ATLAS_SCALE_MIN", 1);
	PyModule_AddIntConstant(poModule, "ATLAS_SCALE_MAX", 3);
}
#endif
