#include <am_inc.h>
#include <amulet/am_strstream.h>
#include <amulet/am_value.hpp>
#include <amulet/stdvalue.h> // Am_Double_Data, ...
#include <amulet/registry.h> // Am_Get_Named_Item
#include <amulet/impl/types_logging.h>
#include <amulet/impl/types_type_support.h>
#include <amulet/impl/types_method.h>
#include <amulet/impl/types_string.h>

const Am_Value Am_No_Value;
Am_Value Am_No_Value_Non_Const;
Am_Value Am_Zero_Value(0, Am_ZERO);

// The Am_Value type procedures
Am_Value::Am_Value(Am_Wrapper *initial)
{
  if (initial != static_cast<Am_Wrapper *>(0)) {
    type = static_cast<Am_Value_Type>(initial->ID());
  } else {
    type = Am_WRAPPER_TYPE;
  }

  value.wrapper_value = initial;
}

Am_Value::Am_Value(double initial) : type(Am_DOUBLE)
{
  value.wrapper_value = new Am_Double_Data(initial);
}

Am_Value::Am_Value(const char *initial)
{
  type = Am_STRING;
  if (initial)
    value.wrapper_value = new Am_String_Data(initial);
  else
    value.wrapper_value = (0L);
}

Am_Value::Am_Value(const Am_String &initial)
{
  type = Am_STRING;
  value.wrapper_value = initial;
}

Am_Value::Am_Value(Am_Method_Wrapper *initial)
{
  value.method_value = initial;
  if (initial)
    type = (Am_Value_Type)initial->ID();
  else
    type = Am_METHOD_TYPE;
}

Am_Value::Am_Value(const Am_Value &initial)
{
  type = initial.type;
  value = initial.value;
  if (Am_Type_Is_Ref_Counted(type) && value.wrapper_value)
    value.wrapper_value->Note_Reference();
}

Am_Value::~Am_Value()
{
  if (Am_Type_Is_Ref_Counted(type) && value.wrapper_value)
    value.wrapper_value->Release();
}

static void
type_error(const char *type_name, const Am_Value &value)
{
  std::cerr << "** Stored value of Am_Value is not of " << type_name << " type."
            << std::endl;
  std::cerr << "** It contains a value of type ";
  Am_Print_Type(std::cerr, value.type);
  std::cerr << "." << std::endl;
  Am_Error();
}

Am_Value::operator Am_Wrapper *() const
{
  switch (Am_Type_Class(type)) {
  case Am_WRAPPER:
    if (value.wrapper_value)
      value.wrapper_value->Note_Reference();
    return value.wrapper_value;
  case Am_SIMPLE_TYPE:
    switch (type) {
    case Am_STRING:
    case Am_DOUBLE:
      if (value.wrapper_value)
        value.wrapper_value->Note_Reference();
      return value.wrapper_value;
    case Am_ZERO:
      return (0L);
    case Am_INT:
    case Am_LONG:
    case Am_BOOL:
    case Am_VOIDPTR:
    case Am_PROC:
      if (!value.voidptr_value)
        return (0L);
    }
  default:
    type_error("Am_Wrapper*", *this);
    return (0L);
  }
}

Am_Value::operator Am_Ptr() const { return (Am_Ptr)value.voidptr_value; }

Am_Value::operator int() const
{
  switch (type) {
  case Am_INT:
  case Am_LONG:
    return (int)value.long_value; //use compiler's conversion
  case Am_BOOL:
    if (value.bool_value)
      return 1;
    else
      return 0;
  case Am_ZERO:
    return 0;
  default:
    type_error("int", *this);
    return 0;
  }
}

Am_Value::operator long() const
{
  switch (type) {
  case Am_LONG:
  case Am_INT:
    return value.long_value;
  case Am_FLOAT:
    return (long)value.float_value;
  case Am_DOUBLE:
    return (long)(double)*(Am_Double_Data *)value.wrapper_value;
  case Am_BOOL:
    if (value.bool_value)
      return 1L;
    else
      return 0L;
  case Am_ZERO:
    return 0;
  default:
    type_error("long", *this);
    return 0L;
  }
}

