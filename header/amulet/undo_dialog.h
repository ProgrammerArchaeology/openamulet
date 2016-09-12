/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file describes the undo dialog box object
   
   Designed and implemented by Brad Myers
*/
#ifndef UNDO_DIALOG_H
#define UNDO_DIALOG_H

#include <am_inc.h>

#include <amulet/object.h>

_OA_DL_IMPORT extern Am_Object Am_Undo_Dialog_Box;

_OA_DL_IMPORT extern Am_Slot_Key Am_UNDO_HANDLER_TO_DISPLAY;
_OA_DL_IMPORT extern Am_Slot_Key Am_UNDO_LABEL;
_OA_DL_IMPORT extern Am_Slot_Key Am_UNDO_SCROLL_GROUP;
_OA_DL_IMPORT extern Am_Slot_Key Am_UNDO_BUTTON_PANEL;
_OA_DL_IMPORT extern Am_Slot_Key Am_UNDO_MENU_OF_COMMANDS;
_OA_DL_IMPORT extern Am_Slot_Key Am_SCROLLING_GROUP_SLOT;

_OA_DL_IMPORT extern Am_Slot_Key
    Am_UNDO_DIALOG_BOX_SLOT; // put the dialog box into this slot of
                             // the Am_Show_Undo_Dialog_Box_Command

extern void Am_Initialize_Undo_Dialog_Box();

_OA_DL_IMPORT extern Am_Object Am_Show_Undo_Dialog_Box_Command;

#endif
