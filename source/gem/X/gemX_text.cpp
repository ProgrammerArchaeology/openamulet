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

#include <X11/Xlib.h>

#include <cstdio>
#include <cstdlib>

#include <am_inc.h>

#include <amulet/gdefs.h>
#include <amulet/gem.h>
#include <amulet/gemX.h>
#include <amulet/registry.h>

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
    font_name = data->name;
    f = data->family;
    is_bold = data->bold;
    is_italic = data->italic;
    is_underline = data->underline;
    s = data->size;
  } else
    Am_Error("Get called on (0L) font");
}

// // // // // // // // // // // // // // // // // // // // // // // // // //
// Utility functions for setting the GContext's font
// // // // // // // // // // // // // // // // // // // // // // // // // //

// Note: the following macros are defined in gemX.h:
//   Am_FIXED_FONT_FAMILY      Am_SERIF_FONT_FAMILY  Am_SANS_SERIF_FONT_FAMILY
//   Am_SMALL_PIXEL_SIZE       Am_MEDIUM_PIXEL_SIZE  Am_LARGE_PIXEL_SIZE
//   Am_VERY_LARGE_PIXEL_SIZE

char **
make_font_name(Display *dpy, Am_Font_Family_Flag family, bool is_bold,
               bool is_italic, bool /*is_underline*/, Am_Font_Size_Flag size)
{
  char wild_font_name[50]; // Given hard-coded defaults, max length is 50
  const char *family_part = 0, *face_part = 0;
  int size_part = 0;

  switch (family) {
  case Am_FONT_FIXED:
    family_part = Am_FIXED_FONT_FAMILY;
    break;
  case Am_FONT_SERIF:
    family_part = Am_SERIF_FONT_FAMILY;
    break;
  case Am_FONT_SANS_SERIF:
    family_part = Am_SANS_SERIF_FONT_FAMILY;
    break;
  case Am_FONT_JFIXED:
  case Am_FONT_JPROPORTIONAL:
  case Am_FONT_KFIXED:
  case Am_FONT_CFIXED:
    family_part = Am_TWO_BYTE_FONT_FAMILY;
    break;
  }

  if (family == Am_FONT_JFIXED || family == Am_FONT_JPROPORTIONAL ||
      family == Am_FONT_KFIXED || family == Am_FONT_CFIXED)
    face_part = "medium-r";
  else if ((is_bold == false) && (is_italic == false))
    face_part = "medium-r";
  else if ((is_bold == true) && (is_italic == false))
    face_part = "bold-r";
  else if ((is_bold == false) && (is_italic == true)) {
    if (family == Am_FONT_SERIF)
      face_part = "medium-i";
    else
      face_part = "medium-o";
  } else if ((is_bold == true) && (is_italic == true)) {
    if (family == Am_FONT_SERIF)
      face_part = "bold-i";
    else
      face_part = "bold-o";
  }

  switch (size) {
  case Am_FONT_SMALL:
    size_part = Am_SMALL_PIXEL_SIZE;
    break;
  case Am_FONT_MEDIUM:
    size_part = Am_MEDIUM_PIXEL_SIZE;
    break;
  case Am_FONT_LARGE:
    size_part = Am_LARGE_PIXEL_SIZE;
    break;
  case Am_FONT_VERY_LARGE:
    size_part = Am_VERY_LARGE_PIXEL_SIZE;
    break;
  case Am_FONT_TSMALL:
    size_part = Am_TSMALL_PIXEL_SIZE;
    break;
  case Am_FONT_TMEDIUM:
    size_part = Am_TMEDIUM_PIXEL_SIZE;
    break;
  case Am_FONT_TLARGE:
    size_part = Am_TLARGE_PIXEL_SIZE;
    break;
  case Am_FONT_TVERY_LARGE:
    size_part = Am_TVERY_LARGE_PIXEL_SIZE;
    break;
  }

  // Guide to fonts.dir format (try xfontsel on an HP):
  // -fndry-fmly-weight-slant-sWidth-adstyl-pixelsize-pointsize-...
  // ...resx-resy-spc-avgWdth-registry-encoding
  // also try xlsfonts -fn <pattern> to see all the fonts that match a
  // given pattern
  //
  // Note that a font name with a leading "*" is not well formed.
  // However, using the leading "*" avoids getting scalable fonts --
  // which can be unreadable in small sizes.  So try for a nonscalable
  // font first with the leading "*", and if that fails then use a
  // well-formed name to try for a scalable font.
  //
  switch (family) {
  case Am_FONT_JFIXED:
  case Am_FONT_JPROPORTIONAL:
    sprintf(wild_font_name, "*-*-%s-%s-*-*-%i-*-*-*-*-*-jisx0208.1983-0",
            family_part, face_part, size_part);
    break;
  case Am_FONT_KFIXED:
    sprintf(wild_font_name, "*-*-%s-%s-*-*-%i-*-*-*-*-*-ksc5601.1987-0",
            family_part, face_part, size_part);
    break;
  case Am_FONT_CFIXED:
    sprintf(wild_font_name, "*-*-%s-%s-*-*-%i-*-*-*-*-*-gb2312.1980-0",
            family_part, face_part, size_part);
    break;
  default: // for one-byte code
    sprintf(wild_font_name, "*-*-%s-%s-*-*-%i-*-*-*-*-*-iso8859-1", family_part,
            face_part, size_part);
    break;
  }

  // Can't initialize a font_name with wild-characters in it
  // (XLoadQueryFont gives a bus error), so get the name of a real font
  // using XListFonts, and return that.
  char **result;
  int actual_count;
  result = XListFonts(dpy, wild_font_name, 1, &actual_count);

  /*  FOR DEBUGGING:
      printf("wild_font_name = %s\n  result[0] = %s\n", wild_font_name,
             result[0]);
  */

  // could not find a non-scalable font, use well-formed name to
  // try for a scalable font (just skip the '*' beginning wild_font_name)
  if (actual_count == 0) {
    result = XListFonts(dpy, wild_font_name + 1, 1, &actual_count);
  }

  // If the standard font was not found, revert to default font.
  if (actual_count == 0) {
    fprintf(stderr, "make_font_name: Could not match a font to\n");
    fprintf(stderr, "                \"%s\".\n", wild_font_name);
    fprintf(stderr, "             or \"%s\".\n", wild_font_name + 1);
    fprintf(stderr, "                Resorting to default font.\n");

    /* for debugging */
    char **fpp;
    int pathnum;
    fpp = XGetFontPath(dpy, &pathnum);
    fprintf(stderr, "Please check Font Path:\n");
    while (pathnum > 0) {
      fprintf(stderr, "  Font Path Name : %s\n", fpp[--pathnum]);
    }
    XFreeFontPath(fpp);
    /* for debugging */

    switch (family) {
    case Am_FONT_JFIXED:
    case Am_FONT_JPROPORTIONAL:
      result = XListFonts(dpy, Am_DEFAULT_FONT_NAME_J, 1, &actual_count);
      break;
    case Am_FONT_KFIXED:
      result = XListFonts(dpy, Am_DEFAULT_FONT_NAME_K, 1, &actual_count);
      break;
    case Am_FONT_CFIXED:
      result = XListFonts(dpy, Am_DEFAULT_FONT_NAME_C, 1, &actual_count);
      break;
    default:
      result = XListFonts(dpy, Am_DEFAULT_FONT_NAME, 1, &actual_count);
      break;
    }
    if (actual_count == 0) {
      fprintf(stderr, "make_font_name: Could not find a default font\n");
      fprintf(stderr, "                Resorting to another default font.\n");
    }
  }
  // If the default font was not found, revert to another default font.
  if (actual_count == 0) {
    result = XListFonts(dpy, Am_DEFAULT_FONT_NAME, 1, &actual_count);
    if (actual_count == 0) {
      char msg[200];
      strcpy(msg, "** make_font_name: could not find default font  ");
      strcat(msg, Am_DEFAULT_FONT_NAME);
      Am_Error(msg);
    }
  }

  return result;
}

