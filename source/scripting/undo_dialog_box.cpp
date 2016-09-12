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

// Support for scripting and selective undo
//
// Designed and implemented by Brad Myers

#include <am_inc.h>

#include AM_IO__H

#include STANDARD_SLOTS__H
#include VALUE_LIST__H
#include UNDO_DIALOG__H
#include WIDGETS_ADVANCED__H //for accelerator registering
#include INTER_ADVANCED__H   //for Am_Modify_Command_Label_Name

#include STR_STREAM__H

#include WIDGETS__H
#include INTER__H
#include OPAL__H
#include DEBUGGER__H
#include INITIALIZER__H

#include OPAL_ADVANCED__H //for Am_Update_All during scripts

#include SCRIPTING_ADVANCED__H
#include SCRIPTING__H

Am_Object Am_Undo_Dialog_Box;
Am_Object Am_Execute_Script_Command;
Am_Object Am_Show_Script_Command;

Am_Object Am_Script_Line_Group, Am_Script_Line_Value_Group,
    Am_Script_Line_Command, Am_Script_Line_Selectable, Am_Script_List_Header,
    Am_Script_Line_Modified, Am_Script_Line_Value;

Am_Slot_Key Am_UNDO_HANDLER_TO_DISPLAY =
    Am_Register_Slot_Name("~UNDO_HANDLER_TO_DISPLAY~");
Am_Slot_Key Am_UNDO_LABEL = Am_Register_Slot_Name("~UNDO_LABEL~");
Am_Slot_Key Am_UNDO_SCROLL_GROUP = Am_Register_Slot_Name("~UNDO_SCROLL_GROUP~");
Am_Slot_Key Am_UNDO_MENU_BAR = Am_Register_Slot_Name("~Am_UNDO_MENU_BAR~");
Am_Slot_Key Am_UNDO_MENU_OF_COMMANDS =
    Am_Register_Slot_Name("~UNDO_MENU_OF_COMMANDS~");
Am_Slot_Key Am_SCROLLING_GROUP_SLOT =
    Am_Register_Slot_Name("~SCROLLING_GROUP~");
Am_Slot_Key Am_UNDO_OPTIONS = Am_Register_Slot_Name("~UNDO_OPTIONS~");
Am_Slot_Key Am_EXECUTE_SCRIPT_ON_NEW =
    Am_Register_Slot_Name("~Am_EXECUTE_SCRIPT_ON_NEW~");
Am_Slot_Key Am_UNDO_DIALOG_BOX_SLOT = Am_Register_Slot_Name("UNDO_DIALOG_BOX");
Am_Slot_Key Am_SCRIPT_EXECUTE_COMMAND =
    Am_Register_Slot_Name("Am_SCRIPT_EXECUTE_COMMAND");
Am_Slot_Key Am_SHOW_SCRIPT_COMMAND =
    Am_Register_Slot_Name("Am_SHOW_SCRIPT_COMMAND");
Am_Slot_Key Am_NAME_OBJECT = Am_Register_Slot_Name("~Am_NAME_OBJECT~");
Am_Slot_Key Am_ORIGINAL_COMMAND_LIST =
    Am_Register_Slot_Name("~Am_ORIGINAL_COMMAND_LIST~");
Am_Slot_Key Am_COMMANDS_IN_SCRIPT =
    Am_Register_Slot_Name("~Am_COMMANDS_IN_SCRIPT~");
Am_Slot_Key Am_COMMANDS_PROTOTYPES =
    Am_Register_Slot_Name("~Am_COMMANDS_PROTOTYPES~");
Am_Slot_Key Am_COMMAND_INDEX_IN_SCRIPT =
    Am_Register_Slot_Name("~Am_COMMAND_INDEX_IN_SCRIPT~");
Am_Slot_Key Am_SCRIPT_WINDOW = Am_Register_Slot_Name("~Am_SCRIPT_WINDOW~");
Am_Slot_Key Am_PLACEHOLDER_LIST =
    Am_Register_Slot_Name("~Am_PLACEHOLDER_LIST~");
Am_Slot_Key Am_PLACEHOLDER_COUNT =
    Am_Register_Slot_Name("~Am_PLACEHOLDER_COUNT~");
Am_Slot_Key Am_TOP_SCRIPT_LINE_GROUP =
    Am_Register_Slot_Name("~Am_TOP_SCRIPT_LINE_GROUP~");
Am_Slot_Key Am_LOCATION_PLACEHOLDER =
    Am_Register_Slot_Name("~Am_LOCATION_PLACEHOLDER~");

bool am_sdebug = false;

Am_Object
get_undo_handler(Am_Object cmd)
{
  Am_Object panel, undo_handler;
  //Am_SAVED_OLD_OWNER of cmd is button_panel, owner is db
  panel = cmd.Get(Am_SAVED_OLD_OWNER);
  undo_handler = panel.Get_Owner().Get(Am_UNDO_HANDLER_TO_DISPLAY);
  return undo_handler;
}

Am_Object
if_list_get_obj(Am_Value v, bool only_if_one)
{
  if (Am_Value_List::Test(v)) {
    Am_Value_List v_list;
    v_list = v;
    if (!only_if_one || (v_list.Length() == 1)) {
      v_list.Start();
      Am_Object o = v_list.Get();
      return o;
    }
  }
  return Am_No_Object;
}

Am_Object
output_obj_or_list(Am_Value &object_modified_value, OSTRSTREAM &oss)
{
  Am_Object object_modified = if_list_get_obj(object_modified_value, true);
  if (object_modified.Valid())
    oss << object_modified;
  else {
    if (object_modified_value.type == Am_OBJECT)
      object_modified = object_modified_value;
    oss << object_modified_value;
  }
  return object_modified;
}

void
am_write_command_name(Am_Object cmd, int cnt, OSTRSTREAM &oss)
{
  Am_Value v, object_modified_value, new_value;
  Am_Object object_modified;
  v = cmd.Peek(Am_SHORT_LABEL, Am_NO_DEPENDENCY);
  if (!v.Valid())
    v = cmd.Peek(Am_LABEL, Am_NO_DEPENDENCY);
  object_modified_value = cmd.Peek(Am_OBJECT_MODIFIED, Am_NO_DEPENDENCY);
  new_value = cmd.Peek(Am_VALUE, Am_NO_DEPENDENCY);
  oss << cnt;
  oss << ". " << v << " ";
  if (object_modified_value.Valid())
    object_modified = output_obj_or_list(object_modified_value, oss);
  if (new_value.Exists() &&
      (!object_modified.Valid() || new_value.type != Am_OBJECT ||
       Am_Object(new_value) != object_modified)) {
    //don't output new_value if same as object_modified (as in create commands)
    oss << " = " << new_value;
  }
  oss << std::ends;
}

//generates a list of command objects whose labels are the right commands and
Am_Define_Value_List_Formula(generate_undo_menu_items)
{
  //don't do anything if my window is invisible
  Am_Object window = self.Get(Am_WINDOW);
  if (!window.Valid() || !(bool)window.Get(Am_VISIBLE))
    return (0L);
  Am_Object db, undo_handler;
  //self is  Am_UNDO_SCROLL_GROUP, owner is db
  db = self.Get_Owner();
  if (db.Valid())
    undo_handler = db.Get(Am_UNDO_HANDLER_TO_DISPLAY);

  if (!undo_handler.Valid())
    return (0L);
  Am_Value_List cur_commands, menu_commands;
  cur_commands = undo_handler.Get(Am_COMMAND);
  Am_Object cmd;
  char line[300];

  int cnt = cur_commands.Length();

  for (cur_commands.Start(); !cur_commands.Last(); cur_commands.Next(), cnt--) {
    OSTRSTREAM_CONSTR(oss, line, 300, std::ios::out);
    reset_ostrstream(oss); // go to the beginning
    cmd = cur_commands.Get();
    am_write_command_name(cmd, cnt, oss);
    OSTRSTREAM_COPY(oss, line, 300);
    cmd.Set(Am_RANK, cnt, Am_OK_IF_NOT_THERE);
    menu_commands.Add(Am_Command.Create().Set(Am_LABEL, line).Set(Am_ID, cmd));
  } //end loop

  //call Note_Changed on the currently selected command so the
  //constraints will re-check whether the operations are still legal
  self.Note_Changed(Am_VALUE);
  return menu_commands;
}

//returns the command to undo if there is exactly one selected in the
//scrolling menu, or (0L) if none or more than one selected.
Am_Object
get_command_to_undo(Am_Object cmd)
{
  Am_Value value = cmd.Get_Object(Am_SAVED_OLD_OWNER)
                       .Get_Sibling(Am_UNDO_SCROLL_GROUP)
                       .Peek(Am_VALUE);
  if (value.Valid()) {
    Am_Value_List l = value;
    if (l.Length() == 1) {
      l.Start();
      // because id is the cmd to undo, it will be the value of the menu
      Am_Object undo_cmd = l.Get();
      return (undo_cmd);
    }
  }
  return Am_No_Object;
}

//returns the selected commands to undo from the scrolling menu.  List
//will be empty if none
Am_Value_List
get_commands_to_undo(Am_Object cmd)
{
  Am_Value value = cmd.Get_Object(Am_SAVED_OLD_OWNER)
                       .Get_Sibling(Am_UNDO_SCROLL_GROUP)
                       .Peek(Am_VALUE);
  Am_Value_List result;
  if (value.Valid())
    result = value;
  return result;
}

//call notechanged on the currently selected command so the
//constraints will re-check whether the operations are still legal
void
mark_changed_command_to_undo(Am_Object cmd)
{
  Am_Object menu, panel;
  panel = cmd.Get(Am_SAVED_OLD_OWNER);
  menu = panel.Get_Sibling(Am_UNDO_SCROLL_GROUP);
  menu.Note_Changed(Am_VALUE);
}

Am_Define_Formula(bool, last_undoable)
{
  Am_Object last_command;
  Am_Object undo_handler = get_undo_handler(self);
  if (undo_handler.Valid())
    last_command = undo_handler.Get(Am_UNDO_ALLOWED);
  return last_command.Valid();
}

Am_Define_Method(Am_Object_Method, void, do_undo_last, (Am_Object cmd))
{
  Am_Object undo_handler = get_undo_handler(cmd);
  Am_Object_Method undoit;
  undoit = undo_handler.Get(Am_PERFORM_UNDO);
  undoit.Call(undo_handler);
  mark_changed_command_to_undo(cmd);
}

Am_Define_Formula(bool, redoable)
{
  Am_Object last_command;
  Am_Object undo_handler = get_undo_handler(self);
  if (undo_handler.Valid())
    last_command = undo_handler.Get(Am_REDO_ALLOWED);
  return last_command.Valid();
}

Am_Define_Method(Am_Object_Method, void, am_do_redo, (Am_Object cmd))
{
  Am_Object undo_handler = get_undo_handler(cmd);
  Am_Object_Method redoit;
  redoit = undo_handler.Get(Am_PERFORM_REDO);
  redoit.Call(undo_handler);
  mark_changed_command_to_undo(cmd);
}

Am_Define_Method(Am_Object_Method, void, do_undo_this, (Am_Object cmd))
{
  Am_Object command_to_undo = get_command_to_undo(cmd);
  Am_Object undo_handler = get_undo_handler(cmd);
  if (command_to_undo.Valid()) {
    Am_Handler_Selective_Undo_Method method;
    method = undo_handler.Get(Am_SELECTIVE_UNDO_METHOD);
    Am_Object new_cmd = method.Call(undo_handler, command_to_undo);
  }
  mark_changed_command_to_undo(cmd);
}

Am_Define_Formula(bool, exists_selection)
{
  Am_Object cmd = get_command_to_undo(self);
  return cmd.Valid();
}

