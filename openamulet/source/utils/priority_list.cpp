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

#include <am_inc.h>

#include PRIORITY_LIST__H
#include PRIORITY_LIST__H

void Am_Priority_List::insert_item_at_right_place(Am_Priority_List_Item *item){
  Am_Priority_List_Item *p = head;
  if (!p) {  // list is empty
    head = item;
    tail = item;
    item->next = 0;
    item->prev = 0;
  }
  else { // search for place
    float my_main_priority = item->main_priority;
    int my_second_priority = item->second_priority;
    int my_third_priority = item->third_priority;
    for ( ; p != 0; p = p->next) {
      if (p->main_priority < my_main_priority ||
	  (p->main_priority == my_main_priority &&
	   (p->second_priority < my_second_priority ||
	    (p->second_priority == my_second_priority &&
	     p->third_priority <= my_third_priority)))) {
	// then put item before p
	if (p->prev) {
	  p->prev->next = item;
	  item->prev = p->prev;
	}
	else { // p is the first in list
	  head = item;
	  item->prev = 0;
	}
	item->next = p;
	p->prev = item;
	return;
      }
    }// end for
    //if get here, then item should go last in the list
    item->next = 0;
    item->prev = tail;
    tail->next = item;
    tail = item;
  }
}

Am_Priority_List* Am_Priority_List::Create() {
  Am_Priority_List* l = new Am_Priority_List;
  l->number = 0;
  l->head = 0;
  l->tail = 0;
  l->current = 0;
  l->refs = 1;
  return l;
}

// destroys the entire list
void Am_Priority_List::Remove() {
  this->Make_Empty();
  delete this;
}

// assumes not already in the list.  Add in priority order
Am_Priority_List* Am_Priority_List::Add (Am_Object obj, float main_priority,
					 int second_priority,
					 int third_priority) {
  Am_Priority_List_Item* item = new Am_Priority_List_Item;
  item->obj = obj;
  item->main_priority = main_priority;
  item->second_priority = second_priority;
  item->third_priority = third_priority;
  this->insert_item_at_right_place(item);
  number++;
  return this;
}

bool Am_Priority_List::Member (Am_Object obj) {
  Am_Priority_List_Item* old_current = current;
  for ( ; current != 0; current = current->next) {
    if (current->obj == obj) return true;
  }
  current = old_current;
  return false;
}

Am_Priority_List_Item* Am_Priority_List::take_out_item(Am_Priority_List_Item*
						       item) {
  if (current==item) {
    current = item->prev; // go to prev so when do next on the next
			  // iteration, will do the next item in the list
  }
  
  if (item->prev) item->prev->next = item->next;
  else head = item->next;

  if (item->next) item->next->prev = item->prev;
  else tail = item->prev;
  return item;
}
    

Am_Priority_List_Item* Am_Priority_List::take_out_obj(Am_Object obj) {
  Am_Priority_List_Item* p = head;
  for ( ; p != 0; p = p->next) {
    if (p->obj == obj) return this->take_out_item(p);
  }
  return 0;
}
    

void Am_Priority_List::Change_Main_Priority (Am_Object obj,
					     float main_priority) {
  Am_Priority_List_Item* item = this->take_out_obj(obj);
  if(!item) {
    Am_ERRORO("Change_Main_Priority for " << obj << " but not on list "
	      << *this, obj, 0);
  }
  item->main_priority = main_priority;
  this->insert_item_at_right_place(item);
}

void Am_Priority_List::Change_Second_Third_Priority (Am_Object obj,
						     int second_priority,
						     int third_priority) {
  Am_Priority_List_Item* item = this->take_out_obj(obj);
  if(!item) {
    Am_ERRORO("Change_Second_Third_Priority for " << obj << " but not on list "
	      << *this, obj, 0);
  }
  item->second_priority = second_priority;
  item->third_priority = third_priority;
  this->insert_item_at_right_place(item);
}

