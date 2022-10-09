#ifndef TYPES_WRAPPER_TYPES_H
#define TYPES_WRAPPER_TYPES_H

#include <am_inc.h>
#include <amulet/am_value.hpp>
#include "types_registered.h"

// The Am_Wrapper type.  Classes derived from this type can be stored in an
// object slot directly.
class _OA_DL_CLASSIMPORT Am_Wrapper : public Am_Registered_Type
{
public:
  Am_Wrapper() : refs(1) { ; }

  Am_Wrapper(const Am_Wrapper &) : refs(1) { ; }

  ~Am_Wrapper() override {}

  // Note that wrapper is being stored
  void Note_Reference() { ++refs; }

  // Return the reference count.
  unsigned Ref_Count() { return refs; }

  // Indicates data is no longer being used.
  void Release()
  {
    if (!--refs)
      delete this;
  }

  bool Is_Unique() { return (refs == 1); }

  bool Is_Zero() { return refs == 0; }

  virtual Am_Wrapper *Make_Unique() = 0; // Return a unique copy of the data.
  virtual bool operator==(const Am_Wrapper &test_value) const = 0; // Equality test.
  Am_ID_Tag ID() const override = 0; // Returns unique tag
  // for each derived type.
  virtual Am_Value From_String(const char *string) const;

private:
  unsigned refs;
};

//////////////////////////////////////
// Definition of the Wrapper Macros //
//////////////////////////////////////

#define AM_WRAPPER_DATA_DECL(Type_name)                                        \
public:                                                                        \
  Am_Wrapper *Make_Unique();                                                   \
  bool operator==(const Am_Wrapper &test_value) const;                         \
  Am_ID_Tag ID() const { return id; }                                          \
  static Type_name##_Data *Narrow(Am_Wrapper *value);                          \
  static Am_ID_Tag Type_name##_Data_ID() { return id; }                        \
private:                                                                       \
  _OA_DL_MEMBERIMPORT static Am_ID_Tag id;

