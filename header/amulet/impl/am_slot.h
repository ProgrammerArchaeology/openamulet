#ifndef AM_SLOT_H
#define AM_SLOT_H

#include "types_slot.h"
#include <amulet/am_value.hpp>

typedef void *Am_Constraint_Tag;
class Am_Slot_Data;
class Am_Object_Context;
class Am_Object_Advanced;
class Am_Constraint;
class Am_Value;

class _OA_DL_CLASSIMPORT Am_Slot
{
public:
  void Set(const Am_Value &new_value, Am_Constraint *cause);
  void Set_Current_Constraint(Am_Constraint *constraint);

  // Get the owner of the slot.
  Am_Object_Advanced Get_Owner() const;
  // Get the name of the slot.
  Am_Slot_Key Get_Key() const;
  // Get the type of the slot.
  Am_Value_Type Get_Type() const;

  // Get the value of the slot.
  const Am_Value &Get() const;

  // Slot messages.
  void Change(Am_Constraint *changing_constraint);
  void Event(Am_Object_Context *oc);
  void Invalidate(Am_Constraint *validating_constraint);
  void Validate();

  Am_Constraint_Tag Add_Constraint(Am_Constraint *new_constraint) const;
  Am_Constraint_Tag Add_Dependency(Am_Constraint *new_dependency) const;
  void Remove_Constraint(Am_Constraint_Tag constraint_tag) const;
  void Remove_Dependency(Am_Constraint_Tag dependency_tag) const;

  // Returns whether or not slot's value is inherited.
  bool Is_Inherited();
  void Make_Unique();
  bool Valid() { return data != (0L); }

  unsigned short Get_Demon_Bits() const;
  void Set_Demon_Bits(unsigned short bits);

  Am_Inherit_Rule Get_Inherit_Rule() const;
  void Set_Inherit_Rule(Am_Inherit_Rule rule);

  unsigned short Get_Type_Check() const;
  void Set_Type_Check(unsigned short type);

  bool Get_Read_Only() const;
  void Set_Read_Only(bool read_only);

  void Text_Inspect() const;

  Am_Slot() { data = (0L); }
  Am_Slot(Am_Slot_Data *in_data) { data = in_data; }

  bool operator==(const Am_Slot &test) const { return data == test.data; }
  bool operator!=(const Am_Slot &test) const { return data != test.data; }
  operator Am_Slot_Data *() const { return data; }

private:
  Am_Slot_Data *data;
};

#endif // AM_SLOT_H
