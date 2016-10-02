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

Am_Object Am_Button = nullptr;

/******************************************************************************
 * button_draw
 */

Am_Define_Method(Am_Draw_Method, void, button_draw,
                 (Am_Object self, Am_Drawonable *drawonable, int x_offset,
                  int y_offset))
{
  int left = (int)self.Get(Am_LEFT) + x_offset;
  int top = (int)self.Get(Am_TOP) + y_offset;
  int width = self.Get(Am_WIDTH);
  int height = self.Get(Am_HEIGHT);
  bool selected = self.Get(Am_SELECTED);
  bool interim_selected = self.Get(Am_INTERIM_SELECTED);
  bool active = self.Get(Am_ACTIVE);
  bool key_selected = self.Get(Am_KEY_SELECTED);
  bool want_final_selected = self.Get(Am_FINAL_FEEDBACK_WANTED);
  bool is_default = self.Get(Am_DEFAULT);
  bool fringe = self.Get(Am_LEAVE_ROOM_FOR_FRINGE);

  Am_Font font(self.Get(Am_FONT));
  Am_Alignment align = self.Get(Am_H_ALIGN);

  Computed_Colors_Record color_rec = self.Get(Am_STYLE_RECORD);
  Am_Widget_Look look = self.Get(Am_WIDGET_LOOK);

  // now find the contents to draw in the button
  Am_String string;
  Am_Object obj;
  Am_Value value;
  // string slot contains a formula which gets the real object based on the
  // value of the COMMAND slot
  value = self.Peek(Am_REAL_STRING_OR_OBJ);
  if (value.type == Am_STRING)
    string = value;
  else if (value.type == Am_OBJECT)
    obj = value;
  else
    Am_ERRORO("Label of widget "
                  << self << " should have string or object value but it is "
                  << value,
              self, Am_REAL_STRING_OR_OBJ);

  // finally ready to draw it
  Am_Draw_Button_Widget(left, top, width, height, string, obj, interim_selected,
                        selected && want_final_selected, active, key_selected,
                        is_default, fringe, font, color_rec, look,
                        Am_PUSH_BUTTON, drawonable, 0, 0, false, align, 0);
}

/******************************************************************************
 * button_mask
 */

Am_Define_Method(Am_Draw_Method, void, button_mask,
                 (Am_Object self, Am_Drawonable *drawonable, int x_offset,
                  int y_offset))
{
  int left = (int)self.Get(Am_LEFT) + x_offset;
  int top = (int)self.Get(Am_TOP) + y_offset;
  int width = self.Get(Am_WIDTH);
  int height = self.Get(Am_HEIGHT);

  bool key_selected = self.Get(Am_KEY_SELECTED);
  bool is_default = self.Get(Am_DEFAULT);
  bool fringe = self.Get(Am_LEAVE_ROOM_FOR_FRINGE);

  Am_Font font(self.Get(Am_FONT));

  Am_Widget_Look look = self.Get(Am_WIDGET_LOOK);
  switch (look.value) {
  case Am_MOTIF_LOOK_val:
    if (fringe && !is_default) {
      if (key_selected)
        drawonable->Draw_Rectangle(Am_Key_Border_Line, Am_No_Style, left, top,
                                   width, height, Am_DRAW_MASK_COPY);
      drawonable->Draw_Rectangle(Am_No_Style, Am_On_Bits,
                                 left + kMotDefault + kMotKeySel,
                                 top + kMotDefault + kMotKeySel,
                                 width - 2 * (kMotDefault + kMotKeySel),
                                 height - 2 * (kMotDefault + kMotKeySel));
    } else if (fringe && is_default && !key_selected)
      drawonable->Draw_Rectangle(Am_No_Style, Am_On_Bits, left + kMotKeySel,
                                 top + kMotKeySel, width - 2 * kMotKeySel,
                                 height - 2 * kMotKeySel);
    else
      drawonable->Draw_Rectangle(Am_No_Style, Am_On_Bits, left, top, width,
                                 height);
    break;

  case Am_WINDOWS_LOOK_val:
    drawonable->Draw_Rectangle(Am_No_Style, Am_On_Bits, left, top, width,
                               height);
    break;

  case Am_MACINTOSH_LOOK_val:
    if (fringe || is_default) {
      if (is_default) {
        static Am_Style thick =
            Am_Style("black", 3).Clone_With_New_Color(Am_On_Bits);
        drawonable->Draw_Roundtangle(thick, Am_No_Style, left, top, width,
                                     height, 8, 8);
      }
      drawonable->Draw_Roundtangle(Am_On_Bits, Am_On_Bits, left + kMacDefault,
                                   top + kMacDefault, width - 2 * kMacDefault,
                                   height - 2 * kMacDefault, 5, 5);
    } else
      drawonable->Draw_Roundtangle(Am_On_Bits, Am_On_Bits, left, top, width,
                                   height, 5, 5);
    break;

  default:
    Am_Error("Unknown Look parameter");
    break;
  }
}

