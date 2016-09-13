/* ****************************** -*-c++-*- *******************************/
/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact garnet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file implements the input handling for Gem that is independent of the
   window manager.

   Designed and implemented by Brad Myers
*/

#include <cstdio>
#include <cstring>

#include <iostream>
#include <am_inc.h>
#include <amulet/am_value.hpp>
#include <amulet/idefs.h>
#include <amulet/impl/types_type_support.h>
#include <amulet/impl/gem_event_handlers.h>
#include <amulet/impl/am_am_drawonable.h>
#include <amulet/impl/inter_user_id.h>
#include <amulet/impl/slots.h>
#include <amulet/impl/am_object.h>

// for windows-specific keyboard initialization code.
// This isn't the best place for this, but it'll do for now.
#ifdef _WIN32
#include "amulet/gemW_input.h"
#else
// We need this anyway no ? It is defined in gem.h
// But we must be careful with redefinitions... -- ortalo
extern void Am_Init_Key_Map();
#endif

#include <amulet/symbol_table.h>

// Function Prototypes
char *alloc_string_for(char c);
void set_key_map(int code, const char *name);
int put_string(char *dest, const char *news, int where);
void string_copy(char *copy_s, const char *s, int len);
bool case_compare(char c1, char c2);
bool find_str_and_remove(char *s, const char *target);
void upcase_str(char *str);
int Find_Char(char *s, char target);
bool find_prefix_and_remove(char *prefix, char *s, char target_char);
bool Fix_Shift_For_Code(short code, bool old_shift);

// // // // // // // // // // // // // // // // // // // //
// GLOBAL VARIABLES
// // // // // // // // // // // // // // // // // // // //

// Global variable holding the input event currently being processed.  Defined
// in gem.h.
Am_Input_Event *Am_Current_Input_Event;
// this Am_Stop_Character should replace the one in gemM_input.cc
// and gemX_input.cc: it's machine independant.
const Am_Input_Char Am_Stop_Character(Am_STOP_CHARACTER);

Am_Symbol_Table *char_map = 0;
Am_Symbol_Table *click_count_map = 0;

char *
alloc_string_for(char c)
{
  char *s = new char[2];
  s[0] = c;
  s[1] = 0;
  return s;
}

// sets up the string->code and code->string mappings.
// If multiple strings map to the same code, the LAST one registered
// is used for the code->string mapping
void
set_key_map(int code, const char *name)
{
  char_map->Add_Item(code, name);
}

// Whether to print input events or not.
int Am_Debug_Print_Input_Events = 0;

