#include "StdAfx.h"
#include "PythonMiniMap.h"

PyObject * miniMapSetScale(PyObject * poSelf, PyObject * poArgs)
{
	float fScale;
	if (!PyTuple_GetFloat(poArgs, 0, &fScale))
		return Py_BuildException();

	CPythonMiniMap::Instance().SetScale(fScale);
	return Py_BuildNone();
}

PyObject * miniMapSetCenterPosition(PyObject * poSelf, PyObject * poArgs)
{
	float fCenterX;
	if (!PyTuple_GetFloat(poArgs, 0, &fCenterX))
		return Py_BuildException();

	float fCenterY;
	if (!PyTuple_GetFloat(poArgs, 1, &fCenterY))
		return Py_BuildException();

	CPythonMiniMap::Instance().SetCenterPosition(fCenterX, fCenterY);
	return Py_BuildNone();
}

PyObject * miniMapSetMiniMapSize(PyObject * poSelf, PyObject * poArgs)
{
	float fWidth;
	if (!PyTuple_GetFloat(poArgs, 0, &fWidth))
		return Py_BuildException();

	float fHeight;
	if (!PyTuple_GetFloat(poArgs, 1, &fHeight))
		return Py_BuildException();

	CPythonMiniMap::Instance().SetMiniMapSize(fWidth, fHeight);
	return Py_BuildNone();
}

PyObject * miniMapDestroy(PyObject * poSelf, PyObject * poArgs)
{
	CPythonMiniMap::Instance().Destroy();
	return Py_BuildNone();
}

PyObject * miniMapCreate(PyObject * poSelf, PyObject * poArgs)
{
	CPythonMiniMap::Instance().Create();
	return Py_BuildNone();
}

PyObject * miniMapUpdate(PyObject * poSelf, PyObject * poArgs)
{
	float fCenterX;
	if (!PyTuple_GetFloat(poArgs, 0, &fCenterX))
		return Py_BuildException();

	float fCenterY;
	if (!PyTuple_GetFloat(poArgs, 1, &fCenterY))
		return Py_BuildException();

	CPythonMiniMap::Instance().Update(fCenterX, fCenterY);
	return Py_BuildNone();
}

PyObject * miniMapRender(PyObject * poSelf, PyObject * poArgs)
{
	float fScrrenX;
	if (!PyTuple_GetFloat(poArgs, 0, &fScrrenX))
		return Py_BuildException();

	float fScrrenY;
	if (!PyTuple_GetFloat(poArgs, 1, &fScrrenY))
		return Py_BuildException();

	CPythonMiniMap::Instance().Render(fScrrenX, fScrrenY);
	return Py_BuildNone();
}

PyObject * miniMapShow(PyObject * poSelf, PyObject * poArgs)
{
	CPythonMiniMap::Instance().Show();
	return Py_BuildNone();
}

PyObject * miniMapHide(PyObject * poSelf, PyObject * poArgs)
{
	CPythonMiniMap::Instance().Hide();
	return Py_BuildNone();
}

PyObject * miniMapisShow(PyObject * poSelf, PyObject * poArgs)
{
	bool bShow = CPythonMiniMap::Instance().CanShow();
	return Py_BuildValue("b", bShow);
}

PyObject * miniMapScaleUp(PyObject * poSelf, PyObject * poArgs)
{
	CPythonMiniMap::Instance().ScaleUp();
	return Py_BuildNone();
}

PyObject * miniMapScaleDown(PyObject * poSelf, PyObject * poArgs)
{
	CPythonMiniMap::Instance().ScaleDown();
	return Py_BuildNone();
}

PyObject * miniMapGetInfo(PyObject * poSelf, PyObject * poArgs)
{
	float fScrrenX;
	if (!PyTuple_GetFloat(poArgs, 0, &fScrrenX))
		return Py_BuildException();

	float fScrrenY;
	if (!PyTuple_GetFloat(poArgs, 1, &fScrrenY))
		return Py_BuildException();

	std::string aString;
	float fPosX, fPosY;
	DWORD dwTextColor;
	bool bFind = CPythonMiniMap::Instance().GetPickedInstanceInfo(fScrrenX, fScrrenY, aString, &fPosX, &fPosY, &dwTextColor);
	int iPosX, iPosY;
	PR_FLOAT_TO_INT(fPosX, iPosX);
	PR_FLOAT_TO_INT(fPosY, iPosY);
	iPosX /= 100;
	iPosY /= 100;
	return Py_BuildValue("isiil", (int)bFind, aString.c_str(), iPosX, iPosY, (signed) dwTextColor);
}

