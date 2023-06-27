#include <windows.h>
#include <windowsx.h>
#include "dib.h"
#include <memory.h>

HPALETTE CDIB::CreatePalette()
{
  int cColors = GetColorTableSize();
  if(!cColors)
    return NULL; // There is no hapalette.
    
  LOGPALETTE* ppal = (LOGPALETTE*)new char[sizeof(LOGPALETTE) + sizeof(PALETTEENTRY)*(cColors-1)];
  
  for(int i=0; i<cColors; i++){
    ppal->palPalEntry[i].peRed = mlpBmi->bmiColors[i].rgbRed;
    ppal->palPalEntry[i].peGreen = mlpBmi->bmiColors[i].rgbGreen;
    ppal->palPalEntry[i].peBlue = mlpBmi->bmiColors[i].rgbBlue;
    ppal->palPalEntry[i].peFlags = NULL;
  }
  
  ppal->palVersion = 0x0300;
  ppal->palNumEntries = cColors;
  
  HPALETTE hpal = ::CreatePalette(ppal);
  
  delete ppal;
  
  return hpal;
}

// returns the number of entries in the DIBs color table.
int CDIB::GetColorTableSize()
{
  // hopefull: The header tells us exactly how many colors.
  if(mlpBmi->bmiHeader.biClrUsed)
    return (int)mlpBmi->bmiHeader.biClrUsed;
    
  if(mlpBmi->bmiHeader.biBitCount > 12)
    return 0; // above 12bits are RGB modes. Below 12 are palette. (up to 4096 palette entries).
  
  return 1 << mlpBmi->bmiHeader.biBitCount;
}

CDIB* CDIB::CreateFromResource(
  HINSTANCE hInstance,
  LPCSTR lpszResName)
{
  HRSRC hrsrc = FindResource(hInstance,lpszResName,RT_BITMAP);
  if(!hrsrc)
    return NULL;
  // Load the resource.
  return new CDIB(LoadResource(hInstance,hrsrc));
}

CDIB::CDIB(HGLOBAL hGlob)
{
  mhRes = hGlob;
  
  mlpBmi = (LPBITMAPINFO)LockResource(mhRes);
  // calculate the size of the color table, and get the offse to the bits.
  int cClrTblSize = GetColorTableSize();
  mlpBits = &mlpBmi->bmiColors[cClrTblSize];
}

CDIB::~CDIB()
{
  if(mhRes){
    UnlockResource(mhRes);
    FreeResource(mhRes);
  }
}

LPBITMAPINFO CDIB::CreateBitmapinfo()
{
  // we must clone the current bitmap info struct to give the user a writeable copy.
  // calculate the size. The BITMAPINFO struct contains one RGBQUAD, so we work out
  // less one.
  int cbColorTableSize = sizeof(RGBQUAD)*(GetColorTableSize()-1);
  int cbSize = sizeof(BITMAPINFO) + cbColorTableSize;
  LPBITMAPINFO lpBmi = (LPBITMAPINFO)new far char[cbSize];
  
  // Clone the entries.
  _fmemcpy(lpBmi,mlpBmi,cbSize);
  
  return lpBmi;
}

void CDIB::DeleteBitmapinfo(
  LPBITMAPINFO lpbmi)
{
  delete lpbmi;
}