Am_Define_Formula(bool, exists_one_or_more_selection)
{
  Am_Value_List cmds = get_commands_to_undo(self);
  return cmds.Valid();
}

Am_Define_Formula(bool, selection_undoable)
{
  Am_Object cmd = get_command_to_undo(self);
  bool allowed = false;
  if (cmd.Valid()) {
    // because id is the cmd to undo, it will be the value of the menu
    Am_Selective_Allowed_Method allowed_method;
    Am_Object undo_handler = get_undo_handler(self);
    if (undo_handler.Valid()) {
      allowed_method =
          undo_handler.Get(Am_SELECTIVE_UNDO_ALLOWED, Am_NO_DEPENDENCY);
      if (allowed_method.Valid())
        allowed = allowed_method.Call(cmd);
    }
  }
  return allowed;
}

Am_Define_Method(Am_Object_Method, void, do_repeat_this, (Am_Object cmd))
{
  Am_Object command_to_undo = get_command_to_undo(cmd);
  Am_Object undo_handler = get_undo_handler(cmd);
  if (command_to_undo.Valid()) {
    Am_Handler_Selective_Undo_Method method;
    method = undo_handler.Get(Am_SELECTIVE_REPEAT_SAME_METHOD);
    if (method.Valid()) {
      Am_Object new_cmd = method.Call(undo_handler, command_to_undo);
    }
  }
  mark_changed_command_to_undo(cmd);
}

Am_Define_Formula(bool, selection_repeatable)
{
  Am_Object cmd = get_command_to_undo(self);
  bool allowed = false;
  if (cmd.Valid()) {
    // because id is the cmd to undo, it will be the value of the menu
    Am_Object undo_handler = get_undo_handler(self);
    if (undo_handler.Valid()) {
      Am_Selective_Allowed_Method allowed_method;
      allowed_method =
          undo_handler.Get(Am_SELECTIVE_REPEAT_SAME_ALLOWED, Am_NO_DEPENDENCY);
      if (allowed_method.Valid())
        allowed = allowed_method.Call(cmd);
    }
  }
  return allowed;
}

void
get_current_sel(Am_Object cmd, Am_Value &cur_sel)
{
  //saved_old_owner is panel, owner of panel is db, which has the
  //selection handles widget, and want its value
  Am_Object sel_widget =
      cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner().Get(Am_SELECTION_WIDGET);
  if (sel_widget.Valid())
    cur_sel = sel_widget.Peek(Am_VALUE);
  else
    cur_sel = Am_No_Value;
}

Am_Define_Method(Am_Object_Method, void, do_repeat_this_on_new, (Am_Object cmd))
{
  Am_Object command_to_undo = get_command_to_undo(cmd);
  Am_Object undo_handler = get_undo_handler(cmd);
  if (command_to_undo.Valid()) {
    Am_Value cur_sel;
    get_current_sel(cmd, cur_sel);
    Am_Handler_Selective_Repeat_New_Method method;
    method = undo_handler.Get(Am_SELECTIVE_REPEAT_ON_NEW_METHOD);
    Am_Object new_cmd =
        method.Call(undo_handler, command_to_undo, cur_sel, Am_No_Value);
  }
  mark_changed_command_to_undo(cmd);
}

Am_Define_Formula(bool, selection_repeatable_on_new)
{
  Am_Object cmd = get_command_to_undo(self);
  bool allowed = false;
  if (cmd.Valid()) {
    Am_Object undo_handler = get_undo_handler(self);
    if (undo_handler.Valid()) {
      // because id is the cmd to undo, it will be the value of the menu
      Am_Value cur_sel;
      get_current_sel(self, cur_sel);
      Am_Selective_New_Allowed_Method allowed_method;
      allowed_method =
          undo_handler.Get(Am_SELECTIVE_REPEAT_NEW_ALLOWED, Am_NO_DEPENDENCY);
      if (allowed_method.Valid())
        allowed = allowed_method.Call(cmd, cur_sel, Am_No_Value);
    }
  }
  return allowed;
}

Am_Define_Method(Am_Object_Method, void, do_flash, (Am_Object cmd))
{
#ifdef DEBUG
  Am_Object command_to_flash = get_command_to_undo(cmd);
  Am_Object object_modified;
  if (command_to_flash.Valid()) {
    Am_Value object_modified_value;
    object_modified_value = command_to_flash.Peek(Am_OBJECT_MODIFIED);
    if (object_modified_value.type == Am_OBJECT) {
      object_modified = object_modified_value;
      Am_Flash(object_modified, std::cout);
    } else {
      // ** TEMP, handle longer lists also **
      object_modified = if_list_get_obj(object_modified_value, true);
      if (object_modified.Valid())
        Am_Flash(object_modified, std::cout);
    }
  }
#else
  std::cout << "*** Unable to flash in this version: inspector disabled ***"
            << std::endl
            << std::flush;
#endif
}

Am_Define_Formula(bool, selection_flashable)
{
#ifdef DEBUG
  Am_Object command_to_flash = get_command_to_undo(self);
  if (command_to_flash.Valid()) {
    Am_Value object_modified_value;
    object_modified_value =
        command_to_flash.Peek(Am_OBJECT_MODIFIED, Am_NO_DEPENDENCY);
    // ** TEMP, handle lists also *******
    if (object_modified_value.type == Am_OBJECT)
      return true;
    else {
      Am_Object object_modified = if_list_get_obj(object_modified_value, true);
      if (object_modified.Valid())
        return true;
    }
  }
  return false;
#else
  return false;
#endif
}

//return true if any one of test_vals is in in_list
bool
any_list_member(Am_Value_List &test_vals, Am_Value_List &in_list)
{
  Am_Value test_val;
  if (am_sdebug)
    std::cout << "testing for " << test_vals << " as member of " << in_list
              << std::endl
              << std::flush;
  for (test_vals.Start(); !test_vals.Last(); test_vals.Next()) {
    test_val = test_vals.Get();
    if (in_list.Start_Member(test_val))
      return true;
  }
  return false;
}

Am_Define_Method(Am_Object_Method, void, do_find_command_for_selection,
                 (Am_Object cmd))
{
  Am_Value_List objects = cmd.Get_Object(Am_SAVED_OLD_OWNER)
                              .Get_Owner()
                              .Get_Object(Am_SELECTION_WIDGET)
                              .Get(Am_VALUE);
  Am_Value_List cmds_using_objects;
  Am_Object menu =
      cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Sibling(Am_UNDO_SCROLL_GROUP);
  Am_Value_List all_cmds = menu.Get(Am_ITEMS);
  Am_Object this_cmd, real_cmd;
  Am_Value obj_mod_val;
  Am_Value_List obj_mod_list;
  if (am_sdebug)
    std::cout << "selected " << objects << " all commands " << all_cmds
              << std::endl
              << std::flush;
  for (all_cmds.Start(); !all_cmds.Last(); all_cmds.Next()) {
    this_cmd = all_cmds.Get();
    real_cmd = this_cmd.Get(Am_ID);
    obj_mod_val = real_cmd.Get(Am_OBJECT_MODIFIED);
    if (am_sdebug)
      std::cout << "  for cmd " << this_cmd << " real " << real_cmd
                << " obj mod " << obj_mod_val << std::endl
                << std::flush;
    if (obj_mod_val.Valid()) {
      if (obj_mod_val.type == Am_OBJECT) {
        obj_mod_list.Make_Empty();
        obj_mod_list.Add(obj_mod_val);
      } else if (Am_Value_List::Test(obj_mod_val)) {
        obj_mod_list = obj_mod_val;
      } else
        Am_ERROR("Object Modified of " << this_cmd << " is not list or obj "
                                       << obj_mod_val);
      if (any_list_member(objects, obj_mod_list))
        cmds_using_objects.Add(this_cmd);
    }
  }
  menu.Set(Am_VALUE, cmds_using_objects);
}

Am_Define_Formula(bool, selection_is_multiple)
{
  Am_Object command = get_command_to_undo(self);
  if (command.Valid()) {
    Am_Value object_modified_value;
    object_modified_value = command.Peek(Am_OBJECT_MODIFIED, Am_NO_DEPENDENCY);
    if (Am_Value_List::Test(object_modified_value)) {
      Am_Value_List obj_list;
      obj_list = object_modified_value;
      if (obj_list.Length() > 1)
        return true;
    }
  }
  return false;
}

Am_Define_Method(Am_Object_Method, void, do_dismiss, (Am_Object cmd))
{
  Am_Object panel, window;
  panel = cmd.Get(Am_SAVED_OLD_OWNER);
  window = panel.Get(Am_WINDOW);
  if (window.Valid())
    window.Set(Am_VISIBLE, false);
}

Am_Font bold_bigger_font =
    Am_Font(Am_FONT_SERIF, true, false, false, Am_FONT_LARGE);

Am_Define_Formula(int, button_panel_top_formula)
{
  int height = self.Get_Object(Am_WINDOW).Get(Am_HEIGHT);
  int my_height = self.Get(Am_HEIGHT);
  return height - my_height - 5;
}

/* ******
  Am_Define_String_Formula(toggle_record_selections_label) {
  Am_Value recording;
  recording = self.PV(Am_VALUE);
  if (recording.Valid()) return Am_String("Don't Record Selections");
  else return Am_String("Record Selections");
}
Am_Define_String_Formula(toggle_record_scrolling_label) {
  Am_Value recording;
  recording = self.PV(Am_VALUE);
  if (recording.Valid()) return Am_String("Don't Record Scrolling");
  else return Am_String("Record Scrolling");
}
***** */

Am_Define_Formula(int, check_panel_left)
{
  return (int)self.Get_Object(Am_WINDOW).Get(Am_WIDTH) -
         (int)self.Get(Am_WIDTH) - 2;
}

Am_Define_Method(Am_Object_Method, void, toggle_record_scrolling,
                 (Am_Object cmd))
{
  Am_Value recording_value;
  recording_value = cmd.Peek(Am_VALUE);
  int recording;
  if (recording_value.Valid())
    recording = 0;
  else
    recording = Am_NOT_USUALLY_UNDONE;
  Am_Object panel, scroll_group;
  panel = cmd.Get(Am_SAVED_OLD_OWNER);
  scroll_group = panel.Get_Owner().Get(Am_SCROLLING_GROUP_SLOT);
  if (scroll_group.Valid()) {
    scroll_group.Set(Am_COMMAND, recording);
  }
}

Am_Define_Method(Am_Object_Method, void, toggle_record_selections,
                 (Am_Object cmd))
{
  Am_Value recording_value;
  recording_value = cmd.Peek(Am_VALUE);
  int recording;
  if (recording_value.Valid())
    recording = 0;
  else
    recording = Am_NOT_USUALLY_UNDONE;
  Am_Object panel, sel_widget, sel_command;
  panel = cmd.Get(Am_SAVED_OLD_OWNER);
  sel_widget = panel.Get_Owner().Get(Am_SELECTION_WIDGET);
  if (sel_widget.Valid()) {
    sel_command = sel_widget.Get_Object(Am_COMMAND);
    if (sel_command.Valid())
      sel_command.Set(Am_IMPLEMENTATION_PARENT, recording);
  }
}

///////////////////////////////////////////////////////////////////////////
// Scripting commands
///////////////////////////////////////////////////////////////////////////

Am_String
gen_script_name()
{
  static int script_count = 1;
  char line[50];
  OSTRSTREAM_CONSTR(oss, line, 50, std::ios::out);
  reset_ostrstream(oss); // go to the beginning
  oss << "Script" << script_count << std::ends;
  OSTRSTREAM_COPY(oss, line, 50);
  script_count++;
  Am_String ret(line);
  return ret;
}

Am_Value_List script_windows;
int num_vis_script_windows = 0;

