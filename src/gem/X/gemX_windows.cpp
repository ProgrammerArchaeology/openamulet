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

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/Xatom.h>
#include <sys/time.h>
#include <stdlib.h>
#ifdef NEED_UNISTD
#include <unistd.h>
#endif

#include <iostream>

#include <am_inc.h>

#include <amulet/gem.h>
#include <amulet/gemX.h>
#include "amulet/gemX_time.h"
#include <amulet/connection.h>

// // // // // // // // // // // // // // // // // // // // // // // // // //
//
// Am_Drawonable creator functions
//
// // // // // // // // // // // // // // // // // // // // // // // // // //

// Declared in gemX.h
// The two different masks are required because you are not allowed to ask
// the GC what its dash value is (see X documentation for XGetGCValues).
// Am_GC_setmask is only used during initialization of the GC.
// One mask is used to set the GC, and the other is used to query the GC.
//
unsigned long Am_GC_setmask =
    (GCForeground | GCLineWidth | GCCapStyle | GCJoinStyle | GCFunction |
     GCLineStyle | GCDashList | GCBackground);

unsigned long Am_GC_getmask =
    (GCForeground | GCLineWidth | GCCapStyle | GCJoinStyle | GCFunction);

GC
create_default_gc(Display *dpy, Window the_xlib_drawable, unsigned long black,
                  unsigned long white)
{
  XGCValues gcvalues;
  gcvalues.foreground = black;
  gcvalues.background = white;
  gcvalues.line_width = 0;
  gcvalues.line_style = LineSolid;
  gcvalues.cap_style = CapButt;
  gcvalues.join_style = JoinMiter;
  gcvalues.dashes = *Am_DEFAULT_DASH_LIST;
  gcvalues.function = GXcopy;
  // should set all the other slots to the appropriate default values

  GC the_gc = XCreateGC(dpy, the_xlib_drawable, Am_GC_setmask, &gcvalues);
  return the_gc;
}

// GetVRoot was stolen from the comp.windows.x FAQ part 6.
// af1x 12-1-95
/* Function Name: GetVRoot
 * Description: Gets the root window, even if it's a virtual root
 * Arguments: the display and the screen
 * Returns: the root window for the client
 */
Window
GetVRoot(Display *dpy, int scr)
{
  Window rootReturn, parentReturn, *children;
  unsigned int numChildren;
  Window root = RootWindow(dpy, scr);
  Atom __SWM_VROOT = None;
  unsigned int i;

  __SWM_VROOT = XInternAtom(dpy, "__SWM_VROOT", False);
  XQueryTree(dpy, root, &rootReturn, &parentReturn, &children, &numChildren);
  for (i = 0; i < numChildren; i++) {
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytesafter;
    Window *newRoot = (0L);

    if (XGetWindowProperty(dpy, children[i], __SWM_VROOT, 0, 1, False,
                           XA_WINDOW, &actual_type, &actual_format, &nitems,
                           &bytesafter,
                           (unsigned char **)&newRoot) == Success &&
        newRoot) {
      root = *newRoot;
      XFree((char *)newRoot);
      break;
    }
  }

  XFree((char *)children);
  return root;
}

void
amulet_x_error(Display *display, XErrorEvent *myerr)
{
  char msg[80];
  XGetErrorText(display, myerr->error_code, msg, 80);
  std::cout << "*** X Error: " << msg << std::endl;
  Am_Error();
}

bool More_Than_One_Display = false;
Display *Main_Display = (Display *)(0L);
Screen_Manager Scrn_Mgr;

static Am_Drawonable *
make_root_drawonable(const char *screen, Display *new_display)
{
  if (Main_Display != (0L)) {
    if (XConnectionNumber(new_display) != XConnectionNumber(Main_Display))
      More_Than_One_Display = true;
  } else
    Main_Display = new_display;

  int screen_num = DefaultScreen(new_display);
  //  Window the_xlib_drawable = RootWindow (new_display, screen_num);
  // in virtual window managers, we might have a virtual root window.
  // GetVRoot gets the virtual root if there, or root window otherwise.
  Window the_xlib_drawable = GetVRoot(new_display, screen_num);
  Window dummy_root;
  unsigned long black = XBlackPixel(new_display, screen_num);
  unsigned long white = XWhitePixel(new_display, screen_num);
  GC gc = create_default_gc(new_display, the_xlib_drawable, black, white);

  int l, t;
  unsigned int w, h;
  unsigned int border_w;
  unsigned int bit_depth;

  if (!XGetGeometry(new_display, the_xlib_drawable, &dummy_root, &l, &t, &w, &h,
                    &border_w, &bit_depth))
    Am_Error("** Get_Root_Drawonable: can't get root window geometry.\n");

  Am_Drawonable_Impl *d =
      new Am_Drawonable_Impl(l, t, w, h, "", "", true, false, Am_No_Style,
                             false, 1, 1, 0, 0, true, false, bit_depth, (0L));

  d->screen = new Screen_Desc(new_display, screen_num, d,
                              XDefaultColormap(new_display, screen_num),
                              bit_depth, black, white, gc);

  // Allocate the cut buffer atom
  d->screen->cut_buffer =
      XInternAtom(d->screen->display, "AM_CUT_BUFFER0", False);
  if (d->screen->cut_buffer == None)
    Am_Error("** X Error: Can't make AM_CUT_BUFFER0 atom\n");

  d->screen->clip_region = Am_Region::Create();
  d->xlib_drawable = the_xlib_drawable;
  d->owner = (0L);
  Am_Initialize_Char_Map();

  // Make sure all 8 cut buffers exist on this display, so we can do
  // XStoreBytes and XFetchBytes successfully.
  XChangeProperty(new_display, the_xlib_drawable, XA_CUT_BUFFER0, XA_STRING, 8,
                  PropModeAppend, 0, 0);
  XChangeProperty(new_display, the_xlib_drawable, XA_CUT_BUFFER1, XA_STRING, 8,
                  PropModeAppend, 0, 0);
  XChangeProperty(new_display, the_xlib_drawable, XA_CUT_BUFFER2, XA_STRING, 8,
                  PropModeAppend, 0, 0);
  XChangeProperty(new_display, the_xlib_drawable, XA_CUT_BUFFER3, XA_STRING, 8,
                  PropModeAppend, 0, 0);
  XChangeProperty(new_display, the_xlib_drawable, XA_CUT_BUFFER4, XA_STRING, 8,
                  PropModeAppend, 0, 0);
  XChangeProperty(new_display, the_xlib_drawable, XA_CUT_BUFFER5, XA_STRING, 8,
                  PropModeAppend, 0, 0);
  XChangeProperty(new_display, the_xlib_drawable, XA_CUT_BUFFER6, XA_STRING, 8,
                  PropModeAppend, 0, 0);
  XChangeProperty(new_display, the_xlib_drawable, XA_CUT_BUFFER7, XA_STRING, 8,
                  PropModeAppend, 0, 0);

  // Cache the display and root drawonable
  Scrn_Mgr.Add(screen, new_display, d);

  return d;
}

