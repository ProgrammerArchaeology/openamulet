#include <string.h>
#include <am_inc.h>

#ifdef OA_VERSION
#include <amulet/univ_map_oa.hpp>
#else
#include <amulet/univ_map.h>
#endif

#include <amulet/impl/types_logging.h>
#include <amulet/impl/types_load_save.h>
#include <amulet/impl/types_method.h>

#if 1
/////////////////////////////////////////
// Implementation of Load/Save context //
/////////////////////////////////////////

AM_DEFINE_METHOD_TYPE_IMPL(Am_Load_Method)
AM_DEFINE_METHOD_TYPE_IMPL(Am_Save_Method)

Am_Load_Save_Context Am_No_Load_Save_Context;
Am_Load_Method Am_No_Load_Method;
Am_Save_Method Am_No_Save_Method;

// Helper class for storage in load/save maps
class Wrapper_Holder
{
public:
  Wrapper_Holder() : data(0L) {}
  Wrapper_Holder(Am_Wrapper *in_data) : data(in_data) {}
  Wrapper_Holder(const Wrapper_Holder &item) : data(item.data)
  {
    if (data) {
      data->Note_Reference();
    }
  }
  ~Wrapper_Holder()
  {
    if (data) {
      data->Release();
    }
  }
  Wrapper_Holder &operator=(const Wrapper_Holder &item)
  {
    data = item.data;
    if (data) {
      data->Note_Reference();
    }
    return (*this);
  }
  bool operator<(const Wrapper_Holder &other) const
  {
    return data < other.data;
  }
  operator Am_Wrapper *()
  {
    if (data) {
      data->Note_Reference();
    }
    return (data);
  }
  bool Valid() { return (data != (0L)); }

  Am_Wrapper *data;
};

Wrapper_Holder No_Wrapper;

// Helper class for storage in load/save maps
class Name_Num
{
public:
  Name_Num() : name(0L), number(-1) {}
  Name_Num(const char *in_name) : name(in_name), number(-1) {}
  Name_Num(const char *in_name, int in_number)
      : name(in_name), number(in_number)
  {
  }
  Name_Num(const Name_Num &proto) : name(proto.name), number(proto.number) {}
  Name_Num &operator=(const Name_Num &proto)
  {
    name = proto.name;
    number = proto.number;
    return *this;
  }
  bool operator<(const Name_Num &other) const
  {
    return std::lexicographical_compare(name, name + strlen(name), other.name,
                                        other.name + strlen(other.name)) ||
           (number < other.number);
  }

  const char *name;
  int number;
};

#ifdef DEBUG
std::ostream &
operator<<(std::ostream &os, Name_Num const &aNameNum)
{
  std::cout << aNameNum.name << " " << aNameNum.number << std::endl;
  return (os);
}

std::ostream &
operator<<(std::ostream &os, Wrapper_Holder const &aWrapperHolder)
{
  std::cout << aWrapperHolder.data << std::endl;
  return (os);
}
#endif

static int
HashValue(const Wrapper_Holder &key, int size)
{
  return (long)key.data % size;
}

static int
KeyComp(const Wrapper_Holder &key1, const Wrapper_Holder &key2)
{
  return !(key1.data == key2.data);
}

static int
HashValue(const Name_Num &key, int size)
{
  int hash = (HashValue(key.name, size) + key.number + 10) % size;
  return hash;
}

static int
KeyComp(const Name_Num &key1, const Name_Num &key2)
{
  return strcmp(key1.name, key2.name) || key1.number != key2.number;
}

#ifdef OA_VERSION
#if 0
//TODO: Remove
template <class T> struct lex_less : public std::binary_function<T, T, bool>
{
    bool operator()(const T& x, const T& y) const
    {
    	return(std::lexicographical_compare(x, x+strlen(x), y, y+strlen(y)));
    }
};
#endif
typedef OpenAmulet::Map<const char *, Am_Load_Method, lex_less<const char *>>
    Am_Map_Loader;
