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

// This file contains member function definitions for the Am_Drawonable_Impl
// object primarily concerned with interaction.
//
// Stuart contributed code which allows the top half (non-ASCII part) of the
// isolatin1 character set to be entered from the keyboard. He included a patch
// to to use XLookupString


#include <iostream>

#include <am_inc.h>

#include GEM__H

#include "amulet/gemG_time.h"
#include GEMG__H

#include <ggi/events.h>
#include <ggi/gwt.h>

//
// Global constant
//
bool Am_Main_Loop_Go = true;

//
// Current state of event modifiers
//
static uint32 Am_Event_Modifiers = 0;

//
// Convert a struct timeval into a milliseconds thing
//
static inline long unsigned int timeval2long (struct timeval t)
{
  return (static_cast<long unsigned int>(t.tv_sec * 1000L)
	  + (static_cast<long unsigned int>(t.tv_usec) / 1000L)); // ms
}
// // // // // // // // // // // // // // // // // // // //
// Convert a GGI input event into a Am_Input_Char
// // // // // // // // // // // // // // // // // // // //

Am_Input_Char create_input_char_from_code
(short code,
 uint32 modifiers, // Am_Event_Modifiers in fact
 Am_Button_Down down,
 Am_Click_Count mouse)
{
  bool shft = ( (modifiers & GII_MOD_SHIFT)  // Regular shift
		|| ( (modifiers & GII_MOD_CAPS) // Capslock for alpha. chars
		     && (code >= 'a') && (code <= 'z')) )
    ? true : false;
  bool ctrl = (modifiers & GII_MOD_CTRL) ? true : false;
  bool meta = (modifiers & GII_MOD_META) ? true : false;

  Am_Input_Char ic = Am_Input_Char(code, shft, ctrl, meta, down, mouse);
  return ic;
}

#if 0
//
// Multi-Click management
//
int Am_Double_Click_Time = 250;  // in milleseconds
static Am_Click_Count Check_Multi_Click(int code, unsigned int state,
					Am_Button_Down down,
					Time time, Screen_Desc* screen)
{
  Am_Click_Count result = Am_SINGLE_CLICK;

  if (Am_Double_Click_Time)
    { // else not interested in multi-click
      // if a down press, then check if double click. If up, then use current
      // down count.  If other mouse event, then ignore multi-click
      if ( down == Am_NEITHER) ; // result is OK, do nothing
      else if ( down == Am_BUTTON_UP)
	{ // use current value

	  if (screen->click_counter >= 7)
	    result = Am_MANY_CLICK;
	  else
	    result = (Am_Click_Count) (screen->click_counter + 1);
	  // otherwise, just use single click, so result OK
	}
      else
	{ // is a down press
	  if ( (code  == screen->last_click_code) &&
	       (state == screen->last_click_state) &&
	       ( time <= screen->last_click_time + Am_Double_Click_Time))
	    {
	      // is multi-click
	      ++(screen->click_counter);

	      if (screen->click_counter >= 7)
		result = Am_MANY_CLICK;
	      else
		result = (Am_Click_Count) (screen->click_counter + 1);
	    }
	  else
	    screen->click_counter = 0;

	  // in either case, set up variables for next time
	  screen->last_click_code = code;
	  screen->last_click_state = state;
	  screen->last_click_time = time;

	}
    }
  return result;
}
#endif

#if 1 /* THE KEY MAP */

// TODO: CHANGE THAT SPECIFIC MAP FUNCTION !!!
#ifndef OA_VERSION
Am_IMPL_MAP(int2int, int, 0, int, 0)
#endif
static Am_Map_int2int *am_key_map = 0;

// returns character code or 0 if modifier or -1 if illegal
short Map_Sym_To_Code(uint32 sym)
{
  // Initialize the keymap
  if (am_key_map == 0)
    Am_Init_Key_Map();
  // Get code
  short c = (short)(am_key_map->GetAt((int) sym));
  if (c)
    return c;

  if ( (sym >= 65505) && (sym <= 65518) )
    {
      // symbol is a modifier key
     std::cerr << "Symbol is a modifier key !!!" <<std::endl;
      return 0;
    }
  else
    { // unknown character
      return -1;
    }
}

void set_am_key_map (int Sym, int Code)
{
  am_key_map->SetAt(Sym, Code);
}

