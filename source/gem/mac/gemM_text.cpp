/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file contains member function definitions for the Am_Drawonable_Impl
   object primarily concerned with drawing objects in windows.
*/

#include <stdlib.h>
#include <stdio.h>

#include <am_inc.h>

#include GDEFS__H
#include GEM__H
#include <gemM.h>

// // // // // // // // // // // // // // // // // // // // // // // // // //
// Fonts
// // // // // // // // // // // // // // // // // // // // // // // // // //

AM_WRAPPER_IMPL(Am_Font);

Am_Font::Am_Font(const char *the_name) { data = new Am_Font_Data(the_name); }

// Defaults:
//   f = Am_FONT_FIXED,
//   is_bold = false,
//   is_italic = false,
//   is_underline = false,
//   s = Am_FONT_MEDIUM
Am_Font::Am_Font(Am_Font_Family_Flag f, bool is_bold, bool is_italic,
                 bool is_underline, Am_Font_Size_Flag s)
{
  data = new Am_Font_Data(f, is_bold, is_italic, is_underline, s);
}

bool
Am_Font::Font_Name_Valid(const char * /* name */)
{
  return true;
}

bool
Am_Font::operator==(const Am_Font &font) const
{
  return data == font.data;
}

bool
Am_Font::operator!=(const Am_Font &font) const
{
  return data != font.data;
}

void
Am_Font::Get(Am_String &font_name, Am_Font_Family_Flag &f, bool &is_bold,
             bool &is_italic, bool &is_underline, Am_Font_Size_Flag &s)
{
  if (data) {
    font_name = data->name;
    f = data->family;
    is_bold = data->bold;
    is_italic = data->italic;
    is_underline = data->underline;
    s = data->size;
  } else
    Am_Error("Am_Font::Get was called on a (0L) font");
}

// // // // // // // // // // // // // // // // // // // // // // // // // //
// Utility functions
// // // // // // // // // // // // // // // // // // // // // // // // // //

AM_WRAPPER_DATA_IMPL(Am_Font, (this))

void
Am_Font_Data::Get_Values(Am_Font_Family_Flag &f, bool &is_bold, bool &is_italic,
                         bool &is_underline, Am_Font_Size_Flag &s) const
{
  f = family;
  is_bold = bold;
  is_italic = italic;
  is_underline = underline;
  s = size;
}

Am_Font_Data::Am_Font_Data(Am_Font_Data *proto)
{
  family = proto->family;
  bold = proto->bold;
  italic = proto->italic;
  underline = proto->underline;
  size = proto->size;
  if (proto->name) {
    name = new char[strlen(proto->name) + 1];
    strcpy(name, proto->name);
  } else
    name = (0L);
  mac_infont = proto->mac_infont;
  mac_outfont = proto->mac_outfont;
  char_width = 0;
}

Am_Font_Data::Am_Font_Data(const char *the_name)
{
  name = new char[strlen(the_name) + 1];
  strcpy(name, the_name);
  // Initialize to (0L), and store value when font is first used
  mac_infont = 0;
  //  mac_outfont = 0;
  char_width = 0;
}

Am_Font_Data::Am_Font_Data(Am_Font_Family_Flag f, bool is_bold, bool is_italic,
                           bool is_underline, Am_Font_Size_Flag s)
{
  family = f;
  bold = is_bold;
  italic = is_italic;
  underline = is_underline;
  size = s;
  // Indicate this is a standard font by zeroing-out the name field
  name = (0L);
  // Initialize to (0L), and store value when font is first used
  mac_infont = 0;
  //  mac_outfont = 0;
  char_width = 0;
}

Am_Font_Data::~Am_Font_Data() {}

int
Am_Font_Data::Char_Width()
{
  return 0;
}

bool
Am_Font_Data::Underline()
{
  return underline;
}

// Note: the following macros are defined in gemM.h:
//   Am_FIXED_FONT_FAMILY      Am_SERIF_FONT_FAMILY  Am_SANS_SERIF_FONT_FAMILY
//   Am_SMALL_POINT_SIZE       Am_MEDIUM_POINT_SIZE  Am_LARGE_POINT_SIZE
//   Am_VERY_LARGE_POINT_SIZE
inline int
amulet_to_mac_family(int am_family)
{
  switch (am_family) {
  case Am_FONT_FIXED:
    return (Am_FIXED_FONT_FAMILY);
    break;
  case Am_FONT_SERIF:
    return (Am_SERIF_FONT_FAMILY);
    break;
  case Am_FONT_SANS_SERIF:
    return (Am_SANS_SERIF_FONT_FAMILY);
    break;
  }
  return 0; // Avoid compiler warning (shouldn't ever get here)
}

