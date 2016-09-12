/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <string.h>
#include <stdio.h>

#include <am_inc.h>

#include AM_IO__H
#include GEM__H

#include "test_utils.h"

#define TESTWINPROPS_D1_LEFT 800
#define TESTWINPROPS_D1_TOP 0
#define TESTWINPROPS_D2_LEFT 870
#define TESTWINPROPS_D2_TOP 80
#define TESTWINPROPS_D4_LEFT 900
#define TESTWINPROPS_D4_TOP 160

// Function Prototypes
void testwin1(wins *wins_ar, int ar_len);
void testwin2(wins *wins_ar, int ar_len);
void testwin3(wins *wins_ar, int ar_len);
void testwin4(wins *wins_ar, int ar_len);
void testwin5(wins *wins_ar, int ar_len);
void testwin6(wins *wins_ar, int ar_len);
void testwin7(wins *wins_ar, int ar_len);
void testwin8(wins *wins_ar, int ar_len);
void testwin9(wins *wins_ar, int ar_len);

///
///  Default window
///
void
testwin1(wins *wins_ar, int ar_len)
{
  int i;
  for (i = 0; i < ar_len; i++) {
    Am_Drawonable *the_d1 =
        (wins_ar[i].root)
            ->Create(TESTWINPROPS_D1_LEFT, TESTWINPROPS_D1_TOP, 100, 100);
    wins_ar[i].d1 = the_d1;
    the_d1->Draw_Rectangle(black, red, 10, 20, 30, 40);
    the_d1->Flush_Output();
  }
}

///
///  Borderless window
///
void
testwin2(wins *wins_ar, int ar_len)
{
  //printf("Hit RETURN to bring up green borderless window:");
  //getchar();
  int i;
  for (i = 0; i < ar_len; i++) {
    Am_Drawonable *the_d2 = wins_ar[i].root->Create(
        TESTWINPROPS_D2_LEFT, TESTWINPROPS_D2_TOP, 200, 200, "D2", "D2 Icon",
        true, false, green, false, 1, 1, 0, 0,
        // No title bar
        false);
    wins_ar[i].d2 = the_d2;
    the_d2->Draw_Rectangle(Am_No_Style, blue, 2, 10, 30, 50);
    the_d2->Draw_Rectangle(black, Am_No_Style, 2, 70, 20, 20);
    the_d2->Flush_Output();
  }
}

///
///  Iconified window
///
void
testwin3(wins *wins_ar, int ar_len)
{
  //printf("Hit RETURN to bring up iconified window:");
  //getchar();

  int i;
  for (i = 0; i < ar_len; i++) {
    Am_Drawonable *the_d3 =
        (wins_ar[i].root)
            ->Create(10, 50, 200, 200, "D3", "D3 Icon", true, true, red);
    wins_ar[i].d3 = the_d3;
  }

  printf("Hit RETURN to de-iconify red window:");
  getchar();

  for (i = 0; i < ar_len; i++) {
    (wins_ar[i].d3)->Set_Iconify(false);
  }

  printf("Hit RETURN to re-iconify red window:");
  getchar();

  for (i = 0; i < ar_len; i++) {
    (wins_ar[i].d3)->Set_Iconify(true);
  }
}

///
///  - User-specified position window
///  - Has max/min width and height
///
void
testwin4(wins *wins_ar, int ar_len)
{
  //printf("Hit RETURN to bring up blue user-positioned window:");
  //getchar();

  int i;
  for (i = 0; i < ar_len; i++) {
    wins_ar[i].d4 = (wins_ar[i].root)
                        ->Create(TESTWINPROPS_D4_LEFT, TESTWINPROPS_D4_TOP, 200,
                                 200, "D4", "D4 Icon", true, false, blue, false,
                                 // Min width and height
                                 100, 100,
                                 // Max width and height
                                 300, 300,
                                 // Want a title bar
                                 true,
                                 // Query user for position
                                 true);
  }

  //  printf("Hit RETURN to change the title of the blue window: ");
  //  getchar ();
  //  char new_title[17];
  //  new_title = "Blue Window";

  char new_title[100];
  printf("Type a new name for the blue window: ");
  scanf("%s", new_title);
  getchar(); // Eat carriage return

  for (i = 0; i < ar_len; i++) {
    (wins_ar[i].d4)->Set_Title(new_title);
  }
  for (i = 0; i < ar_len; i++) {
    strcat(new_title, " Icon");
    (wins_ar[i].d4)->Set_Icon_Title(new_title);
    (wins_ar[i].d4)->Flush_Output();
  }
}

