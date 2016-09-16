/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code w written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <amulet.h>
#include <amulet/undo_dialog.h>
#include <amulet/scripting.h>
#include <amulet/inter_advanced.h> // Am_Valid_and_Visible_List_Or_Object

using namespace std;

#define ARROW_BITMAP "data/images/arrow.gif"
#define ARROW01_BITMAP "samples/circuit/arrow01.gif"
#define NOT_BITMAP "samples/circuit/not.gif"
#define AND_BITMAP "samples/circuit/and.gif"
#define OR_BITMAP "samples/circuit/or.gif"
#define XOR_BITMAP "samples/circuit/xor.gif"
#define ONE_BITMAP "samples/circuit/one.gif"
#define ZERO_BITMAP "samples/circuit/zero.gif"
#define QUESTION_BITMAP "samples/circuit/question.gif"
#define CLASSIFIER_FILENAME "samples/circuit/circuit.cl"
#define ICON_TOP_OFFSET 6

Am_Slot_Key INPUT_1 = Am_Register_Slot_Name ("INPUT_1");
Am_Slot_Key INPUT_2 = Am_Register_Slot_Name ("INPUT_2");
Am_Slot_Key OUTPUT_1 = Am_Register_Slot_Name ("OUTPUT_1");
Am_Slot_Key INPUT_1_PORT = Am_Register_Slot_Name ("INPUT_1_PORT");
Am_Slot_Key INPUT_2_PORT = Am_Register_Slot_Name ("INPUT_2_PORT");
Am_Slot_Key OUTPUT_1_PORT = Am_Register_Slot_Name ("OUTPUT_1_PORT");
Am_Slot_Key OUTPUT_VALUE = Am_Register_Slot_Name ("OUTPUT_VALUE");
Am_Slot_Key PICTURE = Am_Register_Slot_Name ("PICTURE");
Am_Slot_Key INPUT_PLACE = Am_Register_Slot_Name ("INPUT_PLACE");
Am_Slot_Key OUTPUT_PLACE = Am_Register_Slot_Name ("OUTPUT_PLACE");
Am_Slot_Key OUTPUT_WAS = Am_Register_Slot_Name ("OUTPUT_WAS");
Am_Slot_Key INPUT_WAS = Am_Register_Slot_Name ("INPUT_WAS");
Am_Slot_Key IN_VALUE = Am_Register_Slot_Name ("IN_VALUE");
Am_Slot_Key NEW_OBJECT_PROTO = Am_Register_Slot_Name ("NEW_OBJECT_PROTO");

enum Places { center_left, top_left, bottom_left, center_right };

Am_Object circuit_win, scroller, menu_bar, tool_panel, my_selection,
  created_objs, zero_one_proto, and_or_proto, in_port_proto,
  out_port_proto, circuit_object_proto, arrow_proto, arrow_run_proto,
  not_proto, and_proto, or_proto, xor_proto, line_proto, animation_proto,
  animations_command;
Am_Image_Array one_image, zero_image, question_image;

void set_up_anim(Am_Object &line, int val) {
  if (!line.Get_Owner().Valid() || !(bool)line.Get(Am_VISIBLE)) return;
  Am_Object anim = animation_proto.Create();
  Am_String s;
  if (val == 0) s = "0";
  else if (val == 1) s = "1";
  else s = "?";
  anim.Set(Am_LEFT, (int)line.Get(Am_X1), Am_NO_ANIMATION);
  anim.Set(Am_TOP, (int)line.Get(Am_Y1)-10, Am_NO_ANIMATION);
  anim.Set(Am_TEXT, s);
  anim.Add(INPUT_1, line);
  scroller.Add_Part(anim); //add to scroller so not selectable

  int new_left = (int)line.Get(Am_X2);
  int new_top = (int)line.Get(Am_Y2)-10;
  anim.Set(Am_LEFT, new_left);
  anim.Set(Am_TOP, new_top);
}
void clean_up_anim(Am_Object &cmd, bool setit) {
  Am_Object interp = cmd.Get(Am_SAVED_OLD_OWNER);
  Am_Object text = interp.Get(Am_OPERATES_ON);
  Am_Object line = text.Get(INPUT_1);
  if (setit) {
    Am_Value v;
    v = line.Peek(IN_VALUE);
    line.Set(Am_VALUE, v);
  }
  text.Destroy();
}
Am_Define_Method(Am_Object_Method, void, anim_set_value, (Am_Object cmd)) {
  clean_up_anim(cmd, true);
}
Am_Define_Method(Am_Object_Method, void, anim_abort_value, (Am_Object cmd)) {
  clean_up_anim(cmd, false);
}
Am_Define_String_Formula(compute_val_text) {
  int val = self.Get_Owner().Get(Am_VALUE);
  Am_String s;
  if (val == 0) s = "0";
  else if (val == 1) s = "1";
  else s = "?";
  return s;
}
Am_Define_Formula(int, output_val_left) {
  return (int)self.Get_Sibling(OUTPUT_1_PORT).Get(Am_LEFT);
}
Am_Define_Formula(int, output_val_top) {
  return (int)self.Get_Sibling(OUTPUT_1_PORT).Get(Am_TOP) -
    (int)self.Get(Am_HEIGHT)-1;
}
int get_value_end_of(Am_Object &obj,
		      Am_Slot_Key slot) {
  Am_Value v;
  int val = -1;
  v = obj.Peek(slot);
  if (v.Valid()) {
    Am_Object other_obj = v;
    val = other_obj.Get(Am_VALUE);
  }
  return val;
}
Am_Define_Formula(int, value_from_picture_or_in) {
  Am_Value v;
  v = self.Peek(INPUT_1); //check if in wire; if so, return its value
  if (v.Valid()) return get_value_end_of(self, INPUT_1);
  else {
    v = self.Get_Object(PICTURE).Peek(Am_VALUE);
    return (v.Valid());
  }
}
Am_Define_Formula(int, out_value_anim) {
  int old_val = self.Get(Am_VALUE); //no dependency
  int val = get_value_end_of(self, INPUT_1);
  if (old_val != val) {
    if ((bool) animations_command.Get(Am_VALUE)) //no dependency
      set_up_anim(self, val);
    else self.Set(Am_VALUE, val);  //not animating, use as out value now
  }
  return val;
}
Am_Define_Formula(int, not_value) {
  int v1 = get_value_end_of(self, INPUT_1);
  if (v1 == 0) return 1;
  else if (v1 == 1) return 0;
  else return -1;
}
Am_Define_Formula(int, and_value) {
  int v1 = get_value_end_of(self, INPUT_1);
  int v2 = get_value_end_of(self, INPUT_2);
  if (v1 >=0 && v2 >= 0)
    return v1 & v2;
  else return -1;
}
Am_Define_Formula(int, or_value) {
  int v1 = get_value_end_of(self, INPUT_1);
  int v2 = get_value_end_of(self, INPUT_2);
  if (v1 >=0 && v2 >= 0)
    return v1 | v2;
  else return -1;
}
Am_Define_Formula(int, xor_value) {
  int v1 = get_value_end_of(self, INPUT_1);
  int v2 = get_value_end_of(self, INPUT_2);
  if (v1 >=0 && v2 >= 0)
    return v1 ^ v2; //bitwise xor operator
  else return -1;
}
Am_Define_Object_Formula(one_zero_formula) {
  Am_Value v;
  v = self.Get_Owner().Get_Owner().Peek(Am_VALUE);
  if (v == 1) return one_image;
  else if (v == 0) return zero_image;
  else return question_image;
}
Am_Define_Formula(int, picture_right) {
  Am_Object picture = self.Get_Sibling(PICTURE);
  return (int)picture.Get(Am_LEFT)+(int)picture.Get(Am_WIDTH);
}
Am_Define_Formula(int, picture_center_y) {
  Am_Object picture = self.Get_Sibling(PICTURE);
  return (int)picture.Get(Am_TOP)+(int)picture.Get(Am_HEIGHT)/2;
}
Am_Define_Formula(int, picture_bottom_minus_offset) {
  Am_Object picture = self.Get_Sibling(PICTURE);
  return (int)picture.Get(Am_TOP)+(int)picture.Get(Am_HEIGHT)-ICON_TOP_OFFSET-2;
}

