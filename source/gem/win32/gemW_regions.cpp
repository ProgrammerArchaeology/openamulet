#include <windows.h>
#include <am_inc.h>

#include <amulet/impl/am_region.h>

#include "amulet/gemW.h"
#include "amulet/gemW_misc.h"
#include "amulet/gemW_clean.h"
#include "amulet/gemW_regions.h"

////////////
// Am_Region

Am_Region *
Am_Region::Create()
{
  return new Am_WinRegion();
}

///////////////
// Am_WinRegion::Am_Region

Am_WinRegion::Am_WinRegion() : m_hrgn(0) { Am_WINCLEAN_CONSTR(Am_WinRegion) }

Am_WinRegion::Am_WinRegion(int the_left, int the_top, unsigned int the_width,
                           unsigned int the_height)
    : m_hrgn(0), m_rect(the_left, the_top, the_width, the_height)
{
  Am_WINCLEAN_CONSTR(Am_WinRegion)
}

Am_WinRegion::Am_WinRegion(const Am_WinRegion &the_region)
    : m_hrgn(0), m_rect(the_region)
{
  Am_WINCLEAN_CONSTR(Am_WinRegion)
}

Am_WinRegion::~Am_WinRegion()
{
  Am_WINCLEAN_DESTR

  ClearAll();
}

void
Am_WinRegion::Clear()
{
#ifdef USE_WINCLEANER
  WinFreeRes();
#else
  if (m_hrgn) {
    DeleteObject(m_hrgn);
    m_hrgn = 0;
  }
#endif
  m_rect.SetEmpty();
}

void
Am_WinRegion::ClearAll()
{
  Am_Position pos;
  while (pos = m_history.HeadPos()) {
    delete (Am_WinRect *)m_history.GetAt(pos);
    m_history.DeleteAt(pos);
  }

  Clear();
}

void
Am_WinRegion::Set(int the_left, int the_top, unsigned int the_width,
                  unsigned int the_height)
{
#ifdef USE_WINCLEANER
  WinFreeRes();
#else
  if (m_hrgn) {
    DeleteObject(m_hrgn);
    m_hrgn = 0;
  }
#endif
  m_rect.Set(the_left, the_top, the_width, the_height);
}

void
Am_WinRegion::Push(const Am_WinRect &wrect)
{
  m_history.InsertAt(m_history.HeadPos(), new Am_WinRect(m_rect));
  if ((m_history.Count() == 1) && (m_rect.IsEmpty())) //first push
    m_rect = wrect;
  else
    Intersect(wrect);
}

void
Am_WinRegion::Push(/*const*/ Am_Region *the_region)
{
  if (!the_region)
    return;
  Push(*Narrow(the_region));
}

void
Am_WinRegion::Push(int the_left, int the_top, unsigned int the_width,
                   unsigned int the_height)
{
  Am_WinRect rect(the_left, the_top, the_width, the_height);
  Push(rect);
}

void
Am_WinRegion::Pop()
{
  Am_Position pos = m_history.HeadPos();
  if (pos) {
    Am_WinRect *prect = (Am_WinRect *)m_history.GetAt(pos);
    m_rect = *prect;
#ifdef USE_WINCLEANER
    WinFreeRes();
#else
    if (m_hrgn) {
      DeleteObject(m_hrgn);
      m_hrgn = 0;
    }
#endif
    delete prect;
    m_history.DeleteAt(pos);
  }
}

void
Am_WinRegion::Union(const Am_WinRect &wrect)
{
#ifdef USE_WINCLEANER
  WinFreeRes();
#else
  if (m_hrgn) {
    DeleteObject(m_hrgn);
    m_hrgn = 0;
  }
#endif
  m_rect += wrect;
}

void
Am_WinRegion::Union(const Am_Region *the_region)
{
  if (!the_region)
    return;
  Union(*Narrow(the_region));
}

void
Am_WinRegion::Union(int the_left, int the_top, unsigned int the_width,
                    unsigned int the_height)
{
  Am_WinRect rect(the_left, the_top, the_width, the_height);
  Union(rect);
}

void
Am_WinRegion::Intersect(const Am_WinRect &wrect)
{
#ifdef USE_WINCLEANER
  WinFreeRes();
#else
  if (m_hrgn) {
    DeleteObject(m_hrgn);
    m_hrgn = 0;
  }
#endif
  m_rect *= wrect;
}

void
Am_WinRegion::Intersect(const Am_Region *the_region)
{
  if (!the_region)
    return;
  Intersect(*Narrow(the_region));
}

void
Am_WinRegion::Intersect(int the_left, int the_top, unsigned int the_width,
                        unsigned int the_height)
{
  Am_WinRect rect(the_left, the_top, the_width, the_height);
  Intersect(rect);
}

bool Am_WinRegion::In(int x, int y) // const
{
  POINT pt = {x, y};
  return (::PtInRect(&m_rect, pt) != 0);
}

bool Am_WinRegion::In(int x, int y, unsigned int width, unsigned int height,
                      bool &total) // const
{
  Am_WinRegion wreg(x, y, width, height);
  return In(&wreg, total);
}

bool Am_WinRegion::In(/*const*/ Am_Region *the_region, bool &total) // const
{
  Am_WinRect rci = m_rect * *Narrow(the_region);
  if (rci.IsEmpty()) {
    total = false;
    return false;
  }

  total = rci == *Narrow(the_region);
  return true;
}

HRGN
Am_WinRegion::WinRegion()
{
#ifdef USE_WINCLEANER
  WinSetUsedRes();
#endif
  if (!m_hrgn)
    m_hrgn = ::CreateRectRgnIndirect(&m_rect);
  return m_hrgn;
}

#ifdef USE_WINCLEANER
AM_IMPL_WINCLEAN(Am_WinRegion)

BOOL
Am_WinRegion::WinHasRes() const
{
  return (BOOL)m_hrgn;
}

BOOL
Am_WinRegion::WinFreeRes()
{
  if (m_hrgn) {
    DeleteObject(m_hrgn);
    m_hrgn = 0;
    return TRUE;
  }
  return FALSE;
}
#endif
