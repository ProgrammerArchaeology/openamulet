#include <amulet.h>

// image filenames
#if defined(_WINDOWS) || defined(_MACINTOSH)
#define IMFN_HAPPY	"lib/images/happy.gif"
#define IMFN_HAPPY_M	"lib/images/happy_m.gif"
#define IMFN_DS9	"lib/images/ds9.gif"
#define IMFN_DS9_M	"lib/images/ds9_m.gif"
#define IMFN_BORG	"lib/images/borg.gif"
#define IMFN_BORG_M	"lib/images/borg_m.gif"
#define IMFN_CROSSHR	"lib/images/crsshr.gif"
#define IMFN_CROSSHR_M	"lib/images/crsshr_m.gif"
#define IMFN_ROM	"lib/images/rom.gif"
#define IMFN_ROM_M	"lib/images/rom_m.gif"
#else
#define IMFN_HAPPY	"lib/images/happy.xbm"
#define IMFN_HAPPY_M	"lib/images/happy_mask.xbm"
#define IMFN_DS9	"lib/images/ds9.xbm"
#define IMFN_DS9_M	"lib/images/ds9_mask.xbm"
#define IMFN_BORG	"lib/images/borg.xbm"
#define IMFN_BORG_M	"lib/images/borg_mask.xbm"
#define IMFN_CROSSHR	"lib/images/crosshair.xbm"
#define IMFN_CROSSHR_M	"lib/images/crosshair_mask.xbm"
#define IMFN_ROM	"lib/images/rom.xbm"
#define IMFN_ROM_M	"lib/images/rom_mask.xbm"
#endif

using namespace std;

Am_Object window;
Am_Image_Array full_image, mask_image;
Am_Cursor my_cursor;

Am_Define_Method( Am_Object_Method, void, change_setting, (Am_Object cmd) )
{
  Am_Object inter = cmd.Get_Owner();
  Am_Input_Char c = inter.Get( Am_START_CHAR );

  switch( c.As_Char() ) {
  case '1': {
    full_image = Am_Image_Array(Am_Merge_Pathname(IMFN_HAPPY));
    mask_image = Am_Image_Array(Am_Merge_Pathname(IMFN_HAPPY_M));
    my_cursor = Am_Cursor(full_image, mask_image, Am_Red, Am_Yellow);
    window.Set(Am_CURSOR, my_cursor);
    break;
  }
  case '2': {
    full_image = Am_Image_Array(Am_Merge_Pathname(IMFN_DS9));
    mask_image = Am_Image_Array(Am_Merge_Pathname(IMFN_DS9_M));
    my_cursor = Am_Cursor(full_image, mask_image, Am_Blue, Am_Green);
    window.Set(Am_CURSOR, my_cursor);
    break;
  }
  case '3': {
    full_image = Am_Image_Array(Am_Merge_Pathname(IMFN_BORG));
    mask_image = Am_Image_Array(Am_Merge_Pathname(IMFN_BORG_M));
    my_cursor = Am_Cursor(full_image, mask_image, Am_Purple, Am_Orange);
    window.Set(Am_CURSOR, my_cursor);
    break;
  }
  case '4': {
    full_image = Am_Image_Array(Am_Merge_Pathname(IMFN_CROSSHR));
    mask_image = Am_Image_Array(Am_Merge_Pathname(IMFN_CROSSHR_M));
    my_cursor = Am_Cursor(full_image, mask_image, Am_Yellow, Am_Black);
    window.Set(Am_CURSOR, my_cursor);
    break;
  }
  case '5': {
    full_image = Am_Image_Array(Am_Merge_Pathname(IMFN_ROM));
    mask_image = Am_Image_Array(Am_Merge_Pathname(IMFN_ROM_M));
    my_cursor = Am_Cursor(full_image, mask_image, Am_White, Am_Red);
    window.Set(Am_CURSOR, my_cursor);
    break;
  }
  case '6': {
    window.Set(Am_CURSOR, Am_Default_Cursor);
    break;
  }
  case 'q': {
    Am_Exit_Main_Event_Loop();
    break;
  }
  }
}
int main (void) {

  Am_Initialize(); 

  window = Am_Window.Create("window")
    .Set(Am_LEFT, 20)
    .Set(Am_TOP,  45)
    .Set(Am_WIDTH, 500)
    .Set(Am_HEIGHT, 200)
    .Set(Am_TITLE, "Happy Cursor Window")
    .Set(Am_FILL_STYLE, Am_Amulet_Purple);
  
  Am_Object how_set_inter = Am_One_Shot_Interactor.Create("change_settings")
    .Set(Am_START_WHEN, "ANY_KEYBOARD")
    .Set (Am_START_WHERE_TEST, true)
    ;
  Am_Object cmd = how_set_inter.Get(Am_COMMAND);
  cmd.Set(Am_DO_METHOD, change_setting);
  window.Add_Part (how_set_inter);

  Am_Screen.Add_Part(window);

  cout << "Type '1' to change cursor to red/yellow happy face" << endl;
  cout << "Type '2' to change cursor to blue/green ds9 ship" << endl;
  cout << "Type '3' to change cursor to purple/orange borg ship" << endl;
  cout << "Type '4' to change cursor to yellow/black crosshair" << endl;
  cout << "Type '5' to change cursor to white/red romulan ship" << endl;
  cout << "Type '6' to restore cursor Am_No_Value" << endl;
  cout << "Type 'q' to quit " << endl << flush;

  Am_Main_Event_Loop(); 
  Am_Cleanup (); 
  return 0;
}

