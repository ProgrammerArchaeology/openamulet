/* ****************************** -*-c++-*- *******************************/
/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#ifndef IMPL_H
#define IMPL_H

#include <am_inc.h>
//#include <amulet/object_advanced.h>

#include "amulet/impl/am_object_advanced.h"
#include "amulet/impl/am_demon_queue.h"
#include "amulet/impl/am_demon_set.h"
//#include "amulet/impl/object_constr.h"
#include "amulet/impl/am_constraint_context.h"

#include <amulet/dynarray.h>

// disable a VC++ 6 warning about operator new/delete for QItem
#pragma warning(disable : 4291)

class CItem;

class QItem
{
#ifdef MEMORY
public:
  void *operator new(size_t) { return memory.New(); }
  void operator delete(void *ptr, size_t) { memory.Delete(ptr); }
  static Dyn_Memory_Manager memory;
#endif
public:
  QItem(Am_Slot_Demon *demon, unsigned short which_bit, const Am_Slot &param);
  ~QItem();

  Am_Slot_Demon *demon;
  Am_Slot_Data *param;
  unsigned short which_bit;
  QItem *next;
};

#define DEMONS_ACTIVE 0x8000

class Am_Demon_Queue_Data
{
public:
  Am_Demon_Queue_Data()
  {
    refs = 1;
    head = (0L);
    tail = (0L);
    invoke_stack = 0;
    active = true;
  }
  ~Am_Demon_Queue_Data()
  {
    QItem *curr;
    QItem *next;
    for (curr = head; curr; curr = next) {
      next = curr->next;
      delete curr;
    }
  }
  void Note_Reference() { ++refs; }
  void Release()
  {
    if (!--refs)
      delete this;
  }
  void Enqueue(QItem *item)
  {
    item->next = (0L);
    if (tail)
      tail->next = item;
    else
      head = item;
    tail = item;
  }

  int refs;
  QItem *head;
  QItem *tail;
  unsigned short invoke_stack;
  bool active;
};

class Slot_Demon_Holder
{
public:
  Am_Slot_Demon *demon;
  Am_Demon_Protocol protocol;
  unsigned short which_bit;
};

class Type_Holder
{
public:
  Am_Type_Check *func;
};

class Am_Demon_Set_Data
{
public:
  Am_Demon_Set_Data();
  Am_Demon_Set_Data(Am_Demon_Set_Data *proto)
  {
    refs = 1;
    create_demon = proto->create_demon;
    copy_demon = proto->copy_demon;
    destroy_demon = proto->destroy_demon;
    change_owner_demon = proto->change_owner_demon;
    add_part_demon = proto->add_part_demon;
    change_length = proto->change_length;
    invalid_length = proto->invalid_length;
    int i;
    for (i = 0; i < change_length; ++i)
      change_demons[i] = proto->change_demons[i];
    for (i = 0; i < invalid_length; ++i)
      invalid_demons[i] = proto->invalid_demons[i];
    max_type_check = proto->max_type_check;
    if (max_type_check) {
      type_check_list = new Type_Holder[max_type_check];
      for (i = 0; i < max_type_check; ++i)
        type_check_list[i] = proto->type_check_list[i];
    } else
      type_check_list = (0L);
  }
  ~Am_Demon_Set_Data() { delete[] type_check_list; }
  void Note_Reference() { ++refs; }
  void Release()
  {
    if (!--refs)
      delete this;
  }

  int refs;
  Am_Object_Demon *create_demon;
  Am_Object_Demon *copy_demon;
  Am_Object_Demon *destroy_demon;
  Am_Part_Demon *change_owner_demon;
  Am_Part_Demon *add_part_demon;
  unsigned short change_length;
  unsigned short invalid_length;
  Slot_Demon_Holder change_demons[5];
  Slot_Demon_Holder invalid_demons[5];
  unsigned short max_type_check;
  Type_Holder *type_check_list;
};

class Am_Object_Context
{
public:
  Am_Object_Context(bool in_inherited) { is_inherited = in_inherited; }
  bool is_inherited;
};

class am_CList
{
public:
  am_CList();

  CItem *Add_Dep(Am_Constraint *item);
  Am_Constraint *Remove_Dep(CItem *item);
  CItem *Add_Con(Am_Constraint *item);
  Am_Constraint *Remove_Con(CItem *item);
  void Add_Inv(CItem *item);
  void Add_Update(CItem *item);
  void Remove_Inv(CItem *item);
  void Clear() { head = (0L); }
  CItem *Find(Am_Constraint *item);
  CItem *Pop();
  void Validate(const Am_Slot &validating_slot);
  void Invalidate(const Am_Slot &slot_invalidated,
                  Am_Constraint *invalidating_constraint,
                  const Am_Value &value);
  void Change(const Am_Slot &slot_invalidated,
              Am_Constraint *invalidating_constraint,
              const Am_Value &prev_value, const Am_Value &new_value);
  void Change(const Am_Slot &slot_invalidated,
              Am_Constraint *invalidating_constraint);
  void Slot_Event(Am_Object_Context *oc, const Am_Slot &slot);
  bool Empty() { return !head; }

  void Remove_Any_Overridden_By(const Am_Slot &slot,
                                Am_Constraint *competing_constraint);

  void destroy(const Am_Slot &slot, bool constraint);

  CItem *head;
};

// "Is Inherited" means one or more instances of the slot may inherit this
// slots value. "One of my instances might inherit my value."
#define BIT_IS_INHERITED 0x0001
// "Inherits" means the value of the slot is inherited from its
// prototype.  "I inherit my prototype's value."
#define BIT_INHERITS 0x0002
#define BIT_IS_INVALID 0x0004
#define BIT_VALIDATING_NOW 0x0008
#define BIT_INVALIDATING_NOW 0x0010
#define BIT_IS_PART 0x0020
#define BIT_READ_ONLY 0x0040
#define BIT_SINGLE_CONSTRAINT 0x0080
#define DATA_BITS 0x00e0 // masks PART, READ_ONLY, SINGLE_CONSTRAINT

class Am_Slot_Data : public Am_Value
{
#ifdef MEMORY
public:
  void *operator new(size_t) { return memory.New(); }
  void operator delete(void *ptr, size_t) { memory.Delete(ptr); }
  static Dyn_Memory_Manager memory;
#endif
public:
  Am_Slot_Data(Am_Object_Data *, Am_Slot_Key);
  Am_Slot_Data(Am_Object_Data *, Am_Slot_Key, Am_Value_Type);

  void Set(const Am_Value &new_value, Am_Slot_Flags set_flags);
  void Destroy();

public:
  Am_Object_Data *context;
  am_CList constraints;
  am_CList dependencies;
  am_CList invalid_constraints;
  Am_Slot_Key key;
  unsigned short enabled_demons;
  unsigned short queued_demons;
  unsigned short type_check;
  unsigned short flags : 8;
  Am_Inherit_Rule rule : 2;
};

#endif
