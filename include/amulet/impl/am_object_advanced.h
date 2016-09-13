#ifndef OBJECT_ADVANCED_PURE_H
#define OBJECT_ADVANCED_PURE_H

#include "am_object.h"
#include "am_slot.h"
#include "opal_demons.h"

class Am_Constraint;
class Am_Demon_Set;
class Am_Demon_Queue;

class _OA_DL_CLASSIMPORT Am_Object_Advanced : public Am_Object
{
public:
  Am_Object_Advanced();
  Am_Object_Advanced(Am_Object_Data *in_data);

  Am_Slot Get_Slot(Am_Slot_Key key) const;

  Am_Slot Get_Owner_Slot() const;
  Am_Slot Get_Part_Slot() const;

  // Returns the object in which a slot is defined.
  Am_Object_Advanced Get_Slot_Locale(Am_Slot_Key key) const;

  Am_Demon_Set Get_Demons() const;
  void Set_Demons(const Am_Demon_Set &methods);

  Am_Demon_Queue Get_Queue() const;
  void Set_Queue(const Am_Demon_Queue &queue);

  unsigned short Get_Default_Demon_Bits() const;
  void Set_Default_Demon_Bits(unsigned short bits);
  unsigned short Get_Demon_Mask() const;
  void Set_Demon_Mask(unsigned short mask);

  Am_Inherit_Rule Get_Default_Inherit_Rule() const;
  void Set_Default_Inherit_Rule(Am_Inherit_Rule rule);

  void Disinherit_Slot(Am_Slot_Key key);

  void Invoke_Demons(bool active);
  bool Demon_Invocation_Active() const;

  void Print_Name_And_Data(std::ostream &os) const;

  // expose your rep, darnit
  Am_Object_Data *Get_Data() const { return data; }

  static Am_Constraint_Context *Swap_Context(Am_Constraint_Context *new_cc)
  {
    Am_Constraint_Context *old_cc = cc;
    cc = new_cc;
    return old_cc;
  }
  static Am_Constraint_Context *Get_Context() { return cc; }
};
// Prints out an identifying name for the object to the output stream
// along with the data address
_OA_DL_IMPORT extern std::ostream &operator<<(std::ostream &os,
                                              const Am_Object_Advanced &object);

typedef unsigned Am_Set_Reason;

#endif //OBJECT_ADVANCED_PURE_H
