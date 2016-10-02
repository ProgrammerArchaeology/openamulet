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

#include <X11/Xlib.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <am_inc.h>

#include <amulet/gdefs.h>
#include <amulet/gem.h>
#include <amulet/gemX.h>
#include "amulet/gem_GIF.h"

// // // // // // // // // // // // // // // // // // // // // // // // // //
// bitmap data for standard stipples
// // // // // // // // // // // // // // // // // // // // // // // // // //

static char stipples[][4] = {{0x00, 0x00, 0x00, 0x00}, {0x01, 0x00, 0x00, 0x00},
                             {0x01, 0x00, 0x04, 0x00}, {0x05, 0x00, 0x04, 0x00},
                             {0x05, 0x00, 0x05, 0x00}, {0x05, 0x02, 0x05, 0x00},
                             {0x05, 0x02, 0x05, 0x08}, {0x05, 0x0a, 0x05, 0x08},
                             {0x05, 0x0a, 0x05, 0x0a}, {0x05, 0x0a, 0x05, 0x0e},
                             {0x05, 0x0b, 0x05, 0x0e}, {0x05, 0x0b, 0x05, 0x0f},
                             {0x05, 0x0f, 0x05, 0x0f}, {0x05, 0x0f, 0x0d, 0x0f},
                             {0x07, 0x0f, 0x0b, 0x0f}, {0x07, 0x0f, 0x0f, 0x0f},
                             {0x0f, 0x0f, 0x0f, 0x0f}};

// // // // // // // // // // // // // // // // // // // //
// Am_Image_Array
// // // // // // // // // // // // // // // // // // // //

AM_WRAPPER_IMPL(Am_Image_Array)

/////
// Am_Image_Array constructors.
/////

Am_Image_Array::Am_Image_Array() { data = nullptr; }

Am_Image_Array::Am_Image_Array(const char *file_name)
{
  data = new Am_Image_Array_Data(file_name);
}

Am_Image_Array::Am_Image_Array(unsigned int width, unsigned int height,
                               int depth,
                               // default color 0
                               Am_Style /*initial_color*/)
{
  data = new Am_Image_Array_Data();

  unsigned char *temp = new unsigned char[width * height];
  data->image_ = new Am_Generic_Image(temp, width, height, depth);

  for (unsigned int i = 0; i < (height * width); i++)
    temp[i] = 0;
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
  data = new Am_Image_Array_Data(stipples[nbits], 4, 4);
}

Am_Image_Array::Am_Image_Array(char *bit_data, int height, int width)
{
  data = new Am_Image_Array_Data(bit_data, height, width);
}

Am_Image_Array Am_No_Image;

////
// Am_Image_Array_Data constructors
////

AM_WRAPPER_DATA_IMPL(Am_Image_Array, (this))

Am_Image_Array_Data *Am_Image_Array_Data::list = nullptr;

Am_Image_Array_Data::Am_Image_Array_Data(const char *file_name)
{
  x_hot = -1;
  y_hot = -1;
  main_bitmap = 0;
  main_mask = 0;
  main_inverted_mask = 0;
  colors = nullptr;
  num_colors = 0;
  image_ = nullptr;
  name = new char[strlen(file_name) + 1];
  strcpy(name, file_name);
  main_display = nullptr;
  head = nullptr;
  next = list;
  list = this;
}

Am_Image_Array_Data::Am_Image_Array_Data(const char *bit_data, int h, int w)

{
  x_hot = -1;
  y_hot = -1;
  main_bitmap = 0;
  main_mask = 0;
  main_inverted_mask = 0;
  colors = nullptr;
  num_colors = 0;
  int nbytes = (((w + 7) / 8) * h);
  unsigned char *temp = new unsigned char[nbytes];
  memcpy(temp, bit_data, nbytes);
  image_ = new Am_Generic_Image((unsigned char *)temp, w, h, 1);
  name = nullptr;
  main_display = nullptr;
  head = nullptr;
  next = list;
  list = this;
}

