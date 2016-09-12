///////////////////////////////////////////////////////////////////////////
// Methods for start, stop and abort widget
///////////////////////////////////////////////////////////////////////////

class Am_Inter_Location;

AM_DEFINE_METHOD_TYPE(Am_Explicit_Widget_Run_Method, void,
		      (Am_Object widget, Am_Value new_value))

_OA_DL_IMPORT extern Am_Explicit_Widget_Run_Method Am_Standard_Widget_Start_Method;
_OA_DL_IMPORT extern Am_Object_Method 	     Am_Standard_Widget_Abort_Method;
_OA_DL_IMPORT extern Am_Explicit_Widget_Run_Method Am_Standard_Widget_Stop_Method;

//put into the Am_REGION_WHERE_TEST of a selection handles to select
//all the objects in the region. Default method is Am_Group_Parts_Inside
AM_DEFINE_METHOD_TYPE(Am_In_Region_Method, Am_Value_List,
		      (Am_Object widget, Am_Object group,
		       Am_Inter_Location region))

_OA_DL_IMPORT extern Am_In_Region_Method Am_Group_Parts_Inside;
