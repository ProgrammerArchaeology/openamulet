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

#ifndef GEMG_H
#define GEMG_H

#include <ggi/gwt.h>

#ifdef NEED_STDLIB
#include <stdlib.h>
#endif
#include <string.h>

#include <sys/types.h>
#include <sys/time.h>

#ifdef NEED_BSTRING
#include <bstring.h>
#endif

#ifdef NEED_SELECT
#include <sys/select.h>
#endif

// OpenVMS, fd_set definition either comes from socket.h or time.h
// The use of fd_set below suggests that the former is intended,
// to go along with select()
#ifdef NEED_SOCKET
#include <socket.h>
#endif

#include <am_inc.h>

#include TYPES__H  // to get the Value object
#include GDEFS__H  // constants and definitions for graphics
#include IDEFS__H  // constants and definitions for input
#include GEM__H
#include GEM_IMAGE__H

// // // // // // // // // // // // // // // // // // // // // // // // // //
// Utility functions and definitions
// // // // // // // // // // // // // // // // // // // // // // // // // //

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

#if 0
extern unsigned long Am_GC_setmask;
extern unsigned long Am_GC_getmask;
#endif

// the following fails on NCD X terminals (no matching font)
// #define Am_DEFAULT_FONT_NAME "*-*-fixed-medium-r-*-*-12-*-*-*-*-*-iso8859-1"
#define Am_DEFAULT_FONT_NAME "*-*-fixed-medium-r-*--*-120-*-*-*-*-iso8859-1"

#define Am_DEFAULT_FONT_NAME_C "*-*-*-medium-r-*-*-16-*-*-*-*-*-gb2312.1980-0"
#define Am_DEFAULT_FONT_NAME_J "*-*-*-medium-r-*-*-16-*-*-*-*-*-jisx0208.1983-0"
#define Am_DEFAULT_FONT_NAME_K "*-*-*-medium-r-*-*-16-*-*-*-*-*-ksc5601.1987-0"

// Font defaults for X
#define Am_FIXED_FONT_FAMILY      "courier"
#define Am_SERIF_FONT_FAMILY      "times"
#define Am_SANS_SERIF_FONT_FAMILY "helvetica"
#define Am_TWO_BYTE_FONT_FAMILY    "*"
// Japanese, Chinese, and Korean font family

#define Am_SMALL_PIXEL_SIZE      10
#define Am_MEDIUM_PIXEL_SIZE     12
#define Am_LARGE_PIXEL_SIZE      18
#define Am_VERY_LARGE_PIXEL_SIZE 24
#define Am_TSMALL_PIXEL_SIZE      8
#define Am_TMEDIUM_PIXEL_SIZE     14
#define Am_TLARGE_PIXEL_SIZE      16
#define Am_TVERY_LARGE_PIXEL_SIZE 24
// 'Am_T*' are sizes added for Two-byte code fonts.
// But we have only a few good fonts on X for two-byte code now.
// In Japanese, for example, the sizes of good fonts are 14, 16, and 24.
// You may be able to use only one font whose size is 16 or 24
// for Chinese and Korean.
// So, I determined as above. In future, they should be changed.
//    Jan.30, 1997            Y.Tsujino
// TODO: Check again one day -- ortalo, 1999

//
// Global variables
//
// Main GGIGEM Root Drawonable
extern Am_Drawonable *GGIGEM_Root_Drawonable;
// For convenience: also GWT root window
extern gwt_window_t GWT_Root_Window;

/////////////////
// GGIGEM_Point
/////////////////
// Used to draw poly-lines
class GGIGEM_Point
{
 public:
  GGIGEM_Point () : x(0),y(0) { }
  GGIGEM_Point (int in_x, int in_y) : x(in_x), y(in_y) { }
  GGIGEM_Point (GGIGEM_Point &o) : x(o.x), y(o.y) { }
  ~GGIGEM_Point () { }
  int x;
  int y;
};

