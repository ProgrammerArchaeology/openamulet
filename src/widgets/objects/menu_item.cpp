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

Am_Object Am_Menu_Item;

Am_Style am_grey_2; //these are used by the check marks
Am_Style am_white_2;

/******************************************************************************
 * menu_item_draw
 */

Am_Define_Method(Am_Draw_Method, void, menu_item_draw,
                 (Am_Object self, Am_Drawonable *draw, int x_offset,
                  int y_offset))
{
  int left = (int)self.Get(Am_LEFT) + x_offset;
  int top = (int)self.Get(Am_TOP) + y_offset;
  int width = self.Get(Am_WIDTH);
  int height = self.Get(Am_HEIGHT);
  bool selected = self.Get(Am_SELECTED);
  bool interim_selected = self.Get(Am_INTERIM_SELECTED);
  bool active = self.Get(Am_ACTIVE);
  bool want_final_selected = self.Get(Am_FINAL_FEEDBACK_WANTED);
  Am_Font font(self.Get(Am_FONT));
  Computed_Colors_Record color_rec = self.Get(Am_STYLE_RECORD);
  Am_Widget_Look look = self.Get(Am_WIDGET_LOOK);

  bool draw_selected = selected && want_final_selected;
  bool checked = self.Get(Am_CHECKED_ITEM, Am_RETURN_ZERO_ON_ERROR).Valid();

  // now find the contents to draw in the button
  Am_String string;
  Am_Object obj;
  Am_Value value;
  bool line = false;

  // string slot contains a formula which gets the real object based on the
  // value of the COMMAND slot
  value = self.Peek(Am_REAL_STRING_OR_OBJ);

  switch (value.type) {
  case Am_STRING:
    string = value;
    break;

  case Am_OBJECT:
    obj = value;
    break;

  case Am_INT: // ok to fall thru ( Am_INT || Am_BOOL )
  case Am_BOOL:
    line = true;
    break;

  default:
    Am_ERRORO("String slot of widget "
                  << self << " should have string or object type, but value is "
                  << value,
              self, Am_REAL_STRING_OR_OBJ);
  }

  Am_String accel_string;
  Am_Value accel_value;
  accel_value = self.Peek(Am_ACCELERATOR_STRING);
  if (accel_value.type == Am_STRING)
    accel_string = accel_value;

  // let's draw it
  if (line) // if it's just a line, draw it.
  {
    switch (look.value) {
    case Am_MOTIF_LOOK_val:
      draw->Draw_Line(color_rec.data->shadow_style, left, top, left + width - 1,
                      top);
      draw->Draw_Line(color_rec.data->highlight_style, left, top + 1,
                      left + width - 1, top + 1);
      break;

    case Am_WINDOWS_LOOK_val:
      draw->Draw_Line(color_rec.data->background_style, left, top + 3,
                      left + width - 1, top + 3);
      draw->Draw_Line(color_rec.data->highlight_style, left, top + 4,
                      left + width - 1, top + 4);
      break;

    case Am_MACINTOSH_LOOK_val:
      draw->Draw_Line(Am_Style("grey", 1), left, top + 8, left + width - 1,
                      top + 8);
      break;

    default:
      Am_Error("Unknown Look parameter");
      break;
    }
    return;
  }

  if (draw_selected || interim_selected)
    switch (look.value) {
    case Am_MOTIF_LOOK_val:
      Am_Draw_Motif_Box(left, top, width, height, false, color_rec, draw);
      break;

    case Am_WINDOWS_LOOK_val: {
      Am_Style fill_color = self.Get(Am_MENU_SELECTED_COLOR);
      draw->Draw_Rectangle(Am_No_Style, fill_color, left, top, width, height);
      break;
    }

    case Am_MACINTOSH_LOOK_val:
      draw->Draw_Rectangle(Am_No_Style, Am_Black, left, top, width, height);
      break;

    default:
      Am_Error("Unknown Look parameter");
      break;
    }

  int leftOffset = self.Get(Am_MENU_ITEM_LEFT_OFFSET),
      topOffset = self.Get(Am_MENU_ITEM_TOP_OFFSET);

  if (checked) {
    draw->Push_Clip(left, top, width, height);
    Am_Style check_style = Am_Line_2;
    if (!active)
      check_style = am_grey_2;
    else if ((look.value == Am_MACINTOSH_LOOK_val) &&
             (interim_selected || (selected && want_final_selected)))
      check_style = am_white_2;
    int check_height = height - 4;
    if (check_height > 12)
      check_height = 12;
    draw->Draw_2_Lines(check_style, Am_No_Style, left + 2, top + (height / 2),
                       left + 5, top + ((height + check_height) / 2), left + 10,
                       top + ((height - check_height) / 2));
    draw->Pop_Clip();
  }

  // now draw the string if any
  Am_Style text_style =
      compute_text_style(active, draw_selected || interim_selected, color_rec,
                         look, Am_MENU_BUTTON);

  int str_width, ascent, descent, a, b, str_left, str_top;

  if ((const char *)string) {
    draw->Get_String_Extents(font, string, strlen(string), str_width, ascent,
                             descent, a, b);

    // always left justify the text for now
    str_left = left + leftOffset;
    str_top = top + topOffset;

    // set a clip region in case string bigger than the button
    draw->Push_Clip(left, top, width, height);
    // does not correctly clip top and bottom
    draw->Draw_Text(text_style, string, strlen(string), font, str_left,
                    str_top);
    draw->Pop_Clip();
  } else if (obj.Valid()) {
    // left justify the object; since a part of the button, will be offset from
    // buttons' left and top automatically.
    int obj_left = leftOffset;
    int obj_top = topOffset;
    int orig_left = obj.Get(Am_LEFT);
    int orig_top = obj.Get(Am_TOP);

    // call the object's draw method to draw the component
    bool line_changed = false;
    if ((selected || interim_selected) && look == Am_MACINTOSH_LOOK) {
      if (obj.Peek(Am_LINE_STYLE).Exists()) {
        if (obj.Get(Am_LINE_STYLE) == Am_Black) {
          obj.Set(Am_LINE_STYLE, Am_White);
          line_changed = true;
        }
      }
    }

    draw->Push_Clip(left, top, width, height);
    Am_Draw(obj, draw, left - orig_left + obj_left, top - orig_top + obj_top);
    draw->Pop_Clip();

    if (line_changed)
      obj.Set(Am_LINE_STYLE, Am_Black);
  }

  if ((const char *)accel_string) {
    // always right justify the accel text
    draw->Get_String_Extents(font, accel_string, strlen(accel_string),
                             str_width, ascent, descent, a, b);
    str_left = left + width - str_width - 5;
    str_top = top + topOffset;

    // set a clip region in case string bigger than the button
    draw->Push_Clip(left, top, width, height);
    draw->Draw_Text(text_style, accel_string, strlen(accel_string), font,
                    str_left, str_top);
    draw->Pop_Clip();
  }
}