Am_Object
am_get_db_from_list(Am_Object &proto, Am_Value_List &db_list, int &cnt)
{
  Am_Object w;
  if (!db_list.Empty()) {
    w = db_list.Pop();
  } else {
    w = proto.Create();
    Am_Screen.Add_Part(w);
  }
  if (am_sdebug)
    std::cout << "window  " << w << std::endl << std::flush;

  w.Set(Am_LEFT,
        ((int)Am_Screen.Get(Am_WIDTH) - (int)w.Get(Am_WIDTH)) / 2 + cnt * 25)
      .Set(Am_TOP, ((int)Am_Screen.Get(Am_HEIGHT) - (int)w.Get(Am_HEIGHT)) / 2 +
                       cnt * 25);
  cnt++;
  return w;
}

Am_Object
get_script_window(Am_Object &undo_db)
{
  Am_Object w = am_get_db_from_list(Am_Script_Window, script_windows,
                                    num_vis_script_windows);
  w.Set(Am_UNDO_MENU_OF_COMMANDS, undo_db, Am_OK_IF_NOT_THERE)
      .Set(Am_SCRIPT_EXECUTE_COMMAND, (0L));
  return w;
}

void
done_with_script_window(Am_Object sw)
{
  sw.Set(Am_VISIBLE, false);
  sw.Set(Am_SCRIPT_EXECUTE_COMMAND, (0L));
  script_windows.Add(sw);
  num_vis_script_windows--;
}

Am_Define_Method(Am_Object_Method, void, script_window_destroy_method,
                 (Am_Object sw))
{
  done_with_script_window(sw);
}

Am_Define_Method(Am_Object_Method, void, script_window_cancel, (Am_Object cmd))
{
  Am_Object script_window = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  done_with_script_window(script_window);
}

char
find_placeholder_char(Am_Object &cmd, Am_Slot_Key slot)
{
  Am_Value_List placeholders =
      cmd.Get(Am_PLACEHOLDERS_SET_BY_THIS_COMMAND, Am_RETURN_ZERO_ON_ERROR);
  if (placeholders.Valid()) {
    Am_Value v;
    Am_Object ph;
    if (am_sdebug)
      std::cout << "searching placeholders " << placeholders << " for slot "
                << slot << std::endl
                << std::flush;
    Am_Slot_Key found_slot;
    for (placeholders.Start(); !placeholders.Last(); placeholders.Next()) {
      ph = placeholders.Get();
      found_slot = (Am_Slot_Key)(int)ph.Get(Am_SLOT_FOR_VALUE);
      if (found_slot == slot) {
        char ret = ph.Get(Am_ID);
        return ret;
      }
    }
  }
  return 0;
}

//mutually recursive, need forward declaration
Am_Object gen_script_list_or_value(char *line, OSTRSTREAM &oss,
                                   Am_Object &proto, Am_Object &group,
                                   Am_Object &cmd, Am_Value &val,
                                   bool start_stream, Am_Object &top_group,
                                   Am_Value_List &part_chain);

Am_Object
gen_script_value_list(char *line, OSTRSTREAM &oss, Am_Object &proto,
                      Am_Object &group, Am_Object &cmd, Am_Value &val,
                      bool start_stream, Am_Object &top_group,
                      Am_Value_List &part_chain)
{
  if (start_stream)
    reset_ostrstream(oss); // go to the beginning
  Am_Value_List l = val;
  oss << "LIST(" << l.Length() << ")" << std::ends;
  OSTRSTREAM_COPY(oss, line, 300);
  Am_Object sub_group = Am_Script_Line_Value_Group.Create()
                            .Add(Am_SLOTS_TO_SAVE, proto.Get(Am_SLOTS_TO_SAVE))
                            .Add(Am_COMMAND, cmd)
                            .Add(Am_PART_CHAIN, part_chain)
                            .Set(Am_VALUE, val);
  group.Add_Part(sub_group);
  sub_group.Add_Part(Am_Script_List_Header.Create()
                         .Set(Am_TEXT, line)
                         .Add(Am_VALUE, Am_No_Value) //just the prefix
                         .Add(Am_COMMAND, cmd)
                         .Add(Am_TOP_SCRIPT_LINE_GROUP, top_group)
                         .Add(Am_PART_CHAIN, part_chain));
  Am_Value sub_value;
  int cnt = 0;
  Am_Value_List new_part_chain;
  for (l.Start(); !l.Last(); l.Next(), cnt++) {
    sub_value = l.Get();
    new_part_chain = part_chain;
    new_part_chain.Add(cnt);
    gen_script_list_or_value(line, oss, proto, sub_group, cmd, sub_value, true,
                             top_group, new_part_chain);
  }
  return sub_group;
}

Am_Object
gen_script_list_or_value(char *line, OSTRSTREAM &oss, Am_Object &proto,
                         Am_Object &group, Am_Object &cmd, Am_Value &val,
                         bool start_stream, Am_Object &top_group,
                         Am_Value_List &part_chain)
{
  Am_Object gen_line;
  if (start_stream)
    reset_ostrstream(oss); // go to the beginning
  if (Am_Value_List::Test(val) && Am_Value_List(val).Length() > 1) {
    gen_line = gen_script_value_list(line, oss, proto, group, cmd, val, false,
                                     top_group, part_chain);
    return gen_line;
  } else {
    Am_Value use_val;
    if (Am_Value_List::Test(val))
      use_val = Am_Value_List(val).Get_First();
    else
      use_val = val;
    oss << " " << use_val << std::ends;
    OSTRSTREAM_COPY(oss, line, 300);
    group.Add_Part(gen_line = proto.Create()
                                  .Set(Am_TEXT, line)
                                  .Add(Am_VALUE, use_val)
                                  .Add(Am_COMMAND, cmd)
                                  .Add(Am_TOP_SCRIPT_LINE_GROUP, top_group)
                                  .Add(Am_PART_CHAIN, part_chain));
    return gen_line;
  }
}

void
am_check_list_slot(Am_Slot_Key slot, Am_Value &v, Am_Object &created_obj,
                   Am_Value_List &slots_to_save, Am_Value_List &pos_slots)
{
  if (slots_to_save.Start_Member(slot)) {
    slots_to_save.Delete(false); //remove it
    pos_slots.Add(slot);
    if (created_obj.Valid())
      v = created_obj.Get(slot);
  }
}

Am_Object
gen_value_script_list_or_value(char *line, Am_Object &group, Am_Object &cmd,
                               Am_Value &v, Am_Slot_Key slot, bool output_eq,
                               bool output_slot_name)
{
  OSTRSTREAM_CONSTR(oss3, line, 300, std::ios::out);
  reset_ostrstream(oss3); // go to the beginning
  if (output_eq) {
    char placeholder_char = find_placeholder_char(cmd, Am_VALUE);
    if (placeholder_char)
      oss3 << " " << placeholder_char << ":";
    oss3 << " :";
  }
  if (output_slot_name) {
    oss3 << " " << Am_Am_Slot_Key(slot) << "=";
  }
  Am_Object script_line_part =
      gen_script_list_or_value(line, oss3, Am_Script_Line_Value, group, cmd, v,
                               false, group, Am_No_Value_List);
  script_line_part.Set(Am_SLOTS_TO_SAVE, slot, Am_OK_IF_NOT_THERE);
  return script_line_part;
}

bool
is_placeholder_or_list(Am_Value &v)
{
  if (v.Valid()) {
    if (v.type == Am_OBJECT) {
      if (Am_Object(v).Is_Instance_Of(Am_A_Placeholder))
        return true;
    } else if (Am_Value_List::Test(v)) {
      Am_Value_List l = v;
      Am_Value subv;
      for (l.Start(); !l.Last(); l.Next()) {
        subv = l.Get();
        if (subv.type == Am_OBJECT &&
            Am_Object(subv).Is_Instance_Of(Am_A_Placeholder))
          return true;
      }
    }
  }
  return false;
}

void
gen_create_parameters_line(Am_Object &cmd, char *line, Am_Object &group)
{
  Am_Object created_obj = cmd.Get(Am_START_OBJECT);
  Am_Value_List slots_to_save = created_obj.Get(Am_SLOTS_TO_SAVE);
  //make sure have local copy, since are going to remove slots
  slots_to_save = slots_to_save.Copy();
  if (am_sdebug)
    std::cout << "gen for " << cmd << " found slots " << slots_to_save
              << " in object " << created_obj << std::endl
              << std::flush;
  Am_Value a, b, c, d;
  Am_Value_List pos_slots;
  Am_Value v;
  bool output_eq = true;
  bool as_line = false;
  Am_Object script_line_part, ph;
  ph = cmd.Get(Am_LOCATION_PLACEHOLDER, Am_RETURN_ZERO_ON_ERROR);
  if (ph.Valid()) {
    v = ph;
    pos_slots = ph.Get(Am_LOCATION_PLACEHOLDER);
    slots_to_save.Set_Difference(pos_slots);
    gen_value_script_list_or_value(line, group, cmd, v, Am_LOCATION_PLACEHOLDER,
                                   output_eq, false);
    output_eq = false;
  } else {
    am_check_list_slot(Am_LEFT, a, created_obj, slots_to_save, pos_slots);
    am_check_list_slot(Am_TOP, b, created_obj, slots_to_save, pos_slots);
    am_check_list_slot(Am_WIDTH, c, created_obj, slots_to_save, pos_slots);
    am_check_list_slot(Am_HEIGHT, d, created_obj, slots_to_save, pos_slots);
    bool growing = (c.Exists() || d.Exists());
    if (!a.Exists() && !b.Exists() && !c.Exists() && !d.Exists()) {
      as_line = true;
      growing = true;
      am_check_list_slot(Am_X1, a, created_obj, slots_to_save, pos_slots);
      am_check_list_slot(Am_Y1, b, created_obj, slots_to_save, pos_slots);
      am_check_list_slot(Am_X2, c, created_obj, slots_to_save, pos_slots);
      am_check_list_slot(Am_Y2, d, created_obj, slots_to_save, pos_slots);
    }
    if (a.Exists() || b.Exists() || c.Exists() || d.Exists()) {
      int ia = (a.Exists() ? (int)a : 0);
      int ib = (b.Exists() ? (int)b : 0);
      int ic = (c.Exists() ? (int)c : 0);
      int id = (d.Exists() ? (int)d : 0);
      Am_Object owner = created_obj.Get_Owner();
      if (!owner.Valid()) {
        owner = cmd.Get_Object(Am_OBJECT_MODIFIED).Get_Owner();
      }
      Am_Inter_Location create_loc(as_line, owner, ia, ib, ic, id, growing);
      v = create_loc;
      Am_Object script_line = gen_value_script_list_or_value(
          line, group, cmd, v, Am_LOCATION_PLACEHOLDER, output_eq, false);
      script_line.Set(Am_LOCATION_PLACEHOLDER, pos_slots, Am_OK_IF_NOT_THERE);
      output_eq = false;
    }
  }
  Am_Slot_Key slot;
  for (slots_to_save.Start(); !slots_to_save.Last(); slots_to_save.Next()) {
    slot = (Am_Slot_Key)(int)slots_to_save.Get();
    if (slot != Am_LOCATION_PLACEHOLDER) {
      v = cmd.Peek(slot);
      if (!is_placeholder_or_list(v))
        v = created_obj.Peek(slot);
      if (!v.Exists())
        v = 0; //use regular zero
      gen_value_script_list_or_value(line, group, cmd, v, slot, output_eq,
                                     true);
      output_eq = false;
    }
  }
}

