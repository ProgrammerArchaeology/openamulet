#include "method_draw.h"
#include "method_trans_coord.h"
#include "method_invalidate.h"
#include "method_object.h"
#include "method_point_in.h"
#include "method_point_in_or_self.h"

//methods for the Am_DESTROY_WINDOW_METHOD slot of windows
_OA_DL_IMPORT extern Am_Object_Method Am_Window_Hide_Method; //set visible to false
_OA_DL_IMPORT extern Am_Object_Method Am_Window_Destroy_And_Exit_Method;  //exit MEL
_OA_DL_IMPORT extern Am_Object_Method Am_Default_Window_Destroy_Method;
         //destroy and exit MEL if no windows are left