//put in x1 sets y1
Am_Define_Formula(int, line_x1y1) {
  Am_Object source_obj = self.Get(INPUT_1);
  Places where = (Places)(int)self.Get(INPUT_PLACE);
  if (where == center_right) {
    int x1 = (int)source_obj.Get(Am_WIDTH) + (int)source_obj.Get(Am_LEFT)-2;
    int y1 = (int)source_obj.Get(Am_HEIGHT) / 2 + (int)source_obj.Get(Am_TOP);
    self.Set(Am_Y1, y1);
    return x1;
  }
  return 0;
}

//put in x2 sets y2
Am_Define_Formula(int, line_x2y2) {
  Am_Object source_obj = self.Get(OUTPUT_1);
  Places where = (Places)(int)self.Get(OUTPUT_PLACE);
  switch (where) {
  case center_left: {
    int x2 = source_obj.Get(Am_LEFT);
    int y2 = (int)source_obj.Get(Am_HEIGHT) / 2 + (int)source_obj.Get(Am_TOP);
    self.Set(Am_Y2, y2);
    return x2;
  }
  case top_left: {
    int x2 = source_obj.Get(Am_LEFT);
    int y2 = (int)source_obj.Get(Am_TOP) + ICON_TOP_OFFSET;
    self.Set(Am_Y2, y2);
    return x2;
  }
  case bottom_left: {
    int x2 = source_obj.Get(Am_LEFT);
    int y2 = (int)source_obj.Get(Am_TOP) + (int)source_obj.Get(Am_HEIGHT) -
      ICON_TOP_OFFSET-1;
    self.Set(Am_Y2, y2);
    return x2;
  }
  case center_right: break;
  }
  return 0;
}

Am_Define_Formula(int, scroll_height) {
  return (int)self.Get_Owner().Get(Am_HEIGHT) - 5 - (int)self.Get(Am_TOP);
}
Am_Define_Formula(int, scroll_width) {
  return (int)self.Get_Owner().Get(Am_WIDTH) - 5 - (int)self.Get(Am_LEFT);
}
Am_Define_No_Self_Formula(int, minw_from_tool) {
  Am_Object tool = tool_panel.Get(Am_VALUE);
  return tool.Get(Am_WIDTH);
}
Am_Define_No_Self_Formula(int, minh_from_tool) {
  Am_Object tool = tool_panel.Get(Am_VALUE);
  return tool.Get(Am_HEIGHT);
}
Am_Define_No_Self_Formula (bool, selection_tool_is_selected) {
  Am_Object tool = tool_panel.Get(Am_VALUE);
  return (tool == arrow_proto);
}
Am_Define_No_Self_Formula (bool, line_tool_is_selected) {
  Am_Object tool = tool_panel.Get(Am_VALUE);
  return (tool == line_proto);
}
Am_Define_No_Self_Formula (bool, bm_tool_is_selected) {
  Am_Object tool = tool_panel.Get(Am_VALUE);
  return (tool.Is_Instance_Of(circuit_object_proto));
}
Am_Define_Formula (bool, run_tool_is_selected_and_no_input) {
  Am_Value v;
  v = tool_panel.Peek(Am_VALUE);
  return (v.Valid() && (Am_Object)v == arrow_run_proto &&
	  !self.Get_Owner().Peek(INPUT_1).Valid());
}
bool is_at_top(int y, Am_Object &ref_obj, Am_Object &obj) {
  int x = 0;
  Am_Translate_Coordinates(ref_obj, x, y, obj, x, y);
  return (y < (int)obj.Get(Am_HEIGHT)/2);
}
Am_Object find_obj_slot(int x, int y, Am_Object &ref_obj, bool source,
			Am_Slot_Key &slot, Places &place) {
  Am_Object obj = Am_Point_In_Part(created_objs, x, y, ref_obj);
  Am_Value v;
  if (obj.Valid() && obj.Is_Instance_Of(circuit_object_proto)) {
    if (source) {
      slot = OUTPUT_1;
      place = center_right;
      return obj;
    }
    else { // is destination
      if (obj.Is_Instance_Of(zero_one_proto) ||
	  obj.Is_Instance_Of(not_proto)) {
	slot = INPUT_1;
	place = center_left;
      }
      else {
	if (is_at_top(y, ref_obj, obj)) {
	  slot = INPUT_1;
	  place = top_left;
	}
	else {
	  slot = INPUT_2;
	  place = bottom_left;
	}
      }
      // destinations can only have one per place
      v = obj.Peek(slot);
      if (v.Valid()) 
	cout << "** Object at output end " << obj
	     << " already has another object as input= " << obj
	     << endl << flush;
      else return obj;
    }
  }
  else cout << "** Wrong type obj for wire: " << obj << endl << flush;
  //if get here, then failed
  Am_Beep(circuit_win);
  return Am_No_Object;
}
  
