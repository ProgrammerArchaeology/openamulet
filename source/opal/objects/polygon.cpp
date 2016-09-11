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
#include <amulet/impl/am_point_list.h>
#include <amulet/impl/am_point_array.h>
#include <amulet/impl/opal_impl.h>
#include <amulet/impl/opal_op.h>
#include <amulet/impl/method_point_in.h>
#include WEB__H

#include <iostream>

int get_line_thickness (Am_Object obj);

inline int imax(int i1, int i2) {if (i1>i2) return i1; else return i2;}

Am_Object Am_Polygon;


///  point_in_line_segment: returns true iff (x,y) lies within 
///   distance "threshold" of the line segment <(x1,y1), (x2,y2)>

static bool point_in_line_segment (int x, int y, 
				   int x1, int y1, int x2, int y2,
				   int threshold)
{
  int left, right, top, bottom;
  if (x1 < x2) {
    left = x1; right = x2;
  }
  else {
    left = x2; right = x1;
  }
  if (y1 < y2) {
    top = y1; bottom = y2;
  }
  else {
    top = y2; bottom = y1;
  }
  
  // do simple bounding box test first
  if (x < left-threshold || x > right+threshold ||
      y < top-threshold || y > bottom+threshold) {
    return false;
  }

  // The old line hit formula.  Seems to work okay.
  
  // equation for line is ax + by + c = 0
  // d/sqrt(a^2+b^2) is the distance between line and point <x,y>
  long a = y1 - y2; //long type needed for Windows
  long b = x2 - x1;
  float c = (float)(long(x1)*y2 - long(x2)*y1);
  float d = ((float)a)*x + ((float)b)*y + c;
  
  return (d*d <= long(threshold) * threshold * (a*a + b*b));
}

//////////////////////////////////////////////////////////
///  functions to support drawing polygons

Am_Define_Method(Am_Draw_Method, void, polygon_draw,
		 (Am_Object self, Am_Drawonable* drawonable,
		  int x_offset, int y_offset)) {
  Am_Style ls = self.Get (Am_LINE_STYLE);
  Am_Style fs = self.Get (Am_FILL_STYLE);

  Am_Point_List pl = self.Get (Am_POINT_LIST);
  Am_Point_Array pts (pl, x_offset, y_offset);
  /// NDY: cache pts in a slot for faster drawing

  int left = self.Get (Am_LEFT);
  int top = self.Get (Am_TOP);
  int width = self.Get (Am_WIDTH);
  int height = self.Get (Am_HEIGHT);

  drawonable->Push_Clip (left + x_offset, top + y_offset, width, height);
  drawonable->Draw_Lines (ls, fs, pts);
  drawonable->Pop_Clip();
}

Am_Define_Method(Am_Draw_Method, void, polygon_mask,
		 (Am_Object self, Am_Drawonable* drawonable,
		  int x_offset, int y_offset)) {
  Am_Style ls = self.Get (Am_LINE_STYLE);
  Am_Style fs = self.Get (Am_FILL_STYLE);

  Am_Point_List pl = self.Get (Am_POINT_LIST);
  Am_Point_Array pts (pl, x_offset, y_offset);
  /// NDY: cache pts in a slot for faster drawing

  int left = self.Get (Am_LEFT);
  int top = self.Get (Am_TOP);
  int width = self.Get (Am_WIDTH);
  int height = self.Get (Am_HEIGHT);

  drawonable->Push_Clip (left + x_offset, top + y_offset, width, height);
  drawonable->Draw_Lines (ls, fs, pts, Am_DRAW_MASK_COPY);
  drawonable->Pop_Clip();
}

/// Returns non-zero if the line segment with endpoints <x1,y1> and <x2,y2>
/// crosses the ray pointing from <x,y> to <infinity,y>.
/// Returns 1 if the line segment crosses the ray going up 
/// (from low y to higher y), or -1 if it crosses the ray going down.

static int crosses_to_right_of (int x, int y, int x1, int y1, int x2, int y2)
{
  if ((y1 < y && y < y2) &&
      (x2-x)*(y1-y) < (x1-x)*(y2-y))
    return 1;
  else if ((y2 < y && y < y1) &&
	   (x1-x)*(y2-y) < (x2-x)*(y1-y))
    return -1;
  else
    return 0;
}

