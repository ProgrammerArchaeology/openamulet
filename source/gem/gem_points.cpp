/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <stdio.h>
#include <math.h>
#include <iostream>

#include <am_inc.h>
#include <amulet/impl/types_logging.h>
#include <amulet/impl/types_wrapper.h>
#include <amulet/impl/am_point_list.h>
#include <amulet/impl/am_point_array.h>

class Am_Point_Item {
 public:
  Am_Point_Item ()
  { prev = (0L); next = NULL; }
  Am_Point_Item (float x0, float y0)
  { x = x0; y = y0; prev = (0L); next = NULL; }

  float x, y;
  Am_Point_Item* prev;
  Am_Point_Item* next;
};

class Am_Point_List_Data : public Am_Wrapper {
  AM_WRAPPER_DATA_DECL (Am_Point_List)
 public:
  Am_Point_List_Data ();
  Am_Point_List_Data (Am_Point_List_Data* proto);
  Am_Point_List_Data (int *ar, int size);
  Am_Point_List_Data (float *ar, int size);
  ~Am_Point_List_Data ();
  bool operator== (const Am_Point_List_Data&)
    { return false; }
  bool operator== (const Am_Point_List_Data&) const
    { return false; }
  void Add (float x, float y, Am_Add_Position position);
  void Insert (float x, float y, Am_Point_Item* current,
	       Am_Insert_Position position);
  void Set (float x, float y, Am_Point_Item* current);
  void Make_Empty ();
  void Make_Unique (Am_Point_List_Data*& data, Am_Point_Item*& current);
  virtual void Print (std::ostream& os) const;

  unsigned short number;
  Am_Point_Item* head;
  Am_Point_Item* tail;
};

// used to round a floating-point coordinate to nearest integer
// (with .5 rounding up to 1) 
inline int round(float f) { return (int)floor(f + 0.5); }

AM_WRAPPER_DATA_IMPL (Am_Point_List, (this))

Am_Point_List_Data::Am_Point_List_Data ()
{
  head = (0L);
  tail = (0L);
  number = 0;
}
  
Am_Point_List_Data::Am_Point_List_Data (Am_Point_List_Data* proto)
{
  Am_Point_Item* current;
  Am_Point_Item* prev;
  Am_Point_Item* new_item;
  Am_Point_Item** list_position;
  list_position = &head;
  new_item = (0L);
  prev = (0L);
  for (current = proto->head; current; current = current->next) {
    new_item = new Am_Point_Item (current->x, current->y);
    new_item->prev = prev;
    *list_position = new_item;
    prev = new_item;
    list_position = &(new_item->next);
  }
  *list_position = (0L);
  tail = new_item;
  number = proto->number;
}

Am_Point_List_Data::Am_Point_List_Data (int *ar, int size)
{
  Am_Point_Item* prev;
  Am_Point_Item* new_item;
  Am_Point_Item** list_position;
  int i;

  list_position = &head;
  new_item = (0L);
  prev = (0L);

  // truncate size to even number
  size &= ~1;
  for (i = 0; i < size; i+=2) {
    new_item = new Am_Point_Item ((float)ar[i], (float)ar[i+1]);
    new_item->prev = prev;
    *list_position = new_item;
    prev = new_item;
    list_position = &(new_item->next);
  }
  *list_position = (0L);
  tail = new_item;
  number = size / 2;
}

Am_Point_List_Data::Am_Point_List_Data (float *ar, int size)
{
  Am_Point_Item* prev;
  Am_Point_Item* new_item;
  Am_Point_Item** list_position;
  int i;

  list_position = &head;
  new_item = (0L);
  prev = (0L);

  // truncate size to even number
  size &= ~1;
  for (i = 0; i < size; i+=2) {
    new_item = new Am_Point_Item (ar[i], ar[i+1]);
    new_item->prev = prev;
    *list_position = new_item;
    prev = new_item;
    list_position = &(new_item->next);
  }
  *list_position = (0L);
  tail = new_item;
  number = size / 2;
}

Am_Point_List_Data::~Am_Point_List_Data ()
{
  Make_Empty ();
}

