//
// ------------------------------------------------------------------------
// 		The OpenAmulet User Interface Development Environment
// ------------------------------------------------------------------------
// This code is based on the Amulet project of Carnegie Mellon University,
// and has been placed in the public domain.  If you are using this code
// or any part of OpenAmulet, please contact amulet@cs.cmu.edu to be put
// on the mailing list or have a look at:
// http://www.openip.org
//
#include <am_inc.h>

#include <amulet/impl/opal_obj2.h>
#include <amulet/impl/opal_constraints.h>

Am_Object Am_Menu;
Am_Object Am_Item_In_Menu = 0;


/******************************************************************************
 * MENUS
 *******************************************************************************/

/******************************************************************************
 * Menu items by design have the following characteristics

 ---------------------------------------- -
 |                                      |
 |                                      | top_border (above text ascent)
 |                                      | _
 |        About Amulet        ^A        | (ascent+descent)
 |                                      | -
 |                                      | bot_border (below text descent)
 |                                      |
 ---------------------------------------- -
 |        |           |      |  |       |
   left_               accel_     right_
   offset              offset     offset

 * The top_border, bot_border, left_offset, and right_offset in Motif include
 * a two pixel Motif selected box
 *
 * The text "About Amulet" could also be replaced by and object. The offsets
 * and border should remain the same
 *
 * Here are the default values and slot names
 *
 * Slot Name                       Motif     Win95     MacOS
 * Am_MENU_ITEM_TOP_OFFSET         5         3         2
 * Am_MENU_ITEM_BOT_OFFSET         5         5         4
 * Am_MENU_ITEM_LEFT_OFFSET        5         22        15
 * Am_MENU_ITEM_ACCEL_OFFSET       16        9         11
 * Am_MENU_ITEM_RIGHT_OFFSET       5         19        6
 *
 * note that the Motif values include the motif selected box
 *
 *
 * Menus have two special slots
 *
 * Slot Name
 * Am_MENU_BORDER                  2         3         1
 * Am_MENU_LINE_HEIGHT             2         9         16
 * Am_MENU_SELECTED_COLOR - only used with Win95 - defaults to Am_Yellow
 *
 */

/******************************************************************************
 * menu_draw
 */

Am_Define_Method( Am_Draw_Method, void, menu_draw,
         ( Am_Object menu, Am_Drawonable *drawonable,
           int x_offset, int y_offset) )
{
  int left = (int)menu.Get( Am_LEFT ) + x_offset;
  int top = (int)menu.Get( Am_TOP ) + y_offset;
  int width = menu.Get( Am_WIDTH );
  int height = menu.Get( Am_HEIGHT );
  Am_Widget_Look look = menu.Get( Am_WIDGET_LOOK );
  Computed_Colors_Record color_rec = menu.Get( Am_STYLE_RECORD );

  am_rect r( left, top, width, height );

  // first draw the border
  switch( look.value )
  {
    case Am_MOTIF_LOOK_val:
      Am_Draw_Motif_Box( left, top, width, height, false, color_rec,
                         drawonable );
      break;

    case Am_WINDOWS_LOOK_val:
    {
      Am_Draw_Windows_Box( left, top, width, height, false, color_rec,
                         drawonable );
      break;
    }
    case Am_MACINTOSH_LOOK_val:
      if( menu.Is_Instance_Of( Am_Menu_In_Scrolling_Menu ) )
	; //don't draw a background if in a scrolling menu
      else if( menu.Is_Instance_Of( Am_Menu_Bar ) ) // drawing menu bar
        drawonable->Draw_Rectangle( Am_Black, Am_White, r.left-1, r.top-1,
                                    r.width+2, r.height+1 );
      else // drawing a menu
      {
        drawonable->Draw_Rectangle( Am_Black, Am_No_Style, r.left+3, r.top+3,
                                    r.width-3, r.height-3 );
        drawonable->Draw_Rectangle( Am_Black, Am_White, r.left, r.top,
                                    r.width-1, r.height-1 );
      }
      break;
    default:
      Am_Error ("Unknown Look parameter");
      break;
  }

  // now draw the graphical parts of the aggregate, using Am_Aggregate's
  // draw method.
  Am_Draw_Method method;
  method = Am_Aggregate.Get( Am_DRAW_METHOD );
  method.Call( menu, drawonable, x_offset, y_offset );
}

/******************************************************************************
 * menu_mask
 */

Am_Define_Method(Am_Draw_Method, void, menu_mask,
         ( Am_Object menu, Am_Drawonable *drawonable,
           int x_offset, int y_offset) )
{
  int left = (int)menu.Get( Am_LEFT ) + x_offset;
  int top = (int)menu.Get( Am_TOP ) + y_offset;
  int width = menu.Get( Am_WIDTH );
  int height = menu.Get( Am_HEIGHT );
  drawonable->Draw_Rectangle( Am_No_Style, Am_On_Bits, left, top, width,
                              height );
}

//  The height is just the vertical extent of the menu's parts, plus border
//  width.

Am_Define_Formula( int, menu_height )
{
  // based on Am_map_height in opal.cc
  // the border depends upon the widget look;

  Am_Value_List components = self.Get (Am_GRAPHICAL_PARTS);
  int border = self.Get( Am_MENU_BORDER );
  Am_Widget_Look look = self.Get( Am_WIDGET_LOOK );

  int height = 0;
  Am_Object item;

  for (components.Start (); !components.Last (); components.Next ()) {
    item = components.Get( );
    if( (bool)item.Get (Am_VISIBLE) )
    {
      int item_top    = item.Get (Am_TOP);
      int item_height = item.Get (Am_HEIGHT);
      int item_bottom = item_top + item_height;
      if( item_bottom > height )
        height = item_bottom;
    }
  }

  if( look == Am_MACINTOSH_LOOK )
    border++;
  return height + border; // we are adding the botton border, Am_TOP_OFFSET takes
                          // care of the top border
}

