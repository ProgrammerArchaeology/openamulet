/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

//  Kanji fonts test program  by Y.Tsujino, 1997.1.10, 1.17, 1.30
//  Kanji fonts test program for size  by Y.Tsujino, 1997.1.11
//  Kanji input test program  by Y.Tsujino, 1997.1.17
//  Japanese Kanji Code Table by Y.Tsujino, 1997.1.23, 1.30

#include <amulet.h>

// Put different kanji encodings are in separate files for Japanese editors
#ifdef _WIN32
#include "testJIO_SJIS.h"
#else
#include "testJIO_JIS.h"
#include "testJIO_EUC.h"
#endif

using namespace std;

Am_Font ETL_Font(Am_FONT_FIXED, false, false, false, Am_FONT_TLARGE);

Am_Font JS_Font(Am_FONT_JFIXED, false, false, false, Am_FONT_SMALL);
Am_Font JM_Font(Am_FONT_JFIXED, false, false, false, Am_FONT_MEDIUM);
Am_Font JL_Font(Am_FONT_JFIXED, false, false, false, Am_FONT_LARGE);
Am_Font JVL_Font(Am_FONT_JFIXED, false, false, false, Am_FONT_VERY_LARGE);

Am_Font JTS_Font(Am_FONT_JFIXED, false, false, false, Am_FONT_TSMALL);
Am_Font JbTS_Font(Am_FONT_JFIXED, true, false, false, Am_FONT_TSMALL);
Am_Font JiTS_Font(Am_FONT_JFIXED, false, true, false, Am_FONT_TSMALL);
Am_Font JbiTS_Font(Am_FONT_JFIXED, true, true, false, Am_FONT_TSMALL);
Am_Font JuTS_Font(Am_FONT_JFIXED, false, false, true, Am_FONT_TSMALL);
Am_Font JbuTS_Font(Am_FONT_JFIXED, true, false, true, Am_FONT_TSMALL);
Am_Font JiuTS_Font(Am_FONT_JFIXED, false, true, true, Am_FONT_TSMALL);
Am_Font JbiuTS_Font(Am_FONT_JFIXED, true, true, true, Am_FONT_TSMALL);

Am_Font JTM_Font(Am_FONT_JFIXED, false, false, false, Am_FONT_TMEDIUM);
Am_Font JbTM_Font(Am_FONT_JFIXED, true, false, false, Am_FONT_TMEDIUM);
Am_Font JiTM_Font(Am_FONT_JFIXED, false, true, false, Am_FONT_TMEDIUM);
Am_Font JbiTM_Font(Am_FONT_JFIXED, true, true, false, Am_FONT_TMEDIUM);
Am_Font JuTM_Font(Am_FONT_JFIXED, false, false, true, Am_FONT_TMEDIUM);
Am_Font JbuTM_Font(Am_FONT_JFIXED, true, false, true, Am_FONT_TMEDIUM);
Am_Font JiuTM_Font(Am_FONT_JFIXED, false, true, true, Am_FONT_TMEDIUM);
Am_Font JbiuTM_Font(Am_FONT_JFIXED, true, true, true, Am_FONT_TMEDIUM);

Am_Font JTL_Font(Am_FONT_JFIXED, false, false, false, Am_FONT_TLARGE);
Am_Font JbTL_Font(Am_FONT_JFIXED, true, false, false, Am_FONT_TLARGE);
Am_Font JiTL_Font(Am_FONT_JFIXED, false, true, false, Am_FONT_TLARGE);
Am_Font JbiTL_Font(Am_FONT_JFIXED, true, true, false, Am_FONT_TLARGE);
Am_Font JuTL_Font(Am_FONT_JFIXED, false, false, true, Am_FONT_TLARGE);
Am_Font JbuTL_Font(Am_FONT_JFIXED, true, false, true, Am_FONT_TLARGE);
Am_Font JiuTL_Font(Am_FONT_JFIXED, false, true, true, Am_FONT_TLARGE);
Am_Font JbiuTL_Font(Am_FONT_JFIXED, true, true, true, Am_FONT_TLARGE);

Am_Font JTVL_Font(Am_FONT_JFIXED, false, false, false, Am_FONT_TVERY_LARGE);
Am_Font JbTVL_Font(Am_FONT_JFIXED, true, false, false, Am_FONT_TVERY_LARGE);
Am_Font JiTVL_Font(Am_FONT_JFIXED, false, true, false, Am_FONT_TVERY_LARGE);
Am_Font JbiTVL_Font(Am_FONT_JFIXED, true, true, false, Am_FONT_TVERY_LARGE);
Am_Font JuTVL_Font(Am_FONT_JFIXED, false, false, true, Am_FONT_TVERY_LARGE);
Am_Font JbuTVL_Font(Am_FONT_JFIXED, true, false, true, Am_FONT_TVERY_LARGE);
Am_Font JiuTVL_Font(Am_FONT_JFIXED, false, true, true, Am_FONT_TVERY_LARGE);
Am_Font JbiuTVL_Font(Am_FONT_JFIXED, true, true, true, Am_FONT_TVERY_LARGE);

Am_Font JPTL_Font(Am_FONT_JPROPORTIONAL, false, false, false, Am_FONT_TLARGE);
Am_Font CTL_Font(Am_FONT_CFIXED, false, false, false, Am_FONT_TLARGE);
Am_Font KTL_Font(Am_FONT_KFIXED, false, false, false, Am_FONT_TLARGE);
Am_Font CTM_Font(Am_FONT_CFIXED, false, false, false, Am_FONT_TMEDIUM);
Am_Font KTM_Font(Am_FONT_KFIXED, false, false, false, Am_FONT_TMEDIUM);

