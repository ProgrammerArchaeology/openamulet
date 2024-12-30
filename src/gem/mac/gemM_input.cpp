/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file contains member function definitions for the Am_Drawonable_Impl
   object primarily concerned with interaction.
   
   added support multiple users to mouse clicks inside the window --bdk
*/

#include <iostream>
#include <stdlib.h>

#include <am_inc.h>

#include <amulet/gem.h>
#include <amulet/gemM.h>

/*******************************************************************************
 * Event Handling
 */

void Handle_Event_Received(EventRecord &mac_event);

/*******************************************************************************
 * Mouse Events
 */

void Check_Mouse_Events(const EventRecord &mac_event);
void Handle_Cursor(const EventRecord &mac_event);
void Handle_MouseDown(const EventRecord &mac_event);
void Handle_MouseUp(const EventRecord &mac_event);
void Handle_MenuBar(const EventRecord &mac_event);
void Handle_ClickContent(const EventRecord &mac_event, WindowRef mac_window);
void Handle_Drag(const EventRecord &mac_event, WindowRef mac_window);
void Handle_Grow(const EventRecord &mac_event, WindowRef mac_window);
void Handle_GoAway(const EventRecord &mac_event, WindowRef mac_window);
void Handle_Zoom(const EventRecord &mac_event, WindowRef mac_window,
                 short window_part);
void Handle_DiskEvt(const EventRecord &mac_event);

int Get_Button_State(int code);
void ButtonDown(int code, const EventRecord &mac_event, WindowRef mac_window);
void ButtonUp(int code, const EventRecord &mac_event);

/*******************************************************************************
 * Key Events
 */

void Handle_KeyDown(const EventRecord &mac_event);
void Handle_AutoKey(const EventRecord &mac_event);
void Handle_KeyUp(const EventRecord &mac_event);
void Key_Press(const EventRecord &mac_event);

/*******************************************************************************
 * Mac OS Events
 */

void Handle_Update(const EventRecord &mac_event);
void Handle_Activate(const EventRecord &mac_event);
void Handle_OS(const EventRecord &mac_event);

/*******************************************************************************
 * Menu Events
 */

bool Handle_Menu(long menu_result);

/*******************************************************************************
 * Utilities
 */

WindowRef find_window_at(Point where);
Am_Drawonable_Impl *find_subwindow_at(Point where, child *children);
Am_Drawonable_Impl *find_drawonable_at(Point where,
                                       WindowRef mac_window = (0L));
Am_Drawonable_Impl *get_keyboard_focus();
void send_input_char(Am_Drawonable_Impl *d, Am_Input_Char &ic,
                     const EventRecord &mac_event);
Am_Click_Count check_multi_click(short code, EventModifiers modifiers,
                                 Point where, Am_Button_Down down, long time);
bool points_are_close(Point a, Point b);
bool is_input_event(EventRecord &event);
bool process_one_input_event(EventRecord &mac_event);

/*******************************************************************************
 * Event Translation
 * note:  these do not support multiple users
 */

Am_Input_Char create_input_char_from_key(const EventRecord &mac_event);
short Map_Message_To_Code(unsigned char charCode, unsigned char vKeyCode,
                          bool shift, bool ctrl, bool meta);

/*******************************************************************************
 * Event Translation
 * note:  supports multiple users
 */

Am_Input_Char create_input_char_from_mouse(short code, Am_Button_Down down,
                                           const EventRecord &mac_event);
static Am_Value get_user_id(EventRecord &mac_event);

/*******************************************************************************
 * Constants
 */

const EventMask kMacNonInputEventMask =
    everyEvent - (mDownMask | mUpMask | keyDownMask | keyUpMask | autoKeyMask);

const char char_MiddleButton = 0x1C;
const char char_RightButton = 0x1D;

enum
{
  Am_LEFT_BUTTON_STATE,
  Am_MIDDLE_BUTTON_STATE,
  Am_RIGHT_BUTTON_STATE
};

/*******************************************************************************
 * Global Variables
 */

static Am_Drawonable_Impl *window_mouse_down = (0L);
static Am_Drawonable_Impl *window_mouse_over = (0L);
static bool button_states[3] = {false, false, false};
static short button_down_code = 0;
static Point last_where = {-32768, -32768};
static short last_click_code = 0;
static EventModifiers last_click_modifiers = 0;
static long last_click_time = 0;
static Am_Click_Count click_counter = (Am_Click_Count)0;

bool Am_Main_Loop_Go = true;    // Declared in idefs.h
int Am_Double_Click_Time = 250; // Declared in gem.h

short
FindWin(Point thePoint, WindowRef *theWindow)
{
  // std::cout << "In FindWin" <<std::endl;
  return FindWindow(thePoint, theWindow);
}

/*******************************************************************************
 * Handle_Event_Received
 */

void
Handle_Event_Received(EventRecord &mac_event)
{

  Handle_Cursor(mac_event);

  Check_Mouse_Events(mac_event);

  switch (mac_event.what) {
  case keyDown:
    Handle_KeyDown(mac_event);
    break;
  case autoKey:
    Handle_AutoKey(mac_event);
    break;
  case keyUp:
    Handle_KeyUp(mac_event);
    break;
  case updateEvt:
    Handle_Update(mac_event);
    break;
  case diskEvt:
    Handle_DiskEvt(mac_event);
    break;
  case activateEvt:
    Handle_Activate(mac_event);
    break;
  case osEvt:
    Handle_OS(mac_event);
    break;
  }
}

