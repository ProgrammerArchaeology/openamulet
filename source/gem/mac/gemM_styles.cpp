/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file contains functions for the style objects in Gem.

*/

#include <am_inc.h>

#include GEM__H
#include UNIV_MAP__H
#include <gemM.h>

// // // // // // // // // // // // // // // // // // // // // // // // // //
// Color name ---> RGBColor mapping
// // // // // // // // // // // // // // // // // // // // // // // // // //

static RGBColor rgb_NullValue = {0, 0, 0};

AM_DECL_MAP(Str2RGB, const char *, RGBColor)
AM_IMPL_MAP(Str2RGB, const char *, (0L), RGBColor, rgb_NullValue);

static Am_Map_Str2RGB *color_map;

inline RGBColor
RGB(int r, int g, int b)
{
  RGBColor color = {((long)r << 8) + r, ((long)g << 8) + g, ((long)b << 8) + b};
  return color;
};

void
Am_Initialize_Color_Map()
{
  static bool initialized = false;

  if (!initialized) {
    color_map = new Am_Map_Str2RGB;

    Am_Map_Str2RGB &map = *color_map;

    // adapted from /usr/lib/X11/rgb.txt
    map["white"] = RGB(255, 255, 255);
    map["black"] = RGB(0, 0, 0);
    map["red"] = RGB(246, 34, 23);
    map["green"] = RGB(95, 251, 23);
    map["blue"] = RGB(21, 53, 255);
    map["yellow"] = RGB(255, 252, 23);
    map["magenta"] = RGB(244, 62, 255);
    map["cyan"] = RGB(87, 254, 255);
    map["orange"] = RGB(248, 122, 23);
    map["orchid"] = RGB(229, 125, 237);
    map["plum"] = RGB(185, 59, 143);
    map["grey"] = RGB(115, 110, 111);
    map["purple"] = RGB(142, 53, 239);
    map["beige"] = RGB(245, 243, 215);
    map["ltgray"] = RGB(160, 159, 157);
    map["dkgray"] = RGB(80, 80, 80);

    initialized = true;
  }
}

// // // // // // // // // // // // // // // // // // // // // // // // // //
// Styles
// // // // // // // // // // // // // // // // // // // // // // // // // //

AM_WRAPPER_IMPL(Am_Style);

const char *
Am_Style::Get_Color_Name() const
{
  if (data)
    return data->color_name;
  else
    return (0L);
}

void
Am_Style::Get_Values(float &r, float &g, float &b) const
{
  if (data) {
    r = data->red;
    g = data->green;
    b = data->blue;
  }
}

void
Am_Style::Get_Values(short &thickness, Am_Line_Cap_Style_Flag &cap,
                     Am_Join_Style_Flag &join, Am_Line_Solid_Flag &line_flag,
                     const char *&dash_l, int &dash_l_length,
                     Am_Fill_Solid_Flag &fill_flag, Am_Fill_Poly_Flag &poly,
                     Am_Image_Array &stipple) const
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
}

void
Am_Style::Get_Values(float &r, float &g, float &b, short &thickness,
                     Am_Line_Cap_Style_Flag &cap, Am_Join_Style_Flag &join,
                     Am_Line_Solid_Flag &line_flag, const char *&dash_l,
                     int &dash_l_length, Am_Fill_Solid_Flag &fill_flag,
                     Am_Fill_Poly_Flag &poly, Am_Image_Array &stipple) const
{
  if (data) {
    r = data->red;
    g = data->green;
    b = data->blue;
    Get_Values(thickness, cap, join, line_flag, dash_l, dash_l_length,
               fill_flag, poly, stipple);
  }
}

Am_Fill_Solid_Flag
Am_Style::Get_Fill_Flag() const
{
  if (data)
    return data->fill_solid;
  else
    return Am_FILL_SOLID;
}

Am_Line_Solid_Flag
Am_Style::Get_Line_Flag() const
{
  if (data)
    return data->line_solid;
  else
    return Am_LINE_SOLID;
}

Am_Fill_Poly_Flag
Am_Style::Get_Fill_Poly_Flag() const
{
  if (data)
    return data->fill_poly;
  else
    return Am_FILL_POLY_EVEN_ODD;
}

Am_Image_Array
Am_Style::Get_Stipple() const
{
  if (data)
    return data->stipple_bitmap;
  else
    return Am_No_Image;
}

void
Am_Style::Get_Line_Thickness_Values(short &thickness,
                                    Am_Line_Cap_Style_Flag &cap) const
{
  if (data) {
    thickness = data->line_thickness;
    cap = data->cap_style;
  }
}

Am_Style::Am_Style(float r, float g, float b, //color part
                   short thickness, Am_Line_Cap_Style_Flag cap,
                   Am_Join_Style_Flag join, Am_Line_Solid_Flag line_flag,
                   const char *dash_l, int dash_l_length,
                   Am_Fill_Solid_Flag fill_flag, Am_Fill_Poly_Flag poly,
                   // stipple must be an opal bitmap object
                   Am_Image_Array stipple)
{
  data = new Am_Style_Data(r, g, b, thickness, cap, join, line_flag, dash_l,
                           dash_l_length, fill_flag, poly, stipple);
}

