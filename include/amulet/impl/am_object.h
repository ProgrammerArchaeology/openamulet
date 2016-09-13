#ifndef OBJECT_PURE_H
#define OBJECT_PURE_H

#include "types_wrapper.h"
#include "types_slot.h"

// The Am_Constraint_Context is an opaque type used to represent a constraint's
// local state when Am_Object methods are called within a constraint formula.
class Am_Constraint_Context;

// Am_Constraint represents a constraint object.  Its type is opaque in this
// context.  The file formula.h declares the standard amulet factory for
// creating constraints.  Am_Constraint itself is defined in object_advanced.h
class Am_Constraint;

// object system reserves slots 0-99
#define Am_NO_SLOT 0
#define Am_NO_INHERIT 2
//do not use 1 since true==1 as slot name has a special meaning for interactors
#define Am_OWNER 10
#define Am_PROTOTYPE 11
#define Am_SOURCE_OF_COPY 12

/////////////////////////////////////////////////////////////////////////////
// Main Object code
/////////////////////////////////////////////////////////////////////////////

#define Am_OK_IF_NOT_THERE 0x01      //use with Set
#define Am_RETURN_ZERO_ON_ERROR 0x03 // has Am_OK_IF_NOT_THERE set also
#define Am_NO_DEPENDENCY 0x04
#define Am_OK_IF_THERE 0x08 //use with Add
#define Am_OVERRIDE_READ_ONLY 0x10
// additional flags Am_KEEP_FORMULAS in formula.h and
// Am_NO_ANIMATION, Am_WITH_ANIMATION and Am_DONT_TELL_ANIMATORS in inter.h

class Am_Object_Data;

//:The Am_Object class is a wrapper class which holds a pointer to the real
// Amulet Object (this pointer (type is Am_Object_Data) is stored inside the 'data' member, which is
// introduced by the Am_WRAPPER_CLASS macro). This class
// is used to represent Amulet objects like windows, graphical objects, and
// interators. An Am_Object has very little overhead as it's only a wrapper
// and nothing more. An actual object is/can be stored inside an Am_Object
// with the use of the Create Method. This method allocates memroy etc. To get
// rid of the allocated object the destroy method has to be called. This is
// even true for stack-objects. If you have an Am_Object on the stack and
// reuse it in a loop to hold Amulet objects retured by create() remember to
// safe them elsewhere (so you can destroy them later) or you have to
// explicitely destroy them. Otherwise a memory-leak will occure.
//
// Example:
// 	for(int32 x=0; x<1000; ++x)
// 	{
// 		Am_Object tmp;
//
// 		tmp = Am_Rectangle.Create();
// 	}
//
// 	Even when the 'tmp' goes out of the scope of the for loop, 999
// 	Am_Objects will lay around in the memory.

class _OA_DL_CLASSIMPORT Am_Object
{
  AM_WRAPPER_DECL(Am_Object)

public:
  // CREATORS
  Am_Object() : data((0L)) { ; }

  //:Creates an instance of an Am_Object object with a name of 'new_name'
  // if given.
  _OA_DL_MEMBERIMPORT Am_Object
  Create(const char *new_name = static_cast<const char *>(0)) const;

  //:Makes a copy of the object as though an instance was created of the
  // object's prototype.  The slot values, and parts of the source are copied
  // as identically as possible.
  _OA_DL_MEMBERIMPORT Am_Object
  Copy(const char *new_name = static_cast<const char *>(0)) const;

  //:Like Copy except all slots that have constraints or are otherwise
  // actively determined are changed into simple values.  The constraints of which are
  // present in the source completely stripped out.
  _OA_DL_MEMBERIMPORT Am_Object
  Copy_Value_Only(const char *new_name = (0L)) const;

  // ACCESSORS
  //:Test whether 'test_object' is same as self.
  _OA_DL_MEMBERIMPORT bool operator==(const Am_Object &test_object) const;
  _OA_DL_MEMBERIMPORT bool operator!=(const Am_Object &test_object) const;

  //:Returns whether the prototype parameter is in the prototype chain of the object.
  // Will return true if the parameter is the object itself.
  _OA_DL_MEMBERIMPORT bool Is_Instance_Of(Am_Object prototype) const;

  //:Is the object a part of the object 'owner' parameter.
  // An object is considered to be part of itself.
  _OA_DL_MEMBERIMPORT bool Is_Part_Of(Am_Object owner) const;

  //:Get the value of a slot. Precondition: 'key' must exists
  _OA_DL_MEMBERIMPORT const Am_Value &Get(Am_Slot_Key key,
                                          Am_Slot_Flags flags = 0) const;

  //:Get the object's name in the global name registry
  _OA_DL_MEMBERIMPORT const char *Get_Name() const;

  //:Peek is a safe Get
  // when an error occurs, it returns an error value rather than crashing with a fatal error
  _OA_DL_MEMBERIMPORT const Am_Value &Peek(Am_Slot_Key key,
                                           Am_Slot_Flags flags = 0) const
  {
    return (Get(key, flags | Am_OK_IF_NOT_THERE));
  }

