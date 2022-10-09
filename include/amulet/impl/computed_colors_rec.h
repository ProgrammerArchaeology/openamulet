#include "am_style.h"

class Computed_Colors_Record_Data : public Am_Wrapper
{
  AM_WRAPPER_DATA_DECL(Computed_Colors_Record)

public:
  // CREATORS
  // Default C'tor
  Computed_Colors_Record_Data() { Note_Reference(); }

  Computed_Colors_Record_Data(const Am_Style &foreground);
  Computed_Colors_Record_Data(Computed_Colors_Record_Data *);

  ~Computed_Colors_Record_Data() override;

  bool operator==(const Computed_Colors_Record_Data &) const { return (false); }

  void Print(std::ostream &out) const override;

  // DATA MEMBERS
  Am_Style key;
  Am_Style foreground_style;
  Am_Style background_style;
  Am_Style shadow_style;
  Am_Style highlight_style;
  Am_Style highlight2_style;

  // true if foreground color is light, so should use black text
  bool light;
};

_OA_DL_IMPORT std::ostream &operator<<(std::ostream &os,
                                       Computed_Colors_Record_Data &rec);

class _OA_DL_CLASSIMPORT Computed_Colors_Record
{
public:
  // CREATORS
  Computed_Colors_Record() { data = nullptr; }

  ~Computed_Colors_Record()
  {
    if (data) {
      data->Release();
    }

    data = nullptr;
  }

  Computed_Colors_Record(const Am_Style &foreground);

  Computed_Colors_Record(const Am_Value &in_value)
  {
    data = (Computed_Colors_Record_Data *)in_value.value.wrapper_value;

    if (data) {
      data->Note_Reference();
    }
  }

  Computed_Colors_Record(Computed_Colors_Record_Data *in_data)
  {
    data = in_data;
  }

  operator Am_Wrapper *() const
  {
    if (data) {
      data->Note_Reference();
    }

    return (data);
  }

  // DATA MEMBERS
  Computed_Colors_Record_Data *data;
};
