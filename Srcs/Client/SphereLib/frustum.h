/* Copyright (C) John W. Ratcliff, 2001.
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright (C) John W. Ratcliff, 2001"
 */
#pragma once

 /***********************************************************************/
 /** FRUSTUM.H   : Represents a clipping frustum.                       */
 /**               You should replace this with your own more robust    */
 /**               view frustum clipper.                                */
 /**                                                                    */
 /**               Written by John W. Ratcliff jratcliff@att.net        */
 /***********************************************************************/

#include "vector.h"
#include "../EterBase/Singleton.h"
#include "set"
#include "deque"
#include "vector"

enum ViewState
{
	VS_INSIDE,   // completely inside the frustum.
	VS_PARTIAL,  // partially inside and partially outside the frustum.
	VS_OUTSIDE   // completely outside the frustum
};

class Frustum : public CSingleton<Frustum>
{
public:
	void BuildViewFrustum(D3DXMATRIX& mat);
	void BuildViewFrustum2(D3DXMATRIX& mat, float fNear, float fFar, float fFov, float fAspect, const D3DXVECTOR3& vCamera, const D3DXVECTOR3& vLook);
	ViewState ViewVolumeTest(const Vector3d& c_v3Center, const float c_fRadius) const;
	ViewState ViewVolumeTestEffects(const Vector3d& c_v3Center, const float c_fRadius) const;

	D3DXVECTOR3 characterPosition;
	std::string	currentMapNameSave;
	int terrainShadow;
	float kingdomMapSelector;
	float getLeaves = 1.0f;
	bool isIndoor = false;
#ifdef ENABLE_OPENGL
	int shadowType = 3;
	LPDIRECT3DTEXTURE9 shadowTexture = nullptr;
	LPDIRECT3DTEXTURE9 shadowTextureBlack = nullptr;
	bool isCerApus4Environment = false;
	bool isCerApus4EnvironmentAzrael = false;
	int dynamicLight = 0;
	int selectRainFrustun = 0;
	D3DXVECTOR3 previousSunPosition;
	bool isChangingSky = false;
	int realtimeSpecular = 0;
	int renderGrass = 0;
	bool isThunderTime = false;
	bool isThunderTimeTerrain = false;
	float specularRainPower = 15.0f;
	float specularTreePower = 3.5f;
	float specularTerrainPower = 3.5f;
	float shinessTerrain = 0.0f;
	int ambianceEffectsLevel = 0;
	float pointLightIntensity = 1.0f;
	int  treeShadow = 0;
#else
	int shadowType = 2; //jtxfix
	LPDIRECT3DTEXTURE9 shadowTexture = nullptr;
	LPDIRECT3DTEXTURE9 shadowTextureBlack = nullptr;
	bool isCerApus4Environment = false;
	bool isCerApus4EnvironmentAzrael = false;
	int dynamicLight = 1; //jtxfix
	int selectRainFrustun = 0; //jtxfix
	D3DXVECTOR3 previousSunPosition;
	bool isChangingSky = false;
	int realtimeSpecular = 1; //jtxfix
	int renderGrass = 1;
	bool isThunderTime = false;
	bool isThunderTimeTerrain = false;
	float specularRainPower = 15.0f;
	float specularTreePower = 3.5f;
	float specularTerrainPower = 3.5f;
	float shinessTerrain = 0.0f;
	int ambianceEffectsLevel = 0; //jtxfix
	float pointLightIntensity = 1.0f;
	int  treeShadow = 2; //jtxfix
#endif
	bool isSnowing = false;
	float increaseShadowRangeFrustum = 16.0f; //jtxfix
	bool refreshEffect = true;
	LPDIRECT3DTEXTURE9 specularTexture = nullptr;
	int rainEffect = 0;
	float windSpeed = 0.6f;

