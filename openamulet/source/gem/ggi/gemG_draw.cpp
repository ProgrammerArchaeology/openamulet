
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

// This file contains member function definitions for the Am_Drawonable_Impl
// object primarily concerned with drawing objects in windows.

#include <am_inc.h>

#include GDEFS__H
#include GEM__H
#include GEMG__H

// Install default values in the Am_DASH_LIST variables (declared in gdefs.h)
const char Am_DEFAULT_DASH_LIST[Am_DEFAULT_DASH_LIST_LENGTH]
 = Am_DEFAULT_DASH_LIST_VALUE;

// // // // // // // // // // // // // // // // // // // // // // // // // //
// utility functions
// // // // // // // // // // // // // // // // // // // // // // // // // //

// If the specified line-thickness is 0, then this function returns 1.
// Otherwise, it returns the specified line-thickness.
static inline int get_graphical_line_thickness (const Am_Style& ls)
{
  if (ls == Am_No_Style)
    return 0;
  else
    {
      short thickness;
      Am_Line_Cap_Style_Flag cap;
      ls.Get_Line_Thickness_Values (thickness, cap);
      return MAX(1, thickness);
    }
}

// // // // // // // // // // // // // // // // // // // // // // // // // //
// private methods
// // // // // // // // // // // // // // // // // // // // // // // // // //

void Am_Drawonable_Impl::set_gc_using_line_style (const Am_Style& line_style,
						  const Am_Draw_Function fct)
  const
{
  short thickness;
  Am_Line_Cap_Style_Flag cap;
  Am_Join_Style_Flag join;
  Am_Line_Solid_Flag solid_flag;
  const char *dash_l;
  int dash_l_length;
  Am_Fill_Solid_Flag fill_flag;
  Am_Fill_Poly_Flag poly_flag;
  Am_Image_Array stipple;
  float r,g,b;

  line_style.Get_Values(r,g,b,thickness, cap, join,
                        solid_flag, dash_l, dash_l_length, fill_flag,
                        poly_flag, stipple);

  ggi_color color;
  color.r = static_cast<uint16>((0xFFFF) * r);
  color.g = static_cast<uint16>((0xFFFF) * g);
  color.b = static_cast<uint16>((0xFFFF) * b);  
  // Try to set color.a ?
  ggi_visual_t visual;
  gwtGetVisual(gwt_window, &visual);
  ggi_pixel c_index = ggiMapColor(visual, &color);
  gwtSetGCForeground(gwt_window, c_index);
}

// FIXME: Currently it is very identical to the line style...
void Am_Drawonable_Impl::set_gc_using_fill_style(const Am_Style& fill_style,
						 const Am_Draw_Function fct)
  const
{
  float r,g,b;

  fill_style.Get_Values(r,g,b);

  ggi_color color;
  color.r = static_cast<uint16>((0xFFFF) * r);
  color.g = static_cast<uint16>((0xFFFF) * g);
  color.b = static_cast<uint16>((0xFFFF) * b);  
  ggi_visual_t visual;
  gwtGetVisual(gwt_window, &visual);
  ggi_pixel c_index = ggiMapColor(visual, &color);
  gwtSetGCForeground(gwt_window, c_index);
} 


// // // // // // // // // // // // // // // // // // // // // // // // // //
// Drawing routines
// // // // // // // // // // // // // // // // // // // // // // // // // //

