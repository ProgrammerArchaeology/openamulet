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

extern "C" {
#include <X11/Xlib.h>
}

#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <string.h> // TODO: Check this header name
#include <am_inc.h>

#include GDEFS__H
#include GEM__H
#include GEMX__H

// Install default values in the Am_DASH_LIST variables (declared in gdefs.h)
//
const char Am_DEFAULT_DASH_LIST[Am_DEFAULT_DASH_LIST_LENGTH]
     = Am_DEFAULT_DASH_LIST_VALUE;

// // // // // // // // // // // // // // // // // // // // // // // // // //
// utility functions
// // // // // // // // // // // // // // // // // // // // // // // // // //

// If the specified line-thickness is 0, then this function returns 1.
// Otherwise, it returns the specified line-thickness.
static short get_graphical_line_thickness (const Am_Style& ls)
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

inline void adjust_gcvalues_color (XGCValues& values, unsigned long& mask,
				   unsigned long fore_index)
{
  if (fore_index != values.foreground) {
    values.foreground = fore_index;
    mask |= GCForeground;
  }
}

void Am_Drawonable_Impl::adjust_gcvalues_color_xor (XGCValues& values,
					    unsigned long& mask,
					    unsigned long fore_index) const
{
  // Compensate for white == 1 on an HP-type display
  if (screen->whitepixel) {
    if (fore_index == screen->blackpixel)
      fore_index = screen->whitepixel;
    else if (fore_index == screen->whitepixel)
      fore_index = screen->blackpixel;
  }

  adjust_gcvalues_color (values, mask, fore_index);
}

void Am_Drawonable_Impl::adjust_gcvalues_stipple (XGCValues& values,
			  unsigned long& mask, Am_Fill_Solid_Flag fill_flag,
			  const Am_Image_Array& image) const
{
  switch (fill_flag) {
  case Am_FILL_STIPPLED: {
    Am_Image_Array_Data* image_data = Am_Image_Array_Data::Narrow (image);
    values.stipple = image_data->Get_X_Pixmap (this);
    if (image_data)
      image_data->Release ();
    values.fill_style = FillStippled;
    mask |= GCStipple | GCFillStyle;
    break;
  }
  case Am_FILL_OPAQUE_STIPPLED: {
    Am_Image_Array_Data* image_data = Am_Image_Array_Data::Narrow (image);
    values.stipple = image_data->Get_X_Pixmap (this);
    if (image_data)
      image_data->Release ();
    values.fill_style = FillOpaqueStippled;
    mask |= GCStipple | GCFillStyle;
    break;
  }
  case Am_FILL_SOLID: {
    values.fill_style = FillSolid;
    mask |= GCFillStyle;
    break;
  }
  }
}

void Am_Drawonable_Impl::adjust_gcvalues_color_drawfn (XGCValues& values,
				   unsigned long& mask,
				   unsigned long fore_index,
				   Am_Draw_Function f) const
{
  switch (f) {
  case Am_DRAW_COPY:
  case Am_DRAW_MASK_COPY:
    if (values.function != GXcopy) {
      mask |= GCFunction;
      values.function = GXcopy;
    }
    adjust_gcvalues_color (values, mask, fore_index);
    break;
  case Am_DRAW_OR:
    if (screen->whitepixel) {
      // HP-type display, where white == 1
      if (values.function != GXand) {
	mask |= GCFunction;
	values.function = GXand;
      }
    // non-HP-type display, white == 0
    } else if (values.function != GXor) {
      mask |= GCFunction;
      values.function = GXor;
    }
    adjust_gcvalues_color (values, mask, fore_index);
    break;
  case Am_DRAW_XOR:
    if (values.function != GXxor) {
      mask |= GCFunction;
      values.function = GXxor;
    }
    adjust_gcvalues_color_xor (values, mask, fore_index);
    break;
  case Am_DRAW_GRAPHIC_OR:
    if (values.function != GXor) {
      mask |= GCFunction;
      values.function = GXor;
    }
    adjust_gcvalues_color (values, mask, fore_index);
    break;
  case Am_DRAW_GRAPHIC_XOR:
    if (values.function != GXxor) {
      mask |= GCFunction;
      values.function = GXxor;
    }
    adjust_gcvalues_color (values, mask, fore_index);
    break;
  case Am_DRAW_GRAPHIC_AND:
    if (values.function != GXand) {
      mask |= GCFunction;
      values.function = GXand;
    }
    adjust_gcvalues_color (values, mask, fore_index);
    break;
  case Am_DRAW_GRAPHIC_NIMP:
    if (values.function != GXandInverted) {
      mask |= GCFunction;
      values.function = GXandInverted;
    }
    adjust_gcvalues_color (values, mask, fore_index);
    break;
  }
}