Am_Value::operator bool() const
{
  if (Am_Type_Class(type) == Am_ERROR_VALUE_TYPE || type == Am_NONE) {
    type_error("bool", *this);
  }
  switch (type) {
  case Am_INT:
  case Am_LONG:
    return !!value.long_value;
  case Am_BOOL:
    return value.bool_value;
  case Am_FLOAT:
    return !!value.float_value;
  case Am_DOUBLE:
    return !!(double)*(Am_Double_Data *)value.wrapper_value;
  case Am_CHAR:
    return !!value.char_value;
  case Am_STRING:
    return !!(const char *)*(Am_String_Data *)value.wrapper_value;
  case Am_VOIDPTR:
    return !!value.voidptr_value;
  case Am_PROC:
    return !!value.proc_value;
  case Am_ZERO:
    return false;
  default:
    return !!value.voidptr_value;
  }
}

Am_Value::operator float() const
{
  switch (type) {
  case Am_FLOAT:
    return value.float_value;
  case Am_DOUBLE:
    return static_cast<float>(*(Am_Double_Data *)value.wrapper_value);
  case Am_INT:
  case Am_LONG:
    return (float)value.long_value;
  case Am_ZERO:
    return 0.0f;
  default:
    type_error("float", *this);
    return 0.0f;
  }
}

Am_Value::operator double() const
{
  switch (type) {
  case Am_FLOAT:
    return value.float_value;
  case Am_DOUBLE:
    return *(Am_Double_Data *)value.wrapper_value;
  case Am_INT:
  case Am_LONG:
    return value.long_value;
  case Am_ZERO:
    return 0.0;
  default:
    type_error("double", *this);
    return 0.0;
  }
}

Am_Value::operator char() const
{
  switch (type) {
  case Am_CHAR:
    return value.char_value;
  case Am_ZERO:
    return '\0';
  default:
    type_error("char", *this);
  }
  return 0; // should never get here, but need to return something
}

Am_Value::operator Am_Generic_Procedure *() const
{
  if (Am_Type_Class(type) == Am_METHOD) {
    if (value.method_value)
      type_error("Am_Generic_Procedure*", *this);
    return (0L);
  }
  switch (type) {
  case Am_PROC:
    return value.proc_value;
  case Am_ZERO:
    return (0L);
  case Am_INT:
  case Am_LONG:
  case Am_VOIDPTR:
    if (!value.voidptr_value)
      return (0L);
  default:
    type_error("Am_Generic_Procedure*", *this);
    return (0L);
  }
}

Am_Value::operator Am_Method_Wrapper *() const
{
  if (Am_Type_Class(type) == Am_METHOD)
    return value.method_value;
  switch (type) {
  case Am_ZERO:
    return (0L);
  case Am_PROC:
  case Am_INT:
  case Am_LONG:
  case Am_VOIDPTR:
    if (!value.voidptr_value)
      return (0L);
  default:
    type_error("Am_Method_Wrapper*", *this);
    return (0L);
  }
}

bool
Am_Value::operator==(Am_Wrapper *test_value) const
{
  if (Am_Type_Class(type) == Am_WRAPPER)
    return (value.wrapper_value == test_value) ||
           (test_value && value.wrapper_value &&
            (*value.wrapper_value == *test_value));
  switch (type) {
  case Am_STRING:
  case Am_DOUBLE:
    return (value.wrapper_value == test_value) ||
           (test_value && value.wrapper_value &&
            (*value.wrapper_value == *test_value));
  case Am_INT:
  case Am_LONG:
  case Am_VOIDPTR:
    if (!value.voidptr_value)
      return test_value == (0L);
  default:
    return false;
  }
}

bool
Am_Value::operator==(Am_Ptr test_value) const
{
  return value.voidptr_value == test_value;
}

bool
Am_Value::operator==(int test_value) const
{
  if (Am_Type_Class(type) == Am_ENUM)
    return value.long_value == test_value;
  else
    switch (type) {
    case Am_INT:
    case Am_LONG:
      return value.long_value == test_value;
    case Am_FLOAT:
      return value.float_value == test_value;
    case Am_DOUBLE:
      return (double)*(Am_Double_Data *)value.wrapper_value == test_value;
    default:
      return false;
    }
}

