// registry.cc
// written by Alan Ferrency
// August 1995
// registry of names for debugging Amulet wrappers and method wrappers.

#include <string.h>

#include <am_inc.h>

#include <amulet/registry.h> // for registry stuff.
#include <amulet/types.h>    // for wrappers
#include <amulet/initializer.h>

#include <amulet/univ_map_oa.hpp>

// The OA version of maps does not work in this file
// There is a link-time problem with the Am_Map_Int2Str,
// and co. data types (they remain undefined, both in the
// release and development version of the library).
// Reverting to old version. -- ortalo
// Maybe an explicit template instanciation is necessary ?
#ifdef OA_VERSION
// to use OA map version, we just link the old map names to the
// new implementations
typedef OpenAmulet::Map_Int2Ptr Am_Map_Int2Ptr;
typedef OpenAmulet::Map_Int2Str Am_Map_Int2Str;
typedef OpenAmulet::Map_Ptr2Int Am_Map_Ptr2Int;
typedef OpenAmulet::Map_Str2Int Am_Map_Str2Int;
#else
#include <amulet/univ_map.h>

// Map (hash table) int -> void*
AM_DECL_MAP(Int2Ptr, int, void *)

// Map int -> char*
AM_DECL_MAP(Int2Str, int, char *)

// Map void* -> int
AM_DECL_MAP(Ptr2Int, void *, int)

// Map char* -> int
AM_DECL_MAP(Str2Int, char *, int)

// Table (double hash table) int <-> char*
AM_DECL_TABLE(Int2Str, Str2Int, int, char *)

AM_IMPL_MAP(Int2Ptr, int, 0, void *, (0L))
AM_IMPL_MAP(Int2Str, int, 0, char *, (0L))
AM_IMPL_MAP(Ptr2Int, void *, (0L), int, 0)
AM_IMPL_MAP(Str2Int, char *, (0L), int, 0)
#endif

// only compile this in if we're debugging.
#ifdef DEBUG

#ifdef OA_VERSION
// needs to be included as OA_MAP has a dump function which uses operator <<
// see univ_map_oa.hpp for function dump()
std::ostream &
operator<<(std::ostream &os, const Am_Registry_Key &aRegistryKey)
{
  std::cout << "Adr: " << &aRegistryKey << "\t" << aRegistryKey.Name()
            << std::endl;
  return (os);
}

std::ostream &
operator<<(std::ostream &os, Am_Registry_Key &aRegistryKey)
{
  std::cout << "Adr: " << &aRegistryKey << "\t" << aRegistryKey.Name()
            << std::endl;
  return (os);
}
#endif

// Implementation details of the registry:
//
// We have a one-way hash table which hashes items (registry entries can be
// wrappers, methods, or constraints) according to their registry key (the
// name and type_ID of the item combined).  For registry entries, we really
// want to key on two pieces of information, the string name of the item, and
// its type ID.  We need both pieces of information to determine that two keys
// are equal.
bool
Am_Registry_Key::operator==(const Am_Registry_Key &test_key) const
{
  if (test_key.name && name)
    return !strcmp(name, test_key.name);
  else
    return false; // no null names are equal to anything
}
bool
Am_Registry_Key::operator!=(const Am_Registry_Key &test_key) const
{
  return !(*this == test_key);
}
bool
Am_Registry_Key::Valid()
{
  return !!(name); // if it's !=0
}
Am_Registry_Key::operator const char *() const { return (name); }

int
HashValue(Am_Registry_Key key, int size)
{
  // af1x
  // This is based on the string hash function from univ_map.cc
  // That one simply summed up the first two and last two characters,
  // and normalized.
  // With most object names, etc, we're going to have the most useful
  // information in the end of the string (often "..._123_127_...") and not
  // in the beginning of the string (usually "Am...") so I'll just use the
  // last four characters instead of anything from the beginning.

  unsigned base;
  const char *name = key.Name();
  unsigned len = name ? strlen(name) : 0;
  switch (len) {
  case 0:
    return 0;
  case 1:
    base = name[0] * 4;
    break;
  case 2:
    base = name[0] * 2 + name[1] * 2;
    break;
  case 3:
    base = name[0] + name[1] + name[2] * 2;
    break;
  default:
    base = name[len - 1] + name[len - 2] + name[len - 3] + name[len - 4];
  }
  return base * unsigned(0x10000L / 4 / 0x100) % size;
}

int
HashValue(const Am_Registered_Type *entry, int size)
{
  // just use the pointer
  // *OLD*  return (long)entry % size ;
  //new: from Yann LE BIANNIC <lebiannic@dassault-avion.fr> 26 Jun 1996
  // the division by sizeof(entry) is intended to prevent "holes" in
  // the hash table
  return ((unsigned long)entry / sizeof(entry)) % size;
}

int
KeyComp(Am_Registry_Key key1, Am_Registry_Key key2)
{
  // uses Am_Registry_Key::operator!=
  // returns strcmp-like compare results: 0 if equal, !=0 if unequal.
  return key1 != key2;
}

int
KeyComp(const Am_Registered_Type *key1, const Am_Registered_Type *key2)
{
  // uses Am_Registry_Entry::operator!=
  // returns strcmp-like compare results: 0 if equal, !=0 if unequal.
  return key1 != key2; // Am_Registered_Type* has no op== so
  // it should just compare pointers.
}

