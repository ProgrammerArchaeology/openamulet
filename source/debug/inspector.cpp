/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file contains the inspector for debugging Amulet objects

   Designed and implemented by Brad Myers
*/

#include <amulet.h>

//define this flag make the inspector use widgets
#define INSPECTOR_USE_WIDGETS

#include <stdlib.h>
#include <string.h>

#include <amulet/am_io.h>
#include <amulet/am_strstream.h>

#include <amulet/opal.h>>
#include <amulet/inter.h>
#include <amulet/types.h>
#include <amulet/debugger.h>
#include <amulet/misc.h>
#include <amulet/registry.h>

#ifdef INSPECTOR_USE_WIDGETS
#include <amulet/widgets.h>
#endif

#include <amulet/standard_slots.h>
#include <amulet/object_advanced.h> // for Am_Constraint_Iterator, Am_Set_Reason
#include <amulet/value_list.h>      // for Am_Value_List type
#include <amulet/formula_advanced.h>
#include <amulet/opal_advanced.h>  // for Am_DRAWONABLE
#include <amulet/inter_advanced.h> // for Am_Modal_Windows

#include <amulet/text_fns.h> // for Am_Set_Cut_Buffer
#include <amulet/gem.h>      // for gem level drawing, drawonables.

#include <amulet/anim.h> // for animators
#include <amulet/initializer._h>

#define MAX_WIDTH_WINDOW 500
#define MAX_WIDTH_LIST 350 // max width of a list before new-line
#define GROUP_OFFSET 4

Am_Object Am_Inspector_Inter; // prototype for inspector interactors
Am_Object Am_Show_Position_Inter;
Am_Object Am_Ask_Inspect_Inter; // **TEMP

Am_Object Am_Ask_Object_Dialog; // dialog box window for ask object to inspect
Am_Object Am_Ask_Inter_Dialog;  // dialog box window for ask inter to trace

Am_Input_Char Am_Show_Inspector_Char; // default = F1
Am_Input_Char Am_Show_Position_Char;  // default = F2
Am_Input_Char Am_Ask_Inspect_Char;    // **TEMP  // default = F3

// a list of all the top level
// windows the inspector currently has open, so we can close them all.
// Each window keeps track of its popups already. Also used by automatic refresh
Am_Value_List Am_All_Inspector_Windows;

Am_Slot_Key Am_INSPECTING_SLOT = Am_Register_Slot_Name("INSPECTING_SLOT");
Am_Slot_Key Am_INSPECTING_SLOT_KEY =
    Am_Register_Slot_Name("INSPECTING_SLOT_KEY");
Am_Slot_Key Am_INSPECTING_OBJECT = Am_Register_Slot_Name("INSPECTING_OBJECT");
Am_Slot_Key Am_INSPECTING_VALUE = Am_Register_Slot_Name("INSPECTING_VALUE");
Am_Slot_Key Am_INSPECTING_CONSTRAINT =
    Am_Register_Slot_Name("INSPECTING_CONSTRAINT");
Am_Slot_Key Am_VALUE_STRING_OBJECT =
    Am_Register_Slot_Name("VALUE_STRING_OBJECT");

Am_Slot_Key Am_GROUP_OBJECT = Am_Register_Slot_Name("GROUP_OBJECT");
Am_Slot_Key Am_INSPECT_OBJ_LIST = Am_Register_Slot_Name("INSPECT_OBJ_LIST");
Am_Slot_Key Am_EDIT_INTER = Am_Register_Slot_Name("EDIT_INTER");
Am_Slot_Key Am_SELECTED_ANY = Am_Register_Slot_Name("SELECTED_ANY");

Am_Slot_Key Am_TRACING_SLOT = Am_Register_Slot_Name("TRACING_SLOT");
Am_Slot_Key Am_STORING_OLD_VALUES = Am_Register_Slot_Name("STORING_OLD_VALUES");

Am_Slot_Key Am_SLOT_PROPS_WINDOW = Am_Register_Slot_Name("SLOT_PROPS_WINDOW");
Am_Slot_Key Am_USE_SLOT_WINDOW = Am_Register_Slot_Name("USE_SLOT_WINDOW");
Am_Slot_Key Am_CONSTRAINT_DEP_WINDOW =
    Am_Register_Slot_Name("CONSTRAINT_DEP_WINDOW");
Am_Slot_Key Am_PROTOTYPES_OWNERS_WINDOW =
    Am_Register_Slot_Name("PROTOTYPES_OWNERS_WINDOW");
Am_Slot_Key Am_OLD_VALUE_STRINGS = Am_Register_Slot_Name("OLD_VALUE_STRINGS");
Am_Slot_Key Am_MAIN_INSPECTOR_WINDOW =
    Am_Register_Slot_Name("MAIN_INSPECTOR_WINDOW");
Am_Slot_Key Am_SORT_BY_NAME = Am_Register_Slot_Name("SORT_BY_NAME");
Am_Slot_Key Am_HIDE_INHERITED = Am_Register_Slot_Name("HIDE_INHERITED");
Am_Slot_Key Am_HIDE_INTERNAL = Am_Register_Slot_Name("HIDE_INTERNAL");
Am_Slot_Key Am_SHOW_PARTS = Am_Register_Slot_Name("SHOW_PARTS");
Am_Slot_Key Am_SHOW_INSTANCES = Am_Register_Slot_Name("SHOW_INSTANCES");

#define Am_HIDE_INHERITED_DEFAULT 0
#define Am_HIDE_INHERITED_TEXT "Hide Inherited Slots"
#define Am_SHOW_INHERITED_TEXT "Show Inherited Slots"
#define Am_HIDE_INHERITED_DEFAULT_TEXT                                         \
  Am_HIDE_INHERITED_DEFAULT ? Am_SHOW_INHERITED_TEXT : Am_HIDE_INHERITED_TEXT

#define Am_HIDE_INTERNAL_DEFAULT 0
#define Am_HIDE_INTERNAL_TEXT "Hide Internal Slots"
#define Am_SHOW_INTERNAL_TEXT "Show Internal Slots"
#define Am_HIDE_INTERNAL_DEFAULT_TEXT                                          \
  Am_HIDE_INTERNAL_DEFAULT ? Am_SHOW_INTERNAL_TEXT : Am_HIDE_INTERNAL_TEXT

#define Am_SORT_BY_NAME_DEFAULT 1
#define Am_SORT_BY_NAME_TEXT "Sort by Name"
#define Am_STOP_SORTING_TEXT "Stop sorting by Name"
#define Am_SORT_BY_NAME_DEFAULT_TEXT                                           \
  Am_SORT_BY_NAME_DEFAULT ? Am_STOP_SORTING_TEXT : Am_SORT_BY_NAME_TEXT

#define Am_SHOW_PARTS_DEFAULT true
#define Am_SHOW_PARTS_TEXT "Show Parts"
#define Am_HIDE_PARTS_TEXT "Hide Parts"
#define Am_SHOW_PARTS_DEFAULT_TEXT                                             \
  Am_SHOW_PARTS_DEFAULT ? Am_HIDE_PARTS_TEXT : Am_SHOW_PARTS_TEXT

#define Am_SHOW_INSTANCES_DEFAULT false
#define Am_SHOW_INSTANCES_TEXT "Show Instances"
#define Am_HIDE_INSTANCES_TEXT "Hide Instances"
#define Am_SHOW_INSTANCES_DEFAULT_TEXT                                         \
  Am_SHOW_INSTANCES_DEFAULT ? Am_HIDE_INSTANCES_TEXT : Am_SHOW_INSTANCES_TEXT

#define Am_AUTOMATIC_REFRESH_DEFAULT true
#define Am_AUTOMATIC_REFRESH_TEXT "Automatic Refresh"
#define Am_MANUAL_REFRESH_TEXT "Manual Refresh"
bool Am_Automatic_Refresh = Am_AUTOMATIC_REFRESH_DEFAULT;

// make the "default text" depend on the current value of the global.
// That way, new windows will have the correct menu item.
#define Am_REFRESH_DEFAULT_TEXT                                                \
  Am_Automatic_Refresh ? Am_MANUAL_REFRESH_TEXT : Am_AUTOMATIC_REFRESH_TEXT

/*
 Each string object set into the inspector window may have these slots set:
  Am_INSPECTING_SLOT - the slot this line is showing
  Am_INSPECTING_SLOT_KEY - true if this string object corresponds to slot key
  Am_INSPECTING_OBJECT - object this string object is displaying
  Am_INSPECTING_VALUE - true if this string object corresponds to the value
  Am_INSPECTING_CONSTRAINT - set with the pointer to the constraint in the slot
   			  if this string object corresponds to a constraint

 In slot_key strings:
  Am_TRACING_SLOT - true if the slot this corresponds to is being traced with
                    Am_Notify_On_Slot_Set (only if activated from inspector)
  Am_STORING_OLD_VALUES - true if the slot this corresponds to is being
                    traced with Am_Start_Slot_Value_Tracing (from inspector)
  Am_VALUE_STRING_OBJECT - set with the string object corresponding to the
                    value of the slot, if it isn't a value list.

 In value string objects:
  Am_OLD_VALUE_STRINGS - a value list of string objects corresponding to the
                    old values of the slot.  It is stored in the value string
		    obj so the tracer objects can find it properly.

 in the top level inspector window, these slots are set:
  Am_INSPECTING_OBJECT - object being inspected in this window
  Am_GROUP_OBJECT - the main group of all the strings
  			(usually a scrolling group)
  Am_INSPECT_OBJ_LIST - history of other objects shown in this window
  Am_EDIT_INTER - the editing interactor (so double-click inter can abort it)
  Am_SELECTED_ANY - whatever string object is double-click selected
  Am_SLOT_PROPS_WINDOW - holds the slot properties window, if any
  Am_CONSTRAINT_DEP_WINDOW - constraint dependencies
  Am_PROTOTYPES_OWNERS_WINDOW - all the owners and prototypes
  Am_USE_SLOT_WINDOW - constraints that use this slot
  Am_VALUE_STRING_OBJECT - set to string object containing the object being
                           inspected (displayed at top in red)
in popup windows, these slots are set:
  Am_MAIN_INSPECTOR_WINDOW - set with main window (will be (0L) in main wins)
*/

Am_Font reg_font = Am_Font(Am_FONT_FIXED, false);
Am_Font bold_font = Am_Font(Am_FONT_FIXED, true);
Am_Font bold_underline_font = Am_Font(Am_FONT_FIXED, true, false, true);

Am_Style very_light_gray = Am_Style((float)0.95, (float)0.95, (float)0.95);

Am_Define_Font_Formula(pick_font)
{
  Am_Value val;
  Am_Object obj;
  obj = self.Get(Am_INSPECTING_OBJECT);
  if (obj.Valid() || (bool)self.Get(Am_INSPECTING_VALUE)) {
    // then is a value slot or an object, use bold
    if ((bool)self.Get(Am_INTERIM_SELECTED))
      return bold_underline_font;
    else
      return bold_font;
  } else
    return reg_font; //only values can be selected
}

Am_Define_Formula(int, get_window_width)
{
  return (int)self.Get_Object(Am_WINDOW).Get(Am_WIDTH) - GROUP_OFFSET -
         GROUP_OFFSET;
}

Am_Define_Formula(int, get_window_height)
{
  int group_top = self.Get(Am_TOP);
  return (int)self.Get_Object(Am_WINDOW).Get(Am_HEIGHT) - GROUP_OFFSET -
         group_top;
}

static void
do_new_line(int &top, int &left, int &max_left, const Am_Object &string_object,
            int indent = 0)
{
  top += (int)string_object.Get(Am_HEIGHT) + 1;
  if (left > max_left)
    max_left = left;
  left = indent;
}

static Am_Object
add_new_string(Am_Object &string_object, Am_Object &group, int &top, int &left,
               int &max_left, char *str, bool new_line, const Am_Style &color)
{
  if (!string_object.Valid()) {
    string_object = Am_Text.Create().Set(Am_FONT, pick_font);
    group.Add_Part(string_object);
  }
  string_object.Set(Am_LEFT, left)
      .Set(Am_TOP, top)
      .Set(Am_TEXT, str)
      .Set(Am_LINE_STYLE, color)
      .Set(Am_INSPECTING_SLOT, 0, Am_OK_IF_NOT_THERE)
      .Set(Am_INSPECTING_SLOT_KEY, false, Am_OK_IF_NOT_THERE)
      .Set(Am_INSPECTING_OBJECT, (0L), Am_OK_IF_NOT_THERE)
      .Set(Am_INSPECTING_VALUE, false, Am_OK_IF_NOT_THERE)
      .Set(Am_TRACING_SLOT, false, Am_OK_IF_NOT_THERE)
      .Set(Am_VALUE_STRING_OBJECT, (0L), Am_OK_IF_NOT_THERE)
      .Set(Am_STORING_OLD_VALUES, false, Am_OK_IF_NOT_THERE)
      .Set(Am_OLD_VALUE_STRINGS, (0L), Am_OK_IF_NOT_THERE)
      .Set(Am_INSPECTING_CONSTRAINT, (0L), Am_OK_IF_NOT_THERE)
      .Set(Am_INTERIM_SELECTED, false, Am_OK_IF_NOT_THERE);
  left += (int)string_object.Get(Am_WIDTH);

  if (new_line)
    do_new_line(top, left, max_left, string_object);
  return string_object;
}

// adds the string to group by replacing the current string in
// group_iter if possible, or else allocating a new part.  Updates top
// and left (top doesn't change unless new_line is true).
static Am_Object
add_string(Am_Value_List &group_iter, Am_Object &group, int &top, int &left,
           int &max_left, char *str, bool new_line, const Am_Style &color)
{
  Am_Object string_object;
  if (!group_iter.Last()) {
    string_object = group_iter.Get();
    group_iter.Next();
  }
  string_object = add_new_string(string_object, group, top, left, max_left, str,
                                 new_line, color);
  return string_object;
}

// adds a string describing object to group by replacing the current string in
// group_iter if possible, or else allocating a new part.  Updates top
// and left (top doesn't change)
static Am_Object
add_object(Am_Value_List &group_iter, Am_Object &group, int &top, int &left,
           int &max_left, const Am_Object &object, bool new_line = true,
           Am_Style color = Am_Black)
{

  char line[100];
  OSTRSTREAM_CONSTR(oss, line, 100, std::ios::out);
  oss << object << std::ends;
  OSTRSTREAM_COPY(oss, line, 100);

  Am_Object str_obj =
      add_string(group_iter, group, top, left, max_left, line, new_line, color);
  str_obj.Set(Am_INSPECTING_OBJECT, object);
  return str_obj;
}

// sees if the slot contains a constraint, and if so, outputs the
// address of the function.  May leave some "comment" text in oss
static void
check_slot_for_constraints(Am_Value_List &group_iter, Am_Object &group,
                           int &top, int &left, int &max_left, char *line,
                           int line_max, const Am_Style &color, OSTRSTREAM &oss,
                           const Am_Object &object, Am_Slot_Key key)
{
  Am_Object_Advanced obj_adv = (Am_Object_Advanced &)object;
  Am_Slot slot_advanced = obj_adv.Get_Slot_Locale(key).Get_Slot(key);
  Am_Constraint_Iterator constr_iter(slot_advanced);
  bool found_one = false;
  Am_Constraint *constr;
  const char *constr_name;
  Am_Object string_obj;
  Am_Style constraint_color;
  Am_Constraint *proto;
  for (constr_iter.Start(); !constr_iter.Last(); constr_iter.Next()) {
    constr = constr_iter.Get();
    proto = constr->Get_Prototype();
    if (proto)
      constraint_color = Am_Blue;
    else
      constraint_color = color;
    if (found_one)
      oss << ", ";
    else
      oss << " (constraint = ";
    oss << std::ends;
    OSTRSTREAM_COPY(oss, line, line_max);
    add_string(group_iter, group, top, left, max_left, line, false,
               constraint_color);
    reset_ostrstream(oss); // go to the beginning
    constr_name = constr->Get_Name();
    if (constr_name)
      oss << constr_name << " ";
    oss << std::hex << constr << std::dec;
    oss << std::ends;
    OSTRSTREAM_COPY(oss, line, line_max);
    string_obj = add_string(group_iter, group, top, left, max_left, line, false,
                            constraint_color);
    string_obj.Set(Am_INSPECTING_SLOT, key);
    string_obj.Set(Am_INSPECTING_CONSTRAINT, (Am_Ptr)constr);

    Am_Object animator = Am_Get_Animator(constr);
    if (animator.Valid())
      string_obj.Set(Am_INSPECTING_OBJECT, animator);

    reset_ostrstream(oss); // go to the beginning
    found_one = true;
  }
  if (found_one)
    oss << ")";
}

//mutually recursive, so need a declaration
static Am_Object add_value_list(Am_Value &value, Am_Slot_Key key, char *line,
                                int line_max, OSTRSTREAM &oss,
                                Am_Value_List &group_iter, Am_Object &group,
                                int &top, int &left, int &max_left,
                                const Am_Style &color);

void
safe_get(Am_Object object, Am_Slot_Key key, Am_Value &value)
{
  Am_Object_Advanced obj_adv = (Am_Object_Advanced &)object;
  obj_adv = obj_adv.Get_Slot_Locale(key);
  // Use Get_Slot_Locale to prevent the Get_Slot call from inadvertantly
  // localizing the slot.
  Am_Slot slot = obj_adv.Get_Slot(key);
  // Calling Get from the slot directly will not validate the slot.
  value = slot.Get();
}

