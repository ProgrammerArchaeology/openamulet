/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file implements a priority list that stores a list of objects
   with their priorities
*/
#ifndef PRIORITY_LIST_H
#define PRIORITY_LIST_H

#include <am_inc.h>

#include "amulet/impl/types_pointer.h"
#include <amulet/object.h>

class Am_Priority_List_Item
{
public:
  Am_Object obj;
  float main_priority;
  int second_priority;
  int third_priority;
  Am_Priority_List_Item *next;
  Am_Priority_List_Item *prev;
};

class Am_Priority_List
{
public:
  static Am_Priority_List *Create();

  static Am_Priority_List *Narrow(Am_Ptr value)
  // no checking, just assume is right type
  {
    return (Am_Priority_List *)value;
  }

  void Remove();

  unsigned short Length() { return number; }

  bool Empty() { return number == 0; }

  // Assumes not already in the list.  Add in priority order.
  // main_priority takes precedence over second_priority, etc.
  Am_Priority_List *Add(Am_Object obj, float main_priority, int second_priority,
                        int third_priority);

  bool Member(Am_Object obj);

  // changes the priority of the object.  Error if object is not
  // already a member.  This may be more efficient than remove and add
  void Change_Main_Priority(Am_Object obj, float main_priority);
  void Change_Second_Third_Priority(Am_Object obj, int second_priority,
                                    int third_priority);

  // Retrieve the value of the current element.  Error if no element is
  // current.  Optionally also retrieves the priorities of the element.
  Am_Object Get();
  Am_Object Get(float &main_priority, int &second_priority,
                int &third_priority);

  // Delete the current element.  Error if no element is current.  The current
  // position is shifted to the element previous to the deleted.
  void Delete();
  // Deletes obj from list if a member.  No error if not a member.
  // Will change the current position.
  void Delete(Am_Object obj);

  // Delete the entire list.  All elements are deleted. The current position
  // becomes undefined.
  void Make_Empty();

  void Start(); // Make first element be current.
  void End();   // Make last element be current.

  void Prev(); // Make previous element be current.
  void Next(); // Make next element be current.

  // Returns true when current element passes the first element.
  bool First();

  // Returns true when current element passes the last element.
  bool Last();

  // checks to see which list is the higher priority item, and returns
  // that one, forwarding its pointer.  Returns true if returned an
  // item, false when no more items.
  bool Two_List_Get_Next(Am_Priority_List *other_list, Am_Object &inter,
                         float &priority);

  void Note_Reference() { ++refs; }
  void Release()
  {
    if (!--refs)
      delete this;
  }

  void Print(std::ostream &os) const; //print my contents on the stream
  void Println() const;

protected:
  void insert_item_at_right_place(Am_Priority_List_Item *item);
  Am_Priority_List_Item *take_out_item(Am_Priority_List_Item *item);
  Am_Priority_List_Item *take_out_obj(Am_Object obj);

  // data
  unsigned short number;
  Am_Priority_List_Item *current;
  Am_Priority_List_Item *head;
  Am_Priority_List_Item *tail;
  short refs;
};
_OA_DL_IMPORT std::ostream &operator<<(std::ostream &os,
                                       Am_Priority_List &list);

// Defines Am_Am_Priority_List which can be used to wrap
// Am_Priority_Lists in slots to make them more debuggable

Am_Define_Pointer_Wrapper(Am_Priority_List)

#endif
