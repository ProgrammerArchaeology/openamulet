#include <windows.h>
#include <windowsx.h>
#include <fstream>
#include <strstream>
#include <string.h>
#include <limits.h>

#include <am_inc.h>
#include <amulet/impl/types_logging.h>
#include <amulet/impl/types_wrapper.h>
#include <amulet/impl/gem_flags.h>
#include <amulet/impl/am_image_array.h>
#include <amulet/impl/am_style.h>

#include "amulet/gemW.h"
#include "amulet/gemW_misc.h"
#include "amulet/gemW_pattern.h"
#include "amulet/gemW_clean.h"
#include "amulet/gemW_image.h"
#include "amulet/gemW_styles.h"

/////////////////
// Am_Image_Array

Am_WRAPPER_IMPL (Am_Image_Array)

// Am_Image_Array constructors.

Am_Image_Array::Am_Image_Array ()
{
  data = (0L);
}

Am_Image_Array::Am_Image_Array (const char* file_name)
{
  data = new Am_Image_Array_Data(file_name);

// if we were unable to open image, make ourself invalid
  if (!data->m_image) { delete data; data=0; }
}

Am_Image_Array::Am_Image_Array (unsigned int width, 
				unsigned int height, int depth,
				Am_Style initial_color)
{
  data = new Am_Image_Array_Data(width, height, depth, initial_color);
}

Am_Image_Array::Am_Image_Array (int percent)
{
  data = new Am_Image_Array_Data(percent);
}

Am_Image_Array::Am_Image_Array (char *bit_data, int height, int width)
{
  data = new Am_Image_Array_Data(bit_data, height, width);
}

// Am_Image_Array creators

Am_Image_Array Am_Image_Array::make_diamond ()
{
  Am_Image_Array im;
  im.data = new Am_Image_Array_Data(-1);
  return im;
}

Am_Image_Array Am_No_Image((Am_Wrapper*)(0L));

// Am_Image_Array setters and getters

int Am_Image_Array::Get_Bit(int x, int y)
{
  if (data)
    return data -> Get_Bit(x, y);
  return 0;
}
void Am_Image_Array::Set_Bit(int x, int y, int val)
{
  if (data) {
    data = Am_Image_Array_Data::Narrow(data->Make_Unique());
    data -> Set_Bit(x, y, val);
  }
}

// Returns in storage the RGB values for each pixel in the image
// using 1 byte each for R, for G and for B.  So ...
// storage must be large enough to accomodate 3*width*height bytes.
// If top_first return RGB values for top row first
// else return RGB for bottom row first.
// Returns true if successful (i.e. implemented).
bool Am_Image_Array::Get_RGB_Image(unsigned char * storage,
				   bool top_first)
{
  return false;
}

int Am_Image_Array::Write_To_File (const char* file_name,
				   Am_Image_File_Format form)
{
  if (data)
    return data -> Write_To_File(file_name, form);
  return 0;
}

void Am_Image_Array::Get_Size (int& width, int& height) const
{
  if (data)
    data -> Get_Size(width, height);
  else
    width = height = 0;
}
/*
void Am_Image_Array::Get_Hot_Spot (int& x, int& y) const
{
  if (data) data -> Get_Hot_Spot(x, y);
  else x = y = 0;
}

void Am_Image_Array::Set_Hot_Spot (int x, int y)
{
  if (data) {
    data = Am_Image_Array_Data::Narrow(data->Make_Unique());
    data -> Set_Hot_Spot(x, y);
  }
}
*/

//////////////////////
// Am_Image_Array_Data::Am_Wrapper

Am_WRAPPER_DATA_IMPL(Am_Image_Array, (this))

// default constructor
Am_Image_Array_Data::Am_Image_Array_Data () :
  m_form(XBM_FILE_FORMAT), m_image((0L)), m_ptHotSpot(0, 0)
{
  Am_WINCLEAN_CONSTR(Am_Image_Array_Data)
  m_nGrayFactor = -1;
}

// copy constructor
Am_Image_Array_Data::Am_Image_Array_Data (Am_Image_Array_Data* proto) :
  m_form(XBM_FILE_FORMAT), m_image((0L)), m_ptHotSpot(0, 0)
{
  Am_WINCLEAN_CONSTR(Am_Image_Array_Data)
    
  HDC hdc = GetDC(0);
  m_image = new Am_WinBMPImage(hdc, proto -> m_image, 0, 0, 0, 0);
  ReleaseDC(0, hdc);

  int hsx, hsy;
  proto -> Get_Hot_Spot(hsx, hsy);
  Set_Hot_Spot(hsx, hsy);
}

