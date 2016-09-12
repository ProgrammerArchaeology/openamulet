#ifndef GDEFS_FONT_H
#define GDEFS_FONT_H

#include "gem_flags.h"

class Am_Font_Data;

class _OA_DL_CLASSIMPORT Am_Font {
  AM_WRAPPER_DECL (Am_Font)
public:
    //creators
  Am_Font (const char* the_name);
  Am_Font (Am_Font_Family_Flag f = Am_FONT_FIXED,
	   bool is_bold = false,
	   bool is_italic = false,
	   bool is_underline = false,
	   Am_Font_Size_Flag s = Am_FONT_MEDIUM);

  void Get(Am_String &name,
	   Am_Font_Family_Flag &f,
	   bool &is_bold,
	   bool &is_italic,
	   bool &is_underline,
	   Am_Font_Size_Flag &s);
  _OA_DL_MEMBERIMPORT  bool operator== (const Am_Font& font) const;
  _OA_DL_MEMBERIMPORT  bool operator!= (const Am_Font& font) const;

  static bool Font_Name_Valid (const char* name);
};

#endif //GDEFS_FONT_H