void Am_Point_List_Data::Add (float x, float y, Am_Add_Position position)
{
  ++number;
  Am_Point_Item* item = new Am_Point_Item (x, y);
  if (position == Am_HEAD) {
    if (head)
      head->prev = item;
    else
      tail = item;
    item->prev = (0L);
    item->next = head;
    head = item;
  }
  else {
    if (tail)
      tail->next = item;
    else
      head = item;
    item->next = (0L);
    item->prev = tail;
    tail = item;
  }
}
     
void Am_Point_List_Data::Insert (float x, float y, Am_Point_Item* current,
                                 Am_Insert_Position position)
{
  ++number;
  Am_Point_Item* item = new Am_Point_Item (x, y);
  if (current) {
    if (position == Am_BEFORE) {
      item->prev = current->prev;
      item->next = current;
      current->prev = item;
      if (item->prev)
        item->prev->next = item;
      else
        head = item;
    }
    else {
      item->next = current->next;
      item->prev = current;
      current->next = item;
      if (item->next)
        item->next->prev = item;
      else
        tail = item;
    }
  }
  else {
    if (position == Am_BEFORE) {
      item->next = (0L);
      item->prev = tail;
      if (tail)
        tail->next = item;
      else
	head = item;
      tail = item;
    }
    else {
      item->prev = (0L);
      item->next = head;
      if (head)
        head->prev = item;
      else
	tail = item;
      head = item;
    }
  }
}

void Am_Point_List_Data::Set (float x, float y, Am_Point_Item* current)
{
  if (!current)
    Am_Error ("Set called while no item is current\n");
  current->x = x;
  current->y = y;
}

void Am_Point_List_Data::Make_Empty ()
{
  Am_Point_Item* next;
  for (Am_Point_Item* curr = head; curr ; curr = next) {
    next = curr->next;
    curr->next = (0L);
    curr->prev = (0L);
    delete curr;
  }
  head = (0L);
  tail = (0L);
  number = 0;
}

void Am_Point_List_Data::Make_Unique (Am_Point_List_Data*& data,
			              Am_Point_Item*& current)
{
  if (!Is_Unique ()) {
    Am_Point_List_Data* new_data = (Am_Point_List_Data*)data->Make_Unique ();
    if (current) {
      Am_Point_Item* new_current = new_data->head;
      Am_Point_Item* old_current = current->prev;
      while (old_current) {
	old_current = old_current->prev;
	new_current = new_current->next;
      }
      current = new_current;
    }
    data = new_data;
  }
}

void Am_Point_List_Data::Print (std::ostream& os) const
{
  Am_Point_Item *current;
  os << "LIST(" << number << ") [";
  for (current = head; current != tail; current = current->next) {
    os << '(' << current->x << ',' << current->y << ") ";
  }
  if (current) {
    os << '(' << current->x << ',' << current->y << ')';
  }
  os << "]";
}

std::ostream& operator<< (std::ostream& os, Am_Point_List& list)
{
  list.Print(os);
  return os;
}

void Am_Point_List::Print (std::ostream& os) const {
  if (data) data->Print (os);
  else os << "EMPTY LIST";
}

Am_Point_List::Am_Point_List ()
{
  data = (0L);
  item = (0L);
}

	 /*
Am_Point_List::Am_Point_List (Am_Point_List& list)
{
  if (list.data)
    list.data->Note_Reference ();
  data = list.data;
  item = list.item;
}
	   */
Am_Point_List::Am_Point_List (int *ar, int size)
{
  data = new Am_Point_List_Data (ar, size);
  item = (0L);
}

Am_Point_List::Am_Point_List (float *ar, int size)
{
  data = new Am_Point_List_Data (ar, size);
  item = (0L);
}

