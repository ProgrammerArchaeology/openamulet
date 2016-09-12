#ifndef WIDGETS_MISC_H
#define WIDGETS_MISC_H

class Am_Style;
class Am_Input_Char;

//buttons
enum Am_Button_Type
{ Am_PUSH_BUTTON, Am_RADIO_BUTTON, Am_CHECK_BUTTON,
	Am_MENU_BUTTON
};

_OA_DL_IMPORT extern Am_Style Am_Motif_Inactive_Stipple; // text draw style when inactive
_OA_DL_IMPORT extern Am_Style Am_Motif_White_Inactive_Stipple; // same, but for white
_OA_DL_IMPORT extern Am_Style Am_Key_Border_Line;

_OA_DL_IMPORT extern Am_Input_Char Am_Default_Widget_Start_Char;


_OA_DL_IMPORT void Am_Pop_Up_Error_Window(const char *error_string);

#define AM_POP_UP_ERROR_WINDOW(error_string)     \
{ char line[250];                                \
  OSTRSTREAM_CONSTR (oss,line, 250, std::ios::out);   \
  oss << error_string << std::ends;                   \
  OSTRSTREAM_COPY(oss,line,250);                 \
  Am_Pop_Up_Error_Window(line);                  \
}

//this returns the About Amulet window, which then might be popped up.
//Or else use the Am_About_Amulet_Command which does this automatically.
_OA_DL_IMPORT extern Am_Object Am_Get_About_Amulet_Window();

// for selection handle widgets
#define HANDLE_SIZE 13      // should be odd
#define HANDLE_SIZE_D2 7   // HANDLE_SIZE / 2

#endif // WIDGETS_MISC_H