Am_Style::Am_Style(const char *color_name, short thickness,
                   Am_Line_Cap_Style_Flag cap, Am_Join_Style_Flag join,
                   Am_Line_Solid_Flag line_flag, const char *dash_l,
                   int dash_l_length, Am_Fill_Solid_Flag fill_flag,
                   Am_Fill_Poly_Flag poly,
                   // stipple must be an opal bitmap object
                   Am_Image_Array stipple)
{
  data = new Am_Style_Data(color_name, thickness, cap, join, line_flag, dash_l,
                           dash_l_length, fill_flag, poly, stipple);
}

Am_Style::Am_Style() { data = (0L); }

Am_Style
Am_Style::Halftone_Stipple(int percent, Am_Fill_Solid_Flag fill_flag)
{
  Am_Style sty(new Am_Style_Data(percent, fill_flag));
  return sty;
}

Am_Style
Am_Style::Thick_Line(unsigned short thickness)
{
  Am_Style style(0, 0, 0, thickness);
  return style;
}

Am_Style
Am_Style::Clone_With_New_Color(Am_Style &foreground) const
{
  return new Am_Style_Data(data, foreground.data);
}

bool
Am_Style::operator==(const Am_Style &style) const
{
  return data == style.data;
}

bool
Am_Style::operator!=(const Am_Style &style) const
{
  return data != style.data;
}

void
Am_Style::Add_Image(Am_Image_Array image)
{
  if (data)
    data = (Am_Style_Data *)data->Make_Unique();
  else
    data = new Am_Style_Data(0.0, 0.0, 0.0, 0, Am_CAP_BUTT, Am_JOIN_MITER,
                             Am_LINE_SOLID, Am_DEFAULT_DASH_LIST,
                             Am_DEFAULT_DASH_LIST_LENGTH, Am_FILL_SOLID,
                             Am_FILL_POLY_EVEN_ODD, 0);
  data->stipple_bitmap = image;
  data->fill_solid = Am_FILL_STIPPLED;
}

Am_Style Am_No_Style;

/* NYI see rich for this, the constructor does not exist (discussed with Alan on 23 May 96) */
/* more info below around line 400 - faulring@cs*/
static Am_Style_Data *on_bits = new Am_Style_Data("Am_On_Bits", true);
Am_Style Am_On_Bits(on_bits);

static Am_Style_Data *off_bits = new Am_Style_Data("Am_Off_Bits", false);
Am_Style Am_Off_Bits(off_bits);

AM_WRAPPER_DATA_IMPL(Am_Style, (this))

Am_Style_Data::Am_Style_Data(Am_Style_Data *proto)
{
  red = proto->red;
  green = proto->green;
  blue = proto->blue;
  line_thickness = proto->line_thickness;
  cap_style = proto->cap_style;
  join_style = proto->join_style;
  line_solid = proto->line_solid;
  dash_list_length = proto->dash_list_length;
  dash_list = new char[dash_list_length];
  memcpy(dash_list, proto->dash_list, dash_list_length * sizeof(char));
  fill_solid = proto->fill_solid;
  fill_poly = proto->fill_poly;
  stipple_bitmap = proto->stipple_bitmap;
  if (proto->color_name) {
    color_name = new char[strlen(proto->color_name) + 1];
    strcpy(color_name, proto->color_name);
  } else
    color_name = (0L);

  mac_color = proto->mac_color;
}

Am_Style_Data::Am_Style_Data(float r, float g, float b, short thickness,
                             Am_Line_Cap_Style_Flag cap,
                             Am_Join_Style_Flag join,
                             Am_Line_Solid_Flag line_flag, const char *dash_l,
                             int dash_l_length, Am_Fill_Solid_Flag fill_flag,
                             Am_Fill_Poly_Flag poly, Am_Image_Array stipple)
{
  red = r;
  green = g;
  blue = b;
  line_thickness = thickness;
  cap_style = cap;
  join_style = join;
  line_solid = line_flag;
  dash_list_length = dash_l_length;
  dash_list = new char[dash_list_length];
  memcpy(dash_list, dash_l, dash_list_length);
  fill_solid = fill_flag;
  fill_poly = poly;
  if (line_flag == Am_LINE_ON_OFF_DASH) {
    if (dash_l_length > 0)
      stipple_bitmap = Am_Image_Array(75);
    else
      stipple_bitmap = Am_Image_Array(50);
  } else
    stipple_bitmap = stipple;
  color_name = 0;

  mac_color.red = 65535.0 * r;
  mac_color.green = 65535.0 * g;
  mac_color.blue = 65535.0 * b;
}

