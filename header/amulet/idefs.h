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

// This file contains graphic constants and types used by both Opal and Gem.
// Amulet users will need to reference classes defined in this file to
// provide parameters to Amulet functions.

#ifndef IDEFS_H
#define IDEFS_H

#include <am_inc.h>

//#include AM_IO__H
//#include TYPES__H  //for bool
#include "amulet/impl/types_logging.h"

#define Am_MAX_CLICKS 5

#if defined(_WIN32) && defined(_MSC_VER)
#pragma warning(disable : 4800)
#pragma pack(push, 2)
#endif

#define Am_SHIFT_STRING "SHIFT_"
#define Am_CONTROL_STRING "CONTROL_"
#define Am_META_STRING "META_"
#define Am_SHORT_SHIFT_STRING "SHFT-"
#define Am_SHORT_CONTROL_STRING "^"
#define Am_SHORT_META_STRING "MET-"
#define Am_ANY_STRING "ANY_"
#define Am_DOWN_STRING "_DOWN"
#define Am_UP_STRING "_UP"
#define Am_CLICK_STRING "_CLICK"
#define Am_DRAG_STRING "_DRAG"

// the length of string needed for the longest key, which is
//     SHIFT_CONTROL_META_DOUBLE_MIDDLE_DOWN = 37
//     SHIFT_CONTROL_META_COMPOSE_CHARACTER = 36
#define Am_LONGEST_CHAR_STRING 38

// special codes that go in the "code" field for non alphabetic input

// special codes for mouse keys

#define Am_MOUSE_CODES_START 0x100
enum Am_Mouse_Event_Codes
{
  Am_LEFT_MOUSE = Am_MOUSE_CODES_START,
  Am_MIDDLE_MOUSE,
  Am_RIGHT_MOUSE,
  Am_MOUSE_MOVED,
  Am_MOUSE_ENTER_WINDOW,
  Am_MOUSE_LEAVE_WINDOW,
  Am_ANY_MOUSE
};

//standard characters
#define Am_DELETE 0x7F
#define Am_ESC 0x1B
#define Am_BACKSPACE (int)'\b'
#define Am_RETURN (int)'\r'
#define Am_NEWLINE (int)'\n'
#define Am_SPACE (int)' '
#define Am_LINEFEED 0x0A
#define Am_TAB (int)'\t'

// special codes for keyboard keys
#define Am_SPECIAL_KEYS_START Am_MOUSE_CODES_START + 10

enum Am_Keyboard_Event_Codes
{
  Am_RUBOUT = Am_SPECIAL_KEYS_START,

  Am_F1,
  Am_F2,
  Am_F3,
  Am_F4,
  Am_F5,
  Am_F6,
  Am_F7,
  Am_F8,
  Am_F9,
  Am_F10,
  Am_F11,
  Am_F12,
  Am_F13,
  Am_F14,
  Am_F15,
  Am_F16,
  Am_F17,
  Am_F18,
  Am_F19,
  Am_F20,

  Am_L1,
  Am_L2,
  Am_L3,
  Am_L4,
  Am_L5,
  Am_L6,
  Am_L7,
  Am_L8,
  Am_L9,
  Am_L10,

  Am_R1,
  Am_R2,
  Am_R3,
  Am_R4,
  Am_R5,
  Am_R6,
  Am_R7,
  Am_R8,
  Am_R9,
  Am_R10,
  Am_R11,
  Am_R12,
  Am_R13,
  Am_R14,
  Am_R15,

  Am_ENTER,
  Am_NUMLOCK,
  Am_SCROLL_LOCK,

  // these are the same as some R keys on Suns, but not everywhere.

  Am_LEFT_ARROW,
  Am_UP_ARROW,
  Am_DOWN_ARROW,
  Am_RIGHT_ARROW,

  //various SUN keyboads
  Am_PRIOR,
  Am_NEXT,
  Am_SELECT,
  Am_MENU,
  Am_HELP,
  Am_ALT_GRAPH,

