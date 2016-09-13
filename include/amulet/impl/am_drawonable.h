#ifndef GEM_DRAWONABLE_H
#define GEM_DRAWONABLE_H

// class std::ostream;

#include "gem_flags.h"
#include "am_style.h"
#include "am_cursor.h"

class Am_Input_Event_Handlers;
class Am_Region;
class Am_Time;
class Am_Image_Array;
class Am_Point_List;
class Am_Point_Array;
class Am_Cursor;
class Am_Font;

class _OA_DL_CLASSIMPORT Am_Drawonable
{
public:
  //:Main loop procedure which dandles all input
  // Call this after set up.  Hopefully, will be replaced by threads.
  static void Main_Loop();
  static void Process_Event(const Am_Time &timeout);
  static void Process_Immediate_Event();
  static void Wait_For_Event();

  // Narrow function.  Allows easier access for drawonables stored in an object slot.
  static Am_Drawonable *Narrow(Am_Ptr ptr)
  {
    return (reinterpret_cast<Am_Drawonable *>(ptr));
  }

  // Get the root Am_Drawonable.  This is a static member function since
  // we don't necessarily have an object to start with.  String parameter
  // used under X to specify a screen.  This also causes any required
  // initialization to happen.  It is OK to call this more than once.
  static Am_Drawonable *Get_Root_Drawonable(const char *screen = (0L));

  // the parent is the Am_Drawonable this is sent to
  virtual Am_Drawonable *
  Create(int l = 0, int t = 0, unsigned int w = 100, unsigned int h = 100,
         const char *tit = "", const char *icon_tit = "", bool vis = true,
         bool initially_iconified = false, Am_Style back_color = Am_No_Style,
         bool save_under_flag = false,
         int min_w = 1, // can't have windows with width 0
         int min_h = 1, // can't have windows with height 0
         int max_w = 0, // 0 is illegal so means no max
         int max_h = 0, bool title_bar_flag = true,
         bool query_user_for_position = false, bool query_user_for_size = false,
         bool clip_by_children_flag = true,
         Am_Input_Event_Handlers *evh = (0L)) = 0;

  // the Am_Drawonable this method is invoked on is the one being destroyed
  virtual void Destroy() = 0;

  virtual Am_Drawonable *
  Create_Offscreen(int width = 0, int height = 0,
                   Am_Style background_color = Am_No_Style) = 0;
  virtual void Reparent(Am_Drawonable *new_parent) = 0;

  virtual bool Inquire_Window_Borders(int &left_border, int &top_border,
                                      int &right_border, int &bottom_border,
                                      int &outer_left, int &outer_top) = 0;
  virtual void Raise_Window(Am_Drawonable *target_d) = 0; //to top
  virtual void Lower_Window(Am_Drawonable *target_d) = 0; //to bottom
  virtual void Set_Iconify(bool iconified) = 0;
  virtual void Set_Title(const char *new_title) = 0;
  virtual void Set_Icon_Title(const char *new_title) = 0;

  // for translate_coordinates to work properly, it is expected that
  // the drawonable's left and top are set by call to Set_Position
  // regardless of the state of the drawonable (onscreen, window ...)
  virtual void Set_Position(int new_left, int new_top) = 0;
  virtual void Set_Size(unsigned int new_width, unsigned int new_height) = 0;
  virtual void Set_Max_Size(unsigned int new_width,
                            unsigned int new_height) = 0;
  virtual void Set_Min_Size(unsigned int new_width,
                            unsigned int new_height) = 0;
  virtual void Set_Visible(bool vis) = 0;
  virtual void Set_Titlebar(bool new_title_bar) = 0;
  virtual void Set_Background_Color(Am_Style new_color) = 0;

  bool Get_Iconify() const { return (iconified); }
  const char *Get_Title() const { return (title); }
  const char *Get_Icon_Title() const { return icon_name; }
  void Get_Position(int &l, int &t) const
  {
    l = left;
    t = top;
  }
  void Get_Size(int &w, int &h) const
  {
    w = width;
    h = height;
  }
  void Get_Max_Size(int &w, int &h) const
  {
    w = max_width;
    h = max_height;
  }
  void Get_Min_Size(int &w, int &h) const
  {
    w = min_width;
    h = min_height;
  }
  bool Get_Visible() const { return visible; }
  void Get_Titlebar(bool &title_bar_flag) const { title_bar_flag = title_bar; }
  Am_Style &Get_Background_Color() { return background_color; }

  int Get_Depth() const { return depth; }
  bool Is_Color() const { return (depth != 1); }

