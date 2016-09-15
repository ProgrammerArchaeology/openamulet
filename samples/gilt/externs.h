#ifndef __EXTERNS_H
#define __EXTERNS_H

#include <amulet.h>
#include <amulet/formula_advanced.h>

extern Am_Slot_Key CUSTOMIZE_METHOD;
extern Am_Slot_Key SUB_LABEL;
// extern Am_Slot_Key Lw_FORMAT;
extern Am_Slot_Key Lw_NAME;
extern Am_Slot_Key FONT_KEY;
extern Am_Slot_Key LAYOUT_KEY;
extern Am_Slot_Key FILL_STYLE_KEY;
extern Am_Slot_Key LINE_STYLE_KEY;
extern Am_Slot_Key TYPE_NAME;
extern Am_Slot_Key SAVE_COMMAND_ITEMS;
extern Am_Slot_Key NAME_OBJ;
extern Am_Slot_Key LABEL_OBJ;
extern Am_Slot_Key RESTRICTED;
extern Am_Slot_Key MIN_VALUE;
extern Am_Slot_Key MAX_VALUE;
extern Am_Slot_Key LOOKS_SELECTED_OBJ;
extern Am_Slot_Key SCROLL_PROPS_OBJ;
extern Am_Slot_Key IMAGE_OBJ;
extern Am_Slot_Key FILE_NAME;
extern Am_Slot_Key CHOICE_PANEL;

extern Am_Slot_Key CONSTRAINT_INFO;
extern Am_Slot_Key WORK_OBJECT;
extern Am_Slot_Key WORK_HANDLE;

extern Am_Object_Method ok_button_pressed_cmd;
extern Am_Object_Method cancel_button_pressed_cmd;

/*****************************  From outline.cc *************/
extern Am_Slot_Key Lw_CHILDREN;
extern Am_Slot_Key Lw_HIDDEN;
extern Am_Slot_Key Lw_HSPACING;
extern Am_Slot_Key Lw_NODEID;
extern Am_Slot_Key Lw_PARENT;
extern Am_Slot_Key Lw_VSPACING;
extern Am_Slot_Key Lw_HITEM;
extern Am_Slot_Key Lw_VITEM;
extern Am_Slot_Key Lw_BITMAP;
extern Am_Slot_Key Lw_LAYOUT;
/***********************************************************/

extern void init_styles();

#define FILL_STYLE_CNT 24
#define LINE_COLOR_CNT 20
#define LINE_THICKNESS_CNT 6
#define FONT_ITEMS 12
#define FONT_CNT 3
#define FONT_SIZE_CNT 4
#define FONT_STYLE_CNT 8

extern Am_Style n2s[];
extern Am_Style n2l[LINE_COLOR_CNT][LINE_THICKNESS_CNT];
extern Am_Font fontarray[FONT_CNT][FONT_SIZE_CNT][FONT_STYLE_CNT];

extern const char *n2lstr[LINE_COLOR_CNT][LINE_THICKNESS_CNT];
extern const char *n2sstr[];
extern const char *layout[];

extern void output_cc_with_header(std::ostream &os_h, std::ostream &os_cc,
                                  bool is_window,
                                  Am_Value_List &top_level_objs_list,
                                  Am_String wingroup_name, int fill_key,
                                  Am_String title, bool explicit_wh, int width,
                                  int height, bool win_fixed_size);
extern void output_cc_no_header(std::ostream &os_cc, bool is_window,
                                Am_Value_List &top_level_objs_list,
                                Am_String wingroup_name, int fill_key,
                                Am_String title, bool explicit_wh, int width,
                                int height, bool win_fixed_size);
extern Am_Object bitmap_proto;
extern Am_Object one_way_arrow_line_proto;
extern Am_Object two_way_arrow_line_proto;

extern Am_String Add_Extension(Am_String in_filename, const char *ext);

extern Am_Object Name_Only_Window;
extern Am_Object Name_Only_Window_Initialize();

extern Am_Object Number_Input_Window;
extern Am_Object Number_Input_Window_Initialize();

extern Am_Object Border_Rect_Window;
extern Am_Object Border_Rect_Window_Initialize();

extern Am_Object About_Gilt_Window;
extern Am_Object About_Gilt_Window_Initialize();
extern Am_Formula run_tool;
extern Am_Object button_proto;
extern Am_Slot_Key MAIN_GROUP;
extern Am_Slot_Key WORK_GROUP;
extern Am_Slot_Key MAIN_GROUP_RECT;
extern Am_Object main_group_rect_proto;

extern Am_Object Name_And_Label_Window;
extern Am_Object Name_And_Image_Window;
extern Am_Object Name_And_Label_Window_Initialize(bool is_image);

extern Am_Object Scroll_Group_Window;
extern Am_Object Scroll_Group_Window_Initialize();

extern Am_Object Image_File_Input;
extern Am_Object Image_Label_Dialog;
extern Am_Object Image_Label_Dialog_Initialize();

extern Am_Slot_Key COMMAND_KEY;
extern Am_Slot_Key NUMBER_OF_NODES;
extern Am_Slot_Key INTER;
extern Am_Slot_Key Lw_GROUP;
extern Am_Slot_Key TEXT1;
extern Am_Slot_Key TEXT2;
extern Am_Slot_Key NEW_DELETE_PANNEL;
extern Am_Slot_Key OUTLINE;
extern Am_Slot_Key CHOICE_INTER;
extern Am_Slot_Key COMMAND_LIST;
extern Am_Slot_Key ACCELERATOR_INPUT;
extern Am_Slot_Key SAVE_MENU_BAR;
extern Am_Slot_Key FAKE_DELETE;

extern Am_Object Menu_Edit_Window;
extern Am_Object Menuedit_Window_Initialize();

extern Am_Formula compute_line_style;

extern const char *n2cr[];
extern const char *n2cl[];
extern Am_Object n2c[];

extern Am_Object menu_proto, node_proto, first_command;
extern Am_Object am_command_proto, am_selection_widget_all_proto, clear_proto,
    clear_all_proto, copy_proto, cut_proto, paste_proto, bottom_proto,
    top_proto, duplicate_proto, group_proto, ungroup_proto, undo_proto,
    redo_proto, quit_proto, open_proto, save_proto, save_as_proto,
    menu_line_proto;

extern void Copy_and_Create_List(Am_Value_List source_list,
                                 Am_Value_List &target_list);

extern void Constraint_Widget_Initialize();
extern Am_Object Constraint_Widget;

extern Am_Object Conflict_Window_Initialize();
extern Am_Object Conflict_Window;
extern Am_Formula Get_Data_And_Adjust(Am_Object object, Am_Slot_Key obj_key,
                                      Am_Slot_Key obj_ref_key, int obj_offset,
                                      float obj_multiplier, Am_Slot_Key key,
                                      float multiplier);
extern Am_Value_List get_depends_list(Am_Formula_Advanced *formula);

extern Am_Object Modified_Selection_Widget;
extern Am_Object Modified_Selection_Widget_Initialize();
extern void modified_set_commands_for_sel(Am_Object &cmd, Am_Object &inter,
                                          Am_Object &widget,
                                          Am_Value clicked_obj,
                                          Am_Value new_value,
                                          Am_Value old_value);
extern void modified_clear_multi_selections(const Am_Object &widget);

#define OBJECT_TEXT 0
#define CONSTRAINT_TEXT 1
#define SIMPLE_TEXT 2

#endif
