#include <am_inc.h>
#include REGISTRY__H
#include <amulet/impl/am_style.h>
#include <amulet/impl/am_font.h>
#include <amulet/impl/opal_objects.h>

///////////////////////
// Standard Opal styles
// defined in Am_Opal_Initialize ();

Am_Style Am_Red;
Am_Style Am_Green;
Am_Style Am_Blue;
Am_Style Am_Yellow;
Am_Style Am_Purple;
Am_Style Am_Cyan;
Am_Style Am_Orange;
Am_Style Am_Black;
Am_Style Am_Dark_Gray;
Am_Style Am_Gray;
Am_Style Am_Light_Gray;
Am_Style Am_White;
Am_Style Am_Amulet_Purple;

Am_Style Am_Motif_Gray;
Am_Style Am_Motif_Light_Gray;
Am_Style Am_Motif_Blue;
Am_Style Am_Motif_Light_Blue;
Am_Style Am_Motif_Green;
Am_Style Am_Motif_Light_Green;
Am_Style Am_Motif_Orange;
Am_Style Am_Motif_Light_Orange;

// Default color used for all widgets etc., set in Am_Opal_Initialize()
Am_Style Am_Default_Color;

Am_Style Am_Thin_Line;
Am_Style Am_Line_0;
Am_Style Am_Line_1;
Am_Style Am_Line_2;
Am_Style Am_Line_4;
Am_Style Am_Line_8;
Am_Style Am_Dotted_Line;
Am_Style Am_Dashed_Line;

Am_Style Am_Gray_Stipple;
Am_Style Am_Opaque_Gray_Stipple;
Am_Style Am_Light_Gray_Stipple;
Am_Style Am_Dark_Gray_Stipple;
Am_Style Am_Diamond_Stipple;
Am_Style Am_Opaque_Diamond_Stipple;

Am_Font Am_Default_Font (Am_FONT_FIXED, false, false, false, Am_FONT_MEDIUM);
Am_Font Am_Japanese_Font (Am_FONT_JFIXED, false, false, false, Am_FONT_TMEDIUM);
		// Japanese standard font

// Data for the built-in Am_Diamond_Stipple
#if !defined(_WIN32)
static char diamond_bits[] = {
   0x80, 0x00, 0xc0, 0x01, 0xe0, 0x03, 0xf0, 0x07, 0xf8, 0x0f, 0xfc, 0x1f,
   0xfe, 0x3f, 0xff, 0x7f, 0xfe, 0x3f, 0xfc, 0x1f, 0xf8, 0x0f, 0xf0, 0x07,
   0xe0, 0x03, 0xc0, 0x01, 0x80, 0x00, 0x00, 0x00};
#else
static const short diamond_bits[] = {
//	0x07, 0x13, 0x31, 0x70,  // an interesting pattern
//	0x0e, 0x8c, 0xc8, 0xe0   // but not diamond fill
    0x08, 0x1c, 0x3e, 0x7f,
	0xfe, 0x7c, 0x38, 0x10
};
#endif

