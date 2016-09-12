/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file contains the command objects for the standard editing
   functions (cut copy, paste, etc.)
   
   Designed and implemented by Brad Myers
*/

#include <am_inc.h>
#include ANIM__H  // for Am_NO_ANIMATION

#include <amulet/impl/opal_obj2.h>
#include <amulet/impl/am_inter_location.h>
#include <amulet/impl/inter_undo.h>
#include <amulet/impl/widget_ed_methods.h>
#include <amulet/impl/widget_op.h>
#include <amulet/impl/wigdet_ed_op.h>
#include <amulet/impl/opal_load_save.h>

#include VALUE_LIST__H
#ifdef DEBUG
#include <amulet/impl/slots_registry.h>
#endif


#include <fstream>  //for load and save

#define IOS_FLAGS std::ios::out

//////////////////////////////////////////////////////////////////////////
// Global Variables
//////////////////////////////////////////////////////////////////////////

Am_Object Am_Global_Clipboard;

Am_Object Am_Graphics_Copy_Command;
Am_Object Am_Graphics_Cut_Command;
Am_Object Am_Graphics_Paste_Command;
Am_Object Am_Graphics_Clear_Command; //delete
Am_Object Am_Graphics_Clear_All_Command;
Am_Object Am_Graphics_Set_Property_Command; //for color, font, etc.
Am_Object Am_Undo_Command;
Am_Object Am_Redo_Command;
Am_Object Am_Graphics_To_Bottom_Command;
Am_Object Am_Graphics_To_Top_Command;
Am_Object Am_Graphics_Duplicate_Command;
Am_Object Am_Quit_No_Ask_Command;
Am_Object Am_Cycle_Value_Command;
Am_Object Am_Open_Command;
Am_Object Am_Save_Command;
Am_Object Am_Save_As_Command;

// **** TO DO ****
Am_Object Am_Quit_Command;
Am_Object Am_Undo_Dialog_Box_Command;
Am_Object Am_Graphics_Refresh_Command = 0;
Am_Object Am_Graphics_Group_Command = 0;
Am_Object Am_Graphics_Ungroup_Command = 0;
Am_Object Am_Pop_Up_Dialog_Box_Command = 0;
Am_Object Am_Font_Dialog_Box_Command;
Am_Object Am_Color_Dialog_Box_Command;
Am_Object Am_Print_Command;

const int DUPLICATE_OFFSET = 10; //offset for duplicate

///////////////////////////////////////////////////////////////////////////
// Useful utilities for commands
///////////////////////////////////////////////////////////////////////////

//copies the objects in a value list to make a new value_list
Am_Value_List Am_Copy_Object_List (Am_Value_List orig,
				   const Am_Object& ref_obj, int offset) {
  Am_Object obj, new_obj;
  Am_Value_List new_list;
  for (orig.Start(); !orig.Last(); orig.Next()) {
    obj = orig.Get();
    new_obj = obj.Copy();
    if (offset) {
      if (new_obj.Peek (Am_X1).Exists ()) {
        new_obj.Set(Am_X1, (int)new_obj.Get(Am_X1) + offset);
        new_obj.Set(Am_Y1, (int)new_obj.Get(Am_Y1) + offset);
        new_obj.Set(Am_X2, (int)new_obj.Get(Am_X2) + offset);
        new_obj.Set(Am_Y2, (int)new_obj.Get(Am_Y2) + offset);
      }
      else {
        new_obj.Set(Am_LEFT, (int)new_obj.Get(Am_LEFT) + offset);
        new_obj.Set(Am_TOP, (int)new_obj.Get(Am_TOP) + offset);
      }
    }
    if (ref_obj.Valid () && obj.Get_Owner () != ref_obj) {
      Am_Inter_Location location (new_obj);
      location.Set_Ref_Obj (obj.Get_Owner ());
      location.Translate_To (ref_obj);
      location.Install (new_obj);
    }
    new_list.Add(new_obj);
  }
  return new_list;
}

// Am_GV_Selection_Widget_For_Command replaced by Am_Get_Selection_Widget_For_Command

// this function supports multiple users
static Am_Object get_selection_widget (const Am_Value v)
{
  if (!(Am_Value_List::Test(v)))  
    return v;
  else {
    Am_Value last_user = Am_Screen.Get(Am_LAST_USER_ID);   
    Am_Value_List v_list = v;
    for (v_list.Start(); !v_list.Last(); v_list.Next()) {
       Am_Object item = v_list.Get();
       if (item.Valid() && item.Get(Am_USER_ID) == last_user) 
         return item;
     }
     return Am_No_Object;
   }
}

//gets the selection widget out of the command object, or if not
//there, then out of the Am_SAVED_OLD_OWNER widget

Am_Object Am_Get_Selection_Widget_For_Command(Am_Object cmd) {

  Am_Value v = cmd.Peek(Am_SELECTION_WIDGET);
  
  if (v.Valid()) return get_selection_widget(v);
  else {
    v = cmd.Get_Object(Am_SAVED_OLD_OWNER).Peek(Am_SELECTION_WIDGET);
    if (v.Valid())  return get_selection_widget(v);
    else return Am_No_Object;
  }
}

// ACTIVE if anything is selected
//   self is the command object
Am_Define_Formula(bool, Am_Active_If_Selection) {
  Am_Object sel_widget = Am_Get_Selection_Widget_For_Command(self);
  Am_Value_List selection = sel_widget.Get(Am_VALUE);
  if (!selection.Empty()) {
    return Am_Check_All_Objects_For_Inactive_Slot(selection, self);
  }
  else return false;
}

Am_Value_List Am_Sort_Obs_In_Group(Am_Value_List unsorted_sel_objs,
				   Am_Object group) {
  Am_Value_List sorted, group_parts;
  group_parts = group.Get(Am_GRAPHICAL_PARTS);
  //std::cout << "Sorting list " << unsorted_sel_objs << " with respect to group "
  //     << group << " groups parts " << group_parts <<std::endl <<std::flush;
  Am_Object part;
  Am_Object selected;
  for (group_parts.Start(); !group_parts.Last(); group_parts.Next()) {
    part = group_parts.Get();
    for (unsorted_sel_objs.Start(); !unsorted_sel_objs.Last();
	 unsorted_sel_objs.Next()) {
      selected = unsorted_sel_objs.Get();
      if (selected == part) {
        unsorted_sel_objs.Delete();
        sorted.Add(part);
        break;
      }
      else if (selected.Is_Part_Of (part) && sorted.Empty ())
        return Am_Sort_Obs_In_Group (unsorted_sel_objs, part);
    }
  }
  if (!unsorted_sel_objs.Empty()) {
    Am_ERRORO("Selected parts " << unsorted_sel_objs
	      << " not in group " << group, group, Am_GRAPHICAL_PARTS);
  }
  //std::cout << " sorted result = " << sorted <<std::endl <<std::flush;
  return sorted;
}

//Return the list of selected objects sorted by their display order, along
//a list of their places so they could be put back if necessary.  Also
//returns the group
void Am_Get_Selection_In_Display_Order(Am_Object selection_widget,
				       Am_Value_List &selected_objs,
				       Am_Object &group) {
  Am_Value_List unsorted_sel_objs;
  unsorted_sel_objs = selection_widget.Get(Am_VALUE);
  group = selection_widget.Get(Am_OPERATES_ON);
  selected_objs = Am_Sort_Obs_In_Group(unsorted_sel_objs, group);
}

void Am_Get_Sel_Or_Param_In_Display_Order(Am_Object selection_widget,
					  Am_Value_List &selected_objs,
					  Am_Object &group,
					  Am_Value new_sel) {
  if (new_sel.Valid()) {
    group = selection_widget.Get(Am_OPERATES_ON);
    if (new_sel.type == Am_OBJECT) {
      selected_objs.Make_Empty();
      selected_objs.Add(new_sel);
    }
    else if (Am_Value_List::Test(new_sel)) {
      Am_Value_List l = new_sel;
      selected_objs = Am_Sort_Obs_In_Group(l, group);
    }
    else Am_ERROR("Specified new value for repeat is not object or list: "
		  << new_sel);
  }
  else Am_Get_Selection_In_Display_Order(selection_widget,
					 selected_objs, group);
}


Am_Object Am_Find_Part_Place(Am_Object obj, Am_Object group) {
  Am_Value_List group_parts;
  group_parts = group.Get(Am_GRAPHICAL_PARTS);
  Am_Object part;
  for(group_parts.Start(); !group_parts.Last(); group_parts.Next()) {
    part = group_parts.Get();
    if (part == obj) {
      group_parts.Next();
      if (group_parts.Last()) return Am_No_Object;
      else {
	part = group_parts.Get();
	return part;
      }
    }
  }
  return Am_No_Object;
}

///////////////////////////////////////////////////////////////////////////
// Property Change
///////////////////////////////////////////////////////////////////////////

/* Property changing:
   Gets the old value out of the objects using the
   Am_GET_OBJECT_VALUE_METHOD.  Default just accesses the
   slot defined in Am_SLOT_FOR_VALUE.  The selected objects are set
   using the Am_SET_OBJECT_VALUE_METHOD.  The default just sets the
   Am_SLOT_FOR_VALUE slot.  Gets the value out of the
   widget using the Am_GET_WIDGET_VALUE_METHOD.  Default method looks
   at the value of the widget.  If the value is an object, then gets
   its Am_SLOT_FOR_VALUE.  This is useful, for example, if the widget
   contains objects which SHOW the property.  If not an object, then
   the value of the widget is used.

   Iterates through all the selected objects.  If an object has the
   Am_CREATED_GROUP slot = true, then recursively operates on its sub-parts

   Another slot you need to set in the command object is the
   Am_SELECTION_WIDGET slot.
*/

AM_DEFINE_METHOD_TYPE_IMPL(Am_Get_Widget_Property_Value_Method);
AM_DEFINE_METHOD_TYPE_IMPL(Am_Get_Object_Property_Value_Method);
AM_DEFINE_METHOD_TYPE_IMPL(Am_Set_Object_Property_Value_Method);

AM_DEFINE_METHOD_TYPE_IMPL(Am_Handle_Loaded_Items_Method);
AM_DEFINE_METHOD_TYPE_IMPL(Am_Items_To_Save_Method);

//if any objs in list are Am_CREATED_GROUP, then replaces them with
//the graphical parts (recursively)
void get_all_sub_parts(Am_Value_List &objs) {
  Am_Object obj;
  Am_Value is_group;
  Am_Value_List sub_parts;
  for (objs.Start(); !objs.Last(); objs.Next()) {
    obj = objs.Get();
    is_group=obj.Peek(Am_CREATED_GROUP);
    if (is_group.Valid()) {
      objs.Delete();
      sub_parts = obj.Get(Am_GRAPHICAL_PARTS);
      objs.Append(sub_parts);
    }
  }
}

Am_Define_Method(Am_Object_Method, void, set_property, (Am_Object cmd)) {
  Am_Get_Widget_Property_Value_Method get_widget_val_method;
  Am_Get_Object_Property_Value_Method get_obj_val_method;
  Am_Set_Object_Property_Value_Method set_obj_val_method;
  Am_Value new_value, old_value;
  Am_Value_List current_selection, old_values;
  Am_Object selection_widget, obj;

  get_widget_val_method = cmd.Get(Am_GET_WIDGET_VALUE_METHOD);
  get_obj_val_method = cmd.Get(Am_GET_OBJECT_VALUE_METHOD);
  set_obj_val_method = cmd.Get(Am_SET_OBJECT_VALUE_METHOD);
  
  get_widget_val_method.Call(cmd, new_value);
  selection_widget = Am_Get_Selection_Widget_For_Command(cmd);
  current_selection = selection_widget.Get(Am_VALUE);
  get_all_sub_parts(current_selection);
  cmd.Set(Am_OBJECT_MODIFIED, current_selection);
  if (current_selection.Length() > 0) {
    for (current_selection.Start(); !current_selection.Last();
	 current_selection.Next()) {
      obj = current_selection.Get();
      get_obj_val_method.Call(cmd, obj, old_value);
      set_obj_val_method.Call(cmd, obj, new_value);
      old_values.Add(old_value);
    }
    cmd.Set(Am_VALUE, new_value);
    cmd.Set(Am_OLD_VALUE, old_values);
    cmd.Set(Am_IMPLEMENTATION_PARENT, 0);
  }
  else // change property widget without changing anything
    cmd.Set(Am_IMPLEMENTATION_PARENT, Am_NOT_USUALLY_UNDONE);
}

void update_objs_from_val(Am_Object cmd, Am_Value objs_value,
			  Am_Value new_data_value) { 
  //std::cout << "update objs " << objs_value  << " from " << new_data_value
  //     <<std::endl <<std::flush; 
  Am_Value_List value_list, obj_list;
  Am_Object obj;
  bool value_is_list = false;
  Am_Set_Object_Property_Value_Method set_obj_val_method;
  set_obj_val_method = cmd.Get(Am_SET_OBJECT_VALUE_METHOD);
  if (Am_Value_List::Test(new_data_value)) {
    value_is_list = true;
    value_list = new_data_value;
    value_list.Start();
  }
  obj_list = objs_value;
  for(obj_list.Start(); !obj_list.Last(); obj_list.Next()) {
    obj = obj_list.Get();
    if (value_is_list) {
      new_data_value = value_list.Get();
      value_list.Next();
    } // else new_data_value is already set
    set_obj_val_method.Call(cmd, obj, new_data_value);
  }
}

