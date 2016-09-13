class Am_Object;
class Am_Value_List;

//global list of all modal windows currently in force
_OA_DL_IMPORT extern Am_Value_List Am_Modal_Windows;
extern void Am_Push_Modal_Window(Am_Object &window);
extern void Am_Remove_Modal_Window(Am_Object &window);