	LPDIRECT3DTEXTURE9 foam = nullptr;
	LPDIRECT3DTEXTURE9 marginFoam = nullptr;
	LPDIRECT3DTEXTURE9 c1_itemshop_normal = nullptr;
	LPDIRECT3DTEXTURE9 m_lpWaterReflectionTexture = nullptr;
	LPDIRECT3DTEXTURE9 m_lpPuddleReflectionTexture = nullptr;
	LPDIRECT3DTEXTURE9 TextureReflectionMap = nullptr;

	LPDIRECT3DTEXTURE9 m_smallTextures;

	LPDIRECT3DTEXTURE9 smalltexture1 = nullptr;
	LPDIRECT3DTEXTURE9 smalltexture2 = nullptr;
	LPDIRECT3DTEXTURE9 smalltexture3 = nullptr;
	LPDIRECT3DTEXTURE9 smalltexture4 = nullptr;
	LPDIRECT3DTEXTURE9 smalltexture5 = nullptr;
	LPDIRECT3DTEXTURE9 smalltexture6 = nullptr;
	LPDIRECT3DTEXTURE9 smalltexture7 = nullptr;

	float fSharpnessPower;
	float fDeathCamera;
	float bIsDead;
	float puddleThreshold = 1.0f;

	//V1.0.6
	bool grassTexturesInitialized = false;
	std::deque<D3DXMATRIX> grassMatrices;
	D3DXVECTOR3 lastGrassPosition = { FLT_MAX, FLT_MAX, FLT_MAX };
	D3DXVECTOR3 lastGrassShadowPosition = { FLT_MAX, FLT_MAX, FLT_MAX };
	std::deque<D3DXMATRIX>  grassShadowMatrices;
	int grassType;

	//grass textures V1.0.6
	LPDIRECT3DTEXTURE9 grass_map1_1 = nullptr;
	LPDIRECT3DTEXTURE9 grass_map1_2 = nullptr;
	LPDIRECT3DTEXTURE9 grass_map1_3 = nullptr;
	LPDIRECT3DTEXTURE9 grass_map1_4 = nullptr;
	LPDIRECT3DTEXTURE9 grass_grain_1 = nullptr;
	LPDIRECT3DTEXTURE9 grass_grain_2 = nullptr;
	LPDIRECT3DTEXTURE9 grass_flower_1 = nullptr;
	LPDIRECT3DTEXTURE9 grass_flower_2 = nullptr;

	D3DXVECTOR3 treeLightDirection;
	int bloodLevel;
	float isSnow;
	float isLava;
	int terrainDepthLevel;
	int frameCounter = 0;

	int graphicsLevel = 0;
	int graphicsWaterLevel = 0;
	int shadowRender = 0;

	float isNightEnvironment;
	float isStormEnvironment;

	D3DXVECTOR3 AmbientLight;

	//V1.0.4
	std::string currentEnvironmentFace;
	float terrainLightPowerEnvBased = 14.0f;
	float characterSpecularLightPower = 1.5f;
	D3DXVECTOR3 waterSunDirection;

	std::set<std::tuple<float, float, float>> waterPoints;

	//V1.0.5
	float isUnderwater = 0.0f;
	float underwaterCameraTransition = 0.0f;
	float checkUnderwaterPosition = 0.0f;
	bool isPlayingUnderwaterSound = false;
	bool isPlayingSurfaceSound = false;
	float waterLevel = 0.0f;
	float puddleLevel = 0.0f;
	float waterOpacity = 1.0f;

	float waterHeightNew;
	bool initialSpawn = true;

