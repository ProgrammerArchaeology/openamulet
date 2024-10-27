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

#include <amulet.h>
#include <fstream>
#include <math.h>

void
save0(const char *filename)
{
  // create file for output
  std::ofstream out_file(filename, std::ios::out);
  if (!out_file) {
    std::cout << "open failed" << std::endl;
    return;
  }

  // reset internal reference counters etc. (should always be done for new stream)
  Am_Default_Load_Save_Context.Reset();

  // write some plain data objects to the stream
  Am_Default_Load_Save_Context.Save(out_file, Am_Value(5));
  Am_Default_Load_Save_Context.Save(out_file, Am_Value((long)4520982));
  Am_Default_Load_Save_Context.Save(out_file, Am_Value(true));
  Am_Default_Load_Save_Context.Save(out_file, Am_Value(false));
  Am_Default_Load_Save_Context.Save(out_file, Am_Value((float)54.234));
  Am_Default_Load_Save_Context.Save(out_file, Am_Value(123.344));
  Am_Default_Load_Save_Context.Save(out_file, Am_Value('a'));
  Am_Default_Load_Save_Context.Save(
      out_file, Am_Value("Hello this is a test to see what\n happens"));

  return;
}

// read-back the output of 'save0' and check that it's as expected
void
load0(const char *filename)
{
  // open the file
  std::ifstream in_file(filename, std::ios::in);
  if (!in_file) {
    std::cout << "open failed" << std::endl;
    return;
  }

  // reset internal reference counters etc. (should always be done for new stream)
  Am_Default_Load_Save_Context.Reset();

  // read-back with in same order
  int a = Am_Default_Load_Save_Context.Load(in_file);
  long b = Am_Default_Load_Save_Context.Load(in_file);
  bool c = Am_Default_Load_Save_Context.Load(in_file);
  bool d = Am_Default_Load_Save_Context.Load(in_file);
  float e = Am_Default_Load_Save_Context.Load(in_file);
  double f = Am_Default_Load_Save_Context.Load(in_file);
  char g = Am_Default_Load_Save_Context.Load(in_file);
  Am_String h = Am_Default_Load_Save_Context.Load(in_file);

  // now do the check, that we read back the correct values
  if (a == 5 && b == 4520982 && c && !d && (fabs(e - 54.234) < 0.0001) &&
      (fabs(f - 123.344) < 0.0001) && g == 'a' &&
      h == "Hello this is a test to see what\n happens") {
    // everything is ok
    std::cout << "Test 0 okay" << std::endl;
  } else {
    // ups, there is a problem
    std::cout << "Test 0 failed" << std::endl;
  }
}

void
save1(const char *filename)
{
  // open output-file
  std::ofstream out_file(filename, std::ios::out);
  if (!out_file) {
    std::cout << "open failed" << std::endl;
    return;
  }

  // reset internal reference counters etc. (should always be done for new stream)
  Am_Default_Load_Save_Context.Reset();

  // create an Am_Value list object and add some data to it
  Am_Value_List list;
  list.Add(5).Add("hello").Add(10);

  // save the list twice, the second save-taks should use references!
  Am_Default_Load_Save_Context.Save(out_file,
                                    Am_Value(list)); // will be checked as l1
  Am_Default_Load_Save_Context.Save(out_file,
                                    Am_Value(list)); // will be checked as l2

  // make a copy of the list
  Am_Value_List list2 = list;

  // and add the list-items two more times (to test referencing)
  list2.Add(list).Add(list2, Am_TAIL, false); // will be checked as l3 & l4

  // Save this list (which should now use references)
  Am_Default_Load_Save_Context.Save(out_file, Am_Value(list2));

  // Save an empty list
  Am_Default_Load_Save_Context.Save(out_file, Am_Value(Am_No_Value_List));

  return;
}

