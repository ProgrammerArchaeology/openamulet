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

#include <amulet/impl/opal_obj2.h>
#include <amulet/impl/widget_op.h>
#include <amulet/impl/inter_undo.h>

Am_Object Am_Option_Button;

extern void set_panel_value_from_inter_value(Am_Object inter);
extern bool inter_value_is_or_contains(Am_Object inter, Am_Object new_object);

// do method for the command in the interactor in the button panel.
// Sets the panel widget's value,
// set the IMPLEMENTATION_PARENT slot of me
// set the Am_VALUE slot of me and of the parent command
Am_Define_Method(Am_Object_Method, void, Am_Inter_For_Panel_Do,
                 (Am_Object command_obj))
{
  Am_Object new_object, item, item_command, panel, panel_command,
      parent_command;
  Am_Object inter = command_obj.Get_Owner(); // owner will be interactor
  Am_Value old_value, new_value, cmd_obj_value;
  if (inter.Valid()) {
    //get old_value
    panel = inter.Get_Owner(); // panel the interactor is in
    old_value = panel.Peek(Am_VALUE);
    panel.Set(Am_OLD_VALUE, old_value);
    set_panel_value_from_inter_value(inter);
    //set command_obj's slots
    new_value = panel.Peek(Am_VALUE);
    command_obj.Set(Am_OLD_VALUE, old_value);
    Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING,
                         "++ DO method setting the Am_VALUE of "
                             << command_obj << " to " << new_value);
    command_obj.Set(Am_VALUE, new_value);

    new_object = inter.Get(Am_INTERIM_VALUE);
    // new_object is a sub-widget (an individual button)
    if (panel.Valid()) {
      cmd_obj_value = panel.Peek(Am_COMMAND);
      if (cmd_obj_value.type == Am_OBJECT) {
        panel_command = cmd_obj_value;
        if (!(panel_command.Is_Instance_Of(Am_Command)))
          panel_command = Am_No_Object;
      }
    }

    if (new_object.Valid() &&
        !panel.Is_Instance_Of(Am_Menu_In_Scrolling_Menu)) {
      // for scrolling menus, don't ever want to use the sub-item's command
      // object's method
      cmd_obj_value = new_object.Peek(Am_COMMAND);
      if (cmd_obj_value.type == Am_OBJECT) {
        item_command = cmd_obj_value;
        if (item_command.Is_Instance_Of(Am_Command)) {
          item = new_object;
          parent_command = item_command;
          //set the value of the item_command to the item's label_or_id or 0
          if (inter_value_is_or_contains(inter, new_object))
            new_value = new_object.Peek(Am_LABEL_OR_ID);
          else
            new_value = 0;
        } else
          item_command = Am_No_Object; // wrong type object
      }
    }
    if (!item_command.Valid()) { //not in the item, try to find a global cmd
      if (panel_command.Valid()) {
        parent_command = panel_command;
        //set the value of the panel_command to be the same as the panel's
        new_value = panel.Peek(Am_VALUE);
      }
    }
    old_value = parent_command.Peek(Am_VALUE);
    parent_command.Set(Am_OLD_VALUE, old_value);
    Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING,
                         "++ DO method setting the Am_VALUE of parent "
                             << parent_command << " to " << new_value);
    parent_command.Set(Am_VALUE, new_value);

    // set my parent to either the item's or panel's command object
    command_obj.Set(Am_IMPLEMENTATION_PARENT, parent_command);
  }
}

/******************************************************************************
 * Option Button
 *    single button, but pops up a menu of choices
 ******************************************************************************/

/******************************************************************************
 * option_button_items
 *   self is menu, window's for_item is main option button, get its ITEMS
 */

Am_Define_Formula(Am_Value, option_button_items)
{
  Am_Value value;
  value = 0;
  Am_Value v;
  Am_Object o;
  v = self.Peek(Am_WINDOW);
  if (v.Valid()) {
    o = v;
    v = o.Peek(Am_FOR_ITEM);
    if (v.Valid()) {
      o = v;
      value = o.Peek(Am_ITEMS);
    }
  }
  return value;
}

