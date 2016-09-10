/* ****************************** -*-c++-*- *******************************/
/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#ifndef VALUE_LIST_H
#define VALUE_LIST_H

#include <am_inc.h>
#include AM_VALUE__H
#include "amulet/impl/types_logging.h"
#include "amulet/impl/types_wrapper.h"
#include "amulet/impl/types_misc.h"

class Am_Value_List_Data;

class Am_List_Item;

//internal
class _OA_DL_CLASSIMPORT Am_Assoc_Data : public Am_Wrapper {
  Am_WRAPPER_DATA_DECL(Am_Assoc)
 public:
  Am_Value value_1, value_2;

  Am_Assoc_Data ()
  {value_1 = Am_No_Value; value_2 = Am_No_Value;}
  Am_Assoc_Data (const Am_Value& val1, const Am_Value& val2)
  {value_1 = val1; value_2 = val2;}
  Am_Assoc_Data (Am_Assoc_Data* proto) //required by wrapper
  {value_1 = proto->value_1; value_2 = proto->value_2;}
  bool operator== (const Am_Assoc_Data& test_value) const
  { return value_1 == test_value.value_1 && value_2 == test_value.value_2; }
  void Print (std::ostream& out) const; // to print out the contents
};

//use this for association lists
class _OA_DL_CLASSIMPORT Am_Assoc {
  Am_WRAPPER_DECL (Am_Assoc)
 public:
  
  Am_Assoc () {data = (0L);}
  Am_Assoc (const Am_Value& val1, const Am_Value& val2)
  { data = new Am_Assoc_Data(val1, val2);}
  Am_Assoc (Am_Wrapper* val1, Am_Wrapper* val2)
  { data = new Am_Assoc_Data(val1, val2);}
  Am_Assoc (const Am_Value& val1, Am_Wrapper* val2)
  { data = new Am_Assoc_Data(val1, val2);}
  Am_Assoc (Am_Wrapper* val1, const Am_Value& val2)
  { data = new Am_Assoc_Data(val1, val2);}

  Am_Value Value_1()
   { if (data) return data->value_1; else Am_Error("Accessing (0L) Assoc");
     return Am_No_Value;}
  Am_Value Value_2()
   { if (data) return data->value_2; else Am_Error("Accessing (0L) Assoc");
     return Am_No_Value;}
};
_OA_DL_IMPORT std::ostream& operator<< (std::ostream& os, const Am_Assoc& list);


class _OA_DL_CLASSIMPORT Am_Value_List {
  Am_WRAPPER_DECL (Am_Value_List)
 public:
  Am_Value_List ();

  bool Exists () const;

  bool operator== (const Am_Value_List& test_list) const;
  bool operator!= (const Am_Value_List& test_list) const;

  // Returns the number of elements in the list.
  unsigned short Length () const;

  // Returns whether list is empty or not.
  bool Empty () const;

  void Start (); // Make first element be current.
  void End ();   // Make last element be current.

  void Prev ();  // Make previous element be current.
  void Next ();  // Make next element be current.

  // Returns TRUE when current element passes the first element.
  bool First () const;

  // Returns TRUE when current element passes the last element.
  bool Last () const;

  // Retrieve the value of the current element.  Error if no element is
  // current.
  Am_Value& Get () const;

  //returns the first item of the list, ignoring the current element.
  // error if no first element
  Am_Value& Get_First () const;

  // Retrieve the type of the current element.  Error if no element is
  // current.
  Am_Value_Type Get_Type () const;

  // Retrieves the value of the index'th item of the list, starting from
  // the front, with 0 being the first item.  Raises error if list is not
  // at least index+1 long.  Moves the pointer to the Nth element
  Am_Value& Get_Nth (int index) const;

  //returns the index of the current element. (Not particularly
  //efficient since has to count from the beginning.)
  int Nth () const;

  // Moves current pointer to the index'th item from the beginning
  // (ignores the current place).  
  // Raises error if list is not at least index+1 long.
  void Move_Nth (int index);

