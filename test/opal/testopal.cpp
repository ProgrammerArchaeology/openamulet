/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <am_inc.h>

#include <amulet/standard_slots.h>

#include <amulet/opal.h>
#include <amulet/opal_advanced.h>
#include <amulet/inter.h>
#include <amulet/debugger.h>

using namespace std;

Am_Slot_Key PART1 = Am_Register_Slot_Name("PART1");
Am_Slot_Key PART2 = Am_Register_Slot_Name("PART2");
Am_Slot_Key PART3 = Am_Register_Slot_Name("PART3");
Am_Slot_Key LINE1 = Am_Register_Slot_Name("LINE1");
Am_Slot_Key TRI1 = Am_Register_Slot_Name("TRI1");
Am_Slot_Key OOK = Am_Register_Slot_Name("OOK");
Am_Slot_Key EEK_SLOT = Am_Register_Slot_Name("EEK");
Am_Slot_Key KEY = Am_Register_Slot_Name("KEY");

void
print_help()
{
  cout << "0 Add/Remove Component (with movement)" << endl;
  cout << "1 Change Title" << endl;
  cout << "2 Create/Destroy Object" << endl;
  cout << "3 Make Object Visible/Invisible" << endl;
  cout << "4 Add/Remove Component (no movement)" << endl;
  cout << "5 Move an Object" << endl;
  cout << "6 Move a Subwindow" << endl;
  cout << "7 Make Subwindow Visible/Invisible" << endl;
  cout << "8 Resize a Subwindow" << endl;
  cout << "9 Resize Main Window" << endl;
  cout << "a Change Fillstyle of an Object" << endl;
  cout << "b Create/Destroy Second Window" << endl;
  cout << "c Remove/Add Window from Screen" << endl;
  cout << "d Copy/Destroy Third Window" << endl;
  cout << "e Change Z Order of Things in Group" << endl;
  cout << "f Change Z Order of Things in Window" << endl;
  cout << "g Change Icon Title" << endl;
  cout << "h Toggle Title Bar" << endl;
  cout << "i Toggle Iconic" << endl;
  cout << "j Change Background Color" << endl;
  cout << "k Toggle MIN width" << endl;
  cout << "l Toggle MAX width, and height" << endl;
  cout << "p Point in object test" << endl;
  cout << "X change line x1" << endl;
  cout << "Y change line x1 and y1" << endl;
  cout << "M move line left" << endl;
  cout << "W change line width" << endl;
  cout << "R reset line position" << endl;
  cout << "C add object to another object ** THIS IS SUPPOSED TO CRASH **\n";
  cout << "? Print List of Commands" << endl;
  cout << "q Quit" << endl;
}

Am_Define_Formula(int, rect_width)
{
  Am_Object owner = self.Get_Owner();
  if (owner)
    return (int)owner.Get(Am_WIDTH) - 20;
  else
    return 0;
}

Am_Define_Formula(int, rect_height)
{
  Am_Object owner = self.Get_Owner();
  if (owner)
    return (int)owner.Get(Am_HEIGHT) - 20;
  else
    return 0;
}

Am_Define_Style_Formula(rect_fill)
{
  if ((bool)self.Get(Am_SELECTED) || (bool)self.Get(Am_INTERIM_SELECTED))
    return Am_Black;
  else
    return Am_White;
}

Am_Object window;
Am_Object inter;
Am_Object second;
Am_Object third;
Am_Object sw;
Am_Object r1;
Am_Object t1;
Am_Object l1;
Am_Object r2;
Am_Object r4;
Am_Object g1;
Am_Object eek;
Am_Object screen2 = nullptr;

Am_Define_No_Self_Formula(int, at_sw_x) { return sw.Get(Am_LEFT); }
Am_Define_No_Self_Formula(int, at_sw_y) { return sw.Get(Am_TOP); }

