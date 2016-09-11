#ifndef TYPES_DEMON_H
#define TYPES_DEMON_H

class Am_Slot;

// The demon types.
// Object Demon - connected to objects.
// Slot Demon - connected to slots.
// Part Demon - connected to objects, but used when parts are added/removed.
// Type Check - connected to slots, used for testing types.
typedef void Am_Object_Demon(Am_Object object);
typedef void Am_Slot_Demon(Am_Slot first_invalidated);
typedef void Am_Part_Demon(Am_Object owner, Am_Object old_object,
                            Am_Object new_object);
typedef const char* Am_Type_Check(const Am_Value& value);

#endif // TYPES_DEMON_H
