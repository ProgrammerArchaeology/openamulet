/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

extern int always_propagate;

#include <stdlib.h>

#include <am_inc.h>

#include FORMULA_ADVANCED__H
#include STANDARD_SLOTS__H

#include FORMULA__H
#include STDVALUE__H
#include REGISTRY__H

#ifdef MEMORY
#include DYNARRAY__H
#endif

typedef union
{
  Am_FProc_Wrapper *f_wrapper;
  Am_FProc_Void *f_void;
  Am_FProc_Int *f_int;
  Am_FProc_Long *f_long;
  Am_FProc_Bool *f_bool;
  Am_FProc_Float *f_float;
  Am_FProc_Double *f_double;
  Am_FProc_Char *f_char;
  Am_FProc_String *f_string;
  Am_FProc_Const_String *f_const_string;
  Am_FProc_Proc *f_proc;
  Am_FProc_Method *f_method;
  Am_FProc_Value *f_value;
  Am_FProc_Const_Value *f_const_value;
} F_Type;

typedef enum {
  F_WRAPPER,
  F_VOID,
  F_INT,
  F_LONG,
  F_BOOL,
  F_FLOAT,
  F_DOUBLE,
  F_CHAR,
  F_STRING,
  F_CONST_STRING,
  F_PROC,
  F_METHOD,
  F_VALUE,
  F_CONST_VALUE
} Formula_Type;

class Formula_Constraint;

class Dependency
{
#ifdef MEMORY
public:
  void *operator new(size_t) { return memory.New(); }
  void operator delete(void *ptr, size_t) { memory.Delete(ptr); }
  static Dyn_Memory_Manager memory;
#endif
public:
  Am_Slot depended;
  Am_Constraint_Tag dependency_tag;
  Dependency *next;

  void Move_Back_Dep(Am_Slot &dest_slot, Am_Constraint *context);
  void Remove();
};

#ifdef MEMORY
Dyn_Memory_Manager Dependency::memory(sizeof(Dependency), "Dependency");
#endif

void
Dependency::Move_Back_Dep(Am_Slot &dest_slot, Am_Constraint *context)
{
  Remove();
  depended = dest_slot;
  dependency_tag = dest_slot.Add_Dependency(context);
}

void
Dependency::Remove()
{
  if (depended)
    depended.Remove_Dependency(dependency_tag);
}

class Formula_Context : public Am_Constraint_Context
{
public:
  Am_ID_Tag ID();
  const Am_Value &Get(const Am_Object_Advanced &object, Am_Slot_Key key,
                      Am_Slot_Flags flags);
  void Set(const Am_Object_Advanced &object, Am_Slot_Key key,
           const Am_Value &new_value, Am_Slot_Flags flags);
  void Note_Changed(const Am_Object_Advanced &object, Am_Slot_Key key);
  void Note_Unchanged(const Am_Object_Advanced &object, Am_Slot_Key key);
  const Am_Value &Raise_Get_Exception(const Am_Value &value,
                                      const Am_Object_Advanced &object,
                                      Am_Slot_Key key, Am_Slot_Flags flags,
                                      const char *msg);
  Am_Wrapper *Get_Data();
  void Set_Data(Am_Wrapper *data);

  Am_Slot depender;
  Dependency *last_position;
  Dependency *current_position;
  Formula_Constraint *context;
  bool changed, inited;
  Am_Value exception;
  bool return_value_from_get;

  Formula_Context(Formula_Constraint *context, const Am_Slot &in_depender,
                  Dependency *depends_on);
  void Clean_Up(Dependency **depends_on);
};

class Am_Formula_Data
{
public:
  Am_Formula_Data(Formula_Type in_type, const char *in_name)
      : refs(1), type(in_type),
#ifdef DEBUG
        name(in_name),
#endif
        stored_data(0L)
  {
    ;
  }
  ~Am_Formula_Data()
  {
    if (stored_data)
      stored_data->Release();
  }
  void Release()
  {
    if (!--refs)
      delete this;
  }
  void Add_Ref() { ++refs; }

