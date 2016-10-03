/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <am_inc.h>

#include <amulet/value_list.h>
#include <amulet/priority_list.h> // to get Am_Priority_List type
#include <amulet/formula.h>
#include <amulet/impl/types_string.h>
#include <amulet/impl/types_method.h>
#include <amulet/impl/gem_event_handlers.h>
#include <amulet/impl/am_am_drawonable.h>
#include <amulet/impl/am_demon_set.h>
#include <amulet/impl/object_misc.h>
#include <amulet/impl/am_part_iterator.h>
#include <amulet/impl/opal_impl.h>
#include <amulet/impl/opal_misc.h>
#include <amulet/impl/opal_op.h>
#include <amulet/impl/am_alignment.h>
#include <amulet/impl/am_window_todo.h>
#include <amulet/impl/am_state_store.h>
#include <amulet/impl/opal_default_demon.h>
#include <amulet/impl/opal_methods.h>
#include <amulet/impl/inter_input.h>
#include "amulet/opal_intnl.h"

Am_Object Am_Window;

static Am_Wrapper *
return_self_proc(Am_Object &self)
{
  return self;
}
Am_Formula return_self(return_self_proc, DSTR("return_self"));

Am_Define_Formula(bool, window_is_color)
{
  Am_Drawonable *d = Am_Drawonable::Narrow(self.Get(Am_DRAWONABLE));
  // is there a better way to determine if d is valid?
  if (d)
    return d->Is_Color();
  else
    return true;
}

//default window destroy method destroys the window, and then sees if
//this is the last visible window, and if so, exits the main loop.
//Many applications will want to override this method.
Am_Define_Method(Am_Object_Method, void, Am_Default_Window_Destroy_Method,
                 (Am_Object window))
{
  Am_Object screen = window.Get_Owner();
  window.Destroy();
  if (screen.Is_Instance_Of(Am_Screen)) { //otherwise, destroying sub-window
    Am_Part_Iterator part_iter(screen);
    Am_Object part;
    for (part_iter.Start(); !part_iter.Last(); part_iter.Next()) {
      part = part_iter.Get();
      if (part.Is_Instance_Of(Am_Window) &&
          ((bool)part.Get(Am_VISIBLE) || (bool)part.Get(Am_ICONIFIED))) {
        //found a visible or iconified window
        return;
      }
    }
    //if get to here, there are no visible or iconified windows
    Am_Exit_Main_Event_Loop();
  }
}

Am_Define_Method(Am_Object_Method, void, Am_Window_Hide_Method,
                 (Am_Object window))
{
  window.Set(Am_VISIBLE, false);
}

Am_Define_Method(Am_Object_Method, void, Am_Window_Destroy_And_Exit_Method,
                 (Am_Object window))
{
  window.Destroy();
  Am_Exit_Main_Event_Loop();
}

static void
window_create(Am_Object window)
{
  Am_Object_Advanced ad_window = (Am_Object_Advanced &)window;
  ad_window.Set(Am_DRAWONABLE, (Am_Ptr)nullptr); //##
  Am_Window_ToDo *win_todo = new Am_Window_ToDo;
  win_todo->window = window;
  win_todo->width = 0;
  win_todo->height = 0;
  win_todo->flags = 0;
  win_todo->prev = nullptr;
  win_todo->next = nullptr;
  ad_window.Add(Am_TODO, (Am_Ptr)win_todo);
  ad_window.Get_Slot(Am_TODO).Set_Inherit_Rule(Am_LOCAL);

  window.Set(Am_RANK, 0);
  am_generic_renew_components(window);

  //// NDY: window might be created as a part already (currently doesn't matter)
}

//demon procedure
static void
window_copy(Am_Object window)
{
  window.Set(Am_DRAWONABLE, (0L));
  Am_Window_ToDo *win_todo = new Am_Window_ToDo;
  win_todo->window = window;
  win_todo->width = 0;
  win_todo->height = 0;
  win_todo->flags = 0;
  win_todo->prev = nullptr;
  win_todo->next = nullptr;
  window.Add(Am_TODO, (Am_Ptr)win_todo);
  am_generic_renew_copied_comp(window);

  //// NDY: window might be created as a part already (currently doesn't matter)
}

//demon procedure
static void
window_destroy_demon(Am_Object window)
{
  if (window == Am_Window)
    return;
  Am_Part_Iterator parts = window;
  Am_Object part;
  parts.Start();
  while (!parts.Last()) {
    part = parts.Get();
    parts.Next();
    if (part.Is_Instance_Of(Am_Window) || part.Is_Instance_Of(Am_Group))
      part.Destroy();
  }
  Am_Drawonable *drawonable = Am_Drawonable::Narrow(window.Get(Am_DRAWONABLE));
  if (drawonable) {
    drawonable->Set_Data_Store(nullptr);
    drawonable->Destroy();
    window.Set(Am_DRAWONABLE, (0L));
  }
  Am_Value value;
  value = window.Peek(Am_TODO);
  if (value.Valid()) {
    Am_Window_ToDo *win_todo = Am_Window_ToDo::Narrow(value);
    win_todo->Remove();
    delete win_todo;
    window.Set(Am_TODO, (0L));
  }
  if (window.Peek(Am_INTER_LIST).Exists()) {
    Am_Priority_List *inter_list =
        Am_Priority_List::Narrow(window.Get(Am_INTER_LIST));
    window.Set(Am_INTER_LIST, (0L));
    if (inter_list) {
      inter_list->Make_Empty();
      inter_list->Release();
    }
  }
}

