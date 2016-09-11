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

// This file contains the functions for handling the Move_Grow Interactor
//
// Designed and implemented by Brad Myers


#include <am_inc.h>

#include AM_IO__H
#include <amulet/impl/inter_all.h>
#include <amulet/impl/am_inter_location.h>
#include <amulet/impl/opal_op.h>
#include <amulet/impl/opal_objects.h>

#include VALUE_LIST__H
#include FORMULA__H

#include INITIALIZER__H

#include <math.h>    // needed for sqrt for min-length of lines
    //math.h is in /usr/local/lib/gcc-lib/hppa1.1-hp-hpux/2.6.0/include/

#ifndef M_SQRT2
static const double M_SQRT2 = sqrt(2.0);
#endif

//
//////////////////////////////////////////////////////////////////
/// Helper functions
//////////////////////////////////////////////////////////////////

// deals with gridding.  Used here and in new_points inter
void Am_Get_Filtered_Input(Am_Object inter, const Am_Object& ref_obj,
			   int x, int y, int& out_x, int& out_y) {
  Am_Value value;
  value = inter.Get(Am_GRID_METHOD);
  if (value.Valid()) {
    Am_Custom_Gridding_Method method;
    method = value;
    Am_INTER_TRACE_PRINT(inter, "Custom Gridding function " << method);
    method.Call(inter, ref_obj, x, y, out_x, out_y);
    Am_INTER_TRACE_PRINT(inter, "     maps (" << x << "," << y << " to (" <<
		      out_x << "," << out_y << ")");
  }
  else { // if no procedure, check gridding by numbers
    int grid_x = inter.Get(Am_GRID_X);
    if (grid_x) { // then do some gridding by number
      int grid_y = inter.Get(Am_GRID_Y);
      if (!grid_y) grid_y = grid_x;  // use same value in each
				     // direction if y not supplied
      // ok for origins to be zero
      int grid_origin_x = inter.Get(Am_GRID_ORIGIN_X);
      int grid_origin_y = inter.Get(Am_GRID_ORIGIN_Y);
      int grid_x_div_2 = grid_x / 2;
      int grid_y_div_2 = grid_y / 2;

      out_x = grid_origin_x +
	(grid_x * (((x + grid_x_div_2) - grid_origin_x) / grid_x));
      out_y = grid_origin_y +
	(grid_y * (((y + grid_y_div_2) - grid_origin_y) / grid_y));
      Am_INTER_TRACE_PRINT(inter, "Gridding maps (" << x << "," << y << " to ("
			<< out_x << "," << out_y << ")");
    }
    else {
      out_x = x;
      out_y = y;
    }

  }
}

/// The Am_Clip_And_Map procedure works as follows:
///    (Am_Clip_And_Map val, val_1, val_2, target_val_1, target_val_2) takes
///    val, clips it to be in the range val_1 .. val_2, and then scales and
///    translates the value (using linear_interpolation) to be between
///    target_val_1 and target_val_2.  There are integer and float versions of
//     this function.
/// val_1 is allowed to be less than or greater than val_2.
///
long Am_Clip_And_Map(long val, long val_1, long val_2, long target_val_1,
		    long target_val_2) {
  if (val_1 == val_2) {
    if (val < val_1) return target_val_1;
    else return target_val_2;
  }
  else { // val_1 != val_2
    if (val < val_1 && val_1 < val_2) return target_val_1;
    else if (val_1 < val_2 && val_2 < val) return target_val_2;
    else if (val < val_2 && val_2 < val_1) return target_val_2;
    else if (val_2 < val_1 && val_1 < val) return target_val_1;
    //casting to long is important under Windows, else overflow!
    else return target_val_1 + (val - val_1) *
                               (target_val_2 - target_val_1)
				/ (val_2 - val_1);
  }
}

float Am_Clip_And_Map(float val, float val_1, float val_2,
		      float target_val_1, float target_val_2) {
  if (val_1 == val_2) {
    if (val < val_1) return target_val_1;
    else return target_val_2;
  }
  else { // val_1 != val_2
    if (val < val_1 && val_1 < val_2) return target_val_1;
    else if (val_1 < val_2 && val_2 < val) return target_val_2;
    else if (val < val_2 && val_2 < val_1) return target_val_2;
    else if (val_2 < val_1 && val_1 < val) return target_val_1;
    else return target_val_1 + ((val - val_1) * (target_val_2 - target_val_1)
				/ (val_2 - val_1));
  }
}

// used for calculating LEFT and TOP for a move.
void calc_position (Am_Object inter, Am_Object obj, const Am_Object& ref_obj,
		    int x, int y,
		    int& out_x, int& out_y) {

  Am_Move_Grow_Where_Attach attach = inter.Get(Am_WHERE_ATTACH);

  // first do x
  switch (attach.value) {
  case Am_ATTACH_NW_val:
  case Am_ATTACH_SW_val:
  case Am_ATTACH_W_val:
    out_x = x; break;
  case Am_ATTACH_NE_val:
  case Am_ATTACH_SE_val:
  case Am_ATTACH_E_val: {
    int width = obj.Get(Am_WIDTH);
    out_x = x - width + 1;
    break;
  }
  case Am_ATTACH_N_val:
  case Am_ATTACH_S_val:
  case Am_ATTACH_CENTER_val: {
    int width = obj.Get(Am_WIDTH);
    out_x = x - (width / 2);
    break;
  }
  case Am_ATTACH_WHERE_HIT_val: {
    int x_off = inter.Get(Am_X_OFFSET);
    out_x = x - x_off;
    break;
  }
  default: Am_ERRORO("Bad Am_WHERE_ATTACH " << attach
		<< " for move on inter " << inter, inter, Am_WHERE_ATTACH);
    break;
  }
    // now do Y
  switch (attach.value) {
  case Am_ATTACH_NW_val:
  case Am_ATTACH_NE_val:
  case Am_ATTACH_N_val:
    out_y = y; break;
  case Am_ATTACH_SW_val:
  case Am_ATTACH_SE_val:
  case Am_ATTACH_S_val: {
    int height = obj.Get(Am_HEIGHT);
    out_y = y - height + 1;
    break;
  }
  case Am_ATTACH_E_val:
  case Am_ATTACH_W_val:
  case Am_ATTACH_CENTER_val: {
    int height = obj.Get(Am_HEIGHT);
    out_y = y - (height / 2);
    break;
  }
  case Am_ATTACH_WHERE_HIT_val: {
    int y_off = inter.Get(Am_Y_OFFSET);
    out_y = y - y_off;
    break;
  }
  default: ; // don't need to do anything here since checked above
  } // end switch

  // set x,y based on gridding
  Am_Get_Filtered_Input(inter, ref_obj, out_x, out_y, out_x, out_y);

  Am_INTER_TRACE_PRINT(inter, "    calc_position attach=" << attach <<
		    " x,y=(" << x << "," << y << ") out_x,out_y=(" << out_x
		    << "," << out_y << ")");
}

// used for calculating line position for a move.
void calc_line_position (Am_Object inter, const Am_Object& ref_obj,
			 int x, int y,
		    int& out_x1, int& out_y1, int& out_x2, int& out_y2) {

  Am_Move_Grow_Where_Attach attach = inter.Get(Am_WHERE_ATTACH);

  Am_Inter_Location orig_points;
  orig_points = inter.Get(Am_OLD_VALUE);
  int x1,y1,x2,y2;
  orig_points.Get_Points(x1,y1,x2,y2);

  int dx = x2 - x1;
  int dy = y2 - y1;

  switch (attach.value) {
  case Am_ATTACH_END_1_val:
    // set x,y based on gridding
    Am_Get_Filtered_Input(inter, ref_obj, x, y, x, y);
    out_x1 = x;
    out_y1 = y;
    out_x2 = out_x1 + dx;
    out_y2 = out_y1 + dy;
    break;

  case Am_ATTACH_END_2_val:
    // set x,y based on gridding
    Am_Get_Filtered_Input(inter, ref_obj, x, y, x, y);
    out_x2 = x;
    out_y2 = y;
    out_x1 = out_x1 - dx;
    out_y1 = out_y1 - dy;
    break;

  case Am_ATTACH_WHERE_HIT_val: {
    int x_off = inter.Get(Am_X_OFFSET);
    int y_off = inter.Get(Am_Y_OFFSET);
    out_x1 = x - x_off;
    out_y1 = y - y_off;
    // set x,y based on gridding
    Am_Get_Filtered_Input(inter, ref_obj, out_x1, out_y1, out_x1, out_y1);
    out_x2 = out_x1 + dx;
    out_y2 = out_y1 + dy;
    break;
  }
  default:std::cerr << "** Amulet Error: Bad Am_WHERE_ATTACH " << attach
		<< " for move on inter " << inter;
    Am_Error();
    break;
  }
  Am_INTER_TRACE_PRINT(inter, "    calc_line_position attach=" << attach
		       << " x,y=(" << x << "," << y
		       << ") out_x,out_y,out_x2,out_y2=(" << out_x1 << ","
		       << out_y1 << "," << out_x2 << "," << out_y2 << ")");
}

