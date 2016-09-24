#include <string.h>

#include <am_inc.h>
#include <amulet/am_io.h>
#include <amulet/impl/types_logging.h>
#include <amulet/object_data.hpp>
#include <amulet/impl/am_constraint.h>
#include <amulet/impl/am_constraint_iterator.h>
#include <amulet/impl/object_debug.h>
#include <amulet/registry.h> // only does something if debug is true
#include <amulet/impl/slots_registry.h>

/*****************************************************************************
 * Am_Object_Data class                                                      *
 *****************************************************************************/
extern Am_Set_Reason Global_Reason_Why_Set;
extern Am_Slot Am_MISSING_SLOT_SLOT;
extern Am_Value Missing_Slot_Value;
extern void slot_error(const char *error_string, Am_Slot_Data *slot);
extern void slot_error(const char *error_string, const Am_Object &object,
                       Am_Slot_Key key);
extern void slot_error(const char *error_string, Am_Object_Data *object_data,
                       Am_Slot_Key key);
extern void slot_error(const char *error_string, Am_Slot_Key key);
extern void Global_Call_Slot_Trace(const Am_Slot &slot);

// global functions
int
num_length(unsigned number)
{
  if (number) {
    unsigned count = 1;
    while (number > 0) {
      number /= 10;
      ++count;
    }
    return count;
  } else
    return 2;
}

void
store_number(char *dest, unsigned number)
{
  unsigned i = strlen(dest) + num_length(number);
  dest[i] = '\0';
  if (number) {
    --i;
    while (number > 0) {
      unsigned digit = number % 10;
      dest[i] = '0' + digit;
      number /= 10;
      --i;
    }
  } else {
    dest[i - 1] = '0';
    i -= 2;
  }
  dest[i] = '_';
}

#ifdef DEBUG
static char *
append_number_to_name(const char *orig_name)
{
  static unsigned Append_Number = 0;
  char *name = new char[strlen(orig_name) + num_length(Append_Number) + 1];
  strcpy(name, orig_name);
  store_number(name, Append_Number);
  ++Append_Number;
  return name;
}
#endif

AM_WRAPPER_DATA_IMPL_ID(Am_Object, (), Am_OBJECT)

Am_Object_Data::Am_Object_Data()
    : prototype((0L)), first_instance((0L)), next_instance((0L)),
      first_part((0L)), next_part((0L)), default_bits(0x0001),
      bits_mask(0x0000), owner_slot((0L), Am_OWNER),
      part_slot(NULL, Am_NO_SLOT), data(sizeof(Am_Slot_Data *))
{
#ifdef LEAK_TRACE
  std::cout << "ROOT\t\tcreated: " << this << "\tReferences: " << Ref_Count()
            << std::endl;
#endif
#ifdef DEBUG
  Am_Register_Name(this, append_number_to_name("ROOT"));
#endif
  owner_slot.context = this;
  owner_slot.value.wrapper_value = (0L);
  owner_slot.type = Am_OBJECT;
  default_rule = Am_INHERIT;
  part_slot.value.wrapper_value = this;
  part_slot.type = Am_OBJECT;
  demon_set = new Am_Demon_Set_Data();
  demons_active = DEMONS_ACTIVE;

  // Prevents deallocation due to ref count.
  Note_Reference();
}

Am_Object_Data::Am_Object_Data(const char *schema_name,
                               Am_Object_Data *in_prototype)
    : owner_slot((0L), Am_OWNER), part_slot(NULL, Am_NO_SLOT),
      demon_set(in_prototype->demon_set),
      demon_queue(in_prototype->demon_queue), data(sizeof(Am_Slot_Data *))
{
#ifdef DEBUG
  if (schema_name) {
#ifdef LEAK_TRACE
    std::cout << schema_name << "\t\tcreated: " << this
              << "\tReferences: " << Ref_Count() << std::endl;
#endif

    char *name = new char[strlen(schema_name) + 1];
    strcpy(name, schema_name);
    Am_Register_Name(this, name);
  } else {
    const char *proto_name = Am_Get_Name_Of_Item(in_prototype);
    if (proto_name == (0L)) {
      std::cout << "Warning: instantiating object without a registered name!\n"
                << std::flush;
#ifdef LEAK_TRACE
      std::cout << proto_name << "\t\tcreated: " << this
                << "\tReferences: " << Ref_Count() << std::endl;
#endif
      Am_Register_Name(this, append_number_to_name("Warning_NULL"));
    } else {
#ifdef LEAK_TRACE
      std::cout << proto_name << "\t\tcreated: " << this
                << "\tReferences: " << Ref_Count() << std::endl;
#endif

      Am_Register_Name(this, append_number_to_name(proto_name));
    }
  }
#endif

  prototype = in_prototype;
  in_prototype->Note_Reference();
  next_instance = in_prototype->first_instance;
  in_prototype->first_instance = this;
  default_bits = in_prototype->default_bits;
  bits_mask = in_prototype->bits_mask;
  default_rule = in_prototype->default_rule;
  demon_set->Note_Reference();
  demons_active = in_prototype->demons_active & DEMONS_ACTIVE;
  first_instance = (0L);
  owner_slot.context = this;
  owner_slot.value.wrapper_value = (0L);
  owner_slot.type = Am_OBJECT;
  first_part = (0L);
  next_part = (0L);
  part_slot.value.wrapper_value = this;
  part_slot.type = Am_OBJECT;
  part_slot.flags = BIT_IS_PART;
}