/* frees font name returned by make_font_name() */
void
free_font_name(char **font_names)
{
  XFreeFontNames(font_names);
}

void
initialize_font_from_file(Display *dpy, int /*screen_number*/,
                          const char *font_name, XFontStruct **font_info)
{
  // If the font is valid, you just have to call XLoadQueryFont on it.
  // If it is bogus, you have to do a lot of work to revert to a default font.

  if ((*font_info = XLoadQueryFont(dpy, font_name)) == (0L)) {
    fprintf(stderr, "initialize_font_from_file: Could not open font\n");
    fprintf(stderr, "  \"%s\".  Resorting to default font.\n", font_name);

    // This implementation of reverting to a default font requires that
    // you already know the name of a font that you want to revert to.
    // (Am_DEFAULT_FONT_NAME is defined in gemX.h)
    char **result;
    int actual_count;
    result = XListFonts(dpy, Am_DEFAULT_FONT_NAME, 1, &actual_count);
    font_name = result[0];
    if ((*font_info = XLoadQueryFont(dpy, font_name)) == (0L)) {
      fprintf(stderr, "initialize_font_from_file: Could not open default\n");
      fprintf(stderr, "  font \"%s\" either.  Exiting...\n", font_name);
    }
    XFreeFontNames(result);

    /*      
    // This is the recommended way to get properties of the default font.
    // However, XQueryFont, in it's infinite wisdom, sets the fid slot of the
    // XFontStruct to have the fid of the GC, rather than the default font.
    // This will thwart our later attempt to set the font with XSetFont,
    // since the fid would be wrong, unless we can find out the fid that we
    // want and install it, undoing X's helpful confusion of the GC fid.
    GC default_gc = DefaultGC(dpy, screen_number);
    *font_info = XQueryFont(dpy, XGContextFromGC(default_gc));

    // By asking the default GC about its font, we can identify the
    // default font and install it ourselves in the XFontStruct.
    XGCValues values;
    if (XGetGCValues (dpy, default_gc, GCFont, &values) == 0)
      Am_Error ("** initialize_font_from_file: could not get GC values.\n");
    // Note: Calling XQueryFont on this font (and attempting to do all of this
    // in one step instead of two) does not work -- XQueryFont will return (0L)
    // for the properties of the default font unless you ask for them in the
    // recommended way above.  Thanks, X!
    (*font_info)->fid = values.font;
    */
  }
}

