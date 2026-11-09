#include "stdafx.h"
#ifdef ENABLE_SHOP_SEARCH_SYSTEM
#include "PythonPrivateShopSearch.h"
#include "Packet.h"
#ifdef PYTHON_DYNAMIC_MODULE_NAME
#include "PythonDynamicModuleNames.h"
#endif

CPythonPrivateShopSearch::CPythonPrivateShopSearch() {}

CPythonPrivateShopSearch::~CPythonPrivateShopSearch() {}

void CPythonPrivateShopSearch::AddItemData(TOfflineShopItemData& rItemData)
{
	m_ItemInstanceVector.push_back(rItemData);
}

void CPythonPrivateShopSearch::ClearItemData()
{
	m_ItemInstanceVector.clear();
}

DWORD CPythonPrivateShopSearch::GetItemDataPtr(DWORD index, TOfflineShopItemData** ppInstance)
{
	*ppInstance = &m_ItemInstanceVector.at(index);
	return 1;
}

PyObject* privateShopSearchGetSearchItemMetinSocket(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
		return Py_BadArgument();
	int iSocketIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iSocketIndex))
		return Py_BadArgument();

	if (iSocketIndex >= ITEM_SOCKET_SLOT_MAX_NUM)
		return Py_BuildException();
	TOfflineShopItemData* pItemData;
	if (!CPythonPrivateShopSearch::Instance().GetItemDataPtr(iSlotIndex, &pItemData))
	{
		return Py_BuildException();
	}

	return Py_BuildValue("i", pItemData->alSockets[iSocketIndex]);
}

PyObject* privateShopSearchGetSearchItemAttribute(PyObject* poSelf, PyObject* poArgs)
{
	int iSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iSlotIndex))
	{
		return Py_BuildException();
	}
	int iAttrSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iAttrSlotIndex))
	{
		return Py_BuildException();
	}

	if (iAttrSlotIndex >= 0 && iAttrSlotIndex < ITEM_ATTRIBUTE_SLOT_MAX_NUM)
	{
		TOfflineShopItemData* pItemData;
		if (CPythonPrivateShopSearch::Instance().GetItemDataPtr(iSlotIndex, &pItemData))
		{
			return Py_BuildValue("ii", pItemData->aAttr[iAttrSlotIndex].bType, pItemData->aAttr[iAttrSlotIndex].sValue);
		}
	}

	return Py_BuildValue("ii", 0, 0);
}

PyObject* privateShopSearchGetItemCount(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonPrivateShopSearch::Instance().GetItemDataCount());
}

PyObject* privateShopSearchGetSearchItemCount(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
	{
		return Py_BadArgument();
	}

	TOfflineShopItemData* pInstance;
	if (!CPythonPrivateShopSearch::Instance().GetItemDataPtr(ipos, &pInstance))
	{
		return Py_BuildException();
	}

	return Py_BuildValue("i", pInstance->count);
}

PyObject* privateShopSearchGetSearchItemPrice(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
	{
		return Py_BadArgument();
	}

	TOfflineShopItemData* pInstance;
	if (!CPythonPrivateShopSearch::Instance().GetItemDataPtr(ipos, &pInstance))
	{
		return Py_BuildException();
	}

	return PyLong_FromLongLong(pInstance->price);
}

PyObject* privateShopSearchGetSearchItemShopVID(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
	{
		return Py_BadArgument();
	}

	TOfflineShopItemData* pInstance;
	if (!CPythonPrivateShopSearch::Instance().GetItemDataPtr(ipos, &pInstance))
	{
		return Py_BuildException();
	}

	return Py_BuildValue("i", pInstance->owner_id);
}

PyObject* privateShopSearchGetSearchItemPos(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
	{
		return Py_BadArgument();
	}

	TOfflineShopItemData* pInstance;
	if (!CPythonPrivateShopSearch::Instance().GetItemDataPtr(ipos, &pInstance))
	{
		return Py_BuildException();
	}

	return Py_BuildValue("i", pInstance->display_pos);
}

PyObject* privateShopSearchGetSearchItemVnum(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
	{
		return Py_BadArgument();
	}

	TOfflineShopItemData* pInstance;
	if (!CPythonPrivateShopSearch::Instance().GetItemDataPtr(ipos, &pInstance))
	{
		return Py_BuildException();
	}

	return Py_BuildValue("i", pInstance->vnum);
}

