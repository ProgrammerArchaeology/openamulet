#if !defined(GWMISC_H)
#define GWMISC_H

#include <stdlib.h>
#include <am_inc.h>

#ifdef OA_VERSION
#include <amulet/univ_map_oa.hpp>
#else
#include <amulet/univ_map.h>
#endif
#include <amulet/univ_lst.h>

//#pragma pack()

#if !defined(DEBUG)
#define GWFASTCALL __fastcall
#else
#define GWFASTCALL
#endif

char *strnew(const char *src);

inline void
strdel(char *s)
{
  if (s)
    free((void *)s);
}

#if !defined(_WIN32)
#define CODE_BASED __based(__segname("_CODE"))
#else
#define CODE_BASED
#endif

#define LOSHORT(X) ((SHORT)(LOWORD(X)))
#define HISHORT(X) ((SHORT)(HIWORD(X)))

#ifndef OA_VERSION
AM_DECL_MAP(Ptr2Ptr, void *, void *)
#endif
AM_DECL_LIST(Long, long, nullptr)

#endif
