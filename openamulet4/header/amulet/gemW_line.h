#if !defined(GWLINE_H)
#define GWLINE_H

//#pragma pack()

void /*GWFASTCALL*/ Am_WinDrawArrDashLine (HDC hdc, HINSTANCE hinst,
	int x1, int y1, int x2, int y2, const char* dashes, int ndash,
	int w = 1, BOOL onoff = TRUE);

void /*GWFASTCALL*/ Am_WinDrawArrDashPolyline (HDC hdc, HINSTANCE hinst,
	const POINT* ppt, int npt, const char* dashes, int ndash,
	int w = 1, BOOL onoff = TRUE);

#endif
