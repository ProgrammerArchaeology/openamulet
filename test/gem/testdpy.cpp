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

#include <amulet/am_io.h>

#include <amulet/gem.h>
#include "test_utils.h"

wins *wins_array; // The wins class is defined in test_utils.h

int
main(int argc, char **argv)
{

  if (argc > 3) {
    Am_Error("** usage: testdpy [scrn1] [scrn2]\n");
  };

  int ar_len;
  wins_array = create_wins_array(argc, argv, ar_len);

  test_colors(wins_array, ar_len);
  test_fonts(wins_array, ar_len);
  test_bits(wins_array, ar_len);
//  test_win_props (wins_array, ar_len);

///
///  Exit
///
#if _MACINTOSH
  // on the Macintosh, we need to be in the main loop in order to
  // get cursor changes correctly.

  cout << "Entering main loop" << endl;
  Am_Drawonable::Main_Loop();
#else
  // but on other machines we want to test for crash on exit.
  printf("Hit RETURN to exit:\n");
  getchar();
#endif

  return 0;
}
