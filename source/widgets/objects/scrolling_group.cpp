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

#include <amulet/impl/opal_obj2.h>
#include <amulet/impl/opal_constraints.h>
#include <amulet/impl/am_state_store.h>
#include <amulet/impl/inter_undo.h>
#include <amulet/impl/anim_misc.h>

Am_Object Am_Scrolling_Group;

///////////////////////////////////////////////////////////////////////////
// Scrolling_Group
///////////////////////////////////////////////////////////////////////////

// scroll bar has SCROLL_BORDER pixels blank border all around.
// SCROLL_EXTRA_WIDTH is 2*SCROLL_BORDER
// SCROLL_MARGIN is # pixels between the scroll bars and the clip region
//#define SCROLL_BORDER 0 //2
//#define SCROLL_EXTRA_WIDTH 0 //4
#define SCROLL_MARGIN 2

// Am_Translate_Coordinates_Method:
//  - Stored in slot Am_TRANSLATE_COORDINATES_METHOD
//  - Given a point in the coordinate system of the group, converts
//    it to be in the coordinate system of scrolling group's owner
Am_Define_Method(Am_Translate_Coordinates_Method, void,
                 scroll_group_translate_coordinates,
                ( const Am_Object& self, const Am_Object& for_part,
                  int in_x, int in_y, int& out_x, int& out_y))
{
  bool using_constraint = true;
  //first, offset by the origin of the inside of the group
  if( using_constraint)
  {
    out_x = (int)self.Get( Am_LEFT) + in_x;
    out_y = (int)self.Get( Am_TOP) + in_y;
  }
  else
  {
    out_x = (int)self.Get( Am_LEFT) + in_x;
    out_y = (int)self.Get( Am_TOP) + in_y;
  }

  Am_Object h_scroller = self.Get_Object(Am_H_SCROLLER);
  Am_Object v_scroller = self.Get_Object(Am_V_SCROLLER);
  if( for_part != h_scroller && for_part != v_scroller)
  {
    // then is for something on the inside of the scrolling region
    if( using_constraint)
    {
      out_x += (int)self.Get( Am_CLIP_LEFT) - (int)self.Get(Am_X_OFFSET);
      out_y += (int)self.Get( Am_CLIP_TOP)  - (int)self.Get(Am_Y_OFFSET);
    }
    else
    {
      out_x += (int)self.Get( Am_CLIP_LEFT) - (int)self.Get(Am_X_OFFSET);
      out_y += (int)self.Get( Am_CLIP_TOP)  - (int)self.Get(Am_Y_OFFSET);
    }
  }
}

//coords are in my owner's
bool
in_inside_scroll_group(
  Am_Object in_obj,
  int x,
  int y )
{
  if( (bool)in_obj.Get( Am_VISIBLE))
  {
    int left, top, clipleft, cliptop, clipwidth, clipheight;
    left = in_obj.Get(Am_LEFT);
    top = in_obj.Get(Am_TOP);
    clipleft = in_obj.Get(Am_CLIP_LEFT);
    cliptop = in_obj.Get(Am_CLIP_TOP);
    if( x < left + clipleft || y < top + cliptop )
      return false;
    clipwidth = in_obj.Get(Am_CLIP_WIDTH);
    clipheight = in_obj.Get(Am_CLIP_HEIGHT);
    if( ( x >= left + clipleft + clipwidth ) || ( y >= top + cliptop + clipheight ) )
      return false;
    return true;
  }
  else
    return false;
}

//coords are w.r.t ref_obj, make them be w.r.t owner of in_obj
void
translate_coords_to_owner(
  Am_Object in_obj,
  Am_Object ref_obj,
  int x, int y, int &x1, int &y1 )
{
  Am_Object owner;
  owner = in_obj.Get_Owner();
  if( owner.Valid() && owner == ref_obj)
  {
    x1 = x;
    y1 = y;  //already OK
  }
  else //not the owner, use expensive transformation
    Am_Translate_Coordinates( ref_obj, x, y, owner, x1, y1 );
}

Am_Define_Method(Am_Point_In_Method, Am_Object, scroll_group_point_in_obj,
                ( const Am_Object& in_obj, int x, int y,
                  const Am_Object& ref_obj))
{
  if( (bool)in_obj.Get( Am_VISIBLE))
  {
    Am_Object owner = in_obj.Get_Owner();
    if( owner.Valid())
    {
      if( owner != ref_obj) // otherwise x,y ok
        Am_Translate_Coordinates( ref_obj, x, y, owner, x, y);
      int left = in_obj.Get(Am_LEFT);
      int top = in_obj.Get(Am_TOP);
      if( (x < left) ||( y < top)) return Am_No_Object;
      if( (x >= left + (int)in_obj.Get( Am_WIDTH)) ||
          (y >= top + (int)in_obj.Get( Am_HEIGHT)))
        return Am_No_Object;
      return in_obj;
    }
  }
  return Am_No_Object;
}

