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
#include <amulet/formula.h>
#include <amulet/impl/opal_impl.h>
#include <amulet/impl/opal_op.h>

Am_Object Am_Bitmap;

/******************************************
 ** Functions to support drawing bitmaps **
 ******************************************/

Am_Define_Formula(int, compute_bitmap_width)
{
  Am_Image_Array image;
  image = self.Get(Am_IMAGE);
  if (image.Valid()) {
    int ret_width, ret_height;
    Am_Drawonable *drawonable = GV_a_drawonable(self);
    drawonable->Get_Image_Size(image, ret_width, ret_height);
    return ret_width;
  }
  // no image, so return 0
  return 0;
}

Am_Define_Formula(int, compute_bitmap_height)
{
  Am_Image_Array image;
  image = self.Get(Am_IMAGE);
  if (image.Valid()) {
    int ret_width, ret_height;
    Am_Object window, owner;
    Am_Drawonable *drawonable = GV_a_drawonable(self);
    drawonable->Get_Image_Size(image, ret_width, ret_height);
    return ret_height;
  }
  // no image, so return 0
  return 0;
}

Am_Define_Method(Am_Draw_Method, void, bitmap_draw,
                 (Am_Object self, Am_Drawonable *drawonable, int x_offset,
                  int y_offset))
{
  Am_Image_Array image = self.Get(Am_IMAGE);
  if (image.Valid()) {
    int left = (int)self.Get(Am_LEFT) + x_offset;
    int top = (int)self.Get(Am_TOP) + y_offset;
    Am_Style ls = self.Get(Am_LINE_STYLE);
    Am_Style fs = self.Get(Am_FILL_STYLE);
    bool monochrome = self.Get(Am_DRAW_MONOCHROME);
    drawonable->Draw_Image(left, top, -1, -1, image, 0, 0, ls, fs, monochrome);
  }
}

Am_Define_Method(Am_Draw_Method, void, bitmap_mask,
                 (Am_Object self, Am_Drawonable *drawonable, int x_offset,
                  int y_offset))
{
  Am_Image_Array image = self.Get(Am_IMAGE);
  if (image.Valid()) {
    int left = (int)self.Get(Am_LEFT) + x_offset;
    int top = (int)self.Get(Am_TOP) + y_offset;
    Am_Style ls = self.Get(Am_LINE_STYLE);
    Am_Style fs = self.Get(Am_FILL_STYLE);
    bool monochrome = self.Get(Am_DRAW_MONOCHROME);
    drawonable->Draw_Image(left, top, -1, -1, image, 0, 0, ls, fs, monochrome,
                           Am_DRAW_MASK_COPY);
  }
}

static void
init()
{
  Am_Bitmap =
      Am_Graphical_Object.Create(DSTR("Am_Bitmap"))
          .Add(Am_LINE_STYLE, Am_Black)
          .Add(Am_FILL_STYLE, Am_No_Style)
          .Add(Am_DRAW_MONOCHROME, false)
          .Add(Am_IMAGE, Am_No_Image)
          .Set(Am_WIDTH, compute_bitmap_width)
          .Set(Am_HEIGHT, compute_bitmap_height)
          //width and height can be set bigger or smaller than calculated
          // will clip if smaller and Am_H_ALIGN and Am_V_ALIGN if bigger
          //    .Set (Am_H_ALIGN, Am_LEFT_ALIGN)
          //    .Set (Am_V_ALIGN, Am_TOP_ALIGN)
          .Set(Am_DRAW_METHOD, bitmap_draw)
          .Set(Am_MASK_METHOD, bitmap_mask);
  Am_Object_Advanced temp = (Am_Object_Advanced &)Am_Bitmap;
  temp.Get_Slot(Am_IMAGE).Set_Demon_Bits(Am_MOVING_REDRAW | Am_EAGER_DEMON);
  temp.Get_Slot(Am_LINE_STYLE)
      .Set_Demon_Bits(Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  temp.Get_Slot(Am_FILL_STYLE)
      .Set_Demon_Bits(Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  temp.Get_Slot(Am_DRAW_MONOCHROME)
      .Set_Demon_Bits(Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
}

static void
cleanup()
{
  Am_Bitmap.Destroy();
}

static Am_Initializer *initializer =
    new Am_Initializer(DSTR("Am_Bitmap"), init, 2.1f, 103, cleanup);
