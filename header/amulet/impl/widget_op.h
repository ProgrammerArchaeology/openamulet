////////////////////////////////////////////////////////////////////////
// Starting, Aborting and Stopping Widgets
////////////////////////////////////////////////////////////////////////
// (these call Start,Stop, and Abort Interactor on the appropriate
//    internal interactors.

//Explicitly start a widget running.  If already running, does nothing.
//If an initial value is provided, then the widget is started with
//this as its value.  It is up to the programmer to make sure the
//Value is legal for the type of widget.  If no initial_value is
//supplied, the widget is started with its current value, if any.

_OA_DL_IMPORT extern void Am_Start_Widget(Am_Object widget,
                                          Am_Value initial_value = Am_No_Value);

//Explicitly abort a widget, interactor or command object.  Usually,
//this will be called with a command object, and the system will find
//the associated widget or interactor and abort it.
// If not running, does nothing.  Tries to make sure the command
// object is not entered into the command history.

_OA_DL_IMPORT extern void Am_Abort_Widget(Am_Object widget_or_inter_or_command);

//Explicitly stop a widget.  If not running, raises an error.  If
//final_value is supplied, then this is the value used as the value of
//the widget.  If final_value is not supplied, the widget uses its current
//value.  Commands associated with the widget will be invoked just as
//if the widget had terminated normally.
_OA_DL_IMPORT extern void Am_Stop_Widget(Am_Object widget,
                                         Am_Value final_value = Am_No_Value);

#define Am_AT_CENTER_SCREEN -10000

// For all of these routines, the Am_Value_List is a list of char* or
// Am_String objects which will be displayed in the dialog box above any
// text input or buttons in the box.  They create a new dialog box
// and then destroy it at the end of the routine.
_OA_DL_IMPORT extern void Am_Show_Alert_Dialog(Am_Value_List alert_texts,
                                               int x = Am_AT_CENTER_SCREEN,
                                               int y = Am_AT_CENTER_SCREEN,
                                               bool modal = true);
_OA_DL_IMPORT extern void Am_Show_Alert_Dialog(Am_String alert_text,
                                               int x = Am_AT_CENTER_SCREEN,
                                               int y = Am_AT_CENTER_SCREEN,
                                               bool modal = true);

_OA_DL_IMPORT extern Am_Value
Am_Get_Input_From_Dialog(Am_Value_List prompt_texts,
                         Am_String initial_value = "",
                         int x = Am_AT_CENTER_SCREEN,
                         int y = Am_AT_CENTER_SCREEN, bool modal = true);

_OA_DL_IMPORT extern Am_Value
Am_Get_Choice_From_Dialog(Am_Value_List prompt_texts,
                          int x = Am_AT_CENTER_SCREEN,
                          int y = Am_AT_CENTER_SCREEN, bool modal = true);

// these are called automatically from the Am_Open_Command, Am_Save_Command and
// Am_Save_As_Command
_OA_DL_IMPORT extern Am_String
Am_Show_File_Dialog_For_Open(Am_String initial_value = "");
_OA_DL_IMPORT extern Am_String
Am_Show_File_Dialog_For_Save(Am_String initial_value = "");

//This might be called for a file types as the command line argument.
//The command object should be the standard open command object
//attached to the menu_bar (needed to get the default selection and
//the Am_HANDLE_OPEN_SAVE_METHOD).
_OA_DL_IMPORT extern void Am_Standard_Open_From_Filename(Am_Object command_obj,
                                                         Am_String file_name);

//gets the selection widget out of the command object, or if not
//there, then out of the Am_SAVED_OLD_OWNER widget
_OA_DL_IMPORT extern Am_Object
Am_GV_Selection_Widget_For_Command(Am_Object cmd);
_OA_DL_IMPORT extern Am_Object
Am_Get_Selection_Widget_For_Command(Am_Object cmd);

// Sharing menubar accelerators

// new_window will use the same accelerators as source_window
_OA_DL_IMPORT extern void Am_Share_Accelerators(Am_Object &source_window,
                                                Am_Object &new_window);
