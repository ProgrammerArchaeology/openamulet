#ifndef GW_DRAWONABLE
#define GW_DRAWONABLE

#include <am_inc.h>

//#include GEM__H
#include "amulet/impl/gem_event_handlers.h"
#include "amulet/impl/am_time.h"
#include "amulet/impl/am_font.h"
#include "amulet/impl/gem_misc.h"

#ifdef OA_VERSION
	#include <amulet/univ_map_oa.hpp>
	
	// defined in univ_map.cpp
	typedef	OpenAmulet::Map<HWND,void*> Am_Map_HModule2Ptr;
	typedef OpenAmulet::Map_Iterator<Am_Map_HModule2Ptr> Am_Map_HModule2Ptr_Iterator;
#else
	#include UNIV_MAP__H
#endif

//#pragma pack()

class Am_WinDrawonable;

struct Am_WinNotifyInfo;

typedef void (*Am_WinNotifyFunc) (const Am_WinNotifyInfo*);

struct Am_WinNotifyInfo
{
	union
	{	
		RECT m_rc;
		bool m_b;	
		Am_Input_Event* m_event;
#ifndef GCC
		struct
		{
			int m_left;
			int m_top;
			int m_width;
			int m_height;
		};						
#else
#define m_left m_rc.left
#define m_top m_rc.top
#define m_width m_rc.right
#define m_height m_rc.bottom
#endif
	};

	Am_WinNotifyInfo(Am_WinNotifyFunc func, Am_WinDrawonable* drw, const RECT& rc) :
	m_func(func), m_drw(drw), m_rc(rc)
	{ 
	};
	Am_WinNotifyInfo(Am_WinNotifyFunc func, Am_WinDrawonable* drw, bool b) :
	m_func(func), m_drw(drw), m_b(b)
	{ 
	};
	Am_WinNotifyInfo(Am_WinNotifyFunc func, Am_WinDrawonable* drw,
		int left, int top, int width, int height) :
	m_func(func), m_drw(drw)
#ifndef GCC
        , m_left(left), m_top(top), m_width(width), m_height(height)
#endif
	{ 
#ifdef GCC
	m_left=left; m_top=top; m_width=width; m_height=height;
#endif
	};
	Am_WinNotifyFunc m_func;
	Am_WinDrawonable* m_drw;
};


//------------------
// Am_WinDrawonable 
//------------------

#define Am_WinDrawonable_CLASS		"Am_GW_Window"
#define Am_WinDrawonable_SBT_CLASS	"Am_GW_SBt_Window"

class Am_WinDrawonable : public Am_Drawonable
{
	public:
		// CREATORS
			Am_WinDrawonable(
				int 					l = 0,
				int 					t = 0,
				unsigned int 			w = 100,
				unsigned int 			h = 100,
				const char				*tit = "",
				const char				*icon_tit = (0L),
				bool 					vis = true,
				bool 					initially_iconified = false,
				Am_Style 				back_color = Am_No_Style,			 
				bool 					save_under_flag = false,
				int 					min_w = 1, // can't have 0 size windows
				int 					min_h = 1,
				int 					max_w = 0,  // 0 is illegal so means no max
				int 					max_h = 0,
				bool 					title_bar_flag = true,
				bool 					clip_by_children_flag = true,
				Am_Input_Event_Handlers	*evh = (0L));
			virtual ~Am_WinDrawonable();

	static void WinInit();
	static void WinUninit();
	static BOOL WinDoIdle();

	static void NotifyIconified(const Am_WinNotifyInfo* info);
	static void NotifyFrameSize(const Am_WinNotifyInfo* info);
	static void NotifyDestroy(const Am_WinNotifyInfo* info);
	static void NotifyConfigure(const Am_WinNotifyInfo* info);	  
	static void NotifyExposure(const Am_WinNotifyInfo* info);
	static void NotifyBackground(const Am_WinNotifyInfo* info);

	static bool ProcessQueuedEvents();

