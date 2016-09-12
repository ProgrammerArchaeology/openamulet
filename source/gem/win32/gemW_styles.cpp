//
// ------------------------------------------------------------------------
// 		The OpenAmulet User Interface Development Environment
// ------------------------------------------------------------------------
// This code is based on the Amulet project of Carnegie Mellon University,
// and has been placed in the public domain.  If you are using this code
// or any part of OpenAmulet, please contact amulet@cs.cmu.edu to be put
// on the mailing list or have a look at:
// http://www.openip.org
//

#include <cstdlib> //abs
#include <windows.h>
#include <windowsx.h>
#include <fstream>
#include <math.h>

#include <am_inc.h>
#include <amulet/impl/types_logging.h>
#include <amulet/impl/types_wrapper.h>
#include <amulet/impl/gem_flags.h>
#include <amulet/impl/am_style.h>
#include <amulet/registry.h>

#include <amulet/symbol_table.h>

#include "amulet/gemW.h"
#include "amulet/gemW_misc.h" //strnew, strdel
#include "amulet/gemW_clean.h"
#include "amulet/gemW_image.h"
#include "amulet/gemW_styles.h"

const char Am_DEFAULT_DASH_LIST[Am_DEFAULT_DASH_LIST_LENGTH] =
    Am_DEFAULT_DASH_LIST_VALUE;

DWORD
WinRopFunc(Am_Draw_Function amdrwfn)
{
  switch (amdrwfn) {
  case Am_DRAW_MASK_COPY:
  case Am_DRAW_COPY:
    return SRCCOPY;
  case Am_DRAW_GRAPHIC_OR:
  case Am_DRAW_OR:
    return SRCPAINT;
  case Am_DRAW_GRAPHIC_XOR:
  case Am_DRAW_XOR:
    return SRCINVERT;
  case Am_DRAW_GRAPHIC_NIMP:
    return GW_GRAPHIC_NIMP;
  case Am_DRAW_GRAPHIC_AND:
    return SRCAND;
  default:
    return BLACKNESS;
  }
}

int
WinRop2Func(Am_Draw_Function amdrwfn)
{
  switch (amdrwfn) {
  case Am_DRAW_MASK_COPY:
  case Am_DRAW_COPY:
    return R2_COPYPEN;
  case Am_DRAW_GRAPHIC_OR:
  case Am_DRAW_OR:
    return R2_MERGEPEN;
  case Am_DRAW_GRAPHIC_XOR:
  case Am_DRAW_XOR:
    return R2_XORPEN;
  case Am_DRAW_GRAPHIC_NIMP:
    return R2_MASKPENNOT;
  case Am_DRAW_GRAPHIC_AND:
    return R2_MASKPEN;
  default:
    return R2_BLACK;
  }
}

int
WinPolyFillMode(Am_Fill_Poly_Flag fpflag)
{
  switch (fpflag) {
  case Am_FILL_POLY_EVEN_ODD:
    return ALTERNATE;
  case Am_FILL_POLY_WINDING:
    return WINDING;
  default:
    return ALTERNATE;
  }
}

#pragma init_seg(lib)
Am_Symbol_Table color_map(17);

#define _rgb(R, G, B) ((R) + ((G) << 3) + ((B) << 6))
#define _rgbExp(V)                                                             \
  (((V) == 0) ? 0 : ((V) == 1) ? 64 : ((V) == 2) ? 128 : ((V) == 3) ? 192 : 255)
#define _rgbCmp(V)                                                             \
  (((V) >= 255) ? 4 : ((V) >= 192) ? 3 : ((V) >= 128) ? 2 : ((V) >= 64) ? 1 : 0)

#define _rgb2RGB(V)                                                            \
  (RGB(_rgbExp((V)&0x7), _rgbExp(((V) >> 3) & 0x7), _rgbExp(((V) >> 6) & 0x7)))