void
initialize_standard_font(Display *dpy, int screen_number, Am_Font_Data *Am_font,
                         XFontStruct **font_info)
{
  Am_Font_Family_Flag family;
  bool is_bold, is_italic, is_underline;
  Am_Font_Size_Flag size;
  char **font_names;

  Am_font->Get_Values(family, is_bold, is_italic, is_underline, size);

  font_names =
      make_font_name(dpy, family, is_bold, is_italic, is_underline, size);

  initialize_font_from_file(dpy, screen_number, font_names[0], font_info);

  free_font_name(font_names);
}

AM_WRAPPER_DATA_IMPL(Am_Font, (this))

Am_Font_Data *Am_Font_Data::list = (0L);

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

const char *
Am_Font_Data::Get_Font_Name() const
{
  return name;
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
  main_xfont = (0L);
  main_display = (0L);
  font_head = (0L);
  char_width = 0;
  next = list;
  list = this;
}

Am_Font_Data::Am_Font_Data(const char *the_name)
{
  name = new char[strlen(the_name) + 1];
  strcpy(name, the_name);
  // Initialize to (0L), and store value when font is first used
  main_xfont = (0L);
  main_display = (0L);
  font_head = (0L);
  char_width = 0;
  next = list;
  list = this;

  // Just in case.  Only underlining should matter here.
  // Above comment is not correct now(Jan.30 1997),
  // bacause whether Japanese, Chinese, Korean, or not is judged by family.
  if (strstr(the_name, "jisx0208") != (0L))
    family = Am_FONT_JFIXED; // Japanese font
  else if (strstr(the_name, "ksc5601") != (0L))
    family = Am_FONT_KFIXED; // Korean font
  else if (strstr(the_name, "gb2312") != (0L) ||
           strstr(the_name, "big5.hku") != NULL)
    family = Am_FONT_CFIXED; // Chinese font
  else
    family = Am_FONT_FIXED;
  bold = false;
  italic = false;
  underline = false;
  size = Am_FONT_SMALL;
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
  main_xfont = (0L);
  main_display = (0L);
  font_head = (0L);
  char_width = 0;
  next = list;
  list = this;
}

