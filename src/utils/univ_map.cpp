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

#include <string.h>

#include <am_inc.h>
#ifdef OA_VERSION
#include <amulet/univ_map_oa.hpp>
#else
#include <amulet/univ_map.h>
#endif

#include <stdlib.h>

#ifndef OA_VERSION
AM_IMPL_MAP(Int2Ptr, int, 0, void *, (0L))
AM_IMPL_MAP(Int2Str, int, 0, char *, (0L))
AM_IMPL_MAP(Ptr2Int, void *, (0L), int, 0)
AM_IMPL_MAP(Str2Int, char *, (0L), int, 0)
#endif

#ifdef OA_VERSION
// declared in gem.h
OpenAmulet::Map_Int2Int Am_Map_int2int;
#endif

int
HashValue(const char *key, int size)
{
  //simply sum up first two and last two characters and normalize
  unsigned base;
  unsigned len = key ? strlen(key) : 0;

  switch (len) {
  case 0: {
    return (0);
  }
  case 1: {
    base = key[0] * 4;
    break;
  }
  default: {
    base = key[0] + key[1] + key[len - 2] + key[len - 1];
    break;
  }
  }

  return (base * unsigned(0x10000L / 4 / 0x100) % size);
}
