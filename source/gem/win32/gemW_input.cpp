#include <string.h>
#include <ctype.h>
#include <windows.h> //virtual scan codes

#include <am_inc.h>
#include <amulet/am_io.h>

#include <amulet/impl/gem_event_handlers.h>
#include <amulet/idefs.h>
#ifdef OA_VERSION
#include <amulet/univ_map_oa.hpp>
typedef OpenAmulet::Map_Int2Int Am_Map_Int2Int;
#else
#include UNIV_MAP__H
#endif
//#include <amulet/univ_map.h>

#include <amulet/symbol_table.h>

#include "amulet/gemW_32.h"
#include "amulet/gemW_input.h"

#ifndef OA_VERSION
AM_IMPL_MAP(Int2Int, int, 0, int, 0)
#endif

#pragma init_seg(lib)
Am_Map_Int2Int scan_map;
Am_Map_Int2Int spec_set;

#define set_scan_map(Scan, Code) (scan_map[Scan] = Code)
#define set_spec_set(Scan) (spec_set[Scan] = 1)

// Alex's magic incantations: I assume this is actually necessary...

void
Am_Init_Key_Map()
{
  //set_scan_map(,		Am_RUBOUT);
  set_scan_map(VK_DELETE, Am_DELETE);
  set_scan_map(VK_ESCAPE, Am_ESC);
  set_scan_map(VK_BACK, Am_BACKSPACE);
  //set_scan_map(,		Am_RETURN);
  //set_scan_map(, 		Am_NEWLINE);
  set_scan_map(VK_SPACE, Am_SPACE);
  //set_scan_map(, 		Am_LINEFEED);
  set_scan_map(VK_TAB, Am_TAB);

  //function keys
  set_scan_map(VK_F1, Am_F1);
  set_scan_map(VK_F2, Am_F2);
  set_scan_map(VK_F3, Am_F3);
  set_scan_map(VK_F4, Am_F4);
  set_scan_map(VK_F5, Am_F5);
  set_scan_map(VK_F6, Am_F6);
  set_scan_map(VK_F7, Am_F7);
  set_scan_map(VK_F8, Am_F8);
  set_scan_map(VK_F9, Am_F9);
  set_scan_map(VK_F10, Am_F10);
  set_scan_map(VK_F11, Am_F11);
  set_scan_map(VK_F12, Am_F12);
  set_scan_map(VK_F13, Am_F13);
  set_scan_map(VK_F14, Am_F14);
  set_scan_map(VK_F15, Am_F15);
  set_scan_map(VK_F16, Am_F16);
  set_scan_map(VK_F17, Am_F17);
  set_scan_map(VK_F18, Am_F18);
  set_scan_map(VK_F19, Am_F19);
  set_scan_map(VK_F20, Am_F20);

  set_scan_map(VK_RETURN, Am_RETURN); //##
  set_scan_map(VK_NUMLOCK, Am_NUMLOCK);

  set_scan_map(VK_LEFT, Am_LEFT_ARROW);
  set_scan_map(VK_UP, Am_UP_ARROW);
  set_scan_map(VK_DOWN, Am_DOWN_ARROW);
  set_scan_map(VK_RIGHT, Am_RIGHT_ARROW);

  //various SUN keyboards
  set_scan_map(VK_PRIOR, Am_PRIOR);
  set_scan_map(VK_NEXT, Am_NEXT);
  set_scan_map(VK_SELECT, Am_SELECT);
  //set_scan_map(VK_MENU,	Am_MENU);
  set_scan_map(VK_HELP, Am_HELP);

  //vax and dec3100 and dec5000 keyboards
  //set_scan_map(,		Am_FIND);
  //set_scan_map(,		Am_INSERT);
  //set_scan_map(,		Am_REMOVE);
  //set_scan_map(,		Am_COMPOSE_CHARACTER);
  //set_scan_map(,		Am_INSERT_HERE);
  set_scan_map(VK_END, Am_DO_KEY); //???

  //hp-ux and hp9000-300 keyboards
  set_scan_map(VK_HOME, Am_HOME);
  set_scan_map(VK_PAUSE, Am_BREAK);
  set_scan_map(VK_CANCEL, Am_CANCEL);
  set_scan_map(VK_CLEAR, Am_CLEAR);
  set_scan_map(VK_EXECUTE, Am_EXECUTE);
  set_scan_map(VK_INSERT, Am_INSERT_CHAR);
  //set_scan_map(,		Am_INSERT_LINE);
  //set_scan_map(,		Am_DELETE_LINE);
  //set_scan_map(,		Am_DELETE_CHAR);
  //set_scan_map(,		Am_CLEAR_LINE);
  //set_scan_map(,		Am_Immutable_WrapperR);

  set_spec_set(VK_LBUTTON);
  set_spec_set(VK_RBUTTON);
  set_spec_set(VK_CANCEL);
  set_spec_set(VK_MBUTTON);
  //VK_BACK	08	BACKSPACE key
  //VK_TAB	09	TAB key
  set_spec_set(VK_CLEAR);
  //VK_RETURN	0D	ENTER key
  set_spec_set(VK_SHIFT);
  set_spec_set(VK_CONTROL);
  set_spec_set(VK_MENU);
  set_spec_set(VK_PAUSE);
  set_spec_set(VK_CAPITAL);
  //VK_ESCAPE	1B	ESC key
  //VK_SPACE	20	SPACEBAR
  set_spec_set(VK_PRIOR);
  set_spec_set(VK_NEXT);
  set_spec_set(VK_END);
  set_spec_set(VK_HOME);
  set_spec_set(VK_LEFT);
  set_spec_set(VK_UP);
  set_spec_set(VK_RIGHT);
  set_spec_set(VK_DOWN);
  set_spec_set(VK_SELECT);
  set_spec_set(VK_EXECUTE);
  set_spec_set(VK_SNAPSHOT);
  set_spec_set(VK_INSERT);
  set_spec_set(VK_DELETE);
  set_spec_set(VK_HELP);
  //VK_NUMPAD0	60	Numeric keypad 0 key
  //VK_NUMPAD1	61	Numeric keypad 1 key
  //VK_NUMPAD2	62	Numeric keypad 2 key
  //VK_NUMPAD3	63	Numeric keypad 3 key
  //VK_NUMPAD4	64	Numeric keypad 4 key
  //VK_NUMPAD5	65	Numeric keypad 5 key
  //VK_NUMPAD6	66	Numeric keypad 6 key
  //VK_NUMPAD7	67	Numeric keypad 7 key
  //VK_NUMPAD8	68	Numeric keypad 8 key
  //VK_NUMPAD9	69	Numeric keypad 9 key
  //VK_MULTIPLY	6A	Multiply key
  //VK_ADD	6B	Add key
  set_spec_set(VK_SEPARATOR);
  //VK_SUBTRACT	6D	Subtract key
  set_spec_set(VK_DECIMAL);
  //VK_DIVIDE	6F	Divide key
  set_spec_set(VK_F1);
  set_spec_set(VK_F2);
  set_spec_set(VK_F3);
  set_spec_set(VK_F4);
  set_spec_set(VK_F5);
  set_spec_set(VK_F6);
  set_spec_set(VK_F7);
  set_spec_set(VK_F8);
  set_spec_set(VK_F9);
  set_spec_set(VK_F10);
  set_spec_set(VK_F11);
  set_spec_set(VK_F12);
  set_spec_set(VK_F13);
  set_spec_set(VK_F14);
  set_spec_set(VK_F15);
  set_spec_set(VK_F16);
  set_spec_set(VK_F17);
  set_spec_set(VK_F18);
  set_spec_set(VK_F19);
  set_spec_set(VK_F20);
  set_spec_set(VK_F21);
  set_spec_set(VK_F22);
  set_spec_set(VK_F23);
  set_spec_set(VK_F24);
  set_spec_set(VK_NUMLOCK);
  set_spec_set(VK_SCROLL);
}