/******************************************************************************
 * calculate_button_fringe
 *   Calculates the fringe based on the look and if is default or key_selected
 *
 *   The following table specifies how much border is necessary
 *     look     default     key_selected     box
 *     Motif    5           2                5
 *     Win95    0           0                5~
 *     MacOS    4           0                5
 *       ~ 3 pixels are for the key_selected line which is inside the button
 *
 *   By default returns 0
 */

int
calculate_button_fringe(Am_Widget_Look look, bool leave_room, bool key_sel,
                        bool def)
{
  if (leave_room) {
    key_sel = true;
    def = true;
  }

  int border = 0;
  switch (look.value) {
  case Am_MOTIF_LOOK_val:
    border = (key_sel ? 2 * kMotKeySel : 0) + (def ? 2 * kMotDefault : 0);
    break;

  case Am_WINDOWS_LOOK_val:
    border = (key_sel ? 2 * kWinKeySel : 0) + (def ? 2 * kWinDefault : 0);
    break;

  case Am_MACINTOSH_LOOK_val:
    border = (key_sel ? 2 * kMacKeySel : 0) + (def ? 2 * kMacDefault : 0);
    break;

  default:
    Am_Error("Unknown Look parameter");
    break;
  }

  return border;
}

/******************************************************************************
 * button_width
 *   Calculates and returns the width needed for a button.
 *   1. calculates the height of the string or object.
 *   2. adds 2*box value
 *   3. adds any necessary fringe dimensions (2*fringe)
 *        If Am_LEAVE_ROOM_FOR_FRINGE == true
 *          fringe = default + key_selected
 *        If Am_LEAVE_ROOM_FOR_FRINGE == false
 *          fringe = ( default if Am_DEFAULT == true )
 *                 + ( key_selected if Am_KEY_SELECTED == true )
 *
 *   Then uses the following table to add more border as needed:
 *     look     default     key_selected     box     total
 *     Motif    5           2                5
 *     Win95    0           0                5~
 *     MacOS    4           0                5
 *       ~ 3 pixels are for the key_selected line which is inside the button
 *
 *   By default returns 20
 */

Am_Define_Formula(int, button_width)
{
  Am_String string;
  Am_Object obj;
  Am_Value value;
  // string slot contains a formula which gets the real object based on the
  // value of the COMMAND slot
  value = self.Peek(Am_REAL_STRING_OR_OBJ);

  if (value.type == Am_STRING)
    string = value;
  else if (value.type == Am_OBJECT)
    obj = value;
  else
    return 20;

  int offset = self.Get(Am_ITEM_OFFSET);
  bool fringe = self.Get(Am_LEAVE_ROOM_FOR_FRINGE),
       key_sel = self.Get(Am_KEY_SELECTED), def = self.Get(Am_DEFAULT);
  Am_Widget_Look look = self.Get(Am_WIDGET_LOOK);

  int border = calculate_button_fringe(look, fringe, key_sel, def);

  switch (look.value) {
  case Am_MOTIF_LOOK_val:
    border += 2 * kMotBox;
    break;

  case Am_WINDOWS_LOOK_val:
    border += 2 * kWinBox;
    break;

  case Am_MACINTOSH_LOOK_val:
    border += 2 * kMacBox;
    break;

  default:
    Am_Error("Unknown Look parameter");
    break;
  }

  if ((const char *)string) {
    Am_Object window(self.Get(Am_WINDOW));
    Am_Font font(self.Get(Am_FONT));

    Am_Drawonable *draw = GV_a_drawonable(window); //will work if not valid
    if (draw) {
      int str_width, ascent, descent, a, b;
      draw->Get_String_Extents(font, string, strlen(string), str_width, ascent,
                               descent, a, b);
      // we get more white vertical than horizontal white space around text
      // so we want to add a few pixels to str_width
      str_width += 3;
      return str_width + border + 2 * offset;
    }
  } else if (obj.Valid())
    return (int)obj.Get(Am_WIDTH) + border + 2 * offset;

  return 20;
}

