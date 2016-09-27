#include <am_inc.h>
#include <amulet/initializer.h>
#include <amulet/value_list.h>
#include <amulet/impl/slots.h>
#include <amulet/impl/am_object.h>
#include <amulet/impl/types_method.h>
#include <amulet/impl/inter_op.h>
#include <amulet/impl/inter_undo.h>
#include <amulet/impl/inter_commands.h>
#include <amulet/impl/opal_objects.h>
#include <amulet/impl/method_object.h>
#include <amulet/formula.h>

#include <iostream>

bool has_impl_parent_and_set(Am_Object command_obj);
Am_Object copy_command_tree(Am_Object command_obj);
void destroy_command_tree(Am_Object command_obj);

Am_Object Am_Multiple_Undo_Object; // can undo all top-level commands

AM_DEFINE_METHOD_TYPE_IMPL(Am_Selective_New_Allowed_Method);
AM_DEFINE_METHOD_TYPE_IMPL(Am_Selective_Repeat_New_Method);
AM_DEFINE_METHOD_TYPE_IMPL(Am_Handler_Selective_Undo_Method);
AM_DEFINE_METHOD_TYPE_IMPL(Am_Handler_Selective_Repeat_New_Method);

////////////////////////////////////////////////////////////
// Multiple Undo
////////////////////////////////////////////////////////////

void
do_multiple_register(Am_Object undo_handler, Am_Object command_obj, bool copy)
{
  Am_Value_List command_list;
  undo_handler.Make_Unique(Am_COMMAND);
  command_list = undo_handler.Get(Am_COMMAND);
  if (copy)
    command_obj = copy_command_tree(command_obj);
  if (command_list.Empty()) {
    command_list.Add(command_obj, Am_HEAD);
    undo_handler.Set(Am_COMMAND, command_list);
  } else {
    // false so don't copy the wrapper object
    command_list.Add(command_obj, Am_HEAD, false);
    undo_handler.Note_Changed(Am_COMMAND);
  }

  // if a pending old_command for Redo, then remove it
  Am_Object old_command;
  old_command = undo_handler.Get(Am_LAST_UNDONE_COMMAND);
  if (old_command.Valid())
    destroy_command_tree(old_command);
  undo_handler.Set(Am_LAST_UNDONE_COMMAND, 0);
}

Am_Define_Method(Am_Register_Command_Method, bool,
                 multiple_undo_register_command,
                 (Am_Object undo_handler, Am_Object command_obj))
{
  if (has_impl_parent_and_set(command_obj))
    return false;
  else {
    do_multiple_register(undo_handler, command_obj, true);
    return true;
  }
}

Am_Define_Method(Am_Object_Method, void, multiple_perform_undo,
                 (Am_Object undo_handler))
{
  Am_Value_List command_list;
  command_list = undo_handler.Get(Am_COMMAND);
  if (command_list.Empty())
    Am_Error("Command list is empty for Multiple Undo Handler");
  else { // undo the current item and all its Am_IMPLEMENTATION_CHILD
    command_list.Start();
    Am_Object last_command;
    last_command = command_list.Get();
    // false so don't copy the list
    command_list.Delete(false); // pop it from the undo list
    // now do the undo action
    am_perform_on_cmd_and_impls(last_command, Am_UNDO_METHOD);

    // remember the command in case Redo
    Am_Object old_command;
    old_command = undo_handler.Get(Am_LAST_UNDONE_COMMAND);
    // destroy any old one saved there
    if (old_command.Valid())
      destroy_command_tree(old_command);
    // now save the new last_command
    undo_handler.Set(Am_LAST_UNDONE_COMMAND, last_command);
    undo_handler.Note_Changed(Am_COMMAND); //destructively modified value
  }
}

Am_Define_Method(Am_Object_Method, void, multiple_perform_redo,
                 (Am_Object undo_handler))
{
  // if a pending last_command for Redo, then do it
  Am_Object last_command;
  last_command = undo_handler.Get(Am_LAST_UNDONE_COMMAND);
  if (!last_command.Valid())
    Am_Error("No last command to Redo for Multiple Redo Handler");
  else {
    am_perform_on_cmd_and_impls(last_command, Am_REDO_METHOD);

    // now put the last_command back on the undo list so can be undone
    Am_Value_List command_list;
    // we know list must exist, or else couldn't have done the first
    // undo, so don't have to check for it to exist here.
    command_list = undo_handler.Get(Am_COMMAND, Am_NO_DEPENDENCY);
    // push new one on the front.  false so don't copy the list
    command_list.Add(last_command, Am_HEAD, false);
    undo_handler.Note_Changed(Am_COMMAND); //destructively modified
    // clear out the last_command
    undo_handler.Set(Am_LAST_UNDONE_COMMAND, 0);
  }
}

