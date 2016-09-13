#include <windows.h>
#include <string.h> //memset, strncpy

#include <amulet/types.h>
#include <amulet/gdefs.h>

#include "amulet/gemW_misc.h" //strnew, strdel
#include "amulet/gemW.h"
#include "amulet/gemW_clean.h"
#include "amulet/gemW_text.h"
#include <amulet/registry.h>

int
WinPtSize(Am_Font_Size_Flag amsf)
{
  switch (amsf) {
  case Am_FONT_SMALL:
    return 6;
  case Am_FONT_MEDIUM:
    return 8;
  case Am_FONT_LARGE:
    return 12;
  case Am_FONT_VERY_LARGE:
    return 18;
  case Am_FONT_TSMALL:
    return 9; // Size for Two-byte codes
  case Am_FONT_TMEDIUM:
    return 11;
  case Am_FONT_TLARGE:
    return 14;
  case Am_FONT_TVERY_LARGE:
    return 20;
  default:
    return 10;
  }
}

BYTE
WinFntFamily(Am_Font_Family_Flag amff)
{
  switch (amff) {
  case Am_FONT_FIXED:
    return FIXED_PITCH | FF_DONTCARE;
  case Am_FONT_SERIF:
    return VARIABLE_PITCH | FF_ROMAN;
  case Am_FONT_SANS_SERIF:
    return VARIABLE_PITCH | FF_SWISS;
  case Am_FONT_JFIXED: // for Japanese font family
  case Am_FONT_CFIXED:
  case Am_FONT_KFIXED:
    return FIXED_PITCH | FF_ROMAN;
  case Am_FONT_JPROPORTIONAL:
    return VARIABLE_PITCH | FF_ROMAN;
  default:
    return DEFAULT_PITCH | FF_DONTCARE;
  }
}

///////////////
// Am_Font_Data::Am_Wrapper

AM_WRAPPER_DATA_IMPL(Am_Font, (this))

Am_Font_Data::Am_Font_Data(Am_Font_Data *proto)
{
  m_family = proto->m_family;
  m_bold = proto->m_bold;
  m_italic = proto->m_italic;
  m_underline = proto->m_underline;
  m_size = proto->m_size;
  m_name = strnew(proto->m_name);

  m_hfont = 0;
}

static bool
isJapanese(const char *p)
{ // Japanese font name?
  while (*p != (0L)) {
    if ((*p & 0x80) != 0)
      return (true);
    p++;
  }
  return (false);
}

Am_Font_Data::Am_Font_Data(const char *the_name)
{
  Am_WINCLEAN_CONSTR(Am_Font_Data)

      m_name = strnew(the_name);
  if (isJapanese(the_name)) { // determine that it is Japanese font name
    if (strstr(the_name, "‚o") == (0L))
      m_family = Am_FONT_JFIXED;
    else
      m_family = Am_FONT_JPROPORTIONAL;
    m_size = Am_FONT_TMEDIUM;
  } else {
    m_family = Am_FONT_FIXED;
    m_size = Am_FONT_MEDIUM;
  }
  m_bold = FALSE;
  m_italic = FALSE;
  m_underline = FALSE;

  m_hfont = 0;
}

Am_Font_Data::Am_Font_Data(Am_Font_Family_Flag f, bool is_bold, bool is_italic,
                           bool is_underline, Am_Font_Size_Flag s)
{
  Am_WINCLEAN_CONSTR(Am_Font_Data)

      m_family = f;
  m_bold = is_bold;
  m_italic = is_italic;
  m_underline = is_underline;
  m_size = s;
  // Indicate this is a standard font by zeroing-out the name field
  m_name = (0L);

  m_hfont = 0;
}

Am_Font_Data::~Am_Font_Data()
{
  Am_WINCLEAN_DESTR

      strdel(m_name);
#ifdef USE_WINCLEANER
  WinFreeRes();
#else
  if (m_hfont) {
    DeleteObject(m_hfont);
    m_hfont = 0;
  }
#endif
}

HFONT
Am_Font_Data::WinFont(HDC hdc)
{
#ifdef USE_WINCLEANER
  WinSetUsedRes();
#endif
  if (!m_hfont) {
    LOGFONT lf;
    memset(&lf, 0, sizeof(lf));
    lf.lfHeight =
        -MulDiv(WinPtSize(m_size), GetDeviceCaps(hdc, LOGPIXELSY), 72);
    if (m_name)
      strncpy(lf.lfFaceName, m_name, LF_FACESIZE);
    else if (m_family == Am_FONT_JFIXED || m_family == Am_FONT_CFIXED ||
             m_family == Am_FONT_KFIXED) {
      if (WinPtSize(m_size) <= 11)
        strncpy(lf.lfFaceName, "‚l‚r ƒSƒVƒbƒN", LF_FACESIZE);
      else
        strncpy(lf.lfFaceName, "‚l‚r –¾’©", LF_FACESIZE);
    } else if (m_family == Am_FONT_JPROPORTIONAL) {
      if (WinPtSize(m_size) <= 11)
        strncpy(lf.lfFaceName, "‚l‚r ‚oƒSƒVƒbƒN", LF_FACESIZE);
      else
        strncpy(lf.lfFaceName, "‚l‚r ‚o–¾’©", LF_FACESIZE);
    }
    lf.lfPitchAndFamily = WinFntFamily(m_family);
    if (m_bold)
      lf.lfWeight = FW_BOLD;
    if (m_italic)
      lf.lfItalic = TRUE;
    if (m_underline)
      lf.lfUnderline = TRUE;
    if ((int)m_family >= (int)Am_FONT_JFIXED) {
      lf.lfCharSet = SHIFTJIS_CHARSET;
      lf.lfOutPrecision = OUT_TT_PRECIS;
    }

    m_hfont = CreateFontIndirect(&lf);
  }
  return m_hfont;
}

