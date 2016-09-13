/* ****************************** -*-c++-*- *******************************/
/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This is an internal file containing object definitions for structures
   shared among the internal Mac-specific Gem files.  Neither Gem nor Amulet
   users should have to handle the objects defined in this file.
   
*/

#ifndef GEMM_H
#define GEMM_H

#include <am_inc.h>

#include <amulet/types.h> // to get the Value object
#include <amulet/gdefs.h> // constants and definitions for graphics
#include <amulet/idefs.h> // constants and definitions for input
#include <amulet/gem.h>
#include <amulet/gem_image.h>

// // // // // // // // // // // // // // // // // // // // // // // // // //
// Utility functions and definitions
// // // // // // // // // // // // // // // // // // // // // // // // // //

#define MIN(a, b) (a < b) ? a : b
#define MAX(a, b) (a > b) ? a : b

#define Am_DEFAULT_FONT_NAME "*-*-fixed-medium-r-*--*-120-*-*-*-*-iso8859-1"

// Font defaults for Mac
#define Am_FIXED_FONT_FAMILY monaco
#define Am_SERIF_FONT_FAMILY times
#define Am_SANS_SERIF_FONT_FAMILY helvetica

#define Am_SMALL_POINT_SIZE 9
#define Am_MEDIUM_POINT_SIZE 12
#define Am_LARGE_POINT_SIZE 18
#define Am_VERY_LARGE_POINT_SIZE 24

void mac_initialize(); // called to make sure mac is initialized
short get_graphical_line_thickness(Am_Style ls);

const unsigned long kAppleMenuID = 128;
const unsigned long kFileMenuID = 129;

const unsigned long kNoSleepTime = 0;
const unsigned long kReservedMemSize = 32768;

const unsigned long kLowOnMemory = 128;
const unsigned long kOutOfMemory = 129;

const unsigned long kAboutBoxAlert = 256;

static bool mac_inited = false;

class Am_Drawonable_Impl;

// // // // // // // // // // // // // // // // // // // // // // // // // //
//
//   Am_Drawonable_Impl
//
// // // // // // // // // // // // // // // // // // // // // // // // // //

struct child
{
  Am_Drawonable_Impl *d;
  struct child *next;
};

class Am_Drawonable_Impl : public Am_Drawonable
{
public:
  Am_Drawonable *
  Create(int l = 0, int t = 0, unsigned int w = 100, unsigned int h = 100,
         const char *tit = "", const char *icon_tit = "", bool vis = true,
         bool initially_iconified = false, Am_Style back_color = Am_No_Style,
         bool save_under_flag = false,
         int min_w = 1, // can't have 0 size windows
         int min_h = 1,
         int max_w = 0, // 0 is illegal so means no max
         int max_h = 0, bool title_bar_flag = true,
         bool query_user_for_position = false, bool query_user_for_size = false,
         bool clip_by_children_flag = true, Am_Input_Event_Handlers *ev = (0L));
  void Destroy();

  Am_Drawonable *Create_Offscreen(int width = 0, int height = 0,
                                  Am_Style background_color = Am_No_Style);

  void Reparent(Am_Drawonable *new_parent);
  bool Inquire_Window_Borders(int &left_border, int &top_border,
                              int &right_border, int &bottom_border,
                              int &outer_left, int &outer_top);

  void Raise_Window(Am_Drawonable *target_d); // to top
  void Lower_Window(Am_Drawonable *target_d); // to bottom
  void Set_Iconify(bool iconified);
  void Set_Title(const char *new_title);
  void Set_Icon_Title(const char *new_title);
  void Set_Position(int new_left, int new_top);
  void Set_Size(unsigned int new_width, unsigned int new_height);
  void Set_Max_Size(unsigned int new_width, unsigned int new_height);
  void Set_Min_Size(unsigned int new_width, unsigned int new_height);
  void Set_Visible(bool vis);
  void Set_Titlebar(bool new_title_bar);
  void Set_Cursor(Am_Cursor cursor);
  void Set_Background_Color(Am_Style new_color);
  //Find the child-most drawonable at the current cursor position
  virtual Am_Drawonable *Get_Drawonable_At_Cursor();

