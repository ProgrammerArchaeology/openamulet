#ifndef AM_DEMON_QUEUE_H
#define AM_DEMON_QUEUE_H

#include "types_demon.h"

// Opaque type for Am_Demon_Queue.
class Am_Demon_Queue_Data;

// The demon queue.
// Demons are not called immediately.  When the demon's event occurs, the
// demon procedure is stored on the demon queue.  When Am_Object::Get is
// called, each procedure on the demon queue is invoked in order.
class _OA_DL_CLASSIMPORT Am_Demon_Queue
{
public:
  // Store a new demon procedure in the queue.
  void Enqueue(Am_Slot_Demon *demon, unsigned short which_bit,
               const Am_Slot &param);

  // Remove demon procedures from the queue.
  void Delete(const Am_Object &object);
  // Delete removes slot demons demons that correspond to a given
  // object.  A slot demon corresponds to an object of which it is part.
  void Delete(const Am_Slot &slot);

  // Tests whether queue has members.
  bool Empty() const;
  // Iterates through queue invoking each demon.  This will use Prevent_Invoke
  // to prevent the queue from being invoked recursively.
  void Invoke();

  // Prevents the queue from being invoked.  When the queue is being prevented,
  // calling Invoke is a no-op (not an error).  These methods use ref counting
  // calling Prevent_Invoke multiple times makes it necessary to call
  // Release_Invoke the same number to release the queue.  (It does not go
  // negative, though.)
  void Prevent_Invoke();
  void Release_Invoke();

  // Renders the queue inoperable.  This is used during clean-up to prevent
  // the queue from hampering destroying objects.
  void Shutdown();

  // Reinitializes the queue to its empty state.
  //  Used when invoking the inspector.
  void Reset();

  // Make a copy of a given queue.
  Am_Demon_Queue &operator=(const Am_Demon_Queue &proto);

  // Destructor.
  ~Am_Demon_Queue();

  // Create a new, empty queue.
  Am_Demon_Queue();
  Am_Demon_Queue(const Am_Demon_Queue &proto);
  Am_Demon_Queue(Am_Demon_Queue_Data *in_data) { data = in_data; }

  operator Am_Demon_Queue_Data *() const { return data; }

private:
  Am_Demon_Queue_Data *data;
};

#endif // AM_DEMON_QUEUE_H
