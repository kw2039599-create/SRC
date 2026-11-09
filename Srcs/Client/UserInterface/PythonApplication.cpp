#include "StdAfx.h"
#include "../eterBase/Error.h"
#include "../eterlib/Camera.h"
#include "../eterlib/AttributeInstance.h"
#include "../gamelib/AreaTerrain.h"
#include "../EterGrnLib/Material.h"
#include "../CWebBrowser/CWebBrowser.h"
#include <float.h>
#include "resource.h"
#include "PythonApplication.h"
#ifdef __ANDROID__
#include <Python27/frameobject.h>
#endif
#include "PythonCharacterManager.h"
#ifdef ENABLE_RENDER_TARGET
#include "CRenderTargetManager.h"
#endif
#include "ProcessScanner.h"

#include "CheckLatestFiles.h"
#ifdef ENABLE_CSHIELD
#include "CShield.h"
#include "CShieldCheckIntegrity.h"
#endif
#ifdef ENABLE_SWITCHBOT
#include "PythonSwitchbot.h"
#endif
#ifdef ATTACK_GUARD
void CPythonApplication::InitializeAttackGuard()
{
	m_dwLastAttackGuardCheck = 0;
}

void CPythonApplication::SecurityCheck()
{
	// Handled by CShield and server-side QX7_MoveGuard / MHX2_AttackGuard
	// (Keeps the binary 100% free of Toolhelp32 / Debugger / XOR heuristics that trigger Windows Defender Bearfoos!ml)
	m_dwLastAttackGuardCheck = ELTimer_GetMSec();
}
#endif


#ifdef ANTI_SPLASH
#include <windows.h>
#include "resource.h" // Ensure resource.h is included to access IDB_ANTI_SPLASH

// Global pointers for the splash window and image
HWND g_hAntiCheatSplash = NULL;
HBITMAP g_hSplashBitmap = NULL;

void ShowAntiCheatSplash(HINSTANCE hInstance)
{
	// 1. Load the BMP directly from the compiled .exe resources
	g_hSplashBitmap = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(IDB_ANTI_SPLASH), IMAGE_BITMAP, 0, 0, 0); 
	
	// If the image fails to load for any reason, silently cancel the splash.
	// The game will continue loading in the background and pop up when ready!
	if (!g_hSplashBitmap)
	{
		return;
	}

	// 2. Automatically retrieve image dimensions
	BITMAP bmp;
	GetObject(g_hSplashBitmap, sizeof(BITMAP), &bmp);
	int imgWidth = bmp.bmWidth;
	int imgHeight = bmp.bmHeight;

	// 3. Pin to the bottom-right corner (clearing the taskbar)
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int posX = screenWidth - imgWidth - 20;
	int posY = screenHeight - imgHeight - 60;

	// 4. Create a topmost, borderless layered tool window
	g_hAntiCheatSplash = CreateWindowEx(
		WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED,
		"STATIC", NULL,
		WS_POPUP | WS_VISIBLE | SS_BITMAP,
		posX, posY, imgWidth, imgHeight,
		NULL, NULL, hInstance, NULL
	);

	if (!g_hAntiCheatSplash)
		return;

	// 5. Apply the image and make pure magenta (RGB 255, 0, 255) transparent
	SendMessage(g_hAntiCheatSplash, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)g_hSplashBitmap);
	SetLayeredWindowAttributes(g_hAntiCheatSplash, RGB(255, 0, 255), 255, LWA_COLORKEY);

	// 6. Force immediate paint
	UpdateWindow(g_hAntiCheatSplash);
}

void HideAntiCheatSplash()
{
	if (g_hAntiCheatSplash)
	{
		DestroyWindow(g_hAntiCheatSplash);
		g_hAntiCheatSplash = NULL;
	}

	if (g_hSplashBitmap)
	{
		DeleteObject(g_hSplashBitmap);
		g_hSplashBitmap = NULL;
	}
}
#endif

extern void GrannyCreateSharedDeformBuffer();
extern void GrannyDestroySharedDeformBuffer();

float MIN_FOG = 2400.0f;
double g_specularSpd=0.007f;

CPythonApplication * CPythonApplication::ms_pInstance;

float c_fDefaultCameraRotateSpeed = 1.5f;
float c_fDefaultCameraPitchSpeed = 1.5f;
float c_fDefaultCameraZoomSpeed = 0.05f;

CPythonApplication::CPythonApplication() :
m_bCursorVisible(TRUE),
m_bLiarCursorOn(false),
m_iCursorMode(CURSOR_MODE_HARDWARE),
m_isWindowed(false),
m_isFrameSkipDisable(false),
m_poMouseHandler(NULL),
m_dwUpdateFPS(0),
m_dwRenderFPS(0),
m_fAveRenderTime(0.0f),
m_dwFaceCount(0),
m_fGlobalTime(0.0f),
m_fGlobalElapsedTime(0.0f),
m_dwLButtonDownTime(0),
m_dwLastIdleTime(0)
{
	m_bIsCameraStatic = false;
	m_bIsFirstPersonMode = false;
	m_fPrevCameraDistance = 1550.0f;
#ifndef _DEBUG
	SetEterExceptionHandler();
#endif

	CTimer::Instance().UseCustomTime();


	m_dwWidth = 800;
	m_dwHeight = 600;

	ms_pInstance = this;
	m_isWindowFullScreenEnable = FALSE;

	m_v3CenterPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_dwStartLocalTime = ELTimer_GetMSec();
	m_tServerTime = 0;
	m_tLocalStartTime = 0;

	m_iPort = 0;
	m_iFPS = 60;

	m_isActivateWnd = false;
	m_isMinimizedWnd = true;

	m_fRotationSpeed = 0.0f;
	m_fPitchSpeed = 0.0f;
	m_fZoomSpeed = 0.0f;

	m_fFaceSpd=0.0f;

	m_dwFaceAccCount=0;
	m_dwFaceAccTime=0;

	m_dwFaceSpdSum=0;
	m_dwFaceSpdCount=0;

	m_FlyingManager.SetMapManagerPtr(&m_pyBackground);

	m_iCursorNum = CURSOR_SHAPE_NORMAL;
	m_iContinuousCursorNum = CURSOR_SHAPE_NORMAL;

	m_isSpecialCameraMode = FALSE;
	m_fCameraRotateSpeed = c_fDefaultCameraRotateSpeed;
	m_fCameraPitchSpeed = c_fDefaultCameraPitchSpeed;
	m_fCameraZoomSpeed = c_fDefaultCameraZoomSpeed;

	m_iCameraMode = CAMERA_MODE_NORMAL;
	m_fBlendCameraStartTime = 0.0f;
	m_fBlendCameraBlendTime = 0.0f;

	m_iForceSightRange = -1;

	CCameraManager::Instance().AddCamera(EVENT_CAMERA_NUMBER);
}

CPythonApplication::~CPythonApplication()
{
}

void CPythonApplication::GetMousePosition(POINT* ppt)
{
	CMSApplication::GetMousePosition(ppt);
}

void CPythonApplication::SetMinFog(float fMinFog)
{
	MIN_FOG = fMinFog;
}

void CPythonApplication::SetFrameSkip(bool isEnable)
{
	if (isEnable)
		m_isFrameSkipDisable=false;
	else
		m_isFrameSkipDisable=true;
}

#ifndef ENABLE_CSHIELD
void CPythonApplication::NotifyHack(const char* c_szFormat, ...)
{
	char szBuf[1024];

	va_list args;
	va_start(args, c_szFormat);	
	_vsnprintf(szBuf, sizeof(szBuf), c_szFormat, args);
	va_end(args);
	m_pyNetworkStream.NotifyHack(szBuf);
}
#else
void CPythonApplication::SetWindowInvisible(bool invisible)
{
	if (invisible)
		SetWindowDisplayAffinity(GetWindowHandle(), WDA_MONITOR);
	else
		SetWindowDisplayAffinity(GetWindowHandle(), WDA_NONE);
}

bool CPythonApplication::IsWindowVisisble()
{
	DWORD affinity = 0;
	if (GetWindowDisplayAffinity(GetWindowHandle(), &affinity))
		if (affinity & WDA_NONE)
			return true;

	return false;
}
#endif

void CPythonApplication::GetInfo(UINT eInfo, std::string* pstInfo)
{
	switch (eInfo)
	{
	case INFO_ACTOR:
		m_kChrMgr.GetInfo(pstInfo);
		break;
	case INFO_EFFECT:
		m_kEftMgr.GetInfo(pstInfo);
		break;
	case INFO_ITEM:
		m_pyItem.GetInfo(pstInfo);
		break;
	case INFO_TEXTTAIL:
		m_pyTextTail.GetInfo(pstInfo);
		break;
	}
}

void CPythonApplication::Abort()
{
	TraceError("============================================================================================================");
	TraceError("Abort!!!!\n\n");

	PostQuitMessage(0);
}

void CPythonApplication::Exit()
{
	PostQuitMessage(0);
}