Am_Registry_Key Am_No_Registry_Key;
const Am_Registered_Type *Am_No_Registry_Entry = nullptr;

// declare and implement custom hash table type
#ifdef OA_VERSION
// we provide a less function which can be used by the Map_Str2Int (const char*) specialization,
// so that keys are compared as strings and not as pointers
template <class T>
struct Am_Registry_Key_less : public std::binary_function<T, T, bool>
{
  bool operator()(const T &x, const T &y) const
  {
    const char *xText = x.Name();
    const char *yText = y.Name();

    // in all other cases perform the comparison
    return (std::lexicographical_compare(xText, xText + strlen(xText), yText,
                                         yText + strlen(yText)));
  }
};

// ATTENTION: If you change this typedef, make the same change to
// the definition in opal.cpp before function Am_Cleanup()
typedef OpenAmulet::Table<Am_Registry_Key, const Am_Registered_Type *,
                          Am_Registry_Key_less<Am_Registry_Key>>
    Am_Table_Registry;
#else
// the forward mapping (key to entry)
AM_DECL_MAP(Registry, Am_Registry_Key, const Am_Registered_Type *)
AM_IMPL_MAP(Registry, Am_Registry_Key, Am_No_Registry_Key,
            const Am_Registered_Type *, Am_No_Registry_Entry);

// the reverse mapping (entry back to key)
AM_DECL_MAP(Registry_Reverse, const Am_Registered_Type *, Am_Registry_Key)
AM_IMPL_MAP(Registry_Reverse, const Am_Registered_Type *, Am_No_Registry_Entry,
            Am_Registry_Key, Am_No_Registry_Key);

// the bidirectional table
AM_DECL_TABLE(Registry, Registry_Reverse, Am_Registry_Key,
              const Am_Registered_Type *);
#endif

Am_Table_Registry *Am_Name_Registry = (0L);

void
Am_Cleanup_Registry()
{
#ifdef LEAK_TRACE
  DERR("Deleting Name Registry: Size is "
       << Am_Name_Registry->Current_Size().first << ","
       << Am_Name_Registry->Current_Size().second << std::endl);
#endif

  delete (Am_Name_Registry);
  Am_Name_Registry = (0L);
}

static Am_Initializer *registry_init =
    new Am_Initializer("Registry", nullptr, 1.0f, 120, Am_Cleanup_Registry);

// utility function to make sure registry is initialized
inline void
verify_name_registry()
{
  static bool initialized(false);
  // make the table pretty big, to hold lots of names.
  // around 1000 names registered in testwidgets (9-6-95)
  // this allocation is freed through the Am_Cleanup() function, see opal.cpp
  if (initialized == false) {
#ifdef OA_VERSION
    Am_Name_Registry =
        new Am_Table_Registry(Am_No_Registry_Key, Am_No_Registry_Entry);
#else
    Am_Name_Registry = new Am_Table_Registry(2000);
#endif

    initialized = true;
  }

  return;
}

// The registry routines which register an item/ name pair
void
Am_Register_Name(const Am_Registered_Type *item, const char *name)
{
  verify_name_registry();
  Am_Registry_Key key(name);
  if (key.Valid()) // only register valid keys
    Am_Name_Registry->SetAt(key, item);
}

// we unregister the names when they're deleted
void
Am_Unregister_Name(const Am_Registered_Type *item)
{
  verify_name_registry();

  // only unregister valid items, if the table still exists
  if (item && Am_Name_Registry != (0L))
    Am_Name_Registry->DeleteKey(item);
}

const char *
Am_Get_Name_Of_Item(const Am_Registered_Type *item)
{
  verify_name_registry();

  if (item && Am_Name_Registry != (0L))
    return (Am_Name_Registry->GetAt(item));
  else
    return nullptr;
}

const Am_Registered_Type *
Am_Get_Named_Item(const char *name)
{
  verify_name_registry();
  Am_Registry_Key key(name);

  if (key.Valid())
    return Am_Name_Registry->GetAt(key);
  else
    return Am_No_Registry_Entry;
}

// ----------------------------------------------------

Am_Map_Int2Str *Am_Type_Registry = nullptr;

inline void
verify_type_registry()
{
  if (Am_Type_Registry == nullptr)
    Am_Type_Registry = new Am_Map_Int2Str;
}

void
Am_Register_Type_Name(Am_ID_Tag id, const char *type_name)
{
  verify_type_registry();
  Am_Type_Registry->SetAt(id, type_name);
}

void
Am_Unregister_Type_Name(Am_ID_Tag id)
{
  verify_type_registry();
  Am_Type_Registry->DeleteKey(id);
}

const char *
Am_Get_Type_Name(Am_ID_Tag id)
{
  verify_type_registry();
  return Am_Type_Registry->GetAt(id);
}

#else // DEBUG

// Provide stubs for the above

#include <am_inc.h>
#include <amulet/registry.h> // for registry stuff.
#include <amulet/types.h>    // for wrappers

#undef Am_Get_Name_Of_Item

const char *
Am_Get_Name_Of_Item(const Am_Registered_Type *item)
{
  return (0);
}

#endif // DEBUG
