//---------------------------------------------------------------
//  File: image.cpp
//
//  Am_WinImage manipulation functions
//---------------------------------------------------------------
#include <windows.h>
#include <windowsx.h>
#include <fstream>
#include <string.h>
#include <stdlib.h>

#include <amulet/gemW_32.h>
#include <amulet/gemW_winimage.h>

const size_t maxwrite = 30*1024; // Write 30K at a time
//---------------------------------------------------------------
//  I m a g e D a t a : : ~ I m a g e D a t a
//  Destructor for an Am_WinImage.

Am_WinImageData::~Am_WinImageData()
{
  // If a DIB exists, delete it.
  if (p_dib) GlobalFreePtr(p_dib);

  // If a palette exists, free it also.
  if (hpal) DeletePalette(hpal);

  // If a DDB exists, destroy it.
  if (hbm_ddb) DeleteBitmap(hbm_ddb);

  // If a mask exists, destroy it.
  if (hbm_mask) DeleteBitmap(hbm_mask);
}
//---------------------------------------------------------------
//  I m a g e : : I m a g e
//  Copy constructor

Am_WinImage::Am_WinImage(const Am_WinImage& img)
{
  img.imdata->count++;
  if(--imdata->count <= 0) delete imdata;
  imdata = img.imdata;
}
//---------------------------------------------------------------
//  I m a g e : : i m a g e _ d a t a
//  Copy the Am_WinImageData pointer from another image

void Am_WinImage::image_data(const Am_WinImage* img)
{
  img->imdata->count++;
  if(--imdata->count <= 0) delete imdata;
  imdata = img->imdata;
}
//---------------------------------------------------------------
//  I m a g e : : o p e r a t o r =
//  Assignment operator

Am_WinImage& Am_WinImage::operator=(const Am_WinImage& img)
{ 
  img.imdata->count++;
  if(--imdata->count <= 0) delete imdata;
  imdata = img.imdata;
  return *this;
}
//---------------------------------------------------------------
// I m a g e : : n u m c o l o r s
// Returns the number of colors used. Returns 0 if image uses
// 24-bit pixels.

unsigned int Am_WinImage::numcolors()
{
  if(imdata->p_dib == 0) return 0;
  LPBITMAPINFOHEADER p_bminfo = (LPBITMAPINFOHEADER)(imdata->p_dib);

  // If the biClrUsed field is nonzero, use that as the number of
  // colors
  if(p_bminfo->biClrUsed != 0)
    return (unsigned int)p_bminfo->biClrUsed;

  // Otherwise, the number of colors depends on the bits per pixel
  switch(p_bminfo->biBitCount) {
  case 1: return 2;
  case 4: return 16;
  case 8: return 256;
  default: return 0; // Must be 24-bit/pixel image
  }
}
//---------------------------------------------------------------
// I m a g e : : bitpixel
// Returns the number of bits per pixel.

unsigned int Am_WinImage::bitpixel()
{
  if(imdata->p_dib == 0) return 0;
  LPBITMAPINFOHEADER p_bminfo = 
    (LPBITMAPINFOHEADER)(imdata->p_dib);

  // If the biBitCount field is nonzero, use that as the number of
  // bits per pixel
  if(p_bminfo->biBitCount != 0) 
    return (unsigned int)p_bminfo->biBitCount;

  // Otherwise, the number of bits per pixel depends on the number of colors
  switch(p_bminfo->biClrUsed)
    {
    case 2: return 1;
    case 16: return 4;
    case 256: return 8;
    default: return 24; // Must be 24-bit/pixel image
    }
}
//---------------------------------------------------------------
// I m a g e : : m a k e _ p a l e t t e
// Create a color palette using information in the DIB

void Am_WinImage::make_palette()
{
  // Set up a pointer to the DIB
  LPBITMAPINFOHEADER p_bminfo = (LPBITMAPINFOHEADER)(imdata->p_dib);
  if(p_bminfo == 0) return;

  // Free any existing palette
  if(imdata->hpal != 0) DeletePalette(imdata->hpal);

  // Set up the palette, if needed
  if (numcolors() > 0) {
    LPLOGPALETTE p_pal = (LPLOGPALETTE) GlobalAllocPtr(GHND,
						       sizeof(LOGPALETTE) + numcolors() * sizeof(PALETTEENTRY));
    if(p_pal) {
      p_pal->palVersion = 0x0300;
      p_pal->palNumEntries = numcolors();

      // Set up palette entries from DIB
      LPBITMAPINFO p_bi = (LPBITMAPINFO)p_bminfo;
      for(unsigned int i = 0; i < numcolors(); i++) {
        p_pal->palPalEntry[i].peRed = p_bi->bmiColors[i].rgbRed;
	p_pal->palPalEntry[i].peGreen = p_bi->bmiColors[i].rgbGreen;
	p_pal->palPalEntry[i].peBlue = 	p_bi->bmiColors[i].rgbBlue;
	p_pal->palPalEntry[i].peFlags = 0;
      }
      imdata->hpal = CreatePalette(p_pal);
      GlobalFreePtr(p_pal);
    }
  }
}

