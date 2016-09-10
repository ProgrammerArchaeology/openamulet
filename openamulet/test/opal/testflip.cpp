/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <amulet.h>

Am_Slot_Key SCROLL = Am_Register_Slot_Name ("SCROLL");

int main (void)
{
  Am_Initialize ();

  Am_Screen
    .Add_Part (Am_Window.Create ("main window")
      .Set (Am_WIDTH, Am_Width_Of_Parts)
      .Set (Am_HEIGHT, Am_Height_Of_Parts)
      .Set (Am_QUERY_POSITION, true)
      .Set (Am_FILL_STYLE, Am_Amulet_Purple)
      .Add_Part (Am_Group.Create ()
        .Set (Am_WIDTH, Am_Width_Of_Parts)
        .Set (Am_HEIGHT, Am_Height_Of_Parts)
        .Set (Am_LAYOUT, Am_Horizontal_Layout)
        .Set (Am_LEFT_OFFSET, 10)
        .Set (Am_TOP_OFFSET, 10)
        .Set (Am_H_SPACING, 10)
        .Set (Am_RIGHT_OFFSET, 10)
        .Set (Am_BOTTOM_OFFSET, 10)
        .Add_Part (SCROLL, Am_Vertical_Scroll_Bar.Create ()
          .Set (Am_HEIGHT, 300)
          .Set (Am_VALUE_2, 99)
          .Set (Am_VALUE, 0)
        )
        .Add_Part (Am_Flip_Book_Group.Create ()
          .Set (Am_WIDTH, 300)
          .Set (Am_HEIGHT, 300)
          .Set (Am_VALUE, Am_From_Sibling (SCROLL, Am_VALUE))
          .Add_Part (Am_Rectangle.Create ()
            .Set (Am_WIDTH, 100)
            .Set (Am_HEIGHT, 100)
            .Set (Am_FILL_STYLE, Am_Blue)
          )
          .Add_Part (Am_Roundtangle.Create ()
            .Set (Am_LEFT, 75)
            .Set (Am_TOP, 175)
            .Set (Am_WIDTH, 150)
            .Set (Am_HEIGHT, 100)
            .Set (Am_FILL_STYLE, Am_Red)
          )
          .Add_Part (Am_Line.Create ()
            .Set (Am_X1, 300)
            .Set (Am_Y1, 0)
            .Set (Am_X2, 0)
            .Set (Am_Y2, 300)
            .Set (Am_LINE_STYLE, Am_Green)
          )
          .Add_Part (Am_Polygon.Create ()
            .Set (Am_WIDTH, 100)
            .Set (Am_HEIGHT, 100)
            .Set (Am_POINT_LIST, Am_Point_List ()
              .Add (170, 50)
              .Add (200, 10)
              .Add (230, 50)
            )
            .Set (Am_LINE_STYLE, Am_Line_2)
            .Set (Am_FILL_STYLE, Am_Yellow)
          )
          .Add_Part (Am_Arc.Create ()
            .Set (Am_LEFT, 200)
            .Set (Am_TOP, 150)
            .Set (Am_WIDTH, 100)
            .Set (Am_HEIGHT, 100)
            .Set (Am_FILL_STYLE, Am_Opaque_Diamond_Stipple)
          )
        )
      )
    )
  ;

  Am_Main_Event_Loop ();
  Am_Cleanup ();

  return 0;
}
