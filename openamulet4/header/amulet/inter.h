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

#ifndef INTER_H
#define INTER_H

#include <am_inc.h>

#include "amulet/impl/types_method.h"  // basic object definitions
#include "amulet/impl/types_enum.h"  // basic object definitions
#include "amulet/impl/am_object.h"  // basic object definitions
#include IDEFS__H   // Am_Input_Char
#include FORMULA__H

//Global variable controlling the number of pixels that the mouse has
//to move before it is considered a DRAG event (like "LEFT_DRAG").
//Default value = 3 pixels.
//Am_Minimum_Move_For_Drag is the maximum the mouse can move
//between the down and the up and still be classified as a "CLICK",
//like "LEFT_CLICK".
_OA_DL_IMPORT extern int Am_Minimum_Move_For_Drag;


#include "amulet/impl/am_inter_location.h"


#include "amulet/impl/inter_objects.h"

#include "amulet/impl/inter_methods.h"

#include "amulet/impl/inter_debug.h"

#include "amulet/impl/inter_text.h"


#include "amulet/impl/inter_misc.h"

#include "amulet/impl/inter_op.h"

#include "amulet/impl/inter_enum.h"

#include "amulet/impl/inter_commands.h"

#include "amulet/impl/inter_undo.h"

#include "amulet/impl/inter_popup.h"

#include "amulet/impl/inter_user_id.h"


#endif
