#if !defined(GWIMAGE_H)
#define GWIMAGE_H

#include "gemW_winimage.h"
#include "gemW_bmpimage.h"
#include "gemW_gifimage.h"

//#pragma pack()

class Am_Image_Array_Data : public Am_Wrapper
{
friend class Am_Image_Array;
  AM_WRAPPER_DATA_DECL(Am_Image_Array)
  AM_DECL_WINCLEAN(Am_Image_Array_Data)
public:
  Am_Image_Array_Data ();
  Am_Image_Array_Data (Am_Image_Array_Data* proto);
  // create by read from file
  Am_Image_Array_Data (const char* file_name);
  // create blank image; 0=white
  Am_Image_Array_Data (int width, int height, int depth,
		       Am_Style initial_color = Am_No_Style);
  // create stipple image; 0=black; -1=diamond
  Am_Image_Array_Data (int percent);
  // create from "raw bits" (here: Windows bitmap data)
  Am_Image_Array_Data (char *bit_data, int height, int width);
  // create by stretching proto to be width X height
  Am_Image_Array_Data (Am_Image_Array_Data* proto, int width, int height);
  virtual ~Am_Image_Array_Data();

  bool operator== (Am_Image_Array_Data&)
    { return false; }

  bool operator== (Am_Image_Array_Data&) const
    { return false; }

public:
  virtual int Get_Bit (int x, int y);
  virtual void Set_Bit (int x, int y, int val);
  virtual void Set_Hot_Spot (int x, int y);
  virtual void Get_Hot_Spot (int& x, int& y) const;
  virtual void Get_Size (int& width, int& height);

  virtual int Write_To_File (const char* file_name,
			     Am_Image_File_Format form);
  //returns 0 or error code

  BOOL Valid () const
    { return m_image != (0L); };
	
  void WinShow(HDC hdc, short xfrom = 0, short yfrom = 0,
	       short xto = 0,   short yto = 0,
	       short width = 0, short height = 0,
	       bool masked = false, bool monochrome = false,
	       bool invert = false, DWORD ropcode = SRCCOPY);

  HPALETTE WinPalette ();
  HBITMAP WinBitmap (HDC hdc);
  //operator HBITMAP () const { return WinBitmap(); };
  int WinGrayFactor () const { return m_nGrayFactor; };

  unsigned WinNumColors () const { return m_image? m_image -> numcolors() : 0; };
  bool WinIsTransparent() const { return m_image? (m_image->is_transparent()!=0) : false; };
  LPSTR WinBits () const { return m_image? m_image -> image_bits() : (0L); };
	
  Am_WinImage* WinImage () const { return m_image; };
protected:
  Am_Image_File_Format m_form;
  Am_WinImage* m_image;

  Am_Point m_ptHotSpot;
  int m_nGrayFactor;
};

#endif
