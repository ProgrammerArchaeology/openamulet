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

#include <amulet.h>
#include <amulet/scripting_advanced.h>

Am_Slot_Key Am_CHANGE_LEFT = Am_Register_Slot_Name("Am_CHANGE_LEFT");
Am_Slot_Key Am_LEFT_HOW = Am_Register_Slot_Name("Am_LEFT_HOW");
Am_Slot_Key Am_LEFT_AMOUNT_WIDGET =
    Am_Register_Slot_Name("Am_LEFT_AMOUNT_WIDGET");
Am_Slot_Key Am_CHANGE_TOP = Am_Register_Slot_Name("Am_CHANGE_TOP");
Am_Slot_Key Am_TOP_HOW = Am_Register_Slot_Name("Am_TOP_HOW");
Am_Slot_Key Am_TOP_AMOUNT_WIDGET =
    Am_Register_Slot_Name("Am_TOP_AMOUNT_WIDGET");
Am_Slot_Key Am_CHANGE_WIDTH = Am_Register_Slot_Name("Am_CHANGE_WIDTH");
Am_Slot_Key Am_WIDTH_HOW = Am_Register_Slot_Name("Am_WIDTH_HOW");
Am_Slot_Key Am_WIDTH_AMOUNT_WIDGET =
    Am_Register_Slot_Name("Am_WIDTH_AMOUNT_WIDGET");
Am_Slot_Key Am_CHANGE_HEIGHT = Am_Register_Slot_Name("Am_CHANGE_HEIGHT");
Am_Slot_Key Am_HEIGHT_HOW = Am_Register_Slot_Name("Am_HEIGHT_HOW");
Am_Slot_Key Am_HEIGHT_AMOUNT_WIDGET =
    Am_Register_Slot_Name("Am_HEIGHT_AMOUNT_WIDGET");

#define REF_OBJ_LEFT 17
#define REF_OBJ_TOP 210
#define REF_OBJ_SIZE 54
#define SHOW_LOC_OBJ_SIZE 10
#define SHOW_LOC_OBJ_OFFSET 3
#define SHOW_LOC_CENTER_OFFSET ((REF_OBJ_SIZE - SHOW_LOC_OBJ_SIZE) / 2)
Am_Slot_Key Am_SHOW_LOC_OBJ = Am_Register_Slot_Name("Am_SHOW_LOC_OBJ");
Am_Slot_Key Am_REF_OBJ = Am_Register_Slot_Name("Am_REF_OBJ");

Am_Object am_locgen;

Am_Object bottomin, bottomout, centerX, centerY, leftin, leftout, rightin,
    rightout, topin, topout, heightdiff, heightpercent, widthdiff, widthpercent;

void
load_loc_bitmap(Am_Object &obj, const char *name)
{
  const char *pathname = Am_Merge_Pathname(name);
  Am_Image_Array icon = Am_Image_Array(pathname);
  delete[] pathname;
  obj = Am_Bitmap.Create(name).Set(Am_IMAGE, icon).Set(Am_LINE_STYLE, Am_Black);
}

void
locgen_load_bitmaps()
{
#if ((defined _WIN32) || (defined _MACINTOSH))
  load_loc_bitmap(bottomin, "lib/images/scripting/bottomin.gif");
  load_loc_bitmap(bottomout, "lib/images/scripting/bottomout.gif");
  load_loc_bitmap(centerX, "lib/images/scripting/centerX.gif");
  load_loc_bitmap(centerY, "lib/images/scripting/centerY.gif");
  load_loc_bitmap(leftin, "lib/images/scripting/leftin.gif");
  load_loc_bitmap(leftout, "lib/images/scripting/leftout.gif");
  load_loc_bitmap(rightin, "lib/images/scripting/rightin.gif");
  load_loc_bitmap(rightout, "lib/images/scripting/rightout.gif");
  load_loc_bitmap(topin, "lib/images/scripting/topin.gif");
  load_loc_bitmap(topout, "lib/images/scripting/topout.gif");
  load_loc_bitmap(heightdiff, "lib/images/scripting/heightdiff.gif");
  load_loc_bitmap(heightpercent, "lib/images/scripting/heightpercent.gif");
  load_loc_bitmap(widthdiff, "lib/images/scripting/widthdiff.gif");
  load_loc_bitmap(widthpercent, "lib/images/scripting/widthpercent.gif");
#else
  load_loc_bitmap(bottomin, "images/scripting/bottomin.xbm");
  load_loc_bitmap(bottomout, "images/scripting/bottomout.xbm");
  load_loc_bitmap(centerX, "images/scripting/centerX.xbm");
  load_loc_bitmap(centerY, "images/scripting/centerY.xbm");
  load_loc_bitmap(leftin, "images/scripting/leftin.xbm");
  load_loc_bitmap(leftout, "images/scripting/leftout.xbm");
  load_loc_bitmap(rightin, "images/scripting/rightin.xbm");
  load_loc_bitmap(rightout, "images/scripting/rightout.xbm");
  load_loc_bitmap(topin, "images/scripting/topin.xbm");
  load_loc_bitmap(topout, "images/scripting/topout.xbm");
  load_loc_bitmap(heightdiff, "images/scripting/heightdiff.xbm");
  load_loc_bitmap(heightpercent, "images/scripting/heightpercent.xbm");
  load_loc_bitmap(widthdiff, "images/scripting/widthdiff.xbm");
  load_loc_bitmap(widthpercent, "images/scripting/widthpercent.xbm");
#endif
}