void
Am_Initialize_Char_Map()
{
  if (char_map != 0 && click_count_map != 0)
    return;

  // allocate hash tables
  char_map = Am_Symbol_Table::Create(190);
  click_count_map = Am_Symbol_Table::Create(8);

  // allocate current event record
  Am_Current_Input_Event = new Am_Input_Event;

  // printable characters
  for (char c = ' '; c <= '~'; ++c)
    set_key_map(c, alloc_string_for(c));

  // special characters
  set_key_map(Am_LEFT_MOUSE, "LEFT");
  set_key_map(Am_MIDDLE_MOUSE, "MIDDLE");
  set_key_map(Am_RIGHT_MOUSE, "RIGHT");
  set_key_map(Am_MOUSE_MOVED, "MOUSE_MOVED");
  set_key_map(Am_MOUSE_ENTER_WINDOW, "MOUSE_ENTER_WINDOW");
  set_key_map(Am_MOUSE_LEAVE_WINDOW, "MOUSE_LEAVE_WINDOW");
  set_key_map(Am_ANY_MOUSE, "MOUSE"); // used for "ANY_MOUSE_DOWN"

  //standard characters
  set_key_map(Am_RUBOUT, "RUBOUT");
  set_key_map(Am_DELETE, "DELETE");
  set_key_map(Am_ESC, "ESC");
  set_key_map(Am_BACKSPACE, "BACKSPACE");
  set_key_map(Am_RETURN, "RETURN");
  set_key_map(Am_NEWLINE, "NEWLINE");
  set_key_map(Am_SPACE, "SPACE");
  set_key_map(Am_LINEFEED, "LINEFEED");
  set_key_map(Am_TAB, "TAB");

  //function keys
  set_key_map(Am_F1, "F1");
  set_key_map(Am_F2, "F2");
  set_key_map(Am_F3, "F3");
  set_key_map(Am_F4, "F4");
  set_key_map(Am_F5, "F5");
  set_key_map(Am_F6, "F6");
  set_key_map(Am_F7, "F7");
  set_key_map(Am_F8, "F8");
  set_key_map(Am_F9, "F9");
  set_key_map(Am_F10, "F10");
  set_key_map(Am_F11, "F11");
  set_key_map(Am_F12, "F12");
  set_key_map(Am_F13, "F13");
  set_key_map(Am_F14, "F14");
  set_key_map(Am_F15, "F15");
  set_key_map(Am_F16, "F16");
  set_key_map(Am_F17, "F17");
  set_key_map(Am_F18, "F18");
  set_key_map(Am_F19, "F19");
  set_key_map(Am_F20, "F20");

  set_key_map(Am_L1, "L1");
  set_key_map(Am_L2, "L2");
  set_key_map(Am_L3, "L3");
  set_key_map(Am_L4, "L4");
  set_key_map(Am_L5, "L5");
  set_key_map(Am_L6, "L6");
  set_key_map(Am_L7, "L7");
  set_key_map(Am_L8, "L8");
  set_key_map(Am_L9, "L9");
  set_key_map(Am_L10, "L10");

  set_key_map(Am_R1, "R1");
  set_key_map(Am_R2, "R2");
  set_key_map(Am_R3, "R3");
  set_key_map(Am_R4, "R4");
  set_key_map(Am_R5, "R5");
  set_key_map(Am_R6, "R6");
  set_key_map(Am_R7, "R7");
  set_key_map(Am_R8, "R8");
  set_key_map(Am_R9, "R9");
  set_key_map(Am_R10, "R10");
  set_key_map(Am_R11, "R11");
  set_key_map(Am_R12, "R12");
  set_key_map(Am_R13, "R13");
  set_key_map(Am_R14, "R14");
  set_key_map(Am_R15, "R15");

  set_key_map(Am_LEFT_ARROW, "LEFT_ARROW");
  set_key_map(Am_UP_ARROW, "UP_ARROW");
  set_key_map(Am_DOWN_ARROW, "DOWN_ARROW");
  set_key_map(Am_RIGHT_ARROW, "RIGHT_ARROW");

  //various SUN keyboards
  set_key_map(Am_PRIOR, "PRIOR");
  set_key_map(Am_NEXT, "NEXT");
  set_key_map(Am_SELECT, "SELECT");
  set_key_map(Am_MENU, "MENU");
  set_key_map(Am_HELP, "HELP");
  set_key_map(Am_ALT_GRAPH, "ALT_GRAPH");
  set_key_map(Am_ENTER, "ENTER");
  set_key_map(Am_NUMLOCK, "NUMLOCK");
  set_key_map(Am_SCROLL_LOCK, "SCROLL_LOCK");

  //vax and dec3100 and dec5000 keyboards
  set_key_map(Am_FIND, "FIND");
  set_key_map(Am_INSERT, "INSERT");
  set_key_map(Am_REMOVE, "REMOVE");
  set_key_map(Am_COMPOSE_CHARACTER, "COMPOSE_CHARACTER");
  set_key_map(Am_INSERT_HERE, "INSERT_HERE");
  set_key_map(Am_DO_KEY, "DO");

  //hp-ux and hp9000-300 keyboards
  set_key_map(Am_HOME, "HOME");
  set_key_map(Am_BREAK, "BREAK");
  set_key_map(Am_CANCEL, "CANCEL");
  set_key_map(Am_CLEAR, "CLEAR");
  set_key_map(Am_EXECUTE, "EXECUTE");
  set_key_map(Am_INSERT_CHAR, "INSERT_CHAR");
  set_key_map(Am_INSERT_LINE, "INSERT_LINE");
  set_key_map(Am_DELETE_LINE, "DELETE_LINE");
  set_key_map(Am_DELETE_CHAR, "DELETE_CHAR");
  set_key_map(Am_CLEAR_LINE, "CLEAR_LINE");
  set_key_map(Am_USER, "USER");

  set_key_map(Am_PF1, "PF1");
  set_key_map(Am_PF2, "PF2");
  set_key_map(Am_PF3, "PF3");
  set_key_map(Am_PF4, "PF4");

  // Mac keyboard
  set_key_map(Am_PAGE, "PAGE");
  set_key_map(Am_PAGE_UP, "PAGE_UP");
  set_key_map(Am_PAGE_DOWN, "PAGE_DOWN");
  set_key_map(Am_END, "END");
  set_key_map(Am_UNDO_KEY, "UNDO");
  set_key_map(Am_REDO_KEY, "REDO");
  set_key_map(Am_PRINT_KEY, "PRINT");
  set_key_map(Am_CANCEL, "CANCEL");

  set_key_map(Am_CUT_KEY, "CUT");
  set_key_map(Am_OPEN_KEY, "OPEN");
  set_key_map(Am_PASTE_KEY, "PASTE");
  set_key_map(Am_FRONT_KEY, "FRONT");
  set_key_map(Am_COPY_KEY, "COPY");
  set_key_map(Am_PROPS_KEY, "PROPS");

  set_key_map(Am_ANY_KEYBOARD, "KEYBOARD"); // used for ANY_KEYBOARD

  click_count_map->Add_Item(Am_SINGLE_CLICK, "SINGLE_");
  click_count_map->Add_Item(Am_DOUBLE_CLICK, "DOUBLE_");
  click_count_map->Add_Item(Am_TRIPLE_CLICK, "TRIPLE_");
  click_count_map->Add_Item(Am_QUAD_CLICK, "QUAD_");
  click_count_map->Add_Item(Am_FIVE_CLICK, "FIVE_");
  click_count_map->Add_Item(Am_MANY_CLICK, "MANYCLICK_");
  click_count_map->Add_Item(Am_ANY_CLICK, "ANY_");

  Am_Init_Key_Map(); // initialize keysym mapping
}

