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

#include <stdlib.h> // For abort ()

#include <string.h>

#include <am_inc.h>
#include <amulet/am_io.h>
#include <amulet/impl/types_logging.h>
#include <amulet/impl/types_basic.h>
#include <amulet/impl/types_type_support.h>
#include <amulet/am_value.hpp>
#include <amulet/misc.h>
#ifdef DEBUG
#include <amulet/registry.h>
#endif

#ifdef OA_VERSION
#include <amulet/univ_map_oa.hpp>
#else
#include <amulet/univ_map.h>
#endif

//for printout of values as a string for Am_Get_Name(Am_Value v)
#include <amulet/am_strstream.h>

void
Am_Null_Method_Error_Proc(...)
{
  Am_Error("** Invalid Method (with procedure ptr = 0) called.");
}

// Am_Null_Method_Error_Function is defined to 0 in nondebug mode,
// so we can't do this.
#ifdef DEBUG
Am_Any_Procedure *Am_Null_Method_Error_Function = &Am_Null_Method_Error_Proc;
#endif

void (*Am_Debugger)(void) = Am_Break_Into_Debugger;

void
Am_Error(const char *error_string)
{
  std::cerr << "** Amulet Error: " << error_string << std::endl;

#ifdef DEBUG
  if (Am_Debugger) {
    Am_Debugger();
  }
#endif

  std::cerr << "**  Program aborted." << std::endl;

#if defined(_WIN32)
  std::cerr << "(press return to exit)" << std::endl;
  getchar();
#endif

  abort();
}

void
Am_Error()
{
#ifdef DEBUG
  if (Am_Debugger)
    Am_Debugger();
#endif

  std::cerr << "**  Program aborted." << std::endl;
#if defined(_WIN32)
  std::cerr << "(press return to exit)" << std::endl;
  getchar();
#endif
  abort();
}

void
Am_Standard_Print(std::ostream &out, unsigned long val, Am_ID_Tag type)
{
#ifdef DEBUG
  out << "(" << Am_Get_Type_Name(type) << ") " << std::hex << val << std::dec;
#else
  out << "(TypeID=" << type << ") " << std::hex << val << std::dec;
#endif
}

#ifdef DEBUG
Am_ID_Tag
Am_Get_Unique_ID_Tag(const char *type_name, Am_ID_Tag in_class)
#else
Am_ID_Tag
Am_Get_Unique_ID_Tag(const char * /*type_name*/, Am_ID_Tag in_class)
#endif
{
#define _FIRST_TAG 500
  static Am_ID_Tag current_tag = _FIRST_TAG;
  Am_ID_Tag tag = current_tag | in_class;
  if (Am_Type_Base(in_class))
    tag = in_class;
  else
    current_tag++;
  if (current_tag < _FIRST_TAG) // this should never happen.
    Am_Error("*** Am_Get_Unique_ID_Tag: overflow!  Too many tags!");
#ifdef DEBUG
  if (type_name)
    Am_Register_Type_Name(tag, type_name);
#endif
  return tag;
}

Am_ID_Tag
Am_Get_Unique_ID_Tag(const char *type_name, Am_Type_Support *support,
                     Am_ID_Tag in_class)
{
  Am_ID_Tag tag = Am_Get_Unique_ID_Tag(type_name, in_class);
  Am_Register_Support(tag, support);
  return (tag);
}

#ifdef DEBUG
Am_ID_Tag
Am_Set_ID_Class(const char *type_name, Am_ID_Tag tag)
{
  Am_Register_Type_Name(tag, type_name);
  return tag;
}
#else
Am_ID_Tag
Am_Set_ID_Class(const char * /* type_name */, Am_ID_Tag tag)
{
  return tag;
}
#endif

static Am_Value_Type Simple = Am_Set_ID_Class(DSTR("Simple"), Am_SIMPLE_TYPE);
static Am_Value_Type EVT = Am_Set_ID_Class(DSTR("error"), Am_ERROR_VALUE_TYPE);
static Am_Value_Type WRAPPER = Am_Set_ID_Class(DSTR("wrapper"), Am_WRAPPER);
static Am_Value_Type METHOD = Am_Set_ID_Class(DSTR("method"), Am_METHOD);
static Am_Value_Type FENUM = Am_Set_ID_Class(DSTR("enum"), Am_ENUM);

