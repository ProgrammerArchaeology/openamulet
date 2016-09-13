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

#include <string.h>

#include <am_inc.h>

#include <amulet/initializer.h>
#include <amulet/impl/types_string.h>
#include <amulet/impl/am_font.h>
#include <amulet/impl/opal_impl.h>
#include <amulet/impl/opal_op.h>
#include <amulet/formula.h>

void Am_text_draw_internal(Am_Object self, Am_Drawonable *drawonable,
                           int x_offset, int y_offset, const char *str,
                           bool mask = false);

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

Am_Object Am_Hidden_Text; //for passwords

// get the x position for the cursor in pixels, for hidden text that
// uses the same character for all characters
Am_Define_Formula(int, compute_cursor_offset_for_hidden)
{
  int cursor_index = self.Get(Am_CURSOR_INDEX);
  if (cursor_index >= 0) {
    Am_Drawonable *drawonable = GV_a_drawonable(self);
    Am_Font font(self.Get(Am_FONT));
    Am_String str(self.Get(Am_TEXT));
    char replacement = self.Get(Am_START_CHAR);
    int max_cursor_index = strlen(str);
    cursor_index = imin(imax(0, cursor_index), max_cursor_index);
    int char_width = drawonable->Get_Char_Width(font, replacement);
    int new_offset = cursor_index * (char_width + 1);
    if (cursor_index > 0 && cursor_index == max_cursor_index)
      new_offset--; //make sure < width
    return new_offset;
  } else
    return 0;
}

Am_Define_Method(Am_Draw_Method, void, hidden_text_draw,
                 (Am_Object self, Am_Drawonable *drawonable, int x_offset,
                  int y_offset))
{
  Am_Value v;
  Am_String str;
  v = self.Peek(Am_TEXT);
  if (!v.Valid())
    return; //if not a valid string, don't draw anything
  str = v;
  int str_len = strlen(str);
  char new_str[250];
  if (str_len > 249)
    str_len = 249;
  char replacement = self.Get(Am_START_CHAR);
  for (int i = 0; i < str_len; i++) {
    new_str[i] = replacement;
  }
  new_str[str_len] = 0;
  Am_text_draw_internal(self, drawonable, x_offset, y_offset, new_str);
}

static void
init()
{
  Am_Hidden_Text = Am_Text.Create(DSTR("Am_Hidden_Text"))
                       .Add(Am_START_CHAR, '*')
                       .Set(Am_CURSOR_OFFSET, compute_cursor_offset_for_hidden)
                       .Set(Am_DRAW_METHOD, hidden_text_draw);

  Am_Object_Advanced temp = (Am_Object_Advanced &)Am_Hidden_Text;
  temp.Get_Slot(Am_START_CHAR)
      .Set_Demon_Bits(Am_MOVING_REDRAW | Am_EAGER_DEMON);
}

//
// This initializer must run AFTER Am_Text's
//
static Am_Initializer *initializer =
    new Am_Initializer(DSTR("Am_Hidden_Text"), init, 2.2f);