// used for calculating width and height for a grow.  May also change left and
// top if growing from the left or top
static void calc_size (Am_Object& inter, const Am_Object& obj,
		       const Am_Object& ref_obj, int x, int y,
		       int& out_x, int& out_y, int& out_width,
		       int& out_height) {
  Am_Move_Grow_Where_Attach attach = inter.Get(Am_WHERE_ATTACH);

  int min_width = inter.Get(Am_MINIMUM_WIDTH);
  int min_height = inter.Get(Am_MINIMUM_HEIGHT);
  int left = obj.Get(Am_LEFT);
  int top = obj.Get(Am_TOP);
  int width = obj.Get(Am_WIDTH);
  int height = obj.Get(Am_HEIGHT);

  int rightp1 = left+width;//+1
  int bottomp1 = top+height;//+1

  if (attach == Am_ATTACH_WHERE_HIT) {
    attach = inter.Get(Am_WHERE_HIT_WHERE_ATTACH);
    int off = inter.Get(Am_X_OFFSET);
    x = x + off;
    off = inter.Get(Am_Y_OFFSET);
    y = y + off;
  }

  // set x,y based on gridding
  Am_Get_Filtered_Input(inter, ref_obj, x, y, x, y);

  // first do left and width
  switch (attach.value) {
  case Am_ATTACH_NW_val:
  case Am_ATTACH_SW_val:
  case Am_ATTACH_W_val:
    if (rightp1 - x <= min_width) out_x = rightp1 - min_width;
    else out_x = x;
    out_width = width + left - out_x;
    break;
  case Am_ATTACH_N_val:
  case Am_ATTACH_S_val: // no changes for these
    out_x = left;
    out_width = width;
    break;
  case Am_ATTACH_NE_val:
  case Am_ATTACH_SE_val:
  case Am_ATTACH_E_val:
    out_x = left;
    if (x - left < min_width) out_width = min_width;
    else out_width = x - left; //+1
    break;
  default: Am_ERRORO("Bad Am_WHERE_ATTACH " << attach
		<< " for grow on inter " << inter, inter, Am_WHERE_ATTACH);
    break;
  }
  // now do top and height
  switch (attach.value) {
  case Am_ATTACH_NW_val:
  case Am_ATTACH_NE_val:
  case Am_ATTACH_N_val:
    if (bottomp1 - y <= min_height) out_y = bottomp1 - min_height;
    else out_y = y;
    out_height = height + (top - out_y);
    break;
  case Am_ATTACH_E_val:
  case Am_ATTACH_W_val: // no changes for these
    out_y = top;
    out_height = height;
    break;
  case Am_ATTACH_SW_val:
  case Am_ATTACH_SE_val:
  case Am_ATTACH_S_val:
    out_y = top;
    if (y - top < min_height) out_height = min_height;
    else out_height = y - top; // + 1
    break;
  default: Am_ERRORO("Bad Am_WHERE_ATTACH " << attach
		<< " for grow on inter " << inter, inter, Am_WHERE_ATTACH);
    break;
  } // don't need default here since checked above
  Am_INTER_TRACE_PRINT(inter, "    calc_size attach=" << attach
		    << " x,y=(" << x << "," << y << ") out x,y,width,height=("
		    << out_x  << "," << out_y << "," << out_width << "," <<
		    out_height << ")");
}

// for moving the endpoint of a line.  Will set both even though only one will
// have changed
static void calc_line_end_move (Am_Object& inter, const Am_Object& ref_obj,
				int x, int y, int& out_x1, int& out_y1,
				int& out_x2, int& out_y2) {
  Am_Move_Grow_Where_Attach attach = inter.Get(Am_WHERE_ATTACH);
  int min_length = inter.Get(Am_MINIMUM_LENGTH);
  Am_Inter_Location orig_points;
  orig_points = inter.Get(Am_OLD_VALUE);
  int x1,y1,x2,y2;
  orig_points.Get_Points(x1,y1,x2,y2);
  if (attach == Am_ATTACH_WHERE_HIT)
    attach = inter.Get(Am_WHERE_HIT_WHERE_ATTACH);

  // set x,y based on gridding
  Am_Get_Filtered_Input(inter, ref_obj, x, y, x, y);

  if (min_length) { // time for expensive math
    int first_x = 0, first_y = 0, moving_x, moving_y;
    switch (attach.value) {
    case Am_ATTACH_END_1_val:
      first_x = x2;
      first_y = y2;
      break;
    case Am_ATTACH_END_2_val:
      first_x = x1;
      first_y = y1;
      break;
    default: Am_ERRORO("Bad Am_WHERE_ATTACH " << attach
		       << " for move line end on inter " << inter,
		       inter, Am_WHERE_ATTACH);
      break;
    }
    int x_dist = x - first_x;
    int y_dist = y - first_y;
    //casting to double is important under Windows!
    double denom = sqrt( double(x_dist) * x_dist  +  double(y_dist) * y_dist );
    if (denom < min_length) {
      if (denom == 0.0) { // don't divide by zero
	moving_x = (int) (x + min_length / M_SQRT2);
	moving_y = (int) (y + min_length / M_SQRT2);
      }
      else { // not zero, use calculated points
        //casting to long is important under Windows!
	moving_x = first_x + (int) ((long(x_dist) * min_length) / denom);
	moving_y = first_y + (int) ((long(y_dist) * min_length) / denom);
      }
    }
    else { // not less than mimimum length
      moving_x = x;
      moving_y = y;
    }
    // now set output values
    switch (attach.value) {
    case Am_ATTACH_END_1_val:
      out_x1 = moving_x;
      out_y1 = moving_y;
      out_x2 = first_x;
      out_y2 = first_y;
      break;
    case Am_ATTACH_END_2_val:
      out_x1 = first_x;
      out_y1 = first_y;
      out_x2 = moving_x;
      out_y2 = moving_y;
      break;
    default: Am_ERRORO("Bad Am_WHERE_ATTACH " << attach
		       << " for move line end on inter " << inter,
		       inter, Am_WHERE_ATTACH);
      break;
    }
  }
  else { // no mimimum length
    switch (attach.value) {
    case Am_ATTACH_END_1_val:
      out_x1 = x;
      out_y1 = y;
      out_x2 = x2;
      out_y2 = y2;
      break;
    case Am_ATTACH_END_2_val:
      out_x1 = x1;
      out_y1 = y1;
      out_x2 = x;
      out_y2 = y;
      break;
    default: Am_ERRORO("Bad Am_WHERE_ATTACH " << attach
		       << " for move line end on inter " << inter,
		       inter, Am_WHERE_ATTACH);
      break;
    }
  }
  Am_INTER_TRACE_PRINT(inter, "    calc_move_line_end attach=" << attach <<
		    " x,y=(" << x << ","
		    << y << ") out x1,y1,x2,y2=(" << out_x1 << "," << out_y1
		    << "," << out_x2 << "," << out_y2 << ")");
}

static Am_Object translate_if_needed (const Am_Object& inter,
				      const Am_Object& obj, int &x, int &y,
				      const Am_Object& ref_obj) {
  Am_Object owner = obj.Get_Owner();
  if (ref_obj != owner) {
    Am_INTER_TRACE_PRINT(inter, "Translating coordinates from " << ref_obj <<
		      " to " << owner);
    Am_Translate_Coordinates(ref_obj, x, y, owner, x, y);
    return owner;
  }
  else return ref_obj;
}

// Main routine, that picks the correct procedure above to calc the points.
//   If just moving, data.width and .height should be set already, otherwise,
//   sets all four
static void calc_all (Am_Object& inter, Am_Object obj, int x, int y,
		      const Am_Object& ref_obj, Am_Inter_Location& data) {
  bool growing = inter.Get(Am_GROWING);
  bool as_line = inter.Get(Am_AS_LINE);
  int a, b, c, d;
  Am_Object owner = translate_if_needed(inter, obj, x, y, ref_obj);
  if (growing) {
    if (as_line)
      calc_line_end_move (inter, owner, x, y, a, b, c, d);
    else // growing but not line
      calc_size (inter, obj, owner, x, y, a, b, c, d);
    data.Set_Location(as_line, owner, a, b, c, d, false);
  }
  else { // moving
    if (as_line) {
      calc_line_position (inter, owner, x, y, a, b, c, d);
      data.Set_Location(true, owner, a, b, c, d, false);
    }
    else {
      calc_position (inter, obj, owner, x, y, a, b);
      data.Set_Location(false, owner, a, b, false);
    }
  }
}

///////////////////////////////////////////////////////////////////////////
///  Next procedures are for setting the initial points
///////////////////////////////////////////////////////////////////////////

// calculates the actual attach based on which end point of obj the initial x
// and y are nearest to
void set_line_initial_where_hit_attach(Am_Object inter,
				       int first_x, int first_y,
				       int x1, int y1, int x2, int y2,
				       bool growing) {
  if (growing) {
    Am_Move_Grow_Where_Attach attach;
    //casting to long is important under Windows!
    long d1 = (long(x1 - first_x)*long(x1 - first_x)) +
        (long(y1 - first_y)*long(y1 - first_y));
    long d2 = (long(x2 - first_x)*long(x2 - first_x)) +
        (long(y2 - first_y)*long(y2 - first_y));
    if (d1 < d2) attach = Am_ATTACH_END_1;
    else attach = Am_ATTACH_END_2;
    Am_INTER_TRACE_PRINT(inter, "Calculated attach point for line is endpoint "
			 << attach);
    inter.Set(Am_WHERE_HIT_WHERE_ATTACH, attach, Am_OK_IF_NOT_THERE);
  }
  else {
    inter.Set(Am_X_OFFSET, first_x - x1);
    inter.Set(Am_Y_OFFSET, first_y - y1);
  }
}

