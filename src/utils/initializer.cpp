#include <am_inc.h>
#include <amulet/initializer.h>
#include <amulet/impl/types_logging.h> // for DERR

Am_Initializer::INITIALIZERLIST *Am_Initializer::am_initializer_list;

Am_Initializer::Am_Initializer(const char *this_name,
                               Am_Initializer_Procedure *init_proc,
                               float this_priority, unsigned aCheckPoint,
                               Am_Initializer_Procedure *cleanup_proc)
    : cleanup(cleanup_proc), init(init_proc), name(this_name),
      priority(this_priority), mCheckPoint(aCheckPoint)
{
  // if this list is empty, we just add the element
  if (am_initializer_list == static_cast<INITIALIZERLIST *>(nullptr)) {
    am_initializer_list = new INITIALIZERLIST();
    am_initializer_list->push_front(this);
  } else {
    bool inserted(false);

    for (INITIALIZERLIST::iterator it = am_initializer_list->begin();
         it != am_initializer_list->end(); ++it) {
      // check to see if we have reached the point
      // where the object is inserted before
      if ((*it)->priority >= this_priority) {
        // insert the object and flag this
        am_initializer_list->insert(it, this);
        inserted = true;

        // exit the loop
        break;
      }
    }

    if (inserted == false) {
      // add to the end of the list
      am_initializer_list->push_back(this);
    }
  }
}

/*
Am_Initializer::~Am_Initializer()
{
  // iterate through the list if Am_Initializer objects and free them
  for (INITIALIZERLIST::iterator it = am_initializer_list->begin();
       it != am_initializer_list->end(); ++it) {
    DEBUG_DELETE(*it);
  }
}
*/

// Calls the init() function for each object registered.
//
// Afterwards, all objects in the list are initialized.
void
Am_Initializer::Do_Initialize()
{
  for (INITIALIZERLIST::iterator it = am_initializer_list->begin();
       it != am_initializer_list->end(); ++it) {
    // check if we have a function to call
    if ((*it)->init != static_cast<Am_Initializer_Procedure *>(nullptr)) {
#ifdef LEAK_TRACE
      DERR("Initializing: " << (*it)->name << "\t\tPriority: "
                            << (*it)->priority << std::endl);
#endif
      // call the init() function stored in the Am_Initializer object
      (*it)->init();
    }
  }
}

// Cleans-up all objects registered in the list; the list
// is traversed in backward order.
//
// Afterwards, all objects in the list are cleaned up.
void
Am_Initializer::Do_Cleanup()
{
  for (INITIALIZERLIST::reverse_iterator reverseIt =
           am_initializer_list->rbegin();
       reverseIt != am_initializer_list->rend(); ++reverseIt) {
    // check if we have a function to call
    if ((*reverseIt)->cleanup != static_cast<Am_Initializer_Procedure *>(nullptr)) {
// call the cleanup() function stored in the Am_Initializer object
#ifdef LEAK_TRACE
      DERR("Cleaningup: " << (*reverseIt)->name << "\t\tPriority: "
                          << (*reverseIt)->priority << std::endl);
#endif
      (*reverseIt)->cleanup();
    }
  }

  // iterate through the list if Am_Initializer objects and free them
  for (INITIALIZERLIST::iterator it = am_initializer_list->begin();
       it != am_initializer_list->end(); ++it) {
    delete (*it);
  }
}
