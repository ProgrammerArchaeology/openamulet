/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file contains functions for the style objects in Gem. */

#include <iostream>

#include <am_inc.h>

#include GEM__H
#include GEMG__H
#include REGISTRY__H

// // // // // // // // // // // // // // // // // // // // // // // // // //
// Am_Style
// // // // // // // // // // // // // // // // // // // // // // // // // //

Am_WRAPPER_IMPL (Am_Style);

static Am_Style default_style (0.0,0.0,0.0); // used by Get_Values

const char* Am_Style::Get_Color_Name () const
{
  if (data)
    return data->color_name;
  else
    return (0L);
}

void Am_Style::Get_Values (float& r, float& g, float& b) const
{
  if (data) {
    r = data->red;
    g = data->green;
    b = data->blue;
  }
  else {
    r = g = b = 0.0;
  }
}

void Am_Style::Get_Values (short& thickness,
			   Am_Line_Cap_Style_Flag& cap,
			   Am_Join_Style_Flag& join,
			   Am_Line_Solid_Flag& line_flag,
			   const char*& dash_l, int& dash_l_length,
			   Am_Fill_Solid_Flag& fill_flag,
			   Am_Fill_Poly_Flag& poly,
			   Am_Image_Array& stipple) const
{
  if (data) {
    thickness = data->line_thickness;
    cap = data->cap_style;
    join = data->join_style;
    line_flag = data->line_solid;
    dash_l = data->dash_list;
    dash_l_length = data->dash_list_length;
    fill_flag = data->fill_solid;
    poly = data->fill_poly;
    stipple = data->stipple_bitmap;
  }
  else {
    // return default values
    default_style.Get_Values (thickness, cap, join, line_flag, dash_l,
			      dash_l_length, fill_flag, poly, stipple);
  }
}

void Am_Style::Get_Values (float& r, float& g, float& b,
			   short& thickness,
			   Am_Line_Cap_Style_Flag& cap,
			   Am_Join_Style_Flag& join,
			   Am_Line_Solid_Flag& line_flag,
			   const char*& dash_l, int& dash_l_length,
			   Am_Fill_Solid_Flag& fill_flag,
			   Am_Fill_Poly_Flag& poly,
			   Am_Image_Array& stipple) const
{
  Get_Values (r, g, b);
  Get_Values (thickness, cap, join, line_flag, dash_l, dash_l_length,
                fill_flag, poly, stipple);
}

Am_Fill_Solid_Flag Am_Style::Get_Fill_Flag () const
{
  if (data)
    return data->fill_solid;
  else
    return Am_FILL_SOLID;
}

Am_Line_Solid_Flag Am_Style::Get_Line_Flag () const
{
  if (data)
    return data->line_solid;
  else
    return Am_LINE_SOLID;
}

Am_Fill_Poly_Flag Am_Style::Get_Fill_Poly_Flag () const
{
  if (data)
    return data->fill_poly;
  else
    return Am_FILL_POLY_EVEN_ODD;
}

Am_Image_Array Am_Style::Get_Stipple () const
{
  if (data)
    return data->stipple_bitmap;
  else
    return Am_No_Image;
}

void Am_Style::Get_Line_Thickness_Values (short& thickness,
					 Am_Line_Cap_Style_Flag& cap) const
{
  if (data) {
    thickness = data->line_thickness;
    cap = data->cap_style;
  }
  else {
    default_style.Get_Line_Thickness_Values (thickness, cap);
  }
}
    
Am_Style::Am_Style (float r, float g, float b,  //color part
		    short thickness,
		    Am_Line_Cap_Style_Flag cap,
		    Am_Join_Style_Flag join,
		    Am_Line_Solid_Flag line_flag,
		    const char *dash_l, int dash_l_length,
		    Am_Fill_Solid_Flag fill_flag,
		    Am_Fill_Poly_Flag poly,
		    // stipple must be an opal bitmap object
		    Am_Image_Array stipple)
{
  data = new Am_Style_Data (r, g, b, thickness, cap, join,
			    line_flag, dash_l, dash_l_length,
			    fill_flag, poly, stipple);
}