//  The coordinate system of x and y is defined w.r.t. ref_obj
Am_Define_Method(Am_Point_In_Method, Am_Object, polygon_point_in_obj,
		 (const Am_Object& in_obj, int x, int y,
		  const Am_Object& ref_obj))
{
  if ((bool)in_obj.Get (Am_VISIBLE)) {
    am_translate_coord_to_me(in_obj, ref_obj, x, y);
    // now x,y is so that 0,0 is the upper left corner of this line (left,top)
    int left = in_obj.Get (Am_LEFT);
    int top = in_obj.Get (Am_TOP);
    int width = in_obj.Get (Am_WIDTH);
    int height = in_obj.Get (Am_HEIGHT);

    int hit_threshold = in_obj.Get (Am_HIT_THRESHOLD);
    int thickness = get_line_thickness (in_obj);
    int threshold = imax (hit_threshold, (thickness+1)/2);
    // *** HACK; why is this necessary??  Otherwise thickness 1 and 2 lines
    // *** are not selectable 
    if (threshold == 1) threshold = 2;

    // do simple bounding box test first
    if (x < -threshold || x > width+threshold ||
	y < -threshold || y > height+threshold) {
      return Am_No_Object;
    }

    Am_Point_List pl = in_obj.Get (Am_POINT_LIST);
    int xfirst, yfirst;
    int x1, y1, x2, y2;
    int crossings = 0;
    bool last;

    pl.Start ();
    if (pl.Last()) return Am_No_Object; // no points!

    pl.Get(xfirst, yfirst);
    xfirst -= left;  yfirst -= top;
    x2 = xfirst;  y2 = yfirst;

    do {
      x1 = x2;  y1 = y2;

      pl.Next(); 
      last = pl.Last();
      if (last) {
	if ((bool) in_obj.Get (Am_SELECT_OUTLINE_ONLY)) {
	  // do not close an open polygon
	  // never came close to an edge, so fail
	  return Am_No_Object;
	}
	x2 = xfirst; y2 = yfirst; 
      }
      else { 
	pl.Get(x2, y2);
	x2 -= left;  y2 -= top; 
      }
      
      if (point_in_line_segment (x, y, x1, y1, x2, y2, threshold))
	// point is near an edge -- definitely a hit
	return in_obj;
      
      crossings += crosses_to_right_of (x, y, x1, y1, x2, y2);

    } while (!last);

    if ((bool)in_obj.Get (Am_SELECT_FULL_INTERIOR)) {
      // any unpaired crossings ==> inside outer edge of polygon
      if (crossings) return in_obj;
      else return Am_No_Object;
    } else {
      // odd number of crossings ==> inside filled space of polygon
      if (crossings & 1) return in_obj;
      else return Am_No_Object;
    }
  }
  else return Am_No_Object;
}

//web functions
static bool polygon_web_create (const Am_Slot& slot)
{
  return slot.Get_Key () == Am_LEFT;
}

static void polygon_web_init (const Am_Slot& slot,
			      Am_Web_Init& init)
{
  Am_Object_Advanced obj = slot.Get_Owner ();
  init.Note_Input (obj, Am_POINT_LIST);
  init.Note_Input (obj, Am_LEFT);
  init.Note_Input (obj, Am_TOP);
  init.Note_Input (obj, Am_WIDTH);
  init.Note_Input (obj, Am_HEIGHT);
  init.Note_Input (obj, Am_LINE_STYLE);
  init.Note_Output (obj, Am_POINT_LIST);
  init.Note_Output (obj, Am_TOP);
  init.Note_Output (obj, Am_WIDTH);
  init.Note_Output (obj, Am_HEIGHT);
}

static inline float scale_factor (int new_dim, int old_dim)
{  
  // if original dimension or new dimension is 1 (implying zero delta
  // between edges), use a delta of .25 instead of 0, to preserve 
  // the actual separation between points in point-list
  return 
    (float) ((new_dim > 1 ? new_dim-1 : 0.25f) 
    / (old_dim > 1 ? old_dim-1 : 0.25f));
}