void Am_Init_Key_Map()
{
  if (am_key_map == 0)
    am_key_map = new Am_Map_int2int(); // TODO: Check size
  // The commented out lines have no Amulet equivalent.  Check there first!
  // Look for GIIK_* constants in ggi/keyboard.h
  set_am_key_map(GIIUC_BackSpace, Am_BACKSPACE);
  set_am_key_map(GIIUC_Tab, Am_TAB);
  set_am_key_map(GIIUC_Linefeed, Am_LINEFEED);
  set_am_key_map(GIIK_Clear, Am_CLEAR);
  set_am_key_map(GIIUC_Return, Am_RETURN);
  set_am_key_map(GIIK_Pause, Am_BREAK);
  set_am_key_map(GIIK_ScrollLock, Am_SCROLL_LOCK);
  set_am_key_map(GIIK_ModeSwitch, Am_ALT_GRAPH);
  //  set_am_key_map(GIIK_SysRq, Am_SYS_REQ);
  set_am_key_map(GIIUC_Escape, Am_ESC);
  set_am_key_map(GIIK_Delete, Am_DELETE);

  set_am_key_map (GIIK_Compose, Am_COMPOSE_CHARACTER);
  set_am_key_map(GIIK_Home, Am_HOME);
  set_am_key_map(GIIK_Left, Am_LEFT_ARROW);
  set_am_key_map(GIIK_Up, Am_UP_ARROW);
  set_am_key_map(GIIK_Right, Am_RIGHT_ARROW);
  set_am_key_map(GIIK_Down, Am_DOWN_ARROW);
  //  set_am_key_map(GIIK_Prior, Am_PRIOR); GIIK_PageUp == GIIK_Prior !
  set_am_key_map(GIIK_PageUp, Am_PAGE_UP);
  // set_am_key_map(GIIK_Next, Am_NEXT); GIIK_PageDown == GIIK_Next
  set_am_key_map(GIIK_PageDown, Am_PAGE_DOWN);
  set_am_key_map(GIIK_End, Am_END);
  //  set_am_key_map(GIIK_Begin, Am_BEGIN);
  set_am_key_map(GIIK_Select, Am_SELECT);
  set_am_key_map(GIIK_PrintScreen, Am_PRINT_KEY);
  //  set_am_key_map(GIIK_Execute, Am_EXECUTE);
  set_am_key_map(GIIK_Insert, Am_INSERT);
  set_am_key_map(GIIK_Undo, Am_UNDO_KEY);
  set_am_key_map(GIIK_Redo, Am_REDO_KEY);
  set_am_key_map(GIIK_Menu, Am_MENU);
  set_am_key_map(GIIK_Find, Am_FIND);
  set_am_key_map(GIIK_Cancel, Am_CANCEL);
  set_am_key_map(GIIK_Help, Am_HELP);
  set_am_key_map(GIIK_Break, Am_BREAK);
  set_am_key_map(GIIK_NumLock, Am_NUMLOCK);

  set_am_key_map(GIIK_PSpace, Am_SPACE);
  set_am_key_map(GIIK_PTab, Am_TAB);
  set_am_key_map(GIIK_PEnter, Am_ENTER);
  set_am_key_map(GIIK_PF1, Am_PF1);
  set_am_key_map(GIIK_PF2, Am_PF2);
  set_am_key_map(GIIK_PF3, Am_PF3);
  set_am_key_map(GIIK_PF4, Am_PF4);
  set_am_key_map(GIIK_PBegin, Am_R11);
  // TODO:? Should be: set_am_key_map(GIIK_PBegin, Am_BEGIN);
  set_am_key_map(GIIK_PEqual, (short)'=');
  set_am_key_map(GIIK_PAsterisk, (short)'*');
  set_am_key_map(GIIK_PPlus, (short)'+');
  set_am_key_map(GIIK_PSeparator, (short)',');
  set_am_key_map(GIIK_PMinus, (short)'-');
  set_am_key_map(GIIK_PDecimal, (short)'.');
  set_am_key_map(GIIK_PSlash, (short)'/');
  set_am_key_map(GIIK_PParenLeft, (short)'(');
  set_am_key_map(GIIK_PParenRight, (short)')');

  set_am_key_map(GIIK_P0, (short)'0');
  set_am_key_map(GIIK_P1, (short)'1');
  set_am_key_map(GIIK_P2, (short)'2');
  set_am_key_map(GIIK_P3, (short)'3');
  set_am_key_map(GIIK_P4, (short)'4');
  set_am_key_map(GIIK_P5, (short)'5');
  set_am_key_map(GIIK_P6, (short)'6');
  set_am_key_map(GIIK_P7, (short)'7');
  set_am_key_map(GIIK_P8, (short)'8');
  set_am_key_map(GIIK_P9, (short)'9');

  set_am_key_map(GIIK_F1, Am_F1);
  set_am_key_map(GIIK_F2, Am_F2);
  set_am_key_map(GIIK_F3, Am_F3);
  set_am_key_map(GIIK_F4, Am_F4);
  set_am_key_map(GIIK_F5, Am_F5);
  set_am_key_map(GIIK_F6, Am_F6);
  set_am_key_map(GIIK_F7, Am_F7);
  set_am_key_map(GIIK_F8, Am_F8);
  set_am_key_map(GIIK_F9, Am_F9);
  set_am_key_map(GIIK_F10, Am_F10);

#if 0 // TODO: Check (should be PF1-PF9 ? ???) -- ortalo
  set_am_key_map(GIIK_L1, Am_L1);
  set_am_key_map(GIIK_L2, Am_L2);
  set_am_key_map(GIIK_L3, Am_L3);
  set_am_key_map(GIIK_L4, Am_L4);
  set_am_key_map(GIIK_L5, Am_L5);
  set_am_key_map(GIIK_L6, Am_L6);
  set_am_key_map(GIIK_L7, Am_L7);
  set_am_key_map(GIIK_L8, Am_L8);
  set_am_key_map(GIIK_L9, Am_L9);
  set_am_key_map(GIIK_L10, Am_L10);
#endif
#if 0 // TODO: Check (should be F11-F25 ???) -- ortalo
  set_am_key_map(GIIK_R1, Am_R1);
  set_am_key_map(GIIK_R2, Am_R2);
  set_am_key_map(GIIK_R3, Am_R3);
  set_am_key_map(GIIK_R4, Am_R4);
  set_am_key_map(GIIK_R5, Am_R5);
  set_am_key_map(GIIK_R6, Am_R6);
  set_am_key_map(GIIK_R7, Am_R7);
  set_am_key_map(GIIK_R8, Am_R8);
  set_am_key_map(GIIK_R9, Am_R9);
  set_am_key_map(GIIK_R10, Am_R10);
  set_am_key_map(GIIK_R11, Am_R11);
  set_am_key_map(GIIK_R12, Am_R12);
  set_am_key_map(GIIK_R13, Am_R13);
  set_am_key_map(GIIK_R14, Am_R14);
  set_am_key_map(GIIK_R15, Am_R15);
#endif

  set_am_key_map(GIIUC_Space, Am_SPACE);
  set_am_key_map(GIIUC_Exclamation, (short)'!');
  set_am_key_map(GIIUC_DoubleQuote, (short)'\"');
  set_am_key_map(GIIUC_NumberSign, (short)'#');
  set_am_key_map(GIIUC_Dollar, (short)'$');
  set_am_key_map(GIIUC_Percent, (short)'%');
  set_am_key_map(GIIUC_Ampersand, (short)'&');
  set_am_key_map(GIIUC_Apostrophe, (short)'\'');
  set_am_key_map(GIIUC_ParenLeft, (short)'(');
  set_am_key_map(GIIUC_ParenRight, (short)')');
  set_am_key_map(GIIUC_Asterisk, (short)'*');
  set_am_key_map(GIIUC_Plus, (short)'+');
  set_am_key_map(GIIUC_Comma, (short)',');
  set_am_key_map(GIIUC_Minus, (short)'-');
  set_am_key_map(GIIUC_Period, (short)'.');
  set_am_key_map(GIIUC_Slash, (short)'/');
  set_am_key_map(GIIUC_0, (short)'0');
  set_am_key_map(GIIUC_1, (short)'1');
  set_am_key_map(GIIUC_2, (short)'2');
  set_am_key_map(GIIUC_3, (short)'3');
  set_am_key_map(GIIUC_4, (short)'4');
  set_am_key_map(GIIUC_5, (short)'5');
  set_am_key_map(GIIUC_6, (short)'6');
  set_am_key_map(GIIUC_7, (short)'7');
  set_am_key_map(GIIUC_8, (short)'8');
  set_am_key_map(GIIUC_9, (short)'9');
  set_am_key_map(GIIUC_Colon, (short)':');
  set_am_key_map(GIIUC_Semicolon, (short)';');
  set_am_key_map(GIIUC_Less, (short)'<');
  set_am_key_map(GIIUC_Equal, (short)'=');
  set_am_key_map(GIIUC_Greater, (short)'>');
  set_am_key_map(GIIUC_Question, (short)'?');
  set_am_key_map(GIIUC_At, (short)'@');
  set_am_key_map(GIIUC_A, (short)'A');
  set_am_key_map(GIIUC_B, (short)'B');
  set_am_key_map(GIIUC_C, (short)'C');
  set_am_key_map(GIIUC_D, (short)'D');
  set_am_key_map(GIIUC_E, (short)'E');
  set_am_key_map(GIIUC_F, (short)'F');
  set_am_key_map(GIIUC_G, (short)'G');
  set_am_key_map(GIIUC_H, (short)'H');
  set_am_key_map(GIIUC_I, (short)'I');
  set_am_key_map(GIIUC_J, (short)'J');
  set_am_key_map(GIIUC_K, (short)'K');
  set_am_key_map(GIIUC_L, (short)'L');
  set_am_key_map(GIIUC_M, (short)'M');
  set_am_key_map(GIIUC_N, (short)'N');
  set_am_key_map(GIIUC_O, (short)'O');
  set_am_key_map(GIIUC_P, (short)'P');
  set_am_key_map(GIIUC_Q, (short)'Q');
  set_am_key_map(GIIUC_R, (short)'R');
  set_am_key_map(GIIUC_S, (short)'S');
  set_am_key_map(GIIUC_T, (short)'T');
  set_am_key_map(GIIUC_U, (short)'U');
  set_am_key_map(GIIUC_V, (short)'V');
  set_am_key_map(GIIUC_W, (short)'W');
  set_am_key_map(GIIUC_X, (short)'X');
  set_am_key_map(GIIUC_Y, (short)'Y');
  set_am_key_map(GIIUC_Z, (short)'Z');

  set_am_key_map(GIIUC_BracketLeft, (short)'[');
  set_am_key_map(GIIUC_BackSlash, (short)'\\');
  set_am_key_map(GIIUC_BracketRight, (short)']');
  set_am_key_map(GIIUC_Circumflex, (short)'^');
  set_am_key_map(GIIUC_Underscore, (short)'_');
  set_am_key_map(GIIUC_Grave, (short)'`'); // NB: Also quote left
  set_am_key_map(GIIUC_a, (short)'a');
  set_am_key_map(GIIUC_b, (short)'b');
  set_am_key_map(GIIUC_c, (short)'c');
  set_am_key_map(GIIUC_d, (short)'d');
  set_am_key_map(GIIUC_e, (short)'e');
  set_am_key_map(GIIUC_f, (short)'f');
  set_am_key_map(GIIUC_g, (short)'g');
  set_am_key_map(GIIUC_h, (short)'h');
  set_am_key_map(GIIUC_i, (short)'i');
  set_am_key_map(GIIUC_j, (short)'j');
  set_am_key_map(GIIUC_k, (short)'k');
  set_am_key_map(GIIUC_l, (short)'l');
  set_am_key_map(GIIUC_m, (short)'m');
  set_am_key_map(GIIUC_n, (short)'n');
  set_am_key_map(GIIUC_o, (short)'o');
  set_am_key_map(GIIUC_p, (short)'p');
  set_am_key_map(GIIUC_q, (short)'q');
  set_am_key_map(GIIUC_r, (short)'r');
  set_am_key_map(GIIUC_s, (short)'s');
  set_am_key_map(GIIUC_t, (short)'t');
  set_am_key_map(GIIUC_u, (short)'u');
  set_am_key_map(GIIUC_v, (short)'v');
  set_am_key_map(GIIUC_w, (short)'w');
  set_am_key_map(GIIUC_x, (short)'x');
  set_am_key_map(GIIUC_y, (short)'y');
  set_am_key_map(GIIUC_z, (short)'z');
  set_am_key_map(GIIUC_BraceLeft, (short)'{');
  set_am_key_map(GIIUC_Bar, (short)'|');
  set_am_key_map(GIIUC_BraceRight, (short)'}');
  set_am_key_map(GIIUC_Tilde, (short)'~');
  // since we are doing latin1 let's handle the top half as well
  set_am_key_map(GIIUC_NoBreakSpace, 0x0a0);
  set_am_key_map(GIIUC_ExclamDown, 0x0a1);
  set_am_key_map(GIIUC_Cent, 0x0a2);
  set_am_key_map(GIIUC_Sterling, 0x0a3);
  set_am_key_map(GIIUC_Currency, 0x0a4);
  set_am_key_map(GIIUC_Yen, 0x0a5);
  set_am_key_map(GIIUC_BrokenBar, 0x0a6);
  set_am_key_map(GIIUC_Section, 0x0a7);
  set_am_key_map(GIIUC_Diaeresis, 0x0a8);
  set_am_key_map(GIIUC_Copyright, 0x0a9);
  set_am_key_map(GIIUC_OrdFeminine, 0x0aa);
  set_am_key_map(GIIUC_GuillemotLeft, 0x0ab);
  set_am_key_map(GIIUC_NotSign, 0x0ac);
  set_am_key_map(GIIUC_SoftHyphen, 0x0ad);
  set_am_key_map(GIIUC_Registered, 0x0ae);
  set_am_key_map(GIIUC_Macron, 0x0af);
  set_am_key_map(GIIUC_Degree, 0x0b0);
  set_am_key_map(GIIUC_PlusMinus, 0x0b1);
  set_am_key_map(GIIUC_TwoSuperior, 0x0b2);
  set_am_key_map(GIIUC_ThreeSuperior, 0x0b3);
  set_am_key_map(GIIUC_Acute, 0x0b4);
  set_am_key_map(GIIUC_Mu, 0x0b5);
  set_am_key_map(GIIUC_Paragraph, 0x0b6);
  set_am_key_map(GIIUC_PeriodCentered, 0x0b7);
  set_am_key_map(GIIUC_Cedilla, 0x0b8);
  set_am_key_map(GIIUC_OneSuperior, 0x0b9);
  set_am_key_map(GIIUC_mKuline, 0x0ba);
  set_am_key_map(GIIUC_GuillemotRight, 0x0bb);
  set_am_key_map(GIIUC_OneQuarter, 0x0bc);
  set_am_key_map(GIIUC_OneHalf, 0x0bd);
  set_am_key_map(GIIUC_ThreeQuarters, 0x0be);
  set_am_key_map(GIIUC_QuestionDown, 0x0bf);
  set_am_key_map(GIIUC_Agrave, 0x0c0);
  set_am_key_map(GIIUC_Aacute, 0x0c1);
  set_am_key_map(GIIUC_Acircumflex, 0x0c2);
  set_am_key_map(GIIUC_Atilde, 0x0c3);
  set_am_key_map(GIIUC_Adiaeresis, 0x0c4);
  set_am_key_map(GIIUC_Aring, 0x0c5);
  set_am_key_map(GIIUC_AE, 0x0c6);
  set_am_key_map(GIIUC_Ccedilla, 0x0c7);
  set_am_key_map(GIIUC_Egrave, 0x0c8);
  set_am_key_map(GIIUC_Eacute, 0x0c9);
  set_am_key_map(GIIUC_Ecircumflex, 0x0ca);
  set_am_key_map(GIIUC_Ediaeresis, 0x0cb);
  set_am_key_map(GIIUC_Igrave, 0x0cc);
  set_am_key_map(GIIUC_Iacute, 0x0cd);
  set_am_key_map(GIIUC_Icircumflex, 0x0ce);
  set_am_key_map(GIIUC_Idiaeresis, 0x0cf);
  set_am_key_map(GIIUC_ETH, 0x0d0);
  set_am_key_map(GIIUC_Ntilde, 0x0d1);
  set_am_key_map(GIIUC_Ograve, 0x0d2);
  set_am_key_map(GIIUC_Oacute, 0x0d3);
  set_am_key_map(GIIUC_Ocircumflex, 0x0d4);
  set_am_key_map(GIIUC_Otilde, 0x0d5);
  set_am_key_map(GIIUC_Odiaeresis, 0x0d6);
  set_am_key_map(GIIUC_Multiply, 0x0d7);
  set_am_key_map(GIIUC_Ooblique, 0x0d8);
  set_am_key_map(GIIUC_Ugrave, 0x0d9);
  set_am_key_map(GIIUC_Uacute, 0x0da);
  set_am_key_map(GIIUC_Ucircumflex, 0x0db);
  set_am_key_map(GIIUC_Udiaeresis, 0x0dc);
  set_am_key_map(GIIUC_Yacute, 0x0dd);
  set_am_key_map(GIIUC_THORN, 0x0de);
  set_am_key_map(GIIUC_ssharp, 0x0df);
  set_am_key_map(GIIUC_agrave, 0x0e0);
  set_am_key_map(GIIUC_aacute, 0x0e1);
  set_am_key_map(GIIUC_acircumflex, 0x0e2);
  set_am_key_map(GIIUC_atilde, 0x0e3);
  set_am_key_map(GIIUC_adiaeresis, 0x0e4);
  set_am_key_map(GIIUC_aring, 0x0e5);
  set_am_key_map(GIIUC_ae, 0x0e6);
  set_am_key_map(GIIUC_ccedilla, 0x0e7);
  set_am_key_map(GIIUC_egrave, 0x0e8);
  set_am_key_map(GIIUC_eacute, 0x0e9);
  set_am_key_map(GIIUC_ecircumflex, 0x0ea);
  set_am_key_map(GIIUC_ediaeresis, 0x0eb);
  set_am_key_map(GIIUC_igrave, 0x0ec);
  set_am_key_map(GIIUC_iacute, 0x0ed);
  set_am_key_map(GIIUC_icircumflex, 0x0ee);
  set_am_key_map(GIIUC_idiaeresis, 0x0ef);
  set_am_key_map(GIIUC_eth, 0x0f0);
  set_am_key_map(GIIUC_ntilde, 0x0f1);
  set_am_key_map(GIIUC_ograve, 0x0f2);
  set_am_key_map(GIIUC_oacute, 0x0f3);
  set_am_key_map(GIIUC_ocircumflex, 0x0f4);
  set_am_key_map(GIIUC_otilde, 0x0f5);
  set_am_key_map(GIIUC_odiaeresis, 0x0f6);
  set_am_key_map(GIIUC_Division, 0x0f7);
  set_am_key_map(GIIUC_oslash, 0x0f8);
  set_am_key_map(GIIUC_ugrave, 0x0f9);
  set_am_key_map(GIIUC_uacute, 0x0fa);
  set_am_key_map(GIIUC_ucircumflex, 0x0fb);
  set_am_key_map(GIIUC_udiaeresis, 0x0fc);
  set_am_key_map(GIIUC_yacute, 0x0fd);
  set_am_key_map(GIIUC_thorn, 0x0fe);
  set_am_key_map(GIIUC_ydiaeresis, 0x0ff);

  // The next section is non-ideal.
  // The best solution is to find (create?) #defines for these magic
  // numbers, like the GIIK_* macros above.
  // I couldn't find any of these keysyms anywhere.  They're all
  // on Sun keyboards.  In the Older Amulet translation, most of these went
  // to L keys, but that's _not_ correct: you should get the keycode you want,
  // instead of translating it to L anything.
  // 5-31-96 af1x

  set_am_key_map (268500736, Am_REMOVE);
  set_am_key_map (268500850, Am_INSERT_CHAR);
  set_am_key_map (268500848, Am_INSERT_LINE);
  set_am_key_map (268500849, Am_DELETE_LINE);
  set_am_key_map (268500851, Am_DELETE_CHAR);
  set_am_key_map (268500847, Am_CLEAR_LINE);
  set_am_key_map (268500845, Am_USER);

  set_am_key_map (268828528, Am_PROPS_KEY);
  set_am_key_map (268828529, Am_FRONT_KEY);
  set_am_key_map (268828530, Am_COPY_KEY);
  set_am_key_map (268828531, Am_OPEN_KEY);
  set_am_key_map (268828532, Am_PASTE_KEY);
  set_am_key_map (268828533, Am_CUT_KEY);
  /*
    // these aren't handled yet: what Am_ consts should we use?
    case 268828535: c = Am_; break; // SunAudioMute
    case 268828536: c = Am_; break; // SunAudioLowerVolume
    case 268828537: c = Am_; break; // SunAudioRaiseVolume
    case 268828534: c = Am_; break; // SunPowerSwitch
    */

}
#endif /* THE KEY_MAP */

