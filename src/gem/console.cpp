// Extended by Robert M. Muench and modified by the Amulet group, June 1997
// To use include console.h and define an external variable:
// extern SetupConsole setupConsole;

#include <windows.h>
#include <stdio.h>
#include <amulet/console.h>

#pragma init_seg(lib)

SetupConsole::SetupConsole() : myStreamIn(0), myStreamOut(0), myStreamErr(0)
{
  AllocConsole();
  HANDLE OutH = GetStdHandle(STD_OUTPUT_HANDLE);
  COORD buffer_size;

  buffer_size.X = 80; // 80 columns
  buffer_size.Y = 25; // 25 rows

  SetConsoleScreenBufferSize(OutH, buffer_size);

  freopen("CONIN$", "rt", stdin);
  freopen("CONOUT$", "wt", stdout);
  freopen("CONOUT$", "wt", stderr);

  myStreamIn = new std::ostream(std::cin);
  myStreamOut = new std::ostream(stdout);
  myStreamErr = new std::ostream(stderr);

  std::cin = *(myStreamIn);
  std::cout = *(myStreamOut);
  std::cerr = *(myStreamErr);
}

SetupConsole::~SetupConsole()
{
  delete (myStreamIn);
  delete (myStreamOut);
  delete (myStreamErr);
  FreeConsole();
}

void
SetupConsole::setFile(const char *fileName)
{
  delete (myStreamIn);
  delete (myStreamOut);
  delete (myStreamErr);

  freopen(fileName, "rt", stdin);
  freopen(fileName, "wt", stdout);
  freopen(fileName, "wt", stderr);

  /// Extended by Robert M. Muench
  myStreamIn = new std::ostream(stdin);
  myStreamOut = new std::ostream(stdout);
  myStreamErr = new std::ostream(stderr);

  std::cin = *(myStreamIn);
  std::cout = *(myStreamOut);
  std::cerr = *(myStreamErr);
}

SetupConsole setupConsole;
