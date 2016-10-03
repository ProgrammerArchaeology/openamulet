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

#include <am_inc.h>

#include <amulet/value_list.h>
#include <amulet/impl/am_am_drawonable.h>
#include <amulet/impl/am_demon_set.h>
#include <amulet/impl/object_misc.h>
#include <amulet/impl/am_part_iterator.h>
#include <amulet/impl/opal_methods.h>
#include <amulet/impl/opal_default_demon.h>
#include <amulet/impl/opal_impl.h>
#include <amulet/impl/opal_misc.h>
#include <amulet/impl/opal_op.h>
#include "amulet/opal_intnl.h"

Am_Object Am_Screen;

static bool
screen_is_color(Am_Object screen)
{
  Am_Drawonable *d = Am_Drawonable::Narrow(screen.Get(Am_DRAWONABLE));
  return (d->Is_Color());
}

Am_Object
Am_Create_Screen(const char *display_name)
{
  Am_Drawonable *new_root = Am_Drawonable::Get_Root_Drawonable(display_name);
  if (!new_root)
    return nullptr;
  int width, height;
  new_root->Get_Size(width, height);
  Am_Object new_screen =
      Am_Screen.Create(display_name)
          .Set(Am_SCREEN, Am_Screen, Am_OK_IF_NOT_THERE)
          .Set(Am_DRAWONABLE, (Am_Am_Drawonable)new_root, Am_OK_IF_NOT_THERE)
          .Set(Am_WIDTH, width)
          .Set(Am_HEIGHT, height);
  Am_Object_Advanced temp = (Am_Object_Advanced &)new_screen;
  //// NDY Create other slots that Am_Screen ought to have
  new_screen.Set(Am_IS_COLOR, screen_is_color(new_screen));
  return new_screen;
}

static void
screen_destroy_demon(Am_Object screen)
{
  Am_Drawonable *drawonable = Am_Drawonable::Narrow(screen.Get(Am_DRAWONABLE));
  if (drawonable) {
    drawonable->Set_Data_Store(nullptr);
    drawonable->Destroy();
    screen.Set(Am_DRAWONABLE, (0L));
  }
  drawonable = Am_Drawonable::Narrow(screen.Get(Am_OFFSCREEN_DRAWONABLE));
  if (drawonable) {
    drawonable->Set_Data_Store(nullptr);
    drawonable->Destroy();
    screen.Set(Am_OFFSCREEN_DRAWONABLE, (0L));
  }
  Am_Value_List draw_list;
  Am_Value fade_value;
  fade_value = screen.Peek(Am_DRAW_BUFFER);
  if (fade_value.Valid()) {
    if (Am_Value_List::Test(fade_value)) {
      draw_list = fade_value;
      for (draw_list.Start(); !draw_list.Last(); draw_list.Next()) {
        drawonable = Am_Drawonable::Narrow(draw_list.Get());
        if (drawonable) {
          drawonable->Set_Data_Store(nullptr);
          drawonable->Destroy();
        }
      }
    } else {
      drawonable = Am_Drawonable::Narrow(fade_value);
      if (drawonable) {
        drawonable->Set_Data_Store(nullptr);
        drawonable->Destroy();
      }
    }
    screen.Set(Am_DRAW_BUFFER, (0L));
  }
  fade_value = screen.Peek(Am_MASK_BUFFER);
  if (fade_value.Valid()) {
    if (Am_Value_List::Test(fade_value)) {
      draw_list = fade_value;
      for (draw_list.Start(); !draw_list.Last(); draw_list.Next()) {
        drawonable = Am_Drawonable::Narrow(draw_list.Get());
        if (drawonable) {
          drawonable->Set_Data_Store(nullptr);
          drawonable->Destroy();
        }
      }
    } else {
      drawonable = Am_Drawonable::Narrow(fade_value);
      if (drawonable) {
        drawonable->Set_Data_Store(nullptr);
        drawonable->Destroy();
      }
    }
    screen.Set(Am_MASK_BUFFER, (0L));
  }
}

