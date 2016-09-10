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
#include <math.h>

#include INITIALIZER__H
#include <amulet/impl/slots_registry.h>
#include <amulet/impl/opal_impl.h>
#include WEB__H

#include <iostream.h>

void get_thickness_adjustments (const Am_Style& ls, int& lt_adjustment,
				       int& rb_adjustment, int& wh_adjustment);
bool line_x_create (const Am_Slot& slot);
void line_x_init (const Am_Slot& slot, Am_Web_Init& init);
void line_x_validate (Am_Web_Events& events);
bool line_y_create (const Am_Slot& slot);
void line_y_init (const Am_Slot& slot, Am_Web_Init& init);
void line_y_validate (Am_Web_Events& events);

inline int imin(int i1, int i2) {if (i1<i2) return i1; else return i2;}
inline int imax(int i1, int i2) {if (i1>i2) return i1; else return i2;}
inline int iabs(int i1) {if (i1<0) return -i1; else return i1;}


Am_Object Am_Arrow_Line;

static void
arrow_line_x_init (const Am_Slot& slot, Am_Web_Init& init)
{
  Am_Object_Advanced obj = slot.Get_Owner ();
  init.Note_Input (obj, Am_X1);
  init.Note_Input (obj, Am_X2);
  init.Note_Input (obj, Am_LEFT);
  init.Note_Input (obj, Am_WIDTH);
  init.Note_Input (obj, Am_LINE_STYLE);
  init.Note_Input (obj, Am_HEAD_LENGTH);
  init.Note_Input (obj, Am_HEAD_WIDTH);
  init.Note_Output (obj, Am_X2);
  init.Note_Output (obj, Am_LEFT);
  init.Note_Output (obj, Am_WIDTH);
}

static void
arrow_line_y_init (const Am_Slot& slot, Am_Web_Init& init)
{
  Am_Object_Advanced obj = slot.Get_Owner ();
  init.Note_Input (obj, Am_Y1);
  init.Note_Input (obj, Am_Y2);
  init.Note_Input (obj, Am_TOP);
  init.Note_Input (obj, Am_HEIGHT);
  init.Note_Input (obj, Am_LINE_STYLE);
  init.Note_Input (obj, Am_HEAD_LENGTH);
  init.Note_Input (obj, Am_HEAD_WIDTH);
  init.Note_Output (obj, Am_Y2);
  init.Note_Output (obj, Am_TOP);
  init.Note_Output (obj, Am_HEIGHT);
}

static void
arrow_line_x_validate (Am_Web_Events& events)
{
  bool x1x2_changed = false;
  bool leftwidth_changed = false;

  events.Start ();
  Am_Slot slot = events.Get ();
  Am_Object self = slot.Get_Owner ();
  int x1 = self.Get (Am_X1);
  int x2 = self.Get (Am_X2);
  int left = self.Get (Am_LEFT);
  int width = self.Get (Am_WIDTH);
  int head_length = self.Get (Am_HEAD_LENGTH);
  int head_width = self.Get (Am_HEAD_WIDTH);
  Am_Style ls = self.Get (Am_LINE_STYLE);
  int lt_adjustment, rb_adjustment, wh_adjustment;
  get_thickness_adjustments (ls, lt_adjustment, rb_adjustment, wh_adjustment);
  if (head_length > head_width) {
    lt_adjustment += head_length;
    rb_adjustment += head_length;
    wh_adjustment += 2 * head_length;
  } else {
    lt_adjustment += head_width;
    rb_adjustment += head_width;
    wh_adjustment += 2 * head_width;
  }
  while (!events.Last ()) {
    slot = events.Get ();
    //std::cout << "__Line_x_Web " << self << " slot " << slot.Get_Key ()
    //  << " l,w,x1,x2 " << left << " " << width << " " << x1 << " " << x2
    //	 <<std::endl <<std::flush;
  switch (slot.Get_Key ()) {
    case Am_X1:
    case Am_X2:
      left = imin (x1, x2) - lt_adjustment;
      width  = iabs (x2 - x1) + wh_adjustment;
      leftwidth_changed = true;
      //std::cout << "__  left " << left << " width " << width <<std::endl <<std::flush;
      break;
    case Am_LEFT:
    case Am_WIDTH:
      if (x1 < x2) {
	x1 = left + lt_adjustment;
	x2 = left + width - rb_adjustment;
      }
      else {
	x1 = left + width - rb_adjustment;
	x2 = left + lt_adjustment;
      }
      x1x2_changed = true;
      //std::cout << "__  x1 " << x1 << " x2 " << x2 <<std::endl <<std::flush;
      break;
    case Am_LINE_STYLE:
    case Am_HEAD_WIDTH:
    case Am_HEAD_LENGTH:
      break;
    default:
      Am_ERROR ("** Bug: unexpected slot in arrow_line_x_validate: " << Am_Get_Slot_Name (slot.Get_Key()));
    }
    events.Next ();
  }
  
  if (x1x2_changed) {
    self.Set (Am_X1, x1);
    self.Set (Am_X2, x2);
  }

  if (leftwidth_changed) {
    self.Set (Am_LEFT, left);
    self.Set (Am_WIDTH, width);
  }
}

