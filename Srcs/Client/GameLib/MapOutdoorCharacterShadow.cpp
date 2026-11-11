#include "StdAfx.h"
#include "../EterLib/StateManager.h"
#include "../EterLib/Camera.h"
#include "FlyingObjectManager.h"
#include "MapManager.h"
#include "MapOutdoor.h"
#include "../UserInterface/PythonBackground.h"
#include <sstream>
#include <string>
#include <cmath>
#include <random>
#include <unordered_set>
#include <unordered_map>
#include <chrono>

int radomzPosition = 1;

float z = 0.025f;
float zMultiplier = 15.0f;
float tolerance = 0.001f;

bool a = true;
D3DXVECTOR3 VECTOR3(0.0f, 0.0f, 1.0f);

static void SanitizeSunVector(D3DXVECTOR3& rSun, const D3DXVECTOR3* pEnvSun = nullptr)
{
	bool bValid = _finite(rSun.x) && _finite(rSun.y) && _finite(rSun.z) &&
	              (std::abs(rSun.x) <= 2.0f) && (std::abs(rSun.y) <= 2.0f) && (std::abs(rSun.z) <= 2.0f) &&
	              (D3DXVec3LengthSq(&rSun) >= 0.001f);

	if (!bValid)
	{
		if (pEnvSun && _finite(pEnvSun->x) && _finite(pEnvSun->y) && _finite(pEnvSun->z) && D3DXVec3LengthSq(pEnvSun) >= 0.001f)
			rSun = *pEnvSun;
		else
			rSun = D3DXVECTOR3(0.35f, 0.35f, 0.85f);
	}

	// Prevent D3DXMatrixLookAtRH gimbal lock when Up vector is (0, 0, 1)
	if ((rSun.x * rSun.x + rSun.y * rSun.y) < 0.0004f)
	{
		rSun.x = 0.02f;
		rSun.y = 0.02f;
	}
}

#include "../EterPack/EterPackManager.h"
#include "shadowmapVS.h"
#include "shadowmapPS.h"
#include "terrainShadowsVS.h"
#include "terrainShadowsPS.h"
#include "shadowmapObjectsVS.h"
#include "shadowmapObjectsPS.h"
#include "shadowmapWaterVS.h"
#include "shadowmapWaterPSceva.h"
#include "shadowmapTreeVS.h"
#include "shadowmapTreePS.h"
#include "shadowmapDepthVS.h"
#include "shadowmapDepthPS.h"
#include "grassPS.h"
#include "grassVS.h"
#include "../UserInterface/InstanceBase.h"

LPDIRECT3DVERTEXSHADER9 CMapOutdoor::LoadAndBuildVertexShader(const char* filename)
{
	LPD3DXBUFFER pShaderBuffer = nullptr;
	LPD3DXBUFFER pErrorBuffer = nullptr;

	HRESULT hr = D3DXCompileShader(
		filename,
		strlen(filename),
		nullptr,
		nullptr,
		"main",
		"vs_3_0",
		0,
		&pShaderBuffer,
		&pErrorBuffer,
		nullptr);

	if (FAILED(hr))
		if (pErrorBuffer) {
			const char* errorMsg = static_cast<const char*>(pErrorBuffer->GetBufferPointer());
			TraceError("Vertex shader compilation error %s", errorMsg);
			return nullptr;
		}

	LPDIRECT3DVERTEXSHADER9 shader = nullptr;
	hr = ms_lpd3dDevice->CreateVertexShader(
		reinterpret_cast<DWORD*>(pShaderBuffer->GetBufferPointer()),
		&shader
	);
	if (FAILED(hr)) TraceError("CreateVertexShader failed");

	if (pShaderBuffer)
		pShaderBuffer->Release();

	return shader;
}

LPDIRECT3DPIXELSHADER9 CMapOutdoor::LoadAndBuildPixelShader(const char* filename, bool HLSL)
{
	LPD3DXBUFFER pShaderBuffer = nullptr;
	LPD3DXBUFFER pErrorBuffer = nullptr;
	HRESULT hr;
	if (HLSL) {
		hr = D3DXCompileShader(
			filename,
			strlen(filename),
			nullptr,
			nullptr,
			"main",
			"ps_3_0",
			0,
			&pShaderBuffer,
			&pErrorBuffer,
			nullptr
		);
	}
	else {
		hr = D3DXAssembleShader(
			filename,
			strlen(filename),
			nullptr,
			nullptr,
			0,
			&pShaderBuffer,
			&pErrorBuffer);
	}
	if (FAILED(hr))
	{
		if (pErrorBuffer) {
			const char* errorMsg = static_cast<const char*>(pErrorBuffer->GetBufferPointer());
			TraceError("Pixel shader compilation error %s", errorMsg);
			return nullptr;
		}
	}

	LPDIRECT3DPIXELSHADER9 shader = nullptr;
	hr = ms_lpd3dDevice->CreatePixelShader(
		reinterpret_cast<DWORD*>(pShaderBuffer->GetBufferPointer()),
		&shader
	);
	if (FAILED(hr))
		TraceError("CreatePixelShader failed");

	if (pShaderBuffer)
		pShaderBuffer->Release();

	return shader;
}

void CMapOutdoor::CreateShadowmapShaders()
{
	const bool bIsMode4 = (Frustum::Instance().graphicsLevel >= 4);

	if (!m_lpShadowMapVertexShaderDepth)
		m_lpShadowMapVertexShaderDepth = LoadAndBuildVertexShader(shadowmapDepthVS);

	if (!m_lpShadowMapPixelShaderDepth)
		m_lpShadowMapPixelShaderDepth = LoadAndBuildPixelShader(shadowmapDepthPS);

	// Terrain
	if (!m_lpShadowMapVertexShaderTerrain)
		m_lpShadowMapVertexShaderTerrain = LoadAndBuildVertexShader(terrainShadowsVS);

	if (!m_lpShadowMapPixelShaderTerrain)
	{
		const char* szTerrainShadowPS = bIsMode4 ? terrainShadowsPS : terrainShadowsPS_Mode3;
		m_lpShadowMapPixelShaderTerrain = LoadAndBuildPixelShader(szTerrainShadowPS);
	}

	if (!m_lpPixelShaderTerrainBase)
	{
		const char* szTerrainBasePS = bIsMode4 ? terrainBasePS : terrainBasePS_Mode3;
		m_lpPixelShaderTerrainBase = LoadAndBuildPixelShader(szTerrainBasePS);
	}

	if (!m_lpVertexShaderTerrainBase)
		m_lpVertexShaderTerrainBase = LoadAndBuildVertexShader(terrainBaseVS);

	// Objects
	if (!m_lpShadowMapVertexShaderObjects)
		m_lpShadowMapVertexShaderObjects = LoadAndBuildVertexShader(shadowmapObjectsVS);

	if (!m_lpShadowMapPixelShaderObjects)
		m_lpShadowMapPixelShaderObjects = LoadAndBuildPixelShader(shadowmapObjectsPS);

	// Main render stage shaders
	if (!m_lpShadowMapVertexShaderMain)
		m_lpShadowMapVertexShaderMain = LoadAndBuildVertexShader(shadowmapVS);
	if (!m_lpShadowMapPixelShaderMain)
	{
		const char* szMainPS = bIsMode4 ? shadowmapPS : shadowmapPS_Mode3;
		m_lpShadowMapPixelShaderMain = LoadAndBuildPixelShader(szMainPS);
	}

	// Grass (Used in Mode 4)
	if (!m_lpVertexShaderGrass)
		m_lpVertexShaderGrass = LoadAndBuildVertexShader(grassVertexShader);
	if (!m_lpPixelShaderGrass)
		m_lpPixelShaderGrass = LoadAndBuildPixelShader(grassPixelShader);

#ifdef __ANDROID__
	if (m_lpShadowMapPixelShaderDepth)   m_lpShadowMapPixelShaderDepth->shaderType = PS_TYPE_SHADOW_DEPTH;
	if (m_lpPixelShaderTerrainBase)      m_lpPixelShaderTerrainBase->shaderType = PS_TYPE_TERRAIN_BASE;
	if (m_lpShadowMapPixelShaderTerrain) m_lpShadowMapPixelShaderTerrain->shaderType = PS_TYPE_TERRAIN_SHADOW;
	if (m_lpShadowMapPixelShaderObjects) m_lpShadowMapPixelShaderObjects->shaderType = PS_TYPE_SHADOW_OBJECTS;
	if (m_lpShadowMapPixelShaderMain)    m_lpShadowMapPixelShaderMain->shaderType = PS_TYPE_SHADOW_MAIN;
	if (m_lpPixelShaderGrass)            m_lpPixelShaderGrass->shaderType = PS_TYPE_GRASS;
#endif

	shadersReleased = false;
}

void CMapOutdoor::CreateWaterShaders()
{
	if (!m_lpShadowMapVertexShaderWater)
		m_lpShadowMapVertexShaderWater = LoadAndBuildVertexShader(jtx_waterVertexShader);

	if (!m_lpShadowMapPixelShaderWater)
		m_lpShadowMapPixelShaderWater = LoadAndBuildPixelShader(jtx_waterPixelShader);

#ifdef __ANDROID__
	if (m_lpShadowMapPixelShaderWater)
		m_lpShadowMapPixelShaderWater->shaderType = PS_TYPE_WATER;
#endif

	shadersWaterReleased = false;
}

void CMapOutdoor::CreateTreeShaders()
{
	if (!m_lpShadowMapVertexShaderTree)
		m_lpShadowMapVertexShaderTree = LoadAndBuildVertexShader(shadowmapTreeVS);
	if (!m_lpShadowMapPixelShaderTree)
		m_lpShadowMapPixelShaderTree = LoadAndBuildPixelShader(shadowmapTreePS);
	if (!m_lpShadowMapPixelShaderDepthTree)
		m_lpShadowMapPixelShaderDepthTree = LoadAndBuildPixelShader(shadowmapDepthTreePS, false);

#ifdef __ANDROID__
	if (m_lpShadowMapPixelShaderTree)      m_lpShadowMapPixelShaderTree->shaderType = PS_TYPE_SHADOW_MAIN;
	if (m_lpShadowMapPixelShaderDepthTree) m_lpShadowMapPixelShaderDepthTree->shaderType = PS_TYPE_SHADOW_DEPTH;
#endif

	shadersTreeReleased = false;
}

void CMapOutdoor::ReleaseShaders()
{
	if (m_lpShadowMapVertexShaderDepth)
	{
		m_lpShadowMapVertexShaderDepth->Release();
		m_lpShadowMapVertexShaderDepth = nullptr;
	}

	if (m_lpShadowMapPixelShaderDepth)
	{
		m_lpShadowMapPixelShaderDepth->Release();
		m_lpShadowMapPixelShaderDepth = nullptr;
	}

	if (m_lpShadowMapVertexShaderTerrain)
	{
		m_lpShadowMapVertexShaderTerrain->Release();
		m_lpShadowMapVertexShaderTerrain = nullptr;
	}

	if (m_lpShadowMapPixelShaderTerrain)
	{
		m_lpShadowMapPixelShaderTerrain->Release();
		m_lpShadowMapPixelShaderTerrain = nullptr;
	}

	if (m_lpPixelShaderTerrainBase)
	{
		m_lpPixelShaderTerrainBase->Release();
		m_lpPixelShaderTerrainBase = nullptr;
	}

	if (m_lpVertexShaderTerrainBase)
	{
		m_lpVertexShaderTerrainBase->Release();
		m_lpVertexShaderTerrainBase = nullptr;
	}

	if (m_lpShadowMapVertexShaderObjects)
	{
		m_lpShadowMapVertexShaderObjects->Release();
		m_lpShadowMapVertexShaderObjects = nullptr;
	}

	if (m_lpShadowMapPixelShaderObjects)
	{
		m_lpShadowMapPixelShaderObjects->Release();
		m_lpShadowMapPixelShaderObjects = nullptr;
	}

	if (m_lpShadowMapVertexShaderMain)
	{
		m_lpShadowMapVertexShaderMain->Release();
		m_lpShadowMapVertexShaderMain = nullptr;
	}

	if (m_lpShadowMapPixelShaderMain)
	{
		m_lpShadowMapPixelShaderMain->Release();
		m_lpShadowMapPixelShaderMain = nullptr;
	}

	if (m_lpVertexShaderGrass)
	{
		m_lpVertexShaderGrass->Release();
		m_lpVertexShaderGrass = nullptr;
	}

	if (m_lpPixelShaderGrass)
	{
		m_lpPixelShaderGrass->Release();
		m_lpPixelShaderGrass = nullptr;
	}

	shadersReleased = true;
}

void CMapOutdoor::ReleaseWaterShaders()
{
	if (m_lpShadowMapVertexShaderWater)
	{
		m_lpShadowMapVertexShaderWater->Release();
		m_lpShadowMapVertexShaderWater = nullptr;
	}

	if (m_lpShadowMapPixelShaderWater)
	{
		m_lpShadowMapPixelShaderWater->Release();
		m_lpShadowMapPixelShaderWater = nullptr;
	}

	shadersWaterReleased = true;
}