Am_Drawonable *
Am_Drawonable::Get_Root_Drawonable(const char *screen)
{

  // if this screen's root drawonable has already been requested, just
  // return it
  Display *new_display;
  Am_Drawonable *draw;
  if (Scrn_Mgr.Get(screen, new_display, draw))
    return draw;
  new_display = XOpenDisplay(screen);
  if (!new_display)
    Am_Error("** Couldn't open display.\n");

  // DEBUGGING STUFF IN X
  // This should be commented out most of the time.
  // If you start getting X errors and want to debug them quickly,
  // uncomment these lines and run your program again.  This will cause
  // X to crash through our own error handler, at the correct error location,
  // instead of crashing sometime randomly after the error occurs.  Also, the
  // stack is preserved, which is very useful.
  // SYNCHRONOUS DRAWING IS VERY SLOW.  It should NOT be turned on except
  // when debugging!
  // af1x 5-21-96  Uncomment the following 2 lines to easily debug in X
  // XSynchronize(new_display, 1);  // turn on synchronous drawing calls
  // XSetErrorHandler(amulet_x_error); // use our error handler to preserve stack

  draw = make_root_drawonable(screen, new_display);
  return draw;
}

void
install_attributes(XSetWindowAttributes *attrib,
                   unsigned long *attrib_value_mask, Am_Drawonable_Impl *d,
                   Screen_Desc *the_screen, Am_Style back_color,
                   bool save_under_flag, bool title_bar_flag, long event_mask)
{
  *attrib_value_mask = 0;

  Am_Style_Data *b_color = Am_Style_Data::Narrow(back_color);
  // Install background color in attribute structure
  if (back_color == Am_No_Style)
    attrib->background_pixel = the_screen->whitepixel;
  else
    attrib->background_pixel = b_color->Get_X_Index(d);
  if (b_color)
    b_color->Release();
  *attrib_value_mask |= CWBackPixel;

  // Set event mask for input handling
  attrib->event_mask = event_mask;
  *attrib_value_mask |= CWEventMask;

  // Set save-under flag
  attrib->save_under = save_under_flag;
  *attrib_value_mask |= CWSaveUnder;

  // Set whether window-manager should provide title-bar
  attrib->override_redirect = !(title_bar_flag);
  *attrib_value_mask |= CWOverrideRedirect;
}

void
set_other_window_properties(Window the_xlib_drawable, Display *dpy,
                            char *window_name, char *icon_name,
                            bool initially_iconified, int min_w, int min_h,
                            int max_w, int max_h, bool query_user_for_position,
                            bool query_user_for_size)
{
  // Since a window width or height of 0 is illegal, always set the min,
  // even if we got the default values of 1.
  if (min_w < 1)
    min_w = 1;
  if (min_h < 1)
    min_h = 1;

  // Data structures for setting the window title and icon title
  XTextProperty window_name_xtp, icon_name_xtp;
  XStringListToTextProperty(&window_name, 1, &window_name_xtp);
  XStringListToTextProperty(&icon_name, 1, &icon_name_xtp);

  // Data structure for setting the initial state (iconic or normal)
  XWMHints *the_wmhints;
  if (!(the_wmhints = XAllocWMHints()))
    Am_Error("** set_other_window_properties: failure allocating memory.\n");
  the_wmhints->flags = StateHint; // Only setting the state hint of struct
  if (initially_iconified == true)
    the_wmhints->initial_state = IconicState;
  else
    the_wmhints->initial_state = NormalState;

  // Data structure for setting the min and max size (all fields initially 0)
  XSizeHints *the_sizehints;
  if (!(the_sizehints = XAllocSizeHints()))
    Am_Error("** set_other_window_properties: failure allocating memory.\n");

  // Since a window width or height of 0 is illegal, always set the min,
  // even if we got the default values of 1.
  the_sizehints->min_width = min_w;
  the_sizehints->min_height = min_h;
  the_sizehints->flags = PMinSize;

  // If no max was specified (got 0 default), ignore the max
  if ((max_w != 0) || (max_h != 0)) {
    the_sizehints->max_width = max_w;
    the_sizehints->max_height = max_h;
    the_sizehints->flags |= PMaxSize;
  }

  // Set the_sizehints flag according to user-specified position
  //    HP Vuewm NOTE:  To get user-specified placement of windows to work
  //    as expected (i.e., like in my Sparc's MWM environment), you may need
  //    to set the following resources from your General Toolbox:
  //         Vuewm*cilentAutoPlace:       False
  //         Vuewm*interactivePlacement:  True
  if (query_user_for_position || query_user_for_size)
    // Then ignore X,Y specified by XCreateWindow
    the_sizehints->flags |= PPosition;
  else
    the_sizehints->flags |= USPosition;

  XSetWMProperties(dpy, the_xlib_drawable, &window_name_xtp, &icon_name_xtp,
                   (0L), 0, the_sizehints, the_wmhints, (0L));

  // Set it to grab window delete notification so we don't crash as much
  // we should probably store the atom in the root drawonable for speed,
  // but it's very rarely used.
  Atom wm_delete_window = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
  (void)XSetWMProtocols(dpy, the_xlib_drawable, &wm_delete_window, 1);

  XFree((char *)window_name_xtp.value);
  XFree((char *)icon_name_xtp.value);
  XFree((char *)the_wmhints);
  XFree((char *)the_sizehints);
}

