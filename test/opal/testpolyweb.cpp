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

#include <math.h>
#include <stdlib.h>

using namespace std;
Am_Slot_Key NUM_POINTS = Am_Register_Slot_Name("NUM_POINTS");
//Am_Slot_Key CENTER_X = Am_Register_Slot_Name ("CENTER_X");
//Am_Slot_Key CENTER_Y = Am_Register_Slot_Name ("CENTER_Y");

Am_Object poly1;

Am_Define_Point_List_Formula(compute_point_list)
{
  int n = self.Get(NUM_POINTS);
  int left = self.Get(Am_LEFT);
  int top = self.Get(Am_TOP);
  int width = self.Get(Am_WIDTH);
  int height = self.Get(Am_HEIGHT);
  int cx = left + width / 2;
  int cy = top + height / 2;
  int r = width / 2;
  const float pi = 3.14159;
  float dtheta = 2 * pi / n;
  Am_Point_List pl;

  // visit n evenly spaced points on a circle
  for (int i = 0; i < n; ++i) {
    float theta = i * dtheta;
    pl.Add(cx - r * cos(theta), cy + r * sin(theta));
  }

  // finally, return the first point (theta=0)
  pl.Add(cx - r, cy);
  return pl;
}

void
print_help()
{
  cout << "Commands:" << endl;
  cout << "  q to quit" << endl;
}

int
random_delta()
{
  return (rand() % 20) - 10;
}

Am_Define_Method(Am_Object_Method, void, change_setting, (Am_Object self))
{
  Am_Object inter = self.Get_Owner();
  Am_Input_Char c = inter.Get(Am_START_CHAR);
  if (c == "q") {
    Am_Exit_Main_Event_Loop();
  } else if (c == "?") {
    print_help();
  } else
    print_help();
}

int
main()
{
  Am_Initialize();

  Am_Screen.Add_Part(
      Am_Window.Create("window")
          .Set(Am_LEFT, 100)
          .Set(Am_TOP, 300)
          .Set(Am_WIDTH, 220)
          .Set(Am_HEIGHT, 220)
          .Set(Am_TITLE, "Test Polygon Web Constraint")
          .Add_Part(poly1 = Am_Polygon.Create("poly1")
                                .Set(Am_FILL_STYLE, Am_Motif_Green)
                                .Set(Am_LINE_STYLE, Am_No_Style)
                                //.Set (Am_RADIUS, 100)
                                .Set(Am_WIDTH, 200)
                                .Set(Am_HEIGHT, 200)
                                .Add(NUM_POINTS, 8)
                                .Set(Am_POINT_LIST, compute_point_list))
          .Add_Part(Am_Move_Grow_Interactor.Create())
          .Add_Part(Am_One_Shot_Interactor.Create()
                        .Set(Am_START_WHEN, "ANY_KEYBOARD")
                        .Get_Object(Am_COMMAND)
                        .Set(Am_DO_METHOD, change_setting)
                        .Get_Owner()));

  print_help();
  Am_Main_Event_Loop();

  Am_Cleanup();
  return 0;
}
