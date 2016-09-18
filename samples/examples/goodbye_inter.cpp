/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

// A simple program that uses an interactor to make the window go away
// Click on the text to exit the application and make the window go away

#include <amulet.h>

Am_Define_Method( Am_Object_Method, void, quit_method, (Am_Object) )
{
  Am_Exit_Main_Event_Loop();
}

int main (void)
{
  Am_Initialize ();

  Am_Screen
    .Add_Part( Am_Window.Create ("window")
      .Set( Am_LEFT, 50 )
      .Set( Am_TOP, 50 )
      .Set( Am_WIDTH, 200 )
      .Set( Am_HEIGHT, 50 )
      .Add_Part( Am_Text.Create ("string")
        .Set( Am_TEXT, "Goodbye World!")
        .Add_Part( Am_One_Shot_Interactor.Create()
          .Get_Object( Am_COMMAND )
          .Set( Am_DO_METHOD, quit_method )
          .Get_Owner()
        )
      )
    );

  Am_Main_Event_Loop ();
  Am_Cleanup ();

  return 0;
}
