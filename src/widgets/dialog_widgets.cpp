/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

// This file contains dialog box widgets
// Alan Ferrency 12-13-95

#include <am_inc.h>
#include <amulet/widgets_advanced.h>

#include <amulet.h>

static void
set_dialog_position(Am_Object &the_dialog, int x, int y)
{
  if (x == Am_AT_CENTER_SCREEN)
    x = ((int)Am_Screen.Get(Am_WIDTH) - (int)the_dialog.Get(Am_WIDTH)) / 2;
  if (y == Am_AT_CENTER_SCREEN)
    y = ((int)Am_Screen.Get(Am_HEIGHT) - (int)the_dialog.Get(Am_HEIGHT)) / 2;
  the_dialog.Set(Am_LEFT, x).Set(Am_TOP, y);
}

Am_Define_Formula(int, height_of_layout)
{
  int h = (int)Am_Height_Of_Parts(self) + 2 * ((int)self.Get(Am_TOP_OFFSET));
  return h < 75 ? 75 : h;
}

Am_Define_Formula(int, width_of_layout)
{
  int w = (int)Am_Width_Of_Parts(self) + 2 * ((int)self.Get(Am_LEFT_OFFSET));
  return w < 120 ? 120 : w;
}

// The items in a dialog's map of text items comes from the dialog's
// Am_ITEMS slot.
Am_Define_Formula(Am_Wrapper *, dialog_items)
{
  return self.Get_Owner().Get_Owner().Get(Am_ITEMS);
}

// returns dialog's am_valid_input slot
Am_Define_Formula(bool, valid_input)
{
  return self.Get_Owner().Get_Object(Am_WINDOW).Get(Am_VALID_INPUT);
}

// Returns the value of the dialog box
Am_Define_Formula(Am_Value, value_from_dialog)
{
  Am_Value value;
  static Am_String val("");

  Am_Object owner = self.Get_Owner();
  if (owner.Valid()) {
    Am_Object dialog = owner.Get_Object(Am_WINDOW, Am_NO_DEPENDENCY);
    if (dialog.Valid())
      value = dialog.Get(Am_VALUE);
    else
      value = Am_Value(val);
  } else
    value = Am_Value(val);
  return value;
}

Am_Define_Formula(Am_Value, value_from_text_part)
{
  Am_Value value;
  static Am_String val("");

  Am_Object owner = self.Get_Owner();
  if (owner.Valid())
    value = owner.Get_Object(Am_WINDOW)
                .Get_Object(Am_DIALOG_GROUP)
                .Get_Object(Am_TEXT_WIDGET)
                .Get(Am_VALUE);
  else
    value = Am_Value(val);
  return value;
}

// this goes in a dialog's Am_VALUE slot, and returns its command's value
Am_Define_Formula(Am_Value, value_from_command)
{
  Am_Value value;
  static Am_String val("");

  Am_Object part = self.Get_Object(Am_COMMAND);
  if (part.Valid())
    value = part.Get(Am_VALUE);
  else
    value = Am_Value(val);
  return value;
}

// Put this in a dialog box widget somewhere; it finds the widget's top
// level Am_COMMAND (for implementation parents
Am_Define_Object_Formula(cmd_from_dialog)
{
  // the window is the widget
  return self.Get_Owner().Get_Object(Am_WINDOW).Get_Object(Am_COMMAND);
}

// set the choice button return value into the choice dialog value slot
Am_Define_Method(Am_Object_Method, void, choice_dialog_stop, (Am_Object cmd))
{
  Am_Object widget = cmd.Get_Owner().Get(Am_WINDOW);
  Am_Value return_value = widget.Get_Object(Am_DIALOG_GROUP)
                              .Get_Object(Am_DIALOG_BUTTONS)
                              .Get(Am_VALUE);
  widget.Get_Object(Am_COMMAND).Set(Am_VALUE, return_value);
}

// set the text widget return value into the input dialog's Am_VALUE slot
Am_Define_Method(Am_Object_Method, void, input_dialog_stop, (Am_Object cmd))
{
  Am_Object widget = cmd.Get_Owner().Get(Am_WINDOW);
  Am_Value return_value = widget.Get_Object(Am_DIALOG_GROUP)
                              .Get_Object(Am_TEXT_WIDGET)
                              .Get(Am_VALUE);
  widget.Get_Object(Am_COMMAND).Set(Am_VALUE, return_value);
}