#define _RGB2rgb(V)                                                            \
  (_rgb(_rgbCmp(GetRValue(V)), _rgbCmp(GetGValue(V)), _rgbCmp(GetBValue(V))))

short ACV /*AdjustColorValue*/ (short val)
{
#if 0
  static const short compon[] = { 0, 64, 128, 192, 255 };
  static const int ncompon = sizeof(compon) / sizeof(short);

  for (int i = 0; i < ncompon - 1; i++)
    if (abs(val - compon[i]) < abs(val - compon[i + 1]))
      return compon[i];

  return compon[ncompon - 1];
#endif
  return val <= 255 ? val : 255;
}

short GCV /*GrayColorValue*/ (short val, int f)
{
  return min(256 - MulDiv(256 - val, f, 16), 255);
}

void
Am_Initialize_Color_Map()
{
  static BOOL _initialized = FALSE;

  if (_initialized)
    return;

  color_map.Add_Item(_rgb(0, 0, 0), "BLACK");
  color_map.Add_Item(_rgb(0, 0, 4), "BLUE");
  color_map.Add_Item(_rgb(0, 4, 0), "GREEN");
  color_map.Add_Item(_rgb(0, 4, 4), "CYAN");
  color_map.Add_Item(_rgb(4, 0, 0), "RED");
  color_map.Add_Item(_rgb(4, 0, 4), "MAGENTA");
  color_map.Add_Item(_rgb(4, 4, 0), "YELLOW");
  color_map.Add_Item(_rgb(4, 4, 4), "WHITE");
  color_map.Add_Item(_rgb(4, 4, 3), "BEIGE");

  color_map.Add_Item(_rgb(2, 2, 2), "GRAY");
  color_map.Add_Item(_rgb(3, 3, 3), "LTGRAY");
  color_map.Add_Item(_rgb(1, 1, 1), "DKGRAY");

  color_map.Add_Item(_rgb(4, 2, 0), "ORANGE");
  color_map.Add_Item(_rgb(0, 4, 2), "ORCHID");
  color_map.Add_Item(_rgb(4, 0, 4), "PURPLE");
  color_map.Add_Item(_rgb(4, 0, 2), "PLUM");

  _initialized = TRUE;
}

///////////
// Am_Style

AM_WRAPPER_IMPL(Am_Style)

Am_Style Am_No_Style;

static Am_Style default_style(0.0, 0.0, 0.0); // used by Get_Values
static Am_Style_Data *default_style_data =
    (Am_Style_Data *)(Am_Wrapper *)default_style;

static Am_Style_Data *on_bits = new Am_Style_Data("Am_On_Bits", true);
Am_Style Am_On_Bits(on_bits);

static Am_Style_Data *off_bits = new Am_Style_Data("Am_Off_Bits", false);
Am_Style Am_Off_Bits(off_bits);

const char *
Am_Style::Get_Color_Name() const
{
  if (data)
    return data->Get_Color_Name();
  else
    return (0);
}

void
Am_Style::Get_Values(float &r, float &g, float &b) const
{
  if (data) {
    r = data->m_red;
    g = data->m_green;
    b = data->m_blue;
  } else
    r = g = b = (float)0.0;
}

void
Am_Style::Get_Values(short &thickness, Am_Line_Cap_Style_Flag &cap,
                     Am_Join_Style_Flag &join, Am_Line_Solid_Flag &line_flag,
                     const char *&dash_l, int &dash_l_length,
                     Am_Fill_Solid_Flag &fill_flag, Am_Fill_Poly_Flag &poly,
                     Am_Image_Array &stipple) const
{
  if (data) {
    thickness = data->m_line_thickness;
    cap = data->m_cap_style;
    join = data->m_join_style;
    line_flag = data->m_line_solid;
    dash_l = data->m_dash_list;
    dash_l_length = data->m_dash_list_length;
    fill_flag = data->m_fill_solid;
    poly = data->m_fill_poly;
    stipple = data->m_stipple_bitmap;
  } else {
    // return default values
    default_style.Get_Values(thickness, cap, join, line_flag, dash_l,
                             dash_l_length, fill_flag, poly, stipple);
  }
}