Am_Object
am_gen_one_script_line(Am_Object &cmd)
{
  Am_Value cnt = cmd.Peek(Am_COMMAND_INDEX_IN_SCRIPT);
  char line[300];
  OSTRSTREAM_CONSTR(oss, line, 300, std::ios::out);
  reset_ostrstream(oss); // go to the beginning
  Am_Object group = Am_Script_Line_Group.Create().Add(Am_COMMAND, cmd);
  Am_Value v, object_modified_value, new_value;
  v = cmd.Peek(Am_REGISTER_COMMAND);
  //if v.valid(), then there is a placeholder for the command itself
  // This only happens for match_commands
  if (!v.Valid()) {
    v = cmd.Peek(Am_SHORT_LABEL);
    if (!v.Valid())
      v = cmd.Peek(Am_LABEL);
  }
  if (cnt.Valid())
    oss << cnt << ". ";
  oss << v << " " << std::ends;
  OSTRSTREAM_COPY(oss, line, 300);
  group.Add_Part(Am_Script_Line_Command.Create()
                     .Set(Am_TEXT, line)
                     .Add(Am_VALUE, v)
                     .Add(Am_TOP_SCRIPT_LINE_GROUP, group)
                     .Add(Am_COMMAND, cmd)
                     .Add(Am_SLOTS_TO_SAVE, Am_REGISTER_COMMAND));

  Am_Value_List part_chain;
  OSTRSTREAM_CONSTR(oss2, line, 300, std::ios::out);
  reset_ostrstream(oss2); // go to the beginning
  object_modified_value = cmd.Peek(Am_OBJECT_MODIFIED);
  char placeholder_char;
  if (object_modified_value.Valid()) {
    placeholder_char = find_placeholder_char(cmd, Am_OBJECT_MODIFIED);
    if (placeholder_char)
      oss2 << placeholder_char << ": ";
    gen_script_list_or_value(line, oss2, Am_Script_Line_Modified, group, cmd,
                             object_modified_value, false, group, part_chain);
  }

  if (!cmd.Is_Instance_Of(Am_Object_Create_Command)) {
    new_value = cmd.Peek(Am_VALUE);
    if (new_value.Exists() &&
        (!object_modified_value.Valid() || new_value.type != Am_OBJECT ||
         new_value != object_modified_value)) {
      //don't output new_value if same as object_modified (as in create commands)
      OSTRSTREAM_CONSTR(oss3, line, 300, std::ios::out);
      reset_ostrstream(oss3); // go to the beginning
      placeholder_char = find_placeholder_char(cmd, Am_VALUE);
      if (placeholder_char)
        oss3 << placeholder_char << ":";
      oss3 << " = ";
      part_chain.Make_Empty();
      gen_script_list_or_value(line, oss3, Am_Script_Line_Value, group, cmd,
                               new_value, false, group, part_chain);
    } // end new_value exists
  }
  if (cmd.Is_Instance_Of(Am_Object_Create_Command) ||
      cmd.Is_Instance_Of(Am_Find_Command)) {
    gen_create_parameters_line(cmd, line, group);
  }
  return group;
}

void
insert_new_show_script_command(Am_Object &cmd_for_menu_bar, Am_Object &undo_db)
{
  Am_Object menu_bar = undo_db.Get(Am_UNDO_MENU_BAR);
  Am_Value_List bar_list = menu_bar.Get(Am_ITEMS);
  bar_list.Start();                           //at File
  bar_list.Next();                            //at Undo/Redo/Repeat
  bar_list.Next();                            //at Find
  bar_list.Next();                            //at Scripting
  Am_Object cmd = bar_list.Get();             //scripting's command
  Am_Value_List sub_list = cmd.Get(Am_ITEMS); //scripting's sub-menu
  sub_list.Add(cmd_for_menu_bar, Am_TAIL, false);
  cmd.Note_Changed(Am_ITEMS);
}

void
register_script_command(Am_Object &execute_script_command, Am_Object &window,
                        Am_Object &undo_db, Am_String &name)
{
  Am_Object cmd_for_menu_bar = execute_script_command.Get(
      Am_SHOW_SCRIPT_COMMAND, Am_RETURN_ZERO_ON_ERROR);
  if (cmd_for_menu_bar.Valid()) { //just update its slots
    cmd_for_menu_bar.Set(Am_LABEL, name);
  } else {
    Am_Add_Accelerator_Command_To_Window(execute_script_command, window);
    cmd_for_menu_bar =
        Am_Show_Script_Command.Create()
            .Set(Am_LABEL, name)
            .Set(Am_SCRIPT_EXECUTE_COMMAND, execute_script_command);
    execute_script_command.Set(Am_SHOW_SCRIPT_COMMAND, cmd_for_menu_bar);
    insert_new_show_script_command(cmd_for_menu_bar, undo_db);
  }
}

void
uninitialize_all_placeholders(Am_Object &execute_command)
{
  Am_Value_List ph_list = execute_command.Get(Am_PLACEHOLDER_LIST);
  Am_Assoc as;
  Am_Object ph;
  for (ph_list.Start(); !ph_list.Last(); ph_list.Next()) {
    as = ph_list.Get();
    ph = as.Value_2();
    ph.Set(Am_PLACEHOLDER_INITIALIZED, false, Am_OK_IF_NOT_THERE);
  }
}

//actually run the commands in the script
void
do_execute_script(Am_Object &execute_cmd, bool just_one, Am_Object &which_cmd)
{
  if (am_sdebug)
    std::cout << "\n%%%%Executing command " << execute_cmd << std::endl
              << std::flush;
  Am_Value_List cmd_list = execute_cmd.Get(Am_COMMANDS_PROTOTYPES);
  Am_Object main_window = execute_cmd.Get(Am_WINDOW);
  Am_Object undo_handler = main_window.Get(Am_UNDO_HANDLER);
  Am_Object sel_widget = execute_cmd.Get(Am_SELECTION_WIDGET);

  Am_Object cmd, new_cmd;
  Am_Value_List modified_objects, new_cmd_list;
  int execute_count =
      execute_cmd.Get(Am_EXECUTE_COUNTER, Am_RETURN_ZERO_ON_ERROR);
  if (execute_count < 1)
    execute_count = 1;
  execute_cmd.Set(Am_VALUE, execute_count);
  if (undo_handler.Valid()) {
    Am_Selective_Allowed_Method same_allowed_method =
        undo_handler.Get(Am_SELECTIVE_REPEAT_SAME_ALLOWED);
    Am_Handler_Selective_Undo_Method same_method =
        undo_handler.Get(Am_SELECTIVE_REPEAT_SAME_METHOD);
    Am_Selective_New_Allowed_Method new_allowed_method =
        undo_handler.Get(Am_SELECTIVE_REPEAT_NEW_ALLOWED);
    Am_Handler_Selective_Repeat_New_Method new_method =
        undo_handler.Get(Am_SELECTIVE_REPEAT_ON_NEW_METHOD);
    Am_Register_Command_Method reg_method =
        undo_handler.Get(Am_REGISTER_COMMAND);

    int cnt = 1;

    if (just_one) {
      if (am_sdebug)
        std::cout << "\nExecuting script cmd " << execute_cmd << std::endl
                  << std::flush;
      bool first_time = true;
      cmd_list.Start();
      if (which_cmd.Valid()) {
        if (which_cmd != cmd_list.Get())
          first_time = false;
      } else
        which_cmd = cmd_list.Get();
      if (first_time)
        uninitialize_all_placeholders(execute_cmd);
      else {
        modified_objects = execute_cmd.Get(Am_OBJECT_MODIFIED);
        new_cmd_list = execute_cmd.Get(Am_COMMANDS_IN_SCRIPT);
      }

      cmd = which_cmd;
      if (am_sdebug)
        std::cout << "\n----Single step, executing " << cmd << std::endl
                  << std::flush;
      if (!Am_Handle_Placeholders_And_Call_Command(
              cnt, cmd, same_allowed_method, same_method, new_allowed_method,
              new_method, sel_widget, undo_handler, new_cmd)) {
        //script failed
        if (am_sdebug)
          std::cout << "handle returned false, exiting\n" << std::flush;
        execute_cmd.Set(Am_COMMAND_IS_ABORTING, true);
        return;
      }
      new_cmd_list.Add(new_cmd);
      modified_objects.Append_New(new_cmd.Peek(Am_OBJECT_MODIFIED));
      execute_cmd.Set(Am_OBJECT_MODIFIED, modified_objects);
      if (am_sdebug)
        std::cout << "Single Step; setting Am_COMMANDS_IN_SCRIPT of "
                  << execute_cmd << " to be " << new_cmd_list << std::endl
                  << std::flush;
      execute_cmd.Set(Am_COMMANDS_IN_SCRIPT, new_cmd_list);
    } else { //do all of the commands
      for (int ec = 1; ec <= execute_count; ec++) {
        if (am_sdebug)
          std::cout << "\nExecuting script cmd " << execute_cmd << " count "
                    << ec << std::endl
                    << std::flush;
        uninitialize_all_placeholders(execute_cmd);
        for (cmd_list.Start(); !cmd_list.Last(); cmd_list.Next(), cnt++) {
          cmd = cmd_list.Get();
          if (!Am_Handle_Placeholders_And_Call_Command(
                  cnt, cmd, same_allowed_method, same_method,
                  new_allowed_method, new_method, sel_widget, undo_handler,
                  new_cmd)) {
            //script failed
            if (am_sdebug)
              std::cout << "handle returned false, exiting\n" << std::flush;
            execute_cmd.Set(Am_COMMAND_IS_ABORTING, true);
            return;
          }
          new_cmd_list.Add(new_cmd);
          modified_objects.Append_New(new_cmd.Peek(Am_OBJECT_MODIFIED));
        } //end loop on cmd
        execute_cmd.Set(Am_OBJECT_MODIFIED, modified_objects);
        if (am_sdebug)
          std::cout << "Setting Am_COMMANDS_IN_SCRIPT of " << execute_cmd
                    << " to be " << new_cmd_list << std::endl
                    << std::flush;
        execute_cmd.Set(Am_COMMANDS_IN_SCRIPT, new_cmd_list);
        if (ec != execute_count) { //not the last time
          if (am_sdebug)
            std::cout << "explicit registering " << execute_cmd << std::endl
                      << std::flush;
          reg_method.Call(undo_handler, execute_cmd);
          Am_Update_All(); //see the results
          modified_objects.Make_Empty();
          new_cmd_list.Make_Empty();
        }
      }
    }
  }
  execute_cmd.Set(Am_EXECUTE_COUNTER, 1, Am_OK_IF_NOT_THERE);
}

//fill / load the script window with the generated commands
void
am_set_commands_into_script_window(Am_Object &script_window,
                                   Am_Object &execute_command)
{
  Am_Value_List cmd_list = execute_command.Get(Am_COMMANDS_PROTOTYPES);
  if (am_sdebug)
    std::cout << "Adding commands " << cmd_list << " to " << script_window
              << std::endl
              << std::flush;
  Am_Object cmd;
  Am_Value_List menu_commands;

  for (cmd_list.Start(); !cmd_list.Last(); cmd_list.Next()) {
    cmd = cmd_list.Get();
    menu_commands.Add(am_gen_one_script_line(cmd));
  }
  if (am_sdebug)
    std::cout << "setting " << script_window.Get_Object(Am_UNDO_SCROLL_GROUP)
              << " value to " << menu_commands << std::endl
              << std::flush;
  script_window.Get_Object(Am_UNDO_SCROLL_GROUP)
      .Set(Am_ITEMS, menu_commands)
      //so nothing selected
      .Set(Am_VALUE, (0L))
      //so nothing yellow or green selected
      .Set(Am_SAVED_OLD_OBJECT_OWNER, (0L), Am_OK_IF_NOT_THERE);
  Am_Object counter =
      script_window.Get(Am_EXECUTE_COUNTER, Am_RETURN_ZERO_ON_ERROR);
  if (counter.Valid())
    counter.Set(Am_VALUE, 1);
}