inline void adjust_gcvalues_thickness (XGCValues& values, unsigned long& mask,
				       short thickness)
{
  if (values.line_width != thickness) {
    mask |= GCLineWidth;
    values.line_width = thickness;
  }
}

inline void set_gcvalues_fill_poly_rule (XGCValues& values,
					    unsigned long& mask,
					    Am_Fill_Poly_Flag poly)
{
  // changed from adjust_gcvalues to set_gcvalues
  // PURIFY: values.fill_rule uninitialized
    mask |= GCFillRule;
    values.fill_rule = poly;
}

inline void adjust_gcvalues_capstyle (XGCValues& values, unsigned long& mask,
				      Am_Line_Cap_Style_Flag cap)
{
  switch (cap) {
  case Am_CAP_BUTT:
    if (values.cap_style != CapButt) {
      mask |= GCCapStyle;
      values.cap_style = CapButt;
    }
    break;
  case Am_CAP_NOT_LAST:
    if (values.cap_style != CapNotLast) {
      mask |= GCCapStyle;
      values.cap_style = CapNotLast;
    }
    break;
  case Am_CAP_ROUND:
    if (values.cap_style != CapRound) {
      mask |= GCCapStyle;
      values.cap_style = CapRound;
    }
    break;
  case Am_CAP_PROJECTING:
    if (values.cap_style != CapProjecting) {
      mask |= GCCapStyle;
      values.cap_style = CapProjecting;
    }
    break;
  }
}

inline void adjust_gcvalues_joinstyle (XGCValues& values, unsigned long& mask,
				       Am_Join_Style_Flag join)
{
  switch (join) {
  case Am_JOIN_MITER:
    if (values.join_style != JoinMiter) {
      mask |= GCJoinStyle;
      values.join_style = JoinMiter;
    }
    break;
  case Am_JOIN_ROUND:
    if (values.join_style != JoinRound) {
      mask |= GCJoinStyle;
      values.join_style = JoinRound;
    }
    break;
  case Am_JOIN_BEVEL:
    if (values.join_style != JoinBevel) {
      mask |= GCJoinStyle;
      values.join_style = JoinBevel;
    }
    break;
  }
}

// Defaults:
//   image = Am_No_Image;

void Am_Drawonable_Impl::set_gc_using_fill (const Am_Style& fill,
					    Am_Draw_Function f,
					    Pixmap image) const
{
    XGCValues values;
    unsigned long mask = 0;

    Am_Fill_Solid_Flag fill_flag;
    unsigned long fore_index;

    if (XGetGCValues (screen->display, screen->gc,
		      Am_GC_getmask, &values) == 0)
      Am_Error ("** set_gc_using_fill: could not get GC values.\n");

    Am_Style_Data* fil = Am_Style_Data::Narrow(fill);
    if (fil) {
      if (f != Am_DRAW_MASK_COPY)
        fore_index = fil->Get_X_Index (this);
      else
        fore_index = Am_On_Bits_Data.Get_X_Index (this);
      fil->Release ();

      // If there is an image, use it for the stipple.  Else, use the
      // image stored in the style.
      if (image) {
        values.stipple = image;
        values.fill_style = FillStippled;
        mask |= GCStipple | GCFillStyle;
      }
      else {
        Am_Image_Array fill_stipple;
        fill_flag = fill.Get_Fill_Flag();
        if ((fill_flag == Am_FILL_OPAQUE_STIPPLED) &&
	    (f == Am_DRAW_MASK_COPY))
          fill_flag = Am_FILL_SOLID;
        else
          fill_stipple = fill.Get_Stipple();
        adjust_gcvalues_stipple (values, mask, fill_flag, fill_stipple);
      }

      Am_Fill_Poly_Flag poly = fill.Get_Fill_Poly_Flag ();
      set_gcvalues_fill_poly_rule (values, mask, poly);
    }
    else {
      fore_index = screen->whitepixel;
    }

    adjust_gcvalues_color_drawfn (values, mask, fore_index, f);
    if (mask) {
      XChangeGC (screen->display, screen->gc, mask, &values);
    }
}