Am_Font MSM_Font(MSM_FONT);
Am_Font MSG_Font(MSG_FONT);
Am_Font MSMP_Font(MSMP_FONT);
Am_Font MSGP_Font(MSGP_FONT);

Am_Object mywin;

Am_Object Size_Face_Group;
Am_Object Size_Face_EUC_Group;
Am_Object More_Sizes_Group;
Am_Object Families_Group;
Am_Object Edit_Group;
Am_Object Edit_EUC_Group;
Am_Object Ktable_Group;
Am_Object Kktable_Group;
Am_Object Kctable_Group;
Am_Object Test_Group;

Am_Slot_Key DISPLAY_PART = Am_Register_Slot_Name("Display Part");

Am_Define_Method(Am_Object_Method, void, change_setting, (Am_Object cmd))
{
  Am_Object inter = cmd.Get_Owner();
  Am_Input_Char c = inter.Get(Am_START_CHAR);

  switch (c.As_Char()) {
  case '1': {
    mywin.Remove_Part(DISPLAY_PART);
    mywin.Add_Part(DISPLAY_PART, Size_Face_Group);
    break;
  }
  case '2': {
    mywin.Remove_Part(DISPLAY_PART);
    mywin.Add_Part(DISPLAY_PART, More_Sizes_Group);
    break;
  }
  case '3': {
    mywin.Remove_Part(DISPLAY_PART);
    mywin.Add_Part(DISPLAY_PART, Families_Group);
    break;
  }
  case '4': {
    mywin.Remove_Part(DISPLAY_PART);
    mywin.Add_Part(DISPLAY_PART, Edit_Group);
    break;
  }
  case '5': {
    mywin.Remove_Part(DISPLAY_PART);
    mywin.Add_Part(DISPLAY_PART, Size_Face_EUC_Group);
    break;
  }
  case '6': {
    mywin.Remove_Part(DISPLAY_PART);
    mywin.Add_Part(DISPLAY_PART, Edit_EUC_Group);
    break;
  }
  case '7': {
    mywin.Remove_Part(DISPLAY_PART);
    mywin.Add_Part(DISPLAY_PART, Ktable_Group);
    break;
  }
  case '8': {
    mywin.Remove_Part(DISPLAY_PART);
    mywin.Add_Part(DISPLAY_PART, Kktable_Group);
    break;
  }
  case '9': {
    mywin.Remove_Part(DISPLAY_PART);
    mywin.Add_Part(DISPLAY_PART, Kctable_Group);
    break;
  }
  case '0': {
    mywin.Remove_Part(DISPLAY_PART);
    mywin.Add_Part(DISPLAY_PART, Test_Group);
    break;
  }
  case 'q': {
    Am_Exit_Main_Event_Loop();
  }
  }
}

void
Test_Group_Init()
{
  Test_Group = Am_Group.Create("Test Group")
                   .Set(Am_LEFT, 10)
                   .Set(Am_TOP, 60)
                   .Set(Am_WIDTH, 730)
                   .Set(Am_HEIGHT, 430);
}

Am_Define_Formula(int, maxwidth)
{
  return ((int)self.Get_Owner().Get(Am_WIDTH) - (int)self.Get(Am_LEFT));
}

Am_Define_Formula(int, maxheight)
{
  return ((int)self.Get_Owner().Get(Am_HEIGHT) - (int)self.Get(Am_TOP));
}

Am_Define_Method(Am_Object_Method, void, qmethod, (Am_Object /*cmd */))
{
  Am_Exit_Main_Event_Loop();
}

Am_Object
Ktable_Init(Am_Font kline_font, const char *ktitle, const char *etitle,
            const char *table_name)
{
  Am_Object Ktable_Group = Test_Group.Create(table_name);

  Am_Object Ktitle = Am_Text.Create("Ktitle")
                         .Set(Am_TOP, 5)
                         .Set(Am_TEXT, ktitle)
                         .Set(Am_FONT, Am_Japanese_Font);
  Ktable_Group.Add_Part(Ktitle);

  Am_Object Etitle = Am_Text.Create("Etitle")
                         .Set(Am_TOP, (int)Ktitle.Get(Am_TOP))
                         .Set(Am_LEFT, (int)Ktitle.Get(Am_LEFT) +
                                           (int)Ktitle.Get(Am_WIDTH) + 10)
                         .Set(Am_TEXT, etitle);
  Ktable_Group.Add_Part(Etitle);

  Am_Object table = Am_Scrolling_Group.Create("table")
                        .Set(Am_TOP, (int)Ktitle.Get(Am_TOP) +
                                         (int)Ktitle.Get(Am_HEIGHT) + 10)
                        .Set(Am_WIDTH, maxwidth)
                        .Set(Am_HEIGHT, maxheight)
                        .Set(Am_INNER_WIDTH, Am_Width_Of_Parts)
                        .Set(Am_INNER_HEIGHT, Am_Height_Of_Parts);
  Ktable_Group.Add_Part(table);

  Am_Object line =
      Am_Text.Create("Kline").Set(Am_LEFT, KLINE_LEFT).Set(Am_FONT, kline_font);

#ifdef _WIN32
#define START_I 0x81
#define END_I 0x9F
#define START_J 0x40
#define END_J 0xFE
#else
#define START_I 0x20
#define END_I 0x7E
#define START_J 0x20
#define END_J 0x7F
#endif

  int i, j, k, pos, step = (int)line.Get(Am_HEIGHT) + 2;
  char num[4], buf[400];

  for (i = START_I, pos = 5; i <= END_I; i++, pos += step) {
    sprintf(num, "%2x", i);
    for (j = START_J, k = 0; j <= END_J; j++) {
      buf[k++] = i;
      buf[k++] = j;
    }
    buf[k] = 0;
    table
        .Add_Part(Am_Text.Create()
                      .Set(Am_LEFT, 5)
                      .Set(Am_TOP, pos)
#ifndef _WIN32
                      .Set(Am_FONT, ETL_Font)
#endif
                      .Set(Am_TEXT, num))
        .Add_Part(line.Create().Set(Am_TOP, pos).Set(Am_TEXT, buf));
  }

#ifdef _WIN32
  for (i = 0xE0; i <= 0xFC; i++, pos += step) {
    sprintf(num, "%2x", i);
    for (j = 0x40, k = 0; j <= 0xFE; j++) {
      buf[k++] = i;
      buf[k++] = j;
    }
    buf[k] = (0L);
    table
        .Add_Part(
            Am_Text.Create().Set(Am_LEFT, 5).Set(Am_TOP, pos).Set(Am_TEXT, num))
        .Add_Part(line.Create().Set(Am_TOP, pos).Set(Am_TEXT, buf));
  }
#endif

  return Ktable_Group;
}