Am_Object_Data::~Am_Object_Data()
{
// all special debug actions have to take place before we delete
// anything from this object !!
#ifdef DEBUG
  const char *name = Am_Get_Name_Of_Item(this);

#ifdef LEAK_TRACE
  if (name != static_cast<const char *>(0)) {
    std::cout << name << "\t\tdestroyed: " << this
              << "\tReferences: " << Ref_Count() << std::endl;
  } else {
    std::cout << "NO-LONGER-NAMED\t\tdestroyed: " << this
              << "\tReferences: " << Ref_Count() << std::endl;
  }
#endif

  Am_Unregister_Name(this);
  delete[] name;
#endif

  Note_Reference();

  if (data.data) {
    destroy_object();
  }

  part_slot.value.wrapper_value = (0L);
}

Am_Object_Data *
Am_Object_Data::create_object(const char *new_name)
{
  // create a new object, this pointer gets returned to the caller
  Am_Object_Data *new_object = new Am_Object_Data(new_name, this);

  Am_Slot_Data *slot;
  Am_Slot_Data *new_slot;
  Am_Constraint *constraint;
  Am_Constraint *new_constraint;
  unsigned int i;

  demon_queue.Prevent_Invoke();
  Am_PUSH_EMPTY_CC() for (i = 0; i < data.length; i++)
  {
    slot = ((Am_Slot_Data **)data.data)[i];
    if ((slot->flags & BIT_IS_PART) || (slot->rule == Am_STATIC) ||
        (slot->rule == Am_LOCAL))
      continue;
    if (!slot->constraints.Empty() || (slot->enabled_demons & bits_mask)) {
      Am_Slot(slot).Validate();
      new_slot = new Am_Slot_Data(new_object, slot->key);
      new_slot->type = slot->type;
      new_slot->type_check = slot->type_check;
      new_slot->enabled_demons = slot->enabled_demons;
      new_slot->flags = slot->flags & DATA_BITS;
      if (Am_Type_Is_Ref_Counted(slot->type) && slot->value.wrapper_value)
        slot->value.wrapper_value->Note_Reference();
      new_slot->value = slot->value;
      Am_Constraint_Iterator iter((const Am_Slot &)slot);
      for (iter.Start(); !iter.Last(); iter.Next()) {
        constraint = iter.Get();
        new_constraint = constraint->Create(slot, new_slot);
        if (new_constraint)
          Am_Slot(new_slot).Add_Constraint(new_constraint);
      }
      if (slot->rule == Am_COPY) {
        new_slot->flags |= BIT_IS_INVALID;
        new_slot->rule = Am_COPY;
      } else {
        slot->flags |= BIT_IS_INHERITED;
        new_slot->flags |= BIT_INHERITS | BIT_IS_INVALID;
        new_slot->rule = Am_INHERIT;
      }
      new_object->data.Add((char *)&new_slot);
    } else
      slot->flags |= BIT_IS_INHERITED;
  }
  Am_POP_EMPTY_CC() demon_queue.Release_Invoke();
  Am_Object_Data *curr_part;
  Am_Object_Data *new_part;
  for (curr_part = first_part; curr_part != (0L);
       curr_part = curr_part->next_part) {
    if (curr_part->part_slot.key != Am_NO_INHERIT) {
      new_part = curr_part->create_object((0L));
      new_part->part_slot.key = curr_part->part_slot.key;
      new_part->next_part = new_object->first_part;
      new_object->first_part = new_part;
      new_part->owner_slot.value.wrapper_value = new_object;
      new_part->part_slot.context = new_object;
      if (curr_part->part_slot.key) {
        Am_Slot_Data *slot = &new_part->part_slot;
        new_object->data.Add((char *)&slot);
      }
    }
  }
  return new_object;
}

Am_Object_Data *
Am_Object_Data::copy_object(const char *new_name)
{
  Am_Object_Data *new_object = new Am_Object_Data(new_name, prototype);

  Am_Slot_Data *slot;
  Am_Slot_Data *new_slot;
  Am_Constraint *constraint;
  Am_Constraint *new_constraint;
  unsigned int i;
  demon_queue.Prevent_Invoke();
  Am_PUSH_EMPTY_CC() for (i = 0; i < data.length; i++)
  {
    slot = ((Am_Slot_Data **)data.data)[i];
    if ((slot->flags & BIT_IS_PART) || (slot->rule == Am_LOCAL))
      continue;
    Am_Slot(slot).Validate();
    new_slot = new Am_Slot_Data(new_object, slot->key);
    new_slot->rule = slot->rule;
    new_slot->type = slot->type;
    new_slot->type_check = slot->type_check;
    new_slot->enabled_demons = slot->enabled_demons;
    new_slot->flags = (slot->flags & ~BIT_IS_INHERITED) | BIT_IS_INVALID;
    if (Am_Type_Is_Ref_Counted(slot->type) && slot->value.wrapper_value)
      slot->value.wrapper_value->Note_Reference();
    new_slot->value = slot->value;
    Am_Constraint_Iterator iter((const Am_Slot &)slot);
    for (iter.Start(); !iter.Last(); iter.Next()) {
      constraint = iter.Get();
      new_constraint = constraint->Copy(slot, new_slot);
      if (new_constraint)
        Am_Slot(new_slot).Add_Constraint(new_constraint);
    }
    new_object->data.Add((char *)&new_slot);
  }
  Am_POP_EMPTY_CC() demon_queue.Release_Invoke();
  Am_Object_Data *curr_part;
  Am_Object_Data *new_part;
  for (curr_part = first_part; curr_part != (0L);
       curr_part = curr_part->next_part) {
    if (curr_part->part_slot.key != Am_NO_INHERIT) {
      new_part = curr_part->copy_object((0L));
      new_part->part_slot.key = curr_part->part_slot.key;
      new_part->next_part = new_object->first_part;
      new_object->first_part = new_part;
      new_part->owner_slot.value.wrapper_value = new_object;
      new_part->part_slot.context = new_object;
      if (curr_part->part_slot.key) {
        Am_Slot_Data *slot = &new_part->part_slot;
        new_object->data.Add((char *)&slot);
      }
      new_object->Note_Reference();
      Am_Value oldval(Am_No_Object), newval(new_object);
      new_part->owner_slot.dependencies.Change(&new_part->owner_slot, (0L),
                                               oldval, newval);
    }
  }
  Note_Reference();
  new_object->Note_Reference();
  Am_Object(new_object)
      .Set(Am_SOURCE_OF_COPY, Am_Object(this), Am_OK_IF_NOT_THERE);
  return new_object;
}

