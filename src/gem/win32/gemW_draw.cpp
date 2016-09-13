/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h> // for multimedia timers
#include <stdlib.h>   // abort
#include <fstream>    // for images
#include <math.h>     // sin, cos
#include <limits.h>   // INT_MAX
#include <ctype.h>    // isupper, tolower

#include <am_inc.h>

#include <amulet/am_io.h>
//#include <amulet/gem.h>
#include <amulet/impl/am_drawonable.h>
#include <amulet/impl/am_region.h>
#include <amulet/impl/am_point_list.h>
#include <amulet/impl/am_point_array.h>
#include <amulet/impl/am_object.h>
#include <amulet/impl/inter_user_id.h>
#include <amulet/impl/slots.h>
#include <amulet/impl/opal_op.h>
//#include "multfont.h"

#include "amulet/gemW.h"
#include "amulet/gemW_misc.h"
#include "amulet/gemW_clean.h"
#include "amulet/gemW_points.h"
#include "amulet/gemW_styles.h"
#include "amulet/gemW_text.h"
#include "amulet/gemW_line.h"
#include "amulet/gemW_input.h"
#include "amulet/gemW_regions.h"
#include "amulet/gemW_image.h"
#include "amulet/gemW_cursor.h"

#include "amulet/gemW_draw.h"

#ifdef OA_VERSION
#include <amulet/univ_map_oa.hpp>
#else
#include <amulet/univ_map.h>
#endif

// Main loop procedure.  Handles all input.  Call this after set
// up.  Most programmers should use Am_Main_Event_Loop instead.
void
Am_Drawonable::Main_Loop()
{
  Am_Time no_timeout;
  while (Am_Main_Loop_Go)
    Process_Event(no_timeout);
}

// Abstract: Process_Event processes all queued non-input events, one input
//  event, and then non-input events up to the next input event.  It waits
//  to process at least one event (input or non-input) before exiting.
// Windows: First call ProcessQueuedEvents to process all the non-input
//  events we've received but haven't processed yet (they are on our queue,
//  not the Windows event queue.)  Then process the events in the Windows
//  event queue one at a time until we've processed one input event and
//  the rest of the non-input events, or until we run out of events.  Here,
//  if we get to a event that is sometimes queued and sometimes not queued
//  (ie WM_PAINT), our window proc std::ends it to our custom message queue
//  because it can't tell the event just came off another queue.
//  So, finally we must process queued events again to clear out all the
//  non-input events we just got off the windows queue.
// I'm not sure if we could remove the first call to ProcessQueuedEvents
//  or not.  I doubt it would work without it.  I'm not sure that this is
//  the proper place to handle WM_QUIT.  I don't see why it can't be in
//  the windows proc (why did Alex do it this way?)  I also don't know
//  if you should remove the WM_QUIT event or not.

// Changes for animation interactor:
// Now Process_Event will time out after the deadline passes.
// It will process as few as 0 events, and at maximum, all of the events
// described above.  It tries hard not to drop events on the floor, by only
// exiting when it's done with event processing.

bool is_input_event(MSG msg); // forward reference needed by this function
void
Am_Drawonable::Process_Event(const Am_Time &deadline)
{
  // Figure out when we stop processing
  bool have_deadline = !deadline.Zero();

  bool rtn = Am_WinDrawonable::ProcessQueuedEvents();

  MSG msg;
  Am_WinDrawonable::WinDoIdle();

  // only wait for a message if none in either queue.
  if (!rtn && !PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE)) {
    if (have_deadline) {
      Am_Time now = Am_Time::Now();
      if (deadline > now) {
        Am_Time timeout = deadline - now;
        Am_WinDrawonable::WaitMessageWithTimeout(timeout.Milliseconds());
      }
    } else
      // no deadline -- wait indefinitely
      WaitMessage();
  }

  while (PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE)) {
    if (msg.message == WM_QUIT) {
      Am_Main_Loop_Go = false;
      return;
    }
    PeekMessage(&msg, 0, 0, 0, PM_REMOVE);
    TranslateMessage(&msg);
    DispatchMessage(&msg);

    // Dispatch may run for a while, so check timeout
    if (have_deadline && deadline.Is_Past())
      return;

    if (is_input_event(msg))
      break;
  }

  while (PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE)) {
    if (msg.message == WM_QUIT) {
      Am_Main_Loop_Go = false;
      return;
    }
    if (is_input_event(msg))
      break;

    PeekMessage(&msg, 0, 0, 0, PM_REMOVE);
    TranslateMessage(&msg);
    DispatchMessage(&msg);

    // Dispatch may run for a while, so check timeout
    if (have_deadline && deadline.Is_Past())
      return;
  }

  Am_WinDrawonable::ProcessQueuedEvents();
}

// This waits for an event to get into the queue, does not process it,
// and returns.  Does nothing if an event is already in the queue.
void
Am_Drawonable::Wait_For_Event()
{
  WaitMessage();
}

// This works just like Process_Event, but does not wait for an event to
//  come in if there isn't one on a queue already.

void
Am_Drawonable::Process_Immediate_Event()
{

  Am_WinDrawonable::ProcessQueuedEvents();

  MSG msg;
  Am_WinDrawonable::WinDoIdle();

  // don't wait for a message if there aren't any.

  while (PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE)) {
    if (msg.message == WM_QUIT) {
      Am_Main_Loop_Go = false;
      return;
    }
    PeekMessage(&msg, 0, 0, 0, PM_REMOVE);
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    if (is_input_event(msg))
      break;
  }
  while (PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE)) {
    if (msg.message == WM_QUIT) {
      Am_Main_Loop_Go = false;
      return;
    }
    if (is_input_event(msg))
      break;
    PeekMessage(&msg, 0, 0, 0, PM_REMOVE);
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  Am_WinDrawonable::ProcessQueuedEvents();
}

// Get the root Am_Drawonable.  This is not a member function since
// don't necessarily have an object to start with.  String parameter
// used under X to specify a screen.  This also causes any required
// initialization to happen.  It is OK to call this more than once.
Am_Drawonable *
Am_Drawonable::Get_Root_Drawonable(const char *screen)
{
  return Am_WinDrawonable::Get_Root_Drawonable(screen);
}

//void Am_Error (const char* szErrMsg)
//{
//	::MessageBox(GetFocus(), szErrMsg, "Amulet Error", MB_ICONEXCLAMATION | MB_OK);
//}

// Global variable holding the input event currently being processed.  Defined
// in gem.h.
// allocate current event record
//Am_Input_Event _Am_Current_Input_Event;
//Am_Input_Event *Am_Current_Input_Event = &_Am_Current_Input_Event;

bool Am_Main_Loop_Go = true;

// Whether to print input events or not.
//int Am_Debug_Print_Input_Events = 0;

//Global variable controlling multiple-click handling.  If 0, then no
//double-click processing.  Otherwise, is the inter-click wait time in
//milleseconds.  Default value = 250.
//(Declared in GEM.H)
int Am_Double_Click_Time = GetProfileInt("windows", "DoubleClickSpeed", 250);

//---------------
// Am_Drawonable
//---------------

// EVENT HANDLING IN WINDOWS AND AMULET
// 6-23-95 af1x notes for future generations:
// I use "message" and "event" interchangeably.
//
// Each window has a window proc associated with it.  This proc is used to
// handle events that occur in that window.  Currently we use one window
// proc for all Amulet windows.
//
// WindowsOS does one of two things with messages for your windows: it either
// queues them on a message queue, or std::ends them directly to your window proc.
// Input events are queued, and many non-input events are not queued.  Some
// events are queued sometimes and not queued other times.  As soon as Windows
// creates a window, it starts sending non-input events to the window proc,
// before it even returns from the window create call.  At this point, Amulet
// is not yet ready to handle these events (like, an expose notify) because
// the structures above the gem level don't know enough about the window to do
// anything with it.  To avoid this, we create a second message  queue, stored
// in m_msgqueue.  Events which are sometimes not queued by windows are queued
// by us onto this alternate queue.  This makes sure we don't deal with them
// until Opal explicitly requests us to deal with them.  (note: this second
// message queue isn't a windows-message queue, just a list we maintain
// ourselves) The procedures below have comments describing what they're
// supposed to do, and how they do it.

Am_WinDrawonable::Am_WinDrawonable(int l, int t, unsigned int w, unsigned int h,
                                   const char *tit, const char *icon_tit,
                                   bool vis, bool initially_iconified,
                                   Am_Style back_color, bool save_under_flag,
                                   int min_w, int min_h, int max_w, int max_h,
                                   bool title_bar_flag,
                                   bool clip_by_children_flag,
                                   Am_Input_Event_Handlers *evh)
    : m_evh(evh)
{
  left = l;
  top = t;
  width = w;
  height = h;

  title = strnew(tit);
  icon_name = strnew(icon_tit);

  visible = vis;
  iconified = initially_iconified;

  background_color = back_color;
  border_width = title_bar_flag ? 2 : 0;

  save_under = save_under_flag;

  clip_by_children = clip_by_children_flag;
  min_width = min_w;
  min_height = min_h;
  max_width = max_w;
  max_height = max_h;
  title_bar = title_bar_flag;

  //ask_position 	= query_user_for_position;
  //ask_size 		= query_user_for_size;

  depth = WndGetColorDepth();
  data_store = (0L);

  //Am_DrawonableWin data
  m_hwnd = 0;
  m_parent = (0L);
  m_hwndParent = 0;
  m_isbuff = FALSE;
  m_usebuff = FALSE;
  m_hbmp = 0;
  m_style = WS_OVERLAPPEDWINDOW;
  m_clip = (0L);
  m_want_enter_leave_events = false;
  m_want_move_events = false;
  m_want_multi_window = false;
}

Am_WinDrawonable::~Am_WinDrawonable()
{
  SetBuffered(FALSE); // delete buffer if any
  WndDestroy();       // kill window before die
  delete m_clip;
  strdel(title);
  strdel(icon_name);
  //if (m_mapWnd2Drw.Count() == 1 /*only root left*/)
  //  PostQuitMessage(0);
  //if (this == m_root) //this was root drawonable
  //	WinUninit();
}

// is_input_event tests msg to determine if it is
// an input event or non-input event (message).
bool
is_input_event(MSG msg)
{
  switch (msg.message) {
  case WM_LBUTTONDOWN:
  case WM_LBUTTONUP:
  case WM_LBUTTONDBLCLK:
  case WM_MBUTTONDOWN:
  case WM_MBUTTONUP:
  case WM_MBUTTONDBLCLK:
  case WM_RBUTTONDOWN:
  case WM_RBUTTONUP:
  case WM_RBUTTONDBLCLK:
  case WM_MOUSEMOVE:
  case WM_CHAR:
  case WM_SYSCHAR:
  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
    return true;
  default:
    return false;
  }
}

//
// Multimedia timers (for animation)
//

UINT Am_WinDrawonable::idTimer;
DWORD Am_WinDrawonable::idThread = GetCurrentThreadId();

void CALLBACK
Am_WinDrawonable::TimeProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1,
                           DWORD dw2)
{
  // timer has expired!
  // send a harmless message (WM_USER is the first user-defined
  // message) to wake up the event loop
  PostThreadMessage(idThread, WM_USER, 0, 0);
  // std::cerr << "timer expired" <<std::endl;
  idTimer = (0L);
}

void
Am_WinDrawonable::WaitMessageWithTimeout(unsigned long milliseconds)
{
  // std::cerr << "setting timer for " << milliseconds << " ms" <<std::endl;
  idTimer = timeSetEvent(milliseconds, 0, TimeProc, 0, TIME_ONESHOT);

  if (!idTimer) {
    // can't set the timer (probably becuase duration is too short)
    // return right away
    std::cerr << "busy waiting" << std::endl;
    return;
  }

  WaitMessage();

  if (idTimer) {
    // timer still running -- kill it!
    // std::cerr << "timer interrupted" <<std::endl;
    timeKillEvent(idTimer);
    idTimer = (0L);
  }
}

//////////////////////////
// Am_Drawonable::Am_WinDC