  // Returns TRUE is given value is a member of the list.  The search begins
  // at whatever element is current.  If the search is successful, the current
  // element will point to the successful member.  If the search is not
  // successful the current element pointer will be left stationary.
  bool Member (Am_Wrapper*     value);
  bool Member (Am_Ptr          value);
  bool Member (int             value);
  bool Member (long            value);
#if !defined(NEED_BOOL)
  bool Member (bool            value);
#endif
  bool Member (float           value);
  bool Member (double          value);
  bool Member (char            value);
  bool Member (const char*     value);
  bool Member (const Am_String& value);
  bool Member (Am_Method_Wrapper* value);
  bool Member (Am_Generic_Procedure* value);
  bool Member (const Am_Value& value);

  //calls Start() then calls Member()
  bool Start_Member (const Am_Value& value);
  bool Start_Member (Am_Wrapper*     value);

  // Retrieve the value of the first element.  
  // Error if list is empty.  Removes the first element from the list.
  // Same as Start(); Get(); Delete();  Changes the pointer.
  Am_Value Pop (bool unique = true);
  
  Am_Value_List& Push (Am_Wrapper* value, bool unique = true)
     { return Add(value, Am_HEAD, unique); }
  Am_Value_List& Push (const Am_Value& value, bool unique = true)
     { return Add(value, Am_HEAD, unique); }

  //look for value_1 as the first item of an assoc in the list, and if
  //found, then returns the corresponding value_2.  Error if any items
  //in list are not Am_Assoc's.  Returns Am_No_Value if not found.
  // Moves the position pointer of list to the found Assoc (in case
  // want to do a Delete), but DOES NOT START from the position
  // (unlike Member)--always starts searching from the beginning
  Am_Value Assoc_2 (const Am_Value& value_1);
  Am_Value Assoc_2 (Am_Wrapper*     value_1);
  //look for value_2 as the second item of an assoc in the list, and if
  //found, then returns the corresponding value_1.
  Am_Value Assoc_1 (const Am_Value& value_2);
  Am_Value Assoc_1 (Am_Wrapper*     value_2);
  
  // Add puts the new element at the head or tail of the list.
  Am_Value_List& Add (Am_Wrapper* value, Am_Add_Position position = Am_TAIL,
                      bool unique = true);
  Am_Value_List& Add (Am_Ptr value, Am_Add_Position position = Am_TAIL,
                      bool unique = true);
  Am_Value_List& Add (int value, Am_Add_Position position = Am_TAIL,
                      bool unique = true);
  Am_Value_List& Add (long value, Am_Add_Position position = Am_TAIL,
                      bool unique = true);
#if !defined(NEED_BOOL)
  Am_Value_List& Add (bool value, Am_Add_Position position = Am_TAIL,
                      bool unique = true);
#endif
  Am_Value_List& Add (float value, Am_Add_Position position = Am_TAIL,
                      bool unique = true);
  Am_Value_List& Add (double value, Am_Add_Position position = Am_TAIL,
                      bool unique = true);
  Am_Value_List& Add (char value, Am_Add_Position position = Am_TAIL,
                      bool unique = true);
  Am_Value_List& Add (const char* value, Am_Add_Position position = Am_TAIL,
                      bool unique = true);
  Am_Value_List& Add (const Am_String& value,
                      Am_Add_Position position = Am_TAIL, bool unique = true);
  Am_Value_List& Add (Am_Method_Wrapper* value,
                      Am_Add_Position position = Am_TAIL, bool unique = true);
  Am_Value_List& Add (Am_Generic_Procedure* value,
                      Am_Add_Position position = Am_TAIL, bool unique = true);
  Am_Value_List& Add (const Am_Value& value,
                      Am_Add_Position position = Am_TAIL, bool unique = true);