  // general operations

  void Beep();

  // destination for bitblt is the Am_Drawonable this message is sent to
  void Bitblt(int d_left, int d_top, int width, int height,
              Am_Drawonable *source, int s_left, int s_top,
              Am_Draw_Function df = Am_DRAW_COPY);

  void Clear_Area(int left, int top, int width, int height);

  void Flush_Output();

  void Translate_Coordinates(int src_x, int src_y, Am_Drawonable *src_d,
                             int &dest_x_return, int &dest_y_return);

  // Am_Image_Array operations:  must have an Am_Drawonable to get the size
  // of an image since it may need to be created
  void Get_Image_Size(const Am_Image_Array &image, int &ret_width,
                      int &ret_height);
  // verify that image is loaded (or loadable)

  void Print(std::ostream &os) const;

  /********** NDY: THIS NEEDS TO BE IMPLEMENTED **********/

  bool Test_Image(const Am_Image_Array & /* image */) const { return true; }

  // Am_Font operations, must have a Am_Drawonable and a Am_Font to get sizes

  int Get_Char_Width(const Am_Font &Am_font, char c);
  int Get_String_Width(const Am_Font &Am_font, const char *the_string,
                       int the_string_length);
  void Get_String_Extents(const Am_Font &Am_font, const char *the_string,
                          int the_string_length, int &width, int &ascent,
                          int &descent, int &left_bearing, int &right_bearing);

  void Get_Font_Properties(const Am_Font &Am_font, int &max_char_width,
                           int &min_char_width, int &char_ascent,
                           int &char_descent);

  // Am_Polygon operations
  // polygon bounding box calculation
  void Get_Polygon_Bounding_Box(const Am_Point_List &pl, const Am_Style &ls,
                                int &out_left, int &out_top, int &width,
                                int &height);

  void Clear_Clip();
  void Set_Clip(Am_Region *the_region);
  void Set_Clip(int the_left, int the_top, unsigned int the_width,
                unsigned int the_height);
  void Push_Clip(Am_Region *the_region);
  void Push_Clip(int the_left, int the_top, unsigned int the_width,
                 unsigned int the_height);
  void Pop_Clip();
  bool In_Clip(int x, int y);
  bool In_Clip(int left, int top, unsigned int width, unsigned int height,
               bool &total);
  bool In_Clip(Am_Region *rgn, bool &total);

  // Specific Drawing functions
  void Draw_Arc(const Am_Style &ls, const Am_Style &fs, int left, int top,
                unsigned int width, unsigned int height, int angle1 = 0,
                int angle2 = 360, Am_Draw_Function f = Am_DRAW_COPY,
                Am_Arc_Style_Flag asf = Am_ARC_PIE_SLICE);
  void
  Draw_Image(int left, int top, int width, int height,
             const Am_Image_Array &image, int i_left = 0, int i_top = 0,
             const Am_Style &ls = Am_No_Style, //color of 'on' bits
             const Am_Style &fs = Am_No_Style, //for background behind image
             bool draw_monochrome = false, //use ls and fs in place of colors
             Am_Draw_Function f = Am_DRAW_COPY);
  void Draw_Line(const Am_Style &ls, int x1, int y1, int x2, int y2,
                 Am_Draw_Function f = Am_DRAW_COPY);
  //internal function used for all polygon drawing functions below
  void poly_draw_it(const Am_Style &ls, const Am_Style &fs, Am_Draw_Function f,
                    Point *ar, int num_points);
  void Draw_Lines(const Am_Style &ls, const Am_Style &fs,
                  const Am_Point_Array &pts, Am_Draw_Function f = Am_DRAW_COPY);
  void Draw_2_Lines(const Am_Style &ls, const Am_Style &fs, int x1, int y1,
                    int x2, int y2, int x3, int y3,
                    Am_Draw_Function f = Am_DRAW_COPY);
  void Draw_3_Lines(const Am_Style &ls, const Am_Style &fs, int x1, int y1,
                    int x2, int y2, int x3, int y3, int x4, int y4,
                    Am_Draw_Function f = Am_DRAW_COPY);
  void Draw_Rectangle(const Am_Style &ls, const Am_Style &fs, int left, int top,
                      int width, int height, Am_Draw_Function f = Am_DRAW_COPY);
  void Draw_Roundtangle(const Am_Style &ls, const Am_Style &fs, int left,
                        int top, int width, int height, unsigned short x_radius,
                        unsigned short y_radius,
                        Am_Draw_Function f = Am_DRAW_COPY);
  void Draw_Text(const Am_Style &ls, const char *s, int str_len,
                 const Am_Font &Am_font, int left, int top,
                 Am_Draw_Function f = Am_DRAW_COPY,
                 const Am_Style &fs = Am_No_Style, // background fill style
                 bool invert = false);