short
Am_WinScanToCode(WORD wScan, bool fChar)
{
  if (fChar) {
    static bool fShiftJIS = false; // true when 1st byte of two-byte code

    if (isprint((int)wScan) || fShiftJIS ||
        0xA0 <= (int)wScan && (int)wScan <= 0xDF) {
      // one-byte code or 2nd byte of two-byte code
      fShiftJIS = false;
      return ((short)wScan);
    } else if (0x81 <= (int)wScan && (int)wScan <= 0x9F ||
               0xE0 <= (int)wScan && (int)wScan <= 0xFC) {
      // 1st byte of two-byte code
      fShiftJIS = true;
      return ((short)wScan);
    } else {
      short code = (short)scan_map.GetAt((int)wScan);
      return code ? code : -1;
    }
  } else {
    short code = (short)scan_map.GetAt((int)wScan);
    return code ? code : isprint((int)wScan) ? (short)wScan : -1;
  }
}

BOOL
WinScanSpec(WORD wScan)
{
  return spec_set.GetAt((int)wScan) != 0;
}

/*
// Helper functions

bool stripref(const char *s, const char *t)
{
	for ( ; *s && (toupper(*s) == toupper(*t)); s++, t++)
		;
	return !(bool)*t;
}

char* stristr(char *s, const char *t)
{
	for ( ; *s; s++)
		if (stripref(s, t))
			return s;
	return (0L);
}

// searches for target in s.  If found (case insensitive!!!), returns true and
// modifies s so it no longer contains target.  If not found, returns false
bool stristr_rem(char *s, const char *t)
{
	char* f = stristr(s, t);
	if (!f)
		return false;
	else {
		memmove(f, f + strlen(t), strlen(f));
		return true;
	}
}

// finds the string ending with target_char at the front of s.
// if found, then sets prefix with that string converted to upper case
// (including the target_char) and
// removes if from s and returns true.  If not found, returns false.
// target_char should not be 0 (the string terminator)
bool strpref_rem (char *pref, char *s, char ctarget) {
	char* prend = strchr(s, ctarget);
	
	if (!prend) return false;
	
	size_t prlen = prend - s + 1; //including ctarget;
	strncpy(pref, s, prlen);
	pref[prlen] = '\0';
	_strupr(pref);
	
	memmove(s, prend + 1, strlen(s) - prlen + 1); //including term.
	    return true;
}		 */
