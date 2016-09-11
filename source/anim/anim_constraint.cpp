//
// ------------------------------------------------------------------------
// 		The OpenAmulet User Interface Development Environment
// ------------------------------------------------------------------------
// This code is based on the Amulet project of Carnegie Mellon University,
// and has been placed in the public domain.  If you are using this code
// or any part of OpenAmulet, please contact amulet@cs.cmu.edu to be put
// on the mailing list or have a look at:
// http://www.openip.org
//

#include <am_inc.h>

#include STANDARD_SLOTS__H
#include VALUE_LIST__H
#include FORMULA__H
#include OBJECT_ADVANCED__H
#include ANIM__H

Am_Define_Method_Type_Impl (Am_Anim_Update_Method);

// forward declaration
class Animation_Constraint;

// The Animation_Constraint class is the connection between slot messages and
// the interpolator.  Slot messages are filtered and sent to the interpolator
// and interpolator messages are relayed to the slot.
class Animation_Constraint : public Am_Constraint {
 public:
  bool Get (const Am_Slot&, Am_Value&, bool&);
  void Invalidated (const Am_Slot&, Am_Constraint*, const Am_Value&);
  void Changed (const Am_Slot& slot, Am_Constraint* invalidating_constraint,
		const Am_Value&, const Am_Value& new_value);
  void Changed (const Am_Slot& slot,
		Am_Constraint* invalidating_constraint);
  void Slot_Event (Am_Object_Context*, const Am_Slot&);
  Am_Constraint* Get_Prototype ();
  bool Is_Instance_Of (Am_Constraint* proto);
  Am_Constraint* Constraint_Added (const Am_Slot& adding_slot);
  Am_Constraint* Dependency_Added (const Am_Slot&);
  void Constraint_Removed (const Am_Slot& removing_slot);
  void Dependency_Removed (const Am_Slot&);
  bool Is_Overridden_By (const Am_Slot& slot,
			 Am_Constraint *competing_constraint);
  Am_Constraint* Create (const Am_Slot&, const Am_Slot&);
  Am_Constraint* Copy (const Am_Slot& current_slot,
		       const Am_Slot& new_slot);
  Am_ID_Tag ID () const;
  const char *Get_Name();

  Animation_Constraint (Am_Object in_interpolator);
  Animation_Constraint (Animation_Constraint* proto, Am_Object in_interpolator);
  ~Animation_Constraint ();
  void Initialize ();
  void Update ();

  void Init_Target_Value (const Am_Slot& new_slot);
  void Set_Target_Value (const Am_Slot& changed_slot, const Am_Value& new_value);

  // These are for the instance hiearchy.  The tree is represented as leftmost
  // child, right sibling.
  Animation_Constraint* prototype;
  Animation_Constraint* first_instance;
  Animation_Constraint* next_instance;

  // State data.  Represents the connection between slot and interpolator.
  Am_Slot primary_slot;
  Am_Object_Advanced interpolator;

  bool multi_slot;  // does interpolator control multiple slots?
                    // if true, then start, target, and SLOT_TO_ANIMATE
                    // are all lists

  bool must_initialize_instance;  // true iff interpolator is instance
                                  // which has not been completely
                                  // initialized yet (a call to Initialize()
                                  // is in the demon queue)

  bool change_enqueued; // true iff a call to Update() is in the demon queue

  bool must_abort_animation;  // true iff the next Update() call must abort
                              // the animation

  Am_Value start;       // starting value for next Update() call
  Am_Value target;      // target value for next Update() call

  // Animation_Constraint's unique id.
  static Am_ID_Tag id;
};

/*****************************************************************************
 *  Definition of Classes                                                    *
 *****************************************************************************/

// Use standard macro to generate ID.
Am_ID_Tag Animation_Constraint::id =
Am_Get_Unique_ID_Tag (DSTR("Animation_Constraint"), Am_CONSTRAINT);

// This constraint sets its value using the Am_Slot::Set method.  It never
// sets its value through Get.
bool Animation_Constraint::Get (const Am_Slot&, Am_Value&, bool&)
{
  return false;
}

