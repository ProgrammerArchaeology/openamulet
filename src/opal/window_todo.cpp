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

// Am_Window_ToDo class implementation, used by Am_Window

#include <am_inc.h>
#include <amulet/impl/am_window_todo.h>

Am_Window_ToDo *Window_ToDo_Head = nullptr;
Am_Window_ToDo *Window_ToDo_Tail = nullptr;

void
Am_Window_ToDo::Merge_Rectangle(int in_left, int in_top, int in_width,
                                int in_height)
{
  if (width && height) {
    int far_left = (in_left < left) ? in_left : left;
    int far_top = (in_top < top) ? in_top : top;
    int far_width = ((in_left + in_width) > (left + width))
                        ? in_left + in_width - far_left
                        : left + width - far_left;
    int far_height = ((in_top + in_height) > (top + height))
                         ? in_top + in_height - far_top
                         : top + height - far_top;
    left = far_left;
    top = far_top;
    width = far_width;
    height = far_height;
  } else {
    left = in_left;
    top = in_top;
    width = in_width;
    height = in_height;
  }
}

void
Am_Window_ToDo::Add()
{
  if (!prev && !next && (Window_ToDo_Head != this)) {
    prev = Window_ToDo_Tail;
    if (Window_ToDo_Tail)
      Window_ToDo_Tail->next = this;
    else
      Window_ToDo_Head = this;
    Window_ToDo_Tail = this;
  }
}

void
Am_Window_ToDo::Remove()
{
  if (prev || next || (Window_ToDo_Head == this)) {
    if (next)
      next->prev = prev;
    else
      Window_ToDo_Tail = prev;
    if (prev)
      prev->next = next;
    else
      Window_ToDo_Head = next;
    prev = nullptr;
    next = nullptr;
  }
}
