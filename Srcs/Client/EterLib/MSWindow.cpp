#include "StdAfx.h"
#include "MsWindow.h"
#include "../UserInterface/Locale_inc.h"
#ifdef ENABLE_MINI_WINDOW_SYSTEM
#include "../UserInterface/MiniMode.h"
#include "resource.h"
#ifndef IDC_CURSOR_HSIZE
#define IDC_CURSOR_HSIZE 140
#define IDC_CURSOR_VSIZE 141
#define IDC_CURSOR_HVSIZE 142
#define IDM_ALWAYS_ON_TOP 0x0010
#endif
#endif

#include <windowsx.h>

CMSWindow::TWindowClassSet CMSWindow::ms_stWCSet;
HINSTANCE CMSWindow::ms_hInstance = NULL;

LRESULT CALLBACK MSWindowProcedure(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	CMSWindow * pWnd = (CMSWindow *) GetWindowLong(hWnd, GWL_USERDATA);

	if (pWnd)
		return pWnd->WindowProcedure(hWnd, uiMsg, wParam, lParam);

	return DefWindowProc(hWnd, uiMsg, wParam, lParam);
}

#ifdef ENABLE_MINI_WINDOW_SYSTEM
LRESULT CMSWindow::WindowProcedure(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uiMsg)
	{
		case WM_SIZE:
		{
			OnSize(wParam, lParam);
			break;
		}
		case WM_GETMINMAXINFO:
		{
			if (!g_miniMode.IsEnabled() && m_hWnd)
			{
				RECT rcMin = { 0, 0, 800, 600 };
				AdjustWindowRectEx(&rcMin, GetWindowStyle(m_hWnd), GetMenu(m_hWnd) != NULL, GetWindowExStyle(m_hWnd));

				MINMAXINFO* pMMI = (MINMAXINFO*)lParam;
				pMMI->ptMinTrackSize.x = rcMin.right - rcMin.left;
				pMMI->ptMinTrackSize.y = rcMin.bottom - rcMin.top;
				return 0;
			}
			break;
		}
		case WM_NCLBUTTONDOWN:
		{
			if (wParam == HTMAXBUTTON)
			{
				g_miniMode.Toggle(m_hWnd);
				return 0;
			}
			break;
		}

		case WM_NCLBUTTONDBLCLK:
		{
			if (wParam == HTMAXBUTTON || wParam == HTCAPTION)
			{
				g_miniMode.ResetWindowSize(m_hWnd);
				return 0;
			}
			break;
		}


		case WM_NCHITTEST:
		{
			LRESULT hit = ::DefWindowProc(m_hWnd, uiMsg, wParam, lParam);
			
			if (hit == HTLEFT || hit == HTRIGHT || hit == HTTOP || hit == HTBOTTOM ||
				hit == HTTOPLEFT || hit == HTTOPRIGHT || hit == HTBOTTOMLEFT || hit == HTBOTTOMRIGHT)
			{
				return hit;
			}
			break;
		}
		
		case WM_SETCURSOR:
		{
			UINT hitCode = LOWORD(lParam);

			if (hitCode != HTCLIENT && hitCode != HTNOWHERE)
			{
				HCURSOR hNewCursor = NULL;
				HINSTANCE hInst = ::GetModuleHandle(NULL);

				if (hitCode == HTLEFT || hitCode == HTRIGHT)
					hNewCursor = ::LoadCursor(hInst, MAKEINTRESOURCE(IDC_CURSOR_HSIZE));
				else if (hitCode == HTTOP || hitCode == HTBOTTOM)
					hNewCursor = ::LoadCursor(hInst, MAKEINTRESOURCE(IDC_CURSOR_VSIZE));
				else if (hitCode == HTTOPLEFT || hitCode == HTBOTTOMRIGHT || hitCode == HTTOPRIGHT || hitCode == HTBOTTOMLEFT)
					hNewCursor = ::LoadCursor(hInst, MAKEINTRESOURCE(IDC_CURSOR_HVSIZE));

				if (hNewCursor)
				{
					::SetCursor(hNewCursor);
					return TRUE;
				}
			}
			break;
		}

		case WM_ACTIVATEAPP:
		{
			m_isActive = (wParam == WA_ACTIVE) || (wParam == WA_CLICKACTIVE);
			break;
		}

		case WM_SYSCOMMAND:
		{
			const UINT cmd = (UINT)(wParam & 0xFFF0);
			
			// Always on Top Control
			if (cmd == IDM_ALWAYS_ON_TOP)
			{
				DWORD dwExStyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);
				bool isTopmost = (dwExStyle & WS_EX_TOPMOST) != 0;

				if (isTopmost)
				{
					::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
					::ModifyMenu(::GetSystemMenu(m_hWnd, FALSE), IDM_ALWAYS_ON_TOP, MF_BYCOMMAND | MF_STRING, IDM_ALWAYS_ON_TOP, "Fixed window");
				}
				else
				{
					::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
					::ModifyMenu(::GetSystemMenu(m_hWnd, FALSE), IDM_ALWAYS_ON_TOP, MF_BYCOMMAND | MF_STRING, IDM_ALWAYS_ON_TOP, "Unfix window");
				}
				return 0;
			}

			// Mini Mode Taskbar Controls
			if (g_miniMode.IsEnabled() && (cmd == SC_RESTORE || cmd == SC_MAXIMIZE))
			{
				g_miniMode.LeaveIfEnabled(m_hWnd);
				return 0;
			}
			
			if (cmd == SC_MAXIMIZE)
			{
				g_miniMode.Toggle(m_hWnd);
				return 0;
			}
			break;
		}
	}

	return ::DefWindowProc(hWnd, uiMsg, wParam, lParam);
}
#else
LRESULT CMSWindow::WindowProcedure(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uiMsg)
	{
		case WM_SIZE:
			OnSize(wParam, lParam);
			break;

		case WM_ACTIVATEAPP:
			m_isActive = (wParam == WA_ACTIVE) || (wParam == WA_CLICKACTIVE);
			break;
	}

	return DefWindowProc(hWnd, uiMsg, wParam, lParam);
}
#endif

