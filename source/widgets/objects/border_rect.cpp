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
#include <am_inc.h>

#include <amulet/impl/opal_obj.h>

Am_Object Am_Border_Rectangle;

/*******************************************************************************
 *  Am_Border_Rectangle
 *  draw routine for a plain border'ed rectangle
 */

Am_Define_Method(Am_Draw_Method, void, border_rectangle_draw,
                 (Am_Object self, Am_Drawonable *draw, int x_offset,
                  int y_offset))
{
  int left = (int)self.Get(Am_LEFT) + x_offset;
  int top = (int)self.Get(Am_TOP) + y_offset;
  int width = self.Get(Am_WIDTH);
  int height = self.Get(Am_HEIGHT);
  bool selected = self.Get(Am_SELECTED);
  Computed_Colors_Record color_rec = self.Get(Am_STYLE_RECORD);
  Am_Widget_Look look = self.Get(Am_WIDGET_LOOK);

  switch (look.value) {
  case Am_MOTIF_LOOK_val:
    Am_Draw_Motif_Box(left, top, width, height, selected, color_rec, draw);
    break;

  case Am_WINDOWS_LOOK_val: {
    Am_Draw_Windows_Box(left, top, width, height, selected, color_rec, draw);
    break;
  }

  case Am_MACINTOSH_LOOK_val:
    draw->Draw_Rectangle(Am_Black, Am_White, left, top, width, height);
    break;

  default:
    Am_Error("Unknown Look parameter");
    break;
  }
}

static void
init()
{
  Am_Border_Rectangle =
      Am_Graphical_Object.Create(DSTR("Border_Rectangle"))
          .Add(Am_SELECTED, false)
          .Add(Am_STYLE_RECORD, Am_Get_Computed_Colors_Record_Form)
          .Set(Am_WIDTH, 50)
          .Set(Am_HEIGHT, 50)
          .Add(Am_FILL_STYLE, Am_Default_Color)
          .Set(Am_DRAW_METHOD, border_rectangle_draw)
          .Add(Am_WIDGET_LOOK, Am_Default_Widget_Look);

  Am_Object_Advanced obj_adv;
  obj_adv = (Am_Object_Advanced &)Am_Border_Rectangle;
  obj_adv.Get_Slot(Am_SELECTED)
      .Set_Demon_Bits(Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  obj_adv.Get_Slot(Am_WIDGET_LOOK)
      .Set_Demon_Bits(Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  obj_adv.Get_Slot(Am_FILL_STYLE)
      .Set_Demon_Bits(Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
}

static void
cleanup()
{
  Am_Border_Rectangle.Destroy();
}

static Am_Initializer *initializer =
    new Am_Initializer(DSTR("Am_Border_Rectangle"), init, 5.0f, 104, cleanup);
