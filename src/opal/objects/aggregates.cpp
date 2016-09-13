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

#include <amulet/initializer.h>
#include <amulet/value_list.h>
#include <amulet/impl/types_method.h>
#include <amulet/impl/am_demon_set.h>
#include <amulet/impl/opal_impl.h>
#include <amulet/impl/opal_op.h>
#include <amulet/impl/opal_misc.h>
#include <amulet/impl/am_alignment.h>
#include <amulet/impl/am_state_store.h>
#include <amulet/impl/opal_default_demon.h>
#include <amulet/impl/opal_methods.h>
#include "amulet/opal_intnl.h"

Am_Object Am_Aggregate;
Am_Object Am_Group;

/******************************************
 ** Support for Aggregates and Groups    **
 ******************************************/

void
Am_Invalid_Rectangle_Intersect(int left, int top, int width, int height,
                               int my_left, int my_top, int my_width,
                               int my_height, int &final_left, int &final_top,
                               int &final_width, int &final_height)
{
  if (left >= 0)
    final_left = my_left + left;
  else
    final_left = my_left;
  if ((left + width) < my_width)
    final_width = my_left + left + width - final_left;
  else
    final_width = my_left + my_width - final_left;
  if (top >= 0)
    final_top = my_top + top;
  else
    final_top = my_top;
  if ((top + height) < my_height)
    final_height = my_top + top + height - final_top;
  else
    final_height = my_top + my_height - final_top;
}

Am_Define_Method(Am_Invalid_Method, void, aggregate_invalid,
                 (Am_Object group, Am_Object /*which_part*/, int left, int top,
                  int width, int height))
{
  Am_Object owner = group.Get_Owner(Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR);
  if (owner.Valid()) {
    int my_left =
        group.Get(Am_LEFT, Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR);
    int my_top = group.Get(Am_TOP, Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR);
    int my_width =
        group.Get(Am_WIDTH, Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR);
    int my_height =
        group.Get(Am_HEIGHT, Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR);
    int final_left, final_top, final_width, final_height;
    // clip incoming rectangle to my rectangle
    Am_Invalid_Rectangle_Intersect(left, top, width, height, my_left, my_top,
                                   my_width, my_height, final_left, final_top,
                                   final_width, final_height);
    if ((final_width > 0) && (final_height > 0))
      Am_Invalidate(owner, group, final_left, final_top, final_width,
                    final_height);
  }
}

Am_Define_Method(Am_Draw_Method, void, aggregate_draw,
                 (Am_Object self, Am_Drawonable *drawonable, int x_offset,
                  int y_offset))
{
  int left = (int)self.Get(Am_LEFT) + x_offset;
  int top = (int)self.Get(Am_TOP) + y_offset;
  int width = self.Get(Am_WIDTH);
  int height = self.Get(Am_HEIGHT);
  if (width && height) {
    drawonable->Push_Clip(left, top, width, height);
    //// DEBUG
    //std::cout << "aggregate " << *self << " clip mask <l" << left << " t" << top
    //     << " w" << width << " h" << height << ">" <<std::endl;
    Am_Value_List components = self.Get(Am_GRAPHICAL_PARTS);
    Am_Object item;
    for (components.Start(); !components.Last(); components.Next()) {
      item = components.Get();
      Am_State_Store *state = Am_State_Store::Narrow(item.Get(Am_PREV_STATE));
      if (state->Visible(drawonable, left, top))
        Am_Draw(item, drawonable, left, top);
    }
    drawonable->Pop_Clip();
  }
}

Am_Define_Method(Am_Draw_Method, void, aggregate_mask,
                 (Am_Object self, Am_Drawonable *drawonable, int x_offset,
                  int y_offset))
{
  int left = (int)self.Get(Am_LEFT) + x_offset;
  int top = (int)self.Get(Am_TOP) + y_offset;
  int width = self.Get(Am_WIDTH);
  int height = self.Get(Am_HEIGHT);
  if (width && height) {
    drawonable->Push_Clip(left, top, width, height);
    Am_Value_List components = self.Get(Am_GRAPHICAL_PARTS);
    Am_Object item;
    for (components.Start(); !components.Last(); components.Next()) {
      item = components.Get();
      Am_State_Store *state = Am_State_Store::Narrow(item.Get(Am_PREV_STATE));
      if (state->Visible(drawonable, left, top)) {
        Am_Draw_Method draw_method = item.Get(Am_MASK_METHOD);
        draw_method.Call(item, drawonable, left, top);
      }
    }
    drawonable->Pop_Clip();
  }
}

Am_Define_Method(Am_Point_In_Or_Self_Method, Am_Object, am_group_point_in_part,
                 (const Am_Object &in_obj, int x, int y,
                  const Am_Object &ref_obj, bool want_self, bool want_groups))
{
  am_translate_coord_to_me(in_obj, ref_obj, x, y);
  if (Am_Point_In_Obj(in_obj, x, y, in_obj).Valid()) {
    Am_Value_List comp;
    comp = in_obj.Get(Am_GRAPHICAL_PARTS);
    Am_Object object;
    for (comp.End(); !comp.First(); comp.Prev()) {
      object = comp.Get();
      if ((want_groups || !am_is_group_and_not_pretending(object)) &&
          Am_Point_In_Obj(object, x, y, in_obj))
        return object;
    }
    //in in_obj but not in part
    if (want_self && (want_groups || !am_is_group_and_not_pretending(in_obj)))
      return in_obj;
    else
      return Am_No_Object;
  } else
    return Am_No_Object;
}