// // // // // // // // // // // // // // // // // // // //
// Am_Input_Char   (defined in idefs.h)
// // // // // // // // // // // // // // // // // // // //

Am_Input_Char Am_No_Input_Char(Am_Value(0, Am_Input_Char::Type_ID()));

class Input_Char_Support : public Am_Type_Support
{
public:
  void Print(std::ostream &os, const Am_Value &value) const
  {
    Am_Input_Char ch = value;
    os << ch;
  }
  // Reads a string, potentially provided by a user and converts to its own
  // type.  Returns Am_No_Value when there is an error.
  Am_Value From_String(const char *string) const
  {
    Am_Input_Char ch(string, false);
    if (ch.code != 0)
      return ch;
    else
      return Am_No_Value;
  }
} Input_Char_Object;

Am_ID_Tag Am_Input_Char::Am_Input_Char_ID =
    Am_Get_Unique_ID_Tag(DSTR("Am_Input_Char"), &Input_Char_Object, Am_ENUM);

// Returns a char if a simple char type, otherwise returns 0
unsigned char
Am_Input_Char::As_Char() const
{
  if (!any_modifier && click_count == Am_NOT_MOUSE && code <= 0xFF) {
    unsigned char ret_val = static_cast<unsigned char>(code);
    if (control)
      ret_val &= 31;
    if (meta)
      ret_val |= 0x80; //or in high bit
    return ret_val;
  } else
    return 0;
}