Am_Object ptext;

Am_Define_Method(Am_Object_Method, void, mymethod, (Am_Object cmd))
{
  ptext.Set(Am_TEXT, cmd.Get(Am_VALUE));
}

Am_Object
Edit_Init(const char *title, const char *ptext_string)
{
  Edit_Group = Test_Group.Create("Edit Group");

  Am_Object kanji = Am_Text.Create("kanjiE")
                        .Set(Am_TOP, 5)
                        .Set(Am_TEXT, title)
                        .Set(Am_FONT, Am_Japanese_Font);
  Edit_Group.Add_Part(kanji);

  Am_Object english =
      Am_Text.Create("english")
          .Set(Am_TOP, (int)kanji.Get(Am_TOP))
          .Set(Am_LEFT, (int)kanji.Get(Am_LEFT) + (int)kanji.Get(Am_WIDTH) + 10)
          .Set(Am_TEXT, "Japanese Input Test");
  Edit_Group.Add_Part(english);

  Am_Object my_input =
      Am_Text_Input_Widget.Create()
          .Set(Am_TOP, (int)kanji.Get(Am_TOP) + (int)kanji.Get(Am_HEIGHT) + 10)
          .Set(Am_LEFT, (int)kanji.Get(Am_LEFT))
          .Set(Am_WIDTH, 300)
          .Set(Am_FONT, Am_Japanese_Font);

  my_input.Get_Object(Am_COMMAND).Set(Am_LABEL, "").Set(Am_DO_METHOD, mymethod);
  Edit_Group.Add_Part(my_input);

  Am_Object tlabel = Am_Text.Create("tlabel")
                         .Set(Am_TOP, (int)my_input.Get(Am_TOP) +
                                          (int)my_input.Get(Am_HEIGHT) + 10)
                         .Set(Am_LEFT, (int)kanji.Get(Am_LEFT))
                         .Set(Am_TEXT, "Input text was : ");
  Edit_Group.Add_Part(tlabel);

  ptext =
      Am_Text.Create("ptext")
          .Set(Am_TOP, (int)tlabel.Get(Am_TOP))
          .Set(Am_LEFT, (int)tlabel.Get(Am_LEFT) + (int)tlabel.Get(Am_WIDTH))
          .Set(Am_FONT, Am_Japanese_Font)
          .Set(Am_TEXT, ptext_string);
  Edit_Group.Add_Part(ptext);

  return Edit_Group;
}

void
Families_Init()
{
  Families_Group = Test_Group.Create("Families");

  Am_Object kanji = Am_Text.Create("kanji7")
                        .Set(Am_TOP, 5)
                        .Set(Am_TEXT, KANJI7)
                        .Set(Am_FONT, Am_Japanese_Font);
  Families_Group.Add_Part(kanji);

  Am_Object english =
      Am_Text.Create("english")
          .Set(Am_TOP, (int)kanji.Get(Am_TOP))
          .Set(Am_LEFT, (int)kanji.Get(Am_LEFT) + (int)kanji.Get(Am_WIDTH) + 10)
          .Set(Am_TEXT, "Japanese Font Family Test");
  Families_Group.Add_Part(english);

  Am_Object Jtext =
      Am_Text.Create("J text")
          .Set(Am_TEXT, J_TEST_STRING)
          .Set(Am_FONT, JTL_Font)
          .Set(Am_TOP, (int)kanji.Get(Am_TOP) + (int)kanji.Get(Am_HEIGHT) + 10);
  Families_Group.Add_Part(Jtext);

  Am_Object JPtext =
      Am_Text.Create("JP text")
          .Set(Am_TEXT, JP_TEST_STRING)
          .Set(Am_FONT, JPTL_Font)
          .Set(Am_LEFT, (int)Jtext.Get(Am_LEFT))
          .Set(Am_TOP, (int)Jtext.Get(Am_TOP) + (int)Jtext.Get(Am_HEIGHT) + 4);
  Families_Group.Add_Part(JPtext);

  Am_Object Ctext = Am_Text.Create("C text")
                        .Set(Am_TEXT, C_TEST_STRING)
                        .Set(Am_FONT, CTL_Font)
                        .Set(Am_LEFT, (int)Jtext.Get(Am_LEFT))
                        .Set(Am_TOP, (int)JPtext.Get(Am_TOP) +
                                         (int)JPtext.Get(Am_HEIGHT) + 4);
  Families_Group.Add_Part(Ctext);

  Am_Object Ktext =
      Am_Text.Create("K text")
          .Set(Am_TEXT, K_TEST_STRING)
          .Set(Am_FONT, KTL_Font)
          .Set(Am_LEFT, (int)Jtext.Get(Am_LEFT))
          .Set(Am_TOP, (int)Ctext.Get(Am_TOP) + (int)Ctext.Get(Am_HEIGHT) + 4);
  Families_Group.Add_Part(Ktext);

#ifdef _WIN32
  Am_Object MSMtext =
      Am_Text.Create("MSM text")
          .Set(Am_TEXT, MSM_TEST_STRING)
          .Set(Am_FONT, MSM_Font)
          .Set(Am_LEFT, (int)Jtext.Get(Am_LEFT))
          .Set(Am_TOP, (int)Ktext.Get(Am_TOP) + (int)Ktext.Get(Am_HEIGHT) + 4);
  Families_Group.Add_Part(MSMtext);

  Am_Object MSGtext = Am_Text.Create("MSG text")
                          .Set(Am_TEXT, MSG_TEST_STRING)
                          .Set(Am_FONT, MSG_Font)
                          .Set(Am_LEFT, (int)Jtext.Get(Am_LEFT))
                          .Set(Am_TOP, (int)MSMtext.Get(Am_TOP) +
                                           (int)MSMtext.Get(Am_HEIGHT) + 4);
  Families_Group.Add_Part(MSGtext);

  Am_Object MSMPtext = Am_Text.Create("MSMP text")
                           .Set(Am_TEXT, MSMP_TEST_STRING)
                           .Set(Am_FONT, MSMP_Font)
                           .Set(Am_LEFT, (int)Jtext.Get(Am_LEFT))
                           .Set(Am_TOP, (int)MSGtext.Get(Am_TOP) +
                                            (int)MSGtext.Get(Am_HEIGHT) + 4);
  Families_Group.Add_Part(MSMPtext);

  Am_Object MSGPtext = Am_Text.Create("MSGP text")
                           .Set(Am_TEXT, MSGP_TEST_STRING)
                           .Set(Am_FONT, MSGP_Font)
                           .Set(Am_LEFT, (int)Jtext.Get(Am_LEFT))
                           .Set(Am_TOP, (int)MSMPtext.Get(Am_TOP) +
                                            (int)MSMPtext.Get(Am_HEIGHT) + 4);
  Families_Group.Add_Part(MSGPtext);
#endif
}

