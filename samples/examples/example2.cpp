/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* Example2: Using dialog boxes and the Am_Text_Input_Widget.  Demonstrates:
   1) Using text input fields
   2) Constraining a value to be the same as a text field's value
   3) Popping up a dialog box and waiting for the answer
   4) Using the error built-in dialog box.
   5) Using a dialog box created using the Gilt Interface Builder
   6) Use of the Am_Tab_To_Next_Widget_Interactor to allow the user to
      tab among the fields
   7) Using the filter on text input field values
   8) Using a number entry field
*/   

#include <amulet.h>
#include <string.h>

#include "example2db.h" //this .h file was created automatically by Gilt

// strstream.h used to do cout to a string
#include <amulet/am_strstream.h>

using namespace std;

// make this be a global variable so it can be accessed by the constraints 
Am_Object age_field;

// pop-up window.  Created once at initialization time, and made
//   visible whenever needed.  Here, we know that we can only have one
//   of these visible at a time.  If needed more than one, then would
//   create instances of this prototype.  Would probably save the
//   created instances in a list and re-use them for efficiency.
Am_Object pop_up_window;

// This is a string to show the value returned from the popup
Am_Object pop_up_value_string;

//Slot used to hold the widget in the popup window.  Give this slot a
//name so it is easy to find the widget given the window.
Am_Slot_Key MY_WIDGET = Am_Register_Slot_Name ("MY_WIDGET");

//Define a formula which returns a string.
//This is used in the Am_TEXT slot of the oldness_label text string
Am_Define_No_Self_Formula(Am_Wrapper*, label_from_age_field) {
  //value of a widget is always in the widget's Am_VALUE slot
  //since age_field is a Am_Number_Input_Widget, the value will always
  //be a number.
  int i = age_field.Get(Am_VALUE);
  const char* ret;
  if (i == 0) ret = "No value yet";
  else if (i < 4) ret = "Baby";
  else if (i < 18) ret = "Child";
  else if (i < 65) ret = "Adult";
  else ret = "Senior";
  return Am_String(ret);
}

// convert the value into a string for the pop_up_value_string
void Set_Value_Into_String(Am_Value &value) {
  cout << "Value is `" << value << "'" << endl << flush;
  if (value.Valid()) { //otherwise, user hit Cancel
    char line[100];
    OSTRSTREAM_CONSTR (oss,line, 100, ios::out);
    oss << "Value is `" << value << "'" << ends;
    OSTRSTREAM_COPY(oss,line,100);
    pop_up_value_string.Set(Am_TEXT, line);
  }
}

//define a method of type Am_Object_Method which is called
//pop_up_new_dialog and returns void and takes a single object as a
//parameter.  This method is used as the DO_METHOD of the command in
//a button.  Note: the cmd argument is not used, so comment it out to
//avoid a compiler warning.
Am_Define_Method(Am_Object_Method, void, pop_up_new_dialog,
		 (Am_Object /* cmd */)) {
  //cause the pre-defined window to become visible and wait for the
  //user to click on the OK or cancel buttons
  Am_Value value = Am_Show_Dialog_And_Wait(pop_up_window);
  //now convert the value into a string for the pop_up_value_string
  Set_Value_Into_String(value);
}

//Use the built-in dialog boxes to read a string
Am_Define_Method(Am_Object_Method, void, pop_up_built_in_dialog,
		 (Am_Object /* cmd */)) {
  Am_Value value =
    Am_Get_Input_From_Dialog (Am_Value_List().Add("Type a string to return:"));
  //now convert the value into a string for the pop_up_value_string
  Set_Value_Into_String(value);
}
  

//Use the dialog box created by Gilt
Am_Define_Method(Am_Object_Method, void, pop_up_gilt_dialog,
		 (Am_Object /* cmd */)) {
  Am_Value ok;
  // Often, before a dialog box is popped up, you want to set the
  // default value of the widgets in the dialog box.  Do this by
  // simply setting the Am_VALUE slot of the widget.
  Am_String default_string = pop_up_value_string.Get(Am_TEXT);

  example2db.Get_Object(GILT_DB_VALUE).Set(Am_VALUE, default_string);
  
  Am_Pop_Up_Window_And_Wait(example2db, ok, true);
  if (ok.Valid()) { //then user hit OK
    // get the new value out of the widget
    Am_Value new_value = example2db.Get_Object(GILT_DB_VALUE).Get(Am_VALUE);
    //now convert the value into a string for the pop_up_value_string
    Set_Value_Into_String(new_value);
  }
}

