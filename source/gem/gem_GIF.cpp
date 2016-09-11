 /* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

// Implementation of machine independant GIF loader.
#include <fstream>
#include <iostream>
#include <string.h>

#include "amulet/gem_GIF.h"

// Load a GIF color map from a stream into an image.
// Return 1/0 success value
int Am_GIF_Image::load_gif_colormap(std::ifstream &ifs, int flags)
{
  //  iminfo.depth = CTSIZE(flags) + 1; set in caller
  //  iminfo.num_colors = 1 << iminfo.depth;
  iminfo.num_colors = 1 << (CTSIZE(flags) + 1);
  // if we have a color map, delete it before reallocating
  if (color_map != (0L)) delete[] color_map;
  color_map = new Am_RGB_Value[iminfo.num_colors];
  if(color_map == (0L)) return 0; // failure
  int i;
  unsigned char buf[3];
  for (i = 0; i < iminfo.num_colors; i++) {
    // ifs.read(buf, 3);  MSL
    ifs.read((char*) buf, 3);
    color_map[i].red = buf[0];
    color_map[i].green = buf[1];
    color_map[i].blue = buf[2];
  }
  return 1;
}

// for transparent images, force 0 to be the index of the transparent
// color, and color_map[0] to be white (all 1's)
// so transparent images work under X
void Am_GIF_Image::adjust_colormap()
{
  if (color_map != (0L)) {
    if (iminfo.raw_transparent_color_index != 0)
      color_map[iminfo.raw_transparent_color_index] = color_map[0];

    color_map[0] = Am_RGB_Value(0xFF,0xFF,0xFF);
  }
}

// Load a gif file into an image structure.
Am_Generic_Image *Am_GIF_Image::Create(const char* filename)
{
  Am_GIF_Image *im = new Am_GIF_Image;
  if (im == (0L)) return NULL; // couldn't allocate the temp_image.
  GIF_Load_Info gli;

  // Buffer to hold a block of image data from the GIF file.
  unsigned char buf[256];

  // Open file for reading
  std::ifstream ifs(filename, std::ios::in);

  if(!ifs) {
    delete im;
    return (0L);
  }

  // all Am_GIF_Image images have depth 8
  im->iminfo.depth = 8;

  // Read GIF header
  // ifs.read((unsigned char*)&gli.hdr, GIF_SIG_LEN); MSL
  ifs.read((char*) &gli.hdr, GIF_SIG_LEN);

  // Check if image file format is acceptable
  if(strncmp(gli.hdr.signature, "GIF", 3) != 0 &&
     gli.hdr.version[0] != '8') {
    delete im;
    return (0L);
  }

  // Read the logical screen descriptor.  We only use the flags field.
  // ifs.read(buf, GIF_SD_SIZE); MSL
  ifs.read((char*) buf, GIF_SD_SIZE);
  int flag = buf[4];

  // If a global color table is present, read it.
  if(flag & HAS_CT) {
    if (!(im->load_gif_colormap(ifs, flag))) {
      delete im;
      return (0L);
    }
  }

  // Next comes a series of graphics blocks or special-purpose
  // blocks
  int byte = 0, read_an_image = 0;
  while(!read_an_image && (byte = ifs.get()) != GIF_TRAILER) {
      // Locate an image descriptor and read the image data
      int c;
      switch(byte) {
	case GIF_EXTSTART: {
	  c = ifs.get();
	  if(c == GIF_COMMENT) {
	  } else if (c == GIF_GCE) {
	    c = ifs.get(); // BlockSize always 4
	    // ifs.read(buf, GIF_GCE_SIZE); MSL
	    ifs.read((char*) buf, GIF_GCE_SIZE);
	    flag = buf[GIF_GCE_PACKED];
	    if (flag & TRANSPARENT) {
	      im->iminfo.transparent = true;
	      im->iminfo.raw_transparent_color_index = buf[GIF_GCE_COLORINDEX];
	    }
	  } else {
	    int blksize = ifs.get();
	    // ifs.read(buf, blksize); MSL
	    ifs.read((char*) buf, blksize);
	  }
	  while((c = ifs.get()) != 0)
	    //ifs.read(buf, c); MSL
	    ifs.read((char*) buf, c);
	  break;
	}
	case GIF_IMGSTART: {
	  // Read GIF image information
	  // ifs.read(buf, GIF_ID_SIZE); MSL
	  ifs.read((char*) buf, GIF_ID_SIZE);
	  im->iminfo.width = (buf[5] << 8) + buf[4];
	  im->iminfo.height = (buf[7] << 8) + buf[6];
	  flag = buf[8];

	  gli.interlaced = (flag & INTERLACED) ? 1 : 0;

	  // Allocate memory for the GIF image and read in GIF image
	  // into this array
	  im->imdata = new unsigned char[(long)(im->iminfo.width)
					*(long)(im->iminfo.height + 10)];
	  if(im->imdata == (0L)) { // failed to allocate image data
	    delete im;
	    return (0L);
	  }

	  // Read local color table, if any
	  if(flag & HAS_CT) {
	    if (!(im->load_gif_colormap(ifs, flag))) {
	      delete im;
	      return (0L);
	    }
	  }

	  // force index of transparent color to be 0
	  // to support X stipple raster ops
	  if (im->iminfo.transparent) {
	    im->adjust_colormap();
	  }

	  // Process the encoded data stream from the file.
	  if (!(im->process_gif(ifs, gli))) {
	   std::cerr << "** error reading gif file.\n";
	    delete im;
	    return (0L);
	  };
	  read_an_image = 1;
	  break;
	}
	default: {
	 std::cerr << "** error reading gif file.\n";
	  delete im;
	  return (0L);
	}
      }
    }
  // Success!
  return im;
}

// process_gif
// Processes a LZW data stream from a GIF file. Decoded data is
// placed in "image" array.
// return 0 for failure, 1 for success.

int Am_GIF_Image::process_gif(std::ifstream &ifs, GIF_Load_Info& gli)
{
  // Buffer to hold a block of image data from the GIF file.
  unsigned char buf[256];
  unsigned short data_mask = iminfo.num_colors - 1;

  // Originally the following three 4KB buffers were stack-allocated.
  // but gcc -fPIC (version 2.7.0, HP/UX 9.x) produced code for it that
  // barfed the HP assembler.  So we'll heap-allocate the buffers instead.
  unsigned short *prefix = new unsigned short[GIF_TBLSIZE];
  unsigned short *suffix = new unsigned short[GIF_TBLSIZE];
  unsigned short *stack = new unsigned short[GIF_TBLSIZE];

  unsigned short istk = 0;

  gli.x = 0; gli.y = 0; // initial values

  // Next byte is the initial number of bits used for LZW codes
  unsigned short init_code_size = ifs.get();

  if(init_code_size < 2 || init_code_size > 8)
    {
      delete [] prefix;
      delete [] suffix;
      delete [] stack;
      return 0;
    }

  unsigned short clear_code = 1 << init_code_size;
  unsigned short eoi_code = clear_code + 1;
  unsigned short free_code = clear_code + 2;

  // The GIF specification says that, for decompression, the code
  // size should be one bit longer than the initial code size.
  unsigned short code_size = init_code_size + 1;

  // Bit mask needed to extract data bits corresponding to the
  // code size.
  unsigned short max_code = 1 << code_size;
  unsigned short read_mask = max_code - 1;

  ifs.clear();

  unsigned short bitpos = 0;
  unsigned long val = 0;

  for(unsigned short count = ifs.get(); count > 0; count = ifs.get())
    {
      // Read a block of data into the buffer
      // ifs.read(buf, count); MSL
      ifs.read((char*) buf, count);
      if (ifs.fail())
	{
	  delete [] prefix;
	  delete [] suffix;
	  delete [] stack;
	  return 0;
	}

      // Process the contents of the buffer
      for(unsigned int i = 0; i < count; i++)
	{
	  const unsigned long tmp = buf[i];
	  val |= (tmp << bitpos);
	  bitpos += 8;

	  while(bitpos >= code_size)
	    {
	      unsigned short curcode = (unsigned short)(val & read_mask);
	      val >>= code_size;
	      bitpos -= code_size;
	      if (curcode == eoi_code)
		{
		  delete [] prefix;
		  delete [] suffix;
		  delete [] stack;
		  return 1;
		}
	      unsigned short oldcode;
	      unsigned short pixel;
	      if (curcode == clear_code)
		{
		  free_code = (1 << init_code_size) + 2;
		  code_size = init_code_size + 1;
		  max_code = 1 << code_size;
		  read_mask = max_code - 1;
		  istk = 0;
		  oldcode = 0xffff;
		}
	      else
		{
		  if (oldcode == 0xffff)
		    {
		      // First code, after initialization, is a raw pixel value
		      pixel = curcode & data_mask;
		      add_to_image(gli, pixel);
		      oldcode = curcode;
		    }
		  else
		    {
		      const unsigned short input_code = curcode;
		      if (curcode >= free_code)
			{
			  // Code is not in table yet. Save last character in stack
			  stack[istk++] = (unsigned char)pixel;
			  curcode = oldcode;
			}
		      while(curcode > data_mask) 
			{
			  // [eab] I don't know enough about the LZW algorithm
			  // to introduce more precise range tests than the following
			  // on curcode and istk
			  if (curcode >= GIF_TBLSIZE || istk >= GIF_TBLSIZE - 2)
			    {
			      delete [] prefix;
			      delete [] suffix;
			      delete [] stack;
			      return 0; // corrupt GIF
			    }
			  // Save bytes corresponding to compression code in stack for
			  // later use.
			  stack[istk++] = suffix[curcode];
			  curcode = prefix[curcode];
			}
		  
		      pixel = curcode & data_mask;
		      stack[istk++] = (unsigned char)pixel;
		  
		      // Add decoded string of bytes to image
		      for (short j = istk - 1; j >= 0; j--)
			add_to_image(gli, stack[j]);

		      istk = 0;

		      // Add current information to decompression tables
		      prefix[free_code] = oldcode;
		      suffix[free_code] = (unsigned char)pixel;
		      oldcode = input_code;

		      // Adjust code size, if necessary.
		      free_code++;
		      if (free_code >= max_code)
			{
			  if (code_size < GIF_MAXCODE)
			    {
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
  delete [] prefix;
  delete [] suffix;
  delete [] stack;
  return 1;
}

// Add a pixel to the image

void Am_GIF_Image::add_to_image(GIF_Load_Info &gli,
				unsigned short pixval)
{
  // force index of transparent color to be 0
  unsigned char pixchar = (unsigned char) pixval;
  if (iminfo.raw_transparent_color_index != 0) {
    if (pixval == 0) pixchar = (unsigned char) iminfo.raw_transparent_color_index;
    else if (pixval == iminfo.raw_transparent_color_index) pixchar = 0;
  }

  imdata[gli.x + iminfo.width * gli.y] = pixchar;

  // Adjust the coordinates
  gli.x++;
  if(gli.x == iminfo.width) {
    gli.x = 0;
    if(!gli.interlaced) gli.y++;
    else {
      switch(gli.pass)
	{
	case 1:  // Every 8th row, start at row 0
	  gli.y += 8;
	  if(gli.y >= iminfo.height) {
	    gli.y = 4;
	    gli.pass++;
	  }
	  break;

	case 2:  // Every 8th row, start at row 4
	  gli.y += 8;
	  if(gli.y >= iminfo.height) {
	    gli.y = 2;
	    gli.pass++;
	  }
	  break;

	case 3:  // Every 4th row, start at row 2
	  gli.y += 4;
	  if(gli.y >= iminfo.height) {
	    gli.y = 1;
	    gli.pass++;
	  }
	  break;

	case 4:  // Every 2nd row, start at row 1
	  gli.y += 2;
	  break;
	}
    }
  }
}
