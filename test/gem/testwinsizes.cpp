/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <stdio.h>
#include <am_inc.h>

#include GEM__H

#define TESTWINSIZES_D1_LEFT 850
#define TESTWINSIZES_D4_LEFT 900

Am_Style red (1.0, 0.0, 0.0);
Am_Style green (0.0, 1.0, 0.0);
Am_Style blue (0.0, 0.0, 1.0);
Am_Style white (1.0, 1.0, 1.0);
Am_Style black (0.0, 0.0, 0.0);

Am_Drawonable *root, *d1, *d2, *d3, *d4;

// Function Prototypes
void print_win_info(Am_Drawonable *d, char *d_string);
void testwin1 ();
void change_min_max(Am_Drawonable *d);
void change_pos_dim(Am_Drawonable *d);
void testwin4 ();

void print_win_info(Am_Drawonable * /*d*/, char * /*d_string*/) {
  /*
  int the_left, the_top, the_width, the_height;
  const char *the_title, *the_icon_title;
  bool the_visible, the_iconified;
  Am_Style the_background;
  bool the_save_under;
  int min_w, min_h, max_w, max_h;
  bool the_no_title_bar, the_clip_by_children;
  int the_depth;

  d->Get_Values(the_left, the_top, the_width, the_height,
		the_title, the_icon_title, the_visible, the_iconified,
		the_background, the_save_under,
		min_w, min_h, max_w, max_h,
		the_no_title_bar, the_clip_by_children, the_depth);

  printf("Am_Drawonable %s:\n", d_string);
  printf(" left = %d,  top = %d,  width = %d,  height = %d,\n",
	 the_left, the_top, the_width, the_height);
  printf(" title = \"%s\",  icon_name = \"%s\",  visible = %d,  iconified = %d,\n",
	 the_title, the_icon_title, the_visible, the_iconified);
  printf(" background = %d, save_under = %d,\n",
	 (Am_Wrapper*)the_background, the_save_under);
  printf(" min_w = %d,  min_h = %d,  max_w = %d,  max_h = %d,\n",
	 min_w, min_h, max_w, max_h);
  printf(" no_title_bar = %d,  depth = %d\n\n",
	 the_no_title_bar, the_depth);
  */
}

void testwin1 () {
  
  d1 = root->Create(TESTWINSIZES_D1_LEFT, 100, 200, 200, "D1", "D1 Icon", true, false, red,
		    false,
		    // Min/max width and height
		    1, 1, 0, 0, true,
		    // Query user for position
		    true, false);
  print_win_info(d1, "D1");
}

void change_min_max(Am_Drawonable *d) {

  unsigned int min_width, min_height, max_width, max_height;
  min_width  = 999;

  printf("Changing MIN/MAX of blue window.  Enter four values\n");
  printf("  seperated by spaces (or CTRL-D RETURN to exit): ");

  scanf("%d %d %d %d", &min_width, &min_height, &max_width, &max_height);
  getchar();  // Eat carriage return

  if (min_width == 999) {
    printf("\n");
    return;
  }
  else {
    d->Set_Min_Size(min_width, min_height);
    d->Set_Max_Size(max_width, max_height);
    d->Flush_Output();
    print_win_info(d, "blue window");
    change_min_max(d);
  }
}

void change_pos_dim(Am_Drawonable *d) {

  unsigned int left, top, width, height;
  left  = 999;

  printf("Changing POSITION/SIZE of blue window.  Enter four values\n");
  printf("  seperated by spaces (or CTRL-D RETURN to exit): ");

  scanf("%d %d %d %d", &left, &top, &width, &height);
  getchar();  // Eat carriage return

  if (left == 999) {
    printf("\n");
    return;
  }
  else {
    d->Set_Position(left, top);
    d->Set_Size(width, height);       // Does Flush_Output()
    print_win_info(d, "blue window");
    change_pos_dim(d);
  }
}

  
///
///  Has max/min width and height
///
void testwin4 () {
  //printf("Hit RETURN to bring up blue window:");
  //getchar();

  d4 = root->Create(TESTWINSIZES_D4_LEFT, 160, 200, 200, "D4", "D4 Icon",
			true, false, blue, false,
		    // Min width and height
		    100, 100,
		    // Max width and height
		    300, 300);
  d4->Flush_Output ();

  print_win_info(d4, "D4");

//  change_min_max(d4);
  change_pos_dim(d4);
}

int main ()
{ 
  root = Am_Drawonable::Get_Root_Drawonable();
  print_win_info(root, "root");

  testwin1();
  testwin4();

  ///
  ///  Exit
  ///
  printf("Hit RETURN to exit:");
  getchar();

  return 0;
}