// // // // // // // // // // // // // // // // // // // // // // // // // //
//
//   Am_Drawonable_Impl
//
// // // // // // // // // // // // // // // // // // // // // // // // // //

class Am_Drawonable_Impl : public Am_Drawonable
{
 public:
  Am_Drawonable* Create (int l = 0, int t = 0,
			 unsigned int w = 100, unsigned int h = 100,
			 const char* tit = "",
			 const char* icon_tit = "",
			 bool vis = true,
			 bool initially_iconified = false,
			 Am_Style back_color = Am_No_Style,
			 bool save_under_flag = false,
			 int min_w = 1, // can't have 0 size windows
			 int min_h = 1,
			 int max_w = 0, // 0 is illegal so means no max
			 int max_h = 0,
			 bool title_bar_flag = true,
			 bool query_user_for_position = false,
			 bool query_user_for_size = false,
			 bool clip_by_children_flag = true,
			 Am_Input_Event_Handlers *ev = (0L));
  void Destroy ();

  Am_Drawonable* Create_Offscreen (int width = 1, int height = 1,
				   Am_Style background_color = Am_No_Style);

  void Reparent (Am_Drawonable *new_parent);
  bool Inquire_Window_Borders(int& left_border, int& top_border,
			      int& right_border, int& bottom_border,
			      int& outer_left, int& outer_top);

  void Raise_Window (Am_Drawonable *target_d);  //to top
  void Lower_Window (Am_Drawonable *target_d);  //to bottom
  void Iconify_Notify (bool new_iconified);
  void Set_Iconify (bool iconified);
  void Set_Title (const char* new_title);
  void Set_Icon_Title (const char* new_title);
  void Set_Position(int new_left, int new_top);
  void Set_Size(unsigned int new_width, unsigned int new_height);
  void Set_Max_Size(unsigned int new_width, unsigned int new_height);
  void Set_Min_Size(unsigned int new_width, unsigned int new_height);
  void Set_Visible(bool vis);
  void Set_Titlebar(bool new_title_bar);
  void Set_Cursor(Am_Cursor cursor);
  void Set_Background_Color(Am_Style new_color);
  Am_Drawonable* Get_Drawonable_At_Cursor();

  // general operations
  void Beep ();

  // destination for bitblt is the Am_Drawonable this message is sent to
  void Bitblt(int d_left, int d_top, int width, int height,
	      Am_Drawonable* source, int s_left, int s_top,
	      Am_Draw_Function df = Am_DRAW_COPY);

  void Clear_Area(int left, int top, int width, int height);

  void Flush_Output ();

  void Translate_Coordinates(int src_x, int src_y, Am_Drawonable *src_d,
			     int& dest_x_return, int& dest_y_return);

  // Am_Image_Array operations:  must have an Am_Drawonable to get the size
  // of an image since it may need to be created
  void Get_Image_Size (const Am_Image_Array& image, int& ret_width, int& ret_height);
  // verify that image is loaded (or loadable)
  bool Test_Image (const Am_Image_Array& image) const;

  // Am_Font operations, must have a Am_Drawonable and a Am_Font to get sizes
  int Get_Char_Width(const Am_Font& Am_font, char c);
  int Get_String_Width(const Am_Font& Am_font, const char* the_string,
		       int the_string_length);
  void Get_String_Extents(const Am_Font& Am_font, const char* the_string,
			  int the_string_length, int& width,
			  int& ascent, int& descent,
			  int& left_bearing, int& right_bearing);

  void Get_Font_Properties (const Am_Font& Am_font,
			    int& max_char_width, int& min_char_width,
			    int& char_ascent, int& char_descent);

  // Am_Polygon operations
  // polygon bounding box calculation
  void Get_Polygon_Bounding_Box (const Am_Point_List& pl, const Am_Style& ls,
				 int& out_left, int& out_top,
				 int& width, int& height);
  //
  // Clip region operations
  //
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

