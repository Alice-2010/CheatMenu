#include "ui.h"
#include "graphics_device.h"

#include <winuser.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

static UI ui;

LRESULT WINAPI WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return 1;
    if (msg == WM_DESTROY)
    {
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hwnd, msg, wParam, lParam);
}

int main(int argc, char** argv)
{
    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW), CS_CLASSDC, WindowProc, 0L, 0L, GetModuleHandleW(NULL), NULL, NULL, NULL, NULL, L"Alice Cheat Menu", NULL };
    if (!::RegisterClassExW(&wc))
        return 1;
    HWND hwnd = ::CreateWindowExW(0, wc.lpszClassName, L"Alice Cheat Menu", WS_OVERLAPPEDWINDOW, 100, 100, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, wc.hInstance, NULL);
    if (!hwnd)
    {
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    ui.Initialize(hwnd);

    MSG msg = {};
    while (!ui.m_bIsClosing)
    {
        while (::PeekMessageW(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
            if (msg.message == WM_QUIT)
                ui.m_bIsClosing = true;
        }
        if (ui.m_bIsClosing)
            break;
        ui.Update();
    }

    ui.Shutdown();
    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 0;
}
