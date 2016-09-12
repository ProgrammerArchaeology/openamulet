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

// Opal operations: Am_Move_Object, Am_To_Top, Am_To_Bottom,
//		    Am_Update, Am_Update_All, Am_Wait_For_Event,
//		    Am_Translate_Coordinates, Am_Invalidate

#include <am_inc.h>
#include AM_IO__H
#include <amulet/impl/types_logging.h>
#include <amulet/impl/slots.h>
#include <amulet/impl/types_method.h>
#include <amulet/impl/am_object.h>
#include <amulet/impl/am_object_advanced.h>
#include <amulet/impl/opal_default_demon.h>
#include <amulet/impl/am_state_store.h>
#include <amulet/impl/opal_op.h>
#include <amulet/impl/opal_misc.h>
#include <amulet/impl/opal_methods.h>
#include <amulet/impl/opal_objects.h>
#include <amulet/impl/am_window_todo.h>
#include "amulet/opal_intnl.h"
#include VALUE_LIST__H

void
Am_Move_Object(Am_Object object, Am_Object ref_object, bool above)
{
  if (!object.Valid() || !ref_object.Valid())
    Am_Error("** Am_Move_Object called with (0L) object or ref_object\n");
  if (object == ref_object) {
    std::cerr << "** Am_Move_Object called same object, " << object
              << ", as both "
                 "moved object and reference object."
              << std::endl;
    Am_Error();
  }
  Am_Object owner = object.Get_Owner();
  if (!owner.Valid())
    return;
  if (ref_object.Get_Owner() != owner) {
    std::cerr << "** Am_Move_Object:  the object " << object << " has owner "
              << owner << "and the ref_object " << ref_object << " has owner "
              << ref_object.Get_Owner() << " they must be the same."
              << std::endl;
    Am_Error();
  }
  if (object.Is_Instance_Of(Am_Graphical_Object)) {
    if (!ref_object.Is_Instance_Of(Am_Graphical_Object)) {
      std::cerr << "** Am_Move_Object called with an object " << object
                << " that is a graphical object and a ref_object " << ref_object
                << " that is not." << std::endl;
      Am_Error();
    }
    owner.Make_Unique(Am_GRAPHICAL_PARTS);
    Am_Value_List components;
    components = owner.Get(Am_GRAPHICAL_PARTS);
    components.Start();
    components.Member(object);
    components.Delete(false);
    components.Start();
    components.Member(ref_object);
    components.Insert(object, above ? Am_AFTER : Am_BEFORE, false);
    int rank = 0;
    Am_Object current;
    for (components.Start(); !components.Last(); components.Next()) {
      current = components.Get();
      current.Set(Am_RANK, rank);
      ++rank;
    }
    owner.Note_Changed(Am_GRAPHICAL_PARTS);
    Main_Demon_Queue.Enqueue(
        graphics_repaint, 0,
        ((Am_Object_Advanced &)object).Get_Slot(Am_VISIBLE));
  } else if (object.Is_Instance_Of(Am_Window)) {
    if (!ref_object.Is_Instance_Of(Am_Window)) {
      std::cerr << "** Am_Move_Object called with an object " << object
                << " that is a window and a ref_object " << ref_object
                << " that is not." << std::endl;
      Am_Error();
    };
    // objects must have same parent
    if (object.Get_Owner() != ref_object.Get_Owner()) {
      std::cerr << "** Am_Move_Object called with two windows"
                << " with different parents: " << object << ", " << ref_object;
    }
    Am_Drawonable *draw1 = Am_Drawonable::Narrow(object.Get(Am_DRAWONABLE));
    Am_Drawonable *draw2 = Am_Drawonable::Narrow(ref_object.Get(Am_DRAWONABLE));
    if (draw1 && draw2) {
      if (above) {
        draw1->Raise_Window(draw2);
      } else {
        draw1->Lower_Window(draw2);
      }
    }
  } else {
    std::cerr << "** Am_Move_Object: Attempt to move " << object
              << " which is not a graphical object or window" << std::endl;
    Am_Error();
  }
}