Am_Image_Array_Data::Am_Image_Array_Data()
{
  x_hot = -1;
  y_hot = -1;
  colors = nullptr;
  num_colors = 0;
  main_bitmap = 0;
  main_mask = 0;
  main_inverted_mask = 0;
  image_ = nullptr;
  name = nullptr;
  main_display = nullptr;
  head = nullptr;
  next = list;
  list = this;
}

Am_Image_Array_Data::Am_Image_Array_Data(Am_Image_Array_Data *proto)
{
  // should I copy or use pointers to cursor, main_bitmap, etc?
  cursor = proto->cursor;
  main_bitmap = proto->main_bitmap;
  main_mask = proto->main_mask;
  main_inverted_mask = proto->main_inverted_mask;
  x_hot = proto->x_hot;
  y_hot = proto->y_hot;
  image_ = new Am_Generic_Image(proto->image_);
  name = new char[strlen(proto->name) + 1];
  strcpy(name, proto->name);
  main_display = proto->main_display;
  // BUGGY: should copy entire bitmap list!
  // (broken for multiple displays)
  head = proto->head;
  next = list;
  list = this;
  // Should copy color list as well
  colors = proto->colors;
  num_colors = proto->num_colors;
}

/////
// Am_Image_Array_Data destructor
/////

Am_Image_Array_Data::~Am_Image_Array_Data()
{
  //   /*  if (image)
  //     {
  //       /*
  //       if (image->data) delete[] image->data;
  //       image->data = nullptr;
  //       delete image;
  //       */
  //       XDestroyImage(image);
  //       image = nullptr;
  //     }*/
  if (name)
    delete[] name;
  if (image_) {
    delete image_;
    image_ = nullptr;
  }
  if (colors) {
    int screen_num = DefaultScreen(main_display);
    Colormap c = XDefaultColormap(main_display, screen_num);
    for (int i = 0; i < num_colors; i++)
      XFreeColors(main_display, c, &(colors[i].pixel), 1, 0);
    delete[] colors;
    colors = nullptr;
  }
  if (main_display && main_bitmap) {
    XFreePixmap(main_display, main_bitmap);
    if (main_mask)
      XFreePixmap(main_display, main_mask);
    if (main_inverted_mask)
      XFreePixmap(main_display, main_inverted_mask);
  }
  Bitmap_Item *current = head;
  Bitmap_Item *next = nullptr;
  while (current) {
    next = current->next;
    current->next = nullptr;
    delete current; // destructor takes care of freeing colors, pixmap, etc.
    current = next;
  }
  head = nullptr;
  remove(this);
}

void
Am_Image_Array_Data::remove(Am_Image_Array_Data *image)
{
  Am_Image_Array_Data *prev = nullptr;
  Am_Image_Array_Data *curr = list;
  while (curr) {
    if (curr == image) {
      if (prev)
        prev->next = curr->next;
      else
        list = curr->next;
      return;
    }
    prev = curr;
    curr = curr->next;
  }
}

void
Am_Image_Array_Data::remove(Display *display)
{
  Am_Image_Array_Data *curr;
  for (curr = list; curr; curr = curr->next) {
    if (curr->main_display == display) {
      if (curr->colors) {
        int screen_num = DefaultScreen(curr->main_display);
        Colormap c = XDefaultColormap(curr->main_display, screen_num);
        for (int i = 0; i < curr->num_colors; i++)
          XFreeColors(curr->main_display, c, &(curr->colors[i].pixel), 1, 0);
        delete[] curr->colors;
        curr->colors = nullptr;
      }
      if (curr->main_display && curr->main_bitmap)
        XFreePixmap(curr->main_display, curr->main_bitmap);
      curr->main_display = nullptr;
    }
    Bitmap_Item *prev = nullptr;
    Bitmap_Item *curr_index = curr->head;
    while (curr_index) {
      if (curr_index->display == display) {
        if (prev)
          prev->next = curr_index->next;
        else
          curr->head = curr_index->next;
        delete curr_index;
        break;
      }
      prev = curr_index;
      curr_index = curr_index->next;
    }
  }
}

