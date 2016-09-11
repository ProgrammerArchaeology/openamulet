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

#include INITIALIZER__H
#include FORMULA__H
#include <amulet/impl/types_method.h>
#include <amulet/impl/am_demon_set.h>
#include <amulet/impl/object_misc.h>
#include <amulet/impl/opal_impl.h>
#include <amulet/impl/am_state_store.h>
#include <amulet/impl/opal_default_demon.h>
#include <amulet/impl/opal_methods.h>
#include "amulet/opal_intnl.h"


			
Am_Object Am_Graphical_Object;


//demon procedure
void graphics_create (Am_Object gr_object)
{
  Am_State_Store* state = new Am_State_Store (gr_object,
      gr_object.Get_Owner (),
      gr_object.Get (Am_VISIBLE, Am_RETURN_ZERO_ON_ERROR),
      gr_object.Get (Am_LEFT, Am_RETURN_ZERO_ON_ERROR),
      gr_object.Get (Am_TOP, Am_RETURN_ZERO_ON_ERROR),
      gr_object.Get (Am_WIDTH, Am_RETURN_ZERO_ON_ERROR),
      gr_object.Get (Am_HEIGHT, Am_RETURN_ZERO_ON_ERROR));
  gr_object.Set (Am_PREV_STATE, (Am_Ptr)state, Am_OK_IF_NOT_THERE);
}

//demon procedure
void graphics_destroy (Am_Object self)
{
  Am_Value value = self.Peek (Am_PREV_STATE, Am_NO_DEPENDENCY);
  self.Set (Am_PREV_STATE, (0L));
  if (!value.Exists())
    return;
  Am_State_Store* state = Am_State_Store::Narrow (value);
  if (state) {
    state->Remove ();
    delete state;
  }
}

//This is a demon procedure
void graphics_change_owner (Am_Object self, Am_Object, Am_Object)
{
  Am_State_Store* state = Am_State_Store::Narrow (self.Get (Am_PREV_STATE));
  if (state)
    state->Add (true);
}

//demon procedure
void graphics_repaint (Am_Slot first_invalidated)
{
  Am_Object self = first_invalidated.Get_Owner ();
  Am_Value value;
  value=self.Peek(Am_PREV_STATE);
  if (!value.Exists())
    return;
  Am_State_Store* state = Am_State_Store::Narrow (value);
  if (state)
    state->Add (false);
}

//demon procedure
void graphics_move (Am_Slot first_invalidated)
{
  Am_Object self = first_invalidated.Get_Owner ();
  Am_Value value;
  value=self.Peek(Am_PREV_STATE);
  if (!value.Exists())
    return;
  Am_State_Store* state = Am_State_Store::Narrow (value);
  if (state)
    state->Add (true);
}



void init_graphical_object()
{
  Am_Graphical_Object = Am_Root_Object.Create(DSTR("Am_Graphical_Object"))
    .Add (Am_LEFT, 0)
    .Add (Am_TOP, 0)
    .Add (Am_WIDTH, 10)
    .Add (Am_HEIGHT, 10)
    .Add (Am_VISIBLE, true)
    .Add (Am_RANK, -1)
    .Add (Am_OWNER_DEPTH, compute_depth)
    .Add (Am_WINDOW, pass_window)
    .Add (Am_PREV_STATE, (0L))
    .Add (Am_DRAW_METHOD, (0L))
    .Add (Am_MASK_METHOD, generic_mask)
    .Add (Am_INVALID_METHOD, (0L))
    .Add (Am_POINT_IN_OBJ_METHOD, generic_point_in_obj)
    .Add (Am_POINT_IN_PART_METHOD, generic_point_in_part)
    .Add (Am_POINT_IN_LEAF_METHOD, generic_point_in_leaf)
    .Add (Am_TRANSLATE_COORDINATES_METHOD, generic_translate_coordinates)
    .Add (Am_AS_LINE, false) //most objects are not like lines
    ;
  Am_Object_Advanced temp = (Am_Object_Advanced&)Am_Graphical_Object;
  Am_Demon_Set demons = temp.Get_Demons ().Copy ();
  demons.Set_Object_Demon (Am_CREATE_OBJ, graphics_create);
  demons.Set_Object_Demon (Am_COPY_OBJ, graphics_create);
  demons.Set_Object_Demon (Am_DESTROY_OBJ, graphics_destroy);
  demons.Set_Part_Demon (Am_CHANGE_OWNER, graphics_change_owner);
  demons.Set_Slot_Demon (Am_STATIONARY_REDRAW, graphics_repaint,
             Am_DEMON_PER_OBJECT | Am_DEMON_ON_CHANGE);
  demons.Set_Slot_Demon (Am_MOVING_REDRAW, graphics_move,
             Am_DEMON_PER_OBJECT | Am_DEMON_ON_CHANGE);
  demons.Set_Type_Check (1, Am_Check_Int_Type);
  unsigned short demon_mask = temp.Get_Demon_Mask ();
  demon_mask |= Am_STATIONARY_REDRAW | Am_MOVING_REDRAW;
  temp.Set_Demon_Mask (demon_mask);
  temp.Set_Demons (demons);
  temp.Get_Slot (Am_LEFT).Set_Demon_Bits (Am_MOVING_REDRAW | Am_EAGER_DEMON);
  temp.Get_Slot (Am_LEFT).Set_Type_Check (1);
  temp.Get_Slot (Am_TOP).Set_Demon_Bits (Am_MOVING_REDRAW | Am_EAGER_DEMON);
  temp.Get_Slot (Am_TOP).Set_Type_Check (1);
  temp.Get_Slot (Am_WIDTH).Set_Demon_Bits (Am_MOVING_REDRAW | Am_EAGER_DEMON);
  temp.Get_Slot (Am_HEIGHT).Set_Demon_Bits (Am_MOVING_REDRAW | Am_EAGER_DEMON);
  temp.Get_Slot (Am_VISIBLE).Set_Demon_Bits (Am_MOVING_REDRAW |
                         Am_EAGER_DEMON);
  temp.Get_Slot (Am_PREV_STATE).Set_Inherit_Rule (Am_LOCAL);
#ifdef DEBUG
  Am_Graphical_Object.Add(Am_OBJECT_IN_PROGRESS, 0)
    .Set_Inherit_Rule(Am_OBJECT_IN_PROGRESS, Am_LOCAL);
#endif
}
