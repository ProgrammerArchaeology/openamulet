/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code has been placed in the public   			          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/*Originally written as AmEdit by S.Nebel (Linkworks Ltd Wellington,NZ) 1997.
   Modified and updated by Brad A. Myers of the Amulet project at CMU.
*/

#include <amulet.h>

#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <amulet/am_strstream.h> // strstream is needed for AM_POP_UP_ERROR_WINDOW

#ifdef GCC
#include <string.h>
#else
extern "C" {
#if defined(_MSC_VER) || defined(NEED_STRING)
#include <string.h>
#endif
}
#endif

#include <amulet/inter_advanced.h> //for Am_Copy_Values_To_Command for polygon

AM_DEFINE_METHOD_TYPE(Am_Customize_Object_Method, void,
                      (Am_Object & command_obj, Am_Object &object))
AM_DEFINE_METHOD_TYPE_IMPL(Am_Customize_Object_Method)

#include "panel.h"
#include "externs.h"
#include "savecpp.h"
#include "outline.h"

using namespace std;

Am_Slot_Key CUSTOMIZE_METHOD = Am_Register_Slot_Name("CUSTOMIZE_METHOD");
Am_Slot_Key SUB_LABEL = Am_Register_Slot_Name("SUB_LABEL");
Am_Slot_Key Lw_NAME = Am_Register_Slot_Name("Lw_NAME");
Am_Slot_Key FONT_KEY = Am_Register_Slot_Name("FONT_KEY");
Am_Slot_Key LAYOUT_KEY = Am_Register_Slot_Name("LAYOUT_KEY");
Am_Slot_Key FILL_STYLE_KEY = Am_Register_Slot_Name("FILL_STYLE_KEY");
Am_Slot_Key LINE_STYLE_KEY = Am_Register_Slot_Name("LINE_STYLE_KEY");
Am_Slot_Key TYPE_NAME = Am_Register_Slot_Name("TYPE_NAME");
Am_Slot_Key SAVE_COMMAND_ITEMS = Am_Register_Slot_Name("SAVE_COMMAND_ITEMS");
Am_Slot_Key SEL_WIDGET = Am_Register_Slot_Name("SEL_WIDGET");
Am_Slot_Key MAIN_GROUP = Am_Register_Slot_Name("MAIN_GROUP");
Am_Slot_Key MAIN_GROUP_RECT = Am_Register_Slot_Name("MAIN_GROUP_RECT");
Am_Slot_Key WORK_GROUP = Am_Register_Slot_Name("WORK_GROUP");
Am_Slot_Key L_FEEDBACK = Am_Register_Slot_Name("L_FEEDBACK");
Am_Slot_Key R_FEEDBACK = Am_Register_Slot_Name("R_FEEDBACK");
Am_Slot_Key GRID_COMMAND = Am_Register_Slot_Name("GRID_COMMAND");
Am_Slot_Key OPEN_COMMAND = Am_Register_Slot_Name("OPEN_COMMAND");
Am_Slot_Key FILE_NAME = Am_Register_Slot_Name("FILE_NAME");
Am_Slot_Key EXISTING_MENU = Am_Register_Slot_Name("EXISTING_MENU");
Am_Slot_Key CONSTRAINT_WIDGET = Am_Register_Slot_Name("CONSTRAINT_WIDGET");
Am_Slot_Key CONSTRAINT_INFO = Am_Register_Slot_Name("CONSTRAINT_INFO");
Am_Slot_Key REF_COUNT = Am_Register_Slot_Name("REF_COUNT");

Am_Object Fill_Style_Panel, Line_Color_Panel, Fill_Style_Button,
    Line_Color_Button;
Am_Object Line_Thickness_Option_Button, win, button_panel;
Am_Object Font_Style_Button, Font_Type_Button, Font_Size_Button;
Am_Object main_window_proto, main_group_rect_proto;
Am_Object destroy_window_command;

paneldialog paneldlg;

Am_Object arrow_bitmap, arc_proto, rectangle_proto, line_proto, polygon_proto,
    text_proto, border_rectangle_proto, button_proto, buttons_proto, ok_proto,
    checkbox_proto, radio_proto, text_input_proto, number_input_proto,
    option_proto, scroll_group_proto, bitmap_proto;

Am_Object one_way_arrow_line_proto;
Am_Object two_way_arrow_line_proto;

Am_Object tool_panel;
Am_Object global_undo_handler;
Am_Value_List Selection_Widget_List, Line_Style_List;

Am_Value_List window_list, closed_window_list;

int window_count;
Am_Value proto_id;
bool CHANGE_TO_ARROW = true;

#if TEST
Am_Object line_test, text_test, test_win;
#endif

#define ARROW_ID 1
#define RUN_ID 2
#define POLYGON_ID 3
#define CONSTRAINT_ID 4

int grid_amt = 20;

/***********************************************************************
  A Helper function to get right Am_Style value for
  *********************************************************************/
Am_Style
Get_Line_Style(Am_Value_List vl)
{
  int c, t;

  vl.Start();
  c = vl.Get();
  vl.Next();
  t = vl.Get();
  ;

  return n2l[c][t];
}

Am_Font
Get_Font_Style(Am_Value_List vl)
{
  int c, t, s;

  vl.Start();
  c = vl.Get();
  vl.Next();
  t = vl.Get();
  vl.Next();
  s = vl.Get();

  return fontarray[c][t][s];
}

void
fix_lw_name(Am_String &name)
{
  char *s = name;
  bool first_char = true;
  while (*s != 0) {
    if (!((*s >= 'a' && *s <= 'z') || (*s >= 'A' && *s <= 'Z') ||
          ((!first_char) && (*s >= '0' && *s <= '9'))))
      *s = '_';
    if (first_char)
      first_char = false;
    s++;
  }
}

Am_Define_Formula(int, grid_if_should)
{
  Am_Value grid_on;
  Am_Object grid_command = self.Get_Owner().Get_Owner().Get(GRID_COMMAND);
  grid_on = grid_command.Peek(Am_VALUE);
  if (grid_on.Valid())
    return grid_amt;
  else
    return 0;
}

Am_Define_Formula(int, grid_if_should_second)
{
  Am_Value grid_on;
  Am_Object grid_command =
      self.Get_Owner().Get_Owner().Get_Owner().Get(GRID_COMMAND);
  grid_on = grid_command.Peek(Am_VALUE);
  if (grid_on.Valid())
    return grid_amt;
  else
    return 0;
}

Am_Define_Formula(bool, selection_tool)
{
  Am_Value v = tool_panel.Peek(Am_VALUE);
  if (v.type == Am_INT && v == ARROW_ID)
    return true;
  else {
    if (v.type == Am_INT && v == CONSTRAINT_ID) {
      self.Get_Sibling(CONSTRAINT_WIDGET, Am_NO_DEPENDENCY)
          .Set(Am_VALUE, self.Get(Am_VALUE, Am_NO_DEPENDENCY));
    }
    return false;
  }
}

Am_Define_Formula(bool, constraint_tool)
{
  Am_Value v = tool_panel.Peek(Am_VALUE);
  if (v.type == Am_INT && v == CONSTRAINT_ID)
    return true;
  else { //How does Am_Selection_Widget work?
    Am_Value_List l = self.Get(Am_VALUE);
    l.Make_Empty();
    self.Set(Am_VALUE, l);
    return false;
  }
}

Am_Define_Formula(bool, run_tool)
{
  Am_Value v = tool_panel.Peek(Am_VALUE);
  Am_Object menu_bar = self.Get_Owner();

  if (v.type == Am_INT && v == RUN_ID) {
    if (self.Is_Instance_Of(menu_proto)) {
      if (menu_bar.Valid()) { // Owner should be valid
        menu_bar = menu_bar.Get_Owner().Get_Sibling(EXISTING_MENU);
        menu_bar.Set(Am_ACTIVE, false);
      }
    }
    return true;
  } else {
    if (self.Is_Instance_Of(menu_proto)) {
      if (menu_bar.Valid()) { // Owner should be valid
        menu_bar = menu_bar.Get_Owner().Get_Sibling(EXISTING_MENU);
        menu_bar.Set(Am_ACTIVE, true);
      }
    }
    return false;
  }
}

Am_Define_No_Self_Formula(bool, line_tool)
{
  Am_Value v = tool_panel.Peek(Am_VALUE);
  if (v.type == Am_OBJECT && v == line_proto)
    return true;
  else
    return false;
}

Am_Define_Object_Formula(compute_feedback_obj)
{
  Am_Object main_group = self.Get_Owner().Get_Owner();
  Am_Object lfeedback = main_group.Get_Object(L_FEEDBACK);
  Am_Object rfeedback = main_group.Get_Object(R_FEEDBACK);

  if ((bool)self.Get(Am_AS_LINE))
    return lfeedback;
  else
    return rfeedback;
}
Am_Define_No_Self_Formula(bool, rubber_bandable_tool_is_selected)
{
  Am_Value v = tool_panel.Peek(Am_VALUE);
  if (v.type == Am_INT)
    return false;
  else
    return true;
}
Am_Define_No_Self_Formula(int, points_from_tool)
{
  Am_Value v = tool_panel.Peek(Am_VALUE);
  if (v.type == Am_OBJECT) {
    Am_Object proto = v;
    v = proto.Get(Am_HOW_MANY_POINTS, Am_NO_DEPENDENCY);
    if (v.Valid() && v.type == Am_INT)
      return v;
  }
  return 2;
}
Am_Define_No_Self_Formula(int, minw_from_tool)
{
  Am_Value v = tool_panel.Peek(Am_VALUE);
  if (v.type == Am_OBJECT) {
    Am_Object proto = v;
    v = proto.Get(Am_HOW_MANY_POINTS, Am_NO_DEPENDENCY);
    if (v.Valid() && v.type == Am_INT && v == 1)
      return proto.Get(Am_WIDTH, Am_NO_DEPENDENCY);
    else
      return proto.Get(Am_MINIMUM_WIDTH);
  }
  return 10;
}
Am_Define_No_Self_Formula(int, minh_from_tool)
{
  Am_Value v = tool_panel.Peek(Am_VALUE);
  if (v.type == Am_OBJECT) {
    Am_Object proto = v;
    v = proto.Get(Am_HOW_MANY_POINTS, Am_NO_DEPENDENCY);
    if (v.Valid() && v.type == Am_INT && v == 1)
      return proto.Get(Am_HEIGHT, Am_NO_DEPENDENCY);
    else
      return proto.Get(Am_MINIMUM_HEIGHT);
  }
  return 10;
}
Am_Define_No_Self_Formula(bool, polygon_tool_is_selected)
{
  Am_Value v = tool_panel.Peek(Am_VALUE);
  if (v.type == Am_INT && v == POLYGON_ID)
    return true;
  else
    return false;
}

Am_Define_No_Self_Formula(Am_Wrapper *, undo_handler_if_not_running)
{
  //disable undoing if running
  Am_Value v = tool_panel.Peek(Am_VALUE);
  if (v.type == Am_INT && v == RUN_ID)
    return Am_No_Object;
  else
    return global_undo_handler;
}

const char *fontnames[] = {
    "Sm", "B", "I", "U", "Me", "B", "I", "U", "La", "B", "I", "U",
};

Am_String
Add_Extension(Am_String in_filename, const char *new_ext)
{
  if (!in_filename.Valid())
    return in_filename;
  char *in_filestr = in_filename;
  int len = strlen(in_filestr);
  if (len == 0)
    return in_filename;
  char *ext_part = strrchr(in_filestr, '.');
  int ext_len = 0;
  if (ext_part) {
    ext_len = strlen(ext_part);
    if (ext_len > 5) //probably not a real extension
      ext_len = 0;
  }
  int new_ext_len = strlen(new_ext);
  int new_len = len - ext_len + new_ext_len;
  char *new_str = new char[new_len + 1]; //terminating \0
  new_str = strncpy(new_str, in_filestr, len - ext_len);
  new_str[len - ext_len] = '\0';
  new_str = strcat(new_str, new_ext);
  return Am_String(new_str, false);
}

/////////////////////// save load stuff /////////////////////////

void
Initialize_New_Item_And_Add(Am_Object &workgroup, Am_Object &item)
{
  Am_Value value;
  Am_Style s;
  cout << "Adding " << item << " to " << workgroup << endl;
  value = item.Peek(FILE_NAME);
  if (value.Exists() && value != "")
    item.Set(Am_IMAGE, Am_Image_Array((char *)(Am_String)value));

  value = item.Peek(Am_ACTIVE);
  if (value.Exists())
    item.Set(Am_ACTIVE, value);

  value = item.Peek(Am_ACTIVE_2);
  if (value.Exists())
    item.Set(Am_ACTIVE_2, run_tool);

  value = item.Peek(FONT_KEY);
  if (value.Exists()) {
    item.Set(Am_FONT, Get_Font_Style((Am_Value_List)value));
  }
  value = item.Peek(LAYOUT_KEY);
  if (value.Exists()) {
    if ((int)value == 1)
      item.Set(Am_LAYOUT, Am_Horizontal_Layout);
    else
      item.Set(Am_LAYOUT, Am_Vertical_Layout);
  }
  value = item.Peek(FILL_STYLE_KEY);
  if (value.Exists()) {
    item.Set(Am_FILL_STYLE, n2s[(int)value]);
  }
  value = item.Peek(LINE_STYLE_KEY);
  if (value.Exists()) {
    s = Get_Line_Style((Am_Value_List)value);
    item.Add(Am_LINE_STYLE, s, Am_OK_IF_THERE);
  }

  value = item.Peek(SUB_LABEL);
  if (value.Exists()) {
    cout << "Adding sublabel\n";
    if (Am_String::Test(value))
      item.Get_Object(Am_COMMAND).Set(Am_LABEL, value);
    else {
      Am_Object cmd = item.Get_Object(Am_COMMAND);
      Am_Object label = (Am_Object)value;
      Initialize_New_Item_And_Add(cmd, label);
    }
  }
  value = item.Peek(Am_GRAPHICAL_PARTS);
  if (value.Exists()) {
    Am_Value_List l = (Am_Value_List)value;
    Am_Object part;
    for (l.Start(); !l.Last(); l.Next()) {
      part = (Am_Object)l.Get();
      Initialize_New_Item_And_Add(item, part);
    }
  }
  if (workgroup.Is_Instance_Of(Am_Command))
    workgroup.Set(Am_LABEL, item);
  else if ((Am_Object)item.Get_Owner() != workgroup)
    workgroup.Add_Part(item);
}

// This method should take the supplied contents list and add it to
// the window, after removing what is already there
Am_Define_Method(Am_Handle_Loaded_Items_Method, void, use_file_contents,
                 (Am_Object /*command*/, Am_Value_List &contents))
{
  int x = 0, y = 0;
  cout << "Items needed to be loaded are ::" << contents << endl;
  Am_Object window;
  bool closed_window = false;

  if (closed_window_list.Empty()) {
    window = main_window_proto.Create();
    window.Set(Am_UNDO_HANDLER, Am_Multiple_Undo_Object.Create());
  } else {
    closed_window = true;
    closed_window_list.Start();
    window = (Am_Object)closed_window_list.Get();
    window.Set(Am_WIDTH, 500).Set(Am_HEIGHT, 650);
    closed_window_list.Delete();
  }
  Am_Object tmp_win;

  for (window_list.Start(); !window_list.Last(); window_list.Next()) {
    tmp_win = (Am_Object)window_list.Get();
    if (((int)tmp_win.Get(Am_LEFT) == (200 + x)) ||
        ((int)tmp_win.Get(Am_TOP) == (200 + y))) {
      x += 20;
      y += 20;
    }
  }

  window.Set(Am_LEFT, (int)(200 + x)).Set(Am_TOP, (int)(200 + y));
  window_list.Add(window);

  Am_Style s;
  Am_Object item;

  Am_Object main_group = window.Get_Object(MAIN_GROUP);
  Am_Object main_group_rect = main_group.Get(MAIN_GROUP_RECT);
  Am_Object workgroup = main_group.Get_Object(WORK_GROUP);
  Selection_Widget_List.Add(main_group.Get_Object(SEL_WIDGET));

  main_window_proto.Get_Object(MAIN_GROUP)
      .Get_Object(SEL_WIDGET)
      .Set(Am_MULTI_SELECTIONS, Selection_Widget_List);

  //now add the new objects, fixing up the slots
  Am_Value value;
  for (contents.Start(); !contents.Last(); contents.Next()) {
    item = contents.Get();
    cout << "Item = " << item << endl;
    if (item.Is_Instance_Of(main_group_rect_proto)) {
      main_group_rect.Set(MAIN_NAME, item.Get(MAIN_NAME));
      main_group_rect.Set(C_FILENAME, item.Get(C_FILENAME));
      main_group_rect.Set(CREATE_HEADER, item.Get(CREATE_HEADER));
      main_group_rect.Set(H_FILENAME, item.Get(H_FILENAME));
      main_group_rect.Set(WINDOW_OR_GROUP, item.Get(WINDOW_OR_GROUP));
      main_group_rect.Set(WIN_TITLE, item.Get(WIN_TITLE));
      window_count--;
      main_group_rect.Set(FIXED_SIZE_OBJ, item.Get(FIXED_SIZE_OBJ));
      main_group_rect.Set(EXPLICIT_SIZE_OBJ, item.Get(EXPLICIT_SIZE_OBJ));
      main_group_rect.Set(WIDTH_OBJ, item.Get(WIDTH_OBJ));
      main_group_rect.Set(HEIGHT_OBJ, item.Get(HEIGHT_OBJ));
      main_group_rect.Set(FILL_STYLE_KEY, item.Get(FILL_STYLE_KEY));
      main_group_rect.Set(Am_FILL_STYLE, n2s[(int)item.Get(FILL_STYLE_KEY)]);
    } else {
      Initialize_New_Item_And_Add(workgroup, item);
    }
  }
  if (closed_window)
    window.Set(Am_VISIBLE, true);
  else
    Am_Screen.Add_Part(window);
}

//This method should return the list of objects to save
Am_Define_Method(Am_Items_To_Save_Method, Am_Value_List, contents_for_save,
                 (Am_Object command))
{
  cout << "Command in this method is :: " << command << endl;
  Am_Object main_group =
      command.Get_Object(Am_SAVED_OLD_OWNER).Get_Sibling(MAIN_GROUP);
  Am_Object main_group_rect = main_group.Get_Object(MAIN_GROUP_RECT);
  Am_Object workgroup = main_group.Get_Object(WORK_GROUP);

  Am_Value_List obs_to_save = workgroup.Get(Am_GRAPHICAL_PARTS);
  obs_to_save.Add(main_group_rect);
  cout << "List to be saved is :: " << obs_to_save << endl;
  return obs_to_save;
}

////////////////////////////////////////////////////////////////////////

Am_Define_Style_Formula(get_fill_style)
{
  self.Set(FILL_STYLE_KEY, (int)self.Get(Am_RANK), Am_OK_IF_NOT_THERE);
  if (self.Get(Am_RANK) == -1)
    return (Am_No_Style);
  else
    return n2s[(int)self.Get(Am_RANK)];
}

Am_Define_Style_Formula(get_line_color)
{
  self.Set(LINE_STYLE_KEY, (int)self.Get(Am_RANK), Am_OK_IF_NOT_THERE);

  if (((int)self.Get(Am_RANK)) < 1)
    return (Am_No_Style);
  else
    return n2l[(int)self.Get(Am_RANK)][0];
}

Am_Define_String_Formula(fontgen)
{
  if (self.Get(Am_RANK) == -1)
    return (Am_String("0"));
  else {
    return (Am_String(fontnames[(int)self.Get(Am_RANK)]));
  }
}

