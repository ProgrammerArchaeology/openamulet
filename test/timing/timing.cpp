//  * ************************************************************************
//  *         The Amulet User Interface Development Environment              *
//  * ************************************************************************
//  * This code was written as part of the Amulet project at                 *
//  * Carnegie Mellon University, and has been placed in the public          *
//  * domain.  If you are using this code or any part of Amulet,             *
//  * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
//  * ************************************************************************

#include <iostream>
#include <time.h>

#include <amulet.h>
#include <amulet/opal_advanced.h> // for update_all
#include <amulet/gem.h>           // for pretend input

//#ifdef OA_VERSION -- do not work -- ortalo
#if 0
#include <amulet/univ_map_oa.hpp> // for data-structure test
typedef OpenAmulet::Map_Int2Str Am_Map_Int2Str;
typedef OpenAmulet::Map_Iterator_Int2Str Am_MapIterator_Int2Str;
#else
#include <amulet/univ_map.h> // for data-structure test
// Map int -> char*
AM_DECL_MAP(Int2Str, int, char *)
AM_IMPL_MAP(Int2Str, int, 0, char *, (0L))
#endif

//  This file contains code to time various functions in Amulet. It is used to
//  measure improvements and to make sure new features don't make Amulet
//  slower.

class timing_info
{
public:
  const char *name;
  unsigned long total_time;
  unsigned long cnt;
  float time_each;
};

int my_index = 0;
timing_info all_times[100];

void
print_summary()
{
  std::cout << "\n---------------------\nSummary (DEBUGGING "
#if defined DEBUG | defined _DEBUG
            << "on"
#else
            << "off"
#endif
            << "): \n";

  int i;
  for (i = 0; i < my_index; i++) {
    std::cout << i << ".  " << all_times[i].time_each << " msec per object for "
              << all_times[i].name << std::endl;
  }
  std::cout << std::flush;
}

// Two global Am_Time objects which store the time just before and after
// the test
Am_Time start_time;
Am_Time end_time;

// Get the actual time
void
start_timing()
{
  start_time = Am_Time::Now();
}

void
end_timing(const char *message, long cnt)
{
  // get the actual end time and calc the needed time
  end_time = Am_Time::Now();
  Am_Time total_time = end_time - start_time;

  // get the needed time in milli seconds
  unsigned long total_msec = total_time.Milliseconds();
  float time_each;

  // write the output seperated by comma, so that we can read it into excel etc.
  std::cout << my_index << "," << message << "," << total_msec;

  // do we have a cnt value given? This value indicates, that we have
  // tested the timing several times
  if (cnt > 1) {
    // if so, calc the needed time per action and extrapolate it action/s
    time_each = static_cast<float>(total_msec) / static_cast<float>(cnt);

    float persec = 1000.0 / time_each;
    std::cout << "," << cnt << "," << time_each << "," << persec;
  } else {
    time_each = total_msec;
  }

  std::cout << std::endl;

  // save the timing for overall report
  all_times[my_index].name = message;
  all_times[my_index].total_time = total_msec;
  all_times[my_index].cnt = cnt;
  all_times[my_index].time_each = time_each;

  // increment the index
  if (my_index < 100) {
    my_index++;
  } else {
    //		std:cout << "To much tests!" << std::endl;
  }
}

Am_Slot_Key NEW_SLOT = Am_Register_Slot_Name("NEW_SLOT");
Am_Slot_Key MY_LEFT = Am_Register_Slot_Name("MY_LEFT");
Am_Slot_Key NEW_SLOT_INH = Am_Register_Slot_Name("NEW_SLOT_INH");
Am_Slot_Key NEW_SLOT_INH2 = Am_Register_Slot_Name("NEW_SLOT_INH2");
Am_Object win;

Am_Define_Formula(int, int_constraint)
{
  // get a slot-value, convert it to int and add a one
  return (static_cast<int>(self.Get(NEW_SLOT)) + 1);
}

Am_Define_Style_Formula(toggle_color)
{
  // get a slot-value, convert it to bool and return a Am_Style
  if (static_cast<bool>(self.Get(Am_SELECTED)) == true) {
    return (Am_Red);
  } else {
    return (Am_Black);
  }
}

int meth_sum = 0;
Am_Define_Method(Am_Object_Method, void, small_method, (Am_Object /* obj */))
{
  meth_sum++;
}