typedef OpenAmulet::Map_Iterator<Am_Map_Loader> Am_MapIterator_Loader;

typedef OpenAmulet::Map<Am_ID_Tag, Am_Save_Method> Am_Map_Saver;
typedef OpenAmulet::Map_Iterator<Am_Map_Saver> Am_MapIterator_Saver;

typedef OpenAmulet::Map<Name_Num, Wrapper_Holder> Am_Map_Prototypes;
typedef OpenAmulet::Map_Iterator<Am_Map_Prototypes> Am_MapIterator_Prototypes;

typedef OpenAmulet::Map<Wrapper_Holder, Name_Num> Am_Map_Names;
typedef OpenAmulet::Map_Iterator<Am_Map_Names> Am_MapIterator_Names;

typedef OpenAmulet::Map<int, Wrapper_Holder> Am_Map_Wrappers;
typedef OpenAmulet::Map_Iterator<Am_Map_Wrappers> Am_MapIterator_Wrappers;

typedef OpenAmulet::Map<Wrapper_Holder, int> Am_Map_References;
typedef OpenAmulet::Map_Iterator<Am_Map_References> Am_MapIterator_References;

#else
AM_DECL_MAP(Loader, const char *, Am_Load_Method)
AM_IMPL_MAP(Loader, const char *, (0L), Am_Load_Method, Am_No_Load_Method)

AM_DECL_MAP(Saver, Am_ID_Tag, Am_Save_Method)
AM_IMPL_MAP(Saver, Am_ID_Tag, Am_NONE, Am_Save_Method, Am_No_Save_Method)

AM_DECL_MAP(Prototypes, Name_Num, Wrapper_Holder)
AM_IMPL_MAP(Prototypes, Name_Num, Name_Num(), Wrapper_Holder, No_Wrapper)

AM_DECL_MAP(Names, Wrapper_Holder, Name_Num)
AM_IMPL_MAP(Names, Wrapper_Holder, No_Wrapper, Name_Num, Name_Num())

AM_DECL_MAP(Wrappers, int, Wrapper_Holder)
AM_IMPL_MAP(Wrappers, int, -1, Wrapper_Holder, No_Wrapper)

AM_DECL_MAP(References, Wrapper_Holder, int)
AM_IMPL_MAP(References, Wrapper_Holder, No_Wrapper, int, -1)

#endif

class Am_Load_Save_Context_Data : public Am_Wrapper
{
  AM_WRAPPER_DATA_DECL(Am_Load_Save_Context)

public:
  Am_Load_Save_Context_Data();
  Am_Load_Save_Context_Data(Am_Load_Save_Context_Data *proto);

  ~Am_Load_Save_Context_Data();

  bool operator==(const Am_Load_Save_Context_Data & /*test*/) const
  {
    return (false);
  }

  // depending on the version of Amulet, this are STL maps or Amulet maps
  // se typedefs above
  Am_Map_Loader *loader;
  Am_Map_Saver *saver;
  Am_Map_Prototypes *prototypes;
  Am_Map_Names *names;
  Am_Map_Wrappers *wrappers;
  Am_Map_References *references;

  // counter for data-value number
  int counter;
};

AM_WRAPPER_DATA_IMPL(Am_Load_Save_Context, (this))

// This ctor is called as a result of including AM_WRAPPER_DECL(Am_Load_Save_Context)
// in class Am_Load_Save_Context, which declares the data-member 'data' of type
// Am_Load_Save_Context_Data.
Am_Load_Save_Context_Data::Am_Load_Save_Context_Data() : counter(0)
{
  // For OpenAmulet the R-Values are typedefs for STL maps
  loader = new Am_Map_Loader();
  saver = new Am_Map_Saver();
  prototypes = new Am_Map_Prototypes();
  names = new Am_Map_Names();
  wrappers = new Am_Map_Wrappers();
  references = new Am_Map_References();

#ifdef OA_VERSION
  // set the default-return value for references
  references->SetDefaultReturnValue(-1);
// TODO: We should set other default values also no ? --ortalo
#endif
}

