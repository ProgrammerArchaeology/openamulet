/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <am_inc.h>

#include WEB__H
#include STANDARD_SLOTS__H

#ifdef MEMORY
	#include DYNARRAY__H
	#include <stddef.h>
#endif

/*****************************************************************************
 *  Definition of Classes                                                    *
 *****************************************************************************/

class Web_Constraint : public Am_Constraint {
 public:
  bool Get (const Am_Slot& fetching_slot, Am_Value& value, bool& changed);
  void Invalidated (const Am_Slot& slot_invalidated,
		    Am_Constraint* invalidating_constraint,
		    const Am_Value& value);
  void Changed (const Am_Slot& slot_changed,
		Am_Constraint* invalidating_constraint,
		const Am_Value& prev_value, const Am_Value& new_value);
  void Changed (const Am_Slot& slot_changed,
		Am_Constraint* invalidating_constraint);
  void Slot_Event (Am_Object_Context* oc, const Am_Slot& slot);
  Am_Constraint* Get_Prototype ();
  bool Is_Instance_Of (Am_Constraint* proto);
  Am_Constraint* Constraint_Added (const Am_Slot& adding_slot);
  Am_Constraint* Dependency_Added (const Am_Slot& adding_slot);
  void Constraint_Removed (const Am_Slot& removing_slot);
  void Dependency_Removed (const Am_Slot& removing_slot);

  bool Is_Overridden_By (const Am_Slot& slot,
			 Am_Constraint *competing_constraint);

  Am_Constraint* Create (const Am_Slot& current_slot,
			 const Am_Slot& new_slot);
  Am_Constraint* Copy (const Am_Slot& current_slot,
		       const Am_Slot& new_slot);
  Am_ID_Tag ID () const;
  const char *Get_Name() {return "*WEB*";}  ////// Temporary, return real name

  Web_Constraint (Am_Web_Data* in_owner)
  { owner = in_owner; }

  Am_Web_Data* owner;
};

class Input_Port;
class Output_Port;

class Am_Web_Data {
 public:
  Am_Web_Data (Am_Web_Create_Proc* create,
               Am_Web_Initialize_Proc* initialize,
               Am_Web_Validate_Proc* validate)
    : interface (this)
  {
    create_proc = create;
    initialize_proc = initialize;
    validate_proc = validate;
    prototype = (0L);
    child = (0L);
    sibling = (0L);
    out_list = (0L);
    in_list = (0L);
    invalid_list = (0L);
    validating = false;
    initialized = false;
    valid = false;
    changed_first = (0L);
    changed_last = (0L);
  }

  Am_Web_Data (Am_Web_Data* in_prototype)
    : interface (this)
  {
    create_proc = in_prototype->create_proc;
    initialize_proc = in_prototype->initialize_proc;
    validate_proc = in_prototype->validate_proc;
    prototype = in_prototype;
    child = (0L);
    sibling = in_prototype->child;
    in_prototype->child = this;
    out_list = (0L);
    in_list = (0L);
    invalid_list = (0L);
    validating = false;
    initialized = false;
    valid = false;
    changed_first = (0L);
    changed_last = (0L);
  }

  ~Am_Web_Data ()
  {
    // Remove from prototype's list
    if (prototype) {
      Am_Web_Data* curr = prototype->child;
      Am_Web_Data* prev = (0L);
      while (curr) {
        if (curr == this) {
          if (prev)
            prev->sibling = sibling;
          else
            prototype->child = sibling;
          sibling = (0L);
          break;
        }
        prev = curr;
        curr = curr->sibling;
      }
    }
    // Move children to prototype's list
    Am_Web_Data* curr = child;
    Am_Web_Data* next;
    while (curr) {
      next = curr->sibling;
      curr->prototype = prototype;
      if (prototype) {
        curr->sibling = prototype->child;
        prototype->child = curr;
      }
      else
        curr->sibling = (0L);
      curr = next;
    }
    child = (0L);
    prototype = (0L);
  }

  operator Am_Constraint* ()
  {
    return &interface;
  }

  void Remove_Input (const Am_Slot& removing_slot);
  void Enqueue (Input_Port* new_port);
  void Clear_Invalid ();
  void Clear_Changed ();
  bool Validate (const Am_Slot& context, Am_Value& out_value, bool& out_changed);
  void Invalidate (const Am_Slot& slot_invalidated);

