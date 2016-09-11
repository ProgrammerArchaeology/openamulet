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

Am_Object Am_Radio_Button;

/******************************************************************************
 * radion_button_draw
 *
 */

Am_Define_Method(Am_Draw_Method, void, radio_button_draw,
         ( Am_Object self, Am_Drawonable* drawonable,
           int x_offset, int y_offset) )
{
  int left = (int)self.Get( Am_LEFT ) + x_offset;
  int top = (int)self.Get( Am_TOP ) + y_offset;
  int width = self.Get( Am_WIDTH );
  int height = self.Get( Am_HEIGHT );
  int box_width = self.Get( Am_BOX_WIDTH );
  int box_height = self.Get( Am_BOX_HEIGHT );
  bool selected = self.Get( Am_SELECTED ).Valid ();
  bool interim_selected = self.Get( Am_INTERIM_SELECTED );
  bool active = self.Get( Am_ACTIVE );
  bool key_selected = self.Get( Am_KEY_SELECTED );
  bool want_final_selected = self.Get( Am_FINAL_FEEDBACK_WANTED );
  bool box_on_left = self.Get( Am_BOX_ON_LEFT );
  Am_Font font( self.Get( Am_FONT ) );
  Am_Alignment align = self.Get( Am_H_ALIGN );

  Computed_Colors_Record color_rec = self.Get( Am_STYLE_RECORD);
  Am_Widget_Look look = self.Get( Am_WIDGET_LOOK );

  // now find the contents to draw in the button
  Am_String string;
  Am_Object obj;
  Am_Value  value;

  // string slot contains a formula which gets the real object based on the
  // value of the COMMAND slot
  value=self.Peek(Am_REAL_STRING_OR_OBJ);
  if( value.type == Am_STRING )
    string = value;
  else if( value.type == Am_OBJECT )
    obj = value;
  else
    Am_ERRORO("Label of widget " << self
             << " should have string or object value but it is "
             << value, self, Am_REAL_STRING_OR_OBJ);
  // finally ready to draw it
  Am_Draw_Button_Widget( left, top, width, height, string, obj,
                         interim_selected, selected && want_final_selected,
                         active, key_selected, false, false, font,
			 color_rec, look, Am_RADIO_BUTTON, drawonable,
			 box_width, box_height, box_on_left, align, 5 );
}

Am_Define_Method(Am_Draw_Method, void, radio_button_mask,
         ( Am_Object self, Am_Drawonable* drawonable,
           int x_offset, int y_offset) )
{
  int left = (int)self.Get( Am_LEFT ) + x_offset;
  int top = (int)self.Get( Am_TOP ) + y_offset;
  int width = self.Get( Am_WIDTH );
  int height = self.Get( Am_HEIGHT );
  int box_width = self.Get( Am_BOX_WIDTH );
  int box_height = self.Get( Am_BOX_HEIGHT );
  bool selected = self.Get( Am_SELECTED );
  bool interim_selected = self.Get( Am_INTERIM_SELECTED );
  bool active = self.Get( Am_ACTIVE );
  bool key_selected = self.Get( Am_KEY_SELECTED );
  bool want_final_selected = self.Get( Am_FINAL_FEEDBACK_WANTED );
  bool box_on_left = self.Get( Am_BOX_ON_LEFT );
  Am_Font font( self.Get( Am_FONT ) );
  Am_Alignment align = self.Get( Am_H_ALIGN );

  Computed_Colors_Record color_rec = self.Get( Am_STYLE_RECORD);
  Am_Widget_Look look = self.Get( Am_WIDGET_LOOK );

  // now find the contents to draw in the button
  Am_String string;
  Am_Object obj;
  Am_Value  value;

  // string slot contains a formula which gets the real object based on the
  // value of the COMMAND slot
  value=self.Peek(Am_REAL_STRING_OR_OBJ);
  if( value.type == Am_STRING )
    string = value;
  else if( value.type == Am_OBJECT )
    obj = value;
  else
    Am_ERRORO("Label of widget " << self
             << " should have string or object value but it is "
             << value, self, Am_REAL_STRING_OR_OBJ);
  // finally ready to draw it
  Am_Draw_Button_Widget( left, top, width, height, string, obj,
                         interim_selected, selected && want_final_selected,
                         active, key_selected, false, false, font,
			 color_rec, look, Am_RADIO_BUTTON, drawonable,
			 box_width, box_height, box_on_left, align, 5, true );
}

/******************************************************************************
 * radio_button_diameter
 *   calculates the diameter of a radion button, which has a dependency upon
 *   the radio button's look.
 */

Am_Define_Formula (int, radio_button_diameter)
{
  Am_Widget_Look look = self.Get( Am_WIDGET_LOOK );
  int width = 0;

  switch( look.value ) {
    case Am_MOTIF_LOOK_val:
      width = 15;
      break;

    case Am_WINDOWS_LOOK_val:   // Win and Mac diameters are the same
    case Am_MACINTOSH_LOOK_val:
      width = 12;
      break;

    default:
      Am_Error ("Unknown Look parameter");
      break;
  }

  return width;
}

static void
init()
{
  Am_Object inter; // interactor in the widget
  Am_Object_Advanced obj_adv; // to get at advanced features like
                              // local-only and demons.

  //////////// Radio Button ////////////
  // Just another button with a different draw method.
  Am_Radio_Button = Am_Button.Create(DSTR("Radio_Button"))
    .Add (Am_BOX_ON_LEFT, true)
    .Add (Am_BOX_WIDTH, radio_button_diameter)
    .Add (Am_BOX_HEIGHT, radio_button_diameter)
    .Set (Am_INVALID_METHOD, radio_check_invalid)
    .Set (Am_TRANSLATE_COORDINATES_METHOD, radio_check_translate_coordinates)
    .Set (Am_POINT_IN_PART_METHOD, radio_check_point_in_part)
    .Set (Am_POINT_IN_LEAF_METHOD, radio_check_point_in_leaf)
    .Set (Am_ITEM_OFFSET, 3)
    .Set (Am_WIDTH, checkbox_width)
    .Set (Am_HEIGHT, checkbox_height)
    .Set (Am_H_ALIGN, Am_Align_From_Box_On_Left)
//    .Set (Am_EXTRA_BUTTON_BORDER, 0)
    .Set (Am_DRAW_METHOD, radio_button_draw)
	.Set (Am_MASK_METHOD, radio_button_mask)
    ;

  inter = Am_Radio_Button.Get(Am_INTERACTOR);
  inter.Set(Am_FIRST_ONE_ONLY, true);

  obj_adv = (Am_Object_Advanced&)Am_Radio_Button;

  obj_adv.Get_Slot (Am_BOX_ON_LEFT)
    .Set_Demon_Bits (Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  obj_adv.Get_Slot (Am_BOX_WIDTH)
    .Set_Demon_Bits (Am_MOVING_REDRAW | Am_EAGER_DEMON);
  obj_adv.Get_Slot (Am_BOX_HEIGHT)
    .Set_Demon_Bits (Am_MOVING_REDRAW | Am_EAGER_DEMON);
}

static void
cleanup()
{
	Am_Radio_Button.Destroy ();
}


static Am_Initializer *initializer = new Am_Initializer(DSTR("Am_Checkbox"), init, 5.202f, 118, cleanup);