bool PERF_CHECKER_RENDER_GAME = false;

void CPythonApplication::RenderGame()
{
#if defined(ENABLE_OPENGL) && !defined(__ANDROID__)
	Frustum::Instance().graphicsLevel = 0;
	Frustum::Instance().graphicsWaterLevel = 0;
	Frustum::Instance().shadowType = 3;
	Frustum::Instance().dynamicLight = 0;
	Frustum::Instance().realtimeSpecular = 0;
	Frustum::Instance().renderGrass = 0;
	Frustum::Instance().treeShadow = 0;
#endif
	if (!PERF_CHECKER_RENDER_GAME)
	{
		Frustum::Instance().frameCounter++;

		if (Frustum::Instance().frameCounter > 100)
			Frustum::Instance().frameCounter = 0;

		static DWORD s_dwLastPuddleTime = ELTimer_GetMSec();
		DWORD dwCurPuddleTime = ELTimer_GetMSec();
		float fPuddleDeltaSec = (dwCurPuddleTime > s_dwLastPuddleTime) ? ((dwCurPuddleTime - s_dwLastPuddleTime) * 0.001f) : 0.0f;
		s_dwLastPuddleTime = dwCurPuddleTime;

		// Clamp delta time so loading screens or alt-tabbing don't jump the puddle state
		if (fPuddleDeltaSec > 0.25f)
			fPuddleDeltaSec = 0.25f;

		const float fAccumulateRatePerSec = 1.0f / 180.0f; // 3 minutes (180s) from dry (1.0) to full coverage (0.0)
		const float fDrainRatePerSec      = 1.0f / 60.0f;  // 1 minute (60s) from full coverage (0.0) to dry (1.0)

		if (Frustum::Instance().selectRainFrustun == 0)
		{
			// Dry climate: drain completely to 1.0f over 1 minute (puddles disabled at >= 1.0f)
			if (Frustum::Instance().puddleThreshold < 1.0f)
			{
				Frustum::Instance().puddleThreshold += fDrainRatePerSec * fPuddleDeltaSec;
				if (Frustum::Instance().puddleThreshold > 1.0f)
					Frustum::Instance().puddleThreshold = 1.0f;
			}
		}
		else if (Frustum::Instance().selectRainFrustun == 8)
		{
			// Light rain (امطار خفيفة): target 0.88f
			const float fLightRainTarget = 0.88f;
			if (Frustum::Instance().puddleThreshold > fLightRainTarget)
			{
				Frustum::Instance().puddleThreshold -= fAccumulateRatePerSec * fPuddleDeltaSec;
				if (Frustum::Instance().puddleThreshold < fLightRainTarget)
					Frustum::Instance().puddleThreshold = fLightRainTarget;
			}
			else if (Frustum::Instance().puddleThreshold < fLightRainTarget)
			{
				Frustum::Instance().puddleThreshold += fDrainRatePerSec * fPuddleDeltaSec;
				if (Frustum::Instance().puddleThreshold > fLightRainTarget)
					Frustum::Instance().puddleThreshold = fLightRainTarget;
			}
		}
		else
		{
			// Heavy rain: accumulate slowly to 0.0f over 3 minutes
			if (Frustum::Instance().puddleThreshold > 0.0f)
			{
				Frustum::Instance().puddleThreshold -= fAccumulateRatePerSec * fPuddleDeltaSec;
				if (Frustum::Instance().puddleThreshold < 0.0f)
					Frustum::Instance().puddleThreshold = 0.0f;
			}
		}

		float fAspect = m_kWndMgr.GetAspect();
		float fFarClip = m_pyBackground.GetFarClip();
		float fNearClip = m_bIsFirstPersonMode ? 10.0f : 120.0f;
#ifdef ENABLE_FOV_OPTION
		float fFOV = m_pySystem.GetFOVLevel();
		if (m_bIsFirstPersonMode && fFOV < 70.0f)
			fFOV = 70.0f;
#else
		float fFOV = m_bIsFirstPersonMode ? 70.0f : 30.0f;
#endif

		m_pyGraphic.SetPerspective(fFOV, fAspect, fNearClip, fFarClip);

		CCullingManager::Instance().Process();

		m_kChrMgr.Deform();

#ifdef ENABLE_MINI_GAME_YUTNORI
		m_kYutNoriManager.DeformModel();
#endif

// 1. Render offscreen reflections & shadow maps BEFORE starting PostProcessing or pushing main game state
		if (Frustum::instance().graphicsWaterLevel == 2)
		{
			m_pyBackground.GetUnderwaterCamera();
#ifdef __ANDROID__
			if (m_pyBackground.GetMapOutdoorRef().IsWaterVisible())
			{
				m_pyBackground.RenderWaterReflection();
			}
			if (Frustum::Instance().puddleThreshold < 1.0f)
			{
				m_pyBackground.RenderPuddleReflection();
			}
#else
			m_pyBackground.RenderWaterReflection();

			if (Frustum::Instance().puddleThreshold < 1.0f)
			{
				m_pyBackground.RenderPuddleReflection();
			}
#endif
		}

		STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		m_pyBackground.RenderCharacterShadowToTexture();
		if (Frustum::instance().graphicsLevel != 0)
		{
			m_pyBackground.RenderCharacterShadowToTexture3();
			m_pyBackground.RenderCharacterShadowToTexture4();
		}
		else
		{
			m_pyBackground.RenderCharacterShadowToTextureOriginal();
		}
		STATEMANAGER.SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);


		// 2. Begin PostProcessing scene after offscreen shadow passes are done
#if !defined(DISABLE_JTX_DYNAMIC_LIGHT) && !defined(__ANDROID__)
		if (Frustum::instance().graphicsLevel != 0)
		{
			GetPostProcessingChain().BeginScene();
		}
#endif

		// 3. Re-establish clean game render state, camera view/proj matrices, and buffers after offscreen passes
		STATEMANAGER.DisableClipPlane(0);
		STATEMANAGER.SetVertexShader(nullptr);
		STATEMANAGER.SetPixelShader(nullptr);

		if (CCamera* pCurCam = CCameraManager::Instance().GetCurrentCamera())
		{
			pCurCam->SetViewParams(pCurCam->GetEye(), pCurCam->GetTarget(), pCurCam->GetUp());
			STATEMANAGER.SetTransform(D3DTS_VIEW, &pCurCam->GetViewMatrix());
		}

		m_pyGraphic.SetGameRenderState();
		m_pyGraphic.PushState();
		m_pyGraphic.SetPerspective(fFOV, fAspect, fNearClip, fFarClip);
		m_pyGraphic.Clear(); // Clears both Target and Depth on the PostProcessing surface so it can never freeze

		{
			long lx, ly;
			m_kWndMgr.GetMousePosition(lx, ly);
			m_pyGraphic.SetCursorPosition(lx, ly);
		}

		m_pyBackground.RenderSky();
		m_pyBackground.RenderBeforeLensFlare();
		m_pyBackground.RenderCloud();

		m_pyBackground.BeginEnvironment();
#if defined(__BL_FOG_FIX__)
		if (!m_pySystem.GetFogMode())
		{
			const float fFogOffStart = 500000.0f;
			const float fFogOffEnd = 1000000.0f;
			STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);
			STATEMANAGER.SetRenderState(D3DRS_FOGSTART, *((DWORD*)&fFogOffStart));
			STATEMANAGER.SetRenderState(D3DRS_FOGEND, *((DWORD*)&fFogOffEnd));
		}
#endif
		m_pyBackground.Render();

		m_pyBackground.SetCharacterDirLight();

		CInstanceBase* pkMainIns = m_pyPlayer.NEW_GetMainActorPtr();
		if (m_bIsFirstPersonMode && pkMainIns)
			pkMainIns->GetGraphicThingInstanceRef().Hide();

		m_kChrMgr.Render();

		if (m_bIsFirstPersonMode && pkMainIns)
			pkMainIns->GetGraphicThingInstanceRef().Show();

		CPythonPlayer::Instance().RenderAutoHuntCircle();

		m_pyBackground.SetBackgroundDirLight();
		m_pyBackground.RenderWater();
		m_pyBackground.RenderSnow();

		// Prevent fixed-function fog & point filtering from washing out / pixelating distant map effects in Mode 0
		STATEMANAGER.SaveRenderState(D3DRS_FOGENABLE, FALSE);
		STATEMANAGER.SaveSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		STATEMANAGER.SaveSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		STATEMANAGER.SaveSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

		m_pyBackground.RenderEffect();

		STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MINFILTER);
		STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MAGFILTER);
		STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MIPFILTER);
		STATEMANAGER.RestoreRenderState(D3DRS_FOGENABLE);

		m_pyBackground.EndEnvironment();

#ifdef ENABLE_IKASHOP_RENEWAL
		m_pyOfflineshop.RenderEntities();
