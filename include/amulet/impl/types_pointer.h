#include "types_enum.h"

//////////////////////////////////////
// Pointer Wrapper Macros -- for easily putting pointers to external objects
// into Amulet objects, with no reference counting or memory management.
//  If you need memory management, use "real" wrappers, below.
//  Note: Type_name is the class itself, not the pointer to the class
//////////////////////////////////////

#define Am_Define_Pointer_Wrapper(Type_name)                                   \
  class _OA_DL_CLASSIMPORT Am_##Type_name                                      \
  {                                                                            \
  public:                                                                      \
    void TypeError(const Am_Value &in_value);                                  \
    Am_##Type_name(const Am_Value &in_value)                                   \
    {                                                                          \
      if (in_value.type != Am_ZERO && in_value.type != Am_##Type_name##_ID) {  \
        TypeError(in_value);                                                   \
      }                                                                        \
      value = (Type_name *)in_value.value.voidptr_value;                       \
    }                                                                          \
    Am_##Type_name(Type_name *in_value = (0L)) { value = in_value; }           \
    Am_##Type_name &operator=(Type_name *in_value)                             \
    {                                                                          \
      value = in_value;                                                        \
      return *this;                                                            \
    }                                                                          \
    Am_##Type_name &operator=(const Am_Value &in_value)                        \
    {                                                                          \
      if (in_value.type != Am_ZERO && in_value.type != Am_##Type_name##_ID) {  \
        TypeError(in_value);                                                   \
      }                                                                        \
      value = (Type_name *)in_value.value.voidptr_value;                       \
      return *this;                                                            \
    }                                                                          \
    operator Am_Value() const                                                  \
    {                                                                          \
      return Am_Value((long)value, Am_##Type_name##_ID);                       \
    }                                                                          \
    bool operator==(Type_name *test_value) const                               \
    {                                                                          \
      return value == test_value;                                              \
    }                                                                          \
    bool operator==(Am_##Type_name test_value) const                           \
    {                                                                          \
      return value == (Type_name *)test_value.value;                           \
    }                                                                          \
    bool operator!=(Type_name *test_value) const                               \
    {                                                                          \
      return value != test_value;                                              \
    }                                                                          \
    bool operator!=(Am_##Type_name test_value) const                           \
    {                                                                          \
      return value != (Type_name *)test_value.value;                           \
    }                                                                          \
    static bool Test(const Am_Value &value)                                    \
    {                                                                          \
      return value.type == Am_##Type_name##_ID;                                \
    }                                                                          \
    static Am_Value_Type Type_ID() { return Am_##Type_name##_ID; }             \
    /* Printing functions */                                                   \
    const char *To_String() const                                              \
    {                                                                          \
      return Am_Enum_To_String_Helper(Am_##Type_name##_ID, (long)value);       \
    }                                                                          \
    void Print(std::ostream &out) const                                        \
    {                                                                          \
      Am_Enum_Print_Helper(out, Am_##Type_name##_ID, (long)value);             \
    }                                                                          \
    void Println() const;                                                      \
    Type_name *value;                                                          \
                                                                               \
  private:                                                                     \
    _OA_DL_MEMBERIMPORT static Am_ID_Tag Am_##Type_name##_ID;                  \
  };                                                                           \
  _OA_DL_IMPORT extern std::ostream &operator<<(std::ostream &os,              \
                                                const Am_##Type_name &value);

#define Am_Define_Pointer_Wrapper_Impl(Type_name, Type_Support_Ptr)            \
  Am_ID_Tag Am_##Type_name::Am_##Type_name##_ID = Am_Get_Unique_ID_Tag(        \
      DSTR("Am_" #Type_name), Type_Support_Ptr, Am_ENUM_TYPE);                 \
  std::ostream &operator<<(std::ostream &os, const Am_##Type_name &item)       \
  {                                                                            \
    item.Print(os);                                                            \
    return os;                                                                 \
  }                                                                            \
  void Am_##Type_name::Println() const                                         \
  {                                                                            \
    Print(std::cout);                                                          \
    std::cout << std::endl << std::flush;                                      \
  }                                                                            \
  void Am_##Type_name::TypeError(const Am_Value &in_value)                     \
  {                                                                            \
    DERR("** Tried to set a Am_" #Type_name " pointer wrapper "                \
         "with a non Am_" #Type_name " wrapper: "                              \
         << in_value << std::endl);                                            \
    Am_Error();                                                                \
  }