Am_Object Am_Priority_List::Get () {
  if (current) return current->obj;
  else {
    Am_Error("Call to Get on priority list but no current item");
    return (0L);
  }
}

Am_Object Am_Priority_List::Get (float &main_priority,
				  int &second_priority,
				  int &third_priority) {
  if (current) {
    main_priority = current->main_priority;
    second_priority = current->second_priority;
    third_priority = current->third_priority;
    return current->obj;
  }
  else {
    Am_Error("Call to Get on priority list but no current item");
    return (0L);
  }
}

void Am_Priority_List::Delete (Am_Object obj) {
  this->Start();
  if (this->Member(obj)) this->Delete();
}
    

void Am_Priority_List::Delete () {
  if (current) {
    Am_Priority_List_Item* item = current;
    this->take_out_item(item); //will adjust current pointer
    delete item;
  }
  else Am_Error("Call to Delete on priority list but no current item");
}

void Am_Priority_List::Make_Empty () {
  Am_Priority_List_Item* item;
  Am_Priority_List_Item* p = head;
  while(p) {
    item = p;
    p = p->next;
    delete item;
  }
  head = 0;
  tail = 0;
  current = 0;
}
    
void Am_Priority_List::Start () { // Make first element be current.
  current = head;
}

void Am_Priority_List::End () {   // Make last element be current.
  current = tail;
}

void Am_Priority_List::Prev () {  // Make previous element be current.
  if (current) current = current->prev;
  else current = tail;
}

void Am_Priority_List::Next () {  // Make next element be current.
  if (current) current = current->next;
  else current = head;
}

// Returns true when current element passes the first element.
bool Am_Priority_List::First () {
  return (current == 0);
}

// Returns true when current element passes the last element.
bool Am_Priority_List::Last () {
  return (current == 0);
}

//other_list may be null
bool Am_Priority_List::Two_List_Get_Next(Am_Priority_List* other_list,
					 Am_Object &inter,
					 float &priority) {
  if (current) {
    if (other_list && other_list->current) {
      //then have to check the priorities
      if (other_list->current->main_priority < current->main_priority ||
	  (other_list->current->main_priority == current->main_priority &&
	   (other_list->current->second_priority < current->second_priority ||
	    (other_list->current->second_priority==current->second_priority &&
	     other_list->current->third_priority<=current->third_priority)))) {
	//then other is lower, I am higher
	inter = current->obj;
	priority = current->main_priority;
	current = current->next;
      }
      else { //other is higher
	inter = other_list->current->obj;
	priority = other_list->current->main_priority;
	other_list->current = other_list->current->next;
      }
    }
    else { // current but not other current, return my next item
      inter = current->obj;
      priority = current->main_priority;
      current = current->next;
    }
  }
  else { // not current
    if (other_list && other_list->current) {
      inter = other_list->current->obj;
      priority = other_list->current->main_priority;
      other_list->current = other_list->current->next;
    }
    else // neither has current value
      return false;
  }
  //if get here, then OK
  return true;
}

void print_current(std::ostream& os, Am_Priority_List_Item *current) {
  os << "(" << current->main_priority << ","
     << current->second_priority << ","
     << current->third_priority << ")" << current->obj << " ";
}

void Am_Priority_List::Print (std::ostream& os) const
{
  Am_Priority_List_Item *current;
  os << "PRIORITY_LIST(" << (int)number << ") [";
  Am_Value v;
  for (current = head; current != tail; current = current->next) {
    print_current(os, current);
  }
  if (current) {
    print_current(os, current);
  }
  os << "]";
}

std::ostream& operator<< (std::ostream& os, Am_Priority_List& list)
{
  list.Print(os);
  return os;
}

class Am_Priority_List_Type_Support_Class : public Am_Type_Support {
  void Print (std::ostream& os, const Am_Value& val) const {
    Am_Priority_List* list = Am_Am_Priority_List (val).value;
    list->Print(os);
  }
} am_priority_list_support;

Am_Define_Pointer_Wrapper_Impl(Am_Priority_List, &am_priority_list_support);