#endif

		// Disable hardware fog & force linear mip-filtering on particles/effects so Mode 0 matches Level 3 clarity
		STATEMANAGER.SaveRenderState(D3DRS_FOGENABLE, FALSE);
		STATEMANAGER.SaveSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		STATEMANAGER.SaveSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		STATEMANAGER.SaveSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
		STATEMANAGER.SaveSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		STATEMANAGER.SaveSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		STATEMANAGER.SaveSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

		m_kEftMgr.Render();
		m_FlyingManager.Render();

		STATEMANAGER.RestoreSamplerState(1, D3DSAMP_MINFILTER);
		STATEMANAGER.RestoreSamplerState(1, D3DSAMP_MAGFILTER);
		STATEMANAGER.RestoreSamplerState(1, D3DSAMP_MIPFILTER);
		STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MINFILTER);
		STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MAGFILTER);
		STATEMANAGER.RestoreSamplerState(0, D3DSAMP_MIPFILTER);
		STATEMANAGER.RestoreRenderState(D3DRS_FOGENABLE);

		m_pyItem.Render();

		m_pyBackground.BeginEnvironment();
#if defined(__BL_FOG_FIX__)
		if (!m_pySystem.GetFogMode())
		{
			const float fFogOffStart = 500000.0f;
			const float fFogOffEnd = 1000000.0f;
			STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);
			STATEMANAGER.SetRenderState(D3DRS_FOGSTART, *((DWORD*)&fFogOffStart));
			STATEMANAGER.SetRenderState(D3DRS_FOGEND, *((DWORD*)&fFogOffEnd));
		}
#endif
		m_pyBackground.RenderPCBlocker();
		m_pyBackground.EndEnvironment();

		m_pyBackground.RenderAfterLensFlare();
#ifdef __ANDROID__
		m_pyBackground.GetMapOutdoorRef().RenderScreenFiltering();
#endif

#if !defined(DISABLE_JTX_DYNAMIC_LIGHT) && !defined(__ANDROID__)
		if (Frustum::instance().graphicsLevel != 0)
		{
			GetPostProcessingChain().Render();
			GetPostProcessingChain().EndScene();
		}
#endif
		SAFE_RELEASE(Frustum::Instance().TextureReflectionMap);

#if defined(ENABLE_RENDER_TARGET) && !defined(__ANDROID__)
		CRenderTargetManager::Instance().RenderBackgrounds();
		CRenderTargetManager::Instance().RenderModels();
#endif

		return;
	}

	DWORD t1=ELTimer_GetMSec();
	m_kChrMgr.Deform();
	DWORD t2=ELTimer_GetMSec();
	DWORD t3=ELTimer_GetMSec();
	m_pyBackground.RenderCharacterShadowToTexture();
	DWORD t4=ELTimer_GetMSec();

	m_pyGraphic.SetGameRenderState();
	m_pyGraphic.PushState();

	float fAspect=m_kWndMgr.GetAspect();
	float fFarClip=m_pyBackground.GetFarClip();

	m_pyGraphic.SetPerspective(30.0f, fAspect, 100.0, fFarClip);

	DWORD t5=ELTimer_GetMSec();

	CCullingManager::Instance().Process();

	DWORD t6=ELTimer_GetMSec();

	{
		long lx, ly;
		m_kWndMgr.GetMousePosition(lx, ly);
		m_pyGraphic.SetCursorPosition(lx, ly);
	}

	m_pyBackground.RenderSky();
	DWORD t7=ELTimer_GetMSec();
	m_pyBackground.RenderBeforeLensFlare();
	DWORD t8=ELTimer_GetMSec();
	m_pyBackground.RenderCloud();
	DWORD t9=ELTimer_GetMSec();
	m_pyBackground.BeginEnvironment();
	m_pyBackground.Render();

	m_pyBackground.SetCharacterDirLight();
	DWORD t10=ELTimer_GetMSec();
	m_kChrMgr.Render();
	CPythonPlayer::Instance().RenderAutoHuntCircle();
	// ---------------------------
	DWORD t11=ELTimer_GetMSec();

	m_pyBackground.SetBackgroundDirLight();
	m_pyBackground.RenderWater();
	DWORD t12=ELTimer_GetMSec();
	m_pyBackground.RenderEffect();
	DWORD t13=ELTimer_GetMSec();
	m_pyBackground.EndEnvironment();
	m_kEftMgr.Render();
	DWORD t14=ELTimer_GetMSec();
	m_pyItem.Render();
	DWORD t15=ELTimer_GetMSec();
	m_FlyingManager.Render();
	DWORD t16=ELTimer_GetMSec();
	m_pyBackground.BeginEnvironment();
	m_pyBackground.RenderPCBlocker();
	m_pyBackground.EndEnvironment();
	DWORD t17=ELTimer_GetMSec();
	m_pyBackground.RenderAfterLensFlare();
	DWORD t18=ELTimer_GetMSec();
	DWORD tEnd=ELTimer_GetMSec();
#if defined(ENABLE_RENDER_TARGET) && !defined(__ANDROID__)
	CRenderTargetManager::Instance().RenderBackgrounds();
	CRenderTargetManager::Instance().RenderModels();
#endif

	if (GetAsyncKeyState(VK_Z))
		STATEMANAGER.SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	if (tEnd-t1<3)
		return;

	static FILE* fp=fopen("perf_game_render.txt", "w");

	fprintf(fp, "GR.Total %d (Time %d)\n", tEnd-t1, ELTimer_GetMSec());
	fprintf(fp, "GR.DFM %d\n", t2-t1);
	fprintf(fp, "GR.EFT.UP %d\n", t3-t2);
	fprintf(fp, "GR.SHW %d\n", t4-t3);
	fprintf(fp, "GR.STT %d\n", t5-t4);
	fprintf(fp, "GR.CLL %d\n", t6-t5);
	fprintf(fp, "GR.BG.SKY %d\n", t7-t6);
	fprintf(fp, "GR.BG.LEN %d\n", t8-t7);
	fprintf(fp, "GR.BG.CLD %d\n", t9-t8);
	fprintf(fp, "GR.BG.MAIN %d\n", t10-t9);
	fprintf(fp, "GR.CHR %d\n",	t11-t10);
	fprintf(fp, "GR.BG.WTR %d\n", t12-t11);
	fprintf(fp, "GR.BG.EFT %d\n", t13-t12);
	fprintf(fp, "GR.EFT %d\n", t14-t13);
	fprintf(fp, "GR.ITM %d\n", t15-t14);
	fprintf(fp, "GR.FLY %d\n", t16-t15);
	fprintf(fp, "GR.BG.BLK %d\n", t17-t16);
	fprintf(fp, "GR.BG.LEN %d\n", t18-t17);

	fflush(fp);
}

void CPythonApplication::UpdateGame()
{
	DWORD t1=ELTimer_GetMSec();
#if defined(ENABLE_RENDER_TARGET) && !defined(__ANDROID__)
	CRenderTargetManager::Instance().UpdateModels();
	CRenderTargetManager::Instance().DeformModels();
#endif
	POINT ptMouse;
	GetMousePosition(&ptMouse);

	CGraphicTextInstance::Hyperlink_UpdateMousePos(ptMouse.x, ptMouse.y);

	DWORD t2=ELTimer_GetMSec();

	//if (m_isActivateWnd)
	{
		CScreen s;
		float fAspect = UI::CWindowManager::Instance().GetAspect();
		float fFarClip = CPythonBackground::Instance().GetFarClip();

		float fNearClip = m_bIsFirstPersonMode ? 10.0f : 120.0f;
#ifdef ENABLE_FOV_OPTION
		float fFOV = CPythonSystem::Instance().GetFOVLevel();
		if (m_bIsFirstPersonMode && fFOV < 70.0f)
			fFOV = 70.0f;
		s.SetPerspective(fFOV, fAspect, fNearClip, fFarClip);
#else
		float fFOV = m_bIsFirstPersonMode ? 70.0f : 30.0f;
		s.SetPerspective(fFOV, fAspect, fNearClip, fFarClip);
#endif
		s.BuildViewFrustum();
	}

	DWORD t3=ELTimer_GetMSec();
	TPixelPosition kPPosMainActor;
	m_pyPlayer.NEW_GetMainActorPosition(&kPPosMainActor);

// STATIC CAMERA INTERCEPT 1
	if (m_bIsCameraStatic)
	{
		GetCenterPosition(&kPPosMainActor);
	}

	DWORD t4=ELTimer_GetMSec();
	m_pyBackground.Update(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);

	DWORD t5=ELTimer_GetMSec();
	m_GameEventManager.SetCenterPosition(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);
	m_GameEventManager.Update();
#ifdef ENABLE_IKASHOP_RENEWAL
	m_pyOfflineshop.UpdateEntities();
#endif
#ifdef ENABLE_MINI_GAME_YUTNORI
	m_kYutNoriManager.UpdateModel();
#endif
	DWORD t6=ELTimer_GetMSec();
	m_kChrMgr.Update();
	DWORD t7=ELTimer_GetMSec();
#ifdef __ANDROID__
	m_FlyingManager.Update();
	DWORD t8=ELTimer_GetMSec();
	m_kEftMgr.Update();
	m_kEftMgr.UpdateSound();
#else
	m_kEftMgr.Update(); //@fixme029
	m_kEftMgr.UpdateSound();
	DWORD t8=ELTimer_GetMSec();
	m_FlyingManager.Update();
#endif
	DWORD t9=ELTimer_GetMSec();
	m_pyItem.Update(ptMouse);
	DWORD t10=ELTimer_GetMSec();
	m_pyPlayer.Update();
	DWORD t11=ELTimer_GetMSec();

	m_pyPlayer.NEW_GetMainActorPosition(&kPPosMainActor);
// STATIC CAMERA INTERCEPT 2
	if (m_bIsCameraStatic)
	{
		GetCenterPosition(&kPPosMainActor);
	}
	SetCenterPosition(kPPosMainActor.x, kPPosMainActor.y, kPPosMainActor.z);
#ifdef __ANDROID__
	Frustum::Instance().characterPosition = D3DXVECTOR3(kPPosMainActor.x, -kPPosMainActor.y, kPPosMainActor.z);
	m_SoundManager.SetPosition(kPPosMainActor.x, -kPPosMainActor.y, kPPosMainActor.z);
	__UpdateCamera();
	OnCameraUpdate();
#endif
	DWORD t12=ELTimer_GetMSec();

	if (PERF_CHECKER_RENDER_GAME)
	{
		if (t12-t1>5)
		{
			static FILE* fp=fopen("perf_game_update.txt", "w");

			fprintf(fp, "GU.Total %d (Time %d)\n", t12-t1, ELTimer_GetMSec());
			fprintf(fp, "GU.GMP %d\n", t2-t1);
			fprintf(fp, "GU.SCR %d\n", t3-t2);
			fprintf(fp, "GU.MPS %d\n", t4-t3);
			fprintf(fp, "GU.BG %d\n", t5-t4);
			fprintf(fp, "GU.GEM %d\n", t6-t5);
			fprintf(fp, "GU.CHR %d\n", t7-t6);
			fprintf(fp, "GU.EFT %d\n", t8-t7);
			fprintf(fp, "GU.FLY %d\n", t9-t8);
			fprintf(fp, "GU.ITM %d\n", t10-t9);
			fprintf(fp, "GU.PLR %d\n", t11-t10);
			fprintf(fp, "GU.POS %d\n", t12-t11);
			fflush(fp);
		}
	}
}

