/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file contains member function definitions for the Am_Drawonable_Impl
   object primarily concerned with drawing objects in windows.
*/

#include <stdio.h>
#include <stdlib.h>
#include <istd::ostream.h>

#include <am_inc.h>

#include GDEFS__H
#include GEM__H
#include MISC__H
#include <gemM.h>

#include <sioux.h>

static Handle gReservedMemory = (0L);

// Install default values in the Am_DASH_LIST variables (declared in gdefs.h)
//
const char Am_DEFAULT_DASH_LIST[Am_DEFAULT_DASH_LIST_LENGTH]
     = Am_DEFAULT_DASH_LIST_VALUE;

// Function Prototypes
void adjust_pen_drawfn (Am_Draw_Function f);
void adjust_pen_thickness (Am_Style ls);

// // // // // // // // // // // // // // // // // // // // // // // // // //
// utility functions
// // // // // // // // // // // // // // // // // // // // // // // // // //

// If the specified line-thickness is 0, then this function returns 1.
// Otherwise, it returns the specified line-thickness.
short
get_graphical_line_thickness(
  Am_Style ls )
{
  if (ls == Am_No_Style)
    return 0;
  else
  {
    short thickness;
    Am_Line_Cap_Style_Flag cap;
    ls.Get_Line_Thickness_Values (thickness, cap);
    return thickness ? thickness : 1;
  }
}

inline void
Am_Drawonable_Impl::adjust_pen_stipple(
  const Am_Image_Array& fill_stipple )
{
  static Pattern pat;
  Am_Image_Array_Data* stip = Am_Image_Array_Data::Narrow( fill_stipple );
  if( stip )
  {
    pat = stip->Get_Mac_Pattern( this );
    stip->Release();
    PenPat( &pat );
  }
  else
    PenPat (&qd.black);
}

inline void
adjust_pen_drawfn(
  Am_Draw_Function    f,
  Am_Fill_Solid_Flag  fill_flag )
{
  // As far as I can tell, the only way to draw a stipple transparently is
  // to draw it with an OR draw function.
  if( (fill_flag == Am_FILL_STIPPLED) && f == Am_DRAW_COPY )
    f = Am_DRAW_OR; // Don't adjust if another draw function was specifically requested

  switch( f )
  {
    case Am_DRAW_COPY:
      PenMode (patCopy);
      break;

    case Am_DRAW_OR:
      PenMode (patOr);
      break;

    case Am_DRAW_XOR:
      PenMode (patXor);
      break;
  }
}

inline
void adjust_pen_thickness (Am_Style ls) {
  short thickness = get_graphical_line_thickness (ls);
  PenSize (thickness, thickness);
}

// Defaults:
//  image = Am_No_Image;
//
void Am_Drawonable_Impl::set_pen_using_fill (const Am_Style& fill,
              Am_Draw_Function f,
              const Am_Image_Array& image)
{
  Am_Fill_Solid_Flag fill_flag;
  Am_Image_Array fill_stipple;
  //Am_Fill_Poly_Flag poly;
  static const RGBColor *fore_color;

    Am_Style_Data* fil = Am_Style_Data::Narrow(fill);
    if (fil) {
      // Part 1:  Color
      fore_color = fil->Get_Mac_Color (this);
      fil->Release ();
    RGBForeColor (fore_color);

//    poly = fill.Get_Fill_Poly_Flag ();

    // Part 2:  Stipple
      // If there is an image, use it for the stipple.  Else, use the
      // image stored in the style.
      if (image.Valid()) {
    fill_flag = Am_FILL_STIPPLED;
    fill_stipple = image;
    } else {
    fill_flag = fill.Get_Fill_Flag();
    fill_stipple = fill.Get_Stipple();
      }
      adjust_pen_stipple (fill_stipple);

    // Part 3:  Draw Function
    adjust_pen_drawfn (f, fill_flag);

    } // close "if (fil)..."
    else {
      // Should we ever get to this case?  I think set_pen_using_fill should
      // only be called when the fill was valid.
      ForeColor(whiteColor);
    }
}