/////
// Am_Image_Array setters and getters
/////

// NDY: implementations of getbit, setbit, write_to_file
int
Am_Image_Array::Get_Bit(int /*x*/, int /*y*/)
{
  if (data)
    return 0; // return something useful
  else
    return 0;
}
void
Am_Image_Array::Set_Bit(int /*x*/, int /*y*/, int /*val*/)
{
  if (data)
    data = Am_Image_Array_Data::Narrow(data->Make_Unique());
  // then do something useful
}
int
Am_Image_Array::Write_To_File(const char * /*file_name*/,
                              Am_Image_File_Format /*form*/)
{
  if (data)
    // do something useful
    return 0;
  else
    return 0;
}
// end of NDY stuff.

// Returns in storage the RGB values for each pixel in the image
// using 1 byte each for R, for G and for B.  So ...
// storage must be large enough to accommodate 3*width*height bytes.
// If top_first return RGB values for top row first
// else return RGB for bottom row first.
// Returns true if successful (i.e. implemented).
bool
Am_Image_Array::Get_RGB_Image(unsigned char *storage, bool top_first)
{
  if (data) {
    return data->Get_RGB_Image(storage, top_first);
  }
  return false;
}

// Defaults:
//   i_left = 0, i_top = 0 (unused)
//   ls = Am_No_Style, fs = Am_No_Style
//   f = Am_DRAW_COPY
// if width is -1, then it uses width/height in the image object
void
Am_Drawonable_Impl::Draw_Image(
    int left, int top, int width, int height, const Am_Image_Array &image,
    int i_left, int i_top,
    const Am_Style &ls,   // color of 'on' bits
    const Am_Style &fs,   // background behind image
    bool draw_monochrome, // use ls and fs in place of colors
    Am_Draw_Function f)
{
  int i_width, i_height;

  Am_Image_Array_Data *image_data = Am_Image_Array_Data::Narrow(image);
  if (!image_data) {
    return;
  }

  Pixmap pixmap = image_data->Get_X_Pixmap(this);

  if (width < 0) {
    Get_Image_Size(image, i_width, i_height);
  } else {
    i_width = width;
    i_height = height;
  }

  int depth = image_data->Get_Depth();
  if (depth > 1) {
    if (f == Am_DRAW_MASK_COPY) {
      if (fs.Valid()) {
        set_gc_using_fill(fs, f);
        XFillRectangle(screen->display, xlib_drawable, screen->gc, left, top,
                       i_width, i_height);
      } else {
        Pixmap mask = image_data->Get_X_Mask(this);
        set_gc_using_fill(ls, f, mask);
        XSetTSOrigin(screen->display, screen->gc, left, top);
        XFillRectangle(screen->display, xlib_drawable, screen->gc, left, top,
                       i_width, i_height);
      }
    } else {
      if (draw_monochrome) {
        if (ls.Valid()) {
          Pixmap mask = image_data->Get_X_Mask(this);

          XSetTSOrigin(screen->display, screen->gc, left, top);
          if (fs.Valid()) {
            set_gc_using_fill(fs, f);
            XFillRectangle(screen->display, xlib_drawable, screen->gc, left,
                           top, i_width, i_height);
          }

          set_gc_using_fill(ls, f, mask);
          XFillRectangle(screen->display, xlib_drawable, screen->gc, left, top,
                         i_width, i_height);
        } else if (fs.Valid()) {
          // foreground transparent; background not transparent
          // use inverted mask
          Pixmap mask = image_data->Get_X_Mask(this, true);

          XSetTSOrigin(screen->display, screen->gc, left, top);
          set_gc_using_fill(fs, f, mask);
          XFillRectangle(screen->display, xlib_drawable, screen->gc, left, top,
                         i_width, i_height);
        }
      } else if (image_data->Is_Transparent()) {
        Pixmap mask = image_data->Get_X_Mask(this);
        XSetTSOrigin(screen->display, screen->gc, left, top);

        if (fs.Valid()) { // use fs to fill background
          set_gc_using_fill(fs, f);
          XFillRectangle(screen->display, xlib_drawable, screen->gc, left, top,
                         i_width, i_height);
        }

        set_gc_using_fill(Am_On_Bits, Am_DRAW_GRAPHIC_OR, mask);
        XSetTSOrigin(screen->display, screen->gc, left, top);
        XFillRectangle(screen->display, xlib_drawable, screen->gc, left, top,
                       i_width, i_height);

        set_gc_using_fill(Am_No_Style, Am_DRAW_GRAPHIC_AND);
        XCopyArea(screen->display, pixmap, xlib_drawable, screen->gc, 0, 0,
                  i_width, i_height, left, top);
      } else { // opaque
        set_gc_using_fill(Am_No_Style, f);
        XCopyArea(screen->display, pixmap, xlib_drawable, screen->gc, 0, 0,
                  i_width, i_height, left, top);
      }
    }
  } else { // bitmap (depth == 1), might be transparent.
    // The idea of transparent and opaque bitmaps is implemented here, but it
    // is not through the fill-flag of any style object.  The way to make a
    // bitmap transparent is to just not pass any fill-style.  If a fill-style
    // is passed, the bitmap will be opaque (i.e., the 'off' bits will be
    // drawn with the color of the fill-style).
    // 4-18-97 eab
    // I have left old behavior in place for bitmaps
    // draw_monochrome requires masking that I haven't been able to get
    // working for bitmaps (xbm files, nameless images)
    // I may be mistaken, but it appears that some nameless images of
    // depth 1 may be stored one pixel per byte and others one pixel
    // per bit!!!
    if (fs.Valid()) {
      set_gc_using_fill(fs, f);
      XFillRectangle(screen->display, xlib_drawable, screen->gc, left, top,
                     i_width, i_height);
    }

    if (ls.Valid()) {
      set_gc_using_fill(ls, f, pixmap);
      XSetTSOrigin(screen->display, screen->gc, left, top);
      XFillRectangle(screen->display, xlib_drawable, screen->gc, left, top,
                     i_width, i_height);
    }
  }
}

