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

// Some common Opal functions, methods, constraints

#include <am_inc.h>
#include AM_IO__H
#include <amulet/impl/types_logging.h>

#include <amulet/impl/types_method.h>
#include <amulet/impl/am_style.h>
#include <amulet/impl/am_drawonable.h>
#include <amulet/impl/gem_misc.h>
#include <amulet/impl/am_object.h>
#include <amulet/impl/slots.h>
#include <amulet/impl/opal_methods.h>
#include <amulet/impl/opal_objects.h>
#include <amulet/impl/opal_op.h>
#include <amulet/impl/opal_misc.h>
#include <amulet/opal_intnl.h>
#include FORMULA__H

//#include OPAL__H
//#include OPAL_ADVANCED__H


const char* Am_Check_Int_Type (const Am_Value& value)
{
  if (value.type == Am_INT || !value.Safe())
    return (0L);
  else {
   std::cerr << "** Amulet error, Expected int type in Object but got " << value
	 <<std::endl <<std::flush;
    return "int type check error";
  }
}


Am_Define_Formula (int, compute_depth)
{
  Am_Object owner = self.Get_Owner ();
  if (owner.Valid ()) {
    int depth = owner.Get (Am_OWNER_DEPTH);
    if (depth == -1)
      return -1;
    else
      return depth + 1;
  }
  else
    return -1;
}

Am_Define_Object_Formula (pass_window)
{
  Am_Object owner = self.Get_Owner ();
  if (owner.Valid() && (owner.Is_Instance_Of (Am_Graphical_Object) ||
        owner.Is_Instance_Of (Am_Window)))
    return owner.Get (Am_WINDOW);
  else
    return (0L);
}

Am_Define_Method (Am_Draw_Method, void, generic_mask,
          (Am_Object self, Am_Drawonable* drawonable,
           int x_offset, int y_offset))
{
  int left = (int)self.Get (Am_LEFT) + x_offset;
  int top = (int)self.Get (Am_TOP) + y_offset;
  int width = self.Get (Am_WIDTH);
  int height = self.Get (Am_HEIGHT);
  drawonable->Draw_Rectangle (Am_No_Style, Am_On_Bits, left, top, width,
                  height);
}

// a cheap way to translates coordinates to be inside of me that works
// for any object except scrolling groups
void am_translate_coord_to_me(Am_Object in_obj, Am_Object ref_obj,
			      int &x, int &y)
{
  if (ref_obj == in_obj) {
    return; //x, y are already OK
  }
  else  {
    Am_Object owner;
    owner = in_obj.Get_Owner();
    if (owner.Valid() && owner == ref_obj) { // cheap transformation
      x -= (int)in_obj.Get (Am_LEFT); //simple translate coords to the inside
      y -= (int)in_obj.Get (Am_TOP);  // of object
    }
    else //not the owner, use expensive transformation
      Am_Translate_Coordinates (ref_obj, x, y, in_obj, x, y);
  }
}

// Check whether the point is inside the object.  Ignores
//   covering (i.e., just checks whether point is inside the
//   object even if the object is covered.
// Note: The coordinate system of x and y is defined to be the coordinate
//  system of the ref_obj.
Am_Define_Method(Am_Point_In_Method, Am_Object, generic_point_in_obj,
         (const Am_Object& in_obj, int x, int y,
          const Am_Object& ref_obj)) {
  if ((bool)in_obj.Get (Am_VISIBLE)) {
    am_translate_coord_to_me(in_obj, ref_obj, x, y);
    if ((x < 0) || (y < 0)) return Am_No_Object;
    if ((x >= (int)in_obj.Get (Am_WIDTH)) ||
    (y >= (int)in_obj.Get (Am_HEIGHT)))
      return Am_No_Object;
    return in_obj;
  }
  else
    return Am_No_Object;
}

// if a group and not Am_PRETEND_TO_BE_LEAF, returns true, else returns false
bool am_is_group_and_not_pretending(Am_Object in_obj)
{
	if(Am_Is_Group_Or_Map(in_obj))
	{
		Am_Value v;
		v=in_obj.Peek(Am_PRETEND_TO_BE_LEAF);
		if(!v.Valid())
		{
			return(true);
		}
	}
	return(false);
}

// for use in objects that don't have parts
Am_Define_Method(Am_Point_In_Or_Self_Method, Am_Object, generic_point_in_part,
         (const Am_Object& in_obj, int x, int y,
          const Am_Object& ref_obj, bool want_self, bool want_groups))
{
  if (want_self && (want_groups || !am_is_group_and_not_pretending(in_obj))) {
    Am_Point_In_Method method;
    method = in_obj.Get(Am_POINT_IN_OBJ_METHOD);
    return method.Call (in_obj, x, y, ref_obj);
  }
  else return Am_No_Object;
}

// for use in objects that don't have leaves: return self if in it
Am_Define_Method(Am_Point_In_Or_Self_Method, Am_Object, generic_point_in_leaf,
         (const Am_Object& in_obj, int x, int y,
          const Am_Object& ref_obj,
          bool /* want_self */, bool want_groups)) {
  if (!want_groups && am_is_group_and_not_pretending(in_obj))
    return Am_No_Object;
  Am_Point_In_Method method;
  method = in_obj.Get(Am_POINT_IN_OBJ_METHOD);
  return method.Call (in_obj, x, y, ref_obj);
}

//-------------------------------------------------------------
// Initial-Author	: Robert M. Münch at: 28.05.98 09:43:08
// Description		: Converts a given point in the coordinate system of 'obj' to be in the coordinate system if its owner
// Pre-Condition	: Formular is stored in slot Am_TRANSLATE_COORDINATES_METHOD
// Post-Condition	: none
// Result			: none
//-------------------------------------------------------------
Am_Define_Method(Am_Translate_Coordinates_Method, void, generic_translate_coordinates, (const Am_Object& obj, const Am_Object& /*for_part*/, int in_x, int in_y, int& out_x, int& out_y))
{
	out_x = static_cast<int>(obj.Get(Am_LEFT)) + in_x;
	out_y = static_cast<int>(obj.Get(Am_TOP)) + in_y;

	return;
}