void set_not_line_initial_where_hit_attach (Am_Object inter,
					    int first_x, int first_y,
					    int left, int top,
					    int width, int height,
					    bool growing) {
  if (growing) { //then need Am_WHERE_HIT_WHERE_ATTACH
    int w3 = width / 3;
    int h3 = height / 3;
    int x_off = left - first_x;  //will be negative if point is inside
    int y_off = top  - first_y;
    Am_Move_Grow_Where_Attach_vals xcontrol;
    Am_Move_Grow_Where_Attach attach;
    //Test right and bottom first so if real small, will grow from bottom-right
    //  first do x direction
    if (first_x > left + w3 + w3) {  // right third
      xcontrol = Am_ATTACH_E_val;
      inter.Set(Am_X_OFFSET, width + x_off - 1);
    }
    else if (first_x < left + w3) { // left third
      xcontrol = Am_ATTACH_W_val;
      inter.Set(Am_X_OFFSET, x_off);
    }
    else {  // center third
      inter.Set(Am_X_OFFSET, 0);
      xcontrol = Am_ATTACH_CENTER_val;
    }
    // now do y, and compute real attach
    if (first_y > top + h3 + h3) { //bottom third
      inter.Set(Am_Y_OFFSET, height + y_off - 1);
      switch (xcontrol) {
      case Am_ATTACH_W_val: attach = Am_ATTACH_SW; break;
      case Am_ATTACH_E_val: attach = Am_ATTACH_SE; break;
      case Am_ATTACH_CENTER_val: attach = Am_ATTACH_S; break;
      default: Am_ERRORO("Bad Am_WHERE_ATTACH "
			 << Am_Move_Grow_Where_Attach (xcontrol)
			 << " for where hit attach inter " << inter,
			 inter, Am_WHERE_ATTACH);
        break;
      }
    }
    else if (first_y < top + h3) { // top third
      inter.Set(Am_Y_OFFSET, y_off);
      switch (xcontrol) {
      case Am_ATTACH_W_val: attach = Am_ATTACH_NW; break;
      case Am_ATTACH_E_val: attach = Am_ATTACH_NE; break;
      case Am_ATTACH_CENTER_val: attach = Am_ATTACH_N; break;
      default: Am_ERRORO("Bad Am_WHERE_ATTACH "
			 << Am_Move_Grow_Where_Attach (xcontrol)
			 << " for where hit attach inter " << inter,
			 inter, Am_WHERE_ATTACH);
        break;
      }
    }
    else { // middle third
      inter.Set(Am_Y_OFFSET, 0);
      switch (xcontrol) {
      case Am_ATTACH_W_val: attach = Am_ATTACH_W; break;
      case Am_ATTACH_E_val: attach = Am_ATTACH_E; break;
      case Am_ATTACH_CENTER_val: //hack, for center, use NW
	attach = Am_ATTACH_NW;
	inter.Set(Am_X_OFFSET, x_off);
	inter.Set(Am_Y_OFFSET, y_off);
	break;
      default: Am_ERRORO("Bad Am_WHERE_ATTACH "
			 << Am_Move_Grow_Where_Attach (xcontrol)
			 << " for where hit attach inter " << inter,
			 inter, Am_WHERE_ATTACH);
        break;
      }
    }
    Am_INTER_TRACE_PRINT(inter, "Calculated attach point for non-line is "
		      << attach);
    inter.Set(Am_WHERE_HIT_WHERE_ATTACH, attach, Am_OK_IF_NOT_THERE);
  }
  else { // moving
    inter.Set(Am_X_OFFSET, first_x - left);
    inter.Set(Am_Y_OFFSET, first_y - top);
  }
}

void Am_Copy_Data_Into_Slot(Am_Object dest_obj, Am_Slot_Key dest_slot,
			    Am_Inter_Location src_data) {
  if (Am_Type_Class (dest_obj.Get_Slot_Type(dest_slot)) == Am_WRAPPER) {
    // then just copy the new data into the old data
    Am_Inter_Location dest_data;
    dest_obj.Make_Unique (dest_slot);
    dest_data = dest_obj.Get(dest_slot);
    dest_data.Copy_From(src_data, false);
    dest_obj.Note_Changed(dest_slot);  //since destructively modified
  }
  else //set slot with a copy of data
    dest_obj.Set(dest_slot, src_data.Copy(), Am_OK_IF_NOT_THERE);
}

// sets Am_OLD_VALUE and INTERIM_VALUE of inter with orig_points
// returns orig_points
Am_Inter_Location set_initial_values (Am_Object inter, Am_Object obj,
				      int first_x, int first_y,
				      Am_Object ref_obj) {
  Am_Inter_Location orig_points, interim_points;
  bool allocated_new_orig = false;
  if (Am_Type_Class (inter.Get_Slot_Type(Am_OLD_VALUE)) == Am_WRAPPER) {
    inter.Make_Unique (Am_OLD_VALUE);
    orig_points = inter.Get(Am_OLD_VALUE);
  }
  else allocated_new_orig = true;

  bool growing = inter.Get(Am_GROWING);
  bool as_line = inter.Get(Am_AS_LINE);
  bool where_hit_attach =
    inter.Get(Am_WHERE_ATTACH) == Am_ATTACH_WHERE_HIT;
  Am_Object owner = obj.Get_Owner();
  if (where_hit_attach) // convert firstX and Y to be same coords as x1,y1
    Am_Translate_Coordinates(ref_obj, first_x, first_y, owner,
			     first_x, first_y);

  if (as_line) {
    int x1 = obj.Get(Am_X1);
    int y1 = obj.Get(Am_Y1);
    int x2 = obj.Get(Am_X2);
    int y2 = obj.Get(Am_Y2);
    orig_points.Set_Location(true, owner, x1, y1, x2, y2, false);

    if (where_hit_attach)
      set_line_initial_where_hit_attach(inter, first_x, first_y,
					x1, y1, x2, y2, growing);
  }
  else { // not a line
    int left = obj.Get(Am_LEFT);
    int top = obj.Get(Am_TOP);
    int width = obj.Get(Am_WIDTH);
    int height = obj.Get(Am_HEIGHT);
    orig_points.Set_Location(false, owner, left, top, width, height, false);
    orig_points.Set_Growing(growing);

    if (where_hit_attach)
      set_not_line_initial_where_hit_attach(inter, first_x, first_y,
					    left, top, width, height, growing);
  }

  //copy the orig points to be the first interim value
  Am_Copy_Data_Into_Slot(inter, Am_INTERIM_VALUE, orig_points);
  interim_points = inter.Get(Am_INTERIM_VALUE); //needed for return value

  if (allocated_new_orig) inter.Set (Am_OLD_VALUE, orig_points, Am_OK_IF_NOT_THERE);
  else inter.Note_Changed (Am_OLD_VALUE);

  return interim_points;
}

//////////////////////////////////////////////////////////////////
/// Actual moving
//////////////////////////////////////////////////////////////////

//coords are w.r.t. ref_obj
static Am_Inter_Location move_grow_interim_val (Am_Object& inter,
			 Am_Object& command_obj, Am_Object& object_to_move,
			 int x, int y, Am_Object& ref_obj, Am_Input_Char& ic) {
  Am_Inter_Location data (inter.Get(Am_INTERIM_VALUE));
  //don't need Make_Unique on Am_INTERIM_VALUE because will have called
  //Am_Copy_Data_Into_Slot during initialize which does it.
  calc_all (inter, object_to_move, x, y, ref_obj, data);
  inter.Note_Changed (Am_INTERIM_VALUE);
  Am_Inter_Call_Both_Method(inter, command_obj, Am_INTERIM_DO_METHOD, x, y,
			    ref_obj, ic, object_to_move, data);
  return data;
}

Am_Object move_grow_set_impl_command(Am_Object inter,
				     Am_Object object_modified,
				     Am_Inter_Location data) {
  Am_Object impl_command;
  impl_command = inter.Get_Object(Am_IMPLEMENTATION_COMMAND);
  if(impl_command.Valid()) {
    impl_command.Set(Am_OBJECT_MODIFIED, object_modified);
    impl_command.Set(Am_GROWING, (bool)inter.Get(Am_GROWING));

    //adjust the data so it has the right owner
    if (!data.Translate_To(object_modified.Get_Owner()))
      Am_ERROR("Can't translate data to owner of " << object_modified);
    //set current value into the Am_VALUE slot
    Am_Copy_Data_Into_Slot(impl_command, Am_VALUE, data);

    //set original data into the OLD_VALUE slot
    Am_Inter_Location orig_data;
    orig_data = inter.Get(Am_OLD_VALUE);
    Am_Copy_Data_Into_Slot(impl_command, Am_OLD_VALUE, orig_data);
  }
  return impl_command;
}

//////////////////////////////////////////////////////////////////
/// Main METHOD routines
//////////////////////////////////////////////////////////////////