void CMapOutdoor::ReleaseTreeShaders()
{
	if (m_lpShadowMapVertexShaderTree)
	{
		m_lpShadowMapVertexShaderTree->Release();
		m_lpShadowMapVertexShaderTree = nullptr;
	}

	if (m_lpShadowMapPixelShaderTree)
	{
		m_lpShadowMapPixelShaderTree->Release();
		m_lpShadowMapPixelShaderTree = nullptr;
	}

	if (m_lpShadowMapPixelShaderDepthTree)
	{
		m_lpShadowMapPixelShaderDepthTree->Release();
		m_lpShadowMapPixelShaderDepthTree = nullptr;
	}

	shadersTreeReleased = true;
}

void CMapOutdoor::SetShadowTextureSize(WORD size)
{
#ifdef __ANDROID__
	if (size > 2048)
		size = 2048;
#endif
	if (m_wShadowMapSize != size)
	{
		recreate = true;
		Tracenf("ShadowTextureSize changed %d -> %d", m_wShadowMapSize, size);
	}

	m_wShadowMapSize = size;
}

void CMapOutdoor::LoadDepthTerrainTexture(const char* textureName, IDirect3DTexture9** texture)
{
	if (!texture || *texture != nullptr || !textureName)
		return;

	CMappedFile	mappedFile;
	LPCVOID		c_pvMap = nullptr;

	bool bFound = CEterPackManager::Instance().Get(mappedFile, textureName, &c_pvMap);
#ifdef __ANDROID__
	if (!bFound)
	{
		std::string lowerPath(textureName);
		for (size_t i = 0; i < lowerPath.size(); ++i)
		{
			if (lowerPath[i] == '\\') lowerPath[i] = '/';
			else lowerPath[i] = (char)tolower((unsigned char)lowerPath[i]);
		}
		bFound = CEterPackManager::Instance().Get(mappedFile, lowerPath.c_str(), &c_pvMap);
	}
#endif
	if (bFound && c_pvMap)
	{
#ifdef __ANDROID__
		if (FAILED(D3DXCreateTextureFromFileInMemoryEx(
			ms_lpd3dDevice,
			c_pvMap,
			mappedFile.Size(),
			1024,
			1024,
			D3DX_DEFAULT,
			0,
			D3DFMT_FROM_FILE,
			D3DPOOL_MANAGED,
			D3DX_FILTER_LINEAR,
			D3DX_FILTER_LINEAR,
			0,
			nullptr,
			nullptr,
			texture)))
#endif
		{
			if (FAILED(D3DXCreateTextureFromFileInMemory(ms_lpd3dDevice, static_cast<const char*>(c_pvMap), mappedFile.Size(), texture)))
			{
				TraceError("Texture not loaded: %s", textureName);
			}
		}
	}
}

bool loadTerrainTexture = true;
bool loadTerrainTextureMode4 = true;

void CMapOutdoor::CreateCharacterShadowTexture()
{
	extern bool GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW;

	if (GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW)
		return;

	recreate = false; // <-- Prevents SkyUtil() from recreating shadow textures a 2nd time on frame 1
	ReleaseCharacterShadowTexture();

	if (IsLowTextureMemory())
		SetShadowTextureSize(128);

	m_ShadowMapViewport.X = 1;
	m_ShadowMapViewport.Y = 1;
	m_ShadowMapViewport.Width = m_wShadowMapSize - 2;
	m_ShadowMapViewport.Height = m_wShadowMapSize - 2;
	m_ShadowMapViewport.MinZ = 0.0f;
	m_ShadowMapViewport.MaxZ = 1.0f;

	m_ShadowMapViewportTree.X = 1;
	m_ShadowMapViewportTree.Y = 1;
	if (Frustum::Instance().shadowType == 2)
	{
		m_ShadowMapViewportTree.Width = (m_wShadowMapSize - 2) / 1.0;
		m_ShadowMapViewportTree.Height = (m_wShadowMapSize - 2) / 1.0;
	}
	else
	{
		m_ShadowMapViewportTree.Width = m_wShadowMapSize;
		m_ShadowMapViewportTree.Height = m_wShadowMapSize;
	}
	m_ShadowMapViewportTree.MinZ = 0.0f;
	m_ShadowMapViewportTree.MaxZ = 1.0f;

	if (Frustum::Instance().shadowType == 2)
	{
		if (FAILED(ms_lpd3dDevice->CreateTexture(m_wShadowMapSize / 1.0, m_wShadowMapSize / 1.0, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT, &m_lpCharacterShadowMapTexture, NULL)))
		{
			TraceError("CMapOutdoor Unable to create Character Shadow render target texture\n");
			return;
		}
	}
	else
	{
		if (FAILED(ms_lpd3dDevice->CreateTexture(m_wShadowMapSize, m_wShadowMapSize, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT, &m_lpCharacterShadowMapTexture, NULL)))
		{
			TraceError("CMapOutdoor Unable to create Character Shadow render target texture\n");
			return;
		}
	}

	if (FAILED(m_lpCharacterShadowMapTexture->GetSurfaceLevel(0, &m_lpCharacterShadowMapRenderTargetSurface)))
	{
		TraceError("CMapOutdoor Unable to GetSurfaceLevel Character Shadow render target texture\n");
		return;
	}

	if (FAILED(ms_lpd3dDevice->CreateTexture(m_wShadowMapSize, m_wShadowMapSize, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT, &m_lpCharacterShadowMapTexture4, NULL)))
	{
		TraceError("CMapOutdoor Unable to create Character Shadow render target texture\n");
		return;
	}

	if (FAILED(m_lpCharacterShadowMapTexture4->GetSurfaceLevel(0, &m_lpCharacterShadowMapRenderTargetSurface4)))
	{
		TraceError("CMapOutdoor Unable to GetSurfaceLevel Character Shadow render target texture\n");
		return;
	}

	if (FAILED(ms_lpd3dDevice->CreateTexture(m_wShadowMapSize, m_wShadowMapSize, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R5G6B5, D3DPOOL_DEFAULT, &m_lpCharacterShadowMapTexture2, NULL)))
	{
		TraceError("CMapOutdoor Unable to create Character Shadow render target texture\n");
		return;
	}

	if (FAILED(m_lpCharacterShadowMapTexture2->GetSurfaceLevel(0, &m_lpCharacterShadowMapRenderTargetSurface2)))
	{
		TraceError("CMapOutdoor Unable to GetSurfaceLevel Character Shadow render target texture\n");
		return;
	}

	if (FAILED(ms_lpd3dDevice->CreateDepthStencilSurface(m_wShadowMapSize, m_wShadowMapSize, D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, true, &m_lpCharacterShadowMapDepthSurface, NULL)))
	{
		TraceError("CMapOutdoor Unable to create Character Shadow depth Surface\n");
		return;
	}

	// Base helper textures + Water & Puddle files (Loaded in BOTH Mode 3 and Mode 4)
	LoadDepthTerrainTexture("d:/ymir work/terrainmaps/temp/shadowhelper.png", &Frustum::Instance().shadowTexture);
	LoadDepthTerrainTexture("d:/ymir work/terrainmaps/temp/shadowhelperblack.png", &Frustum::Instance().shadowTextureBlack);
	LoadDepthTerrainTexture("d:/ymir work/terrainmaps/temp/spheremap.jpg", &Frustum::Instance().specularTexture);

	// Required for Mode 3 flat terrain + Mode 3 & 4 puddles and water foam
	LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/empty.dds", &Frustum::Instance().emptyNormalMap);
	LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/brown_mud_03_nor_gl_2k.dds", &Frustum::Instance().brown_mud_03_diff_2k);
	LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/dry_ground_01_nor_gl_2k.dds", &Frustum::Instance().dry_ground_01_diff_4k);
	LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/foam.jpg", &Frustum::Instance().foam);
#ifdef __ANDROID__
	LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/specular/ground_0007_roughness_2k.dds", &Frustum::Instance().waterSandSpecularTexture);
	LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/specular/ground_0019_color_2k.dds", &Frustum::Instance().waterLeavesSpecularTexture);
	LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/specular/ground_0030_color_specular.dds", &Frustum::Instance().iceSpecularTexture);
	LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/specular/Ground050_2K-JPG_Roughness.dds", &Frustum::Instance().sandSpecularTexture);
#endif
	loadTerrainTexture = false;

#ifndef __ANDROID__
	// High-quality ground normal/specular maps, tree bark & grass (ONLY loaded in Graphics Mode 4)
	if (loadTerrainTextureMode4 && Frustum::Instance().graphicsLevel >= 4)
	{
		//JTX UPDATE V1.0.4 Textures create
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/tile_final_normal.dds", &Frustum::Instance().normalMapTile01);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/dirt_normal.dds", &Frustum::Instance().normalMapRacketDirt);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/leafy_grass_nor_gl_2k.dds", &Frustum::Instance().normalMapGrass);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/stone_normal.dds", &Frustum::Instance().normalMapStone);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/snow_steps_normal.dds", &Frustum::Instance().normalMapSnow);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/beach_normal.dds", &Frustum::Instance().normalMapBitch);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/field.dds", &Frustum::Instance().normalMapField);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/sand01_normal.dds", &Frustum::Instance().normalMapSand01);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/sand03_normal.dds", &Frustum::Instance().normalMapSand03);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/dry_ground_rocks_nor_gl_1k.dds", &Frustum::Instance().normalMapDryGroundRock);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/cobblestone_color_nor_gl_2k.dds", &Frustum::Instance().normalMapCobblestoneTemple1);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/cobblestone_floor_13_nor_gl_2k.dds", &Frustum::Instance().normalMapCobblestoneTemple2);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/aerial_grass_rock_nor_gl_1k.dds", &Frustum::Instance().normalMapAerialGrass);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/snow_02_nor_gl_1k.dds", &Frustum::Instance().normalMapSnow1);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/snow1.dds", &Frustum::Instance().normalMapSnow2);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/snow2.dds", &Frustum::Instance().normalMapSnow3);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/snow3.dds", &Frustum::Instance().normalMapSnow4);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/ice1.dds", &Frustum::Instance().normalMapSnowIce);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/forest_leaves_03_nor_gl_1k.dds", &Frustum::Instance().forest_leaves_03_diff_1k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/dirt_aerial_02_nor_gl_1k.dds", &Frustum::Instance().dirt_aerial_02_diff_1k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/mud_cracked_dry_riverbed_002_nor_gl_2k.dds", &Frustum::Instance().mud_cracked_dry_riverbed_002_diff_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/rocky_trail_02_nor_gl_1k.dds", &Frustum::Instance().rocky_trail_02_diff_1k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/rock_wall_02_nor_gl_2k.dds", &Frustum::Instance().rock_wall_02_diff_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/leafy_grass_nor_gl_1k.dds", &Frustum::Instance().leafy_grass_diff_1k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/mud_forest_nor_gl_2k.dds", &Frustum::Instance().mud_forest_diff_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/grass_map1_n.dds", &Frustum::Instance().grass_map1);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/mossy_rock_nor_gl_1k.dds", &Frustum::Instance().mossy_rock_diff_1k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/brown_mud_leaves_01_nor_gl_1k.dds", &Frustum::Instance().brown_mud_leaves_01_diff_1k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/cobblestone_large_01_nor_gl_1k.dds", &Frustum::Instance().cobblestone_large_01_diff_1k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/cobblestone_05_nor_gl_1k.dds", &Frustum::Instance().cobblestone_05_diff_1k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/playground_sand_nor_gl_1k.dds", &Frustum::Instance().playground_sand_diff_1k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/monastery_stone_floor_nor_gl_2k.dds", &Frustum::Instance().monastery_stone_floor_diff_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/cobblestone_floor_01_nor_gl_2k.dds", &Frustum::Instance().cobblestone_floor_01_diff_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/cobblestone_floor_02_nor_gl_2k.dds", &Frustum::Instance().cobblestone_floor_02_diff_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/concrete_rock_path_nor_gl_2k.dds", &Frustum::Instance().concrete_rock_path_diff_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/dirt_floor_nor_gl_2k.dds", &Frustum::Instance().dirt_floor_diff_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/forest_floor_nor_gl_2k.dds", &Frustum::Instance().forest_floor_diff_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/forrest_ground_03_nor_gl_2k.dds", &Frustum::Instance().forrest_ground_03_diff_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/gray_rocks_nor_gl_2k.dds", &Frustum::Instance().gray_rocks_diff_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/grey_stone_path_nor_gl_2k.dds", &Frustum::Instance().grey_stone_path_diff_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/leaves_forest_ground_nor_gl_2k.dds", &Frustum::Instance().leaves_forest_ground_diff_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/rock_06_nor_gl_2k.dds", &Frustum::Instance().rock_06_diff_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/rock_boulder_cracked_nor_gl_2k.dds", &Frustum::Instance().rock_boulder_cracked_diff_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/rock_face_03_nor_gl_2k.dds", &Frustum::Instance().rock_face_03_diff_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/rustic_stone_wall_nor_gl_2k.dds", &Frustum::Instance().rustic_stone_wall_diff_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/ground_0019_normal_directx_2k.dds", &Frustum::Instance().ground_0019_color_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/ground_0007_normal_directx_2k.dds", &Frustum::Instance().ground_0007_color_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/sand_dunes_n.dds", &Frustum::Instance().sand_dunes);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/sandstone_cracks_nor_gl_2k.dds", &Frustum::Instance().sandstone_cracks_diff_4k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/dry_ground_01_nor_gl_2k.dds", &Frustum::Instance().dry_ground_01_diff_4k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/ice_0002_normal_directx_2k.dds", &Frustum::Instance().ice_0002_color_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/ground_0030_normal_directx_2k.dds", &Frustum::Instance().ground_0030_color_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/ground_0031_normal_directx_2k.dds", &Frustum::Instance().ground_0031_color_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/rock_0008_normal_directx_2k.dds", &Frustum::Instance().rock_0008_color_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/grassy_cobblestone_nor_gl_2k.dds", &Frustum::Instance().grassy_cobblestone_diff_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/aerial_rocks_04_nor_gl_2k.dds", &Frustum::Instance().aerial_rocks_04_diff_1k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/coast_sand_rocks_02_nor_gl_2k.dds", &Frustum::Instance().coast_sand_rocks_02_diff_1k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/brown_mud_03_nor_gl_2k.dds", &Frustum::Instance().brown_mud_03_diff_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/slate_driveway_nor_gl_2k.dds", &Frustum::Instance().slate_driveway_diff_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/cracked_red_ground_nor_gl_2k.dds", &Frustum::Instance().cracked_red_ground_diff_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/concrete_floor_painted_nor_gl_2k.dds", &Frustum::Instance().concrete_floor_painted_diff_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/burned_ground_01_nor_gl_2k.dds", &Frustum::Instance().burned_ground_01_diff_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/brown_mud_dry_nor_gl_2k.dds", &Frustum::Instance().brown_mud_dry_diff_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/brown_mud_nor_gl_2k.dds", &Frustum::Instance().brown_mud_diff_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/brown_mud_02_nor_gl_2k.dds", &Frustum::Instance().brown_mud_02_diff_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/lichen_rock_nor_gl_2k.dds", &Frustum::Instance().lichen_rock_diff_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/lava1_n.dds", &Frustum::Instance().lava1);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/lava2_n.dds", &Frustum::Instance().lava2);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/grass_2k_n.dds", &Frustum::Instance().grass_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/Gravel014_2K-JPG_NormalDX.dds", &Frustum::Instance().Gravel014_2K);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/Ground033_2K-JPG_NormalDX.dds", &Frustum::Instance().Ground033_2K);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/Ground035_2K-JPG_NormalDX.dds", &Frustum::Instance().Ground035_2K);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/Ground044_2K-JPG_NormalDX.dds", &Frustum::Instance().Ground044_2K);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/Ground049B_2K-JPG_NormalDX.dds", &Frustum::Instance().Ground049B_2K);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/Ground050_2K-JPG_NormalDX.dds", &Frustum::Instance().Ground050_2K);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/Ground051_2K-JPG_NormalDX.dds", &Frustum::Instance().Ground051_2K);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/Ground054_2K-JPG_NormalDX.dds", &Frustum::Instance().Ground054_2K);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/Ground059_2K-JPG_NormalDX.dds", &Frustum::Instance().Ground059_2K);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/Ground071_2K-JPG_NormalDX.dds", &Frustum::Instance().Ground071_2K);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/Rock029_2K-JPG_NormalDX.dds", &Frustum::Instance().Rock029_2K);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/ground013_2.dds", &Frustum::Instance().ground013);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/Ground019_2K-JPG_NormalDX.dds", &Frustum::Instance().ground019);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/Ground030_2K-JPG_NormalDX.dds", &Frustum::Instance().ground030);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/Ground037_2K-JPG_NormalDX.dds", &Frustum::Instance().ground037);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/Ground069_2K-JPG_NormalDX.dds", &Frustum::Instance().ground069);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/PavingStones009_2K-JPG_NormalDX.dds", &Frustum::Instance().pavingstones009);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/Rock028_2K-JPG_NormalDX.dds", &Frustum::Instance().rock028);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/Rock051_2K-JPG_NormalDX.dds", &Frustum::Instance().rock051);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/forest_leaves_02_nor_gl_2k.dds", &Frustum::Instance().forest_leaves_02_diffuse_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/forest_leaves_04_nor_gl_2k.dds", &Frustum::Instance().forest_leaves_04_diff_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/forrest_ground_01_nor_gl_2k.dds", &Frustum::Instance().forrest_ground_01_diff_2k);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/Ground018_2K-JPG_NormalDX.dds", &Frustum::Instance().Ground018_2K);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/Ground023_2K-JPG_NormalDX.dds", &Frustum::Instance().Ground023_2K);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/Ground024_2K-JPG_NormalDX.dds", &Frustum::Instance().Ground024_2K);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/Ground038_2K-JPG_NormalDX.dds", &Frustum::Instance().Ground038_2K);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/Ground040_2K-JPG_NormalDX.dds", &Frustum::Instance().Ground040_2K);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/Ground042_2K-JPG_NormalDX.dds", &Frustum::Instance().Ground042_2K);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/Ground067_2K-JPG_NormalDX.dds", &Frustum::Instance().Ground067_2K);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/Ground068_2K-JPG_NormalDX.dds", &Frustum::Instance().Ground068_2K);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/empty.dds", &Frustum::Instance().emptyNormalMap);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/normalmaps/sand_02_nor_gl_2k.dds", &Frustum::Instance().sand_02_diff_2k);

		//V1.0.4b Specular Texture
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/specular/ground_0007_roughness_2k.dds", &Frustum::Instance().waterSandSpecularTexture);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/specular/ground_0019_color_2k.dds", &Frustum::Instance().waterLeavesSpecularTexture);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/specular/ground_0030_color_specular.dds", &Frustum::Instance().iceSpecularTexture);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/specular/Ground050_2K-JPG_Roughness.dds", &Frustum::Instance().sandSpecularTexture);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/cobblestone_05_disp_2k2.jpg", &Frustum::Instance().displacementTexture);

		//V1.0.5 itemshop & tree bark
		D3DXCreateTextureFromFile(ms_lpd3dDevice, "a1-007-itemshop-02.dds", &Frustum::Instance().c1_itemshop_normal);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/tree/pine_bark_nor_gl_2k.dds", &Frustum::Instance().treeBarkNormal);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/tree/pine_bark_rough_4k.jpg", &Frustum::Instance().treeBarkSpecular);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/temp/tree/treewillowoakbarkmoss.dds", &Frustum::Instance().treeBarkTexture);

		//V1.0.6 grass
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/grass/grass_map1_1.dds", &Frustum::Instance().grass_map1_1);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/grass/grass_map1_2.dds", &Frustum::Instance().grass_map1_2);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/grass/grass_map1_3.dds", &Frustum::Instance().grass_map1_3);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/grass/grass_map1_4.dds", &Frustum::Instance().grass_map1_4);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/grass/grains1.dds", &Frustum::Instance().grass_grain_1);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/grass/grains2.dds", &Frustum::Instance().grass_grain_2);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/grass/flower1.dds", &Frustum::Instance().grass_flower_1);
		LoadDepthTerrainTexture("D:/ymir work/terrainmaps/grass/flower2.dds", &Frustum::Instance().grass_flower_2);

		loadTerrainTextureMode4 = false;
	}
