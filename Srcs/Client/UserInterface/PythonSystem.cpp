#include "StdAfx.h"
#include "PythonSystem.h"
#include "PythonApplication.h"
#include <algorithm>
#include <string>
#define DEFAULT_VALUE_ALWAYS_SHOW_NAME		true

#if defined(__BL_FOG_FIX__)
void CPythonSystem::SetFogMode(bool bEnable)
{
	m_Config.bIsFogModeEnabled = bEnable;
}

bool CPythonSystem::GetFogMode() const
{
	return m_Config.bIsFogModeEnabled;
}
#endif

void CPythonSystem::SetInterfaceHandler(PyObject * poHandler)
{
//	if (m_poInterfaceHandler)
//		Py_DECREF(m_poInterfaceHandler);

	m_poInterfaceHandler = poHandler;

//	if (m_poInterfaceHandler)
//		Py_INCREF(m_poInterfaceHandler);
}

void CPythonSystem::DestroyInterfaceHandler()
{
	m_poInterfaceHandler = NULL;
}

void CPythonSystem::SaveWindowStatus(int iIndex, int iVisible, int iMinimized, int ix, int iy, int iHeight)
{
	m_WindowStatus[iIndex].isVisible	= iVisible;
	m_WindowStatus[iIndex].isMinimized	= iMinimized;
	m_WindowStatus[iIndex].ixPosition	= ix;
	m_WindowStatus[iIndex].iyPosition	= iy;
	m_WindowStatus[iIndex].iHeight		= iHeight;
}

void CPythonSystem::GetDisplaySettings()
{
	memset(m_ResolutionList, 0, sizeof(TResolution) * RESOLUTION_MAX_NUM);
	m_ResolutionCount = 0;

	LPDIRECT3D8 lpD3D = CPythonGraphic::Instance().GetD3D();

	D3DADAPTER_IDENTIFIER8 d3dAdapterIdentifier;
	D3DDISPLAYMODE d3ddmDesktop;

#ifdef DIRECTX9
	lpD3D->GetAdapterIdentifier(0, D3DENUM_WHQL_LEVEL, &d3dAdapterIdentifier);
#else
	lpD3D->GetAdapterIdentifier(0, D3DENUM_NO_WHQL_LEVEL, &d3dAdapterIdentifier);
#endif
	lpD3D->GetAdapterDisplayMode(0, &d3ddmDesktop);

#ifdef DIRECTX9
	DWORD dwNumAdapterModes = lpD3D->GetAdapterModeCount(0, d3ddmDesktop.Format);
#else
	DWORD dwNumAdapterModes = lpD3D->GetAdapterModeCount(0);
#endif

	for (UINT iMode = 0; iMode < dwNumAdapterModes; iMode++)
	{
		D3DDISPLAYMODE DisplayMode;
#ifdef DIRECTX9
		lpD3D->EnumAdapterModes(0, d3ddmDesktop.Format, iMode, &DisplayMode);
#else
		lpD3D->EnumAdapterModes(0, iMode, &DisplayMode);
#endif
		DWORD bpp = 0;

		if (DisplayMode.Width < 800 || DisplayMode.Height < 600)
			continue;

		if (DisplayMode.Format == D3DFMT_R5G6B5)
			bpp = 16;
		else if (DisplayMode.Format == D3DFMT_X8R8G8B8)
			bpp = 32;
		else
			continue;

		int check_res = false;

		for (int i = 0; !check_res && i < m_ResolutionCount; ++i)
		{
			if (m_ResolutionList[i].bpp != bpp ||
				m_ResolutionList[i].width != DisplayMode.Width ||
				m_ResolutionList[i].height != DisplayMode.Height)
				continue;

			int check_fre = false;

			for (int j = 0; j < m_ResolutionList[i].frequency_count; ++j)
			{
				if (m_ResolutionList[i].frequency[j] == DisplayMode.RefreshRate)
				{
					check_fre = true;
					break;
				}
			}

			if (!check_fre)
				if (m_ResolutionList[i].frequency_count < FREQUENCY_MAX_NUM)
					m_ResolutionList[i].frequency[m_ResolutionList[i].frequency_count++] = DisplayMode.RefreshRate;

			check_res = true;
		}

		if (!check_res)
		{
			if (m_ResolutionCount < RESOLUTION_MAX_NUM)
			{
				m_ResolutionList[m_ResolutionCount].width			= DisplayMode.Width;
				m_ResolutionList[m_ResolutionCount].height			= DisplayMode.Height;
				m_ResolutionList[m_ResolutionCount].bpp				= bpp;
				m_ResolutionList[m_ResolutionCount].frequency[0]	= DisplayMode.RefreshRate;
				m_ResolutionList[m_ResolutionCount].frequency_count	= 1;

				++m_ResolutionCount;
			}
		}
	}
}

int	CPythonSystem::GetResolutionCount()
{
	return m_ResolutionCount;
}

int CPythonSystem::GetFrequencyCount(int index)
{
	if (index >= m_ResolutionCount)
		return 0;

    return m_ResolutionList[index].frequency_count;
}

bool CPythonSystem::GetResolution(int index, OUT DWORD *width, OUT DWORD *height, OUT DWORD *bpp)
{
	if (index >= m_ResolutionCount)
		return false;

	*width = m_ResolutionList[index].width;
	*height = m_ResolutionList[index].height;
	*bpp = m_ResolutionList[index].bpp;
	return true;
}

bool CPythonSystem::GetFrequency(int index, int freq_index, OUT DWORD *frequncy)
{
	if (index >= m_ResolutionCount)
		return false;

	if (freq_index >= m_ResolutionList[index].frequency_count)
		return false;

	*frequncy = m_ResolutionList[index].frequency[freq_index];
	return true;
}

int	CPythonSystem::GetResolutionIndex(DWORD width, DWORD height, DWORD bit)
{
	DWORD re_width, re_height, re_bit;
	int i = 0;

	while (GetResolution(i, &re_width, &re_height, &re_bit))
	{
		if (re_width == width)
			if (re_height == height)
				if (re_bit == bit)
					return i;
		i++;
	}

	return 0;
}

int	CPythonSystem::GetFrequencyIndex(int res_index, DWORD frequency)
{
	DWORD re_frequency;
	int i = 0;

	while (GetFrequency(res_index, i, &re_frequency))
	{
		if (re_frequency == frequency)
			return i;

		i++;
	}

	return 0;
}