bool
Am_Input_Char::helper_check_equal(Am_Input_Char i) const
{
  if (!(any_modifier || i.any_modifier ||
        (shift == i.shift && control == i.control && meta == i.meta)))
    return false; // modifiers don't match

  // now check code
  if (code == Am_ANY_KEYBOARD || i.code == Am_ANY_KEYBOARD) {
    // any_keyboard match
    if (i.click_count == Am_NOT_MOUSE && click_count == Am_NOT_MOUSE)
      return true;
    else
      return false;
  } else if (click_count == Am_NOT_MOUSE || i.click_count == Am_NOT_MOUSE) {
    // specific keyboard key
    if (code == i.code)
      return true;
    else
      return false;
    // note: ignores button up or down for keyboard keys since
    // only support down.
  } else { // here must be a mouse button
    // first, check click count
    if (!(click_count == Am_ANY_CLICK || i.click_count == Am_ANY_CLICK ||
          click_count == i.click_count))
      return false;
    // now button down, up
    if (!(button_down == Am_ANY_DOWN_UP || i.button_down == Am_ANY_DOWN_UP ||
          button_down == i.button_down))
      return false;
    //finally, check code
    if (code == i.code || code == Am_ANY_MOUSE || i.code == Am_ANY_MOUSE)
      return true;
    else
      return false;
  } // end else
}

//puts string news into dest starting at index `where'
// returns index of the `0' at the end of the string
int
put_string(char *dest, const char *news, int where)
{
  int len = strlen(news);
  int i, cnt;
  for (i = where, cnt = 0; cnt < len; i++, cnt++)
    dest[i] = news[cnt];
  dest[where + len] = 0; //string terminator
  return where + len;    // index of the 0
}

// copy s into copy_s with length at most len
void
string_copy(char *copy_s, const char *s, int len)
{
  int i;
  for (i = 0; i < (len - 1) && s[i] != 0; i++) {
    copy_s[i] = s[i];
  }
  copy_s[i] = 0; // terminator for string
}

bool
case_compare(char c1, char c2)
{
  if (c1 == c2)
    return true;
  else {
    if (c1 >= 'a' && c1 <= 'z')
      c1 = c1 + ('A' - 'a');
    if (c2 >= 'a' && c2 <= 'z')
      c2 = c2 + ('A' - 'a');
    if (c1 == c2)
      return true;
    else
      return false;
  }
}

// searches for target in s.  If found (case insensitive), returns true and
// modifies s so it no longer contains target.  If not found, returns false
bool
find_str_and_remove(char *s, const char *target)
{
  // uses a dumb algorithm
  int start, i, j;
  int len = strlen(target);
  int tot_len = strlen(s);
  bool matches = false;
  for (i = 0; i < tot_len; i++) {
    start = i;
    for (j = 0; j < len; j++) {
      if (case_compare(s[i + j], target[j]))
        matches = true;
      else {
        matches = false;
        break;
      }
    }
    if (matches) {
      for (i = start, j = start + len; j <= tot_len; i++, j++)
        // use <= on j so also copies the zero at the end
        s[i] = s[j];
      return true;
    }
  }
  // if get to here, then no match
  return false;
}

void
upcase_str(char *str)
{
  char c;
  for (int i = 0; str[i] != 0; i++) {
    c = str[i];
    if (c >= 'a' && c <= 'z')
      str[i] = c + ('A' - 'a');
  }
}

// Find char target in s, return index or 0 if not there
int
Find_Char(char *s, char target)
{
  for (int i = 0; s[i] != 0; i++) {
    if (s[i] == target)
      return i;
  }
  return 0;
}

// finds the string ending with target_char at the front of s.
// if found, then sets prefix with that string converted to upper case
// (including the target_char) and
// removes if from s and returns true.  If not found, returns false.
// target_char should not be 0 (the string terminator)
bool
find_prefix_and_remove(char *prefix, char *s, char target_char)
{
  int i, j, k;
  int pos = -1;
  int tot_len = strlen(s);
  for (i = 0; i < tot_len; i++) {
    if (s[i] == target_char) {
      pos = i + 1; // character after target_char
      for (j = 0; j < pos; j++)
        prefix[j] = s[j];
      prefix[pos] = 0; //terminator
      upcase_str(prefix);
      for (j = pos, k = 0; j <= tot_len; j++, k++)
        // use j <= so includes terminator
        s[k] = s[j];
      return true;
    }
  }
  return false;
}

bool
Fix_Shift_For_Code(short code, bool old_shift)
{
  // if uppercase letter, then shift should be true
  if ((code >= 'A') && (code <= 'Z'))
    return true;
  // if a special character, don't allow shift
  else if (((code > ' ') && (code < '0')) || ((code > '9') && (code < 'A')) ||
           ((code > 'Z') && (code < 'a')) || ((code > 'z') && (code <= '~')))
    return false;
  else
    return old_shift;
}

