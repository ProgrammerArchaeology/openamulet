//
// ------------------------------------------------------------------------
// 		The OpenAmulet User Interface Development Environment
// ------------------------------------------------------------------------
// This code is based on the Amulet project of Carnegie Mellon University,
// and has been placed in the public domain.  If you are using this code
// or any part of OpenAmulet, please contact amulet@cs.cmu.edu to be put
// on the mailing list or have a look at:
// http://www.openip.org
//

// This file contains low-level objects to support bitmaps, pixmaps, and
// stippled line and filling styles.


#ifdef __VMS
#include <stdlib.h>
#else
#include <stdio.h>
#endif
#include <iostream.h>
#include <am_inc.h>

#include GDEFS__H
#include GEM__H
#include GEMG__H
#include "amulet/gem_GIF.h"

// // // // // // // // // // // // // // // // // // // // // // // // // //
// bitmap data for standard stipples
// // // // // // // // // // // // // // // // // // // // // // // // // //

static const char stipples[][4] =
{
  {0x00, 0x00, 0x00, 0x00},
  {0x01, 0x00, 0x00, 0x00},
  {0x01, 0x00, 0x04, 0x00},
  {0x05, 0x00, 0x04, 0x00},
  {0x05, 0x00, 0x05, 0x00},
  {0x05, 0x02, 0x05, 0x00},
  {0x05, 0x02, 0x05, 0x08},
  {0x05, 0x0a, 0x05, 0x08},
  {0x05, 0x0a, 0x05, 0x0a},
  {0x05, 0x0a, 0x05, 0x0e},
  {0x05, 0x0b, 0x05, 0x0e},
  {0x05, 0x0b, 0x05, 0x0f},
  {0x05, 0x0f, 0x05, 0x0f},
  {0x05, 0x0f, 0x0d, 0x0f},
  {0x07, 0x0f, 0x0b, 0x0f},
  {0x07, 0x0f, 0x0f, 0x0f},
  {0x0f, 0x0f, 0x0f, 0x0f}
};

// // // // // // // // // // // // // // // // // // // //
// Am_Image_Array
// // // // // // // // // // // // // // // // // // // //

Am_WRAPPER_IMPL (Am_Image_Array);

/////
// Am_Image_Array constructors.
/////

Am_Image_Array::Am_Image_Array () : data(0L)
{ }

Am_Image_Array::Am_Image_Array (const char* file_name)
{
   data = new Am_Image_Array_Data(file_name);
}

Am_Image_Array::Am_Image_Array (unsigned int width,
				unsigned int height, int depth,
				// default color 0 TODO: Check this...
				Am_Style /*initial_color*/)
{
  data = new Am_Image_Array_Data();
  unsigned char* temp = new unsigned char[width * height];
  data->generic_image = new Am_Generic_Image(temp, width, height, depth);
  for (unsigned int i = 0; i < (height * width); i++)
    temp[i] = 0;
}

Am_Image_Array::Am_Image_Array (int percent)
{

  if ((percent < 0) || (percent > 100))
    {
     std::cerr << percent << " is not a valid percent.  Please use a number"
	   << "between 0 and 100" << endl
	   << "Creating a null image instead." <<std::endl;
      data = 0L;
      return;
    }
  int nbits = (int)((float) percent / 5.88);
  data = new Am_Image_Array_Data(stipples[nbits], 4, 4);
}

Am_Image_Array::Am_Image_Array(char *bit_data, int height, int width)
{
  data = new Am_Image_Array_Data(bit_data, height, width);
}

// Constant object
Am_Image_Array Am_No_Image;

////
// Am_Image_Array_Data constructors
////

Am_WRAPPER_DATA_IMPL (Am_Image_Array, (this));

Am_Image_Array_Data::Am_Image_Array_Data (const char* file_name)
  : generic_image(0L)
{
  name = new char [strlen(file_name) + 1];
  strcpy (name, file_name);
}

