/* ****************************** -*-c++-*- *******************************/
/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#ifndef STANDARD_SLOTS_H
#define STANDARD_SLOTS_H

#include <am_inc.h>

#include <amulet/object.h>
#include "amulet/impl/types_logging.h"
#include "amulet/impl/types_enum.h"

// Object slots (0-99)
// see object.h

const Am_Slot_Key Am_No_Slot_Key = 0;

//use this wrapper to get the slot key to have the right value type.
//For example,  Am_Value_List().Add(Am_Am_Slot_Key(Am_LEFT))
// then, should print with the names
class _OA_DL_CLASSIMPORT Am_Am_Slot_Key
{
public:
  Am_Am_Slot_Key(const Am_Value &in_value)
  {
    if (in_value.type != Am_ZERO && in_value.type != Am_INT &&
        in_value.type != Am_LONG && in_value.type != Am_SLOT_KEY_TYPE) {
      DERR("** Tried to set a Am_Am_Slot_Key enum with a non Am_Am_Slot_Key "
           "wrapper: "
           << in_value << std::endl);
      Am_Error();
    }
    value = (Am_Slot_Key)in_value.value.long_value;
  }
  Am_Am_Slot_Key(Am_Slot_Key in_value = 0) { value = in_value; }
  Am_Am_Slot_Key &operator=(Am_Slot_Key in_value)
  {
    value = in_value;
    return *this;
  }
  Am_Am_Slot_Key &operator=(const Am_Value &in_value)
  {
    if (in_value.type != Am_ZERO && in_value.type != Am_INT &&
        in_value.type != Am_LONG && in_value.type != Am_SLOT_KEY_TYPE) {
      DERR("** Tried to set a Am_Am_Slot_Key enum with a non Am_Am_Slot_Key "
           "wrapper: "
           << in_value << std::endl);
      Am_Error();
    }
    value = (Am_Slot_Key)in_value.value.long_value;
    return *this;
  }
  operator Am_Value() const { return Am_Value((long)value, Am_SLOT_KEY_TYPE); }
  operator Am_Slot_Key() const { return value; }
  bool operator==(Am_Slot_Key test_value) const { return value == test_value; }
  bool operator==(Am_Am_Slot_Key test_value) const
  {
    return value == test_value.value;
  }
  bool operator!=(Am_Slot_Key test_value) const { return value != test_value; }
  bool operator!=(Am_Am_Slot_Key test_value) const
  {
    return value != test_value.value;
  }
  static bool Test(const Am_Value &value)
  {
    return value.type == Am_SLOT_KEY_TYPE;
  }
  static Am_Value_Type Type_ID() { return Am_SLOT_KEY_TYPE; }
  /* Printing functions */
  const char *To_String() const
  {
    return Am_Enum_To_String_Helper(Am_SLOT_KEY_TYPE, (long)value);
  }
  void Print(std::ostream &out) const;
  void Println() const;

  Am_Slot_Key value;
};
_OA_DL_IMPORT extern std::ostream &operator<<(std::ostream &os,
                                              const Am_Am_Slot_Key &value);

#include "amulet/impl/slots.h"

#include "amulet/impl/slots_registry.h"

#endif
