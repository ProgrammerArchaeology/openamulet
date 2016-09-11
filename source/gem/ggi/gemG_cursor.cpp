 /* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file contains low-level objects to support cursors */

#include <am_inc.h>
#include GDEFS__H // Am_Cursor declarations
#include GEMG__H  // Am_Cursor_Data declarations

//
// Global variables
//
Am_Cursor Am_Default_Cursor;

// // // // // // // // // // // // // // // // // // // //
// Am_Cursor
// // // // // // // // // // // // // // // // // // // //

//
// Am_Cursor
//

Am_WRAPPER_IMPL (Am_Cursor)

Am_Cursor::Am_Cursor()
{
  data = new Am_Cursor_Data();
}

Am_Cursor::Am_Cursor(Am_Image_Array image, Am_Image_Array mask, 
		     Am_Style fg_color, Am_Style bg_color)
{
  data = new Am_Cursor_Data(image, mask, fg_color, bg_color);
}

// normal functions for Am_Cursor
void Am_Cursor::Set_Hot_Spot(int x, int y)
{
  data->x_hot = x;
  data->y_hot = y;
}

void Am_Cursor::Get_Hot_Spot(int &x, int &y) const
{
  x = data->x_hot;
  y = data->y_hot;
}

void Am_Cursor::Get_Size(int &width, int &height)
{
  data->image.Get_Size(width, height);
}

//
// Am_Cursor_Data
//

Am_WRAPPER_DATA_IMPL (Am_Cursor, (this));