Am_Define_Method(Am_Object_Method, void, savecppcmd, (Am_Object cmd))
{

  Am_Object main_group =
      cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner().Get_Object(MAIN_GROUP);
  Am_Object window = main_group.Get_Owner();
  Am_Object main_group_rect = main_group.Get_Object(MAIN_GROUP_RECT);
  Am_Object workgroup = main_group.Get_Object(WORK_GROUP);
  Am_Object sel_widget = main_group.Get_Object(SEL_WIDGET);

  Am_String save_name =
      sel_widget.Get(Am_DEFAULT_LOAD_SAVE_FILENAME, Am_RETURN_ZERO_ON_ERROR);

  if (!save_name.Valid())
    save_name = "";
  Am_String main_obj_name = main_group_rect.Get(MAIN_NAME);
  if (!main_obj_name.Valid() || main_obj_name == "") {
    main_obj_name = Add_Extension(save_name, "");
  }
  if (!main_obj_name.Valid() || main_obj_name == "")
    main_obj_name = "object";
  fix_lw_name(main_obj_name);

  Am_String c_file_name = main_group_rect.Get(C_FILENAME);
  if (!c_file_name.Valid() || c_file_name == "") {
    c_file_name = Add_Extension(save_name, ".cc");
  }

  bool create_header = main_group_rect.Get(CREATE_HEADER);
  Am_String h_file_name = main_group_rect.Get(H_FILENAME);
  if (!h_file_name.Valid() || h_file_name == "") {
    h_file_name = Add_Extension(save_name, ".h");
  }
  bool create_window = main_group_rect.Get(WINDOW_OR_GROUP);
  Am_String window_title = main_group_rect.Get(WIN_TITLE);
  bool win_fixed_size = main_group_rect.Get(FIXED_SIZE_OBJ);
  bool explicit_size = main_group_rect.Get(EXPLICIT_SIZE_OBJ);
  int width = main_group_rect.Get(Am_WIDTH);
  int height = main_group_rect.Get(Am_HEIGHT);

  savecpp_window.Get_Object(MAIN_NAME).Set(Am_VALUE, main_obj_name);
  savecpp_window.Get_Object(C_FILENAME).Set(Am_VALUE, c_file_name);

  if (create_header)
    savecpp_window.Get_Object(CREATE_HEADER)
        .Set(Am_VALUE, Am_Value_List().Add(1));
  else
    savecpp_window.Get_Object(CREATE_HEADER).Set(Am_VALUE, Am_Value_List());

  savecpp_window.Get_Object(H_FILENAME).Set(Am_VALUE, h_file_name);

  if (create_window)
    savecpp_window.Get_Object(WINDOW_OR_GROUP).Set(Am_VALUE, 1);
  else
    savecpp_window.Get_Object(WINDOW_OR_GROUP).Set(Am_VALUE, 2);

  savecpp_window.Get_Object(WIN_TITLE).Set(Am_VALUE, window_title);

  if (win_fixed_size)
    savecpp_window.Get_Object(FIXED_SIZE_OBJ)
        .Set(Am_VALUE, Am_Value_List().Add(1));
  else
    savecpp_window.Get_Object(FIXED_SIZE_OBJ).Set(Am_VALUE, Am_Value_List());

  if (explicit_size)
    savecpp_window.Get_Object(EXPLICIT_SIZE_OBJ).Set(Am_VALUE, 1);
  else
    savecpp_window.Get_Object(EXPLICIT_SIZE_OBJ).Set(Am_VALUE, 2);

  savecpp_window.Get_Object(WIDTH_OBJ).Set(Am_VALUE, width);
  savecpp_window.Get_Object(HEIGHT_OBJ).Set(Am_VALUE, height);

  Am_Value ok;
  Am_Pop_Up_Window_And_Wait(savecpp_window, ok, true);
  if (ok.Valid()) {
    main_obj_name = savecpp_window.Get_Object(MAIN_NAME).Get(Am_VALUE);
    fix_lw_name(main_obj_name);

    c_file_name = savecpp_window.Get_Object(C_FILENAME).Get(Am_VALUE);
    Am_Value_List l = savecpp_window.Get_Object(CREATE_HEADER).Get(Am_VALUE);
    l.Start();
    create_header = l.Member(1);
    h_file_name = savecpp_window.Get_Object(H_FILENAME).Get(Am_VALUE);
    int val = savecpp_window.Get_Object(WINDOW_OR_GROUP).Get(Am_VALUE);
    create_window = val == 1;
    window_title = savecpp_window.Get_Object(WIN_TITLE).Get(Am_VALUE);
    l = savecpp_window.Get_Object(FIXED_SIZE_OBJ).Get(Am_VALUE);
    l.Start();
    win_fixed_size = l.Member(1);
    val = savecpp_window.Get_Object(EXPLICIT_SIZE_OBJ).Get(Am_VALUE);
    explicit_size = val == 1;
    width = savecpp_window.Get_Object(WIDTH_OBJ).Get(Am_VALUE);
    height = savecpp_window.Get_Object(HEIGHT_OBJ).Get(Am_VALUE);

    main_group_rect.Set(MAIN_NAME, main_obj_name);
    main_group_rect.Set(C_FILENAME, c_file_name);
    main_group_rect.Set(CREATE_HEADER, create_header);
    main_group_rect.Set(H_FILENAME, h_file_name);
    main_group_rect.Set(WINDOW_OR_GROUP, create_window);
    main_group_rect.Set(WIN_TITLE, window_title);

    window.Set(Am_TITLE, window_title);

    main_group_rect.Set(FIXED_SIZE_OBJ, win_fixed_size);
    main_group_rect.Set(EXPLICIT_SIZE_OBJ, explicit_size);
    main_group_rect.Set(WIDTH_OBJ, width);
    main_group_rect.Set(HEIGHT_OBJ, height);

    int fill_key = main_group_rect.Get(FILL_STYLE_KEY);
    Am_Value_List top_level_objs_list = workgroup.Get(Am_GRAPHICAL_PARTS);
    if (create_header) {
      output_cc_with_header(h_out_file, cc_out_file, create_window,
                            top_level_objs_list, main_obj_name, fill_key,
                            window_title, explicit_size, width, height,
                            win_fixed_size);
      h_out_file.close();
      cout << "Wrote " << h_file_name << " and " << c_file_name << endl
           << flush;
    } else {
      output_cc_no_header(cc_out_file, create_window, top_level_objs_list,
                          main_obj_name, fill_key, window_title, explicit_size,
                          width, height, win_fixed_size);
      cout << "Wrote " << c_file_name << endl << flush;
    }
    cc_out_file.close();
  }
}

Am_Define_Method(Am_Object_Method, void, fillstyler, (Am_Object cmd))
{
  Am_Value_List l;
  Am_Value k;
  Am_Pop_Up_Window_And_Wait(Fill_Style_Panel, k, true);
  if (k != Am_No_Value) {
    int key = k;
    Am_Style s = n2s[key];

    Am_Object sel_widget;
    Am_Object main_group_rect;
    Am_Object undo_handler;
    Am_Value_List list_modified, old_values, new_values;

    Fill_Style_Button.Set(Am_FILL_STYLE, s);
    Fill_Style_Button.Set(FILL_STYLE_KEY, key);
    cmd.Note_Changed(Am_LABEL);

    for (Selection_Widget_List.Start(); !Selection_Widget_List.Last();
         Selection_Widget_List.Next()) {

      sel_widget = Selection_Widget_List.Get();
      l = sel_widget.Get(Am_VALUE);

      undo_handler =
          sel_widget.Get_Owner().Get_Owner().Get_Object(Am_UNDO_HANDLER);
      win.Set(Am_UNDO_HANDLER, undo_handler);

      if (!l.Empty()) {
        for (l.Start(); !l.Last(); l.Next()) {
          Am_Object am = l.Get();
          Am_Value v = am.Peek(Am_FILL_STYLE);
          if (v.Exists()) {
            list_modified.Add(am);
            old_values.Add(am.Get(FILL_STYLE_KEY));
            new_values.Add(key);
            am.Set(Am_FILL_STYLE, s).Set(FILL_STYLE_KEY, key);
          }
        }
        break;
      }
    }
    cmd.Set(Am_VALUE, new_values);
    cmd.Set(Am_OBJECT_MODIFIED, list_modified);
    cmd.Set(Am_OLD_VALUE, old_values);
  }
}

Am_Define_Method(Am_Object_Method, void, undo_fillstyler, (Am_Object cmd))
{
  Am_Object obj;
  Am_Value ov, nv;
  Am_Value_List obj_list = cmd.Get(Am_OBJECT_MODIFIED);
  Am_Value_List old_values = cmd.Get(Am_OLD_VALUE);
  Am_Value_List new_values = cmd.Get(Am_VALUE);

  for (obj_list.Start(), old_values.Start(), new_values.Start();
       !obj_list.Last();
       obj_list.Next(), old_values.Next(), new_values.Next()) {
    obj = obj_list.Get();
    nv = (int)old_values.Get();
    ov = (int)new_values.Get();
    obj.Set(FILL_STYLE_KEY, nv);
    new_values.Set(nv);
    old_values.Set(ov);
    obj.Set(Am_FILL_STYLE, n2s[(int)nv]);
  }
  Fill_Style_Button.Set(Am_FILL_STYLE, n2s[(int)nv]);
  Fill_Style_Button.Set(FILL_STYLE_KEY, (int)nv);
  cmd.Set(Am_VALUE, new_values);
  cmd.Set(Am_OBJECT_MODIFIED, obj_list);
  cmd.Set(Am_OLD_VALUE, old_values);
}

Am_Define_Method(Am_Object_Method, void, linestyler, (Am_Object cmd))
{
  Am_Value_List l;
  Am_Value v;
  Am_Value_List list_modified, old_values, new_values, vl, style_key;
  Am_Style s;
  Am_Object sel_widget, undo_handler;
  int color, thickness;
  bool flag = true;

  color = Line_Color_Button.Get(LINE_STYLE_KEY);
  thickness = Line_Thickness_Option_Button.Get(LINE_STYLE_KEY);

  if (cmd.Get_Object(Am_SAVED_OLD_OWNER) == Line_Thickness_Option_Button) {
    thickness = (int)cmd.Get_Object(Am_VALUE).Get(LINE_STYLE_KEY);
    s = n2l[color][thickness];
    Line_Thickness_Option_Button.Set(LINE_STYLE_KEY, thickness);
  } else {
    Am_Value tmp;
    Am_Pop_Up_Window_And_Wait(Line_Color_Panel, tmp, true);
    if (tmp != Am_No_Value) {
      color = (int)tmp;
      s = n2l[color][thickness];
      Line_Color_Button.Set(Am_FILL_STYLE, n2l[color][0]);
      Line_Color_Button.Set(LINE_STYLE_KEY, color);
      cmd.Note_Changed(Am_LABEL);
    } else
      flag = false;
  }

  if (flag) {
    style_key.Add(color).Add(thickness);

    for (Selection_Widget_List.Start(); !Selection_Widget_List.Last();
         Selection_Widget_List.Next()) {
      sel_widget = (Am_Object)Selection_Widget_List.Get();
      l = sel_widget.Get(Am_VALUE);
      if (!l.Empty()) {
        l.Start();
        if (!((Am_Object)l.Get()).Is_Instance_Of(main_group_rect_proto)) {
          // if background is selected, only one object is in the list
          undo_handler =
              sel_widget.Get_Owner().Get_Owner().Get_Object(Am_UNDO_HANDLER);
          win.Set(Am_UNDO_HANDLER, undo_handler);

          for (l.Start(); !l.Last(); l.Next()) {
            Am_Object am = l.Get();
            v = am.Peek(Am_LINE_STYLE);
            if (v.Exists()) {
              list_modified.Add(am);
              old_values.Add(am.Get(LINE_STYLE_KEY));
              new_values.Add(style_key);
              am.Set(Am_LINE_STYLE, s).Set(LINE_STYLE_KEY, style_key);
            }
          }
        } else
          Am_Abort_Widget(cmd);
        break;
      }
    }

    cmd.Set(Am_VALUE, new_values);
    cmd.Set(Am_OBJECT_MODIFIED, list_modified);
    cmd.Set(Am_OLD_VALUE, old_values);
  }
}

Am_Define_Method(Am_Object_Method, void, undo_linestyler, (Am_Object cmd))
{
  Am_Object obj;
  Am_Value_List nv, ov;
  Am_Value_List obj_list = cmd.Get(Am_OBJECT_MODIFIED);
  Am_Value_List old_values = cmd.Get(Am_OLD_VALUE);
  Am_Value_List new_values = cmd.Get(Am_VALUE);

  int c, t;

  for (obj_list.Start(), old_values.Start(), new_values.Start();
       !obj_list.Last();
       obj_list.Next(), old_values.Next(), new_values.Next()) {
    obj = obj_list.Get();
    nv = (Am_Value_List)old_values.Get();
    ov = new_values.Get();
    nv.Start();
    c = nv.Get();
    nv.Next();
    t = nv.Get();

    obj.Set(Am_LINE_STYLE, n2l[c][t]);
    new_values.Set(nv);
    obj.Set(LINE_STYLE_KEY, nv);
    old_values.Set(ov);
  }
  if (cmd.Is_Part_Of(Line_Color_Panel))
    Line_Color_Button.Set(Am_FILL_STYLE, n2l[c][0]);

  cmd.Set(Am_VALUE, new_values);
  cmd.Set(Am_OBJECT_MODIFIED, obj_list);
  cmd.Set(Am_OLD_VALUE, old_values);
}

Am_Define_Method(Am_Object_Method, void, fontstyler, (Am_Object cmd))
{
  Am_Value_List l;
  Am_Object vobj, undo_handler, sel_widget;
  Am_Value v;
  Am_Value_List list_modified, old_values, new_values, key;
  int type, size, style;
  Am_Font f;

  style = Font_Style_Button.Get(FONT_KEY);
  type = Font_Type_Button.Get(FONT_KEY);
  size = Font_Size_Button.Get(FONT_KEY);

  if (cmd.Is_Part_Of(Font_Style_Button)) {
    style = 0;
    char *k;
    Am_Value_List vl = cmd.Get(Am_VALUE);
    for (vl.Start(); !vl.Last(); vl.Next()) {
      k = (char *)((Am_String)vl.Get());
      if (*k == 'B')
        style += 1;
      else if (*k == 'I')
        style += 2;
      else
        style += 4;
    }
    f = fontarray[type][size][style];
    Font_Style_Button.Set(FONT_KEY, style);
  } else if (cmd.Is_Part_Of(Font_Type_Button)) {
    vobj = cmd.Get(Am_VALUE);
    type = vobj.Get(FONT_KEY);
    f = fontarray[type][size][style];
    Font_Type_Button.Set(FONT_KEY, type);
  } else {
    vobj = cmd.Get(Am_VALUE);
    size = vobj.Get(FONT_KEY);
    f = fontarray[type][size][style];
    Font_Size_Button.Set(FONT_KEY, size);
  }

  key.Add(type).Add(size).Add(style);
#if TEST
  list_modified.Add(text_test);
  old_values.Add(text_test.Get(FONT_KEY));
  new_values.Add(key);
  text_test.Set(Am_FONT, f);
#endif
  Am_Object am;

  for (Selection_Widget_List.Start(); !Selection_Widget_List.Last();
       Selection_Widget_List.Next()) {
    sel_widget = (Am_Object)Selection_Widget_List.Get();
    l = sel_widget.Get(Am_VALUE);

    if (!l.Empty()) {
      undo_handler =
          sel_widget.Get_Owner().Get_Owner().Get_Object(Am_UNDO_HANDLER);
      win.Set(Am_UNDO_HANDLER, undo_handler);

      for (l.Start(); !l.Last(); l.Next()) {
        am = l.Get();
        v = am.Peek(Am_FONT);
        if (v.Exists()) {
          list_modified.Add(am);
          old_values.Add(am.Get(FONT_KEY));
          new_values.Add(key);
          am.Set(Am_FONT, f);
          am.Set(FONT_KEY, key);
        }
      }
      break;
    }
  }
  cmd.Set(Am_VALUE, new_values);
  cmd.Set(Am_OBJECT_MODIFIED, list_modified);
  cmd.Set(Am_OLD_VALUE, old_values);
}

Am_Define_Method(Am_Object_Method, void, undo_fontstyler, (Am_Object cmd))
{
  Am_Object obj;
  Am_Value_List nv, ov;
  Am_Value_List obj_list = cmd.Get(Am_OBJECT_MODIFIED);
  Am_Value_List old_values = cmd.Get(Am_OLD_VALUE);
  Am_Value_List new_values = cmd.Get(Am_VALUE);

  for (obj_list.Start(), old_values.Start(), new_values.Start();
       !obj_list.Last();
       obj_list.Next(), old_values.Next(), new_values.Next()) {
    obj = obj_list.Get();
    nv = old_values.Get();
    ov = new_values.Get();
    obj.Set(Am_FONT, Get_Font_Style((Am_Value_List)nv));
    new_values.Set(nv);
    old_values.Set(ov);
    obj.Set(FONT_KEY, nv);
  }
  cmd.Set(Am_VALUE, new_values);
  cmd.Set(Am_OBJECT_MODIFIED, obj_list);
  cmd.Set(Am_OLD_VALUE, old_values);
}

Am_Define_Method(Am_Object_Method, void, aboutcmd, (Am_Object /* cmd */))
{
  Am_Value ok;
  Am_Pop_Up_Window_And_Wait(About_Gilt_Window, ok, true);
}

Am_Define_Method(Am_Object_Method, void, align_up, (Am_Object cmd))
{
  int minval = 1000000;
  Am_Value_List l = cmd.Get_Object(Am_SAVED_OLD_OWNER)
                        .Get_Owner()
                        .Get_Object(MAIN_GROUP)
                        .Get_Object(SEL_WIDGET)
                        .Get(Am_VALUE);
  Am_Value_List object_modified, new_value, old_value;

  for (l.Start(); !l.Last(); l.Next()) {
    Am_Object am = l.Get();
    int val = am.Get(Am_TOP);
    if (minval > val)
      minval = val;
  }
  for (l.Start(); !l.Last(); l.Next()) {
    Am_Object am = l.Get();
    object_modified.Add(am);
    old_value.Add(am.Get(Am_TOP));
    am.Set(Am_TOP, minval);
    new_value.Add(am.Get(Am_TOP));
  }
  cmd.Set(Am_VALUE, new_value);
  cmd.Set(Am_OBJECT_MODIFIED, object_modified);
  cmd.Set(Am_OLD_VALUE, old_value);
}

Am_Define_Method(Am_Object_Method, void, align_down, (Am_Object cmd))
{
  int minval = -10000;
  Am_Value_List l = cmd.Get_Object(Am_SAVED_OLD_OWNER)
                        .Get_Owner()
                        .Get_Object(MAIN_GROUP)
                        .Get_Object(SEL_WIDGET)
                        .Get(Am_VALUE);
  Am_Value_List object_modified, new_value, old_value;

  Am_Object am;
  int val;

  for (l.Start(); !l.Last(); l.Next()) {
    am = l.Get();
    val = am.Get(Am_TOP);
    if (minval < val)
      minval = val;
  }
  for (l.Start(); !l.Last(); l.Next()) {
    am = l.Get();
    object_modified.Add(am);
    old_value.Add(am.Get(Am_TOP));
    am.Set(Am_TOP, minval);
    new_value.Add(am.Get(Am_TOP));
  }
  cmd.Set(Am_VALUE, new_value);
  cmd.Set(Am_OBJECT_MODIFIED, object_modified);
  cmd.Set(Am_OLD_VALUE, old_value);
}

Am_Define_Method(Am_Object_Method, void, undo_vertical_align, (Am_Object cmd))
{
  Am_Value_List object_modified, new_value, old_value;
  Am_Object obj;
  object_modified = cmd.Get(Am_OBJECT_MODIFIED);
  new_value = cmd.Get(Am_VALUE);
  old_value = cmd.Get(Am_OLD_VALUE);
  int ov, nv;

  for (object_modified.Start(), old_value.Start(), new_value.Start();
       !object_modified.Last();
       object_modified.Next(), new_value.Next(), old_value.Next()) {
    obj = object_modified.Get();
    nv = (int)old_value.Get();
    ov = (int)new_value.Get();
    obj.Set(Am_TOP, nv);
    new_value.Set(nv);
    old_value.Set(ov);
  }
  cmd.Set(Am_VALUE, new_value);
  cmd.Set(Am_OBJECT_MODIFIED, object_modified);
  cmd.Set(Am_OLD_VALUE, old_value);
}

Am_Define_Method(Am_Object_Method, void, align_right, (Am_Object cmd))
{
  int minval = -1000000;
  Am_Value_List l = cmd.Get_Object(Am_SAVED_OLD_OWNER)
                        .Get_Owner()
                        .Get_Object(MAIN_GROUP)
                        .Get_Object(SEL_WIDGET)
                        .Get(Am_VALUE);
  Am_Value_List object_modified, new_value, old_value;

  for (l.Start(); !l.Last(); l.Next()) {
    Am_Object am = l.Get();
    int val = (int)am.Get(Am_LEFT) + (int)am.Get(Am_WIDTH);
    if (minval < val)
      minval = val;
  }
  for (l.Start(); !l.Last(); l.Next()) {
    Am_Object am = l.Get();
    object_modified.Add(am);
    old_value.Add(am.Get(Am_LEFT));
    am.Set(Am_LEFT, minval - (int)am.Get(Am_WIDTH));
    new_value.Add(am.Get(Am_LEFT));
  }
  cmd.Set(Am_VALUE, new_value);
  cmd.Set(Am_OBJECT_MODIFIED, object_modified);
  cmd.Set(Am_OLD_VALUE, old_value);
}

Am_Define_Method(Am_Object_Method, void, align_left, (Am_Object cmd))
{
  int minval = 1000000;
  Am_Value_List l = cmd.Get_Object(Am_SAVED_OLD_OWNER)
                        .Get_Owner()
                        .Get_Object(MAIN_GROUP)
                        .Get_Object(SEL_WIDGET)
                        .Get(Am_VALUE);
  Am_Value_List object_modified, old_value, new_value;

  for (l.Start(); !l.Last(); l.Next()) {
    Am_Object am = l.Get();
    int val = am.Get(Am_LEFT);
    if (minval > val)
      minval = val;
  }
  for (l.Start(); !l.Last(); l.Next()) {
    Am_Object am = l.Get();
    object_modified.Add(am);
    old_value.Add(am.Get(Am_LEFT));
    am.Set(Am_LEFT, minval);
    new_value.Add(am.Get(Am_LEFT));
  }
  cmd.Set(Am_VALUE, new_value);
  cmd.Set(Am_OBJECT_MODIFIED, object_modified);
  cmd.Set(Am_OLD_VALUE, old_value);
}

Am_Define_Method(Am_Object_Method, void, undo_horizontal_align, (Am_Object cmd))
{
  Am_Value_List object_modified, new_value, old_value;
  Am_Object obj;
  int ov, nv;
  object_modified = cmd.Get(Am_OBJECT_MODIFIED);
  new_value = cmd.Get(Am_VALUE);
  old_value = cmd.Get(Am_OLD_VALUE);

  for (object_modified.Start(), old_value.Start(), new_value.Start();
       !object_modified.Last();
       object_modified.Next(), new_value.Next(), old_value.Next()) {
    obj = object_modified.Get();
    nv = (int)old_value.Get();
    ov = (int)new_value.Get();
    obj.Set(Am_LEFT, nv);
    new_value.Set(nv);
    old_value.Set(ov);
  }
  cmd.Set(Am_VALUE, new_value);
  cmd.Set(Am_OBJECT_MODIFIED, object_modified);
  cmd.Set(Am_OLD_VALUE, old_value);
}

