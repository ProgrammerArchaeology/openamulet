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

#include AM_IO__H
#include <amulet/impl/types_logging.h>
#include <amulet/impl/am_object.h>
#include <amulet/impl/opal_misc.h>
#include <amulet/impl/opal_constraints.h>
#include <amulet/impl/slots.h>
#include VALUE_LIST__H
#include FORMULA__H
#include <amulet/impl/am_alignment.h>

void get_max_sizes (Am_Value_List& components,
		    int& max_width, int& max_height)
{
  max_width = 0; max_height = 0;
  Am_Object item;
  for (components.Start (); !components.Last (); components.Next ()) {
    item = components.Get ();
    if ((bool)item.Get (Am_VISIBLE)) {
      int width = item.Get (Am_WIDTH);
      int height = item.Get (Am_HEIGHT);
      if (width > max_width)
	max_width = width;
      if (height > max_height)
	max_height = height;
    }
  }
}

void get_fixed_sizes (Am_Object self, Am_Value_List& components,
		      int& fixed_width, int& fixed_height)
{
  Am_Value value;
  value = self.Get(Am_FIXED_WIDTH);
  // the case where CC gets a bool as an int is taken care of automatically:
  // Am_MAX_FIXED_SIZE == 1 == (int)true; Am_NOT_FIXED_SIZE == 0 == (int)false
  if (value.type == Am_INT)
    fixed_width = value;
  else
    fixed_width = (bool)value ? Am_MAX_FIXED_SIZE : Am_NOT_FIXED_SIZE;
  value = self.Get(Am_FIXED_HEIGHT);
  if (value.type == Am_INT)
    fixed_height = value;
  else
    fixed_height = (bool)value ? Am_MAX_FIXED_SIZE : Am_NOT_FIXED_SIZE;
  if ((fixed_height == Am_MAX_FIXED_SIZE) ||
      (fixed_width == Am_MAX_FIXED_SIZE)) {
    int max_width, max_height;
    get_max_sizes (components, max_width, max_height);
    if (fixed_width == Am_MAX_FIXED_SIZE)
      fixed_width = max_width;
    if (fixed_height == Am_MAX_FIXED_SIZE)
      fixed_height = max_height;
  }
}

void get_max_rank_and_size (Am_Object self,
			    int& max_rank, int& max_size)
{
  Am_Value value;
  value = self.Peek(Am_MAX_RANK);
  if (value.type == Am_INT)
    max_rank = value;
  else
    max_rank = 0;
  value = self.Peek(Am_MAX_SIZE);
  if (value.type == Am_INT)
    max_size = value;
  else
    max_size = 0;
}

void find_line_size_and_rank (Am_Value_List& components, int indent,
			      int fixed_primary, Am_Slot_Key primary_slot,
			      int primary_spacing, Am_Slot_Key secondary_slot,
			      int max_rank, int max_size,
			      int& rank, int& max_secondary)
{
  Am_Object start_item;
  start_item = components.Get ();
  rank = 0;
  max_secondary = 0;
  int position = indent;
  while (!components.Last ()) {
    Am_Object item;
    item = components.Get ();
    if ((bool)item.Get (Am_VISIBLE)) {
      int primary = item.Get (primary_slot);
      int secondary = item.Get (secondary_slot);
      if (rank &&
	  ((max_rank && (rank == max_rank)) ||
	   (max_size &&
	    ((position + (fixed_primary ? fixed_primary : primary)) >=
	     max_size))))
	break;
      ++rank;
      position += (fixed_primary ? fixed_primary : primary) + primary_spacing;
      if (secondary > max_secondary)
        max_secondary = secondary;
    }
    components.Next ();
  }
  components.Start ();
  components.Member (start_item);
}

