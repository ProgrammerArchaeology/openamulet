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

// Support for Searching for objects
//
// Designed and implemented by Brad Myers


#include <am_inc.h>
#include AM_IO__H

#include STANDARD_SLOTS__H
#include VALUE_LIST__H
#include WIDGETS__H
#include SCRIPTING_ADVANCED__H
#include SCRIPTING__H
#include INITIALIZER__H
#include INTER_ADVANCED__H //for Am_Inter_Find_Undo_Handler

Am_Slot_Key Am_SEARCH_DIALOG = Am_Register_Slot_Name ("~Am_SEARCH_DIALOG~");
Am_Slot_Key Am_MAIN_BUTTON = Am_Register_Slot_Name ("~Am_MAIN_BUTTON~");
Am_Slot_Key Am_USE_ITEM = Am_Register_Slot_Name ("~Am_USE_ITEM~");
Am_Slot_Key Am_ITEM_VALUE = Am_Register_Slot_Name ("~Am_ITEM_VALUE~");
Am_Slot_Key Am_SLOTS_TO_ITEMS =
		Am_Register_Slot_Name ("~Am_SLOTS_TO_ITEMS~");
Am_Slot_Key Am_TYPE_USE_ITEM =
		Am_Register_Slot_Name ("~Am_TYPE_USE_ITEM~");
Am_Slot_Key Am_SLOTS_AND_VALUES_USED =
		Am_Register_Slot_Name ("~Am_SLOTS_AND_VALUES_USED~");

Am_Object search_item_proto;
Am_Object search_use_item_proto;
Am_Object Am_Search_Window;
Am_Object Am_Find_Command;
Am_Object Am_Show_Search_Command;
Am_Object Am_Fake_Start_Object;

Am_Font search_bold_font, search_big_font;

Am_Value_List am_all_proto_names;

#define ITEM_SIZE 30
#define WINDOW_WIDTH 450
#define OFFSET 5
#define INDENT_OFFSET 30

void Am_TEMP_Register_Prototype(const char* name, Am_Object& proto) {
  am_all_proto_names.Add(Am_Assoc(name, proto));
}



/////////////////////////////////////////////////////////////////////////

void set_slots_and_values(Am_Object &fake_start_object,
			  Am_Value_List &slots_and_values_used) {
  Am_Assoc as;
  Am_Slot_Key key;
  Am_Value target_v;
  Am_Value_List just_slots;
  for (slots_and_values_used.Start(); !slots_and_values_used.Last();
       slots_and_values_used.Next()) {
    as = slots_and_values_used.Get();
    key = Am_Am_Slot_Key(as.Value_1());
    target_v = as.Value_2();
    fake_start_object.Set(key, target_v, Am_OK_IF_NOT_THERE);
    just_slots.Add((int)key);
  }
  fake_start_object.Set(Am_SLOTS_TO_SAVE, just_slots, Am_OK_IF_NOT_THERE);
}

Am_Value_List get_new_slots_and_values(Am_Object &cmd, Am_Value &new_value,
				       Am_Value new_obj_val) {
  Am_Object fake_start_object;
  if (new_obj_val.Valid() && new_obj_val.type == Am_OBJECT)
    fake_start_object = new_obj_val;
  else fake_start_object = cmd.Get(Am_START_OBJECT);
  // **** do something with new_value ****
  Am_Value_List slots_and_values_used;
  Am_Value_List just_slots = fake_start_object.Get(Am_SLOTS_TO_SAVE);
  Am_Am_Slot_Key key;
  Am_Value target_v;
  for (just_slots.Start(); !just_slots.Last(); just_slots.Next()) {
    key = just_slots.Get();
    target_v = fake_start_object.Get(key);
    slots_and_values_used.Add(Am_Assoc(key, target_v));
  }
  if (am_sdebug)
   std::cout << "Create new slots_and_values_used from " << cmd << " and new val "
	 << new_value << " start object " << fake_start_object
	 << " list " << slots_and_values_used <<std::endl <<std::flush;
  return slots_and_values_used;
}

bool test_match_object(Am_Object &object,
		       Am_Value_List &slots_and_values_used) {
  Am_Assoc as;
  Am_Am_Slot_Key key;
  Am_Value object_v, target_v;
  for (slots_and_values_used.Start(); !slots_and_values_used.Last();
       slots_and_values_used.Next()) {
    as = slots_and_values_used.Get();
    key = as.Value_1();
    target_v = as.Value_2();
    if ((Am_Slot_Key)key == Am_PROTOTYPE) {
      Am_Object proto = target_v;
      if (!object.Is_Instance_Of(proto)) {
	return false;
      }
    }
    else {
      object_v = object.Peek(key);
      if (!object_v.Exists() || object_v != target_v) {
	return false;
      }
    }
  }
  return true;
}

Am_Object do_search_forward(Am_Value_List &parts, Am_Value_List &old_sel_list,
			    Am_Value_List &slots_and_values_used) {
  //move parts list to the last selected part
  parts.Start ();
  Am_Object object;
  if (old_sel_list.Valid()) {
    int index = -1;
    for (; !parts.Last (); parts.Next ()) {
      object = parts.Get();
      if (old_sel_list.Start_Member(object))
	index = parts.Nth();
    }
    if (index == -1) Am_ERROR("Lost old value " << old_sel_list
			      << " from parts list for selection widget "
			      << parts);
    if (index == parts.Length() -1 ) { //already at end
      return Am_No_Object;
    }
    parts.Move_Nth(index+1);
  }
  //now search forward for new object
  for (; !parts.Last (); parts.Next ()) {
    object = parts.Get();
    if (test_match_object(object, slots_and_values_used))
      return object;
  }
  return Am_No_Object;
}

