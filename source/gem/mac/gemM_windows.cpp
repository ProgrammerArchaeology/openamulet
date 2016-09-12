/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file contains Am_Drawonable_Impl member functions for
   the Mac version of Gem

*/

#include <string.h>

#include <am_inc.h>
#include <assert.h>

#include IDEFS__H // For Am_Main_Loop_Go
#include GEM__H
#include <gemM.h>

// Static variables
GrafPtr Am_Drawonable_Impl::util_port = (0L);
Am_Drawonable *current_focus = (0L);

/* Prototypes */
void set_window_pos(Am_Drawonable *d, WindowRef behind_window);
void create_mac_window(Am_Drawonable_Impl *d);
void change_childrens_windows(child *children, WindowRef new_win);
void update_subwins(Am_Drawonable_Impl *d, int left, int top, int width,
                    int height);

extern PixMapHandle lock_current_pixmap();
extern void unlock_current_pixmap(PixMapHandle);

/*******************************************************************************
 * set_window_pos
 */

void
set_window_pos(Am_Drawonable *d, WindowRef behind_window)
{
  WindowRef mac_window = ((Am_Drawonable_Impl *)d)->mac_window;
  if (behind_window == (0L)) {
    bool has_title_bar;
    d->Get_Titlebar(has_title_bar);

    if (has_title_bar)
      SelectWindow(mac_window);
    else
      BringToFront(mac_window);
  } else {
    SendBehind(mac_window, behind_window);

    RgnHandle structureRgn = NewRgn();
    GetWindowStructureRgn(mac_window, structureRgn);
    PaintOne(mac_window, structureRgn);
    CalcVis(mac_window);

    DisposeRgn(structureRgn);
  }
}

// // // // // // // // // // // // // // // // // // // // // // //
//
//  Low-level functions for subwindows:  The Mac does not have built-in
//  functionality for subwindows, so we have to associate sets of drawonables
//  with their top-level Mac window.  Drawing in a subwindow involves
//  translating the origin of the top-level window to the origin of the
//  subwindow, then clipping drawing to the dimensions of the subwindow.
//
//  Each drawonable has a list of children.  When drawonables are
//  reparented, the mac_window of all children of the drawonable must
//  be changed to point to the new parent.  Similarly, when a window is
//  exposed, the expose-event-handler must be invoked on all children.
//
// // // // // // // // // // // // // // // // // // // // // // //

// This function computes the total offset of the current subwindow from its
// top-level container.  Set x and y to zero before invoking this function,
// and they will be incremented by iterating over the containers.

/*******************************************************************************
 * add_total_offset
 */

void
Am_Drawonable_Impl::add_total_offset(int &x, int &y)
{
  if (owner != (0L)) {
    if (((Am_Drawonable_Impl *)owner)->owner) {
      x -= left;
      y -= top;
      ((Am_Drawonable_Impl *)owner)->add_total_offset(x, y);
    }
  }
}

/*******************************************************************************
 * add_total_offset_global
 */

void
Am_Drawonable_Impl::add_total_offset_global(int &x, int &y)
{
  Am_Drawonable_Impl *scan = this;
  while (scan) {
    if (scan->owner != (0L)) {
      x -= scan->left;
      y -= scan->top;
    }
    scan = (Am_Drawonable_Impl *)scan->owner;
  }
}

/*******************************************************************************
 * add_child
 */

void
Am_Drawonable_Impl::add_child(Am_Drawonable_Impl *d)
{
  child *old_last_child = last_child;
  last_child = new child;
  last_child->d = d;
  last_child->next = (0L);

  if (old_last_child)
    old_last_child->next = last_child;
  else
    children = last_child;
}

/*******************************************************************************
 * delete_child
 *   Delete d from the list of this's children. The function assumes
 *   that it will find d in the list of children (no special error checking
 *   when d is not found).
 */

void
Am_Drawonable_Impl::delete_child(Am_Drawonable_Impl *d)
{
  child *curr = children;
  child *prev = (0L);
  // Iterate over children
  while (curr) {
    if (curr->d == d) {
      if (prev)
        prev->next = curr->next;
      else
        children = curr->next;
      delete curr;
      break;
    } //if
    prev = curr;
    curr = curr->next;
  } // while
}

