/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* Test functions for the animation interactor
   
   Designed and implemented by Brad Myers
*/

#include <am_inc.h>

#include <amulet/am_io.h>
#include <amulet/standard_slots.h>
#include <amulet/value_list.h>
#include <amulet/inter.h>
#include <amulet/opal.h>
#include <amulet/opal_advanced.h>
#include <amulet/formula.h>
#include <amulet/misc.h>
#include <amulet/anim.h>

Am_Define_Method(Am_Object_Method, void, my_do, (Am_Object cmd))
{
  std::cout << "\n+-+-+- Do method for Command " << cmd << std::endl
            << std::flush;
}

Am_Object window, rect1, rect2, rect3, rect4, anim1, anim2, anim3, anim4, anim5,
    anim6;

const char *pixmapfilename[] = {
    "images/eye1.gif",  "images/eye2.gif",  "images/eye3.gif",
    "images/eye4.gif",  "images/eye5.gif",  "images/eye6.gif",
    "images/eye13.gif", "images/eye14.gif", "images/eye15.gif",
    "images/eye16.gif", "images/eye17.gif", "images/eye18.gif"};

Am_Value_List pixmap_images, indices_right, indices_left;
Am_Object eye_icon, icon, anim7, anim8;

Am_Define_Value_List_Formula(which_way)
{
  int x_offset = self.Get_Object(Am_IMPLEMENTATION_CHILD).Get(Am_X_OFFSET);
  if (x_offset > 0)
    return indices_right;
  else
    return indices_left;
}

Am_Define_Formula(int, plus_or_minus)
{
  Am_Object inter = self.Get_Owner();
  Am_Input_Char ic = inter.Get(Am_START_CHAR);
  if (ic.code == Am_LEFT_MOUSE)
    return -8;
  return 8;
}

