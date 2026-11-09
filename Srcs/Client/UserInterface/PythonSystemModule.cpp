#include "StdAfx.h"
#include "PythonSystem.h"

#if defined(__BL_ENVIRONMENT_EFFECT_OPTION__)
PyObject* systemSetNightModeOption(PyObject* poSelf, PyObject* poArgs)
{
	bool bEnable;
	if (!PyTuple_GetBoolean(poArgs, 0, &bEnable))
		return Py_BuildException();

	CPythonSystem::Instance().SetNightModeOption(bEnable);
	return Py_BuildNone();
}

PyObject* systemGetNightModeOption(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetNightModeOption());
}

PyObject* systemSetSnowModeOption(PyObject* poSelf, PyObject* poArgs)
{
	bool bEnable;
	if (!PyTuple_GetBoolean(poArgs, 0, &bEnable))
		return Py_BuildException();

	CPythonSystem::Instance().SetSnowModeOption(bEnable);
	return Py_BuildNone();
}

PyObject* systemGetSnowModeOption(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetSnowModeOption());
}

PyObject* systemSetSnowTextureModeOption(PyObject* poSelf, PyObject* poArgs)
{
	bool bEnable;
	if (!PyTuple_GetBoolean(poArgs, 0, &bEnable))
		return Py_BuildException();

	CPythonSystem::Instance().SetSnowTextureModeOption(bEnable);
	return Py_BuildNone();
}

PyObject* systemGetSnowTextureModeOption(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetSnowTextureModeOption());
}
#endif

PyObject * systemGetWidth(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetWidth());
}

PyObject * systemGetHeight(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetHeight());
}

PyObject * systemSetInterfaceHandler(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* poHandler;
	if (!PyTuple_GetObject(poArgs, 0, &poHandler))
		return Py_BuildException();

	CPythonSystem::Instance().SetInterfaceHandler(poHandler);
	return Py_BuildNone();
}

PyObject * systemDestroyInterfaceHandler(PyObject* poSelf, PyObject* poArgs)
{
	CPythonSystem::Instance().DestroyInterfaceHandler();
	return Py_BuildNone();
}

PyObject * systemReserveResource(PyObject* poSelf, PyObject* poArgs)
{
	std::set<std::string> ResourceSet;
	CResourceManager::Instance().PushBackgroundLoadingSet(ResourceSet);
	return Py_BuildNone();
}

PyObject * systemisInterfaceConfig(PyObject* poSelf, PyObject* poArgs)
{
	int isInterfaceConfig = CPythonSystem::Instance().isInterfaceConfig();
	return Py_BuildValue("i", isInterfaceConfig);
}

PyObject * systemSaveWindowStatus(PyObject* poSelf, PyObject* poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	int iVisible;
	if (!PyTuple_GetInteger(poArgs, 1, &iVisible))
		return Py_BuildException();

	int iMinimized;
	if (!PyTuple_GetInteger(poArgs, 2, &iMinimized))
		return Py_BuildException();

	int ix;
	if (!PyTuple_GetInteger(poArgs, 3, &ix))
		return Py_BuildException();

	int iy;
	if (!PyTuple_GetInteger(poArgs, 4, &iy))
		return Py_BuildException();

	int iHeight;
	if (!PyTuple_GetInteger(poArgs, 5, &iHeight))
		return Py_BuildException();

	CPythonSystem::Instance().SaveWindowStatus(iIndex, iVisible, iMinimized, ix, iy, iHeight);
	return Py_BuildNone();
}

PyObject * systemGetWindowStatus(PyObject* poSelf, PyObject* poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	const CPythonSystem::TWindowStatus & c_rWindowStatus = CPythonSystem::Instance().GetWindowStatusReference(iIndex);
	return Py_BuildValue("iiiii", c_rWindowStatus.isVisible,
								  c_rWindowStatus.isMinimized,
								  c_rWindowStatus.ixPosition,
								  c_rWindowStatus.iyPosition,
								  c_rWindowStatus.iHeight);
}