Am_Define_Method(Am_Inter_Internal_Method, void, Am_Move_Grow_Start_Method,
		 (Am_Object& inter, Am_Object& object, Am_Object& event_window,
		  Am_Input_Event *ev)) {
  int x = ev->x;
  int y = ev->y;
  Am_Input_Char ic = ev->input_char;
  Am_Object command_obj = inter.Get_Object(Am_COMMAND);
  Am_INTER_TRACE_PRINT(inter, "Move_Grow starting over " << object);

  //do this before prototype's method, so won't ever change state
  bool growing = inter.Get(Am_GROWING);
  Am_Slot_Key slot_for_inactive =
    (growing ? Am_GROW_INACTIVE : Am_MOVE_INACTIVE);
  //check if object is allowed to grow or move
  if (!Am_Check_Inter_Abort_Inactive_Object(object, slot_for_inactive, inter))
    return;

  // first, call the prototype's method
  Am_Inter_Internal_Method inter_method;
  inter_method = Am_Interactor.Get(Am_INTER_START_METHOD);
  inter_method.Call(inter, object, event_window, ev);

  //sets Am_OLD_VALUE of inter with orig points of object
  Am_Inter_Location data = set_initial_values (inter, object, x, y,
					       event_window);

  inter.Set(Am_OBJECT_MODIFIED, object);

  // Now, call the START_DO method
  Am_Inter_Call_Both_Method(inter, command_obj, Am_START_DO_METHOD, x, y,
			    event_window, ic, object, data);

  // check if start method aborted or destroyed this interactor
  Am_Value state = inter.Peek(Am_CURRENT_STATE);
  if (!state.Exists() || ((int)state == Am_INTER_ABORTING)) return;

  // now call INTERIM_DO method on first points
  data = move_grow_interim_val(inter, command_obj, object, x, y,
			       event_window, ic);

  // if not continuous, call DO method
  if (!(bool)inter.Get(Am_CONTINUOUS)) {
    // check if INTERIM_DO method aborted or destroyed this interactor
    state = inter.Peek(Am_CURRENT_STATE);
    if (!state.Exists() || ((int)state == Am_INTER_ABORTING)) return;

    //data is points calculated by move_grow_interim_val
    Am_Call_Final_Do_And_Register(inter, command_obj, x, y, event_window, ic,
				  object, data, move_grow_set_impl_command);
  }
}

Am_Define_Method(Am_Inter_Internal_Method, void, Am_Move_Grow_Abort_Method,
		 (Am_Object& inter, Am_Object& /* object */,
		  Am_Object& event_window, Am_Input_Event *ev)) {
  Am_INTER_TRACE_PRINT(inter, "Move_Grow Aborting");
  // ignore object parameter, used saved object
  Am_Object object;
  object = inter.Get(Am_START_OBJECT);

  // first, call the prototype's method

  Am_Inter_Internal_Method inter_method;
  inter_method = Am_Interactor.Get(Am_INTER_ABORT_METHOD);
  inter_method.Call(inter, object, event_window, ev);

  // Now, call the correct Command method
  Am_Object command_obj;
  command_obj = inter.Get(Am_COMMAND);
  Am_Inter_Location orig_points;
  orig_points = inter.Get(Am_OLD_VALUE);

  Am_Inter_Call_Both_Method(inter, command_obj, Am_ABORT_DO_METHOD,
	    ev->x, ev->y, event_window, ev->input_char, object, orig_points);
}


Am_Define_Method(Am_Inter_Internal_Method, void, Am_Move_Grow_Outside_Method,
		 (Am_Object& inter, Am_Object& /* object */,
		  Am_Object& event_window, Am_Input_Event *ev)) {
  Am_INTER_TRACE_PRINT(inter, "Move_Grow Outside");

  // ignore object parameter, used saved object
  Am_Object object;
  object = inter.Get(Am_START_OBJECT);

  // first, call the prototype's method
  Am_Inter_Internal_Method inter_method;
  inter_method = Am_Interactor.Get(Am_INTER_OUTSIDE_METHOD);
  inter_method.Call(inter, object, event_window, ev);

  // Now, call the abort Command method
  Am_Object command_obj;
  command_obj = inter.Get(Am_COMMAND);
  Am_Inter_Location orig_points;
  orig_points = inter.Get(Am_OLD_VALUE);
  Am_Inter_Call_Both_Method(inter, command_obj, Am_ABORT_DO_METHOD,
	    ev->x, ev->y, event_window, ev->input_char, object, orig_points);
}

Am_Define_Method(Am_Inter_Internal_Method, void,
		 Am_Move_Grow_Back_Inside_Method,
		 (Am_Object& inter, Am_Object& /*object*/,
		  Am_Object& event_window, Am_Input_Event *ev)) {
  int x = ev->x;
  int y = ev->y;
  // ignore object parameter, used saved object
  Am_Object object, command_obj;
  object = inter.Get(Am_START_OBJECT);
  Am_INTER_TRACE_PRINT(inter, "Move_Grow back inside over " << object);
  // first, call the prototype's method
  Am_Inter_Internal_Method inter_method;
  inter_method = Am_Interactor.Get(Am_INTER_BACK_INSIDE_METHOD);
  inter_method.Call(inter, object, event_window, ev);

  // get the original points to use as the data for start
  Am_Inter_Location data;
  data = inter.Get(Am_OLD_VALUE);
  command_obj = inter.Get(Am_COMMAND);
  Am_Inter_Call_Both_Method(inter, command_obj, Am_START_DO_METHOD, x, y,
			    event_window, ev->input_char, object, data);

  // now call INTERIM_DO method on current point
  data = move_grow_interim_val(inter, command_obj, object, x, y, event_window,
			       ev->input_char);
}

Am_Define_Method(Am_Inter_Internal_Method, void, Am_Move_Grow_Running_Method,
		 (Am_Object& inter, Am_Object& /* object */,
		  Am_Object& event_window, Am_Input_Event *ev)) {
  int x = ev->x;
  int y = ev->y;
  // ignore object parameter, used saved object
  Am_Object object;
  object = inter.Get(Am_START_OBJECT);

  Am_INTER_TRACE_PRINT(inter, "Move_Grow running over " << object);

  // first, call the prototype's method
  Am_Inter_Internal_Method inter_method;
  inter_method = Am_Interactor.Get(Am_INTER_RUNNING_METHOD);
  inter_method.Call(inter, object, event_window, ev);

  Am_Object command_obj;
  command_obj = inter.Get_Object(Am_COMMAND);
  Am_Inter_Location data = move_grow_interim_val(inter, command_obj,
						 object, x, y, event_window,
						 ev->input_char);
}

Am_Define_Method(Am_Inter_Internal_Method, void, Am_Move_Grow_Stop_Method,
		 (Am_Object& inter, Am_Object& /* object */,
		  Am_Object& event_window, Am_Input_Event *ev)) {
  int x = ev->x;
  int y = ev->y;
  // ignore object parameter, used saved object
  Am_Object object;
  object = inter.Get(Am_START_OBJECT);
  Am_INTER_TRACE_PRINT(inter, "Move_Grow stopping over " << object);
  //now, call interim_do on last point
  Am_Input_Char ic = ev->input_char;
  Am_Object command_obj;
  command_obj = inter.Get_Object(Am_COMMAND);
  Am_Inter_Location data = move_grow_interim_val(inter, command_obj, object,
						 x, y, event_window, ic);

  // call DO method
  //data is points calculated by move_grow_interim_val
  Am_Call_Final_Do_And_Register(inter, command_obj, x, y, event_window, ic,
				object, data, move_grow_set_impl_command);

  // LAST, call the prototype's method
  Am_Inter_Internal_Method inter_method;
  inter_method = Am_Interactor.Get(Am_INTER_STOP_METHOD);
  inter_method.Call(inter, object, event_window, ev);
}

//exported for when want to pretend an interim method is a final method, like
//for scroll bar indicators (scroll_widgets.cc)
void Am_Move_Grow_Register_For_Undo(Am_Object inter) {
  Am_Object command_obj = inter.Get_Object(Am_COMMAND);
  Am_Object object_modified;
  object_modified = inter.Get(Am_START_OBJECT);
  Am_Inter_Location data;
  data = inter.Get(Am_INTERIM_VALUE);
  Am_Register_For_Undo(inter, command_obj, object_modified, data,
		       move_grow_set_impl_command);
}

//////////////////////////////////////////////////////////////////////////////
// Default methods to make the Move_Grow Interactor work
//////////////////////////////////////////////////////////////////////////////

void Am_Report_Set_Vis(Am_Object inter, Am_Object obj, bool value) {
 std::cout << "++Object " << inter << " setting Am_VISIBLE of " << obj << " to ";
  if (value)std::cout << "true\n" <<std::flush;
  else std::cout << "false\n" <<std::flush;
}

//////////////////////////////////////////////////////////////////////////////
// Slots to Set stuff

// *** DON'T SUPPORT SLOTS_TO_SET because it makes UNDO really difficult **
// ********
//////////////////////////////////////////////////////////////////////////////

