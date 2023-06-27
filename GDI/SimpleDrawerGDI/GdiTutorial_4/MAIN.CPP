#include <windows.h>
#include <windowsx.h>
#include "resource.h"
#include "bmpapi.h"
#include "dib.h"

CDIB* dibMain;
CDIB* dibBkg;

HPALETTE hpal;

BOOL demo_OnCreate(
   HWND hwnd,
   LPCREATESTRUCT lpcs)
{
  HINSTANCE hInstance = GetWindowInstance(hwnd);
  
  dibMain = CDIB::CreateFromResource(hInstance,MAKEINTRESOURCE(IDB_BITMAP1));
  dibBkg = CDIB::CreateFromResource(hInstance,MAKEINTRESOURCE(IDB_BITMAP3));
  
  hpal = dibMain->CreatePalette();
  
  return TRUE;
}

void demo_OnDestroy(
  HWND hwnd)
{
  delete dibMain;
  delete dibBkg;
  DeleteObject(hpal);
  
  PostQuitMessage(0);
}

void demo_OnPaint(
  HWND hwnd)
{
  PAINTSTRUCT ps;
  HDC hdc = BeginPaint(hwnd,&ps);
  HPALETTE hpalT = SelectPalette(hdc,hpal,FALSE);
  
  // Start by filling the update rect with the background tile.
  LPBITMAPINFO lpbmi = dibBkg->GetBitmapInfo();
  int nWidth = (int)lpbmi->bmiHeader.biWidth;
  int nHeight = (int)lpbmi->bmiHeader.biHeight;
  
  for(int x= ps.rcPaint.left/nWidth; x < (ps.rcPaint.right + nWidth - 1)/nWidth; x++)
    for(int y= ps.rcPaint.top/nHeight; y < (ps.rcPaint.bottom + nHeight - 1)/nHeight; y++)
      StretchDIBits(hdc,x*nWidth,y*nHeight,nWidth,nHeight,0,0,nWidth,nHeight,dibBkg->GetBits(),lpbmi,DIB_RGB_COLORS,SRCCOPY);
  
  // Because were not sure if our palette contains black and white we deselect if while blitting the
  // mono mask.
  SelectPalette(hdc,hpalT,FALSE);      
  
  // Now we have to blit the two transparent images.
  // Start with the mask.
  lpbmi = dibMain->CreateBitmapinfo();
  nWidth = (int)lpbmi->bmiHeader.biWidth;
  nHeight = (int)lpbmi->bmiHeader.biHeight;

  // Get the index of the transparent color. Thats the top left corner!
  // (transparency like this is supported only by 256 color DIB's).
  int idxTrans = ((LPBYTE)dibMain->GetBits())[0];
  RGBQUAD rgb_black = {0,0,0,0};
  RGBQUAD rgb_white = {255,255,255,0};
  
  // Fill the color table with black except for the transparent item. that we fill with white.
  int cColors = dibMain->GetColorTableSize();
  for(int i=0; i<cColors; i++){
    lpbmi->bmiColors[i] = rgb_black;
  }
  lpbmi->bmiColors[idxTrans] = rgb_white;
  
  nWidth = (int)lpbmi->bmiHeader.biWidth;
  nHeight = (int)lpbmi->bmiHeader.biHeight;
  
  StretchDIBits(hdc,0,0,nWidth,nHeight,0,0,nWidth,nHeight,dibMain->GetBits(),lpbmi,DIB_RGB_COLORS,SRCAND);

  // For the color bitmap we perform the reverse. All the colors are left alone save the transparent
  // index that must be zero.
  CDIB::DeleteBitmapinfo(lpbmi);
  lpbmi = dibMain->CreateBitmapinfo();
  lpbmi->bmiColors[idxTrans] = rgb_black;
  
  SelectPalette(hdc,hpal,FALSE);
  

  StretchDIBits(hdc,0,0,nWidth,nHeight,0,0,nWidth,nHeight,dibMain->GetBits(),lpbmi,DIB_RGB_COLORS,SRCPAINT);
  
  CDIB::DeleteBitmapinfo(lpbmi);
  
  // And finish off the paint function
  SelectPalette(hdc,hpalT,FALSE);
  EndPaint(hwnd,&ps);
}


BOOL demo_OnQueryNewPalette(
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

void demo_OnPaletteChanged(
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


LRESULT CALLBACK demo_WindowProc(
  HWND hwnd,
  UINT uMsg,
  WPARAM wParam,
  LPARAM lParam)
{
  switch(uMsg)
  {
  HANDLE_MSG(hwnd,WM_CREATE,demo_OnCreate);
  HANDLE_MSG(hwnd,WM_DESTROY,demo_OnDestroy);
  HANDLE_MSG(hwnd,WM_PAINT,demo_OnPaint);
  HANDLE_MSG(hwnd,WM_QUERYNEWPALETTE,demo_OnQueryNewPalette);
  HANDLE_MSG(hwnd,WM_PALETTECHANGED,demo_OnPaletteChanged);
  }
  
  return DefWindowProc(hwnd,uMsg,wParam,lParam);
}  


void demo_RegisterClass(
  HINSTANCE hInstance)
{
  // In this example we don't register a background brush at all.
  // We want to paint our own "cloud" background.
  WNDCLASS wc = {
    0,
    demo_WindowProc,
    0,0,
    hInstance,
    LoadIcon(NULL,IDI_APPLICATION),
    LoadCursor(NULL,IDC_ARROW),
    (HBRUSH)(0),
    NULL,
    "DEMO"
  };
  RegisterClass(&wc);
}


int WINAPI WinMain(
  HINSTANCE hInstance,
  HINSTANCE hInstPrev,
  LPSTR lpszCmdLine,
  int nCmdShow)
{
  demo_RegisterClass(hInstance);
  
  HWND hwnd = CreateWindowEx(
    0L,
    "DEMO","Show Transparent Device Independent Bitmap",
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