/******************************************************************************
 * button_height
 *   Calculates and returns the height needed for a button.
 *   1. calculates the height of the string or object.
 *   2. adds 2*box value
 *   3. adds any necessary fringe dimensions (2*fringe)
 *        If Am_LEAVE_ROOM_FOR_FRINGE == true
 *          fringe = default + key_selected
 *        If Am_LEAVE_ROOM_FOR_FRINGE == false
 *          fringe = ( default if Am_DEFAULT == true )
 *                 + ( key_selected if Am_KEY_SELECTED == true )
 *
 *   Then uses the following table to add more border as needed:
 *     look     default     key_selected     box     total
 *     Motif    5           2                2       9
 *     Win95    0           0                5~      5
 *     MacOS    4           0                3       7
 *       ~ 3 pixels are for the key_selected line which is inside the button
 *
 *   By default returns 10
 */

Am_Define_Formula(int, button_height)
{
  Am_String string;
  Am_Object obj = nullptr;
  Am_Value value;
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
    return 10;

  int offset = self.Get(Am_ITEM_OFFSET);
  bool fringe = self.Get(Am_LEAVE_ROOM_FOR_FRINGE),
       key_sel = self.Get(Am_KEY_SELECTED), def = self.Get(Am_DEFAULT);
  Am_Widget_Look look = self.Get(Am_WIDGET_LOOK);

  int border = calculate_button_fringe(look, fringe, key_sel, def);

  switch (look.value) {
  case Am_MOTIF_LOOK_val:
    border += 2 * kMotBox;
    break;

  case Am_WINDOWS_LOOK_val:
    border += 2 * kWinBox;
    break;

  case Am_MACINTOSH_LOOK_val:
    border += 2 * kMacBox;
    break;

  default:
    Am_Error("Unknown Look parameter");
    break;
  }

  if ((const char *)string) {
    Am_Object window(self.Get(Am_WINDOW));
    Am_Font font(self.Get(Am_FONT));

    Am_Drawonable *draw = GV_a_drawonable(window); //will work if not valid
    if (draw) {
      int str_width, ascent, descent, a, b;
      draw->Get_String_Extents(font, string, strlen(string), str_width, ascent,
                               descent, a, b);
      return ascent + descent + border + 2 * offset;
    }
  } else if (obj.Valid())
    return (int)obj.Get(Am_HEIGHT) + border + 2 * offset +
           ((look == Am_WINDOWS_LOOK) ? 6 : 0); // + 6 for key_selection room

  return 10;
}

Am_Define_Method(Am_Invalid_Method, void, button_invalid,
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
    int part_left = which_part.Get(Am_LEFT, flags);
    int part_top = which_part.Get(Am_TOP, flags);
    int part_width = which_part.Get(Am_WIDTH, flags);
    int part_height = which_part.Get(Am_HEIGHT, flags);
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
                 button_translate_coordinates,
                 (const Am_Object &obj, const Am_Object &for_part, int in_x,
                  int in_y, int &out_x, int &out_y))
{
  if (for_part.Valid()) {
    const Am_Slot_Flags flags = Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR;
    int my_width = obj.Get(Am_WIDTH, flags);
    int my_height = obj.Get(Am_HEIGHT, flags);
    int part_left = for_part.Get(Am_LEFT, flags);
    int part_top = for_part.Get(Am_TOP, flags);
    int part_width = for_part.Get(Am_WIDTH, flags);
    int part_height = for_part.Get(Am_HEIGHT, flags);
    int display_left = (my_width - part_width) / 2;
    int display_top = (my_height - part_height) / 2;
    out_x = (int)obj.Get(Am_LEFT) + display_left - part_left + in_x;
    out_y = (int)obj.Get(Am_TOP) + display_top - part_top + in_y;
  } else {
    out_x = (int)obj.Get(Am_LEFT) + in_x;
    out_y = (int)obj.Get(Am_TOP) + in_y;
  }
}