	//V1.0.4 Texture Declaration
	LPDIRECT3DTEXTURE9 normalMapTile01 = nullptr;
	LPDIRECT3DTEXTURE9 normalMapRacketDirt = nullptr;
	LPDIRECT3DTEXTURE9 normalMapGrass = nullptr;
	LPDIRECT3DTEXTURE9 normalMapStone = nullptr;
	LPDIRECT3DTEXTURE9 normalMapTree = nullptr;
	LPDIRECT3DTEXTURE9 normalMapSnow = nullptr;
	LPDIRECT3DTEXTURE9 normalMapBitch = nullptr;
	LPDIRECT3DTEXTURE9 normalMapField = nullptr;
	LPDIRECT3DTEXTURE9 normalMapSand01 = nullptr;
	LPDIRECT3DTEXTURE9 normalMapSand03 = nullptr;
	LPDIRECT3DTEXTURE9 normalMapLeaves = nullptr;
	LPDIRECT3DTEXTURE9 normalMapDryGroundRock = nullptr;
	LPDIRECT3DTEXTURE9 normalMapCobblestoneTemple1 = nullptr;
	LPDIRECT3DTEXTURE9 normalMapCobblestoneTemple2 = nullptr;
	LPDIRECT3DTEXTURE9 normalMapAerialGrass = nullptr;
	LPDIRECT3DTEXTURE9 normalMapSnow1 = nullptr;
	LPDIRECT3DTEXTURE9 normalMapSnow2 = nullptr;
	LPDIRECT3DTEXTURE9 normalMapSnow3 = nullptr;
	LPDIRECT3DTEXTURE9 normalMapSnow4 = nullptr;
	LPDIRECT3DTEXTURE9 normalMapSnowIce = nullptr;
	LPDIRECT3DTEXTURE9 forest_leaves_03_diff_1k = nullptr;
	LPDIRECT3DTEXTURE9 dirt_aerial_02_diff_1k = nullptr;
	LPDIRECT3DTEXTURE9 mud_cracked_dry_riverbed_002_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 rocky_trail_02_diff_1k = nullptr;
	LPDIRECT3DTEXTURE9 rock_wall_02_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 leafy_grass_diff_1k = nullptr;
	LPDIRECT3DTEXTURE9 mud_forest_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 grass_map1 = nullptr;
	LPDIRECT3DTEXTURE9 mossy_rock_diff_1k = nullptr;
	LPDIRECT3DTEXTURE9 brown_mud_leaves_01_diff_1k = nullptr;
	LPDIRECT3DTEXTURE9 cobblestone_large_01_diff_1k = nullptr;
	LPDIRECT3DTEXTURE9 cobblestone_05_diff_1k = nullptr;
	LPDIRECT3DTEXTURE9 playground_sand_diff_1k = nullptr;
	LPDIRECT3DTEXTURE9 monastery_stone_floor_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 cobblestone_floor_01_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 cobblestone_floor_02_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 concrete_rock_path_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 dirt_floor_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 forest_floor_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 forrest_ground_03_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 gray_rocks_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 grey_stone_path_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 leaves_forest_ground_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 rock_06_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 rock_boulder_cracked_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 rock_face_03_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 rustic_stone_wall_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 sand_02_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 ground_0019_color_2k = nullptr;
	LPDIRECT3DTEXTURE9 ground_0007_color_2k = nullptr;
	LPDIRECT3DTEXTURE9 sand_dunes = nullptr;
	LPDIRECT3DTEXTURE9 sandstone_cracks_diff_4k = nullptr;
	LPDIRECT3DTEXTURE9 dry_ground_01_diff_4k = nullptr;
	LPDIRECT3DTEXTURE9 ice_0002_color_2k = nullptr;
	LPDIRECT3DTEXTURE9 ground_0030_color_2k = nullptr;
	LPDIRECT3DTEXTURE9 ground_0031_color_2k = nullptr;
	LPDIRECT3DTEXTURE9 rock_0008_color_2k = nullptr;
	LPDIRECT3DTEXTURE9 grassy_cobblestone_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 aerial_rocks_04_diff_1k = nullptr;
	LPDIRECT3DTEXTURE9 coast_sand_rocks_02_diff_1k = nullptr;
	LPDIRECT3DTEXTURE9 brown_mud_03_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 grass_2k = nullptr;
	LPDIRECT3DTEXTURE9 slate_driveway_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 cracked_red_ground_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 concrete_floor_painted_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 burned_ground_01_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 brown_mud_dry_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 brown_mud_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 brown_mud_02_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 lichen_rock_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 lava1 = nullptr;
	LPDIRECT3DTEXTURE9 lava2 = nullptr;
	LPDIRECT3DTEXTURE9 Gravel014_2K = nullptr;
	LPDIRECT3DTEXTURE9 Ground033_2K = nullptr;
	LPDIRECT3DTEXTURE9 Ground035_2K = nullptr;
	LPDIRECT3DTEXTURE9 Ground044_2K = nullptr;
	LPDIRECT3DTEXTURE9 Ground049B_2K = nullptr;
	LPDIRECT3DTEXTURE9 Ground050_2K = nullptr;
	LPDIRECT3DTEXTURE9 Ground051_2K = nullptr;
	LPDIRECT3DTEXTURE9 Ground054_2K = nullptr;
	LPDIRECT3DTEXTURE9 Ground059_2K = nullptr;
	LPDIRECT3DTEXTURE9 Ground071_2K = nullptr;
	LPDIRECT3DTEXTURE9 Rock029_2K = nullptr;
	LPDIRECT3DTEXTURE9 ground013 = nullptr;
	LPDIRECT3DTEXTURE9 ground019 = nullptr;
	LPDIRECT3DTEXTURE9 ground030 = nullptr;
	LPDIRECT3DTEXTURE9 ground037 = nullptr;
	LPDIRECT3DTEXTURE9 ground069 = nullptr;
	LPDIRECT3DTEXTURE9 pavingstones009 = nullptr;
	LPDIRECT3DTEXTURE9 rock028 = nullptr;
	LPDIRECT3DTEXTURE9 rock051 = nullptr;