  F_Type form_data;
  Formula_Type type;
#ifdef DEBUG
  const char *name;
#endif
  Am_Wrapper *stored_data;
  int refs;
};

class Formula_Constraint : public Am_Formula_Advanced
{
#ifdef MEMORY
public:
  void *operator new(size_t) { return memory.New(); }
  void operator delete(void *ptr, size_t) { memory.Delete(ptr); }
  static Dyn_Memory_Manager memory;
#endif
public:
  Formula_Constraint(Am_Formula_Data *in_data);
  Formula_Constraint(Formula_Constraint *proto_formula);

  ~Formula_Constraint()
  {
    if (data)
      data->Release();
    data = (0L);
  }

  bool Get(const Am_Slot &fetching_slot, Am_Value &value, bool &changed);

  void Invalidated(const Am_Slot &slot_invalidated,
                   Am_Constraint *invalidating_constraint,
                   const Am_Value &value);
  void Changed(const Am_Slot &slot_changed, Am_Constraint *changing_constraint,
               const Am_Value &prev_value, const Am_Value &new_value);
  void Changed(const Am_Slot &slot_changed, Am_Constraint *changing_constraint);
  void Slot_Event(Am_Object_Context *oc, const Am_Slot &slot);
  Am_Constraint *Get_Prototype();
  bool Is_Instance_Of(Am_Constraint *proto);

  Am_Constraint *Constraint_Added(const Am_Slot &adding_slot);
  Am_Constraint *Dependency_Added(const Am_Slot &adding_slot);
  void Constraint_Removed(const Am_Slot &removing_slot);
  void Dependency_Removed(const Am_Slot &removing_slot);

  bool Is_Overridden_By(const Am_Slot &slot,
                        Am_Constraint *competing_constraint);

  Am_Constraint *Create(const Am_Slot &current_slot, const Am_Slot &new_slot);
  Am_Constraint *Copy(const Am_Slot &current_slot, const Am_Slot &new_slot);
  Am_ID_Tag ID() const;
  const char *Get_Name();

  Am_Formula_Mode Get_Mode();
  void Set_Mode(Am_Formula_Mode mode);
  Am_Slot Get_Context();

  static Am_ID_Tag id;

  Formula_Type type;
  bool internal_remove;
  unsigned short flags;
  Am_Slot context;
  F_Type formula;
  Dependency *depends_on;
  Am_Wrapper *data;
  Formula_Constraint *prototype;
  Formula_Constraint *first_instance;
  Formula_Constraint *next_instance;
#ifdef DEBUG
  const char *formula_name;
#endif
};

#ifdef MEMORY
Dyn_Memory_Manager Formula_Constraint::memory(sizeof(Formula_Constraint),
                                              "Formula Constraint");
#endif

Am_Depends_Iterator::Am_Depends_Iterator() : context(0L), current(0L) { ; }

Am_Depends_Iterator::Am_Depends_Iterator(const Am_Formula_Advanced *formula)
    : context(formula), current(0L)
{
  ;
}

Am_Depends_Iterator &
Am_Depends_Iterator::operator=(const Am_Formula_Advanced *formula)
{
  context = formula;
  current = (0L);
  return *this;
}

unsigned short
Am_Depends_Iterator::Length() const
{
  if (context) {
    //      Dependency* curr;
    unsigned short count = 0;
    for (Dependency *curr = ((Formula_Constraint *)context)->depends_on;
         curr != (0L); curr = curr->next)
      ++count;
    return count;
  } else
    return 0;
}

void
Am_Depends_Iterator::Start()
{
  if (context)
    current = (Am_Dependency_Data *)((Formula_Constraint *)context)->depends_on;
}

void
Am_Depends_Iterator::Next()
{
  if (current)
    current = (Am_Dependency_Data *)((Dependency *)current)->next;
  else if (context)
    current = (Am_Dependency_Data *)((Formula_Constraint *)context)->depends_on;
  while (current && !(((Dependency *)current)->depended))
    current = (Am_Dependency_Data *)((Dependency *)current)->next;
}