static void
init_pixmaps()
{
  Am_Image_Array icon;
  //  Am_Object window2;
  for (int i = 0; i < 12; i++) {
    const char *initialname = pixmapfilename[i];
    const char *pathname = Am_Merge_Pathname(initialname);
    std::cout << "Reading image " << pathname << std::endl << std::flush;
    icon = Am_Image_Array(pathname);
    pixmap_images.Add(icon);
    delete[] pathname;
    /*
    window2 = Am_Window.Create()
      .Set (Am_LEFT, 500 )
      .Set (Am_TOP, 50 + 30 * i)
      .Set (Am_WIDTH, Am_Width_Of_Parts)
      .Set (Am_HEIGHT, Am_Height_Of_Parts)
      .Add_Part(Am_Bitmap.Create()
		.Set (Am_IMAGE, icon))
      ;
    Am_Screen.Add_Part(window2);
    */
  }
  indices_right.Add(0).Add(1).Add(2).Add(3).Add(4).Add(5);
  indices_left.Add(6).Add(7).Add(8).Add(9).Add(10).Add(11);
  std::cout << "creating eye icon \n" << std::flush;
  eye_icon = Am_Bitmap.Create("Eye_Icon")
                 .Set(Am_LEFT, 5)
                 .Set(Am_TOP, 280)    // Am_Bottom_Is_Bottom_Of_Owner)
                 .Set(Am_IMAGE, icon) //iniiialize with some value
      ;
  std::cout << eye_icon << " being added to " << window << std::endl
            << std::flush;
  window.Add_Part(eye_icon);
  std::cout << "done" << window << std::endl << std::flush;
  Am_Object cmd, cmd2;
  eye_icon.Add_Part(
      anim7 =
          Am_Animation_Interactor.Create("anim7")
              .Set(Am_START_WHEN, true)
              //change every 30 times a second
              //.Set(Am_REPEAT_DELAY, Am_Time(33))
              .Set_Part(
                  Am_COMMAND,
                  cmd = Am_Animation_Object_In_Owner_Command.Create("move_eye")
                            .Set(Am_X_OFFSET, 4)
                            .Set(Am_Y_OFFSET, 0)
                            .Set(Am_ANIMATE_END_ACTION, Am_ANIMATION_BOUNCE)));
  cmd.Set(Am_IMPLEMENTATION_PARENT,
          Am_Animation_Through_List_Command.Create("change_eye_pict")
              .Set(Am_SLOT_TO_ANIMATE, Am_IMAGE)
              .Set(Am_VALUES_FOR_SLOT_TO_ANIMATE, pixmap_images)
              .Set(Am_DO_METHOD, my_do)
              .Set(Am_ANIMATE_END_ACTION, Am_ANIMATION_WRAP) //start back at beg
              .Set(Am_IMPLEMENTATION_CHILD, cmd)
              .Set(Am_INDICES_FOR_VALUES_FOR_SLOT_TO_ANIMATE, which_way));
  Am_Start_Interactor(anim7);
  eye_icon.Add_Part(
      anim8 =
          Am_Animation_Interactor.Create("anim8")
              .Set(Am_START_WHEN, "ANY_MOUSE_DOWN")
              .Set(Am_STOP_WHEN, "ANY_MOUSE_UP")
              .Set(Am_START_WHERE_TEST, true)
              .Set_Part(Am_COMMAND, cmd2 = Am_Animation_Object_In_Owner_Command
                                               .Create("move_eye_and_stop")
                                               .Set(Am_X_OFFSET, plus_or_minus)
                                               .Set(Am_Y_OFFSET, 0)
                                               .Set(Am_DO_METHOD, my_do)
                                               .Set(Am_ANIMATE_END_ACTION,
                                                    Am_ANIMATION_STOP)));
  cmd2.Set(
      Am_IMPLEMENTATION_PARENT,
      Am_Animation_Through_List_Command.Create("change_eye_pict2")
          .Set(Am_SLOT_TO_ANIMATE, Am_IMAGE)
          .Set(Am_VALUES_FOR_SLOT_TO_ANIMATE, pixmap_images)
          .Set(Am_ANIMATE_END_ACTION, Am_ANIMATION_WRAP) //start back at beg
          .Set(Am_DO_METHOD, my_do)
          .Set(Am_IMPLEMENTATION_CHILD, cmd2)
          .Set(Am_INDICES_FOR_VALUES_FOR_SLOT_TO_ANIMATE, which_way));
}

Am_Define_Method(Am_Object_Method, void, change_setting, (Am_Object cmd))
{
  Am_Object inter = cmd.Get_Owner();
  Am_Input_Char c = inter.Get(Am_START_CHAR);
  std::cout << "---- got " << c << std::endl;
  switch (c.As_Char()) {
  case 'q':
    Am_Exit_Main_Event_Loop();
    break;
  case 's':
    std::cout << "stopping all\n" << std::flush;
    Am_Abort_Interactor(anim1);
    Am_Abort_Interactor(anim2);
    Am_Abort_Interactor(anim3);
    Am_Abort_Interactor(anim4);
    Am_Abort_Interactor(anim5);
    Am_Abort_Interactor(anim6);
    Am_Abort_Interactor(anim7);
    Am_Abort_Interactor(anim8);
    break;
  case 'g':
    std::cout << " go--starting all\n" << std::flush;
    Am_Start_Interactor(anim1);
    Am_Start_Interactor(anim2);
    Am_Start_Interactor(anim3);
    Am_Start_Interactor(anim4);
    Am_Start_Interactor(anim5);
    Am_Start_Interactor(anim6);
    Am_Start_Interactor(anim7);
    break;
  case 'i': {
    std::cout << "index of picture to load [0..11]: " << std::flush;
    int i;
    std::cin >> i;
    Am_Image_Array icon = pixmap_images.Get_Nth(i);
    eye_icon.Set(Am_IMAGE, icon);
    std::cout << "Lists are: right = " << indices_right
              << "\n left = " << indices_left << std::endl
              << std::flush;
    break;
  }
  case 'l': {
    Am_Stop_Interactor(anim7);
    break;
  }
  case 'd':
    std::cout << " debugging all\n" << std::flush;
    Am_Set_Inter_Trace(Am_INTER_TRACE_ALL);
    break;
  case 'h':
    std::cout << " debugging SHORT\n" << std::flush;
    Am_Set_Inter_Trace(Am_INTER_TRACE_SHORT);
    break;
  case 'n':
    std::cout << " debugging none\n" << std::flush;
    Am_Set_Inter_Trace(Am_INTER_TRACE_NONE);
    break;
  case 'S':
    std::cout << " debugging setting only\n" << std::flush;
    Am_Set_Inter_Trace(Am_INTER_TRACE_NONE);
    Am_Set_Inter_Trace(Am_INTER_TRACE_SETTING);
    break;
  default:
    std::cout << "** Illegal, want:\n"
              << " q = quit.\n"
              << " s = stop all animations\n"
              << " g = go (start all animations\n"
              << " i = pick icon index\n"
              << " l = left, right controls of walker\n"
              << " Debugging:\n"
              << "    d=full-debug, h=short-debug, n=no-debug, S=setting only\n"
              << std::endl
              << std::flush;
    break;
  } // end switch
}

