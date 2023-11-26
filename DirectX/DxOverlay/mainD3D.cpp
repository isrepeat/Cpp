////#define WIN32_LEAN_AND_MEAN
////#define NOMINMAX
////#define UNICODE
//#include "DxDevice.h"
////#include <Windows.h>
//#include <cassert>
//
////#ifdef DrawText
////#undef DrawText
////#endif // DrawText
//
//
//bool global_windowDidResize = false;
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
//        global_windowDidResize = true;
//        break;
//    }
//
//    default:
//        result = DefWindowProcW(hwnd, msg, wparam, lparam);
//    }
//    return result;
//}
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
//        RECT initialRect = { 0, 0, 1024, 768 };
//        AdjustWindowRectEx(&initialRect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);
//        LONG initialWidth = initialRect.right - initialRect.left;
//        LONG initialHeight = initialRect.bottom - initialRect.top;
//
//        hwnd = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW,
//            winClass.lpszClassName,
//            L"01. Initialising Direct3D 11",
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
//
//    DxDevice dxDev;
//
//
//    // Create D3D11 Device and Context
//    //ID3D11Device1* d3d11Device = dxDev.GetD3DDevice().Get();
//    //ID3D11DeviceContext1* d3d11DeviceContext;
////    {
////        ID3D11Device* baseDevice;
////        ID3D11DeviceContext* baseDeviceContext;
////        D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
////        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
////#if defined(DEBUG_BUILD)
////        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
////#endif
////
////        HRESULT hResult = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE,
////            0, creationFlags,
////            featureLevels, ARRAYSIZE(featureLevels),
////            D3D11_SDK_VERSION, &baseDevice,
////            0, &baseDeviceContext);
////        if (FAILED(hResult)) {
////            MessageBoxA(0, "D3D11CreateDevice() failed", "Fatal Error", MB_OK);
////            return GetLastError();
////        }
////
////        // Get 1.1 interface of D3D11 Device and Context
////        hResult = baseDevice->QueryInterface(__uuidof(ID3D11Device1), (void**)&d3d11Device);
////        assert(SUCCEEDED(hResult));
////        baseDevice->Release();
////
////        hResult = baseDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), (void**)&d3d11DeviceContext);
////        assert(SUCCEEDED(hResult));
////        baseDeviceContext->Release();
////    }
////
////#ifdef DEBUG_BUILD
////    // Set up debug layer to break on D3D11 errors
////    ID3D11Debug* d3dDebug = nullptr;
////    d3d11Device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug);
////    if (d3dDebug)
////    {
////        ID3D11InfoQueue* d3dInfoQueue = nullptr;
////        if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue)))
////        {
////            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
////            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
////            d3dInfoQueue->Release();
////        }
////        d3dDebug->Release();
////    }
////#endif
//
//    // Create Swap Chain
//    IDXGISwapChain1* d3d11SwapChain;
//    {
//        // Get DXGI Factory (needed to create Swap Chain)
//        IDXGIFactory2* dxgiFactory;
//        {
//            IDXGIDevice1* dxgiDevice;
//            hr = dxDev.GetD3DDevice()->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgiDevice);
//            assert(SUCCEEDED(hr));
//
//            IDXGIAdapter* dxgiAdapter;
//            hr = dxgiDevice->GetAdapter(&dxgiAdapter);
//            assert(SUCCEEDED(hr));
//            dxgiDevice->Release();
//
//            DXGI_ADAPTER_DESC adapterDesc;
//            dxgiAdapter->GetDesc(&adapterDesc);
//
//            OutputDebugStringA("Graphics Device: ");
//            OutputDebugStringW(adapterDesc.Description);
//
//            hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgiFactory);
//            assert(SUCCEEDED(hr));
//            dxgiAdapter->Release();
//        }
//
//        DXGI_SWAP_CHAIN_DESC1 d3d11SwapChainDesc = {};
//        d3d11SwapChainDesc.Width = 800; // use window width
//        d3d11SwapChainDesc.Height = 800; // use window height
//        d3d11SwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
//        d3d11SwapChainDesc.SampleDesc.Count = 1;
//        d3d11SwapChainDesc.SampleDesc.Quality = 0;
//        d3d11SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//        d3d11SwapChainDesc.BufferCount = 2;
//        d3d11SwapChainDesc.Scaling = DXGI_SCALING_STRETCH;
//        d3d11SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
//        d3d11SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
//        d3d11SwapChainDesc.Flags = 0;
//
//        HRESULT hResult = dxgiFactory->CreateSwapChainForHwnd(dxDev.GetD3DDevice().Get(), hwnd, &d3d11SwapChainDesc, 0, 0, &d3d11SwapChain);
//        assert(SUCCEEDED(hResult));
//
//        dxgiFactory->Release();
//    }
//
//    // Create Framebuffer Render Target
//    ID3D11RenderTargetView* d3d11FrameBufferView;
//    {
//        ID3D11Texture2D* d3d11FrameBuffer;
//        hr = d3d11SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3d11FrameBuffer);
//        assert(SUCCEEDED(hr));
//
//        hr = dxDev.GetD3DDevice()->CreateRenderTargetView(d3d11FrameBuffer, 0, &d3d11FrameBufferView);
//        assert(SUCCEEDED(hr));
//        d3d11FrameBuffer->Release();
//    }
//
//
//    // Create Font
//    Microsoft::WRL::ComPtr<IDWriteTextFormat> textFormat;
//    {
//        hr = dxDev.GetDwriteFactory()->CreateTextFormat(L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 42.0f, L"en-US", textFormat.GetAddressOf());
//        assert(SUCCEEDED(hr));
//    }
//
//    // Create brushes
//    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> blackBrush;
//    {
//        hr = dxDev.GetContext()->D2D()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), blackBrush.GetAddressOf());
//        assert(SUCCEEDED(hr));
//    }
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
//
//
//        if (global_windowDidResize) {
//            global_windowDidResize = false;
//
//            dxDev.GetContext()->D3D()->OMSetRenderTargets(0, 0, 0);
//            d3d11FrameBufferView->Release();
//
//            HRESULT res = d3d11SwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
//            assert(SUCCEEDED(res));
//
//            ID3D11Texture2D* d3d11FrameBuffer;
//            res = d3d11SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3d11FrameBuffer);
//            assert(SUCCEEDED(res));
//
//            res = dxDev.GetD3DDevice()->CreateRenderTargetView(d3d11FrameBuffer, NULL, &d3d11FrameBufferView);
//            assert(SUCCEEDED(res));
//            d3d11FrameBuffer->Release();
//        }
//
//
//        FLOAT backgroundColor[4] = { 0.1f, 0.2f, 0.6f, 1.0f };
//        dxDev.GetContext()->D3D()->ClearRenderTargetView(d3d11FrameBufferView, backgroundColor);
//        
//
//
//        static const WCHAR sc_helloWorld[] = L"Hello, World!";
//
//        // Retrieve the size of the render target.
//        D2D1_SIZE_F renderTargetSize = dxDev.GetContext()->D2D()->GetSize();
//
//        dxDev.GetContext()->D2D()->BeginDraw();
//
//        dxDev.GetContext()->D2D()->SetTransform(D2D1::Matrix3x2F::Identity());
//
//        dxDev.GetContext()->D2D()->Clear(D2D1::ColorF(D2D1::ColorF::White));
//
//        //dxDev.GetContext()->D2D()->DrawRectangle(D2D1::RectF(0, 0, renderTargetSize.width, renderTargetSize.height), blackBrush.Get());
//        dxDev.GetContext()->D2D()->DrawRectangle(D2D1::RectF(0, 0, 400, 400), blackBrush.Get());
//
//        //dxDev.GetContext()->D2D()->DrawTextW(
//        //    sc_helloWorld,
//        //    ARRAYSIZE(sc_helloWorld) - 1,
//        //    textFormat.Get(),
//        //    D2D1::RectF(0, 0, renderTargetSize.width, renderTargetSize.height),
//        //    blackBrush.Get()
//        //);
//
//        dxDev.GetContext()->D2D()->EndDraw();
//
//
//
//        d3d11SwapChain->Present(1, 0);
//    }
//
//    return 0;
//}