// === MOUSE EVENTS ===

/*******************************************************************************
 * Check_Mouse_Events
 */

void
Check_Mouse_Events(const EventRecord &mac_event)
{
  Boolean do_mouse_down = (mac_event.what == mouseDown);
  Boolean do_mouse_up =
      (button_states[Get_Button_State(last_click_code)] == true &&
       (mac_event.what == mouseUp || do_mouse_down == true ||
        Button() == false));
  if (do_mouse_up)
    Handle_MouseUp(mac_event);
  if (do_mouse_down)
    Handle_MouseDown(mac_event);
}

/*******************************************************************************
 * Handle_Cursor
 */

void
Handle_Cursor(const EventRecord &mac_event)
{
  Am_Input_Char ic;

  Am_Drawonable_Impl *d = find_drawonable_at(mac_event.where);
  SetCursor(d ? d->mac_cursor : &qd.arrow);

  if (d != window_mouse_over) {
    if (window_mouse_over != (0L) &&
        window_mouse_over->want_enter_leave == true) {
      ic = create_input_char_from_mouse(Am_MOUSE_LEAVE_WINDOW, Am_NEITHER,
                                        mac_event);
      send_input_char(window_mouse_over, ic, mac_event);
    }
    if (d != (0L) && d->want_enter_leave == true) {
      ic = create_input_char_from_mouse(Am_MOUSE_ENTER_WINDOW, Am_NEITHER,
                                        mac_event);
      send_input_char(d, ic, mac_event);
    }
    window_mouse_over = d;
  }

  Am_Drawonable_Impl *destWin = window_mouse_over; // where to send the message

  if (window_mouse_over) {
    if (!window_mouse_over->want_move) {
      destWin = window_mouse_down;
    }
  } else {
    destWin = window_mouse_down;
  }

  if (destWin) {
    if (!EqualPt(last_where, mac_event.where)) {
      ic = create_input_char_from_mouse(Am_MOUSE_MOVED, Am_NEITHER, mac_event);
      send_input_char(destWin, ic, mac_event);

      last_where = mac_event.where;
    }
  }
}

/*******************************************************************************
 * Handle_MouseDown
 */

void
Handle_MouseDown(const EventRecord &mac_event)
{
  WindowRef mac_window;
  short window_part = FindWin(mac_event.where, &mac_window);

  switch (window_part) {
  case inMenuBar:
    Handle_MenuBar(mac_event);
    break;
  case inSysWindow:
    SystemClick(&mac_event, mac_window);
    break;
  case inContent:
    Handle_ClickContent(mac_event, mac_window);
    break;
  case inDrag:
    Handle_Drag(mac_event, mac_window);
    break;
  case inGrow:
    Handle_Grow(mac_event, mac_window);
    break;
  case inGoAway:
    Handle_GoAway(mac_event, mac_window);
    break;
  case inZoomIn:
  case inZoomOut:
    Handle_Zoom(mac_event, mac_window, window_part);
    break;
  }
}

/*******************************************************************************
 * Handle_MouseUp
 */

void
Handle_MouseUp(const EventRecord &mac_event)
{
  ButtonUp(button_down_code, mac_event);
}

/*******************************************************************************
 * Handle_MenuBar
 */

void
Handle_MenuBar(const EventRecord &mac_event)
{
  long menu_result = MenuSelect(mac_event.where);
  Handle_Menu(menu_result);
}

/*******************************************************************************
 * Handle_ClickContent
 */

void
Handle_ClickContent(const EventRecord &mac_event, WindowRef mac_window)
{
  int code = Am_LEFT_MOUSE;

  if (mac_event.modifiers & optionKey) {
    if (mac_event.modifiers & shiftKey) {
      code = Am_MIDDLE_MOUSE; // option and shift key
    } else {
      code = Am_RIGHT_MOUSE; // just shiftkey
    }
  }

  ButtonDown(code, mac_event, mac_window);
}

/*******************************************************************************
 * Handle_Drag
 */

void
Handle_Drag(const EventRecord &mac_event, WindowRef mac_window)
{
  Rect drag_bounds = (**GetGrayRgn()).rgnBBox;
  InsetRect(&drag_bounds, 4, 4);
  DragWindow(mac_window, mac_event.where, &drag_bounds);

  Am_Drawonable_Impl *d = Get_Drawable_Backpointer(mac_window);
  if (d != (0L)) {
    SetPortWindowPort(mac_window);
    SetOrigin(0, 0);
    Point windowPos = {0, 0};
    LocalToGlobal(&windowPos);

    int w, h;
    d->Get_Size(w, h);

    d->reconfigure(windowPos.h, windowPos.v, w, h);
    if (d->event_handlers != (0L)) {
      d->event_handlers->Configure_Notify(d, windowPos.h, windowPos.v, w, h);
    }
  }
}

/*******************************************************************************
 * Handle_Grow
 */

