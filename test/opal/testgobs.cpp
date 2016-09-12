/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <am_inc.h>

#include <amulet.h>
#include DEBUGGER__H
#include OPAL_ADVANCED__H

#include <string.h> //memcpy

Am_Object arc_window, font_window, roundtangle_window, bitmap_window,
    point_window;
Am_Object font_text0, font_text1, font_text2, font_text3, font_text4,
    font_text5, font_text6, font_text7;
Am_Object inter;

static char dash15_list[2] = {15, 15};
static char dash8_list[2] = {8, 8};
static char dash20_list[2] = {20, 20};

Am_Style dash15(0.0f, 0.0f, 0.0f, 8, Am_CAP_BUTT, Am_JOIN_MITER,
                Am_LINE_ON_OFF_DASH, dash15_list);
Am_Style dash8(0.0f, 0.0f, 0.0f, 20, Am_CAP_BUTT, Am_JOIN_MITER,
               Am_LINE_ON_OFF_DASH, dash8_list);
Am_Style dash20(0.0f, 0.0f, 1.0f, 12, Am_CAP_BUTT, Am_JOIN_MITER,
                Am_LINE_ON_OFF_DASH, dash20_list);

Am_Font fixed_small(Am_FONT_FIXED, false, false, false, Am_FONT_SMALL);
Am_Font fixed_large(Am_FONT_FIXED, false, false, false, Am_FONT_LARGE);
Am_Font fixed_vl_bold_italic(Am_FONT_FIXED, true, true, false,
                             Am_FONT_VERY_LARGE);
Am_Font times_medium(Am_FONT_SERIF);
Am_Font times_large_italic(Am_FONT_SERIF, false, true, false, Am_FONT_LARGE);
Am_Font helvetica_medium(Am_FONT_SANS_SERIF, false, false, false,
                         Am_FONT_MEDIUM);
Am_Font helvetica_vl_bold(Am_FONT_SANS_SERIF, true, false, false,
                          Am_FONT_VERY_LARGE);

Am_Slot_Key FONT_TEXT0_SLOT = Am_Register_Slot_Name("FONT_TEXT0");
Am_Slot_Key FONT_TEXT1_SLOT = Am_Register_Slot_Name("FONT_TEXT1");
Am_Slot_Key FONT_TEXT2_SLOT = Am_Register_Slot_Name("FONT_TEXT2");
Am_Slot_Key FONT_TEXT3_SLOT = Am_Register_Slot_Name("FONT_TEXT3");
Am_Slot_Key FONT_TEXT4_SLOT = Am_Register_Slot_Name("FONT_TEXT4");
Am_Slot_Key FONT_TEXT5_SLOT = Am_Register_Slot_Name("FONT_TEXT5");
Am_Slot_Key FONT_TEXT6_SLOT = Am_Register_Slot_Name("FONT_TEXT6");
Am_Slot_Key FONT_TEXT7_SLOT = Am_Register_Slot_Name("FONT_TEXT7");
Am_Slot_Key PREV = Am_Register_Slot_Name("PREV");
Am_Slot_Key SHIP_TYPE = Am_Register_Slot_Name("SHIP_TYPE");
Am_Slot_Key BITMAP_PART = Am_Register_Slot_Name("BITMAP_PART");

//ship image filenames
#if defined(_WINDOWS)
//#define IMFN_FEDERATION "lib/images/ent.bmp"
//#define IMFN_KLINGON "lib/images/kling.bmp"
#define IMFN_FEDERATION "lib/images/arrow.gif"
#define IMFN_KLINGON "lib/images/amside.bmp"

#else
#define IMFN_FEDERATION "lib/images/ent.xbm"
#define IMFN_KLINGON "lib/images/kling.xbm"
#endif