#ifdef __ANDROID__
extern int g_AndroidScreenWidth;
extern int g_AndroidScreenHeight;
#endif

DWORD CPythonSystem::GetWidth()
{
#ifdef __ANDROID__
	if (g_AndroidScreenWidth > 0)
		m_Config.width = (DWORD)g_AndroidScreenWidth;
#endif
	return m_Config.width;
}

DWORD CPythonSystem::GetHeight()
{
#ifdef __ANDROID__
	if (g_AndroidScreenHeight > 0)
		m_Config.height = (DWORD)g_AndroidScreenHeight;
#endif
	return m_Config.height;
}
DWORD CPythonSystem::GetBPP()
{
	return m_Config.bpp;
}
DWORD CPythonSystem::GetFrequency()
{
	return m_Config.frequency;
}

bool CPythonSystem::IsNoSoundCard()
{
	return m_Config.bNoSoundCard;
}

bool CPythonSystem::IsSoftwareCursor()
{
	return m_Config.is_software_cursor;
}

float CPythonSystem::GetMusicVolume()
{
	return m_Config.music_volume;
}

int CPythonSystem::GetSoundVolume()
{
	return m_Config.voice_volume;
}

void CPythonSystem::SetMusicVolume(float fVolume)
{
	m_Config.music_volume = fVolume;
}

void CPythonSystem::SetSoundVolumef(float fVolume)
{
	m_Config.voice_volume = int(5 * fVolume);
}

#ifdef ENABLE_FOV_OPTION
void CPythonSystem::SetFOVLevel(float fFOV)
{
	m_Config.iFOVLevel = fMINMAX(30.0f, fFOV, 120.0f);
}

float CPythonSystem::GetFOVLevel()
{
	return m_Config.iFOVLevel;
}
#endif

int CPythonSystem::GetDistance()
{
	return m_Config.iDistance;
}

int CPythonSystem::GetShadowLevel()
{
	return m_Config.iShadowLevel;
}

void CPythonSystem::SetShadowLevel(unsigned int level)
{
	m_Config.iShadowLevel = MIN(level, 5);
	CPythonBackground::instance().RefreshShadowLevel();
}

int CPythonSystem::IsSaveID()
{
	return m_Config.isSaveID;
}

const char * CPythonSystem::GetSaveID()
{
	return m_Config.SaveID;
}

bool CPythonSystem::isViewCulling()
{
	return m_Config.is_object_culling;
}

void CPythonSystem::SetSaveID(int iValue, const char * c_szSaveID)
{
	if (iValue != 1)
		return;

	m_Config.isSaveID = iValue;
	strncpy(m_Config.SaveID, c_szSaveID, sizeof(m_Config.SaveID) - 1);
}

CPythonSystem::TConfig * CPythonSystem::GetConfig()
{
	return &m_Config;
}

void CPythonSystem::SetConfig(TConfig * pNewConfig)
{
	m_Config = *pNewConfig;
}

void CPythonSystem::SetDefaultConfig()
{
	memset(&m_Config, 0, sizeof(m_Config));

	m_Config.width				= 1024;
	m_Config.height				= 768;
#ifdef __ANDROID__
	if (g_AndroidScreenWidth > 0 && g_AndroidScreenHeight > 0)
	{
		m_Config.width  = (DWORD)g_AndroidScreenWidth;
		m_Config.height = (DWORD)g_AndroidScreenHeight;
	}
#endif
	m_Config.bpp				= 32;

#if defined( LOCALE_SERVICE_WE_JAPAN )
	m_Config.bWindowed			= true;
#else
	m_Config.bWindowed			= false;
#endif

	m_Config.is_software_cursor	= false;
	m_Config.is_object_culling	= true;
	m_Config.iDistance			= 3;

	m_Config.gamma				= 3;
#ifdef ENABLE_OFFLINESHOP_SYSTEM
	m_Config.shopnames_range = 10.0f;
#endif
	m_Config.music_volume		= 1.0f;
	m_Config.voice_volume		= 5;

	m_Config.bDecompressDDS		= 0;
	m_Config.bSoftwareTiling	= 0;
	m_Config.iShadowLevel		= 3;
#ifdef ENABLE_FOV_OPTION
    m_Config.iFOVLevel            = 30.0f;
#endif
	m_Config.bViewChat			= true;
	m_Config.bAlwaysShowName	= DEFAULT_VALUE_ALWAYS_SHOW_NAME;
	m_Config.bShowDamage		= true;
	m_Config.bShowSalesText		= true;
#ifdef ENABLE_IKASHOP_RENEWAL
#ifdef EXTEND_IKASHOP_PRO
	m_Config.shopRangeView = 5000.0f;
#endif
#endif
#if defined(__BL_FOG_FIX__)
	m_Config.bIsFogModeEnabled	= true;
#endif
#if defined(__BL_ENVIRONMENT_EFFECT_OPTION__)
	m_Config.bNightMode			= true;
	m_Config.bSnowMode			= true;
	m_Config.bSnowTextureMode	= true;
	m_Config.bRainMode			= false;
#endif
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
	m_Config.bShowMobAIFlag		= true;
#endif
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
	m_Config.bShowMobLevel		= true;
#endif
}

int CPythonSystem::GetGraphicsLevel()
{
	return m_Config.iGraphicsLevel;
}

void CPythonSystem::SetGraphicsLevel(unsigned int level)
{
	m_Config.iGraphicsLevel = MIN(level, 4);
	CPythonBackground::instance().RefreshGraphicsLevel();
}

int CPythonSystem::GetRainLevel()
{
	return MIN(MAX(m_Config.iRainLevel,
		CPythonBackground::RAIN_0), /* min */
		CPythonBackground::RAIN_3 /* max */
	);
}

void CPythonSystem::SetRainLevel(unsigned int level)
{
	m_Config.iRainLevel = MIN(MAX(level,
		CPythonBackground::RAIN_0), /* min */
		CPythonBackground::RAIN_3 /* max */
	);
}

//specular
int CPythonSystem::GetSpecularLevel()
{
	return MIN(MAX(m_Config.iSpecularLevel,
		CPythonBackground::SPECULAR_on), /* min */
		CPythonBackground::SPECULAR_off /* max */
	);
}

void CPythonSystem::SetSpecularLevel(unsigned int level)
{
	m_Config.iSpecularLevel = MIN(MAX(level,
		CPythonBackground::SPECULAR_on), /* min */
		CPythonBackground::SPECULAR_off /* max */
	);
}