void line_for_list(Am_Object &o2, Am_Object& line, bool remove_it) {
  Am_Value v;
  Am_Value_List outvals;
  v = o2.Peek(OUTPUT_1);
  if (v.Valid()) outvals = v;
  outvals.Start();
  bool mem = outvals.Member(line);
  if (remove_it) {
    if (mem) outvals.Delete();
  }
  else if (!mem)
    outvals.Add(line, Am_TAIL);
  o2.Set(OUTPUT_1, outvals, Am_OK_IF_NOT_THERE);
}

// Am_Create_New_Object_Proc for new object command
Am_Define_Method(Am_Create_New_Object_Method, Am_Object, create_new_line,
                 (Am_Object inter, Am_Inter_Location data,
                  Am_Object old_object)) {
  Am_Object new_obj;

  if (old_object.Valid()) {
    new_obj = old_object.Copy();
  }
  else {
    Am_Object ref_obj;
    int x1,y1,x2,y2;
    bool create_line;
    data.Get_Location(create_line, ref_obj, x1,y1, x2,y2);
    Places placein, placeout;
    Am_Slot_Key slotin, slotout;
    Am_Object source_obj = find_obj_slot(x1, y1, ref_obj, true, slotin,
					 placein);
    if (source_obj.Valid()) {
      Am_Object dest_obj = find_obj_slot(x2, y2, ref_obj, false, slotout,
					 placeout);
      if (dest_obj.Valid()) {
	new_obj = line_proto.Create ();
	created_objs.Add_Part (new_obj); //add first so anim will trigger
	new_obj.Set (INPUT_1, source_obj, Am_OK_IF_NOT_THERE)
	  .Set (INPUT_PLACE, (int)placein)
	  .Set (OUTPUT_1, dest_obj, Am_OK_IF_NOT_THERE)
	  .Set (OUTPUT_PLACE, (int)placeout)
	  ;
	line_for_list(source_obj, new_obj, false);
	dest_obj.Set(slotout, new_obj, Am_OK_IF_NOT_THERE);
      }
    }
  }
  if (!new_obj.Valid()) //keep inter from being queued for undo
    Am_Abort_Interactor(inter);
  return new_obj;
}

void clean_other_for_line(Am_Object &o2, Am_Object &line, Am_Slot_Key slot) {
  Am_Value v;
  v = o2.Peek(INPUT_1);
  if (v == line) {
    o2.Set(INPUT_1, 0);
    line.Set(slot, INPUT_1, Am_OK_IF_NOT_THERE);
  }
  else {
    v = o2.Peek(INPUT_2);
    if (v == line) {
      o2.Set(INPUT_2, 0);
      line.Set(slot, INPUT_2, Am_OK_IF_NOT_THERE);
    }
    else {
      line_for_list(o2, line, true);
      line.Set(slot, OUTPUT_1, Am_OK_IF_NOT_THERE);
    }
  }
}
void delete_line(Am_Object &line, Am_Object& obj) {
  if (line.Valid()) {
    // line.Set(Am_VISIBLE, false);
    line.Set(Am_SAVED_OLD_OWNER, line.Get_Owner(), Am_OK_IF_NOT_THERE);
    line.Remove_From_Owner();

    Am_Object o2 = line.Get(INPUT_1);
    Am_Slot_Key slot = INPUT_WAS;
    if (o2 == obj) {
      o2 = line.Get(OUTPUT_1);
      slot = OUTPUT_WAS;
    }
    clean_other_for_line(o2, line, slot);
  }
}

Am_Define_Method(Am_Object_Method, void, create_line_undo,
		 (Am_Object cmd)) {
  Am_Object line = cmd.Get(Am_OBJECT_MODIFIED);
  Am_Value has_been_undone;
  has_been_undone = cmd.Peek(Am_HAS_BEEN_UNDONE);
  cmd.Set(Am_HAS_BEEN_UNDONE, !has_been_undone.Valid());
  if (line.Valid()) {
    // cout << " for line " << line << " cmd " << cmd
    //  << " has_been_undone " << has_been_undone << endl << flush;
    Am_Object o2;
    if (has_been_undone.Valid()) { //becoming visible
      o2 = line.Get(OUTPUT_1);
      int where_in_o2 = (int)line.Get(OUTPUT_WAS);
      o2.Set(where_in_o2, line);
      o2 = line.Get(INPUT_1);
      line_for_list(o2, line, false);
    }
    else { //make it invisible
      o2 = line.Get(INPUT_1);
      clean_other_for_line(o2, line, INPUT_WAS);
      o2 = line.Get(OUTPUT_1);
      clean_other_for_line(o2, line, OUTPUT_WAS);
    }
  }
}

bool internal_selective_repeat_create_line_allowed(Am_Object& /*command_obj*/,
						   Am_Object &orig_line) {
  int output_place = orig_line.Get(OUTPUT_PLACE);
  Am_Object output_1 = orig_line.Get(OUTPUT_1);
  // cout << "sel_undo_allowed test, cmd " << command_obj
  // << " orig_line " << orig_line << " output_1 " << output_1
  // << " output_place " << output_place << endl << flush;
  Am_Value v;
  v = output_1;
  if (!Am_Valid_and_Visible_List_Or_Object(v)) {
    // cout << "  return false because old output not there" << endl << flush;
    return false;
  }
  Am_Am_Slot_Key slot;
  if (output_place == top_left || output_place == center_left)
    slot = INPUT_1;
  else if (output_place == bottom_left) slot = INPUT_2;
  else Am_ERROR("Bad slot place " << output_place);
  Am_Object cur_in = output_1.Peek(slot);
  if (cur_in.Valid()) {
    // cout << "  return false because there is another input " << cur_in
    // << endl << flush;
    return false;
  }
  // cout << "  return true\n" << flush;
  return true;
}

Am_Define_Method(Am_Selective_Allowed_Method, bool,
		 selective_repeat_same_create_line_allowed,
		 (Am_Object command_obj)) {
  //can repeat create of this line if the orig input port is now empty
  Am_Object orig_line = command_obj.Get(Am_START_OBJECT);
  return internal_selective_repeat_create_line_allowed(command_obj, orig_line);
}

Am_Define_Method(Am_Selective_New_Allowed_Method, bool,
		 selective_repeat_new_create_line_allowed,
		 (Am_Object command_obj, Am_Value new_selection,
		  Am_Value /* new_value */)) {
  if (new_selection.Valid() && new_selection.type == Am_OBJECT) {
    Am_Object orig_line = new_selection;
    return internal_selective_repeat_create_line_allowed(command_obj,
							 orig_line);
  }
  return false;
}