void
Am_Style::Get_Values(float &r, float &g, float &b, short &thickness,
                     Am_Line_Cap_Style_Flag &cap, Am_Join_Style_Flag &join,
                     Am_Line_Solid_Flag &line_flag, const char *&dash_l,
                     int &dash_l_length, Am_Fill_Solid_Flag &fill_flag,
                     Am_Fill_Poly_Flag &poly, Am_Image_Array &stipple) const
{
  Get_Values(r, g, b);
  Get_Values(thickness, cap, join, line_flag, dash_l, dash_l_length, fill_flag,
             poly, stipple);
}

Am_Fill_Solid_Flag
Am_Style::Get_Fill_Flag() const
{
  if (data)
    return data->m_fill_solid;
  else
    return Am_FILL_SOLID;
}

Am_Line_Solid_Flag
Am_Style::Get_Line_Flag() const
{
  if (data)
    return data->m_line_solid;
  else
    return Am_LINE_SOLID;
}

Am_Image_Array
Am_Style::Get_Stipple() const
{
  if (data) {
    return (data->m_stipple_bitmap);
  } else {
    return (static_cast<int>(0));
  }
}

void
Am_Style::Get_Line_Thickness_Values(short &thickness,
                                    Am_Line_Cap_Style_Flag &cap) const
{
  if (data) {
    thickness = data->m_line_thickness;
    cap = data->m_cap_style;
  } else {
    default_style.Get_Line_Thickness_Values(thickness, cap);
  }

  return;
}

Am_Style::Am_Style(
    float r, float g, float b, // color part
    short thickness, Am_Line_Cap_Style_Flag cap, Am_Join_Style_Flag join,
    Am_Line_Solid_Flag line_flag, const char *dash_l, int dash_l_length,
    Am_Fill_Solid_Flag fill_flag, Am_Fill_Poly_Flag poly,
    Am_Image_Array stipple // stipple must be an opal bitmap object
    )
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
  Am_Initialize_Color_Map();

  data = new Am_Style_Data(color_name, thickness, cap, join, line_flag, dash_l,
                           dash_l_length, fill_flag, poly, stipple);
}

Am_Style::Am_Style() { data = (0L); }

Am_Style
Am_Style::Halftone_Stipple(int percent, Am_Fill_Solid_Flag fill_flag)
{
  Am_Style style(new Am_Style_Data(
      (float)0.0, (float)0.0, (float)0.0, 0, Am_CAP_BUTT, Am_JOIN_MITER,
      Am_LINE_SOLID, Am_DEFAULT_DASH_LIST, Am_DEFAULT_DASH_LIST_LENGTH,
      fill_flag, Am_FILL_POLY_EVEN_ODD, Am_Image_Array(percent)));
  return style;
}

#ifdef ___
Am_Style
Am_Style::Diamond_Stipple()
{
  Am_Style style(new Am_Style_Data(
      (float)0.0, (float)0.0, (float)0.0, 0, Am_CAP_BUTT, Am_JOIN_MITER,
      Am_LINE_SOLID, Am_DEFAULT_DASH_LIST, Am_DEFAULT_DASH_LIST_LENGTH,
      Am_FILL_STIPPLED, Am_FILL_POLY_EVEN_ODD, Am_Image_Array(-1)));
  return style;
}
#endif

Am_Style
Am_Style::Thick_Line(unsigned short thickness)
{
  Am_Style style((float)0.0, (float)0.0, (float)0.0, thickness);
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
  if (data) {
    data = (Am_Style_Data *)data->Make_Unique();
    data->m_stipple_bitmap = image;
  } else
    data = new Am_Style_Data((float)0.0, (float)0.0, (float)0.0, 0, Am_CAP_BUTT,
                             Am_JOIN_MITER, Am_LINE_SOLID, Am_DEFAULT_DASH_LIST,
                             Am_DEFAULT_DASH_LIST_LENGTH, Am_FILL_STIPPLED,
                             Am_FILL_POLY_EVEN_ODD, image);
}

