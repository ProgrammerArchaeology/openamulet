//
// ------------------------------------------------------------------------
// 		The OpenAmulet User Interface Development Environment
// ------------------------------------------------------------------------
// This code is based on the Amulet project of Carnegie Mellon University,
// and has been placed in the public domain.  If you are using this code
// or any part of OpenAmulet, please contact amulecs.cmu.edu to be put
// on the mailing list or have a look at:
// http://www.openip.org
//

#include <string.h>

#include <am_inc.h>

#include <amulet/am_io.h>

#include <amulet/standard_slots.h>
#include <amulet/initializer.h>
#include <amulet/object.h>
#include <amulet/object_advanced.h>
#include <amulet/impl/opal_objects.h>

#include <amulet/object_data.hpp>

//only does something if debug is true
#include <amulet/registry.h>

#include <stdlib.h> // For abort () for Am_Error

unsigned short Num_To_Bit[] = {0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020,
                               0x0040, 0x0080, 0x0100, 0x0200, 0x0400, 0x0800,
                               0x1000, 0x2000, 0x4000, 0x8000};

AM_DEFINE_METHOD_TYPE_IMPL(Am_Object_Method)

static Am_Value_Type useless = Am_Set_ID_Class(DSTR("constraint"), 0x3000);

// creating the top-most prototype
Am_Object_Data Root_Data;
Am_Object Am_Root_Object(&Root_Data);

Am_Slot_Data Missing_Slot_Data((nullptr), Am_NO_SLOT, Am_MISSING_SLOT);
Am_Slot Am_MISSING_SLOT_SLOT(&Missing_Slot_Data);
Am_Value Missing_Slot_Value = Am_MISSING_SLOT_SLOT.Get();

Am_Value Get_On_Null_Object_Value(0, Am_GET_ON_NULL_OBJECT);

//Am_Zero_Value and Am_No_Value are provided by types.h

Am_Object Am_No_Object;

#ifdef DEBUG

Am_Slot_Set_Trace_Proc *Am_Global_Slot_Trace_Proc = nullptr;
Am_Set_Reason Global_Reason_Why_Set = Am_TRACE_NOT_SPECIFIED;

Am_Object_Create_Trace_Proc *Am_Global_Object_Trace_Proc = nullptr;
Am_Method_Get_Trace_Proc *Am_Global_Method_Get_Proc = nullptr;

void
Global_Call_Slot_Trace(const Am_Slot &slot)
{
  if (Am_Global_Slot_Trace_Proc)
    Am_Global_Slot_Trace_Proc(slot, Global_Reason_Why_Set);
}

void
Global_Call_Object_Trace(const Am_Object &old_obj, const Am_Object &new_obj,
                         Am_Set_Reason reason)
{
  if (Am_Global_Object_Trace_Proc)
    Am_Global_Object_Trace_Proc(old_obj, new_obj, reason);
}

void
Global_Check_Get_Method_Trace(const Am_Object &of_obj, Am_Slot_Key slot_key,
                              Am_Value &value)
{
  if (Am_Global_Method_Get_Proc && (Am_Type_Class(value.type) == Am_METHOD))
    Am_Global_Method_Get_Proc(of_obj, slot_key, value);
}

#endif

std::ostream &
operator<<(std::ostream &os, const Am_Object &object)
{
  object.Print(os);
  return os;
}
std::ostream &
operator<<(std::ostream &os, const Am_Object_Advanced &object)
{
  object.Print_Name_And_Data(os);
  return os;
}

#ifdef MEMORY
Dyn_Memory_Manager QItem::memory(sizeof(QItem), "QItem");
#endif

inline QItem::QItem(Am_Slot_Demon *in_demon, unsigned short in_which_bit,
                    const Am_Slot &in_param)
    : demon(in_demon), which_bit(in_which_bit), param(in_param)
{
  ;
}

QItem::~QItem()
{
  if (param) {
    param->queued_demons &= ~which_bit;
    param->context->demons_active &= ~which_bit;
  }
}

void
Am_Demon_Queue::Enqueue(Am_Slot_Demon *demon, unsigned short which_bit,
                        const Am_Slot &param)
{
  if (demon && data->active)
    data->Enqueue(new QItem(demon, which_bit, param));
}

void
Am_Demon_Queue::Delete(const Am_Object &object)
{
  if (!data->active)
    return;
  QItem *prev = nullptr;
  QItem *curr = data->head;
  QItem *next;
  Am_Wrapper *obj_data = object;
  while (curr) {
    next = curr->next;
    if (curr->param->context == obj_data) {
      if (prev)
        prev->next = next;
      else
        data->head = next;
      delete curr;
    } else
      prev = curr;
    curr = next;
  }
  data->tail = prev;
  obj_data->Release();
}

void
Am_Demon_Queue::Delete(const Am_Slot &slot)
{
  if (!data->active)
    return;
  QItem *prev = nullptr;
  QItem *curr = data->head;
  QItem *next;
  Am_Slot_Data *slot_data = slot;
  while (curr) {
    next = curr->next;
    if (curr->param == slot_data) {
      if (prev)
        prev->next = next;
      else
        data->head = next;
      delete curr;
    } else
      prev = curr;
    curr = next;
  }
  data->tail = prev;
}

bool
Am_Demon_Queue::Empty() const
{
  return !data->head;
}

void
Am_Demon_Queue::Invoke()
{
  if (data->invoke_stack || !data->active)
    return;
  Prevent_Invoke();
  Am_PUSH_EMPTY_CC() while (data->head)
  {
    QItem *curr = data->head;
    curr->demon(curr->param);
    if (data->head == curr) {
      data->head = data->head->next;
      delete curr;
    } else
      curr->param = nullptr;
    if (!data->head)
      data->tail = nullptr;
  }
  Am_POP_EMPTY_CC() Release_Invoke();
}

void
Am_Demon_Queue::Prevent_Invoke()
{
  ++data->invoke_stack;
}

void
Am_Demon_Queue::Release_Invoke()
{
  --data->invoke_stack;
}

void
Am_Demon_Queue::Shutdown()
{
  data->active = false;
}

void
Am_Demon_Queue::Reset()
{
  if (!data->active)
    return;

  QItem *curr = data->head;
  QItem *next;

  while (curr) {
    next = curr->next;
    delete curr;
    curr = next;
  }
  data->head = data->tail = nullptr;
  data->invoke_stack = 0;
}

Am_Demon_Queue &
Am_Demon_Queue::operator=(const Am_Demon_Queue &proto)
{
  Am_Demon_Queue_Data *old_data = data;
  data = proto.data;
  data->Note_Reference();
  old_data->Release();
  return *this;
}

Am_Demon_Queue::~Am_Demon_Queue() { data->Release(); }

Am_Demon_Queue::Am_Demon_Queue() : data(new Am_Demon_Queue_Data())
{
  //  data = new Am_Demon_Queue_Data ();
}

Am_Demon_Queue::Am_Demon_Queue(const Am_Demon_Queue &proto) : data(proto.data)
{
  data->Note_Reference();
}

static void
Am_Validate_Demon(Am_Slot slot)
{
  slot.Validate();
}

Am_Demon_Set_Data::Am_Demon_Set_Data()
    : refs(1), create_demon(nullptr), copy_demon(nullptr), destroy_demon(nullptr),
      change_owner_demon(nullptr), add_part_demon(nullptr), change_length(0),
      invalid_length(1), max_type_check(0), type_check_list(nullptr)
{
  invalid_demons[0].demon = Am_Validate_Demon;
  invalid_demons[0].protocol = Am_DEMON_PER_SLOT;
  invalid_demons[0].which_bit = 0x0001;
}

Am_Object_Demon *
Am_Demon_Set::Get_Object_Demon(Am_Object_Demon_Type type) const
{
  switch (type) {
  case Am_CREATE_OBJ:
    return data->create_demon;
  case Am_COPY_OBJ:
    return data->copy_demon;
  case Am_DESTROY_OBJ:
    return data->destroy_demon;
  }
  return nullptr;
}

void
Am_Demon_Set::Set_Object_Demon(Am_Object_Demon_Type type,
                               Am_Object_Demon *demon)
{
  switch (type) {
  case Am_CREATE_OBJ:
    data->create_demon = demon;
    break;
  case Am_COPY_OBJ:
    data->copy_demon = demon;
    break;
  case Am_DESTROY_OBJ:
    data->destroy_demon = demon;
    break;
  }
}

Am_Slot_Demon *
Am_Demon_Set::Get_Slot_Demon(unsigned short which_bit) const
{
  int i;
  for (i = 0; i < data->invalid_length; ++i)
    if (data->invalid_demons[i].which_bit == which_bit)
      return data->invalid_demons[i].demon;
  for (i = 0; i < data->change_length; ++i)
    if (data->change_demons[i].which_bit == which_bit)
      return data->change_demons[i].demon;
  return nullptr;
}

void
Am_Demon_Set::Set_Slot_Demon(unsigned short which_bit, Am_Slot_Demon *method,
                             Am_Demon_Protocol protocol)
{
  int i;
  for (i = 0; i < data->invalid_length; ++i)
    if (data->invalid_demons[i].which_bit == which_bit) {
      --data->invalid_length;
      data->invalid_demons[i] = data->invalid_demons[data->invalid_length];
      break;
    }
  for (i = 0; i < data->change_length; ++i)
    if (data->change_demons[i].which_bit == which_bit) {
      --data->change_length;
      data->change_demons[i] = data->change_demons[data->change_length];
      break;
    }
  if (protocol & Am_DEMON_ON_CHANGE) {
    data->change_demons[data->change_length].demon = method;
    data->change_demons[data->change_length].which_bit = which_bit;
    data->change_demons[data->change_length].protocol = protocol & 0x0001;
    ++data->change_length;
  }
  if (protocol & Am_DEMON_ON_INVALIDATE) {
    data->invalid_demons[data->invalid_length].demon = method;
    data->invalid_demons[data->invalid_length].which_bit = which_bit;
    data->invalid_demons[data->invalid_length].protocol = protocol & 0x0001;
    ++data->invalid_length;
  }
}

Am_Part_Demon *
Am_Demon_Set::Get_Part_Demon(Am_Part_Demon_Type type) const
{
  switch (type) {
  case Am_ADD_PART:
    return data->add_part_demon;
  case Am_CHANGE_OWNER:
    return data->change_owner_demon;
  }
  return nullptr;
}

void
Am_Demon_Set::Set_Part_Demon(Am_Part_Demon_Type type, Am_Part_Demon *demon)
{
  switch (type) {
  case Am_ADD_PART:
    data->add_part_demon = demon;
    break;
  case Am_CHANGE_OWNER:
    data->change_owner_demon = demon;
    break;
  }
}

Am_Type_Check *
Am_Demon_Set::Get_Type_Check(unsigned short type) const
{
  if (type < data->max_type_check)
    return data->type_check_list[type].func;
  else
    return nullptr;
}

void
Am_Demon_Set::Set_Type_Check(unsigned short type, Am_Type_Check *demon)
{
  if (type > data->max_type_check) {
    Type_Holder *new_list = new Type_Holder[type];
    unsigned int i;
    for (i = 0; i < data->max_type_check; ++i)
      new_list[i] = data->type_check_list[i];
    for (i = data->max_type_check + 1; i < type; ++i)
      new_list[i].func = nullptr;
    if (data->type_check_list)
      delete data->type_check_list;
    data->type_check_list = new_list;
    data->max_type_check = type;
  }
  data->type_check_list[type - 1].func = demon;
}