// Create a new xlib_drawable for d
void
Am_Drawonable_Impl::Create_X_Drawable(Am_Drawonable_Impl *parent_d,
                                      bool ask_position, bool ask_size,
                                      XSetWindowAttributes *attrib,
                                      unsigned long attrib_value_mask)
{
  // X does not like zero width or height windows, so let's be careful that
  // we do not end up with any.
  if (width < min_width)
    width = min_width;
  if (height < min_height)
    height = min_height;

  xlib_drawable =
      XCreateWindow(screen->display, parent_d->xlib_drawable, left, top, width,
                    height, border_width, parent_d->depth, InputOutput,
                    CopyFromParent, attrib_value_mask, attrib);

  Set_Drawable_Backpointer(screen->display, xlib_drawable, this);

  set_other_window_properties(xlib_drawable, screen->display, title, icon_name,
                              iconified, min_width, min_height, max_width,
                              max_height, ask_position, ask_size);
}

// Create a new offscreen xlib_drawable for d
void
Am_Drawonable_Impl::Create_Offscreen_X_Drawable()
{
  xlib_drawable = XCreatePixmap(screen->display, screen->root->xlib_drawable,
                                width, height, depth);

  //  Set_Drawable_Backpointer(screen->display, xlib_drawable, this);
}

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
Am_Drawonable *
Am_Drawonable_Impl::Create(int l, int t, unsigned int w, unsigned int h,
                           const char *window_name, const char *icon_name,
                           bool vis, bool initially_iconified,
                           Am_Style back_color, bool save_under_flag, int min_w,
                           int min_h, int max_w, int max_h, bool title_bar_flag,
                           bool query_user_for_position,
                           bool query_user_for_size, bool clip_by_children_flag,
                           Am_Input_Event_Handlers *evh)
{
  Am_Drawonable_Impl *d = new Am_Drawonable_Impl(
      l, t, w, h, window_name, icon_name, vis, initially_iconified, back_color,
      save_under_flag, min_w, min_h, max_w, max_h, title_bar_flag,
      clip_by_children_flag, screen->depth, evh);

  d->owner = this;

  //copy all the global xlib properties
  d->screen = screen;
  screen->Note_Reference();

  // initialize for event handling
  d->Initialize_Event_Mask(); // must come before call to generate_attrib

  // Create window attribute struct and value-mask for attribute fields
  //
  XSetWindowAttributes attrib;
  unsigned long attrib_value_mask;
  install_attributes(&attrib, &attrib_value_mask, d, screen, back_color,
                     save_under_flag, title_bar_flag, d->current_event_mask);

  d->Create_X_Drawable(this, query_user_for_position, query_user_for_size,
                       &attrib, attrib_value_mask);

  if (vis)
    d->Map_And_Wait((query_user_for_position == false) &&
                    (query_user_for_size == false) &&
                    (initially_iconified == false));
  return d;
}

Screen_Desc::~Screen_Desc()
{
  XFreeGC(display, gc);
  clip_region->Destroy();
  Am_Font_Data::remove(display);
  Am_Cursor_Data::remove(display);
  Am_Image_Array_Data::remove(display);
  Am_Style_Data::remove(display);
  XCloseDisplay(display);
}

void
Am_Drawonable_Impl::Destroy()
{
  if (offscreen)
    XFreePixmap(screen->display, xlib_drawable);
  else {
    Set_Drawable_Backpointer(screen->display, xlib_drawable, (0L));
    XDestroyWindow(screen->display, xlib_drawable);
    Scrn_Mgr.Remove(this); // ought to check if a root drawonable
  }
  Flush_Output();
  screen->Release();
  delete this;
}

// // // // // // // // // // // // // // // // // // // //
//
//   Setting Window Properties
//
// // // // // // // // // // // // // // // // // // // //

XSizeHints
get_xsizehints(Am_Drawonable_Impl *d)
{
  long the_flags;
  XSizeHints the_sizehints;
  if (!(XGetWMNormalHints(d->screen->display, d->xlib_drawable, &the_sizehints,
                          &the_flags)))
    Am_Error("** get_xsizehints: failure retrieving XSizeHints.\n");
  return the_sizehints;
}

void
Am_Drawonable_Impl::Set_Title(const char *new_title)
{
  title = new char[strlen(new_title) + 1];
  strcpy(title, new_title);
  XTextProperty the_xtp;
  XStringListToTextProperty(&title, 1, &the_xtp);
  XSetWMName(screen->display, xlib_drawable, &the_xtp);
  XFree((char *)the_xtp.value);
}

void
Am_Drawonable_Impl::Set_Icon_Title(const char *new_title)
{
  icon_name = new char[strlen(new_title) + 1];
  strcpy(icon_name, new_title);
  XTextProperty the_xtp;
  XStringListToTextProperty(&icon_name, 1, &the_xtp);
  XSetWMIconName(screen->display, xlib_drawable, &the_xtp);
  XFree((char *)the_xtp.value);
}

void
Am_Drawonable_Impl::Set_Position(int new_left, int new_top)
{
  if (left == new_left && top == new_top)
    return;
  left = new_left;
  top = new_top;
  if (offscreen)
    return;
  XMoveWindow(screen->display, xlib_drawable, new_left, new_top);

  /* BAM removed 6/12/97 -- why is this here?
     int lb, tb, rb, bb, outer_left, outer_top;
     if (Inquire_Window_Borders (lb, tb, rb, bb, outer_left, outer_top)) {
       XMoveWindow(screen->display, xlib_drawable, new_left+lb, new_top+tb);
       // BUG: should sync when Process_Event is called
       // XSync (screen->display, False);
     }
  */
}

// Note: Set_Size() intercepts invalid size requests and clips the new width
// and height requests to the current max/min values stored in the
// Am_Drawonable.
void
Am_Drawonable_Impl::Set_Size(unsigned int req_width, unsigned int req_height)
{
  unsigned int new_width, new_height;
  if (max_width)
    new_width = MIN((MAX(req_width, min_width)), max_width);
  else
    new_width = MAX(req_width, min_width);
  if (max_height)
    new_height = MIN((MAX(req_height, min_height)), max_height);
  else
    new_height = MAX(req_height, min_height);
  if ((width != new_width) || (height != new_height)) {
    width = new_width;
    height = new_height;
    if (offscreen) {
      Window temp_drawable = xlib_drawable;
      this->Create_Offscreen_X_Drawable();
      XCopyArea(screen->display, temp_drawable, xlib_drawable, screen->gc, 0, 0,
                width, height, 0, 0);
      XFreePixmap(screen->display, temp_drawable);
    } else
      XResizeWindow(screen->display, xlib_drawable, width, height);
    ///     std::cout << "Set size:         " << (void*)this << " " << width << " " << height <<std::endl;
  }
  //// Probably not needed.
  //  Flush_Output();
  //// BUG: should sync when Process_Event is called
  //  XSync (screen->display, False);
}