/******************************************************************************
 * Am_Option_Button_Sub_Window_Proto
 *   a prototype
 */

Am_Object Am_Option_Button_Sub_Window_Proto; // defined below

/******************************************************************************
 * create_sub_menu_proc
 */

int
create_sub_menu_proc(Am_Object &self)
{
  Am_Object new_window =
      self.Get(Am_SUB_MENU, Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR);
  if (new_window.Valid())
    Am_Error("create_sub_menu called but already has a menu");
  new_window =
      Am_Option_Button_Sub_Window_Proto.Create().Set(Am_FOR_ITEM, self);
  Am_Screen.Add_Part(new_window);
  self.Set(Am_SUB_MENU, new_window, Am_OK_IF_NOT_THERE);
  return -1;
}

/******************************************************************************
 * create_sub_menu
 */

Am_Formula create_sub_menu(create_sub_menu_proc, DSTR("create_sub_menu"));

/******************************************************************************
 * option_sub_win_top
 */

Am_Define_Formula(int, option_sub_win_top)
{
  Am_Object for_item = self.Get(Am_FOR_ITEM);
  bool fringe = for_item.Get(Am_LEAVE_ROOM_FOR_FRINGE),
       key_sel = for_item.Get(Am_KEY_SELECTED), def = for_item.Get(Am_DEFAULT);
  Am_Widget_Look look = for_item.Get(Am_WIDGET_LOOK);

  int border = calculate_button_fringe(look, fringe, key_sel, def) / 2;

  if (for_item.Valid()) {
    //find top of item in my menu that is currently selected
    int x = 0;
    int y = 0;
    Am_Value inter_value;
    inter_value = for_item.Peek(Am_COMPUTE_INTER_VALUE);
    if (inter_value.Valid()) {
      Am_Object sel_menu_item = inter_value;
      y = (int)sel_menu_item.Get(Am_TOP, Am_NO_DEPENDENCY);
    }
    // get the coordinates of the current item w.r.t. the screen
    Am_Translate_Coordinates(for_item, 0, -y + border + 1, Am_Screen, x, y);
    return y;
  } else
    return 0;
}

/******************************************************************************
 * option_sub_win_left
 */

Am_Define_Formula(int, option_sub_win_left)
{
  Am_Object for_item = self.Get(Am_FOR_ITEM);
  bool fringe = for_item.Get(Am_LEAVE_ROOM_FOR_FRINGE),
       key_sel = for_item.Get(Am_KEY_SELECTED), def = for_item.Get(Am_DEFAULT);
  Am_Widget_Look look = for_item.Get(Am_WIDGET_LOOK);

  int border = calculate_button_fringe(look, fringe, key_sel, def) / 2;

  if (for_item.Valid()) {
    int x = 0; //initializations so no compiler warnings for translate_coords
    int y = 0;
    // get the coordinates of the left of the menu item w.r.t. the screen
    Am_Translate_Coordinates(for_item, border, 0, Am_Screen, x, y);
    return x;
  } else
    return 0;
}

/******************************************************************************
 * get_value_to_use_for_value_from
 *  sets value with the value to use and returns true if active or false if not
*/

bool
get_value_to_use_for_value_from(Am_Value &value)
{
  if (value.type == Am_OBJECT) {
    Am_Object cmd = value;
    if (cmd.Is_Instance_Of(Am_Menu_Line_Command)) {
      value = true; // this takes care of the menu line case
      return false;
    } else if (cmd.Is_Instance_Of(Am_Command)) {
      // then get the value out of the command object
      value = cmd.Peek(Am_ID);
      if (!value.Valid())
        value = cmd.Peek(Am_LABEL);
      Am_Value active_value;
      active_value = cmd.Peek(Am_ACTIVE);
      return active_value.Valid();
    } else
      return true; //value is set with a non-command object, use it
  } else
    return true; //value is not an object, just use value as is
}