Am_Define_Method(Am_Object_Method, void, change_setting, (Am_Object self))
{
  Am_Object inter = self.Get_Owner();
  Am_Input_Char c = inter.Get(Am_START_CHAR);
  if (c == "q") {
    if (screen2)
      screen2.Destroy();
    Am_Exit_Main_Event_Loop();
  } else if (c == "?") {
    print_help();
  } else if (c == "0") {
    static bool mode0 = false;
    if (mode0) {
      mode0 = false;
      g1.Set(Am_LEFT, 90).Set(Am_TOP, 90);
      window.Add_Part(g1);
      g1.Set(Am_LEFT, 140).Set(Am_TOP, 150);
    } else {
      mode0 = true;
      g1.Set(Am_LEFT, 100).Set(Am_TOP, 100);
      g1.Remove_From_Owner();
      g1.Set(Am_LEFT, 90).Set(Am_TOP, 90);
    }
  } else if (c == "1") {
    static bool mode1 = false;
    if (mode1) {
      mode1 = false;
      window.Set(Am_TITLE, "Test Opal");
    } else {
      mode1 = true;
      window.Set(Am_TITLE, "Hello World!");
    }
  } else if (c == "2") {
    static bool mode2 = false;
    if (mode2) {
      mode2 = false;
      g1.Add_Part(eek = Am_Rectangle.Create("eek")
                            .Set(Am_FILL_STYLE, Am_Blue)
                            .Set(Am_LEFT, 5)
                            .Set(Am_TOP, 5)
                            .Set(Am_WIDTH, 50)
                            .Set(Am_HEIGHT, 30));
    } else {
      mode2 = true;
      eek.Destroy();
    }
  } else if (c == "3") {
    static bool mode3 = false;
    if (mode3) {
      mode3 = false;
      r2.Set(Am_VISIBLE, true);
      r2.Set(Am_LEFT, 20).Set(Am_TOP, 20);
    } else {
      mode3 = true;
      r2.Set(Am_LEFT, 100).Set(Am_TOP, 100);
      r2.Set(Am_VISIBLE, false);
    }
  } else if (c == "4") {
    static bool mode4 = false;
    if (mode4) {
      mode4 = false;
      window.Add_Part(g1);
    } else {
      mode4 = true;
      g1.Remove_From_Owner();
    }
  } else if (c == "5") {
    if (!eek.Valid()) {
      cout << "** Can't do test 5 now because rectangle is gone; "
              "Use 2 to bring it back\n"
           << flush;
    } else {
      static bool mode5 = false;
      if (mode5) {
        mode5 = false;
        eek.Set(Am_LEFT, 5).Set(Am_TOP, 5);
      } else {
        mode5 = true;
        eek.Set(Am_LEFT, 10).Set(Am_TOP, 10);
      }
    }
  } else if (c == "6") {
    static bool mode6 = false;
    if (mode6) {
      mode6 = false;
      sw.Set(Am_TOP, 150);
    } else {
      mode6 = true;
      sw.Set(Am_TOP, 130);
    }
  } else if (c == "7") {
    static bool mode7 = false;
    if (mode7) {
      mode7 = false;
      sw.Set(Am_VISIBLE, true);
    } else {
      mode7 = true;
      sw.Set(Am_VISIBLE, false);
    }
  } else if (c == "8") {
    static bool mode8 = false;
    if (mode8) {
      mode8 = false;
      sw.Set(Am_WIDTH, 80);
    } else {
      mode8 = true;
      sw.Set(Am_WIDTH, 100);
    }
  } else if (c == "9") {
    static bool mode9 = false;
    if (mode9) {
      mode9 = false;
      window.Set(Am_WIDTH, 200);
    } else {
      mode9 = true;
      window.Set(Am_WIDTH, 250);
    }
  } else if (c == "a") {
    static bool modea = false;
    if (modea) {
      modea = false;
      r4.Set(Am_FILL_STYLE, Am_White);
    } else {
      modea = true;
      r4.Set(Am_FILL_STYLE, Am_Yellow);
    }
  } else if (c == "b") {
    static bool modeb = false;
    if (modeb) {
      modeb = false;
      second.Destroy();
    } else {
      modeb = true;
      second = window.Create("new window").Set(Am_LEFT, 375);
      int depth = second.Get_Object(PART1).Get(Am_OWNER_DEPTH);
      cout << "Depth of r1 = " << depth << " [-1]" << endl;
      Am_Screen.Add_Part(second);
    }
  } else if (c == "c") {
    static bool modec = false;
    if (modec) {
      modec = false;
      Am_Screen.Add_Part(window);
    } else {
      modec = true;
      window.Remove_From_Owner();
    }
  } else if (c == "d") {
    static bool moded = false;
    if (moded) {
      moded = false;
      third.Destroy();
    } else {
      moded = true;
      third = window.Copy().Set(Am_TOP, 455);
      Am_Screen.Add_Part(third);
    }
  } else if (c == "e") {
    static bool modee = false;
    if (modee) {
      modee = false;
      Am_To_Bottom(g1.Get_Object(OOK));
    } else {
      modee = true;
      Am_To_Top(g1.Get_Object(OOK));
    }
  } else if (c == "f") {
    static bool modef = false;
    if (modef) {
      modef = false;
      Am_To_Bottom(r1);
    } else {
      modef = true;
      Am_Move_Object(r1, r2, true);
    }
  } else if (c == "g") {
    static bool mode = false;
    mode = !mode;
    window.Set(Am_ICON_TITLE, mode ? "Hello World" : "Foo and Bar");
  } else if (c == "h") {
    static bool mode = false;
    mode = !mode;
    window.Set(Am_OMIT_TITLE_BAR, mode);
  } else if (c == "i") {
    static bool mode = false;
    mode = !mode;
    window.Set(Am_ICONIFIED, mode);
  } else if (c == "j") {
    static bool mode = false;
    window.Set(Am_FILL_STYLE, mode ? Am_Orange : Am_Blue);
    mode = !mode;
  } else if (c == "k") {
    static int mode = 0;
    switch (mode) {
    case 0:
      cout << "use min = false" << endl;
      window.Set(Am_MIN_WIDTH, 50);
      break;
    case 1:
      cout << "min width = 50" << endl;
      window.Set(Am_USE_MIN_WIDTH, true);
      break;
    case 2:
      cout << "min width = 70" << endl;
      window.Set(Am_MIN_WIDTH, 70);
      break;
    case 3:
      cout << "min width = false" << endl;
      window.Set(Am_USE_MIN_WIDTH, false);
      break;
    }
    mode = (mode + 1) % 4;
  } else if (c == "l") {
    static int mode = 0;
    switch (mode) {
    case 0:
      cout << "max width = false max height = 250" << endl;
      window.Set(Am_MAX_WIDTH, 200);
      window.Set(Am_MAX_HEIGHT, 250);
      window.Set(Am_USE_MAX_HEIGHT, true);
      break;
    case 1:
      cout << "max width = 200 max height = 250" << endl;
      window.Set(Am_USE_MAX_WIDTH, true);
      break;
    case 2:
      cout << "max width = 150 max height = false" << endl;
      window.Set(Am_MAX_WIDTH, 150);
      window.Set(Am_USE_MAX_HEIGHT, false);
      break;
    case 3:
      cout << "max width = false max height = false" << endl;
      window.Set(Am_USE_MAX_WIDTH, false);
      break;
    }
    mode = (mode + 1) % 4;
  } else if (c == "X") {
    int x1 = l1.Get(Am_X1);
    l1.Set(Am_X1, x1 + 5);
  } else if (c == "p") {
    int x = inter.Get(Am_FIRST_X);
    int y = inter.Get(Am_FIRST_Y);
    cout << "Checking point (" << x << "," << y << ")\n" << flush;
    Am_Object in_obj = Am_Point_In_Leaf(window, x, y, window);
    if (in_obj)
      cout << "   in object " << in_obj << endl << flush;
    else
      cout << " NOT in an object\n" << flush;
  } else if (c == "Y") {
    int x1 = l1.Get(Am_X1);
    int y1 = l1.Get(Am_Y1);
    l1.Set(Am_X1, x1 + 5);
    l1.Set(Am_Y1, y1 + 5);
  } else if (c == "M") {
    int left = l1.Get(Am_LEFT);
    // int top = l1.Get(Am_TOP);
    l1.Set(Am_LEFT, left + 5);
    // l1.Set(Am_TOP, top+5);
  } else if (c == "W") {
    int width = l1.Get(Am_WIDTH);
    //int height = l1.Get(Am_HEIGHT);
    l1.Set(Am_WIDTH, width + 5);
    //l1.Set(Am_HEIGHT, height+5);
  } else if (c == "R") {
    l1.Set(Am_X1, 70).Set(Am_Y1, 15).Set(Am_X2, 145).Set(Am_Y2, 145);
  } else if (c == "C") {
    cout << "** Adding object to a non-group obj. **THIS SHOULD CRASH**\n\n";
    Am_Object o2 = Am_Rectangle.Create();
    l1.Add_Part(o2);
  } else
    print_help();
}