/*******************************************************************************
 * add_total_offset_global
 */

void
update_subwins(Am_Drawonable_Impl *d, int left, int top, int width, int height)
{
  if (d->event_handlers)
    d->event_handlers->Exposure_Notify(d, left, top, width, height);

  child *scan = d->children;
  while (scan) {
    int x = left;
    int y = top;
    int xOffset, yOffset;
    scan->d->Get_Position(xOffset, yOffset);

    x -= xOffset;
    y -= yOffset;
    update_subwins(scan->d, x, y, width, height);

    scan = scan->next;
  }
}

/*******************************************************************************
 * get_upper_left
 */

void
Am_Drawonable_Impl::get_upper_left(int &x, int &y)
{
  x = 0;
  y = 0;
  add_total_offset(x, y);
  x = -x;
  y = -y;
}

/*******************************************************************************
 * calc_exposed_rgn
 */

void
Am_Drawonable_Impl::calc_exposed_rgn()
{
  int x, y;
  get_upper_left(x, y);

  if (Get_Visible() && !Get_Iconify()) {
    SetRectRgn(exposed_rgn, x, y, x + width, y + height);
    if (is_toplevel_window() && title_bar == true) {
      static RgnHandle grow_rgn = NewRgn();
      SetRectRgn(grow_rgn, x + width - 15, y + height - 15, x + width,
                 y + height);
      DiffRgn(exposed_rgn, grow_rgn, exposed_rgn);
    }
    RgnHandle owner_rgn = ((Am_Drawonable_Impl *)owner)->exposed_rgn;
    if (owner_rgn != (0L))
      SectRgn(exposed_rgn, owner_rgn, exposed_rgn);
  } else
    SetRectRgn(exposed_rgn, 0, 0, 0, 0);
  subtract_child_windows(children);
}

/*******************************************************************************
 * subtract_child_windows
 */

void
Am_Drawonable_Impl::subtract_child_windows(child *children)
{
  child *scan = children;
  while (scan) {
    Am_Drawonable_Impl *subwindow = (Am_Drawonable_Impl *)scan->d;
    subwindow->calc_exposed_rgn();
    DiffRgn(exposed_rgn, subwindow->exposed_rgn, exposed_rgn);

    scan = scan->next;
  }
}

/*******************************************************************************
 * focus_on_this
 *   Invoke focus_on_this() to direct future drawing to this drawonable.
 */

void
Am_Drawonable_Impl::focus_on_this()
{
#ifdef DEBUG
  if (!mac_port) {
    Am_Error("mac_port not set for Am_Drawonable_Impl::focus_on_this");
    return;
  }
#endif

  RgnHandle window_rgn = NewRgn();

  if (mac_port == (0L))
    return;

  // Direct future drawing to this drawonable
  if (qd.thePort != (GrafPtr)mac_port)
    SetPort((GrafPtr)mac_port);

  // Assuming this is a subwindow, compute its origin's offset from the
  // top-most parent's origin, and translate future drawing
  int x_offset = 0;
  int y_offset = 0;
  add_total_offset(x_offset, y_offset);
  SetOrigin(x_offset, y_offset);

  // Assuming this is a subwindow, clip future drawing into this subwindow.
  // Must compute intersection of window's current clip-region with the
  // dimensions of this subwindow.

  Am_Drawonable_Impl *window_d = Get_Drawable_Backpointer(mac_window);
  if (window_d)
    window_d->calc_exposed_rgn();

  CopyRgn(exposed_rgn, window_rgn);
  OffsetRgn(window_rgn, x_offset, y_offset);
  RgnHandle current_clip_rgn = ((Am_Region_Impl *)clip_region)->region_to_use();
  if (current_clip_rgn) {
    SectRgn(window_rgn, current_clip_rgn, window_rgn);
  }
  SetClip(window_rgn);

  current_focus = this;

  DisposeRgn(window_rgn);
}

/*******************************************************************************
 * out_of_focus
 */

void
Am_Drawonable_Impl::out_of_focus()
{
  if (this == current_focus)
    current_focus = (0L);
}

/*******************************************************************************
 *
 * Am_Drawonable creator functions
 *
 ******************************************************************************/

