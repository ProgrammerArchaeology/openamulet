/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <amulet.h>

Am_Slot_Key SCROLL_PART = Am_Register_Slot_Name ("SCROLL");

#define IMFN_FEDERATION "lib/images/ent.gif"

int main (void)
{
  static const char Am_DOTTED_DASH_LIST[2] = {2, 2};

  Am_Initialize ();

  Am_Image_Array ship_image (Am_Merge_Pathname(IMFN_FEDERATION));

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
        .Add_Part (SCROLL_PART, Am_Vertical_Scroll_Bar.Create ()
          .Set (Am_HEIGHT, 300)
          .Set (Am_VALUE_2, 99)
          .Set (Am_VALUE, 0)
        )
        .Add_Part (Am_Group.Create ()
          .Set (Am_WIDTH, 300)
          .Set (Am_HEIGHT, 300)
          .Add (Am_VALUE, Am_From_Sibling (SCROLL_PART, Am_VALUE))
          .Add_Part (Am_Group.Create ()
            .Set (Am_WIDTH, 300)
            .Set (Am_HEIGHT, 300)
            .Add_Part (Am_Rectangle.Create ()
              .Set (Am_LEFT, 100)
              .Set (Am_TOP, 100)
              .Set (Am_WIDTH, 100)
              .Set (Am_HEIGHT, 100)
              .Set (Am_FILL_STYLE, Am_Cyan)
            )
            .Add_Part (Am_Button.Create ()
              .Set (Am_LEFT, 150)
              .Set (Am_TOP, 175)
            )
          )
          .Add_Part (Am_Fade_Group.Create ()
            .Set (Am_WIDTH, 300)
            .Set (Am_HEIGHT, 300)
            .Set (Am_VALUE, Am_From_Owner (Am_VALUE))
            .Add_Part (Am_Group.Create ()
              .Set (Am_WIDTH, Am_Width_Of_Parts)
              .Set (Am_HEIGHT, Am_Height_Of_Parts)
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
            )
            .Add_Part (Am_Line.Create ()
              .Set (Am_X1, 300)
              .Set (Am_Y1, 0)
              .Set (Am_X2, 0)
              .Set (Am_Y2, 300)
              .Set (Am_LINE_STYLE, Am_Green)
            )
	    .Add_Part (Am_Arrow_Line.Create ()
	      .Set (Am_X1, 200)
	      .Set (Am_Y1, 0)
	      .Set (Am_X2, 0)
	      .Set (Am_Y2, 200)
	      .Set (Am_LINE_STYLE, Am_Purple)
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
//              .Set (Am_FILL_STYLE, Am_Yellow)
              .Set (Am_FILL_STYLE, Am_No_Style)
            )
            .Add_Part (Am_Arc.Create ()
              .Set (Am_LEFT, 200)
              .Set (Am_TOP, 150)
              .Set (Am_WIDTH, 100)
              .Set (Am_HEIGHT, 100)
              .Set (Am_FILL_STYLE, Am_Opaque_Diamond_Stipple)
            )
            .Add_Part (Am_Bitmap.Create ()
              .Set (Am_LEFT, 175)
              .Set (Am_TOP, 100)
              .Set (Am_IMAGE, ship_image)
              .Set (Am_LINE_STYLE, Am_Orange)
			  .Set (Am_DRAW_MONOCHROME, true)
            )
            .Add_Part (Am_Line.Create ()
              .Set (Am_X1, 50)
              .Set (Am_Y1, 150)
              .Set (Am_X2, 150)
              .Set (Am_Y2, 250)
              .Set (Am_LINE_STYLE, Am_Style (0.0f, 0.0f, 1.0f, 4, Am_CAP_BUTT,
					     Am_JOIN_MITER,
					     Am_LINE_ON_OFF_DASH,
					     Am_DOTTED_DASH_LIST, 2))
            )
	    .Add_Part (Am_Text.Create ()
	      .Set (Am_TEXT, "This is a test")
	      .Set (Am_LEFT, 180)
	      .Set (Am_TOP, 30)
	    )
	    .Add_Part (Am_Radio_Button_Panel.Create ()
	      .Set (Am_TOP, 110)
	      .Set (Am_LEFT, 20)
	      .Set (Am_ITEMS, Am_Value_List ()
	        .Add ("Radio")
	        .Add ("Buttons")
	      )
	    )
	    .Add_Part (Am_Checkbox_Panel.Create ()
	      .Set (Am_TOP, 150)
	      .Set (Am_LEFT, 20)
	      .Set (Am_ITEMS, Am_Value_List ()
	        .Add ("Check")
	        .Add ("Boxes")
	      )
	    )
	    .Add_Part (Am_Button.Create ()
	      .Set (Am_TOP, 200)
	      .Set (Am_LEFT, 20)
	    )
	    .Add_Part (Am_Button.Create ()
	      .Set (Am_TOP, 240)
	      .Set (Am_LEFT, 20)
              .Set (Am_LEAVE_ROOM_FOR_FRINGE, false)
	    )
	    .Add_Part (Am_Text_Input_Widget.Create ()
	      .Set (Am_TOP, 280)
	      .Set (Am_LEFT, 20)
	    )
// MISSING
//  menu_draw
//  menu_item_draw
//  scroll_arrow_draw
//  scroll_draw
//  scrolling_group_draw
//  selection_handles_draw
//  text_input_draw
//  border_rectangle_draw
          )
          .Add_Part (Am_Rectangle.Create ()
            .Set (Am_LEFT, 175)
            .Set (Am_TOP, 150)
            .Set (Am_WIDTH, 100)
            .Set (Am_HEIGHT, 100)
            .Set (Am_FILL_STYLE, Am_Purple)
          )
          .Add_Part (Am_Button.Create ()
            .Set (Am_LEFT, 10)
            .Set (Am_TOP, 50)
          )
        )
      )
    )
  ;

  Am_Main_Event_Loop ();
  Am_Cleanup ();

  return 0;
}