static Am_Object
add_value(Am_Value &value, Am_Slot_Key key, char *line, int line_max,
          OSTRSTREAM &oss, Am_Value_List &group_iter, Am_Object &group,
          int &top, int &left, int &max_left, bool new_line = true,
          const Am_Style &color = Am_Black, bool value_list_value = false)
{
  Am_Object string_obj;
  if (value.type == Am_OBJECT) {
    string_obj = add_object(group_iter, group, top, left, max_left, value,
                            new_line, color);
    string_obj.Set(Am_INSPECTING_SLOT, key);
    if (!value_list_value)
      string_obj.Set(Am_INSPECTING_VALUE, true);
  } else if (Am_Type_Class(value.type) == Am_WRAPPER &&
             Am_Value_List::Test(value.value.wrapper_value))
    add_value_list(value, key, line, line_max, oss, group_iter, group, top,
                   left, max_left, color);
  else {
    reset_ostrstream(oss); // go to the beginning
    oss << value << std::ends;
    OSTRSTREAM_COPY(oss, line, line_max);
    string_obj = add_string(group_iter, group, top, left, max_left, line,
                            new_line, color);
    string_obj.Set(Am_INSPECTING_SLOT, key);
    if (!value_list_value)
      string_obj.Set(Am_INSPECTING_VALUE, true);
  }
  return string_obj;
}

#define NEW_LINE_INDENT 40
#define INTRA_LIST_SPACE 15

static Am_Object
add_value_list(Am_Value &value, Am_Slot_Key key, char *line, int line_max,
               OSTRSTREAM &oss, Am_Value_List &group_iter, Am_Object &group,
               int &top, int &left, int &max_left, const Am_Style &color)
{
  Am_Object string_object;
  Am_Value_List list;
  list = value;
  int length = list.Length();
  int cnt = 0;
  reset_ostrstream(oss); // go to the beginning
  oss << " LIST(" << length << ") [ " << std::ends;
  OSTRSTREAM_COPY(oss, line, line_max);
  string_object =
      add_string(group_iter, group, top, left, max_left, line, false, color);
  Am_Value v;
  // make sure first name should go on this line
  if (left >= MAX_WIDTH_LIST)
    do_new_line(top, left, max_left, string_object, NEW_LINE_INDENT);
  for (list.Start(); !list.Last(); list.Next()) {
    v = list.Get();
    string_object = add_value(v, key, line, line_max, oss, group_iter, group,
                              top, left, max_left, false, color, true);
    cnt++;
    // if past limit, but not last item
    if (left >= MAX_WIDTH_LIST && cnt < length) {
      do_new_line(top, left, max_left, string_object, NEW_LINE_INDENT);
    } else
      left += INTRA_LIST_SPACE;
  }
  string_object =
      add_string(group_iter, group, top, left, max_left, " ]", false, color);
  //indent here in case list of lists
  do_new_line(top, left, max_left, string_object, NEW_LINE_INDENT);
  return string_object;
}

static void
oss_slot_name_or_num(Am_Slot_Key key, OSTRSTREAM &oss, bool out_ends)
{
  const char *key_name = Am_Get_Slot_Name(key);
  if (key_name)
    oss << key_name;
  else
    oss << "[" << (int)key << "]";
  if (out_ends)
    oss << std::ends;
}

// The value string is the string object corresponding to the value of the
// slot.  Use it to determine where to put the old_value list.
#define OLD_VALUES_COLOR Am_Orange
void
Am_Add_Old_Values(Am_Value_List old_values, Am_Object value_string,
                  Am_Value_List *group_iter)
{
  if (!value_string.Valid()) // || !old_values.Valid())//|| old_values.Empty())
    return;
  Am_Object group = value_string.Get_Owner();
  int left = (int)value_string.Get(Am_LEFT) + (int)value_string.Get(Am_WIDTH) +
             INTRA_LIST_SPACE;
  int top = value_string.Get(Am_TOP);
  int max_left = group.Get(Am_INNER_WIDTH);
  Am_Value_List old_value_strings;
  old_value_strings = value_string.Get(Am_OLD_VALUE_STRINGS);
  if (!old_value_strings.Valid())
    old_value_strings = Am_Value_List();
  old_value_strings.Start();
  Am_Value_List new_value_strings;
  new_value_strings.Start();
  Am_Object string_object;
  Am_Value value;

  char line[250];

  for (old_values.Start(); !old_values.Last(); old_values.Next()) {
    OSTRSTREAM_CONSTR(oss, line, 250, std::ios::out);
    value = old_values.Get();
    reset_ostrstream(oss); // go to the beginning
    oss << value << std::ends;
    OSTRSTREAM_COPY(oss, line, 250);

    if (group_iter)
      string_object = add_string(*group_iter, group, top, left, max_left, line,
                                 false, OLD_VALUES_COLOR);
    else {
      if (!old_value_strings.Last()) {
        string_object = old_value_strings.Get();
        old_value_strings.Delete();
        old_value_strings.Next();
      } else
        string_object = (0L);
      string_object = add_new_string(string_object, group, top, left, max_left,
                                     line, false, OLD_VALUES_COLOR);
    }
    new_value_strings.Add(string_object, Am_TAIL);
    left += INTRA_LIST_SPACE / 2;
    if (left >= max_left) {
      string_object.Set(Am_TEXT, "...");
      // delete the rest of the old_values for efficiency
      break;
    } // stop after it's off the side of the window
  }
  //  if (!group_iter)
  while (!old_value_strings.Last()) {
    string_object = old_value_strings.Get();
    group.Remove_Part(string_object);
    old_value_strings.Next();
  }
  value_string.Set(Am_OLD_VALUE_STRINGS, new_value_strings);
  return;
}

static Am_Object
add_slot_name(Am_Slot_Key key, char *line, OSTRSTREAM &oss, int max_line,
              Am_Value_List &group_iter, Am_Object &group, int &top, int &left,
              int &max_left, const Am_Style &color)
{
  oss_slot_name_or_num(key, oss, true);
  OSTRSTREAM_COPY(oss, line, 100);
  Am_Object string_obj =
      add_string(group_iter, group, top, left, max_left, line, false, color);
  string_obj.Set(Am_INSPECTING_SLOT, key);
  string_obj.Set(Am_INSPECTING_SLOT_KEY, true);
  return string_obj;
}

//set which object is selected in the window
static void
set_selection(Am_Object &window, Am_Object &new_selected)
{
  Am_Object old_selected;
  old_selected = window.Get(Am_SELECTED_ANY);
  if (old_selected.Valid())
    old_selected.Set(Am_INVERT, false);
  window.Set(Am_SELECTED_ANY, new_selected);
  if (new_selected.Valid()) {
    new_selected.Set(Am_INVERT, true);
    Am_String text = new_selected.Get(Am_TEXT);
    Am_Set_Cut_Buffer(window, text);
  }
}

static void
add_slot(Am_Value_List &group_iter, Am_Object &group, int &top, int &left,
         int &max_left, const Am_Object &object, Am_Slot_Key key, bool selectit)
{
  Am_Style color;
  Am_Object string_obj;
  if (object.Is_Slot_Inherited(key))
    color = Am_Blue;
  else
    color = Am_Black; //local

  char line[250];
  OSTRSTREAM_CONSTR(oss, line, 250, std::ios::out);
  reset_ostrstream(oss); // go to the beginning
  left = 28;             //Left margin for each slot name
  string_obj = add_slot_name(key, line, oss, 250, group_iter, group, top, left,
                             max_left, color);
  string_obj.Set(Am_INSPECTING_SLOT, key);
  string_obj.Set(Am_INSPECTING_SLOT_KEY, true);
  if (selectit) {
    Am_Object window = string_obj.Get(Am_WINDOW);
    set_selection(window, string_obj);
  }

  reset_ostrstream(oss); // go to the beginning
  check_slot_for_constraints(group_iter, group, top, left, max_left, line, 250,
                             color, oss, object, key);
  oss << " : " << std::ends;
  OSTRSTREAM_COPY(oss, line, 250);

  add_string(group_iter, group, top, left, max_left, line, false, color);

  Am_Value value;

  safe_get(object, key, value);

  Am_Object value_string_obj =
      add_value(value, key, line, 250, oss, group_iter, group, top, left,
                max_left, true, color);
  string_obj.Set(Am_VALUE_STRING_OBJECT, value_string_obj);
  Am_Trace_Status status = Am_Get_Tracer_Status(object, key);
  string_obj.Set(Am_TRACING_SLOT, status);
  Am_Update_Tracer_String_Object(object, key, value_string_obj);
  if (status & (Am_TRACING | Am_STORING_OLD)) {
    Am_Value_List old_values =
        Am_Get_Tracer_Old_Values(object, key, value_string_obj);
    Am_Add_Old_Values(old_values, value_string_obj, &group_iter);
  }
}

// go through all the slots of object and set a line of group for each
// one.  Adjusts the number of parts of the group appropriately
static void
get_slots_for_object(Am_Object &group, const Am_Object &object, int &max_left,
                     int &top, Am_Slot_Key slot_to_select)
{
  Am_Value_List group_iter;
  group_iter = group.Get(Am_GRAPHICAL_PARTS);
  group_iter.Start();

  Am_Object window, str_obj;
  window = group.Get_Owner();
  bool sorting_by_name = window.Get(Am_SORT_BY_NAME);
  bool hide_inherited = window.Get(Am_HIDE_INHERITED);
  bool hide_internal = window.Get(Am_HIDE_INTERNAL);
  bool show_parts = window.Get(Am_SHOW_PARTS);
  bool show_instances = window.Get(Am_SHOW_INSTANCES);

  char line[256];

  top = 0;
  int left = 0;
  max_left = 0;
  add_string(group_iter, group, top, left, max_left, "Inspecting: ", false,
             Am_Red);
  window.Set(
      Am_VALUE_STRING_OBJECT,
      add_object(group_iter, group, top, left, max_left, object, true, Am_Red));
  add_string(group_iter, group, top, left, max_left, "Instance of ", false,
             Am_Black);
  add_object(group_iter, group, top, left, max_left, object.Get_Prototype());

  add_string(group_iter, group, top, left, max_left, "Part of ", false,
             Am_Black);

  str_obj = add_object(group_iter, group, top, left, max_left,
                       object.Get_Owner(), false);

  Am_Slot_Key part_key = object.Get_Key();
  if ((part_key != Am_NO_INHERIT) && (part_key != Am_NO_SLOT)) {
    OSTRSTREAM_CONSTR(oss, line, 250, std::ios::out);
    add_string(group_iter, group, top, left, max_left, " in part slot ", false,
               Am_Black);
    str_obj = add_slot_name(part_key, line, oss, 250, group_iter, group, top,
                            left, max_left, Am_Black);
  }
  do_new_line(top, left, max_left, str_obj);

  // print some information on the slots being displayed
  strcpy(line, "Slots:"); // CC-friendly

  if (sorting_by_name)
    strcat(line, " Sorted by name.");
  if (hide_inherited)
    strcat(line, " Inherited slots hidden.");
  if (hide_internal)
    strcat(line, " Internal slots hidden.");

  add_string(group_iter, group, top, left, max_left, line, true, Am_Black);
  bool found_one = false;

  Am_Invalidate_All_Tracer_String_Objects(object);

  Am_Slot_Iterator slot_iter(object);
  Am_Slot_Key key;
  Am_Value_List slot_list;
  // create a (sorted) list of the slot keys we want.
  for (slot_iter.Start(); !slot_iter.Last(); slot_iter.Next()) {
    key = (int)slot_iter.Get();
    if (hide_inherited && object.Is_Slot_Inherited(key))
      continue;
    if (hide_internal && (Am_Get_Slot_Name(key))[0] == Am_INTERNAL_SLOT_PREFIX)
      continue;
    if (sorting_by_name) {
      // do an insertion sort into the list
      if (slot_list.Empty())
        slot_list.Add(key);
      else {
        const char *this_slot_name = Am_Get_Slot_Name(key);
        if (!this_slot_name)
          slot_list.Add(key, Am_HEAD); // unnamed slot
        else {
          // find place to insert
          const char *other_slot_name;
          for (slot_list.Start(); !slot_list.Last(); slot_list.Next()) {
            other_slot_name = Am_Get_Slot_Name((int)slot_list.Get());
            // strcmp(a,b) returns a-b (<0 if a<b; >0 if a>b)
            if (other_slot_name != (0L) && // skip it if it's null
                strcmp(this_slot_name, other_slot_name) < 0)
              break;
          }
          slot_list.Insert(key, Am_BEFORE);
        }
      }
    } else
      slot_list.Add(key);
  }
  for (slot_list.Start(); !slot_list.Last(); slot_list.Next()) {
    key = (int)slot_list.Get();
    add_slot(group_iter, group, top, left, max_left, object, key,
             key == slot_to_select);
    found_one = true;
  }
  if (!found_one) {
    add_string(group_iter, group, top, left, max_left, "    (NONE)", true,
               Am_Black);
  }

  if (show_parts) {
    add_string(group_iter, group, top, left, max_left, " Parts:", true,
               Am_Black);

    Am_Part_Iterator part_iter(object);
    Am_Object part;
    found_one = false;

    Am_Slot_Key part_key;
    Am_Object string_object;
    //
    char line[250];
    OSTRSTREAM_CONSTR(oss, line, 250, std::ios::out);
    reset_ostrstream(oss); // go to the beginning

    for (part_iter.Start(); !part_iter.Last(); part_iter.Next()) {
      part = part_iter.Get();
      left = 28; //Left margin for each part name
      string_object = add_object(group_iter, group, top, left, max_left, part,
                                 false, Am_Black);
      //output where slot is and inherited
      part_key = part.Get_Key();
      if (part_key == Am_NO_INHERIT)
        add_string(group_iter, group, top, left, max_left,
                   "   *Won't be Inherited*", true, Am_Black);
      else if (part_key != Am_NO_SLOT) {
        reset_ostrstream(oss); // go to the beginning
        oss << "   in part slot ";

        oss_slot_name_or_num(part_key, oss, true);
        OSTRSTREAM_COPY(oss, line, 250);

        add_string(group_iter, group, top, left, max_left, line, true,
                   Am_Black);
      } else
        do_new_line(top, left, max_left, string_object, 28);

      found_one = true;
    }
    if (!found_one) {
      add_string(group_iter, group, top, left, max_left, "    (NONE)", true,
                 Am_Black);
    }
  }

  if (show_instances) {
    add_string(group_iter, group, top, left, max_left, " Instances:", true,
               Am_Black);

    Am_Instance_Iterator inst_iter(object);
    Am_Object inst;
    found_one = false;

    // create a (sorted) list of the parts
    for (inst_iter.Start(); !inst_iter.Last(); inst_iter.Next()) {
      inst = inst_iter.Get();
      left = 28; //Left margin for each instance name
      add_object(group_iter, group, top, left, max_left, inst);
      found_one = true;
    }
    if (!found_one) {
      add_string(group_iter, group, top, left, max_left, "    (NONE)", true,
                 Am_Black);
    }
  }
  // now get rid of any extra parts of the group
  Am_Object part;
  for (; !group_iter.Last(); group_iter.Next()) {
    part = group_iter.Get();
    group.Remove_Part(part);
  }
}

static void
refresh_display_of_object(Am_Object &window, const Am_Object &object,
                          Am_Slot_Key slot_to_select = Am_NO_SLOT)
{
  Am_Object group = window.Get_Object(Am_GROUP_OBJECT);
  int width, height;
  get_slots_for_object(group, object, width, height, slot_to_select);
  width += 20; //fudge factor
  height += 5;
  group.Set(Am_INNER_HEIGHT, height);
  group.Set(Am_INNER_WIDTH, width);
  group.Set(Am_LEFT_OFFSET, 0); //go back to the top each time
  group.Set(Am_TOP_OFFSET, 0);

#if 0
  // automatic resizing code
  // removed because it resized the window frequently and undesirably
  // -- rcm
  int max_height = Am_Screen.Get(Am_HEIGHT);
  if (width > MAX_WIDTH_WINDOW) window.Set(Am_WIDTH, MAX_WIDTH_WINDOW);
  else window.Set(Am_WIDTH, width+25);
  int bottom = (int)group.Get(Am_TOP) + 5;
  if (height + bottom + 25 > max_height) window.Set(Am_HEIGHT, max_height);
  else window.Set(Am_HEIGHT, height + bottom+25);
#endif
}

////////////////////////////////////////////////////////////////
// Main top-level function for refilling the Inspector Window
////////////////////////////////////////////////////////////////
static void
set_object_into_window(Am_Object &window, const Am_Object &object,
                       Am_Slot_Key slot_to_select = Am_NO_SLOT)
{
  Am_Object_Advanced obj_adv = (Am_Object_Advanced &)object;

  char line[250];
  OSTRSTREAM_CONSTR(oss, line, 250, std::ios::out);
  reset_ostrstream(oss); // go to the beginning
  // use object_advanced printing to get the data address printed also
  oss << "Inspecting " << obj_adv << std::ends;
  OSTRSTREAM_COPY(oss, line, 250);
  window.Set(Am_TITLE, line);
  //in icon, just use object name
  reset_ostrstream(oss); // go to the beginning
  oss << object << std::ends;
  OSTRSTREAM_COPY(oss, line, 250);
  window.Set(Am_ICON_TITLE, line);

  window.Set(Am_INSPECTING_OBJECT, object);
  set_selection(window, Am_No_Object);

  Am_Value_List obj_list;
  obj_list = window.Get(Am_INSPECT_OBJ_LIST);
  obj_list.Add(object, Am_HEAD); // *** use destructive modification??
  window.Set(Am_INSPECT_OBJ_LIST, obj_list);
  refresh_display_of_object(window, object, slot_to_select);
}

////////////////////////////////////////////////////////////////
// Utilities for the pop-up windows
////////////////////////////////////////////////////////////////

