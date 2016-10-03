/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <am_inc.h>

#include <amulet/am_io.h>

// This is needed to initialize
// However, this test should not need it! --ortalo
#include <amulet/opal.h>

#include <amulet/formula_advanced.h>
#include <amulet/standard_slots.h>
#include <amulet/value_list.h>

#include <amulet/types.h>

#include <amulet/formula.h>
#include <amulet/web.h>

#include <amulet/registry.h>

#include <string.h>

#include <ostream>

using namespace std;

#ifdef AMULET2_INSTRUMENT
#undef Set
#undef Add
#endif

#define LEFT 10000
#define PARENT 10010
#define TOP 10020
#define WIDTH 10030
#define HEIGHT 10040
#define RIGHT 10050
#define SHADOW 10060
Am_Value_Type INPUT_A = Am_Register_Slot_Name("INPUT A");
Am_Value_Type INPUT_B = Am_Register_Slot_Name("INPUT B");
Am_Value_Type OUTPUT_A = Am_Register_Slot_Name("OUTPUT A");
Am_Value_Type OUTPUT_B = Am_Register_Slot_Name("OUTPUT B");
Am_Value_Type OBJECT = Am_Register_Slot_Name("OBJECT");
Am_Value_Type BAD_DEMON_TRIGGER = Am_Register_Slot_Name("BAD_DEMON_TRIGGER");

int suppress_successful = 1;
int test_counter = 0;
int failed_tests = 0;

//pointer wrapper tests
class my_int
{
public:
  int my_int_val;
};

Am_Define_Pointer_Wrapper(my_int) Am_Define_Pointer_Wrapper(float)

    class my_int_wrapper_support : public Am_Type_Support
{
public:
  void Print(ostream &os, const Am_Value &val) const
  {
    my_int *m = Am_my_int(val).value;
    os << "PINT(" << std::hex << (Am_Ptr)val << std::dec
       << ")=" << m->my_int_val;
  }
} my_int_wrapper_support_obj;

Am_Define_Pointer_Wrapper_Impl(my_int, &my_int_wrapper_support_obj);
Am_Define_Pointer_Wrapper_Impl(float, (0L));

void
test_int(float id, int result, int expected)
{
  ++test_counter;
  if (suppress_successful) {
    if ((test_counter % 50) == 0)
      cout << "Test " << test_counter << endl;
  } else
    cout << id << "     ";

  if (result == expected) {
    if (!suppress_successful)
      cout << "  result is " << result << endl;
  } else {
    cout << id << "       expected " << expected << ", result is " << result
         << endl
         << "*** TEST DID NOT SUCCEED. ***" << endl;
    ++failed_tests;
  }
}

void
test_flt(float id, float result, float expected)
{
  ++test_counter;
  if (suppress_successful) {
    if ((test_counter % 50) == 0)
      cout << "Test " << test_counter << endl;
  } else
    cout << id << "     ";

  if (result == expected) {
    if (!suppress_successful)
      cout << "  result is " << result << endl;
  } else {
    cout << id << "       expected " << expected << ", result is " << result
         << endl
         << "*** TEST DID NOT SUCCEED. ***" << endl;
    ++failed_tests;
  }
}

void
test_val(float id, Am_Wrapper *result, Am_Wrapper *expected)
{
  ++test_counter;
  if (suppress_successful) {
    if ((test_counter % 50) == 0)
      cout << "Test " << test_counter << endl;
  } else
    cout << id << "     ";

  if (*result == *expected) {
    if (!suppress_successful)
      cout << "  result is " << result << endl;
  } else {
    cout << id << "       expected " << expected << " result is " << result
         << endl
         << "*** TEST DID NOT SUCCEED. ***" << endl;
    ++failed_tests;
  }
  result->Release();
  expected->Release();
}

void
test_obj(float id, Am_Object result, Am_Object expected)
{
  ++test_counter;
  if (suppress_successful) {
    if ((test_counter % 50) == 0)
      printf("Test %d\n", test_counter);
  } else
    printf("%f     ", id);

  if (result == expected) {
    if (!suppress_successful)
      cout << "  result is " << result << endl;
  } else {
    cout << id << "       expected " << expected << ", result is " << result
         << endl
         << "*** TEST DID NOT SUCCEED. ***" << endl;
    ++failed_tests;
  }
}

void
test_slots(float id, Am_Object obj, Am_Slot_Key keys[], int size)
{
  Am_Slot_Key result_keys[10];
  int num_results = 0;
  int i, j;

  Am_Slot_Iterator iter = obj;
  iter.Start();
  while (!iter.Last()) {
    result_keys[num_results++] = iter.Get();
    iter.Next();
  }
  if (num_results == size) {
    bool fail = false;
    for (i = 0; i < num_results - 1; ++i)
      for (j = i + 1; j < num_results; ++j)
        if (result_keys[i] == result_keys[j])
          fail = true;
    if (!fail) {
      for (i = 0; i < num_results; ++i) {
        fail = true;
        for (j = 0; j < size; ++j)
          if (keys[j] == result_keys[i]) {
            fail = false;
            break;
          }
        if (fail)
          break;
      }
      if (!fail) {
        if (!suppress_successful)
          printf("  result is expected\n");
        return;
      } else
        printf("%f       returned keys do not match expected\n", id);
    } else
      printf("%f       slot key returned more than once\n", id);
  } else
    printf("%f       number of slot do not match\n", id);
  for (i = 0; i < size; ++i)
    printf("expected key[%d] = %d ", i, keys[i]);
  printf("\n");
  for (i = 0; i < num_results; ++i)
    printf("result key[%d] = %d ", i, result_keys[i]);
  printf("\n");
  printf("*** TEST DID NOT SUCCEED. ***\n");
  ++failed_tests;
}

void
test_inst(float id, Am_Object obj, Am_Object names[], int size)
{
  Am_Object result_names[10];
  int num_results = 0;
  int i, j;

  Am_Instance_Iterator iter = obj;
  iter.Start();
  while (!iter.Last()) {
    result_names[num_results++] = iter.Get();
    iter.Next();
  }
  if (num_results == size) {
    bool fail = false;
    for (i = 0; i < num_results - 1; ++i)
      for (j = i + 1; j < num_results; ++j)
        if (result_names[i] == result_names[j])
          fail = true;
    if (!fail) {
      for (i = 0; i < num_results; ++i) {
        fail = true;
        for (j = 0; j < size; ++j)
          if (names[j] == result_names[i]) {
            fail = false;
            break;
          }
        if (fail)
          break;
      }
      if (!fail) {
        if (!suppress_successful)
          cout << "  result is expected" << endl;
        return;
      } else
        cout << id << "       returned objects do not match expected" << endl;
    } else
      cout << id << "       object returned more than once" << endl;
  } else
    printf("%f       number of objects do not match\n", id);
  for (i = 0; i < size; ++i)
    cout << "expected object[" << i << "] = " << names[i] << " ";
  cout << endl;
  for (i = 0; i < num_results; ++i)
    cout << "result object[" << i << "] = " << result_names[i] << " ";
  cout << endl;
  cout << "*** TEST DID NOT SUCCEED. ***" << endl;
  ++failed_tests;
}

void
test_pts(float id, Am_Object obj, Am_Object names[], int size)
{
  Am_Object result_names[10];
  int num_results = 0;
  int i, j;

  Am_Part_Iterator iter = obj;
  iter.Start();
  while (!iter.Last()) {
    result_names[num_results++] = iter.Get();
    iter.Next();
  }
  if (num_results == size) {
    bool fail = false;
    for (i = 0; i < num_results - 1; ++i)
      for (j = i + 1; j < num_results; ++j)
        if (result_names[i] == result_names[j])
          fail = true;
    if (!fail) {
      for (i = 0; i < num_results; ++i) {
        fail = true;
        for (j = 0; j < size; ++j)
          if (names[j] == result_names[i]) {
            fail = false;
            break;
          }
        if (fail)
          break;
      }
      if (!fail) {
        if (!suppress_successful)
          printf("  result is expected\n");
        return;
      } else
        printf("%f       returned objects do not match expected\n", id);
    } else
      printf("%f       object returned more than once\n", id);
  } else
    printf("%f       number of objects do not match\n", id);
  for (i = 0; i < size; ++i)
    cout << "expected object[" << i << "] = " << names[i] << " ";
  cout << endl;
  for (i = 0; i < num_results; ++i)
    cout << "result object[" << i << "] = " << result_names[i] << " ";
  cout << endl;
  cout << "*** TEST DID NOT SUCCEED. ***" << endl;
  ++failed_tests;
}

