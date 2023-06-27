#ifndef CDIB_HEADER
#define CDIB_HEADER

class CDIB
{
public:
static CDIB* Create(int cx, int cy, int nColorDepth);
static CDIB* CreateFromFile(LPCSTR);
static CDIB* CreateFromResource(HINSTANCE hInstance,LPCSTR lpszResourceName);
  
  ~CDIB();
  
  HPALETTE CreatePalette(); // creates a palette from the DIB's colortable.
  HBITMAP CreateBitmap(HPALETTE); // creates a DDB from the DIB, mapping to the hPalette.
  
  LPBITMAPINFO GetBitmapInfo(){return mlpBmi;}
  LPVOID GetBits(){return mlpBits;}
  
  LPBITMAPINFO CreateBitmapinfo();
  void DeleteBitmapinfo(LPBITMAPINFO lpbmi);
  int  GetColorTableSize();
  
protected:
  CDIB(HGLOBAL hglob);
  
private:  
  LPBITMAPINFO mlpBmi;
  LPVOID mlpBits;
  HGLOBAL mhRes;
};

#endif
