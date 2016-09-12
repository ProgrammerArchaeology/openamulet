///////////////////////////////////////////////////////////////////////////
// Graphical Editing Commands
///////////////////////////////////////////////////////////////////////////
extern void Am_Editing_Commands_Initialize();

// useful utilities for editing commands
_OA_DL_IMPORT extern Am_Value_List
Am_Copy_Object_List(Am_Value_List orig, const Am_Object &ref_obj = Am_No_Object,
                    int offset = 0);

_OA_DL_IMPORT extern Am_Value_List
Am_Sort_Obs_In_Group(Am_Value_List unsorted_sel_objs, Am_Object group);
_OA_DL_IMPORT extern void Am_Get_Selection_In_Display_Order(
    Am_Object selection_widget, Am_Value_List &selected_objs, Am_Object &group);
_OA_DL_IMPORT extern Am_Object Am_Find_Part_Place(Am_Object obj,
                                                  Am_Object group);
