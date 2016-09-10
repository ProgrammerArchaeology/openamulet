/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

// misc.cc
// This contains misc. machine dependant utility routines
// used throughout amulet.
//
// This should be broken into 3 machine-dependant files instead of just one
// file with lots of #ifdefs.

#include <am_inc.h>

#include MISC__H
#include AM_IO__H
#include <string.h>
#include <amulet/filefind.h>

// We need a better test for UNIX machines
#if !defined(_WIN32) & !defined(_MACINTOSH)

#include <signal.h>
#include <stdlib.h>
//#ifdef NEED_UNISTD
#include <unistd.h>
//#endif
#include <sys/time.h>

#include <sys/select.h>

void Am_Break_Into_Debugger ()
{
 std::cerr <<
    "** Breaking into the debugger:  Expect a SIGINT interrupt signal.\n";
  kill(getpid(), SIGINT);
}

#ifdef __VMS
extern "C" {
int LIB$WAIT(float *time);
}

void Am_Wait (int milliseconds)
{
  float time_wait = milliseconds;
  time_wait = milliseconds/1000.0;
  (void) LIB$WAIT(& time_wait);
}
#else

void Am_Wait (int milliseconds)
{
  timeval tim;
  tim.tv_sec = milliseconds/1000;
  tim.tv_usec = (milliseconds % 1000) * 1000;
  // Use select to hack a subsecond pause on UNIX.
  // There's no machine independant way to pause for a fraction of a second.
  // select usually does something, but NULLs say, "do nothing."
  // tim tells it how long to do nothing.
  select (0, (0L), NULL, NULL, &tim);
}
#endif

char* Am_Get_Amulet_Pathname () {
  char* amulet_dir = getenv("AMULET_DIR");
  if (amulet_dir != 0)
    return amulet_dir;
  return DEFAULT_AMULET_DIR;
}

char *Am_Merge_Pathname(char *name)
{
#if 0
  char * am_dir = Am_Get_Amulet_Pathname();
  if (!am_dir) am_dir = "..";
  char *ret_val = (char *) new char [(strlen(am_dir) + strlen(name) + 2)];

  strcpy(ret_val, am_dir);
  strcat(ret_val, "/");
  strcat(ret_val, name);
  return ret_val;
#else
  Am_Filename found;
  const bool ok = Am_Default_Data_Locator.Find_File(name, found);
  if (!ok)
    {
     std::cout << "Unable to locate file '" << name << "'" <<std::endl;
    }
  char *ret_val = (char *) new char [found.name().length() + 1];
  strcpy (ret_val, found.c_str());
  return ret_val;
#endif
}

#elif (_WIN32)

// the Windows specific code
#include <windows.h>
#include <stdlib.h>

void Am_Break_Into_Debugger ()
{
  DebugBreak();
}

void Am_Wait (int milliseconds)
{
  // this probably doesn't work.
  Sleep(milliseconds);
}

char* Am_Get_Amulet_Pathname()
{
	// do we have an environment variable?
	char* amulet_dir = getenv("AMULET_DIR");
	if(amulet_dir != 0)
	{
		// yep, than return its value
		return(amulet_dir);
	}

	// no, return the default value
	return(DEFAULT_AMULET_DIR);
}

char *Am_Merge_Pathname(char *name)
{
	// get the installation path
	char * am_dir = Am_Get_Amulet_Pathname();

	// is it a valid path?
	if(am_dir == 0)
	{
		// no, than use the parent directory
		am_dir = "..";
	}

	// and construct the path
	char *ret_val = (char *) new char [(strlen(am_dir) + strlen(name) + 2)];

	strcpy(ret_val, am_dir);
	strcat(ret_val, "/");
	strcat(ret_val, name);

	return(ret_val);
}

#elif (_MACINTOSH)
#pragma mark <=== Mac Specific Code ===>

#include AM_IO__H
#include TYPES__H

static char* gAmuletPathname = (0L);

/*******************************************************************************
 * Am_Wait
 */