Am_Font_Data::~Am_Font_Data()
{
  if (name)
    delete name;
  if (main_display) {
    //// BUG: must note that font is unused
    XFreeFont(main_display, main_xfont);
  }
  Font_Index *current = font_head;
  Font_Index *next = (0L);
  while (current) {
    next = current->next;
    current->next = (0L);
    //// BUG: must note that font is unused
    XFreeFont(current->dpy, current->xfont);
    delete current;
    current = next;
  }
  font_head = (0L);
  remove(this);
}

void
Am_Font_Data::remove(Am_Font_Data *font)
{
  Am_Font_Data *prev = (0L);
  Am_Font_Data *curr = list;
  while (curr) {
    if (curr == font) {
      if (prev)
        prev->next = curr->next;
      else
        list = curr->next;
      return;
    }
    prev = curr;
    curr = curr->next;
  }
}

void
Am_Font_Data::remove(Display *display)
{
  Am_Font_Data *curr;
  for (curr = list; curr; curr = curr->next) {
    if (curr->main_display == display) {
      XFreeFont(curr->main_display, curr->main_xfont);
      curr->main_display = (0L);
    }
    Font_Index *prev = (0L);
    Font_Index *curr_index = curr->font_head;
    while (curr_index) {
      if (curr_index->dpy == display) {
        if (prev)
          prev->next = curr_index->next;
        else
          curr->font_head = curr_index->next;
        XFreeFont(curr_index->dpy, curr_index->xfont);
        delete curr_index;
        break;
      }
      prev = curr_index;
      curr_index = curr_index->next;
    }
  }
}

int
Am_Font_Data::Char_Width(Display *d)
{
  return (d == main_display) ? char_width : 0;
}

bool
Am_Font_Data::Underline()
{
  return underline;
}

bool
Am_Font_Data::Get_Font(Display *dpy, XFontStruct *&xfont)
{
  Font_Index *current;
  for (current = font_head; current != (0L); current = current->next)
    if (current->dpy == dpy) {
      xfont = current->xfont;
      return true;
    }
  return false;
}

XFontStruct *
Am_Font_Data::Get_X_Font(const Am_Drawonable_Impl *d)
{
  Display *disp = d->Get_Display();
  XFontStruct *xfont;
  if (disp == main_display)
    return main_xfont;
  else if (Get_Font(disp, xfont))
    return xfont;
  else {
    XFontStruct *font_info;
    const char *font_name = Get_Font_Name();
    if (font_name == (0L))
      initialize_standard_font(disp, d->screen->screen_number, this,
                               &font_info);
    else
      initialize_font_from_file(disp, d->screen->screen_number, font_name,
                                &font_info);

    if (main_display)
      Add_Font(disp, font_info);
    else {
      main_display = disp;
      main_xfont = font_info;
      // set char_width for fixed width fonts, on main display only
      char_width = (font_info->min_bounds.width == font_info->max_bounds.width)
                       ? font_info->min_bounds.width
                       : 0;
    }

    return font_info;
  }
}

void
Am_Font_Data::Add_Font(Display *display, XFontStruct *xfont)
{
  Font_Index *new_node = new Font_Index(display, xfont);
  new_node->next = font_head;
  font_head = new_node;
}

void
Am_Drawonable_Impl::set_gc_using_font(const Am_Font &Am_font) const
{
  Am_Font_Data *font = Am_Font_Data::Narrow(Am_font);
  XSetFont(screen->display, screen->gc, font->Get_X_Font(this)->fid);
  font->Release();
}

