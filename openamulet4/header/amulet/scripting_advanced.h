/* ****************************** -*-c++-*- *******************************/
/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* Advanced and internal stuff for scripting
   
   Designed and implemented by Brad Myers
*/
#ifndef SCRIPTING_ADVANCED_H
#define SCRIPTING_ADVANCED_H

#include <am_inc.h>
#include TYPES__H
#include WIDGETS__H 
#include VALUE_LIST__H

_OA_SCRIPT_DL_IMPORT extern bool am_sdebug;

_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_COMMANDS_IN_SCRIPT;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_NAME_OBJECT;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_SCRIPT_EXECUTE_COMMAND;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_SHOW_SCRIPT_COMMAND;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_UNDO_OPTIONS;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_UNDO_MENU_BAR;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_UNDO_SCROLL_GROUP;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_COMMAND_INDEX_IN_SCRIPT;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_SCRIPT_LINE_COMMAND;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_SCRIPT_WINDOW;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_PLACEHOLDER_LIST;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_PLACEHOLDER_COUNT;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_PLACEHOLDER_INITIALIZED;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_COMMANDS_PROTOTYPES;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_LOCATION_PLACEHOLDER;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_ORIGINAL_COMMAND_LIST;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_INVOKE_MATCH_COMMAND;

//IDs for commands in a widget in the dialog box
#define Am_ID_ON_THIS 2
#define Am_ID_ON_SELECTION 3

_OA_SCRIPT_DL_IMPORT extern Am_Object am_get_db_from_list(Am_Object & proto, Am_Value_List& db_list,
				     int & cnt);

_OA_SCRIPT_DL_IMPORT extern Am_Object Am_Script_Window;

extern Am_Object Am_Script_Window_Initialize ();

_OA_SCRIPT_DL_IMPORT extern Am_Object_Method am_script_ok;
_OA_SCRIPT_DL_IMPORT extern Am_Object_Method am_script_remove;
_OA_SCRIPT_DL_IMPORT extern Am_Object_Method am_script_execute;
_OA_SCRIPT_DL_IMPORT extern Am_Object_Method am_execute_script_step_one;

_OA_SCRIPT_DL_IMPORT extern Am_Object_Method script_window_cancel;
_OA_SCRIPT_DL_IMPORT extern Am_Object_Method script_window_destroy_method;
_OA_SCRIPT_DL_IMPORT extern Am_Object_Method am_script_select_members;
_OA_SCRIPT_DL_IMPORT extern Am_Where_Method am_on_selectable_line_part;
_OA_SCRIPT_DL_IMPORT extern Am_Object_Method am_select_line_part;
_OA_SCRIPT_DL_IMPORT extern Am_Object_Method am_clear_select_line_part;

_OA_SCRIPT_DL_IMPORT extern Am_Formula am_generalize_script_active;
_OA_SCRIPT_DL_IMPORT extern Am_Object_Method am_do_generalize_script;
_OA_SCRIPT_DL_IMPORT extern Am_Object_Method am_generalize_script_double_click;

_OA_SCRIPT_DL_IMPORT extern Am_Formula am_insert_commands_ok;
_OA_SCRIPT_DL_IMPORT extern Am_Object_Method am_insert_commands_before;
_OA_SCRIPT_DL_IMPORT extern Am_Object_Method am_insert_commands_after;
_OA_SCRIPT_DL_IMPORT extern Am_Object_Method am_edit_commands_list_undo;
_OA_SCRIPT_DL_IMPORT extern Am_Formula am_delete_commands_ok;
_OA_SCRIPT_DL_IMPORT extern Am_Object_Method am_delete_script_commands;


_OA_SCRIPT_DL_IMPORT extern bool am_find_cmd_in_sel(Am_Value_List &sel_list, Am_Object &target_cmd,
			       Am_Object &found_sel);

_OA_SCRIPT_DL_IMPORT extern void am_write_command_name(Am_Object cmd, int cnt, OSTRSTREAM &oss); 

_OA_SCRIPT_DL_IMPORT extern void Am_Pop_Up_Generalize_DB(Am_Object &script_window,
				    Am_Value_List &sel_list);

// generalizers

