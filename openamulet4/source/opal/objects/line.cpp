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
#include <amulet/impl/slots_registry.h>
#include <amulet/impl/opal_impl.h>
#include <amulet/impl/opal_op.h>
#include <amulet/impl/method_point_in.h>
#include WEB__H

#include <iostream.h>

inline int imin(int i1, int i2) {if (i1<i2) return i1; else return i2;}
inline int imax(int i1, int i2) {if (i1>i2) return i1; else return i2;}
inline int iabs(int i1) {if (i1<0) return -i1; else return i1;}


Am_Object Am_Line;


// gets the real line_thickness of the line_style of obj
int get_line_thickness (Am_Object obj)
{
  Am_Style ls = obj.Get(Am_LINE_STYLE);
  if (ls) {
    Am_Line_Cap_Style_Flag cap;
    short thickness;
    ls.Get_Line_Thickness_Values (thickness, cap);
    return thickness ? thickness : 1;
  }
  else
    return 0;  // no thickness because no line_style
}


/****************************************
 ** Functions to support drawing lines **
 ****************************************/

//web functions
bool line_x_create (const Am_Slot& slot)
{
  return slot.Get_Key () == Am_X1;
}

void line_x_init (const Am_Slot& slot,
		  Am_Web_Init& init)
{
  Am_Object_Advanced obj = slot.Get_Owner ();
  init.Note_Input (obj, Am_X1);
  init.Note_Input (obj, Am_X2);
  init.Note_Input (obj, Am_LEFT);
  init.Note_Input (obj, Am_WIDTH);
  init.Note_Input (obj, Am_LINE_STYLE);
  init.Note_Output (obj, Am_X2);
  init.Note_Output (obj, Am_LEFT);
  init.Note_Output (obj, Am_WIDTH);
}

void get_thickness_adjustments (const Am_Style& ls, int& lt_adjustment,
				       int& rb_adjustment, int& wh_adjustment)
{
  if (ls.Valid ()) {
    Am_Line_Cap_Style_Flag cap;
    short thickness;
    ls.Get_Line_Thickness_Values (thickness, cap);
    thickness = thickness ? thickness : 1;
    if (cap == Am_CAP_PROJECTING) {
      lt_adjustment = thickness;
      rb_adjustment = thickness;
      wh_adjustment = (2 * thickness);
    }
    else {
      lt_adjustment = thickness / 2;
      rb_adjustment = (thickness+1) / 2;
      wh_adjustment = thickness;
    }
  }
  else {
    lt_adjustment = 0;
    rb_adjustment = 0;
    wh_adjustment = 1;
  }
}

void line_x_validate (Am_Web_Events& events)
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
  Am_Style ls = self.Get (Am_LINE_STYLE);
  int lt_adjustment, rb_adjustment, wh_adjustment;
  get_thickness_adjustments (ls, lt_adjustment, rb_adjustment, wh_adjustment);
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
      } else {
	x1 = left + width - rb_adjustment;
	x2 = left + lt_adjustment;
      }
      x1x2_changed = true;
      //std::cout << "__  x1 " << x1 << " x2 " << x2 <<std::endl <<std::flush;
      break;
    case Am_LINE_STYLE:
      break;
    default:
      Am_ERROR ("** Bug: unexpected slot in line_x_validate: " << Am_Get_Slot_Name (slot.Get_Key()));
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

bool line_y_create (const Am_Slot& slot)
{
  return slot.Get_Key () == Am_Y1;
}

void line_y_init (const Am_Slot& slot,
		  Am_Web_Init& init)
{
  Am_Object_Advanced obj = slot.Get_Owner ();
  init.Note_Input (obj, Am_Y1);
  init.Note_Input (obj, Am_Y2);
  init.Note_Input (obj, Am_TOP);
  init.Note_Input (obj, Am_HEIGHT);
  init.Note_Input (obj, Am_LINE_STYLE);
  init.Note_Output (obj, Am_Y2);
  init.Note_Output (obj, Am_TOP);
  init.Note_Output (obj, Am_HEIGHT);
}