  //:Set the object's name in the global name registry
  Am_Object &Set_Name(const char *name);

  //:Prints all the slots of the object and their values for debugging to cout
  _OA_DL_MEMBERIMPORT void Text_Inspect() const;
  //:Prints one slot of the object and its values for debugging to cout
  _OA_DL_MEMBERIMPORT void Text_Inspect(Am_Slot_Key key) const;

  //:Returns an object's key if the object is a named part of another object.
  // Returns Am_NO_SLOT when the object is not a part or is an unnamed part.
  // Returns Am_NO_INHERIT which object is an unnamed part that doesn't
  // get inherited when owner is instantiated.
  _OA_DL_MEMBERIMPORT Am_Slot_Key Get_Key() const;

  //:Return the object from which this object was instanced.
  _OA_DL_MEMBERIMPORT Am_Object Get_Prototype() const;

  //:Queries a slot for an object.
  // This causes an error if the slot containts a non-object value.
  _OA_DL_MEMBERIMPORT Am_Object Get_Object(Am_Slot_Key key,
                                           Am_Slot_Flags flags = 0) const;

  //:Like Get_Object but doesn't generate an error of the slot doesn't exists
  _OA_DL_MEMBERIMPORT Am_Object Peek_Object(Am_Slot_Key key,
                                            Am_Slot_Flags flags = 0) const
  {
    return (Get_Object(key, flags | Am_OK_IF_NOT_THERE));
  }

  //:Returns the object's owner.
  // Returns (0L) if the object does not have an owner.
  _OA_DL_MEMBERIMPORT Am_Object Get_Owner(Am_Slot_Flags flags = 0) const
  {
    return (Get_Object(Am_OWNER, flags));
  }

  //:Find a named part of the owner of the actual object.
  _OA_DL_MEMBERIMPORT Am_Object Get_Sibling(Am_Slot_Key key,
                                            Am_Slot_Flags flags = 0) const
  {
    return (Get_Owner(flags).Get_Object(key, flags));
  }

  //:Like Get_Sibling but doesn't generate an error of the slot-key doesn't exists
  _OA_DL_MEMBERIMPORT Am_Object Peek_Sibling(Am_Slot_Key key,
                                             Am_Slot_Flags flags = 0) const
  {
    return (Get_Owner(flags).Get_Object(key, flags | Am_OK_IF_NOT_THERE));
  }

  //:Returns the type of the value stored in a slot.
  // If the slot does not exist, Am_NONE is returned.
  _OA_DL_MEMBERIMPORT Am_Value_Type Get_Slot_Type(Am_Slot_Key key,
                                                  Am_Slot_Flags flags = 0) const
  {
    return (Get(key, flags | Am_OK_IF_NOT_THERE).type);
  }

  //:Returns true if slot is inherited from some prototype.
  _OA_DL_MEMBERIMPORT bool Is_Slot_Inherited(Am_Slot_Key key) const;

  //:Make a slot independent from all other slots.
  void Make_Unique(Am_Slot_Key key);

  //:Returns if the value of slot 'key' is independent from all other slots in this object.
  bool Is_Unique(Am_Slot_Key key);

  // MANIPULATORS
  // Set the value of an existing slot.
  Am_Object &Set(Am_Slot_Key key, Am_Wrapper *value, Am_Slot_Flags flags = 0);
  Am_Object &Set(Am_Slot_Key key, Am_Ptr value, Am_Slot_Flags flags = 0);
  Am_Object &Set(Am_Slot_Key key, int value, Am_Slot_Flags flags = 0);
  Am_Object &Set(Am_Slot_Key key, long value, Am_Slot_Flags flags = 0);
  Am_Object &Set(Am_Slot_Key key, float value, Am_Slot_Flags flags = 0);
  Am_Object &Set(Am_Slot_Key key, double value, Am_Slot_Flags flags = 0);
  Am_Object &Set(Am_Slot_Key key, char value, Am_Slot_Flags flags = 0);
  Am_Object &Set(Am_Slot_Key key, const char *value, Am_Slot_Flags flags = 0);
  Am_Object &Set(Am_Slot_Key key, const Am_String &value,
                 Am_Slot_Flags flags = 0);
  Am_Object &Set(Am_Slot_Key key, Am_Generic_Procedure *value,
                 Am_Slot_Flags flags = 0);
  Am_Object &Set(Am_Slot_Key key, Am_Method_Wrapper *value,
                 Am_Slot_Flags flags = 0);
  Am_Object &Set(Am_Slot_Key key, const Am_Value &value,
                 Am_Slot_Flags flags = 0);
  Am_Object &Set(Am_Slot_Key key, Am_Constraint *constraint,
                 Am_Slot_Flags flags = 0);
  Am_Object &Set(Am_Slot_Key key, bool value, Am_Slot_Flags flags = 0);

