#if !defined(GWREGION_H)
#define GWREGION_H

//#pragma pack()

class Am_WinRect : public tagRECT {
public:
	Am_WinRect ()
		{ SetEmpty(); };
	Am_WinRect (int the_left, int the_top,
		unsigned int the_width, unsigned int the_height)
	    { Set(the_left, the_top, the_width, the_height); };
public:
	void Set (int the_left, int the_top,
		unsigned int the_width, unsigned int the_height)
		{ ::SetRect(this, the_left, the_top, the_left + the_width, the_top + the_height); };
	void SetEmpty ()
		{ ::SetRectEmpty(this); };
	bool operator == (const Am_WinRect& wrect) const
		{ return ::EqualRect(this, &wrect)!=0; };
	Am_WinRect operator + (const Am_WinRect& wrect) const
		{ Am_WinRect rc; ::UnionRect(&rc, this, &wrect); return rc; };
	Am_WinRect operator * (const Am_WinRect& wrect) const
		{ Am_WinRect rc; ::IntersectRect(&rc, this, &wrect); return rc; };
	Am_WinRect& operator += (const Am_WinRect& wrect)
		{ Am_WinRect rc; ::UnionRect(&rc, this, &wrect); *this = rc; return *this; };
	Am_WinRect& operator *= (const Am_WinRect& wrect)
		{ Am_WinRect rc; ::IntersectRect(&rc, this, &wrect); *this = rc; return *this; };
	BOOL IsEmpty () const
		{ return ::IsRectEmpty(this); };
protected:
	Am_WinRect& operator = (const RECT& rc)
		{ *(RECT*)this = rc; return *this; };
};

class Am_WinRegion : public Am_Region {
	AM_DECL_WINNARROW(Region)
	AM_DECL_WINCLEAN(Am_WinRegion)
public:
	Am_WinRegion ();
	Am_WinRegion (int the_left, int the_top,
		unsigned int the_width, unsigned int the_height);
	Am_WinRegion (const Am_WinRegion& the_region);
	virtual ~Am_WinRegion ();
public:
	virtual void Destroy ()
		{ delete this; };
	virtual void Clear ();
	virtual void ClearAll ();
	virtual void Set (int the_left, int the_top,
			   unsigned int the_width,
			   unsigned int the_height);
	
	void Push (const Am_WinRect& wrect);
	virtual void Push (/*const*/ Am_Region *the_region);
	virtual void Push (int the_left, int the_top,
			    unsigned int the_width,
			    unsigned int the_height);
	virtual void Pop ();
	bool IsStackEmpty () const //not in Am_Region
		{ return m_history.IsEmpty(); };
	
	void Union (const Am_WinRect& wrect);
	virtual void Union (const Am_Region *the_region); // not in Am_Region
	virtual void Union (int the_left, int the_top,
			     unsigned int the_width,
			     unsigned int the_height);

	void Intersect (const Am_WinRect& wrect);
	virtual void Intersect (const Am_Region *the_region); // not in Am_Region
	virtual void Intersect (int the_left, int the_top,
				 unsigned int the_width,
				 unsigned int the_height);

	virtual bool In (int x, int y); // const;
	virtual bool In (int x, int y, unsigned int width,
				unsigned int height, bool &total); // const;
	virtual bool In (/*const*/ Am_Region *rgn, bool &total); // const;
	
	HRGN WinRegion ();
	const Am_WinRect& WinRect () const { return operator const Am_WinRect&(); };
	
protected:
	operator const Am_WinRect& () const
		{ return m_rect; };
protected:
	HRGN m_hrgn;
	Am_WinRect m_rect;
	Am_List_Ptr m_history;
};

#endif