  void Get_Values(int &l, int &t, int &w, int &h, const char *&tit,
                  const char *&icon_tit, bool &vis, bool &iconified_now,
                  Am_Style &back_color, bool &save_under_flag, int &min_w,
                  int &min_h, int &max_w, int &max_h, bool &title_bar_flag,
                  bool &clip_by_children_flag, int &bit_depth)
  {
    l = left;
    t = top;
    w = width;
    h = height;
    tit = title;
    icon_tit = icon_name;
    vis = visible;
    iconified_now = iconified;
    back_color = background_color;
    save_under_flag = save_under;
    min_w = min_width;
    min_h = min_height;
    max_w = max_width;
    max_h = max_height;
    title_bar_flag = title_bar;
    clip_by_children_flag = clip_by_children;
    bit_depth = depth;
  }

  // general operations

  virtual void Beep() = 0;

  // destination for bitblt is the Am_Drawonable this message is sent to
  virtual void Bitblt(int d_left, int d_top, int width, int height,
                      Am_Drawonable *source, int s_left, int s_top,
                      Am_Draw_Function df = Am_DRAW_COPY) = 0;

  virtual void Clear_Area(int left, int top, int width, int height) = 0;

  virtual void Flush_Output() = 0;

  // Note: to translate from screen coordinates, pass your root drawonable
  // as src_d.
  virtual void Translate_Coordinates(int src_x, int src_y, Am_Drawonable *src_d,
                                     int &dest_x_return,
                                     int &dest_y_return) = 0;

  // Am_Image_Array operations:  must have an Am_Drawonable to get the size
  // of an image since it may need to be created
  virtual void Get_Image_Size(const Am_Image_Array &image, int &ret_width,
                              int &ret_height) = 0;
  // verify that image is loaded (or loadable)
  virtual bool Test_Image(const Am_Image_Array &image) const = 0;

  // Am_Font operations, must have a Am_Drawonable and a Am_Font to get sizes

  virtual int Get_Char_Width(const Am_Font &Am_font, char c) = 0;
  virtual int Get_String_Width(const Am_Font &Am_font, const char *the_string,
                               int the_string_length) = 0;
  // The total height of the bounding rectangle for this string, or any
  // string in this font, is ascent + descent.  The left_bearing is the
  // distance from the origin of the text to the first "inked" pixel.
  // The right_bearing is the distance from the origin of the text to
  // the last "inked" pixel.
  virtual void Get_String_Extents(const Am_Font &Am_font,
                                  const char *the_string, int the_string_length,
                                  int &width, int &ascent, int &descent,
                                  int &left_bearing, int &right_bearing) = 0;

  // The max ascent and descent include vertical spacing between rows
  // of text.   The min ascent and descent are computed on a per-char basis.
  virtual void Get_Font_Properties(const Am_Font &Am_font, int &max_char_width,
                                   int &min_char_width, int &max_char_ascent,
                                   int &max_char_descent) = 0;

  // Am_Polygon operations
  // polygon bounding box calculation
  virtual void Get_Polygon_Bounding_Box(const Am_Point_List &pl,
                                        const Am_Style &ls, int &out_left,
                                        int &out_top, int &width,
                                        int &height) = 0;

  // Clipping functions:
  // Note that there is only one GC for the screen, and there is only one
  // clip mask that is shared by all windows on that screen.  So as you set
  // the clip mask for a window, you are actually setting it for all windows
  // on the same screen as that window.
  virtual void Clear_Clip() = 0;
  virtual void Set_Clip(Am_Region *region) = 0;
  virtual void Set_Clip(int left, int top, unsigned int width,
                        unsigned int height) = 0;
  virtual void Push_Clip(Am_Region *region) = 0;
  virtual void Push_Clip(int left, int top, unsigned int width,
                         unsigned int height) = 0;
  virtual void Pop_Clip() = 0;

  virtual bool In_Clip(int x, int y) = 0;
  virtual bool In_Clip(int left, int top, unsigned int width,
                       unsigned int height, bool &total) = 0;
  virtual bool In_Clip(Am_Region *rgn, bool &total) = 0;

  // Specific Drawing functions