am_loc_values::am_loc_values()
{
  left_control = am_change_equals;
  top_control = am_change_equals;
  width_control = am_change_equals;
  height_control = am_change_equals;

  left_equals = 0;
  left_offset = 0;
  left_object_offset = 0;
  left_centered = 50;
  top_equals = 0;
  top_offset = 0;
  top_object_offset = 0;
  top_centered = 50;
  width_equals = 0;
  width_offset = 0;
  width_object_offset = 0;
  width_object_percent = 100;
  height_equals = 0;
  height_offset = 0;
  height_object_offset = 0;
  height_object_percent = 100;

  growing = true;
}

void
set_ref_grey(Am_Object &win, bool grey)
{
  win.Get_Object(Am_SHOW_LOC_OBJ)
      .Set(Am_FILL_STYLE, (grey ? Am_Gray_Stipple : Am_Black));
  win.Get_Object(Am_REF_OBJ)
      .Set(Am_LINE_STYLE, (grey ? Am_Gray_Stipple : Am_Line_2));
}

Am_Define_Method(Am_Object_Method, void, set_left_equals, (Am_Object cmd))
{
  Am_Object win = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  am_loc_values *loc_values = (am_loc_values *)(Am_Ptr)win.Get(Am_VALUES);
  loc_values->left_control = win.Get_Object(Am_LEFT_HOW).Get(Am_VALUE);
  int val = loc_values->left_equals;
  win.Get_Object(Am_LEFT_AMOUNT_WIDGET).Set(Am_VALUE, val);
  set_ref_grey(win, true);
}
Am_Define_Method(Am_Object_Method, void, set_left_offset, (Am_Object cmd))
{
  Am_Object win = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  am_loc_values *loc_values = (am_loc_values *)(Am_Ptr)win.Get(Am_VALUES);
  loc_values->left_control = win.Get_Object(Am_LEFT_HOW).Get(Am_VALUE);
  int val = loc_values->left_offset;
  win.Get_Object(Am_LEFT_AMOUNT_WIDGET).Set(Am_VALUE, val);
  set_ref_grey(win, true);
}
Am_Define_Method(Am_Object_Method, void, set_left_object_offset,
                 (Am_Object cmd))
{
  Am_Object win = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  am_loc_values *loc_values = (am_loc_values *)(Am_Ptr)win.Get(Am_VALUES);
  loc_values->left_control = win.Get_Object(Am_LEFT_HOW).Get(Am_VALUE);
  int val = loc_values->left_object_offset;
  win.Get_Object(Am_LEFT_AMOUNT_WIDGET).Set(Am_VALUE, val);
  set_ref_grey(win, false);
  int x;
  switch ((int)cmd.Get(Am_VALUE)) {
  case am_change_offset_left_outside:
    x = REF_OBJ_LEFT - SHOW_LOC_OBJ_SIZE - 1;
    break;
  case am_change_offset_left_inside:
    x = REF_OBJ_LEFT + SHOW_LOC_OBJ_OFFSET;
    break;
  case am_change_offset_right_inside:
    x = REF_OBJ_LEFT + REF_OBJ_SIZE - SHOW_LOC_OBJ_SIZE - SHOW_LOC_OBJ_OFFSET;
    break;
  case am_change_offset_right_outside:
    x = REF_OBJ_LEFT + REF_OBJ_SIZE + 1;
    break;
  default:
    Am_ERROR("Bad left control " << (int)cmd.Get(Am_VALUE));
  }
  win.Get_Object(Am_SHOW_LOC_OBJ).Set(Am_LEFT, x);
}
Am_Define_Method(Am_Object_Method, void, set_left_centered, (Am_Object cmd))
{
  Am_Object win = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  am_loc_values *loc_values = (am_loc_values *)(Am_Ptr)win.Get(Am_VALUES);
  loc_values->left_control = win.Get_Object(Am_LEFT_HOW).Get(Am_VALUE);
  int val = loc_values->left_centered;
  win.Get_Object(Am_LEFT_AMOUNT_WIDGET).Set(Am_VALUE, val);
  set_ref_grey(win, false);
  win.Get_Object(Am_SHOW_LOC_OBJ)
      .Set(Am_LEFT, REF_OBJ_LEFT + SHOW_LOC_CENTER_OFFSET);
}
Am_Define_Method(Am_Object_Method, void, set_top_equals, (Am_Object cmd))
{
  Am_Object win = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  am_loc_values *loc_values = (am_loc_values *)(Am_Ptr)win.Get(Am_VALUES);
  loc_values->top_control = win.Get_Object(Am_TOP_HOW).Get(Am_VALUE);
  int val = loc_values->top_equals;
  win.Get_Object(Am_TOP_AMOUNT_WIDGET).Set(Am_VALUE, val);
  set_ref_grey(win, true);
}
Am_Define_Method(Am_Object_Method, void, set_top_offset, (Am_Object cmd))
{
  Am_Object win = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  am_loc_values *loc_values = (am_loc_values *)(Am_Ptr)win.Get(Am_VALUES);
  loc_values->top_control = win.Get_Object(Am_TOP_HOW).Get(Am_VALUE);
  int val = loc_values->top_offset;
  win.Get_Object(Am_TOP_AMOUNT_WIDGET).Set(Am_VALUE, val);
  set_ref_grey(win, true);
}
Am_Define_Method(Am_Object_Method, void, set_top_object_offset, (Am_Object cmd))
{
  Am_Object win = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  am_loc_values *loc_values = (am_loc_values *)(Am_Ptr)win.Get(Am_VALUES);
  loc_values->top_control = win.Get_Object(Am_TOP_HOW).Get(Am_VALUE);
  int val = loc_values->top_object_offset;
  win.Get_Object(Am_TOP_AMOUNT_WIDGET).Set(Am_VALUE, val);
  set_ref_grey(win, false);
  int y;
  switch ((int)cmd.Get(Am_VALUE)) {
  case am_change_offset_top_outside:
    y = REF_OBJ_TOP - SHOW_LOC_OBJ_SIZE - 1;
    break;
  case am_change_offset_top_inside:
    y = REF_OBJ_TOP + SHOW_LOC_OBJ_OFFSET;
    break;
  case am_change_offset_bottom_inside:
    y = REF_OBJ_TOP + REF_OBJ_SIZE - SHOW_LOC_OBJ_SIZE - SHOW_LOC_OBJ_OFFSET;
    break;
  case am_change_offset_bottom_outside:
    y = REF_OBJ_TOP + REF_OBJ_SIZE + 1;
    break;
  default:
    Am_ERROR("Bad top control " << (int)cmd.Get(Am_VALUE));
  }
  win.Get_Object(Am_SHOW_LOC_OBJ).Set(Am_TOP, y);
}
Am_Define_Method(Am_Object_Method, void, set_top_centered, (Am_Object cmd))
{
  Am_Object win = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  am_loc_values *loc_values = (am_loc_values *)(Am_Ptr)win.Get(Am_VALUES);
  loc_values->top_control = win.Get_Object(Am_TOP_HOW).Get(Am_VALUE);
  int val = loc_values->top_centered;
  win.Get_Object(Am_TOP_AMOUNT_WIDGET).Set(Am_VALUE, val);
  set_ref_grey(win, false);
  win.Get_Object(Am_SHOW_LOC_OBJ)
      .Set(Am_TOP, REF_OBJ_TOP + SHOW_LOC_CENTER_OFFSET);
}
Am_Define_Method(Am_Object_Method, void, set_width_equals, (Am_Object cmd))
{
  Am_Object win = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  am_loc_values *loc_values = (am_loc_values *)(Am_Ptr)win.Get(Am_VALUES);
  loc_values->width_control = win.Get_Object(Am_WIDTH_HOW).Get(Am_VALUE);
  int val = loc_values->width_equals;
  win.Get_Object(Am_WIDTH_AMOUNT_WIDGET).Set(Am_VALUE, val);
}
Am_Define_Method(Am_Object_Method, void, set_width_offset, (Am_Object cmd))
{
  Am_Object win = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  am_loc_values *loc_values = (am_loc_values *)(Am_Ptr)win.Get(Am_VALUES);
  loc_values->width_control = win.Get_Object(Am_WIDTH_HOW).Get(Am_VALUE);
  int val = loc_values->width_offset;
  win.Get_Object(Am_WIDTH_AMOUNT_WIDGET).Set(Am_VALUE, val);
}
Am_Define_Method(Am_Object_Method, void, set_width_object_offset,
                 (Am_Object cmd))
{
  Am_Object win = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  am_loc_values *loc_values = (am_loc_values *)(Am_Ptr)win.Get(Am_VALUES);
  loc_values->width_control = win.Get_Object(Am_WIDTH_HOW).Get(Am_VALUE);
  int val = loc_values->width_object_offset;
  win.Get_Object(Am_WIDTH_AMOUNT_WIDGET).Set(Am_VALUE, val);
}
Am_Define_Method(Am_Object_Method, void, set_width_object_percent,
                 (Am_Object cmd))
{
  Am_Object win = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  am_loc_values *loc_values = (am_loc_values *)(Am_Ptr)win.Get(Am_VALUES);
  loc_values->width_control = win.Get_Object(Am_WIDTH_HOW).Get(Am_VALUE);
  int val = loc_values->width_object_percent;
  win.Get_Object(Am_WIDTH_AMOUNT_WIDGET).Set(Am_VALUE, val);
}
Am_Define_Method(Am_Object_Method, void, set_height_equals, (Am_Object cmd))
{
  Am_Object win = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  am_loc_values *loc_values = (am_loc_values *)(Am_Ptr)win.Get(Am_VALUES);
  loc_values->height_control = win.Get_Object(Am_HEIGHT_HOW).Get(Am_VALUE);
  int val = loc_values->height_equals;
  win.Get_Object(Am_HEIGHT_AMOUNT_WIDGET).Set(Am_VALUE, val);
}
Am_Define_Method(Am_Object_Method, void, set_height_offset, (Am_Object cmd))
{
  Am_Object win = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  am_loc_values *loc_values = (am_loc_values *)(Am_Ptr)win.Get(Am_VALUES);
  loc_values->height_control = win.Get_Object(Am_HEIGHT_HOW).Get(Am_VALUE);
  int val = loc_values->height_offset;
  win.Get_Object(Am_HEIGHT_AMOUNT_WIDGET).Set(Am_VALUE, val);
}
Am_Define_Method(Am_Object_Method, void, set_height_object_offset,
                 (Am_Object cmd))
{
  Am_Object win = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  am_loc_values *loc_values = (am_loc_values *)(Am_Ptr)win.Get(Am_VALUES);
  loc_values->height_control = win.Get_Object(Am_HEIGHT_HOW).Get(Am_VALUE);
  int val = loc_values->height_object_offset;
  win.Get_Object(Am_HEIGHT_AMOUNT_WIDGET).Set(Am_VALUE, val);
}
Am_Define_Method(Am_Object_Method, void, set_height_object_percent,
                 (Am_Object cmd))
{
  Am_Object win = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  am_loc_values *loc_values = (am_loc_values *)(Am_Ptr)win.Get(Am_VALUES);
  loc_values->height_control = win.Get_Object(Am_HEIGHT_HOW).Get(Am_VALUE);
  int val = loc_values->height_object_percent;
  win.Get_Object(Am_HEIGHT_AMOUNT_WIDGET).Set(Am_VALUE, val);
}
Am_Define_Method(Am_Object_Method, void, set_left_zero, (Am_Object cmd))
{
  Am_Object win = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  am_loc_values *loc_values = (am_loc_values *)(Am_Ptr)win.Get(Am_VALUES);
  loc_values->left_control = win.Get_Object(Am_LEFT_HOW).Get(Am_VALUE);
  win.Get_Object(Am_LEFT_AMOUNT_WIDGET).Set(Am_VALUE, 0);
}
Am_Define_Method(Am_Object_Method, void, set_top_zero, (Am_Object cmd))
{
  Am_Object win = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  am_loc_values *loc_values = (am_loc_values *)(Am_Ptr)win.Get(Am_VALUES);
  loc_values->top_control = win.Get_Object(Am_TOP_HOW).Get(Am_VALUE);
  win.Get_Object(Am_TOP_AMOUNT_WIDGET).Set(Am_VALUE, 0);
}
Am_Define_Method(Am_Object_Method, void, set_width_zero, (Am_Object cmd))
{
  Am_Object win = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  am_loc_values *loc_values = (am_loc_values *)(Am_Ptr)win.Get(Am_VALUES);
  loc_values->width_control = win.Get_Object(Am_WIDTH_HOW).Get(Am_VALUE);
  win.Get_Object(Am_WIDTH_AMOUNT_WIDGET).Set(Am_VALUE, 0);
}
Am_Define_Method(Am_Object_Method, void, set_height_zero, (Am_Object cmd))
{
  Am_Object win = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  am_loc_values *loc_values = (am_loc_values *)(Am_Ptr)win.Get(Am_VALUES);
  loc_values->height_control = win.Get_Object(Am_HEIGHT_HOW).Get(Am_VALUE);
  win.Get_Object(Am_HEIGHT_AMOUNT_WIDGET).Set(Am_VALUE, 0);
}