Am_Define_Method(Am_Point_In_Or_Self_Method, Am_Object, button_point_in_part,
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
      int part_left = part.Get(Am_LEFT, flags);
      int part_top = part.Get(Am_TOP, flags);
      int part_width = part.Get(Am_WIDTH, flags);
      int part_height = part.Get(Am_HEIGHT, flags);
      int display_left = (my_width - part_width) / 2;
      int display_top = (my_height - part_height) / 2;
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

Am_Define_Method(Am_Point_In_Or_Self_Method, Am_Object, button_point_in_leaf,
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
      int part_left = part.Get(Am_LEFT, flags);
      int part_top = part.Get(Am_TOP, flags);
      int part_width = part.Get(Am_WIDTH, flags);
      int part_height = part.Get(Am_HEIGHT, flags);
      int display_left = (my_width - part_width) / 2;
      int display_top = (my_height - part_height) / 2;
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

Am_Define_Method(Am_Object_Method, void, button_abort_method,
                 (Am_Object widget))
{
  Am_Object inter = widget.Get_Object(Am_INTERACTOR);
  Am_Abort_Interactor(inter);
  //now restore the widget's correct value
  //assume the value has already been set, so just invert the current value
  Am_Value v;
  v = widget.Peek(Am_VALUE);
  if (v.Valid()) {
    widget.Set(Am_VALUE, (0L));
    widget.Set(Am_SELECTED, false);
  } else {
    v = widget.Peek(Am_LABEL_OR_ID);
    widget.Set(Am_VALUE, v);
    widget.Set(Am_SELECTED, true);
  }
}

// Do method for the command object in the interator for the single button.
// Set the value of the widget, me and my parent command
Am_Define_Method(Am_Object_Method, void, button_inter_command_do,
                 (Am_Object command_obj))
{
  Am_Value old_value, new_value, inter_value;

  // set the widget's and parent's value.
  Am_Object parent, inter, widget;
  parent = command_obj.Get(Am_IMPLEMENTATION_PARENT);
  inter = command_obj.Get_Owner();
  widget = inter.Get_Owner();
  old_value = widget.Peek(Am_VALUE);
  inter_value = inter.Peek(Am_VALUE);
  if (inter_value.Valid())
    new_value = widget.Peek(Am_LABEL_OR_ID);
  else
    new_value = (0L);

  Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING,
                       "++ DO method setting the Am_VALUE of "
                           << widget << " to " << new_value);
  Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING,
                       "++ DO method setting the Am_VALUE of "
                           << command_obj << " to " << new_value);
  widget.Set(Am_VALUE, new_value);
  command_obj.Set(Am_OLD_VALUE, old_value);
  command_obj.Set(Am_VALUE, new_value);

  if (parent.Valid()) {
    //set old value to current value
    //old_value=parent.Peek(Am_VALUE); // RGM: This is wrong.
    parent.Set(Am_OLD_VALUE, old_value);
    parent.Set(Am_VALUE, new_value);
  }
}

/******************************************************************************
 * button_sel_from_value
 *   in selected slot of a button widget, for circular constraints so if VALUE
 *   set, the button gets highlighted correctly
 */

Am_Define_Formula(bool, button_sel_from_value)
{
  Am_Value value;
  value = self.Peek(Am_VALUE);
  return value.Valid();
}

/******************************************************************************
 * get_button_widget_value
 *  goes in the VALUE slot of the interactor of a single
 *  button, to get the value of the widget.  This is
 *  necessary to set up a circular constraint in case someone sets the value of
 *  the command from the outside.
 */

Am_Define_Object_Formula(get_button_widget_value)
{
  Am_Object widget = self.Get_Owner(); // widget the interactor is in
  if (widget.Valid()) {
    Am_Value value;
    value = widget.Peek(Am_VALUE);
    if (value.Valid())
      return widget;
  }
  return Am_No_Object;
}

/******************************************************************************
 * Am_Destroy_Button
 *   demon procedure
 *   see if have an allocated object attached to me, and if so, destroy
 *   it also.
 *   Also removes any accelerators
 */

