/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <stdlib.h>

#include <am_inc.h>

#include AM_IO__H
#include GEM__H

#define TESTINPUT_D_LEFT  800
#define TESTINPUT_D_TOP     0
#define TESTINPUT_D2_LEFT 800
#define TESTINPUT_D2_TOP  300

using namespace std;
Am_Input_Char test_ic;
bool test_ic_set = false;
Am_Drawonable *d, *d2;

class my_input_handlers : public Am_Input_Event_Handlers {
    void Iconify_Notify (Am_Drawonable* draw, bool) {
	cout << "My Iconify Notify on " << draw << endl; }
    void Frame_Resize_Notify (Am_Drawonable* draw, int, int, int, int) {
	cout << "My Frame Resize Notify on " << draw << endl; }
    void Destroy_Notify (Am_Drawonable *draw) {
	cout << "My Destroy Notify on " << draw << endl; }
    void Configure_Notify (Am_Drawonable *draw, int /* left */, int /* top */,
				   int /* width */, int /* height */) {
	cout << "My Configure Notify on " << draw << endl; }
    void Exposure_Notify (Am_Drawonable *draw,
				  int /* left */, int /* top */,
				  int /* width */, int /* height */) {
	cout << "My Exposure Notify on " << draw << endl; }
    void Input_Event_Notify (Am_Drawonable *draw, Am_Input_Event *ev) {
      char s[100];
      
      ev->input_char.As_Short_String(s);
      cout << "My Input Event \"" << ev->input_char << "\" short=\""
	   << s << "\" on ";
      if (draw == d) cout << "d\n";
      else if (draw == d2) cout << "d2\n";

      Am_Value v = Am_Value (ev->input_char);
      Am_Input_Char ic2 = v;
      Am_Value v2 = Am_Value (ic2);
      unsigned char c = ev->input_char.As_Char();
     
      cout << "   (unsigned char)=`" << c << "'=(char)=" << (unsigned int)c
	   << (long)c << " (long) " << v.value.long_value
	   << " back to Input_Char = " << v2.value.long_value
	   << " new long = " << v2.value.long_value << endl;
      
      if (test_ic_set) {
	  if (ev->input_char == test_ic) cout << "   *=* ";
	  else cout << "   *NOT* = ";
	  cout << " test_ic (" << test_ic << ")\n" << flush;
	}
      if ( ev->input_char == "CONTROL_M") {
	cout << "==Turning on want move\n";
	    draw->Set_Want_Move(true);
	}
      else if ( ev->input_char == "CONTROL_m") {
	    cout << "==Turning off want move\n";
	    draw->Set_Want_Move(false);
	}
	else if ( ev->input_char == "CONTROL_x") {
	    cout << "==Turning on want move\n";
	    draw->Set_Want_Move(true);
	}
	else if ( ev->input_char == "CONTROL_e") {
	    cout << "==Turning off enter leave\n";
	    draw->Set_Enter_Leave(false);
   	}
	else if ( ev->input_char == "CONTROL_E") {
	    cout << "==Turning on enter leave\n";
	    draw->Set_Enter_Leave(true);
	  }
	else if ( ev->input_char == "CONTROL_w") {
	    cout << "==Turning off multiple windows\n";
	    draw->Set_Multi_Window(false);
   	}
	else if ( ev->input_char == "CONTROL_W") {
	    cout << "==Turning on multiple windows\n";
	    draw->Set_Multi_Window(true);
	  }
	else if ( ev->input_char == "CONTROL_t") {
	    cout << "type new input string to test against: ";
	    cin >> s;
	    test_ic = Am_Input_Char (s);
	    test_ic_set = true;
	  }
 	else if ( ev->input_char == "CONTROL_q") {
	    cout << "Quitting\n" << flush;     
	    exit(0);
	  }
   }
};

int main ()
{ 

  my_input_handlers handlers;

  test_ic = Am_Input_Char ("Q");
  
  Am_Drawonable *root = Am_Drawonable::Get_Root_Drawonable();

  cout << "Size of Am_Input_Char is " << sizeof(test_ic) << endl;

  Am_Debug_Print_Input_Events = 1;

  d = root->Create (TESTINPUT_D_LEFT, TESTINPUT_D_TOP, 500, 200, "testinput",
       "testinput icon", true, false, Am_No_Style, false, 1, 1, 0, 0,
       true, false, false, false, &handlers);

  d2 = root->Create (TESTINPUT_D2_LEFT, TESTINPUT_D2_TOP, 500, 200, "testinput2",
       "testinput2 icon", true, false, Am_No_Style, false, 1, 1, 0, 0,
       true, false, false, false, &handlers);

  cout << "window 1 = " << d << endl;
  cout << "window 2 = " << d2 << endl;

  d->Flush_Output ();
  d2->Flush_Output ();
    
  cout << "******  Now type keys into window.\n";
  cout << "^M want mouse move, ^m don't want mouse move.\n";
  cout << "^E want enter leave, ^e don't want enter leave.\n";
  cout << "^W want multiple windows, ^w don't want multiple windows.\n";
  cout << "^t to change test input string.\n";

  Am_Drawonable::Main_Loop();

  return 0;
}
