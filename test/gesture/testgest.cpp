/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <am_inc.h>

// For exit()
#include <stdlib.h>

#include AM_IO__H

#include STANDARD_SLOTS__H
#include INTER_ADVANCED__H
#include VALUE_LIST__H
#include INTER__H
#include OPAL__H
#include FORMULA__H
#include TEXT_FNS__H
#include DEBUGGER__H
#include REGISTRY__H

#include GESTURE__H

using namespace std;

Am_Define_Method(Am_Object_Method, void, Gesture1_Do, (Am_Object cmd))
{
  Am_Object inter = cmd.Get_Owner();
  Am_Value v;

  v = cmd.Peek(Am_VALUE);
  if (v.Valid())
    cout << "** Recognized: " << v << endl;
  else
    cout << "** UNRECOGNIZED" << endl;

  cout << "(nap = " << (double)inter.Get(Am_NONAMBIGUITY_PROB)
       << ", dist=" << (double)inter.Get(Am_DIST_TO_MEAN) << ")" << endl;
}

int
main(int argc, char *argv[])
{
  Am_Gesture_Classifier gc;

  Am_Initialize();

  if (argc > 1) {
    gc = Am_Gesture_Classifier(argv[1]);
    if (!gc.Valid()) {
      cerr << "can't open %s\n" << argv[1] << endl;
      exit(-1);
    }
  }

  Am_Object window = Am_Window.Create("window")
                         .Set(Am_LEFT, 20)
                         .Set(Am_TOP, 50)
                         .Set(Am_WIDTH, 500)
                         .Set(Am_HEIGHT, 500);
  Am_Screen.Add_Part(window);

  Am_Object gesture1_feedback =
      Am_Polygon.Create("gesture1_feedback").Set(Am_FILL_STYLE, (0L));
  window.Add_Part(gesture1_feedback);

  Am_Object gesture1 = Am_Gesture_Interactor.Create("gesture1")
                           .Set(Am_FEEDBACK_OBJECT, gesture1_feedback)
                           .Set(Am_CLASSIFIER, gc)
                           .Set(Am_MIN_NONAMBIGUITY_PROB, 0.95)
                           .Set(Am_MAX_DIST_TO_MEAN, 200);
  gesture1.Get_Object(Am_COMMAND).Set(Am_DO_METHOD, Gesture1_Do);
  window.Add_Part(gesture1);

  Am_Main_Event_Loop();
  Am_Cleanup();

  return 0;
}
