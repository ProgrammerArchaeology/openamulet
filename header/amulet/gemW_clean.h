//
// This file was designed for reference-counting GDI objects
// created by Amulet.  It was needed on the (short-lived) 
// Windows 3.1 port;  it is NOT needed on Win32.
//  q.v. "Win32 Subsystem Object Cleanup", Microsoft Knowledge 
//   Base, 5 Jan 95:
//
//   The Win32 subsystem guarantees that all Win32 objects owned by a
//   process will be freed when an application terminates. To accomplish
//   this, the Win32 subsystem keeps track of who owns these objects; it
//   also keeps a reference count. Reference counts are used when the
//   object is owned by more then one process. For example, a memory mapped
//   file can be used to provide interprocess communication, where more
//   than one process would own that object. The subsystem must make sure
//   that the reference count is zero before the object can be freed.
//    
//   Freeing of Win32 objects can occur at different times. In general, it
//   occurs at process termination, but for some objects, it occurs at thread
//   termination.
//    
//   NOTE: When running Win32-based applications with Windows 3.1 using the
//   Win32s environment, it is the responsibility of the Win32-based application
//   to ensure that all allocated GDI objects are deleted before the program
//   terminates. This is different from the behavior of the application with
//   Windows NT. With Windows NT, the GDI subsystem cleans up all orhphaned GDI
//   objects. Because there is no GDI subsystem with Windows 3.1, this behavior
//   is not supported.
 
#ifndef USE_WINCLEANER
// don't do any GDI reference counting

// stub macros
#define	Am_WINCLEAN_CONSTR(C)
#define	Am_WINCLEAN_DESTR

#define AM_DECL_WINCLEAN(C)
#define AM_IMPL_WINCLEAN(C)
#define AM_IMPL_WINCLEAN_FORWRAPPERS(C)

#else
// do GDI reference counting

#if !defined(GWCLEAN_H)
#define GWCLEAN_H

//#pragma pack()

enum Am_WinCleanAction { Am_CLEAN_HASRES, Am_CLEAN_CHECKUSED,
	Am_CLEAN_FREERES, Am_CLEAN_DELETE };

#if !defined(_WIN32)
#define Am_CLEAN_STARTLEVEL	20
#define Am_CLEAN_STOPLEVEL	25
#else
#define Am_CLEAN_STARTCOUNT	64
#endif

typedef BOOL Am_WinCleanFunc (void*, Am_WinCleanAction);

class Am_WinCleaner {
public:
	Am_WinCleaner ();
	virtual ~Am_WinCleaner ();
public:
	void In (void* item, Am_WinCleanFunc func) {
		m_map.SetAt(item, func);
	}
	void Out (void* item) {
		m_map.DeleteKey(item);
	}
	BOOL CheckResources (); //returns TRUE if further processing needed
#if !defined(_WIN32)
protected:
	UINT m_start;
	UINT m_stop;
#endif
private:
	Am_Map_Ptr2Ptr m_map;
};

extern Am_WinCleaner g_cleaner; // the one & only global cleaner

#define Am_WINCLEAN_FUNC(C)	Clean_##C

#define AM_DECL_WINCLEAN(C)											 \
	friend Am_WINCLEAN_FUNC(C) (void* item, Am_WinCleanAction act);	 \
private:															 \
	BOOL m_fUsedRes;												 \
	void WinSetUsedRes ()											 \
		{ m_fUsedRes = TRUE; };										 \
	BOOL WinCheckUsedRes ()											 \
		{ BOOL f = m_fUsedRes; m_fUsedRes = FALSE; return f; };		 \
protected:															 \
	BOOL WinHasRes () const;										 \
	BOOL WinFreeRes ();
																	 
#define AM_IMPL_WINCLEAN(C)											 \
BOOL Am_WINCLEAN_FUNC(C) (void* item, Am_WinCleanAction act)         \
{                                                                    \
	switch (act) {                                                   \
		case Am_CLEAN_HASRES:	                                     \
			return ((C*)item) -> WinHasRes();						 \
		case Am_CLEAN_CHECKUSED:                                     \
			return ((C*)item) -> WinCheckUsedRes();					 \
		case Am_CLEAN_DELETE:                                        \
			Am_TRACE("##Clean: deleting " #C " @" << item <<std::endl)   \
			delete (C*)item;                                         \
			return TRUE;                                             \
		case Am_CLEAN_FREERES:                                       \
			if (((C*)item) -> WinFreeRes()) {                        \
				Am_TRACE("##Clean: free res " #C " @" << item <<std::endl)\
				return TRUE;                                         \
			}                                                        \
			break;                                                   \
	}                                                                \
	return FALSE;                                                    \
}

#define AM_IMPL_WINCLEAN_FORWRAPPER(C)								 \
BOOL Am_WINCLEAN_FUNC(C) (void* item, Am_WinCleanAction act)         \
{                                                                    \
	switch (act) {                                                   \
		case Am_CLEAN_HASRES:	                                     \
			return ((C*)item) -> WinHasRes();						 \
		case Am_CLEAN_CHECKUSED:                                     \
			return ((C*)item) -> WinCheckUsedRes();					 \
		case Am_CLEAN_DELETE:                                        \
			/*Am_TRACE("##Clean: releasing " #C " @" << item <<std::endl)*/  \
			Am_TRACE("##Clean: not releasing " #C " @" << item <<std::endl)  \
			((C*)item) -> WinFreeRes();                              \
			/*((C*)item) -> Release();*/                                 \
			/*delete (C*)item;*/                                     \
			return TRUE;                                             \
		case Am_CLEAN_FREERES:                                       \
			if (((C*)item) -> WinFreeRes()) {                        \
				Am_TRACE("##Clean: free res " #C " @" << item <<std::endl)\
				return TRUE;                                         \
			}                                                        \
			break;                                                   \
	}                                                                \
	return FALSE;                                                    \
}

#define	Am_WINCLEAN_CONSTR(C) \
	{ WinCheckUsedRes(); g_cleaner->In(this, Am_WINCLEAN_FUNC(C)); }
#define	Am_WINCLEAN_DESTR		{ g_cleaner->Out(this); }

#endif // defined(GWCLEAN_H)

#endif // USE_WINCLEANER
