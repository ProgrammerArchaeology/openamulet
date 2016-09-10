#if !defined(GWCURSOR_H)
#define GWCURSOR_H

//#pragma pack()

class Am_Cursor_Data : public Am_Wrapper
{
friend class Am_Cursor;
	Am_WRAPPER_DATA_DECL(Am_Cursor)
	Am_DECL_WINCLEAN(Am_Cursor_Data)
public:
	Am_Cursor_Data ();
	Am_Cursor_Data (Am_Cursor_Data* proto);

	Am_Cursor_Data (Am_Image_Array image, Am_Image_Array mask);
	virtual ~Am_Cursor_Data ();

	bool operator== (Am_Cursor_Data&)
		{ return false; }

	bool operator== (Am_Cursor_Data&) const
		{ return false; }

public:
	void Set_Hot_Spot (int x, int y);
	void Get_Hot_Spot (int& x, int& y) const;

	void Get_Size (int& width, int& height);
public:
	HCURSOR WinCursor (HINSTANCE hinst);
protected:
	Am_Point m_ptHotSpot;
	Am_WinImage* m_img;
	Am_WinImage* m_msk;
	HCURSOR m_hcurs;
};

#endif