/* ******
* void slot_set_and_print(Am_Object obj, Am_Slot_Key slot, Am_Slot_Key
*			default_slot, int val) {
*  // slot == 1 corresponds to using TRUE.   Hope no slots are 1
*  if (slot == 1) slot = default_slot;
*  Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING, " " << slot << "=" << val);
*  obj.Set(slot, val);
*}
*
* //slots_to_set can be (0L), or a list of 4 values.  Each value can be NULL,
* //which means don't set that value, true (or 1) which means use the default
* //value, or a slot name, to set for that position.
*Am_Set_Slots_From_Location(Am_Object obj, Am_Object main_obj,
*				Am_Value slots_to_set,
*				Am_Inter_Location data, bool growing,
*				int x_offset, int y_offset) {
*  Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING, "++Object " << main_obj <<
*      " setting obj=" << obj);
*  if (!slots_to_set.Valid()) {
*    if (data->as_line) {
*      slot_set_and_print(obj, Am_X1, 0, data->data.line.x1 + x_offset);
*      slot_set_and_print(obj, Am_Y1, 0, data->data.line.y1 + y_offset);
*      slot_set_and_print(obj, Am_X2, 0, data->data.line.x2 + x_offset);
*      slot_set_and_print(obj, Am_Y2, 0, data->data.line.y2 + y_offset);
*    }
*    else {
*      slot_set_and_print(obj, Am_LEFT, 0, data->data.rect.left + x_offset);
*      slot_set_and_print(obj, Am_TOP, 0, data->data.rect.top + y_offset);
*      if (growing) {
*	slot_set_and_print(obj, Am_WIDTH, 0, data->data.rect.width);
*	slot_set_and_print(obj, Am_HEIGHT, 0, data->data.rect.height);
*      }
*    }
*  }
*  else { // complex control over which slots to use
*    // slots_to_set should be a list of slots to use or NULLs
*    Am_Value_List list;
*    list = slots_to_set;
*    list.Start();
*    Am_Value slot;
*    if (data->as_line) {
*      list.Get(slot);
*      if (slot.Valid())
*	slot_set_and_print(obj, (int)slot,Am_X1,data->data.line.x1 + x_offset);
*
*      list.Next();
*      list.Get(slot);
*      if (slot.Valid())
*	slot_set_and_print(obj, (int)slot,Am_Y1,data->data.line.y1 + y_offset);
*
*      list.Next();
*      list.Get(slot);
*      if (slot.Valid())
*	slot_set_and_print(obj, (int)slot,Am_X2,data->data.line.x2 + x_offset);
*
*      list.Next();
*      list.Get(slot);
*      if (slot.Valid())
*	slot_set_and_print(obj, (int)slot,Am_Y2,data->data.line.y2 + y_offset);
*    }
*    else { //not a line
*      list.Get(slot);
*      if (slot.Valid())
*	slot_set_and_print(obj, (int)slot, Am_LEFT,
*			   data->data.rect.left + x_offset);
*
*      list.Next();
*      list.Get(slot);
*      if (slot.Valid())
*	slot_set_and_print(obj, (int)slot, Am_TOP,
*			   data->data.rect.top + y_offset);
*
*      if (growing) {
*	list.Next();
*	list.Get(slot);
*	if (slot.Valid())
*	  slot_set_and_print(obj, (int)slot, Am_WIDTH, data->data.rect.width);
*
*	list.Next();
*	list.Get(slot);
*	if (slot.Valid())
*	  slot_set_and_print(obj, (int)slot, Am_HEIGHT,data->data.rect.height);
*      }
*    }
*  }
*}
*void Am_Modify_Object_Pos (Am_Object obj, Am_Inter_Location data,
*			   bool growing, Am_Value slots_to_set,
*			   Am_Object inter) {}
****************************** */

/* ** * OLD, now use data.Install()
*static void set_slots_from_location (Am_Object& obj, int a, int b, int c,
*				     int d, bool growing, bool as_line,
*				     int x_offset, int y_offset) {
*  if (as_line) {
*    Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING, " setting obj=" << obj
*		      << " X1=" << a + x_offset << " Y1=" << b + y_offset
*		      << " X2=" << c + x_offset << " Y2=" << d + y_offset);
*    obj.Set(Am_X1, a + x_offset); // , Am_NO_ANIMATION
*    obj.Set(Am_Y1, b + y_offset); // , Am_NO_ANIMATION
*    obj.Set(Am_X2, c + x_offset); // , Am_NO_ANIMATION
*    obj.Set(Am_Y2, d + y_offset); // , Am_NO_ANIMATION
*  }
*  else {
*    #ifdef DEBUG
*    if (Am_Inter_Tracing(Am_INTER_TRACE_SETTING)) {
*     std::cout << " setting obj=" << obj
*	   << " LEFT=" << a + x_offset << " TOP="  << b + y_offset;
*      if (growing)
*std::cout << " WIDTH=" << c << " HEIGHT=" << d;
*     std::cout <<std::endl <<std::flush;
*    }
*    #endif
*    obj.Set(Am_LEFT, a + x_offset); // , Am_NO_ANIMATION
*    obj.Set(Am_TOP, b + y_offset); // , Am_NO_ANIMATION
*    if (growing) {
*      obj.Set(Am_WIDTH, c); // , Am_NO_ANIMATION
*      obj.Set(Am_HEIGHT, d); // , Am_NO_ANIMATION
*    }
*  }
*}
* ************* */

void Am_Modify_Object_Pos (Am_Object& obj, const Am_Inter_Location& data,
			   bool growing) {
  data.Install(obj, growing);
}
/* OLD ********
*  Am_Object ref_obj, owner;
*  int x_offset = 0;
*  int y_offset = 0;
*  bool as_line;
*  int a, b, c, d;
*  data.Get_Location (as_line, ref_obj, a, b, c, d);
*  owner = obj.Get_Owner ();
*  if (ref_obj != owner) {
*    Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING,
*		      "Translating set coordinates from " << ref_obj <<
*		      " to " << owner);
*    Am_Translate_Coordinates (ref_obj, 0, 0, owner, x_offset, y_offset);
*  }
*  Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING,
*                    "++Object setting obj position=" << obj);
*  set_slots_from_location (obj, a, b, c, d, growing, as_line,
*			   x_offset, y_offset);
*}
* ***** */

void Am_Set_Feedback_As_Object_Line(Am_Object feedback,
				    Am_Object object_modified) {
  int x1, y1, x2, y2;
  Am_Object object_owner, feedback_owner;
  object_owner = object_modified.Get_Owner();
  feedback_owner = feedback.Get_Owner();
  x1 = object_modified.Get(Am_X1);
  y1 = object_modified.Get(Am_Y1);
  x2 = object_modified.Get(Am_X2);
  y2 = object_modified.Get(Am_Y2);
  Am_Translate_Coordinates (object_owner, x1, y1, feedback_owner, x1, y1);
  Am_Translate_Coordinates (object_owner, x2, y2, feedback_owner, x2, y2);
  feedback.Set(Am_X1, x1);
  feedback.Set(Am_Y1, y1);
  feedback.Set(Am_X2, x2);
  feedback.Set(Am_Y2, y2);
}

//////////////////////////////////////////////////////////////////////////////
// Multi-window stuff
//////////////////////////////////////////////////////////////////////////////

// Checks if the window that feedback is in matches the
// window in the interactor, and if not, moves the feedback object to the
// corresponding owner in Am_MULTI_FEEDBACK_OWNERS or Am_MULTI_OWNERS.
// returns the owner if change something
Am_Object Am_Check_And_Fix_Feedback_Group (Am_Object& feedback,
					   const Am_Object& inter) {
  if (feedback.Get_Owner().Is_Instance_Of(Am_Screen))
    return Am_No_Object;

  Am_Value v;
  Am_Value_List fb_owner_list, owner_list;
  v=inter.Peek(Am_MULTI_FEEDBACK_OWNERS);
  Am_Object new_owner;
  if (!v.Valid()) {
    Am_Check_And_Fix_Object_Group (feedback, inter, new_owner);
    return new_owner;
  }
  if (Am_Value_List::Test (v))
    fb_owner_list = v;
  else
    Am_ERRORO ("Am_MULTI_FEEDBACK_OWNERS slot of " << inter
	      << " must be list of objects but is " << v,
	       inter, Am_MULTI_FEEDBACK_OWNERS);

  v=inter.Peek(Am_MULTI_OWNERS);
  if (v.Valid ()) {
    if (Am_Value_List::Test (v))
      owner_list = v;
    else  //nothing to do if Am_MULTI_OWNERS is true/false
      return Am_No_Object;
  }
  else
    return Am_No_Object; //nothing to do if Am_MULTI_OWNERS not set

  Am_Object fb_owner, owner;
  Am_Object curr_fb_owner, curr_owner, temp;

  int x = inter.Get(Am_INTERIM_X);
  int y = inter.Get(Am_INTERIM_Y);
  Am_Object event_window = inter.Get (Am_WINDOW);

  for (fb_owner_list.Start (), owner_list.Start ();
       !fb_owner_list.Last (); fb_owner_list.Next (), owner_list.Next ()) {
    curr_fb_owner = fb_owner_list.Get ();
    if (curr_fb_owner.Is_Part_Of (event_window) &&
	Am_Point_In_All_Owners (curr_fb_owner, x, y, event_window)) {
      temp = Am_Point_In_Obj (curr_fb_owner, x, y, event_window);
      if (temp.Valid ()) {
        curr_owner = owner_list.Get ();
        if (!fb_owner.Valid () || curr_owner.Is_Part_Of (owner)) {
          fb_owner = curr_fb_owner;
          owner = curr_owner;
	}
      }
    }
  }

  #ifdef DEBUG
  if (fb_owner.Valid()) {
    Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING, "Inter " << inter
			 << " moving feedback object " << feedback <<
			 " to new owner " << fb_owner <<
			 " because mouse moved from prev owner " <<
			 feedback.Get_Owner () << " to " << fb_owner);
  }
  else {
    Am_INTER_TRACE_PRINT(inter, "Inter " << inter
			 << " moved object over window " << event_window
			 << " but ignored because not in MULTI_* list");
  }
  #endif

  if (fb_owner.Valid () && fb_owner != feedback.Get_Owner ()) {
    feedback.Remove_From_Owner(); // make the object no longer be a part of
                                  // its old place
    fb_owner.Add_Part(feedback);
  }
  return fb_owner;
}