  // Specific Drawing functions
  void Draw_Arc (const Am_Style& ls, const Am_Style& fs,
		 int left, int top,
		 unsigned int width, unsigned int height,
		 int angle1 = 0, int angle2 = 360,
		 Am_Draw_Function f = Am_DRAW_COPY,
		 Am_Arc_Style_Flag asf = Am_ARC_PIE_SLICE );
  void Draw_Image (int left, int top, int width, int height,
		   const Am_Image_Array& image,
		   int i_left = 0, int i_top = 0,
		   const Am_Style& ls = Am_No_Style, //color of 'on' bits
		   const Am_Style& fs = Am_No_Style, //for background behind image
		   bool draw_monochrome = false,     //use ls and fs in place of colors
		   Am_Draw_Function f = Am_DRAW_COPY );
  void Draw_Line (const Am_Style& ls,
		  int x1, int y1, int x2, int y2,
		  Am_Draw_Function f = Am_DRAW_COPY);
  void Draw_Lines (const Am_Style& ls,
		   const Am_Style& fs,
		   const Am_Point_Array& pts,
		   Am_Draw_Function f = Am_DRAW_COPY);
  void Draw_2_Lines (const Am_Style& ls, const Am_Style& fs,
		     int x1, int y1, int x2, int y2, int x3, int y3,
		     Am_Draw_Function f = Am_DRAW_COPY);
  void Draw_3_Lines (const Am_Style& ls, const Am_Style& fs,
		     int x1, int y1, int x2, int y2, int x3, int y3,
		     int x4, int y4,
		     Am_Draw_Function f = Am_DRAW_COPY);
  void Draw_Rectangle (const Am_Style& ls, const Am_Style& fs,
		       int left, int top, int width, int height,
		       Am_Draw_Function f = Am_DRAW_COPY );
  void Draw_Roundtangle (const Am_Style& ls, const Am_Style& fs,
			 int left, int top, int width, int height,
			 unsigned short x_radius,
			 unsigned short y_radius,
			 Am_Draw_Function f = Am_DRAW_COPY );
  void Draw_Text (const Am_Style& ls, const char *s, int str_len,
		  const Am_Font& Am_font,
		  int left, int top,
		  Am_Draw_Function f = Am_DRAW_COPY,
		  const Am_Style& fs = Am_No_Style, // background fill style
		  bool invert = false);

  // cut buffer, eventually, this needs to be generalized
  void Set_Cut_Buffer(const char *s);
  char* Get_Cut_Buffer();

  // input handling

  void Set_Enter_Leave (bool want_enter_leave_events);
  void Set_Want_Move (bool want_move_events);
  void Set_Multi_Window (bool want_multi_window);

  void Get_Window_Mask (bool& want_enter_leave_events,
			bool& want_move_events,
			bool& want_multi)
    {
      want_enter_leave_events = want_enter_leave;
      want_move_events = want_move;
      want_multi = want_multi_window;
    }

  void Discard_Pending_Events();

  void Set_Input_Dispatch_Functions (Am_Input_Event_Handlers* evh);
  void Get_Input_Dispatch_Functions (Am_Input_Event_Handlers*& evh);

  //constructor function
  Am_Drawonable_Impl (int l, int t, unsigned int w, unsigned int h,
		      const char* tit, const char* icon_tit, bool vis,
		      bool initially_iconified,
		      Am_Style back_color,
		      bool save_under_flag,
		      int min_w, int min_h, int max_w, int max_h,
		      bool title_bar_flag,
		      bool clip_by_children_flag,
		      unsigned int bit_depth,
		      Am_Input_Event_Handlers *evh)
    : is_root(false), clip_region(0L), gwt_window(0L)
    {
      left = l; top = t;
      width = ((w>0) ? w : 1); height = ((h>0) ? h :1);
      title = new char[strlen(tit)+1]; strcpy (title, tit);
      icon_name = new char[strlen(icon_tit)+1]; strcpy (icon_name, icon_tit);
      visible = vis; iconified = initially_iconified;
      background_color = back_color; border_width = title_bar_flag?2:0;
      save_under = save_under_flag;
      min_width = ((min_w>0) ? min_w : 1);
      min_height = ((min_h>0) ? min_h : 1);
      max_width = max_w; max_height = max_h;
      title_bar = title_bar_flag;
      clip_by_children = clip_by_children_flag;  depth = bit_depth;
      event_handlers = evh;
      offscreen = false;
    }

