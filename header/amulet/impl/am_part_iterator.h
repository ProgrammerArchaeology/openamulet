#ifndef OBJECT_PART_ITER_H
#define OBJECT_PART_ITER_H

#include "am_object.h"

// This is a helper class used for iterating through an object's parts.
// The iterator is inialized by assigning the owner object to the iterator.
// The elements of the list will include both named and unnamed parts.

class _OA_DL_CLASSIMPORT Am_Part_Iterator {
 public:
  Am_Part_Iterator ();
  Am_Part_Iterator (Am_Object object);
  
  Am_Part_Iterator& operator= (Am_Object object);
  
  unsigned short Length ();       // Number of parts in the list.
  void Start ();                  // Begin list at the start.
  void Next ();                   // Move to next element in list.
  bool Last ();                   // Is this the last element?
  Am_Object Get ();               // Get the current element.
 private:
  Am_Object current;
  Am_Object owner;
};

#endif // OBJECT_PART_ITER_H