//don't want any of the valid options to be zero
enum Am_Which_Search_Direction { Am_No_Search,
				 Am_Search_Forward, Am_Search_Back,
				 Am_Search_Up, Am_Search_Down,
			         Am_Search_Left, Am_Search_Right,
			         Am_Search_In, Am_Search_Out };

// To right: go from right-most selected left edge to first (left-most) object
// that overlaps vertically
// To inside: backmost object in front of selected that is fully inside
// To outside: frontmost object in back of selected that is fully outside
Am_Object do_search_dir(Am_Which_Search_Direction dir,
			Am_Value_List &parts, Am_Value_List &old_sel_list,
			Am_Value_List &slots_and_values_used) {

  //compute the starting positions
  int max_l = -32000;
  int min_r = 32000;
  int min_t = 32000;
  int max_b = -32000;

  int min_l = 32000;
  int max_r = -32000;
  int max_t = -32000;
  int min_b = 32000;
  int this_l, this_t, this_b, this_r;
  Am_Object object;
  if (old_sel_list.Valid()) {
    for (old_sel_list.Start (); !old_sel_list.Last (); old_sel_list.Next ()) {
      object = old_sel_list.Get();
      this_l = object.Get(Am_LEFT);
      this_t = object.Get(Am_TOP);
      this_r = this_l + (int)object.Get(Am_WIDTH) - 1;
      this_b = this_t + (int)object.Get(Am_HEIGHT) - 1;
      if (this_l > max_l) max_l = this_l;
      if (this_t < min_t) min_t = this_t;
      if (this_b > max_b) max_b = this_b;
      if (this_r < min_r) min_r = this_r;
      if (this_l < min_l) min_l = this_l;
      if (this_t > max_t) max_t = this_t;
      if (this_b < min_b) min_b = this_b;
      if (this_r > max_r) max_r = this_r;
    }
  }
  else { //if no objects, set edge values
    min_l = -32000;
    max_l = -32000;
    max_t = -32000;
    min_t = -32000;
    min_r = 32000;
    max_r = 32000;
    min_b = 32000;
    max_b = 32000;
  }
  //now search forward for new object
  int target_min_amount = 32000;
  int target_max_amount = 0;
  Am_Object cur_target_object;

  if (dir == Am_Search_Out) { //have to go in opposite order
    for (parts.End(); !parts.First (); parts.Prev ()) {
      object = parts.Get();
      if (!old_sel_list.Start_Member(object) &&
	  test_match_object(object, slots_and_values_used)) {
	this_l = object.Get(Am_LEFT);
	this_t = object.Get(Am_TOP);
	this_r = this_l + (int)object.Get(Am_WIDTH) - 1;
	this_b = this_t + (int)object.Get(Am_HEIGHT) - 1;
	if (this_l <= min_l && this_t <= min_t &&
	    this_b >= max_b && this_r >= max_r) {
	  return object;
	}
      }
    }
  }
  else {
    for (parts.Start(); !parts.Last (); parts.Next ()) {
      object = parts.Get();
      if (!old_sel_list.Start_Member(object) &&
	  test_match_object(object, slots_and_values_used)) {
	this_l = object.Get(Am_LEFT);
	this_t = object.Get(Am_TOP);
	this_r = this_l + (int)object.Get(Am_WIDTH) - 1;
	this_b = this_t + (int)object.Get(Am_HEIGHT) - 1;
	switch (dir) {
	case Am_Search_Up:
	  if (this_b <= min_b && this_b > target_max_amount &&
	      this_r >= min_l && this_l <= max_r) {
	    target_max_amount = this_b;
	    cur_target_object = object;
	  }
	  break;
	case Am_Search_Down:
	  if (this_t >= max_t && this_t < target_min_amount &&
	      this_r >= min_l && this_l <= max_r) {
	    target_min_amount = this_t;
	    cur_target_object = object;
	  }
	  break;
	case Am_Search_Left:
	  if (this_r <= min_r && this_r > target_max_amount &&
	      this_b >= min_t && this_t <= max_b) {
	    target_max_amount = this_r;
	    cur_target_object = object;
	  }
	  break;
	case Am_Search_Right:
	  if (this_l >= max_l && this_l < target_min_amount &&
	      this_b >= min_t && this_t <= max_b) {
	    target_min_amount = this_l;
	    cur_target_object = object;
	  }
	  break;
	case Am_Search_In:
	  if (this_l >= min_l && this_t >= min_t &&
	      this_b <= max_b && this_r <= max_r) {
	    return object;
	  }
	  break;
	default: Am_ERROR("Bad dir " << (int)dir);
	}
      }
    }
  }
  return cur_target_object;
}

Am_Object do_search_backward(Am_Value_List &parts,
			     Am_Value_List &old_sel_list,
			     Am_Value_List &slots_and_values_used) {
  //move parts list to the first selected part
  parts.End ();
  Am_Object object;
  if (old_sel_list.Valid()) {
    int index = -1;
    for (; !parts.First (); parts.Prev ()) {
      object = parts.Get();
      if (old_sel_list.Start_Member(object))
	index = parts.Nth();
    }
    if (index == -1) Am_ERROR("Lost old value " << old_sel_list
			      << " from parts list for selection widget "
			      << parts);
    if (index == 0 ) { //already at beginning
      return Am_No_Object;
    }
    parts.Move_Nth(index-1);
  }
  //now search forward for new object
  for (; !parts.First (); parts.Prev ()) {
    object = parts.Get();
    if (test_match_object(object, slots_and_values_used))
      return object;
  }
  return Am_No_Object;
}