Am_Image_Array_Data::Am_Image_Array_Data (const char* bit_data, int h, int w)
  : name(0L)
{
   const int nbytes = (((w + 7) / 8) * h);
   unsigned char* temp = new unsigned char [nbytes];
   memcpy(temp, bit_data, nbytes);
   generic_image = new Am_Generic_Image ((unsigned char*)temp, w, h, 1);
}

Am_Image_Array_Data::Am_Image_Array_Data () : generic_image(0L), name(0L)
{ }

Am_Image_Array_Data::Am_Image_Array_Data (Am_Image_Array_Data* proto)
{
  generic_image = new Am_Generic_Image(proto->generic_image);
  name = new char [strlen(proto->name) + 1];
  strcpy (name, proto->name);
}

/////
// Am_Image_Array_Data destructor
/////

Am_Image_Array_Data::~Am_Image_Array_Data ()
{
  if (name) delete[] name;
  if (generic_image) delete generic_image;
}


/////
// Am_Image_Array setters and getters
/////

// NDY: implementations of getbit, setbit, write_to_file
int Am_Image_Array::Get_Bit(int /*x*/, int /*y*/)
{
  if (data)
    return 0; // return something useful
  else return 0;
}
void Am_Image_Array::Set_Bit(int /*x*/, int /*y*/, int /*val*/)
{
  if (data)
    data = Am_Image_Array_Data::Narrow(data->Make_Unique());
    // then do something useful
}
int Am_Image_Array::Write_To_File (const char* /*file_name*/,
				   Am_Image_File_Format /*form*/)
{
  if (data)
    // do something useful
    return 0;
  else return 0;
}
// end of NDY stuff.

// Returns in storage the RGB values for each pixel in the image
// using 1 byte each for R, for G and for B.  So ...
// storage must be large enough to accomodate 3*width*height bytes.
// If top_first return RGB values for top row first
// else return RGB for bottom row first.
// Returns true if successful (i.e. implemented).
bool Am_Image_Array::Get_RGB_Image(unsigned char * storage, bool top_first)
{
  if (data)
    return data->Get_RGB_Image(storage, top_first);
  else
    return false;
}