static void
revitalize_subwindow(Am_Object window)
{
  Am_Value value;
  value = window.Peek(Am_TODO);
  if (!value.Valid())
    return;
  Am_Window_ToDo *win_todo = Am_Window_ToDo::Narrow(value);
  Am_Drawonable *drawonable = Am_Drawonable::Narrow(window.Get(Am_DRAWONABLE));
  if (drawonable) {
    win_todo->flags |= Am_WINTODO_REPARENT;
    win_todo->Add();
  } else {
    win_todo->flags |= Am_WINTODO_CREATE;
    win_todo->Add();
  }
  Am_Part_Iterator parts = window;
  Am_Object current;
  for (parts.Start(); !parts.Last(); parts.Next()) {
    current = parts.Get();
    if (current.Is_Instance_Of(Am_Window))
      revitalize_subwindow(current);
  }
}

static void
kill_subwindow(Am_Object window)
{
  Am_Drawonable *drawonable = Am_Drawonable::Narrow(window.Get(Am_DRAWONABLE));
  Am_Window_ToDo *win_todo = Am_Window_ToDo::Narrow(window.Get(Am_TODO));
  if (drawonable) {
    win_todo->flags |= Am_WINTODO_DESTROY;
    win_todo->Add();
  }
  Am_Part_Iterator parts = window;
  Am_Object current;
  for (parts.Start(); !parts.Last(); parts.Next()) {
    current = parts.Get();
    if (current.Is_Instance_Of(Am_Window))
      kill_subwindow(current);
  }
}

//this is a demon procedure
static void
window_change_owner(Am_Object window, Am_Object /*old_owner*/,
                    Am_Object new_owner)
{
  Am_Drawonable *drawonable = Am_Drawonable::Narrow(window.Get(Am_DRAWONABLE));
  Am_Window_ToDo *win_todo = Am_Window_ToDo::Narrow(window.Get(Am_TODO));

  if (new_owner.Valid()) {
    if (new_owner.Is_Instance_Of(Am_Screen) ||
        ((new_owner.Is_Instance_Of(Am_Group) ||
          new_owner.Is_Instance_Of(Am_Window)) &&
         !new_owner.Is_Part_Of(window))) {
      if (drawonable) {
        win_todo->flags |= Am_WINTODO_REPARENT;
        win_todo->Add();
      } else {
        win_todo->flags |= Am_WINTODO_CREATE;
        win_todo->Add();
      }
      Am_Part_Iterator parts = window;
      Am_Object current;
      for (parts.Start(); !parts.Last(); parts.Next()) {
        current = parts.Get();
        if (current.Is_Instance_Of(Am_Window))
          revitalize_subwindow(current);
      }
    } else
      Am_Error("Must add a window to either a Screen, Group, "
               "or another Window.\n");
  } else {
    if (drawonable) {
      win_todo->flags |= Am_WINTODO_DESTROY;
      win_todo->Add();
    }
    Am_Part_Iterator parts = window;
    Am_Object current;
    for (parts.Start(); !parts.Last(); parts.Next()) {
      current = parts.Get();
      if (current.Is_Instance_Of(Am_Window))
        kill_subwindow(current);
    }
  }
}

#if defined(_WIN32)
#define ASSERT_WINDOW1(W)                                                      \
  if (!(W))                                                                    \
    return;
#define ASSERT_WINDOW ASSERT_WINDOW1(window)
#else
#define ASSERT_WINDOW
#define ASSERT_WINDOW1(W)
#endif

class Am_Standard_Opal_Handlers : public Am_Input_Event_Handlers
{
public:
  void Iconify_Notify(Am_Drawonable *draw, bool iconified)
  {
    Am_Wrapper *window_data = (Am_Wrapper *)draw->Get_Data_Store();
    window_data->Note_Reference();
    Am_Object window((Am_Object_Data *)window_data);
    ASSERT_WINDOW
    bool w_iconified = window.Get(Am_ICONIFIED);
    if (w_iconified != iconified)
      window.Set(Am_ICONIFIED, iconified);
  }
  void Frame_Resize_Notify(Am_Drawonable *draw, int left, int top, int right,
                           int bottom)
  {
    Am_Wrapper *window_data = (Am_Wrapper *)draw->Get_Data_Store();
    window_data->Note_Reference();
    Am_Object window((Am_Object_Data *)window_data);
    ASSERT_WINDOW
    int w_left = window.Get(Am_LEFT_BORDER_WIDTH);
    int w_top = window.Get(Am_TOP_BORDER_WIDTH);
    int w_right = window.Get(Am_RIGHT_BORDER_WIDTH);
    int w_bottom = window.Get(Am_BOTTOM_BORDER_WIDTH);
    if (w_left != left)
      window.Set(Am_LEFT_BORDER_WIDTH, left);
    if (w_top != top)
      window.Set(Am_TOP_BORDER_WIDTH, top);
    if (w_right != right)
      window.Set(Am_RIGHT_BORDER_WIDTH, right);
    if (w_bottom != bottom)
      window.Set(Am_BOTTOM_BORDER_WIDTH, bottom);
  }
  void Destroy_Notify(Am_Drawonable *draw)
  {
    Am_Wrapper *window_data = (Am_Wrapper *)draw->Get_Data_Store();
    window_data->Note_Reference();
    Am_Object window((Am_Object_Data *)window_data);
    //call the destroy_method in the window
    if (window.Valid()) {
      Am_Object_Method method;
      method = window.Get(Am_DESTROY_WINDOW_METHOD);
      if (method.Valid())
        method.Call(window);
    }
  }
  void Configure_Notify(Am_Drawonable *draw, int left, int top, int width,
                        int height)
  {
    Am_Wrapper *window_data = (Am_Wrapper *)draw->Get_Data_Store();
    window_data->Note_Reference();
    Am_Object window((Am_Object_Data *)window_data);
    ASSERT_WINDOW
    //// DEBUG
    // std::cout << "configure event window=" << window << " left=" << left << " top="
    //     << top << " width=" << width << " height=" << height <<std::endl;
    int w_left = window.Get(Am_LEFT);
    int w_top = window.Get(Am_TOP);
    int w_width = window.Get(Am_WIDTH);
    int w_height = window.Get(Am_HEIGHT);
    if (w_left != left)
      window.Set(Am_LEFT, left);
    if (w_top != top)
      window.Set(Am_TOP, top);
    if (w_width != width)
      window.Set(Am_WIDTH, width);
    if (w_height != height)
      window.Set(Am_HEIGHT, height);
  }
  void Exposure_Notify(Am_Drawonable *draw, int left, int top, int width,
                       int height)
  {
    Am_Wrapper *window_data = (Am_Wrapper *)draw->Get_Data_Store();
    window_data->Note_Reference();
    Am_Object window((Am_Object_Data *)window_data);
    ASSERT_WINDOW
    //// DEBUG
    // std::cout << "expose window left=" << left <<
    //     " top=" << top << " width=" << width << " height=" << height <<std::endl;
    Am_Window_ToDo *win_todo = Am_Window_ToDo::Narrow(window.Get(Am_TODO));
    win_todo->flags |= Am_WINTODO_EXPOSE;
    win_todo->Merge_Rectangle(left, top, width, height);
    win_todo->Add();
  }
  void Input_Event_Notify(Am_Drawonable *draw, Am_Input_Event *ev)
  // used for keys, mouse buttons, mouse moved, and enter-leave.
  {
    // get the window from the drawonable
    Am_Wrapper *window_data = (Am_Wrapper *)draw->Get_Data_Store();
    window_data->Note_Reference();
    Am_Object event_window((Am_Object_Data *)window_data);
    ASSERT_WINDOW1(event_window)

    if (Interactor_Input_Func) {
      (*Interactor_Input_Func)(event_window, ev);
    }
  }
} Global_Opal_Handlers;