Am_Style_Data::Am_Style_Data(const char *cname, short thickness,
                             Am_Line_Cap_Style_Flag cap,
                             Am_Join_Style_Flag join,
                             Am_Line_Solid_Flag line_flag, const char *dash_l,
                             int dash_l_length, Am_Fill_Solid_Flag fill_flag,
                             Am_Fill_Poly_Flag poly, Am_Image_Array stipple)
{
  Am_Initialize_Color_Map();

  color_name = new char[strlen(cname) + 1];
  strcpy(color_name, cname);
  line_thickness = thickness;
  cap_style = cap;
  join_style = join;
  line_solid = line_flag;
  dash_list_length = dash_l_length;
  dash_list = new char[dash_list_length];
  memcpy(dash_list, dash_l, dash_list_length);
  fill_solid = fill_flag;
  fill_poly = poly;
  if (line_flag == Am_LINE_ON_OFF_DASH) {
    if (dash_l_length > 0)
      stipple_bitmap = Am_Image_Array(75);
    else
      stipple_bitmap = Am_Image_Array(50);
  } else
    stipple_bitmap = stipple;

  mac_color = color_map->GetAt(cname);

  //    mac_color = name_to_mac_color (cname);
}

Am_Style_Data::Am_Style_Data(int percent, Am_Fill_Solid_Flag fill_flag)
{
  red = 0.0;
  green = 0.0;
  blue = 0.0;
  line_thickness = 0;
  cap_style = Am_CAP_BUTT;
  join_style = Am_JOIN_MITER;
  line_solid = Am_LINE_SOLID;
  const char *dash_l = Am_DEFAULT_DASH_LIST;
  dash_list_length = Am_DEFAULT_DASH_LIST_LENGTH;
  dash_list = new char[dash_list_length];
  memcpy(dash_list, dash_l, dash_list_length);
  fill_solid = fill_flag;
  fill_poly = Am_FILL_POLY_EVEN_ODD;
  stipple_bitmap = Am_Image_Array(percent);
  color_name = 0;

  mac_color.red = 0;
  mac_color.green = 0;
  mac_color.blue = 0;
}

/* ** NYI ** 20 May 96
 * The next two constructors are new in V3. I am keeping the same design as
 * in X since it should simpify everything. However, these two constructors
 * have not yet been implemented.
 *  o The first is called above for Am_On_Bits and Am_Off_Bits (approx ln 300).
 *  o The second is called by Am_Style::Clone_With_New_Color. Currently a stub
 *
 * 31 May 96 - implemented but not tested - faulring@cs
 * 27 Jul 96 - tested, appears to work, though nothing was transparent
 */
Am_Style_Data::Am_Style_Data(const char *name, bool bit_is_on)
{
  Am_Initialize_Color_Map();

  color_name = new char[strlen(name) + 1];
  strcpy(color_name, name);
  line_thickness = 0;
  cap_style = Am_CAP_BUTT;
  join_style = Am_JOIN_MITER;
  line_solid = Am_LINE_SOLID;
  const char *dash_l = Am_DEFAULT_DASH_LIST;
  dash_list_length = Am_DEFAULT_DASH_LIST_LENGTH;
  dash_list = new char[dash_list_length];
  memcpy(dash_list, dash_l, dash_list_length);
  fill_solid = Am_FILL_SOLID;
  fill_poly = Am_FILL_POLY_EVEN_ODD;
  stipple_bitmap = Am_No_Image;

  if (bit_is_on) {
    red = green = blue = 0.0f;
    mac_color.red = 0x0000;
    mac_color.green = 0x0000;
    mac_color.blue = 0x0000;
  } else {
    red = green = blue = 1.0f;
    mac_color.red = 0xFFFF;
    mac_color.green = 0xFFFF;
    mac_color.blue = 0xFFFF;
  }
}

Am_Style_Data::Am_Style_Data(Am_Style_Data *proto, Am_Style_Data *new_color)
{
  // get non-color related info from proto
  line_thickness = proto->line_thickness;
  cap_style = proto->cap_style;
  join_style = proto->join_style;
  line_solid = proto->line_solid;
  dash_list_length = proto->dash_list_length;
  dash_list = new char[dash_list_length];
  memcpy(dash_list, proto->dash_list, dash_list_length * sizeof(char));
  fill_solid = proto->fill_solid;
  fill_poly = proto->fill_poly;
  stipple_bitmap = proto->stipple_bitmap;

  // get color-related info from new_color
  red = new_color->red;
  green = new_color->green;
  blue = new_color->blue;
  if (new_color->color_name) {
    color_name = new char[strlen(new_color->color_name) + 1];
    strcpy(color_name, new_color->color_name);
  } else
    color_name = (0L);

  mac_color = new_color->mac_color;
}

// ???? why are we not deallocating the memory allocated for color_name ?????
Am_Style_Data::~Am_Style_Data() {}

/*
//gets the index to use on display
unsigned long Am_Style_Data::Get_Mac_Pixel(Am_Drawonable_Impl* d) {
}
*/

const RGBColor *
Am_Style_Data::Get_Mac_Color(Am_Drawonable_Impl * /* d */)
{
  return &mac_color;
}
