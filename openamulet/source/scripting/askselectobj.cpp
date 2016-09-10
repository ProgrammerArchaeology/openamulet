//
// ------------------------------------------------------------------------
// 		The OpenAmulet User Interface Development Environment
// ------------------------------------------------------------------------
// This code is based on the Amulet project of Carnegie Mellon University,
// and has been placed in the public domain.  If you are using this code
// or any part of OpenAmulet, please contact amulet@cs.cmu.edu to be put
// on the mailing list or have a look at:
// http://www.openip.org
//

#include <amulet.h>
#include OBJECT__H			// am_object
#include OPAL__H			// am_window
#include STANDARD_SLOTS__H	// am_destroy_window_method
#include WIDGETS__H			// am_default_pop_up_window_destroy_method

Am_Object am_askselectobj;


Am_Object am_askselectobj_Initialize () {
  am_askselectobj = Am_Window.Create(DSTR("askselectobj"))
    .Set(Am_DESTROY_WINDOW_METHOD, Am_Default_Pop_Up_Window_Destroy_Method)
    .Set(Am_FILL_STYLE, Am_Default_Color)
    .Set(Am_TITLE, "Select Objects")
    .Set(Am_ICON_TITLE, "Select Objects")
    .Set(Am_WIDTH , 380)
    .Set(Am_HEIGHT, 100)
    ;
  am_askselectobj
    .Add_Part(Am_Text.Create()
      .Set(Am_LEFT, Am_Center_X_Is_Center_Of_Owner)
      .Set(Am_TOP, 16)
      .Set(Am_WIDTH, 350)
      .Set(Am_HEIGHT, 15)
      .Set(Am_TEXT, "Please select one or more objects and then hit OK,")
      .Set(Am_LINE_STYLE, Am_Black)
      .Set(Am_FILL_STYLE, Am_No_Style)
    )
    .Add_Part(Am_Text.Create()
      .Set(Am_LEFT, Am_Center_X_Is_Center_Of_Owner)
      .Set(Am_TOP, 32)
      .Set(Am_WIDTH, 301)
      .Set(Am_HEIGHT, 15)
      .Set(Am_TEXT, "or hit Cancel to stop executing the script.")
      .Set(Am_LINE_STYLE, Am_Black)
      .Set(Am_FILL_STYLE, Am_No_Style)
    )
    .Add_Part(Am_Button_Panel.Create()
      .Set(Am_LEFT, Am_Center_X_Is_Center_Of_Owner)
      .Set(Am_TOP, 55)
      .Set(Am_FILL_STYLE, Am_Default_Color)
      .Set(Am_LAYOUT, Am_Horizontal_Layout)
      .Set(Am_H_SPACING, 10)
      .Set(Am_V_SPACING, 0)
      .Set(Am_MAX_RANK, 0)
      .Set(Am_ITEMS, Am_Value_List()
        .Add(Am_Standard_OK_Command.Create())
        .Add(Am_Standard_Cancel_Command.Create())
        )
    )
  ;
  return am_askselectobj;
}