// Defaults:
//   f = Am_DRAW_COPY
void Am_Drawonable_Impl::Draw_Rectangle (const Am_Style& ls, const Am_Style& fs,
					 int left, int top, int width, int height,
					 Am_Draw_Function f)
{
  if ((width <= 0) || (height <= 0))    return;

  if (ls.Valid())
    { //if there is a line style
      int thickness = get_graphical_line_thickness(ls);
      int th2 = thickness*2;
      int half_thickness = thickness/2;
      if ((height < th2) || (width < th2))
	{ // line-style too thick to fit in bbox,
	  //so fill bbox with line-style color
	  set_gc_using_fill_style (ls, f);
	  gwtDrawBox(gwt_window, left, top, width, height);
	}
      else
	{ // The usual case -- bbox can accommodate the line-style
	  if (fs.Valid()) {
	    set_gc_using_fill_style (fs, f);
	    gwtDrawBox(gwt_window, (left + thickness), (top + thickness),
		       (width - th2), (height - th2));
	  }
	  set_gc_using_line_style (ls, f);
	  const int x1 = left + half_thickness;
	  const int x2 = left + width - half_thickness;
	  const int y1 = top + half_thickness;
	  const int y2 = top + height - half_thickness;
	  gwtDrawLine(gwt_window, x1, y1, x2, y1);
	  gwtDrawLine(gwt_window, x1, y1, x1, y2);
	  gwtDrawLine(gwt_window, x2, y1, x2, y2);
	  gwtDrawLine(gwt_window, x1, y2, x2, y2);
	  /* The above is equivalent to:
	     gwtDrawBoxOutline(gwt_window, (left + half_thickness),
	     (top + half_thickness), (width - thickness), (height - thickness));
	  */
	}
    }
  else
    {  // No line style
      if (fs.Valid())
	{ //if there is a filling style but no line style
	  set_gc_using_fill_style (fs, f);
	  gwtDrawBox(gwt_window, left, top, width, height);
	}
    }
}


#define Am_0_DEGREES        0
#define Am_90_DEGREES   90*64
#define Am_180_DEGREES 180*64
#define Am_270_DEGREES 270*64
#define Am_360_DEGREES 360*64

// Defaults:
//   angle1 = 0,
//   angle2 = 360,
//   f = Am_DRAW_COPY,p
//   asf = Am_ARC_PIE_SLICE
void Am_Drawonable_Impl::Draw_Arc (const Am_Style& ls, const Am_Style& fs,
				   int left, int top,
				   unsigned int width, unsigned int height,
				   int angle1, int angle2,
				   Am_Draw_Function f,
				   Am_Arc_Style_Flag)
{
  // There is some obvious hacking going on in here: see even_thickness below.
  // We have 2 problems to deal with:
  // 1. X fills arcs one pixel narrower and taller than it draws them.
  // 2. roundoff errors with odd line thicknesses since we divide
  // thickness in half occasionally.
  // The solution below _seems_ to work for all cases, but I admit it's
  // just a trial and error solution.  We might be better off getting a
  // Real Solution starting from scratch, if it breaks again.
  // Nov 7 1995 af1x

  if (width<=0 || height<=0)    return;

  angle1 *= 64; // Hmm: this is X11 specific I guess
  angle2 *= 64;

 std::cerr << "GWT or GGI: Draw_Arc to be implemented !!!!" <<std::endl;

  // However: we simulate the execution (and notably the GC updates)
  if (ls.Valid())
    { //if there is a line style
      int thickness   = get_graphical_line_thickness(ls);
      int half_thickness = thickness/2;
      if ((height <= (static_cast<unsigned>(thickness) * 2))
	  || (width <= (static_cast<unsigned>(thickness) * 2)))
	{ // line-style too thick to fit in bbox,
	  // so fill bbox with line-style color
	  set_gc_using_fill_style (ls, f);
	  /* XFillArc(disp, xlib_drawable, gc, left, top, width, height,
	     angle1, angle2);
	  */
	}
      else
	{ // The usual case -- bbox can accommodate the line-style
	  if (fs.Valid())
	    { //if there is a filling style
	      int even_thickness = 2 * half_thickness;
	      int thickness2  = thickness + even_thickness;
	      int fill_width  = width > (unsigned)thickness2 ? width  - thickness2 : 0;
	      int fill_height = height > (unsigned)thickness2 ? height - thickness2 : 0;
	      if (fill_width && fill_height)
		{
		  set_gc_using_fill_style (fs, f);
		  /* XFillArc (disp, xlib_drawable, gc,
		     (left + even_thickness), (top + even_thickness),
		     fill_width, fill_height, angle1, angle2);
		  */
		}
	    }
	  set_gc_using_line_style (ls, f);
	  /* XDrawArc (disp, xlib_drawable, gc,
	     (left + half_thickness), (top + half_thickness),
	     (width - thickness), (height - thickness),
	     angle1, angle2);
	  */
	}
    }
  else
    { // No line style
      if (fs.Valid())
	{ //if there is a filling style but no line style
	  set_gc_using_fill_style (fs, f);
	  /* XFillArc (disp, xlib_drawable, gc, left, top, width, height,
	     angle1, angle2);
	  */
	}
    }
}

