 /* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file contains low-level objects to support cursors */

#include <X11/Xlib.h>

#include <cstdio>
#include <iostream>
#include <am_inc.h>

#include GDEFS__H
#include GEM__H
#include GEMX__H

// // // // // // // // // // // // // // // // // // // //
// Am_Cursor
// // // // // // // // // // // // // // // // // // // //

AM_WRAPPER_IMPL (Am_Cursor)

/////
// Am_Cursor constructors.
/////

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
void Am_Cursor::Set_Hot_Spot(int x, int y) {
  data->x_hot = x;
  data->y_hot = y;
}

void Am_Cursor::Get_Hot_Spot(int &x, int &y) const {
  x = data->x_hot;
  y = data->y_hot;
}

void Am_Cursor::Get_Size(int &width, int &height)
{
  data->image.Get_Size(width, height);
}

Am_Cursor Am_Default_Cursor;

/* functions for Am_Cursor_Data */
AM_WRAPPER_DATA_IMPL (Am_Cursor, (this))

Am_Cursor_Data* Am_Cursor_Data::list = (0L);

Am_Cursor_Data::Am_Cursor_Data (Am_Cursor_Data *proto) 
{
  main_display = proto->main_display;
  main_cursor = proto->main_cursor;
  image = proto->image;
  mask = proto->mask;
  fg_color = proto->fg_color;
  bg_color = proto->bg_color;

  // should I copy the whole list over?
  head = proto->head;
  next = list;
  list = this;
}

Am_Cursor_Data::Am_Cursor_Data()
{
  main_display = (0L);
  main_cursor = 0;
  head = (0L);
  next = list;
  list = this;
}
  
// takes in the image for the cursor, the mask, and the 2 colors and
//   sets the right data to them.  Delays making actual cursor till
//   Get_X_Cursor is called.

Am_Cursor_Data::Am_Cursor_Data(Am_Image_Array im, Am_Image_Array m,
			       Am_Style fg_col, Am_Style bg_col)
{
  main_display = (0L);
  main_cursor = 0;
  image = im;
  mask = m;
  fg_color = fg_col;
  bg_color = bg_col;
  head = (0L);
  next = list;
  list = this;
  x_hot = y_hot = -1;
}

Am_Cursor_Data::~Am_Cursor_Data()
{
  // needs to delete the linked list of cursor_items
  Cursor_Item *cl;
  cl = head;
  if (cl)
    while (cl->next) {
      delete head;
      head = cl->next;
      cl->next = cl->next->next;
    }
  delete head;
  remove (this);
}

void Am_Cursor_Data::remove (Am_Cursor_Data* cursor)
{
  Am_Cursor_Data* prev = (0L);
  Am_Cursor_Data* curr = list;
  while (curr) {
    if (curr == cursor) {
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

void Am_Cursor_Data::remove (Display* display)
{
  Am_Cursor_Data* curr;
  for (curr = list; curr; curr = curr->next) {
    if (curr->main_display == display)
      curr->main_display = (0L);
    Cursor_Item* prev = (0L);
    Cursor_Item* curr_index = curr->head;
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

/* takes the data we have laying around for the cursor and 
   gets the X specific stuff for it so we can use it now */

Cursor Am_Cursor_Data::Get_X_Cursor(Am_Drawonable_Impl *draw) 
{
  // get the pointers to the images and the colors
  if (! image.Valid()) { return 0; }

  Am_Image_Array_Data* im = Am_Image_Array_Data::Narrow (image);
  Am_Image_Array_Data* im_mask = Am_Image_Array_Data::Narrow (mask);
  Am_Style_Data* fg_data = Am_Style_Data::Narrow(fg_color);
  Am_Style_Data* bg_data = Am_Style_Data::Narrow(bg_color);

  if (!(im && im_mask && fg_data && bg_data))
    {
      std::cerr << "Error in making cursor " << std::endl;
      Am_Error ();
      return 0;
    }

  // now actually get the images and the colors
  XColor fg_col = fg_data->Get_X_Color(draw);
  XColor bg_col = bg_data->Get_X_Color(draw);
  im->Get_X_Pixmap(draw);
  im_mask->Get_X_Pixmap(draw);
  if ((im->Get_Depth() > 1) || (im_mask->Get_Depth() > 1))
    Am_Error("** Tried to make a cursor out of a pixmap with depth > 1\n");
  
  if (fg_data) fg_data->Release ();
  if (bg_data) bg_data->Release ();
  
  // get the hot spot
  int used_x_hot, used_y_hot;
  im->Get_Hot_Spot(used_x_hot, used_y_hot);

  if ((used_x_hot == -1) || (used_y_hot == -1))
    {
      used_x_hot = x_hot;
      used_y_hot = y_hot;
    }

  if ((used_x_hot == -1) || (used_y_hot == -1))
    {
      std::cerr << "Cursor has no hot spot" <<std::endl;
    }
  else
    {
      // make a cursor
      Cursor cursor = XCreatePixmapCursor(draw->screen->display, im->main_bitmap,
					  im_mask->main_bitmap, &fg_col, 
					  &bg_col, used_x_hot, used_y_hot);
      if (cursor == BadMatch)
	{
	  std::cerr << "Bad Match error in making cursor " <<std::endl;
          Am_Error ();
	}
      else if (cursor == BadAlloc)
	{
	  std::cerr << "Bad Alloc error in making cursor " <<std::endl;
          Am_Error ();
	}
      else if (cursor == BadPixmap)
	{
	  std::cerr << "Bad Pixmap error in making cursor " <<std::endl;
          Am_Error ();
	}
      main_cursor = cursor;
    }

  if (im) im->Release ();
  if (im_mask) im_mask->Release ();
  
  return main_cursor;
}

/* adds the cursor to the linked list of cursor_items */
void Am_Cursor_Data::Add_Cursor (Display *display, Cursor cursor)
{
  Cursor_Item *new_node = new Cursor_Item (display, cursor);

  new_node->next = head;
  head = new_node;
}

/* gets a cursor item for use */
bool Am_Cursor_Data::Get_Cursor (Display *display, Cursor &cursor) 
{
  Cursor_Item *current;

  for (current = head; current != (0L); current = current->next)
    if (current->display == display) {
      cursor = current->cursor;
      return true;
    }
  return false;
}
