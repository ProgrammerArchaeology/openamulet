/////////////////////////////////////////////////////////////////////////
//  Pop Up Windows
/////////////////////////////////////////////////////////////////////////
//Typical usage: programmer calls Am_Pop_Up_Window_And_Wait, and
//the window has a call to Am_Finish_Pop_Up_Waiting as part of the
//do_method of the OK and CANCEL buttons, with OK setting the value to
//something, and cancel setting the value to (0L).

// Sets the visible of the window to true, and then waits for someone
// to call Am_Finish_Pop_Up_Waiting on that window.  Returns the value
// passed to Am_Finish_Pop_Up_Waiting.
_OA_DL_IMPORT extern void Am_Pop_Up_Window_And_Wait(Am_Object window,
				      Am_Value &return_value,
				      bool modal = true);

//Sets window's visible to FALSE, and makes the
//Am_Pop_Up_Window_And_Wait called on the same window return
_OA_DL_IMPORT extern void Am_Finish_Pop_Up_Waiting(Am_Object window, Am_Value return_value);