void Am_Drawonable_Impl::set_pen_using_line (const Am_Style& line,
              Am_Draw_Function f) {
  static const RGBColor* fore_color;

    Am_Style_Data *lin = Am_Style_Data::Narrow(line);
    if (lin) {
      // Part 1:  Color
    fore_color = lin->Get_Mac_Color(this);
       lin->Release ();
       RGBForeColor (fore_color);
       // Part 2:  Stipple
    adjust_pen_stipple (line.Get_Stipple());
    // Part 3:  Draw Function
    adjust_pen_drawfn (f, line.Get_Fill_Flag());
    // Part 4:  Line Thickness
    adjust_pen_thickness (line);

  }
else
{
      // Should we ever get to this case?  I think set_pen_using_line should
      // only be called when the style was valid.
    ForeColor(whiteColor);
  }
  }

void
Am_Drawonable_Impl::set_fore_color(
  const Am_Style&  style)
{
  Am_Style_Data*  style_data = Am_Style_Data::Narrow(style);
  if (style_data) {
    const RGBColor*  fore_color = style_data->Get_Mac_Color(this);
    style_data->Release();

    RGBForeColor(fore_color);
  } else {
    ForeColor(blackColor);
  }
}

void
Am_Drawonable_Impl::set_back_color(
  const Am_Style&  style)
{
  Am_Style_Data*  style_data = Am_Style_Data::Narrow(style);
  if (style_data)
  {
    const RGBColor*  fore_color = style_data->Get_Mac_Color(this);
    style_data->Release();

    RGBBackColor(fore_color);
  }
  else
  {
    BackColor(whiteColor);
  }
}

// // // // // // // // // // // // // // // // // // // // // // // // // //
// Drawing routines
// // // // // // // // // // // // // // // // // // // // // // // // // //

// Defaults:
//   f = Am_DRAW_COPY
void Am_Drawonable_Impl::Draw_Rectangle (const Am_Style& ls,
          const Am_Style& fs,
           int left, int top, int width,
           int height, Am_Draw_Function f) {
#ifdef DEBUG
  if (!mac_port) {
  	Am_Error("mac_port not set for Am_Drawonable_Impl::Draw_Rectangle");
  	return;
  }
#endif

  Rect r;
  
  if ((width <= 0) || (height <= 0))
    return;

  if ((fs != Am_No_Style) || (ls != Am_No_Style)) {
    focus_on_this();
    PixMapHandle pix_map = lock_current_pixmap();

    int thickness = get_graphical_line_thickness (ls);
    int right = left + width;
    int bottom = top + height;
    if ( fs != Am_No_Style ) {
      // For what it's worth, don't let the fill region overlap the border
      SetRect (&r, left+thickness, top+thickness, right-thickness,
         bottom-thickness);
      set_pen_using_fill (fs, f);
      PaintRect (&r);
    }
    if ( ls != Am_No_Style ) {
      SetRect (&r, left, top, right, bottom);
      set_pen_using_line (ls, f);
      FrameRect (&r);
    }
  unlock_current_pixmap(pix_map);
  }
}

