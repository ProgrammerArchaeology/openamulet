#ifndef TYPES_REGISTERED_H
#define TYPES_REGISTERED_H

#include <am_inc.h>
#include AM_VALUE__H

// The Am_Registered_Type class.  Classes derived from this type have runtime
// type ID's, and pointers to them can be registered in the name registry.

// class std::ostream;

class _OA_DL_CLASSIMPORT Am_Registered_Type
{
public:
  // returns unique tag for each derived type.
  virtual Am_ID_Tag ID() const = 0;

  //printing and reading for debugging
  virtual const char *To_String() const;
  virtual void Print(std::ostream &out) const;

  void Println() const;

  virtual ~Am_Registered_Type() {} // destructor should be virtual
};

#endif //TYPES_REGISTERED_H
