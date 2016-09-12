class Am_Input_Char;

_OA_DL_IMPORT extern Am_Input_Char Am_Default_Start_Char;
_OA_DL_IMPORT extern Am_Input_Char Am_Default_Stop_Char;

//value for the implementation parent for commands in a script
#define Am_IN_A_SCRIPT -10

// Amount priority of interactors added when running
#define Am_INTER_PRIORITY_DIFF 100.0f

// Setting Am_USER_ID to this value in an interactor allows multiple
// users to change the object
// see notes in inter_basics.cc
#define Am_EVERYONE_AT_SAME_TIME -1 /* supports multiple users */
#define Am_ONE_AT_A_TIME -2         /* supports multiple users */