Am_Object accessed;
Am_Object accessed1;

Am_Define_Formula(int, gvLeft) { return self.Get(LEFT); }

Am_Define_Formula(int, talking_gvminleftright)
{
  cout << "In Top formula." << endl;
  int left = self.Get(LEFT);
  int right = self.Get(RIGHT);
  cout << "Top Gets Left " << left << " Right " << right << "." << endl;
  if (left < right)
    return left;
  else
    return right;
}

Am_Define_Formula(int, talking_gvltop)
{
  cout << "In Left formula." << endl;
  int top = self.Get(TOP);
  cout << "Left Returns " << top << "." << endl;
  return top;
}

Am_Define_Formula(int, talking_gvrtop)
{
  cout << "In Right formula." << endl;
  int top = self.Get(TOP);
  cout << "Right Returns " << top << "." << endl;
  return top;
}

Am_Define_Formula(int, gvParLeft)
{
  Am_Object parent;
  parent = self.Get(PARENT);
  return parent.Get(LEFT);
}

Am_Value
gvCleft_proc(Am_Object &)
{
  Am_Value value = accessed.Get(LEFT, Am_OK_IF_NOT_THERE);
  return value;
}
Am_Formula gvCleft(gvCleft_proc, "gvCleft");

Am_Define_Formula(int, top) { return self.Get(TOP); }

Am_Define_Formula(int, top_plus_10) { return 10 + (int)top(/*cc,*/ self); }

int getLeft(Am_Object) { return accessed.Get(LEFT, Am_NO_DEPENDENCY); }

int gv1left(Am_Object) { return accessed1.Get(/*cc,*/ LEFT); }

Am_Define_Formula(int, g64) { return (int)self.Get(WIDTH) + 3; }

Am_Define_Formula(int, g90) { return (int)self.Get(WIDTH) + 10; }

Am_Define_Formula(int, g91) { return (int)self.Get(HEIGHT) + 5; }

Am_Define_Method(Am_Object_Method, void, objproc0, (Am_Object /* none */))
{
  puts("hello world");
}

Am_Define_Formula(int, ownerleft) { return self.Get_Owner().Get(LEFT); }

Am_Define_Formula(int, ownertop) { return self.Get_Owner().Get(TOP); }

Am_Define_Formula(int, parttop)
{
  Am_Value v = self.Get(SHADOW);
  if (v.type == Am_OBJECT) {
    Am_Object part = v;
    int i = part.Get(TOP);
    cout << " parttop for " << self << " found shadow obj " << part
         << " its TOP is " << i << endl
         << flush;
    return i;
  } else {
    cout << " parttop for " << self << " found shadow val " << v << endl
         << flush;
    return v;
  }
}

Am_Define_Formula(int, siblingheight)
{
  return self.Get_Sibling(TOP).Get(HEIGHT);
}

Am_Define_Formula(int, rightheight)
{
  return self.Get_Object(RIGHT).Get(HEIGHT);
}

int global_int = 10;

// we want int* but this is the only option ?
Am_Define_No_Self_Formula(Am_Ptr, retpint) { return (Am_Ptr)&global_int; }

Am_Define_No_Self_Formula(float, retflt) { return 7.8; }

Am_Define_No_Self_Formula(double, retdbl) { return 1004.123L; }

int data_test;

class Store_Data : public Am_Wrapper
{
  AM_WRAPPER_DATA_DECL(Store)
public:
  Store_Data(int in_datum) { datum = in_datum; }
  Store_Data(Store_Data *proto) { datum = proto->datum; }
  bool operator==(const Store_Data &test) const { return datum == test.datum; }

  void Print(ostream &out) const { out << "STORE Print=" << datum; }

  int datum;
};

AM_WRAPPER_DATA_IMPL(Store, (this))

Am_Define_No_Self_Formula(const char *, retstr)
{
  Store_Data *store =
      (Store_Data *)Am_Object_Advanced::Get_Context()->Get_Data();
  data_test = store->datum;
  return "hello";
}

Am_Define_No_Self_Formula(Am_Method_Wrapper *, retproc) { return objproc0; }

Am_Define_Formula(Am_Wrapper *, retobj) { return self; }

Am_Define_Formula(int, add_lr)
{
  return (int)self.Get(LEFT, Am_NO_DEPENDENCY) +
         (int)self.Get(RIGHT, Am_NO_DEPENDENCY);
}

////Am_Value_List* retlist (Am_Constraint_Context& cc, Am_Object self)
//Am_Wrapper* retlist (Am_Constraint_Context&, Am_Object)
Am_Define_No_Self_Formula(Am_Wrapper *, retlist)
{
  Am_Value_List list;
  list.Add(1).Add(2).Add(3);
  return list;
}

Am_Object change_test;

void
change_owner(Am_Object, Am_Object, Am_Object new_owner)
{
  change_test = new_owner;
}

const char *
integer_type_check(const Am_Value &value)
{
  if ((value.type == Am_INT) && (value.value.long_value >= 0))
    return nullptr;
  else
    return "Big Time Error";
}

Am_Define_Formula(Am_Value, multival)
{
  Am_Value value = self.Get(LEFT);
  return value;
}

Am_Define_Formula(int, flags_test_formula)
{
  Am_Value value1 = self.Get(WIDTH); //missing slot
  test_int(280.50, value1.type, Am_ZERO);
  value1 = self.Get(WIDTH, Am_OK_IF_NOT_THERE); //missing slot
  test_int(280.51, value1.type, Am_MISSING_SLOT);

  value1 = Am_No_Object.Get(LEFT);
  test_int(280.52, value1.type, Am_ZERO);
  value1 = Am_No_Object.Get(LEFT, Am_OK_IF_NOT_THERE);
  test_int(280.53, value1.type, Am_GET_ON_NULL_OBJECT);

  value1 = self.Get(HEIGHT); //invalid formula
  test_int(280.54, value1.type, Am_ZERO);
  value1 = self.Get(HEIGHT, Am_OK_IF_NOT_THERE); //invalid formula
  test_int(280.55, value1.type, Am_FORMULA_INVALID);

  return 30; //this value should be ignored because formula should be invalid
}

class Sub_Object : public Am_Object
{
public:
  Sub_Object() { data = nullptr; }
  Sub_Object(Am_Wrapper *in_data) { data = (Am_Object_Data *)in_data; }

  static Sub_Object Make(const char *name) // hides Am_Object::Create
  {
    Am_Object new_object = Am_Root_Object.Create(name);
    new_object.Add(LEFT, 42);
    return (Am_Wrapper *)new_object;
  }
  static Sub_Object Narrow(Am_Object object)
  {
    Am_Value value = object.Get(LEFT);
    if ((value.type == Am_INT) && ((int)value == 42))
      return (Sub_Object)(Am_Wrapper *)object; // two casts needed because
                                               // no constructor for Am_Object
    else {
      Am_Error("** All hell breaks loose\n");
      return (Sub_Object)(Am_Wrapper *)Am_No_Object;
    }
  }
  int My_Other_Method() { return Get(LEFT); }
};

void
create_object(Am_Object self)
{
  self.Set(LEFT, 20, Am_OK_IF_NOT_THERE);
}

class Am_Foo_Data;
class Am_Foo
{
  AM_WRAPPER_DECL(Am_Foo)
public:
  Am_Foo(int store);
  void Set(int store);
  int Get();
};

class Am_Foo_Data : public Am_Wrapper
{
  AM_WRAPPER_DATA_DECL(Am_Foo)
public:
  Am_Foo_Data(int in_store) { store = in_store; }
  void Set(int in_store) { store = in_store; }
  int Get() { return store; }
  bool operator==(const Am_Foo_Data &test_value) const { return test_value.store == store; }
private:
  int store;
};

AM_WRAPPER_DATA_IMPL(Am_Foo, (store))

AM_WRAPPER_IMPL(Am_Foo)

Am_Foo::Am_Foo(int store) { data = new Am_Foo_Data(store); }

void
Am_Foo::Set(int store)
{
  data = (Am_Foo_Data *)data->Make_Unique();
  data->Set(store);
}

int
Am_Foo::Get()
{
  return data->Get();
}

bool
test_web_create(const Am_Slot &slot)
{
  return slot.Get_Key() == LEFT;
}