//---------------------------------------------------------------
// I m a g e : : delete_ddb
// delete device dependent bitmap if has DIB
// returns TRUE if deleted

BOOL Am_WinImage::delete_ddb ()
{
  // If a DDB exists, destroy it & return true
  if (imdata->hbm_ddb) {
    if (!(LPBITMAPINFOHEADER)(imdata->p_dib)) DDBtoDIB();

    if (!(LPBITMAPINFOHEADER)(imdata->p_dib)) return FALSE;
	
    DeleteBitmap(imdata->hbm_ddb);
    imdata->hbm_ddb = 0;
    return TRUE;
  }
  return FALSE;
}

//---------------------------------------------------------------
// I m a g e : : image_bits
// returns pointer to image data

LPSTR Am_WinImage::image_bits ()
{
  if (!imdata -> p_dib) DDBtoDIB();
  // Set up a pointer to the DIB
  LPBITMAPINFOHEADER p_bminfo = (LPBITMAPINFOHEADER)(imdata->p_dib);
  if(p_bminfo == 0) return (0L);

  // Set up pointer to the image data (skip over BITMAPINFOHEADER
  // and palette).
  return (LPSTR)p_bminfo +  sizeof(BITMAPINFOHEADER) +
    numcolors() * sizeof(RGBQUAD);
}

//---------------------------------------------------------------
// I m a g e : : D I B t o D D B
// Create a device dependent bitmap from the DIB

void Am_WinImage::DIBtoDDB(HDC hdc)
{
  // Set up a pointer to the DIB
  LPBITMAPINFOHEADER p_bminfo = (LPBITMAPINFOHEADER)(imdata->p_dib);
  if (!p_bminfo) return;
    
  // If a DDB exists, destroy it first.
  delete_ddb();

  // Build the device-dependent bitmap.

  // Set up pointer to the image data (skip over BITMAPINFOHEADER
  // and palette).
  LPSTR p_image = (LPSTR)p_bminfo + sizeof(BITMAPINFOHEADER) +
    numcolors() * sizeof(RGBQUAD);

  // Realize palette, if there is one. Note that this does not do
  // anything on the standard 16-color VGA driver because that
  // driver does not allow changing the palette, but the new palette
  // should work on Super VGA displays.

  HPALETTE hpalold = (0L);
  if (imdata->hpal) {
    hpalold = SelectPalette(hdc, imdata->hpal, FALSE);
    RealizePalette(hdc);
  }

  // Convert the DIB into a DDB (device dependent bitmap) and 
  // block transfer (blt) it to the device context.
  if (numcolors() > 2)
    imdata->hbm_ddb = CreateDIBitmap(hdc, p_bminfo, CBM_INIT, p_image,
				     (LPBITMAPINFO)p_bminfo, DIB_RGB_COLORS);
  else {
    imdata->hbm_ddb = CreateBitmap(width(), height(), 1, 1, (0L));
    HDC memdcn = CreateCompatibleDC(hdc);
    HDC memdco = CreateCompatibleDC(hdc);
    if(memdcn != 0 && memdco != 0) {
      HBITMAP ohbm = SelectBitmap(memdco, CreateDIBitmap(hdc, p_bminfo,
							 CBM_INIT, p_image,
							 (LPBITMAPINFO)p_bminfo,
							 DIB_RGB_COLORS));
      HBITMAP nhbm = SelectBitmap(memdcn, imdata->hbm_ddb);
      BitBlt(memdcn, 0, 0, width(), height(), memdco, 0, 0, SRCCOPY/*NOTSRCCOPY*/);
      DeleteBitmap(SelectBitmap(memdco, ohbm));
      SelectBitmap(memdcn, nhbm);
      DeleteDC(memdco);
      DeleteDC(memdcn);
    }
  }
		                                 
  // Save width and height
  imdata->w = width();
  imdata->h = height();

  // Don't need the palette once the bitmap is converted to DDB
  // format.
  if (hpalold) SelectPalette(hdc, hpalold, FALSE);
}
//---------------------------------------------------------------
// I m a g e : : D D B t o D I B
// Create a device independent bitmap from the DDB 
// specified by imdata->hbm_ddb

