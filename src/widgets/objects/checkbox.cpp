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

#include <amulet/impl/opal_obj2.h>

Am_Object Am_Checkbox;

/******************************************************************************
 * checkbox_draw
 */

Am_Define_Method(Am_Draw_Method, void, checkbox_draw,
                 (Am_Object self, Am_Drawonable *drawonable, int x_offset,
                  int y_offset))
{
  int left = (int)self.Get(Am_LEFT) + x_offset;
  int top = (int)self.Get(Am_TOP) + y_offset;
  int width = self.Get(Am_WIDTH);
  int height = self.Get(Am_HEIGHT);
  int box_width = self.Get(Am_BOX_WIDTH);
  int box_height = self.Get(Am_BOX_HEIGHT);
  bool selected = self.Get(Am_SELECTED).Valid();
  bool interim_selected = self.Get(Am_INTERIM_SELECTED);
  bool active = self.Get(Am_ACTIVE);
  bool key_selected = self.Get(Am_KEY_SELECTED);
  bool want_final_selected = self.Get(Am_FINAL_FEEDBACK_WANTED);
  bool box_on_left = self.Get(Am_BOX_ON_LEFT);

  Am_Font font(self.Get(Am_FONT));
  Am_Alignment align = self.Get(Am_H_ALIGN);

  Computed_Colors_Record color_rec = self.Get(Am_STYLE_RECORD);
  Am_Widget_Look look = self.Get(Am_WIDGET_LOOK);

  // now find the contents to draw in the button
  Am_String string;
  Am_Object obj = 0;
  Am_Value value;
  // string slot contains a formula which gets the real object based on the
  // value of the COMMAND slot
  value = self.Peek(Am_REAL_STRING_OR_OBJ);
  if (value.type == Am_STRING)
    string = value;
  else if (value.type == Am_OBJECT)
    obj = value;
  else
    Am_ERRORO("String slot of widget "
                  << self << " should have string or object type, but value is "
                  << value,
              self, Am_REAL_STRING_OR_OBJ);

  Am_Draw_Button_Widget(left, top, width, height, string, obj, interim_selected,
                        selected && want_final_selected, active, key_selected,
                        false, false, font, color_rec, look, Am_CHECK_BUTTON,
                        drawonable, box_width, box_height, box_on_left, align,
                        5);
}

Am_Define_Method(Am_Draw_Method, void, checkbox_mask,
                 (Am_Object self, Am_Drawonable *drawonable, int x_offset,
                  int y_offset))
{
  int left = (int)self.Get(Am_LEFT) + x_offset;
  int top = (int)self.Get(Am_TOP) + y_offset;
  int width = self.Get(Am_WIDTH);
  int height = self.Get(Am_HEIGHT);
  int box_width = self.Get(Am_BOX_WIDTH);
  int box_height = self.Get(Am_BOX_HEIGHT);
  bool selected = self.Get(Am_SELECTED).Valid();
  bool interim_selected = self.Get(Am_INTERIM_SELECTED);
  bool active = self.Get(Am_ACTIVE);
  bool key_selected = self.Get(Am_KEY_SELECTED);
  bool want_final_selected = self.Get(Am_FINAL_FEEDBACK_WANTED);
  bool box_on_left = self.Get(Am_BOX_ON_LEFT);

  Am_Font font(self.Get(Am_FONT));
  Am_Alignment align = self.Get(Am_H_ALIGN);

  Computed_Colors_Record color_rec = self.Get(Am_STYLE_RECORD);
  Am_Widget_Look look = self.Get(Am_WIDGET_LOOK);

  // now find the contents to draw in the button
  Am_String string;
  Am_Object obj = 0;
  Am_Value value;
  // string slot contains a formula which gets the real object based on the
  // value of the COMMAND slot
  value = self.Peek(Am_REAL_STRING_OR_OBJ);
  if (value.type == Am_STRING)
    string = value;
  else if (value.type == Am_OBJECT)
    obj = value;
  else
    Am_ERRORO("String slot of widget "
                  << self << " should have string or object type, but value is "
                  << value,
              self, Am_REAL_STRING_OR_OBJ);

  Am_Draw_Button_Widget(left, top, width, height, string, obj, interim_selected,
                        selected && want_final_selected, active, key_selected,
                        false, false, font, color_rec, look, Am_CHECK_BUTTON,
                        drawonable, box_width, box_height, box_on_left, align,
                        5, true);
}

/******************************************************************************
 * checkbox_width
 *   calculates the width of the checkbox
 */