Am_Load_Save_Context_Data::Am_Load_Save_Context_Data(
    Am_Load_Save_Context_Data *proto)
    : counter(0)
{
  loader = proto->loader->Copy();
  saver = proto->saver->Copy();
  prototypes = proto->prototypes->Copy();
  names = proto->names->Copy();
  wrappers = new Am_Map_Wrappers();
  references = new Am_Map_References();

#ifdef OA_VERSION
  // set the default-return value for references
  references->SetDefaultReturnValue(-1);
// TODO: idem as above ?
#endif
}

Am_Load_Save_Context_Data::~Am_Load_Save_Context_Data()
{
  loader->Clear();
  delete (loader);

  saver->Clear();
  delete (saver);

  prototypes->Clear();
  delete (prototypes);

  names->Clear();
  delete (names);

  wrappers->Clear();
  delete (wrappers);

  references->Clear();
  delete (references);
}

AM_WRAPPER_IMPL(Am_Load_Save_Context)

void
Am_Load_Save_Context::Register_Prototype(const char *name, Am_Wrapper *value)
{
  if (!data) {
    data = new Am_Load_Save_Context_Data();
  }

  data = static_cast<Am_Load_Save_Context_Data *>(data->Make_Unique());

  Wrapper_Holder holder(value);

  data->names->SetAt(holder, Name_Num(name));
  data->prototypes->SetAt(Name_Num(name), holder);
}

void
Am_Load_Save_Context::Register_Prototype(const char *name, unsigned base,
                                         Am_Wrapper *value)
{
  if (!data) {
    data = new Am_Load_Save_Context_Data();
  }

  data = (Am_Load_Save_Context_Data *)data->Make_Unique();
  Wrapper_Holder holder(value);
  data->names->SetAt(holder, Name_Num(name, base));
  data->prototypes->SetAt(Name_Num(name, base), holder);
}

void
Am_Load_Save_Context::Register_Loader(const char *type_name,
                                      const Am_Load_Method &method)
{
  if (!data) {
    data = new Am_Load_Save_Context_Data();
  }

  data = static_cast<Am_Load_Save_Context_Data *>(data->Make_Unique());

  // store the type_name / loader_method pair into the map
  data->loader->SetAt(type_name, method);
}

void
Am_Load_Save_Context::Register_Saver(Am_ID_Tag type,
                                     const Am_Save_Method &method)
{
  if (!data) {
    data = new Am_Load_Save_Context_Data();
  }

  data = static_cast<Am_Load_Save_Context_Data *>(data->Make_Unique());

  // store the type_name / saver_method pair into the map
  data->saver->SetAt(type, method);
}

void
Am_Load_Save_Context::Reset()
{
  if (data) {
    data = (Am_Load_Save_Context_Data *)data->Make_Unique();
    data->counter = 0;
    data->wrappers->Clear();
    data->references->Clear();
  }
}