void Am_WinImage::DDBtoDIB()
{
  // Do nothing if the DDB does not exist or if the DIB exists
  if(imdata->hbm_ddb == 0) return;
  if(imdata->p_dib != (0L)) return;

  BITMAP bm;
  GetObject(imdata->hbm_ddb, sizeof(bm), (LPSTR)&bm);

  // Set up a BITMAPINFOHEADER for the DIB
  BITMAPINFOHEADER bh;
  bh.biSize        = sizeof(BITMAPINFOHEADER);
  bh.biWidth       = bm.bmWidth;
  bh.biHeight      = bm.bmHeight;
  bh.biPlanes      = 1;
  bh.biBitCount    = (WORD)(bm.bmPlanes * bm.bmBitsPixel);
  bh.biCompression = BI_RGB;

  imdata->w = (unsigned short)bm.bmWidth;
  imdata->h = (unsigned short)bm.bmHeight;

  // Compute bytes per line, rounding up to align at a 4-byte
  // boundary
  imdata->bytes_per_line = (unsigned short)(((long)bh.biWidth * 
					     (long)bh.biBitCount + 31L) / 32 * 4);

  bh.biSizeImage   = (long)bh.biHeight * (long)imdata->bytes_per_line;

  bh.biXPelsPerMeter  = 0;
  bh.biYPelsPerMeter  = 0;

  // Determine number of colors in the palette
  short ncolors = 0;
  switch(bh.biBitCount) {
  case 1: 
    ncolors = 2;
    break;
  case 4: 
    ncolors = 16;
    break;
  case 8: 
    ncolors = 256;
    break;
  default: 
    ncolors = 0;
  }

  bh.biClrUsed        = ncolors;
  bh.biClrImportant   = 0;

  // Compute total size of DIB
  unsigned long dibsize = sizeof(BITMAPINFOHEADER) +
    ncolors * sizeof(RGBQUAD) + bh.biSizeImage;

  // Allocate memory for the DIB
  imdata->p_dib = GlobalAllocPtr(GHND, dibsize);
  if(imdata->p_dib == (0L)) return;

  // Set up palette
  HDC hdc = GetDC((0L));

  // Copy BITMAPINFO structure bh into beginning of DIB.
  _fmemcpy(imdata->p_dib, &bh, (size_t)bh.biSize);

  LPSTR p_image = (LPSTR)imdata->p_dib + 
    (WORD)bh.biSize + ncolors * sizeof(RGBQUAD);

  // Call GetDIBits to get the image and fill the palette indices
  // into a BITMAPINFO structure
  GetDIBits(hdc, imdata->hbm_ddb, 0, (WORD)bh.biHeight,
	    p_image,(LPBITMAPINFO)imdata->p_dib, 
	    DIB_RGB_COLORS);

  // All done. Clean up and return.
  ReleaseDC((0L), hdc);
}

//---------------------------------------------------------------
// I m a g e : : GetDDB
// creates DDB if needed & returns DDB

HBITMAP Am_WinImage::GetDDB (HDC hdc)
{
  if (!get_ddb()) {
    // Set up a pointer to the DIB
    LPBITMAPINFOHEADER p_bminfo = (LPBITMAPINFOHEADER)(imdata->p_dib);
    if(p_bminfo != (0L)) {
      // Set up the palette, if needed
      if (imdata->hpal == 0 && numcolors() > 0) make_palette();
	
      // Convert to DDB, if necessary
      if(imdata->hbm_ddb == 0) DIBtoDDB(hdc);
    }
  }
  return get_ddb();
}

//---------------------------------------------------------------
// I m a g e : : GetMask
// creates Mask DDB if needed & returns Mask

HBITMAP Am_WinImage::GetMask (HDC hdc)
{
  if (! imdata->hbm_mask) Make_Mask(hdc);
  return imdata->hbm_mask;
}

//---------------------------------------------------------------
// I m a g e : : GetPalette
// creates palette if needed & returns palette