Am_Define_Formula(int, checkbox_width)
{
  Am_String string;
  Am_Object obj;
  Am_Value value;
  // string slot contains a formula which gets the real object based on the
  // value of the COMMAND slot
  int box_width = self.Get(Am_BOX_WIDTH);
  value = self.Peek(Am_REAL_STRING_OR_OBJ);
  if (value.type == Am_STRING)
    string = value;
  else if (value.type == Am_OBJECT)
    obj = value;
  else
    return 20 + box_width;

  int offset = self.Get(Am_ITEM_OFFSET);

  if ((const char *)string) {
    Am_Object window;
    Am_Font font;
    window = self.Get(Am_WINDOW);
    font = self.Get(Am_FONT);
    //GV_a_drawonable will work if not window valid
    Am_Drawonable *draw = GV_a_drawonable(window);
    if (draw) {
      int str_width, ascent, descent, a, b;
      draw->Get_String_Extents(font, string, strlen(string), str_width, ascent,
                               descent, a, b);
      return box_width + str_width + 8 + 2 * offset;
    }
  } else if (obj.Valid()) {
    int obj_width = obj.Get(Am_WIDTH);
    return box_width + obj_width + 8 + 2 * offset;
  }

  return 20 + box_width;
}

/******************************************************************************
 * checkbox_height
 *   calculates the height of the checkbox
 */

Am_Define_Formula(int, checkbox_height)
{
  Am_String string;
  Am_Object obj = 0;
  Am_Value value;
  int box_height = self.Get(Am_BOX_HEIGHT);
  // string slot contains a formula which gets the real object based on the
  // value of the COMMAND slot
  value = self.Peek(Am_REAL_STRING_OR_OBJ);
  if (value.type == Am_STRING) {
    string = value;
    if (strlen(string) == 0) { //don't use zero length string for height
      string = "W";
    }
  } else if (value.type == Am_OBJECT)
    obj = value;
  else
    return box_height + 4;

  int offset = self.Get(Am_ITEM_OFFSET);
  if ((const char *)string) {
    Am_Object window;
    Am_Font font;
    window = self.Get(Am_WINDOW);
    font = self.Get(Am_FONT);
    //GV_a_drawonable will work if window not valid
    Am_Drawonable *draw = GV_a_drawonable(window);
    if (draw) {
      int str_width, ascent, descent, a, b;
      draw->Get_String_Extents(font, string, strlen(string), str_width, ascent,
                               descent, a, b);
      int str_height = ascent + descent;
      return (str_height > box_height ? str_height : box_height) + 4 + offset +
             offset;
    }
  } else if (obj.Valid()) {
    int obj_height = (int)obj.Get(Am_HEIGHT);
    return (obj_height > box_height ? obj_height : box_height) + 4 + offset +
           offset;
  }
  return box_height + 4;
}

/******************************************************************************
 * checkbox_box_width
 *   calculates the width of the checkbox box, which has a dependency upon the
 *   checkbox's look.
 */

Am_Define_Formula(int, checkbox_box_width)
{
  Am_Widget_Look look = self.Get(Am_WIDGET_LOOK);
  int width = 0;

  switch (look.value) {
  case Am_MOTIF_LOOK_val:
    width = 15;
    break;

  case Am_WINDOWS_LOOK_val:
    width = 13;
    break;

  case Am_MACINTOSH_LOOK_val:
    width = 12;
    break;

  default:
    Am_Error("Unknown Look parameter");
    break;
  }

  return width;
}

/******************************************************************************
 * checkbox_box_height
 *   calculates the height of the checkbox box, which has a dependency upon the
 *   checkbox's look.
 */

Am_Define_Formula(int, checkbox_box_height)
{
  Am_Widget_Look look = self.Get(Am_WIDGET_LOOK);
  int height = 0;

  switch (look.value) {
  case Am_MOTIF_LOOK_val:
    height = 15;
    break;

  case Am_WINDOWS_LOOK_val:
    height = 13;
    break;

  case Am_MACINTOSH_LOOK_val:
    height = 12;
    break;

  default:
    Am_Error("Unknown Look parameter");
    break;
  }

  return height;
}

