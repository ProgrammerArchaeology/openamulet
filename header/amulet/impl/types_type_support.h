#ifndef TYPES_TYPE_SUPPORT_H
#define TYPES_TYPE_SUPPORT_H

#include <am_inc.h>
#include AM_VALUE__H

// class std::ostream;

// This class is used to implement reading and writing functions for wrappers
// that are not derived from Am_Wrapper.  Presently, it is used for enumeration
// wrappers and for pointer wrappers.
class _OA_DL_CLASSIMPORT Am_Type_Support
{
public:
  // Prints to a stream a short debugging statement.  Used for displaying for
  // a user, not for permanent storage.
  virtual void Print(std::ostream &os, const Am_Value &value) const;

  void Println(const Am_Value &value) const;

  //Returns the string if easy and doesn't require any allocation.
  //Otherwise, returns (0L).
  virtual const char *To_String(const Am_Value &v) const;

  // Reads a string, potentially provided by a user and converts to its own
  // type.  Returns Am_No_Value when there is an error.
  virtual Am_Value From_String(const char *string) const;

  virtual ~Am_Type_Support() {} // virtual destructor
};

// Simple wrapper types can be registered with the system so that their values
// may be printed in the debugger and other places.
extern void Am_Register_Support(Am_Value_Type type, Am_Type_Support *support);
extern Am_Type_Support *Am_Find_Support(Am_Value_Type type);

// Special ID generator useful for registering the support class.
extern Am_ID_Tag Am_Get_Unique_ID_Tag(const char *type_name,
                                      Am_Type_Support *support, Am_ID_Tag base);

#endif //TYPES_TYPE_SUPPORT_H
