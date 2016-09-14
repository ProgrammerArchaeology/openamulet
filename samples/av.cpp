/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <stdlib.h>
#include <amulet.h>

using namespace std;

/*
av: amulet gif viewing program
Takes a list of gif images as parameters, and puts each one in a window
on the screen.
*/

Am_Define_Method(Am_Object_Method, void, quit_method, (Am_Object))
{
  Am_Exit_Main_Event_Loop();
}

int
main(int argc, char **argv)
{
  if (argc <= 1) {
    cerr << "Usage: av <filename.gif> ...\n";
    exit(-1);
  }

  Am_Initialize();

  Am_Object window;
  Am_Image_Array image_array;
  int i;
  const char *filename;
  for (i = 1; i < argc; i++) {
    filename = argv[i];
    image_array = Am_Image_Array(filename);
    if (!image_array.Valid()) {
      cerr << "av: Invalid gif file: " << filename << endl;
      continue;
    }
    window = Am_Window.Create()
                 .Set(Am_LEFT, 50 + 10 * i)
                 .Set(Am_TOP, 50 + 10 * i)
                 .Set(Am_MIN_WIDTH, 1)
                 .Set(Am_USE_MIN_WIDTH, true)
                 .Set(Am_WIDTH, Am_Width_Of_Parts)
                 .Set(Am_MIN_HEIGHT, 1)
                 .Set(Am_USE_MIN_HEIGHT, true)
                 .Set(Am_HEIGHT, Am_Height_Of_Parts)
                 .Add_Part(Am_Bitmap.Create().Set(Am_IMAGE, image_array))
                 .Add_Part(Am_One_Shot_Interactor.Create()
                               .Get_Object(Am_COMMAND)
                               .Set(Am_DO_METHOD, quit_method)
                               .Get_Owner());
    Am_Screen.Add_Part(window);
  }
  Am_Main_Event_Loop();
  Am_Cleanup();

  return 0;
}
