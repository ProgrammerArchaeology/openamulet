/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code has been placed in the public   			          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* Originally written as AmEdit by S.Nebel (Linkworks Ltd Wellington,NZ) 1997.
   Modified and updated by Brad A. Myers of the Amulet project at CMU.
*/

#include "externs.h"

Am_Style n2s[FILL_STYLE_CNT];
Am_Style n2l[LINE_COLOR_CNT][LINE_THICKNESS_CNT];
Am_Font fontarray[FONT_CNT][FONT_SIZE_CNT][FONT_STYLE_CNT];

Am_Define_Method(Am_Object_Method, void, ok_button_pressed_cmd, (Am_Object cmd))
{
  Am_Object owner = cmd.Get_Owner();
  if (owner.Valid())
    Am_Finish_Pop_Up_Waiting(owner.Get(Am_WINDOW), Am_Value(true));
}

Am_Define_Method(Am_Object_Method, void, cancel_button_pressed_cmd,
                 (Am_Object cmd))
{
  Am_Object owner = cmd.Get_Owner();
  if (owner.Valid())
    Am_Finish_Pop_Up_Waiting(owner.Get(Am_WINDOW), Am_Value(false));
}

void
init_styles()
{

  Am_Style tmp[6];
  tmp[0] = Am_Dashed_Line;
  tmp[1] = Am_Dotted_Line;
  tmp[2] = Am_Line_1;
  tmp[3] = Am_Line_2;
  tmp[4] = Am_Line_4;
  tmp[5] = Am_Line_8;

  n2s[0] = Am_No_Style;
  n2s[1] = Am_Red;
  n2s[2] = Am_Green;
  n2s[3] = Am_Blue;
  n2s[4] = Am_Yellow;
  n2s[5] = Am_Purple;
  n2s[6] = Am_Cyan;
  n2s[7] = Am_Orange;
  n2s[8] = Am_Black;
  n2s[9] = Am_White;
  n2s[10] = Am_Amulet_Purple;
  n2s[11] = Am_Motif_Gray;
  n2s[12] = Am_Motif_Light_Gray;
  n2s[13] = Am_Motif_Blue;
  n2s[14] = Am_Motif_Light_Blue;
  n2s[15] = Am_Motif_Green;
  n2s[16] = Am_Motif_Light_Green;
  n2s[17] = Am_Motif_Orange;
  n2s[18] = Am_Motif_Light_Orange;
  n2s[19] = Am_Gray_Stipple;
  n2s[20] = Am_Opaque_Gray_Stipple;
  n2s[21] = Am_Light_Gray_Stipple;
  n2s[22] = Am_Dark_Gray_Stipple;
  n2s[23] = Am_Diamond_Stipple;

  n2l[0][0] = Am_No_Style;
  n2l[1][0] = Am_Red;
  n2l[2][0] = Am_Green;
  n2l[3][0] = Am_Blue;
  n2l[4][0] = Am_Yellow;
  n2l[5][0] = Am_Purple;
  n2l[6][0] = Am_Cyan;
  n2l[7][0] = Am_Orange;
  n2l[8][0] = Am_Black;
  n2l[9][0] = Am_White;
  n2l[10][0] = Am_Amulet_Purple;
  n2l[11][0] = Am_Motif_Gray;
  n2l[12][0] = Am_Motif_Light_Gray;
  n2l[13][0] = Am_Motif_Blue;
  n2l[14][0] = Am_Motif_Light_Blue;
  n2l[15][0] = Am_Motif_Green;
  n2l[16][0] = Am_Motif_Light_Green;
  n2l[17][0] = Am_Motif_Orange;
  n2l[18][0] = Am_Motif_Light_Orange;
  n2l[19][0] = Am_Style::Am_Style(0.6, 0.5, 0.3);

  int j = 0;
  float r, g, b;
  short thickness;
  Am_Line_Cap_Style_Flag cap;
  const char dotted[2] = {2, 2};
  Am_Style s;

  int i;
  for (i = 1; i < LINE_THICKNESS_CNT; i++)
    n2l[0][i] = Am_No_Style;

  for (i = 1; i < LINE_COLOR_CNT; i++) {
    s = n2l[i][0];
    s.Get_Values(r, g, b);
    for (j = 0; j < LINE_THICKNESS_CNT; j++) {
      tmp[j].Get_Line_Thickness_Values(thickness, cap);
      n2l[i][j] =
          Am_Style::Am_Style(r, g, b, thickness, cap, Am_JOIN_MITER,
                             (j > 1) ? Am_LINE_SOLID : Am_LINE_ON_OFF_DASH,
                             (j > 0) ? Am_DEFAULT_DASH_LIST : dotted);
    }
  }

  fontarray[0][0][0] =
      Am_Font(Am_FONT_FIXED, false, false, false, Am_FONT_SMALL);
  fontarray[0][0][1] =
      Am_Font(Am_FONT_FIXED, true, false, false, Am_FONT_SMALL);
  fontarray[0][0][2] =
      Am_Font(Am_FONT_FIXED, false, true, false, Am_FONT_SMALL);
  fontarray[0][0][3] = Am_Font(Am_FONT_FIXED, true, true, false, Am_FONT_SMALL);
  fontarray[0][0][4] =
      Am_Font(Am_FONT_FIXED, false, false, true, Am_FONT_SMALL);
  fontarray[0][0][5] = Am_Font(Am_FONT_FIXED, true, false, true, Am_FONT_SMALL);
  fontarray[0][0][6] = Am_Font(Am_FONT_FIXED, false, true, true, Am_FONT_SMALL);
  fontarray[0][0][7] = Am_Font(Am_FONT_FIXED, true, true, true, Am_FONT_SMALL);

  fontarray[0][1][0] =
      Am_Font(Am_FONT_FIXED, false, false, false, Am_FONT_MEDIUM);
  fontarray[0][1][1] =
      Am_Font(Am_FONT_FIXED, true, false, false, Am_FONT_MEDIUM);
  fontarray[0][1][2] =
      Am_Font(Am_FONT_FIXED, false, true, false, Am_FONT_MEDIUM);
  fontarray[0][1][3] =
      Am_Font(Am_FONT_FIXED, true, true, false, Am_FONT_MEDIUM);
  fontarray[0][1][4] =
      Am_Font(Am_FONT_FIXED, false, false, true, Am_FONT_MEDIUM);
  fontarray[0][1][5] =
      Am_Font(Am_FONT_FIXED, true, false, true, Am_FONT_MEDIUM);
  fontarray[0][1][6] =
      Am_Font(Am_FONT_FIXED, false, true, true, Am_FONT_MEDIUM);
  fontarray[0][1][7] = Am_Font(Am_FONT_FIXED, true, true, true, Am_FONT_MEDIUM);

  fontarray[0][2][0] =
      Am_Font(Am_FONT_FIXED, false, false, false, Am_FONT_LARGE);
  fontarray[0][2][1] =
      Am_Font(Am_FONT_FIXED, true, false, false, Am_FONT_LARGE);
  fontarray[0][2][2] =
      Am_Font(Am_FONT_FIXED, false, true, false, Am_FONT_LARGE);
  fontarray[0][2][3] = Am_Font(Am_FONT_FIXED, true, true, false, Am_FONT_LARGE);
  fontarray[0][2][4] =
      Am_Font(Am_FONT_FIXED, false, false, true, Am_FONT_LARGE);
  fontarray[0][2][5] = Am_Font(Am_FONT_FIXED, true, false, true, Am_FONT_LARGE);
  fontarray[0][2][6] = Am_Font(Am_FONT_FIXED, false, true, true, Am_FONT_LARGE);
  fontarray[0][2][7] = Am_Font(Am_FONT_FIXED, true, true, true, Am_FONT_LARGE);

  fontarray[0][3][0] =
      Am_Font(Am_FONT_FIXED, false, false, false, Am_FONT_VERY_LARGE);
  fontarray[0][3][1] =
      Am_Font(Am_FONT_FIXED, true, false, false, Am_FONT_VERY_LARGE);
  fontarray[0][3][2] =
      Am_Font(Am_FONT_FIXED, false, true, false, Am_FONT_VERY_LARGE);
  fontarray[0][3][3] =
      Am_Font(Am_FONT_FIXED, true, true, false, Am_FONT_VERY_LARGE);
  fontarray[0][3][4] =
      Am_Font(Am_FONT_FIXED, false, false, true, Am_FONT_VERY_LARGE);
  fontarray[0][3][5] =
      Am_Font(Am_FONT_FIXED, true, false, true, Am_FONT_VERY_LARGE);
  fontarray[0][3][6] =
      Am_Font(Am_FONT_FIXED, false, true, true, Am_FONT_VERY_LARGE);
  fontarray[0][3][7] =
      Am_Font(Am_FONT_FIXED, true, true, true, Am_FONT_VERY_LARGE);

  fontarray[1][0][0] =
      Am_Font(Am_FONT_SERIF, false, false, false, Am_FONT_SMALL);
  fontarray[1][0][1] =
      Am_Font(Am_FONT_SERIF, true, false, false, Am_FONT_SMALL);
  fontarray[1][0][2] =
      Am_Font(Am_FONT_SERIF, false, true, false, Am_FONT_SMALL);
  fontarray[1][0][3] = Am_Font(Am_FONT_SERIF, true, true, false, Am_FONT_SMALL);
  fontarray[1][0][4] =
      Am_Font(Am_FONT_SERIF, false, false, true, Am_FONT_SMALL);
  fontarray[1][0][5] = Am_Font(Am_FONT_SERIF, true, false, true, Am_FONT_SMALL);
  fontarray[1][0][6] = Am_Font(Am_FONT_SERIF, false, true, true, Am_FONT_SMALL);
  fontarray[1][0][7] = Am_Font(Am_FONT_SERIF, true, true, true, Am_FONT_SMALL);

  fontarray[1][1][0] =
      Am_Font(Am_FONT_SERIF, false, false, false, Am_FONT_MEDIUM);
  fontarray[1][1][1] =
      Am_Font(Am_FONT_SERIF, true, false, false, Am_FONT_MEDIUM);
  fontarray[1][1][2] =
      Am_Font(Am_FONT_SERIF, false, true, false, Am_FONT_MEDIUM);
  fontarray[1][1][3] =
      Am_Font(Am_FONT_SERIF, true, true, false, Am_FONT_MEDIUM);
  fontarray[1][1][4] =
      Am_Font(Am_FONT_SERIF, false, false, true, Am_FONT_MEDIUM);
  fontarray[1][1][5] =
      Am_Font(Am_FONT_SERIF, true, false, true, Am_FONT_MEDIUM);
  fontarray[1][1][6] =
      Am_Font(Am_FONT_SERIF, false, true, true, Am_FONT_MEDIUM);
  fontarray[1][1][7] = Am_Font(Am_FONT_SERIF, true, true, true, Am_FONT_MEDIUM);

  fontarray[1][2][0] =
      Am_Font(Am_FONT_SERIF, false, false, false, Am_FONT_LARGE);
  fontarray[1][2][1] =
      Am_Font(Am_FONT_SERIF, true, false, false, Am_FONT_LARGE);
  fontarray[1][2][2] =
      Am_Font(Am_FONT_SERIF, false, true, false, Am_FONT_LARGE);
  fontarray[1][2][3] = Am_Font(Am_FONT_SERIF, true, true, false, Am_FONT_LARGE);
  fontarray[1][2][4] =
      Am_Font(Am_FONT_SERIF, false, false, true, Am_FONT_LARGE);
  fontarray[1][2][5] = Am_Font(Am_FONT_SERIF, true, false, true, Am_FONT_LARGE);
  fontarray[1][2][6] = Am_Font(Am_FONT_SERIF, false, true, true, Am_FONT_LARGE);
  fontarray[1][2][7] = Am_Font(Am_FONT_SERIF, true, true, true, Am_FONT_LARGE);

  fontarray[1][3][0] =
      Am_Font(Am_FONT_SERIF, false, false, false, Am_FONT_VERY_LARGE);
  fontarray[1][3][1] =
      Am_Font(Am_FONT_SERIF, true, false, false, Am_FONT_VERY_LARGE);
  fontarray[1][3][2] =
      Am_Font(Am_FONT_SERIF, false, true, false, Am_FONT_VERY_LARGE);
  fontarray[1][3][3] =
      Am_Font(Am_FONT_SERIF, true, true, false, Am_FONT_VERY_LARGE);
  fontarray[1][3][4] =
      Am_Font(Am_FONT_SERIF, false, false, true, Am_FONT_VERY_LARGE);
  fontarray[1][3][5] =
      Am_Font(Am_FONT_SERIF, true, false, true, Am_FONT_VERY_LARGE);
  fontarray[1][3][6] =
      Am_Font(Am_FONT_SERIF, false, true, true, Am_FONT_VERY_LARGE);
  fontarray[1][3][7] =
      Am_Font(Am_FONT_SERIF, true, true, true, Am_FONT_VERY_LARGE);

  fontarray[2][0][0] =
      Am_Font(Am_FONT_SANS_SERIF, false, false, false, Am_FONT_SMALL);
  fontarray[2][0][1] =
      Am_Font(Am_FONT_SANS_SERIF, true, false, false, Am_FONT_SMALL);
  fontarray[2][0][2] =
      Am_Font(Am_FONT_SANS_SERIF, false, true, false, Am_FONT_SMALL);
  fontarray[2][0][3] =
      Am_Font(Am_FONT_SANS_SERIF, true, true, false, Am_FONT_SMALL);
  fontarray[2][0][4] =
      Am_Font(Am_FONT_SANS_SERIF, false, false, true, Am_FONT_SMALL);
  fontarray[2][0][5] =
      Am_Font(Am_FONT_SANS_SERIF, true, false, true, Am_FONT_SMALL);
  fontarray[2][0][6] =
      Am_Font(Am_FONT_SANS_SERIF, false, true, true, Am_FONT_SMALL);
  fontarray[2][0][7] =
      Am_Font(Am_FONT_SANS_SERIF, true, true, true, Am_FONT_SMALL);

  fontarray[2][1][0] =
      Am_Font(Am_FONT_SANS_SERIF, false, false, false, Am_FONT_MEDIUM);
  fontarray[2][1][1] =
      Am_Font(Am_FONT_SANS_SERIF, true, false, false, Am_FONT_MEDIUM);
  fontarray[2][1][2] =
      Am_Font(Am_FONT_SANS_SERIF, false, true, false, Am_FONT_MEDIUM);
  fontarray[2][1][3] =
      Am_Font(Am_FONT_SANS_SERIF, true, true, false, Am_FONT_MEDIUM);
  fontarray[2][1][4] =
      Am_Font(Am_FONT_SANS_SERIF, false, false, true, Am_FONT_MEDIUM);
  fontarray[2][1][5] =
      Am_Font(Am_FONT_SANS_SERIF, true, false, true, Am_FONT_MEDIUM);
  fontarray[2][1][6] =
      Am_Font(Am_FONT_SANS_SERIF, false, true, true, Am_FONT_MEDIUM);
  fontarray[2][1][7] =
      Am_Font(Am_FONT_SANS_SERIF, true, true, true, Am_FONT_MEDIUM);

  fontarray[2][2][0] =
      Am_Font(Am_FONT_SANS_SERIF, false, false, false, Am_FONT_LARGE);
  fontarray[2][2][1] =
      Am_Font(Am_FONT_SANS_SERIF, true, false, false, Am_FONT_LARGE);
  fontarray[2][2][2] =
      Am_Font(Am_FONT_SANS_SERIF, false, true, false, Am_FONT_LARGE);
  fontarray[2][2][3] =
      Am_Font(Am_FONT_SANS_SERIF, true, true, false, Am_FONT_LARGE);
  fontarray[2][2][4] =
      Am_Font(Am_FONT_SANS_SERIF, false, false, true, Am_FONT_LARGE);
  fontarray[2][2][5] =
      Am_Font(Am_FONT_SANS_SERIF, true, false, true, Am_FONT_LARGE);
  fontarray[2][2][6] =
      Am_Font(Am_FONT_SANS_SERIF, false, true, true, Am_FONT_LARGE);
  fontarray[2][2][7] =
      Am_Font(Am_FONT_SANS_SERIF, true, true, true, Am_FONT_LARGE);

  fontarray[2][3][0] =
      Am_Font(Am_FONT_SANS_SERIF, false, false, false, Am_FONT_VERY_LARGE);
  fontarray[2][3][1] =
      Am_Font(Am_FONT_SANS_SERIF, true, false, false, Am_FONT_VERY_LARGE);
  fontarray[2][3][2] =
      Am_Font(Am_FONT_SANS_SERIF, false, true, false, Am_FONT_VERY_LARGE);
  fontarray[2][3][3] =
      Am_Font(Am_FONT_SANS_SERIF, true, true, false, Am_FONT_VERY_LARGE);
  fontarray[2][3][4] =
      Am_Font(Am_FONT_SANS_SERIF, false, false, true, Am_FONT_VERY_LARGE);
  fontarray[2][3][5] =
      Am_Font(Am_FONT_SANS_SERIF, true, false, true, Am_FONT_VERY_LARGE);
  fontarray[2][3][6] =
      Am_Font(Am_FONT_SANS_SERIF, false, true, true, Am_FONT_VERY_LARGE);
  fontarray[2][3][7] =
      Am_Font(Am_FONT_SANS_SERIF, true, true, true, Am_FONT_VERY_LARGE);
}

