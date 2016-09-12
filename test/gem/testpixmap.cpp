/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/*
  This program tests the display of transparent and opaque GIF images
  Typing a digit between '1' and '7' causes different images to be
  overlayed on top of an eye.
   - image '1' is an X bitmap file; the remaining images are .gifs
   - even images are transparent; odd images are opaque
   - the background (transparent) color is white for all images
     except spaceship
  Typing 'f' attempts to display a nonexistent GIF (does nothing)
  The fill style can be changed by typing a lowercase letter
   - 'b' black, 'g' green, 'n' none, 'r' red, 'w' white
  The line style can be changed by typing an uppercase letter
   - 'B' black, 'G' green, 'N' none, 'R' red, 'W' white
  The drawing mode can be set to monochrome by typing 'm'
  The drawing mode can be set to color by typing 'c'
  The fill style is used for the background of transparent images and
  when drawing in monochrome mode.  A fill style of Am_No_Style is
  transparent.
  The line style is used for the foreground when drawing in
  monochrome mode.
  The eye should be pink with a blue iris.
  Typing 't' will print out the current image in RGB form if supported
    (row 0 first).
  Typing 'T' will print out bugsmall.gif in RBG form if supported
    (last row first).
  Typing 'z' will print out and display spiral bitmap image if supported
    (row 0 first);
  Typing 'Z' will print out spiral bitmap image if supported
    (last row first);
*/

#include <amulet.h>

using namespace std;
Am_Object bitmap, fill_style, line_style, mode, image_name;
char spiral[] = {0x78, 0x00, 0xc4, 0x01, 0x32, 0x06, 0x1a, 0x0c,
                 0xca, 0x08, 0x4a, 0x09, 0x0a, 0x09, 0x9a, 0x09,
                 0xe2, 0x08, 0x0e, 0x06, 0x0c, 0x03};

// 0x1e, 0x00, 0x23, 0x80, 0x4c, 0x60, 0x58, 0x30,
// 0x53, 0x10, 0x52, 0x90, 0x50, 0x20, 0x59, 0x90,
// 0x47, 0x10, 0x60, 0x60, 0x30, 0xc0

void
print_RGB_image(unsigned const char *storage, const char *name, int width,
                int height, bool top_first)
{
  int row, col, index = 0;
  if (top_first) {
    cout << "RGB for " << name << " first row first" << endl;
    for (row = 0; row < height; row++) {
      cout << "Row " << row << ":";
      for (col = 0; col < width; col++) {
        if ((col / 5) * 5 == col)
          cout << endl;
        cout << " (" << (int)storage[index++] << "," << (int)storage[index++]
             << "," << (int)storage[index++] << ")";
      }
      cout << endl;
    }
  } else {
    cout << "RGB for " << name << " last row first" << endl;
    for (row = height - 1; row >= 0; row--) {
      cout << "Row " << row << ":";
      for (col = 0; col < width; col++) {
        if ((col / 5) * 5 == col)
          cout << endl;
        cout << " (" << (int)storage[index++] << "," << (int)storage[index++]
             << "," << (int)storage[index++] << ")";
      }
      cout << endl;
    }
  }
}

