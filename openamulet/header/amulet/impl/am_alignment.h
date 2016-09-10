#include <am_inc.h>
#include "types_enum.h"
//#include AM_IO__H

//This enum is used internally; users should use the Am_Alignment
// values instead.
enum Am_Alignment_vals
{ Am_CENTER_ALIGN_val, Am_TOP_ALIGN_val, Am_BOTTOM_ALIGN_val,
  Am_LEFT_ALIGN_val, Am_RIGHT_ALIGN_val };

// For Am_H_ALIGN and Am_V_ALIGN slots.
Am_Define_Enum_Type(Am_Alignment, Am_Alignment_vals)
const Am_Alignment Am_CENTER_ALIGN (Am_CENTER_ALIGN_val);
const Am_Alignment Am_TOP_ALIGN (Am_TOP_ALIGN_val);
const Am_Alignment Am_BOTTOM_ALIGN (Am_BOTTOM_ALIGN_val);
const Am_Alignment Am_LEFT_ALIGN (Am_LEFT_ALIGN_val);
const Am_Alignment Am_RIGHT_ALIGN (Am_RIGHT_ALIGN_val);

