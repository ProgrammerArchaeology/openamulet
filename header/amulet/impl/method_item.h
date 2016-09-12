#include "types_method.h"

// Used in Am_Map.  When a component is created, the map calls this method
// to allow instance specific to be performed on the item.
// Stored in slot Am_ITEM_METHOD.
AM_DEFINE_METHOD_TYPE(Am_Item_Method, Am_Object,
		      (int rank, Am_Value& value, Am_Object item_instance))

// These are the standard prototype function for setting up each individual
// item in a map.  Stores the value in the slot Am_ITEM.
_OA_DL_IMPORT extern Am_Item_Method Am_Standard_Item_Method;