void
init_arc_window()
{

  arc_window = Am_Window.Create("arc_window")
                   .Set(Am_LEFT, 800)
                   .Set(Am_TOP, 10)
                   .Set(Am_WIDTH, 320)
                   .Set(Am_HEIGHT, 300)
                   .Set(Am_TITLE, "Testing Opal Arcs")

                   .Add_Part(Am_Arc.Create("arc1")
                                 .Set(Am_LEFT, 130)
                                 .Set(Am_TOP, 160)
                                 .Set(Am_WIDTH, 20)
                                 .Set(Am_HEIGHT, 20)
                                 .Set(Am_FILL_STYLE, Am_No_Style))

                   .Add_Part(Am_Arc.Create("arc2")
                                 .Set(Am_LEFT, 20)
                                 .Set(Am_TOP, 20)
                                 .Set(Am_WIDTH, 150)
                                 .Set(Am_HEIGHT, 70)
                                 .Set(Am_ANGLE1, 0)
                                 .Set(Am_ANGLE2, 180)
                                 .Set(Am_FILL_STYLE, Am_No_Style))

                   .Add_Part(Am_Arc.Create("arc3")
                                 .Set(Am_LEFT, 50)
                                 .Set(Am_TOP, 140)
                                 .Set(Am_WIDTH, 70)
                                 .Set(Am_HEIGHT, 140)
                                 .Set(Am_LINE_STYLE, dash15)
                                 .Set(Am_FILL_STYLE, Am_Red))

                   .Add_Part(Am_Arc.Create("arc4")
                                 .Set(Am_LEFT, 60)
                                 .Set(Am_TOP, 40)
                                 .Set(Am_WIDTH, 90)
                                 .Set(Am_HEIGHT, 90)
                                 .Set(Am_ANGLE1, 300)
                                 .Set(Am_ANGLE2, 270)
                                 .Set(Am_LINE_STYLE, dash15)
                                 .Set(Am_FILL_STYLE, Am_Blue))

                   .Add_Part(Am_Arc.Create("arc5")
                                 .Set(Am_LEFT, 0)
                                 .Set(Am_TOP, -30)
                                 .Set(Am_WIDTH, 50)
                                 .Set(Am_HEIGHT, 300)
                                 .Set(Am_ANGLE1, 240)
                                 .Set(Am_ANGLE2, 60)
                                 .Set(Am_LINE_STYLE, Am_Line_4)
                                 .Set(Am_FILL_STYLE, Am_Green))

                   .Add_Part(Am_Arc.Create("arc6")
                                 .Set(Am_LEFT, 10)
                                 .Set(Am_TOP, 240)
                                 .Set(Am_WIDTH, 280)
                                 .Set(Am_HEIGHT, 50)
                                 .Set(Am_ANGLE1, -36)
                                 .Set(Am_ANGLE2, 72)
                                 .Set(Am_LINE_STYLE, Am_No_Style)
                                 .Set(Am_FILL_STYLE, Am_Blue))

                   .Add_Part(Am_Arc.Create("arc7")
                                 .Set(Am_LEFT, -10)
                                 .Set(Am_TOP, 70)
                                 .Set(Am_WIDTH, 300)
                                 .Set(Am_HEIGHT, 200)
                                 .Set(Am_ANGLE1, 23)
                                 .Set(Am_ANGLE2, 45)
                                 .Set(Am_LINE_STYLE, dash8)
                                 .Set(Am_FILL_STYLE, Am_Red))

                   .Add_Part(Am_Arc.Create("arc8")
                                 .Set(Am_LEFT, 200)
                                 .Set(Am_TOP, 160)
                                 .Set(Am_WIDTH, 80)
                                 .Set(Am_HEIGHT, 80)
                                 .Set(Am_LINE_STYLE, dash20)
                                 .Set(Am_FILL_STYLE, Am_Yellow))

                   .Add_Part(Am_Arc.Create("arc9")
                                 .Set(Am_LEFT, 50)
                                 .Set(Am_TOP, 140)
                                 .Set(Am_WIDTH, 70)
                                 .Set(Am_HEIGHT, 140)
                                 .Set(Am_FILL_STYLE, Am_No_Style))

                   .Add_Part(Am_Arc.Create("arc10")
                                 .Set(Am_LEFT, 60)
                                 .Set(Am_TOP, 40)
                                 .Set(Am_WIDTH, 90)
                                 .Set(Am_HEIGHT, 90)
                                 .Set(Am_ANGLE1, 300)
                                 .Set(Am_ANGLE2, 270)
                                 .Set(Am_FILL_STYLE, Am_No_Style))

                   .Add_Part(Am_Arc.Create("arc11")
                                 .Set(Am_LEFT, 200)
                                 .Set(Am_TOP, 160)
                                 .Set(Am_WIDTH, 80)
                                 .Set(Am_HEIGHT, 80)
                                 .Set(Am_FILL_STYLE, Am_No_Style));
}

void
init_roundtangle_window()
{

  roundtangle_window =
      Am_Window.Create("roundtangle_window")
          .Set(Am_LEFT, 200)
          .Set(Am_TOP, 10)
          .Set(Am_WIDTH, 300)
          .Set(Am_HEIGHT, 320)
          .Set(Am_TITLE, "Testing Opal Roundtangles")
          .Set(Am_DOUBLE_BUFFER, true)

          .Add_Part(Am_Roundtangle.Create("round1")
                        .Set(Am_LEFT, 20)
                        .Set(Am_TOP, 20)
                        .Set(Am_WIDTH, 250)
                        .Set(Am_HEIGHT, 280)
                        .Set(Am_RADIUS, Am_LARGE_RADIUS)
                        .Set(Am_FILL_STYLE, Am_Red))

          .Add_Part(Am_Roundtangle.Create("round2")
                        .Set(Am_LEFT, 60)
                        .Set(Am_TOP, 60)
                        .Set(Am_WIDTH, 170)
                        .Set(Am_HEIGHT, 200)
                        .Set(Am_RADIUS, Am_MEDIUM_RADIUS)
                        .Set(Am_FILL_STYLE, Am_Yellow))

          .Add_Part(Am_Roundtangle.Create("round3")
                        .Set(Am_LEFT, 90)
                        .Set(Am_TOP, 90)
                        .Set(Am_WIDTH, 110)
                        .Set(Am_HEIGHT, 140)
                        .Set(Am_FILL_STYLE, Am_Opaque_Diamond_Stipple));
}