  // Insert puts the new element before or after the current position
  // in the list.  The current position is set by using the Start, End, Next,
  // and Prev methods.
  void Insert (Am_Wrapper*     value, Am_Insert_Position position,
               bool unique = true);
  void Insert (Am_Ptr          value, Am_Insert_Position position,
               bool unique = true);
  void Insert (int             value, Am_Insert_Position position,
               bool unique = true);
  void Insert (long            value, Am_Insert_Position position,
               bool unique = true);
#if !defined(NEED_BOOL)
  void Insert (bool            value, Am_Insert_Position position,
               bool unique = true);
#endif
  void Insert (float           value, Am_Insert_Position position,
               bool unique = true);
  void Insert (double          value, Am_Insert_Position position,
               bool unique = true);
  void Insert (char            value, Am_Insert_Position position,
               bool unique = true);
  void Insert (const char*     value, Am_Insert_Position position,
               bool unique = true);
  void Insert (const Am_String& value, Am_Insert_Position position,
               bool unique = true);
  void Insert (Am_Method_Wrapper* value, Am_Insert_Position position,
               bool unique = true);
  void Insert (Am_Generic_Procedure* value, Am_Insert_Position position,
               bool unique = true);
  void Insert (const Am_Value& value, Am_Insert_Position position,
               bool unique = true);

  // Change the value of the current element.  Error if no element is current.
  void Set (Am_Wrapper*     value, bool unique = true);
  void Set (Am_Ptr          value, bool unique = true);
  void Set (int             value, bool unique = true);
  void Set (long            value, bool unique = true);
#if !defined(NEED_BOOL)
  void Set (bool            value, bool unique = true);
#endif
  void Set (float           value, bool unique = true);
  void Set (double          value, bool unique = true);
  void Set (char            value, bool unique = true);
  void Set (const char*     value, bool unique = true);
  void Set (const Am_String& value, bool unique = true);
  void Set (Am_Method_Wrapper* value, bool unique = true);
  void Set (Am_Generic_Procedure* value, bool unique = true);
  void Set (const Am_Value&       value, bool unique = true);

  //adds items in other_list to my end.  Returns me (this) (so can be cascaded)
  Am_Value_List& Append (Am_Value_List other_list, bool unique = true);

  //if other_list is a list, then adds all values in it that are not
  //already in me.  If other_list is not a list, if it is Valid() then
  //adds that item to me if not already there.  Returns me.
  Am_Value_List& Append_New (Am_Value other_value, bool unique = true);
  Am_Value_List& Append_New (Am_Value_List& other_value, bool unique = true);

  // Delete the current element.  Error if no element is current.  The current
  // position is shifted to the element previous to the deleted.
  void Delete (bool unique = true);

  //removes from main list any values that appear in other_list.
  //Current element is ignored, but may be modified.
  void Set_Difference(Am_Value_List other_list, bool unique = true);
  
  // Creates an actual copy of the list contents.  Useful for making a list
  // unique when the program does not know how it will become unique.
  Am_Value_List Copy ();

  // Creates a copy of the list's contents.  If any of the elements of
  // the list are lists themselves, they are copied as well,
  // recursively.
  Am_Value_List Recursive_Copy ();

  // Delete the entire list.  All elements are deleted. The current position
  void Make_Empty ();  // becomes undefined.

  // Creates an empty list that is not (0L).
  static Am_Value_List Empty_List ();

  // returns true of v1 == v2.  Or, if either v1 or v2 is a list
  // and the other isn't a list, and the list contains a single value
  // which is == to the other value, then returns true.
  static bool List_Or_Value_Equal(const Am_Value& v1, const Am_Value& v2);

 private:
  Am_List_Item* item;
};

// Create a Type ID for Am_Value_List.
const Am_Value_Type Am_VALUE_LIST = Am_WRAPPER_TYPE | 2;

_OA_DL_IMPORT extern Am_Value_List Am_No_Value_List;

_OA_DL_IMPORT std::ostream& operator<< (std::ostream& os, const Am_Value_List& list);
     

// Example:
//  #include <amulet/valuelist.h>
//  main ()
//  {
//    Am_Value_List my_list;
//    my_list.Add (0).Add (1).Add (2).Add (3);
//
//    // Iterating through a list.
//    int val;
//    for (my_list.Start (); !my_list.Last (); my_list.Next ()) {
//      val = my_list.Get ();
//     std::cout << val;
//    }
//   std::cout <<std::endl;
// to go backwards:
//   for (my_list.End (); !my_list.First (); my_list.Prev ()) {

//
//    // Testing for membership.
//    my_list.Start ();
//    if (my_list.Member (2))
//     std::cout << "search successful" <<std::endl;
//    else
//     std::cout << "search not successful" <<std::endl;
//  }
#endif