void update_val_from_objs(Am_Object cmd, Am_Value objs_value,
			  Am_Value_List &new_list) {
  Am_Get_Object_Property_Value_Method get_obj_val_method;
  Am_Value_List value_list, obj_list;
  Am_Value old_value;
  Am_Object obj;
  get_obj_val_method = cmd.Get(Am_GET_OBJECT_VALUE_METHOD);
  obj_list = objs_value;
  value_list.Start();
  for(obj_list.Start(); !obj_list.Last(); obj_list.Next()) {
    obj = obj_list.Get();
    get_obj_val_method.Call(cmd, obj, old_value);
    value_list.Add(old_value);
  }
  //std::cout << "update val for " << objs_value << " to " << value_list 
  // <<std::endl <<std::flush; 
  new_list = value_list;
}

void change_property_general_undo_redo(Am_Object command_obj, bool undo,
				       bool selective, bool reload_data,
				       Am_Value new_sel = Am_No_Value,
				       Am_Value new_value = Am_No_Value) {
  Am_Value v;
  v=command_obj.Peek(Am_OLD_VALUE);
  //std::cout << "for command " << command_obj << " old = " << v <<std::endl <<std::flush;
  if (reload_data && new_sel.Valid()) {
    command_obj.Set(Am_OBJECT_MODIFIED, new_sel);
  }
  else new_sel=command_obj.Peek(Am_OBJECT_MODIFIED);

  if (new_sel.Valid ()) { //then have something to operate on
    Am_Value old_data_value, new_data_value;
    //new will hold the value to use now
    //old will hold the value to store into the old-value of the command
    if (selective) {
      Am_Value_List old_data_list;
      update_val_from_objs(command_obj, new_sel, old_data_list);
      //std::cout << "Selective; read obj value to get " << old_data_list
      // <<std::endl <<std::flush;
      old_data_value = old_data_list;
    }
    else old_data_value=command_obj.Peek(Am_VALUE);
    if (undo) new_data_value = command_obj.Peek(Am_OLD_VALUE);
    else if (reload_data && new_value.Valid())
      new_data_value = new_value;
    else new_data_value = command_obj.Peek(Am_VALUE);

    //std::cout << "--Change Prop " << command_obj << " on " << new_sel
    //      << " undo " << undo << " selective " << selective
    //      << " new value " << new_data_value << " old " << old_data_value
    //      <<std::endl <<std::flush;

    update_objs_from_val(command_obj, new_sel, new_data_value);
    command_obj.Set(Am_OLD_VALUE, old_data_value);
    command_obj.Set(Am_VALUE, new_data_value);
  }
}
Am_Define_Method(Am_Object_Method, void, property_undo,
		 (Am_Object command_obj)) {
  change_property_general_undo_redo(command_obj, true, false, false);
}
Am_Define_Method(Am_Object_Method, void, property_sel_undo,
		 (Am_Object command_obj)){
  change_property_general_undo_redo(command_obj, true, true, false);
}
Am_Define_Method(Am_Object_Method, void, property_sel_repeat,
		 (Am_Object command_obj)){
  change_property_general_undo_redo(command_obj, false, true, false);
}
Am_Define_Method(Am_Selective_Repeat_New_Method, void,
		 property_sel_repeat_new,
		 (Am_Object command_obj, Am_Value new_sel,
		  Am_Value new_value)){
  change_property_general_undo_redo(command_obj, false, true, true, new_sel,
				    new_value);
}

Am_Define_Method(Am_Get_Widget_Property_Value_Method, void,
		 default_widget_value_method,
		 (Am_Object command_obj, Am_Value &new_value)) {
  Am_Object widget, widget_value_obj;
  widget = command_obj.Get(Am_SAVED_OLD_OWNER);
  Am_Value val;
  val=widget.Peek(Am_VALUE);
  if (val.type == Am_OBJECT) {
    widget_value_obj = val;
    Am_Slot_Key slot;
    slot = (int)command_obj.Get(Am_SLOT_FOR_VALUE);
    new_value=widget_value_obj.Peek(slot);
  }
  else Am_Error("Widget's value is not an object");
}

Am_Define_Method(Am_Get_Object_Property_Value_Method, void,
		 default_get_object_value_method,
	 (Am_Object command_obj, Am_Object obj, Am_Value &old_value)) {
  Am_Slot_Key slot;
  slot = (int)command_obj.Get(Am_SLOT_FOR_VALUE);
  old_value=obj.Peek(slot);
}
Am_Define_Method(Am_Set_Object_Property_Value_Method, void,
		 default_set_object_value_method,
	 (Am_Object command_obj, Am_Object obj, Am_Value new_value)) {
  Am_Slot_Key slot;
  slot = (int)command_obj.Get(Am_SLOT_FOR_VALUE);
#ifdef DEBUG
  if (Am_Inter_Tracing(Am_INTER_TRACE_SETTING)) {
    const char * name = Am_Get_Slot_Name(slot);
    std::cout << "++ Am_Graphics_Set_Property_Command " << command_obj
	 << " setting the ";
    if (name) std::cout << name;
    else std::cout << slot;
    std::cout << " of " << obj << " to " << new_value <<std::endl << std::flush;
  }
#endif

  obj.Set(slot, new_value);
}

///////////////////////////////////////////////////////////////////////////
// Clear (Delete) and Clear All and Cut
///////////////////////////////////////////////////////////////////////////

/* Clear (Delete):
   Deletes the currently selected objects.  Undo puts them back where
   they were (in the correct place in the group.)  Selective undo just
   puts them back at the top of the group (assumes that the old
   positions won't be valid anymore).  In either case, the objects
   become selected (by setting the Am_VALUE of the Am_SELECTION_WIDGET).
   Selective Repeat deletes the objects again, and repeat-on-new deletes
   the currently-selected objects.

   Slot to set in the command object: Am_SELECTION_WIDGET
   (Gets the group out of the Am_OPERATES_ON slot of the Am_SELECTION_WIDGET)
*/

/* Clear All (Delete All):
   Deletes all the objects.  Undo puts them back where
   they were (in the correct place in the group.) and restores the old
   selection.  Selective undo just
   puts them back at the top of the group (assumes that the old
   positions won't be valid anymore).  
   Selective Repeat deletes the objects again, and repeat-on-new deletes
   all the current objects.

   Slot to set in the command object: Am_SELECTION_WIDGET
   (Gets the group out of the Am_OPERATES_ON slot of the Am_SELECTION_WIDGET)
*/

/* Cut: A lot like Delete, but the deleted objects go into the clipboard.
   Deletes the currently selected objects, removes the current
   contents of the clipboard, and puts the deleted objects as the
   value of the clipboard.  Undo puts the objects back where
   they were in the correct place in the group, and restores the
   clipboard to its former value.  Selective undo just
   puts the objects back at the top of the group (assumes that the old
   positions won't be valid anymore), and does NOT affect the
   clipboard.  In either case, the objects become selected (by setting
   the Am_VALUE of the Am_SELECTION_WIDGET).
   Selective Repeat deletes the objects again, and repeat-on-new deletes
   the currently-selected objects.  Repeat does set the clipboard.

   Slots to set in the command object: Am_SELECTION_WIDGET
   Am_CLIPBOARD: the clipboard to use.  If (0L), uses Am_Global_Clipboard
   (Gets the group out of the Am_OPERATES_ON slot of the Am_SELECTION_WIDGET)
*/

void copy_objects_into_clipboard(Am_Object cmd, Am_Value_List selected_objs) {
  Am_Object clipboard;
  Am_Value_List old_clipboard_objs, copy_selected_objs;
  Am_Object sel_widget = Am_Get_Selection_Widget_For_Command(cmd);
  Am_Object ref_obj = sel_widget.Get_Object (Am_OPERATES_ON);
  copy_selected_objs = Am_Copy_Object_List(selected_objs, ref_obj);
  clipboard = cmd.Get(Am_CLIPBOARD);
  if (!clipboard.Valid()) clipboard = Am_Global_Clipboard;
  //std::cout << "setting clipboard " << clipboard << " with objects "
  //  << copy_selected_objs <<std::endl <<std::flush;
  old_clipboard_objs = clipboard.Get(Am_VALUE);
  clipboard.Set(Am_VALUE, copy_selected_objs);
  cmd.Set(Am_OLD_CLIPBOARD_OBJECTS, old_clipboard_objs);
}

void delete_objects_internal(Am_Object& cmd, Am_Value new_sel) {
  Am_Object selection_widget, group, obj, place;
  Am_Value_List selected_objs, selected_objs_places;
  selection_widget = Am_Get_Selection_Widget_For_Command(cmd);
  Am_Get_Sel_Or_Param_In_Display_Order(selection_widget, selected_objs, group,
				       new_sel);
  cmd.Set(Am_OLD_VALUE, Am_No_Object);
  cmd.Set(Am_OBJECT_MODIFIED, selected_objs);
  cmd.Set(Am_SAVED_OLD_OBJECT_OWNER, group);
  cmd.Set(Am_HAS_BEEN_UNDONE, false);
  Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING, "++ `Deleting' " <<
		       selected_objs << " by removing them from " << group);

  for(selected_objs.Start(); !selected_objs.Last(); selected_objs.Next()) {
    obj = selected_objs.Get();
    place = Am_Find_Part_Place(obj, group);
    group.Remove_Part(obj);
    // std::cout << "Removing object " << obj << " place " << place <<std::endl <<std::flush;
    selected_objs_places.Add(place);
  }
  cmd.Set(Am_OBJECT_MODIFIED_PLACES, selected_objs_places, Am_OK_IF_NOT_THERE);
  cmd.Set(Am_VALUE, selected_objs_places.Length()); //something to set for
						    //the value; not used
  selection_widget.Set(Am_VALUE, Am_Value_List()); //no selection
}

Am_Define_Method(Am_Object_Method, void, delete_objects, (Am_Object cmd)) {
  delete_objects_internal(cmd, Am_No_Value);
}  


//not very efficient, but get selection to save it, then set selection
//to all objects and call the regular clear command
Am_Define_Method(Am_Object_Method, void, delete_all_objects,(Am_Object cmd)) {
  Am_Object selection_widget, group;
  Am_Value_List selected_objs, all_objs;
  selection_widget = Am_Get_Selection_Widget_For_Command(cmd);
  Am_Get_Selection_In_Display_Order(selection_widget, selected_objs, group);
  all_objs = group.Get(Am_GRAPHICAL_PARTS);
  selection_widget.Set(Am_VALUE, all_objs);
  delete_objects_internal(cmd, Am_No_Value);
  cmd.Set(Am_OLD_VALUE, selected_objs);
}

void cut_objects_internal(Am_Object& cmd, Am_Value new_sel) {
  Am_Value_List selected_objs;
  delete_objects_internal(cmd, new_sel);  //delete the objects from the screen
  selected_objs = cmd.Get(Am_OBJECT_MODIFIED); //set by delete_objects_proc
  copy_objects_into_clipboard(cmd, selected_objs);
}

Am_Define_Method(Am_Object_Method, void, cut_objects, (Am_Object cmd)) {
  cut_objects_internal(cmd, Am_No_Value);
}

//if add_part, then put objects back into the group.  If
//add_part==false, then undo of to-top or bottom, so objects already
//in the group
void Am_Put_Objects_Back_In_Place(Am_Object group, Am_Value_List objects,
				  Am_Value_List places, bool add_part) {
  //std::cout << "Put objects back into group " << group << " objects " << objects
  // << " places " << places <<std::endl <<std::flush;
  if (objects.Length() != places.Length()) 
    Am_Error("** Lists not the same length in put_back");
  //go through the lists backwards
  Am_Object object, place;
  Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING, "++ Adding parts " << objects
		       << " to " << group);
  for(objects.End(), places.End(); !objects.First();
      objects.Prev(), places.Prev()) {
    object = objects.Get();
    place = places.Get();
    if (add_part && !object.Get_Owner ().Valid ()) group.Add_Part(object);
    if (place.Valid() && place.Get_Owner () == object.Get_Owner ())
      Am_Move_Object(object, place, false); //put object before place
    else Am_To_Top(object);
  }
}

void Am_Put_Objects_Into_Group(Am_Object group, Am_Value_List objects) {
  Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING, "++ Adding parts " << objects
		       << " to " << group);
  Am_Object object;
  for(objects.Start(); !objects.Last(); objects.Next()) {
    object = objects.Get();
    if (!object.Get_Owner ().Valid ())
      group.Add_Part(object);
  }
}

void delete_these_objects(Am_Object group, Am_Value_List deleted_objs) {
  Am_Object obj;
  Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING, "++ `Deleting' objects " <<
		       deleted_objs << " by removing them from " << group);
  for(deleted_objs.Start(); !deleted_objs.Last(); deleted_objs.Next()) {
    obj = deleted_objs.Get();
    group.Remove_Part(obj);
  }
}