bool
Am_Depends_Iterator::Last() const
{
  return !current;
}

Am_Slot
Am_Depends_Iterator::Get() const
{
  if (current)
    return ((Dependency *)current)->depended;
  else
    return (0L);
}

Formula_Context::Formula_Context(Formula_Constraint *in_context,
                                 const Am_Slot &in_depender,
                                 Dependency *depends_on)
    : depender(in_depender), inited(true), context(in_context),
      last_position(0L), current_position(depends_on), changed(false),
      return_value_from_get(true)
{
  ;
}

void
Formula_Context::Clean_Up(Dependency **depends_on)
{
  if (current_position != (0L)) {
    if (last_position)
      last_position->next = (0L);
    else
      *depends_on = (0L);
    Dependency *current = current_position;
    Dependency *free_node;
    context->internal_remove = true;
    while (current) {
      free_node = current;
      current = current->next;
      free_node->Remove();
      free_node->depended = Am_Slot((0L));
      free_node->next = (0L);
      delete free_node;
    }
    context->internal_remove = false;
  }
}

Am_ID_Tag Formula_Constraint::id =
    Am_Get_Unique_ID_Tag(DSTR("Formula_Constraint"), Am_CONSTRAINT);

const char *
Formula_Constraint::Get_Name()
{
#ifdef DEBUG
  Formula_Constraint *me = (Formula_Constraint *)this;
  return me->formula_name;
#else
  return (0L); //no name if not debugging
#endif
}

// Primary constructor for Formula_Constraint, makes one from a
// procedure and a type, and an optional name
Formula_Constraint::Formula_Constraint(Am_Formula_Data *in_data)
    : context(0L), formula(in_data->form_data), type(in_data->type),
#ifdef DEBUG
      formula_name(in_data->name),
#endif
      flags(0), data(in_data->stored_data), depends_on(0L), prototype(0L),
      first_instance(0L), next_instance(0L), internal_remove(false)
{
  if (in_data->stored_data)
    in_data->stored_data->Note_Reference();
}

Formula_Constraint::Formula_Constraint(Formula_Constraint *proto_formula)
    : context(0L), formula(proto_formula->formula), type(proto_formula->type),
#ifdef DEBUG
      formula_name(proto_formula->formula_name),
#endif
      flags(proto_formula->flags), data(proto_formula->data), depends_on(0L),
      prototype(0L), first_instance(0L), next_instance(0L),
      internal_remove(false)
{
  if (proto_formula->data)
    proto_formula->data->Note_Reference();
}

bool
Formula_Constraint::Get(const Am_Slot &fetching_slot, Am_Value &value,
                        bool &changed)
{
  Formula_Context fc(this, fetching_slot, depends_on);

  Am_Object owner = context.Get_Owner();

  Am_PUSH_CC(&fc) switch (type)
  {
  case F_WRAPPER:
    value = formula.f_wrapper(owner);
    break;
  case F_VOID:
    value = formula.f_void(owner);
    break;
  case F_INT:
    value = formula.f_int(owner);
    break;
  case F_LONG:
    value = formula.f_long(owner);
    break;
  case F_BOOL:
    value = formula.f_bool(owner);
    break;
  case F_FLOAT:
    value = formula.f_float(owner);
    break;
  case F_DOUBLE: {
    double hold = formula.f_double(owner);
    if (!fc.changed)
      value = hold;
    break;
  }
  case F_CHAR:
    value = formula.f_char(owner);
    break;
  case F_STRING: {
    const char *hold = formula.f_string(owner);
    if (!fc.changed)
      value = hold;
    break;
  }
  case F_CONST_STRING: {
    const char *hold = formula.f_const_string(owner);
    if (!fc.changed)
      value = hold;
    break;
  }
  case F_PROC:
    value = formula.f_proc(owner);
    break;
  case F_METHOD:
    value = formula.f_method(owner);
    break;
  case F_VALUE:
    value = formula.f_value(owner);
    if (Am_Type_Class(value.type) == Am_ERROR_VALUE_TYPE)
      value.Set_Empty();
    break;
  case F_CONST_VALUE:
    value = formula.f_const_value(owner);
    if (Am_Type_Class(value.type) == Am_ERROR_VALUE_TYPE)
      value.Set_Empty();
    break;
  }
  Am_POP_CC()

      fc.Clean_Up(&depends_on);
  changed = fc.changed;
  if (!fc.inited) {
    value = fc.exception;
    value.Set_Value_Type(Am_FORMULA_INVALID);
  }
  return fc.return_value_from_get;
}