Am_Input_Char create_input_char_from_key (gii_event ev)
{
  short code;
  if ((ev.key.sym == 0) || (ev.key.sym == GIIK_VOID))
    {
      code = 0;
      // TODO: Remove that test output:
     std::cerr << "Obtained a 'zero' event.key.sym: " << ev.key.sym <<std::endl;
    }
  else
    {
      code = Map_Sym_To_Code (ev.key.sym);
     std::cerr << "keysym translation: sym=" << ev.key.sym << " code=" << code <<std::endl;
    }

  if (code > 0)
    {
      // We only support keyboard keys going down
      return create_input_char_from_code (code, Am_Event_Modifiers, Am_NEITHER,
					  Am_NOT_MOUSE);
    }
  else
    {
      // in case is an illegal character or modifier
      if (code < 0)
std::cout << "** Unknown keyboard symbol " << ev.key.sym << " ignored\n" <<std::flush;
      return Am_Input_Char (); // null means illegal
  }
}

Am_Input_Char create_input_char_from_mouse (uint32 button,
					    Am_Button_Down down,
					    long unsigned int time)
{
    int code = 0;
    switch (GII_PBUTTON_(button))
      {
      case GII_PBUTTON_LEFT: // or PRIMARY
	code = Am_LEFT_MOUSE;
	break;
      case GII_PBUTTON_MIDDLE: // or TERTIARY
	code = Am_MIDDLE_MOUSE;
	break;
      case GII_PBUTTON_RIGHT: // or SECUNDARY
	code = Am_RIGHT_MOUSE;
	break;
      default:
std::cerr << "** Unknown mouse button " << button <<std::endl;
	Am_Error ();
	break;
    }

    //    Am_Click_Count cnt = Check_Multi_Click(code, state, down, time, screen);
    Am_Click_Count cnt = Am_SINGLE_CLICK;

    return create_input_char_from_code (code, Am_Event_Modifiers, down, cnt);
}

