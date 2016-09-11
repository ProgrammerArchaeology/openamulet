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
#include TYPEDEFS__HPP
#include AM_IO__H
#include AM_VALUE__H
#include TYPES__H

#include VALUE_LIST__H
#include STANDARD_SLOTS__H

#include GDEFS__H
#include IDEFS__H
#include OPAL__H
#include INTER__H
#include WIDGETS__H
#include GESTURE__H
#include ANIM__H
#include MISC__H
#include INITIALIZER__H

#ifdef AMULET_NETWORKS
	#include NETWORK__H
#endif

// in debug mode we want to use the inspector
#ifdef DEBUG
	#include DEBUGGER__H
#endif

#endif // AMULET_H