  ~Am_Drawonable_Impl ()
    {
      if (title) delete[] title;
      if (icon_name) delete[] icon_name;
    }

  void Print (std::ostream& os) const;

  // Data
  gwt_window_t gwt_window;
  bool is_root; // root window?
  Am_Region* clip_region;
  bool offscreen; // offscreen pixmap?
  // variable to hold the event handlers for this window
  Am_Input_Event_Handlers *event_handlers;
  Am_Drawonable *owner;  //Am_Drawonable this is nested in
  // wrapper type id
  static Am_ID_Tag id;

  // flags for input handling
  bool want_enter_leave;
  bool want_multi_window;
  bool want_move;
  long current_event_mask;
 private:
  void set_gc_using_fill_style (const Am_Style& fill, const Am_Draw_Function f) const;
  void set_gc_using_line_style (const Am_Style& line, const Am_Draw_Function f) const;
  void poly_draw_it (const Am_Style& ls, const Am_Style& fs,
		     Am_Draw_Function f,
		     GGIGEM_Point* ar, const int num_points) const;
};

//
// Given the GWT window, returns the corresponding drawonable object
//
extern void Set_Drawable_Backpointer (gwt_window_t window, Am_Drawonable_Impl *d);
extern Am_Drawonable_Impl *Get_Drawable_Backpointer (gwt_window_t window);

//
// Am_Font_Data
//
class Am_Font_Data : public Am_Wrapper
{
  friend class Am_Font;
  Am_WRAPPER_DATA_DECL(Am_Font);
 public:
  void Print (std::ostream& os) const;
  Am_Font_Data (Am_Font_Data* proto);
  Am_Font_Data (const char* the_name);
  Am_Font_Data (Am_Font_Family_Flag f = Am_FONT_FIXED,
		bool is_bold = false,
		bool is_italic = false,
		bool is_underline = false,
		Am_Font_Size_Flag s = Am_FONT_MEDIUM);

  ~Am_Font_Data ();

  bool operator== (Am_Font_Data&)
  { return false; }
  bool operator== (Am_Font_Data&) const
  { return false; }

  const char* Get_Font_Name () const;
  //returns a pointer to the string, don't dealloc

  // get values
  void Get_Values (Am_Font_Family_Flag& f,
		   bool& is_bold,
		   bool& is_italic,
		   bool& is_underline,
		   Am_Font_Size_Flag& s) const;

  int Char_Width ();
  bool Underline ();

  // data
protected:
  char *name;
  Am_Font_Family_Flag family;
  Am_Font_Size_Flag size;
  bool bold;
  bool italic;
  bool underline;
  int char_width;
};

#if 0
class Color_Index
{
 public:
  Color_Index (Display *disp, XColor idx)
  { next = (0L); dpy = disp; index = idx; }
  Display *dpy;
  //unsigned long index;
  XColor index;
  Color_Index* next;
};
#endif
//
// Cursor storage
//
class Am_Cursor_Data : public Am_Wrapper
{
  friend class Am_Cursor;
  Am_WRAPPER_DATA_DECL(Am_Cursor)