// // // // // // // // // // // // // // // // // // // //
// Main Input Event Handler
// // // // // // // // // // // // // // // // // // // //

void set_input_event (Am_Input_Event *ev, Am_Input_Char ic, int x, int y,
		      unsigned long time, Am_Drawonable *draw) {
  // multiple user support is provided in Am_Input_Event::Set()
  ev->Set(ic, x, y, time, draw);
}

bool exit_if_stop_char(Am_Input_Char ic)
{
  if (ic == Am_Stop_Character)
    {
     std::cerr << "Got stop event: exiting Amulet main loop." <<std::endl;
      Am_Main_Loop_Go = false;
      return true;
    }
  else return false;
}

#if 0
void handle_selection_request (XEvent& ev, Am_Drawonable_Impl* draw) {
  // someone wants our selection.
  XEvent notify;
  Atom p;
  if (draw->screen->cut_data == (0L))
    p = None;
  else {
    p = ev.xselectionrequest.property;
    XChangeProperty(ev.xselectionrequest.display, //draw->screen->display,
		    ev.xselectionrequest.requestor,
		    ev.xselectionrequest.property,
		    XA_STRING, 8, PropModeReplace,
		    (unsigned char*)draw->screen->cut_data,
		    strlen(draw->screen->cut_data));
  }
  notify.xany.type = SelectionNotify;
  notify.xselection.display =
    ev.xselectionrequest.display;
  notify.xselection.send_event = True;
  notify.xselection.requestor =
    ev.xselectionrequest.requestor;
  notify.xselection.selection =
    ev.xselectionrequest.selection;
  notify.xselection.target = XA_STRING;
  notify.xselection.property = p;
  notify.xselection.time =
    ev.xselectionrequest.time;

  int error = XSendEvent(draw->screen->display,
			 ev.xselectionrequest.requestor,
			 True, 0, &notify);
 std::cerr << "send event result = " << error <<std::endl;

  XSync(draw->screen->display, False);
  // I think we're possibly leaving the property we changed hanging.
}
#endif
char* Am_Drawonable_Impl::Get_Cut_Buffer()
{
  if (offscreen)
    return (0L); // not meaningful for offscreen bitmaps
 std::cerr << "GWT Get_Cut_Buffer not implemented!" <<std::endl;
  return (0L);
}