// Constructor from parts.  Make sure the SHIFT is consistent with the code.
// Under WinNT Purify complains that 4 bytes of uninitialized memory is read
// I think this may be an artifact -- the offending assignment moves when
// I rearrange the code, and I see no problem when running under the debugger
// eab 5/7/97

Am_Input_Char::Am_Input_Char(short c, bool shf, bool ctrl, bool mt,
                             Am_Button_Down down, Am_Click_Count click,
                             bool any_mod)
{
  if (char_map == 0 || click_count_map == 0)
    Am_Initialize_Char_Map();

  code = c;
  shift = shf;
  control = ctrl;
  meta = mt;
  button_down = down;
  click_count = click;
  any_modifier = any_mod;

  // make sure the shift bit is consistent with the character
  if ((code >= 'a') && (code <= 'z')) {

    if (shift) //convert to upper case
      code = code + ('A' - 'a');
  } else
    shift = Fix_Shift_For_Code(code, shift);
}

// Constructor from a string
Am_Input_Char::Am_Input_Char(const char *s, bool want_error)
{
  if (char_map == 0 || click_count_map == 0)
    Am_Initialize_Char_Map();

  bool found_any = false;
  any_modifier = false; //field of the object

  char copy_s[Am_LONGEST_CHAR_STRING];
  string_copy(copy_s, s, Am_LONGEST_CHAR_STRING - 1);

  if (find_str_and_remove(copy_s, Am_SHIFT_STRING) ||
      find_str_and_remove(copy_s, Am_SHORT_SHIFT_STRING))
    shift = true;
  else
    shift = false;

  if (find_str_and_remove(copy_s, Am_CONTROL_STRING) ||
      find_str_and_remove(copy_s, Am_SHORT_CONTROL_STRING))
    control = true;
  else
    control = false;

  if (find_str_and_remove(copy_s, Am_META_STRING) ||
      find_str_and_remove(copy_s, Am_SHORT_META_STRING))
    meta = true;
  else
    meta = false;

  if (find_str_and_remove(copy_s, Am_ANY_STRING)) {
    if (shift || control || meta)
      // found a modifier, maybe ANY is for click count, save for later
      found_any = true;
    else
      any_modifier = true;
  }

  click_count = Am_NOT_MOUSE;
  button_down = Am_NEITHER;

  //first try whole string as a key, especially for "PAGE_DOWN"
  // ** NOTE: There is a PAGE code as well as a PAGE_DOWN.  If we
  // ever support up and down for keyboard keys, we will not be able
  // to distinguish between the PAGE key going down, and the
  // PAGE_DOWN key.
  short temp_code = char_map->Get_Value(copy_s);
  if (temp_code < 0) { // then not found
    // try the upper case version
    upcase_str(copy_s);
    temp_code = char_map->Get_Value(copy_s);
    if (temp_code < 0) {
      //then whole string isn't a match, try taking it apart
      if (find_str_and_remove(copy_s, Am_DOWN_STRING)) {
        click_count = Am_SINGLE_CLICK;
        button_down = Am_BUTTON_DOWN;
      }
      if (find_str_and_remove(copy_s, Am_UP_STRING)) {
        click_count = Am_SINGLE_CLICK;
        button_down = Am_BUTTON_UP;
      }
      if (find_str_and_remove(copy_s, Am_CLICK_STRING)) {
        click_count = Am_SINGLE_CLICK;
        button_down = Am_BUTTON_CLICK;
      }
      if (find_str_and_remove(copy_s, Am_DRAG_STRING)) {
        click_count = Am_SINGLE_CLICK;
        button_down = Am_BUTTON_DRAG;
      }

      if ((Am_Click_Count)click_count !=
          Am_NOT_MOUSE) { // then is mouse, check for
                          // double or more click
        int prefix_code = 0;
        char prefix[Am_LONGEST_CHAR_STRING];
        if (find_prefix_and_remove(prefix, copy_s, '_')) {
          prefix_code = click_count_map->Get_Value(prefix);
          if (prefix_code > 0) // then found
            click_count = (Am_Click_Count)prefix_code;
        }
        if (prefix_code == 0 && (any_modifier || found_any))
          // no explicit click specified, and have an "ANY_"
          click_count = Am_ANY_CLICK;
      }

      temp_code = char_map->Get_Value(copy_s);
      if (temp_code < 0) { // then not found
        // try the upper case version
        upcase_str(copy_s);
        temp_code = char_map->Get_Value(copy_s);
        if (temp_code < 0) { // then still not found
          if (want_error) {
            std::cout << "** Amulet Error: `" << s
                      << "' could not be parsed as an Am_Input_Char because "
                         "couldn't find `"
                      << copy_s << "'\n";
            if (Find_Char(copy_s, '-'))
              std::cout << "   Maybe because you used a hyphen `-' instead of "
                           "underscore `_'?\n";
            Am_Error();
          } else { //error, but don't want to raise the error
            code = 0;
            return;
          }
        }
      }
    }
  }
  // make sure the SHIFT bit is consistent with the character if alphabetic
  if ((temp_code >= 'a') && (temp_code <= 'z')) {
    if (shift) //convert to upper case
      temp_code = temp_code + ('A' - 'a');
  } else
    shift = Fix_Shift_For_Code(temp_code, shift);

  code = temp_code;
}

