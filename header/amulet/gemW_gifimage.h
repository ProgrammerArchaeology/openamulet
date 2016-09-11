//---------------------------------------------------------------
//  File: GIFimage.h
//
//  Defines the Am_WinGIFImage class representing GIF images.
//
//  CompuServe requires the following statement in software that
//  uses the GIF image format:
//
//    "The Graphics Interchange Format(c) is the Copyright 
//     property of CompuServe Incorporated. GIF(sm) is a 
//     Service Mark property of CompuServe Incorporated."
//
//---------------------------------------------------------------
#if !defined(__GIFIMAGE_H)
#define __GIFIMAGE_H

#include "gemW_winimage.h"

#pragma pack(push,1)

#define HAS_CT      0x80
#define INTERLACED  0x40
#define GIF_TRANSPARENT 0x01 // transparent image
#define CTSIZE(x)   (x & 0x07)

#define GIF_IMGSTART ','
#define GIF_EXTSTART '!'
#define GIF_TRAILER  ';'
#define GIF_COMMENT  0xfe
#define GIF_GCE		 0xf9 // graphics control extension

#define GIF_MAXCODE   12
#define GIF_TBLSIZE   4096

class Am_WinGIFImage: public Am_WinImage
{
public:
  Am_WinGIFImage ();
  Am_WinGIFImage (HBITMAP hbm, unsigned short w, unsigned short h);
  Am_WinGIFImage (HDC hdc, Am_WinImage *img, short x, short y,
		  unsigned short w, unsigned short h);
  Am_WinGIFImage (HDC hdc, Am_WinImage *img, short x, short y, 
		  unsigned short w, unsigned short h, short neww, short newh);
  // Copy Constructor
  Am_WinGIFImage (const Am_WinImage& img);

  ~Am_WinGIFImage();
    
  int read(const char* filename);
  int write(const char* filename) { return 1;} // Do nothing for now 

  int process_gif(std::ifstream &ifs, unsigned char *image);
  void add_to_image(unsigned char *image, unsigned short pixval);
    
private:
  // Structures for GIF blocks
  struct GIFHeader {
    char   signature[3]; // Should contain: "GIF"
    char   version[3];   // "87a" or "89a"
  };

  struct LogicalScreenDescriptor {
    unsigned short   width;
    unsigned short   height;
    unsigned char    flags;
    unsigned char    bgcolor;
    unsigned char    aspect_ratio;
  };

  struct ImageInfo {
    unsigned short left;
    unsigned short top;
    unsigned short width;
    unsigned short height;
    unsigned char  flags;
  };

  struct GraphicsControlExtension {
    unsigned char blocksize;
    unsigned char flags;
    unsigned short delaytime;
    unsigned char color_index;
  };
 
  // This array is used to store decoded GIF image
  unsigned char           *gif_image; 

  GIFHeader               hdr;
  LogicalScreenDescriptor lsdesc;
  ImageInfo               iminfo;
  GraphicsControlExtension gce;
  unsigned short          interlaced;
  unsigned short          palettesize;
  unsigned short          bits_per_pixel;
  unsigned short          init_code_size;
  unsigned short          code_size;
  unsigned short          clear_code;
  unsigned short          eoi_code;
  unsigned short          max_code;
  unsigned short          free_code;
  unsigned short          curcode;
  unsigned short          oldcode;
  unsigned short          read_mask;
  unsigned short          data_mask;
  unsigned short          pixel;
  unsigned short          *prefix;
  unsigned char           *suffix;
  unsigned char           *stack; 
  unsigned short          istk;   // Stack index
  unsigned short          pass;   // For interlaced images
  unsigned short          x, y;   // Am_WinImage coordinates
  unsigned short		  transparent_pixval;
};

#pragma pack(pop)

#endif
