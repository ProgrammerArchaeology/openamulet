/* ************************************************************************ 

  *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

//  *
//  *      This test is just like testsubwins, except that it launches
//  *   the Main_Loop and uses event handlers.  The Expose event hander
//  *   redraws the text strings in the windows as necessary, and the
//  *   input event handler reparents windows whenever you click the
//  *   mouse in a Drawonable.
//  *      An important reason for the existence of this test file is to
//  *   check that this event-handler-oriented code and the xterm-oriented
//  *   code found in testsubwins are both supported by Gem.
//  *

// For exit()
#include <stdlib.h>

#include <am_inc.h>

#include <amulet/gem.h>

#define TESTLOOP_D1_LEFT 850

Am_Drawonable *root, *d1, *d2, *d3;

Am_Style black(0.0, 0.0, 0.0);
Am_Style green(0.0, 1.0, 0.0);
Am_Font font1(Am_FONT_FIXED, true, false, false, Am_FONT_VERY_LARGE);
Am_Font font2(Am_FONT_FIXED, false, false, false, Am_FONT_LARGE);
Am_Font font3;

int state_counter = 0;

void
print_state_instruction()
{
  switch (state_counter) {
  case 0:
    printf("Do LEFT_DOWN in window to make D3 a sibling of D2:\n");
    break;
  case 1:
    printf("Do LEFT_DOWN in window to put D3 back where it was:\n");
    break;
  case 2:
    printf("Do LEFT_DOWN in window to make D2 a top-level window:\n");
    break;
  case 3:
    printf("Do LEFT_DOWN in window to put D2 back where it was:\n");
    break;
  case 4:
    printf("Do LEFT_DOWN in window to exit:\n");
    break;
  }
  fflush(stdout);
}

void
redraw(Am_Drawonable *d)
{
  static const char *string1 = "Top-level window";
  static int string1_len = 16;
  static const char *string2 = "Child of top-level";
  static int string2_len = 18;
  static const char *string3 = "Child of child window";
  static int string3_len = 21;

  if (d == d1)
    d1->Draw_Text(black, string1, string1_len, font1, 5, 87);
  else if (d == d2)
    d2->Draw_Text(black, string2, string2_len, font2, 5, 7);
  else
    d3->Draw_Text(black, string3, string3_len, font3, 5, 7);
  d1->Flush_Output();
}

void
test_reparent1()
{

  switch (state_counter) {
  case 0:
    break;
  case 1:
    d3->Reparent(d1);
    break;
  case 2:
    d3->Reparent(d2);
    break;
  case 3:
    d2->Reparent(root);
    break;
  case 4:
    d2->Reparent(d1);
    break;
  case 5:
    exit(0);
    break;
  }
  d1->Flush_Output();
  print_state_instruction();
  state_counter = state_counter + 1;
}

class testloop_handlers : public Am_Input_Event_Handlers
{
  void Iconify_Notify(Am_Drawonable *, bool) override {}
  void Frame_Resize_Notify(Am_Drawonable *, int, int, int, int) override {}
  void Destroy_Notify(Am_Drawonable *) override {}
  void Configure_Notify(Am_Drawonable * /*d*/, int /*l*/, int /*t*/, int /*w*/,
                        int /*h*/) override
  {
    //    d->Set_Position(l, t);
    //    d->Set_Size(w,h);
  }
  void Exposure_Notify(Am_Drawonable *draw, int, int, int, int) override
  {
    redraw(draw);
  }
  void Input_Event_Notify(Am_Drawonable *, Am_Input_Event *ev) override
  {
    if (ev->input_char == "LEFT_DOWN")
      test_reparent1();
    else if (ev->input_char == "MIDDLE_DOWN")
      d2->Set_Position(0, 0);
    else if (ev->input_char == "RIGHT_DOWN") {
      static bool visible = true;
      if (visible == true)
        visible = false;
      else
        visible = true;
      d2->Set_Visible(visible);
    }
  }
};

void
init_windows(testloop_handlers &handlers)
{

  // Am_Debug_Print_Input_Events = 1;

  root = Am_Drawonable::Get_Root_Drawonable();
  d1 = root->Create(TESTLOOP_D1_LEFT, 180, 220, 180, "D1");
  d2 = d1->Create(-15, 100, 175, 100, "D2", "D2 Icon", true, false, Am_No_Style,
                  false, 1, 1, 0, 0,
                  // want a title-bar when placed at top-level
                  true);
  d3 = d2->Create(30, 18, 150, 100, "D3");

  d1->Set_Input_Dispatch_Functions(&handlers);
  d2->Set_Input_Dispatch_Functions(&handlers);
  d3->Set_Input_Dispatch_Functions(&handlers);

  d1->Flush_Output();
  d1->Process_Event(0UL);
}

int
main()
{
  Am_Debug_Print_Input_Events = 0;

  testloop_handlers handlers;
  init_windows(handlers);

  test_reparent1();
  Am_Drawonable::Main_Loop();

  return 0;
}
