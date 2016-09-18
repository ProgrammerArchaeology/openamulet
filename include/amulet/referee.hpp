
#ifndef __REFEREE_H
#define __REFEREE_H

#include <typeinfo>
#include <istd::ostream>

#define PROVIDE_OLD_ID_TAGS

/***************************************
 *    Base class of all Amulet types   *
 ***************************************/
class Am_Registered_Type
{
public:
  // CREATORS
  Am_Registered_Type() {}
  Am_Registered_Type(const Am_Registered_Type &o) {}
  virtual ~Am_Registered_Type() {}
// ACCESSORS
#ifndef PROVIDE_OLD_ID_TAGS
  virtual const type_info &ID() const { return (typeid(*this)); }
#else
  virtual Am_ID_Tag ID() const = 0;
#endif
  virtual void Print(stdstd::ostream &out) const;
#if 0 // Defined in source/utils/types.cpp
    {
      out << this->To_String();
      return;
    }
#endif
  void Println() const
  {
    Print(std::cout);
    // TODO: I don't like the std::flush' here. Remove ? --ortalo
    std::cout << std::endl << std::flush;
    return;
  }
  virtual const char *To_String(void) const;
#if 0 // Defined in source/utils/types.cpp
    {
      return (typeid(*this).name());
    }
#endif
};

/*************************************************
 *   Definition of the wrapping base class for   *
 * all Amulet objects.                           *
 *************************************************/
class Am_Wrap_Base : public Am_Registered_Type
{
private:
  unsigned int refs;

public:
  // CREATORS
  Am_Wrap_Base() : Am_Registered_Type(), refs(1) {}
  Am_Wrap_Base(const Am_Wrap_Base &original)
      : Am_Registered_Type(original), refs(1)
  {
  }
  virtual ~Am_Wrap_Base() {}
  // OPERATORS
  // An Am_Wrap_Base object can be tested with any other one
  virtual bool operator==(const Am_Wrap_Base &test_value) const
  {
    return false;
  }
  // MANIPULATORS
  void Note_Reference(void) // Note that wrapper is being used
  {
    ++refs;
  }
  void Release(void) // Indicates data is no longer being used.
  {
    if (refs >= 1) { // TODO: WITHOUT THIS THERE IS A SEGFAULT !!!
                     // TODO: Study in depth !!! (NB:Optimization removes pb)
      --refs;
      if (refs == 0)
        delete (this);
    }
  }
  virtual Am_Wrap_Base *Make_Unique() = 0;
  // ACCESSORS
  bool Is_Unique(void) { return (refs == 1); }
  // TODO: Deprecate this function --ortalo
  bool Is_Zero(void) { return (refs == 0); }
  unsigned int Ref_Count(void) // Return the reference count.
  {
    return refs;
  }
  // VIRTUAL
  virtual Am_Value From_String(const char *string) const;
#ifdef PROVIDE_OLD_ID_TAGS
  virtual Am_ID_Tag ID() const {}
#endif
};
typedef Am_Wrap_Base Am_Wrapper;

#if 0
/****************************************************
 *  Definition of the generic storage Am_Value type *
 ****************************************************/
class Am_Value
{
public:
  // DATA MEMBERS
  union
  {
    Am_Wrap_Base *wrapper_value;
    long       long_value;
    bool       bool_value;
    float      float_value;
    char       char_value;
  } value;
};

const Am_ID_Tag Am_WRAPPER_TYPE = 0x1234;
#endif

/**************************************************
 *  Definition of the inner template for Amulet   *
 *  (reference counted and lazily copied objects) *
 **************************************************/
template <class T> class Am_Referent : public Am_Wrap_Base
{
#ifdef PROVIDE_OLD_ID_TAGS
private:
  static const Am_ID_Tag id;

public:
  Am_ID_Tag ID() const { return id; }
  static Am_ID_Tag Type_ID() { return id; }
// You should provide yourself the 'Am_Truc_Data_ID::Am_Truc_Data_ID ()'
// constant functions !!
#endif
public:
  // CREATORS
  Am_Referent() : Am_Wrap_Base() {}
  Am_Referent(T &o) : Am_Wrap_Base(o) {}
#if 1
  Am_Referent(Am_Referent &o) : Am_Wrap_Base(o) {}
#endif
  virtual ~Am_Referent() {}
  // ACCESSORS
  static T *Narrow(Am_Wrap_Base *value) { return (dynamic_cast<T *>(value)); }
  // MANIPULATORS
  virtual Am_Wrap_Base *Make_Unique(void)
  {
    T *me = dynamic_cast<T *>(this);
    if (Is_Unique())
      return me;
    else {
      Release();
      return new T(*me);
      // TODO: Check that creator is always used
      // TODO: with only one argument...? Or provide a mean
      // TODO: to pass create additional create args...
    }
  }
  // OPERATORS
  bool operator==(const Am_Wrap_Base &test_value) const
  {
    // TODO: We compare pointers as 'Am_Wrap_Base*' - is it really needed to do a cast ?
    // TODO: Maybe this == &test_value would be enough ? -- ortalo
    // Check the type, then pointer or objects equality
    return (typeid(*this) == typeid(test_value) &&
            (static_cast<const Am_Wrap_Base *>(this) ==
                 static_cast<const Am_Wrap_Base *>(&test_value) ||
             (*(dynamic_cast<const T *>(this))) ==
                 dynamic_cast<const T &>(test_value)));
  }
};

#if 0
/*
 * Initialisation of static members
 */
template <class T, unsigned int in_id = Am_WRAPPER_TYPE>
Am_ID_Tag Am_Referent<T,in_id>::id = Am_Get_Unique_ID_Tag("toto", in_id);
#endif