// Defaults:
//   angle1 = 0,
//   angle2 = 360,
//   f = Am_DRAW_COPY,p
//   asf = Am_ARC_PIE_SLICE
void Am_Drawonable_Impl::Draw_Arc (const Am_Style& ls,
           const Am_Style& fs,
           int left, int top,
           unsigned int width, unsigned int height,
           int angle1, int angle2,
           Am_Draw_Function f,
           Am_Arc_Style_Flag /* asf */)
{
#ifdef DEBUG
  if (!mac_port) {
  	Am_Error("mac_port not set for Am_Drawonable_Impl::Draw_Arc");
  	return;
  }
#endif

  static Rect r;
  if (!width || !height)
   return;

  if (( fs != Am_No_Style) || (ls != Am_No_Style)) {
    focus_on_this();
    PixMapHandle pix_map = lock_current_pixmap();

    int thickness = get_graphical_line_thickness (ls);
    int right = left + width;
    int bottom = top + height;

    if (fs != Am_No_Style) {
      // For what it's worth, don't let the fill region overlap the border
      SetRect (&r, left+thickness, top+thickness, right-thickness,
         bottom-thickness);
      set_pen_using_fill (fs, f);
      PaintArc (&r, 90-angle1, -angle2);
    }

    if (ls != Am_No_Style) {
      SetRect (&r, left, top, right, bottom);
      set_pen_using_line (ls, f);
      FrameArc (&r, 90-angle1, -angle2);
    }
  unlock_current_pixmap(pix_map);
  }
}

// Defaults:
//   f = Am_DRAW_COPY
void Am_Drawonable_Impl::Draw_Line (const Am_Style& ls,
          int x1, int y1, int x2, int y2,
          Am_Draw_Function f)
{
#ifdef DEBUG
  if (!mac_port) {
  	Am_Error("mac_port not set for Am_Drawonable_Impl::Draw_Line");
  	return;
  }
#endif
focus_on_this();
  PixMapHandle pix_map = lock_current_pixmap();

  set_pen_using_line (ls, f);
  int thickness = get_graphical_line_thickness (ls);
  thickness /= 2;
  x1 -= thickness;
  x2 -= thickness;
  y1 -= thickness;
  y2 -= thickness;

  MoveTo(x1, y1);
  LineTo(x2, y2);
  unlock_current_pixmap(pix_map);
}

/* Draw_Roundtangle
 *
 * Defaults:
 *   f = Am_DRAW_COPY
 */
void
Am_Drawonable_Impl::Draw_Roundtangle(
  const Am_Style& ls,
  const Am_Style& fs,
  int left,
  int top,
  int width,
  int height,
  unsigned short x_radius,
  unsigned short y_radius,
  Am_Draw_Function f )
{
#ifdef DEBUG
  if (!mac_port) {
  	Am_Error("mac_port not set for Am_Drawonable_Impl::Draw_Roundtangle");
  	return;
  }
#endif

  static Rect r;
  if ((width <= 0) || (height <= 0))
    return;

  if ((fs != Am_No_Style) || (ls != Am_No_Style)) {
    focus_on_this();
    PixMapHandle pix_map = lock_current_pixmap();

    int thickness = get_graphical_line_thickness (ls);
    int th2 = thickness + thickness;
    int right = left + width;
    int bottom = top + height;
    int corner_w = x_radius + x_radius;
    int corner_h = y_radius + y_radius;
    // For what it's worth, don't let the fill region overlap the border
    if ( fs != Am_No_Style ) {
      SetRect (&r, left+thickness, top+thickness,
         right-thickness, bottom-thickness);
      set_pen_using_fill (fs, f);
      PaintRoundRect (&r, corner_w-th2, corner_h-th2);
    }
    if ( ls != Am_No_Style ) {
      SetRect (&r, left, top, right, bottom);
      set_pen_using_line (ls, f);
      FrameRoundRect (&r, corner_w, corner_h);
    }
   unlock_current_pixmap(pix_map);
  }
}

/* Clear_Clip
 *   clear the clip_region
 */
void
Am_Drawonable_Impl::Clear_Clip ()
{
  clip_region->Clear ();
}

/* Set_Clip
 *   Sets the clip region, clearing any other regions currently on the stack.
 *
 *   BUG: The body of this function should really be "clip_region->Set (region)",
 *   where Am_Region_Impl::Set (Am_Region_Impl *region) does the copy, but
 *   no such function is defined in the header file.
 */