bool
Am_Value::operator==(long test_value) const
{
  if (Am_Type_Class(type) == Am_ENUM)
    return value.long_value == test_value;
  else
    switch (type) {
    case Am_LONG:
    case Am_INT:
      return value.long_value == test_value;
    case Am_FLOAT:
      return value.float_value == test_value;
    case Am_DOUBLE:
      return (double)*(Am_Double_Data *)value.wrapper_value == test_value;
    default:
      return false;
    }
}

bool
Am_Value::operator==(bool test_value) const
{
  if (Am_Type_Class(type) != Am_NONE)
    return !!value.voidptr_value == test_value;
  switch (type) {
  case Am_INT:
  case Am_LONG:
    return !!value.long_value == test_value;
  case Am_BOOL:
    return value.bool_value == test_value;
  case Am_FLOAT:
    return !!value.float_value == test_value;
  case Am_DOUBLE:
    return !!(double)*(Am_Double_Data *)value.wrapper_value == test_value;
  case Am_CHAR:
    return !!value.char_value == test_value;
  case Am_STRING:
    return !!(const char *)*(Am_String_Data *)value.wrapper_value == test_value;
  case Am_VOIDPTR:
    return !!value.voidptr_value == test_value;
  case Am_PROC:
    return !!value.proc_value == test_value;
  default:
    return false;
  }
}

bool
Am_Value::operator==(float test_value) const
{
  switch (type) {
  case Am_FLOAT:
    return value.float_value == test_value;
  case Am_DOUBLE:
    return (double)*(Am_Double_Data *)value.wrapper_value == test_value;
  case Am_INT:
  case Am_LONG:
    return value.long_value == test_value;
  default:
    return false;
  }
}

bool
Am_Value::operator==(double test_value) const
{
  switch (type) {
  case Am_FLOAT:
    return value.float_value == test_value;
  case Am_DOUBLE:
    return (double)*(Am_Double_Data *)value.wrapper_value == test_value;
  case Am_INT:
  case Am_LONG:
    return value.long_value == test_value;
  default:
    return false;
  }
}

bool
Am_Value::operator==(char test_value) const
{
  if (type == Am_CHAR)
    return value.char_value == test_value;
  else
    return false;
}

bool
Am_Value::operator==(const char *test_value) const
{
  switch (type) {
  case Am_STRING:
    return *(Am_String_Data *)value.wrapper_value == test_value;
  case Am_INT:
  case Am_LONG:
  case Am_VOIDPTR:
    return !value.voidptr_value && !test_value;
  default:
    return false;
  }
}

bool
Am_Value::operator==(const Am_String &test_value) const
{
  switch (type) {
  case Am_STRING:
    return *(Am_String_Data *)value.wrapper_value == test_value;
  case Am_INT:
  case Am_LONG:
  case Am_VOIDPTR:
    return !value.voidptr_value && !(const char *)test_value;
  default:
    return false;
  }
}

bool
Am_Value::operator==(Am_Generic_Procedure *test_value) const
{
  if (Am_Type_Class(type) == Am_METHOD)
    return !value.method_value && !test_value;
  switch (type) {
  case Am_PROC:
    return value.proc_value == test_value;
  case Am_INT:
  case Am_LONG:
  case Am_VOIDPTR:
    return !value.voidptr_value && !test_value;
  default:
    return false;
  }
}

bool
Am_Value::operator==(Am_Method_Wrapper *test_value) const
{
  if (Am_Type_Class(type) == Am_METHOD)
    return value.method_value == test_value;
  switch (type) {
  case Am_PROC:
  case Am_INT:
  case Am_LONG:
  case Am_VOIDPTR:
    return !value.voidptr_value && !test_value;
  default:
    return false;
  }
}