////////////////
// Am_Style_Data::Am_Wrapper

AM_WRAPPER_DATA_IMPL(Am_Style, (this))

#if _MSC_VER >= 1200
HBRUSH Am_Style_Data::hbrNullBrush =
    reinterpret_cast<HBRUSH__ *>(GetStockObject(NULL_BRUSH));
HPEN Am_Style_Data::hpenNullPen =
    reinterpret_cast<HPEN__ *>(GetStockObject(NULL_PEN));
#else
HBRUSH Am_Style_Data::hbrNullBrush = GetStockObject(NULL_BRUSH);
HPEN Am_Style_Data::hpenNullPen = GetStockObject(NULL_PEN);
#endif

COLORREF Am_Style_Data::crefBlack = RGB(0, 0, 0);
COLORREF Am_Style_Data::crefWhite = RGB(255, 255, 255);

void inline Am_Style_Data::copy(Am_Style_Data *proto)
{
  m_red = proto->m_red;
  m_green = proto->m_green;
  m_blue = proto->m_blue;
  m_color_name = strnew(proto->m_color_name);
  m_line_thickness = proto->m_line_thickness;
  m_cap_style = proto->m_cap_style;
  m_join_style = proto->m_join_style;
  m_line_solid = proto->m_line_solid;
  m_dash_list_length = proto->m_dash_list_length;
  m_dash_list = new char[m_dash_list_length];
  strncpy(m_dash_list, proto->m_dash_list, m_dash_list_length);
  m_fill_solid = proto->m_fill_solid;
  m_fill_poly = proto->m_fill_poly;
  m_stipple_bitmap = proto->m_stipple_bitmap;

  m_hbrush = proto->m_hbrush;
  m_hpen = proto->m_hpen;
  m_cref = proto->m_cref;
}

Am_Style_Data::Am_Style_Data(Am_Style_Data *proto)
{
  Am_WINCLEAN_CONSTR(Am_Style_Data)

      copy(proto);
}

Am_Style_Data::Am_Style_Data(float r, float g, float b, short thickness,
                             Am_Line_Cap_Style_Flag cap,
                             Am_Join_Style_Flag join,
                             Am_Line_Solid_Flag line_flag, const char *dash_l,
                             int dash_l_length, Am_Fill_Solid_Flag fill_flag,
                             Am_Fill_Poly_Flag poly, Am_Image_Array stipple)
{
  Am_WINCLEAN_CONSTR(Am_Style_Data)

      m_red = r;
  m_green = g;
  m_blue = b;
  m_color_name = (0L);
  m_line_thickness = thickness;
  m_cap_style = cap;
  m_join_style = join;
  m_line_solid = line_flag;
  m_dash_list_length = dash_l_length;
  m_dash_list = new char[m_dash_list_length];
  strncpy(m_dash_list, dash_l, m_dash_list_length);
  m_fill_solid = fill_flag;
  m_fill_poly = poly;
  m_stipple_bitmap = stipple;

  m_hbrush = 0;
  m_hpen = 0;
  m_cref =
      RGB(ACV((short)ceil(m_red * 255.0)), ACV((short)ceil(m_green * 255.0)),
          ACV((short)ceil(m_blue * 255.0)));
  // AdjustColor();
}