void internal_search_using(Am_Object &cmd, Am_Which_Search_Direction dir,
			   Am_Object &sel_widget,
			   Am_Value_List &old_sel_list,
			   Am_Value_List &slots_and_values_used) {
  //here have the list of values to be used in slots_and_values_used

  cmd.Set(Am_OLD_VALUE, old_sel_list);
  cmd.Set(Am_SELECTION_WIDGET, sel_widget, Am_OK_IF_NOT_THERE);
  cmd.Set(Am_OBJECT_MODIFIED, old_sel_list);

  Am_Object inter = cmd.Get_Owner();
  Am_Object main_group = sel_widget.Get(Am_OPERATES_ON);
  Am_Value_List parts = main_group.Get(Am_GRAPHICAL_PARTS);

  Am_Object found;
  if (dir == Am_Search_Forward)
    found = do_search_forward(parts, old_sel_list, slots_and_values_used);
  else if (dir == Am_Search_Back)
    found = do_search_backward(parts, old_sel_list, slots_and_values_used);
  else
    found = do_search_dir(dir, parts, old_sel_list, slots_and_values_used);
  if (found.Valid()) {
    Am_Value_List new_value_list;
    new_value_list.Add(found);
    cmd.Set(Am_VALUE, new_value_list);
    Am_Object fake_start_object = cmd.Peek(Am_START_OBJECT);
    if (!fake_start_object.Valid()) {
      fake_start_object = Am_Fake_Start_Object.Create();
      cmd.Set(Am_START_OBJECT, fake_start_object, Am_OK_IF_NOT_THERE);
    }
    set_slots_and_values(fake_start_object, slots_and_values_used);
    cmd.Set(Am_SLOTS_AND_VALUES_USED, slots_and_values_used, Am_OK_IF_NOT_THERE);
    cmd.Set(Am_CURRENT_DIRECTION, (int)dir);
    sel_widget.Set(Am_VALUE, new_value_list);
  }
  else {
    Am_Beep();
    cmd.Set(Am_COMMAND_IS_ABORTING, true);
  }
}


void internal_search(Am_Object &cmd, Am_Which_Search_Direction dir) {
  Am_Object search_widget = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  Am_Object sel_widget = search_widget.Get(Am_SELECTION_WIDGET);
  Am_Value_List slots_to_item = search_widget.Get(Am_SLOTS_TO_ITEMS);

  Am_Assoc as;
  Am_Am_Slot_Key key;
  Am_Object item;
  Am_Value v;
  Am_Value_List slots_and_values_used;
  for (slots_to_item.Start(); !slots_to_item.Last(); slots_to_item.Next()) {
    as = slots_to_item.Get();
    item = as.Value_2();
    if (item.Get_Object(Am_USE_ITEM).Get(Am_VALUE).Valid()) {
      key = as.Value_1();
      if ((Am_Slot_Key)key == Am_PROTOTYPE)
	v = item.Get_Object(Am_ITEM_VALUE).Get(Am_VALUE);
      else v = item.Get_Object(Am_ITEM_VALUE).Get(Am_VALUES);
      slots_and_values_used.Add(Am_Assoc(key, v));
    }
  }
  Am_Value_List old_sel_list = sel_widget.Get(Am_VALUE);
  internal_search_using(cmd, dir, sel_widget, old_sel_list,
			slots_and_values_used);
}

Am_Define_Method(Am_Object_Method, void, do_find_next, (Am_Object cmd)) {
  internal_search(cmd, Am_Search_Forward);
}
Am_Define_Method(Am_Object_Method, void, do_find_previous, (Am_Object cmd)) {
  internal_search(cmd, Am_Search_Back);
}
Am_Define_Method(Am_Object_Method, void, do_find_up, (Am_Object cmd)) {
  internal_search(cmd, Am_Search_Up);
}
Am_Define_Method(Am_Object_Method, void, do_find_down, (Am_Object cmd)) {
  internal_search(cmd, Am_Search_Down);
}
Am_Define_Method(Am_Object_Method, void, do_find_left, (Am_Object cmd)) {
  internal_search(cmd, Am_Search_Left);
}
Am_Define_Method(Am_Object_Method, void, do_find_right, (Am_Object cmd)) {
  internal_search(cmd, Am_Search_Right);
}
Am_Define_Method(Am_Object_Method, void, do_find_inside, (Am_Object cmd)) {
  internal_search(cmd, Am_Search_In);
}
Am_Define_Method(Am_Object_Method, void, do_find_outside, (Am_Object cmd)) {
  internal_search(cmd, Am_Search_Out);
}

Am_Define_Method(Am_Object_Method, void, undo_find, (Am_Object cmd)) {
  Am_Value_List old_val = cmd.Get(Am_OLD_VALUE);
  Am_Object sel_widget =  cmd.Get(Am_SELECTION_WIDGET);
  Am_Value_List new_val = sel_widget.Get(Am_VALUE);
  sel_widget.Set(Am_VALUE, old_val);
  cmd.Set(Am_OLD_VALUE, new_val);
  cmd.Set(Am_VALUE, old_val);
}

Am_Define_Method(Am_Object_Method, void, find_repeat_same, (Am_Object cmd)) {
  Am_Value_List cmd_val = cmd.Get(Am_VALUE);
  Am_Object sel_widget =  cmd.Get(Am_SELECTION_WIDGET);
  Am_Value_List old_val = sel_widget.Get(Am_VALUE);
  sel_widget.Set(Am_VALUE, cmd_val);
  cmd.Set(Am_OLD_VALUE, old_val);
  if (am_sdebug)
   std::cout << "Repeat Same Find, setting " << sel_widget << " to " << cmd_val
	 << " instead of " << old_val <<std::endl <<std::flush;
}

