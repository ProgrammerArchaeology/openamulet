 /* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* 
   This file contains the Macros needed for cross compiler streams
   
   -----------------------------------------------------------------------
   TO USE:
   
      #include STR_STREAM__H
      
   when you want to pass an ostrstream as a parameter,

      void foo ( ostrstream &a ) { ... }

   becomes

      void foo ( OSTRSTREAM &a ) { ... }

   Next, had to rebuild the stream mechanism for templatized streams since they
   did not work the same way as before.  Using the macros provided...

  this   
      char line[100];
      ostrstream oss(line,100,std::ios::out);
      oss << object <<std::ends;   // <== side effect operator shifts object into line

  becomes this:

      char line[100];  
      OSTRSTREAM_CONSTR (oss,line,100,std::ios::out);      
      oss << object <<std::ends;
      OSTRSTREAM_COPY(oss,line,100); // <== this copies oss into line which is
                                     // the side effect we intend

   OSTRSTREAM_COPY should be placed at each std::ends;"
   -----------------------------------------------------------------------
   
   TO SUPPORT WITH A NEW COMPILER:
   also when you change stuff below, be sure to change Am_POP_UP_ERROR_WINDOW
   in widgets.h too or it will not work
   
   bdk 6/15/97
   rev 7/17/97
*/

#include <strstream>

#define OSTRSTREAM std::ostrstream 
#define OSTRSTREAM_CONSTR(a,b,c,d) 	std::ostrstream a(b,c,d)
#define OSTRSTREAM_COPY(a,b,c) 		/* do nothing here */

//lint --e(752) ok, isn't referenced here but included for convinience
extern void reset_ostrstream(OSTRSTREAM &oss);  // implemented in types.cc