  //vax and dec3100 and dec5000 keyboards
  Am_FIND,
  Am_INSERT,
  Am_REMOVE,
  Am_COMPOSE_CHARACTER,
  Am_INSERT_HERE,
  Am_DO_KEY,

  //hp-ux and hp9000-300 keyboards
  Am_HOME,
  Am_BREAK,
  Am_CANCEL,
  Am_CLEAR,
  Am_EXECUTE,
  Am_INSERT_CHAR,
  Am_INSERT_LINE,
  Am_DELETE_LINE,
  Am_DELETE_CHAR,
  Am_CLEAR_LINE,
  Am_USER,

  Am_PF1,
  Am_PF2,
  Am_PF3,
  Am_PF4,

  //Mac keyboards
  Am_PAGE,
  Am_PAGE_UP,
  Am_END,
  Am_PAGE_DOWN,

  // Other standard keys
  Am_UNDO_KEY,
  Am_REDO_KEY,
  Am_PRINT_KEY,
  Am_CUT_KEY,
  Am_OPEN_KEY,
  Am_PASTE_KEY,
  Am_FRONT_KEY,
  Am_COPY_KEY,
  Am_PROPS_KEY,

  Am_ANY_KEYBOARD
};

enum Am_Click_Count
{
  Am_NOT_MOUSE = 0,    // When not a mouse button.
  Am_SINGLE_CLICK = 1, // Also mouse moved, w/ Am_NEITHER.
  Am_DOUBLE_CLICK = 2,
  Am_TRIPLE_CLICK = 3,
  Am_QUAD_CLICK = 4,
  Am_FIVE_CLICK = 5,
  Am_MANY_CLICK = 6,
  Am_ANY_CLICK = 7 //when don't care about how many clicks
};

enum Am_Button_Down
{
  Am_NEITHER = 0,
  Am_BUTTON_DOWN = 1,
  Am_BUTTON_UP = 2,
  Am_ANY_DOWN_UP = 3,
  Am_BUTTON_CLICK = 4,
  Am_BUTTON_DRAG = 5
};

// These masks should be applied to the high bytes of the long value
#define AmShiftMask 0x0001
#define AmControlMask 0x0002
#define AmMetaMask 0x0004
#define AmAnyModMask 0x0008
#define AmBDownMask 0x00F0
#define AmCCountMask 0x0F00

class _OA_DL_CLASSIMPORT Am_Input_Char
{
public:
  _OA_DL_IMPORT friend std::ostream &operator<<(std::ostream &os,
                                                Am_Input_Char &i);

  short code; // the base code
  // ** should be :16 but objectcenter bug

  bool shift : 1; // whether these modifier keys were down
  bool control : 1;
  bool meta : 1;
  bool any_modifier : 1; //true if don't care about modifiers

  Am_Button_Down button_down : 4; // whether a down or up transition.  For
                                  // keyboard, only support down.
                                  // ** should be 3 bits but objectcenter bug

  Am_Click_Count click_count : 4; // 0=not mouse, 2=double-click, etc.
                                  // ** should be 3 bits but objectcenter bug
  //constructors

  //from a string like "META_LEFT_DOWN"
  Am_Input_Char(const char *s, bool want_error = true);

  Am_Input_Char(short c = 0, bool shf = false, bool ctrl = false,
                bool mt = false, Am_Button_Down down = Am_NEITHER,
                Am_Click_Count click = Am_NOT_MOUSE, bool any_mod = false);

  //the following needs to be efficient since done at least twice for every
  //input event, including mouse movements
  Am_Input_Char(const Am_Value &value)
  {
    long l = value.value.long_value;
    if (l) {
      if (value.type != Am_Input_Char_ID) {
#ifdef DEBUG
        Am_Error("** Tried to set an Am_Input_Char with a non Am_Input_Char "
                 "wrapper.");
#else
        Am_Error();
#endif
      }
    }

    short low_bytes = (short)l;
    short high_bytes = (short)(l >> 16);

    code = low_bytes;
    shift = (high_bytes & AmShiftMask);
    control = (high_bytes & AmControlMask) >> 1;
    meta = (high_bytes & AmMetaMask) >> 2;
    any_modifier = (high_bytes & AmAnyModMask) >> 3;
    button_down = (Am_Button_Down)((high_bytes & AmBDownMask) >> 4);
    click_count = (Am_Click_Count)((high_bytes & AmCCountMask) >> 8);
  }

