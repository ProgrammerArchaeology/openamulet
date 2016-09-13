#ifndef SLOTS_OPAL_H
#define SLOTS_OPAL_H

#include "slots.h"

enum
{
  // Opal internal slot keys
  Am_DRAW_METHOD = Am_FIRST_OPAL_INTERNAL_SLOT,
  Am_MASK_METHOD,
  Am_DRAWONABLE,
  Am_DRAW_BUFFER,
  Am_MASK_BUFFER,
  Am_SCREEN,
  Am_TODO,
  Am_INVALID_METHOD,
  Am_PREV_STATE,
  Am_POINT_IN_OBJ_METHOD,
  Am_POINT_IN_PART_METHOD,
  Am_POINT_IN_LEAF_METHOD,
  Am_TRANSLATE_COORDINATES_METHOD,
  Am_INIT_WANT_ENTER_LEAVE,  // want enter-leave events
  Am_INIT_WANT_MULTI_WINDOW, // want multi-window events
  Am_OLD_WIDTH,              // for resize_groups
  Am_OLD_HEIGHT,             // for resize_groups
  Am_WAITING_FOR_COMPLETION, //pop_up windows
  Am_COMPLETION_VALUE,
  Am_FADE_DEPTH,        // Num fade groups from this obj up to window
  Am_OBJECT_IN_PROGRESS //for checking whether crashed last time
};
//counting down, don't go below biggest in opal section of standard_slots.h

#endif // SLOTS_OPAL_H