void
Am_Drawonable_Impl::Set_Min_Size(unsigned int min_w, unsigned int min_h)
{
  // A window of zero width or height is illegal under X, so make it at
  // least one pixel in each dimension.
  if (min_w < 1)
    min_w = 1;
  if (min_h < 1)
    min_h = 1;

  // Set internal Am_Drawonable data
  min_width = min_w;
  min_height = min_h;

  // Retrieve drawable's XSizeHints structure
  XSizeHints the_sizehints = get_xsizehints(this);

  // Install new values in struct, and install new struct in drawable
  the_sizehints.flags |= PMinSize;
  the_sizehints.min_width = min_w;
  the_sizehints.min_height = min_h;
  XSetWMNormalHints(screen->display, xlib_drawable, &the_sizehints);

  /* Logical problem: the following code fails if the new min is larger
	than the old max, because the Set_Size request will get clipped
	to the old max.

	// Call Set_Size to enlarge window if appropriate
	bool need_to_set_size = false;
	unsigned int new_width = width;
	unsigned int new_height = height;
	if (min_w > width)  { need_to_set_size = true;  new_width = min_w; }
	if (min_h > height) { need_to_set_size = true;  new_height = min_h; }
	if (need_to_set_size)
	Set_Size(new_width, new_height);
	*/
}

void
Am_Drawonable_Impl::Set_Max_Size(unsigned int max_w, unsigned int max_h)
{

  // Set internal Am_Drawonable data
  max_width = max_w;
  max_height = max_h;

  // Retrieve drawable's XSizeHints structure
  XSizeHints the_sizehints = get_xsizehints(this);

  // Install new values in struct, and install new struct in drawable
  if ((max_w == 0) && (max_h == 0))
    // Then deactivate max -- BUG: Doesn't have any effect
    the_sizehints.flags &= ~PMaxSize;
  else
    the_sizehints.flags |= PMaxSize;
  the_sizehints.max_width = max_w ? max_w : 30000;
  the_sizehints.max_height = max_h ? max_h : 30000;

  XSetWMNormalHints(screen->display, xlib_drawable, &the_sizehints);
}

void
Am_Drawonable_Impl::Set_Visible(bool vis)
{
  if (offscreen)
    return; // not meaningful for offscreen pixmap
  visible = vis;

  if (vis)
    //    Map_And_Wait((ask_position == false) && (ask_size == false));
    Map_And_Wait(false);
  else
    Unmap_And_Wait();
}

void
Am_Drawonable_Impl::Set_Titlebar(bool new_title_bar)
{
  bool old_title_bar = title_bar;

  // The default for new_width should be whatever the old_width was.
  title_bar = new_title_bar;
  border_width = title_bar ? 2 : 0;

  // If the window is already mapnped, then we have to unmap and map it
  // again for the change to take effect.  After all, the override_redirect
  // attribute tells the window manager not to add a title bar as the window
  // is mapped.
  bool need_to_remap = (visible && (old_title_bar != new_title_bar));
  if (need_to_remap)
    Set_Visible(false);

  // Data structure for setting the override_redirect attribute.  Have to
  // build a whole new XSetWindowAttributes structure out of stored data.
  // (Would querying the xlib_drawable with XGetWindowAttributes and changing
  // one attribute be more desirable?  Note that the structures used in
  // XGet/XChangeWindowAttributes are incompatible, and you'd have to create
  // a new structure anyway...)
  XSetWindowAttributes attrib;
  unsigned long attrib_value_mask;
  install_attributes(&attrib, &attrib_value_mask, this, screen,
                     background_color, save_under, new_title_bar,
                     current_event_mask);

  XSetWindowBorderWidth(screen->display, xlib_drawable, border_width);
  XChangeWindowAttributes(screen->display, xlib_drawable, attrib_value_mask,
                          &attrib);

  if (need_to_remap)
    Set_Visible(true); // does Flush_Output() when window was visible;
}

// This function clears the window with the new background color and
// generates an expose event.  The clip mask is ignored as the window
// is cleared with the new background color.
//
void
Am_Drawonable_Impl::Set_Background_Color(Am_Style new_color)
{
  background_color = new_color;
  if (offscreen) {
    // want to fill the pixmap with background?
    return;
  }
  Am_Style_Data *n_color = (Am_Style_Data::Narrow(new_color));
  XSetWindowBackground(screen->display, xlib_drawable,
                       n_color->Get_X_Index(this));
  if (n_color)
    n_color->Release();
  XClearArea(screen->display, xlib_drawable, 0, 0, 0, 0, true);
  Flush_Output();
}

void
Am_Drawonable_Impl::Reparent(Am_Drawonable *new_parent)
{
  // You cannot move a window to/from the top-level without unmapping
  // and remapping it.
  bool need_to_remap = (visible);
  if (need_to_remap)
    Set_Visible(false);

  XReparentWindow(screen->display, xlib_drawable,
                  ((Am_Drawonable_Impl *)new_parent)->xlib_drawable, left, top);
  owner = new_parent;
  if (need_to_remap)
    Set_Visible(true); // Does Flush_Output() when window was visible and
                       // title_bar property changed.
}

void
Am_Drawonable_Impl::Map_And_Wait(bool wait)
{
  expect_map_change = true;
  XMapWindow(screen->display, xlib_drawable);
  Flush_Output();
  if (wait)
    wait_for_mapnotify(screen->display, &xlib_drawable);
}

// Note: It is insufficient to go into a loop and wait for an UnmapNotify
// event to arrive.  For some reason (maybe an X bug), you can unmap
// the window, receive the UnmapNotify event, and then try and fail to
// map the window again (you have to perform the succession of unmap/map
// calls quickly to see the bug).  Instead of relying on the UnmapNotify
// event, this function unmaps the window and then asks X for its map state
// via the XGetWindowAttributes call.  As discussed on p. 80, XLIB Vol 1,
// this function will also flush the request buffer, and has the effect of
// waiting for the unmap request to be fully processed.
void
Am_Drawonable_Impl::Unmap_And_Wait()
{
  static XWindowAttributes attribs;
  expect_map_change = true;
  XUnmapWindow(screen->display, xlib_drawable);
  Flush_Output();
  // Call XGetWindowAttributes instead of waiting for the UnmapNotify event
  XGetWindowAttributes(screen->display, xlib_drawable, &attribs);
}