// abort the input dialog box from cancel button:
// abort text widget and call dialog's abort method.
Am_Define_Method(Am_Object_Method, void, input_dialog_abort, (Am_Object cmd))
{
  Am_Object widget = cmd.Get_Owner().Get_Object(Am_WINDOW);
  Am_Object text_widget =
      widget.Get_Object(Am_DIALOG_GROUP).Get_Object(Am_TEXT_WIDGET);
  Am_Abort_Widget(text_widget);
  // explicitly call dialog box's abort method.  We don't have any
  // implementation parent here, because then this stop method would call
  // the dialog's stop method as well as the abort method.
  // Is there a better way?
  Am_Object widget_cmd = widget.Get_Object(Am_COMMAND);
  Am_Object_Method aborter = widget_cmd.Get(Am_ABORT_DO_METHOD);
  aborter.Call(widget_cmd);
}

// explicitly finish a dialog box using finish_pop_up
Am_Define_Method(Am_Explicit_Widget_Run_Method, void, Am_Finish_Dialog_Method,
                 (Am_Object widget, Am_Value initial_value))
{
  if (initial_value.Valid())
    widget.Set(Am_VALUE, initial_value);
  if (widget.Valid()) {
    Am_Object window = widget.Get(Am_WINDOW);
    if (window.Valid())
      Am_Finish_Pop_Up_Waiting(window, Am_Value(false));
  }
  // else do nothing, something's not valid.
}

// explicitly run a dialog box non-modally using pop_up_window
Am_Define_Method(Am_Explicit_Widget_Run_Method, void, Am_Show_Dialog_Method,
                 (Am_Object widget, Am_Value iv))
{
  Am_Pop_Up_Window_And_Wait(widget, iv, false);
}

Am_Define_Method(Am_Object_Method, void, standard_ok_method, (Am_Object cmd))
{
  Am_Object widget = cmd.Get(Am_SAVED_OLD_OWNER);
  if (!widget.Valid())
    Am_Error("Lost widget");
  Am_Object window = widget.Get(Am_WINDOW);
  if (!window.Valid())
    Am_Error("Lost window");
  Am_Value v = cmd.Get(Am_VALUE);
  Am_Finish_Pop_Up_Waiting(window, v);
}
Am_Define_Method(Am_Object_Method, void, standard_cancel_method,
                 (Am_Object cmd))
{
  Am_Object widget = cmd.Get(Am_SAVED_OLD_OWNER);
  Am_Object window = widget.Get(Am_WINDOW);
  Am_Finish_Pop_Up_Waiting(window, Am_No_Value);
}

Am_Define_Method(Am_Object_Method, void, standard_about_amulet_method,
                 (Am_Object /* cmd */))
{
  Am_Object about_win = Am_Get_About_Amulet_Window();
  if (!about_win.Valid()) {
    Am_Show_Alert_Dialog("Can't find About Amulet Picture");
    return;
  }
  set_dialog_position(about_win, Am_AT_CENTER_SCREEN, Am_AT_CENTER_SCREEN);
  Am_Value dummy;
  Am_Pop_Up_Window_And_Wait(about_win, dummy, true);
}

Am_Value_List cached_alert_dialogs, cached_text_input_dialogs,
    cached_choice_dialogs;

Am_Object am_empty_dialog;
Am_Object Am_Text_Input_Dialog;
Am_Object Am_Alert_Dialog;
Am_Object Am_Choice_Dialog;

Am_Object Am_Standard_OK_Command;
Am_Object Am_Standard_Cancel_Command;

Am_Object am_about_amulet_window; //only loaded if needed
Am_Object Am_About_Amulet_Command;