void Am_Drawonable_Impl::Set_Clip(
  Am_Region* region )
{
  RgnHandle mac_rgn = ((Am_Region_Impl*)region)->region_to_use ();
  if (mac_rgn) {
    clip_region->Clear();
    RgnHandle new_rgn = NewRgn();
    CopyRgn (mac_rgn, new_rgn);
    ((Am_Region_Impl*)clip_region)->mac_rgns[0] = new_rgn;
  }
  else
    clip_region->Clear();
}

/* Set_Clip
 *
 */
void
Am_Drawonable_Impl::Set_Clip(
  int left,
  int top,
  unsigned int width,
  unsigned int height )
{
  clip_region->Set (left, top, width, height);
}

/* Push_Clip
 *
 */
void
Am_Drawonable_Impl::Push_Clip(
  Am_Region* region )
{
  clip_region->Push (region);
}

/* Push_Clip
 *
 */
void
Am_Drawonable_Impl::Push_Clip(
  int left,
  int top,
  unsigned int width,
  unsigned int height )
{
  clip_region->Push (left, top, width, height);
}

/* Pop_Clip
 *
 */
void
Am_Drawonable_Impl::Pop_Clip()
{
  clip_region->Pop ();
}

/* In_Clip
 *
 */
bool
Am_Drawonable_Impl::In_Clip(
  int x,
  int y )
{
  return clip_region->In (x, y);
}

/* In_Clip
 *
 */
bool
Am_Drawonable_Impl::In_Clip(
  int left,
  int top,
  unsigned int width,
  unsigned int height,
  bool &total )
{
  return clip_region->In (left, top, width, height, total);
}

bool Am_Drawonable_Impl::In_Clip (Am_Region *rgn, bool &total)
{
  return clip_region->In (rgn, total);
}

void Am_Drawonable_Impl::Clear_Area(int arg_left, int arg_top,
            int arg_width, int arg_height)
{
#ifdef DEBUG
  if (!mac_port) {
  	Am_Error("mac_port not set for Am_Drawonable_Impl::Clear_Area");
  	return;
  }
#endif

  Rect windRect;
  SetRect( &windRect, arg_left, arg_top,
                      arg_left+arg_width, arg_top+arg_height );
  focus_on_this();
  Am_Style_Data* fil = Am_Style_Data::Narrow( background_color );
  
  PixMapHandle pix_map = lock_current_pixmap();
  if (pix_map) {
    if( fil ) {
      const RGBColor *my_rgb_color = fil->Get_Mac_Color( this );
      fil->Release();
      RGBBackColor( my_rgb_color );
    }
    else
      BackColor( whiteColor );

   EraseRect( &windRect );
  }
  unlock_current_pixmap(pix_map);
}

/* sets the cursor for this drawonable */
void Am_Drawonable_Impl::Set_Cursor(Am_Cursor new_cursor)
{
  cursor = new_cursor;

  Am_Cursor_Data*  cursor_data = Am_Cursor_Data::Narrow(cursor);
  if( mac_cursor ) {
    Cursor* tmp = cursor_data->Get_Mac_Cursor(this);
    if (tmp) mac_cursor = tmp;
  }
  cursor_data->Release();
}

// mac os initialization

// Currently I can get the grow zone proc called successfully once.
// However if it warns about low mem and then gets called again, the
// system hangs. So for now in order to increase stability, we will quit
// when this is called.
//
// I think the problem is that on the second call there is not enough
// memory to create the alert. Also, the debugger seems to show that
// gReservedMemory == nil, even on the first call.

long
GrowZoneCallBack(
  Size /* bytesNeeded */ )
{
  long oldA5 = SetCurrentA5();

//  if( kReservedMemSize < bytesNeeded || gReservedMemory == (0L) )
//  { // we don't have enough, warn the user and abort
    CautionAlert( kOutOfMemory, (0L) );
    SetA5( oldA5 );
    ExitToShell();
//  }
/*
  // else we can free the memory, so do it and warn the user
  if( gReservedMemory != nil )
  {
    DisposeHandle( gReservedMemory );
    gReservedMemory = nil;
  }

  short itemHit = CautionAlert( kLowOnMemory, (0L) );
  if( itemHit == 1 ) // item 1: quit; item 2: continue
  {
    SetA5( oldA5 );
    ExitToShell();
  }

*/
  SetA5( oldA5 );
  return kReservedMemSize;
}

