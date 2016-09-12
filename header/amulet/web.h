/* ****************************** -*-c++-*- *******************************/
/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#ifndef WEB_H
#define WEB_H

#include <am_inc.h>

#include OBJECT_ADVANCED__H
/*
#include "amulet/impl/object_advanced_pure.h"
#include "amulet/impl/object_constr.h"
#include "amulet/impl/am_constraint_context.h"
#include "amulet/impl/am_constraint_iterator.h"
#include "amulet/impl/am_dependency_iterator.h"
#include "amulet/impl/slots.h"
*/

// Opaque data class for Webs
class Am_Web_Data;
class Am_Web_Event;

// The Web Events class is used to examine which slots have been
// changed that have invalidated the web.
class _OA_DL_CLASSIMPORT Am_Web_Events
{
public:
  unsigned short Length(); // Number of items in the list
  void Start();            // Set the list to the beginning.
  void End();
  void Next();
  void Prev();
  bool Last();
  bool First();
  Am_Value &Get_Prev_Value();
  Am_Slot Get();
  bool Find_Next(Am_Slot_Key key);
  bool Find_Prev(Am_Slot_Key key);

  Am_Web_Events(Am_Web_Data *in_data);

private:
  Am_Web_Data *data;
  Am_Web_Event *current;
};

class _OA_DL_CLASSIMPORT Am_Web_Init
{
public:
  void Note_Input(Am_Object_Advanced object, Am_Slot_Key key);
  void Note_Output(Am_Object_Advanced object, Am_Slot_Key key);

  Am_Web_Init(Am_Web_Data *in_data) { data = in_data; }

private:
  Am_Web_Data *data;
};

typedef bool Am_Web_Create_Proc(const Am_Slot &slot);
typedef void Am_Web_Initialize_Proc(const Am_Slot &slot, Am_Web_Init &init);
typedef void Am_Web_Validate_Proc(Am_Web_Events &events);

class _OA_DL_CLASSIMPORT Am_Web
{
public:
  Am_Web(Am_Web_Create_Proc *create, Am_Web_Initialize_Proc *initialize,
         Am_Web_Validate_Proc *validate);
  operator Am_Constraint *();

  static Am_ID_Tag Web_ID();

private:
  Am_Web_Create_Proc *create;
  Am_Web_Initialize_Proc *initialize;
  Am_Web_Validate_Proc *validate;
};

#endif
