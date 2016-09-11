/* ****************************** -*-c++-*- *******************************/
/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

// gem_image.h
// 
// Machine dependent low-level image data format.
// This is used for loading files from disk into memory.  The machine 
// dependent gem image code converts this data into something usable, and
// takes care of displaying it, and so on.

// Transparent images:
// To accomodate X11 raster ops, I have forced the transparent color
// to be at index 0 in the color map and to be white (all ones)
// I know that forcing the transparent color to be white is necessary
// I do not know if it is actually necessary to force the transparent
// color index to be 0 (eab 4/18/97)

// A generic colormap entry.
#ifndef GEM_IMAGE_H
#define GEM_IMAGE_H

class _OA_DL_CLASSIMPORT Am_RGB_Value
{
public:
  Am_RGB_Value (): red(0), green(0), blue(0) {}
  Am_RGB_Value (short int r, short int g, short int b): 
    red(r), green(g), blue(b) {}
  Am_RGB_Value (const Am_RGB_Value& proto):
    red(proto.red), green(proto.green), blue(proto.blue) {}
  unsigned short red;
  unsigned short green;
  unsigned short blue;
};

// Information about the image
class _OA_DL_CLASSIMPORT Am_Image_Info
{
public:
  Am_Image_Info ():
    width(0), height(0), depth(0), num_colors(0),
    transparent(false), raw_transparent_color_index(0) {}
  Am_Image_Info (int w, int h, int d, int n, bool t, int tx):
    width(w), height(h), depth(d), num_colors(n),
    transparent(t), raw_transparent_color_index(tx) {}
  Am_Image_Info (Am_Image_Info &proto):
    width(proto.width), height(proto.height), depth(proto.depth), 
    num_colors(proto.num_colors), transparent(proto.transparent),
    raw_transparent_color_index(proto.raw_transparent_color_index) {}
  unsigned short width;
  unsigned short height;
  int depth;
  int num_colors;
  bool transparent;
  unsigned short raw_transparent_color_index;
};

// An abstract generic image class.  Derive particular classes 
// (ie Am_GIF_Image) from this class. 

class _OA_DL_CLASSIMPORT Am_Generic_Image
{
public:
  // The creator for subclasses loads an image from a file if possible, 
  // returning a valid image for success or (0L) for failure.
  //  static Am_Generic_Image* Create(char* filename);
  
  Am_Generic_Image():
    color_map((0L)), imdata (NULL) {}
  Am_Generic_Image (unsigned char* data, int width, int height, int depth):
    iminfo(width, height, depth, 0, false, 0), color_map((0L)), imdata(data) {}
  Am_Generic_Image (Am_Generic_Image* proto):
    iminfo(proto->iminfo), color_map(proto->color_map) {
      // bug: assumes one byte per pixel.
      imdata = new unsigned char[iminfo.width * iminfo.height];
      memcpy (imdata, proto->imdata, iminfo.width * iminfo.height);
  }
  ~Am_Generic_Image () {
    if (imdata!= (0L)) delete[] imdata; 
    if (color_map != (0L)) delete[] color_map; }
protected:
  // data
  Am_Image_Info iminfo;
  Am_RGB_Value* color_map;
  unsigned char* imdata;
  
public:  
  inline void Get_Size (unsigned short &width, unsigned short &height) { 
    width = iminfo.width; 
    height = iminfo.height;  }
  inline unsigned char* Get_Data () {return imdata;}
  inline int Get_Depth () {return iminfo.depth;}
  inline int Get_Color_Map (Am_RGB_Value*& map){
    map = color_map; return iminfo.num_colors; }
  inline bool Is_Transparent () { return iminfo.transparent; }
};

#endif
