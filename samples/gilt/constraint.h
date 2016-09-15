#ifndef _CONSTRAINT
#define _CONSTRAINT

#include <amulet.h>

enum Which_Constraint_Handle_vals
{
  NO_HANDLE_val,
  CH_TOP_val,
  CH_LEFT_val,
  CH_BOTTOM_val,
  CH_RIGHT_val,
  CH_HEIGHT_val,
  CH_WIDTH_val,
  CH_CENTER_X_val,
  CH_CENTER_Y_val,
  CH_X1_val,
  CH_X2_val,
  CH_Y1_val,
  CH_Y2_val,
  CH_SPECIAL_val
};

Am_Define_Enum_Type(Which_Constraint_Handle,
                    Which_Constraint_Handle_vals) const Which_Constraint_Handle
    NO_HANDLE(NO_HANDLE_val);
const Which_Constraint_Handle CH_TOP(CH_TOP_val);
const Which_Constraint_Handle CH_LEFT(CH_LEFT_val);
const Which_Constraint_Handle CH_BOTTOM(CH_BOTTOM_val);
const Which_Constraint_Handle CH_RIGHT(CH_RIGHT_val);
const Which_Constraint_Handle CH_HEIGHT(CH_HEIGHT_val);
const Which_Constraint_Handle CH_WIDTH(CH_WIDTH_val);
const Which_Constraint_Handle CH_CENTER_X(CH_CENTER_X_val);
const Which_Constraint_Handle CH_CENTER_Y(CH_CENTER_Y_val);
const Which_Constraint_Handle CH_X1(CH_X1_val);
const Which_Constraint_Handle CH_X2(CH_X2_val);
const Which_Constraint_Handle CH_Y1(CH_Y1_val);
const Which_Constraint_Handle CH_Y2(CH_Y2_val);
const Which_Constraint_Handle CH_SPECIAL(CH_SPECIAL_val);

Am_Define_Enum_Long_Type(Constraint_How_Set) const Constraint_How_Set
    CONSTRAINT_SETUP(0);
const Constraint_How_Set ABORT_SETUP(1);
const Constraint_How_Set MULTIPLE_SETUP(2);

Am_Define_Method_Type(Add_Handle_Method, Am_Object, (Am_Object, Am_Object));

typedef struct
{
  char var;
  bool busy;
  Am_Object *object;
} constraint_variable;

#endif