// Note: to translate to screen coordinates, invoke this method on the
// root drawonable.
//
void
Am_Drawonable_Impl::Translate_Coordinates(int src_x, int src_y,
                                          Am_Drawonable *src_d,
                                          int &dest_x_return,
                                          int &dest_y_return)
{
  Window dummy_child_return;

  if (!(src_d))
    Am_Error("** Translate_Coordinates: source not supplied.\n");
  if (screen->display != ((Am_Drawonable_Impl *)src_d)->screen->display) {
    std::cerr << "** Translate_Coordinates: THIS " << this << " and src_d "
              << src_d << " have different displays" << std::endl;
    Am_Error();
  }
  if (!(XTranslateCoordinates(screen->display,
                              ((Am_Drawonable_Impl *)src_d)->xlib_drawable,
                              xlib_drawable, src_x, src_y, &dest_x_return,
                              &dest_y_return, &dummy_child_return))) {
    std::cerr << "** Translate_Coordinates: THIS " << this << " and src_d "
              << src_d << " have different screens" << std::endl;
    Am_Error();
  }
}

void
Am_Drawonable_Impl::Flush_Output()
{
  XSync(screen->display, false);
  //    std::flush(screen->display); //is this the right function?
}

// // // // // // // // // // // // // // // // // // // //
//
//   Value retrieval functions
//
// // // // // // // // // // // // // // // // // // // //

// This function does not initialize the left, top, and border values --
// it only adjusts them if it recognizes that the outer values are different
// than the inner values.  The parameters should already have been initialized
// to the "inner" values before calling this function.
//
// Known bugs:  assumes right and bottom borders are the same as left
// border.

bool
Am_Drawonable_Impl::add_wm_border_offset(Window query_drawable,
                                         Window expected_parent, int &lb,
                                         int &tb, int &rb, int &bb,
                                         int &outer_left, int &outer_top)
{

  // Ask X for the parent of 'query_drawable' -- store value in xlib_parent
  Window root_return, xlib_parent, *children_return;
  unsigned int nchildren_return;
  XQueryTree(screen->display, query_drawable, &root_return, &xlib_parent,
             &children_return, &nchildren_return);

  // don't need list of children, so free it right away
  if (children_return)
    XFree((char *)children_return);

  if (expected_parent == xlib_parent) {
    //std::cout << expected_parent << " == " << xlib_parent <<std::endl;

    // If the original window is supposed to have a title bar, and it is
    // at the top-level, but it has not yet been reparented to have a title
    // bar, then all this calculation is bogus.
    if (title_bar && (owner == screen->root) &&
        (query_drawable == xlib_drawable)) {
      //std::cout << "   parenting is messed up, returning false" <<std::endl;
      return false;
    }

    else {
      // done computing border widths (whatever was computed the last time
      // through the recursion is already correct) on MWM only!
      // TWM also uses bw_return here.
      if (xlib_parent == screen->root->xlib_drawable) {
        // We're dealing with a top-level window.  Find its actual outer vals.
        unsigned int w_return, h_return, bw_return, d_return;
        XGetGeometry(screen->display, query_drawable, &root_return, &outer_left,
                     &outer_top, &w_return, &h_return, &bw_return, &d_return);

        //     std::cout << "add_wm_borders: bw == " << bw_return
        //	<< ", query drawable == " << query_drawable <<std::endl;

        // In TWM, the borders are specified by this window's borderwidth.
        // Add this to the borders.  In MWM the values are 0 anyway.
        lb += bw_return;
        rb += bw_return;
        tb += bw_return;
        bb += bw_return;
      }
      //    std::cout << "  Outer: " << outer_left << ", " << outer_top << " " <<std::endl;

      return true;
    }
  } else {
    //std::cout << expected_parent << " != " << xlib_parent <<std::endl;
    if (xlib_parent) {
      // Return offset of current query_drawable drawable from its parent,
      // PLUS the offset of the parent, and set outer left and top during
      // recurson.
      int x_return, y_return;
      unsigned int w_return, h_return, bw_return, d_return;
      XGetGeometry(screen->display, query_drawable, &root_return, &x_return,
                   &y_return, &w_return, &h_return, &bw_return, &d_return);
      lb += x_return;
      tb += y_return;
      //     std::cout << "add_wm_borders: lb == " << lb << ", tb == " << tb
      //	   << ", bw == " << bw_return << ", query drawable == "
      //           << query_drawable <<std::endl;
      rb = lb;
      bb = lb;
      // At this point, expected parent must be the root, because there
      // is never decoration on subwindows.
      return (add_wm_border_offset(xlib_parent, expected_parent, lb, tb, rb, bb,
                                   outer_left, outer_top));
    } else {
      // (0L) xlib_parent means query_window has no parent now, which
      // is probably due to some race condition.  Can't return valid
      // data, so return false.
      //std::cout << "   (0L) parent, returning false" <<std::endl;
      return false;
    }
  }
}

bool
Am_Drawonable_Impl::Inquire_Window_Borders(int &left_border, int &top_border,
                                           int &right_border,
                                           int &bottom_border, int &outer_left,
                                           int &outer_top)
{
  left_border = top_border = right_border = bottom_border = 0;
  // By "default", outer_left and outer_top will be the same as the inner
  // left and top.  The subsequent call to add_wm_border_offset() will
  // correct this if necessary.
  outer_left = left;
  outer_top = top;

  if (xlib_drawable) {
    // Ask Gem for the parent of 'this' -- store value in expected_parent
    Window expected_parent = ((Am_Drawonable_Impl *)owner)->xlib_drawable;
    // If the xlib_drawable has decoration, this function will walk up the
    // drawable's hierarchy to the root, adding border offsets.  If it has
    // no decoration, it will return immediately and we will return the
    // zero border widths.
    // Return true or false depending on whether the parenting data is
    // internally consistent.
    return (add_wm_border_offset(xlib_drawable, expected_parent, left_border,
                                 top_border, right_border, bottom_border,
                                 outer_left, outer_top));
  }

  else {
    std::cout << "** Asking for border-width of a window before it has"
              << std::endl;
    std::cout << "** an xlib_drawable not implemented yet.  Guessing..."
              << std::endl;

    if (title_bar && (owner == screen->root)) {
      left_border = 11;
      top_border = 27;
      right_border = 11;
      bottom_border = 11;
    } else {
      left_border = top_border = right_border = bottom_border = 0;
    }
    return true;
  }
}

