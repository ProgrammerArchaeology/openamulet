/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* Support for generalizing script actions
   
   Designed and implemented by Brad Myers
*/

#include <am_inc.h>
#include AM_IO__H

#include STANDARD_SLOTS__H
#include VALUE_LIST__H
#include SCRIPTING_ADVANCED__H
#include STR_STREAM__H
#include SCRIPTING__H

#include OPAL_ADVANCED__H //for Am_Beep_Happened

#include <amulet/impl/widget_misc.h>

AM_DEFINE_METHOD_TYPE_IMPL(Am_Placeholder_Replace_Method)
AM_DEFINE_METHOD_TYPE_IMPL(Am_Placeholder_Set_Value_Method)
AM_DEFINE_METHOD_TYPE_IMPL(Am_Placeholder_Create_Method)

//put into script when real object is the result of a previous create command
Am_Object Am_A_Placeholder; //prototype for all the placeholders
Am_Object Am_Constant_Placeholder;
Am_Object Am_Custom_Code_Placeholder;

//for objects
Am_Object Am_All_Values_From_Command_Placeholder;
Am_Object Am_An_Object_From_Command_Placeholder;
Am_Object Am_The_Selected_Objects_Placeholder;
Am_Object Am_User_Select_At_Run_Time_Placeholder;

//for locations
Am_Object Am_Inter_Location_Placeholder;

//for properties and other values
Am_Object Am_Ask_User_Property_Placeholder;
Am_Object Am_Property_From_Palette_Placeholder;
Am_Object Am_Property_From_Object_Placeholder;
Am_Object Am_Cycle_Values_Placeholder;

// match placeholders
Am_Object Am_Type_Match_Placeholder;
Am_Object Am_Any_Value_Placeholder;

//slots in the placeholder objects
Am_Slot_Key Am_PLACEHOLDER_REPLACE_METHOD = //replaces with actual value
    Am_Register_Slot_Name("~PLACEHOLDER_REPLACE_METHOD~");

Am_Slot_Key Am_PLACEHOLDER_SET_VALUE_METHOD = //set value into ph
    Am_Register_Slot_Name("~PLACEHOLDER_SET_VALUE_METHOD~");

Am_Slot_Key Am_PLACEHOLDER_CREATE_METHOD = //set value into ph
    Am_Register_Slot_Name("~PLACEHOLDER_CREATE_METHOD~");

Am_Slot_Key Am_PLACEHOLDER_MATCH_METHOD = //match value in ph
    Am_Register_Slot_Name("~PLACEHOLDER_MATCH_METHOD~");

Am_Slot_Key Am_PART_CHAIN = Am_Register_Slot_Name("~PART_CHAIN~");

//set into cmds that have placeholders
Am_Slot_Key Am_PLACEHOLDERS_SET_BY_THIS_COMMAND =
    Am_Register_Slot_Name("~PLACEHOLDERS_SET_BY_THIS_COMMAND~");
Am_Slot_Key Am_PLACEHOLDER_INITIALIZED =
    Am_Register_Slot_Name("~Am_PLACEHOLDER_INITIALIZED~");

Am_Slot_Key Am_REGISTRY_FOR_PALETTES =
    Am_Register_Slot_Name("Am_REGISTRY_FOR_PALETTES");

///////////////////////////////  executing the placeholders ///////////////

//for the placeholders that can just return their value
Am_Define_Method(Am_Placeholder_Replace_Method, bool, value_replace_placeholder,
                 (Am_Object & ph, Am_Value &new_value,
                  Am_Object & /* sel_widget */))
{
  new_value = ph.Get(Am_VALUE);
  return true;
}

//return is whether this is OK or not.  orig_value is set if
//did_a_replace is set with true, in which case the slot of cmd is set with
//the replaced value
bool
Am_Replace_Placeholders(Am_Object &cmd, Am_Slot_Key slot, bool &did_a_replace,
                        Am_Value &orig_value, Am_Value &new_value,
                        Am_Object &sel_widget)
{
  orig_value = cmd.Peek(slot);
  if (am_sdebug)
    std::cout << "Checking placeholders for cmd " << cmd << " slot "
              << Am_Am_Slot_Key(slot) << " orig value " << orig_value
              << std::endl
              << std::flush;
  Am_Value val, ret_val;
  bool ok;
  did_a_replace = false;
  Am_Object ph;
  if (!orig_value.Valid())
    return true; //nothing to do, and is OK
  else if (Am_Value_List::Test(orig_value)) {
    Am_Value_List orig_list = orig_value;
    //first, search to see if any placeholders
    bool found_one = false;
    for (orig_list.Start(); !orig_list.Last(); orig_list.Next()) {
      val = orig_list.Get();
      if (val.type == Am_OBJECT &&
          Am_Object(val).Is_Instance_Of(Am_A_Placeholder)) {
        found_one = true;
        break;
      }
    }
    if (!found_one)
      return true;
    else {
      Am_Value_List new_list;
      for (orig_list.Start(); !orig_list.Last(); orig_list.Next()) {
        val = orig_list.Get();
        if (val.type == Am_OBJECT &&
            Am_Object(val).Is_Instance_Of(Am_A_Placeholder)) {
          ph = val;
          Am_Placeholder_Replace_Method replace_method =
              ph.Get(Am_PLACEHOLDER_REPLACE_METHOD);
          ok = replace_method.Call(ph, ret_val, sel_widget);
          if (!ok)
            return false;
          if (ret_val.Valid()) { //otherwise, leave new_list without ret_val
            if (Am_Value_List::Test(ret_val))
              new_list.Append(ret_val);
            else
              new_list.Add(ret_val);
          }
        } else
          new_list.Add(val);
      }
      did_a_replace = true;
      new_value = new_list;
      if (am_sdebug)
        std::cout << "Found placeholders in list and replaced with "
                  << new_value << std::endl
                  << std::flush;
    }
  } else { //not a list
    if (orig_value.type == Am_OBJECT &&
        Am_Object(orig_value).Is_Instance_Of(Am_A_Placeholder)) {
      ph = orig_value;
      Am_Placeholder_Replace_Method replace_method =
          ph.Get(Am_PLACEHOLDER_REPLACE_METHOD);
      ok = replace_method.Call(ph, new_value, sel_widget);
      if (Am_Value_List::Test(new_value) &&
          Am_Value_List(new_value).Length() == 1) {
        Am_Value_List l = new_value;
        l.Start();
        new_value = l.Get();
      }
      if (!ok)
        return false;
      did_a_replace = true;
      if (am_sdebug)
        std::cout << "Found one placeholder and replaced with " << new_value
                  << std::endl
                  << std::flush;
    }
  }
  return true;
}

// Check for replacing the parameters of the created object
bool
Am_Replace_Create_Placeholders(Am_Object &cmd, bool &replaced_create,
                               Am_Object &sel_widget,
                               Am_Value &new_start_object,
                               Am_Value &new_loc_value)
{
  replaced_create = false;
  Am_Value_List slots_to_save =
      cmd.Get(Am_SLOTS_TO_SAVE, Am_RETURN_ZERO_ON_ERROR);
  if (slots_to_save.Valid()) {
    Am_Value new_value, orig_value;
    Am_Slot_Key slot;
    bool did_a_replace;
    Am_Object start_object;
    for (slots_to_save.Start(); !slots_to_save.Last(); slots_to_save.Next()) {
      slot = (Am_Slot_Key)(int)slots_to_save.Get();
      if (!Am_Replace_Placeholders(cmd, slot, did_a_replace, orig_value,
                                   new_value, sel_widget))
        return false;
      if (did_a_replace) {
        if (slot == Am_LOCATION_PLACEHOLDER)
          new_loc_value = new_value;
        else {
          if (!start_object.Valid()) {
            start_object = cmd.Get(Am_START_OBJECT);
            if (!start_object.Valid())
              Am_ERRORO("Generalizing Create command "
                            << cmd << " but no start object",
                        cmd, Am_START_OBJECT);
            start_object = start_object.Copy();
            new_start_object = start_object;
            if (am_sdebug)
              std::cout << "Created new start object " << new_start_object
                        << std::endl
                        << std::flush;
          }
          start_object.Set(slot, new_value, Am_OK_IF_NOT_THERE);
          if (am_sdebug)
            std::cout << "Setting slot " << Am_Am_Slot_Key(slot) << " of "
                      << start_object << " to be " << new_value << std::endl
                      << std::flush;
        }
        replaced_create = true;
      }
    }
  }
  return true;
}

bool
get_value_from_chain(Am_Value &start_value, Am_Value_List &part_chain,
                     Am_Value &new_value)
{
  if (am_sdebug)
    std::cout << "Searching using chain " << part_chain << " from "
              << start_value << std::endl
              << std::flush;
  Am_Value this_value = start_value;
  Am_Value v;
  int key;
  Am_Value_List list;
  Am_Object obj;
  for (part_chain.Start(); !part_chain.Last(); part_chain.Next()) {
    key = part_chain.Get();
    if (key < 0) { // then is slot name
      if (this_value.type != Am_OBJECT) {
        AM_POP_UP_ERROR_WINDOW("Could not get slot " << -key << " out of value "
                                                     << this_value);
        return false;
      }
      obj = this_value;
      v = obj.Peek(-key);
      if (!v.Exists()) {
        AM_POP_UP_ERROR_WINDOW("Could not find value referenced as "
                               << part_chain << " from " << new_value
                               << " using slot " << -key << " of " << obj);
        return false;
      }
      this_value = v;
    } else { //key is an index
      if (this_value.type == Am_OBJECT) {
        obj = this_value;
        list = obj.Get(Am_GRAPHICAL_PARTS);
      } else if (Am_Value_List::Test(this_value)) {
        list = this_value;
      } else {
        AM_POP_UP_ERROR_WINDOW("Could not get index " << key << " out of value "
                                                      << this_value);
        return false;
      }
      if (key >= list.Length()) {
        AM_POP_UP_ERROR_WINDOW("Could not find value referenced as "
                               << part_chain << " from " << new_value
                               << " using index " << key << " in list "
                               << list);
        return false;
      }
      this_value = list.Get_Nth(key);
    }
  } //end loop
  if (am_sdebug)
    std::cout << " part chain found value " << this_value << std::endl
              << std::flush;
  new_value = this_value;
  return true;
}

Am_Define_Method(Am_Placeholder_Set_Value_Method, bool,
                 placeholder_value_from_slot,
                 (Am_Object & ph, Am_Object &new_cmd,
                  Am_Object & /* sel_widget */))
{
  Am_Slot_Key slot = (Am_Slot_Key)(int)ph.Get(Am_SLOT_FOR_VALUE);
  Am_Value v = new_cmd.Peek(slot);
  if (!v.Exists()) {
    AM_POP_UP_ERROR_WINDOW("Can't get value from slot "
                           << Am_Get_Slot_Name(slot) << " of command "
                           << new_cmd.Get(Am_LABEL));
    return false;
  }
  Am_Value_List part_chain = ph.Get(Am_PART_CHAIN, Am_RETURN_ZERO_ON_ERROR);
  if (part_chain.Valid()) {
    Am_Value new_value;
    bool ok = get_value_from_chain(v, part_chain, new_value);
    if (!ok)
      return false;
    else
      v = new_value;
  }
  if (am_sdebug)
    std::cout << "for placeholder " << ph << " setting value to " << v
              << " from slot " << Am_Am_Slot_Key(slot) << " of " << new_cmd
              << std::endl
              << std::flush;
  ph.Set(Am_VALUE, v);
  return true;
}

// look for any placeholders in cmd and if any, set their values from new_cmd
// return true if all are found
bool
find_objects_for_placeholders(Am_Slot_Key slot_for_placeholders, Am_Object &cmd,
                              Am_Object &new_cmd, Am_Object &sel_widget)
{
  Am_Value_List all_ph =
      cmd.Get(slot_for_placeholders, Am_RETURN_ZERO_ON_ERROR);
  Am_Object ph;
  bool ok;
  for (all_ph.Start(); !all_ph.Last(); all_ph.Next()) {
    ph = all_ph.Get();
    Am_Placeholder_Set_Value_Method method =
        ph.Get(Am_PLACEHOLDER_SET_VALUE_METHOD);
    //if no method, just ignore this placeholder
    if (method.Valid()) {
      ok = method.Call(ph, new_cmd, sel_widget);
      if (!ok)
        return false;
    }
  }
  return true;
}

Am_String
am_gen_command_label(Am_Object &for_cmd)
{
  char line[300];
  OSTRSTREAM_CONSTR(oss, line, 300, std::ios::out);
  reset_ostrstream(oss); // go to the beginning
  Am_Value v = for_cmd.Peek(Am_SHORT_LABEL);
  if (!v.Valid())
    v = for_cmd.Peek(Am_LABEL);
  Am_Value index = for_cmd.Peek(Am_COMMAND_INDEX_IN_SCRIPT);
  if (index.Valid()) {
    oss << index << ". ";
  }
  oss << v << std::ends;
  OSTRSTREAM_COPY(oss, line, 300);
  Am_String s = line;
  return s;
}