// x and y are in coord of ref_obj
Am_Define_Method(Am_Point_In_Or_Self_Method, Am_Object, scroll_group_point_in_part,
                ( const Am_Object& in_obj, int x, int y,
                  const Am_Object& ref_obj,
                  bool want_self, bool want_groups))
{
  //only works for objects inside my clip region, so can't "click through"
  //the scroll bars.
  int x1, y1;
  translate_coords_to_owner(in_obj, ref_obj, x, y, x1, y1);
  if( in_inside_scroll_group( in_obj, x1, y1)) {
    int offx, offy;
    scroll_group_translate_coordinates_proc(in_obj, Am_No_Object,
                                            0, 0, offx, offy);
    x1 -= offx;  //translate coordinates to the inside
    y1 -= offy;  //of the scrolling-group, which is the owner of the parts
    Am_Value_List comp;
    comp = in_obj.Get( Am_GRAPHICAL_PARTS);
    Am_Object object;
    for( comp.End() ; !comp.First() ; comp.Prev() ) {
      object = comp.Get() ;
      if( (want_groups || !am_is_group_and_not_pretending(object)) &&
          Am_Point_In_Obj( object, x1, y1, in_obj))
        return object;
    }
    //went through list, not in a part
    if( want_self && (want_groups || !am_is_group_and_not_pretending(in_obj)))
      return in_obj;
    else return Am_No_Object;
  }
  else return Am_No_Object;
}

Am_Define_Method(Am_Point_In_Or_Self_Method, Am_Object, scroll_group_point_in_leaf,
                ( const Am_Object& in_obj, int x, int y,
                  const Am_Object& ref_obj,
                  bool want_self, bool want_groups))
{
  //only works for objects inside my clip region, so can't "click through"
  //the scroll bars.
  int x1, y1;
  translate_coords_to_owner( in_obj, ref_obj, x, y, x1, y1 );
  if( in_inside_scroll_group( in_obj, x, y ) )
  {
    Am_Value val;
    val=in_obj.Peek(Am_PRETEND_TO_BE_LEAF);
    if( val.Valid() )
      return in_obj; // true if slot exists and is non-null
    else
    {
      int offx, offy;
      scroll_group_translate_coordinates_proc( in_obj, Am_No_Object, 0, 0,
                                               offx, offy);
      x1 -= offx;  //translate coordinates to the inside
      y1 -= offy;  //of the scrolling-group, which is the owner of the parts
      Am_Value_List comp;
      comp = in_obj.Get( Am_GRAPHICAL_PARTS );
      Am_Object object, ret;
      for( comp.End(); !comp.First(); comp.Prev() )
      {
        object = comp.Get() ;
        ret = Am_Point_In_Leaf( object, x1, y1, in_obj, want_self, want_groups );
        if( ret.Valid() )
          return ret;
      }
      //went through list, not in a part
      if( want_self && ( want_groups || !am_is_group_and_not_pretending( in_obj ) ) )
        return in_obj;
      else
        return Am_No_Object;
    }
  }
  else // if not in me, return (0L)
    return Am_No_Object;
}

// The left of the area that is used for the view of the insides
Am_Define_Formula(int, scroll_clip_left)
{
  int ret = get_scroll_border_thickness(self);
  if( (bool)self.Get(Am_V_SCROLL_BAR) && (bool)self.Get(Am_V_SCROLL_BAR_ON_LEFT))
  {
    Am_Object v_scroller = self.Get_Object(Am_V_SCROLLER);
    Am_Widget_Look look = v_scroller.Get( Am_WIDGET_LOOK );
    int extra_width = (look == Am_MOTIF_LOOK) ? 4 : 0;
    int scroll_width = (int)v_scroller.Get( Am_WIDTH ) - extra_width + SCROLL_MARGIN;
    ret += scroll_width;
  }
  return ret;
}

// The top of the area that is used for the view of the insides
Am_Define_Formula(int, scroll_clip_top)
{
  int ret = get_scroll_border_thickness(self);
  if( (bool)self.Get(Am_H_SCROLL_BAR) && ( bool)self.Get(Am_H_SCROLL_BAR_ON_TOP))
  {
    Am_Object h_scroller = self.Get_Object(Am_H_SCROLLER);
    Am_Widget_Look look = h_scroller.Get( Am_WIDGET_LOOK );
    int extra_width = (look == Am_MOTIF_LOOK) ? 4 : 0;
    int scroll_height = (int)h_scroller.Get(Am_HEIGHT) - extra_width + SCROLL_MARGIN;
    ret += scroll_height;
  }
  return ret;
}

