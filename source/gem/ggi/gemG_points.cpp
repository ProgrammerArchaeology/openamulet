/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <X11/Xlib.h>
#include <math.h>
#include <ggi/gwt.h>

#include <cstdio>
#include <iostream>

#include <am_inc.h>

#include GDEFS__H
#include GEM__H
#include GEMG__H

// // // // // // // // // // // // // // // // // // // //
// Am_Point_Array  (defined in gdefs.h)
// // // // // // // // // // // // // // // // // // // //

Am_WRAPPER_IMPL (Am_Point_Array);

Am_Point_Array::Am_Point_Array (Am_Point_List pl)
{
  data = new Am_Point_Array_Data (pl, 0, 0);
}
  

Am_Point_Array::Am_Point_Array (Am_Point_List pl, int offset_x, int offset_y)
{
  data = new Am_Point_Array_Data (pl, offset_x, offset_y);
}

Am_Point_Array::Am_Point_Array (int *ar, int num_coords)
{
  data = new Am_Point_Array_Data (ar, num_coords);
}

Am_Point_Array::Am_Point_Array (int num_points)
{
  data = new Am_Point_Array_Data (num_points);
}

int Am_Point_Array::Length ()
{
  return data->size;
}

void Am_Point_Array::Get (int index, int &x, int &y)
{
  x = data->ar[index].x;
  y = data->ar[index].y;
}

void Am_Point_Array::Set (int index, int x, int y)
{
  data->ar[index].x = x;
  data->ar[index].y = y;
}

void Am_Point_Array::Translate (int offset_x, int offset_y)
{
  int i;
  int size = data->size;
  GGIGEM_Point *ar = data->ar;

  for (int i = 0; i < size; ++i)
    {
      ar[i].x += offset_x;
      ar[i].y += offset_y;
    } 
}

// // // // // // // // // // // // // // // // // // // //
// Am_Point_Array_Data  (defined in gemX.h)
// // // // // // // // // // // // // // // // // // // //

Am_WRAPPER_DATA_IMPL (Am_Point_Array, (this));

Am_Point_Array_Data::Am_Point_Array_Data (Am_Point_Array_Data *proto)
{
  size = proto->size;
  ar = new GGIGEM_Point[size];
  for (int i = 0; i < size; ++i)
    {
      ar[i] = proto->ar[i];
    }
  //  memcpy(ar, proto->ar, sizeof(*ar)*size);
}

Am_Point_Array_Data::Am_Point_Array_Data (int ar_size)
{
  size = ar_size;
  ar = new GGIGEM_Point[size];
}

Am_Point_Array_Data::Am_Point_Array_Data (int *coords, int num_coords)
{
  size = num_coords / 2;
  ar = new GGIGEM_Point[size];

  for (int i = 0; i < size; ++i)
    {
      ar[i].x = coords[2*i];
      ar[i].y = coords[2*i + 1];
    }
}

Am_Point_Array_Data::Am_Point_Array_Data (Am_Point_List pl,
					  int offset_x, int offset_y)
{
  size = pl.Length();
  ar = new GGIGEM_Point[size];

  int i;
  for (pl.Start(), i = 0; i < size; ++i, pl.Next())
    {
      int x,y;
      pl.Get (x, y);
      ar[i].x = x + offset_x;
      ar[i].y = y + offset_y;
    }
}

Am_Point_Array_Data::~Am_Point_Array_Data ()
{
  delete [] ar;
  ar = (0L);
  size = 0;
}

// // // // // // // // // // // // // // // // // // // //
// Am_Drawonable_Impl functions
// // // // // // // // // // // // // // // // // // // //

static void compute_mitered_join_extents
(Am_Point_List pl, int thick, Am_Line_Cap_Style_Flag cap, 
 int &left, int &top, int &right, int &bottom);