const char *n2sstr[] = {"Am_No_Style",
                        "Am_Red",
                        "Am_Green",
                        "Am_Blue",
                        "Am_Yellow",
                        "Am_Purple",
                        "Am_Cyan",
                        "Am_Orange",
                        "Am_Black",
                        "Am_White",
                        "Am_Amulet_Purple",
                        "Am_Motif_Gray",
                        "Am_Motif_Light_Gray",
                        "Am_Motif_Blue",
                        "Am_Motif_Light_Blue",
                        "Am_Motif_Green",
                        "Am_Motif_Light_Green",
                        "Am_Motif_Orange",
                        "Am_Motif_Light_Orange",
                        "Am_Gray_Stipple",
                        "Am_Opaque_Gray_Stipple",
                        "Am_Light_Gray_Stipple",
                        "Am_Dark_Gray_Stipple",
                        "Am_Diamond_Stipple"};

const char *n2lstr[LINE_COLOR_CNT][LINE_THICKNESS_CNT] = {
    {"Am_No_Style_Dash", "Am_No_Style_Dotted", "Am_No_Style_1", "Am_No_Style_2",
     "Am_No_Style_4", "Am_No_Style_8"},
    {"Am_Red_Dashed", "Am_Red_Dotted", "Am_Red_1", "Am_Red_2", "Am_Red_4",
     "Am_Red_8"},
    {"Am_Green_Dashed", "Am_Green_Dotted", "Am_Green_1", "Am_Green_2",
     "Am_Green_4", "Am_Green_8"},
    {"Am_Blue_Dashed", "Am_Blue_Dotted", "Am_Blue_1", "Am_Blue_2", "Am_Blue_4",
     "Am_Blue_8"},
    {"Am_Yellow_Dashed", "Am_Yellow_Dotted", "Am_Yellow_1", "Am_Yellow_2",
     "Am_Yellow_4", "Am_Yellow_8"},
    {"Am_Purple_Dashed", "Am_Purple_Dotted", "Am_Purple_1", "Am_Purple_2",
     "Am_Purple_4", "Am_Purple_8"},
    {"Am_Cyan_Dashed", "Am_Cyan_Dotted", "Am_Cyan_1", "Am_Cyan_2", "Am_Cyan_4",
     "Am_Cyan_8"},
    {"Am_Orange_Dashed", "Am_Orange_Dotted", "Am_Orange_1", "Am_Orange_2",
     "Am_Orange_4", "Am_Orange_8"},
    {"Am_Dashed_Line", "Am_Dotted_Line", "Am_Line_1", "Am_Line_2", "Am_Line_4",
     "Am_Line_8"},
    {"Am_White_Dashed", "Am_White_Dotted", "Am_White_1", "Am_White_2",
     "Am_White_4", "Am_White_8"},
    {"Am_Amulet_Purple_Dashed", "Am_Amulet_Purple_Dotted", "Am_Amulet_Purple_1",
     "Am_Amulet_Purple_2", "Am_Amulet_Purple_4", "Am_Amulet_Purple_8"},
    {"Am_Motif_Gray_Dashed", "Am_Motif_Gray_Dotted", "Am_Motif_Gray_1",
     "Am_Motif_Gray_2", "Am_Motif_Gray_4", "Am_Motif_Gray_8"},
    {"Am_Motif_Light_Gray_Dashed", "Am_Motif_Light_Gray_Dotted",
     "Am_Motif_Light_Gray_1", "Am_Motif_Light_Gray_2", "Am_Motif_Light_Gray_4",
     "Am_Motif_Light_Gray_8"},
    {"Am_Motif_Blue_Dashed", "Am_Motif_Blue_Dotted", "Am_Motif_Blue_1",
     "Am_Motif_Blue_2", "Am_Motif_Blue_4", "Am_Motif_Blue_8"},
    {"Am_Motif_Light_Blue_Dashed", "Am_Motif_Light_Blue_Dotted",
     "Am_Motif_Light_Blue_1", "Am_Motif_Light_Blue_2", "Am_Motif_Light_Blue_4",
     "Am_Motif_Light_Blue_8"},
    {"Am_Motif_Green_Dashed", "Am_Motif_Green_Dotted", "Am_Motif_Green_1",
     "Am_Motif_Green_2", "Am_Motif_Green_4", "Am_Motif_Green_8"},
    {"Am_Motif_Light_Green_Dashed", "Am_Motif_Light_Green_Dotted",
     "Am_Motif_Light_Green_1", "Am_Motif_Light_Green_2",
     "Am_Motif_Light_Green_4", "Am_Motif_Light_Green_8"},
    {"Am_Motif_Orange_Dashed", "Am_Motif_Orange_Dotted", "Am_Motif_Orange_1",
     "Am_Motif_Orange_2", "Am_Motif_Orange_4", "Am_Motif_Orange_8"},
    {"Am_Motif_Light_Orange_Dashed", "Am_Motif_Light_Orange_Dotted",
     "Am_Motif_Light_Orange_1", "Am_Motif_Light_Orange_2",
     "Am_Motif_Light_Orange_4", "Am_Motif_Light_Orange_8"}};

const char *layout[] = {"Am_Vertical_Layout", "Am_Horizontal_Layout"};
