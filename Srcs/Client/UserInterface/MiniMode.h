#pragma once
#include <windows.h>

class CMiniMode
{
public:
    void Toggle(HWND hWnd);
    void ResetWindowSize(HWND hWnd);

    bool IsEnabled() const { return m_enabled; }

    void LeaveIfEnabled(HWND hWnd)
    {
        if (m_enabled) Leave(hWnd);
    }

private:
    bool m_enabled = false;

    WINDOWPLACEMENT m_wp{};
    LONG_PTR m_restoreStyle = 0;
    LONG_PTR m_restoreExStyle = 0;

    void Enter(HWND hWnd);
    void Leave(HWND hWnd);
};

extern CMiniMode g_miniMode;