inline int
amulet_to_mac_face(int am_bold, int am_italic, int am_under)
{
  int mac_face = 0;
  if (am_bold)
    mac_face += 1;
  if (am_italic)
    mac_face += 2;
  if (am_under)
    mac_face += 4;
  return mac_face;
}

inline int
amulet_to_mac_size(int am_size)
{
  switch (am_size) {
  case Am_FONT_SMALL:
    return (Am_SMALL_POINT_SIZE);
    break;
  case Am_FONT_MEDIUM:
    return (Am_MEDIUM_POINT_SIZE);
    break;
  case Am_FONT_LARGE:
    return (Am_LARGE_POINT_SIZE);
    break;
  case Am_FONT_VERY_LARGE:
    return (Am_VERY_LARGE_POINT_SIZE);
    break;
  }
  return 0; // Avoid compiler warning (shouldn't ever get here)
}

void
Am_Font_Data::install_standard_infont(Am_Drawonable_Impl * /* d */)
{
  static Point scale_pt = {1, 1};
  // Allocate Mac-specific "input" data structure
  mac_infont = (FMInput *)NewPtr(sizeof(FMInput));
  // Install data
  mac_infont->needBits = true;
  mac_infont->device = 0;
  mac_infont->numer = scale_pt;
  mac_infont->denom = scale_pt;
  mac_infont->family = amulet_to_mac_family(family);
  mac_infont->face = amulet_to_mac_face(bold, italic, underline);
  mac_infont->size = amulet_to_mac_size(size);
}

// Not Implemented Yet -- just use parameters for default font
void
Am_Font_Data::install_nonstandard_infont(Am_Drawonable_Impl * /* d */)
{
  static Point scale_pt = {1, 1};
  // Allocate Mac-specific "input" data structure
  mac_infont = (FMInput *)NewPtr(sizeof(FMInput));
  // Install data
  mac_infont->needBits = true;
  mac_infont->device = 0;
  //d->mac_window->device;
  mac_infont->numer = scale_pt;
  mac_infont->denom = scale_pt;
  mac_infont->family = amulet_to_mac_family(Am_FONT_FIXED);
  mac_infont->face = 0; // Not bold, italic or underlined
  mac_infont->size = amulet_to_mac_size(Am_FONT_MEDIUM);
}

FMInput *
Am_Font_Data::Get_Mac_InFont(Am_Drawonable_Impl *d)
{
  if (!mac_infont) {
    if (name)
      install_nonstandard_infont(d);
    else
      install_standard_infont(d);
  }
  return mac_infont;
}

FMOutPtr
Am_Font_Data::Get_Mac_OutFont(Am_Drawonable_Impl *d)
{
  //if (!mac_outfont)
  mac_outfont = *FMSwapFont(Get_Mac_InFont(d));
  return &mac_outfont;
}

// // // // // // // // // // // // // // // // // // // // // // // // // //
// Draw_Text
// // // // // // // // // // // // // // // // // // // // // // // // // //

void
Am_Drawonable_Impl::set_pen_using_font(const Am_Font &Am_font)
{
  Am_Font_Data *font = Am_Font_Data::Narrow(Am_font);

  if (font) {
    FMInput *mac_infont = font->Get_Mac_InFont(this);
    TextFont(mac_infont->family);
    TextFace(mac_infont->face);
    TextSize(mac_infont->size);
    font->Release();
  }
  // otherwise do nothing
}

// Defaults:
//   f = Am_DRAW_COPY,
//   bs = 0,
//   underline = false,
//   invert = false

void
Am_Drawonable_Impl::Draw_Text(const Am_Style &ls, const char *s, int str_len,
                              const Am_Font &Am_font, int left, int top,
                              Am_Draw_Function f,
                              const Am_Style &fs, // background fill style
                              bool invert)
{
#ifdef DEBUG
  if (!mac_port) {
    Am_Error("mac_port not set for Am_Drawonable_Impl::Draw_Text");
    return;
  }
#endif

  static int width, ascent, descent, left_bearing, right_bearing;
  focus_on_this();
  Get_String_Extents(Am_font, s, str_len, width, ascent, descent, left_bearing,
                     right_bearing);

  // temp hack
  Am_Style line_style(invert ? fs : ls);
  Am_Style fill_style(invert ? ls : fs);

  if (invert) {
    if (!fill_style.Valid())
      fill_style = Am_Style(0.0, 0.0, 0.0); // White text default
    if (!line_style.Valid())
      line_style = Am_Style(1.0, 1.0, 1.0); // Black back default
  }
  Draw_Rectangle(Am_No_Style, fill_style, left, top, width, ascent + descent,
                 f);

  PixMapHandle pix_map = lock_current_pixmap();

  // Temp for stippled text
  if (line_style.Get_Fill_Flag() == Am_FILL_STIPPLED) {
    TextMode(grayishTextOr);
  } else {
    TextMode(srcOr);
  }
  set_pen_using_line(line_style, f);

  set_pen_using_font(Am_font);
  MoveTo(left, top + ascent);

  DrawText(s, 0, str_len);
  unlock_current_pixmap(pix_map);
}

