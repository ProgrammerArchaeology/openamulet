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

// This file contains the basic functions for Command objects and
// Undo handling
//
// Designed and implemented by Brad Myers

#include <string.h>

#include <am_inc.h>
#include <amulet/am_io.h>
#include <amulet/impl/types_logging.h>
#include <amulet/impl/types_string.h>
#include <amulet/impl/types_method.h>

#include <amulet/inter_advanced.h>
#include <amulet/impl/am_object.h>
#include <amulet/impl/inter_commands.h>
//#include <amulet/impl/inter_undo.h>
#include <amulet/impl/slots.h>
#include <amulet/value_list.h>
//#include <amulet/formula.h>
#include <amulet/impl/opal_misc.h>
#include <amulet/impl/opal_op.h>
//#include <amulet/impl/opal_objects.h>
#include <amulet/impl/method_object.h>

////////////////////////////////////////////////////////////
// Utility functions to process all the relevant commands
////////////////////////////////////////////////////////////

Am_Object
Am_Inter_Find_Undo_Handler(const Am_Object &inter_or_widget)
{
  Am_Value value;
  Am_Object undo_object, window;
  value = inter_or_widget.Peek(Am_WINDOW);
  if (value.Valid()) {
    window = value;
    value = window.Peek(Am_UNDO_HANDLER);
    if (value.Valid())
      undo_object = value;
  }
  return undo_object;
}

// command_obj's DO has already been "done", now do the DO methods of
// the Am_IMPLEMENTATION_PARENT's of command_obj, recursively on up
void
Am_Process_All_Parent_Commands(Am_Object &command_obj, Am_Object &undo_handler,
                               const Am_Register_Command_Method &reg_method)
{
  Am_Value value;
  Am_Object_Method do_method;
  while (true) {
    value = command_obj.Peek(Am_IMPLEMENTATION_PARENT);
    if (!value.Valid() || value.type != Am_OBJECT)
      break;
    //here is an object, see if right type
    command_obj = value;
    if (!command_obj.Is_Instance_Of(Am_Command))
      break;
    value = command_obj.Peek(Am_DO_METHOD);
    if (Am_Object_Method::Test(value)) {
      command_obj.Set(Am_COMMAND_IS_ABORTING, false, Am_OK_IF_NOT_THERE);
      do_method = value;
      Am_INTER_TRACE_PRINT(command_obj, "%%Executing Am_DO_METHOD of "
                                            << command_obj << "=" << do_method);
      do_method.Call(command_obj);
      if ((bool)command_obj.Get(Am_COMMAND_IS_ABORTING)) {
        return; //break out of loop without registering this command
      }
    }
    if (reg_method.Valid())
      reg_method.Call(undo_handler, command_obj);
  }
}

void
Am_Execute_Command(Am_Object &command, const Am_Object &widget)
{
  // is this really a command_object?
  if (!command.Is_Instance_Of(Am_Command)) {
    // no, than have a nice day
    return;
  }

  // get the value of the DO_METHOD
  Am_Value value = command.Peek(Am_DO_METHOD);

  // is the DO_METHOD an Am_Object_Method instance?
  if (!Am_Object_Method::Test(value)) {
    return;
  }

  // Create function object and call the registered DO_METHOD
  command.Set(Am_COMMAND_IS_ABORTING, false, Am_OK_IF_NOT_THERE);
  Am_Object_Method do_method(value);
  Am_INTER_TRACE_PRINT(command, "%%Executing Am_DO_METHOD of " << command << "="
                                                               << do_method);
  do_method.Call(command);

  // was (is?) the command aborted?
  if ((bool)command.Get(Am_COMMAND_IS_ABORTING)) {
    // yes, break out of loop without registering this command for undo
    return;
  }

  Am_Object undo_handler = Am_Inter_Find_Undo_Handler(widget);
  if (!undo_handler.Valid())
    return;

  Am_Register_Command_Method reg_method = undo_handler.Get(Am_REGISTER_COMMAND);
  if (reg_method.Valid())
    reg_method.Call(undo_handler, command);

  Am_Process_All_Parent_Commands(command, undo_handler, reg_method);
}