void
Handle_Grow(const EventRecord &mac_event, WindowRef mac_window)
{
  Am_Drawonable_Impl *d = Get_Drawable_Backpointer(mac_window);
  if (d != (0L)) {
    int min_w, min_h, max_w, max_h;
    d->Get_Min_Size(min_w, min_h);
    d->Get_Max_Size(max_w, max_h);

    Rect grow_bounds;

    // Amulet specifies 0 to mean any maximum size
    // We limit it to the maximum area of the desktop
    Rect desktop_bounds = (**GetGrayRgn()).rgnBBox;
    grow_bounds.left = MAX(min_w, 50);
    grow_bounds.top = MAX(min_h, 50);
    grow_bounds.right =
        max_w ? max_w : desktop_bounds.right - desktop_bounds.left;
    grow_bounds.bottom =
        max_h ? max_h : desktop_bounds.bottom - desktop_bounds.top;

    long new_size = GrowWindow(mac_window, mac_event.where, &grow_bounds);
    if (new_size) {

      int x, y;
      d->Get_Position(x, y);
      int w = LoWord(new_size);
      int h = HiWord(new_size);
      d->Set_Size(w, h);
      if (d->event_handlers != (0L)) {
        d->event_handlers->Configure_Notify(d, x, y, w, h);
      }
    }
  }
}

/*******************************************************************************
 * Handle_GoAway
 */

void
Handle_GoAway(const EventRecord &mac_event, WindowRef mac_window)
{
  if (TrackGoAway(mac_window, mac_event.where)) {
    Am_Drawonable_Impl *d = Get_Drawable_Backpointer(mac_window);
    if (d != (0L) && d->event_handlers != nullptr) {
      d->event_handlers->Destroy_Notify(d);
    }
  }
}

/*******************************************************************************
 * Handle_Zoom
 */

void
Handle_Zoom(const EventRecord & /* mac_event */, WindowRef /* mac_window */,
            short /* window_part */)
{
}

/*******************************************************************************
 * Get_Button_State
 */

int
Get_Button_State(int code)
{
  switch (code) {
  case Am_LEFT_MOUSE:
    return Am_LEFT_BUTTON_STATE;
    break;

  case Am_MIDDLE_MOUSE:
    return Am_MIDDLE_BUTTON_STATE;
    break;

  case Am_RIGHT_MOUSE:
    return Am_RIGHT_BUTTON_STATE;
    break;
  }

  return 0;
}

/*******************************************************************************
 * ButtonDown
 */

void
ButtonDown(int code, const EventRecord &mac_event, WindowRef mac_window)
{
  if (mac_window == (0L))
    return;

  int id = Get_Button_State(code);
  button_states[id] = true;

  if (mac_window != FrontWindow())
    SelectWindow(mac_window);

  Am_Input_Char ic =
      create_input_char_from_mouse(code, Am_BUTTON_DOWN, mac_event);

  Am_Drawonable_Impl *d = find_drawonable_at(mac_event.where, mac_window);
  if (d != (0L)) {
    send_input_char(d, ic, mac_event);
  }

  window_mouse_down = d;
  button_down_code = code;
}

/*******************************************************************************
 * ButtonUp
 */

void
ButtonUp(int code, const EventRecord &mac_event)
{
  int id = Get_Button_State(code);
  button_states[id] = false;

  Am_Input_Char ic =
      create_input_char_from_mouse(code, Am_BUTTON_UP, mac_event);

  // When window_mouse_down and window_mouse_over are not equal and
  // want_multi_window == true for both of them, the message is sent to window_mouse_over
  // Otherwise, the message is always returned to window_mouse_down

  Am_Drawonable_Impl *destWin = window_mouse_down; // Where to send the message

  if (window_mouse_down->want_multi_window) {
    window_mouse_over = find_drawonable_at(mac_event.where);
    if (window_mouse_over && window_mouse_over->want_multi_window) {
      destWin = window_mouse_over;
    }
  }

  if (destWin != (0L)) {
    send_input_char(destWin, ic, mac_event);
  }
  window_mouse_down = (0L);
}

// === Key Events ===

/*******************************************************************************
 * Handle_KeyDown
 */
void
Handle_KeyDown(const EventRecord &mac_event)
{
  bool key_processed = false;
  char ch = mac_event.message & charCodeMask;

  if (mac_event.modifiers & cmdKey) {
    // Check to see if this is a mac menu bar short cut
    long menu_result = MenuKey(ch);
    key_processed = Handle_Menu(menu_result);
  }

  if (!key_processed)
    Key_Press(mac_event);
}

/*******************************************************************************
 * Handle_KeyUp
 */

void
Handle_KeyUp(const EventRecord & /* mac_event */)
{
}

/*******************************************************************************
 * Handle_AutoKey
 */

void
Handle_AutoKey(const EventRecord &mac_event)
{
  char ch = mac_event.message & charCodeMask;
  Key_Press(mac_event);
}

/*******************************************************************************
 * Key_Press
 */

void
Key_Press(const EventRecord &mac_event)
{
  Am_Input_Char ic = create_input_char_from_key(mac_event);
  if (ic.code != 0) { // Check for legal code
    if (ic.Exit_If_Stop_Char() == true)
      return;

    Am_Drawonable_Impl *d = get_keyboard_focus();
    if (d != (0L))
      send_input_char(d, ic, mac_event);
  }
}

/*******************************************************************************
 * Handle_Update
 */

void
Handle_Update(const EventRecord &mac_event)
{
  WindowRef mac_window = (WindowRef)mac_event.message;

  Am_Drawonable_Impl *d = Get_Drawable_Backpointer(mac_window);

  SetPortWindowPort(mac_window);
  BeginUpdate(mac_window);

  if (d != (0L)) {
    SetOrigin(0, 0);

    Rect r = (**GetWindowPort(mac_window)->visRgn).rgnBBox;
    int x, y, w, h;
    x = r.left;
    y = r.top;
    w = r.right - x;
    h = r.bottom - y;

    d->update_area(x, y, w, h);
    d->draw_grow_box();
  }

  EndUpdate(mac_window);
}