  Am_Web_Create_Proc* create_proc;
  Am_Web_Initialize_Proc* initialize_proc;
  Am_Web_Validate_Proc* validate_proc;
  Am_Web_Data* prototype;
  Am_Web_Data* child;
  Am_Web_Data* sibling;
  Output_Port* out_list;
  Input_Port* in_list;
  Input_Port* invalid_list;
  Input_Port* changed_first;
  Input_Port* changed_last;
  Web_Constraint interface;
  bool initialized;
  bool valid;
  bool validating;

  static Am_ID_Tag id;
};

class Web_Context : public Am_Constraint_Context {
 public:
  Am_ID_Tag ID ();
  const Am_Value& Get (const Am_Object_Advanced& object, Am_Slot_Key key,
		       Am_Slot_Flags flags);
  void Set (const Am_Object_Advanced& object, Am_Slot_Key key,
            const Am_Value& new_value, Am_Slot_Flags flags);
  void Note_Changed (const Am_Object_Advanced&, Am_Slot_Key);
  void Note_Unchanged (const Am_Object_Advanced&, Am_Slot_Key);
  const Am_Value& Raise_Get_Exception (const Am_Value& value,
			   const Am_Object_Advanced&, Am_Slot_Key,
			   Am_Slot_Flags flags, const char *msg);
  Am_Wrapper* Get_Data ();
  void Set_Data (Am_Wrapper* data);

  Web_Context (Am_Web_Data* in_owner, const Am_Slot& in_validating_slot)
      : validating_slot (in_validating_slot)
  {
    owner = in_owner;
    been_set = false;
    changed = false;
  }

  Am_Web_Data* owner;

  Am_Slot validating_slot;
  Am_Value value;
  bool changed;
  bool been_set;
};

class Am_Web_Events_Data {
 public:
  Am_Web_Events_Data (Am_Web_Data* in_web)
  {
    web = in_web;
    current = (0L);
  }

  Am_Web_Data* web;
  Input_Port* current;
};

class Output_Port {
#ifdef MEMORY
 public:
  void* operator new (size_t)
  {
     return memory.New ();
  }
  void operator delete (void* ptr, size_t)
  {
    memory.Delete (ptr);
  }
  static Dyn_Memory_Manager memory;
#endif
 public:
  Output_Port (const Am_Slot& in_context)
      : context (in_context)
  {
    next = (0L);
  }

  Output_Port* Search (const Am_Slot& target)
  {
    Output_Port* curr;
    for (curr = this; curr; curr = curr->next)
      if (curr->context == target)
        return curr;
    return (0L);
  }

  Output_Port* Search (const Am_Object_Advanced& object, Am_Slot_Key key)
  {
    Output_Port* curr;
    for (curr = this; curr; curr = curr->next)
      if ((curr->context.Get_Owner () == object) &&
          (curr->context.Get_Key () == key))
        return curr;
    return (0L);
  }

  void Destroy (Am_Constraint* self)
  {
    Am_Slot slot = context;
    context = (Am_Slot_Data*)0;
    if (next) {
      next->Destroy (self);
      next = (0L);
    }
    Am_Constraint_Iterator iter (slot);
    Am_Constraint* curr;
    for (iter.Start (); !iter.Last (); iter.Next ()) {
      curr = iter.Get ();
      if (curr == self) {
        Am_Constraint_Tag tag = iter.Get_Tag ();
        slot.Remove_Constraint (tag);
        break;
      }
    }
    delete this;
  }

  Am_Slot context;    // slot that port services
  Output_Port* next;   // general output port list
};

#ifdef MEMORY
Dyn_Memory_Manager Output_Port::memory (sizeof (Output_Port), "Output_Port");
#endif

class Input_Port {
#ifdef MEMORY
 public:
  void* operator new (size_t)
  {
     return memory.New ();
  }
  void operator delete (void* ptr, size_t)
  {
    memory.Delete (ptr);
  }
  static Dyn_Memory_Manager memory;
#endif
 public:
  Input_Port (const Am_Slot& in_context)
      : context (in_context)
  {
    next = (0L);
    next_invalid = (0L);
    next_changed = (0L);
    prev_changed = (0L);
    output_port = (0L);
    invalid = false;
    changed = false;
  }

