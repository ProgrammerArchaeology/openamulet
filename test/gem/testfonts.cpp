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

#include "test_utils.h"  // For wins class

#define FONT_WIN_TOP 50

// These global variables provide an interface between drawing strings
// and calculating their dimensions.
const char *test_string;
int test_str_len;
int test_string_width, test_string_ascent, test_string_descent,
  test_string_left_bearing, test_string_right_bearing, test_string_height;
Am_Font test_font;

// Function Prototypes
void update_font_numbers(Am_Drawonable *font_win, bool print_numbers);
void test_font_variety(wins *wins_ar, int ar_len, Am_Style& bs, bool invert);
void test_font_dimensions(wins *wins_ar, int ar_len);
void test_char_dimensions(wins *wins_ar, int ar_len);
void test_font_props(wins *wins_ar, int ar_len);
void test_bogus_name(wins *wins_ar, int ar_len);
void test_filled_and_inverted_text(wins *wins_ar, int ar_len, Am_Style& bs);
void test_fonts (wins *wins_ar, int ar_len );

// This function sets the global font dimension variables according to the
// current values you have installed in test_string, test_str_len, and
// test_font.
// 
void update_font_numbers(Am_Drawonable *font_win, bool print_numbers = false) {
  font_win->Get_String_Extents(test_font, test_string, test_str_len,
			 test_string_width, test_string_ascent,
			 test_string_descent, test_string_left_bearing,
			 test_string_right_bearing);
  test_string_height = test_string_ascent + test_string_descent;
  if (print_numbers) {
    printf("test_string_width = %d,  test_string_ascent = %d,  test_string_descent = %d\n",
	   test_string_width, test_string_ascent, test_string_descent);
    printf("left_bearing = %d,  right_bearing = %d\n",
	   test_string_left_bearing, test_string_right_bearing);
  }
}

void test_font_variety(wins *wins_ar, int ar_len, Am_Style& bs, bool invert) {
  // Since family_array and size_array are static, they persists across
  // multiple invocations of the enclosing function.  They will not be
  // reinitialized when the function is invoked a second time.
  static Am_Font_Family_Flag family_array[3] = { Am_FONT_FIXED, Am_FONT_SERIF,
						 Am_FONT_SANS_SERIF };
  static Am_Font_Size_Flag   size_array[4]   = { Am_FONT_SMALL, Am_FONT_MEDIUM,
						 Am_FONT_LARGE,
						 Am_FONT_VERY_LARGE };
  int top = 0;
  test_string = "Egypt";
  test_str_len = 5;
  for(int fam=0;fam<3;fam++) {
    for(int size=1;size<4;size+=2) {
      for (int it=0;it<2;it++) {
	for (int under=0;under<2;under++) {
	  test_font = Am_Font (family_array[fam], false, it,
			       under, size_array[size]);
	  
	  for (int i=0; i<ar_len; i++) {
	    Am_Drawonable *font_win = wins_ar[i].font_win;
	    update_font_numbers(font_win);
	    
	    font_win->Draw_Text(Am_Black, test_string, test_str_len, test_font,
				10, top, Am_DRAW_COPY, bs, invert);
	    font_win->Draw_Rectangle(black, Am_No_Style, 10, top, test_string_width,
			     test_string_height);
	  }
	  top = top + test_string_height + 4;
	}
      }
    }
  }
  for (int i=0; i<ar_len; i++) {
    (wins_ar[i].font_win)->Flush_Output();
  }
}

void test_font_dimensions(wins *wins_ar, int ar_len) {

  test_string = "XpgyI";
  test_str_len = 5;
  test_font = font_vgi;
  int i;
  for (i=0; i<ar_len; i++) {
    Am_Drawonable *font_win = wins_ar[i].font_win;
    update_font_numbers(font_win);
    
    font_win->Draw_Text (black, test_string, test_str_len,
			 test_font, 80, 260); 
    font_win->Draw_Rectangle(black, Am_No_Style, 80, 260, test_string_width,
			     test_string_height);
    font_win->Flush_Output ();
  }

  test_string = ".,.,.";

  for (i=0; i<ar_len; i++) {
    Am_Drawonable *font_win = wins_ar[i].font_win;
    update_font_numbers(font_win);
    
    font_win->Draw_Text (black, test_string, test_str_len,
			 test_font, 120, 260);
    font_win->Draw_Rectangle(black, Am_No_Style, 120, 260, test_string_width,
			     test_string_height);
    font_win->Flush_Output ();
  }
}

