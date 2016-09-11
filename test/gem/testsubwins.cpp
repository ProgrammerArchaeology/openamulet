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

Am_Drawonable *root, *d1, *d2, *d3, *d4;

Am_Style black (0.0, 0.0, 0.0);
Am_Style green (0.0, 1.0, 0.0);
Am_Font font1 (Am_FONT_FIXED, true, false, false, Am_FONT_VERY_LARGE);
Am_Font font2 (Am_FONT_FIXED, false, false, false, Am_FONT_LARGE);
Am_Font font3;

void init_windows() {
  root = Am_Drawonable::Get_Root_Drawonable();
  d1 = root->Create(850, 180, 220, 100);
  d2 = d1->Create(-15, 22, 175, 100, "D2", "D2 Icon", true, false,
		  Am_No_Style, false, 1, 1, 0, 0,
		  // want a title-bar when placed at top-level
		  true);
  d3 = d2->Create(30, 18, 150, 100);
}

void draw_strings() {
  
  static char *string1 = "Top-level window";
  static int string1_len = 16;
  static char *string2 = "Child of top-level";
  static int string2_len = 18;
  static char *string3 = "Child of child window";
  static int string3_len = 21;

  d1->Draw_Text(black, string1, string1_len, font1, 5, 7);
  d2->Draw_Text(black, string2, string2_len, font2, 5, 7);
  d3->Draw_Text(black, string3, string3_len, font3, 5, 7);
  d1->Flush_Output ();  
}  

void test_reparent1() {

  printf("Hit RETURN to make D3 a sibling of D2:");
  getchar();
  d3->Reparent(d1);

  printf("Hit RETURN to put D3 back where it was:");
  getchar();
  d3->Reparent(d2);

  printf("Hit RETURN to make D2 a top-level window:");
  getchar();
  d2->Reparent(root);

  printf("Hit RETURN to put D2 back where it was:");
  getchar();
  d2->Reparent(d1);
}

int main ()
{
  init_windows();
  draw_strings();

  test_reparent1();
  
  ///
  ///  Exit
  ///
  printf("Hit RETURN to exit:");
  getchar();

  return 0;
}