void
load1(const char *filename)
{
  // open input-file
  std::ifstream in_file(filename, std::ios::in);
  if (!in_file) {
    std::cout << "open failed" << std::endl;
    return;
  }

  // reset internal reference counters etc. (should always be done for new stream)
  Am_Default_Load_Save_Context.Reset();

  // read-in the different Am_Value_List objects
  Am_Value_List l1 = Am_Default_Load_Save_Context.Load(in_file);
  Am_Value_List l2 = Am_Default_Load_Save_Context.Load(in_file);
  Am_Value_List l3 = Am_Default_Load_Save_Context.Load(in_file);
  Am_Value_List l4 = Am_Default_Load_Save_Context.Load(in_file);

  if ((int)l1.Get_Nth(0) == 5 && Am_String(l1.Get_Nth(1)) == "hello" &&
      (int)l1.Get_Nth(2) == 10 && l1 == l2 && (int)l3.Get_Nth(0) == 5 &&
      Am_String(l3.Get_Nth(1)) == "hello" && (int)l3.Get_Nth(2) == 10 &&
      Am_Value_List(l3.Get_Nth(3)) == l1 &&
      Am_Value_List(l3.Get_Nth(4)) == l2 && l4 == Am_No_Value_List) {
    std::cout << "Test 1 okay" << std::endl;
  } else {
    std::cout << "Test 1 failed" << std::endl;
  }

  return;
}

Am_Object Proto_Rect, Proto_Line;

void
save2(const char *filename)
{
  std::ofstream out_file(filename, std::ios::out);
  if (!out_file) {
    std::cout << "open failed" << std::endl;
    return;
  }

  Am_Value_List obj_list;
  obj_list
      .Add(Proto_Rect.Create()
               .Set(Am_FILL_STYLE, Am_Red)
               .Set(Am_LEFT, 10)
               .Set(Am_TOP, 10))
      .Add(Proto_Rect.Create()
               .Set(Am_FILL_STYLE, Am_Red)
               .Set(Am_LEFT, 70)
               .Set(Am_TOP, 10))
      .Add(Proto_Rect.Create()
               .Set(Am_FILL_STYLE, Am_Blue)
               .Set(Am_LEFT, 10)
               .Set(Am_TOP, 70))
      .Add(Proto_Line.Create()
               .Set(Am_LINE_STYLE, Am_Blue)
               .Add(Am_X1, 70)
               .Add(Am_Y1, 70)
               .Add(Am_X2, 120)
               .Add(Am_Y2, 120));

  Am_Default_Load_Save_Context.Reset();
  Am_Default_Load_Save_Context.Save(out_file, Am_Value(obj_list));
}

void
load2(const char *filename)
{
  std::ifstream in_file(filename, std::ios::in);
  if (!in_file) {
    std::cout << "open failed - " << filename << std::endl;
    return;
  }

  Am_Default_Load_Save_Context.Reset();
  Am_Value_List list = Am_Default_Load_Save_Context.Load(in_file);
  Am_Object o0 = list.Get_Nth(0);
  Am_Object o1 = list.Get_Nth(1);
  Am_Object o2 = list.Get_Nth(2);
  Am_Object o3 = list.Get_Nth(3);
  if (list.Length() == 4 && o0.Is_Instance_Of(Proto_Rect) &&
      o1.Is_Instance_Of(Proto_Rect) && o2.Is_Instance_Of(Proto_Rect) &&
      o3.Is_Instance_Of(Proto_Line) &&
      Am_Style(o0.Get(Am_FILL_STYLE)) == Am_Red &&
      Am_Style(o1.Get(Am_FILL_STYLE)) == Am_Red &&
      Am_Style(o2.Get(Am_FILL_STYLE)) == Am_Blue &&
      Am_Style(o3.Get(Am_LINE_STYLE)) == Am_Blue &&
      (int)o0.Get(Am_LEFT) == 10 && (int)o0.Get(Am_TOP) == 10 &&
      (int)o1.Get(Am_LEFT) == 70 && (int)o1.Get(Am_TOP) == 10 &&
      (int)o2.Get(Am_LEFT) == 10 && (int)o2.Get(Am_TOP) == 70 &&
      (int)o3.Get(Am_X1) == 70 && (int)o3.Get(Am_Y1) == 70 &&
      (int)o3.Get(Am_X2) == 120 && (int)o3.Get(Am_Y2) == 120)
    std::cout << "Test 2 okay" << std::endl;
  else
    std::cout << "Test 2 failed" << std::endl;
}

