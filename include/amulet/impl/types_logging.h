#include <iostream>

#include "types_basic.h"
#include "types_slot.h"
class Am_Object;

// Am_Debugger points to a routine that is invoked whenever DEBUG is defined
// and an Am_Error occurs.
_OA_DL_IMPORT extern void (*Am_Debugger)(void);

// Call Am_Error to generate a software generated error.  Hopefully, someday
// this will be replaced by exceptions.
[[noreturn]] _OA_DL_IMPORT extern void Am_Error(const char *error_string);
// Am_Error prints out "** Program Aborted"
[[noreturn]] _OA_DL_IMPORT extern void Am_Error();

//This version can be used with a print-out string like:
//  Am_ERROR(object << " is not valid")
#define Am_ERROR(error_string)                                                 \
  {                                                                            \
    std::cerr << "** Amulet_Error: " << error_string << std::endl;             \
    Am_Error();                                                                \
  }

// Macros which avoid that static strings get included into the release
// version of the library therefore leading to smaller applications
#ifndef DERR
#ifdef DEBUG
#define DERR(str) std::cerr << str
#else
#define DERR(str)
#endif
#endif

#ifndef Am_DERR
#ifdef DEBUG
#define Am_DERR(str) Am_Error(str)
#else
#define Am_DERR(str) Am_Error()
#endif
#endif

#ifndef DSTR
#ifdef DEBUG
#define DSTR(str) str
#else
#define DSTR(str) ((char *)0)
#endif
#endif

//These versions pop up the inspector on the specified object and slot

_OA_DL_IMPORT extern void (*Am_Object_Debugger)(const Am_Object &obj,
                                                Am_Slot_Key slot);
_OA_DL_IMPORT extern void Am_Error(const char *error_string,
                                   const Am_Object &obj, Am_Slot_Key slot);
_OA_DL_IMPORT extern void Am_Error(const Am_Object &obj, Am_Slot_Key slot);

#ifdef DEBUG
#define Am_ERRORO(error_string, obj, slot)                                     \
  {                                                                            \
    std::cerr << "** Amulet_Error: " << error_string << std::endl;             \
    Am_Error(obj, slot);                                                       \
  }
#else
#define Am_ERRORO(error_string, obj, slot)                                     \
  {                                                                            \
    std::cerr << "** Amulet_Error: " << error_string << std::endl;             \
    Am_Error();                                                                \
  }
#endif

//prints val as (type_name) 0xXXXXX
_OA_DL_IMPORT extern void Am_Standard_Print(std::ostream &out,
                                            unsigned long val, Am_ID_Tag type);
