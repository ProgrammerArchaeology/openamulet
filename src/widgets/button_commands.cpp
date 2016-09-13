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

//Get the window from the widget, or if a popup, get the real widget's
//window.  Normally, would use the Am_SAVED_OLD_OWNER slot, but this
//doesn't work when create an instance of a window with a menubar
//since the Am_SAVED_OLD_OWNER is set *after* the check_accel_string
//constraint is evaluated
Am_Object
get_window(Am_Object &widget)
{
  Am_Object window = widget.Get(Am_WINDOW);
  if (window.Valid()) {
    if (window.Is_Instance_Of(Am_Pop_Up_Menu_From_Widget_Proto)) {
      Am_Object main_obj = window.Get(Am_FOR_ITEM);
      if (main_obj.Valid())
        window = main_obj.Get(Am_WINDOW);
      else
        window = Am_No_Object;
    }
  }
  return window;
}

/******************************************************************************
 * Am_Active_From_Command
 *   in the active slot of a widget
 */

Am_Define_Formula(bool, Am_Active_From_Command)
{
  bool ret = true;
  Am_Value v;
  v = self.Peek(Am_COMMAND);
  if (v.Valid() && v.type == Am_OBJECT) {
    Am_Object cmd = v;
    if (cmd.Is_Instance_Of(Am_Command))
      ret = cmd.Get(Am_ACTIVE);
  }
  return ret;
}

/******************************************************************************
 * Am_Active_And_Active2
 *   goes in the active slot of the interactor
 */

Am_Define_Formula(bool, Am_Active_And_Active2)
{
  bool ret = true;
  Am_Object button = self.Get_Owner(Am_NO_DEPENDENCY);
  if (button.Valid())
    ret = (bool)button.Get(Am_ACTIVE) && (bool)button.Get(Am_ACTIVE_2);
  return ret;
}

Am_Define_Formula(bool, Am_Default_From_Command)
{
  bool ret = false; // assume that we are not default if no command object
  Am_Value v;
  v = self.Peek(Am_COMMAND);
  if (v.Valid() && v.type == Am_OBJECT) {
    Am_Object cmd = v;
    if (cmd.Is_Instance_Of(Am_Command))
      ret = cmd.Get(Am_DEFAULT);
  }
  return ret;
}

Am_Define_Formula(Am_Value, am_checked_from_command)
{
  Am_Value ret = Am_No_Value; // not checked if no command object
  Am_Value v;
  v = self.Peek(Am_COMMAND);
  if (v.Valid() && v.type == Am_OBJECT) {
    Am_Object cmd = v;
    if (cmd.Is_Instance_Of(Am_Command))
      ret = cmd.Peek(Am_CHECKED_ITEM);
  }
  return ret;
}

/******************************************************************************
 * Am_Get_Owners_Command
 *   goes in the Am_IMPLEMENTATION_PARENT slot of the command in the
 *   interactor to get the Command of the widget
 */

Am_Define_Object_Formula(Am_Get_Owners_Command)
{
  //owner is inter, owner of inter is widget, get command from widget
  Am_Value v;
  Am_Object command;
  v = self.Get_Owner().Get_Owner().Peek(Am_COMMAND);
  if (v.Valid() && v.type == Am_OBJECT) {
    command = v;
    if (!command.Is_Instance_Of(Am_Command))
      command = Am_No_Object;
  }
  return command;
}

/******************************************************************************
 * set_command_from_button
 */

void
set_command_from_button(Am_Object parent_command, Am_Object button)
{
  Am_Value value = 0;
  if (button.Valid())
    value = button.Peek(Am_LABEL_OR_ID);
  parent_command.Set(Am_VALUE, value);
}

/******************************************************************************
 *   put into the Am_LABEL_OR_ID slot of the widget.  If there is a command
 *   and it has an ID field, use that value, otherwise use
 *   Am_REAL_STRING_OR_OBJ value
 */

Am_Define_Formula(Am_Value, Am_Get_Label_Or_ID)
{
  Am_Value value;
  //return will be set into value
  value = self.Peek(Am_COMMAND);
  if (value.type == Am_OBJECT) {
    Am_Object cmd = value;
    if (cmd.Is_Instance_Of(Am_Command)) {
      value = cmd.Peek(Am_ID);
      if (!value.Valid())
        value = cmd.Peek(Am_LABEL); //if no ID, then use the label
    }
  }
  //otherwise, value is already set with the correct value to use
  //Note: can't use Am_REAL_STRING_OR_OBJ since it converts values
  //into a string
  return value;
}