//returns true if everything is OK, and then sets new_cmd
bool
Am_Handle_Placeholders_And_Call_Command(
    int cnt, Am_Object &cmd, Am_Selective_Allowed_Method &same_allowed_method,
    Am_Handler_Selective_Undo_Method &same_method,
    Am_Selective_New_Allowed_Method &new_allowed_method,
    Am_Handler_Selective_Repeat_New_Method &new_method, Am_Object &sel_widget,
    Am_Object &undo_handler, Am_Object &new_cmd)
{
  char line[300];

  Am_Beep_Happened = false; //global in opal_advanced.h

  OSTRSTREAM_CONSTR(oss, line, 300, std::ios::out);
  reset_ostrstream(oss); // go to the beginning
  am_write_command_name(cmd, cnt, oss);
  OSTRSTREAM_COPY(oss, line, 300);
  if (am_sdebug)
    std::cout << "Doing cmd: " << line << " || " << cmd << std::endl
              << std::flush;

  bool replaced_obj_mod = false;
  bool replaced_create = false;
  bool replaced_value = false;
  Am_Value orig_obj_modified, orig_value, new_obj_mod, new_value;
  bool ok;

  if (!cmd.Is_Instance_Of(Am_Object_Create_Command)) {
    ok = Am_Replace_Placeholders(cmd, Am_OBJECT_MODIFIED, replaced_obj_mod,
                                 orig_obj_modified, new_obj_mod, sel_widget);
    if (!ok)
      return false;
  }
  if (cmd.Is_Instance_Of(Am_Object_Create_Command) ||
      cmd.Is_Instance_Of(Am_Find_Command)) {
    ok = Am_Replace_Create_Placeholders(cmd, replaced_create, sel_widget,
                                        new_obj_mod, new_value);
    if (!ok) {
      return false;
    }
  } else {
    ok = Am_Replace_Placeholders(cmd, Am_OBJECT_MODIFIED, replaced_obj_mod,
                                 orig_obj_modified, new_obj_mod, sel_widget);
    if (!ok)
      return false;
    ok = Am_Replace_Placeholders(cmd, Am_VALUE, replaced_value, orig_value,
                                 new_value, sel_widget);
    if (!ok) {
      return false;
    }
  }

  if (am_sdebug)
    std::cout << "%% Going to call method on cmd " << cmd << std::endl
              << std::flush;

  bool selectdir = cmd.Peek(Am_CURRENT_DIRECTION).Valid(); //HACK

  if (replaced_obj_mod || replaced_value || replaced_create || selectdir) {
    if (!new_allowed_method.Call(cmd, new_obj_mod, new_value)) {
      AM_POP_UP_ERROR_WINDOW("Can't execute command "
                             << cnt << ". " << cmd.Get(Am_LABEL)
                             << " on NEW objects  " << new_obj_mod
                             << " with new value " << new_value
                             << "; command object " << cmd);
      return false;
    }
    if (am_sdebug)
      std::cout << "%% " << new_method << " on " << cmd << " new_obj "
                << new_obj_mod << " val " << new_value << std::endl
                << std::flush;
    new_cmd = new_method.Call(undo_handler, cmd, new_obj_mod, new_value);
  } else { // not on new objects
    if (!same_allowed_method.Call(cmd)) {
      AM_POP_UP_ERROR_WINDOW("Can't execute command "
                             << cnt << ". " << cmd.Get(Am_LABEL)
                             << " on SAME; command object " << cmd);
      return false;
    }
    new_cmd = same_method.Call(undo_handler, cmd);
  }

  if (!find_objects_for_placeholders(Am_PLACEHOLDERS_SET_BY_THIS_COMMAND, cmd,
                                     new_cmd, sel_widget)) {
    return false;
  }
  if (Am_Beep_Happened) {
    Am_String s = am_gen_command_label(cmd);
    AM_POP_UP_ERROR_WINDOW("Quitting due to Beep from command "
                           << s << "; command object " << cmd);
    return false;
  }
  return true;
}

/////////////////////////// creating placeholders /////////////////////

//creates a placeholder
Am_Object
general_placeholder(Am_Object &new_cmd, const Am_Value &for_value,
                    const char *prompt1, const char *prompt2, bool use_cmd_name,
                    Am_Object &proto, Am_Value_List &part_chain, int &cnt,
                    Am_Slot_Key slot)
{
  char line[300];
  OSTRSTREAM_CONSTR(oss, line, 300, std::ios::out);
  reset_ostrstream(oss); // go to the beginning
  char placeholder_char = 'A' + cnt++;
  if (prompt1)
    oss << prompt1;
  oss << placeholder_char;
  if (prompt2)
    oss << "_" << prompt2;
  if (use_cmd_name)
    oss << new_cmd.Get(Am_LABEL) << "_command";
  oss << std::ends;
  OSTRSTREAM_COPY(oss, line, 300);
  Am_Object placeholder =
      proto.Create(line)
          .Set(Am_VALUE, for_value)
          .Set(Am_OLD_VALUE, for_value,
               Am_OK_IF_NOT_THERE) //save original value
          .Set(Am_ID, placeholder_char)
          .Set(Am_LABEL, line)
          .Set(Am_COMMAND, new_cmd) // command that created this object
          .Set(Am_PART_CHAIN, part_chain)
          .Set(Am_SLOTS_TO_SAVE, slot);
  if (am_sdebug)
    std::cout << "created placeholder `" << placeholder << "' for cmd "
              << new_cmd << std::endl
              << std::flush;
  if (new_cmd.Valid()) {
    Am_Value_List old_ph_list = new_cmd.Get(Am_PLACEHOLDERS_SET_BY_THIS_COMMAND,
                                            Am_RETURN_ZERO_ON_ERROR);
    old_ph_list.Add(placeholder);
    new_cmd.Set(Am_PLACEHOLDERS_SET_BY_THIS_COMMAND, old_ph_list,
                Am_OK_IF_NOT_THERE);
    Am_Value_List old_slot_list =
        new_cmd.Get(Am_SLOTS_TO_SAVE, Am_RETURN_ZERO_ON_ERROR);
    old_slot_list.Append_New(slot);
    new_cmd.Set(Am_SLOTS_TO_SAVE, old_slot_list, Am_OK_IF_NOT_THERE);
  }
  return placeholder;
}

Am_Define_Method(Am_Placeholder_Create_Method, Am_Object,
                 all_values_placeholder_create,
                 (Am_Object & create_cmd, Am_Value for_value,
                  Am_Value_List &part_chain, int &cnt, Am_Slot_Key slot))
{
  if (part_chain.Valid())
    Am_ERROR("Shouldn't be a part_chain " << part_chain << " for all objects");
  Am_Object placeholder = general_placeholder(
      create_cmd, for_value, "All_Values_", "Returned_by_", true,
      Am_All_Values_From_Command_Placeholder, part_chain, cnt, slot);
  placeholder.Set(Am_SLOT_FOR_VALUE, Am_VALUE);
  return placeholder;
}

Am_Define_Method(Am_Placeholder_Create_Method, Am_Object,
                 an_object_placeholder_create,
                 (Am_Object & create_cmd, Am_Value for_value,
                  Am_Value_List &part_chain, int &cnt, Am_Slot_Key slot))
{
  const char *s;
  if (part_chain.Valid())
    s = "Part_of_the_objects_";
  else
    s = "The_Object_";
  Am_Object placeholder = general_placeholder(
      create_cmd, for_value, s, "Created_by_", true,
      Am_An_Object_From_Command_Placeholder, part_chain, cnt, slot);
  placeholder.Set(Am_SLOT_FOR_VALUE, Am_VALUE);
  return placeholder;
}

Am_Object
am_call_create_placeholder(Am_Object proto, Am_Value &objs,
                           Am_Object &create_cmd,
                           Am_Value_List &placeholder_list,
                           Am_Value_List &part_chain, int &cnt,
                           Am_Slot_Key slot)
{
  Am_Placeholder_Create_Method method = proto.Get(Am_PLACEHOLDER_CREATE_METHOD);
  Am_Object placeholder = method.Call(create_cmd, objs, part_chain, cnt, slot);
  placeholder_list.Add(Am_Assoc(objs, placeholder));
  return placeholder;
}

/////////////////////////////// Ask User To Select Objects /////////////////

Am_Define_Method(Am_Placeholder_Create_Method, Am_Object,
                 user_select_placeholder_create,
                 (Am_Object & create_cmd, Am_Value for_value,
                  Am_Value_List & /*part_chain*/, int &cnt, Am_Slot_Key slot))
{
  Am_Object placeholder = general_placeholder(
      create_cmd, for_value, "Ask_User_To_Select_Objects_", (0L), false,
      Am_User_Select_At_Run_Time_Placeholder, Am_No_Value_List, cnt, slot);
  return placeholder;
}

Am_Define_Method(Am_Placeholder_Replace_Method, bool,
                 user_select_objs_replace_placeholder,
                 (Am_Object & ph, Am_Value &new_value, Am_Object &sel_widget))
{
  if (ph.Get(Am_PLACEHOLDER_INITIALIZED).Valid()) {
    new_value = ph.Get(Am_VALUE);
    return true;
  } else {
    Am_Value ok;
    ph.Set(Am_PLACEHOLDER_INITIALIZED, true);
    Am_Pop_Up_Window_And_Wait(am_askselectobj, ok, false);
    if (ok.Valid()) {
      new_value = sel_widget.Get(Am_VALUE);
      if (am_sdebug)
        std::cout << "for placeholder " << ph << " setting value to "
                  << new_value << std::endl
                  << std::flush;
      ph.Set(Am_VALUE, new_value);
      return true;
    } else
      return false;
  }
}

/////////////////////////////// Objects that are already selected ////////////

Am_Define_Method(Am_Placeholder_Create_Method, Am_Object,
                 selected_objs_placeholder_create,
                 (Am_Object & create_cmd, Am_Value for_value,
                  Am_Value_List & /* part_chain */, int &cnt, Am_Slot_Key slot))
{
  Am_Object placeholder = general_placeholder(
      create_cmd, for_value, "The_Current_Selected_Objects_", (0L), false,
      Am_The_Selected_Objects_Placeholder, Am_No_Value_List, cnt, slot);
  return placeholder;
}

//return the selected set of objects
Am_Define_Method(Am_Placeholder_Replace_Method, bool,
                 selected_objs_replace_placeholder,
                 (Am_Object & ph, Am_Value &new_value, Am_Object &sel_widget))
{
  if (ph.Peek(Am_PLACEHOLDER_INITIALIZED).Valid()) {
    new_value = ph.Get(Am_VALUE);
    return true;
  } else {
    ph.Set(Am_PLACEHOLDER_INITIALIZED, true);
    new_value = sel_widget.Get(Am_VALUE);
    if (am_sdebug)
      std::cout << "for placeholder " << ph << " setting value to " << new_value
                << std::endl
                << std::flush;
    ph.Set(Am_VALUE, new_value);
    return true;
  }
}
/////////////////////////////// Am_Constant_Placeholder /////////////////

Am_Define_Method(Am_Placeholder_Create_Method, Am_Object,
                 constant_placeholder_create,
                 (Am_Object & create_cmd, Am_Value for_value,
                  Am_Value_List & /* part_chain */, int &cnt, Am_Slot_Key slot))
{
  char line[300];
  OSTRSTREAM_CONSTR(oss, line, 300, std::ios::out);
  reset_ostrstream(oss); // go to the beginning
  oss << for_value << std::ends;
  OSTRSTREAM_COPY(oss, line, 300);

  Am_Object placeholder =
      general_placeholder(create_cmd, for_value, "The_Constant_", line, false,
                          Am_Constant_Placeholder, Am_No_Value_List, cnt, slot);
  return placeholder;
}

Am_Define_Method(Am_Placeholder_Replace_Method, bool,
                 constant_do_match_placeholder,
                 (Am_Object & ph, Am_Value &new_value,
                  Am_Object & /* sel_widget */))
{
  Am_Value old_value = ph.Get(Am_VALUE);
  if (new_value == old_value)
    return true;
  else
    return false;
}

/////////////////////////////// Location Placeholders /////////////////

Am_Define_Method(Am_Placeholder_Create_Method, Am_Object,
                 location_placeholder_create,
                 (Am_Object & create_cmd, Am_Value for_value,
                  Am_Value_List &part_chain, int &cnt, Am_Slot_Key slot))
{
  Am_Object placeholder = general_placeholder(
      create_cmd, for_value, "Computed_Location_", (0L), false,
      Am_Inter_Location_Placeholder, part_chain, cnt, slot);
  return placeholder;
}

Am_Define_Method(Am_Create_New_Object_Method, Am_Object,
                 done_location_ask_inter,
                 (Am_Object inter, Am_Inter_Location data,
                  Am_Object /* old_object */))
{
  Am_Object db = inter.Get(Am_LOCATION_PLACEHOLDER);
  db.Set(Am_VALUE, data);
  Am_Finish_Pop_Up_Waiting(db, Am_Value(data));
  return Am_No_Object;
}

bool
pop_up_ask_location(Am_Object &sel_widget, bool growing, Am_Object &ref_obj,
                    int &ask_left, int &ask_top, int &ask_width,
                    int &ask_height, int old_width, int old_height)
{
  if (!ref_obj.Valid()) {
    Am_ERROR("No ref_object for pop_up_ask_location");
  }
  Am_Object inter =
      ref_obj.Get(Am_LOCATION_PLACEHOLDER, Am_RETURN_ZERO_ON_ERROR);
  const char *msg;
  if (!inter.Valid()) {
    Am_Object feedback = sel_widget.Get_Object(Am_RECT_FEEDBACK_OBJECT).Copy();
    inter = Am_New_Points_Interactor.Create()
                .Set(Am_FEEDBACK_OBJECT, feedback)
                .Set(Am_PRIORITY, 110)
                .Set(Am_CREATE_NEW_OBJECT_METHOD, done_location_ask_inter)
                .Get_Object(Am_COMMAND)
                .Set(Am_IMPLEMENTATION_PARENT, Am_NOT_USUALLY_UNDONE)
                .Get_Owner();
    ref_obj.Add_Part(Am_LOCATION_PLACEHOLDER, inter).Add_Part(feedback);
  }
  inter.Set(Am_ACTIVE, true)
      .Set(Am_LOCATION_PLACEHOLDER, am_askclickpos, Am_OK_IF_NOT_THERE);
  if (growing) {
    inter.Set(Am_HOW_MANY_POINTS, 2)
        .Set(Am_MINIMUM_WIDTH, 0)
        .Set(Am_MINIMUM_HEIGHT, 0);
    msg = "Press and drag in the window to define a new position and size";
  } else {
    inter.Set(Am_HOW_MANY_POINTS, 1)
        .Set(Am_MINIMUM_WIDTH, old_width)
        .Set(Am_MINIMUM_HEIGHT, old_height);
    msg = "Click in the window to define a new position";
  }
  am_askclickpos.Set(Am_VALUE, (0L));
  am_askclickpos.Get_Object(Am_LABEL).Set(Am_TEXT, msg);
  Am_Value ok;

  Am_Pop_Up_Window_And_Wait(am_askclickpos, ok, false);
  inter.Set(Am_ACTIVE, false);
  if (ok.Valid()) {
    Am_Inter_Location new_loc = am_askclickpos.Get(Am_VALUE);
    if (am_sdebug)
      std::cout << "pop up returned the value " << new_loc << std::endl
                << std::flush;
    Am_Object loc_ref_obj;
    bool as_line;
    new_loc.Translate_To(ref_obj);
    new_loc.Get_Location(as_line, loc_ref_obj, ask_left, ask_top, ask_width,
                         ask_height);
    return true;
  } else
    return false;
}

