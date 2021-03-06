/* ************************************************************* *
 *   The Amulet User Interface Development Environment           *
 * ************************************************************* *
 *   Created automatically by the Gilt program in Amulet.        *
 *   Do not edit this file directly.                             *
 *   For more information on Amulet, contact amulet@cs.cmu.edu   *
 * ************************************************************* *
 *   Generated on Wed Jan 29 11:01:44 1997

 *   Amulet version 3.0 alpha
 * ************************************************************* */

#include <amulet.h>

Am_Object example2db;

Am_Slot_Key GILT_DB_VALUE = Am_Register_Slot_Name ("GILT_DB_VALUE");


Am_Object example2db_Initialize () {
  example2db = Am_Window.Create("example2db")
    .Set(Am_DESTROY_WINDOW_METHOD, Am_Default_Pop_Up_Window_Destroy_Method)
    .Set(Am_FILL_STYLE, Am_Motif_Light_Green)
    .Set(Am_TITLE, "Gilt-Created Dialog Box")
    .Set(Am_ICON_TITLE, "Gilt-Created Dialog Box")
    .Set(Am_WIDTH, 438)
    .Set(Am_HEIGHT, 151)
    .Add_Part(Am_Border_Rectangle.Create()
      .Set(Am_LEFT, 12)
      .Set(Am_TOP, 7)
      .Set(Am_WIDTH, 426)
      .Set(Am_HEIGHT, 59)
      .Set(Am_SELECTED, 0)
      .Set(Am_FILL_STYLE, Am_Motif_Light_Green)
    )
    .Add_Part(Am_Text.Create()
      .Set(Am_LEFT, 14)
      .Set(Am_TOP, 17)
      .Set(Am_WIDTH, 407)
      .Set(Am_HEIGHT, 18)
      .Set(Am_TEXT, "This is a Dialog Box created with the")
      .Set(Am_FONT, Am_Font(Am_FONT_FIXED, false, true, false, Am_FONT_LARGE))
      .Set(Am_LINE_STYLE, Am_Black)
      .Set(Am_FILL_STYLE, Am_No_Style)
    )
    .Add_Part(Am_Text.Create()
      .Set(Am_LEFT, 103)
      .Set(Am_TOP, 40)
      .Set(Am_WIDTH, 242)
      .Set(Am_HEIGHT, 18)
      .Set(Am_TEXT, "Gilt Interface Builder")
      .Set(Am_FONT, Am_Font(Am_FONT_FIXED, false, true, false, Am_FONT_LARGE))
      .Set(Am_LINE_STYLE, Am_Black)
      .Set(Am_FILL_STYLE, Am_No_Style)
    )
    .Add_Part(GILT_DB_VALUE, Am_Text_Input_Widget.Create("GILT_DB_VALUE")
      .Set(Am_LEFT, 30)
      .Set(Am_TOP, 75)
      .Set(Am_WIDTH, 389)
      .Set(Am_HEIGHT, 25)
      .Get_Object(Am_COMMAND)
        .Set(Am_LABEL, "Value To Use:")
        .Get_Owner()
      .Set(Am_FILL_STYLE, Am_Motif_Light_Green)
    )
    .Add_Part(Am_Button_Panel.Create()
      .Set(Am_LEFT, 162)
      .Set(Am_TOP, 113)
      .Set(Am_FILL_STYLE, Am_Motif_Light_Green)
      .Set(Am_LAYOUT, Am_Horizontal_Layout)
      .Set(Am_H_SPACING, 40)
      .Set(Am_V_SPACING, 0)
      .Set(Am_MAX_RANK, 0)
      .Set(Am_ITEMS, Am_Value_List()
        .Add(Am_Standard_OK_Command.Create())
        .Add(Am_Standard_Cancel_Command.Create())
        )
    )
    .Add_Part(Am_Tab_To_Next_Widget_Interactor.Create())
  ;
  return example2db;
}
