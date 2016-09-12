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
#include <amulet/object.h>         // am_object
#include <amulet/opal.h>           // am_window
#include <amulet/standard_slots.h> // am_destroy_window_method
#include <amulet/widgets.h>        // am_default_pop_up_window_destroy_method

Am_Object am_genval;

Am_Slot_Key CONSTANT_OBJECTS_WIDGET =
    Am_Register_Slot_Name("CONSTANT_OBJECTS_WIDGET");
Am_Slot_Key ALL_FROM_COMMAND_WIDGET =
    Am_Register_Slot_Name("ALL_FROM_COMMAND_WIDGET");
Am_Slot_Key GREEN_AND_YELLOW = Am_Register_Slot_Name("GREEN_AND_YELLOW");

Am_Object
am_genval_Initialize()
{
  am_genval = Am_Window.Create(DSTR("am_genval"))
                  .Set(Am_DESTROY_WINDOW_METHOD,
                       Am_Default_Pop_Up_Window_Destroy_Method)
                  .Set(Am_FILL_STYLE, Am_Default_Color)
                  .Set(Am_TITLE, "Generalize Value")
                  .Set(Am_ICON_TITLE, "Generalize Value")
                  .Set(Am_WIDTH, 695)
                  .Set(Am_HEIGHT, 670);
  am_genval
      .Add_Part(Am_Border_Rectangle.Create()
                    .Set(Am_LEFT, 14)
                    .Set(Am_TOP, 112)
                    .Set(Am_WIDTH, 455)
                    .Set(Am_HEIGHT, 149)
                    .Set(Am_SELECTED, 0)
                    .Set(Am_FILL_STYLE, Am_Default_Color))
      .Add_Part(Am_Border_Rectangle.Create()
                    .Set(Am_LEFT, 14)
                    .Set(Am_TOP, 39)
                    .Set(Am_WIDTH, 455)
                    .Set(Am_HEIGHT, 65)
                    .Set(Am_SELECTED, 0)
                    .Set(Am_FILL_STYLE, Am_Default_Color))
      .Add_Part(Am_Text.Create()
                    .Set(Am_LEFT, 9)
                    .Set(Am_TOP, 9)
                    .Set(Am_WIDTH, 198)
                    .Set(Am_HEIGHT, 22)
                    .Set(Am_TEXT, "Generalize Value")
                    .Set(Am_FONT, Am_Font(Am_FONT_FIXED, true, false, false,
                                          Am_FONT_LARGE))
                    .Set(Am_LINE_STYLE, Am_Black)
                    .Set(Am_FILL_STYLE, Am_No_Style))
      .Add_Part(
          Am_Radio_Button_Panel.Create()
              .Set(Am_LEFT, 27)
              .Set(Am_TOP, 123)
              .Set(Am_FILL_STYLE, Am_Default_Color)
              .Set(Am_ITEMS,
                   Am_Value_List()
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, "Constant: ")
                                .Set(Am_ID, 1))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, "The value of the palette when "
                                               "the script runs")
                                .Set(Am_ID, 2))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, "Ask user when script runs")
                                .Set(Am_ID, 3))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL,
                                     "The value returned from command:")
                                .Set(Am_ID, 4))
                       .Add(Am_Command.Create()
                                .Set(Am_LABEL, "Custom...")
                                .Set(Am_ID, 5)))
              .Set(Am_LAYOUT, Am_Vertical_Layout)
              .Set(Am_H_SPACING, 0)
              .Set(Am_V_SPACING, 0)
              .Set(Am_MAX_RANK, 0))
      .Add_Part(CONSTANT_OBJECTS_WIDGET,
                Am_Text_Input_Widget.Create(DSTR("CONSTANT_OBJECTS_WIDGET"))
                    .Set(Am_LEFT, 121)
                    .Set(Am_TOP, 123)
                    .Set(Am_WIDTH, 193)
                    .Set(Am_HEIGHT, 25)
                    .Get_Object(Am_COMMAND)
                    .Set(Am_LABEL, "")
                    .Get_Owner()
                    .Set(Am_FILL_STYLE, Am_Default_Color))
      .Add_Part(ALL_FROM_COMMAND_WIDGET,
                Am_Text_Input_Widget.Create(DSTR("ALL_FROM_COMMAND_WIDGET"))
                    .Set(Am_LEFT, 276)
                    .Set(Am_TOP, 197)
                    .Set(Am_WIDTH, 146)
                    .Set(Am_HEIGHT, 25)
                    .Get_Object(Am_COMMAND)
                    .Set(Am_LABEL, "")
                    .Get_Owner()
                    .Set(Am_FILL_STYLE, Am_Default_Color))
      .Add_Part(
          Am_Button_Panel.Create()
              .Set(Am_LEFT, 167)
              .Set(Am_TOP, 277)
              .Set(Am_FILL_STYLE, Am_Default_Color)
              .Set(Am_LAYOUT, Am_Horizontal_Layout)
              .Set(Am_H_SPACING, 20)
              .Set(Am_V_SPACING, 0)
              .Set(Am_MAX_RANK, 0)
              .Set(Am_ITEMS, Am_Value_List()
                                 .Add(Am_Standard_OK_Command.Create())
                                 .Add(Am_Standard_Cancel_Command.Create())))
      .Add_Part(GREEN_AND_YELLOW,
                Am_Radio_Button_Panel.Create(DSTR("GREEN_AND_YELLOW"))
                    .Set(Am_LEFT, 27)
                    .Set(Am_TOP, 45)
                    .Set(Am_FILL_STYLE, Am_Default_Color)
                    .Set(Am_ITEMS,
                         Am_Value_List()
                             .Add(Am_Command.Create()
                                      .Set(Am_LABEL,
                                           "Generalize green selected object")
                                      .Set(Am_ID, 1))
                             .Add(Am_Command.Create()
                                      .Set(Am_LABEL, "Generalize green and "
                                                     "yellow selected objects")
                                      .Set(Am_ID, 2)))
                    .Set(Am_LAYOUT, Am_Vertical_Layout)
                    .Set(Am_H_SPACING, 0)
                    .Set(Am_V_SPACING, 0)
                    .Set(Am_MAX_RANK, 0))
      .Add_Part(Am_Button.Create()
                    .Set(Am_LEFT, 418)
                    .Set(Am_TOP, 188)
                    .Set(Am_WIDTH, 53)
                    .Set(Am_HEIGHT, 40)
                    .Set(Am_FILL_STYLE, Am_Default_Color)
                    .Get_Object(Am_COMMAND)
                    .Set(Am_LABEL, "Load")
                    .Get_Owner())
      .Add_Part(Am_Tab_To_Next_Widget_Interactor.Create());
  return am_genval;
}