Am_Define_Method(Am_Object_Method, void, left_amount_widget_set,
                 (Am_Object cmd))
{
  Am_Object win = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  int new_val = cmd.Get(Am_VALUE);
  am_loc_values *loc_values = (am_loc_values *)(Am_Ptr)win.Get(Am_VALUES);
  int left_control = win.Get_Object(Am_LEFT_HOW).Get(Am_VALUE);
  switch (left_control) {
  case am_change_equals:
    loc_values->left_equals = new_val;
    break;
  case am_change_offset:
    loc_values->left_offset = new_val;
    break;
  case am_ask_user_generalize:
    break; //shouldn't happen
  case am_ask_user_type:
    break; //shouldn't happen
  case am_change_offset_centered:
    loc_values->left_centered = new_val;
    break;
  case am_change_offset_left_outside:
  case am_change_offset_left_inside:
  case am_change_offset_right_inside:
  case am_change_offset_right_outside:
    loc_values->left_object_offset = new_val;
    break;
  default:
    Am_ERROR("Bad type for left control " << left_control);
  } //end switch
}
Am_Define_Method(Am_Object_Method, void, top_amount_widget_set, (Am_Object cmd))
{
  Am_Object win = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  int new_val = cmd.Get(Am_VALUE);
  am_loc_values *loc_values = (am_loc_values *)(Am_Ptr)win.Get(Am_VALUES);
  int top_control = win.Get_Object(Am_TOP_HOW).Get(Am_VALUE);
  switch (top_control) {
  case am_change_equals:
    loc_values->top_equals = new_val;
    break;
  case am_change_offset:
    loc_values->top_offset = new_val;
    break;
  case am_ask_user_generalize:
    break; //shouldn't happen
  case am_ask_user_type:
    break; //shouldn't happen
  case am_change_offset_centered:
    loc_values->top_centered = new_val;
    break;
  case am_change_offset_top_outside:
  case am_change_offset_top_inside:
  case am_change_offset_bottom_inside:
  case am_change_offset_bottom_outside:
    loc_values->top_object_offset = new_val;
    break;
  default:
    Am_ERROR("Bad type for top control " << top_control);
  } //end switch
}
Am_Define_Method(Am_Object_Method, void, width_amount_widget_set,
                 (Am_Object cmd))
{
  Am_Object win = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  int new_val = cmd.Get(Am_VALUE);
  am_loc_values *loc_values = (am_loc_values *)(Am_Ptr)win.Get(Am_VALUES);
  int width_control = win.Get_Object(Am_WIDTH_HOW).Get(Am_VALUE);
  switch (width_control) {
  case am_change_equals:
    loc_values->width_equals = new_val;
    break;
  case am_change_offset:
    loc_values->width_offset = new_val;
    break;
  case am_ask_user_generalize:
    break; //shouldn't happen
  case am_ask_user_type:
    break; //shouldn't happen
  case am_change_object_wh:
    loc_values->width_object_offset = new_val;
    break;
  case am_change_object_percent:
    loc_values->width_object_percent = new_val;
    break;
  default:
    Am_ERROR("Bad type for width control " << width_control);
  } //end switch
}
Am_Define_Method(Am_Object_Method, void, height_amount_widget_set,
                 (Am_Object cmd))
{
  Am_Object win = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  int new_val = cmd.Get(Am_VALUE);
  am_loc_values *loc_values = (am_loc_values *)(Am_Ptr)win.Get(Am_VALUES);
  int height_control = win.Get_Object(Am_HEIGHT_HOW).Get(Am_VALUE);
  switch (height_control) {
  case am_change_equals:
    loc_values->height_equals = new_val;
    break;
  case am_change_offset:
    loc_values->height_offset = new_val;
    break;
  case am_ask_user_generalize:
    break; //shouldn't happen
  case am_ask_user_type:
    break; //shouldn't happen
  case am_change_object_wh:
    loc_values->height_object_offset = new_val;
    break;
  case am_change_object_percent:
    loc_values->height_object_percent = new_val;
    break;
  default:
    Am_ERROR("Bad type for height control " << height_control);
  } //end switch
}