Am_Define_Formula (int, Am_Horizontal_Layout)
{
  Am_Value_List components;
  components = self.Get (Am_GRAPHICAL_PARTS);
  Am_Object component;
  for (components.Start(); !components.Last(); components.Next()) {
    component = components.Get();
    if (!(bool)component.Get(Am_VISIBLE)) components.Delete();
  }
  int fixed_width, fixed_height;
  get_fixed_sizes (self, components, fixed_width, fixed_height);
  int max_rank, max_size;
  get_max_rank_and_size (self, max_rank, max_size);
  int left_offset = self.Get (Am_LEFT_OFFSET);
  int left = left_offset;
  int top = self.Get (Am_TOP_OFFSET);
  int h_spacing = self.Get (Am_H_SPACING);
  int v_spacing = self.Get (Am_V_SPACING);
  Am_Alignment h_align = self.Get (Am_H_ALIGN);
  Am_Alignment v_align = self.Get (Am_V_ALIGN);
  int indent = self.Get (Am_INDENT);
  components.Start ();
  while (!components.Last ()) {
    int line_rank;
    int line_height;
    find_line_size_and_rank (components, left, fixed_width, Am_WIDTH,
			     h_spacing, Am_HEIGHT, max_rank, max_size,
			     line_rank, line_height);
    int rank = 0;
    while (rank < line_rank) {
      Am_Object item;
      item = components.Get ();
      if ((bool)item.Get (Am_VISIBLE)) {
        int width = item.Get (Am_WIDTH);
        int height = item.Get (Am_HEIGHT);
	if (fixed_width) {
	  switch (h_align.value) {
	  case Am_LEFT_ALIGN_val:
            item.Set (Am_LEFT, left);
	    break;
	  case Am_RIGHT_ALIGN_val:
            item.Set (Am_LEFT, left + fixed_width - width);
	    break;
	  case Am_CENTER_ALIGN_val:
            item.Set (Am_LEFT, left + (fixed_width - width) / 2);
	    break;
	  default:
	    Am_ERRORO("Bad alignment value " << h_align
		      << " in Am_H_ALIGN of " << self, self, Am_H_ALIGN);
	  }
	}
	else
	  item.Set (Am_LEFT, left);
        switch (v_align.value) {
	case Am_TOP_ALIGN_val:
          item.Set (Am_TOP, top);
	  break;
	case Am_BOTTOM_ALIGN_val:
          item.Set (Am_TOP, top + (fixed_height ? fixed_height : line_height)
		     - height);
	  break;
	case Am_CENTER_ALIGN_val:
          item.Set (Am_TOP, top + ((fixed_height ? fixed_height: line_height)
				    - height) / 2);
	  break;
	default:
	    Am_ERRORO("Bad alignment value " << v_align
		      << " in Am_V_ALIGN of " << self, self, Am_V_ALIGN);
	}
        left += (fixed_width ? fixed_width : width) + h_spacing;
        ++rank;
      }
      components.Next ();
    }
    top += (fixed_height ? fixed_height : line_height) + v_spacing;
    left = left_offset + indent;
  }
  return 0;
}

Am_Define_Formula (int, Am_Vertical_Layout)
{
  Am_Value_List components;
  components = self.Get (Am_GRAPHICAL_PARTS);
  Am_Object component;
  for (components.Start(); !components.Last(); components.Next()) {
    component = components.Get();
    if (!(bool)component.Get(Am_VISIBLE)) components.Delete();
  }
  int fixed_width, fixed_height;
  get_fixed_sizes (self, components, fixed_width, fixed_height);
  int max_rank, max_size;
  get_max_rank_and_size (self, max_rank, max_size);
  int top_offset = self.Get (Am_TOP_OFFSET);
  int left = self.Get (Am_LEFT_OFFSET);
  int top = top_offset;
  int h_spacing = self.Get (Am_H_SPACING);
  int v_spacing = self.Get (Am_V_SPACING);
  Am_Alignment h_align = self.Get (Am_H_ALIGN);
  Am_Alignment v_align = self.Get (Am_V_ALIGN);
  int indent = self.Get (Am_INDENT);
  components.Start ();
  while (!components.Last ()) {
    int line_rank;
    int line_width;
    find_line_size_and_rank (components, top, fixed_height, Am_HEIGHT,
			     v_spacing, Am_WIDTH, max_rank, max_size,
			     line_rank, line_width);
    int rank = 0;
    while (rank < line_rank) {
      Am_Object item;
      item = components.Get ();
      if ((bool)item.Get (Am_VISIBLE)) {
        int width = item.Get (Am_WIDTH);
        int height = item.Get (Am_HEIGHT);
	if (fixed_height) {
	  switch (v_align.value) {
	  case Am_TOP_ALIGN_val:
            item.Set (Am_TOP, top);
	    break;
	  case Am_BOTTOM_ALIGN_val:
            item.Set (Am_TOP, top + fixed_height - height);
	    break;
	  case Am_CENTER_ALIGN_val:
            item.Set (Am_TOP, top + (fixed_height - height) / 2);
	    break;
	  default:
	    Am_ERRORO("Bad alignment value " << v_align
		      << " in Am_V_ALIGN of " << self, self, Am_V_ALIGN);
	  }
	}
	else
	  item.Set (Am_TOP, top);
        switch (h_align.value) {
	case Am_LEFT_ALIGN_val:
          item.Set (Am_LEFT, left);
	  break;
	case Am_RIGHT_ALIGN_val:
          item.Set (Am_LEFT, left + (fixed_width ? fixed_width : line_width)
		     - width);
	  break;
	case Am_CENTER_ALIGN_val:
          item.Set (Am_LEFT, left + ((fixed_width ? fixed_width : line_width)
				    - width) / 2);
	  break;
	default:
	    Am_ERRORO("Bad alignment value " << h_align
		      << " in Am_H_ALIGN of " << self, self, Am_H_ALIGN);
	}
        top += (fixed_height ? fixed_height : height) + v_spacing;
        ++rank;
      }
      components.Next ();
    }
    left += (fixed_width ? fixed_width : line_width) + h_spacing;
    top = top_offset + indent;
  }
  return 0;
}

