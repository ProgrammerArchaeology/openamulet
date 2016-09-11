/* ****************************** -*-c++-*- *******************************/
/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#ifndef FORMULA_H
#define FORMULA_H

#include <am_inc.h>

#include "amulet/impl/types_registered.h"
class Am_Object;
class Am_Constraint;

// Am_Set_Flags: Bit flags for Set to keep formulas even when set
#define Am_KEEP_FORMULAS    0x010000

// get all typedefs
#include "typedefs.hpp"

// Opaque definition
class Am_Formula_Data;

//each has optional formula name parameter
class _OA_DL_CLASSIMPORT Am_Formula : public Am_Registered_Type {
 public:
  Am_Formula (const Am_Formula& in_formula);
  Am_Formula (Am_FProc_Wrapper* formula, const char* name = 0);
  Am_Formula (Am_FProc_Proc*    formula, const char* name = 0);

  Am_Formula (Am_FProc_Method*  formula, const char* name = 0);
  Am_Formula (Am_FProc_Int*     formula, const char* name = 0);
  Am_Formula (Am_FProc_Long*    formula, const char* name = 0);
  Am_Formula (Am_FProc_Bool*    formula, const char* name = 0);
  Am_Formula (Am_FProc_Float*   formula, const char* name = 0);
  Am_Formula (Am_FProc_Double*  formula, const char* name = 0);
  Am_Formula (Am_FProc_Char*    formula, const char* name = 0);
  Am_Formula (Am_FProc_String*  formula, const char* name = 0);
  Am_Formula (Am_FProc_Const_String*  formula, const char* name = 0);
  Am_Formula (Am_FProc_Void*    formula, const char* name = 0);
  Am_Formula (Am_FProc_Value*   formula, const char* name = 0);
  Am_Formula (Am_FProc_Const_Value*   formula, const char* name = 0);

  ~Am_Formula ();
  Am_Formula& operator= (const Am_Formula& in_formula);

  operator Am_Constraint* ();

  Am_Constraint* Multi_Constraint (bool multi_local = false);
  void Set_Data (Am_Wrapper* data);

  // Calls the stored procedure.
  const Am_Value operator () (Am_Object& context) const;

  Am_ID_Tag ID() const;

 private:
  Am_Formula_Data* data;
};

#define Am_Define_Formula(return_type, formula_name)   			     \
/* forward declaration of procedure so can reference it */ 		     \
static return_type formula_name##_proc (Am_Object&);                         \
Am_Formula formula_name (formula_name##_proc, #formula_name);		     \
return_type formula_name##_proc (Am_Object& self)
/* put code here { ... } */

#define Am_Define_No_Self_Formula(return_type, formula_name)  		     \
/* forward declaration of procedure so can reference it */ 		     \
static return_type formula_name##_proc (Am_Object&);                         \
Am_Formula formula_name (formula_name##_proc, #formula_name);		     \
return_type formula_name##_proc (Am_Object&)
/* put code here { ... } */

#define Am_Define_Object_Formula(formula_name) \
     Am_Define_Formula (Am_Wrapper*, formula_name)
#define Am_Define_String_Formula(formula_name) \
     Am_Define_Formula (Am_Wrapper*, formula_name)

#endif
