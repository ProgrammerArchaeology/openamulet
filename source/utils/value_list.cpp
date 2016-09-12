/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <string.h>

#include <am_inc.h>

#include AM_IO__H

#include VALUE_LIST__H

#include STDVALUE__H

class Am_List_Item : public Am_Value {
 public:
  Am_List_Item ()
  { prev = (0L); next = NULL; }
  Am_List_Item& operator= (const Am_Value& value)
  {
    *(Am_Value*)this = value;
    return *this;
  }
  Am_List_Item* prev;
  Am_List_Item* next;
};

class Am_Value_List_Data : public Am_Wrapper {
  AM_WRAPPER_DATA_DECL (Am_Value_List)
 public:
  Am_Value_List_Data ();
  Am_Value_List_Data (Am_Value_List_Data* proto);
  ~Am_Value_List_Data ();
  bool operator== (const Am_Value_List_Data& test_value) const;
  void Add (const Am_Value& value, Am_Add_Position position);
  void Insert (const Am_Value& value, Am_List_Item* current,
	       Am_Insert_Position position);
  void Set (const Am_Value& value, Am_List_Item* current);
  void Make_Empty ();
  void Make_Unique (Am_Value_List_Data*& data, Am_List_Item*& current);
  virtual void Print (std::ostream& os) const;

  unsigned short number;
  Am_List_Item* head;
  Am_List_Item* tail;
};

AM_WRAPPER_DATA_IMPL_ID (Am_Value_List, (this), Am_VALUE_LIST)

Am_Value_List_Data::Am_Value_List_Data ()
{
  head = (0L);
  tail = (0L);
  number = 0;
}
  
Am_Value_List_Data::Am_Value_List_Data (Am_Value_List_Data* proto)
{
  Am_List_Item* current;
  Am_List_Item* prev;
  Am_List_Item* new_item;
  Am_List_Item** list_position;
  list_position = &head;
  new_item = (0L);
  prev = (0L);
  for (current = proto->head; current; current = current->next) {
    new_item = new Am_List_Item;
    *new_item = *current;
    new_item->prev = prev;
    *list_position = new_item;
    prev = new_item;
    list_position = &(new_item->next);
  }
  *list_position = (0L);
  tail = new_item;
  number = proto->number;
}

Am_Value_List_Data::~Am_Value_List_Data ()
{
  Make_Empty ();
}

bool Am_Value_List_Data::operator== (const Am_Value_List_Data& test_value)
     const
{
  Am_List_Item* curr1 = head;
  Am_List_Item* curr2 = test_value.head;
  while ((curr1 != (0L)) && (curr2 != NULL)) {
    if (!(*curr1 == *curr2))
      return false;
    curr1 = curr1->next;
    curr2 = curr2->next;
  }
  return (curr1 == (0L)) && (curr2 == NULL);
}