Am_Input_Event_Handlers *Am_Global_Opal_Handlers = &Global_Opal_Handlers;

// check slots of the window object to set the initial event mask of
// the drawonable.  After creation, the drawonable's event mask will be set
// automatically by interactors and so the slots are only relevant at
// object creation time.  They are needed since the drawonable is
// typically created for a window AFTER interactors are attached to
// the window (on X)
static void
set_event_mask(Am_Object window, Am_Drawonable *drawonable)
{
  Am_Value value;
  value = window.Peek(Am_INIT_WANT_ENTER_LEAVE);
  if (value.Valid())
    drawonable->Set_Enter_Leave(true);
  value = window.Peek(Am_INIT_WANT_MULTI_WINDOW);
  if (value.Valid())
    drawonable->Set_Multi_Window(true);
  value = window.Peek(Am_WINDOW_WANT_MOVE_CNT);
  if (value.Valid())
    drawonable->Set_Want_Move(true);
}

Am_String
get_icon_title(Am_Object &window)
{
  const Am_Value& v = window.Get(Am_ICON_TITLE);
  if (v.Valid())
    return v;
  else
    return window.Get(Am_TITLE);
}

static bool
create_drawonable(Am_Object window)
{
  Am_Object owner = window.Get_Owner();
  if (!owner)
    return false;
  if (!owner.Is_Instance_Of(Am_Screen) && !owner.Is_Instance_Of(Am_Window)) {
    owner = owner.Get(Am_WINDOW);
    if (!owner)
      return false;
  }
  Am_Drawonable *parent = Am_Drawonable::Narrow(owner.Get(Am_DRAWONABLE));
  if (!parent) {
    Am_Window_ToDo *owner_todo = Am_Window_ToDo::Narrow(owner.Get(Am_TODO));
    if (owner_todo->flags) {
      Am_Window_ToDo *my_todo = Am_Window_ToDo::Narrow(window.Get(Am_TODO));
      my_todo->Add();
    }
    return false;
  }
  int left = window.Get(Am_LEFT);
  int top = window.Get(Am_TOP);
  int width = window.Get(Am_WIDTH);
  int height = window.Get(Am_HEIGHT);
  Am_String title, icon_title;
  title = window.Get(Am_TITLE);
  icon_title = get_icon_title(window);
  bool visible = window.Get(Am_VISIBLE);
  bool iconified = window.Get(Am_ICONIFIED);
  Am_Style back_fill;
  back_fill = window.Get(Am_FILL_STYLE);
  bool save_under = window.Get(Am_SAVE_UNDER);
  int min_width;
  if ((bool)window.Get(Am_USE_MIN_WIDTH))
    min_width = window.Get(Am_MIN_WIDTH);
  else
    min_width = 1;
  int min_height;
  if ((bool)window.Get(Am_USE_MIN_HEIGHT))
    min_height = window.Get(Am_MIN_HEIGHT);
  else
    min_height = 1;
  int max_width;
  if ((bool)window.Get(Am_USE_MAX_WIDTH))
    max_width = window.Get(Am_MAX_WIDTH);
  else
    max_width = 0;
  int max_height;
  if ((bool)window.Get(Am_USE_MAX_HEIGHT))
    max_height = window.Get(Am_MAX_HEIGHT);
  else
    max_height = 0;
  bool query_pos = window.Get(Am_QUERY_POSITION);
  bool query_size = window.Get(Am_QUERY_SIZE);
  bool omit_title_bar = window.Get(Am_OMIT_TITLE_BAR);
  if (owner.Is_Instance_Of(Am_Window))
    omit_title_bar = true;

  Am_Drawonable *drawonable = parent->Create(
      left, top, width, height, title, icon_title, visible, iconified,
      back_fill, save_under, min_width, min_height, max_width, max_height,
      !omit_title_bar, query_pos, query_size, false, Am_Global_Opal_Handlers);
  drawonable->Set_Data_Store((Am_Wrapper *)window);
  Am_Value v = window.Get(Am_CURSOR);
  if (v.Valid())
    drawonable->Set_Cursor((Am_Cursor)v);

  window.Set(Am_DRAWONABLE, (Am_Am_Drawonable)drawonable);
  set_event_mask(window, drawonable);
  return true;
}

