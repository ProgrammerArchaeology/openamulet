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


extern "C" {
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
}

#include <iostream>

#include <am_inc.h>

#include GEM__H

#include "amulet/gemX_time.h"
#include GEMX__H

extern Screen_Manager Scrn_Mgr;

bool Am_Main_Loop_Go = true;

// An array of XEvent names used for debugging.  If you have an
// XEvent variable named event_return, do event_names[event_return.type] to
// print the name of it's XEvent type.
// Taken from Xlib Programming Manual Vol. One, p. 256-57
//
static const char *event_names[] = {
  "",
  "",
  "KeyPress",
  "KeyRelease",
  "ButtonPress",
  "ButtonRelease",
  "MotionNotify",
  "EnterNotify",
  "LeaveNotify",
  "FocusIn",
  "FocusOut",
  "KeymapNotify",
  "Expose",
  "GraphicsExpose",
  "NoExpose",
  "VisibilityNotify",
  "CreateNotify",
  "DestroyNotify",
  "UnmapNotify",
  "MapNotify",
  "MapRequest",
  "ReparentNotify",
  "ConfigureNotify",
  "ConfigureRequest",
  "GravityNotify",
  "ResizeRequest",
  "CirculateNotify",
  "CirculateRequest",
  "PropertyNotify",
  "SelectionClear",
  "SelectionRequest",
  "SelectionNotify",
  "ColormapNotify",
  "ClientMessage",
  "MappingNotify",
};

Bool is_mapnotify(Display * /* dpy */, XEvent *event_return, XPointer xlib_window) {
  switch (event_return->type) {
  case MapNotify:
    // Got MapNotify event, but if multiple windows have been created and
    // Process_Event has not yet been called, then old MapNotify events may
    // still be in the queue.  Check that we get the one for our window.
    if (event_return->xmap.window == *(Window*)xlib_window) {
      return True;
    }
    break;
  default:
    break;
  }
  return False;
}

// // // // // // // // // // // // // // // // // // // //
// Convert an X input event into a Am_Input_Char
// // // // // // // // // // // // // // // // // // // //

int Am_Double_Click_Time = 250;  // in milleseconds

Am_Click_Count Check_Multi_Click(int code, unsigned int state,
				 Am_Button_Down down,
				 Time time, Screen_Desc* screen) {

    Am_Click_Count result = Am_SINGLE_CLICK;

    if (Am_Double_Click_Time) { // else not interested in multi-click
	// if a down press, then check if double click. If up, then use current
	// down count.  If other mouse event, then ignore multi-click
	if ( down == Am_NEITHER) ; // result is OK, do nothing
	else if ( down == Am_BUTTON_UP) { // use current value

	    if (screen->click_counter >= 7) result = Am_MANY_CLICK;
	    else result = (Am_Click_Count) (screen->click_counter + 1);
		// otherwise, just use single click, so result OK
	}
	else { // is a down press
	    if ( (code  == screen->last_click_code) &&
		 (state == screen->last_click_state) &&
		 ( time <= screen->last_click_time + Am_Double_Click_Time)){
		// is multi-click
		++(screen->click_counter);

		if (screen->click_counter >= 7) result = Am_MANY_CLICK;
		else result = (Am_Click_Count) (screen->click_counter + 1);
	    }
	    else screen->click_counter = 0;

	    // in either case, set up variables for next time
	    screen->last_click_code = code;
	    screen->last_click_state = state;
	    screen->last_click_time = time;

	}
    }
    return result;
}


Am_Input_Char create_input_char_from_code (short code,
					   unsigned int state,
					   Am_Button_Down down,
					   Am_Click_Count mouse) {

    bool shft = false;
    bool ctrl = false;
    bool meta = false;

    if ((state & ShiftMask)) shft = true;

    //only use shift lock for alphabetic characters
    if ((state & LockMask) &&
	(code >= 'a') && (code <= 'z')) shft = true;

    if (state & ControlMask) ctrl = true;
    if (state & Mod1Mask) meta = true;

    Am_Input_Char ic = Am_Input_Char(code, shft, ctrl, meta, down, mouse);

    return ic;
}

#ifndef OA_VERSION
Am_IMPL_MAP(int2int, int, 0, int, 0)
#endif
static Am_Map_int2int *am_key_map = 0;

// returns character code or 0 if modifier or -1 if illegal
short Map_Sym_To_Code(KeySym sym)
{
  short c;
  if (am_key_map == 0) Am_Init_Key_Map();
  c = (short)(am_key_map->GetAt((int) sym));
  if (c) return c;

  if ( (sym >= 65505) && (sym <= 65518) ) {
    // symbol is a modifier key
    c = 0;
  }
  else { // unknown character
    c = -1;
  }
  return c;
}

void set_am_key_map (int Sym, int Code)
{
  am_key_map->SetAt(Sym, Code);
}

#if XlibSpecificationRelease < 6
// define XK_* constants that are not defined in keysymdefs.h prior to X11R6
// these values are consistent with those used in Amulet V2
#define XK_Page_Up		0xFF55
#define XK_Page_Down		0xFF56
#define XK_KP_Home		0xFF95
#define XK_KP_Left		0xFF96
#define XK_KP_Up		0xFF97
#define XK_KP_Right		0xFF98
#define XK_KP_Down		0xFF99
#define XK_KP_Prior		0xFF9A
#define XK_KP_Page_Up		0xFF9A
#define XK_KP_Next		0xFF9B
#define XK_KP_Page_Down		0xFF9B
#define XK_KP_End		0xFF9C
#define XK_KP_Begin		0xFF9D
#define XK_KP_Insert		0xFF9E
#define XK_KP_Delete		0xFF9F
#endif