Am_Demon_Set
Am_Demon_Set::Copy() const
{
  return new Am_Demon_Set_Data(data);
}

Am_Demon_Set &
Am_Demon_Set::operator=(const Am_Demon_Set &proto)
{
  Am_Demon_Set_Data *old_data = data;
  data = proto.data;
  data->Note_Reference();
  old_data->Release();
  return *this;
}

Am_Demon_Set::~Am_Demon_Set() { data->Release(); }

Am_Demon_Set::Am_Demon_Set() : data(new Am_Demon_Set_Data())
{
  //  data = new Am_Demon_Set_Data ();
}

Am_Demon_Set::Am_Demon_Set(const Am_Demon_Set &proto) : data(proto.data)
{
  data->Note_Reference();
}

Am_Instance_Iterator::Am_Instance_Iterator()
    : current(Am_No_Object), prototype(Am_No_Object)
{
}

Am_Instance_Iterator::Am_Instance_Iterator(Am_Object object)
    : current(Am_No_Object), prototype(object)
{
}

Am_Instance_Iterator &
Am_Instance_Iterator::operator=(Am_Object object)
{
  prototype = object;
  current = Am_No_Object;
  return *this;
}

unsigned short
Am_Instance_Iterator::Length()
{
  if (prototype.Valid()) {
    Am_Object_Data *i;
    unsigned short count = 0;
    Am_Object_Data *proto_data = Am_Object_Data::Narrow(prototype);
    for (i = proto_data->first_instance; i != nullptr; i = i->next_instance)
      ++count;
    proto_data->Release();
    return count;
  } else
    return 0;
}

void
Am_Instance_Iterator::Start()
{
  if (prototype.Valid()) {
    Am_Object_Data *proto_data = Am_Object_Data::Narrow(prototype);
    if (proto_data->first_instance)
      proto_data->first_instance->Note_Reference();
    current = proto_data->first_instance;
    proto_data->Release();
  }
}

void
Am_Instance_Iterator::Next()
{
  if (current.Valid()) {
    Am_Object_Data *curr_data = Am_Object_Data::Narrow(current);
    if (curr_data->next_instance)
      curr_data->next_instance->Note_Reference();
    current = curr_data->next_instance;
    curr_data->Release();
  } else if (prototype.Valid()) {
    Am_Object_Data *proto_data = Am_Object_Data::Narrow(prototype);
    if (proto_data->first_instance)
      proto_data->first_instance->Note_Reference();
    current = proto_data->first_instance;
    proto_data->Release();
  }
}

bool
Am_Instance_Iterator::Last()
{
  return !current.Valid();
}

Am_Object
Am_Instance_Iterator::Get()
{
  return current;
}

class Am_Slot_Iterator_Data
{
public:
  Am_Slot_Iterator_Data()
  {
    curr_pos = 0;
    curr_obj = nullptr;
  }
  ~Am_Slot_Iterator_Data()
  {
    if (curr_obj)
      curr_obj->Release();
  }
  void Find_Actual(Am_Object_Data *context);
  unsigned curr_pos;
  Am_Object_Data *curr_obj;
};

void
Am_Slot_Iterator_Data::Find_Actual(Am_Object_Data *context)
{
  Am_Slot_Key key;
  while (curr_obj) {
    if (curr_pos >= curr_obj->data.length) {
      curr_obj->Release();
      curr_obj = curr_obj->prototype;
      if (curr_obj)
        curr_obj->Note_Reference();
      curr_pos = 0;
    } else {
      key = ((Am_Slot_Data **)curr_obj->data.data)[curr_pos]->key;
      context->Note_Reference();
      curr_obj->Note_Reference();
      if (Am_Object_Advanced(context).Get_Slot_Locale(key) ==
          Am_Object(curr_obj))
        return;
      ++curr_pos;
    }
  }
}

Am_Slot_Iterator::Am_Slot_Iterator()
    : context(Am_No_Object), data(new Am_Slot_Iterator_Data)
{
  //  data = new Am_Slot_Iterator_Data;
}

Am_Slot_Iterator::Am_Slot_Iterator(Am_Object object)
    : context(object), data(new Am_Slot_Iterator_Data)
{
  //  data = new Am_Slot_Iterator_Data;
}

Am_Slot_Iterator::~Am_Slot_Iterator() { delete data; }

Am_Slot_Iterator &
Am_Slot_Iterator::operator=(Am_Object object)
{
  data->curr_pos = 0;
  data->curr_obj = nullptr;
  context = object;
  return *this;
}

unsigned short
Am_Slot_Iterator::Length()
{
  if (context.Valid()) {
    int count = 0;
    Am_Slot_Iterator si = context;
    for (si.Start(); !si.Last(); si.Next())
      ++count;
    return count;
  } else
    return 0;
}

void
Am_Slot_Iterator::Start()
{
  if (context.Valid()) {
    data->curr_obj = Am_Object_Data::Narrow(context);
    data->curr_pos = 0;
    Am_Object_Data *cont_data = Am_Object_Data::Narrow(context);
    data->Find_Actual(cont_data);
    cont_data->Release();
  }
}
void
Am_Slot_Iterator::Next()
{
  if (data->curr_obj) {
    ++data->curr_pos;
    Am_Object_Data *cont_data = Am_Object_Data::Narrow(context);
    data->Find_Actual(cont_data);
    cont_data->Release();
  } else
    Start();
}

bool
Am_Slot_Iterator::Last()
{
  return !data->curr_obj;
}

Am_Slot_Key
Am_Slot_Iterator::Get()
{
  if (data->curr_obj)
    return ((Am_Slot_Data **)data->curr_obj->data.data)[data->curr_pos]->key;
  else
    return Am_NO_SLOT;
}

Am_Part_Iterator::Am_Part_Iterator()
    : current(Am_No_Object), owner(Am_No_Object)
{
  ;
}

Am_Part_Iterator::Am_Part_Iterator(Am_Object object)
    : current(Am_No_Object), owner(object)
{
  ;
}

Am_Part_Iterator &
Am_Part_Iterator::operator=(Am_Object object)
{
  current = Am_No_Object;
  owner = object;
  return *this;
}

unsigned short
Am_Part_Iterator::Length()
{
  if (owner.Valid()) {
    Am_Object_Data *i;
    unsigned short count = 0;
    Am_Object_Data *owner_data = Am_Object_Data::Narrow(owner);
    for (i = owner_data->first_part; i != nullptr; i = i->next_part)
      ++count;
    owner_data->Release();
    return count;
  } else
    return 0;
}

void
Am_Part_Iterator::Start()
{
  if (owner.Valid()) {
    Am_Object_Data *owner_data = Am_Object_Data::Narrow(owner);
    if (owner_data->first_part)
      owner_data->first_part->Note_Reference();
    current = owner_data->first_part;
    owner_data->Release();
  }
}

void
Am_Part_Iterator::Next()
{
  if (current.Valid()) {
    Am_Object_Data *curr_data = Am_Object_Data::Narrow(current);
    if (curr_data->next_part)
      curr_data->next_part->Note_Reference();
    current = curr_data->next_part;
    curr_data->Release();
  } else if (owner.Valid()) {
    Am_Object_Data *owner_data = Am_Object_Data::Narrow(owner);
    if (owner_data->first_part)
      owner_data->first_part->Note_Reference();
    current = owner_data->first_part;
    owner_data->Release();
  }
}

bool
Am_Part_Iterator::Last()
{
  return !current.Valid();
}

Am_Object
Am_Part_Iterator::Get()
{
  return current;
}

static void
slot_error(const char *error_string, Am_Slot_Data *slot)
{
  slot->context->Note_Reference();
  Am_Object obj = Am_Object(slot->context);
  std::cerr << "** " << error_string << "  Object = " << obj << ", Slot = ";
  Am_Print_Key(std::cerr, slot->key);
  std::cerr << "." << std::endl;
  Am_Error(obj, slot->key);
}

void
slot_error(const char *error_string, const Am_Object &object, Am_Slot_Key key)
{
  std::cerr << "** " << error_string << "  Object = " << object << ", Slot = ";
  Am_Print_Key(std::cerr, key);
  std::cerr << "." << std::endl;
  Am_Error((Am_Object &)object, key);
}

void
slot_error(const char *error_string, Am_Object_Data *object_data,
           Am_Slot_Key key)
{
  object_data->Note_Reference();
  Am_Object object = object_data;
  std::cerr << "** " << error_string << "  Object = " << object << ", Slot = ";
  Am_Print_Key(std::cerr, key);
  std::cerr << "." << std::endl;
  Am_Error(object, key);
}

void
slot_error(const char *error_string, Am_Slot_Key key)
{
  std::cerr << "** " << error_string << ", Slot = ";
  Am_Print_Key(std::cerr, key);
  std::cerr << "." << std::endl;
  Am_Error(Am_No_Object, key);
}

#ifdef MEMORY
#include <stddef.h>
#endif

class CItem
{
public:
#ifdef MEMORY
  void *operator new(size_t) { return memory.New(); }
  void operator delete(void *ptr, size_t) { memory.Delete(ptr); }
  static Dyn_Memory_Manager memory;
#endif
  bool Get(const Am_Slot &validating_slot, Am_Value &out_value, bool &changed)
  {
    return value->Get(validating_slot, out_value, changed);
  }

  Am_Constraint *value;
  union
  {
    CItem *prev;
    CItem *next_invalid;
  };
  CItem *next;
};

#ifdef MEMORY
Dyn_Memory_Manager CItem::memory(sizeof(CItem), "ConstraintItem");
#endif

Am_Constraint_Iterator::Am_Constraint_Iterator() : current(nullptr) { ; }

Am_Constraint_Iterator::Am_Constraint_Iterator(const Am_Slot &slot)
    : context(slot), current(nullptr)
{
  ;
}

Am_Constraint_Iterator &
Am_Constraint_Iterator::operator=(const Am_Slot &slot)
{
  context = slot;
  current = nullptr;
  return *this;
}

unsigned short
Am_Constraint_Iterator::Length() const
{
  if ((Am_Slot_Data *)context) {
    unsigned short count = 0;
    CItem *current = ((Am_Slot_Data *)context)->constraints.head;
    while (current) {
      ++count;
      current = current->next;
    }
    return count;
  } else
    return 0;
}

void
Am_Constraint_Iterator::Start()
{
  if (context)
    current = (Am_Constraint_Iterator_Data *)((Am_Slot_Data *)context)
                  ->constraints.head;
}
void
Am_Constraint_Iterator::Next()
{
  if (current)
    current = (Am_Constraint_Iterator_Data *)((CItem *)current)->next;
  else if (context)
    current = (Am_Constraint_Iterator_Data *)((Am_Slot_Data *)context)
                  ->constraints.head;
}

bool
Am_Constraint_Iterator::Last() const
{
  return !current;
}

Am_Constraint *
Am_Constraint_Iterator::Get() const
{
  if (current)
    return ((CItem *)current)->value;
  else
    return nullptr;
}

Am_Constraint_Tag
Am_Constraint_Iterator::Get_Tag() const
{
  return (Am_Constraint_Tag)current;
}