  public:
  // creators
  Am_Cursor_Data()
    : image(Am_No_Image), mask(Am_No_Image),
    fg_color(Am_No_Style), bg_color(Am_No_Style),
    x_hot(0), y_hot(0)
    { }
  Am_Cursor_Data(Am_Image_Array in_image, Am_Image_Array in_mask,
		 Am_Style in_fg_color, Am_Style in_bg_color)
    : image(in_image), mask(in_mask), fg_color(in_fg_color),
    bg_color(in_bg_color), x_hot(0), y_hot(0)
    { }
  Am_Cursor_Data (Am_Cursor_Data *proto)
    : image(proto->image), mask(proto->mask),
    fg_color(proto->fg_color), bg_color(proto->bg_color),
    x_hot(proto->x_hot), y_hot(proto->y_hot)
    { }
  ~Am_Cursor_Data ()
    { }
  // I think I need this for a wrapper class
  bool operator== (Am_Cursor_Data&)
  { return false; }
  bool operator== (Am_Cursor_Data&) const
  { return false; }

private:
  /* holds data till we need it */
  Am_Image_Array image;
  Am_Image_Array mask;
  Am_Style fg_color;
  Am_Style bg_color;

  int x_hot;
  int y_hot;
};

#if 0
class Bitmap_Item
{
 public:
  Bitmap_Item (Display *disp, Pixmap in_bitmap, XColor* in_col, int n_col)
    : display(disp), bitmap(in_bitmap), mask(0), inverted_mask(0),
    colors(in_col), num_colors(n_col), next(0L)
    {}
  ~Bitmap_Item () {
    if (display) {
      if (bitmap) XFreePixmap(display, bitmap);
      if (mask) XFreePixmap(display, mask);
      if (inverted_mask) XFreePixmap(display, inverted_mask);
    }
    if (colors) {
      int screen_num = DefaultScreen(display);
      Colormap c = XDefaultColormap(display, screen_num);
      for (int i = 0; i < num_colors; i++)
	XFreeColors(display, c, &(colors[i].pixel), 1, 0);
      delete[] colors;
      colors = (0L);
    }
  }
  Display *display;
  Pixmap bitmap;
  Pixmap mask;
  Pixmap inverted_mask;
  // keep track of colors we use, so we can free them.
  XColor *colors;
  int num_colors;
  Bitmap_Item* next;
};
#endif

class Am_Image_Array_Data : public Am_Wrapper
{
  friend class Am_Image_Array;
  Am_WRAPPER_DATA_DECL(Am_Image_Array)
public:
  Am_Image_Array_Data ();
  Am_Image_Array_Data (Am_Image_Array_Data* proto);
  Am_Image_Array_Data (const char* file_name);
  Am_Image_Array_Data (const char* bit_data, int height, int width);

  ~Am_Image_Array_Data ();

  bool operator== (Am_Image_Array_Data&)
  { return false; }
  bool operator== (Am_Image_Array_Data&) const
  { return false; }

  // Return platform independent image in RGB form
  bool Get_RGB_Image(unsigned char * storage, bool top_first = true);

  // notice this isn't necessarily correct: it returns the depth of the
  // machine-independent pixmap, not the depth of the X pixmap we're using.
  int Get_Depth()
  {if (generic_image) return generic_image->Get_Depth(); else return 0;}

  bool Is_Transparent()
  {if (generic_image) return generic_image->Is_Transparent(); else return false;}

private:
  // utility routines
  bool is_gif();
  bool make_generic_image_from_name();
  // Am_Generic_Image stores machine-independant data.
  Am_Generic_Image *generic_image;
  /* holds the name of the bitmap till we need it */
  char *name;
};

class Am_Style_Data : public Am_Wrapper
{
  friend class Am_Style;
  Am_WRAPPER_DATA_DECL(Am_Style)
public:
  void Print (std::ostream& os) const;

  Am_Style_Data (Am_Style_Data* proto);
  Am_Style_Data (float r, float g, float b,
                 short thickness,  Am_Line_Cap_Style_Flag cap,
                 Am_Join_Style_Flag join,  Am_Line_Solid_Flag line_flag,
                 const char* dash_l, int dash_l_length,
                 Am_Fill_Solid_Flag fill_flag,
                 Am_Fill_Poly_Flag poly,  Am_Image_Array stipple);

