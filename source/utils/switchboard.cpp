//-----------------------------------------------------------------------------------------
// Copyright:
//		Copyright 1997,1998 SCRAP GmbH, Karlsruhe, Germany
//		Copyright 1997,1998 PACT GmbH, Munich, Germany
//
// Initial-Author:
//		Robert M. Münch at: 13.06.98 14:49:05
//
// Synopsis:
//		Start text here
//
// Description:
//		Start text here
//
// Files:
//		This file: switchboard.cpp
//		Start text here
//
// Version-Control-Information:
//		$Id: switchboard.cpp,v 1.1.1.1 2005/11/09 18:47:26 mitosys Exp $
//
// See Also:
//		Start text here
//
// Bugs:
//		None
//
// History:
//		Start text here
//
//-----------------------------------------------------------------------------------------

//-------------------------------------------------------------
// system include files
//-------------------------------------------------------------
#include <stdlib.h> // to define NULL

//-------------------------------------------------------------
// application include files
//-------------------------------------------------------------
#ifndef SWTICHBOARD_HPP
#include <amulet/switchboard.hpp>
#endif

#include <amulet/callback.hpp>

//-------------------------------------------------------------
// Re-enables warnings
//-------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning(default : 4018) // < signed/unsigned
// #pragma warning(default: 4100) // unreferenced formal parameter
#pragma warning(default : 4201) // nameless union
#pragma warning(default : 4214) // non 'int' bitfields
// #pragma warning(default: 4244) // int to char loss of data
// #pragma warning(default: 4511) // copy ctor couldn't be generated
// #pragma warning(default: 4512) // assignment operator couldn't be generated
// #pragma warning(default: 4514) // unreferenced inline function
#pragma warning(default : 4663) // c++ language change
// #pragma warning(default: 4786) // identifier was truncated to 255 characters
#endif

//-------------------------------------------------------------
// external functions
//-------------------------------------------------------------

//-------------------------------------------------------------
// constants / statics / typedefs
//-------------------------------------------------------------
Switch_Board *Switch_Board::mSelf = static_cast<Switch_Board *>(0);

//-------------------------------------------------------------
// forward declarations / definitions
//-------------------------------------------------------------

//-------------------------------------------------------------
// class definition/implementation
//-------------------------------------------------------------

//-------------------------------------------------------------
// Initial-Author	: Robert M. Münch at: 13.06.98 14:50:11
// Description		: The constructor is protected to insure that only one
// 					  instance of this class can exist.  The class is
// 					  constructed by calling the static instance() method.
// Pre-Condition	: none
// Post-Condition	: one instance will be generated
// Result			: one instance
//-------------------------------------------------------------
Switch_Board::Switch_Board()
{
  // save the handle for the singleton-pattern check
  mSelf = this;
}

//-------------------------------------------------------------
// Initial-Author	: Robert M. Münch at: 13.06.98 14:51:52
// Description		: This is the access method to the singleton object
// 					  and is responsible for instantiating the Switch_Board.
// Pre-Condition	: none
// Post-Condition	: an instance of Switch_Board exists
// Result			: Instance always returns the reference to the instance.
//-------------------------------------------------------------
Switch_Board &
Switch_Board::instance()
{
  // do we have an instance?
  if (mSelf != static_cast<Switch_Board *>(0)) {
    // yes, return it
    return (*mSelf);
  } else {
    // create new instance
    mSelf = new Switch_Board;
    return (*mSelf);
  }
}

//-------------------------------------------------------------
// Initial-Author	: Robert M. Münch at: 16.06.98 11:14:30
// Description		: Adds the aCallbackObject to the array of mCallbacks. The array dynamically grows
// 					  to accommodate the new aCallbackObject.
// Pre-Condition	:
// Post-Condition	:
// Result			:
//-------------------------------------------------------------
void
Switch_Board::_subscribe(Callback *aCallbackObject)
{
  // insert the new Callback object into the map
  mCallbacks.insert(std::pair<const std::string, Callback *>(
      aCallbackObject->getSubscription(), aCallbackObject));

  return;
}

//-------------------------------------------------------------
// Initial-Author	: Robert M. Münch at: 16.06.98 11:16:30
// Description		:
// Pre-Condition	:
// Post-Condition	:
// Result			:
//-------------------------------------------------------------
void
Switch_Board::_unsubscribe(Callback *aCallbackObject)
{
  // is this a valid pointer?
  if (mCallbacks.empty() == false) {
    // find the callback-object in the map
    std::multimap<const std::string, Callback *>::iterator foundObject =
        mCallbacks.find(aCallbackObject->getSubscription());

    // and remove it if we have found it
    if (foundObject != mCallbacks.end()) {
      mCallbacks.erase(foundObject);
    }
  }

  return;
}

//-------------------------------------------------------------
// Initial-Author	: Robert M. Münch at: 16.06.98 10:46:31
// Description		: Iterates through the subscribes and for each one
// 					  that has a subscription asks the aCallbackObject to deliver the message.
// Pre-Condition	: none
// Post-Condition	: all subscribers which have subscribed to this message are notified
// Result			: true is returned if the message is delivered to at least one subscriber.
//-------------------------------------------------------------
bool
Switch_Board::_post(const char *aSubscription)
{
  // flag to see if at least one message was deliverd
  bool delivered = false;

  // do we have any subscribers?
  if (mCallbacks.empty() == false) {
    // find the range of objects which have to be noted
    std::pair<std::multimap<const std::string, Callback *>::iterator,
              std::multimap<const std::string, Callback *>::iterator>
        iteratorPair = mCallbacks.equal_range(aSubscription);

    for (std::multimap<const std::string, Callback *>::iterator it =
             iteratorPair.first;
         it != iteratorPair.second; ++it) {
      // make an object out of it and call the function-operator ()
      // see class Callback for implementation
      (*((*it).second))();

      delivered = true;
    }
  }

  // return the flag
  return (delivered);
}

//-------------------------------------------------------------
// non-member functions
//-------------------------------------------------------------