const long NUM_OBJ_CREATE = 5000L;
const long NUM_OBJ_SET = 100000L;
const long NUM_UPDATE_RECT = 3000L;
const long NUM_UPDATE_RECT_OVER_OBJS = 100L;
const long NUM_PRETEND_PRESS = 2000L;
const long NUM_OBJ_DESTROY = 50L;
const long NUM_OBJ_DATA_STRUCT = 15000L;

Am_Drawonable *draw_for_win;
Am_Input_Event_Handlers *evh;
Am_Input_Event ev;

void
init_pretend_input(Am_Object win)
{
  // get the drawonable
  draw_for_win = reinterpret_cast<Am_Drawonable *>(
      static_cast<Am_Ptr>(win.Get(Am_DRAWONABLE)));

  // get the dispatch function
  draw_for_win->Get_Input_Dispatch_Functions(evh);

  // create an input-event by hand, hope ev.timestamp doesn't matter
  ev.input_char = Am_Input_Char(Am_LEFT_MOUSE, false, false, false,
                                Am_BUTTON_DOWN, Am_SINGLE_CLICK);
  ev.draw = draw_for_win;
}

void
pretend_input(Am_Button_Down down, int x, int y)
{
  ev.input_char.button_down = down;
  ev.x = x;
  ev.y = y;

  // activate input-event
  evh->Input_Event_Notify(draw_for_win, &ev);
}

