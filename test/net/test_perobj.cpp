#include "amulet.h"

using namespace std;

Am_Object rect;

Am_Define_Method(Am_Object_Method, void, set_together_method,
                 (Am_Object cmdobj))
{
  cmdobj = cmdobj;

  cout << "===== start ===== set together\n";
  rect.Set(Am_TOP, 10);
  rect.Set(Am_LEFT, 10);

  cout << "=====  end  ===== set together\n" << flush;
}

int
main(int argc, char *argv[])
{
  char remote_host_data[100];
  char *remote_host;
  Am_Initialize();

  cout << argc << endl;
  if (argc != 2) {
    remote_host = remote_host_data;
    sprintf(remote_host, "basalt.amulet.cs.cmu.edu");
  } else
    remote_host = argv[1];

  Am_Network.Add(remote_host);

  Am_Object rect_proto =
      Am_Rectangle.Create("rect_proto")
          .Add(Am_SLOTS_TO_SAVE, Am_Value_List().Add(Am_TOP).Add(Am_LEFT));

  Am_Object my_win = Am_Window.Create("my_win");
  rect = rect_proto.Create("rect");

  my_win.Add_Part(rect);
  Am_Screen.Add_Part(my_win);

  Am_Object my_inter = Am_Move_Grow_Interactor.Create("my_inter");
  my_win.Add_Part(my_inter);

  Am_Object set_together_cmd = Am_Command.Create("set_together cmd")
                                   .Set(Am_LABEL, "Set Together")
                                   .Set(Am_DO_METHOD, set_together_method);

  Am_Object set_together_button = Am_Button.Create("set_together button")
                                      .Set(Am_LEFT, 20)
                                      .Set(Am_TOP, 50)
                                      .Set(Am_COMMAND, set_together_cmd);
  Am_Object quit_button = Am_Button.Create("quit_button")
                              .Set(Am_LEFT, 20)
                              .Set(Am_TOP, 140)
                              .Set(Am_COMMAND, Am_Quit_No_Ask_Command);

  my_win.Add_Part(set_together_button);
  my_win.Add_Part(quit_button);

  Am_Network.Link(rect, "rect");

  Am_Main_Event_Loop();
  Am_Cleanup();
  return 0;
}