// The area of the group that is used for the view of the insides
Am_Define_Formula(int, scroll_clip_width)
{
  int borderwidth = 2*get_scroll_border_thickness(self);
  int group_width = (int)self.Get(Am_WIDTH) - borderwidth;
  if( (bool)self.Get(Am_V_SCROLL_BAR))
  {
    Am_Object v_scroller = self.Get_Object(Am_V_SCROLLER);
    Am_Widget_Look look = v_scroller.Get( Am_WIDGET_LOOK );
    int extra_width = (look == Am_MOTIF_LOOK) ? 4 : 0;
    return group_width - (int) v_scroller.Get(Am_WIDTH) + extra_width - SCROLL_MARGIN;
  }
  else
    return group_width;
}

// The area of the group that is used for the view of the insides
Am_Define_Formula(int, scroll_clip_height)
{
  int borderwidth = 2*get_scroll_border_thickness(self);
  int group_height = (int)self.Get(Am_HEIGHT) - borderwidth;
  if( (bool)self.Get(Am_H_SCROLL_BAR))
  {
    Am_Object h_scroller = self.Get_Object(Am_H_SCROLLER);
    Am_Widget_Look look = h_scroller.Get( Am_WIDGET_LOOK );
    int extra_width = (look == Am_MOTIF_LOOK) ? 4 : 0;
    return group_height - (int) h_scroller.Get(Am_HEIGHT) + extra_width - SCROLL_MARGIN;
  }
  else
    return group_height;
}

Am_Define_Formula(int, v_scroll_left)
{
  Am_Object group = self.Get_Owner();
  Am_Widget_Look look = self.Get( Am_WIDGET_LOOK );
  int scroll_border = (look == Am_MOTIF_LOOK) ? 2 : 0;
  if( (bool)group.Get( Am_V_SCROLL_BAR_ON_LEFT ) )
    return -scroll_border;
  else // on right
    return (int)group.Get(Am_WIDTH) - (int)self.Get(Am_WIDTH) + scroll_border;
}

Am_Define_Formula(int, v_scroll_top)
{
  Am_Object group = self.Get_Owner();
  int group_border = get_scroll_border_thickness(group);
  Am_Widget_Look look = self.Get( Am_WIDGET_LOOK );
  int scroll_border = (look == Am_MOTIF_LOOK) ? 2 : 0;
  int ret = (int)group.Get(Am_CLIP_TOP) - scroll_border - group_border;
  return ret;
}

Am_Define_Formula(int, v_scroll_height)
{
  Am_Object group = self.Get_Owner();
  int group_border = get_scroll_border_thickness(group);
  Am_Widget_Look look = self.Get( Am_WIDGET_LOOK );
  int scroll_border = (look == Am_MOTIF_LOOK) ? 2 : 0;
  int ret = (int)group.Get(Am_CLIP_HEIGHT) + 2*scroll_border + 2*group_border;
  return ret;
}

Am_Define_Formula(bool, v_scroll_visible)
{
  Am_Object group = self.Get_Owner();
  if ((bool)group.Get(Am_VISIBLE) && (bool)group.Get(Am_V_SCROLL_BAR))
    return true;
  else
    return false;
}

Am_Define_Formula(bool, v_scroll_active)
{
  Am_Object group = self.Get_Owner();
  if( (bool)group.Get(Am_ACTIVE) &&
      ( (int)group.Get(Am_INNER_HEIGHT) >= (int)group.Get(Am_CLIP_HEIGHT) ) )
    return true;
  else
  {
    if (group.Valid()) {
      group.Set( Am_Y_OFFSET, 0);  //make sure at origin when go inactive
      group.Get_Object(Am_V_SCROLLER, Am_NO_DEPENDENCY).Get_Object(Am_COMMAND, Am_NO_DEPENDENCY).Set( Am_VALUE, 0);
    }
    return false;
  }
}

Am_Define_Formula(int, v_scroll_value_2)
{
  Am_Object group = self.Get_Owner();
  int val = (int)group.Get(Am_INNER_HEIGHT) - (int)group.Get(Am_CLIP_HEIGHT);
  if (val < 1) val = 1;
  return val;
}

Am_Define_Formula(float, v_scroll_percent_visible)
{
  Am_Object group = self.Get_Owner();
  float innerh =  group.Get(Am_INNER_HEIGHT);
  if (innerh == 0.0f) innerh = 1.0f;
  float val = (float)group.Get(Am_CLIP_HEIGHT) / innerh;
  if (val > 1.0) val = 1.0f;
  return val;
}