Am_Define_Method(Am_Selective_Repeat_New_Method, void, find_repeat_new,
		 (Am_Object cmd, Am_Value new_sel,
		  Am_Value new_value)){
  Am_Object sel_widget =  cmd.Get(Am_SELECTION_WIDGET);
  Am_Value_List old_sel_list;
  if (new_sel.Exists() && Am_Value_List::Test(new_sel))
    old_sel_list = new_sel;
  else old_sel_list = sel_widget.Get(Am_VALUE);
  Am_Value_List slots_and_values_used =
    get_new_slots_and_values(cmd, new_value, new_sel);
  Am_Which_Search_Direction dir =
    (Am_Which_Search_Direction)(int)cmd.Get(Am_CURRENT_DIRECTION);
  internal_search_using(cmd, dir, sel_widget, old_sel_list,
			slots_and_values_used);
}

////////////////////////////////////////////////////////////////////////


Am_Define_Formula(int, item_value_width) {
  Am_Object my_window = self.Get(Am_WINDOW);
  int y;
  int x;
  Am_Translate_Coordinates(self, 0, 0, my_window, x, y);
  return WINDOW_WIDTH - 10 - x;
}

Am_Define_Formula(bool, one_object_selected) {
  Am_Object search_widget = self.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  Am_Object sel_widget = search_widget.Get(Am_SELECTION_WIDGET);
  Am_Value_List selection = sel_widget.Get(Am_VALUE);
  return (selection.Length() == 1);
}

/* NOT USED, doesn't matter if anything is specified
Am_Define_Formula(bool, something_specified) {
  Am_Object search_widget = self.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  Am_Value_List slots_to_item = search_widget.Get(Am_SLOTS_TO_ITEMS);
  Am_Assoc as;
  Am_Object item;
  Am_Am_Slot_Key key;
  Am_Value v;
  for (slots_to_item.Start(); !slots_to_item.Last(); slots_to_item.Next()) {
    as = slots_to_item.Get();
    item = as.Value_2();
    if (item.Get_Object(Am_USE_ITEM).Get(Am_VALUE).Valid()) {
      key = as.Value_1();
      if (key == Am_PROTOTYPE) return true;
      v = item.Get_Object(Am_ITEM_VALUE).Get(Am_VALUE);
      if (v.Valid()) {
	if (v.type == Am_STRING) {
	  Am_String str = v;
	  const char * s = str;
	  if (strlen(s) != 0) return true;
	}
	else return true; //valid non strings are OK
      }
    }
  }
  return false;
}
*/

bool am_find_proto_and_name(Am_Object &target_obj, Am_Assoc& answer) {
  Am_Object this_proto;
  for (am_all_proto_names.Start(); !am_all_proto_names.Last();
       am_all_proto_names.Next()) {
    answer = am_all_proto_names.Get();
    this_proto = answer.Value_2();
    if (target_obj.Is_Instance_Of(this_proto)) {
      return true;
     }
  }
  return false;
}

void set_type_of_search(Am_Object &item, Am_Object proto) {
  Am_Assoc as;
  if (am_find_proto_and_name(proto, as)) {
    item.Get_Object(Am_ITEM_VALUE).Set(Am_VALUE, proto);
  }
  else Am_ERROR("proto " << proto << " not found in proto list "
	             << am_all_proto_names);
}


Am_Define_Method(Am_Object_Method, void, do_load_sel_object_search,
		 (Am_Object cmd)) {
  Am_Object search_widget = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  Am_Object sel_widget = search_widget.Get(Am_SELECTION_WIDGET);
  Am_Value_List slots_to_item = search_widget.Get(Am_SLOTS_TO_ITEMS);
  Am_Value_List sel_list = sel_widget.Get(Am_VALUE);
  Am_Object selected = sel_list.Get_First();
  Am_Value_List slots_to_save = selected.Get(Am_SLOTS_TO_SAVE);
  Am_Assoc as;
  Am_Am_Slot_Key key;
  Am_Object item;
  Am_Value v;
  bool set_on;
  for (slots_to_item.Start(); !slots_to_item.Last(); slots_to_item.Next()) {
    as = slots_to_item.Get();
    key = as.Value_1();
    set_on = true;
    if ((Am_Slot_Key)key == Am_PROTOTYPE)
      v = selected.Get_Prototype();
    else if (slots_to_save.Start_Member(key)) {
      v = selected.Peek(key);
      if (!v.Exists()) v = 0; //supposed to be in this object, but invalid
    }
    else {
      set_on = false;
      v = selected.Peek(key);
    }
    item = as.Value_2();
    if (v.Exists()) {
      item.Get_Object(Am_USE_ITEM).Set(Am_VALUE, set_on);
      if ((Am_Slot_Key)key == Am_PROTOTYPE) set_type_of_search(item, v);
      else {
	item.Get_Object(Am_ITEM_VALUE).Set(Am_VALUE, v);
	item.Get_Object(Am_ITEM_VALUE).Set(Am_VALUES, v);
      }
    }
    else {
      item.Get_Object(Am_USE_ITEM).Set(Am_VALUE, false);
      if ((Am_Slot_Key)key != Am_PROTOTYPE)
	item.Get_Object(Am_ITEM_VALUE).Set(Am_VALUE, "");
    }
  }
}