Am_Object_Data *
Am_Object_Data::copy_object_value_only(const char *new_name)
{
  Am_Object_Data *new_object = new Am_Object_Data(new_name, prototype);

  Am_Slot_Data *slot;
  Am_Slot_Data *new_slot;

  unsigned int i;
  demon_queue.Prevent_Invoke();
  Am_PUSH_EMPTY_CC() for (i = 0; i < data.length; i++)
  {
    slot = ((Am_Slot_Data **)data.data)[i];
    // BAM:  copy-value-only DOES copy local slots since needed for
    // copies of commands: need all the slots that aren't inheritable.
    // if we change this later, may need inherit-rule for
    // not-inherited-but-is-copied  // (slot->rule == Am_LOCAL))
    if ((slot->flags & BIT_IS_PART))
      continue;
    Am_Slot(slot).Validate();
    new_slot = new Am_Slot_Data(new_object, slot->key);
    new_slot->rule = slot->rule;
    new_slot->type = slot->type;
    new_slot->type_check = slot->type_check;
    new_slot->enabled_demons = slot->enabled_demons;
    new_slot->flags = slot->flags & ~BIT_IS_INHERITED;
    if (Am_Type_Is_Ref_Counted(slot->type) && slot->value.wrapper_value)
      slot->value.wrapper_value->Note_Reference();
    new_slot->value = slot->value;
    new_object->data.Add((char *)&new_slot);
  }
  Am_POP_EMPTY_CC() demon_queue.Release_Invoke();
  Am_Object_Data *curr_part;
  Am_Object_Data *new_part;
  for (curr_part = first_part; curr_part != (0L);
       curr_part = curr_part->next_part) {
    if (curr_part->part_slot.key != Am_NO_INHERIT) {
      new_part = curr_part->copy_object_value_only((0L));
      new_part->part_slot.key = curr_part->part_slot.key;
      new_part->next_part = new_object->first_part;
      new_object->first_part = new_part;
      new_part->owner_slot.value.wrapper_value = new_object;
      new_part->part_slot.context = new_object;
      if (curr_part->part_slot.key) {
        Am_Slot_Data *slot = &new_part->part_slot;
        new_object->data.Add((char *)&slot);
      }
      new_object->Note_Reference();
      Am_Value oldval(Am_No_Object), newval(new_object);
      new_part->owner_slot.dependencies.Change(&new_part->owner_slot, (0L),
                                               oldval, newval);
    }
  }
  Note_Reference();
  new_object->Note_Reference();
  Am_Object(new_object)
      .Set(Am_SOURCE_OF_COPY, Am_Object(this), Am_OK_IF_NOT_THERE);
  return new_object;
}

void
Am_Object_Data::invoke_create_demons()
{
  Am_Object_Data *curr_part;
  for (curr_part = first_part; curr_part != (0L);
       curr_part = curr_part->next_part)
    curr_part->invoke_create_demons();
  if ((demons_active & DEMONS_ACTIVE) && demon_set->create_demon) {
    Note_Reference();
    demon_set->create_demon(Am_Object(this));
  }
}

void
Am_Object_Data::invoke_copy_demons()
{
  Am_Object_Data *curr_part;
  for (curr_part = first_part; curr_part != (0L);
       curr_part = curr_part->next_part)
    curr_part->invoke_copy_demons();
  if ((demons_active & DEMONS_ACTIVE) && demon_set->copy_demon) {
    Note_Reference();
    demon_set->copy_demon(Am_Object(this));
  }
}

