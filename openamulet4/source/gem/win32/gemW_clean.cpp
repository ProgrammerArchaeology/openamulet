#ifdef USE_WINCLEANER
// this entire file is unnecessary on Win32 platforms (which are
// all we support).
//
// See gemW_clean.h for discussion.
//
 
#include <windows.h>

#include <amulet/univ_map.h>
#include <amulet/am_io.h>

#include "gemW_32.h"
#include "gemW_misc.h"
#include "gemW_clean.h"

#if defined(_WIN32)
#define GetFreeSystemResources(A) 90
#endif

Am_WinCleaner::Am_WinCleaner ()
  : m_map (23)
{
#if !defined(_WIN32)
	m_start = GetFreeSystemResources(GFSR_GDIRESOURCES);
	if (m_start > Am_CLEAN_STARTLEVEL) m_start = Am_CLEAN_STARTLEVEL;
	m_stop = m_start + (Am_CLEAN_STOPLEVEL - Am_CLEAN_STARTLEVEL);
#endif
}

Am_WinCleaner::~Am_WinCleaner ()
{
	Am_MapIterator_Ptr2Ptr next (m_map);
	
	for (Am_WinCleanFunc* func = (Am_WinCleanFunc*)next(); func; func = (Am_WinCleanFunc*)next())
		func(next.Key(), Am_CLEAN_DELETE); //item stays in map, so we don't need next.Reset()!

	m_map.Clear();
}

BOOL Am_WinCleaner::CheckResources ()
{
#if defined(_WIN32)
	static Am_MapIterator_Ptr2Ptr next;
	next.Init (m_map);

	int nCountHasRes = 0;

	for (Am_WinCleanFunc* func = (Am_WinCleanFunc*)next(); func;
			func = (Am_WinCleanFunc*)next())
		if (func(next.Key(), Am_CLEAN_HASRES))
			nCountHasRes++;

	if (nCountHasRes > Am_CLEAN_STARTCOUNT)
		for (Am_WinCleanFunc* func = (Am_WinCleanFunc*)next(); func;
				func = (Am_WinCleanFunc*)next())
			if (!func(next.Key(), Am_CLEAN_CHECKUSED))
				if (nCountHasRes > Am_CLEAN_STARTCOUNT &&
						func(next.Key(), Am_CLEAN_FREERES))
					nCountHasRes--;

	return FALSE;
#else
	if (GetFreeSystemResources(GFSR_GDIRESOURCES) < m_start) {
		static Am_MapIterator_Ptr2Ptr next;
		next.Init (m_map);
		
		for (Am_WinCleanFunc* func = (Am_WinCleanFunc*)next(); func;
				func = (Am_WinCleanFunc*)next())
			if (!func(next.Key(), Am_CLEAN_CHECKUSED) &&
					func(next.Key(), Am_CLEAN_FREERES))
				//returns TRUE if further cleaning needed
				return GetFreeSystemResources(GFSR_GDIRESOURCES) < m_start;
	}
	return FALSE;
#endif
}

#pragma init_seg(lib)
Am_WinCleaner *g_cleaner; //the one and only cleaner
int cleaner_counter::count;

#endif // USE_WINCLEANER