/*******************************************************************************
 * Handle_Activate
 */

void
Handle_Activate(const EventRecord &mac_event)
{
  WindowRef mac_window = (WindowRef)mac_event.message;

  Am_Drawonable_Impl *d = Get_Drawable_Backpointer(mac_window);
  if (d != (0L)) {
    SetPortWindowPort(mac_window);
    SetOrigin(0, 0);

    d->draw_grow_box();
  }
}

/*******************************************************************************
 * Handle_OS
 */
void
Handle_OS(const EventRecord &mac_event)
{
  switch ((mac_event.message & osEvtMessageMask) >> 24) {
  case suspendResumeMessage:
    if (mac_event.message & resumeFlag)
      HiliteMenu(0);
    break;

  case mouseMovedMessage:
    // currently the mouse moved event is not requested, eventually should do so
    break;
  }
}

/*******************************************************************************
 * Handle_DiskEvt
 */

void
Handle_DiskEvt(const EventRecord &mac_event)
{
  Point point;
  SetPt(&point, 120, 120);
  if (mac_event.message >> 16 != noErr) { // hi word contains error code

    DILoad();
    DIBadMount(point, mac_event.message);
    DIUnload();
  }
}

// === Menu Functions ===

/*******************************************************************************
 * Handle_Menu
 *   Handles any interaction with the standard mac menus. Returns true if
 *   handled a menu selection, false otherwise.
 */

// Bug: ?? it appears that commands that do not exist are said to be handled
// I am not sure about this. We want to pass along unhandles commands, right?
bool
Handle_Menu(long menu_result)
{
  bool cmd_handled = false;
  int menu_id = HiWord(menu_result);
  int menu_item = LoWord(menu_result);

  switch (menu_id) {
  case kAppleMenuID:
    if (menu_item == 1) {
      Alert(kAboutBoxAlert, nil);
    }

    if (menu_item > 2) {
      MenuHandle apple_menu = GetMenuHandle(kAppleMenuID);
      Str255 item_text;

      GetMenuItemText(apple_menu, menu_item, item_text);
      OpenDeskAcc(item_text);
    }
    cmd_handled = true;
    break;

  case kFileMenuID:
    if (menu_item == 1) {
      Am_Main_Loop_Go = false;
      cmd_handled = true;
    }
    break;
  }

  if (cmd_handled == true)
    HiliteMenu(0);

  return cmd_handled;
}

/*******************************************************************************
 * === Utilities ===
 ******************************************************************************/

/*******************************************************************************
 * find_window_at
 */

WindowRef find_window_at(Point where) // where is in global coordinates
{
  WindowRef window;
  short window_part = FindWin(where, &window);

  if (window_part == inContent)
    return window;
  else
    return (0L);

  /*
  RgnHandle contentRgn = NewRgn();
  WindowRef window = FrontWindow();
  GetWindowContentRgn( window, contentRgn );
  if( PtInRgn( where, contentRgn ) )
    return window;
  else
    return nil;
*/

  /*
  WindowRef fWindow = FrontWindow(), // should be FWindow
            window  = fWindow;

  short window_part;

  if( fWindow != sFrontWindow )
  {
    window_part = FindWin( where, &window );
    sFrontWindow = window;
  }

  GetWindowContentRgn( window, RgnHandle r)

  if( window_part == inContent )
    return window;
  else
    return (0L);
*/
}

/*******************************************************************************
 * find_subwindow_at
 */

Am_Drawonable_Impl *
find_subwindow_at(Point where, child *children)
{
  Am_Drawonable_Impl *d = (0L);

  child *scan = children;
  while (scan) {
    if (scan->d->Get_Visible()) {
      Rect bounds;
      scan->d->get_global_bounds(bounds);

      if (PtInRect(where, &bounds)) {
        d = find_subwindow_at(where, scan->d->children);
        if (d)
          return d;
        else
          return scan->d;
      }
    }
    scan = scan->next;
  }

  return (0L);
}

/*******************************************************************************
 * find_drawonable_at
 */

Am_Drawonable_Impl *find_drawonable_at(Point where,
                                       WindowRef mac_window) // default = (0L)
{
  if (mac_window == (0L))
    mac_window = find_window_at(where);

  Am_Drawonable_Impl *d = Get_Drawable_Backpointer(mac_window);
  if (d) {
    Am_Drawonable_Impl *subwindow = find_subwindow_at(where, d->children);
    if (subwindow)
      return subwindow;
  }

  return d;
}

/*******************************************************************************
 * get_keyboard_focus
 */

Am_Drawonable_Impl *
get_keyboard_focus()
{
  Point where;
  GetMouse(&where);
  LocalToGlobal(&where);

  return find_drawonable_at(where, (0L));
}

/*******************************************************************************
 * send_input_char
 */

void
send_input_char(Am_Drawonable_Impl *d, Am_Input_Char &ic,
                const EventRecord &mac_event)
{
  Am_Input_Event_Handlers *evh = d->event_handlers;
  if (evh != (0L)) {
    SetPortWindowPort(d->mac_window);

    int h = mac_event.where.h;
    int v = mac_event.where.v;

    d->add_total_offset_global(h, v);

    Am_Input_Event event;
    event.Set(ic, h, v, mac_event.when, d);

    Am_Current_Input_Event = &event;
    evh->Input_Event_Notify(d, Am_Current_Input_Event);
  } else if (Am_Debug_Print_Input_Events)
    std::cout << "<> Input ignored for " << d << "because no Event_Handler"
              << std::endl;
}