// Defaults:
//   f = Am_DRAW_COPY
void Am_Drawonable_Impl::Draw_Line (const Am_Style& ls,
				    int x1, int y1, int x2, int y2,
				    Am_Draw_Function f)
{
  if (ls != Am_No_Style)
    { // if there is a line style
      set_gc_using_line_style (ls, f);
      gwtDrawLine(gwt_window, x1, y1, x2, y2);
    }
}

static void draw_roundtangle_fill (Am_Drawonable_Impl *d, int left, int top,
				   int width, int height, int right, int bottom,
				   unsigned short x_radius, unsigned short y_radius,
				   int thickness, int half_thickness, int th2,
				   int corner_width, int corner_height)
{
#if 0
  XArc arc[4];
  // more hackery.  see Draw_Arc for an explanation of why this is necessary.
  // Nov 7 1995 af1x
  int x1_aux = left + 2 * half_thickness;
  int y1_aux = top  + 2 * half_thickness;
  int x1_rad = left + x_radius;
  int y1_rad = top  + y_radius;
  th2 = thickness + 2 * half_thickness;
  int arc_fill_width   = corner_width  - th2;
  int arc_fill_height  = corner_height - th2;
  int side_fill_width  = x_radius - half_thickness;
  int side_fill_height = height - corner_height;
  int middle_fill_width = width - corner_width;
  int middle_fill_height = height - th2;

  if ((side_fill_width > 0) && (side_fill_height > 0))
    {
      // Left
      XFillRectangle (d->screen->display, d->xlib_drawable, d->screen->gc,
		      x1_aux, y1_rad, side_fill_width, side_fill_height);
      // Right
      XFillRectangle (d->screen->display, d->xlib_drawable, d->screen->gc,
		      (right - side_fill_width - thickness),
		      y1_rad, side_fill_width, side_fill_height);
    }

  if ((middle_fill_width > 0) && (middle_fill_height > 0))
    {
      // Middle
      XFillRectangle (d->screen->display, d->xlib_drawable, d->screen->gc,
		      x1_rad, y1_aux, middle_fill_width, middle_fill_height);
    }
  if ((arc_fill_width > 0) && (arc_fill_height > 0))
    {
      int x2_aux = right  - corner_width  + 2 * half_thickness;
      int y2_aux = bottom - corner_height + 2 * half_thickness;
      for(int i=0;i<4;i++)
	{
	  arc[i].width  = arc_fill_width;
	  arc[i].height = arc_fill_height;
	  arc[i].angle2 = Am_90_DEGREES;
	}
      // Top left
      arc[0].x = x1_aux;  arc[0].y = y1_aux;  arc[0].angle1 = Am_90_DEGREES;
      // Bottom left
      arc[1].x = x1_aux;  arc[1].y = y2_aux;  arc[1].angle1 = Am_180_DEGREES;
      // Bottom right
      arc[2].x = x2_aux;  arc[2].y = y2_aux;  arc[2].angle1 = Am_270_DEGREES;
      // Top right
      arc[3].x = x2_aux;  arc[3].y = y1_aux;  arc[3].angle1 = Am_0_DEGREES;
      XFillArcs(d->screen->display, d->xlib_drawable, d->screen->gc, arc, 4);
  }
#endif
}

