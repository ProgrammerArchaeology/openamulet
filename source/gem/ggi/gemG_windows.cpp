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

// This file contains Am_Drawonable_Impl member functions for
// the X version of Gem

#include <sys/time.h>
#include <stdlib.h>
#ifndef NEED_SELECT
#ifdef GCC
extern "C" {
int select ();
}
#endif
#endif
#ifdef NEED_UNISTD
#include <unistd.h>
#endif

#include <iostream>

#include <am_inc.h>

#include GEM__H
#include GEMG__H
#include "amulet/gemG_time.h"
#include CONNECTION__H

// GGI files inclusion
#include <ggi/gii.h>
#include <ggi/ggi.h>
#include <ggi/gwt.h>
//
// GGI Main definitions
//
// TODO: These should be set to GT_AUTO in the end
#define DEFAULT_SCREEN_WIDTH 800
#define DEFAULT_SCREEN_HEIGHT 600
#define DEFAULT_VIRTUAL_WIDTH 800
#define DEFAULT_VIRTUAL_HEIGHT 600
#define DEFAULT_MODE GT_AUTO

//
// GGI main global variables
//

// Root drawonable
Am_Drawonable *GGIGEM_Root_Drawonable = (0L);
// For convenience: also GWT root window
gwt_window_t GWT_Root_Window = (0L);
// Root mode
static ggi_graphtype gwt_root_mode = DEFAULT_MODE;

// Global useful values
ggi_pixel ggi_black, ggi_white;

