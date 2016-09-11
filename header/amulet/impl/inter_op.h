#include IDEFS__H
#include "inter_misc.h"
#include "inter_input.h"

// class Am_Input_Char;
class Am_Inter_Location;
class Am_Value_List;

// Other functions

//Explicitly abort an interactor.  If not running, does nothing.
_OA_DL_IMPORT extern void Am_Abort_Interactor(Am_Object inter, bool update_now = true);

//Explicitly stop an interactor.  If not running, raises an error.
//If not supply stop_obj, uses last one from inter.  
// If not supply stop_window, uses stop_obj's and sets stop_x and stop_y
//to stop_obj's origin.  If supply stop_window, must also supply stop_x _y
_OA_DL_IMPORT extern void Am_Stop_Interactor(Am_Object inter,
			       Am_Object stop_obj = Am_No_Object,
			       Am_Input_Char stop_char = Am_Default_Stop_Char,
			       Am_Object stop_window = Am_No_Object, int stop_x = 0,
			       int stop_y = 0, bool update_now = true);

//Explicitly start an interactor.  If already running, does nothing.
//If not supply start_obj, uses inter's owner.
// If not supply start_window, uses start_obj's and sets start_x and start_y
//to start_obj's origin.  If supply start_window, must also supply start_x _y
_OA_DL_IMPORT extern void Am_Start_Interactor(Am_Object inter,
			Am_Object start_obj = Am_No_Object,
		       	Am_Input_Char start_char = Am_Default_Start_Char,
			Am_Object start_window = Am_No_Object, int start_x = 0,
			int start_y = 0, bool update_now = true);

/// The Am_Clip_And_Map procedure works as follows:
///    (Am_Clip_And_Map val, val_1, val_2, target_val_1, target_val_2) takes
///    val, clips it to be in the range val_1 .. val_2, and if target_val_1 and
///    target_val_2 are provided, then scales and
///    translates the value (using linear_interpolation) to be between
///    target_val_1 and target_val_2.  There are integer and float versions of
//     this function.
///   val_1 is allowed to be less than or greater than val_2.

_OA_DL_IMPORT extern long Am_Clip_And_Map(long val, long val_1, long val_2, 
			   long target_val_1 = 0, long target_val_2 = 0);
_OA_DL_IMPORT extern float Am_Clip_And_Map(float val, float val_1, float val_2,
			     float target_val_1 = 0.0,
			     float target_val_2 = 0.0);

_OA_DL_IMPORT extern void Am_Choice_Set_Value (Am_Object inter, bool set_selected);

//copies Am_VALUE, Am_OBJECT_MODIFIED and Am_SAVED_OLD_OWNER from
//from_object to its Am_COMMAND part
_OA_DL_IMPORT extern void Am_Copy_Values_To_Command(Am_Object from_object);


/////// used for multi-window moves (move-grow and new-points)

// Checks if the window that feedback is in matches the
// window in the interactor, and if not, moves the feedback object to the
// corresponding owner in Am_MULTI_FEEDBACK_OWNERS or Am_MULTI_OWNERS
// returns the owner if change something
_OA_DL_IMPORT extern Am_Object Am_Check_And_Fix_Feedback_Group (Am_Object& feedback,
					     const Am_Object& inter);

// Checks if the window that object is in matches the
// window in interactor, and if not, moves
// the object to the corresponding owner in Am_MULTI_OWNERS.
// Returns true if everything is OK.  Returns false if should abort.
// Sets new_owner if changes the owner, otherwise new_owner is Am_No_Object
_OA_DL_IMPORT extern bool Am_Check_And_Fix_Object_Group (Am_Object& obj,
					   const Am_Object &inter,
					   Am_Object &new_owner);

// Like Check_And_Fix_Object_Group this procedure determines what group the
// given object belongs to.  This returns the group instead of automatically
// setting it.  Returns Am_No_Object instead of returning false.
_OA_DL_IMPORT extern Am_Object Am_Find_Destination_Group (const Am_Object& obj,
					    const Am_Object& inter);

//Used when interactor needs to be added or removed from an extra
//window's list, for example when the feedback object is a window
//itself, call this on the window  If add and want_move, then turns
//on mouse-moved events for the window.
_OA_DL_IMPORT extern void Am_Add_Remove_Inter_To_Extra_Window(Am_Object inter,
						Am_Object window, bool add,
						bool want_move);

//Use when Feedback is a window.  Make sure window in inter isn't feedback,
//and if so, find a new window and return it (also fixes the window
//and interim_x and _y in the inter).  Call this AFTER feedback
//has disappeared.  Otherwise, just returns inter_window.
extern Am_Object
  Am_Check_And_Fix_Point_In_Feedback_Window(Am_Object &inter,
					    Am_Object &feedback);