Am_Define_Method(Am_Object_Method, void, same_widths, (Am_Object cmd))
{
  int max_width = -1000000;
  Am_Value_List object_modified, new_value, old_value;
  Am_Value_List l = cmd.Get_Object(Am_SAVED_OLD_OWNER)
                        .Get_Owner()
                        .Get_Object(MAIN_GROUP)
                        .Get_Object(SEL_WIDGET)
                        .Get(Am_VALUE);

  Am_Object am;

  for (l.Start(); !l.Last(); l.Next()) {
    am = l.Get();
    int val = am.Get(Am_WIDTH);
    if (val > max_width)
      max_width = val;
  }
  for (l.Start(); !l.Last(); l.Next()) {
    am = l.Get();
    object_modified.Add(am);
    old_value.Add(am.Get(Am_WIDTH));
    am.Set(Am_WIDTH, max_width);
    new_value.Add(am.Get(Am_WIDTH));
  }
  cmd.Set(Am_VALUE, new_value);
  cmd.Set(Am_OBJECT_MODIFIED, object_modified);
  cmd.Set(Am_OLD_VALUE, old_value);
}

Am_Define_Method(Am_Object_Method, void, undo_same_widths, (Am_Object cmd))
{
  Am_Value_List object_modified, new_value, old_value;
  Am_Object obj;
  int ov, nv;
  object_modified = cmd.Get(Am_OBJECT_MODIFIED);
  new_value = cmd.Get(Am_VALUE);
  old_value = cmd.Get(Am_OLD_VALUE);

  for (object_modified.Start(), old_value.Start(), new_value.Start();
       !object_modified.Last();
       object_modified.Next(), new_value.Next(), old_value.Next()) {
    obj = object_modified.Get();
    nv = (int)old_value.Get();
    ov = (int)new_value.Get();
    obj.Set(Am_WIDTH, nv);
    new_value.Set(nv);
    old_value.Set(ov);
  }
  cmd.Set(Am_VALUE, new_value);
  cmd.Set(Am_OBJECT_MODIFIED, object_modified);
  cmd.Set(Am_OLD_VALUE, old_value);
}

Am_Define_Method(Am_Object_Method, void, same_heights, (Am_Object cmd))
{
  int max_height = -1000000;
  Am_Value_List object_modified, new_value, old_value;
  Am_Value_List l = cmd.Get_Object(Am_SAVED_OLD_OWNER)
                        .Get_Owner()
                        .Get_Object(MAIN_GROUP)
                        .Get_Object(SEL_WIDGET)
                        .Get(Am_VALUE);
  Am_Object am;

  for (l.Start(); !l.Last(); l.Next()) {
    am = l.Get();
    int val = am.Get(Am_HEIGHT);
    if (val > max_height)
      max_height = val;
  }
  for (l.Start(); !l.Last(); l.Next()) {
    am = l.Get();
    object_modified.Add(am);
    old_value.Add(am.Get(Am_HEIGHT));
    am.Set(Am_HEIGHT, max_height);
    new_value.Add(am.Get(Am_HEIGHT));
  }
  cmd.Set(Am_VALUE, new_value);
  cmd.Set(Am_OBJECT_MODIFIED, object_modified);
  cmd.Set(Am_OLD_VALUE, old_value);
}

Am_Define_Method(Am_Object_Method, void, undo_same_heights, (Am_Object cmd))
{
  Am_Value_List object_modified, new_value, old_value;
  Am_Object obj;
  int ov, nv;
  object_modified = cmd.Get(Am_OBJECT_MODIFIED);
  new_value = cmd.Get(Am_VALUE);
  old_value = cmd.Get(Am_OLD_VALUE);

  for (object_modified.Start(), old_value.Start(), new_value.Start();
       !object_modified.Last();
       object_modified.Next(), new_value.Next(), old_value.Next()) {
    obj = object_modified.Get();
    nv = (int)old_value.Get();
    ov = (int)new_value.Get();
    obj.Set(Am_HEIGHT, nv);
    new_value.Set(nv);
    old_value.Set(ov);
  }
  cmd.Set(Am_VALUE, new_value);
  cmd.Set(Am_OBJECT_MODIFIED, object_modified);
  cmd.Set(Am_OLD_VALUE, old_value);
}

Am_Define_Method(Am_Object_Method, void, go_up, (Am_Object cmd))
{
  Am_Value_List l = cmd.Get_Object(Am_SAVED_OLD_OWNER)
                        .Get_Owner()
                        .Get_Object(MAIN_GROUP)
                        .Get_Object(SEL_WIDGET)
                        .Get(Am_VALUE);
  Am_Value_List list_modified, old_values, new_values;

  for (l.Start(); !l.Last(); l.Next()) {
    Am_Object am = l.Get();
    list_modified.Add(am);
    old_values.Add(am.Get(Am_TOP));
    am.Set(Am_TOP, (int)am.Get(Am_TOP) - 1);
    new_values.Add(am.Get(Am_TOP));
  }

  cmd.Set(Am_VALUE, new_values);
  cmd.Set(Am_OLD_VALUE, old_values);
  cmd.Set(Am_OBJECT_MODIFIED, list_modified);
}

Am_Define_Method(Am_Object_Method, void, go_down, (Am_Object cmd))
{
  Am_Value_List l = cmd.Get_Object(Am_SAVED_OLD_OWNER)
                        .Get_Owner()
                        .Get_Object(MAIN_GROUP)
                        .Get_Object(SEL_WIDGET)
                        .Get(Am_VALUE);
  Am_Value_List list_modified, old_values, new_values;

  for (l.Start(); !l.Last(); l.Next()) {
    Am_Object am = l.Get();
    list_modified.Add(am);
    old_values.Add(am.Get(Am_TOP));
    am.Set(Am_TOP, (int)am.Get(Am_TOP) + 1);
    new_values.Add(am.Get(Am_TOP));
  }
  cmd.Set(Am_VALUE, new_values);
  cmd.Set(Am_OLD_VALUE, old_values);
  cmd.Set(Am_OBJECT_MODIFIED, list_modified);
}

Am_Define_Method(Am_Object_Method, void, undo_vertical_nudge, (Am_Object cmd))
{
  Am_Object obj;
  Am_Value_List obj_list = cmd.Get(Am_OBJECT_MODIFIED);
  Am_Value_List curr_values = cmd.Get(Am_VALUE);
  Am_Value_List old_values = cmd.Get(Am_OLD_VALUE);
  int nv, ov;

  for (obj_list.Start(), curr_values.Start(), old_values.Start();
       !obj_list.Last();
       obj_list.Next(), curr_values.Next(), old_values.Next()) {
    obj = obj_list.Get();
    nv = (int)old_values.Get();
    ov = (int)curr_values.Get();
    obj.Set(Am_TOP, nv);
    old_values.Set(ov);
    curr_values.Set(nv);
  }
  cmd.Set(Am_VALUE, curr_values);
  cmd.Set(Am_OBJECT_MODIFIED, obj_list);
  cmd.Set(Am_OLD_VALUE, old_values);
}

Am_Define_Method(Am_Object_Method, void, go_left, (Am_Object cmd))
{
  Am_Value_List l = cmd.Get_Object(Am_SAVED_OLD_OWNER)
                        .Get_Owner()
                        .Get_Object(MAIN_GROUP)
                        .Get_Object(SEL_WIDGET)
                        .Get(Am_VALUE);
  Am_Value_List list_modified, old_values, new_values;

  for (l.Start(); !l.Last(); l.Next()) {
    Am_Object am = l.Get();
    list_modified.Add(am);
    old_values.Add(am.Get(Am_LEFT));
    am.Set(Am_LEFT, (int)am.Get(Am_LEFT) - 1);
    new_values.Add(am.Get(Am_LEFT));
  }
  cmd.Set(Am_VALUE, new_values);
  cmd.Set(Am_OLD_VALUE, old_values);
  cmd.Set(Am_OBJECT_MODIFIED, list_modified);
}

Am_Define_Method(Am_Object_Method, void, go_right, (Am_Object cmd))
{
  Am_Value_List l = cmd.Get_Object(Am_SAVED_OLD_OWNER)
                        .Get_Owner()
                        .Get_Object(MAIN_GROUP)
                        .Get_Object(SEL_WIDGET)
                        .Get(Am_VALUE);
  Am_Value_List list_modified, old_values, new_values;

  for (l.Start(); !l.Last(); l.Next()) {
    Am_Object am = l.Get();
    list_modified.Add(am);
    old_values.Add(am.Get(Am_LEFT));
    am.Set(Am_LEFT, (int)am.Get(Am_LEFT) + 1);
    new_values.Add(am.Get(Am_LEFT));
  }

  cmd.Set(Am_VALUE, new_values);
  cmd.Set(Am_OLD_VALUE, old_values);
  cmd.Set(Am_OBJECT_MODIFIED, list_modified);
}

Am_Define_Method(Am_Object_Method, void, undo_horizontal_nudge, (Am_Object cmd))
{
  Am_Object obj;
  Am_Value_List obj_list = cmd.Get(Am_OBJECT_MODIFIED);
  Am_Value_List curr_values = cmd.Get(Am_VALUE);
  Am_Value_List old_values = cmd.Get(Am_OLD_VALUE);
  int nv, ov;

  for (obj_list.Start(), curr_values.Start(), old_values.Start();
       !obj_list.Last();
       obj_list.Next(), curr_values.Next(), old_values.Next()) {
    obj = obj_list.Get();
    nv = (int)old_values.Get();
    ov = (int)curr_values.Get();
    obj.Set(Am_LEFT, nv);
    old_values.Set(ov);
    curr_values.Set(nv);
  }

  cmd.Set(Am_VALUE, curr_values);
  cmd.Set(Am_OLD_VALUE, old_values);
  cmd.Set(Am_OBJECT_MODIFIED, obj_list);
}

Am_Define_Method(Am_Object_Method, void, go_into_special_mode,
                 (Am_Object /* cmd */))
{
  Am_Object sel_widget;

  for (Selection_Widget_List.Start(); !Selection_Widget_List.Last();
       Selection_Widget_List.Next()) {
    sel_widget = Selection_Widget_List.Get();
    sel_widget.Set(Am_VALUE, Am_Value_List()); //rest down by constraints
  }
}

Am_Define_Method(Am_Object_Method, void, fake_do_cancel, (Am_Object cmd))
{
  Am_String which_cmd = cmd.Get(Am_LABEL);
  Am_Beep();
  Am_Show_Alert_Dialog(Am_Value_List()
                           .Add(which_cmd)
                           .Add("button pressed in Run mode.")
                           .Add("Leaving Run Mode."));
  tool_panel.Set(Am_VALUE, ARROW_ID); //return to selection after each create
}

void
do_customize(Am_Object &cmd, Am_Object &obj)
{
  Am_Value v = obj.Peek(CUSTOMIZE_METHOD);
  if (v.Valid()) {
    Am_Customize_Object_Method method = v;
    method.Call(cmd, obj);
  }
  //this shouldn't happen:
  else
    AM_POP_UP_ERROR_WINDOW("Object " << obj << " cannot be customized");
}

Am_Define_Method(Am_Object_Method, void, customize, (Am_Object cmd))
{
  Am_Object owner = cmd.Get(Am_VALUE);
  if (owner.Valid())
    do_customize(cmd, owner);
}

Am_Define_Method(Am_Object_Method, void, customize_selected, (Am_Object cmd))
{
  Am_Object sel_widget = cmd.Get_Object(Am_SAVED_OLD_OWNER)
                             .Get_Owner()
                             .Get_Object(MAIN_GROUP)
                             .Get_Object(SEL_WIDGET);
  Am_Value_List selected = sel_widget.Get(Am_VALUE);
  if (selected.Empty())
    Am_Error("Nothing selected"); //not possible
  else if (selected.Length() > 1) {
    Am_Pop_Up_Error_Window("Can only get properties on one object at a time.");
    Am_Abort_Widget(cmd); //keep it out of the undo history
  } else {
    selected.Start();
    Am_Object obj = selected.Get();
    do_customize(cmd, obj);
  }
}

Am_Define_Method(Am_Customize_Object_Method, void, customize_name_only,
                 (Am_Object & cmd, Am_Object &owner))
{
  Am_String vs = owner.Get(Lw_NAME);
  Name_Only_Window.Get_Object(NAME_OBJ).Set(Am_VALUE, vs);
  Am_Value ok;
  Am_Pop_Up_Window_And_Wait(Name_Only_Window, ok, true);
  if (ok.Valid()) {
    Am_String new_vs = Name_Only_Window.Get_Object(NAME_OBJ).Get(Am_VALUE);
    fix_lw_name(new_vs);
    owner.Set(Lw_NAME, new_vs);
    cmd.Set(Am_OBJECT_MODIFIED, owner);
    cmd.Set(Am_SLOTS_TO_SAVE, Am_Value_List().Add(Lw_NAME), Am_OK_IF_NOT_THERE);
    cmd.Set(Am_OLD_VALUE, Am_Value_List().Add(vs));
    cmd.Set(Am_VALUE, Am_Value_List().Add(new_vs));
  } else
    Am_Abort_Widget(cmd); //keep out of undo history
}

Am_Define_Method(Am_Customize_Object_Method, void, customize_text,
                 (Am_Object & cmd, Am_Object &owner))
{
  Am_String ls = owner.Get(Am_TEXT);
  Am_String vs = owner.Get(Lw_NAME);
  Name_And_Label_Window.Get_Object(NAME_OBJ).Set(Am_VALUE, vs);
  Name_And_Label_Window.Get_Object(LABEL_OBJ).Set(Am_VALUE, ls);
  Am_Value ok;

  Am_Pop_Up_Window_And_Wait(Name_And_Label_Window, ok, true);
  if (ok.Valid()) {
    Am_String new_vs = Name_And_Label_Window.Get_Object(NAME_OBJ).Get(Am_VALUE);
    fix_lw_name(new_vs);
    owner.Set(Lw_NAME, new_vs);
    Am_String new_ls =
        Name_And_Label_Window.Get_Object(LABEL_OBJ).Get(Am_VALUE);
    owner.Set(Am_TEXT, new_ls);
    cmd.Set(Am_OBJECT_MODIFIED, owner);
    cmd.Set(Am_SLOTS_TO_SAVE, Am_Value_List().Add(Lw_NAME).Add(Am_TEXT),
            Am_OK_IF_NOT_THERE);
    cmd.Set(Am_OLD_VALUE, Am_Value_List().Add(vs).Add(ls));
    cmd.Set(Am_VALUE, Am_Value_List().Add(new_vs).Add(new_ls));
  } else
    Am_Abort_Widget(cmd); //keep out of undo history
}

Am_Define_Method(Am_Customize_Object_Method, void, customize_name_label,
                 (Am_Object & cmd, Am_Object &owner))
{
  Am_String ls = owner.Get_Object(Am_COMMAND).Get(Am_LABEL);
  Am_String vs = owner.Get(Lw_NAME);
  Name_And_Label_Window.Get_Object(NAME_OBJ).Set(Am_VALUE, vs);
  Name_And_Label_Window.Get_Object(LABEL_OBJ).Set(Am_VALUE, ls);
  Am_Value ok;
  if (owner.Get(Am_MINIMUM_HEIGHT) != 0) {
    owner.Set(Am_MINIMUM_WIDTH, 0);
    owner.Set(Am_MINIMUM_HEIGHT, 0);
  }

  Am_Pop_Up_Window_And_Wait(Name_And_Label_Window, ok, true);
  if (ok.Valid()) {
    Am_String new_vs = Name_And_Label_Window.Get_Object(NAME_OBJ).Get(Am_VALUE);
    fix_lw_name(new_vs);
    owner.Set(Lw_NAME, new_vs);
    Am_String new_ls =
        Name_And_Label_Window.Get_Object(LABEL_OBJ).Get(Am_VALUE);
    owner.Get_Object(Am_COMMAND).Set(Am_LABEL, new_ls);
    owner.Set(SUB_LABEL, new_ls);
    cmd.Set(Am_OBJECT_MODIFIED, owner);
    cmd.Set(Am_SLOTS_TO_SAVE, Am_Value_List().Add(Lw_NAME).Add(SUB_LABEL),
            Am_OK_IF_NOT_THERE);
    cmd.Set(Am_OLD_VALUE, Am_Value_List().Add(vs).Add(ls));
    cmd.Set(Am_VALUE, Am_Value_List().Add(new_vs).Add(new_ls));
  } else
    Am_Abort_Widget(cmd); //keep out of undo history
}

Am_Define_Method(Am_Customize_Object_Method, void, customize_name_and_image,
                 (Am_Object & cmd, Am_Object &owner))
{
  Am_Image_Array ls = owner.Get(Am_IMAGE);
  Am_String vs = owner.Get(Lw_NAME);
  Am_String fname = owner.Get(FILE_NAME);

  Name_And_Image_Window.Get_Object(NAME_OBJ).Set(Am_VALUE, vs);
  Name_And_Image_Window.Get_Object(LABEL_OBJ).Set(Am_VALUE, fname);

  Am_Value ok;
  Am_Image_Array new_image;
  int old_w, old_h;

  old_w = (int)owner.Get(Am_WIDTH);
  old_h = (int)owner.Get(Am_HEIGHT);

  Am_Pop_Up_Window_And_Wait(Name_And_Image_Window, ok, true);
  if (ok.Valid()) {
    Am_String new_vs = Name_And_Image_Window.Get_Object(NAME_OBJ).Get(Am_VALUE);
    fix_lw_name(new_vs);
    owner.Set(Lw_NAME, new_vs);
    Am_String new_ls =
        Name_And_Image_Window.Get_Object(LABEL_OBJ).Get(Am_VALUE);
    new_image = Am_Image_Array((char *)new_ls);
    owner.Set(Am_IMAGE, new_image);
    owner.Note_Changed(Am_IMAGE);
    owner.Set(FILE_NAME, new_ls);

    cmd.Set(Am_OBJECT_MODIFIED, owner);
    cmd.Set(Am_SLOTS_TO_SAVE, Am_Value_List().Add(Lw_NAME).Add(Am_IMAGE),
            Am_OK_IF_NOT_THERE);
    cmd.Set(Am_OLD_VALUE,
            Am_Value_List().Add(vs).Add(ls).Add(old_w).Add(old_h));
    cmd.Set(Am_VALUE, Am_Value_List()
                          .Add(new_vs)
                          .Add(new_image)
                          .Add((int)owner.Get(Am_WIDTH))
                          .Add((int)owner.Get(Am_HEIGHT)));
  } else
    Am_Abort_Widget(cmd); //keep out of undo history
}

Am_Define_Method(Am_Customize_Object_Method, void, customize_image_label,
                 (Am_Object & cmd, Am_Object &owner))
{
  Am_Value label = owner.Get_Object(Am_COMMAND).Get(Am_LABEL);
  Am_Value v, new_label;
  Am_Object source =
      Image_Label_Dialog.Get_Object(LABEL_OBJ).Get_Object(LABEL_OBJ);
  Am_Object name_o =
      Image_Label_Dialog.Get_Object(NAME_OBJ).Get_Object(NAME_OBJ);
  Am_Object image =
      Image_Label_Dialog.Get_Object(IMAGE_OBJ).Get_Object(IMAGE_OBJ);
  Am_Value_List command_list = Image_Label_Dialog.Get_Object(LABEL_OBJ)
                                   .Get_Object(CHOICE_PANEL)
                                   .Get(Am_ITEMS);
  Am_String vs, ls;
  Am_String new_vs;
  Am_Value_List graphics_list = image.Get(Am_GRAPHICAL_PARTS);
  Am_Object com;
  Am_String name = owner.Get(Lw_NAME);

  Image_Label_Dialog.Get_Object(NAME_OBJ).Get_Object(NAME_OBJ).Set(Am_VALUE,
                                                                   name);
  if (!graphics_list.Empty()) {
    graphics_list.Start();
    ((Am_Object)graphics_list.Get()).Destroy();
    graphics_list.Make_Empty();
    image.Set(Am_GRAPHICAL_PARTS, graphics_list);
  }

  if (Am_String::Test(label)) {
    Image_Label_Dialog.Set(FILE_NAME, "");
    source.Set(Am_VALUE, (Am_String)label);
    command_list.Start();
  } else {
    source.Set(Am_VALUE, "");
    command_list.End();
    if (((Am_Object)label).Is_Instance_Of(bitmap_proto))
      Image_Label_Dialog.Set(
          FILE_NAME, (char *)(Am_String)((Am_Object)label).Get(FILE_NAME));
    else
      Image_Label_Dialog.Set(FILE_NAME, "");
    ((Am_Object)label).Set(Am_TOP, 10).Set(Am_LEFT, 10);
    image.Add_Part(((Am_Object)label).Copy());
  }
  com = (Am_Object)command_list.Get();

  Image_Label_Dialog.Get_Object(LABEL_OBJ)
      .Get_Object(CHOICE_PANEL)
      .Set(Am_VALUE, com.Get(Am_LABEL));

  ((Am_Object_Method)com.Get(Am_DO_METHOD)).Call(com);

  vs = (Am_String)owner.Get(Lw_NAME);
  name_o.Set(Am_VALUE, vs);

  Am_Pop_Up_Window_And_Wait(Image_Label_Dialog, v, true);

  if (v.Valid()) {
    cmd.Set(Am_OLD_VALUE, Am_Value_List()
                              .Add(name)
                              .Add(label)
                              .Add(owner.Get(Am_WIDTH))
                              .Add(owner.Get(Am_HEIGHT)));
    cmd.Set(Am_OBJECT_MODIFIED, owner);

    new_vs = name_o.Get(Am_VALUE);
    fix_lw_name(new_vs);
    owner.Set(Lw_NAME, new_vs);

    if ((bool)source.Get(Am_ACTIVE)) {
      owner.Get_Object(Am_COMMAND)
          .Set(Am_LABEL, (new_label = (Am_String)source.Get(Am_VALUE)));
    } else {
      Am_Value_List l = image.Get(Am_GRAPHICAL_PARTS);
      if (!l.Empty()) {
        l.Start();
        new_label = ((Am_Object)l.Get()).Copy();
        owner.Get_Object(Am_COMMAND).Set(Am_LABEL, new_label);
      } else
        new_label = "";
    }
    cmd.Set(Am_VALUE, Am_Value_List()
                          .Add(new_vs)
                          .Add(new_label)
                          .Add(owner.Get(Am_WIDTH))
                          .Add(owner.Get(Am_HEIGHT)));
    cmd.Set(Am_SLOTS_TO_SAVE,
            Am_Value_List().Add(Lw_NAME).Add(SUB_LABEL).Add(Am_WIDTH).Add(
                Am_HEIGHT),
            Am_OK_IF_NOT_THERE);
  } else
    Am_Abort_Widget(cmd);
}

