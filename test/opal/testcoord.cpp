/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <amulet.h>

/* ************************************************************************
 * This program is designed to test the Am_Translate_Coordinates function
 * 
 * - The top of "Second Window" is constrained to be the same as the top
 *   of the red rectangle, so "Second Window" will appear relative to the
 *   top of the screen as "Red Window" is to the top or "Main Window".
 * - The top of "Third Window" is constrained to be the same as the top
 *   of the red rectangle translated to Am_Screen coordinates, so the top
 *   of "Third Window" will appear aligned with "Red Window".
 * - The menu in "Invisible Window" should move along with the window
 *   even if it is moved while invisible
 * ************************************************************************/

Am_Object my_win, sub_win, inv_win;

Am_Define_No_Self_Formula(int, from_sub_win) { return sub_win.Get(Am_TOP); }

Am_Define_No_Self_Formula(int, trans_from_sub_win)
{
  int top, left;
  if (!Am_Translate_Coordinates(sub_win, 0, 0, Am_Screen, left, top)) {
    Am_Error("Could not translate coordinates of sub_win");
    return 0;
  }
  return top;
}

Am_Define_Method(Am_Object_Method, void, make_visible, (Am_Object))
{
  inv_win.Set(Am_VISIBLE, !(bool)inv_win.Get(Am_VISIBLE));
}

Am_Define_Method(Am_Object_Method, void, do_move, (Am_Object))
{
  inv_win.Set(Am_LEFT, (int)inv_win.Get(Am_LEFT) + 100);
  inv_win.Set(Am_TOP, (int)inv_win.Get(Am_TOP) + 100);
}

Am_Define_Method(Am_Object_Method, void, quit_method, (Am_Object))
{
  Am_Exit_Main_Event_Loop();
}

int
main(void)
{
  Am_Initialize();

  Am_Object my_win = Am_Window.Create("my_win")
                         .Set(Am_LEFT, 20)
                         .Set(Am_TOP, 50)
                         .Set(Am_WIDTH, 200)
                         .Set(Am_HEIGHT, 200)
                         .Set(Am_TITLE, "Main Window")
                         .Set(Am_ICON_TITLE, "Main Window")
                         .Add_Part(Am_One_Shot_Interactor.Create()
                                       .Set(Am_START_WHEN, "v")
                                       .Set(Am_DO_METHOD, make_visible))
                         .Add_Part(Am_One_Shot_Interactor.Create()
                                       .Set(Am_START_WHEN, "m")
                                       .Set(Am_DO_METHOD, do_move))
                         .Add_Part(Am_One_Shot_Interactor.Create()
                                       .Set(Am_START_WHEN, "q")
                                       .Set(Am_DO_METHOD, quit_method));

  sub_win = Am_Rectangle.Create("sub_win")
                .Set(Am_LEFT, 20)
                .Set(Am_TOP, 50)
                .Set(Am_WIDTH, 100)
                .Set(Am_HEIGHT, 100)
                .Set(Am_FILL_STYLE, Am_Red)
                .Add_Part(Am_Move_Grow_Interactor.Create("move red rectangle"));

  Am_Object second_win = Am_Window.Create("second_win")
                             .Set(Am_LEFT, 240)
                             .Set(Am_TOP, from_sub_win.Multi_Constraint())
                             .Set(Am_WIDTH, 100)
                             .Set(Am_HEIGHT, 100)
                             .Set(Am_TITLE, "Second Window")
                             .Set(Am_ICON_TITLE, "Second Window");

  Am_Object third_win = Am_Window.Create("third_win")
                            .Set(Am_LEFT, 360)
                            .Set(Am_TOP, trans_from_sub_win.Multi_Constraint())
                            .Set(Am_WIDTH, 100)
                            .Set(Am_HEIGHT, 100)
                            .Set(Am_ICON_TITLE, "Third Window")
                            .Set(Am_TITLE, "Third Window");

  inv_win =
      Am_Window.Create("inv_win")
          .Set(Am_LEFT, 480)
          .Set(Am_TOP, 50)
          .Set(Am_VISIBLE, false)
          .Set(Am_WIDTH, 100)
          .Set(Am_HEIGHT, 100)
          .Set(Am_ICON_TITLE, "Popup Window")
          .Set(Am_TITLE, "Popup Window")
          .Add_Part(Am_Menu_Bar.Create().Set(
              Am_ITEMS, Am_Value_List().Add(
                            Am_Command.Create()
                                .Set(Am_LABEL, "MENU")
                                .Set(Am_ITEMS, Am_Value_List().Add("Item1").Add(
                                                   "Item2")))));

  my_win.Add_Part(sub_win);
  Am_Screen.Add_Part(my_win)
      .Add_Part(second_win)
      .Add_Part(third_win)
      .Add_Part(inv_win);

  std::cout << "Move the red rectangle to move second and third windows."
            << std::endl;
  std::cout << "  The top of the second window should be positioned relative "
               "to screen"
            << std::endl;
  std::cout << "  as top of red rectangle is positioned to main window."
            << std::endl;
  std::cout << "Type v in main window to toggle visibility of popup window."
            << std::endl;
  std::cout
      << "Type m in main window to move popup 100 points to right and down."
      << std::endl;
  std::cout
      << "  Menu should open in the correct position when invisible window"
      << std::endl;
  std::cout << "  is moved while invisible." << std::endl;
  std::cout << "Type q in main window to quit" << std::endl;

  Am_Main_Event_Loop();
  Am_Cleanup();

  return 0;
}