Am_Constraint *
Formula_Constraint::Get_Prototype()
{
  return prototype;
}

bool
Formula_Constraint::Is_Instance_Of(Am_Constraint *proto)
{
  Formula_Constraint *current = this;
  while (current) {
    if (current == proto)
      return true;
    current = current->prototype;
  }
  return false;
}

void
Formula_Constraint::Invalidated(const Am_Slot &, Am_Constraint *,
                                const Am_Value &)
{
  if (!(flags & Am_FORMULA_DO_NOT_PROPAGATE) && context)
    context.Invalidate(this);
}

void
Formula_Constraint::Changed(const Am_Slot &, Am_Constraint * /*cause*/,
                            const Am_Value &old_value,
                            const Am_Value &new_value)
{
  if (!(flags & Am_FORMULA_DO_NOT_PROPAGATE) && context /*&& !cause*/
#ifndef SEND_ONLY_CHANGES
      && old_value != new_value
#endif
      )
    context.Invalidate(this);
}

void
Formula_Constraint::Changed(const Am_Slot &, Am_Constraint * /*cause*/)
{
  if (!(flags & Am_FORMULA_DO_NOT_PROPAGATE) && context /*&& !cause*/)
    context.Invalidate(this);
}

void
Formula_Constraint::Slot_Event(Am_Object_Context *oc, const Am_Slot &)
{
  context.Event(oc);
}

Am_Constraint *
Formula_Constraint::Constraint_Added(const Am_Slot &adding_slot)
{
  context = adding_slot;
  return this;
}

Am_Constraint *
Formula_Constraint::Dependency_Added(const Am_Slot &)
{
  return this;
}

void
Formula_Constraint::Constraint_Removed(const Am_Slot &)
{
  Dependency *current;
  Dependency *free_node;

  context = Am_Slot((0L));
  current = depends_on;
  internal_remove = true;
  while (current != (0L)) {
    free_node = current;
    current = current->next;
    free_node->next = (0L);
    free_node->Remove();
    delete free_node;
  }
  depends_on = (0L);
  internal_remove = false;
  if (prototype) {
    Formula_Constraint *prev_con = (0L);
    Formula_Constraint *curr_con = prototype->first_instance;
    while (curr_con) {
      if (curr_con == this) {
        if (prev_con)
          prev_con->next_instance = curr_con->next_instance;
        else
          prototype->first_instance = curr_con->next_instance;
        curr_con->next_instance = (0L);
        break;
      }
      prev_con = curr_con;
      curr_con = curr_con->next_instance;
    }
  }
  prototype = (0L);
  Formula_Constraint *curr_con = first_instance;
  Formula_Constraint *next_con;
  while (curr_con) {
    next_con = curr_con->next_instance;
    curr_con->prototype = (0L);
    curr_con->next_instance = (0L);
    curr_con = next_con;
  }
  first_instance = (0L);
  delete this;
}

void
Formula_Constraint::Dependency_Removed(const Am_Slot &removing_slot)
{
  if (!internal_remove) {
    Dependency *current;
    for (current = depends_on; current != (0L); current = current->next) {
      if (current->depended == removing_slot) {
        current->depended = Am_Slot((0L));
        break;
      }
    }
  }
}

