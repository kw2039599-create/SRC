#include "StdAfx.h"
#include "PythonApplication.h"
#include "../eterlib/Camera.h"
#include "../CWebBrowser/CWebBrowser.h"

#include <winuser.h>

static int gs_nMouseCaptureRef = 0;

void CPythonApplication::SafeSetCapture()
{
	if (GetCapture() != m_hWnd)
		SetCapture(m_hWnd);
	gs_nMouseCaptureRef++;
}

void CPythonApplication::SafeReleaseCapture()
{
	gs_nMouseCaptureRef--;
	if (gs_nMouseCaptureRef <= 0)
	{
		gs_nMouseCaptureRef = 0;
		ReleaseCapture();
	}
}
void CPythonApplication::__SetFullScreenWindow(HWND hWnd, DWORD dwWidth, DWORD dwHeight, DWORD dwBPP)
{
	DEVMODE DevMode;
	DevMode.dmSize = sizeof(DevMode);
	DevMode.dmBitsPerPel = dwBPP;
	DevMode.dmPelsWidth = dwWidth;
	DevMode.dmPelsHeight = dwHeight;
	DevMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

	LONG Error = ChangeDisplaySettings(&DevMode, CDS_FULLSCREEN);
	if(Error == DISP_CHANGE_RESTART)
	{
		ChangeDisplaySettings(0,0);
	}
}

void CPythonApplication::__MinimizeFullScreenWindow(HWND hWnd, DWORD dwWidth, DWORD dwHeight)
{
	ChangeDisplaySettings(0, 0);
	SetWindowPos(hWnd, 0, 0, 0,
				 dwWidth,
				 dwHeight,
				 SWP_SHOWWINDOW);
	ShowWindow(hWnd, SW_MINIMIZE);
}