/*******************************************************************************
 * check_multi_click
 */

Am_Click_Count
check_multi_click(short code, EventModifiers modifiers, Point where,
                  Am_Button_Down down, long time)
{
  Am_Click_Count result = Am_SINGLE_CLICK;
  if (Am_Double_Click_Time) {
    // else not interested in multi-click
    // if a down press, then check if double click. If up, then use current
    // down count.  If other mouse event, then ignore multi-click
    if (down == Am_NEITHER) // result is OK, do nothing
      ;
    else if (down == Am_BUTTON_UP) { // use current value

      if (click_counter >= 7)
        result = Am_MANY_CLICK;
      else
        result = (Am_Click_Count)(click_counter + 1);
      // otherwise, just use single click, so result OK
    } else { // is a down press

      if ((code == last_click_code) && (modifiers == last_click_modifiers) &&
          ((time - last_click_time) <= GetDblTime()) &&
          (points_are_close(where, last_where))) {
        // is multi-click
        click_counter = (Am_Click_Count)(click_counter + 1);

        if (click_counter >= 7)
          result = Am_MANY_CLICK;
        else
          result = (Am_Click_Count)(click_counter + 1);
      } else {
        click_counter = (Am_Click_Count)0;
      }

      last_click_code = code;
      last_click_modifiers = modifiers;
      last_click_time = time;
    }
  }
  return result;
}

/*******************************************************************************
 * points_are_close
 */

bool
points_are_close(Point a, Point b)
{
#define Abs(a) ((a) < 0 ? (-a) : (a))
#define MULTI_CLICK_DISTANCE 4

  return (Abs(a.h - b.h) <= MULTI_CLICK_DISTANCE &&
          Abs(a.v - b.v) <= MULTI_CLICK_DISTANCE);
}

/*******************************************************************************
 * is_input_event
 */

bool
is_input_event(EventRecord &event)
{
  switch (event.what) {
  case mouseDown:
  case mouseUp:
  case keyDown:
  case keyUp:
  case autoKey:
    return true;
  }
  return false;
}

/*******************************************************************************
 * process_one_input_event
 */

bool process_one_input_event() // eventually this should accept a timeout value
{
  EventRecord mac_event;
  bool result = false;

  if (WaitNextEvent(everyEvent, &mac_event, kNoSleepTime, nil)) {

    Handle_Event_Received(mac_event);

    // If that was not an input event, then process all the remaining
    // non-input events until we have processed an input event.
    if (!is_input_event(mac_event)) {

      while (EventAvail(everyEvent, &mac_event)) {

        WaitNextEvent(everyEvent, &mac_event, kNoSleepTime, nil);
        Handle_Event_Received(mac_event);

        if (is_input_event(mac_event))
          break;
      }
    } else
      result = true;

    // Process all remaining non-input events
    while (EventAvail(kMacNonInputEventMask, &mac_event)) {

      WaitNextEvent(kMacNonInputEventMask, &mac_event, kNoSleepTime, nil);
      Handle_Event_Received(mac_event);
    }
  } else {
    Handle_Cursor(mac_event);
    Check_Mouse_Events(mac_event);
  }

  return result;
}

/*******************************************************************************
 *  Process_Immediate_Event:
 *    Does not wait for an event, but processes the first event in the queue
 *    and all non-input events after it until an input event is seen. The
 *    function returns when it encounters an input event (excluding the case
 *    where the first event is an input event) or when the queue is empty.
 */

void
Am_Drawonable::Process_Immediate_Event()
{
  process_one_input_event();
}

/*******************************************************************************
 *  Process_Event:
 *    waits for the next event, and processes exactly one input event and all
 *    non-input events before and after that input event before returning.
 *
 *    For example
 *       before            after
 *    xxxIyyyIzzz   --->   Izzz
 *
 *    The function returns when it encounters a second input event or when the
 *    queue is empty.
 */

void
Am_Drawonable::Process_Event(const Am_Time & /* timeout */)
{
  //  Am_Time_Data* time_data = Am_Time_Data::Narrow(timeout);
  //  unsigned long ticks = time_data->To_Ticks();
  //  time_data->Release();

  process_one_input_event();
}

/*******************************************************************************
 * Main_Loop
 */

void
Am_Drawonable::Main_Loop()
{
  Am_Time no_timeout;
  while (Am_Main_Loop_Go)
    Process_Event(no_timeout);
}

/*******************************************************************************
 * Am_Drawonable member functions
 *******************************************************************************
 */

/*******************************************************************************
 * Initialize_Event_Mask
 */

void
Am_Drawonable_Impl::Initialize_Event_Mask()
{
  want_enter_leave = false;
  want_multi_window = false;
  want_move = false;
}

/*******************************************************************************
 * set_drawable_event_mask
 */

void
Am_Drawonable_Impl::set_drawable_event_mask()
{
}

/*******************************************************************************
 * Set_Enter_Leave
 */

void
Am_Drawonable_Impl::Set_Enter_Leave(bool want_enter_leave_events)
{
  if (want_enter_leave_events != want_enter_leave) {
    want_enter_leave = want_enter_leave_events;
    this->set_drawable_event_mask();
  }
}

/*******************************************************************************
 * Set_Want_Move
 */