PyObject * systemGetConfig(PyObject * poSelf, PyObject * poArgs)
{
	CPythonSystem::TConfig *tmp = CPythonSystem::Instance().GetConfig();

	int iRes = CPythonSystem::Instance().GetResolutionIndex(tmp->width, tmp->height, tmp->bpp);
	int iFrequency = CPythonSystem::Instance().GetFrequencyIndex(iRes, tmp->frequency);

	return Py_BuildValue("iiiiiiii",  iRes,
									  iFrequency,
									  tmp->is_software_cursor,
									  tmp->is_object_culling,
									  tmp->music_volume,
									  tmp->voice_volume,
									  tmp->gamma,
									  tmp->iDistance);
}

PyObject * systemSetSaveID(PyObject * poSelf, PyObject * poArgs)
{
	int iValue;
	if (!PyTuple_GetInteger(poArgs, 0, &iValue))
		return Py_BuildException();

	char * szSaveID;
	if (!PyTuple_GetString(poArgs, 1, &szSaveID))
		return Py_BuildException();

	CPythonSystem::Instance().SetSaveID(iValue, szSaveID);
	return Py_BuildNone();
}

PyObject * systemisSaveID(PyObject * poSelf, PyObject * poArgs)
{
	int value = CPythonSystem::Instance().IsSaveID();
	return Py_BuildValue("i", value);
}

PyObject * systemGetSaveID(PyObject * poSelf, PyObject * poArgs)
{
	const char * c_szSaveID = CPythonSystem::Instance().GetSaveID();
	return Py_BuildValue("s", c_szSaveID);
}

PyObject * systemGetMusicVolume(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("f", CPythonSystem::Instance().GetMusicVolume());
}

PyObject * systemGetSoundVolume(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetSoundVolume());
}

PyObject * systemSetMusicVolume(PyObject * poSelf, PyObject * poArgs)
{
	float fVolume;
	if (!PyTuple_GetFloat(poArgs, 0, &fVolume))
		return Py_BuildException();

	CPythonSystem::Instance().SetMusicVolume(fVolume);
	return Py_BuildNone();
}

#ifdef ENABLE_FOV_OPTION
PyObject * systemSetFOVLevel(PyObject * poSelf, PyObject * poArgs)
{
    float fFOV;
    if (!PyTuple_GetFloat(poArgs, 0, &fFOV))
        return Py_BuildException();
    CPythonSystem::Instance().SetFOVLevel(fFOV);
    return Py_BuildNone();
}
PyObject * systemGetFOVLevel(PyObject * poSelf, PyObject * poArgs)
{
    return Py_BuildValue("f", CPythonSystem::Instance().GetFOVLevel());
}
#endif

PyObject * systemSetSoundVolumef(PyObject * poSelf, PyObject * poArgs)
{
	float fVolume;
	if (!PyTuple_GetFloat(poArgs, 0, &fVolume))
		return Py_BuildException();

	CPythonSystem::Instance().SetSoundVolumef(fVolume);
	return Py_BuildNone();
}

PyObject * systemIsSoftwareCursor(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsSoftwareCursor());
}

PyObject * systemSetViewChatFlag(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetViewChatFlag(iFlag);

	return Py_BuildNone();
}

PyObject * systemIsViewChat(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsViewChat());
}

PyObject * systemSetAlwaysShowNameFlag(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetAlwaysShowNameFlag(iFlag);

	return Py_BuildNone();
}

PyObject * systemSetShowDamageFlag(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetShowDamageFlag(iFlag);

	return Py_BuildNone();
}

PyObject * systemSetShowSalesTextFlag(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetShowSalesTextFlag(iFlag);

	return Py_BuildNone();
}

