#ifndef TYPES_ENUM_TYPES_H
#define TYPES_ENUM_TYPES_H

#include "types_type_support.h"

/////////////////////////////////////////
// Creating and registering Enum types //
/////////////////////////////////////////

extern const char *Am_Enum_To_String_Helper(Am_ID_Tag type, long value);
extern void Am_Enum_Print_Helper(std::ostream &out, Am_ID_Tag type, long value);

//lint -emacro(1925,AM_DEFINE_ENUM_TYPE) value is OK to be public
//lint -emacro(1721,AM_DEFINE_ENUM_TYPE) operator=(am_value&) is OK not to be a assignment operator
#define AM_DEFINE_ENUM_TYPE(Type_name, Enum_name)                              \
  class _OA_DL_CLASSIMPORT Type_name                                           \
  {                                                                            \
  public:                                                                      \
    void TypeError(const Am_Value &in_value);                                  \
    Type_name(const Am_Value &in_value) : value(static_cast<Enum_name>(0))     \
    {                                                                          \
      if (in_value.type != Am_ZERO && in_value.type != Type_name##_ID) {       \
        TypeError(in_value);                                                   \
      }                                                                        \
      value = static_cast<Enum_name>(in_value.value.long_value);               \
    }                                                                          \
    Type_name(Enum_name in_value) : value(in_value) { ; }                      \
    Type_name() : value(static_cast<Enum_name>(0)) { ; }                       \
    Type_name &operator=(Enum_name in_value)                                   \
    {                                                                          \
      value = in_value;                                                        \
      return *this;                                                            \
    }                                                                          \
    Type_name &operator=(const Am_Value &in_value)                             \
    {                                                                          \
      if (in_value.type != Am_ZERO && in_value.type != Type_name##_ID) {       \
        TypeError(in_value);                                                   \
      }                                                                        \
      value = static_cast<Enum_name>(in_value.value.long_value);               \
      return *this;                                                            \
    }                                                                          \
    operator Am_Value() const                                                  \
    {                                                                          \
      return Am_Value((long)value, Type_name##_ID);                            \
    }                                                                          \
    bool operator==(const Enum_name &test_value) const                         \
    {                                                                          \
      return value == test_value;                                              \
    }                                                                          \
    bool operator==(const Type_name &test_value) const                         \
    {                                                                          \
      return value == test_value.value;                                        \
    }                                                                          \
    bool operator!=(const Enum_name &test_value) const                         \
    {                                                                          \
      return value != test_value;                                              \
    }                                                                          \
    bool operator!=(const Type_name &test_value) const                         \
    {                                                                          \
      return value != test_value.value;                                        \
    }                                                                          \
    static bool Test(const Am_Value &value)                                    \
    {                                                                          \
      return value.type == Type_name##_ID;                                     \
    }                                                                          \
    static Am_Value_Type Type_ID() { return Type_name##_ID; }                  \
    /* Printing functions */                                                   \
    const char *To_String() const                                              \
    {                                                                          \
      return Am_Enum_To_String_Helper(Type_name##_ID, (long)value);            \
    }                                                                          \
    void Print(std::ostream &out) const;                                       \
    void Println() const;                                                      \
    Enum_name value;                                                           \
                                                                               \
  private:                                                                     \
    _OA_DL_MEMBERIMPORT static Am_ID_Tag Type_name##_ID;                       \
  };                                                                           \
  _OA_DL_IMPORT extern std::ostream &operator<<(std::ostream &os,              \
                                                const Type_name &value);

#define AM_DEFINE_ENUM_LONG_TYPE(Type_name)                                    \
  class _OA_DL_CLASSIMPORT Type_name                                           \
  {                                                                            \
  public:                                                                      \
    void TypeError(const Am_Value &in_value);                                  \
    Type_name(const Am_Value &in_value)                                        \
    {                                                                          \
      if (in_value.type != Am_ZERO && in_value.type != Type_name##_ID) {       \
        TypeError(in_value);                                                   \
      }                                                                        \
      value = (long)in_value.value.long_value;                                 \
    }                                                                          \
    Type_name(long in_value = 0) { value = in_value; }                         \
    Type_name &operator=(long in_value)                                        \
    {                                                                          \
      value = in_value;                                                        \
      return *this;                                                            \
    }                                                                          \
    Type_name &operator=(const Am_Value &in_value)                             \
    {                                                                          \
      if (in_value.type != Am_ZERO && in_value.type != Type_name##_ID) {       \
        TypeError(in_value);                                                   \
      }                                                                        \
      value = (long)in_value.value.long_value;                                 \
      return *this;                                                            \
    }                                                                          \
    operator Am_Value() const                                                  \
    {                                                                          \
      return Am_Value((long)value, Type_name##_ID);                            \
    }                                                                          \
    bool operator==(long test_value) const { return value == test_value; }     \
    bool operator==(Type_name test_value) const                                \
    {                                                                          \
      return value == test_value.value;                                        \
    }                                                                          \
    bool operator!=(long test_value) const { return value != test_value; }     \
    bool operator!=(Type_name test_value) const                                \
    {                                                                          \
      return value != test_value.value;                                        \
    }                                                                          \
    static bool Test(const Am_Value &value)                                    \
    {                                                                          \
      return value.type == Type_name##_ID;                                     \
    }                                                                          \
    static Am_Value_Type Type_ID() { return Type_name##_ID; }                  \
    /* Printing functions */                                                   \
    const char *To_String() const                                              \
    {                                                                          \
      return Am_Enum_To_String_Helper(Type_name##_ID, (long)value);            \
    }                                                                          \
    void Print(std::ostream &out) const;                                       \
    void Println() const;                                                      \
    long value;                                                                \
                                                                               \
  private:                                                                     \
    _OA_DL_MEMBERIMPORT static Am_ID_Tag Type_name##_ID;                       \
  };                                                                           \
  _OA_DL_IMPORT extern std::ostream &operator<<(std::ostream &os,              \
                                                const Type_name &value);

#define AM_DEFINE_ENUM_TYPE_IMPL(Type_name, Type_Support_Ptr)                  \
  Am_ID_Tag Type_name::Type_name##_ID =                                        \
      Am_Get_Unique_ID_Tag(DSTR(#Type_name), Type_Support_Ptr, Am_ENUM_TYPE);  \
  _OA_DL_IMPORT std::ostream &operator<<(std::ostream &os,                     \
                                         const Type_name &item)                \
  {                                                                            \
    item.Print(os);                                                            \
    return os;                                                                 \
  }                                                                            \
  void Type_name::Print(std::ostream &out) const                               \
  {                                                                            \
    Am_Enum_Print_Helper(out, Type_name##_ID, (long)value);                    \
  }                                                                            \
  void Type_name::Println() const                                              \
  {                                                                            \
    Print(std::cout);                                                          \
    std::cout << std::endl << std::flush;                                      \
  }                                                                            \
  void Type_name::TypeError(const Am_Value &in_value)                          \
  {                                                                            \
    DERR("** Tried to set a " #Type_name " enum with a non " #Type_name        \
         " wrapper: "                                                          \
         << in_value << std::endl);                                            \
    Am_Error();                                                                \
  }

// The special macro AM_DEFINE_ENUM_SUPPORT permits an enumeration wrapper to
// be automatically defined with a printer and reader support class.  The
// second parameter is a string which contains a list of all the constants
// in the enumeration.  The string must be in order and each constant must be
// separated by a single space (the last constant is followed by \0).  The
// enumeration is assumed to be zero-based with no skipped numbers.
class _OA_DL_CLASSIMPORT Am_Enum_Support : public Am_Type_Support
{
protected:
  const char *value_string;
  char **item;
  int number;
  Am_Value_Type type;

public:
  Am_Enum_Support(const char *value_string, Am_Value_Type type);
  ~Am_Enum_Support();
  void Set_Type(Am_Value_Type in_type) { type = in_type; }

  void Print(std::ostream &os, const Am_Value &value) const;
  const char *To_String(const Am_Value &v) const;
  virtual Am_Value From_String(const char *string) const;

  int Number() { return number; }
  Am_Value Fetch(int item);
};

#define AM_DEFINE_ENUM_SUPPORT(Type_name, Value_string)                        \
  void Type_name::Print(std::ostream &out) const                               \
  {                                                                            \
    Am_Enum_Print_Helper(out, Type_name##_ID, (long)value);                    \
  }                                                                            \
  void Type_name::Println() const                                              \
  {                                                                            \
    Print(std::cout);                                                          \
    std::cout << std::endl << std::flush;                                      \
  }                                                                            \
  Am_ID_Tag Type_name::Type_name##_ID =                                        \
      Am_Get_Unique_ID_Tag(DSTR(#Type_name), Am_ENUM_TYPE);                    \
  Am_Enum_Support Type_name##_Support(Value_string, Type_name::Type_ID());     \
  _OA_DL_IMPORT std::ostream &operator<<(std::ostream &os,                     \
                                         const Type_name &item)                \
  {                                                                            \
    item.Print(os);                                                            \
    return os;                                                                 \
  }                                                                            \
  void Type_name::TypeError(const Am_Value &in_value)                          \
  {                                                                            \
    DERR("** Tried to set a " #Type_name " enum with a non " #Type_name        \
         " wrapper: "                                                          \
         << in_value << std::endl);                                            \
    Am_Error();                                                                \
  }

#endif //TYPES_ENUM_TYPES_H