//adds the created objects to created_objects_assoc
void
am_note_created_objects(Am_Object &new_cmd, Am_Value &created_objects,
                        Am_Value_List &created_objects_assoc)
{
  created_objects_assoc.Add(Am_Assoc(created_objects, new_cmd));
  if (am_sdebug)
    std::cout << "Noted create of " << created_objects << " added to assoc "
              << created_objects_assoc << std::endl
              << std::flush;
}

Am_Object
create_script_for(Am_Value_List &commands_for_script, Am_Input_Char accel_char,
                  Am_String &name, bool on_new, Am_Object &window,
                  Am_Object &selection_widget)
{
  Am_Object cmd, new_cmd;
  Am_Object execute_script_command =
      Am_Execute_Script_Command.Create()
          .Set(Am_LABEL, name)
          .Set(Am_ACCELERATOR, accel_char)
          .Add(Am_WINDOW, window)
          .Add(Am_EXECUTE_SCRIPT_ON_NEW, on_new)
          .Add(Am_SELECTION_WIDGET, selection_widget);
  Am_Value_List original_command_list, new_cmd_list, placeholder_list,
      created_objects_assoc;
  Am_Value created_objects;
  Am_Object placeholder;
  int cnt = 0;
  int command_index = 1;
  //commands come in backwards, swap them
  for (commands_for_script.End(); !commands_for_script.First();
       commands_for_script.Prev(), command_index++) {
    cmd = commands_for_script.Get();
    if (am_sdebug)
      std::cout << "\nprocessing command " << command_index << ": " << cmd
                << std::endl
                << std::flush;
    original_command_list.Add(cmd);
    new_cmd = cmd.Copy_Value_Only();
    new_cmd.Set(Am_COMMAND_INDEX_IN_SCRIPT, command_index, Am_OK_IF_NOT_THERE);
    //if not creating from scratch, then check the parameters
    //group_command is both a from_create AND it needs its parameters munged
    if (new_cmd.Is_Instance_Of(Am_Object_Create_Command))
      am_check_start_obj_parameters(new_cmd, placeholder_list,
                                    created_objects_assoc, cnt);
    else
      am_check_obj_modified(new_cmd, placeholder_list, created_objects_assoc,
                            cnt);
    //be sure to note created objects AFTER dealing with the parameters of
    //this command, so this command won't refer to itself
    if (new_cmd.Is_Instance_Of(Am_Defines_A_Collection_Command)) {
      created_objects = new_cmd.Get(Am_VALUE);
      am_note_created_objects(new_cmd, created_objects, created_objects_assoc);
    }

    new_cmd.Set(Am_IMPLEMENTATION_PARENT, Am_IN_A_SCRIPT);
    new_cmd_list.Add(new_cmd);
  }
  //when script is executed, the Am_COMMANDS_IN_SCRIPT gets the new cmds,
  //which can be undone, but the Am_COMMANDS_PROTOTYPES stay the same
  execute_script_command.Set(Am_COMMANDS_PROTOTYPES, new_cmd_list);
  execute_script_command.Set(Am_COMMANDS_IN_SCRIPT, new_cmd_list);
  execute_script_command.Set(Am_ORIGINAL_COMMAND_LIST, original_command_list);
  execute_script_command.Set(Am_PLACEHOLDER_LIST, placeholder_list);
  execute_script_command.Set(Am_PLACEHOLDER_COUNT, cnt);

  if (am_sdebug)
    std::cout << "%% Execute command " << execute_script_command << " orig "
              << original_command_list << " new " << new_cmd_list << std::endl
              << std::flush;
  return execute_script_command;
}

Am_Object
create_script_execute_command(Am_Object &script_window,
                              Am_Value_List &commands_to_execute,
                              Am_Object &window, Am_Object &sel_widget,
                              Am_String name)
{
  // Am_Object on_new_buttons = script_window.Get_Object(Am_UNDO_OPTIONS);
  // bool on_new = (int)on_new_buttons.Get(Am_VALUE) == Am_ID_ON_SELECTION;
  bool on_new = false;
  Am_Input_Char noop; //Am_No_Input_Char not defined

  Am_Object script_execute_command =
      script_window.Get(Am_SCRIPT_EXECUTE_COMMAND);
  if (script_execute_command.Valid()) { //then edit it
    script_execute_command.Set(Am_LABEL, name)
        .Set(Am_ACCELERATOR, noop)
        .Set(Am_EXECUTE_SCRIPT_ON_NEW, on_new);
  } else
    script_execute_command = create_script_for(commands_to_execute, noop, name,
                                               on_new, window, sel_widget);
  return script_execute_command;
}

//hit OK on the window
void
finalize_script_execute_command(Am_Object &script_window, Am_Object &undo_db,
                                Am_Object &window, bool register_show)
{
  Am_Object script_execute_command =
      script_window.Get(Am_SCRIPT_EXECUTE_COMMAND);
  Am_String name = script_window.Get_Object(Am_NAME_OBJECT).Get(Am_VALUE);
  script_execute_command
      .Set(Am_LABEL, name)
      // accelerator of execute command set directly by invoke.cc
      // .Set(Am_ACCELERATOR, accel_char)
      .Set(Am_EXECUTE_SCRIPT_ON_NEW, false);
  if (register_show)
    register_script_command(script_execute_command, window, undo_db, name);

  Am_Object match_command = script_execute_command.Get(
      Am_BEFORE_OR_AFTER_COMMAND, Am_RETURN_ZERO_ON_ERROR);
  if (match_command.Valid())
    am_install_match_command(match_command, script_execute_command);
}

void
internal_start_create_script_window(Am_Value_List &commands_for_script,
                                    Am_Object &cmd)
{
  if (!commands_for_script.Empty()) {
    Am_Object undo_db = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get(Am_WINDOW);
    Am_Object script_window = get_script_window(undo_db);

    Am_Object sel_widget = undo_db.Get(Am_SELECTION_WIDGET);
    Am_Object window = sel_widget.Get(Am_WINDOW);
    Am_String name = gen_script_name();
    script_window.Get_Object(Am_NAME_OBJECT).Set(Am_VALUE, name);
    Am_Object script_execute_command = create_script_execute_command(
        script_window, commands_for_script, window, sel_widget, name);
    script_window.Set(Am_SCRIPT_EXECUTE_COMMAND, script_execute_command);
    script_window.Set(Am_SHOW_SCRIPT_COMMAND, (0L));

    am_set_commands_into_script_window(script_window, script_execute_command);
    script_window.Set(Am_VISIBLE, true);
  }
}

Am_Define_Method(Am_Object_Method, void, start_create_script_window,
                 (Am_Object cmd))
{
  Am_Value_List commands_for_script = get_commands_to_undo(cmd);
  internal_start_create_script_window(commands_for_script, cmd);
}
Am_Define_Method(Am_Object_Method, void, double_create_script_window,
                 (Am_Object cmd))
{
  //cmd is in inter, owner = scroll menu
  Am_Value_List commands_for_script =
      cmd.Get_Object(Am_SAVED_OLD_OWNER)
          .Get_Owner()
          .Get(Am_VALUE, Am_RETURN_ZERO_ON_ERROR);
  internal_start_create_script_window(commands_for_script, cmd);
}

//show the script window for the script
Am_Define_Method(Am_Object_Method, void, show_script_do, (Am_Object show_cmd))
{
  Am_Object execute_command = show_cmd.Get(Am_SCRIPT_EXECUTE_COMMAND);
  Am_Object undo_db = show_cmd.Get_Object(Am_SAVED_OLD_OWNER).Get(Am_WINDOW);
  Am_String name = execute_command.Get(Am_LABEL);

  Am_Object script_window = get_script_window(undo_db);
  script_window.Get_Object(Am_NAME_OBJECT).Set(Am_VALUE, name);
  script_window.Set(Am_SCRIPT_EXECUTE_COMMAND, execute_command);
  script_window.Set(Am_SHOW_SCRIPT_COMMAND, show_cmd);
  am_set_commands_into_script_window(script_window, execute_command);
  script_window.Set(Am_VISIBLE, true);
}

void
remove_show_script_command(Am_Object &execute_script_command,
                           Am_Object &cmd_for_menu_bar, Am_Object &undo_db)
{
  Am_Object menu_bar = undo_db.Get(Am_UNDO_MENU_BAR);
  Am_Value_List bar_list = menu_bar.Get(Am_ITEMS);
  bar_list.Start();                           //at File
  bar_list.Next();                            //at Undo/Redo/Repeat
  bar_list.Next();                            //at Find
  bar_list.Next();                            //at Scripting
  Am_Object cmd = bar_list.Get();             //scripting's command
  Am_Value_List sub_list = cmd.Get(Am_ITEMS); //scripting's sub-menu
  sub_list.Start();
  if (!sub_list.Member(cmd_for_menu_bar))
    Am_ERROR("Lost cmd_for_menu_bar " << cmd_for_menu_bar << " for execute "
                                      << execute_script_command << " from list "
                                      << sub_list << " on menubar cmd " << cmd);
  sub_list.Delete(false);
  cmd.Note_Changed(Am_ITEMS);
}

// Do method in the execute command to execute the commands associated with
// the script
Am_Define_Method(Am_Object_Method, void, execute_script_do,
                 (Am_Object execute_cmd))
{
  do_execute_script(execute_cmd, false, Am_No_Object);
}

Am_Define_Method(Am_Object_Method, void, execute_script_undo,
                 (Am_Object execute_command))
{
  Am_Value_List cmd_list = execute_command.Get(Am_COMMANDS_IN_SCRIPT);
  Am_Object cmd;
  //go backwards when undoing.
  for (cmd_list.End(); !cmd_list.First(); cmd_list.Prev()) {
    cmd = cmd_list.Get();
    Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SHORT, "Undoing command " << cmd);
    am_perform_on_cmd_and_impls(cmd, Am_UNDO_METHOD);
  }
}
Am_Define_Method(Am_Object_Method, void, execute_script_redo,
                 (Am_Object execute_command))
{
  Am_Value_List cmd_list = execute_command.Get(Am_COMMANDS_IN_SCRIPT);
  Am_Object cmd;
  //go forwards when redoing
  for (cmd_list.Start(); !cmd_list.Last(); cmd_list.Next()) {
    cmd = cmd_list.Get();
    Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SHORT, "Undoing command " << cmd);
    am_perform_on_cmd_and_impls(cmd, Am_REDO_METHOD);
  }
}

Am_Define_Method(Am_Object_Method, void, execute_script_repeat,
                 (Am_Object execute_command))
{
  do_execute_script(execute_command, false, Am_No_Object);
}

Am_Define_Method(Am_Selective_Repeat_New_Method, void,
                 execute_script_repeat_new,
                 (Am_Object execute_command, Am_Value /* new_selection */,
                  Am_Value new_value))
{
  if (new_value.Valid() && new_value.type == Am_INT)
    execute_command.Set(Am_EXECUTE_COUNTER, new_value);
  do_execute_script(execute_command, false, Am_No_Object);
}

//method for "OK" button in script window: create and register the script
//command
Am_Define_Method(Am_Object_Method, void, am_script_ok, (Am_Object cmd))
{
  Am_Object script_window = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  Am_Object undo_db = script_window.Get(Am_UNDO_MENU_OF_COMMANDS);
  Am_Object sel_widget = undo_db.Get(Am_SELECTION_WIDGET);
  Am_Object window = sel_widget.Get(Am_WINDOW);
  finalize_script_execute_command(script_window, undo_db, window, true);
  done_with_script_window(script_window);
}

