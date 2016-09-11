/* ****************************** -*-c++-*- *******************************/
/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#ifndef TEXT_FNS_H
#define TEXT_FNS_H

#include <am_inc.h>

#include GDEFS__H
#include "amulet/impl/am_object.h"
#include IDEFS__H
#include "amulet/impl/inter_text.h"
#include TYPEDEFS__HPP

// *************************************************
// Functions for editing text and moving the cursor
// *************************************************

_OA_DL_IMPORT extern void Am_Move_Cursor_To_Start (Am_Object text);

_OA_DL_IMPORT extern void Am_Move_Cursor_To_End (Am_Object text);

_OA_DL_IMPORT extern void Am_Move_Cursor_Right (Am_Object text, int n);

_OA_DL_IMPORT extern void Am_Move_Cursor_Word_Right (Am_Object text);

_OA_DL_IMPORT extern void Am_Move_Cursor_Right (Am_Object text);

_OA_DL_IMPORT extern void Am_Move_Cursor_Left (Am_Object text, int n);

_OA_DL_IMPORT extern void Am_Move_Cursor_Word_Left (Am_Object text);

_OA_DL_IMPORT extern void Am_Move_Cursor_Left (Am_Object text);

_OA_DL_IMPORT extern void Am_Move_Cursor_To (Am_Object text, int n);

_OA_DL_IMPORT extern void Am_Set_Text (Am_Object text, char *str);

_OA_DL_IMPORT extern void Am_Insert_Char_At_Cursor (Am_Object text, char c,
				      bool advance_cursor = true);

// Inserts the new text into the Opal Am_Text object at the current cursor
// index.  By default, the cursor is moved to the end of the inserted text.
// If advance_cursor is false, the cursor will remain at the beginning of
// the inserted text.
_OA_DL_IMPORT extern void Am_Insert_String_At_Cursor (Am_Object text, const char *str,
					bool advance_cursor = true);

// Always moves the cursor left if there was a character to delete
_OA_DL_IMPORT extern void Am_Delete_Char_Before_Cursor (Am_Object text);

_OA_DL_IMPORT extern void Am_Delete_Char_After_Cursor (Am_Object text);

_OA_DL_IMPORT extern void Am_Delete_Word_Before_Cursor (Am_Object text);

_OA_DL_IMPORT extern void Am_Delete_Word_After_Cursor (Am_Object text);

_OA_DL_IMPORT extern void Am_Kill_From_Cursor (Am_Object text);

_OA_DL_IMPORT extern void Am_Delete_Entire_String (Am_Object text);

// Given an object of type opal:text and two coordinates x and y, returns
// the index of the character in the string that the point lies on.
// x and y are in the coordinate system of text's window.
_OA_DL_IMPORT extern int Am_Get_Cursor_Index (Am_Object text, int x, int y);

// turn the cursor visibility on or off
_OA_DL_IMPORT extern void Am_Obj_Or_Feedback_Cursor_On_Off (Am_Object obj_over,
					      Am_Object feedback_obj,
					      bool turn_on_p,
					      Am_Object inter);

//turn pending delete on or off
_OA_DL_IMPORT extern void Am_Set_Pending_Delete (Am_Object text, bool pending_delete);

// Sets the cut buffer with the string
_OA_DL_IMPORT extern void Am_Set_Cut_Buffer (Am_Object win, char *str);

// Returns the contents of the cut buffer
_OA_DL_IMPORT extern char* Am_Get_Cut_Buffer (Am_Object win);

// Yanks the contents of the cut buffer into the string at index
_OA_DL_IMPORT extern void Am_Add_Cut_Buffer (Am_Object win, char *str, int index);

// Does kill line from string and adds string to cut buffer.
// Returns the new string.  Index always stays the same.
_OA_DL_IMPORT extern void Am_Cut_To_End_Add_Cut_Buffer (char *str, int index,
					  bool alreadycutting,
					  Am_Object win);

_OA_DL_IMPORT extern void Am_Set_Cut_Buffer_From_Text (Am_Object text);

_OA_DL_IMPORT extern void Am_Insert_Cut_Buffer_At_Cursor (Am_Object text);

// Am_Edit_Translation_Table
// a table mapping Am_Input_Char's to Am_Text_Edit_Operation's.

class Am_Edit_Translation_Table_Data;

class Am_Edit_Translation_Table {
  Am_WRAPPER_DECL (Am_Edit_Translation_Table)
 public:
  Am_Edit_Translation_Table ();

  static Am_Edit_Translation_Table Table ();
  static Am_Edit_Translation_Table Default_Table ();

  Am_Text_Edit_Operation* Translate (Am_Input_Char ic) const;

  void Add (Am_Input_Char ic, Am_Text_Edit_Operation *func);
  void Remove (Am_Input_Char ic);
};

extern Am_Text_Edit_Method Am_Default_Text_Edit_Method;

#endif