// Used in multiple undo handlers in the Am_UNDO_ALLOWED slot
Am_Define_Object_Formula(multiple_undo_allowed_form)
{
  Am_Value_List command_list;
  command_list = self.Get(Am_COMMAND);
  if (command_list.Empty())
    return nullptr;
  else {
    command_list.Start();
    return command_list.Get();
  }
}

// Used in multiple undo handlers in the Am_REDO_ALLOWED slot
Am_Define_Object_Formula(multiple_redo_allowed_form)
{
  Am_Object last_command;
  last_command = self.Get(Am_LAST_UNDONE_COMMAND);
  return last_command;
}

//copy command_obj and all children, perform the selective undo action, then
//queue the new (copied) commands onto the front of the command list
Am_Define_Method(Am_Handler_Selective_Undo_Method, Am_Object,
                 handler_perform_selective_undo,
                 (Am_Object undo_handler, Am_Object command_obj))
{
  bool should_register = !command_obj.Peek(Am_IMPLEMENTATION_PARENT).Valid();
  Am_Object new_top =
      am_perform_on_cmd_and_impls(command_obj, Am_SELECTIVE_UNDO_METHOD, true,
                                  Am_No_Value, Am_No_Value, should_register);
  if (should_register)
    do_multiple_register(undo_handler, new_top, false);
  return new_top;
}

////// For the Undo Handler slots of selective undo handlers

//if command_obj or its first child that has a method say its OK
Am_Define_Method(Am_Selective_Allowed_Method, bool,
                 handler_selective_undo_allowed, (Am_Object command_obj))
{
  Am_Selective_Allowed_Method allowed_method;
  Am_Object_Method method;
  Am_Value value;
  while (true) {
    if (!command_obj.Valid())
      return false;
    allowed_method =
        command_obj.Get(Am_SELECTIVE_UNDO_ALLOWED, Am_NO_DEPENDENCY);
    if (allowed_method.Valid())
      if (!(allowed_method.Call(command_obj)))
        return false;
    method = command_obj.Get(Am_SELECTIVE_UNDO_METHOD, Am_NO_DEPENDENCY);
    if (method.Valid())
      return true;
    value = command_obj.Peek(Am_IMPLEMENTATION_CHILD, Am_NO_DEPENDENCY);
    if (value.Valid() && value.type == Am_OBJECT)
      command_obj = value;
    else
      return false;
  }
}

//if command_obj or its first child that has a method say its OK
Am_Define_Method(Am_Selective_Allowed_Method, bool,
                 handler_selective_repeat_same_allowed, (Am_Object command_obj))
{
  Am_Selective_Allowed_Method allowed_method;
  Am_Value value;
  while (true) {
    if (!command_obj.Valid())
      return false;
    allowed_method =
        command_obj.Get(Am_SELECTIVE_REPEAT_SAME_ALLOWED, Am_NO_DEPENDENCY);
    if (allowed_method.Valid())
      if (!(allowed_method.Call(command_obj)))
        return false;
    Am_Object_Method method;
    method = command_obj.Get(Am_SELECTIVE_REPEAT_SAME_METHOD, Am_NO_DEPENDENCY);
    if (method.Valid())
      return true;

    value = command_obj.Peek(Am_IMPLEMENTATION_CHILD, Am_NO_DEPENDENCY);
    if (value.Valid() && value.type == Am_OBJECT)
      command_obj = value;
    else
      return false;
  }
}

//if command_obj or its first child that has a method say its OK
Am_Define_Method(Am_Selective_New_Allowed_Method, bool,
                 handler_selective_new_allowed,
                 (Am_Object command_obj, Am_Value new_selection,
                  Am_Value new_value))
{
  Am_Selective_New_Allowed_Method allowed_method;
  Am_Value value;
  while (true) {
    if (!command_obj.Valid())
      return false;
    allowed_method =
        command_obj.Get(Am_SELECTIVE_REPEAT_NEW_ALLOWED, Am_NO_DEPENDENCY);
    if (allowed_method.Valid())
      if (!(allowed_method.Call(command_obj, new_selection, new_value)))
        return false;
    Am_Selective_Repeat_New_Method method;
    method =
        command_obj.Get(Am_SELECTIVE_REPEAT_ON_NEW_METHOD, Am_NO_DEPENDENCY);
    if (method.Valid())
      return true;
    value = command_obj.Peek(Am_IMPLEMENTATION_CHILD, Am_NO_DEPENDENCY);
    if (value.Valid() && value.type == Am_OBJECT)
      command_obj = value;
    else
      return false;
  }
}

