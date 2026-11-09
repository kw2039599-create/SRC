#include "StdAfx.h"
#include "MapOutdoor.h"
#include "../EterLib/Camera.h"

#include "../eterlib/StateManager.h"

#include "../ScriptLib/StdAfx.h"

#include "../UserInterface/PythonSystem.h"
#include "../UserInterface/PythonBackground.h"

float time3 = 0.0f;
float timeAuraEnabled = 0.0f;
float wetness = 1.0f;
float checkMap = 0.0f;
float shinessRain = 0.0f;
float puddleThreshold = 0.0f;

namespace
{
	struct STerrainTexShaderCacheEntry
	{
		LPDIRECT3DTEXTURE9 pd3dDiffuse = nullptr;
		LPDIRECT3DTEXTURE9 pd3dNormal = nullptr;
		LPDIRECT3DTEXTURE9 pd3dSpecular = nullptr;
		float fUScale = 0.0f;
		float fUV = 1.0f;
		float fSpecularPower = 18.0f;
	};

	static STerrainTexShaderCacheEntry s_akTerrainTexCache[256];

	inline void ResolveTerrainTextureShaderParams(DWORD dwTexIndex, const TTerrainTexture& rTexture, LPDIRECT3DTEXTURE9& rpNormal, LPDIRECT3DTEXTURE9& rpSpec, float& rfUV, float& rfSpecPower)
	{
		auto& rFrustum = Frustum::Instance();
#ifdef __ANDROID__
		rpNormal = rFrustum.emptyNormalMap ? rFrustum.emptyNormalMap : rTexture.pd3dTexture;
		rpSpec = rpNormal;
		if (rFrustum.terrainDepthLevel == 0 || rFrustum.graphicsLevel < 4)
			rfUV = 1.0f;
		else
			rfUV = (rTexture.stFilename.find("beach") != std::string::npos) ? 3.0f : (rTexture.UScale / 5.0f);
		rfSpecPower = 1.5f;
		return;
#endif
		if (rFrustum.terrainDepthLevel == 0 || rFrustum.graphicsLevel < 4)
		{
			rpNormal = rFrustum.emptyNormalMap ? rFrustum.emptyNormalMap : rTexture.pd3dTexture;
			rpSpec = rpNormal;
			rfUV = (rTexture.stFilename.find("beach") != std::string::npos) ? 3.0f : (rTexture.UScale / 5.0f);
			rfSpecPower = 0.25f * 6.0f;
			return;
		}

		if (dwTexIndex < 256)
		{
			STerrainTexShaderCacheEntry& rEntry = s_akTerrainTexCache[dwTexIndex];
			if (rEntry.pd3dDiffuse == rTexture.pd3dTexture &&
				rEntry.fUScale == rTexture.UScale &&
				rEntry.pd3dNormal != nullptr)
			{
				rpNormal = rEntry.pd3dNormal;
				rpSpec = rEntry.pd3dSpecular;
				rfUV = rEntry.fUV;
				rfSpecPower = rEntry.fSpecularPower;
				return;
			}
		}

		float uv = rTexture.UScale / 5.0f;
		float textureSpecularPower = 3.0f;
		LPDIRECT3DTEXTURE9 pNorm = rFrustum.normalMapRacketDirt;
		LPDIRECT3DTEXTURE9 pSpec = rFrustum.normalMapRacketDirt;

		const std::string& stName = rTexture.stFilename;

		if (stName.find("tile01") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.normalMapTile01;
		}
		else if (stName.find("sand02") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.normalMapRacketDirt;
			textureSpecularPower = 1.5f;
		}
		else if (stName.find("snow_01_diff_4k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.normalMapSnow;
			textureSpecularPower = 1.1f;
		}
		else if (stName.find("snow_02") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.normalMapSnow1;
			textureSpecularPower = 1.1f;
		}
		else if (stName.find("snow1") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.normalMapSnow2;
			textureSpecularPower = 0.6f;
		}
		else if (stName.find("snow2") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.normalMapSnow3;
			textureSpecularPower = 0.6f;
		}
		else if (stName.find("snow3") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.normalMapSnow4;
			textureSpecularPower = 0.6f;
		}
		else if (stName.find("ice") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.normalMapSnowIce;
			textureSpecularPower = 2.5f;
		}
		else if (stName.find("beach") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.normalMapBitch;
			uv = 3.0f;
		}
		else if (stName.find("field") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.normalMapRacketDirt;
			textureSpecularPower = 1.5f;
		}
		else if (stName.find("sand_01_diff_4k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.normalMapSand03;
			textureSpecularPower = 2.75f;
		}
		else if (stName.find("dry_ground_rocks") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.normalMapDryGroundRock;
			textureSpecularPower = 1.75f;
		}
		else if (stName.find("cobblestone_color") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.normalMapCobblestoneTemple1;
			textureSpecularPower = 3.5f;
		}
		else if (stName.find("cobblestone_floor_13") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.normalMapCobblestoneTemple2;
			textureSpecularPower = 3.5f;
		}
		else if (stName.find("aerial_grass_rock_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.normalMapAerialGrass;
			textureSpecularPower = 3.5f;
		}
		else if (stName.find("forest_leaves_03_diff_1k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.forest_leaves_03_diff_1k;
			textureSpecularPower = 1.5f;
		}
		else if (stName.find("dirt_aerial_02_diff_1k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.dirt_aerial_02_diff_1k;
		}
		else if (stName.find("mud_cracked_dry_riverbed_002_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.normalMapSand03;
			textureSpecularPower = 2.0f;
		}
		else if (stName.find("rocky_trail_02_diff_1k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.rocky_trail_02_diff_1k;
		}
		else if (stName.find("rock_wall_02_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.rock_wall_02_diff_2k;
			textureSpecularPower = 2.5f;
		}
		else if (stName.find("mud_forest_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.mud_forest_diff_2k;
			textureSpecularPower = 1.5f;
		}
		else if (stName.find("grass_map1") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.grass_map1;
			textureSpecularPower = 1.3f;
		}
		else if (stName.find("mossy_rock_diff_1k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.mossy_rock_diff_1k;
			textureSpecularPower = 3.5f;
		}
		else if (stName.find("leafy_grass_diff_1k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.normalMapGrass;
			textureSpecularPower = 1.5f;
		}
		else if (stName.find("brown_mud_leaves_01_diff_1k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.brown_mud_leaves_01_diff_1k;
			textureSpecularPower = 2.5f;
		}
		else if (stName.find("raked_dirt_diff_1k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.normalMapRacketDirt;
			textureSpecularPower = 2.5f;
		}
		else if (stName.find("cobblestone_large_01_diff_1k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.cobblestone_large_01_diff_1k;
			textureSpecularPower = 1.75f;
		}
		else if (stName.find("cobblestone_05_diff_1k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.cobblestone_05_diff_1k;
			textureSpecularPower = 2.5f;
		}
		else if (stName.find("playground_sand_diff_1k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.playground_sand_diff_1k;
			textureSpecularPower = 1.5f;
		}
		else if (stName.find("monastery_stone_floor_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.monastery_stone_floor_diff_2k;
			textureSpecularPower = 3.5f;
		}
		else if (stName.find("cobblestone_floor_01_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.cobblestone_floor_01_diff_2k;
			textureSpecularPower = 2.5f;
		}
		else if (stName.find("cobblestone_floor_02_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.cobblestone_floor_02_diff_2k;
			textureSpecularPower = 2.5f;
		}
		else if (stName.find("concrete_rock_path_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.concrete_rock_path_diff_2k;
			textureSpecularPower = 2.5f;
		}
		else if (stName.find("dirt_floor_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.dirt_floor_diff_2k;
			textureSpecularPower = 1.5f;
		}
		else if (stName.find("forest_floor_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.forest_floor_diff_2k;
			textureSpecularPower = 1.5f;
		}
		else if (stName.find("forrest_ground_03_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.forrest_ground_03_diff_2k;
			textureSpecularPower = 1.5f;
		}
		else if (stName.find("gray_rocks_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.gray_rocks_diff_2k;
			textureSpecularPower = 3.5f;
		}
		else if (stName.find("grey_stone_path_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.grey_stone_path_diff_2k;
			textureSpecularPower = 3.5f;
		}
		else if (stName.find("leaves_forest_ground_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.leaves_forest_ground_diff_2k;
			textureSpecularPower = 2.5f;
		}
		else if (stName.find("rock_06_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.rock_06_diff_2k;
			textureSpecularPower = 3.5f;
		}
		else if (stName.find("rock_boulder_cracked_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.rock_boulder_cracked_diff_2k;
			textureSpecularPower = 1.5f;
		}
		else if (stName.find("rock_face_03_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.rock_face_03_diff_2k;
			textureSpecularPower = 3.5f;
		}
		else if (stName.find("rustic_stone_wall_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.rustic_stone_wall_diff_2k;
			textureSpecularPower = 3.5f;
		}
		else if (stName.find("sand_02_diff_2k_wet") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.sand_02_diff_2k;
			textureSpecularPower = 4.0f;
		}
		else if (stName.find("sand_02_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.sand_02_diff_2k;
			textureSpecularPower = 4.0f;
		}
		else if (stName.find("ground_0019_color_2k") != std::string::npos)
		{
			pNorm = rFrustum.ground_0019_color_2k;
			pSpec = rFrustum.waterLeavesSpecularTexture;
			textureSpecularPower = 7.0f;
		}
		else if (stName.find("ground_0007_color_2k") != std::string::npos)
		{
			pNorm = rFrustum.ground_0007_color_2k;
			pSpec = rFrustum.waterSandSpecularTexture;
			textureSpecularPower = 3.9f;
		}
		else if (stName.find("sand_dunes") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.sand_dunes;
			textureSpecularPower = 0.9f;
		}
		else if (stName.find("sandstone_cracks_diff_4k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.sandstone_cracks_diff_4k;
			textureSpecularPower = 0.9f;
		}
		else if (stName.find("dry_ground_01_diff_4k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.dry_ground_01_diff_4k;
			textureSpecularPower = 1.9f;
		}
		else if (stName.find("ice_0002_color_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.ice_0002_color_2k;
			textureSpecularPower = 1.5f;
		}
		else if (stName.find("ground_0030_color_2k") != std::string::npos)
		{
			pNorm = rFrustum.ground_0030_color_2k;
			pSpec = rFrustum.iceSpecularTexture;
			textureSpecularPower = 2.2f;
		}
		else if (stName.find("ground_0031_color_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.ground_0031_color_2k;
			textureSpecularPower = 1.5f;
		}
		else if (stName.find("rock_0008_color_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.rock_0008_color_2k;
			textureSpecularPower = 1.5f;
		}
		else if (stName.find("aerial_rocks_04_diff_1k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.aerial_rocks_04_diff_1k;
			textureSpecularPower = 2.5f;
		}
		else if (stName.find("coast_sand_rocks_02_diff_1k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.coast_sand_rocks_02_diff_1k;
			textureSpecularPower = 3.5f;
		}
		else if (stName.find("brown_mud_03_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.brown_mud_03_diff_2k;
			textureSpecularPower = 4.5f;
		}
		else if (stName.find("grassy_cobblestone_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.grassy_cobblestone_diff_2k;
			textureSpecularPower = 1.5f;
		}
		else if (stName.find("slate_driveway_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.slate_driveway_diff_2k;
			textureSpecularPower = 2.5f;
		}
		else if (stName.find("cracked_red_ground_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.cracked_red_ground_diff_2k;
			textureSpecularPower = 2.5f;
		}
		else if (stName.find("concrete_floor_painted_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.concrete_floor_painted_diff_2k;
			textureSpecularPower = 1.5f;
		}
		else if (stName.find("burned_ground_01_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.burned_ground_01_diff_2k;
			textureSpecularPower = 2.5f;
		}
		else if (stName.find("brown_mud_dry_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.brown_mud_dry_diff_2k;
			textureSpecularPower = 2.5f;
		}
		else if (stName.find("brown_mud_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.brown_mud_diff_2k;
			textureSpecularPower = 2.5f;
		}
		else if (stName.find("brown_mud_02_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.brown_mud_02_diff_2k;
			textureSpecularPower = 2.5f;
		}
		else if (stName.find("lichen_rock_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.lichen_rock_diff_2k;
			textureSpecularPower = 1.5f;
		}
		else if (stName.find("lava1") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.lava1;
			textureSpecularPower = 3.5f;
		}
		else if (stName.find("lava2") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.lava2;
			textureSpecularPower = 1.5f;
		}
		else if (stName.find("gravel014_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.Gravel014_2K;
			textureSpecularPower = 1.5f;
		}
		else if (stName.find("ground035_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.Ground035_2K;
			textureSpecularPower = 1.5f;
		}
		else if (stName.find("ground033_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.Ground033_2K;
			textureSpecularPower = 0.85f;
		}
		else if (stName.find("ground044_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.Ground044_2K;
			textureSpecularPower = 1.5f;
		}
		else if (stName.find("ground049b_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.Ground049B_2K;
			textureSpecularPower = 1.5f;
		}
		else if (stName.find("ground050_2k") != std::string::npos)
		{
			pNorm = rFrustum.Ground050_2K;
			pSpec = rFrustum.sandSpecularTexture;
			textureSpecularPower = 2.0f;
		}
		else if (stName.find("ground051_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.Ground051_2K;
			textureSpecularPower = 1.5f;
		}
		else if (stName.find("ground054_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.Ground054_2K;
			textureSpecularPower = 0.8f;
		}
		else if (stName.find("ground059_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.Ground059_2K;
			textureSpecularPower = 1.5f;
		}
		else if (stName.find("ground071_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.Ground071_2K;
			textureSpecularPower = 1.5f;
		}
		else if (stName.find("rock029_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.Rock029_2K;
			textureSpecularPower = 3.5f;
		}
		else if (stName.find("ground013") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.ground013;
			textureSpecularPower = 1.5f;
		}
		else if (stName.find("ground019") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.ground019;
			textureSpecularPower = 2.0f;
		}
		else if (stName.find("ground030") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.ground030;
			textureSpecularPower = 0.3f;
		}
		else if (stName.find("ground037") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.ground037;
			textureSpecularPower = 0.9f;
		}
		else if (stName.find("ground069") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.ground069;
			textureSpecularPower = 0.8f;
		}
		else if (stName.find("pavingstones009") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.pavingstones009;
			textureSpecularPower = 4.0f;
		}
		else if (stName.find("rock028") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.rock028;
			textureSpecularPower = 3.1f;
		}
		else if (stName.find("rock051") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.rock051;
			textureSpecularPower = 3.1f;
		}
		else if (stName.find("forest_leaves_02_diffuse_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.forest_leaves_02_diffuse_2k;
			textureSpecularPower = 1.1f;
		}
		else if (stName.find("forest_leaves_04_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.forest_leaves_04_diff_2k;
			textureSpecularPower = 1.1f;
		}
		else if (stName.find("forrest_ground_01_diff_2k") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.forrest_ground_01_diff_2k;
			textureSpecularPower = 1.1f;
		}
		else if (stName.find("ground018") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.Ground018_2K;
			textureSpecularPower = 1.1f;
		}
		else if (stName.find("ground023") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.Ground023_2K;
			textureSpecularPower = 2.1f;
		}
		else if (stName.find("ground024") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.Ground024_2K;
			textureSpecularPower = 1.7f;
		}
		else if (stName.find("ground038") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.Ground038_2K;
			textureSpecularPower = 1.1f;
		}
		else if (stName.find("ground040") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.Ground040_2K;
			textureSpecularPower = 1.1f;
		}
		else if (stName.find("ground042") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.Ground042_2K;
			textureSpecularPower = 1.1f;
		}
		else if (stName.find("ground067") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.Ground067_2K;
			textureSpecularPower = 0.9f;
		}
		else if (stName.find("ground068") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.Ground068_2K;
			textureSpecularPower = 1.6f;
		}
		else if (stName.find("sand") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.normalMapSand01;
			textureSpecularPower = 1.2f;
		}
		else if (stName.find("grass") != std::string::npos)
		{
			pNorm = pSpec = rFrustum.grass_2k;
			textureSpecularPower = 0.2f;
		}
		else
		{
			pNorm = pSpec = rFrustum.normalMapRacketDirt;
			textureSpecularPower = 0.5f;
		}
		if (!pNorm) pNorm = rFrustum.emptyNormalMap ? rFrustum.emptyNormalMap : rTexture.pd3dTexture;
		if (!pSpec) pSpec = pNorm;
		textureSpecularPower *= 6.0f;

		if (dwTexIndex < 256)
		{
			STerrainTexShaderCacheEntry& rEntry = s_akTerrainTexCache[dwTexIndex];
			rEntry.pd3dDiffuse = rTexture.pd3dTexture;
			rEntry.pd3dNormal = pNorm;
			rEntry.pd3dSpecular = pSpec;
			rEntry.fUScale = rTexture.UScale;
			rEntry.fUV = uv;
			rEntry.fSpecularPower = textureSpecularPower;
		}

		rpNormal = pNorm;
		rpSpec = pSpec;
		rfUV = uv;
		rfSpecPower = textureSpecularPower;
	}
}

void CMapOutdoor::__RenderTerrain_RenderHardwareTransformPatch()
{
	DWORD dwFogColor;
	float fFogFarDistance;
	float fFogNearDistance;
	if (mc_pEnvironmentData)
	{
		dwFogColor = mc_pEnvironmentData->FogColor;
		fFogNearDistance = mc_pEnvironmentData->GetFogNearDistance();
		fFogFarDistance = mc_pEnvironmentData->GetFogFarDistance();
	}
	else
	{
		dwFogColor = 0xffffffff;
		fFogNearDistance = 5000.0f;
		fFogFarDistance = 10000.0f;
	}

	//////////////////////////////////////////////////////////////////////////
	// Render State & TextureStageState

	STATEMANAGER.SaveTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
	STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
	STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);

	STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHAREF, 0x00000000);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);

	STATEMANAGER.SaveRenderState(D3DRS_TEXTUREFACTOR, dwFogColor);

	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
#ifdef DIRECTX9
	STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
#else
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
#endif

	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
#ifdef DIRECTX9
	STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
#else
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
#endif

	if (Frustum::Instance().dynamicLight == 3)
	{
		STATEMANAGER.SetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		STATEMANAGER.SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	}

#ifdef WORLD_EDITOR
	if (GetAsyncKeyState(VK_CAPITAL))
	{
		CSpeedTreeWrapper::ms_bSelfShadowOn = false;
		STATEMANAGER.SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_GAUSSIANCUBIC);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_GAUSSIANCUBIC);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_GAUSSIANCUBIC);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_MINFILTER, D3DTEXF_GAUSSIANCUBIC);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTEXF_GAUSSIANCUBIC);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_MIPFILTER, D3DTEXF_GAUSSIANCUBIC);
	}
	else
	{
		CSpeedTreeWrapper::ms_bSelfShadowOn = true;
		STATEMANAGER.SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
	}
#else
	CSpeedTreeWrapper::ms_bSelfShadowOn = true;
	STATEMANAGER.SetBestFiltering(0);
	STATEMANAGER.SetBestFiltering(1);
	STATEMANAGER.SetBestFiltering(2);
#endif

	m_matWorldForCommonUse._41 = 0.0f;
	m_matWorldForCommonUse._42 = 0.0f;
	STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matWorldForCommonUse);
	D3DXMATRIX worldMat;
	D3DXMatrixTranspose(&worldMat, &m_matWorldForCommonUse);
	STATEMANAGER.SetVertexShaderConstant(20, &worldMat, 4);

	STATEMANAGER.SaveTransform(D3DTS_TEXTURE0, &m_matWorldForCommonUse);
	STATEMANAGER.SaveTransform(D3DTS_TEXTURE1, &m_matWorldForCommonUse);

	// Render State & TextureStageState
	//////////////////////////////////////////////////////////////////////////

#ifdef DIRECTX9
	STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL);
#else
	STATEMANAGER.SetVertexShader(D3DFVF_XYZ | D3DFVF_NORMAL);
#endif

	m_iRenderedSplatNumSqSum = 0;
	m_iRenderedPatchNum = 0;
	m_iRenderedSplatNum = 0;
	m_RenderedTextureNumVector.clear();

	time3 += 0.1f;
	if (Frustum::Instance().selectRainFrustun != 0)
	{
		if (Frustum::Instance().isThunderTime == true)
		{
			shinessRain += 0.1f;
			if (shinessRain > 5.0f)
				shinessRain = 5.0f;
		}
		else
		{
			shinessRain = 0.0f;
		}
	}
	UpdateCharacterMovement();

	if ( std::strcmp(Frustum::Instance().currentMapNameSave.c_str(), "metin2_map_a1") == 0 )
		Frustum::Instance().kingdomMapSelector = 1.0f;
	else if ( std::strcmp(Frustum::Instance().currentMapNameSave.c_str(), "metin2_map_b1") == 0 )
		Frustum::Instance().kingdomMapSelector = 2.0f;
	else if ( std::strcmp(Frustum::Instance().currentMapNameSave.c_str(), "metin2_map_c1") == 0 )
		Frustum::Instance().kingdomMapSelector = 3.0f;
	else
		Frustum::Instance().kingdomMapSelector = 0.0f;

	std::pair<float, long> fog_far(fFogFarDistance + 1600.0f, 0);
	std::pair<float, long> fog_near(fFogNearDistance - 3200.0f, 0);

	std::vector<std::pair<float, long> >::iterator far_it = std::upper_bound(m_PatchVector.begin(), m_PatchVector.end(), fog_far);
	std::vector<std::pair<float, long> >::iterator near_it = std::upper_bound(m_PatchVector.begin(), m_PatchVector.end(), fog_near);

#ifdef WORLD_EDITOR
	near_it = m_PatchVector.begin();
	far_it = m_PatchVector.end();
#endif

	WORD wPrimitiveCount;
	D3DPRIMITIVETYPE ePrimitiveType;

	BYTE byCUrrentLODLevel = 0;

	float fLODLevel1Distance = __GetNoFogDistance();
	float fLODLevel2Distance = __GetFogDistance();

	SelectIndexBuffer(0, &wPrimitiveCount, &ePrimitiveType);

	DWORD dwFogEnable = STATEMANAGER.GetRenderState(D3DRS_FOGENABLE);
	std::vector<std::pair<float, long> >::iterator it = m_PatchVector.begin();

#ifndef WORLD_EDITOR
	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);

	for (; it != near_it; ++it)
	{
		if (byCUrrentLODLevel == 0 && fLODLevel1Distance <= it->first)
		{
			byCUrrentLODLevel = 1;
			SelectIndexBuffer(1, &wPrimitiveCount, &ePrimitiveType);
		}
		else if (byCUrrentLODLevel == 1 && fLODLevel2Distance <= it->first)
		{
			byCUrrentLODLevel = 2;
			SelectIndexBuffer(2, &wPrimitiveCount, &ePrimitiveType);
		}

		__HardwareTransformPatch_RenderPatchSplat(it->second, wPrimitiveCount, ePrimitiveType);
		if (m_iRenderedSplatNum >= m_iSplatLimit)
			break;

		if (m_bDrawWireFrame)
			DrawWireFrame(it->second, wPrimitiveCount, ePrimitiveType);
	}
#endif

	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, dwFogEnable);

	if (m_iRenderedSplatNum < m_iSplatLimit)
	{
		for (it = near_it; it != far_it; ++it)
		{
			if (byCUrrentLODLevel == 0 && fLODLevel1Distance <= it->first)
			{
				byCUrrentLODLevel = 1;
				SelectIndexBuffer(1, &wPrimitiveCount, &ePrimitiveType);
			}
			else if (byCUrrentLODLevel == 1 && fLODLevel2Distance <= it->first)
			{
				byCUrrentLODLevel = 2;
				SelectIndexBuffer(2, &wPrimitiveCount, &ePrimitiveType);
			}

			__HardwareTransformPatch_RenderPatchSplat(it->second, wPrimitiveCount, ePrimitiveType);

			if (m_iRenderedSplatNum >= m_iSplatLimit)
				break;

			if (m_bDrawWireFrame)
				DrawWireFrame(it->second, wPrimitiveCount, ePrimitiveType);
		}
	}

	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

	STATEMANAGER.SetTexture(0, NULL);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, FALSE);

	STATEMANAGER.SetTexture(1, NULL);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, FALSE);

	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	if (m_iRenderedSplatNum < m_iSplatLimit)
	{
		for (it = far_it; it != m_PatchVector.end(); ++it)
		{
			if (byCUrrentLODLevel == 0 && fLODLevel1Distance <= it->first)
			{
				byCUrrentLODLevel = 1;
				SelectIndexBuffer(1, &wPrimitiveCount, &ePrimitiveType);
			}
			else if (byCUrrentLODLevel == 1 && fLODLevel2Distance <= it->first)
			{
				byCUrrentLODLevel = 2;
				SelectIndexBuffer(2, &wPrimitiveCount, &ePrimitiveType);
			}

			__HardwareTransformPatch_RenderPatchNone(it->second, wPrimitiveCount, ePrimitiveType);

			if (m_iRenderedSplatNum >= m_iSplatLimit)
				break;

			if (m_bDrawWireFrame)
				DrawWireFrame(it->second, wPrimitiveCount, ePrimitiveType);
		}
	}

	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, dwFogEnable);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);

	std::sort(m_RenderedTextureNumVector.begin(), m_RenderedTextureNumVector.end());

	//////////////////////////////////////////////////////////////////////////
	// Render State & TextureStageState

	STATEMANAGER.RestoreRenderState(D3DRS_TEXTUREFACTOR);

	STATEMANAGER.RestoreTransform(D3DTS_TEXTURE0);
	STATEMANAGER.RestoreTransform(D3DTS_TEXTURE1);

	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_TEXCOORDINDEX);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS);
	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXCOORDINDEX);
	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS);

	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAREF);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);

	// Render State & TextureStageState
	//////////////////////////////////////////////////////////////////////////
}

void CMapOutdoor::__RenderTerrain_RenderHardwareTransformPatch2()
{
	m_matWorldForCommonUse._41 = 0.0f;
	m_matWorldForCommonUse._42 = 0.0f;
	STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matWorldForCommonUse);
	D3DXMATRIX worldMat;
	D3DXMatrixTranspose(&worldMat, &m_matWorldForCommonUse);
	STATEMANAGER.SetVertexShaderConstant(20, &worldMat, 4);

	STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL);

	m_iRenderedSplatNumSqSum = 0;
	m_iRenderedPatchNum = 0;
	m_iRenderedSplatNum = 0;
	m_RenderedTextureNumVector.clear();

	std::pair<float, long> fog_far(500000 + 1600.0f, 0);
	std::pair<float, long> fog_near(250000 - 3200.0f, 0);

	std::vector<std::pair<float, long> >::iterator far_it = std::upper_bound(m_PatchVector.begin(), m_PatchVector.end(), fog_far);
	std::vector<std::pair<float, long> >::iterator near_it = std::upper_bound(m_PatchVector.begin(), m_PatchVector.end(), fog_near);

	WORD wPrimitiveCount;
	D3DPRIMITIVETYPE ePrimitiveType;

	BYTE byCUrrentLODLevel = 0;

	float fLODLevel1Distance = __GetNoFogDistance();
	float fLODLevel2Distance = __GetFogDistance();

	SelectIndexBuffer(0, &wPrimitiveCount, &ePrimitiveType);

	std::vector<std::pair<float, long> >::iterator it = m_PatchVector.begin();

#ifndef WORLD_EDITOR
	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);

	for (; it != near_it; ++it)
	{
		if (byCUrrentLODLevel == 0 && fLODLevel1Distance <= it->first)
		{
			byCUrrentLODLevel = 1;
			SelectIndexBuffer(1, &wPrimitiveCount, &ePrimitiveType);
		}
		else if (byCUrrentLODLevel == 1 && fLODLevel2Distance <= it->first)
		{
			byCUrrentLODLevel = 2;
			SelectIndexBuffer(2, &wPrimitiveCount, &ePrimitiveType);
		}

		__HardwareTransformPatch_RenderPatchSplat2(it->second, wPrimitiveCount, ePrimitiveType);
		if (m_iRenderedSplatNum >= m_iSplatLimit)
			break;

		if (m_bDrawWireFrame)
			DrawWireFrame(it->second, wPrimitiveCount, ePrimitiveType);
	}
#endif

	if (m_iRenderedSplatNum < m_iSplatLimit)
	{
		for (it = near_it; it != far_it; ++it)
		{
			if (byCUrrentLODLevel == 0 && fLODLevel1Distance <= it->first)
			{
				byCUrrentLODLevel = 1;
				SelectIndexBuffer(1, &wPrimitiveCount, &ePrimitiveType);
			}
			else if (byCUrrentLODLevel == 1 && fLODLevel2Distance <= it->first)
			{
				byCUrrentLODLevel = 2;
				SelectIndexBuffer(2, &wPrimitiveCount, &ePrimitiveType);
			}

			__HardwareTransformPatch_RenderPatchSplat2(it->second, wPrimitiveCount, ePrimitiveType);

			if (m_iRenderedSplatNum >= m_iSplatLimit)
				break;

			if (m_bDrawWireFrame)
				DrawWireFrame(it->second, wPrimitiveCount, ePrimitiveType);
		}
	}

	if (m_iRenderedSplatNum < m_iSplatLimit)
	{
		for (it = far_it; it != m_PatchVector.end(); ++it)
		{
			if (byCUrrentLODLevel == 0 && fLODLevel1Distance <= it->first)
			{
				byCUrrentLODLevel = 1;
				SelectIndexBuffer(1, &wPrimitiveCount, &ePrimitiveType);
			}
			else if (byCUrrentLODLevel == 1 && fLODLevel2Distance <= it->first)
			{
				byCUrrentLODLevel = 2;
				SelectIndexBuffer(2, &wPrimitiveCount, &ePrimitiveType);
			}

			__HardwareTransformPatch_RenderPatchNone(it->second, wPrimitiveCount, ePrimitiveType);

			if (m_iRenderedSplatNum >= m_iSplatLimit)
				break;

			if (m_bDrawWireFrame)
				DrawWireFrame(it->second, wPrimitiveCount, ePrimitiveType);
		}
	}

	std::sort(m_RenderedTextureNumVector.begin(), m_RenderedTextureNumVector.end());
}

void CMapOutdoor::UpdateCharacterMovement()
{
	static D3DXVECTOR3 previousPosition = Frustum::Instance().characterPosition;

	D3DXVECTOR3 currentPosition = Frustum::Instance().characterPosition;

	const float epsilon = 0.0001f;

	bool isMoving =
		(fabs(currentPosition.x - previousPosition.x) > epsilon) ||
		(fabs(currentPosition.y - previousPosition.y) > epsilon) ||
		(fabs(currentPosition.z - previousPosition.z) > epsilon);

	float characterMoved = isMoving;

	STATEMANAGER.SetPixelShaderConstant(93, &previousPosition, 1);
	STATEMANAGER.SetPixelShaderConstant(94, &characterMoved, 1);

	if (isMoving)
	{
		if (Frustum::Instance().frameCounter % 8 == 0)
			previousPosition = currentPosition;
	}
}

void CMapOutdoor::__HardwareTransformPatch_RenderPatchSplat(long patchnum, WORD wPrimitiveCount, D3DPRIMITIVETYPE ePrimitiveType)
{
	assert(NULL != m_pTerrainPatchProxyList && "__HardwareTransformPatch_RenderPatchSplat");
	CTerrainPatchProxy* pTerrainPatchProxy = &m_pTerrainPatchProxyList[patchnum];

	if (!pTerrainPatchProxy->isUsed())
		return;

	long sPatchNum = pTerrainPatchProxy->GetPatchNum();
	if (sPatchNum < 0)
		return;

	BYTE ucTerrainNum = pTerrainPatchProxy->GetTerrainNum();
	if (0xFF == ucTerrainNum)
		return;

	CTerrain* pTerrain;
	if (!GetTerrainPointer(ucTerrainNum, &pTerrain))
		return;

	DWORD dwFogColor;
	if (mc_pEnvironmentData)
		dwFogColor = mc_pEnvironmentData->FogColor;
	else
		dwFogColor = 0xffffffff;

	WORD wCoordX, wCoordY;
	pTerrain->GetCoordinate(&wCoordX, &wCoordY);

	TTerrainSplatPatch& rTerrainSplatPatch = pTerrain->GetTerrainSplatPatch();

	D3DXMATRIX matTexTransform, matSplatAlphaTexTransform, matSplatColorTexTransform;
	m_matWorldForCommonUse._41 = -(float)(wCoordX * CTerrainImpl::TERRAIN_XSIZE);
	m_matWorldForCommonUse._42 = (float)(wCoordY * CTerrainImpl::TERRAIN_YSIZE);
	D3DXMatrixMultiply(&matTexTransform, &m_matViewInverse, &m_matWorldForCommonUse);
	D3DXMatrixMultiply(&matSplatAlphaTexTransform, &matTexTransform, &m_matSplatAlpha);
	STATEMANAGER.SetTransform(D3DTS_TEXTURE1, &matSplatAlphaTexTransform);

	D3DXMATRIX matTexTransformAlpha;
	D3DXMatrixMultiplyTranspose(&matTexTransformAlpha, &m_matWorldForCommonUse, &m_matSplatAlpha);
	STATEMANAGER.SetVertexShaderConstant(28, &matTexTransformAlpha, 4);

	D3DXMATRIX matTiling;
	D3DXMatrixScaling(&matTiling, 1.0f / 640.0f, -1.0f / 640.0f, 0.0f);
	matTiling._41 = 0.0f;
	matTiling._42 = 0.0f;

	D3DXMatrixMultiply(&matSplatColorTexTransform, &m_matViewInverse, &matTiling);
	STATEMANAGER.SetTransform(D3DTS_TEXTURE0, &matSplatColorTexTransform);

	CGraphicVertexBuffer* pkVB = pTerrainPatchProxy->HardwareTransformPatch_GetVertexBufferPtr();
	if (!pkVB)
		return;

	STATEMANAGER.SetStreamSource(0, pkVB->GetD3DVertexBuffer(), m_iPatchTerrainVertexSize);

	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

	auto& rFrustum = Frustum::Instance();
	const bool bUseCustomTerrainShader = (
		rFrustum.dynamicLight == 3 &&
		std::strcmp(rFrustum.currentMapNameSave.c_str(), "metin2_map_n_flame_01") != 0
	);
	D3DMATERIAL9 material;
	D3DLIGHT9 light;
	D3DVECTOR cameraPos;
	D3DXCOLOR fogColorX;
	D3DVECTOR fogProps;

	if (bUseCustomTerrainShader || (rFrustum.shadowType != 3 && !rFrustum.isIndoor))
	{
		D3DXMATRIX worldMat;
		STATEMANAGER.GetTransform(D3DTS_WORLD, &worldMat);
		D3DXMatrixTranspose(&worldMat, &worldMat);
		STATEMANAGER.SetVertexShaderConstant(20, &worldMat, 4);

		D3DXMATRIX viewProjMat, viewMat, projMat;
		STATEMANAGER.GetTransform(D3DTS_VIEW, &viewMat);
		STATEMANAGER.GetTransform(D3DTS_PROJECTION, &projMat);
		D3DXMatrixMultiplyTranspose(&viewProjMat, &viewMat, &projMat);
		STATEMANAGER.SetVertexShaderConstant(4, &viewProjMat, 4);

		STATEMANAGER.SetVertexShaderConstant(8, &m_MatLightViewProj, 4);
		STATEMANAGER.SetVertexShaderConstant(16, &m_MatLightViewProj2, 4);

		STATEMANAGER.GetMaterial(&material);
		STATEMANAGER.GetLight(0, &light);
		cameraPos = CCameraManager::instance().GetCurrentCamera()->GetEye();

		D3DCOLOR fogColor;
		STATEMANAGER.GetRenderState(D3DRS_FOGCOLOR, &fogColor);
		fogColorX = D3DXCOLOR(fogColor);
		STATEMANAGER.GetRenderState(D3DRS_FOGSTART, (DWORD*)&fogProps.x);
		STATEMANAGER.GetRenderState(D3DRS_FOGEND, (DWORD*)&fogProps.y);
	}

	int iPrevRenderedSplatNum = m_iRenderedSplatNum;

#ifdef WORLD_EDITOR
#else
	float fDepthBias = 0.00001f;
	ms_lpd3dDevice->SetRenderState(D3DRS_DEPTHBIAS, *(DWORD*)(&fDepthBias));
#if defined(ENABLE_ENVIRONMENT_EFFECT_OPTION)
#else

	if (bUseCustomTerrainShader)
	{
		STATEMANAGER.SetVertexShader(m_lpVertexShaderTerrainBase);
		STATEMANAGER.SetPixelShader(m_lpPixelShaderTerrainBase);

		STATEMANAGER.SetPixelShaderConstant(0, &material.Diffuse, 1);
		D3DXVECTOR3 sunPosition = current_v3SunPosition;
		sunPosition.z = sunPosition.z / 1.5f;
		STATEMANAGER.SetPixelShaderConstant(1, &sunPosition, 1);
		STATEMANAGER.SetPixelShaderConstant(2, &light.Diffuse, 1);
		STATEMANAGER.SetPixelShaderConstant(3, &light.Diffuse, 1);
		STATEMANAGER.SetPixelShaderConstant(4, &cameraPos, 1);
		STATEMANAGER.SetPixelShaderConstant(5, (float*)fogColorX, 1);
		STATEMANAGER.SetPixelShaderConstant(6, &fogProps, 1);
		STATEMANAGER.SetPixelShaderConstant(7, &material.Diffuse, 1);

		int specularLightEnabled = rFrustum.realtimeSpecular;
		STATEMANAGER.SetPixelShaderConstant(11, &specularLightEnabled, 1);

		float shiness;
		D3DXVECTOR3 specularLight;
		if (rFrustum.selectRainFrustun == 0)
		{
			shiness = 2.0f;
			specularLight = { 0.3f, 0.3f, 0.3f };
			STATEMANAGER.SetPixelShaderConstant(8, &shiness, 1);
		}
		else
		{
			if (rFrustum.isThunderTime == true)
			{
				specularLight = { 1.0f, 1.0f, 1.0f };
				STATEMANAGER.SetPixelShaderConstant(8, &shinessRain, 1);
			}
			else
			{
				shiness = 8.0f;
				specularLight = { 20.5f, 20.5f, 20.5f };
				STATEMANAGER.SetPixelShaderConstant(8, &shiness, 1);
			}
		}

		STATEMANAGER.SetPixelShaderConstant(9, &specularLight, 1);
		STATEMANAGER.SetPixelShaderConstant(10, &rFrustum.terrainLightPowerEnvBased, 1);
		STATEMANAGER.SetPixelShaderConstant(12, &time3, 1);

		float enableLights = 1.0f;
		STATEMANAGER.SetPixelShaderConstant(13, &enableLights, 1);

		float terrainLightMultiply = 1.75f;
		STATEMANAGER.SetPixelShaderConstant(14, &terrainLightMultiply, 1);

		float isNightEnvironment = rFrustum.isNightEnvironment;
		float isStormEnvironment = rFrustum.isStormEnvironment;
		STATEMANAGER.SetPixelShaderConstant(86, &rFrustum.characterPosition, 1);
		STATEMANAGER.SetPixelShaderConstant(87, &rFrustum.puddleThreshold, 1);
		STATEMANAGER.SetPixelShaderConstant(88, &rFrustum.kingdomMapSelector, 1);
		STATEMANAGER.SetPixelShaderConstant(89, &isNightEnvironment, 1);
		STATEMANAGER.SetPixelShaderConstant(90, &isStormEnvironment, 1);

		if (m_lpCharacterShadowMapTexture4)
			STATEMANAGER.SetTexture(3, m_lpCharacterShadowMapTexture4);
		STATEMANAGER.SetTexture(5, rFrustum.brown_mud_03_diff_2k);
		STATEMANAGER.SetTexture(6, rFrustum.dry_ground_01_diff_4k);
		STATEMANAGER.SetTexture(7, rFrustum.m_lpPuddleReflectionTexture);
		STATEMANAGER.SetTexture(8, rFrustum.foam);
	}

	bool isFirst = true;
	const DWORD dwNumTextures = pTerrain->GetNumTextures();
	for (DWORD j = 1; j < dwNumTextures; ++j)
	{
		TTerainSplat& rSplat = rTerrainSplatPatch.Splats[j];

		if (!rSplat.Active)
			continue;

		if (rTerrainSplatPatch.PatchTileCount[sPatchNum][j] == 0)
			continue;

		const TTerrainTexture& rTexture = m_TextureSet.GetTexture(j);

		if (!bUseCustomTerrainShader)
		{
			D3DXMatrixMultiply(&matSplatColorTexTransform, &m_matViewInverse, &rTexture.m_matTransform);
			STATEMANAGER.SetTransform(D3DTS_TEXTURE0, &matSplatColorTexTransform);
			if (isFirst)
			{
				STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
				STATEMANAGER.SetTexture(0, rTexture.pd3dTexture);
				STATEMANAGER.SetTexture(1, rSplat.pd3dTexture);
				STATEMANAGER.DrawIndexedPrimitive(ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);
				STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
				isFirst = false;
			}
			else
			{
				STATEMANAGER.SetTexture(0, rTexture.pd3dTexture);
				STATEMANAGER.SetTexture(1, rSplat.pd3dTexture);
				STATEMANAGER.DrawIndexedPrimitive(ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);
			}
		}
		else
		{
			D3DXMATRIX matTexTransformSplat;
			D3DXMatrixTranspose(&matTexTransformSplat, &rTexture.m_matTransform);
			STATEMANAGER.SetVertexShaderConstant(24, &matTexTransformSplat, 4);

			float alphaOverride[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			if (isFirst)
			{
				alphaOverride[0] = 1.0f;
				isFirst = false;
			}
			STATEMANAGER.SetPixelShaderConstant(15, alphaOverride, 1);

			LPDIRECT3DTEXTURE9 pNormTex = nullptr;
			LPDIRECT3DTEXTURE9 pSpecTex = nullptr;
			float uv = 1.0f;
			float textureSpecularPower = 18.0f;
			ResolveTerrainTextureShaderParams(j, rTexture, pNormTex, pSpecTex, uv, textureSpecularPower);

			STATEMANAGER.SetTexture(0, rTexture.pd3dTexture);
			STATEMANAGER.SetTexture(1, rSplat.pd3dTexture);
			STATEMANAGER.SetTexture(2, pNormTex);
			STATEMANAGER.SetTexture(4, pSpecTex);

			STATEMANAGER.SetPixelShaderConstant(17, &uv, 1);
			STATEMANAGER.SetPixelShaderConstant(18, &textureSpecularPower, 1);

			STATEMANAGER.DrawIndexedPrimitive(ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);
		}

		std::vector<int>::iterator aIterator = std::find(m_RenderedTextureNumVector.begin(), m_RenderedTextureNumVector.end(), (int)j);
		if (aIterator == m_RenderedTextureNumVector.end())
			m_RenderedTextureNumVector.push_back(j);
		++m_iRenderedSplatNum;
		if (m_iRenderedSplatNum >= m_iSplatLimit)
			break;
	}

	if (bUseCustomTerrainShader)
	{
		STATEMANAGER.SetTexture(2, nullptr);
		STATEMANAGER.SetTexture(3, nullptr);
		STATEMANAGER.SetTexture(4, nullptr);
		STATEMANAGER.SetTexture(5, nullptr);
		STATEMANAGER.SetTexture(6, nullptr);
		STATEMANAGER.SetTexture(7, nullptr);
		STATEMANAGER.SetTexture(8, nullptr);
		STATEMANAGER.SetVertexShader(nullptr);
		STATEMANAGER.SetPixelShader(nullptr);
	}

#endif
#endif // END_IFDEF_WORLD_EDITOR

	fDepthBias = -0.00003f;
	ms_lpd3dDevice->SetRenderState(D3DRS_DEPTHBIAS, *(DWORD*)(&fDepthBias));

	if (rFrustum.shadowType != 3 && rFrustum.isIndoor == false)
	{
		STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);

		STATEMANAGER.SetRenderState(D3DRS_FOGCOLOR, 0xFFFFFFFF);
		STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
		STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);

		STATEMANAGER.SetTexture(0, m_lpCharacterShadowMapTexture);

		STATEMANAGER.SetVertexShader(m_lpShadowMapVertexShaderTerrain);
		STATEMANAGER.SetPixelShader(m_lpShadowMapPixelShaderTerrain);

		STATEMANAGER.SetPixelShaderConstant(0, &material.Diffuse, 1);
		STATEMANAGER.SetPixelShaderConstant(1, &current_v3SunPosition, 1);
		STATEMANAGER.SetPixelShaderConstant(2, &light.Diffuse, 1);
		STATEMANAGER.SetPixelShaderConstant(3, &light.Diffuse, 1);
		STATEMANAGER.SetPixelShaderConstant(4, &cameraPos, 1);

		// Shadow pass uses multiplicative blending (ZERO, SRCCOLOR), so fog color in c5 MUST be white (0xFFFFFFFF)
		D3DXCOLOR whiteFogColor(0xFFFFFFFF);
		STATEMANAGER.SetPixelShaderConstant(5, (float*)whiteFogColor, 1);
		STATEMANAGER.SetPixelShaderConstant(6, &fogProps, 1);
		STATEMANAGER.SetPixelShaderConstant(7, &material.Diffuse, 1);

		float shadowOpacity = 55.0f;
		STATEMANAGER.SetPixelShaderConstant(10, (int*)&shadowOpacity, 1);

		int specularLightEnabled = rFrustum.realtimeSpecular;
		STATEMANAGER.SetPixelShaderConstant(11, &specularLightEnabled, 1);

		float shiness = 0.0f;
		D3DXVECTOR3 specularLight = { 0.0f, 0.0f, 0.0f };
		float reducePointLightIntensity = 15.0f;

		STATEMANAGER.SetPixelShaderConstant(8, &shiness, 1);
		STATEMANAGER.SetPixelShaderConstant(9, &specularLight, 1);
		STATEMANAGER.SetPixelShaderConstant(12, &time3, 1);

		float enableLights = 1.0f;
		STATEMANAGER.SetPixelShaderConstant(13, &enableLights, 1);

		float terrainLightMultiply = 1.75f;
		STATEMANAGER.SetPixelShaderConstant(14, &terrainLightMultiply, 1);
		STATEMANAGER.SetPixelShaderConstant(15, &reducePointLightIntensity, 1);

		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

		if (m_lpCharacterShadowMapTexture)
			STATEMANAGER.SetTexture(1, m_lpCharacterShadowMapTexture);
		if (m_lpCharacterShadowMapTexture4)
			STATEMANAGER.SetTexture(2, m_lpCharacterShadowMapTexture4);

		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

		STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

		ms_faceCount += wPrimitiveCount;
		STATEMANAGER.DrawIndexedPrimitive(ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);
		++m_iRenderedSplatNum;

		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

		STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		STATEMANAGER.SetRenderState(D3DRS_FOGCOLOR, dwFogColor);

		STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

		STATEMANAGER.SetVertexShader(nullptr);
		STATEMANAGER.SetPixelShader(nullptr);
	}
	++m_iRenderedPatchNum;

	int iCurRenderedSplatNum = m_iRenderedSplatNum - iPrevRenderedSplatNum;

	m_iRenderedSplatNumSqSum += iCurRenderedSplatNum * iCurRenderedSplatNum;

	fDepthBias = 0.0f;
	ms_lpd3dDevice->SetRenderState(D3DRS_DEPTHBIAS, *(DWORD*)(&fDepthBias));
}

void CMapOutdoor::__HardwareTransformPatch_RenderPatchSplat2(long patchnum, WORD wPrimitiveCount, D3DPRIMITIVETYPE ePrimitiveType)
{
	assert(NULL != m_pTerrainPatchProxyList && "__HardwareTransformPatch_RenderPatchSplat");
	CTerrainPatchProxy* pTerrainPatchProxy = &m_pTerrainPatchProxyList[patchnum];

	if (!pTerrainPatchProxy->isUsed())
		return;

	long sPatchNum = pTerrainPatchProxy->GetPatchNum();
	if (sPatchNum < 0)
		return;

	BYTE ucTerrainNum = pTerrainPatchProxy->GetTerrainNum();
	if (0xFF == ucTerrainNum)
		return;

	CTerrain* pTerrain;
	if (!GetTerrainPointer(ucTerrainNum, &pTerrain))
		return;

	DWORD dwFogColor;
	if (mc_pEnvironmentData)
		dwFogColor = mc_pEnvironmentData->FogColor;
	else
		dwFogColor = 0xffffffff;

	WORD wCoordX, wCoordY;
	pTerrain->GetCoordinate(&wCoordX, &wCoordY);

	TTerrainSplatPatch& rTerrainSplatPatch = pTerrain->GetTerrainSplatPatch();

	D3DXMATRIX matTexTransform, matSplatAlphaTexTransform, matSplatColorTexTransform;
	m_matWorldForCommonUse._41 = -(float)(wCoordX * CTerrainImpl::TERRAIN_XSIZE);
	m_matWorldForCommonUse._42 = (float)(wCoordY * CTerrainImpl::TERRAIN_YSIZE);
	D3DXMatrixMultiply(&matTexTransform, &m_matViewInverse, &m_matWorldForCommonUse);
	D3DXMatrixMultiply(&matSplatAlphaTexTransform, &matTexTransform, &m_matSplatAlpha);
	STATEMANAGER.SetTransform(D3DTS_TEXTURE1, &matSplatAlphaTexTransform);

	D3DXMATRIX matTiling;
	D3DXMatrixScaling(&matTiling, 1.0f / 640.0f, -1.0f / 640.0f, 0.0f);
	matTiling._41 = 0.0f;
	matTiling._42 = 0.0f;

	D3DXMatrixMultiply(&matSplatColorTexTransform, &m_matViewInverse, &matTiling);
	STATEMANAGER.SetTransform(D3DTS_TEXTURE0, &matSplatColorTexTransform);

	CGraphicVertexBuffer* pkVB = pTerrainPatchProxy->HardwareTransformPatch_GetVertexBufferPtr();
	if (!pkVB)
		return;

	STATEMANAGER.SetStreamSource(0, pkVB->GetD3DVertexBuffer(), m_iPatchTerrainVertexSize);

	int iPrevRenderedSplatNum = m_iRenderedSplatNum;

	TTerrainTexture& rTexture = m_TextureSet.GetTexture(2);

	D3DXMatrixMultiply(&matSplatColorTexTransform, &m_matViewInverse, &rTexture.m_matTransform);

	STATEMANAGER.SetTransform(D3DTS_TEXTURE0, &matSplatColorTexTransform);

	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	STATEMANAGER.SetTexture(0, rTexture.pd3dTexture);
	STATEMANAGER.SetTexture(1, nullptr);

	STATEMANAGER.DrawIndexedPrimitive(ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);
	++m_iRenderedPatchNum;

	int iCurRenderedSplatNum = m_iRenderedSplatNum - iPrevRenderedSplatNum;

	m_iRenderedSplatNumSqSum += iCurRenderedSplatNum * iCurRenderedSplatNum;
}

void CMapOutdoor::__HardwareTransformPatch_RenderPatchNone(long patchnum, WORD wPrimitiveCount, D3DPRIMITIVETYPE ePrimitiveType)
{
	assert(NULL != m_pTerrainPatchProxyList && "__HardwareTransformPatch_RenderPatchNone");
	CTerrainPatchProxy* pTerrainPatchProxy = &m_pTerrainPatchProxyList[patchnum];

	if (!pTerrainPatchProxy->isUsed())
		return;

	CGraphicVertexBuffer* pkVB = pTerrainPatchProxy->HardwareTransformPatch_GetVertexBufferPtr();
	if (!pkVB)
		return;

	STATEMANAGER.SetStreamSource(0, pkVB->GetD3DVertexBuffer(), m_iPatchTerrainVertexSize);
	STATEMANAGER.DrawIndexedPrimitive(ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);
}

void CMapOutdoor::__HardwareTransformPatch_RenderPatchSplatReflect(long patchnum, WORD wPrimitiveCount, D3DPRIMITIVETYPE ePrimitiveType)
{
	assert(NULL != m_pTerrainPatchProxyList && "__HardwareTransformPatch_RenderPatchSplat");
	CTerrainPatchProxy* pTerrainPatchProxy = &m_pTerrainPatchProxyList[patchnum];

	if (!pTerrainPatchProxy->isUsed())
		return;

	long sPatchNum = pTerrainPatchProxy->GetPatchNum();
	if (sPatchNum < 0)
		return;

	BYTE ucTerrainNum = pTerrainPatchProxy->GetTerrainNum();
	if (0xFF == ucTerrainNum)
		return;

	CTerrain* pTerrain;
	if (!GetTerrainPointer(ucTerrainNum, &pTerrain))
		return;

	DWORD dwFogColor;
	if (mc_pEnvironmentData)
		dwFogColor = mc_pEnvironmentData->FogColor;
	else
		dwFogColor = 0xffffffff;

	WORD wCoordX, wCoordY;
	pTerrain->GetCoordinate(&wCoordX, &wCoordY);

	TTerrainSplatPatch& rTerrainSplatPatch = pTerrain->GetTerrainSplatPatch();

	D3DXMATRIX matTexTransform, matSplatAlphaTexTransform, matSplatColorTexTransform;
	m_matWorldForCommonUse._41 = -(float)(wCoordX * CTerrainImpl::TERRAIN_XSIZE);
	m_matWorldForCommonUse._42 = (float)(wCoordY * CTerrainImpl::TERRAIN_YSIZE);
	D3DXMatrixMultiply(&matTexTransform, &m_matViewInverse, &m_matWorldForCommonUse);
	D3DXMatrixMultiply(&matSplatAlphaTexTransform, &matTexTransform, &m_matSplatAlpha);
	STATEMANAGER.SetTransform(D3DTS_TEXTURE1, &matSplatAlphaTexTransform);

	D3DXMATRIX matTiling;
	D3DXMatrixScaling(&matTiling, 1.0f / 640.0f, -1.0f / 640.0f, 0.0f);
	matTiling._41 = 0.0f;
	matTiling._42 = 0.0f;

	D3DXMatrixMultiply(&matSplatColorTexTransform, &m_matViewInverse, &matTiling);
	STATEMANAGER.SetTransform(D3DTS_TEXTURE0, &matSplatColorTexTransform);

	CGraphicVertexBuffer* pkVB = pTerrainPatchProxy->HardwareTransformPatch_GetVertexBufferPtr();
	if (!pkVB)
		return;

	STATEMANAGER.SetStreamSource(0, pkVB->GetD3DVertexBuffer(), m_iPatchTerrainVertexSize);

	int iPrevRenderedSplatNum = m_iRenderedSplatNum;

	TTerrainTexture& rTexture = m_TextureSet.GetTexture(10);

	D3DXMatrixMultiply(&matSplatColorTexTransform, &m_matViewInverse, &rTexture.m_matTransform);

	STATEMANAGER.SetTransform(D3DTS_TEXTURE0, &matSplatColorTexTransform);

	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	STATEMANAGER.SetTexture(0, rTexture.pd3dTexture);
	STATEMANAGER.SetTexture(1, nullptr);

	STATEMANAGER.DrawIndexedPrimitive(ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);
	++m_iRenderedPatchNum;

	int iCurRenderedSplatNum = m_iRenderedSplatNum - iPrevRenderedSplatNum;

	m_iRenderedSplatNumSqSum += iCurRenderedSplatNum * iCurRenderedSplatNum;
}

void CMapOutdoor::__RenderTerrain_RenderHardwareTransformPatchReflect()
{
	DWORD dwFogColor;
	float fFogFarDistance;
	float fFogNearDistance;
	if (mc_pEnvironmentData)
	{
		dwFogColor = mc_pEnvironmentData->FogColor;
		fFogNearDistance = mc_pEnvironmentData->GetFogNearDistance();
		fFogFarDistance = mc_pEnvironmentData->GetFogFarDistance();
	}
	else
	{
		dwFogColor = 0xffffffff;
		fFogNearDistance = 5000.0f;
		fFogFarDistance = 10000.0f;
	}

	STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHAREF, 0x00000000);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);

	STATEMANAGER.SaveRenderState(D3DRS_TEXTUREFACTOR, dwFogColor);

	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
#ifdef DIRECTX9
	STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
#else
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
#endif

	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
#ifdef DIRECTX9
	STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
#else
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
#endif

	if (Frustum::Instance().dynamicLight == 3)
	{
		STATEMANAGER.SetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		STATEMANAGER.SetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	}

	CSpeedTreeWrapper::ms_bSelfShadowOn = true;

	m_matWorldForCommonUse._41 = 0.0f;
	m_matWorldForCommonUse._42 = 0.0f;
	STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matWorldForCommonUse);
	D3DXMATRIX worldMat;
	D3DXMatrixTranspose(&worldMat, &m_matWorldForCommonUse);
	STATEMANAGER.SetVertexShaderConstant(20, &worldMat, 4);

	STATEMANAGER.SaveTransform(D3DTS_TEXTURE0, &m_matWorldForCommonUse);
	STATEMANAGER.SaveTransform(D3DTS_TEXTURE1, &m_matWorldForCommonUse);

#ifdef DIRECTX9
	STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL);
#else
	STATEMANAGER.SetVertexShader(D3DFVF_XYZ | D3DFVF_NORMAL);
#endif

	m_iRenderedSplatNumSqSum = 0;
	m_iRenderedPatchNum = 0;
	m_iRenderedSplatNum = 0;
	m_RenderedTextureNumVector.clear();

	std::pair<float, long> fog_far(fFogFarDistance + 1600.0f, 0);
	std::pair<float, long> fog_near(fFogNearDistance - 3200.0f, 0);

	std::vector<std::pair<float, long> >::iterator far_it = std::upper_bound(m_PatchVector.begin(), m_PatchVector.end(), fog_far);
	std::vector<std::pair<float, long> >::iterator near_it = std::upper_bound(m_PatchVector.begin(), m_PatchVector.end(), fog_near);

#ifdef WORLD_EDITOR
	near_it = m_PatchVector.begin();
	far_it = m_PatchVector.end();
#endif

	WORD wPrimitiveCount;
	D3DPRIMITIVETYPE ePrimitiveType;

	BYTE byCUrrentLODLevel = 0;

	float fLODLevel1Distance = __GetNoFogDistance();
	float fLODLevel2Distance = __GetFogDistance();

	SelectIndexBuffer(0, &wPrimitiveCount, &ePrimitiveType);

	DWORD dwFogEnable = STATEMANAGER.GetRenderState(D3DRS_FOGENABLE);
	std::vector<std::pair<float, long> >::iterator it = m_PatchVector.begin();

#ifndef WORLD_EDITOR
	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);

	for (; it != near_it; ++it)
	{
		if (byCUrrentLODLevel == 0 && fLODLevel1Distance <= it->first)
		{
			byCUrrentLODLevel = 1;
			SelectIndexBuffer(1, &wPrimitiveCount, &ePrimitiveType);
		}
		else if (byCUrrentLODLevel == 1 && fLODLevel2Distance <= it->first)
		{
			byCUrrentLODLevel = 2;
			SelectIndexBuffer(2, &wPrimitiveCount, &ePrimitiveType);
		}

		__HardwareTransformPatch_RenderPatchSplatReflect(it->second, wPrimitiveCount, ePrimitiveType);
		if (m_iRenderedSplatNum >= m_iSplatLimit)
			break;

		if (m_bDrawWireFrame)
			DrawWireFrame(it->second, wPrimitiveCount, ePrimitiveType);
	}
#endif

	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, dwFogEnable);

	if (m_iRenderedSplatNum < m_iSplatLimit)
	{
		for (it = near_it; it != far_it; ++it)
		{
			if (byCUrrentLODLevel == 0 && fLODLevel1Distance <= it->first)
			{
				byCUrrentLODLevel = 1;
				SelectIndexBuffer(1, &wPrimitiveCount, &ePrimitiveType);
			}
			else if (byCUrrentLODLevel == 1 && fLODLevel2Distance <= it->first)
			{
				byCUrrentLODLevel = 2;
				SelectIndexBuffer(2, &wPrimitiveCount, &ePrimitiveType);
			}

			__HardwareTransformPatch_RenderPatchSplatReflect(it->second, wPrimitiveCount, ePrimitiveType);

			if (m_iRenderedSplatNum >= m_iSplatLimit)
				break;

			if (m_bDrawWireFrame)
				DrawWireFrame(it->second, wPrimitiveCount, ePrimitiveType);
		}
	}

	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

	STATEMANAGER.SetTexture(0, NULL);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, FALSE);

	STATEMANAGER.SetTexture(1, NULL);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, FALSE);

	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	if (m_iRenderedSplatNum < m_iSplatLimit)
	{
		for (it = far_it; it != m_PatchVector.end(); ++it)
		{
			if (byCUrrentLODLevel == 0 && fLODLevel1Distance <= it->first)
			{
				byCUrrentLODLevel = 1;
				SelectIndexBuffer(1, &wPrimitiveCount, &ePrimitiveType);
			}
			else if (byCUrrentLODLevel == 1 && fLODLevel2Distance <= it->first)
			{
				byCUrrentLODLevel = 2;
				SelectIndexBuffer(2, &wPrimitiveCount, &ePrimitiveType);
			}

			__HardwareTransformPatch_RenderPatchNone(it->second, wPrimitiveCount, ePrimitiveType);

			if (m_iRenderedSplatNum >= m_iSplatLimit)
				break;

			if (m_bDrawWireFrame)
				DrawWireFrame(it->second, wPrimitiveCount, ePrimitiveType);
		}
	}

	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, 0);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);

	std::sort(m_RenderedTextureNumVector.begin(), m_RenderedTextureNumVector.end());

	STATEMANAGER.RestoreRenderState(D3DRS_TEXTUREFACTOR);

	STATEMANAGER.RestoreTransform(D3DTS_TEXTURE0);
	STATEMANAGER.RestoreTransform(D3DTS_TEXTURE1);

	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAREF);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);
}

void CMapOutdoor::__RenderTerrain_RenderHardwareTransformPatchOriginal()
{
	DWORD dwFogColor;
	float fFogFarDistance;
	float fFogNearDistance;
	if (mc_pEnvironmentData)
	{
		dwFogColor = mc_pEnvironmentData->FogColor;
		fFogNearDistance = mc_pEnvironmentData->GetFogNearDistance();
		fFogFarDistance = mc_pEnvironmentData->GetFogFarDistance();
	}
	else
	{
		dwFogColor = 0xffffffff;
		fFogNearDistance = 5000.0f;
		fFogFarDistance = 10000.0f;
	}

	//////////////////////////////////////////////////////////////////////////
	// Render State & TextureStageState

	STATEMANAGER.SaveTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
	STATEMANAGER.SaveTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
	STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
	STATEMANAGER.SaveTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);

	STATEMANAGER.SaveRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHAREF, 0x00000000);
	STATEMANAGER.SaveRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);

	STATEMANAGER.SaveRenderState(D3DRS_TEXTUREFACTOR, dwFogColor);

	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
#ifdef DIRECTX9
	STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
#else
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
#endif

	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
#ifdef DIRECTX9
	STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
#else
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
#endif

#ifdef WORLD_EDITOR
	if (GetAsyncKeyState(VK_CAPITAL))
	{
		CSpeedTreeWrapper::ms_bSelfShadowOn = false;
		STATEMANAGER.SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_GAUSSIANCUBIC);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_GAUSSIANCUBIC);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_GAUSSIANCUBIC);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_MINFILTER, D3DTEXF_GAUSSIANCUBIC);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTEXF_GAUSSIANCUBIC);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_MIPFILTER, D3DTEXF_GAUSSIANCUBIC);
	}
	else
	{
		CSpeedTreeWrapper::ms_bSelfShadowOn = true;
		STATEMANAGER.SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		STATEMANAGER.SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR);
		STATEMANAGER.SetTextureStageState(1, D3DTSS_MIPFILTER, D3DTEXF_LINEAR);
	}
#else
	CSpeedTreeWrapper::ms_bSelfShadowOn = true;
	STATEMANAGER.SetBestFiltering(0);
	STATEMANAGER.SetBestFiltering(1);
	STATEMANAGER.SetBestFiltering(2);
	STATEMANAGER.SetBestFiltering(3);
#endif

	m_matWorldForCommonUse._41 = 0.0f;
	m_matWorldForCommonUse._42 = 0.0f;
	STATEMANAGER.SetTransform(D3DTS_WORLD, &m_matWorldForCommonUse);

	STATEMANAGER.SaveTransform(D3DTS_TEXTURE0, &m_matWorldForCommonUse);
	STATEMANAGER.SaveTransform(D3DTS_TEXTURE1, &m_matWorldForCommonUse);

	// Render State & TextureStageState
	//////////////////////////////////////////////////////////////////////////

#ifdef DIRECTX9
	STATEMANAGER.SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL);
#else
	STATEMANAGER.SetVertexShader(D3DFVF_XYZ | D3DFVF_NORMAL);
#endif

	m_iRenderedSplatNumSqSum = 0;
	m_iRenderedPatchNum = 0;
	m_iRenderedSplatNum = 0;
	m_RenderedTextureNumVector.clear();

	std::pair<float, long> fog_far(fFogFarDistance + 1600.0f, 0);
	std::pair<float, long> fog_near(fFogNearDistance - 3200.0f, 0);

	std::vector<std::pair<float, long> >::iterator far_it = std::upper_bound(m_PatchVector.begin(), m_PatchVector.end(), fog_far);
	std::vector<std::pair<float, long> >::iterator near_it = std::upper_bound(m_PatchVector.begin(), m_PatchVector.end(), fog_near);

#ifdef WORLD_EDITOR
	near_it = m_PatchVector.begin();
	far_it = m_PatchVector.end();
#endif

	WORD wPrimitiveCount;
	D3DPRIMITIVETYPE ePrimitiveType;

	BYTE byCUrrentLODLevel = 0;

	float fLODLevel1Distance = __GetNoFogDistance();
	float fLODLevel2Distance = __GetFogDistance();

	SelectIndexBuffer(0, &wPrimitiveCount, &ePrimitiveType);

	DWORD dwFogEnable = STATEMANAGER.GetRenderState(D3DRS_FOGENABLE);
	std::vector<std::pair<float, long> >::iterator it = m_PatchVector.begin();

#ifndef WORLD_EDITOR
	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);

	for (; it != near_it; ++it)
	{
		if (byCUrrentLODLevel == 0 && fLODLevel1Distance <= it->first)
		{
			byCUrrentLODLevel = 1;
			SelectIndexBuffer(1, &wPrimitiveCount, &ePrimitiveType);
		}
		else if (byCUrrentLODLevel == 1 && fLODLevel2Distance <= it->first)
		{
			byCUrrentLODLevel = 2;
			SelectIndexBuffer(2, &wPrimitiveCount, &ePrimitiveType);
		}

		__HardwareTransformPatch_RenderPatchSplatOriginal(it->second, wPrimitiveCount, ePrimitiveType);
		if (m_iRenderedSplatNum >= m_iSplatLimit)
			break;

		if (m_bDrawWireFrame)
			DrawWireFrame(it->second, wPrimitiveCount, ePrimitiveType);
	}
#endif

	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, dwFogEnable);

	if (m_iRenderedSplatNum < m_iSplatLimit)
	{
		for (it = near_it; it != far_it; ++it)
		{
			if (byCUrrentLODLevel == 0 && fLODLevel1Distance <= it->first)
			{
				byCUrrentLODLevel = 1;
				SelectIndexBuffer(1, &wPrimitiveCount, &ePrimitiveType);
			}
			else if (byCUrrentLODLevel == 1 && fLODLevel2Distance <= it->first)
			{
				byCUrrentLODLevel = 2;
				SelectIndexBuffer(2, &wPrimitiveCount, &ePrimitiveType);
			}

			__HardwareTransformPatch_RenderPatchSplatOriginal(it->second, wPrimitiveCount, ePrimitiveType);

			if (m_iRenderedSplatNum >= m_iSplatLimit)
				break;

			if (m_bDrawWireFrame)
				DrawWireFrame(it->second, wPrimitiveCount, ePrimitiveType);
		}
	}

	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

	STATEMANAGER.SetTexture(0, NULL);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, FALSE);

	STATEMANAGER.SetTexture(1, NULL);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, FALSE);

	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	if (m_iRenderedSplatNum < m_iSplatLimit)
	{
		for (it = far_it; it != m_PatchVector.end(); ++it)
		{
			if (byCUrrentLODLevel == 0 && fLODLevel1Distance <= it->first)
			{
				byCUrrentLODLevel = 1;
				SelectIndexBuffer(1, &wPrimitiveCount, &ePrimitiveType);
			}
			else if (byCUrrentLODLevel == 1 && fLODLevel2Distance <= it->first)
			{
				byCUrrentLODLevel = 2;
				SelectIndexBuffer(2, &wPrimitiveCount, &ePrimitiveType);
			}

			__HardwareTransformPatch_RenderPatchNone(it->second, wPrimitiveCount, ePrimitiveType);

			if (m_iRenderedSplatNum >= m_iSplatLimit)
				break;

			if (m_bDrawWireFrame)
				DrawWireFrame(it->second, wPrimitiveCount, ePrimitiveType);
		}
	}

	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, dwFogEnable);
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);

	std::sort(m_RenderedTextureNumVector.begin(), m_RenderedTextureNumVector.end());

	//////////////////////////////////////////////////////////////////////////
	// Render State & TextureStageState

	STATEMANAGER.RestoreRenderState(D3DRS_TEXTUREFACTOR);

	STATEMANAGER.RestoreTransform(D3DTS_TEXTURE0);
	STATEMANAGER.RestoreTransform(D3DTS_TEXTURE1);

	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_TEXCOORDINDEX);
	STATEMANAGER.RestoreTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS);
	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXCOORDINDEX);
	STATEMANAGER.RestoreTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS);