//Draw the scroll bars, then the background, then transform the
//coordinates and draw the graphical parts.
Am_Define_Method(Am_Draw_Method, void, scrolling_group_draw,
                 (Am_Object self, Am_Drawonable* drawonable,
                  int x_offset, int y_offset))
{
  int myleft = (int)self.Get( Am_LEFT) + x_offset;
  int mytop = (int)self.Get( Am_TOP) + y_offset;
  int mywidth = self.Get( Am_WIDTH);
  int myheight = self.Get( Am_HEIGHT);
  //Am_CLIP_LEFT and TOP are in my coord system, need window coords so add
  int insideleft = (int)self.Get(Am_CLIP_LEFT) + myleft;
  int insidetop = (int)self.Get(Am_CLIP_TOP) + mytop;
  int insideheight = self.Get(Am_CLIP_HEIGHT);
  int insidewidth = self.Get(Am_CLIP_WIDTH);
  Am_State_Store* state;
  if( mywidth && myheight) {
    // set a clip region here in case group is too small for the scroll bars
    drawonable->Push_Clip( myleft, mytop, mywidth, myheight);

    if( (bool)self.Get(Am_H_SCROLL_BAR)) {
      Am_Object h_scroller = self.Get_Object(Am_H_SCROLLER);
      state = Am_State_Store::Narrow( h_scroller.Get( Am_PREV_STATE));
      if( state->Visible( drawonable, myleft, mytop))
        Am_Draw( h_scroller, drawonable, myleft, mytop);
    }
    if( (bool)self.Get(Am_V_SCROLL_BAR)) {
      Am_Object v_scroller = self.Get_Object(Am_V_SCROLLER);
      state = Am_State_Store::Narrow( v_scroller.Get( Am_PREV_STATE));
      if( state->Visible( drawonable, myleft, mytop))
        Am_Draw( v_scroller, drawonable, myleft, mytop);
    }
    // draw the background
    Am_Style background_style, line_style;
    // use Am_INNER_FILL_STYLE if supplied, otherwise Am_FILL_STYLE
    Am_Value value;
    value=self.Peek(Am_INNER_FILL_STYLE);
    if( Am_Type_Class( value.type) == Am_WRAPPER) background_style = value;
    else background_style = self.Get(Am_FILL_STYLE);
    //  filled with background_style
    value=self.Peek(Am_LINE_STYLE);
    short thickness = 0;
    if( value.Valid())
    {
      line_style = value;  //else leave line_style (0L)
      Am_Line_Cap_Style_Flag cap;
      line_style.Get_Line_Thickness_Values( thickness, cap);
      if( thickness == 0)
        thickness = 1;
    }
    drawonable->Draw_Rectangle (line_style, background_style,
                                insideleft-thickness, insidetop-thickness,
                                insidewidth  + 2*thickness,
                                insideheight + 2*thickness);

    drawonable->Pop_Clip(); // done scroll bar drawin.type
    //set clip of inside
    drawonable->Push_Clip (insideleft, insidetop, insidewidth, insideheight);
    //now offset the area by the x and y offsets
    int x_offset = self.Get(Am_X_OFFSET);
    int y_offset = self.Get(Am_Y_OFFSET);
    insideleft -= x_offset;
    insidetop -= y_offset;

    Am_Value_List components;
    components = self.Get (Am_GRAPHICAL_PARTS);
    Am_Object item;
    for (components.Start(); !components.Last(); components.Next())
    {
      item = components.Get();
      state = Am_State_Store::Narrow (item.Get (Am_PREV_STATE));
      if (state->Visible (drawonable, insideleft, insidetop))
        Am_Draw (item, drawonable, insideleft, insidetop);
    }
    drawonable->Pop_Clip();
  }
}

Am_Define_Method(Am_Invalid_Method, void, scrolling_group_invalid,
                 (Am_Object group, Am_Object which_part,
                  int left, int top, int width, int height))
{
  Am_Object owner = group.Get_Owner(Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR);
  if( owner )
  {
    int my_left = group.Get (Am_LEFT, Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR);
    int my_top = group.Get (Am_TOP, Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR);
    int my_width = group.Get (Am_WIDTH, Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR);
    int my_height = group.Get (Am_HEIGHT, Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR);
    int final_left, final_top, final_width, final_height;
    Am_Object h_scroller = group.Get_Object(Am_H_SCROLLER, Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR);
    Am_Object v_scroller = group.Get_Object(Am_V_SCROLLER, Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR);
    // check if one of the scroll bars
    if (which_part != h_scroller && which_part != v_scroller)
    {
      // then transform based on offsets for the inside

      // adjust both the clip region and the object to be in my
      // parent's coordinate system
      int insideleft = group.Get(Am_CLIP_LEFT, Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR);
      int insidetop = group.Get(Am_CLIP_TOP, Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR);
      my_width = group.Get(Am_CLIP_WIDTH, Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR); //just re-set the value directly
      my_height = group.Get(Am_CLIP_HEIGHT, Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR);

      my_left += insideleft;
      my_top += insidetop;

      //now offset the area by the x and y offsets
      left -= (int)group.Get(Am_X_OFFSET, Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR);
      top -=  (int)group.Get(Am_Y_OFFSET, Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR);
    } // done with inside scrolling part

    // now clip incoming rectangle to my rectangle
    Am_Invalid_Rectangle_Intersect(left, top, width, height,
                                   my_left, my_top, my_width, my_height,
                                   final_left, final_top,
                                   final_width, final_height);
    if ((final_width > 0) && (final_height > 0))
      Am_Invalidate (owner, group, final_left, final_top, final_width,
                     final_height);
  }
}