void Am_Drawonable_Impl::Get_Polygon_Bounding_Box
(const Am_Point_List& pl, const Am_Style& ls, 
 int &out_left, int &out_top,
 int &width, int &height)
{
  int right, bottom, left, top;
  
  short thick;
  Am_Line_Cap_Style_Flag cap;
  Am_Join_Style_Flag join;
  Am_Line_Solid_Flag solid;
  const char* dash;
  int dashl;
  Am_Fill_Solid_Flag fill;
  Am_Fill_Poly_Flag poly;
  Am_Image_Array stipple;
  
  ls.Get_Values(thick, cap, join, solid, dash, dashl, fill, poly, stipple);
  
  if ((thick > 0) && (join == Am_JOIN_MITER) && (pl.Length() > 2))
    {
      // slow: does several flops per point to compute the true vertex of
      // the mitered join
      compute_mitered_join_extents (pl, thick, cap, 
				    left, top, right, bottom);
      out_left = left;
      out_top = top;
      width = right - left + 1;
      height = bottom - top + 1;
    }
  else
    {
      const short th = thick / 2;
      // fast: just four comparisons per point, then adjust thickness
      pl.Get_Extents(left, top, right, bottom);
      out_left = left - th;
      out_top = top - th;
      width = right - left + 1 + 2 * th;
      height = bottom - top + 1 + 2 * th;
    }
}

// get next vertex (i.e., non-duplicate point) and compute the unit vector
// to it
static bool next_vertex (Am_Point_List &pl, int x, int y,
			 int &nextx, int &nexty, 
			 double &ux, double &uy)
{
  for (; !pl.Last(); pl.Next())
    {
      pl.Get (nextx, nexty);
      const int dx = nextx - x; 
      const int dy = nexty - y;
      const double mag = sqrt(dx*dx + dy*dy);
      
      if (mag != 0.0)
	{  
	  ux = (double)dx / mag;
	  uy = (double)dy / mag;
	  return true;
	}
    }
  return false;
}

static void compute_mitered_join_extents
(Am_Point_List pl, int thick, Am_Line_Cap_Style_Flag cap,
 int &out_left, int &out_top, int &out_right, int &out_bottom)
{
  const double halfthick = (double)thick/2;

  int left, right, top, bottom;
  int minx, maxx, miny, maxy;

  // induction variables for loop: 
  int x,y;           // vertex under examination 
  int nextx, nexty;  // following vertex
  double ax,ay; // unit vector from (x,y) to its previous neighbor in pl
  double bx,by; // unit vector from (x,y) to its following neighbor in pl

  //  assert(pl.Length() > 2);

  // Get first and last points of polygon
  int firstx, firsty;
  int lastx, lasty;
  pl.Start ();
  pl.Get (firstx, firsty);
  pl.End ();
  pl.Get (lastx, lasty);

  if ((lastx == firstx) && (lasty == firsty))
    {
      // closed polygon -- first and last point meet at a mitered vertex

      // initialize lrtb
      left = right = firstx;
      top = bottom = firsty;

      // Set up loop conditions to calculate mitered join at point 1:
      pl.Prev();
      pl.Get (x, y); // point n-1
      pl.Start(); // now at point 1
      next_vertex (pl, x, y, x, y, bx, by);
      ax = -bx;  ay = -by;

      // (1) vector A points from point 1 (aka point n) to point n-1
      // (2) point 1 is in (x, y)
      // (3) iterator is about to return point 2
    }
  else
    {
      // open polygon -- first and last points are just capped
      int cap_th = (cap == Am_CAP_PROJECTING) ? thick : thick/2;
      
      left = firstx - cap_th;
      right = firstx + cap_th;
      top = firsty - cap_th;
      bottom = firsty + cap_th;
      
      minx = lastx - cap_th;
      maxx = lastx + cap_th;
      miny = lasty - cap_th;
      maxy = lasty + cap_th;
      
      left   = left   < minx  ? left   : minx ;
      right  = right  > maxx  ? right  : maxx ;
      top    = top    < miny  ? top    : miny ;
      bottom = bottom > maxy  ? bottom : maxy ;
      
      // Set up conditions for loop to calculate mitered join at point 2:
      pl.Start();
      pl.Next();  // now at point 2
      next_vertex (pl, firstx, firsty, x, y, bx, by);
      ax = -bx;
      ay = -by;
      
      // (1) vector A points from point 2 to point 1.
      // (2) point 2 is in (x,y)
      // (3) iterator is about to return point 3
    }
  
  while (next_vertex (pl, x, y, nextx, nexty, bx, by))
    {
      // if angle between a and b is less than about 11 degrees, 
      // X Windows uses a beveled join instead of a miter.
      double costheta = fabs(ax*bx + ay*by);
      if (costheta > 0.981627183 /*cos(11 degrees)*/)
	{
	  int th0 = thick / 2;
	  int th1 = (thick+1) / 2;
	  minx = x - th0;
	  maxx = x + th1;
	  miny = y - th0;
	  maxy = y + th1;
	}
      else
	{
	  // o = ABS(th * (a+b)/(ortho(a) T b))
	  double f = halfthick / (ax*by - ay*bx);
	  int ox = (int)ceil(fabs(f * (ax+bx)));
	  int oy = (int)ceil(fabs(f * (ay+by)));
	  minx = x - ox;
	  maxx = x + ox;
	  miny = y - oy;
	  maxy = y + oy;
	}
    
      left   = left   < minx  ? left   : minx ;
      right  = right  > maxx  ? right  : maxx ;
      top    = top    < miny  ? top    : miny ;
      bottom = bottom > maxy  ? bottom : maxy ;
      
      // update induction variables for next iteration:
      // (1) look at next vertex
      x=nextx, y=nexty;
      
      // (2) next vertex's A vector points back to this vertex, so it
      // is just -B.
      ax=-bx, ay=-by;
    }
  
  out_left = left;
  out_right = right;
  out_top = top;
  out_bottom = bottom;
}