//demon procedure called when "uncommon" slots are changed
static void
window_uncommon_slot(Am_Slot first_invalidated)
{
  Am_Object window = first_invalidated.Get_Owner();
  Am_Drawonable *drawonable = Am_Drawonable::Narrow(window.Get(Am_DRAWONABLE));
  if (drawonable) {
    Am_Window_ToDo *win_todo = Am_Window_ToDo::Narrow(window.Get(Am_TODO));
    if (drawonable->Get_Visible() != (bool)window.Get(Am_VISIBLE)) {
      win_todo->flags |= Am_WINTODO_VISIBLE;
      win_todo->Add();
    }
    if (!(Am_String(window.Get(Am_TITLE)) == drawonable->Get_Title())) {
      win_todo->flags |= Am_WINTODO_TITLE;
      win_todo->Add();
    }
    if (!(Am_String(window.Get(Am_ICON_TITLE)) ==
          drawonable->Get_Icon_Title())) {
      win_todo->flags |= Am_WINTODO_ICON_TITLE;
      win_todo->Add();
    }
    if (drawonable->Get_Iconify() != (bool)window.Get(Am_ICONIFIED)) {
      win_todo->flags |= Am_WINTODO_ICONIFY;
      win_todo->Add();
    }
    bool title_p;
    drawonable->Get_Titlebar(title_p);
    if (title_p == (bool)window.Get(Am_OMIT_TITLE_BAR)) {
      Am_Object owner = window.Get_Owner();
      while (owner) {
        if (owner.Is_Instance_Of(Am_Screen)) {
          win_todo->flags |= Am_WINTODO_TITLE_BAR;
          win_todo->Add();
          break;
        } else if (owner.Is_Instance_Of(Am_Window))
          break;
        owner = owner.Get_Owner();
      }
    }
    if (drawonable->Get_Background_Color() !=
        Am_Style(window.Get(Am_FILL_STYLE))) {
      win_todo->flags |= Am_WINTODO_FILL_STYLE;
      win_todo->Add();
    }
    if (drawonable->Get_Cursor() != (Am_Cursor)window.Get(Am_CURSOR)) {
      win_todo->flags |= Am_WINTODO_CURSOR;
      win_todo->Add();
    }
    int width, height;
    drawonable->Get_Min_Size(width, height);
    if ((bool)window.Get(Am_USE_MIN_WIDTH)) {
      if (width != (int)window.Get(Am_MIN_WIDTH)) {
        win_todo->flags |= Am_WINTODO_MIN_SIZE;
        win_todo->Add();
      }
    } else if (width != 1) {
      win_todo->flags |= Am_WINTODO_MIN_SIZE;
      win_todo->Add();
    }
    if ((bool)window.Get(Am_USE_MIN_HEIGHT)) {
      if (height != (int)window.Get(Am_MIN_HEIGHT)) {
        win_todo->flags |= Am_WINTODO_MIN_SIZE;
        win_todo->Add();
      }
    } else if (height != 1) {
      win_todo->flags |= Am_WINTODO_MIN_SIZE;
      win_todo->Add();
    }
    drawonable->Get_Max_Size(width, height);
    if ((bool)window.Get(Am_USE_MAX_WIDTH)) {
      if (width != (int)window.Get(Am_MAX_WIDTH)) {
        win_todo->flags |= Am_WINTODO_MAX_SIZE;
        win_todo->Add();
      }
    } else if (width != 0) {
      win_todo->flags |= Am_WINTODO_MAX_SIZE;
      win_todo->Add();
    }
    if ((bool)window.Get(Am_USE_MAX_HEIGHT)) {
      if (height != (int)window.Get(Am_MAX_HEIGHT)) {
        win_todo->flags |= Am_WINTODO_MAX_SIZE;
        win_todo->Add();
      }
    } else if (height != 0) {
      win_todo->flags |= Am_WINTODO_MAX_SIZE;
      win_todo->Add();
    }
    //// NDY: These need to be defined in gem.
    // Am_CLIP_CHILDREN  Am_WINTODO_CLIP
    // Am_SAVE_UNDER     Am_WINTODO_SAVE_UNDER
  }
}

//demon procedure called when "common" slots are changed
static void
window_common_slot(Am_Slot first_invalidated)
{
  Am_Object window = first_invalidated.Get_Owner();
  Am_Drawonable *drawonable = Am_Drawonable::Narrow(window.Get(Am_DRAWONABLE));
  if (drawonable) {
    Am_Window_ToDo *win_todo = Am_Window_ToDo::Narrow(window.Get(Am_TODO));
    int w_left = window.Get(Am_LEFT, Am_RETURN_ZERO_ON_ERROR);
    int w_top = window.Get(Am_TOP, Am_RETURN_ZERO_ON_ERROR);
    int w_width = window.Get(Am_WIDTH, Am_RETURN_ZERO_ON_ERROR);
    int w_height = window.Get(Am_HEIGHT, Am_RETURN_ZERO_ON_ERROR);
    int d_left, d_top, d_width, d_height;
    drawonable->Get_Position(d_left, d_top);
    drawonable->Get_Size(d_width, d_height);
    if ((w_left != d_left) || (w_top != d_top)) {
      win_todo->flags |= Am_WINTODO_POSITION;
      win_todo->Add();
    }
    if ((w_width != d_width) || (w_height != d_height)) {
      win_todo->flags |= Am_WINTODO_SIZE;
      win_todo->Add();
    }
    //// NDY: May want to make other slots be common
  }
}

