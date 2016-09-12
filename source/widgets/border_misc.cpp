//
// ------------------------------------------------------------------------
// 		The OpenAmulet User Interface Development Environment
// ------------------------------------------------------------------------
// This code is based on the Amulet project of Carnegie Mellon University,
// and has been placed in the public domain.  If you are using this code
// or any part of OpenAmulet, please contact amulet@cs.cmu.edu to be put
// on the mailing list or have a look at:
// http://www.openip.org/
//
#include <am_inc.h>

#include AM_IO__H
#include <amulet/impl/types_logging.h>
#include <amulet/impl/am_style.h>
#include <amulet/impl/am_drawonable.h>
#include <amulet/impl/computed_colors_rec.h>
#include <amulet/impl/opal_objects.h>
#include <amulet/impl/widget_draw.h>

/*******************************************************************************
 * Simple bordered rectangle
 ******************************************************************************/

/*******************************************************************************
 * Am_Draw_Motif_Box
 *   Draws the "raised" or "depressed" box that is ubiquitous in Motif.
 */

void
Am_Draw_Motif_Box(int left, int top, int width, int height, bool depressed,
                  const Computed_Colors_Record &rec, Am_Drawonable *draw)
{
  Am_Style top_fill;
  Am_Style bot_fill;
  Am_Style inside_fill;
  if (depressed) {
    top_fill = rec.data->shadow_style;
    bot_fill = rec.data->highlight_style;
    inside_fill = rec.data->background_style;
  } else {
    top_fill = rec.data->highlight_style;
    bot_fill = rec.data->shadow_style;
    inside_fill = rec.data->foreground_style;
  }
  //top edges
  draw->Draw_Rectangle(Am_No_Style, top_fill, left, top, width, height);
  //bottom edges
  draw->Draw_2_Lines(bot_fill, Am_No_Style, left + width - 1, top + 1,
                     left + width - 1, top + height - 1, left + 1,
                     top + height - 1);
  draw->Draw_2_Lines(bot_fill, Am_No_Style, left + width - 2, top + 2,
                     left + width - 2, top + height - 2, left + 2,
                     top + height - 2);

  //inside of box
  draw->Draw_Rectangle(Am_No_Style, inside_fill, left + 2, top + 2, width - 4,
                       height - 4);
}

/*******************************************************************************
 * Am_Draw_Windows_Box
 *   Draws the "raised" or "depressed" box that is ubiquitous in Windows.
 */

void
Am_Draw_Windows_Box(int x, int y, int w, int h, bool depressed,
                    const Computed_Colors_Record &rec, Am_Drawonable *draw)
{
  Am_Style top_in_fill;
  Am_Style top_out_fill;
  Am_Style bot_in_fill;
  Am_Style bot_out_fill;
  Am_Style inside_fill;

  if (!depressed) {
    top_in_fill = /*Am_Motif_Gray;*/ rec.data->highlight2_style;
    top_out_fill = /*Am_White;*/ rec.data->highlight_style;
    bot_in_fill = /*Am_Dark_Gray;*/ rec.data->/*background_style*/ shadow_style;
    bot_out_fill = Am_Black;
    inside_fill = /*Am_Motif_Gray;*/ rec.data->foreground_style;
  } else {
    bot_in_fill = /*Am_White;*/ rec.data->highlight2_style;
    bot_out_fill = /*Am_Motif_Gray;*/ rec.data->highlight_style;
    top_in_fill = /*Am_Dark_Gray;*/ rec.data->/*background_style*/ shadow_style;
    top_out_fill = Am_Black;
    inside_fill = /*Am_Motif_Gray;*/ rec.data->background_style;
  }

  // top-left outer, though strictly speaking Windows uses white for this
  draw->Draw_2_Lines(top_out_fill, Am_No_Style, x, y + h - 1, x, y, x + w - 1,
                     y);

  // bottom-right outer
  draw->Draw_2_Lines(bot_out_fill, Am_No_Style, x, y + h - 1, x + w - 1,
                     y + h - 1, x + w - 1, y - 1);

  // top-left inner and inside
  draw->Draw_Rectangle(top_in_fill, inside_fill, x + 1, y + 1, w - 2, h - 2);

  // bottom-right inner
  draw->Draw_2_Lines(bot_in_fill, Am_No_Style, x + 1, y + h - 2, x + w - 2,
                     y + h - 2, x + w - 2, y);
}

/*******************************************************************************
 * Am_Draw_Rect_Border
 *   Used mainly for the Windows drawing routines, for drawing 3D like rectangles.
 *   Upper_left is the line style used to draw the top and left sides. Lower_right
 *   for the bottom and right.
 */

void
Am_Draw_Rect_Border(am_rect r, Am_Style upper_left, Am_Style lower_right,
                    Am_Drawonable *draw)
{
  r.width -= 1;
  r.height -= 1;
  draw->Draw_2_Lines(upper_left, Am_No_Style, r.left, r.top + r.height - 1,
                     r.left, r.top, r.left + r.width - 1, r.top);
  draw->Draw_2_Lines(lower_right, Am_No_Style, r.left, r.top + r.height,
                     r.left + r.width, r.top + r.height, r.left + r.width,
                     r.top);
}

/*******************************************************************************
 * Inset_Rect
 */

void
Inset_Rect(am_rect &r, int inset)
{
  r.left += inset;
  r.top += inset;
  r.width -= inset * 2;
  r.height -= inset * 2;
}
