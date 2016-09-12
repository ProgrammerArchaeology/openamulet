/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <stdio.h>
#include <am_inc.h>

#include FORMULA__H
#include OPAL__H
#include STANDARD_SLOTS__H

extern Am_Object *Am_Text_Button;
#define Am_GRAY_WIDTH 1000
#define Am_SHADOW_OFFSET 1001
#define Am_FINAL_FUNCTION 1002
#define Am_SELECTED 1003

Am_Slot_Key TOGGLE_BUTTON = Am_Register_Slot_Name("TOGGLE_BUTTON");

Am_Define_Formula(char *, text_string)
{
  Am_Object *button = self.Get(TOGGLE_BUTTON);
  int selected = button.GV(Am_SELECTED);
  if (selected)
    return "This is a test";
  else
    return "Hello World!";
}

Am_Define_Formula(int, text_left)
{
  Am_Object *window = self.Get(Am_WINDOW);
  int win_width = window.GV(Am_WIDTH);
  int my_width = self.GV(Am_WIDTH);
  return (win_width - my_width) / 2;
}

Am_Define_Formula(int, text_top)
{
  Am_Object *window = self.Get(Am_WINDOW);
  int win_height = window.GV(Am_HEIGHT);
  int my_height = self.GV(Am_HEIGHT);
  int ascent = self.GV(Am_ASCENT);
  return (win_height / 3) - (my_height / 2) + ascent;
}

Am_Define_Formula(int, toggle_left)
{
  Am_Object *window = self.Get(Am_WINDOW);
  int win_width = window.GV(Am_WIDTH);
  int my_width = self.GV(Am_WIDTH);
  return (win_width / 3) - (my_width / 2);
}

Am_Define_Formula(int, quit_left)
{
  Am_Object *window = self.Get(Am_WINDOW);
  int win_width = window.GV(Am_WIDTH);
  int my_width = self.GV(Am_WIDTH);
  return (2 * (win_width / 3)) - (my_width / 2);
}

Am_Define_Formula(int, button_top)
{
  Am_Object *window = self.Get(Am_WINDOW);
  int win_height = window.GV(Am_HEIGHT);
  int my_height = self.GV(Am_HEIGHT);
  return win_height - my_height - 10;
}

void
quit_function(Am_Object *inter)
{
  Am_Exit_Main_Event_Loop();
}

int
main()
{
  Am_Object *toggle;

  Am_Initialize();

  Am_Screen.Add_Part(
      Am_Window.Create("main")
          .Set(Am_TITLE, "My C++ Example")
          .Set(Am_LEFT, 200)
          .Set(Am_TOP, 100)
          .Set(Am_WIDTH, 200)
          .Set(Am_HEIGHT, 150)
          .Add_Part(toggle = Am_Text_Button.Create("toggle")
                                 .Set(Am_STRING, "TOGGLE")
                                 .Set(Am_LEFT, Am_Formula::Create(toggle_left))
                                 .Set(Am_TOP, Am_Formula::Create(button_top))
                                 .Set(Am_GRAY_WIDTH, 3)
                                 .Set(Am_SHADOW_OFFSET, 5))
          .Add_Part(Am_Text_Button.Create("quit")
                        .Set(Am_STRING, "QUIT")
                        .Set(Am_LEFT, Am_Formula::Create(quit_left))
                        .Set(Am_TOP, Am_Formula::Create(button_top))
                        .Set(Am_GRAY_WIDTH, 3)
                        .Set(Am_SHADOW_OFFSET, 5)
                        .Set(Am_FINAL_FUNCTION, quit_function))
          .Add_Part(Am_Text.Create("text")
                        .Set(Am_STRING, Am_Formula::Create(text_string))
                        .Set(Am_LEFT, Am_Formula::Create(text_left))
                        .Set(Am_TOP, Am_Formula::Create(text_top))
                        .Set(TOGGLE_BUTTON, toggle)));

  Am_Main_Event_Loop();
  Am_Cleanup();

  return 0;
}
