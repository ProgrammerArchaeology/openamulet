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

// This is an internal file containing object definitions for structures
// shared among the internal X-specific Gem files.  Only users of Amulet who
// need to get at internal X-specific pieces should use the file.  This file
// is NOT used by the Mac or Windows versions of Amulet

// NOTE FOR DEBUGGING X WINDOWS CODE:
// See comments in gemX_windows.cc Am_Drawonable_Impl::Get_Root_Drawonable
// for information on how to easily debug X Windows errors in Amulet code.
// af1x 5-21-96

#ifndef GEMX_H
#define GEMX_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>

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

extern unsigned long Am_GC_setmask;
extern unsigned long Am_GC_getmask;

// the following fails on NCD X terminals (no matching font)
// #define Am_DEFAULT_FONT_NAME "*-*-fixed-medium-r-*-*-12-*-*-*-*-*-iso8859-1"

#define Am_DEFAULT_FONT_NAME "*-*-fixed-medium-r-*--*-120-*-*-*-*-iso8859-1"
#define Am_DEFAULT_FONT_NAME_J "*-*-*-medium-r-*-*-16-*-*-*-*-*-jisx0208.1983-0"
#define Am_DEFAULT_FONT_NAME_C "*-*-*-medium-r-*-*-16-*-*-*-*-*-gb2312.1980-0"
#define Am_DEFAULT_FONT_NAME_K "*-*-*-medium-r-*-*-16-*-*-*-*-*-ksc5601.1987-0"

// Font defaults for X
#define Am_FIXED_FONT_FAMILY "courier"
#define Am_SERIF_FONT_FAMILY "times"
#define Am_SANS_SERIF_FONT_FAMILY "helvetica"
#define Am_TWO_BYTE_FONT_FAMILY "*"
// Japanese, Chinese, and Korean font family

#define Am_SMALL_PIXEL_SIZE 10
#define Am_MEDIUM_PIXEL_SIZE 12
#define Am_LARGE_PIXEL_SIZE 18
#define Am_VERY_LARGE_PIXEL_SIZE 24
#define Am_TSMALL_PIXEL_SIZE 8
#define Am_TMEDIUM_PIXEL_SIZE 14
#define Am_TLARGE_PIXEL_SIZE 16
#define Am_TVERY_LARGE_PIXEL_SIZE 24
// 'Am_T*' are sizes added for Two-byte code fonts.
// But we have only a few good fonts on X for two-byte code now.
// In Japanese, for example, the sizes of good fonts are 14, 16, and 24.
// You may be able to use only one font whose size is 16 or 24
//    for Chinese and Korean.
// So, I determined as above. In future, they should be changed.
//    Jan.30, 1997            Y.Tsujino

typedef void Am_External_Event_Handler(XEvent *event);

// // // // // // // // // // // // // // // // // // // // // // // // // //
// Screen_Desc
// // // // // // // // // // // // // // // // // // // // // // // // // //

extern bool More_Than_One_Display; // tells whether more than one
                                   // display is in use.  Needed
                                   // by Main_Event_Loop.
extern Display *Main_Display; // if only one display, then it is stored here.

class Am_Drawonable_Impl;

class Screen_Manager
{
public:
  Screen_Manager();
  ~Screen_Manager();
  void Add(const char *screen, Display *dpy, Am_Drawonable *draw);
  bool Get(const char *name, Display *&dpy, Am_Drawonable *&draw);
  void Remove(const char *name);
  Am_Drawonable *Member(Display *dpy);
  void Block(timeval *timeout);
  void Wait_For_Event(timeval *timeout);
  void Next_Event(XEvent *event_return, timeval *timeout);
  void Remove(Am_Drawonable *draw);
  bool Pending(XEvent *event_return = nullptr);
  void Put_Event_Back(XEvent &event);

private:
  // circular list of screens
  struct Screen_Info
  {
    Screen_Info(const char *_name, Display *_dpy, Am_Drawonable *_draw)
    {
      if (_name) {
        name = new char[strlen(_name) + 1];
        strcpy(name, _name);
      } else
        name = nullptr;
      dpy = _dpy;
      draw = _draw;
    }
    ~Screen_Info()
    {
      if (name)
        delete[] name;
      /*draw->Destroy ();*/
      /*XCloseDisplay(dpy);*/
    }
    char *name;          // screen name
    Display *dpy;        // X display handle
    Am_Drawonable *draw; // Amulet root drawonable
    Screen_Info *next;
    Screen_Info *prev;
  };

