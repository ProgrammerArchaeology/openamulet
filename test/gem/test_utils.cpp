/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file contains utility functions shared by many of the Gem test
   programs, such as testcolor and testwinprops.
*/

#if defined(SHORT_NAMES)
#include "testutil.h"
#else
#include "test_utils.h"
#endif

Am_Style red    ("red");
Am_Style green  ("green");
Am_Style blue   ("blue");
Am_Style white  ("white");
Am_Style orchid ("orchid");
Am_Style purple (0.5f, 0.3f, 0.8f);
Am_Style yellow (1.0f, 1.0f, 0.0f);
Am_Style black  (0.0f, 0.0f, 0.0f);
Am_Style black4 = Am_Style::Thick_Line (4);
Am_Style plum   ("plum",   20, Am_CAP_BUTT, Am_JOIN_MITER);
Am_Style orange ("orange", 20, Am_CAP_ROUND, Am_JOIN_ROUND);
Am_Style grey   ("grey",   20, Am_CAP_BUTT, Am_JOIN_BEVEL);
Am_Style gray_stipple = Am_Style::Halftone_Stipple(50);

Am_Style miter_ls (0.0f, 0.0f, 0.0f, 30, Am_CAP_BUTT, Am_JOIN_MITER);
Am_Style round_ls (0.0f, 0.0f, 0.0f, 30, Am_CAP_ROUND, Am_JOIN_ROUND);
Am_Style bevel_ls (0.0f, 0.0f, 0.0f, 30, Am_CAP_BUTT, Am_JOIN_BEVEL);
  
Am_Style not_last   (0.0f, 0.0f, 0.0f, 0, Am_CAP_NOT_LAST, Am_JOIN_MITER);
Am_Style butt       (0.0f, 0.0f, 0.0f, 30, Am_CAP_BUTT, Am_JOIN_ROUND);
Am_Style projecting (0.0f, 0.0f, 0.0f, 30, Am_CAP_PROJECTING, Am_JOIN_ROUND);

static char dash15_list[2] = {15, 15};
static char  dash8_list[2] = {8, 8};
static char dash20_list[2] = {20, 20};

Am_Style dash15 (0.0f, 0.0f, 0.0f, 8,   Am_CAP_BUTT, Am_JOIN_MITER,
			       Am_LINE_ON_OFF_DASH, dash15_list);
Am_Style  dash8 (0.0f, 0.0f, 0.0f, 20,   Am_CAP_BUTT, Am_JOIN_MITER,
			       Am_LINE_ON_OFF_DASH, dash8_list);
Am_Style dash20 (0.0f, 0.0f, 1.0f, 12,  Am_CAP_BUTT, Am_JOIN_MITER,
			       Am_LINE_ON_OFF_DASH, dash20_list);

Am_Font default_font (Am_FONT_FIXED, false, false, false, Am_FONT_MEDIUM);
Am_Font font1 (Am_FONT_FIXED, true, false, false, Am_FONT_MEDIUM);
Am_Font font2 (Am_FONT_FIXED, false, true, false, Am_FONT_VERY_LARGE);
Am_Font font3 (Am_FONT_FIXED, false, false, true, Am_FONT_MEDIUM);
Am_Font font_large (Am_FONT_FIXED, true, false, false, Am_FONT_LARGE);
Am_Font font_vgi ("vgi-25");

// Create wins_array and return the number of elements in the array
//
wins* create_wins_array (int argc, char **argv, int &ar_len) {
  wins *wins_array = new wins[argc ? argc : 1];
  // On the Mac, argc is 0.  On Unix and PC, it is at least 1.
  if (argc <= 1) {
    wins_array[0].root = Am_Drawonable::Get_Root_Drawonable();
    ar_len = 1;
  }
  else if (argc == 2) {
    wins_array[0].root = Am_Drawonable::Get_Root_Drawonable(argv[1]);
    ar_len = 1;
  }
  else if (argc == 3) {
    wins_array[0].root = Am_Drawonable::Get_Root_Drawonable(argv[1]);
    wins_array[1].root = Am_Drawonable::Get_Root_Drawonable(argv[2]);
    ar_len = 2;
  }
  return wins_array;
}

void draw_corners (Am_Drawonable *d,
		   int rect_left, int rect_top, int rect_width,
		   int rect_height) {
  int angle1 = 0;
  int angle2 = 360;
  int arc_width = 6;
  int half_width = arc_width / 2;
  int x1  = rect_left - half_width;
  int y1  = rect_top  - half_width;
  int x2  = (rect_left + rect_width)  - half_width;
  int y2  = (rect_top  + rect_height) - half_width;

  d->Draw_Arc (black, red, x1, y1, arc_width, arc_width, angle1, angle2);
  d->Draw_Arc (black, red, x1, y2, arc_width, arc_width, angle1, angle2);
  d->Draw_Arc (black, red, x2, y1, arc_width, arc_width, angle1, angle2);
  d->Draw_Arc (black, red, x2, y2, arc_width, arc_width, angle1, angle2);

  int xx1 = x1 + half_width;
  int xx2 = x2 + half_width;
  int yy1 = y1 + half_width;
  int yy2 = y2 + half_width;
  d->Draw_Line (red, xx1, yy1, xx2, yy1);
  d->Draw_Line (red, xx1, yy1, xx1, yy2);
  d->Draw_Line (red, xx1, yy2, xx2, yy2);
  d->Draw_Line (red, xx2, yy1, xx2, yy2);
}