Am_Value
Am_Load_Save_Context::Load(std::istream &is)
{
  if (!data) {
    Am_Error("Load called on a (0L) load/save context");
  }

  // operator >> will skip whitespace
  char ch;
  is >> ch;

  // check what type we have
  switch (ch) {
  case 'D': {
    // read in the number
    int number;
    if (!(is >> number)) {
      // ups, there is none...
      return Am_No_Value;
    }
    data->counter = number;

    // read in the type name
    char type_name[100]; // TODO: Remove that 100 chars arbitrary limit
    if (!(is >> type_name)) {
      return Am_No_Value;
    }

    // get the loader-method for the given type-name by asking the
    // map which stores the type-name/loader-method pairs
    Am_Load_Method method = data->loader->GetAt(type_name);

    // did we received a valid method-pointer?
    if (method.Valid() == false) {
      // no
      return Am_No_Value;
    }

    // yes, than read the value
    Am_Value value = method.Call(is, *this);

    // check to see if all types are the same
    if (value.Valid() && Am_Type_Class(value.type) == Am_WRAPPER) {
      data->wrappers->SetAt(number, Wrapper_Holder(value));
    }

    // and return the found value
    return (value);
  }

  case 'R': {
    if (!(is >> data->counter))
      return Am_No_Value;
    // is.get (ch); // skip eoln
    // if (ch != '\n') return Am_No_Value;
    Wrapper_Holder value = data->wrappers->GetAt(data->counter);
    if (!value.Valid())
      return Am_No_Value;
    return Am_Value(value);
  }

  case 'N': {
    // is.get (ch); // skip eoln
    // if (ch != '\n') return Am_No_Value;
    return Am_Value(No_Wrapper);
  }

  case 'P': {
    char name[100]; // TODO: Remove that 100 chars arbitrary limit
    if (!(is >> name))
      return Am_No_Value;
    is.get(ch); // will be space or (part of) EOLN
    Wrapper_Holder value;
    if (ch == ' ') {
      int base;
      if (!(is >> base))
        return Am_No_Value;
      value = data->prototypes->GetAt(Name_Num(name, base));
      // is.get (ch); // skip eoln
      // if (ch != '\n') return Am_No_Value;
    } else
      value = data->prototypes->GetAt(Name_Num(name));
    if (!value.Valid())
      return Am_No_Value;
    return Am_Value(value);
  }

  default: {
    // no known type found...
    return (Am_No_Value);
  }
  }
}

const char *
Am_Load_Save_Context::Is_Registered_Prototype(Am_Wrapper *value)
{
  Wrapper_Holder holder(value);
  if (!data)
    return (0L);
  Name_Num name = data->names->GetAt(holder);
  return name.name;
}

void
Am_Load_Save_Context::Save(std::ostream &os, const Am_Value &value)
{
  if (!data) {
    Am_Error("Save called on a (0L) load/save context");
  }

  // do we have a wrapper type?
  if (Am_Type_Class(value.type) == Am_WRAPPER) {
    // doesn't seemed to be a valid value
    if (value.Valid() == false) {
      os << "N" << std::endl;
      return;
    }

    // does there already exist a reference to the object in the
    // file? (the object was already saved, if the reference is not -1)
    Wrapper_Holder holder(value);
    int reference = data->references->GetAt(holder);
    if (reference != -1) {
      // yes, the object already exists, just write out a reference
      os << "R" << reference << std::endl;
      return;
    }

    // get the name of the object/slot
    Name_Num name = data->names->GetAt(holder);
    if (name.name) {
      // write out the name
      os << "P" << name.name;

      // do we have several same names?
      if (name.number == -1) {
        // no, that's it
        os << std::endl;
      } else {
        // yes, than write out a number so that we can distinguish them
        os << " " << name.number << std::endl;
      }

      return;
    }

    // store the data-value number for the just written object, so
    // an other lookup of the reference will return the correct
    // number in the file
    data->references->SetAt(holder, data->counter);
  }

  // write data packet counter and increase packet counter
  os << "D" << data->counter << std::endl;
  ++data->counter;

  // Lookup the method-pointer for saving objects of type 'value.type'
  Am_Save_Method method = data->saver->GetAt(value.type);

  // do we have a good method pointer?
  if (!method.Valid()) {
    // no, this is an error!
    std::cerr << "** Didn't found a method for saving values of type ";
    Am_Print_Type(std::cerr, value.type);
    std::cerr << std::endl;
    Am_Error();
  }

  // yes, than write the value to the stream
  method.Call(os, *this, value);
}

void
Am_Load_Save_Context::Recursive_Load_Ahead(Am_Wrapper *value)
{
  if (!data)
    Am_Error("Recursive_Load_Ahead called on a (0L) load/save context");
  if (value)
    data->wrappers->SetAt(data->counter, Wrapper_Holder(value));
  else
    Am_Error("Not allowed to store (0L) as a load ahead");
}

