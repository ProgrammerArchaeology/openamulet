// The template parameter T gives the type of objects to be stored in this
// wrapper (like Am_Wrapper<My_Big_Data_Object>	myWrapper)
template <class T> class Am_Wrapper
{
public:
  // CREATORS
  Am_Wrapper(const Am_Wrapper<T> &aWrapper);
  Am_Wrapper(const Am_Value &aValue);
  Am_Wrapper(T *aWrapperData);

  ~Am_Wrapper();
  // OPERATORS
  Am_Wrapper<T> &operator=(const Am_Wrapper<T> &aWrapper);
  Am_Wrapper<T> &operator=(const Am_Value &aValue);
  Am_Wrapper<T> &operator=(T *aWrapperData);

  // Type conversion operator
  operator Am_Wrapper *() const;

  // MANIPULATORS
  static T Narrow(Am_Wrapper *aObject);
  static bool Test(const Am_Wrapper *aObject);
  static bool Test(const Am_Value &aValue);
  static Am_Value_Type Type_ID();

  // ACCESSORS
  bool Valid() const;
  const char *To_String() const;
  void Print(std::ostream &out) const;
  void PrintLn() const;

  // DATA MEMBERS

protected:
  // CREATORS
  // MANIPULATORS
  // ACCESSORS
  // DATA MEMBERS

private:
  // CREATORS
  // MANIPULATORS
  // ACCESSORS
  // DATA MEMBERS
  T *wrapperData;
};

// typedef Am_Object wrapper<Am_Object_Data_Rep>
template <class T> class wrapper
{
protected:
  wrapped<T> *data;

public:
  wrapper<T>(const wrapper &prev)
  {
    data = prev.data;
    if (data)
      data->Note_Reference;
  }
  wrapper(const Am_Value &in_value)
  {
    data = (T *)in_value.value.wrapper_value;
    if (data) {
      // TODO: Do type ID error checking
      data->Note_Reference();
    }
  }
  wrapper(T *in_data) { data = in_data; }
  ~wrapper()
  {
    if (data) {
      if (data->Is_Zero())
        Am_Error("Tried to delete the same type twice");
      data->Release();
    }
    data = nullptr;
  }
}

// typedef Am_Object_Data wrapped<Am_Object_Data_Rep>
template <class T, int in_id = Am_WRAPPER_TYPE>
class wrapped : public Am_Wrapper
{
private:
  // TODO: Check how to pass type name...
  // TODO:Maybe use some feature of T ? (T.name?)
  static Am_ID_Tag id = Am_Get_Unique_ID_Tag(typeof(T), in_id);

public:
  Am_Wrapper *Make_Unique()
  {
    if (Is_Unique())
      return this;
    else {
      Release();
      // TODO: Check that creator is always used
      // with only one argument...?
      return new wrapped<T>(this);
    }
  }
  static wrapped<T> Narrow(Am_Wrapper *value)
  {
    if (value && (value->ID() == id))
      return (wrapped<T> *)value;
    else
      return nullptr;
  }
  bool operator==(const Am_Wrapper &test_value) const
  {
    if (id == test_value.ID())
      return (&test_value == this) ||
             (this->operator==((wrapped<T> &)test_value));
    else
      return false;
  }
  Am_ID_Tag ID() const { return id; }
  //TODO: Check if we need another check for ID ?
}