void Am_Value_List_Data::Add (const Am_Value& value, Am_Add_Position position)
{
  ++number;
  Am_List_Item* item = new Am_List_Item;
  *item = value;
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
     
void Am_Value_List_Data::Insert (const Am_Value& value, Am_List_Item* current,
                                 Am_Insert_Position position)
{
  ++number;
  Am_List_Item* item = new Am_List_Item;
  *item = value;
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

void Am_Value_List_Data::Set (const Am_Value& value, Am_List_Item* current)
{
  if (!current)
    Am_Error ("Set called while no item is current\n");
  *current = value;
}

void Am_Value_List_Data::Make_Empty ()
{
  Am_List_Item* next;
  for (Am_List_Item* curr = head; curr ; curr = next) {
    next = curr->next;
    curr->next = (0L);
    curr->prev = (0L);
    delete curr;
  }
  head = (0L);
  tail = (0L);
  number = 0;
}

void Am_Value_List_Data::Make_Unique (Am_Value_List_Data*& data,
			              Am_List_Item*& current)
{
  if (!Is_Unique ()) {
    Am_Value_List_Data* new_data = (Am_Value_List_Data*)data->Make_Unique ();
    if (current) {
      Am_List_Item* new_current = new_data->head;
      Am_List_Item* old_current = current->prev;
      while (old_current) {
	old_current = old_current->prev;
	new_current = new_current->next;
      }
      current = new_current;
    }
    data = new_data;
  }
}

void Am_Value_List_Data::Print (std::ostream& os) const
{
  Am_List_Item *current;
  os << "LIST(" << (int)number << ") [";
  Am_Value v;
  for (current = head; current != tail; current = current->next) {
    os << *current << ", ";
  }
  if (current) {
    os << *current;
  }
  os << "]";
}

std::ostream& operator<< (std::ostream& os, const Am_Value_List& list)
{
  list.Print(os);
  return os;
}

Am_Value_List::Am_Value_List ()
{
  data = (0L);
  item = (0L);
}

Am_Value_List::Am_Value_List (const Am_Value_List& list)
{
  data = list.data;
  item = list.item;
  if (data)
    data->Note_Reference ();
}

Am_Value_List::Am_Value_List (const Am_Value& list_value)
{
  data = (Am_Value_List_Data*)list_value.value.wrapper_value;
  if (data) {
    if (Am_Value_List_Data::Am_Value_List_Data_ID () != list_value.type)
      Am_ERROR ("Tried to set a Am_Value_List with a non list value = "
		<< list_value);
    data->Note_Reference ();
  }
  item = (0L);
}

Am_Value_List::~Am_Value_List ()
{
  if (data) {
    if (data->Is_Zero ())
      Am_Error ();
    data->Release ();
  }
  data = (0L);
  item = (0L);
}
     
Am_Value_List& Am_Value_List::operator= (const Am_Value_List& list)
{
  Am_Value_List_Data* old_data = data;
  data = list.data;
  item = list.item;
  if (data)
    data->Note_Reference ();
  if (old_data)
    old_data->Release ();
  return *this;
}

Am_Value_List& Am_Value_List::operator= (const Am_Value& list_value)
{
  Am_Value_List_Data* old_data = data;
  data = (Am_Value_List_Data*)list_value.value.wrapper_value;
  if (data) {
    if (Am_Value_List_Data::Am_Value_List_Data_ID () != list_value.type
	&& list_value.type != Am_ZERO) {
      Am_ERROR ("Tried to set a Am_Value_List with a non list value = " <<
		list_value);
    }
    data->Note_Reference ();
  }
  if (old_data)
    old_data->Release ();
  item = (0L);
  return *this;
}

Am_Value_List::operator Am_Wrapper* () const
{
  if (data)
    data->Note_Reference ();
  return data;
}

bool Am_Value_List::Valid () const
{
  return !Empty();
}

bool Am_Value_List::Exists () const
{
  return data != (0L);
}

bool Am_Value_List::Test (const Am_Wrapper* in_data)
{
  if (in_data)
    return in_data->ID () ==
      Am_Value_List_Data::Am_Value_List_Data_ID ();
  else
    return false;
}
bool Am_Value_List::Test (const Am_Value& in_value)
{
  return (in_value.value.wrapper_value &&
          (in_value.type == Am_Value_List_Data::Am_Value_List_Data_ID ()));
}

Am_Value_Type Am_Value_List::Type_ID ()
{
  return Am_Value_List_Data::Am_Value_List_Data_ID ();
}

bool Am_Value_List::operator== (const Am_Value_List& test_value) const
{
  return (test_value.data == data) ||
    (data && test_value.data &&
     ((const Am_Value_List_Data &) (*data) == 
      (const Am_Value_List_Data &) (*test_value.data)));
}

bool Am_Value_List::operator!= (const Am_Value_List& test_value) const
{
  return (test_value.data != data) &&
    (!data || !test_value.data ||
     !((const Am_Value_List_Data &) (*data) == 
      (const Am_Value_List_Data &) (*test_value.data)));
}

unsigned short Am_Value_List::Length () const
{
  if (data)
    return data->number;
  else
    return 0;
}

bool Am_Value_List::Empty () const
{
  if (data)
    return data->number == 0;
  else
    return true;
}

void Am_Value_List::Start ()
{
  if (data)
    item = data->head;
}

void Am_Value_List::End ()
{
  if (data)
    item = data->tail;
}

void Am_Value_List::Prev ()
{
  if (item)
    item = item->prev;
  else if (data)
    item = data->tail;
}

void Am_Value_List::Next ()
{
  if (item)
    item = item->next;
  else if (data)
    item = data->head;
}

bool Am_Value_List::First () const
{
  return (item == (0L));
}

bool Am_Value_List::Last () const
{
  return (item == (0L));
}

Am_Value& Am_Value_List::Get () const
{
  if (!item)
    Am_Error ("** Am_Value_List::Get: no item is current\n");
  return *item;
}

Am_Value& Am_Value_List::Get_First () const {
  if (!data)
    Am_Error ("** Am_Value_List::Get_First: list is empty");
  return *(data->head);
}

//has to return Am_Value instead of Am_Value& like get because we are
//deallocating the cell that holds the value
Am_Value Am_Value_List::Pop (bool unique) {
  if (!Valid()) Am_Error("** Pop on empty list");
  Start();
  Am_Value v = Get();
  Delete(unique);
  return v;
}

Am_Value_Type Am_Value_List::Get_Type () const
{
  if (!item)
    Am_Error ("** Am_Value_List::Get_Type: no item is current\n");
  return item->type;
}

Am_Value& Am_Value_List::Get_Nth (int index) const
{
  if (!data) Am_Error("Value list is empty in Get_Nth");
  Am_List_Item* this_item = data->head;
  if (index >= data->number)
    Am_ERROR("Value list is too short, len = " << data->number
	     << " but requested " << index);
  for (int i = 0; i < index; i++) {
    this_item = this_item->next;
  }
  return *this_item;
}

int Am_Value_List::Nth () const {
  if (!item)
    Am_Error ("Am_Value_List::Nth no item is current");
  int i = 0;
  Am_List_Item* this_item = data->head;
  while (this_item) {
    if (this_item == item) return i;
    i++;
    this_item = this_item->next;
  }
  Am_Error("Lost current item from list");
  return -1;
}

void Am_Value_List::Move_Nth (int index)
{
  if (!data)
    Am_Error ("Value list is empty in Move_Nth");
  Am_List_Item* this_item = data->head;
  if (index >= data->number)
    Am_ERROR ("Value list is too short, len = " << data->number
	      << " but requested " << index);
  for (int i = 0; i < index; i++) {
    this_item = this_item->next;
  }
  item = this_item;
}

#define Make_Member(type)               \
bool Am_Value_List::Member (type value) \
{                                       \
  Am_List_Item* old_item = item;        \
  while (item) {                        \
    if (*item == value)                 \
      return true;                      \
    item = item->next;                  \
  }                                     \
  item = old_item;                      \
  return false;                         \
}

bool Am_Value_List::Member (Am_Wrapper* value)
{
  Am_List_Item* old_item = item;
  while (item) {
    if (*item == value) {
      if (value) value->Release ();
      return true;
    }
    item = item->next;
  }
  item = old_item;
  if (value) value->Release ();
  return false;
}

Make_Member (Am_Ptr)
Make_Member (int)
Make_Member (long)
Make_Member (bool)
Make_Member (float)
Make_Member (double)
Make_Member (char)
Make_Member (const char*)
Make_Member (const Am_String&)
Make_Member (Am_Generic_Procedure*)
Make_Member (Am_Method_Wrapper*)
Make_Member (const Am_Value&)

#undef Make_Member

#define Make_Add(type)                         \
Am_Value_List& Am_Value_List::Add (type value, \
    Am_Add_Position position, bool unique)     \
{                                              \
  if (data) {                                  \
    if (unique)                                \
      data->Make_Unique (data, item);          \
  }                                            \
  else                                         \
    data = new Am_Value_List_Data ();          \
  Am_Value store (value);                      \
  data->Add (store, position);                 \
  return *this;                                \
}

Make_Add (Am_Wrapper*)
Make_Add (Am_Ptr)
Make_Add (int)
Make_Add (long)
Make_Add (bool)
Make_Add (float)
Make_Add (double)
Make_Add (char)
Make_Add (const char*)
Make_Add (const Am_String&)
Make_Add (Am_Generic_Procedure*)
Make_Add (Am_Method_Wrapper*)

bool Am_Value_List::Start_Member (const Am_Value& value) {
  Start();
  return Member(value);
}
bool Am_Value_List::Start_Member (Am_Wrapper*     value) {
  Start();
  return Member(value);
}


Am_Value_List& Am_Value_List::Add (const Am_Value& value,
    Am_Add_Position position, bool unique)
{
  if (data) {
    if (unique)
      data->Make_Unique (data, item);
  }
  else
    data = new Am_Value_List_Data ();
  data->Add (value, position);
  return *this;
}

#undef Make_Add

#define Make_Insert(type)                     \
void Am_Value_List::Insert (type value,       \
    Am_Insert_Position position, bool unique) \
{                                             \
  if (data) {                                 \
    if (unique)                               \
      data->Make_Unique (data, item);         \
  }                                           \
  else                                        \
    data = new Am_Value_List_Data ();         \
  Am_Value store (value);                     \
  data->Insert (store, item, position);       \
}

Make_Insert (Am_Wrapper*)
Make_Insert (Am_Ptr)
Make_Insert (int)
Make_Insert (long)
Make_Insert (bool)
Make_Insert (float)
Make_Insert (double)
Make_Insert (char)
Make_Insert (const char*)
Make_Insert (const Am_String&)
Make_Insert (Am_Generic_Procedure*)
Make_Insert (Am_Method_Wrapper*)

void Am_Value_List::Insert (const Am_Value& value,
    Am_Insert_Position position, bool unique)
{
  if (data) {
    if (unique)
      data->Make_Unique (data, item);
  }
  else
    data = new Am_Value_List_Data ();
  data->Insert (value, item, position);
}

#undef Make_Insert

#define Make_Set(type)                            \
void Am_Value_List::Set (type value, bool unique) \
{                                                 \
  if (data) {                                     \
    if (unique)                                   \
      data->Make_Unique (data, item);             \
  }                                               \
  Am_Value store (value);                         \
  data->Set (store, item);                        \
}

Make_Set (Am_Wrapper*)
Make_Set (Am_Ptr)
Make_Set (int)
Make_Set (long)
Make_Set (bool)
Make_Set (float)
Make_Set (double)
Make_Set (char)
Make_Set (const char*)
Make_Set (const Am_String&)
Make_Set (Am_Generic_Procedure*)
Make_Set (Am_Method_Wrapper*)

void Am_Value_List::Set (const Am_Value& value, bool unique)
{
  if (data) {
    if (unique)
      data->Make_Unique (data, item);
  }
  data->Set (value, item);
}

#undef Make_Set

Am_Value_List& Am_Value_List::Append (Am_Value_List other_list, bool unique)
{
  if (data) {
    if (unique) data->Make_Unique (data, item);
  }
  else data = new Am_Value_List_Data ();

  for(other_list.Start(); !other_list.Last(); other_list.Next())
    Add(other_list.Get(), Am_TAIL, false); //only need to make me unique once
  return *this;
}

Am_Value_List& Am_Value_List::Append_New (Am_Value_List& other_value,
					  bool unique) {
  if (data) {
    if (unique) data->Make_Unique (data, item);
  }
  else data = new Am_Value_List_Data ();
  Am_Value v;
  for(other_value.Start(); !other_value.Last(); other_value.Next()) {
    v = other_value.Get();
    if (!Start_Member(v))
      Add(v, Am_TAIL, false); //only need to make me unique once
  }
  return *this;
}
  

Am_Value_List& Am_Value_List::Append_New (Am_Value other_value, bool unique) {
  if (data) {
    if (unique) data->Make_Unique (data, item);
  }
  else data = new Am_Value_List_Data ();

  if (Am_Value_List::Test(other_value)) {
    Am_Value_List other_list = other_value;
    Append_New(other_list, false);
  }
  else {
    if (other_value.Valid()) {
      if (!Start_Member(other_value))
	Add(other_value, Am_TAIL, false); //only need to make me unique once
    }
  }
  return *this;
}

void Am_Value_List::Delete (bool unique)
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
    Am_List_Item* new_item;
    new_item = item->prev;
    delete item;
    item = new_item;
    --data->number;
  }
  else
    Am_Error ("** Am_Value_List::Delete: no item is current\n");
}

