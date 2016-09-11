/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <string.h>
#include <stdlib.h>

#include <am_inc.h>
#include DYNARRAY__H


#ifdef USE_SMARTHEAP
#include "dynmm_smartheap.cpp"
#else
#include "dynmm_default.cpp"
#endif
