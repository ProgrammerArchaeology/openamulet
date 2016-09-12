//
// ------------------------------------------------------------------------
// 		The OpenAmulet User Interface Development Environment
// ------------------------------------------------------------------------
// This code is based on the Amulet project of Carnegie Mellon University,
// and has been placed in the public domain.  If you are using this code
// or any part of OpenAmulet, please contact amulet@cs.cmu.edu to be put
// on the mailing list or have a look at:
// http:// www.openip.org
//

#ifndef INITIALIZERS_H
#define INITIALIZERS_H

#ifndef INITIALIZER_H
#define INITIALIZER_H
#endif

#if _MSC_VER >= 1000 // only for VC++ 5 or later
#pragma once         // include header only once
#endif

//-----------------------------------------------------------------------------------------
// Copyright:
//
// Initial-Author:
//		Brad Meyrs
//		Robert M. Münch at: 09.05.98 13:42:13
//
// Synopsis:
//		Start text here
//
// Description:
// 		Support for initializing the system in a reliable way
//   	Each file should have a global variable like the following:
//
//		Am_Initializer* buttons_init = new Am_Initializer("Buttons", Am_Buttons_Initialize, 10.1, buttons_cleanup_proc);
//
//		All objects initialized by Am_Initializer are stored in a list
//		and this list is sorted on the priority. The priority is like a version
//		number and lower-version == older-versions, are stored at the
//		beginning of the list. To support this sorting, version numbers are reversed
//		1.10 -> 10.1, 1.20 -> 20.1
//   	and the cleanup is optional but should be give
//
// Files:
//		This file: initializer.h
//		Start text here
//
// Version-Control-Information:
//		$Id: initializer.h,v 1.1.1.2 2005/11/09 18:43:55 mitosys Exp $
//
// See Also:
//		Start text here
//
// Bugs:
//		None
//
// History:
//		Start text here
//
//-----------------------------------------------------------------------------------------

//-------------------------------------------------------------
// system include files
//-------------------------------------------------------------
#ifndef __SGI_STL_LIST_H
#include <list>
#endif

//-------------------------------------------------------------
// application include files
//-------------------------------------------------------------

//-------------------------------------------------------------
// Re-enables warnings
//-------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning(default : 4018) // < signed/unsigned
#pragma warning(default : 4201) // nameless union
#pragma warning(default : 4214) // non 'int' bitfields
#pragma warning(default : 4663) // c++ language change
#endif

//-------------------------------------------------------------
// external functions
//-------------------------------------------------------------

//-------------------------------------------------------------
// constants / statics / typedefs
//-------------------------------------------------------------

//-------------------------------------------------------------
// forward declarations / definitions
//-------------------------------------------------------------

//-------------------------------------------------------------
// class definition/implementation
//-------------------------------------------------------------

class _OA_DL_CLASSIMPORT Am_Initializer
{
public:
  // CREATORS
  typedef std::list<Am_Initializer *> INITIALIZERLIST;
  typedef void Am_Initializer_Procedure();
  Am_Initializer(char *this_name, Am_Initializer_Procedure *init_proc,
                 float priority, unsigned aCheckPoint = 1,
                 Am_Initializer_Procedure *cleanup_proc = 0);
  //			~Am_Initializer();

  // MANIPULATORS
  // ACCESSORS
  static void Do_Initialize();
  static void Do_Cleanup();
  // DATA MEMBERS

protected:
  // CREATORS
  // MANIPULATORS
  // ACCESSORS
  // DATA MEMBERS
  static INITIALIZERLIST *am_initializer_list;

  char *name;
  float priority;
  unsigned mCheckPoint;

  Am_Initializer_Procedure *init;
  Am_Initializer_Procedure *cleanup;
};

//-------------------------------------------------------------
// non-member functions
//-------------------------------------------------------------
#endif
