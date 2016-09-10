////////////////////////////////////////////////////////////////////////
/////// Graphical editing operations////////////////
////////////////////////////////////////////////////////////////////////

// For the property command, to get and set the object's properties
Am_Define_Method_Type(Am_Get_Widget_Property_Value_Method, void,
		      (Am_Object command_obj, Am_Value &new_value))
Am_Define_Method_Type(Am_Get_Object_Property_Value_Method, void,
		      (Am_Object command_obj, Am_Object object,
		       Am_Value &old_value))
Am_Define_Method_Type(Am_Set_Object_Property_Value_Method, void,
		      (Am_Object command_obj, Am_Object object,
		       Am_Value new_value))

//put these into the Am_HANDLE_OPEN_SAVE_METHOD slot of the command
Am_Define_Method_Type(Am_Handle_Loaded_Items_Method, void,
		      (Am_Object command_obj, Am_Value_List &contents))
Am_Define_Method_Type(Am_Items_To_Save_Method, Am_Value_List,
		      (Am_Object command_obj))

Am_Define_Method_Type(Am_Drop_Target_Interim_Do_Method, bool,
		      (Am_Object& command_obj, const Am_Value& value))
Am_Define_Method_Type(Am_Drop_Target_Do_Method, void,
		      (Am_Object& command_obj, const Am_Value& new_value))

enum Am_Background_Drop_Result
{
  Am_DROP_NORMALLY, Am_DROP_NOT_ALLOWED, Am_DROP_IN_TARGET
};

Am_Define_Method_Type(Am_Drop_Target_Background_Interim_Do_Method,
		      Am_Background_Drop_Result,
		      (Am_Object& command_obj, const Am_Inter_Location& loc,
		       const Am_Value& value))
Am_Define_Method_Type(Am_Drop_Target_Background_Do_Method, void,
		      (Am_Object& command_obj, const Am_Inter_Location& loc,
		       const Am_Value& new_value))