  void Remove(Screen_Info *);

  // since it's circular, we need only one pointer into it
  Screen_Info *head; // next display to check for events
                     // (head->prev returned the most recent event)

  fd_set read_flag; // for select system call.
  int nfds;         // largest X connection number, plus one
};

class Screen_Desc
{
public:
  Screen_Desc(Display *dpy, int num, Am_Drawonable_Impl *root_drawonable,
              Colormap c, int d, unsigned long black, unsigned long white,
              GC screen_gc)
  {
    display = dpy;
    screen_number = num;
    root = root_drawonable;
    colormap = c;
    depth = d;
    blackpixel = black;
    whitepixel = white;
    gc = screen_gc;
    last_click_state = 0;
    last_click_code = 0;
    last_click_time = 0;
    click_counter = 0;
    cut_data = nullptr;
    refs = 1;
  }
  ~Screen_Desc();
  void Note_Reference() { ++refs; }
  void Release()
  {
    if (!--refs)
      delete this;
  }

  // reference counter (for deallocation)
  int refs;

  // data
  Display *display;         // xlib display
  Atom cut_buffer;          // atom to store selection in
  char *cut_data;           // the data we've put into the cut buffer
  int screen_number;        //xlib screen number
  Am_Drawonable_Impl *root; // root drawonable
  Colormap colormap;        //default color map for this screen
  int depth;                //depth of screen
  unsigned long blackpixel; //index of black pixel
  unsigned long whitepixel; //index of white pixel
  GC gc;                    // a graphics context to use
  Am_Region *clip_region;

  // variables to implement double-clicking (needs to be separate per screen)
  unsigned int last_click_state;
  int last_click_code;
  unsigned long last_click_time;
  int click_counter;
};