void Am_Value_List::Set_Difference(Am_Value_List other_list, bool unique) {
  if (data) {
    if (unique) data->Make_Unique ();
  }
  else Am_Error("Value Lists is not initialize");
  for (other_list.Start(); !other_list.Last(); other_list.Next()) {
    if (Start_Member(other_list.Get()))
      Delete(false);
  }
}

Am_Value_List Am_Value_List::Copy ()
{
  if (data)
    return Am_Value_List (new Am_Value_List_Data (data));
  else 
    return Am_No_Value_List;
}

Am_Value_List Am_Value_List::Recursive_Copy ()
{
  if (data) {
    Am_Value_List_Data* new_data = new Am_Value_List_Data (data);
    Am_List_Item* this_item = new_data->head;
    while (this_item) {
      if (this_item->type == Am_Value_List::Type_ID()) {
	Am_Value_List this_list = (*this_item);
	this_list = this_list.Recursive_Copy();
	Am_Value v;
	v = this_list;
	*this_item = v;
      }
      this_item = this_item->next;
    }
    return Am_Value_List (new_data);
  }
  else 
    return Am_No_Value_List;
}

void Am_Value_List::Make_Empty ()
{
  if (data) {
    data->Release ();
    data = (0L);
  }
}

Am_Value_List Am_Value_List::Empty_List ()
{
  return Am_Value_List (new Am_Value_List_Data ());
}