Am_Define_Method(Am_Object_Method, void, ok_method,
		 (Am_Object /* cmd */)) {
  //Get the value to return out of the radio button panel in the pop_up_window
  Am_Value value_to_return =
    pop_up_window.Get_Object(MY_WIDGET).Get(Am_VALUE);
  
  // To tell the dialog box to go away, use Am_Finish_Pop_Up_Waiting.
  // If the window that was popped up was not a global variable, then
  // could have stored the corresponding window in a slot of the
  // command object.
  Am_Finish_Pop_Up_Waiting(pop_up_window, value_to_return);
}

Am_Define_Method(Am_Object_Method, void, cancel_method,
		 (Am_Object /* cmd */)) {
  Am_Finish_Pop_Up_Waiting(pop_up_window, Am_No_Value);
}

//initialize the global variable pop_up_window with an invisible
//window, which will be used as a pop up dialog box.
void Create_Pop_Up_Dialog() {
  pop_up_window = Am_Window.Create("pop up window")
    .Set (Am_VISIBLE, false) //start off window as invisible
    .Set (Am_LEFT, 100)
    .Set (Am_TOP, 100)
    .Set (Am_WIDTH, 275)
    .Set (Am_HEIGHT, 100)
    .Set (Am_FILL_STYLE, Am_Motif_Light_Blue)
    // the Am_DESTROY_WINDOW_METHOD is called if the user closes the
    // window using the close box.  The default method Destroys the
    // window, which we don't want to do.  Instead, use popup window handler,
    // which does Cancel.
    .Set (Am_DESTROY_WINDOW_METHOD, Am_Default_Pop_Up_Window_Destroy_Method)
    .Add_Part(MY_WIDGET, Am_Radio_Button_Panel.Create()
	      .Set(Am_LEFT, 20)
	      .Set(Am_TOP, 20)
	      .Set (Am_FILL_STYLE, Am_Motif_Light_Blue)
	      .Set (Am_ITEMS, Am_Value_List()
		    .Add("Hamburger")
		    .Add("Hot dog")
		    .Add("Chicken Nuggets")
		    )
	      .Set(Am_VALUE, "Hamburger") //initial value for widget
	      )
    .Add_Part(Am_Button_Panel.Create("ok-cancel")
	      //use a built-in formula for left that will make be
	      //centered in window
	      .Set(Am_LEFT, 175)
	      .Set(Am_TOP, 10)
	      .Set(Am_FILL_STYLE, Am_Motif_Light_Blue)
	      .Set(Am_ITEMS, Am_Value_List()
		   //the default Am_Standard_OK_Command just returns
		   //true, but our custom ok_method will return the
		   //correct value.
		   .Add(Am_Command.Create("ok")
			.Set(Am_LABEL, "OK")
			.Set(Am_DO_METHOD, ok_method))
		   .Add(Am_Command.Create("cancel")
			.Set(Am_LABEL, "CANCEL")
			.Set(Am_DO_METHOD, cancel_method))
		   )
	      )
    ;
}

// This method is called when the user types RETURN to finish off the
// value in the soc_field widget.  It can do whatever is desired to
// show the error, and it can modify the text to make it more correct.
// It then returns what should happen.  Am_Text_Abort_Or_Stop_Code is
// defined in inter.h, and options are Am_TEXT_OK, Am_TEXT_ABORT_AND_RESTORE,
// Am_TEXT_KEEP_RUNNING, Am_TEXT_STOP_ANYWAY.
// If needed, the Am_Text_Input_Widget object itself can be gotten
// using inter.Get_Owner(); 

Am_Define_Method(Am_Text_Check_Legal_Method, Am_Text_Abort_Or_Stop_Code,
		 check_soc_number, (Am_Object &text, Am_Object& /*inter */)){
  Am_String str = text.Get(Am_TEXT);
  int len = strlen(str);
  if (len == 9) return Am_TEXT_OK;
  else {
    AM_POP_UP_ERROR_WINDOW("Social Security Number must be 9 digits.")
    return Am_TEXT_KEEP_RUNNING;
  }
}
  