//grass
int CPythonSystem::GetGrassType()
{
	return MIN(MAX(m_Config.iGrassType,
		CPythonBackground::GRASS_0), /* min */
		CPythonBackground::GRASS_3 /* max */
	);
}

void CPythonSystem::SetGrassType(unsigned int level)
{
	m_Config.iGrassType = MIN(MAX(level,
		CPythonBackground::GRASS_0), /* min */
		CPythonBackground::GRASS_3 /* max */
	);
}

//blood
int CPythonSystem::GetBloodLevel()
{
	return MIN(MAX(m_Config.iBloodLevel,
		CPythonBackground::BLOOD_on), /* min */
		CPythonBackground::BLOOD_off /* max */
	);
}

void CPythonSystem::SetBloodLevel(unsigned int level)
{
	m_Config.iBloodLevel = MIN(MAX(level,
		CPythonBackground::BLOOD_on), /* min */
		CPythonBackground::BLOOD_off /* max */
	);
}

int CPythonSystem::GetShadowQualityLevel()
{
	return MIN(MAX(m_Config.iShadowQualityLevel,
		CPythonBackground::SHADOW_QUALITY_BAD), /* min */
		CPythonBackground::SHADOW_QUALITY_ULTRA /* max */
	);
}

void CPythonSystem::SetShadowQualityLevel(unsigned int level)
{
	m_Config.iShadowQualityLevel = MIN(MAX(level,
		CPythonBackground::SHADOW_QUALITY_BAD), /* min */
		CPythonBackground::SHADOW_QUALITY_ULTRA /* max */
	);
}

int CPythonSystem::GetTerrainShadow()
{
	return MIN(MAX(m_Config.iTerrainShadow,
		CPythonBackground::TERRAIN_SHADOW_off), /* min */
		CPythonBackground::TERRAIN_SHADOW_on /* max */
	);
}

void CPythonSystem::SetTerrainShadow(unsigned int level)
{
	m_Config.iTerrainShadow = MIN(MAX(level,
		CPythonBackground::TERRAIN_SHADOW_off), /* min */
		CPythonBackground::TERRAIN_SHADOW_on /* max */
	);
}

int CPythonSystem::GetTreeShadow()
{
	return MIN(MAX(m_Config.iTreeShadow,
		CPythonBackground::TREE_SHADOW_on), /* min */
		CPythonBackground::TREE_SHADOW_all /* max */
	);
}

void CPythonSystem::SetTreeShadow(unsigned int level)
{
	m_Config.iTreeShadow = MIN(MAX(level,
		CPythonBackground::TREE_SHADOW_on), /* min */
		CPythonBackground::TREE_SHADOW_all /* max */
	);
}

void CPythonSystem::SetSharpnessPower(float fsharpness)
{
	m_Config.fSharpnessPower = fMINMAX(0.0f, fsharpness, 5.0f);
}

float CPythonSystem::GetSharpnessPower()
{
	Frustum::Instance().fSharpnessPower = m_Config.fSharpnessPower;
	return m_Config.fSharpnessPower;
}

bool CPythonSystem::IsWindowed()
{
	return m_Config.bWindowed;
}

bool CPythonSystem::IsViewChat()
{
	return m_Config.bViewChat;
}

void CPythonSystem::SetViewChatFlag(int iFlag)
{
	m_Config.bViewChat = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsAlwaysShowName()
{
	return m_Config.bAlwaysShowName;
}

void CPythonSystem::SetAlwaysShowNameFlag(int iFlag)
{
	m_Config.bAlwaysShowName = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsShowDamage()
{
	return m_Config.bShowDamage;
}

void CPythonSystem::SetShowDamageFlag(int iFlag)
{
	m_Config.bShowDamage = iFlag == 1 ? true : false;
}

bool CPythonSystem::IsShowSalesText()
{
	return m_Config.bShowSalesText;
}

void CPythonSystem::SetShowSalesTextFlag(int iFlag)
{
	m_Config.bShowSalesText = iFlag == 1 ? true : false;
}

#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
bool CPythonSystem::IsShowMobAIFlag()
{
	return m_Config.bShowMobAIFlag;
}

void CPythonSystem::SetShowMobAIFlagFlag(int iFlag)
{
	m_Config.bShowMobAIFlag = iFlag == 1 ? true : false;
}
#endif
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
bool CPythonSystem::IsShowMobLevel()
{
	return m_Config.bShowMobLevel;
}

void CPythonSystem::SetShowMobLevelFlag(int iFlag)
{
	m_Config.bShowMobLevel = iFlag == 1 ? true : false;
}
#endif

bool CPythonSystem::IsAutoTiling()
{
	if (m_Config.bSoftwareTiling == 0)
		return true;

	return false;
}

void CPythonSystem::SetSoftwareTiling(bool isEnable)
{
	if (isEnable)
		m_Config.bSoftwareTiling=1;
	else
		m_Config.bSoftwareTiling=2;
}

bool CPythonSystem::IsSoftwareTiling()
{
	if (m_Config.bSoftwareTiling==1)
		return true;

	return false;
}

bool CPythonSystem::IsUseDefaultIME()
{
	return m_Config.bUseDefaultIME;
}

bool CPythonSystem::LoadConfig()
{
	FILE * fp = NULL;

	if (NULL == (fp = fopen("metin2.cfg", "rt")))
		return false;

	char buf[256];
	char command[256];
	char value[256];

	while (fgets(buf, 256, fp))
	{
		if (sscanf(buf, " %s %s\n", command, value) == EOF)
			break;

		if (!stricmp(command, "WIDTH"))
			m_Config.width = atoi(value);
		else if (!stricmp(command, "HEIGHT"))
			m_Config.height = atoi(value);
		else if (!stricmp(command, "BPP"))
			m_Config.bpp = atoi(value);
		else if (!stricmp(command, "FREQUENCY"))
			m_Config.frequency = atoi(value);
		else if (!stricmp(command, "SOFTWARE_CURSOR"))
			m_Config.is_software_cursor = atoi(value) ? true : false;
		else if (!stricmp(command, "OBJECT_CULLING"))
			m_Config.is_object_culling = atoi(value) ? true : false;
		else if (!stricmp(command, "VISIBILITY"))
			m_Config.iDistance = atoi(value);
		else if (!stricmp(command, "SHARPNESS"))
			m_Config.fSharpnessPower = atof(value);
		else if (!stricmp(command, "SHADOW_QUALITY_LEVEL"))
			m_Config.iShadowQualityLevel = atoi(value);
		else if (!stricmp(command, "TERRAIN_SHADOW"))
			m_Config.iTerrainShadow = atoi(value);
		else if (!stricmp(command, "SHADOW_DETAILS"))
			m_Config.iTreeShadow = atoi(value);
		else if (!stricmp(command, "RAIN_LEVEL"))
			m_Config.iRainLevel = atoi(value);
		else if (!stricmp(command, "GRASS_TYPE"))
			m_Config.iGrassType = atoi(value);
		else if (!stricmp(command, "SPECULAR"))
			m_Config.iSpecularLevel = atoi(value);
		else if (!stricmp(command, "BLOOD"))
			m_Config.iBloodLevel = atoi(value);
		else if (!stricmp(command, "SHADOW_LEVEL"))
			m_Config.iShadowLevel = atoi(value);
		else if (!stricmp(command, "GRAPHICS_LEVEL"))
			m_Config.iGraphicsLevel = atoi(value);
		else if (!stricmp(command, "MUSIC_VOLUME")) {
			if(strchr(value, '.') == 0) { // Old compatiability
				m_Config.music_volume = pow(10.0f, (-1.0f + (((float) atoi(value)) / 5.0f)));
				if(atoi(value) == 0)
					m_Config.music_volume = 0.0f;
			} else
				m_Config.music_volume = atof(value);
		} else if (!stricmp(command, "VOICE_VOLUME"))
			m_Config.voice_volume = (char) atoi(value);
		else if (!stricmp(command, "GAMMA"))
			m_Config.gamma = atoi(value);
#ifdef ENABLE_OFFLINESHOP_SYSTEM
		else if (!stricmp(command, "shop_range"))
		{
			if (strchr(value, '.') == 0)
			{
				m_Config.shopnames_range = pow(10.0f, (-1.0f + (((float)atoi(value)) / 5.0f)));
				if (atoi(value) == 0)
					m_Config.shopnames_range = 10.0f;
			}
			else
				m_Config.shopnames_range = atof(value);
		}
#endif
		else if (!stricmp(command, "IS_SAVE_ID"))
			m_Config.isSaveID = atoi(value);
		else if (!stricmp(command, "SAVE_ID"))
			strncpy(m_Config.SaveID, value, 20);
		else if (!stricmp(command, "WINDOWED"))
		{
			m_Config.bWindowed = atoi(value) == 1 ? true : false;
		}
		else if (!stricmp(command, "USE_DEFAULT_IME"))
			m_Config.bUseDefaultIME = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SOFTWARE_TILING"))
			m_Config.bSoftwareTiling = atoi(value);
		else if (!stricmp(command, "DECOMPRESSED_TEXTURE"))
			m_Config.bDecompressDDS = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "NO_SOUND_CARD"))
			m_Config.bNoSoundCard = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "VIEW_CHAT"))
			m_Config.bViewChat = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "ALWAYS_VIEW_NAME"))
			m_Config.bAlwaysShowName = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SHOW_DAMAGE"))
			m_Config.bShowDamage = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SHOW_SALESTEXT"))
			m_Config.bShowSalesText = atoi(value) == 1 ? true : false;