bool
pop_up_ask_type_location(int old_left, int old_top, int old_width,
                         int old_height, int left_control, int top_control,
                         int width_control, int height_control, int &ask_left,
                         int &ask_top, int &ask_width, int &ask_height)
{
  if (am_sdebug)
    std::cout << "**** Ask user to type location " << std::endl << std::flush;
  //** if value typed is a list, then just use the next value next time **//
  if (left_control == am_ask_user_type) {
    std::cout << "Left (old=" << old_left << "): " << std::flush;
    std::cin >> ask_left;
  }
  if (top_control == am_ask_user_type) {
    std::cout << "Top (old=" << old_top << "): " << std::flush;
    std::cin >> ask_top;
  }
  if (width_control == am_ask_user_type) {
    std::cout << "Width (old=" << old_width << "): " << std::flush;
    std::cin >> ask_width;
  }
  if (height_control == am_ask_user_type) {
    std::cout << "Height (old=" << old_height << "): " << std::flush;
    std::cin >> ask_height;
  }

  return true;
}

bool
get_location_from_object(Am_Object & /*ph*/, Am_Value &other_object,
                         Am_Object & /* ref_obj */, int &obj_left, int &obj_top,
                         int &obj_width, int &obj_height, Am_Object &sel_widget)
{
  if (am_sdebug)
    std::cout << "Get location from other object " << other_object << std::endl
              << std::flush;
  Am_Value_List sel = sel_widget.Get(Am_VALUE);
  if (!sel.Valid())
    return false;
  sel.Start();
  Am_Object obj = sel.Get();
  // ****
  if (am_sdebug)
    std::cout << "TEMP using selected object " << obj << std::endl
              << std::flush;
  obj_left = obj.Get(Am_LEFT);
  obj_top = obj.Get(Am_TOP);
  obj_width = obj.Get(Am_WIDTH);
  obj_height = obj.Get(Am_HEIGHT);
  return true;
}

//return the selected set of objects
Am_Define_Method(Am_Placeholder_Replace_Method, bool,
                 location_replace_placeholder,
                 (Am_Object & ph, Am_Value &new_value, Am_Object &sel_widget))
{
  if (ph.Peek(Am_PLACEHOLDER_INITIALIZED).Valid()) {
    new_value = ph.Get(Am_VALUE);
    return true;
  } else {
    ph.Set(Am_PLACEHOLDER_INITIALIZED, true);
    Am_Inter_Location old_val = ph.Get(Am_VALUE);

    //controls are zero if not change that value
    int left_control = ph.Get(Am_LEFT_HOW);
    int left_amount = ph.Get(Am_LEFT_AMOUNT_WIDGET);
    int top_control = ph.Get(Am_TOP_HOW);
    int top_amount = ph.Get(Am_TOP_AMOUNT_WIDGET);
    int width_control = ph.Get(Am_WIDTH_HOW);
    int width_amount = ph.Get(Am_WIDTH_AMOUNT_WIDGET);
    int height_control = ph.Get(Am_HEIGHT_HOW);
    int height_amount = ph.Get(Am_HEIGHT_AMOUNT_WIDGET);
    bool growing = ((width_control != 0) || (height_control != 0));

    int left, top, width, height;
    int obj_left, obj_top, obj_width, obj_height;
    int ask_left, ask_top, ask_width, ask_height;
    Am_Object ref_obj;
    bool as_line;
    old_val.Get_Location(as_line, ref_obj, left, top, width, height);
    Am_Inter_Location pop_val, obj_val;

    if (am_sdebug)
      std::cout << "old_val " << left << ", " << top << ", " << width << ", "
                << height << " ref_obj " << ref_obj << std::endl
                << std::flush;

    Am_Value other_object =
        ph.Get(Am_CONSTANT_OBJECTS_WIDGET, Am_RETURN_ZERO_ON_ERROR);
    bool ok;
    if (other_object.Valid()) {
      ok = get_location_from_object(ph, other_object, ref_obj, obj_left,
                                    obj_top, obj_width, obj_height, sel_widget);
      if (!ok)
        return false;
      if (am_sdebug)
        std::cout << "Other object " << obj_left << ", " << obj_top << ", "
                  << obj_width << ", " << obj_height << std::endl
                  << std::flush;
    }
    if (left_control == am_ask_user_generalize ||
        top_control == am_ask_user_generalize ||
        width_control == am_ask_user_generalize ||
        height_control == am_ask_user_generalize) {
      ok = pop_up_ask_location(sel_widget, growing, ref_obj, ask_left, ask_top,
                               ask_width, ask_height, width, height);
      if (!ok)
        return false;
    }
    if (left_control == am_ask_user_type || top_control == am_ask_user_type ||
        width_control == am_ask_user_type ||
        height_control == am_ask_user_type) {
      ok = pop_up_ask_type_location(left, top, width, height, left_control,
                                    top_control, width_control, height_control,
                                    ask_left, ask_top, ask_width, ask_height);
      if (!ok)
        return false;
    }

    if (am_sdebug)
      std::cout << "Controls left = " << left_control << " top= " << top_control
                << " width= " << width_control << " height= " << height_control
                << std::endl
                << std::flush;
    if (am_sdebug)
      std::cout << "Amounts left = " << left_amount << " top= " << top_amount
                << " width= " << width_amount << " height= " << height_amount
                << std::endl
                << std::flush;

    Am_Inter_Location new_loc;

    switch (width_control) {
    case 0:
      break;
    case am_change_equals:
      width = width_amount;
      break;
    case am_change_offset:
      width = width_amount;
      new_loc.Set_C_Control(Am_LOCATION_RELATIVE);
      break;
    case am_ask_user_generalize:
    case am_ask_user_type:
      width = ask_width;
      break;
    case am_change_object_wh:
      width = obj_width + width_amount;
      break;
    case am_change_object_percent:
      width = (int)(((float)obj_width * width_amount) / 100.0);
      break;
    default:
      Am_ERROR("Bad type for width control " << width_control);
    } //end switch

    switch (height_control) {
    case 0:
      break;
    case am_change_equals:
      height = height_amount;
      break;
    case am_change_offset:
      height = height_amount;
      new_loc.Set_D_Control(Am_LOCATION_RELATIVE);
      break;
    case am_ask_user_generalize:
    case am_ask_user_type:
      height = ask_height;
      break;
    case am_change_object_wh:
      height = obj_height + height_amount;
      break;
    case am_change_object_percent:
      height = (int)(((float)obj_height * height_amount) / 100.0);
      break;
    default:
      Am_ERROR("Bad type for height control " << height_control);
    } //end switch

    switch (left_control) {
    case 0:
      break;
    case am_change_equals:
      left = left_amount;
      break;
    case am_change_offset:
      left = left_amount;
      new_loc.Set_A_Control(Am_LOCATION_RELATIVE);
      break;
    case am_ask_user_generalize:
    case am_ask_user_type:
      left = ask_left;
      break;
    case am_change_offset_left_outside:
      left = obj_left - width - left_amount;
      break;
    case am_change_offset_left_inside:
      left = obj_left + left_amount;
      break;
    case am_change_offset_centered:
      left = obj_left + (int)(((float)(-width) / 2.0) +
                              ((((float)obj_width) * left_amount) / 100.0));
      break;
    case am_change_offset_right_inside:
      left = obj_left + obj_width - width - left_amount;
      break;
    case am_change_offset_right_outside:
      left = obj_left + obj_width - 1 + left_amount;
      break;

    default:
      Am_ERROR("Bad type for left control " << left_control);
    } //end switch

    switch (top_control) {
    case 0:
      break;
    case am_change_equals:
      top = top_amount;
      break;
    case am_change_offset:
      top = top_amount;
      new_loc.Set_B_Control(Am_LOCATION_RELATIVE);
      break;
    case am_ask_user_generalize:
    case am_ask_user_type:
      top = ask_top;
      break;
    case am_change_offset_top_outside:
      top = obj_top - height - top_amount;
      break;
    case am_change_offset_top_inside:
      top = obj_top + top_amount;
      break;
    case am_change_offset_centered:
      top = obj_top + (int)(((float)(-height) / 2.0) +
                            ((((float)obj_height) * top_amount) / 100.0));
      break;
    case am_change_offset_bottom_inside:
      top = obj_top + obj_height - height - top_amount;
      break;
    case am_change_offset_bottom_outside:
      top = obj_top + obj_height - 1 + top_amount;
      break;
    default:
      Am_ERROR("Bad type for top control " << top_control);
    } //end switch

    if (am_sdebug)
      std::cout << "Computed values " << left << ", " << top << ", " << width
                << ", " << height << " growing " << growing << std::endl
                << std::flush;

    new_loc.Set_Location(false, ref_obj, left, top, width, height, false);
    new_loc.Set_Growing(growing, false);

    new_value = new_loc;
    if (am_sdebug)
      std::cout << "for placeholder " << ph << " setting value to " << new_value
                << std::endl
                << std::flush;
    ph.Set(Am_VALUE, new_value);
    return true;
  }
}

/////////////////////////////// Ask User For Value Placeholder ///////////////

Am_Define_Method(Am_Placeholder_Create_Method, Am_Object,
                 ask_user_value_placeholder_create,
                 (Am_Object & create_cmd, Am_Value for_value,
                  Am_Value_List & /*part_chain*/, int &cnt, Am_Slot_Key slot))
{
  Am_Object placeholder = general_placeholder(
      create_cmd, for_value, "Ask_User_For_Value_", (0L), false,
      Am_Ask_User_Property_Placeholder, Am_No_Value_List, cnt, slot);
  return placeholder;
}

Am_Define_Method(Am_Placeholder_Replace_Method, bool,
                 ask_user_value_replace_placeholder,
                 (Am_Object & ph, Am_Value &new_value,
                  Am_Object & /* sel_widget */))
{
  new_value = ph.Get(Am_VALUE);
  if (ph.Get(Am_PLACEHOLDER_INITIALIZED).Valid()) {
    return true;
  } else {
    Am_Value ok;
    ph.Set(Am_PLACEHOLDER_INITIALIZED, true);
    am_askforvalue.Get_Object(Am_CONSTANT_OBJECTS_WIDGET)
        .Set(Am_VALUES, new_value)
        .Set(Am_VALUE, new_value);
    Am_Pop_Up_Window_And_Wait(am_askforvalue, ok, true);
    if (ok.Valid()) {
      new_value =
          am_askforvalue.Get_Object(Am_CONSTANT_OBJECTS_WIDGET).Get(Am_VALUES);
      if (am_sdebug)
        std::cout << "for placeholder " << ph << " setting value to "
                  << new_value << std::endl
                  << std::flush;
      ph.Set(Am_VALUE, new_value);
      return true;
    } else
      return false;
  }
}

////////////////// Get Property from Palette Placeholder ///////////////

Am_Define_Method(Am_Placeholder_Create_Method, Am_Object,
                 property_from_palette_placeholder_create,
                 (Am_Object & create_cmd, Am_Value for_value,
                  Am_Value_List &part_chain, int &cnt, Am_Slot_Key slot))
{
  Am_Object palette = part_chain.Get_First();
  const char *s = palette.Get_Name();
  Am_Object placeholder = general_placeholder(
      create_cmd, for_value, "Get_Value_From_Palette_", s, false,
      Am_Property_From_Palette_Placeholder, Am_No_Value_List, cnt, slot);
  placeholder.Set(Am_REGISTRY_FOR_PALETTES, palette, Am_OK_IF_NOT_THERE);
  return placeholder;
}

Am_Define_Method(Am_Placeholder_Replace_Method, bool,
                 property_from_palette_replace_placeholder,
                 (Am_Object & ph, Am_Value &new_value,
                  Am_Object & /* sel_widget */))
{
  if (ph.Get(Am_PLACEHOLDER_INITIALIZED).Valid()) {
    new_value = ph.Get(Am_VALUE);
    return true;
  } else {
    ph.Set(Am_PLACEHOLDER_INITIALIZED, true);
    Am_Object palette = ph.Get(Am_REGISTRY_FOR_PALETTES);
    new_value = palette.Get(Am_VALUE);
    if (new_value.type == Am_OBJECT) {
      Am_Object palette_value_obj = new_value;
      new_value = palette_value_obj.Peek(Am_FILL_STYLE);
    }
    if (am_sdebug)
      std::cout << "Got value " << new_value << " from palette " << palette
                << " for " << ph << std::endl
                << std::flush;
    return true;
  }
}

////////////////// Get Property from List of Values ///////////////

Am_Define_Method(Am_Placeholder_Create_Method, Am_Object,
                 cycle_values_placeholder_create,
                 (Am_Object & create_cmd, Am_Value for_value,
                  Am_Value_List & /* part_chain */, int &cnt, Am_Slot_Key slot))
{
  Am_Object placeholder = general_placeholder(
      create_cmd, for_value, "Get_Value_From_List_", (0L), false,
      Am_Cycle_Values_Placeholder, Am_No_Value_List, cnt, slot);
  return placeholder;
}