Am_Dependency_Iterator::Am_Dependency_Iterator() : current(nullptr) { ; }

Am_Dependency_Iterator::Am_Dependency_Iterator(const Am_Slot &slot)
    : context(slot), current(nullptr)
{
  ;
}

Am_Dependency_Iterator &
Am_Dependency_Iterator::operator=(const Am_Slot &slot)
{
  context = slot;
  current = nullptr;
  return *this;
}

unsigned short
Am_Dependency_Iterator::Length() const
{
  if (context) {
    unsigned short count = 0;
    CItem *current = ((Am_Slot_Data *)context)->dependencies.head;
    while (current) {
      ++count;
      current = current->next;
    }
    return count;
  } else
    return 0;
}

void
Am_Dependency_Iterator::Start()
{
  if (context)
    current = (Am_Dependency_Iterator_Data *)((Am_Slot_Data *)context)
                  ->dependencies.head;
}
void
Am_Dependency_Iterator::Next()
{
  if (current)
    current = (Am_Dependency_Iterator_Data *)((CItem *)current)->next;
  else if (context)
    current = (Am_Dependency_Iterator_Data *)((Am_Slot_Data *)context)
                  ->dependencies.head;
}

bool
Am_Dependency_Iterator::Last() const
{
  return !current;
}

Am_Constraint *
Am_Dependency_Iterator::Get() const
{
  if (current)
    return ((CItem *)current)->value;
  else
    return nullptr;
}

Am_Constraint_Tag
Am_Dependency_Iterator::Get_Tag() const
{
  return (Am_Constraint_Tag)current;
}

/*****************************************************************************
 * Am_Object class methods                                                   *
 *****************************************************************************/

Am_Object::Am_Object(const Am_Object &prev) : data(prev.data)
{
  if (data) {
    data->Note_Reference();
  }
}

Am_Object::Am_Object(const Am_Value &in_value)
    : data(static_cast<Am_Object_Data *>(in_value.value.wrapper_value))
{
  if (data) {
    if (Am_Object_Data::Am_Object_Data_ID() != in_value.type)
      Am_ERROR("** Tried to set Am_Object with a non Am_Object value "
               << in_value);
    data->Note_Reference();
  }
}

Am_Object::~Am_Object()
{
  if (data) {
    if (data->Is_Zero())
      Am_ERROR("** Tried to delete object twice." << *this);
    data->Release();
  }
  data = nullptr;
}

Am_Object &
Am_Object::operator=(const Am_Object &prev)
{
  Am_Object_Data *old_data = data;
  data = prev.data;
  if (data)
    data->Note_Reference();
  if (old_data)
    old_data->Release();
  return *this;
}

Am_Object &
Am_Object::operator=(const Am_Value &in_value)
{
  if (in_value.value.wrapper_value &&
      (in_value.type != Am_Object_Data::Am_Object_Data_ID() &&
       in_value.type != Am_ZERO))
    Am_ERRORO("** Tried to assign Am_Object "
                  << *this << " with a non Am_Object value " << in_value,
              *this, Am_NO_SLOT);
  Am_Object_Data *old_data = data;
  data = (Am_Object_Data *)in_value.value.wrapper_value;
  if (data) {
    data->Note_Reference();
  }
  if (old_data)
    old_data->Release();
  return *this;
}

Am_Object &
Am_Object::operator=(Am_Object_Data *in_data)
{
  if (data)
    data->Release();
  data = in_data;
  return *this;
}

Am_Object::operator Am_Wrapper *() const
{
  if (data)
    data->Note_Reference();
  return data;
}

bool
Am_Object::Valid() const
{
  return (data != nullptr) && (data->data.data != nullptr);
}

bool
Am_Object::Test(const Am_Wrapper *in_data)
{
  if (in_data)
    return in_data->ID() == Am_Object_Data::Am_Object_Data_ID();
  else
    return false;
}

bool
Am_Object::Test(const Am_Value &in_value)
{
  return (in_value.value.wrapper_value &&
          (in_value.type == Am_Object_Data::Am_Object_Data_ID()));
}

Am_Object
Am_Object::Create(const char *new_name) const
{
  if (!data)
    Am_ERROR("** Create called on a (0L) object." << *this);

  // create a local object and fill it up
  Am_Object new_object;
  new_object.data = data->create_object(new_name);
  new_object.data->demon_queue.Prevent_Invoke();
  Am_PUSH_EMPTY_CC() new_object.data->invoke_create_demons();
  Am_POP_EMPTY_CC() new_object.data->demon_queue.Release_Invoke();
  new_object.data->validate_object();
  new_object.data->note_parts();
#ifdef DEBUG
  Global_Call_Object_Trace(*this, new_object, Am_TRACE_OBJECT_CREATED);
#endif

  // return a copy of the object to the caller
  return new_object;
}

Am_Object
Am_Object::Copy(const char *new_name) const
{
  if (data) {
    if (data->demons_active & DEMONS_ACTIVE)
      data->demon_queue.Invoke();
    Am_Object new_object;
    new_object.data = data->copy_object(new_name);
    new_object.data->demon_queue.Prevent_Invoke();
    Am_PUSH_EMPTY_CC() new_object.data->invoke_copy_demons();
    Am_POP_EMPTY_CC() new_object.data->demon_queue.Release_Invoke();
    new_object.data->validate_object();
    new_object.data->note_parts();
#ifdef DEBUG
    Global_Call_Object_Trace(*this, new_object, Am_TRACE_OBJECT_COPIED);
#endif
    return new_object;
  } else
    return Am_Root_Object;
}

Am_Object
Am_Object::Copy_Value_Only(const char *new_name) const
{
  if (data) {
    if (data->demons_active & DEMONS_ACTIVE)
      data->demon_queue.Invoke();
    Am_Object new_object;
    new_object.data = data->copy_object_value_only(new_name);
    new_object.data->demon_queue.Prevent_Invoke();
    Am_PUSH_EMPTY_CC() new_object.data->invoke_copy_demons();
    Am_POP_EMPTY_CC() new_object.data->demon_queue.Release_Invoke();
    new_object.data->note_parts();
#ifdef DEBUG
    Global_Call_Object_Trace(*this, new_object, Am_TRACE_OBJECT_COPIED);
#endif
    return new_object;
  } else
    return Am_Root_Object;
}

bool
Am_Object::operator==(const Am_Object &test_object) const
{
  return data == test_object.data;
}

bool
Am_Object::operator!=(const Am_Object &test_object) const
{
  return data != test_object.data;
}

bool
Am_Object::Is_Instance_Of(Am_Object in_prototype) const
{
  Am_Object_Data *current = data;
  while (current) {
    if (current == in_prototype.data)
      return true;
    current = current->prototype;
  }
  return false;
}

bool
Am_Object::Is_Part_Of(Am_Object in_owner) const
{
  Am_Object_Data *current = data;
  while (current) {
    if (current == in_owner.data)
      return true;
    current = (Am_Object_Data *)current->owner_slot.value.wrapper_value;
  }
  return false;
}

void
Am_Object::Text_Inspect(Am_Slot_Key key) const
{
  std::cout << "\nFor Object " << *this << " (" << (void *)data << ")\n";
  if (data) {
    Am_Slot_Data *slot = data->find_slot(key);
    std::cout << "  Slot ";
    data->print_slot_name_and_value(key, slot);
  } else
    std::cout << "  ** Object is not Valid\n";
  std::cout << std::flush;
}

void
Am_Object::Text_Inspect() const
{
  std::cout << "\nPrinting Slots of Object " << *this << " (" << (void *)data
            << ")\n";
  if (data) {
    Am_Object o = this->Get_Prototype();
    std::cout << " Instance of: " << o << std::endl;
    o = this->Get_Owner();
    std::cout << " Part of: " << o << std::endl;

    Am_Slot_Data *slot;
    Am_Slot_Iterator slot_iter(*this);
    Am_Slot_Key key;
    for (slot_iter.Start(); !slot_iter.Last(); slot_iter.Next()) {
      key = slot_iter.Get();
      slot = data->find_slot(key);
      std::cout << "  ";
      data->print_slot_name_and_value(key, slot);
    }
  } else
    std::cout << "  ** Object is not Valid\n";
  std::cout << std::flush;
}

const Am_Value &
Am_Object::Get(Am_Slot_Key key, Am_Slot_Flags flags) const
{
  // *** IF YOU MODIFY THIS METHOD, be sure to modify Get(Am_Slot_Key key), which specializes it

  if (!data || !data->data.data)
    return cc->Raise_Get_Exception(Get_On_Null_Object_Value,
                                   (const Am_Object_Advanced &)*this, key,
                                   flags, "Get called on (0L) object");

  if (data->demons_active & DEMONS_ACTIVE)
    data->demon_queue.Invoke();

  const Am_Value &value =
      cc->Get((const Am_Object_Advanced &)*this, key, flags);

  if (Am_Type_Class(value.type) == Am_ERROR_VALUE_TYPE)
    return cc->Raise_Get_Exception(value, (const Am_Object_Advanced &)*this,
                                   key, flags, nullptr);
#ifdef DEBUG
  Am_Value v = value; // type-check error in VC++
  Global_Check_Get_Method_Trace(*this, key, v);
#endif
  return value;
}

//////////////////////////////////
//Am_Object doesn't use AM_WRAPPER_IMPL, so repeat all the
//printing functions here

const char *
Am_Object::To_String() const
{
  if (data)
    return data->To_String();
  else
    return nullptr;
}

Am_Value
Am_Object::From_String(const char *string)
{
  if (data)
    return data->From_String(string);
  else
    return Am_No_Value;
}
void
Am_Object::Print(std::ostream &out) const
{
  if (data)
    data->Print(out);
  else
    out << "(0L) Object";
}

Am_Object &
Am_Object::Set_Name(const char *in_name)
{
// these next two lines should be commented out by macros in registry.h
// when we're not in DEBUG mode.
#ifdef DEBUG
  const char *name = Am_Get_Name_Of_Item(data);
  Am_Unregister_Name(data);
  delete[] name;
  char *new_name = new char[strlen(in_name) + 1];
  strcpy(new_name, in_name);
  Am_Register_Name(data, new_name);
#endif
  return *this;
}

const char *
Am_Object::Get_Name() const
{
  // this will be commented out by macros in registry.h in non-DEBUG mode
  return Am_Get_Name_Of_Item(data);
}

Am_Slot_Key
Am_Object::Get_Key() const
{
  if (!data)
    Am_ERROR("** Get_Key called on a (0L) object." << (Am_Object &)*this);
  return data->part_slot.key;
}

Am_Object
Am_Object::Get_Prototype() const
{
  if (!data)
    Am_ERROR("** Get_Prototype called on a (0L) object." << (Am_Object &)*this);
  if (data->prototype)
    data->prototype->Note_Reference();
  return data->prototype;
}

Am_Object
Am_Object::Get_Object(Am_Slot_Key key, Am_Slot_Flags flags) const
{
  const Am_Value &v = Get(key, flags | Am_RETURN_ZERO_ON_ERROR_BIT);

  if (v.type == Am_OBJECT)
    return v;
  else if (v.value.wrapper_value == nullptr)
    return Am_No_Object;
  else
    slot_error("Slot does not contain a value of type Object", *this, key);
  return Am_No_Object; //should never get here
}