// Invalidated messages do not matter because the slot will be revalidated
// through the validate demon (assuming it isn't turned off).
void Animation_Constraint::Invalidated (
  const Am_Slot& /* slot */,
  Am_Constraint* /* invalidating_constraint */,
  const Am_Value& /* new_value */)
{
  // std::cout << "Animation_Constraint::Invalidated " << slot.Get_Key() << ", " <<
  //  invalidating_constraint <<std::endl;
//   if ((invalidating_constraint != this) && interpolator.Valid()) {
//     Am_Value uninit;
//     uninit.Set_Uninitialized();

//     Set_Target_Value (slot, uninit);
//   }
}

// This is the demon that invokes the animator's update method whenever
// the slot changes.  It runs as a demon for two reasons: (1) for multi-slot
// animations, all Changed events must be processed before the update method
// is called; and (2) you can't just set the value of the animated slot
// during the Changed event, because if the slot is being validated by another
// constraint, the other constraint will win.  So we enqueue this
// slot demon instead to ensure that the transfer will happen later.
static void animator_changed_demon (Am_Slot slot)
{
  Am_Constraint_Iterator iter = slot;
  Am_Constraint* curr;
  for (iter.Start (); !iter.Last (); iter.Next ()) {
    curr = iter.Get ();
    if (curr->ID () == Animation_Constraint::id) {
      ((Animation_Constraint*)curr)->Update ();
      break;
    }
  }
}

//
// Update: std::ends a change to the interpolator.
//   WARNING: call this method only from a demon, because it performs
//    Gets on Am_Objects.  The constraint system appears to rely on the fact
//    that constraint solvers will never invoke the demon queue during
//    their Changed, Invalidated, etc. events.
//
void Animation_Constraint::Update ()
{
  // std::cout << "Animation_Constraint::Update" <<std::endl;

  if (change_enqueued) {
    change_enqueued = false;

    bool running = interpolator.Get (Am_RUNNING);

    if (running) {
      if (must_abort_animation) {
	must_abort_animation = false;
	Am_Abort_Animator (interpolator);
      }
      else {
	Am_Interrupt_Animator (interpolator, target);
	return;
      }
    }

    // Decide whether to start the animation.
    // Do so only if at least one slot's target is different from its start,
    // and all slots have initialized starts and targets
    bool different;
    bool initialized;
    if (!multi_slot) {
      different = (start != target);
      initialized = start.Exists() && target.Exists();
    }
    else {
      Am_Value_List starts = start;
      Am_Value_List targets = target;

      different = false;
      initialized = true;

      for (starts.Start(), targets.Start();
	   !starts.Last() && !targets.Last();
	   starts.Next(), targets.Next()) {
	Am_Value s = starts.Get ();
	Am_Value t = targets.Get ();

	if (!s.Exists() || !t.Exists())
	  initialized = false;

	if (s != t)
	  different = true;
      }
    }

    if (different && initialized)
      Am_Start_Animator (interpolator, start, target);
  }
}

