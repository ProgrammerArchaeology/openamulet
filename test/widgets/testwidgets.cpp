/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <am_inc.h>

#include <amulet/standard_slots.h>
#include <amulet/value_list.h>

#include <amulet/widgets.h>
#include <amulet/debugger.h>
#include <amulet/registry.h>
#include <amulet/misc.h>
#include <amulet/widgets_advanced.h> //**TEMP for accelerator_inter
#include <amulet/am_strstream.h> // strstream is needed for AM_POP_UP_ERROR_WINDOW which is tested below

Am_Slot_Key R0 = Am_Register_Slot_Name("R0");
Am_Slot_Key R1 = Am_Register_Slot_Name("R1");
Am_Slot_Key R2 = Am_Register_Slot_Name("R2");
Am_Slot_Key R3 = Am_Register_Slot_Name("R3");
Am_Slot_Key R4 = Am_Register_Slot_Name("R4");
Am_Slot_Key R5 = Am_Register_Slot_Name("R5");
Am_Slot_Key R6 = Am_Register_Slot_Name("R6");
Am_Slot_Key SAME_AS = Am_Register_Slot_Name("SAME_AS");

Am_Font bold_san_font(Am_FONT_SANS_SERIF, true, false, false, Am_FONT_LARGE);

Am_Object window;
Am_Object button1;
Am_Object abort_button;
Am_Object button4;
Am_Object button6;
Am_Object button8;
Am_Object button_panel;
Am_Object button_panel2;
Am_Object button_panel3;
Am_Object checkboxes;
Am_Object menu1;
Am_Object cmd4;
Am_Object rect_in_button;
Am_Object rect2_in_button;
Am_Object rect3_in_button;
Am_Object rect;
Am_Object undo_handler;
Am_Object button_panel_cmd1;
Am_Object button_panel_rect1;
Am_Object button_panel_obj2;
Am_Object vscroll;
Am_Object hscroll;
Am_Object scrolling_window;
Am_Object scrolling_group;
Am_Object h_scroll_on_top, v_scroll_on_left, has_v_scroll, has_h_scroll;
Am_Object menu_bar;
Am_Object textinputwidget;
Am_Object textinputwidget2;
Am_Object rfeedback;
Am_Object lfeedback;
Am_Style Amulet_Dark_Purple;
Am_Object amulet_icon;
Am_Object new_other_command;
Am_Object option_button;
Am_Object cy_cmd;
Am_Object number_input_widget;
Am_Object scroll_menu;

Am_Define_Method(Am_Object_Method, void, my_do, (Am_Object cmd))
{
  Am_Value value;
  value = cmd.Peek(Am_VALUE);
  std::cout << "\n+-+-+- Command " << cmd << " value = " << value << " type ";
  Am_Print_Type(std::cout, value.type);
  std::cout << " Saved_Old_Owner=" << cmd.Peek(Am_SAVED_OLD_OWNER) << std::endl
            << std::flush;
}

Am_Define_Formula(bool, look_is_me)
{
  Am_Value my_look = self.Get(Am_WIDGET_LOOK);
  Am_Value current_look = Am_Screen.Get(Am_WIDGET_LOOK);
  return my_look == current_look;
}

Am_Define_Method(Am_Object_Method, void, set_look, (Am_Object cmd))
{
  Am_Value my_look = cmd.Get(Am_WIDGET_LOOK);
  std::cout << "Setting look to " << my_look << std::endl << std::flush;
  Am_Set_Default_Look(my_look);
}

Am_Define_Formula(Am_Value, same_as_sameas)
{
  Am_Value value;
  Am_Object other = self.Get(SAME_AS);
  value = other.Peek(Am_VALUE);
  return value;
}

#define ON_RIGHT 1
#define FIXED_WIDTH 2
#define FIXED_HEIGHT 3

bool
find_in_list(Am_Value list_value, int ID)
{
  if (!list_value.Valid())
    return false;                  // nothing selected
  Am_Value_List list = list_value; // otherwise it is a value list
  list.Start();
  return list.Member(ID);
}

Am_Define_No_Self_Formula(Am_Wrapper *, value_from_cycle_command)
{
  Am_Value valv;
  valv = cy_cmd.Peek(Am_VALUE);
  int val = 0;
  if (valv.type == Am_INT)
    val = valv;
  Am_String s;
  Am_Value v;
  v = cy_cmd.Peek(Am_LABEL);
  std::cout << "New value for command " << cy_cmd << " is " << valv
            << " label is " << v << std::endl
            << std::flush;
  if (v.type == Am_STRING)
    s = v;
  else
    s = "NOT TEXT";
  return s;
}

Am_Define_No_Self_Formula(bool, on_left_form)
{
  Am_Value value;
  // get the value from the checkbox widget's value.
  value = checkboxes.Peek(Am_VALUE);
  return !find_in_list(value, ON_RIGHT);
}

Am_Define_No_Self_Formula(bool, fixed_height_form)
{
  Am_Value value;
  // get the value from the checkbox widget's value.
  value = checkboxes.Peek(Am_VALUE);
  return find_in_list(value, FIXED_HEIGHT);
}

Am_Define_No_Self_Formula(bool, fixed_width_form)
{
  Am_Value value;
  // get the value from the checkbox widget's value.
  value = checkboxes.Peek(Am_VALUE);
  return find_in_list(value, FIXED_WIDTH);
}

void
Undo_Redo_Selective(Am_Slot_Key allowed_slot, Am_Slot_Key method_slot,
                    const char *prompt_str)
{
  Am_Object undo_handler, last_command;
  undo_handler = window.Get(Am_UNDO_HANDLER);
  Am_Value_List l;
  l = undo_handler.Get(Am_COMMAND);
  Am_Value v;
  Am_Object cmd, new_cmd;
  Am_Selective_Allowed_Method allowed_method;
  Am_Value current_selection, obj_modified;
  allowed_method = undo_handler.Get(allowed_slot);

  bool allowed;
  int cnt;
  for (l.Start(), cnt = 0; !l.Last(); l.Next(), cnt++) {
    cmd = l.Get();
    v = cmd.Peek(Am_LABEL);
    obj_modified = cmd.Peek(Am_OBJECT_MODIFIED);
    std::cout << cnt << " " << v << " on " << obj_modified << " (cmd = " << cmd
              << ") ";
    allowed = allowed_method.Call(cmd);
    if (allowed)
      std::cout << "OK\n";
    else
      std::cout << "NOT OK\n";
  }
  std::cout << "--Type index of command to " << prompt_str
            << " (or -1 to exit): " << std::flush;
  int which;
  std::cin >> which;
  if (which < 0)
    return;
  for (l.Start(), cnt = 0; cnt < which; l.Next(), cnt++)
    ;
  cmd = l.Get();
  Am_Handler_Selective_Undo_Method method;
  method = undo_handler.Get(method_slot);
  std::cout << prompt_str << " on cmd " << cmd << " method = " << method
            << std::endl
            << std::flush;
  new_cmd = method.Call(undo_handler, cmd);
  std::cout << "  new_cmd = " << new_cmd << std::endl << std::flush;
}

Am_Define_Method(Am_Object_Method, void, do_undo, (Am_Object /*cmd*/))
{
  Am_Object last_command;
  last_command = undo_handler.Get(Am_UNDO_ALLOWED);
  if (last_command.Valid()) {
    Am_Value v;
    v = last_command.Peek(Am_LABEL);
    std::cout << " undoing cmd " << last_command << " = " << v << std::endl
              << std::flush;
    Am_Object_Method method;
    method = undo_handler.Get(Am_PERFORM_UNDO);
    method.Call(undo_handler);
  } else
    std::cout << " nothing to undo\n" << std::flush;
}

Am_Define_Method(Am_Object_Method, void, do_redo, (Am_Object /*cmd*/))
{
  Am_Object last_command;
  last_command = undo_handler.Get(Am_REDO_ALLOWED);
  if (last_command.Valid()) {
    Am_Value v;
    v = last_command.Peek(Am_LABEL);
    std::cout << " re-doing cmd " << last_command << " = " << v << std::endl
              << std::flush;
    Am_Object_Method method;
    method = undo_handler.Get(Am_PERFORM_REDO);
    method.Call(undo_handler);
  } else
    std::cout << " nothing to redo\n" << std::flush;
}

Am_Text_Abort_Or_Stop_Code cur_code = Am_TEXT_ABORT_AND_RESTORE;

Am_Define_Method(Am_Text_Check_Legal_Method, Am_Text_Abort_Or_Stop_Code,
                 check_num_method, (Am_Object & text, Am_Object &inter))
{
  Am_String str = text.Get(Am_TEXT);
  char *s = str;
  int len = strlen(s);
  char *ptr;
  long i = strtol(s, &ptr, 10);
  std::cout << "** Checking string = `" << s << "' len = " << len
            << " int value = " << i << std::endl
            << std::flush;
  if (ptr < s + len) {
    Am_Beep(window);
    std::cout << "** ptr " << (void *)ptr << " less than s+len "
              << (void *)(s + len) << " returning " << (int)cur_code
              << std::flush;
    return cur_code;
  } else {
    Am_Object widget = inter.Get_Owner();
    std::cout << "Setting value of widget " << widget << " with " << i
              << std::endl
              << std::flush;
    widget.Set(Am_VALUE, i);
    return Am_TEXT_OK;
  }
}

