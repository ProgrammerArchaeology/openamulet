/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

// R, G, B scrollbars to choose a color, with quit button.

#include <amulet.h>

Am_Object H, S, V, ht, st, vt;

Am_Define_No_Self_Formula(const char *, hue_text)
{
  static char txt[10];

  Am_Object c;
  c = H.Get(Am_COMMAND);
  float r = c.Get(Am_VALUE);
  sprintf(txt, "%4.1f", r);
  return txt;
}

Am_Define_No_Self_Formula(const char *, sat_text)
{
  static char txt[10];

  Am_Object c;
  c = S.Get(Am_COMMAND);
  float r = c.Get(Am_VALUE);
  sprintf(txt, "%3.2f", r);
  return txt;
}

Am_Define_No_Self_Formula(const char *, value_text)
{
  static char txt[10];

  Am_Object c;
  c = V.Get(Am_COMMAND);
  float r = c.Get(Am_VALUE);
  sprintf(txt, "%3.2f", r);
  return txt;
}

Am_Define_No_Self_Formula(Am_Wrapper *, fill_from_scrolls)
{
  float h, s, v;
  static Am_Style temp;

  h = 0.0;
  s = v = 0.8f;

  if (H.Valid() && S.Valid() && V.Valid()) {
    Am_Object c;
    c = H.Get(Am_COMMAND);
    if (c.Valid())
      h = c.Get(Am_VALUE);
    c = S.Get(Am_COMMAND);
    if (c.Valid())
      s = c.Get(Am_VALUE);
    c = V.Get(Am_COMMAND);
    if (c.Valid())
      v = c.Get(Am_VALUE);
  }
  float r, g, b;
  Am_HSV_To_RGB(h, s, v, r, g, b);
  temp = Am_Style(r, g, b);
  return temp;
}

int
main(void)
{
  Am_Object button;
  Am_Initialize();

  H = Am_Vertical_Scroll_Bar.Create("Hue")
          .Set(Am_TOP, 10)
          .Set(Am_LEFT, 10)
          .Set(Am_VALUE_1, 0.0)
          .Set(Am_VALUE_2, 360.0)
          .Set(Am_SMALL_INCREMENT, 1.0)
          .Set(Am_LARGE_INCREMENT, 10.0);
  Am_Object c;
  c = H.Get(Am_COMMAND);
  c.Set(Am_VALUE, 0.0);

  ht = Am_Text.Create("Hue")
           .Set(Am_LEFT, 5)
           .Set(Am_TOP, 220)
           .Set(Am_TEXT, hue_text);

  S = Am_Vertical_Scroll_Bar.Create("Sat")
          .Set(Am_TOP, 10)
          .Set(Am_LEFT, 40)
          .Set(Am_VALUE_1, 0.0)
          .Set(Am_VALUE_2, 1.0)
          .Set(Am_SMALL_INCREMENT, 0.01)
          .Set(Am_LARGE_INCREMENT, 0.1);
  c = S.Get(Am_COMMAND);
  c.Set(Am_VALUE, 0.8);

  st = Am_Text.Create("saturation")
           .Set(Am_LEFT, 40)
           .Set(Am_TOP, 220)
           .Set(Am_TEXT, sat_text);

  V = Am_Vertical_Scroll_Bar.Create("Value")
          .Set(Am_TOP, 10)
          .Set(Am_LEFT, 70)
          .Set(Am_VALUE_1, 0.0)
          .Set(Am_VALUE_2, 1.0)
          .Set(Am_SMALL_INCREMENT, 0.01)
          .Set(Am_LARGE_INCREMENT, 0.1);
  c = V.Get(Am_COMMAND);
  c.Set(Am_VALUE, 0.8);

  vt = Am_Text.Create("Value")
           .Set(Am_LEFT, 70)
           .Set(Am_TOP, 220)
           .Set(Am_TEXT, value_text);

  Am_Screen.Add_Part(
      Am_Window.Create("window")
          .Set(Am_TOP, 50)
          .Set(Am_LEFT, 50)
          .Set(Am_WIDTH, 100)
          .Set(Am_HEIGHT, 290)
          .Set(Am_FILL_STYLE, (fill_from_scrolls))
          .Add_Part(
              Am_Button.Create("button")
                  .Set(Am_LEFT, 10)
                  .Set(Am_TOP, 250)
                  .Set(Am_WIDTH, 80)
                  .Set(Am_FILL_STYLE, fill_from_scrolls)
                  .Set_Part(Am_COMMAND, Am_Quit_No_Ask_Command.Create().Set(
                                            Am_LABEL, "Goodbye")))
          .Add_Part(H)
          .Add_Part(S)
          .Add_Part(V)
          .Add_Part(ht)
          .Add_Part(st)
          .Add_Part(vt));

  Am_Main_Event_Loop();
  Am_Cleanup();

  return 0;
}