#ifdef WJ_SHOW_MOB_INFO
#include "PythonCharacterManager.h"
#include "PythonNonPlayer.h"
#include "PythonSystem.h"
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
PyObject * systemIsShowMobAIFlag(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsShowMobAIFlag());
}
PyObject * systemSetShowMobAIFlag(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetShowMobAIFlagFlag(iFlag);

	for (CPythonCharacterManager::CharacterIterator it=CPythonCharacterManager::Instance().CharacterInstanceBegin(), ti=CPythonCharacterManager::Instance().CharacterInstanceEnd();
			it!=ti;
			++it)
	{
		CInstanceBase * pkInst = *it;
		if (pkInst && pkInst->IsEnemy())
			pkInst->MobInfoAiFlagRefresh();
	}
	return Py_BuildNone();
}
#endif
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
PyObject * systemIsShowMobLevel(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsShowMobLevel());
}
PyObject * systemSetShowMobLevel(PyObject * poSelf, PyObject * poArgs)
{
	int iFlag;
	if (!PyTuple_GetInteger(poArgs, 0, &iFlag))
		return Py_BuildException();

	CPythonSystem::Instance().SetShowMobLevelFlag(iFlag);

	for (CPythonCharacterManager::CharacterIterator it=CPythonCharacterManager::Instance().CharacterInstanceBegin(), ti=CPythonCharacterManager::Instance().CharacterInstanceEnd();
			it!=ti;
			++it)
	{
		CInstanceBase * pkInst = *it;
		if (pkInst && pkInst->IsEnemy())
			pkInst->MobInfoLevelRefresh();
	}
	return Py_BuildNone();
}
#endif
#endif

PyObject * systemIsAlwaysShowName(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsAlwaysShowName());
}

PyObject * systemIsShowDamage(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsShowDamage());
}

PyObject * systemIsShowSalesText(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().IsShowSalesText());
}

PyObject * systemSetConfig(PyObject * poSelf, PyObject * poArgs)
{
	int res_index;
	int width;
	int height;
	int bpp;
	int frequency_index;
	int frequency;
	int software_cursor;
	int shadow;
	int object_culling;
	int music_volume;
	int voice_volume;
	int gamma;
	int distance;

	if (!PyTuple_GetInteger(poArgs, 0, &res_index))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &frequency_index))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 2, &software_cursor))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 3, &shadow))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 4, &object_culling))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 5, &music_volume))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 6, &voice_volume))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 7, &gamma))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 8, &distance))
		return Py_BuildException();

	if (!CPythonSystem::Instance().GetResolution(res_index, (DWORD *) &width, (DWORD *) &height, (DWORD *) &bpp))
		return Py_BuildNone();

	if (!CPythonSystem::Instance().GetFrequency(res_index,frequency_index, (DWORD *) &frequency))
		return Py_BuildNone();

	CPythonSystem::TConfig tmp;

	memcpy(&tmp, CPythonSystem::Instance().GetConfig(), sizeof(tmp));

	tmp.width				= width;
	tmp.height				= height;
	tmp.bpp					= bpp;
	tmp.frequency			= frequency;
	tmp.is_software_cursor	= software_cursor ? true : false;
	tmp.is_object_culling	= object_culling ? true : false;
	tmp.music_volume		= (char) music_volume;
	tmp.voice_volume		= (char) voice_volume;
	tmp.gamma				= gamma;
	tmp.iDistance			= distance;

	CPythonSystem::Instance().SetConfig(&tmp);
	return Py_BuildNone();
}

PyObject * systemApplyConfig(PyObject * poSelf, PyObject * poArgs)
{
	CPythonSystem::Instance().ApplyConfig();
	return Py_BuildNone();
}

PyObject * systemSaveConfig(PyObject * poSelf, PyObject * poArgs)
{
	int ret = CPythonSystem::Instance().SaveConfig();
	return Py_BuildValue("i", ret);
}

PyObject * systemGetResolutionCount(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetResolutionCount());
}