/// general_polygon_validate() makes every effort to scale & translate
/// pl such that its bounding box is (left,top,width,height).
/// If it cannot (for instance, because all points in pl have same
/// x coordinate but width > 1), then scales pl as close as possible and
/// sets width and height to pl's actual bounding box.  
/// (left and top are never changed.)

static void general_polygon_validate (Am_Point_List &pl,
			    int &left, int &top,
			    int &width, int &height,
			    Am_Style ls, Am_Drawonable *draw)
{
  int pl_left, pl_top, pl_width, pl_height;
  draw->Get_Polygon_Bounding_Box(pl, ls, pl_left, pl_top, 
				 pl_width, pl_height);

  if (left == pl_left && top == pl_top &&
      width == pl_width && height == pl_height)
    return;  // no work to do!
    

#ifdef TEST_POLYGON_CONSTRAINT
 std::cout << "**** Searching for polygon scaling factors" <<std::endl;
#endif

  // Try a binary search for the right scaling factors.

  // initialize induction variables for loop:
  float xfactor = scale_factor (width, pl_width);
  float xscale_low = 0.5f * xfactor;
  float xscale_high = 2.0f * xfactor;
  float xscale = (xscale_low + xscale_high)/2;

  float yfactor = scale_factor (height, pl_height);
  float yscale_low = 0.5f * yfactor;
  float yscale_high = 2.0f * yfactor;
  float yscale = (yscale_low + yscale_high)/2;

  Am_Point_List new_pl = pl;
  new_pl.Scale (xscale, yscale, pl_left, pl_top);

  int curr_left, curr_top, curr_width, curr_height;
  draw->Get_Polygon_Bounding_Box(new_pl, ls, curr_left, curr_top,
				 curr_width, curr_height);

  int n = 16;         // limits depth of binary search

  // now begin loop
  while ((curr_width != width || curr_height != height) && n > 0) {
    // update induction variables
    if (curr_width < width)         xscale_low = xscale;
    else if (curr_width > width)    xscale_high = xscale;
    xscale = (xscale_low + xscale_high)/2;

    if (curr_height < height)         yscale_low = yscale;
    else if (curr_height > height)    yscale_high = yscale;
    yscale = (yscale_low + yscale_high)/2;

    new_pl = pl;
    new_pl.Scale (xscale, yscale, pl_left, pl_top);
    draw->Get_Polygon_Bounding_Box(new_pl, ls, curr_left, curr_top,
				   curr_width, curr_height);

    --n;
  }

  // curr_width and curr_height are now as close as they're gonna get.
  // translate to (left,top)
  new_pl.Translate (left - curr_left, top - curr_top);

  // return the final points list
  pl = new_pl;
}

