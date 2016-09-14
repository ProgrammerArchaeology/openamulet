/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/
//test
// R, G, B scrollbars to choose a color, with quit button.

#include <amulet.h>

Am_Object R, G, B, rt, gt, bt;

Am_Define_No_Self_Formula(const char *, red_text)
{
  static char txt[10];

  Am_Object c;
  c = R.Get(Am_COMMAND);
  float r = c.Get(Am_VALUE);
  sprintf(txt, "%3.2f", r);
  return txt;
}

Am_Define_No_Self_Formula(const char *, blue_text)
{
  static char txt[10];

  Am_Object c;
  c = B.Get(Am_COMMAND);
  float r = c.Get(Am_VALUE);
  sprintf(txt, "%3.2f", r);
  return txt;
}

Am_Define_No_Self_Formula(const char *, green_text)
{
  static char txt[10];

  Am_Object c;
  c = G.Get(Am_COMMAND);
  float r = c.Get(Am_VALUE);
  sprintf(txt, "%3.2f", r);
  return txt;
}

Am_Define_No_Self_Formula(Am_Wrapper *, fill_from_scrolls)
{
  float r, g, b;
  static Am_Style temp;

  r = g = b = 0.8f;

  if (R.Valid() && G.Valid() && B.Valid()) {
    Am_Object c;
    c = R.Get(Am_COMMAND);
    if (c.Valid())
      r = c.Get(Am_VALUE);
    c = G.Get(Am_COMMAND);
    if (c.Valid())
      g = c.Get(Am_VALUE);
    c = B.Get(Am_COMMAND);
    if (c.Valid())
      b = c.Get(Am_VALUE);
  }
  temp = Am_Style(r, g, b);
  return temp;
}

Am_Define_Method(Am_Object_Method, void, kill_me, (Am_Object))
{
  Am_Exit_Main_Event_Loop();
}

int
main(void)
{
  Am_Object button;
  Am_Initialize();

  R = Am_Vertical_Scroll_Bar.Create("Red")
          .Set(Am_TOP, 10)
          .Set(Am_LEFT, 10)
          .Set(Am_VALUE_1, 0.0)
          .Set(Am_VALUE_2, 1.0)
          .Set(Am_SMALL_INCREMENT, 0.01)
          .Set(Am_LARGE_INCREMENT, 0.1);
  Am_Object c;
  c = R.Get(Am_COMMAND);
  c.Set(Am_VALUE, 0.8);

  rt = Am_Text.Create("Red")
           .Set(Am_LEFT, 10)
           .Set(Am_TOP, 220)
           .Set(Am_TEXT, (red_text));

  G = Am_Vertical_Scroll_Bar.Create("Green")
          .Set(Am_TOP, 10)
          .Set(Am_LEFT, 40)
          .Set(Am_VALUE_1, 0.0)
          .Set(Am_VALUE_2, 1.0)
          .Set(Am_SMALL_INCREMENT, 0.01)
          .Set(Am_LARGE_INCREMENT, 0.1);
  c = G.Get(Am_COMMAND);
  c.Set(Am_VALUE, 0.8);

  gt = Am_Text.Create("green")
           .Set(Am_LEFT, 40)
           .Set(Am_TOP, 220)
           .Set(Am_TEXT, (green_text));

  B = Am_Vertical_Scroll_Bar.Create("Blue")
          .Set(Am_TOP, 10)
          .Set(Am_LEFT, 70)
          .Set(Am_VALUE_1, 0.0)
          .Set(Am_VALUE_2, 1.0)
          .Set(Am_SMALL_INCREMENT, 0.01)
          .Set(Am_LARGE_INCREMENT, 0.1);
  c = B.Get(Am_COMMAND);
  c.Set(Am_VALUE, 0.8);

  bt = Am_Text.Create("Blue")
           .Set(Am_LEFT, 70)
           .Set(Am_TOP, 220)
           .Set(Am_TEXT, (blue_text));

  Am_Screen.Add_Part(
      Am_Window.Create("window")
          .Set(Am_TOP, 50)
          .Set(Am_LEFT, 50)
          .Set(Am_WIDTH, 100)
          .Set(Am_HEIGHT, 290)
          .Set(Am_FILL_STYLE, (fill_from_scrolls))
          .Add_Part(button = Am_Button.Create("button")
                                 .Set(Am_LEFT, 10)
                                 .Set(Am_TOP, 250)
                                 .Set(Am_WIDTH, 80)
                                 .Set(Am_FILL_STYLE, (fill_from_scrolls)))
          .Add_Part(R)
          .Add_Part(G)
          .Add_Part(B)
          .Add_Part(rt)
          .Add_Part(gt)
          .Add_Part(bt));

  button.Get_Object(Am_COMMAND)
      .Set(Am_LABEL, "Goodbye")
      .Set(Am_DO_METHOD, kill_me);

  Am_Main_Event_Loop();
  Am_Cleanup();

  return 0;
}