PyObject * systemGetFrequencyCount(PyObject * poSelf, PyObject * poArgs)
{
	int	index;

	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonSystem::Instance().GetFrequencyCount(index));
}

PyObject * systemGetResolution(PyObject * poSelf, PyObject * poArgs)
{
	int	index;
	DWORD width = 0, height = 0, bpp = 0;

	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	CPythonSystem::Instance().GetResolution(index, &width, &height, &bpp);
	return Py_BuildValue("iii", width, height, bpp);
}

PyObject * systemGetCurrentResolution(PyObject * poSelf, PyObject *poArgs)
{
	CPythonSystem::TConfig *tmp = CPythonSystem::Instance().GetConfig();
	return Py_BuildValue("iii", tmp->width, tmp->height, tmp->bpp);
}

PyObject * systemGetFrequency(PyObject * poSelf, PyObject * poArgs)
{
	int	index, frequency_index;
	DWORD frequency = 0;

	if (!PyTuple_GetInteger(poArgs, 0, &index))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &frequency_index))
		return Py_BuildException();

	CPythonSystem::Instance().GetFrequency(index, frequency_index, &frequency);
	return Py_BuildValue("i", frequency);
}

PyObject* systemGetShadowLevel(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetShadowLevel());
}

PyObject* systemSetShadowLevel(PyObject* poSelf, PyObject* poArgs)
{
	int level;

	if (!PyTuple_GetInteger(poArgs, 0, &level))
		return Py_BuildException();

	if (level > 0)
		CPythonSystem::Instance().SetShadowLevel(level);

	return Py_BuildNone();
}

PyObject* systemGetGraphicsLevel(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetGraphicsLevel());
}

PyObject* systemSetGraphicsLevel(PyObject* poSelf, PyObject* poArgs)
{
	int level;

	if (!PyTuple_GetInteger(poArgs, 0, &level))
		return Py_BuildException();

	CPythonSystem::Instance().SetGraphicsLevel(level);

	return Py_BuildNone();
}

PyObject* systemGetRainLevel(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetRainLevel());
}

PyObject* systemSetRainLevel(PyObject* poSelf, PyObject* poArgs)
{
	int iLevel;
	if (!PyTuple_GetInteger(poArgs, 0, &iLevel))
		return Py_BuildException();

	CPythonSystem::Instance().SetRainLevel(iLevel);
	return Py_BuildNone();
}

PyObject* systemGetShadowQualityLevel(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetShadowQualityLevel());
}

PyObject* systemSetShadowQualityLevel(PyObject* poSelf, PyObject* poArgs)
{
	int iLevel;
	if (!PyTuple_GetInteger(poArgs, 0, &iLevel))
		return Py_BuildException();

	CPythonSystem::Instance().SetShadowQualityLevel(iLevel);
	return Py_BuildNone();
}

PyObject* systemGetTerrainShadow(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetTerrainShadow());
}

PyObject* systemSetTerrainShadow(PyObject* poSelf, PyObject* poArgs)
{
	int iLevel;
	if (!PyTuple_GetInteger(poArgs, 0, &iLevel))
		return Py_BuildException();

	CPythonSystem::Instance().SetTerrainShadow(iLevel);
	return Py_BuildNone();
}

PyObject* systemGetTreeShadow(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetTreeShadow());
}

PyObject* systemSetTreeShadow(PyObject* poSelf, PyObject* poArgs)
{
	int iLevel;
	if (!PyTuple_GetInteger(poArgs, 0, &iLevel))
		return Py_BuildException();

	CPythonSystem::Instance().SetTreeShadow(iLevel);
	return Py_BuildNone();
}

#ifdef ENABLE_OFFLINESHOP_SYSTEM
PyObject* systemSetShopNamesRange(PyObject* poSelf, PyObject* poArgs)
{
	float fRange;
	if (!PyTuple_GetFloat(poArgs, 0, &fRange))
		return Py_BuildException();

	CPythonSystem::Instance().SetShopNamesRange(fRange);
	return Py_BuildNone();
}