// // // // // // // // // // // // // // // // // // // // // // // // // //
//
//   Am_Drawonable_Impl
//
// // // // // // // // // // // // // // // // // // // // // // // // // //

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
         bool clip_by_children_flag = true, Am_Input_Event_Handlers *ev = nullptr);
  void Destroy();

  // internal functions
  void Create_X_Drawable(Am_Drawonable_Impl *parent_d, bool ask_position,
                         bool ask_size, XSetWindowAttributes *attrib,
                         unsigned long attrib_value_mask);
  void Create_Offscreen_X_Drawable();
  Display *Get_Display() const { return screen->display; }
  Colormap Get_Colormap() const { return screen->colormap; }
  void Initialize_Event_Mask();

  // X-Windows doesn't allow 0-sized windows, so we use default values of 1
  Am_Drawonable *Create_Offscreen(int width = 1, int height = 1,
                                  Am_Style background_color = Am_No_Style);

  bool add_wm_border_offset(Window xlib_drawable, Window expected_parent,
                            int &lb, int &tb, int &rb, int &bb, int &outer_left,
                            int &outer_top);
  void Reparent(Am_Drawonable *new_parent);
  bool Inquire_Window_Borders(int &left_border, int &top_border,
                              int &right_border, int &bottom_border,
                              int &outer_left, int &outer_top);

  void Raise_Window(Am_Drawonable *target_d); //to top
  void Lower_Window(Am_Drawonable *target_d); //to bottom
  void Iconify_Notify(bool new_iconified);
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
  Am_Drawonable *Get_Drawonable_At_Cursor();

  // general operations

  void Beep();

  // destination for bitblt is the Am_Drawonable this message is sent to
  void Bitblt(int d_left, int d_top, int width, int height,
              Am_Drawonable *source, int s_left, int s_top,
              Am_Draw_Function df = Am_DRAW_COPY);

  void Clear_Area(int left, int top, int width, int height);

  void Flush_Output();

  void Map_And_Wait(bool wait);
  void Unmap_And_Wait();

  void Translate_Coordinates(int src_x, int src_y, Am_Drawonable *src_d,
                             int &dest_x_return, int &dest_y_return);

  // Am_Image_Array operations:  must have an Am_Drawonable to get the size
  // of an image since it may need to be created
  void Get_Image_Size(const Am_Image_Array &image, int &ret_width,
                      int &ret_height);
  // verify that image is loaded (or loadable)
  bool Test_Image(const Am_Image_Array &image) const;

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
  //
  // Clipping region management (mostly inlined)
  //
  void Set_Clip(Am_Region *the_region); // This one actually talks to X11
  void Clear_Clip()
  {
    screen->clip_region->Clear();
    Set_Clip(screen->clip_region);
  }
  void Set_Clip(int l, int t, unsigned int w, unsigned int h)
  {
    screen->clip_region->Set(l, t, w, h);
    Set_Clip(screen->clip_region);
  }
  void Push_Clip(Am_Region *the_region)
  {
    screen->clip_region->Push(the_region);
    Set_Clip(screen->clip_region);
  }
  void Push_Clip(int l, int t, unsigned int w, unsigned int h)
  {
    screen->clip_region->Push(l, t, w, h);
    Set_Clip(screen->clip_region);
  }
  void Pop_Clip()
  {
    screen->clip_region->Pop();
    Set_Clip(screen->clip_region);
  }
  bool In_Clip(int x, int y) { return screen->clip_region->In(x, y); }
  bool In_Clip(int l, int t, unsigned int w, unsigned int h, bool &total)
  {
    return screen->clip_region->In(l, t, w, h, total);
  }
  bool In_Clip(Am_Region *the_region, bool &total)
  {
    return screen->clip_region->In(the_region, total);
  }
#if 0
 void Clear_Clip ();
 void Set_Clip (Am_Region *the_region);
 void Set_Clip (int the_left, int the_top,
		 unsigned int the_width, unsigned int the_height);
 void Push_Clip (Am_Region *the_region);
 void Push_Clip (int the_left, int the_top,
		  unsigned int the_width, unsigned int the_height);
 void Pop_Clip ();
  bool In_Clip (int x, int y);
  bool In_Clip (int left, int top, unsigned int width,
		unsigned int height, bool &total);
  bool In_Clip (Am_Region *rgn, bool &total);