#ifdef ENABLE_IKASHOP_RENEWAL
#ifdef EXTEND_IKASHOP_PRO
		else if (!stricmp(command, "SHOP_RANGE_VIEW"))
			m_Config.shopRangeView = std::stof(value);
#endif
#endif
//#ifdef ENABLE_FOV_OPTION
//        else if (!stricmp(command, "FIELD_OF_VIEW"))
//            m_Config.iFOVLevel = atoi(value);
//#endif
#if defined(__BL_FOG_FIX__)
		else if (!stricmp(command, "FOG_MODE_ON"))
			m_Config.bIsFogModeEnabled = atoi(value) == 1 ? true : false;
#endif
#if defined(__BL_ENVIRONMENT_EFFECT_OPTION__)
		else if (!stricmp(command, "NIGHT_MODE_ON"))
			m_Config.bNightMode = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SNOW_MODE_ON"))
			m_Config.bSnowMode = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "SNOW_TEXTURE_MODE"))
			m_Config.bSnowTextureMode = atoi(value) == 1 ? true : false;
		else if (!stricmp(command, "RAIN_MODE_ON"))
			m_Config.bRainMode = atoi(value) == 1 ? true : false;
#endif

#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
		else if (!stricmp(command, "SHOW_MOBAIFLAG"))
			m_Config.bShowMobAIFlag = atoi(value) == 1 ? true : false;
#endif
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
		else if (!stricmp(command, "SHOW_MOBLEVEL"))
			m_Config.bShowMobLevel = atoi(value) == 1 ? true : false;
#endif
	}

	if (m_Config.bWindowed)
	{
		unsigned screen_width = GetSystemMetrics(SM_CXFULLSCREEN);
		unsigned screen_height = GetSystemMetrics(SM_CYFULLSCREEN);

		if (screen_width >= 800 && (m_Config.width > screen_width || m_Config.width < 800))
		{
			m_Config.width = screen_width;
		}
		if (screen_height >= 600 && (m_Config.height > screen_height || m_Config.height < 600))
		{
			m_Config.height = screen_height;
		}
	}

	#if defined(__BL_ENVIRONMENT_EFFECT_OPTION__)
	if (!m_Config.bRainMode)
		m_Config.iRainLevel = 0;
	else if (m_Config.iRainLevel == 0)
		m_Config.iRainLevel = 3;
#endif

#ifdef __ANDROID__
	if (g_AndroidScreenWidth > 0 && g_AndroidScreenHeight > 0)
	{
		m_Config.width  = (DWORD)g_AndroidScreenWidth;
		m_Config.height = (DWORD)g_AndroidScreenHeight;
	}
#endif
	m_OldConfig = m_Config;

	fclose(fp);

#if defined(__BL_PICK_FILTER__)
	TPickUpFilter.Save();
#endif

	return true;
}

