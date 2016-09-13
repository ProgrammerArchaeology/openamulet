#include "widget_look.h"
class Am_Formula;

////////////////// Interface between widgets.cc and the various widget files

_OA_DL_IMPORT extern Am_Formula Am_Get_Computed_Colors_Record_Form;

extern void Am_Draw_Motif_Box(int left, int top, int width, int height,
                              bool depressed, const Computed_Colors_Record &rec,
                              Am_Drawonable *draw);

extern void Am_Draw_Windows_Box(int left, int top, int width, int height,
                                bool depressed,
                                const Computed_Colors_Record &rec,
                                Am_Drawonable *draw);

struct am_rect
{
  am_rect(int l = 0, int t = 0, int w = 0, int h = 0)
  {
    left = l;
    top = t, width = w, height = h;
  }
  int left, top, width, height;
};
typedef struct am_rect am_rect;

extern void Inset_Rect(am_rect &r, int inset);
extern void Am_Draw_Rect_Border(am_rect r, Am_Style upper_left,
                                Am_Style lower_right, Am_Drawonable *draw);
extern void draw_down_arrow(int left, int top, int width, int height,
                            Am_Widget_Look look, bool depressed, bool active,
                            bool option,
                            const Computed_Colors_Record &color_rec,
                            Am_Drawonable *draw);
