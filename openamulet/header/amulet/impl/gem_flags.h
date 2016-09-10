#ifndef GDEFS_FLAGS_H
#define GDEFS_FLAGS_H

class Am_Image_Array;

// the drawing properties supported by gem
enum Am_Draw_Function
{
	Am_DRAW_COPY,
	Am_DRAW_OR,
	Am_DRAW_XOR,
	Am_DRAW_GRAPHIC_OR,
	Am_DRAW_GRAPHIC_XOR,
	Am_DRAW_GRAPHIC_AND,
	Am_DRAW_GRAPHIC_NIMP,
	Am_DRAW_MASK_COPY
};

//for styles: fill tiled not supported
enum Am_Fill_Solid_Flag { Am_FILL_SOLID = 0, Am_FILL_STIPPLED = 2,
			  Am_FILL_OPAQUE_STIPPLED = 3};

enum Am_Fill_Poly_Flag { Am_FILL_POLY_EVEN_ODD = 0, Am_FILL_POLY_WINDING = 1 };

enum Am_Line_Cap_Style_Flag { Am_CAP_NOT_LAST = 0, Am_CAP_BUTT = 1,
			      Am_CAP_ROUND = 2, Am_CAP_PROJECTING = 3 };

enum Am_Join_Style_Flag { Am_JOIN_MITER = 0, Am_JOIN_ROUND = 1,
			  Am_JOIN_BEVEL = 2 };

// double dash not supported
enum Am_Line_Solid_Flag { Am_LINE_SOLID = 0, Am_LINE_ON_OFF_DASH = 1 };

#define Am_DEFAULT_DASH_LIST_VALUE {4,4}
#define Am_DEFAULT_DASH_LIST_LENGTH 2
_OA_DL_IMPORT extern const char Am_DEFAULT_DASH_LIST[2];

enum Am_Arc_Style_Flag { Am_ARC_CHORD = 0, Am_ARC_PIE_SLICE = 1 };

enum Am_Radius_Flag { Am_SMALL_RADIUS = -1, Am_MEDIUM_RADIUS = -2, 
		      Am_LARGE_RADIUS = -3};

//for fonts

enum Am_Font_Family_Flag { 
  Am_FONT_FIXED, Am_FONT_SERIF, Am_FONT_SANS_SERIF,  // Standard fonts for one-byte code
  Am_FONT_JFIXED, // Japanese standard font
  Am_FONT_JPROPORTIONAL, // Japanese proportional font, equals Am_FONT_JFIXED on X
  Am_FONT_CFIXED, // Chinese font, equals Am_FONT_JFIXED on Windows 
  Am_FONT_KFIXED // Korean font, equals Am_FONT_JFIXED on Windows
};

enum Am_Font_Size_Flag { Am_FONT_SMALL = 0, Am_FONT_MEDIUM = 1,
			 Am_FONT_LARGE = 2, Am_FONT_VERY_LARGE = 3,
		// the constant definitions for the sizes of Two-byte code are below 
			Am_FONT_TSMALL = 0x10, Am_FONT_TMEDIUM = 0x11,
			Am_FONT_TLARGE = 0x12, Am_FONT_TVERY_LARGE = 0x13 
};

// This is used to hold the data for pixmaps and bitmaps.
// I assume most users will only want to read from files, so this object 
// is not normally available to users.  (otherwise it should be moved
// to gconstants and be made a subclass of Am_Value)

enum Am_Image_File_Format { XBM_FILE_FORMAT, XPM_FILE_FORMAT, 
			    GIF_FILE_FORMAT };


_OA_DL_IMPORT extern Am_Image_Array Am_No_Image;

// this is a special id we give to the main computer in a multiple user environment
#define Am_USER_ZERO 0      // supports multiple users

#endif // GDEFS_FLAGS_H