/******************************************************************************
 * option_button_value
 *   get the value out of my sub-menu, if any, otherwise use first active item
 */

Am_Define_Formula(Am_Value, option_button_value)
{
  Am_Value value;
  Am_Object sub_menu = self.Get_Object(Am_SUB_MENU).Get_Object(Am_SUB_MENU);
  value = sub_menu.Peek(Am_VALUE);
  if (!value.Valid()) {
    Am_Value v;
    v = self.Peek(Am_ITEMS);
    value = Am_No_Value;
    if (v.Valid() && Am_Value_List::Test(v)) {
      Am_Value_List items = v;
      Am_Value firstv;
      //search for a valid item
      items.Start();
      firstv = items.Get(); //save first item
      for (; !items.Last(); items.Next()) {
        value = items.Get();
        if (value.Valid()) {
          if (get_value_to_use_for_value_from(value))
            return value;
        }
      }
      //if get here, then all items are inactive, just use first one
      value = firstv;
      get_value_to_use_for_value_from(value);
    }
  }
  return value;
}

const int NOTCH_OFFSET = 8; //distance from edge of button to notch
const int NOTCH_WIDTH = 12;
const int NOTCH_HEIGHT = 8;

/******************************************************************************
 * option_button_width
 *   width of menu, plus notch offset plus notch size
 */

Am_Define_Formula(int, option_button_width)
{
  Am_Object sub_menu_win = self.Get(Am_SUB_MENU);
  int menu_width = sub_menu_win.Get(Am_WIDTH);
  Am_Widget_Look look = self.Get(Am_WIDGET_LOOK);
  switch (look.value) {
  case Am_MOTIF_LOOK_val:
    return menu_width + NOTCH_OFFSET + NOTCH_WIDTH + NOTCH_OFFSET;

  case Am_WINDOWS_LOOK_val:
    return menu_width + 16;

  case Am_MACINTOSH_LOOK_val:
    return menu_width + 23;

  default:
    Am_Error("Unknown Look parameter");
    break;
  }
  return 20;
}

/******************************************************************************
 * option_button_draw
 */