/*******************************************************************************
 * Get_Root_Drawonable
 */

Am_Drawonable *
Am_Drawonable::Get_Root_Drawonable(const char * /* screen */)
{
  int l, t;
  unsigned int w, h;
  unsigned int bit_depth;

  // Compute screen geometry
  RgnHandle grayRgn = GetGrayRgn();
  Rect desktopRect = (**grayRgn).rgnBBox;

  l = desktopRect.left;
  t = desktopRect.top;
  w = desktopRect.right - desktopRect.left;
  h = desktopRect.bottom - desktopRect.top;

  GDHandle maxDevice = GetMaxDevice(&desktopRect);
  bit_depth = (**(**maxDevice).gdPMap).pixelSize;

  Am_Drawonable_Impl *d =
      new Am_Drawonable_Impl(l, t, w, h, "", "", true, false, Am_No_Style,
                             false, 1, 1, 0, 0, true, false, bit_depth, (0L));

  d->owner = 0;
  d->exposed_rgn = (0L);
  Am_Initialize_Char_Map();

  return d;
}

/*******************************************************************************
 * create_mac_window
 */

const noBorderProc = 317 << 4;

void
Am_Drawonable_Impl::create_mac_window()
{
  // Install data in Mac-specific data structures
  //   Bounding-box of window...
  Rect windRect;
  SetRect(&windRect, left, top, left + width, top + height);
  c2pstr(title);
  // used Pascal routine here because the C version gave would not let me lock the
  // windows pixmap
  mac_window = NewCWindow(
      &mac_winrec, &windRect, (const unsigned char *)title,
      (visible && !iconified), (title_bar ? documentProc : noBorderProc),
      (WindowRef)-1, // draw window in front of all other windows
      true,          // draw a go-away box in the upper-left corner
      0L);
  p2cstr((StringPtr)title);
  is_toplevel = true;
  mac_port = GetWindowPort(mac_window);
#ifdef DEBUG
  // in Spotlight this will fail if the mac_port is invalid
  UpdateGWorld(&mac_port, 0, &(mac_port->portRect), nil, nil, keepLocal);
  SetGWorld(mac_port, nil);
  PixMapHandle pmh = lock_current_pixmap();
  unlock_current_pixmap(pmh);
#endif
  Set_Drawable_Backpointer(mac_window, this);
}

/*******************************************************************************
 * create_mac_offscreen
 */

void
Am_Drawonable_Impl::create_mac_offscreen()
{
  GWorldPtr theGWorld = (0L);
  Rect boundsRect;

  SetRect(&boundsRect, 0, 0, width, height);
  QDErr errCode =
      NewGWorld(&theGWorld, 0, &boundsRect, (0L), NULL, (GWorldFlags)0);

  mac_port = (CGrafPtr)theGWorld;
  mac_window = (0L);

  is_offscreen = true;
}

/*******************************************************************************
 * Create
 *   Defaults:
 *     l = 0,  t = 0,  w = 100,  h = 100
 *     window-name = "",
 *     icon_name = "",
 *     vis = true,
 *     initially_iconified = false,
 *     back_color = (0L),
 *     save_under_flag = false,
 *       can't have 0 size windows
 *     min_w = 1, min_h = 1,
 *       0 means no max
 *     max_w = 0, max_h = 0,
 *     title_bar_flag = true,
 *     query_user_for_position = false,
 *     query_user_for_size = false,
 *     clip_by_children_flag = true,
 *     evh = (0L);
 */