void internal_selective_repeat_create_line(Am_Object & command_obj,
					   Am_Object & new_start_obj) {
  Am_Object orig_line;
  if (new_start_obj.Valid()) orig_line = new_start_obj;
  else orig_line = command_obj.Get(Am_START_OBJECT);
  int output_place = orig_line.Get(OUTPUT_PLACE);
  Am_Object dest_obj = orig_line.Get(OUTPUT_1);
  Am_Object source_obj = orig_line.Get(INPUT_1);
  //  cout << "selective_repeat_same_create_line, cmd " << command_obj
  // << " orig_line " << orig_line << " dest_obj " << dest_obj
  // << " output_place " << output_place << " source_obj " << source_obj
  //       << endl << flush;
  Am_Am_Slot_Key slot;
  if (output_place == top_left || output_place == center_left)
    slot = INPUT_1;
  else if (output_place == bottom_left) slot = INPUT_2;
  else Am_ERROR("Bad slot place " << output_place);


  Am_Object new_obj = line_proto.Create ();
  created_objs.Add_Part (new_obj); //add first so anim will trigger
  new_obj.Set (INPUT_1, source_obj, Am_OK_IF_NOT_THERE)
    .Set (INPUT_PLACE, (int)center_right)
    .Set (OUTPUT_1, dest_obj, Am_OK_IF_NOT_THERE)
    .Set (OUTPUT_PLACE, (int)output_place)
    ;
  line_for_list(source_obj, new_obj, false);
  dest_obj.Set(slot, new_obj, Am_OK_IF_NOT_THERE);
  // cout << "  Created new line " << new_obj << endl << flush;
  command_obj.Set(Am_VALUE, new_obj);
  command_obj.Set(Am_OBJECT_MODIFIED, new_obj);
  command_obj.Set(Am_START_OBJECT, new_obj.Copy());
}

Am_Define_Method(Am_Object_Method, void, selective_repeat_same_create_line,
		 (Am_Object command_obj)) {
  internal_selective_repeat_create_line(command_obj, Am_No_Object);
}

Am_Define_Method(Am_Selective_Repeat_New_Method, void,
		 selective_repeat_new_create_line,
		 (Am_Object command_obj, Am_Value new_selection,
		  Am_Value /* new_value */)) {
  Am_Object new_object = new_selection;
  internal_selective_repeat_create_line(command_obj, new_object);
}  


void undo_delete_line(Am_Object &line, Am_Object& obj, bool vis) {
  if (line.Valid()) {
    // line.Set(Am_VISIBLE, vis);
    if (vis) {
      Am_Object owner = line.Get(Am_SAVED_OLD_OWNER);
      owner.Add_Part(line);
    }
    else {
      line.Set(Am_SAVED_OLD_OWNER, line.Get_Owner(), Am_OK_IF_NOT_THERE);
      line.Remove_From_Owner();
    }
    Am_Object o2 = line.Get(INPUT_1);
    Am_Slot_Key slot = INPUT_WAS;
    if (o2 == obj) {
      o2 = line.Get(OUTPUT_1);
      slot = OUTPUT_WAS;
    }
    Am_Slot_Key where_in_o2 = (int)line.Get(slot);
    if (where_in_o2 == OUTPUT_1) line_for_list(o2, line, !vis);
    else {
      if (vis) o2.Set(where_in_o2, line);
      else o2.Set(where_in_o2, 0);
    }
  }
}
//make all lines attached to obj's OUTPUT_1 slot vis or invisible
void handle_OUTPUT_line_list(Am_Object &obj, bool vis, bool first_time) {
  Am_Value_List l;
  Am_Value v;
  v = obj.Peek(OUTPUT_1);
  if (v.Valid()) {
    l = v;
    Am_Object line;
    for (l.Start(); !l.Last(); l.Next()) {
      line = l.Get();
      if (first_time) delete_line(line, obj);
      else undo_delete_line(line, obj, vis);
    }
  }
}

void internal_delete_lines_also(Am_Value_List &deleting) {
  Am_Object obj, line, o2;
  for (deleting.Start(); !deleting.Last(); deleting.Next()) {
    obj = deleting.Get();
    if (obj.Is_Instance_Of(line_proto)) {
      o2 = obj.Get(INPUT_1, Am_OK_IF_NOT_THERE);
      clean_other_for_line(o2, obj, INPUT_WAS);
      o2 = obj.Get(OUTPUT_1, Am_OK_IF_NOT_THERE);
      clean_other_for_line(o2, obj, OUTPUT_WAS);
    }
    else {
      line = obj.Get(INPUT_1, Am_OK_IF_NOT_THERE);
      delete_line(line, obj);
      line = obj.Get(INPUT_2, Am_OK_IF_NOT_THERE);
      delete_line(line, obj);
      handle_OUTPUT_line_list(obj, false, true);
    }
  }
}
Am_Define_Method(Am_Object_Method, void, undo_delete_lines_also,
		 (Am_Object cmd)) {
  Am_Object child = cmd.Get(Am_IMPLEMENTATION_CHILD);
  Am_Value_List deleting = child.Get(Am_OBJECT_MODIFIED);
  Am_Object obj, line, owner, o2;
  bool vis;
  Am_Slot_Key where_in_o2;
  for (deleting.Start(); !deleting.Last(); deleting.Next()) {
    obj = deleting.Get();
    owner = obj.Get_Owner();
    vis = owner.Valid();
    if (obj.Is_Instance_Of(line_proto)) {
      line = obj;
      o2 = line.Get(OUTPUT_1);
      where_in_o2 = (int)line.Get(OUTPUT_WAS);
      if (vis) o2.Set(where_in_o2, line);
      else o2.Set(where_in_o2, 0);

      o2 = line.Get(INPUT_1);
      where_in_o2 = (int)line.Get(INPUT_WAS);
      line_for_list(o2, line, !vis);
    }
    else {
      line = obj.Get(INPUT_1, Am_OK_IF_NOT_THERE);
      undo_delete_line(line, obj, vis);
      line = obj.Get(INPUT_2, Am_OK_IF_NOT_THERE);
      undo_delete_line(line, obj, vis);
      handle_OUTPUT_line_list(obj, vis, false);
    }
  }
}

Am_Define_Method(Am_Object_Method, void, do_delete_lines_also,
		 (Am_Object command_obj)) {
  Am_Object child = command_obj.Get(Am_IMPLEMENTATION_CHILD);
  Am_Value_List deleting = child.Get(Am_OBJECT_MODIFIED);
  internal_delete_lines_also(deleting);
}

Am_Define_Method(Am_Selective_Repeat_New_Method, void,
		 selective_repeat_new_del_also,
		 (Am_Object /* command_obj */, Am_Value new_selection,
		  Am_Value /* new_value */)) {
  Am_Value_List deleting = new_selection;
  internal_delete_lines_also(deleting);
}  


