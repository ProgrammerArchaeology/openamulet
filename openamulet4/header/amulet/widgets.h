/* ****************************** -*-c++-*- *******************************/
/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file contains the definitions for widgets in the various
   looks-and-feels
*/

#ifndef WIDGETS_H
#define WIDGETS_H

#include <am_inc.h>

#include OBJECT__H  //basic object definitions
#include OPAL__H
#include INTER__H  // command objects, etc.
#include VALUE_LIST__H
#include RICH_TEXT__H
#include UNDO_DIALOG__H
#include STR_STREAM__H

#include "amulet/impl/widget_look.h"

#include "amulet/impl/widget_objects.h"

#include "amulet/impl/widget_misc.h"


#include "amulet/impl/widget_op.h"

#include "amulet/impl/widget_ed_methods.h"

//put into Am_ACTIVE slot a widget or command that has a
//Am_SELECTION_WIDGET slot holding the widget.  This is used by many
//of the above editing commands.
_OA_DL_IMPORT extern Am_Formula Am_Active_If_Selection;


////////////////////////////////////////////////////////////////////////
/// Dialog box routines
////////////////////////////////////////////////////////////////////////

//Put this method in the Am_DESTROY_WINDOW_METHOD of pop-up windows so
//they will exit nicely (as if the user hit CANCEL)

_OA_DL_IMPORT extern Am_Object_Method Am_Default_Pop_Up_Window_Destroy_Method;

// dialog_widgets.cc
_OA_DL_IMPORT extern Am_Object Am_Text_Input_Dialog;
_OA_DL_IMPORT extern Am_Object Am_Choice_Dialog;
_OA_DL_IMPORT extern Am_Object Am_Alert_Dialog;

_OA_DL_IMPORT extern Am_Object Am_OK_Button; //put this in a dialog box for OK
_OA_DL_IMPORT extern Am_Object Am_OK_Cancel_Buttons;//put this in a dialog box for OK/Cancel


//  Given a user-made dialog box, this sets its stop method, brings it
//  up, and returns its value using Am_Pop_Up_Window_And_Wait.
//  This is useful for efficiency if a certain dialog box is used many
//  times and you don't need to destroy it after use.
//Returns the value of Any input widgets or true if hit OK, and
//returns false if the user hits cancel

_OA_DL_IMPORT extern Am_Value Am_Show_Dialog_And_Wait (Am_Object the_dialog,
					 bool modal = true);

_OA_DL_IMPORT extern Am_Object Am_File_Dialog;




#endif