// // // // // // // // // // // // // // // // // // // // // // // // // //
// Draw_Text
// // // // // // // // // // // // // // // // // // // // // // // // // //

// "unwrap_j" translates the byte sequence, which may include the escape
// sequence to two-byte characters sequence without the escape sequence,
// and put it on "buf16".
// It returns the length of the two-byte characters sequence.
#define BUF16SIZE 1024
static XChar2b buf16[BUF16SIZE];
// Can not display the more than 1024 characters(2048 bytes) long sequences
static int
unwrap_j(const char *p, int size)
{
  int i = 0;

  if (size < 0)
    size = BUF16SIZE * 2 + 8; // maxsize including escape sequence
  if (*p ==
      0x1B) { // Two-byte code such as 7bit-JIS code beginning with escape sequence
    p++;
    size--;
    if (*p++ != '$')
      return (0);
    size--;
    switch (*p) {
    case '(':
    case ')':
    case '*':
    case '+':
    case '-':
    case '.':
    case '/':
    case ',':
      p++;
      size--;
      break;
    }
    p++;
    size--; // System do not check the character set.
    while (size > 0 && i < BUF16SIZE && *p != 0x1B && *p != '\0') {
      // ESC or '\0' terminated
      buf16[i].byte1 = *p++;
      size--;
      buf16[i].byte2 = *p;
      if (*p == 0x1B || *p == '\0')
        break;
      p++;
      i++;
      size--;
    }
  } else if ((*p & 0x80) != 0) { // EUC code, that is, its msb is always 1.
    while (size > 0 && i < BUF16SIZE && *p != '\0') {
      // '\0' or Non-EUC code terminated
      if ((*p & 0x80) == 0)
        break;
      buf16[i].byte1 = *p++ & 0x7F;
      size--;
      buf16[i].byte2 = *p & 0x7F;
      if ((*p & 0x80) == 0 || *p == '\0')
        break;
      p++;
      i++;
      size--;
    }
  } else { // any two-byte code without escape sequence
    while (size > 0 && i < 1024 && *p != '\0') { // '\0' terminated
      buf16[i].byte1 = *p++;
      size--;
      buf16[i].byte2 = *p;
      if (*p == '\0')
        break;
      p++;
      i++;
      size--;
    }
  }
  return (i);
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
  GC gc = screen->gc;
  Display *disp = screen->display;
  int x, y, width, height, cw;
  int ascent, descent, garbage;
  XCharStruct overall;
  Am_Font_Family_Flag family;
  bool is_bold, is_italic, is_underline;
  Am_Font_Size_Flag size;

  Am_Style line_style = ls; // These are potentially modified, so we make a copy
  Am_Style fill_style = fs;

  Am_Font_Data *font = Am_Font_Data::Narrow(Am_font);
  font->Get_Values(family, is_bold, is_italic, is_underline, size);
  if (family == Am_FONT_JFIXED || family == Am_FONT_JPROPORTIONAL ||
      family == Am_FONT_KFIXED || family == Am_FONT_CFIXED) {
    str_len = unwrap_j(s, str_len);
    XTextExtents16(font->Get_X_Font(this), buf16, str_len, &garbage, &ascent,
                   &descent, &overall);
  } else
    XTextExtents(font->Get_X_Font(this), s, str_len, &garbage, &ascent,
                 &descent, &overall);

  // On large sizes of times italic the char descent is greater than the
  // font descent.

  descent = descent > overall.descent ? descent : overall.descent;

  if ((cw = font->Char_Width(disp)) == 0) {
    int lb = overall.lbearing > 0 ? 0 : overall.lbearing;
    x = left - lb;
    y = top + ascent;
    height = ascent + descent;
    width = overall.rbearing - lb;
  } else { // fixed width font
    x = left;
    y = top + ascent;
    height = ascent + descent;
    width = str_len * cw;
  }

  if (!line_style.Valid())
    line_style = Am_Style(0.0, 0.0, 0.0); // black if no style
  if (!invert) {
    if (fill_style.Valid()) // draw background rectangle
      this->Draw_Rectangle(Am_No_Style, fill_style, left, top, width, height,
                           f);
    // draw text
    set_gc_using_line(line_style, f);
    set_gc_using_font(Am_font);

    if (family == Am_FONT_JFIXED || family == Am_FONT_JPROPORTIONAL ||
        family == Am_FONT_KFIXED || family == Am_FONT_CFIXED) {
      XDrawString16(disp, xlib_drawable, gc, x, y, buf16, str_len);
      if (is_bold) {
        XDrawString16(disp, xlib_drawable, gc, x + 1, y, buf16, str_len);
      }
      if (font->Underline())
        this->Draw_Line(line_style, left, y + 2, left + width - 1, y + 2, f);
    } else {
      XDrawString(disp, xlib_drawable, gc, x, y, s, str_len);
      if (font->Underline())
        this->Draw_Line(line_style, left, y, left + width - 1, y, f);
    }
  } else { // inverted text: string, underline use fs, box uses ls.
    if (!fill_style.Valid())
      fill_style = Am_Style(1.0, 1.0, 1.0); // white if no fill style
    // draw background rect with line style
    this->Draw_Rectangle(Am_No_Style, line_style, left, top, width, height, f);
    // draw text using background style
    set_gc_using_line(fill_style, f);
    set_gc_using_font(Am_font);

    if (family == Am_FONT_JFIXED || family == Am_FONT_JPROPORTIONAL ||
        family == Am_FONT_KFIXED || family == Am_FONT_CFIXED) {
      XDrawString16(disp, xlib_drawable, gc, x, y, buf16, str_len);
      if (is_bold) {
        XDrawString16(disp, xlib_drawable, gc, x + 1, y, buf16, str_len);
      }
      if (font->Underline())
        this->Draw_Line(line_style, left, y + 2, left + width - 1, y + 2, f);
    } else {
      XDrawString(disp, xlib_drawable, gc, x, y, s, str_len);
      if (font->Underline())
        this->Draw_Line(fill_style, left, y, left + width - 1, y, f);
    }
  }
  font->Release();
}