Am_Define_Method(Am_Object_Method, void, done_search,
		 (Am_Object cmd)) {
  Am_Object search_window = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  search_window.Set(Am_VISIBLE, false);
}

Am_Object generate_search_group(Am_Value_List &pos_slots, char * main_label,
				int & current_y, bool main_check_box,
				Am_Value_List &slots_to_item,
				Am_Value_List &slots_values) {
  Am_Object group;
  Am_Am_Slot_Key this_slot;
  const char * s;
  Am_Object this_item;
  int max_item_width = 0;
  int this_w;
  int this_y = 0;
  Am_Value_List sub_parts;
  group = Am_Group.Create()
    .Set(Am_LEFT, 5)
    .Set(Am_TOP, current_y)
    .Set(Am_WIDTH, WINDOW_WIDTH-10)
    .Add_Part(Am_Border_Rectangle.Create()
        .Set(Am_LEFT, 0)
        .Set(Am_TOP, 0)
        .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH))
        .Set(Am_HEIGHT, Am_From_Owner(Am_HEIGHT))
        .Set(Am_SELECTED, 0)
        .Set(Am_FILL_STYLE, Am_Default_Color)
    );

  if (main_check_box) {
    group.Add_Part(Am_MAIN_BUTTON, search_item_proto.Create()
	      .Set(Am_LEFT, 10)
	      .Set(Am_TOP, 5)
	      .Set(Am_LABEL, main_label)
	      .Set(Am_ACTIVE, true)
	      );
    max_item_width = group.Get_Object(Am_MAIN_BUTTON).Get_Object(Am_USE_ITEM)
      .Get(Am_WIDTH);
    group.Get_Object(Am_MAIN_BUTTON).Set(Am_X_OFFSET, max_item_width);
    group.Get_Object(Am_MAIN_BUTTON).Get_Object(Am_USE_ITEM)
      .Set(Am_FONT, search_bold_font);
    this_y += ITEM_SIZE + 5;
  }
  else {
    group.Add_Part(Am_Text.Create()
		   .Set(Am_LEFT, 2*OFFSET)
		   .Set(Am_TOP, 2*OFFSET)
		   .Set(Am_TEXT, main_label)
		   .Set(Am_FONT, search_bold_font)
		   .Set(Am_LINE_STYLE, Am_Black)
		   .Set(Am_FILL_STYLE, Am_No_Style)
		   );
    this_y += 20;
  }
  max_item_width = 0;
  Am_Value this_value;
  for (pos_slots.Start(); !pos_slots.Last(); pos_slots.Next()) {
    this_slot = Am_Am_Slot_Key(pos_slots.Get());
    s = Am_Get_Slot_Name(this_slot);
    this_item = search_item_proto.Create()
      .Set(Am_LEFT, INDENT_OFFSET)
      .Set(Am_TOP, this_y)
      .Set(Am_LABEL, s)
      ;
    this_value = slots_values.Assoc_2(this_slot);
    if (!this_value.Exists())
      Am_ERROR("Lost value for key "<<this_slot<<" from "<<slots_values);
    this_item.Get_Object(Am_ITEM_VALUE).Set(Am_VALUES, this_value);
    slots_to_item.Add(Am_Assoc(this_slot, this_item));
    sub_parts.Add(this_item);
    this_y += ITEM_SIZE;
    this_w = this_item.Get_Object(Am_USE_ITEM).Get(Am_WIDTH);
    if (this_w > max_item_width) max_item_width = this_w;
    group.Add_Part(this_item);
  }
  for (sub_parts.Start(); !sub_parts.Last(); sub_parts.Next()) {
    this_item = sub_parts.Get();
    this_item.Set(Am_X_OFFSET, max_item_width+2);
  }
  group.Set(Am_HEIGHT, this_y);
  current_y += this_y;
  return group;
}

