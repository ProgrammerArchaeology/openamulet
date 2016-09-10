class Am_Object;
////////////////////////////////////////////////////////////////////////
// The command objects you can make instances of:

_OA_DL_IMPORT extern Am_Object Am_Command;  //base of the Command object hierarchy

//Am_Defines_A_Collection_Command are used for all commands whose
//output is a collection of objects which might be used by subsequent
//operations.  This includes create commands (like create and paste),
//as well as selection commands like Select-All.
_OA_DL_IMPORT extern Am_Object Am_Defines_A_Collection_Command;

//Am_Select_Command is an instance of Am_Defines_A_Collection_Command
//used for commands which cause their to be a selection.  
_OA_DL_IMPORT extern Am_Object Am_Select_Command;

//Am_Create_Command is the prototype for the commands below that
//create new objects.  These new commands are used for operations that
//create new objects, so that the macro facility knows it is a create
//operation.  All slots are the same as Am_Command.
_OA_DL_IMPORT extern Am_Object Am_Create_Command;

// Use this one for commands that create objects "from scratch", like 
// Am_New_Points_Interactor
_OA_DL_IMPORT extern Am_Object Am_Object_Create_Command;

// Use the following command for operations that create new objects
// but not from scratch.  For example, this is used by Duplicate, Paste,
// group, and other operations that cause new objects to appear, but which
// aren't "regular" creates.
_OA_DL_IMPORT extern Am_Object Am_From_Other_Create_Command;