static void
window_expose(Am_Object window)
{
  Am_Window_ToDo *win_todo = Am_Window_ToDo::Narrow(window.Get(Am_TODO));
  if (win_todo->width && win_todo->height) {
    Am_Drawonable *drawonable =
        Am_Drawonable::Narrow(window.Get(Am_DRAWONABLE));
    if (drawonable) {
      int double_buffer = window.Get(Am_DOUBLE_BUFFER);
      if (double_buffer == 1) {
        Am_Object screen;

        for (screen = window.Get_Owner();
             (!(screen.Is_Instance_Of(Am_Screen)) && screen.Valid());
             screen = screen.Get_Owner())
          ;
        if (!screen.Valid())
          Am_Error("** updating a window not added to a screen?\n");
        Am_Drawonable *o_drawonable =
            Am_Drawonable::Narrow(screen.Get(Am_OFFSCREEN_DRAWONABLE));
        Am_Style fill;
        fill = window.Get(Am_FILL_STYLE);
        if (!o_drawonable) {
          Am_Drawonable *d = Am_Drawonable::Narrow(screen.Get(Am_DRAWONABLE));
          o_drawonable =
              d->Create_Offscreen(win_todo->width, win_todo->height, fill);
          // o_drawonable = d->Create (0, 0, win_todo->width,
          //				     win_todo->height);
          screen.Set(Am_OFFSCREEN_DRAWONABLE, (Am_Ptr)o_drawonable);
        }
        int w, h, w2, h2;
        o_drawonable->Get_Size(w, h);
        drawonable->Get_Size(w2, h2);
        //if (w < win_todo->width || h < win_todo->height) {
        if (w < w2 || h < h2) {
          // prevent thrashing between a (10x100) and (100x10) drawonable
          w = w > w2 ? w : w2;
          h = h > h2 ? h : h2;
          //        w = w > win_todo->width ? w : win_todo->width;
          //        h = h > win_todo->height ? h : win_todo->height;
          o_drawonable->Set_Size(w, h);
        }
        //o_drawonable->Flush_Output ();

        o_drawonable->Set_Background_Color(fill);
        //o_drawonable->Flush_Output ();
        o_drawonable->Set_Clip(win_todo->left, win_todo->top, win_todo->width,
                               win_todo->height);
        o_drawonable->Clear_Area(win_todo->left, win_todo->top, win_todo->width,
                                 win_todo->height);
        Am_Value_List components;
        components = window.Get(Am_GRAPHICAL_PARTS);
        Am_Object item;
        //o_drawonable->Flush_Output ();

        for (components.Start(); !components.Last(); components.Next()) {
          item = components.Get();
          Am_State_Store *state =
              Am_State_Store::Narrow(item.Get(Am_PREV_STATE));
          if (state->Visible(o_drawonable, 0, 0))
            Am_Draw(item, o_drawonable, 0, 0);
        }
        // objects are drawn.  now blit.
        drawonable->Set_Clip(win_todo->left, win_todo->top, win_todo->width,
                             win_todo->height);
        drawonable->Bitblt(win_todo->left, win_todo->top, win_todo->width,
                           win_todo->height, o_drawonable, win_todo->left,
                           win_todo->top);
      } // if (double_buffer)
      else {
        if (double_buffer == Am_WIN_DOUBLE_BUFFER_EXTERNAL) {
          drawonable =
              Am_Drawonable::Narrow(window.Get(Am_OFFSCREEN_DRAWONABLE));
        }

        //// DEBUG
        // std::cout << "draw window " << window << " <l"
        //     << win_todo->left << " t" << win_todo->top << " w" << win_todo->width
        //     << " h" << win_todo->height << ">" <<std::endl <<std::flush;
        drawonable->Set_Clip(win_todo->left, win_todo->top, win_todo->width,
                             win_todo->height);
        //// DEBUG
        // std::cout << "window clip <l" << win_todo->left << " t" << win_todo->top
        //     << " w" << win_todo->width << " h" << win_todo->height << ">" <<std::endl;
        if (double_buffer != Am_WIN_DOUBLE_BUFFER_EXTERNAL) {
          drawonable->Clear_Area(win_todo->left, win_todo->top, win_todo->width,
                                 win_todo->height);
        }
        Am_Value_List components;
        components = window.Get(Am_GRAPHICAL_PARTS);
        Am_Object item;
        for (components.Start(); !components.Last(); components.Next()) {
          item = components.Get();
          Am_State_Store *state =
              Am_State_Store::Narrow(item.Get(Am_PREV_STATE));
          if (state->Visible(drawonable, 0, 0))
            Am_Draw(item, drawonable, 0, 0);
        }
      } // else (double_buffer)
      drawonable->Flush_Output();
    }
    win_todo->width = 0;
    win_todo->height = 0;
  }
}

static bool
destroy_drawonable(Am_Object window)
{
  Am_Drawonable *drawonable = Am_Drawonable::Narrow(window.Get(Am_DRAWONABLE));
  if (drawonable) {
    Am_Part_Iterator parts = window;
    Am_Object current;
    Am_Drawonable *sub_drawonable;
    Am_Window_ToDo *my_todo;
    for (parts.Start(); !parts.Last(); parts.Next()) {
      current = parts.Get();
      if (current.Is_Instance_Of(Am_Window)) {
        sub_drawonable = Am_Drawonable::Narrow(current.Get(Am_DRAWONABLE));
        if (sub_drawonable) {
          my_todo = Am_Window_ToDo::Narrow(window.Get(Am_TODO));
          my_todo->Add();
          return false;
        }
      }
    }
    drawonable->Set_Data_Store(nullptr);
    drawonable->Destroy();
    window.Set(Am_DRAWONABLE, (0L));
  }
  return true;
}