void
Am_Object_Data::destroy_object()
{
  bool active = (demons_active & DEMONS_ACTIVE) != 0;
  demons_active &= ~DEMONS_ACTIVE;
  if (owner_slot.value.wrapper_value) {
    Note_Reference();
    Am_Object(this).Remove_From_Owner();
  }
  if (demon_queue) {
    Note_Reference();
    demon_queue.Delete(Am_Object(this));
  }
  if (active) {
    demon_queue.Invoke();
    if (demon_set->destroy_demon) {
      Note_Reference();
      demon_set->destroy_demon(Am_Object(this));
    }
  }
  Am_Object_Data *current;
  Am_Object_Data *next;
  for (current = first_part; current != (0L); current = current->next_part)
    current->demon_removal();
  current = first_part;
  first_part = (0L);
  while (current) {
    next = current->next_part;
    current->Note_Reference();
    Am_Object(current).Destroy();
    current = next;
  }
  Am_Slot_Data **data_array = (Am_Slot_Data **)data.data;
  data.data = (0L);
  if (prototype) {
    current = prototype->first_instance;
    Am_Object_Data *prev = (0L);
    while (current) {
      if (current == this) {
        if (prev)
          prev->next_instance = next_instance;
        else
          prototype->first_instance = next_instance;
        break;
      }
      prev = current;
      current = current->next_instance;
    }
    prototype->Release();
    prototype = (0L);
  }
  if (data_array) {
    Am_Slot_Data *slot;
    Am_Slot_Data *proto_slot;
    unsigned i;
    for (i = 0; i < data.length; ++i) {
      slot = data_array[i];
      if (!(slot->flags & BIT_IS_PART)) {
        proto_slot =
            prototype ? prototype->find_slot(slot->key) : (Am_Slot_Data *)(0L);
        delete_slot(slot, proto_slot);
      }
      data_array[i] = (0L);
    }
    data.data = (char *)data_array;
    data.Destroy();
    part_slot.dependencies.destroy(&part_slot, false);
    owner_slot.dependencies.destroy(&owner_slot, false);
    data.length = 0;
  } else
    Am_Error("** Object deleted twice");
  if (demon_set) {
    demon_set->Release();
    demon_set = (0L);
  }
}

void
Am_Object_Data::validate_object()
{
  unsigned int i;
  Am_Slot_Data *slot;
  demon_queue.Prevent_Invoke();
  Am_PUSH_EMPTY_CC() for (i = 0; i < data.length; i++)
  {
    slot = ((Am_Slot_Data **)data.data)[i];
    Am_Slot(slot).Validate();
  }
  Am_Value oldval(Am_No_Object), newval;
  Am_Object_Data *curr_part;
  for (curr_part = first_part; curr_part != (0L);
       curr_part = curr_part->next_part) {
    curr_part->validate_object();
    Am_Object_Data *part =
        (Am_Object_Data *)curr_part->part_slot.value.wrapper_value;
    if (part)
      part->Note_Reference();
    newval = part;
    curr_part->part_slot.dependencies.Change(&curr_part->part_slot, (0L),
                                             oldval, newval);
  }
  if (owner_slot.value.wrapper_value)
    owner_slot.value.wrapper_value->Note_Reference();
  newval = owner_slot.value.wrapper_value;
  owner_slot.dependencies.Change(&owner_slot, (0L), oldval, newval);
  Am_POP_EMPTY_CC() demon_queue.Release_Invoke();
}

void
Am_Object_Data::note_parts()
{
  Am_Object_Context oc(false);
  owner_slot.dependencies.Slot_Event(&oc, &owner_slot);
  Am_Object_Data *curr_part;
  for (curr_part = first_part; curr_part != (0L);
       curr_part = curr_part->next_part) {
    curr_part->note_parts();
    curr_part->part_slot.dependencies.Slot_Event(&oc, &curr_part->part_slot);
  }
}

void
Am_Object_Data::demon_removal()
{
  bool active = (demons_active & DEMONS_ACTIVE) != 0;
  demons_active &= ~DEMONS_ACTIVE;
  Note_Reference();
  demon_queue.Delete(this);
  if (active && demon_set->destroy_demon) {
    Note_Reference();
    demon_set->destroy_demon(this);
  }
  Am_Object_Data *current;
  for (current = first_part; current != (0L); current = current->next_part)
    current->demon_removal();
}

Am_Slot_Data *
Am_Object_Data::find_slot(Am_Slot_Key key)
{
  if (key == Am_OWNER)
    return &owner_slot;

  unsigned i;
  Am_Slot_Data **slot_array;
  Am_Slot_Data *slot;

  for (i = data.length, slot_array = (Am_Slot_Data **)data.data; i > 0;
       --i, ++slot_array) {
    slot = *slot_array;
    if (key == slot->key) {
      return slot;
    }
  }
  if (prototype) {
    slot = prototype->find_slot(key);
    if (slot && static_cast<Am_Inherit_Rule>(slot->rule) != Am_LOCAL)
      return slot;
  }
  return (0L);
}

Am_Slot_Data *
Am_Object_Data::find_prototype(Am_Slot_Key key)
{
  if (key == Am_OWNER)
    return &owner_slot;

  unsigned i;
  Am_Slot_Data **slot_array;
  Am_Slot_Data *slot;

  for (i = data.length, slot_array = (Am_Slot_Data **)data.data; i > 0;
       --i, ++slot_array) {
    slot = *slot_array;
    if (key == slot->key) {
      return slot;
    }
  }
  if (prototype)
    return prototype->find_prototype(key);
  else
    return (Am_Slot_Data *)Am_MISSING_SLOT_SLOT;
}

