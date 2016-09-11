/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <stdlib.h>

#include <am_inc.h>

#include AM_IO__H
#include GEM__H
#if defined(SHORT_NAMES)
#include "testutil.h"
#else
#include "test_utils.h"
#endif

Am_Drawonable *root, *d1, *d2, *d3, *d4, *d5, *d6;
//#if defined(_WINDOWS)
Am_Drawonable *db;
//#endif

#define TESTGEM_D1_LEFT 800
#define TESTGEM_D2_LEFT 480
#define TESTGEM_D4_LEFT 330
#define TESTGEM_D5_LEFT 700
#define TESTGEM_D5_TOP  290
#define TESTGEM_D6_LEFT 350
#define TESTGEM_D6_TOP  400

// Function Prototypes
void init_wins ();
void test_arcs (Am_Drawonable *d);
void test_thin_line_roundtangles (Am_Drawonable *d2);
void test_roundtangles (Am_Drawonable *d);
void test_draw_fns (Am_Drawonable *d);
void test_bboxes (Am_Drawonable *d);
void test_bitblt ();
void test_fill_stipples (Am_Drawonable *d);

void init_wins () {
  root = Am_Drawonable::Get_Root_Drawonable();
  d1 = root->Create (TESTGEM_D1_LEFT, 40, 300, 300, "Testing Arcs", "Arcs");
  d2 = root->Create (TESTGEM_D2_LEFT, 40, 300, 320, "Testing Roundtangles",
		     "Roundtangles");
  d3 = root->Create (10, 10, 300, 480,
		     "Testing Line Properites",
		     "Line Props");
  d4 = root->Create (TESTGEM_D4_LEFT, 40, 130, 200, "Testing Draw Functions",
		     "Draw Fns");
  d5 = root->Create (TESTGEM_D5_LEFT, TESTGEM_D5_TOP, 300, 500, "Testing Bounding Boxes", "Bboxes");
  d6 = root->Create (TESTGEM_D6_LEFT, TESTGEM_D6_TOP, 200, 200, "Testing Fill Stipples", "Fill Stipples");
//#if defined(_WINDOWS)
	db = root -> Create_Offscreen(200, 200);
//#endif
  d2->Raise_Window((0L));
}

void test_arcs (Am_Drawonable *d)
{ 
  d->Process_Event (0UL);

  d->Draw_Arc(black,       white, 130, 160,  20,  20,   0, 360);
  d->Draw_Arc(black, Am_No_Style,  20,  20, 150,  70,   0, 180);
  d->Draw_Arc(dash15,        red,  50, 140,  70, 140,   0, 360);
  d->Draw_Arc(dash15,       blue,  60,  40,  90,  90, 300, 270);
  d->Draw_Arc(black4,      green,   0, -30,  50, 300, 240,  60);
  d->Draw_Arc(Am_No_Style,  blue,  10, 240, 280,  50, -36,  72);
  d->Draw_Arc(dash8,         red, -10,  70, 300, 200,  23,  45);
  d->Draw_Arc(dash20,     yellow, 200, 160,  80,  80,   0, 360);

  d->Draw_Arc(black, Am_No_Style,  50, 140, 70, 140);
  d->Draw_Arc(black, Am_No_Style,  60,  40, 90,  90, 300, 270);
  d->Draw_Arc(black, Am_No_Style, 200, 160, 80,  80);
  
  d->Flush_Output ();
}

void test_thin_line_roundtangles (Am_Drawonable *d2)
{
  // Large draw-radius (according to formula in Am_DRAW_RADIUS slot)
  d2->Draw_Roundtangle(black,    red, 20, 20, 250, 280, 83, 83);
  // Medium draw-radius
  d2->Draw_Roundtangle(black, yellow, 60, 60, 170, 200, 42, 42);
  // Small draw-radius
  d2->Draw_Roundtangle(black,  green, 90, 90, 110, 140, 22, 22);

  d2->Flush_Output();
}

