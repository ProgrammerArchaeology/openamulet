// Why this here? there's amulet/impl/types_wrapper.h

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
#ifndef AM_WRAPPER_HPP
#define AM_WRAPPER_HPP

#ifndef REGISTERED_TYPE_HPP
#include REGISTERED_TYPE__HPP
#endif

// The Am_Wrapper class. Objects of this type can be directly stored in an
// OpenAmulet object slot.	The class implements the reference-counting feature
// used by the heavy-weight objects of OA. Classes derived from this class
// therefore don't have to care about the reference-counting
class _OA_DL_CLASSIMPORT Am_Wrapper : public Am_Registered_Type
{
public:
  // CREATORS
  Am_Wrapper() : refs(1) { ; }

  Am_Wrapper(const Am_Wrapper &) : refs(1) { ; }

  virtual ~Am_Wrapper() { ; }

  // OPERATORS
  virtual bool operator==(const Am_Wrapper &test_value) = 0;

  // MANIPULATORS
  // Return a unique copy of the data.
  virtual Am_Wrapper *Make_Unique() = 0;

  //:Note that the wrapped object is being newly referenced by someone
  void Note_Reference() { ++refs; }

  //:A reference to this object is being released. The object will
  // delete itself if the reference is 0.
  void Release()
  {
    --refs;

    if (refs == 0) {
      delete (this);
    }
  }

  // ACCESSORS
  // Returns unique ID of the leave class this class is a base class for
  virtual Am_ID_Tag ID() const = 0;

  virtual Am_Value From_String(const char *string) const;

  //:Return the reference count.
  unsigned Ref_Count() const { return (refs); }

  //:Is this object only referenced by one and only one other object?
  bool Is_Unique() const { return (refs == 1); }

  bool Is_Zero() const { return (refs == 0); }

private:
  // DATA MEMBERS
  unsigned refs;
};

#endif
