#include <amulet.h>
#include "externs.h"

#include WIDGETS_ADVANCED__H
#include INTER_ADVANCED__H //needed for Am_Inter_Tracing
#include GEM__H
#include REGISTRY__H
#include OPAL_ADVANCED__H
#include OBJECT__H
#include VALUE_LIST__H
#include STANDARD_SLOTS__H

Am_Object Modified_Selection_Widget;
extern Am_Formula compute_drag_start_when;

inline int IMIN(int a, int b) { return (a < b)? a : b;}
inline int IMAX(int a, int b) { return (a > b)? b : a;}

////////  THese functions are defined in selection_widgets.cc  ////////////
// Constraint_Widget.cc is modified version of Am_Selection_Widget
// These functions are needed to prevent Am_Error to happen when
// background of window is selected
// These functions are copied so that when these functions change in selection_widgets.cc
// one does not have to re edit them.
void modified_set_commands_for_sel_values(Am_Object & cmd, Am_Value clicked_obj,
					  Am_Value new_value, Am_Value old_value){
  Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING,
		       "++ Setting into " << cmd << " old " << old_value
		       << " new " << new_value);
  cmd.Set(Am_OLD_VALUE, old_value);
  cmd.Set(Am_VALUE, new_value);
  cmd.Set(Am_OBJECT_MODIFIED, clicked_obj);
}

void modified_set_commands_for_sel(Am_Object & cmd, Am_Object &inter, Am_Object& widget,
				   Am_Value clicked_obj,
				   Am_Value new_value, Am_Value old_value){
  Am_Object inter_command, widget_command;
  inter_command = inter.Get_Object(Am_COMMAND);
  if (inter_command.Valid()) 
    modified_set_commands_for_sel_values(inter_command, clicked_obj, new_value, old_value);
  widget_command = widget.Get_Object(Am_COMMAND);
  if (widget_command.Valid()) 
    modified_set_commands_for_sel_values(widget_command, clicked_obj, new_value, old_value);
  if (cmd.Valid()) {
    Am_Object impl_parent = cmd.Get(Am_IMPLEMENTATION_PARENT,
				    Am_RETURN_ZERO_ON_ERROR);
    if (impl_parent.Valid() && impl_parent != widget_command
	&& impl_parent != inter_command) {
      modified_set_commands_for_sel_values(cmd, clicked_obj, new_value, old_value);
      modified_set_commands_for_sel_values(impl_parent, clicked_obj, new_value, old_value);
    }
  }
}
  

void modified_clear_multi_selections(const Am_Object &widget) {
  Am_Value value;
  value=widget.Peek(Am_MULTI_SELECTIONS);
  if (value.Valid () && Am_Value_List::Test (value)) {
    Am_Value_List widget_list (value);
    for (widget_list.Start (); !widget_list.Last (); widget_list.Next ()) {
      Am_Object curr_widget = widget_list.Get ();
      if (curr_widget != widget)
	curr_widget.Set (Am_VALUE, Am_No_Value_List);
      // Clear selection of joined widgets.  NDY: Need to consider undo.
    }
  }
}

void modified_update_data_from_objs(Am_Value objs_value, Am_Value &new_data_value) {
  Am_Object obj, owner;
  int left, top, w, h;
  if (Am_Value_List::Test(objs_value)) {
    Am_Value_List objs_list, new_data_list;
    objs_list = objs_value;
    for(objs_list.Start(); !objs_list.Last(); objs_list.Next()) {
      obj = objs_list.Get();
      owner = obj.Get_Owner();
      left = obj.Get(Am_LEFT);
      top = obj.Get(Am_TOP);
      w = obj.Get(Am_WIDTH);
      h = obj.Get(Am_HEIGHT);
      new_data_list.Add(Am_Inter_Location(false, owner, left, top, w, h));
    }
    new_data_value = new_data_list;
  }
  else if (Am_Object::Test(objs_value)) {
    obj = objs_value;
    owner = obj.Get_Owner();
    left = obj.Get(Am_LEFT);
    top = obj.Get(Am_TOP);
    w = obj.Get(Am_WIDTH);
    h = obj.Get(Am_HEIGHT);
    Am_Inter_Location new_data(false, owner, left, top, w, h);
    new_data_value = new_data;
  }
  else Am_Error("objs not a list or object");
}

