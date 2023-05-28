////#include "Drawer2D.h"
//#include <Windows.h>
//#include <cassert>
//#include <string>
//#include <thread>
//
//#pragma comment(lib, "Msimg32")
//
//LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
//void Render(HWND hwnd);
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
//            L"MyWindowClassTitle",
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
//    //auto th = std::thread([] {
//    //    while (true) {
//    //        HWND hwndDesktop = GetDesktopWindow();
//    //        Render(hwndDesktop);
//    //        Sleep(1);
//    //    }
//    //    });
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
//    //if (th.joinable())
//    //    th.join();
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
//        //InvalidateRect(hwnd, &rc, false);
//        break;
//    }
//
//    case WM_PAINT:
//    {
//        PAINTSTRUCT ps;
//        HDC hdc = BeginPaint(hwnd, &ps);
//        
//
//        RECT rc;
//        GetClientRect(hwnd, &rc);
//        rc.right = rc.right / 2;
//        
//        HBRUSH brush = CreateSolidBrush(RGB(0, 0, 100));
//        FillRect(hdc, &rc, brush);
//        DeleteObject(brush);
//
//        ////HWND hwndDesktop = GetDesktopWindow();
//        //Render(hwnd);
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
//
////void Render(HWND hwnd) {
////    HDC hdcTarget = GetDC(hwnd);
////
////    COLORREF bkColorOld, textColorOld;
////    int bkModeOld;
////
////    bkModeOld = SetBkMode(hdcTarget, TRANSPARENT);
////    bkColorOld = SetBkColor(hdcTarget, RGB(200, 200, 0));
////    textColorOld = SetTextColor(hdcTarget, RGB(0, 255, 0));
////
////    Ellipse(hdcTarget, 30, 30, 120, 90);
////
////    HFONT hFont, hFontOld;
////    hFont = CreateFontW(50, 0, 0, 0, FW_MEDIUM, 0, 0, 0, 0, 0, 0, 0, 0, L"Georgia");
////    hFontOld = (HFONT)SelectObject(hdcTarget, hFont);
////
////    std::wstring inText = L"Hello wolrd";
////
////    RECT TextArea = { 0, 0, 250, 50 };
////    //DrawTextW(hdc, inText.c_str(), inText.size(), &TextArea, DT_CALCRECT);
////    if ((TextArea.right > TextArea.left) && (TextArea.bottom > TextArea.top)) {
////        DrawTextW(hdcTarget, inText.c_str(), inText.size(), &TextArea, DT_NOCLIP);
////    }
////
////
////    // Restore
////    SetBkMode(hdcTarget, bkModeOld);
////    SetBkColor(hdcTarget, bkColorOld);
////    SetTextColor(hdcTarget, textColorOld);
////    SelectObject(hdcTarget, hFontOld);
////    DeleteObject(hFont);
////}
//
//
//
//HBITMAP CreateAlphaTextBitmap(const std::wstring& inText, HFONT inFont, COLORREF inColour) {
//    int TextLength = inText.size();
//    if (TextLength <= 0) return NULL;
//
//    // Create DC and select font into it
//    HDC hTextDC = CreateCompatibleDC(NULL);
//    HFONT hOldFont = (HFONT)SelectObject(hTextDC, inFont);
//    HBITMAP hMyDIB = NULL;
//
//    // Get text area
//    RECT TextArea = { 0, 0, 0, 0 };
//    DrawTextW(hTextDC, inText.c_str(), TextLength, &TextArea, DT_CALCRECT);
//
//    if ((TextArea.right > TextArea.left) && (TextArea.bottom > TextArea.top)) {
//        BITMAPINFOHEADER BMIH;
//        memset(&BMIH, 0x0, sizeof(BITMAPINFOHEADER));
//
//        void* pvBits = NULL;
//
//        // Specify DIB setup
//        BMIH.biSize = sizeof(BMIH);
//        BMIH.biWidth = TextArea.right - TextArea.left;
//        BMIH.biHeight = TextArea.bottom - TextArea.top;
//        BMIH.biPlanes = 1;
//        BMIH.biBitCount = 32;
//        BMIH.biCompression = BI_RGB;
//
//        // Create and select DIB into DC
//        hMyDIB = CreateDIBSection(hTextDC, (LPBITMAPINFO)&BMIH, 0, (LPVOID*)&pvBits, NULL, 0);
//        HBITMAP hOldBMP = (HBITMAP)SelectObject(hTextDC, hMyDIB);
//
//        if (hOldBMP != NULL) {
//            // Set up DC properties
//            SetTextColor(hTextDC, 0x00FFFFFF);
//            SetBkColor(hTextDC, 0x00000000);
//            SetBkMode(hTextDC, OPAQUE);
//
//            // Draw text to buffer
//            DrawTextW(hTextDC, inText.c_str(), TextLength, &TextArea, DT_NOCLIP);
//
//            BYTE* DataPtr = (BYTE*)pvBits;
//            BYTE FillR = GetRValue(inColour);
//            BYTE FillG = GetGValue(inColour);
//            BYTE FillB = GetBValue(inColour);
//            BYTE ThisA;
//
//            for (int LoopY = 0; LoopY < BMIH.biHeight; LoopY++) {
//                for (int LoopX = 0; LoopX < BMIH.biWidth; LoopX++) {
//                    ThisA = *DataPtr; // Move alpha and pre-multiply with RGB
//                    *DataPtr++ = (FillB * ThisA) >> 8;
//                    *DataPtr++ = (FillG * ThisA) >> 8;
//                    *DataPtr++ = (FillR * ThisA) >> 8;
//                    *DataPtr++ = ThisA; // Set Alpha
//                }
//            }
//
//            // De-select bitmap
//            SelectObject(hTextDC, hOldBMP);
//        }
//    }
//
//    // De-select font and destroy temp DC
//    SelectObject(hTextDC, hOldFont);
//    DeleteDC(hTextDC);
//
//    // Return DIBSection
//    return hMyDIB;
//}
//
//
//void Render(HWND hwnd) {
//    HDC hdcTarget = GetDC(hwnd);
//
//    //RECT TextArea = { 0, 0, 0, 0 };
//    HFONT TempFont = CreateFontW(150, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, L"Georgia");
//    HBITMAP MyBMP = CreateAlphaTextBitmap(L"Demo Text", TempFont, 0x00FF00);
//    DeleteObject(TempFont);
//
//    if (MyBMP) { // Create temporary DC and select new Bitmap into it
//        HDC hTempDC = CreateCompatibleDC(hdcTarget);
//        HBITMAP hOldBMP = (HBITMAP)SelectObject(hTempDC, MyBMP);
//
//        if (hOldBMP) {
//            BITMAP BMInf; // Get Bitmap image size
//            GetObject(MyBMP, sizeof(BITMAP), &BMInf);
//
//            // Fill blend function and blend new text to window
//            BLENDFUNCTION bf;
//            bf.BlendOp = AC_SRC_OVER;
//            bf.BlendFlags = 0;
//            bf.SourceConstantAlpha = 64;
//            bf.AlphaFormat = AC_SRC_ALPHA;
//            AlphaBlend(hdcTarget, 0, 0, BMInf.bmWidth, BMInf.bmHeight,
//                hTempDC, 0, 0, BMInf.bmWidth, BMInf.bmHeight, bf);
//
//            //RECT rc = { 0,0, BMInf.bmWidth, BMInf.bmHeight };
//            //InvalidateRect(hwnd, &rc, false);
//            // Clean up
//            SelectObject(hTempDC, hOldBMP);
//            DeleteObject(MyBMP);
//            DeleteDC(hTempDC);
//
//            RECT invalidRect = { 0, 0, BMInf.bmWidth, BMInf.bmHeight };
//            //GetClientRect(hwnd, &invalidRect);
//            RedrawWindow(hwnd, &invalidRect, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
//        }
//    }
//}