Am_Define_Method(Am_Draw_Method, void, am_window_draw,
                 (Am_Object window, Am_Drawonable * /*drawonable */,
                  int /* x_offset */, int /* y_offset */))
{
  Am_Window_ToDo *win_todo = Am_Window_ToDo::Narrow(window.Get(Am_TODO));
  unsigned long flags = win_todo->flags;

  //// NDY: Need to consider Am_WINTODO_REPARENT (currently impossible)
  if (flags & Am_WINTODO_CREATE) {
    if (!create_drawonable(window))
      return;
  } else if (flags & Am_WINTODO_DESTROY) {
    if (destroy_drawonable(window))
      win_todo->flags = 0;
    return;
  } else if (flags & Am_WINTODO_REPARENT)
    Am_Error("I didn't think reparenting could happen\n");
  else {
    Am_Drawonable *drawonable =
        Am_Drawonable::Narrow(window.Get(Am_DRAWONABLE));
    bool flush = false;
    if (!drawonable)
      return;
    if (flags & Am_WINTODO_POSITION) {
      int left = window.Get(Am_LEFT);
      int top = window.Get(Am_TOP);
      drawonable->Set_Position(left, top);
      //// DEBUG
      // std::cout << "Window " << window << " moved to ("
      //     << left << ", " << top << ")" <<std::endl;
      flush = true;
    }
    if (flags & Am_WINTODO_SIZE) {
      int width = window.Get(Am_WIDTH);
      int height = window.Get(Am_HEIGHT);
      drawonable->Set_Size(width, height);
      flush = true;
    }
    if (flags & Am_WINTODO_VISIBLE) {
      drawonable->Set_Visible(window.Get(Am_VISIBLE));
      flush = true;
    }
    if (flags & Am_WINTODO_TITLE) {
      drawonable->Set_Title(Am_String(window.Get(Am_TITLE)));
      flush = true;
    }
    if (flags & Am_WINTODO_ICON_TITLE) {
      Am_String title = get_icon_title(window);
      drawonable->Set_Icon_Title(title);
      flush = true;
    }
    if (flags & Am_WINTODO_TITLE_BAR) {
      // Subwindows are not allowed to have titlebars!
      bool omit_titlebar = window.Get(Am_OMIT_TITLE_BAR);
      Am_Object owner = window.Get_Owner();
      if (owner.Is_Instance_Of(Am_Screen)) {
        if (omit_titlebar)
          drawonable->Set_Titlebar(false);
        else
          drawonable->Set_Titlebar(true);
        flush = true;
      }
    }
    if (flags & Am_WINTODO_ICONIFY) {
      drawonable->Set_Iconify(window.Get(Am_ICONIFIED));
      flush = true;
    }
    if (flags & Am_WINTODO_FILL_STYLE) {
      drawonable->Set_Background_Color(Am_Style(window.Get(Am_FILL_STYLE)));
      flush = true;
    }
    if (flags & Am_WINTODO_CURSOR) {
      drawonable->Set_Cursor((Am_Cursor)window.Get(Am_CURSOR));
      flush = true;
    }
    if (flags & Am_WINTODO_MIN_SIZE) {
      int width, height;
      if ((bool)window.Get(Am_USE_MIN_WIDTH))
        width = window.Get(Am_MIN_WIDTH);
      else
        width = 1;
      if ((bool)window.Get(Am_USE_MIN_HEIGHT))
        height = window.Get(Am_MIN_HEIGHT);
      else
        height = 1;
      drawonable->Set_Min_Size(width, height);
    }
    if (flags & Am_WINTODO_MAX_SIZE) {
      int width, height;
      if ((bool)window.Get(Am_USE_MAX_WIDTH))
        width = window.Get(Am_MAX_WIDTH);
      else
        width = 0;
      if ((bool)window.Get(Am_USE_MAX_HEIGHT))
        height = window.Get(Am_MAX_HEIGHT);
      else
        height = 0;
      drawonable->Set_Max_Size(width, height);
    }
    //// NDY: implement other TODO features.  Need to be defined in gem.
    //Am_WINTODO_SAVE_UNDER  Am_SAVE_UNDER
    //Am_WINTODO_CLIP        Am_CLIP_CHILDREN
    if (flush)
      drawonable->Flush_Output();
  }
  if (flags & Am_WINTODO_EXPOSE) {
    if ((bool)window.Get(Am_VISIBLE))
      window_expose(window);
  }
  win_todo->flags = 0;
}

Am_Define_Method(Am_Invalid_Method, void, window_invalid,
                 (Am_Object window, Am_Object /*which_part*/, int left, int top,
                  int width, int height))
{
  if ((bool)window.Get(Am_VISIBLE, Am_NO_DEPENDENCY) &&
      (Am_Ptr)window.Get(Am_DRAWONABLE, Am_NO_DEPENDENCY)) {
    //// DEBUG
    // std::cout << "invalidate window, " << *window << ", l" << left << " t"
    //     << top << " w" << width << " h" << height <<std::endl;
    if (left < 0) {
      width += left;
      left = 0;
    }
    if (top < 0) {
      height += top;
      top = 0;
    }
    if ((width > 0) && (height > 0)) {
      Am_Window_ToDo *win_todo =
          Am_Window_ToDo::Narrow(window.Get(Am_TODO, Am_NO_DEPENDENCY));
      win_todo->flags |= Am_WINTODO_EXPOSE;
      win_todo->Merge_Rectangle(left, top, width, height);
      win_todo->Add();
    }
  }
}