static void
display_pop_up_window(Am_Object &pop_win, char *line, int width, int height,
                      Am_Object &main_inspector_window, Am_Object &string_obj)
{
  //done, now set up window
  pop_win.Set(Am_TITLE, line);
  pop_win.Set(Am_ICON_TITLE, line);
  pop_win.Set(Am_VISIBLE, true);
  pop_win.Set(Am_WIDTH, width + 5);
  pop_win.Set(Am_HEIGHT, height + 2);

  int win_left, win_top;
  Am_Translate_Coordinates(string_obj, -(width + 30), -24, Am_Screen, win_left,
                           win_top);
  if (win_left < 0) { //then put it on the right
    win_left = (int)main_inspector_window.Get(Am_LEFT) +
               (int)main_inspector_window.Get(Am_WIDTH) - 10;
  }
  if (win_top < 0)
    win_top = 0;
  pop_win.Set(Am_LEFT, win_left);
  pop_win.Set(Am_TOP, win_top);
  Am_To_Top(pop_win);
}

//forward declaration
static void create_inspector_interactors(Am_Object &window, bool want_edit);

static Am_Object
get_pop_up_window(Am_Object &main_inspector_window, Am_Slot_Key pop_up_slot,
                  Am_Object &for_obj, Am_Value_List &group_iter)
{
  Am_Object pop_up_window;
  pop_up_window = main_inspector_window.Get(pop_up_slot);
  if (!pop_up_window.Valid()) {
    pop_up_window = Am_Window.Create()
                        .Set(Am_FILL_STYLE, very_light_gray)
                        .Set(Am_MAIN_INSPECTOR_WINDOW, main_inspector_window,
                             Am_OK_IF_NOT_THERE)
                        .Set(Am_DESTROY_WINDOW_METHOD, Am_Window_Hide_Method);
    pop_up_window.Set(Am_GROUP_OBJECT, pop_up_window, Am_OK_IF_NOT_THERE);
    create_inspector_interactors(pop_up_window, false);
    Am_Screen.Add_Part(pop_up_window);
    main_inspector_window.Set(pop_up_slot, pop_up_window);
  }
  for_obj = main_inspector_window.Get(Am_INSPECTING_OBJECT);
  group_iter = pop_up_window.Get(Am_GRAPHICAL_PARTS);
  group_iter.Start();

  return pop_up_window;
}

static void
add_slot_and_obj(Am_Slot_Key key, Am_Object &for_obj, Am_Value_List &group_iter,
                 Am_Object &group, int &top, int &left, int &max_left,
                 char *line, OSTRSTREAM &oss, int max_line, bool want_in_slot,
                 bool new_line = false, const Am_Style &color = Am_Black)
{
  if (want_in_slot)
    add_string(group_iter, group, top, left, max_left, " in slot  ", false,
               color);
  add_slot_name(key, line, oss, max_line, group_iter, group, top, left,
                max_left, color);
  add_string(group_iter, group, top, left, max_left, " of ", false, color);
  add_object(group_iter, group, top, left, max_left, for_obj, new_line, color);
}

////////////////////////////////////////////////////////////////
// Constraint dependencies window
////////////////////////////////////////////////////////////////

//forward declaration because recursive
static void add_constraint_dependencies(Am_Value_List &group_iter,
                                        Am_Object &group, int &top, int &left,
                                        int &max_left, char *line, int line_max,
                                        OSTRSTREAM &oss,
                                        Am_Formula_Advanced *form, int depth);

static const char *
add_constraint_name_and_deps(Am_Constraint *constr, Am_Value_List &group_iter,
                             Am_Object &group, int &top, int &left,
                             int &max_left, char *line, int line_max,
                             OSTRSTREAM &oss, int depth, Am_Slot_Key key,
                             Am_Object &for_obj, bool show_slot)
{
  int i;
  Am_Style color = Am_Black;
  if (depth == 0)
    color = Am_Red;
  const char *constr_name = constr->Get_Name();
  if (!show_slot) {
    reset_ostrstream(oss); // go to the beginning
    for (i = 0; i < depth; i++)
      oss << "  "; //indent
    oss << "Contains Constraint  ";
  }
  if (constr_name)
    oss << constr_name << " ";
  oss << std::hex << constr << std::dec << std::ends;
  OSTRSTREAM_COPY(oss, line, line_max);

  add_string(group_iter, group, top, left, max_left, line, true, color);
  if (show_slot) {
    reset_ostrstream(oss); // go to the beginning
    add_slot_and_obj(key, for_obj, group_iter, group, top, left, max_left, line,
                     oss, true, true, Am_Red);
  }
  if (Am_Formula_Advanced::Test(constr)) {
    Am_Formula_Advanced *form = Am_Formula_Advanced::Narrow(constr);
    reset_ostrstream(oss); // go to the beginning
    add_constraint_dependencies(group_iter, group, top, left, max_left, line,
                                line_max, oss, form, depth + 1);
  } else
    add_string(group_iter, group, top, left, max_left,
               "      UNKNOWN because not FORMULA", true, Am_Black);
  return constr_name;
}

#define MAX_PRINT_DEPTH 6

static void
check_and_add_constraints_for(const Am_Slot &dep_slot,
                              Am_Value_List &group_iter, Am_Object &group,
                              int &top, int &left, int &max_left, char *line,
                              int line_max, OSTRSTREAM &oss, int depth)
{
  int i;
  if (depth > MAX_PRINT_DEPTH) {
    reset_ostrstream(oss); // go to the beginning
    for (i = 0; i < depth; i++)
      oss << "  "; //indent
    oss << "..." << std::ends;
    OSTRSTREAM_COPY(oss, line, line_max);

    add_string(group_iter, group, top, left, max_left, line, true, Am_Black);
  } else {
    Am_Constraint_Iterator constr_iter(dep_slot);
    Am_Constraint *constr;
    Am_Object string_obj;
    for (constr_iter.Start(); !constr_iter.Last(); constr_iter.Next()) {
      constr = constr_iter.Get();
      add_constraint_name_and_deps(constr, group_iter, group, top, left,
                                   max_left, line, line_max, oss, depth + 1, 0,
                                   Am_No_Object, false);
    }
  }
}

static void
add_constraint_dependencies(Am_Value_List &group_iter, Am_Object &group,
                            int &top, int &left, int &max_left, char *line,
                            int line_max, OSTRSTREAM &oss,
                            Am_Formula_Advanced *form, int depth)
{
  Am_Depends_Iterator dep_iter = form;
  Am_Slot dep_slot;
  Am_Object_Advanced dep_obj_adv;
  Am_Object dep_obj;
  Am_Slot_Key dep_key;
  int i;
  Am_Value value;
  for (dep_iter.Start(); !dep_iter.Last(); dep_iter.Next()) {
    dep_slot = dep_iter.Get();
    reset_ostrstream(oss); // go to the beginning
    for (i = 0; i < depth; i++)
      oss << "  "; //indent
    dep_key = dep_slot.Get_Key();
    dep_obj_adv = dep_slot.Get_Owner();
    dep_obj = dep_obj_adv;
    add_slot_and_obj(dep_key, dep_obj, group_iter, group, top, left, max_left,
                     line, oss, false, false, Am_Black);
    add_string(group_iter, group, top, left, max_left, " =  ", false, Am_Black);
    safe_get(dep_obj, dep_key, value);
    add_value(value, dep_key, line, line_max, oss, group_iter, group, top, left,
              max_left, true, Am_Black);
    reset_ostrstream(oss); // go to the beginning
    check_and_add_constraints_for(dep_slot, group_iter, group, top, left,
                                  max_left, line, line_max, oss, depth + 1);
  }
}

static void
display_constraint_dependencies_window(Am_Object &main_inspector_window,
                                       Am_Constraint *constr, Am_Slot_Key key,
                                       Am_Object &string_obj)
{
  Am_Object pop_up_window, for_obj;
  Am_Value_List group_iter;
  pop_up_window = get_pop_up_window(
      main_inspector_window, Am_CONSTRAINT_DEP_WINDOW, for_obj, group_iter);
  int top = 0;
  int left = 0;
  int max_left = 0;

  char line[250];
  OSTRSTREAM_CONSTR(oss, line, 250, std::ios::out);
  reset_ostrstream(oss); // go to the beginning
  // title
  oss << "Dependencies of constraint ";
  OSTRSTREAM_COPY(oss, line, 250);

  const char *constr_name = add_constraint_name_and_deps(
      constr, group_iter, pop_up_window, top, left, max_left, line, 250, oss, 0,
      key, for_obj, true);
  //done, now set up window
  reset_ostrstream(oss); // go to the beginning
  if (constr_name)
    oss << constr_name;
  else
    oss << std::hex << constr << std::dec;
  oss << " Depends On" << std::ends;
  OSTRSTREAM_COPY(oss, line, 250);

  display_pop_up_window(pop_up_window, line, max_left, top,
                        main_inspector_window, string_obj);
}

////////////////////////////////////////////////////////////////
// window for prototypes and owners
////////////////////////////////////////////////////////////////

static void
display_prototypes_owners_window(Am_Object &main_inspector_window,
                                 Am_Object &for_obj, Am_Object &string_obj)
{
  Am_Object pop_up_window, string_object;
  Am_Object temp;
  Am_Value_List group_iter;
  pop_up_window = get_pop_up_window(
      main_inspector_window, Am_PROTOTYPES_OWNERS_WINDOW, temp, group_iter);
  int top = 0;
  int left = 0;
  int max_left = 0;

  char line[250];
  OSTRSTREAM_CONSTR(oss, line, 250, std::ios::out);
  reset_ostrstream(oss); // go to the beginning
  // title
  add_string(group_iter, pop_up_window, top, left, max_left, "Prototypes for ",
             false, Am_Red);
  add_object(group_iter, pop_up_window, top, left, max_left, for_obj, false,
             Am_Red);
  add_string(group_iter, pop_up_window, top, left, max_left, ":", true, Am_Red);
  Am_Object proto = for_obj.Get_Prototype();
  while (proto.Valid()) {
    left = 28;
    add_object(group_iter, pop_up_window, top, left, max_left, proto, true,
               Am_Black);
    if (proto == Am_Root_Object)
      break;
    proto = proto.Get_Prototype();
  }
  // title
  add_string(group_iter, pop_up_window, top, left, max_left, "Owners for ",
             false, Am_Red);
  add_object(group_iter, pop_up_window, top, left, max_left, for_obj, false,
             Am_Red);
  add_string(group_iter, pop_up_window, top, left, max_left, ":", true, Am_Red);
  Am_Object owner = for_obj.Get_Owner();
  while (owner.Valid()) {
    left = 28;
    add_object(group_iter, pop_up_window, top, left, max_left, owner, true,
               Am_Black);
    owner = owner.Get_Owner();
  }
  //done, now set up window
  reset_ostrstream(oss); // go to the beginning
  oss << "Prototypes & Owners for " << for_obj << std::ends;
  OSTRSTREAM_COPY(oss, line, 250);
  display_pop_up_window(pop_up_window, line, max_left, top,
                        main_inspector_window, string_obj);
}

////////////////////////////////////////////////////////////////
// Use slot window
////////////////////////////////////////////////////////////////

static void
display_slot_dependencies_window(Am_Object &main_inspector_window,
                                 Am_Slot_Key key, Am_Object &string_obj)
{
  Am_Object pop_up_window, for_obj, string_object;
  Am_Value_List group_iter;
  pop_up_window = get_pop_up_window(main_inspector_window, Am_USE_SLOT_WINDOW,
                                    for_obj, group_iter);
  int top = 0;
  int left = 0;
  int max_left = 0;

  char line[250];
  OSTRSTREAM_CONSTR(oss, line, 250, std::ios::out);
  reset_ostrstream(oss); // go to the beginning
  // title
  add_string(group_iter, pop_up_window, top, left, max_left, "Slot  ", false,
             Am_Red);
  add_slot_name(key, line, oss, 250, group_iter, pop_up_window, top, left,
                max_left, Am_Red);
  add_string(group_iter, pop_up_window, top, left, max_left, " in  ", false,
             Am_Red);
  add_object(group_iter, pop_up_window, top, left, max_left, for_obj, false,
             Am_Red);
  add_string(group_iter, pop_up_window, top, left, max_left, " is used by:",
             true, Am_Red);

  Am_Object_Advanced obj_adv = (Am_Object_Advanced &)for_obj;
  Am_Slot slot = obj_adv.Get_Slot_Locale(key).Get_Slot(key);
  Am_Dependency_Iterator dep_iter = slot;
  Am_Constraint *dep_constr;
  const char *constr_name;
  Am_Object dep_obj;
  Am_Object_Advanced dep_obj_adv;
  Am_Slot_Key dep_key;
  Am_Formula_Advanced *dep_form;
  Am_Slot in_slot;
  for (dep_iter.Start(); !dep_iter.Last(); dep_iter.Next()) {
    dep_constr = dep_iter.Get();
    reset_ostrstream(oss); // go to the beginning
    oss << "  Constraint ";
    constr_name = dep_constr->Get_Name();
    if (constr_name)
      oss << constr_name << " ";
    else
      oss << std::hex << dep_constr << std::dec;
    oss << std::ends;
    OSTRSTREAM_COPY(oss, line, 250);

    string_object = add_string(group_iter, pop_up_window, top, left, max_left,
                               line, false, Am_Black);
    if (Am_Formula_Advanced::Test(dep_constr)) {
      dep_form = Am_Formula_Advanced::Narrow(dep_constr);
      in_slot = dep_form->Get_Context();
      dep_key = in_slot.Get_Key();
      dep_obj_adv = in_slot.Get_Owner();
      dep_obj = dep_obj_adv;
      reset_ostrstream(oss); // go to the beginning
      add_slot_and_obj(dep_key, dep_obj, group_iter, pop_up_window, top, left,
                       max_left, line, oss, true, true);
    } else
      do_new_line(top, left, max_left, string_object, 0);
  }

  //done, now set up window
  reset_ostrstream(oss); // go to the beginning
  oss << "Uses of ";
  oss_slot_name_or_num(key, oss, true);
  OSTRSTREAM_COPY(oss, line, 250);
  display_pop_up_window(pop_up_window, line, max_left, top,
                        main_inspector_window, string_obj);
}

////////////////////////////////////////////////////////////////
// Slot properties window
////////////////////////////////////////////////////////////////
static Am_Object
get_inherited_from_obj(Am_Object &obj, Am_Slot_Key key)
{
  while (true) {
    if (!obj.Valid())
      return Am_No_Object;
    if (!obj.Is_Slot_Inherited(key))
      return obj;
    obj = obj.Get_Prototype();
  }
}

static void
display_slot_props_window(Am_Object &main_inspector_window, Am_Slot_Key key,
                          Am_Object &string_obj)
{
  Am_Object slot_props_window, for_obj, string_object;
  Am_Value_List group_iter;
  slot_props_window = get_pop_up_window(
      main_inspector_window, Am_SLOT_PROPS_WINDOW, for_obj, group_iter);

  int top = 0;
  int left = 0;
  int max_left = 0;

  char line[250];
  OSTRSTREAM_CONSTR(oss, line, 250, std::ios::out);
  reset_ostrstream(oss); // go to the beginning
  // title
  add_string(group_iter, slot_props_window, top, left, max_left,
             "Properties of the  ", false, Am_Red);
  add_slot_name(key, line, oss, 250, group_iter, slot_props_window, top, left,
                max_left, Am_Red);
  add_string(group_iter, slot_props_window, top, left, max_left, " slot of  ",
             false, Am_Red);
  add_object(group_iter, slot_props_window, top, left, max_left, for_obj, true,
             Am_Red);

  Am_Object_Advanced obj_adv = (Am_Object_Advanced &)for_obj;
  Am_Slot slot = obj_adv.Get_Slot_Locale(key).Get_Slot(key);

  //slot address
  add_string(group_iter, slot_props_window, top, left, max_left,
             "Slot memory address : ", false, Am_Black);
  Am_Slot_Data *slot_data = slot;
  reset_ostrstream(oss); // go to the beginning
  oss << std::hex << (void *)slot_data << std::dec << std::ends;
  OSTRSTREAM_COPY(oss, line, 250);

  add_string(group_iter, slot_props_window, top, left, max_left, line, true,
             Am_Black);

  //current value type
  Am_Value value;
  value = for_obj.Peek(key);
  add_string(group_iter, slot_props_window, top, left, max_left,
             "Current value type : ", false, Am_Black);
  const char *s = Am_Get_Type_Name(value.type);
  add_string(group_iter, slot_props_window, top, left, max_left, (char *)s,
             true, Am_Black);

  //where inherited from
  add_string(group_iter, slot_props_window, top, left, max_left,
             "Inherited from : ", false, Am_Black);
  if (for_obj.Is_Slot_Inherited(key)) {
    Am_Object inherited_from_obj;
    inherited_from_obj = get_inherited_from_obj(for_obj, key);
    add_object(group_iter, slot_props_window, top, left, max_left,
               inherited_from_obj, true, Am_Black);
  } else
    add_string(group_iter, slot_props_window, top, left, max_left,
               "(NOT INHERITED)", true, Am_Black);

  //slot declared type
  unsigned short type_check = slot.Get_Type_Check();
  add_string(group_iter, slot_props_window, top, left, max_left,
             "Declared type : ", false, Am_Black);
  reset_ostrstream(oss); // go to the beginning
  oss << type_check << std::ends;
  OSTRSTREAM_COPY(oss, line, 250);

  add_string(group_iter, slot_props_window, top, left, max_left, line, true,
             Am_Black);

  //slot inheritance form
  Am_Inherit_Rule rule = slot.Get_Inherit_Rule();
  add_string(group_iter, slot_props_window, top, left, max_left,
             "Inheritance Rule : ", false, Am_Black);
  switch (rule) {
  case Am_LOCAL:
    s = "LOCAL";
    break;
  case Am_INHERIT:
    s = "INHERIT";
    break;
  case Am_COPY:
    s = "COPY";
    break;
  case Am_STATIC:
    s = "STATIC";
    break;
  default:
    Am_Error("bad slot inheritance rule");
  }
  add_string(group_iter, slot_props_window, top, left, max_left, (char *)s,
             true, Am_Black);

  //constraint inheritance mode
  //bool constraint_mode = slot.Get_Single_Constraint_Mode();
  //add_string(group_iter, slot_props_window, top, left, max_left,
  //	     "Single Constraint Mode : ", false, Am_Black);
  //if (constraint_mode) s = "true";
  //else s = "false";
  //add_string(group_iter, slot_props_window, top, left, max_left,
  //(char*)s, true, Am_Black);

  // read only
  bool read_only = slot.Get_Read_Only();
  add_string(group_iter, slot_props_window, top, left, max_left, "Read Only : ",
             false, Am_Black);
  if (read_only)
    s = "true";
  else
    s = "false";
  add_string(group_iter, slot_props_window, top, left, max_left, (char *)s,
             true, Am_Black);

  //demon bits
  unsigned short demon_bits = slot.Get_Demon_Bits();
  add_string(group_iter, slot_props_window, top, left, max_left,
             "Demon bits : ", false, Am_Black);
  reset_ostrstream(oss); // go to the beginning
  oss << std::hex << (int)demon_bits << std::dec << std::ends;
  OSTRSTREAM_COPY(oss, line, 250);

  add_string(group_iter, slot_props_window, top, left, max_left, line, true,
             Am_Black);

  //done, now set up window
  reset_ostrstream(oss); // go to the beginning
  oss_slot_name_or_num(key, oss, false);
  oss << " of " << for_obj << std::ends;
  OSTRSTREAM_COPY(oss, line, 250);

  display_pop_up_window(slot_props_window, line, max_left, top,
                        main_inspector_window, string_obj);
}