// // // // // // // // // // // // // // // // // // // // // // // // // //
//
// Am_Drawonable creator functions
//
// // // // // // // // // // // // // // // // // // // // // // // // // //
//
// Root drawonable creation function
//
Am_Drawonable* Am_Drawonable::Get_Root_Drawonable (const char* screen)
{
  // if this screen's root drawonable has already been requested, just
  // return it
  // TODO: Cache the screen root windows...
  // TODO: Currently, I always return the same thing
  if (GGIGEM_Root_Drawonable != (0L))
    return GGIGEM_Root_Drawonable;

  // // // // // // // // // // // // // // // // // // //
  //    GGI
  // // // // // // // // // // // // // // // // // // //
  // TODO: Only ONE root under GGI for now...
  // TODO:If the screen's root drawonable has already been requested, we
  // simply return the old one.

  // GGI initialization
  giiInit();
  ggiInit();
  gwtInit();
  // Root allocation
  gii_input_t inp = giiOpen(NULL);
  ggi_visual_t vis = ggiOpen(NULL);
  gwtAttach(vis);
  if (ggiSetGraphMode(vis,
		      DEFAULT_SCREEN_WIDTH,
		      DEFAULT_SCREEN_HEIGHT,
		      DEFAULT_VIRTUAL_WIDTH,
		      DEFAULT_VIRTUAL_HEIGHT,
		      DEFAULT_MODE))
    {
      Am_Error("** Couldn't set default GGI graphic mode.\n");
    }
  gwt_window_t root;
  int err = gwtCreateRootWindow(inp, vis, &root);

  if (err)
    Am_Error("** Couldn't open GWT root window.\n");

  // Obtain information on the mode
  ggi_mode mode;
  ggiGetMode(vis,&mode);
  // Remembers it (for offscreen window essentially)
  gwt_root_mode = mode.graphtype;
  unsigned int depth;
  switch (mode.graphtype)
    {
    case GT_1BIT:    depth = 1; break;
    case GT_4BIT:    depth = 4; break;
    case GT_8BIT:    depth = 8; break;
    case GT_15BIT:   depth = 15; break;
    case GT_16BIT:   depth = 16 ;break;
    case GT_24BIT:   depth = 24; break;
    case GT_32BIT:   depth = 32; break;
    default:
      Am_Error("Unable to determine GGI visual depth");
      break;
    }
  
  printf("Got GGI mode (%dx%d [%dx%d] %dbpp).\n",
	 mode.visible.x, mode.visible.y,
	 mode.virt.x, mode.virt.y, depth);
  
  // Finds black and white
  ggi_color rgb;
  rgb.r = 0xFFFF; rgb.g = 0xFFFF; rgb.b = 0xFFFF; // White
  ggi_white = ggiMapColor(vis, &rgb);
  rgb.r = 0x0000; rgb.g = 0x0000; rgb.r = 0x0000; // Black
  ggi_black = ggiMapColor(vis, &rgb);

  // Set the initial GC
  gwtSetGCBackground(root, ggi_white);
  gwtSetGCForeground(root, ggi_black);
  // Clear the screen also
  gwtFillscreen(root);

  Am_Drawonable_Impl *ggi_draw =
    new Am_Drawonable_Impl (0, 0, mode.virt.x, mode.virt.y,
			    "", "", true, false, Am_No_Style,
			    false, 1, 1, 0, 0, true,
			    false, depth, (0L));
  ggi_draw->gwt_window = root;
  ggi_draw->is_root = true;
  ggi_draw->clip_region = Am_Region::Create();
  // Creates a backpointer in the window also
  // TODO: Maybe we could use something different ?
  Set_Drawable_Backpointer(root, ggi_draw);

  // Initializes the global variables
  GGIGEM_Root_Drawonable = ggi_draw;
  GWT_Root_Window = root;

  return ggi_draw;
}
//
// Drawonable creation function
//
// Defaults:
//   l = 0,  t = 0,  w = 100,  h = 100
//   window-name = "",
//   icon_name = "",
//   vis = true,
//   initially_iconified = false,
//   back_color = (0L),
//   save_under_flag = false,
//   // can't have 0 size windows
//   min_w = 1, min_h = 1,
//   // 0 means no max
//   max_w = 0, max_h = 0,
//   title_bar_flag = true,
//   query_user_for_position = false,
//   query_user_for_size = false,
//   clip_by_children_flag = true,
//   evh = (0L);
Am_Drawonable* Am_Drawonable_Impl::Create (int l, int t, unsigned int w,
					   unsigned int h,
					   const char* window_name,
					   const char* icon_name,
					   bool vis,
					   bool initially_iconified,
					   Am_Style back_color,
					   bool save_under_flag,
					   int min_w, int min_h,
					   int max_w, int max_h,
					   bool title_bar_flag,
					   bool query_user_for_position,
					   bool query_user_for_size,
					   bool clip_by_children_flag,
					   Am_Input_Event_Handlers *evh)
{
  Am_Drawonable_Impl* d =
    new Am_Drawonable_Impl (l, t, w, h, window_name,
			    icon_name, vis, initially_iconified, back_color,
			    save_under_flag, min_w, min_h,
			    max_w, max_h, title_bar_flag,
			    clip_by_children_flag, this->depth, evh);
    d->owner = this;
    //
    // GGI - GWT window creation
    //
    gwt_window_t new_win;
    gwtCreateWindow(this->gwt_window, &new_win); // created hidden
    gwtShow(new_win); // shows the window (NB: Before move&resize!)
    gwtMove(new_win, l, t);
    gwtResize(new_win, w, h);

    // Remembers the window
    d->gwt_window = new_win;
    d->clip_region = Am_Region::Create();
    // Also sets a backpointer in the private field of window
    Set_Drawable_Backpointer(new_win, d);

    // Set the initial GC (especialy backgound)
    gwtSetGCForeground(new_win,ggi_white); // foreground
    float r,g,b;
    back_color.Get_Values(r,g,b); // obtain background color
    ggi_color color;
    color.r = (int)((0xFFFF) * r);
    color.g = (int)((0xFFFF) * g);
    color.b = (int)((0xFFFF) * b);
    ggi_visual_t visual;
    gwtGetVisual(new_win, &visual);
    ggi_pixel c_index = ggiMapColor(visual, &color);
    gwtSetGCBackground(new_win, c_index); // background
    // Clears the screen
    gwtFillscreen(new_win);

    gwtDrawVisibleRegion(new_win);
    return d;
}
//
// Drawonable destruction
//
void Am_Drawonable_Impl::Destroy ()
{
  gwtDestroy(this->gwt_window);
  // TODO: Should we flush the output here ?
  delete this;
}

// // // // // // // // // // // // // // // // // // // //
//
//   Setting Window Properties
//
// // // // // // // // // // // // // // // // // // // //

void Am_Drawonable_Impl::Set_Title (const char* new_title)
{
 std::cerr << "GWT Set_Title to be implemented" <<std::endl;
}

void Am_Drawonable_Impl::Set_Icon_Title (const char* new_title)
{
 std::cerr << "GWT Set_Icon_Title to be implemented" <<std::endl;
}

