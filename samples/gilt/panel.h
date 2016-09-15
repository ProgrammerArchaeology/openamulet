#ifndef __PANEL_H
#define __PANEL_H
#include <amulet.h>

#ifndef Am_bool
#define Am_bool bool
#endif

class paneldialog
{
public:
  paneldialog(){};
  void Init();
  void SetValues(Am_String &name, Am_Value_List &il, Am_Value &layout_key,
                 Am_Value &box_on_left, Am_Value &fw, Am_Value &hspace,
                 Am_Value &vspace, Am_Value &maxrnk);

  void GetValues(Am_String &name, Am_Value_List &il, Am_Value &layout_key,
                 Am_Value &box_on_left, Am_Value &fixed_width, Am_Value &hspace,
                 Am_Value &vspace, Am_Value &maxrnk);

  Am_bool Execute(Am_String &name, Am_Value_List &il, Am_Value &layout_key,
                  Am_Value &box_on_left, Am_Value &fw, Am_Value &hspace,
                  Am_Value &vspace, Am_Value &maxrnk, bool can_change_items);
  void Cancel();
};
#endif