//in the Am_Scrolling_Group itself to define how big a page jump is
Am_Define_Formula(int, h_scroll_jump_page)
{
  return (int)self.Get(Am_WIDTH) - 10;
}

//in the Am_Scrolling_Group itself to define how big a page jump is
Am_Define_Formula(int, v_scroll_jump_page)
{
  return (int)self.Get(Am_HEIGHT) - 10;
}

////////////////////////////////////////////////////////////////////////
// Scroll group do and undo
////////////////////////////////////////////////////////////////////////

void scroll_group_inter_abort_do(Am_Object command_obj, Am_Slot_Key slot)
{
  clear_save_pos_for_undo_proc(command_obj); //do the standard scrollbar stuff
  Am_Object inter = command_obj.Get_Owner();
  Am_Object scrollbar = inter.Get_Owner();
  Am_Object scroll_group = scrollbar.Get_Owner();
  Am_Object scroll_bar_command = scrollbar.Get_Object(Am_COMMAND);
  int val = scroll_bar_command.Get(Am_OLD_VALUE);
  scroll_group.Set(slot, val);
  //std::cout << "...Abort Setting Value of " << scrollbar << " to " << val
  //      <<std::endl <<std::flush;
  scrollbar.Set(Am_VALUE, val);
}

Am_Define_Method(Am_Object_Method, void, scroll_group_v_inter_abort_do,
                 (Am_Object command))
{
  scroll_group_inter_abort_do(command, Am_Y_OFFSET);
}

Am_Define_Method(Am_Object_Method, void, scroll_group_h_inter_abort_do,
                 (Am_Object command))
{
  scroll_group_inter_abort_do(command, Am_X_OFFSET);
}

//update the y_offset of the group
Am_Define_Method(Am_Object_Method, void, scroll_group_v_do,
                 (Am_Object command))
{
  Am_Object v_scroll = command.Get_Owner();
  Am_Object scroll_group = v_scroll.Get_Owner();
  int val = v_scroll.Get(Am_VALUE);
  scroll_group.Set( Am_Y_OFFSET, val);
}

//update the x_offset of the group
Am_Define_Method(Am_Object_Method, void, scroll_group_h_do,
                 (Am_Object command))
{
  Am_Object h_scroll = command.Get_Owner();
  Am_Object scroll_group = h_scroll.Get_Owner();
  int val = h_scroll.Get(Am_VALUE);
  scroll_group.Set( Am_X_OFFSET, val);
}

void scroll_group_general_undo_redo(Am_Object command_obj, bool undo,
                                    bool selective, Am_Slot_Key slot)
{
  Am_Value new_value, old_value;
  Am_Object scroll_bar, scroll_group;
  scroll_bar = command_obj.Get(Am_SAVED_OLD_OWNER);
  scroll_group = scroll_bar.Get_Owner();
  if (scroll_group.Valid()) {

    if (selective)  // then get current value from the scroll_bar
      new_value=scroll_bar.Peek(Am_VALUE);
    else // get current value from the command_obj
      new_value=command_obj.Peek(Am_VALUE);

    if (undo) old_value=command_obj.Peek(Am_OLD_VALUE);
    else  // repeat
      old_value=command_obj.Peek(Am_VALUE);

    command_obj.Set( Am_OLD_VALUE, new_value);
    command_obj.Set( Am_VALUE, old_value);
    //also set scroll group
    if (scroll_group.Valid()) {
      scroll_group.Set(slot, old_value);
    }
  }
}

Am_Define_Method(Am_Object_Method, void, scroll_group_v_undo,
                 (Am_Object command_obj))
{
  scroll_group_general_undo_redo(command_obj, true, false, Am_Y_OFFSET);
}

Am_Define_Method(Am_Object_Method, void, scroll_group_v_selective_undo,
                 (Am_Object command_obj))
{
  scroll_group_general_undo_redo(command_obj, true, true, Am_Y_OFFSET);
}

Am_Define_Method(Am_Object_Method, void, scroll_group_v_selective_repeat,
                 (Am_Object command_obj))
{
  scroll_group_general_undo_redo(command_obj, false, true, Am_Y_OFFSET);
}

Am_Define_Method(Am_Object_Method, void, scroll_group_h_undo,
                 (Am_Object command_obj))
{
  scroll_group_general_undo_redo(command_obj, true, false, Am_X_OFFSET);
}

Am_Define_Method(Am_Object_Method, void, scroll_group_h_selective_undo,
                 (Am_Object command_obj))
{
  scroll_group_general_undo_redo(command_obj, true, true, Am_X_OFFSET);
}