Am_Define_Method(Am_Customize_Object_Method, void, customize_number_input,
                 (Am_Object & cmd, Am_Object &owner))
{
  Am_String ls = owner.Get_Object(Am_COMMAND).Get(Am_LABEL);
  Am_String vs = owner.Get(Lw_NAME);
  Number_Input_Window.Get_Object(NAME_OBJ).Set(Am_VALUE, vs);
  Number_Input_Window.Get_Object(LABEL_OBJ).Set(Am_VALUE, ls);
  Am_Value old_val1 = owner.Get(Am_VALUE_1);
  Am_Value old_val2 = owner.Get(Am_VALUE_2);
  bool restricted = old_val1.Exists();
  int value1 = 0;
  int value2 = 0;
  if (restricted) {
    Number_Input_Window.Get_Object(RESTRICTED)
        .Set(Am_VALUE, Am_Value_List().Add(1));
    value1 = old_val1;
    value2 = owner.Get(Am_VALUE_2);
  } else
    Number_Input_Window.Get_Object(RESTRICTED).Set(Am_VALUE, Am_Value_List());
  Number_Input_Window.Get_Object(MIN_VALUE).Set(Am_VALUE, value1);
  Number_Input_Window.Get_Object(MAX_VALUE).Set(Am_VALUE, value2);

  Am_Value ok;
  Am_Pop_Up_Window_And_Wait(Number_Input_Window, ok, true);
  if (ok.Valid()) {
    Am_String new_vs = Number_Input_Window.Get_Object(NAME_OBJ).Get(Am_VALUE);
    fix_lw_name(new_vs);
    owner.Set(Lw_NAME, new_vs);
    Am_String new_ls = Number_Input_Window.Get_Object(LABEL_OBJ).Get(Am_VALUE);
    owner.Get_Object(Am_COMMAND).Set(Am_LABEL, new_ls);
    owner.Set(SUB_LABEL, new_ls);
    restricted = Number_Input_Window.Get_Object(MAX_VALUE).Get(Am_ACTIVE);
    Am_Value val1 = Am_No_Value;
    Am_Value val2 = Am_No_Value;
    if (restricted) {
      val1 = Number_Input_Window.Get_Object(MIN_VALUE).Get(Am_VALUE);
      val2 = Number_Input_Window.Get_Object(MAX_VALUE).Get(Am_VALUE);
    }
    owner.Set(Am_VALUE_1, val1);
    owner.Set(Am_VALUE_2, val2);

    cmd.Set(Am_OBJECT_MODIFIED, owner);
    cmd.Set(Am_SLOTS_TO_SAVE, Am_Value_List()
                                  .Add(Lw_NAME)
                                  .Add(SUB_LABEL)
                                  .Add(Am_VALUE_1)
                                  .Add(Am_VALUE_2),
            Am_OK_IF_NOT_THERE);
    cmd.Set(Am_OLD_VALUE,
            Am_Value_List().Add(vs).Add(ls).Add(old_val1).Add(old_val2));
    cmd.Set(Am_VALUE,
            Am_Value_List().Add(new_vs).Add(new_ls).Add(val1).Add(val2));
  } else
    Am_Abort_Widget(cmd); //keep out of undo history
}

Am_Define_Method(Am_Customize_Object_Method, void, customize_border_rect,
                 (Am_Object & cmd, Am_Object &owner))
{
  Am_String vs = owner.Get(Lw_NAME);
  Border_Rect_Window.Get_Object(NAME_OBJ).Set(Am_VALUE, vs);
  bool selected = owner.Get(Am_SELECTED);
  if (selected) {
    Border_Rect_Window.Get_Object(LOOKS_SELECTED_OBJ)
        .Set(Am_VALUE, Am_Value_List().Add(1));
  } else
    Border_Rect_Window.Get_Object(LOOKS_SELECTED_OBJ)
        .Set(Am_VALUE, Am_Value_List());

  Am_Value ok;
  Am_Pop_Up_Window_And_Wait(Border_Rect_Window, ok, true);
  if (ok.Valid()) {
    Am_String new_vs = Border_Rect_Window.Get_Object(NAME_OBJ).Get(Am_VALUE);
    fix_lw_name(new_vs);
    owner.Set(Lw_NAME, new_vs);
    Am_Value_List l =
        Border_Rect_Window.Get_Object(LOOKS_SELECTED_OBJ).Get(Am_VALUE);
    l.Start();
    bool new_selected = false;
    if (l.Member(1))
      new_selected = true;
    owner.Set(Am_SELECTED, new_selected);

    cmd.Set(Am_OBJECT_MODIFIED, owner);
    cmd.Set(Am_SLOTS_TO_SAVE, Am_Value_List().Add(Lw_NAME).Add(Am_SELECTED),
            Am_OK_IF_NOT_THERE);
    cmd.Set(Am_OLD_VALUE, Am_Value_List().Add(vs).Add(selected));
    cmd.Set(Am_VALUE, Am_Value_List().Add(new_vs).Add(new_selected));
  } else
    Am_Abort_Widget(cmd); //keep out of undo history
}

Am_Define_Method(Am_Customize_Object_Method, void, customize_scroll_group,
                 (Am_Object & cmd, Am_Object &owner))
{
  Am_String vs = owner.Get(Lw_NAME);
  int iw = owner.Get(Am_INNER_WIDTH);
  int ih = owner.Get(Am_INNER_HEIGHT);
  bool vsb = owner.Get(Am_V_SCROLL_BAR);
  bool hsb = owner.Get(Am_H_SCROLL_BAR);
  bool sbt = owner.Get(Am_H_SCROLL_BAR_ON_TOP);
  bool sbl = owner.Get(Am_V_SCROLL_BAR_ON_LEFT);

  Scroll_Group_Window.Get_Object(NAME_OBJ).Set(Am_VALUE, vs);
  Scroll_Group_Window.Get_Object(Am_INNER_WIDTH).Set(Am_VALUE, iw);
  Scroll_Group_Window.Get_Object(Am_INNER_HEIGHT).Set(Am_VALUE, ih);

  Am_Value_List l;
  if (vsb)
    l.Add(1);
  if (hsb)
    l.Add(2);
  if (sbt)
    l.Add(3);
  if (sbl)
    l.Add(4);
  Scroll_Group_Window.Get_Object(SCROLL_PROPS_OBJ).Set(Am_VALUE, l);

  Am_Value ok;
  Am_Pop_Up_Window_And_Wait(Scroll_Group_Window, ok, true);
  if (ok.Valid()) {
    Am_String new_vs = Scroll_Group_Window.Get_Object(NAME_OBJ).Get(Am_VALUE);
    int new_iw = Scroll_Group_Window.Get_Object(Am_INNER_WIDTH).Get(Am_VALUE);
    int new_ih = Scroll_Group_Window.Get_Object(Am_INNER_HEIGHT).Get(Am_VALUE);
    l = Scroll_Group_Window.Get_Object(SCROLL_PROPS_OBJ).Get(Am_VALUE);
    l.Start();
    bool new_vsb = l.Member(1);
    l.Start();
    bool new_hsb = l.Member(2);
    l.Start();
    bool new_sbt = l.Member(3);
    l.Start();
    bool new_sbl = l.Member(4);

    fix_lw_name(new_vs);
    owner.Set(Lw_NAME, new_vs);
    owner.Set(Am_INNER_WIDTH, new_iw);
    owner.Set(Am_INNER_HEIGHT, new_ih);
    owner.Set(Am_V_SCROLL_BAR, new_vsb);
    owner.Set(Am_H_SCROLL_BAR, new_hsb);
    owner.Set(Am_H_SCROLL_BAR_ON_TOP, new_sbt);
    owner.Set(Am_V_SCROLL_BAR_ON_LEFT, new_sbl);

    cmd.Set(Am_OBJECT_MODIFIED, owner);
    cmd.Set(Am_SLOTS_TO_SAVE, Am_Value_List()
                                  .Add(Lw_NAME)
                                  .Add(Am_INNER_WIDTH)
                                  .Add(Am_INNER_HEIGHT)
                                  .Add(Am_V_SCROLL_BAR)
                                  .Add(Am_H_SCROLL_BAR)
                                  .Add(Am_H_SCROLL_BAR_ON_TOP)
                                  .Add(Am_V_SCROLL_BAR_ON_LEFT),
            Am_OK_IF_NOT_THERE);
    cmd.Set(
        Am_OLD_VALUE,
        Am_Value_List().Add(vs).Add(iw).Add(ih).Add(vsb).Add(hsb).Add(sbt).Add(
            sbl));
    cmd.Set(Am_VALUE, Am_Value_List()
                          .Add(new_vs)
                          .Add(new_iw)
                          .Add(new_ih)
                          .Add(new_vsb)
                          .Add(new_hsb)
                          .Add(new_sbt)
                          .Add(new_sbl));
  } else
    Am_Abort_Widget(cmd); //keep out of undo history
}

Am_Define_Method(Am_Customize_Object_Method, void, customize_panels,
                 (Am_Object & cmd, Am_Object &owner))
{
  owner.Make_Unique(Am_ITEMS);
  Am_String name = owner.Peek(Lw_NAME);
  Am_Value_List il = ((Am_Value_List)owner.Get(Am_ITEMS));
  Am_Value layout_key = owner.Peek(LAYOUT_KEY);
  Am_Value box_on_left = owner.Peek(Am_BOX_ON_LEFT);
  Am_Value fw = owner.Peek(Am_FIXED_WIDTH);
  Am_Value hspace, vspace, maxrank;
  Am_Value_List slots_to_save = owner.Get(Am_SLOTS_TO_SAVE);
  slots_to_save.Start();
  if (slots_to_save.Member(Am_H_SPACING))
    hspace = owner.Get(Am_H_SPACING);
  slots_to_save.Start();
  if (slots_to_save.Member(Am_V_SPACING))
    vspace = owner.Get(Am_V_SPACING);
  slots_to_save.Start();
  if (slots_to_save.Member(Am_MAX_RANK))
    maxrank = owner.Get(Am_MAX_RANK);
  slots_to_save.Start();
  bool can_edit_items = slots_to_save.Member(Am_ITEMS);

  Am_String old_name = name;
  Am_Value_List old_il = il;
  Am_Value old_layout_key = layout_key;
  Am_Value old_box_on_left = box_on_left;
  Am_Value old_fw = fw;
  Am_Value old_hspace = hspace;
  Am_Value old_vspace = vspace;
  Am_Value old_maxrank = maxrank;

  if (paneldlg.Execute(name, il, layout_key, box_on_left, fw, hspace, vspace,
                       maxrank, can_edit_items)) {
    fix_lw_name(name);
    owner.Set(Lw_NAME, name).Set(Am_FIXED_WIDTH, fw);
    if (can_edit_items) {
      owner.Set(Am_ITEMS, il);
      owner.Note_Changed(Am_ITEMS);
    }
    if (layout_key.Exists()) {
      if ((int)layout_key == 1)
        owner.Set(LAYOUT_KEY, 1).Set(Am_LAYOUT, Am_Horizontal_Layout);
      else
        owner.Set(LAYOUT_KEY, 0).Set(Am_LAYOUT, Am_Vertical_Layout);
    }
    if (hspace.Exists())
      owner.Set(Am_H_SPACING, hspace);
    if (vspace.Exists())
      owner.Set(Am_V_SPACING, vspace);
    if (maxrank.Exists())
      owner.Set(Am_MAX_RANK, maxrank);

    cmd.Set(Am_OBJECT_MODIFIED, owner);
    Am_Value_List sl = Am_Value_List()
                           .Add(Lw_NAME)
                           .Add(Am_FIXED_WIDTH)
                           .Add(LAYOUT_KEY)
                           .Add(Am_H_SPACING)
                           .Add(Am_V_SPACING)
                           .Add(Am_MAX_RANK);
    Am_Value_List oldl = Am_Value_List()
                             .Add(old_name)
                             .Add(old_fw)
                             .Add(old_layout_key)
                             .Add(old_hspace)
                             .Add(old_vspace)
                             .Add(old_maxrank);
    Am_Value_List newl = Am_Value_List()
                             .Add(name)
                             .Add(fw)
                             .Add(layout_key)
                             .Add(hspace)
                             .Add(vspace)
                             .Add(maxrank);
    if (can_edit_items) {
      sl.Add(Am_ITEMS);
      oldl.Add(old_il);
      newl.Add(il);
    }
    cmd.Set(Am_SLOTS_TO_SAVE, sl, Am_OK_IF_NOT_THERE);
    cmd.Set(Am_OLD_VALUE, oldl);
    cmd.Set(Am_VALUE, newl);
  } else
    Am_Abort_Widget(cmd); //keep out of undo history
}

Am_Define_Method(Am_Object_Method, void, undo_set_properties, (Am_Object cmd))
{
  Am_Value_List old_values = cmd.Get(Am_OLD_VALUE);
  Am_Value_List new_values = cmd.Get(Am_VALUE);
  Am_Value_List slots = cmd.Get(Am_SLOTS_TO_SAVE);
  Am_Object object = cmd.Get(Am_OBJECT_MODIFIED);

  Am_Slot_Key slot;
  Am_Value old_val;
  for (slots.Start(), old_values.Start(); !slots.Last();
       slots.Next(), old_values.Next()) {
    slot = (int)slots.Get();
    old_val = old_values.Get();

    object.Set(slot, old_val);
    // some slots need special handling as well
    if (slot == Am_ITEMS) {
      object.Note_Changed(Am_ITEMS);
    } else if (slot == LAYOUT_KEY) {
      if ((int)old_val == 1)
        object.Set(Am_LAYOUT, Am_Horizontal_Layout);
      else
        object.Set(Am_LAYOUT, Am_Vertical_Layout);
    } else if (slot == SUB_LABEL) {
      object.Get_Object(Am_COMMAND).Set(Am_LABEL, old_val);
    }
  }
  //now swap old and new in case redo
  cmd.Set(Am_VALUE, old_values);
  cmd.Set(Am_OLD_VALUE, new_values);
}

Am_Object
create_new_obj_internal(Am_Object &prototype, Am_Inter_Location &data,
                        Am_Object workgroup)
{
  bool set_width_height = (int)prototype.Get(Am_HOW_MANY_POINTS) == 2;

  Am_Object ref_obj;
  int a, b, c, d;
  bool create_line;
  data.Get_Location(create_line, ref_obj, a, b, c, d);

  if (ref_obj != workgroup) {
    Am_Translate_Coordinates(ref_obj, a, b, workgroup, a, b);
    if (create_line)
      Am_Translate_Coordinates(ref_obj, c, d, workgroup, c, d);
  }

  /*
  Am_Object fill_color_obj = fill_color_panel.Get(Am_VALUE);
  Am_Style fill_color = fill_color_obj.Get(Am_FILL_STYLE);
  
  Am_Object line_color_obj = line_color_panel.Get(Am_VALUE);
  Am_Style line_color = line_color_obj.Get(Am_FILL_STYLE);
  */

  Am_Object new_obj;
  if (create_line)
    new_obj = prototype
                  .Create()
                  // .Set (Am_LINE_STYLE, line_color)
                  .Set(Am_X1, a)
                  .Set(Am_Y1, b)
                  .Set(Am_X2, c)
                  .Set(Am_Y2, d)
                  .Set(Lw_NAME, "");
  else if (prototype.Valid()) {
    new_obj = prototype
                  .Create()
                  // .Set (Am_LINE_STYLE, line_color)
                  // .Set (Am_FILL_STYLE, fill_color)
                  .Set(Am_LEFT, a)
                  .Set(Am_TOP, b)
                  .Set(Lw_NAME, "");
    if (set_width_height) {
      new_obj.Set(Am_WIDTH, c).Set(Am_HEIGHT, d);
    }
  } else
    Am_Error("No Prototype");

  workgroup.Add_Part(new_obj);
  if (CHANGE_TO_ARROW)
    tool_panel.Set(Am_VALUE, ARROW_ID); //return to selection after each create
  return new_obj;
}

// Am_Create_New_Object_Proc for new object command
Am_Define_Method(Am_Create_New_Object_Method, Am_Object, create_new_object,
                 (Am_Object inter, Am_Inter_Location data,
                  Am_Object old_object))
{
  Am_Object new_obj;
  Am_Object workgroup = inter.Get_Owner();

  if (old_object.Valid()) {
    Am_Error("Should not be an old object since no selective redo");
    /* 
    new_obj = old_object.Copy();
    workgroup.Add_Part (new_obj);
    */
    return Am_No_Object;
  } else {
    Am_Object proto = tool_panel.Get(Am_VALUE);
    if (proto != proto_id && !CHANGE_TO_ARROW)
      CHANGE_TO_ARROW = true; // if mode has been changed
    new_obj = create_new_obj_internal(proto, data, workgroup);
    Am_Value v = new_obj.Peek(Am_FILL_STYLE);

    if (v.Exists()) {
      new_obj.Set(Am_FILL_STYLE, Fill_Style_Button.Get(Am_FILL_STYLE));
      new_obj.Set(FILL_STYLE_KEY,
                  Fill_Style_Button.Get(FILL_STYLE_KEY, Am_OK_IF_NOT_THERE));
    }
    v = new_obj.Peek(Am_LINE_STYLE);
    if (v.Exists()) {
      Am_Value_List key_list;
      int color, thickness;
      color = Line_Color_Button.Get(LINE_STYLE_KEY);
      thickness = Line_Thickness_Option_Button.Get(LINE_STYLE_KEY);
      new_obj.Set(Am_LINE_STYLE, n2l[color][thickness]);
      new_obj.Set(LINE_STYLE_KEY, key_list.Add(color).Add(thickness),
                  Am_OK_IF_NOT_THERE);
    }
    v = new_obj.Peek(Am_FONT);
    if (v.Exists()) {
      int type, size, style;
      Am_Value_List key_list;
      type = Font_Type_Button.Get(FONT_KEY);
      size = Font_Size_Button.Get(FONT_KEY);
      style = Font_Style_Button.Get(FONT_KEY);
      new_obj.Set(Am_FONT, fontarray[type][size][style]);
      new_obj.Set(FONT_KEY, key_list.Add(type).Add(size).Add(style),
                  Am_OK_IF_NOT_THERE);
    }

    return new_obj;
  }
}

Am_Define_Method(Am_Object_Method, void, stick_to_this_mode, (Am_Object inter))
{
  Am_Object panel = inter.Get_Owner();

  proto_id = panel.Get(Am_VALUE);
  CHANGE_TO_ARROW = false;
}