  Am_Input_Char &operator=(const Am_Value &value)
  {
    if (value.type != Am_Input_Char_ID) {
#ifdef DEBUG
      Am_Error(
          "** Tried to set an Am_Input_Char with a non Am_Input_Char wrapper.");
#else
      Am_Error();
#endif
    }

    long l = value.value.long_value;
    short low_bytes = (short)l;
    short high_bytes = (short)(l >> 16);

    code = low_bytes;
    shift = (high_bytes & AmShiftMask);
    control = (high_bytes & AmControlMask) >> 1;
    meta = (high_bytes & AmMetaMask) >> 2;
    any_modifier = (high_bytes & AmAnyModMask) >> 3;
    button_down = (Am_Button_Down)((high_bytes & AmBDownMask) >> 4);
    click_count = (Am_Click_Count)((high_bytes & AmCCountMask) >> 8);

    return (*this);
  }

  Am_Input_Char &operator=(const Am_Input_Char &value)
  {
    code = value.code;
    shift = value.shift;
    control = value.control;
    meta = value.meta;
    any_modifier = value.any_modifier;
    button_down = value.button_down;
    click_count = value.click_count;
    return *this;
  }

  operator Am_Value() const;

  static bool Test(const Am_Value &value)
  {
    return value.type == Am_Input_Char_ID;
  }

  static Am_Value_Type Type_ID() { return Am_Input_Char_ID; }

  unsigned char As_Char() const; // returns a char if a simple char, otherwise
  // 0.  Uses unsigned, because can get the high bit on if use
  // the META. NOT an operator char() because then
  // key to access the high parts of the font
  //  obj->Set(SLOT, Am_Input_Char("LEFT_DOWN")) is ambiguous
  // between Am_Value (which is correct) and char (which is wrong)

  void As_String(char *s) const;       // convert to a string by writing into s.
                                       // s should be at least
                                       // Am_LONGEST_CHAR_STRING characters long
  void As_Short_String(char *s) const; // Short string like might be used
                                       // in a menu.  s should be at least
                                       // Am_LONGEST_CHAR_STRING chars long.
  bool Exit_If_Stop_Char() const;

  bool Valid() const { return code != 0; }

  void Print(std::ostream &os);

  bool operator==(Am_Input_Char i) const
  {
    //quick exit for mouse_moved == something else, for efficiency.
    // Mouse-moved doesn't match ANY-* so is an easy test
    if ((i.code == Am_MOUSE_MOVED && code != Am_MOUSE_MOVED) ||
        (code == Am_MOUSE_MOVED && i.code != Am_MOUSE_MOVED))
      return false;
    else
      return helper_check_equal(i);
  }

  bool operator!=(Am_Input_Char i) const { return !(*this == i); }

protected:
  bool helper_check_equal(Am_Input_Char i) const;
  _OA_DL_MEMBERIMPORT static Am_ID_Tag Am_Input_Char_ID;
#if defined(_WIN32)
  void FixCodeAndShift();
#endif
};

_OA_DL_IMPORT extern Am_Input_Char Am_No_Input_Char;

//Debug tracing of gem input events

_OA_DL_IMPORT extern int Am_Debug_Print_Input_Events;

// Break from event loop/ Drawonable::Main_Loop() if this event happens
#define Am_STOP_CHARACTER "SHIFT_META_F1"
_OA_DL_IMPORT extern const Am_Input_Char Am_Stop_Character;
_OA_DL_IMPORT extern bool Am_Main_Loop_Go;

// initializer.  Must be called before using keytranslations
extern void Am_Initialize_Char_Map();

#if defined(_WIN32) && defined(_MSC_VER)
#pragma pack(pop)
#endif

#endif