static void modified_remove_object_from_selection (const Am_Object& object,
					  const Am_Object& command)
{
  Am_Object widget = command.Get (Am_SELECTION_WIDGET);
  Am_Value_List select_list = widget.Get (Am_VALUE);
  select_list.Start ();
  if (select_list.Member (object)) {
    select_list.Delete ();
    widget.Set (Am_VALUE, select_list);
  }
}


void modified_update_objs_from_value(Am_Value objs_value, Am_Value new_data_value,
			    bool growing, Am_Object& command_obj) {
  int min_left = 29999;
  int min_top  = 29999;
  Am_Object obj;
  Am_Inter_Location new_data;
  if (Am_Value_List::Test(objs_value)) {
    Am_Value_List objs_list, new_data_list;
    if (!Am_Value_List::Test(new_data_value))
      Am_ERROR("list of objs " << objs_value << " but not list of values "
	       << new_data_value);
    objs_list = objs_value;
    new_data_list = new_data_value;
    if (objs_list.Length() != new_data_list.Length())
      Am_ERROR("Lists have different lengths: objs " << objs_list<< " values "
	       << new_data_list);
    int cur_left, cur_top;
    for(objs_list.Start(), new_data_list.Start(); !objs_list.Last();
	objs_list.Next(), new_data_list.Next()) {
      obj = objs_list.Get();
      new_data = new_data_list.Get();
      if (Am_Check_And_Fix_Owner_For_Object (obj, new_data))
        modified_remove_object_from_selection (obj, command_obj);
      Am_Modify_Object_Pos (obj, new_data, growing);
      cur_left = obj.Get(Am_LEFT);
      cur_top = obj.Get(Am_TOP);
      min_left = IMIN(min_left, cur_left);
      min_top = IMIN(min_top, cur_top);
    }
  }
  else if (Am_Object::Test(objs_value)) {
    obj = objs_value;
    new_data = new_data_value;
    if (Am_Check_And_Fix_Owner_For_Object (obj, new_data))
      modified_remove_object_from_selection (obj, command_obj);
    Am_Modify_Object_Pos (obj, new_data, growing);
    min_left = obj.Get(Am_LEFT);
    min_top = obj.Get(Am_TOP);
  }
  else Am_Error("objs not a list or object");
  command_obj.Set(Am_LEFT, min_left);
  command_obj.Set(Am_TOP, min_top);
}

void modified_adjust_all_objects_position_for_undo(Am_Value_List list, int left_offset,
					  int top_offset,
					  Am_Value_List &new_list) {
  Am_Object obj;
  Am_Object owner;
  new_list.Make_Empty();
  int old_left, old_top, new_left, new_top, w, h;
  for(list.Start(); !list.Last(); list.Next()) {
    obj = list.Get();
    owner = obj.Get_Owner();
    old_left = obj.Get(Am_LEFT);
    old_top = obj.Get(Am_TOP);
    w = obj.Get(Am_WIDTH);
    h = obj.Get(Am_HEIGHT);
    new_left = old_left + left_offset;
    new_top = old_top + top_offset;
    Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING, "++Setting " << obj <<
			 " left " << new_left << " top " << new_top);
    obj.Set(Am_LEFT, new_left, Am_NO_ANIMATION);
    obj.Set(Am_TOP, new_top, Am_NO_ANIMATION);
    new_list.Add(Am_Inter_Location(false, owner, new_left, new_top, w, h));
  }
}
void modified_get_min_from_loc(Am_Object & ref_obj, Am_Inter_Location &loc,
		               int &min_left, int& min_top) {
  int a, b, c, d;
  bool as_line;
  Am_Object loc_ref;
  loc.Get_Location(as_line, loc_ref, a, b, c, d);
  if (loc_ref != ref_obj)
    Am_Translate_Coordinates(loc_ref, a, b, ref_obj, a, b);
  if (a < min_left) min_left = a;
  if (b < min_top) min_top = b;
  if (as_line) {
    if (loc_ref != ref_obj)
      Am_Translate_Coordinates(loc_ref, c, d, ref_obj, c, d);
    if (c < min_left) min_left = c;
    if (d < min_top) min_top = d;
  }
}

