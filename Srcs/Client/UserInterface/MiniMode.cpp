#include "StdAfx.h"
#include "MiniMode.h"
#include "PythonApplication.h"

CMiniMode g_miniMode;

static RECT GetWorkArea()
{
    RECT rc{};
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
    return rc;
}

void CMiniMode::Toggle(HWND hWnd)
{
    if (!hWnd) return;
    if (m_enabled) Leave(hWnd);
    else Enter(hWnd);
}

void CMiniMode::Enter(HWND hWnd)
{
    m_wp = {};
    m_wp.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(hWnd, &m_wp);

    m_restoreStyle   = GetWindowLongPtr(hWnd, GWL_STYLE);
    m_restoreExStyle = GetWindowLongPtr(hWnd, GWL_EXSTYLE);

    RECT wa = GetWorkArea();
    const int miniW = 300;
    const int miniH = 200;
    int x = wa.right  - miniW - 10;
    int y = wa.bottom - miniH - 10;

    LONG_PTR style = m_restoreStyle | (WS_CAPTION | WS_THICKFRAME | WS_SYSMENU);
    SetWindowLongPtr(hWnd, GWL_STYLE, style);

    bool isPinned = (m_restoreExStyle & WS_EX_TOPMOST) != 0;
    ::SetWindowPos(
        hWnd,
        isPinned ? HWND_TOPMOST : HWND_NOTOPMOST,
        x, y, miniW, miniH,
        SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_SHOWWINDOW
    );

    m_enabled = true;
}

void CMiniMode::Leave(HWND hWnd)
{
    SetWindowLongPtr(hWnd, GWL_STYLE, m_restoreStyle);

    SetWindowPlacement(hWnd, &m_wp);

    bool isPinned = (m_restoreExStyle & WS_EX_TOPMOST) != 0;
    ::SetWindowPos(
        hWnd,
        isPinned ? HWND_TOPMOST : HWND_NOTOPMOST,
        0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED
    );

    m_enabled = false;
}

void CMiniMode::ResetWindowSize(HWND hWnd)
{
    if (!hWnd) return;

    if (m_enabled)
        Toggle(hWnd);

    CPythonSystem& rkSys = CPythonSystem::Instance();

    int iTargetWidth = rkSys.GetWidth();
    int iTargetHeight = rkSys.GetHeight();

    RECT rcWnd = { 0, 0, iTargetWidth, iTargetHeight };
    AdjustWindowRect(&rcWnd, GetWindowLong(hWnd, GWL_STYLE), FALSE);

    int finalWidth = rcWnd.right - rcWnd.left;
    int finalHeight = rcWnd.bottom - rcWnd.top;

    bool isPinned = (GetWindowLongPtr(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST) != 0;
    ::SetWindowPos(
        hWnd,
        isPinned ? HWND_TOPMOST : HWND_NOTOPMOST,
        0, 0, finalWidth, finalHeight,
        SWP_NOMOVE | SWP_FRAMECHANGED
    );
}