void Animation_Constraint::Changed (const Am_Slot& slot,
              Am_Constraint* invalidating_constraint,
	      const Am_Value&, const Am_Value& new_value)
{
  Am_Object_Advanced object = slot.Get_Owner ();
  Am_Demon_Queue queue = object.Get_Queue ();

  queue.Prevent_Invoke();
  // MUST Release_Invoke(), SO DON'T RETURN -- use "goto done" instead

  // std::cout << "Animation_Constraint::Changed " << slot.Get_Key() << ", " <<
  //  invalidating_constraint << ", new value " << new_value <<std::endl;
  if (invalidating_constraint != this) {
    if (must_initialize_instance)
      Initialize ();

    // animated: should this change should be animated?
    bool animated = interpolator.Get_Slot (Am_ACTIVE).Get().Valid();
    //bool animated = interpolator.Get (Am_ACTIVE, Am_RETURN_ZERO_ON_ERROR);

    if (invalidating_constraint
	     && Am_Explicit_Set::Test (invalidating_constraint)) {
      Am_Slot_Flags flags =
	Am_Explicit_Set::Narrow (invalidating_constraint)->flags;

      if (flags & Am_WITH_ANIMATION)
	animated = true;
      else if (flags & Am_NO_ANIMATION)
	animated = false;
      else if (flags & Am_DONT_TELL_ANIMATORS)
	goto done;  // setter doesn't want us to know (FIX: this could
      // cause inconsistency between animators' Am_VALUE and the slot value)
    }

    // running: is the animator currently running?
    bool running = interpolator.Get_Slot (Am_RUNNING).Get().Valid();
    //bool running = interpolator.Get (Am_RUNNING, Am_RETURN_ZERO_ON_ERROR);

    if (running && !animated)
      // unanimated change which may have to abort a running animation
      must_abort_animation = true;

    if ((running || animated) && !change_enqueued) {
      // put a demon on the queue which will eventually send this change
      // to the animator (unless such a demon is already there)
      change_enqueued = true;
      queue.Enqueue (animator_changed_demon, 0, slot);
      // The which_bit parameter for Enqueue is zero so that
      // when the demon is called it won't affect the demon_bits
      // in the slot.

      // this is the first change (of possibly a series of Sets affecting
      // this constraint).  Initialize the starting value.
      start = target;
    }

    Set_Target_Value (slot, new_value);
    if (!animated)
      start = target;
  }

done:
  queue.Release_Invoke();
}

void Animation_Constraint::Changed (const Am_Slot& slot,
	      Am_Constraint* invalidating_constraint)
{
  if (invalidating_constraint != this) {
    Am_Value new_value = slot.Get();
    Changed (slot, invalidating_constraint, new_value, new_value);
  }
}

// This constraint will never have dependencies so slot event is useless.
void Animation_Constraint::Slot_Event (Am_Object_Context*, const Am_Slot&)
{ }

// Just return the prototype.
Am_Constraint* Animation_Constraint::Get_Prototype ()
{ return prototype; }

// This is a straightforward parent check.
bool Animation_Constraint::Is_Instance_Of (Am_Constraint* proto)
{
  Animation_Constraint* curr = this;
  while (curr) {
    if (curr == proto)
      return true;
    curr = curr->prototype;
  }
  return false;
}

static void animator_initialize_demon (Am_Slot slot)
{
  Am_Constraint_Iterator iter = slot;
  Am_Constraint* curr;
  for (iter.Start (); !iter.Last (); iter.Next ()) {
    curr = iter.Get ();
    if (curr->ID () == Animation_Constraint::id) {
      ((Animation_Constraint*)curr)->Initialize ();
      break;
    }
  }
}

void Animation_Constraint::Initialize ()
{
  if (must_initialize_instance) {
    must_initialize_instance = false;

    // This is a newly-created instance of a multi-slot animation constraint.
    // The instance must set itself into the additional slots
    // on the new object.
    Am_Object obj = primary_slot.Get_Owner ();
    Am_Value_List slots = interpolator.Get_Slot (Am_SLOT_TO_ANIMATE).Get();

    // skip the first slot, because it's the primary slot.
    // We're already attached to that one.
    slots.Start();
    slots.Next();

    for (; !slots.Last(); slots.Next())
      obj.Set ((int)slots.Get(), this);
  }
}

// When constraint is added, also insert this constraint as a
// dependency, too.  Also, initialize the interpolator.
Am_Constraint* Animation_Constraint::Constraint_Added (
               const Am_Slot& adding_slot)
{
  Am_Slot_Key key = (int) adding_slot.Get_Key();

  if (!(Am_Slot_Data*)primary_slot) {
    primary_slot = adding_slot;

    // Make sure interpolator has a handle to the object it's animating.
    interpolator.Set (Am_OPERATES_ON, primary_slot.Get_Owner());

    if (multi_slot) {
      // queue up a demon to connect this new instance to the other
      // slots it should animate
      Am_Object_Advanced object = primary_slot.Get_Owner();
      Am_Demon_Queue queue = object.Get_Queue ();
      queue.Enqueue (animator_initialize_demon, 0, primary_slot);
      // The which_bit parameter for Enqueue is zero so that when the demon is
      // called it won't affect the demon_bits in the slot.

      must_initialize_instance = true;
    }
    else
      interpolator.Set (Am_SLOT_TO_ANIMATE, key);
  }
  else {
    Am_Value_List slots;

    if (must_initialize_instance)
      Initialize ();

    if (multi_slot) {
      // already a multi-slot constraint
      slots = interpolator.Get_Slot (Am_SLOT_TO_ANIMATE).Get();
    }
    else {
      // convert to multi-slot constraint
      slots = Am_Value_List().Add (interpolator.Get_Slot (Am_SLOT_TO_ANIMATE)
				   .Get());
      multi_slot = true;
    }

    // add the slot to the SLOT_TO_ANIMATE list if it isn't already present
    slots.Start();
    if (!slots.Member (key))
      slots.Add ((int)key);

    interpolator.Set (Am_SLOT_TO_ANIMATE, slots);
  }

  Init_Target_Value (adding_slot);
  start = target;

  adding_slot.Add_Dependency (this);
  ((Am_Slot*)(&adding_slot))->Invalidate (this);
  return this;
}