Am_Style_Data::Am_Style_Data(const char *color_name, short thickness,
                             Am_Line_Cap_Style_Flag cap,
                             Am_Join_Style_Flag join,
                             Am_Line_Solid_Flag line_flag, const char *dash_l,
                             int dash_l_length, Am_Fill_Solid_Flag fill_flag,
                             Am_Fill_Poly_Flag poly, Am_Image_Array stipple)
{
  Am_WINCLEAN_CONSTR(Am_Style_Data)

      m_color_name = strnew(color_name);
  _strupr(m_color_name);
  int rgb = color_map.Get_Value(m_color_name);
  m_cref = (rgb >= 0) ? _rgb2RGB(rgb) : RGB(0, 0, 0);
  m_red = (float)(GetRValue(m_cref) / 255.0);
  m_green = (float)(GetGValue(m_cref) / 255.0);
  m_blue = (float)(GetBValue(m_cref) / 255.0);

  m_line_thickness = thickness;
  m_cap_style = cap;
  m_join_style = join;
  m_line_solid = line_flag;
  m_dash_list_length = dash_l_length;
  m_dash_list = new char[m_dash_list_length];
  strncpy(m_dash_list, dash_l, m_dash_list_length);
  m_fill_solid = fill_flag;
  m_fill_poly = poly;
  m_stipple_bitmap = stipple;

  m_hbrush = 0;
  m_hpen = 0;
}

Am_Style_Data::Am_Style_Data(COLORREF wincolor)
{
  Am_WINCLEAN_CONSTR(Am_Style_Data) copy(default_style_data);

  m_red = (float)(GetRValue(wincolor) / 255.0);
  m_green = (float)(GetGValue(wincolor) / 255.0);
  m_blue = (float)(GetBValue(wincolor) / 255.0);
  m_color_name = (0L);

  m_hbrush = 0;
  m_hpen = 0;
  m_cref = wincolor;
}

Am_Style_Data::Am_Style_Data(const char *name, bool bit_is_on)
{
  Am_WINCLEAN_CONSTR(Am_Style_Data) copy(default_style_data);

  m_color_name = strnew(name);

  if (bit_is_on) {
    m_red = m_green = m_blue = 1.0f;
    m_cref = crefWhite;
  } else {
    m_red = m_green = m_blue = 0.0f;
    m_cref = crefBlack;
  }
}

Am_Style_Data::Am_Style_Data(Am_Style_Data *proto, Am_Style_Data *new_color)
{
  Am_WINCLEAN_CONSTR(Am_Style_Data) copy(proto);

  // FIX: change color to new_color's
  m_red = new_color->m_red;
  m_green = new_color->m_green;
  m_blue = new_color->m_blue;
  m_color_name = strnew(new_color->m_color_name);
  m_cref = new_color->m_cref;
  m_hbrush = 0;
  m_hpen = 0;
}

Am_Style_Data::~Am_Style_Data()
{
  Am_WINCLEAN_DESTR

      delete[] m_dash_list;
  strdel(m_color_name);
#ifdef USE_WINCLEANER
  WinFreeRes();
#else
  if (m_hbrush) {
    DeleteObject(m_hbrush);
    m_hbrush = 0;
  }
  if (m_hpen) {
    DeleteObject(m_hpen);
    m_hpen = 0;
  }
#endif
  m_stipple_bitmap = (Am_Image_Array_Data *)(0L);
}

const char *
Am_Style_Data::Get_Color_Name()
{
  return m_color_name ? m_color_name : color_map[_RGB2rgb(m_cref)];
}

void
Am_Style_Data::AdjustColor()
{
  HDC hdc = GetDC(0);
  m_cref = GetNearestColor(hdc, m_cref);
  ReleaseDC(0, hdc);
}

