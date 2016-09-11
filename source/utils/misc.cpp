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
#if !defined(_WIN32)

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

const char* Am_Get_Amulet_Pathname () {
  const char* amulet_dir = getenv("AMULET_DIR");
  if (amulet_dir != 0)
    return amulet_dir;
  return DEFAULT_AMULET_DIR;
}

const char *Am_Merge_Pathname(const char *name)
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

const char* Am_Get_Amulet_Pathname()
{
	// do we have an environment variable?
	const char* amulet_dir = getenv("AMULET_DIR");
	if(amulet_dir != 0)
	{
		// yep, than return its value
		return(amulet_dir);
	}

	// no, return the default value
	return(DEFAULT_AMULET_DIR);
}

const char *Am_Merge_Pathname(const char *name)
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

#else
#pragma mark <=== Unsupported Platform Code ===>
// Unsupported platform

#include AM_IO__H
void Am_Break_Into_Debugger ()
{
 std::cerr << "Breaking into the debugger not implemented yet for this machine.\n";
}
#endif
