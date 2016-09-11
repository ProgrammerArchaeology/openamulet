/* ****************************** -*-c++-*- *******************************/
/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#ifndef OPAL_H
#define OPAL_H

#include <am_inc.h>
#include "amulet/impl/object_misc.h"
#include "amulet/impl/types_enum.h"
#include "amulet/impl/types_load_save.h"

#include GDEFS__H
#include FORMULA__H

#include "amulet/impl/opal_objects.h"

#include "amulet/impl/opal_constraints.h"

#include "amulet/impl/am_alignment.h"

#include "amulet/impl/opal_misc.h"

#include "amulet/impl/opal_op.h"

#include "amulet/impl/opal_methods.h"

#include "amulet/impl/method_timer.h"

// Verify if an image can be loaded.  This will test load the image.  If it
// succeeds, the procedure will return true and the image may be used without
// error.  Otherwise, the procedure will return false and the image should not
// be used (reassign value to Am_No_Image_Array or another image).
_OA_DL_IMPORT extern bool Am_Test_Image_File (const Am_Object& screen,
				const Am_Image_Array& image);

#include "amulet/impl/opal_load_save.h"

#endif