PyObject* systemGetShopNamesRange(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("f", CPythonSystem::Instance().GetShopNamesRange());
}
#endif

#if defined(__BL_PICK_FILTER__)
PyObject* systemSetPickUpFilter(PyObject* poSelf, PyObject* poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	const bool bFilter = CPythonSystem::Instance().TPickUpFilter.GetFilter(iIndex);
	CPythonSystem::Instance().TPickUpFilter.SetFilter(iIndex, !bFilter);

	// ADD THIS LINE TO SAVE INSTANTLY
	CPythonSystem::Instance().TPickUpFilter.Save(); 

	return Py_BuildNone();
}

PyObject* systemGetPickUpFilter(PyObject* poSelf, PyObject* poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	const bool bFilter = CPythonSystem::Instance().TPickUpFilter.GetFilter(iIndex);

	return Py_BuildValue("i", bFilter);
}

PyObject* systemSetPickUpFilterSize(PyObject* poSelf, PyObject* poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	const bool bSize = CPythonSystem::Instance().TPickUpFilter.GetSize(iIndex);
	CPythonSystem::Instance().TPickUpFilter.SetSize(iIndex, !bSize);

	// ADD THIS LINE TO SAVE INSTANTLY
	CPythonSystem::Instance().TPickUpFilter.Save();

	return Py_BuildNone();
}

PyObject* systemGetPickUpFilterSize(PyObject* poSelf, PyObject* poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	const bool bSize = CPythonSystem::Instance().TPickUpFilter.GetSize(iIndex);

	return Py_BuildValue("i", bSize);
}

PyObject* systemSetPickUpFilterRefine(PyObject* poSelf, PyObject* poArgs)
{
	BYTE bMin;
	if (!PyTuple_GetByte(poArgs, 0, &bMin))
		return Py_BuildException();

	BYTE bMax;
	if (!PyTuple_GetByte(poArgs, 1, &bMax))
		return Py_BuildException();

	CPythonSystem::Instance().TPickUpFilter.SetRefine(bMin, bMax);


	CPythonSystem::Instance().TPickUpFilter.Save();

	return Py_BuildNone();
}

PyObject* systemGetPickUpFilterRefine(PyObject* poSelf, PyObject* poArgs)
{
	const std::pair<BYTE, BYTE> pRefine = CPythonSystem::Instance().TPickUpFilter.GetRefine();

	return Py_BuildValue("ii", pRefine.first, pRefine.second);
}

PyObject* systemSetPickUpFilterLevel(PyObject* poSelf, PyObject* poArgs)
{
	long lMin;
	if (!PyTuple_GetLong(poArgs, 0, &lMin))
		return Py_BuildException();

	long lMax;
	if (!PyTuple_GetLong(poArgs, 1, &lMax))
		return Py_BuildException();

	CPythonSystem::Instance().TPickUpFilter.SetLevel(lMin, lMax);


	CPythonSystem::Instance().TPickUpFilter.Save();

	return Py_BuildNone();
}

PyObject* systemGetPickUpFilterLevel(PyObject* poSelf, PyObject* poArgs)
{
	const std::pair<long, long> pLevel = CPythonSystem::Instance().TPickUpFilter.GetLevel();

	return Py_BuildValue("ii", pLevel.first, pLevel.second);
}

PyObject* systemSetPickUpFilterModeAll(PyObject* poSelf, PyObject* poArgs)
{
	bool bAll;
	if (!PyTuple_GetBoolean(poArgs, 0, &bAll))
		return Py_BuildException();

	CPythonSystem::Instance().TPickUpFilter.SetModeAll(bAll);


	CPythonSystem::Instance().TPickUpFilter.Save();

	return Py_BuildNone();
}

PyObject* systemGetPickUpFilterModeAll(PyObject* poSelf, PyObject* poArgs)
{
	const bool bAll = CPythonSystem::Instance().TPickUpFilter.IsModeAll();

	return Py_BuildValue("i", bAll);
}
#endif