// create by read from file
Am_Image_Array_Data::Am_Image_Array_Data (const char* file_name) :
  m_form(XBM_FILE_FORMAT), m_image((0L)), m_ptHotSpot(0, 0)
{
  Am_WINCLEAN_CONSTR(Am_Image_Array_Data)
    
  const char *ext = strrchr(file_name, '.');
  if (ext == (0L)) {
    Am_TRACE("** " << file_name << ": cannot load image" <<std::endl)
    return;
  } else ext++;

  // Change to an hourglass cursor
  SetCapture(GetFocus());
  HCURSOR hcursOld = SetCursor(LoadCursor((0L), IDC_WAIT));
  int res;

  // Load file
  if(_stricmp(ext, "BMP") == 0) {
    m_image = new Am_WinBMPImage;
    res = m_image->read(file_name);
    m_form = XBM_FILE_FORMAT;
  } else if(_stricmp(ext, "GIF") == 0) {
    m_image = new Am_WinGIFImage;
    res = m_image->read(file_name);
    m_form = GIF_FILE_FORMAT;
  }

  // Reset cursor
  SetCursor(hcursOld);
  ReleaseCapture();

  m_nGrayFactor = -1;
  Set_Hot_Spot(0, 0);
	
  // Display a message if image format is unknown
  if (!res) {
    delete m_image;
    m_image = (0L);
    Am_TRACE("** " << file_name << ": cannot load image" <<std::endl)
      }
}

// create blank image; 0=white
Am_Image_Array_Data::Am_Image_Array_Data (int width, int height, int depth,
					  Am_Style initial_color) :
  m_form(XBM_FILE_FORMAT), m_image((0L)), m_ptHotSpot(0, 0)
{
  Am_WINCLEAN_CONSTR(Am_Image_Array_Data)

  HDC hdc = GetDC(0);
  HBITMAP hbmpNew = CreateBitmap(width, height, 1, depth, (0L));
  //CreateCompatibleBitmap(hdc, width, height);
  HDC hdcMem = CreateCompatibleDC(hdc);

#if _MSC_VER >= 1200
  HBITMAP hbmpOld = reinterpret_cast<HBITMAP__*>(SelectObject(hdcMem, hbmpNew));
#else
  HBITMAP hbmpOld = SelectObject(hdcMem, hbmpNew);
#endif

  PREPARE_DATA(Am_Style, clrd, initial_color)
	
#if _MSC_VER >= 1200
  HBRUSH hbrushOld = reinterpret_cast<HBRUSH__*>(SelectObject(hdcMem, clrd? clrd -> WinBrush() : GetStockObject(WHITE_BRUSH)));
#else
  HBRUSH hbrushOld = SelectObject(hdcMem, clrd? clrd -> WinBrush() : GetStockObject(WHITE_BRUSH));
#endif
	
  PatBlt(hdcMem, 0, 0, width, height, PATCOPY);

  SelectObject(hdcMem, hbrushOld);
  SelectObject(hdcMem, hbmpOld);
  DeleteDC(hdcMem);
#if _MSC_VER >= 1200
  ReleaseDC(reinterpret_cast<HWND__*>(hdc), 0);
#else
  ReleaseDC(hdc, 0);
#endif

  m_image = new Am_WinBMPImage(hbmpNew, (unsigned)width, (unsigned)height);

  m_nGrayFactor = -1;
	
  DISCARD_DATA(clrd)
}

// create stipple image; 0=black; -1=diamond
Am_Image_Array_Data::Am_Image_Array_Data (int percent) :
  m_form(XBM_FILE_FORMAT), m_image((0L)), m_ptHotSpot(0, 0)
{
  Am_WINCLEAN_CONSTR(Am_Image_Array_Data)
    
  m_nGrayFactor = (percent >= 0)? ((percent < 100)? percent * 16 / 100 : 16) : -1;
	
  HBITMAP hbmpNew = CreateBitmap(8, 8, 1, 1,
				 (m_nGrayFactor >= 0)? (LPBYTE)stipple_bits[ 16 - m_nGrayFactor] :
				 (LPBYTE)diamond_bits);
  //CreateCompatibleBitmap(hdc, width, height);
  m_image = new Am_WinBMPImage(hbmpNew, 8, 8);
}