  virtual void Draw_Arc(const Am_Style &ls, const Am_Style &fs, int left,
                        int top, unsigned int width, unsigned int height,
                        int angle1 = 0, int angle2 = 360,
                        Am_Draw_Function f = Am_DRAW_COPY,
                        Am_Arc_Style_Flag asf = Am_ARC_PIE_SLICE) = 0;
  // if draw_monochrome is true,
  //   fs is used for background pixels on transparent images
  //      or for white pixels on opaque images
  //   thus if fs is Am_No_Style, opaque images will be drawn as transparent
  //   ls is used for all non-background pixels on transparent images
  //      or for non-white pixels on opaque images
  //   if ls is Am_No_Style, foreground will be transparent
  virtual void
  Draw_Image(int left, int top, int width, int height,
             const Am_Image_Array &image, int i_left = 0, int i_top = 0,
             const Am_Style &ls = Am_No_Style, //color of 'on' bits
             const Am_Style &fs = Am_No_Style, //for background behind image
             bool draw_monochrome = false, //use ls and fs in place of colors
             Am_Draw_Function f = Am_DRAW_COPY) = 0;
  virtual void Draw_Line(const Am_Style &ls, int x1, int y1, int x2, int y2,
                         Am_Draw_Function f = Am_DRAW_COPY) = 0;
  virtual void Draw_Lines(const Am_Style &ls, const Am_Style &fs,
                          const Am_Point_Array &pts,
                          Am_Draw_Function f = Am_DRAW_COPY) = 0;
  virtual void Draw_2_Lines(const Am_Style &ls, const Am_Style &fs, int x1,
                            int y1, int x2, int y2, int x3, int y3,
                            Am_Draw_Function f = Am_DRAW_COPY) = 0;
  virtual void Draw_3_Lines(const Am_Style &ls, const Am_Style &fs, int x1,
                            int y1, int x2, int y2, int x3, int y3, int x4,
                            int y4, Am_Draw_Function f = Am_DRAW_COPY) = 0;
  virtual void Draw_Rectangle(const Am_Style &ls, const Am_Style &fs, int left,
                              int top, int width, int height,
                              Am_Draw_Function f = Am_DRAW_COPY) = 0;
  virtual void Draw_Roundtangle(const Am_Style &ls, const Am_Style &fs,
                                int left, int top, int width, int height,
                                unsigned short x_radius,
                                unsigned short y_radius,
                                Am_Draw_Function f = Am_DRAW_COPY) = 0;
  virtual void
  Draw_Text(const Am_Style &ls, const char *s, int str_len,
            const Am_Font &Am_font, int left, int top,
            Am_Draw_Function f = Am_DRAW_COPY,
            const Am_Style &fs = Am_No_Style, // background fill style
            bool invert = false) = 0;

  // cut buffer, eventually, this needs to be generalized
  virtual void Set_Cut_Buffer(const char *s) = 0;
  virtual char *Get_Cut_Buffer() = 0;

  // input handling

  virtual void Set_Enter_Leave(bool want_enter_leave_events) = 0;
  virtual void Set_Want_Move(bool want_move_events) = 0;
  virtual void Set_Multi_Window(bool want_multi_window) = 0;

  virtual void Get_Window_Mask(bool &want_enter_leave_events,
                               bool &want_move_events,
                               bool &want_multi_window) = 0;

  virtual void Discard_Pending_Events() = 0;
  virtual void Set_Input_Dispatch_Functions(Am_Input_Event_Handlers *evh) = 0;
  virtual void Get_Input_Dispatch_Functions(Am_Input_Event_Handlers *&evh) = 0;

  void Set_Data_Store(void *data) { data_store = data; }
  void *Get_Data_Store() const { return data_store; }

  // cursors
  virtual void Set_Cursor(Am_Cursor new_cursor) = 0;
  virtual Am_Cursor &Get_Cursor() { return cursor; }

  //Find the child-most drawonable at the current cursor position
  virtual Am_Drawonable *Get_Drawonable_At_Cursor() = 0;

  virtual void Print(std::ostream &os) const = 0;

  virtual ~Am_Drawonable(){};

protected:
  //data
  int left;
  int top;
  unsigned int width;
  unsigned int height;
  char *title;
  char *icon_name;
  bool visible;
  bool iconified;
  Am_Style background_color;
  unsigned int border_width;
  bool save_under;
  unsigned int min_width;
  unsigned int min_height;
  unsigned int max_width;
  unsigned int max_height;
  bool title_bar;
  bool clip_by_children;
  int depth;
  void *data_store;
  Am_Cursor cursor;
}; // end Am_Drawonable

// operator<< defined in gem_keytrans.cc since platform-independent
_OA_DL_IMPORT std::ostream &operator<<(std::ostream &os, Am_Drawonable *d);

#endif //GEM_DRAWONABLE_H