static void polygon_web_validate ( Am_Web_Events& events)
{
  events.End ();
  Am_Slot slot = events.Get ();
  Am_Object self = slot.Get_Owner ();
  Am_Drawonable* draw = Get_a_drawonable (self);
  Am_Point_List pl = self.Get (Am_POINT_LIST);
  Am_Style ls = self.Get (Am_LINE_STYLE);
  int left = self.Get (Am_LEFT);
  int top  = self.Get (Am_TOP);
  int width = self.Get (Am_WIDTH);
  int height = self.Get (Am_HEIGHT);

  bool rescaled = false;
  bool pl_changed = false;
  bool bbox_changed = false;

  // compute ORIGINAL values of left/top/width/height by working
  // backwards through the change events
  events.End ();
  while (!events.First ()) {
    slot = events.Get ();

    Am_Slot_Key key = slot.Get_Key ();
    if (key == Am_POINT_LIST) {
      // these changes supersedes all earlier changes, so find its
      // bounding box and stop looking backwards
      if (pl.Valid() && draw)
	draw->Get_Polygon_Bounding_Box(pl, ls, left, top, width, height);
      bbox_changed = true;
      break;
    }
    else 
      switch (key) {
      case Am_WINDOW:
      case Am_DRAWONABLE:
	break;
      case Am_LINE_STYLE:
	ls = events.Get_Prev_Value ();
	break;
      case Am_LEFT:
	left = events.Get_Prev_Value ();
	break;
      case Am_TOP:
	top = events.Get_Prev_Value ();
	break;
      case Am_WIDTH:
	width = events.Get_Prev_Value ();
	break;
      case Am_HEIGHT:
	height = events.Get_Prev_Value ();
	break;

      default:
	Am_ERROR ("** Bug: unexpected slot in polygon_web_validate: " << Am_Get_Slot_Name (key));
      }

    events.Prev ();
  }

  // now go forwards, looking at the CHANGES made to each slot
  events.Next ();
  while (!events.Last ()) {
    slot = events.Get ();
    switch (slot.Get_Key ()) {
#if 0
    case Am_POINT_LIST:
      pl = self.Get (Am_POINT_LIST);
      if (pl.Valid() && draw)
	draw->Get_Polygon_Bounding_Box(pl, ls, left, top, width, height);
      bbox_changed = true;
      break;
#endif

    case Am_WINDOW:
    case Am_DRAWONABLE:
      if (pl.Valid() && draw)
	draw->Get_Polygon_Bounding_Box(pl, ls, left, top, width, height);
      bbox_changed = true;
      break;

    case Am_LINE_STYLE:
      // recompute bounding box with new line style
      ls = self.Get (Am_LINE_STYLE, Am_NO_DEPENDENCY);
      if (pl.Valid() && draw)
	draw->Get_Polygon_Bounding_Box(pl, ls, left, top, width, height);
      bbox_changed = true;
      break;

    case Am_LEFT:
      {
	int new_left = self.Get (Am_LEFT, Am_NO_DEPENDENCY);
	pl.Translate (new_left - left, 0);
	left = new_left;
	pl_changed = true;
	break;
      }

    case Am_TOP:
      {
	int new_top = self.Get (Am_TOP, Am_NO_DEPENDENCY);
	pl.Translate (0, new_top - top);
	top = new_top;
	pl_changed = true;
	break;
      }

    case Am_WIDTH:
      {
	int new_width = self.Get (Am_WIDTH, Am_NO_DEPENDENCY);
	pl.Scale (scale_factor (new_width, width), 1.0f, left, top);
	width = new_width;
	rescaled = true;
	pl_changed = true;
	break;
      }

    case Am_HEIGHT:
      {
	int new_height = self.Get (Am_HEIGHT, Am_NO_DEPENDENCY);
	pl.Scale (1.0f, scale_factor (new_height, height), left, top);
	height = new_height;
	rescaled = true;
	pl_changed = true;
	break;
      }
    }
    events.Next ();
  }

  if (rescaled && pl.Valid() && draw)
    // check that scaling produced a polygon with the correct bounding box.
    // how could this fail?  several possibilites: line thickness
    // doesn't scale;  and mitered joins may grow or shrink 
    // orthogonally to the scaling.

    general_polygon_validate (pl, left, top, width, height, ls, draw);

  
#ifdef TEST_POLYGON_CONSTRAINT
  // debugging check that bounding box is correct
  if (pl.Valid() && draw) {
    int curr_left, curr_top, curr_width, curr_height;
    draw->Get_Polygon_Bounding_Box(pl, ls, curr_left, curr_top, 
				   curr_width, curr_height);
    if (!(left == curr_left &&
	  top == curr_top &&
	  width == curr_width &&
	  height == curr_height))
     std::cout << "** polygon_web_validate: bounding box ("
	<< left << "," << top << "," << width << "," << height << ")"
	  << " inconsistent with point list ("
	    << curr_left << "," << curr_top << "," << curr_width << "," << 
	      curr_height << ")" <<std::endl;
  }
#endif

  if (pl_changed)
    self.Set (Am_POINT_LIST, pl);

  if (bbox_changed) {
    self.Set (Am_LEFT, left);
    self.Set (Am_TOP, top);
    self.Set (Am_WIDTH, width);
    self.Set (Am_HEIGHT, height);
  }
}

