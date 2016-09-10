/* ****************************** -*-c++-*- *******************************/
/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#ifndef OBJECT_ADVANCED_H
#define OBJECT_ADVANCED_H

#include "amulet/impl/am_object_advanced.h"

#define Am_RETURN_ZERO_ON_ERROR_BIT 0x02

class Am_Slot;

#include "amulet/impl/am_slot.h"

#include "amulet/impl/am_demon_queue.h"
#include "amulet/impl/am_demon_set.h"

class Am_Object_Context;

#include "amulet/impl/am_constraint.h"

#include "amulet/impl/object_debug.h"

class Am_Object_Advanced;


#include "amulet/impl/am_constraint_iterator.h"

#include "amulet/impl/am_dependency_iterator.h"

#include "amulet/impl/am_object_advanced.h"

#include "amulet/impl/am_constraint_context.h"

class _OA_DL_CLASSIMPORT Am_Explicit_Set : public Am_Constraint
{
	public:
	bool Get(const Am_Slot& fetching_slot, Am_Value& value, bool& changed);

	void Invalidated(const Am_Slot& slot_invalidated,
		Am_Constraint* invalidating_constraint,
		const Am_Value& value);
	void Changed(const Am_Slot& slot_changed,
		Am_Constraint* changing_constraint,
		const Am_Value& prev_value, const Am_Value& new_value);
	void Changed(const Am_Slot& slot_changed,
		Am_Constraint* changing_constraint);
	void Slot_Event(Am_Object_Context* oc, const Am_Slot& slot);
	Am_Constraint* Get_Prototype();
	bool Is_Instance_Of(Am_Constraint* proto);

	Am_Constraint* Constraint_Added(const Am_Slot& adding_slot);
	Am_Constraint* Dependency_Added(const Am_Slot& adding_slot);
	void Constraint_Removed(const Am_Slot& removing_slot);
	void Dependency_Removed(const Am_Slot& removing_slot);

	bool Is_Overridden_By(const Am_Slot& slot, 
		Am_Constraint *competing_constraint);

	Am_Constraint* Create(const Am_Slot& current_slot,
		const Am_Slot& new_slot);
	Am_Constraint* Copy(const Am_Slot& current_slot,
		const Am_Slot& new_slot);
	Am_ID_Tag ID() const;
	const char* Get_Name();

	Am_Explicit_Set(Am_Slot_Flags in_flags)
	{ flags = in_flags; 
	}
	operator Am_Constraint* ()
	{ return this; 
	}
	static bool Test(Am_Constraint* formula);
	static Am_Explicit_Set* Narrow(Am_Constraint* formula);

	static Am_ID_Tag id;

	Am_Slot_Flags flags;
};

extern void Ore_Initialize();

#endif