	// multimedia timer handler
	static void WaitMessageWithTimeout(unsigned long milliseconds);
	static UINT idTimer;
	static DWORD idThread;
	static void CALLBACK TimeProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);

	// Overridden Am_Drawonable methods

	// Get the root Am_Drawonable.  This is not a member function since
	// don't necessarily have an object to start with.  String parameter
	// used under X to specify a screen.  This also causes any required
	// initialization to happen.  It is OK to call this more than once.
	static Am_Drawonable* Get_Root_Drawonable(const char* screen = (0L));

	// parent is the Am_Drawonable this is sent to
	virtual Am_Drawonable* Create(int l = 0, int t = 0,
		unsigned int w = 100, unsigned int h = 100,
		const char* tit = "",
		const char* icon_tit = "",
		bool vis = true,
		bool initially_iconified = false,
		Am_Style back_color = Am_No_Style,	   
		bool save_under_flag = false,
		int min_w = 1, // can't have 0 size windows
		int min_h = 1,
		int max_w = 0,  // 0 is illegal so means no max
		int max_h = 0,
		bool title_bar_flag = true,
		bool query_user_for_position = false,
		bool query_user_for_size = false,
		bool clip_by_children_flag = true,
		Am_Input_Event_Handlers *evh = (0L));

	public:
	// the Am_Drawonable this method is invoked on is the one being destroyed
	virtual void Destroy();

	virtual Am_Drawonable* Create_Offscreen(int width = 0, int height = 0, Am_Style back_color = Am_No_Style);
	virtual void Reparent(Am_Drawonable *new_parent);
	virtual bool Inquire_Window_Borders(int& left_border, int& top_border, int& right_border, int& bottom_border, int& outer_left, int& outer_top); // const;

	virtual void Raise_Window(/*const*/ Am_Drawonable *target_d); // const;  //to top
	virtual void Lower_Window(/*const*/ Am_Drawonable *target_d); // const;  //to bottom
	virtual void Set_Iconify(bool iconify);
	virtual void Set_Title(const char* new_title);
	virtual void Set_Icon_Title(const char* new_title);
	virtual void Set_Position(int new_left, int new_top);
	virtual void Set_Size(unsigned int new_width, unsigned int new_height);
	virtual void Set_Max_Size(unsigned int new_width, unsigned int new_height);
	virtual void Set_Min_Size(unsigned int new_width, unsigned int new_height);
	virtual void Set_Visible(bool vis);
	virtual void Set_Titlebar(bool new_title_bar);
	virtual void Set_Background_Color(Am_Style new_color);
	virtual void Print(std::ostream& os) const;

	/*
	bool Get_Iconify () const { return iconified; }
	const char* Get_Title () const { return title; }
	const char* Get_Icon_Title () const { return icon_name; }
	void Get_Position (int& l, int& t) const { l = left; t = top; }
	void Get_Size (int& w, int& h) const { w = width; h = height; }
	void Get_Max_Size (int& w, int& h) const { w = max_width; h = max_height; }
	void Get_Min_Size (int& w, int& h) const { w = min_width; h = min_height; }
	bool Get_Visible () const { return visible; }
	void Get_Titlebar (bool& title_bar_flag) const
	{ title_bar_flag = title_bar; width = border_width; }

	int Get_Depth () const { return depth; }

	void Get_Values (int& l, int& t, int& w, int& h,
	     const char*& tit, const char*& icon_tit, bool& vis,
	     bool& iconified_now, Am_Style*& back_color,
	     unsigned int& border_w, bool& save_under_flag,
	     int& min_w, int& min_h, int& max_w, int& max_h,
	     bool& query_user_for_position,
	     bool& query_user_for_size,
	     bool& title_bar_flag,
	     bool& clip_by_children_flag,
	     int& bit_depth) const
	{ l = left;  t = top;  w = width;  h = height;
	tit = title;
	icon_tit = icon_name;
	vis = visible;
	iconified_now = iconified;
	back_color = background_color;
	border_w = border_width;
	save_under_flag = save_under;
	min_w = min_width;  min_h = min_height;
	max_w = max_width;  max_h = max_height;
	query_user_for_position = ask_position;
	query_user_for_size = ask_size;
	title_bar_flag = title_bar;
	clip_by_children_flag = clip_by_children;
	bit_depth = depth;
	}
	*/

	// general operations
	virtual void Beep() /*const*/{ ::MessageBeep(MB_OK); };

	// destination for bitblt is the Am_Drawonable this message is sent to
	virtual void Bitblt(int d_left, int d_top, int width, int height, Am_Drawonable* source, int s_left, int s_top, Am_Draw_Function df = Am_DRAW_COPY);

	virtual void Clear_Area(int c_left, int c_top, int c_width, int c_height);
	virtual void Fill_Area(Am_Style fill, int c_left, int c_top, int c_width, int c_height); // not in Am_Drawonable
	virtual void Flush_Output();

	// Note: to translate to screen coordinates, pass your root drawonable
	// as dest_d.
	virtual void Translate_Coordinates(int src_x, int src_y,
		/*const*/Am_Drawonable *src_d,
		int& dest_x_return,
		int& dest_y_return); // const;

	// Translates a point from drawonable that hasn't neccessarily been
	// created to screen coordinates.  This function only works on root
	// drawonables.
	virtual void Translate_From_Virtual_Source(
		int src_x, int src_y,
		bool title_bar, int border_width,
		int& dest_x_return,
		int& dest_y_return);

	// Am_Image_Array operations:  must have an Am_Drawonable to get the size
	// of an image since it may need to be created
	virtual void Get_Image_Size(const Am_Image_Array& image, int& ret_width, int& ret_height);

	// verify that image is loaded (or loadable)
	virtual bool Test_Image(const Am_Image_Array& image) const;

	// Am_Font operations, must have a Am_Drawonable and a Am_Font to get sizes
	virtual int Get_Char_Width(const Am_Font& font, char c = '\0'); //const;
	// if c == '\0' returns aver. char width
	virtual int Get_String_Width(const Am_Font& font, const char* s, int len); //const;

	// The total height of the bounding rectangle for this string, or any
	// string in this font, is ascent + descent.  The left_bearing is the
	// distance from the origin of the text to the first "inked" pixel.
	// The right_bearing is the distance from the origin of the text to
	// the last "inked" pixel.
	virtual void Get_String_Extents(const Am_Font& font, const char* s,
		int len, int& swidth, int& ascent, int& descent,
		int& left_bearing, int& right_bearing); //const;

	// The max ascent and descent include vertical spacing between rows
	// of text.   The min ascent and descent are computed on a per-char basis.
	virtual void Get_Font_Properties(const Am_Font& font,
		int& max_char_width, int& min_char_width,
		int& max_char_ascent, int& max_char_descent); //const;

	// Am_Polygon operations
	// polygon bounding box calculation
	virtual void Get_Polygon_Bounding_Box(const Am_Point_List& pl, const Am_Style& ls,
		int& out_left, int& out_top,
		int& width, int& height);

	// Clipping functions:
	// Note that there is only one GC for the screen, and there is only one
	// clip mask that is shared by all windows on that screen.  So as you set
	// the clip mask for a window, you are actually setting it for all windows
	// on the same screen as that window.
	virtual void Clear_Clip();
	virtual void Set_Clip(/*const*/Am_Region* region);
	virtual void Set_Clip(int left, int top, unsigned int width, unsigned int height);
	virtual void Push_Clip(/*const*/Am_Region* region);
	virtual void Push_Clip(int left, int top, unsigned int width, unsigned int height);
	virtual void Pop_Clip();
	Am_WinRegion* Get_Clip() const
	{ return m_clip; 
	}; // not in Am_Drawonable
	virtual bool In_Clip(int x, int y);
	virtual bool In_Clip(int left, int top, unsigned int width,
		unsigned int height, bool &total);
	virtual bool In_Clip(Am_Region *rgn, bool &total);

	// Specific Drawing functions

	virtual void Draw_Arc(const Am_Style& ls, const Am_Style& fs,
		int left, int top,
		unsigned int width, unsigned int height,
		int angle1 = 0, int angle2 = 360,
		Am_Draw_Function f = Am_DRAW_COPY,
		Am_Arc_Style_Flag asf = Am_ARC_PIE_SLICE);
	virtual void Draw_Image(int left, int top, int width, int height,
		const Am_Image_Array& image, int i_left = 0, int i_top = 0,
		const Am_Style& ls = Am_No_Style, //color of 'on' bits
		const Am_Style& fs = Am_No_Style, //for background behind image
		bool draw_monochrome = false,     //use ls and fs in place of colors
		Am_Draw_Function f = Am_DRAW_COPY);
	virtual void Draw_Line(const Am_Style& ls, 
		int x1, int y1, int x2, int y2,
		Am_Draw_Function f = Am_DRAW_COPY);
	virtual void Draw_Lines(const Am_Style& ls, const Am_Style& fs, 
		const Am_Point_Array& pl,
		Am_Draw_Function f = Am_DRAW_COPY);
	virtual void Draw_2_Lines(const Am_Style& ls, const Am_Style& fs,
		int x1, int y1, int x2, int y2, int x3, int y3,
		Am_Draw_Function f = Am_DRAW_COPY);
	virtual void Draw_3_Lines(const Am_Style& ls, const Am_Style& fs,
		int x1, int y1, int x2, int y2, int x3, int y3,
		int x4, int y4,
		Am_Draw_Function f = Am_DRAW_COPY);
	virtual void Draw_Rectangle(const Am_Style& ls, const Am_Style& fs,
		int left, int top, int width, int height,
		Am_Draw_Function f = Am_DRAW_COPY);
	virtual void Draw_Roundtangle(const Am_Style& ls, const Am_Style& fs,
		int left, int top, int width, int height,
		unsigned short x_radius,
		unsigned short y_radius,
		Am_Draw_Function f = Am_DRAW_COPY);
	virtual void Draw_Text(const Am_Style& ls, const char *s, int str_len,
		const Am_Font& font,
		int left, int top, 
		Am_Draw_Function f = Am_DRAW_COPY,
		const Am_Style& fs = Am_No_Style,
		bool invert = false);

	// cut buffer, eventually, this needs to be generalized
	virtual void Set_Cut_Buffer(const char *s);
	virtual char* Get_Cut_Buffer();

	// input handling

	virtual void Set_Enter_Leave(bool want_enter_leave_events);
	virtual void Set_Want_Move(bool want_move_events);
	virtual void Set_Multi_Window(bool want_multi_window);

	virtual void Get_Window_Mask(bool& want_enter_leave_events,
		bool& want_move_events,
		bool& want_multi_window); //const;

	virtual void Discard_Pending_Events();
	virtual void Set_Input_Dispatch_Functions(Am_Input_Event_Handlers* evh)
	{ m_evh = evh; 
	};
	virtual void Get_Input_Dispatch_Functions(Am_Input_Event_Handlers*& evh) //const
	{ evh = m_evh; 
	};
	/*    
	void Set_Data_Store (void* data)
	{ data_store = data; };
	void* Get_Data_Store () const
	{ return data_store; };
	*/
	// cursors
	virtual void Set_Cursor(Am_Cursor cursor);

	virtual Am_Drawonable* Get_Drawonable_At_Cursor();

	public:
	class Am_WinDC
	{
		public:
		Am_WinDC(const Am_WinDrawonable* drw);
		Am_WinDC(const Am_WinDC& dc);
		Am_WinDC(HWND hwnd);
		~Am_WinDC();
		operator HDC() const
		{ return m_hdc; 
		};
		void SelectTool(HGDIOBJ htool);
		void DeselectTool(HGDIOBJ htool);
		void DeselectAll();
		void Set_DC_Transparent_Stipple(const Am_Style& ls,
			const Am_Style& fs, int emulate=0);
		void Adjust_DC_Transparent_Stipple(const Am_Style& fs);
		void Set_DC(const Am_Style& ls, const Am_Style& fs,
			Am_Draw_Function f,int emulate=0);
		void Set_DC_Text(const Am_Style& ls, const Am_Style& fs,
			Am_Draw_Function f, bool invert);
		void Set_DC_Font(const Am_Font& font);
		protected:
		HDC m_hdc;
		HWND m_hwnd;
		Am_List_Long m_tools;
	};

	// Windows class management functions
	virtual const char* ClsGetName(bool fSaveBits = false) const;

