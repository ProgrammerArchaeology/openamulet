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

#include <amulet/impl/slots_opal.h>
#include <amulet/impl/slots_registry.h>
#include <amulet/impl/am_object.h>

void
set_slot_names()
{
#ifdef DEBUG
  Am_Register_Slot_Key(Am_NO_SLOT, "NO_SLOT");
  Am_Register_Slot_Key(Am_NO_INHERIT, "NO_INHERIT");
  Am_Register_Slot_Key(Am_OWNER, "OWNER");
  Am_Register_Slot_Key(Am_PROTOTYPE, "PROTOTYPE");
  Am_Register_Slot_Key(Am_SOURCE_OF_COPY, "SOURCE_OF_COPY");
  Am_Register_Slot_Key(Am_LEFT, "LEFT");
  Am_Register_Slot_Key(Am_TOP, "TOP");
  Am_Register_Slot_Key(Am_WIDTH, "WIDTH");
  Am_Register_Slot_Key(Am_HEIGHT, "HEIGHT");
  Am_Register_Slot_Key(Am_WINDOW, "WINDOW");
  Am_Register_Slot_Key(Am_VISIBLE, "VISIBLE");
  Am_Register_Slot_Key(Am_TITLE, "TITLE");
  Am_Register_Slot_Key(Am_ICON_TITLE, "ICON_TITLE");
  Am_Register_Slot_Key(Am_TEXT, "TEXT");
  Am_Register_Slot_Key(Am_CURSOR_INDEX, "CURSOR_INDEX");
  Am_Register_Slot_Key(Am_CURSOR_OFFSET, "CURSOR_OFFSET");
  Am_Register_Slot_Key(Am_INVERT, "INVERT");
  Am_Register_Slot_Key(Am_OFFSCREEN_DRAWONABLE, "~OFFSCREEN_DRAWONABLE~");
  Am_Register_Slot_Key(Am_DOUBLE_BUFFER, "DOUBLE_BUFFER");
  Am_Register_Slot_Key(Am_DESTROY_WINDOW_METHOD, "~DESTROY_WINDOW_METHOD~");
  Am_Register_Slot_Key(Am_POINT_LIST, "POINT_LIST");
  Am_Register_Slot_Key(Am_FILL_STYLE, "FILL_STYLE");
  Am_Register_Slot_Key(Am_LINE_STYLE, "LINE_STYLE");
  Am_Register_Slot_Key(Am_ANGLE1, "ANGLE1");
  Am_Register_Slot_Key(Am_ANGLE2, "ANGLE2");
  Am_Register_Slot_Key(Am_RADIUS, "RADIUS");
  Am_Register_Slot_Key(Am_DRAW_RADIUS, "DRAW_RADIUS");
  Am_Register_Slot_Key(Am_FONT, "FONT");
  Am_Register_Slot_Key(Am_IMAGE, "IMAGE");
  Am_Register_Slot_Key(Am_IS_COLOR, "IS_COLOR");
  Am_Register_Slot_Key(Am_MAX_WIDTH, "MAX_WIDTH");
  Am_Register_Slot_Key(Am_MAX_HEIGHT, "MAX_HEIGHT");
  Am_Register_Slot_Key(Am_MIN_WIDTH, "MIN_WIDTH");
  Am_Register_Slot_Key(Am_MIN_HEIGHT, "MIN_HEIGHT");
  Am_Register_Slot_Key(Am_USE_MAX_WIDTH, "USE_MAX_WIDTH");
  Am_Register_Slot_Key(Am_USE_MAX_HEIGHT, "USE_MAX_HEIGHT");
  Am_Register_Slot_Key(Am_USE_MIN_WIDTH, "USE_MIN_WIDTH");
  Am_Register_Slot_Key(Am_USE_MIN_HEIGHT, "USE_MIN_HEIGHT");
  Am_Register_Slot_Key(Am_ICONIFIED, "ICONIFIED");
  Am_Register_Slot_Key(Am_QUERY_POSITION, "QUERY_POSITION");
  Am_Register_Slot_Key(Am_QUERY_SIZE, "QUERY_SIZE");
  Am_Register_Slot_Key(Am_LEFT_BORDER_WIDTH, "LEFT_BORDER_WIDTH");
  Am_Register_Slot_Key(Am_TOP_BORDER_WIDTH, "TOP_BORDER_WIDTH");
  Am_Register_Slot_Key(Am_RIGHT_BORDER_WIDTH, "RIGHT_BORDER_WIDTH");
  Am_Register_Slot_Key(Am_BOTTOM_BORDER_WIDTH, "BOTTOM_BORDER_WIDTH");
  Am_Register_Slot_Key(Am_CURSOR, "CURSOR");
  Am_Register_Slot_Key(Am_OMIT_TITLE_BAR, "OMIT_TITLE_BAR");
  Am_Register_Slot_Key(Am_SAVE_UNDER, "SAVE_UNDER");
  Am_Register_Slot_Key(Am_CLIP_CHILDREN, "CLIP_CHILDREN");
  Am_Register_Slot_Key(Am_CLIP, "CLIP");
  Am_Register_Slot_Key(Am_GRAPHICAL_PARTS, "GRAPHICAL_PARTS");
  Am_Register_Slot_Key(Am_FLIP_BOOK_PARTS, "FLIP_BOOK_PARTS");
  Am_Register_Slot_Key(Am_PRETEND_TO_BE_LEAF, "PRETEND_TO_BE_LEAF");
  Am_Register_Slot_Key(Am_RANK, "~RANK~");
  Am_Register_Slot_Key(Am_OWNER_DEPTH, "~OWNER_DEPTH~");
  Am_Register_Slot_Key(Am_FADE_DEPTH, "~FADE_DEPTH~");
  Am_Register_Slot_Key(Am_OBJECT_IN_PROGRESS, "~OBJECT_IN_PROGRESS~");
  Am_Register_Slot_Key(Am_X1, "X1");
  Am_Register_Slot_Key(Am_Y1, "Y1");
  Am_Register_Slot_Key(Am_X2, "X2");
  Am_Register_Slot_Key(Am_Y2, "Y2");
  Am_Register_Slot_Key(Am_DIRECTIONAL, "DIRECTIONAL");
  Am_Register_Slot_Key(Am_HEAD_LENGTH, "HEAD_LENGTH");
  Am_Register_Slot_Key(Am_HEAD_WIDTH, "HEAD_WIDTH");
  Am_Register_Slot_Key(Am_TAIL_LENGTH, "TAIL_LENGTH");
  Am_Register_Slot_Key(Am_TAIL_WIDTH, "TAIL_WIDTH");
  Am_Register_Slot_Key(Am_LAYOUT, "LAYOUT");
  Am_Register_Slot_Key(Am_H_SPACING, "H_SPACING");
  Am_Register_Slot_Key(Am_V_SPACING, "V_SPACING");
  Am_Register_Slot_Key(Am_H_ALIGN, "H_ALIGN");
  Am_Register_Slot_Key(Am_V_ALIGN, "V_ALIGN");
  Am_Register_Slot_Key(Am_X_OFFSET, "X_OFFSET");
  Am_Register_Slot_Key(Am_Y_OFFSET, "Y_OFFSET");
  Am_Register_Slot_Key(Am_LEFT_OFFSET, "LEFT_OFFSET");
  Am_Register_Slot_Key(Am_TOP_OFFSET, "TOP_OFFSET");
  Am_Register_Slot_Key(Am_RIGHT_OFFSET, "RIGHT_OFFSET");
  Am_Register_Slot_Key(Am_BOTTOM_OFFSET, "BOTTOM_OFFSET");
  Am_Register_Slot_Key(Am_FIXED_WIDTH, "FIXED_WIDTH");
  Am_Register_Slot_Key(Am_FIXED_HEIGHT, "FIXED_HEIGHT");
  Am_Register_Slot_Key(Am_INDENT, "INDENT");
  Am_Register_Slot_Key(Am_MAX_RANK, "MAX_RANK");
  Am_Register_Slot_Key(Am_MAX_SIZE, "MAX_SIZE");
  Am_Register_Slot_Key(Am_ITEMS, "ITEMS");
  Am_Register_Slot_Key(Am_ITEM, "ITEM");
  Am_Register_Slot_Key(Am_ITEM_METHOD, "ITEM_METHOD");
  Am_Register_Slot_Key(Am_ITEM_PROTOTYPE, "ITEM_PROTOTYPE");

  Am_Register_Slot_Key(Am_DRAW_METHOD, "~DRAW_METHOD~");
  Am_Register_Slot_Key(Am_MASK_METHOD, "~MASK_METHOD~");
  Am_Register_Slot_Key(Am_DRAWONABLE, "~DRAWONABLE~");
  Am_Register_Slot_Key(Am_DRAW_BUFFER, "~DRAW_BUFFER~");
  Am_Register_Slot_Key(Am_MASK_BUFFER, "~MASK_BUFFER~");
  Am_Register_Slot_Key(Am_SCREEN, "~SCREEN~");
  Am_Register_Slot_Key(Am_TODO, "~TODO~");
  Am_Register_Slot_Key(Am_INVALID_METHOD, "~INVALID_METHOD~");
  Am_Register_Slot_Key(Am_PREV_STATE, "~PREV_STATE~");
  Am_Register_Slot_Key(Am_POINT_IN_OBJ_METHOD, "~POINT_IN_OBJ_METHOD~");
  Am_Register_Slot_Key(Am_POINT_IN_PART_METHOD, "~POINT_IN_PART_METHOD~");
  Am_Register_Slot_Key(Am_POINT_IN_LEAF_METHOD, "~POINT_IN_LEAF_METHOD~");
  Am_Register_Slot_Key(Am_TRANSLATE_COORDINATES_METHOD,
                       "~TRANSLATE_COORDINATES_METHOD~");
  Am_Register_Slot_Key(Am_INIT_WANT_ENTER_LEAVE, "~INIT_WANT_ENTER_LEAVE~");
  Am_Register_Slot_Key(Am_INIT_WANT_MULTI_WINDOW, "~INIT_WANT_MULTI_WINDOW~");
  Am_Register_Slot_Key(Am_OLD_WIDTH, "~OLD_WIDTH~");
  Am_Register_Slot_Key(Am_OLD_HEIGHT, "~OLD_HEIGHT~");
  Am_Register_Slot_Key(Am_WAITING_FOR_COMPLETION, "~WAITING_FOR_COMPLETION~");
  Am_Register_Slot_Key(Am_COMPLETION_VALUE, "~COMPLETION_VALUE~");
  Am_Register_Slot_Key(Am_SELECT_OUTLINE_ONLY, "SELECT_OUTLINE_ONLY");
  Am_Register_Slot_Key(Am_SELECT_FULL_INTERIOR, "SELECT_FULL_INTERIOR");
  Am_Register_Slot_Key(Am_HIT_THRESHOLD, "HIT_THRESHOLD");

  Am_Register_Slot_Key(Am_SAVE_OBJECT_METHOD, "SAVE_OBJECT_METHOD");
  Am_Register_Slot_Key(Am_SLOTS_TO_SAVE, "SLOTS_TO_SAVE");
  Am_Register_Slot_Key(Am_DRAW_MONOCHROME, "DRAW_MONOCHROME");
  // Slots used by predefined formula constraints
  Am_Register_Slot_Key(Am_CENTER_X_OBJ, "CENTER_X_OBJ");
  Am_Register_Slot_Key(Am_CENTER_Y_OBJ, "CENTER_Y_OBJ");
#endif
}