bool CPythonSystem::SaveConfig()
{
	FILE *fp;

	if (NULL == (fp = fopen("metin2.cfg", "wt")))
		return false;

	fprintf(fp, "WIDTH						%d\n"
				"HEIGHT						%d\n"
				"BPP						%d\n"
				"FREQUENCY					%d\n"
				"SOFTWARE_CURSOR			%d\n"
				"OBJECT_CULLING				%d\n"
				"VISIBILITY					%d\n"
				"MUSIC_VOLUME				%.3f\n"
				"VOICE_VOLUME				%d\n"
				"GAMMA						%d\n"
				"shop_range					%.3f\n"
				"IS_SAVE_ID					%d\n"
				"SAVE_ID					%s\n"
				"DECOMPRESSED_TEXTURE		%d\n",
				m_Config.width,
				m_Config.height,
				m_Config.bpp,
				m_Config.frequency,
				m_Config.is_software_cursor,
				m_Config.is_object_culling,
				m_Config.iDistance,
				m_Config.music_volume,
				m_Config.voice_volume,
				m_Config.gamma,
				m_Config.shopnames_range,
				m_Config.isSaveID,
				m_Config.SaveID,
				m_Config.bDecompressDDS);
#if defined(__BL_FOG_FIX__)
	fprintf(fp, "FOG_MODE_ON\t\t\t%d\n", m_Config.bIsFogModeEnabled);
#endif
	if (m_Config.bWindowed == 1)
		fprintf(fp, "WINDOWED				%d\n", m_Config.bWindowed);
	if (m_Config.bViewChat == 0)
		fprintf(fp, "VIEW_CHAT				%d\n", m_Config.bViewChat);
	if (m_Config.bAlwaysShowName != DEFAULT_VALUE_ALWAYS_SHOW_NAME)
		fprintf(fp, "ALWAYS_VIEW_NAME		%d\n", m_Config.bAlwaysShowName);
	if (m_Config.bShowDamage == 0)
		fprintf(fp, "SHOW_DAMAGE		%d\n", m_Config.bShowDamage);
	if (m_Config.bShowSalesText == 0)
		fprintf(fp, "SHOW_SALESTEXT		%d\n", m_Config.bShowSalesText);
#ifdef ENABLE_IKASHOP_RENEWAL
#ifdef EXTEND_IKASHOP_PRO
	fprintf(fp, "SHOP_RANGE_VIEW		%f\n", m_Config.shopRangeView);
#endif
#endif
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
	if (m_Config.bShowMobAIFlag == 0)
		fprintf(fp, "SHOW_MOBAIFLAG		%d\n", m_Config.bShowMobAIFlag);
#endif
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
	if (m_Config.bShowMobLevel == 0)
		fprintf(fp, "SHOW_MOBLEVEL		%d\n", m_Config.bShowMobLevel);
#endif

	fprintf(fp, "USE_DEFAULT_IME		%d\n", m_Config.bUseDefaultIME);
	fprintf(fp, "SOFTWARE_TILING		%d\n", m_Config.bSoftwareTiling);
	fprintf(fp, "SHADOW_LEVEL			%d\n", m_Config.iShadowLevel);
//#ifdef ENABLE_FOV_OPTION
 //   fprintf(fp, "FIELD_OF_VIEW            %.1f\n", m_Config.iFOVLevel);
//#endif
#if defined(__BL_ENVIRONMENT_EFFECT_OPTION__)
	fprintf(fp, "NIGHT_MODE_ON\t\t\t%d\n", m_Config.bNightMode);
	fprintf(fp, "SNOW_MODE_ON\t\t\t%d\n", m_Config.bSnowMode);
	fprintf(fp, "SNOW_TEXTURE_MODE\t\t%d\n", m_Config.bSnowTextureMode);
	fprintf(fp, "RAIN_MODE_ON\t\t\t%d\n", m_Config.bRainMode);
#endif
	fprintf(fp, "GRAPHICS_LEVEL				%d\n", m_Config.iGraphicsLevel);
	fprintf(fp, "SHADOW_QUALITY_LEVEL		%d\n", m_Config.iShadowQualityLevel);
	fprintf(fp, "TERRAIN_SHADOW				%d\n", m_Config.iTerrainShadow);
	fprintf(fp, "SHADOW_DETAILS				%d\n", m_Config.iTreeShadow);
	fprintf(fp, "RAIN_LEVEL					%d\n", m_Config.iRainLevel);
	fprintf(fp, "GRASS_TYPE					%d\n", m_Config.iGrassType);
	fprintf(fp, "SPECULAR					%d\n", m_Config.iSpecularLevel);
	fprintf(fp, "BLOOD						%d\n", m_Config.iBloodLevel);
	fprintf(fp, "SHARPNESS					%.2f\n", m_Config.fSharpnessPower);
	fprintf(fp, "\n");

	fclose(fp);
	return true;
}

bool CPythonSystem::LoadInterfaceStatus()
{
	FILE * File;
	File = fopen("interface.cfg", "rb");

	if (!File)
		return false;

	fread(m_WindowStatus, 1, sizeof(TWindowStatus) * WINDOW_MAX_NUM, File);
	fclose(File);
	return true;
}

void CPythonSystem::SaveInterfaceStatus()
{
	if (!m_poInterfaceHandler)
		return;

	PyCallClassMemberFunc(m_poInterfaceHandler, "OnSaveInterfaceStatus", Py_BuildValue("()"));

	FILE * File;

	File = fopen("interface.cfg", "wb");

	if (!File)
	{
		TraceError("Cannot open interface.cfg");
		return;
	}

	fwrite(m_WindowStatus, 1, sizeof(TWindowStatus) * WINDOW_MAX_NUM, File);
	fclose(File);
}

bool CPythonSystem::isInterfaceConfig()
{
	return m_isInterfaceConfig;
}

const CPythonSystem::TWindowStatus & CPythonSystem::GetWindowStatusReference(int iIndex)
{
	return m_WindowStatus[iIndex];
}

void CPythonSystem::ApplyConfig()
{
	if (m_OldConfig.gamma != m_Config.gamma)
	{
		float val = 1.0f;

		switch (m_Config.gamma)
		{
			case 0: val = 0.4f;	break;
			case 1: val = 0.7f; break;
			case 2: val = 1.0f; break;
			case 3: val = 1.2f; break;
			case 4: val = 1.4f; break;
		}

		CPythonGraphic::Instance().SetGamma(val);
	}

	if (m_OldConfig.is_software_cursor != m_Config.is_software_cursor)
	{
		if (m_Config.is_software_cursor)
			CPythonApplication::Instance().SetCursorMode(CPythonApplication::CURSOR_MODE_SOFTWARE);
		else
			CPythonApplication::Instance().SetCursorMode(CPythonApplication::CURSOR_MODE_HARDWARE);
	}

	m_OldConfig = m_Config;

	ChangeSystem();
}

