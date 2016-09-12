#include <am_inc.h>
#include INITIALIZER__H
#include VALUE_LIST__H
#include <amulet/impl/slots.h>
#include <amulet/impl/am_object.h>
#include <amulet/impl/types_method.h>
#include <amulet/impl/inter_op.h>
#include <amulet/impl/inter_undo.h>
#include <amulet/impl/inter_commands.h>
#include <amulet/impl/opal_objects.h>
#include <amulet/impl/method_object.h>
#include FORMULA__H

bool has_impl_parent_and_set(Am_Object command_obj);
Am_Object copy_command_tree(Am_Object command_obj);
void destroy_command_tree(Am_Object command_obj);

Am_Object Am_Single_Undo_Object; // can only undo last command

////////////////////////////////////////////////////////////
// Single Undo, no selective undo
////////////////////////////////////////////////////////////

Am_Define_Method(Am_Register_Command_Method, bool, single_undo_register_command,
                 (Am_Object undo_handler, Am_Object command_obj))
{
  if (has_impl_parent_and_set(command_obj))
    return false;
  else {
    Am_Object old_command;
    old_command = undo_handler.Get(Am_COMMAND);

    if (old_command.Valid())
      destroy_command_tree(old_command);
    undo_handler.Set(Am_COMMAND, copy_command_tree(command_obj));
    return true;
  }
}

Am_Define_Method(Am_Object_Method, void, single_perform_undo,
                 (Am_Object undo_handler))
{
  Am_Object last_command;
  last_command = undo_handler.Get(Am_COMMAND);
  if (!last_command.Valid())
    Am_Error("No last command to undo for Single Undo Handler");
  else {
    am_perform_on_cmd_and_impls(last_command, Am_UNDO_METHOD);
    // leave the command as current, in case Redo
  }
}

Am_Define_Method(Am_Object_Method, void, single_perform_redo,
                 (Am_Object undo_handler))
{
  Am_Object last_command;
  last_command = undo_handler.Get(Am_COMMAND);
  if (!last_command.Valid())
    Am_Error("No last command to Redo for Single Redo Handler");
  else {
    am_perform_on_cmd_and_impls(last_command, Am_REDO_METHOD);
    // leave the command as current, in case undo this
  }
}

// Used in single undo and Redo handlers in the Am_UNDO_ALLOWED and
// Am_REDO_ALLOWED slots (both use the same function)
Am_Define_Object_Formula(single_undo_allowed_form)
{
  Am_Object last_command;
  last_command = self.Get(Am_COMMAND);
  return last_command;
}

static void
init()
{
  Am_Single_Undo_Object =
      Am_Undo_Handler.Create(DSTR("Am_Single_Undo_Object"))
          .Set(Am_REGISTER_COMMAND, single_undo_register_command)
          .Set(Am_PERFORM_UNDO, single_perform_undo)
          .Set(Am_PERFORM_REDO, single_perform_redo)
          .Set(Am_UNDO_ALLOWED, single_undo_allowed_form)
          .Set(Am_REDO_ALLOWED, single_undo_allowed_form);
}

static Am_Initializer *initializer =
    new Am_Initializer(DSTR("Am_Undo_Handler"), init, 3.14f);
