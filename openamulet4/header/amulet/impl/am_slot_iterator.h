class Am_Slot_Iterator_Data;

// This is a helper class used for iterating through the slots of an object.
// The iterator is initialized by setting it with the object whose slots are
// to be examined.  The elements of the list are actually slot keys and not
// actual slots.  This iterator will list all keys that can be accessed from
// the object including those defined in prototype objects.
class _OA_DL_CLASSIMPORT Am_Slot_Iterator {
 public:
  Am_Slot_Iterator ();
  Am_Slot_Iterator (Am_Object object);
  ~Am_Slot_Iterator ();
  
  Am_Slot_Iterator& operator= (Am_Object object);
  
  unsigned short Length ();       // Number of slots in the list.
  void Start ();                  // Begin list at the start.
  void Next ();                   // Move to next element in list.
  bool Last ();                   // Is this the last element?
  Am_Slot_Key Get ();             // Get the current element.
 private:
  Am_Slot_Iterator_Data* data;
  Am_Object context;
};
