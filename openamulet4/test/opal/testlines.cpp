#include <am_inc.h>

#include STANDARD_SLOTS__H

#include OPAL__H
#include INTER__H

using namespace std;

Am_Slot_Key PART1 = Am_Register_Slot_Name ("PART1");
Am_Slot_Key PART2 = Am_Register_Slot_Name ("PART2");
Am_Slot_Key PART3 = Am_Register_Slot_Name ("PART3");
Am_Slot_Key LINE1 = Am_Register_Slot_Name ("LINE1");

void print_help ()
{
  cout << "1 Switch Line" << endl;
  cout << "? Print List of Commands" << endl;
  cout << "q Quit" << endl;
}

Am_Object line;

Am_Define_Method(Am_Object_Method, void, change_setting, (Am_Object self))
{
  Am_Object inter = self.Get_Owner();
  Am_Input_Char c =inter.Get (Am_START_CHAR);
  if (c == "q") {
    Am_Exit_Main_Event_Loop ();
  }
  else if (c == "?") {
    print_help ();
  }
  else if (c == "0") {
    static bool toggle = false;
    line.Set (Am_X2, toggle ? 100 : 0);
    toggle = !toggle;
  }
  else if (c == "1") {
    static bool toggle = false;
    line.Set (Am_LEFT, toggle ? 0 : 50);
    toggle = !toggle;
  }
}

int main ()
{
  Am_Initialize ();

  line = Am_Line.Create ("the line")
    .Set (Am_X1, 0)
    .Set (Am_Y1, 0)
    .Set (Am_X2, 100)
    .Set (Am_Y2, 100)
    .Add (Am_FILL_STYLE, Am_Black)
    ;
  
  Am_Object group = Am_Group.Create ("group")
    .Set (Am_LEFT, 5)
    .Set (Am_TOP, 5)
    .Set (Am_WIDTH, 100)
    .Set (Am_HEIGHT, 100)
    .Add_Part (LINE1, line)
    ;

  Am_Object instance = group.Create ("instance")
    .Set (Am_LEFT, 5)
    .Set (Am_TOP, 110)
    ;

  Am_Object copy = group.Copy ()
    .Set (Am_LEFT, 110)
    .Set (Am_TOP, 5)
    ;
    
  Am_Object stander = group.Create ("stander")
    .Set (Am_LEFT, 110)
    .Set (Am_TOP, 110)
    ;
  Am_Object line_inst = stander.Get_Object (LINE1);
  line_inst.Set (Am_X1, 0);
  line_inst.Set (Am_Y1, 0);
  line_inst.Set (Am_X2, 100);
  line_inst.Set (Am_Y2, 100);

  Am_Object window = Am_Window.Create ("window")
    .Set (Am_FILL_STYLE, Am_Motif_Gray)
    .Set (Am_LEFT, 50)
    .Set (Am_TOP, 100)
    .Set (Am_WIDTH, 215)
    .Set (Am_HEIGHT, 215)
    .Set (Am_TITLE, "Test Line Inheritance")
    .Add_Part (group)
    .Add_Part (instance)
    .Add_Part (copy)
    .Add_Part (stander)
    .Add_Part (Am_Choice_Interactor.Create("keyboard press")
      .Set (Am_START_WHEN, "ANY_KEYBOARD")
      .Set (Am_CONTINUOUS, 0)
      .Get_Object (Am_COMMAND)
        .Set (Am_DO_METHOD, change_setting)
        .Get_Owner ()
      )
    ;
  Am_Screen.Add_Part (window);
    
  Am_Main_Event_Loop ();
  Am_Cleanup ();

  return 0;
}