void
Am_Object_Data::find_slot_and_position(Am_Slot_Key key, Am_Slot_Data *&out_slot,
                                       unsigned &out_i)
{
  unsigned i;
  Am_Slot_Data **slot_array;
  Am_Slot_Data *slot;

  for (i = data.length, slot_array = (Am_Slot_Data **)data.data; i > 0;
       --i, ++slot_array) {
    slot = *slot_array;
    if (key == slot->key) {
      out_slot = slot;
      out_i = data.length - i;
      return;
    }
  }
  out_slot = (0L);
}

void
Am_Object_Data::set_slot(Am_Slot_Key key, const Am_Value &value,
                         Am_Slot_Flags set_flags)
{
#ifdef DEBUG
  Global_Reason_Why_Set = Am_TRACE_SLOT_SET;
#endif
  Am_Slot_Data *slot = find_prototype(key);

  if (key == Am_OWNER) {
    slot_error("Cannot set Am_OWNER slot directly; use Add_Part or Set_Part",
               this, key);
  }
  bool slot_exists = slot && slot->type != Am_MISSING_SLOT &&
                     (static_cast<Am_Inherit_Rule>(slot->rule) != Am_LOCAL ||
                      slot->context == this);
  if (!slot_exists && !(set_flags & Am_OK_IF_NOT_THERE)) {
    slot_error("Set called on slot which is not there.  Use Add instead", this,
               key);
  } else if (slot_exists && !(set_flags & Am_OK_IF_THERE)) {
    slot_error("Add called on slot which is already there.  Use Set instead",
               this, key);
  }
  if (Am_Type_Class(value.type) == Am_ERROR_VALUE_TYPE) {
    std::cerr << "** Trying to set the error value: " << value
              << " into a slot.\n";
    slot_error("", this, key);
  }

  if (slot->context != this) {
    if ((slot == Am_MISSING_SLOT_SLOT) ||
        (static_cast<Am_Inherit_Rule>(slot->rule) != Am_STATIC)) {
      Am_Slot_Data *new_slot = new Am_Slot_Data(this, key);
      if ((slot != Am_MISSING_SLOT_SLOT) && !(slot->flags & BIT_IS_PART)) {
        if (slot->rule == Am_COPY)
          sever_copies(slot);
        new_slot->flags = BIT_IS_INHERITED | (slot->flags & DATA_BITS);
        new_slot->type_check = slot->type_check;
        new_slot->enabled_demons = slot->enabled_demons;
        new_slot->rule = slot->rule;
      }
      data.Add((char *)&new_slot);
      slot = new_slot;
    }
  } else if (slot->flags & BIT_IS_PART) {
#if 0
    // too constraining -- rcm
    slot_error("Cannot use Set or Add on a Part slot.  "
	       "Use Set_Part to change the part; "
	       "or Remove_Part then regular Set or Add", slot);
#endif
    //// NOTE: presently parts are always inherited.  May not be true in future.
    slot->value.wrapper_value->Note_Reference();
    Am_Object(*slot).Remove_From_Owner();
    Am_Slot_Data *new_slot = new Am_Slot_Data(this, key);
    data.Add((char *)&new_slot);
    new_slot->Set(value, set_flags);
    slot->value.wrapper_value->Note_Reference();
    ((Am_Object)((Am_Object_Data *)slot->value.wrapper_value)).Destroy();
    slot->value.wrapper_value->Release();
    return;
  } else if (slot->rule == Am_COPY && slot->flags & BIT_IS_INHERITED)
    sever_copies(slot);
  slot->Set(value, set_flags);
  slot->flags &= ~BIT_INHERITS;
}

void
Am_Object_Data::set_slot(Am_Slot_Key key, Am_Constraint *constraint,
                         Am_Slot_Flags set_flags)
{
#ifdef DEBUG
  Global_Reason_Why_Set = Am_TRACE_SLOT_SET;
#endif
  Am_Slot_Data *slot = find_slot(key);

  if (key == Am_OWNER) {
    slot_error("Cannot set Am_OWNER slot directly; use Add_Part or Set_Part",
               this, key);
  }
  bool slot_exists = slot && slot->type != Am_MISSING_SLOT &&
                     (static_cast<Am_Inherit_Rule>(slot->rule) != Am_LOCAL ||
                      slot->context == this);

  if (!slot_exists && !(set_flags & Am_OK_IF_NOT_THERE)) {
    slot_error("Set called on slot which is not there.  Use Add instead", this,
               key);
  } else if (slot_exists && !(set_flags & Am_OK_IF_THERE)) {
    slot_error("Add called on slot which is already there.  Use Set instead",
               this, key);
  }

  // Check if slot is really a part.
  if (slot && (slot->flags & BIT_IS_PART) && slot->context == this) {
    slot->value.wrapper_value->Note_Reference();
    Am_Object(*slot).Remove_From_Owner();
    slot->value.wrapper_value->Note_Reference();
    ((Am_Object)((Am_Object_Data *)slot->value.wrapper_value)).Destroy();
    slot->value.wrapper_value->Release();
    slot = (0L);
  }

  if (!slot || slot->context != this) {
    if (!slot || (static_cast<Am_Inherit_Rule>(slot->rule) != Am_STATIC)) {
      Am_Slot_Data *new_slot = new Am_Slot_Data(this, key);
      if (slot) {
        new_slot->enabled_demons = slot->enabled_demons;
        if (slot->rule == Am_COPY) {
          new_slot->flags = slot->flags & DATA_BITS;
          new_slot->rule = Am_COPY;
        } else {
          new_slot->flags = BIT_IS_INHERITED | (slot->flags & DATA_BITS);
          new_slot->rule = Am_INHERIT;
        }
        new_slot->type = slot->type;
        if (Am_Type_Is_Ref_Counted(slot->type) && slot->value.wrapper_value)
          slot->value.wrapper_value->Note_Reference();
        new_slot->value = slot->value;
      } else
        new_slot->type = Am_FORMULA_INVALID;
      data.Add((char *)&new_slot);
      slot = new_slot;
    }
  }
  if (slot->flags & BIT_IS_INHERITED) {
    if (slot->rule == Am_INHERIT)
      propagate_constraint(key, constraint, slot);
    else if (slot->rule == Am_COPY)
      sever_copies(slot);
  }
  Am_Slot(slot).Add_Constraint(constraint);
  Am_Slot(slot).Invalidate(constraint);
  if (slot->flags & BIT_INHERITS) {
    slot->flags &= ~BIT_INHERITS;
    Am_Object_Context oc(false);
    slot->dependencies.Slot_Event(&oc, slot);
  }
#ifdef DEBUG
  Global_Call_Slot_Trace(slot);
#endif
}

