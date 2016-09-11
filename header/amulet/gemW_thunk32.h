#if defined(__cplusplus)
extern "C" {
#endif

#define Am_THUNK_DLL	"GWTHNK32"

typedef BOOL (WINAPI *Am_ThunkType_TextOut) (
	HDC  hdc,	// handle of device context 
	int  nXStart,	// x-coordinate of starting position  
	int  nYStart,	// y-coordinate of starting position  
	LPCTSTR  lpString,	// address of string 
	int  cbString 	// number of characters in string 
   );

typedef BOOL (WINAPI *Am_ThunkType_GetTextExtentPoint32) (
    HDC  hdc,	// handle of device context 
    LPCTSTR  lpString,	// address of text string 
    int  cbString,	// number of characters in string 
    LPSIZE  lpSize 	// address of structure for string size  
   );	

typedef BOOL (WINAPI *Am_ThunkType_UnivArc) (
	int  fFunc, // which subfunction
    HDC  hdc,	// handle of device context 
    int  nLeftRect,	// x-coordinate of upper-left corner of bounding rectangle 
    int  nTopRect,	// y-coordinate of upper-left corner of bounding rectangle 
    int  nRightRect,	// x-coordinate of lower-right corner of bounding rectangle  
    int  nBottomRect,	// y-coordinate of lower-right corner of bounding rectangle  
    int  nXStartArc,	// first radial ending point 
    int  nYStartArc,	// first radial ending point 
    int  nXEndArc,	// second radial ending point 
    int  nYEndArc 	// second radial ending point 
   );	

typedef BOOL (WINAPI *Am_ThunkType_Ellipse) (
    HDC  hdc,	// handle of device context 
    int  nLeftRect,	// x-coordinate of upper-left corner of bounding rectangle 
    int  nTopRect,	// y-coordinate of upper-left corner of bounding rectangle 
    int  nRightRect,	// x-coordinate of lower-right corner of bounding rectangle  
    int  nBottomRect	// y-coordinate of lower-right corner of bounding rectangle  
   );	

#define Am_Thunk_TextOut	((Am_ThunkType_TextOut)GetProcAddress(m_hThunk32s, MAKEINTRESOURCE(1)))
#define Am_Thunk_GetTextExtentPoint32	\
	((Am_ThunkType_GetTextExtentPoint32)GetProcAddress(m_hThunk32s, MAKEINTRESOURCE(2)))
#define Am_Thunk_Arc(A1, A2, A3, A4, A5, A6, A7, A8, A9)	\
	(((Am_ThunkType_UnivArc)GetProcAddress(m_hThunk32s, MAKEINTRESOURCE(3))) \
		(1, A1, A2, A3, A4, A5, A6, A7, A8, A9))
#define Am_Thunk_Chord(A1, A2, A3, A4, A5, A6, A7, A8, A9)	\
	(((Am_ThunkType_UnivArc)GetProcAddress(m_hThunk32s, MAKEINTRESOURCE(3))) \
		(2, A1, A2, A3, A4, A5, A6, A7, A8, A9))
#define Am_Thunk_Pie(A1, A2, A3, A4, A5, A6, A7, A8, A9)	\
	(((Am_ThunkType_UnivArc)GetProcAddress(m_hThunk32s, MAKEINTRESOURCE(3))) \
		(3, A1, A2, A3, A4, A5, A6, A7, A8, A9))
#define Am_Thunk_Ellipse	((Am_ThunkType_Ellipse)GetProcAddress(m_hThunk32s, MAKEINTRESOURCE(4)))

#if defined(__cplusplus)
}
#endif