void modified_get_left_and_top_from_new_value(Am_Value &new_value, Am_Object ref_obj,
				              int &min_left, int &min_top) {
  min_left = 30000;
  min_top = 30000;
  Am_Inter_Location loc;
  if (Am_Inter_Location::Test(new_value)) {
    loc = new_value;
    modified_get_min_from_loc(ref_obj, loc, min_left, min_top);
  }
  else if (Am_Value_List::Test(new_value)) {
    Am_Value item;
    Am_Value_List list = new_value;
    for(list.Start(); !list.Last(); list.Next()) {
      item = list.Get();
      if  (!Am_Inter_Location::Test(item)) {
	Am_ERROR("New value for selective redo is a list containing an non-Am_Inter_Location: " << new_value);
      }
      else {
	loc = item;
	modified_get_min_from_loc(ref_obj, loc, min_left, min_top);
      }
    }
  }
  else Am_ERROR("New value for selective redo not list or Am_Inter_Location: "
		<< new_value);
}

void modified_toggle_object_in_list(const Am_Object &widget, 
				  Am_Object new_object,
				  Am_Value_List &list) {
  list.Start();
  if(list.Member(new_object)) {
    Am_INTER_TRACE_PRINT(widget, "Selection handle removing " << new_object
                         << " from " << widget);    
    list.Delete();
  }
  else { // not a member, add it
    Am_INTER_TRACE_PRINT(widget, "Selection handle adding " << new_object <<
			 " to " << widget);
    // Not allowed to have objects selected that are parts of
    // new_object, owners of new_object, or have owners different from
    // new_object.  This can happen when you have select into groups.
    for (list.Start (); !list.Last (); list.Next ()) {
      Am_Object item = list.Get ();
      if (item.Is_Part_Of (new_object) ||
	  new_object.Is_Part_Of (item) ||
	  (!item.Is_Instance_Of(main_group_rect_proto) &&
            item.Get_Owner () != new_object.Get_Owner ()))
	list.Delete ();
    }
    list.Add(new_object);
  }
}


Am_Define_Method(Am_Mouse_Event_Method, void, modified_sel_object,
		 (Am_Object inter, int mouse_x, int mouse_y,
		  Am_Object ref_obj, Am_Input_Char ic)) {
  Am_Object new_object;
  bool window_border = false;
  if (inter.Valid()) {
    Am_Object widget = inter.Get_Owner();
    modified_clear_multi_selections(widget);
    new_object = inter.Get(Am_START_OBJECT);
    Am_Value_List list = widget.Get(Am_VALUE);
    
    if(new_object.Valid ()) { //valid object
      Am_Value new_value;
      bool toggle_in_set = ic.shift;
      Am_Object main_rect = widget.Get_Sibling(MAIN_GROUP_RECT);

      if (new_object == widget) { // then clicked in the background
        int width = (int)main_rect.Get(Am_WIDTH);
        int height = (int)main_rect.Get(Am_HEIGHT);
        int top = (int)main_rect.Get_Owner().Get(Am_TOP);

        if(mouse_x < 20 || (mouse_x > (width - 20)) 
           || (mouse_y < (top + 20)) || (mouse_y > (top + height - 20)))
            window_border = true;    
        if (toggle_in_set) {   //  don't do anything
          if(window_border){
            new_object = main_rect;
            list.Start(); 
            if(list.Member(main_rect)) list.Delete();
            else list.Add(new_object);
            new_value = list;
          }  else {          
             Am_Abort_Interactor(inter);
             return;   
          }
        }
        else {  // select nothing
          Am_INTER_TRACE_PRINT(widget, "Selection handle setting empty for "
			       << widget);
          if(window_border){
            list.Make_Empty();
            list.Add(main_rect);
            new_object = main_rect;
          }
          else{
            list = Am_Value_List();
            new_object = NULL;
          }
          new_value = list;
        }
      }
      else { // over a specific object
	Am_Value value;
        Am_Object diff_obj;
        Am_Where_Method method;
        bool abort_ok = true;
    	value=widget.Peek(Am_SELECT_CLOSEST_POINT_STYLE);
        if (value.Valid()) {
	    method = widget.Get(Am_SELECT_CLOSEST_POINT_METHOD);
	    diff_obj =
	    method.Call(widget, new_object, ref_obj, mouse_x, mouse_y);
	    if (diff_obj.Valid()) abort_ok = false;
	}
        list.Start();
	if (toggle_in_set)
            modified_toggle_object_in_list(widget, new_object, list);
	else { //if object is selected, do nothing, otherwise,
	  // make new_object be the only selection
	  if(list.Member(new_object)){
             if (abort_ok) {
	         Am_Abort_Interactor(inter); //make sure not queued for undo
	         return;
	     }
          }
	  else {
	    Am_INTER_TRACE_PRINT(widget, "Selection handle setting " << widget
				 << " to contain only " << new_object);
	    list.Make_Empty();
	    list.Add(new_object);
	  }
	}
        new_value = list;
      }
      Am_Value old_value;
      old_value = widget.Peek(Am_VALUE);
      widget.Set(Am_VALUE, new_value);
      Am_Object command = inter.Get_Object(Am_COMMAND);
      modified_set_commands_for_sel(command, inter, widget, new_object, new_value, old_value);
    }
  }
}