#ifdef WJ_CHANGELOOK_SYSTEM
PyObject* privateShopSearchGetSearchItemTransmutation(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
	{
		return Py_BadArgument();
	}

	TOfflineShopItemData* pInstance;
	if (CPythonPrivateShopSearch::Instance().GetItemDataPtr(ipos, &pInstance))
		return Py_BuildValue("i", pInstance->transmutation);

	return Py_BuildValue("i",0);
}
#endif

#ifdef ENABLE_NEW_NAME_ITEM
PyObject* privateShopSearchGetSearchItemNewName(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
	{
		return Py_BadArgument();
	}
	TOfflineShopItemData* pInstance;
	if (CPythonPrivateShopSearch::Instance().GetItemDataPtr(ipos, &pInstance))
		return Py_BuildValue("s", pInstance->name);
	return Py_BuildValue("s", "^");
}
#endif
#ifdef ENABLE_PERMA_ITEM
PyObject* privateShopSearchGetSearchItemPerma(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
	{
		return Py_BadArgument();
	}
	TOfflineShopItemData* pInstance;
	if (CPythonPrivateShopSearch::Instance().GetItemDataPtr(ipos, &pInstance))
		return Py_BuildValue("i", pInstance->perma);
	return Py_BuildValue("i", 0);
}
#endif


PyObject* privateShopSearchGetSearchItemBuyerName(PyObject* poSelf, PyObject* poArgs)
{
	int ipos;
	if (!PyTuple_GetInteger(poArgs, 0, &ipos))
	{
		return Py_BadArgument();
	}

	TOfflineShopItemData* pInstance;
	if (!CPythonPrivateShopSearch::Instance().GetItemDataPtr(ipos, &pInstance))
	{
		return Py_BuildException();
	}

	return Py_BuildValue("s", pInstance->szBuyerName);
}

PyObject* privateShopSearchClearSearchItems(PyObject* poSelf, PyObject* poArgs)
{
	CPythonPrivateShopSearch::Instance().ClearItemData();
	return Py_BuildNone();
}

void initprivateShopSearch()
{
	static PyMethodDef s_methods[] =
	{
		{ "GetItemCount",		privateShopSearchGetItemCount,			METH_VARARGS },
		{ "GetSearchItemCount",			privateShopSearchGetSearchItemCount,				METH_VARARGS },
		{ "GetSearchItemPrice",			privateShopSearchGetSearchItemPrice,				METH_VARARGS },
		{ "GetSearchItemVnum",			privateShopSearchGetSearchItemVnum,				METH_VARARGS },
		{ "GetSearchItemShopVID",			privateShopSearchGetSearchItemShopVID,				METH_VARARGS },
		{ "ClearSearchItems", privateShopSearchClearSearchItems, METH_VARARGS },
		{ "GetSearchItemMetinSocket",		privateShopSearchGetSearchItemMetinSocket,			METH_VARARGS },
		{ "GetSearchItemAttribute",		privateShopSearchGetSearchItemAttribute,			METH_VARARGS },
#ifdef WJ_CHANGELOOK_SYSTEM
		{ "GetSearchItemTransmutation",	privateShopSearchGetSearchItemTransmutation,		METH_VARARGS },
#endif
#ifdef ENABLE_NEW_NAME_ITEM
		{ "GetSearchItemNewName",	privateShopSearchGetSearchItemNewName,		METH_VARARGS },
#endif
#ifdef ENABLE_PERMA_ITEM
		{ "GetSearchItemPerma",	privateShopSearchGetSearchItemPerma,		METH_VARARGS },
#endif
		{ "GetSearchItemPos", privateShopSearchGetSearchItemPos, METH_VARARGS	},
		{ "GetSearchItemSeller", privateShopSearchGetSearchItemBuyerName, METH_VARARGS },
		{ NULL,							NULL,									NULL },
	};

#ifdef PYTHON_DYNAMIC_MODULE_NAME
	PyObject* poModule = Py_InitModule(GetModuleName(SHOP_SEARCH_MODULE).c_str(), s_methods);
#else
	PyObject* poModule = Py_InitModule("privateShopSearch", s_methods);
#endif
}
#endif

