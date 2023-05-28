#include <windows.h>
#include <windowsx.h>
#include "resource.h"


// Our DDB handles are global variables.
// hbmImage holds the handle of the color image
// hbmMask holds teh handle of the mono mask.
HBITMAP hbmImage;
HBITMAP hbmMask;

BOOL gdi02_OnCreate(
   HWND hwnd,
   LPCREATESTRUCT lpcs)
{
  // The bitmap should be stored as a resource in the exe file.
  // We pass the hInstance of the application, and the ID of the
  // bitmap to the LoadBitmap API function and it returns us an
  // HBITMAP to a DDB created from the resource data.
  HINSTANCE hInstance = GetWindowInstance(hwnd);
  hbmImage = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP1));

  BITMAP bm;
  GetObject(hbmImage,sizeof(bm),&bm);  
  
  // Now, we need to "correct" the image bitmap, and create a
  // monochrome bitmap. Step 1 - we create a monochrome bitmap
  // the same size as the original. then we trick GDI into
  // blitting the Image onto the mask in such a way as the
  // mask is created correctly.
  hbmMask = CreateBitmap(bm.bmWidth,bm.bmHeight,1,1,NULL);
  
  // We will need two DCs to work with. One to hold the Image
  // (the source), and one to hold the mask (destination).
  // When blitting onto a monochrome bitmap from a color, pixels
  // in the source color bitmap that are equal to the background
  // color are blitted as white. All the remaining pixels are
  // blitted as black.
  
  HDC hdcSrc = CreateCompatibleDC(NULL);
  HDC hdcDst = CreateCompatibleDC(NULL);
  
  HBITMAP hbmSrcT = SelectBitmap(hdcSrc,hbmImage);
  HBITMAP hbmDstT = SelectBitmap(hdcDst,hbmMask);
  
  COLORREF clrTopLeft = GetPixel(hdcSrc,0,0);
  COLORREF clrSaveBk = SetBkColor(hdcSrc,clrTopLeft);
  
  // This call sets up the mask bitmap.
  BitBlt(hdcDst,0,0,bm.bmWidth,bm.bmHeight,hdcSrc,0,0,SRCCOPY);
  
  // Now, we need to paint onto the original image, making
  // sure that the "transparent" area is set to black. What
  // we do is AND the monochrome image onto the color Image
  // first. When blitting from mono to color, the monochrome
  // pixel is first transformed as follows:
  // if it 1 (black) it is mapped to the color set by SetTextColor().
  // if is 0 (white) is is mapped to the color set by SetBkColor().
  // Only then is the raster operation performed.
  
  COLORREF clrSaveDstText = SetTextColor(hdcSrc,RGB(255,255,255));
  SetBkColor(hdcSrc,RGB(0,0,0));
  
  BitBlt(hdcSrc,0,0,bm.bmWidth,bm.bmHeight,hdcDst,0,0,SRCAND);
  
  // Clean up by deselecting any objects, and delete the 
  // DC's.
  SetTextColor(hdcDst,clrSaveDstText);
  
  SetBkColor(hdcSrc,clrSaveBk);
  SelectBitmap(hdcSrc,hbmSrcT);
  SelectBitmap(hdcDst,hbmDstT);
  
  DeleteDC(hdcSrc);
  DeleteDC(hdcDst);
  
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
  
  // We are going to paint the two DDB's in sequence to the destination.
  // 1st the monochrome bitmap will be blitted using an AND operation to
  // cut a hole in the destination. The color image will then be ORed
  // with the destination, filling it into the hole, but leaving the
  // surrounding area untouched.
  
  HDC hdcMem = CreateCompatibleDC(NULL);
  
  HBITMAP hbmT = SelectBitmap(hdcMem,hbmMask);
  
  BITMAP bm;
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
  EndPaint(hwnd,&ps);
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
  }
  
  return DefWindowProc(hwnd,uMsg,wParam,lParam);
}  


void gdi02_RegisterClass(
  HINSTANCE hInstance)
{
  // Loads  the checkerboard pattern. This *must* be an 8x8 bitmap, as a
  // brush can only use the top 8x8 pixels of a bitmap.
  HBITMAP hbm = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP2));
  
  WNDCLASS wc = {
    0,
    gdi02_WindowProc,
    0,0,
    hInstance,
    LoadIcon(NULL,IDI_APPLICATION),
    LoadCursor(NULL,IDC_ARROW),
    CreatePatternBrush(hbm),
    NULL,
    "GDI02"
  };
  // A pattern brush, once created no longer relies on the bitmap that
  // was used to create it. We should delete the bitmap as we no
  // longer need it.
  DeleteObject(hbm);
  
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
    "GDI02","Show Transparent Bitmap",
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