void test_roundtangles (Am_Drawonable *d)
{ 
  static char dash1_list[2] = {15, 15};
  static char dash2_list[2] = {30, 30};
  Am_Style dash1   (0.0f, 0.0f, 1.0f, 8,   Am_CAP_BUTT, Am_JOIN_MITER,
		    Am_LINE_ON_OFF_DASH, dash1_list);
  Am_Style dash2   (1.0f, 0.0f, 1.0f, 12,   Am_CAP_BUTT, Am_JOIN_MITER,
		    Am_LINE_ON_OFF_DASH, dash2_list);
  Am_Style dash3   (0.0f, 0.0f, 1.0f, 12,   Am_CAP_BUTT, Am_JOIN_MITER,
		    Am_LINE_ON_OFF_DASH, dash2_list);
  Am_Style dash4   (1.0f, 0.0f, 0.0f, 12,   Am_CAP_BUTT, Am_JOIN_MITER,
		    Am_LINE_ON_OFF_DASH, dash2_list);

  d->Flush_Output ();

  d->Draw_Roundtangle(black, Am_No_Style,  10,  20, 150, 100, 20, 20);
  d->Draw_Roundtangle(dash2,       green,  10,  20, 150, 100, 20, 20);

  d->Draw_Roundtangle(black, Am_No_Style, 180,  30, 100, 150, 40, 40);
  d->Draw_Roundtangle(dash1,         red, 180,  30, 100, 150, 40, 40);

  d->Draw_Roundtangle(black, Am_No_Style,  40, 130,  60, 100, 30, 30);
  d->Draw_Roundtangle(dash3,      yellow,  40, 130,  60, 100, 30, 30);

  d->Draw_Roundtangle(black, Am_No_Style,  20, 250, 100,  60, 30, 30);
  d->Draw_Roundtangle(dash4,        blue,  20, 250, 100,  60, 30, 30);

  d->Draw_Roundtangle(Am_No_Style,      yellow, 110, 130,  60, 100, 30, 30);
  d->Draw_Roundtangle(      black, Am_No_Style, 110, 130,  60, 100, 30, 30);

  d->Draw_Rectangle(dash1, red, 180, 220, 100, 60);

  d->Flush_Output ();
}

void test_draw_fns (Am_Drawonable *d)
{
  d->Flush_Output ();

  // Reference rectangle (vertical)
  d->Draw_Rectangle (Am_No_Style, black, 50, 2, 20, 175);
  d->Flush_Output ();

  d->Draw_Rectangle (Am_No_Style, black, 10, 10, 100, 20);

  d->Draw_Rectangle (Am_No_Style, gray_stipple, 10, 55, 100, 20, Am_DRAW_XOR);

  d->Draw_Rectangle (black, Am_No_Style, 10, 110, 100, 20, Am_DRAW_OR);

  d->Draw_Rectangle (black, white, 10, 150, 100, 20, Am_DRAW_XOR);
  d->Flush_Output ();
}

void test_bboxes (Am_Drawonable *d)
{
  Am_Style black30 (0.0f, 0.0f, 0.0f, 30);
  Am_Style black10 (0.0f, 0.0f, 0.0f, 10);
  unsigned short angle1 =   0;
  unsigned short angle2 = 360;

  d->Draw_Rectangle (black30, red, 60, 20, 75, 75);
  draw_corners(d, 60, 20, 75, 75);

  d->Draw_Rectangle (black30, red, 60, 120, 75, 60);
  draw_corners(d, 60, 120, 75, 60);

  d->Draw_Rectangle (black30, red, 60, 220, 75, 20);
  draw_corners(d, 60, 220, 75, 20);

  d->Draw_Roundtangle (black30, red, 60, 280, 75, 75, 10, 10);
  draw_corners(d, 60, 280, 75, 75);

  d->Draw_Roundtangle (black30, red, 60, 380, 75, 50, 10, 10);
  draw_corners(d, 60, 380, 75, 50);

  d->Draw_Roundtangle (black30, red, 60, 450, 75, 20, 10, 10);
  draw_corners(d, 60, 450, 75, 20);

  //right column
  d->Draw_Arc (black30, red, 180, 20, 75, 75, angle1, angle2);
  draw_corners(d, 180, 20, 75, 75);

  d->Draw_Arc (black30, red, 180, 120, 75, 50, angle1, angle2);
  draw_corners(d, 180, 120, 75, 50);

  d->Draw_Arc (black30, red, 180, 220, 75, 20, angle1, angle2);
  draw_corners(d, 180, 220, 75, 20);

  d->Flush_Output();
}

