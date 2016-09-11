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
#include VALUE_LIST__H
#include <amulet/impl/opal_impl.h>
#include <amulet/impl/gem_misc.h>
#include <amulet/impl/am_state_store.h>
#include FORMULA__H


Am_Object Am_Fade_Group;


static Am_Style clear_halftone (int percent)
{
  if (percent < 0)
    percent = 0;
  else if (percent > 99)
    percent = 99;
  static Am_Style halftone_array [100];
  if (!halftone_array[percent].Valid ())
    halftone_array[percent] = Am_Style::Halftone_Stipple (percent).
        Clone_With_New_Color (Am_Off_Bits);
  return halftone_array[percent];
}


void Am_Get_Effects_Buffers (Am_Object screen, unsigned short buffer,
			     Am_Drawonable*& primary, Am_Drawonable*& mask)
{
  Am_Value primary_value, mask_value;
  primary_value=screen.Peek(Am_DRAW_BUFFER);
  mask_value=screen.Peek(Am_MASK_BUFFER);
  int i;
  if (Am_Value_List::Test (primary_value)) {
    Am_Value_List primary_list = primary_value;
    Am_Value_List mask_list = mask_value;
    primary_list.Start ();
    mask_list.Start ();
    for (i = 0; (i < buffer) && !primary_list.Last (); ++i) {
      primary_list.Next ();
      mask_list.Next ();
    }
    if (primary_list.Last ()) {
      primary = (0L);
      mask = (0L);
    }
    else {
      primary = Am_Drawonable::Narrow (primary_list.Get ());
      mask = Am_Drawonable::Narrow (mask_list.Get ());
    }
  }
  else if (buffer == 0) {
    primary = Am_Drawonable::Narrow (primary_value);
    mask = Am_Drawonable::Narrow (mask_value);
  }
  else {
    primary = (0L);
    mask = (0L);
  }
  if (!primary) {
    Am_Drawonable* screen_drawonable =
          Am_Drawonable::Narrow (screen.Get(Am_DRAWONABLE));
    primary = screen_drawonable->
          Create_Offscreen (100, 100, Am_Off_Bits);
    mask = screen_drawonable->
          Create_Offscreen (100, 100, Am_Off_Bits);
    if (Am_Value_List::Test (primary_value)) {
      Am_Value_List primary_list = primary_value;
      Am_Value_List mask_list = mask_value;
      primary_list.Start ();
      mask_list.Start ();
      for (i = 0; i < buffer; ++i) {
        if (primary_list.Last ()) {
          primary_list.Add ((Am_Ptr)(0L));
          mask_list.Add ((Am_Ptr)(0L));
	}
        else {
          primary_list.Next ();
          mask_list.Next ();
	}
      }
      primary_list.Set ((Am_Ptr)primary);
      mask_list.Set ((Am_Ptr)mask);
      screen.Set (Am_DRAW_BUFFER, primary_list);
      screen.Set (Am_MASK_BUFFER, mask_list);
    }
    else if (buffer == 0) {
      screen.Set (Am_DRAW_BUFFER, (Am_Ptr)primary);
      screen.Set (Am_MASK_BUFFER, (Am_Ptr)mask);
    }
    else {
      Am_Value_List primary_list, mask_list;
      primary_list.Add (primary_value);
      mask_list.Add (mask_value);
      for (i = 1; i < buffer; ++i) {
        primary_list.Add ((Am_Ptr)(0L));
        mask_list.Add ((Am_Ptr)(0L));
      }
      primary_list.Add ((Am_Ptr)primary);
      mask_list.Add ((Am_Ptr)mask);
      screen.Set (Am_DRAW_BUFFER, primary_list);
      screen.Set (Am_MASK_BUFFER, mask_list);
    }
  }
}


Am_Define_Method (Am_Draw_Method, void, fade_group_draw,
		  (Am_Object self, Am_Drawonable* drawonable,
		   int x_offset, int y_offset))
{
  int left = (int)self.Get (Am_LEFT) + x_offset;
  int top = (int)self.Get (Am_TOP) + y_offset;
  int width = self.Get (Am_WIDTH);
  int height = self.Get (Am_HEIGHT);
  int value = self.Get (Am_VALUE);
  if (width <= 0 || height <= 0 || value >= 99)
    return;
  Am_Value_List components = self.Get (Am_GRAPHICAL_PARTS);
  Am_Object item;
  if (value == 0) {
    drawonable->Push_Clip (left, top, width, height);
    for (components.Start (); !components.Last (); components.Next ()) {
      item = components.Get ();
      Am_State_Store* state =
            Am_State_Store::Narrow (item.Get (Am_PREV_STATE));
      if (state->Visible (drawonable, left, top))
        Am_Draw (item, drawonable, left, top);
    }
    drawonable->Pop_Clip ();
    return;
  }
  Am_Object window = self.Get (Am_WINDOW);
  Am_Object screen;
  for (screen = window.Get_Owner(); !screen.Is_Instance_Of(Am_Screen);
       screen = screen.Get_Owner()) ;
  Am_Drawonable* local_drawonable;
  Am_Drawonable* mask_drawonable;
  int depth = self.Get (Am_FADE_DEPTH);
  Am_Get_Effects_Buffers (screen, depth, local_drawonable, mask_drawonable);
  int draw_width, draw_height;
  local_drawonable->Get_Size (draw_width, draw_height);
  if ((left+width > draw_width) || (top+height > draw_height)) {
    local_drawonable->Set_Size (left+width, top+height);
    mask_drawonable->Set_Size (left+width, top+height);
  }
  local_drawonable->Push_Clip (left, top, width, height);
  local_drawonable->Clear_Area (left, top, width, height);
  mask_drawonable->Clear_Area (left, top, width, height);
  for (components.Start (); !components.Last (); components.Next ()) {
    item = components.Get ();
    Am_State_Store* state =
          Am_State_Store::Narrow (item.Get (Am_PREV_STATE));
    if (state->Visible (local_drawonable, left, top)) {
      Am_Draw (item, local_drawonable, left, top);
      Am_Draw_Method mask_draw = item.Get (Am_MASK_METHOD);
      mask_draw.Call (item, mask_drawonable, left, top);
    }
  }
  local_drawonable->Draw_Rectangle (Am_No_Style, clear_halftone (value),
				    left, top, width, height, Am_DRAW_COPY);
  mask_drawonable->Draw_Rectangle (Am_No_Style, clear_halftone (value),
				   left, top, width, height, Am_DRAW_COPY);
  local_drawonable->Pop_Clip ();
  drawonable->Push_Clip (left, top, width, height);
  drawonable->Bitblt (left, top, width, height, mask_drawonable,
		      left, top, Am_DRAW_GRAPHIC_NIMP);
  drawonable->Bitblt (left, top, width, height, local_drawonable,
		      left, top, Am_DRAW_GRAPHIC_OR);
  drawonable->Pop_Clip ();
}