// Defaults:
//   i_left = 0, i_top = 0 (unused)
//   ls = Am_No_Style, fs = Am_No_Style
//   f = Am_DRAW_COPY
// if width is -1, then it uses width/height in the image object
void Am_Drawonable_Impl::Draw_Image(int left, int top, int width, int height,
				    const Am_Image_Array& image,
				    int i_left, int i_top,
				    const Am_Style& ls, // color of 'on' bits
				    const Am_Style& fs, // background behind image
				    bool draw_monochrome,// use ls and fs in place of colors
				    Am_Draw_Function f)
{
  int i_width, i_height;
  
  Am_Image_Array_Data *image_data = Am_Image_Array_Data::Narrow(image);
  if(!image_data)
    return;

  if (width < 0)
    {
      Get_Image_Size(image, i_width, i_height);
    }
  else
    {
      i_width = width;
      i_height = height;
    }
  const int depth = image_data->Get_Depth();
  if (depth > 1)
    {
      if(f == Am_DRAW_MASK_COPY)
	{
	  if(fs.Valid())
	    {
	      set_gc_using_fill_style(fs, f);
	      gwtDrawBox(gwt_window, left, top, i_width, i_height);
	    }
	  else
	    {
	      // TODO: If we have a mask in image_data: OBTAIN IT USE IT
	      set_gc_using_fill_style(ls, f /* mask here? */);
	      gwtDrawBox(gwt_window, left, top, i_width, i_height);
	    }
	}
      else
	{
	  if (draw_monochrome)
	    {
	      if(ls.Valid())
		{
		  if(fs.Valid())
		    {
		      set_gc_using_fill_style(fs, f);	
		      gwtDrawBox(gwt_window, left, top, i_width, i_height);
		    }
		  // TODO: USE the mask !!
		  set_gc_using_fill_style(ls, f /* mask here? */);
		  gwtDrawBox(gwt_window, left, top, i_width, i_height);
		}
	      else if (fs.Valid())
		{
		  // foreground transparent; background not transparent
		  // use inverted mask TODO: USE that mask!
		  set_gc_using_fill_style(fs, f /* mask here? */);
		  gwtDrawBox(gwt_window, left, top, i_width, i_height);
		}
	    }
	  else if(image_data->Is_Transparent())
	    {
	      if(fs.Valid())
		{  // use fs to fill background
		  set_gc_using_fill_style(fs, f);
		  gwtDrawBox(gwt_window, left, top, i_width, i_height);
		}
	      // TODO: Obtain and use mask
	      set_gc_using_fill_style(Am_On_Bits, Am_DRAW_GRAPHIC_OR /* mask here? */);
	      gwtDrawBox(gwt_window, left, top, i_width, i_height);

	      set_gc_using_fill_style(Am_No_Style, Am_DRAW_GRAPHIC_AND);
	     std::cerr << "GWT_GEM: Draw_Image not implemented!" <<std::endl;
	      // XCopyArea(screen->display, pixmap, xlib_drawable, screen->gc, 0, 0, i_width, i_height, left, top);
	    }
	  else
	    {  // opaque
	      set_gc_using_fill_style(Am_No_Style, f);
	     std::cerr << "GWT_GEM: Draw_Image not implemented!" <<std::endl;
	      // XCopyArea(screen->display, pixmap, xlib_drawable, screen->gc, 0, 0, i_width, i_height, left, top);
	    }
    	}
    }
  else
    {  // bitmap (depth == 1), might be transparent.
      // The idea of transparent and opaque bitmaps is implemented here, but it
      // is not through the fill-flag of any style object.  The way to make a
      // bitmap transparent is to just not pass any fill-style.  If a fill-style
      // is passed, the bitmap will be opaque (i.e., the 'off' bits will be
      if(fs.Valid())
	{
	  set_gc_using_fill_style(fs, f);
	  gwtDrawBox(gwt_window, left, top, i_width, i_height);
	}
      if(ls.Valid())
	{
	  // TODO: Use the image pixmap here as mask for the line style app. */
	  set_gc_using_fill_style(ls, f /* pixmap here? */);
	  gwtDrawBox(gwt_window, left, top, i_width, i_height);
	}
    }
}

void Am_Drawonable_Impl::Get_Image_Size (const Am_Image_Array& image,
					 int& ret_width, int& ret_height)
{
  Am_Image_Array_Data* data = (Am_Image_Array_Data::Narrow(image));
  if (data)
    {
      // Make sure the image has been created for this display
      // TODO: Study these size problems    cf:data->Get_X_Pixmap (this);
      image.Get_Size(ret_width, ret_height);
      data->Release ();
    }
}

// Verifies that bitmap has a correct filename and loaded correctly.
bool Am_Drawonable_Impl::Test_Image (const Am_Image_Array& image) const
{
  Am_Image_Array_Data* data = (Am_Image_Array_Data::Narrow(image));

  if (data) {
    // Make sure the image has been created for this display
    // TODO: Study that also. cf:Pixmap bitmap = data->Safe_Get_X_Pixmap (this);
    data->Release ();
    return true;
  }
  return false;
}

// This function is not guaranteed to return the expected width and height
// of the bitmap, since it may not have been created on the display yet.
// You should call Am_Drawonable::Get_Image_Size() instead.
// 6/24/97 [eab] changed to force image_ initialization when possible
//	   (so it is possible to Get_RGB_Image without displaying it)
void Am_Image_Array::Get_Size (int& ret_width, int& ret_height) const
{
  ret_width = ret_height = 0;
  if (data) {
    if (data->generic_image == (0L)) {
      if (! data->make_generic_image_from_name())
	return;
    }
    short unsigned int w, h; // using temps explicitly gets rid of a warning
    data->generic_image->Get_Size(w, h);
    ret_width = w;
    ret_height = h;
  }
}

