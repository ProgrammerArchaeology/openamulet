//---------------------------------------------------------------
//  File: gemW_gifimage.cpp
//
//  Am_WinImage manipulation functions for GIF format images.
//
//  CompuServe requires the following statement in software that
//  uses the GIF image format:
//
//    "The Graphics Interchange Format(c) is the Copyright
//     property of CompuServe Incorporated. GIF(sm) is a
//     Service Mark property of CompuServe Incorporated."
//
//---------------------------------------------------------------
#include <windows.h>
#include <windowsx.h>
#include <fstream>
#include <string.h>

#include <amulet/gemW_32.h>
#include <amulet/gemW_gifimage.h>

// Utility function quantize_bits:
// Device independant bitmaps (specified with BITMAPINFOHEADER) can only have
// 1, 4, 8, 16, 24, or 32 bits per pixel.  So, when we compress the gif-read
// information into a DIB, we have to quantize the GIF bits per pixel (which can be
// any value) into one of these values.  Reading from disk to GIFImage is unquantized,
// converting that to a DIB is quantized.  But we must also specify the correct
// number of colors, and not let it assume that all the pixels are used.  That
// ensures we get the right offset to the image data.
// I discovered all of this reading the online help about BITMAPINFOHEADER and
// CreateDIBitmap.
// 5-22-96 af1x

int
quantize_bits(int b)
{
  return b > 24 ? 32 : b > 16 ? 24 : b > 8 ? 16 : b > 4 ? 8 : b > 1 ? 4 : 1;
}

//---------------------------------------------------------------
// Am_WinGIFImage:: G I F I m a g e
// Constructor for the Am_WinGIFImage class

Am_WinGIFImage::Am_WinGIFImage()
    : palettesize(0), x(0), y(0), pass(1), oldcode(0), interlaced(0), pixel(0),
      gif_image((0L)), istk(0), prefix(NULL), suffix(NULL), stack((0L)),
      transparent_pixval(0)
{
}

Am_WinGIFImage::Am_WinGIFImage(HBITMAP hbm, unsigned short w, unsigned short h)
    : Am_WinImage(hbm, w, h), palettesize(0), x(0), y(0), pass(1), oldcode(0),
      interlaced(0), pixel(0), gif_image((0L)), istk(0), prefix(NULL),
      suffix(NULL), stack((0L)), transparent_pixval(0)
{
}

Am_WinGIFImage::Am_WinGIFImage(HDC hdc, Am_WinImage *img, short x, short y,
                               unsigned short w, unsigned short h)
    : Am_WinImage(hdc, img, x, y, w, h), palettesize(0), x(0), y(0), pass(1),
      oldcode(0), interlaced(0), pixel(0), gif_image((0L)), istk(0),
      prefix(NULL), suffix(NULL), stack((0L)), transparent_pixval(0)
{
}

Am_WinGIFImage::Am_WinGIFImage(HDC hdc, Am_WinImage *img, short x, short y,
                               unsigned short w, unsigned short h, short neww,
                               short newh)
    : Am_WinImage(hdc, img, x, y, w, h, neww, newh), palettesize(0), x(0), y(0),
      pass(1), oldcode(0), interlaced(0), pixel(0), gif_image((0L)), istk(0),
      prefix(NULL), suffix(NULL), stack((0L)), transparent_pixval(0)
{
}

Am_WinGIFImage::Am_WinGIFImage(const Am_WinImage &img)
    : Am_WinImage(img), palettesize(0), x(0), y(0), pass(1), oldcode(0),
      interlaced(0), pixel(0), gif_image((0L)), istk(0), prefix(NULL),
      suffix(NULL), stack((0L)), transparent_pixval(0)
{
}

//---------------------------------------------------------------
// Am_WinGIFImage:: ~ G I F I m a g e
// Destructor for the Am_WinGIFImage class

Am_WinGIFImage::~Am_WinGIFImage()
{
  if (prefix != (0L))
    delete prefix;
  if (suffix != (0L))
    delete suffix;
  if (stack != (0L))
    delete stack;
  if (gif_image != (0L))
    delete gif_image;
}

//---------------------------------------------------------------
//  G I F I m a g e : : r e a d
//  Read a GIF image.