static void
arrow_line_y_validate (Am_Web_Events& events)
{
  bool y1y2_changed = false;
  bool topheight_changed = false;

  events.Start ();
  Am_Slot slot = events.Get ();
  Am_Object self = slot.Get_Owner ();
  int y1 = self.Get (Am_Y1);
  int y2 = self.Get (Am_Y2);
  int top = self.Get (Am_TOP);
  int height = self.Get (Am_HEIGHT);
  int head_length = self.Get (Am_HEAD_LENGTH);
  int head_width = self.Get (Am_HEAD_WIDTH);
  Am_Style ls = self.Get (Am_LINE_STYLE);
  int lt_adjustment, wh_adjustment, rb_adjustment;
  get_thickness_adjustments (ls, lt_adjustment, rb_adjustment, wh_adjustment);
  if (head_length > head_width) {
    lt_adjustment += head_length;
    rb_adjustment += head_length;
    wh_adjustment += 2 * head_length;
  } else {
    lt_adjustment += head_width;
    rb_adjustment += head_width;
    wh_adjustment += 2 * head_width;
  }
  while (!events.Last ()) {
    slot = events.Get ();
    switch (slot.Get_Key ()) {
    case Am_Y1:
    case Am_Y2:
      top = imin (y1, y2) - lt_adjustment;
      height  = iabs (y2 - y1) + wh_adjustment;
      topheight_changed = true;
      break;
    case Am_TOP:
    case Am_HEIGHT:
      if (y1 < y2) {
	y1 = top + lt_adjustment;
	y2 = top + height - rb_adjustment;
      }
      else {
	y1 = top + height - rb_adjustment;
	y2 = top + lt_adjustment;
      }
      y1y2_changed = true;
      break;
    case Am_LINE_STYLE:
    case Am_HEAD_WIDTH:
    case Am_HEAD_LENGTH:
      break;
    default:
      Am_ERROR ("** Bug: unexpected slot in arrow_line_y_validate: " << Am_Get_Slot_Name (slot.Get_Key()));
    }
    events.Next ();
  }

  if (y1y2_changed) {
    self.Set (Am_Y1, y1);
    self.Set (Am_Y2, y2);
  }

  if (topheight_changed) {
    self.Set (Am_TOP, top);
    self.Set (Am_HEIGHT, height);
  }
}


Am_Define_Method (Am_Draw_Method, void, arrow_line_draw,
		  (Am_Object self, Am_Drawonable* drawonable,
		   int x_offset, int y_offset))
{
  int x1 = (int)self.Get (Am_X1) + x_offset;
  int y1 = (int)self.Get (Am_Y1) + y_offset;
  int x2 = (int)self.Get (Am_X2) + x_offset;
  int y2 = (int)self.Get (Am_Y2) + y_offset;
  int head_width = self.Get (Am_HEAD_WIDTH);
  int head_length = self.Get (Am_HEAD_LENGTH);
  int dx = x2 - x1;
  int dy = y2 - y1;
  double d = sqrt ((double)(dx * dx + dy * dy));
  if (d < 1.0) {
    d = 1.0;
    dx = 1;
  }
  double lx = (head_length * dx) / d;
  double ly = (head_length * dy) / d;
  double wx = (head_width * dx) / d;
  double wy = (head_width * dy) / d;
  Am_Style ls = self.Get (Am_LINE_STYLE);
  //
  //
  // WARNING!!	egcs 2.90.29 will hit an internal error if you swap the
  //			ordering of the first two calls.
  //
  drawonable->Draw_Line (ls, (int)(x2 - lx + wy + 0.5),
			 (int)(y2 - ly - wx + 0.5), x2, y2);
  drawonable->Draw_Line (ls, x1, y1, x2, y2);
  drawonable->Draw_Line (ls, (int)(x2 - lx - wy + 0.5),
			 (int)(y2 - ly + wx + 0.5), x2, y2);
}