// Adding the dependency was already expected so this message is vacuous.
Am_Constraint* Animation_Constraint::Dependency_Added (const Am_Slot&)
{
  return this;
}

void Animation_Constraint::Constraint_Removed (const Am_Slot& removing_slot)
{
  Am_Value slots_value = interpolator.Get (Am_SLOT_TO_ANIMATE);
  if (Am_Value_List::Test (slots_value)) {
    Am_Value_List slots = slots_value;
    Am_Slot_Key key = removing_slot.Get_Key ();
    slots.Start ();
    if (slots.Member ((int)key))
      slots.Delete ();
    interpolator.Set (Am_SLOT_TO_ANIMATE, slots);
	if (slots.Empty ())
      delete this;
  }
  else
    delete this;
}

// Deallocation is handled in Constraint_Removed so this is empty.
void Animation_Constraint::Dependency_Removed (const Am_Slot&)
{ }

bool Animation_Constraint::Is_Overridden_By
     (const Am_Slot& /*slot*/, Am_Constraint *competitor)
{
  // overridden by another animation constraint
  return (competitor && competitor->ID() == id);
}

// Slot is being instanced.  Make an instance of constraint.
Am_Constraint* Animation_Constraint::Create (const Am_Slot& current_slot,
					     const Am_Slot& /*new_slot*/)
{
  if (current_slot == primary_slot)
    return new Animation_Constraint (this, interpolator.Create());
  else
    return (0L);
}

// Slot is being copied.  Make a copy by making an instance of the prototype.
Am_Constraint* Animation_Constraint::Copy (const Am_Slot& current_slot,
					   const Am_Slot& /*new_slot*/)
{
  if (current_slot == primary_slot){
    Animation_Constraint *copy = new Animation_Constraint (prototype,
							   interpolator.Copy());
    copy->multi_slot = multi_slot;
    return copy;
  }
  else
    return (0L);
}

// Just return the id.
Am_ID_Tag Animation_Constraint::ID () const
{ return id; }

const char* Animation_Constraint::Get_Name ()
{
  return interpolator.Get_Name();
}

// This constructs a root node.
Animation_Constraint::Animation_Constraint (Am_Object in_interpolator)
{
  prototype = (0L);
  first_instance = (0L);
  next_instance = (0L);
  interpolator = (Am_Object_Advanced&)in_interpolator;
  interpolator.Set (Am_ANIM_CONSTRAINT, (Am_Ptr)this);
  interpolator.Set (Am_RUNNING, false);
  multi_slot = false;
  change_enqueued = false;
  must_initialize_instance = false;
  must_abort_animation = false;
}

// This creates an instance of a previous node.
Animation_Constraint::Animation_Constraint (Animation_Constraint* proto,
					    Am_Object in_interpolator)
{
  prototype = proto;
  first_instance = (0L);
  if (proto) {
    next_instance = proto->first_instance;
    proto->first_instance = (0L);
  }
  interpolator = (Am_Object_Advanced&)in_interpolator;
  interpolator.Set (Am_ANIM_CONSTRAINT, (Am_Ptr)this);
  interpolator.Set (Am_RUNNING, false);
  multi_slot = proto ? proto->multi_slot : false;
  change_enqueued = false;
  must_initialize_instance = false;
  must_abort_animation = false;
}