  Input_Port* Search (const Am_Slot& target)
  {
    Input_Port* curr;
    for (curr = this; curr; curr = curr->next)
      if (curr->context == target)
        return curr;
    return (0L);
  }

  Input_Port* Search (const Am_Object_Advanced& object, Am_Slot_Key key)
  {
    Input_Port* curr;
    for (curr = this; curr; curr = curr->next)
      if ((curr->context.Get_Owner () == object) &&
          (curr->context.Get_Key () == key))
        return curr;
    return (0L);
  }

  void Destroy (Am_Constraint* self)
  {
    Am_Slot slot = context;
    context = (Am_Slot_Data*)0;
    if (next) {
      next->Destroy (self);
      next = (0L);
    }
    Am_Dependency_Iterator iter (slot);
    Am_Constraint* curr;
    for (iter.Start (); !iter.Last (); iter.Next ()) {
      curr = iter.Get ();
      if (curr == self) {
        Am_Constraint_Tag tag = iter.Get_Tag ();
        slot.Remove_Dependency (tag);
        break;
      }
    }
    delete this;
  }

  Am_Slot context;         // slot that port services
  Input_Port* next;         // general input port list
  Input_Port* next_invalid; // invalid port list
  Input_Port* next_changed; // changed port list
  Input_Port* prev_changed; // changed port list
  Output_Port* output_port; // connection to corresponding output_port
  bool invalid;
  bool changed;

  Am_Value prev_value;
};

#ifdef MEMORY
Dyn_Memory_Manager Input_Port::memory (sizeof (Input_Port), "Input_Port");
#endif

/*****************************************************************************
 *  Implementation of Web_Constraint                                         *
 *****************************************************************************/

bool Web_Constraint::Get (const Am_Slot& fetching_slot, Am_Value& out_value,
			  bool& out_changed)
{
  if (!owner->valid && !owner->validating)
    return owner->Validate (fetching_slot, out_value, out_changed);
  return false;
}

void Web_Constraint::Invalidated (const Am_Slot& slot_invalidated,
				  Am_Constraint* invalidating_constraint,
				  const Am_Value&)
{
  if (invalidating_constraint == this)
    return;
  Input_Port* port = owner->in_list->Search (slot_invalidated);
  if (port && !port->invalid && !port->changed) {
    port->invalid = true;
    port->next_invalid = owner->invalid_list;
    owner->invalid_list = port;
    owner->Invalidate (slot_invalidated);
  }
}

void Web_Constraint::Changed (const Am_Slot& slot_changed,
			      Am_Constraint* changing_constraint,
			      const Am_Value& old_value,
			      const Am_Value& new_value)
{
  if (changing_constraint == this
#ifndef SEND_ONLY_CHANGES
      || old_value == new_value
#endif
      )
    return;
  Input_Port* port = owner->in_list->Search (slot_changed);
  if (!port)
    return;
  // check if port changed twice (cannot maintain multiple values)
  if (port->changed) {
    Am_Value temp_value; bool temp_changed;
    owner->Validate (slot_changed, temp_value, temp_changed);
  }
  port->prev_value = old_value;
  owner->Enqueue (port);
  /*if (!slot_changed)*/
    owner->Invalidate (slot_changed);
}

void Web_Constraint::Changed (const Am_Slot& slot_changed,
			      Am_Constraint* changing_constraint)
{
  if (changing_constraint == this)
    return;
  Input_Port* port = owner->in_list->Search (slot_changed);
  if (!port)
    return;
  if (port->changed) {
    Am_Value temp_value; bool temp_changed;
    owner->Validate (slot_changed, temp_value, temp_changed);
  }
  owner->Enqueue (port);
  /*if (!slot_changed)*/
    owner->Invalidate (slot_changed);
}

void Web_Constraint::Slot_Event (Am_Object_Context* oc, const Am_Slot& slot)
{
  Output_Port* curr;
  for (curr = owner->out_list; curr; curr = curr->next) {
    if (curr->context != slot)
      curr->context.Event (oc);
  }
}

Am_Constraint* Web_Constraint::Get_Prototype ()
{
  return *owner->prototype;
}