Am_Drawonable *
Am_Drawonable_Impl::Create(int l, int t, unsigned int w, unsigned int h,
                           const char *window_name, const char *icon_name,
                           bool vis, bool initially_iconified,
                           Am_Style back_color, bool save_under_flag, int min_w,
                           int min_h, int max_w, int max_h, bool title_bar_flag,
                           bool /* query_user_for_position */,
                           bool /* query_user_for_size */,
                           bool clip_by_children_flag,
                           Am_Input_Event_Handlers *evh)
{
  Am_Drawonable_Impl *d = new Am_Drawonable_Impl(
      l, t, w, h, window_name, icon_name, vis, initially_iconified, back_color,
      save_under_flag, min_w, min_h, max_w, max_h, title_bar_flag,
      clip_by_children_flag, depth, evh);

  d->owner = this;
  d->mac_cursor = &qd.arrow;

  if (owner == (0L))
    // d is a top-level drawonable, which needs its own window
    d->create_mac_window();
  else {
    // d will be a "subwindow" of this
    d->mac_window = mac_window;
    d->mac_port = mac_port;
  }

  d->exposed_rgn = NewRgn();
  SetRectRgn(d->exposed_rgn, d->left, d->top, d->left + d->width,
             d->top + d->height);

  d->want_enter_leave = false;
  d->want_multi_window = false;
  d->want_move = false;

  // BUG?: Must color background of window manually, but is this the place
  // to do it?
  d->Clear_Area(0, 0, w, h);

  add_child(d);
  return d;
}

/*******************************************************************************
 * Create_Offscreen
 */

Am_Drawonable *
Am_Drawonable_Impl::Create_Offscreen(int width, int height,
                                     Am_Style background_color)
{
  Am_Drawonable_Impl *d = new Am_Drawonable_Impl(
      0, 0, width, height, "", "", false, false, background_color, false, 0, 0,
      0, 0, false, true, depth, (0L));

  d->exposed_rgn = NewRgn();
  SetRectRgn(d->exposed_rgn, d->left, d->top, d->left + d->width,
             d->top + d->height);

  d->owner = this;
  d->create_mac_offscreen();

  return d;
}

/*******************************************************************************
 * Am_Drawonable_Impl
 *   constructor function
 */

Am_Drawonable_Impl::Am_Drawonable_Impl(
    int l, int t, unsigned int w, unsigned int h, const char *tit,
    const char *icon_tit, bool vis, bool initially_iconified,
    Am_Style back_color, bool save_under_flag, int min_w, int min_h, int max_w,
    int max_h, bool title_bar_flag, bool clip_by_children_flag,
    unsigned int bit_depth, Am_Input_Event_Handlers *evh)
{
  left = l;
  top = t;
  width = w;
  height = h;
  title = new char[strlen(tit) + 1];
  strcpy(title, tit);
  icon_name = new char[strlen(icon_tit) + 1];
  strcpy(icon_name, icon_tit);
  visible = vis;
  iconified = initially_iconified;
  background_color = back_color;
  border_width = title_bar_flag ? 2 : 0;
  save_under = save_under_flag;
  min_width = min_w;
  min_height = min_h;
  max_width = max_w;
  max_height = max_h;
  title_bar = title_bar_flag;
  clip_by_children = clip_by_children_flag;
  depth = bit_depth;
  event_handlers = evh;
  // initialize title bar height member?
  clip_region = Am_Region::Create();
  children = (0L);
  last_child = (0L);

  is_toplevel = false;
  is_offscreen = false;

  if (util_port == (0L)) {
    util_port = (GrafPtr)NewPtr(sizeof(GrafPort));
    OpenPort(util_port);
  }
}

/*******************************************************************************
 * Destroy
 */

void
Am_Drawonable_Impl::Destroy()
{
  if (is_toplevel_window() == true) {
    ((Am_Drawonable_Impl *)owner)->delete_child(this);

    Set_Drawable_Backpointer(mac_window, (0L));

  } else if (is_offscreen) {
    if (mac_port)
      DisposeGWorld((GWorldPtr)mac_port);
  }

  delete this;
}

/*******************************************************************************
 * ~Am_Drawonable_Impl
 */

Am_Drawonable_Impl::~Am_Drawonable_Impl()
{
  delete title;
  delete icon_name;
}

/*******************************************************************************
 * Set_Title
 */

void
Am_Drawonable_Impl::Set_Title(const char *new_title)
{
  delete[] title;
  title = new char[strlen(new_title) + 1];
  strcpy(title, new_title);
  if (is_toplevel_window() == true)
    setwtitle(mac_window, title);
}

/*******************************************************************************
 * Set_Position
 */

void
Am_Drawonable_Impl::Set_Position(int new_left, int new_top)
{
  if (is_toplevel_window() == true) {
    MoveWindow(mac_window, new_left, new_top, false);
    left = new_left;
    top = new_top;
  } else {
    // Invalidate sub window  area
    invalidate();
    left = new_left;
    top = new_top;
    invalidate();
  }
}