void restore_old_clipboard_objs(Am_Object command_obj) {
  Am_Object clipboard;
  Am_Value_List old_clipboard_objs;
  clipboard = command_obj.Get(Am_CLIPBOARD);
  if (!clipboard.Valid()) clipboard = Am_Global_Clipboard;
  old_clipboard_objs = command_obj.Get(Am_OLD_CLIPBOARD_OBJECTS);
  clipboard.Set(Am_VALUE, old_clipboard_objs);
}

//undo_clipboard should be true for CUT and false for CLEAR (delete)
void delete_general_undo_redo(Am_Object command_obj,
			      bool selective,
			      bool undo_clipboard) {
  Am_Object group, selection_widget;
  Am_Value_List deleted_objs, places, old_selection;
  selection_widget = Am_Get_Selection_Widget_For_Command(command_obj);
  group = command_obj.Get(Am_SAVED_OLD_OBJECT_OWNER);
  deleted_objs = command_obj.Get(Am_OBJECT_MODIFIED);
  if (!selective)
    places = command_obj.Get(Am_OBJECT_MODIFIED_PLACES);
  bool currently_undone = command_obj.Get(Am_HAS_BEEN_UNDONE);

  if (currently_undone) { // then redo = re-delete the
			    // objects.  Places should still be valid
    if (selective)
      Am_Error("** Shouldn't be possible to redo if selective");
    
    delete_these_objects(group, deleted_objs);
    if (undo_clipboard)
      copy_objects_into_clipboard(command_obj, deleted_objs);
  }
  else { // undo = add objects back
    //std::cout << "undo, selective = " << selective << " objs "
    //      << deleted_objs <<std::endl <<std::flush;
    if (!selective) {
      Am_Put_Objects_Back_In_Place(group, deleted_objs, places, true);
      if (undo_clipboard)
	restore_old_clipboard_objs(command_obj);
    }
    else Am_Put_Objects_Into_Group(group, deleted_objs);
    //selective undo doesn't reset the clipboard

    //now set the selection.  If clear_all, then OLD_VALUE will be
    //set with the original selection.  Otherwise, just use the
    //deleted objects.
    Am_Value old_sel_val;
    old_sel_val=command_obj.Peek(Am_OLD_VALUE);
    if (old_sel_val.Valid()) old_selection = old_sel_val;
    else old_selection = deleted_objs;
    selection_widget.Set(Am_VALUE, old_selection);
  }
  command_obj.Set(Am_HAS_BEEN_UNDONE, !currently_undone);
}

Am_Define_Method(Am_Object_Method, void, delete_objects_undo,
		 (Am_Object command_obj)) {
  delete_general_undo_redo(command_obj, false, false);
}
Am_Define_Method(Am_Object_Method, void, delete_objects_sel_undo,
		 (Am_Object command_obj)){
  delete_general_undo_redo(command_obj, true, false);
}
Am_Define_Method(Am_Object_Method, void, delete_objects_sel_repeat,
		 (Am_Object command_obj)){
  delete_objects_internal(command_obj, Am_No_Value);
}
Am_Define_Method(Am_Selective_Repeat_New_Method, void,
		 delete_objects_sel_repeat_new,
		 (Am_Object command_obj, Am_Value new_sel,
		  Am_Value /* new_value */)){
  delete_objects_internal(command_obj, new_sel);
}

Am_Define_Method(Am_Object_Method, void, cut_objects_undo,
		 (Am_Object command_obj)) {
  delete_general_undo_redo(command_obj, false, true);
}
Am_Define_Method(Am_Object_Method, void, cut_objects_sel_undo,
		 (Am_Object command_obj)){
  delete_general_undo_redo(command_obj, true, true);
}
Am_Define_Method(Am_Object_Method, void, cut_objects_sel_repeat,
		 (Am_Object command_obj)){
  cut_objects_internal(command_obj, Am_No_Value);
}
Am_Define_Method(Am_Selective_Repeat_New_Method, void,
		 cut_objects_sel_repeat_new,
		 (Am_Object command_obj, Am_Value new_sel,
		  Am_Value /* new_value */)){
  cut_objects_internal(command_obj, new_sel);
}

// need the selective allowed methods because they are the opposite of
// usual: can undo if NOT there
Am_Define_Method(Am_Selective_Allowed_Method, bool,
		 Am_Delete_Selective_Undo_Allowed, (Am_Object command_obj)) {
  if (!command_obj.Valid()) return false;
  Am_Value value;
  value=command_obj.Peek(Am_OBJECT_MODIFIED, Am_NO_DEPENDENCY);
  bool ret = Am_Valid_and_Visible_List_Or_Object(value);
  //std::cout << "*** valid and vis returns " << ret << " for obj mod " << value
  //      << " in command " << command_obj <<std::endl <<std::flush;
  return !ret;
}