bool
Am_Value::operator==(const Am_Value &test_value) const
{
  switch (Am_Type_Class(type)) {
  case Am_SIMPLE_TYPE:
    switch (type) {
    case Am_STRING:
    case Am_DOUBLE:
      return test_value == value.wrapper_value;
    case Am_VOIDPTR:
      return test_value == value.voidptr_value;
    case Am_INT:
    case Am_LONG:
      return test_value == value.long_value;
    case Am_BOOL:
      return test_value == value.bool_value;
    case Am_FLOAT:
      return test_value == value.float_value;
    case Am_CHAR:
      return test_value == value.char_value;
    case Am_PROC:
      return test_value == value.proc_value;
    case Am_NONE:
      return test_value.type == Am_NONE;
    case Am_ZERO:
      return test_value.type == Am_ZERO;
    default:
      return false;
    }
  case Am_WRAPPER:
    return test_value == value.wrapper_value;
  case Am_METHOD:
    return test_value == value.method_value;
  case Am_ENUM:
    return type == test_value.type &&
           test_value.value.long_value == value.long_value;
  case Am_ERROR_VALUE_TYPE:
    return test_value.type == type;
  }
  return false; // should never get here, but need to return something
}

bool
Am_Value::operator!=(Am_Wrapper *test_value) const
{
  if (Am_Type_Class(type) == Am_WRAPPER)
    return (value.wrapper_value != test_value) &&
           (!test_value || !value.wrapper_value ||
            !(*value.wrapper_value == *test_value));
  switch (type) {
  case Am_STRING:
  case Am_DOUBLE:
    return (value.wrapper_value != test_value) &&
           (!test_value || !value.wrapper_value ||
            !(*value.wrapper_value == *test_value));
  case Am_INT:
  case Am_LONG:
  case Am_VOIDPTR:
    if (!value.voidptr_value)
      return test_value != (0L);
  default:
    return true;
  }
}

bool
Am_Value::operator!=(Am_Ptr test_value) const
{
  return value.voidptr_value != test_value;
}

bool
Am_Value::operator!=(int test_value) const
{
  switch (type) {
  case Am_INT:
  case Am_LONG:
  case Am_ENUM:
    return value.long_value != test_value;
  case Am_FLOAT:
    return value.float_value != test_value;
  case Am_DOUBLE:
    return (double)*(Am_Double_Data *)value.wrapper_value != test_value;
  default:
    return true;
  }
}

bool
Am_Value::operator!=(long test_value) const
{
  switch (type) {
  case Am_LONG:
  case Am_INT:
  case Am_ENUM:
    return value.long_value != test_value;
  case Am_FLOAT:
    return value.float_value != test_value;
  case Am_DOUBLE:
    return (double)*(Am_Double_Data *)value.wrapper_value != test_value;
  default:
    return true;
  }
}

bool
Am_Value::operator!=(bool test_value) const
{
  if (Am_Type_Class(type) != Am_NONE)
    return !!value.voidptr_value != test_value;
  switch (type) {
  case Am_INT:
  case Am_LONG:
    return !!value.long_value != test_value;
  case Am_BOOL:
    return value.bool_value != test_value;
  case Am_FLOAT:
    return !!value.float_value != test_value;
  case Am_DOUBLE:
    return !!(double)*(Am_Double_Data *)value.wrapper_value != test_value;
  case Am_CHAR:
    return !!value.char_value != test_value;
  case Am_STRING:
    return !!(const char *)*(Am_String_Data *)value.wrapper_value != test_value;
  case Am_VOIDPTR:
    return !!value.voidptr_value != test_value;
  case Am_PROC:
    return !!value.proc_value != test_value;
  default:
    return true;
  }
}

bool
Am_Value::operator!=(float test_value) const
{
  switch (type) {
  case Am_FLOAT:
    return value.float_value != test_value;
  case Am_DOUBLE:
    return (double)*(Am_Double_Data *)value.wrapper_value != test_value;
  case Am_INT:
  case Am_LONG:
    return value.long_value != test_value;
  default:
    return true;
  }
}

bool
Am_Value::operator!=(double test_value) const
{
  switch (type) {
  case Am_FLOAT:
    return value.float_value != test_value;
  case Am_DOUBLE:
    return (double)*(Am_Double_Data *)value.wrapper_value != test_value;
  case Am_INT:
  case Am_LONG:
    return value.long_value != test_value;
  default:
    return true;
  }
}

