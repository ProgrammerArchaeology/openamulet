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

#include <am_inc.h>

#include INITIALIZER__H
#include <amulet/impl/types_string.h>
#include <amulet/impl/am_font.h>
#include <amulet/impl/opal_impl.h>
#include <amulet/impl/opal_misc.h>
#include <amulet/impl/opal_op.h>
#include FORMULA__H

inline int
imin(int i1, int i2)
{
  if (i1 < i2)
    return i1;
  else
    return i2;
}
inline int
imax(int i1, int i2)
{
  if (i1 > i2)
    return i1;
  else
    return i2;
}

Am_Object Am_Text;

/***************************************
 ** Functions to support drawing text **
 ***************************************/

Am_Define_Formula(int, compute_string_width)
{
  Am_Font font(self.Get(Am_FONT));
  Am_String string(self.Get(Am_TEXT));

  if (string.Valid()) {
    Am_Object screen = Am_GV_Screen(self);
    int str_len = strlen(string);
    int width = Am_Get_String_Width(screen, font, string, str_len);

    // assure text boxes are always at least 10 pixels wide so you can click
    return width > 10 ? width : 10;
  } else
    return 10;
}

Am_Define_Formula(int, compute_string_height)
{
  Am_Font font(self.Get(Am_FONT));
  Am_Object screen = Am_GV_Screen(self);

  int max_width, min_width, ascent, descent;
  Am_Get_Font_Properties(screen, font, max_width, min_width, ascent, descent);
  return (ascent + descent);
}

// get the x position for the cursor in pixels
Am_Define_Formula(int, compute_cursor_offset)
{
  int cursor_index = self.Get(Am_CURSOR_INDEX);
  if (cursor_index >= 0) {
    Am_Drawonable *drawonable = GV_a_drawonable(self);
    Am_Font font(self.Get(Am_FONT));
    Am_String str(self.Get(Am_TEXT));
    int max_cursor_index = strlen(str);
    cursor_index = imin(imax(0, cursor_index), max_cursor_index);
    int new_offset = drawonable->Get_String_Width(font, str, cursor_index);
    if (cursor_index > 0 && cursor_index == max_cursor_index)
      new_offset--; //make sure < width
    return new_offset;
  } else
    return 0;
}

//make sure the cursor is visible inside the object by scrolling string l or r
Am_Define_Formula(int, compute_string_offset)
{
  Am_String str;
  str = self.Get(Am_TEXT);
  if (!str.Valid())
    return 0; // no string in there, so offset 0.

  int cursor_index = self.Get(Am_CURSOR_INDEX);
  // get old value of this slot
  int old_string_offset;
  Am_Value old_string_offset_value;
  old_string_offset_value = self.Peek(Am_X_OFFSET, Am_NO_DEPENDENCY);
  if (!old_string_offset_value.Exists())
    old_string_offset = 0;
  else
    old_string_offset = old_string_offset_value;

  int new_offset = old_string_offset;
  int width = self.Get(Am_WIDTH);
  Am_Drawonable *drawonable = GV_a_drawonable(self);
  Am_Font font(self.Get(Am_FONT));

  int max_cursor_index = strlen(str);
  int full_string_width =
      drawonable->Get_String_Width(font, str, max_cursor_index);
  if (old_string_offset > 0 && full_string_width - old_string_offset < width) {
    //then need to scroll because width changed
    new_offset = imax(0, full_string_width - width);
  } else if (cursor_index >= 0) { //otherwise, don't change position
    Am_Drawonable *drawonable = GV_a_drawonable(self);
    int cursor_offset = self.Get(Am_CURSOR_OFFSET);
    if (cursor_offset < old_string_offset) { // then scroll right
      if (cursor_offset == 0)
        new_offset = 0;
      else {
        // find the position of the character at the cursor
        cursor_index = imin(imax(0, cursor_index), max_cursor_index);
        new_offset = drawonable->Get_String_Width(font, str, cursor_index);
      }
    } else {
      if (cursor_offset > old_string_offset + width) { // scroll left
        // find the position of the character at the cursor
        cursor_index = imin(imax(0, cursor_index), max_cursor_index);
        new_offset = drawonable->Get_String_Width(font, str, cursor_index);
        new_offset = new_offset - width;
        if (new_offset < 0)
          new_offset = 0;
      }
    }
  }
  return new_offset;
}

void
Am_text_draw_internal(Am_Object self, Am_Drawonable *drawonable, int x_offset,
                      int y_offset, const char *str, bool mask = false)

