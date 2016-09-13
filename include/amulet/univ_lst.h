/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

// This file declares lists for several data types
// as well as template macros for custom lists.
// *************************************************************************

#ifndef UNIV_LST_H
#define UNIV_LST_H

//#include <stdlib.h> //malloc, free, abs
//#include <string.h> //memset
//#include <assert.h> //assert

#include <am_inc.h>
#include <amulet/typedefs.hpp>
#include <amulet/types.h> //for type 'bool'

#include "univ_lst.hpp" //templates

// List of void*
AM_DECL_LIST(Ptr, void *, (0L))

#endif