//----------------------------------------
// 	WindowsOS window management functions 
//----------------------------------------
	#ifdef _WIN32
		friend int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,	LPSTR lpszCmdLine, int nCmdShow);
	#endif

	static 	LRESULT CALLBACK __export MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	virtual LRESULT DefWndProc(UINT message, WPARAM wParam, LPARAM lParam);

	//:Returns the windowsOS window-handle associated with this drawonable
	HWND WndGet() const
	{
		return(m_hwnd);
	};

	void 			WndAttach(HWND hwnd);
	void 			WndClose();
	bool 			WndCreate(bool fQueryPos = false, bool fQuerySize = false,	bool fSaveBits = false);
	void 			WndDestroy();
	HWND 			WndDetach();
	virtual void 	WndDraw(HDC hdc, const RECT& rc);
	void 			WndFillInfo(HWND hwnd);

	//:Returns the number of colors for the actual display
	// Is missnamed as Depth associates a plane-depth...
	static int 		WndGetColorDepth(HDC hdc=0);

	void 			WndIconify();
	static void 	WndInvalidateAll();
	void 			WndRestore();
	DWORD 			WndStyleFromData();

	// Windows message handlers
	virtual LRESULT WndOnEraseBkgnd(HDC hdc);
	virtual void WndOnPaint(const PAINTSTRUCT& ps);
	virtual void WndOnSize(WORD fwType, int nWidth, int nHeight);
	virtual void WndOnMove(int xPos, int yPos);
	virtual void WndOnGetMinMax(MINMAXINFO __far* lpMMI);
	virtual void WndOnShow(BOOL fShow);
	virtual void WndOnInput(short sType, Am_Button_Down down, Am_Click_Count click,
		bool fCtrl, bool fShift, bool fAlt, int xPos, int yPos);
	virtual void WndOnSetCursor(WPARAM wParam, LPARAM lParam);

	BOOL UsesBufferring() const
	{ return m_usebuff; 
	}
	void SetUsesBufferring(BOOL usebuff);
	BOOL IsBuffered() const
	{ return m_isbuff; 
	}
	void SetBuffered(BOOL isbuff);
	HBITMAP BufferBitmap() const
	{ return m_hbmp; 
	}
	BOOL HasBuffer() const
	{ return BufferBitmap() != 0; 
	}
	void CreateBuffer();
	void ResizeBuffer(unsigned int new_width, unsigned int new_height);
	void FlushBuffer() const; // copy buffer on screen if window is visible
	void FillBuffer() const; // copy screen to buffer if window is visible

	// Parent / children management
	void SetParent(Am_WinDrawonable* parent);


	protected:				  
		void poly_draw_it(Am_Style ls, Am_Style fs, Am_Draw_Function f,	POINT *pld, int num_points);

	// DATAMEMBERS
		static Am_WinDrawonable		*m_root;
		static HINSTANCE 			m_inst;
		#ifndef _WIN32
			static Am_Map_Int2Ptr 	m_mapWnd2Drw;
		#else
			static Am_Map_HModule2Ptr 	m_mapWnd2Drw;
			static HMODULE 			m_hThunk32s;
		#endif

		//:WindowsOS windows handle
		HWND 	m_hwnd;
		HWND 	m_hwndParent;
		DWORD 	m_style;
		DWORD 	m_exstyle;

		BOOL 	m_usebuff;
		BOOL 	m_isbuff;
		HBITMAP m_hbmp;

		Am_WinDrawonable			*m_parent;
		Am_Input_Event_Handlers		*m_evh;
		Am_WinRegion				*m_clip;
		Am_List_Ptr 				m_children;
		static Am_List_Ptr 			m_msgqueue;

		bool m_want_enter_leave_events;
		bool m_want_move_events;
		bool m_want_multi_window;

	private:
		Am_DECL_WINNARROW(Drawonable)
		friend std::ostream& operator<< (std::ostream& os, Am_Drawonable *d);
};

#endif
