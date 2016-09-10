//-----------------------------------------------------------------------------------------
// Copyright:
//
// Initial-Author:
//		Robert M. Münch at: 09.05.98 15:03:22
//
// Synopsis:
//		Start text here
//
// Description:
//		Start text here
//
// Files:
//		This file: initializer.cpp
//		Start text here
//
// Version-Control-Information:
//		$Id: initializer.cpp,v 4.1.4.1 2005/11/09 18:19:27 mitosys Exp $
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
#include <am_inc.h>
#include INITIALIZER__H
#include <amulet/impl/types_logging.h> // for DERR

//-------------------------------------------------------------
// application include files 
//-------------------------------------------------------------

#ifdef USE_SMARTHEAP
	#ifndef _SMARTHEAP_HPP
		#include <smrtheap.hpp>
	#endif
#endif

//-------------------------------------------------------------
// Re-enables warnings
//-------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning(default: 4018) // < signed/unsigned
// #pragma warning(default: 4100) // unreferenced formal parameter
#pragma warning(default: 4201) // nameless union
#pragma warning(default: 4214) // non 'int' bitfields
// #pragma warning(default: 4244) // int to char loss of data
// #pragma warning(default: 4511) // copy ctor couldn't be generated
// #pragma warning(default: 4512) // assignment operator couldn't be generated
// #pragma warning(default: 4514) // unreferenced inline function
#pragma warning(default: 4663) // c++ language change
// #pragma warning(default: 4786) // identifier was truncated to 255 characters
#endif

//-------------------------------------------------------------
// external functions 
//-------------------------------------------------------------

//-------------------------------------------------------------
// constants / statics / typedefs
//-------------------------------------------------------------
Am_Initializer::INITIALIZERLIST *Am_Initializer::am_initializer_list;

//-------------------------------------------------------------
// forward declarations / definitions
//-------------------------------------------------------------

//-------------------------------------------------------------
// class definition/implementation 
//-------------------------------------------------------------
Am_Initializer::Am_Initializer(char *this_name, Am_Initializer_Procedure *init_proc, float this_priority, unsigned aCheckPoint, Am_Initializer_Procedure *cleanup_proc) :
	cleanup(cleanup_proc),
	init(init_proc),
	name(this_name),
	priority(this_priority),
	mCheckPoint(aCheckPoint)
{
 	// if this list is empty, we jsut add the element           
 	if(am_initializer_list == static_cast<INITIALIZERLIST*>(0)) 
 	{
		#ifdef USE_SMARTHEAP
	 		am_initializer_list = new INITIALIZERLIST();
		#else
			am_initializer_list = new INITIALIZERLIST();
		#endif
 		am_initializer_list->push_front(this);                   
 	}                                                           
 	else                                                        
 	{                                                           
		bool inserted(false);

		for(INITIALIZERLIST::iterator it = am_initializer_list->begin(); it != am_initializer_list->end(); ++it)
		{
			// check to see if we have reached the point
			// where the object is inserted before
			if((*it)->priority >= this_priority)
			{
				// insert the object and flag this
				am_initializer_list->insert(it,this);
				inserted = true;

				// exit the loop
				break;
			}
		}

		if(inserted == false)
		{
			// add to the end of the list
			am_initializer_list->push_back(this);
		}
	}
}

/*
Am_Initializer::~Am_Initializer()                                                                              
{                                                                                                              
	// iterate through the list if Am_Initializer objects and free them                                           
  	for(INITIALIZERLIST::iterator it = am_initializer_list->begin(); it != am_initializer_list->end(); ++it)  
  	{                                                                                                         
		DEBUG_DELETE(*it);                                                                                    
  	}                                                                                                         
}                                                                                                              
*/

//-------------------------------------------------------------
// Initial-Author	: Robert M. Münch at: 09.05.98 15:45:23
// Description		: calls the init() function for each object registered
// Pre-Condition	: none
// Post-Condition	: all objects in the list are initialized
// Result			: none
//-------------------------------------------------------------
void Am_Initializer::Do_Initialize()
{
	for(INITIALIZERLIST::iterator it = am_initializer_list->begin(); it != am_initializer_list->end(); ++it)
	{
		// check if we have a function to call
		if((*it)->init != static_cast<Am_Initializer_Procedure*>(0))
		{
			#ifdef USE_SMARTHEAP
				unsigned oldCheckPoint = dbgMemSetCheckpoint((*it)->mCheckPoint);
			#endif

			// call the init() function stored in the Am_Initializer object
#ifdef LEAK_TRACE
			DERR("Initializing: " << (*it)->name << "\t\tPriority: " << (*it)->priority << std::endl);
#endif
			(*it)->init();

			#ifdef USE_SMARTHEAP
				dbgMemSetCheckpoint(oldCheckPoint);
			#endif
		}
	}
}

//-------------------------------------------------------------
// Initial-Author	: Robert M. Münch at: 09.05.98 15:46:36
// Description		: cleans-up all objects registered in the list; the list is traversed in backward order
// Pre-Condition	: none
// Post-Condition	: all objects inthe list are cleanedup
// Result			: non
//-------------------------------------------------------------
void Am_Initializer::Do_Cleanup()
{
	for(INITIALIZERLIST::reverse_iterator reverseIt = am_initializer_list->rbegin(); reverseIt != am_initializer_list->rend(); ++reverseIt)
	{
		// check if we have a function to call
		if((*reverseIt)->cleanup != static_cast<Am_Initializer_Procedure*>(0))
		{
			// call the cleanup() function stored in the Am_Initializer object
#ifdef LEAK_TRACE
			DERR("Cleaningup: " << (*reverseIt)->name << "\t\tPriority: " << (*reverseIt)->priority << std::endl);
#endif
			(*reverseIt)->cleanup();
		}
	}

	// iterate through the list if Am_Initializer objects and free them
	for(INITIALIZERLIST::iterator it = am_initializer_list->begin(); it != am_initializer_list->end(); ++it)
	{
		#ifdef USE_SMARTHEAP
			DEBUG_DELETE(*it);
		#else
			delete(*it);
		#endif
	}
}

//-------------------------------------------------------------
// non-member functions 
//-------------------------------------------------------------
