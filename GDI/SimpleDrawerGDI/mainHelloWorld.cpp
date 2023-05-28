////#include "Drawer2D.h"
//#include <Windows.h>
//#include <cassert>
//#include <string>
//
//LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
//
//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
//{
//    HRESULT hr = S_OK;
//
//    // Open a window
//    HWND hwnd;
//    {
//        WNDCLASSEXW winClass = {};
//        winClass.cbSize = sizeof(WNDCLASSEXW);
//        winClass.style = CS_HREDRAW | CS_VREDRAW;
//        winClass.lpfnWndProc = &WndProc;
//        winClass.hInstance = hInstance;
//        winClass.hIcon = LoadIconW(0, IDI_APPLICATION);
//        winClass.hCursor = LoadCursorW(0, IDC_ARROW);
//        winClass.lpszClassName = L"MyWindowClass";
//        winClass.hIconSm = LoadIconW(0, IDI_APPLICATION);
//
//        if (!RegisterClassExW(&winClass)) {
//            MessageBoxA(0, "RegisterClassEx failed", "Fatal Error", MB_OK);
//            return GetLastError();
//        }
//
//        RECT initialRect = { 0, 0, 800, 600 };
//        AdjustWindowRectEx(&initialRect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);
//        LONG initialWidth = initialRect.right - initialRect.left;
//        LONG initialHeight = initialRect.bottom - initialRect.top;
//
//        hwnd = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW,
//            winClass.lpszClassName,
//            winClass.lpszClassName,
//            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
//            CW_USEDEFAULT, CW_USEDEFAULT,
//            initialWidth,
//            initialHeight,
//            0, 0, hInstance, 0);
//
//        if (!hwnd) {
//            MessageBoxA(0, "CreateWindowEx failed", "Fatal Error", MB_OK);
//            return GetLastError();
//        }
//    }
//
//
//    // Main Loop
//    bool isRunning = true;
//    while (isRunning)
//    {
//        MSG msg = {};
//        while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
//        {
//            if (msg.message == WM_QUIT)
//                isRunning = false;
//            TranslateMessage(&msg);
//            DispatchMessageW(&msg);
//        }
//    }
//
//    return 0;
//}
//
//LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
//{
//    LRESULT result = 0;
//    switch (msg)
//    {
//    case WM_KEYDOWN:
//    {
//        if (wparam == VK_ESCAPE)
//            DestroyWindow(hwnd);
//        break;
//    }
//    case WM_DESTROY:
//    {
//        PostQuitMessage(0);
//        break;
//    }
//
//    case WM_SIZE:
//    {
//        RECT rc;
//        ::GetClientRect(hwnd, &rc);
//        break;
//    }
//
//    case WM_PAINT:
//    {
//        PAINTSTRUCT ps;
//        HDC hdc = BeginPaint(hwnd, &ps);
//        Ellipse(hdc, 30, 30, 120, 90);
//
//
//        DWORD color = GetSysColor(COLOR_BTNFACE);
//        SetBkColor(hdc, color);
//
//        HFONT hFont, holdFont;
//        hFont = CreateFontW(50, 0, 0, 0, FW_MEDIUM, 0, 0, 0, 0, 0, 0, 0, 0, L"Georgia");
//        holdFont = (HFONT)SelectObject(hdc, hFont);
//
//        std::wstring inText = L"Hello wolrd";
//
//        RECT TextArea = { 0, 0, 250, 50 };
//        //DrawTextW(hdc, inText.c_str(), inText.size(), &TextArea, DT_CALCRECT);
//        if ((TextArea.right > TextArea.left) && (TextArea.bottom > TextArea.top)) {
//            DrawTextW(hdc, inText.c_str(), inText.size(), &TextArea, DT_NOCLIP);
//        }
//
//        SelectObject(hdc, holdFont);
//        DeleteObject(hFont);
//
//        EndPaint(hwnd, &ps);
//        break;
//    }
//
//    default:
//        result = DefWindowProcW(hwnd, msg, wparam, lparam);
//    }
//    return result;
//}