#include <windows.h>
#include <windowsx.h>
#include <fstream>

#include <am_inc.h>
#include <amulet/impl/types_logging.h>
#include <amulet/impl/types_wrapper.h>
#include <amulet/impl/gem_flags.h>
#include <amulet/impl/am_image_array.h>
#include <amulet/impl/am_style.h>
#include <amulet/impl/am_cursor.h>

#include "amulet/gemW.h"
#include "amulet/gemW_misc.h"
#include "amulet/gemW_pattern.h"
#include "amulet/gemW_clean.h"
#include "amulet/gemW_styles.h"
#include "amulet/gemW_image.h"

#include "amulet/gemW_cursor.h"

////////////
// Am_Cursor

Am_WRAPPER_IMPL(Am_Cursor)

Am_Cursor::Am_Cursor ()
{
	data = (0L);
}
  
Am_Cursor::Am_Cursor (Am_Image_Array image, Am_Image_Array mask,
	Am_Style /*fg_color*/, Am_Style /*bg_color*/) //cannot have color cursors
{
	data = new Am_Cursor_Data(image, mask);
}
  
void Am_Cursor::Set_Hot_Spot (int x, int y)
{
	data -> Set_Hot_Spot(x, y);
}

void Am_Cursor::Get_Hot_Spot (int& x, int& y) const
{
	data -> Get_Hot_Spot(x, y);
}

void Am_Cursor::Get_Size (int& width, int& height)
{
	data -> Get_Size(width, height);
}

Am_Cursor Am_Default_Cursor;

//////////////////
// Am_Cursor_Data::Am_Wrapper

Am_WRAPPER_DATA_IMPL(Am_Cursor, (this))

// default constructor
Am_Cursor_Data::Am_Cursor_Data () :
	m_img((0L)), m_msk(NULL), m_hcurs(0)
{
    Am_WINCLEAN_CONSTR(Am_Cursor_Data)

	Set_Hot_Spot(0, 0);
}

// copy constructor
Am_Cursor_Data::Am_Cursor_Data (Am_Cursor_Data* proto) :
	m_img((0L)), m_msk(NULL), m_hcurs(0)
{
    Am_WINCLEAN_CONSTR(Am_Cursor_Data)
    
	HDC hdc = GetDC(0);
	m_img = new Am_WinImage(hdc, proto -> m_img, 0, 0, 0, 0);
	m_msk = new Am_WinImage(hdc, proto -> m_msk, 0, 0, 0, 0);
	ReleaseDC(0, hdc);

	int hsx, hsy;
	proto -> Get_Hot_Spot(hsx, hsy);
	Set_Hot_Spot(hsx, hsy);
}

Am_Cursor_Data::Am_Cursor_Data (Am_Image_Array image, Am_Image_Array mask) :
	m_img((0L)), m_msk(NULL), m_hcurs(0)
{
    Am_WINCLEAN_CONSTR(Am_Cursor_Data)

	PREPARE_DATA(Am_Image_Array, imgd, image)
	PREPARE_DATA(Am_Image_Array, mskd, mask)
	if (!imgd || !mskd || !imgd -> Valid() || !mskd -> Valid()) {
		DISCARD_DATA(imgd)
		DISCARD_DATA(mskd)
		return;
	}

	int cw = GetSystemMetrics(SM_CXCURSOR);
	int ch = GetSystemMetrics(SM_CYCURSOR);
	
	imgd -> Get_Hot_Spot(m_ptHotSpot.x, m_ptHotSpot.y);

	HDC hdc = GetDC(0);

	Am_WinImage tempimg(hdc, imgd -> WinImage(), 0, 0, 0, 0, cw, -ch);
	m_msk = new Am_WinImage(hdc, mskd -> WinImage(), 0, 0, 0, 0, cw, -ch);
	m_img = new Am_WinImage(CreateBitmap(cw, ch, 1, 1, (0L)), cw, ch);
	
	DISCARD_DATA(imgd)
	DISCARD_DATA(mskd)	

	HDC hdcmemd = CreateCompatibleDC(hdc);
	HDC hdcmems = CreateCompatibleDC(hdc);
	//prepare XOR-mask bitmap
	HBITMAP hbmoldd = SelectBitmap(hdcmemd, m_img->GetDDB(hdc));
	HBITMAP hbmolds = SelectBitmap(hdcmems, m_msk->GetDDB(hdc));
	BitBlt(hdcmemd, 0, 0, cw, ch, hdcmems, 0, 0, SRCCOPY);
	SelectBitmap(hdcmems, tempimg.GetDDB(hdc));
	BitBlt(hdcmemd, 0, 0, cw, ch, hdcmems, 0, 0, SRCERASE);
	SelectBitmap(hdcmems, hbmolds);
	SelectBitmap(hdcmemd, hbmoldd);
	DeleteDC(hdcmemd);
	DeleteDC(hdcmems);
	ReleaseDC(0, hdc);
}

Am_Cursor_Data::~Am_Cursor_Data ()
{
	Am_WINCLEAN_DESTR

	delete m_msk;
	delete m_img;
	
	DestroyCursor(m_hcurs);
}

void Am_Cursor_Data::Set_Hot_Spot (int x, int y)
{
	m_ptHotSpot.x = x;
	m_ptHotSpot.y = y;
}
    
void Am_Cursor_Data::Get_Hot_Spot (int& x, int& y) const
{
	x = m_ptHotSpot.x;
	y = m_ptHotSpot.y;
}

void Am_Cursor_Data::Get_Size (int& width, int& height)
{
	if (m_msk) {
		width = m_msk -> width();
		height = m_msk -> height();
	}
	else
		width = height = 0;
}

HCURSOR Am_Cursor_Data::WinCursor (HINSTANCE hinst)
{
#ifdef USE_WINCLEANER
	WinSetUsedRes();
#endif
	if (!m_hcurs && m_img && m_msk) {
		int cw, ch;
		Get_Size(cw, ch);
	
		m_hcurs = ::CreateCursor(hinst,
			m_ptHotSpot.x, m_ptHotSpot.y,
			cw, ch,
			m_msk -> image_bits(), m_img -> image_bits());
		m_msk -> delete_ddb();
		m_img -> delete_ddb();
	}
	return m_hcurs;
}

#ifdef USE_WINCLEANER
Am_IMPL_WINCLEAN_FORWRAPPER(Am_Cursor_Data)

BOOL Am_Cursor_Data::WinHasRes () const
{
	return (BOOL)(m_msk -> get_ddb() || m_img -> get_ddb());
}

BOOL Am_Cursor_Data::WinFreeRes ()
{
	BOOL bRes = FALSE;
	
	if (m_msk)
		bRes = bRes || m_msk -> delete_ddb();
	if (m_img)
		bRes = bRes || m_img -> delete_ddb();
	
	// cannot temprarily delete Windows cursor, for it may be in use!
	return bRes;
}
#endif
