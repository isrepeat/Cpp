#include <windows.h>
#include <windowsx.h>
#include "resource.h"


// Our DDB handle is a global variable.
HBITMAP hbm;


BOOL gdi01_OnCreate(
   HWND hwnd,
   LPCREATESTRUCT lpcs)
{
  // The bitmap should be stored as a resource in the exe file.
  // We pass the hInstance of the application, and the ID of the
  // bitmap to the LoadBitmap API function and it returns us an
  // HBITMAP to a DDB created from the resource data.
  HINSTANCE hInstance = GetWindowInstance(hwnd);
  hbm = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP1));
  
  return TRUE;
}

void gdi01_OnDestroy(
  HWND hwnd)
{
  // It is good practice to destroy all GDI objects when one is
  // finished with them. Usually the sooner the better. Bitmaps
  // can easilly expand to several megabytes of memory: A DDB is
  // stored in the format of the device it is compatible with:
  // On a 32 bit display a 256 color bitmap would be stored with
  // four bytes per pixel.
  DeleteObject(hbm);
  
  PostQuitMessage(0);
}

void gdi01_OnPaint(
  HWND hwnd)
{
  // Calling BeginPaint clears the update region that was set by calls
  // to InvalidateRect(). Once the update region is cleared no more
  // WM_PAINT messages will be sent to the window until InvalidateRect
  // is called again.
  PAINTSTRUCT ps;
  HDC hdc = BeginPaint(hwnd,&ps);
  
  // To paint with a DDB it first needs to be associated
  // with a memory device context. We make a DC that
  // is compatible with the screen by passing NULL to
  // CreateCompatibleDC.
  // Then we need to associate our saved bitmap with the
  // device context.
  
  HDC hdcMem = CreateCompatibleDC(NULL);
  HBITMAP hbmT = SelectBitmap(hdcMem,hbm);
  
  // Now, the BitBlt function is used to transfer the contents of the 
  // drawing surface from one DC to another. Before we can paint the
  // bitmap however we need to know how big it is. We call the GDI
  // function GetObject to get the relevent details.
  BITMAP bm;
  GetObject(hbm,sizeof(bm),&bm);
  
  BitBlt(hdc,0,0,bm.bmWidth,bm.bmHeight,hdcMem,0,0,SRCCOPY);
  
  // Now, clean up. A memory DC always has a drawing
  // surface in it. It is created with a 1X1 monochrome
  // bitmap that we saved earlier, and need to put back
  // before we destroy it.
  SelectBitmap(hdcMem,hbmT);
  DeleteDC(hdcMem);
  
  // EndPaint balances off the BeginPaint call.
  EndPaint(hwnd,&ps);
}  


LRESULT CALLBACK gdi01_WindowProc(
  HWND hwnd,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam)
{
  switch(uMsg)
  {
  HANDLE_MSG(hwnd,WM_CREATE,gdi01_OnCreate);
  HANDLE_MSG(hwnd,WM_DESTROY,gdi01_OnDestroy);
  HANDLE_MSG(hwnd,WM_PAINT,gdi01_OnPaint);
  }
  
  return DefWindowProc(hwnd,uMsg,wParam,lParam);
}  


void gdi01_RegisterClass(
  HINSTANCE hInstance)
{
  WNDCLASS wc = {
    0,
    gdi01_WindowProc,
    0,0,
    hInstance,
    LoadIcon(NULL,IDI_APPLICATION),
    LoadCursor(NULL,IDC_ARROW),
    (HBRUSH)(COLOR_WINDOW+1),
    NULL,
    "GDI01"
  };
  RegisterClass(&wc);
}


int WINAPI WinMain(
  HINSTANCE hInstance,
  HINSTANCE hInstPrev,
  LPSTR lpszCmdLine,
  int nCmdShow)
{
  gdi01_RegisterClass(hInstance);
  
  HWND hwnd = CreateWindowEx(
    0L,
    "GDI01","Show Bitmap",
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT,0,CW_USEDEFAULT,0,
    HWND_DESKTOP,NULL,hInstance,NULL);
    
  ShowWindow(hwnd,nCmdShow);
  UpdateWindow(hwnd);
  
  MSG msg;
  while(GetMessage(&msg,0,0,0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return msg.wParam;
}