/////////////////////////////////////////////////////////////////////
//////// BEGIN Specialized AM_WRAPPER_IMPL
//////// This code was manually instantiated from AM_WRAPPER_IMPL,
//////// adding the "item" field to constructors and assignment ops.
//////// Please keep it up-to-date with respect to AM_WRAPPER_IMPL.
////////////////////////////////////////////////////////////////////
Am_Point_List::Am_Point_List (const Am_Point_List& prev)
{
  data = prev.data;
  item = prev.item;
  if (data)
    data->Note_Reference ();
}
Am_Point_List::Am_Point_List (const Am_Value& in_value)
{
  data = (Am_Point_List_Data*)in_value.value.wrapper_value;
  item = (0L);
  if (data) {
    if (Am_Point_List_Data::Am_Point_List_Data_ID () != in_value.type)
      Am_Error ("** Tried to set a Am_Point_List with a non "
                "Am_Point_List wrapper.");
    data->Note_Reference ();
  }
}
Am_Point_List::~Am_Point_List ()
{
  if (data) {
    if (data->Is_Zero ())
      Am_Error ();
    data->Release ();
  }
  data = (0L);
  item = (0L);
}
Am_Point_List& Am_Point_List::operator= (const Am_Point_List& prev)
{
  if (data)
    data->Release ();
  data = prev.data;
  item = prev.item;
  if (data)
    data->Note_Reference ();
  return *this;
}
Am_Point_List& Am_Point_List::operator= (const Am_Value& in_value)
{
  if (data)
    data->Release ();
  data = (Am_Point_List_Data*)in_value.value.wrapper_value;
  item = (0L);
  if (data) {
    if (in_value.type != Am_Point_List_Data::Am_Point_List_Data_ID ()
	&& in_value.type != Am_ZERO)
      Am_Error ("** Tried to set a Am_Point_List with a non "
                "Am_Point_List wrapper.");
    data->Note_Reference ();
  }
  return *this;
}
Am_Point_List& Am_Point_List::operator= (Am_Point_List_Data* in_data)
{
  if (data) data->Release ();
  data = in_data;
  item = (0L);
  return *this;
}
Am_Point_List::operator Am_Wrapper* () const
{
  if (data)
    data->Note_Reference ();
  return data;
}
bool Am_Point_List::Valid () const
{
  return data != (0L);
}
Am_Point_List Am_Point_List::Narrow (Am_Wrapper* in_data)
{
  if (in_data) {
    if (Am_Point_List_Data::Am_Point_List_Data_ID () == in_data->ID ())
      return (Am_Point_List_Data*)in_data;
    else
      Am_Error ("** Tried to set a Am_Point_List with a non "
                "Am_Point_List wrapper.");
  }
  return (Am_Point_List_Data*)(0L);
}
bool Am_Point_List::Test (const Am_Wrapper* in_data)
{
  return (in_data &&
          (in_data->ID () ==
           Am_Point_List_Data::Am_Point_List_Data_ID ()));
}
bool Am_Point_List::Test (const Am_Value& in_value)
{
  return (in_value.value.wrapper_value && 
          (in_value.type ==               
           Am_Point_List_Data::Am_Point_List_Data_ID ()));
}
Am_Value_Type Am_Point_List::Type_ID ()
{
  return Am_Point_List_Data::Am_Point_List_Data_ID ();
}
/////////////////////////////////////////////////////////////////////
//////// END Specialized AM_WRAPPER_IMPL
/////////////////////////////////////////////////////////////////////
     
bool Am_Point_List::operator== (const Am_Point_List& test_value) const
{
  return (test_value.data == data) ||
    (data && test_value.data &&
     ((const Am_Point_List_Data &) (*data) == 
      (const Am_Point_List_Data &) (*test_value.data)));
}

bool Am_Point_List::operator!= (const Am_Point_List& test_value) const
{
  return (test_value.data != data) &&
    (!data || !test_value.data ||
     !((const Am_Point_List_Data &) (*data) == 
      (const Am_Point_List_Data &) (*test_value.data)));
}

unsigned short Am_Point_List::Length () const
{
  if (data)
    return data->number;
  else
    return 0;
}

bool Am_Point_List::Empty () const
{
  if (data)
    return data->number == 0;
  else
    return true;
}

void Am_Point_List::Start ()
{
  if (data)
    item = data->head;
}

void Am_Point_List::End ()
{
  if (data)
    item = data->tail;
}

void Am_Point_List::Prev ()
{
  if (item)
    item = item->prev;
  else if (data)
    item = data->tail;
}

void Am_Point_List::Next ()
{
  if (item)
    item = item->next;
  else if (data)
    item = data->head;
}