void CPythonSystem::ChangeSystem()
{
	CSoundManager& rkSndMgr = CSoundManager::Instance();
	rkSndMgr.SetMusicVolume(m_Config.music_volume);
	rkSndMgr.SetSoundVolumeGrade(m_Config.voice_volume);
#if defined(__BL_ENVIRONMENT_EFFECT_OPTION__)
	CPythonBackground& rkBG = CPythonBackground::Instance();

	rkBG.SetNightModeOption(m_Config.bNightMode);
	rkBG.SetSnowModeOption(m_Config.bSnowMode);
	rkBG.SetSnowTextureModeOption(m_Config.bSnowTextureMode);
#endif
#if defined(__BL_FOG_FIX__)
	CPythonBackground::Instance().SetFogMode(m_Config.bIsFogModeEnabled);
#endif
}

#if defined(__BL_ENVIRONMENT_EFFECT_OPTION__)
void CPythonSystem::SetNightModeOption(bool bEnable)
{
	m_Config.bNightMode = bEnable;
}

bool CPythonSystem::GetNightModeOption() const
{
	return m_Config.bNightMode;
}

void CPythonSystem::SetSnowModeOption(bool bEnable)
{
	m_Config.bSnowMode = bEnable;
}

bool CPythonSystem::GetSnowModeOption() const
{
	return m_Config.bSnowMode;
}

void CPythonSystem::SetSnowTextureModeOption(bool bEnable)
{
	m_Config.bSnowTextureMode = bEnable;
}

bool CPythonSystem::GetSnowTextureModeOption() const
{
	return m_Config.bSnowTextureMode;
}
#endif

#if defined(__BL_PICK_FILTER__)
#include <fstream>

CPythonSystem::CPickUpFilter::CPickUpFilter()
{
	///// READ ////
	std::ifstream inputFile;
	inputFile.open(cPickUpFilterFileName, std::ios::binary);
	if (inputFile.is_open())
	{
		inputFile.read(reinterpret_cast<char*>(bPickFilter),	sizeof(bool) * EPICKFILTER::EPICKFILTER_MAX);
		inputFile.read(reinterpret_cast<char*>(bPickSize),		sizeof(bool) * ESIZE::ESIZE_MAX);
		inputFile.read(reinterpret_cast<char*>(&bModeAll),		sizeof(bool));
		inputFile.read(reinterpret_cast<char*>(&m_bRefineMin),	sizeof(BYTE));
		inputFile.read(reinterpret_cast<char*>(&m_bRefineMax),	sizeof(BYTE));
		inputFile.read(reinterpret_cast<char*>(&m_lLevelMin),	sizeof(long));
		inputFile.read(reinterpret_cast<char*>(&m_lLevelMax),	sizeof(long));

		inputFile.close();
	}
	else
	{
		///// SET DEFAULT ////
		std::fill(std::begin(bPickFilter), std::end(bPickFilter), true);
		std::fill(std::begin(bPickSize), std::end(bPickSize), true);

		bModeAll = false;
		
		m_bRefineMin = 0;
		m_bRefineMax = 9;

		m_lLevelMin = 0;
		m_lLevelMax = 999;
	}
}

void CPythonSystem::CPickUpFilter::Save()
{
	std::ofstream outputFile;
	outputFile.open(cPickUpFilterFileName, std::ios::binary);
	if (outputFile.is_open())
	{
		outputFile.write(reinterpret_cast<char*>(bPickFilter),		sizeof(bool) * EPICKFILTER::EPICKFILTER_MAX);
		outputFile.write(reinterpret_cast<char*>(bPickSize),		sizeof(bool) * ESIZE::ESIZE_MAX);
		outputFile.write(reinterpret_cast<char*>(&bModeAll),		sizeof(bool));
		outputFile.write(reinterpret_cast<char*>(&m_bRefineMin),	sizeof(BYTE));
		outputFile.write(reinterpret_cast<char*>(&m_bRefineMax),	sizeof(BYTE));
		outputFile.write(reinterpret_cast<char*>(&m_lLevelMin),		sizeof(long));
		outputFile.write(reinterpret_cast<char*>(&m_lLevelMax),		sizeof(long));

		outputFile.close();
	}
	else
	{
		Tracenf("CPickUpFilter::Save() Cannot create a file for save settings.");
	}
}

CPythonSystem::CPickUpFilter::~CPickUpFilter()
{
	Save();
}

void CPythonSystem::CPickUpFilter::SetFilter(size_t sIndex, bool b)
{
	if (sIndex >= EPICKFILTER::EPICKFILTER_MAX)
	{
		Tracenf("CPickUpFilter::SetFilter(Index=%d) : Out of range", sIndex);
		return;
	}

	bPickFilter[sIndex] = b;
}

void CPythonSystem::CPickUpFilter::SetSize(size_t sIndex, bool b)
{
	if (sIndex >= ESIZE::ESIZE_MAX)
	{
		Tracenf("CPickUpFilter::SetSize(Index=%d) : Out of range", sIndex);
		return;
	}

	if (sIndex == ESIZE::BIG && b == false)
		SetFilter(EPICKFILTER::SUB_WEAPON_TWO_HANDED, false);

	bPickSize[sIndex] = b;
}

void CPythonSystem::CPickUpFilter::SetRefine(BYTE min, BYTE max)
{
	m_bRefineMin = static_cast<BYTE>(MINMAX(0, min, 9));
	m_bRefineMax = static_cast<BYTE>(MINMAX(0, max, 9));
}

void CPythonSystem::CPickUpFilter::SetLevel(long min, long max)
{
	m_lLevelMin = MINMAX(0, min, 999);
	m_lLevelMax = MINMAX(0, max, 999);
}

void CPythonSystem::CPickUpFilter::SetModeAll(bool b)
{
	bModeAll = b;
}

void CPythonSystem::CPickUpFilter::ClearExceptions()
{
	m_setWhitelist.clear();
	m_setBlacklist.clear();
}

void CPythonSystem::CPickUpFilter::AddWhitelist(const char* szName)
{
	std::string str(szName);
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	m_setWhitelist.insert(str);
}

void CPythonSystem::CPickUpFilter::AddBlacklist(const char* szName)
{
	std::string str(szName);
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	m_setBlacklist.insert(str);
}