#endif

  // Utility drawing functions -- declared as member functions because
  // they need to access data in the drawable
  void adjust_gcvalues_color_xor(XGCValues &values, unsigned long &mask,
                                 unsigned long fore_index) const;
  void adjust_gcvalues_color_drawfn(XGCValues &values, unsigned long &mask,
                                    unsigned long fore_index,
                                    Am_Draw_Function f) const;
  void adjust_gcvalues_stipple(XGCValues &values, unsigned long &mask,
                               Am_Fill_Solid_Flag fill_flag,
                               const Am_Image_Array &image) const;
  void set_gc_using_fill(const Am_Style &fill, Am_Draw_Function f,
                         Pixmap image = 0) const;
  void set_gc_using_line(const Am_Style &line, Am_Draw_Function f) const;
  void set_gc_using_font(const Am_Font &Am_font) const;

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
                    XPoint *ar, int num_points);
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

  //constructor function
  Am_Drawonable_Impl(int l, int t, unsigned int w, unsigned int h,
                     const char *tit, const char *icon_tit, bool vis,
                     bool initially_iconified, Am_Style back_color,
                     bool save_under_flag, int min_w, int min_h, int max_w,
                     int max_h, bool title_bar_flag, bool clip_by_children_flag,
                     unsigned int bit_depth, Am_Input_Event_Handlers *evh)

  {
    left = l;
    top = t;
    // avoid 0-sized windows
    width = ((w > 0) ? w : 1);
    height = ((h > 0) ? h : 1);
    title = new char[strlen(tit) + 1];
    strcpy(title, tit);
    icon_name = new char[strlen(icon_tit) + 1];
    strcpy(icon_name, icon_tit);
    visible = vis;
    iconified = initially_iconified;
    background_color = back_color;
    border_width = title_bar_flag ? 2 : 0;

    save_under = save_under_flag;

    // avoid 0-sized window
    min_width = ((min_w > 0) ? min_w : 1);
    min_height = ((min_h > 0) ? min_h : 1);

    max_width = max_w;
    max_height = max_h;
    title_bar = title_bar_flag;
    clip_by_children = clip_by_children_flag;
    depth = bit_depth;
    event_handlers = evh;
    // initialize title bar height member
    offscreen = false;
    ext_handler = nullptr;
    expect_map_change = false;
  }

  ~Am_Drawonable_Impl()
  {
    if (title)
      delete[] title;
    if (icon_name)
      delete[] icon_name;
  }

  // X-specific.  Get the closest match for a specified RGB,
  // returns X pixel value.
  void Allocate_Closest_Color(XColor &c) const;

  // to hack another application
  static Am_Drawonable *Create_Drawonable_From_XWindow(
      Window created_drawable, Display *created_display,
      Am_External_Event_Handler *ext_ev_handler, const char *tit = "",
      const char *icon_tit = "", bool vis = true,
      bool initially_iconified = false, Am_Style back_color = Am_No_Style,
      bool save_under_flag = false,
      int min_w = 1, // can't have 0 size windows
      int min_h = 1,
      int max_w = 0, // 0 is illegal so means no max
      int max_h = 0, bool title_bar_flag = true,
      bool query_user_for_position = false, bool query_user_for_size = false,
      bool clip_by_children_flag = true, Am_Input_Event_Handlers *evh = nullptr);
  // to hack another application
  static Am_Drawonable *Create_Offscreen_Drawonable_From_XWindow(
      Window created_drawable, Display *created_display,
      Am_External_Event_Handler *ext_ev_handler,
      Am_Style back_color = Am_No_Style);

  void iconify_notify(bool new_iconified) { iconified = new_iconified; }

  void Print(std::ostream &os) const;

  //data
  // variable to hold the event handlers for this window
  Am_Input_Event_Handlers *event_handlers;
  Screen_Desc *screen; //other Xlib information

  //extra data
  Am_Drawonable *owner; //Am_Drawonable this is nested in
  Window xlib_drawable;
  static Am_ID_Tag id;

  bool offscreen; // offscreen pixmap?

  // flags for input handling
  bool want_enter_leave;
  bool want_multi_window;
  bool want_move;
  long current_event_mask;

  Am_External_Event_Handler *ext_handler;

  // internal utility functions
  void set_drawable_event_mask();
  void reconfigure(int new_left, int new_top, int new_width, int new_height)
  {
    left = new_left;
    top = new_top;
    width = new_width;
    height = new_height;
  }

  // map change flag
  // used to distinguish internally generated map/unmap requests
  // from user-generated iconify/deiconify requests
  bool expect_map_change;
};

// Given an XLIB window, returns the corresponding drawonable object
extern Am_Drawonable_Impl *Get_Drawable_Backpointer(Display *dpy,
                                                    Window xlib_window);
extern void Set_Drawable_Backpointer(Display *dpy, Window xlib_window,
                                     Am_Drawonable_Impl *d);

// Font_Index is used in Am_Font to store a list of fonts on displays other
// than the main display.  Get_X_Font first checks disp to see if it's the
// main display, and optimizes that common case.  Get_Font is used after
// that for fonts on other displays. (See: Am_Font_Impl::{Add_Font,Get_Font})