HPALETTE Am_WinImage::GetPalette ()
{
  if (!palette()) {
    // Set up a pointer to the DIB
    LPBITMAPINFOHEADER p_bminfo = 
      (LPBITMAPINFOHEADER)(imdata->p_dib);
    if (p_bminfo != (0L)) {
      // Set up the palette, if needed
      if(imdata->hpal == 0 && numcolors() > 0) make_palette();
    }
  }
  return palette();
}
//---------------------------------------------------------------
// I m a g e : : s h o w
// Display a DIB on a Windows device specified by a
// device context

void Am_WinImage::show(HDC hdc, short xfrom, short yfrom,
		       short xto, short yto, short wdth, short hght,
		       BOOL masked, BOOL monochrome, BOOL invert, DWORD ropcode)
{
  // Set up a pointer to the DIB
  LPBITMAPINFOHEADER p_bminfo = (LPBITMAPINFOHEADER)(imdata->p_dib);
  if (p_bminfo != (0L)) {
    // Set up the palette, if needed
    if (imdata->hpal == 0 && numcolors() > 0)
	  make_palette();

    // Convert to DDB, if necessary
    if (imdata->hbm_ddb == 0)
	  DIBtoDDB(hdc);

    // Make mask, if necessary
    if (masked || monochrome) {
      if (imdata->hbm_mask == 0)
	    Make_Mask(hdc);
      if (imdata->hbm_mask == 0)
	    return; // error
    }
  }

  if (imdata->hbm_ddb != 0) {
    // "Blit" the DDB (and Mask) to hdc
    HDC memdc = CreateCompatibleDC(hdc);
    if (memdc != 0) {
      // If width or height is zero, use corresponding dimension
      // from the image.
      if (wdth == 0)
	    wdth = width();
      if (hght == 0)
	    hght = height();

      if (masked) {
        OSVERSIONINFO platform;
	    platform.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
        BOOL MaskBlt_defined = FALSE;

        if (GetVersionEx(& platform)) {
	      if (platform.dwPlatformId == VER_PLATFORM_WIN32_NT)
		    MaskBlt_defined = TRUE;
	    }

	    if (MaskBlt_defined) {
          // foreground and background here are reversed according to
          // the documentation of MaskBlt: 0 background and 1 foreground
          // I suspect that 0 may be black and 1 white when the mask
          // created using SetDIBits is displayed
          // NOTE: MaskBlt is not implemented under Win95
          DWORD fore = GW_DEST;
          DWORD back = ropcode;
          if (invert) { fore = ropcode; back = GW_DEST; }

          if (monochrome)
            (void) SelectBitmap (memdc, imdata->hbm_mask);
		  else
            (void) SelectBitmap (memdc, imdata->hbm_ddb);

          MaskBlt (hdc, xto, yto, wdth, hght, memdc, xfrom, yfrom,
                   imdata->hbm_mask, 0, 0, MAKEROP4(fore,back));
        }
		else { // no MaskBlt
	      HDC maskdc;

	      if (monochrome) {
	        // use the mask for both the mask and the (monochrome) image
	        (void) SelectBitmap (memdc, imdata->hbm_mask);
	        maskdc = memdc;
          }
		  else {
	        // need a separate mask and image DC
	        (void) SelectBitmap (memdc, imdata->hbm_ddb);
	        maskdc = CreateCompatibleDC (hdc);
	        if (maskdc != 0)
			  (void) SelectBitmap (maskdc, imdata->hbm_mask);
          }

          BitBlt (hdc, xto, yto, wdth, hght, memdc, xfrom, yfrom, SRCINVERT);
		  
	      if (maskdc != 0) {
	        COLORREF oldBkColor = SetBkColor (hdc, RGB (255, 255, 255));
	        COLORREF oldTextColor = SetTextColor (hdc, RGB (0, 0, 0));
	        int oldBkMode = SetBkMode (hdc, OPAQUE);

	        (void) SelectBitmap (maskdc, imdata->hbm_mask);

			BitBlt (hdc, xto, yto, wdth, hght, maskdc, xfrom, yfrom,
			        invert ? GW_GRAPHIC_NIMP : SRCAND);

	        SetBkColor (hdc, oldBkColor);
	        SetTextColor (hdc, oldTextColor);
	        SetBkMode (hdc, oldBkMode); 

	        if (!monochrome)
			  DeleteDC (maskdc);
	      }

	      BitBlt (hdc, xto, yto, wdth, hght, memdc, xfrom, yfrom, SRCINVERT); 
	    }
      }
	  else if (monochrome) {
        (void) SelectBitmap (memdc, imdata->hbm_mask);
	    BitBlt (hdc, xto, yto, wdth, hght, memdc, xfrom, yfrom, ropcode);
      }
	  else {
        (void) SelectBitmap (memdc, imdata->hbm_ddb);
	    BitBlt (hdc, xto, yto, wdth, hght, memdc, xfrom, yfrom, ropcode);
      }
      DeleteDC (memdc);
    }
  }
}

