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

// This file contains exported interface to Gem: the base,
// machine-independent graphics layer.  Things defined in this file are
// not seen by Amulet users (e.g., they would use the Opal image and
// window objects instead of the Gem objects);  these objects are meant
// to be used by Gem users.


#ifndef GEM_H
#define GEM_H

#include <am_inc.h>

#include TYPES__H  			// to get the Value object
#include GDEFS__H  			// constants and definitions for graphics
#include IDEFS__H  			// constants and definitions for input
#include INTER__H           // supports multiple users
#include STANDARD_SLOTS__H  // supports multiple users

#ifdef OA_VERSION
#include <amulet/univ_map_oa.hpp>
#else
#include UNIV_MAP__H 		// for decl_map int2int
#endif

class Am_Drawonable;


//Global variable controlling multiple-click handling.  If 0, then no
//double-click processing.  Otherwise, is the inter-click wait time in
//milleseconds.  Default value = 250.
_OA_DL_IMPORT extern int Am_Double_Click_Time;

#include "amulet/impl/gem_event_handlers.h"

#include "amulet/impl/am_region.h"


class Am_Time;

#include "amulet/impl/am_drawonable.h"

#include "amulet/impl/am_am_drawonable.h"

// For key map
#ifdef OA_VERSION
	// defined in univ_map.cpp
	typedef	OpenAmulet::Map_Int2Int	Am_Map_int2int;
#else
	AM_DECL_MAP(int2int, int, int)
#endif

void Am_Init_Key_Map();

#include "amulet/impl/gem_misc.h"

#endif