Am_WinDrawonable::Am_WinDC::Am_WinDC(const Am_WinDrawonable *drw)
{
  if (drw->IsBuffered()) {
    m_hwnd = 0;
    if (!drw->HasBuffer())
      ((Am_WinDrawonable *)drw)
          ->CreateBuffer(); //override const to call CreateBuffer
    HDC hidc = ::CreateIC("DISPLAY", (0L), NULL, NULL);
    m_hdc = ::CreateCompatibleDC(hidc);
    // use bitmap to draw
    SelectObject((HDC)(*this), drw->BufferBitmap());
    ::DeleteDC(hidc);
  } else {
    m_hwnd = drw->WndGet();
    m_hdc = ::GetDC(m_hwnd);
  }

  if (drw->Get_Clip()) {
    ::SelectClipRgn(m_hdc, drw->Get_Clip()->WinRegion());
    //::SetBoundsRect(m_hdc, &(drw -> Get_Clip() -> WinRect()), DCB_ACCUMULATE);
  }
}

Am_WinDrawonable::Am_WinDC::Am_WinDC(const Am_WinDC &dc)
{
  m_hwnd = 0;
  m_hdc = ::CreateCompatibleDC(dc);
}

Am_WinDrawonable::Am_WinDC::Am_WinDC(HWND hwnd)
{
  m_hwnd = hwnd;
  m_hdc = ::GetDC(m_hwnd);
}

Am_WinDrawonable::Am_WinDC::~Am_WinDC()
{
  if (m_hdc) {
    if (m_hwnd)
      ::ReleaseDC(m_hwnd, m_hdc);
    else
      ::DeleteDC(m_hdc);
  }
  m_hdc = 0;
  m_hwnd = 0;
}

// use this only for GDI resources that should be deleted when
// Am_WinDC is destroyed!!!
void
Am_WinDrawonable::Am_WinDC::SelectTool(HGDIOBJ htool)
{
  if (htool) {
    m_tools.InsertAt(m_tools.HeadPos(), (long)::SelectObject(*this, htool));
    m_tools.InsertAt(m_tools.HeadPos(), (long)htool);
  }
}

void
Am_WinDrawonable::Am_WinDC::DeselectTool(HGDIOBJ htool)
{
  for (Am_Position pos = m_tools.HeadPos(); pos;
       pos = m_tools.NextPos(m_tools.NextPos(pos)))
    if ((HGDIOBJ)m_tools.GetAt(pos) == htool) {
      Am_Position nextpos = m_tools.NextPos(pos);
      ::SelectObject(*this, (HGDIOBJ)nextpos);
      m_tools.DeleteAt(pos);
      m_tools.DeleteAt(nextpos);
      break;
    }
}

void
Am_WinDrawonable::Am_WinDC::DeselectAll()
{
  // This is only called by the Am_WinDC destructor
  // delete objects as they are deselected
  Am_Position pos;
  while (pos = m_tools.HeadPos()) {
    Am_Position nextpos = m_tools.NextPos(pos);
    DeleteObject(SelectObject(*this, (HGDIOBJ)m_tools.GetAt(nextpos)));
    m_tools.DeleteAt(pos);
    m_tools.DeleteAt(nextpos);
  }
}

bool
is_transparent_stippled(const Am_Style &fs)
{
  bool answer = false;

  if (fs.Valid()) {
    Am_Style_Data *wfs = Am_Style_Data::Narrow(fs);
    if (wfs->Is_Stippled() && wfs->FillSolid() == Am_FILL_STIPPLED)
      answer = true;
    wfs->Release();
  }
  return answer;
}

void
Am_WinDrawonable::Am_WinDC::Set_DC_Transparent_Stipple(const Am_Style &ls,
                                                       const Am_Style &fs,
                                                       int emulate)
{
  Am_Style_Data *wls = Am_Style_Data::Narrow(ls);
  Am_Style_Data *wfs = Am_Style_Data::Narrow(fs);

  HPEN pen;
  HBRUSH brush;

  if (wls)
    pen = wls->WinPen(emulate);
  else
    pen = Am_Style_Data::hpenNullPen;

  if (wfs)
    brush = wfs->WinBrush(*this);
  else
    brush = Am_Style_Data::hbrNullBrush;

  SetTextColor((HDC)(*this), Am_Style_Data::crefBlack);
  SetBkColor((HDC)(*this), Am_Style_Data::crefWhite);
  SetROP2((HDC)(*this), WinRop2Func(Am_DRAW_GRAPHIC_AND));

  SelectObject((HDC)(*this), pen);
  SelectObject((HDC)(*this), brush);

  if (wls)
    wls->Release();
  if (wfs)
    wfs->Release();
}

void
Am_WinDrawonable::Am_WinDC::Adjust_DC_Transparent_Stipple(const Am_Style &fs)
{
  if (fs.Valid()) {
    Am_Style_Data *wfs = Am_Style_Data::Narrow(fs);

    SetTextColor((HDC)(*this), wfs->m_cref);
    wfs->Release();
  }

  SetBkColor((HDC)(*this), Am_Style_Data::crefBlack);
  SetROP2((HDC)(*this), WinRop2Func(Am_DRAW_GRAPHIC_OR));
}

/*************************************************************
 *  sets dc rop2, pen and brush according to f, ls and fs
 ************************************************************/
void
Am_WinDrawonable::Am_WinDC::Set_DC(const Am_Style &ls, const Am_Style &fs,
                                   Am_Draw_Function f, int emulate)
{

  SetROP2((HDC)(*this), WinRop2Func(f));

  HPEN pen;
  HBRUSH brush;

  if (ls.Valid()) {
    Am_Style_Data *wls;
    if (f == Am_DRAW_MASK_COPY) {
      static Am_Style Mask_Pen_Styles[21]; // holder for different pen widths

      short thickness;
      Am_Line_Cap_Style_Flag cap;
      ls.Get_Line_Thickness_Values(thickness, cap);
      if (thickness < 0)
        thickness = 0;
      if (thickness > 20)
        thickness = 20;
      Am_Style pen_style = Mask_Pen_Styles[thickness];
      if (!pen_style.Valid()) {
        pen_style = ls.Clone_With_New_Color(Am_On_Bits);
        Mask_Pen_Styles[thickness] = pen_style;
      }
      wls = Am_Style_Data::Narrow(pen_style);
    } else
      wls = Am_Style_Data::Narrow(ls);
    pen = wls->WinPen(emulate);
    wls->Release();
  } else
    pen = Am_Style_Data::hpenNullPen;

  if (fs.Valid()) {
    Am_Style_Data *wfs;
    if (f == Am_DRAW_MASK_COPY)
      wfs = Am_Style_Data::Narrow(Am_On_Bits);
    else
      wfs = Am_Style_Data::Narrow(fs);
    brush = wfs->WinBrush(*this);
    wfs->Release();
  } else
    brush = Am_Style_Data::hbrNullBrush;

  SelectObject((HDC)(*this), pen);
  SelectObject((HDC)(*this), brush);
}

/*************************************************************
 *  sets dc rop2, text and background according to f, ls and fs
 ************************************************************/
void
Am_WinDrawonable::Am_WinDC::Set_DC_Text(const Am_Style &ls, const Am_Style &fs,
                                        Am_Draw_Function f, bool invert)
{
  SetROP2((HDC)(*this), WinRop2Func(f));

  COLORREF crText;
  COLORREF crBkgnd;

  if (f == Am_DRAW_MASK_COPY) {
    Am_Style_Data *on_bits = Am_Style_Data::Narrow(Am_On_Bits);
    Am_Style_Data *off_bits = Am_Style_Data::Narrow(Am_Off_Bits);
    crText = on_bits->m_cref;
    crBkgnd = off_bits->m_cref;
    on_bits->Release();
    off_bits->Release();
  } else {
    if (ls.Valid()) {
      Am_Style_Data *wls = Am_Style_Data::Narrow(ls);
      crText = wls->m_cref;
      wls->Release();
    } else
      crText = Am_Style_Data::crefBlack;

    if (fs.Valid()) {
      Am_Style_Data *wfs = Am_Style_Data::Narrow(fs);
      crBkgnd = wfs->m_cref;
      wfs->Release();
    } else
      crBkgnd = Am_Style_Data::crefWhite;
  }

  if (invert) {
    SetTextColor((HDC)(*this), crBkgnd);
    SetBkColor((HDC)(*this), crText);
    SetBkMode((HDC)(*this), OPAQUE);
  } else {
    SetTextColor((HDC)(*this), crText);
    if (fs.Valid())
      SetBkColor((HDC)(*this), crBkgnd);
    else
      SetBkMode((HDC)(*this), TRANSPARENT);
  }
}

void
Am_WinDrawonable::Am_WinDC::Set_DC_Font(const Am_Font &font)
{
  if (font.Valid()) {
    Am_Font_Data *fntd = Am_Font_Data::Narrow(font);
    SelectObject(*this, fntd->WinFont(*this));
    fntd->Release();
  }
}

///////////////////
// Am_WinDrawonable

#pragma init_seg(user)
//Am_WinDrawonable _rootWinDrawonable;

Am_WinDrawonable *Am_WinDrawonable::m_root = (0L); // = &_rootWinDrawonable;

HINSTANCE Am_WinDrawonable::m_inst = 0;
Am_List_Ptr Am_WinDrawonable::m_msgqueue;

#if defined(_WIN32)
HMODULE Am_WinDrawonable::m_hThunk32s = (0L);
#endif

#define PREPARE_WLS PREPARE_DATA(Am_Style, wls, ls)
//	Am_Style_Data* wls = Am_Style_Data::Narrow(ls);
#define PREPARE_WFS PREPARE_DATA(Am_Style, wfs, fs)
//	Am_Style_Data* wfs = Am_Style_Data::Narrow(fs);
#define WLS (wls)
#define WFS (wfs)
#define DISCARD_WLS DISCARD_DATA(wls)
//	wls -> Release();
#define DISCARD_WFS DISCARD_DATA(wfs)
//	wfs -> Release();

//  ProcessQueuedEvents deals with all the events we've queued on our
// own custom event queue.  The events are stored as a struct containing
// the event handling routine and any required parameters to that routine.
// So here, we simply call that routine with the struct, and delete the
// message from the queue.  We return false if there were no events in
// the queue to process, and true if we did process some events.
// POTENTIAL BUG:  ProcessQueuedEvents processes mostly non-input events.
// If processing one of these events produces another non-input event,
// you can run into infinite loops.  GWSTREAMS was a problem here.

bool
Am_WinDrawonable::ProcessQueuedEvents()
{
  bool tmp = !m_msgqueue.IsEmpty();
  while (!m_msgqueue.IsEmpty()) {
    Am_WinNotifyInfo *info =
        (Am_WinNotifyInfo *)m_msgqueue.GetAt(m_msgqueue.HeadPos());
    (info->m_func)(info);
    delete info;
    m_msgqueue.DeleteAt(m_msgqueue.HeadPos());
  }
  return tmp;
}

void
Am_WinDrawonable::Print(std::ostream &os) const
{
  os << Get_Title();
  os << "(" << std::hex << (unsigned long)this << std::dec << ")";
}

void
Am_WinDrawonable::WinInit()
{
  if (m_root)
    return;

  m_root = new Am_WinDrawonable;
  m_root->WndAttach(GetDesktopWindow());

  //g_cleaner = new Am_WinCleaner();

  Am_Initialize_Char_Map();
//Am_Initialize_Color_Map();
//Am_Initialize_Multifont();

#if !defined(_WIN32)
  m_inst = *(HINSTANCE __far *)MAKELP(GetCurrentTask(), 0x1C);
#endif

  WNDCLASS wc;

  if (!GetClassInfo(m_inst, Am_WinDrawonable_CLASS, &wc)) {
    //class is not yet registered
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;     // No per-class extra data.
    wc.cbWndExtra = 0;     // No per-window extra data.
    wc.hInstance = m_inst; // Application that owns the class.
    wc.hIcon = LoadIcon((0L), IDI_APPLICATION);
    wc.hCursor = LoadCursor((0L), IDC_ARROW);
#if _MSC_VER >= 1200
    wc.hbrBackground =
        reinterpret_cast<HBRUSH__ *>(GetStockObject(WHITE_BRUSH));
#else
    wc.hbrBackground = GetStockObject(WHITE_BRUSH);
#endif
    wc.lpszMenuName = (0L);
    wc.lpszClassName = Am_WinDrawonable_CLASS;

    RegisterClass(&wc);
  }

  if (!GetClassInfo(m_inst, Am_WinDrawonable_SBT_CLASS, &wc)) {
    wc.style |= CS_SAVEBITS;
    wc.lpszClassName = Am_WinDrawonable_SBT_CLASS;

    RegisterClass(&wc);
  }
}