void
More_Sizes_Init()
{
  More_Sizes_Group = Test_Group.Create("More Sizes");

  Am_Object kanji = Am_Text.Create("kanji3")
                        .Set(Am_TOP, 5)
                        .Set(Am_TEXT, KANJI3)
                        .Set(Am_FONT, Am_Japanese_Font);
  More_Sizes_Group.Add_Part(kanji);

  Am_Object english =
      Am_Text.Create("english")
          .Set(Am_TOP, (int)kanji.Get(Am_TOP))
          .Set(Am_LEFT, (int)kanji.Get(Am_LEFT) + (int)kanji.Get(Am_WIDTH) + 10)
          .Set(Am_TEXT, "Japanese Font Size Test");
  More_Sizes_Group.Add_Part(english);

  Am_Object JStext =
      Am_Text.Create("JS text")
          .Set(Am_TEXT, JS_TEST_STRING)
          .Set(Am_FONT, JS_Font)
          .Set(Am_TOP, (int)kanji.Get(Am_TOP) + (int)kanji.Get(Am_HEIGHT) + 10);
  More_Sizes_Group.Add_Part(JStext);

  Am_Object JMtext = Am_Text.Create("JM text")
                         .Set(Am_TEXT, JM_TEST_STRING)
                         .Set(Am_FONT, JM_Font)
                         .Set(Am_TOP, (int)JStext.Get(Am_TOP) +
                                          (int)JStext.Get(Am_HEIGHT) + 4);
  More_Sizes_Group.Add_Part(JMtext);

  Am_Object JTStext = Am_Text.Create("JTS text")
                          .Set(Am_TEXT, JTS_TEST_STRING)
                          .Set(Am_FONT, JTS_Font)
                          .Set(Am_TOP, (int)JMtext.Get(Am_TOP) +
                                           (int)JMtext.Get(Am_HEIGHT) + 4);
  More_Sizes_Group.Add_Part(JTStext);

  Am_Object JTMtext = Am_Text.Create("JTM text")
                          .Set(Am_TEXT, JTM_TEST_STRING)
                          .Set(Am_FONT, JTM_Font)
                          .Set(Am_TOP, (int)JTStext.Get(Am_TOP) +
                                           (int)JTStext.Get(Am_HEIGHT) + 4);
  More_Sizes_Group.Add_Part(JTMtext);

  Am_Object JLtext = Am_Text.Create("JL text")
                         .Set(Am_TEXT, JL_TEST_STRING)
                         .Set(Am_FONT, JTL_Font)
                         .Set(Am_TOP, (int)JTMtext.Get(Am_TOP) +
                                          (int)JTMtext.Get(Am_HEIGHT) + 4);
  More_Sizes_Group.Add_Part(JLtext);

  Am_Object JTLtext = Am_Text.Create("JTL text")
                          .Set(Am_TEXT, JTL_TEST_STRING)
                          .Set(Am_FONT, JTL_Font)
                          .Set(Am_TOP, (int)JLtext.Get(Am_TOP) +
                                           (int)JLtext.Get(Am_HEIGHT) + 4);
  More_Sizes_Group.Add_Part(JTLtext);

  Am_Object JVLtext = Am_Text.Create("JVL text")
                          .Set(Am_TEXT, JVL_TEST_STRING)
                          .Set(Am_FONT, JVL_Font)
                          .Set(Am_TOP, (int)JTLtext.Get(Am_TOP) +
                                           (int)JTLtext.Get(Am_HEIGHT) + 4);
  More_Sizes_Group.Add_Part(JVLtext);

  Am_Object JTVLtext = Am_Text.Create("JTVL text")
                           .Set(Am_TEXT, JTVL_TEST_STRING)
                           .Set(Am_FONT, JTVL_Font)
                           .Set(Am_TOP, (int)JVLtext.Get(Am_TOP) +
                                            (int)JVLtext.Get(Am_HEIGHT) + 4);
  More_Sizes_Group.Add_Part(JTVLtext);
}

