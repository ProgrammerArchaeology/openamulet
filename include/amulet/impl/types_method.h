#ifndef TYPES_METHODS_H
#define TYPES_METHODS_H

#include "types_registered.h"
// class std::ostream;
/////////////////////////////////////////////////////////////////////////////
// Wrappers for methods
/////////////////////////////////////////////////////////////////////////////

class _OA_DL_CLASSIMPORT Am_Method_Wrapper : public Am_Registered_Type
{
public:
  // constructor registers method's name in name registry.
  Am_Method_Wrapper(Am_ID_Tag *id_ptr, Am_Generic_Procedure *p,
                    const char *name = nullptr);
  Am_ID_Tag ID() const override { return *ID_Ptr; }
  Am_Generic_Procedure *Call;
  void Print(std::ostream &out) const override;
  virtual Am_Value From_String(const char *string) const;

protected:
  Am_ID_Tag *ID_Ptr;
};

/////////  TOP LEVEL MACRO FOR DEFINING NEW TYPES OF METHODS
/// args must be a parenthesized list of the parameter names and types.

#define AM_DEFINE_METHOD_TYPE_IMPL(Type_name)                                  \
  Am_ID_Tag Type_name::Type_name##_ID =                                        \
      Am_Get_Unique_ID_Tag(#Type_name, Am_METHOD_TYPE);                        \
  _OA_DL_IMPORT std::ostream &operator<<(std::ostream &os,                     \
                                         const Type_name &method)              \
  {                                                                            \
    method.Print(os);                                                          \
    return os;                                                                 \
  }                                                                            \
  void Type_name::TypeError(const Am_Value &value)                             \
  {                                                                            \
    DERR("** Tried to assign a " #Type_name " method with a non " #Type_name   \
         " wrapper: "                                                          \
         << value << std::endl);                                               \
    Am_Error();                                                                \
  }

//This is assigned into the Call slot when there is no procedure pointer.
//If debugging, and get a null method, it will print out an error
//message.  If not debugging, and de-reference the 0, will get a
//"Illegal instruction" error at run time.  The
//Am_Null_Method_Error_Function is defined with ... so it can take any params.
#ifdef DEBUG
extern Am_Any_Procedure *Am_Null_Method_Error_Function;
#else
#define Am_Null_Method_Error_Function 0
#endif

#define AM_DEFINE_METHOD_TYPE(Type_name, Return_type, Args)                                                                                  \
  /* now, a typedef for the procedure type */                                                                                                \
  typedef Return_type Type_name##_Type Args;                                                                                                 \
  class _OA_DL_CLASSIMPORT Type_name                                                                                                         \
  {                                                                                                                                          \
  public:                                                                                                                                    \
    /* Variables: */                                                                                                                         \
    /*   the wrapper I was created from  */                                                                                                  \
    Am_Method_Wrapper *from_wrapper;                                                                                                         \
    /*   a pointer to the procedure of the appropriate type */                                                                               \
    Type_name##_Type *Call;                                                                                                                  \
    /* allocate an ID for this type       called Type_name_ID*/                                                                              \
    _OA_DL_MEMBERIMPORT static Am_ID_Tag Type_name##_ID;                                                                                     \
    bool Valid() const                                                                                                                       \
    {                                                                                                                                        \
      return Call != (Type_name##_Type *)Am_Null_Method_Error_Function;                                                                      \
    }                                                                                                                                        \
    /* method to see if a procedure is my type (and valid) */                                                                                \
    static int Test(Am_Method_Wrapper *wrapper)                                                                                              \
    {                                                                                                                                        \
      return wrapper && wrapper->ID() == Type_name::Type_name##_ID &&                                                                        \
             wrapper->Call !=                                                                                                                \
                 (Am_Generic_Procedure *)Am_Null_Method_Error_Function;                                                                      \
    }                                                                                                                                        \
    static int Test(const Am_Value &value)                                                                                                   \
    {                                                                                                                                        \
      return value.value.method_value &&                                                                                                     \
             value.type == Type_name::Type_name##_ID &&                                                                                      \
             value.value.method_value->Call !=                                                                                               \
                 (Am_Generic_Procedure *)Am_Null_Method_Error_Function;                                                                      \
    }                                                                                                                                        \
    static Am_Value_Type Type_ID() { return Type_name##_ID; }                                                                                \
    /* empty constructor */                                                                                                                  \
    Type_name()                                                                                                                              \
    {                                                                                                                                        \
      Call = (Type_name##_Type *)Am_Null_Method_Error_Function;                                                                              \
      from_wrapper = 0;                                                                                                                      \
    }                                                                                                                                        \
    /* constructor of this class from a wrapper */                                                                                           \
    Type_name(Am_Method_Wrapper *wrapper)                                                                                                    \
    {                                                                                                                                        \
      from_wrapper = wrapper;                                                                                                                \
      if (from_wrapper)                                                                                                                      \
        Call = (Type_name##_Type *)from_wrapper->Call;                                                                                       \
      else                                                                                                                                   \
        Call = (Type_name##_Type *)Am_Null_Method_Error_Function;                                                                            \
    }                                                                                                                                        \
    /* constructor of this class from a wrapper, with the                      \
     procedure pointer passed for type-checking purposes */ \
    Type_name(Am_Method_Wrapper *wrapper, Type_name##_Type *)                                                                                \
    {                                                                                                                                        \
      from_wrapper = wrapper;                                                                                                                \
      if (from_wrapper)                                                                                                                      \
        Call = (Type_name##_Type *)from_wrapper->Call;                                                                                       \
      else                                                                                                                                   \
        Call = (Type_name##_Type *)Am_Null_Method_Error_Function;                                                                            \
    }                                                                                                                                        \
    void TypeError(const Am_Value &value);                                                                                                   \
    Type_name(const Am_Value &value)                                                                                                         \
    {                                                                                                                                        \
      from_wrapper = value;                                                                                                                  \
      if (from_wrapper) {                                                                                                                    \
        if (value.type != Type_name::Type_name##_ID) {                                                                                       \
          TypeError(value);                                                                                                                  \
        }                                                                                                                                    \
        Call = (Type_name##_Type *)from_wrapper->Call;                                                                                       \
      } else                                                                                                                                 \
        Call = (Type_name##_Type *)Am_Null_Method_Error_Function;                                                                            \
    }                                                                                                                                        \
    Type_name &operator=(Am_Method_Wrapper *wrapper)                                                                                         \
    {                                                                                                                                        \
      from_wrapper = wrapper;                                                                                                                \
      if (from_wrapper)                                                                                                                      \
        Call = (Type_name##_Type *)from_wrapper->Call;                                                                                       \
      else                                                                                                                                   \
        Call = (Type_name##_Type *)Am_Null_Method_Error_Function;                                                                            \
      return *this;                                                                                                                          \
    }                                                                                                                                        \
    Type_name &operator=(const Am_Value &value)                                                                                              \
    {                                                                                                                                        \
      from_wrapper = value;                                                                                                                  \
      if (from_wrapper) {                                                                                                                    \
        if (value.type != Type_name::Type_name##_ID) {                                                                                       \
          TypeError(value);                                                                                                                  \
        }                                                                                                                                    \
        Call = (Type_name##_Type *)from_wrapper->Call;                                                                                       \
      } else                                                                                                                                 \
        Call = (Type_name##_Type *)Am_Null_Method_Error_Function;                                                                            \
      return *this;                                                                                                                          \
    }                                                                                                                                        \
    /* convert me into a Am_Method_Wrapper so I can be stored into a slot */                                                                 \
    operator Am_Method_Wrapper *() const { return from_wrapper; }                                                                            \
    /* Printing functions */                                                                                                                 \
    const char *To_String() const { return from_wrapper->To_String(); }                                                                      \
    void Print(std::ostream &out) const { from_wrapper->Print(out); }                                                                        \
  };                                                                                                                                         \
  _OA_DL_IMPORT extern std::ostream &operator<<(std::ostream &os,                                                                            \
                                                const Type_name &method);

/////////  MACRO FOR DEFINING NEW METHODS OF A PRE_DEFINED TYPE
/// return_type and args must be the same as used for defining Type_name

#define Am_Define_Method(Type_name, Return_type, Method, Args)                                                                                                                                                                                                                                  \
  /* declare the procedure so can make a pointer to it */                                                                                                                                                                                                                                       \
  static Return_type Method##_proc Args;                                                                                                                                                                                                                                                        \
  /* allocate a wrapper object holding the procedure */                                                                                                                                                                                                                                         \
  Am_Method_Wrapper Method##_inst = Am_Method_Wrapper(                                                                                                                                                                                                                                          \
      &Type_name::Type_name##_ID, (Am_Generic_Procedure *)&Method##_proc,                                                                                                                                                                                                                       \
      DSTR(#Method));                                                                                                                                                                                                                                                                           \
  /* Create a Type_name class object for the procedure, and call it Method    \
   (passing the wrapper object is sufficient for construction, but we       \
   also pass the procedure pointer to type-check its signature against      \
   the signature expected of Type_name methods). */ \
  Type_name Method(&Method##_inst, Method##_proc);                                                                                                                                                                                                                                              \
  /* now the procedure header, so this can be followed by the code */                                                                                                                                                                                                                           \
  Return_type Method##_proc Args
/* code goes here { ... }  */

#define Am_Define_Method_No_Static(Type_name, Return_type, Method, Args)                                                                                                                                                                                                                        \
  /* declare the procedure so can make a pointer to it */                                                                                                                                                                                                                                       \
  Return_type Method##_proc Args;                                                                                                                                                                                                                                                               \
  /* allocate a wrapper object holding the procedure */                                                                                                                                                                                                                                         \
  Am_Method_Wrapper Method##_inst = Am_Method_Wrapper(                                                                                                                                                                                                                                          \
      &Type_name::Type_name##_ID, (Am_Generic_Procedure *)&Method##_proc,                                                                                                                                                                                                                       \
      DSTR(#Method));                                                                                                                                                                                                                                                                           \
  /* Create a Type_name class object for the procedure, and call it Method    \
   (passing the wrapper object is sufficient for construction, but we       \
   also pass the procedure pointer to type-check its signature against      \
   the signature expected of Type_name methods). */ \
  Type_name Method(&Method##_inst, Method##_proc);                                                                                                                                                                                                                                              \
  /* now the procedure header, so this can be followed by the code */                                                                                                                                                                                                                           \
  Return_type Method##_proc Args
/* code goes here { ... }  */

#endif //TYPES_METHODS_H
