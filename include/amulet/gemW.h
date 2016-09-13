#if !defined(GEMW_H)
#define GEMW_H

#if defined(_WIN32)
#include "gemW_32.h"
#endif

void Am_WinInit();

struct Am_Point
{
  int x;
  int y;
  Am_Point(int ax, int ay) : x(ax), y(ay){};
  Am_Point() : x(0), y(0){};
};

#define AM_DECL_WINNARROW(Name)                                                \
public:                                                                        \
  static Am_Win##Name *Narrow(void *item) { return (Am_Win##Name *)item; }     \
  static const Am_Win##Name *Narrow(const void *item)                          \
  {                                                                            \
    return (const Am_Win##Name *)item;                                         \
  }                                                                            \
  static Am_Win##Name *Narrow(Am_##Name *item)                                 \
  {                                                                            \
    return (Am_Win##Name *)item;                                               \
  }                                                                            \
  static const Am_Win##Name *Narrow(const Am_##Name *item)                     \
  {                                                                            \
    return (const Am_Win##Name *)item;                                         \
  }

// Macros to access wrapped data
//	Am_Style_Data* wls = Am_Style_Data::Narrow(ls); => PREPARE_DATA(Am_Style, wls, ls)
//	wls -> Release(); => DISCARD_DATA(wls)

#define PREPARE_DATA(T, D, W) T##_Data *D = T##_Data::Narrow(W);
#define DISCARD_DATA(D)                                                        \
  if (D)                                                                       \
    D->Release();

#endif