Am_Object
Size_Face_Init(const char *kanji_text, const char *test_string,
               const char *group_name)
{
  Size_Face_Group = Test_Group.Create(group_name);

  Am_Object kanji = Am_Text.Create("kanji1")
                        .Set(Am_TOP, 5)
                        .Set(Am_TEXT, kanji_text)
                        .Set(Am_FONT, Am_Japanese_Font);
  Size_Face_Group.Add_Part(kanji);

  Am_Object english =
      Am_Text.Create("english1")
          .Set(Am_TOP, (int)kanji.Get(Am_TOP))
          .Set(Am_LEFT, (int)kanji.Get(Am_LEFT) + (int)kanji.Get(Am_WIDTH) + 10)
          .Set(Am_TEXT,
               "Japanese Font Test: style(b, i, u, l) x size(ts, tm, tl, tvl)");
  Size_Face_Group.Add_Part(english);

  // cout << "kanji.Get(Am_LEFT)" << (int)kanji.Get(Am_LEFT) << "\n" << flush;
  // cout << "kanji.Get(Am_WIDTH)" << (int)kanji.Get(Am_WIDTH) << "\n" << flush;
  // cout << "kanji.Get(Am_TOP)" << (int)kanji.Get(Am_TOP) << "\n" << flush;
  // cout << "kanji.Get(Am_HEIGHT)" << (int)kanji.Get(Am_HEIGHT) << "\n\n" << flush;

  Am_Object JStext =
      Am_Text.Create("JS text")
          .Set(Am_TEXT, test_string)
          .Set(Am_FONT, JTS_Font)
          .Set(Am_TOP, (int)kanji.Get(Am_TOP) + (int)kanji.Get(Am_HEIGHT) + 10);
  Size_Face_Group.Add_Part(JStext);

  // cout << "JStext.Get(Am_LEFT)" << (int)JStext.Get(Am_LEFT) << "\n" << flush;
  // cout << "JStext.Get(Am_WIDTH)" << (int)JStext.Get(Am_WIDTH) << "\n" << flush;
  // cout << "JStext.Get(Am_TOP)" << (int)JStext.Get(Am_TOP) << "\n" << flush;
  // cout << "JStext.Get(Am_HEIGHT)" << (int)JStext.Get(Am_HEIGHT) << "\n\n" << flush;

  Am_Object JbStext = Am_Text.Create("JbS text")
                          .Set(Am_TEXT, test_string)
                          .Set(Am_FONT, JbTS_Font)
                          .Set(Am_TOP, (int)JStext.Get(Am_TOP) +
                                           (int)JStext.Get(Am_HEIGHT) + 2);
  Size_Face_Group.Add_Part(JbStext);

  Am_Object JiStext = Am_Text.Create("JiS text")
                          .Set(Am_TEXT, test_string)
                          .Set(Am_FONT, JiTS_Font)
                          .Set(Am_TOP, (int)JbStext.Get(Am_TOP) +
                                           (int)JbStext.Get(Am_HEIGHT) + 2);
  Size_Face_Group.Add_Part(JiStext);

  Am_Object JbiStext = Am_Text.Create("JbiS text")
                           .Set(Am_TEXT, test_string)
                           .Set(Am_FONT, JbiTS_Font)
                           .Set(Am_TOP, (int)JiStext.Get(Am_TOP) +
                                            (int)JiStext.Get(Am_HEIGHT) + 2);
  Size_Face_Group.Add_Part(JbiStext);

  Am_Object JuStext = Am_Text.Create("JuS text")
                          .Set(Am_TEXT, test_string)
                          .Set(Am_FONT, JuTS_Font)
                          .Set(Am_TOP, (int)JbiStext.Get(Am_TOP) +
                                           (int)JbiStext.Get(Am_HEIGHT) + 2);
  Size_Face_Group.Add_Part(JuStext);

  Am_Object JbuStext = Am_Text.Create("JbuS text")
                           .Set(Am_TEXT, test_string)
                           .Set(Am_FONT, JbuTS_Font)
                           .Set(Am_TOP, (int)JuStext.Get(Am_TOP) +
                                            (int)JuStext.Get(Am_HEIGHT) + 2);
  Size_Face_Group.Add_Part(JbuStext);

  Am_Object JiuStext = Am_Text.Create("JiuS text")
                           .Set(Am_TEXT, test_string)
                           .Set(Am_FONT, JiuTS_Font)
                           .Set(Am_TOP, (int)JbuStext.Get(Am_TOP) +
                                            (int)JbuStext.Get(Am_HEIGHT) + 2);
  Size_Face_Group.Add_Part(JiuStext);

  Am_Object JbiuStext = Am_Text.Create("JbiuS text")
                            .Set(Am_TEXT, test_string)
                            .Set(Am_FONT, JbiuTS_Font)
                            .Set(Am_TOP, (int)JiuStext.Get(Am_TOP) +
                                             (int)JiuStext.Get(Am_HEIGHT) + 2);
  Size_Face_Group.Add_Part(JbiuStext);

  Am_Object JMtext =
      Am_Text.Create("JM text")
          .Set(Am_TEXT, test_string)
          .Set(Am_FONT, JTM_Font)
          .Set(Am_LEFT,
               (int)JbiuStext.Get(Am_LEFT) + (int)JbiuStext.Get(Am_WIDTH) + 10)
          .Set(Am_TOP, (int)kanji.Get(Am_TOP) + (int)kanji.Get(Am_HEIGHT) + 10);
  Size_Face_Group.Add_Part(JMtext);

  // cout << "JMtext.Get(Am_LEFT)" << (int)JMtext.Get(Am_LEFT) << "\n" << flush;
  // cout << "JMtext.Get(Am_WIDTH)" << (int)JMtext.Get(Am_WIDTH) << "\n" << flush;
  // cout << "JMtext.Get(Am_TOP)" << (int)JMtext.Get(Am_TOP) << "\n" << flush;
  // cout << "JMtext.Get(Am_HEIGHT)" << (int)JMtext.Get(Am_HEIGHT) << "\n\n" << flush;

  Am_Object JbMtext = Am_Text.Create("JbM text")
                          .Set(Am_TEXT, test_string)
                          .Set(Am_FONT, JbTM_Font)
                          .Set(Am_LEFT, (int)JMtext.Get(Am_LEFT))
                          .Set(Am_TOP, (int)JMtext.Get(Am_TOP) +
                                           (int)JMtext.Get(Am_HEIGHT) + 2);
  Size_Face_Group.Add_Part(JbMtext);

  Am_Object JiMtext = Am_Text.Create("JiM text")
                          .Set(Am_TEXT, test_string)
                          .Set(Am_FONT, JiTM_Font)
                          .Set(Am_LEFT, (int)JMtext.Get(Am_LEFT))
                          .Set(Am_TOP, (int)JbMtext.Get(Am_TOP) +
                                           (int)JbMtext.Get(Am_HEIGHT) + 2);
  Size_Face_Group.Add_Part(JiMtext);

  Am_Object JbiMtext = Am_Text.Create("JbiM text")
                           .Set(Am_TEXT, test_string)
                           .Set(Am_FONT, JbiTM_Font)
                           .Set(Am_LEFT, (int)JMtext.Get(Am_LEFT))
                           .Set(Am_TOP, (int)JiMtext.Get(Am_TOP) +
                                            (int)JiMtext.Get(Am_HEIGHT) + 2);
  Size_Face_Group.Add_Part(JbiMtext);

  Am_Object JuMtext = Am_Text.Create("JuM text")
                          .Set(Am_TEXT, test_string)
                          .Set(Am_FONT, JuTM_Font)
                          .Set(Am_LEFT, (int)JMtext.Get(Am_LEFT))
                          .Set(Am_TOP, (int)JbiMtext.Get(Am_TOP) +
                                           (int)JbiMtext.Get(Am_HEIGHT) + 2);
  Size_Face_Group.Add_Part(JuMtext);

  Am_Object JbuMtext = Am_Text.Create("JbuM text")
                           .Set(Am_TEXT, test_string)
                           .Set(Am_FONT, JbuTM_Font)
                           .Set(Am_LEFT, (int)JMtext.Get(Am_LEFT))
                           .Set(Am_TOP, (int)JuMtext.Get(Am_TOP) +
                                            (int)JuMtext.Get(Am_HEIGHT) + 2);
  Size_Face_Group.Add_Part(JbuMtext);

  Am_Object JiuMtext = Am_Text.Create("JiuM text")
                           .Set(Am_TEXT, test_string)
                           .Set(Am_FONT, JiuTM_Font)
                           .Set(Am_LEFT, (int)JMtext.Get(Am_LEFT))
                           .Set(Am_TOP, (int)JbuMtext.Get(Am_TOP) +
                                            (int)JbuMtext.Get(Am_HEIGHT) + 2);
  Size_Face_Group.Add_Part(JiuMtext);

  Am_Object JbiuMtext = Am_Text.Create("JbiuM text")
                            .Set(Am_TEXT, test_string)
                            .Set(Am_FONT, JbiuTM_Font)
                            .Set(Am_LEFT, (int)JMtext.Get(Am_LEFT))
                            .Set(Am_TOP, (int)JiuMtext.Get(Am_TOP) +
                                             (int)JiuMtext.Get(Am_HEIGHT) + 2);
  Size_Face_Group.Add_Part(JbiuMtext);

  Am_Object JLtext =
      Am_Text.Create("JL text")
          .Set(Am_TEXT, test_string)
          .Set(Am_FONT, JTL_Font)
          .Set(Am_LEFT,
               (int)JbiuMtext.Get(Am_LEFT) + (int)JbiuMtext.Get(Am_WIDTH) + 10)
          .Set(Am_TOP, (int)kanji.Get(Am_TOP) + (int)kanji.Get(Am_HEIGHT) + 10);
  Size_Face_Group.Add_Part(JLtext);

  // cout << "JLtext.Get(Am_LEFT)" << (int)JLtext.Get(Am_LEFT) << "\n" << flush;
  // cout << "JLtext.Get(Am_WIDTH)" << (int)JLtext.Get(Am_WIDTH) << "\n" << flush;
  // cout << "JLtext.Get(Am_TOP)" << (int)JLtext.Get(Am_TOP) << "\n" << flush;
  // cout << "JLtext.Get(Am_HEIGHT)" << (int)JLtext.Get(Am_HEIGHT) << "\n\n" << flush;

  Am_Object JbLtext = Am_Text.Create("JbL text")
                          .Set(Am_TEXT, test_string)
                          .Set(Am_FONT, JbTL_Font)
                          .Set(Am_LEFT, (int)JLtext.Get(Am_LEFT))
                          .Set(Am_TOP, (int)JLtext.Get(Am_TOP) +
                                           (int)JLtext.Get(Am_HEIGHT) + 2);
  Size_Face_Group.Add_Part(JbLtext);

  Am_Object JiLtext = Am_Text.Create("JiL text")
                          .Set(Am_TEXT, test_string)
                          .Set(Am_FONT, JiTL_Font)
                          .Set(Am_LEFT, (int)JLtext.Get(Am_LEFT))
                          .Set(Am_TOP, (int)JbLtext.Get(Am_TOP) +
                                           (int)JbLtext.Get(Am_HEIGHT) + 2);
  Size_Face_Group.Add_Part(JiLtext);

  Am_Object JbiLtext = Am_Text.Create("JbiL text")
                           .Set(Am_TEXT, test_string)
                           .Set(Am_FONT, JbiTL_Font)
                           .Set(Am_LEFT, (int)JLtext.Get(Am_LEFT))
                           .Set(Am_TOP, (int)JiLtext.Get(Am_TOP) +
                                            (int)JiLtext.Get(Am_HEIGHT) + 2);
  Size_Face_Group.Add_Part(JbiLtext);

  Am_Object JuLtext = Am_Text.Create("JuL text")
                          .Set(Am_TEXT, test_string)
                          .Set(Am_FONT, JuTL_Font)
                          .Set(Am_LEFT, (int)JLtext.Get(Am_LEFT))
                          .Set(Am_TOP, (int)JbiLtext.Get(Am_TOP) +
                                           (int)JbiLtext.Get(Am_HEIGHT) + 2);
  Size_Face_Group.Add_Part(JuLtext);

  Am_Object JbuLtext = Am_Text.Create("JbuL text")
                           .Set(Am_TEXT, test_string)
                           .Set(Am_FONT, JbuTL_Font)
                           .Set(Am_LEFT, (int)JLtext.Get(Am_LEFT))
                           .Set(Am_TOP, (int)JuLtext.Get(Am_TOP) +
                                            (int)JuLtext.Get(Am_HEIGHT) + 2);
  Size_Face_Group.Add_Part(JbuLtext);

  Am_Object JiuLtext = Am_Text.Create("JiuL text")
                           .Set(Am_TEXT, test_string)
                           .Set(Am_FONT, JiuTL_Font)
                           .Set(Am_LEFT, (int)JLtext.Get(Am_LEFT))
                           .Set(Am_TOP, (int)JbuLtext.Get(Am_TOP) +
                                            (int)JbuLtext.Get(Am_HEIGHT) + 2);
  Size_Face_Group.Add_Part(JiuLtext);

  Am_Object JbiuLtext = Am_Text.Create("JbiuL text")
                            .Set(Am_TEXT, test_string)
                            .Set(Am_FONT, JbiuTL_Font)
                            .Set(Am_LEFT, (int)JLtext.Get(Am_LEFT))
                            .Set(Am_TOP, (int)JiuLtext.Get(Am_TOP) +
                                             (int)JiuLtext.Get(Am_HEIGHT) + 2);
  Size_Face_Group.Add_Part(JbiuLtext);

  Am_Object JVLtext = Am_Text.Create("JVL text")
                          .Set(Am_TEXT, test_string)
                          .Set(Am_FONT, JTVL_Font)
                          .Set(Am_LEFT, (int)JStext.Get(Am_LEFT))
                          .Set(Am_TOP, (int)JbiuLtext.Get(Am_TOP) +
                                           (int)JbiuLtext.Get(Am_HEIGHT) + 20);
  Size_Face_Group.Add_Part(JVLtext);

  // cout << "JVLtext.Get(Am_LEFT)" << (int)JVLtext.Get(Am_LEFT) << "\n" << flush;
  // cout << "JVLtext.Get(Am_WIDTH)" << (int)JVLtext.Get(Am_WIDTH) << "\n" << flush;
  // cout << "JVLtext.Get(Am_TOP)" << (int)JVLtext.Get(Am_TOP) << "\n" << flush;
  // cout << "JVLtext.Get(Am_HEIGHT)" << (int)JVLtext.Get(Am_HEIGHT) << "\n\n" << flush;

  Am_Object JbVLtext = Am_Text.Create("JbVL text")
                           .Set(Am_TEXT, test_string)
                           .Set(Am_FONT, JbTVL_Font)
                           .Set(Am_LEFT, (int)JVLtext.Get(Am_LEFT))
                           .Set(Am_TOP, (int)JVLtext.Get(Am_TOP) +
                                            (int)JVLtext.Get(Am_HEIGHT) + 2);
  Size_Face_Group.Add_Part(JbVLtext);

  Am_Object JiVLtext = Am_Text.Create("JiVL text")
                           .Set(Am_TEXT, test_string)
                           .Set(Am_FONT, JiTVL_Font)
                           .Set(Am_LEFT, (int)JVLtext.Get(Am_LEFT))
                           .Set(Am_TOP, (int)JbVLtext.Get(Am_TOP) +
                                            (int)JbVLtext.Get(Am_HEIGHT) + 2);
  Size_Face_Group.Add_Part(JiVLtext);

  Am_Object JbiVLtext = Am_Text.Create("JbiVL text")
                            .Set(Am_TEXT, test_string)
                            .Set(Am_FONT, JbiTVL_Font)
                            .Set(Am_LEFT, (int)JVLtext.Get(Am_LEFT))
                            .Set(Am_TOP, (int)JiVLtext.Get(Am_TOP) +
                                             (int)JiVLtext.Get(Am_HEIGHT) + 2);
  Size_Face_Group.Add_Part(JbiVLtext);

  Am_Object JuVLtext = Am_Text.Create("JuVL text")
                           .Set(Am_TEXT, test_string)
                           .Set(Am_FONT, JuTVL_Font)
                           .Set(Am_LEFT, (int)JVLtext.Get(Am_LEFT))
                           .Set(Am_TOP, (int)JbiVLtext.Get(Am_TOP) +
                                            (int)JbiVLtext.Get(Am_HEIGHT) + 2);
  Size_Face_Group.Add_Part(JuVLtext);

  Am_Object JbuVLtext = Am_Text.Create("JbuVL text")
                            .Set(Am_TEXT, test_string)
                            .Set(Am_FONT, JbuTVL_Font)
                            .Set(Am_LEFT, (int)JVLtext.Get(Am_LEFT))
                            .Set(Am_TOP, (int)JuVLtext.Get(Am_TOP) +
                                             (int)JuVLtext.Get(Am_HEIGHT) + 2);
  Size_Face_Group.Add_Part(JbuVLtext);

  Am_Object JiuVLtext = Am_Text.Create("JiuVL text")
                            .Set(Am_TEXT, test_string)
                            .Set(Am_FONT, JiuTVL_Font)
                            .Set(Am_LEFT, (int)JVLtext.Get(Am_LEFT))
                            .Set(Am_TOP, (int)JbuVLtext.Get(Am_TOP) +
                                             (int)JbuVLtext.Get(Am_HEIGHT) + 2);
  Size_Face_Group.Add_Part(JiuVLtext);

  Am_Object JbiuVLtext =
      Am_Text.Create("JbiuVL text")
          .Set(Am_TEXT, test_string)
          .Set(Am_FONT, JbiuTVL_Font)
          .Set(Am_LEFT, (int)JVLtext.Get(Am_LEFT))
          .Set(Am_TOP,
               (int)JiuVLtext.Get(Am_TOP) + (int)JiuVLtext.Get(Am_HEIGHT) + 2);
  Size_Face_Group.Add_Part(JbiuVLtext);

  return Size_Face_Group;
}