void
Am_Load_Save_Context::Save_Type_Name(std::ostream &os, const char *type_name)
{
  if (!data)
    Am_Error("Save_Type_Name called on a (0L) load/save context");
  os << type_name << std::endl;
}

#else
//////////////////////////////////////
// Implementation of Load/Save context

AM_DEFINE_METHOD_TYPE_IMPL(Am_Load_Method)
AM_DEFINE_METHOD_TYPE_IMPL(Am_Save_Method)

Am_Load_Save_Context Am_No_Load_Save_Context;
Am_Load_Method Am_No_Load_Method;
Am_Save_Method Am_No_Save_Method;

class Wrapper_Holder
{
public:
  Wrapper_Holder() { data = (0L); }

  Wrapper_Holder(Am_Wrapper *in_data) { data = in_data; }

  Wrapper_Holder(const Wrapper_Holder &item)
  {
    data = item.data;
    if (data) {
      data->Note_Reference();
    }
  }

  ~Wrapper_Holder()
  {
    if (data) {
      data->Release();
    }
  }

  Wrapper_Holder &operator=(const Wrapper_Holder &item)
  {
    data = item.data;
    if (data) {
      data->Note_Reference();
    }

    return (*this);
  }

  operator Am_Wrapper *()
  {
    if (data) {
      data->Note_Reference();
    }

    return (data);
  }

  bool Valid() { return (data != (0L)); }

  Am_Wrapper *data;
};

#ifdef OA_VERSION
// we specialize the less function used by the map data-type
bool
std::less<Wrapper_Holder>::operator()(const class Wrapper_Holder &a,
                                      const class Wrapper_Holder &b) const
{
  return (a.data < b.data);
}

#endif
Wrapper_Holder No_Wrapper;

class Name_Num
{
public:
  Name_Num()
  {
    name = (0L);
    number = -1;
  }
  Name_Num(const char *in_name)
  {
    name = in_name;
    number = -1;
  }
  Name_Num(const char *in_name, int in_number)
  {
    name = in_name;
    number = in_number;
  }
  Name_Num(const Name_Num &proto)
  {
    name = proto.name;
    number = proto.number;
  }
  Name_Num &operator=(const Name_Num &proto)
  {
    name = proto.name;
    number = proto.number;
    return *this;
  }

  const char *name;
  int number;
};

#ifdef OA_VERSION
// we specialize the less function used by the map data-type
bool
std::less<Name_Num>::operator()(const class Name_Num &a,
                                const class Name_Num &b) const
{
  return ((std::lexicographical_compare(a.name, a.name + strlen(a.name), b.name,
                                        b.name + strlen(b.name))) ||
          (a.number < b.number));
}

#ifdef DEBUG
std::ostream &
operator<<(std::ostream &os, Name_Num const &aNameNum)
{
  std::cout << aNameNum.name << " " << aNameNum.number << std::endl;
  return (os);
}

std::ostream &
operator<<(std::ostream &os, Wrapper_Holder const &aWrapperHolder)
{
  std::cout << aWrapperHolder.data << std::endl;
  return (os);
}
#endif

#endif

static int
HashValue(const Wrapper_Holder &key, int size)
{
  return (long)key.data % size;
}

static int
KeyComp(const Wrapper_Holder &key1, const Wrapper_Holder &key2)
{
  return !(key1.data == key2.data);
}

static int
HashValue(const Name_Num &key, int size)
{
  int hash = (HashValue(key.name, size) + key.number + 10) % size;
  return hash;
}

static int
KeyComp(const Name_Num &key1, const Name_Num &key2)
{
  return strcmp(key1.name, key2.name) || key1.number != key2.number;
}

#ifdef OA_VERSION
typedef OpenAmulet::Map<const char *, Am_Load_Method, lex_less<const char *>>
    Am_Map_Loader;
typedef OpenAmulet::Map_Iterator<Am_Map_Loader> Am_MapIterator_Loader;