//////////////////////////////////
//Am_Value_List doesn't use AM_WRAPPER_IMPL, so repeat all the
//printing functions here

const char * Am_Value_List::To_String() const {
  if (data) return data->To_String();
  else return (0L);
}

Am_Value Am_Value_List::From_String (const char* string) {
  if (data) return data->From_String(string);
  else return Am_No_Value;
}
void Am_Value_List::Print(std::ostream& out) const {
  if (data) data->Print(out);
  else out << "(Am_Value_List)(0L)";
}

Am_Value_List Am_No_Value_List;

////////////////////////////////// Association Lists //////////////////////

AM_WRAPPER_IMPL (Am_Assoc)
AM_WRAPPER_DATA_IMPL (Am_Assoc, (this))

void Am_Assoc_Data::Print(std::ostream& out) const
{
  out << To_String();
  return;
}

std::ostream& operator<< (std::ostream& os, const Am_Assoc& as) {
  as.Print(os);
  return os;
}

Am_Value Am_Value_List::Assoc_2 (const Am_Value& value_1) {
  if (!data) return Am_No_Value;
  Am_List_Item* this_item = data->head;
  Am_Assoc_Data* assoc;
  while (this_item) {
    assoc = Am_Assoc_Data::Narrow(this_item->value.wrapper_value);
    if (assoc->value_1 == value_1) {
      item = this_item; //move pointer to found item
      return assoc->value_2;
    }
    this_item = this_item->next;
  }
  return Am_No_Value;
}