void add_bottom_part(Am_Object & this_search_window, int& current_y) {
  this_search_window
    .Add_Part(Am_Border_Rectangle.Create()
      .Set(Am_LEFT, 5)
      .Set(Am_TOP, current_y)
      .Set(Am_WIDTH, WINDOW_WIDTH-10)
      .Set(Am_HEIGHT, 58)
      .Set(Am_SELECTED, 0)
      .Set(Am_FILL_STYLE, Am_Default_Color)
    )
    .Add_Part(Am_Text.Create()
      .Set(Am_LEFT, 10)
      .Set(Am_TOP, current_y+20)
      .Set(Am_TEXT, "Find by Location:")
      .Set(Am_FONT, search_bold_font)
      .Set(Am_LINE_STYLE, Am_Black)
      .Set(Am_FILL_STYLE, Am_No_Style)
    )
    .Add_Part(Am_Button_Panel.Create()
      .Set(Am_LEFT, 140)
      .Set(Am_TOP, current_y+5)
      .Set(Am_FILL_STYLE, Am_Default_Color)
      .Set(Am_ITEMS, Am_Value_List()
        .Add(Am_Find_Command.Create()
             .Set(Am_LABEL, "Find up")
             .Set(Am_DO_METHOD, do_find_up)
             )
        .Add(Am_Find_Command.Create()
             .Set(Am_LABEL, "Find left")
             .Set(Am_DO_METHOD, do_find_left)
             )
        .Add(Am_Find_Command.Create()
             .Set(Am_LABEL, "Find Inside")
             .Set(Am_DO_METHOD, do_find_inside)
             )
        .Add(Am_Find_Command.Create()
             .Set(Am_DO_METHOD, do_find_down)
             .Set(Am_LABEL, "Find down")
             )
        .Add(Am_Find_Command.Create()
             .Set(Am_LABEL, "Find right")
             .Set(Am_DO_METHOD, do_find_right)
             )
        .Add(Am_Find_Command.Create()
             .Set(Am_LABEL, "Find Outside")
             .Set(Am_DO_METHOD, do_find_outside)
             )
        )
      .Set(Am_LAYOUT, Am_Horizontal_Layout)
      .Set(Am_H_SPACING, 0)
      .Set(Am_V_SPACING, 0)
      .Set(Am_MAX_RANK, 3)
    );
  current_y = current_y + 58 + 5 ;
  this_search_window
    .Add_Part(Am_Border_Rectangle.Create()
      .Set(Am_LEFT, 5)
      .Set(Am_TOP, current_y)
      .Set(Am_WIDTH, WINDOW_WIDTH-10)
      .Set(Am_HEIGHT, 43)
      .Set(Am_SELECTED, 0)
      .Set(Am_FILL_STYLE, Am_Default_Color)
      )
    .Add_Part(Am_Button_Panel.Create()
      .Set(Am_LEFT, Am_Center_X_Is_Center_Of_Owner)
      .Set(Am_TOP, current_y+5)
      .Set(Am_FILL_STYLE, Am_Default_Color)
      .Set(Am_LAYOUT, Am_Horizontal_Layout)
      .Set(Am_H_SPACING, 20)
      .Set(Am_V_SPACING, 0)
      .Set(Am_MAX_RANK, 0)
      .Set(Am_ITEMS, Am_Value_List()
        .Add(Am_Find_Command.Create()
             .Set(Am_LABEL, "Find Next")
             .Set(Am_DO_METHOD, do_find_next)
	     .Set(Am_DEFAULT, true)
	     .Set(Am_ACCELERATOR, Am_Input_Char("RETURN")))
        .Add(Am_Find_Command.Create()
             .Set(Am_LABEL, "Find Previous")
             .Set(Am_DO_METHOD, do_find_previous))
        .Add(Am_Command.Create()
	     .Set(Am_IMPLEMENTATION_PARENT, Am_NOT_USUALLY_UNDONE)
	     .Set(Am_DO_METHOD, done_search)
             .Set(Am_LABEL, "Done"))
        )
     );
  current_y = current_y + 43 + 5;
}

Am_Define_String_Formula(label_from_saved_old_owner) {
  Am_Object widget = self.Get(Am_SAVED_OLD_OWNER);
  return widget.Get_Owner().Get(Am_LABEL);
}
Am_Define_Formula(bool, active_from_saved_old_owner) {
  Am_Object widget = self.Get(Am_SAVED_OLD_OWNER);
  return widget.Get_Owner().Get(Am_ACTIVE);
}

Am_Define_Method(Am_Object_Method, void, new_item_value, (Am_Object cmd)){
  Am_Object text_widget = cmd.Get(Am_SAVED_OLD_OWNER);
  Am_Object my_use = text_widget.Get_Sibling(Am_USE_ITEM);
  my_use.Set(Am_VALUE, true);
}

Am_Object Am_Create_Search_Dialog_For(/* Am_Load_Save_Context& ls_context, */
				      Am_Object &main_undo_handler) {
  Am_Object this_search_window = Am_Search_Window.Create();
  Am_Value_List all_slots;
  Am_Object proto;
  Am_Assoc as;
  Am_Value_List local_all_slots;
  Am_Value_List all_types;
  Am_Value_List slots_values;
  Am_Value v;
  for (am_all_proto_names.Start(); !am_all_proto_names.Last();
       am_all_proto_names.Next()) {
    as = am_all_proto_names.Get();
    proto = as.Value_2();
    local_all_slots = proto.Get(Am_SLOTS_TO_SAVE);
    all_slots = all_slots.Append_New(local_all_slots);
    for (local_all_slots.Start(); !local_all_slots.Last();
	 local_all_slots.Next()) {
      Am_Am_Slot_Key this_slot = local_all_slots.Get();
      if (!(slots_values.Assoc_2(this_slot).Exists())) {
	v = proto.Get(this_slot, Am_OK_IF_NOT_THERE);
	if (!v.Exists()) v = 0;
	slots_values.Add(Am_Assoc(this_slot, v));
      }
    }

    all_types.Add(Am_Command.Create()
		  .Set(Am_DO_METHOD, new_item_value)
		  .Set(Am_LABEL, as.Value_1())
		  .Set(Am_IMPLEMENTATION_PARENT, Am_NOT_USUALLY_UNDONE)
		  .Set(Am_ID, proto));
  }
  Am_Value_List pos_slots;
  Am_Value a;
  am_check_list_slot(Am_LEFT, a, Am_No_Object, all_slots, pos_slots);
  am_check_list_slot(Am_TOP, a, Am_No_Object, all_slots, pos_slots);
  am_check_list_slot(Am_WIDTH, a, Am_No_Object, all_slots, pos_slots);
  am_check_list_slot(Am_HEIGHT, a, Am_No_Object, all_slots, pos_slots);
  am_check_list_slot(Am_X1, a, Am_No_Object, all_slots, pos_slots);
  am_check_list_slot(Am_Y1, a, Am_No_Object, all_slots, pos_slots);
  am_check_list_slot(Am_X2, a, Am_No_Object, all_slots, pos_slots);
  am_check_list_slot(Am_Y2, a, Am_No_Object, all_slots, pos_slots);
  Am_Value_List color_slots;
  am_check_list_slot(Am_FILL_STYLE, a, Am_No_Object, all_slots, color_slots);
  am_check_list_slot(Am_LINE_STYLE, a, Am_No_Object, all_slots, color_slots);

  this_search_window.Get_Object(Am_TYPE_USE_ITEM).Set(Am_ITEMS, all_types);
  Am_Value_List slots_to_item;
  slots_to_item.Add(Am_Assoc(Am_Am_Slot_Key(Am_PROTOTYPE),
			     this_search_window.Get_Object(Am_TYPE_USE_ITEM)));

  Am_Object group;

  int current_y = 136;
  //  Generate the location section
  if (pos_slots.Valid()) {
    group = generate_search_group(pos_slots, "Location", current_y, false,
				  slots_to_item, slots_values);
    this_search_window.Add_Part(group);
  }
  current_y += OFFSET;

  //  Generate the color section
  if (color_slots.Valid()) {
    group = generate_search_group(color_slots, "Colors", current_y, false,
				  slots_to_item, slots_values);
    this_search_window.Add_Part(group);
  }
  current_y += OFFSET;

  //  Generate the "other" section
  if (all_slots.Valid()) {
    group = generate_search_group(all_slots, "Other", current_y, false,
				  slots_to_item, slots_values);
    this_search_window.Add_Part(group);
  }
  current_y += OFFSET;
  add_bottom_part(this_search_window, current_y);
  this_search_window.Set(Am_HEIGHT, current_y);
  this_search_window.Set(Am_SLOTS_TO_ITEMS, slots_to_item);
  this_search_window.Set(Am_UNDO_HANDLER, main_undo_handler);
  if (main_undo_handler.Valid())
    main_undo_handler.Set(Am_SEARCH_DIALOG, this_search_window,
			  Am_OK_IF_NOT_THERE);
  return this_search_window;
}