class Font_Index
{
public:
  Font_Index(Display *disp, XFontStruct *in_xfont)
      : dpy(disp), xfont(in_xfont), next(nullptr)
  {
  }
  Display *dpy;
  XFontStruct *xfont;
  Font_Index *next;
};

class Am_Font_Data : public Am_Wrapper
{
  friend class Am_Font;
  AM_WRAPPER_DATA_DECL(Am_Font);

public:
  void Print(std::ostream &os) const override;
  Am_Font_Data(Am_Font_Data *proto);
  Am_Font_Data(const char *the_name);
  Am_Font_Data(Am_Font_Family_Flag f = Am_FONT_FIXED, bool is_bold = false,
               bool is_italic = false, bool is_underline = false,
               Am_Font_Size_Flag s = Am_FONT_MEDIUM);

  ~Am_Font_Data();

  bool operator==(const Am_Font_Data &) const { return false; }

  void Add_Font(Display *, XFontStruct *);
  bool Get_Font(Display *, XFontStruct *&);
  XFontStruct *Get_X_Font(const Am_Drawonable_Impl *d);
  const char *Get_Font_Name() const;
  //returns a pointer to the string, don't dealloc

  // get values
  void Get_Values(Am_Font_Family_Flag &f, bool &is_bold, bool &is_italic,
                  bool &is_underline, Am_Font_Size_Flag &s) const;

  int Char_Width(Display *d);
  bool Underline();

  static void remove(Am_Font_Data *font);
  static void remove(Display *display);

  // data
protected:
  XFontStruct *main_xfont;
  Display *main_display;
  Font_Index *font_head;
  Am_Font_Family_Flag family;

  bool bold;
  bool italic;
  bool underline;
  Am_Font_Size_Flag size;
  char *name;

  int char_width; //??? keep it?

  Am_Font_Data *next;
  static Am_Font_Data *list;
};

class Color_Index
{
public:
  Color_Index(Display *disp, XColor idx) : dpy(disp), index(idx), next(nullptr) {}
  Display *dpy;
  // TODO: Remove that: unsigned long index;
  XColor index;
  Color_Index *next;
};

class Cursor_Item
{
public:
  Cursor_Item(Display *disp, Cursor in_cursor)
      : display(disp), cursor(in_cursor), next(nullptr)
  {
  }
  Display *display;
  Cursor cursor;
  Cursor_Item *next;
};

class Am_Cursor_Data : public Am_Wrapper
{
  friend class Am_Cursor;
  AM_WRAPPER_DATA_DECL(Am_Cursor);

public:
  // creators
  Am_Cursor_Data(Am_Cursor_Data *proto);
  Am_Cursor_Data();
  Am_Cursor_Data(Am_Image_Array image, Am_Image_Array mask, Am_Style fg_color,
                 Am_Style bg_color);
  ~Am_Cursor_Data();

  Cursor Get_X_Cursor(Am_Drawonable_Impl *draw);
  void Add_Cursor(Display *, Cursor);
  bool Get_Cursor(Display *, Cursor &);

  // I think I need this for a wrapper class
  bool operator==(const Am_Cursor_Data &) const { return false; }

  Display *main_display;
  Cursor main_cursor;

  static void remove(Am_Cursor_Data *cursor);
  static void remove(Display *display);

protected:
  /* holds data till we need it */
  Am_Image_Array image;
  Am_Image_Array mask;
  Am_Style fg_color;
  Am_Style bg_color;

  Cursor_Item *head;

  int x_hot;
  int y_hot;

  Am_Cursor_Data *next;
  static Am_Cursor_Data *list;
};