void Am_Drawonable_Impl::poly_draw_it (const Am_Style& ls, const Am_Style& fs,
				       Am_Draw_Function f,
				       GGIGEM_Point* ar, const int num_points)
  const
{
  if (fs != Am_No_Style)
    { // if there is a fill style, fill it.
      set_gc_using_fill_style (fs, f);
     std::cerr << "GWT poly_draw_it not implemented (polygon fill)!" <<std::endl;
      // XFillPolygon (screen->display, xlib_drawable, screen->gc,
      //               ar, num_points, Complex, CoordModeOrigin);
    }
  if (ls != Am_No_Style)
    { // if there is a line style, outline it.
      set_gc_using_line_style (ls, f);
      for (int i = 1; i < num_points; i++)
	{
	  gwtDrawLine(gwt_window, ar[i-1].x, ar[i-1].y,
		      ar[i].x, ar[i].y);
	}
    }
}    

				     
// defaults:
//   f = Am_DRAW_COPY
void Am_Drawonable_Impl::Draw_Lines (const Am_Style& ls, const Am_Style& fs,
				     const Am_Point_Array& pts,
				     Am_Draw_Function f)
{
  Am_Point_Array_Data *data = Am_Point_Array_Data::Narrow (pts);

  if (data) {
    poly_draw_it(ls, fs, f, data->ar, data->size);
    data->Release();
  }
}

// default: f = Am_DRAW_COPY
// Optimization: This one is used a lot by widgets to draw arrows and
// such, so provide a function that doesn't need to allocate an Am_Point_Array.
void Am_Drawonable_Impl::Draw_2_Lines (const Am_Style& ls, const Am_Style& fs,
				       int x1, int y1, int x2, int y2,
				       int x3, int y3,
				       Am_Draw_Function f)
{
  GGIGEM_Point ar[3];
  ar[0].x = x1;
  ar[0].y = y1;
  ar[1].x = x2;
  ar[1].y = y2;
  ar[2].x = x3;
  ar[2].y = y3;
  poly_draw_it(ls, fs, f, ar, 3); // 3 is number of points
}

// default: f = Am_DRAW_COPY
// Optimization: This one is used a lot by widgets to draw arrows and
// such, so provide a function that doesn't need to allocate an Am_Point_Array
void Am_Drawonable_Impl::Draw_3_Lines (const Am_Style& ls, const Am_Style& fs,
				       int x1, int y1, int x2, int y2,
				       int x3, int y3, int x4, int y4,
				       Am_Draw_Function f)
{
  GGIGEM_Point ar[4];
  ar[0].x = x1;
  ar[0].y = y1;
  ar[1].x = x2;
  ar[1].y = y2;
  ar[2].x = x3;
  ar[2].y = y3;
  ar[3].x = x4;
  ar[3].y = y4;
  poly_draw_it(ls, fs, f, ar, 4); // 4 is number of points
}
