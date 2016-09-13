//---------------------------------------------------------------
//  File: gemW_bmpimage.h
//
//  Defines the Am_WinBMPImage class representing a Windows BMP image.
//
//---------------------------------------------------------------
#if !defined(__BMPIMAGE_H)
#define __BMPIMAGE_H

#include "gemW_winimage.h"

#pragma pack(push, 1)

class Am_WinBMPImage : public Am_WinImage
{
public:
  Am_WinBMPImage() {}
  Am_WinBMPImage(HBITMAP hbm, unsigned short w, unsigned short h)
      : Am_WinImage(hbm, w, h){};
  Am_WinBMPImage(HDC hdc, Am_WinImage *img, short x, short y, unsigned short w,
                 unsigned short h)
      : Am_WinImage(hdc, img, x, y, w, h){};
  Am_WinBMPImage(HDC hdc, Am_WinImage *img, short x, short y, unsigned short w,
                 unsigned short h, short neww, short newh)
      : Am_WinImage(hdc, img, x, y, w, h, neww, newh){};
  // Copy Constructor
  Am_WinBMPImage(const Am_WinImage &img) : Am_WinImage(img){};

  ~Am_WinBMPImage() {}

  Am_WinBMPImage &operator=(const Am_WinImage &img)
  {
    return (Am_WinBMPImage &)Am_WinImage::operator=(img);
  };

  int read(const char *filename);
  int write(const char *filename);
  int read(std::istream &is);

private:
  BITMAPFILEHEADER bmphdr;
};

#pragma pack(pop)

#endif
