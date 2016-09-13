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
#ifndef REGISTERED_TYPE_HPP
#define REGISTERED_TYPE_HPP

// The Am_Registered_Type pure virtual class.  Classes derived from this type
// have runtime type ID's, and pointers to them can be registered in the name
// registry.
class Am_Registered_Type
{
public:
  //:Destructor should be virtual
  virtual ~Am_Registered_Type()
  {
    // nothing to do
    ;
  }

  //:returns unique tag for each derived type.
  virtual Am_ID_Tag ID() const = 0;

  //printing and reading for debugging
  virtual const char *To_String() const;
  virtual void Print(std::ostream &out) const;
  void Println() const
  {
    Print(cout);
    std::cout << std::endl << std::flush;
  }
};

#endif