bool
Am_Object::Is_Part_Slot(Am_Slot_Key key) const
{
  if (data) {
    Am_Slot_Data *slot = data->find_slot(key);
    if (slot) {
      if (slot->flags & BIT_IS_PART)
        return true;
    }
  }
  return false;
}

bool
Am_Object::Is_Slot_Inherited(Am_Slot_Key key) const
{
  if (data) {
    Am_Slot slot = data->find_slot(key);
    if (slot) {
      // slot.Validate ();
      return (slot.Get_Owner() != *this) || slot.Is_Inherited();
    }
  }
  return false;
}

void
Am_Object::Make_Unique(Am_Slot_Key key)
{
  if (data) {
    if (data->demons_active & DEMONS_ACTIVE)
      data->demon_queue.Invoke();
    data->Note_Reference();
    Am_Object_Advanced(data).Get_Slot(key).Make_Unique();
  }
}

bool
Am_Object::Is_Unique(Am_Slot_Key key)
{
  if (data) {
    if (data->demons_active & DEMONS_ACTIVE)
      data->demon_queue.Invoke();
    Am_Slot_Data *slot = data->find_slot(key);
    return (slot->context == data) && !(slot->flags & BIT_INHERITS) &&
           (!Am_Type_Is_Ref_Counted(slot->type) || !slot->value.wrapper_value ||
            (slot->value.wrapper_value->Ref_Count() == 1));
  }
  return true;
}

#define Make_Set(type)                                                         \
  Am_Object &Am_Object::Set(Am_Slot_Key key, type value,                       \
                            Am_Slot_Flags set_flags)                           \
  {                                                                            \
    return Set(key, Am_Value(value), set_flags);                               \
  }

Make_Set(Am_Wrapper *) Make_Set(Am_Ptr) Make_Set(int) Make_Set(long)
    Make_Set(bool) Make_Set(float) Make_Set(double) Make_Set(char)
        Make_Set(const char *) Make_Set(const Am_String &)
            Make_Set(Am_Generic_Procedure *) Make_Set(Am_Method_Wrapper *)

                Am_Object &Am_Object::Set(Am_Slot_Key key,
                                          const Am_Value &value,
                                          Am_Slot_Flags set_flags)
{
  if (!data)
    Am_ERROR("** Set called on a (0L) object." << *this);
  cc->Set((const Am_Object_Advanced &)*this, key, value,
          set_flags | Am_OK_IF_THERE);
  return *this;
}

Am_Object &
Am_Object::Set(Am_Slot_Key key, Am_Constraint *constraint,
               Am_Slot_Flags set_flags)
{
  if (!data)
    Am_ERROR("** Set called on a (0L) object." << *this);
  data->set_slot(key, constraint, set_flags | Am_OK_IF_THERE);
  return *this;
}

#undef Make_Set

#define Make_Add(type)                                                         \
  Am_Object &Am_Object::Add(Am_Slot_Key key, type value,                       \
                            Am_Slot_Flags set_flags)                           \
  {                                                                            \
    if (!data)                                                                 \
      Am_ERROR("** Add called on a (0L) object." << *this);                    \
    Am_Value holder(value);                                                    \
    cc->Set((const Am_Object_Advanced &)*this, key, holder,                    \
            set_flags | Am_OK_IF_NOT_THERE);                                   \
    return *this;                                                              \
  }

Make_Add(Am_Wrapper *) Make_Add(Am_Ptr) Make_Add(int) Make_Add(long)
    Make_Add(bool) Make_Add(float) Make_Add(double) Make_Add(char)
        Make_Add(const char *) Make_Add(const Am_String &)
            Make_Add(Am_Generic_Procedure *) Make_Add(Am_Method_Wrapper *)

                Am_Object &Am_Object::Add(Am_Slot_Key key,
                                          const Am_Value &value,
                                          Am_Slot_Flags set_flags)
{
  if (!data)
    Am_ERROR("** Add called on a (0L) object." << *this);
  cc->Set((const Am_Object_Advanced &)*this, key, value,
          set_flags | Am_OK_IF_NOT_THERE);
  return *this;
}

Am_Object &
Am_Object::Add(Am_Slot_Key key, Am_Constraint *constraint,
               Am_Slot_Flags set_flags)
{
  if (!data)
    Am_ERROR("** Add called on a (0L) object." << *this);
  data->set_slot(key, constraint, set_flags | Am_OK_IF_NOT_THERE);
  return *this;
}

#undef Make_Add

void
Am_Object::Note_Changed(Am_Slot_Key key)
{
  if (!data || !data->data.data)
    cc->Raise_Get_Exception(Get_On_Null_Object_Value,
                            (const Am_Object_Advanced &)*this, key, 0,
                            "Note_Changed called on (0L) object");
  else
    cc->Note_Changed((const Am_Object_Advanced &)*this, key);
}

void
Am_Object::Note_Unchanged(Am_Slot_Key key)
{
  if (!data || !data->data.data)
    cc->Raise_Get_Exception(Get_On_Null_Object_Value,
                            (const Am_Object_Advanced &)*this, key, 0,
                            "Note_Unchanged called on (0L) object");
  else
    cc->Note_Unchanged((const Am_Object_Advanced &)*this, key);
}

// Adds a part at the beginning of the part-list.
//
// Returns a reference to the actual object, now having one more part.
Am_Object &
Am_Object::Add_Part(Am_Object new_part, bool inherit, long aPosition)
{
  // is this a valid object?
  if (data == static_cast<Am_Object_Data *>(nullptr)) {
    Am_ERROR("Add_Part called on a (0L) object" << *this);
  }

  // do we have a valid part?
  if (new_part.Valid()) {
    // get the OpenAmulet object data
    Am_Object_Data *added_part = new_part.data;

    // does this object has an other owner?
    if (added_part->owner_slot.value.wrapper_value !=
        static_cast<Am_Wrapper *>(nullptr)) {
      std::cerr << "** Attempted to add object, " << new_part
                << ", to another owner " << *this << "." << std::endl;
      std::cerr << "** Object's owner is " << new_part.Get_Owner() << "."
                << std::endl;
      Am_ERROR(new_part);
    }

    // add a reference to the part
    added_part->Note_Reference();

    // set the owner of the part to ourself
    added_part->owner_slot.value.wrapper_value = data;
    added_part->part_slot.context = data;

    if (aPosition != 0 && data->first_part) {
      // start at the beginning of the list
      Am_Object_Data *anchor = data->first_part;

      // move to the insert-position, check if the list is long enough
      for (aPosition--; (aPosition != 0) && (anchor->next_part != nullptr);
           --aPosition) {
        // ok, get next part
        anchor = anchor->next_part;
      }

      // insert the new part behind the anchor point, but because we
      // subtracted one in the loop initializer, the new part is added
      // before the given position
      added_part->next_part = anchor->next_part;
      anchor->next_part = added_part;
    } else {
      // bush the new part to the front of the part-list
      added_part->next_part = data->first_part;
      data->first_part = added_part;
    }

    added_part->part_slot.key = inherit ? Am_NO_SLOT : Am_NO_INHERIT;
    data->demon_queue.Prevent_Invoke();
    Am_PUSH_EMPTY_CC()

        if ((data->demons_active & DEMONS_ACTIVE) &&
            data->demon_set->add_part_demon)
    {
      data->demon_set->add_part_demon(*this, Am_No_Object, new_part);
    }

    if ((added_part->demons_active & DEMONS_ACTIVE) &&
        added_part->demon_set->change_owner_demon) {
      added_part->demon_set->change_owner_demon(new_part, Am_No_Object, *this);
    }

    Am_POP_EMPTY_CC() data->demon_queue.Release_Invoke();
    data->Note_Reference();

    Am_Value oldval(Am_No_Object), newval(data);

    added_part->owner_slot.dependencies.Change(&added_part->part_slot, nullptr,
                                               oldval, newval);
    newval = new_part;
    added_part->part_slot.dependencies.Change(&added_part->part_slot, nullptr,
                                              oldval, newval);
  }

  // return ourself to the caller, now with one part more
  return (*this);
}

Am_Object &
Am_Object::Add_Part(Am_Slot_Key key, Am_Object new_part,
                    Am_Slot_Flags set_flags)
{
  if (!data)
    slot_error("Add_Part called on a (0L) object", key);
  Am_Slot_Data *slot = data->find_slot(key);
  Am_Object prev_part;
  if (slot && slot->type != Am_MISSING_SLOT) {
    if ((slot->flags & BIT_IS_PART)) {
      if (!(set_flags & Am_OK_IF_THERE)) {
        slot_error("Add_Part called on part slot which is already there.  "
                   "Use Set_Part instead",
                   slot);
        return Am_No_Object;
      }
      prev_part = Get_Object(key); //not very efficient, but safer
    } else {
      if (!(set_flags & Am_OK_IF_THERE)) {
        slot->context->Note_Reference();
        Am_Object obj = Am_Object(slot->context);
        std::cerr << "** Add_Part can only be called with slots that are not "
                     "already in the object.  But Object "
                  << obj << " already has slot ";
        Am_Print_Key(std::cerr, slot->key);
        std::cerr << ".\n";
        Am_Error(obj, slot->key);
        return Am_No_Object;
      }
    }
  }
  if (new_part.Valid()) {
    Am_Object_Data *added_part = new_part.data;
    Am_Object_Data *prev_owner =
        (Am_Object_Data *)added_part->owner_slot.value.wrapper_value;
    if (prev_owner) {
      std::cerr << "** Attempted to add object, " << new_part << ", to object, "
                << *this << ", when it already has owner, " << prev_owner << "."
                << std::endl;
      Am_ERROR(new_part);
    }
    if (slot)
      Remove_Slot(key); //if old slot exists, remove it

    added_part->Note_Reference();
    added_part->owner_slot.value.wrapper_value = data;
    added_part->part_slot.context = data;
    added_part->next_part = data->first_part;
    data->first_part = added_part;
    added_part->part_slot.key = key;
    Am_Slot_Data *new_slot = &(added_part->part_slot);
    data->data.Add((char *)&new_slot);
    data->demon_queue.Prevent_Invoke();
    Am_PUSH_EMPTY_CC() if ((data->demons_active & DEMONS_ACTIVE) &&
                           data->demon_set->add_part_demon)
        data->demon_set->add_part_demon(*this, Am_No_Object, new_part);
    if ((added_part->demons_active & DEMONS_ACTIVE) &&
        added_part->demon_set->change_owner_demon)
      added_part->demon_set->change_owner_demon(new_part, Am_Object(prev_owner),
                                                *this);
    Am_POP_EMPTY_CC() data->demon_queue.Release_Invoke();
    if (prev_owner)
      prev_owner->Note_Reference();
    data->Note_Reference();
    Am_Value oldval(prev_owner), newval(data);
    added_part->owner_slot.dependencies.Change(&added_part->part_slot, nullptr,
                                               oldval, newval);
    oldval = Am_No_Object;
    newval = new_part;
    added_part->part_slot.dependencies.Change(&added_part->part_slot, nullptr,
                                              oldval, newval);
    if (prev_part.Valid())
      prev_part.Destroy();
  }
  return *this;
}