Am_Define_Formula (Am_Value, find_get_sel_widgets_impl_parent) {
  Am_Value value;
  Am_Object sel_handles = self.Get(Am_SELECTION_WIDGET);
  if (sel_handles.Valid()) {
    Am_Object sel_handles_command = sel_handles.Get_Object(Am_COMMAND);
    if (sel_handles_command.Valid())
      value = sel_handles_command.Peek(Am_IMPLEMENTATION_PARENT);
  }
  return value;
}

Am_Define_Method(Am_Object_Method, void, pop_up_search_window,
		 (Am_Object cmd)) {
  Am_Object search_window = cmd.Get(Am_SEARCH_DIALOG);
  if (!search_window.Get(Am_SELECTION_WIDGET).Valid()) {
    Am_Object sel_widget = Am_Get_Selection_Widget_For_Command(cmd);
    search_window.Set(Am_SELECTION_WIDGET, sel_widget);
  }
  search_window.Set(Am_VISIBLE, true);
  if (!search_window.Get_Owner().Valid())
    Am_Screen.Add_Part(search_window);
  Am_To_Top(search_window);
}


/////////////////////////////////////////////////////////////////////////

void Am_Initialize_Search_DB() {
  search_big_font = Am_Font(Am_FONT_FIXED, true, false, false, Am_FONT_LARGE);
  search_bold_font = Am_Font(Am_FONT_FIXED, true, false, false, Am_FONT_MEDIUM);
  search_use_item_proto = Am_Checkbox.Create(DSTR("use_item"))
       .Set(Am_LEFT, 0)
	.Set(Am_TOP, 3)
	.Get_Object(Am_COMMAND)
	  .Set(Am_ID, true)
	  .Set(Am_LABEL, label_from_saved_old_owner)
	  .Set(Am_ACTIVE, active_from_saved_old_owner)
	  .Set(Am_IMPLEMENTATION_PARENT, Am_NOT_USUALLY_UNDONE)
	  .Get_Owner()
	;
  Am_Search_Window = Am_Window.Create(DSTR("Search Window"))
    .Set(Am_DESTROY_WINDOW_METHOD, Am_Default_Pop_Up_Window_Destroy_Method)
    .Set(Am_FILL_STYLE, Am_Default_Color)
    .Set(Am_TITLE, "Search For")
    .Set(Am_WIDTH, WINDOW_WIDTH)
    .Set(Am_HEIGHT, 495)
    .Add(Am_SLOTS_TO_ITEMS, (0L))
    .Add(Am_SELECTION_WIDGET, (0L))
    .Add_Part(Am_Text.Create()
      .Set(Am_LEFT, 6)
      .Set(Am_TOP, 11)
      .Set(Am_TEXT, "Search for an Object with Values:")
      .Set(Am_FONT, search_big_font)
      .Set(Am_LINE_STYLE, Am_Black)
      .Set(Am_FILL_STYLE, Am_No_Style)
    )
    .Add_Part(Am_Button_Panel.Create()
      .Set(Am_LEFT, Am_Center_X_Is_Center_Of_Owner)
      .Set(Am_TOP, 43)
      .Set(Am_FILL_STYLE, Am_Default_Color)
      .Set(Am_ITEMS, Am_Value_List()
        .Add(Am_Command.Create()
             .Set(Am_LABEL, "Load from Selected Object")
             .Set(Am_ACTIVE, one_object_selected)
             .Set(Am_DO_METHOD, do_load_sel_object_search)
	     .Set(Am_IMPLEMENTATION_PARENT, Am_NOT_USUALLY_UNDONE)
	     )
        .Add(Am_Command.Create()
             .Set(Am_LABEL, "Load from Selected Command")
             .Set(Am_ACTIVE, /* ok_load_sel_command_search */ true)
             .Set(Am_DO_METHOD, /* do_load_sel_command_search */ (0L))
	     .Set(Am_IMPLEMENTATION_PARENT, Am_NOT_USUALLY_UNDONE)
             )
        )
      .Set(Am_LAYOUT, Am_Horizontal_Layout)
      .Set(Am_H_SPACING, 20)
      .Set(Am_V_SPACING, 0)
    )
    .Add_Part(Am_Text.Create()
      .Set(Am_LEFT, 7)
      .Set(Am_TOP, 78)
      .Set(Am_TEXT, "Check the:")
      .Set(Am_FONT, search_bold_font)
      .Set(Am_LINE_STYLE, Am_Black)
      .Set(Am_FILL_STYLE, Am_No_Style)
    )
    .Add_Part(Am_TYPE_USE_ITEM, Am_Group.Create(DSTR("TYPE_GROUP"))
      .Set(Am_LEFT, 5)
      .Set(Am_TOP, 94)
      .Set(Am_WIDTH, WINDOW_WIDTH-10)
      .Set(Am_HEIGHT, 38)
      .Add(Am_LABEL, "Type of Object")
      .Add(Am_ACTIVE, true)
      .Add(Am_ITEMS, (0L))
      .Add_Part(Am_Border_Rectangle.Create()
        .Set(Am_LEFT, 0)
        .Set(Am_TOP, 0)
        .Set(Am_WIDTH, WINDOW_WIDTH-10)
        .Set(Am_HEIGHT, Am_From_Owner(Am_HEIGHT))
        .Set(Am_SELECTED, 0)
        .Set(Am_FILL_STYLE, Am_Default_Color)
      )
      .Add_Part(Am_USE_ITEM, search_use_item_proto.Create(DSTR("Use_Type"))
        .Set(Am_LEFT, 10)
        .Set(Am_TOP, 9)
	.Set(Am_FONT, search_bold_font)
      )
      .Add_Part(Am_ITEM_VALUE, Am_Option_Button.Create(DSTR("Types"))
        .Set(Am_LEFT, 146)
        .Set(Am_TOP, 0)
        .Set(Am_FILL_STYLE, Am_Default_Color)
        .Set(Am_ITEMS, Am_From_Owner(Am_ITEMS))
	.Get_Object(Am_COMMAND)
	  .Set(Am_LABEL, "Pick Type")
	  .Set(Am_IMPLEMENTATION_PARENT, Am_NOT_USUALLY_UNDONE)
	  .Set(Am_DO_METHOD, new_item_value)
	  .Get_Owner()

      )
    ) //end type_group
    ;


  search_item_proto = Am_Group.Create(DSTR("search_item_proto"))
     .Add(Am_LABEL, "Left")
     .Add(Am_ACTIVE, true)
     .Add(Am_X_OFFSET, 68)
     .Set(Am_LEFT, 0)
     .Set(Am_TOP, 0)
     .Set(Am_WIDTH, WINDOW_WIDTH-10)
     .Set(Am_HEIGHT, Am_Height_Of_Parts)
     .Add_Part(Am_USE_ITEM, search_use_item_proto.Create())
     .Add_Part(Am_ITEM_VALUE, Am_Text_Input_Widget.Create(DSTR("item_value"))
	       .Set(Am_LEFT, Am_From_Owner(Am_X_OFFSET))
	       .Set(Am_TOP, 0)
	       .Add(Am_VALUES, Am_No_Value) // the real value calculated
	       // am_check_valid_type defined in valgen.cc
	       .Set(Am_TEXT_CHECK_LEGAL_METHOD, am_check_valid_type)
	       .Set(Am_WIDTH, item_value_width)

	       .Get_Object(Am_COMMAND)
	         .Set(Am_LABEL, "")
	         .Set(Am_IMPLEMENTATION_PARENT, Am_NOT_USUALLY_UNDONE)
	         .Set(Am_DO_METHOD, new_item_value)
	         .Get_Owner()
	       )
     ;
  Am_Find_Command = Am_Select_Command.Create(DSTR("Find_Command"))
    .Add (Am_SELECTION_WIDGET, (0L)) //set this to associated sel..han..widget
    .Add (Am_CURRENT_DIRECTION, (0L))
    .Set(Am_LABEL, "Find")
    .Set(Am_DO_METHOD, (0L)) //set in the instances
    .Set(Am_UNDO_METHOD, undo_find)
    .Set(Am_REDO_METHOD, undo_find)
    .Set(Am_SELECTIVE_UNDO_METHOD, undo_find)
    .Set(Am_SELECTIVE_REPEAT_SAME_METHOD, find_repeat_same)
    .Set(Am_SELECTIVE_REPEAT_ON_NEW_METHOD, find_repeat_new)
    .Set(Am_SELECTIVE_REPEAT_NEW_ALLOWED, Am_Selective_New_Allowed_Return_True)
    .Set(Am_IMPLEMENTATION_PARENT, find_get_sel_widgets_impl_parent)
    ;
  Am_Show_Search_Command = Am_Command.Create(DSTR("Show Search Window"))
    .Set(Am_LABEL, "Find...")
    .Set(Am_DO_METHOD, pop_up_search_window)
    .Set(Am_IMPLEMENTATION_PARENT, Am_NOT_USUALLY_UNDONE)
    .Add(Am_SELECTION_WIDGET, (0L)) //must be set
    .Add(Am_SEARCH_DIALOG, (0L)) //must be set
    ;
  Am_Fake_Start_Object = Am_Root_Object.Create(DSTR("Fake_Start_Object"));
}

//smaller number than undo which is 100
Am_Initializer* search_db_init =
new Am_Initializer(DSTR("Search_Dialogbox"), Am_Initialize_Search_DB, 99.0);