/**************************************************
 *  Definition of the outer template for Amulet   *
 *  (reference counted and lazily copied objects) *
 **************************************************/
// Suggested usage:
//    typedef Am_Referee<referent_class> Am_Ptr
// or class Am_Object : public Am_Referee<referent_class>
// where:
//  referee_class is a class inheriting
//   from Am_Referent<referent_class>
template <class T> class Am_Referee
{
public:
  // TYPEDEFS
  typedef T referent_type;

protected:
  // DATA MEMBERS
  // This is a pointer to the real object used
  referent_type *data;
#ifdef PROVIDE_OLD_ID_TAGS
public:
  static Am_Value_Type Type_ID() { return referent_type::Type_ID(); }
#endif
public:
  // DATA MEMBERS
  // Null object
  static const referent_type *null;
  // CREATORS
  // Default constructor
  Am_Referee() : data(const_cast<referent_type *>(null)) {}
  // Copy constructor
  Am_Referee(const Am_Referee &original) : data(original.data)
  {
    // And increments the reference count
    if (data != null)
      data->Note_Reference();
  }
  // Constructor from inner type
  explicit Am_Referee(referent_type *in_data) : data(in_data)
  {
    // We obtain a new reference...
    if (data != null)
      data->Note_Reference();
  }
  // Constructor from Am_Value
  explicit Am_Referee(const Am_Value &in_value)
      : data(dynamic_cast<referent_type *>(in_value.value.wrapper_value))
  {
    if (data != null)
      data->Note_Reference();
  }
  ~Am_Referee()
  {
    // Releases the Am_Referent object
    if (data != null)
      data->Release();
    // NULLify the Am_Referee
    data = const_cast<referent_type *>(null);
  }
  // OPERATORS
  Am_Referee &operator=(referent_type *in_data)
  {
    // Avoid self-assignment
    if (data != in_data) {
      if (in_data != null)
        in_data->Note_Reference();
      if (data != null)
        data->Release();
      data = in_data;
    }
    return (*this);
  }
  // TODO: This method should be another template w/ T2... -- ortalo
  Am_Referee &operator=(const Am_Referee<T> &prev)
  {
#if 1
    return operator=(prev.data);
#else // Old way
    if (prev.data != null)
      (prev.data)->Note_Reference();
    if (data != null)
      data->Release();
    data = prev.data;
    return *this;
#endif
  }
  Am_Referee &operator=(const Am_Value &in_value)
  {
#if 1
    return operator=(
        dynamic_cast<referent_type *>(in_value.value.wrapper_value));
#else // Old way
    referent_type *in_data =
        static_cast<referent_type *>(in_value.value.wrapper_value);
    // Avoid self-assignment
    if (data != in_data) {
      referent_type *old_data = data;
      data = in_data;
      if (data != null) {
        // Type checking done by the compiler
        data->Note_Reference();
      }
      if (old_data != null)
        old_data->Release();
    }
    return (*this);
#endif
  }
  operator Am_Wrap_Base *() const
  {
    std::cout << "Going here " << data << std::endl;
    if (data != null)
      data->Note_Reference();
    return data;
  }
#if 0
  // TODO: useful ? --ortalo
  operator referent_type* () const
    {
      if (data != null)
	data->Note_Reference();
      return data;
    }
#endif
  // ACCESSORS
  bool Valid() const { return (data != null); }

  static referent_type *Narrow(Am_Wrap_Base *in_data)
  {
    std::cout << "in_data: " << in_data << std::endl;
    // If the wrapper is not a null one...
    if (in_data != static_cast<Am_Wrap_Base *>(0)) {
      std::cout << "Non null wrapper" << std::endl;
      if (typeid(referent_type) == typeid(*in_data)) {
        // TODO: Is this really correct ????? static_cast ? -- ortalo
        return (dynamic_cast<referent_type *>(in_data));
      } else {
        std::cerr << "Tried to get a referent_type from non-Am_Referee<T> !"
                  << std::endl;
        exit(1);
      }
    } else {
      std::cout << "NULL Cplusplus object" << std::endl;
      // Return empty object (?)
      return (null);
    }
    // Should NEVER be reached !
  }
  static bool Test(const Am_Wrap_Base *in_data)
  {
    return ((in_data != static_cast<Am_Wrap_Base *>(0)) &&
            (typeid(*in_data) == typeid(referent_type)));
  }
  static bool Test(const Am_Value &in_value)
  {
    return Test(in_value.value.wrapper_value);
  }
  //
  // Bridges to generic user-visible referent_type methods
  //
  const char *To_String() const
  {
    if (data != null)
      // Better no ?
      return data->To_String();
    else
      return (static_cast<const char *>(0));
  }
#if 1
  Am_Value From_String(const char *string)
  {
    if (data)
      return data->From_String(string);
    else
      return Am_No_Value;
  }
#endif
  void Print(stdstd::ostream &out) const
  {
    if (data != null)
      data->Print(out);
    else
      // TODO: Obtain a type name...
      out << "(Am_Referee<T>)(0L)";
  }
  void Println() const
  {
    Print(std::cout);
    std::cout << std::endl << std::flush;
  }
};

/*
 * Initialisation of static members
 */
template <class T>
const Am_Referee<T>::referent_type *
    Am_Referee<T>::null = (Am_Referee<T>::referent_type *)0L;
// TODO: 0L is used everywhere, so...
// TODO:Am_Referee<T>::null = (Am_Referee<T>::referent_type*)0x87654321;

#endif /* defined __REFEREE_H */