Am_Define_Formula(int, win_parrot_left)
{
  Am_Object my_window;
  my_window = self.Get(Am_WINDOW);
  int x, y;
  Am_Translate_Coordinates(window, 0, 0, my_window, x, y);
  return x;
}

Am_Define_Formula(int, win_parrot_top)
{
  Am_Object my_window;
  my_window = self.Get(Am_WINDOW);
  int x, y;
  Am_Translate_Coordinates(window, 0, 0, my_window, x, y);
  return y;
}

Am_Define_No_Self_Formula(int, parrot_left) { return sw.Get(Am_LEFT); }

Am_Define_No_Self_Formula(int, parrot_top) { return sw.Get(Am_TOP); }

Am_Define_No_Self_Formula(int, parrot_width) { return sw.Get(Am_WIDTH); }

Am_Define_No_Self_Formula(int, parrot_height) { return sw.Get(Am_HEIGHT); }

int
main(int argc, char **argv)
{
  Am_Initialize();

  window = Am_Window.Create("window")
               .Set(Am_FILL_STYLE, Am_Red)
               .Set(Am_LEFT, 50)
               .Set(Am_TOP, 200)
               .Set(Am_WIDTH, 200)
               .Set(Am_HEIGHT, 220)
               .Set(Am_TITLE, "Test Opal");

  inter = Am_Choice_Interactor.Create("keyboard press");
  inter.Set(Am_START_WHEN, "ANY_KEYBOARD");
  inter.Set(Am_CONTINUOUS, 0);
  inter.Get_Object(Am_COMMAND).Set(Am_DO_METHOD, change_setting);

  window.Add_Part(KEY, inter);

  sw = Am_Window.Create("sub window")
           .Set(Am_FILL_STYLE, Am_Blue)
           .Set(Am_LEFT, 20)
           .Set(Am_TOP, 150)
           .Set(Am_WIDTH, 80)
           .Set(Am_HEIGHT, 60);

  r1 = Am_Rectangle.Create("r1")
           .Set(Am_FILL_STYLE, Am_Green)
           .Set(Am_LEFT, 10)
           .Set(Am_TOP, 10)
           .Set(Am_WIDTH, 180)
           .Set(Am_HEIGHT, 180)
           .Set(Am_WIDTH, rect_width)
           .Set(Am_HEIGHT, rect_height);

  static int tria[8] = {50, 100, 100, 200, 150, 100, 50, 100};
  Am_Point_List triangle(tria, 8);
  t1 = Am_Polygon.Create("triangle").Set(Am_POINT_LIST, triangle);

  Am_Style bevel(0, 0, 0, 30, Am_CAP_BUTT, Am_JOIN_BEVEL);
  r2 = Am_Rectangle.Create("r2")
           .Set(Am_LEFT, 20)
           .Set(Am_TOP, 20)
           .Set(Am_WIDTH, 120)
           .Set(Am_HEIGHT, 90)
           .Set(Am_LINE_STYLE, bevel)
           .Set(Am_FILL_STYLE, Am_Blue);

  Am_Object r3 = Am_Rectangle.Create("r3")
                     .Add(Am_SELECTED, false)
                     .Add(Am_INTERIM_SELECTED, false)
                     .Set(Am_LEFT, 20)
                     .Set(Am_TOP, 20)
                     .Set(Am_WIDTH, 120)
                     .Set(Am_HEIGHT, 90)
                     .Set(Am_FILL_STYLE, rect_fill);

  r4 = Am_Rectangle.Create("r4")
           .Set(Am_LEFT, 10)
           .Set(Am_TOP, 10)
           .Set(Am_WIDTH, 60)
           .Set(Am_HEIGHT, 40)
           .Set(Am_FILL_STYLE, Am_White);

  Am_Style thick_orange_line(0.75, 0.25, 0.0, 5);
  l1 = Am_Line.Create("line1")
           .Set(Am_LINE_STYLE, thick_orange_line)
           .Set(Am_X1, 70)
           .Set(Am_Y1, 15)
           .Set(Am_X2, 145)
           .Set(Am_Y2, 145);
  Am_Object l2 = l1.Create("line2")
                     .Set(Am_X1, at_sw_x)
                     .Set(Am_Y1, at_sw_y)
                     .Set(Am_X2, 145)
                     .Set(Am_Y2, 85);

  g1 = Am_Group.Create("g1")
           .Add_Part(OOK, Am_Rectangle.Create("ook")
                              .Set(Am_LEFT, 0)
                              .Set(Am_TOP, 0)
                              .Set(Am_WIDTH, 50)
                              .Set(Am_HEIGHT, 30)
                              .Set(Am_FILL_STYLE, Am_White))
           .Add_Part(EEK_SLOT, eek = Am_Rectangle.Create("eek")
                                         .Set(Am_FILL_STYLE, Am_Blue)
                                         .Set(Am_LEFT, 5)
                                         .Set(Am_TOP, 5)
                                         .Set(Am_WIDTH, 50)
                                         .Set(Am_HEIGHT, 30))
           .Set(Am_LEFT, 140)
           .Set(Am_TOP, 150)
           .Set(Am_WIDTH, 55)
           .Set(Am_HEIGHT, 35);

  Am_Screen.Add_Part(window);
  window.Add_Part(PART1, r1);
  window.Add_Part(r3);
  window.Add_Part(r2);
  window.Add_Part(LINE1, l1);
  window.Add_Part(l2);
  window.Add_Part(PART2, g1);
  window.Add_Part(PART3, sw);
  window.Add_Part(TRI1, t1);
  sw.Add_Part(r4);

  Am_Screen.Add_Part(
      Am_Window.Create("filter window")
          .Set(Am_TITLE, "Filter Window")
          .Set(Am_FILL_STYLE, Am_Blue)
          .Set(Am_TOP, 40)
          .Add_Part(Am_Group.Create("win parrot")
                        .Set(Am_LEFT, win_parrot_left)
                        .Set(Am_TOP, win_parrot_top)
                        .Set(Am_WIDTH, 500)
                        .Set(Am_HEIGHT, 500)
                        .Add_Part(Am_Rectangle.Create("sub parrot")
                                      .Set(Am_FILL_STYLE, Am_Red)
                                      .Set(Am_LINE_STYLE, (0L))
                                      .Set(Am_LEFT, parrot_left)
                                      .Set(Am_TOP, parrot_top)
                                      .Set(Am_WIDTH, parrot_width)
                                      .Set(Am_HEIGHT, parrot_height))));

  Am_Object object = Am_Point_In_Part(window, 90, 60, window);
  if (object)
    cout << "Part of Window at 90x60 is " << object << " [r2]" << endl;
  else
    cout << "No Part of Window at 60x180" << endl;
  object = Am_Point_In_Leaf(window, 160, 160, window);
  if (object)
    cout << "Leaf of Window at 160x160 is " << object << " [eek]" << endl;
  else
    cout << "No Leaf of Window at 160x160" << endl;
  if (Am_Point_In_Obj(r1, 80, 50, window))
    cout << "Window point 90x60 is in r1" << endl;
  else
    cout << "Window point 90x60 is not in r1" << endl;

  int x, y;
  Am_Translate_Coordinates(r2, 10, 10, r1, x, y);
  cout << "Coordinate (10x10) in r2 is coordinate (" << x << "x" << y
       << ") in r1 [20x20]" << endl;
  Am_Translate_Coordinates(r2, 10, 10, r3, x, y);
  cout << "Coordinate (10x10) in r2 is coordinate (" << x << "x" << y
       << ") in r3 [10x10]" << endl;
  Am_Translate_Coordinates(eek, 10, 10, r1, x, y);
  cout << "Coordinate (10x10) in eek is coordinate (" << x << "x" << y
       << ") in r1 [145x155]" << endl;
  Am_Translate_Coordinates(r4, 10, 10, r1, x, y);
  cout << "Coordinate (10x10) in r4 is coordinate (" << x << "x" << y
       << ") in r1 [30x160]" << endl;

  print_help();

  if (argc > 1) {
    screen2 = Am_Create_Screen(argv[1]);
    if (screen2)
      screen2.Add_Part(Am_Window.Create("other screen")
                           .Set(Am_WIDTH, 200)
                           .Set(Am_HEIGHT, 100)
                           .Set(Am_TITLE, "Another Screen")
                           .Set(Am_QUERY_POSITION, true)
                           .Add_Part(Am_Rectangle.Create("other rectangle")
                                         .Set(Am_LEFT, 50)
                                         .Set(Am_TOP, 25)
                                         .Set(Am_WIDTH, 100)
                                         .Set(Am_HEIGHT, 50)
                                         .Set(Am_FILL_STYLE, Am_Yellow)));
  }

  Am_Object foo_group =
      Am_Group.Create("foo").Add_Part(PART1, Am_Rectangle.Create("bar"));
  Am_Object noo_group =
      foo_group.Create("noo").Set_Part(PART1, Am_Rectangle.Create("bart"));

  Am_Main_Event_Loop();
  Am_Cleanup();

  return 0;
}