void Am_Drawonable_Impl::Set_Position(int new_left, int new_top)
{
  if ((left == new_left) && (top == new_top))
    return;
  left = new_left;
  top = new_top;
  if (offscreen)
    return; // TODO: Should we raise an error here ?
  if (is_root)
    Am_Error("Unable to move a GWT root window !!!");
  // Real movement
  gwtMove(this->gwt_window, new_left, new_top);
}

// Note: Set_Size() intercepts invalid size requests and clips the new width
// and height requests to the current max/min values stored in the
// Am_Drawonable.
void Am_Drawonable_Impl::Set_Size(unsigned int req_width,
				  unsigned int req_height)
{
  // max_{height,width} == 0 means not limit in fact
  unsigned int new_width = (max_width == 0)
    ? MAX(req_width, min_width)
    : MIN(MAX(req_width, min_width), max_width);
  unsigned new_height = (max_height == 0)
    ? MAX(req_height, min_height)
    : MIN(MAX(req_height, min_height), max_height);

  if ((width != new_width) || (height != new_height))
    {
      width = new_width;
      height = new_height;
      if (offscreen)
	{
	 std::cerr << "GWT Set_Size to be implemented for offscreen windows" <<std::endl;
	}
      else
	{
	  gwtResize(this->gwt_window, req_width, req_height);
	}
    }
}

void Am_Drawonable_Impl::Set_Min_Size(unsigned int min_w, unsigned int min_h)
{
  // Make a window at least one pixel wide in each dimension.
  if (min_w < 1) min_w = 1;
  if (min_h < 1) min_h = 1;
  // Set internal Am_Drawonable data
  min_width = min_w;
  min_height = min_h;

 std::cerr << "GWT Set_Min_Size Note that amulet provides without gwt support" <<std::endl;
}

void Am_Drawonable_Impl::Set_Max_Size(unsigned int max_w, unsigned int max_h)
{
  // Set internal Am_Drawonable data
  max_width = max_w;
  max_height = max_h;

 std::cerr << "GWT Set_Max_Size Note that amulet provides without gwt support" <<std::endl;
}

void Am_Drawonable_Impl::Set_Visible (bool vis)
{
  if (offscreen)
    return; // Not meaningful for offscreen pixmap TODO: Raise error ?
  visible = vis;
  if (vis)
    {
      gwtShow(this->gwt_window);
    }
  else
    {
      gwtHide(this->gwt_window);
    }
}

void Am_Drawonable_Impl::Set_Titlebar (bool new_title_bar)
{
 std::cerr << "GWT Set_Titlebar to be implemented" <<std::endl;
#if 0
  bool old_title_bar = title_bar;
  // The default for new_width should be whatever the old_width was.
   title_bar = new_title_bar;
  border_width = title_bar ? 2 : 0;
#endif
}

// This function clears the window with the new background color and
// generates an expose event.  The clip mask is ignored as the window
// is cleared with the new background color.
// TODO: Check is this is really useful ?
void Am_Drawonable_Impl::Set_Background_Color(Am_Style new_color)
{
  background_color = new_color;
  if (offscreen) {
    // want to fill the pixmap with background?
    return;
  }
  Am_Style_Data *n_color = (Am_Style_Data::Narrow(new_color));
 std::cerr << "GWT Set_Titlebar to be implemented: obtain GGI color" <<std::endl;
  if (n_color)
    n_color->Release ();
  gwtFillscreen(this->gwt_window);
}

void Am_Drawonable_Impl::Reparent (Am_Drawonable *new_parent)
{
  gwtReparent(this->gwt_window,
	      dynamic_cast<Am_Drawonable_Impl*>(new_parent)->gwt_window);
  owner = new_parent;
}

// Note: to translate to screen coordinates, invoke this method on the
// root drawonable.
void Am_Drawonable_Impl::Translate_Coordinates(int src_x, int src_y,
					       Am_Drawonable *src_d,
					       int& dest_x_return, int& dest_y_return)
{
  if (!(src_d))
    Am_Error ("** Translate_Coordinates: source not supplied.\n");

 std::cerr << "GWT Translate_Coordinates to be implemented" <<std::endl;
#if 0
  if (screen->display != ((Am_Drawonable_Impl *)src_d)->screen->display) {
   std::cerr << "** Translate_Coordinates: THIS " << this << " and src_d " << src_d
	 << " have different displays" <<std::endl;
    Am_Error ();
  }
#endif
}

