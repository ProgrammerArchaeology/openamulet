#include <windows.h>

#include "amulet/gemW_32.h"
#include "amulet/gemW_misc.h"
#include "amulet/gemW_line.h"

struct Am_WinDDAInfo
{
  HDC m_hdc;
  int m_curdash;
  int m_curpix;
  //	BOOL m_ink;
  int m_ndash;
  int *m_dashes;
  Am_WinDDAInfo(HDC hdc, const char *dashes, int ndash, int w, BOOL onoff);
  ~Am_WinDDAInfo();
};

extern "C" void CALLBACK LineDDAProc(int x, int y, LPARAM data);

#ifndef __GNUC__

//LARGE data model only!
extern "C" void CALLBACK
LineDDAProc(int x, int y, LPARAM data)
{
#if !defined(_MSC_VER) ||                                                      \
    (_MSC_VER >= 1010) /* MSVC 4.1 chokes on this inline asm */
  LineTo(((Am_WinDDAInfo *)data)->m_hdc, x, y);
#else
#if !defined(_WIN32)
  __asm {
		push ds
		lds si, data //*ds:si = Am_WinDDA
		mov dx, [si].m_curpix //dx = m_curpix

		les bx, [si].m_dashes //*es:bx = m_dashes
		mov di, [si].m_curdash //di = m_curdash
		shl di, 1 //m_curdash *= sizeof(int)
		mov ax, es:[bx + di] //ax = m_dashes[m_curdash]
		
		cmp dx, ax //m_curpix < m_dashes[m_curdash]?
		jge l_dashend //n

    //not yet end of dash
		inc dx //y; m_curpix++
		mov [si].m_curpix, dx //save m_curpix
		pop ds
		jmp SHORT l_end //return

l_dashend:
        //end of dash
		shr di, 1 //m_curdash /= sizeof(int)
		inc di //m_curdash++
		cmp di, [si].m_ndash //m_curdash >= m_ndash?
		jl l_setdash //n
		xor di, di //y; m_curdash = 0
l_setdash:
		mov [si].m_curdash, di //save m_curdash
		xor dx, dx //m_curpix = 0
		mov [si].m_curpix, dx                     //save m_curpix
                           //		mov ax, [si].m_ink		//ax = m_ink
                           //		xor ax, 0x0001			//m_ink = !m_ink
    //		mov [si].m_ink, ax		//save m_ink
		
		mov dx, [si].m_hdc //dx = m_hdc
		pop ds
		push dx         //_pascal (m_hdc, x, y)
		push x
		push y
                           //		and ax, 0x0001			//m_ink?
        //		jz l_lineto //we already inverted m_ink!
		and di, 0x0001 //ink?
		jnz l_lineto
		call MoveTo //n; move CP
		jmp SHORT l_end
l_lineto:
		call LineTo //y; draw
l_end:
  }
#else
  __asm {
		push esi
		push edi

		mov esi, data //*esi = Am_WinDDA
		mov edx, [esi].m_curpix //edx = m_curpix

		mov ebx, [esi].m_dashes //*ebx = m_dashes
		mov edi, [esi].m_curdash //edi = m_curdash
		shl edi, 2 //m_curdash *= sizeof(int)
		mov eax, [ebx + edi] //eax = m_dashes[m_curdash]
		
		cmp edx, eax //m_curpix < m_dashes[m_curdash]?
		jge l_dashend //n

    //not yet end of dash
		inc edx //y; m_curpix++
		mov [esi].m_curpix, edx //save m_curpix
		jmp SHORT l_end  //return

l_dashend:
        //end of dash
		shr edi, 2 //m_curdash /= sizeof(int)
		inc edi //m_curdash++
		cmp edi, [esi].m_ndash //m_curdash >= m_ndash?
		jl l_setdash //n
		xor edi, edi //y; m_curdash = 0
l_setdash:
		mov [esi].m_curdash, edi //save m_curdash
		xor edx, edx //m_curpix = 0
		mov [esi].m_curpix, edx //save m_curpix
		
		mov edx, [esi].m_hdc //edx = m_hdc
		and edi, 0x00000001L      //ink?
		jnz l_lineto
                     //n; move CP
    //MoveToEx(m_hdc, x, y, (0L))
		push 0x00000000L //(0L)
		push y
		push x
		push edx					
		call DWORD PTR [MoveToEx]
		jmp SHORT l_end
l_lineto:
                     //y; draw
        //LineTo(m_hdc, x, y)
		push y
		push x
		push edx					
		call DWORD PTR [LineTo]
l_end:
		mov eax, 0x00000001L      //continue enumeration

		pop edi
		pop esi
  }
#endif // Win32
#endif // MSC_VER
}

#endif

#define isodd(I) ((I)&0x0001)
#define iseven(I) (!isodd(I))

inline int
imax(int a, int b)
{
  return (a > b) ? a : b;
}

Am_WinDDAInfo::Am_WinDDAInfo(HDC hdc, const char *dashes, int ndash, int w,
                             BOOL onoff)
{
  m_hdc = hdc;

  int n = (isodd(ndash) && !onoff) ? ndash * 2 : ndash;
  m_dashes = new int[n];

  if (isodd(w))
    w--;

  int i;
  for (i = 0; i < ndash; i++) {
    m_dashes[i] = imax(dashes[i] - w, 1);
    i++;
    m_dashes[i] = dashes[i] + w;
  }
  for (i = ndash; i < n; i++)
    m_dashes[i] = m_dashes[i - ndash];

  if (onoff || iseven(ndash)) {
    m_ndash = n;
    m_curdash = 0;
    m_curpix = 0;
  } else {                            //!onoff & isodd(ndash)
    m_dashes[0] += dashes[ndash - 1]; //merge first & last
    m_ndash = n - 1;
    m_curdash = 0;
    m_curpix = imax(dashes[0] - w / 2, 1);
  }
  //	m_ink = TRUE;
}

Am_WinDDAInfo::~Am_WinDDAInfo() { delete[] m_dashes; }

void /*GWFASTCALL*/ Am_WinDrawArrDashLine(HDC hdc, HINSTANCE hinst, int x1,
                                          int y1, int x2, int y2,
                                          const char *dashes, int ndash, int w,
                                          BOOL onoff)
{
  Am_WinDDAInfo info(hdc, dashes, ndash, w, onoff);

  MoveTo(hdc, x1, y1);

  LineDDA(x1, y1, x2, y2, (LINEDDAPROC)LineDDAProc, (LPARAM)&info);
  if (iseven(info.m_curdash))
    LineTo(hdc, x2, y2); // complete last dash
}

void /*GWFASTCALL*/ Am_WinDrawArrDashPolyline(HDC hdc, HINSTANCE hinst,
                                              const POINT *ppt, int npt,
                                              const char *dashes, int ndash,
                                              int w, BOOL onoff)
{
  Am_WinDDAInfo info(hdc, dashes, ndash, w, onoff);

  MoveTo(hdc, ppt[0].x, ppt[0].y);

  for (int i = 0; i < npt - 1; i++) {
    LineDDA(ppt[i].x, ppt[i].y, ppt[i + 1].x, ppt[i + 1].y,
            (LINEDDAPROC)LineDDAProc, (LPARAM)&info);
    if (iseven(info.m_curdash))
      LineTo(hdc, ppt[i + 1].x, ppt[i + 1].y); // complete last dash
  }
}