/*******************************************************************************
 * Set_Position
 *   Note: Set_Size() intercepts invalid size requests and clips the new width
 *   and height requests to the current max/min values stored in the
 *   Am_Drawonable.
 */

void
Am_Drawonable_Impl::Set_Size(unsigned int req_width, unsigned int req_height)
{
#ifdef DEBUG
  if (!mac_port) {
    Am_Error("mac_port not set for Am_Drawonable_Impl::Bitblt");
    return;
  }
#endif
  static Rect r;
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
    if (is_offscreen == true) // going to resize the offscreen GWorld
    {
      GWorldPtr theGWorld = (GWorldPtr)mac_port;
      Rect newBounds;
      SetRect(&newBounds, 0, 0, new_width, new_height);

      DisposeGWorld(theGWorld);
      NewGWorld(&theGWorld, 0, &newBounds, (0L), NULL, (GWorldFlags)0);
      mac_port = (CGrafPtr)theGWorld;
      /*
      LockPixels( GetGWorldPixMap( theGWorld ) );
      UpdateGWorld( &theGWorld, 0, &newBounds, (0L), NULL, clipPix );
      UnlockPixels( GetGWorldPixMap( theGWorld ) );
      mac_port = (CGrafPtr) theGWorld;
*/
      width = mac_port->portRect.right - mac_port->portRect.left;
      height = mac_port->portRect.bottom - mac_port->portRect.top;
      SetRectRgn(exposed_rgn, left, top, left + width, top + height);
    } else {
      invalidate();
      width = new_width;
      height = new_height;

      if (is_toplevel_window() == true)
        SizeWindow(mac_window, width, height, false);

      invalidate();
      out_of_focus();
    }
  }
}

/*******************************************************************************
 * Set_Min_Size
 */

void
Am_Drawonable_Impl::Set_Min_Size(unsigned int min_w, unsigned int min_h)
{
  // Set internal Am_Drawonable data
  min_width = min_w;
  min_height = min_h;
}

/*******************************************************************************
 * Set_Max_Size
 */

void
Am_Drawonable_Impl::Set_Max_Size(unsigned int max_w, unsigned int max_h)
{
  // Set internal Am_Drawonable data
  max_width = max_w;
  max_height = max_h;
}

/*******************************************************************************
 * Set_Visible
 */

void
Am_Drawonable_Impl::Set_Visible(bool vis)
{
  if (visible != vis) {
    visible = vis;
    if (is_toplevel_window() == true)
      // Does not raise, lower, or change active like Show/HideWindow would.
      ShowHide(mac_window, visible && !iconified);
    else
      invalidate();
    out_of_focus();
  }
}

/*******************************************************************************
 * Set_Titlebar
 */

void
Am_Drawonable_Impl::Set_Titlebar(bool new_title_bar)
{
  bool old_title_bar = title_bar;
  title_bar = new_title_bar;

  if (old_title_bar != new_title_bar) {
    if (is_toplevel_window() == true) {
      DisposeWindow(mac_window);
      create_mac_window();
    }
  }
}

/*******************************************************************************
 * Set_Titlebar
 *   This function clears the window with the new background color and
 *   generates an expose event.  The clip mask is ignored as the window
 *   is cleared with the new background color.
 */

void
Am_Drawonable_Impl::Set_Background_Color(Am_Style new_color)
{
  background_color = new_color;
  Am_Style_Data *n_color = (Am_Style_Data::Narrow(new_color));
  if (n_color)
    n_color->Release();

  Clear_Area(0, 0, width, height);
  invalidate();
}

/*******************************************************************************
 * Raise_Window
 */

void
Am_Drawonable_Impl::Raise_Window(Am_Drawonable *target_d)
{
  if (is_toplevel_window() == true) {
    WindowRef target_mac_window =
        target_d ? ((Am_Drawonable_Impl *)target_d)->mac_window : (0L);
    set_window_pos(this, target_mac_window);
  }
}

/*******************************************************************************
 * Lower_Window
 */