void
Am_Destroy_Button(Am_Object object)
{
  Am_Object attached, command;
  attached = object.Peek_Object(Am_ATTACHED_OBJECT, Am_NO_DEPENDENCY);
  if (attached.Valid())
    attached.Remove_From_Owner();

  if (object.Peek(Am_ATTACHED_COMMAND, Am_NO_DEPENDENCY).Exists()) {
    Am_Value cmd_v = object.Peek(Am_COMMAND, Am_NO_DEPENDENCY);
    attached = object.Get(Am_ATTACHED_COMMAND, Am_NO_DEPENDENCY);
    if (cmd_v.type == Am_OBJECT) {
      command = cmd_v;
      if (attached == command)
        command.Remove_From_Owner();
    }
  }
  if (object.Peek(Am_SUB_MENU, Am_NO_DEPENDENCY).Exists()) {
    attached = object.Get(Am_SUB_MENU);
    attached.Destroy();
  }
  Am_Object_Demon *proto_demon = ((Am_Object_Advanced &)Am_Aggregate)
                                     .Get_Demons()
                                     .Get_Object_Demon(Am_DESTROY_OBJ);
  if (proto_demon)
    proto_demon(object);

  //now handle accelerators, if any
  Am_Value cmd_value;
  Am_Object old_cmd_obj, old_cmd_window;
  cmd_value = object.Peek(Am_ACCELERATOR_LIST, Am_NO_DEPENDENCY);
  if (cmd_value.Valid()) {
    old_cmd_obj = cmd_value;
    cmd_value = object.Peek(Am_ACCELERATOR_INTER, Am_NO_DEPENDENCY);
    if (cmd_value.Valid()) {
      old_cmd_window = cmd_value;
      Am_Remove_Accelerator_Command_From_Window(old_cmd_obj, old_cmd_window);
    }
  }
}

