#ifndef OBJECT_CONSTR_H
#define OBJECT_CONSTR_H

#include "types_registered.h"

class _OA_DL_CLASSIMPORT Am_Constraint : public Am_Registered_Type
{
public:
  virtual bool Get(const Am_Slot &fetching_slot, Am_Value &value,
                   bool &changed) = 0;

  virtual void Invalidated(const Am_Slot &slot_invalidated,
                           Am_Constraint *invalidating_constraint,
                           const Am_Value &value) = 0;
  virtual void Changed(const Am_Slot &slot_changed,
                       Am_Constraint *changing_constraint,
                       const Am_Value &prev_value,
                       const Am_Value &new_value) = 0;
  virtual void Changed(const Am_Slot &slot_changed,
                       Am_Constraint *changing_constraint) = 0;
  virtual void Slot_Event(Am_Object_Context *oc, const Am_Slot &slot) = 0;

  virtual Am_Constraint *Get_Prototype() = 0;
  virtual bool Is_Instance_Of(Am_Constraint *proto) = 0;

  virtual Am_Constraint *Constraint_Added(const Am_Slot &adding_slot) = 0;
  virtual Am_Constraint *Dependency_Added(const Am_Slot &adding_slot) = 0;
  virtual void Constraint_Removed(const Am_Slot &removing_slot) = 0;
  virtual void Dependency_Removed(const Am_Slot &removing_slot) = 0;

  virtual bool Is_Overridden_By(const Am_Slot &slot,
                                Am_Constraint *competing_constraint) = 0;

  virtual Am_Constraint *Create(const Am_Slot &current_slot,
                                const Am_Slot &new_slot) = 0;
  virtual Am_Constraint *Copy(const Am_Slot &current_slot,
                              const Am_Slot &new_slot) = 0;
  virtual Am_ID_Tag ID() const = 0;
  virtual const char *Get_Name() = 0;
};

// Create a class for constraint ids.
const Am_Value_Type Am_CONSTRAINT = 0x3000;

#endif // OBJECT_CONSTR_H
