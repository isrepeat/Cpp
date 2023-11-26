#include <windows.h>
#include <windowsx.h>
#include "resource.h"
#include "bmpapi.h"


// Our DDB handles are global variables.
// hbmImage holds the handle of the color image
// hbmMask holds the handle of the mono mask.
HBITMAP hbmBackground;
HBITMAP hbmImage;
HBITMAP hbmMask;
HPALETTE hpal;

BOOL gdi02_OnCreate(
   HWND hwnd,
   LPCREATESTRUCT lpcs)
{
  HINSTANCE hInstance = GetWindowInstance(hwnd);
  
  // We use the helper functions in bmpapi.cpp to help us out.
  hpal = CreatePaletteFromBitmapResource(hInstance,MAKEINTRESOURCE(IDB_BITMAP1));
  
  // Before any bitmaps can be created using a palette, the palette has to be
  // realized atleast once. This builds a cache of the physical palette that
  // will be used to set the bitmaps bytes.
  HDC hdc = GetDC(hwnd);
  HPALETTE hpalT = SelectPalette(hdc,hpal,FALSE);
  RealizePalette(hdc);
  SelectPalette(hdc,hpalT,FALSE);
  ReleaseDC(hwnd,hdc);
  
  hbmImage = CreateDDBFromBitmapResource(hInstance,MAKEINTRESOURCE(IDB_BITMAP1),hpal);
  
  hbmBackground = CreateDDBFromBitmapResource(hInstance,MAKEINTRESOURCE(IDB_BITMAP3),hpal);

  // Extract the colorref of the top left pixel so we can pass it thru to the
  // Monochrome mask creation function.
  hdc = CreateCompatibleDC(NULL);  
  HBITMAP hbmT = SelectBitmap(hdc,hbmImage);
  COLORREF clrTopLeft = GetPixel(hdc,0,0);
  SelectBitmap(hdc,hbmT);
  DeleteDC(hdc);
  
  hbmMask = CreateMonoMask(hbmImage,hpal,clrTopLeft);
  
  return TRUE;
}

void gdi02_OnDestroy(
  HWND hwnd)
{
  // It is good practice to destroy all GDI objects when one is
  // finished with them. Usually the sooner the better. Bitmaps
  // can easilly expand to several megabytes of memory: A DDB is
  // stored in the format of the device it is compatible with:
  // On a 32 bit display a 256 color bitmap would be stored with
  // four bytes per pixel.
  DeleteObject(hbmImage);
  DeleteObject(hbmMask);
  DeleteObject(hpal);
  
  PostQuitMessage(0);
}

void gdi02_OnPaint(
  HWND hwnd)
{
  // Calling BeginPaint clears the update region that was set by calls
  // to InvalidateRect(). Once the update region is cleared no more
  // WM_PAINT messages will be sent to the window until InvalidateRect
  // is called again.
  PAINTSTRUCT ps;
  HDC hdc = BeginPaint(hwnd,&ps);
  HPALETTE hpalT = SelectPalette(hdc,hpal,FALSE);
  
  HDC hdcMem = CreateCompatibleDC(NULL);
  
  // Step one is to blit the cloud image over the background:
  BITMAP bm;
  GetObject(hbmBackground,sizeof(bm),&bm);
  
  HBITMAP hbmT = SelectBitmap(hdcMem,hbmBackground);
  
  for(int x= ps.rcPaint.left/bm.bmWidth; x < (ps.rcPaint.right + bm.bmWidth - 1)/bm.bmWidth; x++)
    for(int y= ps.rcPaint.top/bm.bmHeight; y < (ps.rcPaint.bottom + bm.bmHeight - 1)/bm.bmHeight; y++)
      BitBlt(hdc,x*bm.bmWidth,y*bm.bmHeight,bm.bmWidth,bm.bmHeight,hdcMem,0,0,SRCCOPY);
  
  // We are going to paint the two DDB's in sequence to the destination.
  // 1st the monochrome bitmap will be blitted using an AND operation to
  // cut a hole in the destination. The color image will then be ORed
  // with the destination, filling it into the hole, but leaving the
  // surrounding area untouched.
  
  SelectBitmap(hdcMem,hbmMask);
  
  GetObject(hbmMask,sizeof(bm),&bm);
  
  BitBlt(hdc,0,0,bm.bmWidth,bm.bmHeight,hdcMem,0,0,SRCAND);
  
  // Notice that we don't bother to save the return from
  // SelectBitmap this time. We know what its going to return - 
  // our mask bitmap.
  // Also note the use of SRCPAINT rather than SRCCOPY.
  SelectBitmap(hdcMem,hbmImage);
  BitBlt(hdc,0,0,bm.bmWidth,bm.bmHeight,hdcMem,0,0,SRCPAINT);
  
  // Now, clean up.
  SelectBitmap(hdcMem,hbmT);
  DeleteDC(hdcMem);
  
  // And finish off the paint function
  SelectPalette(hdc,hpalT,FALSE);
  EndPaint(hwnd,&ps);
}

BOOL gdi02_OnQueryNewPalette(
  HWND hwnd)
{
  HDC hdc = GetDC(hwnd);
  HPALETTE hpalT = SelectPalette(hdc,hpal,FALSE);
  int cClr = RealizePalette(hdc);
  SelectPalette(hdc,hpalT,FALSE);
  ReleaseDC(hwnd,hdc);
  
  if(cClr)
    InvalidateRect(hwnd,NULL,TRUE);
    
  return cClr != 0;
}

void gdi02_OnPaletteChanged(
  HWND hwnd,
  HWND hwndPaletteChange)
{
  if(hwnd == hwndPaletteChange)
    return;
    
  HDC hdc = GetDC(hwnd);
  HPALETTE hpalT = SelectPalette(hdc,hpal,FALSE);
  int cClr = RealizePalette(hdc);
  SelectPalette(hdc,hpalT,FALSE);
  ReleaseDC(hwnd,hdc);
  
  if(cClr)
    InvalidateRect(hwnd,NULL,TRUE);
}


LRESULT CALLBACK gdi02_WindowProc(
  HWND hwnd,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam)
{
  switch(uMsg)
  {
  HANDLE_MSG(hwnd,WM_CREATE,gdi02_OnCreate);
  HANDLE_MSG(hwnd,WM_DESTROY,gdi02_OnDestroy);
  HANDLE_MSG(hwnd,WM_PAINT,gdi02_OnPaint);
  HANDLE_MSG(hwnd,WM_QUERYNEWPALETTE,gdi02_OnQueryNewPalette);
  HANDLE_MSG(hwnd,WM_PALETTECHANGED,gdi02_OnPaletteChanged);
  }
  
  return DefWindowProc(hwnd,uMsg,wParam,lParam);
}  


void gdi02_RegisterClass(
  HINSTANCE hInstance)
{
  // In this example we don't register a background brush at all.
  // We want to paint our own "cloud" background.
  WNDCLASS wc = {
    0,
    gdi02_WindowProc,
    0,0,
    hInstance,
    LoadIcon(NULL,IDI_APPLICATION),
    LoadCursor(NULL,IDC_ARROW),
    (HBRUSH)(0),
    NULL,
    "GDI03"
  };
  RegisterClass(&wc);
}


int WINAPI WinMain(
  HINSTANCE hInstance,
  HINSTANCE hInstPrev,
  LPSTR lpszCmdLine,
  int nCmdShow)
{
  gdi02_RegisterClass(hInstance);
  
  HWND hwnd = CreateWindowEx(
    0L,
    "GDI03","Show Transparent Bitmap",
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