void
test_web_init(const Am_Slot &slot, Am_Web_Init &init)
{
  Am_Object_Advanced obj = slot.Get_Owner();
  int value = slot.Get();
  int size = obj.Get(SHADOW);
  obj.Set(WIDTH, value + size);
  init.Note_Output(obj, WIDTH);
  init.Note_Input(obj, LEFT);
  init.Note_Input(obj, WIDTH);
  init.Note_Input(obj, SHADOW);
}

void
test_web_validate(Am_Web_Events &events)
{
  Am_Object self;
  Am_Slot slot;
  events.End();
  while (!events.First()) {
    slot = events.Get();
    if (slot.Get_Key() == SHADOW) {
      if (!self.Valid())
        self = slot.Get_Owner();
    } else
      break;
    slot = Am_Slot();
    events.Prev();
  }
  int size = 0;
  int value;
  if (slot.Valid()) {
    if (!self.Valid())
      self = slot.Get_Owner();
    size = self.Get(SHADOW);
    switch (slot.Get_Key()) {
    case LEFT:
      value = self.Get(LEFT);
      self.Set(WIDTH, value + size);
      break;
    case WIDTH:
      value = self.Get(WIDTH);
      self.Set(LEFT, value - size);
      break;
    default:
      Am_Error("** Web invalidated a bad constraint");
      break;
    }
  } else if (self.Valid()) {
    size = self.Get(SHADOW);
    value = self.Get(LEFT);
    self.Set(WIDTH, value + size);
  }
}

bool
bug_web_create(const Am_Slot &)
{
  return false;
}

void
bug_web_init(const Am_Slot &slot, Am_Web_Init &init)
{
  Am_Object_Advanced obj = slot.Get_Owner();
  init.Note_Input(obj, INPUT_A);
  init.Note_Input(obj, INPUT_B);
  init.Note_Output(obj, INPUT_B);
  init.Note_Output(obj, OUTPUT_A);
  init.Note_Output(obj, OUTPUT_B);
}

static void
bug_web_validate(Am_Web_Events &events)
{
  events.Start();
  Am_Slot slot = events.Get();
  Am_Object self = slot.Get_Owner();
  // int input_a = self.Get (INPUT_A);
  int input_b = self.Get(INPUT_B);
  int out_a = self.Get(OUTPUT_A, Am_NO_DEPENDENCY);
  int out_b = self.Get(OUTPUT_B, Am_NO_DEPENDENCY);

  //cout << "web constraint running (in_a=" << input_a << ", in_b=" << input_b
  //     << ", A=" << out_a << ", B=" << out_b << ")" << endl;

  for (; !events.Last(); events.Next()) {
    slot = events.Get();
    Am_Slot_Key key = slot.Get_Key();
    if (key == INPUT_A || key == INPUT_B) {
      ++out_a;
      ++out_b;
    }
  }

  //  cout << "web constraint done (in_a=" << input_a << ", in_b=" << input_b
  //       << ", A=" << out_a << ", B=" << out_b << ")" << endl;

  self.Set(INPUT_B, input_b);
  self.Set(OUTPUT_A, out_a);
  self.Set(OUTPUT_B, out_b);
}

void
bug_demon(Am_Slot first_invalidated)
{
  Am_Object web_holder = first_invalidated.Get_Owner().Get(OBJECT);
  int n = web_holder.Get(OUTPUT_B);
  web_holder.Set(INPUT_A, 1);
  n = web_holder.Get(INPUT_B);
  web_holder.Set(INPUT_B, 1);
}

enum Test_Enum_enum
{
  ONE,
  TWO,
  THREE
};
AM_DEFINE_ENUM_TYPE(Test_Enum, Test_Enum_enum)

class Test_Enum_Support : public Am_Type_Support
{
  void Print(ostream &os, const Am_Value &value) const
  {
    Test_Enum_enum actual = Test_Enum(value).value;
    switch (actual) {
    case ONE:
      os << "ONE";
      break;
    case TWO:
      os << "TWO";
      break;
    case THREE:
      os << "THREE";
      break;
    default:
      os << "ERROR";
      break;
    }
  }
} Test_Enum_Object;

AM_DEFINE_ENUM_TYPE_IMPL(Test_Enum, &Test_Enum_Object)

AM_DEFINE_ENUM_TYPE(Test_Support, Test_Enum_enum)

AM_DEFINE_ENUM_SUPPORT(Test_Support, "ONE TWO THREE")