	STATEMANAGER.RestoreRenderState(D3DRS_ALPHABLENDENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHATESTENABLE);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAREF);
	STATEMANAGER.RestoreRenderState(D3DRS_ALPHAFUNC);

	// Render State & TextureStageState
	//////////////////////////////////////////////////////////////////////////
}

void CMapOutdoor::__HardwareTransformPatch_RenderPatchSplatOriginal(long patchnum, WORD wPrimitiveCount, D3DPRIMITIVETYPE ePrimitiveType)
{
	assert(NULL != m_pTerrainPatchProxyList && "__HardwareTransformPatch_RenderPatchSplat");
	CTerrainPatchProxy* pTerrainPatchProxy = &m_pTerrainPatchProxyList[patchnum];

	if (!pTerrainPatchProxy->isUsed())
		return;

	long sPatchNum = pTerrainPatchProxy->GetPatchNum();
	if (sPatchNum < 0)
		return;

	BYTE ucTerrainNum = pTerrainPatchProxy->GetTerrainNum();
	if (0xFF == ucTerrainNum)
		return;

	CTerrain* pTerrain;
	if (!GetTerrainPointer(ucTerrainNum, &pTerrain))
		return;

	DWORD dwFogColor;
	if (mc_pEnvironmentData)
		dwFogColor = mc_pEnvironmentData->FogColor;
	else
		dwFogColor = 0xffffffff;

	WORD wCoordX, wCoordY;
	pTerrain->GetCoordinate(&wCoordX, &wCoordY);

	TTerrainSplatPatch& rTerrainSplatPatch = pTerrain->GetTerrainSplatPatch();

	D3DXMATRIX matTexTransform, matSplatAlphaTexTransform, matSplatColorTexTransform;
	m_matWorldForCommonUse._41 = -(float)(wCoordX * CTerrainImpl::TERRAIN_XSIZE);
	m_matWorldForCommonUse._42 = (float)(wCoordY * CTerrainImpl::TERRAIN_YSIZE);
	D3DXMatrixMultiply(&matTexTransform, &m_matViewInverse, &m_matWorldForCommonUse);
	D3DXMatrixMultiply(&matSplatAlphaTexTransform, &matTexTransform, &m_matSplatAlpha);
	STATEMANAGER.SetTransform(D3DTS_TEXTURE1, &matSplatAlphaTexTransform);

	D3DXMATRIX matTiling;
	D3DXMatrixScaling(&matTiling, 1.0f / 640.0f, -1.0f / 640.0f, 0.0f);
	matTiling._41 = 0.0f;
	matTiling._42 = 0.0f;

	D3DXMatrixMultiply(&matSplatColorTexTransform, &m_matViewInverse, &matTiling);
	STATEMANAGER.SetTransform(D3DTS_TEXTURE0, &matSplatColorTexTransform);

	CGraphicVertexBuffer* pkVB = pTerrainPatchProxy->HardwareTransformPatch_GetVertexBufferPtr();
	if (!pkVB)
		return;

	STATEMANAGER.SetStreamSource(0, pkVB->GetD3DVertexBuffer(), m_iPatchTerrainVertexSize);

	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

	int iPrevRenderedSplatNum = m_iRenderedSplatNum;

	bool isFirst = true;
	for (DWORD j = 1; j < pTerrain->GetNumTextures(); ++j)
	{
		TTerainSplat& rSplat = rTerrainSplatPatch.Splats[j];

		if (!rSplat.Active)
			continue;

		if (rTerrainSplatPatch.PatchTileCount[sPatchNum][j] == 0)
			continue;

		const TTerrainTexture& rTexture = m_TextureSet.GetTexture(j);

		D3DXMatrixMultiply(&matSplatColorTexTransform, &m_matViewInverse, &rTexture.m_matTransform);
		STATEMANAGER.SetTransform(D3DTS_TEXTURE0, &matSplatColorTexTransform);
		if (isFirst)
		{
#ifdef ENABLE_OPENGL
			STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
			STATEMANAGER.SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
#endif
			STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
			STATEMANAGER.SetTexture(0, rTexture.pd3dTexture);
			STATEMANAGER.SetTexture(1, rSplat.pd3dTexture);
			STATEMANAGER.DrawIndexedPrimitive(ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);
			STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
#ifdef ENABLE_OPENGL
			STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			STATEMANAGER.SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
#endif
			isFirst = false;
		}
		else
		{
			STATEMANAGER.SetTexture(0, rTexture.pd3dTexture);
			STATEMANAGER.SetTexture(1, rSplat.pd3dTexture);
			STATEMANAGER.DrawIndexedPrimitive(ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);
		}

		std::vector<int>::iterator aIterator = std::find(m_RenderedTextureNumVector.begin(), m_RenderedTextureNumVector.end(), (int)j);
		if (aIterator == m_RenderedTextureNumVector.end())
			m_RenderedTextureNumVector.push_back(j);
		++m_iRenderedSplatNum;
		if (m_iRenderedSplatNum >= m_iSplatLimit)
			break;
	}

	// Mode 0 Static + Dynamic Shadows
	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);

	STATEMANAGER.SetRenderState(D3DRS_FOGCOLOR, 0xFFFFFFFF);
	STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
	STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);

	D3DXMATRIX matShadowTexTransform;
	D3DXMatrixMultiply(&matShadowTexTransform, &matTexTransform, &m_matStaticShadow);

	STATEMANAGER.SetTransform(D3DTS_TEXTURE0, &matShadowTexTransform);
	STATEMANAGER.SetTexture(0, pTerrain->GetShadowTexture());

	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	STATEMANAGER.SetTransform(D3DTS_TEXTURE1, &m_matDynamicShadow);

	STATEMANAGER.SetTexture(1, m_lpCharacterShadowMapTextureOriginal);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	STATEMANAGER.SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	ms_faceCount += wPrimitiveCount;
	STATEMANAGER.DrawIndexedPrimitive(ePrimitiveType, 0, m_iPatchTerrainVertexCount, 0, wPrimitiveCount);
	++m_iRenderedSplatNum;

	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	STATEMANAGER.SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	STATEMANAGER.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	STATEMANAGER.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	STATEMANAGER.SetRenderState(D3DRS_FOGCOLOR, dwFogColor);

	STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

	++m_iRenderedPatchNum;

	int iCurRenderedSplatNum = m_iRenderedSplatNum - iPrevRenderedSplatNum;

	m_iRenderedSplatNumSqSum += iCurRenderedSplatNum * iCurRenderedSplatNum;
}
//martysama0134's fb0d8d5d0566583187a94f9a823b2075