void
Am_To_Top(Am_Object object)
{
  if (!object.Valid())
    Am_Error("** Am_To_Top called with (0L) object\n");
  Am_Object owner = object.Get_Owner();
  if (!owner.Valid())
    return;
  if (object.Is_Instance_Of(Am_Graphical_Object)) {
    owner.Make_Unique(Am_GRAPHICAL_PARTS);
    Am_Value_List components;
    components = owner.Get(Am_GRAPHICAL_PARTS);
    int rank = object.Get(Am_RANK);
    if (rank + 1 != components.Length()) {
      components.Start();
      components.Member(object);
      components.Delete(false);
      components.Next();
      Am_Object current;
      while (!components.Last()) {
        current = components.Get();
        current.Set(Am_RANK, rank);
        ++rank;
        components.Next();
      }
      object.Set(Am_RANK, (int)components.Length());
      components.Add(object, Am_TAIL, false);
      owner.Note_Changed(Am_GRAPHICAL_PARTS);
      Main_Demon_Queue.Enqueue(
          graphics_repaint, 0,
          ((Am_Object_Advanced &)object).Get_Slot(Am_VISIBLE));
    }
  } else if (object.Is_Instance_Of(Am_Window)) {
    Am_Drawonable *draw = Am_Drawonable::Narrow(object.Get(Am_DRAWONABLE));
    if (draw)
      draw->Raise_Window((0L));
  } else {
    std::cerr << "** Am_To_Top: Attempt to raise " << object
              << " which is not "
                 "a graphical object or window"
              << std::endl;
    Am_Error();
  }
}

void
Am_To_Bottom(Am_Object object)
{
  if (!object.Valid())
    Am_Error("** Am_To_Bottom called with (0L) object\n");
  Am_Object owner = object.Get_Owner();
  if (!owner.Valid())
    return;
  if (object.Is_Instance_Of(Am_Graphical_Object)) {
    owner.Make_Unique(Am_GRAPHICAL_PARTS);
    Am_Value_List components;
    components = owner.Get(Am_GRAPHICAL_PARTS);
    int rank = object.Get(Am_RANK);
    if (rank != 0) {
      components.Start();
      components.Member(object);
      components.Delete(false);
      Am_Object current;
      while (!components.First()) {
        current = components.Get();
        current.Set(Am_RANK, rank);
        --rank;
        components.Prev();
      }
      object.Set(Am_RANK, 0);
      components.Add(object, Am_HEAD, false);
      owner.Note_Changed(Am_GRAPHICAL_PARTS);
      Main_Demon_Queue.Enqueue(
          graphics_repaint, 0,
          ((Am_Object_Advanced &)object).Get_Slot(Am_VISIBLE));
    }
  } else if (object.Is_Instance_Of(Am_Window)) {
    Am_Drawonable *draw = Am_Drawonable::Narrow(object.Get(Am_DRAWONABLE));
    if (draw)
      draw->Lower_Window((0L));
  } else {
    std::cerr << "** Am_To_Bottom: Attempt to lower " << object
              << " which is not "
                 "a graphical object or window"
              << std::endl;
    Am_Error();
  }
}

void
Am_Update(Am_Object window)
{
  Am_Value value;
  value = window.Peek(Am_TODO);
  if (value.Valid()) {
    Am_Window_ToDo *win_todo = Am_Window_ToDo::Narrow(value);
    if (win_todo->flags) {
      win_todo->Remove();
      Am_Draw_Method draw_method;
      draw_method = window.Get(Am_DRAW_METHOD);
      //ignores drawonable and offset parameters
      draw_method.Call(window, (0L), 0, 0);
    }
  }
}

//if debugging, then checks to see whether trying to redraw a window
//after it was already started to draw, but didn't finish, which means
//that it crashed last time it was trying to be drawn, in which case
//it doesn't try to redraw it again, but only one time.
void
Am_Update_All()
{
  if (!Main_Demon_Queue.Empty())
    Main_Demon_Queue.Invoke();

  Main_Demon_Queue.Prevent_Invoke();
  Am_State_Store::Invoke();
  Am_Window_ToDo *current;
  Am_Draw_Method draw_method;
#ifdef DEBUG
  int was_inprogress;
#endif
  Am_Object win;
  while (Window_ToDo_Head) {
    current = Window_ToDo_Head;
    Window_ToDo_Head = Window_ToDo_Head->next;
    if (!Window_ToDo_Head)
      Window_ToDo_Tail = (0L);
    current->next = (0L);
    current->prev = (0L);
    win = current->window;
    if (win.Valid()) {
#ifdef DEBUG
      was_inprogress = win.Get(Am_OBJECT_IN_PROGRESS, Am_RETURN_ZERO_ON_ERROR);
      if (was_inprogress & 1) {
        std::cerr << "** Draw on window " << win
                  << " but crashed last time, so skipping it.\n"
                  << std::flush;
        continue; //skip this object
      } else
        win.Set(Am_OBJECT_IN_PROGRESS, 1, Am_OK_IF_NOT_THERE);
#endif
      draw_method = win.Get(Am_DRAW_METHOD);
      draw_method.Call(win, (0L), 0, 0);
#ifdef DEBUG
      win.Set(Am_OBJECT_IN_PROGRESS, 0);
#endif
    }
  }
  Main_Demon_Queue.Release_Invoke();
}