// // // // // // // // // // // // // // // // // // // //
//
//   Internal functions for backpointers from Drawable to Am_Drawonable
//
// // // // // // // // // // // // // // // // // // // //

XContext Backpointer = XUniqueContext();

Am_Drawonable_Impl *
Get_Drawable_Backpointer(Display *dpy, Window xlib_window)
{
  XPointer data_return;
  if (XFindContext(dpy, xlib_window, Backpointer, &data_return))
    return (0L); //XFindContext returns non-zero error codes
  else
    return (Am_Drawonable_Impl *)data_return;
}

void
Set_Drawable_Backpointer(Display *dpy, Window xlib_window,
                         Am_Drawonable_Impl *d)
{
  XSaveContext(dpy, xlib_window, Backpointer, (XPointer)d);
}

void
Am_Drawonable_Impl::Print(std::ostream &os) const
{
  os << this->Get_Title();
  os << " (" << std::hex << (void *)this << std::dec << ")";
}

// // // // // // // // // // // // // // // // // // // //
//
// All the NIY functions
//
// // // // // // // // // // // // // // // // // // // //

Am_Drawonable *
Am_Drawonable_Impl::Create_Offscreen(int width, int height,
                                     Am_Style background_color)
{
  Am_Drawonable_Impl *d = new Am_Drawonable_Impl(
      0, 0, width, height, "", "", false, false, background_color, false, 0, 0,
      0, 0, false, true, screen->depth, (0L));
  d->owner = this;
  d->offscreen = true;

  //copy all the global xlib properties
  d->screen = screen;
  screen->Note_Reference();

  // initialize for event handling
  //    d->Initialize_Event_Mask(); // must come before call to generate_attrib

  d->Create_Offscreen_X_Drawable();

  return d;
}

void
Am_Drawonable_Impl::Raise_Window(Am_Drawonable *target_d)
{
  XWindowChanges values;
  unsigned int value_mask;

  if (target_d) {
    // Then place "this" window just above target_d
    value_mask = (CWSibling | CWStackMode);
    values.sibling = ((Am_Drawonable_Impl *)target_d)->xlib_drawable;
    values.stack_mode = Above;
  } else {
    // Then place "this" window at the top of the stack
    value_mask = CWStackMode;
    values.stack_mode = Above;
  }
  XReconfigureWMWindow(screen->display, xlib_drawable, screen->screen_number,
                       value_mask, &values);
  Flush_Output();
}

void Am_Drawonable_Impl::Lower_Window(Am_Drawonable *target_d) //to bottom
{
  XWindowChanges values;
  unsigned int value_mask;

  if (target_d) {
    // Then place "this" window just below target_d
    value_mask = (CWSibling | CWStackMode);
    values.sibling = ((Am_Drawonable_Impl *)target_d)->xlib_drawable;
    values.stack_mode = Below;
  } else {
    // Then place "this" window at the bottom of the stack
    value_mask = CWStackMode;
    values.stack_mode = Below;
  }
  XReconfigureWMWindow(screen->display, xlib_drawable, screen->screen_number,
                       value_mask, &values);
  Flush_Output();
}

void
Am_Drawonable_Impl::Set_Iconify(bool new_iconified)
{
  if (iconified != new_iconified) {
    iconified = new_iconified;
    if (new_iconified == true)
      if (!(XIconifyWindow(screen->display, xlib_drawable,
                           screen->screen_number)))
        Am_Error("** Couldn't iconify window.\n");
      else
        Flush_Output();
    else
      Map_And_Wait(true);
  }
}

void
Am_Drawonable_Impl::Beep()
{
  XBell(screen->display, 0);
}

// destination for bitblt is the Am_Drawonable this message is sent to
// Defaults:
//   df = Am_DRAW_COPY
//
void
Am_Drawonable_Impl::Bitblt(int d_left, int d_top, int width, int height,
                           Am_Drawonable *source, int s_left, int s_top,
                           Am_Draw_Function df)
{
  GC gc = screen->gc;
  Display *disp = screen->display;

  Window src_drawable = ((Am_Drawonable_Impl *)source)->xlib_drawable;
  Window dest_drawable = xlib_drawable;

  // a bit inefficient- only really need to change draw function.
  set_gc_using_fill(Am_No_Style, df);

  XCopyArea(disp, src_drawable, dest_drawable, gc, s_left, s_top, width, height,
            d_left, d_top);
}

// cut buffer, eventually, this needs to be generalized
void
Am_Drawonable_Impl::Set_Cut_Buffer(const char *s)
{
  // X says that if you have to use cut buffers (instead of selections,
  // which are complicated and annoying), then you should do this:
  // First, make sure all 8 buffers exist by concatting 0 length data
  // to them.  We do this in get_root_drawonable when we first make
  // the display.
  // Second, when setting the buffer, rotate the buffers by 1 first.
  if (!s || offscreen)
    return; // do nothing with a (0L) string for now.

  XRotateBuffers(screen->display, 1);
  // +1 byte for the /0.
  XStoreBytes(screen->display, s, (strlen(s) + 1) * sizeof(char));

  /////
  // We want to use selections as well, to be more ICCC compliant.
  // Set the string into our cut buffer property on this screen,
  // then grab the X selection.  The real work is done later when
  // we get selection request events.
  //  XChangeProperty(screen->display, screen->root->xlib_drawable,
  //		  screen->cut_buffer, XA_STRING, 8 /* data quantization */,
  //		  PropModeReplace, (const unsigned char*)s, strlen(s));

  if (screen->cut_data != (0L))
    delete[] screen->cut_data;
  screen->cut_data = new char[(strlen(s) + 1) * sizeof(char)];
  strcpy(screen->cut_data, s);

  // Use the time of the most recent input event- assume that event
  // triggered this cut.
  Time time = Am_Current_Input_Event
                  ? Am_Current_Input_Event->time_stamp
                  : CurrentTime - 1; // they say, don't use current time.
  XSetSelectionOwner(screen->display, XA_PRIMARY, xlib_drawable, time);
}

// Member functions for Screen_ Mgr class

Screen_Manager::Screen_Manager()
{
  head = (0L);
  FD_ZERO(&read_flag);
  nfds = 0;
}