void
Am_Wait(
  int milliseconds )
{
  const float kTicksPerMilliSec = (float) 60 / 1000;
  unsigned long gotTicks,
       reqTicks = (float) kTicksPerMilliSec * milliseconds + 0.5;
  Delay( reqTicks, &gotTicks );
}

/*******************************************************************************
 * Am_Break_Into_Debugger
 */

void
Am_Break_Into_Debugger()
{
  long gestaltResponse = 0;
  OSErr err = Gestalt( gestaltOSAttr, &gestaltResponse );

  if( err != noErr )
  {
   std::cerr << "Unable to determine if a debugger was present." <<std::endl;
    return;
  }

  // check if the gestaltSysDebuggerSupport bit is set
  if( (gestaltResponse >> gestaltSysDebuggerSupport ) & 0x00000001 )
    Debugger();
  else
   std::cerr << "This machine has no debugger present." <<std::endl;
}

/*******************************************************************************
 * Am_Get_Amulet_Pathname
 *   Returns the pathname to the amulet directory. One should treat this string
 *   a strictly constant.
 */

char*
Am_Get_Amulet_Pathname()
{
  if( gAmuletPathname != nil )
    return gAmuletPathname;
  else
    return nil;
}

/*******************************************************************************
 * Am_Init_Pathname
 *   Should only call once during initialization to set up the amulet pathname
 */

void
Am_Init_Pathname()
{
  const unsigned char amulet_env[] = "\pamulet.env";

  OSErr  err = noErr;
  short  foundVRefNum;
  long   foundDirID;
  FSSpec fileSpec;
  short  refNum = 0;
  char*  pathname = (0L);
  long   fileLength = 0,
         pathnameLength = 0;

  err = FindFolder( kOnSystemDisk, kPreferencesFolderType, false, &foundVRefNum, &foundDirID );

  if( err == noErr )
    err = FSMakeFSSpec( foundVRefNum, foundDirID, amulet_env, &fileSpec );

  if( err == noErr )
    err = FSpOpenDF( &fileSpec, fsRdPerm, &refNum );

  if( err == noErr )
    err = GetEOF(refNum, &fileLength);

  pathname = new char[ fileLength + 1 ];
  if( pathname == nil )
    err = memFullErr;

  if( err == noErr )
    err = FSRead( refNum, &fileLength, pathname );

  if( err ) // handle an error
  {
    if( refNum )
      FSClose( refNum );
    if( pathname )
      delete pathname;

    switch( err )
    {
      case fnfErr:
        Am_Error( "Unable to find amulet.env in Preferences Folder\nThis text file must contain the full pathname to the amulet\ndirectory in form volume_name:folder\nThere should be no : at the end of the pathname ");
        break;

      default:
        Am_Error( "Amulet unable to find the default path length" );
        break;
    }
	  return;
  }

  while( pathnameLength <= fileLength )
  {
    if( pathname[pathnameLength] == '\r')
    {
      pathnameLength++;
      break;
    }
    pathnameLength++;
  }
  pathnameLength--;

  pathname[ pathnameLength ] = '\0';

  gAmuletPathname = new char[ pathnameLength + 1 ];
  memcpy( gAmuletPathname, pathname, pathnameLength + 1 );
  delete pathname;
}

/*******************************************************************************
 * Am_Merge_Pathname
 */

char*
Am_Merge_Pathname(
  char *name )
{
  char* am_dir = Am_Get_Amulet_Pathname();
  if( am_dir == (0L) )
    Am_Error( "** Am_Merge_Pathname: No Amulet Pathname. Aborting\n");

  char *ret_val = (char*) new char [(strlen(am_dir) + strlen(name) + 2)];

  char* p = name;
  char	ch;
  while( (ch = *p) != '\0')
  {
  	if (ch == '/') *p = ':';
  	p++;
  }

  strcpy(ret_val, am_dir);
  strcat(ret_val, ":");
  strcat(ret_val, name);
  return ret_val;
}

#else
#pragma mark <=== Unsupported Platform Code ===>
// Unsupported platform

#include AM_IO__H
void Am_Break_Into_Debugger ()
{
 std::cerr << "Breaking into the debugger not implemented yet for this machine.\n";
}
#endif