Am_Define_Method(Am_Invalid_Method, void, radio_check_invalid,
                 (Am_Object group, Am_Object which_part, int left, int top,
                  int width, int height))
{
  const Am_Slot_Flags flags = Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR;
  Am_Object owner = group.Get_Owner(flags);
  if (owner.Valid()) {
    int my_left = group.Get(Am_LEFT, flags);
    int my_top = group.Get(Am_TOP, flags);
    int my_width = group.Get(Am_WIDTH, flags);
    int my_height = group.Get(Am_HEIGHT, flags);
    int box_width = group.Get(Am_BOX_WIDTH, flags);
    bool box_on_left = group.Get(Am_BOX_ON_LEFT, flags);
    int offset_left = 5;
    int part_left = which_part.Get(Am_LEFT, flags);
    int part_top = which_part.Get(Am_TOP, flags);
    int part_width = which_part.Get(Am_WIDTH, flags);
    int part_height = which_part.Get(Am_HEIGHT, flags);
    my_width -= box_width + offset_left;
    if (box_on_left)
      my_left += box_width + offset_left;
    int display_left = (my_width - part_width) / 2;
    int display_top = (my_height - part_height) / 2;
    left = left + display_left - part_left;
    top = top + display_top - part_top;
    int final_left, final_top, final_width, final_height;
    // clip incoming rectangle to my rectangle
    Am_Invalid_Rectangle_Intersect(left, top, width, height, my_left, my_top,
                                   my_width, my_height, final_left, final_top,
                                   final_width, final_height);
    if ((final_width > 0) && (final_height > 0))
      Am_Invalidate(owner, group, final_left, final_top, final_width,
                    final_height);
  }
}

Am_Define_Method(Am_Translate_Coordinates_Method, void,
                 radio_check_translate_coordinates,
                 (const Am_Object &obj, const Am_Object &for_part, int in_x,
                  int in_y, int &out_x, int &out_y))
{
  const Am_Slot_Flags flags = Am_RETURN_ZERO_ON_ERROR;
  int my_left = obj.Get(Am_LEFT, flags);
  int my_top = obj.Get(Am_TOP, flags);
  if (for_part.Valid()) {
    int my_width = obj.Get(Am_WIDTH, flags);
    int my_height = obj.Get(Am_HEIGHT, flags);
    int box_width = obj.Get(Am_BOX_WIDTH, flags);
    bool box_on_left = obj.Get(Am_BOX_ON_LEFT, flags);
    int offset_left = 5;
    my_width -= box_width + offset_left;
    if (box_on_left)
      my_left += box_width + offset_left;

    int part_left = for_part.Get(Am_LEFT, flags);
    int part_top = for_part.Get(Am_TOP, flags);
    int part_width = for_part.Get(Am_WIDTH, flags);
    int part_height = for_part.Get(Am_HEIGHT, flags);
    int display_left = (my_width - part_width) / 2;
    int display_top = (my_height - part_height) / 2;
    out_x = my_left + display_left - part_left + in_x;
    out_y = my_top + display_top - part_top + in_y;
  } else {
    out_x = my_left + in_x;
    out_y = my_top + in_y;
  }
}

Am_Define_Method(Am_Point_In_Or_Self_Method, Am_Object,
                 radio_check_point_in_part,
                 (const Am_Object &in_obj, int x, int y,
                  const Am_Object &ref_obj, bool want_self, bool want_groups))
{
  am_translate_coord_to_me(in_obj, ref_obj, x, y);
  if (Am_Point_In_Obj(in_obj, x, y, in_obj).Valid()) {
    Am_Value_List comp = in_obj.Get(Am_GRAPHICAL_PARTS);
    if (!comp.Empty()) {
      Am_Object part = comp.Get_Nth(0);
      const Am_Slot_Flags flags = Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR;
      int my_width = in_obj.Get(Am_WIDTH, flags);
      int my_height = in_obj.Get(Am_HEIGHT, flags);
      int box_width = in_obj.Get(Am_BOX_WIDTH, flags);
      bool box_on_left = in_obj.Get(Am_BOX_ON_LEFT, flags);
      int offset_left = 5;
      my_width -= box_width + offset_left;
      int extra_left = 0;
      if (box_on_left)
        extra_left = box_width + offset_left;

      int part_left = part.Get(Am_LEFT, flags);
      int part_top = part.Get(Am_TOP, flags);
      int part_width = part.Get(Am_WIDTH, flags);
      int part_height = part.Get(Am_HEIGHT, flags);

      int display_left = (my_width - part_width) / 2;
      int display_top = (my_height - part_height) / 2;
      x = x + part_left - display_left - extra_left;
      y = y + part_top - display_top;
      if ((want_groups || !am_is_group_and_not_pretending(part)) &&
          Am_Point_In_Obj(part, x, y, in_obj))
        return part;
    }
    //in in_obj but not in part
    if (want_self && (want_groups || !am_is_group_and_not_pretending(in_obj)))
      return in_obj;
  }
  return Am_No_Object;
}