/******************************************************************************
 * menu_item_height
 */

Am_Define_Formula(int, menu_item_height)
{
  Am_String string;
  Am_Object obj;
  Am_Value value;

  int topOffset = self.Get(Am_MENU_ITEM_TOP_OFFSET),
      botOffset = self.Get(Am_MENU_ITEM_BOT_OFFSET),
      lineHeight = self.Get_Owner().Get(Am_MENU_LINE_HEIGHT);

  // string slot contains a formula which gets the real object based on the
  // value of the COMMAND slot
  value = self.Peek(Am_REAL_STRING_OR_OBJ);
  switch (value.type) {
  case Am_STRING:
    string = value;
    break;

  case Am_OBJECT:
    obj = value;
    break;

  case Am_INT: // ok to fall thru ( Am_INT || Am_BOOL )
  case Am_BOOL:
    return lineHeight;
  }

  if ((const char *)string) {
    Am_Object window(self.Get(Am_WINDOW));
    Am_Font font(self.Get(Am_FONT));

    Am_Drawonable *draw = GV_a_drawonable(window); //will work if not valid
    if (draw) {
      int str_width, ascent, descent, a, b;
      draw->Get_String_Extents(font, string, strlen(string), str_width, ascent,
                               descent, a, b);
      return topOffset + ascent + botOffset;
    }
  } else if (obj.Valid())
    return (int)obj.Get(Am_HEIGHT) + topOffset + botOffset;

  return 10;
}

/******************************************************************************
 * menu_item_width
 */