Screen_Manager::~Screen_Manager()
{
  if (head != (0L)) {
    Screen_Info *screen = head;
    do {
      Screen_Info *next_screen = screen->next;
      delete screen;
      screen = next_screen;
    } while (screen != head);
  }
}

void
Screen_Manager::Add(const char *name, Display *dpy, Am_Drawonable *draw)
{
  Screen_Info *new_screen = new Screen_Info(name, dpy, draw);

  int fd = XConnectionNumber(dpy);
  FD_SET(fd, &read_flag);
  if (fd >= nfds)
    nfds = fd + 1;

  if (!head) {
    // empty list -- start it
    head = new_screen;
    new_screen->next = new_screen->prev = new_screen;
  } else {
    // add new_screen just before head
    Screen_Info *tail = head->prev;

    head->prev = new_screen;
    new_screen->next = head;

    tail->next = new_screen;
    new_screen->prev = tail;

    // preserve invariant that head->prev is last display that produced an
    // event
    head = new_screen;
  }
}

void
Screen_Manager::Remove(Screen_Manager::Screen_Info *screen)
{
  if (head == screen)
    head = (screen->next == screen) ? (0L) : screen->next;

  int fd = XConnectionNumber(screen->dpy);
  FD_CLR(fd, &read_flag);
  while (nfds > 0 && !FD_ISSET(nfds - 1, &read_flag))
    --nfds;

  screen->next->prev = screen->prev;
  screen->prev->next = screen->next;
  delete screen;
}

void
Screen_Manager::Remove(Am_Drawonable *draw)
{
  if (!head)
    return;

  Screen_Info *curr = head;
  do {
    if (draw == curr->draw) {
      Remove(curr);
      return;
    }
    curr = curr->next;
  } while (curr != head);
}

void
Screen_Manager::Remove(const char *name)
{
  if (!head)
    return; // no screens yet, nothing to do!

  Screen_Info *screen = head;
  do {
    if (!name || !screen->name) {
      if (name == screen->name) {
        Remove(screen);
        return;
      }
    } else if (!strcmp(name, screen->name)) {
      Remove(screen);
      return;
    }
    screen = screen->next;
  } while (screen != head);
}

bool
Screen_Manager::Get(const char *name, Display *&dpy, Am_Drawonable *&draw)
{
  if (!head)
    return false;

  Screen_Info *screen = head;
  do {
    if (!screen->name || !name) {
      if (screen->name == name) {
        dpy = screen->dpy;
        draw = screen->draw;
        return true;
      }
    } else if (!strcmp(screen->name, name)) {
      dpy = screen->dpy;
      draw = screen->draw;
      return true;
    }
    screen = screen->next;
  } while (screen != head);

  return false;
}

Am_Drawonable *
Screen_Manager::Member(Display *dpy)
{
  if (!head)
    return (0L);
  Screen_Info *screen = head;
  do {
    if (screen->dpy == dpy)
      return screen->draw;
    screen = screen->next;
  } while (screen != head);

  return (0L);
}

void
Screen_Manager::Block(timeval *timeout)
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

  if (More_Than_One_Display && (timeout == NULL)) {
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
  int new_nfds = 0;

  if (Am_Connection::Active()) {
    new_nfds = Am_Connection::Merge_Mask(&read_Fd_Select);
    if (nfds > new_nfds)
      new_nfds = nfds;
  } else // No open connections
  {
    new_nfds = nfds;
  }

  // Return from Networking Hooks
  //////////////////////////////////////////////////////////////
  status = select(new_nfds, &read_Fd_Select, (0L), NULL, timeout);
  if (status < 0)
    perror("Am_Error in Block");

  //////////////////////////////////////////////////////////////
  // One last Networking hook

  if (Am_Connection::Sockets_Have_Input(&read_Fd_Select))
    Am_Connection::Handle_Sockets(&read_Fd_Select);

  // End of Networking hooks.
  /////////////////////////////////////////////////////////////
}

bool
Screen_Manager::Pending(XEvent *event_return)
{
  Screen_Info *screen = head;
  do {
    if (XPending(screen->dpy)) {
      if (event_return) {
        XNextEvent(screen->dpy, event_return);
        head = screen->next;
      }
      return true;
    }
    screen = screen->next;
  } while (screen != head);

  return false;
}

void
Screen_Manager::Wait_For_Event(timeval *timeout)
{
  while (!Pending())
    Block(timeout);
}

void
Screen_Manager::Next_Event(XEvent *event_return, timeval *timeout)
{
  //  while (!Pending (event_return))
  //   Block(timeout);
  //   Pending(event_return);
  if (!Pending(event_return)) {
    Block(timeout);
    Pending(event_return);
  }
}

void
Screen_Manager::Put_Event_Back(XEvent &event)
{
  XPutBackEvent(head->prev->dpy, &event);
}

// // // // // // // // // // // // // // // // // // // //
//
//   To make a drawonable associated with another application
//
// // // // // // // // // // // // // // // // // // // //