//---------------------------------------------------------------
// Am_WinImage::I m a g e
// Construct an image by copying a portion of the bitmap from 
// another image

Am_WinImage::Am_WinImage(HDC hdc, Am_WinImage *img, short x, short y, 
			 unsigned short w, unsigned short h)
{
  imdata = new Am_WinImageData;
  if(img == (0L)) return;

  unsigned short iw = img->width();
  unsigned short ih = img->height();

  if(x < 0) x = 0;
  if(y < 0) y = 0;

  // If width or height is 0, adjust them
  if(w == 0) w = iw;
  if(h == 0) h = ih;

  // Make sure width and height are not too large
  if((w+x) > iw) w = iw - x;
  if((h+y) > ih) h = ih - y;

  // Save width and height
  imdata->w = w;
  imdata->h = h;

  // Create a new bitmap for the new image
  //    imdata->hbm_ddb = CreateCompatibleBitmap(hdc, w, h);
  imdata->hbm_ddb = CreateBitmap(w, h, 1, img -> bitpixel(), (0L));
  if (imdata->hbm_ddb != 0) {
    HDC memdcn = CreateCompatibleDC(hdc);
    HDC memdco = CreateCompatibleDC(hdc);
    if (memdcn != 0 && memdco != 0) {
      HBITMAP ohbm = SelectBitmap(memdco, img->GetDDB(hdc));
      HBITMAP nhbm = SelectBitmap(memdcn, imdata->hbm_ddb);
      BitBlt(memdcn, 0, 0, w, h, memdco, x, y, SRCCOPY);
      SelectBitmap(memdco, ohbm);
      SelectBitmap(memdcn, nhbm);
      DeleteDC(memdco);
      DeleteDC(memdcn);
    }
  }
}
//---------------------------------------------------------------
// Am_WinImage::I m a g e
// Construct an image by stretching a portion of the bitmap from 
// another image
// New width and height could be negative to flip the new image.

Am_WinImage::Am_WinImage(HDC hdc, Am_WinImage *img, short x, short y, 
			 unsigned short w, unsigned short h, short neww, short newh)
{
  imdata = new Am_WinImageData;
  if(img == (0L)) return;

  unsigned short iw = img->width();
  unsigned short ih = img->height();

  if(x < 0) x = 0;
  if(y < 0) y = 0;

  // If width or height is 0, adjust them
  if(w == 0) w = iw;
  if(h == 0) h = ih;
  if(neww == 0) neww = w;
  if(newh == 0) newh = h;
    
  unsigned short nw = abs(neww);
  unsigned short nh = abs(newh);

  // Make sure width and height are not too large
  if((w+x) > iw) w = iw - x;
  if((h+y) > ih) h = ih - y;

  // Save width and height
  imdata->w = nw;
  imdata->h = nh;

  // Create a new bitmap for the new image
  //    imdata->hbm_ddb = CreateCompatibleBitmap(hdc, neww, newh);
  imdata->hbm_ddb = CreateBitmap(nw, nh, 1, img -> bitpixel(), (0L));
  if (imdata->hbm_ddb != 0) {
    HDC memdcn = CreateCompatibleDC(hdc);
    HDC memdco = CreateCompatibleDC(hdc);
    if(memdcn != 0 && memdco != 0) {
      HBITMAP ohbm = SelectBitmap(memdco, img->GetDDB(hdc));
      HBITMAP nhbm = SelectBitmap(memdcn, imdata->hbm_ddb);
      StretchBlt(memdcn, 0, 0, nw, nh,
		 memdco, (neww >= 0)? x : w - 1, (newh >= 0)? y : h - 1,
		 (neww >= 0)? w : -(int)w, (newh >= 0)? h : -(int)h, SRCCOPY);
      SelectBitmap(memdco, ohbm);
      SelectBitmap(memdcn, nhbm);
      DeleteDC(memdco);
      DeleteDC(memdcn);
    }
  }
}
//---------------------------------------------------------------
// Am_WinImage:: w r i t e _ d i b
// Write out the DIB starting at the current location in
// a stream (assumed to be opened with std::ios::out | std::ios::binary)

