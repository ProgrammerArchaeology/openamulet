/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#ifndef FORMULA_ADV_H
#define FORMULA_ADV_H

#include <am_inc.h>

#include <amulet/formula.h>
#include <amulet/object_advanced.h>

typedef unsigned short Am_Formula_Mode;

// MULTI_CONSTRAINT: Allow this formula constraint to remain when other
// formula constraints or explicit values are set into the same slot.
// The default value for this flag is off, so the constraint will be removed
// when another formula constraint or explicit value is set into the slot.
#define Am_FORMULA_MULTI_CONSTRAINT 0x01

// MULTI_LOCAL_CONSTRAINT: Allow this formula constraint to coexist with other
// local formula constraints at the same instance level.  The default value
// for this flag is off, so the constraint will be removed when another
// formula constraint is set at the same instance level.
// Example:  a.left contains an inherited formula f1 and a local formula
// f2 (both have MULTI_CONSTRAINT set and MULTI_LOCAL_CONSTRAINT cleared).
// If a new formula f3 is set into a.left, then f2 will disappear, leaving
// (f1, f3) on the slot.  But if f2 has MULTI_LOCAL_CONSTRAINT set, then f2
// will remain, leaving (f1, f2, f3) on the slot.
#define Am_FORMULA_MULTI_LOCAL_CONSTRAINT 0x02

// DO_NOT_PROPAGATE: Prevent this formula constraint from propagating
// invalidation messages.  Default value is off, so messages are propagated.
#define Am_FORMULA_DO_NOT_PROPAGATE 0x04

class _OA_DL_CLASSIMPORT Am_Formula_Advanced : public Am_Constraint
{
public:
  static bool Test(Am_Constraint *formula);
  static Am_Formula_Advanced *Narrow(Am_Constraint *formula);

  virtual Am_Formula_Mode Get_Mode() = 0;
  virtual void Set_Mode(Am_Formula_Mode mode) = 0;

  virtual Am_Slot Get_Context() = 0;
};

class Am_Dependency_Data;

class _OA_DL_CLASSIMPORT Am_Depends_Iterator
{
public:
  Am_Depends_Iterator();
  Am_Depends_Iterator(const Am_Formula_Advanced *formula);

  Am_Depends_Iterator &operator=(const Am_Formula_Advanced *formula);

  unsigned short Length() const; // Number of slots in the list.
  void Start();                  // Begin list at the start.
  void Next();                   // Move to next element in list.
  bool Last() const;             // Is this the last element?
  Am_Slot Get() const;           // Get the current element.

private:
  const Am_Formula_Advanced *context;
  Am_Dependency_Data *current;
};

#endif
