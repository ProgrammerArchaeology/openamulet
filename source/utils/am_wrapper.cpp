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
#include <amulet/impl/types_wrapper.h>

// only in debug mode we need the registry features
#ifdef DEBUG
#include REGISTRY__H
#endif

//the default looks up the name, and if found, returns it.  Otherwise,
//returns Am_No_Object.  Ignores the item it is called on.
#ifdef DEBUG
Am_Value
Am_Wrapper::From_String(const char *aString) const
{
  // The dynamic cast will only succeed if the returned object really is an Am_Wrapper object
  Am_Wrapper *item = dynamic_cast<Am_Wrapper *>(
      const_cast<Am_Registered_Type *>(Am_Get_Named_Item(aString)));
  if (item) {
    item->Note_Reference();
    Am_Value v(item);
    return (v);
  } else {
    return (Am_No_Value);
  }
}
#else
Am_Value
Am_Wrapper::From_String(const char *aString) const
{
  return (Am_No_Value);
}
#endif
