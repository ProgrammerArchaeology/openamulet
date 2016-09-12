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

/* This file contains the implementation of the X version of Am_Region
   Designed and implemented by Andrew Mickish
   Updated by Rodolphe Ortalo, 1999
*/

#include <am_inc.h>

#include <amulet/gem.h>
#include <amulet/gemX.h>

// // // // // // // // // // // // // // // // // // // // // // // // // //
//
// Am_Region creator
//
// // // // // // // // // // // // // // // // // // // // // // // // // //
//
// This calls the Am_Region_Impl constructor in fact
//
Am_Region *
Am_Region::Create()
{
  return new Am_Region_Impl();
}

// // // // // // // // // // // // // // // // // // // // // // // // // //
//
// Am_Region_Impl destructor
//
// // // // // // // // // // // // // // // // // // // // // // // // // //
//
// This destructor is public
//
void
Am_Region_Impl::Destroy()
{
  Clear();
  delete this;
}
// TODO: See if a Am_Region_Impl::~Am_Region_Impl destructor
// TODO:is needed (is there a leak currently?).

// // // // // // // // // // // // // // // // // // // // // // // // // //
//
// Am_Region_Impl manipulator functions
//
// // // // // // // // // // // // // // // // // // // // // // // // // //
//
// Public functions
//
void
Am_Region_Impl::Clear()
{
  while (stack.empty() != true) {
    XDestroyRegion(stack.top());
    stack.pop();
  }
}

void
Am_Region_Impl::Set(int the_left, int the_top, unsigned int the_width,
                    unsigned int the_height)
{
  XRectangle x_rect; // was static... ???
  x_rect.x = the_left;
  x_rect.y = the_top;
  x_rect.width = the_width;
  x_rect.height = the_height;

  Clear();
  stack.push(XCreateRegion());
  XUnionRectWithRegion(&x_rect, stack.top(), stack.top());
}

#ifdef POLYGONAL_REGIONS
// This function can handle non-rectangular clipping-regions
// Deane Edmonds provided this code for X-Windows, it's not integral part
// of OpenAmulet yet, so to use this feature read-on:
//
// Deane Edmonds wrote:
// The next step is to get the clipping code to actually use the region as-is
// rather than just taking its bounding box. For my purposes I wanted to clip
// bitmaps, so I cheated and overrode the Am_DRAW_METHOD of the bitmap and
// created my own draw routine in the app.
//
// TODO: Check the update to STL stack container -- ortalo 990528
void
Am_Region_Impl::Set(Am_Point_List points)
{
  Clear();
  int numPoints = points.Length();
  if (numPoints > 0) {
    int x, y;
    XPoint *xpoints = new XPoint[numPoints];
    points.Start();
    for (int i = 0; i < numPoints; ++i) {
      points.Get(x, y);
      xpoints[i].x = x;
      xpoints[i].y = y;
      points.Next();
    }
    stack.push(XPolygonRegion(xpoints, numPoints, EvenOddRule));
  } else {
    stack.push(XCreateRegion());
  }
}
#endif // POLYGONAL_REGIONS

// You can call this function with an uninitialized Am_Region (i.e., you
// don't have to call Set_Region first.
void
Am_Region_Impl::Push(Am_Region *the_region)
{
  // Safely copy the given region into a new one
  Region new_region = XCreateRegion();
  Am_Region_Impl *the_region_impl = dynamic_cast<Am_Region_Impl *>(the_region);
  if ((the_region_impl->stack).empty() != true) {
    XUnionRegion((the_region_impl->stack).top(), new_region, new_region);
  }
  // Intersect with previous if needed
  if (stack.empty() != true) {
    XIntersectRegion(stack.top(), new_region, new_region);
  }
  // And push the new one
  stack.push(new_region);
}

// You can call this function with an uninitialized Am_Region (i.e., you
// don't have to call Set_Region first.
//
void
Am_Region_Impl::Push(int the_left, int the_top, unsigned int the_width,
                     unsigned int the_height)
{
  // Sets a rectangle
  XRectangle x_rect; // was static ???
  x_rect.x = the_left;
  x_rect.y = the_top;
  x_rect.width = the_width;
  x_rect.height = the_height;
  // Create a new rectangular region
  Region new_region = XCreateRegion();
  XUnionRectWithRegion(&x_rect, new_region, new_region);
  // Intersect with previous if needed
  if (stack.empty() != true) {
    XIntersectRegion(stack.top(), new_region, new_region);
  }
  // And push the new one
  stack.push(new_region);
}

void
Am_Region_Impl::Pop()
{
  if (stack.empty() != true) {
    XDestroyRegion(stack.top());
    stack.pop();
  }
}

// Unions are performed on the most recently modified region.  That is, if
// a sequence of pushes have generated a particualr region, the union
// operation will be performed on the region resulting from the last push.
//
// You can call this function with an uninitialized Am_Region (i.e., you
// don't have to call Set_Region first.
void
Am_Region_Impl::Union(int the_left, int the_top, unsigned int the_width,
                      unsigned int the_height)
{
  // Create a new region for top if needed
  if (stack.empty()) {
    stack.push(XCreateRegion());
  }
  // Set up a rectangle
  XRectangle x_rect; // was static ???
  x_rect.x = the_left;
  x_rect.y = the_top;
  x_rect.width = the_width;
  x_rect.height = the_height;
  // Does the union
  XUnionRectWithRegion(&x_rect, stack.top(), stack.top());
}

void
Am_Region_Impl::Intersect(int the_left, int the_top, unsigned int the_width,
                          unsigned int the_height)
{
  if (stack.empty()) {
    // Create a new region for top if needed
    stack.push(XCreateRegion());
  } else {
    // Set up a rectangle
    XRectangle x_rect; // was static ???
    x_rect.x = the_left;
    x_rect.y = the_top;
    x_rect.width = the_width;
    x_rect.height = the_height;
    Region rect_region = XCreateRegion();
    XUnionRectWithRegion(&x_rect, rect_region, rect_region);
    // Does the intersection
    XIntersectRegion(rect_region, stack.top(), stack.top());
    XDestroyRegion(rect_region);
  }
}

// Returns true if the point is inside the region.  A point exactly on the
// boundary of the region is considered inside the region.
bool
Am_Region_Impl::In(int x, int y)
{
  return ((stack.empty()) ? false : XPointInRegion(stack.top(), x, y));
}

// Returns true if the rectangle is completely inside or intersects the region.
// The total parameter is set to true if the rectangle is completely inside
// the region.
bool
Am_Region_Impl::In(int x, int y, unsigned int width, unsigned int height,
                   bool &total)
{
  if (stack.empty()) {
    total = false;
    return false;
  } else {
    int x_result = XRectInRegion(stack.top(), x, y, width, height);

    total = ((x_result == RectangleIn) ? true : false);
    return ((x_result == RectangleOut) ? false : true);
  }
}

// Returns true if the rectangle is completely inside or intersects the region.
// The total parameter is set to true if the rectangle is completely inside
// the region.
bool
Am_Region_Impl::In(Am_Region *region, bool &total)
{
  XRectangle x_rect; // Was static ???
  Am_Region_Impl *region_impl = dynamic_cast<Am_Region_Impl *>(region);
  if ((region_impl->stack)
          .empty()) { // The given region is empty... We return false, but well...
    total = true;
    return false;
  } else {
    XClipBox((region_impl->stack).top(), &x_rect);
    return In(x_rect.x, x_rect.y, x_rect.width, x_rect.height, total);
  }
}