//modified so that error "Moving object from different group" is not
//displayed for main_group_rect_proto object

void modified_calculate_group_size(Am_Value_List list, int &min_left, int &min_top, 
			  int &max_right, int &max_bottom,
			  Am_Object &owner) {
  min_left = 29999;
  min_top  = 29999;
  max_right = -29999;
  max_bottom = -29999;
  int cur_left, cur_top;
  Am_Object obj, cur_owner, added_obj;
  for(list.Start(); !list.Last(); list.Next()) {
    obj = list.Get();
    cur_owner = obj.Get_Owner();
    if (cur_owner.Valid()) {
      if (!owner.Valid()){
        owner = cur_owner;
        added_obj = (Am_Object)obj;
      }
      else if (owner != cur_owner && 
               !obj.Is_Instance_Of(main_group_rect_proto) &&
               !added_obj.Is_Instance_Of(main_group_rect_proto)){
        Am_Error("Moving objects from different groups.");
      }
    }
    if (obj.Is_Instance_Of (Am_Line)) {
      int x1, x2, y1, y2;
      x1 = obj.Get(Am_X1);
      x2 = obj.Get(Am_X2);
      y1 = obj.Get(Am_Y1);
      y2 = obj.Get(Am_Y2);
      min_left = IMIN (IMIN (min_left, x1), x2);
      min_top = IMIN (IMIN (min_top, y1), y2);
      max_right = IMAX (IMAX (max_right, x1), x2);
      max_bottom = IMAX (IMAX (max_bottom, y1), y2);
    }
    else {
      cur_left = obj.Get(Am_LEFT);
      cur_top = obj.Get(Am_TOP);
      min_left = IMIN(min_left, cur_left);
      min_top = IMIN(min_top, cur_top);
      max_right = IMAX(max_right, cur_left + (int)obj.Get(Am_WIDTH));
      max_bottom = IMAX(max_bottom, cur_top + (int)obj.Get(Am_HEIGHT));
    }
  }
}

//figure out the bounds of all the selected objects, and assign it into
//fake_group
void modified_calculate_fake_group_size_and_set(Am_Object& fake_group,
				       Am_Value_List& list) {
  int min_left = 29999;
  int min_top  = 29999;
  int max_right = -29999;
  int max_bottom = -29999;
  int left_offset, top_offset;
  Am_Object owner;
  modified_calculate_group_size(list, min_left, min_top, max_right, max_bottom, owner);
  Am_Translate_Coordinates(owner, 0, 0, fake_group.Get_Owner(),
			   left_offset, top_offset);
  fake_group.Set(Am_LEFT, min_left + left_offset);
  fake_group.Set(Am_TOP, min_top + top_offset);
  int wh = max_right - min_left;
  if (wh == 0) wh = 1;
  fake_group.Set(Am_WIDTH, wh);
  wh = max_bottom - min_top;
  if (wh == 0) wh = 1;
  fake_group.Set(Am_HEIGHT, wh);
  // cout << "Calculate fake group " << fake_group << " size = " << min_left
  // << "," << min_top << "," << max_right - min_left << ","
  // << max_bottom - min_top << endl << flush;
}