void CPythonApplication::SkipRenderBuffering(DWORD dwSleepMSec)
{
	m_dwBufSleepSkipTime=ELTimer_GetMSec()+dwSleepMSec;
}

bool CPythonApplication::Process()
{
#ifdef ATTACK_GUARD
	SecurityCheck();
#endif
#if defined(CHECK_LATEST_DATA_FILES)
	if (CheckLatestFiles_PollEvent())
		return false;
#endif
	ELTimer_SetFrameMSec();

	// 	m_Profiler.Clear();
	DWORD dwStart = ELTimer_GetMSec();

	///////////////////////////////////////////////////////////////////////////////////////////////////
	static DWORD	s_dwUpdateFrameCount = 0;
	static DWORD	s_dwRenderFrameCount = 0;
	static DWORD	s_dwFaceCount = 0;
	static UINT		s_uiLoad = 0;
	static DWORD	s_dwCheckTime = ELTimer_GetMSec();

	if (ELTimer_GetMSec() - s_dwCheckTime > 1000)
	{
		m_dwUpdateFPS		= s_dwUpdateFrameCount;
		m_dwRenderFPS		= s_dwRenderFrameCount;
		m_dwLoad			= s_uiLoad;

		m_dwFaceCount		= s_dwFaceCount / max(1, s_dwRenderFrameCount);

		s_dwCheckTime		= ELTimer_GetMSec();

		s_uiLoad = s_dwFaceCount = s_dwUpdateFrameCount = s_dwRenderFrameCount = 0;
	}

	// Update Time
#ifdef ENABLE_CSHIELD
	static DWORD time = ELTimer_GetMSec();
	if (ELTimer_GetMSec() - time > 10000)
	{
		if (!ValidateCShield())
		{
#ifdef ENABLE_CSHIELD_DEBUG
			ErrorMessage("Could not validate CShield");
#endif
			CShieldClose();
		}
		time = ELTimer_GetMSec();
	}
#endif
	static BOOL s_bFrameSkip = false;
	static UINT s_uiNextFrameTime = ELTimer_GetMSec();

#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime1=ELTimer_GetMSec();
#endif
	CTimer& rkTimer=CTimer::Instance();
	rkTimer.Advance();

	m_fGlobalTime = rkTimer.GetCurrentSecond();
	m_fGlobalElapsedTime = rkTimer.GetElapsedSecond();

	UINT uiFrameTime = rkTimer.GetElapsedMilliecond();
	s_uiNextFrameTime += uiFrameTime;

	DWORD updatestart = ELTimer_GetMSec();
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime2=ELTimer_GetMSec();
#endif
	// Network I/O
	m_pyNetworkStream.Process();
	//m_pyNetworkDatagram.Process();

	m_kGuildMarkUploader.Process();

	m_kGuildMarkDownloader.Process();
	m_kAccountConnector.Process();
#if defined(ENABLE_VOICE_CHAT)
	m_VoiceChat.Process();
#endif

#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime3=ELTimer_GetMSec();
#endif
	//////////////////////
	// Input Process
	// Keyboard
	UpdateKeyboard();
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime4=ELTimer_GetMSec();
#endif
	#ifndef __ANDROID__
	// Mouse
	POINT Point;
	if (GetCursorPos(&Point))
	{
		ScreenToClient(m_hWnd, &Point);
		OnMouseMove(Point.x, Point.y);
	}
#endif
	//////////////////////
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime5=ELTimer_GetMSec();
#endif
	//if (m_isActivateWnd)
	#ifndef __ANDROID__
	__UpdateCamera();
#endif
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime6=ELTimer_GetMSec();
#endif
	// Update Game Playing
	CResourceManager::Instance().Update();
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime7=ELTimer_GetMSec();
#endif
#ifndef __ANDROID__
	OnCameraUpdate();
#endif
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime8=ELTimer_GetMSec();
#endif
	#ifdef __ANDROID__
	static int s_iAndroidFrame = 0;
	++s_iAndroidFrame;
	if (s_iAndroidFrame <= 30)
	{
		Tracenf("[Frame %d] Before OnMouseUpdate (m_poMouseHandler=%p, ref=%zd, type=%p)",
			s_iAndroidFrame, m_poMouseHandler,
			m_poMouseHandler ? (ssize_t)m_poMouseHandler->ob_refcnt : -1,
			m_poMouseHandler ? m_poMouseHandler->ob_type : nullptr);
	}
#endif
	OnMouseUpdate();
#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime9=ELTimer_GetMSec();
#endif
#ifdef __ANDROID__
	if (s_iAndroidFrame <= 30)
		Tracenf("[Frame %d] Before OnUIUpdate", s_iAndroidFrame);
#endif
	OnUIUpdate();
#ifdef __ANDROID__
	if (s_iAndroidFrame <= 30)
	{
		Tracenf("[Frame %d] After OnUIUpdate (m_poMouseHandler=%p, ref=%zd, type=%p)",
			s_iAndroidFrame, m_poMouseHandler,
			m_poMouseHandler ? (ssize_t)m_poMouseHandler->ob_refcnt : -1,
			m_poMouseHandler ? m_poMouseHandler->ob_type : nullptr);
	}
#endif
// Guard against NaN or zero-distance camera vectors on bridge collision seams
	if (CCamera* pCam = CCameraManager::Instance().GetCurrentCamera())
	{
		static D3DXVECTOR3 s_v3LastValidEye(0.0f, -1500.0f, 600.0f);
		static D3DXVECTOR3 s_v3LastValidTarget(0.0f, 0.0f, 0.0f);
		static D3DXVECTOR3 s_v3LastValidUp(0.0f, 0.0f, 1.0f);

		const D3DXVECTOR3& rEye = pCam->GetEye();
		const D3DXVECTOR3& rTarget = pCam->GetTarget();
		const D3DXVECTOR3& rUp = pCam->GetUp();
		D3DXVECTOR3 vDiff = rTarget - rEye;

		bool bInvalid = !_finite(rEye.x) || !_finite(rEye.y) || !_finite(rEye.z) ||
						!_finite(rTarget.x) || !_finite(rTarget.y) || !_finite(rTarget.z) ||
						!_finite(rUp.x) || !_finite(rUp.y) || !_finite(rUp.z) ||
						(D3DXVec3LengthSq(&vDiff) < 1.0f) ||
						(D3DXVec3LengthSq(&rUp) < 0.001f);

		if (bInvalid)
		{
			pCam->SetViewParams(s_v3LastValidEye, s_v3LastValidTarget, s_v3LastValidUp);
		}
		else
		{
			s_v3LastValidEye = rEye;
			s_v3LastValidTarget = rTarget;
			s_v3LastValidUp = rUp;
		}
	}

#ifdef __PERFORMANCE_CHECK__
	DWORD dwUpdateTime10=ELTimer_GetMSec();

	if (dwUpdateTime10-dwUpdateTime1>10)
	{
		static FILE* fp=fopen("perf_app_update.txt", "w");

		fprintf(fp, "AU.Total %d (Time %d)\n", dwUpdateTime9-dwUpdateTime1, ELTimer_GetMSec());
		fprintf(fp, "AU.TU %d\n", dwUpdateTime2-dwUpdateTime1);
		fprintf(fp, "AU.NU %d\n", dwUpdateTime3-dwUpdateTime2);
		fprintf(fp, "AU.KU %d\n", dwUpdateTime4-dwUpdateTime3);
		fprintf(fp, "AU.MP %d\n", dwUpdateTime5-dwUpdateTime4);
		fprintf(fp, "AU.CP %d\n", dwUpdateTime6-dwUpdateTime5);
		fprintf(fp, "AU.RU %d\n", dwUpdateTime7-dwUpdateTime6);
		fprintf(fp, "AU.CU %d\n", dwUpdateTime8-dwUpdateTime7);
		fprintf(fp, "AU.MU %d\n", dwUpdateTime9-dwUpdateTime8);
		fprintf(fp, "AU.UU %d\n", dwUpdateTime10-dwUpdateTime9);
		fprintf(fp, "----------------------------------\n");
		fflush(fp);
	}
#endif

	m_dwCurUpdateTime = ELTimer_GetMSec() - updatestart;

	DWORD dwCurrentTime = ELTimer_GetMSec();
	BOOL  bCurrentLateUpdate = FALSE;

	s_bFrameSkip = false;

	if (dwCurrentTime > s_uiNextFrameTime)
	{
		int dt = (int)(dwCurrentTime - s_uiNextFrameTime);
		UINT uiSafeFrameTime = max(1u, uiFrameTime);
		int nAdjustTime = (dt / (int)uiSafeFrameTime) * (int)uiSafeFrameTime;


		if (dt >= 500)
		{
			s_uiNextFrameTime = dwCurrentTime;
			CTimer::Instance().Adjust(nAdjustTime);
		}
		s_bFrameSkip = true;
		bCurrentLateUpdate = TRUE;
	}

	if (m_isFrameSkipDisable)
		s_bFrameSkip = false;
#ifdef __ANDROID__
	s_bFrameSkip = false;
#endif
	if (!s_bFrameSkip)
	{
		CGrannyMaterial::TranslateSpecularMatrix(g_specularSpd, g_specularSpd, 0.0f);

		DWORD dwRenderStartTime = ELTimer_GetMSec();

		bool canRender = true;

		if (m_isMinimizedWnd)
		{
			canRender = false;
		}
		else
		{
			if (m_pyGraphic.IsLostDevice())
			{
				CPythonBackground& rkBG = CPythonBackground::Instance();
				rkBG.ReleaseCharacterShadowTexture();
#ifndef DISABLE_JTX_DYNAMIC_LIGHT
				GetPostProcessingChain().ReleaseResources();
#endif
#if defined(ENABLE_RENDER_TARGET) && !defined(__ANDROID__)
				CRenderTargetManager::Instance().ReleaseRenderTargetTextures();
#endif
				STATEMANAGER.ReleaseRenderTargets();

				if (m_pyGraphic.RestoreDevice())
				{
					rkBG.CreateCharacterShadowTexture();
#ifndef DISABLE_JTX_DYNAMIC_LIGHT
					GetPostProcessingChain().CreateResources();
#endif
#if defined(ENABLE_RENDER_TARGET) && !defined(__ANDROID__)
					CRenderTargetManager::Instance().CreateRenderTargetTextures();
#endif
				}
				else
					canRender = false;
			}
		}

		if (!IsActive())
		{
			SkipRenderBuffering(3000);
		}

		// Always drain SpherePackFactory integration/recompute FIFOs even while minimized
		CCullingManager::Instance().Update();

		if (!canRender)
		{
			SkipRenderBuffering(0);
			if (m_isMinimizedWnd)
			{
				Sleep(16);
			}
		}
		else
		{
			// RestoreLostDevice
			if (m_pyGraphic.Begin())
			{
				m_pyGraphic.Clear();

#ifdef _DEBUG
				m_pyGraphic.SetClearColor(0.3f, 0.3f, 0.3f);
				m_pyGraphic.Clear();
#endif

				/////////////////////
				// Interface
				m_pyGraphic.SetInterfaceRenderState();

#ifdef __ANDROID__
				if (s_iAndroidFrame <= 30)
					Tracenf("[Frame %d] Before OnUIRender", s_iAndroidFrame);
#endif
				OnUIRender();
#ifdef __ANDROID__
				if (s_iAndroidFrame <= 30)
					Tracenf("[Frame %d] Before OnMouseRender", s_iAndroidFrame);
#endif
				OnMouseRender();
				/////////////////////

				m_pyGraphic.End();

				m_pyGraphic.Show();
#ifdef __ANDROID__
				if (s_iAndroidFrame <= 30)
					Tracenf("[Frame %d] Render Done", s_iAndroidFrame);
#endif

				DWORD dwRenderEndTime = ELTimer_GetMSec();

				static DWORD s_dwRenderCheckTime = dwRenderEndTime;
				static DWORD s_dwRenderRangeTime = 0;
				static DWORD s_dwRenderRangeFrame = 0;

				m_dwCurRenderTime = dwRenderEndTime - dwRenderStartTime;
				s_dwRenderRangeTime += m_dwCurRenderTime;
				++s_dwRenderRangeFrame;

				if (dwRenderEndTime-s_dwRenderCheckTime>1000)
				{
					m_fAveRenderTime=float(double(s_dwRenderRangeTime)/double(s_dwRenderRangeFrame));

					s_dwRenderCheckTime=ELTimer_GetMSec();
					s_dwRenderRangeTime=0;
					s_dwRenderRangeFrame=0;
				}

				DWORD dwCurFaceCount=m_pyGraphic.GetFaceCount();
				m_pyGraphic.ResetFaceCount();
				s_dwFaceCount += dwCurFaceCount;

				if (dwCurFaceCount > 5000)
				{
					if (dwRenderEndTime > m_dwBufSleepSkipTime)
					{
						static float s_fBufRenderTime = 0.0f;

						float fCurRenderTime = m_dwCurRenderTime;

						if (fCurRenderTime > s_fBufRenderTime)
						{
							float fRatio = fMAX(0.5f, (fCurRenderTime - s_fBufRenderTime) / 30.0f);
							s_fBufRenderTime = (s_fBufRenderTime * (100.0f - fRatio) + (fCurRenderTime + 5) * fRatio) / 100.0f;
						}
						else
						{
							float fRatio = 0.5f;
							s_fBufRenderTime = (s_fBufRenderTime * (100.0f - fRatio) + fCurRenderTime * fRatio) / 100.0f;
						}

						if (s_fBufRenderTime > 100.0f)
							s_fBufRenderTime = 100.0f;

						DWORD dwBufRenderTime = s_fBufRenderTime;

						if (m_isWindowed)
						{
							if (dwBufRenderTime>58)
								dwBufRenderTime=64;
							else if (dwBufRenderTime>42)
								dwBufRenderTime=48;
							else if (dwBufRenderTime>26)
								dwBufRenderTime=32;
							else if (dwBufRenderTime>10)
								dwBufRenderTime=16;
							else
								dwBufRenderTime=8;
						}

						m_fAveRenderTime=s_fBufRenderTime;
					}

					m_dwFaceAccCount += dwCurFaceCount;
					m_dwFaceAccTime += m_dwCurRenderTime;

					if (m_dwFaceAccTime > 0)
						m_fFaceSpd = (m_dwFaceAccCount / m_dwFaceAccTime);

					if (-1 == m_iForceSightRange)
					{
						m_pyBackground.SetViewDistanceSet(0, 25600.0f);
					}
					else
					{
						m_pyBackground.SetViewDistanceSet(0, float(m_iForceSightRange));
					}
				}
				else
				{
					m_pyBackground.SetViewDistanceSet(0, 25600.0f);
				}

				++s_dwRenderFrameCount;
			}
		}
	}

	#ifndef __ANDROID__
	if (m_isMinimizedWnd)
	{
		Sleep(16);
		s_uiNextFrameTime = ELTimer_GetMSec() + 16;
	}
	else
	{
		int rest = (int)(s_uiNextFrameTime - ELTimer_GetMSec());
		if (rest > 100)
		{
			rest = 16;
			s_uiNextFrameTime = ELTimer_GetMSec() + 16;
		}
		if (rest > 0 && !bCurrentLateUpdate)
		{
			s_uiLoad -= rest;
			Sleep(rest);
		}
	}
#endif

	++s_dwUpdateFrameCount;

	++s_dwUpdateFrameCount;

	s_uiLoad += ELTimer_GetMSec() - dwStart;
	//m_Profiler.ProfileByScreen();
	return true;
}

