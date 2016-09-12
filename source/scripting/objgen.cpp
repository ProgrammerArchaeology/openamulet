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
#include SCRIPTING_ADVANCED__H

Am_Object am_objgen;

Am_Slot_Key Am_CONSTANT_OBJECTS_WIDGET =
    Am_Register_Slot_Name("CONSTANT_OBJECTS_WIDGET");
Am_Slot_Key Am_INDEXES_OF_COMMANDS_WIDGET =
    Am_Register_Slot_Name("INDEXES_OF_COMMANDS_WIDGET");
Am_Slot_Key Am_SOME_OBJECTS_WIDGET =
    Am_Register_Slot_Name("SOME_OBJECTS_WIDGET");
Am_Slot_Key Am_ALL_FROM_COMMAND_WIDGET =
    Am_Register_Slot_Name("ALL_FROM_COMMAND_WIDGET");
Am_Slot_Key Am_GREEN_AND_YELLOW = Am_Register_Slot_Name("Am_GREEN_AND_YELLOW");
Am_Slot_Key Am_COMMAND_LOAD_BUTTON =
    Am_Register_Slot_Name("Am_COMMAND_LOAD_BUTTON");

Am_Object
am_objgen_Initialize()
{
  am_objgen = Am_Window.Create(DSTR("am_generalizeobj"))
                  .Set(Am_DESTROY_WINDOW_METHOD,
                       Am_Default_Pop_Up_Window_Destroy_Method)
                  .Set(Am_FILL_STYLE, Am_Default_Color)
                  .Set(Am_TITLE, "Generalize Objects")
                  .Set(Am_ICON_TITLE, "Generalize Objects")
                  .Set(Am_WIDTH, 535)
                  .Set(Am_HEIGHT, 345)

                  .Add(Am_PLACEHOLDERS_SET_BY_THIS_COMMAND, (0L))
                  .Add(Am_SCRIPT_WINDOW, (0L))
                  .Add(Am_COMMAND, (0L))
                  .Add(Am_SLOTS_TO_SAVE, (0L))
                  .Add(Am_VALUE, (0L));
  am_objgen
      .Add_Part(Am_Border_Rectangle.Create()
                    .Set(Am_LEFT, 14)
                    .Set(Am_TOP, 112)
                    .Set(Am_WIDTH, 505)
                    .Set(Am_HEIGHT, 175)
                    .Set(Am_SELECTED, 0)
                    .Set(Am_FILL_STYLE, Am_Default_Color))
      .Add_Part(Am_Border_Rectangle.Create()
                    .Set(Am_LEFT, 14)
                    .Set(Am_TOP, 39)
                    .Set(Am_WIDTH, 505)
                    .Set(Am_HEIGHT, 65)
                    .Set(Am_SELECTED, 0)
                    .Set(Am_FILL_STYLE, Am_Default_Color))
      .Add_Part(Am_Text.Create()
                    .Set(Am_LEFT, 9)
                    .Set(Am_TOP, 9)
                    .Set(Am_WIDTH, 198)
                    .Set(Am_HEIGHT, 22)
                    .Set(Am_TEXT, "Generalize Objects")
                    .Set(Am_FONT, Am_Font(Am_FONT_FIXED, true, false, false,
                                          Am_FONT_LARGE))
                    .Set(Am_LINE_STYLE, Am_Black)
                    .Set(Am_FILL_STYLE, Am_No_Style));
  am_objgen.Add_Part(
      Am_UNDO_OPTIONS,
      Am_Radio_Button_Panel.Create()
          .Set(Am_LEFT, 27)
          .Set(Am_TOP, 127)
          .Set(Am_FILL_STYLE, Am_Default_Color)
          .Set(Am_ITEMS,
               Am_Value_List()
                   .Add(Am_Command.Create()
                            .Set(Am_LABEL, "Constant: ")
                            .Set(Am_ID, am_constant_generalize))
                   .Add(Am_Command.Create()
                            .Set(Am_LABEL,
                                 "What is selected before the operation ")
                            .Set(Am_ID, am_selected_generalize))
                   .Add(Am_Command.Create()
                            .Set(Am_LABEL, "Ask user when script runs")
                            .Set(Am_ID, am_ask_user_generalize))
                   .Add(Am_Command.Create()
                            .Set(Am_LABEL, "All objects returned from command")
                            .Set(Am_ID, am_all_values_generalize))
                   .Add(Am_Command.Create()
                            .Set(Am_LABEL, "The object numbered")
                            .Set(Am_ID, am_the_objects_generalize))
                   .Add(Am_Command.Create()
                            .Set(Am_LABEL, "Custom...")
                            .Set(Am_ACTIVE, false)
                            .Set(Am_ID, am_custom_generalize)))
          .Set(Am_LAYOUT, Am_Vertical_Layout)
          .Set(Am_H_SPACING, 0)
          .Set(Am_V_SPACING, 0));
  am_objgen.Add_Part(
      Am_CONSTANT_OBJECTS_WIDGET,
      Am_Text_Input_Widget.Create(DSTR("Am_CONSTANT_OBJECTS_WIDGET"))
          .Set(Am_LEFT, 125)
          .Set(Am_ACTIVE, am_active_if_constant_sel)
          .Set(Am_TOP, 125)
          .Set(Am_WIDTH, 380)
          .Set(Am_HEIGHT, 25)
          .Set(Am_ACTIVE_2, false) // can't type here ** NIY **
          .Get_Object(Am_COMMAND)
          .Set(Am_LABEL, "")
          .Get_Owner()
          .Set(Am_FILL_STYLE, Am_Default_Color));
  am_objgen.Add_Part(
      Am_INDEXES_OF_COMMANDS_WIDGET,
      Am_Text_Input_Widget.Create(DSTR("INDEXES_OF_COMMANDS_WIDGET"))
          .Set(Am_LEFT, 199)
          .Set(Am_TOP, 228)
          .Set(Am_WIDTH, 105)
          .Set(Am_ACTIVE_2, false) // **TEMPORARY
          .Set(Am_ACTIVE, am_active_the_object_sel)
          .Set(Am_HEIGHT, 25)
          .Get_Object(Am_COMMAND)
          .Set(Am_LABEL, "")
          .Get_Owner()
          .Set(Am_FILL_STYLE, Am_Default_Color));
  am_objgen.Add_Part(Am_SOME_OBJECTS_WIDGET,
                     Am_Text_Input_Widget.Create(DSTR("SOME_OBJECTS_WIDGET"))
                         .Set(Am_LEFT, 305)
                         .Set(Am_TOP, 228)
                         .Set(Am_WIDTH, 196)
                         .Set(Am_HEIGHT, 25)
                         .Set(Am_ACTIVE_2, false) // **TEMPORARY
                         .Set(Am_ACTIVE, am_active_the_object_sel)
                         .Get_Object(Am_COMMAND)
                         .Set(Am_LABEL, "from command")
                         // .Set(Am_DO_METHOD, am_check_command_line_number)
                         .Get_Owner()
                         .Set(Am_FILL_STYLE, Am_Default_Color));
  am_objgen.Add_Part(
      Am_ALL_FROM_COMMAND_WIDGET,
      Am_Text_Input_Widget.Create(DSTR("ALL_FROM_COMMAND_WIDGET"))
          .Set(Am_LEFT, 287)
          .Set(Am_TOP, 198)
          .Set(Am_ACTIVE, am_active_all_values_sel)
          .Set(Am_ACTIVE_2, false) // **TEMPORARY
          .Set(Am_WIDTH, 165)
          .Set(Am_HEIGHT, 25)
          .Get_Object(Am_COMMAND)
          .Set(Am_LABEL, "")
          // .Set(Am_DO_METHOD, am_check_command_line_number)
          .Get_Owner()
          .Set(Am_FILL_STYLE, Am_Default_Color));
  am_objgen.Add_Part(
      Am_Button_Panel.Create()
          .Set(Am_LEFT, Am_Center_X_Is_Center_Of_Owner)
          .Set(Am_TOP, 305)
          .Set(Am_FILL_STYLE, Am_Default_Color)
          .Set(Am_LAYOUT, Am_Horizontal_Layout)
          .Set(Am_H_SPACING, 20)
          .Set(Am_V_SPACING, 0)
          .Set(Am_MAX_RANK, 0)
          .Set(Am_ITEMS, Am_Value_List()
                             .Add(Am_Standard_OK_Command.Create().Set(
                                 Am_DO_METHOD, am_objgen_ok))
                             .Add(Am_Standard_Cancel_Command.Create().Set(
                                 Am_DO_METHOD, am_objgen_cancel))));
  am_objgen.Add_Part(
      Am_GREEN_AND_YELLOW,
      Am_Radio_Button_Panel.Create(DSTR("GREEN_AND_YELLOW"))
          .Set(Am_LEFT, 27)
          .Set(Am_TOP, 45)
          .Set(Am_FILL_STYLE, Am_Default_Color)
          .Set(Am_ITEMS,
               Am_Value_List()
                   .Add(Am_Command.Create()
                            .Set(Am_LABEL, "Generalize green selected object")
                            .Set(Am_ID, am_generalize_only_green))
                   .Add(Am_Command.Create()
                            .Set(Am_LABEL,
                                 "Generalize green and yellow selected objects")
                            .Set(Am_ID, am_generalize_green_and_yellow)))
          .Set(Am_LAYOUT, Am_Vertical_Layout)
          .Set(Am_H_SPACING, 0)
          .Set(Am_V_SPACING, 0)
          .Set(Am_MAX_RANK, 0));
  am_objgen
      .Add_Part(Am_COMMAND_LOAD_BUTTON,
                Am_Button.Create(DSTR("Am_LOAD_BUTTON"))
                    .Set(Am_LEFT, 457)
                    .Set(Am_TOP, 200)
                    .Set(Am_LEAVE_ROOM_FOR_FRINGE, false)
                    .Set(Am_ACTIVE, am_active_all_or_the_object_sel)
                    .Get_Object(Am_COMMAND)
                    .Set(Am_LABEL, "Load")
                    .Set(Am_DO_METHOD, am_load_current_command)
                    .Get_Owner())
      .Add_Part(Am_Tab_To_Next_Widget_Interactor.Create());
  return am_objgen;
}