//This function is defined in selection_widgets.cc in a method form.
Am_Object owner_start_where_operates_on_function(Am_Object inter,
                    Am_Object object, Am_Object event_window, 
                    int x, int y){
    Am_Object operates_on = object.Get(Am_OPERATES_ON);
    Am_Where_Method method;
    method = object.Get(Am_START_WHERE_TEST);
    return method.Call(inter, operates_on, event_window, x, y);
}


Am_Define_Method(Am_Where_Method, Am_Object, modified_owner_start_where_or_fake,
		 (Am_Object inter, Am_Object object, Am_Object event_window,
		  int x, int y)) {
  Am_Object ret = owner_start_where_operates_on_function(inter, object,
						     event_window, x, y);
  if (ret.Valid()) { //have a valid selection
    Am_Value_List list;
    list = object.Get(Am_VALUE);
    list.Start();
    if (!list.Member(ret))
      ret = Am_No_Object;  //click caused the object to now not be selected
    else if (list.Length() > 1) { // otherwise, ret is OK
      Am_Object fake_group = object.Get (Am_FAKE_GROUP);
      if (!fake_group.Get_Owner ().Valid ()) {
        fake_group = fake_group.Create ();
        object.Add_Part (fake_group);
        object.Set (Am_FAKE_GROUP, fake_group);
      }
      else if (fake_group.Get_Owner () != object) {
        fake_group.Remove_From_Owner ();
        object.Add_Part (fake_group);
      }
      modified_calculate_fake_group_size_and_set(fake_group, list);
      ret = fake_group;
    }
  }
  return ret;
}

//// for growing error is not displayed

void modified_update_new_objs_from_pos(Am_Value objs_value, int left, int top,
			      Am_Value &new_data_value,
			      bool growing, Am_Object command_obj) {
  if (growing) // then must be a single object, so new_data_value will be OK
    modified_update_objs_from_value(objs_value, new_data_value, growing, command_obj);
  else {
    Am_Object obj;
    if (Am_Value_List::Test(objs_value)) {
      Am_Value_List objs_list, new_data_value_list;
      objs_list = objs_value;
      // calculate group offset and move
      int min_left, min_top, max_right, max_bottom;
      Am_Object owner;
      modified_calculate_group_size(objs_list, min_left, min_top, max_right, max_bottom, owner);
      modified_adjust_all_objects_position_for_undo(objs_list, left - min_left,
					   top - min_top, new_data_value_list);
      new_data_value = new_data_value_list;
      command_obj.Set(Am_LEFT, min_left);
      command_obj.Set(Am_TOP, min_top);
    }
    else if (objs_value.type == Am_OBJECT) {
      obj = objs_value;
      obj.Set(Am_LEFT, left, Am_NO_ANIMATION);
      obj.Set(Am_TOP, top, Am_NO_ANIMATION);
      command_obj.Set(Am_LEFT, left);
      command_obj.Set(Am_TOP, top);
    }
  }
}

