//
// ------------------------------------------------------------------------
// 		The OpenAmulet User Interface Development Environment
// ------------------------------------------------------------------------
// This code is based on the Amulet project of Carnegie Mellon University,
// and has been placed in the public domain.  If you are using this code
// or any part of OpenAmulet, please contact amulet@cs.cmu.edu to be put
// on the mailing list or have a look at:
// http://www.scrap.de/html/amulet.htm
//

#include <amulet.h>
#include <amulet/scripting_advanced.h>

Am_Object am_valinvokegen;
Am_Object am_commandinvokegen;

Am_Slot_Key Am_TYPE_OBJECTS_WIDGET =
    Am_Register_Slot_Name("Am_TYPE_OBJECTS_WIDGET");

Am_Define_Formula(bool, am_active_if_type_sel)
{
  return self.Get_Sibling(Am_UNDO_OPTIONS).Get(Am_VALUE) == am_type_generalize;
}

Am_Define_Formula(bool, is_a_object_gen)
{
  return self.Get_Object(Am_SAVED_OLD_OWNER)
      .Get_Sibling(Am_TYPE_OBJECTS_WIDGET)
      .Get(Am_VALUES)
      .Valid();
}

Am_Object
am_valinvokegen_Initialize()
{
  am_valinvokegen = Am_Window.Create(DSTR("am_valinvokegen"))
                        .Set(Am_DESTROY_WINDOW_METHOD,
                             Am_Default_Pop_Up_Window_Destroy_Method)
                        .Set(Am_FILL_STYLE, Am_Default_Color)
                        .Set(Am_TITLE, "Generalize Value for Match")
                        .Set(Am_WIDTH, 472)
                        .Set(Am_HEIGHT, 245)

                        .Add(Am_COMMAND, (0L))
                        .Add(Am_SLOTS_TO_SAVE, (0L))
                        .Add(Am_SCRIPT_WINDOW, (0L))
                        .Add(Am_PLACEHOLDERS_SET_BY_THIS_COMMAND, (0L))
                        .Add(Am_VALUE, (0L)) //the real value
                        .Add(Am_INVOKE_MATCH_COMMAND, (0L));
  am_valinvokegen.Add_Part(Am_Text.Create()
                               .Set(Am_LEFT, 9)
                               .Set(Am_TOP, 9)
                               .Set(Am_TEXT, "Generalize the value for match")
                               .Set(Am_FONT, Am_Font(Am_FONT_FIXED, true, false,
                                                     false, Am_FONT_LARGE))
                               .Set(Am_LINE_STYLE, Am_Black)
                               .Set(Am_FILL_STYLE, Am_No_Style));
  am_valinvokegen.Add_Part(
      Am_UNDO_OPTIONS,
      Am_Radio_Button_Panel.Create(DSTR("radio control"))
          .Set(Am_LEFT, 15)
          .Set(Am_TOP, 40)
          .Set(Am_FILL_STYLE, Am_Default_Color)
          .Set(Am_ITEMS,
               Am_Value_List()
                   .Add(Am_Command.Create()
                            .Set(Am_LABEL, "The specific value: ")
                            .Set(Am_ID, am_constant_generalize))
                   .Add(Am_Command.Create()
                            .Set(Am_LABEL, "Any object of type:")
                            .Set(Am_ACTIVE, is_a_object_gen)
                            .Set(Am_ID, am_type_generalize))
                   .Add(Am_Command.Create()
                            .Set(Am_LABEL, "Any value")
                            .Set(Am_ID, am_any_value_generalize))
                   .Add(Am_Command.Create()
                            .Set(Am_LABEL, "The value returned from command:")
                            .Set(Am_ACTIVE, false) // ** TEMP
                            .Set(Am_ID, am_all_values_generalize))
                   .Add(Am_Command.Create()
                            .Set(Am_LABEL, "Custom...")
                            .Set(Am_ACTIVE, false)
                            .Set(Am_ID, am_custom_generalize)))
          .Set(Am_LAYOUT, Am_Vertical_Layout)
          .Set(Am_H_SPACING, 0)
          .Set(Am_V_SPACING, 10));
  am_valinvokegen.Add_Part(
      Am_CONSTANT_OBJECTS_WIDGET,
      Am_Text_Input_Widget.Create(DSTR("CONSTANT_OBJECTS_WIDGET"))
          .Set(Am_ACTIVE, am_active_if_constant_sel)
          .Set(Am_LEFT, 170)
          .Set(Am_TOP, 42)
          .Set(Am_WIDTH, 193)
          .Add(Am_VALUES, Am_No_Value) //set with old value
          .Set(Am_TEXT_CHECK_LEGAL_METHOD, am_check_valid_type)
          .Get_Object(Am_COMMAND)
          .Set(Am_LABEL, "")
          .Get_Owner()
          .Set(Am_FILL_STYLE, Am_Default_Color));
  am_valinvokegen.Add_Part(
      Am_TYPE_OBJECTS_WIDGET,
      Am_Text_Input_Widget.Create(DSTR("TYPE_OBJECTS_WIDGET"))
          .Set(Am_ACTIVE, am_active_if_type_sel)
          .Set(Am_LEFT, 170)
          .Set(Am_TOP, 77)
          .Set(Am_WIDTH, 193)
          .Add(Am_VALUES, Am_No_Value) //set with old value
          .Set(Am_ACTIVE_2, false)
          .Get_Object(Am_COMMAND)
          .Set(Am_LABEL, "")
          .Get_Owner()
          .Set(Am_FILL_STYLE, Am_Default_Color));
  am_valinvokegen.Add_Part(
      Am_ALL_FROM_COMMAND_WIDGET,
      Am_Text_Input_Widget.Create(DSTR("ALL_FROM_COMMAND_WIDGET"))
          .Set(Am_ACTIVE, am_active_all_values_sel)
          .Set(Am_ACTIVE_2, false) // **TEMPORARY
          .Set(Am_LEFT, 256)
          .Set(Am_TOP, 142)
          .Set(Am_WIDTH, 146)
          .Get_Object(Am_COMMAND)
          .Set(Am_LABEL, "")
          .Get_Owner()
          .Set(Am_FILL_STYLE, Am_Default_Color));
  am_valinvokegen.Add_Part(
      Am_COMMAND_LOAD_BUTTON,
      Am_Button.Create(DSTR("Am_LOAD_BUTTON"))
          .Set(Am_LEFT, 412)
          .Set(Am_TOP, 142)
          .Set(Am_FILL_STYLE, Am_Default_Color)
          .Set(Am_LEAVE_ROOM_FOR_FRINGE, false)
          .Set(Am_ACTIVE, am_active_all_values_sel)
          .Get_Object(Am_COMMAND)
          .Set(Am_LABEL, "Load")
          .Set(Am_DO_METHOD, am_load_current_command) // ** DOESN'T WORK
          .Get_Owner());
  am_valinvokegen.Add_Part(
      Am_Button_Panel.Create(DSTR("OK-cancel"))
          .Set(Am_LEFT, Am_Center_X_Is_Center_Of_Owner)
          .Set(Am_TOP, 210)
          .Set(Am_FILL_STYLE, Am_Default_Color)
          .Set(Am_LAYOUT, Am_Horizontal_Layout)
          .Set(Am_H_SPACING, 20)
          .Set(Am_V_SPACING, 0)
          .Set(Am_MAX_RANK, 0)
          .Set(Am_ITEMS, Am_Value_List()
                             .Add(Am_Standard_OK_Command.Create().Set(
                                 Am_DO_METHOD, am_valinvokegen_ok))
                             .Add(Am_Standard_Cancel_Command.Create().Set(
                                 Am_DO_METHOD, am_valinvokegen_cancel))));
  am_valinvokegen.Add_Part(Am_Tab_To_Next_Widget_Interactor.Create());
  return am_valinvokegen;
}