void Am_Handle_Event_Received (gii_event event, gwt_window_t win)
{
  // Obtain the drawonable
  Am_Drawonable_Impl *drawonable = Get_Drawable_Backpointer(win);
  Am_Drawonable_Impl *draw = dynamic_cast<Am_Drawonable_Impl*>(drawonable);
  if (win != draw->gwt_window)
   std::cerr << "<> Be careful !!! window backpointer and drawonable window differ !!" <<std::endl;

 std::cerr << "Am_Handle_Event_Received: type=" << (int)event.any.type
       << " event_window=" << win
       << " draw=" << draw <<std::endl;

  if (!draw)
    {
      if (Am_Debug_Print_Input_Events)
std::cout << "<> Input ignored because no drawonable\n" <<std::flush;
      return;
    }
#if 0
  if (draw->ext_handler) {
    //TODO: ?? call the external handler IN ADDITION to local handlers
    draw->ext_handler(&event_return);
  }
#endif
  
  Am_Input_Event_Handlers *evh = draw->event_handlers;

  if (!evh)
    {
      if (Am_Debug_Print_Input_Events)
std::cout << "<> Input ignored for " << draw
	     << " because no Event_Handler" <<std::endl <<std::flush;
      // TODO: But we dispatch the event further... (necessary?)
     std::cerr << "Dispatching an event to sub windows.." <<std::endl;
      gwtChildWindowsEventSend(win, &event);
      return;
    }

  switch (event.any.type)
    {
    case evKeyPress:
     std::cerr << " Key press received" <<std::endl;
      {
std::cerr << "   Key press running" <<std::endl;
	// First, we update modifiers
	Am_Event_Modifiers = event.key.modifiers;
	// Create the input char
	Am_Input_Char ic = create_input_char_from_key (event);
	if (ic.code != 0)
	  { // then is a legal code
	    if (exit_if_stop_char(ic))
	      return;
	    // Obtain cursor position;
	    int x,y;
	    gwtGetPointerLocation(win, &x, &y);
	    Am_Current_Input_Event
	      ->Set(ic, x, y,timeval2long(event.key.time), draw);
	    draw->event_handlers->Input_Event_Notify(draw, Am_Current_Input_Event);
	  }
      }
      break;
    case evPtrButtonPress:
      {
	Am_Input_Char ic =
	  create_input_char_from_mouse (event.pbutton.button,
					Am_BUTTON_DOWN,
					timeval2long(event.pbutton.time));
	if (exit_if_stop_char(ic))
	  return;
	// Obtain cursor position;
	int x,y;
	gwtGetPointerLocation(win, &x, &y);
	Am_Current_Input_Event
	  ->Set(ic, x, y, timeval2long(event.pbutton.time), draw);
	draw->event_handlers->Input_Event_Notify(draw, Am_Current_Input_Event);
      }
      break;
    case evPtrButtonRelease:
      {
	Am_Input_Char ic
	  = create_input_char_from_mouse (event.pbutton.button,
					  Am_BUTTON_UP,
					  timeval2long(event.pbutton.time));
	if (exit_if_stop_char(ic))
	  return;
	// Obtain cursor position;
	int x,y;
	gwtGetPointerLocation(win, &x, &y);
	Am_Current_Input_Event
	  ->Set(ic, x, y, timeval2long(event.pbutton.time), draw);
	draw->event_handlers->Input_Event_Notify(draw, Am_Current_Input_Event);
      }
      break;
    case evPtrRelative:
    case evPtrAbsolute:
      {
	Am_Input_Char ic = create_input_char_from_code (Am_MOUSE_MOVED,
							Am_Event_Modifiers,
							Am_NEITHER,
							Am_SINGLE_CLICK);
	// Obtain cursor position;
	int x,y;
	gwtGetPointerLocation(win, &x, &y);
	Am_Current_Input_Event
	  ->Set(ic, x, y, timeval2long(event.pmove.time), draw);
	draw->event_handlers->Input_Event_Notify(draw, Am_Current_Input_Event);
      }
      break;
#if 0
      // TODO: Do these one day
    case EnterNotify:
      ic = create_input_char_from_code (Am_MOUSE_ENTER_WINDOW,
					event_return.xcrossing.state,
					Am_NEITHER, Am_SINGLE_CLICK);
      set_input_event (Am_Current_Input_Event, ic, event_return.xcrossing.x,
		       event_return.xcrossing.y,
		       event_return.xcrossing.time, draw);
      draw->event_handlers->Input_Event_Notify(draw, Am_Current_Input_Event);
      break;
    case LeaveNotify:
      ic = create_input_char_from_code (Am_MOUSE_LEAVE_WINDOW,
					event_return.xcrossing.state,
					Am_NEITHER, Am_SINGLE_CLICK);
      set_input_event (Am_Current_Input_Event, ic, event_return.xcrossing.x,
		       event_return.xcrossing.y,
		       event_return.xcrossing.time, draw);
      draw->event_handlers->Input_Event_Notify(draw, Am_Current_Input_Event);
      break;
#endif
    case evExpose:
      if (Am_Debug_Print_Input_Events)
std::cout << "<> Exposure Event, x=" << event.expose.x
	     << " y=" << event.expose.y
	     << " width=" << event.expose.w
	     << " height=" << event.expose.h
	     << " drawonable=" << draw <<std::endl;
      // TODO: Get the invalid region and its rectangles
      // TODO: instead of doing step by step
      gwt_region_t redrawn;
      gwtRegionCreate(&redrawn);
      gwtRegionSetRect(redrawn, event.expose.x, event.expose.y,
		       event.expose.w, event.expose.h);
      draw->event_handlers
	->Exposure_Notify(draw, event.expose.x, event.expose.y,
			  event.expose.w, event.expose.h);
      gwtValidate(draw->gwt_window, redrawn);
      gwtRegionDestroy(redrawn);
      break;
#if 0
      // TODO: Handle that one day ?
    case DestroyNotify:
      if (Am_Debug_Print_Input_Events)
std::cout << "<> DestroyNotify, drawonable=" << draw <<std::endl;
      draw->event_handlers->Destroy_Notify(draw);
	break;
#endif

#if 0
	//// BUG: Not used
	// cases UnmapNotify and MapNotify were commented out
	// EAB: there does not appear to be a way to distinguish internally-generated
	// map/unmap requests from user-generated iconify/deiconify requests
    case UnmapNotify:
      if (Am_Debug_Print_Input_Events)
std::cout << "<> UnmapNotify, drawonable=" << draw <<std::endl;
      //	draw->event_handlers->Unmap_Notify(draw);
      if (draw->expect_map_change) {
	draw->expect_map_change = false;
      } else {
	draw->iconify_notify(true);
	draw->event_handlers->Iconify_Notify(draw, true);
      }
      break;
    case MapNotify:
      if (Am_Debug_Print_Input_Events)
std::cout << "<> MapNotify, drawonable=" << draw <<std::endl;
      //	  draw->event_handlers->Map_Notify(draw, false);
      if (draw->expect_map_change) {
	draw->expect_map_change = false;
      } else {
	draw->iconify_notify(false);
	draw->event_handlers->Iconify_Notify(draw, false);
      }
      break;
    case ReparentNotify: {
      if (Am_Debug_Print_Input_Events)
std::cout << "<> ReparentNotify, drawonable=" << draw <<std::endl;
      int left, top, right, bottom, outer_left, outer_top;
      draw->Inquire_Window_Borders (left, top, right, bottom,
				    outer_left, outer_top);
      draw->event_handlers->Frame_Resize_Notify(draw, left, top,
						right, bottom);
      break;
    }
    case ConfigureNotify: {
      // Configure events may come in for windows that haven't really
      // changed size or position, and in this case the events are ignored
      // (i.e., Gem does not pass them on to the user's event-handler).
      // For example, a 'valid' configure event may be percieved and
      // dispatched that causes the user's ConfigureNotify Event Handler
      // to set the size of the window, which in turn generates a 'bogus'
      // configure event.  When the bogus event is percieved here, we
      // detect it by noticing that its values are equal to those already
      // installed in the drawonable, and throw it away.
      //
      // A Good ConfigureNotify handler should respond to the event
      // not by moving the window again, but by just setting its internal
      // state to correspond to where the window was moved to.  The window
      // manager doesn't have to put the window where you request, or make
      // it the correct size.  It tells you where it put it through a
      // ConfigureNotify event.  If you try to force the WM to put it
      // where you want by looking at the configureNotify event to find
      // out where it actually put it, you might get walking windows
      // no matter what you do.
      //
      // Note: Gem specifications dictate that the "left" and "top" of a
      // window is the left and top of its frame (not necessarily its
      // drawable area).  The "width" and "height" of a window is the
      // width and height of its drawable area.
      // Also, things drawn inside the window have their coordinates
      // relative to the drawable area's left and top.
      //
      int x, y, w, h, left, top, width, height, outer_left, outer_top,
	lb, tb, rb, bb;

      // We need to use inquire_window_borders to calculate the correct
      // x and y values, because X windows returns local coordinates
      // in the event, not global coordinates, so they're always (0,0).

      if (draw->Inquire_Window_Borders (lb, tb, rb, bb,
					outer_left, outer_top)) {
	x = outer_left;
	y = outer_top;
	w = event_return.xconfigure.width;
	h = event_return.xconfigure.height;
	draw->Get_Position (left, top);
	draw->Get_Size (width, height);
	//	 std::cout << "Configure notify: " << (void*)draw <<  " " << w << " " << h << " " << event_return.xany.window <<std::endl;

	  // Only generate a Gem configure-notify event if something changed
	if ((x != left) || (y != top) || (w != width) || (h != height)) {
	  if (Am_Debug_Print_Input_Events)
	   std::cout << "<> Configure Notify, x=" << x << " y=" << y <<
	      " width=" << width << " height=" << height <<
	      " drawonable=" << draw <<std::endl;
	  draw->reconfigure (x, y, w, h);
	  draw->event_handlers->Configure_Notify(draw, x, y, w, h);
	}
      }
      break;
    }
    case ClientMessage:
      // this is where we get window destroy messages
      // from the window manager.
      {
	// this is speed-inefficient, but it never happens, so who cares.
	Atom wm_delete_window = XInternAtom(event_return.xclient.display,
					    "WM_DELETE_WINDOW", False);
	if ((unsigned long) event_return.xclient.data.l[0] != wm_delete_window) break;
	// not a delete window client message.
	draw->event_handlers->Destroy_Notify(draw);
      }
      break;
    case SelectionClear:
      // We get this event when we're forced to release our selection.
      // We'll ignore it for now.
      break;
    case SelectionRequest:
      handle_selection_request(event_return, draw);
      break;
#endif
      // Next all the events we do not take into account
    case evKeyRelease:
      break;
      // Finally, all the events I do not expect to occur...
    case evValRelative:
    case evValAbsolute:
    case evCommand:
    case evInformation:
     std::cerr << "An_Handle_Event_Received (GWT) Unexpected event!" <<std::endl;
	// we do get these events unfortunately, so silently ignore them.
	break;
    } // end switch
}