// Checks if the window that object is in matches the
// window in interactor, and if not, moves
// the object to the corresponding owner in Am_MULTI_OWNERS.
// Returns true if everything is OK.  Returns false if should abort.
// Sets new_owner if changes the owner, otherwise new_owner is Am_No_Object
bool Am_Check_And_Fix_Object_Group (Am_Object& obj, const Am_Object &inter,
				    Am_Object &new_owner) {
  new_owner = Am_No_Object;
  // if we are moving a top-level window, don't need to do anything
  if (obj.Get_Owner ().Is_Instance_Of (Am_Screen)) return true;

  Am_Value_List owner_list;
  Am_Value v;
  v=inter.Peek(Am_MULTI_OWNERS);
  if (Am_Value_List::Test (v))
    owner_list = v;
  else
    return true; //otherwise, leave in original window

  Am_Object owner, old_owner;
  int x = inter.Get(Am_INTERIM_X);
  int y = inter.Get(Am_INTERIM_Y);
  Am_Object event_window = inter.Get (Am_WINDOW);

  Am_Object curr_owner, temp;
  for (owner_list.Start (); !owner_list.Last (); owner_list.Next ()) {
    curr_owner = owner_list.Get ();
    if (curr_owner.Is_Part_Of (event_window) &&
	Am_Point_In_All_Owners(curr_owner, x, y, event_window)) {
      temp = Am_Point_In_Obj (curr_owner, x, y, event_window);
      if (temp.Valid ()) {
        if (!owner.Valid () || curr_owner.Is_Part_Of (owner))
          owner = curr_owner;
      }
    }
  }
  if (!owner.Valid ())
    return false;
  new_owner = owner;
  old_owner = obj.Get_Owner ();
  if (owner != old_owner) {
    if (owner.Is_Part_Of (obj))
      return false;
    Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING, "Inter " << inter
			 << " moving object " << obj <<
			 " to new owner " << owner <<
			 " because mouse moved from prev owner " <<
			 old_owner);
    obj.Remove_From_Owner(); // make the object no longer be a part of its old
                             // place
    owner.Add_Part(obj);
  }
  return true;
}

// Like Check_And_Fix_Object_Group this procedure determines what group the
// given object belongs to.  This returns the group instead of automatically
// setting it.  Returns Am_No_Object instead of returning false.
Am_Object Am_Find_Destination_Group (const Am_Object& obj,
				     const Am_Object& inter)
{
  // if we are moving a top-level window, don't need to do anything
  Am_Object old_owner = obj.Get_Owner ();
  if (old_owner.Is_Instance_Of (Am_Screen))
    return old_owner;

  Am_Value_List owner_list;
  Am_Value v;
  v=inter.Peek(Am_MULTI_OWNERS);
  if (v.Valid()) {
    if (Am_Value_List::Test (v))
      owner_list = v;
    else
      return old_owner; // otherwise, leave in original group
  }
  else
    return old_owner;

  Am_Object owner;
  int x = inter.Get (Am_INTERIM_X);
  int y = inter.Get (Am_INTERIM_Y);
  Am_Object event_window = inter.Get (Am_WINDOW);

  Am_Object curr_owner, temp;
  for (owner_list.Start (); !owner_list.Last (); owner_list.Next ()) {
    curr_owner = owner_list.Get ();
    if (curr_owner.Is_Part_Of (event_window) &&
	Am_Point_In_All_Owners (curr_owner, x, y, event_window)) {
      temp = Am_Point_In_Obj (curr_owner, x, y, event_window);
      if (temp.Valid ()) {
        if (!owner.Valid () || curr_owner.Is_Part_Of (owner))
          owner = curr_owner;
      }
    }
  }
  if (owner.Valid () && (owner != old_owner) && owner.Is_Part_Of (obj))
    return Am_No_Object;
  return owner;
}

Am_Object Am_Find_Window_At_Cursor(Am_Object some_window) {
  Am_Drawonable* draw =  //doesn't matter which one
    Am_Drawonable::Narrow (some_window.Get(Am_DRAWONABLE));
  //first, run demons and flush queued events to make sure feedback
  //window has actually disappeared.
  Am_Update_All (); // update all windows
  draw->Flush_Output();
  Am_Drawonable* new_draw = draw->Get_Drawonable_At_Cursor();
  // std::cout << "found new drawonable " << new_draw <<std::endl <<std::flush;
  if (new_draw) {
    Am_Wrapper* window_data = (Am_Wrapper*)new_draw->Get_Data_Store ();
    if (window_data) {
      window_data->Note_Reference ();
      Am_Object window = (Am_Object_Data*)window_data;
      // std::cout << "which is window  " << window <<std::endl <<std::flush;
      return window;
      }
  }
  // if get here, can't find new window
  // std::cout << "Found no window at cursor\n" <<std::flush;
  return Am_No_Object;
}

//Use when Feedback is a window.  Make sure window in inter isn't feedback,
//and if so, find a new window and return it (also fixes the window
//and interim_x and _y in the inter).  Call this AFTER feedback
//has disappeared.  Otherwise, just returns inter_window.
Am_Object Am_Check_And_Fix_Point_In_Feedback_Window(Am_Object &inter,
						    Am_Object &feedback) {
  Am_Object inter_window = inter.Get(Am_WINDOW);
  // std::cout << "checking if inter window " << inter_window
  //     << " is feedback " << feedback <<std::endl <<std::flush;
  if (inter_window == feedback) {
    // std::cout << "Fixing to be not feedback\n" <<std::flush;
    Am_Object new_window = Am_Find_Window_At_Cursor(inter_window);
    if (new_window.Valid()) {
      int x = inter.Get(Am_INTERIM_X);
      int y = inter.Get(Am_INTERIM_Y);
      int newx, newy;
      Am_INTER_TRACE_PRINT(inter, "Translating coordinates of " << inter
			   << " from " << inter_window <<
			   " to " << new_window);
      Am_Translate_Coordinates (inter_window, x, y, new_window, newx, newy);
      inter.Set(Am_WINDOW, new_window);
      inter.Set(Am_INTERIM_X, newx);
      inter.Set(Am_INTERIM_Y, newy);
    }
    return new_window;
  }
  // else not feedback window, so just return original
  return inter_window;
}

//returns true if everything OK.  Returns false if multi-window and
//stopped in a window that isn't in the interactor's list or over no windows.
bool Am_Check_Final_Point_In_Multi(Am_Object inter) {
  Am_Value v;
  Am_Value_List owner_list;
  v=inter.Peek(Am_MULTI_OWNERS);
  if (v.Valid()) {
    if (Am_Value_List::Test(v)) owner_list = v;
    else return true; //MULTI_OWNER = true, so just return true
    }
  else return true; //not a multi-window
  Am_Object final_window = Am_Find_Window_At_Cursor(inter.Get(Am_WINDOW));
  if (!final_window.Valid())
    return false;  //no window under cursor
  else {
    Am_Object owner, win;
    for (owner_list.Start(); !owner_list.Last(); owner_list.Next()) {
      owner = owner_list.Get();
      win = owner.Get(Am_WINDOW);
      if (win == final_window)
	return true;
    }
    return false;
  }
}

// void Am_Temporary_Turn_Off_Animator(Am_Object &obj) {
//   if (obj.Valid()) {
//     Am_Object animator = Am_Get_Animator (obj, Am_LEFT);
//     if (animator.Valid()) {
//       animator.Set(Am_SAVED_OLD_ANIMATED_ACTIVE,
// 		   (bool)animator.Get(Am_ACTIVE));
//      std::cout << "Setting ACTIVE of " << animator << " of " << obj
// 	   << " to false\n" <<std::flush;
//       animator.Set(Am_ACTIVE, false);
//     }
//   }
// }
// void Am_Temporary_Restore_Animator(Am_Object &obj) {
//   if (obj.Valid()) {
//     Am_Object animator = Am_Get_Animator(obj, Am_LEFT);
//     if (animator.Valid()) {
//       bool active = animator.Get(Am_SAVED_OLD_ANIMATED_ACTIVE);
//      std::cout << "Restoring ACTIVE of " << animator << " of " << obj
// 	   << " to " << active <<std::endl <<std::flush;
//       animator.Set(Am_ACTIVE, active);
//     }
//   }
// }