//////////////////////////////////////////////////////////////////////////////
// refresh all pop-ups, if necessary
//////////////////////////////////////////////////////////////////////////////

//if selected a slot or constraint, and the pop-up windows are visible, move
//them
static void
check_move_props_windows(Am_Object &main_inspector_window,
                         Am_Object &string_obj)
{
  if (!string_obj.Valid())
    return;
  Am_Slot_Key key = (int)string_obj.Get(Am_INSPECTING_SLOT);
  Am_Object pop_up_window;

  if ((bool)string_obj.Get(Am_INSPECTING_SLOT_KEY)) { // a slot
    pop_up_window = main_inspector_window.Get(Am_SLOT_PROPS_WINDOW);
    if (pop_up_window.Valid()) // then displaying props for a slot
      display_slot_props_window(main_inspector_window, key, string_obj);
    pop_up_window = main_inspector_window.Get(Am_USE_SLOT_WINDOW);
    if (pop_up_window.Valid()) // then uses of the slot
      display_slot_dependencies_window(main_inspector_window, key, string_obj);
  }

  pop_up_window = main_inspector_window.Get(Am_CONSTRAINT_DEP_WINDOW);
  if (pop_up_window.Valid()) { // then displaying dependencies for a const
    Am_Constraint *constr =
        (Am_Constraint *)(Am_Ptr)(string_obj.Get(Am_INSPECTING_CONSTRAINT));
    if (constr)
      display_constraint_dependencies_window(main_inspector_window, constr, key,
                                             string_obj);
  }
  pop_up_window = main_inspector_window.Get(Am_PROTOTYPES_OWNERS_WINDOW);
  if (pop_up_window.Valid()) { // then displaying protos and owners
    Am_Object for_obj;
    for_obj = string_obj.Get(Am_INSPECTING_OBJECT);
    if (for_obj.Valid())
      display_prototypes_owners_window(main_inspector_window, for_obj,
                                       string_obj);
  }
}

#ifdef INSPECTOR_USE_WIDGETS

//////////////////////////////////////////////////////////////////////////////
// utilities for the command methods for the menubar
//////////////////////////////////////////////////////////////////////////////

static Am_Object
get_window_from_cmd(const Am_Object &command_obj)
{
  Am_Object inter, widget, window, for_item;
  inter = command_obj.Get_Owner();
  if (inter.Valid()) {
    widget = inter.Get_Owner();
    if (widget.Valid()) {
      window = widget.Get(Am_WINDOW);
      if (window.Valid()) {
        for_item = window.Get(Am_FOR_ITEM);
        if (for_item.Valid()) {
          window = for_item.Get(Am_WINDOW);
          return window;
        }
      }
    }
  }
  return Am_No_Object;
}

// returns the selected object if the user has selected one,
// or the object being inspected otherwise.
static Am_Object
get_selected_obj_from_win(const Am_Object &window)
{
  Am_Object selected_string, selected_obj;
  if (window.Valid()) {
    selected_string = window.Get(Am_SELECTED_ANY);
    if (selected_string.Valid()) {
      selected_obj = selected_string.Get(Am_INSPECTING_OBJECT);
      return selected_obj;
    } else
      return window.Get(Am_INSPECTING_OBJECT);
  }
  return Am_No_Object;
}

static Am_Object
get_selected_obj(const Am_Object &command_obj)
{
  Am_Object window;
  window = get_window_from_cmd(command_obj);
  return get_selected_obj_from_win(window);
}

static Am_Object
gv_window_from_cmd(const Am_Object &command_obj)
{
  Am_Object inter, widget, window, for_item;
  inter = command_obj.Get_Owner();
  if (inter.Valid()) {
    widget = inter.Get_Owner();
    if (widget.Valid()) {
      window = widget.Get(Am_WINDOW); //will be Sub_Menu_Window
      if (window.Valid()) {
        for_item = window.Get(Am_FOR_ITEM);
        if (for_item.Valid()) {
          window = for_item.Get(Am_WINDOW);
          return window;
        }
      }
    }
  }
  return Am_No_Object;
}

static Am_Object
gv_selected_string_obj(const Am_Object &command_obj)
{
  Am_Object window, selected_string, selected_obj;
  window = gv_window_from_cmd(command_obj);
  if (window.Valid()) {
    selected_string = window.Get(Am_SELECTED_ANY);
    return selected_string;
  }
  return Am_No_Object;
}

static Am_Object
gv_selected_obj(const Am_Object &command_obj)
{
  Am_Object selected_string, selected_obj;
  selected_string = gv_selected_string_obj(command_obj);
  if (selected_string.Valid()) {
    selected_obj = selected_string.Get(Am_INSPECTING_OBJECT);
    return selected_obj;
  }
  return Am_No_Object;
}

// *** eventually, display in comment line
static void
inspector_error(const Am_Object &window, const char *s)
{
  Am_Beep(window);
  std::cerr << s << std::endl << std::flush;
}

static void
show_previous_object(Am_Object &window)
{
  Am_Value_List obj_list;
  obj_list = window.Get(Am_INSPECT_OBJ_LIST);
  obj_list.Start();
  if (obj_list.Empty())
    inspector_error(window, "** No current object");
  else {
    // ** use destructive modification?? ***
    obj_list.Delete(); //remove the first object, which is one currently viewed
    obj_list.Start();
    if (obj_list.Empty())
      inspector_error(window, "** No previous object");
    else {
      Am_Object last_object = (Am_Object)obj_list.Get();
      obj_list.Delete(); //remove that object since
                         //set_object_into_window will add it again
      window.Set(Am_INSPECT_OBJ_LIST, obj_list);
      if (last_object.Valid())
        set_object_into_window(window, last_object);
      else
        inspector_error(window, "** Previous object is empty");
    }
  }
}

Am_Define_Method(Am_Object_Method, void, kill_window, (Am_Object window))
{
  Am_Object pop_up_window;
  pop_up_window = window.Get(Am_SLOT_PROPS_WINDOW);
  if (pop_up_window.Valid()) {
    pop_up_window.Destroy();
  }
  pop_up_window = window.Get(Am_CONSTRAINT_DEP_WINDOW);
  if (pop_up_window.Valid()) {
    pop_up_window.Destroy();
  }
  pop_up_window = window.Get(Am_USE_SLOT_WINDOW);
  if (pop_up_window.Valid()) {
    pop_up_window.Destroy();
  }
  pop_up_window = window.Get(Am_PROTOTYPES_OWNERS_WINDOW);
  if (pop_up_window.Valid()) {
    pop_up_window.Destroy();
  }
  Am_All_Inspector_Windows.Start();
  Am_All_Inspector_Windows.Member(window);
  if (!Am_All_Inspector_Windows.Last())
    Am_All_Inspector_Windows.Delete();
  window.Destroy();
}

//forward declaration since recursive call
static Am_Object create_inspector_window(const Am_Object &for_object, int left,
                                         int top, Am_Slot_Key slot_to_select);

static void
Inspector_NIY(const Am_Object &command_obj)
{
  Am_Object window;
  window = get_window_from_cmd(command_obj);
  if (window.Valid())
    inspector_error(window, "** Sorry, not implemented yet");
  else
    Am_Error("window not valid");
}

//////////////////////////////////////////////////////////////////////////////
// All the command methods for the menubar
//////////////////////////////////////////////////////////////////////////////

Am_Define_Method(Am_Object_Method, void, refresh_display,
                 (Am_Object command_obj))
{
  Am_Object window, object;
  window = get_window_from_cmd(command_obj);
  object = window.Get(Am_INSPECTING_OBJECT);
  refresh_display_of_object(window, object);
}

Am_Define_Method(Am_Object_Method, void, inspect_obj_this_win,
                 (Am_Object command_obj))
{
  Am_Object window, selected_obj;
  window = get_window_from_cmd(command_obj);
  if (window.Valid()) {
    selected_obj = get_selected_obj_from_win(window);
    if (selected_obj.Valid())
      set_object_into_window(window, selected_obj);
    else
      inspector_error(window, "** No object selected");
  } else
    Am_Error("window not valid");
}

Am_Define_Method(Am_Object_Method, void, inspect_obj_new_win,
                 (Am_Object command_obj))
{
  Am_Object window, selected_obj;
  window = get_window_from_cmd(command_obj);
  if (window.Valid()) {
    selected_obj = get_selected_obj_from_win(window);
    if (selected_obj.Valid()) {
      int left = window.Get(Am_LEFT);
      int top = window.Get(Am_TOP);
      create_inspector_window(selected_obj, left + 50, top + 50, Am_NO_SLOT);
    } else
      inspector_error(window, "** No object selected");
  } else
    Am_Error("window not valid");
}

Am_Define_Method(Am_Object_Method, void, inspect_protos_and_owners,
                 (Am_Object command_obj))
{
  Am_Object window, selected_string_obj, for_obj;
  window = get_window_from_cmd(command_obj);
  if (window.Valid()) {
    selected_string_obj = window.Get(Am_SELECTED_ANY);
    // this is a hack: inspect the protos/owners for the object in the
    // window if no object is user-selected.
    // should: redo the popup window stuff so it takes an object instead
    // of a string object.
    if (!selected_string_obj.Valid())
      selected_string_obj = window.Get(Am_VALUE_STRING_OBJECT);
    if (selected_string_obj.Valid()) {
      for_obj = selected_string_obj.Get(Am_INSPECTING_OBJECT);
      if (for_obj.Valid()) {
        display_prototypes_owners_window(window, for_obj, selected_string_obj);
      }
    }
  }
}

Am_Define_Formula(bool, dialog_entry_is_valid)
{
  Am_Object window;
  Am_String the_entry;
  the_entry = self.Get(Am_VALUE);
  const Am_Registered_Type *item = Am_Get_Named_Item(the_entry);
  return item != (0L);
}

Am_Define_Formula(bool, dialog_entry_is_valid_inter)
{
  Am_Object window;
  Am_String the_entry;
  the_entry = self.Get(Am_VALUE);
  Am_Object the_object = self.From_String(the_entry);
  return (the_object.Valid() && the_object.Is_Instance_Of(Am_Interactor));
}

static void
initialize_ask_obj_dialog()
{
  Am_Ask_Object_Dialog =
      Am_Text_Input_Dialog.Create("Am_Inspect_By_Name")
          .Set(Am_ITEMS, Am_Value_List().Add("Name of object to inspect:"))
          .Set(Am_VALID_INPUT, dialog_entry_is_valid)
          .Set(Am_LEFT, 100)
          .Set(Am_TOP, 100);
  Am_Ask_Inter_Dialog =
      Am_Text_Input_Dialog.Create("Am_Inter_Trace_By_Name")
          .Set(Am_ITEMS, Am_Value_List().Add("Name of interactor to trace:"))
          .Set(Am_VALID_INPUT, dialog_entry_is_valid_inter)
          .Set(Am_LEFT, 100)
          .Set(Am_TOP, 100);
}

// prompt for object name in dialog, and inspect object
Am_Define_Method(Am_Object_Method, void, inspect_obj_named,
                 (Am_Object command_obj))
{
  Am_Object window;
  window = get_window_from_cmd(command_obj);
  if (!Am_Ask_Object_Dialog.Valid())
    initialize_ask_obj_dialog();
  Am_Ask_Object_Dialog.Set(Am_MAIN_INSPECTOR_WINDOW, window,
                           Am_OK_IF_NOT_THERE);

  Am_Object text_widget = Am_Ask_Object_Dialog.Get_Object(Am_DIALOG_GROUP)
                              .Get_Object(Am_TEXT_WIDGET);
  Am_Start_Widget(text_widget);

  Am_String object_name = Am_Show_Dialog_And_Wait(Am_Ask_Object_Dialog);
  if (!object_name.Valid())
    return; // hit cancel

  Am_Object obj = command_obj.From_String(object_name);
  if (obj.Valid()) {
    if (window.Valid() && window.Get_Owner().Valid())
      set_object_into_window(window, obj);
    else
      Am_Inspect(obj);
  } else
    std::cerr << "** That's not a valid object name.\n";
}

// if a constraint is selected, show its dependencies
Am_Define_Method(Am_Object_Method, void, inspect_constraints,
                 (Am_Object command_obj))
{
  Am_Object window, selected_string_obj;
  window = get_window_from_cmd(command_obj);
  if (window.Valid()) {
    selected_string_obj = window.Get(Am_SELECTED_ANY);
    if (selected_string_obj.Valid()) {
      Am_Constraint *constr = (Am_Constraint *)(Am_Ptr)(
          selected_string_obj.Get(Am_INSPECTING_CONSTRAINT));
      if (constr) {
        Am_Slot_Key key = (int)selected_string_obj.Get(Am_INSPECTING_SLOT);
        display_constraint_dependencies_window(window, constr, key,
                                               selected_string_obj);
      }
    }
  }
}

Am_Define_Method(Am_Object_Method, void, inspect_previous,
                 (Am_Object command_obj))
{
  Am_Object window = get_window_from_cmd(command_obj);
  if (window.Valid())
    show_previous_object(window);
  else
    Am_Error("window not valid");
}

Am_Define_Method(Am_Object_Method, void, flash_object, (Am_Object command_obj))
{
  Am_Object window, selected_obj;
  window = get_window_from_cmd(command_obj);
  if (window.Valid()) {
    selected_obj = get_selected_obj_from_win(window);
    if (selected_obj.Valid())
      Am_Flash(selected_obj); // *** SHOULD MAKE OUTPUT GO TO comment line
    else
      inspector_error(window, "** No object selected");
  } else
    Am_Error("window not valid");
}

Am_Define_Method(Am_Object_Method, void, done_this, (Am_Object command_obj))
{
  Am_Object window = get_window_from_cmd(command_obj);
  if (window.Valid())
    kill_window_proc(window);
  else
    Am_Error("window not valid");
}

// ** remove all the inspector windows
Am_Define_Method(Am_Object_Method, void, done_all,
                 (Am_Object /* command_obj */))
{
  for (Am_All_Inspector_Windows.Start(); !Am_All_Inspector_Windows.Last();
       Am_All_Inspector_Windows.Next())
    kill_window_proc(Am_All_Inspector_Windows.Get());
}

Am_Define_Method(Am_Object_Method, void, quit_all,
                 (Am_Object /* command_obj */))
{
  // *** SHOULD PROBABLY CONFIRM THIS
  Am_Exit_Main_Event_Loop();
}

// ** toggles whether the inherited slots are displayed or not
Am_Define_Method(Am_Object_Method, void, hide_inherited_slots,
                 (Am_Object command_obj))
{
  Am_String cmdstr = command_obj.Get(Am_LABEL);
  char *label = cmdstr;

  Am_Object window, object;
  window = get_window_from_cmd(command_obj);
  if (!strcmp(label, Am_HIDE_INHERITED_TEXT)) { // then turn on
    window.Set(Am_HIDE_INHERITED, true);
    command_obj.Set(Am_LABEL, Am_SHOW_INHERITED_TEXT);
  } else {
    window.Set(Am_HIDE_INHERITED, false);
    command_obj.Set(Am_LABEL, Am_HIDE_INHERITED_TEXT);
  }
  object = window.Get(Am_INSPECTING_OBJECT);
  refresh_display_of_object(window, object);
}

// ** toggles whether the internal slots are displayed or not (those with a "~"
// at the beginning of their names).  The actually work is done when the
// object is added to the window.  This takes care of updating state and interface.
Am_Define_Method(Am_Object_Method, void, hide_internal_slots,
                 (Am_Object command_obj))
{
  Am_String cmdstr = command_obj.Get(Am_LABEL);
  char *label = cmdstr;

  Am_Object window, object;
  window = get_window_from_cmd(command_obj);
  if (!strcmp(label, Am_HIDE_INTERNAL_TEXT)) { // then turn on
    window.Set(Am_HIDE_INTERNAL, true);
    command_obj.Set(Am_LABEL, Am_SHOW_INTERNAL_TEXT);
  } else {
    window.Set(Am_HIDE_INTERNAL, false);
    command_obj.Set(Am_LABEL, Am_HIDE_INTERNAL_TEXT);
  }
  object = window.Get(Am_INSPECTING_OBJECT);
  refresh_display_of_object(window, object);
}

