////////////////////////////////////////////////////////////////////////
// UNDO stuff
////////////////////////////////////////////////////////////////////////

//set this into the Am_IMPLEMENTATION_PARENT of operations like
//selection and scrolling that are not usually queued for undoing
#define Am_NOT_USUALLY_UNDONE -1

//set this into the Am_IMPLEMENTATION_CHILD if don't want children
//commands undone when this is undone
#define Am_DONT_UNDO_CHILDREN -2

////////////////////////////////////////////////////////////////////////
// The UNDO objects you can make instances of.  These should be in the
//  Am_UNDO_HANDLER slots of either a window or an application object
//  which is in the Am_APPLICATION slot of a window

_OA_DL_IMPORT extern Am_Object Am_Undo_Handler; // general, prototype undo handler obj
_OA_DL_IMPORT extern Am_Object Am_Single_Undo_Object;    // can only undo last command
_OA_DL_IMPORT extern Am_Object Am_Multiple_Undo_Object;  // can undo all top-level commands

// Methods in UNDO objects include REGISTER_COMMAND, PERFORM_UNDO,
// PERFORM_UNDO_THE_UNDO

// REGISTER_COMMAND takes a procedure of the form:
AM_DEFINE_METHOD_TYPE(Am_Register_Command_Method, bool,
		      (Am_Object undo_handler, Am_Object command_obj))

// PERFORM_UNDO and UNDO_THE_UNDO slots are of type
//  Am_Object_Method (in types.h)

//////  Method types for selective undo
// For the Am_SELECTIVE_UNDO_ALLOWED and Am_SELECTIVE_REPEAT_SAME_ALLOWED
// slots:
AM_DEFINE_METHOD_TYPE(Am_Selective_Allowed_Method, bool,
		      (Am_Object command_obj))

// For the Am_SELECTIVE_REPEAT_NEW_ALLOWED slot:
// One or both of new_selection or new_value can be Am_No_Value which
// means this parameter should NOT use the new one
AM_DEFINE_METHOD_TYPE(Am_Selective_New_Allowed_Method, bool,
		      (Am_Object command_obj, Am_Value new_selection,
		       Am_Value new_value))

// for the Am_SELECTIVE_REPEAT_ON_NEW_METHOD slot.
// One or both of new_selection or new_value can be Am_No_Value which
// means this parameter should NOT use the new one
AM_DEFINE_METHOD_TYPE(Am_Selective_Repeat_New_Method, void,
		      (Am_Object command_obj, Am_Value new_selection,
		       Am_Value new_value))

// for the Am_SELECTIVE_UNDO_METHOD and
// Am_SELECTIVE_REPEAT_SAME_METHOD slots of the UNDO HANDLER
// returns the copy of command_obj that is put onto the undo history
AM_DEFINE_METHOD_TYPE(Am_Handler_Selective_Undo_Method, Am_Object,
		      (Am_Object undo_handler, Am_Object command_obj))

// for the Am_SELECTIVE_REPEAT_ON_NEW_METHOD slot of the UNDO HANDLER
// returns the copy of command_obj that is put onto the undo history
AM_DEFINE_METHOD_TYPE(Am_Handler_Selective_Repeat_New_Method, Am_Object,
		      (Am_Object undo_handler, Am_Object command_obj,
		       Am_Value new_selection, Am_Value new_value))

_OA_DL_IMPORT extern Am_Selective_Allowed_Method Am_Selective_Allowed_Return_True;
_OA_DL_IMPORT extern Am_Selective_Allowed_Method Am_Selective_Allowed_Return_False;
_OA_DL_IMPORT extern Am_Selective_New_Allowed_Method 
			Am_Selective_New_Allowed_Return_False;
_OA_DL_IMPORT extern Am_Selective_New_Allowed_Method 
			Am_Selective_New_Allowed_Return_True;


////////////////////////////////////////////////////////////////////////
// For the UNDO mechanisms
////////////////////////////////////////////////////////////////////////

//returns the undo handler attached to the window of inter_or_widget
extern Am_Object Am_Inter_Find_Undo_Handler(const Am_Object& inter_or_widget);

// command_obj's DO has already been "done", now do the DO methods of
// all the Am_IMPLEMENTATION_PARENT's of command_obj, recursively on up
extern void Am_Process_All_Parent_Commands(Am_Object& command_obj,
			   Am_Object& undo_handler,
			   const Am_Register_Command_Method &reg_method);

// Call the command's do method and process all the parents as well.  The
// widget parameter is provided to find the needed undo handler.
extern void Am_Execute_Command (Am_Object& command, const Am_Object& widget);

extern void Am_Modify_Command_Label_Name(Am_Object cmd, Am_Slot_Key slot);

//execute the slot's method (usually undo, selective_undo,
//selective_repeat, etc.) on the command and all of its
//implementation_parent commands
extern Am_Object am_perform_on_cmd_and_impls(Am_Object & last_command,
					 Am_Slot_Key slot, bool copy = false,
					 Am_Value new_sel = Am_No_Value,
					 Am_Value new_val = Am_No_Value,
					 bool fix_name = true);

