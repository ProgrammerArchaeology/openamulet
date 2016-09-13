//
// ------------------------------------------------------------------------
// 		The OpenAmulet User Interface Development Environment
// ------------------------------------------------------------------------
// This code is based on the Amulet project of Carnegie Mellon University,
// and has been placed in the public domain.  If you are using this code
// or any part of OpenAmulet, please contact amulet@cs.cmu.edu to be put
// on the mailing list or have a look at:
// http://www.openip.org
//

#ifndef LISTBOX_ABC_HPP
#define LISTBOX_ABC_HPP

#include <string>

//:The abstract-base-class (ABC) for the Am_Listbox class.
// This class is needed because we need to cast a pointer. As Am_Listbox
// is a template class there is no common pointer the pointer-type depends
// on the template parameters. To solve this we use a non-template abstract-base-class
// which only has one pure-virtual member function. That's the function which gets
// called through the pointer. Now all kind of Am_Listbox objects, regardless of the
// template parameters used can be access through this ABC and the 'selectMethod()'
// function can be called on them.
class Am_Listbox_ABC
{
public:
  // This function is used by the fillListbox template-function below to
  // get it hands on all kind of Am_Listbox objects
  virtual void deleteAllItems() = 0;
  virtual void deleteFirstItem() = 0;
  virtual void deleteLastItem() = 0;

  virtual void insertItemBack(std::string const &item) = 0;
  virtual void insertItemFront(std::string const &item) = 0;

  virtual void selectMethod() = 0;
};
#endif