// ** prompts for a slot name in the comment line and puts it first
// commented out of menu
Am_Define_Method(Am_Object_Method, void, find_slot, (Am_Object command_obj))
{
  // ** NIY
  Inspector_NIY(command_obj);
}

// prompts for a slot name in the comment line and adds that as a new slot
// if it doesn't exist
// commented out of menu
Am_Define_Method(Am_Object_Method, void, add_new_slot, (Am_Object command_obj))
{
  // ** NIY
  Inspector_NIY(command_obj);
}

Am_Define_Method(Am_Object_Method, void, remove_slot, (Am_Object command_obj))
{
  Am_Object window, object, sel;
  Am_Slot_Key slot;
  window = get_window_from_cmd(command_obj);
  object = window.Get(Am_INSPECTING_OBJECT);
  sel = window.Get(Am_SELECTED_ANY);
  if (!sel.Valid())
    return;
  slot = (int)sel.Get(Am_INSPECTING_SLOT);
  if (!slot)
    return;
  object.Remove_Slot(slot);
  refresh_display_of_object(window, object);
}

// ** toggles the display of the instances of the object
Am_Define_Method(Am_Object_Method, void, show_instances,
                 (Am_Object command_obj))
{
  Am_String cmdstr = command_obj.Get(Am_LABEL);
  char *label = cmdstr;

  Am_Object window, object;
  window = get_window_from_cmd(command_obj);
  if (!strcmp(label, Am_SHOW_INSTANCES_TEXT)) { // then turn on
    window.Set(Am_SHOW_INSTANCES, true);
    command_obj.Set(Am_LABEL, Am_HIDE_INSTANCES_TEXT);
  } else {
    window.Set(Am_SHOW_INSTANCES, false);
    command_obj.Set(Am_LABEL, Am_SHOW_INSTANCES_TEXT);
  }
  object = window.Get(Am_INSPECTING_OBJECT);
  refresh_display_of_object(window, object);
}

// ** toggles the display of the instances of the object
Am_Define_Method(Am_Object_Method, void, show_parts, (Am_Object command_obj))
{
  Am_String cmdstr = command_obj.Get(Am_LABEL);
  char *label = cmdstr;

  Am_Object window, object;
  window = get_window_from_cmd(command_obj);
  if (!strcmp(label, Am_SHOW_PARTS_TEXT)) { // then turn on
    window.Set(Am_SHOW_PARTS, true);
    command_obj.Set(Am_LABEL, Am_HIDE_PARTS_TEXT);
  } else {
    window.Set(Am_SHOW_PARTS, false);
    command_obj.Set(Am_LABEL, Am_SHOW_PARTS_TEXT);
  }
  object = window.Get(Am_INSPECTING_OBJECT);
  refresh_display_of_object(window, object);
}

// ** toggles the display of the pop-up window that shows the properties of the
//slot
Am_Define_Method(Am_Object_Method, void, show_slot_properties,
                 (Am_Object command_obj))
{
  Am_Object window, selected_string_obj;
  window = get_window_from_cmd(command_obj);
  if (window.Valid()) {
    selected_string_obj = window.Get(Am_SELECTED_ANY);
    if (selected_string_obj.Valid()) {
      Am_Slot_Key key = (int)selected_string_obj.Get(Am_INSPECTING_SLOT);
      display_slot_props_window(window, key, selected_string_obj);
    }
  }
}

// ** toggles the display of the pop-up window that shows the uses of the slot
Am_Define_Method(Am_Object_Method, void, show_slot_uses,
                 (Am_Object command_obj))
{
  Am_Object window, selected_string_obj;
  window = get_window_from_cmd(command_obj);
  if (window.Valid()) {
    selected_string_obj = window.Get(Am_SELECTED_ANY);
    if (selected_string_obj.Valid()) {
      Am_Slot_Key key = (int)selected_string_obj.Get(Am_INSPECTING_SLOT);
      display_slot_dependencies_window(window, key, selected_string_obj);
    }
  }
}

// ** toggles whether the inspector window's display is updated immediately
// each time any value changes, vs. only when refresh is selected from the
// menu.  Note: to get any real optimization by turning this off, we have
// one global variable for automatic refresh of all windows, instead of
// one per window.
Am_Define_Method(Am_Object_Method, void, update_when_values_change,
                 (Am_Object command_obj))
{
  Am_String cmdstr = command_obj.Get(Am_LABEL);
  char *label = cmdstr;

  Am_Object window, object;
  window = get_window_from_cmd(command_obj);
  if (!strcmp(label, Am_AUTOMATIC_REFRESH_TEXT)) { // then turn on
    Am_Automatic_Refresh = true;
    command_obj.Set(Am_LABEL, Am_MANUAL_REFRESH_TEXT);
  } else {
    Am_Automatic_Refresh = false;
    command_obj.Set(Am_LABEL, Am_AUTOMATIC_REFRESH_TEXT);
  }
}

// ** if a slot is selected, starts showing its old values
Am_Define_Method(Am_Object_Method, void, show_old_slot_values,
                 (Am_Object command_obj))
{
  Am_Object window, object, string_object;
  window = get_window_from_cmd(command_obj);
  if (window.Valid()) {
    object = window.Get(Am_INSPECTING_OBJECT);
    if (object.Valid()) {
      string_object = window.Get(Am_SELECTED_ANY);
      if (string_object.Valid()) {
        Am_Slot_Key slot_key = (int)string_object.Get(Am_INSPECTING_SLOT);
        if (slot_key) {
          Am_Object value_string_obj;
          value_string_obj = string_object.Get(Am_VALUE_STRING_OBJECT);
          if (!value_string_obj.Valid() ||
              !value_string_obj.Get(Am_INSPECTING_VALUE).Valid()) {
            std::cerr << "** You can't show old values of a list.\n";
            return;
          }
          Am_Trace_Status tracing = Am_Get_Tracer_Status(object, slot_key);
          if (tracing == (Am_TRACING | Am_STORING_OLD)) {
            Am_Stop_Slot_Value_Tracing(object, slot_key);
            // clear out old value string objects
            Am_Add_Old_Values(Am_Value_List(), value_string_obj, (0L));
          } else {
            Am_Start_Slot_Value_Tracing(object, slot_key, value_string_obj);
            Am_Value_List old_values =
                Am_Get_Tracer_Old_Values(object, slot_key, value_string_obj);
            Am_Add_Old_Values(old_values, value_string_obj, (0L));
          }
          value_string_obj.Set(Am_TRACING_SLOT,
                               Am_Get_Tracer_Status(object, slot_key));
        } else {
          std::cerr << "** problem with slot key.\n";
          return;
        }
      } else {
        std::cerr << "** You must first select a slot to trace.\n";
        return;
      }
    }
  } else
    Am_Error("inspector window not valid");
}

// ** all the slots are displayed sorted by name
Am_Define_Method(Am_Object_Method, void, sort_by_name, (Am_Object command_obj))
{
  Am_String cmdstr = command_obj.Get(Am_LABEL);
  char *label = cmdstr;

  Am_Object window, object;
  window = get_window_from_cmd(command_obj);
  if (!strcmp(label, Am_SORT_BY_NAME_TEXT)) { // then turn on sorting
    window.Set(Am_SORT_BY_NAME, true);
    command_obj.Set(Am_LABEL, Am_STOP_SORTING_TEXT);
  } else {
    window.Set(Am_SORT_BY_NAME, false);
    command_obj.Set(Am_LABEL, Am_SORT_BY_NAME_TEXT);
  }
  object = window.Get(Am_INSPECTING_OBJECT);
  refresh_display_of_object(window, object);
}

// ** all the slots are displayed sorted by how recently set
// commented out of menu
Am_Define_Method(Am_Object_Method, void, sort_by_recently_set,
                 (Am_Object command_obj))
{
  // ** NIY
  Inspector_NIY(command_obj);
}

// ** break into the debugger now
Am_Define_Method(Am_Object_Method, void, break_now,
                 (Am_Object /* command_obj */))
{
  Am_Break_Into_Debugger();
}

// ** start tracing of setting of the selected slot
Am_Define_Method(Am_Object_Method, void, trace_when_this_slot_set,
                 (Am_Object command_obj))
{
  Am_Object window, object, string_object, value_string_obj;
  window = get_window_from_cmd(command_obj);
  if (window.Valid()) {
    object = window.Get(Am_INSPECTING_OBJECT);
    if (object.Valid()) {
      string_object = window.Get(Am_SELECTED_ANY);
      if (string_object.Valid()) {
        Am_Slot_Key slot_key = (int)string_object.Get(Am_INSPECTING_SLOT);
        if (slot_key) {
          Am_Trace_Status tracing = (int)string_object.Get(Am_TRACING_SLOT);
          value_string_obj = string_object.Get(Am_VALUE_STRING_OBJECT);
          if (tracing == Am_TRACING) {
            Am_Clear_Slot_Notify(object, slot_key);
            string_object.Set(Am_TRACING_SLOT, 0);
          } else {
            Am_Notify_On_Slot_Set(object, slot_key);
            tracing = Am_Update_Tracer_String_Object(object, slot_key,
                                                     value_string_obj);
            string_object.Set(Am_TRACING_SLOT, tracing);
          }
        } else {
          std::cerr << "** problem with slot key.\n";
          return;
        }
      } else {
        std::cerr << "** You must first select a slot to trace.\n";
        return;
      }
    }
  } else
    Am_Error("inspector window not valid");
}

// ** set up to break when the selected slot set
Am_Define_Method(Am_Object_Method, void, break_when_this_slot_set,
                 (Am_Object command_obj))
{
  Am_Object window, object, string_object, value_string_obj;
  window = get_window_from_cmd(command_obj);
  if (window.Valid()) {
    object = window.Get(Am_INSPECTING_OBJECT);
    if (object.Valid()) {
      string_object = window.Get(Am_SELECTED_ANY);
      if (string_object.Valid()) {
        Am_Slot_Key slot_key = (int)string_object.Get(Am_INSPECTING_SLOT);
        if (slot_key) {
          Am_Trace_Status tracing = (int)string_object.Get(Am_TRACING_SLOT);
          value_string_obj = string_object.Get(Am_VALUE_STRING_OBJECT);
          if (tracing & (Am_TRACING | Am_BREAKING)) {
            Am_Clear_Slot_Notify(object, slot_key);
            string_object.Set(Am_TRACING_SLOT, 0);
          } else {
            Am_Break_On_Slot_Set(object, slot_key);
            tracing = Am_Update_Tracer_String_Object(object, slot_key,
                                                     value_string_obj);
            string_object.Set(Am_TRACING_SLOT, tracing);
          }
        } else {
          std::cerr << "** problem with slot key.\n";
          return;
        }
      } else {
        std::cerr << "** You must first select a slot to break.\n";
        return;
      }
    }
  } else
    Am_Error("inspector window not valid");
}

void
object_break_trace(Am_Object &command_obj, Am_Set_Reason reason, bool do_break)
{
  Am_Object window, selected_obj;
  window = get_window_from_cmd(command_obj);
  if (window.Valid()) {
    selected_obj = get_selected_obj_from_win(window);
    if (!selected_obj.Valid())
      selected_obj = window.Get(Am_INSPECTING_OBJECT);
    if (selected_obj.Valid())
      Am_Notify_Or_Break_On_Object_Action(selected_obj, reason, do_break);
    else
      inspector_error(window, "** No object selected");
  } else
    Am_Error("window not valid");
}

Am_Define_Method(Am_Object_Method, void, trace_when_object_copied,
                 (Am_Object command_obj))
{
  object_break_trace(command_obj, Am_TRACE_OBJECT_COPIED, false);
}
Am_Define_Method(Am_Object_Method, void, trace_when_object_instanced,
                 (Am_Object command_obj))
{
  object_break_trace(command_obj, Am_TRACE_OBJECT_CREATED, false);
}
Am_Define_Method(Am_Object_Method, void, trace_when_object_destroyed,
                 (Am_Object command_obj))
{
  object_break_trace(command_obj, Am_TRACE_OBJECT_DESTROYED, false);
}
Am_Define_Method(Am_Object_Method, void, break_when_object_copied,
                 (Am_Object command_obj))
{
  object_break_trace(command_obj, Am_TRACE_OBJECT_COPIED, true);
}
Am_Define_Method(Am_Object_Method, void, break_when_object_instanced,
                 (Am_Object command_obj))
{
  object_break_trace(command_obj, Am_TRACE_OBJECT_CREATED, true);
}
Am_Define_Method(Am_Object_Method, void, break_when_object_destroyed,
                 (Am_Object command_obj))
{
  object_break_trace(command_obj, Am_TRACE_OBJECT_DESTROYED, true);
}

void
method_break_trace(Am_Object &command_obj, bool do_break)
{
  Am_Object window, object, string_object, value_string_obj;
  window = get_window_from_cmd(command_obj);
  if (window.Valid()) {
    object = window.Get(Am_INSPECTING_OBJECT);
    if (object.Valid()) {
      string_object = window.Get(Am_SELECTED_ANY);
      if (string_object.Valid()) {
        Am_Slot_Key slot_key = (int)string_object.Get(Am_INSPECTING_SLOT);
        if (slot_key) {
          //  ** Should figure out whether already tracing, and turn it off
          std::cerr << "Adding ";
          if (do_break)
            std::cerr << " break ";
          else
            std::cerr << " trace ";
          std::cerr << " when method in object " << object << " slot "
                    << Am_Get_Slot_Name(slot_key) << " is accessed.\n"
                    << std::flush;
          Am_Notify_Or_Break_On_Method(object, slot_key, do_break);
        } else {
          std::cerr << "** problem with slot key.\n";
          return;
        }
      } else {
        std::cerr << "** String object not valid.\n";
        return;
      }
    } else
      Am_Error("Object not valid");
  } else
    Am_Error("inspector window not valid");
}

// set up to trace when the selected method is called
Am_Define_Method(Am_Object_Method, void, trace_when_get_method,
                 (Am_Object command_obj))
{
  method_break_trace(command_obj, false);
}

// set up to break when the selected method is called
Am_Define_Method(Am_Object_Method, void, break_when_get_method,
                 (Am_Object command_obj))
{
  method_break_trace(command_obj, true);
}

// ** pops up a dialog box that allows breaking or tracing with more control
// commented out of menu
Am_Define_Method(Am_Object_Method, void, trace_break_more_choices,
                 (Am_Object command_obj))
{
  // ** NIY
  Inspector_NIY(command_obj);
}

Am_Define_Method(Am_Object_Method, void, trace_no_interactors,
                 (Am_Object /* command_obj */))
{
  Am_Clear_Inter_Trace();
}

// prompt for object name in dialog, and inspect object
Am_Define_Method(Am_Object_Method, void, trace_inter_named,
                 (Am_Object command_obj))
{
  Am_Object window;
  window = get_window_from_cmd(command_obj);
  if (!Am_Ask_Inter_Dialog.Valid())
    initialize_ask_obj_dialog(); // initializes both dialogs
  Am_Ask_Inter_Dialog.Set(Am_MAIN_INSPECTOR_WINDOW, window, Am_OK_IF_NOT_THERE);
  Am_Object text_widget = Am_Ask_Inter_Dialog.Get_Object(Am_DIALOG_GROUP)
                              .Get_Object(Am_TEXT_WIDGET);
  Am_Start_Widget(text_widget);

  Am_String inter_name = Am_Show_Dialog_And_Wait(Am_Ask_Inter_Dialog);
  if (!inter_name.Valid())
    return; // hit cancel
  Am_Object obj = command_obj.From_String(inter_name);
  if (obj.Valid()) {
    if (obj.Is_Instance_Of(Am_Interactor)) {
      // then we have a good interactor to trace
      Am_Set_Inter_Trace(obj);
    } else
      std::cerr << "** Object " << obj << " is not an Interactor\n";
  } else
    std::cerr << " ** Object " << inter_name << " is not valid\n";

  // bug: how do we turn it off without turning them all off?
}

Am_Define_Method(Am_Object_Method, void, trace_this_interactor,
                 (Am_Object command_obj))
{
  Am_Object selected_obj = get_selected_obj(command_obj);
  if (selected_obj.Valid())
    Am_Set_Inter_Trace(selected_obj);
}

Am_Define_Method(Am_Object_Method, void, trace_all_interactors,
                 (Am_Object /* command_obj */))
{
  Am_Set_Inter_Trace(Am_INTER_TRACE_ALL);
}

Am_Define_Method(Am_Object_Method, void, trace_next_interactor,
                 (Am_Object /* command_obj */))
{
  Am_Set_Inter_Trace(Am_INTER_TRACE_NEXT);
}

Am_Define_Method(Am_Object_Method, void, trace_input_events,
                 (Am_Object /* command_obj */))
{
  Am_Set_Inter_Trace(Am_INTER_TRACE_EVENTS);
}

Am_Define_Method(Am_Object_Method, void, trace_interactor_set_slots,
                 (Am_Object /* command_obj */))
{
  Am_Set_Inter_Trace(Am_INTER_TRACE_SETTING);
}

Am_Define_Method(Am_Object_Method, void, trace_interactor_priorities,
                 (Am_Object /* command_obj */))
{
  Am_Set_Inter_Trace(Am_INTER_TRACE_PRIORITIES);
}

Am_Define_Method(Am_Object_Method, void, short_trace_interactors,
                 (Am_Object /* command_obj */))
{
  Am_Set_Inter_Trace(Am_INTER_TRACE_SHORT);
}

Am_Define_Method(Am_Object_Method, void, pause_animations,
                 (Am_Object /* command_obj */))
{
  Am_Set_Timer_State(Am_TIMERS_SUSPENDED);
}

