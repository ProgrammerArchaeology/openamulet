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

#include <amulet/initializer.h>
#include <amulet/impl/opal_impl.h>
#include <amulet/formula.h>

inline int
imin(int i1, int i2)
{
  if (i1 < i2)
    return i1;
  else
    return i2;
}

Am_Object Am_Roundtangle;

Am_Define_Method(Am_Draw_Method, void, roundtangle_draw,
                 (Am_Object self, Am_Drawonable *drawonable, int x_offset,
                  int y_offset))
{
  int left = (int)self.Get(Am_LEFT) + x_offset;
  int top = (int)self.Get(Am_TOP) + y_offset;
  int width = self.Get(Am_WIDTH);
  int height = self.Get(Am_HEIGHT);
  int radius = self.Get(Am_DRAW_RADIUS);
  Am_Style ls = self.Get(Am_LINE_STYLE);
  Am_Style fs = self.Get(Am_FILL_STYLE);
  drawonable->Draw_Roundtangle(ls, fs, left, top, width, height, radius,
                               radius);
}

Am_Define_Method(Am_Draw_Method, void, roundtangle_mask,
                 (Am_Object self, Am_Drawonable *drawonable, int x_offset,
                  int y_offset))
{
  int left = (int)self.Get(Am_LEFT) + x_offset;
  int top = (int)self.Get(Am_TOP) + y_offset;
  int width = self.Get(Am_WIDTH);
  int height = self.Get(Am_HEIGHT);
  int radius = self.Get(Am_DRAW_RADIUS);
  Am_Style ls = self.Get(Am_LINE_STYLE);
  Am_Style fs = self.Get(Am_FILL_STYLE);
  drawonable->Draw_Roundtangle(ls, fs, left, top, width, height, radius, radius,
                               Am_DRAW_MASK_COPY);
}

Am_Define_Formula(int, compute_draw_radius)
{
  int radius = self.Get(Am_RADIUS);
  int smaller_side = imin(self.Get(Am_WIDTH), self.Get(Am_HEIGHT));
  if (radius >= 0)
    return imin(radius, (int)(smaller_side / 2));
  else {
    radius = (Am_Radius_Flag)radius;
    switch (radius) {
    case Am_SMALL_RADIUS:
      return (int)(smaller_side / 5);
    case Am_MEDIUM_RADIUS:
      return (int)(smaller_side / 4);
    case Am_LARGE_RADIUS:
      return (int)(smaller_side / 3);
    default:
      return 0;
    }
  }
}

static void
init()
{
  Am_Roundtangle = Am_Graphical_Object.Create(DSTR("Am_Roundtangle"))
                       .Add(Am_RADIUS, Am_SMALL_RADIUS)
                       .Add(Am_DRAW_RADIUS, compute_draw_radius)
                       .Add(Am_FILL_STYLE, Am_Black)
                       .Add(Am_LINE_STYLE, Am_Black)
                       .Set(Am_DRAW_METHOD, roundtangle_draw)
                       .Set(Am_MASK_METHOD, roundtangle_mask);

  Am_Object_Advanced temp = (Am_Object_Advanced &)Am_Roundtangle;

  // Maybe only need to register one of Am_RADIUS or Am_DRAW_RADIUS with
  // Demon since Am_DRAW_RADIUS formula depends on Am_RADIUS?
  //// QUESTION: Does changing the radius potentially change the bounding box?
  //// I think not.
  temp.Get_Slot(Am_RADIUS).Set_Demon_Bits(Am_MOVING_REDRAW | Am_EAGER_DEMON);
  temp.Get_Slot(Am_DRAW_RADIUS)
      .Set_Demon_Bits(Am_MOVING_REDRAW | Am_EAGER_DEMON);
  temp.Get_Slot(Am_FILL_STYLE)
      .Set_Demon_Bits(Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  temp.Get_Slot(Am_LINE_STYLE)
      .Set_Demon_Bits(Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
}

static void
cleanup()
{
  Am_Roundtangle.Destroy();
}

static Am_Initializer *initializer =
    new Am_Initializer(DSTR("Am_Roundtangle"), init, 2.1f, 121, cleanup);