void
Am_WinDrawonable::WinUninit()
{ /*
	//BUG:
	// This code SHOULD be in here (I think), but it crashes on the delete
	// line so I took it out for the v2.03Beta release.  4-8-96 af1x. 
	if (Am_WinDrawonable::m_root==(0L))
	return;

	delete Am_WinDrawonable::m_root.Release();
	Am_WinDrawonable::m_root = (0L);
	*/
}

Am_Drawonable *
Am_WinDrawonable::Get_Root_Drawonable(const char * /*screen*/)
{ //ignore screen
  if (!m_root)
    Am_WinDrawonable::WinInit();
  return m_root;
}

// parent is the Am_Drawonable this is sent to
Am_Drawonable *
Am_WinDrawonable::Create(int l, int t, unsigned int w, unsigned int h,
                         const char *tit, const char *icon_tit, bool vis,
                         bool initially_iconified, Am_Style back_color,
                         bool save_under_flag,
                         int min_w, // can't have 0 size windows
                         int min_h,
                         int max_w, // 0 is illegal so means no max
                         int max_h, bool title_bar_flag,
                         bool query_user_for_position, bool query_user_for_size,
                         bool clip_by_children_flag,
                         Am_Input_Event_Handlers *evh)
{
  Am_WinDrawonable *newdrw =
      new Am_WinDrawonable(l, t, w, h, tit, icon_tit, vis, initially_iconified,
                           back_color, save_under_flag, min_w, min_h, max_w,
                           max_h, title_bar_flag, clip_by_children_flag, evh);
  if (!newdrw)
    return (0L);
  newdrw->SetParent(this);
  newdrw->WndCreate(query_user_for_position, query_user_for_size,
                    save_under_flag);
  return newdrw;
}

void
Am_WinDrawonable::Destroy()
{
  delete this;
}

Am_Drawonable *
Am_WinDrawonable::Create_Offscreen(int width, int height, Am_Style back_color)
{
  Am_WinDrawonable *wdrw = (Am_WinDrawonable *)Create(
      0 /*CW_USEDEFAULT*/, 0 /*CW_USEDEFAULT*/,
      // we don't use default, because Windows places an invisible window
      // out of visible desktop => it wouldn't appear even when we make it visble
      width, height, (0L), (0L), false, false, back_color);
  if (wdrw) {
    //set bufferring on
    wdrw->SetUsesBufferring(TRUE);
    wdrw->SetBuffered(TRUE);
  }
  return wdrw;
}

void
Am_WinDrawonable::Reparent(Am_Drawonable *new_parent)
{
  if ((new_parent != m_parent) && (new_parent != this)) {
    for (Am_Position pos = m_children.HeadPos(); pos;
         pos = m_children.NextPos(pos))
      Narrow(m_children[pos])->WndDetach();
    if (!new_parent)
      new_parent = m_root;
    if (m_hwnd) {
      DestroyWindow(WndDetach());
      SetParent(Narrow(new_parent));
      WndCreate();
    }
    //		if (m_evh)
    //			m_evh -> Reparent_Notify(this);
  }
}

bool Am_WinDrawonable::Inquire_Window_Borders(int &left_border, int &top_border,
                                              int &right_border,
                                              int &bottom_border,
                                              int &outer_left,
                                              int &outer_top) //const
{
  // m_style = WndStyleFromData(); -- must be called before!
  // (this is done if window is created)
  int infx = 0, infy = 0, infb = 0;

  if ((m_style & WS_THICKFRAME) == WS_THICKFRAME) {
    infx = GetSystemMetrics(SM_CXFRAME);
    infy = GetSystemMetrics(SM_CYFRAME);
  }

  if ((m_style & WS_CAPTION) == WS_CAPTION)
    infb = GetSystemMetrics(SM_CYCAPTION);
  // for WinNT the following may be the correct computation ???
  // the old code never executed this path
  // infb = GetSystemMetrics(SM_CYCAPTION) - GetSystemMetrics(SM_CYBORDER);

  left_border = right_border = infx;
  top_border = infy + infb;
  bottom_border = infy;

  outer_left = left;
  outer_top = top;

  return true;
}