bool
Am_Object_Data::notify_change(Am_Slot_Key key, Am_Constraint *cause,
                              const Am_Value &old_value,
                              const Am_Value &new_value)
{
  unsigned i;
  Am_Slot_Data **slot_array;
  Am_Slot_Data *slot;

  for (i = data.length, slot_array = (Am_Slot_Data **)data.data; i > 0;
       --i, ++slot_array) {
    slot = *slot_array;
    if (key == slot->key) {
      if (slot->flags & BIT_INHERITS) {
        slot->dependencies.Change(slot, cause, old_value, new_value);
        if (slot->flags & BIT_IS_INHERITED)
          if (propagate_change(key, cause, old_value, new_value))
            slot->flags &= ~BIT_IS_INHERITED;
        enqueue_change(slot);
        if (Am_Type_Is_Ref_Counted(slot->type) && slot->value.wrapper_value)
          slot->value.wrapper_value->Release();
        if (Am_Type_Is_Ref_Counted(new_value.type) &&
            new_value.value.wrapper_value)
          new_value.value.wrapper_value->Note_Reference();
        slot->value = new_value.value;
        slot->type = new_value.type;
#ifdef DEBUG
        Global_Call_Slot_Trace(slot);
#endif
        return false;
      } else
        return true;
    }
  }
  propagate_change(key, cause, old_value, new_value);
  return false;
}

bool
Am_Object_Data::propagate_change(Am_Slot_Key key, Am_Constraint *cause,
                                 const Am_Value &old_value,
                                 const Am_Value &new_value)
{
#ifdef DEBUG
  Global_Reason_Why_Set |= Am_TRACE_INHERITANCE_PROPAGATION;
#endif
  bool no_inherit = true;
  Am_Object_Data *current = first_instance;
  while (current) {
    no_inherit =
        current->notify_change(key, cause, old_value, new_value) && no_inherit;
    current = current->next_instance;
  }
#ifdef DEBUG
  Global_Reason_Why_Set &= ~Am_TRACE_INHERITANCE_PROPAGATION;
#endif
  return no_inherit;
}

bool
Am_Object_Data::notify_change(Am_Slot_Key key, Am_Constraint *cause)
{
  unsigned i;
  Am_Slot_Data **slot_array;
  Am_Slot_Data *slot;

  for (i = data.length, slot_array = (Am_Slot_Data **)data.data; i > 0;
       --i, ++slot_array) {
    slot = *slot_array;
    if (key == slot->key) {
      if (slot->flags & BIT_INHERITS) {
        slot->dependencies.Change(slot, cause);
        if (slot->flags & BIT_IS_INHERITED)
          if (propagate_change(key, cause))
            slot->flags &= ~BIT_IS_INHERITED;
        enqueue_change(slot);
#ifdef DEBUG
        Global_Call_Slot_Trace(slot);
#endif
        return false;
      } else
        return true;
    }
  }
  propagate_change(key, cause);
  return false;
}

bool
Am_Object_Data::propagate_change(Am_Slot_Key key, Am_Constraint *cause)
{
#ifdef DEBUG
  Global_Reason_Why_Set |= Am_TRACE_INHERITANCE_PROPAGATION;
#endif
  bool no_inherit = true;
  Am_Object_Data *current = first_instance;
  while (current) {
    no_inherit = current->notify_change(key, cause) && no_inherit;
    current = current->next_instance;
  }
#ifdef DEBUG
  Global_Reason_Why_Set &= ~Am_TRACE_INHERITANCE_PROPAGATION;
#endif
  return no_inherit;
}

bool
Am_Object_Data::notify_unique(Am_Slot_Key key, Am_Wrapper *new_value)
{
  unsigned i;
  Am_Slot_Data **slot_array;
  Am_Slot_Data *slot;

  for (i = data.length, slot_array = (Am_Slot_Data **)data.data; i > 0;
       --i, ++slot_array) {
    slot = *slot_array;
    if (key == slot->key) {
      if (slot->flags & BIT_INHERITS) {
        if (new_value)
          new_value->Note_Reference();
        slot->value.wrapper_value = new_value;
        if (slot->flags & BIT_IS_INHERITED)
          if (propagate_unique(key, new_value))
            slot->flags &= ~BIT_IS_INHERITED;
        return false;
      } else
        return true;
    }
  }
  propagate_unique(key, new_value);
  return false;
}