Am_Define_Method(Am_Draw_Method, void, option_button_draw,
                 (Am_Object self, Am_Drawonable *draw, int x_offset,
                  int y_offset))
{
  int left = (int)self.Get(Am_LEFT) + x_offset;
  int top = (int)self.Get(Am_TOP) + y_offset;
  int width = self.Get(Am_WIDTH);
  int height = self.Get(Am_HEIGHT);
  bool active = self.Get(Am_ACTIVE);
  bool key_selected = self.Get(Am_KEY_SELECTED);
  bool fringe = self.Get(Am_LEAVE_ROOM_FOR_FRINGE);
  Am_Font font;
  font = self.Get(Am_FONT);
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
    Am_ERRORO("String slot of widget "
                  << self << " should have string or object type, but value is "
                  << value,
              self, 0);

  int offset = self.Get(Am_MENU_ITEM_LEFT_OFFSET);

  // finally ready to draw it
  switch (look.value) {
  case Am_MOTIF_LOOK_val: {
    Am_Draw_Button_Widget(left, top, width, height, string, obj, false, false,
                          active, key_selected, false, fringe, font, color_rec,
                          look, Am_PUSH_BUTTON, draw, 0, 0, false,
                          Am_LEFT_ALIGN, offset);

    //now draw notch
    int x = left + width - NOTCH_OFFSET - NOTCH_WIDTH - 4;
    //center in Y
    int y = top + (height - NOTCH_HEIGHT) / 2;
    draw->Push_Clip(left, top, width, height);
    Am_Draw_Motif_Box(x, y, NOTCH_WIDTH, NOTCH_HEIGHT, false, color_rec, draw);
    draw->Pop_Clip();
    break;
  }

  case Am_WINDOWS_LOOK_val:
    Am_Draw_Button_Widget(left, top, width, height, string, obj, false, false,
                          active, key_selected, false, fringe, font, color_rec,
                          look, Am_PUSH_BUTTON, draw, 0, 0, false,
                          Am_LEFT_ALIGN, offset);
    draw_down_arrow(left + width - 16, top, 16, height, Am_WINDOWS_LOOK, false,
                    true, true, color_rec, draw);
    break;

  case Am_MACINTOSH_LOOK_val: {
    if (fringe) {
      left += 4;
      top += 4;
      width -= 8;
      height -= 8;
    }

    draw->Draw_Rectangle(Am_Black, Am_No_Style, left + 3, top + 3, width - 3,
                         height - 3);
    draw->Draw_Rectangle(Am_Black, Am_White, left, top, width - 1, height - 1);

    left += 1;
    top += 1;
    width -= 3;
    height -= 3;

    Am_Style style = active ? Am_Black : Am_Motif_Inactive_Stipple;

    if ((const char *)string) {
      int str_width, ascent, descent, a, b, str_left, str_top;

      draw->Get_String_Extents(font, string, strlen(string), str_width, ascent,
                               descent, a, b);

      str_left = offset + left;
      str_top = top + (height - ascent - descent) / 2;

      draw->Draw_Text(style, string, strlen(string), font, str_left, str_top);

    } else if (obj.Valid()) {
      int obj_width = obj.Get(Am_WIDTH), obj_height = obj.Get(Am_HEIGHT);
      int obj_left = obj.Get(Am_LEFT), obj_top = obj.Get(Am_TOP);
      int final_left = (width - 20 - obj_width) / 2,
          final_top = 2 + (height - obj_height) / 2;

      Am_Draw(obj, draw, left - obj_left + final_left,
              top - obj_top + final_top);
    }

    // draw little down arrow
    int tri_x = left + width - 15, tri_y = top + (height - 6) / 2,
        len =
            10; // len - is really 11, but one extra pixel is drawn to the right
    for (register long i = 1; i <= 6; i++) {
      draw->Draw_Line(style, tri_x, tri_y, tri_x + len, tri_y);
      tri_x++;
      tri_y++;
      len -= 2;
    }
    break;
  }

  default:
    Am_Error("Unknown Look parameter");
    break;
  }
}

/******************************************************************************
 * option_button_start_do
 *   displays sub-menu and starts its interactor
 */

Am_Define_Method(Am_Object_Method, void, option_button_start_do,
                 (Am_Object inter))
{
  Am_Object option_button = inter.Get_Owner();
  Am_Object sub_menu_window = option_button.Get(Am_SUB_MENU);
  if (sub_menu_window.Valid()) {
    Am_Object main_win = option_button.Get(Am_WINDOW);
    set_popup_win_visible(sub_menu_window, true, main_win);
    Am_Object inter =
        sub_menu_window.Get_Object(Am_SUB_MENU).Get_Object(Am_INTERACTOR);
    Am_Value inter_value;
    inter_value = option_button.Peek(Am_COMPUTE_INTER_VALUE);
    Am_Start_Interactor(inter, inter_value);
  }
}

/******************************************************************************
 * hide_sub_menu
 */

void
hide_sub_menu(Am_Object command_obj)
{
  Am_Object inter = command_obj.Get_Owner(); // owner will be interactor
  if (inter.Valid()) {
    Am_Object window = inter.Get_Owner().Get(Am_WINDOW);
    if (window.Valid())
      set_popup_win_visible(window, false);
  }
}

/******************************************************************************
 * option_sub_menu_do
 */

Am_Define_Method(Am_Object_Method, void, option_sub_menu_do,
                 (Am_Object command_obj))
{
  //standard stuff
  Am_Inter_For_Panel_Do_proc(command_obj);
  //make my menu disappear
  hide_sub_menu(command_obj);
}