int
main()
{
  std::cout << "General initialize\n" << std::flush;
  Am_Initialize();

  window = Am_Window.Create("window")
               .Set(Am_TITLE, "Test Animation")
               .Set(Am_FILL_STYLE, Am_Yellow)
               .Set(Am_LEFT, 20)
#ifndef _MACINTOSH
               .Set(Am_TOP, 20)
#else
               .Set(Am_TOP, 50)
#endif
               .Set(Am_WIDTH, 500)
               .Set(Am_HEIGHT, 310)
               .Set(Am_DOUBLE_BUFFER, true)
               .Add_Part(rect1 = Am_Rectangle.Create("rect1")
                                     .Set(Am_LEFT, 10)
                                     .Set(Am_TOP, 10)
                                     .Set(Am_WIDTH, 40)
                                     .Set(Am_HEIGHT, 40)
                                     .Set(Am_FILL_STYLE, Am_Red))

               .Add_Part(rect2 = Am_Rectangle.Create("rect2")
                                     .Set(Am_LEFT, 60)
                                     .Set(Am_TOP, 10)
                                     .Set(Am_WIDTH, 40)
                                     .Set(Am_HEIGHT, 40)
                                     .Set(Am_FILL_STYLE, Am_Blue))
               .Add_Part(rect3 = Am_Rectangle.Create("rect3")
                                     .Set(Am_LEFT, 110)
                                     .Set(Am_TOP, 10)
                                     .Set(Am_WIDTH, 40)
                                     .Set(Am_HEIGHT, 40)
                                     .Set(Am_FILL_STYLE, Am_Green))
               .Add_Part(rect4 = Am_Rectangle.Create("rect4")
                                     .Set(Am_LEFT, 160)
                                     .Set(Am_TOP, 10)
                                     .Set(Am_WIDTH, 40)
                                     .Set(Am_HEIGHT, 40)
                                     .Set(Am_FILL_STYLE, Am_Orange));

  rect1.Add_Part(
      anim1 = Am_Animation_Interactor.Create("anim1")
                  .Set(Am_START_WHEN, true)
                  .Set_Part(Am_COMMAND,
                            Am_Incrementer_Animation_Command.Create().Set(
                                Am_SLOT_TO_ANIMATE, Am_HEIGHT)));

  rect2.Add_Part(
      anim2 =
          Am_Animation_Interactor.Create("anim2")
              .Set(Am_START_WHEN, true)
              .Set_Part(Am_COMMAND,
                        Am_Animation_Object_In_Owner_Command.Create()
                            .Set(Am_X_OFFSET, 2)
                            .Set(Am_Y_OFFSET, 3)
                            .Set(Am_ANIMATE_END_ACTION, Am_ANIMATION_BOUNCE)));

  rect3.Add_Part(
      anim3 =
          Am_Animation_Interactor.Create("anim3")
              .Set(Am_START_WHEN, true)
              .Set_Part(Am_COMMAND,
                        Am_Animation_Object_In_Owner_Command.Create()
                            .Set(Am_X_OFFSET, -4)
                            .Set(Am_Y_OFFSET, 4)
                            .Set(Am_ANIMATE_END_ACTION, Am_ANIMATION_WRAP)));

  rect4.Add_Part(
      anim4 = Am_Animation_Interactor.Create("anim4").Set_Part(
          Am_COMMAND,
          Am_Animator_Animation_Command.Create()
              .Set(Am_DO_METHOD, my_do)
              .Set(Am_SLOT_TO_ANIMATE, Am_Value_List().Add(Am_LEFT).Add(Am_TOP))
              .Set(Am_VALUE_1, Am_Value_List().Add(160).Add(10))
              .Set(Am_VALUE_2, Am_Value_List().Add(420).Add(260))
              .Set(Am_TIME_FOR_ANIMATION, Am_Time(10000)) //milliseconds
          ));
  rect1.Add_Part(
      anim5 =
          Am_Animation_Interactor
              .Create("anim5")
              //change every 1/2 second
              .Set(Am_REPEAT_DELAY, Am_Time(500))
              .Set_Part(Am_COMMAND, Am_Animation_Through_List_Command.Create()
                                        .Set(Am_SLOT_TO_ANIMATE, Am_FILL_STYLE)
                                        .Set(Am_VALUES_FOR_SLOT_TO_ANIMATE,
                                             Am_Value_List()
                                                 .Add(Am_Red)
                                                 .Add(Am_Blue)
                                                 .Add(Am_Cyan)
                                                 .Add(Am_Purple)
                                                 .Add(Am_Orange)
                                                 .Add(Am_Green)
                                                 .Add(Am_Yellow)
                                                 .Add(Am_Black)
                                                 .Add(Am_White)
                                                 .Add(Am_Amulet_Purple)
                                                 .Add(Am_Motif_Light_Gray)
                                                 .Add(Am_Motif_Light_Blue)
                                                 .Add(Am_Motif_Light_Green)
                                                 .Add(Am_Motif_Light_Orange))));
  rect3.Add_Part(
      anim6 = Am_Animation_Interactor
                  .Create("anim6")
                  //change every 1/2 second
                  .Set(Am_REPEAT_DELAY, Am_Time(500))
                  .Set(Am_START_WHEN, true)
                  .Set_Part(Am_COMMAND, Am_Animation_Blink_Command.Create()));

  std::cout << "first one gets taller and changes color, blue bounces,\n"
            << "green wraps and blinks, orange goes to corner and stops\n"
            << std::flush;

  Am_Object how_set_inter =
      Am_One_Shot_Interactor.Create("change_settings")
          .Set(Am_PRIORITY, 200) //higher than normal running
          .Set(Am_START_WHEN, "ANY_KEYBOARD");
  Am_Object cmd;
  cmd = how_set_inter.Get(Am_COMMAND);
  cmd.Set(Am_DO_METHOD, change_setting);
  window.Add_Part(how_set_inter);
  Am_Screen.Add_Part(window);

  std::cout << "Loading pixmaps\n" << std::flush;
  init_pixmaps();

  std::cout << "Starting interactors\n" << std::flush;

  Am_Start_Interactor(anim1);
  Am_Start_Interactor(anim2);
  Am_Start_Interactor(anim3);
  Am_Start_Interactor(anim4);
  Am_Start_Interactor(anim5);
  Am_Start_Interactor(anim6);

  Am_Main_Event_Loop();
  Am_Cleanup();

  return 0;
}