Am_Define_Method(Am_Object_Method, void, change_setting, (Am_Object cmd))
{
  Am_Object inter = cmd.Get_Owner();
  Am_Input_Char c = inter.Get(Am_START_CHAR);

  std::cout << "---- got " << c << std::endl;
  switch (c.As_Char()) {
  case '1': //test value printing
  {
    Am_Value v1 = 15;
    Am_Value v2 = "This is a string";
    Am_Value v3 = "This is a string";
    Am_Value v4 = 9.0;
    std::cout << "v1= " << v1 << std::endl << std::flush;
    std::cout << "v2= " << v2 << std::endl << std::flush;
    std::cout << "v3= " << v3 << std::endl << std::flush;
    std::cout << "v4= " << v4 << std::endl << std::flush;
    std::cout << "v1==v2 = " << (v1 == v2) << std::endl << std::flush;
    std::cout << "v2==v3(true) = " << (v2 == v3) << std::endl << std::flush;
    break;
  }

  case 'b': {
    bool sel = rect.Get(Am_SELECTED);
    rect.Set(Am_SELECTED, !sel);
    std::cout << " setting selected to " << !sel << std::endl << std::flush;
    break;
  }
  case 'K': {
    switch (cur_code) {
    case Am_TEXT_ABORT_AND_RESTORE:
      std::cout
          << "Code for when text edit not an int now= Am_TEXT_KEEP_RUNNING\n"
          << std::flush;
      cur_code = Am_TEXT_KEEP_RUNNING;
      break;
    case Am_TEXT_KEEP_RUNNING:
      std::cout
          << "Code for when text edit not an int now= Am_TEXT_STOP_ANYWAY\n"
          << std::flush;
      cur_code = Am_TEXT_STOP_ANYWAY;
      break;
    default:
      std::cout << "Code for when text edit not an int now= "
                   "Am_TEXT_ABORT_AND_RESTORE\n"
                << std::flush;
      cur_code = Am_TEXT_ABORT_AND_RESTORE;
      break;
    }
    break;
  }

  case 'c': {
    static int new_object_cnt = 1;
    Am_Style color;
    const char *colorstr = 0;
    switch (new_object_cnt) {
    case 1:
      color = Am_Motif_Light_Orange;
      colorstr = "Motif_Light_Orange";
      break;
    case 2:
      color = Am_Motif_Gray;
      colorstr = "Motif_Gray";
      break;
    case 3:
      color = Am_Motif_Light_Gray;
      colorstr = "Motif_Light_Gray";
      break;
    case 4:
      color = Am_Motif_Blue;
      colorstr = "Motif_Blue";
      break;
    case 5:
      color = Am_Amulet_Purple;
      colorstr = "Amulet Purple";
      break;
    case 6:
      color = Am_Motif_Green;
      colorstr = "Motif_Green";
      break;
    case 7:
      color = Am_Motif_Light_Green;
      colorstr = "Motif_Light_Green";
      break;
    case 8:
      color = Am_Motif_Orange;
      colorstr = "Motif_Orange";
      break;
    case 9:
      color = Am_Black;
      colorstr = "Black";
      break;
    case 10:
      color = Am_White;
      colorstr = "White";
      break;
    case 11:
      color = Am_Yellow;
      colorstr = "Yellow";
      break;
    case 12:
      color = Am_Purple;
      colorstr = "Purple";
      break;
    case 13:
      color = Am_Blue;
      colorstr = "Blue";
      break;
    case 14:
      color = Am_Red;
      colorstr = "Red";
      break;
    }
    new_object_cnt = (new_object_cnt % 14) + 1;
    std::cout << " setting color to " << colorstr << std::endl << std::flush;
    rect.Set(Am_FILL_STYLE, color);
    button_panel.Set(Am_FILL_STYLE, color);
    button_panel2.Set(Am_FILL_STYLE, color);
    menu_bar.Set(Am_FILL_STYLE, color);
    textinputwidget.Set(Am_FILL_STYLE, color);
    option_button.Set(Am_FILL_STYLE, color);
    vscroll.Set(Am_FILL_STYLE, color);
    menu1.Set(Am_FILL_STYLE, color);
    button_panel3.Set(Am_FILL_STYLE, color);
    break;
  }
  case 'F': {
    static int which_font = 1;
    if (which_font == 1) {
      std::cout << "Setting font to bold, big\n" << std::flush;
      button_panel.Set(Am_FONT, bold_san_font);
      button_panel2.Set(Am_FONT, bold_san_font);
      menu_bar.Set(Am_FONT, bold_san_font);
      textinputwidget.Set(Am_FONT, bold_san_font);
      which_font = 2;
    } else {
      std::cout << "Setting font to regular\n" << std::flush;
      button_panel.Set(Am_FONT, Am_Default_Font);
      button_panel2.Set(Am_FONT, Am_Default_Font);
      menu_bar.Set(Am_FONT, Am_Default_Font);
      textinputwidget.Set(Am_FONT, Am_Default_Font);
      which_font = 1;
    }
    break;
  }
  case 's': {
    static int new_object_cnt = 1;
    switch (new_object_cnt) {
    case 1:
      button1.Set(Am_WIDTH, 100);
      button1.Set(Am_HEIGHT, 60);
      break;
    case 2:
      button1.Set(Am_WIDTH, 15);
      button1.Set(Am_HEIGHT, 60);
      break;
    case 3:
      button1.Get_Object(Am_COMMAND).Set(Am_LABEL, "New Label");
      break;
    }
    new_object_cnt = (new_object_cnt % 3) + 1;
    break;
  }
  case 'a': {
    bool active = button1.Get(Am_ACTIVE);
    std::cout << "Setting active to " << !active << std::endl << std::flush;
    button1.Get_Object(Am_COMMAND).Set(Am_ACTIVE, !active);
    textinputwidget.Get_Object(Am_COMMAND).Set(Am_ACTIVE, !active);
    vscroll.Get_Object(Am_COMMAND).Set(Am_ACTIVE, !active);
    break;
  }
  case 'A': {
    bool active = button1.Get(Am_ACTIVE_2);
    std::cout << "Setting active2 to " << !active << std::endl << std::flush;
    button1.Set(Am_ACTIVE_2, !active);
    textinputwidget.Set(Am_ACTIVE_2, !active);
    break;
  }
  case 'k': {
    bool key_sel = button1.Get(Am_KEY_SELECTED);
    std::cout << "Setting Am_KEY_SELECTED to " << !key_sel << std::endl
              << std::flush;
    button1.Set(Am_KEY_SELECTED, !key_sel);
    vscroll.Set(Am_KEY_SELECTED, !key_sel);
    textinputwidget.Set(Am_KEY_SELECTED, !key_sel);
    break;
  }
  case 'f': {
    bool active = button1.Get(Am_FINAL_FEEDBACK_WANTED);
    std::cout << "Setting Am_FINAL_FEEDBACK_WANTED to " << !active << std::endl
              << std::flush;
    button1.Set(Am_FINAL_FEEDBACK_WANTED, !active);
    abort_button.Set(Am_FINAL_FEEDBACK_WANTED, !active);
    button4.Set(Am_FINAL_FEEDBACK_WANTED, !active);
    button6.Set(Am_FINAL_FEEDBACK_WANTED, !active);
    button_panel.Set(Am_FINAL_FEEDBACK_WANTED, !active);
    button_panel2.Set(Am_FINAL_FEEDBACK_WANTED, !active);
    menu1.Set(Am_FINAL_FEEDBACK_WANTED, !active);
    break;
  }
  case 'v': {
    Am_Value value;
    value = button4.Peek(Am_VALUE);
    std::cout << "Button4 value type = " << value.type << " " << value
              << std::endl;
    bool new_value = !value.Valid();
    std::cout << "  Setting to be " << new_value << std::endl << std::flush;
    button4.Set(Am_VALUE, new_value);
    break;
  }
  case 'V': {
    Am_Value value;
    value = button1.Peek(Am_VALUE);
    std::cout << button1 << " value type = " << value.type << " " << value
              << std::endl;
    bool new_value = !value.Valid();
    std::cout << "  Setting to be " << new_value << std::endl << std::flush;
    button1.Set(Am_VALUE, new_value);
    break;
  }

  case 'p': {
    static int panel_contents = 1;
    switch (panel_contents) {
    case 1: {
      Am_Value v;
      v = button_panel_cmd1.Peek(Am_LABEL);
      std::cout << " Changing " << button_panel2 << " to cmd1, value = " << v
                << std::endl
                << std::flush;
      button_panel2.Set(Am_VALUE, v);
      break;
    }
    case 2:
      std::cout << " Changing button_panel2 to rect1\n" << std::flush;
      button_panel2.Set(Am_VALUE, button_panel_rect1);
      break;
    case 3:
      std::cout << " Changing button_panel2 to id=15 (blue rect)\n"
                << std::flush;
      button_panel2.Set(Am_VALUE, 15);
      break;
    case 4: {
      Am_Choice_How_Set how_set = button_panel2.Get(Am_HOW_SET);
      if (how_set == Am_CHOICE_LIST_TOGGLE)
        how_set = Am_CHOICE_SET;
      else
        how_set = Am_CHOICE_LIST_TOGGLE;
      std::cout << " Changing panel to want multiple selections = "
                << (how_set == Am_CHOICE_LIST_TOGGLE) << std::endl
                << std::flush;
      button_panel2.Set(Am_HOW_SET, how_set);
      break;
    }
    }
    panel_contents = (panel_contents % 4) + 1;
    break;
  }
  case 'P': {
    Am_Object top_cmd = button_panel.Get_Object(Am_COMMAND);
    std::cout << "Top command = " << top_cmd << std::endl << std::flush;
    static int panel_active = 1;
    switch (panel_active) {
    case 1:
      std::cout << " Setting whole panel1 NOT Active\n" << std::flush;
      top_cmd.Set(Am_ACTIVE, false);
      break;
    case 2:
      std::cout << " Setting whole panel1 active but NOT Active2\n"
                << std::flush;
      top_cmd.Set(Am_ACTIVE, true);
      button_panel.Set(Am_ACTIVE_2, false);
      break;
    case 3:
      std::cout << " Changing item2 to be not active\n" << std::flush;
      button_panel.Set(Am_ACTIVE_2, true);
      button_panel_obj2.Set(Am_ACTIVE, false);
      break;
    case 4:
      std::cout << " All active\n" << std::flush;
      button_panel_obj2.Set(Am_ACTIVE, true);
      break;
    }
    panel_active = (panel_active % 4) + 1;
    break;
  }
  case 'D': {
    static bool want_pending_delete = true;
    want_pending_delete = !want_pending_delete;
    std::cout << "Setting Am_WANT_PENDING_DELETE to " << want_pending_delete
              << std::endl
              << std::flush;
    textinputwidget.Set(Am_WANT_PENDING_DELETE, want_pending_delete);
    textinputwidget2.Set(Am_WANT_PENDING_DELETE, want_pending_delete);
    break;
  }
  case 'd': {
    bool isDefault = (bool)button1.Get(Am_DEFAULT);
    button1.Get_Object(Am_COMMAND).Set(Am_DEFAULT, (bool)!isDefault);
    std::cout << "Setting widgets to " << (!isDefault ? "" : "not ")
              << "default." << std::endl;
    break;
  }
  /* Not implemented in V3.0 -> will use the type mechanism
  case 'e': {
    static int widget_warning_cnt = 1;
    switch (widget_warning_cnt) {
    case 1: {
      std::cout << "Setting " << checkboxes << " with 0 (no warning)\n" << std::flush;
      checkboxes.Set(Am_VALUE, 0);
      break;
    }
    case 2: {
      std::cout << "Setting " << checkboxes << " with (4 5) (warning)\n" << std::flush;
      checkboxes.Set(Am_VALUE, Am_Value_List().Add(4).Add(5));
      break;
    }
    case 3: {
      std::cout << "Setting " << checkboxes << " with ('Boxes') (warning)\n"<<std::flush;
      checkboxes.Set(Am_VALUE, Am_Value_List().Add("Boxes on right"));
      break;
    }
    case 4: {
      std::cout << "Setting " << checkboxes << " with (3) (no warning)\n"<<std::flush;
      checkboxes.Set(Am_VALUE, Am_Value_List().Add(3));
      break;
    }
    case 5: {
      std::cout << "Setting " << checkboxes << " with (3 0) (warning)\n"<<std::flush;
      checkboxes.Set(Am_VALUE, Am_Value_List().Add(3).Add(0));
      break;
    }
    case 6: {
      std::cout << "Setting " << button_panel3 << " with 0 (no warning)\n" << std::flush;
      button_panel3.Set(Am_VALUE, 0);
      break;
    }
    case 7: {
      std::cout << "Setting " << button_panel3 << " with Red (no warning)\n"<<std::flush;
      button_panel3.Set(Am_VALUE, "Red");
      break;
    }
    case 8: {
      std::cout << "Setting " << button_panel3 << " with black (warning)\n"<<std::flush;
      button_panel3.Set(Am_VALUE, "Black");
      break;
    }
    }//switch
    widget_warning_cnt = (widget_warning_cnt % 8) + 1;
    break;
  }
*/
  case 'm': {
    Am_Object cmd;
    Am_Value v;
    static int menu_bar_cnt = 1;
    Am_Value_List value_list, sub_list;
    value_list = menu_bar.Get(Am_ITEMS);
    value_list.Start(); //at Amulet
    value_list.Next();  //at file
    switch (menu_bar_cnt) {
    case 1: {
      std::cout << " Disabling Save\n" << std::flush;
      cmd = value_list.Get();       //file's command
      sub_list = cmd.Get(Am_ITEMS); //file's sub-menu
      sub_list.Start();             //at open
      sub_list.Next();              //at save as
      sub_list.Next();              //at save
      cmd = sub_list.Get();
      std::cout << "  Setting the active slot of " << cmd << std::endl
                << std::flush;
      cmd.Set(Am_ACTIVE, false);
      break;
    }
    case 2: {
      std::cout << " Enabling Save\n" << std::flush;
      cmd = value_list.Get();         //file's command
      value_list = cmd.Get(Am_ITEMS); //file's sub-menu
      value_list.Start();             //at open
      value_list.Next();              //at save as
      value_list.Next();              //at save
      cmd = value_list.Get();
      std::cout << "  Setting the active slot of " << cmd << std::endl
                << std::flush;
      cmd.Set(Am_ACTIVE, true);
      break;
    }
    case 3: {
      std::cout << " Disabling Whole File Menu\n" << std::flush;
      cmd = value_list.Get(); //file's command
      std::cout << "  Setting the active slot of " << cmd << std::endl
                << std::flush;
      cmd.Set(Am_ACTIVE, false);
      break;
    }
    case 4: {
      std::cout << " Enabling File\n" << std::flush;
      cmd = value_list.Get(); //file's command
      std::cout << "  Setting the active slot of " << cmd << std::endl
                << std::flush;
      cmd.Set(Am_ACTIVE, true);
      break;
    }
    case 5: {
      std::cout << " Adding new item to Other\n" << std::flush;
      value_list.Next();            // should be edit cmd
      value_list.Next();            // should be other cmd
      cmd = value_list.Get();       //other's cmd
      sub_list = cmd.Get(Am_ITEMS); //other's sub-menu
      sub_list.Start();             //at label3.1
      //false because destructive modification
      sub_list.Insert(new_other_command.Create(), Am_AFTER, false);
      cmd.Note_Changed(Am_ITEMS);
      break;
    }
    case 6: {
      std::cout << " Adding ANOTHER new item to Other\n" << std::flush;
      value_list.Next();            // should be edit cmd
      value_list.Next();            // should be other cmd
      cmd = value_list.Get();       //other's cmd
      sub_list = cmd.Get(Am_ITEMS); //other's sub-menu
      sub_list.Start();             //at label3.1
      //false because destructive modification
      sub_list.Insert(Am_Command.Create()
                          .Set(Am_LABEL, "Another New Command")
                          .Set(Am_DO_METHOD, my_do),
                      Am_AFTER, false);
      cmd.Note_Changed(Am_ITEMS);
      break;
    }
    case 7: {
      std::cout << " Change name of new item in Other\n" << std::flush;
      value_list.Next();            // should be edit cmd
      value_list.Next();            // should be other cmd
      cmd = value_list.Get();       //other's cmd
      sub_list = cmd.Get(Am_ITEMS); //other's sub-menu
      sub_list.Start();             //at label3.1
      sub_list.Next();              //at new item
      cmd = sub_list.Get();
      cmd.Set(Am_LABEL, "New Name for Item");
      break;
    }
    case 8: {
      std::cout << " Adding new top-level item\n" << std::flush;
      value_list.Next(); // at edit cmd
      Am_Object new_command =
          Am_Command.Create("New_Top_Command")
              .Set(Am_LABEL, "New Top")
              .Set(Am_DO_METHOD, my_do)
              .Set(Am_ITEMS, Am_Value_List()
                                 .Add("New sub1")
                                 .Add(new_other_command.Create().Set(
                                     Am_ACCELERATOR, "META_Z"))
                                 .Add("New sub2"));
      //false because destructive modification
      value_list.Insert(new_command, Am_AFTER, false);
      //changed the top-level items list
      menu_bar.Note_Changed(Am_ITEMS);
      break;
    }
    case 9: {
      std::cout << " Removing top-level item\n" << std::flush;
      value_list.Next(); // should be edit
      value_list.Next(); // should be at new item
      //false because destructive modification
      value_list.Delete(false);
      //changed the top-level items list
      menu_bar.Note_Changed(Am_ITEMS);
      break;
    }
    case 10: {
      std::cout << " Removing new item from Other\n" << std::flush;
      value_list.Next();            // should be edit cmd
      value_list.Next();            // should be other cmd
      cmd = value_list.Get();       //other's cmd
      sub_list = cmd.Get(Am_ITEMS); //other's sub-menu
      sub_list.Start();             //at label3.1
      sub_list.Next();              //new item
      //false because destructive modification
      sub_list.Delete(false);
      cmd.Note_Changed(Am_ITEMS);
      break;
    }
    case 11: {
      std::cout << " Removing Other new item from Other\n" << std::flush;
      value_list.Next();            // should be edit cmd
      value_list.Next();            // should be other cmd
      cmd = value_list.Get();       //other's cmd
      sub_list = cmd.Get(Am_ITEMS); //other's sub-menu
      sub_list.Start();             //at label3.1
      sub_list.Next();              //new item
      //false because destructive modification
      sub_list.Delete(false);
      cmd.Note_Changed(Am_ITEMS);
      break;
    }
    } //end switch
    menu_bar_cnt = (menu_bar_cnt % 11) + 1;
    break;
  }
  case 't': {
    static int button_contents = 1;
    switch (button_contents) {
    case 1:
      std::cout << " Changing label of " << cmd4 << " to a long string\n"
                << std::flush;
      cmd4.Set(Am_LABEL, "A Rather Long String");
      break;
    case 2:
      std::cout << " Changing label of " << cmd4 << " to red rect2\n"
                << std::flush;
      cmd4.Set(Am_LABEL, rect2_in_button);
      break;
    case 3:
      std::cout << " Changing label of " << cmd4 << " to green rect\n"
                << std::flush;
      cmd4.Set(Am_LABEL, rect_in_button);
      break;
    case 4:
      std::cout << " Changing label of " << cmd4 << " to short string\n"
                << std::flush;
      cmd4.Set(Am_LABEL, "OK");
      break;
    }
    button_contents = (button_contents % 4) + 1;
    break;
  }
  case 'T': {
    static int button8_contents = 1;
    switch (button8_contents) {
    case 1:
      std::cout << " Changing command to be another string\n" << std::flush;
      button8.Set(Am_COMMAND, "A Rather Long String");
      break;
    case 2:
      std::cout << " Changing command to be an object\n" << std::flush;
      button8.Set(Am_COMMAND, rect2_in_button);
      break;
    case 3:
      std::cout << " Changing command to be a PART\n" << std::flush;
      //should unset the slot and remove rect2_in_button
      button8.Remove_Slot(Am_COMMAND);
      button8.Set_Part(Am_COMMAND, rect3_in_button.Create());
      break;
    case 4:
      std::cout << " Changing command to be original string\n" << std::flush;
      //should delete the part created above
      button8.Remove_Part(Am_COMMAND);
      button8.Set(Am_COMMAND, "Command is String");
      break;
    }
    button8_contents = (button8_contents % 4) + 1;
    break;
  }
  case 'B': {
    bool on = !(bool)Am_Window.Get(Am_DOUBLE_BUFFER);
    if (on)
      std::cout << "Turning double buffering ON.\n";
    else
      std::cout << "Turning double buffering OFF.\n";
    Am_Window.Set(Am_DOUBLE_BUFFER, on);
    break;
  }
  case 'C': {
    Am_Object new_win = scrolling_window.Copy();
    std::cout << "Created new window as copy " << new_win << std::endl
              << std::flush;
    Am_Screen.Add_Part(new_win);
    break;
  }
  case 'i': {
    static bool tracing = false;
    if (tracing)
      Am_Set_Inter_Trace(Am_INTER_TRACE_NONE);
    else
      Am_Set_Inter_Trace(Am_INTER_TRACE_ALL);
    tracing = !tracing;
    break;
  }
  case 'I': {
    Am_Object new_win = scrolling_window.Create();
    std::cout << "Created new window as instance " << new_win << std::endl
              << std::flush;
    Am_Screen.Add_Part(new_win);
    break;
  }
  case 'l': {
    bool bol = button_panel3.Get(Am_BOX_ON_LEFT);
    std::cout << "Setting Am_BOX_ON_LEFT to " << !bol << std::endl
              << std::flush;
    button_panel3.Set(Am_BOX_ON_LEFT, !bol);
    break;
  }
  case 'L': {
    Am_Widget_Look look = Am_Widget_Group.Get(Am_WIDGET_LOOK);

    switch (look.value) {
    case Am_MOTIF_LOOK_val:
      look = Am_WINDOWS_LOOK;
      std::cout << "Setting Windows Look\n";
      break;

    case Am_WINDOWS_LOOK_val:
      look = Am_MACINTOSH_LOOK;
      std::cout << "Setting Macintosh Look\n";
      break;

    case Am_MACINTOSH_LOOK_val:
      look = Am_MOTIF_LOOK;
      std::cout << "Setting Motif Look\n";
      break;

    default:
      Am_Error("Unknown Look parameter");
      break;
    }

    Am_Set_Default_Look(look);
    break;
  }
  case 'h': {
    bool fh = button_panel2.Get(Am_FIXED_HEIGHT);
    std::cout << "Setting Am_FIXED_HEIGHT to " << !fh << std::endl
              << std::flush;
    button_panel2.Set(Am_FIXED_HEIGHT, !fh);
    break;
  }
  case 'w': {
    bool fw = button_panel2.Get(Am_FIXED_WIDTH);
    std::cout << "Setting Am_FIXED_WIDTH to " << !fw << std::endl << std::flush;
    button_panel2.Set(Am_FIXED_WIDTH, !fw);
    break;
  }
  case 'x': {
    Undo_Redo_Selective(Am_SELECTIVE_UNDO_ALLOWED, Am_SELECTIVE_UNDO_METHOD,
                        "Selective Undo");
    break;
  }
  case 'R': {
    Undo_Redo_Selective(Am_SELECTIVE_REPEAT_SAME_ALLOWED,
                        Am_SELECTIVE_REPEAT_SAME_METHOD, "Selective Repeat");
    break;
  }
  case 'S': {
    Am_Object widget_to_start;
    static int start_which_widget = 1;
    switch (start_which_widget) {
    case 1:
      widget_to_start = button1;
      break;
    case 2:
      widget_to_start = button_panel;
      break;
    case 3:
      widget_to_start = checkboxes;
      break;
    case 4:
      widget_to_start = button_panel3;
      break;
    case 5:
      widget_to_start = vscroll;
      break;
    case 6:
      widget_to_start = textinputwidget;
      break;
    case 7:
      widget_to_start = menu_bar;
      break;
    }
    start_which_widget = (start_which_widget % 7) + 1;
    std::cout << "Explicitly starting widget " << widget_to_start << std::endl
              << std::flush;
    Am_Start_Widget(widget_to_start);
    break;
  }
  case 'n': {
    static int number_input_mode = 1;
    switch (number_input_mode) {
    case 1:
      number_input_widget.Set(Am_VALUE_1, 0);
      number_input_widget.Set(Am_VALUE_2, 100);
      std::cout << "Number widget between 0..100 (int), db\n" << std::flush;
      break;
    case 2:
      number_input_widget.Set(Am_VALUE_1, -1.0);
      number_input_widget.Set(Am_VALUE_2, 1.0);
      std::cout << "Number widget between -1.0..1.0 (float), db\n"
                << std::flush;
      break;
    case 3:
      number_input_widget.Set(Am_USE_ERROR_DIALOG, false);
      number_input_widget.Set(Am_VALUE_1, 0);
      number_input_widget.Set(Am_VALUE_2, 100);
      std::cout << "Number widget between 0..100 (int), no db\n" << std::flush;
      break;
    case 4:
      number_input_widget.Set(Am_VALUE_1, -1.0);
      number_input_widget.Set(Am_VALUE_2, 1.0);
      std::cout << "Number widget between -1.0..1.0 (float), no db\n"
                << std::flush;
      break;
    case 5:
      number_input_widget.Set(Am_VALUE_1, 'c');
      std::cout << "Number widget will crash (v1 is char)\n" << std::flush;
      break;
    case 6:
      number_input_widget.Set(Am_VALUE_1, 0);
      number_input_widget.Set(Am_VALUE_2, 4.5);
      std::cout << "Number widget will crash (not same)\n" << std::flush;
      break;
    case 7:
      number_input_widget.Set(Am_USE_ERROR_DIALOG, true);
      number_input_widget.Set(Am_VALUE_1, Am_No_Value);
      number_input_widget.Set(Am_VALUE_2, Am_No_Value);
      std::cout << "Number widget OK, no checking\n" << std::flush;
      break;
    }
    number_input_mode = (number_input_mode % 7) + 1;
    break;
  }
  case 'z': {
    static int scrolling_menu_mode = 1;
    switch (scrolling_menu_mode) {
    case 1:
      std::cout << "1.setting scroll_menu value to Item14, Item15\n"
                << std::flush;
      scroll_menu.Set(Am_VALUE, Am_Value_List().Add("Item14").Add("Item15"));
      break;
    case 2:
      std::cout << "2.setting scroll menu to be not active\n" << std::flush;
      scroll_menu.Set(Am_ACTIVE, false);
      break;
    case 3:
      std::cout << "3.setting scroll menu to be active\n" << std::flush;
      scroll_menu.Set(Am_ACTIVE, true);
      break;
    case 4: {
      std::cout << "4.Adding new item\n" << std::flush;
      Am_Value_List l = scroll_menu.Get(Am_ITEMS);
      l.Add("New Item");
      scroll_menu.Set(Am_ITEMS, l);
      break;
    }
    case 5:
      std::cout << "5.Show horizontal scroll bar\n" << std::flush;
      scroll_menu.Set(Am_H_SCROLL_BAR, true);
      break;
    case 6:
      std::cout << "6.Remove horizontal scroll bar\n" << std::flush;
      scroll_menu.Set(Am_H_SCROLL_BAR, false);
      break;
    case 7:
      std::cout << "7.Change to Choice_Set\n" << std::flush;
      scroll_menu.Set(Am_HOW_SET, Am_CHOICE_SET);
      break;
    case 8:
      std::cout << "8.Change back to List toggle \n" << std::flush;
      scroll_menu.Set(Am_HOW_SET, Am_CHOICE_LIST_TOGGLE);
      break;
    case 9:
      std::cout << "9.Change to Toggle\n" << std::flush;
      scroll_menu.Set(Am_HOW_SET, Am_CHOICE_TOGGLE);
      break;
    case 10:
      std::cout << "10.Change back to List toggle\n" << std::flush;
      scroll_menu.Set(Am_HOW_SET, Am_CHOICE_LIST_TOGGLE);
      break;
    }
    scrolling_menu_mode = (scrolling_menu_mode % 10) + 1;
    break;
  }

  case 'X': {
    Am_Value v = Am_Get_Input_From_Dialog(
        Am_Value_List()
            .Add("Type a value into the box.")
            .Add("I'll print it to std::cout if you hit okay."),
        "Hello there", 150, 150); //, true);
    if (v.Valid())
      std::cout << v << std::endl;
    break;
  }
  case 'Y': {
    Am_Value v = Am_Get_Choice_From_Dialog(
        Am_Value_List().Add("Do you really want to do that?"), 120, 250, true);
    if (v.Valid())
      std::cout << v << std::endl;
    break;
  }
  case 'Z': {
    Am_Show_Alert_Dialog(Am_Value_List()
                             .Add("This is a test")
                             .Add("This is only a test")
                             .Add("If this were an actual error")
                             .Add("I would have crashed by now"));
    break;
  }
  case 'o': {
    std::cout << "setting " << option_button << " value to 4\n" << std::flush;
    option_button.Set(Am_VALUE, 4);
    break;
  }
  case 'r': {
    bool fringe = button_panel2.Get(Am_LEAVE_ROOM_FOR_FRINGE);
    std::cout << "Setting Am_LEAVE_ROOM_FOR_FRINGE to " << !fringe << std::endl;
    button_panel2.Set(Am_LEAVE_ROOM_FOR_FRINGE, !fringe);
    break;
  }
  case 'q':
    Am_Exit_Main_Event_Loop();
    break;
  case '\r':
  default:
    std::cout
        << "** Options are:\n"
        << "   a = toggle button active\n"
        << "   A = toggle button active2\n"
        << "   b = toggle rectangle\n"
        << "   B = toggle double buffering\n"
        << "   c = set widgets color\n"
        << "   C = copy scroll window, I = instance scroll window\n"
        << "   d = toggle default\n"
        << "   D = toggle Pending-Delete for text widgets\n"
        // **    << "   e = set widget with illegal value to check warning\n"
        //       << "   E = \n"
        << "   f = toggle button want final feedback\n"
        << "   F = set widgets font\n"
        //       << "   g = \n"
        //       << "   G = \n"
        << "   h = toggle fixed height on button panel 2\n"
        //       << "   H = \n"
        << "   i = inter tracing\n"
        << "   I = add another scrolling window\n"
        //       << "   j = \n"
        //       << "   J = \n"
        << "   k = toggle button key-selected\n"
        << "   K = change abort mode for editing integer\n"
        << "   l = toggle BOX_ON_LEFT\n"
        << "   L = rotate look and feel (Motif -> Win -> Mac -> Motif...)\n"
        << "   m = modify menubar items\n"
        //       << "   M = \n"
        << "   n = change settings of number input\n"
        //       << "   N = \n"
        << "   o = set value of option button\n"
        //       << "   O = \n"
        << "   p = set value of panel\n"
        << "   P = toggle active of whole panel\n"
        << "   q = quit\n"
        //       << "   Q = \n"
        << "   r = toggle LEAVE_ROOM_FOR_FRINGE on button panel 2\n"
        << "   R = selective repeat same undo\n"
        << "   s = set button size\n"
        << "   S = start widget\n"
        << "   t,T = toggle button text or object\n"
        //       << "   u = \n"
        //       << "   U = \n"
        << "   v,V = set value of button\n"
        << "   w = toggle fixed width on button panel 2\n"
        //       << "   W = \n"
        << "   x = selective undo\n"
        << "   X = Am_Get_Input_From_Dialog\n"
        //       << "   y = \n"
        << "   Y = Am_Get_Choice_From_Dialog\n"
        << "   z = test scrolling menu\n"
        << "   Z = Am_Show_Alert_Dialog\n"
        << " TAB = go to next text input field (shift-TAB goes backwards)\n"
        << std::endl;
    break;
  } // end switch
}
//free: wyz W

