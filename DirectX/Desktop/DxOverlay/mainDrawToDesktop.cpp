#include "Drawer2D.h"
//#include <Windows.h>
#include <cassert>


Drawer2D drawer;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    LRESULT result = 0;
    switch (msg)
    {
    case WM_KEYDOWN:
    {
        if (wparam == VK_ESCAPE)
            DestroyWindow(hwnd);
        break;
    }
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        break;
    }

    case WM_SIZE:
    {
        RECT rc;
        ::GetClientRect(hwnd, &rc);
        drawer.Resize(Size(rc.right - rc.left, rc.bottom - rc.top));
        break;
    }

    default:
        result = DefWindowProcW(hwnd, msg, wparam, lparam);
    }
    return result;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
    HRESULT hr = S_OK;

    // Open a window
    HWND hwnd;
    {
        WNDCLASSEXW winClass = {};
        winClass.cbSize = sizeof(WNDCLASSEXW);
        winClass.style = CS_HREDRAW | CS_VREDRAW;
        winClass.lpfnWndProc = &WndProc;
        winClass.hInstance = hInstance;
        winClass.hIcon = LoadIconW(0, IDI_APPLICATION);
        winClass.hCursor = LoadCursorW(0, IDC_ARROW);
        winClass.lpszClassName = L"MyWindowClass";
        winClass.hIconSm = LoadIconW(0, IDI_APPLICATION);

        if (!RegisterClassExW(&winClass)) {
            MessageBoxA(0, "RegisterClassEx failed", "Fatal Error", MB_OK);
            return GetLastError();
        }

        RECT initialRect = { 0, 0, 1024, 768 };
        AdjustWindowRectEx(&initialRect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);
        LONG initialWidth = initialRect.right - initialRect.left;
        LONG initialHeight = initialRect.bottom - initialRect.top;

        hwnd = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW,
            winClass.lpszClassName,
            winClass.lpszClassName,
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT,
            initialWidth,
            initialHeight,
            0, 0, hInstance, 0);

        if (!hwnd) {
            MessageBoxA(0, "CreateWindowEx failed", "Fatal Error", MB_OK);
            return GetLastError();
        }
    }



    drawer.CreateSwapChain(hwnd);

    //DxDevice dxDev;


    //HWND hWndDesktop = GetDesktopWindow();
    //HDC hdcDesktop = GetDC(hWndDesktop);

    //// Get the dimensions of the client drawing area.
    //RECT rc;
    //GetClientRect(hWndDesktop, &rc);

    //// Create a DC render target.
    //D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
    //    D2D1_RENDER_TARGET_TYPE_DEFAULT,
    //    D2D1::PixelFormat(
    //        DXGI_FORMAT_B8G8R8A8_UNORM,
    //        D2D1_ALPHA_MODE_IGNORE),
    //    0,
    //    0,
    //    D2D1_RENDER_TARGET_USAGE_NONE,
    //    D2D1_FEATURE_LEVEL_DEFAULT
    //);

    //Microsoft::WRL::ComPtr<ID2D1DCRenderTarget> renderTargetDC;
    //hr = dxDev.GetD2DFactory()->CreateDCRenderTarget(&props, renderTargetDC.GetAddressOf()); 

    //// Bind the DC to the DC render target.
    //hr = renderTargetDC->BindDC(hdcDesktop, &rc);
    


    // Main Loop
    bool isRunning = true;
    while (isRunning)
    {
        MSG msg = {};
        while (PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                isRunning = false;
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }


        drawer.Render();
    }

    return 0;
}