void
Am_Drawonable_Impl::Lower_Window(Am_Drawonable *target_d)
{
  if (is_toplevel_window() == true) {
    WindowRef target_mac_window =
        target_d ? ((Am_Drawonable_Impl *)target_d)->mac_window : (0L);
    set_window_pos(this, target_mac_window);
  }
}

/*******************************************************************************
 * Set_Iconify
 * Iconified state is treated just like visibility
 */

void
Am_Drawonable_Impl::Set_Iconify(bool new_iconified)
{
  if (iconified != new_iconified) {
    iconified = new_iconified;
    // Does not raise, lower, or change active like Show/HideWindow.
    if (is_toplevel_window() == true)
      ShowHide(mac_window, (visible && !iconified));
    else
      invalidate();

    out_of_focus();
  }
}

/*******************************************************************************
 * Bitblt
 *   destination for bitblt is the Am_Drawonable this message is sent to
 *   Defaults:
 *     df = Am_DRAW_COPY
 */

void
Am_Drawonable_Impl::Bitblt(int d_left, int d_top, int width, int height,
                           Am_Drawonable *source, int s_left, int s_top,
                           Am_Draw_Function /* df */)
{
#ifdef DEBUG
  if (!mac_port) {
    Am_Error("mac_port not set for Am_Drawonable_Impl::Bitblt");
    return;
  }
#endif
  focus_on_this();

  static Rect src_bounds, dest_bounds;
  SetRect(&src_bounds, s_left, s_top, s_left + width, s_top + height);
  SetRect(&dest_bounds, d_left, d_top, d_left + width, d_top + height);

  PixMapHandle src = ((Am_Drawonable_Impl *)source)->mac_port->portPixMap;
  PixMapHandle dest = mac_port->portPixMap;

  ForeColor(blackColor);
  BackColor(whiteColor);
  CopyBits((BitMap *)*src, (BitMap *)*dest, &src_bounds, &dest_bounds, srcCopy,
           nil);
}

/*******************************************************************************
 * Set_Icon_Title
 */

void
Am_Drawonable_Impl::Set_Icon_Title(const char *new_title)
{
  delete[] icon_name;
  icon_name = new char[strlen(new_title) + 1];
  strcpy(icon_name, new_title);
}

/*******************************************************************************
 * Translate_Coordinates
 *   Note: to translate to screen coordinates, invoke this method on the
 *   root drawonable.
 */

void
Am_Drawonable_Impl::Translate_Coordinates(int src_x, int src_y,
                                          Am_Drawonable * /* src_d */,
                                          int &dest_x_return,
                                          int &dest_y_return)
{
  dest_x_return = src_x;
  dest_y_return = src_y;
  add_total_offset_global(dest_x_return, dest_y_return);
}

/*******************************************************************************
 * Flush_Output
 */

void
Am_Drawonable_Impl::Flush_Output()
{
}

/*******************************************************************************
 *
 *   Value retrieval functions
 *
 ******************************************************************************/

/*******************************************************************************
 * Inquire_Window_Borders
 */

bool
Am_Drawonable_Impl::Inquire_Window_Borders(int &left_border, int &top_border,
                                           int &right_border,
                                           int &bottom_border, int &outer_left,
                                           int &outer_top)
{
  left_border = top_border = right_border = bottom_border = 0;
  outer_left = left;
  outer_top = top;
  return true;
}

/*******************************************************************************
 *
 *   Internal functions for backpointers from Mac window to Am_Drawonable
 *
 ******************************************************************************/

/*******************************************************************************
 * Get_Drawable_Backpointer
 */

Am_Drawonable_Impl *
Get_Drawable_Backpointer(WindowRef a_mac_win)
{
  if (!a_mac_win)
    return (0L);
  Am_Drawonable_Impl *d = (Am_Drawonable_Impl *)GetWRefCon(a_mac_win);
  // The refCon slot of any Mac window might be set arbitrarily --
  // check that interpreting the refCon value as a Drawonable makes sense.
  if (d && (d->mac_window == a_mac_win))
    return d;
  else
    return (0L);
}

/*******************************************************************************
 * Set_Drawable_Backpointer
 */