// // // // // // // // // // // // // // // // // // // // // // // // // //
// Am_Font operations, must have a Am_Drawonable and a Am_Font to get sizes
// // // // // // // // // // // // // // // // // // // // // // // // // //

int
Am_Drawonable_Impl::Get_Char_Width(const Am_Font &Am_font, char c)
{
  int cw;

  Am_Font_Data *font = Am_Font_Data::Narrow(Am_font);
  if ((cw = font->Char_Width(screen->display)) > 0) {
    font->Release();
    return cw;
  }

  // Variable width font: call XTextExtents to get the actual character width,
  // instead of the logical width (how far it moves to the next character pos)

  int garbage, lb;
  XCharStruct overall;
  XTextExtents(font->Get_X_Font(this), &c, 1, &garbage, &garbage, &garbage,
               &overall);
  // don't cut into the character, if it starts to the right of the origin.
  lb = overall.lbearing > 0 ? 0 : overall.lbearing;

  font->Release();
  return overall.rbearing - lb;
}

int
Am_Drawonable_Impl::Get_String_Width(const Am_Font &Am_font,
                                     const char *the_string,
                                     int the_string_length)
{
  int cw;

  Am_Font_Data *font = Am_Font_Data::Narrow(Am_font);
  if (!font)
    return 0;

  Am_Font_Family_Flag family;
  bool is_bold, is_italic, is_underline;
  Am_Font_Size_Flag size;
  font->Get_Values(family, is_bold, is_italic, is_underline, size);
  if ((cw = font->Char_Width(screen->display)) > 0) {
    if (family == Am_FONT_JFIXED || family == Am_FONT_JPROPORTIONAL ||
        family == Am_FONT_KFIXED || family == Am_FONT_CFIXED) {
      the_string_length = unwrap_j(the_string, the_string_length);
    }
    font->Release();
    return cw * the_string_length;
  }

  // Variable width font: call XTextExtents to get the actual string width,
  // instead of the logical width (how far it moves to the next character pos)

  int garbage;
  XCharStruct overall;
  if (family == Am_FONT_JFIXED || family == Am_FONT_JPROPORTIONAL ||
      family == Am_FONT_KFIXED || family == Am_FONT_CFIXED) {
    the_string_length = unwrap_j(the_string, the_string_length);
    XTextExtents16(font->Get_X_Font(this), buf16, the_string_length, &garbage,
                   &garbage, &garbage, &overall);
  } else
    XTextExtents(font->Get_X_Font(this), the_string, the_string_length,
                 &garbage, &garbage, &garbage, &overall);

  font->Release();
  return overall.width;

  // The following code was removed because it returned the width of "inked"
  // pixels. This caused problems when the_string had spaces on the end

  // don't cut into the character, if it starts to the right of the origin.
  // int lb = overall.lbearing > 0 ? 0 : overall.lbearing;

  //font->Release ();
  //overall.rbearing - lb;
}