///
///  Locally-scoped window existence
///
void
testwin5(wins *wins_ar, int ar_len)
{
  printf("Entering testwin5:\n");
  printf("  Hit RETURN to create an invisible locally-scoped window:");
  getchar();

  int i;
  Am_Drawonable *d5_array[3]; // Maximum value ar_len could be
  for (i = 0; i < ar_len; i++) {
    d5_array[i] = (wins_ar[i].root)
                      ->Create(10, 40, 200, 200, "D5", "D5 Icon",
                               // visibility
                               false);
  }

  printf("  Hit RETURN to toggle visibility of locally-scoped window: ");
  getchar();
  for (i = 0; i < ar_len; i++) {
    d5_array[i]->Set_Visible(true);
  }

  printf("  Hit RETURN to \"Destroy\" locally-scoped window:");
  getchar();

  for (i = 0; i < ar_len; i++) {
    d5_array[i]->Destroy();
  }
}

///
///  Window visibility
///
void
testwin6(wins *wins_ar, int ar_len)
{
  printf("Entering testwin6:\n");
  printf("  Hit RETURN to make d2 invisible: ");
  getchar();

  int i;
  for (i = 0; i < ar_len; i++) {
    (wins_ar[i].d2)->Set_Visible(false);
  }

  printf("  Hit RETURN to set title on d2:");
  getchar();
  for (i = 0; i < ar_len; i++) {
    (wins_ar[i].d2)->Set_Titlebar(true);
    (wins_ar[i].d2)->Flush_Output();
  }

  printf("  Hit RETURN to make d2 visible again: ");
  getchar();
  for (i = 0; i < ar_len; i++) {
    (wins_ar[i].d2)->Set_Visible(true);
  }
}

void
testwin9(wins *wins_ar, int ar_len)
{
  printf("Entering testwin9:\n");
  printf("  Hit RETURN to raise D1:");
  getchar();
  int i;
  for (i = 0; i < ar_len; i++) {
    (wins_ar[i].d1)->Raise_Window((0L));
  }

  printf("  Hit RETURN to raise D2:");
  getchar();
  for (i = 0; i < ar_len; i++) {
    (wins_ar[i].d2)->Raise_Window(wins_ar[i].d1);
  }

  printf("  Hit RETURN to raise D1:");
  getchar();
  for (i = 0; i < ar_len; i++) {
    (wins_ar[i].d1)->Raise_Window(wins_ar[i].d2);
  }

  printf("  Hit RETURN to lower D1:");
  getchar();
  for (i = 0; i < ar_len; i++) {
    (wins_ar[i].d1)->Lower_Window((0L));
  }

  printf("  Hit RETURN to lower D2:");
  getchar();
  for (i = 0; i < ar_len; i++) {
    (wins_ar[i].d2)->Lower_Window(wins_ar[i].d1);
  }

  printf("  Hit RETURN to lower D1:");
  getchar();
  for (i = 0; i < ar_len; i++) {
    (wins_ar[i].d1)->Lower_Window(wins_ar[i].d2);
  }
}

void
testwin7(wins *wins_ar, int ar_len)
{

  printf("Entering testwin7:\n");
  printf("  Hit RETURN to change background color of D2:");
  getchar();
  int i;
  for (i = 0; i < ar_len; i++) {
    (wins_ar[i].d2)->Set_Background_Color(yellow);
  }

  printf("  Hit RETURN to revert background color of D2:");
  getchar();
  for (i = 0; i < ar_len; i++) {
    (wins_ar[i].d2)->Set_Background_Color(green);
  }

  printf("  Hit RETURN to change background color of D1:");
  getchar();
  for (i = 0; i < ar_len; i++) {
    (wins_ar[i].d1)->Set_Background_Color(purple);
  }
}

///
///  Window borders
///
void
testwin8(wins *wins_ar, int ar_len)
{

  printf("Entering testwin8:\n");
  printf("  Hit RETURN to remove title from d1:");
  getchar();
  int i;
  for (i = 0; i < ar_len; i++) {
    (wins_ar[i].d1)->Set_Titlebar(false);
    (wins_ar[i].d1)->Flush_Output();
  }

  printf("  Hit RETURN to \"Destroy\" green window:");
  getchar();
  for (i = 0; i < ar_len; i++) {
    (wins_ar[i].d2)->Destroy();
  }
}

void
test_win_props(wins *wins_ar, int ar_len)
{
  testwin1(wins_ar, ar_len);
  testwin2(wins_ar, ar_len);
  testwin3(wins_ar, ar_len);
  testwin4(wins_ar, ar_len);
  testwin5(wins_ar, ar_len);
  testwin6(wins_ar, ar_len);
  testwin9(wins_ar, ar_len);
  testwin7(wins_ar, ar_len);
  testwin8(wins_ar, ar_len);
}