//////////////////////////////////////////////////////////////////////////
// Atlas
PyObject * miniMapLoadAtlas(PyObject * poSelf, PyObject * poArgs)
{
	if (!CPythonMiniMap::Instance().LoadAtlas())
		TraceError("CPythonMiniMap::Instance().LoadAtlas() Failed");
	return Py_BuildNone();
}

PyObject * miniMapUpdateAtlas(PyObject * poSelf, PyObject * poArgs)
{
	CPythonMiniMap::Instance().UpdateAtlas();
	return Py_BuildNone();
}

PyObject * miniMapRenderAtlas(PyObject * poSelf, PyObject * poArgs)
{
	float fScrrenX;
	if (!PyTuple_GetFloat(poArgs, 0, &fScrrenX))
		return Py_BuildException();

	float fScrrenY;
	if (!PyTuple_GetFloat(poArgs, 1, &fScrrenY))
		return Py_BuildException();

	CPythonMiniMap::Instance().RenderAtlas(fScrrenX, fScrrenY);
	return Py_BuildNone();
}

PyObject * miniMapShowAtlas(PyObject * poSelf, PyObject * poArgs)
{
	CPythonMiniMap::Instance().ShowAtlas();
	return Py_BuildNone();
}

PyObject * miniMapHideAtlas(PyObject * poSelf, PyObject * poArgs)
{
	CPythonMiniMap::Instance().HideAtlas();
	return Py_BuildNone();
}

PyObject * miniMapisShowAtlas(PyObject * poSelf, PyObject * poArgs)
{
	bool bShow = CPythonMiniMap::Instance().CanShowAtlas();
	return Py_BuildValue("b", bShow);
}

PyObject * miniMapIsAtlas(PyObject * poSelf, PyObject * poArgs)
{
	bool isData = CPythonMiniMap::Instance().IsAtlas();
	return Py_BuildValue("b", isData);
}

PyObject * miniMapGetAtlasInfo(PyObject * poSelf, PyObject * poArgs)
{
	float fScrrenX;
	if (!PyTuple_GetFloat(poArgs, 0, &fScrrenX))
		return Py_BuildException();
	float fScrrenY;
	if (!PyTuple_GetFloat(poArgs, 1, &fScrrenY))
		return Py_BuildException();

	std::string aString = "";
	float fPosX = 0.0f;
	float fPosY = 0.0f;
	DWORD dwTextColor = 0;
	DWORD dwGuildID = 0;
	bool bFind = CPythonMiniMap::Instance().GetAtlasInfo(fScrrenX, fScrrenY, aString, &fPosX, &fPosY, &dwTextColor, &dwGuildID);
	int iPosX, iPosY;
	PR_FLOAT_TO_INT(fPosX, iPosX);
	PR_FLOAT_TO_INT(fPosY, iPosY);
	iPosX /= 100;
	iPosY /= 100;
	return Py_BuildValue("isiili", (int)bFind, aString.c_str(), iPosX, iPosY, (signed) dwTextColor, dwGuildID);
}

PyObject * miniMapGetAtlasSize(PyObject * poSelf, PyObject * poArgs)
{
	float fSizeX, fSizeY;
	bool bGet = CPythonMiniMap::Instance().GetAtlasSize(&fSizeX, &fSizeY);

/*
	float fSizeXoo256 = fSizeX / 256.0f;
	float fSizeYoo256 = fSizeY / 256.0f;

	if (fSizeXoo256 >= fSizeYoo256)
	{
		fSizeX /= fSizeYoo256;
		fSizeY = 256.0f;
	}
	else
	{
		fSizeX = 256.0f;
		fSizeY /= fSizeXoo256;
	}
*/

	int iSizeX, iSizeY;
	PR_FLOAT_TO_INT(fSizeX, iSizeX);
	PR_FLOAT_TO_INT(fSizeY, iSizeY);

	return Py_BuildValue("bii", (int)bGet, iSizeX, iSizeY);
}