class Bitmap_Item
{
public:
  Bitmap_Item(Display *disp, Pixmap in_bitmap, XColor *in_col, int n_col)
      : display(disp), bitmap(in_bitmap), mask(0), inverted_mask(0),
        colors(in_col), num_colors(n_col), next(nullptr)
  {
  }
  ~Bitmap_Item()
  {
    if (display) {
      if (bitmap)
        XFreePixmap(display, bitmap);
      if (mask)
        XFreePixmap(display, mask);
      if (inverted_mask)
        XFreePixmap(display, inverted_mask);
    }
    if (colors) {
      int screen_num = DefaultScreen(display);
      Colormap c = XDefaultColormap(display, screen_num);
      for (int i = 0; i < num_colors; i++)
        XFreeColors(display, c, &(colors[i].pixel), 1, 0);
      delete[] colors;
      colors = nullptr;
    }
  }
  Display *display;
  Pixmap bitmap;
  Pixmap mask;
  Pixmap inverted_mask;
  // keep track of colors we use, so we can free them.
  XColor *colors;
  int num_colors;
  Bitmap_Item *next;
};

class Am_Image_Array_Data : public Am_Wrapper
{
  friend class Am_Image_Array;
  AM_WRAPPER_DATA_DECL(Am_Image_Array);

public:
  Am_Image_Array_Data();
  Am_Image_Array_Data(Am_Image_Array_Data *proto);
  Am_Image_Array_Data(const char *file_name);
  Am_Image_Array_Data(const char *bit_data, int height, int width);

  ~Am_Image_Array_Data();

  bool operator==(const Am_Image_Array_Data &) const { return false; }

  Pixmap Get_X_Pixmap(const Am_Drawonable_Impl *draw);
  Pixmap Safe_Get_X_Pixmap(const Am_Drawonable_Impl *draw);
  Pixmap Get_X_Mask(const Am_Drawonable_Impl *draw, bool invert = false);

  // Return platform independent image in RGB form
  bool Get_RGB_Image(unsigned char *storage, bool top_first = true);

  // notice this isn't necessarily correct: it returns the depth of the
  // machine-independent pixmap, not the depth of the X pixmap we're using.
  inline int Get_Depth() { return ((image_) ? image_->Get_Depth() : 0); }
  //TODO: Remove! if (image_) return image_->Get_Depth(); else return 0;}
  inline bool Is_Transparent()
  {
    return ((image_) ? image_->Is_Transparent() : false);
  }
  //TODO: Remove!  {if (image_) return image_->Is_Transparent(); else return false;}
  void Get_Hot_Spot(int &x, int &y) const
  {
    x = x_hot;
    y = y_hot;
  }

  // these should be protected but set_cursor needs to access them.
  // actually Cursor shouldn't be here at all.
  Cursor cursor;
  Pixmap main_bitmap;
  Pixmap main_mask;
  Pixmap main_inverted_mask;

  static void remove(Am_Image_Array_Data *image);
  static void remove(Display *display);

protected:
  Pixmap Make_Pixmap_From_Generic_Image(const Am_Drawonable_Impl *, XColor *&,
                                        int &);
  Pixmap Make_Mask_From_Generic_Image(const Am_Drawonable_Impl *, bool invert);
  void Add_Bitmap(Display *, Pixmap, XColor *, int);
  bool Get_Bitmap(Display *, Pixmap &);

  // utility routines
  bool is_gif();
  bool make_generic_image_from_name();

  // holds the hot spot locally for bitmaps, in case we make a cursor.
  int x_hot, y_hot;

  // Am_Generic_Image stores machine-independent data.
  // we need only one copy of the machine-independent data.
  // XImage stores the machine specific Image data.
  // we shouldn't need to store this at all, assuming we can get it into an
  // XPixmap successfully.

  Am_Generic_Image *image_;
  //  XImage *image;

  // store the XColors we allocated for the gif, so we can destroy them
  // later.
  int num_colors;
  XColor *colors;

  /* holds the name of the bitmap till we need it */
  char *name;