typedef OpenAmulet::Map<Am_ID_Tag, Am_Save_Method> Am_Map_Saver;
typedef OpenAmulet::Map_Iterator<Am_Map_Saver> Am_MapIterator_Saver;

typedef OpenAmulet::Map<Name_Num, Wrapper_Holder> Am_Map_Prototypes;
typedef OpenAmulet::Map_Iterator<Am_Map_Prototypes> Am_MapIterator_Prototypes;

typedef OpenAmulet::Map<Wrapper_Holder, Name_Num> Am_Map_Names;
typedef OpenAmulet::Map_Iterator<Am_Map_Names> Am_MapIterator_Names;

typedef OpenAmulet::Map<int, Wrapper_Holder> Am_Map_Wrappers;
typedef OpenAmulet::Map_Iterator<Am_Map_Wrappers> Am_MapIterator_Wrappers;

typedef OpenAmulet::Map<Wrapper_Holder, int> Am_Map_References;
typedef OpenAmulet::Map_Iterator<Am_Map_References> Am_MapIterator_References;

#else
AM_DECL_MAP(Loader, const char *, Am_Load_Method)
AM_IMPL_MAP(Loader, const char *, (0L), Am_Load_Method, Am_No_Load_Method)

AM_DECL_MAP(Saver, Am_ID_Tag, Am_Save_Method)
AM_IMPL_MAP(Saver, Am_ID_Tag, Am_NONE, Am_Save_Method, Am_No_Save_Method)

AM_DECL_MAP(Prototypes, Name_Num, Wrapper_Holder)
AM_IMPL_MAP(Prototypes, Name_Num, Name_Num(), Wrapper_Holder, No_Wrapper)

AM_DECL_MAP(Names, Wrapper_Holder, Name_Num)
AM_IMPL_MAP(Names, Wrapper_Holder, No_Wrapper, Name_Num, Name_Num())

AM_DECL_MAP(Wrappers, int, Wrapper_Holder)
AM_IMPL_MAP(Wrappers, int, -1, Wrapper_Holder, No_Wrapper)

AM_DECL_MAP(References, Wrapper_Holder, int)
AM_IMPL_MAP(References, Wrapper_Holder, No_Wrapper, int, -1)

#endif

class Am_Load_Save_Context_Data : public Am_Wrapper
{
  AM_WRAPPER_DATA_DECL(Am_Load_Save_Context)

public:
  Am_Load_Save_Context_Data();
  Am_Load_Save_Context_Data(Am_Load_Save_Context_Data *proto);

  ~Am_Load_Save_Context_Data();

  bool operator==(const Am_Load_Save_Context_Data & /*test*/) const
  {
    return (false);
  }

  // depending on the version of Amulet, this are STL maps or Amulet maps
  // se typedefs above
  Am_Map_Loader *loader;
  Am_Map_Saver *saver;
  Am_Map_Prototypes *prototypes;
  Am_Map_Names *names;
  Am_Map_Wrappers *wrappers;
  Am_Map_References *references;

  // counter for data-value number
  int counter;
};

AM_WRAPPER_DATA_IMPL(Am_Load_Save_Context, (this))

// This ctor is called as a result of including AM_WRAPPER_DECL(Am_Load_Save_Context)
// in class Am_Load_Save_Context, which declares the data-member 'data' of type
// Am_Load_Save_Context_Data.
Am_Load_Save_Context_Data::Am_Load_Save_Context_Data()
{
  // For OpenAmulet the R-Values are typedefs for STL maps
  loader = new Am_Map_Loader();
  saver = new Am_Map_Saver();
  prototypes = new Am_Map_Prototypes();
  names = new Am_Map_Names();
  wrappers = new Am_Map_Wrappers();
  references = new Am_Map_References();
  counter = 0;

#ifdef OA_VERSION
  // set the default-return value for references
  references->SetDefaultReturnValue(-1);
#endif
}