#else
	loadTerrainTextureMode4 = false;
#endif

	// Water and Puddle Reflection Render Targets (Active for BOTH Mode 3 and Mode 4)
	if (!Frustum::Instance().m_lpWaterReflectionTexture)
	{
#ifdef __ANDROID__
		m_waterReflectionTexSize = 2048;
#else
		m_waterReflectionTexSize = 1024;
#endif
		if (FAILED(ms_lpd3dDevice->CreateTexture(m_waterReflectionTexSize, m_waterReflectionTexSize, 1, D3DUSAGE_RENDERTARGET,
			D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &Frustum::Instance().m_lpWaterReflectionTexture, NULL)))
		{
			TraceError("CMapOutdoor Unable to create Water Reflection render target texture\n");
			return;
		}
	}

	if (!m_lpWaterReflectionDepthSurface)
	{
		if (FAILED(ms_lpd3dDevice->CreateDepthStencilSurface(m_waterReflectionTexSize, m_waterReflectionTexSize, D3DFMT_D16,
			D3DMULTISAMPLE_NONE, 0, true, &m_lpWaterReflectionDepthSurface, NULL)))
		{
			TraceError("CMapOutdoor Unable to create Water Reflection depth surface\n");
			return;
		}
	}

	if (!Frustum::Instance().m_lpPuddleReflectionTexture)
	{
#ifdef __ANDROID__
		m_waterPuddleReflectionTexSize = 2048;
#else
		m_waterPuddleReflectionTexSize = 1024;
#endif

		if (FAILED(ms_lpd3dDevice->CreateTexture(m_waterPuddleReflectionTexSize, m_waterPuddleReflectionTexSize, 1, D3DUSAGE_RENDERTARGET,
			D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &Frustum::Instance().m_lpPuddleReflectionTexture, NULL)))
		{
			TraceError("CMapOutdoor Unable to create Puddle Reflection render target texture\n");
			return;
		}
	}

	if (!m_lpPuddleReflectionDepthSurface)
	{
		if (FAILED(ms_lpd3dDevice->CreateDepthStencilSurface(m_waterPuddleReflectionTexSize, m_waterPuddleReflectionTexSize, D3DFMT_D16,
			D3DMULTISAMPLE_NONE, 0, true, &m_lpPuddleReflectionDepthSurface, NULL)))
		{
			TraceError("CMapOutdoor Unable to create Water Reflection depth surface\n");
			return;
		}
	}
}

