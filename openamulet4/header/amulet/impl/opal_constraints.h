#include <am_inc.h>
#include FORMULA__H

// Store in Group or Map's Am_LAYOUT slot for automatic layout.
_OA_DL_IMPORT extern Am_Formula Am_Horizontal_Layout;
_OA_DL_IMPORT extern Am_Formula Am_Vertical_Layout;

// Other useful constraints for laying things out.
_OA_DL_IMPORT extern Am_Formula Am_Fill_To_Bottom;
_OA_DL_IMPORT extern Am_Formula Am_Fill_To_Right;
_OA_DL_IMPORT extern Am_Formula Am_Fill_To_Rest_Of_Width;
_OA_DL_IMPORT extern Am_Formula Am_Fill_To_Rest_Of_Height;
_OA_DL_IMPORT extern Am_Formula Am_Width_Of_Parts;
_OA_DL_IMPORT extern Am_Formula Am_Height_Of_Parts;
_OA_DL_IMPORT extern Am_Formula Am_Center_X_Is_Center_Of;
_OA_DL_IMPORT extern Am_Formula Am_Center_Y_Is_Center_Of;
_OA_DL_IMPORT extern Am_Formula Am_Center_X_Is_Center_Of_Owner;
_OA_DL_IMPORT extern Am_Formula Am_Center_Y_Is_Center_Of_Owner;
_OA_DL_IMPORT extern Am_Formula Am_Right_Is_Right_Of_Owner;
_OA_DL_IMPORT extern Am_Formula Am_Bottom_Is_Bottom_Of_Owner;

_OA_DL_IMPORT extern Am_Formula Am_Same_As (Am_Slot_Key key);
_OA_DL_IMPORT extern Am_Formula Am_From_Owner (Am_Slot_Key key);
_OA_DL_IMPORT extern Am_Formula Am_From_Part (Am_Slot_Key part, Am_Slot_Key key);
_OA_DL_IMPORT extern Am_Formula Am_From_Sibling (Am_Slot_Key sibling, Am_Slot_Key key);
_OA_DL_IMPORT extern Am_Formula Am_From_Object (Am_Object object, Am_Slot_Key key);

_OA_DL_IMPORT extern Am_Formula Am_Same_As (Am_Slot_Key key, int offset,
			      float multiplier = 1.0);
_OA_DL_IMPORT extern Am_Formula Am_From_Owner (Am_Slot_Key key, int offset,
				 float multiplier = 1.0);
_OA_DL_IMPORT extern Am_Formula Am_From_Part (Am_Slot_Key part, Am_Slot_Key key, int offset,
				float multiplier = 1.0);
_OA_DL_IMPORT extern Am_Formula Am_From_Sibling (Am_Slot_Key sibling, Am_Slot_Key key,
				   int offset, float multiplier = 1.0);
_OA_DL_IMPORT extern Am_Formula Am_From_Object (Am_Object object, Am_Slot_Key key,
				  int offset, float multiplier = 1.0);
_OA_DL_IMPORT extern Am_Formula Am_Bottom_Of_Sibling (Am_Slot_Key sibling,
				int offset = 0, float multiplier = 1.0);
_OA_DL_IMPORT extern Am_Formula Am_Right_Of_Sibling (Am_Slot_Key sibling,
				int offset = 0, float multiplier = 1.0);
_OA_DL_IMPORT extern Am_Formula Am_Rest_Of_Height_Above(Am_Slot_Key sibling,
					  int offset = 0);