static void draw_roundtangle_outline (Am_Drawonable_Impl *d, int left, int top,
				      int right, int bottom,
				      unsigned short x_radius,
				      unsigned short y_radius,
				      int thickness, int half_thickness,
				      int,  // th2 not used
				      int corner_width, int corner_height)
{
#if 0
  // Hackery by af1x.  Odd line thicknesses confused this routine immensely.
  // This is even more uglify-kludged than arc draw.

  // 0 for even lines, 1 for odd lines.
  int kludge_factor = thickness - (2 * half_thickness);

  // Note: lines and arcs use the same paradigm for drawing line-styles --
  // the lines and arcs are both centered on the given coordinates
  int left_line_x  = left   + half_thickness;
  int top_line_y   = top    + half_thickness;
  int right_line_x = right  - half_thickness - kludge_factor;
  int bot_line_y   = bottom - half_thickness - kludge_factor;
  int horiz_line_x1, horiz_line_x2, vert_line_y1, vert_line_y2;

  int arc_width  = corner_width  - thickness;
  int arc_height = corner_height - thickness;

  // If arcs would overlap, then don't draw arcs (see below)
  if ((arc_width > 0) && (arc_height > 0))
    {
      horiz_line_x1  = left /* + thickness */ + x_radius;
      horiz_line_x2  = right /* - thickness */ - x_radius;
      vert_line_y1   = top /* + thickness */ + y_radius;
      vert_line_y2   = bottom /* - thickness */ - y_radius;
    }
  else
    { // just draw lines into the corners
      horiz_line_x1  = left;
      horiz_line_x2  = right;
      vert_line_y1   = top;
      vert_line_y2   = bottom;
    }

  // Might be 4 segments, but if an axis is too short, we won't use all
  // of the structures
  XSegment seg[4];
  int seg_count = 0;

  if( vert_line_y1  < vert_line_y2  )
    {
      // Top left to bottom left
      seg[seg_count].x1 = left_line_x;     seg[seg_count].y1 = vert_line_y1;
      seg[seg_count].x2 = left_line_x;     seg[seg_count].y2 = vert_line_y2;
      seg_count += 1;
      // Bottom right to top right
      seg[seg_count].x1 = right_line_x;    seg[seg_count].y1 = vert_line_y2;
      seg[seg_count].x2 = right_line_x;    seg[seg_count].y2 = vert_line_y1;
      seg_count += 1;
    }

  if( horiz_line_x1 < horiz_line_x2 )
    {
      // Bottom left to bottom right
      seg[seg_count].x1 = horiz_line_x1;   seg[seg_count].y1 = bot_line_y;
      seg[seg_count].x2 = horiz_line_x2;   seg[seg_count].y2 = bot_line_y;
      seg_count += 1;
      // Top right to top left
      seg[seg_count].x1 = horiz_line_x2;   seg[seg_count].y1 = top_line_y;
      seg[seg_count].x2 = horiz_line_x1;   seg[seg_count].y2 = top_line_y;
      seg_count += 1;
    }
  XDrawSegments(d->screen->display, d->xlib_drawable, d->screen->gc,
		seg, seg_count);

  if ((arc_width > 0) && (arc_height > 0))
    {
      int right_arc_left = right - arc_width - half_thickness - kludge_factor;
      int bottom_arc_top = bottom - arc_height - half_thickness- kludge_factor;
      XArc arc[4];
      for(int i=0;i<4;i++)
	{
	  arc[i].width  = arc_width;
	  arc[i].height = arc_height;
	  arc[i].angle2 = Am_90_DEGREES;
	}
      // Top left
      arc[0].x = left_line_x;
      arc[0].y = top_line_y;
      arc[0].angle1 = Am_90_DEGREES;
      // Bottom left
      arc[1].x = left_line_x;
      arc[1].y = bottom_arc_top;
      arc[1].angle1 = Am_180_DEGREES;
      // Bottom right
      arc[2].x = right_arc_left;
      arc[2].y = bottom_arc_top;
      arc[2].angle1 = Am_270_DEGREES;
      // Top right
      arc[3].x = right_arc_left;
      arc[3].y = top_line_y;
      arc[3].angle1 = Am_0_DEGREES;
      XDrawArcs(d->screen->display, d->xlib_drawable, d->screen->gc, arc, 4);
    }
#endif
}