  // Add a new slot.
  Am_Object &Add(Am_Slot_Key key, Am_Wrapper *value, Am_Slot_Flags flags = 0);
  Am_Object &Add(Am_Slot_Key key, Am_Ptr value, Am_Slot_Flags flags = 0);
  Am_Object &Add(Am_Slot_Key key, int value, Am_Slot_Flags flags = 0);
  Am_Object &Add(Am_Slot_Key key, long value, Am_Slot_Flags flags = 0);
  Am_Object &Add(Am_Slot_Key key, bool value, Am_Slot_Flags flags = 0);
  Am_Object &Add(Am_Slot_Key key, float value, Am_Slot_Flags flags = 0);
  Am_Object &Add(Am_Slot_Key key, double value, Am_Slot_Flags flags = 0);
  Am_Object &Add(Am_Slot_Key key, char value, Am_Slot_Flags flags = 0);
  Am_Object &Add(Am_Slot_Key key, const char *value, Am_Slot_Flags flags = 0);
  Am_Object &Add(Am_Slot_Key key, const Am_String &value,
                 Am_Slot_Flags flags = 0);
  Am_Object &Add(Am_Slot_Key key, Am_Generic_Procedure *value,
                 Am_Slot_Flags flags = 0);
  Am_Object &Add(Am_Slot_Key key, Am_Method_Wrapper *value,
                 Am_Slot_Flags flags = 0);
  Am_Object &Add(Am_Slot_Key key, const Am_Value &value,
                 Am_Slot_Flags flags = 0);
  // Add a new slot containing a constraint to the slot.
  Am_Object &Add(Am_Slot_Key key, Am_Constraint *constraint,
                 Am_Slot_Flags flags = 0);

  // Note_Changed allows one to cause a slot to act as though its value has
  // been changed even if the actual value stored has not changed.  This
  // function is especially useful for slot changes due to side effects.
  void Note_Changed(Am_Slot_Key key);

  void Note_Unchanged(Am_Slot_Key key);

  //:Adds a part to an object
  // The part can either be named by providing a key or unnamed. A position where
  // the new part is inserted can be given. The new part is inserted before that position
  // positions start with 0
  Am_Object &Add_Part(Am_Object new_part, bool inherit = true,
                      long aPositio = 0);

  //slot must not already exist (unless Am_OK_IF_THERE flag).
  //slot must never be a regular slot
  Am_Object &Add_Part(Am_Slot_Key key, Am_Object new_part,
                      Am_Slot_Flags set_flags = 0);

  //Slot must exist (unless Am_OK_IF_THERE), and must already be a part slot
  Am_Object &Set_Part(Am_Slot_Key key, Am_Object new_part,
                      Am_Slot_Flags set_flags = 0);

  //is that slot a part slot?  Also returns false if slot isn't there.
  _OA_DL_MEMBERIMPORT bool Is_Part_Slot(Am_Slot_Key key) const;

  // Destroys the slot.  Returns the original object so Remove_Slot can be put
  // into a chain of sets and add_parts, etc.
  Am_Object &Remove_Slot(Am_Slot_Key key);

  // Removes a part from an object.  The part can be named either by a key or
  // by the actual part object.  Returns the original object so Remove_Slot
  // can be put into a chain of sets and add_parts, etc.
  Am_Object &Remove_Part(Am_Slot_Key key);
  Am_Object &Remove_Part(Am_Object part);

  // Make the object no longer be a part.
  void Remove_From_Owner();

  // Removes all locally defined constraints from a slot.
  void Remove_Constraint(Am_Slot_Key key);

  // Destroy the object and all its parts.
  void Destroy();

  // Get and Set "Advanced" properties of slots
  Am_Object &Set_Demon_Bits(Am_Slot_Key key, unsigned short bits);
  _OA_DL_MEMBERIMPORT unsigned short Get_Demon_Bits(Am_Slot_Key key) const;

  Am_Object &Set_Inherit_Rule(Am_Slot_Key key, Am_Inherit_Rule rule);
  _OA_DL_MEMBERIMPORT Am_Inherit_Rule Get_Inherit_Rule(Am_Slot_Key key) const;

  Am_Object &Set_Type_Check(Am_Slot_Key key, unsigned short type);
  _OA_DL_MEMBERIMPORT unsigned short Get_Type_Check(Am_Slot_Key key) const;

  Am_Object &Set_Read_Only(Am_Slot_Key key, bool read_only);
  _OA_DL_MEMBERIMPORT bool Get_Read_Only(Am_Slot_Key key) const;

  // Runs all demons and validates all constraints on this object
  // (and on any other objects that happen to share the same demon queue)
  Am_Object &Validate();

protected:
  static Am_Constraint_Context *cc;
};

// Prints out an identifying name for the object to the output stream.
_OA_DL_IMPORT extern std::ostream &operator<<(std::ostream &os,
                                              const Am_Object &object);

// The (0L) object.
_OA_DL_IMPORT extern Am_Object Am_No_Object;

// Create a Type ID for Am_Object.
const Am_Value_Type Am_OBJECT = Am_WRAPPER_TYPE | 1;

#endif //OBJECT_PURE_H