Am_Define_Method(Am_Object_Method, void, do_quit, (Am_Object /*cmd*/))
{
  std::cout << "Exiting TestWidgets, goodbye\n" << std::flush;
  Am_Exit_Main_Event_Loop();
}

Am_Define_Method(Am_Object_Method, void, abort_method, (Am_Object cmd))
{
  std::cout << "****Command " << cmd << " aborting\n" << std::flush;
  Am_Abort_Widget(cmd);
}

Am_Define_Formula(Am_Value, get_other_form)
{
  Am_Value value;
  Am_Object other;
  other = self.Get(Am_OBJECT_MODIFIED);
  value = other.Peek(Am_VALUE);
  return value;
}

#define COLORSTR "Color to "
#define DEFAULTSTR "White"
#define MAXNAMESIZE 20

Am_Define_String_Formula(name_from_value)
{
  Am_Value value;
  value = self.Peek(Am_VALUE);
  static char outs[MAXNAMESIZE];
  strcpy(outs, COLORSTR);
  //concatentate current color on end
  if (value.type == Am_STRING) {
    strcat(outs, Am_String(value));
  } else
    strcat(outs, DEFAULTSTR);
  std::cout << "new name for " << self << " is `" << outs << "'\n"
            << std::flush;
  return Am_String(outs);
}