Am_Define_Method(Am_Placeholder_Replace_Method, bool,
                 cycle_values_replace_placeholder,
                 (Am_Object & ph, Am_Value &new_value,
                  Am_Object & /* sel_widget */))
{
  // if (ph.Get(Am_PLACEHOLDER_INITIALIZED).Valid())
  //   ??should it start list each time initialized??
  ph.Set(Am_PLACEHOLDER_INITIALIZED, true);
  Am_Value_List values = ph.Get(Am_VALUE);
  int count = ph.Get(Am_CURRENT_PHASE);
  //  ************* Beep when done option !!  ****
  if (count >= values.Length() - 1)
    count = 0;
  else
    count++;
  ph.Set(Am_CURRENT_PHASE, count);
  new_value = values.Get_Nth(count);
  if (am_sdebug)
    std::cout << "Got value " << new_value << " as index " << count
              << " from list " << values << " for " << ph << std::endl
              << std::flush;
  return true;
}

////////////////// Am_Type_Match_Placeholder ///////////////

Am_Define_Method(Am_Placeholder_Create_Method, Am_Object,
                 type_match_placeholder_create,
                 (Am_Object & create_cmd, Am_Value for_value,
                  Am_Value_List &part_chain, int &cnt, Am_Slot_Key slot))
{
  part_chain.Start();
  Am_String type_name = part_chain.Get(); //hack
  part_chain.Next();
  Am_Object proto = part_chain.Get();
  Am_Object placeholder = general_placeholder(
      create_cmd, for_value, "Any_Object_Of_Type_", type_name, false,
      Am_Type_Match_Placeholder, Am_No_Value_List, cnt, slot);
  placeholder.Add(Am_VALUES, proto);
  return placeholder;
}

Am_Define_Method(Am_Placeholder_Replace_Method, bool, type_do_match_placeholder,
                 (Am_Object & ph, Am_Value &new_value,
                  Am_Object & /* sel_widget */))
{
  Am_Object proto = ph.Get(Am_VALUES);

  if (new_value.type != Am_OBJECT)
    return false;
  Am_Object new_object = new_value;
  if (new_object.Is_Instance_Of(proto))
    return true;
  else
    return false;
}

////////////////// Am_Any_Value_Placeholder ///////////////

Am_Define_Method(Am_Placeholder_Create_Method, Am_Object,
                 any_value_match_placeholder_create,
                 (Am_Object & create_cmd, Am_Value for_value,
                  Am_Value_List &part_chain, int &cnt, Am_Slot_Key slot))
{
  Am_String str = part_chain.Get_First(); //hack
  Am_Object placeholder = general_placeholder(create_cmd, for_value, str, (0L),
                                              false, Am_Any_Value_Placeholder,
                                              Am_No_Value_List, cnt, slot);
  return placeholder;
}

Am_Define_Method(Am_Placeholder_Replace_Method, bool,
                 any_value_do_match_placeholder,
                 (Am_Object & /* ph */, Am_Value & /* new_value */,
                  Am_Object & /* sel_widget */))
{
  return true;
}

/////////////////////// Setting up the placeholders ////////////////////

int
find_index_of_part(Am_Object &obj)
{
  Am_Object owner = obj.Get_Owner();
  Am_Value_List parts = owner.Get(Am_GRAPHICAL_PARTS);
  if (!parts.Start_Member(obj))
    Am_ERRORO("part " << obj << " not in graphical parts of its owner "
                      << owner,
              owner, Am_GRAPHICAL_PARTS);
  //current index
  return parts.Nth();
}

//returns true if part is a part (recursively) of owner.  If so, constructs a
//chain of slot names (or indexes if no part name) to
//get from owner down to part.  Since slot keys are ints, to distinguish, slot
//keys are made negative.  If index is >0 then it is used as the first item in
//the list.
bool
check_obj_or_owners(Am_Object &part, Am_Object &owner,
                    Am_Value_List &part_chain, int index)
{
  if (part == owner) {
    if (index >= 0)
      part_chain.Add(index);
    return true;
  }
  Am_Object obj = part.Get_Owner();
  while (true) {
    if (!obj.Valid())
      return false;
    if (owner == obj)
      break;
    obj = obj.Get_Owner();
  }
  //if get here, then is an owner
  obj = part;
  int key;
  while (true) {
    key = obj.Get_Key();
    if (key == Am_NO_SLOT || key == Am_NO_INHERIT)
      key = find_index_of_part(obj);
    else
      key = -key;
    part_chain.Add(key, Am_HEAD);
    obj = obj.Get_Owner();
    if (owner == obj)
      break;
  }
  if (index >= 0)
    part_chain.Add(index, Am_HEAD);
  if (am_sdebug)
    std::cout << "Part " << part << " owner " << owner << " chain "
              << part_chain << std::endl
              << std::flush;
  return true;
}

bool
find_in_assoc_list_member_or_owner(Am_Object &obj, Am_Value_List &list,
                                   Am_Object &value_2,
                                   Am_Value_List &part_chain)
{
  Am_Assoc as;
  Am_Object orig_placeholder, cmd, found_obj;
  Am_Value fo;
  Am_Value v;
  bool found = false;
  for (list.Start(); !list.Last(); list.Next()) {
    as = list.Get();
    v = as.Value_1();
    if (v.type == Am_OBJECT) {
      found_obj = v;
      if (check_obj_or_owners(obj, found_obj, part_chain, -1))
        found = true;
    } else if (Am_Value_List::Test(v)) {
      Am_Value_List sub_list = v;
      int index = 0;
      for (sub_list.Start(); !sub_list.Last(); sub_list.Next(), index++) {
        found_obj = sub_list.Get();
        if (check_obj_or_owners(obj, found_obj, part_chain, index)) {
          found = true;
          break;
        }
      }
    }
    if (found) {
      value_2 = as.Value_2();
      return true;
    }
  }
  return false;
}

Am_Object
check_assoc_2_or_part(Am_Object &obj, Am_Value_List &placeholder_list,
                      Am_Value_List &created_objects_assoc, int &cnt,
                      Am_Slot_Key slot)
{
  if (am_sdebug)
    std::cout << "looking for " << obj << " in placeholder_list "
              << placeholder_list << " or created_assoc "
              << created_objects_assoc << std::endl
              << std::flush;
  //first check if obj is already in the list
  Am_Object ph = placeholder_list.Assoc_2(obj);
  Am_Object create_cmd;
  if (ph.Valid())
    return ph;
  Am_Value obj_val;
  obj_val = obj;
  Am_Value_List part_chain;
  if (find_in_assoc_list_member_or_owner(obj, placeholder_list, ph,
                                         part_chain)) {
    Am_Value_List new_part_chain =
        ph.Get(Am_PART_CHAIN, Am_RETURN_ZERO_ON_ERROR);
    if (am_sdebug)
      std::cout << "found as part of placeholder " << ph << " existing chain "
                << new_part_chain << " new chain " << part_chain << std::endl
                << std::flush;

    if (new_part_chain.Valid())
      new_part_chain.Append(part_chain);
    else
      new_part_chain = part_chain;
    create_cmd = ph.Get(Am_COMMAND);
    ph = am_call_create_placeholder(Am_An_Object_From_Command_Placeholder,
                                    obj_val, create_cmd, placeholder_list,
                                    part_chain, cnt, slot);
    if (am_sdebug)
      std::cout << "created placeholder " << ph << std::endl << std::flush;
    return ph;
  } else if (find_in_assoc_list_member_or_owner(obj, created_objects_assoc,
                                                create_cmd, part_chain)) {
    if (am_sdebug)
      std::cout << "found as part of list created by " << create_cmd
                << " part chain " << part_chain << std::endl
                << std::flush;
    ph = am_call_create_placeholder(Am_An_Object_From_Command_Placeholder,
                                    obj_val, create_cmd, placeholder_list,
                                    part_chain, cnt, slot);
    if (am_sdebug)
      std::cout << "created placeholder " << ph << std::endl << std::flush;
    return ph;
  }
  return Am_No_Object;
}

//returns true if modifies something
bool
check_one_obj_modified(Am_Value &obj_modified, Am_Slot_Key slot,
                       Am_Object &in_cmd, Am_Value_List &placeholder_list,
                       Am_Value_List &created_objects_assoc, int &cnt)
{
  Am_Object placeholder, obj, create_cmd;
  Am_Value v, ret;
  Am_Value_List objs;
  if (!obj_modified.Valid())
    return false;
  if (Am_Value_List::Test(obj_modified))
    objs = obj_modified;
  else
    objs.Add(obj_modified);

  //first check the whole list
  placeholder = placeholder_list.Assoc_2(objs);
  if (placeholder.Valid()) {
    //then replace with placeholder
    if (am_sdebug)
      std::cout << "found whole group placeholder for " << obj_modified
                << " is " << placeholder << std::endl
                << std::flush;
    ret = placeholder;
  } else {
    //check whole list of objects
    if (am_sdebug)
      std::cout << "checking whole list " << objs << " in assoc "
                << created_objects_assoc << std::endl
                << std::flush;
    create_cmd = created_objects_assoc.Assoc_2(objs);
    if (create_cmd.Valid()) {
      v = objs;
      if (am_sdebug)
        std::cout << " checking placeholder_list " << placeholder_list
                  << std::endl
                  << std::flush;
      placeholder = am_call_create_placeholder(
          Am_All_Values_From_Command_Placeholder, v, create_cmd,
          placeholder_list, Am_No_Value_List, cnt, slot);
      ret = placeholder;
    } else { //check inside the list
      //very inefficient, copies list every time!
      bool found_one = false;
      Am_Value_List placeholders;
      Am_Value_List copy_of_obj_modified;
      for (objs.Start(); !objs.Last(); objs.Next()) {
        obj = objs.Get();
        placeholder = check_assoc_2_or_part(obj, placeholder_list,
                                            created_objects_assoc, cnt, slot);
        if (placeholder.Valid()) {

          found_one = true;
          //then replace with placeholder
          if (am_sdebug)
            std::cout << "in list found placeholder for " << obj << " is "
                      << placeholder << std::endl
                      << std::flush;
          copy_of_obj_modified.Add(placeholder);
        } else
          copy_of_obj_modified.Add(obj);
      }
      if (found_one)
        ret = copy_of_obj_modified;
    }
  }

  if (ret.Valid()) {
    if (Am_Value_List::Test(obj_modified)) {
      if (!Am_Value_List::Test(ret))
        ret = Am_Value_List().Add(ret);
    } else { // original is not a list, see if unneccesarily have a list
      if (Am_Value_List::Test(ret)) {
        Am_Value_List ret_list = ret;
        if (ret_list.Length() == 1)
          ret = ret_list.Get_First();
      }
    }
    if (am_sdebug)
      std::cout << " setting " << slot << " of " << in_cmd << " to be " << ret
                << std::endl
                << std::flush;
    in_cmd.Set(slot, ret, Am_OK_IF_NOT_THERE);
    return true;
  } else { //didn't find any placeholders
    /* Not a good idea *
    if (Am_Value_List::Test(obj_modified)) {
      Am_Object new_ph = am_call_create_placeholder(
		    Am_The_Selected_Objects_Placeholder,
		    obj_modified, in_cmd, placeholder_list,
		    Am_No_Value_List, cnt, slot);
		    if (am_sdebug)
		   std::cout << "New placeholder is " << new_ph <<std::endl <<std::flush;
      in_cmd.Set(slot, new_ph, Am_OK_IF_NOT_THERE);
      return true;
    }
    * */
  }
  return false;
}

//checks if the obj_modified are in the placeholder_list or
//created_objects_assoc.  If in placeholder_list, then replaces obj_modified
//slot with the placeholder.  If in created_objects_assoc, then creates a new
//placeholder for the objects.  If neither, but is a list of objects, then
//replaces with the selected_objects placeholder
void
am_check_obj_modified(Am_Object &in_cmd, Am_Value_List &placeholder_list,
                      Am_Value_List &created_objects_assoc, int &cnt)
{
  Am_Value obj_modified = in_cmd.Get(Am_OBJECT_MODIFIED);
  check_one_obj_modified(obj_modified, Am_OBJECT_MODIFIED, in_cmd,
                         placeholder_list, created_objects_assoc, cnt);
}
void
am_check_start_obj_parameters(Am_Object &in_cmd,
                              Am_Value_List &placeholder_list,
                              Am_Value_List &created_objects_assoc, int &cnt)
{
  Am_Object start_object = in_cmd.Get(Am_START_OBJECT);
  if (start_object.Valid()) {
    Am_Value_List slots_to_save = start_object.Get(Am_SLOTS_TO_SAVE);
    Am_Value_List slots_modified;
    Am_Slot_Key slot;
    Am_Value v;
    for (slots_to_save.Start(); !slots_to_save.Last(); slots_to_save.Next()) {
      slot = (Am_Am_Slot_Key)slots_to_save.Get();
      v = start_object.Peek(slot);
      // **** should probably also check for lists of objects
      if (v.Valid() && v.type == Am_OBJECT) {
        if (am_sdebug)
          std::cout << "checking slot " << slot << " oldval " << v << std::endl
                    << std::flush;
        if (check_one_obj_modified(v, slot, in_cmd, placeholder_list,
                                   created_objects_assoc, cnt))
          slots_modified.Add(slot);
      }
    }
    if (slots_modified.Valid()) {
      in_cmd.Set(Am_SLOTS_TO_SAVE, slots_modified, Am_OK_IF_NOT_THERE);
    }
  }
}

/////////////////////////////////////////////////////////////////////////
// Generalizers for selected
/////////////////////////////////////////////////////////////////////////

Am_Value_List objgen_windows;
int num_vis_objgen_windows = 0;

Am_Object
get_objgen_window()
{
  Am_Object w =
      am_get_db_from_list(am_objgen, objgen_windows, num_vis_objgen_windows);
  w.Get_Object(Am_CONSTANT_OBJECTS_WIDGET).Set(Am_VALUE, "");
  w.Get_Object(Am_INDEXES_OF_COMMANDS_WIDGET).Set(Am_VALUE, "");
  w.Get_Object(Am_SOME_OBJECTS_WIDGET).Set(Am_VALUE, "");
  w.Get_Object(Am_ALL_FROM_COMMAND_WIDGET).Set(Am_VALUE, "");

  return w;
}
void
done_with_objgen_window(Am_Object sw)
{
  sw.Set(Am_VISIBLE, false);
  objgen_windows.Add(sw);
  num_vis_objgen_windows--;
}

