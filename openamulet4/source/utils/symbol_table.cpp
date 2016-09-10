/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

extern "C"
{
	#include <stdlib.h>
}

#include <am_inc.h>
#include <amulet/symbol_table.h>

// Use a special high-speed memory-pool
#if defined(MEMORY) && defined(USE_SMARTHEAP)
	MEM_POOL Am_Symbol_Table::memory = 0;
#endif

#ifndef OA_VERSION
	Am_IMPL_MAP(CStr2Int, const char*, (0L), int, -1)
#endif
