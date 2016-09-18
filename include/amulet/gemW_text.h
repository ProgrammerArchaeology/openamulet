#if !defined(GWFONT_H)
#define GWFONT_H

//#pragma pack()

class Am_Font_Data : public Am_Wrapper
{
  friend class Am_Font;
  AM_WRAPPER_DATA_DECL(Am_Font)
  AM_DECL_WINCLEAN(Am_Font_Data)
public:
  void Print(std::ostream &os) const;
  Am_Font_Data(Am_Font_Data *proto);
  Am_Font_Data(const char *the_name);
  Am_Font_Data(Am_Font_Family_Flag f = Am_FONT_FIXED, bool is_bold = false,
               bool is_italic = false, bool is_underline = false,
               Am_Font_Size_Flag s = Am_FONT_MEDIUM);

  ~Am_Font_Data();

public:
  bool operator==(const Am_Font_Data &) const { return false; }

  HFONT WinFont(HDC hdc);

  bool IsBold() const { return m_bold; }
  bool IsItalic() const { return m_italic; }
  bool IsUnderline() const { return m_underline; }
  Am_Font_Size_Flag Size() const { return m_size; }
protected:
  Am_Font_Family_Flag m_family;
  bool m_bold;
  bool m_italic;
  bool m_underline;
  Am_Font_Size_Flag m_size;
  char *m_name;

  HFONT m_hfont;
};

#endif