static void
init()
{
  Am_Input_Char return_char = Am_Input_Char("RETURN");

  am_empty_dialog =
      Am_Window.Create(DSTR("empty_dialog_window"))
          .Set(Am_LEFT_OFFSET, 5) // used in width_of_layout
          .Set(Am_TOP_OFFSET, 5)  // used in height_of_layout
          .Set(Am_WIDTH, width_of_layout)
          .Set(Am_HEIGHT, height_of_layout)
          .Add(Am_WIDGET_LOOK, Am_Default_Widget_Look)
          .Set(Am_FILL_STYLE, Am_Motif_Gray)
          .Add(Am_ITEMS, 0)
          .Set(Am_V_SPACING, 5)
          .Set(Am_H_SPACING, 10)
          .Set(Am_H_ALIGN, Am_CENTER_ALIGN)
          .Add(Am_WIDGET_START_METHOD, Am_Show_Dialog_Method)
          .Add(Am_WIDGET_ABORT_METHOD, Am_Finish_Dialog_Method)
          .Add(Am_WIDGET_STOP_METHOD, Am_Finish_Dialog_Method)
          .Add(Am_VALUE, "")
          .Set(Am_VALUE, value_from_command.Multi_Constraint())
          .Set(Am_DESTROY_WINDOW_METHOD,
               Am_Default_Pop_Up_Window_Destroy_Method)
          .Add_Part(
              Am_COMMAND,
              Am_Command.Create(DSTR("Command_In_Dialog")).Set(Am_VALUE, ""))
          .Add_Part(
              Am_DIALOG_GROUP,
              Am_Widget_Group.Create(DSTR("Group_In_Dialog"))
                  .Set(Am_LEFT, Am_Center_X_Is_Center_Of_Owner)
                  .Set(Am_TOP, Am_Center_Y_Is_Center_Of_Owner)
                  .Set(Am_WIDTH, Am_Width_Of_Parts)
                  .Set(Am_HEIGHT, Am_Height_Of_Parts)
                  .Set(Am_LAYOUT, Am_Vertical_Layout)
                  .Set(Am_V_SPACING, Am_From_Owner(Am_V_SPACING))
                  .Set(Am_H_SPACING, Am_From_Owner(Am_H_SPACING))
                  .Set(Am_H_ALIGN, Am_From_Owner(Am_H_ALIGN))
                  .Add(Am_FILL_STYLE, Am_From_Owner(Am_FILL_STYLE))
                  .Add_Part(
                      Am_Map.Create(DSTR("Dialog_Prompt_Text"))
                          .Set_Part(Am_ITEM_PROTOTYPE,
                                    Am_Text.Create(DSTR("Prompt_Text_Proto"))
                                        .Add(Am_ITEM, "") //initialize
                                        .Set(Am_TEXT, Am_Same_As(Am_ITEM))
                                        .Set(Am_X_OFFSET, 0))
                          .Set(Am_LAYOUT, Am_Vertical_Layout)
                          .Set(Am_V_SPACING, Am_From_Owner(Am_V_SPACING))
                          .Set(Am_H_ALIGN, Am_From_Owner(Am_H_ALIGN))
                          .Set(Am_ITEMS, dialog_items)))
          .Get_Object(Am_COMMAND)
          .Set(Am_VALUE, "")
          //      .Set_Single_Constraint_Mode (Am_VALUE, false)
          .Get_Owner();

  Am_Alert_Dialog =
      am_empty_dialog.Create(DSTR("alert_dialog"))
          .Set(Am_TITLE, "Alert")
          .Set(Am_ICON_TITLE, "Alert")
          .Get_Object(Am_DIALOG_GROUP)
          .Add_Part(Am_Button.Create(DSTR("alert_dialog_ok"))
                        .Set(Am_WIDTH, 50)
                        .Set(Am_FILL_STYLE, Am_From_Owner(Am_FILL_STYLE))
                        .Set(Am_WIDGET_LOOK, Am_From_Owner(Am_WIDGET_LOOK))
                        .Get_Object(Am_COMMAND)
                        .Set(Am_LABEL, "OK")
                        .Set(Am_DEFAULT, true)
                        .Set(Am_ACCELERATOR, return_char)
                        .Set(Am_IMPLEMENTATION_PARENT, cmd_from_dialog)
                        //		 .Set (Am_DO_METHOD, do_hide_dialog)
                        .Get_Owner())
          .Get_Owner();
  ;

  Am_Text_Input_Dialog =
      am_empty_dialog.Create(DSTR("text_input_dialog"))
          .Set(Am_TITLE, "Text Input Dialog")
          .Set(Am_ICON_TITLE, "Text Input Dialog")
          .Add(Am_VALID_INPUT, true)
          // A high priority one-shot interactor to catch return presses.
          // It needs high priority to take precedence over the running text
          // widget.
          .Add_Part(Am_One_Shot_Interactor.Create(DSTR("text_catch_return"))
                        .Set(Am_START_WHEN, "RETURN")
                        .Set(Am_PRIORITY, 105)
                        .Set(Am_ACTIVE, valid_input)
                        .Get_Object(Am_COMMAND)
                        .Set(Am_IMPLEMENTATION_PARENT, cmd_from_dialog)
                        .Set(Am_DO_METHOD, input_dialog_stop)
                        .Get_Owner())
          .Get_Object(Am_COMMAND)
          .Set(Am_VALUE, "")
          .Set(Am_VALUE, value_from_text_part)
          .Get_Owner()
          .Get_Object(Am_DIALOG_GROUP)
          .Add_Part(Am_TEXT_WIDGET,
                    Am_Text_Input_Widget.Create(DSTR("text_input_in_dialog"))
                        .Set(Am_LEFT, Am_Center_X_Is_Center_Of_Owner)
                        .Set(Am_WIDTH, 200)
                        .Set(Am_LEFT_OFFSET, 0)
                        .Set(Am_FILL_STYLE, Am_From_Owner(Am_FILL_STYLE))
                        .Set(Am_WIDGET_LOOK, Am_From_Owner(Am_WIDGET_LOOK))
                        .Get_Object(Am_TEXT_WIDGET_TEXT_OBJ)
                        .Set(Am_TEXT, value_from_dialog)
                        .Get_Owner()
                        // we don't want the text widget to do anything on
                        // stop, because when you click outside a text widget
                        // it stops.  It interferes with the buttons.
                        .Get_Object(Am_COMMAND)
                        .Set(Am_LABEL, "")
                        .Get_Owner())
          .Add_Part(
              Am_DIALOG_BUTTONS,
              Am_Button_Panel.Create(DSTR("buttons_in_text_dialog"))
                  .Set(Am_FIXED_WIDTH, true)
                  .Set(Am_LAYOUT, Am_Horizontal_Layout)
                  .Set(Am_H_SPACING, Am_From_Owner(Am_H_SPACING))
                  .Set(Am_FILL_STYLE, Am_From_Owner(Am_FILL_STYLE))
                  .Set(Am_WIDGET_LOOK, Am_From_Owner(Am_WIDGET_LOOK))
                  .Set(
                      Am_ITEMS,
                      Am_Value_List()
                          .Add(Am_Command.Create(DSTR("text_input_okay"))
                                   .Set(Am_ACTIVE, valid_input)
                                   .Set(Am_LABEL, "OK")
                                   .Set(Am_DEFAULT, true)
                                   .Set(Am_IMPLEMENTATION_PARENT,
                                        cmd_from_dialog)
                                   .Set(Am_DO_METHOD, input_dialog_stop))
                          .Add(
                              Am_Command.Create(DSTR("text_input_cancel"))
                                  .Set(Am_LABEL, "Cancel")
                                  // don't want an implementation parent;
                                  // call the abort method of the dialog's command
                                  // explicitly.
                                  //.Set (Am_IMPLEMENTATION_PARENT, cmd_from_dialog)
                                  .Set(Am_DO_METHOD, input_dialog_abort))))
          .Get_Owner();
  ;

  Am_Choice_Dialog =
      am_empty_dialog.Create(DSTR("Choice_Dialog"))
          .Set(Am_TITLE, "Choice Dialog")
          .Set(Am_ICON_TITLE, "Choice Dialog")
          //    .Set (Am_VALUE, value_from_button_part)
          .Get_Object(Am_DIALOG_GROUP)
          .Add_Part(
              Am_DIALOG_BUTTONS,
              Am_Button_Panel.Create(DSTR("buttons_in_choice_dialog"))
                  .Set(Am_FIXED_WIDTH, true)
                  .Set(Am_LAYOUT, Am_Horizontal_Layout)
                  .Set(Am_H_SPACING, Am_From_Owner(Am_H_SPACING))
                  .Set(Am_FILL_STYLE, Am_From_Owner(Am_FILL_STYLE))
                  .Set(Am_WIDGET_LOOK, Am_From_Owner(Am_WIDGET_LOOK))
                  .Set(Am_ITEMS,
                       Am_Value_List()
                           .Add(Am_Command.Create(DSTR("choice_okay"))
                                    .Set(Am_LABEL, "OK")
                                    .Set(Am_DEFAULT, true)
                                    .Set(Am_ACCELERATOR, return_char)
                                    .Set(Am_IMPLEMENTATION_PARENT,
                                         cmd_from_dialog)
                                    .Set(Am_DO_METHOD, choice_dialog_stop))
                           .Add(Am_Command.Create(DSTR("choice_cancel"))
                                    .Set(Am_LABEL, "Cancel")
                                    .Set(Am_IMPLEMENTATION_PARENT,
                                         cmd_from_dialog)
                                    .Set(Am_DO_METHOD, choice_dialog_stop))))
          .Get_Owner();

  Am_Standard_OK_Command = Am_Command.Create(DSTR("Am_Standard_OK_Command"))
                               .Set(Am_VALUE, true)
                               .Set(Am_DO_METHOD, standard_ok_method)
                               .Set(Am_LABEL, "OK")
                               .Set(Am_DEFAULT, true)
                               .Set(Am_ACCELERATOR, return_char);
  Am_Standard_Cancel_Command =
      Am_Command.Create(DSTR("Am_Standard_Cancel_Command"))
          .Set(Am_VALUE, Am_No_Value)
          .Set(Am_DO_METHOD, standard_cancel_method)
          .Set(Am_LABEL, "Cancel");

  Am_About_Amulet_Command =
      Am_Command.Create(DSTR("Am_About_Amulet_Command"))
          .Set(Am_VALUE, Am_No_Value)
          .Set(Am_DO_METHOD, standard_about_amulet_method)
          .Set(Am_LABEL, "About Amulet")
          .Set(Am_IMPLEMENTATION_PARENT, true) //not undoable
      ;
}

