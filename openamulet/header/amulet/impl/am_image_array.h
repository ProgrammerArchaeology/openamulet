#ifndef GDEFS_IMAGE_H
#define GDEFS_IMAGE_H

#include "types_wrapper.h"

class Am_Image_Array_Data;
class Am_Style;

class _OA_DL_CLASSIMPORT Am_Image_Array
{
  Am_WRAPPER_DECL (Am_Image_Array)
public:
  // creators
  // create by read from file
  Am_Image_Array();
  Am_Image_Array(const char* file_name);
  Am_Image_Array(int percent);
  Am_Image_Array(char *bit_data, int height, int width);
  Am_Image_Array(unsigned int width, unsigned int height, int depth, Am_Style intial_color);

  Am_Image_Array make_diamond();

  // other functions
  int Get_Bit(int x, int y); // NDY
  void Set_Bit(int x, int y, int val);  //NDY

  // Returns in storage the RGB values for each pixel in the image
  // using 1 byte each for R, for G and for B.  So ...
  // storage must be large enough to accomodate 3*width*height bytes.
  // If top_first return RGB values for top row first
  // else return RGB for bottom row first.
  // Returns true if successful (i.e. implemented).
  bool Get_RGB_Image(unsigned char * storage, bool top_first = true);

  // Size will be zero until the image is drawn.  Get the size of an
  // image through Am_Drawonable::Get_Image_Size.
  _OA_DL_MEMBERIMPORT void Get_Size (int& width, int& height) const;
  
  int Write_To_File (const char* file_name,
		     Am_Image_File_Format form);
				//returns 0 or error code.  NDY
};

_OA_DL_IMPORT extern Am_Image_Array Am_No_Image;

#endif //GDEFS_IMAGE_H