bool
Am_Object_Data::propagate_unique(Am_Slot_Key key, Am_Wrapper *new_value)
{
  bool no_inherit = true;
  Am_Object_Data *current = first_instance;
  while (current) {
    no_inherit = current->notify_unique(key, new_value) && no_inherit;
    current = current->next_instance;
  }
  return no_inherit;
}

void
Am_Object_Data::sever_slot(Am_Slot_Data *proto)
{
  unsigned i;
  Am_Slot_Data **slot_array;
  Am_Slot_Data *slot;
  Am_Slot_Key key = proto->key;

  for (i = data.length, slot_array = (Am_Slot_Data **)data.data; i > 0;
       --i, ++slot_array) {
    slot = *slot_array;
    if (key == slot->key)
      return;
  }
  slot = new Am_Slot_Data(this, key);
  slot->rule = Am_COPY;
  slot->enabled_demons = proto->enabled_demons;
  slot->type_check = proto->type_check;
  slot->flags = BIT_IS_INHERITED | (proto->flags & DATA_BITS);
  data.Add((char *)&slot);
  slot->Set(*proto, 0);
}

void
Am_Object_Data::sever_copies(Am_Slot_Data *proto)
{
  proto->flags &= ~BIT_IS_INHERITED;
  Am_Object_Data *current = first_instance;
  while (current) {
    current->sever_slot(proto);
    current = current->next_instance;
  }
}

void
Am_Object_Data::remove_temporary_slot(Am_Slot_Key key)
{
  unsigned i;
  Am_Slot_Data **slot_array;
  Am_Slot_Data *slot;

  for (i = 0, slot_array = (Am_Slot_Data **)data.data; i < data.length;
       ++i, ++slot_array) {
    slot = *slot_array;
    if (key == slot->key) {
      if (slot->flags & BIT_INHERITS) {
        data.Delete(i);
        slot->dependencies.Invalidate(slot, (0L), *slot);
        slot->Destroy();
      } else
        return;
    }
  }
  remove_temporaries(key);
}

void
Am_Object_Data::remove_temporaries(Am_Slot_Key key)
{
  Am_Object_Data *current = first_instance;
  while (current) {
    current->remove_temporary_slot(key);
    current = current->next_instance;
  }
}

void
Am_Object_Data::convert_temporary_slot(Am_Slot_Key key)
{
  unsigned i;
  Am_Slot_Data **slot_array;
  Am_Slot_Data *slot;

  for (i = data.length, slot_array = (Am_Slot_Data **)data.data; i > 0;
       --i, ++slot_array) {
    slot = *slot_array;
    if (key == slot->key) {
      if (slot->flags & BIT_INHERITS) {
        slot->flags &= ~BIT_INHERITS;
        slot->rule = Am_COPY;
      } else
        return;
    }
  }
  convert_temporaries(key);
}

void
Am_Object_Data::convert_temporaries(Am_Slot_Key key)
{
  Am_Object_Data *current = first_instance;
  while (current) {
    current->convert_temporary_slot(key);
    current = current->next_instance;
  }
}

void
Am_Object_Data::remove_subconstraint(Am_Slot_Key key, Am_Constraint *proto)
{
  Am_Slot_Data *slot = find_slot(key);
  if (slot && slot->context == this) {
    Am_Constraint *constraint;
    Am_Constraint_Tag tag;
    Am_Constraint_Iterator iter((const Am_Slot &)slot);
    iter.Start();
    while (!iter.Last()) {
      constraint = iter.Get();
      tag = iter.Get_Tag();
      iter.Next();
      if (constraint->Get_Prototype() == proto) {
        if (slot->rule == Am_INHERIT)
          prop_remove_constraint(key, constraint);
        Am_Slot(slot).Remove_Constraint(tag);
      }
    }
  }
}

void
Am_Object_Data::prop_remove_constraint(Am_Slot_Key key, Am_Constraint *proto)
{
  Am_Object_Data *current = first_instance;
  while (current) {
    current->remove_subconstraint(key, proto);
    current = current->next_instance;
  }
}

void
Am_Object_Data::set_constraint(Am_Slot_Key key, Am_Constraint *constraint,
                               Am_Slot_Data *proto)
{
  Am_Slot_Data *slot = find_slot(key);
  if (slot->context != this) {
    Am_Slot_Data *new_slot = new Am_Slot_Data(this, key);
    new_slot->enabled_demons = slot->enabled_demons;
    new_slot->flags =
        BIT_IS_INHERITED | BIT_INHERITS | (slot->flags & DATA_BITS);
    new_slot->rule = Am_INHERIT;
    data.Add((char *)&new_slot);
    slot = new_slot;
  }
  if (slot->flags & BIT_INHERITS) {
    Am_Constraint *new_constraint = constraint->Create(proto, slot);
    if (new_constraint) {
      if (slot->flags & BIT_IS_INHERITED) {
        if (slot->rule == Am_INHERIT)
          propagate_constraint(key, new_constraint, slot);
        else if (slot->rule == Am_COPY)
          sever_copies(slot);
      }
      Am_Slot(slot).Add_Constraint(new_constraint);
      Am_Slot(slot).Invalidate(new_constraint);
    }
  }
}