_OA_SCRIPT_DL_IMPORT extern void Am_Initialize_Generalizers();

_OA_SCRIPT_DL_IMPORT extern Am_Object Am_A_Placeholder; //prototype for all the placeholders
_OA_SCRIPT_DL_IMPORT extern Am_Object Am_Constant_Placeholder; //placeholder for constant values
_OA_SCRIPT_DL_IMPORT extern Am_Object Am_Custom_Code_Placeholder;

//for objects
_OA_SCRIPT_DL_IMPORT extern Am_Object Am_All_Values_From_Command_Placeholder;
_OA_SCRIPT_DL_IMPORT extern Am_Object Am_An_Object_From_Command_Placeholder;
_OA_SCRIPT_DL_IMPORT extern Am_Object Am_The_Selected_Objects_Placeholder;
_OA_SCRIPT_DL_IMPORT extern Am_Object Am_User_Select_At_Run_Time_Placeholder;

//for locations
_OA_SCRIPT_DL_IMPORT extern Am_Object Am_Inter_Location_Placeholder;

//for properties and other values
_OA_SCRIPT_DL_IMPORT extern Am_Object Am_Ask_User_Property_Placeholder;
_OA_SCRIPT_DL_IMPORT extern Am_Object Am_Property_From_Palette_Placeholder;
_OA_SCRIPT_DL_IMPORT extern Am_Object Am_Property_From_Object_Placeholder;
_OA_SCRIPT_DL_IMPORT extern Am_Object Am_Cycle_Values_Placeholder;

// match placeholders
_OA_SCRIPT_DL_IMPORT extern Am_Object Am_Type_Match_Placeholder;
_OA_SCRIPT_DL_IMPORT extern Am_Object Am_Any_Value_Placeholder;

//slots in the placeholder objects
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_PLACEHOLDER_REPLACE_METHOD;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_PLACEHOLDER_SET_VALUE_METHOD;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_PLACEHOLDER_CREATE_METHOD;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_PLACEHOLDER_MATCH_METHOD;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_PART_CHAIN;

//set into cmds that have placeholders
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_PLACEHOLDERS_SET_BY_THIS_COMMAND;

_OA_SCRIPT_DL_IMPORT extern Am_Object Am_Script_Line_Group, Am_Script_Line_Command,
  Am_Script_Line_Selectable, Am_Script_Line_Modified,
  Am_Script_Line_Value;

//procedures in generalizers.cc
_OA_SCRIPT_DL_IMPORT extern bool Am_Handle_Placeholders_And_Call_Command(int cnt, Am_Object &cmd,
		     Am_Selective_Allowed_Method &same_allowed_method,
		     Am_Handler_Selective_Undo_Method &execute_method,
		     Am_Selective_New_Allowed_Method &same_method,
		     Am_Handler_Selective_Repeat_New_Method &new_method,
		     Am_Object & sel_widget,
		     Am_Object & undo_handler,
		     Am_Object & new_cmd);
_OA_SCRIPT_DL_IMPORT extern void am_check_obj_modified(Am_Object &in_cmd,
			   Am_Value_List &place_holder_list,
			   Am_Value_List &created_objects_assoc, int &cnt);
_OA_SCRIPT_DL_IMPORT extern void am_check_start_obj_parameters(Am_Object &in_cmd,
			   Am_Value_List &place_holder_list,
			   Am_Value_List &created_objects_assoc, int &cnt);
_OA_SCRIPT_DL_IMPORT extern void am_set_commands_into_script_window(Am_Object &script_window,
					       Am_Object & execute_command);

Am_Define_Method_Type(Am_Placeholder_Replace_Method, bool,
		      (Am_Object &ph, Am_Value &new_value,
		       Am_Object & sel_widget);
Am_Define_Method_Type(Am_Placeholder_Set_Value_Method, bool,
		      (Am_Object &ph, Am_Object & new_cmd,
		       Am_Object & sel_widget)));
Am_Define_Method_Type(Am_Placeholder_Create_Method, Am_Object,
		      (Am_Object &create_cmd, Am_Value for_value,
		       Am_Value_List& part_chain, int &cnt,
		       Am_Slot_Key slot));


