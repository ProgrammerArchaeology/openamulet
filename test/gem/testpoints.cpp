/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <am_inc.h>

#include <amulet/gem.h>
#include <stdlib.h>
#include <iostream>

#define TESTPOINTS_D_LEFT 800

#define THICKNESS 8

using namespace std;

Am_Style black(0.0, 0.0, 0.0, THICKNESS);
Am_Style white(1.0, 1.0, 1.0, THICKNESS);
//Am_Style red (1.0, 0.0, 0.0);
Am_Style red(1.0, 0.0, 0.0, 1, Am_CAP_BUTT, Am_JOIN_MITER, Am_LINE_SOLID,
             Am_DEFAULT_DASH_LIST, Am_DEFAULT_DASH_LIST_LENGTH, Am_FILL_SOLID,
             Am_FILL_POLY_WINDING);

Am_Style none;

/*
extern bool bad_point (Am_Point p1, Am_Point p2, Am_Point p3, Am_Style ls);

char *tf(bool n)
{
  return n ? "true" : "false";
}
*/

int
main()
{

  Am_Drawonable *root = Am_Drawonable::Get_Root_Drawonable();

  Am_Drawonable *d =
      root->Create(TESTPOINTS_D_LEFT, 40, 300, 800, "Am_Point_List Test");
  d->Flush_Output();

  ///
  ///  The POLYLINE
  ///
  // Since zig_zag_array is static, it persists across multiple invocations
  // of the function.  It will not be reinitialized when the function is
  // invoked a second time.
  static int zig_zag_array[14] = {60,  450, 20,  425, 75,  400, 150,
                                  450, 150, 400, 200, 400, 280, 450};
  Am_Point_List zig_zag_pl(zig_zag_array, 14);
  static int zig_zag2[12] = {50,  500, 50,  750, 140, 500,
                             140, 750, 200, 750, 270, 500};
  Am_Point_List zig_zag2_pl(zig_zag2, 12);

  // test calls to the new create methods
  Am_Point_List triangle_pl;
  Am_Point_List diamond_pl;

  //   triangle_pl.Add(50,300);
  //   triangle_pl.Add(150,200);
  //   triangle_pl.Add(150,300);
  //   triangle_pl.Add(170,200);
  triangle_pl.Add(50, 200);
  triangle_pl.Add(51, 300);
  triangle_pl.Add(52, 200);
  triangle_pl.Add(53, 300);

  diamond_pl.Add(235, 205);
  diamond_pl.Add(285, 255);
  diamond_pl.Add(235, 305);
  diamond_pl.Add(185, 255);
  diamond_pl.Add(235, 205);

  /*  static int star1[12] = {100, 0, 159, 181, 5, 69, 195, 69, 41, 181, 100, 0};*/
  static int star2[12] = {100, 0, 41, 181, 195, 69, 5, 69, 159, 181, 100, 0};
  Am_Point_List star_pl(star2, 12);

  int left, top, width, height;

  // draw all of the new triangles, with bounding boxes around them

  d->Draw_Lines(black, none, zig_zag2_pl);
  d->Get_Polygon_Bounding_Box(zig_zag2_pl, black, left, top, width, height);
  d->Draw_Rectangle(red, none, left, top, width, height, Am_DRAW_COPY);

  d->Draw_Lines(black, none, zig_zag_pl);
  d->Get_Polygon_Bounding_Box(zig_zag_pl, black, left, top, width, height);
  d->Draw_Rectangle(red, none, left, top, width, height, Am_DRAW_COPY);

  d->Draw_Lines(black, none, triangle_pl);
  d->Get_Polygon_Bounding_Box(triangle_pl, black, left, top, width, height);
  d->Draw_Rectangle(red, none, left, top, width, height, Am_DRAW_COPY);

  d->Draw_Lines(black, red, diamond_pl);
  d->Get_Polygon_Bounding_Box(diamond_pl, black, left, top, width, height);
  d->Draw_Rectangle(red, none, left, top, width, height, Am_DRAW_COPY);

  d->Draw_Lines(black, red, star_pl);
  d->Get_Polygon_Bounding_Box(star_pl, black, left, top, width, height);
  d->Draw_Rectangle(red, none, left, top, width, height, Am_DRAW_COPY);

  d->Flush_Output();
  char add;
  // enter the new x and y points and draw
  int x = 0, y = 0, index = 0, num_points = 0;

  cout << "star left: " << left << " top: " << top << " width: " << width
       << " height: " << height << endl;

  cout << "No clip regions set, so anything sticking out of the bounding\n"
       << "  boxes won't be cleared.";

  cout << "Hit return to move and resize star." << endl;
  getchar();

  d->Draw_Rectangle(white, white, left, top, width, height, Am_DRAW_COPY);
  star_pl.Scale(0.25, 0.25, left, top);
  star_pl.Translate(50, 50);

  d->Draw_Lines(black, red, star_pl);
  d->Get_Polygon_Bounding_Box(star_pl, black, left, top, width, height);
  d->Draw_Rectangle(red, none, left, top, width, height, Am_DRAW_COPY);

  d->Flush_Output();

  cout << "star left: " << left << " top: " << top << " width: " << width
       << " height: " << height << endl;

  Am_Point_List new_list;

  cout << "Type CTRL-D to exit.\n";

  while (true) {
    cout << "Please enter a 'd' to delete a point or a 'a' to add one: ";
    if (!(cin >> add))
      break;

    if (add == 'a') {
      cout << "Please enter coordinates: ";
      cin >> x >> y;
      if ((x < 0) || (x > 480)) {
        cout << "\nInvalid x coordinate\n";
        continue;
      }
      if ((y < 0) || (y > 800)) {
        cout << "\nInvalid y coordinate\n";
        continue;
      }

      d->Draw_Lines(white, white, new_list);
      d->Get_Polygon_Bounding_Box(new_list, black, left, top, width, height);
      d->Draw_Rectangle(white, none, left, top, width, height, Am_DRAW_COPY);
      new_list.Add(x, y);
      num_points++;
      if (num_points >= 2) {
        // the point being entered consititues the other half of a line
        d->Draw_Lines(black, none, new_list);
        d->Get_Polygon_Bounding_Box(new_list, black, left, top, width, height);
        d->Draw_Rectangle(red, none, left, top, width, height, Am_DRAW_COPY);
        d->Flush_Output();
      }
    } else {
      cout << "Please enter the index of the point to delete [0.."
           << new_list.Length() - 1 << "]: ";
      cin >> index;
      printf("index: %d\n", index);
      d->Draw_Lines(white, white, new_list);
      d->Get_Polygon_Bounding_Box(new_list, black, left, top, width, height);
      d->Draw_Rectangle(white, none, left, top, width, height, Am_DRAW_COPY);
      int i;
      for (new_list.Start(), i = 0; i < index; ++i)
        if (!new_list.Last())
          new_list.Next();
      if (new_list.Last())
        cout << "Point #" << index << " does not exist." << endl;
      else
        new_list.Delete();
      num_points--;
      d->Draw_Lines(black, none, new_list);
      d->Get_Polygon_Bounding_Box(new_list, black, left, top, width, height);
      d->Draw_Rectangle(red, none, left, top, width, height, Am_DRAW_COPY);
      d->Flush_Output();
      x = 1;
    }
  }
  cout << endl;

  return 0;
}
