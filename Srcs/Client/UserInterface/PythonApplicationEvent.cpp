#include "StdAfx.h"
#include "PythonApplication.h"
#include "../eterlib/Camera.h"
#ifdef ENABLE_RENDER_TARGET
#include "CRenderTargetManager.h"
#endif
#ifdef ENABLE_MINI_WINDOW_SYSTEM
#include "MiniMode.h"
#endif

void CPythonApplication::OnCameraUpdate()
{
	if ( m_pyBackground.IsMapReady() )
	{
		CCamera* pkCameraMgr = CCameraManager::Instance().GetCurrentCamera();
		if (pkCameraMgr)
			pkCameraMgr->Update();
	}
}

void CPythonApplication::OnUIUpdate()
{
	UI::CWindowManager& rkUIMgr=UI::CWindowManager::Instance();
	rkUIMgr.Update();
}

void CPythonApplication::OnUIRender()
{
	UI::CWindowManager& rkUIMgr=UI::CWindowManager::Instance();
	rkUIMgr.Render();
#ifdef ENABLE_MINI_GAME_YUTNORI
	// Drawn after UI to ensure it renders on top of the window
	m_kYutNoriManager.RenderBackground();
	m_kYutNoriManager.RenderModel();
#endif
}

void CPythonApplication::OnSizeChange(int width, int height)
{
	if (width < 320 || height < 240 || m_isMinimizedWnd || ::IsIconic(m_hWnd))
		return;

#ifdef ENABLE_MINI_WINDOW_SYSTEM
	if (g_miniMode.IsEnabled())
		return;
#endif

	m_kWndMgr.SetResolution(width, height);

	// Do not reset device or resize UI canvas before Python initializes
	if (m_kWndMgr.GetScreenWidth() <= 0 || m_kWndMgr.GetScreenHeight() <= 0)
		return;

	int canvasW = max(800, width);
	int canvasH = max(600, height);

	if (m_kWndMgr.GetScreenWidth() != canvasW || m_kWndMgr.GetScreenHeight() != canvasH)
	{
		m_kWndMgr.SetScreenSize(canvasW, canvasH);
	}

	m_dwWidth = canvasW;
	m_dwHeight = canvasH;

	SetRect(&m_rect, 0, 0, width, height);
	AdjustWindowRectEx(&m_rect, GetWindowStyle(m_hWnd), GetMenu(m_hWnd) != NULL, GetWindowExStyle(m_hWnd));

	CPythonBackground& rkBG = CPythonBackground::Instance();
	rkBG.ReleaseCharacterShadowTexture();
#ifndef DISABLE_JTX_DYNAMIC_LIGHT
	GetPostProcessingChain().ReleaseResources();
#endif
#ifdef ENABLE_RENDER_TARGET
	CRenderTargetManager::Instance().ReleaseRenderTargetTextures();
#endif
	STATEMANAGER.ReleaseRenderTargets();

	if (m_grpDevice.ResizeBackBuffer(width, height))
	{
		bool wasTopmost = (GetWindowLong(m_hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST) != 0;

		if (m_grpDevice.Reset())
		{
			if (rkBG.IsMapReady())
				rkBG.CreateCharacterShadowTexture();
#ifndef DISABLE_JTX_DYNAMIC_LIGHT
			GetPostProcessingChain().CreateResources();
#endif
#ifdef ENABLE_RENDER_TARGET
			CRenderTargetManager::Instance().CreateRenderTargetTextures();
#endif
		}

		if (!wasTopmost)
		{
			::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		}
	}
	else
	{
		if (rkBG.IsMapReady())
			rkBG.CreateCharacterShadowTexture();
#ifndef DISABLE_JTX_DYNAMIC_LIGHT
		GetPostProcessingChain().CreateResources();
#endif
#ifdef ENABLE_RENDER_TARGET
		CRenderTargetManager::Instance().CreateRenderTargetTextures();
#endif
	}
}

void CPythonApplication::OnMouseMiddleButtonDown(int x, int y)
{
	CCameraManager& rkCmrMgr=CCameraManager::Instance();
	CCamera* pkCmrCur=rkCmrMgr.GetCurrentCamera();
	if (pkCmrCur)
		pkCmrCur->BeginDrag(x, y);

	if ( !m_pyBackground.IsMapReady() )
		return;

	SetCursorNum(CAMERA_ROTATE);
	if ( CURSOR_MODE_HARDWARE == GetCursorMode())
		SetCursorVisible(FALSE, true);
}

void CPythonApplication::OnMouseMiddleButtonUp(int x, int y)
{
	CCameraManager& rkCmrMgr=CCameraManager::Instance();
	CCamera* pkCmrCur=rkCmrMgr.GetCurrentCamera();
	if (pkCmrCur)
		pkCmrCur->EndDrag();

	if ( !m_pyBackground.IsMapReady() )
		return;

	SetCursorNum(NORMAL);
	if ( CURSOR_MODE_HARDWARE == GetCursorMode())
		SetCursorVisible(TRUE);
}

void CPythonApplication::OnMouseWheel(int nLen)
{
#if defined(__BL_MOUSE_WHEEL_TOP_WINDOW__)
	bool bUIWheel = false;

	if (nLen > 0)
		bUIWheel = UI::CWindowManager::Instance().OnMouseWheelButtonUp();
	else
		bUIWheel = UI::CWindowManager::Instance().OnMouseWheelButtonDown();
	
	if (!bUIWheel)
	{
		CCameraManager& rkCmrMgr = CCameraManager::Instance();
		CCamera* pkCmrCur = rkCmrMgr.GetCurrentCamera();
		if (pkCmrCur)
			pkCmrCur->Wheel(nLen);
	}
#else
	CCameraManager& rkCmrMgr=CCameraManager::Instance();
	CCamera* pkCmrCur=rkCmrMgr.GetCurrentCamera();
	if (pkCmrCur)
		pkCmrCur->Wheel(nLen);
#endif
}

void CPythonApplication::OnMouseMove(int x, int y)
{
	CCameraManager& rkCmrMgr=CCameraManager::Instance();
	CCamera* pkCmrCur=rkCmrMgr.GetCurrentCamera();

	POINT Point;
	if (pkCmrCur)
	{
		if ( CPythonBackground::Instance().IsMapReady() && pkCmrCur->Drag(x, y, &Point) )
		{
#ifndef __ANDROID__
			x = Point.x;
			y = Point.y;
			ClientToScreen(m_hWnd, &Point);

			SetCursorPos(Point.x, Point.y);
#endif
		}
	}

	if (!m_isMinimizedWnd && !::IsIconic(m_hWnd))
	{
		RECT rcWnd{};
		::GetClientRect(m_hWnd, &rcWnd);
		int w = rcWnd.right - rcWnd.left;
		int h = rcWnd.bottom - rcWnd.top;
		if (w >= 320 && h >= 240)
		{
			UI::CWindowManager& rkWndMgr = UI::CWindowManager::Instance();
			rkWndMgr.SetResolution(w, h);
		}
	}

	UI::CWindowManager::Instance().RunMouseMove(x, y);
}

void CPythonApplication::OnMouseLeftButtonDown(int x, int y)
{
	UI::CWindowManager& rkWndMgr=UI::CWindowManager::Instance();
	rkWndMgr.RunMouseLeftButtonDown(x, y);
}

void CPythonApplication::OnMouseLeftButtonUp(int x, int y)
{
	UI::CWindowManager& rkWndMgr=UI::CWindowManager::Instance();
	rkWndMgr.RunMouseLeftButtonUp(x, y);
}

void CPythonApplication::OnMouseLeftButtonDoubleClick(int x, int y)
{
	UI::CWindowManager& rkWndMgr=UI::CWindowManager::Instance();
#if !defined(__BL_MOUSE_WHEEL_TOP_WINDOW__)
	rkWndMgr.RunMouseLeftButtonDown(x, y);
#endif
	rkWndMgr.RunMouseLeftButtonDoubleClick(x, y);
}

void CPythonApplication::OnMouseRightButtonDown(int x, int y)
{
	UI::CWindowManager& rkWndMgr=UI::CWindowManager::Instance();
	rkWndMgr.RunMouseRightButtonDown(x, y);
}

void CPythonApplication::OnMouseRightButtonUp(int x, int y)
{
	UI::CWindowManager& rkWndMgr=UI::CWindowManager::Instance();
	rkWndMgr.RunMouseRightButtonUp(x, y);
}

#ifdef ENABLE_MOUSEWHEEL_EVENT
bool CPythonApplication::OnMouseWheelEvent(short wDelta)
{
	UI::CWindowManager& rkWndMgr=UI::CWindowManager::Instance();
	return rkWndMgr.RunMouseWheel(wDelta);
}
#endif

void CPythonApplication::OnKeyDown(int iIndex)
{
	UI::CWindowManager& rkWndMgr=UI::CWindowManager::Instance();

	if (DIK_ESCAPE == iIndex)
	{
		rkWndMgr.RunPressEscapeKey();
	}

	rkWndMgr.RunKeyDown(iIndex);
}

void CPythonApplication::OnKeyUp(int iIndex)
{
	UI::CWindowManager& rkWndMgr=UI::CWindowManager::Instance();
	rkWndMgr.RunKeyUp(iIndex);
}

void CPythonApplication::RunIMEUpdate()
{
	UI::CWindowManager& rkWndMgr=UI::CWindowManager::Instance();
	rkWndMgr.RunIMEUpdate();
}
void CPythonApplication::RunIMETabEvent()
{
	UI::CWindowManager& rkWndMgr=UI::CWindowManager::Instance();
	rkWndMgr.RunIMETabEvent();
}
void CPythonApplication::RunIMEReturnEvent()
{
	UI::CWindowManager& rkWndMgr=UI::CWindowManager::Instance();
	rkWndMgr.RunIMEReturnEvent();
}
void CPythonApplication::OnIMEKeyDown(int iIndex)
{
	UI::CWindowManager& rkWndMgr=UI::CWindowManager::Instance();
	rkWndMgr.RunIMEKeyDown(iIndex);
}
/////////////////////////////

void CPythonApplication::RunIMEChangeCodePage()
{
	UI::CWindowManager& rkWndMgr=UI::CWindowManager::Instance();
	rkWndMgr.RunChangeCodePage();
}
void CPythonApplication::RunIMEOpenCandidateListEvent()
{
	UI::CWindowManager& rkWndMgr=UI::CWindowManager::Instance();
	rkWndMgr.RunOpenCandidate();
}
void CPythonApplication::RunIMECloseCandidateListEvent()
{
	UI::CWindowManager& rkWndMgr=UI::CWindowManager::Instance();
	rkWndMgr.RunCloseCandidate();
}
void CPythonApplication::RunIMEOpenReadingWndEvent()
{
	UI::CWindowManager& rkWndMgr=UI::CWindowManager::Instance();
	rkWndMgr.RunOpenReading();
}
void CPythonApplication::RunIMECloseReadingWndEvent()
{
	UI::CWindowManager& rkWndMgr=UI::CWindowManager::Instance();
	rkWndMgr.RunCloseReading();
}

/////////////////////////////
void CPythonApplication::RunPressExitKey()
{
	UI::CWindowManager& rkWndMgr=UI::CWindowManager::Instance();
	rkWndMgr.RunPressExitKey();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CPythonApplication::OnMouseUpdate()
{
#ifdef _DEBUG
	if (!m_poMouseHandler)
	{
		//assert(!" CPythonApplication::OnMouseUpdate - Mouse handler has not set!");
		return;
	}
#endif _DEBUG

	UI::CWindowManager& rkWndMgr=UI::CWindowManager::Instance();
	long lx, ly;
	rkWndMgr.GetMousePosition(lx, ly);
	PyCallClassMemberFunc(m_poMouseHandler, "Update", Py_BuildValue("(ii)", lx, ly));
}

void CPythonApplication::OnMouseRender()
{
#ifdef _DEBUG
	if (!m_poMouseHandler)
	{
		//assert(!" CPythonApplication::OnMouseRender - Mouse handler has not set!");
		return;
	}
#endif _DEBUG

	PyCallClassMemberFunc(m_poMouseHandler, "Render", Py_BuildValue("()"));
}
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