void
Create_Tool_Panel()
{
  //don't want changing the mode to go onto the undo history
  Am_Object undoable_cmd =
      Am_Command.Create().Set(Am_IMPLEMENTATION_PARENT, true);
  Am_Value_List l;
  l.Add(undoable_cmd.Create().Set(Am_LABEL, arrow_bitmap).Set(Am_ID, ARROW_ID))
      .Add(undoable_cmd.Create()
               .Set(Am_LABEL, "Run")
               .Set(Am_DO_METHOD, go_into_special_mode)
               .Set(Am_ID, RUN_ID))
      .Add(undoable_cmd.Create()
               .Set(Am_LABEL, Am_Rectangle.Create()
                                  .Set(Am_FILL_STYLE, Am_Red)
                                  .Set(Am_LINE_STYLE, Am_Black)
                                  .Set(Am_WIDTH, 30)
                                  .Set(Am_HEIGHT, 15))
               .Set(Am_ID, rectangle_proto))
      .Add(undoable_cmd.Create()
               .Set(Am_LABEL, Am_Line.Create()
                                  .Set(Am_LINE_STYLE, Am_Red)
                                  .Set(Am_X1, 0)
                                  .Set(Am_Y1, 0)
                                  .Set(Am_X2, 30)
                                  .Set(Am_Y2, 15))
               .Set(Am_ID, line_proto));
  l.Add(undoable_cmd.Create()
            .Set(Am_LABEL, Am_Arc.Create()
                               .Set(Am_FILL_STYLE, Am_Yellow)
                               .Set(Am_LINE_STYLE, Am_Black)
                               .Set(Am_WIDTH, 20)
                               .Set(Am_HEIGHT, 10))
            .Set(Am_ID, arc_proto))
      .Add(undoable_cmd.Create()
               .Set(Am_LABEL,
                    Am_Polygon.Create()
                        .Set(Am_POINT_LIST, Am_Point_List()
                                                // an irregular closed polygon
                                                .Add(15.0f, 0.0f)
                                                .Add(0.0f, 20.0f)
                                                .Add(30.0f, 40.0f)
                                                .Add(30.0f, 20.0f)
                                                .Add(15.0f, 20.0f)
                                                .Add(10.0f, 0.0f))
                        .Set(Am_WIDTH, 16)
                        .Set(Am_HEIGHT, 10)
                        .Set(Am_LINE_STYLE, Am_Black)
                        .Set(Am_FILL_STYLE, Am_Green))
               .Set(Am_ID, POLYGON_ID))
      .Add(undoable_cmd.Create()
               .Set(Am_LABEL, "Constraint")
               .Set(Am_DO_METHOD, go_into_special_mode)
               .Set(Am_ID, CONSTRAINT_ID))
      .Add(undoable_cmd.Create().Set(Am_LABEL, "Text").Set(Am_ID, text_proto));

  l.Add(undoable_cmd.Create()
            .Set(Am_LABEL, "Border")
            .Set(Am_ID, border_rectangle_proto))
      .Add(undoable_cmd.Create()
               .Set(Am_LABEL, "Button")
               .Set(Am_ID, button_proto))
      .Add(undoable_cmd.Create()
               .Set(Am_LABEL, "Buttons")
               .Set(Am_ID, buttons_proto))
      .Add(
          undoable_cmd.Create().Set(Am_LABEL, "OK-Cancel").Set(Am_ID, ok_proto))
      .Add(undoable_cmd.Create()
               .Set(Am_LABEL, "Option")
               .Set(Am_ID, option_proto))
      .Add(undoable_cmd.Create()
               .Set(Am_LABEL, "Checkboxes")
               .Set(Am_ID, checkbox_proto))
      .Add(
          undoable_cmd.Create().Set(Am_LABEL, "Radios").Set(Am_ID, radio_proto))
      .Add(undoable_cmd.Create()
               .Set(Am_LABEL, "Text Input")
               .Set(Am_ID, text_input_proto))
      .Add(undoable_cmd.Create()
               .Set(Am_LABEL, "Number Input")
               .Set(Am_ID, number_input_proto))
      .Add(undoable_cmd.Create()
               .Set(Am_LABEL, "Scroll Group")
               .Set(Am_ID, scroll_group_proto))
      .Add(undoable_cmd.Create()
               .Set(Am_LABEL, "Bitmap")
               .Set(Am_ID, bitmap_proto));

  tool_panel = Am_Button_Panel.Create("tool panel")
                   .Set(Am_LEFT, 10)
                   .Set(Am_TOP, 40)
                   .Set(Am_FIXED_HEIGHT, false)
                   .Set(Am_FIXED_WIDTH, false)
                   .Set(Am_ITEM_OFFSET, -2)
                   .Set(Am_FINAL_FEEDBACK_WANTED, true)
                   .Set(Am_ITEMS, l)
                   .Set(Am_H_SPACING, 2)
                   .Set(Am_LAYOUT, Am_Horizontal_Layout)
                   .Set(Am_MAX_SIZE, 100)
                   .Set(Am_VALUE, ARROW_ID)
                   .Add_Part(Am_One_Shot_Interactor.Create()
                                 .Set(Am_PRIORITY, 2.0)
                                 .Set(Am_START_WHEN, "DOUBLE_LEFT_DOWN")
                                 .Set(Am_START_WHERE_TEST, Am_Inter_In_Part)
                                 .Set(Am_DO_METHOD, stick_to_this_mode));
  Am_Value_List parts = tool_panel.Get(Am_GRAPHICAL_PARTS);
  parts.Start();
  Am_Object obj, label_obj;
  for (int i = 0; i < 6; i++) {
    obj = parts.Get();
    obj.Set(Am_WIDTH, 31);
    obj.Set(Am_HEIGHT, 20);
    parts.Next();
  }
}

#if defined(_WINDOWS) || defined(_MACINTOSH)
#define ARROW_BITMAP "lib/images/arrow.gif"
#else
#define ARROW_BITMAP "lib/images/arrow.xbm"
#endif

Am_Define_Formula(bool, look_is_me)
{
  Am_Value my_look = self.Get(Am_WIDGET_LOOK);
  Am_Value current_look = Am_Screen.Get(Am_WIDGET_LOOK);
  return my_look == current_look;
}

Am_Define_Method(Am_Object_Method, void, set_look, (Am_Object cmd))
{
  Am_Value my_look = cmd.Get(Am_WIDGET_LOOK);
  Am_Set_Default_Look(my_look);
}

////////////////////////////////////////////////////////////////////////////
// Polygon stuff
////////////////////////////////////////////////////////////////////////////

Am_Object partial_poly; // polygon under construction

Am_Define_Method(Am_Mouse_Event_Method, void, polygon_start,
                 (Am_Object inter, int /* x */, int /* y */,
                  Am_Object /*event_window*/, Am_Input_Char /* ic */))
{
  // start the polygon
  partial_poly = polygon_proto.Create().Set(Am_POINT_LIST, Am_Point_List());

  inter.Get_Owner().Add_Part(partial_poly);

  // show feedback
  Am_Object feedback(inter.Get(Am_FEEDBACK_OBJECT));
  if (feedback.Valid()) {
    feedback.Set(Am_VISIBLE, true);
    Am_To_Top(feedback);
    // feedback position will be set by polygon_interim_do
  }
}

Am_Define_Method(Am_Mouse_Event_Method, void, polygon_interim_do,
                 (Am_Object inter, int x, int y, Am_Object event_window,
                  Am_Input_Char ic))
{
  Am_Object workgroup = inter.Get_Owner();

  if (event_window != workgroup)
    Am_Translate_Coordinates(event_window, x, y, workgroup, x, y);

  // move endpoint of feedback line
  Am_Object feedback = inter.Get(Am_FEEDBACK_OBJECT);
  if (feedback.Valid()) {
    bool feedback_vis = feedback.Get(Am_VISIBLE);
    if (feedback_vis == false) {
      feedback.Set(Am_VISIBLE, true);
    }
    feedback.Set(Am_X2, x);
    feedback.Set(Am_Y2, y);
  }

  static Am_Input_Char vertex_event("any_left_down");
  if (ic == vertex_event) {
    // user has left-clicked!
    if (!Am_Point_In_All_Owners(partial_poly, x, y, workgroup)) {
      // clicked outside of drawing window -- throw away this point and stop
      Am_Stop_Interactor(inter, Am_No_Object, Am_No_Input_Char, Am_No_Object, 0,
                         0);
    } else {
      Am_Point_List pl = partial_poly.Get(Am_POINT_LIST);
      if (pl.Empty()) {
        // the click that started it all -- first point of the polygon
        partial_poly.Set(Am_POINT_LIST, pl.Add((float)x, (float)y));
        feedback.Set(Am_X1, x);
        feedback.Set(Am_Y1, y);
      } else {
        int first_x;
        int first_y;
        pl.Start();
        pl.Get(first_x, first_y);

        int delta_x = x - first_x;
        int delta_y = y - first_y;

        if (delta_x < 5 && delta_x > -5 && delta_y < 5 && delta_y > -5) {
          // clicked on (er, near) the initial point again -- close
          // the polygon and stop
          pl.Add((float)first_x, (float)first_y, Am_TAIL, false);
          Am_Stop_Interactor(inter, Am_No_Object, Am_No_Input_Char,
                             Am_No_Object, 0, 0);
        } else {
          // add new point to polygon, reset feedback origin to it, and
          // keep running
          pl.Add((float)x, (float)y, Am_TAIL, false);
          feedback.Set(Am_X1, x);
          feedback.Set(Am_Y1, y);
        }
        partial_poly.Note_Changed(Am_POINT_LIST);
      }
    }
  }
}

Am_Define_Method(Am_Mouse_Event_Method, void, polygon_abort,
                 (Am_Object inter, int x, int y, Am_Object event_window,
                  Am_Input_Char /* ic */))
{
  Am_Object workgroup = inter.Get_Owner();

  if (event_window != workgroup)
    Am_Translate_Coordinates(event_window, x, y, workgroup, x, y);

  // hide feedback
  Am_Object feedback;
  feedback = inter.Get(Am_FEEDBACK_OBJECT);
  if (feedback.Valid())
    feedback.Set(Am_VISIBLE, false);

  // destroy polygon under construction
  partial_poly.Destroy();
}

Am_Define_Method(Am_Mouse_Event_Method, void, polygon_do,
                 (Am_Object inter, int /*x*/, int /*y*/,
                  Am_Object /*event_window*/, Am_Input_Char /* ic */))
{
  // hide feedback
  Am_Object workgroup = inter.Get_Owner();
  Am_Object feedback;
  feedback = inter.Get(Am_FEEDBACK_OBJECT);
  if (feedback.Valid())
    feedback.Set(Am_VISIBLE, false);

  Am_Object new_object = partial_poly;

  // take care of undo/redo
  inter.Set(Am_VALUE, new_object);
  inter.Set(Am_OBJECT_MODIFIED, new_object);
  Am_Copy_Values_To_Command(inter);

  tool_panel.Set(Am_VALUE, ARROW_ID); //return to selection after each create
}

Am_Define_Method(Am_Object_Method, void, close_window, (Am_Object cmd))
{
  Am_Object window = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  Am_Object s = window.Get_Object(MAIN_GROUP).Get(SEL_WIDGET);
  Am_Object current;

  Selection_Widget_List.Start();
  current = Selection_Widget_List.Get();
  while (current != s && !Selection_Widget_List.Last()) {
    Selection_Widget_List.Next();
    current = Selection_Widget_List.Get();
  }

  Selection_Widget_List.Delete();
  main_window_proto.Get_Object(MAIN_GROUP)
      .Get_Object(SEL_WIDGET)
      .Set(Am_MULTI_SELECTIONS, Selection_Widget_List);

  for (window_list.Start(); !window_list.Last(); window_list.Next()) {
    current = window_list.Get();
    if (current == window) {
      window_list.Delete();
      break;
    }
  }
  window.Get_Object(MAIN_GROUP)
      .Get_Object(MAIN_GROUP_RECT)
      .Set(Am_FILL_STYLE, Am_White);
  Am_Value_List gl = window.Get_Object(MAIN_GROUP)
                         .Get_Object(WORK_GROUP)
                         .Get(Am_GRAPHICAL_PARTS);
  for (gl.Start(); !gl.Last(); gl.Next())
    ((Am_Object)gl.Get()).Destroy();
  closed_window_list.Add(window);
  window.Set(Am_VISIBLE, false);
}

Am_Define_Method(Am_Object_Method, void, my_destroy_window_method,
                 (Am_Object window))
{
  Am_Object s = window.Get_Object(MAIN_GROUP).Get(SEL_WIDGET);
  Am_Object current;

  Selection_Widget_List.Start();
  current = Selection_Widget_List.Get();
  while (current != s && !Selection_Widget_List.Last()) {
    Selection_Widget_List.Next();
    current = Selection_Widget_List.Get();
  }

  for (window_list.Start(); !window_list.Last(); window_list.Next()) {
    current = window_list.Get();
    if (current == window) {
      window_list.Delete();
      break;
    }
  }

  Selection_Widget_List.Delete();
  main_window_proto.Get_Object(MAIN_GROUP)
      .Get_Object(SEL_WIDGET)
      .Set(Am_MULTI_SELECTIONS, Selection_Widget_List);
  Am_Default_Window_Destroy_Method.Call(window);
}

Am_Define_Object_Formula(get_workgroup)
{
  return self.Get_Owner().Get_Object(WORK_GROUP);
}

Am_Define_Object_Formula(get_sel_widget)
{
  return self.Get_Owner().Get_Object(MAIN_GROUP).Get_Object(SEL_WIDGET);
}

Am_Define_Formula(int, bitmap_minimum_width)
{
  Am_Value v = self.Get(Am_WIDTH);
  if (v.Valid())
    return (int)v;
  else
    return 10;
}

Am_Define_Formula(int, bitmap_minimum_height)
{
  Am_Value v = self.Get(Am_HEIGHT);
  if (v.Valid())
    return (int)v;
  else
    return 10;
}

Am_Define_Method(Am_Object_Method, void, enable_menu_edit_window,
                 (Am_Object cmd))
{
  Am_Object window = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  int top = ((int)window.Get(Am_TOP)) + 40;
  int left = ((int)window.Get(Am_LEFT)) + 40;
  Am_Object work_group = window.Get_Object(MAIN_GROUP).Get_Object(WORK_GROUP);
  Am_Value_List l = work_group.Get(Am_GRAPHICAL_PARTS);
  char string[100];

  Outline *ol = (Outline *)(Am_Ptr)Menu_Edit_Window.Get(OUTLINE);
  Am_Object root = ol->Get_Root();
  Am_Object lgroup = Menu_Edit_Window.Get_Object(Lw_GROUP).Get_Object(Lw_GROUP);

  Am_Object menu = Am_No_Object;

  for (l.Start(); !l.Last(); l.Next()) {
    if (((Am_Object)l.Get()).Is_Instance_Of(menu_proto)) {
      menu = (Am_Object)l.Get();
      break;
    }
  }

  root = ol->Get_Root();
  Am_Value_List children = root.Get(Lw_CHILDREN);

  for (children.Start(); !children.Last(); children.Next())
    ol->RemoveNode((int)((Am_Object)children.Get()).Get(Lw_NODEID));

  sprintf(string, "Menu Edit: %s", (char *)(Am_String)window.Get(Am_TITLE));
  Menu_Edit_Window.Set(Am_TITLE, string).Set(Am_TOP, top).Set(Am_LEFT, left);

  if (menu != Am_No_Object) {
    Am_Value_List l1 = menu.Get(Am_ITEMS);
    Am_Value_List l2;
    Am_Object node;
    int i = 2, p;
    int key;

    for (l1.Start(); !l1.Last(); l1.Next()) {
      node = (Am_Object)l1.Get();
      key = node.Get(COMMAND_KEY);
      lgroup.Add_Part(
          ol->AddNode(i, 1, node_proto.Create()
                                .Get_Object(TEXT1)
                                .Set(Am_TEXT, (Am_String)node.Get(Am_LABEL))
                                .Get_Owner()
                                .Get_Object(TEXT2)
                                .Set(Am_TEXT, (Am_String)n2cr[key])
                                .Set(COMMAND_KEY, key)
                                .Get_Owner()));
      l2 = node.Get(Am_ITEMS);
      p = i;
      i++;
      for (l2.Start(); !l2.Last(); l2.Next()) {
        node = (Am_Object)l2.Get();
        key = node.Get(COMMAND_KEY);
        lgroup.Add_Part(
            ol->AddNode(i, p, node_proto.Create()
                                  .Get_Object(TEXT1)
                                  .Set(Am_TEXT, (Am_String)node.Get(Am_LABEL))
                                  .Get_Owner()
                                  .Get_Object(TEXT2)
                                  .Set(Am_TEXT, (Am_String)n2cr[key])
                                  .Set(COMMAND_KEY, key)
                                  .Get_Owner()));
        i++;
      }
    }

    Menu_Edit_Window.Get_Object(NEW_DELETE_PANNEL)
        .Set(NUMBER_OF_NODES, (int)(i - 1));
  }

  Am_Object ci = lgroup.Get(CHOICE_INTER);
  ci.Set(Am_VALUE, ol->Get_Root());
  ci = Menu_Edit_Window.Get_Object(COMMAND_KEY)
           .Get_Object(COMMAND_LIST)
           .Get_Object(CHOICE_INTER);
  ci.Set(Am_VALUE, first_command);

  Am_Value v;
  Am_Pop_Up_Window_And_Wait(Menu_Edit_Window, v, true);
  if (v != Am_No_Object) {
    if (menu != Am_No_Object) {
      l.Delete();
      menu.Destroy();
    }
    work_group.Add_Part((Am_Object)v);
  }
}

Am_Define_String_Formula(how_many_windows)
{
  char string[100];
  sprintf(string, " NoName %d", ++window_count);
  return (Am_String)string;
}

Am_Define_String_Formula(get_title)
{
  Am_Value v =
      self.Get_Object(MAIN_GROUP).Get_Object(MAIN_GROUP_RECT).Peek(WIN_TITLE);

  if (v.Valid())
    return (Am_String)v;
  else
    return (Am_String) "Amulet";
}

Am_Define_Object_Formula(return_lfeedback)
{
  return self.Get_Owner().Get_Sibling(L_FEEDBACK);
}

Am_Define_Method(Am_Object_Method, void, out_of_current_mode, (Am_Object cmd))
{
  cmd.Set(Am_OLD_VALUE, tool_panel.Get(Am_VALUE));
  if (!CHANGE_TO_ARROW)
    CHANGE_TO_ARROW = true;
  tool_panel.Set(Am_VALUE, ARROW_ID);
  cmd.Set(Am_VALUE, ARROW_ID);
  cmd.Set(Am_OBJECT_MODIFIED, tool_panel);
}

Am_Define_Method(Am_Object_Method, void, undo_out_of_current_mode,
                 (Am_Object cmd))
{
  Am_Value i = (int)cmd.Get(Am_OLD_VALUE);
  cmd.Set(Am_OLD_VALUE, cmd.Get(Am_VALUE));
  tool_panel.Set(Am_VALUE, i);
  cmd.Set(Am_VALUE, i);
  cmd.Set(Am_OBJECT_MODIFIED, tool_panel);
}

/*
Am_Define_Formula(int, compute_arrowline_top){
    int x1 = self.Get(Am_X1)
    int x2 = self.Get(Am_X2)
    int y1 = self.Get(Am_Y1)
    int y2 = self.Get(Am_Y2)    

    bool upside_down = (y1 > y2) ? true : false;
    bool rightside_left = (x1 > x2) ? true : false;
    
    self.Set(Am_LEFT, (rightside_left) ? x2 : x1);
    return (upside_down) ? y2 : y1;
}    


Am_Define_Formula(int, rotate_p1){
    double x1 = (double)self.Get(AmX2);
    double y1 = (double)self.Get(Am_Y2) + 4;
    Am_Object owner = self.Get_Owner();

    double length = sqrt(pow((double)owner.Get(Am_WIDTH), 2.0) 
                        + pow((double)owner.Get(Am_HEIGHT), 2.0));
    
    double theta = acos(
*/

#define TINY_ABOUT_AMULET "lib/images/tinyaboutamulet.gif"