//get as string
// no automatic converter to a string since would need to allocate a
// string.  s should be at least Am_LONGEST_CHAR_STRING characters long.
void
Am_Input_Char::As_String(char *s) const
{
  int index = 0;
  if (shift)
    index = put_string(s, Am_SHIFT_STRING, index);
  if (control)
    index = put_string(s, Am_CONTROL_STRING, index);
  if (meta)
    index = put_string(s, Am_META_STRING, index);
  if (any_modifier)
    index = put_string(s, Am_ANY_STRING, index);

  const char *code_string;
  if (code == 0)
    code_string = "NO_CHAR(code=0)";
  else
    code_string = char_map->Get_Key(code);
  if (code_string == 0) {
    // Printing THIS causes compile errors in CC
    char error[100];
    sprintf(error, "** No string for code %d.\n", (unsigned short)code);
    Am_Error(error);
  }

  if (click_count > 0) // then is mouse
  {
    if (click_count > 1 &&
        // don't put on two "ANY_"s
        (click_count < Am_ANY_CLICK || !any_modifier)) {
      const char *click_str = click_count_map->Get_Key(click_count);
      index = put_string(s, click_str, index);
    }

    index = put_string(s, code_string, index);
    if (button_down == Am_BUTTON_DOWN)
      index = put_string(s, Am_DOWN_STRING, index);
    else if (button_down == Am_BUTTON_UP)
      index = put_string(s, Am_UP_STRING, index);
    else if (button_down == Am_BUTTON_CLICK)
      index = put_string(s, Am_CLICK_STRING, index);
    else if (button_down == Am_BUTTON_DRAG)
      index = put_string(s, Am_DRAG_STRING, index);
  } else // not mouse
    index = put_string(s, code_string, index);
}

// Short string like might be used in a menu.  s should be at least
// Am_LONGEST_CHAR_STRING characters long.
void
Am_Input_Char::As_Short_String(char *s) const
{
  int index = 0;
  if (control) {
    index = put_string(s, Am_SHORT_CONTROL_STRING, index);
  }
  if (meta) {
    index = put_string(s, Am_SHORT_META_STRING, index);
  }
  if (shift) {
    //don't bother with shift for upper case letters
    if (code < 'A' || code > 'Z')
      index = put_string(s, Am_SHORT_SHIFT_STRING, index);
  }
  if (any_modifier) {
    index = put_string(s, Am_ANY_STRING, index);
  }

  const char *code_string = char_map->Get_Key(code);
  if (code_string == 0) {
    // Printing THIS causes compile errors in CC
    char error[100];
    sprintf(error, "** Illegal code %d.\n", (unsigned short)code);
    Am_Error(error);
  }

  if (click_count > 0) { // then is mouse

    if (click_count > 1 &&
        // don't put on two "ANY_"s
        (click_count < Am_ANY_CLICK || !any_modifier)) {
      const char *click_str = click_count_map->Get_Key(click_count);
      index = put_string(s, click_str, index);
    }
    index = put_string(s, code_string, index);
    if (button_down == Am_BUTTON_DOWN)
      index = put_string(s, Am_DOWN_STRING, index);
    else if (button_down == Am_BUTTON_UP)
      index = put_string(s, Am_UP_STRING, index);
    else if (button_down == Am_BUTTON_CLICK)
      index = put_string(s, Am_CLICK_STRING, index);
    else if (button_down == Am_BUTTON_DRAG)
      index = put_string(s, Am_DRAG_STRING, index);
  } else // not mouse
    index = put_string(s, code_string, index);
}

