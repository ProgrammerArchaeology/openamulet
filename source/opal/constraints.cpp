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
#include "amulet/opal_inlines.h"

#include <amulet/impl/types_logging.h>
#include <amulet/impl/am_object.h>
#include <amulet/impl/opal_misc.h>
#include <amulet/impl/opal_constraints.h>
#include <amulet/impl/slots.h>
#include <amulet/value_list.h>
#include <amulet/formula.h>

/*************************************************
 ** Standard exported constraints, argumentless **
 *************************************************/

// Put in an Am_HEIGHT slot. Sets the height to the height of the object's
// owner, minus this object's Am_TOP, minus Am_BOTTOM_OFFSET
Am_Define_Formula(int, Am_Fill_To_Bottom)
{
  Am_Object owner = self.Get_Owner();
  if (!owner.Valid())
    return 0;
  return (int)(owner.Get(Am_HEIGHT)) - (int)(self.Get(Am_TOP)) -
         (int)(owner.Get(Am_BOTTOM_OFFSET));
}

// Put in an Am_WIDTH slot: see fill_to_bottom
Am_Define_Formula(int, Am_Fill_To_Right)
{
  Am_Object owner = self.Get_Owner();
  if (!owner.Valid())
    return 0;
  return (int)(owner.Get(Am_WIDTH)) - (int)(self.Get(Am_LEFT)) -
         (int)(owner.Get(Am_RIGHT_OFFSET));
}

Am_Define_Formula(int, Am_Fill_To_Rest_Of_Width)
{
  Am_Object owner = self.Get_Owner();
  Am_Value_List parts = owner.Get(Am_GRAPHICAL_PARTS);
  int width = 0;
  Am_Object part;
  for (parts.Start(); !parts.Last(); parts.Next()) {
    part = parts.Get();
    if (part != self)
      width += (int)part.Get(Am_WIDTH);
  }
  return (int)owner.Get(Am_WIDTH) -
         (width + (int)owner.Get(Am_LEFT_OFFSET) +
          (int)owner.Get(Am_RIGHT_OFFSET) +
          (int)owner.Get(Am_H_SPACING) * (parts.Length() - 1));
}

Am_Define_Formula(int, Am_Fill_To_Rest_Of_Height)
{
  Am_Object owner = self.Get_Owner();
  Am_Value_List parts = owner.Get(Am_GRAPHICAL_PARTS);
  int height = 0;
  Am_Object part;
  for (parts.Start(); !parts.Last(); parts.Next()) {
    part = parts.Get();
    if (part != self)
      height += (int)part.Get(Am_HEIGHT);
  }
  return (int)owner.Get(Am_HEIGHT) -
         (height + (int)owner.Get(Am_TOP_OFFSET) +
          (int)owner.Get(Am_BOTTOM_OFFSET) +
          (int)owner.Get(Am_V_SPACING) * (parts.Length() - 1));
}

Am_Define_Formula(int, Am_Width_Of_Parts)
{
  int max_x = 0, comp_right;
  Am_Value_List components;
  Am_Object comp;
  components = self.Get(Am_GRAPHICAL_PARTS);
  for (components.Start(); !components.Last(); components.Next()) {
    comp = components.Get();
    if (comp.Get(Am_VISIBLE).Valid()) {
      // compute how much of the component extends right of the origin
      comp_right = ((int)(comp.Get(Am_LEFT)) + (int)(comp.Get(Am_WIDTH)));
      max_x = imax(max_x, comp_right);
    }
  }
  if (self.Peek(Am_RIGHT_OFFSET, Am_NO_DEPENDENCY).Exists())
    max_x += (int)self.Get(Am_RIGHT_OFFSET);
  return max_x; // always >=0 since it's initialized to 0
}

Am_Define_Formula(int, Am_Height_Of_Parts)
{
  int max_y = 0, comp_bottom;
  Am_Value_List components;
  Am_Object comp;
  components = self.Get(Am_GRAPHICAL_PARTS);
  for (components.Start(); !components.Last(); components.Next()) {
    comp = components.Get();
    if (comp.Get(Am_VISIBLE).Valid()) {
      // compute how much of the component extends below the origin
      comp_bottom = ((int)(comp.Get(Am_TOP)) + (int)(comp.Get(Am_HEIGHT)));
      max_y = imax(max_y, comp_bottom);
    }
  }
  if (self.Peek(Am_BOTTOM_OFFSET, Am_NO_DEPENDENCY).Exists())
    max_y += (int)self.Get(Am_BOTTOM_OFFSET);
  return max_y; // always >=0 since it's initialized to 0
}

Am_Define_Formula(int, Am_Center_X_Is_Center_Of)
{
  int my_width = self.Get(Am_WIDTH);
  Am_Object center_x_obj;
  center_x_obj = self.Get(Am_CENTER_X_OBJ);
  if (center_x_obj.Valid()) {
    int center_x_obj_left = center_x_obj.Get(Am_LEFT);
    int center_x_obj_width = center_x_obj.Get(Am_WIDTH);
    return center_x_obj_left + ((center_x_obj_width - my_width) / 2);
  } else {
    return 0;
  }
}

Am_Define_Formula(int, Am_Center_Y_Is_Center_Of)
{
  int my_height = self.Get(Am_HEIGHT);
  Am_Object center_y_obj;
  center_y_obj = self.Get(Am_CENTER_Y_OBJ);
  if (center_y_obj.Valid()) {
    int center_y_obj_top = center_y_obj.Get(Am_TOP);
    int center_y_obj_height = center_y_obj.Get(Am_HEIGHT);
    return center_y_obj_top + ((center_y_obj_height - my_height) / 2);
  } else {
    return 0;
  }
}

Am_Define_Formula(int, Am_Center_X_Is_Center_Of_Owner)
{
  int my_width = self.Get(Am_WIDTH);
  Am_Object center_x_obj = self.Get_Owner();
  if (center_x_obj.Valid()) {
    int center_x_obj_width = center_x_obj.Get(Am_WIDTH);
    return (center_x_obj_width - my_width) / 2;
  } else {
    return 0;
  }
}

Am_Define_Formula(int, Am_Center_Y_Is_Center_Of_Owner)
{
  int my_height = self.Get(Am_HEIGHT);
  Am_Object center_y_obj = self.Get_Owner();
  if (center_y_obj.Valid()) {
    int center_y_obj_height = center_y_obj.Get(Am_HEIGHT);
    return (center_y_obj_height - my_height) / 2;
  } else {
    return 0;
  }
}

Am_Define_Formula(int, Am_Right_Is_Right_Of_Owner)
{
  int my_width = self.Get(Am_WIDTH);
  Am_Object owner = self.Get_Owner();
  if (owner) {
    int owner_width = owner.Get(Am_WIDTH);
    int right_offset = owner.Get(Am_RIGHT_OFFSET);
    return owner_width - my_width - right_offset;
  } else {
    return 0;
  }
}

Am_Define_Formula(int, Am_Bottom_Is_Bottom_Of_Owner)
{
  int my_height = self.Get(Am_HEIGHT);
  Am_Object owner = self.Get_Owner();
  if (owner) {
    int owner_height = owner.Get(Am_HEIGHT);
    int bottom_offset = owner.Get(Am_BOTTOM_OFFSET);
    return owner_height - my_height - bottom_offset;
  } else {
    return 0;
  }
}