//Use to replace owner of object when undoing and redoing.  Makes sure owner
// of obj is data.ref_obj and if not, moves it
_OA_DL_IMPORT extern bool Am_Check_And_Fix_Owner_For_Object(Am_Object &obj,
					      Am_Inter_Location &data);

//returns true if valid.  I.e. returns false if object is marked as
//invalid for slot_for_inactive
_OA_DL_IMPORT extern bool Am_Check_One_Object_For_Inactive(Am_Object &object,
	      Am_Slot_Key slot_for_inactive);
//returns false if object is marked as invalid for slot_for_inactive
//uses Am_SLOT_FOR_THIS_COMMAND_INACTIVE of self
_OA_DL_IMPORT extern bool Am_Check_One_Object_For_Inactive_Slot(Am_Object &object,
		   Am_Object &self);

_OA_DL_IMPORT extern bool Am_Check_All_Objects_For_Inactive(Am_Value_List &selection,
	      Am_Slot_Key slot_for_inactive);
_OA_DL_IMPORT extern bool Am_Check_All_Objects_For_Inactive_Slot(Am_Value_List &selection,
		   Am_Object &self);
_OA_DL_IMPORT extern bool Am_Check_One_Or_More_For_Inactive(Am_Value list_or_obj,
	      Am_Slot_Key slot_for_inactive);
_OA_DL_IMPORT extern bool Am_Check_One_Or_More_For_Inactive_Slot(Am_Value list_or_obj,
		   Am_Object &self);

//returns false is inactive so should abort, true if OK
_OA_DL_IMPORT extern bool Am_Check_Inter_Abort_Inactive_Object(Am_Object &object,
					 Am_Slot_Key slot_for_inactive,
					 Am_Object &inter);

/////////////////////////////////////////////////////////////////////////////
// Generally useful functions
/////////////////////////////////////////////////////////////////////////////

//checks that object & all owner's are visible (all the way to the Am_Screen).
_OA_DL_IMPORT extern bool Am_Object_And_Owners_Valid_And_Visible(Am_Object obj);
_OA_DL_IMPORT extern bool Am_Object_And_Owners_Valid_And_Visible(
						   Am_Object obj);

// calls Am_Object_And_Owners_Valid_And_Visible on each object in
// value, which can be a single object or a value list of objects
_OA_DL_IMPORT bool Am_Valid_and_Visible_List_Or_Object(Am_Value value,
					 bool want_visible = true);

// Applies the gridding, if any, defined by the inter's GRID slots
_OA_DL_IMPORT extern void Am_Get_Filtered_Input(Am_Object inter, const Am_Object& ref_obj,
				  int x, int y, int& out_x, int & out_y);

// sets the left,top,width,height or x1,y1,x2,y2 of obj from data
_OA_DL_IMPORT extern void Am_Modify_Object_Pos(Am_Object& obj, const Am_Inter_Location& data,
				 bool growing);

//turn_off saves the old value of the animator on the left slot, if any, and
//turns it off.  Restore sets it back to its original value.
// extern void Am_Temporary_Turn_Off_Animator(Am_Object &obj);
// extern void Am_Temporary_Restore_Animator(Am_Object &obj);

_OA_DL_IMPORT extern bool Am_Inter_Call_Both_Method(Am_Object& inter, Am_Object& command_obj,
				      Am_Slot_Key method_slot, int x, int y,
				      Am_Object& ref_obj, Am_Input_Char& ic,
				      Am_Object& object_modified,
				      Am_Inter_Location& points);
_OA_DL_IMPORT extern void Am_Inter_Call_Method(Am_Object& inter_or_cmd,
				 Am_Slot_Key method_slot,
				 int x, int y, Am_Object& ref_obj,
				 Am_Input_Char& ic, Am_Object& object_modified,
				 Am_Inter_Location& points);

typedef Am_Object Am_Impl_Command_Setter(Am_Object inter,
					 Am_Object object_modified,
					 Am_Inter_Location data);
_OA_DL_IMPORT extern void Am_Call_Final_Do_And_Register(Am_Object inter,
				  Am_Object command_obj,
				  int x, int y, Am_Object ref_obj,
				  Am_Input_Char ic,
		 		  Am_Object object_modified,
				  Am_Inter_Location data,
				  Am_Impl_Command_Setter* impl_command_setter);
_OA_DL_IMPORT extern void Am_Register_For_Undo(Am_Object inter, Am_Object command_obj,
				 Am_Object object_modified,
				 Am_Inter_Location data,
				 Am_Impl_Command_Setter* impl_command_setter);


//exported for when want to pretend a move-grow interim method is a
//final method, like for scroll bar indicators (scroll_widgets.cc)
_OA_DL_IMPORT extern void Am_Move_Grow_Register_For_Undo(Am_Object inter);

//Initialize the state if not there yet
_OA_DL_IMPORT extern Am_Inter_State Am_Get_Inter_State(Am_Object inter);