bool
Am_Value::operator!=(char test_value) const
{
  if (type == Am_CHAR)
    return value.char_value != test_value;
  else
    return true;
}

bool
Am_Value::operator!=(const char *test_value) const
{
  switch (type) {
  case Am_STRING:
    return !(*(Am_String_Data *)value.wrapper_value == test_value);
  case Am_INT:
  case Am_LONG:
  case Am_VOIDPTR:
    if (!value.voidptr_value)
      return test_value != (0L);
  default:
    return true;
  }
}

bool
Am_Value::operator!=(const Am_String &test_value) const
{
  switch (type) {
  case Am_STRING:
    return !(*(Am_String_Data *)value.wrapper_value == test_value);
  case Am_INT:
  case Am_LONG:
  case Am_VOIDPTR:
    return value.voidptr_value || (const char *)test_value;
  default:
    return true;
  }
}

bool
Am_Value::operator!=(Am_Generic_Procedure *test_value) const
{
  if (Am_Type_Class(type) == Am_METHOD)
    return value.voidptr_value || test_value;
  switch (type) {
  case Am_PROC:
    return value.proc_value != test_value;
  case Am_INT:
  case Am_LONG:
  case Am_VOIDPTR:
    return value.voidptr_value || test_value;
  default:
    return true;
  }
}

bool
Am_Value::operator!=(Am_Method_Wrapper *test_value) const
{
  if (Am_Type_Class(type) == Am_METHOD)
    return value.method_value != test_value;
  switch (type) {
  case Am_PROC:
  case Am_INT:
  case Am_LONG:
  case Am_VOIDPTR:
    return value.voidptr_value || test_value;
  default:
    return true;
  }
}

bool
Am_Value::operator!=(const Am_Value &test_value) const
{
  return type != test_value.type ||
         Am_Type_Class(type) == Am_ERROR_VALUE_TYPE ||
         (Am_Type_Is_Ref_Counted(type) &&
          value.wrapper_value != test_value.value.wrapper_value &&
          (!value.wrapper_value || !test_value.value.wrapper_value ||
           !(*value.wrapper_value == *test_value.value.wrapper_value))) ||
         (!Am_Type_Is_Ref_Counted(type) &&
          value.voidptr_value != test_value.value.voidptr_value);
}

bool
Am_Value::Valid() const
{
  switch (Am_Type_Class(type)) {
  case Am_ERROR_VALUE_TYPE:
    return false;
  case Am_WRAPPER:
  case Am_METHOD:
  case Am_ENUM:
    return value.voidptr_value != (0L);
  case Am_SIMPLE_TYPE:
    switch (type) {
    case Am_STRING:
    case Am_VOIDPTR:
      return value.voidptr_value != (0L);
    case Am_INT:
    case Am_LONG:
      return value.long_value != 0;
    case Am_BOOL:
      return value.bool_value != false;
    case Am_FLOAT:
      return value.float_value != 0.0;
    case Am_DOUBLE:
      return *(Am_Double_Data *)value.wrapper_value != 0.0;
    case Am_CHAR:
      return value.char_value != 0;
    case Am_PROC:
      return value.proc_value != (0L);
    default:
      return false;
    }
  default:
    return false;
  }
}

//returns true for any value, and zero value
//returns false for none and the error values
bool
Am_Value::Exists() const
{
  switch (Am_Type_Class(type)) {
  case Am_ERROR_VALUE_TYPE:
    return false;
  case Am_WRAPPER:
  case Am_ENUM:
    return true;
  case Am_SIMPLE_TYPE:
    switch (type) {
    case Am_NONE:
      return false;
    default:
      return true;
    }
  default:
    return true;
  }
}

//returns true for any value, zero, and none
//returns false only for the error values
bool
Am_Value::Safe() const
{
  return (Am_Type_Class(type) != Am_ERROR_VALUE_TYPE);
}

void
Am_Value::Set_Empty()
{
  if (Am_Type_Is_Ref_Counted(type) && value.wrapper_value)
    value.wrapper_value->Release();
  value.voidptr_value = (0L);
  type = Am_NONE;
}

