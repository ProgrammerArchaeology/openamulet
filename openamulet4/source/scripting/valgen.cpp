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

#ifdef DEBUG
#include DEBUGGER__H //for Am_Parse_Input_As_Type
#endif

#include REGISTRY__H //for Am_Get_Type_Name

#include <amulet/impl/widget_misc.h>

Am_Object am_valgen;

Am_Slot_Key Am_NEXT_CONSTANT_OBJECTS_WIDGET =
	Am_Register_Slot_Name ("NEXT_CONSTANT_OBJECTS_WIDGET");

Am_Define_Method(Am_Text_Check_Legal_Method, Am_Text_Abort_Or_Stop_Code,
		 am_check_valid_type,
		 ( Am_Object &text, Am_Object& inter) ) {
  Am_String str = text.Get(Am_TEXT);
  Am_Object widget = inter.Get_Owner();
  Am_Value old_value = widget.Get(Am_VALUES);
  Am_Value output_value;
#ifdef DEBUG
  bool ok = Am_Parse_Input_As_Type(str, old_value.type, output_value);
#else
  // TODO: Study an alternate solution for the release version !! -- ortalo
  bool ok = false;
 std::cout << "*** Unable to parse type: inspector is disabled ***"
       <<std::endl <<std::flush;
#endif
  if (!ok) {
    Am_POP_UP_ERROR_WINDOW("Sorry, could not parse `" << str << "' as a "
			   << Am_Get_Type_Name(old_value.type));
    return Am_TEXT_ABORT_AND_RESTORE;
  }
  else {
    widget.Set(Am_VALUES, output_value);
    return Am_TEXT_OK;
  }
}

Am_Define_Method(Am_Text_Check_Legal_Method, Am_Text_Abort_Or_Stop_Code,
		 am_check_valid_list_type,
		 ( Am_Object &text, Am_Object& inter) ) {
  Am_String str = text.Get(Am_TEXT);
  Am_Object widget = inter.Get_Owner();
  Am_Value_List old_values = widget.Get(Am_VALUES);
  Am_Value first_val = old_values.Get_First();
  Am_Value output_value;
#ifdef DEBUG
  bool ok = Am_Parse_Input_As_List_Of_Type(str, first_val.type, output_value);
#else
  // TODO: Study an alternate solution for the release version !! -- ortalo
  bool ok = false;
 std::cout << "*** Unable to parse list of type: inspector is disabled ***"
       <<std::endl <<std::flush;
#endif
  if (!ok) {
    Am_POP_UP_ERROR_WINDOW("Sorry, could not parse `" << str
			   << "' as a list of "
			   << Am_Get_Type_Name(first_val.type));
    return Am_TEXT_ABORT_AND_RESTORE;
  }
  else {
    widget.Set(Am_VALUES, output_value);
    widget.Set(Am_VALUE, output_value); //may look different if new length
    return Am_TEXT_OK;
  }
}

Am_Define_Formula(bool, have_palette) {
  Am_Object window = self.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  return (window.Peek(Am_REGISTRY_FOR_PALETTES).Valid());
}

Am_Define_Formula(bool, am_active_if_next_value_sel) {
  return self.Get_Sibling(Am_UNDO_OPTIONS).Get(Am_VALUE) ==
    am_next_value_generalize;
}