void
Create_Prototypes()
{
  const char *pathname = Am_Merge_Pathname(ARROW_BITMAP);
  Am_Image_Array picture = Am_Image_Array(pathname);
  delete[] pathname;
  if (!picture.Valid())
    Am_Error("Arrow bitmap image not found");
  arrow_bitmap = Am_Bitmap.Create("Arrow bitmap")
                     .Set(Am_IMAGE, picture)
                     .Set(Am_LINE_STYLE, Am_Black);
  /*
  one_way_arrow_line_proto = Am_Group.Create()
    .Add(Am_LINE_STYLE, Am_Thin_Line)
    .Set(Am_TOP, compute_arrowline_top)
    .Add(Am_X1, 0).Add(Am_Y1, 0).Add(Am_X2, 0).Add(Am_Y2, 0)
    .Set(Am_WIDTH, Am_Width_Of_Parts)
    .Set(Am_HEIGHT, Am_Height_Of_Parts)
    .Add_Part(Am_Line.Create()
              .Set(Am_LINE_STYLE, Am_From_Owner(Am_LINE_STYLE))
              .Set(Am_X1, Am_From_Owner(Am_X1))
              .Set(Am_Y1, Am_From_Owner(Am_Y1))
              .Set(Am_X2, Am_From_Owner(Am_X2))
              .Set(Am_Y2, Am_From_Owner(Am_Y2)))
    .Add_Part(Am_Line.Create()
              .Set(Am_LINE_STYLE, Am_From_Owner(Am_LINE_STYLE))
              .Set(Am_X1, rotate_p1)
              .Set(Am_X2, Am_From_Owner(Am_X2))
              .Set(Am_Y2, Am_From_Owner(Am_Y2)))
    .Add_Part(Am_Line.Create()
              .Set(Am_LINE_STYLE, Am_From_Owner(Am_LINE_STYLE))
              .Set(Am_X1, rotate_p2)
              .Set(Am_X2, Am_From_Owner(Am_X2))
              .Set(Am_Y2, Am_From_Owner(Am_Y2)));              
*/
  const char *path = Am_Merge_Pathname(TINY_ABOUT_AMULET);

  bitmap_proto = Am_Bitmap.Create("bitmap proto")
                     .Add(CUSTOMIZE_METHOD, customize_name_and_image)
                     .Add(Lw_NAME, "")
                     .Add(FILE_NAME, path)
                     .Add(Am_HOW_MANY_POINTS, 1)
                     .Add(FILL_STYLE_KEY, 0)
                     .Add(TYPE_NAME, "Am_Bitmap")
                     .Set(Am_IMAGE, Am_Image_Array(path))
                     .Add(Am_MINIMUM_WIDTH, Am_Same_As(Am_WIDTH))
                     .Add(Am_MINIMUM_HEIGHT, Am_Same_As(Am_HEIGHT))
                     .Add(Am_INACTIVE_COMMANDS,
                          Am_Command.Create().Add(Am_GROW_INACTIVE, true))
                     .Add(CONSTRAINT_INFO, Am_No_Value)
                     .Add(REF_COUNT, Am_Value_List()
                                         .Add(0)
                                         .Add(0)
                                         .Add(0)
                                         .Add(0)
                                         .Add(0)
                                         .Add(0)
                                         .Add(0)
                                         .Add(0))
                     .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
                     .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                                .Add(Lw_NAME)
                                                .Add(Am_LEFT)
                                                .Add(Am_TOP)
                                                .Add(Am_WIDTH)
                                                .Add(Am_HEIGHT)
                                                .Add(FILE_NAME)
                                                .Add(CONSTRAINT_INFO)
                                                .Add(REF_COUNT));

  arc_proto = Am_Arc.Create("Circle_Proto")
                  .Add(CUSTOMIZE_METHOD, customize_name_only)
                  .Add(Lw_NAME, "")
                  .Add(TYPE_NAME, "Am_Arc")
                  .Add(Am_HOW_MANY_POINTS, 2)
                  .Add(Am_MINIMUM_WIDTH, 10)
                  .Add(Am_MINIMUM_HEIGHT, 10)
                  .Set(Am_FILL_STYLE, Am_No_Style)
                  .Add(FILL_STYLE_KEY, 0)
                  .Set(Am_LINE_STYLE, Am_Black)
                  .Add(LINE_STYLE_KEY, 8)
                  .Add(CONSTRAINT_INFO, Am_No_Value)
                  .Add(REF_COUNT, Am_Value_List()
                                      .Add(0)
                                      .Add(0)
                                      .Add(0)
                                      .Add(0)
                                      .Add(0)
                                      .Add(0)
                                      .Add(0)
                                      .Add(0))
                  .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
                  .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                             .Add(Lw_NAME)
                                             .Add(Am_LEFT)
                                             .Add(Am_TOP)
                                             .Add(Am_WIDTH)
                                             .Add(Am_HEIGHT)
                                             .Add(LINE_STYLE_KEY)
                                             .Add(FILL_STYLE_KEY)
                                             .Add(CONSTRAINT_INFO)
                                             .Add(REF_COUNT));
  rectangle_proto = Am_Rectangle.Create("Rectangle_Proto")
                        .Add(CUSTOMIZE_METHOD, customize_name_only)
                        .Add(Lw_NAME, "")
                        .Add(TYPE_NAME, "Am_Rectangle")
                        .Add(Am_HOW_MANY_POINTS, 2)
                        .Add(Am_MINIMUM_WIDTH, 10)
                        .Add(Am_MINIMUM_HEIGHT, 10)
                        .Set(Am_FILL_STYLE, Am_No_Style)
                        .Add(FILL_STYLE_KEY, 0)
                        .Set(Am_LINE_STYLE, Am_Black)
                        .Add(LINE_STYLE_KEY, 8)
                        .Add(CONSTRAINT_INFO, Am_No_Value)
                        .Add(REF_COUNT, Am_Value_List()
                                            .Add(0)
                                            .Add(0)
                                            .Add(0)
                                            .Add(0)
                                            .Add(0)
                                            .Add(0)
                                            .Add(0)
                                            .Add(0))
                        .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
                        .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                                   .Add(Lw_NAME)
                                                   .Add(Am_LEFT)
                                                   .Add(Am_TOP)
                                                   .Add(Am_WIDTH)
                                                   .Add(Am_HEIGHT)
                                                   .Add(LINE_STYLE_KEY)
                                                   .Add(FILL_STYLE_KEY)
                                                   .Add(CONSTRAINT_INFO)
                                                   .Add(REF_COUNT));
  polygon_proto = Am_Polygon.Create("Polygon_Proto")
                      .Add(CUSTOMIZE_METHOD, customize_name_only)
                      .Add(Lw_NAME, "")
                      .Add(TYPE_NAME, "Am_Polygon")
                      .Set(Am_FILL_STYLE, Am_No_Style)
                      .Add(FILL_STYLE_KEY, 0)
                      .Set(Am_LINE_STYLE, Am_Black)
                      .Add(LINE_STYLE_KEY, 8)
                      .Add(CONSTRAINT_INFO, Am_No_Value)
                      .Add(REF_COUNT, Am_Value_List()
                                          .Add(0)
                                          .Add(0)
                                          .Add(0)
                                          .Add(0)
                                          .Add(0)
                                          .Add(0)
                                          .Add(0)
                                          .Add(0))
                      .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
                      .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                                 .Add(Lw_NAME)
                                                 .Add(Am_POINT_LIST)
                                                 .Add(LINE_STYLE_KEY)
                                                 .Add(FILL_STYLE_KEY)
                                                 .Add(CONSTRAINT_INFO)
                                                 .Add(REF_COUNT));
  line_proto = Am_Line.Create("Line_Proto")
                   .Add(CUSTOMIZE_METHOD, customize_name_only)
                   .Add(Lw_NAME, "")
                   .Add(TYPE_NAME, "Am_Line")
                   .Set(Am_LINE_STYLE, Am_Black)
                   .Add(LINE_STYLE_KEY, 8)
                   .Add(Am_MINIMUM_LENGTH, 10)
                   .Add(Am_HOW_MANY_POINTS, 2)
                   .Add(CONSTRAINT_INFO, Am_No_Value)
                   .Add(REF_COUNT, Am_Value_List().Add(0).Add(0).Add(0).Add(0))
                   .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
                   .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                              .Add(Lw_NAME)
                                              .Add(Am_X1)
                                              .Add(Am_Y1)
                                              .Add(Am_X2)
                                              .Add(Am_Y2)
                                              .Add(LINE_STYLE_KEY)
                                              .Add(CONSTRAINT_INFO)
                                              .Add(REF_COUNT));
  text_proto = Am_Text.Create("Text_Proto")
                   .Add(CUSTOMIZE_METHOD, customize_text)
                   .Add(Lw_NAME, "")
                   .Add(TYPE_NAME, "Am_Text")
                   .Set(Am_TEXT, "Text")
                   .Add(FONT_KEY, Am_Value_List().Add(0).Add(1).Add(0))
                   .Set(Am_FILL_STYLE, Am_No_Style)
                   .Add(FILL_STYLE_KEY, 0)
                   .Set(Am_LINE_STYLE, Am_Black)
                   .Add(LINE_STYLE_KEY, 8)
                   .Add(Am_HOW_MANY_POINTS, 1)
                   .Add(CONSTRAINT_INFO, Am_No_Value)
                   .Add(REF_COUNT, Am_Value_List()
                                       .Add(0)
                                       .Add(0)
                                       .Add(0)
                                       .Add(0)
                                       .Add(0)
                                       .Add(0)
                                       .Add(0)
                                       .Add(0))
                   .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
                   .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                              .Add(Lw_NAME)
                                              .Add(Am_LEFT)
                                              .Add(Am_TOP)
                                              .Add(Am_WIDTH)
                                              .Add(Am_HEIGHT)
                                              .Add(Am_TEXT)
                                              .Add(FONT_KEY)
                                              .Add(LINE_STYLE_KEY)
                                              .Add(FILL_STYLE_KEY)
                                              .Add(CONSTRAINT_INFO)
                                              .Add(REF_COUNT));
  border_rectangle_proto =
      Am_Border_Rectangle.Create("Border_Rectangle_Proto")
          .Add(CUSTOMIZE_METHOD, customize_border_rect)
          .Add(Lw_NAME, "")
          .Add(TYPE_NAME, "Am_Border_Rectangle")
          .Add(Am_HOW_MANY_POINTS, 2)
          .Add(Am_MINIMUM_WIDTH, 10)
          .Add(Am_MINIMUM_HEIGHT, 10)
          .Add(FILL_STYLE_KEY, 10)
          .Set(Am_FILL_STYLE, Am_Amulet_Purple)
          .Add(CONSTRAINT_INFO, Am_No_Value)
          .Add(REF_COUNT, Am_Value_List()
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0))
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
          .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                     .Add(Lw_NAME)
                                     .Add(Am_LEFT)
                                     .Add(Am_TOP)
                                     .Add(Am_WIDTH)
                                     .Add(Am_HEIGHT)
                                     .Add(Am_SELECTED)
                                     .Add(FILL_STYLE_KEY)
                                     .Add(CONSTRAINT_INFO)
                                     .Add(REF_COUNT));
  button_proto = Am_Button.Create("Button_Proto")
                     .Set(Am_ACTIVE_2, run_tool)
                     .Add(CUSTOMIZE_METHOD, customize_image_label)
                     .Add(Am_HOW_MANY_POINTS, 1)
                     .Add(FONT_KEY, Am_Value_List().Add(0).Add(1).Add(0))
                     .Add(SUB_LABEL, Am_From_Part(Am_COMMAND, Am_LABEL))
                     .Add(FILL_STYLE_KEY, 10)
                     .Set(Am_FILL_STYLE, Am_Amulet_Purple)
                     .Add(Lw_NAME, "")
                     .Add(TYPE_NAME, "Am_Button")
                     .Add(Am_MINIMUM_WIDTH, 66)
                     .Add(Am_MINIMUM_HEIGHT, 40)
                     .Add(CONSTRAINT_INFO, Am_No_Value)
                     .Add(REF_COUNT, Am_Value_List()
                                         .Add(0)
                                         .Add(0)
                                         .Add(0)
                                         .Add(0)
                                         .Add(0)
                                         .Add(0)
                                         .Add(0)
                                         .Add(0))
                     .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
                     .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                                .Add(Am_LEFT)
                                                .Add(Am_TOP)
                                                .Add(Am_WIDTH)
                                                .Add(Am_HEIGHT)
                                                .Add(FONT_KEY)
                                                .Add(FILL_STYLE_KEY)
                                                .Add(SUB_LABEL)
                                                .Add(Lw_NAME)
                                                .Add(CONSTRAINT_INFO)
                                                .Add(REF_COUNT));
  buttons_proto =
      Am_Button_Panel.Create("Buttons_Proto")
          .Add(Lw_NAME, "")
          .Add(TYPE_NAME, "Am_Button_Panel")
          .Add(LAYOUT_KEY, 0)
          .Set(Am_LAYOUT, Am_Vertical_Layout)
          .Add(FONT_KEY, Am_Value_List().Add(0).Add(1).Add(0))
          .Add(CUSTOMIZE_METHOD, customize_panels)
          .Set(Am_ACTIVE_2, run_tool)
          .Add(FILL_STYLE_KEY, 10)
          .Set(Am_FILL_STYLE, Am_Amulet_Purple)
          .Set(Am_ITEMS, Am_Value_List().Add("Button 1").Add("Button 2"))
          .Add(Am_HOW_MANY_POINTS, 1)
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
          .Add(REF_COUNT, Am_Value_List()
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0))
          .Add(CONSTRAINT_INFO, Am_No_Value)
          .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                     .Add(Am_LEFT)
                                     .Add(Am_TOP)
                                     .Add(FONT_KEY)
                                     .Add(FILL_STYLE_KEY)
                                     .Add(Am_ITEMS)
                                     .Add(LAYOUT_KEY)
                                     .Add(Lw_NAME)
                                     .Add(Am_H_SPACING)
                                     .Add(Am_V_SPACING)
                                     .Add(Am_MAX_RANK)
                                     .Add(CONSTRAINT_INFO)
                                     .Add(REF_COUNT));
  ok_proto = Am_Button_Panel.Create("OK_Proto")
                 .Add(Lw_NAME, "")
                 .Add(CUSTOMIZE_METHOD, customize_panels)
                 .Add(TYPE_NAME, "Am_Button_Panel")
                 .Add(LAYOUT_KEY, 1)
                 .Set(Am_LAYOUT, Am_Horizontal_Layout)
                 .Add(FONT_KEY, Am_Value_List().Add(0).Add(1).Add(0))
                 .Set(Am_ACTIVE_2, run_tool)
                 .Add(FILL_STYLE_KEY, 10)
                 .Set(Am_FILL_STYLE, Am_Amulet_Purple)
                 .Set(Am_ITEMS,
                      Am_Value_List()
                          .Add(Am_Standard_OK_Command.Create()
                                   .Set(Am_DO_METHOD, fake_do_cancel)
                                   .Set(Am_ACCELERATOR,
                                        NULL)) //get rid of the accelerator here
                          .Add(Am_Standard_Cancel_Command.Create().Set(
                              Am_DO_METHOD, fake_do_cancel)))
                 .Add(SAVE_COMMAND_ITEMS, true)
                 .Add(Am_HOW_MANY_POINTS, 1)
                 .Add(CONSTRAINT_INFO, Am_No_Value)
                 .Add(REF_COUNT, Am_Value_List()
                                     .Add(0)
                                     .Add(0)
                                     .Add(0)
                                     .Add(0)
                                     .Add(0)
                                     .Add(0)
                                     .Add(0)
                                     .Add(0))
                 .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
                 .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                            .Add(Am_LEFT)
                                            .Add(Am_TOP)
                                            .Add(FONT_KEY)
                                            .Add(FILL_STYLE_KEY)
                                            .Add(LAYOUT_KEY)
                                            .Add(Lw_NAME)
                                            .Add(Am_H_SPACING)
                                            .Add(Am_V_SPACING)
                                            .Add(Am_MAX_RANK)
                                            .Add(SAVE_COMMAND_ITEMS)
                                            .Add(CONSTRAINT_INFO)
                                            .Add(REF_COUNT));

  checkbox_proto =
      Am_Checkbox_Panel.Create("Checkbox_Proto")
          .Add(Lw_NAME, "")
          .Add(TYPE_NAME, "Am_Checkbox_Panel")
          .Add(LAYOUT_KEY, 0)
          .Set(Am_LAYOUT, Am_Vertical_Layout)
          .Add(FONT_KEY, Am_Value_List().Add(0).Add(1).Add(0))
          .Add(FILL_STYLE_KEY, 10)
          .Set(Am_FILL_STYLE, Am_Amulet_Purple)
          .Add(CUSTOMIZE_METHOD, customize_panels)
          .Set(Am_ACTIVE_2, run_tool)
          .Set(Am_ITEMS, Am_Value_List().Add("Checkbox 1").Add("Checkbox 2"))
          .Add(Am_HOW_MANY_POINTS, 1)
          .Add(REF_COUNT, Am_Value_List()
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0))
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
          .Add(CONSTRAINT_INFO, Am_No_Value)
          .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                     .Add(Am_LEFT)
                                     .Add(Am_TOP)
                                     .Add(FONT_KEY)
                                     .Add(FILL_STYLE_KEY)
                                     .Add(Am_ITEMS)
                                     .Add(LAYOUT_KEY)
                                     .Add(Lw_NAME)
                                     .Add(Am_H_SPACING)
                                     .Add(Am_V_SPACING)
                                     .Add(Am_MAX_RANK)
                                     .Add(CONSTRAINT_INFO)
                                     .Add(REF_COUNT));
  radio_proto =
      Am_Radio_Button_Panel.Create("Radio_Proto")
          .Add(Lw_NAME, "")
          .Add(TYPE_NAME, "Am_Radio_Button_Panel")
          .Add(LAYOUT_KEY, 0)
          .Set(Am_LAYOUT, Am_Vertical_Layout)
          .Add(FONT_KEY, Am_Value_List().Add(0).Add(1).Add(0))
          .Add(FILL_STYLE_KEY, 10)
          .Set(Am_FILL_STYLE, Am_Amulet_Purple)
          .Add(CUSTOMIZE_METHOD, customize_panels)
          .Set(Am_ACTIVE_2, run_tool)
          .Set(Am_ITEMS, Am_Value_List().Add("Radio 1").Add("Radio 2"))
          .Add(Am_HOW_MANY_POINTS, 1)
          .Add(CONSTRAINT_INFO, Am_No_Value)
          .Add(REF_COUNT, Am_Value_List()
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0))
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
          .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                     .Add(Am_LEFT)
                                     .Add(Am_TOP)
                                     .Add(FONT_KEY)
                                     .Add(FILL_STYLE_KEY)
                                     .Add(Am_ITEMS)
                                     .Add(LAYOUT_KEY)
                                     .Add(Lw_NAME)
                                     .Add(Am_H_SPACING)
                                     .Add(Am_V_SPACING)
                                     .Add(Am_MAX_RANK)
                                     .Add(CONSTRAINT_INFO)
                                     .Add(REF_COUNT));
  option_proto =
      Am_Option_Button.Create("Option_Proto")
          .Add(Lw_NAME, "")
          .Add(TYPE_NAME, "Am_Option_Button")
          .Add(FONT_KEY, Am_Value_List().Add(0).Add(1).Add(0))
          .Add(FILL_STYLE_KEY, 10)
          .Set(Am_FILL_STYLE, Am_Amulet_Purple)
          .Set(Am_ACTIVE_2, run_tool)
          .Add(CUSTOMIZE_METHOD, customize_panels)
          .Set(Am_ITEMS, Am_Value_List().Add("Option 1").Add("Option 2"))
          .Add(Am_HOW_MANY_POINTS, 1)
          .Add(CONSTRAINT_INFO, Am_No_Value)
          .Add(REF_COUNT, Am_Value_List()
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0))
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
          .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                     .Add(Am_LEFT)
                                     .Add(Am_TOP)
                                     .Add(FONT_KEY)
                                     .Add(FILL_STYLE_KEY)
                                     .Add(Am_ITEMS)
                                     .Add(Lw_NAME)
                                     .Add(CONSTRAINT_INFO)
                                     .Add(REF_COUNT));
  text_input_proto =
      Am_Text_Input_Widget.Create("Text_Input_Proto")
          .Add(Lw_NAME, "")
          .Add(TYPE_NAME, "Am_Text_Input_Widget")
          .Add(CUSTOMIZE_METHOD, customize_name_label)
          .Add(FONT_KEY, Am_Value_List().Add(0).Add(1).Add(0))
          .Add(FILL_STYLE_KEY, 10)
          .Set(Am_FILL_STYLE, Am_Amulet_Purple)
          .Set(Am_ACTIVE_2, run_tool)
          .Add(Am_HOW_MANY_POINTS, 2)
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
          .Add(Am_MINIMUM_WIDTH, 115)
          .Add(Am_MINIMUM_HEIGHT, 25)
          .Add(CONSTRAINT_INFO, Am_No_Value)
          .Add(REF_COUNT, Am_Value_List()
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0))
          .Add(SUB_LABEL,
               Am_Text_Input_Widget.Get_Object(Am_COMMAND).Get(Am_LABEL))
          .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                     .Add(Am_LEFT)
                                     .Add(Am_TOP)
                                     .Add(Am_WIDTH)
                                     .Add(Am_HEIGHT)
                                     .Add(SUB_LABEL)
                                     .Add(FONT_KEY)
                                     .Add(FILL_STYLE_KEY)
                                     .Add(Lw_NAME)
                                     .Add(CONSTRAINT_INFO)
                                     .Add(REF_COUNT));
  number_input_proto =
      Am_Number_Input_Widget.Create("Number_Input_Proto")
          .Add(Lw_NAME, "")
          .Add(TYPE_NAME, "Am_Number_Input_Widget")
          .Add(CUSTOMIZE_METHOD, customize_number_input)
          .Add(FONT_KEY, Am_Value_List().Add(0).Add(1).Add(0))
          .Add(FILL_STYLE_KEY, 10)
          .Set(Am_FILL_STYLE, Am_Amulet_Purple)
          .Add(Am_MINIMUM_WIDTH, 115)
          .Add(Am_MINIMUM_HEIGHT, 25)
          .Add(SUB_LABEL,
               Am_Number_Input_Widget.Get_Object(Am_COMMAND).Get(Am_LABEL))
          .Set(Am_ACTIVE_2, run_tool)
          .Add(Am_HOW_MANY_POINTS, 2)
          .Add(CONSTRAINT_INFO, Am_No_Value)
          .Add(REF_COUNT, Am_Value_List()
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0))
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
          .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                     .Add(Am_LEFT)
                                     .Add(Am_TOP)
                                     .Add(Am_WIDTH)
                                     .Add(Am_HEIGHT)
                                     .Add(FONT_KEY)
                                     .Add(FILL_STYLE_KEY)
                                     .Add(SUB_LABEL)
                                     .Add(Lw_NAME)
                                     .Add(Am_VALUE_1)
                                     .Add(Am_VALUE_2)
                                     .Add(CONSTRAINT_INFO)
                                     .Add(REF_COUNT));
  scroll_group_proto =
      Am_Scrolling_Group.Create("Scroll_Group_Proto")
          .Add(Lw_NAME, "")
          .Add(TYPE_NAME, "Am_Scrolling_Group")
          .Set(Am_ACTIVE_2, run_tool)
          .Add(CUSTOMIZE_METHOD, customize_scroll_group)
          .Add(Am_MINIMUM_WIDTH, 75)
          .Add(Am_MINIMUM_HEIGHT, 75)
          .Add(Am_HOW_MANY_POINTS, 2)
          .Add(FILL_STYLE_KEY, 10)
          .Set(Am_FILL_STYLE, Am_Amulet_Purple)
          .Add(CONSTRAINT_INFO, Am_No_Value)
          .Add(REF_COUNT, Am_Value_List()
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0))
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
          .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                     .Add(Am_LEFT)
                                     .Add(Am_TOP)
                                     .Add(Am_WIDTH)
                                     .Add(Am_HEIGHT)
                                     .Add(FILL_STYLE_KEY)
                                     .Add(Am_INNER_WIDTH)
                                     .Add(Am_INNER_HEIGHT)
                                     .Add(Am_H_SCROLL_BAR)
                                     .Add(Am_V_SCROLL_BAR)
                                     .Add(Am_H_SCROLL_BAR_ON_TOP)
                                     .Add(Am_V_SCROLL_BAR_ON_LEFT)
                                     .Add(Lw_NAME)
                                     .Add(CONSTRAINT_INFO)
                                     .Add(REF_COUNT));

  main_group_rect_proto =
      Am_Rectangle.Create("main_group_rect_proto")
          .Add(FILL_STYLE_KEY, 9)
          .Add(MAIN_NAME, "")
          .Add(C_FILENAME, "")
          .Add(CREATE_HEADER, true)
          .Add(H_FILENAME, "")
          .Add(WINDOW_OR_GROUP, true)
          .Add(WIN_TITLE, how_many_windows)
          .Add(FIXED_SIZE_OBJ, false)
          .Add(EXPLICIT_SIZE_OBJ, false)
          .Add(WIDTH_OBJ, 0)
          .Add(HEIGHT_OBJ, 0)
          .Add(CONSTRAINT_INFO, Am_No_Value)
          .Add(Am_INACTIVE_COMMANDS, Am_Command.Create()
                                         .Add(Am_MOVE_INACTIVE, true)
                                         .Add(Am_GROW_INACTIVE, true)
                                         .Add(Am_TO_TOP_INACTIVE, true)
                                         .Add(Am_CLEAR_INACTIVE, true)
                                         .Add(Am_COPY_INACTIVE, true)
                                         .Add(Am_CUT_INACTIVE, true)
                                         .Add(Am_DUPLICATE_INACTIVE, true)
                                         .Add(Am_GROUP_INACTIVE, true))
          .Add(REF_COUNT, Am_Value_List()
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0)
                              .Add(0))
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
          .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                     .Add(MAIN_NAME)
                                     .Add(C_FILENAME)
                                     .Add(CREATE_HEADER)
                                     .Add(H_FILENAME)
                                     .Add(WINDOW_OR_GROUP)
                                     .Add(WIN_TITLE)
                                     .Add(FIXED_SIZE_OBJ)
                                     .Add(EXPLICIT_SIZE_OBJ)
                                     .Add(WIDTH_OBJ)
                                     .Add(HEIGHT_OBJ)
                                     .Add(FILL_STYLE_KEY)
                                     .Add(CONSTRAINT_INFO)
                                     .Add(REF_COUNT));

  Am_Default_Load_Save_Context.Register_Prototype("MAIN_GROUP",
                                                  main_group_rect_proto);

  /********************************************************************
    Following is setup for main_window_proto, main_window_proto is a 
    prototype for a window which cnotains menubar and working space.
    *******************************************************************/

  Am_Object grid_command = Am_Cycle_Value_Command.Create("grid").Set(
      Am_LABEL_LIST, Am_Value_List().Add("Turn Grid On").Add("Turn Grid Off"));

  Am_Object open_command = Am_Open_Command.Create().Set(
      Am_HANDLE_OPEN_SAVE_METHOD, use_file_contents);

  Am_Object main_group, menubar, rfeedback;

  main_window_proto =
      Am_Window.Create("main_window_proto")
          .Set(Am_WIDTH, 500)
          .Set(Am_HEIGHT, 650)
          .Set(Am_LEFT, 200)
          .Set(Am_TOP, 200)
          .Set(Am_TITLE, get_title)
          .Add(GRID_COMMAND, grid_command)
          .Add(OPEN_COMMAND, open_command)
          .Set(Am_DESTROY_WINDOW_METHOD, my_destroy_window_method)
          .Add_Part(EXISTING_MENU, menubar = Am_Menu_Bar.Create("menubar"))
          .Add_Part(MAIN_GROUP,
                    main_group =
                        Am_Group.Create("main_group")
                            .Set(Am_HEIGHT, Am_From_Owner(Am_HEIGHT, -30))
                            .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH)))
          .Add_Part(Am_One_Shot_Interactor.Create()
                        .Set(Am_START_WHEN, Am_Input_Char("ESC"))
                        .Get_Object(Am_COMMAND)
                        .Set(Am_DO_METHOD, out_of_current_mode)
                        .Set(Am_UNDO_METHOD, undo_out_of_current_mode)
                        .Set(Am_REDO_METHOD, undo_out_of_current_mode)
                        .Get_Owner());

  Am_Object lfeedback = Am_Line.Create("lfeedback")
                            .Set(Am_LINE_STYLE, Am_Dotted_Line)
                            .Set(Am_VISIBLE, 0);
  rfeedback = Am_Rectangle.Create("rfeedback")
                  .Set(Am_FILL_STYLE, 0)
                  .Set(Am_LINE_STYLE, Am_Dotted_Line)
                  .Set(Am_VISIBLE, 0);

  main_group.Set(Am_TOP, 30)
      .Set(Am_LEFT, 0)
      .Add_Part(L_FEEDBACK, lfeedback)
      .Add_Part(R_FEEDBACK, rfeedback)
      .Add_Part(MAIN_GROUP_RECT,
                main_group_rect_proto.Set(Am_LINE_STYLE, Am_Line_1)
                    .Set(FILL_STYLE_KEY, 9)
                    .Set(Am_FILL_STYLE, Am_White)
                    .Set(Am_LEFT, 0)
                    .Set(Am_TOP, 0)
                    .Set(Am_HEIGHT, Am_From_Owner(Am_HEIGHT))
                    .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH)))
      .Add_Part(
          WORK_GROUP,
          Am_Group.Create("workgroup")
              .Set(Am_LEFT, 0)
              .Set(Am_TOP, 0)
              .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH))
              .Set(Am_HEIGHT, Am_From_Owner(Am_HEIGHT))
              .Add_Part(
                  Am_Choice_Interactor.Create()
                      .Set(Am_HOW_SET, Am_CHOICE_SET)
                      //has to be higher than 3 or the sel_widget steals the click
                      .Set(Am_PRIORITY, 5)
                      .Set(Am_START_WHEN,
                           Am_Value_List() //either middle, doubleclick
                               .Add(Am_Input_Char("MIDDLE_DOWN"))
                               .Add(Am_Input_Char("DOUBLE_LEFT_DOWN")))
                      .Set(Am_SET_SELECTED, false)
                      .Get_Object(Am_COMMAND)
                      .Set(Am_DO_METHOD, customize)
                      .Set(Am_UNDO_METHOD, undo_set_properties)
                      .Set(Am_REDO_METHOD, undo_set_properties)
                      .Get_Owner())
              .Add_Part(Am_New_Points_Interactor.Create("create_inter")
                            .Set(Am_AS_LINE, line_tool)
                            .Set(Am_FEEDBACK_OBJECT, compute_feedback_obj)
                            .Set(Am_CREATE_NEW_OBJECT_METHOD, create_new_object)
                            .Set(Am_ACTIVE, rubber_bandable_tool_is_selected)
                            .Set(Am_GRID_X, grid_if_should_second)
                            .Set(Am_GRID_Y, grid_if_should_second)
                            .Set(Am_MINIMUM_WIDTH, minw_from_tool)
                            .Set(Am_MINIMUM_HEIGHT, minh_from_tool)
                            .Set(Am_HOW_MANY_POINTS, points_from_tool))
              .Add_Part(Am_New_Points_Interactor.Create("create_polygons")
                            .Set(Am_START_WHEN, "ANY_SINGLE_LEFT_DOWN")
                            .Set(Am_STOP_WHEN, "ANY_DOUBLE_LEFT_DOWN")
                            .Set(Am_AS_LINE, true)
                            .Set(Am_FEEDBACK_OBJECT, return_lfeedback)
                            .Set(Am_START_DO_METHOD, polygon_start)
                            .Set(Am_INTERIM_DO_METHOD, polygon_interim_do)
                            .Set(Am_DO_METHOD, polygon_do)
                            .Set(Am_ABORT_DO_METHOD, polygon_abort)
                            .Set(Am_CREATE_NEW_OBJECT_METHOD, create_new_object)
                            .Set(Am_GRID_X, grid_if_should_second)
                            .Set(Am_GRID_Y, grid_if_should_second)
                            .Set(Am_ACTIVE, polygon_tool_is_selected)))
      .Add_Part(SEL_WIDGET,
                Modified_Selection_Widget.Create("sel_widget")
                    .Set(Am_ACTIVE, selection_tool)
                    .Set(Am_GRID_X, grid_if_should)
                    .Set(Am_GRID_Y, grid_if_should)
                    .Set_Inherit_Rule(Am_MULTI_SELECTIONS, Am_INHERIT)
                    .Set(Am_MULTI_SELECTIONS, Selection_Widget_List)
                    .Set(Am_OPERATES_ON, get_workgroup))
      .Add_Part(CONSTRAINT_WIDGET, Constraint_Widget.Create("constraint widget")
                                       .Set(Am_ACTIVE, constraint_tool)
                                       .Set(Am_OPERATES_ON, get_workgroup));
  Am_Object arl_fd = main_group.Get_Object(CONSTRAINT_WIDGET)
                         .Get_Object(Am_LINE_FEEDBACK_OBJECT);

  main_group.Add_Part(arl_fd);

  menubar.Set(Am_SELECTION_WIDGET, get_sel_widget)
      .Set(
          Am_ITEMS,
          Am_Value_List()
              .Add(Am_Command.Create("File_Command")
                       .Set(Am_IMPLEMENTATION_PARENT, true) //not undoable
                       .Set(Am_LABEL, "File")
                       .Set(Am_ITEMS,
                            Am_Value_List()
                                .Add(open_command)
                                .Add(Am_Save_As_Command.Create().Set(
                                    Am_HANDLE_OPEN_SAVE_METHOD,
                                    contents_for_save))
                                .Add(Am_Save_Command.Create().Set(
                                    Am_HANDLE_OPEN_SAVE_METHOD,
                                    contents_for_save))
                                .Add(Am_Command.Create("Generate C++")
                                         .Set(Am_LABEL, "Generate c++")
                                         .Set(Am_ACTIVE, true)
                                         .Set(Am_DO_METHOD, savecppcmd))
                                .Add(Am_Command.Create("Close")
                                         .Set(Am_LABEL, "Close")
                                         .Set(Am_DO_METHOD, close_window))))
              .Add(Am_Command.Create("Edit_Command")
                       .Set(Am_LABEL, "Edit")
                       .Set(Am_IMPLEMENTATION_PARENT, true) //not undoable
                       .Set(Am_ITEMS,
                            Am_Value_List()
                                .Add(Am_Undo_Command.Create())
                                .Add(Am_Redo_Command.Create())
                                .Add(Am_Menu_Line_Command.Create())
                                .Add(Am_Command.Create()
                                         .Set(Am_LABEL, "Edit Menu")
                                         .Set(Am_DO_METHOD,
                                              enable_menu_edit_window))
                                .Add(Am_Menu_Line_Command.Create())
                                .Add(Am_Graphics_Cut_Command.Create())
                                .Add(Am_Graphics_Copy_Command.Create())
                                .Add(Am_Graphics_Paste_Command.Create())
                                .Add(Am_Graphics_Clear_Command.Create())
                                .Add(Am_Graphics_Clear_All_Command.Create())
                                .Add(Am_Menu_Line_Command.Create())
                                .Add(Am_Graphics_Duplicate_Command.Create())
                                .Add(Am_Selection_Widget_Select_All_Command
                                         .Create())
                                .Add(Am_Menu_Line_Command.Create())
                                .Add(Am_Command.Create()
                                         .Set(Am_LABEL, "Properties...")
                                         .Set(Am_ACTIVE, Am_Active_If_Selection)
                                         .Set(Am_DO_METHOD, customize_selected)
                                         .Set(Am_UNDO_METHOD,
                                              undo_set_properties)
                                         .Set(Am_REDO_METHOD,
                                              undo_set_properties))))
              .Add(
                  Am_Command.Create("Arrange_Command")
                      .Set(Am_LABEL, "Arrange")
                      .Set(Am_IMPLEMENTATION_PARENT, true) //not undoable
                      .Set(
                          Am_ITEMS,
                          Am_Value_List()
                              .Add(Am_Graphics_To_Top_Command.Create())
                              .Add(Am_Graphics_To_Bottom_Command.Create())
                              .Add(Am_Menu_Line_Command.Create())
                              .Add(Am_Graphics_Group_Command.Create())
                              .Add(Am_Graphics_Ungroup_Command.Create())
                              .Add(Am_Menu_Line_Command.Create())
                              .Add(grid_command)
                              .Add(Am_Menu_Line_Command.Create())
                              .Add(Am_Command.Create("Motif_Command")
                                       .Set(Am_LABEL, "Motif Look")
                                       .Add(Am_WIDGET_LOOK, Am_MOTIF_LOOK)
                                       .Add(Am_CHECKED_ITEM, look_is_me)
                                       .Set(Am_DO_METHOD, set_look)
                                       .Set(Am_IMPLEMENTATION_PARENT,
                                            true)) //not undo
                              .Add(Am_Command.Create("Win_Command")
                                       .Set(Am_LABEL, "Windows Look")
                                       .Add(Am_WIDGET_LOOK, Am_WINDOWS_LOOK)
                                       .Add(Am_CHECKED_ITEM, look_is_me)
                                       .Set(Am_IMPLEMENTATION_PARENT,
                                            true) //not undo
                                       .Set(Am_DO_METHOD, set_look))
                              .Add(Am_Command.Create("Mac_Command")
                                       .Set(Am_LABEL, "Macintosh Look")
                                       .Add(Am_WIDGET_LOOK, Am_MACINTOSH_LOOK)
                                       .Add(Am_CHECKED_ITEM, look_is_me)
                                       .Set(Am_IMPLEMENTATION_PARENT,
                                            true) //not undo
                                       .Set(Am_DO_METHOD, set_look))
                              .Add(Am_Menu_Line_Command.Create())
                              .Add(Am_Command.Create()
                                       .Set(Am_LABEL, "Nudge Left")
                                       .Set(Am_ACTIVE, Am_Active_If_Selection)
                                       .Set(Am_ACCELERATOR, "LEFT_ARROW")
                                       .Set(Am_DO_METHOD, go_left)
                                       .Set(Am_UNDO_METHOD,
                                            undo_horizontal_nudge)
                                       .Set(Am_REDO_METHOD,
                                            undo_horizontal_nudge))
                              .Add(Am_Command.Create()
                                       .Set(Am_ACTIVE, Am_Active_If_Selection)
                                       .Set(Am_LABEL, "Nudge Right")
                                       .Set(Am_ACCELERATOR, "RIGHT_ARROW")
                                       .Set(Am_DO_METHOD, go_right)
                                       .Set(Am_UNDO_METHOD,
                                            undo_horizontal_nudge)
                                       .Set(Am_REDO_METHOD,
                                            undo_horizontal_nudge))
                              .Add(
                                  Am_Command.Create()
                                      .Set(Am_LABEL, "Nudge Down")
                                      .Set(Am_ACTIVE, Am_Active_If_Selection)
                                      .Set(Am_ACCELERATOR, "DOWN_ARROW")
                                      .Set(Am_DO_METHOD, go_down)
                                      .Set(Am_UNDO_METHOD, undo_vertical_nudge)
                                      .Set(Am_REDO_METHOD, undo_vertical_nudge))
                              .Add(
                                  Am_Command.Create()
                                      .Set(Am_LABEL, "Nudge Up")
                                      .Set(Am_ACTIVE, Am_Active_If_Selection)
                                      .Set(Am_ACCELERATOR, "UP_ARROW")
                                      .Set(Am_DO_METHOD, go_up)
                                      .Set(Am_UNDO_METHOD, undo_vertical_nudge)
                                      .Set(Am_REDO_METHOD, undo_vertical_nudge))
                              .Add(Am_Menu_Line_Command.Create())
                              .Add(
                                  Am_Command.Create()
                                      .Set(Am_LABEL, "Align Lefts")
                                      .Set(Am_ACTIVE, Am_Active_If_Selection)
                                      .Set(Am_ACCELERATOR, "CONTROL_LEFT_ARROW")
                                      .Set(Am_DO_METHOD, align_left)
                                      .Set(Am_UNDO_METHOD,
                                           undo_horizontal_align)
                                      .Set(Am_REDO_METHOD,
                                           undo_horizontal_align))
                              .Add(Am_Command.Create()
                                       .Set(Am_LABEL, "Align Rights")
                                       .Set(Am_ACCELERATOR,
                                            "CONTROL_RIGHT_ARROW")
                                       .Set(Am_ACTIVE, Am_Active_If_Selection)
                                       .Set(Am_DO_METHOD, align_right)
                                       .Set(Am_UNDO_METHOD,
                                            undo_horizontal_align)
                                       .Set(Am_REDO_METHOD,
                                            undo_horizontal_align))
                              .Add(
                                  Am_Command.Create()
                                      .Set(Am_LABEL, "Align Tops")
                                      .Set(Am_ACTIVE, Am_Active_If_Selection)
                                      .Set(Am_ACCELERATOR, "CONTROL_UP_ARROW")
                                      .Set(Am_DO_METHOD, align_up)
                                      .Set(Am_UNDO_METHOD, undo_vertical_align)
                                      .Set(Am_REDO_METHOD, undo_vertical_align))
                              .Add(
                                  Am_Command.Create()
                                      .Set(Am_LABEL, "Align Bottoms")
                                      .Set(Am_ACTIVE, Am_Active_If_Selection)
                                      .Set(Am_ACCELERATOR, "CONTROL_DOWN_ARROW")
                                      .Set(Am_DO_METHOD, align_down)
                                      .Set(Am_UNDO_METHOD, undo_vertical_align)
                                      .Set(Am_REDO_METHOD, undo_vertical_align))
                              .Add(Am_Menu_Line_Command.Create())
                              .Add(Am_Command.Create()
                                       .Set(Am_LABEL, "Same Widths")
                                       .Set(Am_ACTIVE, Am_Active_If_Selection)
                                       .Set(Am_ACCELERATOR, "META_RIGHT_ARROW")
                                       .Set(Am_DO_METHOD, same_widths)
                                       .Set(Am_UNDO_METHOD, undo_same_widths)
                                       .Set(Am_REDO_METHOD, undo_same_widths))
                              .Add(
                                  Am_Command.Create()
                                      .Set(Am_LABEL, "Same Heights")
                                      .Set(Am_ACTIVE, Am_Active_If_Selection)
                                      .Set(Am_ACCELERATOR, "META_DOWN_ARROW")
                                      .Set(Am_DO_METHOD, same_heights)
                                      .Set(Am_UNDO_METHOD, undo_same_heights)
                                      .Set(Am_REDO_METHOD, undo_same_heights))))
              .Add(Am_Command.Create("About_Command")
                       .Set(Am_IMPLEMENTATION_PARENT, true) //not undoable
                       .Set(Am_LABEL, "About")
                       .Set(Am_ITEMS,
                            Am_Value_List()
                                .Add(Am_About_Amulet_Command.Create())
                                .Add(Am_Command.Create("About")
                                         .Set(Am_LABEL, "About Gilt")
                                         .Set(Am_ACTIVE, true)
                                         .Set(Am_DO_METHOD, aboutcmd)))));

  // .Add(Lw_NAME) to list of slots to save and load.  Unlike other
  // objects, Am_Resize_Parts_Group already has a Am_SLOTS_TO_SAVE list.

  Am_Value_List slots_to_save = Am_Resize_Parts_Group.Get(Am_SLOTS_TO_SAVE);
  slots_to_save.Add(Lw_NAME);
  Am_Resize_Parts_Group.Add(CUSTOMIZE_METHOD, customize_name_only)
      .Add(Lw_NAME, "")
      .Add(TYPE_NAME, "Am_Group")
      .Set(Am_SLOTS_TO_SAVE, slots_to_save);

  //tell the default loader what to call all of the prototypes in the file
  Am_Default_Load_Save_Context.Register_Prototype("ARC", arc_proto);
  Am_Default_Load_Save_Context.Register_Prototype("RECT", rectangle_proto);
  Am_Default_Load_Save_Context.Register_Prototype("LIN", line_proto);
  Am_Default_Load_Save_Context.Register_Prototype("POLY", polygon_proto);

  Am_Default_Load_Save_Context.Register_Prototype("TEXT", text_proto);
  Am_Default_Load_Save_Context.Register_Prototype("BORDER",
                                                  border_rectangle_proto);
  Am_Default_Load_Save_Context.Register_Prototype("BUTTON", button_proto);
  Am_Default_Load_Save_Context.Register_Prototype("BUTTONS", buttons_proto);
  Am_Default_Load_Save_Context.Register_Prototype("OK", ok_proto);
  Am_Default_Load_Save_Context.Register_Prototype("CHECKBOX", checkbox_proto);
  Am_Default_Load_Save_Context.Register_Prototype("RADIO", radio_proto);
  Am_Default_Load_Save_Context.Register_Prototype("TEXT_INPUT",
                                                  text_input_proto);
  Am_Default_Load_Save_Context.Register_Prototype("NUMBER_INPUT",
                                                  number_input_proto);
  Am_Default_Load_Save_Context.Register_Prototype("OPTION", option_proto);
  Am_Default_Load_Save_Context.Register_Prototype("SCROLL_GROUP",
                                                  scroll_group_proto);
  Am_Default_Load_Save_Context.Register_Prototype("BITMAP", bitmap_proto);
}