bool Web_Constraint::Is_Instance_Of (Am_Constraint* proto)
{
  if (proto->ID () == Am_Web_Data::id) {
    Am_Web_Data* proto_data = ((Web_Constraint*)proto)->owner;
    Am_Web_Data* curr;
    for (curr = owner; curr; curr = curr->prototype)
      if (curr == proto_data)
        return true;
  }
  return false;
}

Am_Constraint* Web_Constraint::Constraint_Added (const Am_Slot& adding_slot)
{
  if (owner->out_list->Search (adding_slot))
    // slot already constrained by this constraint
    return (Am_Constraint*)0;

  Output_Port* port = new Output_Port (adding_slot);
  port->next = owner->out_list;
  owner->out_list = port;

  // bond input port to corresponding output port
  Input_Port* curr = owner->in_list->Search (adding_slot);
  if (curr)
    curr->output_port = port;

  return this;
}

void Web_Constraint::Constraint_Removed (const Am_Slot& removing_slot)
{
  Output_Port* prev = (0L);
  Output_Port* curr = owner->out_list;
  while (curr) {
    if (curr->context == removing_slot) {
      if (prev)
	prev->next = curr->next;
      else
	owner->out_list = curr->next;
      curr->next = (0L);
      delete curr;
      if (owner->create_proc && owner->create_proc (removing_slot)) {
        if (owner->in_list) {
	  owner->in_list->Destroy (this);
	  owner->in_list = (0L);
	}
        if (owner->out_list) {
	  owner->out_list->Destroy (this);
	  owner->out_list = (0L);
	}
      }
      if (!owner->in_list && !owner->out_list)
        delete owner;
      break;
    }
    prev = curr;
    curr = curr->next;
  }
}

Am_Constraint* Web_Constraint::Dependency_Added (const Am_Slot& adding_slot)
{
  if (owner->in_list->Search (adding_slot))
    // slot already a dependency
    return (Am_Constraint*)0;

  Input_Port* port = new Input_Port (adding_slot);
  port->next = owner->in_list;
  owner->in_list = port;

  // bond input port to corresponding output port
  Output_Port* curr = owner->out_list->Search (adding_slot);
  if (curr)
    port->output_port = curr;

  return this;
}

void Web_Constraint::Dependency_Removed (const Am_Slot& removing_slot)
{
  Input_Port* prev = (0L);
  Input_Port* port = owner->in_list;
  while (port) {
    if (port->context == removing_slot) {
      if (prev)
	prev->next = port->next;
      else
	owner->in_list = port->next;
      port->next = (0L);
      if (port->prev_changed)
        port->prev_changed->next_changed = port->next_changed;
      if (owner->changed_first == port)
        owner->changed_first = port->next_changed;
      if (port->next_changed)
        port->next_changed->prev_changed = port->prev_changed;
      if (owner->changed_last == port)
        owner->changed_last = port->prev_changed;
      port->next_changed = (0L);
      port->prev_changed = (0L);
      prev = (0L);
      Input_Port* curr = owner->invalid_list;
      while (curr) {
        if (curr == port) {
          if (prev)
            prev->next_invalid = curr->next_invalid;
          else
            owner->invalid_list = curr->next_invalid;
          break;
        }
        prev = curr;
        curr = curr->next_invalid;
      }
      port->next_invalid = (0L);
      delete port;
      if (!owner->in_list && !owner->out_list)
        delete owner;
      break;
    }
    prev = port;
    port = port->next;
  }
}

bool Web_Constraint::Is_Overridden_By
              (const Am_Slot& /*slot*/, Am_Constraint* /*competitor*/)
{
  // don't care about competing constraints
  return false;
}

Am_Constraint* Web_Constraint::Create (const Am_Slot&, const Am_Slot& new_slot)
{
  if (owner->create_proc && owner->create_proc (new_slot))
    return *(new Am_Web_Data (owner));
  return (0L);
}

Am_Constraint* Web_Constraint::Copy (const Am_Slot&, const Am_Slot& new_slot)
{
  if (owner->create_proc && owner->create_proc (new_slot)) {
    if (owner->prototype)
      return *(new Am_Web_Data (owner->prototype));
    else
      return *(new Am_Web_Data (owner->create_proc, owner->initialize_proc,
				owner->validate_proc));
  }
  return (0L);
}

Am_ID_Tag Web_Constraint::ID () const
{
  return Am_Web_Data::id;
}