Am_Define_Method(Am_Object_Method, void, resume_animations,
                 (Am_Object /* command_obj */))
{
  Am_Set_Timer_State(Am_TIMERS_RUNNING);
}

Am_Define_Method(Am_Object_Method, void, single_step_animations,
                 (Am_Object /* command_obj */))
{
  Am_Set_Timer_State(Am_TIMERS_SINGLE_STEP);
}

////////////
// Constraints for menubar
////////////

Am_Define_Formula(bool, object_selected)
{
  Am_Object selected_obj = gv_selected_obj(self);
  return (selected_obj.Valid());
}
Am_Define_Formula(bool, inter_object_selected)
{
  Am_Object selected_obj = gv_selected_obj(self);
  if (selected_obj.Valid())
    if (selected_obj.Is_Instance_Of(Am_Interactor))
      return true;
  return false;
}

Am_Define_Formula(bool, constraint_selected)
{
  Am_Object selected_string_obj = gv_selected_string_obj(self);
  if (selected_string_obj.Valid()) {
    Am_Ptr constr = (Am_Ptr)(
        selected_string_obj.Get(Am_INSPECTING_CONSTRAINT, Am_NO_DEPENDENCY));
    if (constr)
      return true;
  }
  return false;
}

Am_Define_Formula(bool, is_there_previous)
{
  Am_Object window = gv_window_from_cmd(self);
  if (window.Valid()) {
    Am_Value_List obj_list;
    obj_list = window.Get(Am_INSPECT_OBJ_LIST);
    if (obj_list.Length() > 1)
      return true; //1 because current is in list
  }
  return false;
}

Am_Define_Formula(bool, slot_selected)
{
  Am_Object selected_string_obj = gv_selected_string_obj(self);
  if (selected_string_obj.Valid()) {
    return selected_string_obj.Get(Am_INSPECTING_SLOT_KEY, Am_NO_DEPENDENCY);
  }
  return false;
}

Am_Define_Formula(bool, method_slot_selected)
{
  Am_Object selected_string_obj = gv_selected_string_obj(self);
  if (selected_string_obj.Valid()) {
    if ((bool)selected_string_obj.Get(Am_INSPECTING_VALUE, Am_NO_DEPENDENCY)) {
      // then selected a value; see if a method value
      Am_Slot_Key key =
          (int)selected_string_obj.Get(Am_INSPECTING_SLOT, Am_NO_DEPENDENCY);
      Am_Object window = gv_window_from_cmd(self);
      Am_Object object_inspected = window.Get(Am_INSPECTING_OBJECT);
      Am_Value value = object_inspected.Peek(key);
      if (Am_Type_Class(value.type) == Am_METHOD)
        return true;
    } else { // see if selected a method slot name
      Am_Slot_Key key = (int)selected_string_obj.Get(Am_INSPECTING_SLOT_KEY,
                                                     Am_NO_DEPENDENCY);
      if (key) {
        Am_Object window = gv_window_from_cmd(self);
        Am_Object object_inspected = window.Get(Am_INSPECTING_OBJECT);
        Am_Value value = object_inspected.Peek(key);
        if (Am_Type_Class(value.type) == Am_METHOD)
          return true;
      }
    }
  }
  return false;
}

Am_Object Am_Inspector_Menu_Bar;

/* ******************* DON'T BOTHER UNTIL MENU HAS SOMETHING IN IT

#if defined(_WIN32) || defined(_MACINTOSH)
#define AMULET_BITMAP "lib/images/amside.gif"
#else
#define AMULET_BITMAP "lib/images/amuletside.xbm"
#endif

Am_Object Am_Amulet_Icon;

static void load_amulet_bitmap() {
  const char *filename = Am_Merge_Pathname(AMULET_BITMAP);
  Am_Image_Array amulet_icon_image (filename);
  delete [] filename;
  if (!amulet_icon_image.Valid())
    Am_Amulet_Icon = Am_Rectangle.Create()
      .Set(Am_WIDTH, 15)
      .Set(Am_HEIGHT, 9);
  else Am_Amulet_Icon = Am_Bitmap.Create("Amulet_Icon")
    .Set (Am_IMAGE, amulet_icon_image)
    .Set (Am_LINE_STYLE, Am_Black)
    ;
}

********************************** */

//////////////////////////////////////////////////////////////////////////////
// The Menubar
//////////////////////////////////////////////////////////////////////////////

static void
initialize_menu_bar()
{
  Am_Value_List l; //PC can't handle a statement that is this big, so
                   //break it up
  l = Am_Value_List()
          // .Add (Am_Command.Create("Amulet_Command")
          // .Set(Am_LABEL, Am_Amulet_Icon)
          // .Set(Am_ITEMS, Am_Value_List ()
          // .Add ("About Inspector...")
          // .Add ("About Amulet...")
          // )
          // )
          .Add(Am_Command.Create("Objects")
                   .Set(Am_LABEL, "Objects")
                   .Set(Am_ITEMS,
                        Am_Value_List()
                            .Add(Am_Command.Create("Inspect_Obj_This_Win")
                                     .Set(Am_LABEL, "Inspect Object")
                                     .Set(Am_DO_METHOD, inspect_obj_this_win)
                                     .Set(Am_ACTIVE, object_selected)
                                     .Set(Am_ACCELERATOR, "^i"))
                            .Add(Am_Command.Create("Inspect_Obj_New_Win")
                                     .Set(Am_LABEL, "Inspect in New Window")
                                     .Set(Am_DO_METHOD, inspect_obj_new_win)
                                     .Set(Am_ACTIVE, object_selected)
                                     .Set(Am_ACCELERATOR, "^I"))
                            .Add(Am_Command.Create("Inspect_Obj_Named")
                                     .Set(Am_LABEL, "Inspect Object Named...")
                                     .Set(Am_DO_METHOD, inspect_obj_named))
                            .Add(Am_Command.Create("Inspect_previous")
                                     .Set(Am_LABEL, "Inspect Previous")
                                     .Set(Am_DO_METHOD, inspect_previous)
                                     .Set(Am_ACTIVE, is_there_previous)
                                     .Set(Am_ACCELERATOR, "^p"))
                            .Add(Am_Menu_Line_Command.Create())
                            .Add(Am_Command.Create("Refresh")
                                     .Set(Am_LABEL, "Refresh display")
                                     .Set(Am_DO_METHOD, refresh_display))
                            .Add(Am_Command.Create("flash_object")
                                     .Set(Am_LABEL, "Flash Object")
                                     .Set(Am_DO_METHOD, flash_object)
                                     .Set(Am_ACCELERATOR, "^f"))
                            .Add(Am_Menu_Line_Command.Create())
                            .Add(Am_Command.Create("Done")
                                     .Set(Am_LABEL, "Done")
                                     .Set(Am_DO_METHOD, done_this)
                                     .Set(Am_ACCELERATOR, "^q"))
                            .Add(Am_Command.Create("Done_All")
                                     .Set(Am_LABEL, "Done All")
                                     .Set(Am_DO_METHOD, done_all))
                            .Add(Am_Menu_Line_Command.Create())
                            .Add(Am_Command.Create("quit")
                                     .Set(Am_LABEL, "Quit Application")
                                     .Set(Am_DO_METHOD, quit_all))))
          .Add(
              Am_Command.Create("Edit_Command")
                  .Set(Am_LABEL, "Edit")
                  .Set(
                      Am_ITEMS,
                      Am_Value_List().Add(Am_Command.Create("Remove Slot")
                                              .Set(Am_LABEL, "Remove Slot")
                                              .Set(Am_DO_METHOD, remove_slot)
                                              .Set(Am_ACTIVE, slot_selected))
                      // 		    .Add (Am_Command.Create("Find Slot")
                      // 			  .Set (Am_LABEL, "Find Slot...")
                      // 			  .Set (Am_DO_METHOD, find_slot)
                      // 			  )
                      // 		    .Add (Am_Command.Create("Add Slot")
                      // 			  .Set (Am_LABEL, "Add Slot...")
                      // 			  .Set (Am_DO_METHOD, add_new_slot)
                      // 			  )
                      // 		    .Add ("Undo")
                      // 		    .Add ("Cut")
                      // 		    .Add ("Copy")
                      // 		    .Add ("Paste")
                      ));

  l.Add(
      Am_Command.Create("View_Command")
          .Set(Am_LABEL, "View")
          .Set(
              Am_ITEMS,
              Am_Value_List()
                  .Add(Am_Command.Create(Am_HIDE_INHERITED_TEXT)
                           .Set(Am_LABEL, Am_HIDE_INHERITED_DEFAULT_TEXT)
                           .Set(Am_DO_METHOD, hide_inherited_slots))
                  .Add(Am_Command.Create(Am_HIDE_INTERNAL_TEXT)
                           .Set(Am_LABEL, Am_HIDE_INTERNAL_DEFAULT_TEXT)
                           .Set(Am_DO_METHOD, hide_internal_slots))
                  .Add(Am_Command.Create(Am_SHOW_PARTS_TEXT)
                           .Set(Am_LABEL, Am_SHOW_PARTS_DEFAULT_TEXT)
                           .Set(Am_DO_METHOD, show_parts))
                  .Add(Am_Command.Create(Am_SHOW_INSTANCES_TEXT)
                           .Set(Am_LABEL, Am_SHOW_INSTANCES_DEFAULT_TEXT)
                           .Set(Am_DO_METHOD, show_instances))
                  .Add(Am_Command.Create(Am_AUTOMATIC_REFRESH_TEXT)
                           .Set(Am_LABEL, Am_REFRESH_DEFAULT_TEXT)
                           .Set(Am_DO_METHOD,
                                update_when_values_change)) //vs freeze values
                  .Add(Am_Menu_Line_Command.Create())
                  .Add(Am_Command.Create("Show Old Slot Values")
                           .Set(Am_LABEL, "Show Old Slot Values")
                           .Set(Am_DO_METHOD, show_old_slot_values)
                           .Set(Am_ACTIVE, slot_selected))
                  .Add(Am_Menu_Line_Command.Create())
                  .Add(Am_Command.Create(Am_SORT_BY_NAME_TEXT)
                           .Set(Am_LABEL, Am_SORT_BY_NAME_DEFAULT_TEXT)
                           .Set(Am_DO_METHOD, sort_by_name))
              //		    .Add (Am_Command.Create("Sort by Recently Set")
              //			  .Set (Am_LABEL, "Sort by Recently Set")
              //			  .Set (Am_DO_METHOD, sort_by_recently_set)
              //			  )
              ));
  l.Add(Am_Command.Create("Windows_Command")
            .Set(Am_LABEL, "Windows")
            .Set(Am_ITEMS,
                 Am_Value_List()
                     .Add(Am_Command.Create("Inspect_Protos and Owners")
                              .Set(Am_LABEL, "Show Prototypes and Owners")
                              .Set(Am_DO_METHOD, inspect_protos_and_owners))
                     .Add(Am_Command.Create("Inspect_constraints")
                              .Set(Am_LABEL, "Show Constraint Dependencies")
                              .Set(Am_DO_METHOD, inspect_constraints)
                              .Set(Am_ACTIVE, constraint_selected))
                     .Add(Am_Command.Create("Show Slot Properties")
                              .Set(Am_LABEL, "Show Slot Properties")
                              .Set(Am_DO_METHOD, show_slot_properties)
                              .Set(Am_ACTIVE, slot_selected))
                     .Add(Am_Command.Create("Show Uses of This Slot")
                              .Set(Am_LABEL, "Show Slot Uses")
                              .Set(Am_DO_METHOD, show_slot_uses)
                              .Set(Am_ACTIVE, slot_selected))));
  l.Add(
      Am_Command.Create("Break_Command")
          .Set(Am_LABEL, "Break/Trace")
          .Set(
              Am_ITEMS,
              Am_Value_List()
                  .Add(Am_Command.Create("Break into C++ Debugger Now")
                           .Set(Am_LABEL, "Break into C++ Debugger Now")
                           .Set(Am_DO_METHOD, break_now))
                  .Add(Am_Menu_Line_Command.Create())
                  .Add(Am_Command.Create("Trace When This Slot Set")
                           .Set(Am_LABEL, "Trace When This Slot Set")
                           .Set(Am_DO_METHOD, trace_when_this_slot_set)
                           .Set(Am_ACTIVE, slot_selected))
                  .Add(Am_Command.Create("Break When This Slot Set")
                           .Set(Am_LABEL,
                                "Break into C++ Debugger When This Slot Set")
                           .Set(Am_DO_METHOD, break_when_this_slot_set)
                           .Set(Am_ACTIVE, slot_selected))
                  .Add(Am_Menu_Line_Command.Create())
                  .Add(Am_Command.Create("object copied")
                           .Set(Am_LABEL, "Trace When This Object Copied")
                           .Set(Am_DO_METHOD, trace_when_object_copied))
                  .Add(Am_Command.Create("object create")
                           .Set(Am_LABEL,
                                "Trace When Instance Created of this Object")
                           .Set(Am_DO_METHOD, trace_when_object_instanced))
                  .Add(Am_Command.Create("object destroy")
                           .Set(Am_LABEL, "Trace When this Object Destroyed")
                           .Set(Am_DO_METHOD, trace_when_object_destroyed))
                  .Add(Am_Menu_Line_Command.Create())
                  .Add(Am_Command.Create("break object copied")
                           .Set(Am_LABEL, "Break When This Object Copied")
                           .Set(Am_DO_METHOD, break_when_object_copied))
                  .Add(Am_Command.Create("break object create")
                           .Set(Am_LABEL,
                                "Break When Instance Created of this Object")
                           .Set(Am_DO_METHOD, break_when_object_instanced))
                  .Add(Am_Command.Create("break object destroy")
                           .Set(Am_LABEL, "Break When this Object Destroyed")
                           .Set(Am_DO_METHOD, break_when_object_destroyed))
                  .Add(Am_Menu_Line_Command.Create())
                  .Add(Am_Command.Create("Trace When Get Method in this Slot")
                           .Set(Am_LABEL, "Trace When Get Method in this Slot")
                           .Set(Am_DO_METHOD, trace_when_get_method)
                           .Set(Am_ACTIVE, method_slot_selected))
                  .Add(Am_Command.Create("Break When Get Method in this Slot")
                           .Set(Am_LABEL, "Break When Get Method in this Slot")
                           .Set(Am_DO_METHOD, break_when_get_method)
                           .Set(Am_ACTIVE, method_slot_selected))
              //		    .Add (Am_Menu_Line_Command.Create())
              // 		    .Add (Am_Command.Create("Trace/Break More Choices...")
              // 			  .Set (Am_LABEL, "Trace/Break More Choices...")
              // 			  .Set (Am_DO_METHOD, trace_break_more_choices)
              // 			   )
              //		    .Add (Am_Command.Create("Trace_Constraint")
              //			  .Set (Am_LABEL, "Trace Dependencies")
              //			  .Set (Am_DO_METHOD, (0L))
              //			  .Set (Am_ACTIVE, constraint_selected)
              //			  )
              //		    .Add (Am_Command.Create("Break_Constraint")
              //			  .Set (Am_LABEL, "Break When Dependencies Change")
              //			  .Set (Am_DO_METHOD, (0L))
              //			  .Set (Am_ACTIVE, constraint_selected)
              //			  )
              ));
  l.Add(Am_Command.Create("Interactors_Command")
            .Set(Am_LABEL, "Interactors")
            .Set(Am_ITEMS,
                 Am_Value_List()
                     .Add(Am_Command.Create("Trace No Interactors")
                              .Set(Am_LABEL, "Turn off Interactor Tracing")
                              .Set(Am_DO_METHOD, trace_no_interactors)
                              .Set(Am_ACCELERATOR, "^T"))
                     .Add(Am_Command.Create("Trace This Interactor")
                              .Set(Am_LABEL, "Trace This Interactor")
                              .Set(Am_DO_METHOD, trace_this_interactor)
                              .Set(Am_ACTIVE, inter_object_selected))
                     .Add(Am_Command.Create("Trace Interactor Named...")
                              .Set(Am_LABEL, "Trace Interactor Named...")
                              .Set(Am_DO_METHOD, trace_inter_named))
                     .Add(Am_Command.Create("Trace All Interactor")
                              .Set(Am_LABEL, "Trace All Interactors")
                              .Set(Am_DO_METHOD, trace_all_interactors)
                              .Set(Am_ACCELERATOR, "^t"))
                     .Add(Am_Command.Create("Trace Next Interactor")
                              .Set(Am_LABEL, "Trace Next Interactor To Run")
                              .Set(Am_DO_METHOD, trace_next_interactor))
                     .Add(Am_Command.Create("Trace Input Events")
                              .Set(Am_LABEL, "Trace Input Events")
                              .Set(Am_DO_METHOD, trace_input_events))
                     .Add(Am_Command.Create("Trace Interactor Set Slots")
                              .Set(Am_LABEL, "Trace Interactor Set Slots")
                              .Set(Am_DO_METHOD, trace_interactor_set_slots))
                     .Add(Am_Command.Create("Trace Interactor Priorities")
                              .Set(Am_LABEL, "Trace Interactor Priorities")
                              .Set(Am_DO_METHOD, trace_interactor_priorities))
                     .Add(Am_Command.Create("Short Trace Interactors")
                              .Set(Am_LABEL, "Short Trace Interactors")
                              .Set(Am_DO_METHOD, short_trace_interactors))));
  l.Add(
      Am_Command.Create("Animations_Command")
          .Set(Am_LABEL, "Animations")
          .Set(Am_ITEMS, Am_Value_List()
                             .Add(Am_Command.Create("Pause Animations")
                                      .Set(Am_LABEL, "Pause")
                                      .Set(Am_DO_METHOD, pause_animations)
                                      .Set(Am_ACCELERATOR, "^a"))
                             .Add(Am_Command.Create("Resume Animations")
                                      .Set(Am_LABEL, "Resume")
                                      .Set(Am_DO_METHOD, resume_animations)
                                      .Set(Am_ACCELERATOR, "^A"))
                             .Add(Am_Command.Create("Single Step Animations")
                                      .Set(Am_LABEL, "Single-step")
                                      .Set(Am_DO_METHOD, single_step_animations)
                                      .Set(Am_ACCELERATOR, "^s"))));
  Am_Inspector_Menu_Bar =
      Am_Menu_Bar.Create("Inspector_Menu_Bar").Set(Am_ITEMS, l);
}