void CMapOutdoor::RenderGrass()
{
	// Dedicated to Graphics Mode 4 only
	if (Frustum::Instance().graphicsLevel < 4)
		return;

	D3DXVECTOR3 grassPosition = Frustum::Instance().characterPosition;
	D3DXMATRIX matShader;

	if (ms_lpd3dDevice == NULL) return;

	static const float radius = 8000.0f;
	static const float cullingDistanceSquared = radius * radius;
	static const size_t maxGrassInstances = 35000;
	static const size_t maxVisibleGrass = 6000;

	if (!Frustum::Instance().grassTexturesInitialized) {

		float width = 80.0f;
		float height = 160.0f;
		GrassVertex grassVertices[8] = {
			{ -width, 0.0f, 0.0f, 0.0f, 1.0f },
			{  width, 0.0f, 0.0f, 1.0f, 1.0f },
			{ -width, 0.0f, height, 0.0f, 0.0f },
			{  width, 0.0f, height, 1.0f, 0.0f },
			{ 0.0f, -width, 0.0f, 0.0f, 1.0f },
			{ 0.0f,  width, 0.0f, 1.0f, 1.0f },
			{ 0.0f, -width, height, 0.0f, 0.0f },
			{ 0.0f,  width, height, 1.0f, 0.0f }
		};

		HRESULT hr = ms_lpd3dDevice->CreateVertexBuffer(8 * sizeof(GrassVertex), 0, GrassVertex::FVF, D3DPOOL_MANAGED, &m_pGrassVertexBuffer, NULL);
		if (FAILED(hr) || m_pGrassVertexBuffer == NULL) return;

		void* pVertices;
		hr = m_pGrassVertexBuffer->Lock(0, 8 * sizeof(GrassVertex), (void**)&pVertices, 0);
		if (SUCCEEDED(hr)) {
			memcpy(pVertices, grassVertices, 8 * sizeof(GrassVertex));
			m_pGrassVertexBuffer->Unlock();
		}
		else {
			m_pGrassVertexBuffer->Release();
			return;
		}

		WORD grassIndices[12] = {
			0, 1, 2,
			2, 1, 3,
			4, 5, 6,
			6, 5, 7
		};

		hr = ms_lpd3dDevice->CreateIndexBuffer(12 * sizeof(WORD), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pGrassIndexBuffer, NULL);
		if (FAILED(hr) || m_pGrassIndexBuffer == NULL) {
			m_pGrassVertexBuffer->Release();
			return;
		}

		void* pIndices;
		hr = m_pGrassIndexBuffer->Lock(0, 12 * sizeof(WORD), (void**)&pIndices, 0);
		if (SUCCEEDED(hr)) {
			memcpy(pIndices, grassIndices, 12 * sizeof(WORD));
			m_pGrassIndexBuffer->Unlock();
		}
		else {
			m_pGrassVertexBuffer->Release();
			m_pGrassIndexBuffer->Release();
			return;
		}

		Frustum::Instance().grassTexturesInitialized = true;
	}

	D3DXVECTOR3 vGrassMoveDiff = grassPosition - Frustum::Instance().lastGrassPosition;
	float distanceMovedSquared = D3DXVec3LengthSq(&vGrassMoveDiff);
	bool isInitialSpawn = Frustum::Instance().grassMatrices.empty();
	bool moveTriggeredSpawn = (!isInitialSpawn && distanceMovedSquared > radius * radius / 4.0f);
	bool needToSpawnGrass = isInitialSpawn || moveTriggeredSpawn;

	if (needToSpawnGrass) {
		Frustum::Instance().lastGrassPosition = grassPosition;
		size_t spawnCount = maxVisibleGrass * 5;
		
		for (size_t i = 0; i < spawnCount; ++i) {
			float angle = (float(rand()) / RAND_MAX) * 2.0f * 3.141245f;
			float distance = radius * sqrtf(float(rand()) / RAND_MAX);

			if (moveTriggeredSpawn) {
				distance = radius + (float(rand()) / RAND_MAX * radius);
			}

			float x = distance * cosf(angle);
			float z = distance * sinf(angle);

			float actualX = grassPosition.x + x;
			float actualZ = grassPosition.y + z;

			float randomValue = (rand() % 20 == 0) ? 0.6f + float(rand()) / RAND_MAX * 0.6f : 1.0f;

			if (isAttrOn(actualX + 1.73648f, -actualZ - 9.84807f, CTerrainImpl::ATTRIBUTE_GRASS)) {
				D3DXMATRIX matTranslation;
				D3DXMatrixIdentity(&matTranslation);
				float fGroundHeight = GetTerrainHeight(actualX, actualZ);
				D3DXMatrixTranslation(&matTranslation, actualX, actualZ, fGroundHeight - 90.0f * randomValue);
				Frustum::Instance().grassMatrices.push_back(matTranslation);

				if (Frustum::Instance().grassMatrices.size() > maxGrassInstances) {
					Frustum::Instance().grassMatrices.pop_front();
				}
			}
		}
	}

	STATEMANAGER.SetVertexShader(m_lpVertexShaderGrass);
	STATEMANAGER.SetPixelShader(m_lpPixelShaderGrass);

	if (m_lpCharacterShadowMapTexture)
		STATEMANAGER.SetTexture(1, m_lpCharacterShadowMapTexture);
	if (m_lpCharacterShadowMapTexture4)
		STATEMANAGER.SetTexture(2, m_lpCharacterShadowMapTexture4);

	D3DXMATRIX worldMat;
	ms_lpd3dDevice->GetTransform(D3DTS_WORLD, &worldMat);
	D3DXMatrixTranspose(&worldMat, &worldMat);
	ms_lpd3dDevice->SetVertexShaderConstantF(0, (const float*)(&worldMat), 4);

	ms_lpd3dDevice->SetVertexShaderConstantF(49, grassPosition, 1);
	ms_lpd3dDevice->SetPixelShaderConstantF(101, grassPosition, 1);

	D3DXMATRIX viewProjMat, viewMat, projMat;
	ms_lpd3dDevice->GetTransform(D3DTS_VIEW, &viewMat);
	ms_lpd3dDevice->GetTransform(D3DTS_PROJECTION, &projMat);
	D3DXMatrixMultiplyTranspose(&viewProjMat, &viewMat, &projMat);
	ms_lpd3dDevice->SetVertexShaderConstantF(4, (const float*)(&viewProjMat), 4);
	STATEMANAGER.SetPixelShaderConstant(1, &current_v3SunPosition, 1);
	ms_lpd3dDevice->SetVertexShaderConstantF(8, (const float*)(&m_MatLightViewProj), 4);
	STATEMANAGER.SetVertexShaderConstant(16, (const float*)(&m_MatLightViewProj2), 4);

	D3DXVECTOR3 windDirection = { 1.0f, 0.5f, -0.5f };
	float windStrength = 15.0f;
	float time = GetTickCount() * 0.001f;
	ms_lpd3dDevice->SetVertexShaderConstantF(46, (float*)&windDirection, 1);
	ms_lpd3dDevice->SetVertexShaderConstantF(47, &windStrength, 1);
	ms_lpd3dDevice->SetVertexShaderConstantF(48, &time, 1);

	D3DXVECTOR3 cameraPos = CCameraManager::instance().GetCurrentCamera()->GetEye();
	STATEMANAGER.SetPixelShaderConstant(4, &cameraPos, 1);

	DWORD fogColor;
	STATEMANAGER.GetRenderState(D3DRS_FOGCOLOR, &fogColor);
	D3DXCOLOR fogColorX(fogColor);
	STATEMANAGER.SetPixelShaderConstant(5, (float*)&fogColorX, 1);

	float fogStart, fogEnd;
	STATEMANAGER.GetRenderState(D3DRS_FOGSTART, (DWORD*)&fogStart);
	STATEMANAGER.GetRenderState(D3DRS_FOGEND, (DWORD*)&fogEnd);
	D3DXVECTOR2 fogRange(fogStart, fogEnd);
	STATEMANAGER.SetPixelShaderConstant(6, (float*)&fogRange, 1);

	STATEMANAGER.SetPixelShaderConstant(86, &grassPosition, 1);

	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	ms_lpd3dDevice->SetStreamSource(0, m_pGrassVertexBuffer, 0, sizeof(GrassVertex));
	ms_lpd3dDevice->SetFVF(GrassVertex::FVF);
	ms_lpd3dDevice->SetIndices(m_pGrassIndexBuffer);

	static std::vector<size_t> visibleGrassIndices;
	visibleGrassIndices.clear();
	if (visibleGrassIndices.capacity() < maxVisibleGrass)
		visibleGrassIndices.reserve(maxVisibleGrass);

	const Frustum& rkFrustum = Frustum::Instance();
	const auto& rkGrassMatrices = rkFrustum.grassMatrices;
	const size_t grassCount = rkGrassMatrices.size();

	for (size_t i = 0; i < grassCount; ++i) {
		const D3DXMATRIX& matrix = rkGrassMatrices[i];
		D3DXVECTOR3 grassInstancePos(matrix._41, matrix._42, matrix._43);

		D3DXVECTOR3 vGrassCullDiff = grassInstancePos - grassPosition;
		if (D3DXVec3LengthSq(&vGrassCullDiff) <= cullingDistanceSquared) {
			Vector3d grassInstancePos3d(grassInstancePos.x, grassInstancePos.y, grassInstancePos.z);
			if (rkFrustum.ViewVolumeTestEffects(grassInstancePos3d, 100.0f) != VS_OUTSIDE) {
				visibleGrassIndices.push_back(i);
				if (visibleGrassIndices.size() >= maxVisibleGrass) break;
			}
		}
	}

	LPDIRECT3DTEXTURE9 lpLastBoundGrassTex = nullptr;
	const int grassType = rkFrustum.grassType;

	for (size_t i : visibleGrassIndices) {
		if (i >= grassCount) continue;

		LPDIRECT3DTEXTURE9 lpTargetTex = nullptr;
		if (grassType == 0)
		{
			if (i % 50 == 0)
				lpTargetTex = rkFrustum.grass_flower_1;
			else if (i % 5 == 0)
				lpTargetTex = rkFrustum.grass_map1_2;
			else
				lpTargetTex = rkFrustum.grass_map1_1;
		}
		else if (grassType == 1)
		{
			lpTargetTex = (i % 10 == 0) ? rkFrustum.grass_map1_3 : rkFrustum.grass_map1_4;
		}
		else if (grassType == 2)
		{
			if (i % 200 == 0)
				lpTargetTex = rkFrustum.grass_flower_1;
			else if (i % 10 == 0)
				lpTargetTex = rkFrustum.grass_grain_2;
			else
				lpTargetTex = rkFrustum.grass_grain_1;
		}
		else if (grassType == 3)
		{
			if (i % 50 == 0)
				lpTargetTex = rkFrustum.grass_flower_1;
			else if (i % 25 == 0)
				lpTargetTex = rkFrustum.grass_flower_2;
			else if (i % 10 == 0)
				lpTargetTex = rkFrustum.grass_grain_2;
			else
				lpTargetTex = rkFrustum.grass_map1_2;
		}

		// Only call SetTexture when the texture pointer actually changes
		if (lpTargetTex != lpLastBoundGrassTex)
		{
			ms_lpd3dDevice->SetTexture(0, lpTargetTex);
			lpLastBoundGrassTex = lpTargetTex;
		}

		const D3DXMATRIX& matrix = rkGrassMatrices[i];
		D3DXMatrixTranspose(&matShader, &matrix);
		STATEMANAGER.SetVertexShaderConstant(20, &matShader, 4);
		ms_lpd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 8, 0, 4);
	}

	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);

	STATEMANAGER.SetVertexShader(nullptr);
	STATEMANAGER.SetPixelShader(nullptr);
}

void CMapOutdoor::RenderGrassShadow()
{
	// Dedicated to Graphics Mode 4 only
	if (Frustum::Instance().graphicsLevel < 4)
		return;

	D3DXVECTOR3 grassPosition = Frustum::Instance().characterPosition;
	D3DXMATRIX matShader;

	if (ms_lpd3dDevice == NULL) return;

	static float radius = 1000.0f;

	if (!Frustum::Instance().grassTexturesInitialized) return;

	D3DXVECTOR3 vGrassShadowMoveDiff = grassPosition - Frustum::Instance().lastGrassShadowPosition;
	float distanceMovedSquared = D3DXVec3LengthSq(&vGrassShadowMoveDiff);
	float thresholdDistance = 1000.0f * 1000.0f;

	if (distanceMovedSquared > thresholdDistance) {
		Frustum::Instance().lastGrassShadowPosition = grassPosition;

		int numGrassInstances = 500;

		for (int i = 0; i < numGrassInstances; ++i) {
			float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.141245f;
			float distance = radius + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 1000);

			float x = distance * cosf(angle);
			float z = distance * sinf(angle);

			float actualX = grassPosition.x + x;
			float actualZ = grassPosition.y + z;
			float randomValue = (rand() % 20 == 0)
				? 0.6f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (1.0f - 0.6f)
				: 1.0f;

			if (isAttrOn(actualX + 1.73648f, -actualZ - 9.84807f, CTerrainImpl::ATTRIBUTE_GRASS)) {
				D3DXMATRIX matTranslation;
				D3DXMatrixIdentity(&matTranslation);
				float fGroundHeight = GetTerrainHeight(actualX, actualZ);
				D3DXMatrixTranslation(&matTranslation, actualX, actualZ, fGroundHeight - 55.0f * randomValue);
				Frustum::Instance().grassShadowMatrices.push_back(matTranslation);
			}
		}
	}

	STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	STATEMANAGER.SaveRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHAREF, 0x08);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
	STATEMANAGER.SaveRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	ms_lpd3dDevice->SetStreamSource(0, m_pGrassVertexBuffer, 0, sizeof(GrassVertex));
	ms_lpd3dDevice->SetFVF(GrassVertex::FVF);
	ms_lpd3dDevice->SetIndices(m_pGrassIndexBuffer);

	const Frustum& frustum = Frustum::Instance();

	float cullingDistanceSquared = radius * radius;

	std::vector<size_t> visibleGrassIndices;

	for (size_t i = 0; i < Frustum::Instance().grassShadowMatrices.size(); ++i) {
		D3DXVECTOR3 grassInstancePos(Frustum::Instance().grassShadowMatrices[i]._41, Frustum::Instance().grassShadowMatrices[i]._42, Frustum::Instance().grassShadowMatrices[i]._43);
		D3DXVECTOR3 vGrassShadowCullDiff = grassInstancePos - grassPosition;
		float distanceSquared = D3DXVec3LengthSq(&vGrassShadowCullDiff);

		if (distanceSquared <= cullingDistanceSquared) {
			visibleGrassIndices.push_back(i);
		}
	}

	const size_t maxDequeSize = 50000;
	const size_t elementsToRemove = 5000;

	if (Frustum::Instance().grassShadowMatrices.size() >= maxDequeSize) {
		Frustum::Instance().grassShadowMatrices.erase(Frustum::Instance().grassShadowMatrices.begin(), Frustum::Instance().grassShadowMatrices.begin() + elementsToRemove);
	}

	for (size_t i : visibleGrassIndices) {
		if (i >= Frustum::Instance().grassShadowMatrices.size()) {
			continue;
		}

		D3DXMATRIX& matrix = Frustum::Instance().grassShadowMatrices[i];
		if (!matrix) {
			continue;
		}

		D3DXVECTOR3 grassInstancePos(matrix._41, matrix._42, matrix._43);
		Vector3d grassInstancePos3d(grassInstancePos.x, grassInstancePos.y, grassInstancePos.z);
		ViewState viewState = frustum.ViewVolumeTestEffects(grassInstancePos3d, 100.0f);

		if (viewState == VS_OUTSIDE) {
			continue;
		}

		if (Frustum::Instance().grassType == 0)
		{
			if (i % 5 == 0)
				ms_lpd3dDevice->SetTexture(0, Frustum::Instance().grass_map1_2);
			else
				ms_lpd3dDevice->SetTexture(0, Frustum::Instance().grass_map1_1);
		}

		if (Frustum::Instance().grassType == 1)
		{
			if (i % 5 == 0)
				ms_lpd3dDevice->SetTexture(0, Frustum::Instance().grass_map1_3);
			else
				ms_lpd3dDevice->SetTexture(0, Frustum::Instance().grass_map1_4);
		}

		if (Frustum::Instance().grassType == 2)
		{
			if (i % 5 == 0)
				ms_lpd3dDevice->SetTexture(0, Frustum::Instance().grass_grain_2);
			else
				ms_lpd3dDevice->SetTexture(0, Frustum::Instance().grass_grain_1);
		}

		if (Frustum::Instance().grassType == 3)
		{
			if (i % 5 == 0)
				ms_lpd3dDevice->SetTexture(0, Frustum::Instance().grass_flower_1);
			else
				ms_lpd3dDevice->SetTexture(0, Frustum::Instance().grass_flower_2);
		}

		D3DXMATRIX matTranslate;
		D3DXMatrixTranslation(&matTranslate, Frustum::Instance().grassShadowMatrices[i]._41, Frustum::Instance().grassShadowMatrices[i]._42, Frustum::Instance().grassShadowMatrices[i]._43);
		ms_lpd3dDevice->SetTransform(D3DTS_WORLD, &matTranslate);
		ms_lpd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 8, 0, 4);
	}

	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_SRCBLEND);
	STATEMANAGER.RestoreRenderState(D3DRS_DESTBLEND);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAREF);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);
	STATEMANAGER.RestoreRenderState(D3DRS_CULLMODE);
}