/*Am_Define_Method(Am_Object_Method, void, enable_fill_color, (Am_Object  cmd ))
{
  Fill_Style_Panel.Set(Am_VISIBLE, true);
  
}
*/

/*Am_Define_Method(Am_Object_Method, void, enable_line_color, (Am_Object  cmd ))
{
  Line_Color_Panel.Set(Am_VISIBLE, true);
  Line_Color_Button.Set(Am_ACTIVE, false);
}*/

Am_Define_Method(Am_Object_Method, void, choose_color, (Am_Object cmd))
{
  Am_Object c = cmd.Get_Object(Am_VALUE);
  Am_Object window = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner().Get_Owner();

  Am_Finish_Pop_Up_Waiting(window, (int)c.Get(Am_RANK));
}

Am_Define_Method(Am_Object_Method, void, create_new, (Am_Object /* cmd */))
{
  Am_Object obj;
  bool closed_window = false;

  Am_Object window;
  if (closed_window_list.Empty()) {
    window = main_window_proto.Create();
    window.Set(Am_UNDO_HANDLER, Am_Multiple_Undo_Object.Create());
  } else {
    closed_window = true;
    closed_window_list.Start();
    window = (Am_Object)closed_window_list.Get();
    window.Set(Am_WIDTH, 500).Set(Am_HEIGHT, 650);
    closed_window_list.Delete();
  }

  int x = 0, y = 0;

  for (window_list.Start(); !window_list.Last(); window_list.Next()) {
    obj = (Am_Object)window_list.Get();

    if ((int)obj.Get(Am_LEFT) == (200 + x) &&
        (int)obj.Get(Am_TOP) == (200 + y)) {
      x += 20;
      y += 20;
    }
  }

  window.Set(Am_LEFT, 200 + x);
  window.Set(Am_TOP, 200 + y);
  window_list.Add(window);

  Selection_Widget_List.Add(
      window.Get_Object(MAIN_GROUP).Get_Object(SEL_WIDGET));
  main_window_proto.Get_Object(MAIN_GROUP)
      .Get_Object(SEL_WIDGET)
      .Set(Am_MULTI_SELECTIONS, Selection_Widget_List);

  if (closed_window)
    window.Set(Am_VISIBLE, true);
  else
    Am_Screen.Add_Part(window);
}