Am_Object &
Am_Object::Set_Part(Am_Slot_Key key, Am_Object new_part,
                    Am_Slot_Flags set_flags)
{
  if (!data)
    slot_error("Set_Part called on a (0L) object", key);
  Am_Slot_Data *slot = data->find_slot(key);
  if (slot && slot->type != Am_MISSING_SLOT) { //slot is there
    if (!(slot->flags & BIT_IS_PART)) {
      Remove_Slot(key);
#if 0
    // keeping parts separate from normal slots turned out to be
    // too constraining -- rcm
      slot_error("Set_Part called with a slot which is not a part slot", slot);
#endif
    }
  } else { //slot not there
    if (!(set_flags & Am_OK_IF_NOT_THERE)) {
      slot_error(
          "Set_Part called on slot which is not there. Use Add_Part instead",
          slot);
    }
  }
  return Add_Part(key, new_part, Am_OK_IF_THERE);
}

Am_Object &
Am_Object::Remove_Slot(Am_Slot_Key key)
{
  if (!data)
    Am_Error("** Remove_Slot called on a (0L) object.");
  Am_Slot_Data *slot;
  unsigned i;
  data->find_slot_and_position(key, slot, i);
  if (slot) {
    slot->dependencies.Invalidate(slot, nullptr, *slot);
    data->data.Delete(i);
    if (!(slot->flags & BIT_IS_PART)) {
      Am_Slot_Data *slot_proto = nullptr;
      if (data->prototype)
        slot_proto = data->prototype->find_slot(key);
      if (slot_proto) {
        data->delete_slot(slot, slot_proto);
        if (slot_proto->enabled_demons & data->prototype->bits_mask) {
          data->Note_Reference();
          Am_Slot_Data *new_slot =
              (Am_Slot_Data *)Am_Object_Advanced(data).Get_Slot(key);
          data->enqueue_change(new_slot);
        }
      } else
        data->delete_slot(slot, nullptr);
    }
  }
  return *this;
}

Am_Object &
Am_Object::Remove_Part(Am_Slot_Key child)
{
  if (!data)
    Am_Error("** Remove_Part called on a (0L) object.");
  Am_Object part = Get_Object(child);
  part.Remove_From_Owner();
  return *this;
}

Am_Object &
Am_Object::Remove_Part(Am_Object child)
{
  child.Remove_From_Owner();
  return *this;
}

void
Am_Object::Remove_From_Owner()
{
  if (!data)
    return;

  Am_Object_Data *owner =
      (Am_Object_Data *)data->owner_slot.value.wrapper_value;

  if (owner) {
    data->remove_part();
    if (owner)
      owner->Note_Reference();
    Am_Value oldval(owner), newval(Am_No_Object);
    data->owner_slot.dependencies.Change(&data->part_slot, nullptr, oldval,
                                         newval);
    data->Note_Reference();
    oldval = data;
    data->part_slot.dependencies.Change(&data->part_slot, nullptr, oldval, newval);
    data->demon_queue.Prevent_Invoke();
    Am_PUSH_EMPTY_CC() if ((data->demons_active & DEMONS_ACTIVE) &&
                           data->demon_set->change_owner_demon)
    {
      owner->Note_Reference();
      data->demon_set->change_owner_demon(*this, owner, Am_No_Object);
    }
    if ((owner->demons_active & DEMONS_ACTIVE) &&
        owner->demon_set->add_part_demon) {
      owner->Note_Reference();
      owner->demon_set->add_part_demon(owner, *this, Am_No_Object);
    }
    Am_POP_EMPTY_CC() data->demon_queue.Release_Invoke();
    data->Release();
  }
}

void
Am_Object::Remove_Constraint(Am_Slot_Key key)
{
  if (!data)
    Am_Error("** Remove_Constraint called on a (0L) object");
  Am_Slot_Data *slot = data->find_slot(key);
  if (slot && slot->context == data) {
    Am_Constraint *constraint;
    Am_Constraint_Tag tag;
    Am_Constraint_Iterator iter((const Am_Slot &)slot);
    iter.Start();
    while (!iter.Last()) {
      constraint = iter.Get();
      tag = iter.Get_Tag();
      iter.Next();
      if (constraint->Get_Prototype() == nullptr) {
        if (slot->rule == Am_INHERIT)
          data->prop_remove_constraint(key, constraint);
        Am_Slot(slot).Remove_Constraint(tag);
      }
    }
  }
}

void
Am_Object::Destroy()
{
#ifdef DEBUG
  Global_Call_Object_Trace(*this, Am_No_Object, Am_TRACE_OBJECT_DESTROYED);
#endif

  if (!data)
    return;

  if (data->owner_slot.value.wrapper_value)
    Remove_From_Owner();

  if (!data->Is_Unique()) {
    Am_Object_Data *current;
    Am_Object_Data *next;

    for (current = data->first_instance; current != nullptr;
         current = current->next_instance)
      current->Note_Reference(); // 1

    current = data->first_instance;
    data->first_instance = nullptr;
    for (; current != nullptr; current = next) {
      next = current->next_instance;
      current->Note_Reference();    // 2
      Am_Object(current).Destroy(); // 2
      current->Release();           // 1
    }

    if (data->data.data)
      data->destroy_object();
  }

  data->Release();
  data = nullptr;
}

//////////////////////////////////////////////////////////////////
// Get and Set "Advanced" properties of slots

Am_Object &
Am_Object::Set_Demon_Bits(Am_Slot_Key key, unsigned short bits)
{
  if (!data)
    Am_Error("Set_Demon_Bits called on a (0L) object.");
  Am_Slot_Data *slot = data->find_slot(key);
  if (!slot || slot->context != data)
    slot_error("Set_Demon_Bits: slot does not exist or is not local", *this,
               key);
  Am_Slot(slot).Set_Demon_Bits(bits);
  return *this;
}

unsigned short
Am_Object::Get_Demon_Bits(Am_Slot_Key key) const
{
  if (!data)
    Am_Error("Get_Demon_Bits called on a (0L) object.");
  Am_Slot_Data *slot = data->find_slot(key);
  if (!slot)
    slot_error("Get_Demon_Bits: slot does not exist", *this, key);
  return Am_Slot(slot).Get_Demon_Bits();
}

Am_Object &
Am_Object::Set_Inherit_Rule(Am_Slot_Key key, Am_Inherit_Rule rule)
{
  if (!data)
    Am_Error("Set_Inherit_Rule called on a (0L) object.");
  Am_Slot_Data *slot = data->find_slot(key);
  if (!slot || slot->context != data)
    slot_error("Set_Inherit_Rule: slot does not exist or is not local", *this,
               key);
  Am_Slot(slot).Set_Inherit_Rule(rule);
  return *this;
}

Am_Inherit_Rule
Am_Object::Get_Inherit_Rule(Am_Slot_Key key) const
{
  if (!data)
    Am_Error("Get_Inherit_Rule called on a (0L) object.");
  Am_Slot_Data *slot = data->find_slot(key);
  if (!slot)
    slot_error("Get_Inherit_Rule: slot does not exist", *this, key);
  return Am_Slot(slot).Get_Inherit_Rule();
}

Am_Object &
Am_Object::Set_Type_Check(Am_Slot_Key key, unsigned short type)
{
  if (!data)
    Am_Error("Set_Type_Check called on a (0L) object.");
  Am_Slot_Data *slot = data->find_slot(key);
  if (!slot || slot->context != data)
    slot_error("Set_Type_Check: slot does not exist or is not local", *this,
               key);
  Am_Slot(slot).Set_Type_Check(type);
  return *this;
}

unsigned short
Am_Object::Get_Type_Check(Am_Slot_Key key) const
{
  if (!data)
    Am_Error("Get_Type_Check called on a (0L) object.");
  Am_Slot_Data *slot = data->find_slot(key);
  if (!slot)
    slot_error("Get_Type_Check: slot does not exist", *this, key);
  return Am_Slot(slot).Get_Type_Check();
}

Am_Object &
Am_Object::Set_Read_Only(Am_Slot_Key key, bool read_only)
{
  if (!data)
    Am_Error("Set_Read_Only called on a (0L) object.");
  Am_Slot_Data *slot = data->find_slot(key);
  if (!slot || slot->context != data)
    slot_error("Set_Read_Only: slot does not exist or is not local", *this,
               key);
  Am_Slot(slot).Set_Read_Only(read_only);
  return *this;
}

Am_Object &
Am_Object::Validate()
{
  if (!data)
    Am_Error("** Validate called on a (0L) object");
  if (data->demons_active & DEMONS_ACTIVE)
    data->demon_queue.Invoke();
  return *this;
}

bool
Am_Object::Get_Read_Only(Am_Slot_Key key) const
{
  if (!data)
    Am_Error("Get_Read_Only called on a (0L) object.");
  Am_Slot_Data *slot = data->find_slot(key);
  if (!slot)
    slot_error("Get_Read_Only: slot does not exist", *this, key);
  return Am_Slot(slot).Get_Read_Only();
}

Am_Object
Am_Object::Narrow(Am_Wrapper *in_data)
{
  if (in_data) {
    if (Am_Object_Data::Am_Object_Data_ID() == in_data->ID())
      return (Am_Object_Data *)in_data;
    else
      Am_DERR("** Tried to set an Am_Object with a non Am_Object wrapper.");
  }
  return (Am_Object_Data *)nullptr;
}

void
Am_Object::Println() const
{
  Print(std::cout);
  std::cout << std::endl << std::flush;
}

/*****************************************************************************
 * Am_Object_Advanced class methods                                          *
 *****************************************************************************/

Am_Object_Advanced::Am_Object_Advanced()
{
  data = nullptr; // NB: No field named data
}

Am_Object_Advanced::Am_Object_Advanced(Am_Object_Data *in_data)
{
  data = in_data; // NB: No field named data
}

Am_Slot
Am_Object_Advanced::Get_Slot(Am_Slot_Key key) const
{
  if (!data)
    Am_Error("** Get_Slot called on a (0L) object");
  if (!data->data.data)
    Am_Error("** Get_Slot called on a destroyed object");
  if (key == Am_OWNER)
    return &data->owner_slot;

  unsigned i;
  Am_Slot_Data **slot_array;
  Am_Slot_Data *slot;

  for (i = data->data.length, slot_array = (Am_Slot_Data **)data->data.data;
       i > 0; --i, ++slot_array) {
    slot = *slot_array;
    if (key == slot->key)
      return slot;
  }
  if (data->prototype) {
    slot = data->prototype->find_slot(key);
    if (slot && !(slot->flags & BIT_IS_PART)) {
      if (slot->rule == Am_STATIC)
        return slot;
      Am_Slot_Data *new_slot = new Am_Slot_Data(data, key);
      new_slot->flags = slot->flags & DATA_BITS;
      if (slot->rule == Am_INHERIT)
        new_slot->flags |= BIT_INHERITS | BIT_IS_INHERITED;
      new_slot->type_check = slot->type_check;
      new_slot->enabled_demons = slot->enabled_demons;
      new_slot->rule = slot->rule;
      if (slot->rule == Am_LOCAL)
        new_slot->type = Am_MISSING_SLOT;
      else {
        new_slot->type = slot->type;
        if (Am_Type_Is_Ref_Counted(slot->type) && slot->value.wrapper_value)
          slot->value.wrapper_value->Note_Reference();
        new_slot->value = slot->value;
      }
      data->data.Add((char *)&new_slot);
      return new_slot;
    }
  }
  Am_Slot_Data *new_slot = new Am_Slot_Data(data, key);
  new_slot->flags = BIT_INHERITS | BIT_IS_INHERITED;
  new_slot->type = Am_MISSING_SLOT;
  new_slot->value.wrapper_value = nullptr;
  data->data.Add((char *)&new_slot);
  return new_slot;
}