//returns true if has a valid Am_IMPLEMENTATION_PARENT.  Also, if parent is a
//command object, sets its Am_IMPLEMENTATION_CHILD slot to me
bool
has_impl_parent_and_set(Am_Object command_obj)
{
  Am_Value value;
  value = command_obj.Peek(Am_IMPLEMENTATION_PARENT);
  if (value.Valid() && value.type == Am_OBJECT) {
    //here is an object, see if right type
    Am_Object parent;
    parent = value;
    if (parent.Is_Instance_Of(Am_Command)) {
      Am_Value impl_value;
      impl_value = parent.Peek(Am_IMPLEMENTATION_CHILD);
      //don't store the child if parent says not to
      if (impl_value.type != Am_INT ||
          (int)impl_value != Am_DONT_UNDO_CHILDREN) {
        // put the actual command_obj there, will be copied later if necessary
        parent.Set(Am_IMPLEMENTATION_CHILD, command_obj);
      }
    }
  }
  return value.Valid(); //if Valid, then there is a parent
}

void
Am_Modify_Command_Label_Name(Am_Object cmd, Am_Slot_Key slot)
{
  Am_Value v_short, v;
  Am_String str;
  v_short = cmd.Peek(Am_SHORT_LABEL);
  if (v_short.Valid())
    v = v_short;
  else
    v = cmd.Peek(Am_LABEL);
  if (v.type == Am_STRING) {
    str = v;
    const char *old_string = str;
    const char *prefix = 0;
    switch (slot) {
    case Am_SELECTIVE_UNDO_METHOD:
      prefix = "Undo ";
      break;
    case Am_SELECTIVE_REPEAT_SAME_METHOD:
    case Am_SELECTIVE_REPEAT_ON_NEW_METHOD:
      prefix = "Repeat ";
      break;
    default:
      Am_Error("Bad slot in modify_label_name");
    }
    char *new_string =
        (char *)new char[(strlen(prefix) + strlen(old_string) + 1)];
    strcpy(new_string, prefix);
    strcat(new_string, old_string);
    //use false so doesn't copy the string since just allocated it
    Am_String str(new_string, false);
    if (v_short.Valid())
      cmd.Set(Am_SHORT_LABEL, str);
    else
      cmd.Set(Am_LABEL, str);
  }
}