void CPythonApplication::UpdateClientRect()
{
	RECT rcApp;
	GetClientRect(&rcApp);
	OnSizeChange(rcApp.right - rcApp.left, rcApp.bottom - rcApp.top);
}

void CPythonApplication::SetMouseHandler(PyObject* poMouseHandler)
{
#ifdef __ANDROID__
	Py_XINCREF(poMouseHandler);
	Py_XDECREF(m_poMouseHandler);
	Tracenf("[MouseHandler] CPythonApplication::SetMouseHandler po=%p refcnt=%zd",
		poMouseHandler, poMouseHandler ? (ssize_t)poMouseHandler->ob_refcnt : 0);
#endif
	m_poMouseHandler = poMouseHandler;
}

int CPythonApplication::CheckDeviceState()
{
	CGraphicDevice::EDeviceState e_deviceState = m_grpDevice.GetDeviceState();

	switch (e_deviceState)
	{
	case CGraphicDevice::DEVICESTATE_NULL:
		return DEVICE_STATE_FALSE;

	case CGraphicDevice::DEVICESTATE_BROKEN:
		return DEVICE_STATE_SKIP;

	case CGraphicDevice::DEVICESTATE_NEEDS_RESET:
		{
			CPythonBackground& rkBG = CPythonBackground::Instance();
			rkBG.ReleaseCharacterShadowTexture();
#ifndef DISABLE_JTX_DYNAMIC_LIGHT
			GetPostProcessingChain().ReleaseResources();
#endif
#if defined(ENABLE_RENDER_TARGET) && !defined(__ANDROID__)
			CRenderTargetManager::Instance().ReleaseRenderTargetTextures();
#endif
			STATEMANAGER.ReleaseRenderTargets();

			if (!m_grpDevice.Reset())
				return DEVICE_STATE_SKIP;

			rkBG.CreateCharacterShadowTexture();
#ifndef DISABLE_JTX_DYNAMIC_LIGHT
			GetPostProcessingChain().CreateResources();
#endif
#if defined(ENABLE_RENDER_TARGET) && !defined(__ANDROID__)
			CRenderTargetManager::Instance().CreateRenderTargetTextures();
#endif
		}
		break;
	}

	return DEVICE_STATE_OK;
}