static void remove_dependency (Am_Slot slot, Animation_Constraint* self)
{
  Am_Dependency_Iterator iter (slot);
  for (iter.Start (); !iter.Last (); iter.Next ()) {
    Am_Constraint* curr = iter.Get ();
    if (curr == self) {
      Am_Constraint_Tag tag = iter.Get_Tag ();
      slot.Remove_Dependency (tag);
      break;
    }
  }
}

static void remove_constraint (Am_Slot slot, Animation_Constraint* self)
{
  Am_Constraint_Iterator iter (slot);
  for (iter.Start (); !iter.Last (); iter.Next ()) {
    Am_Constraint* curr = iter.Get ();
    if (curr == self) {
      Am_Constraint_Tag tag = iter.Get_Tag ();
      slot.Remove_Constraint (tag);
      break;
    }
  }
}

// This removes the constraint from the instance heirarchy.  Children of
// this node become root nodes.
Animation_Constraint::~Animation_Constraint ()
{
  Am_Value keys_value = interpolator.Get (Am_SLOT_TO_ANIMATE);
  Am_Object source = interpolator.Get (Am_OPERATES_ON);
  if (source.Valid ()) {
    Am_Object_Advanced obj_adv = (Am_Object_Advanced&)source;
    if (Am_Value_List::Test (keys_value)) {
      Am_Value_List keys = keys_value;
      for (keys.Start (); !keys.Last (); keys.Next ()) {
        Am_Slot_Key key = (Am_Slot_Key)(int)keys.Get ();
        Am_Slot slot = obj_adv.Get_Slot (key);
        remove_dependency (slot, this);
        remove_constraint (slot, this);
      }
    }
	else {
		Am_Slot_Key key = (Am_Slot_Key)(int)keys_value;
		Am_Slot slot = obj_adv.Get_Slot (key);
		remove_dependency (slot, this);
        remove_constraint (slot, this);
	}
  }

  interpolator.Set (Am_ANIM_CONSTRAINT, 0);

  Animation_Constraint* prev;
  Animation_Constraint* curr;
  // Disconnect instances.
  for (curr = first_instance; curr; curr = curr->next_instance)
    curr->prototype = (0L);
  first_instance = (0L);

  // Disconnect from prototype.
  if (prototype) {
    prev = (0L);
    curr = prototype->first_instance;
    while (curr) {
      if (curr == this) {
	if (prev)
	  prev->next_instance = curr->next_instance;
	else
	  prototype->first_instance = curr->next_instance;
      }
      prev = curr;
      curr = curr->next_instance;
    }
  }
  next_instance = (0L);
  prototype = (0L);
}

void Animation_Constraint::Set_Target_Value (const Am_Slot& changed_slot,
					     const Am_Value& new_value)
{
  if (must_initialize_instance)
    return;

  if (multi_slot) {
    Am_Slot_Key key = changed_slot.Get_Key ();
    Am_Value_List slots = interpolator.Get_Slot (Am_SLOT_TO_ANIMATE).Get();
    Am_Value_List targets = target;

    for (slots.Start(), targets.Start();
	 !slots.Last() && !targets.Last();
	 slots.Next(), targets.Next())
      if ((int)slots.Get() == key) {
	targets.Set (new_value);
	target = targets;
	return;
      }

   std::cerr << "** Animator " << interpolator << " received change notification for "
      << "unknown slot ";
    Am_Print_Key(std::cerr, key);
   std::cerr << "." <<std::endl;
    Am_Error ();
  }
  else
    target = new_value;
}