Am_Style::Am_Style (const char* color_name,
		    short thickness,
		    Am_Line_Cap_Style_Flag cap,
		    Am_Join_Style_Flag join,
		    Am_Line_Solid_Flag line_flag,
		    const char* dash_l, int dash_l_length,
		    Am_Fill_Solid_Flag fill_flag,
		    Am_Fill_Poly_Flag poly,
		    // stipple must be an opal bitmap object
		    Am_Image_Array stipple)
{
  data = new Am_Style_Data (color_name, thickness, cap, join,
			    line_flag, dash_l, dash_l_length,
			    fill_flag, poly, stipple);
}

// Do not use this 
// Am_Style::Am_Style (char* bit_data, int height, int width)
// {
//   data = new Am_Style_Data (bit_data, height, width);
// }

Am_Style::Am_Style () : data(0L)
{ }

Am_Style Am_Style::Halftone_Stipple (int percent,
				     Am_Fill_Solid_Flag fill_flag)
{
  Am_Style style (new Am_Style_Data(percent, fill_flag));
  return style;
}

Am_Style Am_Style::Thick_Line (unsigned short thickness)
{
  Am_Style style (0, 0, 0, thickness);
  return style;
}

Am_Style Am_Style::Clone_With_New_Color (Am_Style& foreground) const
{
  return new Am_Style_Data (data, foreground.data);
}

bool Am_Style::operator== (const Am_Style& style) const
{
  return data == style.data;
}

bool Am_Style::operator!= (const Am_Style& style) const
{
  return data != style.data;
}

void Am_Style::Add_Image(Am_Image_Array image)
{
  if (data)
    data = (Am_Style_Data *)data->Make_Unique();
  else
    data = new Am_Style_Data (0.0, 0.0, 0.0, 0, Am_CAP_BUTT, Am_JOIN_MITER,
			      Am_LINE_SOLID, Am_DEFAULT_DASH_LIST,
			      Am_DEFAULT_DASH_LIST_LENGTH, Am_FILL_SOLID,
			      Am_FILL_POLY_EVEN_ODD, 0);
  data->stipple_bitmap = image;
  data->fill_solid = Am_FILL_STIPPLED;
}

// Constant values

Am_Style Am_No_Style;

Am_Style_Data Am_On_Bits_Data ("Am_On_Bits", true);
Am_Style Am_On_Bits (&Am_On_Bits_Data);

Am_Style_Data Am_Off_Bits_Data ("Am_Off_Bits", false);
Am_Style Am_Off_Bits ((Am_Wrapper*)&Am_Off_Bits_Data);


// // // // // // // // // // // // // // // // // // // // // // // // // //
// Am_Style_Data
// // // // // // // // // // // // // // // // // // // // // // // // // //

Am_WRAPPER_DATA_IMPL (Am_Style, (this));

Am_Style_Data::Am_Style_Data (Am_Style_Data* proto)
{
  red = proto->red;
  green = proto->green;
  blue = proto->blue;
  line_thickness = proto->line_thickness;
  cap_style = proto->cap_style;
  join_style = proto->join_style;
  line_solid = proto->line_solid;
  dash_list_length = proto->dash_list_length;
  dash_list = new char [dash_list_length];
  memcpy (dash_list, proto->dash_list, dash_list_length * sizeof(char));
  fill_solid = proto->fill_solid;
  fill_poly = proto->fill_poly;
  stipple_bitmap = proto->stipple_bitmap;
  if (proto->color_name) {
    char* color_hold = new char [strlen (proto->color_name) + 1];
    strcpy (color_hold, proto->color_name);
    color_name = color_hold;
  }
  else
    color_name = (0L);
}
  
Am_Style_Data::Am_Style_Data (float r, float g, float b,
               short thickness,  Am_Line_Cap_Style_Flag cap,
               Am_Join_Style_Flag join,  Am_Line_Solid_Flag line_flag,
               const char* dash_l, int dash_l_length,
               Am_Fill_Solid_Flag fill_flag,
               Am_Fill_Poly_Flag poly,  Am_Image_Array stipple)
{
  red = r;  green = g;  blue = b;
  line_thickness = thickness;
  cap_style = cap;
  join_style = join;
  line_solid = line_flag;
  dash_list_length = dash_l_length;
  dash_list = new char [dash_list_length];
  memcpy (dash_list, dash_l, dash_list_length * sizeof(char));
  fill_solid = fill_flag;
  fill_poly = poly;
  stipple_bitmap = stipple;
  color_name = (0L);
}

