// This is a helper class used for iterating through an object's instances.
// To initialize the iterator, assign the prototype object to it, or assign
// the object when the iterator gets created.
class _OA_DL_CLASSIMPORT Am_Instance_Iterator
{
public:
  Am_Instance_Iterator();
  Am_Instance_Iterator(Am_Object object);

  Am_Instance_Iterator &operator=(Am_Object &object);

  unsigned short Length(); // Number of instances in the list.
  void Start();            // Begin list at the start.
  void Next();             // Move to next element in list.
  bool Last();             // Is this the last element?
  Am_Object Get();         // Get the current element.
private:
  Am_Object current;
  Am_Object prototype;
};
