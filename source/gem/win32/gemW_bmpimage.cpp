//---------------------------------------------------------------
//  File: gemW_bmpimage.cpp
//
//  Am_WinImage manipulation functions for Windows BMP images.
//---------------------------------------------------------------
#include <windows.h>
#include <windowsx.h>
#include <fstream>
#include <string.h>
#include <limits.h>

#include <amulet/gemW_32.h>
#include <amulet/gemW_bmpimage.h>

const size_t maxread = 30 * 1024;  // Read 30K at a time
const size_t maxwrite = 30 * 1024; // Write 30K at a time
//---------------------------------------------------------------
//  B M P I m a g e : : r e a d
//  Read and interpret a Windows .BMP image (Device Independent
//  Bitmap).

int
Am_WinBMPImage::read(const char *filename)
{
  // If there is an existing image, detach the image data
  // before reading a new image
  if (imdata->p_dib != 0)
    detach();

  // Open file for reading
  std::ifstream ifs(filename, std::ios::in | std::ios::binary);

  // Call Am_WinBMPImage::read(ifstream& ifs) to read in image
  return read(ifs);
}
//---------------------------------------------------------------
// B M P I m a g e : : r e a d ( i f s t r e a m & )
// Read image information from an open file

int
Am_WinBMPImage::read(std::istream &is)
{
  if (!is.ipfx(0)) {
    // Error reading file. Return 0.
    return 0;
  }

  // Read the file header
  is.read((char *)&bmphdr, sizeof(BITMAPFILEHEADER));

  // Check if image file format is acceptable (the type
  // must be 'BM'
  if (bmphdr.bfType != (('M' << 8) | 'B'))
    return 0;

  // Determine size of DIB to read -- that's the file size (as
  // specified by the bfSize field of the BITMAPFILEHEADER
  // structure) minus the size of the BITMAPFILEHEADER
  long bmpsize = bmphdr.bfSize - sizeof(BITMAPFILEHEADER);

  // Allocate space for the bitmap
  imdata->p_dib = GlobalAllocPtr(GHND, bmpsize);

  // If memory allocation fails, return 0
  if (imdata->p_dib == 0)
    return 0;

  // Load the file in big chunks. We don't have to interpret
  // because our internal format is also BMP.

  // Allocate a large buffer to read from file
  char *rbuf = new char[maxread];
  if (rbuf == (0L)) {
    detach();
    return 0;
  }

  unsigned char __huge *data = (unsigned char __huge *)imdata->p_dib;
  unsigned int chunksize;
  unsigned int i;

  while (bmpsize > 0) {
    if (bmpsize > maxread)
      chunksize = maxread;
    else
      chunksize = (unsigned int)bmpsize;
    is.read(static_cast<char *>(rbuf), chunksize);

    // Copy into DIB
    for (i = 0; i < chunksize; i++)
      data[i] = rbuf[i];
    bmpsize -= chunksize;
    data += chunksize;
  }
  delete rbuf;

  // Compute bytes per line, rounding up to align at a 4-byte
  // boundary
  LPBITMAPINFOHEADER p_bi = (LPBITMAPINFOHEADER)imdata->p_dib;
  imdata->bytes_per_line =
      (unsigned short)(((long)p_bi->biWidth * (long)p_bi->biBitCount + 31L) /
                       32 * 4);

  return 1;
}
//---------------------------------------------------------------
//  B M P I m a g e : : w r i t e
//  Write a Windows .BMP image to a file (in Device Independent
//  Bitmap format)

int
Am_WinBMPImage::write(const char *filename)
{
  // If there is no image, return without doing anything
  if (imdata->p_dib == 0)
    return 0;

  // Open file for binary write operations.
  std::ofstream ofs(filename, std::ios::out | std::ios::binary);
  if (!ofs)
    return 0;

  // Set up BMP file header
  bmphdr.bfType = ('M' << 8) | 'B';
  bmphdr.bfReserved1 = 0;
  bmphdr.bfReserved2 = 0;
  bmphdr.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
                     numcolors() * sizeof(RGBQUAD);
  bmphdr.bfSize =
      (long)height() * (long)imdata->bytes_per_line + bmphdr.bfOffBits;

  // Write the file header to the file
  ofs.write(reinterpret_cast<const char *>(&bmphdr), sizeof(BITMAPFILEHEADER));

  // Save the file in big chunks.

  // Allocate a large buffer to be used when transferring
  // data to the file

  unsigned char *wbuf = new unsigned char[maxwrite];
  if (wbuf == (0L))
    return 0;

  unsigned char __huge *data = (unsigned char __huge *)imdata->p_dib;
  unsigned int chunksize;
  long bmpsize = bmphdr.bfSize - sizeof(BITMAPFILEHEADER);

  unsigned int i;

  while (bmpsize > 0) {
    if (bmpsize > maxwrite)
      chunksize = maxwrite;
    else
      chunksize = (unsigned int)bmpsize;
    // Copy image from DIB to buffer
    for (i = 0; i < chunksize; i++)
      wbuf[i] = data[i];
    ofs.write(reinterpret_cast<char *>(wbuf), chunksize);
    bmpsize -= chunksize;
    data += chunksize;
  }
  delete wbuf;
  return 1;
}