Am_Define_Formula(int, menu_item_width)
{
  Am_String string;
  Am_Object obj, window;
  Am_Value value, cmd;
  Am_Font font;

  window = self.Get(Am_WINDOW);
  font = self.Get(Am_FONT);
  int str_width, ascent, descent, a, b;
  int accel_width = 0; // includes the offset before the accel string

  int leftOffset = self.Get(Am_MENU_ITEM_LEFT_OFFSET),
      accelOffset = self.Get(Am_MENU_ITEM_ACCEL_OFFSET),
      rightOffset = self.Get(Am_MENU_ITEM_RIGHT_OFFSET);

  Am_Drawonable *draw = GV_a_drawonable(window); //will work if not valid
  if (draw) {
    cmd = self.Peek(Am_COMMAND);
    if (cmd.Valid() && cmd.type == Am_OBJECT) {
      Am_Object cmd_obj = (Am_Object)cmd;
      if (cmd_obj.Is_Instance_Of(Am_Menu_Line_Command))
        return 0; // don't worry about the width of menu-lines
      else        //can only have an accelerator if have a command
      {
        value = self.Peek(Am_ACCELERATOR_STRING);
        if (value.type == Am_STRING) {
          Am_String accel_string;
          accel_string = value;
          draw->Get_String_Extents(font, accel_string, strlen(accel_string),
                                   accel_width, ascent, descent, a, b);
          accel_width += accelOffset;
        }
      }
    }

    // slot contains a formula which gets the real object based on the
    // value of the COMMAND slot
    value = self.Peek(Am_REAL_STRING_OR_OBJ);
    if (value.type == Am_STRING)
      string = value;
    else if (value.type == Am_OBJECT)
      obj = value;
    else
      return 20;
    if ((const char *)string) {
      draw->Get_String_Extents(font, string, strlen(string), str_width, ascent,
                               descent, a, b);
      return leftOffset + str_width + accel_width + rightOffset;
    } else if (obj.Valid()) {
      int obj_width = obj.Get(Am_WIDTH);
      return leftOffset + obj_width + accel_width + rightOffset;
    }
  }
  //if get here, something wrong, don't have a real size
  return 20;
}

Am_Define_Method(Am_Invalid_Method, void, menu_item_invalid,
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
    int display_left = group.Get(Am_MENU_ITEM_LEFT_OFFSET, flags);
    int display_top = group.Get(Am_MENU_ITEM_TOP_OFFSET, flags);
    int part_left = which_part.Get(Am_LEFT, flags);
    int part_top = which_part.Get(Am_TOP, flags);

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
                 menu_item_translate_coordinates,
                 (const Am_Object &obj, const Am_Object &for_part, int in_x,
                  int in_y, int &out_x, int &out_y))
{
  if (for_part.Valid()) {
    const Am_Slot_Flags flags = Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR;
    int display_left = obj.Get(Am_MENU_ITEM_LEFT_OFFSET, flags);
    int display_top = obj.Get(Am_MENU_ITEM_TOP_OFFSET, flags);
    int part_left = for_part.Get(Am_LEFT, flags);
    int part_top = for_part.Get(Am_TOP, flags);
    out_x = (int)obj.Get(Am_LEFT) + display_left - part_left + in_x;
    out_y = (int)obj.Get(Am_TOP) + display_top - part_top + in_y;
  } else {
    out_x = (int)obj.Get(Am_LEFT) + in_x;
    out_y = (int)obj.Get(Am_TOP) + in_y;
  }
}

