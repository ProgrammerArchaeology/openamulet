#ifndef OPAL_DRAW_METHOD_H
#define OPAL_DRAW_METHOD_H

#include "types_method.h"
#include "types_logging.h"
#include "slots.h"
#include "slots_opal.h"

class Am_Drawonable;

// Creation and use of Draw Methods:
//  - Store method in Am_DRAW_METHOD slot.
//  - Method gets called by owner object when it gets drawn.
//  - Self points to own data, drawonable is the window being drawn in,
//    x and y offsets are for converting the object's position to window
//    coordinates.
//  - Do not call Set or any other demon invoking operation in the method.
AM_DEFINE_METHOD_TYPE(Am_Draw_Method, void,
                      (Am_Object self, Am_Drawonable *drawonable, int x_offset,
                       int y_offset))

_OA_DL_IMPORT extern void Am_Draw(Am_Object object, Am_Drawonable *drawonable,
                                  int x_offset, int y_offset);

#endif /* OPAL_DRAW_METHOD_H */