Am_Define_Method(Am_Point_In_Or_Self_Method, Am_Object, am_group_point_in_leaf,
                 (const Am_Object &in_obj, int x, int y,
                  const Am_Object &ref_obj, bool want_self, bool want_groups))
{
  am_translate_coord_to_me(in_obj, ref_obj, x, y);
  if (Am_Point_In_Obj(in_obj, x, y, in_obj)) {
    Am_Value val;
    val = in_obj.Peek(Am_PRETEND_TO_BE_LEAF);
    if (val.Valid())
      return in_obj; // true if slot exists and is non-null
    else {
      Am_Value_List comp;
      comp = in_obj.Get(Am_GRAPHICAL_PARTS);
      Am_Object object, ret;
      for (comp.End(); !comp.First(); comp.Prev()) {
        object = comp.Get();
        ret = Am_Point_In_Leaf(object, x, y, in_obj, want_self, want_groups);
        if (ret.Valid())
          return ret;
      }
      //went through list, not in a part
      if (want_self && (want_groups || !am_is_group_and_not_pretending(in_obj)))
        return in_obj;
      else
        return Am_No_Object;
    }
  } else //  not in me
    return Am_No_Object;
}

//demon procedure
void
group_create(Am_Object self)
{
  Am_Object_Demon *create_demon = ((Am_Object_Advanced &)Am_Aggregate)
                                      .Get_Demons()
                                      .Get_Object_Demon(Am_CREATE_OBJ);
  create_demon(self);
  am_generic_renew_components(self);
}

//demon procedure
void
group_copy(Am_Object self)
{
  Am_Object_Demon *copy_demon = ((Am_Object_Advanced &)Am_Aggregate)
                                    .Get_Demons()
                                    .Get_Object_Demon(Am_COPY_OBJ);
  copy_demon(self);
  am_generic_renew_copied_comp(self);
}

//this goes in a demon procedure
void
am_generic_add_part(Am_Object owner, Am_Object old_object, Am_Object new_object)
{
  Am_Value_List components;
  if (old_object.Valid() && old_object.Is_Instance_Of(Am_Graphical_Object)) {
    owner.Make_Unique(Am_GRAPHICAL_PARTS);
    components = owner.Get(Am_GRAPHICAL_PARTS);
    components.Start();
    if (components.Member(old_object)) {
      components.Delete(false);
      int current_rank = old_object.Get(Am_RANK);
      components.Next();
      Am_Object current_obj;
      while (!components.Last()) {
        current_obj = components.Get();
        current_obj.Set(Am_RANK, current_rank);
        ++current_rank;
        components.Next();
      }
      Main_Demon_Queue.Enqueue(
          graphics_move, 0,
          ((Am_Object_Advanced &)old_object).Get_Slot(Am_LEFT));
      owner.Note_Changed(Am_GRAPHICAL_PARTS);
    }
  }
  if (new_object.Valid() && new_object.Is_Instance_Of(Am_Graphical_Object)) {
    owner.Make_Unique(Am_GRAPHICAL_PARTS);
    components = owner.Get(Am_GRAPHICAL_PARTS);
    bool was_valid = components.Valid();
    new_object.Set(Am_RANK, (int)components.Length());
    components.Add(new_object, Am_TAIL, false);
    Main_Demon_Queue.Enqueue(
        graphics_move, 0, ((Am_Object_Advanced &)new_object).Get_Slot(Am_LEFT));
    if (was_valid)
      owner.Note_Changed(Am_GRAPHICAL_PARTS);
    else
      owner.Set(Am_GRAPHICAL_PARTS, components);
  }
}

void
init_aggregates()
{
  Am_Aggregate = Am_Graphical_Object.Create(DSTR("Am_Aggregate"))
                     .Set(Am_DRAW_METHOD, aggregate_draw)
                     .Set(Am_MASK_METHOD, aggregate_mask)
                     .Set(Am_INVALID_METHOD, aggregate_invalid)
                     .Add(Am_GRAPHICAL_PARTS, Am_Value_List())
                     .Set(Am_POINT_IN_PART_METHOD, am_group_point_in_part)
                     .Set(Am_POINT_IN_LEAF_METHOD, am_group_point_in_leaf)
                     .Add(Am_LAYOUT, 0)
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
                     .Add(Am_MAX_SIZE, false);

  Am_Group = Am_Aggregate.Create(DSTR("Am_Group"));

  Am_Object_Advanced temp = (Am_Object_Advanced &)Am_Group;
  Am_Demon_Set demons = temp.Get_Demons().Copy();
  demons.Set_Object_Demon(Am_CREATE_OBJ, group_create);
  demons.Set_Object_Demon(Am_COPY_OBJ, group_copy);
  demons.Set_Part_Demon(Am_ADD_PART, am_generic_add_part);
  temp.Set_Demons(demons);
}