int
Am_WinGIFImage::read(const char *filename)
{
  prefix = new unsigned short[GIF_TBLSIZE];
  suffix = new unsigned char[GIF_TBLSIZE];
  stack = new unsigned char[GIF_TBLSIZE];
  istk = 0;

  // If the prefix, suffix, and stack arrays are not allocated
  // successfully, do nothing.
  if (prefix == (0L) || suffix == NULL || stack == NULL)
    return 0;

  unsigned char buf[256]; // buffer for a block of image data

  // If there is an existing image, detach the image data
  // before reading a new image
  if (imdata->p_dib != 0)
    detach();

  // Open file for reading
  std::ifstream ifs(filename, std::ios::in | std::ios::binary);
  if (!ifs)
    return 0; // error reading file.

  // Read GIF header
  ifs.read((char *)&hdr, sizeof(GIFHeader));
  // Check if image file format is acceptable
  if (strncmp(hdr.signature, "GIF", 3) != 0 && hdr.version[0] != '8')
    return 0; // error

  // Read the logical screen descriptor
  ifs.read((char *)&lsdesc, sizeof(LogicalScreenDescriptor));

  // If a global color table is present, read it.
  RGBTRIPLE *ctbl = (0L);
  if (lsdesc.flags & HAS_CT) {
    bits_per_pixel = CTSIZE(lsdesc.flags) + 1;
    palettesize = 1 << bits_per_pixel;
    data_mask = palettesize - 1;
    ctbl = new RGBTRIPLE[palettesize];
    if (ctbl == (0L))
      return 0;

    ifs.read((char *)ctbl, palettesize * sizeof(RGBTRIPLE));
  }

  // Next comes a series of graphics blocks or special-purpose
  // blocks
  int byte = 0, read_an_image = 0;
  while (!read_an_image && (byte = ifs.get()) != GIF_TRAILER) {
    // Locate an image descriptor and read the image data
    int c;
    switch (byte) {
    case GIF_EXTSTART: {
      // Skip extension blocks
      c = ifs.get();
      if (c == GIF_COMMENT) {
      } else if (c == GIF_GCE) {
        ifs.read((char *)&gce, sizeof(GraphicsControlExtension));
        if (gce.flags & GIF_TRANSPARENT) {
          imdata->transparent = 1;
          transparent_pixval = gce.color_index;
          imdata->transparent_index = transparent_pixval;
        }
      } else {
        int blksize = ifs.get();
        ifs.read((char *)buf, blksize);
      }
      while ((c = ifs.get()) != 0)
        ifs.read((char *)buf, c);
      break;
    }
    case GIF_IMGSTART: {
      // Read GIF image information
      ifs.read((char *)&iminfo, sizeof(ImageInfo));
      if (iminfo.flags & INTERLACED)
        interlaced = 1;

      // Allocate memory for the GIF image and read in GIF image
      // into this array
      long GIF_image_size = (long)iminfo.width * (long)iminfo.height;
      gif_image = new unsigned char __huge[GIF_image_size];
      if (gif_image == (0L)) {
        if (ctbl != (0L))
          delete ctbl;
        return 0;
      }

      // Read local color table, if any
      if (iminfo.flags & HAS_CT) {
        bits_per_pixel = CTSIZE(iminfo.flags) + 1;
        palettesize = 1 << bits_per_pixel;
        data_mask = palettesize - 1;
        if (ctbl != (0L))
          delete ctbl;
        ctbl = new RGBTRIPLE[palettesize];
        if (ctbl == (0L))
          return 0;
        ifs.read((char *)ctbl, palettesize * sizeof(RGBTRIPLE));
      }
      // Next byte is the initial number of bits used for LZW codes
      init_code_size = ifs.get();

      if (init_code_size < 2 || init_code_size > 8) {
        if (ctbl != (0L))
          delete ctbl;
        return 0;
      }
      clear_code = 1 << init_code_size;
      eoi_code = clear_code + 1;
      free_code = clear_code + 2;

      // The GIF specification says that, for decompression, the code
      // size should be one bit longer than the initial code size.
      code_size = init_code_size + 1;

      // Bit mask needed to extract data bits corresponding to the
      // code size.
      max_code = 1 << code_size;
      read_mask = max_code - 1;
      // Process the encoded data stream from the file.
      if (process_gif(ifs, gif_image) == 0) {
        if (ctbl != (0L))
          delete ctbl;
        return 0;
      }
      read_an_image = 1;
      break;
    }
    default: {
      if (ctbl != (0L))
        delete ctbl;
      return 0;
      break;
    }
    }
  }

  // Allocate memory for the device independent bitmap (DIB)
  // Note that the number of bytes in each line of a DIB image
  // must be a multiple of 4.

  unsigned short actual_bytes_per_line =
      (iminfo.width * quantize_bits(bits_per_pixel) + 7) / 8;

  imdata->bytes_per_line = actual_bytes_per_line;

  if (imdata->bytes_per_line % 4)
    imdata->bytes_per_line = 4 * (imdata->bytes_per_line / 4 + 1);

  imdata->p_dib = GlobalAllocPtr(
      GHND, sizeof(BITMAPINFOHEADER) + palettesize * sizeof(RGBQUAD) +
                (long)imdata->bytes_per_line * (long)iminfo.height);

  // If memory allocation fails, return 0
  if (imdata->p_dib == 0)
    return 0;

  // Set up bitmap info header
  LPBITMAPINFOHEADER p_bminfo = (LPBITMAPINFOHEADER)imdata->p_dib;
  p_bminfo->biSize = sizeof(BITMAPINFOHEADER);
  p_bminfo->biWidth = iminfo.width;
  p_bminfo->biHeight = iminfo.height;
  p_bminfo->biPlanes = 1;
  // biBitCount must be 1, 4, 8, 16, 24, or 32
  p_bminfo->biBitCount = quantize_bits(bits_per_pixel);
  p_bminfo->biCompression = BI_RGB;
  p_bminfo->biSizeImage = (long)iminfo.height * (long)imdata->bytes_per_line;
  p_bminfo->biXPelsPerMeter = 0;
  p_bminfo->biYPelsPerMeter = 0;
  p_bminfo->biClrUsed = palettesize;
  p_bminfo->biClrImportant = 0;

  // Set up the color palette
  if (palettesize > 0) {
    RGBQUAD *palette =
        (RGBQUAD *)((LPSTR)imdata->p_dib + sizeof(BITMAPINFOHEADER));

    for (unsigned int i = 0; i < palettesize; i++) {
      // Load palette from the "ctbl" array (the colors get loaded
      // in the wrong order in the ctbl array, hence the apparent
      // mismatch of color names in the following statements).
      palette[i].rgbRed = ctbl[i].rgbtBlue;
      palette[i].rgbGreen = ctbl[i].rgbtGreen;
      palette[i].rgbBlue = ctbl[i].rgbtRed;
      palette[i].rgbReserved = 0;
    }
  }
  if (ctbl != (0L))
    delete ctbl;

  // Load image data into the DIB. Note the DIB image must be
  // stored "bottom to top" line order. That's why we position
  // data at the end of the array so that the image can be
  // stored backwards--from the last line to the first.
  unsigned char *data = (unsigned char *)imdata->p_dib +
                        sizeof(BITMAPINFOHEADER) +
                        palettesize * sizeof(RGBQUAD) +
                        (unsigned long)(iminfo.height - 1) *
                            (unsigned long)(imdata->bytes_per_line);

  // Define a macro to access bytes in the GIF image according
  // to specified line and byte index.

  unsigned int lineindex, byteindex;

#define bytepos(lineindex, byteindex)                                          \
  ((long)(lineindex) * (long)iminfo.width + (long)(byteindex))

  // Construct packed pixels out of decoded GIF image.

  unsigned short onebyte, bits_copied, loc, few_bits, k,
      bbpb = 8 / (quantize_bits(bits_per_pixel));

  for (lineindex = 0; lineindex < iminfo.height;
       lineindex++, data -= imdata->bytes_per_line) {
    if (actual_bytes_per_line < imdata->bytes_per_line) {
      for (loc = actual_bytes_per_line; loc < imdata->bytes_per_line; loc++) {
        data[loc] = 0;
      }
    }
    loc = 0;
    onebyte = 0;
    bits_copied = 0;
    for (byteindex = 0; byteindex < iminfo.width;) {
      for (k = 0; k < bbpb; k++, byteindex++) {
        if (byteindex >= iminfo.width)
          few_bits = 0;
        else
          few_bits = gif_image[bytepos(lineindex, byteindex)];
        // af1x 5-21-96
        // The outer loop won't break if byteindex > iminfo.width here.
        // Garbage bytes after the gif_image data might munge our real picture
        // data here.  We can't break out because we need things to be
        // shifted the right amount and stuck into data[].

        // Shift the current value first
        if (k > 0)
          onebyte <<= quantize_bits(bits_per_pixel);

        // Then OR the new bits with current pixel
        onebyte |= few_bits;
        bits_copied += quantize_bits(bits_per_pixel);

        if (bits_copied >= 8) {
          data[loc] = (unsigned char)onebyte;
          loc++;
          bits_copied = 0;
          onebyte = 0;
        }
      }
    }
  }
  // Delete arrays that are no longer needed
  delete gif_image;
  delete suffix;
  delete prefix;
  delete stack;
  gif_image = (0L);
  suffix = (0L);
  prefix = (0L);
  stack = (0L);

  // Success!
  return 1;
}
//---------------------------------------------------------------
// Am_WinGIFImage:: p r o c e s s _ g i f
// Processes a LZW data stream from a GIF file. Decoded data is
// placed in "image" array.