Am_Define_No_Self_Formula(Am_Wrapper *, color_from_panel)
{
  Am_Value value;
  //std::cout << "_Reevaluate color_from_panel for command " << cmd << std::endl << std::flush;
  value = button_panel3.Peek(Am_VALUE);
  std::cout << "Color Value = " << value << std::endl << std::flush;
  if (value.type == Am_STRING) {
    Am_String s;
    s = value;
    if ((const char *)s) {
      std::cout << "new color = " << (const char *)s << std::endl << std::flush;
      if (strcmp(s, "Red") == 0)
        return Am_Red;
      else if (strcmp(s, "Blue") == 0)
        return Am_Blue;
      else if (strcmp(s, "Green") == 0)
        return Am_Green;
      else if (strcmp(s, "Yellow") == 0)
        return Am_Yellow;
      else if (strcmp(s, "Orange") == 0)
        return Am_Orange;
      else
        return Am_White;
    } else
      return Am_White;
  } else
    return Am_White;
}

Am_Define_Formula(int, owner_width_minus_20)
{
  return (int)self.Get_Owner().Get(Am_WIDTH) - 20;
}

Am_Define_Formula(int, owner_height_minus_20)
{
  return (int)self.Get_Owner().Get(Am_HEIGHT) - 20;
}

#define HAS_H 10
#define HAS_V 11
#define V_ON_LEFT 12
#define H_ON_TOP 13