void Animation_Constraint::Init_Target_Value (const Am_Slot& new_slot)
{
  if (must_initialize_instance)
    return;

  Am_Object owner = primary_slot.Get_Owner ();
  Am_Object_Advanced obj = (Am_Object_Advanced&)owner;
  Am_Slot_Key key;
  Am_Slot_Key new_key = new_slot.Get_Key ();

  if (multi_slot) {
    Am_Value_List slots = interpolator.Get_Slot (Am_SLOT_TO_ANIMATE).Get();
    Am_Value_List targets;

    for (slots.Start(); !slots.Last(); slots.Next()) {
      key = (int)slots.Get ();
      targets.Add ((key == new_key) ? new_slot.Get() : obj.Get_Slot(key).Get());
    }

    target = targets;
  }
  else {
    key = (int)interpolator.Get_Slot (Am_SLOT_TO_ANIMATE).Get();
    target = (key == new_key) ? new_slot.Get() : obj.Get_Slot(key).Get();
  }
}

Am_Constraint* Am_Animate_With (const Am_Object& animator)
{
  Am_Value v;

  v = animator.Get (Am_ANIM_CONSTRAINT);

  if (v.Valid())
    return (Am_Constraint*)(Am_Ptr)v;
  else return new Animation_Constraint (animator);
}

Am_Object Am_Get_Animator (Am_Object obj, Am_Slot_Key key)
{
  Am_Slot slot = ((Am_Object_Advanced&)obj).Get_Slot (key);
  Am_Constraint_Iterator iter = slot;
  Am_Constraint* curr;
  for (iter.Start (); !iter.Last (); iter.Next ()) {
    curr = iter.Get ();
    if (curr->ID () == Animation_Constraint::id) {
      Animation_Constraint *anim_constraint = (Animation_Constraint*)curr;
      return anim_constraint->interpolator;
    }
  }

  return Am_No_Object;
}

Am_Object Am_Get_Animator (Am_Constraint *constraint)
{
  if (constraint->ID () == Animation_Constraint::id) {
    Animation_Constraint *anim_constraint = (Animation_Constraint*)constraint;
    return anim_constraint->interpolator;
  }
  else
    return Am_No_Object;
}

inline static void set_animated_slot (Am_Slot_Key key,
			       Am_Object_Advanced& obj,
			       Am_Constraint* constraint,
			       const Am_Value& value)
{
  if (constraint) {
    Am_Slot slot = obj.Get_Slot (key);
    slot.Set (value, constraint);
    slot.Invalidate (constraint);
  }
  else
    obj.Set (key, value, Am_KEEP_FORMULAS | Am_DONT_TELL_ANIMATORS);
}

void Am_Set_Animated_Slots(Am_Object interp)
{
	Am_Value curr_value = interp.Get (Am_VALUE);
	if (!curr_value.Exists()) return;

	Am_Object obj = interp.Get_Object (Am_OPERATES_ON);
	if (!obj.Valid()) return;

	// pass the computed value back to the animation constraint
	Am_Value slot_to_animate = interp.Get (Am_SLOT_TO_ANIMATE);
	Am_Constraint* constraint;

	constraint = (Am_Constraint*)(Am_Ptr)interp.Get(Am_ANIM_CONSTRAINT);

	//
	// Do we have a list of slots or just one?
	//
	if (Am_Value_List::Test (slot_to_animate))
	{
		Am_Value_List slots = slot_to_animate;

		//
		// Do we have a list of values, or just one?
		//
		if (Am_Value_List::Test(curr_value))
		{
			Am_Value_List values = curr_value;

			values.Start();

			//
			// Go through both lists, until one of them runs out, assigning
			// each slot the corresponding value.
			//
			for(slots.Start(); !slots.Last() && !values.Last(); slots.Next())
			{
				set_animated_slot ((int)slots.Get(), (Am_Object_Advanced&)obj, constraint, values.Get());
				values.Next();
			}
		}
		else
		{
			//
			// We've only got a single value, so assign it just to the
			// first slot.
			//
			set_animated_slot ((int)slots.Get(), (Am_Object_Advanced&)obj, constraint, curr_value);
		}
	}
	//
	// We come here if we only have a single slot.
	//
	else
	{
		//
		// If `curr_value' is really a list, then get its first value.
		//
		if (Am_Value_List::Test(curr_value))
			curr_value = ((Am_Value_List)curr_value).Get();

		//
		// We have just one slot so assign it the value.
		//
		set_animated_slot ((int)slot_to_animate, (Am_Object_Advanced&)obj, constraint, curr_value);
	}
}