void
Am_Object_Data::propagate_constraint(Am_Slot_Key key, Am_Constraint *constraint,
                                     Am_Slot_Data *proto)
{
#ifdef DEBUG
  Global_Reason_Why_Set |= Am_TRACE_INHERITANCE_PROPAGATION;
#endif
  Am_Object_Data *current = first_instance;
  while (current) {
    current->set_constraint(key, constraint, proto);
    current = current->next_instance;
  }
#ifdef DEBUG
  Global_Reason_Why_Set &= ~Am_TRACE_INHERITANCE_PROPAGATION;
#endif
}

void
Am_Object_Data::delete_slot(Am_Slot_Data *slot, Am_Slot_Data *proto_slot)
{
  Am_Value_Type type = (Am_Value_Type)slot->type;
  if (proto_slot) {
    Am_Inherit_Rule rule = (Am_Inherit_Rule)proto_slot->rule;
    if ((rule != Am_LOCAL) && (rule != Am_STATIC))
      proto_slot->flags |= BIT_IS_INHERITED;
    if ((slot->rule == Am_COPY) || (rule == Am_COPY))
      sever_copies(slot);
    else if ((slot->rule == Am_INHERIT) && (rule == Am_INHERIT)) {
#if 1 // propagate all changes
      if (*slot != *proto_slot)
#endif
        if (propagate_change(slot->key, (0L), *slot, *proto_slot))
          proto_slot->flags &= ~BIT_IS_INHERITED;
    }
  } else if (type != Am_MISSING_SLOT)
    (void)propagate_change(slot->key, (0L), *slot, Missing_Slot_Value);
  slot->Destroy();
}

void
Am_Object_Data::remove_part()
{
  Am_Object_Data *prev = (0L);
  Am_Object_Data *curr;
  Am_Object_Data *owner = (Am_Object_Data *)owner_slot.value.wrapper_value;
  owner_slot.value.wrapper_value = (0L);
  for (curr = owner->first_part; curr != (0L); curr = curr->next_part) {
    if (curr == this) {
      if (prev)
        prev->next_part = curr->next_part;
      else
        owner->first_part = curr->next_part;
      break;
    }
    prev = curr;
  }
  part_slot.context = (0L);
  if (part_slot.key) {
    part_slot.key = Am_NO_SLOT;
    Am_Slot_Data *slot;
    unsigned i;
    owner->find_slot_and_position(part_slot.key, slot, i);
    if (slot)
      owner->data.Delete(i);
  }
}

void
Am_Object_Data::enqueue_change(Am_Slot_Data *slot)
{
  if (demons_active & DEMONS_ACTIVE) {
    Slot_Demon_Holder *holder;
    unsigned short demon_bit;
    for (int i = 0; i < demon_set->change_length; ++i) {
      holder = &(demon_set->change_demons[i]);
      demon_bit = holder->which_bit;
      if ((demon_bit & slot->enabled_demons) &&
          !(demon_bit & slot->queued_demons) &&
          (!holder->protocol || !(demon_bit & demons_active))) {
        slot->queued_demons |= demon_bit;
        demons_active |= demon_bit;
        demon_queue.Enqueue(holder->demon, demon_bit, slot);
      }
    }
  }
}

void
Am_Object_Data::enqueue_invalid(Am_Slot_Data *slot)
{
  if (demons_active & DEMONS_ACTIVE) {
    Slot_Demon_Holder *holder;
    unsigned short demon_bit;
    for (int i = 0; i < demon_set->invalid_length; ++i) {
      holder = &(demon_set->invalid_demons[i]);
      demon_bit = holder->which_bit;
      if ((demon_bit & slot->enabled_demons) &&
          !(demon_bit & slot->queued_demons) &&
          (!holder->protocol || !(demon_bit & demons_active))) {
        slot->queued_demons |= demon_bit;
        demons_active |= demon_bit;
        demon_queue.Enqueue(holder->demon, demon_bit, slot);
      }
    }
  }
}

void
Am_Object_Data::Print(std::ostream &os) const
{
  os << "<";
#ifdef DEBUG
  const char *name = Am_Get_Name_Of_Item(this);
  if (!name || (name[0] == '\0'))
    os << std::hex << (long)this << std::dec;
  else
    os << name;
#else
  std::cout << "Am_Object:" << std::hex << (long)this << std::dec;
#endif
  os << ">";
}

void
Am_Object_Data::print_slot_name_and_value(Am_Slot_Key key,
                                          Am_Slot_Data *slot) const
{
  const char *key_name = Am_Get_Slot_Name(key);
  if (key_name)
    std::cout << key_name;
  else
    std::cout << "[" << (int)key << "]";
  if (slot) {
    if (slot->flags & BIT_IS_INVALID) //print out value with a warning
      std::cout << " **SLOT VALUE IS INVALID**";
    bool is_inherited =
        (slot->context != this) || !!(slot->flags & BIT_INHERITS);
    if (is_inherited)
      std::cout << " (inherited)";
    std::cout << " = " << *slot << std::endl;
  } else
    std::cout << " ** SLOT IS MISSING\n";
}