  // cut buffer, eventually, this needs to be generalized
  void Set_Cut_Buffer(const char *s);
  char *Get_Cut_Buffer();

  // input handling
  void Initialize_Event_Mask();
  void set_drawable_event_mask();
  void Set_Enter_Leave(bool want_enter_leave_events);
  void Set_Want_Move(bool want_move_events);
  void Set_Multi_Window(bool want_multi_window);

  void Get_Window_Mask(bool &want_enter_leave_events, bool &want_move_events,
                       bool &want_multi)
  {
    want_enter_leave_events = want_enter_leave;
    want_move_events = want_move;
    want_multi = want_multi_window;
  }

  void Discard_Pending_Events();
  void Set_Input_Dispatch_Functions(Am_Input_Event_Handlers *evh);
  void Get_Input_Dispatch_Functions(Am_Input_Event_Handlers *&evh);

  // Utility drawing functions -- declared as member functions because
  // they need to access data in the drawonable
  void adjust_pen_stipple(const Am_Image_Array &fill_stipple);
  void set_pen_using_fill(const Am_Style &fill, Am_Draw_Function f,
                          const Am_Image_Array &image = Am_No_Image);
  void set_pen_using_line(const Am_Style &line, Am_Draw_Function f);
  void set_pen_using_font(const Am_Font &Am_font);

  void set_fore_color(const Am_Style &style);
  void set_back_color(const Am_Style &style);

  // Invoke focus_on_this() to direct future drawing to this drawonable.
  // BUG: For efficiency, this should be called once per update of an entire
  // window, rather than once per drawn object.  Maybe move this into Opal?
  void focus_on_this();
  void add_total_offset(int &x, int &y);
  void add_total_offset_global(int &x, int &y);
  void add_child(Am_Drawonable_Impl *d);
  void delete_child(Am_Drawonable_Impl *d);

  //constructor function
  Am_Drawonable_Impl(int l, int t, unsigned int w, unsigned int h,
                     const char *tit, const char *icon_tit, bool vis,
                     bool initially_iconified, Am_Style back_color,
                     bool save_under_flag, int min_w, int min_h, int max_w,
                     int max_h, bool title_bar_flag, bool clip_by_children_flag,
                     unsigned int bit_depth, Am_Input_Event_Handlers *evh);

  ~Am_Drawonable_Impl();

  //Utility constructor function
  void create_mac_window();
  void create_mac_offscreen();

  // Called from gemM_input.cc for window movement
  void reconfigure(int new_left, int new_top, int new_width, int new_height)
  {
    left = new_left;
    top = new_top;
    width = new_width;
    height = new_height;
  }

  // new functions
  bool is_toplevel_window() { return is_toplevel; }
  void update_area(int x, int y, int width, int height);
  void invalidate();
  void get_upper_left(int &x, int &y);
  //void	calc_window_rgn(RgnHandle mac_rgn);
  //void	calc_vis_rgn(RgnHandle vis_rgn);

  void calc_exposed_rgn();
  void subtract_child_windows(child *children);

  void out_of_focus();
  void draw_grow_box();
  void get_global_bounds(Rect &bounds);

