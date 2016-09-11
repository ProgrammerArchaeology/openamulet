#ifndef GDEFS_CURSOR_H
#define GDEFS_CURSOR_H

class Am_Cursor_Data;

class _OA_DL_CLASSIMPORT Am_Cursor 
{
  Am_WRAPPER_DECL(Am_Cursor)

public:
  // creators
  Am_Cursor();
  Am_Cursor(Am_Image_Array image, Am_Image_Array mask,
	    Am_Style fg_color, Am_Style bg_color);
  
  // sets the cursor in the proper window
  // do we want the function for this to be in cursor or in drawonable?
  void Set_Hot_Spot (int x, int y);
  _OA_DL_MEMBERIMPORT void Get_Hot_Spot (int& x, int& y) const;

  // Size will be zero until the image is drawn.  Get the size of an
  // image through Am_Drawonable::Get_Image_Size.
  void Get_Size (int& width, int& height);
};

_OA_DL_IMPORT extern Am_Cursor Am_Default_Cursor;

#endif //GDEFS_CURSOR_H