_OA_SCRIPT_DL_IMPORT extern Am_Object am_askselectobj;
extern Am_Object am_askselectobj_Initialize ();


//object generalization

_OA_SCRIPT_DL_IMPORT extern Am_Object am_objgen;
extern Am_Object am_objgen_Initialize ();

_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_CONSTANT_OBJECTS_WIDGET;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_INDEXES_OF_COMMANDS_WIDGET;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_SOME_OBJECTS_WIDGET;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_ALL_FROM_COMMAND_WIDGET;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_GREEN_AND_YELLOW;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_COMMAND_LOAD_BUTTON;

_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_EXECUTE_COUNTER;

#define am_constant_generalize 1
#define am_selected_generalize 2
#define am_all_values_generalize 4
#define am_the_objects_generalize 5
#define am_custom_generalize 6

#define am_offset_generalize 7

#define am_ask_user_base 100
#define am_ask_user_generalize 101
#define am_ask_user_type 102
#define am_all_objects_generalize 103


#define am_generalize_only_green 1
#define am_generalize_green_and_yellow 2

_OA_SCRIPT_DL_IMPORT extern Am_Object_Method am_objgen_ok;
_OA_SCRIPT_DL_IMPORT extern Am_Object_Method am_objgen_cancel;
_OA_SCRIPT_DL_IMPORT extern Am_Object_Method am_load_current_command;

_OA_SCRIPT_DL_IMPORT extern Am_Formula am_active_if_constant_sel;
_OA_SCRIPT_DL_IMPORT extern Am_Formula am_active_the_object_sel;
_OA_SCRIPT_DL_IMPORT extern Am_Formula am_active_all_values_sel;
_OA_SCRIPT_DL_IMPORT extern Am_Formula am_active_all_or_the_object_sel;

// location generalization

#define am_change_equals 10
#define am_change_offset 11
#define am_change_offset_left_outside 12
#define am_change_offset_left_inside 13
#define am_change_offset_centered 14
#define am_change_offset_right_inside 15
#define am_change_offset_right_outside 16
#define am_change_offset_top_outside 17
#define am_change_offset_top_inside 18
#define am_change_offset_bottom_inside 19
#define am_change_offset_bottom_outside 20
#define am_change_object_wh 22
#define am_change_object_percent 23


class am_loc_values {
public:
  int left_control, top_control, width_control, height_control;
  int left_equals, left_offset, left_object_offset, left_centered;
  int top_equals, top_offset, top_object_offset, top_centered;
  int width_equals, width_offset, width_object_offset,
    width_object_percent;
  int height_equals, height_offset, height_object_offset,
    height_object_percent, height_same_as_width;

  bool growing;
  Am_Inter_Location orig_value;
  
  am_loc_values (); //default values
};

_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_CHANGE_LEFT;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_LEFT_HOW;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_LEFT_AMOUNT_WIDGET;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_CHANGE_TOP;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_TOP_HOW;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_TOP_AMOUNT_WIDGET;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_CHANGE_WIDTH;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_WIDTH_HOW;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_WIDTH_AMOUNT_WIDGET;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_CHANGE_HEIGHT;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_HEIGHT_HOW;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_HEIGHT_AMOUNT_WIDGET;

_OA_SCRIPT_DL_IMPORT extern Am_Object am_locgen;
extern Am_Object am_locgen_Initialize ();

_OA_SCRIPT_DL_IMPORT extern Am_Formula am_active_if_change_left;
_OA_SCRIPT_DL_IMPORT extern Am_Formula am_active_if_change_top;
_OA_SCRIPT_DL_IMPORT extern Am_Formula am_active_if_change_width;
_OA_SCRIPT_DL_IMPORT extern Am_Formula am_active_if_change_height;
_OA_SCRIPT_DL_IMPORT extern Am_Formula am_active_if_need_object;

_OA_SCRIPT_DL_IMPORT extern Am_Object_Method am_locgen_ok;
_OA_SCRIPT_DL_IMPORT extern Am_Object_Method am_locgen_cancel;
_OA_SCRIPT_DL_IMPORT extern Am_Object_Method am_loc_from_object;