void
time_object_creation()
{
  Am_Object objs[NUM_OBJ_CREATE];
  long i;
  int cnt;

  // create NUM_OBJ_CREATE objects from the Am_Root_Object
  start_timing();
  for (i = 0; i < NUM_OBJ_CREATE; i++) {
    objs[i] = Am_Root_Object.Create();
  }
  Am_Update_All();
  end_timing("create objects from Root", NUM_OBJ_CREATE);

  // destroy all these objects
  start_timing();
  for (i = 0; i < NUM_OBJ_CREATE; i++) {
    objs[i].Destroy();
  }
  Am_Update_All();
  end_timing("destroy objects from Root", NUM_OBJ_CREATE);

  // create NUM_OBJ_CREATE objects from Am_Root_Object once more
  start_timing();
  for (i = 0; i < NUM_OBJ_CREATE; i++) {
    objs[i] = Am_Root_Object.Create();
  }
  Am_Update_All();
  end_timing("create objects from Root 2nd time", NUM_OBJ_CREATE);

  // and destroy them all once more
  start_timing();
  for (i = 0; i < NUM_OBJ_CREATE; i++) {
    objs[i].Destroy();
  }
  Am_Update_All();
  end_timing("destroy objects from Root 2nd time", NUM_OBJ_CREATE);

  // Add two slots to the prototypes and create NUM_OBJ_CREATE rectangels
  Am_Graphical_Object.Add(NEW_SLOT_INH2, 2);
  Am_Rectangle.Add(NEW_SLOT_INH, 1);
  start_timing();
  for (i = 0; i < NUM_OBJ_CREATE; i++) {
    objs[i] = Am_Rectangle.Create();
  }
  Am_Update_All();
  end_timing("create objects from Am_Rectangle", NUM_OBJ_CREATE);

  // add a new slot object to the existing rectangle objects
  cnt = 1;
  start_timing();
  for (i = 0; i < NUM_OBJ_CREATE; i++) {
    objs[i].Add(NEW_SLOT, cnt++);
  }
  Am_Update_All();
  end_timing("add NEW_SLOT to rectangles", NUM_OBJ_CREATE);

  // Set/store a new value in the NEW_SLOT
  start_timing();
  for (i = 0; i < NUM_OBJ_CREATE; i++) {
    objs[i].Set(NEW_SLOT, cnt--);
  }
  Am_Update_All();
  end_timing("set NEW_SLOT to diff value", NUM_OBJ_CREATE);

  // create two objects and add a new slot to them
  Am_Object one_rect = Am_Rectangle.Create();
  Am_Object one_root = Am_Root_Object.Create();
  one_root.Add(MY_LEFT, -1);
  one_rect.Add(MY_LEFT, -1);

  // cnt == 1 at this point
  // set the same slot of the root object to a different value
  start_timing();
  for (i = 0; i < NUM_OBJ_SET; i++) {
    cnt++;
    if (cnt > 30000) {
      cnt = 0;
    }

    one_root.Set(MY_LEFT, cnt);
  }
  Am_Update_All();
  end_timing("set added MY_LEFT slot repeatedly in same root object",
             NUM_OBJ_SET);

  // set the same slot of the rectangle to a different value
  start_timing();
  for (i = 0; i < NUM_OBJ_SET; i++) {
    cnt++;
    if (cnt > 30000) {
      cnt = 0;
    }

    one_rect.Set(Am_LEFT, cnt);
  }
  Am_Update_All();
  end_timing("set existing Am_LEFT slot repeatedly in rectangle", NUM_OBJ_SET);

  // retrieve the Am_Value of a slot from each object
  Am_Value v;
  start_timing();
  for (i = 0; i < NUM_OBJ_CREATE; i++) {
    v = objs[i].Get(NEW_SLOT);
  }
  end_timing("get Am_Value of NEW_SLOT from all objects", NUM_OBJ_CREATE);

  // retrieve the slot from each object as int and sum them
  long sum = 0;
  start_timing();
  for (i = 0; i < NUM_OBJ_CREATE; i++) {
    sum = sum + static_cast<int>(objs[i].Get(NEW_SLOT));
  }
  end_timing("get NEW_SLOT as int from all objects and sum them",
             NUM_OBJ_CREATE);

  // add NEW_SLOT to rectangel
  one_rect.Add(NEW_SLOT, 15);
  sum = 0;
  start_timing();
  for (i = 0; i < NUM_OBJ_SET; i++) {
    sum = sum + static_cast<int>(one_rect.Get(NEW_SLOT));
  }
  end_timing("get NEW_SOT as int from same object and sum them", NUM_OBJ_SET);

  one_root.Add(NEW_SLOT, Am_Window);
  Am_Object o;
  start_timing();
  for (i = 0; i < NUM_OBJ_SET; i++) {
    o = one_root.Get(NEW_SLOT);
  }
  end_timing("get value as Am_Object", NUM_OBJ_SET);

  start_timing();
  int int2;
  for (i = 0; i < NUM_OBJ_SET; i++) {
    if (cnt++ > 30000)
      cnt = 0;
    one_rect.Set(MY_LEFT, cnt);
    int2 = one_rect.Get(MY_LEFT);
  }
  Am_Update_All();
  end_timing("set + get as int", NUM_OBJ_SET);

  start_timing();
  for (i = 0; i < NUM_OBJ_CREATE; i++) {
    //    if( i % 200 == 0 )
    //      std::cout << i << std::endl;
    objs[i].Add(MY_LEFT, int_constraint);
  }
  Am_Update_All();
  //  std::cout << "Am_Update_All completed" << std::endl;
  end_timing("install a constraint into MY_LEFT", NUM_OBJ_CREATE);

  one_rect.Get(MY_LEFT);
  start_timing();
  for (i = 0; i < NUM_OBJ_CREATE; i++) {
    if (cnt++ > 30000)
      cnt = 0;
    objs[i].Set(NEW_SLOT, cnt);
  }
  end_timing("set NEW_SLOT, invalidating all constraints", NUM_OBJ_CREATE);

  start_timing();
  one_rect.Get(MY_LEFT);
  end_timing("one get which validates all constraints", NUM_OBJ_CREATE);

  one_rect.Set(MY_LEFT, int_constraint);
  start_timing();
  for (i = 0; i < NUM_OBJ_SET; i++) {
    if (cnt++ > 30000)
      cnt = 0;
    one_rect.Set(NEW_SLOT, cnt);
    int2 = one_rect.Get(MY_LEFT);
  }
  Am_Update_All();
  end_timing("set NEW_SLOT, get(MY_LEFT) from constraint", NUM_OBJ_SET);

  sum = 0;
  start_timing();
  for (i = 0; i < NUM_OBJ_CREATE; i++) {
    sum = sum + (int)objs[i].Get(NEW_SLOT_INH);
  }
  Am_Update_All();
  end_timing("get value rect INHERITED one level", NUM_OBJ_CREATE);

  sum = 0;
  start_timing();
  for (i = 0; i < NUM_OBJ_CREATE; i++) {
    sum = sum + (int)objs[i].Get(NEW_SLOT_INH2);
  }
  Am_Update_All();
  end_timing("get value rect inherited TWO levels", NUM_OBJ_CREATE);

  Am_Object callme = Am_Rectangle.Create().Add(NEW_SLOT, small_method);
  Am_Object_Method meth;
  start_timing();
  for (i = 0; i < NUM_OBJ_SET; i++) {
    meth = callme.Get(NEW_SLOT);
    meth.Call(callme);
  }
  Am_Update_All();
  end_timing("get method and call it", NUM_OBJ_SET);

  Am_Object win;
  start_timing();
  win = Am_Window.Create()
            .Set(Am_TOP, 50)
            .Set(Am_LEFT, 50)
            .Set(Am_WIDTH, 400)
            .Set(Am_HEIGHT, 400);
  Am_Screen.Add_Part(win);
  Am_Update_All();
  end_timing("create window and update", 0);

  Am_Object new_rect = Am_Rectangle.Create()
                           .Set(Am_LEFT, 0)
                           .Set(Am_TOP, 112)
                           .Set(Am_WIDTH, 40)
                           .Set(Am_HEIGHT, 40)
                           .Set(Am_FILL_STYLE, Am_Red);
  win.Add_Part(new_rect);
  Am_Update_All();
  cnt = 0;
  start_timing();
  for (i = 0; i < NUM_UPDATE_RECT; i++) {
    if (cnt++ > 350)
      cnt = 0;
    new_rect.Set(Am_LEFT, cnt);
    Am_Update_All();
  }
  end_timing("update rect double-buffered", NUM_UPDATE_RECT);
  new_rect.Destroy();

  new_rect = Am_Button.Create().Set(Am_LEFT, 0).Set(Am_TOP, 112);
  win.Add_Part(new_rect);
  Am_Update_All();
  cnt = 0;
  start_timing();
  for (i = 0; i < NUM_UPDATE_RECT; i++) {
    if (cnt++ > 350)
      cnt = 0;
    new_rect.Set(Am_LEFT, cnt);
    Am_Update_All();
  }
  end_timing("update button double-buffered", NUM_UPDATE_RECT);
  new_rect.Destroy();

  static int tria[8] = {25, 50, 50, 100, 75, 50, 25, 50};
  Am_Point_List triangle(tria, 8);
  new_rect = Am_Polygon.Create("triangle")
                 .Set(Am_FILL_STYLE, Am_Red)
                 .Set(Am_POINT_LIST, triangle);
  win.Add_Part(new_rect);
  Am_Update_All();
  cnt = 0;
  start_timing();
  for (i = 0; i < NUM_UPDATE_RECT; i++) {
    if (cnt++ > 300)
      cnt = 0;
    new_rect.Set(Am_LEFT, cnt);
    Am_Update_All();
  }
  end_timing("update triangle double-buffered", NUM_UPDATE_RECT);
  new_rect.Destroy();

  //////////////////////// not double buffered
  Am_Object win2;
  win2 = Am_Window.Create()
             .Set(Am_TOP, 50)
             .Set(Am_LEFT, 470)
             .Set(Am_WIDTH, 400)
             .Set(Am_HEIGHT, 400)
             .Set(Am_DOUBLE_BUFFER, false);
  Am_Screen.Add_Part(win2);
  Am_Update_All();

  new_rect = Am_Rectangle.Create()
                 .Set(Am_LEFT, 0)
                 .Set(Am_TOP, 112)
                 .Set(Am_WIDTH, 40)
                 .Set(Am_HEIGHT, 40)
                 .Set(Am_FILL_STYLE, Am_Red);
  win2.Add_Part(new_rect);
  Am_Update_All();
  cnt = 0;
  start_timing();
  for (i = 0; i < NUM_UPDATE_RECT; i++) {
    if (cnt++ > 350)
      cnt = 0;
    new_rect.Set(Am_LEFT, cnt);
    Am_Update_All();
  }
  end_timing("update rect NOT double-buffered", NUM_UPDATE_RECT);
  new_rect.Destroy();

  new_rect = Am_Button.Create().Set(Am_LEFT, 0).Set(Am_TOP, 112);
  win2.Add_Part(new_rect);
  Am_Update_All();
  cnt = 0;
  start_timing();
  for (i = 0; i < NUM_UPDATE_RECT; i++) {
    if (cnt++ > 350)
      cnt = 0;
    new_rect.Set(Am_LEFT, cnt);
    Am_Update_All();
  }
  end_timing("update button NOT double-buffered", NUM_UPDATE_RECT);
  new_rect.Destroy();

  new_rect = Am_Polygon.Create("triangle")
                 .Set(Am_FILL_STYLE, Am_Red)
                 .Set(Am_POINT_LIST, triangle);
  win2.Add_Part(new_rect);
  Am_Update_All();
  cnt = 0;
  start_timing();
  for (i = 0; i < NUM_UPDATE_RECT; i++) {
    if (cnt++ > 300)
      cnt = 0;
    new_rect.Set(Am_LEFT, cnt);
    Am_Update_All();
  }
  end_timing("update triangle NOT double-buffered", NUM_UPDATE_RECT);

  start_timing();
  win2.Destroy();
  Am_Update_All();
  end_timing("destroy window containing polygon", 0);

  ///////////////////////////////// pretend input ////////////////
  init_pretend_input(win);
  start_timing();
  for (i = 0; i < NUM_PRETEND_PRESS; i++) {
    pretend_input(Am_BUTTON_DOWN, 10, 10);
    pretend_input(Am_BUTTON_UP, 10, 10);
  }
  Am_Update_All();
  end_timing("pretend DOWN+UP, no interactors", NUM_PRETEND_PRESS);

  new_rect = Am_Rectangle.Create()
                 .Set(Am_LEFT, 112)
                 .Set(Am_TOP, 112)
                 .Set(Am_WIDTH, 40)
                 .Set(Am_HEIGHT, 40)
                 .Set(Am_FILL_STYLE, Am_Red)
                 .Add_Part(Am_One_Shot_Interactor.Create());
  win.Add_Part(new_rect);
  Am_Update_All();
  start_timing();
  for (i = 0; i < NUM_PRETEND_PRESS; i++) {
    pretend_input(Am_BUTTON_DOWN, 113, 113);
    pretend_input(Am_BUTTON_UP, 113, 113);
  }
  Am_Update_All();
  end_timing("invoke one_shot_inter, no graphics", NUM_PRETEND_PRESS);

  new_rect.Set(Am_FILL_STYLE, toggle_color);
  Am_Update_All();
  start_timing();
  for (i = 0; i < NUM_PRETEND_PRESS; i++) {
    pretend_input(Am_BUTTON_DOWN, 113, 113);
    pretend_input(Am_BUTTON_UP, 113, 113);
    Am_Update_All();
  }
  end_timing("invoke one_shot_inter, toggle color", NUM_PRETEND_PRESS);
  new_rect.Destroy();

  new_rect = Am_Rectangle.Create()
                 .Set(Am_LEFT, 2)
                 .Set(Am_TOP, 112)
                 .Set(Am_WIDTH, 40)
                 .Set(Am_HEIGHT, 40)
                 .Set(Am_FILL_STYLE, Am_Green)
                 .Add_Part(Am_Move_Grow_Interactor.Create());
  win.Add_Part(new_rect);
  pretend_input(Am_BUTTON_DOWN, 3, 113);
  ev.input_char = Am_Input_Char(Am_MOUSE_MOVED, false, false, false, Am_NEITHER,
                                Am_SINGLE_CLICK);
  Am_Update_All();
  start_timing();
  cnt = 0;
  for (i = 0; i < NUM_PRETEND_PRESS; i++) {
    if (cnt++ > 350)
      cnt = 0;
    pretend_input(Am_NEITHER, cnt, 113);
    Am_Update_All();
  }
  end_timing("move_grow_inter", NUM_PRETEND_PRESS);
  pretend_input(Am_BUTTON_UP, cnt, 113);
  new_rect.Destroy();

  ///////////////////////////////// rects in window ////////////////
  cnt = 0;
  int y = 0;
  for (i = 0; i < NUM_OBJ_CREATE; i++) {
    objs[i].Set(Am_LEFT, cnt);
    objs[i].Set(Am_TOP, y);
    objs[i].Set(Am_FILL_STYLE, Am_Yellow);
    cnt += 5;
    if (cnt > 350) {
      cnt = 0;
      y += 5;
    }
  }
  start_timing();
  for (i = 0; i < NUM_OBJ_CREATE; i++) {
    win.Add_Part(objs[i]);
  }
  end_timing("Add_Part rect to window", NUM_OBJ_CREATE);
  start_timing();
  Am_Update_All();
  end_timing("Update Window with 5000 Rectangles", 0);

  new_rect = Am_Rectangle.Create()
                 .Set(Am_LEFT, 0)
                 .Set(Am_TOP, 112)
                 .Set(Am_WIDTH, 40)
                 .Set(Am_HEIGHT, 40)
                 .Set(Am_FILL_STYLE, Am_Red);
  win.Add_Part(new_rect);
  Am_Update_All();
  cnt = 0;
  start_timing();
  for (i = 0; i < NUM_UPDATE_RECT_OVER_OBJS; i++) {
    if (cnt++ > 350)
      cnt = 0;
    new_rect.Set(Am_LEFT, cnt);
    Am_Update_All();
  }
  end_timing("update rect over other rects", NUM_UPDATE_RECT_OVER_OBJS);

  start_timing();
  for (i = 0; i < NUM_OBJ_DESTROY; i++) {
    objs[i].Destroy();
  }
  Am_Update_All();
  end_timing("destroy objects while in window", NUM_OBJ_DESTROY);

  //destroy the rest of the objects
  win.Destroy();
  for (i = NUM_OBJ_DESTROY; i < NUM_OBJ_CREATE; i++) {
    objs[i].Destroy();
  }
}