void modified_sel_move_grow_general_undo_redo(Am_Object command_obj, bool undo,
				     bool selective, bool reload_data,
				     Am_Value new_sel = Am_No_Value,
				     Am_Value new_value = Am_No_Value) {
  Am_Object inter;
  inter = command_obj.Get(Am_SAVED_OLD_OWNER);
  
  if (reload_data && new_sel.Valid()) {
    command_obj.Set(Am_OBJECT_MODIFIED, new_sel);
    Am_Object parent;
    parent = command_obj.Get(Am_IMPLEMENTATION_PARENT);
    if (parent.Valid())
      parent.Set(Am_OBJECT_MODIFIED, new_sel);
  }
  else new_sel=command_obj.Peek(Am_OBJECT_MODIFIED);
  
  #ifdef DEBUG
  if (inter.Valid () && Am_Inter_Tracing(inter)) {
    if (selective) cout << "Selective ";
    if (undo) cout << "Undo"; else cout << "repeat";
    cout << " command " << command_obj << " on " << new_sel <<endl << flush;
  }
  #endif
  if (new_sel.Valid ()) {
    Am_Value old_data_value, new_data_value;

    old_data_value=command_obj.Peek(Am_OLD_VALUE);
    if (!reload_data) new_data_value=command_obj.Peek(Am_VALUE);
    bool growing = command_obj.Get(Am_GROWING);
    if (selective) {
      if (undo) modified_update_data_from_objs(new_sel, new_data_value);
      else      modified_update_data_from_objs(new_sel, old_data_value);
    }
    if (undo) {
      modified_update_objs_from_value(new_sel, old_data_value, growing, command_obj);
      // swap current and old values, in case undo or undo-the-undo again
      command_obj.Set(Am_OLD_VALUE, new_data_value);
      command_obj.Set(Am_VALUE, old_data_value);
    }
    else {
      if (reload_data) {
	int left, top;
	if (new_value.Valid()) {
	  new_data_value = new_value;
	  Am_Object ref_obj = command_obj.Get(Am_SAVED_OLD_OBJECT_OWNER);
	  modified_get_left_and_top_from_new_value(new_value, ref_obj, left, top);
	}
	else {
	  left = command_obj.Get(Am_LEFT);
	  top = command_obj.Get(Am_TOP);
	}
	modified_update_new_objs_from_pos(new_sel, left, top, new_data_value,
				 growing, command_obj);
	command_obj.Set(Am_LEFT, left);
	command_obj.Set(Am_TOP, top);
      }
      else modified_update_objs_from_value(new_sel, new_data_value,
				  growing, command_obj);
      if (selective) command_obj.Set(Am_OLD_VALUE, old_data_value);
      if (reload_data) command_obj.Set(Am_VALUE, new_data_value);
    }
  }
}


Am_Define_Method(Am_Object_Method, void, modified_sel_move_undo,
		 (Am_Object command_obj)) {
  modified_sel_move_grow_general_undo_redo(command_obj, true, false, false);
}
Am_Define_Method(Am_Object_Method, void, modified_sel_move_selective_undo,
		 (Am_Object command_obj)){
  modified_sel_move_grow_general_undo_redo(command_obj, true, true, false);
}
Am_Define_Method(Am_Object_Method, void, modified_sel_move_selective_repeat,
		 (Am_Object command_obj)){
  modified_sel_move_grow_general_undo_redo(command_obj, false, true, false);
}
Am_Define_Method(Am_Selective_Repeat_New_Method, void,
		 modified_sel_move_selective_repeat_new,
		 (Am_Object command_obj, Am_Value new_sel, Am_Value new_val)){
  modified_sel_move_grow_general_undo_redo(command_obj, false, true, true, new_sel,
				  new_val);
}




Am_Object Modified_Selection_Widget_Initialize(){
  Modified_Selection_Widget = Am_Selection_Widget.Create()
              .Get_Object(Am_INTERACTOR)
              .Set(Am_DO_METHOD, modified_sel_object)
              .Get_Owner()
              .Get_Object(Am_MOVE_INTERACTOR)
              .Set(Am_START_WHERE_TEST, modified_owner_start_where_or_fake)
              .Get_Object(Am_COMMAND)
              .Set(Am_UNDO_METHOD, modified_sel_move_undo)
              .Set(Am_REDO_METHOD, modified_sel_move_undo)
              .Set(Am_SELECTIVE_UNDO_METHOD, modified_sel_move_selective_undo)
              .Set(Am_SELECTIVE_REPEAT_SAME_METHOD, modified_sel_move_selective_repeat)
              .Set(Am_SELECTIVE_REPEAT_ON_NEW_METHOD, modified_sel_move_selective_repeat_new)              
              .Get_Owner()
              .Get_Owner()
              .Get_Object(Am_SELECT_OUTSIDE_INTERACTOR)
              .Set(Am_DO_METHOD, modified_sel_object)
              .Get_Owner();

  return Modified_Selection_Widget;
}