/*****************************************************************************
 *  Implementation of Am_Web_Data                                            *
 *****************************************************************************/

Am_ID_Tag Am_Web_Data::id =
     Am_Get_Unique_ID_Tag(DSTR("Web_Constraint"), Am_CONSTRAINT);

void Am_Web_Data::Enqueue (Input_Port* new_port)
{
  new_port->changed = true;
  new_port->prev_changed = changed_last;
  if (changed_last)
    changed_last->next_changed = new_port;
  changed_last = new_port;
  if (!changed_first)
    changed_first = new_port;
}

void Am_Web_Data::Clear_Invalid ()
{
  Input_Port* curr = invalid_list;
  Input_Port* next;
  while (curr) {
    curr->invalid = false;
    next = curr->next_invalid;
    curr->next_invalid = (0L);
    if (!curr->changed)
      curr->context.Validate ();
    curr = next;
  }
  invalid_list = (0L);
}

void Am_Web_Data::Clear_Changed ()
{
  Input_Port* curr = changed_first;
  Input_Port* next;
  while (curr) {
    next = curr->next_changed;
    curr->prev_value = Am_No_Value;
    curr->changed = false;
    curr->next_changed = (0L);
    curr->prev_changed = (0L);
    curr = next;
  }
  changed_first = (0L);
  changed_last = (0L);
}

bool Am_Web_Data::Validate (const Am_Slot& context, Am_Value& out_value,
			    bool& out_changed)
{
  validating = true;
  Web_Context wc (this, context);
  if (initialized) {
    Clear_Invalid ();
    if (changed_first) {
      if (validate_proc) {
        Am_Web_Events events (this);
	Am_PUSH_CC(&wc)
        validate_proc (events);
	Am_POP_CC()
      }
      Clear_Changed ();
    }
  }
  else {
    if (initialize_proc) {
      Am_Web_Init init (this);
      Am_PUSH_CC(&wc)
      initialize_proc (context, init);
      Am_POP_CC()
    }
    initialized = true;
  }
  valid = true;
  validating = false;
  out_value = wc.value;
  out_changed = wc.changed;
  return wc.been_set;
}

void Am_Web_Data::Invalidate (const Am_Slot& slot_invalidated)
{
  valid = false;
  Output_Port* curr;
  for (curr = out_list; curr; curr = curr->next) {
    if (curr->context != slot_invalidated)
          curr->context.Invalidate (&interface);
  }
}

/*****************************************************************************
 *  Implementation of Web_Context                                            *
 *****************************************************************************/

Am_ID_Tag Web_Context::ID ()
{
  return Am_Web_Data::id;
}

const Am_Value& Web_Context::Get (const Am_Object_Advanced& object,
				  Am_Slot_Key key, Am_Slot_Flags flags)
{
  if (flags & Am_NO_DEPENDENCY)
    return Am_Empty_Constraint_Context->Get (object, key, flags);

  Input_Port* curr = owner->in_list->Search (object, key);
  if (curr) {
    curr->context.Validate ();
    return curr->context.Get ();
  }
  Am_Slot slot = object.Get_Slot (key);
  slot.Add_Dependency (*owner);
  slot.Validate ();
  return slot.Get();
}

void Web_Context::Set (const Am_Object_Advanced& object, Am_Slot_Key key,
                       const Am_Value& new_value, Am_Slot_Flags flags)
{
  if (flags & Am_NO_DEPENDENCY) {
    Am_Empty_Constraint_Context->Set (object, key, new_value, flags);
    return;
  }

  Output_Port* curr = owner->out_list->Search (object, key);
  if (curr) {
    if (curr->context == validating_slot) {
      been_set = true;
      value = new_value;
      changed = false;
    }
    else
      curr->context.Set (new_value, *owner);
    return;
  }
  Am_Slot slot = object.Get_Slot (key);
  slot.Set (new_value, *owner);
  slot.Add_Constraint (*owner);
}

void Web_Context::Note_Changed (const Am_Object_Advanced& object, Am_Slot_Key key)
{
  Output_Port* curr = owner->out_list->Search (object, key);
  if (curr) {
    if (curr->context == validating_slot) {
      been_set = true;
      changed = true;
    }
    else
      curr->context.Change (*owner);
    return;
  }
  Am_Slot slot = object.Get_Slot (key);
  slot.Change (*owner);
  slot.Add_Constraint (*owner);
}