/*

class Am_External_Drawonable_Impl : public Am_Drawonable_Impl {
public:
  //constructor function
  Am_External_Drawonable_Impl (int l, int t, unsigned int w, unsigned int h,
		      const char* tit, const char* icon_tit, bool vis,
		      bool initially_iconified,
		      Am_Style back_color,
		      bool save_under_flag,
		      int min_w, int min_h, int max_w, int max_h,
		      bool title_bar_flag,
		      bool clip_by_children_flag,
		      unsigned int bit_depth,
		      Am_Input_Event_Handlers *evh)

  { left = l; top = t; width = w; height = h;
    title = new char[strlen(tit)+1]; strcpy (title, tit);
    icon_name = new char[strlen(icon_tit)+1]; strcpy (icon_name, icon_tit);
    visible = vis; iconified = initially_iconified;
    background_color = back_color; border_width = title_bar_flag?2:0;
    save_under = save_under_flag; min_width = min_w; min_height = min_h;
    max_width = max_w; max_height = max_h;
    title_bar = title_bar_flag;
    clip_by_children = clip_by_children_flag;  depth = bit_depth;
    event_handlers = evh;
    // initialize title bar height member
    offscreen = false;
  }

  ~Am_External_Drawonable_Impl ()
  {
    if (title) delete[] title;
    if (icon_name) delete[] icon_name;
  }
  void Clear_Area(int left, int top, int width, int height);
  bool Inquire_Window_Borders
    (int& left_border, int& top_border, int& right_border, int& bottom_border,
     int& outer_left, int& outer_top);

  //extra data
  Am_Drawonable_Impl* main_drawonable;
};

bool Am_External_Drawonable_Impl::Inquire_Window_Borders
    (int& left_border, int& top_border, int& right_border, int& bottom_border,
     int& outer_left, int& outer_top)
{
  left_border = top_border = right_border = bottom_border = 0;
  outer_left = left;  outer_top = top;

  Window other_xlib_drawable = main_drawonable->xlib_drawable;

  if (other_xlib_drawable) {
    // Ask Gem for the parent of 'this' -- store value in expected_parent
    Window expected_parent =
      ((Am_Drawonable_Impl*)(main_drawonable->owner))->xlib_drawable;
    // If the xlib_drawable has decoration, this function will walk up the
    // drawable's hierarchy to the root, adding border offsets.  If it has
    // no decoration, it will return immediately and we will return the
    // zero border widths.
    // Return true or false depending on whether the parenting data is
    // internally consistent.
    return (add_wm_border_offset (other_xlib_drawable,
				  expected_parent, left_border,
				  top_border, right_border, bottom_border,
				  outer_left, outer_top));
  }
}

void Am_External_Drawonable_Impl::Clear_Area(int left, int top, int width,
					     int height) {
 std::cout << this << " Not clearing area ("
       << left << "," << top << ","
       << width << "," << height << ")\n" <<std::flush;
}

*/

Am_Drawonable *
Am_Drawonable_Impl::Create_Drawonable_From_XWindow(
    Window created_drawable, Display *created_display,
    Am_External_Event_Handler *ext_ev_handler, const char *tit,
    const char *icon_tit, bool vis, bool initially_iconified,
    Am_Style back_color, bool save_under_flag, int min_w, int min_h, int max_w,
    int max_h, bool title_bar_flag, bool query_user_for_position,
    bool query_user_for_size, bool clip_by_children_flag,
    Am_Input_Event_Handlers *evh)
{

  Window p, r;
  Window *c;
  unsigned int nc;
  XQueryTree(created_display, created_drawable, &r, &p, &c, &nc);
  if (c)
    XFree(c);

  Am_Drawonable_Impl *parent = Get_Drawable_Backpointer(created_display, p);
  if (!parent) {
    parent = (Am_Drawonable_Impl *)Scrn_Mgr.Member(created_display);
    if (!parent) {
      parent =
          (Am_Drawonable_Impl *)make_root_drawonable((0L), created_display);
      std::cerr << "** Warning, Can't find parent for display "
                << created_display << " so created new one " << parent
                << std::endl
                << std::flush;
    }
  }

  Window dummy_root;
  int l, t;
  unsigned int w, h;
  unsigned int border_w;
  unsigned int bit_depth;
  if (!XGetGeometry(created_display, created_drawable, &dummy_root, &l, &t, &w,
                    &h, &border_w, &bit_depth))
    Am_Error("** Can't get window geometry.\n");

  Am_Drawonable_Impl *d = new /* Am_External_Drawonable_Impl */
      Am_Drawonable_Impl(l, t, w, h, (char *)tit, (char *)icon_tit, vis,
                         initially_iconified, back_color, save_under_flag,
                         min_w, min_h, max_w, max_h, title_bar_flag,
                         clip_by_children_flag, parent->screen->depth, evh);

  d->owner = parent;
  d->screen = parent->screen;
  d->ext_handler = ext_ev_handler;

  parent->screen->Note_Reference();

  // initialize for event handling
  d->Initialize_Event_Mask(); // must come before call to generate_attrib

  // Create window attribute struct and value-mask for attribute fields
  //
  XSetWindowAttributes attrib;
  unsigned long attrib_value_mask;
  install_attributes(&attrib, &attrib_value_mask, d, d->screen, back_color,
                     save_under_flag, title_bar_flag, d->current_event_mask);

  d->xlib_drawable = created_drawable;

  XChangeWindowAttributes(created_display, created_drawable, attrib_value_mask,
                          &attrib);
  Set_Drawable_Backpointer(created_display, created_drawable, d);

  set_other_window_properties(d->xlib_drawable, d->screen->display, (char *)tit,
                              (char *)icon_tit, initially_iconified, min_w,
                              min_h, max_w, max_h, query_user_for_position,
                              query_user_for_size);
  return d;
}

Am_Drawonable *
Am_Drawonable_Impl::Create_Offscreen_Drawonable_From_XWindow(
    Window created_drawable, Display *created_display,
    Am_External_Event_Handler *ext_ev_handler, Am_Style back_color)
{
  Window dummy_root;
  int l, t;
  unsigned int w, h;
  unsigned int border_w;
  unsigned int bit_depth;
  if (!XGetGeometry(created_display, created_drawable, &dummy_root, &l, &t, &w,
                    &h, &border_w, &bit_depth))
    Am_Error("** Can't get window geometry.\n");

  //copy all the global xlib properties
  Am_Drawonable *parent_draw = Scrn_Mgr.Member(created_display);
  if (!parent_draw) {
    parent_draw = make_root_drawonable((0L), created_display);
    std::cerr << "** Warning, Can't find parent for display " << created_display
              << " so created new one " << parent_draw << std::endl
              << std::flush;
  }

  Am_Drawonable_Impl *parent = (Am_Drawonable_Impl *)parent_draw;

  Am_Drawonable_Impl *d = new /* Am_External_Drawonable_Impl */
      Am_Drawonable_Impl(0, 0, w, h, "", "", false, false, back_color, false, 0,
                         0, 0, 0, false, true, parent->screen->depth, (0L));
  d->offscreen = true;
  d->owner = parent;
  d->screen = parent->screen;
  d->ext_handler = ext_ev_handler;

  parent->screen->Note_Reference();

  d->xlib_drawable = created_drawable;

  /*  no attributes if offscreen??

  // Create window attribute struct and value-mask for attribute fields
  //
  XSetWindowAttributes attrib;
  unsigned long attrib_value_mask;
  install_attributes(&attrib, &attrib_value_mask,
		       d, d->screen, back_color,
		       false, false,
		       d->current_event_mask);

  XChangeWindowAttributes(created_display, created_drawable,
			  attrib_value_mask, &attrib);
*/

  Set_Drawable_Backpointer(created_display, created_drawable, d);

  return d;
}