void line_y_validate (Am_Web_Events& events)
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
  Am_Style ls = self.Get (Am_LINE_STYLE);
  int lt_adjustment, wh_adjustment, rb_adjustment;
  get_thickness_adjustments (ls, lt_adjustment, rb_adjustment, wh_adjustment);
  while (!events.Last ()) {
    slot = events.Get ();
    //std::cout << "__Line_y_Web " << self << " slot " << slot.Get_Key ()
    //  << " t,h,y1,y2 " << top << " " << height << " " << y1 << " " << y2
    //	 <<std::endl <<std::flush;
    switch (slot.Get_Key ()) {
    case Am_Y1:
    case Am_Y2:
      top = imin (y1, y2) - lt_adjustment;
      height  = iabs (y2 - y1) + wh_adjustment;
      topheight_changed = true;
      //std::cout << "__  top " << top << " height " << height <<std::endl <<std::flush;
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
      //std::cout << "__  y1 " << y1 << " y2 " << y2 <<std::endl <<std::flush;
      break;
    case Am_LINE_STYLE:
      break;
    default:
      Am_ERROR ("** Bug: unexpected slot in line_y_validate: " << Am_Get_Slot_Name (slot.Get_Key()));
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

Am_Define_Method (Am_Draw_Method, void, line_draw,
		  (Am_Object self, Am_Drawonable* drawonable,
		   int x_offset, int y_offset))
{
  int x1 = (int)self.Get (Am_X1) + x_offset;
  int y1 = (int)self.Get (Am_Y1) + y_offset;
  int x2 = (int)self.Get (Am_X2) + x_offset;
  int y2 = (int)self.Get (Am_Y2) + y_offset;
  Am_Style ls = self.Get (Am_LINE_STYLE);
  drawonable->Draw_Line (ls, x1, y1, x2, y2);
}

Am_Define_Method (Am_Draw_Method, void, line_mask,
		  (Am_Object self, Am_Drawonable* drawonable,
		   int x_offset, int y_offset))
{
  int x1 = (int)self.Get (Am_X1) + x_offset;
  int y1 = (int)self.Get (Am_Y1) + y_offset;
  int x2 = (int)self.Get (Am_X2) + x_offset;
  int y2 = (int)self.Get (Am_Y2) + y_offset;
  Am_Style ls = self.Get (Am_LINE_STYLE);
  drawonable->Draw_Line (ls, x1, y1, x2, y2, Am_DRAW_MASK_COPY);
}


//  The coordinate system of x and y is defined w.r.t. ref_obj
Am_Define_Method(Am_Point_In_Method, Am_Object, line_point_in_obj,
		 (const Am_Object& in_obj, int x, int y,
		  const Am_Object& ref_obj)) {
  if ((bool)in_obj.Get (Am_VISIBLE)) {
    am_translate_coord_to_me(in_obj, ref_obj, x, y);
    // now x,y is so that 0,0 is the upper left corner of this line (left,top)
      int left = in_obj.Get (Am_LEFT);
    int top = in_obj.Get (Am_TOP);
    int x1 = (int)in_obj.Get (Am_X1) - left;
    int y1 = (int)in_obj.Get (Am_Y1) - top;
    int x2 = (int)in_obj.Get (Am_X2) - left;
    int y2 = (int)in_obj.Get (Am_Y2) - top;

    int hit_threshold = in_obj.Get (Am_HIT_THRESHOLD);
    int thickness = get_line_thickness (in_obj);
    int threshold = imax (hit_threshold, (thickness+1)/2);
    // *** HACK; why is this necessary??  Otherwise thickness 1 and 2 lines
    // *** are not selectable 
    if (threshold == 1) threshold = 2;

    // do simple bounding box test first
    if (x < imin (x1, x2) - threshold || x > imax (x1, x2) + threshold ||
	y < imin (y1, y2) - threshold || y > imax (y1, y2) + threshold) {
      return Am_No_Object;
    }

    // The old line hit formula.  Seems to work okay.
    
    //     // equation for line is ax + by + c = 0
    //     // d/sqrt(a^2+b^2) is the distance between line and point <x,y>
    //     long a = y1 - y2; //long type needed for Windows
    //     long b = x2 - x1;
    //     float c = long(x1)*y2 - long(x2)*y1;
    //     float d = ((float)a)*x + ((float)b)*y + c;
    // 
    //     if (d*d <= long(threshold) * threshold * (a*a + b*b) ) {
    //       return in_obj;
    //     }
    //     else {
    //       return Am_No_Object;
    //     }
    
    // the new line hit formula.  Also seems to work, based on
    // some Maple calculations.  More expensive.
    float numer = (float)(x * y1 - x * y2 + y * x2 - y * x1 + y2 * x1 - y1 * x2);
    float denom = (float)(y1 * y1 - 2 * y1 * y2 + y2 * y2
      + x2 * x2 - 2 * x2 * x1 + x1 * x1);
    float distance2 = numer * numer / denom;
    
    if (distance2 <= float(threshold) * float(threshold))
      return in_obj;
  }

  /* else... */
  return Am_No_Object;
}

static void init()
{
  Am_Web xweb (line_x_create, line_x_init, line_x_validate);
  Am_Web yweb (line_y_create, line_y_init, line_y_validate);

  Am_Line = Am_Graphical_Object.Create (DSTR("Am_Line"))
    .Set (Am_AS_LINE, true)
    .Add (Am_LINE_STYLE, Am_Black)
    .Set (Am_DRAW_METHOD, line_draw)
    .Set (Am_MASK_METHOD, line_mask)
    .Add (Am_X1, 0) // initialize all the values to first
    .Add (Am_Y1, 0)
    .Add (Am_X2, 0)
    .Add (Am_Y2, 0)
    .Set (Am_LEFT, 0)
    .Set (Am_TOP, 0)
    .Set (Am_WIDTH, 1)
    .Set (Am_HEIGHT, 1)
    .Add (Am_DIRECTIONAL, false)
    .Add (Am_HIT_THRESHOLD, 0)
    .Set (Am_POINT_IN_OBJ_METHOD, line_point_in_obj)
    .Set (Am_X1, xweb)
    .Set (Am_Y1, yweb)
    ;

  Am_Object_Advanced temp = (Am_Object_Advanced&)Am_Line;
  temp.Get_Slot (Am_LINE_STYLE).Set_Demon_Bits (Am_MOVING_REDRAW |
						Am_EAGER_DEMON);
  temp.Get_Slot (Am_X1).Set_Demon_Bits (Am_MOVING_REDRAW | Am_EAGER_DEMON);
  temp.Get_Slot (Am_Y1).Set_Demon_Bits (Am_MOVING_REDRAW | Am_EAGER_DEMON);
  temp.Get_Slot (Am_X2).Set_Demon_Bits (Am_MOVING_REDRAW | Am_EAGER_DEMON);
  temp.Get_Slot (Am_Y2).Set_Demon_Bits (Am_MOVING_REDRAW | Am_EAGER_DEMON);
}

static void
cleanup()
{
	Am_Line.Destroy ();
}


static Am_Initializer *initializer = new Am_Initializer(DSTR("Am_Line"), init, 2.1f, 111, cleanup);