Am_Load_Save_Context_Data::Am_Load_Save_Context_Data(
    Am_Load_Save_Context_Data *proto)
{
  loader = proto->loader->Copy();
  saver = proto->saver->Copy();
  prototypes = proto->prototypes->Copy();
  names = proto->names->Copy();
  wrappers = new Am_Map_Wrappers();
  references = new Am_Map_References();
  counter = 0;

#ifdef OA_VERSION
  // set the default-return value for references
  references->SetDefaultReturnValue(-1);
#endif
}

Am_Load_Save_Context_Data::~Am_Load_Save_Context_Data()
{
  loader->Clear();
  delete (loader);

  saver->Clear();
  delete (saver);

  prototypes->Clear();
  delete (prototypes);

  names->Clear();
  delete (names);

  wrappers->Clear();
  delete (wrappers);

  references->Clear();
  delete (references);
}

AM_WRAPPER_IMPL(Am_Load_Save_Context)

void
Am_Load_Save_Context::Register_Prototype(const char *name, Am_Wrapper *value)
{
  if (!data) {
    data = new Am_Load_Save_Context_Data();
  }

  data = static_cast<Am_Load_Save_Context_Data *>(data->Make_Unique());

  Wrapper_Holder holder(value);

  data->names->SetAt(holder, Name_Num(name));
  data->prototypes->SetAt(Name_Num(name), holder);
}

void
Am_Load_Save_Context::Register_Prototype(const char *name, unsigned base,
                                         Am_Wrapper *value)
{
  if (!data) {
    data = new Am_Load_Save_Context_Data();
  }

  data = (Am_Load_Save_Context_Data *)data->Make_Unique();
  Wrapper_Holder holder(value);
  data->names->SetAt(holder, Name_Num(name, base));
  data->prototypes->SetAt(Name_Num(name, base), holder);
}

void
Am_Load_Save_Context::Register_Loader(const char *type_name,
                                      const Am_Load_Method &method)
{
  if (!data) {
    data = new Am_Load_Save_Context_Data();
  }

  data = static_cast<Am_Load_Save_Context_Data *>(data->Make_Unique());

  // store the type_name / loader_method pair into the map
  data->loader->SetAt(type_name, method);
}

void
Am_Load_Save_Context::Register_Saver(Am_ID_Tag type,
                                     const Am_Save_Method &method)
{
  if (!data) {
    data = new Am_Load_Save_Context_Data();
  }

  data = static_cast<Am_Load_Save_Context_Data *>(data->Make_Unique());

  // store the type_name / saver_method pair into the map
  data->saver->SetAt(type, method);
}

void
Am_Load_Save_Context::Reset()
{
  if (data) {
    data = (Am_Load_Save_Context_Data *)data->Make_Unique();
    data->counter = 0;
    data->wrappers->Clear();
    data->references->Clear();
  }
}