static void
cleanup()
{
  am_empty_dialog.Destroy();
}

static Am_Initializer *initializer =
    new Am_Initializer(DSTR("Dialog_Widgets"), init, 5.6f, 109, cleanup);

// This goes in a dialog widget's Am_COMMAND object's stop method
Am_Define_Method(Am_Object_Method, void, do_stop_waiting_for_dialog,
                 (Am_Object cmd))
{
  Am_Object widget = cmd.Get_Owner();
  if (widget.Valid()) {
    // we want to stop the text input widget if it exists
    Am_Value v;
    v = widget.Peek(Am_VALUE);
    Am_Finish_Pop_Up_Waiting(widget, v);
  }
  // else do nothing, something's not valid.
}

// same as stop waiting, but return a (0L) value.
Am_Define_Method(Am_Object_Method, void, do_abort_waiting_for_dialog,
                 (Am_Object cmd))
{
  Am_Object widget = cmd.Get_Owner();
  if (widget.Valid())
    Am_Finish_Pop_Up_Waiting(widget, (0L));
}

// Assumes window was popped up, and pops it down, as if cancel was called
Am_Define_Method(Am_Object_Method, void,
                 Am_Default_Pop_Up_Window_Destroy_Method, (Am_Object window))
{
  if (window.Valid())
    Am_Finish_Pop_Up_Waiting(window, Am_No_Value);
}