//////////////////////////////////////////////////////////////////////////////
// Do Methods for Interactor
//////////////////////////////////////////////////////////////////////////////

Am_Define_Method(Am_Current_Location_Method, void, Am_Move_Grow_Start_Do,
		 (Am_Object inter, Am_Object object_modified,
		  Am_Inter_Location data)) {

  Am_Object feedback;
  feedback = inter.Get(Am_FEEDBACK_OBJECT);
  if (feedback.Valid ()) {
    //test for a frequent bug:
    #ifdef DEBUG
    if (!feedback.Get_Owner().Valid() ||
	!feedback.Get(Am_WINDOW).Valid()) {
     std::cout << "** AMULET WARNING: feedback object " << feedback
	   << " in interactor " << inter
	   << " does not seem to be in a group or window,"
	   << " so it won't be visible\n" <<std::flush;
    }
    #endif
    bool growing = inter.Get(Am_GROWING);
    bool as_line = (bool)inter.Get(Am_AS_LINE) &&
      		   (bool)feedback.Get(Am_AS_LINE);

    //Am_Temporary_Turn_Off_Animator(feedback);

    bool feedback_is_window = feedback.Is_Instance_Of(Am_Window);
    Am_Check_And_Fix_Feedback_Group(feedback, inter);
    if (feedback_is_window)
      Am_Add_Remove_Inter_To_Extra_Window(inter, feedback, true, true);
    Am_REPORT_VIS_CONDITION(Am_INTER_TRACE_SETTING, inter, feedback, true);
    feedback.Set(Am_VISIBLE, true);
    Am_To_Top(feedback);
    //if a line and have feedback and !growing, initialize feedback's points
    // since interim_do when moving just sets left and top which will leave the
    // line pointing the wrong way
    if (!feedback_is_window) {//don't change size of feedback as a window
      if (as_line && !growing) {
	Am_Set_Feedback_As_Object_Line(feedback, object_modified);
      }
      else {
	//initialize size of the feedback object (if not growing, then
	//interim_do does not set the size), so use true for growing here.
	Am_Modify_Object_Pos (feedback, data, true);
      }
    }
  }
  //Am_Temporary_Turn_Off_Animator(object_modified);

}

//Am_Inter_Location value should be already set into Am_INTERIM_VALUE.
//Modifies either the Am_FEEDBACK_OBJECT or the Am_START_OBJECT
Am_Define_Method(Am_Current_Location_Method, void, Am_Move_Grow_Interim_Do,
		 (Am_Object inter, Am_Object object_modified,
		  Am_Inter_Location data)) {
  bool growing = inter.Get (Am_GROWING);
  Am_Object obj;
  obj = inter.Get(Am_FEEDBACK_OBJECT);
  if (obj.Valid ()) { //don't switch windows if growing
    if (!growing) Am_Check_And_Fix_Feedback_Group(obj, inter);
  }
  else { // modify the real object
    obj = object_modified;
    if (!growing && obj.Valid()) {
      Am_Object new_owner; //not used
      Am_Check_And_Fix_Object_Group (obj, inter, new_owner);
    }
  }
  if (obj.Valid ()) {
    Am_Modify_Object_Pos (obj, data, growing);
  }
}

//data will be orig_points for the object
Am_Define_Method(Am_Current_Location_Method, void, Am_Move_Grow_Abort_Do,
		 (Am_Object inter, Am_Object object_modified,
		  Am_Inter_Location data)) {
  Am_Object obj;
  obj = inter.Get(Am_FEEDBACK_OBJECT);
  if (obj.Valid ()) {
    Am_REPORT_VIS_CONDITION(Am_INTER_TRACE_SETTING, inter, obj, false);
    //Am_Temporary_Restore_Animator(obj);
    if (obj.Is_Instance_Of(Am_Window))
      Am_Add_Remove_Inter_To_Extra_Window(inter, obj, false, false);
    obj.Set(Am_VISIBLE, false);
  }
  else {
    obj = object_modified;
    if (obj.Valid ()) {
      Am_Check_And_Fix_Owner_For_Object(obj, data); //fix obj to data
      bool growing = inter.Get(Am_GROWING);
      Am_Modify_Object_Pos (obj, data, growing);
    }
  }
  //Am_Temporary_Restore_Animator(object_modified);

}

Am_Define_Method(Am_Current_Location_Method, void, Am_Move_Grow_Do,
		 (Am_Object inter, Am_Object object_modified,
		  Am_Inter_Location data)) {

  Am_Object feedback;
  feedback = inter.Get(Am_FEEDBACK_OBJECT);
  bool growing = inter.Get(Am_GROWING);
  bool beep_on_abort = inter.Get(Am_INTER_BEEP_ON_ABORT);
  if (growing && beep_on_abort && data.Is_Zero_Size()) {
    Am_INTER_TRACE_PRINT(inter, "Aborting " << inter << " because zero size");
    Am_Beep();
    Am_Abort_Interactor(inter);
    return;
  }
  if (feedback.Valid ()) {
    Am_REPORT_VIS_CONDITION(Am_INTER_TRACE_SETTING, inter, feedback, false);
    feedback.Set(Am_VISIBLE, false);
    //Am_Temporary_Restore_Animator(feedback);
    if (feedback.Is_Instance_Of(Am_Window)) {
      Am_Add_Remove_Inter_To_Extra_Window(inter, feedback, false, false);
      //special test to see if need to figure out new window for the object
      Am_Object inter_window =
	Am_Check_And_Fix_Point_In_Feedback_Window(inter, feedback);
      //std::cout << "new window is " << inter_window <<std::endl <<std::flush;
      if (!inter_window.Valid()) { //release outside all windows
	Am_INTER_TRACE_PRINT(inter, "Aborting " << inter <<
			     " because stopped outside all windows");
	if (beep_on_abort) Am_Beep();
	Am_Abort_Interactor(inter);
	return;
      }
    }
  }
  //if multi-window, abort if final point is outside of the windows
  // ** Is this always what is desired? **
  if (!Am_Check_Final_Point_In_Multi(inter)) {
    Am_INTER_TRACE_PRINT(inter, "Aborting " << inter <<
		       " because stopped outside windows in Am_MULTI_OWNERS");
    if ((bool)inter.Get(Am_INTER_BEEP_ON_ABORT))
      Am_Beep();
    Am_Abort_Interactor(inter);
    return;
  }
  //std::cout << "...object modified " << object_modified <<std::endl <<std::flush;
  if (object_modified.Valid ()) {
    if (!growing) {
      //see if need to move the real object
      Am_Object new_owner; //not used
      if (!Am_Check_And_Fix_Object_Group (object_modified, inter, new_owner)) {
	return; // return on error
      }
      //make sure the data has the right owner, in case changed windows
      if (!data.Translate_To(object_modified.Get_Owner()))
	Am_ERRORO("Can't translate data to new owner of " << object_modified,
		 inter, Am_START_OBJECT);
    }
    Am_Modify_Object_Pos (object_modified, data, growing);
    //move first, then restore
    //Am_Temporary_Restore_Animator(object_modified);
  }

  //set VALUE of inter with final location
  Am_Copy_Data_Into_Slot(inter, Am_VALUE, data);
  inter.Set(Am_OBJECT_MODIFIED, object_modified);
  Am_Copy_Values_To_Command(inter);
}

//////////////////////////////////////////////////////////////////////////////
// Undo and Redo methods for Command
//////////////////////////////////////////////////////////////////////////////

//makes sure owner of obj is data.ref_obj and if not, moves it
bool Am_Check_And_Fix_Owner_For_Object(Am_Object &obj,
				       Am_Inter_Location &data) {
  Am_Object owner = obj.Get_Owner();
  Am_Object new_owner = data.Get_Ref_Obj();
  if (owner != new_owner) {
    Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING, "Moving object " << obj <<
			 " to new owner " << new_owner << " from "  << owner);
    obj.Remove_From_Owner(); //make the object no longer be a part of its old
			     //place
    new_owner.Add_Part(obj);
    return true;
  }
  return false;
}

void Am_Set_Data_From_Object(Am_Object obj, Am_Inter_Location data,
			     bool growing) {
  Am_Object ref_obj;
  bool as_line;
  Am_Value value;
  ref_obj = obj.Get_Owner();
  value=obj.Peek(Am_AS_LINE);
  if (!value.Valid()) as_line = false;
  else data.Get_As_Line(as_line);
  if (as_line) {
    data.Set_Location(true, ref_obj, (int)obj.Get(Am_X1), (int)obj.Get(Am_Y1),
		      (int)obj.Get(Am_X2), (int)obj.Get(Am_Y2), false);
  }
  else { // not a line growing (lines move like non-lines)
    data.Set_Location(false, ref_obj, (int)obj.Get(Am_LEFT),
		      (int)obj.Get(Am_TOP),
		      (int)obj.Get(Am_WIDTH), (int)obj.Get(Am_HEIGHT), false);
  }
  data.Set_Growing(growing);
}