int
Am_WinGIFImage::process_gif(std::ifstream &ifs, unsigned char *image)
{
  // Buffer to hold a block of image data from the GIF file.
  unsigned char buf[256];

  unsigned short count, input_code, bitpos = 0;
  unsigned long val = 0, tmp;

  ifs.clear(0);

  for (count = ifs.get(); count > 0; count = ifs.get()) {
    // Read a block of data into the buffer
    ifs.read((char *)buf, count);
    if (ifs.fail())
      return 0;

    // Process the contents of the buffer
    for (unsigned int i = 0; i < count; i++) {
      tmp = buf[i];
      val |= (tmp << bitpos);
      bitpos += 8;

      while (bitpos >= code_size) {
        curcode = (unsigned short)(val & read_mask);
        val >>= code_size;
        bitpos -= code_size;
        if (curcode == eoi_code)
          return 1;
        if (curcode == clear_code) {
          free_code = (1 << init_code_size) + 2;
          code_size = init_code_size + 1;
          max_code = 1 << code_size;
          read_mask = max_code - 1;
          istk = 0;
          oldcode = 0xffff;
        } else {
          if (oldcode == 0xffff) {
            // First code, after initialization, is a raw pixel value
            pixel = curcode & data_mask;
            add_to_image(image, pixel);
            oldcode = curcode;
          } else {
            input_code = curcode;
            if (curcode >= free_code) {
              // Code is not in table yet. Save last character in stack
              stack[istk++] = (unsigned char)pixel;
              curcode = oldcode;
            }
            while (curcode > data_mask) {
              // [eab] I don't know enough about the LZW algorithm
              // to introduce more precise range tests than the following
              if (curcode >= GIF_TBLSIZE || istk >= GIF_TBLSIZE - 2)
                return 0; // corrupt GIF

              // Save bytes corresponding to compression code in stack for
              // later use.
              stack[istk++] = suffix[curcode];
              curcode = prefix[curcode];
            }

            pixel = curcode & data_mask;
            stack[istk++] = (unsigned char)pixel;

            // Add decoded string of bytes to image
            short j;
            for (j = istk - 1; j >= 0; j--) {
              if (interlaced)
                add_to_image(image, stack[j]);
              else {
                image[(long)x + (long)iminfo.width * (long)y] = stack[j];
                // Adjust the coordinates, if necessary
                x++;
                if (x == iminfo.width) {
                  x = 0;
                  y++;
                }
              }
            }
            istk = 0;

            // Add current information to decompression tables
            prefix[free_code] = oldcode;
            suffix[free_code] = (unsigned char)pixel;
            oldcode = input_code;

            // Adjust code size, if necessary.
            free_code++;
            if (free_code >= max_code) {
              if (code_size < GIF_MAXCODE) {
                code_size++;
                max_code *= 2;
                read_mask = max_code - 1;
              }
            }
          }
        }
      }
    }
  }
  return 1;
}
//---------------------------------------------------------------
// Am_WinGIFImage:: a d d _ t o _ i m a g e
// Add a pixel to the image

void
Am_WinGIFImage::add_to_image(unsigned char *image, unsigned short pixval)
{
  image[x + iminfo.width * y] = (unsigned char)pixval;

  // Adjust the coordinates
  x++;
  if (x == iminfo.width) {
    x = 0;
    if (!interlaced)
      y++;
    else {
      switch (pass) {
      case 1: // Every 8th row, start at row 0
        y += 8;
        if (y >= iminfo.height) {
          y = 4;
          pass++;
        }
        break;
      case 2: // Every 8th row, start at row 4
        y += 8;
        if (y >= iminfo.height) {
          y = 2;
          pass++;
        }
        break;
      case 3: // Every 4th row, start at row 2
        y += 4;
        if (y >= iminfo.height) {
          y = 1;
          pass++;
        }
        break;
      case 4: // Every 2nd row, start at row 1
        y += 2;
        break;
      }
    }
  }
}
