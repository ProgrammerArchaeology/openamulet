///////////////////////////////////////////////////////////////////////////
// Accelerators for buttons
///////////////////////////////////////////////////////////////////////////

class Am_Input_Char;

_OA_DL_IMPORT extern Am_Object Am_Accelerator_Inter;

//returns Am_No_Object if OK to set an accelerator with that character.
//Returns a command if that character is already in use by that command in
//that window.  You might use the command to print out an error message.
_OA_DL_IMPORT extern Am_Object Am_Check_Accelerator_Char_For_Window(Am_Input_Char accel,
						    Am_Object window);
_OA_DL_IMPORT extern void Am_Add_Accelerator_Command_To_Window(Am_Object command,
						 Am_Object window);
_OA_DL_IMPORT extern void Am_Remove_Accelerator_Command_From_Window(Am_Object command,
						      Am_Object window);