Am_Define_Formula(bool, am_active_if_change_left)
{
  return self.Get_Sibling(Am_CHANGE_LEFT).Get(Am_VALUE).Valid();
}
Am_Define_Formula(bool, am_active_if_change_top)
{
  return self.Get_Sibling(Am_CHANGE_TOP).Get(Am_VALUE).Valid();
}
Am_Define_Formula(bool, am_active_if_change_width)
{
  return self.Get_Sibling(Am_CHANGE_WIDTH).Get(Am_VALUE).Valid();
}
Am_Define_Formula(bool, am_active_if_change_height)
{
  return self.Get_Sibling(Am_CHANGE_HEIGHT).Get(Am_VALUE).Valid();
}
Am_Define_Formula(bool, am_active_if_change_left_and_not_user)
{
  return self.Get_Sibling(Am_CHANGE_LEFT).Get(Am_VALUE).Valid() &&
         (int)self.Get_Sibling(Am_LEFT_HOW).Get(Am_VALUE) < am_ask_user_base;
}
Am_Define_Formula(bool, am_active_if_change_top_and_not_user)
{
  return self.Get_Sibling(Am_CHANGE_TOP).Get(Am_VALUE).Valid() &&
         (int)self.Get_Sibling(Am_TOP_HOW).Get(Am_VALUE) < am_ask_user_base;
}
Am_Define_Formula(bool, am_active_if_change_width_and_not_user)
{
  return self.Get_Sibling(Am_CHANGE_WIDTH).Get(Am_VALUE).Valid() &&
         (int)self.Get_Sibling(Am_WIDTH_HOW).Get(Am_VALUE) < am_ask_user_base;
}
Am_Define_Formula(bool, am_active_if_change_height_and_not_user)
{
  return self.Get_Sibling(Am_CHANGE_HEIGHT).Get(Am_VALUE).Valid() &&
         (int)self.Get_Sibling(Am_HEIGHT_HOW).Get(Am_VALUE) < am_ask_user_base;
}

