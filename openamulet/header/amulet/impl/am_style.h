#ifndef GDEFS_STYLE_H
#define GDEFS_STYLE_H

#include "gem_flags.h"
#include "am_image_array.h"


// Styles include all the drawing properties, including the color and
// various line and fill control styles.
// The color part uses red, green, blue that go from 0.0 to 1.0, where
// 1.0 is full on.

class Am_Style_Data;

class _OA_DL_CLASSIMPORT Am_Style
 {
  Am_WRAPPER_DECL (Am_Style)
public:
    //creators
  Am_Style ();
  
  //full properties create
   Am_Style (float r, float g, float b,  //color part
	     short thickness = 0,
	     Am_Line_Cap_Style_Flag cap = Am_CAP_BUTT,
	     Am_Join_Style_Flag join = Am_JOIN_MITER,
	     Am_Line_Solid_Flag line_flag = Am_LINE_SOLID,
	     const char* dash_l = Am_DEFAULT_DASH_LIST,
	     int dash_l_length = Am_DEFAULT_DASH_LIST_LENGTH,
	     Am_Fill_Solid_Flag fill_flag = Am_FILL_SOLID,
	     Am_Fill_Poly_Flag poly = Am_FILL_POLY_EVEN_ODD,
	     Am_Image_Array stipple = Am_No_Image);
  
  Am_Style (const char* color_name,
	    short thickness = 0,
	    Am_Line_Cap_Style_Flag cap = Am_CAP_BUTT,
	    Am_Join_Style_Flag join = Am_JOIN_MITER,
	    Am_Line_Solid_Flag line_flag = Am_LINE_SOLID,
	    const char *dash_l = Am_DEFAULT_DASH_LIST,
	    int dash_l_length = Am_DEFAULT_DASH_LIST_LENGTH,
	    Am_Fill_Solid_Flag fill_flag = Am_FILL_SOLID,
	    Am_Fill_Poly_Flag poly = Am_FILL_POLY_EVEN_ODD,
	    Am_Image_Array stipple = Am_No_Image);

  _OA_DL_MEMBERIMPORT bool operator== (const Am_Style& style) const;
  _OA_DL_MEMBERIMPORT bool operator!= (const Am_Style& style) const;

  static Am_Style Thick_Line (unsigned short thickness); 
  static Am_Style Halftone_Stipple (int percent,
				    Am_Fill_Solid_Flag
				    fill_flag = Am_FILL_STIPPLED);
  _OA_DL_MEMBERIMPORT Am_Style Clone_With_New_Color (Am_Style& foreground) const;

  // accessor
  _OA_DL_MEMBERIMPORT void Get_Values (float& r, float& g, float& b) const;
  
  _OA_DL_MEMBERIMPORT void Get_Values (short& thickness,
		   Am_Line_Cap_Style_Flag& cap,
		   Am_Join_Style_Flag& join,
		   Am_Line_Solid_Flag& line_flag,
		   const char*& dash_l, int& dash_l_length,
		   Am_Fill_Solid_Flag& fill_flag,
		   Am_Fill_Poly_Flag& poly,
		   Am_Image_Array& stipple) const;
  
  _OA_DL_MEMBERIMPORT void Get_Values (float& r, float& g, float& b,
		   short& thickness,
		   Am_Line_Cap_Style_Flag& cap,
		   Am_Join_Style_Flag& join,
		   Am_Line_Solid_Flag& line_flag,
		   const char*& dash_l, int& dash_l_length,
		   Am_Fill_Solid_Flag& fill_flag,
		   Am_Fill_Poly_Flag& poly,
		   Am_Image_Array& stipple) const;

  _OA_DL_MEMBERIMPORT Am_Fill_Solid_Flag Get_Fill_Flag() const;
  _OA_DL_MEMBERIMPORT Am_Line_Solid_Flag Get_Line_Flag() const;
  _OA_DL_MEMBERIMPORT Am_Image_Array Get_Stipple() const;
  _OA_DL_MEMBERIMPORT Am_Fill_Poly_Flag Get_Fill_Poly_Flag () const;
  
  //Get the properties needed to calculate the line width 
  _OA_DL_MEMBERIMPORT void Get_Line_Thickness_Values (short& thickness,
				  Am_Line_Cap_Style_Flag& cap) const;

  const char* Get_Color_Name () const;
  //returns a pointer to the string, don't dealloc
  
  void Add_Image (Am_Image_Array image);
};

_OA_DL_IMPORT extern Am_Style Am_No_Style;

#endif //GDEFS_STYLE_H