Am_Define_Method(Am_Object_Method, void, scroll_group_h_selective_repeat,
                 (Am_Object command_obj))
{
  scroll_group_general_undo_redo(command_obj, false, true, Am_X_OFFSET);
}

//////// Horizontal scrollbar stuff

Am_Define_Formula(int, h_scroll_left)
{
  Am_Object group = self.Get_Owner();
  int group_border = get_scroll_border_thickness(group);
  Am_Widget_Look look = self.Get( Am_WIDGET_LOOK );
  int scroll_border = (look == Am_MOTIF_LOOK) ? 2 : 0;
  return (int)group.Get(Am_CLIP_LEFT) - scroll_border - group_border;
}

Am_Define_Formula(int, h_scroll_top)
{
  Am_Object group = self.Get_Owner();
  Am_Widget_Look look = self.Get( Am_WIDGET_LOOK );
  int scroll_border = (look == Am_MOTIF_LOOK) ? 2 : 0;
  if( (bool)group.Get( Am_H_SCROLL_BAR_ON_TOP ) )
    return -scroll_border;
  else // on bottom
    return (int)group.Get(Am_HEIGHT) - (int)self.Get(Am_HEIGHT) + scroll_border;
}

Am_Define_Formula(int, h_scroll_width)
{
  Am_Object group = self.Get_Owner();
  int group_border = get_scroll_border_thickness(group);
  Am_Widget_Look look = self.Get( Am_WIDGET_LOOK );
  int scroll_border = (look == Am_MOTIF_LOOK) ? 2 : 0;
  return (int)group.Get(Am_CLIP_WIDTH) + 2*scroll_border + 2*group_border;
}

Am_Define_Formula(bool, h_scroll_visible)
{
  Am_Object group = self.Get_Owner();
  if ((bool)group.Get(Am_VISIBLE) &&
      (bool)group.Get(Am_H_SCROLL_BAR)) return true;
  else return false;
}

Am_Define_Formula(bool, h_scroll_active) {
  Am_Object group = self.Get_Owner();
  if ((bool)group.Get(Am_ACTIVE) &&
      ((int)group.Get(Am_INNER_WIDTH) >=
       (int)group.Get(Am_CLIP_WIDTH)))
    return true;
  else {
    if (group.Valid()) {
      group.Set( Am_X_OFFSET, 0);  //make sure at origin when go inactive
      group.Get_Object(Am_H_SCROLLER, Am_NO_DEPENDENCY).Get_Object(Am_COMMAND, Am_NO_DEPENDENCY).Set( Am_VALUE, 0);
    }
    return false;
  }
}

Am_Define_Formula(int, h_scroll_value_2) {
  Am_Object group = self.Get_Owner();
  int val = (int)group.Get(Am_INNER_WIDTH) - (int)group.Get(Am_CLIP_WIDTH);
  if (val < 1) val = 1;
  return val;
}

Am_Define_Formula(float, h_scroll_percent_visible) {
  Am_Object group = self.Get_Owner();
  float innerw =  group.Get(Am_INNER_WIDTH);
  if (innerw == 0.0f) innerw = 1.0f;
  float val = (float)group.Get(Am_CLIP_WIDTH) / innerw;
  if (val > 1.0) val = 1.0f;
  return val;
}

//explicitly remove Am_V_SCROLLER and Am_H_SCROLLER from the
//graphical objects list so programmers iterating through the list
//won't be confused by them
void remove_scrollers(Am_Object scroll_group)
{
  Am_Value_List components;
  components = scroll_group.Get (Am_GRAPHICAL_PARTS);
  // probably would be OK to just set the list to empty, but this is safer
  components.Start();
  if(!components.Member(scroll_group.Get_Object(Am_V_SCROLLER)))
    Am_Error("Scroll group doesn't contain V_scroller");
  components.Delete();

  components.Start();
  if(!components.Member(scroll_group.Get_Object(Am_H_SCROLLER)))
    Am_Error("Scroll group doesn't contain H_scroller");
  components.Delete();
  scroll_group.Set( Am_GRAPHICAL_PARTS, components);
}

////////////////////////////////////////////////////////////////////////

//goes in scroll bar inter
Am_Define_Formula (Am_Value, set_v_scroller_value_from_y_offset)
{
  Am_Value value;
  Am_Object scroll_group = self.Get_Owner();
  value = scroll_group.Peek(Am_Y_OFFSET);
  self.Set (Am_VALUE, value, Am_DONT_TELL_ANIMATORS);
  return value;
}

//goes in scroll bar inter
Am_Define_Formula (Am_Value, set_h_scroller_value_from_x_offset)
{
  Am_Value value;
  Am_Object scroll_group = self.Get_Owner();
  value = scroll_group.Peek(Am_X_OFFSET);
  self.Set (Am_VALUE, value, Am_DONT_TELL_ANIMATORS);
  return value;
}

