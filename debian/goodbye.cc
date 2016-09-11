/* **************************************************************-*-c++-*-*
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

// A simple program that uses a button to make the window go away

#include <amulet.h>

int main( void )
{
  Am_Initialize();

  Am_Object my_win = Am_Window.Create ("my_win")
    .Set (Am_LEFT, 20)
    .Set (Am_TOP, 50)
    .Set( Am_WIDTH, 200 )
    .Set( Am_HEIGHT, 50 )
    .Set( Am_FILL_STYLE, Am_Amulet_Purple )
    .Add_Part( Am_Button.Create("button")
	       .Set_Part( Am_COMMAND, Am_Quit_No_Ask_Command.Create()
			  .Set( Am_LABEL, "Goodbye, world!" )
			  )
	       );
  
  Am_Screen.Add_Part(my_win);

  Am_Main_Event_Loop();
  Am_Cleanup();

  return 0;
}
