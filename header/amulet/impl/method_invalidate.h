#ifndef METHOD_INVALIDATE_H
#define METHOD_INVALIDATE_H

#include "types_method.h"

// Creation and use of Invalidation Methods:
//  - The method is stored in the Am_INVALID_METHOD slot
//  - Method is called up from parts to their corresponding owner
//  - Parameters are the self object, the part which has changed,
//    and the rectangular region to invalidate
//  - The invalid region is converted to the coordinates of the object's owner
//    and passed to the owner of the next object.
//  - Do not call Set or any other demon invoking operation in the method.
AM_DEFINE_METHOD_TYPE(Am_Invalid_Method, void,
		      (Am_Object self, Am_Object which_part, int left,
		       int top, int width, int height))

extern void Am_Invalidate(Am_Object owner, Am_Object which_part,
			   int left, int top, int width, int height);

#endif // METHOD_INVALIDATE_H