void CMapOutdoor::ReleaseCharacterShadowTexture()
{
	SAFE_RELEASE(m_lpCharacterShadowMapRenderTargetSurface);
	SAFE_RELEASE(m_lpCharacterShadowMapRenderTargetSurface2);
	SAFE_RELEASE(m_lpCharacterShadowMapRenderTargetSurface4);
	SAFE_RELEASE(m_lpCharacterShadowMapTexture);
	SAFE_RELEASE(m_lpCharacterShadowMapTexture2);
	SAFE_RELEASE(m_lpCharacterShadowMapTexture4);
	SAFE_RELEASE(m_lpCharacterShadowMapDepthSurface);
	SAFE_RELEASE(Frustum::Instance().m_lpWaterReflectionTexture);
	SAFE_RELEASE(Frustum::Instance().m_lpPuddleReflectionTexture);
	SAFE_RELEASE(Frustum::Instance().TextureReflectionMap);
	SAFE_RELEASE(m_lpWaterReflectionDepthSurface);
	SAFE_RELEASE(m_lpPuddleReflectionDepthSurface);
	STATEMANAGER.ReleaseRenderTargets();
	SAFE_RELEASE(m_waterOcclusionQuery);
}

void CMapOutdoor::ReleaseCharacterShadowTextureOriginal()
{
	SAFE_RELEASE(m_lpCharacterShadowMapRenderTargetSurfaceOriginal);
	SAFE_RELEASE(m_lpCharacterShadowMapTextureOriginal);
	SAFE_RELEASE(m_lpCharacterShadowMapDepthSurfaceOriginal);
}

void CMapOutdoor::ReleaseReflectionTexture()
{
	STATEMANAGER.ReleaseRenderTargets();
	SAFE_RELEASE(m_waterOcclusionQuery);
	SAFE_RELEASE(Frustum::Instance().m_lpWaterReflectionTexture);
	SAFE_RELEASE(m_lpWaterReflectionDepthSurface);
}

DWORD dwLightEnable = FALSE;

void CMapOutdoor::SkyUtil()
{
	static int s_iLastShaderMode = -1;
	const int iCurShaderMode = (Frustum::Instance().graphicsLevel >= 4) ? 4 : (Frustum::Instance().graphicsLevel > 0 ? 3 : 0);
	if (s_iLastShaderMode != -1 && s_iLastShaderMode != iCurShaderMode)
	{
		ReleaseShaders();
		ReleaseTreeShaders();
	}
	s_iLastShaderMode = iCurShaderMode;

	if (Frustum::Instance().graphicsLevel == 0)
	{
		ReleaseShaders();
		ReleaseCharacterShadowTexture();
		if (!m_lpCharacterShadowMapTextureOriginal)
			CreateCharacterShadowTextureOriginal();
	}
	else
	{
		ReleaseCharacterShadowTextureOriginal();
	}

	if (Frustum::Instance().graphicsWaterLevel == 0 || Frustum::Instance().graphicsLevel == 0)
		ReleaseWaterShaders();

	if (Frustum::Instance().graphicsLevel == 0)
		ReleaseTreeShaders();

	if (Frustum::Instance().graphicsLevel > 0 && shadersReleased == true)
		CreateShadowmapShaders();

	if (Frustum::Instance().graphicsLevel > 0 && shadersWaterReleased == true && Frustum::Instance().graphicsWaterLevel > 0)
		CreateWaterShaders();

	if (Frustum::Instance().graphicsLevel > 0 && shadersTreeReleased == true)
		CreateTreeShaders();

	// Ensure Mode 4 textures load on demand if switching from Mode 3 -> Mode 4 at runtime
	if (Frustum::Instance().graphicsLevel >= 4 && loadTerrainTextureMode4)
	{
		recreate = true;
	}

	if ((recreate || !m_lpCharacterShadowMapTexture) && Frustum::Instance().graphicsLevel != 0)
	{
		CreateCharacterShadowTexture();
		recreate = false;
	}

	const TEnvironmentData* envData = 0;
	CFlyingManager::Instance().GetMapManagerPtr()->GetCurrentEnvironmentData(&envData);
	if (!envData)
		return;
	D3DXVECTOR3 v3SunPosition = envData->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction;
	v3SunPosition.z = v3SunPosition.z * 1.5f;

	Frustum::Instance().currentEnvironmentFace = envData->strSkyBoxFaceFileName[0].c_str();

	Frustum::Instance().waterSunDirection = { 0.0, 0.0, 0.0 };
	Frustum::Instance().characterSpecularLightPower = 0.8f;
	Frustum::Instance().isNightEnvironment = 0.0f;
	Frustum::Instance().isStormEnvironment = 0.0f;
	Frustum::Instance().AmbientLight = { 0.4f, 0.4f, 0.4f };
	isNoapteEnvironment = false;
	Frustum::Instance().treeNormalMap = 2.0f;
	Frustum::Instance().treeFinalColor = 65.0f;

	if (Frustum::Instance().currentEnvironmentFace.find("environment01_front") != std::string::npos)
	{
		Frustum::Instance().terrainLightPowerEnvBased = 7.0f;
		Frustum::Instance().isNightEnvironment = 1.0f;
		Frustum::Instance().waterOpacity = 0.7f;
		isNoapteEnvironment = true;
	}
	else if (Frustum::Instance().currentEnvironmentFace.find("environment02_front") != std::string::npos)
	{
		Frustum::Instance().terrainLightPowerEnvBased = 5.0f;
		Frustum::Instance().characterSpecularLightPower = 1.4f;
		Frustum::Instance().waterOpacity = 1.0f;
	}
	else if (Frustum::Instance().currentEnvironmentFace.find("environment03_front") != std::string::npos)
	{
		Frustum::Instance().terrainLightPowerEnvBased = 4.5f;
		Frustum::Instance().characterSpecularLightPower = 1.0f;
		Frustum::Instance().waterOpacity = 1.0f;
	}
	else if (Frustum::Instance().currentEnvironmentFace.find("environment04_front") != std::string::npos)
	{
		Frustum::Instance().terrainLightPowerEnvBased = 7.0f;
		Frustum::Instance().waterOpacity = 0.6f;
	}
	else if (Frustum::Instance().currentEnvironmentFace.find("environment05_front") != std::string::npos)
	{
		Frustum::Instance().terrainLightPowerEnvBased = 4.0f;
		Frustum::Instance().characterSpecularLightPower = 1.1f;
		Frustum::Instance().waterOpacity = 1.0f;
		Frustum::Instance().AmbientLight = { 0.03f, 0.03f, 0.03f };
	}
	else if (Frustum::Instance().currentEnvironmentFace.find("environment06_front") != std::string::npos)
	{
		Frustum::Instance().terrainLightPowerEnvBased = 8.0f;
		Frustum::Instance().characterSpecularLightPower = 1.6f;
		Frustum::Instance().waterOpacity = 1.0f;
		Frustum::Instance().AmbientLight = { 0.8f, 0.8f, 0.8f };
	}
	else if (Frustum::Instance().currentEnvironmentFace.find("environment07_front") != std::string::npos)
	{
		Frustum::Instance().terrainLightPowerEnvBased = 7.0f;
		Frustum::Instance().characterSpecularLightPower = 1.1f;
		Frustum::Instance().waterOpacity = 1.0f;
		Frustum::Instance().AmbientLight = { 0.03f, 0.03f, 0.03f };
	}
	else if (Frustum::Instance().currentEnvironmentFace.find("environment08_front") != std::string::npos)
	{
		Frustum::Instance().terrainLightPowerEnvBased = 4.0f;
		Frustum::Instance().characterSpecularLightPower = 1.1f;
		Frustum::Instance().waterOpacity = 0.8f;
		Frustum::Instance().treeNormalMap = 2.0f;
		Frustum::Instance().treeFinalColor = 25.0f;
	}
	else if (Frustum::Instance().currentEnvironmentFace.find("environment09_front") != std::string::npos)
	{
		Frustum::Instance().terrainLightPowerEnvBased = 11.5f;
		Frustum::Instance().characterSpecularLightPower = 1.1f;
		Frustum::Instance().waterOpacity = 0.7f;
		Frustum::Instance().AmbientLight = { 0.8f, 0.8f, 0.8f };
	}
	else if (Frustum::Instance().currentEnvironmentFace.find("environment10_front") != std::string::npos)
	{
		Frustum::Instance().terrainLightPowerEnvBased = 7.5f;
		Frustum::Instance().characterSpecularLightPower = 1.45f;
		Frustum::Instance().waterOpacity = 1.5f;
		Frustum::Instance().AmbientLight = { 0.8f, 0.8f, 0.8f };
	}
	else if (Frustum::Instance().currentEnvironmentFace.find("environment11_front") != std::string::npos)
	{
		Frustum::Instance().terrainLightPowerEnvBased = 7.5f;
		Frustum::Instance().waterSunDirection = { -0.020000, -0.967921, -0.215650 };
		Frustum::Instance().characterSpecularLightPower = 2.1f;
		Frustum::Instance().waterOpacity = 1.5f;
		Frustum::Instance().AmbientLight = { 0.03f, 0.03f, 0.03f };
		Frustum::Instance().treeNormalMap = 0.0f;
		Frustum::Instance().treeFinalColor = 155.0f;
	}
	else if (Frustum::Instance().currentEnvironmentFace.find("rain") != std::string::npos)
	{
		Frustum::Instance().terrainLightPowerEnvBased = 7.0f;
		Frustum::Instance().characterSpecularLightPower = 2.0f;
		Frustum::Instance().isNightEnvironment = 1.0f;
		Frustum::Instance().waterOpacity = 1.0f;
		isNoapteEnvironment = true;
	}
	else if (Frustum::Instance().currentEnvironmentFace.find("storm") != std::string::npos)
	{
		Frustum::Instance().terrainLightPowerEnvBased = 10.0f;
		Frustum::Instance().characterSpecularLightPower = 2.0f;
		Frustum::Instance().isNightEnvironment = 1.0f;
		Frustum::Instance().isStormEnvironment = 1.0f;
		Frustum::Instance().waterOpacity = 1.0f;
		isNoapteEnvironment = true;
	}
	else
	{
		Frustum::Instance().terrainLightPowerEnvBased = 6.0f;
		Frustum::Instance().characterSpecularLightPower = 1.0f;
		Frustum::Instance().isNightEnvironment = 0.0f;
		Frustum::Instance().isStormEnvironment = 1.0f;
		Frustum::Instance().waterOpacity = 1.0f;
	}

#ifndef JTX_WATER_REFLECTION
	Frustum::Instance().waterOpacity = 0.7f;
#endif

	SanitizeSunVector(current_v3SunPosition, &v3SunPosition);
	Frustum::Instance().treeLightDirection = current_v3SunPosition;
}