static bool is_input_event (gii_event event)
{
  switch (event.any.type)
    {
    case evPtrButtonPress:
    case evPtrButtonRelease:
    case evKeyPress:
    case evKeyRepeat:
    case evPtrRelative:
    case evPtrAbsolute:
#if 0
      // TODO: We should have these things also one day...
    case EnterNotify:
    case LeaveNotify:
#endif
      return true;
    default:
      return false;
    }
}

/*
 *  Flush_Extra_Move_Events:  If parameter is a mouse move event, then throw
 *				away all other contiguous move events until
 * 				the last one, and leave event_return filled
 *				with the last one.  Disp_ version for multi-
 *				screen, and UseX_ for not
 */
// TODO: Check this function exact usage (currently deactivated)
#if 0
void Disp_Flush_Extra_Move_Events(gii_event& event_return)
{
  int cnt = 0;
  if (event_return.any.type & emPtrMove)
    {
      struct timeval tv = {0, 0};
      while (gwtEventPoll(GWT_Root_Window, emPtrMove, &tv) > 0)
	{
	  gwtEventRead(GWT_Root_Window, &event_return, emPtrMove);
	  cnt++;
	}
    }
  if (Am_Debug_Print_Input_Events && cnt > 0)
   std::cout << "<> Ignoring " << cnt << " move events\n" <<std::flush;
}
#endif