// windows need a special method because of the border of the window
Am_Define_Method(Am_Translate_Coordinates_Method, void,
                 window_translate_coordinates,
                 (const Am_Object &obj, const Am_Object & /*for_part*/,
                  int in_x, int in_y, int &out_x, int &out_y))
{
  Am_Object owner = obj.Get_Owner(Am_NO_DEPENDENCY);
  if (owner.Is_Instance_Of(Am_Window)) { // sub window
    out_x = in_x + (int)obj.Get(Am_LEFT);
    out_y = in_y + (int)obj.Get(Am_TOP);
  } else {
    // compute out_x/y from slots, not from drawonable
    out_x = in_x + (int)obj.Get(Am_LEFT) + (int)obj.Get(Am_LEFT_BORDER_WIDTH);
    out_y = in_y + (int)obj.Get(Am_TOP) + (int)obj.Get(Am_TOP_BORDER_WIDTH);

    //get this always in case this is called from a formula so the
    //translate-coords will be called when the border changes which
    //might happen due to a frame_resize message from gem.
    obj.Get(Am_OMIT_TITLE_BAR);

    /* eab 6/16/97 removed reliance on drawonable
    //// HACK: This code forces the drawonable to be in the most up to date
    //// state.  It will only work for top level windows.  (And may have
    //// problems with top-level windows, too.)  The best implementation for
    //// this is to be able to calculate the position without needing a
    //// drawonable at all.
    Am_Drawonable* drawonable =
      Am_Drawonable::Narrow (obj.Get (Am_DRAWONABLE, Am_NO_DEPENDENCY));
    if (drawonable) {
      int left_border, top_border, right_border, bottom_border;
      int outer_left, outer_top;
      drawonable->Inquire_Window_Borders(left_border, top_border,
                                         right_border, bottom_border,
                                         outer_left, outer_top);
      out_x = in_x + left_border + outer_left;
      out_y = in_y + top_border + outer_top;
    } else {
      obj.Get (Am_DRAWONABLE); // Set a dependency in case drawonable appears later
      out_x = 0;
      out_y = 0;
    }
    */
  }
}

//////////////////////////////////////////////////////////////////////////
// Initialize section
//////////////////////////////////////////////////////////////////////////

void init_graphical_object();
void init_screen();