void Am_Drawonable_Impl::Flush_Output ( )
{
 std::cerr << "GWT Flush_Output to be implemented" <<std::endl;
}

// // // // // // // // // // // // // // // // // // // //
//
//   Value retrieval functions
//
// // // // // // // // // // // // // // // // // // // //

bool Am_Drawonable_Impl::Inquire_Window_Borders
    (int& left_border, int& top_border, int& right_border, int& bottom_border,
     int& outer_left, int& outer_top)
{
  left_border = top_border = right_border = bottom_border = 0;
  // By "default", outer_left and outer_top will be the same as the inner
  // left and top. TODO: Check the sense of that
  outer_left = left;
  outer_top = top;
  return true;
}

// // // // // // // // // // // // // // // // // // // //
//
//   Internal functions for backpointers from Drawable to Am_Drawonable
//
// // // // // // // // // // // // // // // // // // // //

Am_Drawonable_Impl *Get_Drawable_Backpointer (gwt_window_t window)
{
  if (!window)
    return (0L);
  void* priv;
  gwtGetPrivate(window, &priv);
  return static_cast<Am_Drawonable_Impl*>(priv);
}

void Set_Drawable_Backpointer (gwt_window_t window, Am_Drawonable_Impl *d)
{
  gwtSetPrivate(window, d);
}

void Am_Drawonable_Impl::Print (std::ostream& os) const {
  os << this->Get_Title()
     << " (" <<std::hex << (void *)this <<std::dec << ")";
}

Am_Drawonable* Am_Drawonable_Impl::Create_Offscreen(int width,
						    int height,
						    Am_Style background_color)
{
  Am_Drawonable_Impl* d =
    new Am_Drawonable_Impl (0, 0, width, height,
			    "", "", false, false,
			    background_color, false, 0, 0,
			    0, 0, false, true, this->depth, (0L));
  d->owner = this;
  d->offscreen = true;

  //   std::cerr << "GWT-GEM: Create_Offscreen to be implemented !!!" <<std::endl;

  // // // // // // // // // // // // // // // // // // //
  //    GGI
  // // // // // // // // // // // // // // // // // // //
  // Creates a visual in memory
  ggi_visual_t off_vis = ggiOpen("display-memory",NULL);
  if (off_vis == 0L)
    Am_Error("Unable to create offscreen visual");
  int err = ggiSetGraphMode(off_vis, width, height, width, height,
			    gwt_root_mode);
  if (err != 0)
    Am_Error("Unable to set correct graph mode on offscreen visual");
  if (GT_SCHEME(gwt_root_mode) == GT_PALETTE)
    {
     std::cerr << " Offscreen: palette mode... setting colorful palette" <<std::endl;
      int other_err = ggiSetColorfulPalette(off_vis);
      if (other_err < 0)
std::cerr << "PROBLEM in setting colorful palette" <<std::endl;
    }
  gii_input_t inp = giiOpen("input-null",NULL);
  // Creates a new gwt root window
  gwt_window_t offscreen_win;
  gwtCreateRootWindow(inp, off_vis, &offscreen_win);
    
  // Copy the various gwt data
  // FIXME: Should we use a special object (Note_Reference ?) ?
  d->is_root = false;
  d->clip_region = Am_Region::Create();
  printf("Create_Offscreen: vis=%d isroot=%d, reg=%d\n",off_vis,d->is_root,d->clip_region);
  d->gwt_window = offscreen_win;
  // Also sets a backpointer in the private field of window
  Set_Drawable_Backpointer(offscreen_win, d);

  // Set the initial GC (especialy backgound)
  gwtSetGCForeground(offscreen_win,ggi_white); // foreground
  float r,g,b;
  background_color.Get_Values(r,g,b); // obtain background color
  ggi_color color;
  color.r = static_cast<uint16>((0xFFFF) * r);
  color.g = static_cast<uint16>((0xFFFF) * g);
  color.b = static_cast<uint16>((0xFFFF) * b);
  ggi_pixel c_index = ggiMapColor(off_vis, &color);
  gwtSetGCBackground(offscreen_win, c_index); // background
  // Clears the screen
  gwtFillscreen(offscreen_win);

  return d;
}