Am_Define_Formula(int, center_self)
{
  return ((int)self.Get_Owner().Get(Am_WIDTH) - (int)self.Get(Am_WIDTH)) / 2;
}

Am_Define_Formula(int, get_owner_height_if_ready)
{
  Am_Value v = self.Get_Owner().Peek(Am_HEIGHT);
  if (v.Valid()) {
    return (int)v - 3;
  } else
    return 50;
}

Am_Define_String_Formula(transparency)
{
  Am_Style s = self.Get_Owner().Get(Am_FILL_STYLE);
  if (s == Am_No_Style)
    return (Am_String) "TP";
  return (Am_String) "";
}

#define PANEL_WIDTH 120 //width of toolpanel area
#define LINE_LENGTH 70

int
main(int argc, char *argv[])
{
  window_count = -1;
  closed_window_list = Am_Value_List();
  Am_Initialize();
  init_styles();
  cout << "\n************** Styles Initialized ***************\n";
  Modified_Selection_Widget_Initialize();
  Constraint_Widget_Initialize();
  Create_Prototypes();
  Create_Tool_Panel();
  Name_Only_Window_Initialize();
  Number_Input_Window_Initialize();
  Border_Rect_Window_Initialize();
  savecpp_window_Initialize();
  About_Gilt_Window_Initialize();
  Scroll_Group_Window_Initialize();
  Name_And_Label_Window_Initialize(true);
  Name_And_Label_Window_Initialize(false);
  Menuedit_Window_Initialize();
  Image_Label_Dialog_Initialize();
  cout << "************* Creating main window *************\n";

  Am_Object first_line;

  Am_Object line = Am_Line.Create()
                       .Set(Am_X1, 0)
                       .Set(Am_X2, LINE_LENGTH)
                       .Set(Am_Y1, 10)
                       .Set(Am_Y2, 10);

  first_line =
      line.Create().Set(Am_LINE_STYLE, Am_Line_1).Add(LINE_STYLE_KEY, 2);

  Am_Object color_line_group =
      Am_Group.Create()
          .Set(Am_WIDTH, 100)
          .Set(Am_LEFT, 0)
          .Set(Am_HEIGHT, Am_Height_Of_Parts)
          .Add_Part(Am_Border_Rectangle.Create()
                        .Set(Am_TOP, 0)
                        .Set(Am_LEFT, 0)
                        .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH))
                        .Set(Am_HEIGHT, get_owner_height_if_ready)
                        .Set(Am_FILL_STYLE, Am_Amulet_Purple))
          .Add_Part(
              Am_Button.Create()
                  .Set(Am_TOP, 3)
                  .Set(Am_LEFT, 5)
                  .Get_Object(Am_COMMAND)
                  .Set(Am_LABEL,
                       Am_Group.Create()
                           .Set(Am_WIDTH, Am_Width_Of_Parts)
                           .Set(Am_HEIGHT, Am_Height_Of_Parts)
                           .Add_Part(
                               Fill_Style_Button =
                                   Am_Group.Create("Fill Style Button")
                                       .Set(Am_TOP, 0)
                                       .Set(Am_LEFT, 60)
                                       .Set(Am_WIDTH, 30)
                                       .Set(Am_HEIGHT, 20)
                                       .Add(FILL_STYLE_KEY, 10)
                                       .Add(Am_FILL_STYLE, Am_Amulet_Purple)
                                       .Add_Part(
                                           Am_Rectangle.Create()
                                               .Set(Am_WIDTH,
                                                    Am_From_Owner(Am_WIDTH))
                                               .Set(Am_HEIGHT,
                                                    Am_From_Owner(Am_HEIGHT))
                                               .Set(Am_LEFT, 0)
                                               .Set(Am_TOP, 0)
                                               .Set(Am_FILL_STYLE,
                                                    Am_From_Owner(
                                                        Am_FILL_STYLE)))
                                       .Add_Part(
                                           Am_Text.Create()
                                               .Set(Am_TOP, 4)
                                               .Set(Am_LEFT, 4)
                                               .Set(Am_FILL_STYLE, Am_No_Style)
                                               .Set(Am_TEXT, transparency)))
                           .Add_Part(Am_Text.Create()
                                         .Set(Am_TOP, 4)
                                         .Set(Am_LEFT, 8)
                                         .Set(Am_FILL_STYLE, Am_No_Style)
                                         .Set(Am_TEXT, "Fill Color")
                                         .Set(Am_FONT, fontarray[1][0][0])))
                  .Set(Am_DO_METHOD, fillstyler)
                  .Set(Am_UNDO_METHOD, undo_fillstyler)
                  .Set(Am_REDO_METHOD, undo_fillstyler)
                  .Get_Owner())
          .Add_Part(
              Am_Button.Create()
                  .Set(Am_TOP, 36)
                  .Set(Am_LEFT, 5)
                  .Get_Object(Am_COMMAND)
                  .Set(Am_LABEL,
                       Am_Group.Create()
                           .Set(Am_WIDTH, Am_Width_Of_Parts)
                           .Set(Am_HEIGHT, Am_Height_Of_Parts)
                           .Add_Part(
                               Line_Color_Button =
                                   Am_Group.Create("Line Color Button")
                                       .Set(Am_TOP, 0)
                                       .Set(Am_LEFT, 60)
                                       .Set(Am_WIDTH, 30)
                                       .Set(Am_HEIGHT, 20)
                                       .Add(LINE_STYLE_KEY, 8)
                                       .Add(Am_FILL_STYLE, Am_Black)
                                       .Add_Part(
                                           Am_Rectangle.Create()
                                               .Set(Am_WIDTH,
                                                    Am_From_Owner(Am_WIDTH))
                                               .Set(Am_HEIGHT,
                                                    Am_From_Owner(Am_HEIGHT))
                                               .Set(Am_LEFT, 0)
                                               .Set(Am_TOP, 0)
                                               .Set(Am_FILL_STYLE,
                                                    Am_From_Owner(
                                                        Am_FILL_STYLE)))
                                       .Add_Part(
                                           Am_Text.Create()
                                               .Set(Am_TOP, 4)
                                               .Set(Am_LEFT, 4)
                                               .Set(Am_FILL_STYLE, Am_No_Style)
                                               .Set(Am_TEXT, transparency)))
                           .Add_Part(Am_Text.Create()
                                         .Set(Am_TOP, 4)
                                         .Set(Am_LEFT, 8)
                                         .Set(Am_FILL_STYLE, Am_No_Style)
                                         .Set(Am_TEXT, "Line Color")
                                         .Set(Am_FONT, fontarray[1][0][0])))
                  .Set(Am_DO_METHOD, linestyler)
                  .Set(Am_UNDO_METHOD, undo_linestyler)
                  .Set(Am_REDO_METHOD, undo_linestyler)
                  .Get_Owner())
          .Add_Part(
              Line_Thickness_Option_Button =
                  Am_Option_Button.Create("Line_Thickness_Option_Button")
                      .Set(Am_LEFT, 0)
                      .Set(Am_TOP, 70)
                      .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH))
                      .Set(Am_HEIGHT, 30)
                      .Add(LINE_STYLE_KEY, 2)
                      .Set(Am_ITEMS,
                           Am_Value_List()
                               .Add(line.Create()
                                        .Set(Am_LINE_STYLE, Am_Dotted_Line)
                                        .Add(LINE_STYLE_KEY, 0))
                               .Add(line.Create()
                                        .Set(Am_LINE_STYLE, Am_Dashed_Line)
                                        .Add(LINE_STYLE_KEY, 1))
                               .Add(first_line)
                               .Add(line.Create()
                                        .Set(Am_LINE_STYLE, Am_Line_2)
                                        .Add(LINE_STYLE_KEY, 3))
                               .Add(Am_Line.Create()
                                        .Set(Am_X1, 0)
                                        .Set(Am_X2, LINE_LENGTH)
                                        .Set(Am_Y1, 5)
                                        .Set(Am_Y2, 5)
                                        .Set(Am_LINE_STYLE, Am_Line_4)
                                        .Add(LINE_STYLE_KEY, 4))
                               .Add(Am_Line.Create()
                                        .Set(Am_X1, 0)
                                        .Set(Am_X2, LINE_LENGTH)
                                        .Set(Am_Y1, 0)
                                        .Set(Am_Y2, 0)
                                        .Set(Am_LINE_STYLE, Am_Line_8)
                                        .Add(LINE_STYLE_KEY, 5)))
                      .Set(Am_VALUE, first_line)
                      .Get_Object(Am_COMMAND)
                      .Set(Am_DO_METHOD, linestyler)
                      .Set(Am_UNDO_METHOD, undo_linestyler)
                      .Set(Am_REDO_METHOD, undo_linestyler)
                      .Get_Owner());

  Am_Object mid_size = Am_Text.Create()
                           .Set(Am_TEXT, "Font_Medium")
                           .Set(Am_FONT, fontarray[0][0][0])
                           .Add(FONT_KEY, 1);

  Am_Object font_group =
      Am_Group.Create()
          .Set(Am_WIDTH, 100)
          .Set(Am_LEFT, 0)
          .Set(Am_HEIGHT, Am_Height_Of_Parts)
          .Add_Part(Am_Border_Rectangle.Create()
                        .Set(Am_FILL_STYLE, Am_Amulet_Purple)
                        .Set(Am_TOP, 0)
                        .Set(Am_LEFT, 0)
                        .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH))
                        .Set(Am_HEIGHT, get_owner_height_if_ready))
          .Add_Part(Am_Text.Create()
                        .Set(Am_TOP, 3)
                        .Set(Am_LEFT, 8)
                        .Set(Am_TEXT, "Font Setting")
                        .Set(Am_FONT, fontarray[0][0][0]))
          .Add_Part(Font_Type_Button =
                        Am_Option_Button.Create("Font_Type_Button")
                            .Set(Am_LEFT, 5)
                            .Set(Am_TOP, 13)
                            .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH, -10))
                            .Add(FONT_KEY, 0)
                            .Set(Am_H_ALIGN, Am_CENTER_ALIGN)
                            .Set(Am_ITEMS,
                                 Am_Value_List()
                                     .Add(Am_Text.Create()
                                              .Set(Am_TEXT, "FIXED")
                                              .Set(Am_FONT, fontarray[0][1][0])
                                              .Add(FONT_KEY, 0))
                                     .Add(Am_Text.Create()
                                              .Set(Am_TEXT, "SERIF")
                                              .Set(Am_FONT, fontarray[1][0][0])
                                              .Add(FONT_KEY, 1))
                                     .Add(Am_Text.Create()
                                              .Set(Am_TEXT, "SANS_SERIF")
                                              .Set(Am_FONT, fontarray[2][0][0])
                                              .Add(FONT_KEY, 2)))
                            .Get_Object(Am_COMMAND)
                            .Set(Am_DO_METHOD, fontstyler)
                            .Set(Am_UNDO_METHOD, undo_fontstyler)
                            .Get_Owner())
          .Add_Part(Font_Size_Button =
                        Am_Option_Button.Create("Font_Size_Button")
                            .Set(Am_LEFT, 5)
                            .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH, -10))
                            .Set(Am_TOP, 40)
                            .Add(FONT_KEY, 1)
                            .Set(Am_ITEMS,
                                 Am_Value_List()
                                     .Add(Am_Text.Create()
                                              .Set(Am_TEXT, "Font_Small")
                                              .Set(Am_FONT, fontarray[0][0][0])
                                              .Add(FONT_KEY, 0))
                                     .Add(mid_size)
                                     .Add(Am_Text.Create()
                                              .Set(Am_TEXT, "Font_Large")
                                              .Set(Am_FONT, fontarray[0][0][0])
                                              .Add(FONT_KEY, 2))
                                     .Add(Am_Text.Create()
                                              .Set(Am_TEXT, "Font_Very_Large")
                                              .Set(Am_FONT, fontarray[0][0][0])
                                              .Add(FONT_KEY, 3)))
                            .Set(Am_VALUE, mid_size)
                            .Get_Object(Am_COMMAND)
                            .Set(Am_DO_METHOD, fontstyler)
                            .Set(Am_UNDO_METHOD, undo_fontstyler)
                            .Get_Owner())
          .Add_Part(
              Font_Style_Button =
                  Am_Button_Panel.Create()
                      .Set(Am_TOP, 70)
                      .Set(Am_LEFT, center_self)
                      .Set(Am_HEIGHT, 30)
                      .Set(Am_LAYOUT, Am_Horizontal_Layout)
                      .Add(FONT_KEY, 0)
                      .Set(Am_H_ALIGN, Am_CENTER_ALIGN)
                      .Set(Am_HOW_SET, Am_CHOICE_LIST_TOGGLE)
                      .Set(Am_FINAL_FEEDBACK_WANTED, true)
                      .Set(Am_ITEMS, Am_Value_List().Add("B").Add("I").Add("U"))
                      .Get_Object(Am_COMMAND)
                      .Set(Am_DO_METHOD, fontstyler)
                      .Set(Am_UNDO_METHOD, undo_fontstyler)
                      .Set(Am_REDO_METHOD, undo_fontstyler)
                      .Get_Owner());

  win =
      Am_Window.Create("Gilt Main Window")
          .Set(Am_HEIGHT, Am_Height_Of_Parts)
          .Set(Am_WIDTH, PANEL_WIDTH)
          .Set(Am_TITLE, "Gilt: Amulet's Interface Builder")
          .Set(Am_ICON_TITLE, "Gilt")
          .Set(Am_DESTROY_WINDOW_METHOD, Am_Window_Destroy_And_Exit_Method)
          .Set(Am_FILL_STYLE, Am_Amulet_Purple)
          .Add_Part(
              Am_Group.Create()
                  .Set(Am_HEIGHT, Am_Height_Of_Parts)
                  .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH))
                  .Set(Am_TOP, 0)
                  .Set(Am_LEFT, 0)
                  .Add_Part(tool_panel.Set(Am_TOP, 3).Set(Am_LEFT, 3))
                  .Add_Part(
                      color_line_group
                          .Set(Am_TOP, ((int)tool_panel.Get(Am_HEIGHT) + 3))
                          .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH)))
                  .Add_Part(
                      font_group
                          .Set(Am_TOP,
                               Am_From_Object(
                                   color_line_group, Am_TOP,
                                   ((int)color_line_group.Get(Am_HEIGHT) + 3)))
                          .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH)))
                  .Add_Part(
                      Am_Button_Panel.Create()
                          .Set(Am_TOP,
                               Am_From_Object(
                                   font_group, Am_TOP,
                                   ((int)font_group.Get(Am_HEIGHT) + 3)))
                          .Set(Am_LEFT, 0)
                          .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH))
                          .Set(Am_ACTIVE, true)
                          .Set(Am_LAYOUT, Am_Horizontal_Layout)
                          .Set(Am_ITEMS,
                               Am_Value_List()
                                   .Add(Am_Open_Command.Create()
                                            .Set(Am_HANDLE_OPEN_SAVE_METHOD,
                                                 use_file_contents)
                                            .Set(Am_LABEL, "Open"))
                                   .Add(Am_Command.Create()
                                            .Set(Am_DO_METHOD, create_new)
                                            .Set(Am_LABEL, "New"))
                                   .Add(Am_Quit_No_Ask_Command.Create()))));

  int t_h = tool_panel.Get(Am_HEIGHT);
  int win_w = win.Get(Am_WIDTH);

  Fill_Style_Panel =
      Am_Window.Create()
          .Set(Am_WIDTH, Am_Width_Of_Parts)
          .Set(Am_HEIGHT, Am_Height_Of_Parts)
          .Set(Am_TOP, Am_From_Object(win, Am_TOP, (t_h + 10)))
          .Set(Am_LEFT, Am_From_Object(win, Am_LEFT, (win_w + 10)))
          .Set(Am_VISIBLE, false)
          .Set(Am_DESTROY_WINDOW_METHOD,
               Am_Default_Pop_Up_Window_Destroy_Method)
          .Add_Part(Am_Map.Create("fill_styles")
                        .Set(Am_TOP, 0)
                        .Set(Am_LEFT, 0)
                        .Set(Am_MAX_RANK, 4)
                        .Set(Am_LAYOUT, Am_Horizontal_Layout)
                        .Set(Am_ITEM_PROTOTYPE,
                             Am_Rectangle.Create()
                                 .Set(Am_WIDTH, 20)
                                 .Set(Am_HEIGHT, 15)
                                 .Add(Am_V_SPACING, 3)
                                 .Add(Am_H_SPACING, 3)
                                 .Set(Am_FILL_STYLE, get_fill_style))
                        .Set(Am_ITEMS, FILL_STYLE_CNT)
                        .Add_Part(Am_Choice_Interactor.Create()
                                      .Set(Am_HOW_SET, Am_CHOICE_SET)
                                      .Get_Object(Am_COMMAND)
                                      .Set(Am_DO_METHOD, choose_color)
                                      .Get_Owner()));

  Line_Color_Panel =
      Am_Window.Create()
          .Set(Am_WIDTH, Am_Width_Of_Parts)
          .Set(Am_HEIGHT, Am_Height_Of_Parts)
          .Set(Am_TOP, Am_From_Object(win, Am_TOP, (t_h + 40)))
          .Set(Am_LEFT, Am_From_Object(win, Am_LEFT, (win_w + 10)))
          .Set(Am_VISIBLE, false)
          .Set(Am_DESTROY_WINDOW_METHOD,
               Am_Default_Pop_Up_Window_Destroy_Method)
          .Add_Part(Am_Map.Create("Line_Colors")
                        .Set(Am_TOP, 0)
                        .Set(Am_LEFT, 0)
                        .Set(Am_MAX_RANK, 5)
                        .Set(Am_LAYOUT, Am_Horizontal_Layout)
                        .Set(Am_ITEM_PROTOTYPE,
                             Am_Rectangle.Create()
                                 .Set(Am_WIDTH, 20)
                                 .Set(Am_HEIGHT, 15)
                                 .Add(Am_V_SPACING, 3)
                                 .Add(Am_H_SPACING, 3)
                                 .Set(Am_FILL_STYLE, get_line_color))
                        .Set(Am_ITEMS, LINE_COLOR_CNT)
                        .Add_Part(Am_Choice_Interactor.Create()
                                      .Set(Am_HOW_SET, Am_CHOICE_SET)
                                      .Get_Object(Am_COMMAND)
                                      .Set(Am_DO_METHOD, choose_color)
                                      .Get_Owner()));

  cout << "*************** Main Window Created ************" << endl;
  global_undo_handler = Am_Multiple_Undo_Object.Create();
  win.Set(Am_UNDO_HANDLER, undo_handler_if_not_running);

  Am_Object win2 = main_window_proto.Create();

  if (argc > 1) {
    Am_Object obj = win2.Get_Object(OPEN_COMMAND);
    Am_String s = (char *)argv[1];
    Am_Standard_Open_From_Filename(obj, s);
    win2.Destroy();
  } else {
    win2.Set(Am_UNDO_HANDLER, Am_Multiple_Undo_Object.Create());
    Selection_Widget_List.Add(
        win2.Get_Object(MAIN_GROUP).Get_Object(SEL_WIDGET));
    main_window_proto.Get_Object(MAIN_GROUP)
        .Get_Object(SEL_WIDGET)
        .Set(Am_MULTI_SELECTIONS, Selection_Widget_List);
    window_list.Add(win2);
    Am_Screen.Add_Part(win2);
  }

  Am_Screen.Add_Part(win);
  Am_Main_Event_Loop();
  Am_Cleanup();
  return 0;
}

/*
  
  TO DO:
  
  * make undoable:
  nudges
  setting fillstyle, linestyle, font
  
  * better font setting
  * better linestyle setting
  * better organization for fill colors
  * fix panel dialog so easier to enter multiple names
  
  * drag and drop so can put things into scrolling windows?
  * drag and drop so can reorder the names in menus
  
  * ability to include standard commands?
  * ability to set DO_methods?
  
  * Menubars -- how set sub-menus?
  
  * support multiple windows??
  
* double click on mode stay in that mode?

* From "Robert M. Muench" <100606.2653@Computerve.COM>

Some questions for GILT: Is it possible to read in the description and
build the dialog on the fly in the client program? This would enable
something like a device-independet way to save a layout and create it
on the fly.

How to synchronize interactive GUI building and custom code which was
added afterwards? I can imagine that I can use GILT to build a
dialog-box etc. and let GILT generate the source-code. Is it possible
to hold all custom-code in an other file so that the GILT file can be
regenerated if the dialog-box changed?

*/