Am_Define_Formula (Am_Value, get_impl_parent_from_group)
{
  Am_Value value;
  //self is cmd, owner is scroll_bar, owner is scroll_group
  Am_Object group = self.Get_Owner().Get_Owner();
  if( group.Valid() )
    value = group.Peek(Am_COMMAND);
  else
    value = Am_NOT_USUALLY_UNDONE;
  return value;
}

static void init()
{
  ///////////////////////////////////////////////////////////////////////////
  // Scrolling_Group
  ///////////////////////////////////////////////////////////////////////////
  Am_Object_Advanced obj_adv;
  Am_Object v_scroll, h_scroll;

  Am_Scrolling_Group = Am_Widget_Group.Create(DSTR("Scrolling_Group"))
     .Add( Am_X_OFFSET, 0 ) // x offset of the scrollable area
     .Add( Am_Y_OFFSET, 0 )
     .Set( Am_WIDTH, 150 )
     .Set( Am_HEIGHT, 150 )
     .Add( Am_ACTIVE, true )
     .Add( Am_ACTIVE_2, true ) // used by interactive tools
     .Add( Am_FILL_STYLE, Am_Default_Color )
     .Add( Am_LINE_STYLE, Am_Black ) // border around scrolling area
     .Add( Am_INNER_FILL_STYLE, 0 ) // if not 0, then inside of window color
     .Add( Am_H_SCROLL_BAR, true )
     .Add( Am_V_SCROLL_BAR, true )
     .Add( Am_H_SCROLL_BAR_ON_TOP, false )
     .Add( Am_V_SCROLL_BAR_ON_LEFT, false )
     .Add( Am_H_SMALL_INCREMENT, 10 ) // in pixels
     .Add( Am_H_LARGE_INCREMENT, h_scroll_jump_page )
     .Add( Am_V_SMALL_INCREMENT, 10 ) // in pixels
     .Add( Am_V_LARGE_INCREMENT, v_scroll_jump_page )
     .Set( Am_DRAW_METHOD, scrolling_group_draw )

     .Set( Am_INVALID_METHOD, scrolling_group_invalid )
     .Set( Am_TRANSLATE_COORDINATES_METHOD, scroll_group_translate_coordinates )
     .Set( Am_POINT_IN_PART_METHOD, scroll_group_point_in_part )
     .Set( Am_POINT_IN_LEAF_METHOD, scroll_group_point_in_leaf )
     .Set( Am_POINT_IN_OBJ_METHOD,  scroll_group_point_in_obj )

     .Add( Am_INNER_WIDTH, 400 )   // ** USE THE FORMULA FOR WIDTH FROM GROUPS
     .Add( Am_INNER_HEIGHT, 400 )  // ** USE THE FORMULA FROM GROUPS
     .Add( Am_COMMAND, Am_NOT_USUALLY_UNDONE ) // whether undoable
     .Add( Am_SET_COMMAND_OLD_OWNER, Am_Set_Old_Owner_To_Me )

     // internal slots
     .Add( Am_CLIP_LEFT, scroll_clip_left )
     .Add( Am_CLIP_TOP, scroll_clip_top )
     .Add( Am_CLIP_WIDTH, scroll_clip_width )
     .Add( Am_CLIP_HEIGHT, scroll_clip_height )
     .Add_Part(Am_V_SCROLLER, v_scroll =
                Am_Vertical_Scroll_Bar.Create(DSTR("V_Scroll_In_Group"))
                .Set( Am_VALUE, 0 )
		.Add( Am_Y1, set_v_scroller_value_from_y_offset )
                .Set( Am_LEFT, v_scroll_left )
                .Set( Am_TOP, v_scroll_top )
                .Set( Am_HEIGHT, v_scroll_height )
                .Set( Am_VISIBLE, v_scroll_visible )
                .Set( Am_ACTIVE, v_scroll_active )
                .Set( Am_ACTIVE_2, Am_From_Owner (Am_ACTIVE_2) )
                .Set( Am_WIDGET_LOOK, Am_From_Owner (Am_WIDGET_LOOK) )
                .Set( Am_FILL_STYLE, Am_From_Owner (Am_FILL_STYLE) )
                .Set( Am_VALUE_1, 0 )
                .Set( Am_VALUE_2, v_scroll_value_2 )
                .Set( Am_SMALL_INCREMENT, Am_From_Owner (Am_V_SMALL_INCREMENT) )
                .Set( Am_LARGE_INCREMENT, Am_From_Owner (Am_V_LARGE_INCREMENT) )
                .Set( Am_PERCENT_VISIBLE, v_scroll_percent_visible )
                );
  // Visual C++ can't handle long expressions
  Am_Scrolling_Group
     .Add_Part(Am_H_SCROLLER, h_scroll =
                Am_Horizontal_Scroll_Bar.Create(DSTR("H_Scroll_In_Group"))
                .Set( Am_VALUE, 0 )
		.Add( Am_X1, set_h_scroller_value_from_x_offset )
                .Set( Am_LEFT, h_scroll_left )
                .Set( Am_TOP, h_scroll_top )
                .Set( Am_WIDTH, h_scroll_width )
                .Set( Am_VISIBLE, h_scroll_visible )
                .Set( Am_ACTIVE, h_scroll_active )
                .Set( Am_ACTIVE_2, Am_From_Owner (Am_ACTIVE_2) )
                .Set( Am_WIDGET_LOOK, Am_From_Owner (Am_WIDGET_LOOK) )
                .Set( Am_FILL_STYLE, Am_From_Owner (Am_FILL_STYLE) )
                .Set( Am_VALUE_1, 0 )
                .Set( Am_VALUE_2, h_scroll_value_2 )
                .Set( Am_SMALL_INCREMENT, Am_From_Owner(Am_H_SMALL_INCREMENT) )
                .Set( Am_LARGE_INCREMENT, Am_From_Owner(Am_H_LARGE_INCREMENT) )
                .Set( Am_PERCENT_VISIBLE, h_scroll_percent_visible )

                 )
     ;
  obj_adv = (Am_Object_Advanced&)Am_Scrolling_Group;
  obj_adv.Get_Slot( Am_LINE_STYLE )
    .Set_Demon_Bits( Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  obj_adv.Get_Slot( Am_X_OFFSET )
    .Set_Demon_Bits( Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  obj_adv.Get_Slot( Am_Y_OFFSET )
    .Set_Demon_Bits( Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  obj_adv.Get_Slot( Am_H_SCROLL_BAR )
    .Set_Demon_Bits( Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  obj_adv.Get_Slot( Am_V_SCROLL_BAR )
    .Set_Demon_Bits( Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  obj_adv.Get_Slot( Am_H_SCROLL_BAR_ON_TOP )
    .Set_Demon_Bits( Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  obj_adv.Get_Slot( Am_V_SCROLL_BAR_ON_LEFT )
    .Set_Demon_Bits( Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  obj_adv.Get_Slot( Am_INNER_FILL_STYLE )
    .Set_Demon_Bits( Am_STATIONARY_REDRAW | Am_EAGER_DEMON);

  //explicitly remove Am_V_SCROLLER and Am_H_SCROLLER from the
  //graphical objects list so programmers iterating through the list
  //won't be confused by them
  remove_scrollers(Am_Scrolling_Group);

  v_scroll.Get_Object(Am_COMMAND )
    .Set( Am_LABEL, "Scroll Vertical" )
    .Set( Am_DO_METHOD, scroll_group_v_do )
    .Set( Am_UNDO_METHOD, scroll_group_v_undo )
    .Set( Am_REDO_METHOD, scroll_group_v_undo )
    .Set( Am_SELECTIVE_UNDO_METHOD, scroll_group_v_selective_undo )
    .Set( Am_SELECTIVE_REPEAT_SAME_METHOD, scroll_group_v_selective_repeat )
    .Set( Am_SELECTIVE_REPEAT_ON_NEW_METHOD, (0L) )
    //not undoable by default
    .Set( Am_IMPLEMENTATION_PARENT, get_impl_parent_from_group )
    .Set( Am_IMPLEMENTATION_CHILD, Am_DONT_UNDO_CHILDREN )
    ;
  v_scroll.Get_Object(Am_INTERACTOR).Get_Object(Am_COMMAND )
    .Set( Am_ABORT_DO_METHOD, scroll_group_v_inter_abort_do);

  h_scroll.Get_Object(Am_COMMAND )
    .Set( Am_LABEL, "Scroll Horizontal" )
    .Set( Am_DO_METHOD, scroll_group_h_do )
    .Set( Am_IMPLEMENTATION_PARENT, get_impl_parent_from_group )
    .Set( Am_UNDO_METHOD, scroll_group_h_undo )
    .Set( Am_REDO_METHOD, scroll_group_h_undo )
    .Set( Am_SELECTIVE_UNDO_METHOD, scroll_group_h_selective_undo )
    .Set( Am_SELECTIVE_REPEAT_SAME_METHOD, scroll_group_h_selective_repeat )
    .Set( Am_SELECTIVE_REPEAT_ON_NEW_METHOD, (0L) )
    .Set( Am_IMPLEMENTATION_CHILD, Am_DONT_UNDO_CHILDREN )
    ;
  h_scroll.Get_Object(Am_INTERACTOR).Get_Object(Am_COMMAND )
    .Set( Am_ABORT_DO_METHOD, scroll_group_h_inter_abort_do );
}

static Am_Initializer* initializer =
	new Am_Initializer(DSTR("Scrolling_Group"), init, 5.11f);