Am_Slot
Am_Object_Advanced::Get_Owner_Slot() const
{
  if (!data)
    Am_Error("** Get_Owner_Slot called on a (0L) object.");
  return &data->owner_slot;
}

Am_Slot
Am_Object_Advanced::Get_Part_Slot() const
{
  if (!data)
    Am_Error("** Get_Part_Slot called on a (0L) object.");
  return &data->part_slot;
}

Am_Object_Advanced
Am_Object_Advanced::Get_Slot_Locale(Am_Slot_Key key) const
{
  if (!data)
    Am_Error("** Get_Slot_Locale called on a (0L) object.");
  Am_Slot_Data *slot = data->find_slot(key);
  if (slot && slot->context) {
    slot->context->Note_Reference();
    return slot->context;
  } else
    return Am_Object_Advanced((nullptr));
}

Am_Demon_Set
Am_Object_Advanced::Get_Demons() const
{
  if (!data)
    Am_Error("** Get_Demons called on a (0L) object.");
  data->demon_set->Note_Reference();
  return data->demon_set;
}

void
Am_Object_Advanced::Set_Demons(const Am_Demon_Set &methods)
{
  if (!data)
    Am_Error("** Set_Demons called on a (0L) object.");
  data->demon_set->Release();
  data->demon_set = methods;
  data->demon_set->Note_Reference();
}

Am_Demon_Queue
Am_Object_Advanced::Get_Queue() const
{
  if (!data)
    Am_Error("** Get_Queue called on a (0L) object.");
  return data->demon_queue;
}

void
Am_Object_Advanced::Set_Queue(const Am_Demon_Queue &queue)
{
  if (!data)
    Am_Error("** Set_Queue called on a (0L) object.");
  data->demon_queue = queue;
}

unsigned short
Am_Object_Advanced::Get_Default_Demon_Bits() const
{
  if (!data)
    Am_Error("** Get_Default_Demon_Bits called on a (0L) object.");
  return data->default_bits;
}

void
Am_Object_Advanced::Set_Default_Demon_Bits(unsigned short bits)
{
  if (!data)
    Am_Error("** Set_Default_Demon_Bits called on a (0L) object.");
  data->default_bits = bits;
}

unsigned short
Am_Object_Advanced::Get_Demon_Mask() const
{
  if (!data)
    Am_Error("** Get_Demon_Mask called on a (0L) object.");
  return data->bits_mask;
}

void
Am_Object_Advanced::Set_Demon_Mask(unsigned short mask)
{
  if (!data)
    Am_Error("** Set_Demon_Mask called on a (0L) object.");
  data->bits_mask = mask;
}

Am_Inherit_Rule
Am_Object_Advanced::Get_Default_Inherit_Rule() const
{
  if (!data)
    Am_Error("** Get_Default_Inherit_Rule called on a (0L) object.");
  return data->default_rule;
}

void
Am_Object_Advanced::Set_Default_Inherit_Rule(Am_Inherit_Rule rule)
{
  if (!data)
    Am_Error("** Set_Default_Inherit_Rule called on a (0L) object.");
  data->default_rule = rule;
}

void
Am_Object_Advanced::Disinherit_Slot(Am_Slot_Key key)
{
  if (!data)
    Am_Error("** Disinherit_Slot called on a (0L) object.");
  Am_Slot_Data *slot = data->find_slot(key);
  if (slot) {
    if (slot->context == data) {
      if (slot->flags & BIT_INHERITS) {
        slot->flags &= ~BIT_INHERITS;
        if (slot->type != Am_MISSING_SLOT) {
          if (slot->flags & BIT_IS_INHERITED)
            if (data->propagate_change(key, nullptr, *slot, Missing_Slot_Value))
              slot->flags &= ~BIT_IS_INHERITED;
          slot->dependencies.Change(slot, nullptr, *slot, Missing_Slot_Value);
          data->enqueue_change(slot);
          *(Am_Value *)slot = Missing_Slot_Value;
        }
      }
      Am_Constraint *constraint;
      Am_Constraint_Tag tag;
      Am_Constraint_Iterator iter((const Am_Slot &)slot);
      iter.Start();
      while (!iter.Last()) {
        constraint = iter.Get();
        tag = iter.Get_Tag();
        iter.Next();
        if (constraint->Get_Prototype() != nullptr) {
          if (slot->rule == Am_INHERIT)
            data->prop_remove_constraint(key, constraint);
          Am_Slot(slot).Remove_Constraint(tag);
        }
      }
    } else {
      Am_Slot_Data *new_slot = new Am_Slot_Data(data, key);
      *(Am_Value *)new_slot = Missing_Slot_Value;
      data->data.Add((char *)&new_slot);
      if (slot->flags & BIT_IS_INHERITED &&
          !data->propagate_change(key, nullptr, *new_slot, Missing_Slot_Value))
        new_slot->flags |= BIT_IS_INHERITED;
    }
  }
}

void
Am_Object_Advanced::Invoke_Demons(bool active)
{
  if (!data)
    Am_Error("** Invoke_Demons called on a (0L) object.");
  if (active)
    data->demons_active |= DEMONS_ACTIVE;
  else
    data->demons_active &= ~DEMONS_ACTIVE;
}

bool
Am_Object_Advanced::Demon_Invocation_Active() const
{
  if (data)
    return !!(data->demons_active & DEMONS_ACTIVE);
  else
    return true;
}

void
Am_Object_Advanced::Print_Name_And_Data(std::ostream &os) const
{
  Print(os);
  os << " (" << (void *)data << ")";
}

am_CList::am_CList() : head(nullptr) { ; }

CItem *
am_CList::Add_Dep(Am_Constraint *item)
{
  CItem *new_item = new CItem;
  new_item->prev = nullptr;
  new_item->next = head;
  new_item->value = item;
  if (head)
    head->prev = new_item;
  head = new_item;
  return new_item;
}

CItem *
am_CList::Add_Con(Am_Constraint *item)
{
  CItem *new_item = new CItem;
  new_item->next_invalid = nullptr;
  new_item->next = head;
  new_item->value = item;
  head = new_item;
  return new_item;
}

Am_Constraint *
am_CList::Remove_Dep(CItem *item)
{
  CItem *prev = item->prev;
  CItem *next = item->next;
  if (prev)
    prev->next = next;
  else
    head = next;
  if (next)
    next->prev = prev;
  Am_Constraint *value = item->value;
  item->prev = nullptr;
  item->next = nullptr;
  item->value = nullptr;
  delete item;
  return value;
}

Am_Constraint *
am_CList::Remove_Con(CItem *item)
{
  CItem *prev = nullptr;
  CItem *curr = head;
  while (curr) {
    if (item == curr) {
      if (prev)
        prev->next = curr->next;
      else
        head = curr->next;
    }
    prev = curr;
    curr = curr->next;
  }
  Am_Constraint *value = item->value;
  item->prev = nullptr;
  item->next = nullptr;
  item->value = nullptr;
  delete item;
  return value;
}

void
am_CList::Add_Inv(CItem *item)
{
  if (item) {
    Remove_Inv(item);
    item->next_invalid = head;
    head = item;
  }
}

void
am_CList::Add_Update(CItem *item)
{
  if (item) {
    Remove_Inv(item);
    if (head) {
      item->next_invalid = head->next_invalid;
      head->next_invalid = item;
    } else {
      item->next_invalid = head;
      head = item;
    }
  }
}

void
am_CList::Remove_Inv(CItem *item)
{
  CItem *prev = nullptr;
  CItem *curr = head;
  while (curr) {
    if (item == curr) {
      if (prev)
        prev->next_invalid = curr->next_invalid;
      else
        head = curr->next_invalid;
    }
    prev = curr;
    curr = curr->next_invalid;
  }
}

CItem *
am_CList::Find(Am_Constraint *item)
{
  CItem *curr = head;
  while (curr) {
    if (curr->value == item) {
      return curr;
    }
    curr = curr->next;
  }
  return nullptr;
}

CItem *
am_CList::Pop()
{
  CItem *retVal = head;
  if (head) {
    head = head->next_invalid;
  }
  return retVal;
}

void
am_CList::Validate(const Am_Slot &validating_slot)
{
  CItem *curr;
  Am_Value value;
  while (head) {
    curr = head;
    head = head->next_invalid;
    curr->next_invalid = nullptr;
    bool changed = false;
    if (curr->value->Get(validating_slot, value, changed) && changed)
      value.type = Am_MISSING_SLOT;
  }
}

void
am_CList::Invalidate(const Am_Slot &slot_invalidated,
                     Am_Constraint *invalidating_constraint,
                     const Am_Value &value)
{
  for (CItem *current = head; current != nullptr; current = current->next)
    current->value->Invalidated(slot_invalidated, invalidating_constraint,
                                value);
}

void
am_CList::Change(const Am_Slot &slot_invalidated,
                 Am_Constraint *changing_constraint,
                 const Am_Value &prewrapper_value, const Am_Value &new_value)
{
  for (CItem *current = head; current != nullptr; current = current->next)
    current->value->Changed(slot_invalidated, changing_constraint,
                            prewrapper_value, new_value);
}

void
am_CList::Change(const Am_Slot &slot_invalidated,
                 Am_Constraint *changing_constraint)
{
  for (CItem *current = head; current != nullptr; current = current->next)
    current->value->Changed(slot_invalidated, changing_constraint);
}

void
am_CList::Slot_Event(Am_Object_Context *oc, const Am_Slot &slot)
{
  for (CItem *current = head; current != nullptr; current = current->next)
    current->value->Slot_Event(oc, slot);
}

void
am_CList::Remove_Any_Overridden_By(const Am_Slot &slot,
                                   Am_Constraint *competing_constraint)
{
  CItem *next;
  for (CItem *current = head; current != nullptr; current = next) {
    next = current->next;
    if (current->value->Is_Overridden_By(slot, competing_constraint))
      // remove it
      slot.Remove_Constraint(current);
  }
}

void
am_CList::destroy(const Am_Slot &slot, bool constraint)
{
  CItem *current = head;
  CItem *deleted;
  while (current != nullptr) {
    deleted = current;
    current = current->next;
    deleted->next = nullptr;
    if (constraint)
      deleted->value->Constraint_Removed(slot);
    else
      deleted->value->Dependency_Removed(slot);
    deleted->prev = nullptr;
    deleted->value = nullptr;
    delete deleted;
  }
  head = nullptr;
}

#ifdef MEMORY
Dyn_Memory_Manager Am_Slot_Data::memory(sizeof(Am_Slot_Data), "Am_Slot_Data");
#endif

Am_Slot_Data::Am_Slot_Data(Am_Object_Data *object, Am_Slot_Key in_key)
    : context(object), key(in_key), flags(0),
      enabled_demons(object ? object->default_bits : 0),
      rule(object ? object->default_rule : Am_LOCAL), queued_demons(0),
      type_check(0)
{
  type = Am_MISSING_SLOT; // NB: No field named type
  value.voidptr_value = nullptr;
}

