#include "amulet/impl/widget_misc.h"
class Computed_Colors_Record;
class Am_Value_List;
class Am_Widget_Look;
class Am_Alignment;
class Am_Style;
class Am_Font;
class Am_Formula;
struct am_rect;
class Am_Invalid_Method;
class Am_Translate_Coordinates_Method;
class Am_Point_In_Or_Self_Method;
class Am_Object_Method;

/******************************************************************************
 * PROTOTYPES
 *****************************************************************************/
int calculate_button_fringe( Am_Widget_Look look, bool leave_room,
  bool key_sel, bool def );

int
set_parts_list_commands_old_owner(

  Am_Value_List &parts,
  Am_Object &widget );

Am_Style compute_text_style(bool active, bool depressed,
			    const Computed_Colors_Record& color_rec,
			    Am_Widget_Look look,
			    Am_Button_Type type);
void
Am_Draw_Button_Widget(
  int left, int top, int width, int height,  // dimensions
  const char* string, Am_Object obj,
  bool interim_selected, bool selected,
  bool active, bool key_selected,
  bool is_default, bool fringe,
  Am_Font font,
  const Computed_Colors_Record& color_rec,
  Am_Widget_Look look,
  Am_Button_Type type,
  Am_Drawonable* draw,
  int box_width, int box_height, bool box_on_left,
  Am_Alignment align, int offset_left, bool mask = false );
int Am_Draw_Push_Button_Box( am_rect r, Am_Drawonable* draw,
  Am_Widget_Look look, bool selected, bool key_selected,
  const Computed_Colors_Record& color_rec );
void Am_Draw_Check_Button_Box( am_rect r, Am_Drawonable* draw,
  Am_Widget_Look look, bool selected, bool interim_selected,
  const Computed_Colors_Record& color_rec, bool mask );
void Am_Draw_Radio_Button_Box( am_rect r, Am_Drawonable* draw,
  Am_Widget_Look look, bool selected, bool interim_selected,
  const Computed_Colors_Record& color_rec, bool mask );




void
Am_Draw_Windows_Box(
  int x, int y, int w, int h,
  bool depressed,
  const Computed_Colors_Record& rec, Am_Drawonable* draw );

void
set_command_from_button(
  Am_Object parent_command,
  Am_Object button );

void get_inter_value_from_panel_value(const Am_Value& panel_value,
                      const Am_Object& panel, Am_Value& value);
void set_popup_win_visible(Am_Object &pop_window, bool vis,
			   Am_Object &main_win = Am_No_Object);

/******************************************************************************
 * CONSTANTS
 *****************************************************************************/
const int kMotBox = 5; const int kMotKeySel = 2; const int kMotDefault = 5;
const int kWinBox = 5; const int kWinKeySel = 0; const int kWinDefault = 0;
const int kMacBox = 5; const int kMacKeySel = 0; const int kMacDefault = 4;

extern Am_Formula Am_Default_From_Command;
extern Am_Formula Am_Get_Label_Or_ID;
extern Am_Formula check_accel_string;
extern Am_Formula Am_Align_From_Box_On_Left;
extern Am_Formula am_checked_from_command;
extern Am_Formula checkbox_width;
extern Am_Formula checkbox_height;
extern Am_Formula button_width;
extern Am_Formula button_height;
extern Am_Formula checkbox_box_width;
extern Am_Formula checkbox_box_height;
extern Am_Formula menu_item_width;
extern Am_Formula menu_item_height;
extern Am_Formula menu_item_left_offset;
extern Am_Formula panel_item_width;
extern Am_Formula panel_item_height;
extern Am_Formula radio_button_diameter;

extern Am_Formula final_feedback_from_owner;
extern Am_Formula look_from_owner;
extern Am_Formula active_from_command_panel;
extern Am_Formula Am_Copy_Item_To_Command;

extern Am_Formula box_on_left_from_owner;
extern Am_Formula box_width_from_owner;
extern Am_Formula box_height_from_owner;

extern Am_Formula option_button_value;
extern Am_Formula get_real_string_from_inter_val;


extern Am_Invalid_Method radio_check_invalid;
extern Am_Translate_Coordinates_Method radio_check_translate_coordinates;
extern Am_Point_In_Or_Self_Method radio_check_point_in_part;
extern Am_Point_In_Or_Self_Method radio_check_point_in_leaf;

extern Am_Object_Method Am_Inter_For_Panel_Do;
// void Am_Inter_For_Panel_Do_proc(Am_Object command_obj);
void copy_item_to_command_proc(Am_Object& panel_item,
				      const Am_Value& value );

extern Am_Object Am_Menu_In_Scrolling_Menu;

extern Am_Object Am_Option_Button_Sub_Window_Proto;

int get_scroll_border_thickness( Am_Object &self);
void clear_save_pos_for_undo_proc(Am_Object command_obj); 

