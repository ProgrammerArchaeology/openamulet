#ifndef METHOD_TRANS_COORD_H
#define METHOD_TRANS_COORD_H

// Am_Translate_Coordinates_Method:
//  - Stored in slot Am_TRANSLATE_COORDINATES_METHOD
//  - Given a point in the coordinate system of obj, converts it to
//    be in the coordinate system of its owner.  for_part is the
//    (optional) part of obj that is being translated for
//  - For almost all objects, it is x+obj.Get(Am_LEFT), y+obj.Get(Am_TOP)
//  - For scrolling groups, it is more complicated.  For windows,
//    uses the gem function.
AM_DEFINE_METHOD_TYPE(Am_Translate_Coordinates_Method, void,
                      (const Am_Object &obj, const Am_Object &for_part,
                       int in_x, int in_y, int &out_x, int &out_y))
#endif // METHOD_TRANS_COORD_H