void
mac_initialize()
{
  if( mac_inited )
    return;

  // Maximize the Applications Heap
  Size size;
  MaxApplZone();
  MaxMem( &size );

  // Make sure the main monitor is using a 5 bits per color inverse
  // table rather than the default 4. This will make CopyBits
  // operations using arithmetic transfer modes a bit more smooth
  // (but uses a little more memory).
	
  {
    GDHandle		aDevice;
		
    aDevice = GetGDevice();
    SetGDevice( GetMainDevice() );
    MakeITable( (0L), NULL, 5 );
    SetGDevice( aDevice );
  }

  // Initialize all the needed managers.
  InitGraf( &qd.thePort );
  InitFonts();
  InitWindows();
  InitMenus();
  TEInit();            // For the TextEdit scrap -- See Inside Mac I-383
  InitDialogs( nil );  // See Inside Mac I-411
  InitCursor();

  // Allow MacOS to send keyUp events to gemM_input.cc  By default, Amulet
  // defines function keys to simulate leftdown, middledown, and rightdown.
  // If we didn't get key-up events, we couldn't generate up-events for
  // these keys.
  SetEventMask( everyEvent );

  //std::flush any events pending on the OS queue
  FlushEvents( everyEvent, 0 );

  // Construct menus
  Handle menuBar = GetNewMBar( 128 );
  SetMenuBar( menuBar );
  DisposeHandle( menuBar );
  AppendResMenu( GetMenuHandle( 128 ), 'DRVR' );
  DrawMenuBar();

/*
  Str255  apple_title = "\p ";
  apple_title[1] = appleMark;

  MenuHandle  apple_menu = NewMenu(APPLE_MENU_ID, apple_title);
  AppendMenu(apple_menu, "\pAbout Amuletƒ;(-");
  AppendResMenu(apple_menu, 'DRVR');

  MenuHandle  file_menu = NewMenu(FILE_MENU_ID, "\pFile");
  AppendMenu(file_menu, "\pQuit/Q");

  InsertMenu(apple_menu, 0);
  InsertMenu(file_menu, 0);
  DrawMenuBar();
*/
  // Init SIOUX
  SIOUXSettings.initializeTB = false;
  SIOUXSettings.standalone = false;
  SIOUXSettings.setupmenus = false;
  SIOUXSettings.autocloseonquit = true;
  SIOUXSettings.asktosaveonclose = false;

  // Initalize Amulet Pathname
  Am_Init_Pathname();

  // Init grow-zone
  gReservedMemory = NewHandle( kReservedMemSize );
  GrowZoneUPP growZoneUPP = NewGrowZoneProc( GrowZoneCallBack );
  SetGrowZone( growZoneUPP );

  mac_inited = true;
}

class mac_init_object
{
 public:
  mac_init_object()
  {
    mac_initialize();
  }
};

mac_init_object  am_mac_init;

/***********************************************
 * lock and unlock pixmap
 *
 */
 
PixMapHandle lock_current_pixmap()
{
  CGrafPtr origPort;
  GDHandle origDev;
  GetGWorld(&origPort, &origDev);
  PixMapHandle pixMapH;
  pixMapH = GetGWorldPixMap(origPort);
  bool good = LockPixels(pixMapH);
  if (!good) {
  	Am_Error("Cannot lock current pixmap");
  	return (0L);
  } else
    return pixMapH;
}
void unlock_current_pixmap(PixMapHandle pixMapH)
{
  UnlockPixels(pixMapH);
}