int Am_WinImage::write_dib(std::ofstream& ofs)
{
  // If there is no image, return without doing anything
  if(imdata->p_dib == (0L)) return 0;

// Check if file is ok
  if(!ofs) return 0;

  // Set up BMP file header
  BITMAPFILEHEADER bfhdr;

  bfhdr.bfType = ('M' << 8) | 'B';
  bfhdr.bfReserved1 = 0;
  bfhdr.bfReserved2 = 0;
  bfhdr.bfOffBits = sizeof(BITMAPFILEHEADER) + 
    sizeof(BITMAPINFOHEADER) + numcolors() * sizeof(RGBQUAD);
  bfhdr.bfSize = (long) height() * 
    (long) imdata->bytes_per_line + bfhdr.bfOffBits;

  // Write the file header to the file
  ofs.write((const char*)&bfhdr, sizeof(BITMAPFILEHEADER));

  // Save the file in big chunks. 

  // Allocate a large buffer to be used when transferring
  // data to the file

  unsigned char *wbuf = new unsigned char[maxwrite];
  if(wbuf == (0L)) return 0;

  unsigned char __huge *data = (unsigned char __huge*)imdata->p_dib;
  unsigned int chunksize;
  long bmpsize = bfhdr.bfSize - sizeof(BITMAPFILEHEADER);

  unsigned int i;
  while(bmpsize > 0) {
    if(bmpsize > maxwrite) chunksize = maxwrite;
    else chunksize = (unsigned int)bmpsize;
    // Copy image from DIB to buffer
    for(i = 0; i < chunksize; i++) wbuf[i] = data[i];
    ofs.write((const char*)wbuf, chunksize);
    bmpsize -= chunksize;
    data += chunksize;
  }
  delete wbuf;
  return 1;
}
//---------------------------------------------------------------
// I m a g e : : Make_Mask
// Create a device dependent mask from the DIB
//
// Interface: for opaque bitmaps, it is the responsibility of the
// caller to set the hdc background color to the desired transparent
// color
// 
// Implementation: converts from color to monochrome using BitBlt
// for transparent images, uses color[0] as the transparent color
// for opaque images, uses the current background color

void Am_WinImage::Make_Mask(HDC hdc)
{
  // Set up a pointer to the DIB
  LPBITMAPINFOHEADER p_bminfo = (LPBITMAPINFOHEADER)(imdata->p_dib);
  if (p_bminfo != (0L)) {
    // Set up the palette, if needed
    if (imdata->hpal == 0 && numcolors () > 0) make_palette ();

    // Convert to DDB, if necessary
    if (imdata->hbm_ddb == 0) DIBtoDDB (hdc);
  }

  // If a mask exists, destroy it first.
  if (imdata->hbm_mask) {
    DeleteBitmap (imdata->hbm_mask);
    imdata->hbm_mask = 0;
  }

  if (imdata->hbm_ddb != 0) {

    HDC srcdc = CreateCompatibleDC (hdc);
    HDC maskdc = CreateCompatibleDC (hdc);
	
    if (srcdc != 0 && maskdc != 0) {

      imdata->hbm_mask = CreateBitmap (width (), height (), 1, 1, (0L));
      (void)SelectBitmap (maskdc, imdata->hbm_mask);

      (void)SelectBitmap (srcdc, imdata->hbm_ddb);
      (void)SetBkMode (srcdc, OPAQUE);	// just in case
	  
      if (is_transparent () && (numcolors () > 0)) {
        // Set up pointer to the image data (skip over BITMAPINFOHEADER
	// and palette).
	LPSTR p_image = (LPSTR)p_bminfo + sizeof (BITMAPINFOHEADER) +
	  numcolors () * sizeof (RGBQUAD);

	// The color in bmiColors[transparent_index ()] is the transparent color
	LPBITMAPINFO p_bi = (LPBITMAPINFO)p_bminfo;
	RGBQUAD color0 = p_bi->bmiColors[transparent_index ()];

	COLORREF oldBkColor = SetBkColor (srcdc,
					  RGB (color0.rgbRed, color0.rgbGreen, color0.rgbBlue));
      }
      // If the bitmap isn't transparent the background color
      // will be the transparent color 
      BitBlt (maskdc, 0, 0, width (), height (), srcdc, 0, 0, SRCCOPY);

      DeleteDC (maskdc);
      DeleteDC (srcdc);
    }
  }
}