void
Am_Drawonable_Impl::Get_String_Extents(const Am_Font &Am_font,
                                       const char *the_string,
                                       int the_string_length, int &width,
                                       int &ascent, int &descent,
                                       int &left_bearing, int &right_bearing)
{
  int the_direction;
  XCharStruct the_overall;

  Am_Font_Data *font = Am_Font_Data::Narrow(Am_font);

  if (font) {
    Am_Font_Family_Flag family;
    bool is_bold, is_italic, is_underline;
    Am_Font_Size_Flag size;
    font->Get_Values(family, is_bold, is_italic, is_underline, size);
    if (family == Am_FONT_JFIXED || family == Am_FONT_JPROPORTIONAL ||
        family == Am_FONT_KFIXED || family == Am_FONT_CFIXED) {
      the_string_length = unwrap_j(the_string, the_string_length);
      XTextExtents16(font->Get_X_Font(this), buf16, the_string_length,
                     &the_direction, &ascent, &descent, &the_overall);
    } else
      XTextExtents(font->Get_X_Font(this), the_string, the_string_length,
                   &the_direction, &ascent, &descent, &the_overall);
    width = the_overall.width;
    left_bearing = the_overall.lbearing;
    right_bearing = the_overall.rbearing;
    font->Release();
  } else {
    width = 0;
    ascent = 0;
    descent = 0;
    left_bearing = 0;
    right_bearing = 0;
  }
}

void
Am_Drawonable_Impl::Get_Font_Properties(const Am_Font &Am_font,
                                        int &max_char_width,
                                        int &min_char_width, int &char_ascent,
                                        int &char_descent)
{
  if (!Am_font.Valid()) {
    max_char_width = 0;
    min_char_width = 0;
    char_ascent = 0;
    char_descent = 0;
    return;
  }
  Am_Font_Data *font = Am_Font_Data::Narrow(Am_font);

  XFontStruct *the_xfont = font->Get_X_Font(this);

  max_char_width = the_xfont->max_bounds.width;
  min_char_width = the_xfont->min_bounds.width;
  // Return max ascent and descent values w.r.t. vertical spacing,
  // not character dimensions.
  // Note: with some characters in some fonts, in particular lowercase 'g' in
  //   times italic 24, the character's descent is greater than the font's
  //   descent.  The ?: should take care of that.
  char_ascent = the_xfont->ascent;
  char_descent = the_xfont->descent > the_xfont->max_bounds.descent
                     ? the_xfont->descent
                     : the_xfont->max_bounds.descent;
  font->Release();
}

void
Am_Font_Data::Print(std::ostream &os) const
{
  const char *lookup_name = (0L);
#ifdef DEBUG
  lookup_name = Am_Get_Name_Of_Item(this);
#endif
  if (lookup_name)
    os << lookup_name;
  else if (name)
    os << "Am_Font From File(" << name << ")";
  else {
    os << "Am_Font(" << std::hex << (unsigned long)this << std::dec << ")=[";
    switch (family) {
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
    if (bold)
      os << ", BOLD";
    if (italic)
      os << ", ITALIC";
    if (underline)
      os << ", UNDERLINE";
    switch (size) {
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