// show a message in a dialog box and wait for the user to click okay.
void
Am_Show_Alert_Dialog(Am_Value_List alert_texts,
                     int x /* = Am_AT_CENTER_SCREEN*/,
                     int y /*= Am_AT_CENTER_SCREEN*/, bool modal /*= false*/)
{
  Am_Object the_dialog;
  if (cached_alert_dialogs.Empty())
    the_dialog = Am_Alert_Dialog.Create();
  else {
    the_dialog = cached_alert_dialogs.Pop();
  }
  the_dialog.Set(Am_ITEMS, alert_texts);
  the_dialog.Get_Object(Am_COMMAND)
      .Set(Am_DO_METHOD, do_stop_waiting_for_dialog)
      .Set(Am_ABORT_DO_METHOD, do_abort_waiting_for_dialog);
  Am_Screen.Add_Part(the_dialog);
  set_dialog_position(the_dialog, x, y);
  Am_Value dummy;
  Am_Pop_Up_Window_And_Wait(the_dialog, dummy, modal);
  Am_Screen.Remove_Part(the_dialog);
  cached_alert_dialogs.Push(the_dialog);
}

// show a message in a dialog box and wait for the user to click okay.
void
Am_Show_Alert_Dialog(Am_String alert_text, int x /* = Am_AT_CENTER_SCREEN */,
                     int y /*= Am_AT_CENTER_SCREEN */, bool modal /*= false*/)
{
  Am_Value_List alert_texts;
  alert_texts.Add(alert_text);
  Am_Show_Alert_Dialog(alert_texts, x, y, modal);
}