#endif

static void
add_instr_to_window(Am_Object &window, const Am_Object &object)
{
  Am_Object obj;
  int top = 2;
  window.Set(Am_INSPECTING_OBJECT, object);

#ifdef INSPECTOR_USE_WIDGETS
  obj = Am_Inspector_Menu_Bar.Create();
  window.Add_Part(obj);
  top = (int)obj.Get(Am_HEIGHT) + 15;
#else

  obj = Am_Text.Create()
            .Set(Am_LEFT, 2)
            .Set(Am_TOP, top)
            .Set(Am_TEXT,
                 "Press ^q=quit. ^p=Previous Object. ^r=Refresh.  ^i=inter "
                 "debug. ^f=flash. Left click to edit int.");
  window.Add_Part(obj);
  top = top + (int)obj.Get(Am_HEIGHT) + 2;

  obj = Am_Text.Create()
            .Set(Am_LEFT, 2)
            .Set(Am_TOP, top)
            .Set(Am_TEXT,
                 "Right click on object to inspect. (Shift-Right for new win)");
  window.Add_Part(obj);
  top = top + (int)obj.Get(Am_HEIGHT) + 2;

  // blank line
  top = top + (int)obj.Get(Am_HEIGHT) + 2;
#endif

#ifdef INSPECTOR_USE_WIDGETS
  obj = Am_Scrolling_Group.Create()
            .Set(Am_INNER_FILL_STYLE, very_light_gray)
#else
  obj = Am_Group
            .Create()
#endif
            .Set(Am_LEFT, GROUP_OFFSET)
            .Set(Am_TOP, top)
            .Set(Am_WIDTH, get_window_width)
            .Set(Am_HEIGHT, get_window_height)
            .Set(Am_V_SCROLL_BAR_ON_LEFT, true);
  window.Add_Part(Am_GROUP_OBJECT, obj);
}

Am_Define_Method(Am_Where_Method, Am_Object, inter_over_object,
                 (Am_Object /* inter */, Am_Object /* object */,
                  Am_Object event_window, int x, int y))
{
  Am_Object group, result;
  group = event_window.Get(Am_GROUP_OBJECT);
  if (!group.Valid())
    Am_Error("where did the group go?");
  result = Am_Point_In_Part(group, x, y, event_window);
  if (result.Valid() && Am_Object(result.Get(Am_INSPECTING_OBJECT)).Valid())
    return result;
  else
    return (0L);
}

Am_Define_Method(Am_Where_Method, Am_Object, inter_over_any,
                 (Am_Object /* inter */, Am_Object /* object */,
                  Am_Object event_window, int x, int y))
{
  Am_Object group, result;
  group = event_window.Get(Am_GROUP_OBJECT);
  if (!group.Valid())
    Am_Error("where did the group go?");
  result = Am_Point_In_Part(group, x, y, event_window);
  return result;
}

Am_Define_Method(Am_Where_Method, Am_Object, inter_over_value,
                 (Am_Object /* inter */, Am_Object /* object */,
                  Am_Object event_window, int x, int y))
{
  Am_Object group, result;
  group = event_window.Get(Am_GROUP_OBJECT);
  if (!group.Valid())
    Am_Error("where did the group go?");
  result = Am_Point_In_Part(group, x, y, event_window);
  if (result.Valid() && ((bool)result.Get(Am_INSPECTING_VALUE)))
    return result;
  else
    return (0L);
}

//if click over a value, then will start, so turn off selection
Am_Define_Method(Am_Where_Method, Am_Object, inter_over_value_for_edit,
                 (Am_Object inter, Am_Object object, Am_Object event_window,
                  int x, int y))
{
  Am_Object result = inter_over_value_proc(inter, object, event_window, x, y);
  if (result.Valid())
    set_selection(event_window, Am_No_Object);
  return result;
}

Am_Define_Method(Am_Object_Method, void, inspector_edit_value, (Am_Object self))
{
  Am_Object string_obj, window, obj_being_inspected;
  Am_Slot_Key which_slot;
  Am_String editstr;
  Am_Value_Type typ;

  string_obj = self.Get(Am_OBJECT_MODIFIED);
  editstr = string_obj.Get(Am_TEXT);

  string_obj.Set(Am_CURSOR_INDEX, Am_NO_CURSOR);
  which_slot = (int)string_obj.Get(Am_INSPECTING_SLOT);
  window = self.Get_Owner().Get(Am_WINDOW);
  obj_being_inspected = window.Get(Am_INSPECTING_OBJECT);
  typ = obj_being_inspected.Get_Slot_Type(which_slot);
  Am_Value output_value;
  //Am_Parse_Input_As_Type is exported by debugger.h and implemented in
  //   debug/input_parser.cc
  bool ok = Am_Parse_Input_As_Type(editstr, typ, output_value);
  if (ok) {
    obj_being_inspected.Set(which_slot, output_value);
  } else {
    Am_Beep(window);
    std::cout << "** Sorry, could not parse `" << editstr << "' as a ";
    Am_Print_Type(std::cout, typ);
    std::cout << std::endl << std::flush;
  }
  refresh_display_of_object(window, obj_being_inspected);
}

Am_Define_Method(Am_Object_Method, void, inspector_refill_with_object,
                 (Am_Object cmd))
{
  Am_Object string_object, new_object;
  string_object = cmd.Get(Am_VALUE);
  if (string_object.Valid()) {
    Am_Object window, main_window;
    window = string_object.Get(Am_WINDOW);
    main_window = window.Get(Am_MAIN_INSPECTOR_WINDOW);
    if (main_window.Valid())
      window = main_window;
    //get the selected string object's  object
    new_object = string_object.Get(Am_INSPECTING_OBJECT);
    if (new_object.Valid()) {
      set_object_into_window(window, new_object);
    } else
      std::cout << "** Not an object\n" << std::flush;
  } else
    std::cout << "** Nothing selected?\n" << std::flush;
}

Am_Define_Method(Am_Object_Method, void, inspector_new_win_for_object,
                 (Am_Object cmd))
{
  Am_Object string_object, new_object;
  string_object = cmd.Get(Am_VALUE);
  if (string_object.Valid()) {
    Am_Object window, main_window;
    window = string_object.Get(Am_WINDOW);
    main_window = window.Get(Am_MAIN_INSPECTOR_WINDOW);
    if (main_window.Valid())
      window = main_window;
    new_object = string_object.Get(Am_INSPECTING_OBJECT);
    if (new_object.Valid()) {
      int left = window.Get(Am_LEFT);
      int top = window.Get(Am_TOP);
      create_inspector_window(new_object, left + 50, top + 50, Am_NO_SLOT);
    } else
      std::cout << "** Not an object\n" << std::flush;
  } else
    std::cout << "** Nothing selected?\n" << std::flush;
}

Am_Define_Method(Am_Object_Method, void, inspector_key_handler, (Am_Object cmd))
{

  Am_Object inter, window, main_window;
  inter = cmd.Get_Owner();
  window = inter.Get(Am_WINDOW);
  main_window = window.Get(Am_MAIN_INSPECTOR_WINDOW);
  if (main_window.Valid())
    window = main_window;
  Am_Input_Char c = inter.Get(Am_START_CHAR);
  if (c == "CONTROL_q")
    kill_window_proc(window);
  else if (c == "CONTROL_p")
    show_previous_object(window);
  else if (c == "CONTROL_f") {
    Am_Object object;
    object = window.Get(Am_INSPECTING_OBJECT);
    Am_Flash(object);
  } else if (c == "CONTROL_r") {
    Am_Object object;
    object = window.Get(Am_INSPECTING_OBJECT);
    refresh_display_of_object(window, object);
  } else if (c == "CONTROL_f") {
    Am_Object object;
    object = window.Get(Am_INSPECTING_OBJECT);
    Am_Flash(object, std::cout);
  } else if (c == "CONTROL_i") {
    static bool debugging = false;
    if (debugging) {
      Am_Clear_Inter_Trace();
      debugging = false;
    } else {
      Am_Set_Inter_Trace(Am_INTER_TRACE_ALL);
      debugging = true;
    }
  } else
    std::cout << "** Commands = ^q for quit, ^p = previous, ^f = flash, ^r = "
                 "refresh, ^i = inter debug\n"
              << std::flush;
}

Am_Define_Method(Am_Object_Method, void, inspector_select_any, (Am_Object cmd))
{
  Am_Object inter, edit_inter, window, selected, main_window;
  inter = cmd.Get_Owner();
  edit_inter = inter.Get(Am_EDIT_INTER);
  if (edit_inter.Valid())
    Am_Abort_Interactor(edit_inter);
  window = inter.Get(Am_WINDOW);
  selected = cmd.Get(Am_VALUE);
  main_window = window.Get(Am_MAIN_INSPECTOR_WINDOW);
  if (main_window.Valid())
    window = main_window;
  set_selection(window, selected);
  check_move_props_windows(window, selected);
}

static void
create_inspector_interactors(Am_Object &window, bool want_edit)
{
  Am_Object inter, cmd, edit_inter;
  inter = Am_One_Shot_Interactor.Create("inspector_keyboard")
              .Set(Am_START_WHEN, "ANY_KEYBOARD");
  cmd = inter.Get(Am_COMMAND);
  cmd.Set(Am_DO_METHOD, inspector_key_handler);
  window.Add_Part(inter);

  inter = Am_Choice_Interactor.Create("choose_object")
              .Set(Am_START_WHEN, Am_Input_Char("RIGHT_DOWN"))
              .Set(Am_HOW_SET, Am_CHOICE_SET)
              .Set(Am_START_WHERE_TEST, inter_over_object);
  cmd = inter.Get(Am_COMMAND);
  cmd.Set(Am_DO_METHOD, inspector_refill_with_object);
  window.Add_Part(inter);

  if (want_edit) {
    edit_inter = Am_Text_Edit_Interactor
                     .Create("edit_value")
                     //.Set (Am_HOW_SET, Am_CHOICE_SET)
                     .Set(Am_START_WHERE_TEST, inter_over_value_for_edit);
    cmd = edit_inter.Get(Am_COMMAND);
    cmd.Set(Am_DO_METHOD, inspector_edit_value);
    window.Add_Part(edit_inter);
  }

  inter = Am_Choice_Interactor.Create("choose_object_new_win")
              .Set(Am_START_WHEN, Am_Input_Char("SHIFT_RIGHT_DOWN"))
              .Set(Am_START_WHERE_TEST, inter_over_object);
  cmd = inter.Get(Am_COMMAND);
  cmd.Set(Am_DO_METHOD, inspector_new_win_for_object);
  window.Add_Part(inter);

  inter = Am_One_Shot_Interactor.Create("choose_any")
              .Set(Am_START_WHEN, Am_Input_Char("DOUBLE_LEFT_DOWN"))
              .Set(Am_START_WHERE_TEST, inter_over_any)
              .Set(Am_HOW_SET, Am_CHOICE_SET)
              .Set(Am_PRIORITY, 200) //so higher than editing when running
              .Add(Am_EDIT_INTER, edit_inter);
  cmd = inter.Get(Am_COMMAND);
  cmd.Set(Am_DO_METHOD, inspector_select_any);
  window.Add_Part(inter);
}

static Am_Object
create_inspector_window(const Am_Object &for_object, int left, int top,
                        Am_Slot_Key slot_to_select)
{
  bool old_auto_refresh = Am_Automatic_Refresh;
  Am_Automatic_Refresh = false;
  Am_Object window =
      Am_Window.Create("Inspector Window")
          .Set(Am_FILL_STYLE, very_light_gray)
          .Set(Am_LEFT, left)
          .Set(Am_TOP, top)
          .Set(Am_WIDTH, MAX_WIDTH_WINDOW)
          .Set(Am_HEIGHT, int(Am_Screen.Get(Am_HEIGHT)) - top - 40)
          .Set(Am_TITLE, "Inspector")
          .Add(Am_SELECTED_ANY, (0L))
          .Add(Am_SLOT_PROPS_WINDOW, (0L))
          .Add(Am_CONSTRAINT_DEP_WINDOW, (0L))
          .Add(Am_USE_SLOT_WINDOW, (0L))
          .Add(Am_PROTOTYPES_OWNERS_WINDOW, (0L))
          .Add(Am_MAIN_INSPECTOR_WINDOW, (0L))
          .Add(Am_INSPECT_OBJ_LIST, Am_Value_List()) // create an empty list
          .Set(Am_DESTROY_WINDOW_METHOD, kill_window)
          .Add(Am_SORT_BY_NAME, Am_SORT_BY_NAME_DEFAULT)
          .Add(Am_HIDE_INTERNAL, Am_HIDE_INTERNAL_DEFAULT)
          .Add(Am_HIDE_INHERITED, Am_HIDE_INHERITED_DEFAULT)
          .Add(Am_SHOW_PARTS, Am_SHOW_PARTS_DEFAULT)
          .Add(Am_SHOW_INSTANCES, Am_SHOW_INSTANCES_DEFAULT)
          .Add(Am_INSPECTING_OBJECT, 0)
          .Add(Am_VALUE_STRING_OBJECT, (0L));
  Am_All_Inspector_Windows.Add(window);
  add_instr_to_window(window, for_object);
  create_inspector_interactors(window, true);
  set_object_into_window(window, for_object, slot_to_select);
  Am_Screen.Add_Part(window);
  Am_Automatic_Refresh = old_auto_refresh;
  return window;
}

static void
cout_object(const Am_Object &object)
{
  if (object.Valid())
    std::cout << object;
  else
    std::cout << "INVALID OBJECT";
}

// inspect the specific object
void
Am_Inspect(const Am_Object &object, Am_Slot_Key slot)
{
  std::cout << "~~~ Inspecting object ";
  cout_object(object);
  std::cout << std::endl << std::flush;
  Am_Modal_Windows.Make_Empty(); //make any modal windows no longer be modal

  int left = (int)Am_Screen.Get(Am_WIDTH) - MAX_WIDTH_WINDOW - 20;
  int top = 45;
  create_inspector_window(object, left, top, slot);
}

void
Clean_Up_For_Debug()
{
  // clear out the demon queue
  // note cannot merely set Main_Demon_Queue = Am_Demon_Queue();
  // since all objects have their own pointers to the queue data
  Main_Demon_Queue.Reset();

  //make sure doesn't exit immediately
  Am_Main_Loop_Go = true;
}

void
Am_Inspect_And_Main_Loop(const Am_Object &object, Am_Slot_Key slot)
{
  // clean up before initializing inspector
  Clean_Up_For_Debug();

  if (!object.Valid())
    Am_Inspect(Am_Screen, Am_NO_SLOT);
  else
    Am_Inspect(object, slot);
  Am_Main_Event_Loop();
}

static void
invoke_inspector_on_error(const Am_Object &obj, Am_Slot_Key slot)
{
  std::cerr << "**  Do you want to enter the Inspector";
  if (obj.Valid())
    std::cerr << " viewing object " << obj;
  std::cerr << "? (type `y' or `n'): " << std::flush;
  char ans;
  std::cin >> ans;
  if (ans == 'y' || ans == 'Y') {
    std::cerr << "Answer was `" << ans << "'.  Starting Inspector..."
              << std::endl
              << std::flush;
    if (obj.Valid())
      Am_Inspect_And_Main_Loop(obj, slot);
    else
      Am_Inspect_And_Main_Loop(Am_No_Object, Am_NO_SLOT);
  } else {
    std::cerr << "Answer was `" << ans << "'." << std::endl << std::flush;
  }
}

//inspect an object with a particular name. Use overloading
void
Am_Inspect(const char *name)
{
#ifdef DEBUG
  //just use any object
  Am_Object obj = Am_Rectangle.From_String(name);
  if (obj.Valid()) {
    Am_Inspect(obj);
  } else
    std::cout << "** Sorry, can't find an object with that name\n"
              << std::flush;
#endif
}

Am_Define_Method(Am_Object_Method, void, ask_inspect, (Am_Object /* cmd */))
{
#ifdef DEBUG
  // **TEMP
  std::cout << "\nInspect which object? " << std::flush;
  char name[200];
  std::cin >> name;
  Am_Object obj = Am_Rectangle.From_String(name);
  if (obj.Valid()) {
    Am_Inspect(obj);
  } else
    std::cout << "** Sorry, can't find an object with that name\n"
              << std::flush;
#endif
}

static Am_Object
print_pos_find_object(const Am_Object &cmd)
{
  Am_Object inter = cmd.Get_Owner();
  Am_Object window;
  window = inter.Get(Am_WINDOW);
  int x = inter.Get(Am_FIRST_X);
  int y = inter.Get(Am_FIRST_Y);
  std::cout << "~~~ Looking for object at (" << x << "," << y << ") in window ";
  cout_object(window);
  std::cout << std::endl << std::flush;
  Am_Object object = Am_Point_In_Leaf(window, x, y, window, false, false);
  if (!object.Valid())
    object = Am_Point_In_Leaf(window, x, y, window, true, true);
  return object;
}

Am_Define_Method(Am_Object_Method, void, inspect_object_at_point,
                 (Am_Object cmd))
{
  Am_Object object = print_pos_find_object(cmd);
  if (object.Valid())
    Am_Inspect(object);
  else
    std::cout << "~~~ No object found\n" << std::flush;
}

