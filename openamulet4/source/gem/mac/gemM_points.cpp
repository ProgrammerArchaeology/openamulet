/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <stdio.h>
#include <istd::ostream.h>

#include <am_inc.h>

#include GDEFS__H
#include GEM__H
#include <gemM.h>

// // // // // // // // // // // // // // // // // // // //
// Am_Point_Array  (defined in gdefs.h)
// Am_Point_Array_Data  (defined in gemX.h)
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
  x = data->ar[index].h;
  y = data->ar[index].v;
}

void Am_Point_Array::Set (int index, int x, int y)
{
  data->ar[index].h = x;
  data->ar[index].v = y;
}

void Am_Point_Array::Translate (int offset_x, int offset_y)
{
  int i;
  int size = data->size;
  Point *ar = data->ar;

  for (i=0; i<size; ++i) {
    ar[i].h += offset_x;
    ar[i].v += offset_y;
  }
}

//////
// Am_Point_Array_Data constructors must set refs=1
//////

Am_WRAPPER_DATA_IMPL (Am_Point_Array, (this))

Am_Point_Array_Data::Am_Point_Array_Data (Am_Point_Array_Data *proto)
{
  size = proto->size;
  ar = new Point[size];
  memcpy(ar, proto->ar, sizeof(*ar)*size);
}

Am_Point_Array_Data::Am_Point_Array_Data (int ar_size)
{
  size = ar_size;
  ar = new Point[size];
}

Am_Point_Array_Data::Am_Point_Array_Data (int *coords, int num_coords)
{
  size = num_coords / 2;
  ar = new Point[size];

  int i;
  for (i=0; i<size; ++i) {
    ar[i].h = coords[2*i];
    ar[i].v = coords[2*i + 1];
  }
}

Am_Point_Array_Data::Am_Point_Array_Data (Am_Point_List pl, int offset_x, int offset_y)
{
  size = pl.Length();
  ar = new Point[size];

  int i, x, y;

  for (i=0, pl.Start(); i < size; ++i, pl.Next()) {
    pl.Get (x, y);
    ar[i].h = x + offset_x;
    ar[i].v = y + offset_y;
  }
}

Am_Point_Array_Data::~Am_Point_Array_Data ()
{
  delete [] ar;
  ar = (0L);
}

//////
// Am_Drawonable_Impl functions
//////

void Am_Drawonable_Impl::Get_Polygon_Bounding_Box
    (const Am_Point_List& pl, const Am_Style& ls, int &out_left, int &out_top,
     int &width, int &height)
{
  int right, bottom, left, top;
  short thick;

  thick = get_graphical_line_thickness(ls);
  pl.Get_Extents(left, top, right, bottom);

  out_left = left;
  out_top = top;
  width = right - left + thick;
  height = bottom - top + thick;
}

void Am_Drawonable_Impl::poly_draw_it (const Am_Style& ls, const Am_Style& fs,
									   Am_Draw_Function f,
									   Point* ar, int num_points) {
#ifdef DEBUG
  if (!mac_port) {
  	Am_Error("mac_port not set for Am_Drawonable_Impl::poly_draw_it");
  	return;
  }
#endif

  if (((fs != Am_No_Style) || (ls != Am_No_Style)) &&
      (num_points > 0)) {
	focus_on_this();
	PixMapHandle pix_map = lock_current_pixmap();

    // Build a polygon
    PolyHandle poly = OpenPoly();
    MoveTo (ar[0].h, ar[0].v);
    for (int i=1; i<num_points; i++) {
      LineTo (ar[i].h, ar[i].v);
    }
    ClosePoly ();

	if (fs != Am_No_Style) { // if there is a fill style, fill it.
	  set_pen_using_fill (fs, f);
	  PaintPoly (poly);
	}
	if (ls != Am_No_Style) { // if there is a line style, outline it.
	  set_pen_using_line (ls, f);
 	  FramePoly (poly);
	}
	KillPoly (poly);
	unlock_current_pixmap(pix_map);
  }
}

// defaults:
//   f = Am_DRAW_COPY
void Am_Drawonable_Impl::Draw_Lines (const Am_Style& ls, const Am_Style& fs,
				     const Am_Point_Array& pts,
				     Am_Draw_Function f) {
  Am_Point_Array_Data *data = Am_Point_Array_Data::Narrow (pts);

  if (data) {
    poly_draw_it(ls, fs, f, data->ar, data->size);
    data->Release();
  }
}

// default: f = Am_DRAW_COPY
// Optimization: This one is used a lot by widgets to draw arrows and
// such, so provide a function that doesn't need to allocate a point_list
void Am_Drawonable_Impl::Draw_2_Lines (const Am_Style& ls, const Am_Style& fs,
				       int x1, int y1, int x2, int y2,
				       int x3, int y3,
				       Am_Draw_Function f) {
  Point ar[3];
  ar[0].h = x1;
  ar[0].v = y1;
  ar[1].h = x2;
  ar[1].v = y2;
  ar[2].h = x3;
  ar[2].v = y3;
  poly_draw_it(ls, fs, f, (Point*)&ar, 3); // 3 is number of points
}

// default: f = Am_DRAW_COPY
// Optimization: This one is used a lot by widgets to draw arrows and
// such, so provide a function that doesn't need to allocate a point_list
void Am_Drawonable_Impl::Draw_3_Lines (const Am_Style& ls, const Am_Style& fs,
				       int x1, int y1, int x2, int y2,
				       int x3, int y3, int x4, int y4,
				       Am_Draw_Function f) {
  Point ar[4];
  ar[0].h = x1;
  ar[0].v = y1;
  ar[1].h = x2;
  ar[1].v = y2;
  ar[2].h = x3;
  ar[2].v = y3;
  ar[3].h = x4;
  ar[3].v = y4;
  poly_draw_it(ls, fs, f, (Point*)&ar, 4); // 4 is number of points
}
