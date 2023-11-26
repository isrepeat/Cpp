#include <Windows.h>
#include <stdio.h>
#include <iostream>

using namespace std;


LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    RECT rect;
    switch (message)
    {
    case WM_PAINT:
    {
        hdc = BeginPaint(hwnd, &ps);
        RECT rect = { 0, 0, 200, 200 };
        HBRUSH blueBrush = CreateSolidBrush(RGB(0, 0, 255));
        FillRect(hdc, &rect, blueBrush);
        EndPaint(hwnd, &ps);
    }
    break;
    case WM_SIZE:
    {
        //HWND targetWindow = (HWND)0x00060BA6; //Test window
        //HWND targetWindow = FindWindowW(L"MyWindowClass", NULL);
        HWND targetWindow = GetDesktopWindow();
        GetWindowRect(targetWindow, &rect);

        float m = (200.0 / 1920.0);
        float n = (200.0 / 1040.0);

        int wh = (rect.right - rect.left) * m;
        int ht = (rect.bottom - rect.top) * n;

        int x = 100 * m;
        int y = 100 * n;

        SetWindowPos(hwnd, HWND_TOPMOST, x + rect.left, y + rect.top, wh, ht, SWP_SHOWWINDOW);
    }
    break;
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    }
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
};

HINSTANCE hinst;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevinstance, PSTR szCmdLine, int iCmdShow) {
    HWND hwnd;

    hinst = GetModuleHandle(NULL);
    // create a window class:
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hinst;
    wc.lpszClassName = L"win32";

    // register class with operating system:
    RegisterClass(&wc);

    // create and show window:
    //hwnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TRANSPARENT, L"win32", L"My program", WS_POPUP & ~WS_BORDER, 0, 0, 0, 0, NULL, NULL, hinst, NULL);
    hwnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW, L"win32", L"My program", NULL, 0, 0, 0, 0, NULL, NULL, hinst, NULL);

    SetLayeredWindowAttributes(hwnd, NULL, 100, LWA_ALPHA);
    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, SW_SHOW);


    MSG msg = {};

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

}