// screens don't use the graphical_parts list so just use the part iterator.
// Everything in a screen must be a window, so return (0L) unless want_groups
Am_Define_Method(Am_Point_In_Or_Self_Method, Am_Object, screen_point_in_part,
                 (const Am_Object &in_obj, int x, int y,
                  const Am_Object &ref_obj, bool want_self, bool want_groups))
{
  if (!want_groups)
    return Am_No_Object;
  am_translate_coord_to_me(in_obj, ref_obj, x, y);
  if (Am_Point_In_Obj(in_obj, x, y, ref_obj)) {
    Am_Part_Iterator parts(in_obj);
    Am_Object object;
    // ** NOTE: order of parts is not necessarily the order the windows are in!
    for (parts.Start(); !parts.Last(); parts.Next()) {
      object = parts.Get();
      if (Am_Point_In_Obj(object, x, y, in_obj))
        return object;
    }
    //went through list, not in a part
    if (want_self)
      return in_obj;
    else
      return Am_No_Object;
  } else // if not in me, return Am_No_Object
    return Am_No_Object;
}

// screens don't use the graphical_parts list so just use the part iterator.
Am_Define_Method(Am_Point_In_Or_Self_Method, Am_Object, screen_point_in_leaf,
                 (const Am_Object &in_obj, int x, int y,
                  const Am_Object &ref_obj, bool want_self, bool want_groups))
{
  am_translate_coord_to_me(in_obj, ref_obj, x, y);
  if (Am_Point_In_Obj(in_obj, x, y, in_obj)) {
    Am_Part_Iterator parts(in_obj);
    Am_Object object, ret;
    // ** NOTE: order of parts is not necessarily the order the windows are in!
    for (parts.Start(); !parts.Last(); parts.Next()) {
      object = parts.Get();
      ret = Am_Point_In_Leaf(object, x, y, in_obj, want_self, want_groups);
      if (ret.Valid())
        return ret;
    }
    //went through list, not in a part
    if (want_self && want_groups) //self is a screen, which is "like" a group
      return in_obj;
    else
      return Am_No_Object;
  } else // if not in me, return (0L)
    return Am_No_Object;
}

void
init_screen()
{
  int width, height;
  Am_Drawonable *root = Am_Drawonable::Get_Root_Drawonable();
  root->Get_Size(width, height);

  Am_Screen =
      Am_Root_Object.Create(DSTR("Am_Screen"))
          .Add(Am_LEFT, 0)
          .Add(Am_TOP, 0)
          .Add(Am_WIDTH, width)
          .Add(Am_HEIGHT, height)
          .Add(Am_DRAWONABLE, (Am_Am_Drawonable)root)
          .Add(Am_OWNER_DEPTH, 0)
          .Add(Am_RANK, 0)
          .Add(Am_VISIBLE, true)
          .Add(Am_POINT_IN_OBJ_METHOD, generic_point_in_obj)
          .Add(Am_POINT_IN_PART_METHOD, screen_point_in_part)
          .Add(Am_POINT_IN_LEAF_METHOD, screen_point_in_leaf)
          .Add(Am_TRANSLATE_COORDINATES_METHOD, generic_translate_coordinates)
          .Add(Am_OFFSCREEN_DRAWONABLE, (0L))
          .Add(Am_DRAW_BUFFER, (0L))
          .Add(Am_MASK_BUFFER, (0L))
          .Add(Am_AS_LINE, false)             //screens are not like lines
          .Add(Am_LAST_USER_ID, Am_USER_ZERO) //supports multiple users
      ;

  Am_Screen.Add(Am_SCREEN, Am_Screen)
      .Add(Am_IS_COLOR, screen_is_color(Am_Screen));

  Am_Object_Advanced temp = (Am_Object_Advanced &)Am_Screen;
  temp.Get_Slot(Am_SCREEN).Set_Inherit_Rule(Am_LOCAL);
  temp.Get_Slot(Am_DRAWONABLE).Set_Inherit_Rule(Am_LOCAL);
  //// NYD: What other slots should Am_Screen have?  Color depth?
  Main_Demon_Queue = temp.Get_Queue();
  Am_Demon_Set demons = temp.Get_Demons().Copy();
  demons.Set_Object_Demon(Am_DESTROY_OBJ, screen_destroy_demon);
  temp.Set_Demons(demons);
}