bool CPythonApplication::CreateDevice(int width, int height, int Windowed, int bit /* = 32*/, int frequency /* = 0*/)
{
	int iRet;

	m_grpDevice.InitBackBufferCount(2);
	m_grpDevice.RegisterWarningString(CGraphicDevice::CREATE_BAD_DRIVER, ApplicationStringTable_GetStringz(IDS_WARN_BAD_DRIVER, "WARN_BAD_DRIVER"));
	m_grpDevice.RegisterWarningString(CGraphicDevice::CREATE_NO_TNL, ApplicationStringTable_GetStringz(IDS_WARN_NO_TNL, "WARN_NO_TNL"));

	iRet = m_grpDevice.Create(GetWindowHandle(), width, height, Windowed ? true : false, bit,frequency);

	switch (iRet)
	{
	case CGraphicDevice::CREATE_OK:
		return true;

	case CGraphicDevice::CREATE_REFRESHRATE:
		return true;

	case CGraphicDevice::CREATE_ENUM:
	case CGraphicDevice::CREATE_DETECT:
		SET_EXCEPTION(CREATE_NO_APPROPRIATE_DEVICE);
		TraceError("CreateDevice: Enum & Detect failed");
		return false;

	case CGraphicDevice::CREATE_NO_DIRECTX:
		SET_EXCEPTION(CREATE_NO_DIRECTX);
		TraceError("CreateDevice: DirectX 8.1 or greater required to run game");
		return false;

	case CGraphicDevice::CREATE_DEVICE:
		SET_EXCEPTION(CREATE_DEVICE);
		TraceError("CreateDevice: GraphicDevice create failed");
		return false;

	case CGraphicDevice::CREATE_FORMAT:
		SET_EXCEPTION(CREATE_FORMAT);
		TraceError("CreateDevice: Change the screen format");
		return false;

	case CGraphicDevice::CREATE_GET_DEVICE_CAPS:
		PyErr_SetString(PyExc_RuntimeError, "GetDevCaps failed");
		TraceError("CreateDevice: GetDevCaps failed");
		return false;

	case CGraphicDevice::CREATE_GET_DEVICE_CAPS2:
		PyErr_SetString(PyExc_RuntimeError, "GetDevCaps2 failed");
		TraceError("CreateDevice: GetDevCaps2 failed");
		return false;

	default:
		if (iRet & CGraphicDevice::CREATE_OK)
		{
			if (iRet & CGraphicDevice::CREATE_NO_TNL)
			{
				CGrannyLODController::SetMinLODMode(true);
				//LogBox(ApplicationStringTable_GetStringz(IDS_WARN_NO_TNL), NULL, GetWindowHandle());
			}
			return true;
		}

		SET_EXCEPTION(UNKNOWN_ERROR);
		TraceError("CreateDevice: Unknown Error!");
		return false;
	}
}

void CPythonApplication::Loop()
{
#ifdef __ANDROID__
	static bool s_bFirstLoopFromScript = true;
	if (s_bFirstLoopFromScript)
	{
		s_bFirstLoopFromScript = false;
		// Keep RunApp() and prototype.py frames alive across JNI render callbacks without calling __import__
		for (PyFrameObject* f = PyEval_GetFrame(); f != NULL; f = f->f_back)
		{
			Py_INCREF(f);
			PyFrame_FastToLocals(f);
			if (f->f_locals && PyDict_Check(f->f_locals))
			{
				PyObject* poMainStream = PyDict_GetItemString(f->f_locals, "mainStream");
				if (poMainStream)
				{
					Py_INCREF(poMainStream);
					PyObject* poNoop = PyRun_String("lambda *a, **kw: None", Py_eval_input, f->f_globals, f->f_locals);
					if (poNoop)
					{
						PyObject_SetAttrString(poMainStream, (char*)"Destroy", poNoop);
						Py_DECREF(poNoop);
					}
					else
					{
						PyErr_Clear();
					}
				}
			}
		}
		Tracen("Metin2 Mobile: First app.Loop() intercepted cleanly, entering frame render loop.");
		return;
	}
	Process();
#else
	while (1)
	{
		if (IsMessage())
		{
			if (!MessageProcess())
				break;
		}
		else
		{
			if (!Process())
				break;

			m_dwLastIdleTime=ELTimer_GetMSec();
		}
	}
#endif
}

