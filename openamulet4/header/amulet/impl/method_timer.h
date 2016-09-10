#include "types_method.h"

//Type of method that the Am_Register_Timer function will call.
//Am_Time defined in gdefs.h
Am_Define_Method_Type(Am_Timer_Method, void,
		      (Am_Object obj, const Am_Time& elapsed_time))