  //data
  // variable to hold the event handlers for this window
  Am_Input_Event_Handlers *event_handlers;

  //extra data
  Am_Drawonable *owner;    //Am_Drawonable this is nested in
  WindowRecord mac_winrec; //holds the window record itself
  WindowRef mac_window; //corresponds to an X drawable  .. pointer to mac_winrec
  CGrafPtr mac_port;
  Cursor *mac_cursor;
  bool is_toplevel;
  bool is_offscreen;
  RgnHandle exposed_rgn;
  static Am_ID_Tag id;

  //yet more data
  Am_Region *clip_region;
  child *children;
  child *last_child;

  // flags for input handling
  bool want_enter_leave;
  bool want_multi_window;
  bool want_move;
  long current_event_mask;

  //protected:
  static GrafPtr util_port;
};

// Given a Mac window, returns the corresponding drawonable object
extern Am_Drawonable_Impl *Get_Drawable_Backpointer(WindowRef a_mac_win);
extern void Set_Drawable_Backpointer(WindowRef a_mac_win,
                                     Am_Drawonable_Impl *d);

class Am_Font_Data : public Am_Wrapper
{
  friend class Am_Font;
  AM_WRAPPER_DATA_DECL(Am_Font)
public:
  Am_Font_Data(Am_Font_Data *proto);
  Am_Font_Data(const char *the_name);
  Am_Font_Data(Am_Font_Family_Flag f = Am_FONT_FIXED, bool is_bold = false,
               bool is_italic = false, bool is_underline = false,
               Am_Font_Size_Flag s = Am_FONT_MEDIUM);

  ~Am_Font_Data();

  bool operator==(Am_Font_Data &) { return false; }

  // get values
  void Get_Values(Am_Font_Family_Flag &f, bool &is_bold, bool &is_italic,
                  bool &is_underline, Am_Font_Size_Flag &s) const;
  FMInput *Get_Mac_InFont(Am_Drawonable_Impl *d);
  FMOutPtr Get_Mac_OutFont(Am_Drawonable_Impl *d);
  void install_standard_infont(Am_Drawonable_Impl *d);
  void install_nonstandard_infont(Am_Drawonable_Impl *d);

  int Char_Width();
  bool Underline();

  // data
protected:
  Am_Font_Family_Flag family;
  bool bold;
  bool italic;
  bool underline;
  Am_Font_Size_Flag size;
  char *name;
  FMInput *mac_infont;
  FMOutput mac_outfont;

  int char_width; //??? keep it?
};

class Am_Cursor_Data : public Am_Wrapper
{
  friend class Am_Cursor;
  AM_WRAPPER_DATA_DECL(Am_Cursor)

public:
  // creators
  Am_Cursor_Data(Am_Cursor_Data *proto);
  Am_Cursor_Data();
  Am_Cursor_Data(Am_Image_Array image, Am_Image_Array mask, Am_Style fg_color,
                 Am_Style bg_color);
  ~Am_Cursor_Data();

  Cursor *Get_Mac_Cursor(Am_Drawonable_Impl *draw);

  bool operator==(Am_Cursor_Data &) { return false; }

protected:
  /* holds data till we need it */
  Am_Image_Array image;
  Am_Image_Array mask;
  Am_Style fg_color;
  Am_Style bg_color;
  Cursor cursor;
  int x_hot;
  int y_hot;
  bool inited;
};

class Am_Image_Array_Data : public Am_Wrapper
{
  friend class Am_Image_Array;
  AM_WRAPPER_DATA_DECL(Am_Image_Array)
public:
  Am_Image_Array_Data();
  Am_Image_Array_Data(Am_Image_Array_Data *proto);
  Am_Image_Array_Data(const char *file_name);
  Am_Image_Array_Data(const char *bit_data, int height, int width);

  ~Am_Image_Array_Data();

  PixMapHandle Get_Mac_PixMap(Am_Drawonable_Impl *draw);
  bool operator==(Am_Image_Array_Data &) { return false; }