//Command in script dialog box to execute the script
Am_Define_Method(Am_Object_Method, void, am_script_execute, (Am_Object cmd))
{
  Am_Object script_window = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  Am_Object script_execute_command =
      script_window.Get(Am_SCRIPT_EXECUTE_COMMAND);
  Am_Object undo_db = script_window.Get(Am_UNDO_MENU_OF_COMMANDS);
  Am_Object sel_widget = undo_db.Get(Am_SELECTION_WIDGET);
  Am_INTER_TRACE_PRINT(cmd, "Executing script " << script_execute_command);
  Am_Execute_Command(script_execute_command, sel_widget);
  // leave script window visible
  script_window.Get_Object(Am_EXECUTE_COUNTER).Set(Am_VALUE, 1);
}

Am_Define_Method(Am_Object_Method, void, am_execute_script_step_one,
                 (Am_Object cmd))
{
  Am_Object script_window = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  Am_Object execute_cmd = script_window.Get(Am_SCRIPT_EXECUTE_COMMAND);
  Am_Value_List sel_cmds =
      script_window.Get_Object(Am_UNDO_SCROLL_GROUP).Get(Am_VALUE);
  Am_Object line, script_cmd;
  if (sel_cmds.Valid()) {
    sel_cmds.Start();
    line = sel_cmds.Get();
    script_cmd = line.Get(Am_COMMAND);
  }
  do_execute_script(execute_cmd, true, script_cmd);
  Am_Value_List all_cmds =
      script_window.Get_Object(Am_UNDO_SCROLL_GROUP).Get(Am_ITEMS);
  all_cmds.Start();
  if (line.Valid()) {
    if (!all_cmds.Member(line))
      Am_ERROR("Line " << line << " lost from " << script_window);
  }
  //pointer is at line
  all_cmds.Next();
  Am_Value_List new_value;
  if (!all_cmds.Last()) { // otherwise, went past end, leave new_value null
    new_value.Add(all_cmds.Get());
  }
  if (am_sdebug)
    std::cout << "--One_step; Setting value of "
              << script_window.Get_Object(Am_UNDO_SCROLL_GROUP) << " to be "
              << new_value << std::endl
              << std::flush;
  script_window.Get_Object(Am_UNDO_SCROLL_GROUP).Set(Am_VALUE, new_value);
}

//Command in script dialog box to remove the script
Am_Define_Method(Am_Object_Method, void, am_script_remove, (Am_Object cmd))
{
  Am_Object script_window = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  Am_Object execute_script_command =
      script_window.Get(Am_SCRIPT_EXECUTE_COMMAND);
  Am_Object cmd_for_menu_bar = execute_script_command.Get(
      Am_SHOW_SCRIPT_COMMAND, Am_RETURN_ZERO_ON_ERROR);
  if (cmd_for_menu_bar.Valid()) { //then was registered
    // SPACE LEAK, should probably destroy Am_COMMANDS_IN_SCRIPT and/or
    // Am_COMMANDS_PROTOTYPES

    Am_Object undo_db = script_window.Get(Am_UNDO_MENU_OF_COMMANDS);
    Am_Object sel_widget = undo_db.Get(Am_SELECTION_WIDGET);
    Am_Object window = sel_widget.Get(Am_WINDOW);
    remove_show_script_command(execute_script_command, cmd_for_menu_bar,
                               undo_db);
    Am_Remove_Accelerator_Command_From_Window(execute_script_command, window);
  }
  Am_Object match_command = execute_script_command.Get(
      Am_BEFORE_OR_AFTER_COMMAND, Am_RETURN_ZERO_ON_ERROR);
  if (match_command.Valid())
    am_remove_match_command(match_command);

  done_with_script_window(script_window);
}

Am_Define_Method(Am_Object_Method, void, am_script_select_members,
                 (Am_Object cmd))
{
  Am_Object script_window = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  Am_Object script_execute_command =
      script_window.Get(Am_SCRIPT_EXECUTE_COMMAND);
  Am_Value_List original_command_list;
  if (script_execute_command.Valid())
    original_command_list =
        script_execute_command.Get(Am_ORIGINAL_COMMAND_LIST);
  else
    Am_Error("where did execute command go");
  Am_Object undo_db = script_window.Get(Am_UNDO_MENU_OF_COMMANDS);
  undo_db.Get_Object(Am_UNDO_SCROLL_GROUP).Set(Am_VALUE, original_command_list);
}

Am_Define_Method(Am_Object_Method, void, am_do_generalize_script,
                 (Am_Object cmd))
{
  Am_Object script_window = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  Am_Object scroll_menu = script_window.Get(Am_UNDO_SCROLL_GROUP);
  Am_Value_List sel_list =
      scroll_menu.Get(Am_SAVED_OLD_OBJECT_OWNER, Am_RETURN_ZERO_ON_ERROR);
  if (sel_list.Valid())
    Am_Pop_Up_Generalize_DB(script_window, sel_list);
}

Am_Define_Formula(bool, am_insert_commands_ok)
{
  Am_Object script_window = self.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  Am_Object undo_db = script_window.Get(Am_UNDO_MENU_OF_COMMANDS);
  Am_Value_List new_cmds =
      undo_db.Get_Object(Am_UNDO_SCROLL_GROUP).Get(Am_VALUE);
  if (!new_cmds.Valid())
    return false;
  Am_Object scroll_menu = script_window.Get(Am_UNDO_SCROLL_GROUP);
  Am_Value_List sel_list = scroll_menu.Get(Am_VALUE);
  if (sel_list.Valid() && sel_list.Length() > 1)
    return false;
  return true;
}

Am_Define_Formula(bool, am_delete_commands_ok)
{
  Am_Object script_window = self.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  Am_Object scroll_menu = script_window.Get(Am_UNDO_SCROLL_GROUP);
  Am_Value_List sel_list = scroll_menu.Get(Am_VALUE);
  return sel_list.Valid();
}

void
renumber_script_commands(Am_Value_List &orig_cmds)
{
  int i = 1;
  Am_Object this_cmd;
  for (orig_cmds.Start(); !orig_cmds.Last(); orig_cmds.Next(), i++) {
    this_cmd = orig_cmds.Get();
    this_cmd.Set(Am_COMMAND_INDEX_IN_SCRIPT, i, Am_OK_IF_NOT_THERE);
  }
}

Am_Define_Method(Am_Object_Method, void, am_delete_script_commands,
                 (Am_Object cmd))
{
  // *** SHOULD FIX UP REFERENCES TO THE DELETED COMMANDS
  // *** PLACEHOLDERS SET BY THESE COMMANDS
  Am_Object script_window = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  Am_Object scroll_menu = script_window.Get(Am_UNDO_SCROLL_GROUP);
  Am_Value_List sel_list = scroll_menu.Get(Am_VALUE);
  Am_Object execute_command = script_window.Get(Am_SCRIPT_EXECUTE_COMMAND);
  Am_Value_List orig_cmds = execute_command.Get(Am_COMMANDS_PROTOTYPES);
  Am_Value_List original_command_list =
      execute_command.Get(Am_ORIGINAL_COMMAND_LIST);

  cmd.Set(Am_OLD_VALUE, orig_cmds);
  cmd.Set(Am_ORIGINAL_COMMAND_LIST, original_command_list);

  Am_Object this_cmd, found_sel;
  Am_Value_List new_cmd_list, new_original_command_list;
  int i = 1;
  for (orig_cmds.Start(); !orig_cmds.Last(); orig_cmds.Next()) {
    this_cmd = orig_cmds.Get();
    if (!am_find_cmd_in_sel(sel_list, this_cmd, found_sel)) {
      new_cmd_list.Add(this_cmd);
      new_original_command_list.Add(this_cmd.Get(Am_SOURCE_OF_COPY));
      this_cmd.Set(Am_COMMAND_INDEX_IN_SCRIPT, i++, Am_OK_IF_NOT_THERE);
    }
  }
  execute_command.Set(Am_COMMANDS_PROTOTYPES, new_cmd_list);
  execute_command.Set(Am_COMMANDS_IN_SCRIPT, new_cmd_list);
  execute_command.Set(Am_ORIGINAL_COMMAND_LIST, new_original_command_list);

  am_set_commands_into_script_window(script_window, execute_command);
}

void
internal_insert_commands_before_after(Am_Object &cmd, bool before)
{
  Am_Object script_window = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  Am_Object scroll_menu = script_window.Get(Am_UNDO_SCROLL_GROUP);
  Am_Value_List sel_list = scroll_menu.Get(Am_VALUE);
  Am_Object undo_db = script_window.Get(Am_UNDO_MENU_OF_COMMANDS);
  Am_Value_List new_cmds =
      undo_db.Get_Object(Am_UNDO_SCROLL_GROUP).Get(Am_VALUE);
  Am_Object execute_command = script_window.Get(Am_SCRIPT_EXECUTE_COMMAND);
  Am_Value_List orig_cmds = execute_command.Get(Am_COMMANDS_PROTOTYPES);
  Am_Value_List original_command_list =
      execute_command.Get(Am_ORIGINAL_COMMAND_LIST);

  cmd.Set(Am_OLD_VALUE, orig_cmds.Copy());
  cmd.Set(Am_ORIGINAL_COMMAND_LIST, original_command_list.Copy(),
          Am_OK_IF_NOT_THERE);

  if (before) {
    if (sel_list.Valid()) {
      Am_Object first_line = sel_list.Get_First();
      Am_Object insert_before = first_line.Get(Am_COMMAND);
      if (!orig_cmds.Start_Member(insert_before))
        Am_ERROR("Lost insert_before cmd " << insert_before << " from list "
                                           << orig_cmds << " of "
                                           << execute_command);
    } else
      orig_cmds.Start();
  } else {
    if (sel_list.Valid()) {
      Am_Object last_line = sel_list.Get_Nth(sel_list.Length() - 1);
      Am_Object insert_after = last_line.Get(Am_COMMAND);
      if (!orig_cmds.Start_Member(insert_after))
        Am_ERROR("Lost insert_after cmd " << insert_after << " from list "
                                          << orig_cmds << " of "
                                          << execute_command);
      orig_cmds.Next();
    } else
      orig_cmds.End();
  }

  //here orig_cmds is set so can insert at the current element

  //commands come in backwards, swap them
  Am_Object this_cmd, new_cmd;
  for (new_cmds.End(); !new_cmds.First(); new_cmds.Prev()) {
    this_cmd = new_cmds.Get();
    if (am_sdebug)
      std::cout << "\n adding command " << this_cmd << std::endl << std::flush;
    original_command_list.Add(this_cmd);
    new_cmd = this_cmd.Copy_Value_Only();
    new_cmd.Set(Am_IMPLEMENTATION_PARENT, Am_IN_A_SCRIPT);
    orig_cmds.Insert(new_cmd, Am_BEFORE);
  }

  //now fix up the numbering
  renumber_script_commands(orig_cmds);

  execute_command.Set(Am_COMMANDS_PROTOTYPES, orig_cmds);
  execute_command.Set(Am_COMMANDS_IN_SCRIPT, orig_cmds);
  execute_command.Set(Am_ORIGINAL_COMMAND_LIST, original_command_list);

  am_set_commands_into_script_window(script_window, execute_command);
}
Am_Define_Method(Am_Object_Method, void, am_insert_commands_before,
                 (Am_Object cmd))
{
  internal_insert_commands_before_after(cmd, true);
}
Am_Define_Method(Am_Object_Method, void, am_insert_commands_after,
                 (Am_Object cmd))
{
  internal_insert_commands_before_after(cmd, false);
}

