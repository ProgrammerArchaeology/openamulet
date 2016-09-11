#ifndef INTER_ENUM_H
#define INTER_ENUM_H

#include "types_enum.h"

// type of the Am_HOW_SET slot for Choice Interactors
Am_Define_Enum_Long_Type(Am_Choice_How_Set)
const Am_Choice_How_Set Am_CHOICE_SET 			(0L);
const Am_Choice_How_Set Am_CHOICE_CLEAR 		(1L);
const Am_Choice_How_Set Am_CHOICE_TOGGLE 		(2L);
const Am_Choice_How_Set Am_CHOICE_LIST_TOGGLE 	(3L);

//This enum is used internally; users should use the Am_Move_Grow_Where_Attach
// values instead.
enum Am_Move_Grow_Where_Attach_vals
{ Am_ATTACH_WHERE_HIT_val, Am_ATTACH_NW_val,
  Am_ATTACH_N_val, Am_ATTACH_NE_val,
  Am_ATTACH_E_val, Am_ATTACH_SE_val, Am_ATTACH_S_val, 
  Am_ATTACH_SW_val, Am_ATTACH_W_val, 
  Am_ATTACH_END_1_val, Am_ATTACH_END_2_val,  
  Am_ATTACH_CENTER_val };

// type of the Am_WHERE_ATTACH slot for Move_Grow Interactors
Am_Define_Enum_Type(Am_Move_Grow_Where_Attach, Am_Move_Grow_Where_Attach_vals)
const Am_Move_Grow_Where_Attach Am_ATTACH_WHERE_HIT(Am_ATTACH_WHERE_HIT_val);
const Am_Move_Grow_Where_Attach Am_ATTACH_NW(Am_ATTACH_NW_val);
const Am_Move_Grow_Where_Attach Am_ATTACH_N(Am_ATTACH_N_val);
const Am_Move_Grow_Where_Attach Am_ATTACH_NE(Am_ATTACH_NE_val);
const Am_Move_Grow_Where_Attach Am_ATTACH_E(Am_ATTACH_E_val);
const Am_Move_Grow_Where_Attach Am_ATTACH_SE(Am_ATTACH_SE_val);
const Am_Move_Grow_Where_Attach Am_ATTACH_S(Am_ATTACH_S_val);
const Am_Move_Grow_Where_Attach Am_ATTACH_SW(Am_ATTACH_SW_val);
const Am_Move_Grow_Where_Attach Am_ATTACH_W(Am_ATTACH_W_val);
const Am_Move_Grow_Where_Attach Am_ATTACH_END_1(Am_ATTACH_END_1_val);
const Am_Move_Grow_Where_Attach Am_ATTACH_END_2(Am_ATTACH_END_2_val);
const Am_Move_Grow_Where_Attach Am_ATTACH_CENTER(Am_ATTACH_CENTER_val);

#endif // INTER_ENUM_H