  Pattern Get_Mac_Pattern(Am_Drawonable_Impl * /* draw */) { return pattern; }
  bool Is_Transparent() { return transp; }
  Am_RGB_Value Transp_Color() { return transp_color; }
protected:
  void load_image_from_file();
  void load_bit_data(const unsigned char *bit_data, unsigned short width,
                     unsigned short height);
  void load_image_data(const unsigned char *bit_data, unsigned short width,
                       unsigned short height, int depth,
                       Am_RGB_Value *color_map, int num_colors);

  GWorldPtr gworld;
  BitMap bitmap;
  Pattern pattern;           // only defined for construction by percent
  char *name;                // holds the name of the bitmap till we need it
  bool transp;               // is true if background is transparent
  Am_RGB_Value transp_color; // and if it is transparent this is the color
};

class Am_Style_Data : public Am_Wrapper
{
  friend class Am_Style;
  AM_WRAPPER_DATA_DECL(Am_Style)
public:
  Am_Style_Data(Am_Style_Data *proto);
  Am_Style_Data(float r, float g, float b, short thickness,
                Am_Line_Cap_Style_Flag cap, Am_Join_Style_Flag join,
                Am_Line_Solid_Flag line_flag, const char *dash_l,
                int dash_l_length, Am_Fill_Solid_Flag fill_flag,
                Am_Fill_Poly_Flag poly, Am_Image_Array stipple);

  Am_Style_Data(const char *cname, short thickness, Am_Line_Cap_Style_Flag cap,
                Am_Join_Style_Flag join, Am_Line_Solid_Flag line_flag,
                const char *dash_l, int dash_l_length,
                Am_Fill_Solid_Flag fill_flag, Am_Fill_Poly_Flag poly,
                Am_Image_Array stipple);

  Am_Style_Data(int percent, Am_Fill_Solid_Flag fill_flag);
  Am_Style_Data(const char *name, bool bit_is_on);
  Am_Style_Data(Am_Style_Data *proto, Am_Style_Data *new_color);
  // not in X version
  Am_Style_Data(char *bit_data, int height, int width);

  Am_Style_Data();
  ~Am_Style_Data();

  bool operator==(Am_Style_Data &) { return false; }

  const char *Get_Color_Name();
  //returns a pointer to the string, don't dealloc

  const RGBColor *Get_Mac_Color(Am_Drawonable_Impl *d);

protected:
  // data
  char *color_name;

  //color part
  float red, green, blue;
  RGBColor mac_color;

  //line style part
  short line_thickness;
  Am_Line_Cap_Style_Flag cap_style;
  Am_Join_Style_Flag join_style;
  Am_Line_Solid_Flag line_solid;
  char *dash_list;
  int dash_list_length;

  //filling style part
  Am_Fill_Solid_Flag fill_solid;
  Am_Fill_Poly_Flag fill_poly;
  Am_Image_Array stipple_bitmap;
};

class Am_Point_Array_Data : public Am_Wrapper
{
  friend class Am_Point_Array;
  friend class Am_Drawonable_Impl; // for direct access to XPoint representation

  AM_WRAPPER_DATA_DECL(Am_Point_Array)
public:
  Am_Point_Array_Data(Am_Point_Array_Data *proto);
  Am_Point_Array_Data(Am_Point_List pl, int offset_x, int offset_y);
  Am_Point_Array_Data(int *coords, int num_coords);
  Am_Point_Array_Data(int size);
  ~Am_Point_Array_Data();

  operator==(Am_Point_Array_Data &) { return false; }

protected:
  Point *ar;
  int size;
};

class Am_Region_Impl : public Am_Region
{
public:
  // creator function is Am_Region::Create
  void Destroy();
  void Clear();
  void Set(int left, int top, unsigned int width, unsigned int height);
  void Push(Am_Region *region);
  void Push(int left, int top, unsigned int width, unsigned int height);
  void Pop();
  void Union(int left, int top, unsigned int width, unsigned int height);
  void Intersect(int left, int top, unsigned int width, unsigned int height);
  bool In(int x, int y);
  bool In(int left, int top, unsigned int width, unsigned int height,
          bool &total);
  bool In(Am_Region *rgn, bool &total);