bool CPythonSystem::CPickUpFilter::CanPickItem(DWORD dwIID)
{
	CItemData* pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(CPythonItem::Instance().GetVirtualNumberOfGroundItem(dwIID), &pItemData))
	{
		Tracenf("CPickUpFilter::CanPickItem(dwIID=%d) : Non-exist item.", dwIID);
		return true;
	}

	// --- EXCEPTION CHECK START ---
	std::string strName = pItemData->GetName();
	
	// Strip +0 ~ +9 to match the UI's base name formatting
	size_t plusPos = strName.find('+');
	if (plusPos != std::string::npos && plusPos < strName.length() - 1 && isdigit(strName[plusPos + 1]))
	{
		strName = strName.substr(0, plusPos);
	}

	// Trim right side spaces to prevent trailing whitespace mismatches
	strName.erase(strName.find_last_not_of(" \n\r\t") + 1);

	// Convert to lowercase for case-insensitive matching
	std::transform(strName.begin(), strName.end(), strName.begin(), ::tolower);

	// If it's in the blacklist, NEVER pick it up.
	if (m_setBlacklist.find(strName) != m_setBlacklist.end())
		return false;

	// If it's in the whitelist, ALWAYS pick it up (bypassing category rules).
	if (m_setWhitelist.find(strName) != m_setWhitelist.end())
		return true;
	// --- EXCEPTION CHECK END ---

	if (CheckRefine(pItemData) == false)
		return false;

	if (CheckLevel(pItemData) == false)
		return false;

	if (CheckSize(pItemData) == false)
		return false;

	if (CheckType(pItemData) == false)
		return false;
	
	return true;
}

std::pair<BYTE, BYTE> CPythonSystem::CPickUpFilter::GetRefine()
{
	return std::make_pair(m_bRefineMin, m_bRefineMax);
}

std::pair<long, long> CPythonSystem::CPickUpFilter::GetLevel()
{
	return std::make_pair(m_lLevelMin, m_lLevelMax);
}

bool CPythonSystem::CPickUpFilter::GetFilter(size_t sIndex) const
{
	if (sIndex >= EPICKFILTER::EPICKFILTER_MAX)
	{
		Tracenf("CPickUpFilter::GetFilter(Index=%d) : Out of range", sIndex);
		return false;
	}

	return bPickFilter[sIndex];
}

bool CPythonSystem::CPickUpFilter::GetSize(size_t sIndex) const
{
	if (sIndex >= ESIZE::ESIZE_MAX)
	{
		Tracenf("CPickUpFilter::GetSize(Index=%d) : Out of range", sIndex);
		return false;
	}

	return bPickSize[sIndex];
}

bool CPythonSystem::CPickUpFilter::IsModeAll() const
{
	return bModeAll;
}

bool CPythonSystem::CPickUpFilter::CheckRefine(const CItemData* pItem) const
{
	const BYTE bType = pItem->GetType();
	
	// FIX: Only check Refine levels if the item is actually Equipment
	if (bType != CItemData::ITEM_TYPE_WEAPON && bType != CItemData::ITEM_TYPE_ARMOR)
		return true;

	const BYTE bRefineLevel = static_cast<BYTE>(pItem->GetRefine());
	if (bRefineLevel >= m_bRefineMin && bRefineLevel <= m_bRefineMax)
		return true;

	return false;
}

bool CPythonSystem::CPickUpFilter::CheckLevel(const CItemData* pItem) const
{
	const BYTE bType = pItem->GetType();
	
	// FIX: Only check Usage Level limits if the item is actually Equipment
	if (bType != CItemData::ITEM_TYPE_WEAPON && bType != CItemData::ITEM_TYPE_ARMOR)
		return true;

	CItemData::TItemLimit ItemLimit;
	for (BYTE i = 0; i < CItemData::ITEM_LIMIT_MAX_NUM; i++)
	{
		if (!pItem->GetLimit(i, &ItemLimit))
			continue;

		if (ItemLimit.bType != CItemData::LIMIT_LEVEL)
			continue;

		const long lLimitLevel = ItemLimit.lValue;
		return (lLimitLevel >= m_lLevelMin && lLimitLevel <= m_lLevelMax);
	}

	return true; // Failsafe for equipment with no level limits
}

bool CPythonSystem::CPickUpFilter::CheckSize(const CItemData* pItem) const
{
	return GetSize(pItem->GetSize() - 1);
}

bool IsRegularPotion(DWORD vnum)
{
	if (vnum >= 27001 && vnum <= 27008) return true;
	if (vnum == 27051 || vnum == 27052) return true;
	if (vnum >= 27116 && vnum <= 27122) return true;
	if (vnum >= 27201 && vnum <= 27206) return true;
	return false;
}

bool IsHairDyeItem(DWORD vnum)
{
	// Standard Metin2 Hair Dye VNUMs + Bleach
	return (vnum >= 70201 && vnum <= 70206);
}

bool IsEventItem(DWORD vnum)
{
	// Check specific event boxes and items extracted from script
	switch (vnum)
	{
		case 79505: // Okey Cards
		case 50024: case 50025: // Chocolate / Rose
		case 50096: // Football
		case 50036: case 70063: case 70064: // Hair items
		case 50253: // Herbs
		case 51501: // Cor Draconis
		case 71146: case 71147: // Love Boxes
		case 50011: // Moonlight
		case 87030: case 87031: case 87032: // Pet Boxes
		case 50037: // Hexagonal / Triangle
		case 50034: // Secrets Box
		case 100300: // Dragon Bean
		case 87055: // Kham Box
			return true;
	}

	// Check Mount Vnums (52701 - 52708) or Rare Mounts list
	if ((vnum >= 52701 && vnum <= 52708) ||
		vnum == 71182 || vnum == 71183 || vnum == 71184 || vnum == 71185 || vnum == 71186 || vnum == 71187 ||
		vnum == 71161 || vnum == 71164 || vnum == 71165 || vnum == 71166 ||
		vnum == 71137 || vnum == 71138 || vnum == 71139 || vnum == 71140 || vnum == 71141 || vnum == 71142 || vnum == 71131)
	{
		return true;
	}

	return false;
}

bool IsEventBoxItem(DWORD vnum)
{
	switch (vnum)
	{
		case 50011: // Moonlight
		case 50037: // Hexagonal
		case 50034: // Secrets Box
		case 71146: case 71147: // Love Boxes
		case 87030: case 87031: case 87032: // Pet Boxes
		case 87055: // Kham Box
			return true;
	}
	return false;
}

