#ifndef WIDGETS_LOOK_H
#define WIDGETS_LOOK_H

#include "types_enum.h"
class Am_Formula;

//This enum is used internally; users should use the Am_Widget_Look
// values instead.
enum Am_Widget_Look_vals
{
  Am_MOTIF_LOOK_val, Am_WINDOWS_LOOK_val, Am_MACINTOSH_LOOK_val,
  #if defined( _WIN32 )
    Am_NATIVE_LOOK_val = Am_WINDOWS_LOOK_val
  #elif defined( _MACINTOSH )
    Am_NATIVE_LOOK_val = Am_MACINTOSH_LOOK_val
  #else
    Am_NATIVE_LOOK_val = Am_MOTIF_LOOK_val
  #endif
};

// options for the Am_WIDGET_LOOK slot of widgets to determine how it is drawn
Am_Define_Enum_Type(Am_Widget_Look, Am_Widget_Look_vals)
const Am_Widget_Look Am_MOTIF_LOOK (Am_MOTIF_LOOK_val);
const Am_Widget_Look Am_WINDOWS_LOOK (Am_WINDOWS_LOOK_val);
const Am_Widget_Look Am_MACINTOSH_LOOK (Am_MACINTOSH_LOOK_val);
const Am_Widget_Look Am_NATIVE_LOOK (Am_NATIVE_LOOK_val);

_OA_DL_IMPORT void Am_Set_Default_Look( Am_Widget_Look inLook = Am_NATIVE_LOOK );
// get value of Am_WIDGET_LOOK from Am_Screen
_OA_DL_IMPORT extern Am_Formula Am_Default_Widget_Look;

#endif // WIDGETS_LOOK_H
