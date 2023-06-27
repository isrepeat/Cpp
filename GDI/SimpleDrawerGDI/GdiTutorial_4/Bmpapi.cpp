#include <windows.h>
#include <windowsx.h>

#include "bmpapi.h"

///////////////////////////////////////////////////////////////////////////////
// DiB/DdB helper functions...

HPALETTE CreatePaletteFromDIBitmap(
  LPBITMAPINFO pbmi)
{
  NPLOGPALETTE pLogPalette; // a logpalette to hold the palette entries.
  int nNumColors;       // number of colors in the color table
  int i;                // loop counter
  HPALETTE hPalette;    // handle to palette to return.
  
  nNumColors = (int)pbmi->bmiHeader.biClrUsed;
  if(!nNumColors && pbmi->bmiHeader.biBitCount <= 8)
    nNumColors = 1<<pbmi->bmiHeader.biBitCount;
    
  if(nNumColors)
  {
    pLogPalette = (NPLOGPALETTE)LocalAlloc(LPTR,sizeof(LOGPALETTE)+(nNumColors-1)*sizeof(PALETTEENTRY));
    
    pLogPalette->palNumEntries = nNumColors;
    pLogPalette->palVersion = 0x300;
    
    for(i=0;i<nNumColors;i++)
    {
      pLogPalette->palPalEntry[i].peBlue = pbmi->bmiColors[i].rgbBlue;
      pLogPalette->palPalEntry[i].peRed = pbmi->bmiColors[i].rgbRed;
      pLogPalette->palPalEntry[i].peGreen = pbmi->bmiColors[i].rgbGreen;
      pLogPalette->palPalEntry[i].peFlags = 0;
    }
    
    hPalette = CreatePalette(pLogPalette);
    
    LocalFree((HLOCAL)pLogPalette);
    
    return hPalette;
  }
  
  return NULL;
}  


HPALETTE CreatePaletteFromBitmapResource(
  HINSTANCE hInstance,
  LPCSTR pszResourceName)
{
  HRSRC hrsrc;          // bitmap resource handle
  HGLOBAL hglob;        // hGlobal to bitmap bits.
  LPVOID pbm;            // actual pointer to bits.
  HPALETTE hPalette;    // handle to palette to return.
               
  // Find load and lock the bitmap resource.
  hrsrc = FindResource(hInstance,pszResourceName,RT_BITMAP);
  hglob = LoadResource(hInstance,hrsrc);
  pbm = LockResource(hglob);
  
  hPalette = CreatePaletteFromDIBitmap((LPBITMAPINFO)pbm);
    
  // Unlock and unload the resource...
  UnlockResource(hglob);
  FreeResource(hglob);
  
  return hPalette;
}


HBITMAP CreateDDBFromPackedDIBitmap(
  LPBITMAPINFO pbmi,
  HPALETTE hPalette)
{
  int nNumColors;       // number of colors in the color table
  int nWidth;
  int nHeight;
  
  LPVOID lpBits;        // DIB bitmap data
  
  HDC hDC;              // temp DC
  HPALETTE hpalT;       // temp palette
  
  HBITMAP hBitmap;      // handle to bitmap to return.
  
  nWidth = (int)pbmi->bmiHeader.biWidth;
  nHeight = (int)pbmi->bmiHeader.biHeight;
  
  // Find out how big the bmiColors is
  nNumColors = (int)pbmi->bmiHeader.biClrUsed;
  if(!nNumColors && pbmi->bmiHeader.biBitCount <= 8)
    nNumColors = 1<<pbmi->bmiHeader.biBitCount;
    
  lpBits = &pbmi->bmiColors[nNumColors];

  // This code worries me.
  // Will the bitmap work if this is the 1st time the palette is realized?                    
  hDC = GetDC(NULL);
  hpalT = SelectPalette(hDC,hPalette,TRUE);
  hBitmap = CreateDIBitmap(hDC,&pbmi->bmiHeader,CBM_INIT,lpBits,pbmi,DIB_RGB_COLORS);
  ReleaseDC(NULL,hDC);
  
  return hBitmap;
}


HBITMAP CreateDDBFromBitmapResource(
  HINSTANCE hInstance,
  LPCSTR pszResourceName,
  HPALETTE hPalette)
{
  HRSRC hrsrc;          // bitmap resource handle
  HGLOBAL hglob;        // hGlobal to bitmap bits.
  LPVOID lpbm;
  HBITMAP hBitmap;      // handle to bitmap to return.

  // Find load and lock the bitmap resource.
  hrsrc = FindResource(hInstance,pszResourceName,RT_BITMAP);
  hglob = LoadResource(hInstance,hrsrc);
  lpbm = LockResource(hglob);
  
  hBitmap = CreateDDBFromPackedDIBitmap((LPBITMAPINFO)lpbm,hPalette);
  
  // Unlock and unload the resource...
  UnlockResource(hglob);
  FreeResource(hglob);
  
  return hBitmap;
}


HBITMAP CreateMonoMask(
  HBITMAP hbmSrc,
  HPALETTE hPalette,
  COLORREF clr)
{
  BITMAP bitmap;
  HBITMAP hbmMono;
  
  HBITMAP hbmDstOld;
  HBITMAP hbmSrcOld;
  
  HDC hdcSrc;
  HDC hdcDst;
  
  HPALETTE hpalSrcOld;
  
  COLORREF clrBkSrcOld;
  
  // Get the info about the bitmap we are manipulating...
  GetObject(hbmSrc,sizeof(bitmap),&bitmap);

  // Make a mono bitmap of the same dimensions... 
  hbmMono = CreateBitmap(bitmap.bmWidth,bitmap.bmHeight,1,1,NULL);
  
  // Make some DC's for us to handle everything with...
  hdcSrc = CreateCompatibleDC(NULL);
  hpalSrcOld = SelectPalette(hdcSrc,hPalette,FALSE);
  hbmSrcOld = SelectBitmap(hdcSrc,hbmSrc);
  
  hdcDst = CreateCompatibleDC(NULL);
  hbmDstOld = SelectBitmap(hdcDst,hbmMono);
  
  // Blit the bitmap into the mono DC.
  
  clrBkSrcOld = SetBkColor(hdcSrc,clr);
  
  BitBlt(hdcDst,0,0,bitmap.bmWidth,bitmap.bmHeight,hdcSrc,0,0,SRCCOPY);
  
  // Now, blit the other way to clear the mask area in the main bitmap.
  BitBlt(hdcSrc,0,0,bitmap.bmWidth,bitmap.bmHeight,hdcDst,0,0,SRCNOTAND);
  
  // Free and release the DC's
  SelectBitmap(hdcDst,hbmDstOld);
  DeleteDC(hdcDst);
  
  SelectPalette(hdcSrc,hpalSrcOld,TRUE);
  SelectBitmap(hdcSrc,hbmSrcOld);
  DeleteDC(hdcSrc);
                 
  // return the mono bmp
  // (the color bmp has also been altered - the mask areas have ben zero'd).
  return hbmMono;
}
