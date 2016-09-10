#ifndef INTER_METHODS_H
#define INTER_METHODS_H

#include "types_method.h"

class Am_Object;
class Am_Inter_Location;
class Am_Input_Event;
class Am_Input_Char;

////////////////////////////////////////////////////////////////////////

// type of method in the Am_START_WHERE_TEST and Am_RUNNING_WHERE_TEST slots
// x and y are w.r.t. event_window
Am_Define_Method_Type(Am_Where_Method, Am_Object,
		      (Am_Object inter, Am_Object object,
		       Am_Object event_window, int x, int y))

// type of method that can be stored in the Am_START_WHEN, Am_STOP_WHEN, and
// Am_ABORT_WHEN slots.
Am_Define_Method_Type(Am_Event_Method, bool,
		      (Am_Object inter, Am_Object event_window,
		       Am_Input_Char ic))

// type of method in the Am_GRID_METHOD slot (to handle custom gridding)
Am_Define_Method_Type(Am_Custom_Gridding_Method, void,
		      (Am_Object inter, const Am_Object& ref_obj, int x, int y,
		       int& out_x, int & out_y))

// type of method in the Am_CREATE_NEW_OBJECT_METHOD slot of
//     Am_New_Points_Interactor.
// Should return the new object created.
// ** old_object is Valid if this is being called as a result of a
// Repeat undo call, and means that a new object should be created
// like old_object.  
Am_Define_Method_Type(Am_Create_New_Object_Method, Am_Object,
		      (Am_Object inter, Am_Inter_Location location,
		       Am_Object old_object))

// Some methods for the _Where tests.
// x and y are with respect to event_window
_OA_DL_IMPORT extern Am_Where_Method Am_Inter_In;
_OA_DL_IMPORT extern Am_Where_Method Am_Inter_In_Part;
_OA_DL_IMPORT extern Am_Where_Method Am_Inter_In_Object_Or_Part; //default in interactors
_OA_DL_IMPORT extern Am_Where_Method Am_Inter_In_Leaf;
_OA_DL_IMPORT extern Am_Where_Method Am_Inter_In_Text;
_OA_DL_IMPORT extern Am_Where_Method Am_Inter_In_Text_Part;// in text part?
_OA_DL_IMPORT extern Am_Where_Method Am_Inter_In_Text_Object_Or_Part; //default in text inter
_OA_DL_IMPORT extern Am_Where_Method Am_Inter_Not_In_Text_Object_Or_Part; //for text_create
_OA_DL_IMPORT extern Am_Where_Method Am_Inter_In_Text_Leaf;
_OA_DL_IMPORT extern Am_Where_Method Am_Inter_Not_In_Text_Leaf; //for text_create

_OA_DL_IMPORT extern Am_Where_Method Am_In_Target;
// default for the Am_SELECT_CLOSEST_POINT_METHOD of Selection Widgets
_OA_DL_IMPORT extern Am_Where_Method Am_Default_Closest_Select_Point;

///////////////////////////////////////////////////////////////////////////

//method types that can go into the inter DO action slots:
// Am_Object_Method (just passed the command or inter object)
// Am_Mouse_Event_Method: passed command or inter object and mouse point
// Am_Current_Location_Method: passed cmd/inter obj, obj modified, and
// 				points for obj
// These types can also go into the command objects directly put into
// an interactor, but the command objects put into widgets must be of 
// Am_Object_Method type only

//this one can be used with any interactor, but not with high-level commands
Am_Define_Method_Type(Am_Mouse_Event_Method, void,
		      (Am_Object inter_or_cmd, int mouse_x, int mouse_y,
		       Am_Object ref_obj, Am_Input_Char ic))

//this one can only be used with move_grow or new_points interactors
Am_Define_Method_Type(Am_Current_Location_Method, void,
		      (Am_Object inter_or_cmd, Am_Object obj_modified,
		       Am_Inter_Location points))


// type of the function in the various internal _METHOD slots of
// interactors (not commands)
Am_Define_Method_Type(Am_Inter_Internal_Method, void,
		      (Am_Object& inter, Am_Object& object,
		       Am_Object& event_window, Am_Input_Event *ev))

#endif // INTER_METHODS_H
