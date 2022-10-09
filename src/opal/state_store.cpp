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

// Am_State_Store class implementation, used by Am_Window

#include <am_inc.h>
#include <amulet/am_io.h>
#include <amulet/impl/types_logging.h>
#include <amulet/impl/slots_opal.h>
#include <amulet/impl/am_drawonable.h>
#include <amulet/impl/am_object.h>
#include <amulet/impl/am_state_store.h>
#include <amulet/impl/method_invalidate.h>

Am_State_Store *Am_State_Store::invalidation_list = nullptr;
bool Am_State_Store::shutdown = false;

Am_State_Store::Am_State_Store(Am_Object in_self, Am_Object in_owner,
                               bool in_visible, int in_left, int in_top,
                               int in_width, int in_height)
{
  self = in_self;
  owner = in_owner;
  visible = in_visible;
  left = in_left;
  top = in_top;
  width = in_width;
  height = in_height;
  in_list = false;
  needs_update = false;
  //// DEBUG
  // std::cout << "New State " << *self << " <l" << left << ", t" << top << ", w"
  //     << width << ", h" << height << ">" <<std::endl;
}

void
Am_State_Store::Add(bool in_needs_update)
{
  if (!in_list) {
    in_list = true;
    next = invalidation_list;
    invalidation_list = this;
  }
  needs_update = needs_update | in_needs_update;
}

void
Am_State_Store::Remove()
{
  if (in_list) {
    Invalidate();
    Am_State_Store *prev = nullptr;
    Am_State_Store *current = invalidation_list;
    while (current) {
      if (current == this) {
        if (prev)
          prev->next = next;
        else
          invalidation_list = next;
        return;
      }
      prev = current;
      current = current->next;
    }
  }
}

void
Am_State_Store::Invalidate()
{
  if (owner.Valid() && visible)
    Am_Invalidate(owner, self, left, top, width, height);
  if (needs_update && self.Valid() && !shutdown) {
    needs_update = false;
    owner = self.Get_Owner(Am_NO_DEPENDENCY);
    visible = self.Get(Am_VISIBLE, Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR);
    left = self.Get(Am_LEFT, Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR);
    top = self.Get(Am_TOP, Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR);
    width = self.Get(Am_WIDTH, Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR);
    height = self.Get(Am_HEIGHT, Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR);
    if (owner.Valid() && visible)
      Am_Invalidate(owner, self, left, top, width, height);
  }
  in_list = false;
}

bool
Am_State_Store::Visible(Am_Drawonable *drawonable, int x_offset, int y_offset)
{
  if (visible) {
    bool total;
    return drawonable->In_Clip(left + x_offset, top + y_offset, width, height,
                               total);
  } else
    return false;
}

//if debugging, then checks to see whether trying to invalidate a window
//after it was already started to invalidate, but didn't finish, which means
//that it crashed last time, in which case
//it doesn't try to invalidate it again, but only one time.

void
Am_State_Store::Invoke()
{
#ifdef DEBUG
  Am_State_Store *current = invalidation_list;
  Am_Object obj;
  while (current) {
    obj = current->self;
    int was_inprogress =
        obj.Get(Am_OBJECT_IN_PROGRESS, Am_RETURN_ZERO_ON_ERROR);
    if (was_inprogress & 1) {
      std::cerr << "** Invalidate on object " << obj
                << " but crashed last time, so skipping it.\n"
                << std::flush;
      obj.Set(Am_OBJECT_IN_PROGRESS, 0);
    } else {
      obj.Set(Am_OBJECT_IN_PROGRESS, 1, Am_OK_IF_NOT_THERE);
      current->Invalidate();
      obj.Set(Am_OBJECT_IN_PROGRESS, 0);
    }
    current = current->next;
  }
  invalidation_list = nullptr;
}
#else //not debugging
  Am_State_Store *current = invalidation_list;
  while (current) {
    current->Invalidate();
    current = current->next;
  }
  invalidation_list = nullptr;
}
#endif