#ifdef ENABLE_IKASHOP_RENEWAL
#ifdef EXTEND_IKASHOP_PRO
PyObject* systemGetShopRangeView(PyObject* poSelf, PyObject* poArgs)
{
	return PyFloat_FromDouble(CPythonSystem::instance().GetShopRangeView());
}

PyObject* systemSetShopRangeView(PyObject* poSelf, PyObject* poArgs)
{
	float view = 0.0f;
	if(!PyTuple_GetFloat(poArgs, 0, &view))
		return Py_BadArgument();

	CPythonSystem::instance().SetShopRangeView(view);
	return Py_BuildNone();
}
#endif
#endif

#if defined(__BL_ENVIRONMENT_EFFECT_OPTION__)
PyObject* systemSetRainModeOption(PyObject* poSelf, PyObject* poArgs)
{
	bool bEnable;
	if (!PyTuple_GetBoolean(poArgs, 0, &bEnable))
		return Py_BuildException();

	CPythonSystem::Instance().SetRainModeOption(bEnable);
	return Py_BuildNone();
}

PyObject* systemGetRainModeOption(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonSystem::Instance().GetRainModeOption());
}
#endif

PyObject* systemClearPickUpExceptions(PyObject* poSelf, PyObject* poArgs)
{
	CPythonSystem::Instance().TPickUpFilter.ClearExceptions();
	return Py_BuildNone();
}

PyObject* systemAddPickUpWhitelist(PyObject* poSelf, PyObject* poArgs)
{
	char* szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	CPythonSystem::Instance().TPickUpFilter.AddWhitelist(szName);
	return Py_BuildNone();
}

PyObject* systemAddPickUpBlacklist(PyObject* poSelf, PyObject* poArgs)
{
	char* szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

	CPythonSystem::Instance().TPickUpFilter.AddBlacklist(szName);
	return Py_BuildNone();
}

