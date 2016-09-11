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

#ifndef TYPES_H
#define TYPES_H

// include the include-file defines
#include <am_inc.h>
//#include AM_IO__H
// class std::ostream;

// am_value.hpp defines some constants, therefore we must guard
// against double definitions
#ifndef AM_VALUE_HPP
	#include AM_VALUE__H
#endif

#ifdef NEED_BOOL
	// Define a Boolean type because some c++ do not support the bool type, yet.
	// Hopefully, this will someday disappear.
	#ifdef _WIN32
		#pragma warning( disable : 4237 )
	#endif

	// dangerous as true is normaly defines as !=0 and not 1
	#define true  1
	#define false 0
	typedef int bool;
#endif // NEED_BOOL


#include "amulet/impl/types_basic.h"
#include "amulet/impl/types_slot.h"

#include "amulet/impl/types_logging.h"


// class Am_Registered_Type; 	//forward reference
// class Am_Wrapper; 			//forward reference
// class Am_Method_Wrapper; 	//forward reference
// class Am_String; 			//forward reference (see below)

#include "amulet/impl/types_registered.h"

#include "amulet/impl/types_method.h"

#include "amulet/impl/types_type_support.h"

#include "amulet/impl/types_enum.h"

#include "amulet/impl/types_pointer.h"

#include "amulet/impl/types_wrapper.h"

#include "amulet/impl/types_string.h"


#include "amulet/impl/types_misc.h"

#include "amulet/impl/types_load_save.h"


#endif