bool
Formula_Constraint::Is_Overridden_By(const Am_Slot & /* slot */,
                                     Am_Constraint *competitor)
{
  if (competitor == (0L) ||
      (Am_Explicit_Set::Test(competitor) &&
       !(Am_Explicit_Set::Narrow(competitor)->flags & Am_KEEP_FORMULAS))) {
    // Set with explicit value overrides me if I lack MULTI_CONSTRAINT
    if (!(flags & Am_FORMULA_MULTI_CONSTRAINT))
      return true;
  } else if (competitor->ID() == id) {
    // another formula constraint overrides me if I lack MULTI_CONSTRAINT
    if (!(flags & Am_FORMULA_MULTI_CONSTRAINT))
      return true;

    Formula_Constraint *new_formula = (Formula_Constraint *)competitor;

    // another formula constraint overrides me if we're at the same
    // instance level (i.e., have same prototype) and I lack
    // MULTI_LOCAL_CONSTRAINT

    if (new_formula->prototype == prototype &&
        !(flags & Am_FORMULA_MULTI_LOCAL_CONSTRAINT)) {

      // change all my instances and prototype to point to new_formula

      Formula_Constraint *curr_con = first_instance;
      Formula_Constraint *next_con;
      while (curr_con) {
        next_con = curr_con->next_instance;
        curr_con->prototype = new_formula;
        curr_con->next_instance = new_formula->first_instance;
        new_formula->first_instance = curr_con;
        curr_con = next_con;
      }
      if (prototype) {
        Formula_Constraint *prev_con = (0L);
        curr_con = prototype->first_instance;
        while (curr_con) {
          if (curr_con == this) {
            if (prev_con)
              prev_con->next_instance = curr_con->next_instance;
            else
              prototype->first_instance = curr_con->next_instance;
            break;
          }
          prev_con = curr_con;
          curr_con = curr_con->next_instance;
        }
      }
      prototype = (0L);
      first_instance = (0L);
      next_instance = (0L);
      return true;
    }
  }

  return false;
}

Am_Constraint *
Formula_Constraint::Create(const Am_Slot &, const Am_Slot &)
{
  Formula_Constraint *new_formula = new Formula_Constraint(this);
  new_formula->next_instance = first_instance;
  first_instance = new_formula;
  new_formula->prototype = this;
  return new_formula;
}

Am_Constraint *
Formula_Constraint::Copy(const Am_Slot &, const Am_Slot &)
{
  Formula_Constraint *new_formula = new Formula_Constraint(this);
  if (prototype) {
    new_formula->next_instance = prototype->first_instance;
    prototype->first_instance = new_formula;
    new_formula->prototype = prototype;
  }
  return new_formula;
}

Am_ID_Tag
Formula_Constraint::ID() const
{
  return id;
}

Am_Formula_Mode
Formula_Constraint::Get_Mode()
{
  return flags;
}

void
Formula_Constraint::Set_Mode(Am_Formula_Mode mode)
{
  flags = mode;
}

Am_Slot
Formula_Constraint::Get_Context()
{
  return context;
}

Am_Formula_Advanced *
Am_Formula_Advanced::Narrow(Am_Constraint *value)
{
  if (Formula_Constraint::id == value->ID())
    return (Am_Formula_Advanced *)value;
  else {
    Am_Error("Constraint narrowed to Formula type is not a Formula.");
    return (0L);
  }
}

bool
Am_Formula_Advanced::Test(Am_Constraint *value)
{
  if (Formula_Constraint::id == value->ID())
    return true;
  else
    return false;
}

Am_Formula::Am_Formula(const Am_Formula &in_formula) : data(in_formula.data)
{
  data->Add_Ref();
}

Am_Formula::Am_Formula(Am_FProc_Wrapper *formula, const char *name)
{
  data = new Am_Formula_Data(F_WRAPPER, name);
  data->form_data.f_wrapper = formula;
}

Am_Formula::Am_Formula(Am_FProc_Void *formula, const char *name)
{
  data = new Am_Formula_Data(F_VOID, name);
  data->form_data.f_void = formula;
}