//return true if there are any objects in the group
Am_Define_Formula(bool, active_if_any) {
  Am_Object selection_widget = Am_Get_Selection_Widget_For_Command(self);
  if (selection_widget.Valid()) {
    Am_Value_List all_parts = selection_widget.Get_Object(Am_OPERATES_ON)
      .Get(Am_GRAPHICAL_PARTS);
    if (all_parts.Valid() && !all_parts.Empty()) {
      return Am_Check_All_Objects_For_Inactive_Slot(all_parts, self);
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////
// Copy
///////////////////////////////////////////////////////////////////////////

/* Copy
   Removes the current contents of the clipboard, and puts a copy of the
   selected objects as the value of the clipboard.  Undo restores the
   clipboard to its former value.  Selective undo is not allowed.
   Selective Repeat copies the objects again, and repeat-on-new just copies
   the currently-selected objects.

   Slots to set in the command object: Am_SELECTION_WIDGET
   Am_CLIPBOARD: the clipboard to use.  If (0L), uses Am_Global_Clipboard
   (Gets the group out of the Am_OPERATES_ON slot of the Am_SELECTION_WIDGET)
*/

void copy_objects_internal(Am_Object &cmd, Am_Value new_sel) {
  Am_Object selection_widget, group;
  Am_Value_List selected_objs;
  selection_widget = Am_Get_Selection_Widget_For_Command(cmd);
  Am_Get_Sel_Or_Param_In_Display_Order(selection_widget, selected_objs, group,
				       new_sel);
  cmd.Set(Am_OBJECT_MODIFIED, selected_objs);
  cmd.Set(Am_HAS_BEEN_UNDONE, false);
  cmd.Set(Am_SAVED_OLD_OBJECT_OWNER, group);
  copy_objects_into_clipboard(cmd, selected_objs);
  cmd.Set(Am_VALUE, selected_objs.Length()); //not used
}

Am_Define_Method(Am_Object_Method, void, copy_objects, (Am_Object cmd)) {
  copy_objects_internal(cmd, Am_No_Value);
}

Am_Define_Method(Am_Object_Method, void, copy_objects_undo,
		 (Am_Object command_obj)) {
  Am_Object selection_widget;
  Am_Value_List copied_objs;
  selection_widget = Am_Get_Selection_Widget_For_Command(command_obj);
  copied_objs = command_obj.Get(Am_OBJECT_MODIFIED);
  bool currently_undone = command_obj.Get(Am_HAS_BEEN_UNDONE);
  
  if (currently_undone) { // then redo = reset the clipboard
    copy_objects_into_clipboard(command_obj, copied_objs);
  }
  else // restore clipboard
    restore_old_clipboard_objs(command_obj);
  command_obj.Set(Am_HAS_BEEN_UNDONE, !currently_undone);
}

//selective undo not supported for copy

Am_Define_Method(Am_Object_Method, void, copy_objects_sel_repeat,
		 (Am_Object command_obj)){
  copy_objects_internal(command_obj, Am_No_Value);
}
Am_Define_Method(Am_Selective_Repeat_New_Method, void,
		 copy_objects_sel_repeat_new,
		 (Am_Object command_obj, Am_Value new_sel,
		  Am_Value /* new_value */)){
  copy_objects_internal(command_obj, new_sel);
}

///////////////////////////////////////////////////////////////////////////
// Paste
///////////////////////////////////////////////////////////////////////////

/* Paste
   Takes the current objects in the clipboard and puts a copy of them into the
   current group on top.  Selects the new objects.
   Undo removes the objects and restores the old selection.  Selective undo
   removes the objects, but doesn't change the selection.
   Selective Repeat pastes the objects again.  Repeat on new is not allowed.

   Slots to set in the command object: Am_SELECTION_WIDGET
   Am_CLIPBOARD: the clipboard to use.  If (0L), uses Am_Global_Clipboard.
   (Gets the group out of the Am_OPERATES_ON slot of the Am_SELECTION_WIDGET)
*/

Am_Define_Method(Am_Object_Method, void, paste_objects, (Am_Object cmd)) {
  Am_Object selection_widget, group;
  Am_Value_List clipboard_objs, copy_clipboard_objs, old_selection;
  Am_Object clipboard;
  clipboard = cmd.Get(Am_CLIPBOARD);
  if (!clipboard.Valid()) clipboard = Am_Global_Clipboard;
  selection_widget = Am_Get_Selection_Widget_For_Command(cmd);
  group = selection_widget.Get(Am_OPERATES_ON);
  clipboard_objs = clipboard.Get(Am_VALUE);
  copy_clipboard_objs = Am_Copy_Object_List(clipboard_objs);
  cmd.Set(Am_OBJECT_MODIFIED, Am_No_Object); //not used
  cmd.Set(Am_HAS_BEEN_UNDONE, false);
  Am_Put_Objects_Into_Group(group, copy_clipboard_objs);
  old_selection = selection_widget.Get(Am_VALUE);
  cmd.Set(Am_OLD_VALUE, old_selection);
  cmd.Set(Am_VALUE, copy_clipboard_objs);
  cmd.Set(Am_SAVED_OLD_OBJECT_OWNER, group);
  selection_widget.Set(Am_VALUE, copy_clipboard_objs);
}

void paste_duplicate_undo_branch(Am_Object command_obj, bool selective) {
  Am_Object selection_widget, group;
  Am_Value_List new_objs;
  selection_widget = Am_Get_Selection_Widget_For_Command(command_obj);
  new_objs = command_obj.Get(Am_VALUE);
  bool currently_undone = command_obj.Get(Am_HAS_BEEN_UNDONE);
  group = command_obj.Get(Am_SAVED_OLD_OBJECT_OWNER);
  
  if (currently_undone) { // then redo = put objects back in and
			    // select them
    Am_Put_Objects_Into_Group(group, new_objs);
    selection_widget.Set(Am_VALUE, new_objs);
  }
  else { // remove the objects, restore selection if not selective
    delete_these_objects(group, new_objs);
    if (!selective) {
      Am_Value_List old_selection;
      old_selection = command_obj.Get(Am_OLD_VALUE);
      selection_widget.Set(Am_VALUE, old_selection);
    }
  }
  command_obj.Set(Am_HAS_BEEN_UNDONE, !currently_undone);
}

Am_Define_Method(Am_Object_Method, void, paste_objects_undo,
		 (Am_Object command_obj)) {
  paste_duplicate_undo_branch(command_obj, false);
}
Am_Define_Method(Am_Object_Method, void, paste_objects_sel_undo,
		 (Am_Object command_obj)){
  paste_duplicate_undo_branch(command_obj, true);
}
Am_Define_Method(Am_Object_Method, void, paste_objects_sel_repeat,
		 (Am_Object command_obj)){
  paste_objects_proc(command_obj);
}

// ACTIVE if there is anything in the clipboard
//   self is the command object
Am_Define_Formula(bool, active_if_clipboard) {
  Am_Object clipboard;
  Am_Value_List clipboard_value;
  clipboard = self.Get(Am_CLIPBOARD);
  if (!clipboard.Valid()) clipboard = Am_Global_Clipboard;
  if (clipboard.Valid()) {
    clipboard_value = clipboard.Get(Am_VALUE);
    return (!clipboard_value.Empty());
  }
  else return false;
}

///////////////////////////////////////////////////////////////////////////
// Duplicate
///////////////////////////////////////////////////////////////////////////

/* Duplicate
   Takes the currently selected objects and makes a copy of them and puts
   the copies into the current group on top.  Selects the new objects.
   Undo removes the objects and restores the old selection.  Selective undo
   removes the objects, but doesn't change the selection.
   Selective Repeat creates a new set of objects.
   Repeat on new makes a duplicate of the currently selected objects.

   Slots to set in the command object: Am_SELECTION_WIDGET
   (Gets the group out of the Am_OPERATES_ON slot of the Am_SELECTION_WIDGET)
*/
Am_Define_Method(Am_Object_Method, void, duplicate_objects, (Am_Object cmd)) {
  Am_Object selection_widget, group;
  Am_Value_List selected_objs, copy_objs, copy_copy_objs, old_selection;
  selection_widget = Am_Get_Selection_Widget_For_Command(cmd);
  Am_Get_Selection_In_Display_Order(selection_widget, selected_objs, group);
  cmd.Set(Am_HAS_BEEN_UNDONE, false);
  copy_objs = Am_Copy_Object_List(selected_objs, group, DUPLICATE_OFFSET);
  Am_Put_Objects_Into_Group(group, copy_objs);
  cmd.Set(Am_OBJECT_MODIFIED, selected_objs); //objects copied from
  copy_copy_objs = Am_Copy_Object_List(selected_objs, group);
  cmd.Set(Am_START_OBJECT, copy_copy_objs);	//in case repeat
  old_selection = selection_widget.Get(Am_VALUE);
  cmd.Set(Am_OLD_VALUE, old_selection);
  cmd.Set(Am_SAVED_OLD_OBJECT_OWNER, group);
  selection_widget.Set(Am_VALUE, copy_objs);
  cmd.Set(Am_VALUE, copy_objs);
}

void duplicate_general_repeat(Am_Object command_obj, bool reload_data,
			      Am_Value new_sel) {
  // repeat = duplicate a new copy of the objects
  Am_Object selection_widget, group;
  Am_Value_List orig_objs, copy_objs, old_selection;
  selection_widget = Am_Get_Selection_Widget_For_Command(command_obj);
  if (reload_data) {
    Am_Value_List copy_copy_objs;
    Am_Get_Sel_Or_Param_In_Display_Order(selection_widget, orig_objs, group,
					 new_sel);
    command_obj.Set(Am_OBJECT_MODIFIED, new_sel);
    copy_copy_objs = Am_Copy_Object_List(orig_objs);
    command_obj.Set(Am_START_OBJECT, copy_copy_objs); //in case repeat
  }
  else {
    orig_objs = command_obj.Get(Am_START_OBJECT);
    group = command_obj.Get(Am_SAVED_OLD_OBJECT_OWNER);
  }
  copy_objs = Am_Copy_Object_List(orig_objs, group, DUPLICATE_OFFSET);
  command_obj.Set(Am_HAS_BEEN_UNDONE, false);
  Am_Put_Objects_Into_Group(group, copy_objs);
  command_obj.Set(Am_VALUE, copy_objs);
  old_selection = selection_widget.Get(Am_VALUE);
  command_obj.Set(Am_OLD_VALUE, old_selection);
  selection_widget.Set(Am_VALUE, copy_objs);
}

Am_Define_Method(Am_Object_Method, void, duplicate_objects_undo,
		 (Am_Object command_obj)) {
  paste_duplicate_undo_branch(command_obj, false);
}
Am_Define_Method(Am_Object_Method, void, duplicate_objects_sel_undo,
		 (Am_Object command_obj)){
  paste_duplicate_undo_branch(command_obj, true);
}
Am_Define_Method(Am_Object_Method, void, duplicate_objects_sel_repeat,
		 (Am_Object command_obj)){
  duplicate_general_repeat(command_obj, false, Am_No_Value);
}
Am_Define_Method(Am_Selective_Repeat_New_Method, void,
		 duplicate_objects_sel_repeat_new,
		 (Am_Object command_obj, Am_Value new_sel,
		  Am_Value /* new_value */)){
  duplicate_general_repeat(command_obj, true, new_sel);
}

Am_Define_Method(Am_Selective_New_Allowed_Method, bool,
		 new_sel_valid,
		 (Am_Object /* command_obj */, Am_Value new_selection,
		  Am_Value /* new_value */)) {
  return new_selection.Valid();
}


///////////////////////////////////////////////////////////////////////////
// Group
///////////////////////////////////////////////////////////////////////////

/* Group
   Takes the selected objects and puts them in a group, then puts that
   group in place of the selected objects.  Undo removes the group and
   puts the objects back where they were (in the correct place.)
   Selective undo just puts them back at the top of the original group
   (assumes that the old positions won't be valid anymore).  In either
   case, the objects become selected (by setting the Am_VALUE of the
   Am_SELECTION_WIDGET). 
   Selective Repeat regroups the objects again if all of them are
   visible and valid, and none are in a group, and repeat-on-new groups
   the selected objects.

   Slot to set in the command object: Am_SELECTION_WIDGET
   (Gets the group out of the Am_OPERATES_ON slot of the Am_SELECTION_WIDGET)
*/

//since groups define their own coordinates, have to adjust
//coordinates of each object when they are grouped or ungrouped
void adjust_objects_positions(Am_Value_List objs, int
			      left_offset, int top_offset) {
  Am_Object obj;
  for(objs.Start(); !objs.Last(); objs.Next()) {
    obj = objs.Get();
    obj.Set(Am_LEFT, (int)obj.Get(Am_LEFT) + left_offset, Am_NO_ANIMATION);
    obj.Set(Am_TOP, (int)obj.Get(Am_TOP) + top_offset, Am_NO_ANIMATION);
  }
}

//move objs from old_group to new_group.  If place is non-null, puts all
//objects directly before place.  If new_group is (0L), then
//just removes them from old group.  
void remove_and_add_objects(Am_Value_List objs, Am_Object old_group,
			    Am_Object new_group, 
			    Am_Object place = Am_No_Object) {
  Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING, "++ Removing parts " <<
		       objs << " from " << old_group << " and adding them to "
		       << new_group);
  Am_Object object;
  for(objs.Start(); !objs.Last(); objs.Next()) {
    object = objs.Get();
    old_group.Remove_Part(object);
    if (new_group.Valid()) {
      new_group.Add_Part(object);
      if (place.Valid()) {
	Am_Move_Object(object, place, false); //put object before place
      }
      else {
	Am_To_Top(object);
      }
    }
  }
}

// returns the last place found
Am_Object put_objects_into_group(Am_Value_List selected_objs,
			    Am_Value_List &selected_objs_places,
			    Am_Object group, Am_Object new_group) {
  int left, top, width, height;
  int left_offset = 9999;
  int top_offset = 9999;
  int right = -9999;
  int bottom = -9999;
  Am_Object obj, place;
  for(selected_objs.Start(); !selected_objs.Last(); selected_objs.Next()) {
    obj = selected_objs.Get();
    left = obj.Get(Am_LEFT);
    top = obj.Get(Am_TOP);
    width = obj.Get(Am_WIDTH);
    height = obj.Get(Am_HEIGHT);
    if (left < left_offset) left_offset = left;
    if (top < top_offset) top_offset = top;
    if (left+width > right) right = left+width;
    if (top+height > bottom) bottom = top+height;
    place = Am_Find_Part_Place(obj, group);
    selected_objs_places.Add(place);
  }
  new_group.Set(Am_LEFT, left_offset)
    .Set(Am_TOP, top_offset, Am_NO_ANIMATION)
    .Set(Am_WIDTH, right-left_offset, Am_NO_ANIMATION)
    .Set(Am_HEIGHT, bottom-top_offset, Am_NO_ANIMATION)
    ;
  adjust_objects_positions(selected_objs, -left_offset, -top_offset);
  remove_and_add_objects(selected_objs, group, new_group);
  return place;
}

void group_objects_internal(Am_Object &cmd, Am_Object &selection_widget,
			      Am_Value_List &selected_objs,
			      Am_Object &group) {
  Am_Object new_group, place;
  Am_Value_List selected_objs_places;
  cmd.Set(Am_OBJECT_MODIFIED, selected_objs);
  cmd.Set(Am_SAVED_OLD_OBJECT_OWNER, group);
  cmd.Set(Am_HAS_BEEN_UNDONE, false);

  Am_Object prototype = cmd.Get (Am_ITEM_PROTOTYPE);
  new_group = prototype.Create()
    .Set(Am_CREATED_GROUP, true, Am_OK_IF_NOT_THERE);
  place = put_objects_into_group(selected_objs, selected_objs_places,
				 group, new_group);
  cmd.Set(Am_OBJECT_MODIFIED_PLACES, selected_objs_places);
  group.Add_Part(new_group); //add to top
  if (place.Valid()) {
    Am_Move_Object(new_group, place, false); //put object before place
  }
  cmd.Set(Am_VALUE, new_group); 
  //new selection is just the new group object
  selection_widget.Set(Am_VALUE, Am_Value_List().Add(new_group));
}

Am_Define_Method(Am_Object_Method, void, group_objects, (Am_Object cmd)) {
  Am_Object selection_widget, group, obj;
  Am_Value_List selected_objs;
  selection_widget = Am_Get_Selection_Widget_For_Command(cmd);
  Am_Get_Selection_In_Display_Order(selection_widget, selected_objs, group);
  group_objects_internal(cmd, selection_widget, selected_objs, group);
}

void group_new_objs(Am_Object& cmd, Am_Value new_sel) {
  Am_Object selection_widget, group, obj;
  Am_Value_List selected_objs;
  selection_widget = Am_Get_Selection_Widget_For_Command(cmd);
  Am_Get_Sel_Or_Param_In_Display_Order(selection_widget, selected_objs, group,
				       new_sel);
  group_objects_internal(cmd, selection_widget, selected_objs, group);
}

void group_general_undo_redo(Am_Object command_obj, bool selective) {
  Am_Object group, new_group, selection_widget;
  Am_Value_List grouped_objs, places;
  selection_widget = Am_Get_Selection_Widget_For_Command(command_obj);
  group = command_obj.Get(Am_SAVED_OLD_OBJECT_OWNER);
  grouped_objs = command_obj.Get(Am_OBJECT_MODIFIED);
  new_group = command_obj.Get(Am_VALUE);
  if (!selective)
    places = command_obj.Get(Am_OBJECT_MODIFIED_PLACES);
  bool currently_undone = command_obj.Get(Am_HAS_BEEN_UNDONE);
  int left_offset = new_group.Get(Am_LEFT);
  int top_offset = new_group.Get(Am_TOP);
  
  if (currently_undone) { // then redo = re-delete the
			    // objects.  Places should still be valid
    if (selective)
      Am_Error("** Shouldn't be possible to redo if selective");
    //new_group will have been on top, so just re-add it
    adjust_objects_positions(grouped_objs, -left_offset, -top_offset);
    remove_and_add_objects(grouped_objs, group, new_group);
    group.Add_Part(new_group);
    //new selection is just the new group object
    selection_widget.Set(Am_VALUE, Am_Value_List().Add(new_group));
  }
  else { // undo = remove group (don't delete it) and put objects
	   // back where they were
    adjust_objects_positions(grouped_objs, left_offset, top_offset);
    remove_and_add_objects(grouped_objs, new_group, Am_No_Object);
    group.Remove_Part(new_group);
    if (!selective) {
      Am_Put_Objects_Back_In_Place(group, grouped_objs, places, true);
    }
    else Am_Put_Objects_Into_Group(group, grouped_objs);
    
    //now set the selection.
    selection_widget.Set(Am_VALUE, grouped_objs);
  }
  command_obj.Set(Am_HAS_BEEN_UNDONE, !currently_undone);
}

Am_Define_Method(Am_Object_Method, void, group_objects_undo,
		 (Am_Object command_obj)) {
  group_general_undo_redo(command_obj, false);
}
Am_Define_Method(Am_Object_Method, void, group_objects_sel_undo,
		 (Am_Object command_obj)){
  group_general_undo_redo(command_obj, true);
}

Am_Define_Method(Am_Object_Method, void, group_objects_sel_repeat,
		 (Am_Object command_obj)) {
  Am_Value new_sel = command_obj.Get(Am_OBJECT_MODIFIED);
  group_new_objs(command_obj, new_sel);
}

Am_Define_Method(Am_Selective_Repeat_New_Method, void,
		 group_objects_sel_repeat_new,
		 (Am_Object command_obj, Am_Value new_sel,
		  Am_Value /* new_value */)){
  group_new_objs(command_obj, new_sel);
}

//returns true if all the objects in obj_list_value are valid and visible and
//if want_in_sub, then if they are in sub_groups, else if they are NOT in
//sub-groups of group.
bool check_all_vis_and_in_group(Am_Value obj_list_value, Am_Object group,
				bool want_in_sub) {
  if (group.Valid()) {
    Am_Object obj, obj_group;
    bool ret = Am_Valid_and_Visible_List_Or_Object(obj_list_value, true);
    if (ret) {
      //now check that none are in a sub-group
      Am_Value_List grouped_objs;
      grouped_objs = obj_list_value;
      for(grouped_objs.Start(); !grouped_objs.Last(); grouped_objs.Next()) {
	obj = grouped_objs.Get();
	obj_group = obj.Get_Owner();
	if ( (obj_group == group) == want_in_sub)
	  return false;
      }
      return true;
    }
  }
  return false;
}

//selective repeat is allowed if all of the objects are visible and
//valid, and if none are in a group (other than the main group)
Am_Define_Method(Am_Selective_Allowed_Method, bool,
		 group_selective_repeat_allowed, (Am_Object command_obj)) {
  Am_Object group, selection_widget;
  selection_widget = Am_Get_Selection_Widget_For_Command(command_obj);
  Am_Value value;
  if (selection_widget.Valid()) {
    group = selection_widget.Get(Am_OPERATES_ON);
    value=command_obj.Peek(Am_OBJECT_MODIFIED);
    return check_all_vis_and_in_group(value, group, false);
  }
  else return false;
}

//selective undo is allowed if all of the objects are visible and
//valid, and if all are still directly in the new_group
Am_Define_Method(Am_Selective_Allowed_Method, bool,
		 group_selective_undo_allowed, (Am_Object command_obj)) {
  Am_Object new_group;
  Am_Value value;
  new_group = command_obj.Get(Am_VALUE);
  value=command_obj.Peek(Am_OBJECT_MODIFIED);
  return check_all_vis_and_in_group(value, new_group, false);
}

///////////////////////////////////////////////////////////////////////////
// UnGroup
///////////////////////////////////////////////////////////////////////////

/* UnGroup
   If the selected objects are groups with the slot Am_CREATED_GROUP
   set to true, then ungroups the objects in those groups and puts the
   new objects in place where the groups were.
   Undo re-established the original groups where they were.
   Selective undo puts the groups at the top of the original group
   (assumes that the old positions won't be valid anymore).  In either
   case, the objects become selected (by setting the Am_VALUE of the
   Am_SELECTION_WIDGET). 
   Selective Repeat ungroups the objects again if the groups are valid,
   and repeat-on-new ungroups the selected objects.

   Slot to set in the command object: Am_SELECTION_WIDGET
   (Gets the group out of the Am_OPERATES_ON slot of the Am_SELECTION_WIDGET)
*/


void ungroup_objects_internal(Am_Object &cmd, Am_Object &selection_widget,
			      Am_Value_List &selected_objs,
			      Am_Object &group) {
  Am_Object obj, place, sub_group;
  int left, top;
  Am_Value_List selected_objs_places, sub_group_parts,
    new_selected_objs;
  selection_widget = Am_Get_Selection_Widget_For_Command(cmd);
  Am_Get_Selection_In_Display_Order(selection_widget, selected_objs, group);
  cmd.Set(Am_OBJECT_MODIFIED, selected_objs);
  cmd.Set(Am_SAVED_OLD_OBJECT_OWNER, group);
  cmd.Set(Am_HAS_BEEN_UNDONE, false);

  for(selected_objs.Start(); !selected_objs.Last(); selected_objs.Next()) {
    sub_group = selected_objs.Get();
    left = sub_group.Get(Am_LEFT);
    top = sub_group.Get(Am_TOP);
    place = Am_Find_Part_Place(sub_group, group);
    selected_objs_places.Add(place);

    // now take out the pieces
    sub_group_parts = sub_group.Get(Am_GRAPHICAL_PARTS);
    //save a copy in case undo
    sub_group.Set(Am_OBJECT_MODIFIED_PLACES, sub_group_parts, Am_OK_IF_NOT_THERE);
    adjust_objects_positions(sub_group_parts, left, top);
    remove_and_add_objects(sub_group_parts, sub_group, group, place);
    new_selected_objs.Append(sub_group_parts);
    group.Remove_Part(sub_group);
  }

  cmd.Set(Am_OBJECT_MODIFIED_PLACES, selected_objs_places);
  cmd.Set(Am_VALUE, new_selected_objs); //all the objects affected
  //new selection is all the objects taken out of groups
  selection_widget.Set(Am_VALUE, new_selected_objs);
}

Am_Define_Method(Am_Object_Method, void, ungroup_objects, (Am_Object cmd)) {
  Am_Object selection_widget, group, obj;
  Am_Value_List selected_objs;
  selection_widget = Am_Get_Selection_Widget_For_Command(cmd);
  Am_Get_Selection_In_Display_Order(selection_widget, selected_objs, group);
  ungroup_objects_internal(cmd, selection_widget, selected_objs, group);
}

void ungroup_new_objs(Am_Object& cmd, Am_Value new_sel) {
  Am_Object selection_widget, group, obj;
  Am_Value_List selected_objs;
  selection_widget = Am_Get_Selection_Widget_For_Command(cmd);
  Am_Get_Sel_Or_Param_In_Display_Order(selection_widget, selected_objs, group,
				       new_sel);
  ungroup_objects_internal(cmd, selection_widget, selected_objs, group);
}

void in_or_out_of_sub_groups(Am_Value_List sub_groups, Am_Object group,
			     bool into_sub_group, bool recalc) {
  Am_Value_List sub_group_parts, dummy_list;
  int left, top;
  Am_Object sub_group;
  for(sub_groups.Start(); !sub_groups.Last(); sub_groups.Next()) {
    sub_group = sub_groups.Get();
    left = sub_group.Get(Am_LEFT);
    top = sub_group.Get(Am_TOP);
    sub_group_parts = sub_group.Get(Am_OBJECT_MODIFIED_PLACES);
    if (into_sub_group) {
      if (recalc)
	put_objects_into_group(sub_group_parts, dummy_list, group, sub_group);
      else {
	adjust_objects_positions(sub_group_parts, -left, -top);
	remove_and_add_objects(sub_group_parts, group, sub_group);
      }
    }
    else {
      adjust_objects_positions(sub_group_parts, left, top);
      remove_and_add_objects(sub_group_parts, sub_group, group);
    }
  }
}

void ungroup_general_undo_redo(Am_Object command_obj, bool selective) {
  Am_Object group, sub_group, selection_widget;
  Am_Value_List sub_groups, places;
  selection_widget = Am_Get_Selection_Widget_For_Command(command_obj);
  group = command_obj.Get(Am_SAVED_OLD_OBJECT_OWNER);
  sub_groups = command_obj.Get(Am_OBJECT_MODIFIED);
  if (!selective)
    places = command_obj.Get(Am_OBJECT_MODIFIED_PLACES);
  bool currently_undone = command_obj.Get(Am_HAS_BEEN_UNDONE);
  
  if (currently_undone) { // then redo = ungroup objects again.
			    // Places should still be valid
    if (selective)
      Am_Error("** Shouldn't be possible to redo if selective");
      //remove objects from sub_groups
      in_or_out_of_sub_groups(sub_groups, group, false, false);
      //remove the sub_groups from group
      remove_and_add_objects(sub_groups, group, Am_No_Object);
      
      //set the selection
      Am_Value_List new_selected_objs;
      new_selected_objs = command_obj.Get(Am_VALUE);
      selection_widget.Set(Am_VALUE, new_selected_objs);
    }
    else { // undo = restore groups and put objects into them
      if (!selective) {
	Am_Put_Objects_Back_In_Place(group, sub_groups, places, true);
	//move objects back into the groups
	in_or_out_of_sub_groups(sub_groups, group, true, false);
      }
      else {
	Am_Put_Objects_Into_Group(group, sub_groups);
	in_or_out_of_sub_groups(sub_groups, group, true, true);
      }
    //remove objects from sub_groups
    in_or_out_of_sub_groups(sub_groups, group, false, false);
    //remove the sub_groups from group
    remove_and_add_objects(sub_groups, group, Am_No_Object);

    //now set the selection.
    selection_widget.Set(Am_VALUE, sub_groups);
    //set the selection
    Am_Value_List new_selected_objs;
    new_selected_objs = command_obj.Get(Am_VALUE);
    selection_widget.Set(Am_VALUE, new_selected_objs);
  }
  command_obj.Set(Am_HAS_BEEN_UNDONE, !currently_undone);
}

Am_Define_Method(Am_Object_Method, void, ungroup_objects_undo,
		 (Am_Object command_obj)) {
  ungroup_general_undo_redo(command_obj, false);
}
Am_Define_Method(Am_Object_Method, void, ungroup_objects_sel_undo,
		 (Am_Object command_obj)){
  ungroup_general_undo_redo(command_obj, true);
}

Am_Define_Method(Am_Object_Method, void, ungroup_objects_sel_repeat,
		 (Am_Object command_obj)){
  Am_Value new_sel = command_obj.Get(Am_OBJECT_MODIFIED);
  ungroup_new_objs(command_obj, new_sel);
}

Am_Define_Method(Am_Selective_Repeat_New_Method, void,
		 ungroup_objects_sel_repeat_new,
		 (Am_Object command_obj, Am_Value new_sel,
		  Am_Value /* new_value */)) {
  ungroup_new_objs(command_obj, new_sel);
}

// ACTIVE if objects are selected and all of them are groups
//   self is the command object
Am_Define_Formula(bool, ungroup_active) {
  Am_Object selection_widget = Am_Get_Selection_Widget_For_Command(self);
  if (selection_widget.Valid()) {
    Am_Value_List selected_objs = selection_widget.Get(Am_VALUE);
    if (selected_objs.Empty()) return false;
    Am_Object obj;
    Am_Value is_group;
    for(selected_objs.Start(); !selected_objs.Last(); selected_objs.Next()) {
      obj = selected_objs.Get();
      is_group=obj.Peek(Am_CREATED_GROUP, Am_NO_DEPENDENCY); //don't need GV since won't change
      if (!is_group.Valid()) return false;
    }
    return Am_Check_All_Objects_For_Inactive_Slot(selected_objs, self);
  }
  else return false;
}

//selected_repeat_on_new is allowed when all the selected objects are groups
Am_Define_Method(Am_Selective_New_Allowed_Method, bool,
		 ungroup_selective_repeat_new_allowed,
		 (Am_Object command_obj, Am_Value new_selection,
		  Am_Value /* new_value */)) {
  Am_Value is_group;
  Am_Object obj;
  Am_Value_List selected_objs;
  selected_objs = new_selection;
  if (selected_objs.Empty()) return false;
  for(selected_objs.Start(); !selected_objs.Last(); selected_objs.Next()) {
    obj = selected_objs.Get();
    is_group=obj.Peek(Am_CREATED_GROUP);
    if (!is_group.Valid()) return false;
  }
  return Am_Check_All_Objects_For_Inactive_Slot(selected_objs, command_obj);
}
  
//selective undo is allowed if all of the objects are visible and
//valid, and if none are in a group (other than the main group)
Am_Define_Method(Am_Selective_Allowed_Method, bool,
		 ungroup_selective_undo_allowed, (Am_Object command_obj)) {
  Am_Object group, selection_widget;
  selection_widget = Am_Get_Selection_Widget_For_Command(command_obj);
  Am_Value value;
  if (selection_widget.Valid()) {
    group = selection_widget.Get(Am_OPERATES_ON);
    value=command_obj.Peek(Am_VALUE); //all the objects in all sub-groups
    return check_all_vis_and_in_group(value, group, false);
  }
  else return false;
}

//selective repeat is allowed if all of the objects are visible and
//valid, and if all are still in sub_groups
Am_Define_Method(Am_Selective_Allowed_Method, bool,
		 ungroup_selective_repeat_allowed, (Am_Object command_obj)) {
  Am_Object group, selection_widget;
  selection_widget = Am_Get_Selection_Widget_For_Command(command_obj);
  Am_Value value;
  if (selection_widget.Valid()) {
    group = selection_widget.Get(Am_OPERATES_ON);
    value=command_obj.Peek(Am_VALUE); //all the objects in all sub-groups
    return check_all_vis_and_in_group(value, group, true);
  }
  else return false;
}

///////////////////////////////////////////////////////////////////////////
// Undo & Redo
///////////////////////////////////////////////////////////////////////////

/* Undo
   Undoes the last command.  This is not undoable.  If the
   Am_SELECTION_WIDGET slot is set, the gets the window from it,
   otherwise, gets the window from the widget of the command (the
   Am_SAVED_OLD_OWNER slot of the command).  Undo is not undoable (use
   undo-the-undo).
*/

Am_Object get_undo_handler_from_cmd(Am_Object cmd) {
  Am_Object widget, window, undo_handler;
  widget = Am_Get_Selection_Widget_For_Command(cmd);
  if (widget.Valid())
    window = widget.Get(Am_WINDOW);
  if (!window.Valid()) {
    widget = cmd.Get(Am_SAVED_OLD_OWNER);
    if (widget.Valid())
      window = widget.Get(Am_WINDOW);
  }
  if (window.Valid())
    undo_handler = window.Get(Am_UNDO_HANDLER);
  return undo_handler;
}
Am_Object GV_undo_handler_from_cmd(Am_Object cmd) {
  Am_Object widget, window, undo_handler;
  Am_Value v;
  widget = Am_Get_Selection_Widget_For_Command(cmd);
  if (widget.Valid()) {
    window = widget.Get(Am_WINDOW);
  }
  if (!window.Valid()) {
    v = cmd.Peek(Am_SAVED_OLD_OWNER);
    if (v.Valid()) {
      widget = v;
      window = widget.Get(Am_WINDOW);
    }
  }
  if (window.Valid()) {
    v = window.Peek(Am_UNDO_HANDLER);
    if (v.Valid())
      return v;
  }
  return Am_No_Object;
}

Am_Define_Method(Am_Object_Method, void, undo_cmd_do_undo, (Am_Object cmd)) {
  Am_Object undo_handler, last_command;
  undo_handler = get_undo_handler_from_cmd(cmd);
  if (undo_handler.Valid()) {
    last_command = undo_handler.Get(Am_UNDO_ALLOWED);
    if (last_command.Valid ()) {
      Am_Object_Method method;
      method = undo_handler.Get(Am_PERFORM_UNDO);
      Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SHORT, "Undoing command " <<
			   last_command);
      method.Call(undo_handler);
    }
  }
}

Am_Define_Method(Am_Object_Method, void, redo_cmd_do_redo, (Am_Object cmd)) {
  Am_Object undo_handler, last_command;
  undo_handler = get_undo_handler_from_cmd(cmd);
  if (undo_handler.Valid()) {
    last_command = undo_handler.Get(Am_REDO_ALLOWED);
    if (last_command.Valid ()) {
      Am_Object_Method method;
      Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SHORT, "Redo command "
			   << last_command);
      method = undo_handler.Get(Am_PERFORM_REDO);
      method.Call(undo_handler);
    }
  }
}

