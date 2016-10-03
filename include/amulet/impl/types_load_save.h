#ifndef TYPES_LOAD_SAVE_H
#define TYPES_LOAD_SAVE_H

#include <amulet/am_io.h>
#include "types_wrapper.h"
#include "types_method.h"

//---------------------------
// Loading and Saving Values
//---------------------------

// forward reference
class Am_Load_Save_Context;

// Method used to load a value from a stream.
// Use the context parameter to load nested items from the same stream.
AM_DEFINE_METHOD_TYPE(Am_Load_Method, Am_Value,
                      (std::istream & is, Am_Load_Save_Context &context))

// Method used to save a value into a stream.
// Use the context parameter to save nested items from the same stream.
AM_DEFINE_METHOD_TYPE(Am_Save_Method, void,
                      (std::ostream & os, Am_Load_Save_Context &context,
                       const Am_Value &value))

_OA_DL_IMPORT extern Am_Load_Method Am_No_Load_Method;
_OA_DL_IMPORT extern Am_Save_Method Am_No_Save_Method;

// forward reference of private class
class Am_Load_Save_Context_Data;

// This class holds the state of load or save in process.  This will keep
// track of the names of items loaded and will handle references to objects
// previously loaded or saved.
class _OA_DL_CLASSIMPORT Am_Load_Save_Context
{
  AM_WRAPPER_DECL(Am_Load_Save_Context)

public:
  Am_Load_Save_Context() { data = nullptr; }

  // This method is used to record items that are referred to by the objects
  // being loaded or saved, but the items themselves are permanent parts of
  // the application hence they shouldn't (or possibly can't) be saved as
  // well.  By providing a base number one can use the same name over and over.
  void Register_Prototype(const char *name, Am_Wrapper *value);
  void Register_Prototype(const char *name, unsigned base, Am_Wrapper *value);

  //returns the name if the value is registered as a prototype.  If
  //not registered, returns (nullptr)
  const char *Is_Registered_Prototype(Am_Wrapper *value);

  // Load methods are registered based on a string type name.  The type name
  // must be a single alphanumeric word (no spaces).  The loader is responsible
  // for returning a value for anything stream that is declared with that type
  // name.  If two or more methods are registered on the same name, the last
  // one defined will be used.
  void Register_Loader(const char *type_name, const Am_Load_Method &method);

  // Save methods are registered based on a value type.  If a single type can
  // be saved in multiple ways, it is up to the save method to delegate the
  // save to the proper method.  If two or more methods are registered on the
  // same type, the last one defined will be used.
  void Register_Saver(Am_ID_Tag type, const Am_Save_Method &method);

  // Reset internal counters and references.  Used to start a new load or save
  // session.  Should be called once per new stream before the first call to
  // Load or Save.  Will not remove registered items such as loaders, savers,
  // and prototypes.
  void Reset();

  // Load value from stream.  Call this once for each value saved.  Returns
  // Am_No_Value when stream is empty.
  Am_Value Load(std::istream &is);

  // Save value to stream.  Call once for each value saved.
  void Save(std::ostream &os, const Am_Value &value);

  // Structures that are recursive must call this function before Load is
  // called recursively.  The value is the pointer to the structure being
  // loaded.  This value will be used by internal values that refer to the
  // parent structure.
  void Recursive_Load_Ahead(Am_Wrapper *value);

  // This procedure must be called by each save method as the first thing it
  // writes to the stream.  The procedure will put the name into the stream
  // with the proper format.  After that, the save method can do whatever it
  // needs.
  void Save_Type_Name(std::ostream &os, const char *type_name);
};

_OA_DL_IMPORT extern Am_Load_Save_Context Am_No_Load_Save_Context;

#endif //TYPES_LOAD_SAVE_H
