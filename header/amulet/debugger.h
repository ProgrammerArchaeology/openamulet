/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

// This file contains the inspector for debugging Amulet objects
// Designed and implemented by Brad Myers
//lint -e752

#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <am_inc.h>

#include OBJECT__H
#include IDEFS__H
#include TYPES__H

#include VALUE_LIST__H
#include OBJECT_ADVANCED__H	// for Am_Slot

const float Am_INSPECTOR_INTER_PRIORITY=300.0;
#define Am_INTERNAL_SLOT_PREFIX '~'

//returns true if input can be parsed as a wanted_type.
//If returns true, then output_value is set with the actual value.
//This is implemented in src/debug/input_parser.cc
_OA_DL_IMPORT extern bool Am_Parse_Input_As_Type(Am_String input, Am_Value_Type wanted_type,
				   Am_Value &output_value);

//input must be produced by a Am_Value_List::Print of a single type
_OA_DL_IMPORT extern bool Am_Parse_Input_As_List_Of_Type(Am_String input, 
					   Am_Value_Type wanted_type, 
					   Am_Value &output_value);

// Sets up interactors so inspector can be brought up for any object in window
extern void Am_Initialize_Inspector();
extern void Am_Cleanup_Inspector();

// inspect the specific object, if a slot is supplied, the select it
_OA_DL_IMPORT extern void Am_Inspect(const Am_Object& object, Am_Slot_Key slot = Am_NO_SLOT);

// inspect the specific object and go into a main_loop
_OA_DL_IMPORT extern void Am_Inspect_And_Main_Loop(const Am_Object& object, 
				     Am_Slot_Key slot = Am_NO_SLOT);

// The next one takes the name of the object.  This is useful from the
// interpreter.
_OA_DL_IMPORT extern void Am_Inspect(const char * name); 

_OA_DL_IMPORT extern void Am_Flash (Am_Object o, std::ostream &flashout = std::cout);

_OA_DL_IMPORT extern void Am_Set_Inspector_Keys(Am_Input_Char show_key,
				  Am_Input_Char show_position_key,
				  Am_Input_Char ask_key);

////////////////////////////////////////////////////////////////////
// For tracing
////////////////////////////////////////////////////////////////////

const int Am_NOT_TRACING = 0x00;
const int Am_TRACING     = 0x01;
const int Am_STORING_OLD = 0x02;
const int Am_BREAKING    = 0x04;

typedef short Am_Trace_Status;

_OA_DL_IMPORT void Am_Notify_On_Slot_Set (Am_Object object = Am_No_Object, Am_Slot_Key key = 0,
			    Am_Value value = Am_No_Value);
_OA_DL_IMPORT void Am_Break_On_Slot_Set (Am_Object object = Am_No_Object, Am_Slot_Key key = 0,
			   Am_Value value = Am_No_Value);
_OA_DL_IMPORT void Am_Clear_Slot_Notify (Am_Object object = Am_No_Object, Am_Slot_Key key = 0,
			   Am_Value value = Am_No_Value);
_OA_DL_IMPORT void Am_Start_Slot_Value_Tracing(Am_Object object, Am_Slot_Key key, 
                          Am_Object old_values_object = Am_No_Object);
_OA_DL_IMPORT void Am_Stop_Slot_Value_Tracing(Am_Object object, Am_Slot_Key key);

_OA_DL_IMPORT Am_Trace_Status Am_Update_Tracer_String_Object(Am_Object obj, Am_Slot_Key key,
				    Am_Object string_obj);
_OA_DL_IMPORT void Am_Invalidate_All_Tracer_String_Objects(Am_Object the_obj);
_OA_DL_IMPORT Am_Value_List Am_Get_Tracer_Old_Values(Am_Object obj, Am_Slot_Key key,
				       Am_Object object);
_OA_DL_IMPORT Am_Trace_Status Am_Get_Tracer_Status (Am_Object obj, Am_Slot_Key key);

_OA_DL_IMPORT void Am_Add_Old_Values (Am_Value_List old_values, Am_Object value_string,
			Am_Value_List* group_iter);

void Am_Initialize_Tracer ();

_OA_DL_IMPORT void Am_Refresh_Inspector_If_Object_Changed(const Am_Slot& slot, 
					    Am_Set_Reason reason);

_OA_DL_IMPORT extern void Am_Notify_Or_Break_On_Object_Action(Am_Object old_object,
					 Am_Set_Reason reason, bool do_break);

_OA_DL_IMPORT extern void Am_Notify_Or_Break_On_Method(Am_Object &object,
					 Am_Slot_Key slot_key, bool do_break);

#endif