#define ENABLE_LOAD_ITEM_LIST_FROM_ROOT
#define ENABLE_LOAD_ITEM_SCALE_FROM_ROOT
#define ENABLE_LOAD_SKILL_TABLE_FROM_ROOT
bool LoadLocaleData(const char* localePath)
{
	CPythonNonPlayer&	rkNPCMgr	= CPythonNonPlayer::Instance();
	CItemManager&		rkItemMgr	= CItemManager::Instance();
	CPythonSkill&		rkSkillMgr	= CPythonSkill::Instance();
#ifdef NEW_PET_SYSTEM
	CPythonSkillPet&    rkSkillPetMgr = CPythonSkillPet::Instance();
#endif
	CPythonNetworkStream& rkNetStream = CPythonNetworkStream::Instance();

	char szItemList[256];
	char szItemProto[256];
	char szItemDesc[256];
#ifdef NEW_PET_SYSTEM
	char szSkillPetFileName[256];
#endif
	char szMobProto[256];
	char szSkillDescFileName[256];
	char szSkillTableFileName[256];
	char szInsultList[256];
#ifdef ENABLE_LOAD_ITEM_LIST_FROM_ROOT
	snprintf(szItemList,	sizeof(szItemList) ,	"item_list.txt");
#else
	snprintf(szItemList,	sizeof(szItemList) ,	"%s/item_list.txt",	localePath);
#endif
	snprintf(szItemProto,	sizeof(szItemProto),	"%s/item_proto",	localePath);
	snprintf(szItemDesc,	sizeof(szItemDesc),	"%s/itemdesc.txt",	localePath);
#ifdef NEW_PET_SYSTEM
	snprintf(szSkillPetFileName, sizeof(szSkillPetFileName), "%s/pet_skill.txt", localePath);
#endif
	snprintf(szMobProto,	sizeof(szMobProto),	"%s/mob_proto",		localePath);
	snprintf(szSkillDescFileName, sizeof(szSkillDescFileName),	"%s/SkillDesc.txt", localePath);
	#ifdef ENABLE_LOAD_SKILL_TABLE_FROM_ROOT
	snprintf(szSkillTableFileName, sizeof(szSkillTableFileName),	"SkillTable.txt");
	#else
	snprintf(szSkillTableFileName, sizeof(szSkillTableFileName),	"%s/SkillTable.txt", localePath);
	#endif
	snprintf(szInsultList,	sizeof(szInsultList),	"%s/insult.txt", localePath);

	rkNPCMgr.Destroy();
	rkItemMgr.Destroy();
	rkSkillMgr.Destroy();
#ifdef NEW_PET_SYSTEM
	rkSkillPetMgr.Destroy();
#endif
#ifdef ENABLE_AURA_SYSTEM
	char szAuraScale[256];
	snprintf(szAuraScale, sizeof(szAuraScale), "%s/aura_scale.txt", localePath);
	if (!rkItemMgr.LoadAuraScale(szAuraScale))
	{
		TraceError("LoadLocaleData - LoadAuraScale(%s) Error", szAuraScale);
	}
#endif

	if (!rkItemMgr.LoadItemList(szItemList))
	{
		TraceError("LoadLocaleData - LoadItemList(%s) Error", szItemList);
	}

	if (!rkItemMgr.LoadItemTable(szItemProto))
	{
		TraceError("LoadLocaleData - LoadItemProto(%s) Error", szItemProto);
		return false;
	}

#if defined(__BL_CHEST_DROP_INFO__)
	char szChestDrop[256];
	snprintf(szChestDrop, sizeof(szChestDrop), "%s/chest_drop", localePath);

	if (!rkItemMgr.LoadChestDropInfo(szChestDrop))
	{
		TraceError("LoadLocaleData - LoadChestDropInfo(%s) Error", szChestDrop);
		// REMOVED: return false; 
	}
#endif

	if (!rkItemMgr.LoadItemDesc(szItemDesc))
	{
		Tracenf("LoadLocaleData - LoadItemDesc(%s) Error", szItemDesc);
	}

	if (!rkNPCMgr.LoadNonPlayerData(szMobProto))
	{
		TraceError("LoadLocaleData - LoadMobProto(%s) Error", szMobProto);
		return false;
	}

	if (!rkSkillMgr.RegisterSkillDesc(szSkillDescFileName))
	{
		TraceError("LoadLocaleData - RegisterSkillDesc(%s) Error", szSkillDescFileName); //@warme670
		return false;
	}

#ifdef NEW_PET_SYSTEM
	if (!rkSkillPetMgr.RegisterSkillPet(szSkillPetFileName))
	{
		TraceError("LoadLocaleData - RegisterSkillPet(%s) Error", szSkillPetFileName);
		return false;
	}
#endif


	if (!rkSkillMgr.RegisterSkillTable(szSkillTableFileName))
	{
		TraceError("LoadLocaleData - RegisterSkillTable(%s) Error", szSkillTableFileName); //@warme670
		return false;
	}

	if (!rkNetStream.LoadInsultList(szInsultList))
	{
		Tracenf("CPythonApplication - CPythonNetworkStream::LoadInsultList(%s)", szInsultList);
	}

#ifdef ENABLE_ACCE_COSTUME_SYSTEM
	char szItemScale[256]{};
	#ifdef ENABLE_LOAD_ITEM_SCALE_FROM_ROOT
	snprintf(szItemScale, sizeof(szItemScale), "item_scale.txt");
	#else
	snprintf(szItemScale, sizeof(szItemScale), "%s/item_scale.txt", localePath);
	#endif

	if (!rkItemMgr.LoadItemScale(szItemScale))
	{
		Tracenf("LoadLocaleData: error while loading %s.", szItemScale);
		return false;
	}
#endif

	return true;
}

unsigned __GetWindowMode(bool windowed)
{
	if (windowed)
#ifdef ENABLE_MINI_WINDOW_SYSTEM
		return WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME | WS_MAXIMIZEBOX;
#else
		return WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
#endif

	return WS_POPUP;
}

