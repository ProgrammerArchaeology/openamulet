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

#include <am_inc.h>

#include <amulet/am_io.h>
#include <amulet/impl/types_logging.h>
#include <amulet/impl/types_registered.h>

// only in debug mode we need the registry features
#ifdef DEBUG
#include REGISTRY__H
#else
//	#ifndef REGISTERED_TYPE_HPP
//		#include REGISTERED_TYPE__HPP
//	#endif
#endif

const char *
Am_Registered_Type::To_String() const
{
#ifdef DEBUG
  const char *name = Am_Get_Name_Of_Item(this);
  return (name);
#else
  return (0L);
#endif
}

void
Am_Registered_Type::Print(std::ostream &out) const
{
#ifdef DEBUG
  const char *name = Am_Get_Name_Of_Item(this);
  if (name) {
    out << name;
    return;
  }
#endif

  //standard print if no name or not debugging
  Am_Standard_Print(out, (unsigned long)this, ID());
}

void
Am_Registered_Type::Println() const
{
  Print(std::cout);
  std::cout << std::endl;
  return;
}