/******************************************************************************
 * option_hide_sub_menu
 */

Am_Define_Method(Am_Object_Method, void, option_hide_sub_menu,
                 (Am_Object command_obj))
{
  hide_sub_menu(command_obj);
}

/******************************************************************************
 * command_from_for_item
 */

Am_Define_Object_Formula(command_from_for_item)
{
  return self.Get_Owner().Get_Object(Am_FOR_ITEM).Get(Am_COMMAND);
}

/******************************************************************************
 * compute_inter_value
 */

Am_Define_Formula(Am_Value, compute_inter_value)
{
  Am_Value value;
  Am_Value panel_value;
  Am_Object panel = self.Get_Object(Am_SUB_MENU).Get_Object(Am_SUB_MENU);
  panel_value = self.Peek(Am_VALUE);
  if (!panel.Valid()) {
    value = (0L);
    return value;
  }
  panel.Set(Am_VALUE, panel_value); //set panel based on value set into me

  get_inter_value_from_panel_value(panel_value, panel, value);

  return value;
}

/******************************************************************************
 * get_real_string_from_inter_val
 */

Am_Define_Formula(Am_Value, get_real_string_from_inter_val)
{
  Am_Value value;
  Am_Value v;
  v = self.Peek(Am_COMPUTE_INTER_VALUE);
  if (v.Valid()) {
    Am_Object button = v;
    value = button.Peek(Am_REAL_STRING_OR_OBJ);
  }
  return value;
}

/******************************************************************************
 * option_button_window_list
 */

Am_Define_Value_List_Formula(option_button_window_list)
{
  Am_Value_List window_list;
  Am_Object menu = self.Get_Owner();
  if (menu.Valid()) {
    Am_Object menu_win = menu.Get(Am_WINDOW);
    if (menu_win.Valid()) {
      Am_Object option_button = menu_win.Get_Object(Am_FOR_ITEM);
      if (option_button.Valid()) {
        Am_Object option_button_window = option_button.Get(Am_WINDOW);
        if (option_button_window.Valid())
          window_list.Add(menu_win).Add(option_button_window);
      }
    }
  }
  return window_list;
}

/******************************************************************************
 * option_sub_menu_set_old_owner
 */

Am_Define_Formula(int, option_sub_menu_set_old_owner)
{
  Am_Object window, option_widget;
  window = self.Get_Owner();
  int ret = 0;
  if (window.Valid()) {
    option_widget = window.Get(Am_FOR_ITEM);
    if (option_widget.Valid()) {
      Am_Value_List parts;
      parts = self.Get(Am_GRAPHICAL_PARTS);
      ret = set_parts_list_commands_old_owner(parts, option_widget);
    }
  }
  return ret;
}

/******************************************************************************
 * option_button_abort_method
 */

Am_Define_Method(Am_Object_Method, void, option_button_abort_method,
                 (Am_Object widget))
{
  //just abort the sub-menu widget.  My value has a constraint to the sub-menu
  //so sub-menu's abort will make my value be correct
  Am_Object sub_menu_widget =
      widget.Get_Object(Am_SUB_MENU).Get_Object(Am_SUB_MENU);
  Am_Abort_Widget(sub_menu_widget);
}

/******************************************************************************
 *   A custom destroy demon for option button to destroy the menu popup window.
 */

void
destroy_option_button(Am_Object object)
{
  Am_Object sub_menu = object.Get(Am_SUB_MENU);
  sub_menu.Destroy();

  //now run the regular button destroy demon, if any
  Am_Object_Demon *proto_demon = ((Am_Object_Advanced &)Am_Button)
                                     .Get_Demons()
                                     .Get_Object_Demon(Am_DESTROY_OBJ);
  if (proto_demon)
    proto_demon(object);
}