bool CPythonApplication::Create(PyObject * poSelf, const char * c_szName, int width, int height, int Windowed)
{
#ifdef __ANDROID__
	extern int g_AndroidScreenWidth;
	extern int g_AndroidScreenHeight;
	if (g_AndroidScreenWidth > 0 && g_AndroidScreenHeight > 0)
	{
		width  = g_AndroidScreenWidth;
		height = g_AndroidScreenHeight;
	}
#endif
#ifdef ATTACK_GUARD
	InitializeAttackGuard();
#endif
#ifdef ANTI_SPLASH
	ShowAntiCheatSplash(GetInstance());
#endif
	Windowed = CPythonSystem::Instance().IsWindowed() ? 1 : 0;

	bool bAnotherWindow = false;

	if (FindWindow(NULL, c_szName))
		bAnotherWindow = true;

	m_dwWidth = width;
	m_dwHeight = height;

	// Window
	UINT WindowMode = __GetWindowMode(Windowed ? true : false);

	if (!CMSWindow::Create(c_szName, 4, 0, WindowMode, ::LoadIcon( GetInstance(), MAKEINTRESOURCE( IDI_METIN2 ) ), IDC_CURSOR_NORMAL))
	{
		TraceError("CMSWindow::Create failed");
		SET_EXCEPTION(CREATE_WINDOW);
		return false;
	}

	if (m_pySystem.IsUseDefaultIME())
		CPythonIME::Instance().UseDefaultIME();

	#ifdef ENABLE_DISCORD_RPC
	m_pyNetworkStream.Discord_Start();
	#endif

	if (!m_pySystem.IsWindowed())
	{
		m_isWindowed = false;
		m_isWindowFullScreenEnable = TRUE;
		__SetFullScreenWindow(GetWindowHandle(), width, height, m_pySystem.GetBPP());

		Windowed = true;
	}
	else
	{
		AdjustSize(m_pySystem.GetWidth(), m_pySystem.GetHeight());

		if (Windowed)
		{
			m_isWindowed = true;
			// @fixme031 BEGIN
			RECT rc{};
			GetClientRect(&rc);
			const auto windowWidth = rc.right - rc.left;
			const auto windowHeight = (rc.bottom - rc.top);
			RECT rc2{};
			GetWindowRect(&rc2);
			const auto windowWidth2 = rc2.right - rc2.left;
			const auto windowHeight2 = (rc2.bottom - rc2.top);
			const auto windowWidthDiff = windowWidth2 - windowWidth;
			const auto windowHeightDiff = windowHeight2 - windowHeight;
			const auto dropshadowSize = (windowWidthDiff / 2 != 0) ? (windowWidthDiff / 2 - 1) : 0;
			constexpr auto taskbarSize = 73;
			constexpr auto titlebarSize = 10;
			if (bAnotherWindow)
				CMSApplication::SetPosition(GetScreenWidth() - windowWidth - dropshadowSize, GetScreenHeight() - windowHeight - taskbarSize);
			else
				SetPosition(-dropshadowSize, (m_pySystem.GetHeight() >= 1000) ? -titlebarSize : 0);
			// @fixme031 END
		}
		else
		{
			m_isWindowed = false;
			SetPosition(0, 0);
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Cursor
	if (!CreateCursors())
	{
		TraceError("CMSWindow::Cursors Create Error");
		SET_EXCEPTION("CREATE_CURSOR");
		return false;
	}

	if (!m_pySystem.IsNoSoundCard())
	{
		// Sound
		if (!m_SoundManager.Create())
		{
			// LogBox(ApplicationStringTable_GetStringz(IDS_WARN_NO_SOUND_DEVICE));
		}
	}

	extern bool GRAPHICS_CAPS_SOFTWARE_TILING;

	if (!m_pySystem.IsAutoTiling())
		GRAPHICS_CAPS_SOFTWARE_TILING = m_pySystem.IsSoftwareTiling();

	// Device
	Tracen("CPythonApplication::Create -> CreateDevice...");
	if (!CreateDevice(m_pySystem.GetWidth(), m_pySystem.GetHeight(), Windowed, m_pySystem.GetBPP(), m_pySystem.GetFrequency()))
		return false;

	Tracen("CPythonApplication::Create -> GrannyCreateSharedDeformBuffer...");
	GrannyCreateSharedDeformBuffer();

	if (m_pySystem.IsAutoTiling())
	{
		if (m_grpDevice.IsFastTNL())
			m_pyBackground.ReserveSoftwareTilingEnable(false);
		else
			m_pyBackground.ReserveSoftwareTilingEnable(true);
	}
	else
		m_pyBackground.ReserveSoftwareTilingEnable(m_pySystem.IsSoftwareTiling());


#ifdef ANTI_SPLASH
// Keep the main game window completely hidden during the loading phase
	SetVisibleMode(false); 

	if (m_isWindowFullScreenEnable)
		SetWindowPos(GetWindowHandle(), HWND_TOP, 0, 0, width, height, SWP_HIDEWINDOW);
#else
	SetVisibleMode(true);

	if (m_isWindowFullScreenEnable)
		SetWindowPos(GetWindowHandle(), HWND_TOP, 0, 0, width, height, SWP_SHOWWINDOW);
#endif
#ifndef __ANDROID__
	if (!InitializeKeyboard(GetWindowHandle()))
		return false;
#endif

	m_pySystem.GetDisplaySettings();

	// Mouse
	if (m_pySystem.IsSoftwareCursor())
		SetCursorMode(CURSOR_MODE_SOFTWARE);
	else
		SetCursorMode(CURSOR_MODE_HARDWARE);

	// Network
	Tracen("CPythonApplication::Create -> m_netDevice.Create...");
	if (!m_netDevice.Create())
	{
		//PyErr_SetString(PyExc_RuntimeError, "NetDevice::Create failed");
		TraceError("NetDevice::Create failed");
		SET_EXCEPTION("CREATE_NETWORK");
		return false;
	}

	if (!m_grpDevice.IsFastTNL())
		CGrannyLODController::SetMinLODMode(true);

	Tracen("CPythonApplication::Create -> m_pyItem / DefaultFont / IME / LightManager...");
	m_pyItem.Create();

	// Other Modules
	DefaultFont_Startup();

	CPythonIME::Instance().Create(GetWindowHandle());
	CPythonIME::Instance().SetText("", 0);
	CPythonTextTail::Instance().Initialize();

	// Light Manager
	m_LightManager.Initialize();

	CGraphicImageInstance::CreateSystem(32);

	STICKYKEYS sStickKeys;
	memset(&sStickKeys, 0, sizeof(sStickKeys));
	sStickKeys.cbSize = sizeof(sStickKeys);
	SystemParametersInfo( SPI_GETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0 );
	m_dwStickyKeysFlag = sStickKeys.dwFlags;

	sStickKeys.dwFlags &= ~(SKF_AVAILABLE|SKF_HOTKEYACTIVE);
	SystemParametersInfo( SPI_SETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0 );


	// SphereMap
	CGrannyMaterial::CreateSphereMap(0, "d:/ymir work/special/spheremap.jpg");
	CGrannyMaterial::CreateSphereMap(1, "d:/ymir work/special/spheremap01.jpg");

#ifdef __ANDROID__
	extern int g_AndroidScreenWidth;
	extern int g_AndroidScreenHeight;
	int iAndroidW = (g_AndroidScreenWidth > 0) ? g_AndroidScreenWidth : m_pySystem.GetWidth();
	int iAndroidH = (g_AndroidScreenHeight > 0) ? g_AndroidScreenHeight : m_pySystem.GetHeight();
	m_dwWidth = iAndroidW;
	m_dwHeight = iAndroidH;
	m_grpDevice.ResizeBackBuffer(iAndroidW, iAndroidH);
	m_grpDevice.Reset();
	m_kWndMgr.SetResolution(iAndroidW, iAndroidH);
	m_kWndMgr.SetScreenSize(iAndroidW, iAndroidH);
	m_pyGraphic.SetViewport(0.0f, 0.0f, (float)iAndroidW, (float)iAndroidH);
	m_isMinimizedWnd = false;
	m_isActivateWnd = true;
#endif
	Tracen("CPythonApplication::Create -> DONE!");
	return true;
}

void CPythonApplication::SetGlobalCenterPosition(LONG x, LONG y)
{
	CPythonBackground& rkBG=CPythonBackground::Instance();
	rkBG.GlobalPositionToLocalPosition(x, y);

	float z = CPythonBackground::Instance().GetHeight(x, y);

	CPythonApplication::Instance().SetCenterPosition(x, y, z);
}

void CPythonApplication::SetCenterPosition(float fx, float fy, float fz)
{
	m_v3CenterPosition.x = +fx;
	m_v3CenterPosition.y = -fy;
	m_v3CenterPosition.z = +fz;
}

void CPythonApplication::GetCenterPosition(TPixelPosition * pPixelPosition)
{
	pPixelPosition->x = +m_v3CenterPosition.x;
	pPixelPosition->y = -m_v3CenterPosition.y;
	pPixelPosition->z = +m_v3CenterPosition.z;
}

void CPythonApplication::SetServerTime(time_t tTime)
{
	m_dwStartLocalTime	= ELTimer_GetMSec();
	m_tServerTime		= tTime;
	m_tLocalStartTime	= time(0);
}

time_t CPythonApplication::GetServerTime()
{
	return (ELTimer_GetMSec() - m_dwStartLocalTime) + m_tServerTime;
}

time_t CPythonApplication::GetServerTimeStamp()
{
	return (time(0) - m_tLocalStartTime) + m_tServerTime;
}

float CPythonApplication::GetGlobalTime()
{
	return m_fGlobalTime;
}

float CPythonApplication::GetGlobalElapsedTime()
{
	return m_fGlobalElapsedTime;
}

void CPythonApplication::SetFPS(int iFPS)
{
	m_iFPS = iFPS;
}

int CPythonApplication::GetWidth()
{
	return m_dwWidth;
}

int CPythonApplication::GetHeight()
{
	return m_dwHeight;
}

void CPythonApplication::SetConnectData(const char * c_szIP, int iPort)
{
	m_strIP = c_szIP;
	m_iPort = iPort;
}

void CPythonApplication::GetConnectData(std::string & rstIP, int & riPort)
{
	rstIP	= m_strIP;
	riPort	= m_iPort;
}

void CPythonApplication::EnableSpecialCameraMode()
{
	m_isSpecialCameraMode = TRUE;
}

void CPythonApplication::SetCameraSpeed(int iPercentage)
{
	m_fCameraRotateSpeed = c_fDefaultCameraRotateSpeed * float(iPercentage) / 100.0f;
	m_fCameraPitchSpeed = c_fDefaultCameraPitchSpeed * float(iPercentage) / 100.0f;
	m_fCameraZoomSpeed = c_fDefaultCameraZoomSpeed * float(iPercentage) / 100.0f;
}

void CPythonApplication::SetForceSightRange(int iRange)
{
	m_iForceSightRange = iRange;
}

void CPythonApplication::Clear()
{
	m_pySystem.Clear();
}

void CPythonApplication::Destroy()
{
	WebBrowser_Destroy();
#ifdef ENABLE_IKASHOP_RENEWAL
	CPythonIkarusShop::instance().DeleteEntities();
#endif

	// SphereMap
	CGrannyMaterial::DestroySphereMap();

	m_kWndMgr.Destroy();

	CPythonSystem::Instance().SaveConfig();

	DestroyCollisionInstanceSystem();

	m_pySystem.SaveInterfaceStatus();

	m_pyEventManager.Destroy();
	m_FlyingManager.Destroy();

	m_pyMiniMap.Destroy();

	m_pyTextTail.Destroy();
	m_pyChat.Destroy();
	m_kChrMgr.Destroy();
	m_RaceManager.Destroy();

	m_pyItem.Destroy();
	m_kItemMgr.Destroy();

	m_pyBackground.Destroy();
#if defined(ENABLE_RENDER_TARGET) && !defined(__ANDROID__)
	CRenderTargetManager::Instance().Destroy();
#endif
	m_kEftMgr.Destroy();
#ifdef ENABLE_MINI_GAME_YUTNORI
	m_kYutNoriManager.Destroy();
#endif
	m_LightManager.Destroy();

	// DEFAULT_FONT
	DefaultFont_Cleanup();
	// END_OF_DEFAULT_FONT

	GrannyDestroySharedDeformBuffer();

	m_pyGraphic.Destroy();
	//m_pyNetworkDatagram.Destroy();

	#ifdef ENABLE_DISCORD_RPC
	m_pyNetworkStream.Discord_Close();
	#endif

	m_pyRes.Destroy();

	m_kGuildMarkDownloader.Disconnect();

	CGrannyModelInstance::DestroySystem();
	CGraphicImageInstance::DestroySystem();

	m_SoundManager.Destroy();
	m_grpDevice.Destroy();

	//CSpeedTreeForestDirectX8::Instance().Clear();

	CAttributeInstance::DestroySystem();
	CTextFileLoader::DestroySystem();
	DestroyCursors();

	CMSApplication::Destroy();

	STICKYKEYS sStickKeys;
	memset(&sStickKeys, 0, sizeof(sStickKeys));
	sStickKeys.cbSize = sizeof(sStickKeys);
	sStickKeys.dwFlags = m_dwStickyKeysFlag;
	SystemParametersInfo( SPI_SETSTICKYKEYS, sizeof(sStickKeys), &sStickKeys, 0 );
}

void CPythonApplication::SetFirstPersonMode(bool bMode)
{
	if (m_bIsFirstPersonMode == bMode)
		return;

	m_bIsFirstPersonMode = bMode;
	CCamera* pCam = CCameraManager::Instance().GetCurrentCamera();

	if (m_bIsFirstPersonMode)
	{
		if (pCam)
		{
			m_fPrevCameraDistance = pCam->GetDistance();
			pCam->SetCameraState(CAMERA_STATE_NORMAL);
			pCam->SetDistance(15.0f);
		}
	}
	else
	{
		if (pCam)
		{
			pCam->SetDistance(fMAX(200.0f, m_fPrevCameraDistance));
		}
	}
}

#ifdef ENABLE_MINI_WINDOW_SYSTEM
HWND CPythonApplication::GetMainWindowHandle()
{
    return GetWindowHandle();
}
#endif
//martysama0134's fb0d8d5d0566583187a94f9a823b2075