void
Am_Drawonable_Impl::Set_Want_Move(bool want_move_events)
{
  if (want_move != want_move_events) // then changing
  {
    want_move = want_move_events;
    this->set_drawable_event_mask();
  }
}

/*******************************************************************************
 * Set_Multi_Window
 */

void
Am_Drawonable_Impl::Set_Multi_Window(bool want_multi)
{
  if (want_multi != want_multi_window) { // then changing

    want_multi_window = want_multi;
    this->set_drawable_event_mask();
  }
}

/*******************************************************************************
 * Discard_Pending_Events
 *   Flushes the event queue. Checks if an event is available, if so gets it.
 */

void
Am_Drawonable_Impl::Discard_Pending_Events()
{
  EventRecord event;
  while (EventAvail(everyEvent, &event))
    GetNextEvent(everyEvent, &event);
}

/*******************************************************************************
 * Set_Input_Dispatch_Functions
 */

void
Am_Drawonable_Impl::Set_Input_Dispatch_Functions(Am_Input_Event_Handlers *evh)
{
  event_handlers = evh;
}

/*******************************************************************************
 * Get_Input_Dispatch_Functions
 */

void
Am_Drawonable_Impl::Get_Input_Dispatch_Functions(Am_Input_Event_Handlers *&evh)
{
  evh = event_handlers;
}

/*******************************************************************************
 * Get_Drawonable_At_Cursor
 *   Find the child-most drawonable at the current cursor position
 */

Am_Drawonable *
Am_Drawonable_Impl::Get_Drawonable_At_Cursor()
{
  Point where;
  GetMouse(&where);
  LocalToGlobal(&where);
  return find_drawonable_at(where);
}

/*******************************************************************************
 * create_input_char_from_mouse
 */

Am_Input_Char
create_input_char_from_mouse(short code, Am_Button_Down down,
                             const EventRecord &mac_event)
{
  Am_Click_Count count = check_multi_click(code, mac_event.modifiers,
                                           mac_event.where, down, TickCount());

  bool shift = ((mac_event.modifiers & shiftKey) != 0) &&
               ((mac_event.modifiers & optionKey) == 0);
  bool control = (mac_event.modifiers & controlKey) != 0;
  bool meta = (mac_event.modifiers & cmdKey) != 0;

  return Am_Input_Char(code, shift, control, meta, down, count);
}

/*******************************************************************************
 * Key Translation
 * Macintosh key event info
 *   In any key event (key-down, key-up and autokey) the event std::ends info
 *   about what keys were pressed in the message and modifiers fields.
 *   o virtual key code
 *     The high-order byte or the high-order word of the message field contains the
 *     virtual key code. This byte signifies the physical key which was pressed.
 *     Note that pressing a modifier key (command, option, control, shift or caps)
 *     does not generate a key event. The virtual key is the lowest level of
 *     translation and so its use should be avoided wherever possible.
 *   o character code
 *     The low-order byte or the high-order word of the message field contains the
 *     character code. This byte basically represents the ascii character which
 *     was generated. This character takes account of state of modifier symbols,
 *     so capitals or other symbols which require the shift key are already
 *     translated by the Event Manager. Unfortunately the option and control
 *     characters often generate high-ascii characters. This creates mapping
 *     problems. The control key range (0x00 - 0x1F) map to a somewhat standard
 *     key mapping.
 *
 * General design and usage of these
 *   These functions translate the info about key events sent by the Mac
 *   Event Manager into an Am_Input_Char object.
 *   o create_input_char_from_key
 *     Takes an event record and returns an Am_Input_Char.
 *     Retranslates the virtual key without modifier keys.
 *     Calls Map_Message_To_Code.
 *   o Map_Message_To_Code
 *     Maps the virtual key code and character code to a code which Am_Input_Char
 *     needs.
 *
 * Why we need to call KeyTranslate
 *   The Mac OS uses a 'KCHR' resource to map virtual keys to ascii character
 *   codes. The resource contains a set of tables for mapping. The table used
 *   depends on the state of modifier keys. The combinations of shift, control,
 *   and option control which table is used. (Note there are usually 8 tables,
 *   since there are 2^3 = 8 different states of these three modifier keys.)
 *   Note that the command key does not effect key mapping.
 *
 *   The result of this mapping scheme is that we get high ascii values when
 *   the option or control keys are pressed. This is bad. A possible solution is
 *   to do the mapping directly from the virtual keys, but this could cause
 *   problems with any non-standard keyboards. Therefore we use the KeyTranslate
 *   system function to retranslate the virtual key code. By doing this we
 *   stay another level of abstraction away from the hardware and support a
 *   wider array of hardware. Yippie.
 *
 *   The event manager calls KeyTranslate with all of the correct modifier keys.
 *   What we do is call KeyTranslate, but never say that the control or option
 *   keys are down. This way we get the correct mapping. Then we send the correct
 *   state of the modifier keys to the Am_Input_Char constructor.
 *
 * How Amulet interprets the character code and virtual key code
 *   o Printable characters (0x20 - 0x7E) => Interpreted as is
 *     The byte contains the code Am_Input_Char needs.
 *   o High character set (0x80 - 0xFF) => ignored
 *     Mostly diacritical characters or other special characters which Amulet
 *     does not support.
 *   o Control characters (selected bytes between 0x01 and 0x1F - see gemM.h for specifics)
 *     These characters are mapped through a switch statement since there are
 *     relatively few of them. A generic Map could have been used, but seemed
 *     unnecessary with only about 15 items.
 *   o Ambiguous characters => three cases were character code is ambiguous
 *     In these three cases (char_Clear, char_Escape and char_FunctionKey) the
 *     character code is ambiguous. char_Clear and char_Escape both return the
 *     same character code, so the virtual key code (vKey_Clear and vKey_Escape)
 *     must be used to distinguish between the two characters.
 *     Whenever a function key is pressed, the character code is always
 *     char_FunctionKey. The virtual key codes vKey_F* (1-15) determine exactly
 *     which key was pressed.
 *
 * References:
 *   o Mac Key Events
 *     Inside Macintosh: Macintosh Toolbox Essentials chapter 2, esp pp. 2-38 thru 2-40
 *   o Key mapping (for Roman character set)
 *     Inside Macintosh: Text pp. 1-54 thru 1-57
 */

