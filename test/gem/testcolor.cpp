/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <am_inc.h>

#include <amulet/gem.h>
#include "test_utils.h" // For wins class

// Draw three rectangles in all different join-styles
//
void
draw_colors(Am_Drawonable *d)
{

  // Right column
  d->Draw_Rectangle(black, red, 160, 10, 100, 55);
  d->Draw_Rectangle(plum, green, 160, 110, 100, 55);
  d->Draw_Rectangle(orange, blue, 160, 210, 100, 55);
  d->Draw_Rectangle(grey, white, 160, 310, 100, 55);
  d->Draw_Rectangle(orange, black, 160, 410, 100, 55);

  // Left column
  d->Draw_Rectangle(black, orange, 30, 10, 100, 55);
  d->Draw_Rectangle(black, orchid, 30, 110, 100, 55);
  d->Draw_Rectangle(black, purple, 30, 210, 100, 55);
  d->Draw_Rectangle(black4, grey, 30, 310, 100, 55);
  d->Draw_Rectangle(black, plum, 30, 410, 100, 55);

  d->Flush_Output();
}

void
test_colors(wins *wins_ar, int ar_len)
{
  for (int i = 0; i < ar_len; i++) {
    Am_Drawonable *the_color_win = (wins_ar[i].root)->Create(10, 50, 300, 480);
    wins_ar[i].color_win = the_color_win;
    draw_colors(the_color_win);
    the_color_win->Flush_Output();
  }
}
