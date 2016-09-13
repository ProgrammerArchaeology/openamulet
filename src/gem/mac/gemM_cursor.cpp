/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file contains low-level objects to support Mac cursors */

#include <stdio.h>
#include <iostream>

#include <am_inc.h>

#include <amulet/gdefs.h>
#include <amulet/gem.h>
#include <gemM.h>

// // // // // // // // // // // // // // // // // // // //
// Am_Cursor
// // // // // // // // // // // // // // // // // // // //

AM_WRAPPER_IMPL(Am_Cursor)

/////
// Am_Cursor constructors.
/////

Am_Cursor::Am_Cursor() { data = new Am_Cursor_Data(); }

Am_Cursor::Am_Cursor(Am_Image_Array image, Am_Image_Array mask,
                     Am_Style fg_color, Am_Style bg_color)
{
  data = new Am_Cursor_Data(image, mask, fg_color, bg_color);
}

// normal functions for Am_Cursor
void
Am_Cursor::Set_Hot_Spot(int x, int y)
{
  if (data) {
    data = Am_Cursor_Data::Narrow(data->Make_Unique());

    data->x_hot = x;
    data->y_hot = y;
  }
}

void
Am_Cursor::Get_Hot_Spot(int &x, int &y) const
{
  if (data) {
    x = data->x_hot;
    y = data->y_hot;
  }
}

void
Am_Cursor::Get_Size(int &width, int &height)
{
  data->image.Get_Size(width, height);
}

Am_Cursor Am_Default_Cursor;

/* functions for Am_Cursor_Data */

AM_WRAPPER_DATA_IMPL(Am_Cursor, (this))

Am_Cursor_Data::Am_Cursor_Data(Am_Cursor_Data *proto)
{
  cursor = proto->cursor;
  image = proto->image;
  mask = proto->mask;
  fg_color = proto->fg_color;
  bg_color = proto->bg_color;
  inited = proto->inited;
}

Am_Cursor_Data::Am_Cursor_Data()
{
  image = (0L);
  mask = (0L);
  fg_color = (0L);
  bg_color = (0L);
  inited = false;
}

/* takes in the image for the cursor, the mask, and the 2 colors and
   sets the right data to them.  Delays making actual cursor till
   Get_Mac_Cursor is called. */

Am_Cursor_Data::Am_Cursor_Data(Am_Image_Array inImage, Am_Image_Array inMask,
                               Am_Style inFGColor, Am_Style inBGColor)
{
  image = inImage;
  mask = inMask;
  fg_color = inFGColor;
  bg_color = inBGColor;
  inited = false;
}

Am_Cursor_Data::~Am_Cursor_Data() {}

/* takes the data we have laying around for the cursor and
   gets the Mac specific stuff for it so we can use it now */

Cursor *
Am_Cursor_Data::Get_Mac_Cursor(Am_Drawonable_Impl *draw)
{

  if (!image.Valid()) {
    return 0;
  }

  if (!inited) {
    memset(cursor.data, 0, sizeof(cursor.data));
    memset(cursor.mask, 0, sizeof(cursor.mask));

    // Get image

    Am_Image_Array_Data *image_data = Am_Image_Array_Data::Narrow(image);

    if (image_data) {
      PixMapHandle image_pixmap = image_data->Get_Mac_PixMap(draw);
      image_data->Release();

      if (image_pixmap && (**image_pixmap).pixelSize == 1) {
        // Copy data into cursor
        if (LockPixels(image_pixmap)) {
          Ptr p = (**image_pixmap).baseAddr;
          short rowBytes = (**image_pixmap).rowBytes & 0x7FFF;
          for (int i = 0; i < 16; i++) {
            memcpy(&cursor.data[i], p, 2);
            p += rowBytes;
          }
        }
        UnlockPixels(image_pixmap);
      }
    }

    // Get mask

    Am_Image_Array_Data *mask_data = Am_Image_Array_Data::Narrow(mask);
    if (mask_data) {
      PixMapHandle mask_pixmap = mask_data->Get_Mac_PixMap(draw);
      mask_data->Release();

      if (mask_pixmap && (**mask_pixmap).pixelSize == 1) {
        // Copy data into cursor mask
        if (LockPixels(mask_pixmap)) {
          Ptr p = (**mask_pixmap).baseAddr;
          short rowBytes = (**mask_pixmap).rowBytes & 0x7FFF;
          for (int i = 0; i < 16; i++) {
            memcpy(&cursor.mask[i], p, 2);
            p += rowBytes;
          }
        }
        UnlockPixels(mask_pixmap);
      }
    }

    // Get hot spot
    cursor.hotSpot.h = x_hot;
    cursor.hotSpot.v = y_hot;
  }
  return &cursor;
}