/*******************************************************************************
 * create_input_char_from_key
 */

Am_Input_Char
create_input_char_from_key(const EventRecord &mac_event)
{
  unsigned char ch = mac_event.message & charCodeMask,
                virtualKey = (mac_event.message & keyCodeMask) >> 8;

  bool shift = ((mac_event.modifiers & (shiftKey | rightShiftKey)) != 0) ||
               (ch >= 'A' && ch <= 'Z');
  bool control = (mac_event.modifiers & (controlKey | rightControlKey)) != 0;
  bool meta = (mac_event.modifiers & cmdKey) != 0;

  // Retranslate the key event
  unsigned short keyCode = 0;

  // what goes into keyCode
  // bits 0-6 contain the virtual key code (the code is limited to 7 bits)
  // bit  7 is 1 if up stroke, and 0 if down stroke -- this needs to be
  //   set to 1 b/c KeyTranslate does not work properly otherwise
  // bits 8-15 contain the modifiers. We only keep shift and alpha lock,
  //   ignoring the control, option and command keys

  unsigned short modifiers =
      mac_event.modifiers & (shiftKey | rightShiftKey | alphaLock);
  keyCode = virtualKey | modifiers | 0x80;

  // get the default 'KCHR' resource
  void *KCHRPtr = (void *)GetScriptManagerVariable(smKCHRCache);

  unsigned long result = KeyTranslate(KCHRPtr, keyCode, 0);
  ch = result & charCodeMask;

  short code = Map_Message_To_Code(ch, virtualKey, shift, control, meta);

  if (code > 0) //  Only support keyboard keys going down
    return Am_Input_Char(code, shift, control, meta, Am_NEITHER, Am_NOT_MOUSE);
  else {
    std::cout << "** Unknown keyboard event, ignored\n"
              << "   Character code was 0x" << std::hex << (int)ch << endl
              << "   Virtual Key code was 0x" << std::hex << (int)virtualKey
              << std::endl;
    return Am_Input_Char();
  }
}

/*******************************************************************************
 * Map_Message_To_Code
 */