Am_Define_Method(Am_Object_Method, void, am_edit_commands_list_undo,
                 (Am_Object cmd))
{
  Am_Object script_window = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  Am_Object execute_command = script_window.Get(Am_SCRIPT_EXECUTE_COMMAND);
  Am_Value_List cur_orig_cmds = execute_command.Get(Am_COMMANDS_PROTOTYPES);
  Am_Value_List cur_original_command_list =
      execute_command.Get(Am_ORIGINAL_COMMAND_LIST);

  Am_Value_List old_orig_cmds = cmd.Get(Am_OLD_VALUE);
  Am_Value_List old_original_command_list = cmd.Get(Am_ORIGINAL_COMMAND_LIST);

  //in case undo the undo
  cmd.Set(Am_OLD_VALUE, cur_orig_cmds.Copy());
  cmd.Set(Am_ORIGINAL_COMMAND_LIST, cur_original_command_list.Copy());

  renumber_script_commands(old_orig_cmds);
  execute_command.Set(Am_COMMANDS_PROTOTYPES, old_orig_cmds);
  execute_command.Set(Am_COMMANDS_IN_SCRIPT, old_orig_cmds);
  execute_command.Set(Am_ORIGINAL_COMMAND_LIST, old_original_command_list);
  am_set_commands_into_script_window(script_window, execute_command);
}

Am_Define_Method(Am_Object_Method, void, am_generalize_script_double_click,
                 (Am_Object cmd))
{
  //cmd in inter on scroll_menu on script_window
  Am_Object scroll_menu = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  Am_Object script_window = scroll_menu.Get_Owner();
  Am_Value_List sel_list =
      scroll_menu.Get(Am_SAVED_OLD_OBJECT_OWNER, Am_RETURN_ZERO_ON_ERROR);
  if (sel_list.Valid())
    Am_Pop_Up_Generalize_DB(script_window, sel_list);
}

Am_Define_Formula(bool, am_generalize_script_active)
{
  Am_Object script_window = self.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  Am_Object scroll_menu = script_window.Get(Am_UNDO_SCROLL_GROUP);
  Am_Value_List sel_list =
      scroll_menu.Get(Am_SAVED_OLD_OBJECT_OWNER, Am_RETURN_ZERO_ON_ERROR);
  return sel_list.Valid();
}

Am_Define_Method(Am_Where_Method, Am_Object, am_on_selectable_line_part,
                 (Am_Object inter, Am_Object /* object */,
                  Am_Object event_window, int x, int y))
{
  Am_Object scroll_menu = inter.Get_Owner();
  Am_Object ret =
      Am_Point_In_Leaf(scroll_menu, x, y, event_window, false, false);
  if (ret.Valid() && ret.Is_Instance_Of(Am_Script_Line_Selectable))
    return ret;
  else
    return Am_No_Object;
}

void
set_text_leaf_parts(Am_Object &obj, Am_Style &new_style)
{
  if (obj.Is_Instance_Of(Am_Text)) {
    obj.Set(Am_FILL_STYLE, new_style);
  } else if (obj.Is_Instance_Of(Am_Group)) {
    Am_Value_List parts = obj.Get(Am_GRAPHICAL_PARTS);
    Am_Object part;
    for (parts.Start(); !parts.Last(); parts.Next()) {
      part = parts.Get();
      set_text_leaf_parts(part, new_style);
    }
  } else
    Am_ERROR("Bad part " << obj);
}

void
find_and_set_sub_lines(Am_Value &target_value, Am_Object &group,
                       Am_Object &not_part, Am_Value_List &ret_list)
{
  Am_Value_List parts = group.Get(Am_GRAPHICAL_PARTS);
  Am_Object part;
  for (parts.Start(); !parts.Last(); parts.Next()) {
    part = parts.Get();
    if (part != not_part) {
      if (Am_Value_List::List_Or_Value_Equal(part.Get(Am_VALUE),
                                             target_value)) {
        ret_list.Add(part);
        set_text_leaf_parts(part, Am_Yellow);
      } else if (part.Is_Instance_Of(Am_Group))
        find_and_set_sub_lines(target_value, part, not_part, ret_list);
    }
  }
}

Am_Value_List
find_and_set_other_lines_with_same(Am_Object &script_line_part,
                                   Am_Object &scroll_menu)
{
  Am_Value target_value;
  Am_Object not_part, script_line_group;
  if (script_line_part.Is_Instance_Of(Am_Script_List_Header)) {
    not_part = script_line_part.Get_Owner(); //the group
    target_value = not_part.Get(Am_VALUE);
  } else {
    target_value = script_line_part.Get(Am_VALUE);
    not_part = script_line_part;
  }
  Am_Value_List ret_list;
  ret_list.Add(not_part); //the main one should be first
  set_text_leaf_parts(not_part, Am_Motif_Light_Green);
  Am_Value_List items = scroll_menu.Get(Am_ITEMS);
  for (items.Start(); !items.Last(); items.Next()) {
    script_line_group = items.Get();
    find_and_set_sub_lines(target_value, script_line_group, not_part, ret_list);
  }
  if (am_sdebug)
    std::cout << "final sel_list is " << ret_list << std::endl << std::flush;
  return ret_list;
}

void
clear_all_line_same(Am_Object &scroll_menu)
{
  Am_Value_List sel_list =
      scroll_menu.Get(Am_SAVED_OLD_OBJECT_OWNER, Am_RETURN_ZERO_ON_ERROR);
  if (sel_list.Valid()) {
    Am_Object sel;
    for (sel_list.Start(); !sel_list.Last(); sel_list.Next()) {
      sel = sel_list.Get();
      set_text_leaf_parts(sel, Am_No_Style);
    }
  }
}

Am_Define_Method(Am_Object_Method, void, am_select_line_part, (Am_Object cmd))
{
  Am_Object inter = cmd.Get(Am_SAVED_OLD_OWNER);
  Am_Object script_line_part = cmd.Get(Am_OBJECT_MODIFIED);
  Am_Object scroll_menu = inter.Get_Owner();
  clear_all_line_same(scroll_menu);
  scroll_menu.Set(Am_VALUE, false);
  Am_Value_List sel_list =
      find_and_set_other_lines_with_same(script_line_part, scroll_menu);
  scroll_menu.Set(Am_SAVED_OLD_OBJECT_OWNER, sel_list, Am_OK_IF_NOT_THERE);
}

Am_Define_Style_Formula(script_line_text_color)
{
  Am_Object menu_item = self.Get_Object(Am_TOP_SCRIPT_LINE_GROUP).Get_Owner();
  if (menu_item.Get(Am_SELECTED).Valid())
    return Am_White;
  else
    return Am_Black;
}

Am_Define_Method(Am_Object_Method, void, am_clear_select_line_part,
                 (Am_Object cmd))
{
  Am_Object scroll_menu = cmd.Get(Am_SAVED_OLD_OWNER);
  clear_all_line_same(scroll_menu);
  scroll_menu.Set(Am_SAVED_OLD_OBJECT_OWNER, Am_No_Object, Am_OK_IF_NOT_THERE);
}

/* **********************
//Demon procedure for when add or remove parts to keep the
//Am_COMMANDS_IN_SCRIPT list up to date.  Unlike the components list, we want
//this list in forward order (last added is last).
//old_object is being removed and new_object is being added
void am_script_command_add_part (Am_Object owner, Am_Object old_object,
			  Am_Object new_object)
{
  if (am_sdebug)
   std::cout << "add part demon on " << owner << "old " << old_object
	 <<  " new " << new_object <<std::endl <<std::flush;

  Am_Value_List components;
  if (old_object.Valid ()) {
    owner.Make_Unique (Am_COMMANDS_IN_SCRIPT);
    components = owner.Get (Am_COMMANDS_IN_SCRIPT);
    components.Start ();
    if (components.Member (old_object)) {
      components.Delete (false);
      owner.Note_Changed (Am_COMMANDS_IN_SCRIPT);
    }
  }
  if (new_object.Valid ()) {
    owner.Make_Unique (Am_COMMANDS_IN_SCRIPT);
    components = owner.Get (Am_COMMANDS_IN_SCRIPT);
    bool was_there = components.Exists ();
    components.Add (new_object, Am_HEAD, false);
    if (was_there)
      owner.Note_Changed (Am_COMMANDS_IN_SCRIPT);
    else
      owner.Set (Am_COMMANDS_IN_SCRIPT, components);
  }
}

*/

///////////////////////////////////////////////////////////////////////////

Am_Define_Method(Am_Object_Method, void, am_show_undo_box, (Am_Object cmd))
{
  Am_Object my_undo_dialog = cmd.Get(Am_UNDO_DIALOG_BOX_SLOT);
  my_undo_dialog.Set(Am_VISIBLE, true);
  Am_To_Top(my_undo_dialog);
}

Am_Define_Object_Formula(get_double_click_inter)
{
  //self is inter, owner is sub_menu ,owner is widget, owner is undo_db,
  // get Am_UNDO_SCROLL_GROUP of undo_db, get Am_ARROW_INTERACTOR of that
  Am_Object inter = self.Get_Owner()
                        .Get_Owner()
                        .Get_Object(Am_UNDO_SCROLL_GROUP)
                        .Get_Object(Am_ARROW_INTERACTOR);
  return inter;
}

Am_Define_Object_Formula(get_undos_search_dialog)
{
  Am_Object search_window;
  Am_Object undo_handler = get_undo_handler(self);
  if (undo_handler.Valid()) {
    search_window = undo_handler.Peek(Am_SEARCH_DIALOG);
  }
  return search_window;
}

///////////////////////////////////////////////////////////////////////////
// Initialize
///////////////////////////////////////////////////////////////////////////

Am_Object Am_Show_Undo_Dialog_Box_Command;