int shadowstep = 10;
bool CMapOutdoor::BeginRenderCharacterShadowToTexture()
{
	time_t curr_time;
	curr_time = time(NULL);
	tm* tm_local = localtime(&curr_time);

	if (tm_local->tm_sec == 9 || tm_local->tm_sec == 29 || tm_local->tm_sec == 49)
	{
		shadowstep = 0;
	}

	D3DXMATRIX matLightView, matLightProj;

	CCamera* pCurrentCamera = CCameraManager::Instance().GetCurrentCamera();

	if (!pCurrentCamera)
		return false;

	if (recreate)
	{
		CreateCharacterShadowTexture();
		recreate = false;
	}

	const TEnvironmentData* envData = 0;
	CFlyingManager::Instance().GetMapManagerPtr()->GetCurrentEnvironmentData(&envData);
	if (!envData)
		return false;
	D3DXVECTOR3 v3SunPosition = envData->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction;
	v3SunPosition.z = v3SunPosition.z * 1.5f;

	if (std::strcmp(Frustum::Instance().currentMapNameSave.c_str(), "metin2_map_skipia_dungeon_02") != 0
		&& std::strcmp(Frustum::Instance().currentMapNameSave.c_str(), "metin2_map_skipia_dungeon_boss") != 0)
	{
		if ((Frustum::Instance().selectRainFrustun == 16 || Frustum::Instance().selectRainFrustun == 25 || Frustum::Instance().selectRainFrustun == 8) && std::strcmp(Frustum::Instance().currentMapNameSave.c_str(), "map_n_snowm_01") != 0)
		{
			if (shadowstep >= 0 && shadowstep <= 2 || shadowstep == 8)
			{
				if (tm_local->tm_sec == 10 || tm_local->tm_sec == 30 || tm_local->tm_sec == 50)
				{
					shadowstep += 1;

					const float random_num = 0.15f + (float(rand()) / float(RAND_MAX)) * 0.05f;

					if (tm_local->tm_sec == 10)
					{
						current_v3SunPosition.z = current_v3SunPosition.z + random_num;
						current_v3SunPosition.x = current_v3SunPosition.x + random_num;
						current_v3SunPosition.y = current_v3SunPosition.y + random_num * 0.1108f;
					}
					if (tm_local->tm_sec == 30)
					{
						current_v3SunPosition.z = current_v3SunPosition.z - random_num;
						current_v3SunPosition.x = current_v3SunPosition.x - random_num;
						current_v3SunPosition.y = current_v3SunPosition.y - random_num * 0.1108f;
					}
					if (tm_local->tm_sec == 50)
					{
						current_v3SunPosition.z = current_v3SunPosition.z + random_num;
						current_v3SunPosition.x = current_v3SunPosition.x - random_num;
						current_v3SunPosition.y = current_v3SunPosition.y + random_num * 0.1108f;
					}
				}
			}
			if (tm_local->tm_sec == 12 || tm_local->tm_sec == 32 || tm_local->tm_sec == 52)
			{
				z = 0.015f;
			}
		}
	}

	D3DXVECTOR3 v3Target = pCurrentCamera->GetTarget();
	if (a)
	{
		current_v3SunPosition = v3SunPosition;
		a = false;
	}
	SanitizeSunVector(current_v3SunPosition, &v3SunPosition);

	const auto vv = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	shadow_eye = (v3Target)-current_v3SunPosition * (200000.0f);
	D3DXMatrixLookAtRH(&matLightView,
		&shadow_eye,
		&v3Target,
		&vv);

	const float fShadowSpaceSize = (Frustum::Instance().graphicsLevel >= 4) ? 14000.0f : 10000.0f;
	SnapShadowSpaceMatrix(matLightView, fShadowSpaceSize);
	D3DXMatrixOrthoRH(&matLightProj, fShadowSpaceSize, fShadowSpaceSize, 1.0f, 300000.0f);

	STATEMANAGER.SaveTransform(D3DTS_VIEW, &matLightView);
	STATEMANAGER.SaveTransform(D3DTS_PROJECTION, &matLightProj);
	CSpeedTreeForestDirectX8::Instance().UpdateCompundMatrix(((v3Target)-current_v3SunPosition * (200000.0f)), matLightView, matLightProj);

	dwLightEnable = STATEMANAGER.GetRenderState(D3DRS_LIGHTING);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

	STATEMANAGER.SaveRenderState(D3DRS_TEXTUREFACTOR, 0xFF808080);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);

	bool bSuccess = true;

#ifdef DIRECTX9
	if (FAILED(ms_lpd3dDevice->GetRenderTarget(0, &m_lpBackupRenderTargetSurface)))
#else
	if (FAILED(ms_lpd3dDevice->GetRenderTarget(&m_lpBackupRenderTargetSurface)))