void Am_Drawonable_Impl::set_gc_using_line (const Am_Style& line,
					    Am_Draw_Function f) const
{
    XGCValues values;
    unsigned long mask = 0;

    short thickness;
    Am_Line_Cap_Style_Flag cap;
    Am_Join_Style_Flag join;
    Am_Line_Solid_Flag line_flag;
    const char *dash_l;
    int dash_l_length;
    Am_Fill_Solid_Flag fill_flag;
    Am_Fill_Poly_Flag poly;
    Am_Image_Array stipple;

    line.Get_Values(thickness, cap, join,
		     line_flag, dash_l, dash_l_length, fill_flag,
		     poly, stipple);

    unsigned long fore_index;
    if (f != Am_DRAW_MASK_COPY) {
      //declare colors are Am_Style_Impls so can call Get_X_Index
      Am_Style_Data *lin = Am_Style_Data::Narrow(line);
      fore_index = lin->Get_X_Index(this);
      if (lin)
        lin->Release ();
    }
    else
      fore_index = Am_On_Bits_Data.Get_X_Index(this);

    if (XGetGCValues (screen->display, screen->gc,
		      Am_GC_getmask, &values) == 0)
      Am_Error ("** set_gc_using_line: could not get GC values.\n");

    adjust_gcvalues_color_drawfn (values, mask, fore_index, f);
    adjust_gcvalues_thickness (values, mask, thickness);
    adjust_gcvalues_capstyle (values, mask, cap);
    adjust_gcvalues_joinstyle (values, mask, join);
    adjust_gcvalues_stipple (values, mask, fill_flag, stipple);

    // You can't query the GC about its current dash pattern, so you have
    // to set it every time.  Fix this by storing the old value in the
    // screen object, so you can compare (without relying on X functions).
    mask |= GCLineStyle;
    switch (line_flag) {
    case Am_LINE_SOLID:
      values.line_style = LineSolid;
      break;
    case Am_LINE_ON_OFF_DASH:
      values.line_style = LineOnOffDash;
      break;
      // not supported
//     case Am_LINE_DOUBLE_DASH:
//       values.line_style = LineDoubleDash;
//       break;
    }

    if (mask) {
      XChangeGC (screen->display, screen->gc, mask, &values);
    }

    // XSetDashes call must be called after XChangeGC
    //
    if (dash_l_length)
      XSetDashes(screen->display, screen->gc, 0, dash_l, dash_l_length);
  }

// // // // // // // // // // // // // // // // // // // // // // // // // //
// Drawing routines
// // // // // // // // // // // // // // // // // // // // // // // // // //

