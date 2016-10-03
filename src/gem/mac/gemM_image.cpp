/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file contains low-level objects to support bitmaps, pixmaps, and
   stippled line and filling styles.
   Special thanks to Kenneth Worley for CopyBits Demo
*/

#include <stdio.h>
#include <iostream>

#include <am_inc.h>

#include <amulet/gdefs.h>
#include <amulet/gem.h>
#include <amulet/gemM.h>
#include "gem_GIF.h"

static int i = 0;

//Function Prototypes
void install_pattern(Pattern &the_pattern, int index);

// // // // // // // // // // // // // // // // // // // // // // // // // //
// bitmap data for standard stipples
// // // // // // // // // // // // // // // // // // // // // // // // // //

// You can see the bit pattern corresponding to these numbers by looking
// in the old Garnet lisp file opal/halftones.lisp.  The Garnet file
// opal/mac.lisp contains the function MAC-device-image converts the bit
// pattern into the numbers below.
//
static char stipples[][4] = {{0x00, 0x0, 0x0, 0x0},    {0x88, 0x0, 0x0, 0x0},
                             {0x88, 0x0, 0x22, 0x0},   {0xAA, 0x0, 0x22, 0x0},
                             {0xAA, 0x0, 0xAA, 0x0},   {0xAA, 0x44, 0xAA, 0x0},
                             {0xAA, 0x44, 0xAA, 0x11}, {0xAA, 0x55, 0xAA, 0x11},
                             {0xAA, 0x55, 0xAA, 0x55}, {0xAA, 0x55, 0xAA, 0x77},
                             {0xAA, 0xDD, 0xAA, 0x77}, {0xAA, 0xDD, 0xAA, 0xFF},
                             {0xAA, 0xFF, 0xAA, 0xFF}, {0xAA, 0xFF, 0xBB, 0xFF},
                             {0xEE, 0xFF, 0xBB, 0xFF}, {0xEE, 0xFF, 0xFF, 0xFF},
                             {0xFF, 0xFF, 0xFF, 0xFF}};

// Fill eight slots with four characters (repeat the same four)
void
install_pattern(Pattern &the_pattern, int index)
{
  memcpy(the_pattern.pat, stipples[index], sizeof(char) * 4);
  memcpy((the_pattern.pat) + (sizeof(char) * 4), stipples[index],
         sizeof(char) * 4);
}

// // // // // // // // // // // // // // // // // // // //
// Am_Image_Array
// // // // // // // // // // // // // // // // // // // //

AM_WRAPPER_IMPL(Am_Image_Array)

/////
// Am_Image_Array constructors.
/////

Am_Image_Array::Am_Image_Array() { data = (0L); }

Am_Image_Array::Am_Image_Array(const char *file_name)
{
  data = new Am_Image_Array_Data(file_name);
}

Am_Image_Array::Am_Image_Array(unsigned int width, unsigned int height,
                               int /*depth*/,
                               // default color 0
                               Am_Style /*initial_color*/)
{
  int nBytes = ((width + 7) / 8) * height;
  char *bit_data = new char[nBytes];
  for (int i = 0; i < nBytes; i++) {
    bit_data[i] = 0;
  }

  data = new Am_Image_Array_Data(bit_data, width, height);
}

Am_Image_Array::Am_Image_Array(int percent)
{
  if ((percent < 0) || (percent > 100)) {
    fprintf(stderr, "%d is not a valid percent.  Please use a number"
                    "between 0 and 100\n",
            percent);
    fprintf(stderr, "Creating a null image instead.\n");
    return;
  }

  int nbits = (int)((float)percent / 5.88);
  Pattern the_pattern;
  install_pattern(the_pattern, nbits);

  data = new Am_Image_Array_Data;
  ;
  data->pattern = the_pattern;
}

Am_Image_Array::Am_Image_Array(char *bit_data, int height, int width)
{
  data = new Am_Image_Array_Data(bit_data, height, width);
  // BUG: clean up this pattern creation
  memcpy(data->pattern.pat, bit_data, 8);
}

/////
// Am_Image_Array creators
/////

Am_Image_Array Am_No_Image((Am_Wrapper *)(0L));

////
// Am_Image_Array_Data constructors
////

AM_WRAPPER_DATA_IMPL(Am_Image_Array, (this))

Am_Image_Array_Data::Am_Image_Array_Data(const char *file_name)
{
  gworld = (0L);
  transp = false;
  transp_color = Am_RGB_Value(0x00, 0x00, 0x00);
  name = new char[strlen(file_name) + 1];
  strcpy(name, file_name);
}