void
Am_Value::Set_Value_Type(Am_Value_Type new_type)
{
  if (Am_Type_Is_Ref_Counted(type) && value.wrapper_value)
    value.wrapper_value->Release();
  value.voidptr_value = (0L);
  type = new_type;
}

Am_Value &
Am_Value::operator=(Am_Wrapper *in_value)
{
  if (Am_Type_Is_Ref_Counted(type) && value.wrapper_value)
    value.wrapper_value->Release();
  value.wrapper_value = in_value;
  if (in_value)
    type = in_value->ID();
  else
    type = Am_WRAPPER_TYPE;
  return *this;
}

Am_Value &
Am_Value::operator=(Am_Ptr in_value)
{
  if (Am_Type_Is_Ref_Counted(type) && value.wrapper_value)
    value.wrapper_value->Release();
  type = Am_VOIDPTR;
  value.voidptr_value = in_value;
  return *this;
}

Am_Value &
Am_Value::operator=(int in_value)
{
  if (Am_Type_Is_Ref_Counted(type) && value.wrapper_value)
    value.wrapper_value->Release();
  type = Am_INT;
  value.long_value = in_value;
  return *this;
}

Am_Value &
Am_Value::operator=(long in_value)
{
  if (Am_Type_Is_Ref_Counted(type) && value.wrapper_value)
    value.wrapper_value->Release();
  type = Am_LONG;
  value.long_value = in_value;
  return *this;
}

Am_Value &
Am_Value::operator=(bool in_value)
{
  if (Am_Type_Is_Ref_Counted(type) && value.wrapper_value)
    value.wrapper_value->Release();
  type = Am_BOOL;
  value.bool_value = in_value;
  return *this;
}

Am_Value &
Am_Value::operator=(float in_value)
{
  if (Am_Type_Is_Ref_Counted(type) && value.wrapper_value)
    value.wrapper_value->Release();
  type = Am_FLOAT;
  value.float_value = in_value;
  return *this;
}

Am_Value &
Am_Value::operator=(double in_value)
{
  //if ((type == Am_DOUBLE) &&
  //    ((double)*(Am_Double_Data*)value.wrapper_value == in_value))
  //  return *this;
  if (Am_Type_Is_Ref_Counted(type) && value.wrapper_value)
    value.wrapper_value->Release();
  type = Am_DOUBLE;
  value.wrapper_value = new Am_Double_Data(in_value);
  return *this;
}

Am_Value &
Am_Value::operator=(char in_value)
{
  if (Am_Type_Is_Ref_Counted(type) && value.wrapper_value)
    value.wrapper_value->Release();
  type = Am_CHAR;
  value.long_value = 0; // clear value first!!!
  value.char_value = in_value;
  return *this;
}

Am_Value &
Am_Value::operator=(const char *in_value)
{
  if (Am_Type_Is_Ref_Counted(type) && value.wrapper_value)
    value.wrapper_value->Release();
  type = Am_STRING;
  value.wrapper_value = new Am_String_Data(in_value);
  return *this;
}

Am_Value &
Am_Value::operator=(const Am_String &in_value)
{
  if (Am_Type_Is_Ref_Counted(type) && value.wrapper_value)
    value.wrapper_value->Release();
  type = Am_STRING;
  value.wrapper_value = in_value;
  return *this;
}

Am_Value &
Am_Value::operator=(Am_Generic_Procedure *in_value)
{
  if (Am_Type_Is_Ref_Counted(type) && value.wrapper_value)
    value.wrapper_value->Release();
  type = Am_PROC;
  value.proc_value = in_value;
  return *this;
}

Am_Value &
Am_Value::operator=(Am_Method_Wrapper *in_value)
{
  if (Am_Type_Is_Ref_Counted(type) && value.wrapper_value)
    value.wrapper_value->Release();
  if (in_value)
    type = (Am_Value_Type)in_value->ID();
  else
    type = Am_METHOD_TYPE;
  value.method_value = in_value;
  return *this;
}

