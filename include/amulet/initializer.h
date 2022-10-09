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

#include <list>

class _OA_DL_CLASSIMPORT Am_Initializer
{
public:
  typedef std::list<Am_Initializer *> INITIALIZERLIST;
  typedef void Am_Initializer_Procedure();
  Am_Initializer(const char *this_name, Am_Initializer_Procedure *init_proc,
                 float priority, unsigned aCheckPoint = 1,
                 Am_Initializer_Procedure *cleanup_proc = nullptr);

  static void Do_Initialize();
  static void Do_Cleanup();

protected:
  static INITIALIZERLIST *am_initializer_list;

  const char *name;
  float priority;
  unsigned mCheckPoint;

  Am_Initializer_Procedure *init;
  Am_Initializer_Procedure *cleanup;
};
#endif