void
time_data_structures()
{
  Am_Map_Int2Str mapInt2Str;
  OpenAmulet::Map_Int2Str oaMapInt2Str;

  // add 100000 randmon objects
  start_timing();
  for (int i = 0; i < NUM_OBJ_DATA_STRUCT; ++i) {
    mapInt2Str[i] = "abc";
  }
  end_timing("adding objects to Int2Str map", NUM_OBJ_DATA_STRUCT);

  // lookup random objects
  srand((unsigned)time(
      NULL)); // Seed the random-number generator with current time so that the numbers will be different every time we run.
  start_timing();
  for (int i = 0; i < NUM_OBJ_DATA_STRUCT; ++i) {
    mapInt2Str.GetAt(rand());
  }
  end_timing("searching objects in Int2Str map", NUM_OBJ_DATA_STRUCT);

  // test iterator
  Am_MapIterator_Int2Str mapIteratorInt2Str;
  mapIteratorInt2Str.Init(&mapInt2Str);
  long counter(0);

  start_timing();
  for (const char *value = mapIteratorInt2Str(); value;
       value = mapIteratorInt2Str()) {
    ++counter;
    int key = mapIteratorInt2Str.Key();
  }
  end_timing("iterating through all objects in Int2Str map",
             NUM_OBJ_DATA_STRUCT);

  // free the map and all objects
  start_timing();
  mapInt2Str.Clear();
  end_timing("deleting Int2Str map", NUM_OBJ_DATA_STRUCT);

  //
  // Open Amulet data-structures
  //

  // add 100000 randmon objects
  start_timing();
  for (int i = 0; i < NUM_OBJ_DATA_STRUCT; ++i) {
    oaMapInt2Str[i] = "abc";
  }
  end_timing("adding objects to OA Int2Str map", NUM_OBJ_DATA_STRUCT);

  // lookup random objects
  srand((unsigned)time(
      NULL)); // Seed the random-number generator with current time so that the numbers will be different every time we run.
  start_timing();
  for (int i = 0; i < NUM_OBJ_DATA_STRUCT; ++i) {
    oaMapInt2Str.GetAt(rand());
  }
  end_timing("searching objects in OA Int2Str map", NUM_OBJ_DATA_STRUCT);

  // test iterator
  OpenAmulet::Map_Iterator_Int2Str oaMapIteratorInt2Str(oaMapInt2Str);
  counter = 0;

  start_timing();
  for (const char *value = oaMapIteratorInt2Str(); value;
       value = oaMapIteratorInt2Str()) {
    ++counter;
    int key = oaMapIteratorInt2Str.Key();
  }
  end_timing("iterating through all objects in OA Int2Str map",
             NUM_OBJ_DATA_STRUCT);

  // free the map and all objects
  start_timing();
  oaMapInt2Str.Clear();
  end_timing("deleting OA Int2Str map", NUM_OBJ_DATA_STRUCT);

  return;
}

int
main()
{
#if defined DEBUG | defined _DEBUG
  std::cout << "Debugging is ON" << std::endl;
#else
  std::cout << "Debugging is OFF" << std::endl;
#endif

  std::cout << "Test #, Message, Total Time in msec, # of objects, team per "
               "object, objects per second"
            << std::endl;

  start_timing();
  Am_Initialize();
  Am_Update_All();
  end_timing("Initialize", 0);

  //	time_object_creation();

  time_data_structures();

  Am_Update_All();
  start_timing();
  Am_Cleanup();
  end_timing("Cleanup", 0);
  print_summary();

  return 0;
}