void initsystemSetting()
{
	static PyMethodDef s_methods[] =
	{
		{ "GetWidth",					systemGetWidth,					METH_VARARGS },
		{ "GetHeight",					systemGetHeight,				METH_VARARGS },

		{ "SetInterfaceHandler",		systemSetInterfaceHandler,		METH_VARARGS },
		{ "DestroyInterfaceHandler",	systemDestroyInterfaceHandler,	METH_VARARGS },
		{ "ReserveResource",			systemReserveResource,			METH_VARARGS },
#if defined(__BL_ENVIRONMENT_EFFECT_OPTION__)
		{ "SetNightModeOption",			systemSetNightModeOption,		METH_VARARGS },
		{ "GetNightModeOption",			systemGetNightModeOption,		METH_VARARGS },
		{ "SetSnowModeOption",			systemSetSnowModeOption,		METH_VARARGS },
		{ "GetSnowModeOption",			systemGetSnowModeOption,		METH_VARARGS },
		{ "SetSnowTextureModeOption",	systemSetSnowTextureModeOption,	METH_VARARGS },
		{ "GetSnowTextureModeOption",	systemGetSnowTextureModeOption,	METH_VARARGS },
		{ "SetRainModeOption",			systemSetRainModeOption,		METH_VARARGS },
		{ "GetRainModeOption",			systemGetRainModeOption,		METH_VARARGS },
#endif
		{ "isInterfaceConfig",			systemisInterfaceConfig,		METH_VARARGS },
		{ "SaveWindowStatus",			systemSaveWindowStatus,			METH_VARARGS },
		{ "GetWindowStatus",			systemGetWindowStatus,			METH_VARARGS },

		{ "GetResolutionCount",			systemGetResolutionCount,		METH_VARARGS },
		{ "GetFrequencyCount",			systemGetFrequencyCount,		METH_VARARGS },

		{ "GetCurrentResolution",		systemGetCurrentResolution,		METH_VARARGS },

		{ "GetResolution",				systemGetResolution,			METH_VARARGS },
		{ "GetFrequency",				systemGetFrequency,				METH_VARARGS },

		{ "ApplyConfig",				systemApplyConfig,				METH_VARARGS },
		{ "SetConfig",					systemSetConfig,				METH_VARARGS },
		{ "SaveConfig",					systemSaveConfig,				METH_VARARGS },
		{ "GetConfig",					systemGetConfig,				METH_VARARGS },

		{ "SetSaveID",					systemSetSaveID,				METH_VARARGS },
		{ "isSaveID",					systemisSaveID,					METH_VARARGS },
		{ "GetSaveID",					systemGetSaveID,				METH_VARARGS },

		{ "GetMusicVolume",				systemGetMusicVolume,			METH_VARARGS },
		{ "GetSoundVolume",				systemGetSoundVolume,			METH_VARARGS },

		{ "SetMusicVolume",				systemSetMusicVolume,			METH_VARARGS },
		{ "SetSoundVolumef",			systemSetSoundVolumef,			METH_VARARGS },
		{ "IsSoftwareCursor",			systemIsSoftwareCursor,			METH_VARARGS },

		{ "SetViewChatFlag",			systemSetViewChatFlag,			METH_VARARGS },
		{ "IsViewChat",					systemIsViewChat,				METH_VARARGS },

		{ "SetAlwaysShowNameFlag",		systemSetAlwaysShowNameFlag,	METH_VARARGS },
		{ "IsAlwaysShowName",			systemIsAlwaysShowName,			METH_VARARGS },
#if defined(__BL_PICK_FILTER__)
		{ "SetPickUpFilter",			systemSetPickUpFilter,			METH_VARARGS },
		{ "GetPickUpFilter",			systemGetPickUpFilter,			METH_VARARGS },

		{ "SetPickUpFilterSize",		systemSetPickUpFilterSize,		METH_VARARGS },
		{ "GetPickUpFilterSize",		systemGetPickUpFilterSize,		METH_VARARGS },

		{ "SetPickUpFilterRefine",		systemSetPickUpFilterRefine,	METH_VARARGS },
		{ "GetPickUpFilterRefine",		systemGetPickUpFilterRefine,	METH_VARARGS },

		{ "SetPickUpFilterLevel",		systemSetPickUpFilterLevel,		METH_VARARGS },
		{ "GetPickUpFilterLevel",		systemGetPickUpFilterLevel,		METH_VARARGS },

		{ "SetPickUpFilterModeAll",		systemSetPickUpFilterModeAll,	METH_VARARGS },
		{ "GetPickUpFilterModeAll",		systemGetPickUpFilterModeAll,	METH_VARARGS },
#endif

		{ "SetShowDamageFlag",			systemSetShowDamageFlag,		METH_VARARGS },
		{ "IsShowDamage",				systemIsShowDamage,				METH_VARARGS },

		{ "SetShowSalesTextFlag",		systemSetShowSalesTextFlag,		METH_VARARGS },
		{ "IsShowSalesText",			systemIsShowSalesText,			METH_VARARGS },

		{ "GetShadowLevel",				systemGetShadowLevel,			METH_VARARGS },
		{ "SetShadowLevel",				systemSetShadowLevel,			METH_VARARGS },

#ifdef ENABLE_IKASHOP_RENEWAL
#ifdef EXTEND_IKASHOP_PRO
		{"GetShopRangeView",			systemGetShopRangeView,			METH_VARARGS },
		{"SetShopRangeView",			systemSetShopRangeView,			METH_VARARGS },
#endif
#endif

#ifdef ENABLE_FOV_OPTION
        { "GetFOVLevel",                systemGetFOVLevel,                METH_VARARGS },
        { "SetFOVLevel",                systemSetFOVLevel,                METH_VARARGS },
#endif

		{ "GetGraphicsLevel",			systemGetGraphicsLevel,			METH_VARARGS },
		{ "SetGraphicsLevel",			systemSetGraphicsLevel,			METH_VARARGS },
		
		{ "GetShadowQualityLevel",		systemGetShadowQualityLevel,	METH_VARARGS },
		{ "SetShadowQualityLevel",		systemSetShadowQualityLevel,	METH_VARARGS },
		
		{ "GetTerrainShadow",			systemGetTerrainShadow,			METH_VARARGS },
		{ "SetTerrainShadow",			systemSetTerrainShadow,			METH_VARARGS },
		
		{ "GetTreeShadow",				systemGetTreeShadow,			METH_VARARGS },
		{ "SetTreeShadow",				systemSetTreeShadow,			METH_VARARGS },
		
		{ "GetRainLevel",				systemGetRainLevel,				METH_VARARGS },
		{ "SetRainLevel",				systemSetRainLevel,				METH_VARARGS },

#ifdef WJ_SHOW_MOB_INFO
		{ "IsShowMobAIFlag",			systemIsShowMobAIFlag,			METH_VARARGS },
		{ "SetShowMobAIFlag",			systemSetShowMobAIFlag,			METH_VARARGS },

		{ "IsShowMobLevel",				systemIsShowMobLevel,			METH_VARARGS },
		{ "SetShowMobLevel",			systemSetShowMobLevel,			METH_VARARGS },
#endif

#ifdef ENABLE_OFFLINESHOP_SYSTEM
		{ "SetShopNamesRange",			systemSetShopNamesRange, METH_VARARGS },
		{ "GetShopNamesRange",			systemGetShopNamesRange, METH_VARARGS },
#endif

		{ "ClearPickUpExceptions",		systemClearPickUpExceptions,	METH_VARARGS },
		{ "AddPickUpWhitelist",			systemAddPickUpWhitelist,		METH_VARARGS },
		{ "AddPickUpBlacklist",			systemAddPickUpBlacklist,		METH_VARARGS },

		{ NULL,							NULL,							NULL }
	};

	PyObject * poModule = Py_InitModule("systemSetting", s_methods);

	PyModule_AddIntConstant(poModule, "WINDOW_STATUS",		CPythonSystem::WINDOW_STATUS);
	PyModule_AddIntConstant(poModule, "WINDOW_INVENTORY",	CPythonSystem::WINDOW_INVENTORY);
	PyModule_AddIntConstant(poModule, "WINDOW_ABILITY",		CPythonSystem::WINDOW_ABILITY);
	PyModule_AddIntConstant(poModule, "WINDOW_SOCIETY",		CPythonSystem::WINDOW_SOCIETY);
	PyModule_AddIntConstant(poModule, "WINDOW_JOURNAL",		CPythonSystem::WINDOW_JOURNAL);
	PyModule_AddIntConstant(poModule, "WINDOW_COMMAND",		CPythonSystem::WINDOW_COMMAND);

	PyModule_AddIntConstant(poModule, "WINDOW_QUICK",		CPythonSystem::WINDOW_QUICK);
	PyModule_AddIntConstant(poModule, "WINDOW_GAUGE",		CPythonSystem::WINDOW_GAUGE);
	PyModule_AddIntConstant(poModule, "WINDOW_MINIMAP",		CPythonSystem::WINDOW_MINIMAP);
	PyModule_AddIntConstant(poModule, "WINDOW_CHAT",		CPythonSystem::WINDOW_CHAT);
#if defined(__BL_PICK_FILTER__)
	PyModule_AddIntConstant(poModule, "PICKUP_FILTER_COUNT", CPythonSystem::CPickUpFilter::EPICKFILTER::EPICKFILTER_MAX);
	PyModule_AddIntConstant(poModule, "PICKUP_FILTER_SIZE_COUNT", CPythonSystem::CPickUpFilter::ESIZE::ESIZE_MAX);
#endif
}
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