Am_Value Am_Value_List::Assoc_2 (Am_Wrapper* value_1) {
  Am_Value v = value_1;
  return Assoc_2(v);
}

Am_Value Am_Value_List::Assoc_1 (const Am_Value& value_2) {
  if (!data) return Am_No_Value;
  Am_List_Item* this_item = data->head;
  Am_Assoc_Data* assoc;
  while (this_item) {
    assoc = Am_Assoc_Data::Narrow(this_item->value.wrapper_value);
    if (assoc->value_2 == value_2) {
      item = this_item; //move pointer to found item
      return assoc->value_1;
    }
    this_item = this_item->next;
  }
  return Am_No_Value;
}

Am_Value Am_Value_List::Assoc_1 (Am_Wrapper* value_2) {
  Am_Value v = value_2;
  return Assoc_1(v);
}

// returns true of v1 == v2.  Or, if either v1 or v2 is a list
// and the other isn't a list, and the list contains a single value
// which is == to the other value, then returns true.
bool Am_Value_List::List_Or_Value_Equal(const Am_Value& v1,
					const Am_Value& v2) {
  if (v1 == v2) return true;
  Am_Value_List l;
  if (Am_Value_List::Test(v1)) {
    l = v1;
    if (l.data && l.data->number == 1 && *(l.data->head) == v2 )
      return true;
  }
  if (Am_Value_List::Test(v2)) {
    l = v2;
    if (l.data && l.data->number == 1 && *(l.data->head) == v1 )
      return true;
  }
  return false;
}
    
     