Am_Define_Method (Am_Draw_Method, void, arrow_line_mask,
		  (Am_Object self, Am_Drawonable* drawonable,
		   int x_offset, int y_offset))
{
  int x1 = (int)self.Get (Am_X1) + x_offset;
  int y1 = (int)self.Get (Am_Y1) + y_offset;
  int x2 = (int)self.Get (Am_X2) + x_offset;
  int y2 = (int)self.Get (Am_Y2) + y_offset;
  int head_width = self.Get (Am_HEAD_WIDTH);
  int head_length = self.Get (Am_HEAD_LENGTH);
  int dx = x2 - x1;
  int dy = y2 - y1;
  double d = sqrt ((double)(dx * dx + dy * dy));
  if (d < 1.0) {
    d = 1.0;
    dx = 1;
  }
  double lx = (head_length * dx) / d;
  double ly = (head_length * dy) / d;
  double wx = (head_width * dx) / d;
  double wy = (head_width * dy) / d;
  Am_Style ls = self.Get (Am_LINE_STYLE);

  //
  // WARNING!!	egcs 2.90.29 will hit an internal error if you swap the
  //			ordering of the first two calls.
  //
  drawonable->Draw_Line (ls, (int)(x2 - lx + wy + 0.5),
			 (int)(y2 - ly - wx + 0.5), x2, y2, Am_DRAW_MASK_COPY);
  drawonable->Draw_Line (ls, x1, y1, x2, y2, Am_DRAW_MASK_COPY);
  drawonable->Draw_Line (ls, (int)(x2 - lx - wy + 0.5),
			 (int)(y2 - ly + wx + 0.5), x2, y2, Am_DRAW_MASK_COPY);
}


#ifdef USE_SMARTHEAP
	#include <smrtheap.hpp>
#endif

static void init()
{
	#ifdef USE_SMARTHEAP
  		unsigned oldCheckPoint = dbgMemSetCheckpoint(3);
  	#endif

  Am_Arrow_Line = Am_Line.Create (DSTR("Am_Arrow_Line"))
    .Set (Am_DIRECTIONAL, true)
    .Add (Am_HEAD_LENGTH, 5)
    .Add (Am_HEAD_WIDTH, 3)
    .Set (Am_DRAW_METHOD, arrow_line_draw)
    .Set (Am_MASK_METHOD, arrow_line_mask)
  ;

  Am_Object_Advanced temp = (Am_Object_Advanced&)Am_Arrow_Line;
  temp.Get_Slot (Am_HEAD_WIDTH).Set_Demon_Bits (Am_MOVING_REDRAW |
						Am_EAGER_DEMON);
  temp.Get_Slot (Am_HEAD_LENGTH).Set_Demon_Bits (Am_MOVING_REDRAW |
						 Am_EAGER_DEMON);
  temp.Disinherit_Slot (Am_X1); // arrow doesn't work like regular lines
  temp.Disinherit_Slot (Am_Y1); // so we have to break the connection
  temp.Disinherit_Slot (Am_X2); // (would be better to make from scratch)
  temp.Disinherit_Slot (Am_Y2);
  temp.Disinherit_Slot (Am_LEFT);
  temp.Disinherit_Slot (Am_TOP);
  temp.Disinherit_Slot (Am_WIDTH);
  temp.Disinherit_Slot (Am_HEIGHT);

  Am_Web arrow_xweb (line_x_create, arrow_line_x_init, arrow_line_x_validate);
  Am_Web arrow_yweb (line_y_create, arrow_line_y_init, arrow_line_y_validate);

  Am_Arrow_Line
    .Add (Am_X1, 0) // reset positioning slot to correspond to arrow lines
    .Add (Am_Y1, 0)
    .Add (Am_X2, 0)
    .Add (Am_Y2, 0)
    .Add (Am_LEFT, -5)
    .Add (Am_TOP, -5)
    .Add (Am_WIDTH, 5)
    .Add (Am_HEIGHT, 5)
    .Set (Am_X1, arrow_xweb)
    .Set (Am_Y1, arrow_yweb)
  ;

	#ifdef USE_SMARTHEAP
		dbgMemSetCheckpoint(oldCheckPoint);
	#endif

}

//
// This initializer must run AFTER Am_Line.
//
static Am_Initializer* initializer =
	new Am_Initializer(DSTR("Am_Arrow_Line"), init, 2.2f);