int main (void)
{
  Am_Initialize(); //All Amulet code starts with this

  // for Gilt-created dialog boxes, don't forget to initialize them!
  // The name of the initialization procedure is always the name of
  // the main object with "_Initialize()" attached.
  
  example2db_Initialize ();

  //Create the main window, setting its size.
  Am_Object window = Am_Window.Create("window")
    .Set(Am_TOP, 50)
    .Set(Am_LEFT, 50)
    .Set(Am_WIDTH, 500)
    .Set(Am_HEIGHT, 200)
    .Set(Am_TITLE, "Amulet Example 2")
    // Set the background color so the widgets will look better
    .Set(Am_FILL_STYLE, Am_Amulet_Purple);

  Am_Object name_field = Am_Text_Input_Widget.Create("name_field")
    .Set(Am_TOP, 30)
    .Set(Am_LEFT, 10)
    .Set(Am_WIDTH, 200);
  name_field.Get_Object(Am_COMMAND)
    .Set(Am_LABEL, "Name");

  age_field = Am_Number_Input_Widget.Create("age_field")
    .Set(Am_TOP, 60)
    .Set(Am_LEFT, 10)
    .Set(Am_WIDTH, 70);
  age_field.Get_Object(Am_COMMAND)
    .Set(Am_LABEL, "Age");

  Am_Font bold_font (Am_FONT_SERIF, true);
  Am_Object oldness_label = Am_Text.Create("oldness_label")
    .Set(Am_LEFT, 100)
    .Set(Am_TOP, 65)
    .Set(Am_TEXT, label_from_age_field)
    .Set(Am_FONT, bold_font)
    ;
  
  Am_Object soc_field = Am_Text_Input_Widget.Create("soc_field")
    .Set(Am_TOP, 90)
    .Set(Am_LEFT, 10)
    .Set(Am_WIDTH, 250)
    .Set(Am_TEXT_CHECK_LEGAL_METHOD, check_soc_number);
  soc_field.Get_Object(Am_COMMAND)
    .Set(Am_LABEL, "Social Security Number");

  Create_Pop_Up_Dialog();  //create the custom pop up window
  
  Am_Object button_panel = Am_Button_Panel.Create("buttons")
    //use a built-in formula for left that will make be centered in window
    .Set(Am_LEFT, Am_Center_X_Is_Center_Of_Owner)
    .Set(Am_TOP, 130)

    .Set(Am_LAYOUT, Am_Horizontal_Layout) // make the buttons be horizontal 
    .Set(Am_FIXED_WIDTH, false) //looks better this way

    .Set(Am_ITEMS, Am_Value_List()
	 .Add(Am_Command.Create()
	      .Set(Am_LABEL, "Pop Up New Dialog")
	      .Set(Am_DO_METHOD, pop_up_new_dialog)
	      )
	 .Add(Am_Command.Create()
	      .Set(Am_LABEL, "Pop Up Built-in Dialog")
	      .Set(Am_DO_METHOD, pop_up_built_in_dialog)
	      )
	 .Add(Am_Command.Create()
	      .Set(Am_LABEL, "Pop Up Gilt Dialog")
	      .Set(Am_DO_METHOD, pop_up_gilt_dialog)
	      )
	 .Add(Am_Quit_No_Ask_Command.Create())
	 )
    ;
  pop_up_value_string = Am_Text.Create("pop_up_value_string")
    .Set(Am_LEFT, 18)
    .Set(Am_TOP, 170)
    .Set(Am_FONT, bold_font)
    .Set(Am_TEXT, "No value yet")
    ;

  //now add all the parts to the window
  window.Add_Part(name_field)
    .Add_Part(age_field)
    .Add_Part(oldness_label)
    .Add_Part(soc_field)
    .Add_Part(button_panel)
    .Add_Part(pop_up_value_string)
    // allow tabbing from one widget to another
    .Add_Part (Am_Tab_To_Next_Widget_Interactor.Create("My_Tab_Interactor"))
    ;

  
  Am_Screen.Add_Part(window);

  Am_Main_Event_Loop(); //Now process input events
  Am_Cleanup ();  //All Amulet code ends with this
  return 0;
}