  Am_Style_Data (const char * cname,
		 short thickness,  Am_Line_Cap_Style_Flag cap,
		 Am_Join_Style_Flag join,  Am_Line_Solid_Flag line_flag,
		 const char* dash_l, int dash_l_length,
		 Am_Fill_Solid_Flag fill_flag,
		 Am_Fill_Poly_Flag poly,  Am_Image_Array stipple);

  Am_Style_Data (int percent, Am_Fill_Solid_Flag fill_flag);

  Am_Style_Data (const char* name, bool bit_is_on);
  Am_Style_Data (Am_Style_Data* proto, Am_Style_Data* new_color);

  Am_Style_Data ();
  ~Am_Style_Data ();

  bool operator== (Am_Style_Data&)
  { return false; }
  bool operator== (Am_Style_Data&) const
  { return false; }

  // returns a pointer to the string, don't dealloc
  const char *Get_Color_Name ();

#if 0
  void Add_Color_Index (Display*, XColor);
  bool Get_Color_Index (Display*, XColor&);

  //gets the index to use on display
  unsigned long Get_X_Index (const Am_Drawonable_Impl *d);
  XColor Get_X_Color (const Am_Drawonable_Impl* d);

  static void remove (Am_Style_Data* style);
  static void remove (Display* display);
#endif
protected:
  // data
  const char* color_name;
#if 0
  Display *main_display;
  //  unsigned long main_color_index;
  XColor main_color;
  Color_Index* color_head;
  bool color_allocated;

  Am_Style_Data* next;
  static Am_Style_Data* list;
#endif
  // color part
  float red, green, blue;
  // line style part
  short line_thickness;
  Am_Line_Cap_Style_Flag cap_style;
  Am_Join_Style_Flag join_style;
  Am_Line_Solid_Flag line_solid;
  char *dash_list;
  int dash_list_length;
  // filling style part
  Am_Fill_Solid_Flag fill_solid;
  Am_Fill_Poly_Flag fill_poly;
  Am_Image_Array stipple_bitmap;
};

extern Am_Style_Data Am_On_Bits_Data;
extern Am_Style_Data Am_Off_Bits_Data;

class Am_Point_Array_Data : public Am_Wrapper
{
  friend class Am_Point_Array;
  friend class Am_Drawonable_Impl;
  // for direct access to GGIGEM_Point representation
  Am_WRAPPER_DATA_DECL(Am_Point_Array)
public:
  Am_Point_Array_Data (Am_Point_Array_Data *proto);
  Am_Point_Array_Data (Am_Point_List pl, int offset_x, int offset_y);
  Am_Point_Array_Data (int *coords, int num_coords);
  Am_Point_Array_Data (int size);
  ~Am_Point_Array_Data ();

  bool operator== (Am_Point_Array_Data&)
  { return false; }
  bool operator== (Am_Point_Array_Data&) const
  { return false; }

protected:
  GGIGEM_Point *ar;
  int size;
};

//----------------
// Am_Region_Impl
//----------------

class Am_Region_Impl : public Am_Region
{
	public:
		// creator function is Am_Region::Create
		void Clear();
		void Destroy();
		void Intersect(int left, int top, unsigned int width, unsigned int height);
		bool In(int x, int y);
		bool In(int left, int top, unsigned int width, unsigned int height, bool& total);
		bool In(Am_Region* rgn, bool& total);
		void Set(int left, int top, unsigned int width, unsigned int height);
		void Push(Am_Region *region);
		void Push(int left, int top, unsigned int width, unsigned int height);
		void Pop();
		void Union(int left, int top, unsigned int width, unsigned int height);

		// Internal functions
		Am_Region_Impl();

		bool all_rgns_used();
		void add_more_rgns();

		gwt_region_t region_to_use()
		{ return(x_rgns[index]); }

		// Data members
		gwt_region_t *x_rgns;
		int size;
		int index;
		int max_index;
};

//-------------
// Inter stuff
//-------------
// process an input event.  This can be called if another application
// wants to also see the input events.  This does not deal with
// removing the event from the event queue.
//extern void Am_Handle_Event_Received (XEvent& event_return);

#endif