void Am_Drawonable_Impl::Raise_Window (Am_Drawonable *target_d)
{
  // TODO: Check if the fact that the owner should match is handled
  if (target_d != (0L))
    gwtRaise(this->gwt_window,
	     dynamic_cast<Am_Drawonable_Impl*>(target_d)->gwt_window);
  else
    gwtRaise(this->gwt_window, NULL);
}

void Am_Drawonable_Impl::Lower_Window (Am_Drawonable *target_d)  //to bottom
{
  // TODO: Check if the fact that the owner should match is handled
  if (target_d != (0L))
    gwtLower(this->gwt_window,
	     dynamic_cast<Am_Drawonable_Impl*>(target_d)->gwt_window);
  else
    gwtLower(this->gwt_window, NULL);
}

void Am_Drawonable_Impl::Set_Iconify (bool new_iconified)
{
  if (iconified != new_iconified)
    iconified = new_iconified;
 std::cerr << "GWT Set_Iconify not implemented !!!" <<std::endl;
}

void Am_Drawonable_Impl::Beep ()
{
 std::cerr << "GWT Beep not implemented !!!" <<std::endl;
}

// destination for bitblt is the Am_Drawonable this message is sent to
// Defaults:
//   df = Am_DRAW_COPY
void Am_Drawonable_Impl::Bitblt(int d_left, int d_top, int width, int height,
				Am_Drawonable* source, int s_left, int s_top,
				Am_Draw_Function df)
{
  // std::cerr << "GWT Bitblt not implemented !!!" <<std::endl;
  Am_Drawonable_Impl *src = dynamic_cast<Am_Drawonable_Impl*>(source);
  // TODO: Handle drawing function (df)
  // TODO: See if 'gwtCopyBox' can/should be used ?
  gwtCrossBlit(src->gwt_window, s_left, s_top, width, height,
	       gwt_window, d_left, d_top);
}

// cut buffer, eventually, this needs to be generalized
void Am_Drawonable_Impl::Set_Cut_Buffer(const char *s)
{
  if (!s || offscreen)
    return;  // do nothing with a (0L) string for now.
 std::cerr << "GWT Set_Cut_Buffer not implemented !!!" <<std::endl;
}

/// END OF FILE ///


////////////////////////////////////////////////////////////////////////////
// This was kept for remembering some details... -- ortalo
// Delete someday
#if 0
void Screen_Manager::Block (timeval *timeout)
{
    //
    // Right now there is no way to wait on several displays at once. To do
    // that we would have to implement a separate thread for each display.
    //
    // So if the caller did not specify a timeout then we will end up
    // blocking on the first display even if there are events waiting on
    // other displays.
    //
    // To get around this, if there are multiple displays then we force a
    // small timeout. This gives us proper operation in applications which
    // only use a single display and a busy-wait-loop in applications which
    // use multiple displays.
    //
    timeval multiDisplayTimeout;

    if (More_Than_One_Display && (timeout == NULL))
    {
        multiDisplayTimeout.tv_sec = 0;
        multiDisplayTimeout.tv_usec = 100;
        timeout = &multiDisplayTimeout;
    }

    int status;
    // We only want to block until we time out.  If we have no event
    // by then, oh well, just exit anyway.

    // read_Fd_Select is destructively modified by select() so save a copy
    fd_set read_Fd_Select = read_flag;

    ////////////////////////////////////////////////////////////////
    // Networking hooks
    int new_nfds=0;

    if (Am_Connection::Active())
    {
        new_nfds=Am_Connection::Merge_Mask(&read_Fd_Select);
        if (nfds > new_nfds) new_nfds=nfds;
    }
    else // No open connections
    {
        new_nfds=nfds;
    }

    // Return from Networking Hooks
    //////////////////////////////////////////////////////////////
#ifdef HP
    status = select (new_nfds, (int*)&read_Fd_Select, (0L), NULL,
           timeout);
#else
    status = select (new_nfds, &read_Fd_Select, (0L), NULL,
           timeout);
#endif
    if (status < 0) perror("Am_Error in Block");

    //////////////////////////////////////////////////////////////
    // One last Networking hook

    if (Am_Connection::Sockets_Have_Input(&read_Fd_Select))
        Am_Connection::Handle_Sockets(&read_Fd_Select);

    // End of Networking hooks.
    /////////////////////////////////////////////////////////////
}
#endif