int
main(int argc, char *[])
{
  Am_Initialize();

  if (argc > 1)
    suppress_successful = 0;

  // Be careful here, slot names may conflict
  // with builtin names -- ortalo
  Am_Register_Slot_Key(LEFT, "TEST_LEFT");
  Am_Register_Slot_Key(PARENT, "TEST_PARENT");
  Am_Register_Slot_Key(TOP, "TEST_TOP");
  Am_Register_Slot_Key(WIDTH, "TEST_WIDTH");
  Am_Register_Slot_Key(HEIGHT, "TEST_HEIGHT");
  Am_Register_Slot_Key(RIGHT, "TEST_RIGHT");
  Am_Register_Slot_Key(SHADOW, "TEST_SHADOW");

  Am_Object a;
  Am_Object b;
  Am_Object c;
  Am_Object d;
  Am_Object e;
  Am_Object_Advanced ad;
  Am_Constraint_Iterator ci;
  Am_Object ob0[4];
  int *pistore;
  Am_Object_Method prstore;
  Am_Value_List vl;
  Am_Value value;

  a = Am_Root_Object.Create("A");
  a.Add(LEFT, 35);
  a.Add(WIDTH, 0);
  a.Add(TOP, 0);
  ////  a->ps ();

  a = Am_Root_Object.Create("A");
  b = Am_Root_Object.Create("B").Add(LEFT, 0);
  test_int(1, b.Get(LEFT), 0);

  a = Am_Root_Object.Create("A").Add(LEFT, 10);
  b = a.Create("B").Set(LEFT, 20);
  c = b.Create("C");
  d = b.Create("D");
  e = d.Create("E");
  test_int(6.01, b.Get(LEFT), 20);
  test_int(6, c.Get(LEFT), 20);
  test_int(7, e.Get(LEFT), 20);
  b.Remove_Slot(LEFT);
  test_int(8, c.Get(LEFT), 10);
  test_int(9, e.Get(LEFT), 10);

  a = Am_Root_Object.Create("A").Add(LEFT, 10);
  b = a.Create("B");
  c = Am_Root_Object.Create("C").Add(PARENT, b).Add(LEFT, gvParLeft);
  test_int(10, c.Get(LEFT), 10);
  b.Remove_Slot(LEFT);
  test_int(11, c.Get(LEFT), 10);

  a = Am_Root_Object.Create("A").Add(LEFT, 10);
  b = a.Create("B").Set(LEFT, 20);
  c = b.Create("C");
  accessed = c;
  d = Am_Root_Object.Create("D").Add(LEFT, gvCleft);
  test_int(14, c.Get(LEFT), 20);
  test_int(15, d.Get(LEFT), 20);
  b.Remove_Slot(LEFT);
  test_int(16, c.Get(LEFT), 10);
  test_int(17, d.Get(LEFT), 10);

  a = Am_Root_Object.Create("A").Add(LEFT, 10);
  b = a.Create("B").Set(LEFT, 20);
  c = b.Create("C");
  d = b.Create("D");
  e = d.Create("");
  test_int(18, c.Get(LEFT), 20);
  test_int(19, e.Get(LEFT), 20);
  a.Remove_Slot(LEFT);
  test_int(20, c.Get(LEFT), 20);
  test_int(21, e.Get(LEFT), 20);

  a = Am_Root_Object.Create("A");
  accessed = a;
  a.Add(LEFT, 12).Add(TOP, gvCleft);
  test_int(22, a.Get(TOP), 12);
  a.Remove_Slot(LEFT);
  //value constraint depends on unavailable slot, so ret value should be NONE
  test_int(23, a.Get_Slot_Type(TOP), Am_NONE);

  b = Am_Root_Object.Create("B").Add(LEFT, 24);
  a = b.Create("A");
  accessed = a;
  a.Set(LEFT, 12).Add(TOP, gvCleft);
  test_int(24, a.Get(TOP), 12);
  a.Remove_Slot(LEFT);
  test_int(25, a.Get(TOP), 24);
  c = a.Create("C");
  test_int(25.1, c.Get(TOP), 24);
  ((Am_Object_Advanced &)c).Disinherit_Slot(TOP);
  c.Add(TOP, 5);
  test_int(25.2, c.Get(TOP), 5);
  a.Set(LEFT, 15);
  test_int(25.3, c.Get(TOP), 5);

  vl.Add(10).Add(11).Add(13);
  vl.Start();
  test_int(26, vl.Member(11), true);
  test_int(26.1, vl.Member(19), false);
  test_int(26.2, vl.Get_Type(), Am_INT);
  vl.Start();
  value = vl.Get();
  test_int(26.4, value, 10);
  a = Am_Root_Object.Create("A").Add(LEFT, vl);
  b = a.Create("B");
  c = b.Create("C");
  Am_Value_List vl2;
  vl2.Add(10).Add(11).Add(13);
  test_val(27, c.Get(LEFT), vl2);

  int i;
  Am_Value value1, value2;
  value1 = a;
  value2 = b;
  a.Set(LEFT, value1);
  a.Add(TOP, value2);
  for (i = 0; i < 20; i++) {
    value1 = a.Get(LEFT);
    value2 = a.Get(TOP);
    a.Set(TOP, value1);
    a.Set(LEFT, value2);
    value1 = 10;
    value2 = 10;
  }

  Am_Value_List vl3;
  vl3.Add(10).Add(11);
  test_int(27.1, vl2 == vl3, false);
  vl2.Make_Empty();
  test_int(27.2, vl2.Length(), 0);
  vl2.Start();
  test_int(27.3, vl2.Last(), true);
  vl2 = vl3;
  test_int(27.4, vl2 == vl3, true);
  vl2.Start();
  test_int(27.5, vl2.Member(11), true);
  vl2.Add("hello");
  vl2.Start();
  test_int(27.6, vl2.Member(11), true);
  vl3.Start();
  test_int(27.7, vl3.Member(11), true);
  vl2.Start();
  test_int(27.8, vl2.Member("hello"), true);
  vl3.Start();
  test_int(27.9, vl3.Member("hello"), false);

  vl3.Next();
  vl3.Next();
  test_int(27.101, vl3.Start_Member(10), true);
  vl3.Start();
  test_int(27.102, vl3.Nth(), 0);
  vl3.Next();
  test_int(27.103, vl3.Nth(), 1);

  test_int(27.110, vl3.Length(), 2);
  test_int(27.111, vl3.Pop(), 10);
  test_int(27.112, vl3.Length(), 1);
  test_int(27.113, vl3.Pop(), 11);
  test_int(27.114, vl3.Length(), 0);

  a = Am_Root_Object.Create("Object_A");
  cout << "list now " << vl3 << endl << flush;
  vl3.Push(a);
  cout << "after push(a) " << vl3 << endl << flush;
  e = vl3.Pop();
  cout << "after pop() " << vl3 << " popped is " << e << endl << flush;

  test_int(27.115, vl3.Length(), 0);
  test_int(27.116, e.Valid(), 1);
  test_int(27.117, e == a, 1);

  Am_Value_List vlassoc;
  vlassoc.Add(Am_Assoc(1, "10"))
      .Add(Am_Assoc(2, "20"))
      .Add(Am_Assoc("backwards-3", 3))
      .Add(Am_Assoc("a", a));
  cout << "Assoc list: " << vlassoc << endl << flush;
  test_int(27.201, vlassoc.Assoc_1("10"), 1);
  test_int(27.202, vlassoc.Assoc_2("backwards-3"), 3);
  value1 = vlassoc.Assoc_1(1); //should be null
  test_int(27.203, value1.Valid(), 0);

  a = Am_Root_Object.Create("A").Add(LEFT, 35).Add(WIDTH, 0).Add(RIGHT, 0);
  static Am_Slot_Key sl0[] = {LEFT, WIDTH, RIGHT, HEIGHT};
  test_slots(28, a, sl0, 3);
  a.Add(HEIGHT, 1);
  test_slots(29, a, sl0, 4);

  a = Am_Root_Object.Create("A").Add(HEIGHT, 1);
  static Am_Slot_Key sl1[] = {HEIGHT, TOP};
  test_slots(30, a, sl1, 1);

  b = Am_Root_Object.Create("B").Add(TOP, 8);
  a = b.Create("A").Add(HEIGHT, 1);
  test_slots(31, a, sl1, 2);
  test_int(32, a.Get(TOP), 8);
  test_slots(33, a, sl1, 2);

  a = Am_Root_Object.Create("A").Add(TOP, 12);
  b = a.Create("B");
  test_int(34, b.Get(TOP), 12);
  a.Set(TOP, 10);
  test_int(35, b.Get(TOP), 10);

  a = Am_Root_Object.Create("A");
  b = a.Create("B");
  ob0[0] = b;
  test_inst(36, a, ob0, 1);
  c = a.Create("C");
  ob0[1] = c;
  test_inst(37, a, ob0, 2);
  d = a.Create("D");
  ob0[2] = d;
  test_inst(38, a, ob0, 3);

  a = Am_Root_Object.Create("A");
  b = a.Create("B");
  test_int(39, a.Is_Instance_Of(a), true);
  test_int(40, b.Is_Instance_Of(a), true);
  test_int(41, b.Is_Instance_Of(b), true);
  test_int(42, a.Is_Instance_Of(b), false);

  a = Am_Root_Object.Create("").Add(TOP, 5);
  accessed = a;
  a.Add(LEFT, top);
  ad = (Am_Object_Advanced &)a;
  Am_Slot sa(ad.Get_Slot(LEFT));
  ci = sa;
  ci.Start();
  Am_Constraint *csa = ci.Get();
  b = a.Create("");
  accessed1 = b;
  ad = (Am_Object_Advanced &)b;
  Am_Slot sb(ad.Get_Slot(LEFT));
  Am_Constraint *csb = csa->Create(sa, sb);
  b.Set(LEFT, csb);
  c = Am_Root_Object.Create("").Add(TOP, 5);
  ad = (Am_Object_Advanced &)c;
  Am_Slot sc(ad.Get_Slot(LEFT));
  Am_Constraint *csc = csa->Create(sa, sc);
  c.Add(LEFT, csc);
  test_int(42.1, csb->Is_Instance_Of(csb), true);
  test_int(42.2, csb->Is_Instance_Of(csa), true);
  test_int(42.3, csc->Is_Instance_Of(csa), true);
  a.Set(LEFT, top_plus_10);
  test_int(42.4, a.Get(LEFT), 15);

  Am_Slot_Key Other = Am_Register_Slot_Name("OTHER");
  a = Am_Root_Object.Create("").Add(TOP, 34).Add(Other, 87);
  b = a.Create("");
  test_int(43.1, b.Is_Slot_Inherited(TOP), true);
  test_int(43, b.Get(TOP), 34);
  test_int(45.1, b.Get(Other), 87);
  test_obj(46, ((Am_Object_Advanced &)b).Get_Slot_Locale(TOP), a);
  test_int(46.5, b.Is_Slot_Inherited(Other), true);
  test_int(47.1, Am_Register_Slot_Name("OTHER"), Other);
  test_int(47.2, Am_Slot_Name_Exists("OTHER"), true);
  test_int(47.3, Am_Slot_Name_Exists("PIZZA"), false);

  a = Am_Root_Object.Create("").Add(TOP, 2).Add(LEFT, 14);
  b = a.Create("");
  c = b.Create("");
  test_int(50, c.Get(TOP), 2);
  a.Set(TOP, gvLeft);
  test_int(51, c.Get(TOP), 14);

  a = Am_Root_Object.Create("").Add(TOP, 2);
  b = a.Create("");
  c = b.Create("");
  test_int(52, c.Get(TOP), 2);

  // dovalues

  a = Am_Root_Object.Create("").Add(WIDTH, 23).Add(LEFT, g64);
  test_int(64, a.Get(LEFT), 26);

  a = Am_Root_Object.Create("").Add(WIDTH, 23).Add(LEFT, g64);
  ad = (Am_Object_Advanced &)a;
  test_int(65, a.Get(LEFT), 26);
  ci = ad.Get_Slot(LEFT);
  test_int(66, ci.Length(), 1);
  ci.Start();
  Am_Constraint_Tag ct = ci.Get_Tag();
  ad.Get_Slot(LEFT).Remove_Constraint(ct);
  test_int(68, a.Get(LEFT), 26);
  ci = ad.Get_Slot(LEFT);
  test_int(69, ci.Length(), 0);

  b = Am_Root_Object.Create("B").Add(WIDTH, 8).Add(HEIGHT, g90);
  a = b.Create("A").Add(TOP, 3).Set(HEIGHT, g91).Set(HEIGHT, 20);
  a.Set(TOP, 4);
  test_int(91, a.Get(TOP), 4);
  test_int(92, a.Get(WIDTH), 8);
  b.Set(WIDTH, 0);

  a = Am_Root_Object.Create("").Add(LEFT, 5);
  b = a.Create("");
  c = b.Create("");
  d = b.Create("").Set(LEFT, 10);
  test_int(200, c.Get(LEFT), 5);
  test_int(201, d.Get(LEFT), 10);

  int istore = 5;
  pistore = &istore;
  Test_Enum te = TWO;
  Test_Support ts = TWO;
  cout << "te equals " << te << " [TWO]" << endl;
  cout << "ts equals " << ts << " [TWO]" << endl;
  ts = THREE;
  cout << "ts equals " << ts << " [THREE]" << endl;
  Am_Type_Support *ws = Am_Find_Support(Test_Support::Type_ID());
  ts = ws->From_String("ONE");
  cout << "ts equals " << ts << " [ONE]" << endl;
  a = Am_Root_Object.Create("")
          .Add(LEFT, (Am_Ptr)pistore)
          .Add(HEIGHT, 1.5)
          .Add(TOP, (double)10.5L)
          .Add(WIDTH, "this is a test string")
          .Add(SHADOW, te)
          .Add(RIGHT, objproc0);
  pistore = (int *)(Am_Ptr)a.Get(LEFT);
  test_int(202, *pistore, 5);
  test_flt(203, a.Get(HEIGHT), 1.5);
  test_flt(204, a.Get(TOP), 10.5);
  Am_Value sval = "hello";
  //  Am_String sfoo = sval;
  Am_String sfoo = Am_String(sval);
  //  Am_String sfoo;
  //  sfoo = sval;
  //  Am_String string = a.Get (WIDTH);
  //  Am_String string = Am_String (a.Get (WIDTH));
  Am_String string;
  string = a.Get(WIDTH);
  test_int(205, string == "this is a test string", 1);
  //  string = a.Get (HEIGHT);  // Causes error.
  //  string = a.Get (16);      // Causes error.
  prstore = a.Get(RIGHT);
  test_int(206, prstore == objproc0, true);
  Am_Value_Type type = a.Get_Slot_Type(WIDTH);
  test_int(207, type, Am_STRING);
  Test_Enum te2 = a.Get(SHADOW);
  // Test_Enum_enum te3 = te2;
  test_int(208, te2 == TWO, true);

  a = Am_Root_Object.Create("Name");
  b = Am_Root_Object.Create("B");
  a.Add_Part(b);
  c = Am_Root_Object.Create("C");
  d = Am_Root_Object.Create("D");
  c.Add_Part(LEFT, d);
  a.Add_Part(RIGHT, c);
  e = a.Create("E");
//  test_int (210, strcmp (a.Get_Name (), "Name"), 0);
#ifdef DEBUG
  test_int(210, strcmp(Am_Get_Name_Of_Item(a), "Name"), 0);
#endif
  ob0[0] = e.Get_Object(RIGHT).Get_Object(LEFT);
  test_inst(211, d, ob0, 1);
  ob0[0] = b;
  ob0[1] = c;
  test_pts(212, a, ob0, 2);
  test_obj(213, b.Get_Sibling(RIGHT), c);
  test_obj(214, e.Get_Object(RIGHT).Get_Owner(), e);
  test_obj(215, d.Get_Owner(), c);
  a.Remove_Part(RIGHT);
  test_pts(216, a, ob0, 1);
  a.Add_Part(c);
  test_pts(217, a, ob0, 2);
  a.Destroy();
  b.Destroy();
  c.Destroy();
  d.Destroy();
  e.Destroy();

  a = Am_Root_Object.Create("A")
          .Add(LEFT, 5)
          .Add_Part(RIGHT, b = Am_Root_Object.Create("B")
                                   .Add(LEFT, ownerleft)
                                   .Add(HEIGHT, 20))
          .Add_Part(TOP, e = Am_Root_Object.Create("E").Add(HEIGHT, 30))
          .Add_Part(WIDTH,
                    d = Am_Root_Object.Create("D").Add(HEIGHT, siblingheight))
          .Add(HEIGHT, rightheight);
  c = Am_Root_Object.Create("C").Add(LEFT, 10);
  test_int(220, a.Get(HEIGHT), 20);
  test_int(221, b.Get(LEFT), 5);
  test_int(222, d.Get(HEIGHT), 30);
  e.Set(HEIGHT, 35);
  test_int(223, d.Get(HEIGHT), 35);
  a.Set(LEFT, 15);
  test_int(224, b.Get(LEFT), 15);
  b.Set(HEIGHT, 25);
  test_int(225, a.Get(HEIGHT), 25);
  a.Remove_Constraint(HEIGHT);
  b.Remove_From_Owner();
  c.Add_Part(RIGHT, b);
  test_int(227, b.Get(LEFT), 10);
  b = a.Create("");
  test_int(228, b.Get_Object(WIDTH).Get(HEIGHT), 35);
  e.Set(HEIGHT, 16);
  test_int(229, e.Get(HEIGHT), 16);
  test_int(230, b.Get_Object(WIDTH).Get(HEIGHT), 16);

  retstr.Set_Data(new Store_Data(32));
  a = Am_Root_Object.Create("A")
          .Add(LEFT, retpint)
          .Add(PARENT, retlist)
          .Add(HEIGHT, retflt)
          .Add(TOP, retdbl)
          .Add(WIDTH, retstr)
          .Add(RIGHT, retproc)
          .Add(SHADOW, retobj);
  pistore = (int *)(Am_Ptr)a.Get(LEFT);
  test_int(240, *pistore, 10);
  test_flt(241, a.Get(HEIGHT), 7.8);
  test_flt(242, a.Get(TOP), 1004.123);
  test_flt(243, a.Get(TOP), 1004.123);
  test_int(244, Am_String(a.Get(WIDTH)) == "hello", true);
  test_int(245, Am_String(a.Get(WIDTH)) == "hello", true);
  prstore = a.Get(RIGHT);
  test_int(246, prstore == objproc0, true);
  vl = Am_Value_List().Add(1).Add(2).Add(3);
  test_val(247, a.Get(PARENT), vl);
  test_int(248, ((Am_Object)a.Get(SHADOW)) == a, true);
  test_int(249, data_test, 32);
  a.Destroy();

  a = Am_Root_Object.Create("").Add(LEFT, 10).Add(WIDTH,
                                                  gvLeft.Multi_Constraint());
  b = a.Create("")
          .Add(TOP, 50)
          .Add(HEIGHT, 100)
          .Set(WIDTH, top.Multi_Constraint());

  test_int(250, b.Get(WIDTH), 50); // b's width is from its top invalidates
  a.Set(LEFT, 20);                 // a's width formula, should be inherited
                                   // by B then.
  test_int(251, b.Get(WIDTH), 20); // This should be a's width formula since
                                   // it's most recently invalidated.
  b.Set(TOP, 30);
  test_int(252, b.Get(WIDTH), 30);
  a.Set(LEFT, 25);
  b.Set(TOP, 35);
  test_int(253, b.Get(WIDTH), 35);
  a.Set(LEFT, 45); // again, invalidating a's width formula.
  test_int(254, b.Get(WIDTH), 45);
  b.Set(TOP, 55);
  test_int(255, b.Get(WIDTH), 55);
  b.Set(WIDTH, g91); // returns HEIGHT + 5
  test_int(256, b.Get(WIDTH), 105);
  b.Set(TOP, 40); // this should not invalidate since width should have a new
                  // formula now.
  test_int(257, b.Get(WIDTH), 105);
  a.Set(LEFT, 60); // again, invalidating a's width formula should go to B
                   // as well.
  test_int(258, b.Get(WIDTH), 60);

  a = Am_Root_Object.Create("proto").Add(LEFT, 10).Add(WIDTH, 50).Add_Part(
      SHADOW, d = Am_Root_Object.Create("").Add(LEFT, ownerleft));
  b = a.Create("A name").Set(WIDTH, 60).Add_Part(
      e = Am_Root_Object.Create("e"));
  c = b.Copy();
#ifdef DEBUG
  cout << "Name of copy of \"A name\" is \"" << Am_Get_Name_Of_Item(c)
       << "\" [proto_N]" << endl;
#endif
  test_int(261, c.Get(LEFT), 10);
  test_int(262, b.Get(LEFT), 10);
  a.Set(LEFT, 20);
  test_int(263, c.Get(LEFT), 20);
  test_int(264, b.Get(LEFT), 20);
  test_int(265, c.Get(WIDTH), 60);
  test_int(266, b.Get(WIDTH), 60);
  b.Set(WIDTH, 70);
  test_int(267, c.Get(WIDTH), 60);
  test_int(268, b.Get(WIDTH), 70);
  ob0[0] = b.Get_Object(SHADOW);
  ob0[1] = c.Get_Object(SHADOW);
  test_inst(269, d, ob0, 2);
  test_int(270, c.Get_Object(SHADOW).Get(LEFT), 20);
  Am_Part_Iterator pi = c;
  test_int(271, pi.Length(), 2);
  pi.Start();
  e = pi.Get();
  if (e.Get_Key() == SHADOW) {
    pi.Next();
    e = pi.Get();
  }
  test_int(272, e.Get_Key(), Am_NO_SLOT);

  // // Am_NULL_SLOT no longer used
  // void* dead = (Am_Ptr)Am_NULL_SLOT.Get ();
  // test_int (280, (int)dead, 0);
  // test_int (281, Am_NULL_SLOT.Get_Type (), Am_NONE);

  a = Am_Root_Object.Create("a1");
  value1 = a.Get(LEFT, Am_OK_IF_NOT_THERE);
  test_int(280.01, value1.type, Am_MISSING_SLOT);
  value1 = Am_No_Object.Get(LEFT, Am_OK_IF_NOT_THERE);
  test_int(280.02, value1.type, Am_GET_ON_NULL_OBJECT);
  value1 = a.Get(LEFT, Am_RETURN_ZERO_ON_ERROR);
  test_int(280.03, value1.type, Am_ZERO);
  i = value1;
  test_int(280.04, i, 0);
  bool bo = value1;
  test_int(280.05, (int)bo, 0);
  float f = value1;
  test_int(280.06, (int)f, 0);
  i = a.Get(LEFT, Am_RETURN_ZERO_ON_ERROR);
  test_int(280.07, i, 0);

  a.Add(LEFT, Am_No_Value);
  value1 = a.Get(LEFT);
  test_int(280.08, value1.type, Am_NONE);

  b = Am_Root_Object.Create("b1");
  a.Set(LEFT, b);
  c = a.Get_Object(LEFT);
  test_int(280.09, (int)(c == b), 1);
  c = a.Peek_Object(TOP);
  test_int(280.10, c.Valid(), 0);

  a.Add(HEIGHT, g90); //depends on WIDTH
  value1 = a.Peek(HEIGHT);
  test_int(280.11, value1.type, Am_FORMULA_INVALID);
  value1 = a.Peek(HEIGHT, Am_RETURN_ZERO_ON_ERROR);
  test_int(280.12, value1.type, Am_ZERO);

  a = Am_Root_Object.Create("a2");
  a.Add(LEFT, 20);
  a.Add(HEIGHT, g90); //depends on WIDTH, so invalid
  a.Add(TOP, flags_test_formula);
  value1 = a.Peek(TOP);
  test_int(280.13, value1.type, Am_FORMULA_INVALID);

  a = Am_Root_Object.Create("a3");
  a.Add(LEFT, 20).Set_Read_Only(LEFT, true);
  a.Set(LEFT, 30, Am_OVERRIDE_READ_ONLY);
  i = a.Get(LEFT);
  test_int(280.14, i, 30);

  //make sure constraints are re-evaluated when Remove_Slot
  a = Am_Root_Object.Create("a3");
  a.Add(TOP, 20);
  a.Add(LEFT, talking_gvltop);
  i = a.Get(LEFT);
  test_int(285.01, i, 20);
  a.Remove_Slot(TOP);
  a.Add(TOP, 50);
  i = a.Get(LEFT);
  test_int(285.02, i, 50);
  a.Set(TOP, 51);
  i = a.Get(LEFT);
  test_int(285.03, i, 51);

  a.Remove_Slot(TOP);
  value1 = a.Peek(LEFT); //run the demons
  test_int(285.05, value1.type, Am_FORMULA_INVALID);
  a.Add(TOP, 70);
  i = a.Get(LEFT);
  test_int(285.06, i, 70);
  a.Set(TOP, 71);
  i = a.Get(LEFT);
  test_int(285.07, i, 71);

  a = Am_Root_Object.Create("");
  ad = (Am_Object_Advanced &)a;
  change_test = Am_No_Object;
  ad.Get_Demons().Set_Part_Demon(Am_CHANGE_OWNER, change_owner);
  b = a.Create("").Add_Part(a);
  Am_Wrapper *data = b;
  cout << "This object has no name \"" << data << "\" ->" << b << "<-" << endl;
  data->Release();
  ad.Get_Queue().Invoke();
  test_obj(290, change_test, b);
  a.Destroy();

  vl.Make_Empty();
  vl.Add("hello").Add(2).Add("world");
  vl.Start();
  test_int(300, vl.Member("world"), true);
  test_int(301, vl.Member(2), false);
  vl.Start();
  test_int(302, vl.Member(2), true);
  test_int(303, vl.Get_Type(), Am_INT);

  b = Am_Root_Object.Create("a").Add(LEFT, 10).Add(TOP, 15).Add_Part(
      SHADOW,
      Am_Root_Object.Create("shadow").Add(WIDTH, ownerleft.Multi_Constraint()));
  a = b.Create("b").Set(LEFT, 0).Set(TOP, 5);
  d = a.Get_Object(SHADOW);
  e = Am_Root_Object.Create("e").Add(LEFT, 10).Add(TOP, 15);
  test_obj(309, a.Get(SHADOW), d);
  test_obj(309.1, ((Am_Object_Advanced &)a).Get_Slot_Locale(SHADOW), a);
  test_int(309.2, a.Get_Slot_Type(SHADOW), Am_OBJECT);
  d.Set(WIDTH, ownertop);
  int hold = d.Get(WIDTH);
  if (hold == 0) {
    test_int(310, d.Get(WIDTH), 0);
    a.Set(TOP, 14);
    test_int(311, d.Get(WIDTH), 14);
  } else {
    test_int(310, d.Get(WIDTH), 5);
    a.Set(LEFT, 20);
    test_int(311, d.Get(WIDTH), 20);
  }
  d.Remove_From_Owner();
  e.Add_Part(SHADOW, d);
  hold = d.Get(WIDTH);
  if (hold == 10) {
    test_int(312, d.Get(WIDTH), 10);
    e.Set(TOP, 25);
    test_int(313, d.Get(WIDTH), 25);
  } else {
    test_int(312, d.Get(WIDTH), 15);
    e.Set(LEFT, 25);
    test_int(313, d.Get(WIDTH), 25);
  }

  a = Am_Root_Object.Create("a").Add(LEFT, 0).Add(TOP, gvLeft);
  ((Am_Object_Advanced &)a).Get_Demons().Set_Type_Check(1, integer_type_check);
  ((Am_Object_Advanced &)a).Get_Slot(LEFT).Set_Type_Check(1);
  b = a.Create("b").Set(LEFT, 5);
  // b.Set (LEFT, "a"); // Results in "Big Time Error"
  test_int(320, a.Get(TOP), 0);
  test_int(321, b.Get(TOP), 5);
  a.Remove_Constraint(TOP);
  test_int(322, a.Get(TOP), 0);
  test_int(323, b.Get(TOP), 5);
  a.Set(LEFT, 10);
  b.Set(LEFT, 15);
  test_int(324, a.Get(TOP), 0);
  test_int(325, b.Get(TOP), 5);

  a = Am_Root_Object.Create("a").Add(LEFT, 0).Add(TOP, multival);
  cout << "This is the object named \"a\" ->" << a << "<-" << endl;
  test_int(330, a.Get(TOP), 0);
  a.Set(LEFT, "hello");
  test_int(331, Am_String(a.Get(TOP)) == "hello", true);
  value = a.Get(TOP);
  a.Add(WIDTH, value);
  test_int(332, Am_String(a.Get(WIDTH)) == "hello", true);
  value = a.Get(TOP);
  test_int(333, Am_String(value) == "hello", true);
  value = 72;
  test_int(334, value, 72);

  a = Am_Root_Object.Create("a").Add_Part(TOP, Sub_Object::Make("sub object"));
  test_int(340, Sub_Object::Narrow(a.Get_Object(TOP)).My_Other_Method(), 42);

  a = Am_Root_Object.Create("a")
          .Add(LEFT, 15)
          .Add_Part(TOP, Am_Root_Object.Create("top").Add(LEFT, ownerleft))
          .Add_Part(SHADOW,
                    Am_Root_Object.Create("shadow")
                        .Add(LEFT, ownerleft)
                        .Add_Part(SHADOW, Am_Root_Object.Create("shadow").Add(
                                              LEFT, ownerleft))
                        .Add_Part(TOP, Am_Root_Object.Create("top").Add(
                                           LEFT, ownerleft)));
  test_int(345, a.Get_Object(SHADOW).Get(LEFT), 15);
  ad = (Am_Object_Advanced &)a;
  Am_Demon_Set proto_set(ad.Get_Demons());
  Am_Demon_Set demon_set(proto_set.Copy());
  demon_set.Set_Object_Demon(Am_CREATE_OBJ, create_object);
  ad.Set_Demons(demon_set);
  b = a.Create("b");
#ifdef DEBUG
  Am_Wrapper *part = b.Get_Object(TOP);
  cout << "Instanced name of \"top\" is \"" << Am_Get_Name_Of_Item(part) << "\""
       << endl;
  part->Release();
#endif
  test_int(346, b.Get(LEFT), 20);
  test_obj(347, b.Get_Object(SHADOW).Get_Owner(), b);
  test_int(348, b.Get_Object(SHADOW).Get(LEFT), 20);
  test_int(349, b.Get_Object(TOP).Get(LEFT), 20);
  test_int(350, b.Get_Object(SHADOW).Get_Object(SHADOW).Get(LEFT), 20);
  test_int(351, b.Get_Object(SHADOW).Get_Object(TOP).Get(LEFT), 20);

  a = Am_Root_Object.Create("a").Add(LEFT, 0).Add(TOP, gvLeft);
  b = a.Create("b");
  test_int(360, b.Get(TOP), 0);

  Am_Foo_Data *foo = new Am_Foo_Data(12);
  Am_Foo_Data *boo = new Am_Foo_Data(10);
  Am_Foo old_foo(0);
  Am_Foo new_foo(1);
  test_int(370, *foo == *boo, false);
  test_int(371, *foo == *foo, true);
  a = Am_Root_Object.Create("a").Add(LEFT, foo).Add(TOP, old_foo);
  b = a.Create("b").Set(TOP, new_foo);
  foo = Am_Foo_Data::Narrow(b.Get(LEFT));
  test_int(379, foo->Get(), 12);
  boo->Release();
  foo->Release();

  a = Am_Root_Object.Create("a").Add(LEFT, retobj);
  b = a.Create("b");
  c = a.Copy();
  test_obj(380, a.Get(LEFT), a);
  test_obj(381, b.Get(LEFT), b);
  test_obj(383, c.Get(LEFT), c);
  a.Destroy();
  c.Destroy();

  a = Am_Root_Object.Create("a").Add(LEFT, 14).Add(TOP, add_lr).Add(RIGHT, 15);
  test_int(385, a.Get(TOP), 29);

  a = Am_Root_Object.Create("a").Add(LEFT, 5);
  ((Am_Object_Advanced &)a).Get_Slot(LEFT).Set_Inherit_Rule(Am_COPY);
  b = a.Create("b");
  c = b.Create("c");
  test_int(390, c.Get(LEFT), 5);
  b.Set(LEFT, 10);
  test_int(391, c.Get(LEFT), 5);

  a = Am_Root_Object.Create("a").Add(LEFT, 5);
  ((Am_Object_Advanced &)a).Get_Slot(LEFT).Set_Inherit_Rule(Am_COPY);
  b = a.Create("b");
  test_int(392, b.Get(LEFT), 5);
  a.Set(LEFT, 10);
  test_int(393, b.Get(LEFT), 5);

  a = Am_Root_Object.Create("a")
          .Add(LEFT, 60)
          .Add(RIGHT, 40)
          .Add(TOP, 50)
          .Add(SHADOW, 0)
          .Set(LEFT, talking_gvltop)
          .Set(RIGHT, talking_gvrtop)
          .Set(TOP, talking_gvminleftright);
  a.Get(SHADOW);
  a.Set(RIGHT, 30);
  cout << "Set Right to 30." << endl;
  a.Get(SHADOW);
  a.Set(LEFT, 40);
  cout << "Set Left to 40." << endl;
  a.Get(SHADOW);

  a = Am_Root_Object.Create("a").Add_Part(SHADOW,
                                          Am_Root_Object.Create("shadow"));
  b = a.Create("b");
  c = b.Get_Object(SHADOW);
  b.Remove_Part(SHADOW).Add(SHADOW, "hello");

  a = Am_Root_Object.Create("a").Add_Part(SHADOW,
                                          Am_Root_Object.Create("shadow"));
  b = a.Create("b");
  b.Remove_Part(SHADOW).Add(SHADOW, 5);
  test_int(400, b.Get(SHADOW), 5);

  cout << "shadow part" << endl;
  a = Am_Root_Object.Create("a")
          .Add_Part(SHADOW, Am_Root_Object.Create("shadow").Add(TOP, 5))
          .Add(TOP, parttop);
  test_int(410, a.Get(TOP), 5);
  cout << "removing shadow" << endl;
  a.Remove_Part(SHADOW);
  cout << "setting shadow = 10" << endl;
  a.Add(SHADOW, 10);
  cout << "should be val" << endl;
  test_int(411, a.Get(TOP), 10);
  cout << "removing slot" << endl;
  a.Remove_Slot(SHADOW);
  // a.Validate(); FIX: uncomment to reveal a bug
  cout << "adding part again" << endl;
  a.Add_Part(SHADOW, Am_Root_Object.Create("shadow").Add(TOP, 15));
  cout << "should be part" << endl;
  test_int(412, a.Get(TOP), 15);
  cout << "changing part" << endl;
  a.Set_Part(SHADOW, Am_Root_Object.Create("shadow").Add(TOP, 20));
  test_int(413, a.Get(TOP), 20);
  cout << "instance b" << endl;
  b = a.Create("b").Set_Part(SHADOW,
                             Am_Root_Object.Create("shadow").Add(TOP, 30));
  test_int(414, b.Get(TOP), 30);

  a = Am_Root_Object.Create("a").Add(TOP, 5).Add(LEFT, top);
  b = a.Create("b").Set(LEFT, 10);
  test_int(420, b.Get(LEFT), 10);
  b.Set(TOP, 15);
  test_int(421, b.Get(LEFT), 10);

  Am_Web web(test_web_create, test_web_init, test_web_validate);

  a = Am_Root_Object.Create("a").Add(LEFT, 0).Set(LEFT, web).Add(SHADOW, 1);

  test_int(430, a.Get(LEFT), 0);
  test_int(431, a.Get(WIDTH), 1);
  a.Set(LEFT, 14);
  test_int(432, a.Get(LEFT), 14);
  test_int(433, a.Get(WIDTH), 15);
  a.Set(LEFT, 10);
  a.Set(WIDTH, 25);
  a.Set(WIDTH, 20);
  test_int(434, a.Get(LEFT), 19);
  test_int(435, a.Get(WIDTH), 20);

  b = a.Create("b");
  test_int(436, b.Get(LEFT), 19);
  test_int(437, b.Get(WIDTH), 20);
  b.Set(LEFT, 10);
  test_int(438, b.Get(LEFT), 10);
  test_int(439, b.Get(WIDTH), 11);
  test_int(440, a.Get(LEFT), 19);
  test_int(441, a.Get(WIDTH), 20);
  b.Add(TOP, 6);
  b.Set(SHADOW, top);
  test_int(442, b.Get(LEFT), 10);
  test_int(443, b.Get(WIDTH), 16);
  b.Set(TOP, 5);
  test_int(444, b.Get(WIDTH), 15);

  a = Am_Root_Object.Create("a").Add(LEFT, 5).Set_Inherit_Rule(LEFT, Am_LOCAL);
  b = a.Create("b");
  test_int(450, b.Get_Slot_Type(LEFT), Am_MISSING_SLOT);
  b.Add(LEFT, 10);
  test_int(451, b.Get_Slot_Type(LEFT), Am_INT);
  c = b.Create("c");
  test_int(452, c.Get_Slot_Type(LEFT), Am_MISSING_SLOT);

  a = Am_Root_Object.Create("a").Add(TOP, multival); //depends on left
  b = a.Create("b");
  c = b.Create("c");
  test_int(460, a.Get_Slot_Type(TOP), Am_FORMULA_INVALID);
  test_int(461, c.Get_Slot_Type(TOP), Am_FORMULA_INVALID);
  a.Add(LEFT, 10);
  test_int(462, a.Get_Slot_Type(TOP), Am_INT);
  test_int(463, c.Get(TOP), 10);
  ((Am_Object_Advanced &)a).Get_Slot(LEFT).Set_Inherit_Rule(Am_STATIC);
  test_int(463, c.Get(TOP), 10);
  c.Set(LEFT, 5);
  test_int(464, a.Get(TOP), 5);
  ((Am_Object_Advanced &)a).Get_Slot(LEFT).Set_Inherit_Rule(Am_LOCAL);
  test_int(465, c.Get_Slot_Type(TOP), Am_FORMULA_INVALID);
  ((Am_Object_Advanced &)a).Get_Slot(LEFT).Set_Inherit_Rule(Am_INHERIT);
  b.Remove_Slot(LEFT);
  c.Remove_Slot(LEFT);
  test_int(466, c.Get(TOP), 5);
  c.Set(LEFT, 15); //slot was re-inherited from a
  test_int(467, a.Get(TOP), 5);
  test_int(468, c.Get(TOP), 15);
  ((Am_Object_Advanced &)a).Get_Slot(LEFT).Set_Inherit_Rule(Am_COPY);
  test_int(469, b.Get(TOP), 5);
  a.Set(LEFT, 20);
  test_int(470, b.Get(TOP), 5);
  ((Am_Object_Advanced &)a).Get_Slot(LEFT).Set_Inherit_Rule(Am_LOCAL);
  test_int(471, b.Get(TOP), 5);
  test_int(472, a.Get(TOP), 20);

  // set up the part objects (analogous to Am_Polygons) with web constraints
  Am_Web bug_web(bug_web_create, bug_web_init, bug_web_validate);
  a = Am_Root_Object.Create("web holder")
          .Add(INPUT_A, 0)
          .Add(INPUT_B, 0)
          .Add(OUTPUT_A, 0)
          .Add(OUTPUT_B, 1)
          .Set(OUTPUT_A, bug_web);

  b = Am_Root_Object.Create("group_object")
          .Add(BAD_DEMON_TRIGGER, 0)
          .Add(OBJECT, a);

  Am_Demon_Set demons;

  Am_Object_Advanced temp = (Am_Object_Advanced &)b;
  demons = temp.Get_Demons().Copy();
  demons.Set_Slot_Demon(0x20, bug_demon,
                        Am_DEMON_PER_OBJECT | Am_DEMON_ON_CHANGE);
  temp.Set_Demons(demons);
  Am_Slot slot(temp.Get_Slot(BAD_DEMON_TRIGGER));
  unsigned short prev_demon_bits = slot.Get_Demon_Bits();
  slot.Set_Demon_Bits(prev_demon_bits | 0x20);

  test_int(480, a.Get(INPUT_A), 0);
  test_int(481, a.Get(INPUT_B), 0);
  test_int(482, a.Get(OUTPUT_A), 0);
  test_int(483, a.Get(OUTPUT_B), 1);

  b.Set(BAD_DEMON_TRIGGER, 1);
  b.Get(BAD_DEMON_TRIGGER);

  test_int(490, a.Get(INPUT_A), 1);
  test_int(491, a.Get(INPUT_B), 1);
  test_int(492, a.Get(OUTPUT_A), 2);
  test_int(493, a.Get(OUTPUT_B), 3);

  {
    Am_Value_List l;
    l.Add(1).Add(2);
    a = Am_Root_Object.Create("a").Add(LEFT, l);
  }
  test_int(500, a.Is_Unique(LEFT), true);
  b = a.Create("b");
  vl = b.Get(LEFT);
  test_int(501, a.Is_Unique(LEFT), false);
  b.Make_Unique(LEFT);
  test_int(502, b.Is_Unique(LEFT), true);

#if 0 // FIX: uncomment to reveal a bug.
      // part slots need to be treated the same as normal slots
  a = Am_Root_Object.Create ("a")
    .Add (TOP, multival); //depends on left
  test_int (510, a.Get_Slot_Type (TOP), Am_FORMULA_INVALID);
  a.Add_Part (LEFT, b = Am_Root_Object.Create ("b"));
  test_obj (511, a.Get (TOP), b);
  b.Remove_From_Owner ();
  test_obj (512, a.Get (TOP), ); // should this be b, or Am_No_Object, or
                                 // FORMULA_INVALID?
  a.Set (LEFT, b);
  test_obj (512, a.Get (TOP), b);
#endif

  // tests for Pointer_Wrappers
  my_int ii;
  ii.my_int_val = 12;
  my_int *pii = &ii;
  float ff = 13.3;
  float *pff = &ff;
  Am_my_int ampint(pii);
  Am_float ampfloat(pff);
  value1 = ampint;
  value2 = (Am_my_int)pii;
  Am_Value value3 = Am_my_int(&ii); //alternative ways to do this
  test_int(600, value1.type == Am_my_int::Type_ID(), 1);
  test_int(601, value1 == value2, 1);
  test_int(602, value1 == value3, 1);
  value3 = ampfloat;
  test_int(603, value3.type == Am_float::Type_ID(), 1);
  test_int(604, value1 == value3, 0);
  cout << "Ptr Wrapper; int value = " << value1 << " float value = " << value3
       << endl;
  a = Am_Root_Object.Create("a").Add(TOP, ampint).Add(LEFT, (Am_Ptr)pii);
  value3 = a.Get(TOP);
  test_int(605, value3.type == Am_my_int::Type_ID(), 1);
  Am_my_int ampint2 = a.Get(TOP);
  test_int(606, ampint2 == ampint, 1);
  my_int *pii2 = Am_my_int(a.Get(TOP)).value;
  test_int(607, ampint == pii2, 1);
  test_int(608, pii2->my_int_val, 12);
  value3 = a.Get(LEFT);
  test_int(609, value3.type, Am_VOIDPTR);
  pii2 = (my_int *)(Am_Ptr)a.Get(LEFT);
  test_int(610, pii2 == pii, 1);

  //   pii2 = a.Get(TOP); correctly fails to compile
  //The following should crash with type errors, and they do
  //  ampfloat = a.Get(TOP);
  //  ampint2 = a.Get(LEFT);

  //test the printing and reading facilities

  cout << "1=";
  value1 = 1;
  value1.Print(cout);
  value1 = 3.2;
  cout << " 3.2=";
  value1.Print(cout);
  value1 = "A_Test";
  cout << " `A_Test'=";
  value1.Println();
  foo = new Am_Foo_Data(12);
  value1 = foo;
  cout << " Foo=" << value1;
  Store_Data *stored = new Store_Data(32);
  cout << " STORE(32)=";
  stored->Println();
  value1 = stored;
  cout << " again=";
  value1.Print(cout);

  value2 = Am_Value(LEFT, Am_SLOT_KEY_TYPE);
  cout << " slotkey(LEFT)=" << value2;
  value2 = Am_Value(value1.type, Am_TYPE_TYPE);
  cout << " type(Store)=" << value2;

  value1 = Am_Value_List();
  cout << " emptyVL=" << value1;
  value1 = Am_Value_List().Add("A").Add("B");
  cout << " VL(2)=" << value1;

  cout << endl << flush;

  if (failed_tests)
    printf("\n *** %d tests failed\n", failed_tests);
  else
    printf("\n +++ No failed tests\n");

#ifdef AMULET2_CONVERSION
  cout << "\nNow a warning to use Add not Set:" << endl;
  a = Am_Root_Object.Create("a").Set(TOP, 0);
  cout << "\nNow a warning to use Set not Add:" << endl;
  a.Add(TOP, 1);
#endif

#if 0
  // This probably does not work anymore -- ortalo
  Am_Root_Object.Destroy();
#endif
  Am_Cleanup();

  return 0;
}