static void
init()
{
  Am_Object inter;            // interactor in the widget
  Am_Object_Advanced obj_adv; // to get at advanced features like
                              // local-only and demons.

  //////////// button /////////////
  // instance of a group so can have a part (the contents)
  Am_Button =
      Am_Widget_Aggregate.Create(DSTR("Button"))
          .Add(Am_VALUE, (0L))
          .Add(Am_OLD_VALUE, (0L))
          .Add(Am_SELECTED,
               false) //set by interactor OR computed from the value
                      //of the command object
          .Set(Am_SELECTED, button_sel_from_value.Multi_Constraint())
          .Add(Am_ITEM_OFFSET, 0) // how far to indent the string or obj
          .Add(Am_INTERIM_SELECTED, false)
          .Add(Am_ACTIVE, Am_Active_From_Command)
          .Add(Am_ACTIVE_2, true) // used by interactive tools
          .Add(Am_KEY_SELECTED, false)
          .Add(Am_DEFAULT, Am_Default_From_Command)
          .Add(Am_LEAVE_ROOM_FOR_FRINGE, true)
          .Add(Am_FONT, Am_Default_Font)
          .Add(Am_FILL_STYLE, Am_Default_Color)
          .Add(Am_STYLE_RECORD, Am_Get_Computed_Colors_Record_Form)
          .Add(Am_FINAL_FEEDBACK_WANTED, false)
          .Set(Am_DRAW_METHOD, button_draw)
          .Set(Am_MASK_METHOD, button_mask)
          .Set(Am_INVALID_METHOD, button_invalid)
          .Set(Am_TRANSLATE_COORDINATES_METHOD, button_translate_coordinates)
          .Set(Am_POINT_IN_PART_METHOD, button_point_in_part)
          .Set(Am_POINT_IN_LEAF_METHOD, button_point_in_leaf)
          .Add(Am_REAL_STRING_OR_OBJ, Am_Get_Real_String_Or_Obj)
          .Set(Am_WIDTH, button_width)
          .Set(Am_HEIGHT, button_height)
          .Set(Am_H_ALIGN, Am_CENTER_ALIGN)
          .Add(Am_LABEL_OR_ID, Am_Get_Label_Or_ID)
          .Add(Am_SET_COMMAND_OLD_OWNER, Am_Set_Old_Owner_To_Me)
          .Add(Am_ACCELERATOR_STRING, check_accel_string)
          .Add(Am_WIDGET_START_METHOD, Am_Standard_Widget_Start_Method)
          .Add(Am_WIDGET_ABORT_METHOD, button_abort_method)
          .Add(Am_WIDGET_STOP_METHOD, Am_Standard_Widget_Stop_Method)
          .Add_Part(Am_COMMAND, Am_Command.Create(DSTR("Button_Command"))
                                    .Set(Am_LABEL, "Button")
                                    .Set(Am_VALUE, 0))
          .Add_Part(
              Am_INTERACTOR,
              inter =
                  Am_Choice_Interactor_Repeat_Same
                      .Create(DSTR("inter_in_button"))
                      .Set(Am_HOW_SET, Am_CHOICE_TOGGLE)
                      .Set(Am_START_WHEN, Am_Default_Widget_Start_Char)
                      .Set(Am_START_WHERE_TEST, Am_Inter_In)
                      .Set(Am_ACTIVE, Am_Active_And_Active2)
                      // set up a circular constraint between the value in the command of
                      // the interactor and the value in the top-level button widget so setting
                      // one will cause the other to change also
                      .Set(Am_VALUE,
                           get_button_widget_value.Multi_Constraint()));
  inter.Get_Object(Am_COMMAND)
      .Set(Am_IMPLEMENTATION_PARENT, Am_Get_Owners_Command)
      .Set(Am_DO_METHOD, button_inter_command_do)
      .Set(Am_UNDO_METHOD, Am_Widget_Inter_Command_Undo)
      .Set(Am_REDO_METHOD, Am_Widget_Inter_Command_Undo)
      .Set(Am_SELECTIVE_UNDO_METHOD, Am_Widget_Inter_Command_Selective_Undo)
      .Set(Am_SELECTIVE_REPEAT_SAME_METHOD,
           Am_Widget_Inter_Command_Selective_Repeat)
      .Set(Am_SELECTIVE_REPEAT_ON_NEW_METHOD, (0L));

  obj_adv = (Am_Object_Advanced &)Am_Button;

  obj_adv.Get_Slot(Am_REAL_STRING_OR_OBJ)
      .Set_Demon_Bits(Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  obj_adv.Get_Slot(Am_SELECTED)
      .Set_Demon_Bits(Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  obj_adv.Get_Slot(Am_INTERIM_SELECTED)
      .Set_Demon_Bits(Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  obj_adv.Get_Slot(Am_KEY_SELECTED)
      .Set_Demon_Bits(Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  obj_adv.Get_Slot(Am_ACTIVE).Set_Demon_Bits(Am_STATIONARY_REDRAW |
                                             Am_EAGER_DEMON);
  obj_adv.Get_Slot(Am_WIDGET_LOOK)
      .Set_Demon_Bits(Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  obj_adv.Get_Slot(Am_FINAL_FEEDBACK_WANTED)
      .Set_Demon_Bits(Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  obj_adv.Get_Slot(Am_DEFAULT)
      .Set_Demon_Bits(Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  obj_adv.Get_Slot(Am_FONT).Set_Demon_Bits(Am_STATIONARY_REDRAW |
                                           Am_EAGER_DEMON);
  obj_adv.Get_Slot(Am_FILL_STYLE)
      .Set_Demon_Bits(Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  obj_adv.Get_Slot(Am_H_ALIGN)
      .Set_Demon_Bits(Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  obj_adv.Get_Slot(Am_LEAVE_ROOM_FOR_FRINGE)
      .Set_Demon_Bits(Am_STATIONARY_REDRAW | Am_EAGER_DEMON);

  // all the next slots should not be inherited
  obj_adv.Get_Slot(Am_SELECTED).Set_Inherit_Rule(Am_COPY);
  obj_adv.Get_Slot(Am_INTERIM_SELECTED).Set_Inherit_Rule(Am_COPY);
  obj_adv.Get_Slot(Am_KEY_SELECTED).Set_Inherit_Rule(Am_COPY);
  obj_adv.Get_Slot(Am_ACTIVE).Set_Inherit_Rule(Am_COPY);
  obj_adv.Get_Slot(Am_ACTIVE_2).Set_Inherit_Rule(Am_COPY);

  Am_Demon_Set demons = obj_adv.Get_Demons().Copy();
  demons.Set_Object_Demon(Am_DESTROY_OBJ, Am_Destroy_Button);
  obj_adv.Set_Demons(demons);
}

static void
cleanup()
{
  Am_Button.Destroy();
}

static Am_Initializer *initializer =
    new Am_Initializer(DSTR("Am_Button"), init, 5.2f, 105, cleanup);
