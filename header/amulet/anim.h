/* ****************************** -*-c++-*- *******************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file contains the definitions for using the Interactors input
   handling and the command objects for when events happen

   Designed and implemented by Brad Myers
*/

#ifndef ANIM_H
#define ANIM_H

#include <am_inc.h>
#include "amulet/impl/types_enum.h"
#include "amulet/impl/types_method.h"

#include "amulet/impl/am_object.h" // basic object definitions
#include IDEFS__H                  // Am_Input_Char
#include FORMULA__H

//lint -e752 ok that local declarators are not referenced

////////////////////////////////////////////////////////////////////////
// Animation stuff
////////////////////////////////////////////////////////////////////////

// extern Am_Object Am_Animation_Interactor;  ### defined above

//This enum is used internally; users should use the Am_Animation_End_Action
// values instead.
enum Am_Animation_End_Action_vals
{
  Am_ANIMATION_STOP_val,
  Am_ANIMATION_WRAP_val,
  Am_ANIMATION_BOUNCE_val
};

// Control for what to do at the end of an animation cycle
AM_DEFINE_ENUM_TYPE(Am_Animation_End_Action, Am_Animation_End_Action_vals)
const Am_Animation_End_Action Am_ANIMATION_STOP(Am_ANIMATION_STOP_val);
const Am_Animation_End_Action Am_ANIMATION_WRAP(Am_ANIMATION_WRAP_val);
const Am_Animation_End_Action Am_ANIMATION_BOUNCE(Am_ANIMATION_BOUNCE_val);

//use Am_Animation_Command when want Do method called each time
_OA_DL_IMPORT extern Am_Object Am_Animation_Command;

//use Am_Incrementer_Animation_Command when want slot or slots
//incremented each time
_OA_DL_IMPORT extern Am_Object Am_Incrementer_Animation_Command;

//use Am_Animation_Object_In_Owner_Command when want object to
//continuously move (wrapping or bouncing) inside its owner)
_OA_DL_IMPORT extern Am_Object Am_Animation_Object_In_Owner_Command;

//use Am_Animator_Animation_Command when want slot or slots to be
//interpolated between two values over a period of time
_OA_DL_IMPORT extern Am_Object Am_Animator_Animation_Command;

// use Am_Constant_Velocity_Command to interpolate a slot's value between
// two values at a constant velocity.
_OA_DL_IMPORT extern Am_Object Am_Constant_Velocity_Command;

//use Am_Animation_Through_List_Command when want a slot to take
//values successively from a list of possible values at each time increment
_OA_DL_IMPORT extern Am_Object Am_Animation_Through_List_Command;

//use Am_Animation_Blink_Command when want a slot to take
//values alternatively from exactly two value at each time increment
_OA_DL_IMPORT extern Am_Object Am_Animation_Blink_Command;

////////////////////////////////////////////////////////////////////////
// Animated constraints
////////////////////////////////////////////////////////////////////////

#include GDEFS__H

//
// from anim.cc
//

// Update method
AM_DEFINE_METHOD_TYPE(Am_Anim_Update_Method, void,
                      (Am_Object interpolator, const Am_Value &value))

#include "amulet/impl/anim_misc.h"

// Global functions
_OA_DL_IMPORT Am_Constraint *Am_Animate_With(const Am_Object &animator);
_OA_DL_IMPORT Am_Object Am_Get_Animator(Am_Object obj, Am_Slot_Key key);
_OA_DL_IMPORT Am_Object Am_Get_Animator(Am_Constraint *constraint);
_OA_DL_IMPORT void Am_Set_Animated_Slots(Am_Object interp);

// from inter_polators.cc

_OA_DL_IMPORT void Am_Start_Animator(Am_Object interp,
                                     const Am_Value &value1 = Am_No_Value,
                                     const Am_Value &value2 = Am_No_Value);
//void Am_Start_Animator(Am_Object interp, const Am_Value value1 = Am_No_Value, const Am_Value value2 = Am_No_Value);
_OA_DL_IMPORT void Am_Interrupt_Animator(const Am_Object &interp,
                                         const Am_Value &new_value);
//void Am_Interrupt_Animator(const Am_Object interp, const Am_Value new_value);
_OA_DL_IMPORT void Am_Abort_Animator(Am_Object interp);
_OA_DL_IMPORT void Am_Stop_Animator(Am_Object interp);

_OA_DL_IMPORT extern Am_Object Am_Animator;
_OA_DL_IMPORT extern Am_Object Am_Stepping_Animator;
_OA_DL_IMPORT extern Am_Object Am_Style_Animator;
_OA_DL_IMPORT extern Am_Object Am_Exaggerated_Animator;

_OA_DL_IMPORT extern Am_Object Am_Visible_Animator;
_OA_DL_IMPORT extern Am_Object Am_Blink_Animator;
_OA_DL_IMPORT extern Am_Object Am_Through_List_Animator;
_OA_DL_IMPORT extern Am_Object Am_Object_In_Owner_Animator;
_OA_DL_IMPORT extern Am_Object Am_Point_List_Animator;
_OA_DL_IMPORT extern Am_Object Am_Fly_Apart_Animator;

// timing functions
AM_DEFINE_METHOD_TYPE(Am_Timing_Function, float,
                      (Am_Object command_obj, Am_Time t))

_OA_DL_IMPORT extern Am_Timing_Function Am_Linear_Timing;
_OA_DL_IMPORT extern Am_Timing_Function Am_Delayed_Timing;
_OA_DL_IMPORT extern Am_Timing_Function Am_Slow_In_Slow_Out;

// path functions
AM_DEFINE_METHOD_TYPE(Am_Path_Function, Am_Value,
                      (Am_Object command_obj, Am_Value value1, Am_Value value2,
                       float tau))

// Am_COMPUTE_DISTANCE: when path function is called with this special value,
// it should return the absolute value of the distance between VALUE_1 and
// VALUE_2 (in arbitrary units).  Used for the velocity computation.
const float Am_COMPUTE_DISTANCE = -1.0F;

_OA_DL_IMPORT extern Am_Path_Function Am_Linear_Path;
_OA_DL_IMPORT extern Am_Path_Function Am_Boolean_Path;
_OA_DL_IMPORT extern Am_Path_Function Am_Style_Path;
_OA_DL_IMPORT extern Am_Path_Function Am_Step_Path;
_OA_DL_IMPORT extern Am_Path_Function Am_Through_List_Path;

// command objects
_OA_DL_IMPORT extern Am_Object Am_Animation_Wrap_Command;
_OA_DL_IMPORT extern Am_Object Am_Animation_Bounce_Command;

#endif // ANIM_H