// // // // // // // // // // // // // // // // // // // // // // // // // //
// Am_Font operations, must have a Am_Drawonable and a Am_Font to get sizes
// // // // // // // // // // // // // // // // // // // // // // // // // //

// This function sets the pen's font state to calculate the string's width
// and does not restore them.  Therefore, this function should not be
// invoked between the setting of a pen's font state and drawing text.
// Note: Draw_Text always sets the pen state before it draws.
//
int
Am_Drawonable_Impl::Get_Char_Width(const Am_Font &Am_font, char c)
{
  Am_Font_Data *font = Am_Font_Data::Narrow(Am_font);

  if (!font)
    return 0;

  FMInput *mac_infont = font->Get_Mac_InFont(this);
  FMOutput *mac_outfont = font->Get_Mac_OutFont(this);
  font->Release();

  // Set the GrafPort's pen state to the specified font so we can use
  // the GrafPort to compute the string's width.
  GrafPtr save_port;
  GetPort(&save_port);
  SetPort(util_port);
  TextFont(mac_infont->family);
  TextFace(mac_infont->face);
  TextSize(mac_infont->size);
  int result = CharWidth(c);
  result += mac_outfont->boldPixels + mac_outfont->italicPixels;

  SetPort(save_port);
  return result;
}

// This function sets the pen's font state to calculate the string's width
// and does not restore them.  Therefore, this function should not be
// invoked between the setting of a pen's font state and drawing text.
// Note: Draw_Text always sets the pen state before it draws.
//
// BUG: This function should be optimized so that it does a simple
// multiplication on the char_width when it is passed a fixed-width font.
//
int
Am_Drawonable_Impl::Get_String_Width(const Am_Font &Am_font,
                                     const char *the_string,
                                     int the_string_length)
{
  Am_Font_Data *font = Am_Font_Data::Narrow(Am_font);

  if (!font)
    return 0;

  FMInput *mac_infont = font->Get_Mac_InFont(this);
  FMOutput *mac_outfont = font->Get_Mac_OutFont(this);
  font->Release();

  // Set the GrafPort's pen state to the specified font so we can use
  // the GrafPort to compute the string's width.
  GrafPtr save_port;
  GetPort(&save_port);
  SetPort(util_port);
  TextFont(mac_infont->family);
  TextFace(mac_infont->face);
  TextSize(mac_infont->size);

  int result = TextWidth(the_string, 0, the_string_length);
  result += mac_outfont->boldPixels + mac_outfont->italicPixels;

  SetPort(save_port);
  return result;
}

// This function does more work than Get_Font_Properties, so only use it
// when you need info about a specific string.
// This function sets the pen's font state via Get_String_Width and does
// not restore them.  Therefore, this function should not be invoked
// between the setting of a pen's font state and drawing text.
// Note: Draw_Text always sets the pen state before it draws.
//
void
Am_Drawonable_Impl::Get_String_Extents(const Am_Font &Am_font,
                                       const char *the_string,
                                       int the_string_length, int &width,
                                       int &ascent, int &descent,
                                       int &left_bearing, int &right_bearing)
{
  Am_Font_Data *font = Am_Font_Data::Narrow(Am_font);

  if (font) {
    FMOutPtr mac_outfont = font->Get_Mac_OutFont(this);
    font->Release();

    width = Get_String_Width(Am_font, the_string, the_string_length);
    ascent = mac_outfont->ascent;
    descent = mac_outfont->descent;
  } else {
    width = ascent = descent = 0;
  }

  // The Mac does not give us these numbers. They are derived from
  // the X implementation. Basically this describes how many white
  // pixels are on the left and right before any "inked" pixels.
  // We assumes zero, which would not be correct if there were spaces
  // before or after the text string.
  left_bearing = right_bearing = 0;
}

// BUG: Remove min_char_width and max_char_width. They are ambiguous
// values and cannot easily be determined for all systems.
void
Am_Drawonable_Impl::Get_Font_Properties(const Am_Font &Am_font,
                                        int &max_char_width,
                                        int &min_char_width, int &char_ascent,
                                        int &char_descent)
{
  Am_Font_Data *font = Am_Font_Data::Narrow(Am_font);

  if (font) {
    FMOutPtr mac_outfont = font->Get_Mac_OutFont(this);
    font->Release();

    max_char_width = mac_outfont->widMax;
    min_char_width = 0; // BUG: Maybe use OutlineMetrics() to get this?
    char_ascent = mac_outfont->ascent;
    char_descent = mac_outfont->descent;
  } else {
    max_char_width = min_char_width = char_ascent = char_descent = 0;
  }
}
