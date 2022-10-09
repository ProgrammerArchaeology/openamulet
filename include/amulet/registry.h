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

// registry.h
// name registration routines for Amulet wrappers, method wrappers, etc.
//
// General idea: we create a structure able to hold all the various types of
// things we want to name.  Currently I think this includes only Am_Wrappers
// and Am_Method_Wrappers.   Brad suggested storing only the ID of the item's
// type, and a pointer to the object.  I don't think this is sufficient.  If
// all we have is the ID, we  have no way to determine if the item is a
// wrapper or a method wrapper. So we couldn't get the name field out of the
// thing to compare it anyway. Obviously we will need to store the type ID,
// but this is already a part of the wrapper or method wrapper.  So I think
// all we really need to  store is, a flag determining whether it's a wrapper
// or method wrapper, and a union for those pointers. To compare IDs and names
// we simply go inside the wrapper pointer.
//
// We can automatically register names of objects when they're constructed,
// and unregister their names before they're destructed, so that shouldn't be
// hard to take care of.

// This code only needs to be included in debug versions of software; other
// versions do not need all the overhead.
#ifndef REGISTRY_H
#define REGISTRY_H
#ifdef DEBUG
#include <am_inc.h>
#include <amulet/impl/types_registered.h>
#include <amulet/impl/types_logging.h>

// This holds all the information we are keying off in the  registry
// hash table. This is just a char*, but we want a non-default HashVaule
// routine, so I'll make it a class for now.
// Eventually we should make HashValue a memberfunction of
// the hash table, in a template or something probably.
class Am_Registry_Key
{
public:
  Am_Registry_Key() : name(nullptr) { ; }
  Am_Registry_Key(const char *the_name) : name(the_name) { ; }
  Am_Registry_Key(const Am_Registry_Key &in_key) : name(in_key.name) { ; }

  ~Am_Registry_Key()
  {
/*				if(name == nullptr)
				{
					DERR("Destroying key: zero pointer " << std::endl);					
				}
				else
				{
					DERR("Destroying key: " << name << "\tAdr: " << this << std::endl);
				}
*/			}

Am_Registry_Key &operator=(const Am_Registry_Key &aAssignee)
{
  name = aAssignee.name;
  return (*this);
}

const char *Name() const { return (name); }

bool operator==(const Am_Registry_Key &test_key) const;
bool operator!=(const Am_Registry_Key &test_key) const;
operator const char *() const;
bool Valid();

protected:
const char *name;
};

extern void Am_Register_Name(const Am_Registered_Type *item, const char *name);
extern void Am_Unregister_Name(const Am_Registered_Type *item);
extern const char *Am_Get_Name_Of_Item(const Am_Registered_Type *item);
extern const Am_Registered_Type *Am_Get_Named_Item(const char *name);

extern void Am_Register_Type_Name(Am_ID_Tag id, const char *type_name);
extern void Am_Unregister_Type_Name(Am_ID_Tag id);

extern const char *Am_Get_Type_Name(Am_ID_Tag id);
#else
// debug is turned off.  compile out the registry calls.
#define Am_Register_Name(one, two) // Am_Register_Name
#define Am_Unregister_Name(one)    // Am_Unregister_Name
#define Am_Get_Name_Of_Item(one) 0 // Am_Get_Name_Of_Item
#define Am_Get_Named_Item(one) 0   // Am_Get_Named_Item

#define Am_Register_Type_Name(one, two) // Am_Register_Type_Name
#define Am_Unregister_Type_Name(one)    // Am_Unregister_Type_Name
#define Am_Get_Type_Name(one) 0         // Am_Get_Type_Name
#endif                                  // DEBUG
#endif
