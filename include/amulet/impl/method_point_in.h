#ifndef METHOD_POINT_IN_H
#define METHOD_POINT_IN_H

// Point in Object Methods:
//  - Stored in slot Am_POINT_IN_OBJ_METHOD
//  - Parameters are self and a point x,y.
//  - The point is in the coordinate system of the ref_obj
//  - Returns the object if the point lies inside the object, false otherwise.
//  - Do not call Set or any other demon invoking operation in the method.
AM_DEFINE_METHOD_TYPE(Am_Point_In_Method, Am_Object,
                      (const Am_Object &in_obj, int x, int y,
                       const Am_Object &ref_obj))

#endif // METHOD_POINT_IN_H