Am_Define_Method(Am_Object_Method, void, change_setting, (Am_Object cmd))
{
  Am_Object inter = cmd.Get_Owner();
  Am_Input_Char c = inter.Get(Am_START_CHAR);

  switch (c.As_Char()) {
  case '1': {
    const char *pathname = Am_Merge_Pathname("lib/images/ent.xbm");
    Am_Image_Array image = Am_Image_Array(pathname);
    delete[] pathname;
    bitmap.Set(Am_IMAGE, image);
    image_name.Set(Am_TEXT, "ent.xbm");
    break;
  }
  case '2': {
    const char *pathname = Am_Merge_Pathname("lib/images/ent.gif");
    Am_Image_Array image = Am_Image_Array(pathname);
    delete[] pathname;
    bitmap.Set(Am_IMAGE, image);
    image_name.Set(Am_TEXT, "ent.gif (transparent)");
    break;
  }
  case '3': {
    const char *pathname = Am_Merge_Pathname("lib/images/opaque_ent.gif");
    Am_Image_Array image = Am_Image_Array(pathname);
    delete[] pathname;
    bitmap.Set(Am_IMAGE, image);
    image_name.Set(Am_TEXT, "opaque_ent.gif");
    break;
  }
  case '4': {
    const char *pathname = Am_Merge_Pathname("lib/images/eye10.gif");
    Am_Image_Array image = Am_Image_Array(pathname);
    delete[] pathname;
    bitmap.Set(Am_IMAGE, image);
    image_name.Set(Am_TEXT, "eye10.gif (transparent)");
    break;
  }
  case '5': {
    const char *pathname = Am_Merge_Pathname("lib/images/opaque_eye10.gif");
    Am_Image_Array image = Am_Image_Array(pathname);
    delete[] pathname;
    bitmap.Set(Am_IMAGE, image);
    image_name.Set(Am_TEXT, "opaque-eye10.gif");
    break;
  }
  case '6': {
    const char *pathname = Am_Merge_Pathname("lib/images/spaceship.gif");
    Am_Image_Array image = Am_Image_Array(pathname);
    delete[] pathname;
    bitmap.Set(Am_IMAGE, image);
    image_name.Set(Am_TEXT, "spaceship.gif (transparent)");
    break;
  }
  case '7': {
    const char *pathname = Am_Merge_Pathname("lib/images/opaque_spaceship.gif");
    Am_Image_Array image = Am_Image_Array(pathname);
    delete[] pathname;
    bitmap.Set(Am_IMAGE, image);
    image_name.Set(Am_TEXT, "opaque_spaceship.gif");
    break;
  }
  case 'm': {
    bitmap.Set(Am_DRAW_MONOCHROME, true);
    mode.Set(Am_TEXT, "monochrome");
    break;
  }
  case 'c': {
    bitmap.Set(Am_DRAW_MONOCHROME, false);
    mode.Set(Am_TEXT, "color");
    break;
  }
  case 'b': {
    bitmap.Set(Am_FILL_STYLE, Am_Black);
    fill_style.Set(Am_TEXT, "Am_Black");
    break;
  }
  case 'g': {
    bitmap.Set(Am_FILL_STYLE, Am_Green);
    fill_style.Set(Am_TEXT, "Am_Green");
    break;
  }
  case 'n': {
    bitmap.Set(Am_FILL_STYLE, Am_No_Style);
    fill_style.Set(Am_TEXT, "Am_No_Style");
    break;
  }
  case 'r': {
    bitmap.Set(Am_FILL_STYLE, Am_Red);
    fill_style.Set(Am_TEXT, "Am_Red");
    break;
  }
  case 'w': {
    bitmap.Set(Am_FILL_STYLE, Am_White);
    fill_style.Set(Am_TEXT, "Am_White");
    break;
  }
  case 'B': {
    bitmap.Set(Am_LINE_STYLE, Am_Black);
    line_style.Set(Am_TEXT, "Am_Black");
    break;
  }
  case 'G': {
    bitmap.Set(Am_LINE_STYLE, Am_Green);
    line_style.Set(Am_TEXT, "Am_Green");
    break;
  }
  case 'N': {
    bitmap.Set(Am_LINE_STYLE, Am_No_Style);
    line_style.Set(Am_TEXT, "Am_No_Style");
    break;
  }
  case 'R': {
    bitmap.Set(Am_LINE_STYLE, Am_Red);
    line_style.Set(Am_TEXT, "Am_Red");
    break;
  }
  case 'W': {
    bitmap.Set(Am_LINE_STYLE, Am_White);
    line_style.Set(Am_TEXT, "Am_White");
    break;
  }
  case 'f': {
    const char *pathname = Am_Merge_Pathname("lib/images/does_not_exist.gif");
    Am_Image_Array image = Am_Image_Array(pathname);
    delete[] pathname;
    bitmap.Set(Am_IMAGE, image);
    image_name.Set(Am_TEXT, "does_not_exist.gif");
    break;
  }
  case 't': {
    Am_Image_Array image = bitmap.Get(Am_IMAGE);
    int width, height;

    image.Get_Size(width, height);
    Am_String name = image_name.Get(Am_TEXT);

    unsigned char *storage = new unsigned char[width * height * 3];
    if (image.Get_RGB_Image(storage)) {
      print_RGB_image(storage, name, width, height, true);
    } else {
      cout << "Get_RGB_Image not implemented on this platform." << endl;
    }
    delete[] storage;
    break;
  }
  case 'T': {
    const char *pathname = Am_Merge_Pathname("lib/images/bugsmall.gif");
    Am_Image_Array image = Am_Image_Array(pathname);
    delete[] pathname;
    int width, height;

    image.Get_Size(width, height);

    unsigned char *storage = new unsigned char[width * height * 3];
    if (image.Get_RGB_Image(storage, false)) {
      print_RGB_image(storage, "bugsmall.gif", width, height, false);
    } else {
      cout << "Get_RGB_Image not implemented on this platform." << endl;
    }
    delete[] storage;
    break;
  }
  case 'z': {
    Am_Image_Array image = Am_Image_Array(spiral, 11, 13);
    bitmap.Set(Am_IMAGE, image);
    image_name.Set(Am_TEXT, "spiral");

    int width, height;
    image.Get_Size(width, height);
    Am_String name = image_name.Get(Am_TEXT);

    unsigned char *storage = new unsigned char[width * height * 3];
    if (image.Get_RGB_Image(storage)) {
      print_RGB_image(storage, "spiral", width, height, true);
    } else {
      cout << "Get_RGB_Image not implemented on this platform." << endl;
    }
    delete[] storage;
    break;
  }
  case 'Z': {
    Am_Image_Array image = Am_Image_Array(spiral, 11, 13);

    int width, height;
    image.Get_Size(width, height);

    unsigned char *storage = new unsigned char[width * height * 3];
    if (image.Get_RGB_Image(storage, false)) {
      print_RGB_image(storage, "spiral", width, height, false);
    } else {
      cout << "Get_RGB_Image not implemented on this platform." << endl;
    }
    delete[] storage;
    break;
  }
  case 'q': {
    Am_Exit_Main_Event_Loop();
  }
  }
}