PyObject * miniMapAddWayPoint(PyObject * poSelf, PyObject * poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();

	float fX;
	if (!PyTuple_GetFloat(poArgs, 1, &fX))
		return Py_BuildException();

	float fY;
	if (!PyTuple_GetFloat(poArgs, 2, &fY))
		return Py_BuildException();

	char * buf;
	if (!PyTuple_GetString(poArgs, 3, &buf))
		return Py_BuildException();

	CPythonMiniMap::Instance().AddWayPoint(CPythonMiniMap::TYPE_WAYPOINT, (DWORD)iID, fX, fY, buf);

	return Py_BuildNone();
}

PyObject * miniMapRemoveWayPoint(PyObject * poSelf, PyObject * poArgs)
{
	int iID;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();

	CPythonMiniMap::Instance().RemoveWayPoint((DWORD)iID);

	return Py_BuildNone();
}

PyObject* miniMapRegisterAtlasWindow(PyObject* poSelf, PyObject* poArgs)
{
	PyObject * poHandler;
	if (!PyTuple_GetObject(poArgs, 0, &poHandler))
		return Py_BuildException();
	CPythonMiniMap::Instance().RegisterAtlasWindow(poHandler);
	return Py_BuildNone();
}

PyObject* miniMapUnregisterAtlasWindow(PyObject* poSelf, PyObject* poArgs)
{
	CPythonMiniMap::Instance().UnregisterAtlasWindow();
	return Py_BuildNone();
}

PyObject* miniMapGetGuildAreaID(PyObject* poSelf, PyObject* poArgs)
{
	float fx;
	if (!PyTuple_GetFloat(poArgs, 0, &fx))
		return Py_BuildException();
	float fy;
	if (!PyTuple_GetFloat(poArgs, 1, &fy))
		return Py_BuildException();

	DWORD dwGuildID = CPythonMiniMap::Instance().GetGuildAreaID(fx, fy);
	return Py_BuildValue("i", dwGuildID);
}

#if defined(ENABLE_NPC_LOCATION_HELPER)
PyObject* miniMapSetAtlasScale(PyObject* poSelf, PyObject* poArgs)
{
	float fScale;
	if (!PyTuple_GetFloat(poArgs, 0, &fScale))
		return Py_BuildException();
	CPythonMiniMap::Instance().SetAtlasScale(fScale);
	return Py_BuildNone();
}

PyObject* miniMapGetAtlasScale(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("f", CPythonMiniMap::Instance().GetAtlasScale());
}

PyObject* miniMapAtlasScaleUp(PyObject* poSelf, PyObject* poArgs)
{
	CPythonMiniMap::Instance().AtlasScaleUp();
	return Py_BuildNone();
}

PyObject* miniMapAtlasScaleDown(PyObject* poSelf, PyObject* poArgs)
{
	CPythonMiniMap::Instance().AtlasScaleDown();
	return Py_BuildNone();
}

PyObject* miniMapChangeAtlasRenderPos(PyObject* poSelf, PyObject* poArgs)
{
	float fX;
	if (!PyTuple_GetFloat(poArgs, 0, &fX))
		return Py_BuildException();
	float fY;
	if (!PyTuple_GetFloat(poArgs, 1, &fY))
		return Py_BuildException();
	CPythonMiniMap::Instance().ChangeAtlasRenderPos(fX, fY);
	return Py_BuildNone();
}

PyObject* miniMapGetAtlasRenderPos(PyObject* poSelf, PyObject* poArgs)
{
	float fX = 0.0f;
	float fY = 0.0f;
	CPythonMiniMap::Instance().GetAtlasRenderPos(&fX, &fY);
	return Py_BuildValue("(ff)", fX, fY);
}

PyObject* miniMapCenterAtlasRenderPos(PyObject* poSelf, PyObject* poArgs)
{
	CPythonMiniMap::Instance().CenterAtlasRenderPos();
	return Py_BuildNone();
}

PyObject* miniMapGetAtlasPanExtents(PyObject* poSelf, PyObject* poArgs)
{
	float fMinX = 0.0f;
	float fMaxX = 0.0f;
	float fMinY = 0.0f;
	float fMaxY = 0.0f;
	CPythonMiniMap::Instance().GetAtlasPanExtents(&fMinX, &fMaxX, &fMinY, &fMaxY);
	return Py_BuildValue("(ffff)", fMinX, fMaxX, fMinY, fMaxY);
}

