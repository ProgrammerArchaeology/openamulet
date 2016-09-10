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

/* This file contains the implementation of the LibGWT version of Am_Region
   Designed and implemented by Rodolphe Ortalo (Hi, Andrew ;-)
*/
#include <am_inc.h>

#include GEM__H
#include GEMG__H

// // // // // // // // // // // // // // // // // // // // // // // // // //
//
// Am_Region creator and destructor functions
//
// // // // // // // // // // // // // // // // // // // // // // // // // //

Am_Region* Am_Region::Create ()
{
  Am_Region_Impl* the_region = new Am_Region_Impl ();
  return the_region;
}

Am_Region_Impl::Am_Region_Impl ()
  : size(5), index(0), max_index(4)
{
  x_rgns = new gwt_region_t[size];
  for(int i=0; i<size; i++)
    x_rgns[i] = (0L);
}

void Am_Region_Impl::Destroy ()
{
  for (int i=0; i<size; i++)
    {
      gwtRegionDestroy(x_rgns[i]);
    }
  index = 0;
  delete [] x_rgns;
  delete this;
}

// // // // // // // // // // // // // // // // // // // // // // // // // //
//
// Am_Region manipulator functions
//
// // // // // // // // // // // // // // // // // // // // // // // // // //

bool Am_Region_Impl::all_rgns_used ()
{
  return (index == max_index);
}

void Am_Region_Impl::add_more_rgns()
{
  size += 5;
  max_index += 5;
  gwt_region_t *new_rgns = new gwt_region_t[size];
  memcpy(new_rgns, x_rgns, sizeof(gwt_region_t) * (size - 5));
  for(int i=size-5; i<size; i++)
    new_rgns[i] = (0L);
  delete [] x_rgns;
  x_rgns = new_rgns;
}

void Am_Region_Impl::Clear ()
{
  // TODO: Check why we do not destroy more regions ???
  index = 0;
  if (x_rgns[index])
    gwtRegionDestroy(x_rgns[index]);
  x_rgns[index] = (0L);
}

void Am_Region_Impl::Set(int the_left, int the_top,
			 unsigned int the_width, unsigned int the_height)
{
  Clear();
  gwtRegionCreate(&(x_rgns[0]));
  gwtRegionSetRect(x_rgns[0], the_left, the_top, the_width, the_height);
}


// You can call this function with an uninitialized Am_Region (i.e., you
// don't have to call Set_Region first.
void Am_Region_Impl::Push(Am_Region *the_region)
{
  if(all_rgns_used())
    add_more_rgns();
  // If this is an uninitialized Am_Region, then we want the index to stay 0
  // so we can initialize the first element in the x_rects array.  If this is
  // an old Am_Region, then initialize the next element.
  if(x_rgns[0])
      index = index + 1;
  if(x_rgns[index])
      gwtRegionDestroy(x_rgns[index]);
  gwtRegionCreate(&(x_rgns[index]));
  gwtRegionUnion(x_rgns[index],
		 static_cast<Am_Region_Impl*>(the_region)->region_to_use());
  if(index)
    gwtRegionIntersection(x_rgns[index], x_rgns[index-1]);

  return;
}

// You can call this function with an uninitialized Am_Region (i.e., you
// don't have to call Set_Region first.
//
void Am_Region_Impl::Push (int the_left, int the_top,
			   unsigned int the_width,
			   unsigned int the_height) {
  if (all_rgns_used())
    add_more_rgns();

  // If this is an uninitialized Am_Region, then we want the index to stay 0
  // so we can initialize the first element in the x_rects array.  If this is
  // an old Am_Region, then initialize the next element.
  if (x_rgns[0])
    index = index + 1;
  if (x_rgns[index])
    gwtRegionDestroy(x_rgns[index]);
  gwtRegionCreate(&(x_rgns[index]));
  gwtRegionAddRect(x_rgns[index], the_left, the_top, the_width, the_height);
  // If this region has already had rects pushed onto it, then intersect
  // this new region with all the ones that have come before it (it is
  // sufficient to intersect it with just the previous region).
  if (index)
    gwtRegionIntersection(x_rgns[index], x_rgns[index-1]);
}

void Am_Region_Impl::Pop ()
{
  if (index)
    {
      gwtRegionDestroy(x_rgns[index]);
      x_rgns[index] = (0L);
      index--;
    }
  else
    // index == 0, so designate an "uninitialized" region
    Clear ();
}

// Unions are performed on the most recently modified region.  That is, if
// a sequence of pushes have generated a particualr region, the union
// operation will be performed on the region resulting from the last push.
//
// You can call this function with an uninitialized Am_Region (i.e., you
// don't have to call Set_Region first.
//
void Am_Region_Impl::Union (int the_left, int the_top,
			    unsigned int the_width,
			    unsigned int the_height)
{
  if (!(x_rgns[index]))
    gwtRegionCreate(&(x_rgns[index]));
  gwtRegionAddRect(x_rgns[index], the_left, the_top, the_width, the_height);
}

void Am_Region_Impl::Intersect (int the_left, int the_top,
			        unsigned int the_width,
			        unsigned int the_height)
{
  if (!(x_rgns[index]))
    gwtRegionCreate(&(x_rgns[index]));
  gwt_region_t tmp_rect_region;
  gwtRegionCreate(&tmp_rect_region);
  gwtRegionSetRect(tmp_rect_region, the_left, the_top, the_width, the_height);
  gwtRegionIntersection(x_rgns[index], tmp_rect_region);
  gwtRegionDestroy(tmp_rect_region);
}

// Returns true if the point is inside the region.  A point exactly on the
// boundary of the region is considered inside the region.
bool Am_Region_Impl::In (int x, int y)
{
  return gwtPointInRegion(x_rgns[index], x, y);
}

// Returns true if the rectangle is completely inside or intersects the region.
// The total parameter is set to true if the rectangle is completely inside
// the region.
bool Am_Region_Impl::In (int x, int y, unsigned int width,
		         unsigned int height, bool& total)
{
  gwt_region_t tmp; int itotal;
  gwtRegionCreate(&tmp);
  gwtRegionSetRect(tmp, x, y, width, height);
  int ires = gwtInRegion(x_rgns[index], tmp, &itotal);
  gwtRegionDestroy(tmp);
  total = (itotal != 0);
  return (ires != 0);
}

// Returns true if the rectangle is completely inside or intersects the region.
// The total parameter is set to true if the rectangle is completely inside
// the region.
bool Am_Region_Impl::In (Am_Region *rgn, bool& total)
{
  // Well: we do much better, we test on real region, not only on
  // the bounding box of the argument...
  int itotal;
  // TODO: Check that this enhancement is correct...
  int ires = gwtInRegion(x_rgns[index],
			 dynamic_cast<Am_Region_Impl*>(rgn)->region_to_use(),
			 &itotal);
  total = (itotal != 0);
  return (ires != 0);
}
#if 0
  // Well: the X function is not very clever...
  static XRectangle x_rect;
  XClipBox (((Am_Region_Impl *)rgn)->region_to_use(), &x_rect);
  return In (x_rect.x, x_rect.y, x_rect.width, x_rect.height, total);
#endif