Am_Define_Formula(bool, am_active_if_need_object)
{
  int val;
  if (self.Get_Sibling(Am_CHANGE_LEFT).Get(Am_VALUE).Valid()) {
    val = self.Get_Sibling(Am_LEFT_HOW).Get(Am_VALUE);
    if (val >= am_change_offset_left_outside && val < am_ask_user_base)
      return true;
  }

  if (self.Get_Sibling(Am_CHANGE_TOP).Get(Am_VALUE).Valid()) {
    val = self.Get_Sibling(Am_TOP_HOW).Get(Am_VALUE);
    if (val >= am_change_offset_left_outside && val < am_ask_user_base)
      return true;
  }

  if (self.Get_Sibling(Am_CHANGE_WIDTH).Get(Am_VALUE).Valid()) {
    val = self.Get_Sibling(Am_WIDTH_HOW).Get(Am_VALUE);
    if (val >= am_change_offset_left_outside && val < am_ask_user_base)
      return true;
  }

  if (self.Get_Sibling(Am_CHANGE_HEIGHT).Get(Am_VALUE).Valid()) {
    val = self.Get_Sibling(Am_HEIGHT_HOW).Get(Am_VALUE);
    if (val >= am_change_offset_left_outside && val < am_ask_user_base)
      return true;
  }

  return false;
}