Am_Object create_new_obj_internal(Am_Inter_Location &data, Am_Object& proto) {
  Am_Object ref_obj;
  int l,t,w,h;
  bool create_line;
  data.Get_Location(create_line, ref_obj, l, t, w, h);
  Am_Translate_Coordinates(ref_obj, l,t, created_objs, l,t);
  Am_Object new_obj = proto.Create ()
    .Set (Am_LEFT, l)
    .Set (Am_TOP, t)
    //**BUG, localness is not inherited correctly
    .Add (INPUT_1, 0)
    .Set_Inherit_Rule(INPUT_1, Am_LOCAL)
    .Add (INPUT_2, 0)
    .Set_Inherit_Rule(INPUT_2, Am_LOCAL)
    .Add (OUTPUT_1, 0)
    .Set_Inherit_Rule(OUTPUT_1, Am_LOCAL);
  created_objs.Add_Part (new_obj);
  return new_obj;
}

// Am_Create_New_Object_Proc for new object command
Am_Define_Method(Am_Create_New_Object_Method, Am_Object, create_new_proto,
                 (Am_Object /* inter */, Am_Inter_Location data,
                  Am_Object old_object)) {
  Am_Object new_obj;
  if (old_object.Valid()) {
    new_obj = old_object.Copy();
    created_objs.Add_Part (new_obj);
  }
  else {
    Am_Object proto = tool_panel.Get(Am_VALUE);
    new_obj = create_new_obj_internal(data, proto);
  }
  return new_obj;
}

Am_Define_Method(Am_Object_Method, void, clear_selection, (Am_Object /*cmd*/)){
  my_selection.Set(Am_VALUE, NULL);
}

Am_Define_Method(Am_Create_New_Object_Method, Am_Object, gesture_creator,
                 (Am_Object cmd, Am_Inter_Location data,
                  Am_Object /* old_object */)) {
  Am_Object prototype = cmd.Get (NEW_OBJECT_PROTO);
  if (prototype == line_proto) {
    Am_Object inter = cmd.Get(Am_SAVED_OLD_OWNER);
    return create_new_line_proc(inter, data, Am_No_Object);
  }
  else return create_new_obj_internal(data, prototype);
}

///////////////////////// Load and save ////////////////////////////

Am_Define_Method (Am_Handle_Loaded_Items_Method, void, read_file_contents,
		  (Am_Object /* command */, Am_Value_List &contents)) {
  Am_Value_List current = created_objs.Get (Am_GRAPHICAL_PARTS);
  Am_Object item;
  for (current.Start (); !current.Last (); current.Next ()) {
    item = current.Get ();
    item.Remove_From_Owner ();
  }
  for (contents.Start (); !contents.Last (); contents.Next ()) {
    item = contents.Get ();
    created_objs.Add_Part (item);
  }
}

Am_Define_Method (Am_Items_To_Save_Method, Am_Value_List, contents_for_save,
		  (Am_Object /* command */)) {
  Am_Value_List saved_stuff = created_objs.Get (Am_GRAPHICAL_PARTS);
  return saved_stuff;
}

Am_Define_No_Self_Formula(int, right_of_tool_panel) {
  return (int)tool_panel.Get(Am_LEFT) + (int)tool_panel.Get(Am_WIDTH) + 5;
}

Am_Define_Object_Formula(same_as_impl_child) {
  return self.Get_Object(Am_IMPLEMENTATION_CHILD).Get(Am_OBJECT_MODIFIED);
}
  