Am_Define_Formula(int, compute_text_top)
{
  Am_Object prev;
  prev = self.Get(PREV);
  int prev_bottom = (int)(prev.Get(Am_TOP)) + (int)(prev.Get(Am_HEIGHT));
  return (prev_bottom + 10);
}

void
init_font_window()
{

  font_window = Am_Window.Create("font_window")
                    .Set(Am_LEFT, 520)
                    .Set(Am_TOP, 10)
                    .Set(Am_WIDTH, 250)
                    .Set(Am_HEIGHT, 300)
                    .Set(Am_TITLE, "Testing Opal Fonts");

  font_text0 = Am_Text.Create("font_text0")
                   .Set(Am_LEFT, 10)
                   .Set(Am_TOP, 20)
                   .Set(Am_TEXT, "Font Test");

  font_text1 = Am_Text.Create("font_text1")
                   .Add(PREV, font_text0)
                   .Set(Am_LEFT, 10)
                   .Set(Am_TOP, compute_text_top)
                   .Set(Am_TEXT, "Font Test")
                   .Set(Am_FONT, fixed_small);

  font_text2 = Am_Text.Create("font_text2")
                   .Add(PREV, font_text1)
                   .Set(Am_LEFT, 10)
                   .Set(Am_TOP, compute_text_top)
                   .Set(Am_TEXT, "Font Test")
                   .Set(Am_FONT, fixed_large);

  font_text3 = Am_Text.Create("font_text3")
                   .Add(PREV, font_text2)
                   .Set(Am_LEFT, 10)
                   .Set(Am_TOP, compute_text_top)
                   .Set(Am_TEXT, "Font Test")
                   .Set(Am_FONT, fixed_vl_bold_italic);

  font_text4 = Am_Text.Create("font_text4")
                   .Add(PREV, font_text3)
                   .Set(Am_LEFT, 10)
                   .Set(Am_TOP, compute_text_top)
                   .Set(Am_TEXT, "Font Test")
                   .Set(Am_FONT, times_medium);

  font_text5 = Am_Text.Create("font_text5")
                   .Add(PREV, font_text4)
                   .Set(Am_LEFT, 10)
                   .Set(Am_TOP, compute_text_top)
                   .Set(Am_TEXT, "Font Test")
                   .Set(Am_FONT, times_large_italic);

  font_text6 = Am_Text.Create("font_text6")
                   .Add(PREV, font_text5)
                   .Set(Am_LEFT, 10)
                   .Set(Am_TOP, compute_text_top)
                   .Set(Am_TEXT, "Font Test")
                   .Set(Am_FONT, helvetica_medium)
                   .Set(Am_FILL_STYLE, Am_Red);

  font_text7 = Am_Text.Create("font_text7")
                   .Add(PREV, font_text6)
                   .Set(Am_LEFT, 10)
                   .Set(Am_TOP, compute_text_top)
                   .Set(Am_TEXT, "Font Test")
                   .Set(Am_FONT, helvetica_vl_bold)
                   .Set(Am_FILL_STYLE, Am_Red)
                   .Set(Am_INVERT, true);

  font_window.Add_Part(FONT_TEXT0_SLOT, font_text0);
  font_window.Add_Part(FONT_TEXT1_SLOT, font_text1);
  font_window.Add_Part(FONT_TEXT2_SLOT, font_text2);
  font_window.Add_Part(FONT_TEXT3_SLOT, font_text3);
  font_window.Add_Part(FONT_TEXT4_SLOT, font_text4);
  font_window.Add_Part(FONT_TEXT5_SLOT, font_text5);
  font_window.Add_Part(FONT_TEXT6_SLOT, font_text6);
  font_window.Add_Part(FONT_TEXT7_SLOT, font_text7);
}

void
print_help()
{
  std::cout << "Press q in font window to quit." << std::endl;
}

