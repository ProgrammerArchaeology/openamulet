#include "am_object.h"
#include "am_drawonable.h"

#define Am_Define_Style_Formula(formula_name) \
     Am_Define_Formula (Am_Wrapper*, formula_name)
#define Am_Define_Value_List_Formula(formula_name) \
     Am_Define_Formula (Am_Wrapper*, formula_name)
#define Am_Define_Font_Formula(formula_name) \
     Am_Define_Formula (Am_Wrapper*, formula_name)
#define Am_Define_Point_List_Formula(formula_name) \
     Am_Define_Formula (Am_Wrapper*, formula_name)
#define Am_Define_Image_Formula(formula_name) \
     Am_Define_Formula (Am_Wrapper*, formula_name)
#define Am_Define_Cursor_Formula(formula_name) \
     Am_Define_Formula (Am_Wrapper*, formula_name)


// For Am_FIXED_WIDTH and Am_FIXED_HEIGHT slots.
#define Am_NOT_FIXED_SIZE 0
#define Am_MAX_FIXED_SIZE 1

// For Am_Text's Am_CURSOR_INDEX
#define Am_NO_CURSOR -1

// Color conversion routines.  RGB to HSV and HSV to RGB
_OA_DL_IMPORT extern void Am_HSV_To_RGB (float h, float s, float v,
			   float& r, float& g, float& b);
_OA_DL_IMPORT extern void Am_RGB_To_HSV (float r, float g, float b,
			   float& h, float& s, float& v);


// set this into the Am_DOUBLE_BUFFERED slot to disable clearing of
// the area before redrawing
#define Am_WIN_DOUBLE_BUFFER_EXTERNAL 2


extern void Am_Initialize_Aux();

// Useful for scroll groups (in widgets)
_OA_DL_IMPORT extern void Am_Invalid_Rectangle_Intersect
	(int left, int top, int width, int height,
	 int my_left, int my_top, int my_width, int my_height,
	 int& final_left, int& final_top, int& final_width, int& final_height);