Am_Slot_Data::Am_Slot_Data(Am_Object_Data *object, Am_Slot_Key in_key,
                           Am_Value_Type in_type)
    : context(object), key(in_key), flags(0),
      enabled_demons(object ? object->default_bits : 0),
      rule(object ? object->default_rule : Am_LOCAL), queued_demons(0),
      type_check(0)
{
  value.voidptr_value = nullptr;
  type = in_type;
}

void
Am_Slot_Data::Set(const Am_Value &new_value, Am_Slot_Flags set_flags)
{
  Am_Explicit_Set cause(set_flags);

  if ((flags & BIT_READ_ONLY) && !(set_flags & Am_OVERRIDE_READ_ONLY))
    slot_error("** Slot is Read Only.", this);

  // ask any existing constraints whether they want to go away
  if (!constraints.Empty())
    constraints.Remove_Any_Overridden_By(this, cause);

  if (flags & BIT_INHERITS) {
    flags &= ~BIT_INHERITS;
    Am_Object_Context oc(false);
    dependencies.Slot_Event(&oc, this);
  }
  // if new_value is different from the value already in the slot,
  // send the change to its dependencies.  If *this is currently invalid,
  // we must assume that new_value might change it.
  bool different = (*this != new_value) || (flags & BIT_IS_INVALID);
  if (different) {
    flags |= BIT_IS_INVALID;
    if (type_check) {
      const char *error =
          context->demon_set->type_check_list[type_check - 1].func(new_value);
      if (error)
        slot_error(error, this);
    }
#ifndef SEND_ONLY_CHANGES
  }
#endif
  if ((flags & BIT_IS_INHERITED) &&
      context->propagate_change(key, cause, *this, new_value))
    flags &= ~BIT_IS_INHERITED;
  dependencies.Change(this, cause, *this, new_value);
#ifndef SEND_ONLY_CHANGES
  if (different) {
#endif
    context->enqueue_change(this);
  }

  if (Am_Type_Is_Ref_Counted(new_value.type) && new_value.value.wrapper_value) {
    new_value.value.wrapper_value->Note_Reference();
  }
  if (Am_Type_Is_Ref_Counted(type) && value.wrapper_value) {
    value.wrapper_value->Release();
  }
  value = new_value.value;
  type = new_value.type;
  flags &= ~BIT_IS_INVALID;
#ifdef DEBUG
  Global_Call_Slot_Trace(this);
#endif
}

void
Am_Slot_Data::Destroy()
{
#ifdef DEBUG
  Global_Reason_Why_Set = Am_TRACE_SLOT_DESTROY;
  Global_Call_Slot_Trace(this);
#endif
  if (queued_demons && context)
    context->demon_queue.Delete(this);
  context = nullptr;
  if (!(flags & BIT_VALIDATING_NOW)) {
    dependencies.destroy(this, false);
    constraints.destroy(this, true);
    delete this;
  }
}

Am_Object_Advanced
Am_Slot::Get_Owner() const
{
  if (data->context)
    data->context->Note_Reference();
  return data->context;
}

Am_Slot_Key
Am_Slot::Get_Key() const
{
  return data->key;
}

const Am_Value &
Am_Slot::Get() const
{
  return *data;
}

void
Am_Slot::Set(const Am_Value &new_value, Am_Constraint *cause)
{
  bool different = *data != new_value || (data->flags & BIT_IS_INVALID);
  if (different) {
    data->flags |= BIT_IS_INVALID;
    if (data->type_check && (new_value.type != Am_ZERO)) {
      const char *error =
          data->context->demon_set->type_check_list[data->type_check - 1].func(
              new_value);
      if (error)
        slot_error(error, data);
    }
#ifndef SEND_ONLY_CHANGES
  }
#endif
  if (data->flags & BIT_IS_INHERITED) {
    if (data->context->propagate_change(data->key, cause, *data, new_value))
      data->flags &= ~BIT_IS_INHERITED;
  }
  data->dependencies.Change(*this, cause, *data, new_value);
#ifndef SEND_ONLY_CHANGES
  if (different) {
#endif
    data->context->enqueue_change(*this);
    if (Am_Type_Is_Ref_Counted(new_value.type) && new_value.value.wrapper_value)
      new_value.value.wrapper_value->Note_Reference();
    if (Am_Type_Is_Ref_Counted(data->type) && data->value.wrapper_value)
      data->value.wrapper_value->Release();
    data->value = new_value.value;
    data->type = new_value.type;
  }
  data->flags &= ~BIT_IS_INVALID;
}

void
Am_Slot::Set_Current_Constraint(Am_Constraint *constraint)
{
  data->invalid_constraints.Add_Inv(data->constraints.Find(constraint));
}

Am_Value_Type
Am_Slot::Get_Type() const
{
  return data->type;
}

void
Am_Slot::Invalidate(Am_Constraint *validating_constraint)
{
  if (data->flags & BIT_INVALIDATING_NOW) {
    data->invalid_constraints.Add_Update(
        data->constraints.Find(validating_constraint));
  } else {
    data->flags |= BIT_INVALIDATING_NOW;
    data->invalid_constraints.Add_Inv(
        data->constraints.Find(validating_constraint));
    if (!(data->flags & (BIT_IS_INVALID | BIT_VALIDATING_NOW))) {
      data->context->enqueue_invalid(data);
      data->flags |= BIT_IS_INVALID;
      data->dependencies.Invalidate(*this, validating_constraint, *data);
    }
    data->flags &= ~BIT_INVALIDATING_NOW;
  }
}

void
Am_Slot::Event(Am_Object_Context *oc)
{
  if (oc && !oc->is_inherited && (data->flags & BIT_INHERITS)) {
    data->flags &= ~BIT_INHERITS;
    data->dependencies.Slot_Event(oc, *this);
  }
}

void
Am_Slot::Validate()
{
#ifdef DEBUG
  Global_Reason_Why_Set = Am_TRACE_CONSTRAINT_FETCH;
  if (!data)
    return; //sometimes happens when enter inspector after crash
#endif
  if (data->flags & BIT_IS_INVALID) {
    data->flags &= ~BIT_IS_INVALID;
    data->flags |= BIT_VALIDATING_NOW;
    CItem *invalid = data->invalid_constraints.Pop();
    Am_Value new_value;
    bool changed = false;
    while (invalid) {
      if (invalid->Get(*this, new_value, changed)) {
        if (changed) {
          if (data->flags & BIT_IS_INHERITED) {
            if (data->context->propagate_change(data->key, invalid->value))
              data->flags &= ~BIT_IS_INHERITED;
          }
          data->dependencies.Change(*this, invalid->value);
          data->context->enqueue_change(data);
        } else {
          bool different = new_value != *data;

          if (different) {
            if (data->type_check && (new_value.type != Am_ZERO)) {
              const char *error = data->context->demon_set
                                      ->type_check_list[data->type_check - 1]
                                      .func(new_value);
              if (error)
                slot_error(error, data);
            }
#ifndef SEND_ONLY_CHANGES
          }
#endif
          if (data->flags & BIT_IS_INHERITED) {
            if (data->context->propagate_change(data->key, invalid->value,
                                                *data, new_value))
              data->flags &= ~BIT_IS_INHERITED;
          }
          data->dependencies.Change(*this, invalid->value, *data, new_value);

#ifndef SEND_ONLY_CHANGES
          if (different) {
#endif
            if (data->context)
              data->context->enqueue_change(data);
            if (Am_Type_Is_Ref_Counted(new_value.type) &&
                new_value.value.wrapper_value)
              new_value.value.wrapper_value->Note_Reference();
            if (Am_Type_Is_Ref_Counted(data->type) && data->value.wrapper_value)
              data->value.wrapper_value->Release();
            data->value = new_value.value;
            data->type = new_value.type;
          }
        }

        data->invalid_constraints.Remove_Inv(invalid);
#ifdef DEBUG
        Global_Call_Slot_Trace(*this);
#endif
        break;
      }

      invalid = data->invalid_constraints.Pop();
    }

    data->flags &= ~BIT_VALIDATING_NOW;
  }

  if (!(data->flags & BIT_VALIDATING_NOW)) {
    data->flags |= BIT_VALIDATING_NOW;
    data->invalid_constraints.Validate(*this);
    data->flags &= ~BIT_VALIDATING_NOW;
  }
  if (!data->context)
    data->Destroy();
}

void
Am_Slot::Change(Am_Constraint *changing_constraint)
{
#ifdef DEBUG
  Global_Reason_Why_Set = Am_TRACE_SLOT_CHANGED;
#endif
  data->flags |= BIT_IS_INVALID;
  data->dependencies.Change(*this, changing_constraint);
  if (data->flags & BIT_IS_INHERITED) {
    if ((static_cast<Am_Inherit_Rule>(data->rule) != Am_COPY) &&
        data->context->propagate_change(data->key, changing_constraint))
      data->flags &= ~BIT_IS_INHERITED;
  }
  data->context->enqueue_change(data);
  data->flags &= ~BIT_IS_INVALID;
#ifdef DEBUG
  Global_Call_Slot_Trace(*this);
#endif
}

Am_Constraint_Tag
Am_Slot::Add_Constraint(Am_Constraint *new_constraint) const
{
  if (!new_constraint)
    slot_error("** (0L) constraint added to slot.\n", data);

  // ask any existing constraints whether they want to go away
  if (!data->constraints.Empty())
    data->constraints.Remove_Any_Overridden_By(*this, new_constraint);

  new_constraint = new_constraint->Constraint_Added(*this);
  if (!new_constraint)
    // constraint didn't want to be added
    return (Am_Constraint_Tag)nullptr;

  CItem *tag = data->constraints.Add_Con(new_constraint);
  data->invalid_constraints.Add_Inv(tag);
  return (Am_Constraint_Tag)tag;
}

void
Am_Slot::Remove_Constraint(Am_Constraint_Tag constraint_tag) const
{
  CItem *item = (CItem *)constraint_tag;
  Am_Constraint *constraint = item->value;
  data->invalid_constraints.Remove_Inv(item);
  data->constraints.Remove_Con(item);
  constraint->Constraint_Removed(*this);
}

Am_Constraint_Tag
Am_Slot::Add_Dependency(Am_Constraint *new_dependency) const
{
  if (!new_dependency)
    slot_error("** (0L) dependency added to slot.\n", data);
  new_dependency = new_dependency->Dependency_Added(*this);
  if (!new_dependency)
    // didn't want to be added
    return (Am_Constraint_Tag)nullptr;

  CItem *tag = data->dependencies.Add_Dep(new_dependency);
  if (!(data->flags & BIT_INHERITS)) {
    Am_Object_Context oc(false);
    new_dependency->Slot_Event(&oc, *this);
  }
  return (Am_Constraint_Tag)tag;
}

void
Am_Slot::Remove_Dependency(Am_Constraint_Tag dependency_tag) const
{
  CItem *item = (CItem *)dependency_tag;
  Am_Constraint *constraint = item->value;
  data->dependencies.Remove_Dep(item);
  constraint->Dependency_Removed(*this);
  //// TODO: The slot might be destroyable when all deps are gone
}