Am_Define_Method(Am_Point_In_Or_Self_Method, Am_Object, menu_item_point_in_part,
                 (const Am_Object &in_obj, int x, int y,
                  const Am_Object &ref_obj, bool want_self, bool want_groups))
{
  am_translate_coord_to_me(in_obj, ref_obj, x, y);
  if (Am_Point_In_Obj(in_obj, x, y, in_obj).Valid()) {
    Am_Value_List comp = in_obj.Get(Am_GRAPHICAL_PARTS);
    if (!comp.Empty()) {
      Am_Object part = comp.Get_Nth(0);
      const Am_Slot_Flags flags = Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR;
      int display_left = in_obj.Get(Am_MENU_ITEM_LEFT_OFFSET, flags);
      int display_top = in_obj.Get(Am_MENU_ITEM_TOP_OFFSET, flags);
      int part_left = part.Get(Am_LEFT, flags);
      int part_top = part.Get(Am_TOP, flags);
      x = x + part_left - display_left;
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

Am_Define_Method(Am_Point_In_Or_Self_Method, Am_Object, menu_item_point_in_leaf,
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
      int display_left = in_obj.Get(Am_MENU_ITEM_LEFT_OFFSET, flags);
      int display_top = in_obj.Get(Am_MENU_ITEM_TOP_OFFSET, flags);
      int part_left = part.Get(Am_LEFT, flags);
      int part_top = part.Get(Am_TOP, flags);
      x = x + part_left - display_left;
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

/******************************************************************************
 * menu_item_top_offset
 */

Am_Define_Formula(int, menu_item_top_offset)
{
  Am_Widget_Look look = self.Get(Am_WIDGET_LOOK);
  switch (look.value) {
  case Am_MOTIF_LOOK_val:
    return 5;

  case Am_WINDOWS_LOOK_val:
    return 3;

  case Am_MACINTOSH_LOOK_val:
    return 2;

  default:
    Am_Error("Unknown Look parameter");
    break;
  }
  return 0; // we should never get here
}

/******************************************************************************
 * menu_item_bot_offset
 */

Am_Define_Formula(int, menu_item_bot_offset)
{
  Am_Widget_Look look = self.Get(Am_WIDGET_LOOK);
  switch (look.value) {
  case Am_MOTIF_LOOK_val:
    return 5;

  case Am_WINDOWS_LOOK_val:
    return 5;

  case Am_MACINTOSH_LOOK_val:
    return 4;

  default:
    Am_Error("Unknown Look parameter");
    break;
  }
  return 0; // we should never get here
}

/******************************************************************************
 * menu_item_left_offset
 */

int
menu_item_left_offset_internal(Am_Widget_Look &look, bool menuBarItem)
{
  switch (look.value) {
  case Am_MOTIF_LOOK_val:
    return menuBarItem ? 5 : 15; //formerly 5

  case Am_WINDOWS_LOOK_val:
    return menuBarItem ? 6 : 22;

  case Am_MACINTOSH_LOOK_val:
    return menuBarItem ? 11 : 15;

  default:
    Am_Error("Unknown Look parameter");
    break;
  }
  return 2; // we should never get here
}

Am_Define_Formula(int, menu_item_left_offset)
{
  Am_Widget_Look look = self.Get(Am_WIDGET_LOOK);
  bool menuBarItem = false;
  if (self.Get_Owner().Is_Instance_Of(Am_Menu_Bar))
    // is this item in the menu bar
    menuBarItem = true;
  return menu_item_left_offset_internal(look, menuBarItem);
}

/******************************************************************************
 * menu_item_accel_offset
 */

Am_Define_Formula(int, menu_item_accel_offset)
{
  Am_Widget_Look look = self.Get(Am_WIDGET_LOOK);
  switch (look.value) {
  case Am_MOTIF_LOOK_val:
    return 16;

  case Am_WINDOWS_LOOK_val:
    return 9;

  case Am_MACINTOSH_LOOK_val:
    return 11;

  default:
    Am_Error("Unknown Look parameter");
    break;
  }
  return 6; // we should never get here
}

/******************************************************************************
 * menu_item_right_offset
 */

Am_Define_Formula(int, menu_item_right_offset)
{
  Am_Widget_Look look = self.Get(Am_WIDGET_LOOK);
  bool menuBarItem = false;
  if (self.Get_Owner().Is_Instance_Of(Am_Menu_Bar))
    // is this item in the menu bar
    menuBarItem = true;

  switch (look.value) {
  case Am_MOTIF_LOOK_val:
    return 5;

  case Am_WINDOWS_LOOK_val:
    return menuBarItem ? 5 : 19;

  case Am_MACINTOSH_LOOK_val:
    return menuBarItem ? 10 : 6;

  default:
    Am_Error("Unknown Look parameter");
    break;
  }
  return 0; // we should never get here
}

static void
init()
{
  am_grey_2 = Am_Style(0.5f, 0.5f, 0.5f, 2);
  am_white_2 = Am_Style(1.0f, 1.0f, 1.0f, 2);

  //////////// Menu Item ////////////
  // Just another button with a different draw method.

  Am_Menu_Item =
      Am_Button.Create(DSTR("Menu_Item"))
          .Set(Am_WIDTH, menu_item_width)
          .Set(Am_DRAW_METHOD, menu_item_draw)
          .Set(Am_INVALID_METHOD, menu_item_invalid)
          .Set(Am_TRANSLATE_COORDINATES_METHOD, menu_item_translate_coordinates)
          .Set(Am_POINT_IN_PART_METHOD, menu_item_point_in_part)
          .Set(Am_POINT_IN_LEAF_METHOD, menu_item_point_in_leaf)
          .Add(Am_MENU_ITEM_LEFT_OFFSET, menu_item_left_offset)
          .Add(Am_MENU_ITEM_ACCEL_OFFSET, menu_item_accel_offset)
          .Add(Am_MENU_ITEM_RIGHT_OFFSET, menu_item_right_offset)
          .Add(Am_MENU_ITEM_TOP_OFFSET, menu_item_top_offset)
          .Add(Am_MENU_ITEM_BOT_OFFSET, menu_item_bot_offset)
          .Add(Am_CHECKED_ITEM, am_checked_from_command)
          .Add(Am_MENU_SELECTED_COLOR, Am_Yellow);

  Am_Object_Advanced obj_adv = (Am_Object_Advanced &)Am_Menu_Item /*inter*/;
  obj_adv.Get_Slot(Am_MENU_SELECTED_COLOR)
      .Set_Demon_Bits(Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
}

static void
cleanup()
{
  Am_Menu_Item.Destroy();
}

static Am_Initializer *initializer =
    new Am_Initializer(DSTR("Am_Menu_Item"), init, 5.203f, 115, cleanup);