_OA_SCRIPT_DL_IMPORT extern Am_Object am_askclickpos;
extern Am_Object am_askclickpos_Initialize ();

// value generalization

_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_REGISTRY_FOR_PALETTES;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_NEXT_CONSTANT_OBJECTS_WIDGET;

#define am_palette_generalize 24
#define am_next_value_generalize 25

_OA_SCRIPT_DL_IMPORT extern Am_Formula am_active_if_next_value_sel;

_OA_SCRIPT_DL_IMPORT extern Am_Object am_valgen;
extern Am_Object am_valgen_Initialize ();

_OA_SCRIPT_DL_IMPORT extern Am_Object_Method am_valgen_ok;
_OA_SCRIPT_DL_IMPORT extern Am_Object_Method am_valgen_cancel;

_OA_SCRIPT_DL_IMPORT extern Am_Object am_askforvalue;
extern Am_Object am_askforvalue_Initialize ();

_OA_SCRIPT_DL_IMPORT extern Am_Object Am_Find_Palette_For(Am_Object& undo_handler,
		     Am_Am_Slot_Key key_wrapper, Am_Value value);

_OA_SCRIPT_DL_IMPORT extern Am_Text_Check_Legal_Method am_check_valid_type;

_OA_SCRIPT_DL_IMPORT extern void am_check_list_slot(Am_Slot_Key slot, Am_Value& v,
			       Am_Object &created_obj,  
			       Am_Value_List& slots_to_save,
			       Am_Value_List &pos_slots);

_OA_SCRIPT_DL_IMPORT extern Am_Object am_invoke_window;
extern Am_Object am_invoke_window_Initialize ();
_OA_SCRIPT_DL_IMPORT extern Am_Object_Method pop_up_invoke_options;

#define am_before_command 51
#define am_after_command 52

_OA_SCRIPT_DL_IMPORT extern Am_String am_gen_command_label(Am_Object &for_cmd);
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_BEFORE_OR_AFTER;
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_BEFORE_OR_AFTER_COMMAND;

_OA_SCRIPT_DL_IMPORT extern Am_Object am_gen_one_script_line(Am_Object & cmd);

// value generalization for Matching

#define am_type_generalize 31
#define am_any_value_generalize 32

_OA_SCRIPT_DL_IMPORT extern Am_Formula am_active_if_type_sel;

_OA_SCRIPT_DL_IMPORT extern Am_Object am_valinvokegen;
extern Am_Object am_valinvokegen_Initialize ();

_OA_SCRIPT_DL_IMPORT extern Am_Object_Method am_valinvokegen_ok;
_OA_SCRIPT_DL_IMPORT extern Am_Object_Method am_valinvokegen_cancel;

_OA_SCRIPT_DL_IMPORT extern Am_Object am_commandinvokegen;
extern Am_Object am_commandinvokegen_Initialize ();

_OA_SCRIPT_DL_IMPORT extern Am_Object_Method am_commandinvokegen_ok;
_OA_SCRIPT_DL_IMPORT extern Am_Object_Method am_commandinvokegen_cancel;

_OA_SCRIPT_DL_IMPORT extern bool am_get_placeholder_from(Am_Value &current_val, Am_Object &ph);

// searching window

_OA_SCRIPT_DL_IMPORT extern bool am_find_proto_and_name(Am_Object &target_obj, Am_Assoc& answer);
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_TYPE_OBJECTS_WIDGET;

_OA_SCRIPT_DL_IMPORT extern void am_script_replace_all(Am_Value_List &sel_list,
			Am_Value &new_val,
			Am_Object &script_window, Am_Slot_Key slot,
			Am_Object &execute_command);
_OA_SCRIPT_DL_IMPORT extern Am_Object am_call_create_placeholder (Am_Object proto,
				      Am_Value& objs, Am_Object &create_cmd,
				      Am_Value_List & placeholder_list,
				      Am_Value_List & part_chain, 
				      int & cnt, Am_Slot_Key slot);

_OA_SCRIPT_DL_IMPORT extern bool am_install_match_command(Am_Object &match_command,
				     Am_Object &script_execute_command);
_OA_SCRIPT_DL_IMPORT extern bool am_remove_match_command(Am_Object &match_command);

#endif