Am_Object
am_perform_on_cmd_and_impls(Am_Object &last_command, Am_Slot_Key slot,
                            bool copy /* = false */,
                            Am_Value new_sel /* = Am_No_Value */,
                            Am_Value new_val /* = Am_No_Value */,
                            bool fix_name /* = true */)
{
  Am_Object_Method method;
  Am_Selective_Repeat_New_Method new_method;
  bool is_new = slot == Am_SELECTIVE_REPEAT_ON_NEW_METHOD;
  Am_Value value;
  Am_Object obj, prev;
#ifdef DEBUG
  bool debug;
#endif
  if (copy) {
    Am_INTER_TRACE_PRINT_NOENDL(last_command, "  Copying command obj "
                                                  << last_command);
    last_command = last_command.Copy_Value_Only();
    Am_INTER_TRACE_PRINT(last_command, " to " << last_command);
    if (fix_name)
      Am_Modify_Command_Label_Name(last_command, slot);
    prev = last_command;
  }
  obj = last_command;
  //first go through and make all copies, and find the leaf
  while (true) {
    value = obj.Peek(Am_IMPLEMENTATION_CHILD);
    if (value.type != Am_OBJECT)
      break;
    obj = value;
    if (copy) {
      Am_INTER_TRACE_PRINT_NOENDL(obj, "  Copying command obj " << obj);
      obj = obj.Copy_Value_Only();
      Am_INTER_TRACE_PRINT(last_command, " to " << obj);
      prev.Set(Am_IMPLEMENTATION_CHILD, obj); //set with copy
      obj.Set(Am_IMPLEMENTATION_PARENT, prev);
      prev = obj;
    }
  }
  //here, obj is the leaf (child-most) object in the chain
  while (true) {
#ifdef DEBUG
    debug = Am_Inter_Tracing(obj);
    if (debug) {
      switch (slot) {
      case Am_UNDO_METHOD:
        std::cout << "%%Executing Am_UNDO_METHOD";
        break;
      case Am_REDO_METHOD:
        std::cout << "%%Executing Am_REDO_METHOD";
        break;
      case Am_SELECTIVE_UNDO_METHOD:
        std::cout << "%%Executing Am_SELECTIVE_UNDO_METHOD";
        break;
      case Am_SELECTIVE_REPEAT_SAME_METHOD:
        std::cout << "%%Executing Am_SELECTIVE_REPEAT_SAME_METHOD";
        break;
      case Am_SELECTIVE_REPEAT_ON_NEW_METHOD:
        std::cout << "%%Executing Am_SELECTIVE_REPEAT_ON_NEW_METHOD";
        break;
      }
      std::cout << " of " << obj;
    }
#endif
    if (is_new) {
      new_method = obj.Get(Am_SELECTIVE_REPEAT_ON_NEW_METHOD);
      if (new_method.Valid()) {
        Am_INTER_TRACE_PRINT(obj, "=" << new_method);
        new_method.Call(obj, new_sel, new_val);
      } else {
        Am_INTER_TRACE_PRINT(obj, " but no method");
      }
    } else {
      method = obj.Get(slot);
      if (method.Valid()) {
        Am_INTER_TRACE_PRINT(obj, "=" << method);
        method.Call(obj);
      } else {
        Am_INTER_TRACE_PRINT(obj, " but no method");
      }
    }
    value = obj.Peek(Am_IMPLEMENTATION_PARENT);
    if (!value.Valid() || value.type != Am_OBJECT)
      break;
    obj = value;
  }
  return last_command;
}

//copies command_obj and any Am_IMPLEMENTATION_CHILD recursively
Am_Object
copy_command_tree(Am_Object command_obj)
{
  command_obj = command_obj.Copy_Value_Only();
  Am_Object child, parent, child_copy;
  parent = command_obj;
  Am_Value value;
  while (true) {
    value = parent.Peek(Am_IMPLEMENTATION_CHILD);
    if (value.Valid() && value.type == Am_OBJECT) {
      child = value;
      child_copy = child.Copy_Value_Only();
      parent.Set(Am_IMPLEMENTATION_CHILD, child_copy);
      child_copy.Set(Am_IMPLEMENTATION_PARENT, parent);
      parent = child_copy;
    } else
      break; //leave loop
  }
  return command_obj;
}

//destroys command_obj and any Am_IMPLEMENTATION_CHILD recursively
void
destroy_command_tree(Am_Object command_obj)
{
  Am_Object parent;
  parent = command_obj;
  Am_Value value;
  while (true) {
    value = parent.Peek(Am_IMPLEMENTATION_CHILD);
    parent.Destroy();
    if (value.Valid() && value.type == Am_OBJECT) {
      parent = value;
    } else
      break; //leave loop
  }
}

////////////////////////////////////////////////////////////
// Inactive slot of objects
////////////////////////////////////////////////////////////

//returns true if valid.  I.e. returns false if object is marked as
//invalid for slot_for_inactive
bool
Am_Check_One_Object_For_Inactive(Am_Object &object,
                                 Am_Slot_Key slot_for_inactive)
{
  Am_Value value;
  value = object.Peek(Am_INACTIVE_COMMANDS);
  if (value.Valid()) {
    Am_Object cmd = value;
    value = cmd.Peek(slot_for_inactive);
    if (value.Valid())
      return false;
  }
  return true; //if get here, all selected items are fine
}

