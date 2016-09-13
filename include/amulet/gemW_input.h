// Amulet
// Windows specific keyboard initialization stuff

#if !defined(GWINCHAR_H)
#define GWINCHAR_H

//#pragma pack()
#ifndef OA_VERSION
AM_DECL_MAP(Int2Int, int, int)
#endif

#include <windows.h> //virtual scan codes

void Am_Init_Key_Map();

short Am_WinScanToCode(WORD wScan, bool fChar);
BOOL
WinScanSpec(WORD wScan); //TRUE if wScan is NOT translated into WM_CHAR message

#define WinKeyPressed(V) ((GetKeyState(V) & 0x8000) != 0x0)
#define WinKeyToggled(V) (GetKeyState(V) & 0x0001)

#ifndef VK_0
#define VK_0 0x30
#define VK_9 0x39
#define VK_A 0x41
#define VK_Z 0x5A
#endif

#endif
