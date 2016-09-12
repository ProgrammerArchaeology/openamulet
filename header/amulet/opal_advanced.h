/* ****************************** -*-c++-*- *******************************/
/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#ifndef OPAL_ADVANCED_H
#define OPAL_ADVANCED_H

#include <am_inc.h>

#include <amulet/object_advanced.h> // for Am_Demon_Queue
//#include <amulet/object.h>
#include "amulet/impl/am_object.h"
#include <amulet/gem.h>
#include <amulet/standard_slots.h>

#include "amulet/impl/opal_methods.h"

#include "amulet/impl/am_state_store.h"

#include "amulet/impl/method_item.h"

#include "amulet/impl/am_window_todo.h"

#include "amulet/impl/opal_default_demon.h"

#include "amulet/impl/opal_misc.h"

#include "amulet/impl/timers.h"

// // // // // // // // // // // // // // // // // // // // // //
//  Things needed to have external controlled drawables
// // // // // // // // // // // // // // // // // // // // // //

//the standard event handler functions for opal and interactors
extern Am_Input_Event_Handlers *Am_Global_Opal_Handlers;

#endif
