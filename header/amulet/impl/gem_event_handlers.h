#ifndef GEM_EVENT_HANDLERS_H
#define GEM_EVENT_HANDLERS_H

#include "am_drawonable.h"
#include <amulet/idefs.h>

class _OA_DL_CLASSIMPORT Am_Input_Event
{
public:
  // MANIPULATORS
  void Set(Am_Input_Char ic, int new_x, int new_y, unsigned long time,
           Am_Drawonable *draw);

  // DATAMEMBERS
  //the char and modifier bits; see idefs.h
  Am_Input_Char input_char;

  // Drawonable this event happened in
  Am_Drawonable *draw;

  int x;
  int y;

  unsigned long time_stamp;

  // supports multiple users
  Am_Value user_id;
};

_OA_DL_IMPORT extern Am_Input_Event *Am_Current_Input_Event;

//:Programmers sub-class this class to define handlers for the various
// input messages.  An object of this class is then used with
// set_input_handler.
class _OA_DL_CLASSIMPORT Am_Input_Event_Handlers
{
public:
  virtual void Iconify_Notify(Am_Drawonable *draw, bool iconified) = 0;
  virtual void Frame_Resize_Notify(Am_Drawonable *draw, int left, int top,
                                   int right, int bottom) = 0;
  virtual void Destroy_Notify(Am_Drawonable *draw) = 0;
  virtual void Configure_Notify(Am_Drawonable *draw, int left, int top,
                                int width, int height) = 0;
  virtual void Exposure_Notify(Am_Drawonable *draw, int left, int top,
                               int width, int height) = 0;

  // next one used for keys, mouse buttons, mouse moved, and enter-leave.
  virtual void Input_Event_Notify(Am_Drawonable *draw, Am_Input_Event *ev) = 0;
};

#endif // GEM_EVENT_HANDLERS_H
