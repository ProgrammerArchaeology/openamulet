// The actual interactor objects

_OA_DL_IMPORT extern Am_Object Am_Interactor;  //base of the Interactor hierarchy

// the next ones are the ones you usually would use

_OA_DL_IMPORT extern Am_Object Am_Choice_Interactor;  // choosing one or more from a set
_OA_DL_IMPORT extern Am_Object Am_One_Shot_Interactor;  // goes immediately at input event
_OA_DL_IMPORT extern Am_Object Am_Move_Grow_Interactor;  // moving and growing with mouse
_OA_DL_IMPORT extern Am_Object Am_New_Points_Interactor;  // entering new points with mouse
_OA_DL_IMPORT extern Am_Object Am_Text_Edit_Interactor;  // text editing
_OA_DL_IMPORT extern Am_Object Am_Text_Create_Interactor;  // text creating
_OA_DL_IMPORT extern Am_Object Am_Rotate_Interactor;  // rotating
_OA_DL_IMPORT extern Am_Object Am_Gesture_Interactor;  // handling gestures
_OA_DL_IMPORT extern Am_Object Am_Animation_Interactor;  // handling animations

//like choice interactor, but when repeat on new, actually does repeat
//on same.  This is used by button widgets.
_OA_DL_IMPORT extern Am_Object Am_Choice_Interactor_Repeat_Same;

_OA_DL_IMPORT extern Am_Object Am_Choice_Internal_Command;
_OA_DL_IMPORT extern Am_Object Am_Move_Grow_Internal_Command;
_OA_DL_IMPORT extern Am_Object Am_New_Points_Internal_Command;
_OA_DL_IMPORT extern Am_Object Am_Edit_Text_Internal_Command;