int
main()
{
  Am_Initialize();

  mywin = Am_Window.Create("mywin")
              .Set(Am_LEFT, 20)
              .Set(Am_TOP, 20)
              .Set(Am_WIDTH, 750)
              .Set(Am_HEIGHT, 500);
  Am_Screen.Add_Part(mywin);

  Am_Object title = Am_Text.Create("title")
                        .Set(Am_LEFT, 10)
                        .Set(Am_TOP, 10)
                        .Set(Am_TEXT, "Japanese I/O Test  Type:  ");
  mywin.Add_Part(title);

  Am_Object instructions1 =
      Am_Text.Create("instructions1")
          .Set(Am_TOP, (int)title.Get(Am_TOP))
          .Set(Am_LEFT, (int)title.Get(Am_LEFT) + (int)title.Get(Am_WIDTH) + 10)
          .Set(Am_TEXT,
               "1 for Size Face, 2 for More Sizes, 3 for Families, 4 for Edit");
  mywin.Add_Part(instructions1);

  Am_Object instructions2 =
      Am_Text.Create("instructions2")
          .Set(Am_TOP, (int)title.Get(Am_TOP) + (int)title.Get(Am_HEIGHT) + 2)
          .Set(Am_LEFT, (int)instructions1.Get(Am_LEFT))
          .Set(Am_TEXT, "5 for Size Face (EUC), 6 for Edit (EUC), 7 for "
                        "Ktable, 8 for Kktable");
  mywin.Add_Part(instructions2);

  Am_Object instructions3 =
      Am_Text.Create("instructions2")
          .Set(Am_TOP, (int)instructions2.Get(Am_TOP) +
                           (int)instructions2.Get(Am_HEIGHT) + 2)
          .Set(Am_LEFT, (int)instructions1.Get(Am_LEFT))
          .Set(Am_TEXT, "9 for Kctable, 0 for Nothing, q (or button) for Quit");
  mywin.Add_Part(instructions3);

  Am_Object qbutton = Am_Button.Create("qbutton")
                          .Set(Am_TOP, (int)title.Get(Am_TOP))
                          .Set(Am_FONT, QBUTTON_FONT);
  qbutton.Get_Object(Am_COMMAND)
      .Set(Am_LABEL, QBUTTON_STRING)
      .Set(Am_DO_METHOD, qmethod);
  qbutton.Set(Am_LEFT,
              (int)mywin.Get(Am_WIDTH) - (int)qbutton.Get(Am_WIDTH) - 10);
  mywin.Add_Part(qbutton);

  Test_Group_Init();
  cout << "Initializing Size Face Group\n" << flush;
  Size_Face_Group = Size_Face_Init(KANJI, TEST_STRING, "Size Face");
  cout << "Initializing Size Face (EUC) Group\n" << flush;
  Size_Face_EUC_Group =
      Size_Face_Init(EUC_KANJI, EUC_TEST_STRING, "Size Face EUC");
  cout << "Initializing More Sizes Group\n" << flush;
  More_Sizes_Init();
  cout << "Initializing Families Group\n" << flush;
  Families_Init();
  cout << "Initializing Edit Group\n" << flush;
  Edit_Group = Edit_Init(KANJIE, PTEXT_STRING);
  cout << "Initializing Edit Group (EUC) \n" << flush;
  Edit_EUC_Group = Edit_Init(EUC_KANJIE, EUC_PTEXT_STRING);
  cout << "Initializing Ktable Group\n" << flush;
  Ktable_Group =
      Ktable_Init(KLINE_FONT, KTITLE, "Japanese Kanji Code Table", "Ktable");
  cout << "Initializing Kktable Group\n" << flush;
  Kktable_Group =
      Ktable_Init(KKLINE_FONT, KKTITLE, "Korean Code Table", "Kktable");
  cout << "Initializing Kctable Group\n" << flush;
  Kctable_Group =
      Ktable_Init(KCLINE_FONT, KCTITLE, "Chinese Code Table", "Kctable");

  Am_Object how_set_inter = Am_One_Shot_Interactor.Create("change_settings")
                                .Set(Am_START_WHEN, "ANY_KEYBOARD")
                                .Set(Am_START_WHERE_TEST, true);

  Am_Object cmd = how_set_inter.Get(Am_COMMAND);
  cmd.Set(Am_DO_METHOD, change_setting);
  mywin.Add_Part(how_set_inter);

  mywin.Add_Part(DISPLAY_PART, Test_Group);

  Am_Main_Event_Loop();
  Am_Cleanup();

  return 0;
}
