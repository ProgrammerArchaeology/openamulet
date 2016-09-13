/* ****************************** -*-c++-*- *******************************/
/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

// The OpenAmulet Interface Development Environment
//
// This code was changed & extended as part of the free OpenAmulet project.
// If you are using this code or any part of OpenAmulet, please have a look at
// http://www.openip.org

// Advanced and internal stuff for widgets
// Designed and implemented by Brad Myers

#ifndef WIDGETS_ADVANCED_H
#define WIDGETS_ADVANCED_H

#include <am_inc.h>

#include <amulet/widgets.h>
//#include <amulet/gem.h>
//#include <amulet/types.h>

#include <amulet/web.h>

#include <amulet/value_list.h>

extern void Am_Widgets_Initialize();

//-------------
// Widget Look
//-------------

//-------------------------
// Color Utility Functions
//-------------------------

_OA_DL_IMPORT extern Am_Formula Am_Default_Motif_Fill_Style;

#include "amulet/impl/computed_colors_rec.h"

#include "amulet/impl/style2motif.h"

#include "amulet/impl/widget_draw.h"

#include "amulet/impl/widget_formula.h"

#include "amulet/impl/widget_undo.h"

#include "amulet/impl/widget_misc.h"

extern void Am_Button_Widgets_Initialize();

#include "amulet/impl/widget_scroll.h"

//text input

///////////////////////////////////////////////////////////////////////////
// Scrolling Text Input Widget
///////////////////////////////////////////////////////////////////////////

extern void Am_Text_Widgets_Initialize();
extern Am_Text_Check_Legal_Method Am_Number_Input_Filter_Method;

///////////////////////////////////////////////////////////////////////////
// Selection Handles
///////////////////////////////////////////////////////////////////////////

_OA_DL_IMPORT extern Am_Object Am_One_Selection_Handle;
extern void Am_Selection_Widget_Initialize();

#include "amulet/impl/wigdet_ed_op.h"

#include "amulet/impl/widget_accelerators.h"

#include "amulet/impl/widget_methods.h"

///////////////////////////////////////////////////////////////////////////
// Dialog boxes
///////////////////////////////////////////////////////////////////////////

_OA_DL_IMPORT extern Am_Object am_empty_dialog;
extern void Am_Dialog_Widgets_Initialize();

//defined in Text_widgets.  Useful for inspector, etc.
//Returns true if parse is OK, and value is put into output_value;
extern bool Am_String_To_Long_Or_Float(Am_String string,
                                       Am_Value &output_value);

#endif