Am_Define_Formula(bool, h_scroll_bar_form)
{
  Am_Value value;
  value = Am_Object(self.Get(R5)).Peek(Am_VALUE);
  return find_in_list(value, HAS_H);
}
Am_Define_Formula(bool, v_scroll_bar_form)
{
  Am_Value value;
  value = Am_Object(self.Get(R5)).Peek(Am_VALUE);
  return find_in_list(value, HAS_V);
}
Am_Define_Formula(bool, v_scroll_on_left_form)
{
  Am_Value value;
  value = Am_Object(self.Get(R5)).Peek(Am_VALUE);
  return find_in_list(value, V_ON_LEFT);
}
Am_Define_Formula(bool, h_scroll_on_top_form)
{
  Am_Value value;
  value = Am_Object(self.Get(R5)).Peek(Am_VALUE);
  return find_in_list(value, H_ON_TOP);
}

Am_Define_Object_Formula(compute_feedback_obj)
{
  Am_Object which_feedback;
  if ((bool)self.Get(Am_AS_LINE)) {
    std::cout << "&-&&- Recompute formula; using LINE feedback\n";
    which_feedback = lfeedback;
  } else {
    std::cout << "&-&&- Recompute formula; using RECT feedback\n";
    which_feedback = rfeedback;
  }
  return which_feedback;
}

Am_Define_Formula(bool, as_line_if_shift)
{
  Am_Input_Char start_char = self.Get(Am_START_CHAR);
  if (start_char == Am_Input_Char("SHIFT_RIGHT_DOWN"))
    return true;
  else
    return false;
}

Am_Define_Method(Am_Create_New_Object_Method, Am_Object, create_new_object,
                 (Am_Object inter, Am_Inter_Location data,
                  Am_Object /* old_object */))
{
  Am_Object owner, ref_obj, new_obj;
  owner = inter.Get_Owner();
  int a, b, c, d;
  bool create_line;
  data.Get_Location(create_line, ref_obj, a, b, c, d);
  if (ref_obj != owner) {
    Am_Translate_Coordinates(ref_obj, a, b, owner, a, b);
    if (create_line)
      Am_Translate_Coordinates(ref_obj, c, d, owner, c, d);
  }
  if (create_line)
    new_obj = Am_Line.Create().Set(Am_X1, a).Set(Am_Y1, b).Set(Am_X2, c).Set(
        Am_Y2, d);
  else
    new_obj = Am_Rectangle.Create()
                  .Set(Am_FILL_STYLE, Am_Green)
                  .Set(Am_LEFT, a)
                  .Set(Am_TOP, b)
                  .Set(Am_WIDTH, c)
                  .Set(Am_HEIGHT, d);

  std::cout << "\n--++-- Created new object " << new_obj << " in " << owner
            << " at (" << a << "," << b << "," << c << "," << d << ")\n"
            << std::flush;
  owner.Add_Part(new_obj);
  return new_obj;
}

#if defined(_WINDOWS)
#define AMULET_BITMAP "images/amside.gif"
#elif defined(_MACINTOSH)
#define AMULET_BITMAP "lib/images/amside.gif"
#else
#define AMULET_BITMAP "lib/images/amuletside.xbm"
#endif

void
load_bitmap()
{
  const char *filename = Am_Merge_Pathname(AMULET_BITMAP);
  Am_Image_Array amulet_icon_image(filename);
  delete[] filename;
  if (!amulet_icon_image.Valid())
    Am_Error("Amulet bitmap image not found");
  Amulet_Dark_Purple = Am_Style(0.85f, 0.75f, 0.95f);
  amulet_icon = Am_Bitmap.Create("Amulet_Icon")
                    .Set(Am_IMAGE, amulet_icon_image)
                    .Set(Am_LINE_STYLE, Am_Black)
                    .Set(Am_FILL_STYLE, Am_No_Style);
}

//// testing dialog boxes
Am_Define_Method(Am_Object_Method, void, do_pop_up_error, (Am_Object /*cmd*/))
{
  AM_POP_UP_ERROR_WINDOW("This is an error message");
}

Am_Define_Method(Am_Object_Method, void, do_pop_up_alert, (Am_Object /*cmd*/))
{
  Am_Show_Alert_Dialog(Am_Value_List()
                           .Add("First line of message")
                           .Add("Second Line of Message"));
}

Am_Define_Method(Am_Object_Method, void, do_pop_up_string, (Am_Object /*cmd*/))
{
  Am_Value v = Am_Get_Input_From_Dialog(
      Am_Value_List().Add("First line of message"), "Initial value");
  std::cout << "Value returned is `" << v << "'\n" << std::flush;
}

Am_Define_Method(Am_Object_Method, void, do_pop_up_choice, (Am_Object /*cmd*/))
{
  Am_Value v = Am_Get_Choice_From_Dialog(
      Am_Value_List().Add("First line of choice").Add("Second Line of Choice"));
  std::cout << "Value returned is `" << v << "'\n" << std::flush;
}