Am_Define_Method(Am_Point_In_Or_Self_Method, Am_Object,
                 radio_check_point_in_leaf,
                 (const Am_Object &in_obj, int x, int y,
                  const Am_Object &ref_obj, bool want_self, bool want_groups))
{
  am_translate_coord_to_me(in_obj, ref_obj, x, y);
  if (Am_Point_In_Obj(in_obj, x, y, in_obj)) {
    Am_Value val;
    val = in_obj.Peek(Am_PRETEND_TO_BE_LEAF);
    if (val.Valid())
      return in_obj; // true if slot exists and is non-null
    Am_Value_List comp = in_obj.Get(Am_GRAPHICAL_PARTS);
    if (!comp.Empty()) {
      Am_Object part = comp.Get_Nth(0);
      const Am_Slot_Flags flags = Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR;
      int my_width = in_obj.Get(Am_WIDTH, flags);
      int my_height = in_obj.Get(Am_HEIGHT, flags);
      int box_width = in_obj.Get(Am_BOX_WIDTH, flags);
      bool box_on_left = in_obj.Get(Am_BOX_ON_LEFT, flags);
      int offset_left = 5;
      my_width -= box_width + offset_left;
      int extra_left = 0;
      if (box_on_left)
        extra_left = box_width + offset_left;

      int part_left = part.Get(Am_LEFT, flags);
      int part_top = part.Get(Am_TOP, flags);
      int part_width = part.Get(Am_WIDTH, flags);
      int part_height = part.Get(Am_HEIGHT, flags);

      int display_left = (my_width - part_width) / 2;
      int display_top = (my_height - part_height) / 2;
      x = x + part_left - display_left - extra_left;
      y = y + part_top - display_top;
      Am_Object ret =
          Am_Point_In_Leaf(part, x, y, in_obj, want_self, want_groups);
      if (ret.Valid())
        return ret;
    }
    //went through list, not in a part
    if (want_self && (want_groups || !am_is_group_and_not_pretending(in_obj)))
      return in_obj;
  }
  return Am_No_Object;
}

static void
init()
{
  Am_Object inter;            // interactor in the widget
  Am_Object_Advanced obj_adv; // to get at advanced features like
                              // local-only and demons.

  //////////// Check box ////////////
  // Just a button with a different draw method.
  Am_Checkbox = Am_Button.Create(DSTR("Checkbox"))
                    .Add(Am_BOX_ON_LEFT, true)
                    .Add(Am_BOX_WIDTH, checkbox_box_width)
                    .Add(Am_BOX_HEIGHT, checkbox_box_height)
                    .Set(Am_INVALID_METHOD, radio_check_invalid)
                    .Set(Am_TRANSLATE_COORDINATES_METHOD,
                         radio_check_translate_coordinates)
                    .Set(Am_POINT_IN_PART_METHOD, radio_check_point_in_part)
                    .Set(Am_POINT_IN_LEAF_METHOD, radio_check_point_in_leaf)
                    .Set(Am_FINAL_FEEDBACK_WANTED, true)
                    .Set(Am_WIDTH, checkbox_width)
                    .Set(Am_HEIGHT, checkbox_height)
                    .Set(Am_H_ALIGN, Am_Align_From_Box_On_Left)
                    //    .Set (Am_EXTRA_BUTTON_BORDER, 0)
                    .Set(Am_DRAW_METHOD, checkbox_draw)
                    .Set(Am_MASK_METHOD, checkbox_mask);

  inter = Am_Checkbox.Get(Am_INTERACTOR);
  inter.Set(Am_FIRST_ONE_ONLY, true);

  obj_adv = (Am_Object_Advanced &)Am_Checkbox;

  obj_adv.Get_Slot(Am_BOX_ON_LEFT)
      .Set_Demon_Bits(Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  obj_adv.Get_Slot(Am_BOX_WIDTH)
      .Set_Demon_Bits(Am_MOVING_REDRAW | Am_EAGER_DEMON);
  obj_adv.Get_Slot(Am_BOX_HEIGHT)
      .Set_Demon_Bits(Am_MOVING_REDRAW | Am_EAGER_DEMON);
}

static void
cleanup()
{
  Am_Checkbox.Destroy();
}

static Am_Initializer *initializer =
    new Am_Initializer(DSTR("Am_Checkbox"), init, 5.201f, 107, cleanup);