#ifdef USE_WINCLEANER
AM_IMPL_WINCLEAN_FORWRAPPER(Am_Font_Data)

BOOL
Am_Font_Data::WinHasRes() const
{
  return (BOOL)m_hfont;
}

BOOL
Am_Font_Data::WinFreeRes()
{
  if (m_hfont) {
    DeleteObject(m_hfont);
    m_hfont = 0;
    return TRUE;
  }
  return FALSE;
}
#endif

//////////
// Am_Font

AM_WRAPPER_IMPL(Am_Font)

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

//bool Am_Font::Font_Name_Valid (const char* name)
//{
//  return true;
//}

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
    font_name = data->m_name;
    f = data->m_family;
    is_bold = data->m_bold;
    is_italic = data->m_italic;
    is_underline = data->m_underline;
    s = data->m_size;
  } else
    Am_Error("Get called on (0L) font");
}

void
Am_Font_Data::Print(std::ostream &os) const
{
  const char *lookup_name = (0L);

  lookup_name = Am_Get_Name_Of_Item(this);

  if (lookup_name)
    os << lookup_name;
  else if (m_name)
    os << "Font From File(" << m_name << ")";
  else {
    os << "Font(" << std::hex << (unsigned long)this << std::dec << ")=[";
    switch (m_family) {
    case Am_FONT_FIXED:
      os << "FIXED";
      break;
    case Am_FONT_SERIF:
      os << "SERIF";
      break;
    case Am_FONT_SANS_SERIF:
      os << "SANS_SERIF";
      break;
    case Am_FONT_JFIXED:
      os << "JFIXED";
      break;
    case Am_FONT_JPROPORTIONAL:
      os << "JPROPORTIONAL";
      break;
    case Am_FONT_CFIXED:
      os << "CFIXED";
      break;
    case Am_FONT_KFIXED:
      os << "KFIXED";
      break;
    }
    if (m_bold)
      os << ", BOLD";
    if (m_italic)
      os << ", ITALIC";
    if (m_underline)
      os << ", UNDERLINE";
    switch (m_size) {
    case Am_FONT_SMALL:
      os << ", SMALL";
      break;
    case Am_FONT_MEDIUM:
      os << ", MEDIUM";
      break;
    case Am_FONT_LARGE:
      os << ", LARGE";
      break;
    case Am_FONT_VERY_LARGE:
      os << ", VERY_LARGE";
      break;
    case Am_FONT_TSMALL:
      os << ", TSMALL";
      break;
    case Am_FONT_TMEDIUM:
      os << ", TMEDIUM";
      break;
    case Am_FONT_TLARGE:
      os << ", TLARGE";
      break;
    case Am_FONT_TVERY_LARGE:
      os << ", TVERY_LARGE";
      break;
    }
    os << "]";
  }
}

/*
Am_Font::Am_Font (Am_Wrapper* font_data)
{
	if (font_data) {
		if (font_data -> ID() == Am_Font_Data::Am_Font_Data_ID())
			data = (Am_Font_Data*)font_data;
		else
			Am_Error ("** Tried to set a Am_Font with a non font wrapper.");
	}
	else
		data = (0L);
}

Am_Font::Am_Font (Am_Font& font)
{
	if (font.data)
		font.data->Note_Reference();
	data = font.data;
}

Am_Font::Am_Font (const char *the_name)
{
	data = new Am_Font_Data(the_name);
}
  
// Defaults:
//   f = Am_FONT_FIXED,
//   is_bold = false,
//   is_italic = false,
//   is_underline = false,
//   s = Am_FONT_MEDIUM
Am_Font::Am_Font (Am_Font_Family_Flag f,
		bool is_bold,
		bool is_italic,
		bool is_underline,
		Am_Font_Size_Flag s)
{
	data = new Am_Font_Data(f, is_bold, is_italic, is_underline, s);
}

Am_Font::~Am_Font ()
{
	if (data) {
		if (data->Is_Zero ())
			Am_Error ();
		data->Release ();
	}
	data = (0L);
}

Am_Font::operator Am_Wrapper* ()
{
	if (data)
		data->Note_Reference();
	return data;
}

Am_Font& Am_Font::operator= (Am_Wrapper* font_data)
{
	if (data)
		data->Release();
	if (font_data) {
		if (font_data->ID() == Am_Font_Data::Am_Font_Data_ID())
			data = (Am_Font_Data*)font_data;
		else
			Am_Error ("** Tried to set a Am_Font with a non font wrapper.");
	}
	else
		data = (0L);
	return *this;
}

Am_Font& Am_Font::operator= (Am_Font font)
{
	if (data)
		data->Release();
	if (font.data)
		font.data->Note_Reference();
	data = font.data;
	return *this;
}
				  */
