//
// ------------------------------------------------------------------------
// 		The OpenAmulet User Interface Development Environment
// ------------------------------------------------------------------------
// This code is based on the Amulet project of Carnegie Mellon University,
// and has been placed in the public domain.  If you are using this code
// or any part of OpenAmulet, please contact amulet@cs.cmu.edu to be put
// on the mailing list or have a look at:
// http://www.openip.org
//

// This file contains the definitions for IMPLEMENTING Interactors.
// Normally, this won't be used by users, unless they are implementing
// a new kind of interactor
//
// Designed and implemented by Brad Myers

#ifndef INTER_ADVANCED_H
#define INTER_ADVANCED_H

#include <am_inc.h>

//#include <amulet/gem.h>
#include "amulet/impl/gem_event_handlers.h"
#include <amulet/inter.h>
#include <amulet/formula.h>
#include <amulet/value_list.h> // for Am_Value_List type

// call this before doing any interactor stuff.  This should be
// called automatically from Am_Initialize();
extern void Am_Initialize_Interactors();

#include "amulet/impl/inter_input.h"

#include "amulet/impl/inter_debug.h"

#include "amulet/impl/inter_formula.h"

//////////////////////////////////////////////////////////////////
// Initialization Routines
//////////////////////////////////////////////////////////////////

extern void Am_Initialize_Top_Command();
extern void Am_Initialize_Move_Grow_Interactor();
extern void Am_Initialize_Choice_Interactor();
extern void Am_Initialize_New_Points_Interactor();
extern void Am_Initialize_Text_Interactor();
extern void Am_Initialize_Animation_Interactor();
extern void Am_Initialize_Animated_Constraints();

extern void Am_Cleanup_Animated_Constraints();

////////////////////////////////////////////////////////////////////////
// Miscellaneous
////////////////////////////////////////////////////////////////////////

#include "amulet/impl/modal_windows.h"

#include "amulet/impl/inter_objects.h"

extern Am_Object am_create_new_object(Am_Object &inter, Am_Object &cmd,
                                      Am_Inter_Location &data,
                                      Am_Object &old_object,
                                      bool override_position);

// used also by inter_text.cpp for the Am_Text_Create_Or_Edit_Interactor
// defined in inter_new_points.cpp
extern void
am_new_points_general_undo_redo(Am_Object command_obj, bool undo,
                                bool selective, bool reload_data,
                                Am_Object new_obj_proto = Am_No_Object,
                                const Am_Value &new_loc = Am_No_Value);

#endif