Am_Define_Method(Am_Object_Method, void, show_position_at_point,
                 (Am_Object cmd))
{
  Am_Object object = print_pos_find_object(cmd);
  if (object.Valid()) {
    std::cout << "~~~ Found ";
    cout_object(object);
    std::cout << std::endl << std::flush;
  } else
    std::cout << "~~~ No object found\n" << std::flush;
}

void
Am_Set_Inspector_Keys(Am_Input_Char show_key, Am_Input_Char show_position_key,
                      Am_Input_Char ask_key)
{
  //will be inherited by all instances, so can just set the top
  Am_Inspector_Inter.Set(Am_START_WHEN, show_key);
  Am_Show_Position_Inter.Set(Am_START_WHEN, show_position_key);
  Am_Ask_Inspect_Inter.Set(Am_START_WHEN, ask_key);
}

// Sets up interactors so inspector can be brought up for any object
// in any window
void
Am_Initialize_Inspector()
{
  Am_Initialize_Tracer();

  if (!Am_Inspector_Inter.Valid()) {
    Am_Show_Inspector_Char = Am_Input_Char("F1");
    Am_Show_Position_Char = Am_Input_Char("F2");
    Am_Ask_Inspect_Char = Am_Input_Char("F3");

    Am_Inspector_Inter = Am_One_Shot_Interactor.Create("Inspector_Inter")
                             .Set(Am_START_WHEN, Am_Show_Inspector_Char)
                             .Set(Am_PRIORITY, Am_INSPECTOR_INTER_PRIORITY)
                             .Set(Am_MULTI_OWNERS, true);

    Am_Inspector_Inter.Get_Object(Am_COMMAND)
        .Set(Am_DO_METHOD, inspect_object_at_point)
        .Set(Am_IMPLEMENTATION_PARENT, true) //so not queued for undo
        .Set_Name("Inspect_Object_At_Point")
        .Add(Am_MULTI_OWNERS, true);

    Am_Show_Position_Inter =
        Am_One_Shot_Interactor.Create("Show_Position_Inter")
            .Set(Am_START_WHEN, Am_Show_Position_Char)
            .Set(Am_PRIORITY, Am_INSPECTOR_INTER_PRIORITY)
            .Set(Am_MULTI_OWNERS, true);

    Am_Show_Position_Inter.Get_Object(Am_COMMAND)
        .Set(Am_DO_METHOD, show_position_at_point)
        .Set(Am_IMPLEMENTATION_PARENT, true) //so not queued for undo
        .Set_Name("Show_Position_At_Point")
        .Add(Am_MULTI_OWNERS, true);

    Am_Ask_Inspect_Inter = Am_One_Shot_Interactor.Create("Ask_Inspect_Inter")
                               .Set(Am_START_WHEN, Am_Ask_Inspect_Char)
                               .Set(Am_PRIORITY, Am_INSPECTOR_INTER_PRIORITY)
                               .Set(Am_MULTI_OWNERS, true);

    Am_Ask_Inspect_Inter.Get_Object(Am_COMMAND)
        .Set(Am_DO_METHOD, ask_inspect)
        .Set(Am_IMPLEMENTATION_PARENT, true) //so not queued for undo
        .Set_Name("Ask_Object_To_Inspect")
        .Add(Am_MULTI_OWNERS, true);

#ifdef INSPECTOR_USE_WIDGETS
    initialize_menu_bar();
#endif
  }

  Am_Screen.Add_Part(Am_Inspector_Inter.Create());
  Am_Screen.Add_Part(Am_Show_Position_Inter.Create());
  Am_Screen.Add_Part(Am_Ask_Inspect_Inter.Create());

  // this must be last (since we don't want an error in inspector initialization
  // to trigger the inspector)
  Am_Object_Debugger = invoke_inspector_on_error;
}

void
Am_Cleanup_Inspector()
{
  // Cealnup used prototypes for inspector interactors
  Am_Inspector_Inter.Destroy();
  Am_Show_Position_Inter.Destroy();
  Am_Ask_Inspect_Inter.Destroy();

  // Cleanup used dialog box window prototypes
  Am_Ask_Object_Dialog.Destroy();
  Am_Ask_Inter_Dialog.Destroy();

#ifdef INSPECTOR_USE_WIDGETS
  Am_Inspector_Menu_Bar.Destroy();
#endif

  return;
}

// finds all subwindows between window and object that totally intersect
// object.  No regard given to Z order, or whether object is a part of the
// subwindow.  Returns a list.
static void
intersecting_subwindows(Am_Object &object, Am_Object window,
                        Am_Value_List &hits)
{
  // assume all are on the same screen.  We just need _some_ drawonable
  // to use for clip region calculation.  If no drawonable, assume error and
  // don't do parts.

  Am_Object w;
  w = window.Get(Am_WINDOW);
  if (!w.Valid())
    return;
  Am_Drawonable *d = Am_Drawonable::Narrow(w.Get(Am_DRAWONABLE));
  if (!d)
    return;

  // if it's null we just started, so make it into an empty list instead.
  if (!hits.Valid()) {
    hits = Am_Value_List();
    d->Clear_Clip();
  }
  // only can be obscured by subwindows, but we must recurse through groups
  // in case someone (erroneously) put a window into a group- this _is_ a
  // debugger...
  if (window.Is_Instance_Of(Am_Window)) {
    int left, top;
    // translate containing object to coord system with (0,0) at (left,top) of
    // object.
    Am_Translate_Coordinates(window, 0, 0, object, left, top);
    d->Push_Clip(left, top, (int)window.Get(Am_WIDTH),
                 (int)window.Get(Am_HEIGHT));
    bool total;
    if (d->In_Clip(0, 0, (int)object.Get(Am_WIDTH), (int)object.Get(Am_HEIGHT),
                   total))
      hits.Add(window);
  }
  Am_Part_Iterator iter = window;
  Am_Object part;
  for (iter.Start(); !iter.Last(); iter.Next()) {
    part = iter.Get();
    if (part.Is_Instance_Of(Am_Graphical_Object) ||
        part.Is_Instance_Of(Am_Window))
      intersecting_subwindows(object, iter.Get(), hits);
  }
  d->Pop_Clip();
}

// object_intersects_subwindow
// should return "true" if the object is fully contained by the clip region
// of any subwindow that it is not drawn in.
// Z order is not (yet) taken into account.

static bool
object_intersects_subwindows(Am_Object &object, Am_Object &stop_window)
{
  Am_Object owner;
  Am_Value_List owners, intersections;

  // make a list of owners so we can make sure we're not mistakenly
  // finding a subwindow the object is drawn in.

  for (owner = object; (owner != stop_window) && (owner.Valid());
       owner = owner.Get_Owner())
    owners.Add(owner);
  owners.Add(stop_window);
  // a list of all subwindows totally intersecting the object
  intersecting_subwindows(object, stop_window, intersections);

  //   // print out owners for debugging.
  //  std::cout << "Owners: ";
  //   for (owners.Start(); !owners.Last(); owners.Next())
  //    std::cout << (Am_Object)owners.Get() << " ";
  //  std::cout <<std::endl;
  //
  //   // print out intersections for debugging.
  //  std::cout << "Intersections: ";
  //   for (intersections.Start(); !intersections.Last(); intersections.Next())
  //    std::cout << (Am_Object)intersections.Get() << " ";
  //  std::cout <<std::endl;

  // are there any intersecting subwindows the object isn't part of?
  for (intersections.Start(); !intersections.Last(); intersections.Next()) {
    owners.Start();
    if (!owners.Member((Am_Object)intersections.Get()))
      return true;
  }
  return false;
}

// Am_Flash
// flashes an object by drawing an XOR rectangle around it, or explains why
// the object cannot be seen on the screen.

void Am_Flash(Am_Object o, std::ostream &flashout) // = cout
{
#define MINIMUM_FLASH_SIZE 10
  // Is the object valid?
  if (!o.Valid()) {
    flashout << "The object cannot flash because it is not valid.\n"
             << std::flush;
    return;
  }

  // Is it a graphical object?
  if (!o.Is_Instance_Of(Am_Graphical_Object)) {
    flashout << "Object " << o << " cannot flash because\n  "
             << "it is not a graphical object\n"
             << std::flush;
    if (o.Is_Instance_Of(Am_Interactor) || o.Is_Instance_Of(Am_Command) ||
        o.Is_Instance_Of(Am_Undo_Handler))
      flashout << "  Interactor, command, and undo objects cannot be flashed.\n"
               << std::flush;
    else if (o.Is_Instance_Of(Am_Window) || o.Is_Instance_Of(Am_Screen))
      flashout << "  Windows and screens cannot be flashed.\n" << std::flush;
    else
      flashout << "  The object may not have been initialized properly.\n"
               << std::flush;
    return;
  }

  // Is it visible?
  // NDY: Currently, group invisibility does _not_ apply to part objects.
  // When (if) that changes, this check should go below in the loop through
  // the part hierarchy to make sure the objects' ancestors are also visible.
  if (!(bool)o.Get(Am_VISIBLE)) {
    flashout << "Object " << o
             << " cannot flash because its Am_VISIBLE slot is false.\n"
             << std::flush;
    return;
  }

  // Since it's a graphical object, assume its size/position slots
  // exist.  If they don't, something would already have crashed anyway.

  int left = o.Get(Am_LEFT);
  int top = o.Get(Am_TOP);
  int width = o.Get(Am_WIDTH);
  int height = o.Get(Am_HEIGHT);

  Am_Object window;

  // Traverse up group hierarchy to the Am_Screen.
  // 1. bring top level window to_top
  // 2. make sure it's actually in am_screen
  // 3. NDY: along the way, check to make sure the object is (at least partially)
  //   in the clip region of what appears in the top level window.
  Am_Object owner = o.Get_Owner(); // updated as we go through the hierarchy
  Am_Object owned = o;
  // save owned object: to print its name if needed, also this is the top-level
  // window.
  Am_Object o_owner = owner; // stays constant
  Am_Object first_window;
  int l, t;
  while (owner.Valid() && !owner.Is_Instance_Of(Am_Screen)) {
    Am_Translate_Coordinates(o_owner, left, top, owner, l, t);
    if (l + width < 0 || t + height < 0 || l > (int)owner.Get(Am_WIDTH) ||
        t > (int)owner.Get(Am_HEIGHT))
    // then the object is out of range
    {
      if (owner == o_owner)
        flashout << "Object " << o
                 << " can't flash because\n  it is completely outside the "
                    "bounding box of its parent.\n"
                 << std::flush;
      else
        flashout << "Object " << o
                 << " can't flash because\n  it is completely outside the "
                    "bounding box of the groups containing it.\n"
                 << std::flush;
      return;
    }
    if (!first_window.Valid() && owner.Is_Instance_Of(Am_Window))
      first_window = owner;
    owned = owner;
    owner = owner.Get_Owner();
  }
  if (!owner.Valid()) {
    // if o is the object owned, so o has no direct owner, then say that
    // directly.  If not, tell the user which object has no owner, as well
    // as the object they want to flash.
    if (o == owned)
      flashout << "Object " << o << " can't flash because it has no owner.\n"
               << "  All graphical objects and groups should be added to a "
                  "window in Am_Screen.\n"
               << std::flush;
    else
      flashout << "Object " << o << " can't flash because\n  "
               << "it is a part-descendant of object " << owned
               << " which has no owner.\n  All graphical objects and groups "
               << "should be added to a window in Am_Screen.\n"
               << std::flush;
    return;
  }
  if (owned.Is_Instance_Of(Am_Window)) // (its owner is Am_Screen)
  {
    window = owned;
    // This is a hack.
    // The correct way to handle flashing is probably to return control to
    // Opal and let an animation interactor do it, but we don't have that
    // option right now.  If we don't call process_event and do_events,
    // the window will not be redrawn after it's brought to top, which is
    // bad.  This just happens to work, but there's no guarantee it will-
    // we have no idea what the latency of the events from X will be, so
    // we have no guarantee they'll get there before we Process_Event.
    Am_To_Top(owned); // bring the top level window to top.
    Am_Drawonable::Process_Immediate_Event();
    Am_Do_Events();
  } else {
    flashout << "Object " << o << "can't flash because\n"
             << " it is a part-descendant of object " << owned
             << "\n  which is a non-window part of Am_Screen.  "
             << "Don't add non-graphical parts\n  to Am_Screen." << std::flush;
    return;
  }

  // Now, the object is in o, and its top level window is in window.
  // The first window above o in the part hierarchy is first_window: this is
  // the window the flashing takes place in.
  // We want to check to make sure that the object is not fully obscured by a
  // subwindow.  If it is, then we won't be able to see it flash.

  if (object_intersects_subwindows(o, window))
    flashout << "Object " << o << " may not flash: \n"
             << "  it might be obscured by a subwindow. \n";

  // Get the object's window's drawonable.

  Am_Value v;
  Am_Drawonable *d;
  v = first_window.Peek(Am_DRAWONABLE);
  if (!v.Exists()) {
    flashout << "Object " << o << " cannot flash because\n"
                                  "  its window is not valid.\n";
    return;
  }
  if (!(d = Am_Drawonable::Narrow((Am_Ptr)v))) {
    flashout << "Object " << o << " cannot flash because\n"
                                  "  its window is not valid.\n";
    return;
  }

  // warn if the object is too small
  if (width < MINIMUM_FLASH_SIZE || height < MINIMUM_FLASH_SIZE)
    flashout << "Object " << o << " is very small.\n"
                                  "A larger area near the object will flash.\n"
             << std::flush;

  flashout << "Object " << o << " is flashing.\n" << std::flush;

  width = width > MINIMUM_FLASH_SIZE ? width : MINIMUM_FLASH_SIZE;
  height = height > MINIMUM_FLASH_SIZE ? height : MINIMUM_FLASH_SIZE;

  // clear the drawonable's clip region so it will draw properly.
  d->Clear_Clip();

  // (0,0) is the top left of o relative to itself.  first_window is the
  // first window above o in the visual hierarchy.  We want to translate the
  // object into first_window, and flash in this window (d is its drawonable)
  // and not the top-level window, because if we draw in the top-level window,
  // the flashing will be obscured if the object is in a subwindow.
  Am_Translate_Coordinates(o, 0, 0, first_window, l, t);

// This won't work
//  Am_Translate_Coordinates(o_owner, left, top, window, l, t);
// This would also work
// Am_Translate_Coordinates(o_owner, left, top, first_window, l, t);
#ifdef _WIN32
  Am_Style flash_line(1.0f, 1.0f, 1.0f, 4); // white, thickness 4
#else
#define flash_line Am_Line_4
#endif
  for (int i = 0; i < 16; i++) {
    d->Draw_Rectangle(flash_line, Am_No_Style, l, t, width, height,
                      Am_DRAW_XOR);
    d->Flush_Output();

    Am_Wait(200); // 200 milliseconds
  }
}

void
Am_Refresh_Inspector_If_Object_Changed(const Am_Slot &slot,
                                       Am_Set_Reason reason)
{
#ifdef DEBUG
  static int flag = 0;
  if (flag)
    return; // prevent recursive call.
  // We have a single global variable controlling refresh of all inspector
  // windows.  Without this, we'd still need to do a list traverse below,
  // whether we're refreshing the windows or not.
  if (!Am_Automatic_Refresh)
    return;
  if (reason & Am_TRACE_SLOT_DESTROY)
    return;
  flag = 1;
  Am_Object w, o;
  Am_Value v;
  o = slot.Get_Owner();
  for (Am_All_Inspector_Windows.Start(); !Am_All_Inspector_Windows.Last();
       Am_All_Inspector_Windows.Next()) {
    w = Am_All_Inspector_Windows.Get();
    safe_get(w, Am_INSPECTING_OBJECT, v);
    if (v.Valid() &&           // slot exits
        ((Am_Object)v == o)) { // and this is the object we're inspecting
      // Refresh the display.
      // Value lists pose a problem, since they're made of multiple string
      // objects.  Since they can change the display so much, refresh the
      // whole window if the value was, or now is, a value list.
      // If it _was_ a value list, it won't pass the Am_INSPECTING_VALUE
      // test below, since we don't set that slot on value list values.
      if (slot.Get().type != Am_VALUE_LIST) {
        // not a value_list, so just update the slot
        Am_Slot_Key key = slot.Get_Key();
        Am_Object ob;
        ob = w.Get_Object(Am_GROUP_OBJECT);
        Am_Part_Iterator pi(ob);
        // look at all of the string objects to find the one (if any)
        // corresponding to the slot that has changed.
        for (pi.Start(); !pi.Last(); pi.Next()) {
          ob = pi.Get();
          v = ob.Peek(Am_INSPECTING_SLOT);
          if (v.Valid() && (int)v == key) {
            v = ob.Peek(Am_INSPECTING_VALUE);
            if (v.Valid()) {
              // we have the right string_object, so reset its Am_TEXT
              char line[250];
              OSTRSTREAM_CONSTR(oss, line, 250, std::ios::out);
              reset_ostrstream(oss); // go to the beginning
              oss << slot.Get() << std::ends;
              OSTRSTREAM_COPY(oss, line, 250);
              ob.Set(Am_TEXT, line);

              flag = 0;
              return;
            }
          }
        }
      }
      // If we get here, we're in the correct window, but either it's
      // a value list or a new slot that didn't exist before.
      refresh_display_of_object(w, o);
    }
  }
  flag = 0; // release "lock" on this procedure.
#endif
}

/* FUTURE WORK
** test method trace/break
** method menu item not enabled when slot selected


search dialog box:
Find all objects that are {instances of | parts of}
 {Am_Root_Object | Am Screen}
(o) and which have a value { == != < > } ______ in slot ______

show for parts which are inherited or not, and what slots they come from

break when this slot APPEARS into this object


 */
