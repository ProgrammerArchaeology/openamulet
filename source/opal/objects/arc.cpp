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

#include INITIALIZER__H
#include <amulet/impl/opal_impl.h>

Am_Object Am_Arc;

Am_Define_Method(Am_Draw_Method, void, arc_draw,
                 (Am_Object self, Am_Drawonable *drawonable, int x_offset,
                  int y_offset))
{
  int left = (int)self.Get(Am_LEFT) + x_offset;
  int top = (int)self.Get(Am_TOP) + y_offset;
  int width = self.Get(Am_WIDTH);
  int height = self.Get(Am_HEIGHT);
  int angle1 = self.Get(Am_ANGLE1);
  int angle2 = self.Get(Am_ANGLE2);
  Am_Style ls = self.Get(Am_LINE_STYLE);
  Am_Style fs = self.Get(Am_FILL_STYLE);
  if ((width > 0) && (height > 0))
    drawonable->Draw_Arc(ls, fs, left, top, width, height, angle1, angle2);
}

Am_Define_Method(Am_Draw_Method, void, arc_mask,
                 (Am_Object self, Am_Drawonable *drawonable, int x_offset,
                  int y_offset))
{
  int left = (int)self.Get(Am_LEFT) + x_offset;
  int top = (int)self.Get(Am_TOP) + y_offset;
  int width = self.Get(Am_WIDTH);
  int height = self.Get(Am_HEIGHT);
  int angle1 = self.Get(Am_ANGLE1);
  int angle2 = self.Get(Am_ANGLE2);
  Am_Style ls = self.Get(Am_LINE_STYLE);
  Am_Style fs = self.Get(Am_FILL_STYLE);
  if ((width > 0) && (height > 0))
    drawonable->Draw_Arc(ls, fs, left, top, width, height, angle1, angle2,
                         Am_DRAW_MASK_COPY);
}

static void
init()
{
  Am_Arc = Am_Graphical_Object.Create(DSTR("Am_Arc"))
               .Add(Am_ANGLE1, 0)
               .Add(Am_ANGLE2, 360)
               .Add(Am_FILL_STYLE, Am_Black)
               .Add(Am_LINE_STYLE, Am_Black)
               .Set(Am_DRAW_METHOD, arc_draw)
               .Set(Am_MASK_METHOD, arc_mask);

  Am_Object_Advanced temp = (Am_Object_Advanced &)Am_Arc;
  temp.Get_Slot(Am_ANGLE1).Set_Demon_Bits(Am_MOVING_REDRAW | Am_EAGER_DEMON);
  temp.Get_Slot(Am_ANGLE2).Set_Demon_Bits(Am_MOVING_REDRAW | Am_EAGER_DEMON);
  temp.Get_Slot(Am_FILL_STYLE)
      .Set_Demon_Bits(Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  temp.Get_Slot(Am_LINE_STYLE)
      .Set_Demon_Bits(Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
}

static void
cleanup()
{
  Am_Arc.Destroy();
}

static Am_Initializer *initializer =
    new Am_Initializer(DSTR("Am_Arc"), init, 2.1f, 102, cleanup);