static Am_Slot_Key
get_my_inactive_slot(Am_Object &self)
{
  Am_Value value;
  value = self.Peek(Am_CHECK_INACTIVE_COMMANDS, Am_NO_DEPENDENCY);
  if (value.Valid()) {
    value = self.Peek(Am_SLOT_FOR_THIS_COMMAND_INACTIVE, Am_NO_DEPENDENCY);
    if (value.Valid())
      return (Am_Slot_Key)(int)value;
  }
  return 0;
}
//uses Am_SLOT_FOR_THIS_COMMAND_INACTIVE of self
bool
Am_Check_One_Object_For_Inactive_Slot(Am_Object &object, Am_Object &self)
{
  Am_Slot_Key slot_for_inactive = get_my_inactive_slot(self);
  if (slot_for_inactive != 0)
    return Am_Check_One_Object_For_Inactive(object, slot_for_inactive);
  else
    return true;
}

//returns false if any items in selection are marked as invalid for
//slot_for_inactive
bool
Am_Check_All_Objects_For_Inactive(Am_Value_List &selection,
                                  Am_Slot_Key slot_for_inactive)
{
  Am_Value value;
  Am_Object obj, cmd;
  for (selection.Start(); !selection.Last(); selection.Next()) {
    obj = selection.Get();
    value = obj.Peek(Am_INACTIVE_COMMANDS, Am_NO_DEPENDENCY);
    if (value.Valid()) {
      cmd = value;
      value = cmd.Peek(slot_for_inactive);
      if (value.Valid())
        return false;
    }
  }
  return true; //if get here, all selected items are fine
}

bool
Am_Check_All_Objects_For_Inactive_Slot(Am_Value_List &selection,
                                       Am_Object &self)
{
  Am_Slot_Key slot_for_inactive = get_my_inactive_slot(self);
  if (slot_for_inactive != 0)
    return Am_Check_All_Objects_For_Inactive(selection, slot_for_inactive);
  else
    return true;
}

bool
Am_Check_One_Or_More_For_Inactive(Am_Value list_or_obj,
                                  Am_Slot_Key slot_for_inactive)
{
  if (!list_or_obj.Valid())
    return true; //if no value, then OK
  if (list_or_obj.type == Am_OBJECT) {
    Am_Object obj;
    obj = list_or_obj;
    return Am_Check_One_Object_For_Inactive(obj, slot_for_inactive);
  } else if (Am_Value_List::Test(list_or_obj)) {
    Am_Value_List vl;
    vl = list_or_obj;
    return Am_Check_All_Objects_For_Inactive(vl, slot_for_inactive);
  } else
    Am_ERROR("Wrong type value " << list_or_obj << ".  Should be obj or list");
  return false; // never gets here
}
bool
Am_Check_One_Or_More_For_Inactive_Slot(Am_Value list_or_obj, Am_Object &self)
{
  Am_Slot_Key slot_for_inactive = get_my_inactive_slot(self);
  if (slot_for_inactive != 0)
    return Am_Check_One_Or_More_For_Inactive(list_or_obj, slot_for_inactive);
  else
    return true;
}

//returns false if inactive so should abort, true if OK
bool
Am_Check_Inter_Abort_Inactive_Object(Am_Object &object,
                                     Am_Slot_Key slot_for_inactive,
                                     Am_Object &inter)
{
  Am_Value value;
  value = inter.Peek(Am_CHECK_INACTIVE_COMMANDS);
  if (value.Valid()) {
    if (!Am_Check_One_Object_For_Inactive(object, slot_for_inactive)) {
      if ((bool)inter.Get(Am_INTER_BEEP_ON_ABORT))
        Am_Beep();
      Am_INTER_TRACE_PRINT(inter, "Inter " << inter
                                           << " aborting because "
                                              "Am_SLOT_FOR_THIS_COMMAND_"
                                              "INACTIVE of object "
                                           << object);
      return false;
    }
  }
  return true;
}