// get a line of text input from a dialog box
Am_Value
Am_Get_Input_From_Dialog(Am_Value_List prompt_texts,
                         Am_String initial_value /*= ""*/,
                         int x /*= Am_AT_CENTER_SCREEN */,
                         int y /* = Am_AT_CENTER_SCREEN */,
                         bool modal /*=false*/)
{
  const char *initial_string;
  if (initial_value.Valid())
    initial_string = initial_value;
  else
    initial_string = "";
  Am_Object the_dialog;
  if (cached_text_input_dialogs.Empty())
    the_dialog = Am_Text_Input_Dialog.Create();
  else {
    the_dialog = cached_text_input_dialogs.Pop();
  }
  the_dialog.Set(Am_ITEMS, prompt_texts);
  the_dialog.Get_Object(Am_COMMAND)
      .Set(Am_DO_METHOD, do_stop_waiting_for_dialog)
      .Set(Am_ABORT_DO_METHOD, do_abort_waiting_for_dialog);
  the_dialog.Set(Am_VALUE, initial_string);
  Am_Screen.Add_Part(the_dialog);
  set_dialog_position(the_dialog, x, y);
  Am_Object text_widget =
      the_dialog.Get_Object(Am_DIALOG_GROUP).Get_Object(Am_TEXT_WIDGET);
  Am_Start_Widget(text_widget, initial_string);
  Am_Value v;
  Am_Pop_Up_Window_And_Wait(the_dialog, v, modal);
  Am_Screen.Remove_Part(the_dialog);
  cached_text_input_dialogs.Push(the_dialog);
  return v;
}

// get a choice from a dialog box: "OK" or "Cancel"
Am_Value
Am_Get_Choice_From_Dialog(Am_Value_List prompt_texts,
                          int x /*= Am_AT_CENTER_SCREEN */,
                          int y /*= Am_AT_CENTER_SCREEN */,
                          bool modal /*=false*/)
{
  Am_Object the_dialog;
  if (cached_choice_dialogs.Empty())
    the_dialog = Am_Choice_Dialog.Create();
  else {
    the_dialog = cached_choice_dialogs.Pop();
  }

  the_dialog.Set(Am_ITEMS, prompt_texts);
  the_dialog.Get_Object(Am_COMMAND)
      .Set(Am_DO_METHOD, do_stop_waiting_for_dialog)
      .Set(Am_ABORT_DO_METHOD, do_abort_waiting_for_dialog);
  Am_Screen.Add_Part(the_dialog);
  set_dialog_position(the_dialog, x, y);
  Am_Value v;
  Am_Pop_Up_Window_And_Wait(the_dialog, v, modal);
  Am_Screen.Remove_Part(the_dialog);
  cached_choice_dialogs.Push(the_dialog);
  return v;
}

// this pops up some (custom?) dialog box that a user creates,
// waits for it to complete, and returns its value.
Am_Value
Am_Show_Dialog_And_Wait(Am_Object the_dialog, bool modal /*= false*/)
{
  Am_Object cmd = the_dialog.Peek_Object(Am_COMMAND);
  if (cmd.Valid()) {
    cmd.Set(Am_DO_METHOD, do_stop_waiting_for_dialog, Am_OK_IF_NOT_THERE)
        .Set(Am_ABORT_DO_METHOD, do_abort_waiting_for_dialog,
             Am_OK_IF_NOT_THERE);
  }
  Am_Value v;
  Am_Pop_Up_Window_And_Wait(the_dialog, v, modal);
  return v;
}

//*** TEMPORARY
Am_String
Am_Show_File_Dialog_For_Open(Am_String initial_value)
{
  Am_Value v = Am_Get_Input_From_Dialog(
      Am_Value_List().Add("Type filename for Read:"), initial_value);
  if (v.Valid())
    return v;
  else
    return "";
}
//*** TEMPORARY
Am_String
Am_Show_File_Dialog_For_Save(Am_String initial_value)
{
  Am_Value v = Am_Get_Input_From_Dialog(
      Am_Value_List().Add("Type filename for Write:"), initial_value);
  if (v.Valid())
    return v;
  else
    return "";
}

