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

Am_Drawonable *root, *d1, *d2, *d3, *d4;

Am_Style black(0.0, 0.0, 0.0);
Am_Style green(0.0, 1.0, 0.0);
Am_Style white(1.0, 1.0, 1.0);
Am_Point_List polygon1, polygon2, polygon3, polygon4;

void
init_windows()
{
  root = Am_Drawonable::Get_Root_Drawonable();
  d1 = root->Create(850, 30, 150, 100, "D1");
  d2 = root->Create(750, 170, 150, 100, "D2");
  d3 = root->Create(650, 300, 150, 100, "D3");
  d4 = root->Create(950, 170, 150, 100, "D4");
}

void
init_point_lists()
{

  static int diamond_array[10] = {75, 10, 10, 50, 75, 90, 140, 50, 75, 10};
  polygon1 = Am_Point_List(diamond_array, 10);

  polygon2 = Am_Point_List(polygon1);
  polygon3 = Am_Point_List(polygon2);
  polygon4 = Am_Point_List(polygon1);
}

void
change_left_point_lists()
{
  polygon2.Start();
  polygon2.Next();
  polygon2.Delete();
}

void
draw_point_lists(Am_Style style)
{
  d1->Draw_Lines(style, Am_No_Style, polygon1);
  d2->Draw_Lines(style, Am_No_Style, polygon2);
  d3->Draw_Lines(style, Am_No_Style, polygon3);
  d4->Draw_Lines(style, Am_No_Style, polygon4);
  d1->Flush_Output();
}

int
main()
{
  init_windows();
  init_point_lists();
  draw_point_lists(black);

  printf("Hit RETURN to apply change to point-list in D2: \n");
  getchar();

  draw_point_lists(white);
  change_left_point_lists();
  draw_point_lists(black);

  //
  // Exit
  //
  printf("Hit RETURN to exit: \n");
  getchar();

  return 0;
}