Am_Object am_valgen_Initialize () {
  am_valgen = Am_Window.Create(DSTR("am_genval"))
    .Set(Am_DESTROY_WINDOW_METHOD, Am_Default_Pop_Up_Window_Destroy_Method)
    .Set(Am_FILL_STYLE, Am_Default_Color)
    .Set(Am_TITLE, "Generalize Value")
    .Set(Am_ICON_TITLE, "Generalize Value")
    .Set(Am_WIDTH, 472)
    .Set(Am_HEIGHT, 328)

    .Add(Am_PLACEHOLDERS_SET_BY_THIS_COMMAND, (0L))
    .Add(Am_SCRIPT_WINDOW, (0L))
    .Add(Am_COMMAND, (0L))
    .Add(Am_SLOTS_TO_SAVE, (0L))
    .Add(Am_REGISTRY_FOR_PALETTES, (0L))
    .Add(Am_VALUE, (0L))
    ;
  am_valgen
    .Add_Part(Am_Border_Rectangle.Create()
      .Set(Am_LEFT, 14)
      .Set(Am_TOP, 112)
      .Set(Am_WIDTH, 455)
      .Set(Am_HEIGHT, 164)
      .Set(Am_SELECTED, 0)
      .Set(Am_FILL_STYLE, Am_Default_Color)
    );
  am_valgen
    .Add_Part(Am_Border_Rectangle.Create()
      .Set(Am_LEFT, 14)
      .Set(Am_TOP, 39)
      .Set(Am_WIDTH, 455)
      .Set(Am_HEIGHT, 65)
      .Set(Am_SELECTED, 0)
      .Set(Am_FILL_STYLE, Am_Default_Color)
    );
  am_valgen
    .Add_Part(Am_Text.Create()
      .Set(Am_LEFT, 9)
      .Set(Am_TOP, 9)
      .Set(Am_WIDTH, 198)
      .Set(Am_HEIGHT, 22)
      .Set(Am_TEXT, "Generalize Value")
      .Set(Am_FONT, Am_Font(Am_FONT_FIXED, true, false, false, Am_FONT_LARGE))
      .Set(Am_LINE_STYLE, Am_Black)
      .Set(Am_FILL_STYLE, Am_No_Style)
    );
  am_valgen
    .Add_Part(Am_UNDO_OPTIONS, Am_Radio_Button_Panel.Create()
      .Set(Am_LEFT, 27)
      .Set(Am_TOP, 123)
      .Set(Am_FILL_STYLE, Am_Default_Color)
      .Set(Am_ITEMS, Am_Value_List()
        .Add(Am_Command.Create()
             .Set(Am_LABEL, "Constant: ")
             .Set(Am_ID, am_constant_generalize))
        .Add(Am_Command.Create()
             .Set(Am_LABEL, "The value of the palette when the script runs")
	     .Set(Am_ACTIVE, have_palette)
             .Set(Am_ID, am_palette_generalize))
        .Add(Am_Command.Create()
             .Set(Am_LABEL, "Next value from the list:")
             .Set(Am_ID, am_next_value_generalize))
        .Add(Am_Command.Create()
             .Set(Am_LABEL, "Ask user when script runs")
             .Set(Am_ID, am_ask_user_generalize))
        .Add(Am_Command.Create()
             .Set(Am_LABEL, "The value returned from command:")
             .Set(Am_ID, am_all_values_generalize))
        .Add(Am_Command.Create()
             .Set(Am_LABEL, "Custom...")
	     .Set(Am_ACTIVE, false)
             .Set(Am_ID, am_custom_generalize))
        )
      .Set(Am_LAYOUT, Am_Vertical_Layout)
      .Set(Am_H_SPACING, 0)
      .Set(Am_V_SPACING, 0)
    );
  am_valgen
    .Add_Part(Am_CONSTANT_OBJECTS_WIDGET,
	      Am_Text_Input_Widget.Create(DSTR("CONSTANT_OBJECTS_WIDGET"))
      .Set(Am_ACTIVE, am_active_if_constant_sel)
      .Set(Am_LEFT, 121)
      .Set(Am_TOP, 123)
      .Set(Am_WIDTH, 193)
      .Set(Am_HEIGHT, 25)
	      .Add(Am_VALUES, Am_No_Value) //set with old value
	      .Set(Am_TEXT_CHECK_LEGAL_METHOD, am_check_valid_type)
      .Get_Object(Am_COMMAND)
        .Set(Am_LABEL, "")
        .Get_Owner()
      .Set(Am_FILL_STYLE, Am_Default_Color)
    );
  am_valgen
    .Add_Part(Am_NEXT_CONSTANT_OBJECTS_WIDGET,
	      Am_Text_Input_Widget.Create(DSTR("NEXT_CONSTANT_OBJECTS_WIDGET"))
      .Set(Am_ACTIVE, am_active_if_next_value_sel)
      .Set(Am_LEFT, 227)
      .Set(Am_TOP, 174)
      .Set(Am_WIDTH, 193)
      .Set(Am_HEIGHT, 25)
	      .Add(Am_VALUES, Am_No_Value) //set with old value
	      .Set(Am_TEXT_CHECK_LEGAL_METHOD, am_check_valid_list_type)
      .Get_Object(Am_COMMAND)
        .Set(Am_LABEL, "")
        .Get_Owner()
      .Set(Am_FILL_STYLE, Am_Default_Color)
    );
  am_valgen
    .Add_Part(Am_ALL_FROM_COMMAND_WIDGET,
	      Am_Text_Input_Widget.Create(DSTR("ALL_FROM_COMMAND_WIDGET"))
	      .Set(Am_ACTIVE, am_active_all_values_sel)
	      .Set(Am_ACTIVE_2, false ) // **TEMPORARY
      .Set(Am_LEFT, 276)
      .Set(Am_TOP, 222)
      .Set(Am_WIDTH, 146)
      .Set(Am_HEIGHT, 25)
      .Get_Object(Am_COMMAND)
        .Set(Am_LABEL, "")
        .Get_Owner()
      .Set(Am_FILL_STYLE, Am_Default_Color)
    );
  am_valgen
    .Add_Part(Am_COMMAND_LOAD_BUTTON, Am_Button.Create(DSTR("Am_LOAD_BUTTON"))
      .Set(Am_LEFT, 423)
      .Set(Am_TOP, 222)
      .Set(Am_FILL_STYLE, Am_Default_Color)
	      .Set(Am_LEAVE_ROOM_FOR_FRINGE, false)
	      .Set(Am_ACTIVE, am_active_all_values_sel)
	      .Get_Object(Am_COMMAND)
	        .Set(Am_LABEL, "Load")
	        .Set(Am_DO_METHOD, am_load_current_command)
	        .Get_Owner()
    );
  am_valgen
    .Add_Part(Am_Button_Panel.Create(DSTR("OK-cancel"))
      .Set(Am_LEFT, 167)
      .Set(Am_TOP, 287)
      .Set(Am_FILL_STYLE, Am_Default_Color)
      .Set(Am_LAYOUT, Am_Horizontal_Layout)
      .Set(Am_H_SPACING, 20)
      .Set(Am_V_SPACING, 0)
      .Set(Am_MAX_RANK, 0)
      .Set(Am_ITEMS, Am_Value_List()
        .Add(Am_Standard_OK_Command.Create()
	      .Set(Am_DO_METHOD, am_valgen_ok)
	     )
        .Add(Am_Standard_Cancel_Command.Create()
	      .Set(Am_DO_METHOD, am_valgen_cancel)
	     )
        )
    );
  am_valgen
    .Add_Part(Am_GREEN_AND_YELLOW,
	      Am_Radio_Button_Panel.Create(DSTR("GREEN_AND_YELLOW"))
      .Set(Am_LEFT, 27)
      .Set(Am_TOP, 45)
      .Set(Am_FILL_STYLE, Am_Default_Color)
      .Set(Am_ITEMS, Am_Value_List()
        .Add(Am_Command.Create()
             .Set(Am_LABEL, "Generalize green selected value")
             .Set(Am_ID, am_generalize_only_green))
        .Add(Am_Command.Create()
             .Set(Am_LABEL, "Generalize green and yellow selected values")
             .Set(Am_ID, am_generalize_green_and_yellow))
        )
      .Set(Am_LAYOUT, Am_Vertical_Layout)
      .Set(Am_H_SPACING, 0)
      .Set(Am_V_SPACING, 0)
      .Set(Am_MAX_RANK, 0)
    );
  am_valgen
    .Add_Part(Am_Tab_To_Next_Widget_Interactor.Create())
  ;
  return am_valgen;
}