void Web_Context::Note_Unchanged (const Am_Object_Advanced& object, Am_Slot_Key key)
{
  Output_Port* curr = owner->out_list->Search (object, key);
  if (curr) {
    if (curr->context == validating_slot) {
      been_set = true;
      changed = false;
    }
    return;
  }
  Am_Slot slot = object.Get_Slot (key);
  slot.Add_Constraint (*owner);
}

//in a formula, if error:     if Am_RETURN_ZERO_ON_ERROR_BIT returns zero
//			      if Am_OK_IF_NOT_THERE, returns the error
//			      else notes uninitialized, and returns zero
const Am_Value& Web_Context::Raise_Get_Exception (const Am_Value& value,
			   const Am_Object_Advanced&, Am_Slot_Key,
			   Am_Slot_Flags flags, const char *)
{
  if (flags & Am_RETURN_ZERO_ON_ERROR_BIT)
    return Am_Zero_Value;
  else if (flags & Am_OK_IF_NOT_THERE)
    return value;
  else {
    // NIY: note uninitialized
    return Am_Zero_Value;
  }
}

Am_Wrapper* Web_Context::Get_Data ()
{
//// NIY
  return 0;
}

void Web_Context::Set_Data (Am_Wrapper* /* data*/)
{
//// NIY
}

/*****************************************************************************
 *  Implementation of Am_Web_Events                                          *
 *****************************************************************************/

unsigned short Am_Web_Events::Length ()
{
  Input_Port* curr;
  unsigned short count = 0;
  for (curr = data->changed_first; curr; curr = curr->next_changed)
    ++count;
  return count;
}

void Am_Web_Events::Start ()
{
  current = (Am_Web_Event*)data->changed_first;
}

void Am_Web_Events::End ()
{
  current = (Am_Web_Event*)data->changed_last;
}

void Am_Web_Events::Next ()
{
  if (current)
    current = (Am_Web_Event*)((Input_Port*)current)->next_changed;
  else
    current = (Am_Web_Event*)data->changed_first;
}

void Am_Web_Events::Prev ()
{
  if (current)
    current = (Am_Web_Event*)((Input_Port*)current)->prev_changed;
  else
    current = (Am_Web_Event*)data->changed_last;
}

bool Am_Web_Events::Last ()
{
  return current == (0L);
}

bool Am_Web_Events::First ()
{
  return current == (0L);
}

Am_Value& Am_Web_Events::Get_Prev_Value ()
{
  if (current)
    return ((Input_Port*)current)->prev_value;
  else
  {
    return(Am_No_Value_Non_Const);
  }
}

Am_Slot Am_Web_Events::Get ()
{
  if (current)
    return ((Input_Port*)current)->context;
  else
    return 0;
}

bool Am_Web_Events::Find_Next (Am_Slot_Key /*key*/)
{
//// NIY
  return false;
}

bool Am_Web_Events::Find_Prev (Am_Slot_Key /*key*/)
{
//// NIY
  return false;
}

Am_Web_Events::Am_Web_Events (Am_Web_Data* in_data)
{
  data = in_data;
  current = (0L);
}

/*****************************************************************************
 *  Implementation of Am_Web_Init                                            *
 *****************************************************************************/

void Am_Web_Init::Note_Input (Am_Object_Advanced object, Am_Slot_Key key)
{
  object.Get_Slot (key).Add_Dependency (*data);
}

void Am_Web_Init::Note_Output (Am_Object_Advanced object, Am_Slot_Key key)
{
  object.Get_Slot (key).Add_Constraint (*data);
}

/*****************************************************************************
 *  Implementation of Am_Web                                                 *
 *****************************************************************************/

Am_Web::Am_Web (Am_Web_Create_Proc* in_create,
		Am_Web_Initialize_Proc* in_initialize,
		Am_Web_Validate_Proc* in_validate)
{
  create = in_create;
  initialize = in_initialize;
  validate = in_validate;
}

Am_Web::operator Am_Constraint* ()
{
  Am_Web_Data* data = new Am_Web_Data (create, initialize, validate);
  return *data;
}

Am_ID_Tag Am_Web::Web_ID ()
{
  return Am_Web_Data::id;
}