////////
// Return platform independent image in RGB form
///////

// assumes that RGB entries in color map are each 1 byte
bool Am_Image_Array_Data::Get_RGB_Image(unsigned char * storage,
					bool top_first)
{
  if (generic_image == (0L))
    {
      if (! make_generic_image_from_name())
	return false;
    }

  unsigned char * idata = generic_image->Get_Data();
  if (idata == (0L)) return false;

  unsigned short width, height;
  generic_image->Get_Size(width, height);

  if (name)
    {
      // image from file; with colormap
      Am_RGB_Value * ctbl;
      int row, col, index;
      
      (void) generic_image->Get_Color_Map(ctbl);
      index = 0;
      
      if (top_first)
	{
	  // start with row 0 and work "down"
	  for (row = 0; row < height; row++)
	    {
	      for (col = 0; col < width; col++)
		{
		  int pixel = idata[row*width + col];
		  storage[index++] = ctbl[pixel].red;
		  storage[index++] = ctbl[pixel].green;
		  storage[index++] = ctbl[pixel].blue;
		}
	    }
	}
      else
	{
	  // start with last row and work "up"
	  for (row = height - 1; row >= 0; row--)
	    {
	      for (col = 0; col < width; col++)
		{
		  int pixel = idata[row*width + col];
		  storage[index++] = ctbl[pixel].red;
		  storage[index++] = ctbl[pixel].green;
		  storage[index++] = ctbl[pixel].blue;
		}
	    }
	}
    }
  else
    {
      // image from bit data
      // this is how I think X stores bitmaps:
      // - the number of bits in a row is the width of the image is
      //   rounded up to an even multiple of 8
      // - the bits in a byte are stored from right to left !
      int row, col, index;
      static unsigned char byte_array [] = {
	0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
      int row_width = (width + 7)/8 * 8; // round to nearest byte;

      index = 0;
      if (top_first)
	{
	  // start with row 0 and work "down"
	  for (row = 0; row < height; row++)
	    {
	      for (col = 0; col < width; col++)
		{
		  int position = row * row_width + col;
		  unsigned char pixel = idata[position/8] & byte_array[position%8];
		  if (pixel)
		    {
		      storage[index++] = 0xFF;
		      storage[index++] = 0xFF;
		      storage[index++] = 0xFF;
		    }
		  else
		    {
		      storage[index++] = 0;
		      storage[index++] = 0;
		      storage[index++] = 0;
		    }
		}
	    }
	}
      else
	{
	  // start with last row and work "up"
	  for (row = height - 1; row >= 0; row--)
	    {
	      for (col = 0; col < width; col++)
		{
		  int position = row * row_width + col;
		  unsigned char pixel = idata[position/8] & byte_array[position%8];
		  if (pixel)
		    {
		      storage[index++] = 0xFF;
		      storage[index++] = 0xFF;
		      storage[index++] = 0xFF;
		    }
		  else
		    {
		      storage[index++] = 0;
		      storage[index++] = 0;
		      storage[index++] = 0;
		    }
		}
	    }
	}
    }
  return true;
}

/////
// Am_Image_Array_Data setters/getters
////

bool Am_Image_Array_Data::is_gif()
{
  if (name)
    {
      const int len = strlen(name);
      if (!strcmp(&(name[len - 4]), ".gif"))
	return true;
    }
  return false;
}

bool Am_Image_Array_Data::make_generic_image_from_name()
{
  // currently generic images are only stored for gifs
  if (is_gif())
    {
      if (generic_image == (0L))
	generic_image = Am_GIF_Image::Create(name);
      // TODO: It seems to me we should return true here, no ?
      else
	return true;
    }
  return false;
}