void CMSWindow::OnSize(WPARAM wParam, LPARAM /*lParam*/)
{
	if (wParam == SIZE_MINIMIZED)
	{
		InvalidateRect(m_hWnd, NULL, true);
		m_isActive = false;
		m_isVisible = false;
	}
	else
	{
		m_isActive = true;
		m_isVisible = true;
	}
}

void CMSWindow::Destroy()
{
	if (!m_hWnd)
		return;

	if (IsWindow(m_hWnd))
		DestroyWindow(m_hWnd);

	m_hWnd = NULL;
	m_isVisible = false;
}

bool CMSWindow::Create(const char* c_szName, int brush, DWORD cs, DWORD ws, HICON hIcon, int iCursorResource)
{
	//assert(ms_hInstance != NULL);
	Destroy();

	const char* c_szClassName = RegisterWindowClass(cs, brush, MSWindowProcedure, hIcon, iCursorResource);

	m_hWnd = CreateWindow(
						c_szClassName,
						c_szName,
						ws,
						0, 0, 0, 0,
						NULL,
						NULL,
						ms_hInstance,
						NULL);

	if (!m_hWnd)
		return false;

#ifdef ENABLE_MINI_WINDOW_SYSTEM
	HMENU hSysMenu = ::GetSystemMenu(m_hWnd, FALSE);
	if (hSysMenu)
	{
		::AppendMenu(hSysMenu, MF_SEPARATOR, 0, NULL);
		::AppendMenu(hSysMenu, MF_STRING, IDM_ALWAYS_ON_TOP, "Fixed window");
	}
#endif

	SetWindowLong(m_hWnd, GWL_USERDATA, (DWORD) this);
	//DestroyWindow(ImmGetDefaultIMEWnd(m_hWnd));

	return true;
}

void CMSWindow::SetVisibleMode(bool isVisible)
{
	m_isVisible = isVisible;

	if (m_isVisible)
	{
		ShowWindow(m_hWnd, SW_SHOW);
	}
	else
	{
		ShowWindow(m_hWnd, SW_HIDE);
	}
}