// Defaults:
//   f = Am_DRAW_COPY
void Am_Drawonable_Impl::Draw_Rectangle (const Am_Style& ls, const Am_Style& fs,
					 int left, int top, int width,
					 int height, Am_Draw_Function f) {
  if ((width <= 0) || (height <= 0))
    return;

  GC gc = screen->gc;
  Display* disp = screen->display;

  if (ls.Valid()) { //if there is a line style
    short thickness = get_graphical_line_thickness(ls);
    int th2 = thickness*2;
    int half_thickness = thickness/2;
    if((height<th2)||(width<th2))
      {// line-style too thick to fit in bbox,
	//so fill bbox with line-style color
	set_gc_using_fill (ls, f);
	XFillRectangle (disp, xlib_drawable, gc, left, top, width, height);
      }
    else
      {// The usual case -- bbox can accommodate the line-style
	if (fs.Valid()) {
	  set_gc_using_fill (fs, f);
	  // keep origin of stipple, if any, in the same spot.
	  XSetTSOrigin(screen->display, screen->gc, 0, 0);
	  XFillRectangle (disp, xlib_drawable, gc,
			  (left + thickness), (top + thickness),
			  (width - th2), (height - th2));
	}
	set_gc_using_line (ls, f);
	XDrawRectangle (disp, xlib_drawable, gc,
			(left + half_thickness), (top + half_thickness),
			(width - thickness), (height - thickness));
      }
  }
  else
    {// No line style
      if (fs.Valid()) { //if there is a filling style but no line style
	set_gc_using_fill (fs, f);
	XFillRectangle (disp, xlib_drawable, gc, left, top, width, height);
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

  if (width<=0 || height<=0)
    return;

  GC gc = screen->gc;
  Display* disp = screen->display;
  angle1 *= 64;
  angle2 *= 64;

  if (ls.Valid()) { //if there is a line style
    int thickness   = get_graphical_line_thickness(ls);
    int half_thickness = thickness/2;
    if ((height <= ((unsigned)thickness) * 2) || (width <= ((unsigned)thickness) * 2))
      { // line-style too thick to fit in bbox,
	//so fill bbox with line-style color
	set_gc_using_fill (ls, f);
	XFillArc(disp, xlib_drawable, gc, left, top, width, height,
		 angle1, angle2);
      }
    else { // The usual case -- bbox can accommodate the line-style
      if (fs.Valid()) { //if there is a filling style
	int even_thickness = 2 * half_thickness;
	int thickness2  = thickness + even_thickness;
	int fill_width  = width > (unsigned)thickness2 ? width  - thickness2 : 0;
	int fill_height = height > (unsigned)thickness2 ? height - thickness2 : 0;
        if (fill_width && fill_height) {
          set_gc_using_fill (fs, f);
	  XFillArc (disp, xlib_drawable, gc,
		    (left + even_thickness), (top + even_thickness),
		    fill_width, fill_height, angle1, angle2);
	}
      }
      set_gc_using_line (ls, f);
      XDrawArc (disp, xlib_drawable, gc,
		(left + half_thickness), (top + half_thickness),
		(width - thickness), (height - thickness),
		angle1, angle2);
    }
  }
  else {// No line style
    if (fs.Valid()) { //if there is a filling style but no line style
      set_gc_using_fill (fs, f);
      XFillArc (disp, xlib_drawable, gc, left, top, width, height,
		angle1, angle2);
    }
  }
}

// Defaults:
//   f = Am_DRAW_COPY
void Am_Drawonable_Impl::Draw_Line (const Am_Style& ls,
			    int x1, int y1, int x2, int y2,
			    Am_Draw_Function f)
{
    GC gc = screen->gc;
    Display* disp = screen->display;

    if (ls != Am_No_Style) { //if there is a line style
        set_gc_using_line (ls, f);
	XDrawLine (disp, xlib_drawable, gc, x1, y1, x2, y2);
    }
}

void draw_roundtangle_fill (Am_Drawonable_Impl *d, int left, int top,
			    int width, int height, int right, int bottom,
			    unsigned short x_radius, unsigned short y_radius,
			    int thickness, int half_thickness, int th2,
			    int corner_width, int corner_height)
{
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

  if ((side_fill_width > 0) && (side_fill_height > 0)) {
    // Left
    XFillRectangle (d->screen->display, d->xlib_drawable, d->screen->gc,
		    x1_aux, y1_rad, side_fill_width, side_fill_height);
    // Right
    XFillRectangle (d->screen->display, d->xlib_drawable, d->screen->gc,
		    (right - side_fill_width - thickness),
		    y1_rad, side_fill_width, side_fill_height);
  }

  if ((middle_fill_width > 0) && (middle_fill_height > 0)) {
    // Middle
    XFillRectangle (d->screen->display, d->xlib_drawable, d->screen->gc,
		    x1_rad, y1_aux, middle_fill_width, middle_fill_height);
  }


  if ((arc_fill_width > 0) && (arc_fill_height > 0)) {
    int x2_aux = right  - corner_width  + 2 * half_thickness;
    int y2_aux = bottom - corner_height + 2 * half_thickness;
    for(int i=0;i<4;i++) {
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
}

void draw_roundtangle_outline (Am_Drawonable_Impl *d, int left, int top,
			       int right, int bottom,
			       unsigned short x_radius,
			       unsigned short y_radius,
			       int thickness, int half_thickness,
			       int,  // th2 not used
			       int corner_width, int corner_height)
{
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
  if ((arc_width > 0) && (arc_height > 0)) {
    horiz_line_x1  = left /* + thickness */ + x_radius;
    horiz_line_x2  = right /* - thickness */ - x_radius;
    vert_line_y1   = top /* + thickness */ + y_radius;
    vert_line_y2   = bottom /* - thickness */ - y_radius;
  }
  else { // just draw lines into the corners
    horiz_line_x1  = left;
    horiz_line_x2  = right;
    vert_line_y1   = top;
    vert_line_y2   = bottom;
  }

  // Might be 4 segments, but if an axis is too short, we won't use all
  // of the structures
  XSegment seg[4];
  int seg_count = 0;

  if( vert_line_y1  < vert_line_y2  ) {
    // Top left to bottom left
    seg[seg_count].x1 = left_line_x;     seg[seg_count].y1 = vert_line_y1;
    seg[seg_count].x2 = left_line_x;     seg[seg_count].y2 = vert_line_y2;
    seg_count += 1;
    // Bottom right to top right
    seg[seg_count].x1 = right_line_x;    seg[seg_count].y1 = vert_line_y2;
    seg[seg_count].x2 = right_line_x;    seg[seg_count].y2 = vert_line_y1;
    seg_count += 1;
  }

  if( horiz_line_x1 < horiz_line_x2 ) {
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

  if ((arc_width > 0) && (arc_height > 0)) {
    int right_arc_left = right - arc_width - half_thickness - kludge_factor;
    int bottom_arc_top = bottom - arc_height - half_thickness- kludge_factor;
    XArc arc[4];
    for(int i=0;i<4;i++) {
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

  GC gc = screen->gc;
  Display* disp = screen->display;
  short thickness = get_graphical_line_thickness(ls);
  int th2 = thickness*2;
  int half_thickness = thickness/2;
  int corner_width  = x_radius + x_radius;
  int corner_height = y_radius + y_radius;
  int right = left + width;
  int bottom = top + height;

  if (ls != Am_No_Style) { //if there is a line style
    if((height<th2)||(width<th2))
      {// line-style too thick to fit in bbox,
	//so fill bbox with line-style color
	set_gc_using_fill (ls, f);
	XFillRectangle (disp, xlib_drawable, gc, left, top,
			width, height);
      }
    else
      {// The usual case -- bbox can accommodate the line-style

	if (fs != Am_No_Style) { //if there is a filling style
	  set_gc_using_fill (fs, f);
	  draw_roundtangle_fill (this, left, top, width, height, right, bottom,
				 x_radius, y_radius, thickness,
				 half_thickness, th2, corner_width,
				 corner_height);
	}
	set_gc_using_line (ls, f);
	draw_roundtangle_outline (this, left, top, right, bottom,
				  x_radius, y_radius, thickness,
				  half_thickness, th2, corner_width,
				  corner_height);
      }
  }
  else {// No line style
    if (fs != Am_No_Style) { //if there is a filling style but no line style
      set_gc_using_fill (fs, f);
      draw_roundtangle_fill(this, left, top, width, height, right, bottom,
			    x_radius, y_radius, thickness,
			    half_thickness, th2, corner_width,
			    corner_height);
    }
  }
}
//
// X11 Drawonable clipping
//
void Am_Drawonable_Impl::Set_Clip (Am_Region* region)
{
  Am_Region_Impl* rgn_impl = dynamic_cast<Am_Region_Impl*>(region);
  Region x_region = rgn_impl->region_to_use();
  if (x_region)
    XSetRegion (screen->display, screen->gc, x_region);
  else
    XSetClipMask (screen->display, screen->gc, None);
}
//
// Note: Clear_Area() respects the clip mask!
//
void Am_Drawonable_Impl::Clear_Area(int arg_left, int arg_top,
				    int arg_width, int arg_height)
{

  set_gc_using_fill (background_color, Am_DRAW_COPY);
  // keep origin of stipple, if any, in the same spot.
  XSetTSOrigin(screen->display, screen->gc, 0, 0);
  XFillRectangle (screen->display, xlib_drawable, screen->gc,
		  arg_left, arg_top, arg_width, arg_height);
  /*

  Region x_rgn = ((Am_Region_Impl *)screen->clip_region)->region_to_use();

  if ( ! x_rgn ) {
    // Then the GC's clip_region is empty, so don't clip
//    XClearArea (screen->display, xlib_drawable, arg_left, arg_top,
//		arg_width, arg_height, 0);
      set_gc_using_fill (background_color, Am_DRAW_COPY);
      XFillRectangle (screen->display, xlib_drawable, screen->gc,
		      arg_left, arg_top, arg_width, arg_height);
    }
  else {

    // If width and height parameters are 0, set them to extend from
    // the arg_left and arg_top to the edges of the window.
    if (arg_width  == 0) arg_width  = width  - arg_left;
    if (arg_height == 0) arg_height = height - arg_top;

    XRectangle clip_rect;
    XClipBox(x_rgn, &clip_rect);

    int right  = arg_left + arg_width;
    int bottom = arg_top + arg_height;
    int gc_right  = clip_rect.x + clip_rect.width;
    int gc_bottom = clip_rect.y + clip_rect.height;

    int clip_left   = MAX(clip_rect.x, arg_left);
    int clip_top    = MAX(clip_rect.y, arg_top);
    int clip_right  = MIN(right, gc_right);
    int clip_bottom = MIN(bottom, gc_bottom);

    int clip_width  = clip_right  - clip_left;
    int clip_height = clip_bottom - clip_top;

//    printf("arg_left = %d, arg_top = %d, arg_width = %d, arg_height = %d,\n",
//	   arg_left, arg_top, arg_width, arg_height);
//    printf("gc_right = %d, gc_bottom = %d\n", gc_right, gc_bottom);
//    printf("clip_left = %d, clip_top = %d, clip_width = %d, clip_height = %d\n\n",
	   clip_left, clip_top, clip_width, clip_height);

    if ((clip_width > 0) && (clip_height > 0)) {
      set_gc_using_fill (background_color, Am_DRAW_COPY);
      XFillRectangle (screen->display, xlib_drawable, screen->gc,
		      clip_left, clip_top, clip_width, clip_height);

      //XClearArea (screen->display, xlib_drawable, clip_left, clip_top,
      //	  clip_width, clip_height, 0);
    }
  }
*/
}

/* sets the cursor for this drawonable */
void Am_Drawonable_Impl::Set_Cursor(Am_Cursor new_cursor)
{
  cursor = new_cursor;

  Am_Cursor_Data *cur = Am_Cursor_Data::Narrow(cursor);
  cur->main_cursor = cur->Get_X_Cursor(this);
  if (cur->main_cursor == 0) {
    XUndefineCursor(screen->display, xlib_drawable);
  } else {
    int tmp = XDefineCursor(screen->display, xlib_drawable, cur->main_cursor);

    if (tmp == BadCursor) {
     std::cerr << "Bad Cursor error in making cursor " <<std::endl;
      Am_Error ();
    } else if (tmp == BadWindow) {
     std::cerr << "Bad Window error in making cursor " <<std::endl;
      Am_Error ();
    }
  }
  cur->Release();
}

inline float color_error (int r1, int r2, int g1, int g2, int b1, int b2)
{
  // sum of squares of differences of color elements
#if !defined(_M_UNIX)
  return float((r1 - r2)) * float((r1 - r2))
    + float((g1 - g2)) * float((g1 - g2))
    + float((b1 - b2)) * float((b1 - b2));
#else
  float d = float((r1 - r2));
  if (d<0) d = -d;
  float d1 = float((g1 - g2));
  if (d1<0) d1 = -d1;
  if (d<d1) d = d1;
  d1 = float((b1 - b2));
  if (d1<0) d1 = -d1;
  if (d<d1) d = d1;
  return d;
#endif
}

void Paint_It_Black (XColor &c, Display *d, Colormap cm)
{
  c.red = 0;
  c.blue = 0;
  c.green = 0;
  if (!XAllocColor (d, cm, &c))
    Am_Error("** Fatal error: can't allocate black!\n");
}

void Am_Drawonable_Impl::Allocate_Closest_Color(XColor &c) const
{
	// if we have a match, do no work.
	if(XAllocColor(screen->display, screen->colormap, &c)) return;

	static int flag = 0;
	if(!flag)
	{
	std::cerr << "** No color cells left; using closest match.\n";
		flag = 1;
	}

	// X manual claims that the following 7 or so lines of code are only
	// slightly less convenient than just using "V->colormap_size" which
	// isn't allowed in case they change the Visual structure.  Crap.
	Visual *V = DefaultVisual(screen->display,  screen->screen_number);
	VisualID vid = XVisualIDFromVisual(V);
	XVisualInfo vit; // template
	vit.visualid = vid;
	int n;
	XVisualInfo *vi = XGetVisualInfo(screen->display, VisualIDMask,
		&vit, &n);
	if(n == 0)
	{
	std::cerr << "** X Error finding closest color match.  Using Black instead.\n";
		XFree(vi);
		Paint_It_Black(c, screen->display, screen->colormap);
		return;
	}
	int n_colors = vi->colormap_size;
	XFree(vi);

	int i;
	XColor *colors = new XColor[n_colors];
	// fill the array with pixel values we want the rgb's for.
	for(i = 0; i<n_colors; i++)
		colors[i].pixel = i;
	XQueryColors(screen->display, screen->colormap, colors, n_colors);
#if defined(_M_UNIX)
	float *ecolors = new float[n_colors];
	// Compute error colors for color table
	for(i=0; i<n_colors; i++)
		ecolors[i] = color_error(c.red, colors[i].red,
			c.green, colors[i].green,
			c.blue, colors[i].blue);
	// Sort array by base in error color
	for(i=0; i<n_colors; i++)
	{
		int j;
		for(j=i+1; j<n_colors; j++)
			if(ecolors[i]>ecolors[j])
			{
				float t = ecolors[i];
				ecolors[i] = ecolors[j];
				ecolors[j] = t;
				XColor ct = colors[i];
				colors[i] = colors[j];
				colors[j] = ct;
			}
			if(XAllocColor(screen->display, screen->colormap, &colors[i]))
			{
				c = colors[i];
				delete[] ecolors;
				delete[] colors;
				return;
			}
	}
	Paint_It_Black(c, screen->display, screen->colormap);
	delete[] ecolors;
	delete[] colors;
#else
	float e;
	int index_of_min = 0;
	float min_error = color_error(c.red, colors[0].red,
		c.green, colors[0].green,
		c.blue, colors[0].blue); // so we don't recalculate
	for(i = 1; i < n_colors; i++)
	{ // skip cell 0: default values.
		e = color_error(c.red, colors[i].red,
			c.green, colors[i].green,
			c.blue, colors[i].blue);
		if(e < min_error)
		{
			index_of_min = i;
			min_error = e;
		}
	}
std::cerr << "   Wanted r" << c.red << ", g" << c.green << ", b" << c.blue << "\n";
std::cerr << "   got    r" << colors[index_of_min].red << ", g" <<
		colors[index_of_min].green << ", b" << colors[index_of_min].blue << "\n";
	e = color_error(c.red, colors[index_of_min].red,
		c.green, colors[index_of_min].green,
		c.blue, colors[index_of_min].blue);
std::cerr << "  Error = " << e;
	e = color_error(c.red, colors[i].red,
		c.green, colors[i].green,
		c.blue, colors[i].blue);
std::cerr << " error with cell i = " << index_of_min <<std::endl;
	if(XAllocColor(screen->display, screen->colormap, &colors[index_of_min]))
		c = colors[index_of_min];
	else
		Paint_It_Black(c, screen->display, screen->colormap);
	delete[] colors;
#endif
}