int
main()
{
  std::cout << "Initialize\n";
  Am_Initialize();
  std::cout << "Loading image " << AMULET_BITMAP << "...  " << std::flush;
  load_bitmap();
  std::cout << "succeeded\n" << std::endl;
  // Am_Set_Inter_Trace(Am_INTER_TRACE_ALL);

  std::cout
      << "Rectangle: change become selected with b, change color with c\n"
      << "Label button: toggle active(a), active2(A), size(s), key-sel(k)\n"
      << "    change value with V\n"
      << "button containing green rectangle, change contents with t,\n"
      << "    change value with v\n"
      << "Second button with Label is an instance of first, also changes\n"
      << "    label with first changed with s\n"
      << "Second button with green label is an instance of other one,\n"
      << "    also changes with t\n"
      << "Copy-value button contains a constraint that copies value from\n"
      << "    first button, so its value should toggle with it\n"
      << " -->see whether buttons selected or not using f\n"
      << std::endl
      << std::flush;

  undo_handler = Am_Multiple_Undo_Object.Create("My_Multi_Undo");

  window = Am_Window.Create("window")
               .Set(Am_TOP, 45)
               .Set(Am_FILL_STYLE, Am_Amulet_Purple)
               .Set(Am_UNDO_HANDLER, undo_handler)
               .Set(Am_WIDTH, 675)
               .Set(Am_HEIGHT, 645);

  rfeedback = Am_Rectangle.Create("rfeedback")
                  .Set(Am_FILL_STYLE, 0)
                  .Set(Am_LINE_STYLE, Am_Dotted_Line)
                  .Set(Am_VISIBLE, 0)
                  .Set(Am_LEFT, 100)
                  .Set(Am_TOP, 10)
                  .Set(Am_WIDTH, 50)
                  .Set(Am_HEIGHT, 50);
  lfeedback = Am_Line.Create("lfeedback")
                  .Set(Am_LINE_STYLE, // thick dotted line
                       Am_Style(0.0f, 0.0f, 0.0f, 5, Am_CAP_BUTT, Am_JOIN_MITER,
                                Am_LINE_ON_OFF_DASH))
                  .Set(Am_VISIBLE, 0);

  rect_in_button = Am_Rectangle.Create("rect_in_button")
                       .Set(Am_WIDTH, 30)
                       .Set(Am_HEIGHT, 20)
                       .Set(Am_FILL_STYLE, Am_Green);
  rect2_in_button = Am_Rectangle.Create("rect2_in_button")
                        .Set(Am_WIDTH, 100)
                        .Set(Am_HEIGHT, 100)
                        .Set(Am_FILL_STYLE, Am_Red);
  rect3_in_button = Am_Rectangle.Create("rect2_in_button")
                        .Set(Am_WIDTH, 30)
                        .Set(Am_HEIGHT, 30)
                        .Set(Am_FILL_STYLE, Am_Blue);

  new_other_command = Am_Command.Create()
                          .Set(Am_LABEL, "New Other Command")
                          .Set(Am_ACCELERATOR, "META_SHIFT_O")
                          .Set(Am_DO_METHOD, my_do);

  Am_Object undo_command =
      Am_Command.Create("undo_command")
          .Set(Am_DO_METHOD, do_undo)
          .Set(Am_LABEL, "Undo")
          // need a parent so the undo itself is not pushed onto the undo stack
          .Set(Am_IMPLEMENTATION_PARENT, true);
  Am_Object redo_command =
      Am_Command.Create("redo_command")
          .Set(Am_DO_METHOD, do_redo)
          .Set(Am_LABEL, "Redo")
          // need a parent so the redo itself is not pushed onto the undo stack
          .Set(Am_IMPLEMENTATION_PARENT, true);

  menu_bar =
      Am_Menu_Bar.Create()
          .Set(
              Am_ITEMS,
              Am_Value_List()
                  .Add(Am_Command.Create("Amulet_Command")
                           .Set(Am_DO_METHOD, my_do)
                           .Set(Am_LABEL, amulet_icon)
                           .Set(Am_ITEMS, Am_Value_List()
                                              .Add("About Testwidgets...")
                                              .Add("About Amulet...")))
                  .Add(
                      Am_Command.Create("File_Command")
                          .Set(Am_LABEL, "File")
                          .Set(Am_DO_METHOD, my_do)
                          .Set(
                              Am_ITEMS,
                              Am_Value_List()
                                  .Add("Open...")
                                  .Add("Save As...")
                                  .Add(Am_Command.Create("Save_Command")
                                           .Set(Am_LABEL, "Save")
                                           .Set(Am_ACCELERATOR, "CONTROL_s")
                                           .Add(Am_CHECKED_ITEM, true)
                                           .Set(Am_DO_METHOD, my_do))
                                  .Add(Am_Command.Create("Quit_Command")
                                           .Set(Am_LABEL, "Quit")
                                           .Set(Am_ACCELERATOR,
                                                "CONTROL_SHIFT_META_F4")
                                           .Set(Am_DO_METHOD, do_quit))
                                  .Add(Am_Menu_Line_Command.Create())
                                  .Add(Am_Command.Create("Error_Command")
                                           .Set(Am_LABEL, "Pop Up Error")
                                           .Set(Am_DO_METHOD, do_pop_up_error))
                                  .Add(Am_Command.Create("Pop_Up_Alert_Command")
                                           .Set(Am_LABEL, "Pop Up Alert")
                                           .Set(Am_DO_METHOD, do_pop_up_alert))
                                  .Add(
                                      Am_Command.Create("Pop_Up_String_Command")
                                          .Set(Am_LABEL, "Pop Up String")
                                          .Set(Am_DO_METHOD, do_pop_up_string))
                                  .Add(
                                      Am_Command.Create("Pop_Up_Choice_Command")
                                          .Set(Am_LABEL, "Pop Up Choice")
                                          .Set(Am_DO_METHOD,
                                               do_pop_up_choice))))
                  .Add(
                      Am_Command.Create("Edit_Command")
                          .Set(Am_LABEL, "Edit")
                          .Set(Am_DO_METHOD, my_do)
                          .Set(
                              Am_ITEMS,
                              Am_Value_List()
                                  .Add(undo_command.Create())
                                  .Add(redo_command.Create())
                                  .Add("Cut")
                                  .Add("Copy")
                                  .Add("Paste")
                                  .Add(Am_Menu_Line_Command.Create(
                                      "my menu line"))
                                  .Add(cy_cmd =
                                           Am_Cycle_Value_Command
                                               .Create("my_cycle")
                                               .Set(Am_LABEL_LIST,
                                                    Am_Value_List()
                                                        .Add("First label")
                                                        .Add("Second Label")
                                                        .Add("Third Label")
                                                        .Add(rect3_in_button
                                                                 .Create(
                                                                     "rect_in_"
                                                                     "val"))))
                                  .Add(Am_Menu_Line_Command.Create(
                                      "my menu line"))
                                  .Add(4)              //test an int
                                  .Add(6.5)            //test a float
                                  .Add('$')            //test a character
                                  .Add(Am_Value_List() //test a wrapper
                                           .Add(1)
                                           .Add(2)
                                           .Add(3))
                                  .Add(my_do) //test a method
                                  .Add(Am_Menu_Line_Command.Create(
                                      "my menu line"))
                                  .Add(Am_Command.Create("Motif_Command")
                                           .Set(Am_LABEL, "Motif Look")
                                           .Add(Am_WIDGET_LOOK, Am_MOTIF_LOOK)
                                           .Add(Am_CHECKED_ITEM, look_is_me)
                                           .Set(Am_DO_METHOD, set_look)
                                           .Set(Am_IMPLEMENTATION_PARENT,
                                                true) //not undo
                                       )
                                  .Add(Am_Command.Create("Win_Command")
                                           .Set(Am_LABEL, "Windows Look")
                                           .Add(Am_WIDGET_LOOK, Am_WINDOWS_LOOK)
                                           .Add(Am_CHECKED_ITEM, look_is_me)
                                           .Set(Am_IMPLEMENTATION_PARENT,
                                                true) //not undo
                                           .Set(Am_DO_METHOD, set_look))
                                  .Add(Am_Command.Create("Mac_Command")
                                           .Set(Am_LABEL, "Macintosh Look")
                                           .Add(Am_WIDGET_LOOK,
                                                Am_MACINTOSH_LOOK)
                                           .Add(Am_CHECKED_ITEM, look_is_me)
                                           .Set(Am_IMPLEMENTATION_PARENT,
                                                true) //not undo
                                           .Set(Am_DO_METHOD, set_look))))
                  .Add(
                      Am_Command.Create("Other_Command")
                          .Set(Am_LABEL, "Other")
                          .Set(Am_DO_METHOD, my_do)
                          .Set(Am_ITEMS,
                               Am_Value_List()
                                   .Add("Label3.1")
                                   .Add(rect3_in_button.Create())
                                   .Add("Label3.2")
                                   .Add(Am_Command.Create("abort me")
                                            .Set(Am_LABEL, "Abort me")
                                            .Set(Am_DO_METHOD, abort_method)))))
          .Set_Part(Am_COMMAND,
                    Am_Command.Create("Menu_Bar_Command")
                        .Set(Am_DO_METHOD, my_do) //this should never be called
                    );
  window.Add_Part(menu_bar);
  window.Add_Part(Am_Text.Create("value of cycle button")
                      .Set(Am_LEFT, 350)
                      .Set(Am_TOP, 40)
                      .Set(Am_TEXT, value_from_cycle_command));
  textinputwidget =
      Am_Text_Input_Widget.Create("text_input").Set(Am_LEFT, 9).Set(Am_TOP, 46);
  textinputwidget.Get_Object(Am_COMMAND)
      .Set_Name("First _Text_Command")
      .Set(Am_DO_METHOD, my_do);
  window.Add_Part(textinputwidget);
  textinputwidget2 = Am_Text_Input_Widget.Create("text_input2")
                         .Set(Am_LEFT, 180)
                         .Set(Am_TOP, 46);
  textinputwidget2.Get_Object(Am_COMMAND)
      .Set_Name("Second_Text_Command")
      .Set(Am_LABEL, rect3_in_button.Create())
      .Set(Am_DO_METHOD, my_do);
  window.Add_Part(textinputwidget2);

  Am_Object textinputwidget3 = Am_Text_Input_Widget.Create("text_input3")
                                   .Set(Am_LEFT, 278)
                                   .Set(Am_TOP, 220)
                                   .Set(Am_WIDTH, 160);
  textinputwidget3.Get_Object(Am_COMMAND)
      .Set_Name("Third_Text_Command")
      .Set(Am_LABEL, "Text3")
      .Set(Am_DO_METHOD, my_do);
  window.Add_Part(textinputwidget3);
  number_input_widget = Am_Number_Input_Widget.Create("number_input4")
                            .Set(Am_LEFT, 330)
                            .Set(Am_TOP, 300)
                            .Set(Am_WIDTH, 180)
                            .Set(Am_VALUE, 4567);
  number_input_widget.Get_Object(Am_COMMAND)
      .Set_Name("number_input_widget_command")
      .Set(Am_LABEL, "Int or Float:")
      .Set(Am_DO_METHOD, my_do);
  window.Add_Part(number_input_widget);
  Am_Object textinputwidget5 = Am_Password_Input_Widget.Create("passwd_input")
                                   .Set(Am_LEFT, 330)
                                   .Set(Am_TOP, 330)
                                   .Set(Am_WIDTH, 180);
  textinputwidget5.Get_Object(Am_COMMAND)
      .Set_Name("Password_Text_Command")
      .Set(Am_LABEL, "Password")
      .Set(Am_DO_METHOD, my_do);
  window.Add_Part(textinputwidget5);

  Am_Object how_set_inter = Am_One_Shot_Interactor.Create("change_settings")
                                .Set(Am_START_WHEN, "ANY_KEYBOARD")
                                .Set(Am_PRIORITY, -1) //lower than normal
      ;
  Am_Object cmd;
  cmd = how_set_inter.Get(Am_COMMAND);
  cmd.Set(Am_DO_METHOD, change_setting);
  cmd.Set(Am_IMPLEMENTATION_PARENT, true); //not undo-able
  cmd.Set_Name("change_settings_command");
  window.Add_Part(how_set_inter);

  rect = Am_Border_Rectangle.Create("my_rect")
             .Set(Am_LEFT, 20)
             .Set(Am_TOP, 80)
             .Set(Am_WIDTH, 60)
             .Set(Am_HEIGHT, 60);
  button1 = Am_Button.Create("button1").Set(Am_LEFT, 100).Set(Am_TOP, 80);
  button1.Get_Object(Am_COMMAND)
      .Set(Am_DO_METHOD, my_do)
      .Set(Am_LABEL, "Button1");
  abort_button =
      Am_Button.Create("abort_button").Set(Am_LEFT, 190).Set(Am_TOP, 100);
  abort_button.Get_Object(Am_COMMAND)
      .Set(Am_LABEL, "Abort me")
      .Set(Am_DO_METHOD, abort_method);

  Am_Object undobutton = Am_Button.Create("undo_button")
                             .Set(Am_LEFT, 20)
                             .Set(Am_TOP, 160)
                             .Set_Part(Am_COMMAND, undo_command);

  Am_Object redobutton = Am_Button.Create("redo_button")
                             .Set(Am_LEFT, 120)
                             .Set(Am_TOP, 160)
                             .Set_Part(Am_COMMAND, redo_command);

  button4 = Am_Button.Create("obj in slot")
                .Set(Am_LEFT, 200)
                .Set(Am_TOP, 160)
                .Set(Am_ITEM_OFFSET, 15);
  cmd4 = button4.Get_Object(Am_COMMAND);
  cmd4.Set(Am_LABEL, rect_in_button);

  Am_Object button5 =
      button1.Create("instance of button").Set(Am_LEFT, 20).Set(Am_TOP, 260);
  Am_Object button7 =
      button4.Create("instance of button4").Set(Am_LEFT, 20).Set(Am_TOP, 310);
  button6 = Am_Button.Create("mutual")
                .Set(Am_LEFT, 20)
                .Add(Am_OBJECT_MODIFIED, button1)
                .Set(Am_VALUE, get_other_form)
                .Set(Am_TOP, 410);
  Am_Object cmd6 = button6.Get_Object(Am_COMMAND);
  cmd6.Set(Am_LABEL, "Copy value from Button1");

  button8 = Am_Button.Create("Button8").Set(Am_LEFT, 20).Set(Am_TOP, 210);
  // should remove the part created automatically and replace with this string
  button8.Remove_Part(Am_COMMAND);
  button8.Add(Am_COMMAND, "Command is String");

  button_panel =
      Am_Button_Panel.Create("My Button Panel")
          .Set(Am_LEFT, 20)
          .Set(Am_TOP, 460)
          .Set(
              Am_ITEMS,
              Am_Value_List()
                  .Add(Am_Command.Create("label1cmd").Set(Am_LABEL, "Label1"))
                  .Add(Am_Command.Create("label2cmd").Set(Am_LABEL, "Label2"))
                  .Add(Am_Command.Create("label3cmd").Set(Am_LABEL, "Label3")));

  Am_Value_List l;
  l = button_panel.Get(Am_GRAPHICAL_PARTS);
  l.Start();
  l.Next();
  button_panel_obj2 = l.Get();

  checkboxes = Am_Checkbox_Panel.Create("my_checkbox_panel")
                   .Set(Am_LEFT, 100)
                   .Set(Am_TOP, 463)
                   .Set(Am_ITEMS,
                        Am_Value_List()
                            .Add(Am_Command.Create("check cmd 1")
                                     .Set(Am_LABEL, "Boxes on right")
                                     .Set(Am_ID, ON_RIGHT))
                            .Add(Am_Command.Create("check cmd 2")
                                     .Set(Am_LABEL, "Fixed width button panel")
                                     .Set(Am_ID, FIXED_WIDTH))
                            .Add(Am_Command.Create("check cmd 3")
                                     .Set(Am_LABEL, "Fixed height button panel")
                                     .Set(Am_ID, FIXED_HEIGHT)))
                   .Set(Am_BOX_ON_LEFT, on_left_form);

  button_panel2 =
      Am_Button_Panel.Create("My Button Panel 2")
          .Set(Am_LEFT, 345)
          .Set(Am_TOP, 365)
          .Set(Am_FIXED_WIDTH, false)
          .Add(Am_BOX_ON_LEFT, on_left_form)
          .Set(Am_FIXED_WIDTH, fixed_width_form)
          .Set(Am_FIXED_HEIGHT, fixed_height_form)
          .Set(Am_ITEMS,
               Am_Value_List()
                   .Add(button_panel_cmd1 = Am_Command.Create("FirstCmd")
                                                .Set(Am_LABEL, "First Label")
                                                .Set(Am_DO_METHOD, my_do))
                   .Add("Not A Command")
                   .Add(button_panel_rect1 = rect_in_button.Create())
                   .Add(Am_Command.Create("Label4Cmd")
                            .Set(Am_LABEL, "Label4 in Button")
                            .Set(Am_DO_METHOD, my_do))
                   .Add(Am_Command.Create("rectinCmd")
                            .Set(Am_LABEL, rect3_in_button.Create())
                            .Set(Am_ID, 15)
                            .Set(Am_DO_METHOD, my_do))
                   .Add(Am_Command.Create("abort_cmd")
                            .Set(Am_LABEL, "Abort me")
                            .Set(Am_DO_METHOD, abort_method))
                   .Add(Am_Command.Create("number_cmd")
                            .Set(Am_LABEL, 97) //number as a label
                            .Set(Am_DO_METHOD, my_do))
                   .Add(123456789) // number not in a label
               )
          .Set_Part(Am_COMMAND, Am_Command.Create("TopCmd")
                                    .Set(Am_LABEL, "Top_Level_Do")
                                    .Set(Am_DO_METHOD, my_do));

  button_panel3 =
      Am_Radio_Button_Panel.Create("Radio Button Panel 3")
          .Set(Am_LEFT, 300)
          .Set(Am_TOP, 80)
          .Set(Am_FINAL_FEEDBACK_WANTED, true) //always want to see the color
          .Set(Am_HOW_SET, Am_CHOICE_TOGGLE)   // can turn it on and off
          .Set(Am_BOX_ON_LEFT, on_left_form)
          .Set(Am_ITEMS, Am_Value_List()
                             .Add("Red")
                             .Add("Blue")
                             .Add("Green")
                             .Add("Yellow")
                             .Add("Orange"));
  Am_Object radios =
      Am_Radio_Button_Panel.Create()
          .Set(Am_LEFT, 244)
          .Set(Am_TOP, 543)
          .Set(Am_ITEM_OFFSET, 0)
          .Set(Am_BOX_ON_LEFT, on_left_form)
          .Set(Am_ITEMS,
               Am_Value_List().Add("Turkey").Add("Chicken").Add("Steak").Add(
                   "Roast Beef"));

  Am_Object color_rect = Am_Rectangle.Create("colorrect")
                             .Set(Am_WIDTH, 50)
                             .Set(Am_HEIGHT, 50)
                             .Set(Am_LEFT, 380)
                             .Set(Am_TOP, 85)
                             .Set(Am_FILL_STYLE, color_from_panel);

#ifdef DEBUG
  cmd =
#endif
      button_panel3.Get_Object(Am_COMMAND)
          .Set(Am_OBJECT_MODIFIED, color_rect)
          .Set(Am_LABEL, name_from_value);
#ifdef DEBUG
  Am_Unregister_Name(cmd);
  Am_Register_Name(cmd, "Change_Color_Command");
#endif

  menu1 =
      Am_Menu.Create("my_menu")
          .Set(Am_LEFT, 150)
          .Set(Am_TOP, 260)
          .Set(Am_ITEMS, Am_Value_List()
                             .Add("Menu item")
                             .Add("Another menu item")
                             .Add(Am_Menu_Line_Command.Create("my menu line"))
                             .Add(Am_Command.Create("item3")
                                      .Set(Am_ACTIVE, false)
                                      .Set(Am_LABEL, "Not active")
                                      .Set(Am_ACCELERATOR, "META_SHIFT_I")
                                      .Set(Am_DO_METHOD, my_do)
                                      .Add(Am_CHECKED_ITEM, true))
                             .Add(Am_Command.Create("item4")
                                      .Set(Am_LABEL, rect_in_button.Create())
                                      .Set(Am_ACCELERATOR, "CONTROL_B")
                                      .Set(Am_DO_METHOD, my_do)
                                      .Add(Am_CHECKED_ITEM, true))
                             .Add(Am_Command.Create("abort me")
                                      .Set(Am_LABEL, "Abort me")
                                      .Set(Am_DO_METHOD, abort_method))
                             .Add(3.1415926535));
  menu1.Get_Object(Am_COMMAND)
      .Set_Name("top level menu command")
      .Set(Am_DO_METHOD, my_do);

  hscroll = Am_Horizontal_Scroll_Bar.Create()
                .Set(Am_LEFT, 20)
                .Set(Am_TOP, 560)
                .Set(Am_VALUE_1, 10) //all integers, goes backwards
                .Set(Am_VALUE_2, 0)
                .Set(Am_SMALL_INCREMENT, 1)
                .Set(Am_LARGE_INCREMENT, 2);
  hscroll.Get_Object(Am_COMMAND).Set(Am_DO_METHOD, my_do);

  vscroll = Am_Vertical_Scroll_Bar.Create()
                .Set(Am_LEFT, 450)
                .Set(Am_TOP, 80)
                .Set(Am_VALUE_1, 0.0) //floats
                .Set(Am_VALUE_2, 100.0);

  vscroll.Get_Object(Am_COMMAND).Set(Am_DO_METHOD, my_do);

  Am_Screen.Add_Part(window);

  option_button =
      Am_Option_Button.Create("my_option_button")
          .Set(Am_LEFT, 20)
          .Set(Am_TOP, 600)
          .Set(Am_ITEMS, Am_Value_List()
                             .Add("Option Menu First item")
                             .Add("Another menu item")
                             .Add(Am_Menu_Line_Command.Create("my menu line"))
                             .Add(Am_Command.Create("item3")
                                      .Set(Am_ACTIVE, false)
                                      .Set(Am_LABEL, "Not active")
                                      .Set(Am_ACCELERATOR, "control_f3")
                                      .Set(Am_DO_METHOD, my_do))
                             .Add(Am_Command.Create("item4 ID=4")
                                      .Set(Am_LABEL, rect_in_button.Create())
                                      .Set(Am_ACCELERATOR, "CONTROL_f4")
                                      .Set(Am_DO_METHOD, my_do)
                                      .Set(Am_ID, 4))
                             .Add(Am_Command.Create("abort me")
                                      .Set(Am_LABEL, "Abort me")
                                      .Set(Am_DO_METHOD, abort_method)
                                      .Set(Am_ID, 5))
                             .Add("") //test an empty item
                             .Add("Last item"));
  option_button.Get_Object(Am_COMMAND)
      .Set_Name("Top_Command_in_option_button")
      .Set(Am_DO_METHOD, my_do);

  window.Add_Part(rect)
      .Add_Part(button1)
      .Add_Part(abort_button)
      .Add_Part(undobutton)
      .Add_Part(redobutton)
      .Add_Part(button4)
      .Add_Part(button5)
      .Add_Part(button6)
      .Add_Part(button7)
      .Add_Part(button8)
      .Add_Part(button_panel)
      .Add_Part(button_panel2)
      .Add_Part(button_panel3)
      .Add_Part(radios)
      .Add_Part(color_rect)
      .Add_Part(checkboxes)
      .Add_Part(menu1)
      .Add_Part(vscroll)
      .Add_Part(hscroll)
      .Add_Part(option_button)
      .Add_Part(Am_Tab_To_Next_Widget_Interactor.Create("My_Tab_Interactor"));

  Am_Object my_popup =
      Am_Pop_Up_Menu_Interactor.Create("my_popup")
          .Set(Am_START_WHEN, "RIGHT_DOWN")
          .Set(Am_FILL_STYLE, Am_Motif_Light_Green)
          .Set(Am_ITEMS, Am_Value_List()
                             .Add(Am_Command.Create("pop_1")
                                      .Set(Am_LABEL, "Pop 1")
                                      .Set(Am_DO_METHOD, my_do))
                             .Add(Am_Command.Create("pop 2")
                                      .Set(Am_LABEL, rect_in_button.Create())
                                      .Set(Am_DO_METHOD, my_do))
                             .Add(Am_Menu_Line_Command.Create("my menu line"))
                             .Add(Am_Command.Create("pop 3")
                                      .Set(Am_LABEL, "Not active")
                                      .Set(Am_ACTIVE, false)
                                      .Set(Am_DO_METHOD, my_do))
                             .Add("Last item"));
  my_popup.Get_Object(Am_COMMAND)
      .Set_Name("command_in_popup_inter")
      .Set(Am_DO_METHOD, my_do);

  window.Add_Part(my_popup);

  Am_Object updown = Am_Vertical_Up_Down_Counter.Create("Up-down")
                         .Set(Am_LEFT, 266)
                         .Set(Am_VALUE, 50)
                         .Set(Am_TOP, 413);
  updown.Get_Object(Am_COMMAND)
      .Set_Name("updown_command")
      .Set(Am_DO_METHOD, my_do);
  Am_Object updownval = Am_Number_Input_Widget.Create("up-down-string")
                            .Set(Am_LEFT, 230)
                            .Set(Am_TOP, 416)
                            .Set(Am_WIDTH, 35)
                            .Set(Am_VALUE_1, 0)
                            .Set(Am_VALUE_2, 100)
                            .Set(Am_VALUE, 50)
                            .Add(SAME_AS, updown)
                            .Set(Am_VALUE, same_as_sameas.Multi_Constraint());
  updownval.Get_Object(Am_COMMAND)
      .Set_Name("updown_string_command")
      .Set(Am_LABEL, "")
      .Set(Am_DO_METHOD, my_do);
  updown.Add(SAME_AS, updownval)
      .Set(Am_VALUE, same_as_sameas.Multi_Constraint());
  window.Add_Part(updown).Add_Part(updownval);

  scroll_menu =
      Am_Scrolling_Menu.Create("scroll_menu")
          .Set(Am_LEFT, 485)
          .Set(Am_TOP, 65)
          .Set(Am_ITEMS,
               Am_Value_List()
                   .Add("Scroll MenuFirst item")
                   .Add("Another menu item")
                   .Add(Am_Menu_Line_Command.Create())
                   .Add(Am_Command.Create("item3")
                            .Set(Am_ACTIVE, false)
                            .Set(Am_LABEL, "Not active")
                            .Set(Am_ACCELERATOR, "control_f7")
                            .Set(Am_DO_METHOD, my_do))
                   .Add(Am_Command.Create("item4 ID=4")
                            .Set(Am_LABEL, rect_in_button.Create())
                            .Set(Am_ACCELERATOR, "CONTROL_f8")
                            .Set(Am_DO_METHOD, my_do)
                            .Set(Am_ID, 4))
                   .Add(Am_Command.Create()
                            .Set(Am_LABEL, "Item 5")
                            .Set(Am_DO_METHOD, my_do)
                            .Set(Am_ID, 5))
                   .Add("") //test an empty item
                   .Add("Item7")
                   .Add(rect_in_button.Create().Set(Am_FILL_STYLE, Am_Yellow))
                   .Add("Item9")
                   .Add("Item10")
                   .Add("Item11")
                   .Add("Item12")
                   .Add("Item13")
                   .Add("Item14")
                   .Add("Item15")
                   .Add("Item16")
                   .Add("Item17")
                   .Add("Item18")
                   .Add("Item19 Is a Really long item which contains a lot of "
                        "words")
                   .Add("Item20"));
  scroll_menu.Get_Object(Am_COMMAND)
      .Set_Name("Top_Command_in_scroll_menu")
      .Set(Am_DO_METHOD, my_do);

  window.Add_Part(scroll_menu);

  scrolling_window = Am_Window.Create("Scrolling_window")
                         .Set(Am_LEFT, 600)
                         .Set(Am_TOP, 300)
                         .Set(Am_WIDTH, 220)
                         .Set(Am_HEIGHT, 320)
                         .Set(Am_FILL_STYLE, Am_Yellow)
                         .Set(Am_UNDO_HANDLER, undo_handler);

  scrolling_group =
      Am_Scrolling_Group.Create("scroll_group")
          .Set(Am_LEFT, 10)
          .Set(Am_TOP, 10)
          .Set(Am_WIDTH, owner_width_minus_20)
          .Set(Am_HEIGHT, owner_height_minus_20)
          .Add_Part(R1, Am_Rectangle.Create()
                            .Set(Am_LEFT, 0)
                            .Set(Am_TOP, 0)
                            .Set(Am_WIDTH, 15)
                            .Set(Am_HEIGHT, 15)
                            .Set(Am_FILL_STYLE, Am_Blue))
          .Add_Part(R2, Am_Rectangle.Create()
                            .Set(Am_LEFT, 380)
                            .Set(Am_TOP, 380)
                            .Set(Am_WIDTH, 20)
                            .Set(Am_HEIGHT, 20)
                            .Set(Am_FILL_STYLE, Am_Yellow))
          .Add_Part(R3, Am_Line.Create()
                            .Set(Am_X1, 400)
                            .Set(Am_Y1, 0)
                            .Set(Am_X2, 0)
                            .Set(Am_Y2, 400)
                            .Set(Am_LINE_STYLE, Am_Red))
          .Add_Part(R4, Am_Line.Create()
                            .Set(Am_X1, 0)
                            .Set(Am_Y1, 0)
                            .Set(Am_X2, 400)
                            .Set(Am_Y2, 400)
                            .Set(Am_LINE_STYLE, Am_Red))
          .Add_Part(Am_Line.Create()
                        .Set(Am_X1, 200)
                        .Set(Am_Y1, 0)
                        .Set(Am_X2, 200)
                        .Set(Am_Y2, 400)
                        .Set(Am_LINE_STYLE, Am_Blue))
          .Add_Part(Am_Line.Create()
                        .Set(Am_X1, 0)
                        .Set(Am_Y1, 200)
                        .Set(Am_X2, 400)
                        .Set(Am_Y2, 200)
                        .Set(Am_LINE_STYLE, Am_Blue))
          .Add_Part(
              R5,
              Am_Checkbox_Panel.Create()
                  .Set(Am_LEFT, 150)
                  .Set(Am_TOP, 150)
                  .Set(Am_ITEMS,
                       Am_Value_List()
                           .Add(has_v_scroll =
                                    Am_Command.Create("v_scroll")
                                        .Set(Am_LABEL, "Has Vertical Scroll")
                                        .Set(Am_ID, HAS_V))
                           .Add(has_h_scroll =
                                    Am_Command.Create("h_scroll")
                                        .Set(Am_LABEL, "Has Horizontal Scroll")
                                        .Set(Am_ID, HAS_H))
                           .Add(v_scroll_on_left =
                                    Am_Command.Create("left")
                                        .Set(Am_LABEL, "Vert Scroll on Left")
                                        .Set(Am_ID, V_ON_LEFT))
                           .Add(h_scroll_on_top =
                                    Am_Command.Create("top")
                                        .Set(Am_LABEL, "Horiz Scroll on Top")
                                        .Set(Am_ID, H_ON_TOP))))
          .Add_Part(rfeedback)
          .Add_Part(lfeedback)
          .Add_Part(R6, Am_Move_Grow_Interactor.Create("move_scrollable_obj")
                            .Set(Am_START_WHERE_TEST, Am_Inter_In_Part))
          .Add_Part(Am_New_Points_Interactor.Create("move_scrollable_obj")
                        .Set(Am_START_WHEN, "ANY_RIGHT_DOWN")
                        .Set(Am_AS_LINE, as_line_if_shift)
                        .Set(Am_FEEDBACK_OBJECT, compute_feedback_obj)
                        .Set(Am_CREATE_NEW_OBJECT_METHOD, create_new_object))

          .Set(Am_H_SCROLL_BAR, h_scroll_bar_form)
          .Set(Am_V_SCROLL_BAR, v_scroll_bar_form)
          .Set(Am_H_SCROLL_BAR_ON_TOP, h_scroll_on_top_form)
          .Set(Am_V_SCROLL_BAR_ON_LEFT, v_scroll_on_left_form);
  scrolling_window.Add_Part(R0, scrolling_group);
  Am_Screen.Add_Part(scrolling_window);

  Am_Share_Accelerators(window, scrolling_window);

  Am_Main_Event_Loop();
  Am_Cleanup();
  return 0;
}