Am_Object
am_locgen_Initialize()
{
  am_locgen = Am_Window.Create(DSTR("generalizeloc"))
                  .Set(Am_DESTROY_WINDOW_METHOD,
                       Am_Default_Pop_Up_Window_Destroy_Method)
                  .Set(Am_FILL_STYLE, Am_Default_Color)
                  .Set(Am_TITLE, "Generalize Size and Position")
                  .Set(Am_WIDTH, 445)
                  .Set(Am_HEIGHT, 292)

                  .Add(Am_VALUE, (0L))
                  .Add(Am_VALUES, (0L))
                  .Add(Am_SCRIPT_WINDOW, (0L))
                  .Add(Am_COMMAND, (0L))
                  .Add(Am_SLOTS_TO_SAVE, (0L));
  locgen_load_bitmaps();
  am_locgen
      .Add_Part(Am_Text.Create()
                    .Set(Am_LEFT, 6)
                    .Set(Am_TOP, 11)
                    .Set(Am_WIDTH, 353)
                    .Set(Am_HEIGHT, 22)
                    .Set(Am_TEXT, "Generalize the Position and Size")
                    .Set(Am_FONT, Am_Font(Am_FONT_FIXED, true, false, false,
                                          Am_FONT_LARGE))
                    .Set(Am_LINE_STYLE, Am_Black)
                    .Set(Am_FILL_STYLE, Am_No_Style))
      .Add_Part(Am_Text.Create()
                    .Set(Am_LEFT, 14)
                    .Set(Am_TOP, 43)
                    .Set(Am_TEXT, "Change the:")
                    .Set(Am_FONT, Am_Font(Am_FONT_SERIF, true, false, false,
                                          Am_FONT_MEDIUM))
                    .Set(Am_LINE_STYLE, Am_Black)
                    .Set(Am_FILL_STYLE, Am_No_Style))

      .Add_Part(
          Am_CHANGE_LEFT,
          Am_Checkbox.Create(DSTR("Am_CHANGE_LEFT"))
              .Set(Am_LEFT, 22)
              .Set(Am_TOP, 65)
              .Set(Am_FILL_STYLE, Am_Default_Color)
              .Set_Part(
                  Am_COMMAND,
                  (Am_Command.Create().Set(Am_LABEL, "Left").Set(Am_ID, true))))
      .Add_Part(
          Am_LEFT_HOW,
          Am_Option_Button.Create(DSTR("Am_LEFT_HOW"))
              .Set(Am_LEFT, 90)
              .Set(Am_TOP, 65)
              .Set(Am_LEAVE_ROOM_FOR_FRINGE, false)
              .Set(Am_ACTIVE, am_active_if_change_left)
              .Set(Am_FILL_STYLE, Am_Default_Color)
              .Set(Am_ITEMS,
                   Am_Value_List()
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, "Equals Constant")
                                .Set(Am_DO_METHOD, set_left_equals)
                                .Set(Am_ID, am_change_equals))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, "Difference from old")
                                .Set(Am_DO_METHOD, set_left_offset)
                                .Set(Am_ID, am_change_offset))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, "Ask User to Click")
                                .Set(Am_DO_METHOD, set_left_zero)
                                .Set(Am_ID, am_ask_user_generalize))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, "Ask User to Type")
                                .Set(Am_DO_METHOD, set_left_zero)
                                .Set(Am_ID, am_ask_user_type))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, rightout)
                                .Set(Am_DO_METHOD, set_left_object_offset)
                                .Set(Am_ID, am_change_offset_right_outside))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, rightin)
                                .Set(Am_DO_METHOD, set_left_object_offset)
                                .Set(Am_ID, am_change_offset_right_inside))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, centerX)
                                .Set(Am_DO_METHOD, set_left_centered)
                                .Set(Am_ID, am_change_offset_centered))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, leftin)
                                .Set(Am_DO_METHOD, set_left_object_offset)
                                .Set(Am_ID, am_change_offset_left_inside))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, leftout)
                                .Set(Am_DO_METHOD, set_left_object_offset)
                                .Set(Am_ID, am_change_offset_left_outside))))
      .Add_Part(Am_LEFT_AMOUNT_WIDGET,
                Am_Number_Input_Widget.Create(DSTR("LEFT_AMOUNT"))
                    .Set(Am_TOP, 65)
                    .Set(Am_LEFT, Am_Right_Of_Sibling(Am_LEFT_HOW, 10))
                    .Set(Am_WIDTH, 100)
                    .Set(Am_ACTIVE, am_active_if_change_left_and_not_user)
                    .Get_Object(Am_COMMAND)
                    .Set(Am_LABEL, "")
                    .Set(Am_DO_METHOD, left_amount_widget_set)
                    .Get_Owner()
                    .Set(Am_FILL_STYLE, Am_Default_Color))

      .Add_Part(
          Am_CHANGE_TOP,
          Am_Checkbox.Create(DSTR("Am_CHANGE_TOP"))
              .Set(Am_LEFT, 22)
              .Set(Am_TOP, 95)
              .Set(Am_FILL_STYLE, Am_Default_Color)
              .Set_Part(
                  Am_COMMAND,
                  (Am_Command.Create().Set(Am_LABEL, "Top").Set(Am_ID, true))))
      .Add_Part(
          Am_TOP_HOW,
          Am_Option_Button.Create(DSTR("Am_TOP_HOW"))
              .Set(Am_LEFT, 90)
              .Set(Am_TOP, 95)
              .Set(Am_LEAVE_ROOM_FOR_FRINGE, false)
              .Set(Am_ACTIVE, am_active_if_change_top)
              .Set(Am_FILL_STYLE, Am_Default_Color)
              .Set(Am_ITEMS,
                   Am_Value_List()
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, "Equals Constant")
                                .Set(Am_DO_METHOD, set_top_equals)
                                .Set(Am_ID, am_change_equals))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, "Difference from old")
                                .Set(Am_DO_METHOD, set_top_offset)
                                .Set(Am_ID, am_change_offset))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, "Ask User to Click")
                                .Set(Am_DO_METHOD, set_top_zero)
                                .Set(Am_ID, am_ask_user_generalize))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, "Ask User to Type")
                                .Set(Am_DO_METHOD, set_top_zero)
                                .Set(Am_ID, am_ask_user_type))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, bottomout)
                                .Set(Am_DO_METHOD, set_top_object_offset)
                                .Set(Am_ID, am_change_offset_bottom_outside))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, bottomin)
                                .Set(Am_DO_METHOD, set_top_object_offset)
                                .Set(Am_ID, am_change_offset_bottom_inside))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, centerY)
                                .Set(Am_DO_METHOD, set_top_centered)
                                .Set(Am_ID, am_change_offset_centered))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, topin)
                                .Set(Am_DO_METHOD, set_top_object_offset)
                                .Set(Am_ID, am_change_offset_top_inside))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, topout)
                                .Set(Am_DO_METHOD, set_top_object_offset)
                                .Set(Am_ID, am_change_offset_top_outside))))
      .Add_Part(Am_TOP_AMOUNT_WIDGET,
                Am_Number_Input_Widget.Create(DSTR("TOP_AMOUNT"))
                    .Set(Am_LEFT, Am_Right_Of_Sibling(Am_TOP_HOW, 10))
                    .Set(Am_WIDTH, 100)
                    .Set(Am_TOP, 95)
                    .Set(Am_ACTIVE, am_active_if_change_top_and_not_user)
                    .Get_Object(Am_COMMAND)
                    .Set(Am_LABEL, "")
                    .Set(Am_DO_METHOD, top_amount_widget_set)
                    .Get_Owner()
                    .Set(Am_FILL_STYLE, Am_Default_Color))

      .Add_Part(Am_CHANGE_WIDTH,
                Am_Checkbox.Create(DSTR("Am_CHANGE_WIDTH"))
                    .Set(Am_LEFT, 22)
                    .Set(Am_TOP, 125)
                    .Set(Am_FILL_STYLE, Am_Default_Color)
                    .Set_Part(Am_COMMAND, (Am_Command.Create()
                                               .Set(Am_LABEL, "Width")
                                               .Set(Am_ID, true))))
      .Add_Part(
          Am_WIDTH_HOW,
          Am_Option_Button.Create(DSTR("Am_WIDTH_HOW"))
              .Set(Am_LEFT, 90)
              .Set(Am_TOP, 125)
              .Set(Am_LEAVE_ROOM_FOR_FRINGE, false)
              .Set(Am_ACTIVE, am_active_if_change_width)
              .Set(Am_FILL_STYLE, Am_Default_Color)
              .Set(Am_ITEMS,
                   Am_Value_List()
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, "Equals Constant")
                                .Set(Am_DO_METHOD, set_width_equals)
                                .Set(Am_ID, am_change_equals))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, "Difference from old")
                                .Set(Am_DO_METHOD, set_width_offset)
                                .Set(Am_ID, am_change_offset))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, "Ask User to Drag")
                                .Set(Am_DO_METHOD, set_width_zero)
                                .Set(Am_ID, am_ask_user_generalize))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, "Ask User to Type")
                                .Set(Am_DO_METHOD, set_width_zero)
                                .Set(Am_ID, am_ask_user_type))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, widthdiff)
                                .Set(Am_DO_METHOD, set_width_object_offset)
                                .Set(Am_ID, am_change_object_wh))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, widthpercent)
                                .Set(Am_DO_METHOD, set_width_object_percent)
                                .Set(Am_ID, am_change_object_percent))))
      .Add_Part(Am_WIDTH_AMOUNT_WIDGET,
                Am_Number_Input_Widget.Create(DSTR("WIDTH_AMOUNT"))
                    .Set(Am_LEFT, Am_Right_Of_Sibling(Am_WIDTH_HOW, 10))
                    .Set(Am_WIDTH, 100)
                    .Set(Am_TOP, 125)
                    .Set(Am_ACTIVE, am_active_if_change_width_and_not_user)
                    .Get_Object(Am_COMMAND)
                    .Set(Am_LABEL, "")
                    .Set(Am_DO_METHOD, width_amount_widget_set)
                    .Get_Owner()
                    .Set(Am_FILL_STYLE, Am_Default_Color))

      .Add_Part(Am_CHANGE_HEIGHT,
                Am_Checkbox.Create(DSTR("Am_CHANGE_HEIGHT"))
                    .Set(Am_LEFT, 22)
                    .Set(Am_TOP, 175)
                    .Set(Am_FILL_STYLE, Am_Default_Color)
                    .Set_Part(Am_COMMAND, (Am_Command.Create()
                                               .Set(Am_LABEL, "Height")
                                               .Set(Am_ID, true))))
      .Add_Part(
          Am_HEIGHT_HOW,
          Am_Option_Button.Create(DSTR("Am_HEIGHT_HOW"))
              .Set(Am_LEFT, 90)
              .Set(Am_ACTIVE, am_active_if_change_height)
              .Set(Am_TOP, 175)
              .Set(Am_LEAVE_ROOM_FOR_FRINGE, false)
              .Set(Am_FILL_STYLE, Am_Default_Color)
              .Set(Am_ITEMS,
                   Am_Value_List()
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, "Equals Constant")
                                .Set(Am_DO_METHOD, set_height_equals)
                                .Set(Am_ID, am_change_equals))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, "Difference from old")
                                .Set(Am_DO_METHOD, set_height_offset)
                                .Set(Am_ID, am_change_offset))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, "Ask User to Drag")
                                .Set(Am_DO_METHOD, set_height_zero)
                                .Set(Am_ID, am_ask_user_generalize))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, "Ask User to Type")
                                .Set(Am_DO_METHOD, set_height_zero)
                                .Set(Am_ID, am_ask_user_type))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, heightdiff)
                                .Set(Am_DO_METHOD, set_height_object_offset)
                                .Set(Am_ID, am_change_object_wh))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, heightpercent)
                                .Set(Am_DO_METHOD, set_height_object_percent)
                                .Set(Am_ID, am_change_object_percent))))
      .Add_Part(Am_HEIGHT_AMOUNT_WIDGET,
                Am_Number_Input_Widget.Create(DSTR("HEIGHT_AMOUNT"))
                    .Set(Am_LEFT, Am_Right_Of_Sibling(Am_HEIGHT_HOW, 10))
                    .Set(Am_WIDTH, 100)
                    .Set(Am_TOP, 175)
                    .Set(Am_ACTIVE, am_active_if_change_height_and_not_user)
                    .Get_Object(Am_COMMAND)
                    .Set(Am_LABEL, "")
                    .Set(Am_DO_METHOD, height_amount_widget_set)
                    .Get_Owner()
                    .Set(Am_FILL_STYLE, Am_Default_Color))
      .Add_Part(Am_REF_OBJ, Am_Rectangle.Create()
                                .Set(Am_FILL_STYLE, Am_No_Style)
                                .Set(Am_LINE_STYLE, Am_Gray_Stipple)
                                .Set(Am_LEFT, REF_OBJ_LEFT)
                                .Set(Am_TOP, REF_OBJ_TOP)
                                .Set(Am_WIDTH, REF_OBJ_SIZE)
                                .Set(Am_HEIGHT, REF_OBJ_SIZE))
      .Add_Part(Am_SHOW_LOC_OBJ,
                Am_Rectangle.Create()
                    .Set(Am_FILL_STYLE, Am_Gray_Stipple)
                    .Set(Am_LINE_STYLE, Am_No_Style)
                    .Set(Am_LEFT, REF_OBJ_LEFT + SHOW_LOC_OBJ_OFFSET)
                    .Set(Am_TOP, REF_OBJ_TOP + SHOW_LOC_OBJ_OFFSET)
                    .Set(Am_WIDTH, SHOW_LOC_OBJ_SIZE)
                    .Set(Am_HEIGHT, SHOW_LOC_OBJ_SIZE))
      .Add_Part(Am_COMMAND_LOAD_BUTTON,
                Am_Button.Create(DSTR("Am_COMMAND_LOAD_BUTTON"))
                    .Set(Am_LEFT, 103)
                    .Set(Am_TOP, 220)
                    .Set(Am_FILL_STYLE, Am_Default_Color)
                    .Set(Am_LEAVE_ROOM_FOR_FRINGE, false)
                    .Set(Am_ACTIVE, am_active_if_need_object)
                    .Get_Object(Am_COMMAND)
                    .Set(Am_LABEL, "Of Object...")
                    .Set(Am_DO_METHOD, am_loc_from_object)
                    .Get_Owner())
      .Add_Part(
          Am_CONSTANT_OBJECTS_WIDGET,
          Am_Text_Input_Widget.Create(DSTR("Am_CONSTANT_OBJECTS_WIDGET"))
              .Set(Am_LEFT, Am_Right_Of_Sibling(Am_COMMAND_LOAD_BUTTON, 10))
              .Set(Am_TOP, 220)
              .Set(Am_WIDTH, 225)
              .Set(Am_ACTIVE, am_active_if_need_object)
              .Set(Am_ACTIVE_2, false) // **TEMPORARY
              .Get_Object(Am_COMMAND)
              .Set(Am_LABEL, "")
              .Get_Owner()
              .Set(Am_FILL_STYLE, Am_Default_Color))
      .Add_Part(
          Am_Button_Panel.Create()
              .Set(Am_LEFT, Am_Center_X_Is_Center_Of_Owner)
              .Set(Am_TOP, 250)
              .Set(Am_FILL_STYLE, Am_Default_Color)
              .Set(Am_LAYOUT, Am_Horizontal_Layout)
              .Set(Am_H_SPACING, 20)
              .Set(Am_V_SPACING, 0)
              .Set(Am_MAX_RANK, 0)
              .Set(Am_ITEMS, Am_Value_List()
                                 .Add(Am_Standard_OK_Command.Create().Set(
                                     Am_DO_METHOD, am_locgen_ok))
                                 .Add(Am_Standard_Cancel_Command.Create().Set(
                                     Am_DO_METHOD, am_locgen_cancel))))
      .Add_Part(Am_Tab_To_Next_Widget_Interactor.Create());
  return am_locgen;
}