bool Am_Point_List::First () const
{
  return (item == (0L));
}

bool Am_Point_List::Last () const
{
  return (item == (0L));
}

void Am_Point_List::Get (int &x, int &y) const
{
  if (!item)
    Am_Error ("** Am_Point_List::Get: no item is current\n");
  x = round(item->x);
  y = round(item->y);
}

void Am_Point_List::Get (float &x, float &y) const
{
  if (!item)
    Am_Error ("** Am_Point_List::Get: no item is current\n");
  x = item->x;
  y = item->y;
}

Am_Point_List& Am_Point_List::Add (float x, float y, 
    Am_Add_Position position, bool unique)     
{                                              
  if (data) {                                  
    if (unique)                                
      data->Make_Unique (data, item);          
  }                                            
  else                                         
    data = new Am_Point_List_Data ();          
  data->Add (x, y, position);                 
  return *this;                                
}

void Am_Point_List::Insert (float x, float y,
    Am_Insert_Position position, bool unique) 
{                                             
  if (data) {                                 
    if (unique)                               
      data->Make_Unique (data, item);         
  }                                           
  else                                        
    data = new Am_Point_List_Data ();         
  data->Insert (x, y, item, position);       
}

void Am_Point_List::Set (float x, float y, bool unique) 
{                                                 
  if (data) {                                     
    if (unique)                                   
      data->Make_Unique (data, item);             
  }                                               
  data->Set (x, y, item);                        
}

void Am_Point_List::Delete (bool unique)
{
  if (item && data) {
    if (unique)
      data->Make_Unique (data, item);
    if (item->prev)
      item->prev->next = item->next;
    else
      data->head = item->next;
    if (item->next)
      item->next->prev = item->prev;
    else
      data->tail = item->prev;
    Am_Point_Item* new_item;
    new_item = item->prev;
    delete item;
    item = new_item;
    --data->number;
  }
  else
    Am_Error ("** Am_Point_List::Delete: no item is current\n");
}

void Am_Point_List::Make_Empty ()
{
  if (data) {
    data->Release ();
    data = (0L);
  }
}

Am_Point_List&  Am_Point_List::Append (Am_Point_List other_list, bool unique) {
  if (data) {
    if (unique) data->Make_Unique (data, item);
  }
  else data = new Am_Point_List_Data ();

  float x, y;
  for(other_list.Start(); !other_list.Last(); other_list.Next()) {
    other_list.Get(x, y);
    Add(x, y, Am_TAIL, false); //only need to make me unique once
  }
  return *this;
}

// extents of points in point list.

void Am_Point_List::Get_Extents (int& min_x, int& min_y,
			     int& max_x, int& max_y) const
{
  if (!data || !data->head) return;

  Am_Point_Item* current = data->head;
  int right, left, bottom, top;
  right  = left = round(current->x);
  bottom = top =  round(current->y);

  for (; current; current = current->next) {
    int x = round(current->x);
    int y = round(current->y);
    left   = left   < x  ? left   : x ;
    right  = right  > x  ? right  : x ;
    top    = top    < y  ? top    : y ;
    bottom = bottom > y  ? bottom : y ;
  }
  min_x = left;  max_x = right;
  min_y = top;   max_y = bottom;
}

// Translate and scale routines for changing top, left, width, height.

// defaults
// origin_x = 0
// origin_y = 0
void Am_Point_List::Scale (float scale_x, float scale_y,
			   int origin_x, int origin_y,
			   bool unique)
{
  if (!data) return;

  if (unique) data->Make_Unique (data, item);          

  Am_Point_Item* current;
  for (current = data->head; current; current = current->next) {
    current->x = ((current->x - origin_x) * scale_x) + origin_x;
    current->y = ((current->y - origin_y) * scale_y) + origin_y;
  }
}

void Am_Point_List::Translate (int offset_x, int offset_y,
			       bool unique)
{
  if (!data) return;

  if (unique) data->Make_Unique (data, item);          

  Am_Point_Item* current;
  for (current = data->head; current; current = current->next) {
    current->x += offset_x;
    current->y += offset_y;
  }
}