#define AM_WRAPPER_DATA_IMPL_NO_ID(Type_name, create_args)                     \
  Type_name##_Data *Type_name##_Data::Narrow(Am_Wrapper *value)                \
  {                                                                            \
    if (value && (value->ID() == id))                                          \
      return (Type_name##_Data *)value;                                        \
    else                                                                       \
      return nullptr;                                                             \
  }                                                                            \
  bool Type_name##_Data::operator==(const Am_Wrapper &test_value) const        \
  {                                                                            \
    if (id == test_value.ID()) {                                               \
      return (static_cast<const Am_Wrapper *>(&test_value) ==                  \
                  static_cast<const Am_Wrapper *>(this) ||                     \
              (static_cast<const Type_name##_Data &>(test_value) == (*this))); \
    } else                                                                     \
      return false;                                                            \
  }                                                                            \
  Am_Wrapper *Type_name##_Data::Make_Unique()                                  \
  {                                                                            \
    if (Is_Unique())                                                           \
      return this;                                                             \
    else {                                                                     \
      Release();                                                               \
      return new Type_name##_Data create_args;                                 \
    }                                                                          \
  }

#define AM_WRAPPER_DATA_IMPL(Type_name, create_args)                           \
  AM_WRAPPER_DATA_IMPL_NO_ID(Type_name, create_args)                           \
  Am_ID_Tag Type_name##_Data::id =                                             \
      Am_Get_Unique_ID_Tag(DSTR(#Type_name), Am_WRAPPER_TYPE);

#define AM_WRAPPER_DATA_IMPL_ID(Type_name, create_args, in_id)                 \
  AM_WRAPPER_DATA_IMPL_NO_ID(Type_name, create_args)                           \
  Am_ID_Tag Type_name##_Data::id =                                             \
      Am_Get_Unique_ID_Tag(DSTR(#Type_name), in_id);

#define AM_WRAPPER_DECL(Type_name)                                             \
public:                                                                        \
  Type_name(const Type_name &);                                                \
  Type_name(const Am_Value &);                                                 \
  Type_name(Type_name##_Data *in_data) { data = in_data; }                     \
  ~Type_name();                                                                \
  Type_name &operator=(const Type_name &);                                     \
  Type_name &operator=(const Am_Value &);                                      \
  Type_name &operator=(Type_name##_Data *in_data);                             \
  _OA_DL_MEMBERIMPORT operator Am_Wrapper *() const;                           \
  _OA_DL_MEMBERIMPORT bool Valid() const;                                      \
  static Type_name Narrow(Am_Wrapper *);                                       \
  static bool Test(const Am_Wrapper *);                                        \
  static bool Test(const Am_Value &in_value);                                  \
  static Am_Value_Type Type_ID();                                              \
  _OA_DL_MEMBERIMPORT const char *To_String() const;                           \
  Am_Value From_String(const char *string);                                    \
  _OA_DL_MEMBERIMPORT void Print(std::ostream &out) const;                     \
  _OA_DL_MEMBERIMPORT void Println() const;                                    \
  Type_name##_Data *data;

#define AM_WRAPPER_IMPL(Type_name)                                             \
  Type_name::Type_name(const Type_name &prev)                                  \
  {                                                                            \
    data = prev.data;                                                          \
    if (data)                                                                  \
      data->Note_Reference();                                                  \
  }                                                                            \
  Type_name::Type_name(const Am_Value &in_value)                               \
  {                                                                            \
    data = (Type_name##_Data *)in_value.value.wrapper_value;                   \
    if (data) {                                                                \
      if (Type_name##_Data::Type_name##_Data_ID() != in_value.type) {          \
        DERR("** Tried to set a " #Type_name " with a non " #Type_name         \
             " wrapper: "                                                      \
             << in_value << std::endl);                                        \
        Am_Error();                                                            \
      }                                                                        \
      data->Note_Reference();                                                  \
    }                                                                          \
  }                                                                            \
  Type_name::~Type_name()                                                      \
  {                                                                            \
    if (data) {                                                                \
      if (data->Is_Zero())                                                     \
        Am_DERR("** Tried to delete a " #Type_name " twice.");                 \
      data->Release();                                                         \
    }                                                                          \
    data = nullptr;                                                               \
  }                                                                            \
  Type_name &Type_name::operator=(const Type_name &prev)                       \
  {                                                                            \
    Type_name##_Data *old_data = data;                                         \
    data = prev.data;                                                          \
    if (data)                                                                  \
      data->Note_Reference();                                                  \
    if (old_data)                                                              \
      old_data->Release();                                                     \
    return *this;                                                              \
  }                                                                            \
  Type_name &Type_name::operator=(const Am_Value &in_value)                    \
  {                                                                            \
    Type_name##_Data *old_data = data;                                         \
    data = (Type_name##_Data *)in_value.value.wrapper_value;                   \
    if (data) {                                                                \
      if (in_value.type != Type_name##_Data::Type_name##_Data_ID() &&          \
          in_value.type != Am_ZERO) {                                          \
        DERR("** Tried to set a " #Type_name " with a non " #Type_name         \
             " wrapper: "                                                      \
             << in_value << std::endl);                                        \
        Am_Error();                                                            \
      }                                                                        \
      data->Note_Reference();                                                  \
    }                                                                          \
    if (old_data)                                                              \
      old_data->Release();                                                     \
    return *this;                                                              \
  }                                                                            \
  Type_name &Type_name::operator=(Type_name##_Data *in_data)                   \
  {                                                                            \
    if (data)                                                                  \
      data->Release();                                                         \
    data = in_data;                                                            \
    return *this;                                                              \
  }                                                                            \
  Type_name::operator Am_Wrapper *() const                                     \
  {                                                                            \
    if (data)                                                                  \
      data->Note_Reference();                                                  \
    return data;                                                               \
  }                                                                            \
  bool Type_name::Valid() const { return data != nullptr; }                       \
  Type_name Type_name::Narrow(Am_Wrapper *in_data)                             \
  {                                                                            \
    if (in_data) {                                                             \
      if (Type_name##_Data::Type_name##_Data_ID() == in_data->ID())            \
        return (Type_name##_Data *)in_data;                                    \
      else                                                                     \
        Am_DERR("** Tried to set a " #Type_name " with a non " #Type_name      \
                " wrapper.");                                                  \
    }                                                                          \
    return (Type_name##_Data *)(nullptr);                                           \
  }                                                                            \
  bool Type_name::Test(const Am_Wrapper *in_data)                              \
  {                                                                            \
    return (in_data &&                                                         \
            (in_data->ID() == Type_name##_Data::Type_name##_Data_ID()));       \
  }                                                                            \
  bool Type_name::Test(const Am_Value &in_value)                               \
  {                                                                            \
    return (in_value.value.wrapper_value &&                                    \
            (in_value.type == Type_name##_Data::Type_name##_Data_ID()));       \
  }                                                                            \
  Am_Value_Type Type_name::Type_ID()                                           \
  {                                                                            \
    return Type_name##_Data::Type_name##_Data_ID();                            \
  }                                                                            \
  const char *Type_name::To_String() const                                     \
  {                                                                            \
    if (data)                                                                  \
      return data->To_String();                                                \
    else                                                                       \
      return nullptr;                                                             \
  }                                                                            \
  Am_Value Type_name::From_String(const char *string)                          \
  {                                                                            \
    if (data)                                                                  \
      return data->From_String(string);                                        \
    else                                                                       \
      return Am_No_Value;                                                      \
  }                                                                            \
  void Type_name::Print(std::ostream &out) const                               \
  {                                                                            \
    if (data)                                                                  \
      data->Print(out);                                                        \
    else                                                                       \
      out << "(" #Type_name ")(nullptr)";                                           \
  }                                                                            \
  void Type_name::Println() const                                              \
  {                                                                            \
    Print(std::cout);                                                          \
    std::cout << std::endl << std::flush;                                      \
  }

#endif //TYPES_WRAPPER_TYPES_H
