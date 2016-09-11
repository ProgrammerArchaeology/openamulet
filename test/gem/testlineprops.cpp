/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <am_inc.h>

#include GEM__H
#include "test_utils.h"

Am_Style style_array[4];

// Utility function for reusing the style_array for iteration when
// creating lines with different line styles
//
void set_style_array (Am_Style arg0 = Am_No_Style,
		      Am_Style arg1 = Am_No_Style,
		      Am_Style arg2 = Am_No_Style,
		      Am_Style arg3 = Am_No_Style) {
  style_array[0] = arg0;
  style_array[1] = arg1;
  style_array[2] = arg2;
  style_array[3] = arg3;
}

// Draw three rectangles in all different join-styles
//
void test_join_styles (Am_Drawonable *d) {
  d->Draw_Rectangle (miter_ls, white, 160,  20, 100, 75);
  draw_corners(d, 160, 20, 100, 75);
  d->Draw_Rectangle (round_ls, white, 160, 120, 100, 75);
  draw_corners(d, 160, 120, 100, 75);
  d->Draw_Rectangle (bevel_ls, white, 160, 220, 100, 75);
  draw_corners(d, 160, 220, 100, 75);
}

// Draw four lines in all different cap-styles
//
void test_cap_styles (Am_Drawonable *d) {
  // A line whose last endpoint is not drawn by virtue of the not_last flag
  d->Draw_Arc(black, white, 16, 6, 8, 8, 0, 360);
  d->Draw_Arc(black, white, 96, 6, 8, 8, 0, 360);
  d->Draw_Line(not_last, 20, 10, 100, 10);

  // A line for comparing the not_last line against (use xmag)
  d->Draw_Arc(black, white, 16, 21, 8, 8, 0, 360);
  d->Draw_Arc(black, white, 96, 21, 8, 8, 0, 360);
  d->Draw_Line(black, 20, 25, 100, 25);

  // Draw three thick lines, each with different cap styles
  int n, y;
  set_style_array (butt, round_ls, projecting);
  for (n=0; n<=2; n++) {
    y = 50+(40*n);
    d->Draw_Line(style_array[n], 20, y, 100, y);
    d->Draw_Line(white, 20, y, 100, y);
    d->Draw_Arc(black, white, 16, y-4, 8, 8, 0, 360);
    d->Draw_Arc(black, white, 96, y-4, 8, 8, 0, 360);
  }
}

// Draw four lines with different dash patterns
//
void test_dashes (Am_Drawonable *d) {
  int n, x;

  const int  dash2_list_length = 6;
  static char dash2_list[dash2_list_length]={3, 3, 3, 3, 9, 3};
  const int  dash3_list_length = 2;
  static char dash3_list[dash3_list_length]={16,16};
  Am_Style no_dash (0.0f, 0.0f, 0.0f, 4, Am_CAP_BUTT, Am_JOIN_MITER,
		    Am_LINE_SOLID);
  Am_Style dash1   (0.0f, 0.0f, 0.0f, 4, Am_CAP_BUTT, Am_JOIN_MITER,
		    Am_LINE_ON_OFF_DASH);
  Am_Style dash2   (0.0f, 0.0f, 0.0f, 4, Am_CAP_BUTT, Am_JOIN_MITER,
		    Am_LINE_ON_OFF_DASH, dash2_list,
		    dash2_list_length);
  Am_Style dash3   (0.0f, 0.0f, 0.0f, 4, Am_CAP_BUTT, Am_JOIN_MITER,
		    Am_LINE_ON_OFF_DASH, dash3_list,
		    dash3_list_length);
  
  set_style_array (no_dash, dash1, dash2, dash3);
  for (n=0; n<=3; n++) {
    x = 20+(20*n);
    d->Draw_Line(style_array[n], x, 160, x, 300);
  }
}

// Draw a rectangle and an ellipse, both with dashed stippled lines
//
void test_stipples(Am_Drawonable *d) {

  static char dash5_list[4]={20, 10, 5, 10};
  static char dash6_list[2]={5, 20};
  
  Am_Style dark_gray_line (0.0f, 0.0f, 0.0f, 3, Am_CAP_BUTT,
			   Am_JOIN_MITER,
			   Am_LINE_ON_OFF_DASH,
			   dash5_list, 4, Am_FILL_STIPPLED,
			   Am_FILL_POLY_EVEN_ODD,
			   Am_Image_Array(75));
  Am_Style gray_line (0.0f, 0.0f, 0.0f, 5, Am_CAP_BUTT,
		      Am_JOIN_MITER,
		      Am_LINE_ON_OFF_DASH,
		      dash6_list, 2, Am_FILL_STIPPLED,
		      Am_FILL_POLY_EVEN_ODD,
		      Am_Image_Array(50));
  d->Draw_Rectangle(dark_gray_line, Am_No_Style, 10, 320, 100, 50);
  d->Draw_Arc(gray_line, Am_No_Style, 150, 320, 100, 50, 0, 360);
}

void test_polylines(Am_Drawonable *d) {

  const int  dash4_list_length = 2;
  static char dash4_list[dash4_list_length]={60,30};
  Am_Style dash4   (0.0f, 0.0f, 0.0f, 20, Am_CAP_ROUND,
		    Am_JOIN_MITER, Am_LINE_ON_OFF_DASH, dash4_list,
		    dash4_list_length,
		    Am_FILL_STIPPLED, Am_FILL_POLY_EVEN_ODD,
		    Am_Image_Array(50));

  // Since zig_zag_array is static, it persists across multiple invocations
  // of the function.  It will not be reinitialized when the function is
  // invoked a second time.
  static int zig_zag_array[14] = {60, 450, 20, 425, 75, 400, 150, 450,
				  150, 400, 200, 400, 280, 450};
  Am_Point_List zig_zag_pl (zig_zag_array, 14);
  d->Draw_Lines(dash4, Am_No_Style, zig_zag_pl);
  d->Draw_Lines(black, Am_No_Style, zig_zag_pl);
}

  

void test_line_props (Am_Drawonable *d)
{ 
  d->Flush_Output ();

  test_join_styles(d);
  test_cap_styles(d);
  test_dashes(d);
  test_stipples(d);
  test_polylines(d);

  d->Flush_Output ();
}