Am_Image_Array_Data::Am_Image_Array_Data(const char *bit_data, int h, int w)
{
  gworld = (0L);
  transp = false;
  transp_color = Am_RGB_Value(0x00, 0x00, 0x00);
  load_bit_data((unsigned char *)bit_data, w, h);
  name = (0L);
}

Am_Image_Array_Data::Am_Image_Array_Data()
{
  gworld = (0L);
  transp = false;
  //  transp_color = Am_RGB_Value(0xFF,0xFF,0xFF);
  transp_color = Am_RGB_Value(0x00, 0x00, 0x00);
  name = (0L);
}

Am_Image_Array_Data::Am_Image_Array_Data(Am_Image_Array_Data *proto)
{
  gworld = proto->gworld;
  transp = proto->transp;
  transp_color = proto->transp_color;
  name = new char[strlen(proto->name) + 1];
  strcpy(name, proto->name);
}

/////
// Am_Image_Array_Data destructor
/////

Am_Image_Array_Data::~Am_Image_Array_Data()
{
  delete[] name;
  if (gworld)
    DisposeGWorld(gworld);
}

/////
// Am_Image_Array setters and getters
/////

// NDY: implementations of getbit, setbit, write_to_file
int
Am_Image_Array::Get_Bit(int /* x */, int /* y */)
{
  if (data)
    return 0; // return something useful
  else
    return 0;
}

void
Am_Image_Array::Set_Bit(int /* x */, int /* y */, int /* val */)
{
  if (data)
    data = Am_Image_Array_Data::Narrow(data->Make_Unique());
  // then do something useful
}

// Returns in storage the RGB values for each pixel in the image
// using 1 byte each for R, for G and for B.  So ...
// storage must be large enough to accommodate 3*width*height bytes.
// If top_first return RGB values for top row first
// else return RGB for bottom row first.
// Returns true if successful (i.e. implemented).
bool
Am_Image_Array::Get_RGB_Image(unsigned char *storage, bool top_first)
{
  return false;
}

int
Am_Image_Array::Write_To_File(const char * /* file_name */,
                              Am_Image_File_Format /* form */)
{
  if (data)
    // do something useful
    return 0;
  else
    return 0;
}
// end of NDY stuff.

void
Am_Drawonable_Impl::Get_Image_Size(const Am_Image_Array &image, int &ret_width,
                                   int &ret_height)
{
  Am_Image_Array_Data *data = (Am_Image_Array_Data::Narrow(image));

  if (data) {
    // Make sure the image has been created for this display
    data->Get_Mac_PixMap(this);
    image.Get_Size(ret_width, ret_height);
    data->Release();
  }
}

// This function is not guaranteed to return the expected width and height
// of the bitmap, since it may not have been created on the display yet.
// You should call Am_Drawonable::Get_Image_Size() instead.
void
Am_Image_Array::Get_Size(int &ret_width, int &ret_height) const
{
  if (data) {
    PixMapHandle pixMap = GetGWorldPixMap(data->gworld);

    ret_width = (**pixMap).bounds.right - (**pixMap).bounds.left;
    ret_height = (**pixMap).bounds.bottom - (**pixMap).bounds.top;
  }
}

/////
// Am_Image_Array_Data setters/getters
////

PixMapHandle
Am_Image_Array_Data::Get_Mac_PixMap(Am_Drawonable_Impl * /* draw */)
{
  if (!gworld && name)
    load_image_from_file();

  // 7 March 1997: Andrew Faulring
  // removed this return statement and inserted new checking
  //     return gworld ? GetGWorldPixMap( gworld ) : (0L);
  // issue: should we return an error if the gworld is invalid?

  //if( !gworld )
  //  Am_Error("Tried to 'Get_Mac_PixMap', but was unable to load image file.");

  // return GetGWorldPixMap( gworld );

  // bdk 1/6/98 unfortunately, load_image_from_file does not create
  // a gworld yet it only loads the image is there is no gworld

  return gworld ? GetGWorldPixMap(gworld) : (0L);
}

void
Am_Image_Array_Data::load_image_from_file()
{
  // cheap test if it's a gif
  int l = strlen(name);
  if (!strcmp(&(name[l - 4]), ".gif")) {

    // then assume it's gif
    Am_Generic_Image *image = Am_GIF_Image::Create(name);
    if (image) {

      // Extract info from Am_Image_Info
      unsigned short width, height;
      int depth, num_colors;
      Am_RGB_Value *color_map;

      image->Get_Size(width, height);
      depth = image->Get_Depth();
      num_colors = image->Get_Color_Map(color_map);
      transp = image->Is_Transparent();
      transp_color = color_map
          [0]; // transp color has already been adjusted to pos 0 during create
      load_image_data(image->Get_Data(), width, height, depth, color_map,
                      num_colors);
    }
  } else
    Am_Error("Tried to load a non-gif file. Mac Gem only supports gifs.");
}