LRESULT CPythonApplication::WindowProcedure(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	const int c_DoubleClickTime = 300;
	const int c_DoubleClickBox = 5;
	static int s_xDownPosition = 0;
	static int s_yDownPosition = 0;
	static bool s_bIsSizingMove = false;

	switch (uiMsg)
	{
		case WM_SYSCOMMAND:
			if (wParam == SC_KEYMENU) // @fixme032
				return 0;
			break;
		case WM_CANCELMODE:
		case WM_KILLFOCUS:
			s_bIsSizingMove = false;
			if (GetCapture() == hWnd)
			{
				gs_nMouseCaptureRef = 0;
				ReleaseCapture();
			}
			break;

		case WM_ACTIVATEAPP:
			{
				m_isActivateWnd = (wParam == WA_ACTIVE) || (wParam == WA_CLICKACTIVE);

				if (m_isActivateWnd)
				{
					m_SoundManager.RestoreVolume();

					if (m_isWindowFullScreenEnable)
					{
						__SetFullScreenWindow(hWnd, m_dwWidth, m_dwHeight, m_pySystem.GetBPP());
					}
				}
				else
				{
					s_bIsSizingMove = false;
					if (GetCapture() == hWnd)
					{
						gs_nMouseCaptureRef = 0;
						ReleaseCapture();
					}

					m_SoundManager.SaveVolume();

					if (m_isWindowFullScreenEnable)
					{
						__MinimizeFullScreenWindow(hWnd, m_dwWidth, m_dwHeight);
					}
				}
			}
			break;

		case WM_INPUTLANGCHANGE:
			return CPythonIME::Instance().WMInputLanguage(hWnd, uiMsg, wParam, lParam);
			break;

		case WM_IME_STARTCOMPOSITION:
			return CPythonIME::Instance().WMStartComposition(hWnd, uiMsg, wParam, lParam);
			break;

		case WM_IME_COMPOSITION:
			return CPythonIME::Instance().WMComposition(hWnd, uiMsg, wParam, lParam);
			break;

		case WM_IME_ENDCOMPOSITION:
			return CPythonIME::Instance().WMEndComposition(hWnd, uiMsg, wParam, lParam);
			break;

		case WM_IME_NOTIFY:
			return CPythonIME::Instance().WMNotify(hWnd, uiMsg, wParam, lParam);
			break;

		case WM_IME_SETCONTEXT:
			lParam &= ~(ISC_SHOWUICOMPOSITIONWINDOW | ISC_SHOWUIALLCANDIDATEWINDOW);
			break;

		case WM_CHAR:
			return CPythonIME::Instance().WMChar(hWnd, uiMsg, wParam, lParam);
			break;

		case WM_KEYDOWN:
			OnIMEKeyDown(LOWORD(wParam));
			break;

		case WM_LBUTTONDOWN:
			SafeSetCapture();

			if (ELTimer_GetMSec() - m_dwLButtonDownTime < c_DoubleClickTime &&
				abs(LOWORD(lParam) - s_xDownPosition) < c_DoubleClickBox &&
				abs(HIWORD(lParam) - s_yDownPosition) < c_DoubleClickBox)
			{
				m_dwLButtonDownTime = 0;

				OnMouseLeftButtonDoubleClick(short(LOWORD(lParam)), short(HIWORD(lParam)));
			}
			else
			{
				m_dwLButtonDownTime = ELTimer_GetMSec();

				OnMouseLeftButtonDown(short(LOWORD(lParam)), short(HIWORD(lParam)));
			}

			s_xDownPosition = LOWORD(lParam);
			s_yDownPosition = HIWORD(lParam);
			return 0;

		case WM_LBUTTONUP:
			m_dwLButtonUpTime = ELTimer_GetMSec();

			if (hWnd == GetCapture())
			{
				SafeReleaseCapture();
				OnMouseLeftButtonUp(short(LOWORD(lParam)), short(HIWORD(lParam)));
			}
			return 0;

		case WM_MBUTTONDOWN:
			SafeSetCapture();

			UI::CWindowManager::Instance().RunMouseMiddleButtonDown(short(LOWORD(lParam)), short(HIWORD(lParam)));
//			OnMouseMiddleButtonDown(short(LOWORD(lParam)), short(HIWORD(lParam)));
			break;

		case WM_MBUTTONUP:
			if (GetCapture() == hWnd)
			{
				SafeReleaseCapture();

				UI::CWindowManager::Instance().RunMouseMiddleButtonUp(short(LOWORD(lParam)), short(HIWORD(lParam)));
//				OnMouseMiddleButtonUp(short(LOWORD(lParam)), short(HIWORD(lParam)));
			}
			break;

		case WM_RBUTTONDOWN:
			SafeSetCapture();
			OnMouseRightButtonDown(short(LOWORD(lParam)), short(HIWORD(lParam)));
			return 0;

		case WM_RBUTTONUP:
			if (hWnd == GetCapture())
			{
				SafeReleaseCapture();

				OnMouseRightButtonUp(short(LOWORD(lParam)), short(HIWORD(lParam)));
			}
			return 0;

		case WM_MOUSEWHEEL:
			if (WebBrowser_IsVisible())
			{
			}
			else
			{
#ifdef ENABLE_MOUSEWHEEL_EVENT
				if (OnMouseWheelEvent(GET_WHEEL_DELTA_WPARAM(wParam)))
					break;
#endif

				OnMouseWheel(short(HIWORD(wParam)));
			}
			break;

		case WM_ENTERSIZEMOVE:
			s_bIsSizingMove = true;
			break;

		case WM_SIZE:
			if (wParam == SIZE_MINIMIZED || ::IsIconic(hWnd))
			{
				m_isMinimizedWnd = true;
				s_bIsSizingMove = false;
			}
			else
			{
				bool bWasMinimized = m_isMinimizedWnd;
				m_isMinimizedWnd = false;

				if ((wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) && !s_bIsSizingMove)
				{
					RECT rcWnd{};
					::GetClientRect(hWnd, &rcWnd);

					UINT uWidth = rcWnd.right - rcWnd.left;
					UINT uHeight = rcWnd.bottom - rcWnd.top;

					// Ignore transitional DWM iconic rects (< 320x240) and skip redundant resets if size didn't change on restore
					if (uWidth >= 320 && uHeight >= 240)
					{
						if (!bWasMinimized || uWidth != m_dwWidth || uHeight != m_dwHeight)
						{
							OnSizeChange(uWidth, uHeight);
						}
					}
				}
			}
			break;

		case WM_EXITSIZEMOVE:
			{
				s_bIsSizingMove = false;

				if (!::IsIconic(hWnd))
				{
					RECT rcWnd{};
					::GetClientRect(hWnd, &rcWnd);

					UINT uWidth = rcWnd.right - rcWnd.left;
					UINT uHeight = rcWnd.bottom - rcWnd.top;
					if (uWidth >= 320 && uHeight >= 240 && (uWidth != m_dwWidth || uHeight != m_dwHeight))
					{
						OnSizeChange(uWidth, uHeight);
					}
				}
			}
			break;

		case WM_SYSKEYUP:
			switch(LOWORD(wParam))
			{
				case 18:
					return FALSE;
					break;
				case VK_F10:
					break;
			}
			break;

#ifdef ENABLE_MINI_WINDOW_SYSTEM
		case WM_SETCURSOR:
			{
				if (LOWORD(lParam) != HTCLIENT)
					break; 

				if (m_hCurrentCursor)
				{
					SetCursor((HCURSOR) m_hCurrentCursor);
				}
				else
				{
					SetCursor(nullptr);
				}
			}
			return TRUE;
#else
		case WM_SETCURSOR:
			if (IsActive())
			{
				if (m_bCursorVisible && CURSOR_MODE_HARDWARE == m_iCursorMode)
				{
					SetCursor((HCURSOR) m_hCurrentCursor);
					return 0;
				}
				else
				{
					SetCursor(nullptr);
					return 0;
				}
			}
			break;
#endif

		case WM_CLOSE:
#ifdef _DEBUG
			PostQuitMessage(0);
#else
			RunPressExitKey();
#endif
			return 0;

		case WM_DESTROY:
			return 0;
		default:
			//Tracenf("%x msg %x", timeGetTime(), uiMsg);
			break;
	}

	return CMSApplication::WindowProcedure(hWnd, uiMsg, wParam, lParam);
}
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