// TODO: Generalize the use of event masks... :-)
static gii_event_mask emAmuletInput = static_cast<gii_event_mask>(emKeyPress | emKeyRepeat | emPointer);

/*
 *  Process_Immediate_Event:  Does not wait for an event, but processes
 *                            the first event in the queue and all non-input
 *                            events after it until an input event is seen.
 *                            The function returns when it encounters an input
 *                            event (excluding the case where the first event
 *                            is an input event) or when the queue is empty.
 */
void Am_Drawonable::Process_Immediate_Event ()
{
  gii_event new_event;
  struct timeval tv = {0, 0};
  bool go_on = true;
  gwt_window_t event_window; // Or pending also in poll
  while ((gwtFamilyEventPoll(GWT_Root_Window, emAll, &tv, &event_window) > 0)
	 && go_on)
    {
      gwtFamilyNextEvent(GWT_Root_Window, &new_event, emAll, &event_window);
      //Disp_Flush_Extra_Move_Events(new_event);
      Am_Handle_Event_Received (new_event, event_window);
      if (is_input_event(new_event)) go_on = false;
      tv.tv_sec = 0; tv.tv_usec = 0;
    }
  
  // Process all remaining non-input events
  // TODO: Check if this is what is intended here ??
  tv.tv_sec = 0; tv.tv_usec = 0;
  while (gwtFamilyEventPoll(GWT_Root_Window, static_cast<gii_event_mask>(emAll & ~emAmuletInput), &tv,
			    &event_window) > 0)
    {
      gwtNextEvent(GWT_Root_Window, &new_event, static_cast<gii_event_mask>(emAll & ~emAmuletInput),
		   &event_window);
      Am_Handle_Event_Received (new_event, event_window);
      tv.tv_sec = 0; tv.tv_usec = 0;
    }
}

/*
 *  Process_Event:  waits for the next event, and processes exactly one
 *                  input event and all non-input events before and after
 *                  that input event before returning.  For example
 *                          before            after
 *                       xxxIyyyIzzz   --->   Izzz
 *                  The function returns when it encounters a second input
 *                  event or when the queue is empty.
 */
// Changes for animation interactor, 3-20-96 af1x
// Now Process_Event will time out as soon as deadline passes.
// It will process as few as 0 events, and at maximum, all of the events
// described above.  It tries hard not to drop events on the floor, by only
// exiting when it's done with event processing.