void
Am_Image_Array_Data::load_bit_data(const unsigned char *bit_data,
                                   unsigned short width, unsigned short height)
{
  if (gworld)
    DisposeGWorld(gworld);

  // Create offscreen GWorld (assuming 1 bit per pixel)
  Rect bounds;
  SetRect(&bounds, 0, 0, width, height);
  NewGWorld(&gworld, 1, &bounds, (0L), nullptr, (GWorldFlags)0);

  // Copy data into GWorld
  PixMapHandle pixMap = GetGWorldPixMap(gworld);
  if (LockPixels(pixMap)) {
    Ptr p = (**pixMap).baseAddr;
    short rowBytes = (**pixMap).rowBytes & 0x7FFF;

    int nBytes = (width + 7) / 8;
    for (int i = 0; i < height; i++) {
      memcpy(p, bit_data, nBytes);
      bit_data += nBytes;
      p += rowBytes;
    }
  }
  UnlockPixels(pixMap);
}

void
Am_Image_Array_Data::load_image_data(const unsigned char *bit_data,
                                     unsigned short width,
                                     unsigned short height, int /* depth */,
                                     Am_RGB_Value *color_map, int num_colors)
{
  if (gworld)
    DisposeGWorld(gworld);

  // Make Color Table
  CTabHandle ctab = (CTabHandle)NewHandle(sizeof(ColorTable) +
                                          num_colors * sizeof(ColorSpec));
  if (ctab) {
    (**ctab).ctSeed = GetCTSeed();
    (**ctab).ctFlags = 0;
    (**ctab).ctSize = num_colors;
    for (int i = 0; i < num_colors; i++) {
      (**ctab).ctTable[i].value = i;
      (**ctab).ctTable[i].rgb.red = (color_map[i].red << 8) + color_map[i].red;
      (**ctab).ctTable[i].rgb.green =
          (color_map[i].green << 8) + color_map[i].green;
      (**ctab).ctTable[i].rgb.blue =
          (color_map[i].blue << 8) + color_map[i].blue;
    }

    // image data is always given assuming 1 byte per pixel

    Rect bounds;
    SetRect(&bounds, 0, 0, width, height);
    gworld = (0L);
    NewGWorld(&gworld, 8, &bounds, ctab, (0L), (GWorldFlags)0);

    // Copy data into GWorld
    PixMapHandle destMap = GetGWorldPixMap(gworld);
    if (LockPixels(destMap)) {
      Ptr p = (**destMap).baseAddr;
      short rowBytes = (**destMap).rowBytes & 0x7FFF;

      int nBytes = width;
      for (int i = 0; i < height; i++) {
        memcpy(p, bit_data, nBytes);
        bit_data += nBytes;
        p += rowBytes;
      }
    }
    UnlockPixels(destMap);

    //    *** What is wrong with UpdateGWorld?
    //    if (depth != 8) {
    //      UpdateGWorld(&gworld, depth, &bounds, ctab, (0L), (GWorldFlags)clipPix);
    //    }

    DisposeHandle((Handle)ctab);
  }
}

/////
// Am_Image_Array drawing function.
/////

// if width is -1, then it ignores them and uses the ones set in the image