  Display *main_display;
  /* pointer to the pixmap that the X server is storing */
  Bitmap_Item *head;

  Am_Image_Array_Data *next;
  static Am_Image_Array_Data *list;
};

class Am_Style_Data : public Am_Wrapper
{
  friend class Am_Style;
  AM_WRAPPER_DATA_DECL(Am_Style);

public:
  void Print(std::ostream &os) const override;

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

  Am_Style_Data();
  ~Am_Style_Data();

  bool operator==(const Am_Style_Data &) const { return false; }

  void Add_Color_Index(Display *, XColor);
  bool Get_Color_Index(Display *, XColor &);

  const char *Get_Color_Name();
  //returns a pointer to the string, don't dealloc

  //gets the index to use on display
  unsigned long Get_X_Index(const Am_Drawonable_Impl *d);
  XColor Get_X_Color(const Am_Drawonable_Impl *d);

  static void remove(Am_Style_Data *style);
  static void remove(Display *display);

protected:
  // data
  const char *color_name;
  Display *main_display;
  //  unsigned long main_color_index;
  XColor main_color;
  Color_Index *color_head;
  bool color_allocated;

  //color part
  float red, green, blue;

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

  Am_Style_Data *next;
  static Am_Style_Data *list;
};

extern Am_Style_Data Am_On_Bits_Data;
extern Am_Style_Data Am_Off_Bits_Data;

/////////////////////////
// Am_Point_Array_Data //
/////////////////////////
#include <vector>
class Am_Point_Array_Data : public Am_Wrapper
{
  friend class Am_Point_Array;
  // for direct access to XPoint representation
  friend class Am_Drawonable_Impl;
  AM_WRAPPER_DATA_DECL(Am_Point_Array);

public:
  Am_Point_Array_Data(Am_Point_Array_Data *proto);
  Am_Point_Array_Data(Am_Point_List pl, int offset_x, int offset_y);
  Am_Point_Array_Data(int *coords, int num_coords);
  Am_Point_Array_Data(int size);
  ~Am_Point_Array_Data();

  bool operator==(const Am_Point_Array_Data &) const { return false; }

protected:
  typedef std::vector<XPoint> vector_of_XPoints;
  vector_of_XPoints points;
#if 0
  XPoint *ar;
  int size;
#endif
};

////////////////////
// Am_Region_Impl //
////////////////////
#include <stack>
class Am_Region_Impl : public Am_Region
{
  friend class Am_Drawonable_Impl; // Needs to access 'region_to_use()'
  friend class Am_Region;          // Needs to access (private) constructor
public:
  // creator function is via Am_Region::Create
  void Clear();
  void Destroy();
  void Intersect(int left, int top, unsigned int width, unsigned int height);
  bool In(int x, int y);
  bool In(int left, int top, unsigned int width, unsigned int height,
          bool &total);
  bool In(Am_Region *rgn, bool &total);
  void Set(int left, int top, unsigned int width, unsigned int height);
#ifdef POLYGONAL_REGIONS
  void Set(Am_Point_List points);
#endif
  void Push(Am_Region *region);
  void Push(int left, int top, unsigned int width, unsigned int height);
  void Pop();
  void Union(int left, int top, unsigned int width, unsigned int height);

private:
  // Data members
  std::stack<Region> stack;
  // Internal functions
  Am_Region_Impl() {}
  // This one is used by class Am_Drawonable_Impl sometimes
  Region region_to_use()
  {
    return ((stack.empty() == true) ? nullptr : stack.top());
  }
};

//-------------
// Inter stuff
//-------------

// Defined in gemX_inter.cc, called by Am_Drawonable::Create, and
// Am_Drawonable_Impl::Map_And_Wait
void wait_for_mapnotify(Display *dpy, Window *xlib_window);

// process an input event.  This can be called if another application
// wants to also see the input events.  This does not deal with
// removing the event from the event queue.
extern void Am_Handle_Event_Received(XEvent &event_return);

#endif