void Am_WinDrawonable::Raise_Window(/*const*/ Am_Drawonable *target_d) //const
{
  HWND hwndTrg;

  if (!target_d || !(hwndTrg = Narrow(target_d)->WndGet())) {
    if (title_bar)
      // this activates the window
      BringWindowToTop(m_hwnd);
    else
      // this line does not appear to be necessary for menu windows,
      // but I have left it in just in case it is necessary for another
      // kind of window!!!
      SetWindowPos(m_hwnd, HWND_TOP, 0, 0, 0, 0,
                   SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREDRAW);
  } else {
    // Bring just above the target:
    // first bring just below the target ...
    SetWindowPos(m_hwnd, hwndTrg, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREDRAW);
    // then bring target just below the window
    SetWindowPos(hwndTrg, m_hwnd, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
  }
}

void Am_WinDrawonable::Lower_Window(/*const*/ Am_Drawonable *target_d) //const
{
  HWND hwndTrg;
  if (!target_d || !(hwndTrg = Narrow(target_d)->WndGet()))
    SetWindowPos(m_hwnd, HWND_BOTTOM, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
  else
    // Bring just below the target
    SetWindowPos(m_hwnd, hwndTrg, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void
Am_WinDrawonable::Set_Iconify(bool iconify)
{
  if (!iconify != !iconified) {
    if (iconify)
      WndIconify();
    else
      WndRestore();
  }
}

void
Am_WinDrawonable::Set_Title(const char *new_title)
{
  strdel(title);
  title = strnew(new_title);
  if (m_hwnd && (!iconified || !icon_name))
    SetWindowText(m_hwnd, title);
}

void
Am_WinDrawonable::Set_Icon_Title(const char *new_title)
{
  strdel(icon_name);
  icon_name = strnew(new_title);
  if (m_hwnd && (iconified && icon_name))
    SetWindowText(m_hwnd, icon_name);
}

void
Am_WinDrawonable::Set_Position(int new_left, int new_top)
{
  if (top == new_top && left == new_left)
    return;
  left = new_left;
  top = new_top;
  if (m_hwnd) {
    SetWindowPos(m_hwnd, 0, new_left, new_top, 0, 0,
                 SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
  }
}

void
Am_WinDrawonable::Set_Size(unsigned int new_width, unsigned int new_height)
{
  if (width == new_width && height == new_height)
    return;
  if (IsBuffered())
    ResizeBuffer(new_width, new_height);

  if (m_hwnd) {
    int bl, bt, br, bb, dummy;
    Inquire_Window_Borders(bl, bt, br, bb, dummy, dummy);
    SetWindowPos(m_hwnd, 0, 0, 0, new_width + bl + br, new_height + bt + bb,
                 SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
  }
  width = new_width;
  height = new_height;
}

void
Am_WinDrawonable::Set_Max_Size(unsigned int new_width, unsigned int new_height)
{
  max_width = new_width;
  max_height = new_height;
}

void
Am_WinDrawonable::Set_Min_Size(unsigned int new_width, unsigned int new_height)
{
  min_width = new_width;
  min_height = new_height;
}

void
Am_WinDrawonable::Set_Visible(bool vis)
{
  // if uses buffering & getting invisible, switch buffer on & fill it
  if (UsesBufferring() && !vis && Get_Visible()) {
    SetBuffered(TRUE);
    FillBuffer();
  }

  if (m_hwnd) {
    // ShowWindow(m_hwnd, (vis)? SW_SHOWNA : SW_HIDE);
    // If windows that have been hidden are made visible using SW_SHOWNA
    // they come up inactive.  So it is necessary to explicitly say whether
    // a hidden window should activated or not when it is made visible!
    if (vis)
      if (title_bar)
        ShowWindow(m_hwnd, SW_SHOW);
      else
        ShowWindow(m_hwnd, SW_SHOWNOACTIVATE);
    else
      ShowWindow(m_hwnd, SW_HIDE);
  } else
    visible = vis;
}

void
Am_WinDrawonable::Set_Titlebar(bool new_title_bar)
{
  title_bar = new_title_bar;
  border_width = title_bar ? 2 : 0;
  m_style = WndStyleFromData();
  if (m_hwnd) {
    DestroyWindow(WndDetach());
    WndCreate();
  }
}

void
Am_WinDrawonable::Set_Background_Color(Am_Style new_color)
{
  background_color = new_color;
  if (m_hwnd) {
    InvalidateRect(m_hwnd, (0L), TRUE);
    UpdateWindow(m_hwnd);
  }
}

// destination for bitblt is the Am_Drawonable this message is sent to
void
Am_WinDrawonable::Bitblt(int d_left, int d_top, int width, int height,
                         Am_Drawonable *source, int s_left, int s_top,
                         Am_Draw_Function df)
{
  Am_WinDC dcSrc(Narrow(source));
  Am_WinDC dcDst(this);
  /*	Am_WinDC dcMem(dc);

	HBITMAP hbmp = CreateCompatibleBitmap(dc, s_width, s_height);
	HBITMAP hbmpOld = SelectObject(dcMem, hbmp);

	BitBlt(dcMem, 0, 0, s_width, s_height, dc, s_left, s_top, SRCCOPY);

	DeleteObject(SelectObject(dcMemDC, hbmpOld));
	*/
  BitBlt(dcDst, d_left, d_top, width, height, dcSrc, s_left, s_top,
         WinRopFunc(df));
}

void
Am_WinDrawonable::Clear_Area(int c_left, int c_top, int c_width, int c_height)
{
  if (m_hwnd || IsBuffered()) {
    Am_WinDC dc(this);
    // PREPARE_DATA(Am_Style, bkgnd, background_color)
    Am_Style_Data *bkgnd = Am_Style_Data::Narrow(background_color);

    if (bkgnd)
      SelectObject(dc, bkgnd->WinBrush(dc));
    else
      SelectObject(dc, (HBRUSH)GetClassLong(WndGet(), GCL_HBRBACKGROUND));

    PatBlt(dc, c_left, c_top, c_width, c_height, PATCOPY);
    if (bkgnd)
      bkgnd->Release(); // DISCARD_DATA(bkgnd)
  }
}

void
Am_WinDrawonable::Fill_Area(Am_Style fs, int c_left, int c_top, int c_width,
                            int c_height)
{
  if (fs.Valid() && (m_hwnd || IsBuffered())) {
    Am_Style_Data *wfs = Am_Style_Data::Narrow(fs);

    Am_WinDC dc(this);
    SelectObject(dc, wfs->WinBrush(dc));
    PatBlt(dc, c_left, c_top, c_width, c_height, PATCOPY);

    wfs->Release();
  }
}

void
Am_WinDrawonable::Flush_Output()
{
  return;
  // af1x 6-19-95
  // Flush_Output is called after we've done some drawing at the opal level.
  // It's needed in X to make sure X flushes its output buffer.  In Windows
  // this is a no-op.
}

void Am_WinDrawonable::Translate_Coordinates(int src_x, int src_y,
                                             /*const*/ Am_Drawonable *src_d,
                                             int &dest_x_return,
                                             int &dest_y_return) //const
{
  POINT pt;
  pt.x = src_x;
  pt.y = src_y;

  if (src_d != m_root)
    ClientToScreen(Narrow(src_d)->WndGet(), &pt);
  if (this != m_root)
    ScreenToClient(m_hwnd, &pt);

  dest_x_return = pt.x;
  dest_y_return = pt.y;
}

void
Am_WinDrawonable::Translate_From_Virtual_Source(int src_x, int src_y,
                                                bool title_bar,
                                                int border_width,
                                                int &dest_x_return,
                                                int &dest_y_return)
{
  DWORD style = title_bar ? WS_OVERLAPPED : WS_POPUP;

  if (border_width)
    style |= WS_THICKFRAME;

  if (title_bar) {
    style |= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    // We can't have title bar without thick frame or at least border:
    if (!border_width)
      style |= WS_BORDER;
  }

  DWORD save_style = m_style;
  m_style = style;
  int bl, bt, br, bb, dummy;
  Inquire_Window_Borders(bl, bt, br, bb, dummy, dummy);
  m_style = save_style;

  dest_x_return = src_x + bl;
  dest_y_return = src_y + bt;
}

void
Am_WinDrawonable::Get_Image_Size(const Am_Image_Array &image, int &ret_width,
                                 int &ret_height)
{
  image.Get_Size(ret_width, ret_height);
}

// Verifies that bitmap has a correct filename and loaded correctly.
bool
Am_WinDrawonable::Test_Image(const Am_Image_Array &image) const
{
  return image.Valid();
}

int Am_WinDrawonable::Get_Char_Width(const Am_Font &font, char c) //const
{
  return Get_String_Width(font, &c, 1);
}

int Am_WinDrawonable::Get_String_Width(const Am_Font &font, const char *s,
                                       int len) //const
{
  int swidth, dummy;

  Get_String_Extents(font, s, len, swidth, dummy, dummy, dummy, dummy);

  return swidth;
}

void Am_WinDrawonable::Get_String_Extents(const Am_Font &font, const char *s,
                                          int len, int &swidth, int &ascent,
                                          int &descent, int &left_bearing,
                                          int &right_bearing) //const
{
  PREPARE_DATA(Am_Font, fntd, font)

  if (!fntd) {
    len = swidth = 0;
    ascent = descent = 0;
    left_bearing = right_bearing = 0;
    return;
  }

  Am_WinDC dc(this);
  SelectObject(dc, fntd->WinFont(dc));

#if defined(_WIN32)
  SIZE sz;
  GetTextExtentPoint32(dc, s, len, &sz); //win32
  swidth = sz.cx;
#else
  swidth = LOSHORT(GetTextExtent(dc, s, len));
#endif

  TEXTMETRIC tm;
  GetTextMetrics(dc, &tm);
  ascent = tm.tmAscent + tm.tmExternalLeading;
  descent = sz.cy - tm.tmAscent; //we don't use tm.tmDescent, so that
  //ascent + descent = tmHeight
  // Evil Hack!  Anyone have a more general solution?
  if (tm.tmUnderlined && fntd->Size() == Am_FONT_MEDIUM)
    descent += 1;
  // tm.tmHeight
  if (tm.tmOverhang) {
    left_bearing = tm.tmOverhang / 2;
    right_bearing = tm.tmOverhang;
    swidth += tm.tmOverhang + tm.tmOverhang / 2;
  } else if (fntd->IsItalic()) {
    left_bearing = tm.tmAveCharWidth / 2;
    right_bearing = tm.tmAveCharWidth;
    swidth += tm.tmAveCharWidth + tm.tmAveCharWidth / 2;
  } else {
    left_bearing = right_bearing = 0;
  }
  // swidth += left_bearing + right_bearing; - don't do it, because
  // left_bearing & right_bearing may be the same variable!

  DISCARD_DATA(fntd)
}

// The max ascent and descent include vertical spacing between rows
// of text.   The min ascent and descent are computed on a per-char basis.
void Am_WinDrawonable::Get_Font_Properties(const Am_Font &font,
                                           int &max_char_width,
                                           int &min_char_width,
                                           int &max_char_ascent,
                                           int &max_char_descent) //const
{
  PREPARE_DATA(Am_Font, fntd, font)

  if (!fntd) {
    max_char_width = 0;
    min_char_width = 0;
    max_char_ascent = 0;
    max_char_descent = 0;
    return;
  }

  Am_WinDC dc(this);
  SelectObject(dc, fntd->WinFont(dc));

  TEXTMETRIC tm;
  GetTextMetrics(dc, &tm);
  max_char_width = tm.tmMaxCharWidth;
  min_char_width = tm.tmAveCharWidth;
  if (tm.tmOverhang) {
    max_char_width += tm.tmOverhang + tm.tmOverhang / 2;
    min_char_width += tm.tmOverhang + tm.tmOverhang / 2;
  } else if (fntd->IsItalic()) {
    max_char_width += tm.tmAveCharWidth + tm.tmAveCharWidth / 2;
    min_char_width += tm.tmAveCharWidth + tm.tmAveCharWidth / 2;
  }
  max_char_ascent = tm.tmAscent + tm.tmExternalLeading;
  max_char_descent = tm.tmDescent;
  // Evil Hack!  Anyone have a more general solution?
  if (tm.tmUnderlined && fntd->Size() == Am_FONT_MEDIUM)
    max_char_descent += 1;
  DISCARD_DATA(fntd)
}

void
Am_WinDrawonable::Get_Polygon_Bounding_Box(const Am_Point_List &pl,
                                           const Am_Style &ls, int &out_left,
                                           int &out_top, int &width,
                                           int &height)
{
  int right, bottom, left, top;

  short thick;
  Am_Line_Cap_Style_Flag cap;
  Am_Join_Style_Flag join;
  Am_Line_Solid_Flag solid;
  const char *dash;
  int dashl;
  Am_Fill_Solid_Flag fill;
  Am_Fill_Poly_Flag poly;
  Am_Image_Array stipple;

  ls.Get_Values(thick, cap, join, solid, dash, dashl, fill, poly, stipple);

  thick = (join == Am_JOIN_MITER) ? thick * 2 : thick / 2;
  pl.Get_Extents(left, top, right, bottom);

  out_left = left - thick;
  out_top = top - thick;
  width = right - left + 1 + 2 * thick;
  height = bottom - top + 1 + 2 * thick;
}

void
Am_WinDrawonable::Clear_Clip()
{
  delete m_clip;
  m_clip = (0L);
}

void
Am_WinDrawonable::Set_Clip(Am_Region *the_region)
{
  Clear_Clip();
  m_clip = new Am_WinRegion(*Am_WinRegion::Narrow(the_region));
}

void
Am_WinDrawonable::Set_Clip(int the_left, int the_top, unsigned int the_width,
                           unsigned int the_height)
{
  Clear_Clip();
  m_clip = new Am_WinRegion(the_left, the_top, the_width, the_height);
}

void
Am_WinDrawonable::Push_Clip(/*const*/ Am_Region *the_region)
{
  if (!m_clip)
    m_clip = new Am_WinRegion(*Am_WinRegion::Narrow(the_region));
  else
    m_clip->Push(the_region);
}

void
Am_WinDrawonable::Push_Clip(int the_left, int the_top, unsigned int the_width,
                            unsigned int the_height)
{
  if (!m_clip)
    m_clip = new Am_WinRegion(the_left, the_top, the_width, the_height);
  else
    m_clip->Push(the_left, the_top, the_width, the_height);
}

void
Am_WinDrawonable::Pop_Clip()
{
  if (m_clip) {
    if (m_clip->IsStackEmpty()) {
      delete m_clip;
      m_clip = (0L);
    } else
      m_clip->Pop();
  }
}

bool
Am_WinDrawonable::In_Clip(int x, int y)
{
  return Get_Clip()->In(x, y);
}

bool
Am_WinDrawonable::In_Clip(int left, int top, unsigned int width,
                          unsigned int height, bool &total)
{
  return Get_Clip()->In(left, top, width, height, total);
}

bool
Am_WinDrawonable::In_Clip(Am_Region *rgn, bool &total)
{
  return Get_Clip()->In(rgn, total);
}

void
Am_WinDrawonable::Draw_Arc(const Am_Style &ls, const Am_Style &fs, int left,
                           int top, unsigned int width, unsigned int height,
                           int angle1, int angle2, Am_Draw_Function f,
                           Am_Arc_Style_Flag asf)
{
  Am_WinDC dc(this);

  int border = 0;
  if (!ls.Valid())
    border = 1;

  if (angle2 % 360) {
    if (width == 0 || height == 0)
      return;

    POINT cntr;
    cntr.x = left + width / 2;
    cntr.y = top + height / 2;
    double mult;
    if (width > height)
      mult = (double)SHRT_MAX * width / (2 * height);
    else
      mult = (double)SHRT_MAX * height / (2 * width);

    POINT start;
    double angle = angle1 * (3.14159 / 180.0);
    //start.x = (int)((SHRT_MAX / 2) * cos(angle)) + cntr.x;
    //start.y = cntr.y - (int)((SHRT_MAX / 2) * sin(angle));
    start.x = (int)(mult * width * cos(angle) / height) + cntr.x;
    start.y = cntr.y - (int)(mult * height * sin(angle) / width);

    POINT end;
    angle += angle2 * (3.14159 / 180.0);
    //end.x = (int)((SHRT_MAX / 2) * cos(angle)) + cntr.x;
    //end.y = cntr.y - (int)((SHRT_MAX / 2) * sin(angle));
    end.x = (int)(mult * width * cos(angle) / height) + cntr.x;
    end.y = cntr.y - (int)(mult * height * sin(angle) / width);

    if (fs.Valid()) {
      if (is_transparent_stippled(fs) && f == Am_DRAW_COPY) {
        // What about other draw functions?

        dc.Set_DC_Transparent_Stipple(Am_No_Style, fs);

        switch (asf) {
        case Am_ARC_CHORD:
          Chord(dc, left, top, left + width + border, top + height + border,
                start.x, start.y, end.x, end.y);
          break;
        case Am_ARC_PIE_SLICE:
          Pie(dc, left, top, left + width + border, top + height + border,
              start.x, start.y, end.x, end.y);
          break;
        }

        dc.Adjust_DC_Transparent_Stipple(fs);

        switch (asf) {
        case Am_ARC_CHORD:
          Chord(dc, left, top, left + width + border, top + height + border,
                start.x, start.y, end.x, end.y);
          break;
        case Am_ARC_PIE_SLICE:
          Pie(dc, left, top, left + width + border, top + height + border,
              start.x, start.y, end.x, end.y);
          break;
        }
      } else {
        dc.Set_DC(Am_No_Style, fs, f);
        switch (asf) {
        case Am_ARC_CHORD:
          Chord(dc, left, top, left + width + border, top + height + border,
                start.x, start.y, end.x, end.y);
          break;
        case Am_ARC_PIE_SLICE:
          Pie(dc, left, top, left + width + border, top + height + border,
              start.x, start.y, end.x, end.y);
          break;
        }
      }
    }
    if (ls.Valid()) {
      dc.Set_DC(ls, Am_No_Style, f, 1); // Emulate dashed line
      Arc(dc, left, top, left + width, top + height, start.x, start.y, end.x,
          end.y);
    }
  } else {                   //angle2 % 360 != 0
    dc.Set_DC(ls, fs, f, 1); // Emulate dashed line
    Ellipse(dc, left, top, left + width + border, top + height + border);
  }
}

void
Am_WinDrawonable::Draw_Image(
    int left, int top, int width, int height, const Am_Image_Array &image,
    int i_left, int i_top,
    const Am_Style &ls,   //color of 'on' bits
    const Am_Style &fs,   //for background behind image
    bool draw_monochrome, //use ls and fs in place of colors
    Am_Draw_Function f)
{
  if (!image.Valid())
    return;

  Am_Image_Array_Data *imgd = Am_Image_Array_Data::Narrow(image);

  int iw, ih;
  imgd->Get_Size(iw, ih);
  if (width != -1)
    iw = width;
  if (height != -1)
    ih = height;

  Am_WinDC dc(this);

  // January '97:
  // instead of treating monochrome images as a special case
  // use more general draw_monochrome parameter
  //  leave old code in place for V3.0 alpha release
  /* begin old code to draw 2 color images transparently
	BOOL bTransparent = (imgd -> WinNumColors() == 2) &&
	(!WFS || WFS -> FillSolid() != Am_FILL_OPAQUE_STIPPLED);

	if (bTransparent) {					
	// To draw transparent color properly, first AND the on bits with 
	// black (0, to turn them off) and the off bits with white (FF,
	// to keep them the same); then OR with the correct color on a 
	// black background.

	SetBkColor(dc, Am_Style_Data::crefWhite);
	SetTextColor(dc, Am_Style_Data::crefBlack);	  
	imgd -> WinShow(dc, i_left, i_top, left, top, iw, ih, false,
	    false, false, SRCAND);
	  
	SetBkColor(dc, Am_Style_Data::crefBlack);
	SetTextColor(dc, WLS?  WLS -> WinPenColor() : Am_Style_Data::crefBlack);
	imgd -> WinShow(dc, i_left, i_top, left, top, iw, ih, false,
	false, false, SRCPAINT);

	if (imgd) imgd->Release();
	if (wls) wls->Release();
	if (wfs) wfs->Release();
	return;
	}
	*/

  bool invert = false;
  bool masked = false;

  if (f == Am_DRAW_MASK_COPY)
    draw_monochrome = true;

  if (draw_monochrome) {
    // both fore and background are transparent
    if (!fs.Valid() && !ls.Valid())
      return;
    if (!fs.Valid() || !ls.Valid()) {
      // one of fore or background is transparent
      masked = true;
      if (!ls.Valid())
        invert = true;
    }
    dc.Set_DC_Text(ls, fs, f, false);
  }
  if (imgd->WinIsTransparent()) {
    // colored foreground on filled background
    if (fs.Valid() && !masked) {
      // puzzling: calling Fill_Area directly doesn't work
      dc.Set_DC(Am_No_Style, fs, f);
      PatBlt((HDC)(dc), left, top, iw, ih, PATCOPY);
    }
    masked = true;
  }
  imgd->WinShow(dc, i_left, i_top, left, top, iw, ih, masked, draw_monochrome,
                invert, WinRopFunc(f));

  if (imgd)
    imgd->Release();
}

void
Am_WinDrawonable::Draw_Line(const Am_Style &ls, int x1, int y1, int x2, int y2,
                            Am_Draw_Function f)
{
  if (!ls.Valid())
    return;

  Am_Style_Data *wls = Am_Style_Data::Narrow(ls); //  PREPARE_WLS
  Am_WinDC dc(this);
  dc.Set_DC(ls, Am_No_Style, f);

  if ((wls->LineCap() == Am_CAP_NOT_LAST) ||
      (wls->LineCap() == Am_CAP_BUTT) && (wls->m_line_thickness > 1)) {
    int lx = x2 - x1;
    int ly = y1 - y2;
    int l = (int)sqrt((double)(lx * lx + ly * ly));
    if (!l) { //length == 0
      wls->Release();
      return;
    }
    int w2 = wls->m_line_thickness / 2 + (wls->LineCap() == Am_CAP_NOT_LAST);

    int dx = MulDiv(lx, w2, l); //lx * w2 / l;
    int dy = MulDiv(ly, w2, l); //ly * w2 / l;

    x1 += dx;
    y1 -= dy;

    x2 -= dx;
    y2 += dy;
  }

  if (wls->LineSolid() == Am_LINE_SOLID) {
    MoveTo(dc, x1, y1);
    LineTo(dc, x2, y2);
  } else
    Am_WinDrawArrDashLine(dc, m_inst, x1, y1, x2, y2, wls->DashArr(),
                          wls->DashArrLen(), wls->m_line_thickness,
                          wls->LineSolid() == Am_LINE_ON_OFF_DASH);
  if (wls)
    wls->Release(); // DISCARD_WLS
}

//void Am_WinDrawonable::Draw_Lines (Am_Style ls, Am_Style fs,
//	Am_Point_List pl, Am_Draw_Function f)
//{
//	PREP_WLS
//
//	Am_WinDC dc(this);
//	SetROP2(dc, WinRop2Func(f));
//
//	Am_Point_List_Data* pld = Am_Point_List_Data::Narrow(pl);
//
//	SelectObject(dc, WLS? WLS -> WinPen() : Am_Style_Data::hpenNullPen);
//
//	if (!WLS || WLS -> LineSolid() == Am_LINE_SOLID)
//		Polyline(dc, (POINT*)(&(*pld)[0]), pld -> Last() + 1);
//	else
//		Am_WinDrawArrDashPolyline(dc, m_inst, (POINT*)(&(*pld)[0]), pld -> Last() + 1,
//			WLS -> DashArr(), WLS -> DashArrLen(),
//			WLS -> Thikness(), WLS -> LineSolid() == Am_LINE_ON_OFF_DASH);
//}

void
Am_WinDrawonable::poly_draw_it(Am_Style ls, Am_Style fs, Am_Draw_Function f,
                               POINT *pld, int num_points)
{
  Am_WinDC dc(this);

  if (fs.Valid()) {
    Am_Style_Data *wfs = Am_Style_Data::Narrow(fs); //  PREPARE_WFS
    SetPolyFillMode(dc, WinPolyFillMode(wfs->FillPolyMode()));
    if (wfs)
      wfs->Release(); // DISCARD_WFS

    if (is_transparent_stippled(fs) && f == Am_DRAW_COPY) {
      // What about other draw functions?

      dc.Set_DC_Transparent_Stipple(Am_No_Style, fs);
      Polygon(dc, pld, num_points);

      dc.Adjust_DC_Transparent_Stipple(fs);
      Polygon(dc, pld, num_points);
    } else {
      dc.Set_DC(Am_No_Style, fs, f);
      Polygon(dc, pld, num_points);
    }
  }

  if (ls.Valid()) {
    dc.Set_DC(ls, Am_No_Style, f);
    Am_Style_Data *wls = Am_Style_Data::Narrow(ls); //  PREPARE_WLS
    if (wls->LineSolid() == Am_LINE_SOLID)
      Polyline(dc, pld, num_points);
    else
      Am_WinDrawArrDashPolyline(dc, m_inst, pld, num_points, wls->DashArr(),
                                wls->DashArrLen(), wls->m_line_thickness,
                                wls->LineSolid() == Am_LINE_ON_OFF_DASH);
    if (wls)
      wls->Release(); // DISCARD_WLS
  }
}

void
Am_WinDrawonable::Draw_Lines(const Am_Style &ls, const Am_Style &fs,
                             const Am_Point_Array &pl, Am_Draw_Function f)
{
  PREPARE_DATA(Am_Point_Array, data, pl)
  if (!data)
    return;
  poly_draw_it(ls, fs, f, data->ar, data->size);
  DISCARD_DATA(data)
}

void
Am_WinDrawonable::Draw_2_Lines(const Am_Style &ls, const Am_Style &fs, int x1,
                               int y1, int x2, int y2, int x3, int y3,
                               Am_Draw_Function f)
{
  POINT ar[3];
  ar[0].x = x1;
  ar[0].y = y1;
  ar[1].x = x2;
  ar[1].y = y2;
  ar[2].x = x3;
  ar[2].y = y3;
  poly_draw_it(ls, fs, f, ar, 3); // 3 is number of points
}

void
Am_WinDrawonable::Draw_3_Lines(const Am_Style &ls, const Am_Style &fs, int x1,
                               int y1, int x2, int y2, int x3, int y3, int x4,
                               int y4, Am_Draw_Function f)
{
  POINT ar[4];
  ar[0].x = x1;
  ar[0].y = y1;
  ar[1].x = x2;
  ar[1].y = y2;
  ar[2].x = x3;
  ar[2].y = y3;
  ar[3].x = x4;
  ar[3].y = y4;
  poly_draw_it(ls, fs, f, ar, 4); // 4 is number of points
}

void
Am_WinDrawonable::Draw_Rectangle(const Am_Style &ls, const Am_Style &fs,
                                 int left, int top, int width, int height,
                                 Am_Draw_Function f)
{
  if (width <= 0 || height <= 0)
    return;

  int border = 0;
  if (!ls.Valid())
    border = 1;

  // If we have complex case, we'll draw it as polygon

  if ((is_transparent_stippled(fs) && f == Am_DRAW_COPY) ||
      ls.Get_Line_Flag() != Am_LINE_SOLID) {
    POINT ar[5];
    ar[0].x = left;
    ar[0].y = top;
    ar[1].x = left + width;
    ar[1].y = top;
    ar[2].x = left + width;
    ar[2].y = top + height;
    ar[3].x = left;
    ar[3].y = top + height;
    ar[4].x = left;
    ar[4].y = top;
    poly_draw_it(Am_No_Style, fs, f, ar, 5);
    if (!border) {
      --ar[1].x;
      --ar[2].x;
      --ar[2].y;
      --ar[3].y;
      poly_draw_it(ls, Am_No_Style, f, ar, 5);
    }
  }
  // else it's just solid-lined opaque-filled case, optimized
  else {
    Am_WinDC dc(this);
    dc.Set_DC(ls, fs, f);
    Rectangle((HDC)(dc), left, top, left + width + border,
              top + height + border);
  }
}

void
Am_WinDrawonable::Draw_Roundtangle(const Am_Style &ls, const Am_Style &fs,
                                   int left, int top, int width, int height,
                                   unsigned short x_radius,
                                   unsigned short y_radius, Am_Draw_Function f)
{
  Am_WinDC dc(this);

  int border = 0;
  if (!ls.Valid())
    border = 1;

  if (is_transparent_stippled(fs) && f == Am_DRAW_COPY) {
    // What about other draw functions?

    dc.Set_DC_Transparent_Stipple(ls, fs, 1); // Emulate dashed line
    RoundRect(dc, left, top, left + width + border, top + height + border,
              x_radius * 2, y_radius * 2);

    dc.Adjust_DC_Transparent_Stipple(fs);
    RoundRect(dc, left, top, left + width + border, top + height + border,
              x_radius * 2, y_radius * 2);
  } else {
    dc.Set_DC(ls, fs, f, 1); // Emulate dashed line
    RoundRect(dc, left, top, left + width + border, top + height + border,
              x_radius * 2, y_radius * 2);
  }
}

void
Am_WinDrawonable::Draw_Text(const Am_Style &ls, const char *s, int str_len,
                            const Am_Font &font, int left, int top,
                            Am_Draw_Function f, const Am_Style &fs, bool invert)
{
  if (!font.Valid())
    return;

  Am_WinDC dc(this);
  dc.Set_DC_Font(font);
  dc.Set_DC_Text(ls, fs, f, invert);

  TextOut(dc, left, top, s, str_len); //win32
}

void
Am_WinDrawonable::Set_Cut_Buffer(const char *s)
{
  if (s) {
    // allocate memory and copy the string to it
    HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, strlen(s) + 1);

    if (!hData)
      return; // out of memory

    LPSTR lpData = (LPSTR)GlobalLock(hData);
    if (!lpData) { // out of memory
      GlobalFree(hData);
      return;
    }
    strcpy(lpData, s);
    GlobalUnlock(hData);

    // clear the current contents of the clipboard, and set
    // the data handle to the new string.

    if (OpenClipboard(m_hwnd)) {
      EmptyClipboard();
      SetClipboardData(CF_TEXT, hData);
      CloseClipboard();
    }
  }
}

char *
Am_WinDrawonable::Get_Cut_Buffer()
{
  if (OpenClipboard(m_hwnd)) { // get text from the clipboard
    HGLOBAL hClipData = GetClipboardData(CF_TEXT);
    if (!hClipData) {
      CloseClipboard();
      return (0L);
    }

    char *s = new char[GlobalSize(hClipData)];
    if (!s) { // out of memory
      CloseClipboard();
      return (0L);
    }

    LPSTR lpClipData = (LPSTR)GlobalLock(hClipData);
    if (!lpClipData) { // out of memory
      CloseClipboard();
      delete[] s;
      return (0L);
    }

    strcpy(s, lpClipData);
    GlobalUnlock(hClipData);
    CloseClipboard();

    return s;
  }
  return (0L);
}

void
Am_WinDrawonable::Set_Enter_Leave(bool want_enter_leave_events)
{
  m_want_enter_leave_events = want_enter_leave_events;
}

void
Am_WinDrawonable::Set_Want_Move(bool want_move_events)
{
  m_want_move_events = want_move_events;
}

void
Am_WinDrawonable::Set_Multi_Window(bool want_multi_window)
{
  m_want_multi_window = want_multi_window;
}

void Am_WinDrawonable::Get_Window_Mask(bool &want_enter_leave_events,
                                       bool &want_move_events,
                                       bool &want_multi_window) //const
{
  want_enter_leave_events = m_want_enter_leave_events;
  want_move_events = m_want_move_events;
  want_multi_window = m_want_multi_window;
}

void
Am_WinDrawonable::Discard_Pending_Events()
{
  if (m_hwnd) {
    MSG msg;
    ValidateRect(m_hwnd, (0L)); // discard WM_PAINT
    while (PeekMessage(&msg, m_hwnd, 0, 0, PM_REMOVE | PM_NOYIELD))
      ;
  }
}

void
Am_WinDrawonable::Set_Cursor(Am_Cursor new_cursor) //can't have color cursors
{
  cursor = new_cursor;
}

// Windows class management functions

const char *
Am_WinDrawonable::ClsGetName(bool fSaveBits) const
{
  return fSaveBits ? Am_WinDrawonable_SBT_CLASS : Am_WinDrawonable_CLASS;
}

BOOL
Am_WinDrawonable::WinDoIdle()
{
#ifdef USE_WINCLEANER
  return g_cleaner->CheckResources();
#else
  return FALSE; // I have no idea what this return value should be (rcm),
                // but CheckResources returns FALSE if it needs no more
                // running time
#endif
}

static char *
skip_string(char *cmdline)
{
  cmdline++;
  while (*cmdline != '"') {
    if (*cmdline == 0)
      return cmdline;
    else if (*cmdline == '\\')
      cmdline++;

    cmdline++;
  }
  return cmdline;
}

static char *
skip_whitespace(char *cmdline)
{
  while (TRUE) {
    if (*cmdline == 0)
      return 0;
    else if ((*cmdline == ' ') || (*cmdline == '\t'))
      *cmdline = 0;
    else
      return cmdline;

    cmdline++;
  }
}

static char *
find_whitespace(char *cmdline)
{
  while (TRUE) {
    if ((*cmdline == ' ') || (*cmdline == '\t'))
      return cmdline;
    else if (*cmdline == '"')
      cmdline = skip_string(cmdline);

    if (*cmdline == 0)
      return cmdline;
    cmdline++;
  }
}

#define NUMCMDS 64

static int
parse_cmdline(HINSTANCE hInst, char *cmdline, char *argv[])
{
  int argc = 1;

  static char filename[256];
  GetModuleFileName(hInst, filename, sizeof(filename));

  argv[0] = filename;
  while (TRUE) {
    cmdline = skip_whitespace(cmdline);
    if (cmdline == 0) {
      return argc;
    } else {
      if (argc == NUMCMDS)
        return argc;
      argv[argc] = cmdline;
      argc++;
    }
    cmdline = find_whitespace(cmdline);
  }
  argv[argc] = (0L);
}

// Windows window management functions
#if !defined(_WIN32)
Am_Map_Int2Ptr Am_WinDrawonable::m_mapWnd2Drw;
#else
Am_Map_HModule2Ptr Am_WinDrawonable::m_mapWnd2Drw;
#endif

// Robby #if defined(_WIN32)

#if defined(DLL)

BOOL APIENTRY
DllMain(HANDLE hInst, DWORD reason, PVOID res)
{
  switch (reason) {
  case DLL_PROCESS_ATTACH:
    break;
  case DLL_PROCESS_DETACH:
    Am_Cleanup();
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
    break;
  }
  return TRUE;
}

#else

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine,
        int nCmdShow)
{
  Am_WinDrawonable::m_inst = hInstance;
  DWORD dwVersion = ::GetVersion();

  if (dwVersion >= 0x80000000 && LOBYTE(LOWORD(dwVersion)) < 4) {
    char szBuf[MAX_PATH];
    if (!GetEnvironmentVariable("AMULET_DIR", szBuf, sizeof(szBuf) - 1))
      lstrcpy(szBuf, "C:\\AMULET");

    lstrcat(szBuf, "\\lib\\PC\\GWTHNK32.DLL");

    Am_WinDrawonable::m_hThunk32s = (HMODULE)LoadLibrary(szBuf);
    if (!Am_WinDrawonable::m_hThunk32s) {
      MessageBox((0L), strcat(szBuf, ": cannot load DLL"),
                 "GemW Initialisation Error", MB_ICONEXCLAMATION | MB_OK);
      return 1;
    }
  }

  Am_WinDrawonable::WinInit();

  // FIX: should be parsing this stupid command line
  extern int main(int argc, char **argv);

  char *argv[NUMCMDS];
  int argc = parse_cmdline(hInstance, lpszCmdLine, argv);
  if (argc == NUMCMDS)
    Am_ERROR("Too many command line arguments: limit is " << NUMCMDS);

  int res = main(argc, argv);

  Am_WinDrawonable::WinUninit();
  //	return res;
  return (0);
}
#endif

// MainWndProc
// This procedure handles messages for all amulet windows.  Events that
// are sometimes not queued on Windows' message queue are put onto our
// own m_msgqueue to be processed later, when we have enough information
// to deal with them.  Input events and other always-queued events are
// processed directly here.
// ProcessQueuedEvents deals with the events on m_msgqueue.  Process_Event
// and Process_Immediate_Event call ProcessQueuedEvents and also pull
// events off the windows message queue to be dealt with.

#pragma warning(disable : 4759) //'segment lost' warning off
LRESULT CALLBACK __export
Am_WinDrawonable::MainWndProc(HWND hWnd, UINT message, WPARAM wParam,
                              LPARAM lParam)
{
  Am_WinDrawonable *drw;

  switch (message) {
  case WM_CREATE:
    drw = Narrow((void *)(((CREATESTRUCT FAR *)lParam)->lpCreateParams));
    drw->m_hwnd = hWnd;
    m_mapWnd2Drw[hWnd] = drw;
    break;
  case WM_CLOSE:
    drw = Narrow(m_mapWnd2Drw.GetAt(hWnd));
    if (drw && drw->m_evh) {
      drw->m_evh->Destroy_Notify(drw);
      //Am_Main_Loop_Go = false;
      return 0L;
    }
    break;
  //case WM_DESTROY:
  //  drw = Narrow(m_mapWnd2Drw.GetAt(hWnd));
  //  if (drw && drw -> m_evh) {
  //    drw -> m_evh -> Destroy_Notify(drw);
  //  }
  //  break;
  //case WM_NCDESTROY:
  //  drw = Narrow(m_mapWnd2Drw.GetAt(hWnd));
  //  if (drw) {
  //    m_mapWnd2Drw.DeleteKey(hWnd);
  //	drw -> WndDetach();
  //	// delete drw; //don't die if window dies!
  //	if (m_mapWnd2Drw.Count() == 1 /*only root*/) {
  //	  WinUninit();
  //	  PostQuitMessage(0);
  //	}
  //  }
  //  break;
  case WM_ERASEBKGND:
    drw = Narrow(m_mapWnd2Drw.GetAt(hWnd));
    if (drw)
      return drw->WndOnEraseBkgnd((HDC)wParam);
    break;
  case WM_PAINT: {
    // if there is no update rect we don't need to do an update.
    tagRECT update_rect;
    GetUpdateRect(hWnd, &update_rect, FALSE);
    if (update_rect.left || update_rect.top || update_rect.bottom ||
        update_rect.right) {
      // we have a valid update rect, so update
      drw = Narrow(m_mapWnd2Drw.GetAt(hWnd));
      if (drw) {
        PAINTSTRUCT ps;
        // The BeginPaint call is used only to get the correct rectangle
        // to update.  The update event is queued, and the DC isn't
        // used here; a new one is found when drawing commences.
        // BeginPaint validates the update region automatically so we
        // don't have to do that explicitly.
        // (even though the actual drawing doesn't occur between BeginPaint and
        // EndPaint, it works)	af1x
        BeginPaint(hWnd, &ps);
        drw->WndOnPaint(ps);
        EndPaint(hWnd, &ps);
        return 0L;
      }
    }
    break;
  }
  case WM_SIZE:
    drw = Narrow(m_mapWnd2Drw.GetAt(hWnd));
    if (drw) {
      drw->WndOnSize(wParam, LOSHORT(lParam), HISHORT(lParam));
      return 0L;
    }
    break;
  case WM_MOVE:
    drw = Narrow(m_mapWnd2Drw.GetAt(hWnd));
    if (drw) {
      drw->WndOnMove(LOSHORT(lParam), HISHORT(lParam));
      return 0L;
    }
    break;
  case WM_GETMINMAXINFO:
    drw = Narrow(m_mapWnd2Drw.GetAt(hWnd));
    if (drw) {
      drw->WndOnGetMinMax((MINMAXINFO __far *)lParam);
      return 0L;
    }
  case WM_SHOWWINDOW:
    drw = Narrow(m_mapWnd2Drw.GetAt(hWnd));
    if (drw) {
      drw->WndOnShow((BOOL)wParam);
      return 0L;
    }
  case WM_LBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_RBUTTONDOWN:
  case WM_LBUTTONDBLCLK:
  case WM_MBUTTONDBLCLK:
  case WM_RBUTTONDBLCLK:
    drw = Narrow(m_mapWnd2Drw.GetAt(hWnd));
    if (drw) {
      short sType;
      Am_Button_Down down;
      Am_Click_Count click;
      switch (message) {
      case WM_LBUTTONDOWN:
        sType = Am_LEFT_MOUSE;
        down = Am_BUTTON_DOWN;
        click = Am_SINGLE_CLICK;
        SetFocus(hWnd);
        SetCapture(hWnd);
        break;
      case WM_MBUTTONDOWN:
        sType = Am_MIDDLE_MOUSE;
        down = Am_BUTTON_DOWN;
        click = Am_SINGLE_CLICK;
        SetFocus(hWnd);
        SetCapture(hWnd);
        break;
      case WM_RBUTTONDOWN:
        sType = Am_RIGHT_MOUSE;
        down = Am_BUTTON_DOWN;
        click = Am_SINGLE_CLICK;
        SetFocus(hWnd);
        SetCapture(hWnd);
        break;
      case WM_LBUTTONDBLCLK:
        sType = Am_LEFT_MOUSE;
        down = Am_BUTTON_DOWN;
        click = Am_DOUBLE_CLICK;
        break;
      case WM_MBUTTONDBLCLK:
        sType = Am_MIDDLE_MOUSE;
        down = Am_BUTTON_DOWN;
        click = Am_DOUBLE_CLICK;
        break;
      case WM_RBUTTONDBLCLK:
        sType = Am_RIGHT_MOUSE;
        down = Am_BUTTON_DOWN;
        click = Am_DOUBLE_CLICK;
        break;
      }
      drw->WndOnInput(sType, down, click, (wParam & MK_CONTROL) == MK_CONTROL,
                      (wParam & MK_SHIFT) == MK_SHIFT, WinKeyPressed(VK_MENU),
                      LOSHORT(lParam), HISHORT(lParam));
      return 0L;
    }
    break;
  case WM_LBUTTONUP:
  case WM_MBUTTONUP:
  case WM_RBUTTONUP:
  case WM_MOUSEMOVE: {
    // Notes af1x 6-6-95:
    // This code deals with multi-window event handling.
    drw = Narrow(m_mapWnd2Drw.GetAt(hWnd));
    if (drw->m_want_multi_window && GetCapture()) {
      POINT pt;
      pt.x = LOSHORT(lParam);
      pt.y = HISHORT(lParam);
      ClientToScreen(hWnd, &pt);
      HWND hWndM = WindowFromPoint(pt);
      Am_WinDrawonable *drwM;
      // If it's a different window,
      // and the window exists,
      // and it's associated with an Amulet drawonable,
      // and that drawonable wants multiwindow events,
      // then set that window to capture future mouse events,
      // convert the event location to new window coordinates,
      // and sent the event there.
      if (hWndM != hWnd && hWndM &&
          (drwM = Narrow(m_mapWnd2Drw.GetAt(hWndM))) &&
          drwM->m_want_multi_window &&
          (message != WM_MOUSEMOVE || drwM->m_want_move_events)) {
        ScreenToClient(hWndM, &pt);
        drw = drwM;
        lParam = MAKELONG(pt.x, pt.y);
      }
    }
    if (drw) {
      short sType;
      Am_Button_Down down;
      switch (message) {
      case WM_LBUTTONUP:
        sType = Am_LEFT_MOUSE;
        down = Am_BUTTON_UP;
        ReleaseCapture();
        break;
      case WM_MBUTTONUP:
        sType = Am_MIDDLE_MOUSE;
        down = Am_BUTTON_UP;
        ReleaseCapture();
        break;
      case WM_RBUTTONUP:
        sType = Am_RIGHT_MOUSE;
        down = Am_BUTTON_UP;
        ReleaseCapture();
        break;
      case WM_MOUSEMOVE:
        sType = Am_MOUSE_MOVED;
        down = Am_NEITHER;
        if (!drw->m_want_move_events)
          return 0L;
        break;
      }
      drw->WndOnInput(sType, down, Am_SINGLE_CLICK,
                      (wParam & MK_CONTROL) == MK_CONTROL,
                      (wParam & MK_SHIFT) == MK_SHIFT, WinKeyPressed(VK_MENU),
                      LOSHORT(lParam), HISHORT(lParam));
      return 0L;
    }
    break;
  }
  case WM_SETCURSOR:
    drw = Narrow(m_mapWnd2Drw.GetAt(hWnd));
    if (drw && LOWORD(lParam) == HTCLIENT) {
      drw->WndOnSetCursor(wParam, lParam);
      return TRUE;
    }
    break;
  case WM_CHAR:
  case WM_SYSCHAR: {
    long pos = GetMessagePos();
    POINT pt;
    pt.x = LOSHORT(pos);
    pt.y = HISHORT(pos);
    ScreenToClient(hWnd, &pt); //convert coords to this window
    drw = Narrow(m_mapWnd2Drw.GetAt(hWnd));
    if (drw) {
      drw->WndOnInput(Am_WinScanToCode(wParam, true), Am_NEITHER, Am_NOT_MOUSE,
                      WinKeyPressed(VK_CONTROL), WinKeyPressed(VK_SHIFT),
                      (bool)((HIWORD(lParam) & KF_ALTDOWN) != 0L), pt.x, pt.y);
    }
    break;
  }
  case WM_KEYDOWN:
  case WM_SYSKEYDOWN: {
    long pos = GetMessagePos();
    POINT pt;
    pt.x = LOSHORT(pos);
    pt.y = HISHORT(pos);
    ScreenToClient(hWnd, &pt); //convert coords to this window
    drw = Narrow(m_mapWnd2Drw.GetAt(hWnd));
    if (drw) {
      bool fCtrl = WinKeyPressed(VK_CONTROL) && (wParam != VK_CONTROL);

      if (fCtrl) {
        bool fCtrlCh =
            WinKeyPressed(VK_CONTROL) && (isalpha(wParam) || isdigit(wParam));
        //control-charachter
        if (fCtrlCh) {
          drw->WndOnInput(
              (WinKeyPressed(VK_SHIFT) || WinKeyToggled(VK_CAPITAL))
                  ? wParam
                  : tolower(wParam),
              Am_NEITHER, Am_NOT_MOUSE, fCtrl, WinKeyPressed(VK_SHIFT),
              (bool)((HIWORD(lParam) & KF_ALTDOWN) != 0L), pt.x, pt.y);
          return 0L;
        }
      }
      if (WinScanSpec(wParam))
        drw->WndOnInput(Am_WinScanToCode(wParam, false), Am_NEITHER,
                        Am_NOT_MOUSE, fCtrl, WinKeyPressed(VK_SHIFT),
                        (bool)((HIWORD(lParam) & KF_ALTDOWN) != 0L), pt.x,
                        pt.y);
    } //if (drw)
    break;
  }
  case WM_WININICHANGE:
    if (_stricmp((LPCSTR)lParam, "windows"))
      Am_Double_Click_Time = GetProfileInt("windows", "DoubleClickSpeed", 250);
    break;
  }
  return DefWindowProc(hWnd, message, wParam, lParam);
}
#pragma warning(default : 4759)

LRESULT
Am_WinDrawonable::DefWndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
  return DefWindowProc(m_hwnd, message, wParam, lParam);
}

LRESULT
Am_WinDrawonable::WndOnEraseBkgnd(HDC hdc)
{
  return (background_color == Am_No_Style)
             ? DefWndProc(WM_ERASEBKGND, (WPARAM)hdc, 0L)
             : 0L; // we'll handle it in WndOnPaint
}

void
Am_WinDrawonable::WndOnPaint(const PAINTSTRUCT &ps)
{
  if (ps.fErase) { // if update region needs to be erased
    //m_msgqueue.InsertAt(m_msgqueue.TailPos(),
    //	new Am_WinNotifyInfo(Am_WinDrawonable::NotifyBackground, this, ps.rcPaint));

    PREPARE_DATA(Am_Style, bkgnd, background_color)

#if _MSC_VER >= 1200
    HBRUSH hbrOld = reinterpret_cast<HBRUSH__ *>(
        SelectObject(ps.hdc,
#else
    HBRUSH hbrOld =
        SelectObject(ps.hdc,
#endif
#if !defined(_WIN32)
                     bkgnd ? bkgnd->WinBrush(ps.hdc)
                           : GetClassWord(WndGet(), GCW_HBRBACKGROUND)
#else
                     bkgnd ? bkgnd->WinBrush(ps.hdc)
                           : (HBRUSH)GetClassLong(WndGet(), GCL_HBRBACKGROUND)
#endif
#if _MSC_VER >= 1200
                         ));
#else
                         );
#endif

    /*	if (Get_Clip()) {
		RECT rc;
		if (::IntersectRect(&rc, &(Get_Clip() -> WinRect()), &ps.rcPaint))
		PatBlt(ps.hdc, rc.left, rc.top,
		rc.right - rc.left, rc.bottom - rc.top,
		PATCOPY);
		}
		else   */

    PatBlt(ps.hdc, ps.rcPaint.left, ps.rcPaint.top,
           ps.rcPaint.right - ps.rcPaint.left,
           ps.rcPaint.bottom - ps.rcPaint.top, PATCOPY);
    SelectObject(ps.hdc, hbrOld);

    DISCARD_DATA(bkgnd)
  } // end erasing

  // if window is buffred & visible, it means it just appeared on screen =>
  //std::flush buffer & switch buffering off
  if (IsBuffered() && Get_Visible()) {
    FlushBuffer();
    SetBuffered(FALSE);
  }

  WndDraw(ps.hdc, ps.rcPaint);
}

void
Am_WinDrawonable::WndOnSize(WORD fwType, int nWidth, int nHeight)
{
  bool was_icon = iconified;
  iconified = (fwType == SIZE_MINIMIZED);

  if (iconified) {
    if (icon_name)
      SetWindowText(m_hwnd, icon_name);
    if (m_evh)
      m_msgqueue.Add(
          new Am_WinNotifyInfo(Am_WinDrawonable::NotifyIconified, this, true));
  } else {          //!iconified
    if (was_icon) { //restored from icon
      SetWindowText(m_hwnd, title);
      if (m_evh)
        m_msgqueue.Add(new Am_WinNotifyInfo(Am_WinDrawonable::NotifyIconified,
                                            this, false));
    } else { //wasn't icon => simple resize
      RECT rc;
      GetClientRect(m_hwnd, &rc);

      unsigned int new_width = rc.right - rc.left;
      unsigned int new_height = rc.bottom - rc.top;
      if (new_width != width || new_height != height) {
        width = new_width;
        height = new_height;
        if (m_evh)
          m_msgqueue.Add(new Am_WinNotifyInfo(Am_WinDrawonable::NotifyConfigure,
                                              this, left, top, width, height));
      }
    }
  }
}

void
Am_WinDrawonable::WndOnMove(int xPos, int yPos)
{
  int bl, bt, br, bb, dummy;
  Inquire_Window_Borders(bl, bt, br, bb, dummy, dummy);
  int new_left = xPos - bl;
  int new_top = yPos - bt;

  if (new_left == left && new_top == top)
    return;

  left = new_left;
  top = new_top;

  // check to see if there's a move event for this window already;
  // if so, delete it.
  Am_Position pos, tmp;
  Am_WinNotifyInfo *item;
  for (pos = m_msgqueue.HeadPos(); pos; pos = m_msgqueue.NextPos(pos)) {
    item = (Am_WinNotifyInfo *)m_msgqueue.GetAt(pos);
    if (item->m_func == Am_WinDrawonable::NotifyConfigure &&
        item->m_drw == this) { // move, this drawonable
      tmp = m_msgqueue.PrevPos(pos);
      delete item; // parallel deletion code in ProcessQueuedEvents !
      m_msgqueue.DeleteAt(pos);
      pos = tmp;
    }
  }
  if (m_evh)
    m_msgqueue.Add(new Am_WinNotifyInfo(Am_WinDrawonable::NotifyConfigure, this,
                                        left, top, width, height));
}

void
Am_WinDrawonable::WndOnGetMinMax(MINMAXINFO __far *lpMMI)
{
  DefWndProc(WM_GETMINMAXINFO, 0, (LPARAM)lpMMI);
  int bl, bt, br, bb, dummy;
  Inquire_Window_Borders(bl, bt, br, bb, dummy, dummy);
  if (min_width)
    lpMMI->ptMinTrackSize.x = min_width + bl + br;
  if (min_height)
    lpMMI->ptMinTrackSize.y = min_height + bt + bb;
  if (max_width)
    lpMMI->ptMaxTrackSize.x = max_width + bl + br;
  if (max_height)
    lpMMI->ptMaxTrackSize.y = max_height + bt + bb;
}

void
Am_WinDrawonable::WndOnShow(BOOL fShow)
{
  visible = fShow;
}

void
Am_WinDrawonable::WndOnInput(short sType, Am_Button_Down down,
                             Am_Click_Count click, bool fCtrl, bool fShift,
                             bool fAlt, int xPos, int yPos)
{
  if (m_evh && (sType >= 0)) {

    Am_Current_Input_Event->input_char =
        Am_Input_Char(sType, fShift, fCtrl, fAlt, down, click);
    //the char and modifier bits; see idefs.h
    if (Am_Current_Input_Event->input_char == Am_Stop_Character) {
      Am_Main_Loop_Go = false;
      return;
    }

    Am_Current_Input_Event->x = xPos;
    Am_Current_Input_Event->y = yPos;
    Am_Current_Input_Event->draw = this; // Drawonable this event happened in
    Am_Current_Input_Event->time_stamp = GetMessageTime();
    Am_Current_Input_Event->user_id =
        Am_User_Id.Get(Am_USER_ID); // supports multiple users

    //we don't queue input events
    m_evh->Input_Event_Notify(this, Am_Current_Input_Event);

    if (Am_Debug_Print_Input_Events) {
      AM_TRACE("<><><> " << Am_Current_Input_Event->input_char
                         << " t = " << Am_Current_Input_Event->time_stamp
                         << " user_id=" << Am_Current_Input_Event->user_id
                         << // supports multiple users
               std::endl);
    }
  }
}

void
Am_WinDrawonable::WndOnSetCursor(WPARAM wParam, LPARAM lParam)
{
  PREPARE_DATA(Am_Cursor, cursd, cursor)

  if (!cursd) {
    DefWndProc(WM_SETCURSOR, wParam, lParam);
    return;
  }

  HCURSOR hcurs = cursd->WinCursor(m_inst);
  if (hcurs)
    ::SetCursor(hcurs);
  else
    DefWndProc(WM_SETCURSOR, wParam, lParam);

  DISCARD_DATA(cursd)
}

void
Am_WinDrawonable::WndInvalidateAll()
{
#if !defined(_WIN32)
  Am_MapIterator_Int2Ptr next(&m_mapWnd2Drw);
#else
#ifdef OA_VERSION
  Am_Map_HModule2Ptr_Iterator next(m_mapWnd2Drw);
#else
  Am_MapIterator_Ptr2Ptr next(&m_mapWnd2Drw);
#endif
#endif

  while (next())
#if _MSC_VER >= 1200
    InvalidateRect(reinterpret_cast<HWND__ *>(next.Key()), (0L), TRUE);
#else
    InvalidateRect(next.Key(), (0L), TRUE);
#endif
}

void
Am_WinDrawonable::WndDraw(HDC hdc, const RECT &rc)
{
  if (m_evh)
    m_msgqueue.Add(
        new Am_WinNotifyInfo(Am_WinDrawonable::NotifyExposure, this, rc));
}

void
Am_WinDrawonable::WndAttach(HWND hwnd)
{
  WndDetach();
  m_hwnd = hwnd;
  WndFillInfo(hwnd);
  m_mapWnd2Drw[hwnd] = this;
}

HWND
Am_WinDrawonable::WndDetach()
{
  HWND hwndOld = m_hwnd;
  if (m_hwnd) { //attached
    m_mapWnd2Drw.DeleteKey(m_hwnd);
    m_hwnd = 0;
  }
  return hwndOld;
}

bool
Am_WinDrawonable::WndCreate(bool fQueryPos, bool fQuerySize, bool fSaveBits)
{
  if (IsWindow(m_hwnd))
    return true; //allready created

  //LPCSTR lpszClassName;	/* address of registered class name	*/
  //LPCSTR lpszWindowName;	/* address of window text	*/
  //DWORD dwStyle;	/* window style	*/
  //int x;	/* horizontal position of window	*/
  //int y;	/* vertical position of window	*/
  //int nWidth;	/* window width	*/
  //int nHeight;	/* window height	*/
  //HWND hwndParent;	/* handle of parent window	*/
  //HMENU hmenu;	/* handle of menu or child-window identifier	*/
  //HINSTANCE hinst;	/* handle of application instance	*/
  //void FAR* lpvParam;	/* address of window-creation data	*/

  m_style = WndStyleFromData();
  int bl, bt, br, bb, dummy;
  Inquire_Window_Borders(bl, bt, br, bb, dummy, dummy);
  // queue message for opal about frame size
  if (m_evh) {
    RECT rc;
    rc.left = bl;
    rc.top = bt;
    rc.right = br;
    rc.bottom = bb;
    m_msgqueue.Add(
        new Am_WinNotifyInfo(Am_WinDrawonable::NotifyFrameSize, this, rc));
  }

  // Primary windows are windows with a (0L) parent; when a primary window is
  // visible, its name appears in a button on the taskbar.
  // Secondary (owned) windows have a non-(0L) parent; when a secondary window
  // is visible, its name does not appear on the taskbar
  // 6/11/97: opal does not support the concept of owned windows
  m_hwnd = CreateWindow(ClsGetName(fSaveBits),
                        (iconified && icon_name) ? icon_name : title, m_style,
                        left, top, width + br + bl, height + bt + bb,
                        m_hwndParent, (0L), m_inst, (LPBYTE)this);

  if (m_hwnd) {
    if (visible) {
      ShowWindow(m_hwnd, (iconified) ? SW_SHOWMINIMIZED : SW_SHOWNORMAL);
      UpdateWindow(m_hwnd);
      //InvalidateRect(m_hwnd, (0L), FALSE); //queue WM_PAINT instead of UpdateWindow
      //-- important for Opal.
    }

    m_style = GetWindowLong(m_hwnd, GWL_STYLE);
    m_exstyle = GetWindowLong(m_hwnd, GWL_EXSTYLE);

    for (Am_Position pos = m_children.HeadPos(); pos;
         pos = m_children.NextPos(pos))
      Narrow(m_children[pos])->WndCreate();
    return true;
  } else
    return false;
}

DWORD
Am_WinDrawonable::WndStyleFromData()
{
  // 5-7-96 af1x
  // Border width is directly related to title_bar.
  // There really is no such thing as border width.
  // Top level windows have either a titlebar and border, or nothing.
  // Child windows have either a (2 pixel) border, or nothing.

  bool fTop = (m_parent == m_root);
  DWORD style = 0;
  // There are only 4 cases
  if (fTop) {        // top level window
    if (title_bar) { // title bar
      style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX |
              WS_MAXIMIZEBOX | WS_THICKFRAME;
    } else { // no title bar (top level)
      style = WS_POPUP;
    }
  } else { // child window
    style = WS_CHILD;
    if (title_bar) { // border
      style |= WS_BORDER | WS_OVERLAPPED;
    }
  }

  style |= WS_CLIPCHILDREN;

  if (iconified)
    style |= WS_MINIMIZE;

  return style;
}

void
Am_WinDrawonable::WndClose()
{
  SendMessage(m_hwnd, WM_CLOSE, 0, 0);
}

void
Am_WinDrawonable::WndDestroy()
{
  if (this != m_root) //this isn't root drawonable
    DestroyWindow(WndDetach());
  else
    WndDetach();
}

void
Am_WinDrawonable::WndFillInfo(HWND hwnd)
{
  if (IsWindow(hwnd)) {
    RECT rect;
    GetWindowRect(hwnd, &rect);
    left = rect.left;
    top = rect.top;
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    strdel(title);
    title = (0L);
    unsigned titleLen = GetWindowTextLength(hwnd);
    if (titleLen) {
      char *titleText = new char[titleLen + 1 /*term. 0*/];
      if (titleText) {
        GetWindowText(hwnd, titleText, titleLen);
        title = strnew(titleText);
        delete titleText;
      }
    }
    icon_name = (0L); //the same as title

    visible = IsWindowVisible(hwnd);

    background_color = Am_No_Style;

    m_style = GetWindowLong(hwnd, GWL_STYLE);
    m_exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    if (m_exstyle & WS_EX_DLGMODALFRAME)
      border_width = GetSystemMetrics(SM_CXDLGFRAME);
    else if (m_style & WS_DLGFRAME)
      border_width = GetSystemMetrics(SM_CXDLGFRAME);
    else if (m_style & WS_THICKFRAME)
      border_width = GetSystemMetrics(SM_CXFRAME);
    else
      border_width = 0;
    if (m_style & WS_BORDER)
      border_width += GetSystemMetrics(SM_CXBORDER);

    title_bar = (bool)((m_style & WS_CAPTION) != 0);
    clip_by_children = (bool)((m_style & WS_CLIPCHILDREN) != 0);

    iconified = IsIconic(hwnd);

    min_width = 0;
    min_height = 0;
    max_width = 0;
    max_height = 0;

    depth = WndGetColorDepth();

    /*WINDOWPLACEMENT place;
		place.length = sizeof(place);
		GetWindowPlacement(hwnd, &place);
		min_width = ptMinPosition.x;
		min_height = ptMinPosition.y;
		max_width = ptMaxPosition.x;
		max_height = ptMaxPosition.y;
		iconified = (place.showCmd == SW_SHOWMINIMIZED);
		*/

    //save_under = save_under_flag;
    //ask_position = query_user_for_position;
    //ask_size = query_user_for_size;
  }
}

void
Am_WinDrawonable::WndIconify()
{
  SendMessage(m_hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0L);
}

void
Am_WinDrawonable::WndRestore()
{
  SendMessage(m_hwnd, WM_SYSCOMMAND, SC_RESTORE, 0L);
}

int
Am_WinDrawonable::WndGetColorDepth(HDC hdc)
{
  int res;

  // do we have a device-context handle?
  if (hdc == 0) {
    // The CreateIC function creates an information context for the specified
    // device (in this case Display). The information context provides a fast way to get information
    // about the device without creating a device context.
    hdc = CreateIC("DISPLAY", (0L), NULL, NULL);

    // The GetDeviceCaps function retrieves device-specific information about a specified device.
    res = GetDeviceCaps(hdc, BITSPIXEL) * GetDeviceCaps(hdc, PLANES);

    // Always free the device-context
    DeleteDC(hdc);
  } else {
    // yes, than use it to retrieves device-specific information
    res = GetDeviceCaps(hdc, BITSPIXEL) * GetDeviceCaps(hdc, PLANES);
  }

  // and return the number if colors
  return (res);
}

void
Am_WinDrawonable::SetUsesBufferring(BOOL usebuff)
{
  m_usebuff = usebuff;
}

void
Am_WinDrawonable::SetBuffered(BOOL isbuff)
{
  if (isbuff == m_isbuff)
    return;

  m_isbuff = isbuff;
  if (!m_isbuff) {
    // m_isbuff == FALSE; switch buffering off
    if (m_hbmp)
      DeleteObject(m_hbmp);
    m_hbmp = 0;
  }
}

void
Am_WinDrawonable::CreateBuffer()
{
  if (IsBuffered()) {

    if (!m_hbmp) {
      HDC hidc = ::CreateIC("DISPLAY", (0L), NULL, NULL);
      m_hbmp = CreateCompatibleBitmap(hidc, width, height);
      //	m_hbmp = CreateBitmap(width, height, 1, Get_Depth(), (0L));
      ::DeleteDC(hidc);
    }
    if (m_hbmp)
      Clear_Area(0, 0, width, height);
  }
}

void
Am_WinDrawonable::ResizeBuffer(unsigned int new_width, unsigned int new_height)
{
  if (IsBuffered()) {
    //resize bitmap, but preserve old contents if any
    HBITMAP hbmpold = m_hbmp; // save old bitmap
    // create new bitmap compatible with the screen
    HDC hidc = ::CreateIC("DISPLAY", (0L), NULL, NULL);
    m_hbmp = CreateCompatibleBitmap(hidc, new_width, new_height);
    ::DeleteDC(hidc);
    //m_hbmp = CreateBitmap(new_width, new_height, 1, Get_Depth(), (0L));
    if (!m_hbmp) {
      m_hbmp = hbmpold;
      return;
    }
    Clear_Area(0, 0, new_width, new_height);
    if (hbmpold) {
      Am_WinDC dc(this); // will use buffer
      Am_WinDC dcsrc(dc);

// replaced dcsrc.SelectTool(hbmpold) and dcsrc.DeselectAll()
// with SelectObject followed by DeleteObject
#if _MSC_VER >= 1200
      HBITMAP saveobj =
          reinterpret_cast<HBITMAP__ *>(SelectObject(dcsrc, hbmpold));
#else
      HBITMAP saveobj = SelectObject(dcsrc, hbmpold);
#endif
      ::BitBlt(dc, 0, 0, width, height, dcsrc, 0, 0, SRCCOPY);
      SelectObject(dcsrc, saveobj);
      DeleteObject(hbmpold);
    }
  }
}

// copy buffer on screen if window is visible
void
Am_WinDrawonable::FlushBuffer() const
{
  if (IsBuffered() && m_hwnd && Get_Visible()) {
    Am_WinDC dc(this);       // will use buffer
    Am_WinDC dcdest(m_hwnd); // will use window's DC
    ::BitBlt(dcdest, 0, 0, width, height, dc, 0, 0, SRCCOPY);
  }
}

// copy screen to buffer if window is visible
void
Am_WinDrawonable::FillBuffer() const
{
  if (IsBuffered() && m_hwnd && Get_Visible()) {
    Am_WinDC dc(this);      // will use buffer
    Am_WinDC dcsrc(m_hwnd); // will use window's DC
    ::BitBlt(dc, 0, 0, width, height, dcsrc, 0, 0, SRCCOPY);
  }
}

// Parent / children management
void
Am_WinDrawonable::SetParent(Am_WinDrawonable *parent)
{
  if (m_parent)
    m_parent->m_children.DeleteAt(m_parent->m_children.FindPos(this));
  m_parent = parent;
  if (m_parent) {
    m_parent->m_children.Add(this);
    m_hwndParent = m_parent->WndGet();
    if (m_hwndParent == GetDesktopWindow())
      m_hwndParent = 0;
  } else
    m_hwndParent = 0;
}

void
Am_WinDrawonable::NotifyIconified(const Am_WinNotifyInfo *info)
{
  info->m_drw->m_evh->Iconify_Notify(info->m_drw, info->m_b);
}

void
Am_WinDrawonable::NotifyFrameSize(const Am_WinNotifyInfo *info)
{
  info->m_drw->m_evh->Frame_Resize_Notify(info->m_drw, info->m_rc.left,
                                          info->m_rc.top, info->m_rc.right,
                                          info->m_rc.bottom);
}

void
Am_WinDrawonable::NotifyDestroy(const Am_WinNotifyInfo *info)
{
  info->m_drw->m_evh->Destroy_Notify(info->m_drw);
}

void
Am_WinDrawonable::NotifyConfigure(const Am_WinNotifyInfo *info)
{
  info->m_drw->m_evh->Configure_Notify(info->m_drw, info->m_left, info->m_top,
                                       info->m_width, info->m_height);
}

void
Am_WinDrawonable::NotifyExposure(const Am_WinNotifyInfo *info)
{
  info->m_drw->m_evh->Exposure_Notify(
      info->m_drw, info->m_rc.left, info->m_rc.top,
      info->m_rc.right - info->m_rc.left, info->m_rc.bottom - info->m_rc.top);
}

void
Am_WinDrawonable::NotifyBackground(const Am_WinNotifyInfo *info)
{
  info->m_drw->Clear_Area(info->m_rc.left, info->m_rc.top,
                          info->m_rc.right - info->m_rc.left,
                          info->m_rc.bottom - info->m_rc.top);
}

Am_Drawonable *
Am_WinDrawonable::Get_Drawonable_At_Cursor()
{
  // find the current mouse position
  POINT pt;
  GetCursorPos(&pt);
  // find the window at the current mouse position
  HWND hWnd = WindowFromPoint(pt);
  // convert the Windows handle to a drawonable, return it
  return Narrow(m_mapWnd2Drw.GetAt(hWnd));
}