Am_Object
am_commandinvokegen_Initialize()
{
  am_commandinvokegen = Am_Window.Create(DSTR("am_commandinvokegen"))
                            .Set(Am_DESTROY_WINDOW_METHOD,
                                 Am_Default_Pop_Up_Window_Destroy_Method)
                            .Set(Am_FILL_STYLE, Am_Default_Color)
                            .Set(Am_TITLE, "Generalize Command for Match")
                            .Set(Am_WIDTH, 472)
                            .Set(Am_HEIGHT, 150)

                            .Add(Am_COMMAND, (0L))
                            .Add(Am_SLOTS_TO_SAVE, (0L))
                            .Add(Am_SCRIPT_WINDOW, (0L))
                            .Add(Am_PLACEHOLDERS_SET_BY_THIS_COMMAND, (0L))
                            .Add(Am_VALUE, (0L)) //the real value
                            .Add(Am_INVOKE_MATCH_COMMAND, (0L));
  am_commandinvokegen.Add_Part(
      Am_Text.Create()
          .Set(Am_LEFT, 9)
          .Set(Am_TOP, 9)
          .Set(Am_TEXT, "Generalize the command for match")
          .Set(Am_FONT,
               Am_Font(Am_FONT_FIXED, true, false, false, Am_FONT_LARGE))
          .Set(Am_LINE_STYLE, Am_Black)
          .Set(Am_FILL_STYLE, Am_No_Style));
  am_commandinvokegen.Add_Part(
      Am_UNDO_OPTIONS,
      Am_Radio_Button_Panel.Create(DSTR("radio control"))
          .Set(Am_LEFT, 15)
          .Set(Am_TOP, 40)
          .Set(Am_FILL_STYLE, Am_Default_Color)
          .Set(Am_ITEMS, Am_Value_List()
                             .Add(Am_Command.Create()
                                      .Set(Am_LABEL, "The specific command: ")
                                      .Set(Am_ID, am_constant_generalize))
                             .Add(Am_Command.Create()
                                      .Set(Am_LABEL, "Any command")
                                      .Set(Am_ID, am_any_value_generalize))
                             .Add(Am_Command.Create()
                                      .Set(Am_LABEL, "Custom...")
                                      .Set(Am_ACTIVE, false)
                                      .Set(Am_ID, am_custom_generalize)))
          .Set(Am_LAYOUT, Am_Vertical_Layout)
          .Set(Am_H_SPACING, 0));
  am_commandinvokegen.Add_Part(
      Am_CONSTANT_OBJECTS_WIDGET,
      Am_Text_Input_Widget.Create(DSTR("CONSTANT_OBJECTS_WIDGET"))
          .Set(Am_ACTIVE, am_active_if_constant_sel)
          .Set(Am_LEFT, 185)
          .Set(Am_TOP, 42)
          .Set(Am_WIDTH, 193)
          .Add(Am_VALUES, Am_No_Value) //set with old value
          .Set(Am_ACTIVE_2, false)
          .Get_Object(Am_COMMAND)
          .Set(Am_LABEL, "")
          .Get_Owner()
          .Set(Am_FILL_STYLE, Am_Default_Color));
  am_commandinvokegen.Add_Part(
      Am_Button_Panel.Create(DSTR("OK-cancel"))
          .Set(Am_LEFT, Am_Center_X_Is_Center_Of_Owner)
          .Set(Am_TOP, 115)
          .Set(Am_FILL_STYLE, Am_Default_Color)
          .Set(Am_LAYOUT, Am_Horizontal_Layout)
          .Set(Am_H_SPACING, 20)
          .Set(Am_V_SPACING, 0)
          .Set(Am_MAX_RANK, 0)
          .Set(Am_ITEMS, Am_Value_List()
                             .Add(Am_Standard_OK_Command.Create().Set(
                                 Am_DO_METHOD, am_commandinvokegen_ok))
                             .Add(Am_Standard_Cancel_Command.Create().Set(
                                 Am_DO_METHOD, am_commandinvokegen_cancel))));
  am_commandinvokegen.Add_Part(Am_Tab_To_Next_Widget_Interactor.Create());
  return am_commandinvokegen;
}