void
Am_Initialize_Undo_Dialog_Box()
{
  Am_Script_Window_Initialize();
  am_objgen_Initialize();
  am_locgen_Initialize();
  am_valgen_Initialize();
  am_askselectobj_Initialize();
  am_askclickpos_Initialize();
  am_askforvalue_Initialize();
  am_invoke_window_Initialize();
  am_valinvokegen_Initialize();
  am_commandinvokegen_Initialize();

  Am_Execute_Script_Command =
      Am_Command.Create(DSTR("Am_Execute_Script_Command"))
          .Set(Am_DO_METHOD, execute_script_do)
          .Set(Am_UNDO_METHOD, execute_script_undo)
          .Set(Am_REDO_METHOD, execute_script_redo)
          .Set(Am_SELECTIVE_REPEAT_SAME_METHOD, execute_script_repeat)
          .Set(Am_SELECTIVE_REPEAT_ON_NEW_METHOD, execute_script_repeat_new)
          // no particular reason why not selective-undoable.  But would need
          // something like do_execute_command that made the copy
          .Set(Am_SELECTIVE_UNDO_ALLOWED, Am_Selective_Allowed_Return_False)
          .Set(Am_SELECTIVE_REPEAT_NEW_ALLOWED,
               Am_Selective_New_Allowed_Return_True)
          .Set(Am_SELECTIVE_REPEAT_SAME_ALLOWED,
               Am_Selective_Allowed_Return_True)

          .Set(Am_LABEL, "Execute_Script")
          .Add(Am_SHOW_SCRIPT_COMMAND, (0L))
          .Add(Am_COMMANDS_IN_SCRIPT, (0L))
          .Add(Am_COMMANDS_PROTOTYPES, (0L))
          .Set(Am_VALUE, Am_No_Value)
          // .Add(Am_COMMANDS_IN_SCRIPT, (0L)) //set by the demon
          .Add(Am_ORIGINAL_COMMAND_LIST, (0L))
          .Add(Am_PLACEHOLDER_LIST, (0L))
          .Add(Am_PLACEHOLDER_COUNT, (0L))

          .Add(Am_BEFORE_OR_AFTER, (0L))
          .Add(Am_BEFORE_OR_AFTER_COMMAND, (0L));

  Am_Object script_group_proto = Am_Group.Create(DSTR("script_group_proto"))
                                     .Set(Am_LAYOUT, Am_Horizontal_Layout)
                                     .Set(Am_V_ALIGN, Am_TOP_ALIGN)
                                     .Set(Am_WIDTH, Am_Width_Of_Parts)
                                     .Set(Am_HEIGHT, Am_Height_Of_Parts)
                                     .Set(Am_MAX_SIZE, 500)
                                     .Set(Am_INDENT, 40)
                                     .Add(Am_VALUE, Am_No_Value);
  Am_Script_Line_Group =
      script_group_proto.Create(DSTR("Am_Script_Line_Group"));
  Am_Script_Line_Value_Group =
      script_group_proto.Create(DSTR("Am_Script_Line_Value_Group"));

  Am_Script_Line_Command = Am_Text.Create(DSTR("Am_Script_Line_Command"))
                               .Set(Am_LINE_STYLE, script_line_text_color);
  Am_Script_Line_Selectable = Am_Text.Create(DSTR("Am_Script_Line_Selectable"))
                                  .Set(Am_LINE_STYLE, script_line_text_color);
  Am_Script_List_Header =
      Am_Script_Line_Selectable.Create(DSTR("Am_Script_List_Header"));
  Am_Script_Line_Modified =
      Am_Script_Line_Selectable.Create(DSTR("Am_Script_Line_Modified"))
          .Add(Am_SLOTS_TO_SAVE, Am_OBJECT_MODIFIED);
  Am_Script_Line_Value =
      Am_Script_Line_Selectable.Create(DSTR("Am_Script_Line_Value"))
          .Add(Am_SLOTS_TO_SAVE, Am_VALUE);

  /*
  Am_Object_Advanced temp = (Am_Object_Advanced&)Am_Execute_Script_Command;
  Am_Demon_Set demons = temp.Get_Demons ().Copy ();
  demons.Set_Part_Demon (Am_ADD_PART, am_script_command_add_part);
  temp.Set_Demons (demons);
  */

  Am_Show_Script_Command = Am_Command.Create(DSTR("Am_Show_Script_Command"))
                               .Set(Am_DO_METHOD, show_script_do)
                               .Set(Am_LABEL, "Show Script")
                               .Add(Am_SCRIPT_EXECUTE_COMMAND, Am_No_Object);

  Am_Object scripts_start;

  Am_Value_List l = Am_Value_List().Add(
      Am_Command.Create(DSTR("File"))
          .Set(Am_LABEL, "File")
          .Set(Am_ITEMS, Am_Value_List()
                             .Add(Am_Command.Create(DSTR("Close"))
                                      .Set(Am_LABEL, "Close")
                                      .Set(Am_DO_METHOD, do_dismiss)
                                      .Set(Am_ACTIVE, true))
                             .Add(Am_Menu_Line_Command.Create())
                             .Add(Am_About_Amulet_Command.Create())));
  l.Add(Am_Command.Create(DSTR("Undoing"))
            .Set(Am_LABEL, "Undo/Redo/Repeat")
            .Set(Am_ITEMS,
                 Am_Value_List()
                     .Add(Am_Command.Create(DSTR("Undo Last"))
                              .Set(Am_LABEL, "Undo Last")
                              .Set(Am_DO_METHOD, do_undo_last)
                              .Set(Am_ACCELERATOR, Am_Input_Char("CONTROL_z"))
                              .Set(Am_ACTIVE, last_undoable))
                     .Add(Am_Command.Create(DSTR("Redo undone command"))
                              .Set(Am_LABEL, "Redo Undone Command")
                              .Set(Am_ACCELERATOR, Am_Input_Char("CONTROL_Z"))
                              .Set(Am_DO_METHOD, am_do_redo)
                              .Set(Am_ACTIVE, redoable))
                     .Add(Am_Command.Create(DSTR("Undo This"))
                              .Set(Am_LABEL, "Undo This")
                              .Set(Am_DO_METHOD, do_undo_this)
                              .Set(Am_ACTIVE, selection_undoable))
                     .Add(Am_Command.Create(DSTR("Repeat This"))
                              .Set(Am_LABEL, "Repeat This")
                              .Set(Am_DO_METHOD, do_repeat_this)
                              .Set(Am_ACTIVE, selection_repeatable))
                     .Add(Am_Command
                              .Create(DSTR("Repeat This on Current Selection"))
                              .Set(Am_LABEL, "Repeat This on Current Selection")
                              .Set(Am_DO_METHOD, do_repeat_this_on_new)
                              .Set(Am_ACCELERATOR, Am_Input_Char("REDO"))
                              .Set(Am_ACTIVE, selection_repeatable_on_new))));
  l.Add(Am_Command.Create(DSTR("Finding"))
            .Set(Am_LABEL, "Find")
            .Set(Am_ITEMS,
                 Am_Value_List()
                     .Add(Am_Show_Search_Command.Create().Set(
                         Am_SEARCH_DIALOG, get_undos_search_dialog))
                     .Add(Am_Command.Create(DSTR("Flash"))
                              .Set(Am_LABEL, "Flash Object")
                              .Set(Am_DO_METHOD, do_flash)
                              .Set(Am_ACTIVE, selection_flashable))
                     .Add(Am_Command.Create(DSTR("FindCommand"))
                              .Set(Am_LABEL, "Find Commands for Selection")
                              .Set(Am_DO_METHOD, do_find_command_for_selection)
                              .Set(Am_ACTIVE, exists_one_or_more_selection))));
  l.Add(Am_Command.Create(DSTR("Scripting"))
            .Set(Am_LABEL, "Scripting")
            .Set(Am_ITEMS,
                 Am_Value_List()
                     .Add(Am_Command.Create(DSTR("Expand"))
                              .Set(Am_LABEL, "Expand")
                              .Set(Am_DO_METHOD, (0L) /* expand_command */)
                              .Set(Am_ACTIVE, selection_is_multiple))
                     .Add(Am_Command.Create(DSTR("Create Script"))
                              .Set(Am_LABEL, "Create Script for Commands...")
                              .Set(Am_DO_METHOD, start_create_script_window)
                              .Set(Am_ACTIVE, exists_one_or_more_selection))
                     .Add(Am_Command.Create(DSTR("Show Script"))
                              .Set(Am_LABEL, "Find Script for Command...")
                              .Set(Am_DO_METHOD,
                                   (0L) /* display_script_for_selected */)
                              .Set(Am_ACTIVE, false /* exists_selection */))
                     .Add(scripts_start = Am_Menu_Line_Command.Create())));
  Am_Undo_Dialog_Box =
      Am_Window.Create(DSTR("Undo Dialog Box"))
          .Set(Am_WIDTH, 425) //defaults, can be overridden
          .Set(Am_HEIGHT, 400)
          .Set(Am_TITLE, "Amulet Selective Undo/Redo/Repeat")
          .Set(Am_ICON_TITLE, "Selective Undo/Redo/Repeat")
          .Add(Am_FONT, Am_Default_Font)
          .Set(Am_FILL_STYLE, Am_Default_Color)
          .Add(Am_WIDGET_LOOK, Am_Default_Widget_Look)
          .Add(Am_UNDO_HANDLER_TO_DISPLAY,
               (0L))                      //set with one to show values for
          .Add(Am_SELECTION_WIDGET, (0L)) //set with selection handles widget
          .Add(Am_SCROLLING_GROUP_SLOT, (0L)) //set with scrolling group widget
          //internal, do not set
          .Set(Am_UNDO_HANDLER, (0L)) //don't undo stuff done here!
          // if close this window from WM, just hide it
          .Set(Am_DESTROY_WINDOW_METHOD, Am_Window_Hide_Method);
  Am_Undo_Dialog_Box.Add_Part(
      Am_UNDO_MENU_BAR,
      Am_Menu_Bar.Create(DSTR("Am_UNDO_MENU_BAR")).Set(Am_ITEMS, l));
  Am_Undo_Dialog_Box.Add_Part(
      Am_UNDO_LABEL, Am_Text.Create(DSTR("Label"))
                         .Set(Am_FONT, bold_bigger_font)
                         .Set(Am_LEFT, 5)
                         .Set(Am_TOP, Am_Bottom_Of_Sibling(Am_UNDO_MENU_BAR, 5))
                         .Set(Am_TEXT, "Select Command to Undo or Repeat:"));
  Am_Undo_Dialog_Box.Set(Am_BOTTOM_OFFSET, 5)
      .Add_Part(
          Am_UNDO_OPTIONS,
          Am_Checkbox_Panel.Create(DSTR("UNDO_OPTIONS"))
              .Set(Am_LEFT, 5)
              .Set(Am_TOP, Am_Bottom_Is_Bottom_Of_Owner)
              .Set(Am_FONT, Am_From_Owner(Am_FONT))
              .Set(Am_FILL_STYLE, Am_From_Owner(Am_FILL_STYLE))
              .Set(Am_WIDGET_LOOK, Am_From_Owner(Am_WIDGET_LOOK))
              .Set(Am_FIXED_WIDTH, false)
              .Set(Am_LAYOUT, Am_Horizontal_Layout)
              .Set(Am_ITEMS,
                   Am_Value_List()
                       .Add(Am_Command.Create(DSTR("Record Selections"))
                                .Set(Am_LABEL, "Record Selections")
                                .Set(Am_DO_METHOD, toggle_record_selections)
                                .Set(Am_ACTIVE, true)
                                .Set(Am_VALUE, false))
                       .Add(Am_Command.Create(DSTR("Record Scrolling"))
                                .Set(Am_LABEL, "Record Scrolling")
                                .Set(Am_DO_METHOD, toggle_record_scrolling)
                                .Set(Am_ACTIVE, true)
                                .Set(Am_VALUE, false))));
  Am_Undo_Dialog_Box.Add_Part(
      Am_UNDO_SCROLL_GROUP,
      Am_Scrolling_Menu.Create(DSTR("UNDO_SCROLL_MENU"))
          .Set(Am_LEFT, 5)
          .Set(Am_TOP, Am_Bottom_Of_Sibling(Am_UNDO_LABEL, 5))
          .Set(Am_HEIGHT, Am_Rest_Of_Height_Above(Am_UNDO_OPTIONS, 3))
          .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH, -10))
          .Set(Am_FILL_STYLE, Am_From_Owner(Am_FILL_STYLE))
          .Set(Am_WIDGET_LOOK, Am_From_Owner(Am_WIDGET_LOOK))
          .Set(Am_ITEMS, generate_undo_menu_items)
          .Set(Am_H_SCROLL_BAR, true)
          .Add_Part(Am_ARROW_INTERACTOR,
                    Am_One_Shot_Interactor.Create(DSTR("double_clk"))
                        .Set(Am_PRIORITY, 101)
                        .Set(Am_START_WHEN, Am_Input_Char("double_left_up"))
                        .Get_Object(Am_COMMAND)
                        .Set(Am_DO_METHOD, double_create_script_window)
                        .Get_Owner())); // end of undo dialog box
  Am_Undo_Dialog_Box.Get_Object(Am_UNDO_SCROLL_GROUP)
      .Get_Object(Am_SUB_MENU)
      .Get_Object(Am_INTERACTOR)
      .Set(Am_RUN_ALSO, get_double_click_inter);

  Am_Show_Undo_Dialog_Box_Command =
      Am_Command.Create(DSTR("Show_Undo_DB"))
          .Set(Am_DO_METHOD, am_show_undo_box)
          .Set(Am_LABEL, "Undo/Redo/Repeat...")
          .Set(Am_IMPLEMENTATION_PARENT, Am_NOT_USUALLY_UNDONE);
  Am_Initialize_Generalizers();
}

Am_Initializer *undo_db_init = new Am_Initializer(
    DSTR("Undo_Dialogbox"), Am_Initialize_Undo_Dialog_Box, 100.0);

/*

Options for colors/everything else
 - constant
 - ask user
 - from the palette (or whatever registers as returning this value)
 - same as other objects

*/
