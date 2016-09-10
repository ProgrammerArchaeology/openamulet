//
// ------------------------------------------------------------------------
// 		The OpenAmulet User Interface Development Environment
// ------------------------------------------------------------------------
// This code is based on the Amulet project of Carnegie Mellon University,
// and has been placed in the public domain.  If you are using this code
// or any part of OpenAmulet, please contact amulecs.cmu.edu to be put
// on the mailing list or have a look at:
// http://www.openip.org
//

#ifndef OBJECT_H
#define OBJECT_H

#include <am_inc.h>

#include "amulet/impl/types_logging.h"
#include "amulet/impl/types_wrapper.h"
#include "amulet/impl/am_object.h"

_OA_DL_IMPORT extern void Am_Print_Key (std::ostream& os, Am_Slot_Key key);

#include "amulet/impl/am_instance_iterator.h"

#include "amulet/impl/am_slot_iterator.h"

#include "amulet/impl/am_part_iterator.h"

#include "amulet/impl/am_dependency_iterator.h"

#include "amulet/impl/types_method.h"

#include "amulet/impl/object_misc.h"

#endif