// print the input_char onto the stream.
std::ostream &
operator<<(std::ostream &os, Am_Input_Char &i)
{
  i.Print(os);
  return os;
}

// this is only used by Mac and Unix
void
Am_Input_Event::Set(Am_Input_Char ic, int new_x, int new_y, unsigned long time,
                    Am_Drawonable *draw)
{
  x = new_x;
  y = new_y;
  input_char = ic;
  time_stamp = time;
  (void)draw;

  user_id = Am_User_Id.Get(Am_USER_ID); // supports multiple users
  if (Am_Debug_Print_Input_Events) {
    std::cout << "\n<><><><><> " << ic << " x=" << x << " y=" << y
              << " time=" << time << " drawonable=" << draw
              << " user=" << user_id << // supports multiple users
        std::endl;
  }
}

bool
Am_Input_Char::Exit_If_Stop_Char() const
{
  if (*this == Am_Stop_Character) {
    std::cerr << "Got stop event: exiting Amulet main loop." << std::endl;
    //    Am_Main_Loop_Go = false;
    return true;
  } else
    return false;
}

// print the input_char onto the stream.
void
Am_Input_Char::Print(std::ostream &os)
{
  char s[Am_LONGEST_CHAR_STRING];
  As_String(s);
  os << s;
}

// // // // // // // // // // // // // // // // // // // // // // // // // // // //
//  Am_Input_Char <--> long
// // // // // // // // // // // // // // // // // // // // // // // // // // // //
//
//  The motivation for this conversion is that we want to store
//  Am_Input_Char instances in slots of objects.  We could make them
//  wrappers, but they are small enough that we can encode their data
//  into a long and store them in slots as long values.
//
//  Encoding (packing) of Am_Input_Char data into 32 bits (27 used):
//  unused click_count button_down any_mod  meta control shift code
//  00000  0000        0000        0         0   0        0    0000000000000000
//

// Convert an Am_Input_Char to a Am_Value:  build a long from the
// Am_Input_Char's bits
Am_Input_Char::operator Am_Value() const
{
  long l;
  l = click_count;
  l = (l << 4) | button_down;
  l = (l << 1) | (any_modifier & 1); // The Mac stores these values as full
  l = (l << 1) | (meta & 1);         // ints.  Pick out just the lowest bit.
  l = (l << 1) | (control & 1);
  l = (l << 1) | (shift & 1);
  l = (l << 16) | code;
  return Am_Value(l, Am_Input_Char_ID);
}

///////////////////////////////////////////////////

// print Am_Drawonable as   TITLE(0xAddress).  This is here since it
// is machine independent
std::ostream &
operator<<(std::ostream &os, Am_Drawonable *d)
{
  if (d)
    os << d->Get_Title();
  else
    os << "(0L) Drawonable";
  os << " (" << std::hex << (void *)d << std::dec << ")";
  return os;
}

class Am_Drawonable_Type_Support_Class : public Am_Type_Support
{
  void Print(std::ostream &os, const Am_Value &val) const
  {
    Am_Drawonable *d = Am_Am_Drawonable(val).value;
    d->Print(os);
  }
} am_drawonable_support;

Am_Define_Pointer_Wrapper_Impl(Am_Drawonable, &am_drawonable_support)