	LPDIRECT3DTEXTURE9 treeBarkTexture = nullptr;
	LPDIRECT3DTEXTURE9 treeBarkNormal = nullptr;
	LPDIRECT3DTEXTURE9 treeBarkSpecular = nullptr;
	LPDIRECT3DTEXTURE9 treeBarkDisplacement = nullptr;

	float treeNormalMap = 2.0f;
	float treeFinalColor = 55.0f;

	LPDIRECT3DTEXTURE9 forest_leaves_02_diffuse_2k = nullptr;
	LPDIRECT3DTEXTURE9 forest_leaves_04_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 forrest_ground_01_diff_2k = nullptr;
	LPDIRECT3DTEXTURE9 Ground018_2K = nullptr;
	LPDIRECT3DTEXTURE9 Ground023_2K = nullptr;
	LPDIRECT3DTEXTURE9 Ground024_2K = nullptr;
	LPDIRECT3DTEXTURE9 Ground038_2K = nullptr;
	LPDIRECT3DTEXTURE9 Ground040_2K = nullptr;
	LPDIRECT3DTEXTURE9 Ground042_2K = nullptr;
	LPDIRECT3DTEXTURE9 Ground067_2K = nullptr;
	LPDIRECT3DTEXTURE9 Ground068_2K = nullptr;

	LPDIRECT3DTEXTURE9 emptyNormalMap = nullptr;

	//V1.0.4c Texture Specular
	LPDIRECT3DTEXTURE9 waterSandSpecularTexture = nullptr;
	LPDIRECT3DTEXTURE9 waterLeavesSpecularTexture = nullptr;
	LPDIRECT3DTEXTURE9 iceSpecularTexture = nullptr;
	LPDIRECT3DTEXTURE9 sandSpecularTexture = nullptr;

	LPDIRECT3DTEXTURE9 displacementTexture = nullptr;

private:
	bool m_bUsingSphere;
	D3DXVECTOR3 m_v3Center;
	float m_fRadius;
	D3DXPLANE m_plane[6];
};
//martysama0134's fb0d8d5d0566583187a94f9a823b2075