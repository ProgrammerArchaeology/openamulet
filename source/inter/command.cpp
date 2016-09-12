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

#include <iostream>

Am_Object Am_Command; // base of the command hierarchy

AM_DEFINE_METHOD_TYPE_IMPL(Am_Selective_Allowed_Method);

////////////////////////////////////////////////////////////
// Multiple Undo, with selective undo
////////////////////////////////////////////////////////////

bool
Am_Object_Valid_And_Not_Part(Am_Object obj)
{
  if (!obj.Valid())
    return false;
  if (!(bool)obj.Get(Am_VISIBLE))
    return false;
  if (obj.Get_Owner().Valid())
    return false; // shouldn't have an owner
  return true;
}

bool
Am_Valid_and_Visible_List_Or_Object(Am_Value value, bool want_visible)
{
  if (!value.Valid())
    return true; //if no value, then OK
  if (value.type == Am_OBJECT) {
    Am_Object obj;
    obj = value;
    if (want_visible)
      return Am_Object_And_Owners_Valid_And_Visible(obj);
    else
      return Am_Object_Valid_And_Not_Part(obj);
  }
  //// BUG: Need to move add_ref to setter to avoid this annoyance
  else if (Am_Type_Class(value.type) == Am_WRAPPER &&
           Am_Value_List::Test(value.value.wrapper_value)) {
    // test each value and return true if all ok
    Am_Value_List l;
    l = value;
    Am_Object o;
    for (l.Start(); !l.Last(); l.Next()) {
      o = l.Get();
      if (want_visible) {
        if (!Am_Object_And_Owners_Valid_And_Visible(o))
          return false;
      } else //want it to be valid but invisible
          if (!Am_Object_Valid_And_Not_Part(o))
        return false;
    }
    return true; //all are ok
  } else
    return true; //slot does not contain an object
}

Am_Define_Method(Am_Selective_Allowed_Method, bool,
                 Am_Standard_Selective_Allowed, (Am_Object command_obj))
{
  if (!command_obj.Valid())
    return false;
  Am_Value value;
  value = command_obj.Peek(Am_OBJECT_MODIFIED, Am_NO_DEPENDENCY);
  bool ret = Am_Valid_and_Visible_List_Or_Object(value);
  // std::cout << "*** valid and vis returns " << ret << " for obj mod " << value
  //     << " in command " << command_obj <<std::endl <<std::flush;
  return ret;
}

Am_Define_Method(Am_Selective_New_Allowed_Method, bool,
                 Am_Standard_Selective_New_Allowed,
                 (Am_Object command_obj, Am_Value new_selection,
                  Am_Value /* new_value */))
{
  if (Am_Valid_and_Visible_List_Or_Object(new_selection))
    return Am_Check_One_Or_More_For_Inactive_Slot(new_selection, command_obj);
  else
    return false;
}

static void
init()
{

  Am_Command =
      Am_Root_Object.Create(DSTR("Am_Command"))
          .Add(Am_DO_METHOD, (0L))
          .Add(Am_UNDO_METHOD, (0L))
          .Add(Am_REDO_METHOD, (0L))
          .Add(Am_SELECTIVE_UNDO_METHOD, (0L))
          .Add(Am_SELECTIVE_REPEAT_SAME_METHOD, (0L))
          .Add(Am_SELECTIVE_REPEAT_ON_NEW_METHOD, (0L))
          .Add(Am_SELECTIVE_UNDO_ALLOWED, Am_Standard_Selective_Allowed)
          .Add(Am_SELECTIVE_REPEAT_SAME_ALLOWED, Am_Standard_Selective_Allowed)
          .Add(Am_SELECTIVE_REPEAT_NEW_ALLOWED,
               Am_Standard_Selective_New_Allowed)

          .Add(Am_START_DO_METHOD, (0L))
          .Add(Am_INTERIM_DO_METHOD, (0L))
          .Add(Am_ABORT_DO_METHOD, (0L))

          .Add(Am_LABEL, "A command")
          .Add(Am_SHORT_LABEL, 0) //if 0 then use Am_LABEL
          .Add(Am_ACTIVE, true)
          .Add(Am_DEFAULT, false)
          .Add(Am_IMPLEMENTATION_PARENT, 0)
          .Add(Am_IMPLEMENTATION_CHILD, 0)
          .Add(Am_COMPOSITE_PARENT, 0)
          .Add(Am_COMPOSITE_CHILDREN, 0)
          .Add(Am_VALUE, 0)
          .Add(Am_OLD_VALUE, 0)
          .Add(Am_OBJECT_MODIFIED, 0)

          .Add(Am_SAVED_OLD_OWNER, (0L))
          .Set_Inherit_Rule(Am_SAVED_OLD_OWNER, Am_LOCAL)

          .Add(Am_SAVED_OLD_OBJECT_OWNER, (0L))

          .Add(Am_CHECK_INACTIVE_COMMANDS, true)

          // used by menus
          .Add(Am_ID, Am_No_Value)
          .Add(Am_ITEMS, 0)
          .Add(Am_ACCELERATOR, 0)

          .Add(Am_HAS_BEEN_UNDONE, false);
}

static Am_Initializer *initializer =
    new Am_Initializer(DSTR("Am_Command"), init, 3.11f);