Am_Define_Formula( int, menu_width )
{
  // based on Am_map_width in opal.cc
  // the border depends upon the widget look;

  Am_Value_List components = self.Get( Am_GRAPHICAL_PARTS );
  int border = self.Get( Am_MENU_BORDER );
  Am_Widget_Look look = self.Get( Am_WIDGET_LOOK );

  int width  = 0;
  Am_Object item;

  for( components.Start (); !components.Last (); components.Next() )
  {
    item = components.Get();
    if( (bool)item.Get( Am_VISIBLE ) )
    {
      int item_left  = item.Get( Am_LEFT );
      int item_width = item.Get( Am_WIDTH );
      int item_right = item_left + item_width;
      if( item_right > width )
        width = item_right;
    }
  }

  if( look == Am_MACINTOSH_LOOK )
    border++;
  return width + border; // we are adding the right border, Am_LEFT_OFFSET takes
                         // care of the left border
}

Am_Define_Formula( int, menu_line_height )
{
  Am_Widget_Look look = self.Get( Am_WIDGET_LOOK );
  switch( look.value )
  {
    case Am_MOTIF_LOOK_val:
      return 2;

    case Am_WINDOWS_LOOK_val:
      return 9;

    case Am_MACINTOSH_LOOK_val:
      return 16;

    default:
      Am_Error ("Unknown Look parameter");
      break;
  }
  return 2; // should never get here
}


/******************************************************************************
 * menu_border_size
 */

Am_Define_Formula( int, menu_border_size )
{
  Am_Widget_Look look = self.Get( Am_WIDGET_LOOK );

  switch( look.value )
  {
    case Am_MOTIF_LOOK_val:
      return 2;

    case Am_WINDOWS_LOOK_val:
      return 3;

    case Am_MACINTOSH_LOOK_val:
      return 1;

    default:
      Am_Error ("Unknown Look parameter");
      break;
  }
  return 0; // default
}



static void
init()
{
  ///////////////////////////////////////////////////////////////////////////
  // Menus
  ///////////////////////////////////////////////////////////////////////////

  // Based on button panel.

  Am_Menu = Am_Button_Panel.Create(DSTR("Menu"))
    .Set( Am_WIDTH, menu_width )
    .Set( Am_HEIGHT, menu_height )
    .Set( Am_HOW_SET, Am_CHOICE_SET )
    .Add( Am_MENU_BORDER, menu_border_size )
    .Set( Am_LEFT_OFFSET, Am_Same_As( Am_MENU_BORDER ) )
    .Set( Am_TOP_OFFSET, Am_Same_As( Am_MENU_BORDER ) )
    .Set( Am_V_SPACING, 0 )
    .Add( Am_MENU_LINE_HEIGHT, menu_line_height )
    .Set( Am_KEY_SELECTED, false )
    .Set( Am_FINAL_FEEDBACK_WANTED, false )
    .Add( Am_STYLE_RECORD, Am_Get_Computed_Colors_Record_Form )
    .Set( Am_DRAW_METHOD, menu_draw )
    .Set( Am_MASK_METHOD, menu_mask )
    ;

	Am_Item_In_Menu = Am_Menu_Item.Create (DSTR("Item_In_Menu_Proto"))
        .Add( Am_REAL_WIDTH, menu_item_width )
        .Add( Am_REAL_HEIGHT, menu_item_height )
        .Set( Am_WIDTH, panel_item_width )
        .Set( Am_HEIGHT, panel_item_height )
        .Set( Am_ACTIVE, active_from_command_panel )
        .Set( Am_SELECTED, false )
        .Set( Am_WIDGET_LOOK, look_from_owner )
        .Set( Am_FONT, Am_Font_From_Owner )
        .Set( Am_ITEM_OFFSET, Am_From_Owner( Am_ITEM_OFFSET ) )
        .Set( Am_FILL_STYLE, Am_From_Owner( Am_FILL_STYLE ) )
        .Set( Am_FINAL_FEEDBACK_WANTED, final_feedback_from_owner )
        .Set( Am_SET_COMMAND_OLD_OWNER, (0L) )
        .Set( Am_ACTIVE_2, Am_From_Owner(Am_ACTIVE_2))
		;

  Am_Menu.Set_Part( Am_ITEM_PROTOTYPE, Am_Item_In_Menu);

  // don't want the individual interactor from the button
  Am_Item_In_Menu.Remove_Part(Am_INTERACTOR);

  // when in a panel, the box's command object is gotten from the item slot
  // which is set automatically by the Map
  Am_Object command_obj = Am_Item_In_Menu.Get_Object(Am_COMMAND);
  Am_Item_In_Menu.Add(Am_ITEM, command_obj); // default value
  Am_Item_In_Menu.Add(Am_ITEM_TO_COMMAND, Am_Copy_Item_To_Command);
}

static void cleanup()
{
	Am_Menu.Destroy ();
}


static Am_Initializer *initializer = new Am_Initializer(DSTR("Am_Menu"), init, 5.207f, 113, cleanup);