Am_Value
Am_Load_Save_Context::Load(istream &is)
{
  if (!data) {
    Am_Error("Load called on a (0L) load/save context");
  }

  // operator >> will skip whitespace
  char ch;
  is >> ch;

  // check what type we have
  switch (ch) {
  case 'D': {
    // read in the number
    int number;
    if (!(is >> number)) {
      // ups, there is none...
      return Am_No_Value;
    }
    data->counter = number;

    // read in the type name
    char type_name[100];
    if (!(is >> type_name)) {
      return Am_No_Value;
    }

    // get the loader-method for the given type-name by asking the
    // map which stores the type-name/loader-method pairs
    Am_Load_Method method = data->loader->GetAt(type_name);

    // did we received a valid method-pointer?
    if (method.Valid() == false) {
      // no
      return Am_No_Value;
    }

    // yes, than read the value
    Am_Value value = method.Call(is, *this);

    // check to see if all types are the same
    if (value.Valid() && Am_Type_Class(value.type) == Am_WRAPPER) {
      data->wrappers->SetAt(number, Wrapper_Holder(value));
    }

    // and return the found value
    return (value);
  }

  case 'R': {
    if (!(is >> data->counter))
      return Am_No_Value;
    // is.get (ch); // skip eoln
    // if (ch != '\n') return Am_No_Value;
    Wrapper_Holder value = data->wrappers->GetAt(data->counter);
    if (!value.Valid())
      return Am_No_Value;
    return Am_Value(value);
  }

  case 'N': {
    // is.get (ch); // skip eoln
    // if (ch != '\n') return Am_No_Value;
    return Am_Value(No_Wrapper);
  }

  case 'P': {
    char name[100];
    if (!(is >> name))
      return Am_No_Value;
    is.get(ch); // will be space or (part of) EOLN
    Wrapper_Holder value;
    if (ch == ' ') {
      int base;
      if (!(is >> base))
        return Am_No_Value;
      value = data->prototypes->GetAt(Name_Num(name, base));
      // is.get (ch); // skip eoln
      // if (ch != '\n') return Am_No_Value;
    } else
      value = data->prototypes->GetAt(Name_Num(name));
    if (!value.Valid())
      return Am_No_Value;
    return Am_Value(value);
  }

  default: {
    // no known type found...
    return (Am_No_Value);
  }
  }
}

const char *
Am_Load_Save_Context::Is_Registered_Prototype(Am_Wrapper *value)
{
  Wrapper_Holder holder(value);
  if (!data)
    return (0L);
  Name_Num name = data->names->GetAt(holder);
  return name.name;
}

void
Am_Load_Save_Context::Save(std::ostream &os, const Am_Value &value)
{
  if (!data) {
    Am_Error("Save called on a (0L) load/save context");
  }

  // do we have a wrapper type?
  if (Am_Type_Class(value.type) == Am_WRAPPER) {
    // doesn't seemed to be a valid value
    if (value.Valid() == false) {
      os << "N" << std::endl;
      return;
    }

    // does there already exist a reference to the object in the
    // file? (the object was already saved, if the reference is not -1)
    Wrapper_Holder holder(value);
    int reference = data->references->GetAt(holder);
    if (reference != -1) {
      // yes, the object already exists, just write out a reference
      os << "R" << reference << std::endl;
      return;
    }

    // get the name of the object/slot
    Name_Num name = data->names->GetAt(holder);
    if (name.name) {
      // write out the name
      os << "P" << name.name;

      // do we have several same names?
      if (name.number == -1) {
        // no, that's it
        os << std::endl;
      } else {
        // yes, than write out a number so that we can distinguish them
        os << " " << name.number << std::endl;
      }

      return;
    }

    // store the data-value number for the just written object, so
    // an other lookup of the reference will return the correct
    // number in the file
    data->references->SetAt(holder, data->counter);
  }

  // write data packet counter and increase packet counter
  os << "D" << data->counter << std::endl;
  ++data->counter;

  // Lookup the method-pointer for saving objects of type 'value.type'
  Am_Save_Method method = data->saver->GetAt(value.type);

  // do we have a good method pointer?
  if (!method.Valid()) {
    // no, this is an error!
    std::cerr << "** Didn't found a method for saving values of type ";
    Am_Print_Type(cerr, value.type);
    std::cerr << std::endl;
    Am_Error();
  }

  // yes, than write the value to the stream
  method.Call(os, *this, value);
}

void
Am_Load_Save_Context::Recursive_Load_Ahead(Am_Wrapper *value)
{
  if (!data)
    Am_Error("Recursive_Load_Ahead called on a (0L) load/save context");
  if (value)
    data->wrappers->SetAt(data->counter, Wrapper_Holder(value));
  else
    Am_Error("Not allowed to store (0L) as a load ahead");
}

void
Am_Load_Save_Context::Save_Type_Name(std::ostream &os, const char *type_name)
{
  if (!data)
    Am_Error("Save_Type_Name called on a (0L) load/save context");
  os << type_name << std::endl;
}
#endif
