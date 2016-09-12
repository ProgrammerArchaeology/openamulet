/* ****************************** -*-c++-*- *******************************/
/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* Standard exported stuff for scripting
   
   Designed and implemented by Brad Myers
*/
#ifndef SCRIPTING_H
#define SCRIPTING_H

#include <am_inc.h>
#include TYPES__H

//lets scripting know about a palette.  If the slot_or_type is a type,
//then the palette is used for all values of that type.  For example,
//use an instance of Am_Style (e.g., Am_Red) to specify the palette is
//for all colors.  If only for a particular slot's values, use an
//Am_Am_Slot_Key wrapper for a slot, e.g.,
//Am_Am_Slot_Key(Am_PROTOTYPE) for the tool panel, or
//Am_Am_Slot_Key(Am_LINE_STYLE) if only for the line style
_OA_SCRIPT_DL_IMPORT extern void
Am_Scripting_Register_Palette(Am_Object undo_handler, Am_Object palette,
                              Am_Value slot_or_type);

//slot of undo handler used for the association list of palettes
_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_REGISTRY_FOR_PALETTES;

_OA_SCRIPT_DL_IMPORT extern void Am_TEMP_Register_Prototype(const char *name,
                                                            Am_Object &proto);
_OA_SCRIPT_DL_IMPORT extern Am_Object Am_Create_Search_Dialog_For(
    /* Am_Load_Save_Context& ls_context */
    Am_Object &main_undo_handler);

_OA_SCRIPT_DL_IMPORT extern Am_Slot_Key Am_SEARCH_DIALOG;

_OA_SCRIPT_DL_IMPORT extern Am_Object Am_Find_Command;
_OA_SCRIPT_DL_IMPORT extern Am_Object Am_Show_Search_Command;

#endif
