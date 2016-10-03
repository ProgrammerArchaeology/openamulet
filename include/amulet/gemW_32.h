#if defined(_WIN32)

#define __huge
#define _huge
#define huge

#define __far
#define _far
#define far

#define __export
#define _export
#define export

#define MoveTo(HDC, X, Y) (MoveToEx(HDC, X, Y, nullptr))

//#if defined(__MINGW32__)
//#include "gemW_mingw32.h"
//#endif

#include "gemW_thunk32.h"

#endif