static Am_Value_Type NONE = Am_Set_ID_Class(DSTR("NO VALUE"), 0x0000);
static Am_Value_Type ZERO = Am_Set_ID_Class(DSTR("ZERO"), Am_ZERO);
static Am_Value_Type INT = Am_Set_ID_Class(DSTR("int"), Am_INT);
static Am_Value_Type LONG = Am_Set_ID_Class(DSTR("long"), Am_LONG);
static Am_Value_Type BOOL = Am_Set_ID_Class(DSTR("bool"), Am_BOOL);
static Am_Value_Type FLOAT = Am_Set_ID_Class(DSTR("float"), Am_FLOAT);
static Am_Value_Type DOUB = Am_Set_ID_Class(DSTR("double"), Am_DOUBLE);
static Am_Value_Type STRIN = Am_Set_ID_Class(DSTR("string"), Am_STRING);
static Am_Value_Type CHAR = Am_Set_ID_Class(DSTR("char"), Am_CHAR);
static Am_Value_Type VOID = Am_Set_ID_Class(DSTR("void*"), Am_VOIDPTR);
static Am_Value_Type PROC =
    Am_Set_ID_Class(DSTR("Am_Generic_Procedure*"), Am_PROC);
static Am_Value_Type INVFORM =
    Am_Set_ID_Class(DSTR("Invalid_Formula_Error"), Am_FORMULA_INVALID);
static Am_Value_Type MISSSLOT =
    Am_Set_ID_Class(DSTR("Missing_Slot_Error"), Am_MISSING_SLOT);
static Am_Value_Type GETONNULL =
    Am_Set_ID_Class(DSTR("Get_On_Null_Object_Error"), Am_GET_ON_NULL_OBJECT);
static Am_Value_Type TYPETYPET =
    Am_Set_ID_Class(DSTR("Am_Value_Type"), Am_TYPE_TYPE);
static Am_Value_Type SLOTKEYTYPET =
    Am_Set_ID_Class(DSTR("Am_Slot_Key"), Am_SLOT_KEY_TYPE);

void
Am_Print_Type(std::ostream &os, Am_Value_Type type)
{
#ifdef DEBUG
  os << Am_Get_Type_Name(type);
#else
  os << type;
#endif
}

std::ostream &
operator<<(std::ostream &os, const Am_Value &value)
{
  value.Print(os);
  return os;
}

//////////////////////////////////////////////
// Implementation of Type wrapper registration

//if you put a type as the value of a value, with the type of the
//value being Am_TYPE_TYPE.
class Am_Type_Type_Support_Class : public Am_Type_Support
{
public:
  void Print(std::ostream &os, const Am_Value &value) const
  {
#ifdef DEBUG
    os << Am_Get_Type_Name((Am_ID_Tag)value.value.long_value);
#else
    os << value.value.long_value;
#endif
  }
#ifdef DEBUG
  const char *To_String(const Am_Value &value) const
  {
    return Am_Get_Type_Name((Am_ID_Tag)value.value.long_value);
#else
  const char *To_String(const Am_Value & /*value*/) const
  {
    return nullptr;
#endif
  }
  Am_Value From_String(const char * /*string*/) const
  {
    std::cerr << "**Sorry, no mapping from type names to Am_ID_Tags\n"
              << std::flush;
    return Am_No_Value; // not implemented since types use a 1-way hash table
  }
  ~Am_Type_Type_Support_Class() {}
};

Am_Type_Support *type_type_support = new Am_Type_Type_Support_Class();

#ifdef OA_VERSION
typedef OpenAmulet::Map<Am_Value_Type, Am_Type_Support *> Am_Map_Support;
typedef OpenAmulet::Map_Iterator<Am_Map_Support> Am_MapIterator_Support;
#else
AM_DECL_MAP(Support, Am_Value_Type, Am_Type_Support *)
AM_IMPL_MAP(Support, Am_Value_Type, Am_NONE, Am_Type_Support *, (0L))
#endif

static Am_Map_Support *Support_Table;

inline void
verify_support_table()
{
  if (!Support_Table) {
    Support_Table = new Am_Map_Support;
    Support_Table->SetAt(Am_TYPE_TYPE, type_type_support);
  }
}

void
Am_Register_Support(Am_Value_Type type, Am_Type_Support *support)
{
  verify_support_table();
  Support_Table->SetAt(type, support);
}

Am_Type_Support *
Am_Find_Support(Am_Value_Type type)
{
  verify_support_table();
  return Support_Table->GetAt(type);
}

void
Am_Type_Support::Print(std::ostream &out, const Am_Value &v) const
{
  Am_Standard_Print(out, (unsigned long)v.value.long_value, v.type);
}
const char *
Am_Type_Support::To_String(const Am_Value & /*v*/) const
{
  return nullptr;
}

Am_Value
Am_Type_Support::From_String(const char *) const
{
  return Am_No_Value;
}

// Used to get around this current C++ madness
void
reset_ostrstream(OSTRSTREAM &oss)
{
  oss.seekp(std::ios::beg);
}

// #endif // hide