Am_Value &
Am_Value::operator=(const Am_Value &in_value)
{
  if (Am_Type_Is_Ref_Counted(type) && value.wrapper_value)
    value.wrapper_value->Release();
  type = in_value.type;
  value = in_value.value;
  if (Am_Type_Is_Ref_Counted(type) && value.wrapper_value)
    value.wrapper_value->Note_Reference();
  return *this;
}

void
Am_Value::Print(std::ostream &os) const
{
  switch (Am_Type_Class(type)) {
  case Am_SIMPLE_TYPE:
    switch (type) {
    case Am_INT:
    case Am_LONG:
      os << value.long_value;
      break;
    case Am_BOOL:
      os << value.bool_value;
      break;
    case Am_FLOAT:
      os << value.float_value;
      break;
    case Am_DOUBLE:
      os << *(Am_Double_Data *)value.wrapper_value;
      break;
    case Am_CHAR:
      os << value.char_value;
      break;
    case Am_STRING:
      if (value.wrapper_value)
        os << (const char *)*(Am_String_Data *)value.wrapper_value;
      else
        os << "((0L))";
      break;
    case Am_VOIDPTR:
      // explicitly cast, for the case when Am_Ptr == char*
      os << "(void*) " << std::hex << (long)value.voidptr_value << std::dec;
      break;
    case Am_PROC:
      os << std::hex << (long)value.proc_value << std::dec;
      break;
    case Am_NONE:
      os << "NONE (No value)";
      break;
    case Am_ZERO:
      os << "ZERO VALUE";
      break;
    default:
      std::cerr << "** This value cannot be printed, type = ";
      Am_Print_Type(std::cerr, type);
      std::cerr << "." << std::endl;
      Am_Error();
      break;
    }
    break;
  case Am_WRAPPER:
    if (value.wrapper_value) // complains about const with call to valid
      value.wrapper_value->Print(os);
    else
      os << "((0L))";
    break;
  case Am_METHOD:
    if (value.method_value)
      value.method_value->Print(os);
    else
      os << "((0L))";
    break;
  case Am_ENUM: {
    Am_Type_Support *support = Am_Find_Support(type);
    if (support)
      support->Print(os, *this);
    else
      Am_Standard_Print(os, value.long_value, type);
    break;
  }
  default: //some other type (works for the error types)
    Am_Standard_Print(os, value.long_value, type);
    break;
  }
}

void
Am_Value::Println() const
{
  Print(std::cout);
  std::cout << std::endl;
}

const char *
Am_Value::To_String() const
{
  const char *ret_str = (0L);
  switch (Am_Type_Class(type)) {
  case Am_WRAPPER:
    if (value.wrapper_value) // complains about const with call to valid
      ret_str = value.wrapper_value->To_String();
    break;
  case Am_METHOD:
    if (value.method_value)
      ret_str = value.method_value->To_String();
    break;
  case Am_ENUM: {
    Am_Type_Support *support = Am_Find_Support(type);
    if (support)
      ret_str = support->To_String(*this);
    break;
  }
  }
  if (ret_str)
    return ret_str;
  else { //use expensive technique
    static char line[250];
    OSTRSTREAM_CONSTR(oss, line, 250, std::ios::out);
    oss << *this << std::ends;
    OSTRSTREAM_COPY(oss, line, 250);
    return line;
  }
}

//******* Should parse the standard types. ****
Am_Value
Am_Value::From_String(const char *string, Am_ID_Tag type)
{
  const Am_Registered_Type *item = Am_Get_Named_Item(string);
  if (item) {
    if (Am_Type_Class(item->ID()) == Am_WRAPPER) {
      Am_Wrapper *wrapper = (Am_Wrapper *)item;
      wrapper->Note_Reference();
      Am_Value v(wrapper);
      return v;
    } else {
      Am_Value v((Am_Method_Wrapper *)item);
      return v;
    }
  }
  //else not registered by name
  Am_Type_Support *support = Am_Find_Support(type);
  if (support)
    return support->From_String(string);
  return Am_No_Value;
}

//not very efficient, mostly for debugging
const char *
Am_To_String(const Am_Value &value)
{
  return value.To_String();
}