void Am_Drawonable::Process_Event (const Am_Time& deadline)
{
  gii_event new_event;
  Am_Drawonable_Impl* root_draw
    = dynamic_cast<Am_Drawonable_Impl*>(GGIGEM_Root_Drawonable);
  bool got_event = false;

  gwt_window_t event_window;
  if (deadline.Zero())
    { // Block until we have an event
      int ret = gwtFamilyNextEvent(root_draw->gwt_window, &new_event, emAll,
				   &event_window);
      got_event = (ret != 0);
    }
  else
    {
      // Figure out when we stop processing
      Am_Time now = Am_Time::Now();
      Am_Time timeout;
      struct timeval tv = {0, 0};
      if (deadline > now)
	{
	  Am_Time timeout = deadline - now;
	  Am_Time_Data* time_data = Am_Time_Data::Narrow(timeout);
	  tv = time_data->time;
	  time_data->Release();
	}
      // Only wait as long as the timeout
      int poll = gwtFamilyEventPoll(GWT_Root_Window, emAll, &tv, &event_window);
      if  (poll > 0)
	{
	  gwtFamilyNextEvent(GWT_Root_Window, &new_event, emAll,
			     &event_window);
	  got_event = true;
	}
    }
  if (!got_event)
    return;

  //Disp_Flush_Extra_Move_Events(new_event);
  Am_Handle_Event_Received (new_event, event_window);

  if (deadline.Is_Past()) return;

  // If that was not an input event, then process all the remaining
  // events until we have processed an input event.
  if (!is_input_event (new_event))
    {
      struct timeval tv = {0, 0};
      while (gwtFamilyEventPoll(GWT_Root_Window, emAll, &tv,
				&event_window) > 0)
	{
	  gwtFamilyNextEvent(GWT_Root_Window, &new_event, emAll,
			     &event_window);
	  Am_Handle_Event_Received (new_event, event_window);
	  if (is_input_event (new_event))
	    break;
	  if (deadline.Is_Past()) return;
	  tv.tv_sec = 0; tv.tv_usec = 0;
	}
    }
  // Process all remaining non-input events
  // TODO: Check if this is what is intended here ??
  struct timeval tv = {0, 0};
  while (gwtFamilyEventPoll(GWT_Root_Window, static_cast<gii_event_mask>(emAll & ~emAmuletInput), &tv,
			    &event_window) > 0)
    {
      gwtFamilyNextEvent(GWT_Root_Window, &new_event, static_cast<gii_event_mask>(emAll & ~emAmuletInput),
		   &event_window);
      Am_Handle_Event_Received (new_event, event_window);
      if (deadline.Is_Past()) return;
      tv.tv_sec = 0; tv.tv_usec = 0;
    }
}



//  Wait_For_Event: waits until the event queue is not empty.  Will return
//  immediately if queue is already not empty.  Does not process anything
//
//  Note: this is never used anywhere. 3-26-96 af1x
void Am_Drawonable::Wait_For_Event ()
{
  struct timeval tv = {0, 0};
  gwt_window_t event_window;
  gwtFamilyEventPoll(GWT_Root_Window, emAll, NULL, &event_window); // TODO: Check!
}

void Am_Drawonable::Main_Loop ()
{
  Am_Time no_timeout;
  while (Am_Main_Loop_Go)
    Process_Event (no_timeout);
}

// // // // // // // // // // // // // // // // // // // //
// Am_Drawonable member functions
// // // // // // // // // // // // // // // // // // // //

#if 0
void Am_Drawonable_Impl::Initialize_Event_Mask ()
{
    want_enter_leave = false;
    want_multi_window = false;
    want_move = false;
    current_event_mask = ( ButtonPressMask     | ButtonReleaseMask |
			   KeyPressMask        | ExposureMask      |
			   StructureNotifyMask );
    // current_event_mask can be now passed to XCreateWindow as part
    // of the XSetWindowAttributes
}

void Am_Drawonable_Impl::set_drawable_event_mask ()
{
  if (want_move) {
    unsigned int pointer_active_mask;

    current_event_mask = // *report-motion-em*
	  ExposureMask | PointerMotionMask |
	  ButtonPressMask | ButtonReleaseMask |
	  KeyPressMask | StructureNotifyMask;
    pointer_active_mask = // *report-motion-pem*
	  (unsigned int)
	  (ButtonPressMask | ButtonReleaseMask | PointerMotionMask);

    if (want_enter_leave) { // add enter leave masks
      current_event_mask  |= EnterWindowMask | LeaveWindowMask;
      pointer_active_mask |=
	    (unsigned int)(EnterWindowMask | LeaveWindowMask);
    }
    if (want_multi_window) { // add owner-grab-button
      current_event_mask |= OwnerGrabButtonMask;
	  // don't use OwnerGrabButtonMask in the p_a_m mask for grab
    }

    // this will change an active grab if one is in process because
    // changing the window's event mask will have no effect if there is
    // an active grab already.  Active grabs happen whenever
    // the mouse button is pressed down.
    XChangeActivePointerGrab (screen->display, pointer_active_mask,
                              None, CurrentTime);
  }
  else {  // don't want motion events, don't need a pointer mask
    current_event_mask = // *ignore-motion-em*
	ExposureMask | ButtonPressMask | ButtonReleaseMask |
	KeyPressMask | StructureNotifyMask;

    if (want_enter_leave)
      current_event_mask |= EnterWindowMask | LeaveWindowMask;
    if (want_multi_window) current_event_mask |= OwnerGrabButtonMask;
  }

  if (Am_Debug_Print_Input_Events)
   std::cout << "Changing Event Mask to " << current_event_mask << " for "
	 << this <<std::endl;

  // now call X to install event mask
  XSelectInput(screen->display, xlib_drawable, current_event_mask);

  // After the XSelectInput, you need to force-output because when
  // using the backgroup m-e-l process, otherwise this doesn't get noticed.
  // *no m-e-l yet* this->Flush_Output();

  // Ignore NoExpose and GraphicsExpose events (double buffering)
  XSetGraphicsExposures(screen->display, screen->gc, false);
}
#endif

void Am_Drawonable_Impl::Set_Enter_Leave ( bool want_enter_leave_events )
{
#if 0
   if (want_enter_leave_events != want_enter_leave) {
	want_enter_leave = want_enter_leave_events;
	this->set_drawable_event_mask ();
    }
#endif
}

void Am_Drawonable_Impl::Set_Want_Move ( bool want_move_events )
{
#if 0
    if (want_move != want_move_events) { // then changing
	want_move = want_move_events;
	this->set_drawable_event_mask ();
    }
#endif
}

void Am_Drawonable_Impl::Set_Multi_Window ( bool want_multi )
{
#if 0
    if (want_multi != want_multi_window) { // then changing
	want_multi_window = want_multi;
	this->set_drawable_event_mask ();
    }
#endif
}

void Am_Drawonable_Impl::Discard_Pending_Events()
{
    // * NIY *
   std::cout << "** Discarding pending input events NIY \n";
}

//Find the child-most drawonable at the current cursor position
Am_Drawonable* Am_Drawonable_Impl::Get_Drawonable_At_Cursor()
{
 std::cerr << "GWT Get_Drawonable_At_Cursor not implemented!" <<std::endl;
  return (0L);
}

void Am_Drawonable_Impl::Set_Input_Dispatch_Functions
(Am_Input_Event_Handlers* evh)
{
  event_handlers = evh;
}

void Am_Drawonable_Impl::Get_Input_Dispatch_Functions
(Am_Input_Event_Handlers*& evh)
{
  evh = event_handlers;
}
