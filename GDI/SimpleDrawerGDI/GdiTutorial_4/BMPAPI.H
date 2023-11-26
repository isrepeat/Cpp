#ifndef _BMPAPI_HEADER
#define _BMPAPI_HEADER

#ifdef _cplusplus
extern "C" {
#endif //_cplusplus

#define SRCNOTAND  0x00220326

HPALETTE CreatePaletteFromDIBitmap(LPBITMAPINFO pbmi);

HPALETTE CreatePaletteFromBitmapResource(HINSTANCE hInstance,LPCSTR pszResourceName);

HBITMAP CreateDDBFromPackedDIBitmap(LPBITMAPINFO pbmi,HPALETTE hPalette);

HBITMAP CreateDDBFromBitmapResource(HINSTANCE hInstance,LPCSTR pszResourceName,HPALETTE hPalette);

HBITMAP CreateMonoMask(HBITMAP hbmSrc,HPALETTE hPalette,COLORREF clr);

#ifdef _cplusplus
} //extern "C" {
#endif //_cplusplus

#endif // _BMPAPI_HEADER