{
  int left = (int)self.Get(Am_LEFT) + x_offset;
  int top = (int)self.Get(Am_TOP) + y_offset;
  int width = self.Get(Am_WIDTH);
  int height = self.Get(Am_HEIGHT);
  int my_x_offset = 0;
  Am_Value v;
  v = self.Peek(Am_X_OFFSET);
  if (v.Valid())
    my_x_offset = v; //might be invalid if no string

  Am_Style ls = self.Get(Am_LINE_STYLE);
  Am_Style fs = self.Get(Am_FILL_STYLE);
  Am_Font font = self.Get(Am_FONT);
  int str_len = strlen(str);
  bool invert = self.Get(Am_INVERT);

  // set a clip region in case string is bigger than text box
  drawonable->Push_Clip(left, top, width, height);
  drawonable->Draw_Text(ls, str, str_len, font, left - my_x_offset, top,
                        mask ? Am_DRAW_MASK_COPY : Am_DRAW_COPY, fs, invert);
  if (((int)(self.Get(Am_CURSOR_INDEX)) >= 0)) {
    int cursor_offset = self.Get(Am_CURSOR_OFFSET);
    int cursor_left = left + cursor_offset - my_x_offset;
    // cursor_height can't just be Am_HEIGHT in case user resized box.
    int ascent, descent, garbage;
    drawonable->Get_Font_Properties(font, garbage, garbage, ascent, descent);
    int cursor_bottom = ascent + descent + top;
    drawonable->Draw_Line(ls, cursor_left, top, cursor_left, cursor_bottom,
                          mask ? Am_DRAW_MASK_COPY : Am_DRAW_COPY);
  }
  drawonable->Pop_Clip();
}

Am_Define_Method(Am_Draw_Method, void, text_draw,
                 (Am_Object self, Am_Drawonable *drawonable, int x_offset,
                  int y_offset))
{
  Am_Value v;
  Am_String str;
  v = self.Peek(Am_TEXT);
  if (!v.Valid())
    return; //if not a valid string, don't draw anything
  str = v;
  Am_text_draw_internal(self, drawonable, x_offset, y_offset, str);
}

Am_Define_Method(Am_Draw_Method, void, text_mask,
                 (Am_Object self, Am_Drawonable *drawonable, int x_offset,
                  int y_offset))
{
  Am_Value v;
  Am_String str;
  v = self.Peek(Am_TEXT);
  if (!v.Valid())
    return; //if not a valid string, don't draw anything
  str = v;
  Am_text_draw_internal(self, drawonable, x_offset, y_offset, str, true);
}

static void
init()
{
  Am_Text =
      Am_Graphical_Object.Create(DSTR("Am_Text"))
          .Add(Am_FONT, Am_Default_Font)
          .Add(Am_TEXT, "")
          //width and height can be set bigger or smaller than calculated
          // will scroll if smaller.
          .Set(Am_WIDTH, compute_string_width)
          .Set(Am_HEIGHT, compute_string_height)
          .Add(Am_CURSOR_INDEX, Am_NO_CURSOR)
          // X_offset allows the string to be scrolled left and right.  The
          // default formula makes sure the cursor index is visible.
          .Add(Am_X_OFFSET, 0) //initial value is 0
          .Set(Am_X_OFFSET, compute_string_offset)
          .Add(Am_CURSOR_OFFSET, compute_cursor_offset)
          .Add(Am_LINE_STYLE, Am_Line_2)
          .Add(Am_FILL_STYLE, Am_No_Style)
          .Add(Am_INVERT,
               false) // invert foreground/ background styles of text?
          .Add(Am_PENDING_DELETE, false)
          .Set(Am_DRAW_METHOD, text_draw)
          .Set(Am_MASK_METHOD, text_mask);

  Am_Object_Advanced temp = (Am_Object_Advanced &)Am_Text;
  temp.Get_Slot(Am_TEXT).Set_Demon_Bits(Am_MOVING_REDRAW | Am_EAGER_DEMON);
  temp.Get_Slot(Am_FONT).Set_Demon_Bits(Am_MOVING_REDRAW | Am_EAGER_DEMON);
  temp.Get_Slot(Am_CURSOR_INDEX)
      .Set_Demon_Bits(Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  temp.Get_Slot(Am_LINE_STYLE)
      .Set_Demon_Bits(Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  temp.Get_Slot(Am_FILL_STYLE)
      .Set_Demon_Bits(Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  temp.Get_Slot(Am_INVERT).Set_Demon_Bits(Am_STATIONARY_REDRAW |
                                          Am_EAGER_DEMON);
}

static void
cleanup()
{
  Am_Text.Destroy();
}

static Am_Initializer *initializer =
    new Am_Initializer(DSTR("Am_Text"), init, 2.1f, 122, cleanup);
