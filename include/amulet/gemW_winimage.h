//---------------------------------------------------------------
//  File: image.h
//
//  Defines the Am_WinImage class.
//
//---------------------------------------------------------------
#if !defined(__IMAGE_H)
#define __IMAGE_H

#include <iostream>

// Win32 Ternary ROP for dest
#define GW_DEST 0x00AA0029
// Win32 Ternary ROP for dest and not source
#define GW_GRAPHIC_NIMP 0x00220326

// This class represents the data for an image
class Am_WinImageData
{
  friend class Am_WinImage;
  friend class Am_WinBMPImage;
  friend class Am_WinGIFImage;

protected:
  Am_WinImageData()
      : p_dib(0), count(1), hpal(0), hbm_ddb(0), hbm_mask(0), bytes_per_line(0),
        w(0), h(0), transparent(0), transparent_index(0)
  {
  }

  ~Am_WinImageData();

protected:
  // This points to a BITMAPINFOHEADER followed by the
  // image data.
  LPVOID p_dib;        // Device independent bitmap
  HPALETTE hpal;       // Color palette
  HBITMAP hbm_ddb;     // Device dependent bitmap
  HBITMAP hbm_mask;    // Device dependent bitmap
  unsigned short w, h; // Width and height
  unsigned short bytes_per_line;
  unsigned short count;
  unsigned short transparent;
  unsigned short transparent_index;
};

// Abstract base class for all images
class Am_WinImage
{
public:
  // Constructors
  Am_WinImage() { imdata = new Am_WinImageData; }
  Am_WinImage(HBITMAP hbm, unsigned short w, unsigned short h)
  {
    imdata = new Am_WinImageData;
    imdata->hbm_ddb = hbm;
    imdata->w = w;
    imdata->h = h;
  }

  Am_WinImage(HDC hdc, Am_WinImage *img, short x, short y, unsigned short w,
              unsigned short h);
  Am_WinImage(HDC hdc, Am_WinImage *img, short x, short y, unsigned short w,
              unsigned short h, short neww, short newh);

  // Copy Constructor
  Am_WinImage(const Am_WinImage &img);

  virtual ~Am_WinImage()
  {
    if (--imdata->count <= 0)
      delete imdata;
  }

  // Operators
  Am_WinImage &operator=(const Am_WinImage &img);

  // Copy the imdata pointer from another image
  void image_data(const Am_WinImage *img);

  // Functions to load and save images
  virtual int read(const char *filename) { return 0; };
  virtual int write(const char *filename) { return 0; };
  virtual int read(std::ifstream &ifs) { return 0; }
  int write_dib(std::ofstream &ofs);

  // Returns pointer to the Windows Device Independent
  // Bitmap (DIB).
  LPVOID get_dib() const { return imdata->p_dib; }

  // Function to return the handle to the device dependent
  // bitmap
  HBITMAP get_ddb() const { return imdata->hbm_ddb; }
  HBITMAP get_mask() const { return imdata->hbm_mask; }
  BOOL delete_ddb();
  HBITMAP GetDDB(HDC hdc);
  HBITMAP GetMask(HDC hdc);

  unsigned short width() const
  {
    if (imdata->p_dib != 0)
      return (unsigned short)((LPBITMAPINFOHEADER)imdata->p_dib)->biWidth;
    return imdata->w;
  }
  unsigned short height() const
  {
    if (imdata->p_dib != 0)
      return (unsigned short)((LPBITMAPINFOHEADER)imdata->p_dib)->biHeight;
    return imdata->h;
  }

  int image_loaded() const
  {
    if (imdata->p_dib == 0)
      return 0;
    else
      return 1;
  }

  void detach()
  {
    if (--imdata->count == 0)
      delete imdata;
    imdata = new Am_WinImageData;
  }

  // Functions to make palette and convert to DDB
  void make_palette();
  void DIBtoDDB(HDC hdc);
  void DDBtoDIB();
  void Make_Mask(HDC hdc);

  // Function that displays the DIB on a Windows device
  // specified by a device context
  void show(HDC hdc, short xfrom = 0, short yfrom = 0, short xto = 0,
            short yto = 0, short width = 0, short height = 0,
            BOOL masked = FALSE, BOOL monochrome = FALSE, BOOL invert = FALSE,
            DWORD ropcode = SRCCOPY);

  LPSTR image_bits();
  unsigned int numcolors();
  unsigned int bitpixel();
  HPALETTE palette() const { return imdata->hpal; }
  HPALETTE GetPalette();

  unsigned short is_transparent() const { return imdata->transparent; }
  unsigned short transparent_index() const { return imdata->transparent_index; }

protected:
  Am_WinImageData *imdata;
};

#endif