#ifdef _WIN32
#define ABOUT_AMULET_GIF "images/aboutamulet.gif"
#else
#define ABOUT_AMULET_GIF "images/aboutamulet.gif"
#endif

//this returns the About Amulet window, which then might be popped up.
Am_Object
Am_Get_About_Amulet_Window()
{
  if (!am_about_amulet_window.Valid()) {
    Am_Font bigfont = Am_Font(Am_FONT_SERIF, true, false, false, Am_FONT_LARGE);
    const char *pathname = Am_Merge_Pathname(ABOUT_AMULET_GIF);
    Am_Image_Array picture = Am_Image_Array(pathname);
    delete[] pathname;
    if (!picture.Valid()) {
      return Am_No_Object;
    }
    Am_Object bitmap =
        Am_Bitmap.Create(DSTR("About Amulet GIF")).Set(Am_IMAGE, picture);
    am_about_amulet_window =
        Am_Window.Create(DSTR("About Amulet"))
            .Set(Am_DESTROY_WINDOW_METHOD,
                 Am_Default_Pop_Up_Window_Destroy_Method)
            .Set(Am_WIDTH, bitmap.Get(Am_WIDTH))
            .Set(Am_HEIGHT, bitmap.Get(Am_HEIGHT))
            .Set(Am_TITLE, "About Amulet")
            .Set(Am_ICON_TITLE, "About Amulet")
            .Set(Am_USE_MIN_WIDTH, true)
            .Set(Am_USE_MAX_WIDTH, true)
            .Set(Am_USE_MIN_HEIGHT, true)
            .Set(Am_USE_MAX_HEIGHT, true)
            .Set(Am_MIN_WIDTH, Am_Same_As(Am_WIDTH))
            .Set(Am_MAX_WIDTH, Am_Same_As(Am_WIDTH))
            .Set(Am_MIN_HEIGHT, Am_Same_As(Am_HEIGHT))
            .Set(Am_MAX_HEIGHT, Am_Same_As(Am_HEIGHT))

            .Add_Part(bitmap)
            .Add_Part(Am_Text.Create()
                          .Set(Am_LEFT, 210)
                          .Set(Am_TOP, 195)
                          .Set(Am_TEXT, "Brad A. Myers,")
                          .Set(Am_FONT, bigfont))
            .Add_Part(Am_Text.Create()
                          .Set(Am_LEFT, 210)
                          .Set(Am_TOP, 215)
                          .Set(Am_TEXT, "Rich McDaniel, Rob Miller,")
                          .Set(Am_FONT, bigfont))
            .Add_Part(Am_Text.Create()
                          .Set(Am_LEFT, 210)
                          .Set(Am_TOP, 235)
                          .Set(Am_TEXT, "Alan Ferrency, Patrick Doane,")
                          .Set(Am_FONT, bigfont))
            .Add_Part(Am_Text.Create()
                          .Set(Am_LEFT, 210)
                          .Set(Am_TOP, 255)
                          .Set(Am_TEXT, "Andrew Faulring, Ellen Borison")
                          .Set(Am_FONT, bigfont))
            .Add_Part(Am_Text.Create()
                          .Set(Am_LEFT, 210)
                          .Set(Am_TOP, 275)
                          .Set(Am_TEXT, "Andy Mickish, and Alex Klimovitski")
                          .Set(Am_FONT, bigfont))
            .Add_Part(Am_Text.Create()
                          .Set(Am_LEFT, 419)
                          .Set(Am_TOP, 120)
                          .Set(Am_TEXT, Am_VERSION)
                          .Set(Am_FONT, Am_Font(Am_FONT_SERIF, true, false,
                                                false, Am_FONT_VERY_LARGE)))
            .Add_Part(Am_One_Shot_Interactor.Create()
                          .Set_Part(Am_COMMAND, Am_Standard_OK_Command.Create())
                          .Set(Am_START_WHEN,
                               Am_Value_List()
                                   .Add(Am_Input_Char("ANY_MOUSE_DOWN"))
                                   .Add(Am_Input_Char("RETURN"))));
  }
  return am_about_amulet_window;
}