int main (int argc, char *argv[]) {
  Am_Initialize ();
  
  Am_Object undo_handler = Am_Multiple_Undo_Object.Create();
  circuit_win = Am_Window.Create ("circuit_win")
    .Set (Am_TOP, 50)
    .Set (Am_LEFT, 50)
    .Set (Am_WIDTH, 500)
    .Set (Am_HEIGHT, 480)
    .Set (Am_TITLE, "Amulet Sample Circuit Program")
    .Set (Am_UNDO_HANDLER, undo_handler)
    .Set (Am_FILL_STYLE, Am_Amulet_Purple)
    ;

  animations_command = Am_Cycle_Value_Command.Create("animations_enabled")
    .Set(Am_LABEL_LIST, Am_Value_List()
	 .Add("Turn Animations On")
	 .Add("Turn Animations Off"));

  const char* pathname = Am_Merge_Pathname(ARROW_BITMAP);
  Am_Image_Array image = Am_Image_Array(pathname);
  delete [] pathname;
  arrow_proto = Am_Bitmap.Create("Arrow bitmap")
    .Set (Am_IMAGE, image);
  pathname = Am_Merge_Pathname(ARROW01_BITMAP);
  image = Am_Image_Array(pathname);
  delete [] pathname;
  arrow_run_proto = Am_Bitmap.Create("Arrow Run bitmap")
    .Set (Am_IMAGE, image);

  pathname = Am_Merge_Pathname(ONE_BITMAP);
  one_image = Am_Image_Array(pathname);
  delete [] pathname;
  pathname = Am_Merge_Pathname(ZERO_BITMAP);
  zero_image = Am_Image_Array(pathname);
  delete [] pathname;
  pathname = Am_Merge_Pathname(QUESTION_BITMAP);
  question_image = Am_Image_Array(pathname);
  delete [] pathname;

  in_port_proto = Am_Rectangle.Create("in_port")
    .Set (Am_HEIGHT, 2)
    .Set (Am_WIDTH, 3)
    .Set (Am_LINE_STYLE, NULL);
  out_port_proto = Am_Rectangle.Create("out_port")
    .Set (Am_HEIGHT, 2)
    .Set (Am_WIDTH, 3)
    .Set (Am_LINE_STYLE, NULL);

  Am_Font small_font(Am_FONT_SERIF, false, false, false, Am_FONT_SMALL);
  circuit_object_proto = Am_Group.Create("circuit_object_proto")
    .Set(Am_WIDTH, Am_Width_Of_Parts)
    .Set(Am_HEIGHT, Am_Height_Of_Parts)
    .Add(Am_VALUE, -1)
    .Add_Part(OUTPUT_1_PORT, out_port_proto.Create()
	      .Set(Am_LEFT, picture_right)
	      .Set(Am_TOP, picture_center_y))
    .Add_Part(INPUT_1_PORT, in_port_proto.Create()
	      .Set(Am_TOP, picture_center_y))
    .Add (INPUT_1, 0)
    .Set_Inherit_Rule(INPUT_1, Am_LOCAL)
    .Add (INPUT_2, 0)
    .Set_Inherit_Rule(INPUT_2, Am_LOCAL)
    .Add (OUTPUT_1, 0)
    .Set_Inherit_Rule(OUTPUT_1, Am_LOCAL)
    .Add_Part(Am_INACTIVE_COMMANDS, Am_Command.Create("Line_MG_Inactive")
	      .Add(Am_GROW_INACTIVE, true));
  zero_one_proto = circuit_object_proto.Create("zero_one")
    .Set(Am_VALUE, value_from_picture_or_in.Multi_Constraint())
    .Add(INPUT_2, Am_No_Value)
    .Add (Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
    .Add (Am_SLOTS_TO_SAVE, Am_Value_List()
	  .Add(Am_LEFT).Add(Am_TOP).Add(INPUT_1).Add(OUTPUT_1).Add(Am_VALUE))
    .Add_Part(PICTURE, Am_Button.Create("zero_one_button")
	      .Set (Am_LEFT, 3)
	      .Set (Am_VALUE, Am_From_Owner(Am_VALUE).Multi_Constraint())
	      .Set (Am_ACTIVE, run_tool_is_selected_and_no_input)
	      .Set (Am_LEAVE_ROOM_FOR_FRINGE, false)
	      .Set_Part(Am_COMMAND, Am_Bitmap.Create("bitmap-in-button")
			.Set(Am_IMAGE, one_zero_formula)));
  zero_one_proto.Get_Object(PICTURE).Get_Object(Am_INTERACTOR)
    .Get_Object(Am_COMMAND).Set(Am_LABEL, "Toggle_Value");
  Am_Object obj;
  Am_Object zero_one_proto_command = Am_Command.Create()
    .Set(Am_LABEL, obj = zero_one_proto.Create())
    .Set(Am_ID, zero_one_proto)
    .Set(Am_IMPLEMENTATION_PARENT, true) //not undoable
    ;
  obj.Get_Object(PICTURE).Set(Am_ACTIVE_2, false);

  pathname = Am_Merge_Pathname(AND_BITMAP);
  image = Am_Image_Array(pathname);
  delete [] pathname;
  and_or_proto = circuit_object_proto.Create()
    .Add (Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
    .Add (Am_SLOTS_TO_SAVE, Am_Value_List()
	  .Add(Am_LEFT).Add(Am_TOP).Add(INPUT_1).Add(INPUT_2).Add(OUTPUT_1)
	  .Add(Am_VALUE))
    .Add_Part(OUTPUT_VALUE, Am_Text.Create("Output_Value")
	      .Set(Am_TEXT, compute_val_text)
	      .Set(Am_FONT, small_font)
	      .Set(Am_LINE_STYLE, Am_Blue)
	      .Set(Am_LEFT, output_val_left)
	      .Set(Am_WIDTH, 5)
	      .Set(Am_TOP, output_val_top))
    .Add_Part(INPUT_2_PORT, in_port_proto.Create()
	      .Set(Am_TOP, picture_bottom_minus_offset))
    .Add_Part(PICTURE, Am_Bitmap.Create("and_or_bitmap")
	      .Set (Am_IMAGE, NULL)
	      .Set (Am_LEFT, 3));
  and_or_proto.Get_Object(INPUT_1_PORT).Set(Am_TOP, ICON_TOP_OFFSET);
  
  and_proto = and_or_proto.Create("And_Gate")
    .Set(Am_VALUE, and_value.Multi_Constraint());
  and_proto.Get_Object(PICTURE).Set (Am_IMAGE, image);

  pathname = Am_Merge_Pathname(NOT_BITMAP);
  image = Am_Image_Array(pathname);
  delete [] pathname;
  not_proto = and_or_proto.Create("Not_Gate")
    .Set(Am_VALUE, not_value.Multi_Constraint())
    .Add(INPUT_2, Am_No_Value)
    ;
  not_proto.Get_Object(PICTURE).Set (Am_IMAGE, image);
  not_proto.Remove_Part(INPUT_2_PORT);
  not_proto.Get_Object(INPUT_1_PORT).Set (Am_TOP, picture_center_y);

  pathname = Am_Merge_Pathname(OR_BITMAP);
  image = Am_Image_Array(pathname);
  delete [] pathname;
  or_proto = and_or_proto.Create("Or_Gate")
    .Set(Am_VALUE, or_value.Multi_Constraint());
  or_proto.Get_Object(PICTURE).Set (Am_IMAGE, image);

  pathname = Am_Merge_Pathname(XOR_BITMAP);
  image = Am_Image_Array(pathname);
  delete [] pathname;
  xor_proto = and_or_proto.Create("Xor_Gate")
    .Set(Am_VALUE, xor_value.Multi_Constraint());
  xor_proto.Get_Object(PICTURE).Set(Am_IMAGE, image);

  line_proto = Am_Line.Create("Wire")
	.Add (INPUT_PLACE, center_right)
	.Add (OUTPUT_PLACE, center_left)
	.Add (INPUT_1, 0)
	// .Set_Inherit_Rule(INPUT_1, Am_LOCAL)
	.Add (OUTPUT_1, 0)
	// .Set_Inherit_Rule(OUTPUT_1, Am_LOCAL)
	.Set (Am_X1, line_x1y1)
	.Set (Am_X2, line_x2y2)
	.Add (IN_VALUE, -1)
	.Set (IN_VALUE, out_value_anim) //value from obj in my INPUT_1 slot
	.Add (Am_VALUE, -1) // Am_Same_As(IN_VALUE))  // -1) 

        .Set (Am_LEFT, 0)
        .Set (Am_TOP, 0)
        .Set (Am_WIDTH, 16)
        .Set (Am_HEIGHT, 8)
        .Add (Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
	.Add (Am_SLOTS_TO_SAVE, Am_Value_List()
	      .Add(INPUT_1).Add(INPUT_PLACE).Add(OUTPUT_1).Add(OUTPUT_PLACE)
	      .Add(Am_VALUE))
	//make so can't move or grow lines
	.Add_Part(Am_INACTIVE_COMMANDS, Am_Command.Create("Line_MG_Inactive")
		  .Add(Am_MOVE_INACTIVE, true)
		  .Add(Am_GROW_INACTIVE, true)
		  .Add(Am_COPY_INACTIVE, true)
		  .Add(Am_DUPLICATE_INACTIVE, true)
		  .Add(Am_CUT_INACTIVE, true));
  Am_Default_Load_Save_Context.Register_Prototype ("ONE_ZERO", zero_one_proto);
  Am_Default_Load_Save_Context.Register_Prototype ("WIRE", line_proto);
  Am_Default_Load_Save_Context.Register_Prototype ("AND", and_proto);
  Am_Default_Load_Save_Context.Register_Prototype ("XOR", xor_proto);
  Am_Default_Load_Save_Context.Register_Prototype ("OR", or_proto);
  Am_Default_Load_Save_Context.Register_Prototype ("NOT", not_proto);

  // ** TEMP
  Am_TEMP_Register_Prototype("AND", and_proto);
  Am_TEMP_Register_Prototype("XOR", xor_proto);
  Am_TEMP_Register_Prototype("OR", or_proto);
  Am_TEMP_Register_Prototype("NOT", not_proto);
  Am_TEMP_Register_Prototype("ONE_ZERO", zero_one_proto);
  Am_TEMP_Register_Prototype("WIRE", line_proto);

  tool_panel = Am_Button_Panel.Create ("tool panel")
    .Set (Am_LEFT, 10)
    .Set (Am_TOP, 40)
    .Set (Am_FIXED_HEIGHT, true)
    .Set (Am_FINAL_FEEDBACK_WANTED, true)
    .Set (Am_ITEM_OFFSET, -1) //closer together
    .Set (Am_ITEMS, Am_Value_List()
          .Add(arrow_proto)
          .Add(arrow_run_proto)
          .Add(line_proto)
          .Add(and_proto)
          .Add(or_proto)
          .Add(xor_proto)
          .Add(not_proto)
	  .Add(zero_one_proto_command))
    .Set(Am_VALUE, arrow_proto);

  tool_panel.Get_Object(Am_COMMAND)
    .Set(Am_IMPLEMENTATION_PARENT, true) //not undoable
    .Set(Am_DO_METHOD, clear_selection);

  scroller = Am_Scrolling_Group.Create("scroller")
    .Set (Am_LEFT, right_of_tool_panel)
    .Set (Am_TOP, 40)
    .Set (Am_INNER_WIDTH, 1000)
    .Set (Am_INNER_HEIGHT, 1000)
    .Set (Am_INNER_FILL_STYLE, Am_White)
    .Set (Am_WIDTH, scroll_width)
    .Set (Am_HEIGHT, scroll_height) ;

  created_objs = Am_Group.Create("created_objs")
    .Set (Am_LEFT, 0)
    .Set (Am_TOP, 0)
    .Set (Am_WIDTH, 1000)
    .Set (Am_HEIGHT, 1000)
    ;
  my_selection = Am_Selection_Widget.Create()
    .Set(Am_OPERATES_ON, created_objs)
    .Set(Am_ACTIVE, selection_tool_is_selected)
    .Set(Am_GRID_X, 30)
    .Set(Am_GRID_Y, 30) ;
  Am_Object del_also = Am_Command.Create("del_also")
    .Set(Am_DO_METHOD, do_delete_lines_also)
    .Set(Am_UNDO_METHOD, undo_delete_lines_also)
    .Set(Am_REDO_METHOD, undo_delete_lines_also)
    .Set(Am_OBJECT_MODIFIED, same_as_impl_child)
    .Set (Am_SELECTIVE_REPEAT_SAME_METHOD, do_delete_lines_also)
    .Set (Am_SELECTIVE_REPEAT_ON_NEW_METHOD,
	  selective_repeat_new_del_also)

    .Set(Am_LABEL, "Delete");
    
  Am_Object my_undo_dialog = Am_Undo_Dialog_Box.Create("My_Undo_Dialog")
    .Set(Am_LEFT, 550)
    .Set(Am_TOP, 200)
    .Set(Am_UNDO_HANDLER_TO_DISPLAY, undo_handler)
    .Set(Am_SELECTION_WIDGET, my_selection)
    .Set(Am_SCROLLING_GROUP_SLOT, scroller)
    .Set(Am_VISIBLE, false)
    ;
  Am_Screen.Add_Part(my_undo_dialog);
  Am_Object search_dialog = Am_Create_Search_Dialog_For(undo_handler)
    .Set(Am_SELECTION_WIDGET, my_selection);

  Am_Object open_command;
  menu_bar = Am_Menu_Bar.Create()
    .Set(Am_SELECTION_WIDGET, my_selection)
    .Set(Am_ITEMS, Am_Value_List ()
         .Add (Am_Command.Create("File_Command")
               .Set(Am_IMPLEMENTATION_PARENT, true) //not undoable
               .Set(Am_LABEL, "File")
               .Set(Am_ITEMS, Am_Value_List ()
                     .Add (open_command = Am_Open_Command.Create()
			   .Set(Am_HANDLE_OPEN_SAVE_METHOD,read_file_contents))
                     .Add (Am_Save_As_Command.Create()
			   .Set(Am_HANDLE_OPEN_SAVE_METHOD, contents_for_save))
                     .Add (Am_Save_Command.Create()
			   .Set(Am_HANDLE_OPEN_SAVE_METHOD, contents_for_save))
                     .Add (Am_Quit_No_Ask_Command.Create())))
           .Add (Am_Command.Create("Edit_Command")
                 .Set(Am_LABEL, "Edit")
                 .Set(Am_IMPLEMENTATION_PARENT, true) //not undoable
                 .Set(Am_ITEMS, Am_Value_List ()
                      .Add (Am_Undo_Command.Create())
                      .Add (Am_Redo_Command.Create())
                      .Add (Am_Show_Undo_Dialog_Box_Command.Create()
                            .Add(Am_UNDO_DIALOG_BOX_SLOT, my_undo_dialog))
                      .Add (Am_Menu_Line_Command.Create())
                      .Add (Am_Graphics_Cut_Command.Create()
			    .Set(Am_IMPLEMENTATION_PARENT, del_also.Create()))
                      .Add (Am_Graphics_Copy_Command.Create())
                      .Add (Am_Graphics_Paste_Command.Create())
                      .Add (Am_Graphics_Clear_Command.Create()
			    .Set(Am_IMPLEMENTATION_PARENT, del_also.Create()))
                      .Add (Am_Graphics_Clear_All_Command.Create())
                      .Add (Am_Menu_Line_Command.Create())
		      .Add (Am_Graphics_Duplicate_Command.Create())
                      .Add (Am_Selection_Widget_Select_All_Command.Create())
                      .Add (Am_Menu_Line_Command.Create())
                      .Add (Am_Show_Search_Command.Create()
			    .Set(Am_SEARCH_DIALOG, search_dialog))
		      ))
           .Add (Am_Command.Create("Arrange_Command")
                 .Set(Am_LABEL, "Arrange")
                 .Set(Am_IMPLEMENTATION_PARENT, true) //not undoable
                 .Set(Am_ITEMS, Am_Value_List ()
                      .Add (Am_Graphics_To_Top_Command.Create())
                      .Add (Am_Graphics_To_Bottom_Command.Create())
		      .Add (animations_command))));
  Am_Object rfeedback = Am_Rectangle.Create ("rfeedback")
    .Set (Am_FILL_STYLE, 0)
    .Set (Am_LINE_STYLE, Am_Dotted_Line)
    .Set (Am_VISIBLE, 0) ;
  Am_Object lfeedback = Am_Line.Create ("lfeedback")
    .Set (Am_LINE_STYLE, Am_Dotted_Line)
    .Set (Am_VISIBLE, 0) ;
  scroller.Add_Part(created_objs)
    .Add_Part(lfeedback)
    .Add_Part(rfeedback)
    .Add_Part(my_selection) ;
  Am_Object create_wire_command =
        Am_Object_Create_Command.Create("create_wire_command")
    .Set(Am_LABEL, "Create_Wire")
    .Set(Am_DO_METHOD, NULL)
    .Set(Am_UNDO_METHOD, create_line_undo)
    .Set(Am_REDO_METHOD, create_line_undo)
    //standard selective_undo_allowed method is OK
    .Set (Am_SELECTIVE_UNDO_METHOD, create_line_undo)
    .Set (Am_SELECTIVE_REPEAT_SAME_ALLOWED,
	  selective_repeat_same_create_line_allowed)
    .Set (Am_SELECTIVE_REPEAT_SAME_METHOD,
	  selective_repeat_same_create_line)
    .Set (Am_SELECTIVE_REPEAT_NEW_ALLOWED,
	  selective_repeat_new_create_line_allowed)
    .Set (Am_SELECTIVE_REPEAT_ON_NEW_METHOD,
	  selective_repeat_new_create_line)
    ;
  created_objs
    .Add_Part (Am_New_Points_Interactor.Create("create_line")
	       .Set(Am_AS_LINE, true)
	       .Set(Am_FEEDBACK_OBJECT, lfeedback)
	       .Set(Am_CREATE_NEW_OBJECT_METHOD, create_new_line)
	       .Set(Am_ACTIVE, line_tool_is_selected)
	       .Set_Part(Am_COMMAND, create_wire_command.Create()))
    .Add_Part (Am_New_Points_Interactor.Create("create_proto")
	       .Set(Am_FEEDBACK_OBJECT, rfeedback)
	       .Set(Am_CREATE_NEW_OBJECT_METHOD, create_new_proto)
	       .Set(Am_HOW_MANY_POINTS, 1)
	       .Set(Am_MINIMUM_WIDTH, minw_from_tool)
	       .Set(Am_MINIMUM_HEIGHT, minh_from_tool)
	       .Set(Am_ACTIVE, bm_tool_is_selected)
	       .Set(Am_GRID_X, 30)
	       .Set(Am_GRID_Y, 30));
  circuit_win.Add_Part(menu_bar)
    .Add_Part(scroller)
    .Add_Part(tool_panel);

  Am_Object anim_proto = Am_Animator.Create ("anim_proto");
  anim_proto.Get_Object(Am_COMMAND)
    .Set(Am_DO_METHOD, anim_set_value)
    .Set(Am_ABORT_DO_METHOD, anim_abort_value);
  animation_proto = Am_Text.Create("animation_proto")
    .Set(Am_FONT, small_font)
    .Set(Am_LINE_STYLE, Am_Red)
    .Set(Am_LEFT, Am_Animate_With (anim_proto))
    .Set(Am_TOP, Am_Animate_With (anim_proto));
  
  Am_Object gesture_feedback = Am_Polygon.Create("gesture feedback")
     .Set (Am_FILL_STYLE, Am_No_Style)
     .Set (Am_VISIBLE, false);
  scroller.Add_Part (gesture_feedback);
  pathname = Am_Merge_Pathname(CLASSIFIER_FILENAME);
  Am_Gesture_Classifier gc(pathname);
  delete [] pathname;
  if (!gc.Valid()) Am_Error ("gesture classifier not found");
  Am_Object gesture_reader = Am_Gesture_Interactor.Create("gesture");
  gesture_reader
    .Set_Part(Am_COMMAND, Am_Gesture_Unrecognized_Command.Create())
    .Set (Am_START_WHEN, "any_right_down")
    .Set (Am_FEEDBACK_OBJECT, gesture_feedback)
    .Set (Am_CLASSIFIER, gc)
    .Set (Am_MAX_DIST_TO_MEAN, 200)
    .Set (Am_ITEMS, Am_Value_List ()
          .Add (Am_Gesture_Create_Command.Create()
                .Set (Am_LABEL, "and")
                .Add (NEW_OBJECT_PROTO, and_proto)
		.Set (Am_CREATE_NEW_OBJECT_METHOD, gesture_creator))
          .Add (Am_Gesture_Create_Command.Create()
                .Set (Am_LABEL, "or")
                .Add (NEW_OBJECT_PROTO, or_proto)
		.Set (Am_CREATE_NEW_OBJECT_METHOD, gesture_creator))
          .Add (Am_Gesture_Create_Command.Create()
                .Set (Am_LABEL, "xor")
                .Add (NEW_OBJECT_PROTO, xor_proto)
		.Set (Am_CREATE_NEW_OBJECT_METHOD, gesture_creator))
          .Add (Am_Gesture_Create_Command.Create()
                .Set (Am_LABEL, "not")
                .Add (NEW_OBJECT_PROTO, not_proto)
		.Set (Am_CREATE_NEW_OBJECT_METHOD, gesture_creator))
          .Add (Am_Undo_Command.Create()
                .Set (Am_LABEL, "undo"))
          .Add (Am_Gesture_Create_Command.Create()
		.Set (Am_LABEL, "line")
		.Set (Am_AS_LINE, true)
		.Add (NEW_OBJECT_PROTO, line_proto)
		.Set (Am_CREATE_NEW_OBJECT_METHOD, gesture_creator)
		.Set (Am_IMPLEMENTATION_PARENT, create_wire_command.Create()))
          .Add (Am_Gesture_Create_Command.Create()
                .Set (Am_LABEL, "onezero")
                .Add (NEW_OBJECT_PROTO, zero_one_proto)
		.Set (Am_CREATE_NEW_OBJECT_METHOD, gesture_creator)));
  created_objs.Add_Part (gesture_reader);

  Am_Screen.Add_Part (circuit_win);


  if (argc > 1) {
    Am_String s = argv[1];
    Am_Standard_Open_From_Filename(open_command, s);
  }
  Am_Main_Event_Loop ();
  Am_Cleanup ();
  return 0;
}

