#ifndef AM_IO_H
//lint -e750
#define AM_IO_H


#include <stdio.h>

#include <iostream>

#if defined(TRACE)
#define AM_TRACE(A)	{std::cout << A; }
#else
#define AM_TRACE(A)	{ ; }
#endif

#endif