void Am_Drawonable_Impl::Draw_Image(
    int left, int top, int width, int height, const Am_Image_Array &image,
    int i_left,               // default: 0
    int i_top,                // default: 0
    const Am_Style &ls,       // default: Am_No_Style -- color of 'on' bits
    const Am_Style &fs,       // default: Am_No_Style -- background behind image
    bool draw_monochrome,     // use ls and fs in place of colors
    Am_Draw_Function /* f */) // default: Am_DRAW_COPY
{
#ifdef DEBUG
  if (!mac_port) {
    Am_Error("mac_port not set for Am_Drawonable_Impl::Draw_Imag");
    return;
  }
#endif

  if ((fs == Am_No_Style) && (ls == Am_No_Style))
    return;

  focus_on_this();

  Am_Image_Array_Data *image_data = Am_Image_Array_Data::Narrow(image);
  if (!image_data)
    return; // can not do any drawing if there is no image data

  PixMapHandle pixMap = image_data->Get_Mac_PixMap(this);
  bool is_transparent = image_data->Is_Transparent();
  Am_RGB_Value transp_color = image_data->Transp_Color();
  image_data->Release();

  if (!pixMap)
    return; // no pixMap, so can not do any drawing

  int i_width, i_height;
  if (width < 0)
    Get_Image_Size(image, i_width, i_height);
  else {
    i_width = width;
    i_height = height;
  }

  if (!LockPixels(pixMap)) {
    // could be error here, we cant lock and cannot draw
    Am_ERROR("Cannot lock pixMap for drawing");
    return;
  }

  // The idea of transparent and opaque bitmaps is implemented here, but it
  // is not through the fill-flag of any style object.  The way to make a
  // bitmap transparent is to just not pass any fill-style.  If a fill-style
  // is passed, the bitmap will be opaque (i.e., the 'off' bits will be drawn)
  // with the color of the fill-style.

  Rect fromRect;
  Rect toRect;
  Rect intermediateRect;
  SetRect(&fromRect, i_left, i_top, i_width, i_height);
  SetRect(&toRect, left, top, left + i_width, top + i_height);
  SetRect(&intermediateRect, 0, 0, i_width, i_height);

  // we have to copy the background to an intermediate offscreen
  // then we stamp on the new image, transparent if it is, srcCopy if it is not transparent
  if (!draw_monochrome) {
    if (!fs.Valid()) {

      // -----------------------------------------------
      // if we have color and No_Style as the fill style
      DrawingPort offScrPix1(&intermediateRect);

      if (BitMap *bits = offScrPix1.GetBitMap()) { // assignment
        CopyBits(&((GrafPtr)mac_port)->portBits, bits, &toRect,
                 &intermediateRect, srcCopy, (0L));
        int transferMode = is_transparent ? transparent : srcCopy;
        CopyBits((BitMap *)*pixMap, bits, &fromRect, &intermediateRect,
                 transferMode, (0L));
        CopyBits(bits, &((GrafPtr)mac_port)->portBits, &intermediateRect,
                 &toRect, srcCopy, (0L));

      } // else we have a bad pixmap and have thrown an error during offScrPix1 constructor
      // ------------------------------------------------

    } else {

      // ------------------------------------------------
      DrawingPort offScrPix2(&intermediateRect);
      BitMap *bits2 = offScrPix2.GetBitMap();
      if (bits2) {
        if (!is_transparent) {
          if (fs != Am_No_Style)
            set_back_color(fs);
          if (ls != Am_No_Style)
            set_fore_color(ls);

          PaintRect(&intermediateRect); // paints to bits2
          CopyBits(bits2, &((GrafPtr)mac_port)->portBits, &fromRect, &toRect,
                   srcCopy, (0L));

          int transferMode = is_transparent ? srcCopy : transparent;
          // transparent here makes the opaque bitmap dominate the screen

          CopyBits((BitMap *)*pixMap, bits2, &fromRect, &intermediateRect,
                   transparent, (0L));
          CopyBits(bits2, &((GrafPtr)mac_port)->portBits, &intermediateRect,
                   &toRect, srcCopy, (0L));

        } else {
          // first fill the back of the rect with the fs color
          if (fs != Am_No_Style)
            set_fore_color(fs);

          // ignor the line color since we are in color mode

          PaintRect(&intermediateRect); // paints background to bits2
          CopyBits(bits2, &((GrafPtr)mac_port)->portBits, &fromRect, &toRect,
                   srcCopy, (0L));

          DrawingPort offScrPix3(&intermediateRect);

          if (BitMap *bits3 = offScrPix3.GetBitMap()) { // assignment

            CopyBits(bits2, bits3, &intermediateRect, &intermediateRect,
                     srcCopy, (0L));
            int transferMode = is_transparent ? transparent : srcCopy;
            CopyBits((BitMap *)*pixMap, bits3, &fromRect, &intermediateRect,
                     transferMode, (0L));
            CopyBits(bits3, &((GrafPtr)mac_port)->portBits, &intermediateRect,
                     &toRect, srcCopy, (0L));

          } // else we have a bad pixmap and have thrown an error during offScrPix3 constructor
        }
      } //else we have a bad pixmap and have thrown an error during offScrPix2 constructor

      // ------------------------------------------------
    }
  } else {
    // we have monochrome images
    DrawingPort offScrPix4(&intermediateRect);
    BitMap *bits4 = offScrPix4.GetBitMap();
    if (bits4) {
      if (fs != Am_No_Style)
        set_back_color(fs);
      if (ls != Am_No_Style)
        set_fore_color(ls);

      PaintRect(&intermediateRect); // paints to bits2
      CopyBits(bits4, &((GrafPtr)mac_port)->portBits, &fromRect, &toRect,
               srcCopy, (0L));

      int transferMode = is_transparent ? srcCopy : transparent;
      // transparent here makes the opaque bitmap dominate the screen
      CopyBits((BitMap *)*pixMap, &((GrafPtr)mac_port)->portBits, &fromRect,
               &toRect, transferMode, (0L));

    } //else we have a bad pixmap and have thrown an error during offScrPix4 constructor
  }
  UnlockPixels(pixMap);
  // do not SetGWorld(origPort, origDevice) here
}
