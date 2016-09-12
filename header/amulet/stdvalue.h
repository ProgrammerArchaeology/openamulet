/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#ifndef STD_VALUE_H
#define STD_VALUE_H

#include <am_inc.h>

#include "amulet/impl/types_wrapper.h"

class _OA_DL_CLASSIMPORT Am_Double_Data : public Am_Wrapper {
  AM_WRAPPER_DATA_DECL (Am_Double)
 public:
  bool operator== (const Am_Double_Data& test_value) const;
  operator double () const;
  Am_Double_Data (double value);
  void Print (std::ostream& out) const;
 private:
  double value;
};

class Am_String_Data : public Am_Wrapper {
  AM_WRAPPER_DATA_DECL (Am_String)
 public:
  bool operator== (const Am_String_Data& test_value) const;
  bool operator== (const char* test_value) const;
  operator const char* () const;
  operator char* () const;
  Am_String_Data (const char* value, bool copy = true);
  ~Am_String_Data ();
  void Print (std::ostream& out) const;
  const char * To_String() const;
  virtual Am_Value From_String(const char * string) const;

 private:
  const char* value;
};

#endif
