#ifndef AM_IO_H
//lint -e750
#define AM_IO_H


extern "C" {
#include <stdio.h>
}

#include <iostream>

#if defined(TRACE)
#define Am_TRACE(A)	{std::cout << A; }
#else
#define Am_TRACE(A)	{ ; }
#endif

#endif