void test_bitblt ()
{
//#if !defined(_WINDOWS)  
//  d5->Bitblt(180, 280, 75, 190, d1, 75, 60);
//  d5->Flush_Output ();
//#else

	db -> Bitblt(0, 0, 75, 190, d1, 75, 60); // bitblt in invisible
	db -> Set_Size(300, 300); // resizing invisible
	db -> Set_Size(75, 190);  // (preserves contents)
	db -> Draw_Rectangle(red, Am_No_Style, 0, 0, 75, 190); // drawing in invisible
	db -> Set_Position(330, 300);
	db -> Set_Visible(true); // showing invisible
	db -> Set_Background_Color(red);
	db -> Draw_Line(black, 0, 0, 75, 190); // drawing in visible
	db -> Draw_Line(black, 75, 0, 0, 190);
	// setting/getting cut buffer shouldn't crash, but should do nothing.
	db -> Set_Cut_Buffer("Hit RETURN to hide:"); // test clipping
	char* s = db -> Get_Cut_Buffer();
	delete [] s; // we are responsible for deleting 's'
	db -> Set_Visible(false); // hiding again (preseves contents)
	d5 -> Bitblt(180, 280, 75, 190, db, 0, 0); // bitblt from invisible
	db -> Destroy();
	d5->Flush_Output();
//#endif
}

void test_time_string() {
  std::cout << "Calling get time...\n" << std::flush;
  Am_String str = Am_Get_Time_And_Date();
  std::cout << "Returned `" << str << "'\n\n" << std::flush;
}

void test_font_get() {
  Am_Font defaultfont = Am_Font();
  Am_Font lf(Am_FONT_SERIF, true, false, true, Am_FONT_VERY_LARGE);
  Am_String font_name;
  Am_Font_Family_Flag f;
  bool is_bold;
  bool is_italic;
  bool is_underline;
  Am_Font_Size_Flag s;

  std::cout << "For default font, values are: \n" << std::flush;
  defaultfont.Get(font_name, f, is_bold, is_italic, is_underline, s);
  std::cout << "name " << font_name << " family " << (int) f << " bold "
       << is_bold << " italic " << is_italic
       << " underline " << is_underline << " size " << (int) s << std::endl << std::flush;

  std::cout << "For font serif,bold,underline,very_large, values are:\n" << std::flush;
  lf.Get(font_name, f, is_bold, is_italic, is_underline, s);
  std::cout << "name " << font_name << " family " << (int) f << " bold "
       << is_bold << " italic " << is_italic
       << " underline " << is_underline << " size " << (int) s << std::endl << std::flush;
}

// test transparent and opaque fill stipples
void test_fill_stipples (Am_Drawonable *d)
{
  Am_Style light_gray_stipple = Am_Style::Halftone_Stipple (30);
  Am_Style light_gray_opaque =  Am_Style::Halftone_Stipple (30, Am_FILL_OPAQUE_STIPPLED);

  d->Draw_Rectangle (black, red, 60, 20, 75, 75);
  d->Draw_Rectangle (Am_No_Style, light_gray_stipple, 70, 30, 55, 55);

  d->Draw_Rectangle (blue, green, 60, 120, 75, 75);
  d->Draw_Rectangle (black, light_gray_opaque, 70, 130, 55, 55);
  d->Flush_Output ();
}

int main () {
  init_wins ();
  test_arcs (d1);
  test_roundtangles (d2);
  test_line_props (d3); // in testlineprops.cc
  test_draw_fns (d4);
  test_bboxes (d5);
  test_bitblt (); // bitblts from arc window to bbox window
  test_fill_stipples (d6);
  test_time_string();
  test_font_get();
  
  ///
  ///  Exit
  ///
  printf("Hit RETURN to exit:");
  getchar();
  d1->Destroy();

  return 0;
}