void
Am_Drawonable_Impl::Get_Image_Size(const Am_Image_Array &image, int &ret_width,
                                   int &ret_height)
{
  Am_Image_Array_Data *data = (Am_Image_Array_Data::Narrow(image));

  if (data) {
    // Make sure the image has been created for this display
    data->Get_X_Pixmap(this);
    image.Get_Size(ret_width, ret_height);
    data->Release();
  }
}

// Verifies that bitmap has a correct filename and loaded correctly.
bool
Am_Drawonable_Impl::Test_Image(const Am_Image_Array &image) const
{
  Am_Image_Array_Data *data = (Am_Image_Array_Data::Narrow(image));

  if (data) {
    // Make sure the image has been created for this display
    Pixmap bitmap = data->Safe_Get_X_Pixmap(this);
    data->Release();
    return (bitmap != 0);
  }
  return false;
}

// This function is not guaranteed to return the expected width and height
// of the bitmap, since it may not have been created on the display yet.
// You should call Am_Drawonable::Get_Image_Size() instead.
// 6/24/97 [eab] changed to force image_ initialization when possible
//	   (so it is possible to Get_RGB_Image without displaying it)
void
Am_Image_Array::Get_Size(int &ret_width, int &ret_height) const
{
  ret_width = ret_height = 0;
  if (data) {
    if (data->image_ == nullptr) {
      if (!data->make_generic_image_from_name())
        return;
    }
    short unsigned int w, h; // using temps explicitly gets rid of a warning
    data->image_->Get_Size(w, h);
    ret_width = w;
    ret_height = h;
  }
}