  // Internal functions
  Am_Region_Impl();
  bool all_rgns_used();
  void add_more_rgns();
  RgnHandle region_to_use() { return mac_rgns[index]; }

  // data
  RgnHandle *mac_rgns;
  int size, index, max_index;
};

/* class Am_Time_Data:
 *     These declarations were copied out of gemX_time.h, which Alan felt
 *     was somewhat not necessary. Eventually that code should be moved
 *     into gemX.h.
 */

typedef struct
{
  long tv_sec, tv_usec;
} timeval;

class Am_Time_Data : public Am_Wrapper
{
  friend class Am_Time;
  AM_WRAPPER_DATA_DECL(Am_Time)
public:
  Am_Time_Data()
  {
    time.tv_usec = 0;
    time.tv_sec = 0;
  }
  Am_Time_Data(Am_Time_Data *other)
  {
    time.tv_usec = other->time.tv_usec;
    time.tv_sec = other->time.tv_sec;
  }
  timeval time;
  operator==(Am_Time_Data &other)
  {
    return time.tv_usec == other.time.tv_usec &&
           time.tv_sec == other.time.tv_sec;
  }
  void Print(std::ostream &out) const;
};

#endif

/*******************************************************************************
 * Key Mapping
 *   See gemM_input.cc for an explanation of these consts
 */

// navigation and deletion characters
const unsigned char char_Home = 0x01;
const unsigned char char_Enter = 0x03;
const unsigned char char_End = 0x04;
const unsigned char char_Help = 0x05;
const unsigned char char_Delete = 0x08;
const unsigned char char_Tab = 0x09;
const unsigned char char_PageUp = 0x0B;
const unsigned char char_PageDown = 0x0C;
const unsigned char char_Return = 0x0D;
const unsigned char char_LeftArrow = 0x1C;
const unsigned char char_RightArrow = 0x1D;
const unsigned char char_UpArrow = 0x1E;
const unsigned char char_DownArrow = 0x1F;
const unsigned char char_FwdDelete = 0x7F;

const unsigned char char_FirstPrinting = 0x20;
const unsigned char char_LastPrinting = 0x7E;

// ambiguous characters which require the virtual key code for correct interpretation

const unsigned char char_Clear = 0x1B;
const unsigned char vKey_Clear = 0x47;

const unsigned char char_Escape = 0X1B;
const unsigned char vKey_Escape = 0x35;

const unsigned char char_FunctionKey = 0x10;
const unsigned char vKey_F1 = 0x7A;
const unsigned char vKey_F2 = 0x78;
const unsigned char vKey_F3 = 0x63;
const unsigned char vKey_F4 = 0x76;
const unsigned char vKey_F5 = 0x60;
const unsigned char vKey_F6 = 0x61;
const unsigned char vKey_F7 = 0x62;
const unsigned char vKey_F8 = 0x64;
const unsigned char vKey_F9 = 0x65;
const unsigned char vKey_F10 = 0x6D;
const unsigned char vKey_F11 = 0x67;
const unsigned char vKey_F12 = 0x6F;
const unsigned char vKey_F13 = 0x69;
const unsigned char vKey_F14 = 0x6B;
const unsigned char vKey_F15 = 0x71;

/********************************************************************************
 * procedure to replace SetPort so all of the drawing pixMaps are
 * locked when we draw on them
 */

class DrawingPort
{
public:
  DrawingPort(Rect *portRect);
  DrawingPort(WindowRef win);
  ~DrawingPort();
  BitMap *GetBitMap();
  GrafPtr GetDrawingPort();
  void SetCurrentPort();

private:
  GWorldPtr offscreenGWorld;
  PixMapHandle pixH;
};

DrawingPort *GetCurrentPort(void);
void SetCurrentPort(CGrafPtr non_drawing_port);

PixMapHandle lock_current_pixmap();
void unlock_current_pixmap(PixMapHandle);