//finds target_cmd in sel_list, and if found, returns true and sets
//found_sel to the item of sel_list
bool
am_find_cmd_in_sel(Am_Value_List &sel_list, Am_Object &target_cmd,
                   Am_Object &found_sel)
{
  Am_Object this_sel;
  for (sel_list.Start(); !sel_list.Last(); sel_list.Next()) {
    this_sel = sel_list.Get();
    if (this_sel.Get(Am_COMMAND) == target_cmd) {
      found_sel = this_sel;
      return true;
    }
  }
  return false;
}

//incoming sel_list are the parts of the line, outgoing are the same
//objects, but sorted in the order they are in the cmd list.  If
//first_only, then just the first element of the sel_list is returned.
Am_Value_List
sort_sel_list_cmd_order(Am_Value_List &sel_list, Am_Object &execute_command,
                        bool modify_first_only)
{
  Am_Value_List new_list, line_list;
  Am_Object line_part;
  if (modify_first_only) {
    new_list.Add(sel_list.Get_First());
    if (am_sdebug)
      std::cout << "Returning singleton line list " << new_list << std::endl
                << std::flush;
    return new_list;
  }
  Am_Value_List orig_cmds = execute_command.Get(Am_COMMANDS_PROTOTYPES);
  Am_Object this_line, this_cmd, found_sel;
  for (orig_cmds.Start(); !orig_cmds.Last(); orig_cmds.Next()) {
    this_cmd = orig_cmds.Get();
    if (am_find_cmd_in_sel(sel_list, this_cmd, found_sel))
      new_list.Add(found_sel);
  }
  if (new_list.Length() != sel_list.Length())
    Am_ERROR("sort didn't work, orig " << sel_list << " new " << new_list
                                       << " all cmds " << orig_cmds);
  if (am_sdebug)
    std::cout << "Returning sorted line list " << new_list << std::endl
              << std::flush;
  return new_list;
}

void
replace_value_in_chain(Am_Object &cmd, Am_Slot_Key slot, Am_Value &old_val,
                       Am_Value_List &part_chain, Am_Value new_value)
{
  if (!Am_Value_List::Test(old_val))
    Am_ERROR("Old val " << old_val << " not a list when part_chain");
  Am_Value_List old_list = old_val;
  Am_Value_List new_list = old_list.Recursive_Copy();
  Am_Value this_value;
  this_value = new_list;
  Am_Value v;
  int key;
  Am_Value_List list;
  for (part_chain.Start(); !part_chain.Last(); part_chain.Next()) {
    key = part_chain.Get();
    if (key < 0)
      Am_ERROR("Bad index in part chain " << part_chain);
    if (Am_Value_List::Test(this_value))
      list = this_value;
    else
      Am_ERROR("Element not a list " << this_value);
    this_value = list.Get_Nth(key);
  } //end loop
  //when get here, the key and list are set
  list.Move_Nth(key);
  list.Set(new_value, false); //destructive modification
  if (am_sdebug)
    std::cout << "__Constructed new list " << new_list << " from old "
              << old_list << " using partchain " << part_chain
              << " and new value " << new_value << std::endl
              << std::flush;
  cmd.Set(slot, new_list);
}

// DESIGN: values are saved into the command object itself.  If a create
// operation, then the values from the Am_SLOTS_TO_SAVE slots of start_object
// are put into the command object by gen_create_parameters_line in
// undo_dialog_box.cc.  Hope they don't conflict with any real slots of the
// command object.  The creation of the placeholder (in general_placeholder)
// should have added the slot that the placeholder is in to the slots_to_save
// list of this command object, so it will be used by
// Am_Replace_Create_Placeholders later.

//change the object parameter as specified
void
am_script_replace_all(Am_Value_List &sel_list, Am_Value &new_val,
                      Am_Object &script_window, Am_Slot_Key slot,
                      Am_Object &execute_command)
{
  Am_Object sel_item, cmd;
  Am_Value old_val;
  Am_Value_List new_val_list, part_chain;
  for (sel_list.Start(); !sel_list.Last(); sel_list.Next()) {
    sel_item = sel_list.Get();
    cmd = sel_item.Get(Am_COMMAND);
    part_chain = sel_item.Peek(Am_PART_CHAIN);
    old_val = cmd.Get(slot, Am_OK_IF_NOT_THERE);
    if (part_chain.Valid())
      replace_value_in_chain(cmd, slot, old_val, part_chain, new_val);
    else {
      if (am_sdebug)
        std::cout << "Setting " << cmd << " slot " << slot << " old_val "
                  << old_val << std::endl
                  << std::flush;
      if (Am_Value_List::Test(old_val) && !Am_Value_List::Test(new_val)) {
        new_val_list = Am_Value_List().Add(new_val);
        if (am_sdebug)
          std::cout << "Creating list, set with " << new_val_list << std::endl
                    << std::flush;
        cmd.Set(slot, new_val_list, Am_OK_IF_NOT_THERE);
      } else {
        if (am_sdebug)
          std::cout << " Setting with " << new_val << std::endl << std::flush;
        cmd.Set(slot, new_val, Am_OK_IF_NOT_THERE);
      }
    }
    // ??space leak, delete old placeholder if there ??
  }
  if (am_sdebug)
    std::cout << "Reloading script window\n" << std::flush;
  if (script_window.Valid())
    am_set_commands_into_script_window(script_window, execute_command);
}

bool
obj_or_objects(Am_Value &v)
{
  if (!v.Valid())
    return false;
  if (v.type == Am_OBJECT)
    return true;
  else if (Am_Value_List::Test(v)) {
    Am_Value_List l = v;
    Am_Value lv;
    for (l.Start(); !l.Last(); l.Next()) {
      lv = l.Get();
      if (lv.type != Am_OBJECT)
        return false;
    }
    return true;
  } else
    return false;
}

Am_Define_Method(Am_Object_Method, void, am_objgen_ok, (Am_Object cmd))
{
  Am_Object objgen_window = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  int how_gen = objgen_window.Get_Object(Am_UNDO_OPTIONS).Get(Am_VALUE);
  bool modify_first_only =
      (int)(objgen_window.Get_Object(Am_GREEN_AND_YELLOW).Get(Am_VALUE)) ==
      am_generalize_only_green;
  Am_Object script_window = objgen_window.Get(Am_SCRIPT_WINDOW);
  Am_Value_List sel_list = objgen_window.Get(Am_COMMAND);
  Am_Slot_Key slot = (Am_Slot_Key)(int)objgen_window.Get(Am_SLOTS_TO_SAVE);
  Am_Object execute_command = script_window.Get(Am_SCRIPT_EXECUTE_COMMAND);
  sel_list =
      sort_sel_list_cmd_order(sel_list, execute_command, modify_first_only);

  Am_Object proto;
  Am_Object script_line = sel_list.Get_First();
  Am_Object create_cmd = script_line.Get(Am_COMMAND);
  Am_Object orig_cmd;
  Am_Value old_val = objgen_window.Get(Am_VALUE);
  Am_Value_List placeholder_list = execute_command.Get(Am_PLACEHOLDER_LIST);
  Am_Value_List part_chain;
  int placeholder_cnt = execute_command.Get(Am_PLACEHOLDER_COUNT);
  int slot_for_value = 0;
  switch (how_gen) {
  case am_constant_generalize:
    old_val =
        objgen_window.Get_Object(Am_CONSTANT_OBJECTS_WIDGET).Get(Am_VALUES);
    proto = Am_Constant_Placeholder;
    break;
  case am_selected_generalize:
    proto = Am_The_Selected_Objects_Placeholder;
    break;
  case am_ask_user_generalize:
    proto = Am_User_Select_At_Run_Time_Placeholder;
    break;
  case am_all_values_generalize: {
    orig_cmd = create_cmd;
    create_cmd = objgen_window.Get_Object(Am_ALL_FROM_COMMAND_WIDGET)
                     .Get(Am_VALUES, Am_RETURN_ZERO_ON_ERROR);
    if (!create_cmd.Valid()) {
      AM_POP_UP_ERROR_WINDOW("Command not filled in for All Objects");
      return; //don't finish with this window
    }
    Am_Value v = create_cmd.Get(Am_VALUE);
    if (!obj_or_objects(v)) {
      v = create_cmd.Get(Am_OBJECT_MODIFIED);
      if (obj_or_objects(v))
        slot_for_value = (int)Am_OBJECT_MODIFIED;
      else {
        AM_POP_UP_ERROR_WINDOW("Command's value is not an object.  Is: " << v);
        return; //don't finish with this window
      }
    }
    proto = Am_All_Values_From_Command_Placeholder;
    break;
  }
  case am_the_objects_generalize: {
    orig_cmd = create_cmd;
    create_cmd = objgen_window.Get_Object(Am_SOME_OBJECTS_WIDGET)
                     .Get(Am_VALUES, Am_RETURN_ZERO_ON_ERROR);
    if (!create_cmd.Valid()) {
      AM_POP_UP_ERROR_WINDOW("Command not filled in for The Object");
      return; //don't finish with this window
    }
    Am_Value v = create_cmd.Get(Am_VALUE);
    if (!obj_or_objects(v)) {
      AM_POP_UP_ERROR_WINDOW("Command's value is not an object.  Is: " << v);
      return; //don't finish with this window
    }
    part_chain = objgen_window.Get_Object(Am_INDEXES_OF_COMMANDS_WIDGET)
                     .Get(Am_VALUES, Am_RETURN_ZERO_ON_ERROR);
    proto = Am_An_Object_From_Command_Placeholder;
    break;
  }
  case am_custom_generalize: {
    Am_Error("Custom not yet implemented");
    break;
  }
  default:
    Am_ERROR("Bad type " << how_gen);
  } //end switch

  Am_Object new_ph =
      am_call_create_placeholder(proto, old_val, create_cmd, placeholder_list,
                                 part_chain, placeholder_cnt, slot);
  if (slot_for_value != 0)
    new_ph.Set(Am_SLOT_FOR_VALUE, slot_for_value);      // hack
  if (orig_cmd.Valid() && slot != Am_OBJECT_MODIFIED) { // hack
    // create_cmd parameter to am_call_create_placeholder is
    // confusingly serving 2 purposes: both as the create cmd and the
    // command the placeholder is in.  Here, it conflicts!
    Am_Value_List old_slot_list =
        orig_cmd.Get(Am_SLOTS_TO_SAVE, Am_RETURN_ZERO_ON_ERROR);
    old_slot_list.Append_New(slot);
    if (am_sdebug)
      std::cout << "Adding slot " << slot << " to list " << old_slot_list
                << " for " << orig_cmd << std::endl
                << std::flush;
    orig_cmd.Set(Am_SLOTS_TO_SAVE, old_slot_list, Am_OK_IF_NOT_THERE);
  } //end hack
  if (am_sdebug)
    std::cout << "New placeholder is " << new_ph << std::endl << std::flush;
  execute_command.Set(Am_PLACEHOLDER_LIST, placeholder_list);
  execute_command.Set(Am_PLACEHOLDER_COUNT, placeholder_cnt);
  Am_Value new_value;
  new_value = new_ph;
  am_script_replace_all(sel_list, new_value, script_window, slot,
                        execute_command);
  done_with_objgen_window(objgen_window);
}

Am_Define_Method(Am_Object_Method, void, am_objgen_cancel, (Am_Object cmd))
{
  Am_Object objgen_window = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  done_with_objgen_window(objgen_window);
}

Am_Define_Formula(bool, am_active_if_constant_sel)
{
  return self.Get_Sibling(Am_UNDO_OPTIONS).Get(Am_VALUE) ==
         am_constant_generalize;
}
Am_Define_Formula(bool, am_active_the_object_sel)
{
  return self.Get_Sibling(Am_UNDO_OPTIONS).Get(Am_VALUE) ==
         am_the_objects_generalize;
}
Am_Define_Formula(bool, am_active_all_values_sel)
{
  return self.Get_Sibling(Am_UNDO_OPTIONS).Get(Am_VALUE) ==
         am_all_values_generalize;
}
Am_Define_Formula(bool, am_active_all_or_the_object_sel)
{
  int sel = self.Get_Sibling(Am_UNDO_OPTIONS).Get(Am_VALUE);
  return sel == am_all_values_generalize || sel == am_the_objects_generalize;
}

Am_Define_Method(Am_Object_Method, void, am_load_current_command,
                 (Am_Object cmd))
{
  Am_Object objgen_window = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  Am_Object script_window = objgen_window.Get(Am_SCRIPT_WINDOW);
  if (am_sdebug)
    std::cout << "Do load; cmd " << cmd << " objgen_window " << objgen_window
              << " script_window " << script_window << std::endl
              << std::flush;
  Am_Value_List l =
      script_window.Get_Object(Am_UNDO_SCROLL_GROUP).Get(Am_VALUE);
  if (!l.Valid()) {
    AM_POP_UP_ERROR_WINDOW("Select one script line in the Script Window first");
  } else if (l.Length() > 1) {
    AM_POP_UP_ERROR_WINDOW("Select only one script line in the Script Window");
  } else {
    l.Start();
    Am_Object script_line = l.Get();
    Am_Object cmd = script_line.Get(Am_COMMAND);
    Am_String s = am_gen_command_label(cmd);
    if (am_sdebug)
      std::cout << " for command " << cmd << " label is " << s << std::endl
                << std::flush;
    if (objgen_window.Get_Object(Am_UNDO_OPTIONS).Get(Am_VALUE) ==
        am_all_values_generalize)
      objgen_window.Get_Object(Am_ALL_FROM_COMMAND_WIDGET)
          .Set(Am_VALUES, cmd, Am_OK_IF_NOT_THERE)
          .Set(Am_VALUE, s);
    else
      objgen_window.Get_Object(Am_SOME_OBJECTS_WIDGET)
          .Set(Am_VALUES, cmd, Am_OK_IF_NOT_THERE)
          .Set(Am_VALUE, s);
  }
}