static void
init()
{
  Am_Object inter;            // interactor in the widget
  Am_Object_Advanced obj_adv; // to get at advanced features like
                              // local-only and demons.

  ///////////////////////////////////////////////////////////////////////////
  // Option button: single button that pops up a menu
  ///////////////////////////////////////////////////////////////////////////

  // can't be a part of option button since needs to be stand-alone
  // window at the top level

  Am_Option_Button_Sub_Window_Proto =
      Am_Pop_Up_Menu_From_Widget_Proto.Create(DSTR("Option_Sub_Window"))
          .Set(Am_FOR_ITEM, (0L))
          .Set(Am_LEFT, option_sub_win_left)
          .Set(Am_TOP, option_sub_win_top)
          .Set(Am_VISIBLE, false);
  Am_Object sub_menu_in_option_button =
      Am_Option_Button_Sub_Window_Proto.Get_Object(Am_SUB_MENU);
  sub_menu_in_option_button.Set(Am_ITEMS, option_button_items)
      .Set_Name(DSTR("sub_menu_in_option_button"))
      .Set(Am_SET_COMMAND_OLD_OWNER, option_sub_menu_set_old_owner)
      .Add_Part(Am_INTERACTOR,
                inter = Am_Button_Panel.Get_Object(Am_INTERACTOR)
                            .Create(DSTR("Inter_In_Option_PopUp")));

  inter.Set(Am_MULTI_OWNERS, option_button_window_list);
  inter.Get_Object(Am_COMMAND)
      .Set(Am_DO_METHOD, option_sub_menu_do)
      .Set(Am_ABORT_DO_METHOD, option_hide_sub_menu);
  sub_menu_in_option_button.Remove_Part(Am_COMMAND);
  sub_menu_in_option_button.Add(Am_COMMAND, command_from_for_item)
      .Set_Inherit_Rule(Am_COMMAND, Am_INHERIT);

  // like menubars
  Am_Option_Button =
      Am_Button.Create(DSTR("Option_Button"))
          .Add(Am_ITEMS, 0)
          .Set(Am_WIDTH, option_button_width)
          .Set(Am_HEIGHT, button_height)
          .Set(Am_DRAW_METHOD, option_button_draw)
          .Add(Am_FOR_ITEM, create_sub_menu)
          .Add(Am_SUB_MENU, 0)
          // need Am_MENU_ITEM_LEFT_OFFSET for drawing the text
          .Add(Am_MENU_ITEM_LEFT_OFFSET, menu_item_left_offset)
          .Set_Inherit_Rule(Am_SUB_MENU, Am_LOCAL)
          .Add(Am_COMPUTE_INTER_VALUE, compute_inter_value)
          .Set(Am_REAL_STRING_OR_OBJ, get_real_string_from_inter_val)
          .Set(Am_WIDGET_ABORT_METHOD, option_button_abort_method);

  obj_adv = (Am_Object_Advanced &)Am_Option_Button;
  Am_Demon_Set demons = obj_adv.Get_Demons().Copy();
  demons.Set_Object_Demon(Am_DESTROY_OBJ, destroy_option_button);
  obj_adv.Set_Demons(demons);

  inter = Am_Option_Button.Get_Object(Am_INTERACTOR)
              .Set(Am_CONTINUOUS, false)
              .Set(Am_DO_METHOD, option_button_start_do)
              .Set_Name(DSTR("inter_in_option_button"));
  //no command in interactor, instead use command in interactor of the menu
  inter.Set(Am_COMMAND, 0);
  inter.Get_Object(Am_IMPLEMENTATION_COMMAND)
      .Set(Am_IMPLEMENTATION_PARENT, Am_NOT_USUALLY_UNDONE);

  //do this last so it doesn't trigger the warning about illegal values
  Am_Option_Button.Set(Am_VALUE, option_button_value.Multi_Constraint());
}

static void
cleanup()
{
  Am_Option_Button.Destroy();
}

static Am_Initializer *initializer =
    new Am_Initializer(DSTR("Am_Option_Button"), init, 5.2090f, 125, cleanup);