PyObject* miniMapGetAtlasMaxPos(PyObject* poSelf, PyObject* poArgs)
{
	float fMaxX = 0.0f;
	float fMaxY = 0.0f;
	CPythonMiniMap::Instance().GetAtlasMaxPos(&fMaxX, &fMaxY);
	return Py_BuildValue("(ff)", fMaxX, fMaxY);
}

PyObject* miniMapConvertGlobalPosToAtlasPos(PyObject* poSelf, PyObject* poArgs)
{
	float fGlobalX;
	if (!PyTuple_GetFloat(poArgs, 0, &fGlobalX))
		return Py_BuildException();
	float fGlobalY;
	if (!PyTuple_GetFloat(poArgs, 1, &fGlobalY))
		return Py_BuildException();
	float fAtlasX = 0.0f;
	float fAtlasY = 0.0f;
	CPythonMiniMap::Instance().ConvertGlobalPosToAtlasPos(fGlobalX, fGlobalY, &fAtlasX, &fAtlasY);
	return Py_BuildValue("(ff)", fAtlasX, fAtlasY);
}

PyObject* miniMapGetAtlasWidthHeightRatio(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("f", CPythonMiniMap::Instance().GetAtlasWidthHeightRatio());
}

PyObject* miniMapSetAtlasNpcHelperView(PyObject* poSelf, PyObject* poArgs)
{
	int iView = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &iView))
		return Py_BuildException();
	CPythonMiniMap::Instance().SetAtlasNpcHelperView(iView != 0);
	return Py_BuildNone();
}

PyObject* miniMapLoadAtlasForMapName(PyObject* poSelf, PyObject* poArgs)
{
	char* pszMapName = NULL;
	if (!PyTuple_GetString(poArgs, 0, &pszMapName))
		return Py_BuildException();
	const bool bLoaded = CPythonMiniMap::Instance().LoadAtlasForMapName(pszMapName);
	return Py_BuildValue("i", bLoaded ? 1 : 0);
}

PyObject* miniMapIsAtlasNpcHelperView(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonMiniMap::Instance().IsAtlasNpcHelperView() ? 1 : 0);
}

PyObject* miniMapSetAtlasHelperViewportSize(PyObject* poSelf, PyObject* poArgs)
{
	float fWidth = 0.0f;
	float fHeight = 0.0f;
	if (!PyTuple_GetFloat(poArgs, 0, &fWidth))
		return Py_BuildException();
	if (!PyTuple_GetFloat(poArgs, 1, &fHeight))
		return Py_BuildException();
	CPythonMiniMap::Instance().SetAtlasHelperViewportSize(fWidth, fHeight);
	return Py_BuildNone();
}

PyObject* miniMapSetAtlasScreenOrigin(PyObject* poSelf, PyObject* poArgs)
{
	float fScreenX = 0.0f;
	float fScreenY = 0.0f;
	if (!PyTuple_GetFloat(poArgs, 0, &fScreenX))
		return Py_BuildException();
	if (!PyTuple_GetFloat(poArgs, 1, &fScreenY))
		return Py_BuildException();
	CPythonMiniMap::Instance().SetAtlasScreenOrigin(fScreenX, fScreenY);
	return Py_BuildNone();
}

PyObject* miniMapGetAtlasHelperViewportSize(PyObject* poSelf, PyObject* poArgs)
{
	float fWidth = 0.0f;
	float fHeight = 0.0f;
	CPythonMiniMap::Instance().GetAtlasHelperViewportSize(&fWidth, &fHeight);
	return Py_BuildValue("(ff)", fWidth, fHeight);
}

PyObject* miniMapSelectNPC(PyObject* poSelf, PyObject* poArgs)
{
	int iMapIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iMapIndex))
		return Py_BuildException();
	int iVnum;
	if (!PyTuple_GetInteger(poArgs, 1, &iVnum))
		return Py_BuildException();
	int iX;
	if (!PyTuple_GetInteger(poArgs, 2, &iX))
		return Py_BuildException();
	int iY;
	if (!PyTuple_GetInteger(poArgs, 3, &iY))
		return Py_BuildException();
	CPythonMiniMap::Instance().SelectNPC(iMapIndex, iVnum, iX, iY);
	return Py_BuildNone();
}