bool
Am_Slot::Is_Inherited()
{
  return !!(data->flags & BIT_INHERITS);
}

void
Am_Slot::Make_Unique()
{
  if (Am_Type_Is_Ref_Counted(data->type) && (data->value.wrapper_value)) {
    Am_Wrapper *wrapper = data->value.wrapper_value->Make_Unique();
    if (wrapper != data->value.wrapper_value) {
      data->value.wrapper_value = wrapper;
      if ((data->flags & BIT_IS_INHERITED) &&
          data->context->propagate_unique(data->key, wrapper))
        data->flags &= ~BIT_IS_INHERITED;
    }
  }
  data->flags &= ~BIT_INHERITS;
}

unsigned short
Am_Slot::Get_Demon_Bits() const
{
  return data->enabled_demons;
}

void
Am_Slot::Set_Demon_Bits(unsigned short in_bits)
{
  data->enabled_demons = in_bits;
  //// BUG: Must propagate to instances if inherited.
}

Am_Inherit_Rule
Am_Slot::Get_Inherit_Rule() const
{
  return (Am_Inherit_Rule)data->rule;
}

void
Am_Slot::Set_Inherit_Rule(Am_Inherit_Rule in_rule)
{
  switch (data->rule) {
  case Am_LOCAL:
    if ((in_rule == Am_INHERIT) || (in_rule == Am_COPY)) {
      if (in_rule == Am_COPY)
        data->context->convert_temporaries(data->key);
      Am_Value none;
      data->context->propagate_change(data->key, nullptr, none, *data);
    } else if (in_rule == Am_STATIC)
      data->context->remove_temporaries(data->key);
    break;
  case Am_STATIC:
    if (in_rule != Am_STATIC) {
      data->dependencies.Invalidate(*this, nullptr, *data);
      data->dependencies.destroy(*this, false);
    }
    break;
  case Am_COPY:
    if (in_rule != Am_COPY)
      data->context->sever_copies(data);
    break;
  case Am_INHERIT:
    if ((in_rule == Am_LOCAL) || (in_rule == Am_STATIC))
      data->context->remove_temporaries(data->key);
    else if (in_rule == Am_COPY)
      data->context->convert_temporaries(data->key);
    break;
  }
  data->rule = in_rule;
}

unsigned short
Am_Slot::Get_Type_Check() const
{
  return data->type_check;
}

void
Am_Slot::Set_Type_Check(unsigned short type)
{
  data->type_check = type;
  //// BUG: Must propagate to instances if inherited.
}

bool
Am_Slot::Get_Read_Only() const
{
  return !!(data->flags & BIT_READ_ONLY);
}

void
Am_Slot::Set_Read_Only(bool read_only)
{
  if (read_only)
    data->flags |= BIT_READ_ONLY;
  else
    data->flags &= ~BIT_READ_ONLY;
}

void
Am_Slot::Text_Inspect() const
{
  std::cout << "Slot " << std::hex << (void *)data << std::dec << " of object ";
  const char *name = Am_Get_Name_Of_Item(data->context);
  if (!name || (name[0] == '\0'))
    std::cout << data->context;
  else
    std::cout << name;
  std::cout << " is ";
  data->context->print_slot_name_and_value(data->key, data);
}

void
Am_Print_Key(std::ostream &os, Am_Slot_Key key)
{
  os << "<";
  const char *name = Am_Get_Slot_Name(key);
  if (name)
    os << name;
  else
    os << "unnamed";
  os << "[" << (int)key << "]>";
}

//
// Constraint contexts
//

class Am_Empty_Context : public Am_Constraint_Context
{
public:
  Am_ID_Tag ID() { return 0; }

  const Am_Value &Get(const Am_Object_Advanced &object, Am_Slot_Key key,
                      Am_Slot_Flags)
  {
    // *** IF YOU MODIFY THIS METHOD, be sure to modify Get(Am_Slot_Key key), which inlines it

    Am_Slot_Data *slot = object.Get_Data()->find_slot(key);
    if (!slot)
      return Missing_Slot_Value;

    Am_Slot(slot).Validate();
    return *slot;
  }

  void Set(const Am_Object_Advanced &object, Am_Slot_Key k,
           const Am_Value &value, Am_Slot_Flags set_flags)
  {
    object.Get_Data()->set_slot(k, value, set_flags);
  }

  void Note_Changed(const Am_Object_Advanced &object, Am_Slot_Key key)
  {
    Am_Slot slot = object.Get_Data()->find_slot(key);
    slot.Change((nullptr));
  }

  void Note_Unchanged(const Am_Object_Advanced &, Am_Slot_Key) {}

  //NOT in a formula, if error: if Am_RETURN_ZERO_ON_ERROR_BIT returns zero
  //			      if Am_OK_IF_NOT_THERE, returns the error
  //			      else raises Am_Error
  const Am_Value &Raise_Get_Exception(const Am_Value &value,
                                      const Am_Object_Advanced &object,
                                      Am_Slot_Key key, Am_Slot_Flags flags,
                                      const char *msg)
  {
    if (flags & Am_OK_IF_NOT_THERE) {
      if (flags & Am_RETURN_ZERO_ON_ERROR_BIT)
        return Am_Zero_Value;
      else
        return value;
    } else {
      if (!msg)
        switch (value.type) {
        case Am_GET_ON_NULL_OBJECT:
          msg = "Get called on (0L) object";
          break;
        case Am_MISSING_SLOT:
          msg = "Slot does not exist";
          break;
        case Am_FORMULA_INVALID:
          msg = "Formula in slot is uninitialized";
          break;
        default:
          msg = "unknown slot error";
          break;
        }
      slot_error(msg, object, key);
      return Am_No_Value; // never gets here
    }
  }

  Am_Wrapper *Get_Data() { return nullptr; }

  void Set_Data(Am_Wrapper *) {}
};

Am_Constraint_Context *Am_Empty_Constraint_Context = new Am_Empty_Context();

Am_Constraint_Context *Am_Object::cc = Am_Empty_Constraint_Context;

//if you put a slot_key as the value of a value, with the type of the
//value being Am_SLOT_KEY_TYPE.
class Am_Slot_Key_Type_Support_Class : public Am_Type_Support
{
public:
  void Print(std::ostream &os, const Am_Value &value) const
  {
    Am_Slot_Key key = (Am_Slot_Key)value.value.long_value;
    const char *name = Am_Get_Slot_Name(key);
    if (name)
      os << name;
    else
      os << "[" << (int)key << "]";
  }
  const char *To_String(const Am_Value &value) const
  {
    return Am_Get_Slot_Name((Am_Slot_Key)value.value.long_value);
  }
  Am_Value From_String(const char *string) const
  {
    Am_Slot_Key key = Am_From_Slot_Name(string);
    if (key) {
      Am_Value v(key, Am_SLOT_KEY_TYPE);
      return v;
    } else
      return Am_No_Value;
  }
};

Am_Type_Support *slot_key_type_support = new Am_Slot_Key_Type_Support_Class();

std::ostream &
operator<<(std::ostream &os, const Am_Am_Slot_Key &item)
{
  item.Print(os);
  return os;
}

void
Am_Am_Slot_Key::Print(std::ostream &out) const
{
  out << To_String();
  return;
}
void
Am_Am_Slot_Key::Println() const
{
  Print(std::cout);
  std::cout << std::endl;
  return;
}

static void
init()
{
#ifdef DEBUG
  Am_Register_Type_Name(Am_SLOT_KEY_TYPE, "Am_Slot_Key");
#endif
  Am_Register_Support(Am_SLOT_KEY_TYPE, slot_key_type_support);
}

static Am_Initializer *initializer =
    new Am_Initializer(DSTR("Ore"), init, 1.0); //this should be first

/////////////////////////////////////////////////////////////////////
//  Am_Explicit_Set:  imitates a constraint in order to
//   carry flags describing the kind of Set desired.
/////////////////////////////////////////////////////////////////////

Am_ID_Tag Am_Explicit_Set::id =
    Am_Get_Unique_ID_Tag(DSTR("Explicit_Set"), Am_CONSTRAINT);

bool
Am_Explicit_Set::Get(const Am_Slot & /* fetching_slot */,
                     Am_Value & /* value */, bool & /* changed */)
{
  return false;
}

void
Am_Explicit_Set::Invalidated(const Am_Slot & /* slot_invalidated */,
                             Am_Constraint * /* invalidating_constraint */,
                             const Am_Value & /* value */)
{
}

void
Am_Explicit_Set::Changed(const Am_Slot & /*slot_changed*/,
                         Am_Constraint * /*changing_constraint*/,
                         const Am_Value & /*prev_value*/,
                         const Am_Value & /*new_value*/)
{
}

void
Am_Explicit_Set::Changed(const Am_Slot & /*slot_changed*/,
                         Am_Constraint * /*changing_constraint*/)
{
}

void
Am_Explicit_Set::Slot_Event(Am_Object_Context * /*oc*/,
                            const Am_Slot & /*slot*/)
{
}

Am_Constraint *
Am_Explicit_Set::Get_Prototype()
{
  return nullptr;
}

bool
Am_Explicit_Set::Is_Instance_Of(Am_Constraint * /* proto */)
{
  return false;
}

Am_Constraint *
Am_Explicit_Set::Constraint_Added(const Am_Slot & /* adding_slot */)
{
  return nullptr;
}

Am_Constraint *
Am_Explicit_Set::Dependency_Added(const Am_Slot & /* adding_slot */)
{
  return nullptr;
}

void
Am_Explicit_Set::Constraint_Removed(const Am_Slot & /* removing_slot */)
{
}

void
Am_Explicit_Set::Dependency_Removed(const Am_Slot & /* removing_slot */)
{
}

bool
Am_Explicit_Set::Is_Overridden_By(const Am_Slot & /* slot */,
                                  Am_Constraint * /* competing_constraint */)
{
  return false;
}

Am_Constraint *
Am_Explicit_Set::Create(const Am_Slot & /* current_slot */,
                        const Am_Slot & /* new_slot */)
{
  return nullptr;
}

Am_Constraint *
Am_Explicit_Set::Copy(const Am_Slot & /* current_slot */,
                      const Am_Slot & /* new_slot */)
{
  return nullptr;
}

Am_ID_Tag
Am_Explicit_Set::ID() const
{
  return id;
}

const char *
Am_Explicit_Set::Get_Name()
{
  return "**EXPLICIT SET**";
}

bool
Am_Explicit_Set::Test(Am_Constraint *formula)
{
  return (formula->ID() == id);
}

Am_Explicit_Set *
Am_Explicit_Set::Narrow(Am_Constraint *formula)
{
  if (id == formula->ID())
    return (Am_Explicit_Set *)formula;
  else {
    Am_Error(
        "Constraint narrowed to Explicit_Set type is not an Explicit_Set.");
    return nullptr;
  }
}

void (*Am_Object_Debugger)(const Am_Object &obj, Am_Slot_Key slot) = nullptr;

void
Am_Error(const Am_Object &obj, Am_Slot_Key slot)
{
  if (Am_Object_Debugger)
    Am_Object_Debugger(obj, slot);
  Am_Error();
}

void
Am_Error(const char *error_string, const Am_Object &obj, Am_Slot_Key slot)
{
  std::cerr << "** Amulet Error: " << error_string << std::endl;
  Am_Error(obj, slot);
}