// this function is called by Am_Initialize
void
Am_Initialize_Aux()
{
  Am_Object_Advanced temp;
  Am_Demon_Set demons;
  unsigned short demon_mask;

  init_screen();

  Am_Window = Am_Root_Object.Create(DSTR("Am_Window"))
                  .Add(Am_WINDOW, return_self)
#ifndef _MACINTOSH
                  .Add(Am_LEFT, 0)
                  .Add(Am_TOP, 0)
#else
                  .Add(Am_LEFT, 20)
                  .Add(Am_TOP, 50)
#endif
                  .Add(Am_WIDTH, 100)
                  .Add(Am_HEIGHT, 100)
                  .Add(Am_GRAPHICAL_PARTS, Am_Value_List())
                  .Add(Am_MAX_WIDTH, 0)
                  .Add(Am_MAX_HEIGHT, 0)
                  .Add(Am_MIN_WIDTH, 1)
                  .Add(Am_MIN_HEIGHT, 1)
                  .Add(Am_TITLE, "Amulet")
                  .Add(Am_ICON_TITLE, Am_No_Value) //use title if no icon title
                  .Add(Am_ICONIFIED, false)
                  .Add(Am_VISIBLE, true)
                  .Add(Am_USE_MIN_WIDTH, false)
                  .Add(Am_USE_MIN_HEIGHT, false)
                  .Add(Am_USE_MAX_WIDTH, false)
                  .Add(Am_USE_MAX_HEIGHT, false);

  Am_Window //need to split because of MSC compiler limit
      .Add(Am_DRAWONABLE, (Am_Ptr)nullptr) //##
      .Add(Am_IS_COLOR, window_is_color)
      .Add(Am_RANK, 0)
      .Add(Am_QUERY_POSITION, false)
      .Add(Am_QUERY_SIZE, false)
      .Add(Am_LEFT_BORDER_WIDTH, 0)
      .Add(Am_TOP_BORDER_WIDTH, 0)
      .Add(Am_RIGHT_BORDER_WIDTH, 0)
      .Add(Am_BOTTOM_BORDER_WIDTH, 0)
      .Add(Am_FILL_STYLE, Am_White)
      .Add(Am_CURSOR, Am_Default_Cursor) // use arrow
      .Add(Am_OMIT_TITLE_BAR, false)
      .Add(Am_CLIP_CHILDREN, false)
      .Add(Am_SAVE_UNDER, false)
      .Add(Am_OWNER_DEPTH, compute_depth)
      .Add(Am_DRAW_METHOD, am_window_draw)
      .Add(Am_INVALID_METHOD, window_invalid)
      .Add(Am_POINT_IN_OBJ_METHOD, generic_point_in_obj)
      .Add(Am_POINT_IN_PART_METHOD, am_group_point_in_part)
      .Add(Am_POINT_IN_LEAF_METHOD, am_group_point_in_leaf)
      .Add(Am_TRANSLATE_COORDINATES_METHOD, window_translate_coordinates)
      .Add(Am_INTER_LIST, (Am_Ptr)nullptr)
      .Add(Am_WINDOW_WANT_MOVE_CNT, 0)
      .Add(Am_DOUBLE_BUFFER, true)
      //.Set (Am_DOUBLE_BUFFER, false)
      .Add(Am_DESTROY_WINDOW_METHOD, Am_Default_Window_Destroy_Method)
      .Add(Am_WAITING_FOR_COMPLETION, (int)Am_INTER_NOT_WAITING)
      .Add(Am_COMPLETION_VALUE, (0L))
      .Add(Am_AS_LINE, false) //windows are not like lines
      // These are for grouping and alignment methods
      .Add(Am_LEFT_OFFSET, 0)
      .Add(Am_TOP_OFFSET, 0)
      .Add(Am_H_SPACING, 0)
      .Add(Am_V_SPACING, 0)
      .Add(Am_RIGHT_OFFSET, 0)
      .Add(Am_BOTTOM_OFFSET, 0)
      .Add(Am_H_ALIGN, Am_CENTER_ALIGN)
      .Add(Am_V_ALIGN, Am_CENTER_ALIGN)
      .Add(Am_FIXED_WIDTH, Am_NOT_FIXED_SIZE)
      .Add(Am_FIXED_HEIGHT, Am_NOT_FIXED_SIZE)
      .Add(Am_INDENT, 0)
      .Add(Am_MAX_RANK, false)
      .Add(Am_MAX_SIZE, false)
      .Add(Am_UNDO_HANDLER, (0L));
  temp = (Am_Object_Advanced &)Am_Window;
  // slots of all Windows that must be local only
  temp.Get_Slot(Am_INTER_LIST).Set_Inherit_Rule(Am_LOCAL);
  temp.Get_Slot(Am_WINDOW_WANT_MOVE_CNT).Set_Inherit_Rule(Am_LOCAL);
  //next two are for windows used as pop-ups
  temp.Get_Slot(Am_WAITING_FOR_COMPLETION).Set_Inherit_Rule(Am_LOCAL);
  temp.Get_Slot(Am_COMPLETION_VALUE).Set_Inherit_Rule(Am_LOCAL);

#ifdef DEBUG
  Am_Window.Add(Am_OBJECT_IN_PROGRESS, 0)
      .Set_Inherit_Rule(Am_OBJECT_IN_PROGRESS, Am_LOCAL);
#endif

  demons = temp.Get_Demons().Copy();
  demons.Set_Object_Demon(Am_CREATE_OBJ, window_create);
  demons.Set_Object_Demon(Am_COPY_OBJ, window_copy);
  demons.Set_Object_Demon(Am_DESTROY_OBJ, window_destroy_demon);
  demons.Set_Part_Demon(Am_ADD_PART, am_generic_add_part);
  demons.Set_Part_Demon(Am_CHANGE_OWNER, window_change_owner);
  demons.Set_Slot_Demon(Am_COMMON_SLOT, window_common_slot,
                        Am_DEMON_PER_OBJECT | Am_DEMON_ON_CHANGE);
  demons.Set_Slot_Demon(Am_UNCOMMON_SLOT, window_uncommon_slot,
                        Am_DEMON_PER_OBJECT | Am_DEMON_ON_CHANGE);
  demon_mask = temp.Get_Demon_Mask();
  demon_mask |= 0x0004 | 0x0008;
  temp.Set_Demon_Mask(demon_mask);
  temp.Set_Demons(demons);

  temp.Get_Slot(Am_LEFT).Set_Demon_Bits(Am_COMMON_SLOT | Am_EAGER_DEMON);
  temp.Get_Slot(Am_TOP).Set_Demon_Bits(Am_COMMON_SLOT | Am_EAGER_DEMON);
  temp.Get_Slot(Am_WIDTH).Set_Demon_Bits(Am_COMMON_SLOT | Am_EAGER_DEMON);
  temp.Get_Slot(Am_HEIGHT).Set_Demon_Bits(Am_COMMON_SLOT | Am_EAGER_DEMON);

  temp.Get_Slot(Am_MAX_WIDTH).Set_Demon_Bits(Am_UNCOMMON_SLOT | Am_EAGER_DEMON);
  temp.Get_Slot(Am_MAX_HEIGHT)
      .Set_Demon_Bits(Am_UNCOMMON_SLOT | Am_EAGER_DEMON);
  temp.Get_Slot(Am_MIN_WIDTH).Set_Demon_Bits(Am_UNCOMMON_SLOT | Am_EAGER_DEMON);
  temp.Get_Slot(Am_MIN_HEIGHT)
      .Set_Demon_Bits(Am_UNCOMMON_SLOT | Am_EAGER_DEMON);
  temp.Get_Slot(Am_TITLE).Set_Demon_Bits(Am_UNCOMMON_SLOT | Am_EAGER_DEMON);
  temp.Get_Slot(Am_ICON_TITLE)
      .Set_Demon_Bits(Am_UNCOMMON_SLOT | Am_EAGER_DEMON);
  temp.Get_Slot(Am_ICONIFIED).Set_Demon_Bits(Am_UNCOMMON_SLOT | Am_EAGER_DEMON);
  temp.Get_Slot(Am_VISIBLE).Set_Demon_Bits(Am_UNCOMMON_SLOT | Am_EAGER_DEMON);
  temp.Get_Slot(Am_USE_MIN_WIDTH)
      .Set_Demon_Bits(Am_UNCOMMON_SLOT | Am_EAGER_DEMON);
  temp.Get_Slot(Am_USE_MIN_HEIGHT)
      .Set_Demon_Bits(Am_UNCOMMON_SLOT | Am_EAGER_DEMON);
  temp.Get_Slot(Am_USE_MAX_WIDTH)
      .Set_Demon_Bits(Am_UNCOMMON_SLOT | Am_EAGER_DEMON);
  temp.Get_Slot(Am_USE_MAX_HEIGHT)
      .Set_Demon_Bits(Am_UNCOMMON_SLOT | Am_EAGER_DEMON);
  /* temporary since need new form of Set to override the read-only
  temp.Get_Slot (Am_LEFT_BORDER_WIDTH).Set_Read_Only (true);
  temp.Get_Slot (Am_TOP_BORDER_WIDTH).Set_Read_Only (true);
  temp.Get_Slot (Am_RIGHT_BORDER_WIDTH).Set_Read_Only (true);
  temp.Get_Slot (Am_BOTTOM_BORDER_WIDTH).Set_Read_Only (true);
*/
  temp.Get_Slot(Am_FILL_STYLE)
      .Set_Demon_Bits(Am_UNCOMMON_SLOT | Am_EAGER_DEMON);
  temp.Get_Slot(Am_CURSOR).Set_Demon_Bits(Am_UNCOMMON_SLOT | Am_EAGER_DEMON);
  temp.Get_Slot(Am_OMIT_TITLE_BAR)
      .Set_Demon_Bits(Am_UNCOMMON_SLOT | Am_EAGER_DEMON);
  temp.Get_Slot(Am_CLIP_CHILDREN)
      .Set_Demon_Bits(Am_UNCOMMON_SLOT | Am_EAGER_DEMON);
  temp.Get_Slot(Am_SAVE_UNDER)
      .Set_Demon_Bits(Am_UNCOMMON_SLOT | Am_EAGER_DEMON);

  init_graphical_object();
}