// returns character code or -1 if illegal
short
Map_Message_To_Code(unsigned char charCode, // an ascii character
                    unsigned char vKeyCode, // the virtual key pressed
                    bool /* shift */, bool /* ctrl */, bool /* meta */)
{

  // Try to convert directly if charCode is printable.

  if (charCode >= char_FirstPrinting && charCode <= char_LastPrinting)
    return charCode;

  // Ignore any high characters (bit 16 is on)
  if (charCode >= 0x80)
    return -1;

  // Now convert control characters
  switch (charCode) {
  case char_Home:
    return Am_HOME;
  case char_Enter:
    return Am_ENTER;
  case char_End:
    return Am_END;
  case char_Help:
    return Am_HELP;
  case char_Delete:
    return Am_DELETE;
  case char_Tab:
    return Am_TAB;
  case char_PageUp:
    return Am_PAGE_UP;
  case char_PageDown:
    return Am_PAGE_DOWN;
  case char_Return:
    return Am_RETURN;
  case char_LeftArrow:
    return Am_LEFT_ARROW;
  case char_RightArrow:
    return Am_RIGHT_ARROW;
  case char_UpArrow:
    return Am_UP_ARROW;
  case char_DownArrow:
    return Am_DOWN_ARROW;
  case char_FwdDelete:
    return Am_DELETE_CHAR;

  case char_Clear: // same as char_Escape
    if (vKeyCode == vKey_Clear)
      return Am_CLEAR;
    else
      return Am_ESC;
    break;

  case char_FunctionKey:
    switch (vKeyCode) {
    case vKey_F1:
      return Am_F1;
    case vKey_F2:
      return Am_F2;
    case vKey_F3:
      return Am_F3;
    case vKey_F4:
      return Am_F4;
    case vKey_F5:
      return Am_F5;
    case vKey_F6:
      return Am_F6;
    case vKey_F7:
      return Am_F7;
    case vKey_F8:
      return Am_F8;
    case vKey_F9:
      return Am_F9;
    case vKey_F10:
      return Am_F10;
    case vKey_F11:
      return Am_F11;
    case vKey_F12:
      return Am_F12;
    case vKey_F13:
      return Am_F13;
    case vKey_F14:
      return Am_F14;
    case vKey_F15:
      return Am_F15;
    }
  }
  return -1;

  /*
    This old stuff was left here in case something broke, since it might be convenient
    to see this. The new key mapping was implemented by Andrew Faulring on 15 Jun 96.
    If you code experiences and key input problems send mail to <amulet-bugs@cs.cmu.edu>
    and to <faulring@cs.cmu.edu>.

    Eventually remove this code on key handling is confirmed to be stable.

    2 Jul 96: Ignore the above note, the new fix had problems getting ascii characters
    when the option or command keys were held down. Need a way to translate the virtual
    key while ignoring the option keys.

*/
  //  char character_code = (message & charCodeMask);  // See Inside Mac, I-250
  //  char key_code = ((message & keyCodeMask) >> 8);
  /*
  char code = 0;

  switch( vKeyCode )
  {
    case 122: code =  Am_F1; break;  case 120: code =  Am_F2; break;
    case  99: code =  Am_F3; break;  case 118: code =  Am_F4; break;
    case  96: code =  Am_F5; break;  case  97: code =  Am_F6; break;
    case  98: code =  Am_F7; break;  case 100: code =  Am_F8; break;
    case 101: code =  Am_F9; break;  case 109: code = Am_F10; break;
    case 103: code = Am_F11; break;  case 111: code = Am_F12; break;
    case 105: code = Am_F13; break;  case 107: code = Am_F14; break;
    case 113: code = Am_F15; break;

    case 50: code = '\''; break;  case 18: code = '1'; break;  case 19: code = '2'; break;
    case 20: code = '3'; break;   case 21: code = '4'; break;  case 23: code = '5'; break;
    case 22: code = '6'; break;   case 26: code = '7'; break;  case 28: code = '8'; break;
    case 25: code = '9'; break;   case 29: code = '0'; break;  case 27: code = '-'; break;
    case 24: code = '='; break;

    case 12: code = (short)'q'; break;  case 13: code = (short)'w'; break;
    case 14: code = (short)'e'; break;  case 15: code = (short)'r'; break;
    case 17: code = (short)'t'; break;  case 16: code = (short)'y'; break;
    case 32: code = (short)'u'; break;  case 34: code = (short)'i'; break;
    case 31: code = (short)'o'; break;  case 35: code = (short)'p'; break;
    case 33: code = (short)'['; break;  case 30: code = (short)']'; break;
    case 42: code = (short)'\\'; break;

    case  0: code = (short)'a'; break;  case  1: code = (short)'s'; break;
    case  2: code = (short)'d'; break;  case  3: code = (short)'f'; break;
    case  5: code = (short)'g'; break;  case  4: code = (short)'h'; break;
    case 38: code = (short)'j'; break;  case 40: code = (short)'k'; break;
    case 37: code = (short)'l'; break;  case 41: code = (short)';'; break;
    case 39: code = (short)'\''; break;

    case  6: code = (short)'z'; break;  case  7: code = (short)'x'; break;
    case  8: code = (short)'c'; break;  case  9: code = (short)'v'; break;
    case 11: code = (short)'b'; break;  case 45: code = (short)'n'; break;
    case 46: code = (short)'m'; break;  case 43: code = (short)','; break;
    case 47: code = (short)'.'; break;  case 44: code = (short)'/'; break;

    case 49: code = (short)' '; break;

    case 123: code = Am_LEFT_ARROW; break;  case 126: code = Am_UP_ARROW; break;
    case 125: code = Am_DOWN_ARROW; break;  case 124: code = Am_RIGHT_ARROW; break;

    case  48: code = Am_TAB; break;      case  51: code = Am_DELETE; break;
    case  53: code = Am_ESC; break;      case  36: code = Am_RETURN; break;
    case 114: code = Am_HELP; break;     case 115: code = Am_HOME; break;
    case 116: code = Am_PAGE_UP; break;  case 117: code = Am_DELETE_CHAR; break;
    case 119: code = Am_END; break;      case 121: code = Am_PAGE_DOWN; break;

    // Keypad
    case 71: code = Am_CLEAR; break;      case 76: code = Am_ENTER; break;
    case 81: code = (short)'='; break;  case 75: code = (short)'/'; break;
    case 67: code = (short)'*'; break;  case 89: code = (short)'7'; break;
    case 91: code = (short)'8'; break;  case 92: code = (short)'9'; break;
    case 78: code = (short)'-'; break;  case 86: code = (short)'4'; break;
    case 87: code = (short)'5'; break;  case 88: code = (short)'6'; break;
    case 69: code = (short)'+'; break;  case 83: code = (short)'1'; break;
    case 84: code = (short)'2'; break;  case 85: code = (short)'3'; break;
    case 82: code = (short)'0'; break;  case 65: code = (short)'.'; break;

    default: code = 0;
  }

  // Note: Never use Mac-generated character-code with alphabetics, since
  // the Mac might map them to accented characters, which gem_keytrans.cc
  // does not support (for example, 'Option-n' + 'n' would map to 'ñ', an
  // accented 'n'.  In other words, the following code strips accent marks.
  //
  // Apply shift-key modifier to character
  if (shift) {
    //   Do alphabetic characters manually (see note above)...
    if (('a' <= code) && (code <= 'z')) code -= 0x20;
    //   Allow Mac to shiftify non-alphabetic characters, except for
    //   special characters like ESCAPE, etc...
    else
      if ((' ' <= code) && (code <= '~') && (!ctrl) && (!meta))
        code = charCode;
  }

  return code;
*/
}

/*******************************************************************************
 * Am_Init_Key_Map
 *   A stub to initialize a key map to translate from mac key events to amulet
 *   character codes. The mac does not use such a table.
 */

void
Am_Init_Key_Map()
{
}