PyObject* miniMapUnselectNPC(PyObject* poSelf, PyObject* poArgs)
{
	CPythonMiniMap::Instance().UnselectNPC();
	return Py_BuildNone();
}

PyObject* miniMapIsNPCSelected(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonMiniMap::Instance().IsNPCSelected() ? 1 : 0);
}

PyObject* miniMapGetSelectedNPCPos(PyObject* poSelf, PyObject* poArgs)
{
	long lX = 0;
	long lY = 0;
	CPythonMiniMap::Instance().GetSelectedNPCPos(&lX, &lY);
	return Py_BuildValue("(ii)", lX, lY);
}

PyObject* miniMapCenterAtlasOnMapPos(PyObject* poSelf, PyObject* poArgs)
{
	float fMapX;
	if (!PyTuple_GetFloat(poArgs, 0, &fMapX))
		return Py_BuildException();
	float fMapY;
	if (!PyTuple_GetFloat(poArgs, 1, &fMapY))
		return Py_BuildException();

	CPythonMiniMap::Instance().CenterAtlasOnMapPos(fMapX, fMapY);
	return Py_BuildNone();
}

PyObject* miniMapIsShowAtlas(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonMiniMap::Instance().IsShowAtlas() ? 1 : 0);
}
#endif

void initMiniMap()
{
	static PyMethodDef s_methods[] =
	{
		{ "SetScale",						miniMapSetScale,								METH_VARARGS },
		{ "ScaleUp",						miniMapScaleUp,									METH_VARARGS },
		{ "ScaleDown",						miniMapScaleDown,								METH_VARARGS },
		{ "SetMiniMapSize",					miniMapSetMiniMapSize,							METH_VARARGS },

		{ "SetCenterPosition",				miniMapSetCenterPosition,						METH_VARARGS },

		{ "Destroy",						miniMapDestroy,									METH_VARARGS },
		{ "Create",							miniMapCreate,									METH_VARARGS },
		{ "Update",							miniMapUpdate,									METH_VARARGS },
		{ "Render",							miniMapRender,									METH_VARARGS },

		{ "Show",							miniMapShow,									METH_VARARGS },
		{ "Hide",							miniMapHide,									METH_VARARGS },

		{ "isShow",							miniMapisShow,									METH_VARARGS },

		{ "GetInfo",						miniMapGetInfo,									METH_VARARGS },

		{ "LoadAtlas",						miniMapLoadAtlas,								METH_VARARGS },
		{ "UpdateAtlas",					miniMapUpdateAtlas,								METH_VARARGS },
		{ "RenderAtlas",					miniMapRenderAtlas,								METH_VARARGS },
		{ "ShowAtlas",						miniMapShowAtlas,								METH_VARARGS },
		{ "HideAtlas",						miniMapHideAtlas,								METH_VARARGS },
		{ "isShowAtlas",					miniMapisShowAtlas,								METH_VARARGS },
		{ "IsAtlas",						miniMapIsAtlas,									METH_VARARGS },
		{ "GetAtlasInfo",					miniMapGetAtlasInfo,							METH_VARARGS },
		{ "GetAtlasSize",					miniMapGetAtlasSize,							METH_VARARGS },

		{ "AddWayPoint",					miniMapAddWayPoint,								METH_VARARGS },
		{ "RemoveWayPoint",					miniMapRemoveWayPoint,							METH_VARARGS },

		{ "RegisterAtlasWindow",			miniMapRegisterAtlasWindow,						METH_VARARGS },
		{ "UnregisterAtlasWindow",			miniMapUnregisterAtlasWindow,					METH_VARARGS },

		{ "GetGuildAreaID",					miniMapGetGuildAreaID,							METH_VARARGS },
#if defined(ENABLE_NPC_LOCATION_HELPER)
		{ "SetAtlasScale", miniMapSetAtlasScale, METH_VARARGS },
		{ "GetAtlasScale", miniMapGetAtlasScale, METH_VARARGS },
		{ "AtlasScaleUp", miniMapAtlasScaleUp, METH_VARARGS },
		{ "AtlasScaleDown", miniMapAtlasScaleDown, METH_VARARGS },
		{ "ChangeAtlasRenderPos", miniMapChangeAtlasRenderPos, METH_VARARGS },
		{ "GetAtlasRenderPos", miniMapGetAtlasRenderPos, METH_VARARGS },
		{ "CenterAtlasRenderPos", miniMapCenterAtlasRenderPos, METH_VARARGS },
		{ "GetAtlasPanExtents", miniMapGetAtlasPanExtents, METH_VARARGS },
		{ "GetAtlasMaxPos", miniMapGetAtlasMaxPos, METH_VARARGS },
		{ "ConvertGlobalPosToAtlasPos", miniMapConvertGlobalPosToAtlasPos, METH_VARARGS },
		{ "GetAtlasWidthHeightRatio", miniMapGetAtlasWidthHeightRatio, METH_VARARGS },
		{ "SetAtlasNpcHelperView", miniMapSetAtlasNpcHelperView, METH_VARARGS },
		{ "LoadAtlasForMapName", miniMapLoadAtlasForMapName, METH_VARARGS },
		{ "IsAtlasNpcHelperView", miniMapIsAtlasNpcHelperView, METH_VARARGS },
		{ "SetAtlasHelperViewportSize", miniMapSetAtlasHelperViewportSize, METH_VARARGS },
		{ "GetAtlasHelperViewportSize", miniMapGetAtlasHelperViewportSize, METH_VARARGS },
		{ "SetAtlasScreenOrigin", miniMapSetAtlasScreenOrigin, METH_VARARGS },
		{ "SelectNPC", miniMapSelectNPC, METH_VARARGS },
		{ "UnselectNPC", miniMapUnselectNPC, METH_VARARGS },
		{ "IsNPCSelected", miniMapIsNPCSelected, METH_VARARGS },
		{ "GetSelectedNPCPos", miniMapGetSelectedNPCPos, METH_VARARGS },
		{ "CenterAtlasOnMapPos", miniMapCenterAtlasOnMapPos, METH_VARARGS },
		{ "IsShowAtlas", miniMapIsShowAtlas, METH_VARARGS },
#endif
		{ NULL, NULL },
	};

	PyObject * poModule = Py_InitModule("miniMap", s_methods);

	PyModule_AddIntConstant(poModule, "TYPE_OPC",			CPythonMiniMap::TYPE_OPC);
	PyModule_AddIntConstant(poModule, "TYPE_OPCPVP",		CPythonMiniMap::TYPE_OPCPVP);
	PyModule_AddIntConstant(poModule, "TYPE_OPCPVPSELF",	CPythonMiniMap::TYPE_OPCPVPSELF);
	PyModule_AddIntConstant(poModule, "TYPE_NPC",			CPythonMiniMap::TYPE_NPC);
	PyModule_AddIntConstant(poModule, "TYPE_MONSTER",		CPythonMiniMap::TYPE_MONSTER);
	PyModule_AddIntConstant(poModule, "TYPE_WARP",			CPythonMiniMap::TYPE_WARP);
	PyModule_AddIntConstant(poModule, "TYPE_WAYPOINT",		CPythonMiniMap::TYPE_WAYPOINT);
	PyModule_AddIntConstant(poModule, "TYPE_PARTY",		CPythonMiniMap::TYPE_PARTY);
	PyModule_AddIntConstant(poModule, "TYPE_EMPIRE",		CPythonMiniMap::TYPE_EMPIRE);
#if defined(ENABLE_NPC_LOCATION_HELPER)
	PyModule_AddIntConstant(poModule, "SQUARED_ATLAS_SIZE", CPythonMiniMap::SQUARED_ATLAS_SIZE);
	PyModule_AddIntConstant(poModule, "ATLAS_TYPE_SMALL", CPythonMiniMap::ATLAS_TYPE_SMALL);
	PyModule_AddIntConstant(poModule, "ATLAS_TYPE_NPC_LOCATION_HELPER", CPythonMiniMap::ATLAS_TYPE_NPC_LOCATION_HELPER);
	PyModule_AddIntConstant(poModule, "ATLAS_TYPE_DONT_SHOW", CPythonMiniMap::ATLAS_TYPE_DONT_SHOW);
#endif
}
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
