/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <am_inc.h>

#include <amulet/gem.h>
#include <stdlib.h>

Am_Drawonable *root, *d1, *d2, *d3, *d4;

Am_Style black(0.0, 0.0, 0.0);
Am_Style green(0.0, 1.0, 0.0);

class testtrans_handlers : public Am_Input_Event_Handlers
{
public:
  void Iconify_Notify(Am_Drawonable *, bool) override {}
  void Frame_Resize_Notify(Am_Drawonable *, int, int, int, int) override {}
  void Destroy_Notify(Am_Drawonable *) override {}
  void Configure_Notify(Am_Drawonable *, int, int, int, int) override {}
  void Exposure_Notify(Am_Drawonable *, int, int, int, int) override {}
  void Input_Event_Notify(Am_Drawonable *d, Am_Input_Event *ev) override
  {

    std::cout << "My Input Event `" << ev->input_char << "' on "
              << d->Get_Title() << std::endl;

    int d1_x, d1_y, d2_x, d2_y, d3_x, d3_y, d4_x, d4_y, root_x, root_y;
    d1->Translate_Coordinates(ev->x, ev->y, d, d1_x, d1_y);
    d2->Translate_Coordinates(ev->x, ev->y, d, d2_x, d2_y);
    d3->Translate_Coordinates(ev->x, ev->y, d, d3_x, d3_y);
    d4->Translate_Coordinates(ev->x, ev->y, d, d4_x, d4_y);
    root->Translate_Coordinates(ev->x, ev->y, d, root_x, root_y);

    printf("  Which is at (%d, %d) w.r.t. D1\n", d1_x, d1_y);
    printf("          and (%d, %d) w.r.t. D2\n", d2_x, d2_y);
    printf("          and (%d, %d) w.r.t. D3\n", d3_x, d3_y);
    printf("          and (%d, %d) w.r.t. D4\n", d4_x, d4_y);
    printf("          and (%d, %d) w.r.t. root\n", root_x, root_y);

    if (ev->input_char == "q")
      exit(0);
  }
} Global_Handlers;

void
init_windows()
{
  root = Am_Drawonable::Get_Root_Drawonable();
  d1 = root->Create(850, 180, 220, 100, "D1");
  d2 = d1->Create(-15, 22, 175, 100, "D2");
  d3 = d2->Create(30, 18, 150, 100, "D3");
  d4 = root->Create(800, 320, 150, 100, "D4");

  d1->Set_Input_Dispatch_Functions(&Global_Handlers);
  d2->Set_Input_Dispatch_Functions(&Global_Handlers);
  d3->Set_Input_Dispatch_Functions(&Global_Handlers);
  d4->Set_Input_Dispatch_Functions(&Global_Handlers);
}

int
main()
{
  init_windows();

  printf("Send input events into the windows.  Send a 'q' to quit.\n");

  Am_Drawonable::Main_Loop();

  return 0;
}