// Defaults:
//   f = Am_DRAW_COPY
void Am_Drawonable_Impl::Draw_Roundtangle (const Am_Style& ls, const Am_Style& fs,
					   int left, int top, int width,
					   int height,
					   unsigned short x_radius,
					   unsigned short y_radius,
					   Am_Draw_Function f)
{
  if ((width <= 0) || (height <= 0))
    return;

  int thickness = get_graphical_line_thickness(ls);
  int th2 = thickness*2;
  int half_thickness = thickness/2;
  int corner_width  = x_radius + x_radius;
  int corner_height = y_radius + y_radius;
  int right = left + width;
  int bottom = top + height;

 std::cerr << "GWT Draw_Roundtangle not implemented" <<std::endl;

  if (ls != Am_No_Style)
    { //if there is a line style
      if ((height < th2) || (width < th2))
	{ // line-style too thick to fit in bbox,
	  //so fill bbox with line-style color
	  set_gc_using_fill_style (ls, f);
	  gwtDrawBox(gwt_window, left, top, width, height);
	}
      else
	{ // The usual case -- bbox can accommodate the line-style
	  if (fs != Am_No_Style)
	    { //if there is a filling style
	      set_gc_using_fill_style (fs, f);
	      draw_roundtangle_fill (this, left, top, width, height, right, bottom,
				     x_radius, y_radius, thickness,
				     half_thickness, th2, corner_width,
				     corner_height);
	    }
	  set_gc_using_line_style (ls, f);
	  draw_roundtangle_outline (this, left, top, right, bottom,
				    x_radius, y_radius, thickness,
				    half_thickness, th2, corner_width,
				    corner_height);
	}
    }
  else
    { // No line style
      if (fs != Am_No_Style)
	{ //if there is a filling style but no line style
	  set_gc_using_fill_style (fs, f);
	  draw_roundtangle_fill(this, left, top, width, height, right, bottom,
				x_radius, y_radius, thickness,
				half_thickness, th2, corner_width,
				corner_height);
	}
    }
 std::cerr << "   (emulating with Draw_Rectangle)" <<std::endl;
  Draw_Rectangle(ls, fs, left, top, width, height, f);
}

//
// Clip region management
//
void Am_Drawonable_Impl::Set_Clip (Am_Region* region)
{
  // TODO: Check if this is really good.. (not in gemX_draw.cpp?!?!?)
  clip_region = region;
  gwt_region_t reg = dynamic_cast<Am_Region_Impl*>(region)->region_to_use();
  // Note: NULL is allowed (and means no clipping for Amulet and LibGWT)
  gwtSetClipping(gwt_window, reg);
}
// TODO: We should inline most of the following clip functions...
void Am_Drawonable_Impl::Set_Clip (int left, int top,
				   unsigned int width,
				   unsigned int height)
{
  clip_region->Set (left, top, width, height);
  Set_Clip (clip_region);
}

void Am_Drawonable_Impl::Clear_Clip ()
{
  clip_region->Clear ();
  Set_Clip (clip_region);
}

void Am_Drawonable_Impl::Push_Clip (Am_Region* region)
{
  clip_region->Push (region);
  Set_Clip (clip_region);
}

void Am_Drawonable_Impl::Push_Clip (int left, int top,
				    unsigned int width,
				    unsigned int height)
{
  clip_region->Push (left, top, width, height);
  Set_Clip (clip_region);
}

void Am_Drawonable_Impl::Pop_Clip ()
{
  clip_region->Pop ();
  Set_Clip (clip_region);
}

bool Am_Drawonable_Impl::In_Clip (int x, int y)
{
  return clip_region->In (x, y);
}

bool Am_Drawonable_Impl::In_Clip (int left, int top, unsigned int width,
	                          unsigned int height, bool &total)
{
  return clip_region->In (left, top, width, height, total);
}

bool Am_Drawonable_Impl::In_Clip (Am_Region *rgn, bool &total)
{
  return clip_region->In (rgn, total);
}

// Note: Clear_Area() respects the clip region!
void Am_Drawonable_Impl::Clear_Area(int arg_left, int arg_top,
				    int arg_width, int arg_height)
{
  set_gc_using_fill_style (background_color, Am_DRAW_COPY);
  gwtDrawBox(gwt_window, arg_left, arg_top, arg_width, arg_height);
}

/* sets the cursor for this drawonable */
void Am_Drawonable_Impl::Set_Cursor(Am_Cursor new_cursor)
{
  cursor = new_cursor;

  Am_Cursor_Data *cur = Am_Cursor_Data::Narrow(cursor);
  // TODO: Implement Set_Cursor 
  // but currently, the dynamic message is deactivated...
  // std::cerr << "GWT-GEM: Set_Cursor not implemented!" <<std::endl;
  cur->Release();
}