void
Set_Drawable_Backpointer(WindowRef a_mac_win, Am_Drawonable_Impl *d)
{
  SetWRefCon(a_mac_win, (long)d);
}

/*******************************************************************************
 * update_area
 */

void
Am_Drawonable_Impl::update_area(int x, int y, int width, int height)
{
  int left, top;
  get_upper_left(left, top);
  x += left;
  y += top;
  update_subwins(this, x, y, width, height);
}

/*******************************************************************************
 * invalidate
 */

void
Am_Drawonable_Impl::invalidate()
{
  if (is_offscreen)
    return;

  SetPort((GrafPtr)mac_port);
  SetOrigin(0, 0);

  int x, y;
  get_upper_left(x, y);

  Rect r;
  SetRect(&r, x, y, x + width, y + height);
  InvalRect(&r);
}

/*******************************************************************************
 * draw_grow_box
 */

void
Am_Drawonable_Impl::draw_grow_box()
{
  if (is_toplevel_window() && title_bar == true) {
    RgnHandle save_clip = NewRgn();
    GetClip(save_clip);

    Rect r;
    SetRect(&r, width - 15, height - 15, width, height);
    ClipRect(&r);
    DrawGrowIcon(mac_window);

    SetClip(save_clip);
    DisposeRgn(save_clip);
  }
}

/*******************************************************************************
 * get_global_bounds
 */

void
Am_Drawonable_Impl::get_global_bounds(Rect &bounds)
{
  int x = 0;
  int y = 0;
  add_total_offset_global(x, y);

  bounds.left = -x;
  bounds.top = -y;
  bounds.right = bounds.left + width;
  bounds.bottom = bounds.top + height;
}

/*******************************************************************************
 * Beep
 */

void
Am_Drawonable_Impl::Beep()
{
  SysBeep(10);
}

/*******************************************************************************
 * Set_Cut_Buffer
 */

void
Am_Drawonable_Impl::Set_Cut_Buffer(const char *s)
{
  ZeroScrap();
  int len = strlen(s);
  PutScrap(len, 'TEXT', (void *)s);
}

/*******************************************************************************
 * Get_Cut_Buffer
 */

char *
Am_Drawonable_Impl::Get_Cut_Buffer()
{
  long offset;
  Handle dataH = NewHandle(0);
  GetScrap(dataH, 'TEXT', &offset);

  int len = GetHandleSize(dataH);
  char *ret_data = new char[len + 1];
  BlockMoveData(*dataH, ret_data, len);
  DisposeHandle(dataH);

  ret_data[len] = '\0';

  return ret_data;
}

/*******************************************************************************
 * Print
 */

void
Am_Drawonable_Impl::Print(std::ostream &os) const
{
  os << this->Get_Title();
  os << " (" << std::hex << (void *)this << std::dec << ")";
}

/*******************************************************************************
 *
 * All the NIY functions
 *
 ******************************************************************************/

/*******************************************************************************
 * Reparent
 */

