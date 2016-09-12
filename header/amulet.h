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

#ifndef AMULET_H
#define AMULET_H

// include all the defines for include filenames
#ifndef AM_INCLUDES_H
#include <am_inc.h>
#endif

// include most of the stuff everyone will need
#include <amulet/typedefs.hpp>
#include <amulet/am_io.h>
#include <amulet/am_value.hpp>
#include <amulet/types.h>

#include <amulet/value_list.h>
#include <amulet/standard_slots.h>

#include <amulet/gdefs.h>
#include <amulet/idefs.h>
#include <amulet/opal.h>
#include <amulet/inter.h>
#include <amulet/widgets.h>
#include <amulet/gesture.h>
#include <amulet/anim.h>
#include <amulet/misc.h>
#include <amulet/initializer.h>

#include <amulet/network.h>

// in debug mode we want to use the inspector
#ifdef DEBUG
#include <amulet/debugger.h>
#endif

#endif // AMULET_H