HBRUSH
Am_Style_Data::WinBrush(HDC hdc)
{
#ifdef USE_WINCLEANER
  WinSetUsedRes();
#endif
  // PREPARE_DATA(Am_Image_Array, stpld, m_stipple_bitmap)
  Am_Image_Array_Data *stpld = Am_Image_Array_Data::Narrow(m_stipple_bitmap);

  if (!m_hbrush) {
    if (m_fill_solid == Am_FILL_SOLID) {
      m_hbrush = CreateSolidBrush(m_cref);
      if (!m_hbrush)
        std::cout << "Error: Ran out of colors" << std::endl;
    } else if (stpld) {
      int iw, ih;
      stpld->Get_Size(iw, ih);
      if (iw != 8 || ih != 8) {
        stpld->Release(); // must free up pointer.
        stpld = new Am_Image_Array_Data(stpld, 8, 8);
        m_stipple_bitmap = Am_Image_Array(
            stpld); // operator= will do Release for the old 'stpld'
        stpld->Note_Reference(); // because we'll do DISCARD_DATA(stpld) below
      }
      m_hbrush = CreatePatternBrush(stpld->WinBitmap(hdc));
      if (!m_hbrush)
        std::cout << "Error: Pattern did not work." << std::endl;
    }
  }

  if (hdc && stpld)
    // NOTE BkMode IS NOT USED when drawing with a PatternBrush
    // PatternBrushes are ALWAYS opaque
    // TextColor is used for 0 Pixels; BkColor for 1 Pixels
    switch (m_fill_solid) {
    case Am_FILL_SOLID:
      break;
    case Am_FILL_STIPPLED:
      SetTextColor(hdc, m_cref);
      //      SetBkMode(hdc, TRANSPARENT);
      break;
    case Am_FILL_OPAQUE_STIPPLED:
      SetTextColor(hdc, m_cref);
      //      SetBkMode(hdc, OPAQUE);
      break;
    }

  if (stpld)
    stpld->Release(); // DISCARD_DATA(stpld)

  return m_hbrush;
}

HPEN
Am_Style_Data::WinPen(int emulate)
{
#ifdef USE_WINCLEANER
  WinSetUsedRes();
#endif
  if (!m_hpen) {
    COLORREF pen_color;
    if (m_stipple_bitmap.Valid()) {
      Am_Image_Array_Data *stpld =
          Am_Image_Array_Data::Narrow(m_stipple_bitmap);
      int f = stpld->WinGrayFactor();
      stpld->Release();

      pen_color = RGB(GCV(GetRValue(m_cref), f), GCV(GetGValue(m_cref), f),
                      GCV(GetBValue(m_cref), f));
    } else
      pen_color = m_cref;

    int pen_flags = PS_SOLID | PS_INSIDEFRAME;

    if (emulate && m_line_solid != Am_LINE_SOLID) {
      if (m_dash_list_length == 2 && m_dash_list[0] <= 3 && m_dash_list[1] <= 3)
        pen_flags = PS_DOT;
      else
        pen_flags = PS_DASH;
    }

    m_hpen = CreatePen(pen_flags, m_line_thickness, pen_color);
    if (!m_hpen)
      std::cout << "Error: Ran out of pen handles" << std::endl;
  }
  return m_hpen;
}

#ifdef USE_WINCLEANER
AM_IMPL_WINCLEAN_FORWRAPPER(Am_Style_Data)

BOOL
Am_Style_Data::WinHasRes() const
{
  return (BOOL)(m_hbrush || m_hpen);
}

BOOL
Am_Style_Data::WinFreeRes()
{
  BOOL fFreed = FALSE;

  if (m_hbrush) {
    DeleteObject(m_hbrush);
    m_hbrush = 0;
    fFreed = TRUE;
  }
  if (m_hpen) {
    DeleteObject(m_hpen);
    m_hpen = 0;
    fFreed = TRUE;
  }
  return fFreed;
}
#endif

void
Am_Style_Data::Print(std::ostream &os) const
{
  const char *lookup_name = (0L);

  lookup_name = Am_Get_Name_Of_Item(this);

  if (lookup_name)
    os << lookup_name;
  else {
    os << "Am_Style(" << std::hex << (unsigned long)this << std::dec << ")=[";
    if (m_color_name)
      os << "color=" << m_color_name;
    else
      os << "color=(" << m_red << "," << m_green << "," << m_blue << ")";
    os << " thickness=" << m_line_thickness;
    //don't bother with the rest of the fields
    os << " ...]";
  }
}