bool
am_get_placeholder_from(Am_Value &current_val, Am_Object &ph)
{
  ph = Am_No_Object;
  Am_Value val;
  if (current_val.type == Am_OBJECT) {
    Am_Object obj = current_val;
    if (obj.Is_Instance_Of(Am_A_Placeholder))
      ph = obj;
  } else if (Am_Value_List::Test(current_val)) {
    bool found_ph = false;
    bool found_reg = false;
    Am_Value_List val_list = current_val;
    for (val_list.Start(); !val_list.Last(); val_list.Next()) {
      val = val_list.Get();
      if (val.type == Am_OBJECT &&
          Am_Object(val).Is_Instance_Of(Am_A_Placeholder)) {
        ph = val;
        if (found_reg) {
          AM_POP_UP_ERROR_WINDOW("Not yet implemented to generalize when some "
                                 "are placeholders already");
          return false;
        }
        if (found_ph) {
          AM_POP_UP_ERROR_WINDOW(
              "Not yet implemented when contains more than one placeholder");
          return false;
        }
        found_ph = true;
      } else { //here, val isn't a placeholder
        if (found_ph) {
          AM_POP_UP_ERROR_WINDOW("Not yet implemented to generalize when some "
                                 "are placeholders already");
          return false;
        }
        found_reg = true;
      }
    }
  }
  return true;
}

void
put_part_chain(Am_Object &objgen_window, Am_Object &ph)
{
  char line[300];
  OSTRSTREAM_CONSTR(oss, line, 300, std::ios::out);
  reset_ostrstream(oss); // go to the beginning
  Am_Value_List part_chain = ph.Get(Am_PART_CHAIN, Am_RETURN_ZERO_ON_ERROR);
  int index;
  if (!part_chain.Valid())
    oss << "1";
  else {
    part_chain.Start();
    index = part_chain.Get();
    oss << index + 1;
  }
  oss << std::ends;
  OSTRSTREAM_COPY(oss, line, 300);
  objgen_window.Get_Object(Am_INDEXES_OF_COMMANDS_WIDGET)
      .Set(Am_VALUES, part_chain, Am_OK_IF_NOT_THERE)
      .Set(Am_VALUE, line);
}

void
popup_generalize_script_objects(Am_Object &main_script_line_part,
                                Am_Object &script_window,
                                Am_Value_List &sel_list)
{
  Am_Value current_val = main_script_line_part.Get(Am_VALUE);
  Am_Object main_cmd = main_script_line_part.Get(Am_COMMAND);
  Am_Object ph;
  bool ok = am_get_placeholder_from(current_val, ph);
  if (!ok)
    return;
  Am_Object objgen_window = get_objgen_window();
  objgen_window.Set(Am_SCRIPT_WINDOW, script_window);
  objgen_window.Set(Am_COMMAND, sel_list);
  objgen_window.Set(Am_SLOTS_TO_SAVE,
                    main_script_line_part.Get(Am_SLOTS_TO_SAVE));
  objgen_window.Set(Am_VALUE, current_val);
  if (ph.Valid()) {
    Am_Value orig_value = ph.Get(Am_VALUE);
    if (orig_value.Valid())
      if (am_sdebug)
        std::cout << "setting value " << orig_value << " type "
                  << orig_value.type << std::endl
                  << std::flush;
    objgen_window
        .Get_Object(Am_CONSTANT_OBJECTS_WIDGET)
        // so won't turn into a string
        .Set(Am_VALUES, orig_value, Am_OK_IF_NOT_THERE)
        .Set(Am_VALUE, orig_value);
    objgen_window.Set(Am_PLACEHOLDERS_SET_BY_THIS_COMMAND, ph);

    if (ph.Is_Instance_Of(Am_All_Values_From_Command_Placeholder)) {
      Am_Object for_cmd = ph.Get(Am_COMMAND);
      Am_String cmd_label = am_gen_command_label(for_cmd);
      objgen_window.Get_Object(Am_UNDO_OPTIONS)
          .Set(Am_VALUE, am_all_values_generalize);
      objgen_window.Get_Object(Am_ALL_FROM_COMMAND_WIDGET)
          .Set(Am_VALUES, for_cmd, Am_OK_IF_NOT_THERE)
          .Set(Am_VALUE, cmd_label);
    } else if (ph.Is_Instance_Of(Am_An_Object_From_Command_Placeholder)) {
      Am_Object for_cmd = ph.Get(Am_COMMAND);
      Am_String cmd_label = am_gen_command_label(for_cmd);
      objgen_window.Get_Object(Am_UNDO_OPTIONS)
          .Set(Am_VALUE, am_the_objects_generalize);
      objgen_window.Get_Object(Am_SOME_OBJECTS_WIDGET)
          .Set(Am_VALUES, for_cmd, Am_OK_IF_NOT_THERE)
          .Set(Am_VALUE, cmd_label);
      put_part_chain(objgen_window, ph);
    } else if (ph.Is_Instance_Of(Am_The_Selected_Objects_Placeholder)) {
      objgen_window.Get_Object(Am_UNDO_OPTIONS)
          .Set(Am_VALUE, am_selected_generalize);
    } else if (ph.Is_Instance_Of(Am_User_Select_At_Run_Time_Placeholder)) {
      objgen_window.Get_Object(Am_UNDO_OPTIONS)
          .Set(Am_VALUE, am_ask_user_generalize);
    } else
      Am_ERROR("Haven't handled placeholder type " << ph);
  } else {
    objgen_window.Get_Object(Am_UNDO_OPTIONS)
        .Set(Am_VALUE, am_constant_generalize);
    Am_Value orig_value = main_script_line_part.Get(Am_VALUE);
    if (am_sdebug)
      std::cout << "setting value " << orig_value << " type " << orig_value.type
                << std::endl
                << std::flush;
    objgen_window.Get_Object(Am_CONSTANT_OBJECTS_WIDGET)
        .Set(Am_VALUE, orig_value)
        // so won't turn into a string
        .Set(Am_VALUES, orig_value, Am_OK_IF_NOT_THERE);
  }
  if (sel_list.Length() > 1) {
    objgen_window.Get_Object(Am_GREEN_AND_YELLOW)
        .Set(Am_ACTIVE, true)
        .Set(Am_VALUE, am_generalize_green_and_yellow);
  } else {
    objgen_window.Get_Object(Am_GREEN_AND_YELLOW)
        .Set(Am_ACTIVE, false)
        .Set(Am_VALUE, am_generalize_only_green);
  }
  //not modal, don't wait
  objgen_window.Set(Am_VISIBLE, true);
}

/////////////////////////////////////////////////////////////////////////
// Location Generalizers
/////////////////////////////////////////////////////////////////////////

Am_Value_List locgen_windows;
int num_vis_locgen_windows = 0;

Am_Object
get_locgen_window()
{
  Am_Object w =
      am_get_db_from_list(am_locgen, locgen_windows, num_vis_locgen_windows);
  return w;
}
void
done_with_locgen_window(Am_Object sw)
{
  sw.Set(Am_VISIBLE, false);
  locgen_windows.Add(sw);
  num_vis_locgen_windows--;
}

Am_Define_Method(Am_Object_Method, void, am_locgen_ok, (Am_Object cmd))
{
  Am_Object locgen_window = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  Am_Object script_window = locgen_window.Get(Am_SCRIPT_WINDOW);
  Am_Value_List sel_list = locgen_window.Get(Am_COMMAND);
  Am_Slot_Key slot = (Am_Slot_Key)(int)locgen_window.Get(Am_SLOTS_TO_SAVE);
  Am_Object execute_command = script_window.Get(Am_SCRIPT_EXECUTE_COMMAND);

  sel_list.Start();
  if (sel_list.Length() != 1)
    if (am_sdebug)
      std::cout << "** Sel_list >1 " << sel_list << " just using first\n"
                << std::flush;

  Am_Value_List placeholder_list = execute_command.Get(Am_PLACEHOLDER_LIST);
  int placeholder_cnt = execute_command.Get(Am_PLACEHOLDER_COUNT);
  Am_Value old_val = locgen_window.Get(Am_VALUE);

  Am_Object script_line = sel_list.Get_First();
  Am_Object create_cmd = script_line.Get(Am_COMMAND);
  if (!create_cmd.Is_Instance_Of(Am_Object_Create_Command) &&
      !create_cmd.Is_Instance_Of(Am_Find_Command))
    create_cmd = Am_No_Object;
  Am_Object new_ph = am_call_create_placeholder(
      Am_Inter_Location_Placeholder, old_val, create_cmd, placeholder_list,
      Am_No_Value_List, placeholder_cnt, slot);
  if (am_sdebug)
    std::cout << "New placeholder is " << new_ph << std::endl << std::flush;
  execute_command.Set(Am_PLACEHOLDER_LIST, placeholder_list);
  execute_command.Set(Am_PLACEHOLDER_COUNT, placeholder_cnt);

  new_ph.Set(Am_VALUES, locgen_window.Get(Am_VALUES), Am_OK_IF_NOT_THERE);
  new_ph.Set(Am_LOCATION_PLACEHOLDER,
             locgen_window.Get(Am_LOCATION_PLACEHOLDER), Am_OK_IF_NOT_THERE);

  int control = locgen_window.Get_Object(Am_CHANGE_LEFT).Get(Am_VALUE);
  if (control)
    control = locgen_window.Get_Object(Am_LEFT_HOW).Get(Am_VALUE);
  new_ph.Set(Am_LEFT_HOW, control);
  new_ph.Set(Am_LEFT_AMOUNT_WIDGET,
             locgen_window.Get_Object(Am_LEFT_AMOUNT_WIDGET).Get(Am_VALUE));

  control = locgen_window.Get_Object(Am_CHANGE_TOP).Get(Am_VALUE);
  if (control)
    control = locgen_window.Get_Object(Am_TOP_HOW).Get(Am_VALUE);
  new_ph.Set(Am_TOP_HOW, control);
  new_ph.Set(Am_TOP_AMOUNT_WIDGET,
             locgen_window.Get_Object(Am_TOP_AMOUNT_WIDGET).Get(Am_VALUE));

  control = locgen_window.Get_Object(Am_CHANGE_WIDTH).Get(Am_VALUE);
  if (control)
    control = locgen_window.Get_Object(Am_WIDTH_HOW).Get(Am_VALUE);
  new_ph.Set(Am_WIDTH_HOW, control);
  new_ph.Set(Am_WIDTH_AMOUNT_WIDGET,
             locgen_window.Get_Object(Am_WIDTH_AMOUNT_WIDGET).Get(Am_VALUE));

  control = locgen_window.Get_Object(Am_CHANGE_HEIGHT).Get(Am_VALUE);
  if (control)
    control = locgen_window.Get_Object(Am_HEIGHT_HOW).Get(Am_VALUE);
  new_ph.Set(Am_HEIGHT_HOW, control);
  new_ph.Set(Am_HEIGHT_AMOUNT_WIDGET,
             locgen_window.Get_Object(Am_HEIGHT_AMOUNT_WIDGET).Get(Am_VALUE));

  if ((bool)locgen_window.Get_Object(Am_CONSTANT_OBJECTS_WIDGET)
          .Get(Am_ACTIVE)) {
    //*** TEMP //should be value from that slot
    new_ph.Set(Am_CONSTANT_OBJECTS_WIDGET, new_ph, Am_OK_IF_NOT_THERE);
  } else
    new_ph.Set(Am_CONSTANT_OBJECTS_WIDGET, Am_No_Object, Am_OK_IF_NOT_THERE);

  Am_Value new_value;
  new_value = new_ph;
  am_script_replace_all(sel_list, new_value, script_window, slot,
                        execute_command);
  done_with_locgen_window(locgen_window);
}

Am_Define_Method(Am_Object_Method, void, am_locgen_cancel, (Am_Object cmd))
{
  Am_Object locgen_window = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  done_with_locgen_window(locgen_window);
}

Am_Define_Method(Am_Object_Method, void, am_loc_from_object,
                 (Am_Object /* cmd */))
{
  std::cout << "*****Should pop up an object window\n" << std::flush;
}

am_loc_values *
create_loc_values(Am_Inter_Location &current_loc, Am_Value &old_val)
{
  am_loc_values *ret = new am_loc_values();
  bool as_line;
  int a, b, c, d;
  Am_Object ref_obj;
  current_loc.Get_Location(as_line, ref_obj, a, b, c, d);
  if (as_line)
    Am_Error("Shouldn't be as_line");
  ret->orig_value = current_loc;
  ret->growing = current_loc.Get_Growing();
  ret->left_equals = a;
  ret->top_equals = b;
  ret->width_equals = c;
  ret->height_equals = d;
  if (old_val.Valid()) {
    if (Am_Inter_Location::Test(old_val)) {
      int oa, ob, oc, od;
      Am_Object oref_obj;
      Am_Inter_Location old_loc = old_val;
      old_loc.Get_Location(as_line, oref_obj, oa, ob, oc, od);
      if (as_line) {
        std::cout << "**old value is as line " << old_loc << std::endl
                  << std::flush;
      } else {
        if (oref_obj != ref_obj)
          Am_Translate_Coordinates(oref_obj, oa, ob, ref_obj, oa, ob);
        ret->left_offset = a - oa;
        ret->top_offset = b - ob;
        ret->width_offset = c - oc;
        ret->height_offset = d - od;
      }
    } else {
      std::cout << "**Old val not an inter location " << old_val << std::endl
                << std::flush;
    }
  }
  return ret;
}

