#ifndef OPAL_POINT_IN_METHOD_H
#define OPAL_POINT_IN_METHOD_H

#include "types_method.h"

// Point in Part/Leaf Methods:
//  - Parameters are self and a point x,y.
//  - The point is in the coordinate system of the ref_obj
//  - if inside in_obj but NOT in part or leaf, then if want_self
//  		returns in_obj otherwise returns (nullptr)
//  - If want_groups is true, the finds the leaf-most element even if
//        it is a group.  If want_groups is false, then will not return a
//        group (if x,y is not over a "primitive" object, returns Am_No_Object)
//  - Returns the object if the point lies inside the object, false otherwise.
//  - Do not call Set or any other demon invoking operation in the method.
AM_DEFINE_METHOD_TYPE(Am_Point_In_Or_Self_Method, Am_Object,
                      (const Am_Object &in_obj, int x, int y,
                       const Am_Object &ref_obj, bool want_self,
                       bool want_groups))

#endif // OPAL_POINT_IN_METHOD_H