void init_styles()
{
  Am_Red = Am_Style(1.0f, 0.0f, 0.0f);
  Am_Green = Am_Style(0.0f, 1.0f, 0.0f);
  Am_Blue = Am_Style(0.0f, 0.0f, 1.0f);
  Am_Yellow = Am_Style(1.0f, 1.0f, 0.0f);
  Am_Purple = Am_Style(1.0f, 0.0f, 1.0f);
  Am_Cyan = Am_Style(0.0f, 1.0f, 1.0f);
  Am_Orange = Am_Style(0.75f, 0.25f, 0.0f);
  Am_Black = Am_Style(0.0f, 0.0f, 0.0f);
  Am_Dark_Gray  = Am_Style(0.25f, 0.25f, 0.25f);
  Am_Gray  = Am_Style(0.5f, 0.5f, 0.5f);
  Am_Light_Gray  = Am_Style(0.75f, 0.75f, 0.75f);
  Am_White = Am_Style(1.0f, 1.0f, 1.0f);
  // safe purple colors on PC are FF66FF,FF33FF,FF00FF,FF0099,
  // CC00FF,CC00CC,CC33FF,CC33CC
  // 0xFF=255= 1.0, 0xCC=204=0.8, 0x66=102=0.4, 0x33=51=0.2,
  //try FFCCFF
  Am_Amulet_Purple = Am_Style(1.0f, 0.8f, 1.0f);
  // V3 (0.93f, 0.75, 1.0f);
  // V2: (0.96f, 0.86f, 1.0f);
  // darker purple (too dark) (0.9023f, 0.73828f, 1.0f);


  Am_Motif_Gray = Am_Style(0.83f, 0.83f, 0.83f); // (0xd3d3 / 0xffff)
  Am_Motif_Light_Gray = Am_Style(0.9f, 0.9f, 0.9f);
  Am_Motif_Blue = Am_Style(0.45f, // (0x7272 / 0xffff)
			               0.62f,  // (0x9f9f / 0xffff)
			               1.0f);
  Am_Motif_Light_Blue = Am_Style(0.7217459f, 0.8998047f, 1.0f);
  Am_Motif_Green = Am_Style(0.37f, // (0x5f5f / 0xffff)
                            0.62f, // (0x9e9e / 0xffff)
                            0.63f); //(0xa0a0 / 0xffff))
  Am_Motif_Light_Green = Am_Style(0.62f, 0.87f, 0.7f);
  Am_Motif_Orange = Am_Style(1.0f, 0.6f, 0.4f);
  Am_Motif_Light_Orange = Am_Style(1.0f, 0.91f, 0.72f);

  // This color will be used by all widgets, you can change it to alter
  // the default color appearance of OpenAmulet
#ifdef _WIN32
  Am_Default_Color		= Am_Light_Gray;
#else
  Am_Default_Color		= Am_Motif_Gray;
#endif

  Am_Thin_Line = Am_Black;
  Am_Line_0 = Am_Black;
  Am_Line_1 = Am_Black;
  Am_Line_2 = Am_Style(0.0f, 0.0f, 0.0f, 2);
  Am_Line_4 = Am_Style(0.0f, 0.0f, 0.0f, 4);
  Am_Line_8 = Am_Style(0.0f, 0.0f, 0.0f, 8);
  Am_Dashed_Line = Am_Style(0.0f, 0.0f, 0.0f, 1, Am_CAP_BUTT, Am_JOIN_MITER,
			    Am_LINE_ON_OFF_DASH);

  static const char Am_DOTTED_DASH_LIST[2] = {2, 2};
  Am_Dotted_Line = Am_Style(0.0f, 0.0f, 0.0f, 1, Am_CAP_BUTT, Am_JOIN_MITER,
			    Am_LINE_ON_OFF_DASH, Am_DOTTED_DASH_LIST, 2);

  Am_Gray_Stipple = Am_Style::Halftone_Stipple (50);
  Am_Opaque_Gray_Stipple = Am_Style::Halftone_Stipple (50,
				       Am_FILL_OPAQUE_STIPPLED);
  Am_Light_Gray_Stipple = Am_Style::Halftone_Stipple (25);
  Am_Dark_Gray_Stipple = Am_Style::Halftone_Stipple (75);
#if !defined(_WIN32)
    Am_Diamond_Stipple = Am_Style (0.0f, 0.0f, 0.0f, 0, Am_CAP_BUTT,
			 Am_JOIN_MITER, Am_LINE_ON_OFF_DASH,
			 Am_DEFAULT_DASH_LIST, Am_DEFAULT_DASH_LIST_LENGTH,
			 Am_FILL_STIPPLED, Am_FILL_POLY_EVEN_ODD,
			 (Am_Image_Array (diamond_bits, 16, 16)));
  Am_Opaque_Diamond_Stipple = Am_Style (0.0f, 0.0f, 0.0f, 0, Am_CAP_BUTT,
			    Am_JOIN_MITER, Am_LINE_ON_OFF_DASH,
			    Am_DEFAULT_DASH_LIST, Am_DEFAULT_DASH_LIST_LENGTH,
			    Am_FILL_OPAQUE_STIPPLED, Am_FILL_POLY_EVEN_ODD,
			    (Am_Image_Array (diamond_bits, 16, 16)));
#else
  Am_Diamond_Stipple = Am_Style (0.0f, 0.0f, 0.0f, 0, Am_CAP_BUTT,
			    Am_JOIN_MITER, Am_LINE_ON_OFF_DASH,
			    Am_DEFAULT_DASH_LIST, Am_DEFAULT_DASH_LIST_LENGTH,
			    Am_FILL_STIPPLED, Am_FILL_POLY_EVEN_ODD,
			    (Am_Image_Array ((char*)diamond_bits, 8, 8)));
  Am_Opaque_Diamond_Stipple = Am_Style (0.0f, 0.0f, 0.0f, 0, Am_CAP_BUTT,
			    Am_JOIN_MITER, Am_LINE_ON_OFF_DASH,
			    Am_DEFAULT_DASH_LIST, Am_DEFAULT_DASH_LIST_LENGTH,
			    Am_FILL_OPAQUE_STIPPLED, Am_FILL_POLY_EVEN_ODD,
			    (Am_Image_Array ((char*)diamond_bits, 8, 8)));
#endif

  // Register style names.
  Am_Register_Name (Am_Red, "Am_Red");
  Am_Register_Name (Am_Green, "Am_Green");
  Am_Register_Name (Am_Blue, "Am_Blue");
  Am_Register_Name (Am_Yellow, "Am_Yellow");
  Am_Register_Name (Am_Purple, "Am_Purple");
  Am_Register_Name (Am_Cyan, "Am_Cyan");
  Am_Register_Name (Am_Orange, "Am_Orange");
  Am_Register_Name (Am_White, "Am_White");
  Am_Register_Name (Am_Amulet_Purple, "Am_Amulet_Purple");
  Am_Register_Name (Am_Motif_Gray, "Am_Motif_Gray");

  Am_Register_Name (Am_Motif_Light_Gray, "Am_Motif_Light_Gray");
  Am_Register_Name (Am_Motif_Blue, "Am_Motif_Blue");
  Am_Register_Name (Am_Motif_Light_Blue, "Am_Motif_Light_Blue");
  Am_Register_Name (Am_Motif_Green, "Am_Motif_Green");
  Am_Register_Name (Am_Motif_Light_Green, "Am_Motif_Light_Green");
  Am_Register_Name (Am_Motif_Orange, "Am_Motif_Orange");
  Am_Register_Name (Am_Motif_Light_Orange, "Am_Motif_Light_Orange");

  Am_Register_Name (Am_Line_2, "Am_Line_2");
  Am_Register_Name (Am_Line_4, "Am_Line_4");
  Am_Register_Name (Am_Line_8, "Am_Line_8");
  Am_Register_Name (Am_Dashed_Line, "Am_Dashed_Line");
  Am_Register_Name (Am_Dotted_Line, "Am_Dotted_Line");
  Am_Register_Name (Am_Gray_Stipple, "Am_Gray_Stipple");
  Am_Register_Name (Am_Opaque_Gray_Stipple, "Am_Opaque_Gray_Stipple");
  Am_Register_Name (Am_Light_Gray_Stipple, "Am_Light_Gray_Stipple");
  Am_Register_Name (Am_Dark_Gray_Stipple, "Am_Dark_Gray_Stipple");
  Am_Register_Name (Am_Diamond_Stipple, "  Am_Diamond_Stipple");
  Am_Register_Name (Am_Opaque_Diamond_Stipple, "Am_Opaque_Diamond_Stipple");

  // these all conflict with Am_Black.  When we print the name out,
  // it will be "Am_Black," but we register them all so you can specify any
  // of them in the inspector.
  Am_Register_Name (Am_Thin_Line, "Am_Thin_Line");
  Am_Register_Name (Am_Line_0, "Am_Line_0");
  Am_Register_Name (Am_Line_1, "Am_Line_1");
  Am_Register_Name (Am_Black, "Am_Black");

  Am_Register_Name (Am_Default_Font, "Am_Default_Font");
}