Am_Style_Data::Am_Style_Data (const char * cname,
	      short thickness,  Am_Line_Cap_Style_Flag cap,
	      Am_Join_Style_Flag join,  Am_Line_Solid_Flag line_flag,
	      const char* dash_l, int dash_l_length,
	      Am_Fill_Solid_Flag fill_flag,
              Am_Fill_Poly_Flag poly,  Am_Image_Array stipple)
{
  char* color_hold = new char [strlen (cname) + 1];
  strcpy (color_hold, cname);
  color_name = color_hold;
  line_thickness = thickness;
  cap_style = cap;
  join_style = join;
  line_solid = line_flag;
  dash_list_length = dash_l_length;
  dash_list = new char [dash_list_length];
  memcpy (dash_list, dash_l, dash_list_length);
  fill_solid = fill_flag;
  fill_poly = poly;
  stipple_bitmap = stipple;
#ifdef DEBUG
  Am_Register_Name(this, cname);
#endif
}

Am_Style_Data::Am_Style_Data (int percent, Am_Fill_Solid_Flag fill_flag) 
{
  red = 0.0;  green = 0.0;  blue = 0.0;
  line_thickness = 0;
  cap_style = Am_CAP_BUTT;
  join_style = Am_JOIN_MITER;
  line_solid = Am_LINE_SOLID;
  const char* dash_l = Am_DEFAULT_DASH_LIST;
  dash_list_length = Am_DEFAULT_DASH_LIST_LENGTH;
  dash_list = new char [dash_list_length];
  memcpy (dash_list, dash_l, dash_list_length);
  fill_solid = fill_flag;
  fill_poly = Am_FILL_POLY_EVEN_ODD;
  stipple_bitmap = Am_Image_Array(percent);
  color_name = (0L);
}

Am_Style_Data::Am_Style_Data (const char* name, bool bit_is_on)
{
  line_thickness = 0;
  cap_style = Am_CAP_BUTT;
  join_style = Am_JOIN_MITER;
  line_solid = Am_LINE_SOLID;
  const char* dash_l = Am_DEFAULT_DASH_LIST;
  dash_list_length = Am_DEFAULT_DASH_LIST_LENGTH;
  dash_list = new char [dash_list_length];
  memcpy (dash_list, dash_l, dash_list_length);
  fill_solid = Am_FILL_SOLID;
  fill_poly = Am_FILL_POLY_EVEN_ODD;
  stipple_bitmap = Am_No_Image;
  color_name = name;
#ifdef DEBUG
  Am_Register_Name(this, name);
#endif
#if 0
  // TODO: Check what is this ??? (main_color was a XColor)
  if (bit_is_on)
    main_color.pixel = 1;
  else
    main_color.pixel = 0;
#endif
  Note_Reference ();
}

Am_Style_Data::Am_Style_Data (Am_Style_Data* proto, Am_Style_Data* new_color)
{
  red = new_color->red;
  green = new_color->green;
  blue = new_color->blue;
  if (new_color->color_name) {
    char* color_hold = new char [strlen (new_color->color_name) + 1];
    strcpy (color_hold, new_color->color_name);
    color_name = color_hold;
  }
  else
    color_name = new_color->color_name;
  line_thickness = proto->line_thickness;
  cap_style = proto->cap_style;
  join_style = proto->join_style;
  line_solid = proto->line_solid;
  dash_list_length = proto->dash_list_length;
  dash_list = new char [dash_list_length];
  memcpy (dash_list, proto->dash_list, dash_list_length * sizeof(char));
  fill_solid = proto->fill_solid;
  fill_poly = proto->fill_poly;
  stipple_bitmap = proto->stipple_bitmap;
}

Am_Style_Data::~Am_Style_Data ()
{
  // TODO: Add a 'color_name_allocated' flag to indicate that you must delete
  //  if (color_name) delete (char*)color_name; // TODO: Check this one
  if (dash_list) delete [] dash_list;
}

void Am_Style_Data::Print (std::ostream& os) const
{
#ifdef DEBUG
  os << Am_Get_Name_Of_Item(this);
#else
  os << "Am_Style(" <<std::hex << (unsigned long)this <<std::dec << ")=[color=";
  if (color_name)
    os << color_name;
  else
    os << "(" << red << "," << green << "," << blue << ")";
  os << " thickness=" << line_thickness
     << " ...]";  //don't bother with the other fields
#endif
}
