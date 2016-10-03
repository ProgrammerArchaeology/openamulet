/* ****************************** -*-c++-*- *******************************/
/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file contains graphic constants and types used by both Opal and Gem.
   Amulet users will need to reference classes defined in this file to
   provide parameters to Amulet functions.
*/

#ifndef GDEFS_H
#define GDEFS_H

#include <am_inc.h>

// class std::ostream;
#include "amulet/impl/types_wrapper.h" // to get the Am_Wrapper object
#include <amulet/value_list.h>         //value lists for file names

#include "amulet/impl/gem_flags.h"

#include "amulet/impl/am_font.h"

class Am_Style;

#include "amulet/impl/am_image_array.h"

#include "amulet/impl/am_cursor.h"

#include "amulet/impl/am_style.h"

#include "amulet/impl/am_point_list.h"

#include "amulet/impl/am_point_array.h"

#include "amulet/impl/am_time.h"

////////////////////////////////////////////////////////////////////////
//Interfaces for getting file data in a machine-independent way
////////////////////////////////////////////////////////////////////////

// **** NOTE: THESE FUNCTIONS ARE NOT IMPLEMENTED YET ****************

// Returns the full filename of the specified name.  Depending on the
// file system, this might add the current directory to the front.
extern Am_String Am_Get_Full_Filename(Am_String partial_filename);

// Pulls all the directory part out of the filename and returns it,
// and then sets file_only_name to the file.  Either one might be
// empty, but won't be (nullptr).
extern Am_String Am_Get_Directory_Part(Am_String filename,
                                       Am_String &file_only_name);

// attaches directory_part before filename (normally, just a string
// concatenation, possibly with a separator like "/")
extern Am_String Am_Concat_Filename(Am_String directory_part,
                                    Am_String filename);

// Given a filename that might have directories, takes it apart and
// returns the list of directories. The highest level directory (e.g.:
// "C:" on a PC, "/" on Unix, etc.) is first in the list.  If filename
// does not have any directory separators (either / or \ depending on
// the system) then a value_list with one entry, the passed in
// filename, will be returned.
extern Am_Value_List Am_Parse_Filename(Am_String filename);

//Returns a list of am_strings which are the names of all the files in
//the directory.  The file named "." is not returned, but ".." is.
extern Am_Value_List Am_Get_Directory_Contents(Am_String directory);

//Returns true of the filename is a directory
extern bool Am_File_Is_Directory(Am_String filename);

extern bool Am_File_Exists(Am_String full_filename);

//Returns number of bytes in the file.
extern long Am_Get_File_Size(Am_String filename);

//Under Unix and PC, returns the extension of the file.  If none,
//returns (nullptr).  Under Mac, returns the file type as a string.
extern Am_String Am_Get_File_Type(Am_String filename);

//... Will probably need other procedures in the future

#endif
