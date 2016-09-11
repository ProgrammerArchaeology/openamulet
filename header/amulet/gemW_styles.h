#if !defined(GWSTYLE_H)
#define GWSTYLE_H

//#pragma pack()

DWORD WinRopFunc (Am_Draw_Function amdrwfn);
int WinRop2Func (Am_Draw_Function amdrwfn);
int WinPolyFillMode (Am_Fill_Poly_Flag fpflag);
//void Am_Initialize_Color_Map ();

class Am_Style_Data : public Am_Wrapper
{
friend class Am_Style;
  Am_WRAPPER_DATA_DECL(Am_Style)
  Am_DECL_WINCLEAN(Am_Style_Data)
public:
  void Print (std::ostream& os) const;
  Am_Style_Data (Am_Style_Data* proto);
  Am_Style_Data (float r, float g, float b,
	  	short thickness,  Am_Line_Cap_Style_Flag cap,
		Am_Join_Style_Flag join,  Am_Line_Solid_Flag line_flag,
		const char* dash_l, int dash_l_length,
		Am_Fill_Solid_Flag fill_flag,
		Am_Fill_Poly_Flag poly, Am_Image_Array stipple);
  Am_Style_Data (const char* color_name,
	  	short thickness,  Am_Line_Cap_Style_Flag cap,
		Am_Join_Style_Flag join,  Am_Line_Solid_Flag line_flag,
		const char* dash_l, int dash_l_length,
		Am_Fill_Solid_Flag fill_flag,
		Am_Fill_Poly_Flag poly, Am_Image_Array stipple);
  //construct from Windows RGB
  Am_Style_Data (COLORREF wincolor);
  Am_Style_Data (const char* name, bool bit_is_on);
  Am_Style_Data (Am_Style_Data* proto, Am_Style_Data* new_color);

  virtual ~Am_Style_Data ();

  bool operator== (Am_Style_Data&)
    { return false; }

  bool operator== (Am_Style_Data&) const
    { return false; }

  virtual const char *Get_Color_Name ();
  
  Am_Line_Cap_Style_Flag LineCap () const { return m_cap_style; };
  Am_Line_Solid_Flag LineSolid () const { return m_line_solid; };
  Am_Fill_Poly_Flag FillPolyMode () const { return m_fill_poly; };
  Am_Fill_Solid_Flag FillSolid () const { return m_fill_solid; };
  const char* DashArr () const { return m_dash_list; };
  int DashArrLen () const { return m_dash_list_length; };

  bool Is_Stippled () const { return m_stipple_bitmap.Valid(); }
    
  void AdjustColor ();
  HBRUSH WinBrush (HDC hdc = 0);
  HPEN WinPen (int emulate=0);

public:
  static HBRUSH hbrNullBrush;
  static HPEN hpenNullPen;
  static COLORREF crefBlack;
  static COLORREF crefWhite;

  void inline copy (Am_Style_Data* proto);

  // color part
  float m_red, m_green, m_blue;
  char* m_color_name;

  // line style part
  short m_line_thickness;
  int m_dash_list_length;
  char* m_dash_list;
	  
  Am_Line_Cap_Style_Flag m_cap_style;
  Am_Join_Style_Flag m_join_style;
  Am_Line_Solid_Flag m_line_solid;

  // filling style part
  Am_Fill_Solid_Flag m_fill_solid;
  Am_Fill_Poly_Flag m_fill_poly;
  Am_Image_Array m_stipple_bitmap;

  HBRUSH m_hbrush;
  HPEN m_hpen;
  COLORREF m_cref;
};

#endif
