/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact garnet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file contains the implementation of the Mac version of Am_Region

   Designed and implemented by Andrew Mickish
*/

#include <am_inc.h>

#include <amulet/gem.h>
#include <gemM.h>

// // // // // // // // // // // // // // // // // // // // // // // // // //
//
// Am_Region creator and destructor functions
//
// // // // // // // // // // // // // // // // // // // // // // // // // //

Am_Region *
Am_Region::Create()
{
  Am_Region_Impl *the_region = new Am_Region_Impl();
  return the_region;
}

Am_Region_Impl::Am_Region_Impl()
{
  size = 5;
  index = 0;
  max_index = size - 1;

  mac_rgns = new RgnHandle[size];
  for (int i = 0; i < size; i++) {
    mac_rgns[i] = (0L);
  }
}

void
Am_Region_Impl::Destroy()
{
  delete[] mac_rgns;
}

// // // // // // // // // // // // // // // // // // // // // // // // // //
//
// Am_Region manipulator functions
//
// // // // // // // // // // // // // // // // // // // // // // // // // //

bool
Am_Region_Impl::all_rgns_used()
{
  return (index == max_index);
}

void
Am_Region_Impl::add_more_rgns()
{
  size += 5;
  max_index += 5;
  RgnHandle *new_rgns = new RgnHandle[size];
  memcpy(new_rgns, mac_rgns, sizeof(RgnHandle) * (size - 5));
  for (int i = size - 5; i < size; i++)
    new_rgns[i] = (0L);
  delete[] mac_rgns;
  mac_rgns = new_rgns;
}

void
Am_Region_Impl::Clear()
{
  index = 0;
  if (mac_rgns[index])
    DisposeRgn(mac_rgns[index]);
  mac_rgns[index] = (0L);
  // Initial region will be regenerated when needed
}

void
Am_Region_Impl::Set(int the_left, int the_top, unsigned int the_width,
                    unsigned int the_height)
{
  Clear();
  mac_rgns[0] = NewRgn();
  SetRectRgn(mac_rgns[0], the_left, the_top, the_left + the_width,
             the_top + the_height);
}

// You can call this function with an uninitialized Am_Region (i.e., you
// don't have to call Set_Region first.
//
void
Am_Region_Impl::Push(Am_Region *the_region)
{
  // Since we want to install a copy of the_region anyway, pick it apart
  // and send the pieces to the rect version of this function.
  RgnHandle mac_rgn = ((Am_Region_Impl *)the_region)->region_to_use();
  Rect rgn_rect = (*mac_rgn)->rgnBBox;
  Push(rgn_rect.left, rgn_rect.top, rgn_rect.right - rgn_rect.left,
       rgn_rect.bottom - rgn_rect.top);
}

// You can call this function with an uninitialized Am_Region (i.e., you
// don't have to call Set_Region first.
//
void
Am_Region_Impl::Push(int the_left, int the_top, unsigned int the_width,
                     unsigned int the_height)
{
  if (all_rgns_used())
    add_more_rgns();

  // If this is an uninitialized Am_Region, then we want the index to stay 0
  // so we can initialize the first element in the x_rects array.  If this is
  // an old Am_Region, then initialize the next element.
  if (mac_rgns[0])
    index = index + 1;

  if (mac_rgns[index])
    DisposeRgn(mac_rgns[index]);
  mac_rgns[index] = NewRgn();
  SetRectRgn(mac_rgns[index], the_left, the_top, the_left + the_width,
             the_top + the_height);

  // If this region has already had rects pushed onto it, then intersect
  // this new region with all the ones that have come before it (it is
  // sufficient to intersect it with just the previous region).
  if (index)
    SectRgn(mac_rgns[index - 1], mac_rgns[index], mac_rgns[index]);
}

void
Am_Region_Impl::Pop()
{
  if (index) {
    DisposeRgn(mac_rgns[index]);
    mac_rgns[index] = (0L);
    index = index - 1;
  } else
    // index == 0, so designate this an "uninitialized" region
    Clear();
}

// Unions are performed on the most recently modified region.  That is, if
// a sequence of pushes have generated a particular region, the union
// operation will be performed on the region resulting from the last push.
//
// You can call this function with an uninitialized Am_Region (i.e., you
// don't have to call Set_Region first.
//
void
Am_Region_Impl::Union(int the_left, int the_top, unsigned int the_width,
                      unsigned int the_height)
{
  static RgnHandle the_rgn = NewRgn();
  SetRectRgn(the_rgn, the_left, the_top, the_left + the_width,
             the_top + the_height);
  if (!(mac_rgns[index])) // May have just been cleared
    mac_rgns[index] = NewRgn();
  UnionRgn(the_rgn, mac_rgns[index], mac_rgns[index]);
}

void
Am_Region_Impl::Intersect(int the_left, int the_top, unsigned int the_width,
                          unsigned int the_height)
{
  static RgnHandle the_rgn = NewRgn();
  SetRectRgn(the_rgn, the_left, the_top, the_left + the_width,
             the_top + the_height);
  if (!(mac_rgns[index]))
    mac_rgns[index] = NewRgn();
  SectRgn(the_rgn, mac_rgns[index], mac_rgns[index]);
}

// Returns true if the point is inside the region.  A point exactly on the
// boundary of the region is considered inside the region.
bool
Am_Region_Impl::In(int x, int y)
{
  Point pt = {x, y};
  return (PtInRgn(pt, mac_rgns[index]));
}

// Returns true if the rectangle is completely inside or intersects the region.
// The total parameter is set to true if the rectangle is completely inside
// the region.
bool
Am_Region_Impl::In(int x, int y, unsigned int width, unsigned int height,
                   bool &total)
{
  static RgnHandle rect_rgn = NewRgn();
  static Rect r;

  // First, check for mere intersection
  RgnHandle containing_rgn = region_to_use();
  SetRect(&r, x, y, x + width, y + height);
  bool intersects = RectInRgn(&r, containing_rgn);

  // If it intersects, check for total enclosure
  if (intersects) {
    // To see if the rectangle is entirely enclosed by the region, subtract
    // the containing_rgn from the rectangle, and see if the result is empty.
    RectRgn(rect_rgn, &r);
    DiffRgn(rect_rgn, containing_rgn, rect_rgn); // Destructively modifies
                                                 // rect_rgn
    total = EmptyRgn(rect_rgn);
  } else
    total = false;
  return intersects;
}

// Returns true if the rectangle is completely inside or intersects the region.
// The total parameter is set to true if the rectangle is completely inside
// the region.
bool
Am_Region_Impl::In(Am_Region *rgn, bool &total)
{
  RgnHandle inner_rgn = ((Am_Region_Impl *)rgn)->region_to_use();
  Rect rgn_rect = (*inner_rgn)->rgnBBox;
  return In(rgn_rect.left, rgn_rect.top, rgn_rect.right - rgn_rect.left,
            rgn_rect.bottom - rgn_rect.top, total);
}