// Converts a point in one object's coordinate system, to that of another
// object.  If the objects are not comparable (like being on different screens
// or not being on a screen at all) then the function will return false.
// Otherwise, it will return true and dest_x and dest_y will contain the
// converted coordinates.  Note that the coordinates are for the
// INSIDE of dest_obj.  This means that if "obj" was at src_x, src_y
// in src_obj and you remove it from src_obj and add it to dest_obj at
// dest_x, dest_y then it will be at the same physical screen position.
bool
Am_Translate_Coordinates(const Am_Object &src_obj, int src_x, int src_y,
                         const Am_Object &dest_obj, int &dest_x, int &dest_y)
{
  Am_Translate_Coordinates_Method translate_method;

  bool using_constraint = true;
  int src_height = 0;
  Am_Object owner = src_obj;
  int off_x;
  int off_y;

  while (owner) {
    ++src_height;
    owner = owner.Get_Owner(Am_NO_DEPENDENCY);
  }

  int dest_height = 0;
  owner = dest_obj;
  while (owner.Valid()) {
    ++dest_height;
    owner = owner.Get_Owner(Am_NO_DEPENDENCY);
  }

  Am_Object src_part;
  Am_Object curr_src = src_obj;

  while (src_height > dest_height) {
    if (curr_src.Is_Instance_Of(Am_Graphical_Object) ||
        curr_src.Is_Instance_Of(Am_Window)) {
      translate_method = curr_src.Get(Am_TRANSLATE_COORDINATES_METHOD);
      translate_method.Call(curr_src, src_part, src_x, src_y, src_x, src_y);
    } else {
      return false;
    }

    src_part = curr_src;
    if (using_constraint) {
      curr_src = curr_src.Get_Owner();
    } else {
      curr_src = curr_src.Get_Owner();
    }
    --src_height;
  }
  Am_Object dest_part;
  Am_Object curr_dest = dest_obj;
  while (dest_height > src_height) {
    if (curr_dest.Is_Instance_Of(Am_Graphical_Object) ||
        curr_dest.Is_Instance_Of(Am_Window)) {
      translate_method = curr_dest.Get(Am_TRANSLATE_COORDINATES_METHOD);
      translate_method.Call(curr_dest, dest_part, 0, 0, off_x, off_y);
      src_x -= off_x;
      src_y -= off_y;
    } else {
      return false;
    }

    dest_part = curr_dest;

    if (using_constraint) {
      curr_dest = curr_dest.Get_Owner();
    } else {
      curr_dest = curr_dest.Get_Owner();
    }

    --dest_height;
  }
  while (curr_src.Valid() && (curr_src != curr_dest)) {
    if (curr_src.Is_Instance_Of(Am_Graphical_Object) ||
        curr_src.Is_Instance_Of(Am_Window)) {
      translate_method = curr_src.Get(Am_TRANSLATE_COORDINATES_METHOD);
      translate_method.Call(curr_src, src_part, src_x, src_y, src_x, src_y);
    } else
      break;

    if (curr_dest.Is_Instance_Of(Am_Graphical_Object) ||
        curr_dest.Is_Instance_Of(Am_Window)) {
      translate_method = curr_dest.Get(Am_TRANSLATE_COORDINATES_METHOD);
      translate_method.Call(curr_dest, dest_part, 0, 0, off_x, off_y);
      src_x -= off_x;
      src_y -= off_y;
    } else
      break;

    src_part = curr_src;
    dest_part = curr_dest;

    if (using_constraint) {
      curr_src = curr_src.Get_Owner();
      curr_dest = curr_dest.Get_Owner();
    } else {
      curr_src = curr_src.Get_Owner();
      curr_dest = curr_dest.Get_Owner();
    }
  }

  if (curr_src.Valid() && curr_src == curr_dest) {
    dest_x = src_x;
    dest_y = src_y;

    return (true);
  } else {
    if (using_constraint) {
      while (curr_src.Valid()) {
        curr_src = curr_src.Get_Owner();
      }

      while (curr_dest.Valid()) {
        curr_dest = curr_dest.Get_Owner();
      }
    }

    return (false);
  }
}

void
Am_Invalidate(Am_Object owner, Am_Object which_part, int left, int top,
              int width, int height)
{
#ifdef DEBUG
  // extra test to see if owner is not a group (or window), then doesn't have an
  // Invalidate method
  Am_Value v;
  v = owner.Peek(Am_INVALID_METHOD, Am_NO_DEPENDENCY);
  if (v.Valid()) {
    Am_Invalid_Method invalidate = v;
    invalidate.Call(owner, which_part, left, top, width, height);
  } else
    Am_ERRORO("Apparently added a part "
                  << which_part << " to a non-group-type object " << owner,
              owner, 0);
#else
  Am_Invalid_Method invalidate = owner.Get(Am_INVALID_METHOD);
  invalidate.Call(owner, which_part, left, top, width, height);
#endif
}