void Am_Init_Key_Map()
{
  // The commented out lines have no Amulet equivalent.  Check there first!
  // Look for these XK_* constants in include/X11/keysymdefs.h

  if (am_key_map == 0) am_key_map = new Am_Map_int2int();
  set_am_key_map(XK_BackSpace, Am_BACKSPACE);
  set_am_key_map(XK_Tab, Am_TAB);
  set_am_key_map(XK_Linefeed, Am_LINEFEED);
  set_am_key_map(XK_Clear, Am_CLEAR);
  set_am_key_map(XK_Return, Am_RETURN);
  set_am_key_map(XK_Pause, Am_BREAK);
  set_am_key_map(XK_Scroll_Lock, Am_SCROLL_LOCK);
  set_am_key_map(XK_Mode_switch, Am_ALT_GRAPH);
  //  set_am_key_map(XK_Sys_Req, Am_SYS_REQ);
  set_am_key_map(XK_Escape, Am_ESC);
  set_am_key_map(XK_Delete, Am_DELETE);

  set_am_key_map (XK_Multi_key, Am_COMPOSE_CHARACTER);
  set_am_key_map(XK_Home, Am_HOME);
  set_am_key_map(XK_Left, Am_LEFT_ARROW);
  set_am_key_map(XK_Up, Am_UP_ARROW);
  set_am_key_map(XK_Right, Am_RIGHT_ARROW);
  set_am_key_map(XK_Down, Am_DOWN_ARROW);
  set_am_key_map(XK_Prior, Am_PRIOR);
  set_am_key_map(XK_Page_Up, Am_PAGE_UP);
  set_am_key_map(XK_Next, Am_NEXT);
  set_am_key_map(XK_Page_Down, Am_PAGE_DOWN);
  set_am_key_map(XK_End, Am_END);
  //  set_am_key_map(XK_Begin, Am_BEGIN);
  set_am_key_map(XK_Select, Am_SELECT);
  set_am_key_map(XK_Print, Am_PRINT_KEY);
  //  set_am_key_map(XK_Execute, Am_EXECUTE);
  set_am_key_map(XK_Insert, Am_INSERT);
  set_am_key_map(XK_Undo, Am_UNDO_KEY);
  set_am_key_map(XK_Redo, Am_REDO_KEY);
  set_am_key_map(XK_Menu, Am_MENU);
  set_am_key_map(XK_Find, Am_FIND);
  set_am_key_map(XK_Cancel, Am_CANCEL);
  set_am_key_map(XK_Help, Am_HELP);
  set_am_key_map(XK_Break, Am_BREAK);
  //  set_am_key_map(XK_Mode_switch, );
  //  set_am_key_map(XK_script_switch, );
  set_am_key_map(XK_Num_Lock, Am_NUMLOCK);

  set_am_key_map(XK_KP_Space, Am_SPACE);
  set_am_key_map(XK_KP_Tab, Am_TAB);
  set_am_key_map(XK_KP_Enter, Am_ENTER);
  set_am_key_map(XK_KP_F1, Am_PF1);
  set_am_key_map(XK_KP_F2, Am_PF2);
  set_am_key_map(XK_KP_F3, Am_PF3);
  set_am_key_map(XK_KP_F4, Am_PF4);
  set_am_key_map(XK_KP_Begin, Am_R11);
  set_am_key_map(XK_KP_Home, Am_HOME);
  set_am_key_map(XK_KP_Left, Am_LEFT_ARROW);
  set_am_key_map(XK_KP_Up, Am_UP_ARROW);
  set_am_key_map(XK_KP_Right, Am_RIGHT_ARROW);
  set_am_key_map(XK_KP_Down, Am_DOWN_ARROW);
  set_am_key_map(XK_KP_Prior, Am_PRIOR);
  set_am_key_map(XK_KP_Page_Up, Am_PAGE_UP);
  set_am_key_map(XK_KP_Next, Am_NEXT);
  set_am_key_map(XK_KP_Page_Down, Am_PAGE_DOWN);
  set_am_key_map(XK_KP_End, Am_END);
  //  set_am_key_map(XK_KP_Begin, Am_BEGIN);
  set_am_key_map(XK_KP_Insert, Am_INSERT);
  set_am_key_map(XK_KP_Delete, Am_DELETE_CHAR);
  set_am_key_map(XK_KP_Equal, (short)'=');
  set_am_key_map(XK_KP_Multiply, (short)'*');
  set_am_key_map(XK_KP_Add, (short)'+');
  set_am_key_map(XK_KP_Separator, (short)',');
  set_am_key_map(XK_KP_Subtract, (short)'-');
  set_am_key_map(XK_KP_Decimal, (short)'.');
  set_am_key_map(XK_KP_Divide, (short)'/');

  set_am_key_map(XK_KP_0, (short)'0');
  set_am_key_map(XK_KP_1, (short)'1');
  set_am_key_map(XK_KP_2, (short)'2');
  set_am_key_map(XK_KP_3, (short)'3');
  set_am_key_map(XK_KP_4, (short)'4');
  set_am_key_map(XK_KP_5, (short)'5');
  set_am_key_map(XK_KP_6, (short)'6');
  set_am_key_map(XK_KP_7, (short)'7');
  set_am_key_map(XK_KP_8, (short)'8');
  set_am_key_map(XK_KP_9, (short)'9');

  set_am_key_map(XK_F1, Am_F1);
  set_am_key_map(XK_F2, Am_F2);
  set_am_key_map(XK_F3, Am_F3);
  set_am_key_map(XK_F4, Am_F4);
  set_am_key_map(XK_F5, Am_F5);
  set_am_key_map(XK_F6, Am_F6);
  set_am_key_map(XK_F7, Am_F7);
  set_am_key_map(XK_F8, Am_F8);
  set_am_key_map(XK_F9, Am_F9);
  set_am_key_map(XK_F10, Am_F10);

  set_am_key_map(XK_L1, Am_L1);
  set_am_key_map(XK_L2, Am_L2);
  set_am_key_map(XK_L3, Am_L3);
  set_am_key_map(XK_L4, Am_L4);
  set_am_key_map(XK_L5, Am_L5);
  set_am_key_map(XK_L6, Am_L6);
  set_am_key_map(XK_L7, Am_L7);
  set_am_key_map(XK_L8, Am_L8);
  set_am_key_map(XK_L9, Am_L9);
  set_am_key_map(XK_L10, Am_L10);

  set_am_key_map(XK_R1, Am_R1);
  set_am_key_map(XK_R2, Am_R2);
  set_am_key_map(XK_R3, Am_R3);
  set_am_key_map(XK_R4, Am_R4);
  set_am_key_map(XK_R5, Am_R5);
  set_am_key_map(XK_R6, Am_R6);
  set_am_key_map(XK_R7, Am_R7);
  set_am_key_map(XK_R8, Am_R8);
  set_am_key_map(XK_R9, Am_R9);
  set_am_key_map(XK_R10, Am_R10);
  set_am_key_map(XK_R11, Am_R11);
  set_am_key_map(XK_R12, Am_R12);
  set_am_key_map(XK_R13, Am_R13);
  set_am_key_map(XK_R14, Am_R14);
  set_am_key_map(XK_R15, Am_R15);
#ifdef XK_LATIN1 // this should always be defined, but make sure just in case
  set_am_key_map(XK_space, Am_SPACE);
  set_am_key_map(XK_exclam, (short)'!');
  set_am_key_map(XK_quotedbl, (short)'\"');
  set_am_key_map(XK_numbersign, (short)'#');
  set_am_key_map(XK_dollar, (short)'$');
  set_am_key_map(XK_percent, (short)'%');
  set_am_key_map(XK_ampersand, (short)'&');
  set_am_key_map(XK_apostrophe, (short)'\'');
  //  set_am_key_map(XK_quoteright same as apostrophe
  set_am_key_map(XK_parenleft, (short)'(');
  set_am_key_map(XK_parenright, (short)')');
  set_am_key_map(XK_asterisk, (short)'*');
  set_am_key_map(XK_plus, (short)'+');
  set_am_key_map(XK_comma, (short)',');
  set_am_key_map(XK_minus, (short)'-');
  set_am_key_map(XK_period, (short)'.');
  set_am_key_map(XK_slash, (short)'/');
  set_am_key_map(XK_0, (short)'0');
  set_am_key_map(XK_1, (short)'1');
  set_am_key_map(XK_2, (short)'2');
  set_am_key_map(XK_3, (short)'3');
  set_am_key_map(XK_4, (short)'4');
  set_am_key_map(XK_5, (short)'5');
  set_am_key_map(XK_6, (short)'6');
  set_am_key_map(XK_7, (short)'7');
  set_am_key_map(XK_8, (short)'8');
  set_am_key_map(XK_9, (short)'9');

  set_am_key_map(XK_colon, (short)':');
  set_am_key_map(XK_semicolon, (short)';');
  set_am_key_map(XK_less, (short)'<');
  set_am_key_map(XK_equal, (short)'=');
  set_am_key_map(XK_greater, (short)'>');
  set_am_key_map(XK_question, (short)'?');
  set_am_key_map(XK_at, (short)'@');
  set_am_key_map(XK_A, (short)'A');
  set_am_key_map(XK_B, (short)'B');
  set_am_key_map(XK_C, (short)'C');
  set_am_key_map(XK_D, (short)'D');
  set_am_key_map(XK_E, (short)'E');
  set_am_key_map(XK_F, (short)'F');
  set_am_key_map(XK_G, (short)'G');
  set_am_key_map(XK_H, (short)'H');
  set_am_key_map(XK_I, (short)'I');
  set_am_key_map(XK_J, (short)'J');
  set_am_key_map(XK_K, (short)'K');
  set_am_key_map(XK_L, (short)'L');
  set_am_key_map(XK_M, (short)'M');
  set_am_key_map(XK_N, (short)'N');
  set_am_key_map(XK_O, (short)'O');
  set_am_key_map(XK_P, (short)'P');
  set_am_key_map(XK_Q, (short)'Q');
  set_am_key_map(XK_R, (short)'R');
  set_am_key_map(XK_S, (short)'S');
  set_am_key_map(XK_T, (short)'T');
  set_am_key_map(XK_U, (short)'U');
  set_am_key_map(XK_V, (short)'V');
  set_am_key_map(XK_W, (short)'W');
  set_am_key_map(XK_X, (short)'X');
  set_am_key_map(XK_Y, (short)'Y');
  set_am_key_map(XK_Z, (short)'Z');

  set_am_key_map(XK_bracketleft, (short)'[');
  set_am_key_map(XK_backslash, (short)'\\');
  set_am_key_map(XK_bracketright, (short)']');
  set_am_key_map(XK_asciicircum, (short)'^');
  set_am_key_map(XK_underscore, (short)'_');
//  set_am_key_map(XK_grave, (short)'');
  set_am_key_map(XK_quoteleft, (short)'`');
  set_am_key_map(XK_a, (short)'a');
  set_am_key_map(XK_b, (short)'b');
  set_am_key_map(XK_c, (short)'c');
  set_am_key_map(XK_d, (short)'d');
  set_am_key_map(XK_e, (short)'e');
  set_am_key_map(XK_f, (short)'f');
  set_am_key_map(XK_g, (short)'g');
  set_am_key_map(XK_h, (short)'h');
  set_am_key_map(XK_i, (short)'i');
  set_am_key_map(XK_j, (short)'j');
  set_am_key_map(XK_k, (short)'k');
  set_am_key_map(XK_l, (short)'l');
  set_am_key_map(XK_m, (short)'m');
  set_am_key_map(XK_n, (short)'n');
  set_am_key_map(XK_o, (short)'o');
  set_am_key_map(XK_p, (short)'p');
  set_am_key_map(XK_q, (short)'q');
  set_am_key_map(XK_r, (short)'r');
  set_am_key_map(XK_s, (short)'s');
  set_am_key_map(XK_t, (short)'t');
  set_am_key_map(XK_u, (short)'u');
  set_am_key_map(XK_v, (short)'v');
  set_am_key_map(XK_w, (short)'w');
  set_am_key_map(XK_x, (short)'x');
  set_am_key_map(XK_y, (short)'y');
  set_am_key_map(XK_z, (short)'z');
  set_am_key_map(XK_braceleft, (short)'{');
  set_am_key_map(XK_bar, (short)'|');
  set_am_key_map(XK_braceright, (short)'}');

  set_am_key_map(XK_asciitilde, (short)'~');

  // since we are doing latin1 let's handle the top half as well
  set_am_key_map(XK_nobreakspace, 0x0a0);
  set_am_key_map(XK_exclamdown, 0x0a1);
  set_am_key_map(XK_cent, 0x0a2);
  set_am_key_map(XK_sterling, 0x0a3);
  set_am_key_map(XK_currency, 0x0a4);
  set_am_key_map(XK_yen, 0x0a5);
  set_am_key_map(XK_brokenbar, 0x0a6);
  set_am_key_map(XK_section, 0x0a7);
  set_am_key_map(XK_diaeresis, 0x0a8);
  set_am_key_map(XK_copyright, 0x0a9);
  set_am_key_map(XK_ordfeminine, 0x0aa);
  set_am_key_map(XK_guillemotleft, 0x0ab);
  set_am_key_map(XK_notsign, 0x0ac);
  set_am_key_map(XK_hyphen, 0x0ad);
  set_am_key_map(XK_registered, 0x0ae);
  set_am_key_map(XK_macron, 0x0af);
  set_am_key_map(XK_degree, 0x0b0);
  set_am_key_map(XK_plusminus, 0x0b1);
  set_am_key_map(XK_twosuperior, 0x0b2);
  set_am_key_map(XK_threesuperior, 0x0b3);
  set_am_key_map(XK_acute, 0x0b4);
  set_am_key_map(XK_mu, 0x0b5);
  set_am_key_map(XK_paragraph, 0x0b6);
  set_am_key_map(XK_periodcentered, 0x0b7);
  set_am_key_map(XK_cedilla, 0x0b8);
  set_am_key_map(XK_onesuperior, 0x0b9);
  set_am_key_map(XK_masculine, 0x0ba);
  set_am_key_map(XK_guillemotright, 0x0bb);
  set_am_key_map(XK_onequarter, 0x0bc);
  set_am_key_map(XK_onehalf, 0x0bd);
  set_am_key_map(XK_threequarters, 0x0be);
  set_am_key_map(XK_questiondown, 0x0bf);
  set_am_key_map(XK_Agrave, 0x0c0);
  set_am_key_map(XK_Aacute, 0x0c1);
  set_am_key_map(XK_Acircumflex, 0x0c2);
  set_am_key_map(XK_Atilde, 0x0c3);
  set_am_key_map(XK_Adiaeresis, 0x0c4);
  set_am_key_map(XK_Aring, 0x0c5);
  set_am_key_map(XK_AE, 0x0c6);
  set_am_key_map(XK_Ccedilla, 0x0c7);
  set_am_key_map(XK_Egrave, 0x0c8);
  set_am_key_map(XK_Eacute, 0x0c9);
  set_am_key_map(XK_Ecircumflex, 0x0ca);
  set_am_key_map(XK_Ediaeresis, 0x0cb);
  set_am_key_map(XK_Igrave, 0x0cc);
  set_am_key_map(XK_Iacute, 0x0cd);
  set_am_key_map(XK_Icircumflex, 0x0ce);
  set_am_key_map(XK_Idiaeresis, 0x0cf);
  set_am_key_map(XK_ETH, 0x0d0);
  set_am_key_map(XK_Ntilde, 0x0d1);
  set_am_key_map(XK_Ograve, 0x0d2);
  set_am_key_map(XK_Oacute, 0x0d3);
  set_am_key_map(XK_Ocircumflex, 0x0d4);
  set_am_key_map(XK_Otilde, 0x0d5);
  set_am_key_map(XK_Odiaeresis, 0x0d6);
  set_am_key_map(XK_multiply, 0x0d7);
  set_am_key_map(XK_Ooblique, 0x0d8);
  set_am_key_map(XK_Ugrave, 0x0d9);
  set_am_key_map(XK_Uacute, 0x0da);
  set_am_key_map(XK_Ucircumflex, 0x0db);
  set_am_key_map(XK_Udiaeresis, 0x0dc);
  set_am_key_map(XK_Yacute, 0x0dd);
  set_am_key_map(XK_THORN, 0x0de);
  set_am_key_map(XK_ssharp, 0x0df);
  set_am_key_map(XK_agrave, 0x0e0);
  set_am_key_map(XK_aacute, 0x0e1);
  set_am_key_map(XK_acircumflex, 0x0e2);
  set_am_key_map(XK_atilde, 0x0e3);
  set_am_key_map(XK_adiaeresis, 0x0e4);
  set_am_key_map(XK_aring, 0x0e5);
  set_am_key_map(XK_ae, 0x0e6);
  set_am_key_map(XK_ccedilla, 0x0e7);
  set_am_key_map(XK_egrave, 0x0e8);
  set_am_key_map(XK_eacute, 0x0e9);
  set_am_key_map(XK_ecircumflex, 0x0ea);
  set_am_key_map(XK_ediaeresis, 0x0eb);
  set_am_key_map(XK_igrave, 0x0ec);
  set_am_key_map(XK_iacute, 0x0ed);
  set_am_key_map(XK_icircumflex, 0x0ee);
  set_am_key_map(XK_idiaeresis, 0x0ef);
  set_am_key_map(XK_eth, 0x0f0);
  set_am_key_map(XK_ntilde, 0x0f1);
  set_am_key_map(XK_ograve, 0x0f2);
  set_am_key_map(XK_oacute, 0x0f3);
  set_am_key_map(XK_ocircumflex, 0x0f4);
  set_am_key_map(XK_otilde, 0x0f5);
  set_am_key_map(XK_odiaeresis, 0x0f6);
  set_am_key_map(XK_division, 0x0f7);
  set_am_key_map(XK_oslash, 0x0f8);
  set_am_key_map(XK_ugrave, 0x0f9);
  set_am_key_map(XK_uacute, 0x0fa);
  set_am_key_map(XK_ucircumflex, 0x0fb);
  set_am_key_map(XK_udiaeresis, 0x0fc);
  set_am_key_map(XK_yacute, 0x0fd);
  set_am_key_map(XK_thorn, 0x0fe);
#endif

  // The next section is non-ideal.
  // The best solution is to find (create?) #defines for these magic
  // numbers, like the XK_ macros above.
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

Am_Input_Char create_input_char_from_key (XKeyEvent& keyevent, Display* disp)
{
  int index;
  if (keyevent.state & ShiftMask)
    index = 1;
  else
    index = 0;

  //KeySym sym = XKeycodeToKeysym (disp, keycode, index);
  KeySym sym;
  char buffer[4];
  XLookupString(&keyevent, buffer, sizeof buffer, &sym, 0);

  if (sym == NoSymbol && index == 1) {
    // try again with unshifted index
    // This makes SHIFT-F1 etc. work on Sun
    sym = XKeycodeToKeysym(disp, keyevent.keycode, 0);
  }

  short code;

  if (sym == NoSymbol)
    code = 0;
  else
    code = Map_Sym_To_Code (sym);

  if (code == -1) {
    // try again with unshifted symbol
    // This makes SHIFT-R7 etc. work on Sun
    KeySym second_sym = XKeycodeToKeysym (disp, keyevent.keycode, 0);
    code = Map_Sym_To_Code (second_sym);
  }

  if (code > 0) {
    // only support keyboard keys going down
    return create_input_char_from_code (code, keyevent.state, Am_NEITHER,
				        Am_NOT_MOUSE);
  }
  else {
    // in case is an illegal character or modifier
    if (code < 0)
     std::cout << "** Unknown keyboard symbol " << sym << " ignored\n" <<std::flush;
    return Am_Input_Char (); // null means illegal
  }
}

Am_Input_Char create_input_char_from_mouse (unsigned int button,
					    unsigned int state,
					    Am_Button_Down down,
					    Time time, Screen_Desc* screen)
{
    int code = 0;
    if (button == Button1) code = Am_LEFT_MOUSE;
    else if (button == Button2) code = Am_MIDDLE_MOUSE;
    else if (button == Button3) code = Am_RIGHT_MOUSE;
    else {
     std::cerr << "** Unknown mouse button " << button << "." <<std::endl;
      Am_Error ();
    }

    Am_Click_Count cnt = Check_Multi_Click(code, state, down, time, screen);

    return create_input_char_from_code (code, state, down, cnt);
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
  if (ic == Am_Stop_Character) {
//   std::cerr << "Got stop event: exiting Amulet main loop." <<std::endl;
    Am_Main_Loop_Go = false;
    return true;
  }
  else return false;
}

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

Bool selection_event(Display* /* d */, XEvent *ev, char*)
{
  if ((ev->xany.type == SelectionNotify) || ev->xany.type == SelectionRequest)
    return True;
  else return False;
}

char* Am_Drawonable_Impl::Get_Cut_Buffer()
{
  if (offscreen) return (0L); // not meaningful for offscreen bitmaps

  // To get a selection properly, we need to send off a SelectionRequest event
  // (this is done with the call to XConvertSelection), and then wait for a
  // SelectionNotify event.  Since we need to finish this routine synchronously
  // instead of waiting for a SelectionNotify event, we'll just wait until
  // any one event comes in, and if it's a SelectionNotify we'll take it
  // and get the selection, otherwise we'll put it back and pretend the request
  // never happened.  This is a gross hack, but it seems to work fairly well
  // when running on my machine, everything locally.  Probably it will fail more
  // if the selection is on a different machine than the Amulet program.

  if (this == screen->root) { // then we're in a root window
   std::cerr << "** Gem warning: Get_Cut_Buffer() won't work in a root window.\n";
    return (0L);
  }
  // Make an atom to get the selection in.

  // Request the selection from the X server
  XConvertSelection(screen->display, XA_PRIMARY, XA_STRING,
		    screen->cut_buffer, xlib_drawable, CurrentTime);
  XSync(screen->display, False);
  // Wait for an event, handling selection requests.
  // if it's a selection notify, get the selection
  XEvent ev;
  while (true) {
    while (!XPending (screen->display));
    //    XNextEvent(screen->display, &ev);
    if (XCheckIfEvent(screen->display, &ev, selection_event, (0L)) == False
	//	&& XCheckIfEvent(screen->display, &ev, selection_event, (0L)) == False)
	) {
	// if we get here, we didn't get any selection notify event back.
	// should we use cut buffer0 here instead?
std::cerr << "** missing selection notify event." <<std::endl;
	//	XPutBackEvent(screen->display, &ev); // checkifevent doesn't dequeue nonmatching events
	return (0L);
      }
    if (ev.xany.type == SelectionNotify) break;
    //    if (ev.xany.type == SelectionRequest) {
    Am_Drawonable_Impl *draw =
      Get_Drawable_Backpointer(ev.xany.display,
			       ev.xany.window);
    if (draw) handle_selection_request(ev, draw);
    //    else handle_selection_request(ev, screen->root);
    //    }
  }
  // if we get here, then we have a selection
  if (ev.xselection.property == None)
    { // then there is no selection value, use cut buffer 0 instead.
     std::cerr << "** No primary selection, using cut buffer." <<std::endl;
      int n;
      char* tempstr = XFetchBytes(screen->display, &n);
      char* str = new char[n+1];
      strcpy (str, tempstr);
      XFree(tempstr);
      return str;
    }
  else { // we have a selection value
    // Get the property's value.  I got this from an email from
    // "S.Ramakrishnan" <ramakris@vtopus.cs.vt.edu> to bam@cs.cmu.edu
    char str[200];
    char *buff;
    int actual_format;
    unsigned long num_items_ret, rest;
    Atom actual_type;
    long begin = 0;
    for (*str = 0, rest = 1; rest; strcat(str, buff), begin++, XFree(buff)) {
      XGetWindowProperty(screen->display, xlib_drawable, ev.xselection.property,
			 begin, 1, True, AnyPropertyType, &actual_type,
			 &actual_format, &num_items_ret, &rest, (unsigned char**)&buff);
      if (!buff) break;
    }
    buff = new char[strlen(str) + 1];
    strcpy (buff, str);
    return buff;
  }
}

void Am_Handle_Event_Received (XEvent& event_return) {
  Am_Drawonable_Impl *draw =
    Get_Drawable_Backpointer(event_return.xany.display,
			     event_return.xany.window);
    if (!draw) {
      if (Am_Debug_Print_Input_Events)
	 std::cout << "<> Input ignored because no drawonable\n" <<std::flush;
      return;
    }
  if (draw->ext_handler) {
    //call the external handler IN ADDITION to local handlers
    draw->ext_handler(&event_return);
  }

    Am_Input_Event_Handlers *evh = draw->event_handlers;
    Am_Input_Char ic;

    if (!evh) {
	if (Am_Debug_Print_Input_Events)
	   std::cout << "<> Input ignored for " << draw <<
		" because no Event_Handler\n" <<std::flush;
	return;
    }

    switch (event_return.xany.type) {
    case KeyPress:
      //ic = create_input_char_from_key (event_return.xkey.keycode,
      //				 event_return.xkey.state,
      //				 draw->screen->display);
	ic = create_input_char_from_key (event_return.xkey,
					 draw->screen->display);
	if (ic.code != 0) { // then is a legal code
	    if (exit_if_stop_char(ic)) return;
	    set_input_event (Am_Current_Input_Event, ic, event_return.xkey.x,
			 event_return.xkey.y, event_return.xkey.time,
			 draw);
	    draw->event_handlers->Input_Event_Notify(draw,
						     Am_Current_Input_Event);
	}
	break;
    case ButtonPress:
	ic = create_input_char_from_mouse (event_return.xbutton.button,
					   event_return.xbutton.state,
					   Am_BUTTON_DOWN,
					   event_return.xbutton.time,
					   draw->screen);
	if (exit_if_stop_char(ic)) return;
	set_input_event (Am_Current_Input_Event, ic, event_return.xbutton.x,
			 event_return.xbutton.y, event_return.xbutton.time,
			 draw);
	draw->event_handlers->Input_Event_Notify(draw, Am_Current_Input_Event);
	break;
    case ButtonRelease:
	ic = create_input_char_from_mouse (event_return.xbutton.button,
					   event_return.xbutton.state,
					   Am_BUTTON_UP,
					   event_return.xbutton.time,
					   draw->screen);
	if (exit_if_stop_char(ic)) return;
	set_input_event (Am_Current_Input_Event, ic, event_return.xbutton.x,
			 event_return.xbutton.y, event_return.xbutton.time,
			 draw);
	draw->event_handlers->Input_Event_Notify(draw, Am_Current_Input_Event);
	break;
    case MotionNotify:
	ic = create_input_char_from_code (Am_MOUSE_MOVED,
					   event_return.xmotion.state,
					   Am_NEITHER, Am_SINGLE_CLICK);
	set_input_event (Am_Current_Input_Event, ic, event_return.xmotion.x,
			 event_return.xmotion.y, event_return.xmotion.time,
			 draw);
	draw->event_handlers->Input_Event_Notify(draw, Am_Current_Input_Event);
	break;
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
    case Expose:
	if (Am_Debug_Print_Input_Events)
	   std::cout << "<> Exposure Event, x=" << event_return.xexpose.x
		 << " y=" << event_return.xexpose.y <<
		" width=" << event_return.xexpose.width <<
		" height=" << event_return.xexpose.height <<
		" drawonable=" << draw <<std::endl;
	draw->event_handlers->Exposure_Notify(draw,
//					      event_return.xexpose.count,
					      event_return.xexpose.x,
					      event_return.xexpose.y,
					      event_return.xexpose.width,
					      event_return.xexpose.height);
	break;
    case DestroyNotify:
	if (Am_Debug_Print_Input_Events)
	   std::cout << "<> DestroyNotify, drawonable=" << draw <<std::endl;
	draw->event_handlers->Destroy_Notify(draw);
	break;
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
      // next all the events we don't expect to occur
    case KeyRelease:
    case FocusIn:
    case FocusOut:
    case KeymapNotify:
    case VisibilityNotify:
    case MapRequest:
    case ConfigureRequest:
    case GravityNotify:
    case ResizeRequest:
    case CirculateNotify:
    case CirculateRequest:
    case PropertyNotify:
    case ColormapNotify:
    case MappingNotify:
std::cout << "** Received event of unexpected type: " <<
	    event_names[event_return.type] <<std::endl;
    case CreateNotify:
    case GraphicsExpose:
    case NoExpose:
    case SelectionNotify: // we get this here if get_cut_buffer misses it.

	// we do get these events unfortunately, so silently ignore them.
	break;
    } // end switch
}

Bool is_input_event (XEvent& event) {
  switch (event.xany.type) {
  case ButtonPress:
  case ButtonRelease:
  case KeyPress:
  case MotionNotify:
  case EnterNotify:
  case LeaveNotify:
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
void Disp_Flush_Extra_Move_Events(XEvent& event_return) {
  int cnt = 0;
  if (event_return.xany.type == MotionNotify) {
    XEvent next_event_return;
    while (Scrn_Mgr.Pending (&next_event_return)) {
      if (next_event_return.xany.type == MotionNotify) {
	event_return = next_event_return;
	cnt ++;
      }
      else {
	Scrn_Mgr.Put_Event_Back (next_event_return);
	break;
      }
    }
  }
  if (Am_Debug_Print_Input_Events && cnt > 0)
   std::cout << "<> Multi Ignoring " << cnt << " move events\n" <<std::flush;
}

void UseX_Flush_Extra_Move_Events(XEvent& event_return) {
  int cnt = 0;
  if (event_return.xany.type == MotionNotify) {
    XEvent next_event_return;
    while (XPending (Main_Display)) {
      XNextEvent (Main_Display, &next_event_return);
      if (next_event_return.xany.type == MotionNotify) {
	event_return = next_event_return;
	cnt ++;
      }
      else {
	XPutBackEvent(Main_Display, &next_event_return);
	break;
      }
    }
  }
  if (Am_Debug_Print_Input_Events && cnt > 0)
   std::cout << "<> Ignoring " << cnt << " move events\n" <<std::flush;
}

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
  XEvent event_return;

  //////////////////////////////////////////////////////////////
  // jh6p Oct 97 : XWindows calls interfere with network hooks
  // Don't use XPending. It bypasses our network select.
  // Use Screen_Manager::Pending and Screen_Manager::Block.
  // Therefore it is like always having multiple displays, since
  // we didn't use XPending for multiple displays.
  //
  // See also: Process_Event
  /////////////////////////////////////////////////////////////

  // if (More_Than_One_Display) {
    while (Scrn_Mgr.Pending (&event_return)) {
      Disp_Flush_Extra_Move_Events(event_return);
      Am_Handle_Event_Received (event_return);
      // If that was an input event, then process all the remaining
      // non-input events (and don't process another input event).
      if (is_input_event (event_return)) {
        while (Scrn_Mgr.Pending (&event_return)) {
          if (is_input_event (event_return)) {
            Scrn_Mgr.Put_Event_Back (event_return);
            return;
          }
          else
            Am_Handle_Event_Received (event_return);
        }
	return;
      }
    }
    //} Always use our Screen_Manager::Pending, don't check for # Disp.

  /*
   * Always use our Screen_Manager::Pending.
   * Don't use XPending. It bypasses our network select.
   * Don't check for # Disp., since that was used to check
   * if we wanted to use XPending or not

  else {
    while (XPending (Main_Display)) {
      XNextEvent (Main_Display, &event_return);
      UseX_Flush_Extra_Move_Events(event_return);
      Am_Handle_Event_Received (event_return);
      // If that was an input event, then process all the remaining
      // non-input events (and don't process another input event).
      if (is_input_event (event_return)) {
        while (XPending (Main_Display)) {
	  XNextEvent (Main_Display, &event_return);
	  if (is_input_event (event_return)) {
	    XPutBackEvent(Main_Display, &event_return);
	    return;
	  }
	  else
	    Am_Handle_Event_Received (event_return);
	}
	return;
      } // if is_input_event
    } //while (XPending...)
  } // else
  *
  * End of defunct single screen XPending processing. *
  *****************************************************/

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
	XEvent event_return;

	// jh6p Oct 97 : XWindows calls interfere with network hooks
	// Don't use XPending. It bypasses our network select.
	// Use Screen_Manager::Pending and Screen_Manager::Block.
	// Therefore it is like always having multiple displays, since
	// we didn't use XPending for multiple displays.
	//
	// See also: Process_Event

	event_return.type = 0;

	// If there is no deadline then we will wait forever, if necessary, for
	// the next event. Otherwise we will only wait up till the deadline.
	if(deadline.Zero())
		Scrn_Mgr.Next_Event(&event_return, static_cast<timeval*>(0L));
	else
	{
		// Figure out when we stop processing
		Am_Time now = Am_Time::Now();
		Am_Time timeout;
		if (deadline > now)
			timeout = deadline - now;

		Am_Time_Data* time_data = Am_Time_Data::Narrow(timeout);

		// Only wait as long as the timeout
		Scrn_Mgr.Next_Event (&event_return, &time_data->time);

		time_data->Release();

		// doesn't always return appn event: could time out.
		if (!event_return.type) return;  // we timed out.
	}

	Disp_Flush_Extra_Move_Events(event_return);
	Am_Handle_Event_Received (event_return);

	if (deadline.Is_Past()) return;

	// If that was not an input event, then process all the remaining
	// non-input events until we have processed an input event.
	if (!is_input_event (event_return))
	{
		while (Scrn_Mgr.Pending (&event_return))
		{
			Am_Handle_Event_Received (event_return);

			if (is_input_event (event_return)) break;

			if (deadline.Is_Past()) return;
		}
	}

	// Process all remaining non-input events
	while (Scrn_Mgr.Pending (&event_return))
	{
		if (is_input_event (event_return))
		{
			Scrn_Mgr.Put_Event_Back (event_return);
			return;
		} else
			Am_Handle_Event_Received (event_return);

		if (deadline.Is_Past()) return;
	}
}



//  Wait_For_Event: waits until the event queue is not empty.  Will return
//  immediately if queue is already not empty.  Does not process anything
//
//  Note: this is never used anywhere. 3-26-96 af1x
void Am_Drawonable::Wait_For_Event ()
{
	Scrn_Mgr.Wait_For_Event(static_cast<timeval*>(0L));
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

void Am_Drawonable_Impl::Set_Enter_Leave ( bool want_enter_leave_events )
{
   if (want_enter_leave_events != want_enter_leave) {
	want_enter_leave = want_enter_leave_events;
	this->set_drawable_event_mask ();
    }
}

void Am_Drawonable_Impl::Set_Want_Move ( bool want_move_events )
{
    if (want_move != want_move_events) { // then changing
	want_move = want_move_events;
	this->set_drawable_event_mask ();
    }
}

void Am_Drawonable_Impl::Set_Multi_Window ( bool want_multi )
{
    if (want_multi != want_multi_window) { // then changing
	want_multi_window = want_multi;
	this->set_drawable_event_mask ();
    }
}

void Am_Drawonable_Impl::Discard_Pending_Events()
{
    // * NIY *
   std::cout << "** Discarding pending input events NIY \n";
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

//Find the child-most drawonable at the current cursor position
Am_Drawonable* Am_Drawonable_Impl::Get_Drawonable_At_Cursor() {
  Display* display = screen->display;
  Window parent, root_return, child_return;
  int root_x_return, root_y_return, win_x_return, win_y_return;
  unsigned int mask_return;
  parent = screen->root->xlib_drawable;
  Am_Drawonable_Impl *return_draw = (0L);
  Am_Drawonable_Impl *draw = (0L);
  // need to find leaf-most window that contains the cursor, keep
  // going down until fail
  while(true) {
    if(!XQueryPointer (display, parent, &root_return, &child_return,
		       &root_x_return, &root_y_return, &win_x_return,
		       &win_y_return, &mask_return))
      return (0L); //if XQueryPointer returns false, then not on right screen
    if (child_return) {
      if (child_return == parent) {
	// looping, return last good drawonable
	return return_draw;
      }
      else draw = Get_Drawable_Backpointer(display, child_return);
    }
    else return return_draw; //no child found, return last good drawonable
    if (draw) { //save in case this is the last good one
      return_draw = draw;
    }
    //have a child, loop to see if can find a child of that child
    parent = child_return;
  }
}

// This function is called during window creation, and when a window changes
// from invisible to visible.  If you do not wait for the MapNotify event
// before drawing, then your window may come up blank.
//

void wait_for_mapnotify (Display *dpy, Window *xlib_window) {
  XEvent event_return;

  while(1) {
    XPeekIfEvent(dpy, &event_return, is_mapnotify, (XPointer)xlib_window);
    // XPeekIfEvent may have returned without setting event_return (in other
    // words, is_mapnotify returned False).
    if (event_return.type == MapNotify)
      return;
  }
}