/******************************************************************************
 * check_accel_string
 */

Am_Define_String_Formula(check_accel_string)
{
  Am_Value cmd_value;
  Am_Object cmd_obj, old_cmd_obj, old_cmd_window;
  cmd_value = self.Peek(Am_ACCELERATOR_LIST, Am_NO_DEPENDENCY);
  if (cmd_value.Valid())
    old_cmd_obj = cmd_value;
  cmd_value = self.Peek(Am_ACCELERATOR_INTER, Am_NO_DEPENDENCY);
  if (cmd_value.Valid())
    old_cmd_window = cmd_value;
  cmd_value = self.Peek(Am_COMMAND);
  if (cmd_value.Valid() && cmd_value.type == Am_OBJECT) {
    cmd_obj = cmd_value;
    if (cmd_obj.Is_Instance_Of(Am_Command)) {
      // False,no? --ortalo
      bool cmd_active =
          self.Peek(Am_ACTIVE).Valid() && self.Peek(Am_ACTIVE_2).Valid();
      Am_Value accel_value;
      Am_Input_Char accel_char;
      accel_value = cmd_obj.Peek(Am_ACCELERATOR);
      if (Am_Input_Char::Test(accel_value))
        accel_char = accel_value;
      else if (accel_value.type == Am_STRING) {
        // convert string into right type
        Am_String sval;
        sval = accel_value;
        accel_char = Am_Input_Char((const char *)sval);
        // store it back into the slot so more efficient next time
        // (because no parsing will be needed next time)
        cmd_obj.Set(Am_ACCELERATOR, accel_char);
      }
      //      std::cout << "@@Widget " << self << " command " << cmd_obj
      //		<< " active " << cmd_active
      //		<< " label = " << self.Get(Am_REAL_STRING_OR_OBJ)
      //		<< " accel " << accel_char << std::endl << std::flush;
      // now do comparison
      if (accel_char.Valid()) { //then is a legal accelerator character
        char s[Am_LONGEST_CHAR_STRING];
        accel_char.As_Short_String(s);
        Am_Object window = get_window(self);

        if ((old_cmd_obj != cmd_obj || old_cmd_window != window)) {
          if (old_cmd_obj.Valid() && old_cmd_window.Valid())
            Am_Remove_Accelerator_Command_From_Window(old_cmd_obj,
                                                      old_cmd_window);
          if (window.Valid() && cmd_active) {
            Am_Add_Accelerator_Command_To_Window(cmd_obj, window);
            self.Set(Am_ACCELERATOR_LIST, cmd_obj, Am_OK_IF_NOT_THERE);
            self.Set(Am_ACCELERATOR_INTER, window, Am_OK_IF_NOT_THERE);
          } else {
            self.Set(Am_ACCELERATOR_LIST, (0L), Am_OK_IF_NOT_THERE);
            self.Set(Am_ACCELERATOR_INTER, (0L), Am_OK_IF_NOT_THERE);
          }
        } else if (!cmd_active && old_cmd_obj.Valid() &&
                   old_cmd_window.Valid()) {
          if (window.Valid())
            Am_Remove_Accelerator_Command_From_Window(old_cmd_obj,
                                                      old_cmd_window);
          self.Set(Am_ACCELERATOR_LIST, (0L), Am_OK_IF_NOT_THERE);
          self.Set(Am_ACCELERATOR_INTER, (0L), Am_OK_IF_NOT_THERE);
        }
        //std::cout << "Returning str=" << Am_String(s) <<std::endl <<std::flush;
        return Am_String(s);
      }
    }
  }
  if (old_cmd_obj.Valid() && old_cmd_window.Valid()) {
    Am_Remove_Accelerator_Command_From_Window(old_cmd_obj, old_cmd_window);
    self.Set(Am_ACCELERATOR_LIST, (0L), Am_OK_IF_NOT_THERE);
    self.Set(Am_ACCELERATOR_INTER, (0L), Am_OK_IF_NOT_THERE);
  }

  // no accel string
  return (0L);
}

Am_Define_Formula(Am_Value, Am_Align_From_Box_On_Left)
{
  return (bool)self.Get(Am_BOX_ON_LEFT) ? Am_LEFT_ALIGN : Am_RIGHT_ALIGN;
}
