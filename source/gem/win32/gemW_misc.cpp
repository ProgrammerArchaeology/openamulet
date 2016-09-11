#include <string.h>

#include "amulet/gemW_misc.h"

char* strnew (const char* src)
{
  return src? strcpy((char*)malloc(strlen(src) + 1), src) : (0L);
}

Am_IMPL_LIST(Long, long, 0L)

#ifndef OA_VERSION
Am_IMPL_MAP(Ptr2Ptr, void*, (0L), void*, NULL)
#endif