void
Am_Drawonable_Impl::Reparent(Am_Drawonable * /* new_parent */)
{
  /*  if (screen->root == owner) {
    if (screen->root == new_parent) {
      // An unusual case, where the old and new owners are both the root.
      // Don't do anything.
    } else {
      // Then 'this' is a top-level window, being demoted to a subwindow.
      // Close it's mac_window and set mac_window to point to new-parent's
      // mac_window.
      Set_Drawable_Backpointer (mac_window, (0L));
      DisposeWindow (mac_window);
      ((Am_Drawonable_Impl*)owner)->delete_child (this);
      mac_window = ((Am_Drawonable_Impl*)new_parent)->mac_window;

      // Invalidate mac_window so it will be redrawn
      if (qd.thePort != (GrafPtr)GetWindowPort(mac_window))
        SetPortWindowPort(mac_window);
      InvalRect (&(GetWindowPort(mac_window)->portRect));
      // Change mac_window pointers in all children
      change_childrens_windows (children, mac_window);
      owner = new_parent;
      ((Am_Drawonable_Impl*)new_parent)->add_child (this);
    }
  } else {
    if (screen->root == new_parent) {
      // Then 'this' is being promoted from a subwindow to a top-level window
      ((Am_Drawonable_Impl*)owner)->delete_child (this);
      create_mac_window ();
      // Invalidate mac_windows so they will be redrawn
      WindowRef old_win = ((Am_Drawonable_Impl*)owner)->mac_window;
      if (qd.thePort != (GrafPtr)GetWindowPort(old_win))
        SetPortWindowPort(old_win);
      InvalRect (&(GetWindowPort(old_win)->portRect));
      // Change mac_window pointers in all children
      change_childrens_windows (children, mac_window);
      owner = new_parent;
      ((Am_Drawonable_Impl*)new_parent)->add_child (this);
    } else {
      // Then 'this' is a subwindow, just changing owners
      ((Am_Drawonable_Impl*)owner)->delete_child (this);
      // Invalidate mac_windows so they will be redrawn
      WindowRef old_win = ((Am_Drawonable_Impl*)owner)->mac_window;
      WindowRef new_win = ((Am_Drawonable_Impl*)new_parent)->mac_window;
      if (qd.thePort != (GrafPtr)GetWindowPort(old_win))
        SetPortWindowPort(old_win);
      InvalRect (&(GetWindowPort(old_win)->portRect));
      // Change mac_window pointers in 'this' and all its children
      if (old_win != new_win) {
        if (qd.thePort != (GrafPtr)GetWindowPort(new_win))
          SetPortWindowPort(new_win);
        InvalRect (&(GetWindowPort(new_win)->portRect));

        mac_window = new_win;
        change_childrens_windows (children, new_win);
      }
      owner = new_parent;
      ((Am_Drawonable_Impl*)new_parent)->add_child (this);
    }
  }*/
}

/*******************************************************************************
 * change_childrens_windows
 *   A recursive function that changes the mac_window
 *   of all children of a drawonable.  The structure of this function is
 *   similar to how child windows are updated in gemM_input.cc.
 */

void
change_childrens_windows(child * /* children */, WindowRef /* new_win */)
{
  /*if (children != (0L)) {
    Am_Drawonable_Impl *d;

    // Perform operations on first child
    d = (Am_Drawonable_Impl*)(children->d);
    d->mac_window = new_win;
    // Perform operations on children of first child
    change_childrens_windows (d->children, new_win);
    // Perform operations on rest of children
    change_childrens_windows (children->next, new_win);
  }*/
}

/*******************************************************************************
 * this code locks pixels for drawing
 * but in one case we need to set the port without locking the pixels because 
 * the port is "bad", so we use SetCurrentPort.
 */

static DrawingPort *current_port =
    nil; // do not set this from outside these functions

DrawingPort::DrawingPort(Rect *portRect)
{
  pixH = (0L);
  offscreenGWorld = (0L);
  OSErr myErr = NewGWorld(&offscreenGWorld, 0, portRect, nil, nil, nil);
  if ((offscreenGWorld == nil) || (myErr != noErr)) {
    Am_ERROR("Cannot create offscreen for drawing");
    pixH = (0L);
    offscreenGWorld = (0L);
    return;
  }

  // port is always locked

  pixH = GetGWorldPixMap(offscreenGWorld);
  if (!LockPixels(pixH)) {
    Am_ERROR("Cannot lock mask for drawing");
    DisposeGWorld(offscreenGWorld);
    pixH = (0L);
    offscreenGWorld = (0L);
    return;
  }

  this->SetCurrentPort(); // always locked
}

DrawingPort::DrawingPort(WindowRef win)
{
  GetWindowPort(win);
  //???
}

DrawingPort::~DrawingPort()
{
  if (pixH && offscreenGWorld) {
    UnlockPixels(pixH);
    DisposeGWorld(offscreenGWorld);
  }
}

BitMap *
DrawingPort::GetBitMap()
{
  return (BitMap *)*pixH;
}

GrafPtr
DrawingPort::GetDrawingPort()
{
  return (GrafPtr)offscreenGWorld;
}

void
DrawingPort::SetCurrentPort()
{
  SetGWorld(offscreenGWorld, nil);
  current_port = this;
}

DrawingPort *
GetCurrentPort()
{
  return current_port;
}

void
SetCurrentPort(CGrafPtr non_drawing_port)
{
  SetGWorld(non_drawing_port, nil);
  current_port = nil;
}