#endif
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Save Window Render Target\n");
		bSuccess = false;
	}

	if (FAILED(ms_lpd3dDevice->GetDepthStencilSurface(&m_lpBackupDepthSurface)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Save Window Depth Surface\n");
		bSuccess = false;
	}

#ifdef DIRECTX9
	ms_lpd3dDevice->SetDepthStencilSurface(m_lpCharacterShadowMapDepthSurface);
	ms_lpd3dDevice->SetRenderTarget(0, m_lpCharacterShadowMapRenderTargetSurface);
#else
	if (FAILED(ms_lpd3dDevice->SetRenderTarget(m_lpCharacterShadowMapRenderTargetSurface, m_lpCharacterShadowMapDepthSurface)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Set Shadow Map Render Target\n");
		bSuccess = false;
	}
#endif

	if (FAILED(ms_lpd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0xFF, 0xFF, 0xFF), 1.0f, 0)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Clear Render Target");
		bSuccess = false;
	}

	if (FAILED(ms_lpd3dDevice->GetViewport(&m_BackupViewport)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Save Window Viewport\n");
		bSuccess = false;
	}

	if (FAILED(ms_lpd3dDevice->SetViewport(&m_ShadowMapViewport)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Set Shadow Map viewport\n");
		bSuccess = false;
	}

	if (FAILED(ms_lpd3dDevice->SetVertexShader(m_lpShadowMapVertexShaderDepth)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Failed to set depth pass vertex shader\n");
		bSuccess = false;
	}
	if (FAILED(ms_lpd3dDevice->SetPixelShader(m_lpShadowMapPixelShaderDepth)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Failed to set depth pass pixel shader\n");
		bSuccess = false;
	}

	D3DXMatrixMultiplyTranspose(&m_MatLightViewProj, &matLightView, &matLightProj);
	ms_lpd3dDevice->SetVertexShaderConstantF(4, (float*)&m_MatLightViewProj, 4);

	return bSuccess;
}

void CMapOutdoor::EndRenderCharacterShadowToTexture()
{
	ms_lpd3dDevice->SetViewport(&m_BackupViewport);

#ifdef DIRECTX9
	ms_lpd3dDevice->SetDepthStencilSurface(m_lpBackupDepthSurface);
	ms_lpd3dDevice->SetRenderTarget(0, m_lpBackupRenderTargetSurface);
#else
	ms_lpd3dDevice->SetRenderTarget(m_lpBackupRenderTargetSurface, m_lpBackupDepthSurface);
#endif

	SAFE_RELEASE(m_lpBackupRenderTargetSurface);
	SAFE_RELEASE(m_lpBackupDepthSurface);

	STATEMANAGER.RestoreTransform(D3DTS_VIEW);
	STATEMANAGER.RestoreTransform(D3DTS_PROJECTION);

	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, dwLightEnable);
	STATEMANAGER.RestoreRenderState(D3DRS_TEXTUREFACTOR);

	ms_lpd3dDevice->SetVertexShader(nullptr);
	ms_lpd3dDevice->SetPixelShader(nullptr);

	CSpeedTreeForestDirectX8::Instance().UpdateCompundMatrix(CCameraManager::Instance().GetCurrentCamera()->GetEye(), ms_matView, ms_matProj);
}

bool CMapOutdoor::BeginRenderCharacterShadowToTexture2()
{
	time_t curr_time;
	curr_time = time(NULL);
	tm* tm_local = localtime(&curr_time);

	if (tm_local->tm_sec == 9 || tm_local->tm_sec == 29 || tm_local->tm_sec == 49)
	{
		shadowstep = 0;
	}

	D3DXMATRIX matLightView, matLightProj;

	CCamera* pCurrentCamera = CCameraManager::Instance().GetCurrentCamera();

	if (!pCurrentCamera)
		return false;

	if (recreate)
	{
		CreateCharacterShadowTexture();
		recreate = false;
	}

	const TEnvironmentData* envData = 0;
	CFlyingManager::Instance().GetMapManagerPtr()->GetCurrentEnvironmentData(&envData);
	if (!envData)
		return false;

	D3DXVECTOR3 v3SunPosition = envData->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction;
	v3SunPosition.z = v3SunPosition.z * 1.5f;

	D3DXVECTOR3 v3Target = pCurrentCamera->GetTarget();
	if (a)
	{
		current_v3SunPosition = v3SunPosition;
		a = false;
	}
	SanitizeSunVector(current_v3SunPosition, &v3SunPosition);

	float distanceX = v3SunPosition.x - current_v3SunPosition.x;
	float distanceY = v3SunPosition.y - current_v3SunPosition.y;
	float distanceZ = v3SunPosition.z - current_v3SunPosition.z;

	if (std::abs(distanceX) > tolerance)
	{
		float stepX = min(std::abs(distanceX), z);
		current_v3SunPosition.x += stepX * (distanceX > 0 ? 1 : -1);
	}

	if (std::abs(distanceY) > tolerance)
	{
		float stepY = min(std::abs(distanceY), z);
		current_v3SunPosition.y += stepY * (distanceY > 0 ? 1 : -1);
	}

	if (std::abs(distanceZ) > tolerance)
	{
		float stepZ = min(std::abs(distanceZ), z * zMultiplier);
		current_v3SunPosition.z += stepZ * (distanceZ > 0 ? 1 : -1);
	}

	if (std::abs(current_v3SunPosition.x - v3SunPosition.x) <= tolerance)
		current_v3SunPosition.x = v3SunPosition.x;
	if (std::abs(current_v3SunPosition.y - v3SunPosition.y) <= tolerance)
		current_v3SunPosition.y = v3SunPosition.y;
	if (std::abs(current_v3SunPosition.z - v3SunPosition.z) <= tolerance)
		current_v3SunPosition.z = v3SunPosition.z;

	if (std::strcmp(Frustum::Instance().currentMapNameSave.c_str(), "metin2_map_skipia_dungeon_02") != 0
		&& std::strcmp(Frustum::Instance().currentMapNameSave.c_str(), "metin2_map_skipia_dungeon_boss") != 0)
	{
		if ((Frustum::Instance().selectRainFrustun == 16 || Frustum::Instance().selectRainFrustun == 25 || Frustum::Instance().selectRainFrustun == 8) && std::strcmp(Frustum::Instance().currentMapNameSave.c_str(), "map_n_snowm_01") != 0)
		{
			if (shadowstep >= 0 && shadowstep <= 2 || shadowstep == 8)
			{
				if (tm_local->tm_sec == 10 || tm_local->tm_sec == 30 || tm_local->tm_sec == 50)
				{
					shadowstep += 1;
					if (tm_local->tm_sec == 10)
					{
						current_v3SunPosition.z = current_v3SunPosition.z + 0.065f;
						current_v3SunPosition.x = current_v3SunPosition.x + 0.065f;
					}
					if (tm_local->tm_sec == 30)
					{
						current_v3SunPosition.z = current_v3SunPosition.z - 0.065f;
						current_v3SunPosition.x = current_v3SunPosition.x - 0.065f;
					}
					if (tm_local->tm_sec == 50)
					{
						current_v3SunPosition.z = current_v3SunPosition.z + 0.065f;
						current_v3SunPosition.x = current_v3SunPosition.x - 0.065f;
					}
					z = 0.1f;
				}
			}
			if (tm_local->tm_sec == 12 || tm_local->tm_sec == 32 || tm_local->tm_sec == 52)
			{
				z = 0.015f;
			}
		}
	}

	SanitizeSunVector(current_v3SunPosition, &v3SunPosition);

	const auto vv = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	const auto shadow_eye = (v3Target)-current_v3SunPosition * (200000.0f);
	D3DXMatrixLookAtRH(&m_matLightView2,
		&shadow_eye,
		&v3Target,
		&vv);

	const float fShadowSpaceSize2 = (Frustum::Instance().graphicsLevel >= 4) ? 29000.0f : 25000.0f;
	SnapShadowSpaceMatrix(m_matLightView2, fShadowSpaceSize2);
	D3DXMatrixOrthoRH(&matLightProj, fShadowSpaceSize2, fShadowSpaceSize2, 1.0f, 300000.0f);

	STATEMANAGER.SaveTransform(D3DTS_VIEW, &m_matLightView2);
	STATEMANAGER.SaveTransform(D3DTS_PROJECTION, &matLightProj);

	dwLightEnable = STATEMANAGER.GetRenderState(D3DRS_LIGHTING);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

	STATEMANAGER.SaveRenderState(D3DRS_TEXTUREFACTOR, 0xFF808080);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);

	bool bSuccess = true;

#ifdef DIRECTX9
	if (FAILED(ms_lpd3dDevice->GetRenderTarget(0, &m_lpBackupRenderTargetSurface)))
#else
	if (FAILED(ms_lpd3dDevice->GetRenderTarget(&m_lpBackupRenderTargetSurface)))
#endif
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Save Window Render Target\n");
		bSuccess = false;
	}

	if (FAILED(ms_lpd3dDevice->GetDepthStencilSurface(&m_lpBackupDepthSurface)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Save Window Depth Surface\n");
		bSuccess = false;
	}

#ifdef DIRECTX9
	ms_lpd3dDevice->SetDepthStencilSurface(m_lpCharacterShadowMapDepthSurface);
	ms_lpd3dDevice->SetRenderTarget(0, m_lpCharacterShadowMapRenderTargetSurface2);
#else
	if (FAILED(ms_lpd3dDevice->SetRenderTarget(m_lpCharacterShadowMapRenderTargetSurface2, m_lpCharacterShadowMapDepthSurface)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Set Shadow Map Render Target\n");
		bSuccess = false;
	}
#endif

	if (FAILED(ms_lpd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0xFF, 0xFF, 0xFF), 1.0f, 0)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Clear Render Target");
		bSuccess = false;
	}

	if (FAILED(ms_lpd3dDevice->GetViewport(&m_BackupViewport)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Save Window Viewport\n");
		bSuccess = false;
	}

	if (FAILED(ms_lpd3dDevice->SetViewport(&m_ShadowMapViewport)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Set Shadow Map viewport\n");
		bSuccess = false;
	}

	return bSuccess;
}

void CMapOutdoor::EndRenderCharacterShadowToTexture2()
{
	ms_lpd3dDevice->SetViewport(&m_BackupViewport);

#ifdef DIRECTX9
	ms_lpd3dDevice->SetDepthStencilSurface(m_lpBackupDepthSurface);
	ms_lpd3dDevice->SetRenderTarget(0, m_lpBackupRenderTargetSurface);
#else
	ms_lpd3dDevice->SetRenderTarget(m_lpBackupRenderTargetSurface, m_lpBackupDepthSurface);
#endif

	SAFE_RELEASE(m_lpBackupRenderTargetSurface);
	SAFE_RELEASE(m_lpBackupDepthSurface);

	STATEMANAGER.RestoreTransform(D3DTS_VIEW);
	STATEMANAGER.RestoreTransform(D3DTS_PROJECTION);

	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, dwLightEnable);
	STATEMANAGER.RestoreRenderState(D3DRS_TEXTUREFACTOR);
}

float currentValue = 12000.0f;
float targetValue = 12000.0f;
float stepSize = 10.0f;
float animationDuration = 2.0f;
float elapsedTime = 0.0f;

bool CMapOutdoor::BeginRenderCharacterShadowToTexture3()
{
	time_t curr_time;
	curr_time = time(NULL);
	tm* tm_local = localtime(&curr_time);

	if (tm_local->tm_sec == 9 || tm_local->tm_sec == 29 || tm_local->tm_sec == 49)
	{
		shadowstep = 0;
	}

	D3DXMATRIX matLightView, matLightProj;

	CCamera* pCurrentCamera = CCameraManager::Instance().GetCurrentCamera();

	if (!pCurrentCamera)
		return false;

	if (recreate || !m_lpCharacterShadowMapTexture)
	{
		CreateCharacterShadowTexture();
		recreate = false;
	}

	const TEnvironmentData* envData = 0;
	CFlyingManager::Instance().GetMapManagerPtr()->GetCurrentEnvironmentData(&envData);
	if (!envData)
		return false;
	D3DXVECTOR3 v3SunPosition = envData->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction;
	v3SunPosition.z = v3SunPosition.z * 1.5f;

	if (v3SunPosition != Frustum::instance().previousSunPosition)
	{
		Frustum::instance().isChangingSky = true;
		Frustum::instance().previousSunPosition = v3SunPosition;
	}
	else
	{
		Frustum::instance().isChangingSky = false;
	}

	if (std::strcmp(Frustum::Instance().currentMapNameSave.c_str(), "metin2_map_skipia_dungeon_02") != 0
		&& std::strcmp(Frustum::Instance().currentMapNameSave.c_str(), "metin2_map_skipia_dungeon_boss") != 0)
	{
		if ((Frustum::Instance().selectRainFrustun == 16 || Frustum::Instance().selectRainFrustun == 25 || Frustum::Instance().selectRainFrustun == 8) && std::strcmp(Frustum::Instance().currentMapNameSave.c_str(), "map_n_snowm_01") != 0)
		{
			if (shadowstep >= 0 && shadowstep <= 2 || shadowstep == 8)
			{
				if (tm_local->tm_sec == 10 || tm_local->tm_sec == 30 || tm_local->tm_sec == 50)
				{
					shadowstep += 1;

					const float random_num = 0.15f + (float(rand()) / float(RAND_MAX)) * 0.05f;

					if (tm_local->tm_sec == 10)
					{
						current_v3SunPosition.z = current_v3SunPosition.z + random_num;
						current_v3SunPosition.x = current_v3SunPosition.x + random_num;
						current_v3SunPosition.y = current_v3SunPosition.y + random_num * 0.1108f;
					}
					if (tm_local->tm_sec == 30)
					{
						current_v3SunPosition.z = current_v3SunPosition.z - random_num;
						current_v3SunPosition.x = current_v3SunPosition.x - random_num;
						current_v3SunPosition.y = current_v3SunPosition.y - random_num * 0.1108f;
					}
					if (tm_local->tm_sec == 50)
					{
						current_v3SunPosition.z = current_v3SunPosition.z + random_num;
						current_v3SunPosition.x = current_v3SunPosition.x - random_num;
						current_v3SunPosition.y = current_v3SunPosition.y + random_num * 0.1108f;
					}
				}
			}
			if (tm_local->tm_sec == 12 || tm_local->tm_sec == 32 || tm_local->tm_sec == 52)
			{
				z = 0.015f;
			}
		}
	}

	D3DXVECTOR3 v3Target = pCurrentCamera->GetTarget();
#ifdef __ANDROID__
	if (D3DXVec3LengthSq(&Frustum::Instance().characterPosition) > 1.0f)
		v3Target = Frustum::Instance().characterPosition;
#endif
	if (a)
	{
		current_v3SunPosition = v3SunPosition;
		a = false;
	}
	SanitizeSunVector(current_v3SunPosition, &v3SunPosition);

	float distanceX = v3SunPosition.x - current_v3SunPosition.x;
	float distanceY = v3SunPosition.y - current_v3SunPosition.y;
	float distanceZ = v3SunPosition.z - current_v3SunPosition.z;

#ifdef __ANDROID__
	static DWORD s_dwLastSunStepTick = ELTimer_GetMSec();
	DWORD dwNowSunTick = ELTimer_GetMSec();
	float fSunDtScale = (dwNowSunTick > s_dwLastSunStepTick) ? ((dwNowSunTick - s_dwLastSunStepTick) / 16.666f) : 1.0f;
	s_dwLastSunStepTick = dwNowSunTick;
	if (fSunDtScale > 10.0f) fSunDtScale = 10.0f;
	const float fStepRate = z * fSunDtScale;
#else
	const float fStepRate = z;
#endif

	if (std::abs(distanceX) > tolerance)
	{
		float stepX = min(std::abs(distanceX), fStepRate);
		current_v3SunPosition.x += stepX * (distanceX > 0 ? 1 : -1);
	}

	if (std::abs(distanceY) > tolerance)
	{
		float stepY = min(std::abs(distanceY), fStepRate);
		current_v3SunPosition.y += stepY * (distanceY > 0 ? 1 : -1);
	}

	if (std::abs(distanceZ) > tolerance)
	{
		float stepZ = min(std::abs(distanceZ), fStepRate * zMultiplier);
		current_v3SunPosition.z += stepZ * (distanceZ > 0 ? 1 : -1);
	}

	if (std::abs(current_v3SunPosition.x - v3SunPosition.x) <= tolerance)
		current_v3SunPosition.x = v3SunPosition.x;
	if (std::abs(current_v3SunPosition.y - v3SunPosition.y) <= tolerance)
		current_v3SunPosition.y = v3SunPosition.y;
	if (std::abs(current_v3SunPosition.z - v3SunPosition.z) <= tolerance)
		current_v3SunPosition.z = v3SunPosition.z;

	SanitizeSunVector(current_v3SunPosition, &v3SunPosition);

	const auto vv = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	shadow_eye = (v3Target)-current_v3SunPosition * (200000.0f);
	D3DXMatrixLookAtRH(&matLightView,
		&shadow_eye,
		&v3Target,
		&vv);

	float newTargetValue;
	if (Frustum::Instance().shadowType == 2) {
#ifdef __ANDROID__
		newTargetValue = 16000.0f;
#else
		newTargetValue = 35000.0f;
#endif
	}
	else {
		// Mode 4 uses 9000.0f (+20m radius); Mode 3 uses original 5000.0f
		newTargetValue = (Frustum::Instance().graphicsLevel >= 4) ? 9000.0f : 5000.0f;
	}
	if (newTargetValue != targetValue) {
		targetValue = newTargetValue;
		elapsedTime = 0.0f;
		float totalDistance = std::abs(targetValue - currentValue);
		stepSize = totalDistance / (animationDuration * 60);
	}

	if (elapsedTime < animationDuration) {
		float distance = targetValue - currentValue;
		if (std::abs(distance) > 0) {
#ifdef __ANDROID__
			float step = stepSize * 0.03f * 60.0f * fSunDtScale;
			currentValue += step * (distance > 0 ? 1 : -1);
			elapsedTime += 0.03f * fSunDtScale;
#else
			float step = stepSize * 0.03f * 60;
			currentValue += step * (distance > 0 ? 1 : -1);
			elapsedTime += 0.03f;
#endif
		}
	}
	else {
		currentValue = targetValue;
	}

	SnapShadowSpaceMatrix(matLightView, currentValue);
	D3DXMatrixOrthoRH(&matLightProj, currentValue, currentValue, 1.0f, 300000.0f);

	STATEMANAGER.SaveTransform(D3DTS_VIEW, &matLightView);
	STATEMANAGER.SaveTransform(D3DTS_PROJECTION, &matLightProj);
	CSpeedTreeForestDirectX8::Instance().UpdateCompundMatrix(((v3Target)-current_v3SunPosition * (200000.0f)), matLightView, matLightProj);

	dwLightEnable = STATEMANAGER.GetRenderState(D3DRS_LIGHTING);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

	STATEMANAGER.SaveRenderState(D3DRS_TEXTUREFACTOR, 0xFF808080);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

	bool bSuccess = true;

#ifdef DIRECTX9
	if (FAILED(ms_lpd3dDevice->GetRenderTarget(0, &m_lpBackupRenderTargetSurface)))
#else
	if (FAILED(ms_lpd3dDevice->GetRenderTarget(&m_lpBackupRenderTargetSurface)))
#endif
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Save Window Render Target\n");
		bSuccess = false;
	}

	if (FAILED(ms_lpd3dDevice->GetDepthStencilSurface(&m_lpBackupDepthSurface)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Save Window Depth Surface\n");
		bSuccess = false;
	}

#ifdef DIRECTX9
	ms_lpd3dDevice->SetDepthStencilSurface(m_lpCharacterShadowMapDepthSurface);
	ms_lpd3dDevice->SetRenderTarget(0, m_lpCharacterShadowMapRenderTargetSurface);
#else
	if (FAILED(ms_lpd3dDevice->SetRenderTarget(m_lpCharacterShadowMapRenderTargetSurface, m_lpCharacterShadowMapDepthSurface)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Set Shadow Map Render Target\n");
		bSuccess = false;
	}
#endif

	if (FAILED(ms_lpd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0xFF, 0xFF, 0xFF), 1.0f, 0)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Clear Render Target");
		bSuccess = false;
	}

	if (FAILED(ms_lpd3dDevice->GetViewport(&m_BackupViewport)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Save Window Viewport\n");
		bSuccess = false;
	}

	if (FAILED(ms_lpd3dDevice->SetViewport(&m_ShadowMapViewportTree)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Set Shadow Map viewport\n");
		bSuccess = false;
	}

	if (FAILED(ms_lpd3dDevice->SetVertexShader(m_lpShadowMapVertexShaderDepth)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Failed to set depth pass vertex shader\n");
		bSuccess = false;
	}
	if (FAILED(ms_lpd3dDevice->SetPixelShader(m_lpShadowMapPixelShaderDepth)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Failed to set depth pass pixel shader\n");
		bSuccess = false;
	}

	D3DXMatrixMultiplyTranspose(&m_MatLightViewProj, &matLightView, &matLightProj);
	ms_lpd3dDevice->SetVertexShaderConstantF(4, (float*)&m_MatLightViewProj, 4);

	return bSuccess;
}

void CMapOutdoor::EndRenderCharacterShadowToTexture3()
{
	ms_lpd3dDevice->SetViewport(&m_BackupViewport);

#ifdef DIRECTX9
	ms_lpd3dDevice->SetDepthStencilSurface(m_lpBackupDepthSurface);
	ms_lpd3dDevice->SetRenderTarget(0, m_lpBackupRenderTargetSurface);
#else
	ms_lpd3dDevice->SetRenderTarget(m_lpBackupRenderTargetSurface, m_lpBackupDepthSurface);
#endif

	SAFE_RELEASE(m_lpBackupRenderTargetSurface);
	SAFE_RELEASE(m_lpBackupDepthSurface);

	STATEMANAGER.RestoreTransform(D3DTS_VIEW);
	STATEMANAGER.RestoreTransform(D3DTS_PROJECTION);

	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, dwLightEnable);
	STATEMANAGER.RestoreRenderState(D3DRS_TEXTUREFACTOR);
	STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

	ms_lpd3dDevice->SetVertexShader(nullptr);
	ms_lpd3dDevice->SetPixelShader(nullptr);

	CSpeedTreeForestDirectX8::Instance().UpdateCompundMatrix(CCameraManager::Instance().GetCurrentCamera()->GetEye(), ms_matView, ms_matProj);
}


void CMapOutdoor::SnapShadowSpaceMatrix(D3DXMATRIX& mat, float spaceSize)
{
	float step;
#ifdef __ANDROID__
	float texelRes = (m_wShadowMapSize >= 512) ? static_cast<float>(m_wShadowMapSize) : 2048.0f;
	step = spaceSize / texelRes;
#else
	if (Frustum::Instance().graphicsLevel >= 4)
	{
		float texelRes = (m_wShadowMapSize >= 512) ? static_cast<float>(m_wShadowMapSize) : 2048.0f;
		step = spaceSize / texelRes;
	}
	else
	{
		step = spaceSize / 128.0f;
	}
#endif

	mat._41 = floor(mat._41 / step) * step;
	mat._42 = floor(mat._42 / step) * step;
	mat._43 = floor(mat._43 / step) * step;
}

bool CMapOutdoor::BeginRenderCharacterShadowToTexture4()
{
	D3DXMATRIX matLightView, matLightProj;

	CCamera* pCurrentCamera = CCameraManager::Instance().GetCurrentCamera();

	if (!pCurrentCamera)
		return false;

	if (recreate || !m_lpCharacterShadowMapTexture4)
	{
		CreateCharacterShadowTexture();
		recreate = false;
	}

	const TEnvironmentData* envData = 0;
	CFlyingManager::Instance().GetMapManagerPtr()->GetCurrentEnvironmentData(&envData);
	D3DXVECTOR3 v3Target = pCurrentCamera->GetTarget();
#ifdef __ANDROID__
	if (D3DXVec3LengthSq(&Frustum::Instance().characterPosition) > 1.0f)
		v3Target = Frustum::Instance().characterPosition;
	shadow_eye = (v3Target) - current_v3SunPosition * (200000.0f);
#endif

	const auto vv = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	D3DXMatrixLookAtRH(&matLightView,
		&shadow_eye,
		&v3Target,
		&vv);

	// Mode 4 uses 8000.0f; Mode 3 uses original 4000.0f
	const float fShadowSpaceSize4 = (Frustum::Instance().graphicsLevel >= 4) ? 8000.0f : 4000.0f;
	SnapShadowSpaceMatrix(matLightView, fShadowSpaceSize4);
	D3DXMatrixOrthoRH(&matLightProj, fShadowSpaceSize4, fShadowSpaceSize4, 1.0f, 300000.0f);

	STATEMANAGER.SaveTransform(D3DTS_VIEW, &matLightView);
	STATEMANAGER.SaveTransform(D3DTS_PROJECTION, &matLightProj);
	CSpeedTreeForestDirectX8::Instance().UpdateCompundMatrix2(((v3Target)-current_v3SunPosition * (200000.0f)), matLightView, matLightProj);

	dwLightEnable = STATEMANAGER.GetRenderState(D3DRS_LIGHTING);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

	STATEMANAGER.SaveRenderState(D3DRS_TEXTUREFACTOR, 0xFF808080);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);

	bool bSuccess = true;

#ifdef DIRECTX9
	if (FAILED(ms_lpd3dDevice->GetRenderTarget(0, &m_lpBackupRenderTargetSurface)))
#else
	if (FAILED(ms_lpd3dDevice->GetRenderTarget(&m_lpBackupRenderTargetSurface)))
#endif
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Save Window Render Target\n");
		bSuccess = false;
	}

	if (FAILED(ms_lpd3dDevice->GetDepthStencilSurface(&m_lpBackupDepthSurface)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Save Window Depth Surface\n");
		bSuccess = false;
	}

#ifdef DIRECTX9
	ms_lpd3dDevice->SetDepthStencilSurface(m_lpCharacterShadowMapDepthSurface);
	ms_lpd3dDevice->SetRenderTarget(0, m_lpCharacterShadowMapRenderTargetSurface4);
#else
	if (FAILED(ms_lpd3dDevice->SetRenderTarget(m_lpCharacterShadowMapRenderTargetSurface4, m_lpCharacterShadowMapDepthSurface)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Set Shadow Map Render Target\n");
		bSuccess = false;
	}
#endif

	if (FAILED(ms_lpd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0xFF, 0xFF, 0xFF), 1.0f, 0)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Clear Render Target");
		bSuccess = false;
	}

	if (FAILED(ms_lpd3dDevice->GetViewport(&m_BackupViewport)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Save Window Viewport\n");
		bSuccess = false;
	}

	if (FAILED(ms_lpd3dDevice->SetViewport(&m_ShadowMapViewport)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Set Shadow Map viewport\n");
		bSuccess = false;
	}

	if (FAILED(ms_lpd3dDevice->SetVertexShader(m_lpShadowMapVertexShaderDepth)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Failed to set depth pass vertex shader\n");
		bSuccess = false;
	}
	if (FAILED(ms_lpd3dDevice->SetPixelShader(m_lpShadowMapPixelShaderDepth)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Failed to set depth pass pixel shader\n");
		bSuccess = false;
	}

	D3DXMatrixMultiplyTranspose(&m_MatLightViewProj2, &matLightView, &matLightProj);
	ms_lpd3dDevice->SetVertexShaderConstantF(4, (float*)&m_MatLightViewProj2, 4);

	return bSuccess;
}

void CMapOutdoor::EndRenderCharacterShadowToTexture4()
{
	ms_lpd3dDevice->SetViewport(&m_BackupViewport);

#ifdef DIRECTX9
	ms_lpd3dDevice->SetDepthStencilSurface(m_lpBackupDepthSurface);
	ms_lpd3dDevice->SetRenderTarget(0, m_lpBackupRenderTargetSurface);
#else
	ms_lpd3dDevice->SetRenderTarget(m_lpBackupRenderTargetSurface, m_lpBackupDepthSurface);
#endif

	SAFE_RELEASE(m_lpBackupRenderTargetSurface);
	SAFE_RELEASE(m_lpBackupDepthSurface);

	STATEMANAGER.RestoreTransform(D3DTS_VIEW);
	STATEMANAGER.RestoreTransform(D3DTS_PROJECTION);

	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, dwLightEnable);
	STATEMANAGER.RestoreRenderState(D3DRS_TEXTUREFACTOR);

	ms_lpd3dDevice->SetVertexShader(nullptr);
	ms_lpd3dDevice->SetPixelShader(nullptr);

	CSpeedTreeForestDirectX8::Instance().UpdateCompundMatrix(CCameraManager::Instance().GetCurrentCamera()->GetEye(), ms_matView, ms_matProj);
}

void CMapOutdoor::CreateCharacterShadowTextureOriginal()
{
	extern bool GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW;

	if (GRAPHICS_CAPS_CAN_NOT_DRAW_SHADOW)
		return;

	ReleaseCharacterShadowTextureOriginal();

	if (IsLowTextureMemory())
		SetShadowTextureSize(128);

	m_ShadowMapViewport.X = 1;
	m_ShadowMapViewport.Y = 1;
	m_ShadowMapViewport.Width = m_wShadowMapSize - 2;
	m_ShadowMapViewport.Height = m_wShadowMapSize - 2;
	m_ShadowMapViewport.MinZ = 0.0f;
	m_ShadowMapViewport.MaxZ = 1.0f;

	if (FAILED(ms_lpd3dDevice->CreateTexture(m_wShadowMapSize, m_wShadowMapSize, 1, D3DUSAGE_RENDERTARGET, D3DFMT_R5G6B5, D3DPOOL_DEFAULT, &m_lpCharacterShadowMapTextureOriginal, nullptr)))
	{
		TraceError("CMapOutdoor Unable to create Character Shadow render target texture\n");
		return;
	}

	if (FAILED(m_lpCharacterShadowMapTextureOriginal->GetSurfaceLevel(0, &m_lpCharacterShadowMapRenderTargetSurfaceOriginal)))
	{
		TraceError("CMapOutdoor Unable to GetSurfaceLevel Character Shadow render target texture\n");
		return;
	}

	if (FAILED(ms_lpd3dDevice->CreateDepthStencilSurface(m_wShadowMapSize, m_wShadowMapSize, D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, true, &m_lpCharacterShadowMapDepthSurfaceOriginal, NULL)))
	{
		TraceError("CMapOutdoor Unable to create Character Shadow depth Surface\n");
		return;
	}
}

bool CMapOutdoor::BeginRenderCharacterShadowToTextureOriginal()
{
	D3DXMATRIX matLightView, matLightProj;

	CCamera* pCurrentCamera = CCameraManager::Instance().GetCurrentCamera();

	if (!pCurrentCamera)
		return false;

	if (recreate || !m_lpCharacterShadowMapTextureOriginal)
	{
		CreateCharacterShadowTextureOriginal();
		recreate = false;
	}

	D3DXVECTOR3 v3Target = pCurrentCamera->GetTarget();

	D3DXVECTOR3 v3Eye(v3Target.x - 1.732f * 1250.0f,
		v3Target.y - 1250.0f,
		v3Target.z + 2.0f * 1.732f * 1250.0f);

	auto val = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	D3DXMatrixLookAtRH(&matLightView,
		&v3Eye,
		&v3Target,
		&val);

	D3DXMatrixOrthoRH(&matLightProj, 2550.0f, 2550.0f, 1.0f, 15000.0f);

	STATEMANAGER.SaveTransform(D3DTS_VIEW, &matLightView);
	STATEMANAGER.SaveTransform(D3DTS_PROJECTION, &matLightProj);

	dwLightEnable = STATEMANAGER.GetRenderState(D3DRS_LIGHTING);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

	STATEMANAGER.SaveRenderState(D3DRS_TEXTUREFACTOR, 0xFF808080);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);

	bool bSuccess = true;

	if (FAILED(ms_lpd3dDevice->GetRenderTarget(0, &m_lpBackupRenderTargetSurface)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Save Window Render Target\n");
		bSuccess = false;
	}

	if (FAILED(ms_lpd3dDevice->GetDepthStencilSurface(&m_lpBackupDepthSurface)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Save Window Depth Surface\n");
		bSuccess = false;
	}

	ms_lpd3dDevice->SetDepthStencilSurface(m_lpCharacterShadowMapDepthSurfaceOriginal);
	ms_lpd3dDevice->SetRenderTarget(0, m_lpCharacterShadowMapRenderTargetSurfaceOriginal);

	if (FAILED(ms_lpd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0xFF, 0xFF, 0xFF), 1.0f, 0)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Clear Render Target");
		bSuccess = false;
	}

	if (FAILED(ms_lpd3dDevice->GetViewport(&m_BackupViewport)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Save Window Viewport\n");
		bSuccess = false;
	}

	if (FAILED(ms_lpd3dDevice->SetViewport(&m_ShadowMapViewport)))
	{
		TraceError("CMapOutdoor::BeginRenderCharacterShadowToTexture : Unable to Set Shadow Map viewport\n");
		bSuccess = false;
	}

	return bSuccess;
}

void CMapOutdoor::EndRenderCharacterShadowToTextureOriginal()
{
	ms_lpd3dDevice->SetViewport(&m_BackupViewport);

#ifdef DIRECTX9
	ms_lpd3dDevice->SetDepthStencilSurface(m_lpBackupDepthSurface);
	ms_lpd3dDevice->SetRenderTarget(0, m_lpBackupRenderTargetSurface);
#else
	ms_lpd3dDevice->SetRenderTarget(m_lpBackupRenderTargetSurface, m_lpBackupDepthSurface);
#endif

	SAFE_RELEASE(m_lpBackupRenderTargetSurface);
	SAFE_RELEASE(m_lpBackupDepthSurface);

	STATEMANAGER.RestoreTransform(D3DTS_VIEW);
	STATEMANAGER.RestoreTransform(D3DTS_PROJECTION);

	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, dwLightEnable);
	STATEMANAGER.RestoreRenderState(D3DRS_TEXTUREFACTOR);
}
//martysama0134's fb0d8d5d0566583187a94f9a823b2075