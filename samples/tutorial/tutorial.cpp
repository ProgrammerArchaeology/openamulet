/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <amulet.h>

int main (void)
{
  Am_Initialize ();
  
  Am_Object my_win = Am_Window.Create ("my_win")
    .Set (Am_LEFT, 20)
    .Set (Am_TOP, 50);
  
  Am_Screen.Add_Part (my_win);
  /* ************************************************************ */
  /* During the Tutorial, do not add or edit text below this line */
  /* ************************************************************ */
  
  Am_Main_Event_Loop ();
  Am_Cleanup ();

  return 0;
}