/*
void Am_Image_Array::Get_Hot_Spot (int& x, int& y) const
{
  if (data) {
    x = data->x_hot;
    y = data->y_hot;
  }
}

void Am_Image_Array::Set_Hot_Spot (int x, int y)
{
  if (data) {
    data = Am_Image_Array_Data::Narrow(data->Make_Unique());

    data->x_hot = x;
    data->y_hot = y;
  }
}
*/

////////
// Return platform independent image in RGB form
///////

// assumes that RGB entries in color map are each 1 byte
bool
Am_Image_Array_Data::Get_RGB_Image(unsigned char *storage, bool top_first)
{
  if (image_ == nullptr) {
    if (!make_generic_image_from_name())
      return false;
  }

  unsigned char *idata = image_->Get_Data();
  if (idata == nullptr)
    return false;

  unsigned short width, height;
  image_->Get_Size(width, height);

  if (name) {
    // image from file; with colormap
    Am_RGB_Value *ctbl;
    int row, col, index;

    (void)image_->Get_Color_Map(ctbl);
    index = 0;

    if (top_first) {
      // start with row 0 and work "down"
      for (row = 0; row < height; row++) {
        for (col = 0; col < width; col++) {
          int pixel = idata[row * width + col];
          storage[index++] = ctbl[pixel].red;
          storage[index++] = ctbl[pixel].green;
          storage[index++] = ctbl[pixel].blue;
        }
      }
    } else {
      // start with last row and work "up"
      for (row = height - 1; row >= 0; row--) {
        for (col = 0; col < width; col++) {
          int pixel = idata[row * width + col];
          storage[index++] = ctbl[pixel].red;
          storage[index++] = ctbl[pixel].green;
          storage[index++] = ctbl[pixel].blue;
        }
      }
    }
  } else {
    // image from bit data
    // this is how I think X stores bitmaps:
    // - the number of bits in a row is the width of the image is
    //   rounded up to an even multiple of 8
    // - the bits in a byte are stored from right to left !
    int row, col, index;
    static unsigned char byte_array[] = {0x01, 0x02, 0x04, 0x08,
                                         0x10, 0x20, 0x40, 0x80};
    int row_width = (width + 7) / 8 * 8; // round to nearest byte;

    index = 0;
    if (top_first) {
      // start with row 0 and work "down"
      for (row = 0; row < height; row++) {
        for (col = 0; col < width; col++) {
          int position = row * row_width + col;
          unsigned char pixel = idata[position / 8] & byte_array[position % 8];
          if (pixel) {
            storage[index++] = 0xFF;
            storage[index++] = 0xFF;
            storage[index++] = 0xFF;
          } else {
            storage[index++] = 0;
            storage[index++] = 0;
            storage[index++] = 0;
          }
        }
      }
    } else {
      // start with last row and work "up"
      for (row = height - 1; row >= 0; row--) {
        for (col = 0; col < width; col++) {
          int position = row * row_width + col;
          unsigned char pixel = idata[position / 8] & byte_array[position % 8];
          if (pixel) {
            storage[index++] = 0xFF;
            storage[index++] = 0xFF;
            storage[index++] = 0xFF;
          } else {
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

/////////////
// Utility function
/////////////

Pixmap
Am_Image_Array_Data::Make_Pixmap_From_Generic_Image(
    const Am_Drawonable_Impl *draw, XColor *&cols, int &n_cols)
{
  // Make a copy of the image data, and adjust colors in the image
  // to the ones we have in the X colormap.
  Am_RGB_Value *ctbl;
  n_cols = image_->Get_Color_Map(ctbl);
  cols = new XColor[n_cols];

  int i;
  // first, allocate all the closest colors.  Could be optimized a lot!
  for (i = 0; i < n_cols; i++) {
    cols[i].red = ctbl[i].red * 256;
    cols[i].green = ctbl[i].green * 256;
    cols[i].blue = ctbl[i].blue * 256;
    draw->Allocate_Closest_Color(cols[i]);
  }

  // Next, set the pixels in the image correctly.
  unsigned short width, height;
  image_->Get_Size(width, height);
  unsigned char *gif_image = image_->Get_Data();

  int depth = draw->screen->depth;
  Visual *visual =
      DefaultVisual(draw->screen->display, draw->screen->screen_number);

  unsigned char *x_image;

  // assume that pixels are packed into even byte boundaries
  if (depth == 1) {
    static unsigned char byte_array[] = {0x80, 0x40, 0x20, 0x10,
                                         0x08, 0x04, 0x02, 0x01};
    // X uses free() so we must use malloc.  (so says sentinel)
    int byte_width = (width + 7) / 8;
    x_image = (unsigned char *)malloc(byte_width * height);
    int i, j;
    for (i = 0; i < height; i++) {
      for (j = 0; j < width; j++) {
        if (cols[gif_image[i * width + j]].pixel)
          x_image[i * byte_width + (j / 8)] |= byte_array[j % 8];
        else
          x_image[i * byte_width + (j / 8)] &= ~byte_array[j % 8];
      }
    }
  } else if (depth <= 8) {
    // X uses free() so we must use malloc.  (so says sentinel)
    x_image = (unsigned char *)malloc(width * height);
    for (i = 0; i < (width * height); i++)
      x_image[i] = (unsigned char)(cols[gif_image[i]].pixel);
  } else if (depth <= 16) {
    // X uses free() so we must use malloc.  (so says sentinel)
    x_image = (unsigned char *)malloc(width * height * 2);
    unsigned short *x_image_16 = (unsigned short *)x_image;
    for (i = 0; i < (width * height); i++)
      x_image_16[i] = (unsigned short)(cols[gif_image[i]].pixel);
  } else {
    // X uses free() so we must use malloc.  (so says sentinel)
    x_image = (unsigned char *)malloc(width * height * 4);
    uint32_t *x_image_32 = (uint32_t*)x_image;
    for (i = 0; i < (width * height); i++)
      x_image_32[i] = (uint32_t)(cols[gif_image[i]].pixel);
  }

  XImage *image = XCreateImage(draw->screen->display, visual, depth, ZPixmap,
                               0 /* offset */, (char *)x_image, width, height,
                               8 /* bitmap_pad */,
                               0 /* (X will calculate) bytes per line */);

  Pixmap pix = XCreatePixmap(draw->screen->display, draw->xlib_drawable,
                             image->width, image->height, image->depth);
  draw->set_gc_using_fill(Am_No_Style, Am_DRAW_COPY);
  // Clear the clip region; restore it later.
  XSetClipMask(draw->screen->display, draw->screen->gc, None);
  XPutImage(draw->screen->display, pix, draw->screen->gc, image, 0, 0, 0, 0,
            image->width, image->height);
  ((Am_Drawonable_Impl *)draw)->Set_Clip(draw->screen->clip_region);

  XDestroyImage(image);

  return pix;
}

Pixmap
Am_Image_Array_Data::Make_Mask_From_Generic_Image(
    const Am_Drawonable_Impl *draw, bool invert)
{
  static unsigned char byte_array[] = {0x01, 0x02, 0x04, 0x08,
                                       0x10, 0x20, 0x40, 0x80};

  // Next, set the pixels in the image correctly.
  unsigned short width, height;
  image_->Get_Size(width, height);
  unsigned char *gif_image = image_->Get_Data();

  // X uses free() so we must use malloc.  (so says sentinel)
  unsigned short byte_width = width / 8;
  if ((width % 8) != 0)
    byte_width++;

  unsigned char *data = new unsigned char[byte_width * height];

  // use transparent color for holes in the mask (zero default)
  int transparent_index = 0; // image_->Get_Transparent_Index();

  int i, j;
  for (i = 0; i < height; i++) {
    for (j = 0; j < width; j++) {
      if (gif_image[i * width + j] == (unsigned char)(transparent_index)) {
        if (invert)
          data[i * byte_width + (j / 8)] |= byte_array[j % 8];
        else
          data[i * byte_width + (j / 8)] &= ~byte_array[j % 8];
      } else {
        if (invert)
          data[i * byte_width + (j / 8)] &= ~byte_array[j % 8];
        else
          data[i * byte_width + (j / 8)] |= byte_array[j % 8];
      }
    }
  }

  Pixmap retval = XCreateBitmapFromData(
      draw->screen->display, draw->xlib_drawable, (char *)data, width, height);
  delete[] data;

  return retval;
}

/////
// Am_Image_Array_Data setters/getters
////

bool
Am_Image_Array_Data::is_gif()
{
  if (name) {
    int len = strlen(name);
    if (!strcmp(&(name[len - 4]), ".gif"))
      return true;
  }
  return false;
}

bool
Am_Image_Array_Data::make_generic_image_from_name()
{
  // currently generic images are only stored for gifs
  if (is_gif()) {
    if (image_ == nullptr)
      image_ = Am_GIF_Image::Create(name);
    if (image_ != nullptr)
      return true;
  }
  return false;
}

Pixmap
Am_Image_Array_Data::Get_X_Pixmap(const Am_Drawonable_Impl *draw)
{
  Display *disp = draw->screen->display;
  if (disp == main_display)
    return main_bitmap;

  Pixmap bitmap;

  if (Get_Bitmap(disp, bitmap))
    return bitmap;

  XColor *cols = nullptr;
  int n_cols = 0;

  /* then we need to read it in or make it */
  if (name) {
    unsigned int w, h;

    /* read image from a file */
    if (is_gif()) {
      if (!make_generic_image_from_name())
        return 0;
      bitmap = Make_Pixmap_From_Generic_Image(draw, cols, n_cols);
    } else { // it's an xbm
      int tmp = XReadBitmapFile(disp, draw->xlib_drawable, name, &w, &h,
                                &bitmap, &x_hot, &y_hot);
      // fill an empty structure to store width, height in.
      image_ = new Am_Generic_Image(nullptr, w, h, 1);
      if (tmp == BitmapOpenFailed) {
        std::cerr << "** Bitmap Error: " << name << " is not a valid file name"
                  << std::endl;
        Am_Error();
      }
      if (tmp == BitmapFileInvalid) {
        std::cerr << "** Bitmap Error: " << name
                  << " does not contain valid bitmap data" << std::endl;
        Am_Error();
      }
      if (tmp == BitmapNoMemory) {
        std::cerr
            << "** Bitmap Error: Out of memory error when reading in bitmap "
               "from "
            << name << std::endl;
        Am_Error();
      }
      if (tmp == BadAlloc)
        Am_Error("** Bitmap Error: Bad Alloc error");
      if (tmp == BadDrawable)
        Am_Error("** Bitmap Error: Bad Drawable error");
      if (tmp != BitmapSuccess) {
        std::cerr << "** Error in reading in bitmap from file " << name
                  << std::endl;
        Am_Error();
      }
    } // else bmp format
  } else {
    // then we need to make it from data
    // Assume that it's in XYBitmap format in the generic image item.
    unsigned short int width, height;
    image_->Get_Size(width, height);
    bitmap = XCreateBitmapFromData(disp, draw->xlib_drawable,
                                   (char *)image_->Get_Data(), width, height);

    if (bitmap == BadAlloc) {
      std::cerr << "Bad Alloc error" << std::endl;
      Am_Error();
    }
    if (bitmap == BadDrawable) {
      std::cerr << "Bad Drawable error" << std::endl;
      Am_Error();
    }
  }

  if (main_display)
    Add_Bitmap(disp, bitmap, cols, n_cols);
  else {
    main_display = disp;
    main_bitmap = bitmap;
    colors = cols;
    num_colors = n_cols;
  }
  return bitmap;
}

Pixmap
Am_Image_Array_Data::Safe_Get_X_Pixmap(const Am_Drawonable_Impl *draw)
{
  Display *disp = draw->screen->display;
  if (disp == main_display)
    return main_bitmap;

  Pixmap bitmap;

  if (Get_Bitmap(disp, bitmap))
    return bitmap;

  XColor *cols = nullptr;
  int n_cols = 0;

  /* then we need to read it in or make it */
  if (name) {
    unsigned int w, h;

    /* read image rom a file */

    if (is_gif()) {
      if (!make_generic_image_from_name())
        return 0;
      bitmap = Make_Pixmap_From_Generic_Image(draw, cols, n_cols);
    } else { // it's an xbm
      int tmp = XReadBitmapFile(disp, draw->xlib_drawable, name, &w, &h,
                                &bitmap, &x_hot, &y_hot);
      // fill an empty structure to store width, height in.
      image_ = new Am_Generic_Image(nullptr, w, h, 1);
      if (tmp != BitmapSuccess)
        return 0;
    } // else bmp format
  } else {
    // then we need to make it from data
    // Assume that it's in XYBitmap format in the generic image item.
    unsigned short int width, height;
    image_->Get_Size(width, height);
    bitmap = XCreateBitmapFromData(disp, draw->xlib_drawable,
                                   (char *)image_->Get_Data(), width, height);

    if (bitmap == BadAlloc) {
      std::cerr << "Bad Alloc error" << std::endl;
      Am_Error();
    }
    if (bitmap == BadDrawable) {
      std::cerr << "Bad Drawable error" << std::endl;
      Am_Error();
    }
  }

  if (main_display)
    Add_Bitmap(disp, bitmap, cols, n_cols);
  else {
    main_display = disp;
    main_bitmap = bitmap;
    colors = cols;
    num_colors = n_cols;
  }
  return bitmap;
}

Pixmap
Am_Image_Array_Data::Get_X_Mask(const Am_Drawonable_Impl *draw, bool invert)
{
  Display *disp = draw->screen->display;
  Pixmap bitmap;

  if (disp == main_display) {
    if (invert)
      bitmap = main_inverted_mask;
    else
      bitmap = main_mask;
    if (bitmap)
      return bitmap;
  }

  Bitmap_Item *bitmap_item = nullptr;
  Bitmap_Item *current;
  for (current = head; current != nullptr; current = current->next) {
    if (current->display == disp) {
      if (invert)
        bitmap = current->inverted_mask;
      else
        bitmap = current->mask;
      if (bitmap)
        return bitmap;
      bitmap_item = current;
      break;
    }
  }

  // You only have to call Get_X_Mask on non bitmaps in which case the image_
  // member must already be loaded.
  if (!image_)
    return 0; // error

  bitmap = Make_Mask_From_Generic_Image(draw, invert);

  if (bitmap_item) {
    if (invert)
      bitmap_item->inverted_mask = bitmap;
    else
      bitmap_item->mask = bitmap;
  } else {
    if (invert)
      main_inverted_mask = bitmap;
    else
      main_mask = bitmap;
  }
  return bitmap;
}

void
Am_Image_Array_Data::Add_Bitmap(Display *display, Pixmap bitmap, XColor *colors,
                                int n_colors)
{
  Bitmap_Item *new_node = new Bitmap_Item(display, bitmap, colors, n_colors);
  new_node->next = head;
  head = new_node;
}

bool
Am_Image_Array_Data::Get_Bitmap(Display *display, Pixmap &bitmap)
{
  Bitmap_Item *current;
  for (current = head; current != nullptr; current = current->next)
    if (current->display == display) {
      bitmap = current->bitmap;
      return true;
    }
  return false;
}