Am_Define_Formula(bool, redo_cmd_active) {
  Am_Object undo_handler, last_command;
  undo_handler = GV_undo_handler_from_cmd(self);
  if (undo_handler.Valid()) {
    last_command = undo_handler.Get(Am_REDO_ALLOWED);
    if (last_command.Valid()) return true;
  }
  return false;
}

Am_Define_Formula(bool, undo_cmd_active) {
  Am_Object undo_handler, last_command;
  undo_handler = GV_undo_handler_from_cmd(self);
  if (undo_handler.Valid()) {
    last_command = undo_handler.Get(Am_UNDO_ALLOWED);
    if (last_command.Valid()) return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////
// To Top & To Bottom
///////////////////////////////////////////////////////////////////////////
/* To Top
   Moves all the selected objects to the top.  
   Undo moves them back to where they were.  Selective undo is not
   allowed (since where they were might not be meaningful).
   Selective Repeat move the objects to top again.  Repeat on new moves
   the selected objects to the top.

   Slots to set in the command object: Am_SELECTION_WIDGET
   (Gets the group out of the Am_OPERATES_ON slot of the Am_SELECTION_WIDGET)
*/

/* To Bottom
   Same as To Top, but objects are moved to the bottom

   Slots to set in the command object: Am_SELECTION_WIDGET
   (Gets the group out of the Am_OPERATES_ON slot of the Am_SELECTION_WIDGET)
*/

void do_to_top_or_bottom(Am_Object cmd, bool to_top,
			 Am_Value new_sel = Am_No_Value) {
  Am_Object selection_widget, group, obj, place;
  Am_Value_List selected_objs, selected_objs_places;
  selection_widget = Am_Get_Selection_Widget_For_Command(cmd);
  Am_Get_Sel_Or_Param_In_Display_Order(selection_widget, selected_objs, group,
				       new_sel);
  cmd.Set(Am_OBJECT_MODIFIED, selected_objs);
  cmd.Set(Am_SAVED_OLD_OBJECT_OWNER, group);
  cmd.Set(Am_HAS_BEEN_UNDONE, false);
  
#ifdef DEBUG
  if (Am_Inter_Tracing(Am_INTER_TRACE_SETTING)) {
    if (to_top) 
      std::cout << "++ Moving parts " << selected_objs << " to top\n" << std::flush;
    else
      std::cout << "++ Moving parts " << selected_objs << " to bottom\n" << std::flush;
  }
#endif
  for(selected_objs.Start(); !selected_objs.Last(); selected_objs.Next()) {
    obj = selected_objs.Get();
    place = Am_Find_Part_Place(obj, group);
    if (to_top) Am_To_Top(obj);
    selected_objs_places.Add(place);
  }
  if (!to_top) {
    // have to go through the list in reverse order
    for(selected_objs.End(); !selected_objs.First(); selected_objs.Prev()) {
      obj = selected_objs.Get();
      Am_To_Bottom(obj);
    }
  }
  cmd.Set(Am_OBJECT_MODIFIED_PLACES, selected_objs_places);
  cmd.Set(Am_VALUE, selected_objs_places.Length()); //not used
}

Am_Define_Method(Am_Object_Method, void, to_top_objects, (Am_Object cmd)) {
  do_to_top_or_bottom(cmd, true);
}
Am_Define_Method(Am_Object_Method, void, to_bottom_objects, (Am_Object cmd)) {
  do_to_top_or_bottom(cmd, false);
}

void to_top_or_bottom_general_undo_redo(Am_Object command_obj, bool undo,
					bool selective, bool reload_data,
					bool to_top,
					Am_Value new_sel = Am_No_Value) {
  Am_Object selection_widget, obj;
  Am_Value_List objs;
  selection_widget = Am_Get_Selection_Widget_For_Command(command_obj);
  objs = command_obj.Get(Am_OBJECT_MODIFIED);

  if (undo) {
    if (selective)
      Am_Error("Selective Undo of to top not allowed");
    bool currently_undone = command_obj.Get(Am_HAS_BEEN_UNDONE);
    if (currently_undone) { // then redo = move objects to
			    // top or bottom again.  Assume places is still OK
      if (to_top) {
	for(objs.Start(); !objs.Last(); objs.Next()) {
	  obj = objs.Get();
	  Am_To_Top(obj);
	}
      }
      else {
	// have to go through the list in reverse order
	for(objs.End(); !objs.First(); objs.Prev()) {
	  obj = objs.Get();
	  Am_To_Bottom(obj);
	}
      }
    }
    else { // regular undo; restore the objects to where they were
      Am_Object group;
      Am_Value_List places;
      group = command_obj.Get(Am_SAVED_OLD_OBJECT_OWNER);
      places = command_obj.Get(Am_OBJECT_MODIFIED_PLACES);
      Am_Put_Objects_Back_In_Place(group, objs, places, false);
    }
    command_obj.Set(Am_HAS_BEEN_UNDONE, !currently_undone);
  }
  else { // repeat, to top or bottom again
    if (!reload_data) new_sel = objs;
    do_to_top_or_bottom(command_obj, to_top, new_sel);
  }
}

Am_Define_Method(Am_Object_Method, void, to_top_objects_undo,
		 (Am_Object command_obj)) {
  to_top_or_bottom_general_undo_redo(command_obj, true, false, false, true);
}
Am_Define_Method(Am_Object_Method, void, to_top_objects_sel_repeat,
		 (Am_Object command_obj)){
  to_top_or_bottom_general_undo_redo(command_obj, false, true, false, true);
}
Am_Define_Method(Am_Selective_Repeat_New_Method, void,
		 to_top_objects_sel_repeat_new,
		 (Am_Object command_obj, Am_Value new_sel,
		  Am_Value /* new_value */)){
  to_top_or_bottom_general_undo_redo(command_obj, false, true, true, true,
				     new_sel);
}

Am_Define_Method(Am_Object_Method, void, to_bottom_objects_undo,
		 (Am_Object command_obj)) {
  to_top_or_bottom_general_undo_redo(command_obj, true, false, false, false);
}
Am_Define_Method(Am_Object_Method, void, to_bottom_objects_sel_repeat,
		 (Am_Object command_obj)){
  to_top_or_bottom_general_undo_redo(command_obj, false, true, false, false);
}
Am_Define_Method(Am_Selective_Repeat_New_Method, void,
		 to_bottom_objects_sel_repeat_new,
		 (Am_Object command_obj, Am_Value new_sel,
		  Am_Value /* new_value */)){
  to_top_or_bottom_general_undo_redo(command_obj, false, true, true, false,
				     new_sel);
}

///////////////////////////////////////////////////////////////////////////
// Quit No ask
///////////////////////////////////////////////////////////////////////////
/* 
  Am_Quit_No_Ask_Command: Quits immediately without asking.  Doesn't check
  whether any documents need saving or anything.  Not undoable.
*/

Am_Define_Method(Am_Object_Method, void, quit_no_ask,
		 (Am_Object /* command_obj */)){
  Am_Exit_Main_Event_Loop ();
}

Am_Define_String_Formula(quit_or_exit) {
  Am_Object widget = self.Get(Am_SAVED_OLD_OWNER, Am_RETURN_ZERO_ON_ERROR);
  if (widget.Valid()) {
    Am_Widget_Look look = widget.Get(Am_WIDGET_LOOK, Am_NO_DEPENDENCY);
    if (look == Am_WINDOWS_LOOK) return Am_String("Exit");
  }
  return Am_String("Quit");
}
    
Am_Define_Formula(Am_Value, control_q_or_none) {
  Am_Object widget = self.Get(Am_SAVED_OLD_OWNER, Am_RETURN_ZERO_ON_ERROR);
  if (widget.Valid()) {
    Am_Widget_Look look = widget.Get(Am_WIDGET_LOOK, Am_NO_DEPENDENCY);
    if (look == Am_WINDOWS_LOOK)
      return Am_No_Value;
  }
  Am_Value v = Am_Input_Char("CONTROL_q");
  return v;
}

///////////////////////////////////////////////////////////////////////////
// Am_Cycle_Value_Command
///////////////////////////////////////////////////////////////////////////
/* 
  Am_Cycle_Value_Command: Provide a list of labels in the Am_LABEL_LIST slot,
  and it goes through the list with each click.  The Am_VALUE of the
  command is set with the index of the current label in the list (from
  0).  If the list has 2 values, then this will be a toggle, like
  "Turn Grid On" <-> "Turn Grid Off" with the Am_VALUE taking on 0 and
  1.  Undoing returns the Am_VALUE and label to the original values.
  You can also explicitly set the Am_VALUE slot of the command to change
  the current state.  Typically, a constraint would depend on the
  Am_VALUE of the command.
*/

Am_Define_Method(Am_Object_Method, void, cycle_value_do,
		 (Am_Object command_obj)) {
  Am_Value_List labels = command_obj.Get(Am_LABEL_LIST);
  int len = labels.Length();
  int cur_val = command_obj.Get(Am_COMPUTE_INTER_VALUE);
  command_obj.Set(Am_OLD_VALUE, cur_val);
  if (len == 0) {
    command_obj.Set(Am_VALUE, 0); //Am_COMPUTE_INTER_VALUE will be set
				  //by formula
  }
  else {
    cur_val++;
    if (cur_val >= len) cur_val = 0;
    command_obj.Set(Am_VALUE, cur_val); //Am_COMPUTE_INTER_VALUE will be set by
					//cycle_value_get_string formula
  }
}

void cycle_value_general_undo_redo(Am_Object command_obj, bool undo,
				   bool selective, bool reload_data = false,
				   int new_val = 0) {
  int cur_val;
  //going to set to new_val, with cur_val as old value for undo
  Am_Object orig_command = command_obj.Get_Object(Am_SOURCE_OF_COPY);
  if (selective) {
    if (orig_command.Valid()) cur_val = orig_command.Get(Am_VALUE);
    else cur_val = 0;
    if (undo) new_val = command_obj.Get(Am_OLD_VALUE);
    else if (!reload_data)
      new_val = command_obj.Get(Am_VALUE);
  }
  else {
     cur_val = command_obj.Get(Am_VALUE);
     new_val = command_obj.Get(Am_OLD_VALUE);
  }
  command_obj.Set(Am_OLD_VALUE, cur_val);
  command_obj.Set(Am_VALUE, new_val);
  if (orig_command.Valid()) {
    orig_command.Set(Am_VALUE, new_val);
  }
}  

Am_Define_Method(Am_Object_Method, void, cycle_value_undo,
		 (Am_Object command_obj)) {
  cycle_value_general_undo_redo(command_obj, true, false);
}
Am_Define_Method(Am_Object_Method, void, cycle_value_sel_undo,
		 (Am_Object command_obj)){
  cycle_value_general_undo_redo(command_obj, true, true);
}
Am_Define_Method(Am_Object_Method, void, cycle_value_sel_repeat,
		 (Am_Object command_obj)){
  cycle_value_general_undo_redo(command_obj, false, true);
}


Am_Define_Method(Am_Selective_Repeat_New_Method, void, cycle_value_repeat_new,
		 (Am_Object command_obj, Am_Value /* new_sel */,
		  Am_Value new_value)) {
  cycle_value_general_undo_redo(command_obj, false, true, true, new_value);
}


//put this in the label slot of the command object
Am_Define_Formula (Am_Value, cycle_value_get_string) {
  Am_Value value;
  Am_Value_List labels = self.Get(Am_LABEL_LIST);
  Am_Value v;
  v = self.Peek(Am_VALUE);
  if (v.type == Am_INT) {
    int cur_val = v;
    self.Set(Am_COMPUTE_INTER_VALUE, cur_val); //keep consistent with value
    if (cur_val < 0 || cur_val >= labels.Length()) {
      value = "**ERROR**";
    }
    else value = labels.Get_Nth(cur_val);
  }
  return value;
}

///////////////////////////////////////////////////////////////////////////
// Am_Open_Command
///////////////////////////////////////////////////////////////////////////

Am_String get_default_filename(Am_Object &command_obj) {
  Am_Object sel_widget = Am_Get_Selection_Widget_For_Command(command_obj);
  if (sel_widget.Valid())
    return sel_widget.Get(Am_DEFAULT_LOAD_SAVE_FILENAME,
			  Am_RETURN_ZERO_ON_ERROR);
  else return Am_No_String;
}
void set_default_filename(Am_Object &command_obj, Am_String &filename) {
  Am_Object sel_widget = Am_Get_Selection_Widget_For_Command(command_obj);
  if (sel_widget.Valid())
    sel_widget.Set(Am_DEFAULT_LOAD_SAVE_FILENAME, filename,
		   Am_OK_IF_NOT_THERE);
}

//When names come in on the PC, they have extra quotes around them
Am_String remove_extra_quotes(Am_String filename) {
  char * s = filename;
  if (s[0] == '\"') {
    int len = strlen(s);
    for (int i = 1; i < len-1; i++) 
      s[i-1] = s[i];
    s[len-2] = s[len];
    filename = s;
  }
  return filename;
}

//This might be called for a file types as the command line argument.
//The command object should be the standard open command object
//attached to the menu_bar (needed to get the default selection and
//the Am_HANDLE_OPEN_SAVE_METHOD).
void Am_Standard_Open_From_Filename(Am_Object command_obj,
				    Am_String filename) {
  filename = remove_extra_quotes(filename);
  set_default_filename(command_obj, filename);
  std::ifstream in_file ((const char*)filename, std::ios::in);
  if (!in_file) {
    Am_Show_Alert_Dialog (Am_Value_List ().Add("File could not be opened:")
			  .Add(filename));
    return;
  }
  Am_Default_Load_Save_Context.Reset ();
  Am_Value contents = Am_Default_Load_Save_Context.Load (in_file);
  if (!contents.Valid () || !Am_Value_List::Test (contents)) {
    Am_Show_Alert_Dialog (Am_Value_List ()
			  .Add ("File does not contain valid data:")
			  .Add (filename));
    return;
  }
  Am_Value_List loaded = contents;
  Am_Handle_Loaded_Items_Method method =
    command_obj.Get(Am_HANDLE_OPEN_SAVE_METHOD);
  if (method.Valid())
    method.Call(command_obj, loaded);
  else Am_ERRORO("Open Command " << command_obj
		 << " must have a method in Am_HANDLE_OPEN_SAVE_METHOD slot",
		 command_obj, Am_HANDLE_OPEN_SAVE_METHOD);
}

Am_Define_Method(Am_Object_Method, void, open_do,
		 (Am_Object command_obj)) {
  Am_String default_file_name = get_default_filename(command_obj);
  Am_String filename = Am_Show_File_Dialog_For_Open(default_file_name);
  if (!filename.Valid () || filename == "")
    return;
  Am_Standard_Open_From_Filename(command_obj, filename);
}

void internal_do_save (Am_Object command_obj, Am_String & filename) {

  std::ofstream out_file;
  out_file.open(filename.To_String(), std::ios::out);

  if (!out_file.good()) {
    Am_Show_Alert_Dialog (Am_Value_List ()
       .Add ("Cannot open C++ file for writing:")
       .Add (filename));
    return;
  }

  set_default_filename(command_obj, filename);

  Am_Value_List items_to_save;
  Am_Items_To_Save_Method method = command_obj.Get(Am_HANDLE_OPEN_SAVE_METHOD);
  if (method.Valid())
    items_to_save = method.Call(command_obj);
  else Am_ERRORO("Save Command " << command_obj
		 << " must have a method in Am_HANDLE_OPEN_SAVE_METHOD slot",
		 command_obj, Am_HANDLE_OPEN_SAVE_METHOD);
  Am_Default_Load_Save_Context.Reset ();
  Am_Default_Load_Save_Context.Save (out_file, Am_Value (items_to_save));
  out_file.close(); 
}

Am_Define_Method(Am_Object_Method, void, save_as_do,
		 (Am_Object command_obj)) {
  Am_String default_file_name = get_default_filename(command_obj);
  Am_String filename = Am_Show_File_Dialog_For_Save(default_file_name);
  if (!filename.Valid () || filename == "")
    return;
  internal_do_save(command_obj, filename);
}

Am_Define_Method(Am_Object_Method, void, save_do,
		 (Am_Object command_obj)) {
  Am_String filename = get_default_filename(command_obj);
  if (!filename.Valid()) {
    filename = Am_Show_File_Dialog_For_Save(filename);
    if (!filename.Valid () || filename == "")
      return;
  }
  internal_do_save(command_obj, filename);
}

////////////////////////////////////////////////////////////////////////////
// Initialization
////////////////////////////////////////////////////////////////////////////

static void init() {

  Am_Global_Clipboard = Am_Root_Object.Create(DSTR("Global_Clipboard"))
    .Add(Am_VALUE, Am_Value_List()) //empty list
    ;

  Am_Graphics_Set_Property_Command = Am_Command.Create(DSTR("Set_Property_Command"))
    .Set(Am_LABEL, "Change Property")
    .Set(Am_ACTIVE, true) //always true since can change global
			  //property even if nothing selected
    .Set(Am_DO_METHOD, set_property)
    .Set(Am_UNDO_METHOD, property_undo)
    .Set(Am_REDO_METHOD, property_undo)
    .Set(Am_SELECTIVE_UNDO_METHOD, property_sel_undo)
    .Set(Am_SELECTIVE_REPEAT_SAME_METHOD, property_sel_repeat)
    .Set(Am_SELECTIVE_REPEAT_ON_NEW_METHOD, property_sel_repeat_new)
    .Add(Am_GET_WIDGET_VALUE_METHOD, default_widget_value_method)
    .Add(Am_GET_OBJECT_VALUE_METHOD, default_get_object_value_method)
    .Add(Am_SET_OBJECT_VALUE_METHOD, default_set_object_value_method)
    .Add(Am_SLOT_FOR_VALUE, (int)Am_FILL_STYLE)
    .Add(Am_SELECTION_WIDGET, (0L))
    .Add(Am_SLOT_FOR_THIS_COMMAND_INACTIVE, (int)Am_SET_PROPERTY_INACTIVE)
    ;

  Am_Graphics_Clear_Command = Am_Command.Create(DSTR("Delete_Objects_Command"))
    .Set(Am_LABEL, "Clear")
    .Set(Am_ACTIVE, Am_Active_If_Selection)
    .Set(Am_DO_METHOD, delete_objects)
    .Set(Am_UNDO_METHOD, delete_objects_undo)
    .Set(Am_REDO_METHOD, delete_objects_undo)
    .Set(Am_SELECTIVE_UNDO_METHOD, delete_objects_sel_undo)
    .Set(Am_SELECTIVE_UNDO_ALLOWED, Am_Delete_Selective_Undo_Allowed)
    .Set(Am_SELECTIVE_REPEAT_SAME_METHOD, delete_objects_sel_repeat)
    .Set(Am_SELECTIVE_REPEAT_ON_NEW_METHOD, delete_objects_sel_repeat_new)
    .Set(Am_SELECTIVE_REPEAT_NEW_ALLOWED,
	 Am_Selective_New_Allowed_Return_True)
    .Add(Am_SELECTION_WIDGET, (0L))
    .Set(Am_ACCELERATOR, Am_Input_Char("DELETE"))
    .Add(Am_SLOT_FOR_THIS_COMMAND_INACTIVE, (int)Am_CLEAR_INACTIVE)
    .Add (Am_OBJECT_MODIFIED_PLACES, 0)
    ;
  Am_Graphics_Clear_All_Command =
    Am_Command.Create(DSTR("Delete_All_Objects_Command"))
    .Set(Am_LABEL, "Clear All")
    .Set(Am_ACTIVE, active_if_any)
    .Set(Am_DO_METHOD, delete_all_objects)
    .Set(Am_UNDO_METHOD, delete_objects_undo)
    .Set(Am_REDO_METHOD, delete_objects_undo)
    .Set(Am_SELECTIVE_UNDO_METHOD, delete_objects_sel_undo)
    .Set(Am_SELECTIVE_UNDO_ALLOWED, Am_Delete_Selective_Undo_Allowed)
    .Set(Am_SELECTIVE_REPEAT_SAME_METHOD, delete_objects_sel_repeat)
    .Set(Am_SELECTIVE_REPEAT_NEW_ALLOWED,
	 Am_Selective_New_Allowed_Return_False)
    .Set(Am_SELECTIVE_REPEAT_ON_NEW_METHOD, (0L))
    .Add(Am_SELECTION_WIDGET, (0L))
    .Set(Am_ACCELERATOR, Am_Input_Char("CONTROL_DELETE"))
    .Add(Am_SLOT_FOR_THIS_COMMAND_INACTIVE, (int)Am_CLEAR_INACTIVE)
    .Add (Am_OBJECT_MODIFIED_PLACES, 0)
    ;

  Am_Graphics_Cut_Command = Am_Command.Create(DSTR("Cut_Objects_Command"))
    .Set(Am_LABEL, "Cut")
    .Set(Am_ACTIVE, Am_Active_If_Selection)
    .Set(Am_DO_METHOD, cut_objects)
    .Set(Am_UNDO_METHOD, cut_objects_undo)
    .Set(Am_REDO_METHOD, cut_objects_undo)
    .Set(Am_SELECTIVE_UNDO_METHOD, cut_objects_sel_undo)
    .Set(Am_SELECTIVE_UNDO_ALLOWED, Am_Delete_Selective_Undo_Allowed)
    .Set(Am_SELECTIVE_REPEAT_SAME_METHOD, cut_objects_sel_repeat)
    .Set(Am_SELECTIVE_REPEAT_ON_NEW_METHOD, cut_objects_sel_repeat_new)
    .Add(Am_SELECTION_WIDGET, (0L))
    .Set(Am_ACCELERATOR, Am_Input_Char("CONTROL_x"))
    .Add(Am_CLIPBOARD, (0L))
    .Add(Am_OLD_CLIPBOARD_OBJECTS, (0L))
    .Add(Am_SLOT_FOR_THIS_COMMAND_INACTIVE, (int)Am_CUT_INACTIVE)
    ;

  Am_Graphics_Copy_Command = Am_Command.Create(DSTR("Copy_Objects_Command"))
    .Set(Am_LABEL, "Copy")
    .Set(Am_ACTIVE, Am_Active_If_Selection)
    .Set(Am_DO_METHOD, copy_objects)
    .Set(Am_UNDO_METHOD, copy_objects_undo)
    .Set(Am_REDO_METHOD, copy_objects_undo)
    .Set(Am_SELECTIVE_UNDO_METHOD, (0L))
    .Set(Am_SELECTIVE_UNDO_ALLOWED, Am_Selective_Allowed_Return_False)
    .Set(Am_SELECTIVE_REPEAT_SAME_METHOD, copy_objects_sel_repeat)
    .Set(Am_SELECTIVE_REPEAT_NEW_ALLOWED, new_sel_valid)
    .Set(Am_SELECTIVE_REPEAT_ON_NEW_METHOD, copy_objects_sel_repeat_new)
    .Add(Am_SELECTION_WIDGET, (0L))
    .Set(Am_ACCELERATOR, Am_Input_Char("CONTROL_c"))
    .Add(Am_CLIPBOARD, (0L))
    .Add(Am_OLD_CLIPBOARD_OBJECTS, (0L))
    .Add(Am_SLOT_FOR_THIS_COMMAND_INACTIVE, (int)Am_COPY_INACTIVE)
    ;

  Am_Graphics_Paste_Command =
    Am_From_Other_Create_Command.Create(DSTR("Paste_Objects_Command"))
    .Set(Am_LABEL, "Paste")
    .Set(Am_ACTIVE, active_if_clipboard)
    .Set(Am_DO_METHOD, paste_objects)
    .Set(Am_UNDO_METHOD, paste_objects_undo)
    .Set(Am_REDO_METHOD, paste_objects_undo)
    .Set(Am_SELECTIVE_UNDO_METHOD, paste_objects_sel_undo)
    .Set(Am_SELECTIVE_REPEAT_SAME_METHOD, paste_objects_sel_repeat)
    .Set(Am_SELECTIVE_REPEAT_ON_NEW_METHOD, (0L))
    .Set(Am_SELECTIVE_REPEAT_NEW_ALLOWED,
	 Am_Selective_New_Allowed_Return_False)
    .Set(Am_ACCELERATOR, Am_Input_Char("CONTROL_v"))
    .Add(Am_SELECTION_WIDGET, (0L))
    .Add(Am_CLIPBOARD, (0L))
    ;
  Am_Graphics_Duplicate_Command	=
    Am_From_Other_Create_Command.Create(DSTR("Duplicate_Objects_Command"))
    .Set(Am_LABEL, "Duplicate")
    .Set(Am_ACTIVE, Am_Active_If_Selection)
    .Set(Am_DO_METHOD, duplicate_objects)
    .Set(Am_UNDO_METHOD, duplicate_objects_undo)
    .Set(Am_REDO_METHOD, duplicate_objects_undo)
    .Set(Am_SELECTIVE_UNDO_METHOD, duplicate_objects_sel_undo)
    .Set(Am_SELECTIVE_REPEAT_NEW_ALLOWED, new_sel_valid)
    .Set(Am_SELECTIVE_REPEAT_SAME_ALLOWED,
	 Am_Selective_Allowed_Return_True)
    .Set(Am_SELECTIVE_REPEAT_SAME_METHOD, duplicate_objects_sel_repeat)
    .Set(Am_SELECTIVE_REPEAT_ON_NEW_METHOD, duplicate_objects_sel_repeat_new)
    .Add(Am_SELECTION_WIDGET, (0L))
    .Set(Am_ACCELERATOR, Am_Input_Char("CONTROL_d"))
    .Add(Am_SLOT_FOR_THIS_COMMAND_INACTIVE, (int)Am_DUPLICATE_INACTIVE)
    .Add (Am_START_OBJECT, 0)
    ;
  
  Am_Graphics_Group_Command =
    Am_From_Other_Create_Command.Create(DSTR("Group_Objects_Command"))
    .Set(Am_LABEL, "Group")
    .Set(Am_ACTIVE, Am_Active_If_Selection)
    .Set(Am_DO_METHOD, group_objects)
    .Set(Am_UNDO_METHOD, group_objects_undo)
    .Set(Am_REDO_METHOD, group_objects_undo)
    .Set(Am_SELECTIVE_UNDO_ALLOWED, group_selective_undo_allowed)
    .Set(Am_SELECTIVE_UNDO_METHOD, group_objects_sel_undo)
    .Set(Am_SELECTIVE_REPEAT_SAME_ALLOWED, group_selective_repeat_allowed)
    .Set(Am_SELECTIVE_REPEAT_SAME_METHOD, group_objects_sel_repeat)
    .Set(Am_SELECTIVE_REPEAT_ON_NEW_METHOD, group_objects_sel_repeat_new)
    .Add(Am_SELECTION_WIDGET, (0L))
    .Set(Am_ACCELERATOR, Am_Input_Char("CONTROL_p"))
    .Add(Am_SLOT_FOR_THIS_COMMAND_INACTIVE, (int)Am_GROUP_INACTIVE)
    .Add(Am_ITEM_PROTOTYPE, Am_Resize_Parts_Group)
    .Add (Am_OBJECT_MODIFIED_PLACES, 0)
    ;

  Am_Graphics_Ungroup_Command = Am_Command.Create(DSTR("Ungroup_Objects_Command"))
    .Set(Am_LABEL, "Ungroup")
    .Set(Am_ACTIVE, ungroup_active)
    .Set(Am_DO_METHOD, ungroup_objects)
    .Set(Am_UNDO_METHOD, ungroup_objects_undo)
    .Set(Am_REDO_METHOD, ungroup_objects_undo)
    .Set(Am_SELECTIVE_UNDO_ALLOWED, ungroup_selective_undo_allowed)
    .Set(Am_SELECTIVE_UNDO_METHOD, ungroup_objects_sel_undo)
    .Set(Am_SELECTIVE_REPEAT_SAME_ALLOWED, ungroup_selective_repeat_allowed)
    .Set(Am_SELECTIVE_REPEAT_SAME_METHOD, ungroup_objects_sel_repeat)
    .Set(Am_SELECTIVE_REPEAT_NEW_ALLOWED, ungroup_selective_repeat_new_allowed)
    .Set(Am_SELECTIVE_REPEAT_ON_NEW_METHOD, ungroup_objects_sel_repeat_new)
    .Add(Am_SELECTION_WIDGET, (0L))
    .Set(Am_ACCELERATOR, Am_Input_Char("CONTROL_h"))
    .Add(Am_SLOT_FOR_THIS_COMMAND_INACTIVE, (int)Am_UNGROUP_INACTIVE)
    .Add (Am_OBJECT_MODIFIED_PLACES, 0)
    ;

  Am_Undo_Command = Am_Command.Create(DSTR("Undo_Command"))
    .Set(Am_LABEL, "Undo")
    .Set(Am_ACTIVE, undo_cmd_active)
    .Set(Am_DO_METHOD, undo_cmd_do_undo)
    .Set(Am_UNDO_METHOD, (0L))
    .Set(Am_REDO_METHOD, (0L))
    .Set(Am_SELECTIVE_UNDO_METHOD, (0L))
    .Set(Am_SELECTIVE_REPEAT_SAME_METHOD, (0L))
    .Set(Am_SELECTIVE_REPEAT_ON_NEW_METHOD, (0L))
    .Set(Am_SELECTIVE_UNDO_ALLOWED, Am_Selective_Allowed_Return_False)
    .Set(Am_SELECTIVE_REPEAT_SAME_ALLOWED,
	 Am_Selective_Allowed_Return_False)
    .Set(Am_SELECTIVE_REPEAT_NEW_ALLOWED,
	 Am_Selective_New_Allowed_Return_False)
    .Add(Am_SELECTION_WIDGET, (0L))
    .Set(Am_ACCELERATOR, Am_Input_Char("CONTROL_z"))
    // need a parent so the undo itself is not pushed onto the undo stack
    .Set(Am_IMPLEMENTATION_PARENT, true)
    ;
  Am_Redo_Command = Am_Command.Create(DSTR("Redo_Command"))
    .Set(Am_LABEL, "Redo")
    .Set(Am_ACTIVE, redo_cmd_active)
    .Set(Am_DO_METHOD, redo_cmd_do_redo)
    .Set(Am_UNDO_METHOD, (0L))
    .Set(Am_REDO_METHOD, (0L))
    .Set(Am_SELECTIVE_UNDO_METHOD, (0L))
    .Set(Am_SELECTIVE_REPEAT_SAME_METHOD, (0L))
    .Set(Am_SELECTIVE_REPEAT_ON_NEW_METHOD, (0L))
    .Set(Am_SELECTIVE_UNDO_ALLOWED, Am_Selective_Allowed_Return_False)
    .Set(Am_SELECTIVE_REPEAT_SAME_ALLOWED,
	 Am_Selective_Allowed_Return_False)
    .Set(Am_SELECTIVE_REPEAT_NEW_ALLOWED,
	 Am_Selective_New_Allowed_Return_False)
    .Add(Am_SELECTION_WIDGET, (0L))
    .Set(Am_ACCELERATOR, Am_Input_Char("CONTROL_SHIFT_Z"))
    // need a parent so the undo itself is not pushed onto the undo stack
    .Set(Am_IMPLEMENTATION_PARENT, true)
    ;
  
  Am_Graphics_To_Top_Command = Am_Command.Create(DSTR("To_Top_Objects_Command"))
    .Set(Am_LABEL, "To Top")
    .Set(Am_ACTIVE, Am_Active_If_Selection)
    .Set(Am_DO_METHOD, to_top_objects)
    .Set(Am_UNDO_METHOD, to_top_objects_undo)
    .Set(Am_REDO_METHOD, to_top_objects_undo)
    .Set(Am_SELECTIVE_UNDO_ALLOWED, Am_Selective_Allowed_Return_False)
    .Set(Am_SELECTIVE_UNDO_METHOD, (0L))
    .Set(Am_SELECTIVE_REPEAT_SAME_METHOD, to_top_objects_sel_repeat)
    .Set(Am_SELECTIVE_REPEAT_ON_NEW_METHOD, to_top_objects_sel_repeat_new)
    .Add(Am_SELECTION_WIDGET, (0L))
    .Set(Am_ACCELERATOR, Am_Input_Char("CONTROL_>"))
    .Add(Am_SLOT_FOR_THIS_COMMAND_INACTIVE, (int)Am_TO_TOP_INACTIVE)
    .Add (Am_OBJECT_MODIFIED_PLACES, 0)
    ;
  
  Am_Graphics_To_Bottom_Command =
    	Am_Command.Create(DSTR("To_Bottom_Objects_Command"))
    .Set(Am_LABEL, "To Bottom")
    .Set(Am_ACTIVE, Am_Active_If_Selection)
    .Set(Am_DO_METHOD, to_bottom_objects)
    .Set(Am_UNDO_METHOD, to_bottom_objects_undo)
    .Set(Am_REDO_METHOD, to_bottom_objects_undo)
    .Set(Am_SELECTIVE_UNDO_ALLOWED, Am_Selective_Allowed_Return_False)
    .Set(Am_SELECTIVE_UNDO_METHOD, (0L))
    .Set(Am_SELECTIVE_REPEAT_SAME_METHOD, to_bottom_objects_sel_repeat)
    .Set(Am_SELECTIVE_REPEAT_ON_NEW_METHOD, to_bottom_objects_sel_repeat_new)
    .Add(Am_SELECTION_WIDGET, (0L))
    .Set(Am_ACCELERATOR, Am_Input_Char("CONTROL_<"))
    .Add(Am_SLOT_FOR_THIS_COMMAND_INACTIVE, (int)Am_TO_BOTTOM_INACTIVE)
    .Add (Am_OBJECT_MODIFIED_PLACES, 0) 
   ;

  Am_Quit_No_Ask_Command = Am_Command.Create(DSTR("Quit_No_Ask_Command"))
    .Set(Am_LABEL, quit_or_exit)
    .Set(Am_ACTIVE, true)
    .Set(Am_DO_METHOD, quit_no_ask)
    .Set(Am_ACCELERATOR, Am_Input_Char("CONTROL_q"))
	 // *** BUG doesn't work ** control_q_or_none)
    ;
  Am_Cycle_Value_Command = Am_Command.Create(DSTR("Cycle_Value_Command"))
    .Set(Am_VALUE, 0) //access this slot to get the index of the current label
    .Add(Am_COMPUTE_INTER_VALUE, 0) //used internally
    .Add(Am_LABEL_LIST, Am_Value_List())
    .Set(Am_LABEL, cycle_value_get_string)
    .Set(Am_ACTIVE, true)
    .Set(Am_DO_METHOD, cycle_value_do)
    .Set(Am_UNDO_METHOD, cycle_value_undo)
    .Set(Am_REDO_METHOD, cycle_value_undo)
    .Set(Am_SELECTIVE_UNDO_ALLOWED, Am_Selective_Allowed_Return_True)
    .Set(Am_SELECTIVE_UNDO_METHOD, cycle_value_sel_undo)
    .Set(Am_SELECTIVE_REPEAT_SAME_ALLOWED, Am_Selective_Allowed_Return_True)
    .Set(Am_SELECTIVE_REPEAT_SAME_METHOD, cycle_value_sel_repeat)
    .Set(Am_SELECTIVE_REPEAT_NEW_ALLOWED,
	 Am_Selective_New_Allowed_Return_True)
    .Set(Am_SELECTIVE_REPEAT_ON_NEW_METHOD, cycle_value_repeat_new)
    ;

  Am_Open_Command = Am_Command.Create(DSTR("Open_Command"))
    .Set(Am_LABEL, "Open...")
    .Set(Am_ACTIVE, true)
    .Set(Am_DO_METHOD, open_do)
    .Add(Am_HANDLE_OPEN_SAVE_METHOD, (0L)) //must be supplied
    .Set(Am_ACCELERATOR, Am_Input_Char("CONTROL_o"))
    .Set(Am_UNDO_METHOD, (0L))
    .Set(Am_REDO_METHOD, (0L))
    .Set(Am_SELECTIVE_UNDO_ALLOWED, Am_Selective_Allowed_Return_True)
    .Set(Am_SELECTIVE_UNDO_METHOD, (0L))
    .Set(Am_SELECTIVE_REPEAT_SAME_ALLOWED, Am_Selective_Allowed_Return_False)
    .Set(Am_SELECTIVE_REPEAT_SAME_METHOD, (0L))
    .Set(Am_SELECTIVE_REPEAT_NEW_ALLOWED,
	 Am_Selective_New_Allowed_Return_False)
    .Set(Am_SELECTIVE_REPEAT_ON_NEW_METHOD, (0L))
    ;
  Am_Save_Command = Am_Command.Create(DSTR("Save_Command"))
    .Set(Am_LABEL, "Save")
    .Set(Am_ACTIVE, true)
    .Set(Am_DO_METHOD, save_do)
    .Add(Am_HANDLE_OPEN_SAVE_METHOD, (0L)) //must be supplied
    .Set(Am_ACCELERATOR, Am_Input_Char("CONTROL_s"))
    .Set(Am_UNDO_METHOD, (0L))
    .Set(Am_REDO_METHOD, (0L))
    .Set(Am_SELECTIVE_UNDO_ALLOWED, Am_Selective_Allowed_Return_False)
    .Set(Am_SELECTIVE_UNDO_METHOD, (0L))
    .Set(Am_SELECTIVE_REPEAT_SAME_ALLOWED, Am_Selective_Allowed_Return_False)
    .Set(Am_SELECTIVE_REPEAT_SAME_METHOD, (0L))
    .Set(Am_SELECTIVE_REPEAT_NEW_ALLOWED,
	 Am_Selective_New_Allowed_Return_False)
    .Set(Am_SELECTIVE_REPEAT_ON_NEW_METHOD, (0L))
    ;
  Am_Save_As_Command = Am_Command.Create(DSTR("Save_As_Command"))
    .Set(Am_LABEL, "Save As...")
    .Set(Am_ACTIVE, true)
    .Set(Am_DO_METHOD, save_as_do)
    .Add(Am_HANDLE_OPEN_SAVE_METHOD, (0L)) //must be supplied
    .Set(Am_ACCELERATOR, (0L)) // no default accelerator
    .Set(Am_UNDO_METHOD, (0L))
    .Set(Am_REDO_METHOD, (0L))
    .Set(Am_SELECTIVE_UNDO_ALLOWED, Am_Selective_Allowed_Return_False)
    .Set(Am_SELECTIVE_UNDO_METHOD, (0L))
    .Set(Am_SELECTIVE_REPEAT_SAME_ALLOWED, Am_Selective_Allowed_Return_False)
    .Set(Am_SELECTIVE_REPEAT_SAME_METHOD, (0L))
    .Set(Am_SELECTIVE_REPEAT_NEW_ALLOWED,
	 Am_Selective_New_Allowed_Return_False)
    .Set(Am_SELECTIVE_REPEAT_ON_NEW_METHOD, (0L))
    ;

  
}


static Am_Initializer* initializer =
	new Am_Initializer(DSTR("Editing_Commands"), init, 5.5);