void CMSWindow::Show()
{
	m_isVisible = true;
	ShowWindow(m_hWnd, SW_SHOW);
}

void CMSWindow::Hide()
{
	m_isVisible = false;
	ShowWindow(m_hWnd, SW_HIDE);
}

bool CMSWindow::IsVisible()
{
	return m_isVisible;
}

bool CMSWindow::IsActive()
{
	return m_isActive;
}

HINSTANCE CMSWindow::GetInstance()
{
	return ms_hInstance;
}

HWND CMSWindow::GetWindowHandle()
{
	return m_hWnd;
}

int	CMSWindow::GetScreenWidth()
{
	return GetSystemMetrics(SM_CXSCREEN);
}

int	CMSWindow::GetScreenHeight()
{
	return GetSystemMetrics(SM_CYSCREEN);
}

void CMSWindow::GetWindowRect(RECT* prc)
{
	::GetWindowRect(m_hWnd, prc);
}

void CMSWindow::GetClientRect(RECT* prc)
{
	::GetClientRect(m_hWnd, prc);
}

void CMSWindow::GetMousePosition(POINT* ppt)
{
	GetCursorPos(ppt);
	ScreenToClient(m_hWnd, ppt);
}

void CMSWindow::SetPosition(int x, int y)
{
	SetWindowPos(m_hWnd, NULL, x, y, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
}

void CMSWindow::SetCenterPosition()
{
	RECT rc;

	GetClientRect(&rc);

	int windowWidth = rc.right - rc.left;
	int windowHeight = rc.bottom - rc.top;

	SetPosition((GetScreenWidth()-windowWidth)/2, (GetScreenHeight()-windowHeight)/2);
}

void CMSWindow::AdjustSize(int width, int height)
{
	SetRect(&m_rect, 0, 0, width, height);

	AdjustWindowRectEx(&m_rect,
						GetWindowStyle(m_hWnd),
						GetMenu(m_hWnd ) != NULL,
						GetWindowExStyle(m_hWnd ) );

	MoveWindow
	(
		m_hWnd,
		0,
		0,
		m_rect.right - m_rect.left,
		m_rect.bottom - m_rect.top,
		FALSE
	);
}

void CMSWindow::SetText(const char* c_szText)
{
	SetWindowText(m_hWnd, c_szText);
}

void CMSWindow::SetSize(int width, int height)
{
	SetWindowPos(m_hWnd, NULL, 0, 0, width, height, SWP_NOZORDER|SWP_NOMOVE);
}

const char * CMSWindow::RegisterWindowClass(DWORD style, int brush, WNDPROC pfnWndProc, HICON hIcon, int iCursorResource)
{
	char szClassName[1024];
	sprintf(szClassName, "eter - s%x:b%x:p:%x", style, brush, (DWORD) pfnWndProc);

	TWindowClassSet::iterator f = ms_stWCSet.find((char*) szClassName);

	if (f != ms_stWCSet.end())
		return *f;

	const char* c_szStaticClassName = stl_static_string(szClassName).c_str();

	ms_stWCSet.insert((char * const) c_szStaticClassName);

	WNDCLASS wc;

	wc.style			= 0;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.lpfnWndProc		= pfnWndProc;
	wc.hCursor			= LoadCursor(ms_hInstance, MAKEINTRESOURCE(iCursorResource));
	wc.hIcon			= hIcon ? hIcon : LoadIcon(ms_hInstance, IDI_APPLICATION);
	wc.hbrBackground	= (HBRUSH) GetStockObject(brush);
	wc.hInstance		= ms_hInstance;
	wc.lpszClassName	= c_szStaticClassName;
	wc.lpszMenuName		= "";

	if (!RegisterClass(&wc))
		return "";

	return c_szStaticClassName;
}

CMSWindow::CMSWindow()
{
	m_hWnd=NULL;
	m_isVisible=false;
}

CMSWindow::~CMSWindow()
{
}
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