// create from "raw bits" (here: Windows bitmap data)
Am_Image_Array_Data::Am_Image_Array_Data (char *bit_data, int height, int width) :
  m_form(XBM_FILE_FORMAT), m_image((0L)), m_ptHotSpot(0, 0)
{
  Am_WINCLEAN_CONSTR(Am_Image_Array_Data)

  HBITMAP hbmpNew = CreateBitmap(height, width, 1, 1,
				 (LPBYTE)bit_data);
  m_image = new Am_WinBMPImage(hbmpNew, height, width);
  m_nGrayFactor = -1;
}

// create by stretching proto to be width X height
Am_Image_Array_Data::Am_Image_Array_Data (Am_Image_Array_Data* proto, int width, int height) :
  m_form(XBM_FILE_FORMAT), m_image((0L)), m_ptHotSpot(0, 0)
{
  Am_WINCLEAN_CONSTR(Am_Image_Array_Data)
    
  int pw, ph;
  proto -> Get_Size(pw, ph);

  if (pw && ph) {
    HDC hdc = GetDC(0);
    m_image = new Am_WinBMPImage(hdc, proto -> m_image, 0, 0, 0, 0, width, height);
    ReleaseDC(0, hdc);
	
    int hsx, hsy;
    proto -> Get_Hot_Spot(hsx, hsy);
		
    Set_Hot_Spot(hsx * width / pw, hsy * height / ph);
  }
  m_nGrayFactor = proto->m_nGrayFactor;
}

Am_Image_Array_Data::~Am_Image_Array_Data()
{
  Am_WINCLEAN_DESTR
#ifdef USE_WINCLEANER
  WinFreeRes();
#endif
  delete m_image;
}

int Am_Image_Array_Data::Get_Bit(int x, int y)
{
  return 0;
}
    
void Am_Image_Array_Data::Set_Bit(int x, int y, int val)
{
}

void Am_Image_Array_Data::Set_Hot_Spot (int x, int y)
{
  m_ptHotSpot.x = x;
  m_ptHotSpot.y = y;
}
    
void Am_Image_Array_Data::Get_Hot_Spot (int& x, int& y) const
{
  x = m_ptHotSpot.x;
  y = m_ptHotSpot.y;
}

void Am_Image_Array_Data::Get_Size (int& width, int& height)
{
  if (m_image) {
    width = m_image -> width();
    height = m_image -> height();
  } else width = height = 0;
}

int Am_Image_Array_Data::Write_To_File (const char* file_name,
					Am_Image_File_Format /*form*/) //returns 0 or error code
{
  if (!m_image) return 1;
	
  if (m_form == XBM_FILE_FORMAT)
    return m_image -> write(file_name);
  else {
    Am_WinBMPImage img;
    img = *m_image;
    return img.write(file_name);
  }
}

void Am_Image_Array_Data::WinShow(HDC hdc, short xfrom, short yfrom,
  short xto, short yto, short width, short height, bool masked,
  bool monochrome, bool invert, DWORD ropcode) 
{
#ifdef USE_WINCLEANER
  WinSetUsedRes();
#endif
  if (m_image)
    m_image -> show(hdc, xfrom, yfrom, xto, yto, width, height,
		    masked, monochrome, invert, ropcode);
}

HPALETTE Am_Image_Array_Data::WinPalette ()
{
#ifdef USE_WINCLEANER
  WinSetUsedRes();
#endif
  return m_image? m_image -> GetPalette() : 0;
}

HBITMAP Am_Image_Array_Data::WinBitmap (HDC hdc)
{
#ifdef USE_WINCLEANER
  WinSetUsedRes();
#endif
  return m_image? m_image -> GetDDB(hdc) : 0;
}

#ifdef USE_WINCLEANER
Am_IMPL_WINCLEAN_FORWRAPPER(Am_Image_Array_Data)

BOOL Am_Image_Array_Data::WinHasRes () const
{
  return (BOOL)(m_image && m_image -> get_ddb());
}

BOOL Am_Image_Array_Data::WinFreeRes ()
{
  return m_image? m_image -> delete_ddb() : FALSE;
}
#endif