void move_grow_general_undo_redo(Am_Object command_obj, bool undo,
				 bool selective, bool reload_data,
				 Am_Object obj = Am_No_Object,
				 Am_Value new_val = Am_No_Value) {
  Am_Object inter;
  inter = command_obj.Get(Am_SAVED_OLD_OWNER);
  bool growing = command_obj.Get(Am_GROWING);
  Am_Inter_Location new_loc;

  if (reload_data && new_val.Valid()) {
    if (!Am_Inter_Location::Test(new_val))
      Am_ERRORO("New value for selective repeat of " << command_obj
		<< " is not Am_Inter_Location: " << new_val,
		command_obj, Am_VALUE);
    new_loc = new_val;
    bool beep_on_abort = inter.Get(Am_INTER_BEEP_ON_ABORT);
    if (growing && beep_on_abort && new_loc.Is_Zero_Size()) {
      Am_INTER_TRACE_PRINT(inter, "Aborting " << command_obj
			     << " because zero size");
      //not queued for undo
      command_obj.Set(Am_COMMAND_IS_ABORTING, true, Am_OK_IF_NOT_THERE);
      Am_Beep();
      return;
    }
  }

  if (reload_data && obj.Valid()) command_obj.Set(Am_OBJECT_MODIFIED, obj);
  else obj = command_obj.Get(Am_OBJECT_MODIFIED);

  #ifdef DEBUG
  if (inter.Valid () && Am_Inter_Tracing(inter)) {
    if (selective)std::cout << "Selective ";
    if (undo)std::cout << "Undo"; else std::cout << "Redo";
   std::cout << " command " << command_obj << " on obj " << obj <<std::endl <<std::flush;
  }
  #endif
  if (obj.Valid ()) {
    Am_Inter_Location old_data, new_data;
    command_obj.Make_Unique (Am_OLD_VALUE); //will be destructively modifying
    command_obj.Make_Unique (Am_VALUE);     //these slots; make sure
					    //not shared
    old_data = command_obj.Get(Am_OLD_VALUE);
    if (reload_data && new_loc.Valid()) {
      new_data = new_loc;
      command_obj.Set(Am_VALUE, new_data);
    }
    else new_data = command_obj.Get(Am_VALUE);


    if (selective) {
      if (undo) Am_Set_Data_From_Object(obj, new_data, growing);
      else      Am_Set_Data_From_Object(obj, old_data, growing);
    }
    if (undo) {
      Am_Check_And_Fix_Owner_For_Object(obj, old_data);
  #ifdef DEBUG
      if (growing != old_data.Get_Growing())
	Am_ERRORO("Growing in cmd " << command_obj
		  << " != old_data growing " << old_data,
		  command_obj, Am_GROWING);
  #endif
      Am_Modify_Object_Pos (obj, old_data, growing);
      // swap current and old values, in case undo or undo-the-undo again
      new_data.Swap_With(old_data, false);
      command_obj.Note_Changed(Am_OLD_VALUE);
      command_obj.Note_Changed(Am_VALUE);
    }
    else {
      Am_Check_And_Fix_Owner_For_Object(obj, new_data);
  #ifdef DEBUG
      if (growing != new_data.Get_Growing())
	Am_ERRORO("Growing in cmd " << command_obj
		  << " != new_data growing " << new_data,
		  command_obj, Am_GROWING);
  #endif
      Am_Modify_Object_Pos (obj, new_data, growing);
      if (selective) {
	command_obj.Note_Changed(Am_OLD_VALUE);
	command_obj.Note_Changed(Am_VALUE);
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
// Methods for the Internal Command object
//////////////////////////////////////////////////////////////////////////////

Am_Define_Method(Am_Object_Method, void, Am_Move_Grow_Command_Undo,
		 (Am_Object command_obj)) {
  move_grow_general_undo_redo(command_obj, true, false, false);
}

//Am_Move_Grow_Command_Redo same as Am_Move_Grow_Command_Undo

//command obj is a copy of the original, so can modify it
Am_Define_Method(Am_Object_Method, void,
		 Am_Move_Grow_Command_Selective_Undo, (Am_Object command_obj)){
  //assume object has moved, so no longer where the command object put it, so
  //put it there again, and save current place in case undone.
  move_grow_general_undo_redo(command_obj, true, true, false);
}

//command obj is a copy of the original, so can modify it
Am_Define_Method(Am_Object_Method, void,
		 Am_Move_Grow_Command_Repeat_Same, (Am_Object command_obj)){
  //assume object has moved, so no longer where the command object put it, so
  //put it there again, and save current place in case undone.
  move_grow_general_undo_redo(command_obj, false, true, false);
}

//command obj is a copy of the original, so can modify it
Am_Define_Method(Am_Selective_Repeat_New_Method, void,
		 Am_Move_Grow_Command_Repeat_New,
		 (Am_Object command_obj, Am_Value new_selection,
		  Am_Value new_value)){
  Am_Object new_obj;
  if (new_selection.Valid()) {
    if (new_selection.type == Am_OBJECT) new_obj = new_selection;
    else Am_ERRORO("Am_Move_Grow_Command_Repeat_New called with non-object "
		   << new_selection, command_obj, 0);
  }
  move_grow_general_undo_redo(command_obj, false, true, true, new_obj,
			      new_value);
}

// Used in move_grow interactors to make the default for Am_AS_LINE be
// if the object being moved is a line type object.
Am_Define_Formula (bool, start_obj_as_line_formula)
{
  Am_Object start_obj;
  start_obj = self.Get(Am_START_OBJECT); // set when running starts
  if(start_obj) {
  // assume objects don't change whether they are line-like or not so
  // use Get instead of GV
    Am_Value value;
    value=start_obj.Peek(Am_AS_LINE, Am_NO_DEPENDENCY);
    if (value.type == Am_INT)
      return (int)value;
    else if (value.type == Am_BOOL)
      return value;
    else return false;
  }
  else return false;
}

////////////////////////////////////////////////////////////
//global variables

Am_Object Am_Move_Grow_Internal_Command;
Am_Object Am_Move_Grow_Interactor;  // moving and growing with mouse

static void init() {

  Am_Move_Grow_Internal_Command =
    Am_Command.Create(DSTR("Am_Move_Grow_Interal_Command"))
    .Set (Am_LABEL, "Move_Grow")
    .Set (Am_UNDO_METHOD, Am_Move_Grow_Command_Undo)
    .Set (Am_REDO_METHOD, Am_Move_Grow_Command_Undo) //use for both
    .Set (Am_SELECTIVE_UNDO_METHOD, Am_Move_Grow_Command_Selective_Undo)
    .Set (Am_SELECTIVE_REPEAT_SAME_METHOD,
	  Am_Move_Grow_Command_Repeat_Same)
    .Set (Am_SELECTIVE_REPEAT_ON_NEW_METHOD,
	  Am_Move_Grow_Command_Repeat_New)
    .Add (Am_GROWING, 0)
    ;

  Am_Move_Grow_Interactor = Am_Interactor.Create(DSTR("Am_Move_Grow_Interactor"))
     .Set (Am_INTER_START_METHOD, Am_Move_Grow_Start_Method)
     .Set (Am_INTER_ABORT_METHOD, Am_Move_Grow_Abort_Method)
     .Set (Am_INTER_OUTSIDE_METHOD, Am_Move_Grow_Outside_Method)
     // default outside stop method is fine
     .Set (Am_INTER_BACK_INSIDE_METHOD, Am_Move_Grow_Back_Inside_Method)
     .Set (Am_INTER_RUNNING_METHOD, Am_Move_Grow_Running_Method)
     .Set (Am_INTER_STOP_METHOD, Am_Move_Grow_Stop_Method)

     .Set (Am_START_DO_METHOD, Am_Move_Grow_Start_Do)
     .Set (Am_INTERIM_DO_METHOD, Am_Move_Grow_Interim_Do)
     .Set (Am_ABORT_DO_METHOD, Am_Move_Grow_Abort_Do)
     .Set (Am_DO_METHOD, Am_Move_Grow_Do)

     .Add (Am_GRID_METHOD, 0)
     .Add (Am_GRID_X, 0)
     .Add (Am_GRID_Y, 0)
     .Add (Am_GRID_ORIGIN_X, 0)
     .Add (Am_GRID_ORIGIN_Y, 0)
     .Add (Am_X_OFFSET, 0) // offsets for calculation "where-hit"
     .Add (Am_Y_OFFSET, 0)
     .Add (Am_WHERE_ATTACH, Am_ATTACH_WHERE_HIT)
     .Add (Am_MINIMUM_WIDTH, 0)
     .Add (Am_MINIMUM_HEIGHT, 0)
     .Add (Am_MINIMUM_LENGTH, 0)
     .Add (Am_GROWING, 0)
     .Add (Am_AS_LINE, start_obj_as_line_formula)
     .Add (Am_FEEDBACK_OBJECT, 0)
     .Add_Part(Am_IMPLEMENTATION_COMMAND,
	   Am_Move_Grow_Internal_Command.Create(DSTR("Move_Grow_Internal_Command")))
     ;
  Am_Move_Grow_Interactor.Get_Object(Am_COMMAND)
    .Set (Am_LABEL, "Move_Grow")
    .Set_Name (DSTR("Am_Command_in_Move_Grow"))
    ;
}

static Am_Initializer* initializer = new Am_Initializer(DSTR("Inter_Move_Grow"), init, 3.3f);