Am_Formula::Am_Formula(Am_FProc_Int *formula, const char *name)
{
  data = new Am_Formula_Data(F_INT, name);
  data->form_data.f_int = formula;
}

Am_Formula::Am_Formula(Am_FProc_Long *formula, const char *name)
{
  data = new Am_Formula_Data(F_LONG, name);
  data->form_data.f_long = formula;
}

Am_Formula::Am_Formula(Am_FProc_Bool *formula, const char *name)
{
  data = new Am_Formula_Data(F_BOOL, name);
  data->form_data.f_bool = formula;
}

Am_Formula::Am_Formula(Am_FProc_Float *formula, const char *name)
{
  data = new Am_Formula_Data(F_FLOAT, name);
  data->form_data.f_float = formula;
}

Am_Formula::Am_Formula(Am_FProc_Double *formula, const char *name)
{
  data = new Am_Formula_Data(F_DOUBLE, name);
  data->form_data.f_double = formula;
}

Am_Formula::Am_Formula(Am_FProc_Char *formula, const char *name)
{
  data = new Am_Formula_Data(F_CHAR, name);
  data->form_data.f_char = formula;
}

Am_Formula::Am_Formula(Am_FProc_String *formula, const char *name)
{
  data = new Am_Formula_Data(F_STRING, name);
  data->form_data.f_string = formula;
}

Am_Formula::Am_Formula(Am_FProc_Const_String *formula, const char *name)
{
  data = new Am_Formula_Data(F_CONST_STRING, name);
  data->form_data.f_const_string = formula;
}

Am_Formula::Am_Formula(Am_FProc_Proc *formula, const char *name)
{
  data = new Am_Formula_Data(F_PROC, name);
  data->form_data.f_proc = formula;
}

Am_Formula::Am_Formula(Am_FProc_Method *formula, const char *name)
{
  data = new Am_Formula_Data(F_METHOD, name);
  data->form_data.f_method = formula;
}

Am_Formula::Am_Formula(Am_FProc_Value *formula, const char *name)
{
  data = new Am_Formula_Data(F_VALUE, name);
  data->form_data.f_value = formula;
}

Am_Formula::Am_Formula(Am_FProc_Const_Value *formula, const char *name)
{
  data = new Am_Formula_Data(F_CONST_VALUE, name);
  data->form_data.f_const_value = formula;
}

Am_Formula::~Am_Formula() { data->Release(); }

Am_Formula &
Am_Formula::operator=(const Am_Formula &in_formula)
{
  data = in_formula.data;
  data->Add_Ref();
  return *this;
}

const Am_Value
Am_Formula::operator()(Am_Object &context) const
{
  Am_Value temp;
  switch (data->type) {
  case F_WRAPPER:
    temp = data->form_data.f_wrapper(context);
    break;
  case F_VOID:
    temp = data->form_data.f_void(context);
    break;
  case F_INT:
    temp = data->form_data.f_int(context);
    break;
  case F_LONG:
    temp = data->form_data.f_long(context);
    break;
  case F_BOOL:
    temp = data->form_data.f_bool(context);
    break;
  case F_FLOAT:
    temp = data->form_data.f_float(context);
    break;
  case F_DOUBLE:
    temp = data->form_data.f_double(context);
    break;
  case F_CHAR:
    temp = data->form_data.f_char(context);
    break;
  case F_STRING:
    temp = data->form_data.f_string(context);
    break;
  case F_PROC:
    temp = data->form_data.f_proc(context);
    break;
  case F_METHOD:
    temp = data->form_data.f_method(context);
    break;
  case F_VALUE:
    temp = data->form_data.f_value(context);
    break;
  case F_CONST_VALUE:
    temp = data->form_data.f_const_value(context);
    break;
  case F_CONST_STRING:
    temp = data->form_data.f_const_string(context);
    break;
  }
  return temp;
}

Am_Formula::operator Am_Constraint *() { return new Formula_Constraint(data); }