void
load3(const char *filename)
{
  std::ifstream in_file(filename, std::ios::in);
  if (!in_file) {
    std::cout << "open failed - " << filename << std::endl;
    return;
  }

  Am_Default_Load_Save_Context.Reset();
  Am_Value value = Am_Default_Load_Save_Context.Load(in_file);
  if (!value.Valid())
    std::cout << "Test 3 okay" << std::endl;
  else
    std::cout << "Test 3 failed" << std::endl;
}

Am_Define_Method(Am_Load_Method, Am_Value, load_rect,
                 (std::istream & is, Am_Load_Save_Context &context))
{
  Am_Object object = Proto_Rect.Create();
  context.Recursive_Load_Ahead(object);
  object.Set(Am_FILL_STYLE, context.Load(is));
  object.Set(Am_LEFT, context.Load(is));
  object.Set(Am_TOP, context.Load(is));
  return Am_Value(object);
}

Am_Define_Method(Am_Load_Method, Am_Value, load_line,
                 (std::istream & is, Am_Load_Save_Context &context))
{
  Am_Object object = Proto_Line.Create();
  context.Recursive_Load_Ahead(object);
  object.Set(Am_LINE_STYLE, context.Load(is));
  object.Add(Am_X1, context.Load(is));
  object.Add(Am_Y1, context.Load(is));
  object.Add(Am_X2, context.Load(is));
  object.Add(Am_Y2, context.Load(is));
  return Am_Value(object);
}

Am_Define_Method(Am_Save_Object_Method, void, save_rect,
                 (std::ostream & os, Am_Load_Save_Context &context,
                  const Am_Object &object))
{
  context.Save_Type_Name(os, "Proto_Rect");
  context.Save(os, object.Get(Am_FILL_STYLE));
  context.Save(os, object.Get(Am_LEFT));
  context.Save(os, object.Get(Am_TOP));
}

Am_Define_Method(Am_Save_Object_Method, void, save_line,
                 (std::ostream & os, Am_Load_Save_Context &context,
                  const Am_Object &object))
{
  context.Save_Type_Name(os, "Proto_Line");
  context.Save(os, object.Get(Am_LINE_STYLE));
  context.Save(os, object.Get(Am_X1));
  context.Save(os, object.Get(Am_Y1));
  context.Save(os, object.Get(Am_X2));
  context.Save(os, object.Get(Am_Y2));
}

int
main(void)
{
  Am_Initialize();

  save0("amulet_test_file_0");
  load0("amulet_test_file_0");

  save1("amulet_test_file_1");
  load1("amulet_test_file_1");

  Proto_Rect = Am_Rectangle.Create("my_rect")
                   .Add(Am_SAVE_OBJECT_METHOD, save_rect)
                   .Set(Am_WIDTH, 50)
                   .Set(Am_HEIGHT, 50);

  Proto_Line =
      Am_Rectangle.Create("my_line").Add(Am_SAVE_OBJECT_METHOD, save_line);

  Am_Default_Load_Save_Context.Register_Prototype("RED", Am_Red);
  Am_Default_Load_Save_Context.Register_Prototype("BLUE", 2, Am_Blue);
  Am_Default_Load_Save_Context.Register_Loader("Proto_Rect", load_rect);
  Am_Default_Load_Save_Context.Register_Loader("Proto_Line", load_line);

  save2("amulet_test_file_2");
  load2("amulet_test_file_2");

  load3("testloadsave");

  Am_Do_Events();
  Am_Cleanup();

  return 0;
}