int
main(void)
{
  Am_Initialize();

  Am_Object window =
      Am_Window.Create()
          .Set(Am_TITLE, "Amulet Pixmap Test")
          .Set(Am_WIDTH, 600)
          .Set(Am_HEIGHT, 300)
          .Set(Am_FILL_STYLE,
               Am_Amulet_Purple) // tests putting on top of color backgrounds
      ;

  const char *pathname = Am_Merge_Pathname("lib/images/bugsmall.gif");
  Am_Image_Array image = Am_Image_Array(pathname);
  delete[] pathname;
  bitmap = Am_Bitmap.Create("bitmap")
               .Set(Am_LEFT, 10)
               .Set(Am_TOP, 10)
               .Set(Am_IMAGE, image);
  window.Add_Part(bitmap);

  pathname = Am_Merge_Pathname("lib/images/eye10.gif");
  image = Am_Image_Array(pathname);
  delete[] pathname;
  Am_Object bitmap2 = Am_Bitmap.Create("bitmap2").Set(Am_IMAGE, image);
  window.Add_Part(bitmap2);
  Am_To_Bottom(bitmap2);

  Am_Object how_set_inter = Am_One_Shot_Interactor.Create("change_settings")
                                .Set(Am_START_WHEN, "ANY_KEYBOARD")
                                .Set(Am_START_WHERE_TEST, true);
  Am_Object cmd = how_set_inter.Get(Am_COMMAND);
  cmd.Set(Am_DO_METHOD, change_setting);
  window.Add_Part(how_set_inter);

  Am_Object settings_group = Am_Group.Create("settings group")
                                 .Set(Am_TOP, 100)
                                 .Set(Am_LEFT, 10)
                                 .Set(Am_WIDTH, Am_Width_Of_Parts)
                                 .Set(Am_HEIGHT, Am_Height_Of_Parts);

  Am_Object settings = Am_Text.Create("settings").Set(Am_TEXT, "Settings:");
  settings_group.Add_Part(settings);

  int text_spacing = (int)settings.Get(Am_HEIGHT) + 2;
  int line_position = text_spacing + (int)settings.Get(Am_HEIGHT);

  Am_Object fill_setting = Am_Text.Create("fill style setting")
                               .Set(Am_TOP, text_spacing)
                               .Set(Am_TEXT, "Fill Style");
  settings_group.Add_Part(fill_setting)
      .Add_Part(Am_Line.Create().Set(Am_TOP, line_position).Set(Am_WIDTH, 135));

  Am_Object line_setting = Am_Text.Create("line style setting")
                               .Set(Am_TOP, text_spacing)
                               .Set(Am_LEFT, 145)
                               .Set(Am_TEXT, "Line Style");
  settings_group.Add_Part(line_setting)
      .Add_Part(Am_Line.Create()
                    .Set(Am_TOP, line_position)
                    .Set(Am_LEFT, 145)
                    .Set(Am_WIDTH, 135));

  Am_Object mode_setting = Am_Text.Create("mode setting")
                               .Set(Am_TOP, text_spacing)
                               .Set(Am_LEFT, 290)
                               .Set(Am_TEXT, "Drawing Mode");
  settings_group.Add_Part(mode_setting)
      .Add_Part(Am_Line.Create()
                    .Set(Am_TOP, line_position)
                    .Set(Am_LEFT, 290)
                    .Set(Am_WIDTH, 135));

  Am_Object image_setting = Am_Text.Create("image setting")
                                .Set(Am_TOP, text_spacing)
                                .Set(Am_LEFT, 435)
                                .Set(Am_TEXT, "Image Name");
  settings_group.Add_Part(image_setting)
      .Add_Part(Am_Line.Create()
                    .Set(Am_TOP, line_position)
                    .Set(Am_LEFT, 435)
                    .Set(Am_WIDTH, 135));

  text_spacing = (text_spacing * 2) + 3;

  fill_style = Am_Text.Create("fill style")
                   .Set(Am_TOP, text_spacing)
                   .Set(Am_TEXT, "Am_No_Style (default)");
  settings_group.Add_Part(fill_style);

  line_style = Am_Text.Create("line style")
                   .Set(Am_TOP, text_spacing)
                   .Set(Am_LEFT, 145)
                   .Set(Am_TEXT, "Am_Black (default)");
  settings_group.Add_Part(line_style);

  mode = Am_Text.Create("mode")
             .Set(Am_TOP, text_spacing)
             .Set(Am_LEFT, 290)
             .Set(Am_TEXT, "color");
  settings_group.Add_Part(mode);

  image_name = Am_Text.Create("image name")
                   .Set(Am_TOP, text_spacing)
                   .Set(Am_LEFT, 435)
                   .Set(Am_TEXT, "bugsmall.gif");
  settings_group.Add_Part(image_name);

  window.Add_Part(settings_group);
  Am_Screen.Add_Part(window);

  cout << "Type '1' to display ent.xbm" << endl;
  cout << "Type '2' to display ent.gif (transparent)" << endl;
  cout << "Type '3' to display opaque_ent.gif" << endl;
  cout << "Type '4' to display eye10.gif (transparent)" << endl;
  cout << "Type '5' to display opaque_eye10.gif" << endl;
  cout << "Type '6' to display spaceship.gif (transparent)" << endl;
  cout << "Type '7' to display opaque_spaceship.gif" << endl;
  cout << "Type 'b' to set fill style to Am_Black" << endl;
  cout << "Type 'g' to set fill style to Am_Green" << endl;
  cout << "Type 'n' to set fill style to Am_No_Style" << endl;
  cout << "Type 'r' to set fill style to Am_Red" << endl;
  cout << "Type 'w' to set fill style to Am_White" << endl;
  cout << "Type 'B' to set line style to Am_Black" << endl;
  cout << "Type 'G' to set line style to Am_Green" << endl;
  cout << "Type 'N' to set line style to Am_No_Style" << endl;
  cout << "Type 'R' to set line style to Am_Red" << endl;
  cout << "Type 'W' to set line style to Am_White" << endl;
  cout << "Type 'm' to set draw monochrome to true" << endl;
  cout << "Type 'c' to set draw monochrome to false" << endl;
  cout << "Type 'f' to read a nonexistent file" << endl;
  cout << "Type 't' to print the current image in RBG form first row first"
       << endl;
  cout << "Type 'T' to print bugsmall.gif in RBG form first row last" << endl;
  cout << "Type 'z' to print and display spiral bitmap in RBG form first row "
          "first"
       << endl;
  cout << "Type 'Z' to print spiral bitmap in RBG form first row last" << endl;
  cout << "Type 'q' to quit " << endl << flush;

  Am_Main_Event_Loop();
  Am_Cleanup();

  return 0;
} /* main */