Am_Constraint *
Am_Formula::Multi_Constraint(bool multi_local)
{
  Formula_Constraint *c = new Formula_Constraint(data);
  c->Set_Mode(c->Get_Mode() | Am_FORMULA_MULTI_CONSTRAINT |
              (multi_local ? Am_FORMULA_MULTI_LOCAL_CONSTRAINT : 0));
  return c;
}

void
Am_Formula::Set_Data(Am_Wrapper *stored_data)
{
  data->stored_data = stored_data;
}

Am_ID_Tag
Am_Formula::ID() const
{
  return Formula_Constraint::id;
}

Am_ID_Tag
Formula_Context::ID()
{
  return Formula_Constraint::id;
}

const Am_Value &
Formula_Context::Get(const Am_Object_Advanced &object, Am_Slot_Key key,
                     Am_Slot_Flags flags)
{
  if (flags & Am_NO_DEPENDENCY)
    return Am_Empty_Constraint_Context->Get(object, key, flags);

  Am_Slot slot;

  if (current_position) {
    slot = current_position->depended;
    if (slot && (slot.Get_Owner() == object) && (slot.Get_Key() == key)) {
      last_position = current_position;
      current_position = current_position->next;
      slot.Validate();
      return slot.Get();
    }
  }
  slot = object.Get_Slot(key);
  if (current_position) {
    context->internal_remove = true;
    current_position->Move_Back_Dep(slot, context);
    context->internal_remove = false;
    last_position = current_position;
    current_position = current_position->next;
  } else {
    Dependency *new_dep = new Dependency();

    new_dep->depended = slot;
    new_dep->dependency_tag = slot.Add_Dependency(context);
    new_dep->next = (0L);
    if (last_position)
      last_position->next = new_dep;
    else
      context->depends_on = new_dep;
    last_position = new_dep;
  }

  slot.Validate();
  return slot.Get();
}

void
Formula_Context::Set(const Am_Object_Advanced &object, Am_Slot_Key key,
                     const Am_Value &value, Am_Slot_Flags flags)
{
  if (!(flags & Am_NO_DEPENDENCY) && (object == depender.Get_Owner()) &&
      (key == depender.Get_Key())) {
    changed = true;
    return_value_from_get = false;
    depender.Set(value, context);
  } else {
    Am_Empty_Constraint_Context->Set(object, key, value, flags);
  }
}

void
Formula_Context::Note_Changed(const Am_Object_Advanced &object, Am_Slot_Key key)
{
  if ((object == depender.Get_Owner()) && (key == depender.Get_Key()))
    changed = true;
  else
    Am_Empty_Constraint_Context->Note_Changed(object, key);
}

void
Formula_Context::Note_Unchanged(const Am_Object_Advanced &object,
                                Am_Slot_Key key)
{
  if ((object == depender.Get_Owner()) && (key == depender.Get_Key()))
    changed = false;
  else
    Am_Empty_Constraint_Context->Note_Unchanged(object, key);
}

//in a formula, if error:     if Am_RETURN_ZERO_ON_ERROR_BIT returns zero
//			      if Am_OK_IF_NOT_THERE, returns the error
//			      else notes uninitialized, and returns zero
const Am_Value &
Formula_Context::Raise_Get_Exception(const Am_Value &value,
                                     const Am_Object_Advanced &, Am_Slot_Key,
                                     Am_Slot_Flags flags, const char *)
{
  if (flags & Am_RETURN_ZERO_ON_ERROR_BIT)
    return Am_Zero_Value;
  else if (flags & Am_OK_IF_NOT_THERE)
    return value;
  else {
    inited = false;
    exception = value;
    return Am_Zero_Value;
  }
}

Am_Wrapper *
Formula_Context::Get_Data()
{
  if (context->data)
    context->data->Note_Reference();
  return context->data;
}

void
Formula_Context::Set_Data(Am_Wrapper *data)
{
  if (context->data)
    context->data->Release();
  context->data = data;
}