Am_Define_Method (Am_Draw_Method, void, fade_group_mask,
		  (Am_Object self, Am_Drawonable* drawonable,
		   int x_offset, int y_offset))
{
  int left = (int)self.Get (Am_LEFT) + x_offset;
  int top = (int)self.Get (Am_TOP) + y_offset;
  int width = self.Get (Am_WIDTH);
  int height = self.Get (Am_HEIGHT);
  int value = self.Get (Am_VALUE);
  if (width <= 0 || height <= 0 || value >= 99)
    return;
  Am_Value_List components = self.Get (Am_GRAPHICAL_PARTS);
  Am_Object item;
  if (value == 0) {
    drawonable->Push_Clip (left, top, width, height);
    for (components.Start (); !components.Last (); components.Next ()) {
      item = components.Get ();
      Am_State_Store* state =
            Am_State_Store::Narrow (item.Get (Am_PREV_STATE));
      if (state->Visible (drawonable, left, top)) {
        Am_Draw_Method mask_draw = item.Get (Am_MASK_METHOD);
        mask_draw.Call (item, drawonable, left, top);
      }
    }
    drawonable->Pop_Clip ();
    return;
  }
  Am_Object window = self.Get (Am_WINDOW);
  Am_Object screen;
  for (screen = window.Get_Owner(); !screen.Is_Instance_Of(Am_Screen);
       screen = screen.Get_Owner()) ;
  Am_Drawonable* local_drawonable;
  Am_Drawonable* mask_drawonable;
  int depth = self.Get (Am_FADE_DEPTH);
  Am_Get_Effects_Buffers (screen, depth, local_drawonable, mask_drawonable);
  int draw_width, draw_height;
  local_drawonable->Get_Size (draw_width, draw_height);
  if ((left+width > draw_width) || (top+height > draw_height)) {
    local_drawonable->Set_Size (left+width, top+height);
    mask_drawonable->Set_Size (left+width, top+height);
  }
  local_drawonable->Push_Clip (left, top, width, height);
  local_drawonable->Clear_Area (left, top, width, height);
  for (components.Start (); !components.Last (); components.Next ()) {
    item = components.Get ();
    Am_State_Store* state =
          Am_State_Store::Narrow (item.Get (Am_PREV_STATE));
    if (state->Visible (local_drawonable, left, top)) {
      Am_Draw_Method mask_draw = item.Get (Am_MASK_METHOD);
      mask_draw.Call (item, local_drawonable, left, top);
    }
  }
  local_drawonable->Draw_Rectangle (Am_No_Style, clear_halftone (value),
      left, top, width, height, Am_DRAW_COPY);
  local_drawonable->Pop_Clip ();
  drawonable->Push_Clip (left, top, width, height);
  drawonable->Bitblt (left, top, width, height, local_drawonable,
                      left, top, Am_DRAW_GRAPHIC_OR);
  drawonable->Pop_Clip ();
}

Am_Define_Formula (int, am_fade_depth)
{
  Am_Object owner = self.Get_Owner ();
  while (owner.Valid () && !owner.Is_Instance_Of (Am_Fade_Group))
    owner = owner.Get_Owner ();
  if (owner.Valid ())
    return (int)owner.Get (Am_FADE_DEPTH) + 1;
  else
    return 0;
}

static void init()
{
  Am_Fade_Group = Am_Group.Create (DSTR("Am_Fade_Group"))
    .Add (Am_FADE_DEPTH, am_fade_depth)
    .Add (Am_VALUE, 100)
    .Set (Am_DRAW_METHOD, fade_group_draw)
    .Set (Am_MASK_METHOD, fade_group_mask)
  ;
  ((Am_Object_Advanced&)Am_Fade_Group).Get_Slot (Am_VALUE).
      Set_Demon_Bits (Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
}


static Am_Initializer *initializer =
	new Am_Initializer(DSTR("Am_Fade_Group"), init, 2.1f);