void test_char_dimensions(wins *wins_ar, int ar_len) {
  // Since family_array and size_array are static, they persists across
  // multiple invocations of the enclosing function.  They will not be
  // reinitialized when the function is invoked a second time.
  static Am_Font_Family_Flag family_array[3] = { Am_FONT_FIXED, Am_FONT_SERIF,
						 Am_FONT_SANS_SERIF };
  static Am_Font_Size_Flag   size_array[3]   = { Am_FONT_SMALL, Am_FONT_MEDIUM,
						 Am_FONT_VERY_LARGE };
  int n;
  int top = 0;
  static const char *char_list = "AbgXij8.-@`|";
  for(int ital=0;ital<2;ital++) {
    n = 0;
    for(int fam=0;fam<3;fam++) {
      for(int size=0;size<3;size++) {
	test_font = Am_Font (family_array[fam], false, ital, false,
			     size_array[size]);
	char the_char = char_list[n++];
	test_string = &the_char;
	test_str_len = 1;
	for (int i=0; i<ar_len; i++) {
	  Am_Drawonable *font_win = wins_ar[i].font_win;
	  update_font_numbers(font_win); // To get test_string_height
	  int char_width = font_win->Get_Char_Width(test_font, the_char);
	  font_win->Draw_Text(black, &the_char, 1, test_font, 175, top);
	  font_win->Draw_Rectangle(black, Am_No_Style, 175, top, char_width,
			     test_string_height);
	}
	top = top + test_string_height + 4;
      }
    }
  }
  for (int i=0; i<ar_len; i++) {
    (wins_ar[i].font_win)->Flush_Output();
  }
}

void test_font_props(wins *wins_ar, int ar_len) {
  static Am_Font_Family_Flag family_array[3] = { Am_FONT_FIXED, Am_FONT_SERIF,
						 Am_FONT_SANS_SERIF };
  static Am_Font_Size_Flag   size_array[4]   = { Am_FONT_SMALL, Am_FONT_MEDIUM,
						 Am_FONT_LARGE,
						 Am_FONT_VERY_LARGE };
  test_string = "Egypt";
  test_str_len = 5;
  for(int bold=0;bold<2;bold++) {
    for(int ital=0;ital<2;ital++) {
      for(int fam=0;fam<3;fam++) {
	for(int size=0;size<4;size++) {
	  int max_char_width, min_char_width,
	    max_char_ascent, max_char_descent;
	  test_font = Am_Font (family_array[fam], bold, ital, false,
				     size_array[size]);
	  for (int i=0; i<ar_len; i++) {
	    Am_Drawonable *font_win = wins_ar[i].font_win;
	    font_win->Get_Font_Properties(test_font,
					  max_char_width, min_char_width,
					  max_char_ascent, max_char_descent);
	    update_font_numbers(font_win);
	  if ((test_string_ascent  > max_char_ascent)  ||
		(test_string_descent > max_char_descent)) {
		//(test_string_width   < (test_str_len * min_char_width)) || //under Windows, we get aver. char. width
		                                                             //as min. char. width
		//(test_string_width   > (test_str_len * max_char_width)))
		  printf("test_font_props() failed:  bold=%d, ital=%d, fam=%d, size=%d\n",
		      bold, ital, fam, size);
	      exit(-1);
	    }
	  }
	}
      }
    }
  }
//  fprintf(stderr, "test_font_props() passed all standard font tests.\n");
}

void test_bogus_name(wins *wins_ar, int ar_len) {
  Am_Font bogus_font ("*-Bogus-Font-*");
  for (int i=0; i<ar_len; i++) {
    Am_Drawonable *font_win = wins_ar[i].font_win;
    font_win->Draw_Text(black, "This is Bogus", 13, bogus_font, 80, 20);
    font_win->Flush_Output();
  }
}

void test_filled_and_inverted_text(wins *wins_ar, int ar_len, Am_Style& bs) {
  for (int i=0; i<ar_len; i++) {
    Am_Drawonable *font_win = wins_ar[i].font_win;
    font_win->Draw_Text(black, "FILLED", 6, font_large, 90, 400,
			Am_DRAW_COPY, bs, false);
    font_win->Draw_Text(black, "INVERTED", 8, font_large, 90, 430,
			Am_DRAW_COPY, bs, true);
    font_win->Flush_Output();
  }
}

void test_fonts (wins *wins_ar, int ar_len )

{ 
    
  for (int i=0; i<ar_len; i++) {
    Am_Drawonable *the_font_win =
      (wins_ar[i].root)->Create (400, FONT_WIN_TOP, 200, 575);
    wins_ar[i].font_win = the_font_win;
    the_font_win->Flush_Output ();
  }

  // base styles of all screens on the style of the main screen.
  
  Am_Style bs = (bool)wins_ar[0].root->Is_Color() ? Am_Red
    : Am_Style (0.0f, 0.0f, 0.0f, 2, Am_CAP_BUTT, Am_JOIN_MITER,
		Am_LINE_SOLID, Am_DEFAULT_DASH_LIST,
		Am_DEFAULT_DASH_LIST_LENGTH,
		Am_FILL_STIPPLED, Am_FILL_POLY_EVEN_ODD,
		Am_Image_Array(50));

  test_font_variety(wins_ar, ar_len, Am_No_Style, false); // normal text
  //test_font_variety(wins_ar, ar_len, bs, false); // red background rect
  //test_font_variety(wins_ar, ar_len, bs, true); // inverted
  
  test_font_dimensions(wins_ar, ar_len);
  test_char_dimensions(wins_ar, ar_len);
  test_font_props(wins_ar, ar_len);
  test_bogus_name(wins_ar, ar_len);
  test_filled_and_inverted_text(wins_ar, ar_len, bs);
}