bool CPythonSystem::CPickUpFilter::CheckType(const CItemData* pItem) const
{
	const BYTE bType = pItem->GetType();
	const BYTE bSubType = pItem->GetSubType();
	const DWORD dwVnum = pItem->GetIndex(); // CItemData uses GetIndex() for VNUM

	// 1. Check Event Items FIRST
	if (IsEventItem(dwVnum))
	{
		if (IsEventBoxItem(dwVnum))
			return GetFilter(EPICKFILTER::SUB_EVENT_BOXES);
		return GetFilter(EPICKFILTER::SUB_EVENT_ITEMS);
	}

	// 2. Main Type Routing
	switch (bType)
	{
	case CItemData::EItemType::ITEM_TYPE_USE:
		switch (bSubType)
		{
		case CItemData::EUseSubTypes::USE_POTION:
		case CItemData::EUseSubTypes::USE_ABILITY_UP:
		case CItemData::EUseSubTypes::USE_POTION_NODELAY:
		case CItemData::EUseSubTypes::USE_POTION_CONTINUE:
			if (IsRegularPotion(dwVnum))
				return GetFilter(EPICKFILTER::SUB_POTION_REGULAR);
			return GetFilter(EPICKFILTER::SUB_POTION_ABILITY);
		}
		// Safely check VNUMs instead of calling a missing client-side method
		if (IsHairDyeItem(dwVnum))
			return GetFilter(EPICKFILTER::SUB_POTION_HAIRDYE);
		break;

	case CItemData::EItemType::ITEM_TYPE_WEAPON:
		switch (bSubType)
		{
		case CItemData::EWeaponSubTypes::WEAPON_SWORD:
			return GetFilter(EPICKFILTER::SUB_WEAPON_SWORD);
		case CItemData::EWeaponSubTypes::WEAPON_DAGGER:
			return GetFilter(EPICKFILTER::SUB_WEAPON_DAGGER);
		case CItemData::EWeaponSubTypes::WEAPON_BOW:
			return GetFilter(EPICKFILTER::SUB_WEAPON_BOW);
		case CItemData::EWeaponSubTypes::WEAPON_TWO_HANDED:
			return GetFilter(EPICKFILTER::SUB_WEAPON_TWO_HANDED);
		case CItemData::EWeaponSubTypes::WEAPON_BELL:
			return GetFilter(EPICKFILTER::SUB_WEAPON_BELL);
		case CItemData::EWeaponSubTypes::WEAPON_FAN:
			return GetFilter(EPICKFILTER::SUB_WEAPON_FAN);
		case CItemData::EWeaponSubTypes::WEAPON_ARROW:
			return GetFilter(EPICKFILTER::SUB_WEAPON_ARROW);
		case CItemData::EWeaponSubTypes::WEAPON_CLAW:
			return GetFilter(EPICKFILTER::SUB_WEAPON_CLAW);
		}
		break;

	case CItemData::EItemType::ITEM_TYPE_ARMOR:
		switch (bSubType)
		{
		case CItemData::EArmorSubTypes::ARMOR_BODY:
			return GetFilter(EPICKFILTER::SUB_ARMOR_BODY);
		case CItemData::EArmorSubTypes::ARMOR_HEAD:
			return GetFilter(EPICKFILTER::SUB_ARMOR_HEAD);
		case CItemData::EArmorSubTypes::ARMOR_SHIELD:
			return GetFilter(EPICKFILTER::SUB_ARMOR_SHIELD);
		case CItemData::EArmorSubTypes::ARMOR_WRIST:
			return GetFilter(EPICKFILTER::SUB_ARMOR_WRIST);
		case CItemData::EArmorSubTypes::ARMOR_FOOTS:
			return GetFilter(EPICKFILTER::SUB_ARMOR_FOOTS);
		case CItemData::EArmorSubTypes::ARMOR_NECK:
			return GetFilter(EPICKFILTER::SUB_ARMOR_NECK);
		case CItemData::EArmorSubTypes::ARMOR_EAR:
			return GetFilter(EPICKFILTER::SUB_ARMOR_EAR);
		case CItemData::EArmorSubTypes::ARMOR_PENDANT:
			return GetFilter(EPICKFILTER::SUB_ARMOR_PENDANT);
		case CItemData::EArmorSubTypes::ARMOR_GLOVE:
			return GetFilter(EPICKFILTER::SUB_ARMOR_GLOVE);
		}
		break;

	case CItemData::EItemType::ITEM_TYPE_METIN:
		return GetFilter(EPICKFILTER::TYPE_METIN);

	case CItemData::EItemType::ITEM_TYPE_ELK:
		return GetFilter(EPICKFILTER::TYPE_YANG);

	case CItemData::EItemType::ITEM_TYPE_SKILLBOOK:
		return GetFilter(EPICKFILTER::TYPE_SKILLBOOK);

	case CItemData::EItemType::ITEM_TYPE_GIFTBOX:
		return GetFilter(EPICKFILTER::TYPE_GIFTBOX);

	case CItemData::EItemType::ITEM_TYPE_BELT:
		return GetFilter(EPICKFILTER::TYPE_BELT);

	case CItemData::EItemType::ITEM_TYPE_POLYMORPH:
		return GetFilter(EPICKFILTER::TYPE_POLY);

	case CItemData::EItemType::ITEM_TYPE_UNIQUE:
	case CItemData::EItemType::ITEM_TYPE_RING:
		return GetFilter(EPICKFILTER::TYPE_RING);

	case CItemData::EItemType::ITEM_TYPE_MATERIAL:
		return GetFilter(EPICKFILTER::TYPE_MATERIAL);
	}

	return true;
}
#endif

void CPythonSystem::Clear()
{
	SetInterfaceHandler(NULL);
}

CPythonSystem::CPythonSystem()
{
	memset(&m_Config, 0, sizeof(TConfig));

	m_poInterfaceHandler = NULL;

	SetDefaultConfig();

	LoadConfig();

	ChangeSystem();

	if (LoadInterfaceStatus())
		m_isInterfaceConfig = true;
	else
		m_isInterfaceConfig = false;
}

CPythonSystem::~CPythonSystem()
{
	assert(m_poInterfaceHandler==NULL && "CPythonSystem MUST CLEAR!");
}

#if defined(__BL_ENVIRONMENT_EFFECT_OPTION__)
void CPythonSystem::SetRainModeOption(bool bEnable)
{
	m_Config.bRainMode = bEnable;
}

bool CPythonSystem::GetRainModeOption() const
{
	return m_Config.bRainMode;
}
#endif
//martysama0134's fb0d8d5d0566583187a94f9a823b2075