//uses old_val to compute the offsets
bool
set_location_window_from_val(Am_Object &locgen_window,
                             Am_Inter_Location &current_val, Am_Value &old_val)
{
  bool as_line;
  int a, b, c, d;
  Am_Object ref_obj;
  current_val.Get_Location(as_line, ref_obj, a, b, c, d);
  bool growing = current_val.Get_Growing();
  if (as_line) {
    AM_POP_UP_ERROR_WINDOW("Not yet implemented to generalize when as a line");
    return false;
  }

  am_loc_values *loc_values = create_loc_values(current_val, old_val);
  locgen_window.Set(Am_VALUES, (Am_Ptr)loc_values);

  locgen_window.Get_Object(Am_CHANGE_LEFT).Set(Am_VALUE, true);
  locgen_window.Get_Object(Am_LEFT_HOW).Set(Am_VALUE, am_change_equals);
  locgen_window.Get_Object(Am_LEFT_AMOUNT_WIDGET).Set(Am_VALUE, a);

  locgen_window.Get_Object(Am_CHANGE_TOP).Set(Am_VALUE, true);
  locgen_window.Get_Object(Am_TOP_HOW).Set(Am_VALUE, am_change_equals);
  locgen_window.Get_Object(Am_TOP_AMOUNT_WIDGET).Set(Am_VALUE, b);

  locgen_window.Get_Object(Am_CHANGE_WIDTH).Set(Am_ACTIVE, growing);
  locgen_window.Get_Object(Am_CHANGE_WIDTH).Set(Am_VALUE, growing);
  locgen_window.Get_Object(Am_WIDTH_HOW).Set(Am_VALUE, am_change_equals);
  locgen_window.Get_Object(Am_WIDTH_AMOUNT_WIDGET).Set(Am_VALUE, c);

  locgen_window.Get_Object(Am_CHANGE_HEIGHT).Set(Am_ACTIVE, growing);
  locgen_window.Get_Object(Am_CHANGE_HEIGHT).Set(Am_VALUE, growing);
  locgen_window.Get_Object(Am_HEIGHT_HOW).Set(Am_VALUE, am_change_equals);
  locgen_window.Get_Object(Am_HEIGHT_AMOUNT_WIDGET).Set(Am_VALUE, d);

  locgen_window.Get_Object(Am_CONSTANT_OBJECTS_WIDGET).Set(Am_VALUE, "");

  locgen_window.Set(Am_VALUE, current_val);

  return true;
}

void
popup_generalize_script_location(Am_Object &main_script_line_part,
                                 Am_Object &script_window,
                                 Am_Value_List &sel_list)
{
  Am_Value current_val = main_script_line_part.Get(Am_VALUE);
  Am_Object main_cmd = main_script_line_part.Get(Am_COMMAND);
  Am_Value old_val = main_cmd.Get(Am_OLD_VALUE, Am_RETURN_ZERO_ON_ERROR);
  Am_Object ph;
  bool ok = am_get_placeholder_from(current_val, ph);
  if (!ok)
    return;
  Am_Object locgen_window = get_locgen_window();
  locgen_window.Set(Am_SCRIPT_WINDOW, script_window)
      .Set(Am_COMMAND, sel_list)
      .Set(Am_SLOTS_TO_SAVE, main_script_line_part.Get(Am_SLOTS_TO_SAVE))
      .Set(Am_LOCATION_PLACEHOLDER,
           main_script_line_part.Get(Am_LOCATION_PLACEHOLDER,
                                     Am_RETURN_ZERO_ON_ERROR),
           Am_OK_IF_NOT_THERE);

  if (am_sdebug)
    std::cout << "Popup win " << locgen_window << " for val " << current_val
              << " from command " << main_cmd << std::endl
              << std::flush;
  if (ph.Valid()) {
    if (am_sdebug)
      std::cout << "loc found placeholder " << ph << std::endl << std::flush;
    Am_Inter_Location orig_loc = ph.Get(Am_VALUE);
    bool growing = orig_loc.Get_Growing();

    locgen_window.Set(Am_VALUE, orig_loc);

    am_loc_values *loc_values;
    am_loc_values *old_loc_values = (am_loc_values *)(Am_Ptr)ph.Get(Am_VALUES);
    if (old_loc_values) {
      loc_values = new am_loc_values();
      *loc_values = *old_loc_values; //copy the values
    } else
      loc_values = create_loc_values(orig_loc, old_val);
    locgen_window.Set(Am_VALUES, (Am_Ptr)loc_values);

    int control = ph.Get(Am_LEFT_HOW);
    ok = (control ? 1 : 0);
    locgen_window.Get_Object(Am_CHANGE_LEFT).Set(Am_VALUE, ok);
    if (!control)
      control = am_change_equals;
    locgen_window.Get_Object(Am_LEFT_HOW).Set(Am_VALUE, control);
    locgen_window.Get_Object(Am_LEFT_AMOUNT_WIDGET)
        .Set(Am_VALUE, ph.Get(Am_LEFT_AMOUNT_WIDGET));

    control = ph.Get(Am_TOP_HOW);
    ok = (control ? 1 : 0);
    locgen_window.Get_Object(Am_CHANGE_TOP).Set(Am_VALUE, ok);
    if (!control)
      control = am_change_equals;
    locgen_window.Get_Object(Am_TOP_HOW).Set(Am_VALUE, control);
    locgen_window.Get_Object(Am_TOP_AMOUNT_WIDGET)
        .Set(Am_VALUE, ph.Get(Am_TOP_AMOUNT_WIDGET));

    control = ph.Get(Am_WIDTH_HOW);
    ok = (control ? 1 : 0);
    locgen_window.Get_Object(Am_CHANGE_WIDTH).Set(Am_ACTIVE, growing);
    locgen_window.Get_Object(Am_CHANGE_WIDTH).Set(Am_VALUE, ok);
    if (!control)
      control = am_change_equals;
    locgen_window.Get_Object(Am_WIDTH_HOW).Set(Am_VALUE, control);
    locgen_window.Get_Object(Am_WIDTH_AMOUNT_WIDGET)
        .Set(Am_VALUE, ph.Get(Am_WIDTH_AMOUNT_WIDGET));

    control = ph.Get(Am_HEIGHT_HOW);
    ok = (control ? 1 : 0);
    locgen_window.Get_Object(Am_CHANGE_HEIGHT).Set(Am_ACTIVE, growing);
    locgen_window.Get_Object(Am_CHANGE_HEIGHT).Set(Am_VALUE, ok);
    if (!control)
      control = am_change_equals;
    locgen_window.Get_Object(Am_HEIGHT_HOW).Set(Am_VALUE, control);
    locgen_window.Get_Object(Am_HEIGHT_AMOUNT_WIDGET)
        .Set(Am_VALUE, ph.Get(Am_HEIGHT_AMOUNT_WIDGET));

    //*** deal with slot Am_CONSTANT_OBJECTS_WIDGET

  } else {
    Am_Inter_Location cur_loc = current_val;
    if (!set_location_window_from_val(locgen_window, cur_loc, old_val))
      return;
  }
  //not modal, don't wait
  locgen_window.Set(Am_VISIBLE, true);
}

/////////////////////////////////////////////////////////////////////////
// Value Generalizers
/////////////////////////////////////////////////////////////////////////

Am_Value_List valgen_windows;
int num_vis_valgen_windows = 0;

Am_Object
get_valgen_window()
{
  Am_Object w =
      am_get_db_from_list(am_valgen, valgen_windows, num_vis_valgen_windows);
  return w;
}
void
done_with_valgen_window(Am_Object sw)
{
  sw.Set(Am_VISIBLE, false);
  valgen_windows.Add(sw);
  num_vis_valgen_windows--;
}

Am_Define_Method(Am_Object_Method, void, am_valgen_ok, (Am_Object cmd))
{
  Am_Object valgen_window = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  Am_Object script_window = valgen_window.Get(Am_SCRIPT_WINDOW);
  Am_Value_List sel_list = valgen_window.Get(Am_COMMAND);
  Am_Slot_Key slot = (Am_Slot_Key)(int)valgen_window.Get(Am_SLOTS_TO_SAVE);
  Am_Object execute_command = script_window.Get(Am_SCRIPT_EXECUTE_COMMAND);
  int how_gen = valgen_window.Get_Object(Am_UNDO_OPTIONS).Get(Am_VALUE);
  bool modify_first_only =
      (int)(valgen_window.Get_Object(Am_GREEN_AND_YELLOW).Get(Am_VALUE)) ==
      am_generalize_only_green;
  sel_list =
      sort_sel_list_cmd_order(sel_list, execute_command, modify_first_only);
  Am_Value_List part_chain;
  Am_Object proto;
  Am_Object script_line = sel_list.Get_First();
  Am_Object create_cmd = script_line.Get(Am_COMMAND);
  if (!create_cmd.Is_Instance_Of(Am_Object_Create_Command) &&
      !create_cmd.Is_Instance_Of(Am_Find_Command))
    create_cmd = Am_No_Object;
  Am_Value old_val = valgen_window.Get(Am_VALUE);

  Am_Value_List placeholder_list = execute_command.Get(Am_PLACEHOLDER_LIST);
  int placeholder_cnt = execute_command.Get(Am_PLACEHOLDER_COUNT);
  switch (how_gen) {
  case am_constant_generalize:
    old_val =
        valgen_window.Get_Object(Am_CONSTANT_OBJECTS_WIDGET).Get(Am_VALUES);
    proto = Am_Constant_Placeholder;
    break;
  case am_palette_generalize:
    part_chain.Add(valgen_window.Get(Am_REGISTRY_FOR_PALETTES)); //hack
    proto = Am_Property_From_Palette_Placeholder;
    break;
  case am_next_value_generalize:
    old_val = valgen_window.Get_Object(Am_NEXT_CONSTANT_OBJECTS_WIDGET)
                  .Get(Am_VALUES);
    proto = Am_Cycle_Values_Placeholder;
    break;
  case am_ask_user_generalize:
    proto = Am_Ask_User_Property_Placeholder;
    break;
  case am_all_values_generalize:
    create_cmd = valgen_window.Get_Object(Am_ALL_FROM_COMMAND_WIDGET)
                     .Get(Am_VALUES, Am_RETURN_ZERO_ON_ERROR);
    if (!create_cmd.Valid()) {
      AM_POP_UP_ERROR_WINDOW("Command not filled in.");
      return; //don't finish with this window
    }
    proto = Am_All_Values_From_Command_Placeholder;
    break;
  case am_custom_generalize: {
    Am_Error("Custom not yet implemented");
    break;
  }
  default:
    Am_ERROR("Bad type " << how_gen);
  } //end switch

  Am_Object new_ph =
      am_call_create_placeholder(proto, old_val, create_cmd, placeholder_list,
                                 part_chain, placeholder_cnt, slot);
  if (am_sdebug)
    std::cout << "ValGen: New placeholder is " << new_ph << std::endl
              << std::flush;
  execute_command.Set(Am_PLACEHOLDER_LIST, placeholder_list);
  execute_command.Set(Am_PLACEHOLDER_COUNT, placeholder_cnt);
  Am_Value new_value;
  new_value = new_ph;
  am_script_replace_all(sel_list, new_value, script_window, slot,
                        execute_command);
  done_with_valgen_window(valgen_window);
}

Am_Define_Method(Am_Object_Method, void, am_valgen_cancel, (Am_Object cmd))
{
  Am_Object valgen_window = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  done_with_valgen_window(valgen_window);
}

Am_Object
find_palette_for_command(Am_Object &main_cmd, Am_Value &current_value,
                         Am_Object &script_window)
{
  Am_Value slot_key_value = main_cmd.Peek(Am_SLOT_FOR_VALUE);
  Am_Am_Slot_Key slot;
  if (!slot_key_value.Valid())
    slot = 0;
  else
    slot = slot_key_value;
  Am_Object undo_db = script_window.Get_Object(Am_UNDO_MENU_OF_COMMANDS);
  Am_Object undo_handler = undo_db.Get(Am_UNDO_HANDLER_TO_DISPLAY);
  if (am_sdebug)
    std::cout << "searching for palette for undo " << undo_handler << " from "
              << undo_db << " from " << script_window << " slot " << slot
              << " value " << current_value << std::endl
              << std::flush;
  Am_Object palette = Am_Find_Palette_For(undo_handler, slot, current_value);
  if (am_sdebug)
    std::cout << "  found  " << palette << std::endl << std::flush;
  return palette;
}

