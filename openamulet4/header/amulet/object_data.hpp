// currently declared in impl.h
#include <am_inc.h>
#include "amulet/impl/types_wrapper.h"
#include "impl.h"	// Am_Slot_Data

class Am_Object_Data : public Am_Wrapper
{
		Am_WRAPPER_DATA_DECL(Am_Object)

	public:
		// CREATORS
			Am_Object_Data();
			Am_Object_Data(const char* schema_name, Am_Object_Data* in_prototype);
			~Am_Object_Data();

		// MANIPULATORS
			Am_Object_Data* create_object(const char* new_name);
			Am_Object_Data* copy_object(const char* new_name);
			Am_Object_Data* copy_object_value_only(const char* new_name);
			void 			invoke_create_demons();
			void 			invoke_copy_demons();
			void 			destroy_object();
			void 			validate_object();
			void 			note_parts();
			void 			demon_removal();
			Am_Slot_Data* 	find_slot(Am_Slot_Key key);
			Am_Slot_Data* 	find_prototype(Am_Slot_Key key);
			void 			find_slot_and_position(Am_Slot_Key key, Am_Slot_Data*& slot, unsigned& i);
			void 			set_slot(Am_Slot_Key key, const Am_Value& value, Am_Slot_Flags set_flags);
			void 			set_slot(Am_Slot_Key key, Am_Constraint* constraint, Am_Slot_Flags set_flags);
			bool 			notify_change(Am_Slot_Key key, Am_Constraint* cause, const Am_Value& old_value, const Am_Value& new_value);
			bool 			propagate_change(Am_Slot_Key key, Am_Constraint* cause, const Am_Value& old_value, const Am_Value& new_value);
			bool 			notify_change(Am_Slot_Key key, Am_Constraint* cause);
			bool 			propagate_change(Am_Slot_Key key, Am_Constraint* cause);
			bool 			notify_unique(Am_Slot_Key key, Am_Wrapper* new_value);
			bool 			propagate_unique(Am_Slot_Key key, Am_Wrapper* new_value);
			void 			sever_slot(Am_Slot_Data* proto);
			void 			sever_copies(Am_Slot_Data* proto);
			void 			remove_temporary_slot(Am_Slot_Key key);
			void 			remove_temporaries(Am_Slot_Key key);
			void 			convert_temporary_slot(Am_Slot_Key key);
			void 			convert_temporaries(Am_Slot_Key key);
			void 			remove_subconstraint(Am_Slot_Key key, Am_Constraint* proto);
			void 			prop_remove_constraint(Am_Slot_Key key, Am_Constraint* proto);
			void 			set_constraint(Am_Slot_Key key, Am_Constraint* constraint, Am_Slot_Data* proto);
			void 			propagate_constraint(Am_Slot_Key key, Am_Constraint* constraint, Am_Slot_Data* proto);
			void 			delete_slot(Am_Slot_Data* slot, Am_Slot_Data* proto_slot);
			void 			remove_part();
			void 			enqueue_change(Am_Slot_Data* slot);
			void 			enqueue_invalid(Am_Slot_Data* slot);
			void 			print_slot_name_and_value(Am_Slot_Key key, Am_Slot_Data* value) const;

		// OPERATORS
			bool operator== (Am_Object_Data&)		{return(false);}
			bool operator== (Am_Object_Data&) const	{return(false);}

		// ACCESSORS
			void Print(std::ostream& out) const;

		// DATA MEMBERS
			//:pointer to prototype's Am_Object_Data object
			Am_Object_Data		*prototype;

			//:pointer to first instance which used this object as a prototype
			// the pointed to object's next_instance data member leads to the next
			// instance which used this object as prototpye
			Am_Object_Data		*first_instance;

			//:pointer to next instance which was created from the same prototype
			// as the actual object. This pointer 'belongs' to the instance list
			// of the prototype for this object
			Am_Object_Data		*next_instance;

			//:Pointer to first part of this object
			Am_Object_Data		*first_part;
			//:Pointer to next part of the owner of this object
			Am_Object_Data		*next_part;

			Am_Demon_Set_Data	*demon_set;
			Am_Demon_Queue 		demon_queue;
			unsigned short 		default_bits;
			unsigned short 		bits_mask;
			unsigned short 		demons_active;

			//:Slot object that points to the Am_Object_Data of the owner of this object
			Am_Slot_Data 		owner_slot;
			Am_Slot_Data 		part_slot;

			Am_Inherit_Rule 	default_rule;

			DynArray 			data;

	protected:
	// CREATORS
	// MANIPULATORS
	// ACCESSORS
	// DATA MEMBERS

	private:
	// CREATORS
	// MANIPULATORS
	// ACCESSORS
	// DATA MEMBERS
};
