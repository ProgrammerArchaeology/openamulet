long Handle_Hit(WindowPtr mac_window, long param);
long Handle_CalcRgns(WindowPtr mac_window, long param);
Point Get_Origin(WindowPtr mac_window);
int Is_Color_GrafPort(WindowPtr mac_window);

pascal long
main(short /*var_code*/, WindowPtr mac_window, short message, long param)
{
  switch (message) {
  case wHit:
    return Handle_Hit(mac_window, param);
  case wCalcRgns:
    return Handle_CalcRgns(mac_window, param);
  }

  return 0;
}

long
Handle_Hit(WindowPtr mac_window, long param)
{
  Point mouse_loc = *(Point *)&param;
  WindowPeek wpeek = (WindowPeek)mac_window;
  return PtInRgn(mouse_loc, wpeek->contRgn) ? wInContent : wNoHit;
}

long
Handle_CalcRgns(WindowPtr mac_window, long /*param*/)
{
  Rect region_rect = mac_window->portRect;
  Point origin = Get_Origin(mac_window);

  OffsetRect(&region_rect, -origin.h, -origin.v);

  WindowPeek wpeek = (WindowPeek)mac_window;
  RectRgn(wpeek->strucRgn, &region_rect);
  RectRgn(wpeek->contRgn, &region_rect);

  return 0;
}

Point
Get_Origin(WindowPtr mac_window)
{
  return topLeft(Is_Color_GrafPort(mac_window)
                     ? (**((CGrafPtr)mac_window)->portPixMap).bounds
                     : mac_window->portBits.bounds);
}

int
Is_Color_GrafPort(WindowPtr mac_window)
{
  return (mac_window->portBits.rowBytes & 0x8000);
}
