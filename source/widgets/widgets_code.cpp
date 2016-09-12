#include <am_inc.h>

#include <amulet/impl/am_object.h>
#include <amulet/impl/slots_opal.h>
#include <amulet/impl/opal_op.h>
#include <amulet/impl/inter_input.h>
#include <amulet/impl/modal_windows.h>

//main_win required if vis == true; otherwise not needed
// If the main window is a modal window, then the popped up window
// needs to be modal as well so it can be operated.
void
set_popup_win_visible(Am_Object &pop_window, bool vis, Am_Object &main_win)
{
  pop_window.Set(Am_VISIBLE, vis);
  if (vis) {
    Am_To_Top(pop_window);
    Am_Value v =
        main_win.Get(Am_WAITING_FOR_COMPLETION, Am_RETURN_ZERO_ON_ERROR);
    pop_window.Set(Am_WAITING_FOR_COMPLETION, v, Am_OK_IF_NOT_THERE);
    if (v.Valid() && (int)v == (int)Am_INTER_WAITING_MODAL) {
      Am_Push_Modal_Window(pop_window);
    }
  } else {
    Am_Value v = pop_window.Peek(Am_WAITING_FOR_COMPLETION);
    if (v.Valid() && (int)v == (int)Am_INTER_WAITING_MODAL) {
      Am_Remove_Modal_Window(pop_window);
    }
  }
}