Am_Define_Method(Am_Object_Method, void, do_command, (Am_Object self))
{
  Am_Object inter = self.Get_Owner();
  Am_Input_Char c = inter.Get(Am_START_CHAR);
  if (c.As_Char() == 'q') {
    Am_Exit_Main_Event_Loop();
  }
}

typedef int SP_Ship_Type;
#define FEDERATION 1
#define KLINGON 2

Am_Image_Array SP_Federation_Image, SP_Klingon_Image;
Am_Object bitmap_group, bitmap_group2;

Am_Define_Image_Formula(ship_image)
{
  SP_Ship_Type st = (int)(self.Get_Owner().Get(SHIP_TYPE));
  if (st == FEDERATION)
    return SP_Federation_Image;
  else if (st == KLINGON)
    return SP_Klingon_Image;
  else {
    //Am_Error("ship_image: Invalid ship type.\n");
    return (0L);
  }
}

void
init_bitmap_window()
{

  // Can't call Am_Merge_Pathname until after Am_Initialize();
  SP_Federation_Image = Am_Image_Array(Am_Merge_Pathname(IMFN_FEDERATION));
  SP_Klingon_Image = Am_Image_Array(Am_Merge_Pathname(IMFN_KLINGON));

  bitmap_window = Am_Window.Create("bitmap_window")
                      .Set(Am_LEFT, 800)
                      .Set(Am_TOP, 340)
                      .Set(Am_WIDTH, 270)
                      .Set(Am_HEIGHT, 100)
                      .Set(Am_TITLE, "Testing Opal Bitmaps")
                      .Set(Am_FILL_STYLE, Am_Black);

  bool color = Am_Screen.Get(Am_IS_COLOR);
  Am_Style ship_color = color ? Am_Green : Am_White;

  Am_Object bitmap_group =
      Am_Group.Create("bitmap_group")
          .Add(SHIP_TYPE, FEDERATION)
          .Set(Am_LEFT, 20)
          .Set(Am_TOP, 30)
          .Set(Am_WIDTH, Am_Width_Of_Parts)
          .Set(Am_HEIGHT, Am_Height_Of_Parts)
          .Add_Part(BITMAP_PART, Am_Bitmap.Create("bitmap1")
                                     .Set(Am_LEFT, 0)
                                     .Set(Am_TOP, 0)
                                     .Set(Am_IMAGE, ship_image)
                                     .Set(Am_LINE_STYLE, ship_color));

  bitmap_group2 = bitmap_group.Create("bitmap_group2")
                      .Set(SHIP_TYPE, KLINGON)
                      .Set(Am_LEFT, 150);

  bitmap_window.Add_Part(bitmap_group);
  bitmap_window.Add_Part(bitmap_group2);
}

void
init_point_window()
{

  point_window = Am_Window.Create("point_window")
                     .Set(Am_LEFT, 200)
                     .Set(Am_TOP, 640)
                     .Set(Am_WIDTH, 300)
                     .Set(Am_HEIGHT, 300)
                     .Set(Am_TITLE, "Testing Opal Polygons");

  Am_Point_List triangle_pl;
  triangle_pl.Add(15.0f, 50.0f).Add(45.0f, 10.0f).Add(75.0f, 50.0f);

  Am_Object triangle_polygon = Am_Polygon.Create("triangle_polygon")
                                   .Set(Am_POINT_LIST, triangle_pl)
                                   .Set(Am_LINE_STYLE, Am_Line_2)
                                   .Set(Am_FILL_STYLE, Am_Yellow);

  static int star_ar[12] = {100, 0, 41, 181, 195, 69, 5, 69, 159, 181, 100, 0};
  Am_Point_List star_pl(star_ar, 12);

  Am_Object star_polygon = Am_Polygon.Create("star_polygon")
                               .Set(Am_POINT_LIST, star_pl)
                               .Set(Am_FILL_STYLE, Am_No_Style);

  point_window.Add_Part(triangle_polygon).Add_Part(star_polygon);
}

int
main()
{
  Am_Initialize();

  init_arc_window();
  init_font_window();
  init_roundtangle_window();
  init_bitmap_window();
  init_point_window();

  Am_Screen.Add_Part(arc_window);
  Am_Screen.Add_Part(font_window);
  Am_Screen.Add_Part(roundtangle_window);
  Am_Screen.Add_Part(bitmap_window);
  Am_Screen.Add_Part(point_window);

  font_window.Add_Part(inter = Am_One_Shot_Interactor.Create("keyboard press")
                                   .Set(Am_START_WHEN, "ANY_KEYBOARD")
                                   .Set(Am_CONTINUOUS, 0)
                                   .Get_Object(Am_COMMAND)
                                   .Set(Am_DO_METHOD, do_command)
                                   .Get_Owner());

  print_help();

  Am_Main_Event_Loop();
  Am_Cleanup();

  return 0;
}