Am_Define_Method(Am_Selective_Allowed_Method, bool,
                 Am_Selective_Allowed_Return_True,
                 (Am_Object /* command_obj */))
{
  return true;
}

Am_Define_Method(Am_Selective_Allowed_Method, bool,
                 Am_Selective_Allowed_Return_False, (Am_Object /* cmd */))
{
  return false;
}
Am_Define_Method(Am_Selective_New_Allowed_Method, bool,
                 Am_Selective_New_Allowed_Return_False,
                 (Am_Object /* cmd */, Am_Value /* new_selection */,
                  Am_Value /* new_value */))
{
  return false;
}

Am_Define_Method(Am_Selective_New_Allowed_Method, bool,
                 Am_Selective_New_Allowed_Return_True,
                 (Am_Object /* cmd */, Am_Value /* new_selection */,
                  Am_Value /* new_value */))
{
  return true;
}

//copy command_obj and all children, perform the selective repeat action,
//then queue the new (copied) commands onto the front of the command list
Am_Define_Method(Am_Handler_Selective_Undo_Method, Am_Object,
                 handler_perform_selective_repeat,
                 (Am_Object undo_handler, Am_Object command_obj))
{
  bool should_register = !command_obj.Peek(Am_IMPLEMENTATION_PARENT).Valid();
  Am_Object new_top = am_perform_on_cmd_and_impls(
      command_obj, Am_SELECTIVE_REPEAT_SAME_METHOD, true, Am_No_Value,
      Am_No_Value, should_register);
  if (should_register)
    do_multiple_register(undo_handler, new_top, false);
  return new_top;
}

//copy command_obj and all children, perform the selective repeat on new
//action, then queue the new commands onto the front of the command list
Am_Define_Method(Am_Handler_Selective_Repeat_New_Method, Am_Object,
                 handler_perform_selective_repeat_on_new,
                 (Am_Object undo_handler, Am_Object command_obj,
                  Am_Value new_selection, Am_Value new_value))
{
  bool should_register = !command_obj.Peek(Am_IMPLEMENTATION_PARENT).Valid();
  Am_Object new_top = am_perform_on_cmd_and_impls(
      command_obj, Am_SELECTIVE_REPEAT_ON_NEW_METHOD, true, new_selection,
      new_value, should_register);
  if (should_register)
    do_multiple_register(undo_handler, new_top, false);
  return new_top;
}

static void
init()
{
  Am_Multiple_Undo_Object =
      Am_Undo_Handler.Create(DSTR("Am_Multiple_Undo_Object"))
          .Set(Am_REGISTER_COMMAND, multiple_undo_register_command)
          .Set(Am_PERFORM_UNDO, multiple_perform_undo)
          .Set(Am_PERFORM_REDO, multiple_perform_redo)
          .Set(Am_UNDO_ALLOWED, multiple_undo_allowed_form)
          .Set(Am_REDO_ALLOWED, multiple_redo_allowed_form)
          .Add(Am_LAST_UNDONE_COMMAND, 0)

          .Set(Am_SELECTIVE_UNDO_METHOD, handler_perform_selective_undo)
          .Set(Am_SELECTIVE_REPEAT_SAME_METHOD,
               handler_perform_selective_repeat)
          .Set(Am_SELECTIVE_REPEAT_ON_NEW_METHOD,
               handler_perform_selective_repeat_on_new)

          .Set(Am_SELECTIVE_UNDO_ALLOWED, handler_selective_undo_allowed)
          .Set(Am_SELECTIVE_REPEAT_SAME_ALLOWED,
               handler_selective_repeat_same_allowed)
          .Set(Am_SELECTIVE_REPEAT_NEW_ALLOWED, handler_selective_new_allowed);
}

static Am_Initializer *initializer =
    new Am_Initializer(DSTR("Am_Undo_Handler"), init, 3.14f);