#if 0  // obsolete
Am_Define_Formula (int, compute_polygon_left)
{
  Am_Object window, owner;
  Am_Drawonable* draw;
  Am_Point_List pl;
  pl = self.GV (Am_POINT_LIST);
  if (pl && (window = self.GV (Am_WINDOW))
      && (draw = Am_Drawonable::Narrow (window.GV (Am_DRAWONABLE)))
      && (owner = self.GV_Owner ()))
    {
      Am_Style ls = self.GV (Am_LINE_STYLE);
      int n, left;
      
      draw->Get_Polygon_Bounding_Box(pl, ls, left, n, n, n);
      return left;
    }
  else return 0;
}

Am_Define_Formula (int, compute_polygon_top)
{
  Am_Object window, owner;
  Am_Drawonable* draw;
  Am_Point_List pl;
  pl = self.GV (Am_POINT_LIST);
  if (pl && (window = self.GV (Am_WINDOW))
      && (draw = Am_Drawonable::Narrow (window.GV (Am_DRAWONABLE)))
      && (owner = self.GV_Owner ()))
    {
      Am_Style ls = self.GV (Am_LINE_STYLE);
      int n, top;
      
      draw->Get_Polygon_Bounding_Box(pl, ls, n, top, n, n);
      return top;
    }
  else return 0;
}

Am_Define_Formula (int, compute_polygon_width)
{
  Am_Object window, owner;
  Am_Drawonable* draw;
  Am_Point_List pl;
  pl = self.GV (Am_POINT_LIST);
  if (pl && (window = self.GV (Am_WINDOW))
      && (draw = Am_Drawonable::Narrow (window.GV (Am_DRAWONABLE)))
      && (owner = self.GV_Owner ()))
    {
      Am_Style ls = self.GV (Am_LINE_STYLE);
      int n, width;
      
      draw->Get_Polygon_Bounding_Box(pl, ls, n, n, width, n);
      return width;
    }
  else return 0;
}

Am_Define_Formula (int, compute_polygon_height)
{
  Am_Object window, owner;
  Am_Drawonable* draw;
  Am_Point_List pl;
  pl = self.GV (Am_POINT_LIST);
  if (pl && (window = self.GV (Am_WINDOW))
      && (draw = Am_Drawonable::Narrow (window.GV (Am_DRAWONABLE)))
      && (owner = self.GV_Owner ()))
    {
      Am_Style ls = self.GV (Am_LINE_STYLE);
      int n, height;
      
      draw->Get_Polygon_Bounding_Box(pl, ls, n, n, n, height);
      return height;
    }
  else return 0;
}
#endif

static void init()
{
  Am_Web polyweb (polygon_web_create, polygon_web_init, polygon_web_validate);

  Am_Point_List empty_list;
  
  Am_Polygon = Am_Graphical_Object.Create (DSTR("Am_Polygon"))
    .Add (Am_LINE_STYLE, Am_Black)
    .Add (Am_FILL_STYLE, Am_Black)
    .Set (Am_DRAW_METHOD, polygon_draw)
    .Set (Am_MASK_METHOD, polygon_mask)
    .Set (Am_WIDTH, 0)   // initialize webbed slots to 0 first
    .Set (Am_HEIGHT, 0)
    .Set (Am_TOP, 0)
    .Set (Am_LEFT, 0)
    .Add (Am_POINT_LIST, empty_list)
    .Add (Am_HIT_THRESHOLD, 0)
    .Add (Am_SELECT_OUTLINE_ONLY, 0)
    .Add (Am_SELECT_FULL_INTERIOR, 0)
    .Set (Am_POINT_IN_OBJ_METHOD, polygon_point_in_obj)
    .Set (Am_LEFT, polyweb)
    ;

  Am_Object_Advanced temp = (Am_Object_Advanced&)Am_Polygon;
  temp.Get_Slot (Am_FILL_STYLE).Set_Demon_Bits (Am_STATIONARY_REDRAW |
						Am_EAGER_DEMON);
  temp.Get_Slot (Am_LINE_STYLE).Set_Demon_Bits (Am_MOVING_REDRAW |
						Am_EAGER_DEMON);
  temp.Get_Slot (Am_POINT_LIST).Set_Demon_Bits (Am_MOVING_REDRAW |
						Am_EAGER_DEMON);
}

static Am_Initializer* initializer = new Am_Initializer(DSTR("Am_Polygon"), init, 2.1f);