void
popup_generalize_script_value(Am_Value &current_val,
                              Am_Object &main_script_line_part,
                              Am_Object &script_window, Am_Value_List &sel_list)
{
  Am_Object main_cmd = main_script_line_part.Get(Am_COMMAND);
  Am_Object ph;
  bool ok = am_get_placeholder_from(current_val, ph);
  if (!ok)
    return;
  Am_Object valgen_window = get_valgen_window();
  valgen_window.Set(Am_SCRIPT_WINDOW, script_window)
      .Set(Am_COMMAND, sel_list)
      .Set(Am_SLOTS_TO_SAVE, main_script_line_part.Get(Am_SLOTS_TO_SAVE));
  if (am_sdebug)
    std::cout << "Popup win " << valgen_window << " for val " << current_val
              << " from command " << main_cmd << std::endl
              << std::flush;
  Am_Value_List list_val;
  if (ph.Valid()) {
    if (am_sdebug)
      std::cout << "val found placeholder " << ph << std::endl << std::flush;
    current_val = ph.Get(Am_VALUE);
    if (am_sdebug)
      std::cout << "setting value " << current_val << " type "
                << current_val.type << std::endl
                << std::flush;
    valgen_window.Set(Am_PLACEHOLDERS_SET_BY_THIS_COMMAND, ph);
    if (ph.Is_Instance_Of(Am_Constant_Placeholder)) {
      valgen_window.Get_Object(Am_UNDO_OPTIONS)
          .Set(Am_VALUE, am_constant_generalize);
    } else if (ph.Is_Instance_Of(Am_Property_From_Palette_Placeholder)) {
      valgen_window.Get_Object(Am_UNDO_OPTIONS)
          .Set(Am_VALUE, am_palette_generalize);
    } else if (ph.Is_Instance_Of(Am_Ask_User_Property_Placeholder)) {
      valgen_window.Get_Object(Am_UNDO_OPTIONS)
          .Set(Am_VALUE, am_ask_user_generalize);
    } else if (ph.Is_Instance_Of(Am_All_Values_From_Command_Placeholder)) {
      valgen_window.Get_Object(Am_UNDO_OPTIONS)
          .Set(Am_VALUE, am_all_values_generalize);
      Am_Object for_cmd = ph.Get(Am_COMMAND);
      Am_String cmd_label = am_gen_command_label(for_cmd);
      valgen_window.Get_Object(Am_ALL_FROM_COMMAND_WIDGET)
          .Set(Am_VALUES, for_cmd, Am_OK_IF_NOT_THERE)
          .Set(Am_VALUE, cmd_label);
    } else if (ph.Is_Instance_Of(Am_Cycle_Values_Placeholder)) {
      valgen_window.Get_Object(Am_UNDO_OPTIONS)
          .Set(Am_VALUE, am_next_value_generalize);
      list_val = current_val;
      int index = ph.Get(Am_CURRENT_PHASE);
      current_val = list_val.Get_Nth(index);
    } else
      Am_ERROR("Haven't handled placeholder type " << ph);
  } else {
    valgen_window.Get_Object(Am_UNDO_OPTIONS)
        .Set(Am_VALUE, am_constant_generalize);
    if (am_sdebug)
      std::cout << "No placeholder; setting value " << current_val << " type "
                << current_val.type << std::endl
                << std::flush;
  }
  if (!list_val.Valid())
    list_val.Add(current_val);
  valgen_window.Set(Am_VALUE, current_val);
  valgen_window.Get_Object(Am_CONSTANT_OBJECTS_WIDGET)
      .Set(Am_VALUES, current_val)
      .Set(Am_VALUE, current_val);
  valgen_window.Get_Object(Am_NEXT_CONSTANT_OBJECTS_WIDGET)
      .Set(Am_VALUE, list_val)
      .Set(Am_VALUES, list_val);
  Am_Object palette =
      find_palette_for_command(main_cmd, current_val, script_window);
  valgen_window.Set(Am_REGISTRY_FOR_PALETTES, palette);

  if (sel_list.Length() > 1) {
    valgen_window.Get_Object(Am_GREEN_AND_YELLOW)
        .Set(Am_ACTIVE, true)
        .Set(Am_VALUE, am_generalize_green_and_yellow);
  } else {
    valgen_window.Get_Object(Am_GREEN_AND_YELLOW)
        .Set(Am_ACTIVE, false)
        .Set(Am_VALUE, am_generalize_only_green);
  }
  valgen_window.Set(Am_VISIBLE, true);
}

/////////////////////////////////////////////////////////////////////////

void
Am_Pop_Up_Generalize_DB(Am_Object &script_window, Am_Value_List &sel_list)
{
  sel_list.Start();
  Am_Object main_script_line_part = sel_list.Get();
  Am_Slot_Key key =
      (Am_Slot_Key)(int)main_script_line_part.Get(Am_SLOTS_TO_SAVE);
  if (am_sdebug)
    std::cout << "sel " << main_script_line_part << " slot " << key << std::endl
              << std::flush;
  if (key == Am_OBJECT_MODIFIED) {
    popup_generalize_script_objects(main_script_line_part, script_window,
                                    sel_list);
  } else { //not object modified
    Am_Value value = main_script_line_part.Get(Am_VALUE);
    if (am_sdebug)
      std::cout << "Value for value is " << value << std::endl << std::flush;
    if (Am_Inter_Location::Test(value) ||
        (value.type == Am_OBJECT &&
         Am_Object(value).Is_Instance_Of(Am_Inter_Location_Placeholder))) {
      popup_generalize_script_location(main_script_line_part, script_window,
                                       sel_list);
    } else if (Am_Value_List::Test(value) &&
               Am_Value_List(value).Length() > 1) {
      AM_POP_UP_ERROR_WINDOW("Generalizing on List of Values not yet "
                             "implemented.  Select one value in list.");
    } else if (value.type == Am_OBJECT) {
      Am_Object obj = value;
      if (!obj.Is_Instance_Of(Am_A_Placeholder)) {
        // not a placeholder
        if (key == Am_VALUE) {
          AM_POP_UP_ERROR_WINDOW(
              "Can't generalize on Objects returned from commands.");
        } else
          popup_generalize_script_objects(main_script_line_part, script_window,
                                          sel_list);
      } else { //is a placeholder
        Am_Value old_val = obj.Get(Am_VALUE);
        std::cout << "for placeholder " << obj << " found old val " << old_val
                  << std::endl
                  << std::flush;
        if (old_val.type == Am_OBJECT)
          popup_generalize_script_objects(main_script_line_part, script_window,
                                          sel_list);
        else
          popup_generalize_script_value(value, main_script_line_part,
                                        script_window, sel_list);
      }
    } else // not an object
      popup_generalize_script_value(value, main_script_line_part, script_window,
                                    sel_list);
  }
}

/////////////////////////////////////////////////////////////////////////

//lets scripting know about a palette.  If the slot_or_type is a type,
//then the palette is used for all values of that type.  For example,
//use an instance of Am_Style (e.g., Am_Red) to specify the palette is
//for all colors.  If only for a particular slot's values, use an
//Am_Am_Slot_Key wrapper for a slot, e.g.,
//Am_Am_Slot_Key(Am_PROTOTYPE) for the tool panel, or
//Am_Am_Slot_Key(Am_LINE_STYLE) if only for the line style
void
Am_Scripting_Register_Palette(Am_Object undo_handler, Am_Object palette,
                              Am_Value slot_or_type)
{
  if (Am_Am_Slot_Key::Test(slot_or_type)) {
    palette.Set(Am_SLOT_FOR_VALUE, slot_or_type, Am_OK_IF_NOT_THERE);
  }
  Am_Value_List palette_registry =
      undo_handler.Get(Am_REGISTRY_FOR_PALETTES, Am_RETURN_ZERO_ON_ERROR);
  Am_Assoc as(palette, slot_or_type);
  palette_registry.Add(as);
  undo_handler.Set(Am_REGISTRY_FOR_PALETTES, palette_registry,
                   Am_OK_IF_NOT_THERE);
  if (am_sdebug)
    std::cout << "Registering palette " << palette << " for " << slot_or_type
              << " to " << undo_handler << " got new list " << palette_registry
              << std::endl
              << std::flush;
}

Am_Object
Am_Find_Palette_For(Am_Object &undo_handler, Am_Am_Slot_Key key_wrapper,
                    Am_Value value)
{
  Am_Value_List palette_registry =
      undo_handler.Get(Am_REGISTRY_FOR_PALETTES, Am_RETURN_ZERO_ON_ERROR);
  Am_Object found = palette_registry.Assoc_1(key_wrapper);
  if (found.Valid())
    return found;
  Am_Assoc as;
  Am_Value v;
  for (palette_registry.Start(); !palette_registry.Last();
       palette_registry.Next()) {
    as = palette_registry.Get();
    v = as.Value_2();
    if (v.type == value.type)
      return as.Value_1();
  }
  return Am_No_Object;
}

/////////////////////////////////////////////////////////////////////////

void
Am_Initialize_Generalizers()
{

  Am_A_Placeholder =
      Am_Root_Object.Create(DSTR("Am_A_Placeholder"))
          .Add(Am_VALUE, (0L))
          .Add(Am_ID, 0)
          .Add(Am_LABEL, (0L))
          .Add(Am_COMMAND, (0L))    // command that created this object
          .Add(Am_PART_CHAIN, (0L)) //if object comes from a chain
          .Add(Am_SLOT_FOR_VALUE, (0L))
          .Add(Am_SLOTS_TO_SAVE, (0L))
          .Add(Am_PLACEHOLDER_REPLACE_METHOD, (0L))
          .Add(Am_PLACEHOLDER_SET_VALUE_METHOD, (0L))
          .Add(Am_PLACEHOLDER_CREATE_METHOD, (0L))
          .Add(Am_PLACEHOLDER_MATCH_METHOD, (0L));

  Am_Constant_Placeholder =
      Am_A_Placeholder.Create("Am_Constant_Placeholder")
          .Set(Am_PLACEHOLDER_REPLACE_METHOD, value_replace_placeholder)
          .Set(Am_PLACEHOLDER_SET_VALUE_METHOD, (0L))
          .Set(Am_PLACEHOLDER_CREATE_METHOD, constant_placeholder_create)
          .Set(Am_PLACEHOLDER_MATCH_METHOD, constant_do_match_placeholder);

  Am_All_Values_From_Command_Placeholder =
      Am_A_Placeholder.Create("Am_All_Values_From_Command_Placeholder")
          .Set(Am_PLACEHOLDER_REPLACE_METHOD, value_replace_placeholder)
          .Set(Am_PLACEHOLDER_SET_VALUE_METHOD, placeholder_value_from_slot)
          .Set(Am_PLACEHOLDER_CREATE_METHOD, all_values_placeholder_create);

  Am_An_Object_From_Command_Placeholder =
      Am_A_Placeholder.Create("Am_An_Object_From_Command_Placeholder")
          .Set(Am_PLACEHOLDER_REPLACE_METHOD, value_replace_placeholder)
          .Set(Am_PLACEHOLDER_SET_VALUE_METHOD, placeholder_value_from_slot)
          .Set(Am_PLACEHOLDER_CREATE_METHOD, an_object_placeholder_create);

  Am_The_Selected_Objects_Placeholder =
      Am_A_Placeholder.Create("Am_The_Selected_Objects_Placeholder")
          .Set(Am_PLACEHOLDER_REPLACE_METHOD, selected_objs_replace_placeholder)
          .Set(Am_PLACEHOLDER_SET_VALUE_METHOD, (0L))
          .Set(Am_PLACEHOLDER_CREATE_METHOD, selected_objs_placeholder_create);
  Am_User_Select_At_Run_Time_Placeholder =
      Am_A_Placeholder.Create("Am_User_Select_At_Run_Time_Placeholder")
          .Set(Am_PLACEHOLDER_REPLACE_METHOD,
               user_select_objs_replace_placeholder)
          .Set(Am_PLACEHOLDER_SET_VALUE_METHOD, (0L))
          .Set(Am_PLACEHOLDER_CREATE_METHOD, user_select_placeholder_create);
  Am_Inter_Location_Placeholder =
      Am_A_Placeholder.Create("Am_Inter_Location_Placeholder")
          .Set(Am_PLACEHOLDER_REPLACE_METHOD, location_replace_placeholder)
          .Set(Am_PLACEHOLDER_SET_VALUE_METHOD, (0L))
          .Set(Am_PLACEHOLDER_CREATE_METHOD, location_placeholder_create)
          .Add(Am_OLD_VALUE, (0L))
          .Add(Am_LEFT_HOW, (0L))
          .Add(Am_LEFT_AMOUNT_WIDGET, (0L))
          .Add(Am_TOP_HOW, (0L))
          .Add(Am_TOP_AMOUNT_WIDGET, (0L))
          .Add(Am_WIDTH_HOW, (0L))
          .Add(Am_WIDTH_AMOUNT_WIDGET, (0L))
          .Add(Am_HEIGHT_HOW, (0L))
          .Add(Am_HEIGHT_AMOUNT_WIDGET, (0L));

  Am_Ask_User_Property_Placeholder =
      Am_A_Placeholder.Create("Am_Ask_User_Property_Placeholder")
          .Set(Am_PLACEHOLDER_REPLACE_METHOD,
               ask_user_value_replace_placeholder)
          .Set(Am_PLACEHOLDER_SET_VALUE_METHOD, (0L))
          .Set(Am_PLACEHOLDER_CREATE_METHOD, ask_user_value_placeholder_create);
  Am_Property_From_Palette_Placeholder =
      Am_A_Placeholder.Create("Am_Property_From_Palette_Placeholder")
          .Set(Am_PLACEHOLDER_REPLACE_METHOD,
               property_from_palette_replace_placeholder)
          .Set(Am_PLACEHOLDER_SET_VALUE_METHOD, (0L))
          .Set(Am_PLACEHOLDER_CREATE_METHOD,
               property_from_palette_placeholder_create);

  Am_Cycle_Values_Placeholder =
      Am_A_Placeholder.Create("Am_Cycle_Values_Placeholder")
          .Set(Am_PLACEHOLDER_REPLACE_METHOD, cycle_values_replace_placeholder)
          .Set(Am_PLACEHOLDER_SET_VALUE_METHOD, (0L))
          .Set(Am_PLACEHOLDER_CREATE_METHOD, cycle_values_placeholder_create)
          .Add(Am_CURRENT_PHASE, 0);

  Am_Type_Match_Placeholder =
      Am_A_Placeholder.Create("Am_Type_Match_Placeholder")
          .Set(Am_PLACEHOLDER_REPLACE_METHOD, value_replace_placeholder)
          .Set(Am_PLACEHOLDER_SET_VALUE_METHOD, (0L))
          .Set(Am_PLACEHOLDER_CREATE_METHOD, type_match_placeholder_create)
          .Set(Am_PLACEHOLDER_MATCH_METHOD, type_do_match_placeholder);

  Am_Any_Value_Placeholder =
      Am_A_Placeholder.Create("Am_Any_Value_Placeholder")
          .Set(Am_PLACEHOLDER_REPLACE_METHOD, value_replace_placeholder)
          .Set(Am_PLACEHOLDER_SET_VALUE_METHOD, (0L))
          .Set(Am_PLACEHOLDER_CREATE_METHOD, any_value_match_placeholder_create)
          .Set(Am_PLACEHOLDER_MATCH_METHOD, any_value_do_match_placeholder);

  /* 
   Am_Custom_Code_Placeholder = Am_A_Placeholder.Create(DSTR("NIY"));
   Am_Property_From_Object_Placeholder;
   */
}
