#include <am_inc.h>

#include AM_IO__H
#include <amulet/impl/types_logging.h>
#include <amulet/impl/slots.h>
#include <amulet/impl/am_object.h>
#include <amulet/impl/am_style.h>
#include <amulet/impl/am_font.h>
#include <amulet/impl/am_alignment.h>
#include <amulet/impl/am_drawonable.h>
#include <amulet/impl/computed_colors_rec.h>
#include <amulet/impl/opal_objects.h>
#include <amulet/impl/gem_misc.h>
#include <amulet/impl/widget_draw.h>
#include <amulet/impl/method_draw.h>
#include "amulet/widgets_intnl.h"

#include <string.h>

Am_Style compute_text_style(bool active, bool depressed,
			    const Computed_Colors_Record& color_rec,
			    Am_Widget_Look look,
			    Am_Button_Type type) {
  bool black = true;;
  if ((type == Am_PUSH_BUTTON || type == Am_MENU_BUTTON) &&
      ((look == Am_MACINTOSH_LOOK && depressed) ||
       (look == Am_MOTIF_LOOK && !color_rec.data->light) ||
       (look == Am_WINDOWS_LOOK && !color_rec.data->light &&
	!(depressed && type == Am_MENU_BUTTON))))
    black = false;
  if(active) {
    if (black) return Am_Black;
    else return Am_White;
  }
  else {
    if (black) return Am_Motif_Inactive_Stipple;
    else return Am_Motif_White_Inactive_Stipple;
  }
}



/*******************************************************************************
 * Am_Draw_Button_Widget
 *   Draws a push button
 */

void
Am_Draw_Button_Widget(
  int left, int top, int width, int height,  // dimensions
  const char* string, Am_Object obj,
  bool interim_selected, bool selected,
  bool active, bool key_selected,
  bool is_default, bool fringe,
  Am_Font font,
  const Computed_Colors_Record& color_rec,
  Am_Widget_Look look,
  Am_Button_Type type,
  Am_Drawonable* draw,
  int box_width, int box_height, bool box_on_left,
  Am_Alignment align, int offset_left, bool mask)
{
  am_rect box_r, r( left, top, width, height );
  bool depressed = selected || interim_selected;

  switch( type )
  {
    case Am_PUSH_BUTTON:
    {
      switch( look.value )
      {
        case Am_MOTIF_LOOK_val:
          if( key_selected )
            draw->Draw_Rectangle( Am_Key_Border_Line, Am_No_Style,
                                  r.left, r.top, r.width, r.height );
          if( fringe || key_selected )
            Inset_Rect( r, kMotKeySel );

          if( is_default )
            Am_Draw_Motif_Box( r.left, r.top, r.width, r.height, true,
                               color_rec, draw );
          if( fringe || is_default )
            Inset_Rect( r, kMotDefault );
          break;

        case Am_WINDOWS_LOOK_val:
          if( is_default && !depressed )
          {
            draw->Draw_Rectangle( Am_Style( "black", 1 ), Am_No_Style,
                                  r.left, r.top, r.width, r.height );
            Inset_Rect( r, 1 );
          }
          break;

        case Am_MACINTOSH_LOOK_val: {
          static Am_Style black( "black", 3 );
          static Am_Style ltgray( "ltgray", 3 );

          if( is_default )
            draw->Draw_Roundtangle( active ? black : ltgray,
                                    Am_No_Style,
                                    r.left, r.top, r.width, r.height, 8, 8 );
          if( fringe || is_default )
            Inset_Rect( r, kMacDefault );
          break;
        }
        default:
          Am_Error ("Unknown Look parameter");
          break;
      }

      int inset = Am_Draw_Push_Button_Box( r, draw, look, depressed,
                                           key_selected, color_rec );
      Inset_Rect( r, inset );
      break;
    }

    case Am_CHECK_BUTTON:
      box_r.left   = box_on_left ? r.left : r.left + (r.width - box_width);
      box_r.top    = top + (r.height - box_height)/2;
      box_r.width  = box_width;
      box_r.height = box_height;
      Am_Draw_Check_Button_Box( box_r, draw, look, selected, interim_selected,
                                color_rec, mask );

      if( box_on_left )
        r.left  += box_width + offset_left;
      r.width -= box_width + offset_left;
      break;

    case Am_RADIO_BUTTON:
      box_r.left   = box_on_left ? r.left : r.left + (r.width - box_width);
      box_r.top    = top + (r.height - box_height)/2;
      box_r.width  = box_width;
      box_r.height = box_height;
      Am_Draw_Radio_Button_Box( box_r, draw, look, selected, interim_selected,
                                color_rec, mask );

      if( box_on_left )
        r.left += box_width + offset_left;
      r.width -= box_width + offset_left;
      break;
    default: Am_Error ("Switch statement is not complete");
      break;
  }

  draw->Push_Clip( r.left, r.top, r.width, r.height );

  if( depressed && look == Am_WINDOWS_LOOK && type == Am_PUSH_BUTTON )
  {
    r.left += 1;
    r.top  += 1;
  }

  am_rect text_or_obj_rect;

  if( string ) // draw the string if it exists
  {
    int   str_width, ascent, descent, a, b, str_left, str_top;

    Am_Style text_style = compute_text_style(active, depressed, color_rec,
					     look, type);

    draw->Get_String_Extents( font, string, strlen( string ), str_width,
                              ascent, descent, a, b);

    // center the text
    switch( align.value )
    {
      case Am_LEFT_ALIGN_val:
        str_left = r.left;// + offset_left;
        break;

      case Am_RIGHT_ALIGN_val:
        str_left = r.left + (r.width - str_width) - 2;
        break;

      case Am_CENTER_ALIGN_val:
      default:
        str_left = r.left + (r.width - str_width) / 2;
        break;
    }
    str_top = r.top + (r.height - ascent - descent) / 2;

    draw->Draw_Text( text_style, string, strlen( string ), font, str_left,
                     str_top, mask ? Am_DRAW_MASK_COPY : Am_DRAW_COPY );

    text_or_obj_rect.left   = str_left;
    text_or_obj_rect.top    = str_top;
    text_or_obj_rect.width  = str_width;
    text_or_obj_rect.height = ascent + descent;
  }
  else if( obj.Valid() ) // draw the object if no string and there is an object
  {
    // center the object in the button
    text_or_obj_rect.width  = obj.Get( Am_WIDTH );
    text_or_obj_rect.height = obj.Get( Am_HEIGHT );
    text_or_obj_rect.left   = (r.width  - text_or_obj_rect.width)  / 2;
    text_or_obj_rect.top    = (r.height - text_or_obj_rect.height) / 2;
    int obj_left = obj.Get (Am_LEFT);
    int obj_top = obj.Get (Am_TOP);

    bool line_changed = false;
    if( depressed && look == Am_MACINTOSH_LOOK )
    {
      if( obj.Peek( Am_LINE_STYLE ).Exists() )
      {
        if( obj.Get( Am_LINE_STYLE ) == Am_Black )
        {
          obj.Set( Am_LINE_STYLE, Am_White );
          line_changed = true;
        }
      }
    }

    Am_Draw( obj, draw, r.left + text_or_obj_rect.left - obj_left,
             r.top + text_or_obj_rect.top - obj_top );

    if( line_changed )
      obj.Set( Am_LINE_STYLE, Am_Black );
  }

  // draw key selected
  if( key_selected && look == Am_WINDOWS_LOOK )
  {
    Am_Style dash = Am_Style::Halftone_Stipple( 50 );
    if( type == Am_PUSH_BUTTON )
    {
      Inset_Rect( r, 2 );
      if( depressed )
      {
        r.width  -= 1;
        r.height -= 1;
      }
      draw->Draw_Rectangle( dash, Am_No_Style, r.left, r.top, r.width,
                            r.height );
      Inset_Rect( r, 1 );
    }
    else if( type == Am_RADIO_BUTTON || type == Am_CHECK_BUTTON )
    {
      r.top    = text_or_obj_rect.top    - 1;
      r.left   = text_or_obj_rect.left   - 2;
      r.width  = text_or_obj_rect.width  + 4;
      r.height = text_or_obj_rect.height + 2;
      draw->Draw_Rectangle( dash, Am_No_Style, r.left, r.top, r.width,
                            r.height, mask ? Am_DRAW_MASK_COPY : Am_DRAW_COPY );
    }
  }
  draw->Pop_Clip();
}

/******************************************************************************
 * Am_Draw_Push_Button_Box
 *   Draws a push button box for a particular look and feel
 */

int
Am_Draw_Push_Button_Box(
  am_rect r, // coords in terms of draw
  Am_Drawonable* draw,
  Am_Widget_Look look,
  bool selected, bool /* key_selected */,
  const Computed_Colors_Record& color_rec )
{
  draw->Push_Clip( r.left, r.top, r.width, r.height );
  int inset = 0;
  switch( look.value )
  {
    case Am_MOTIF_LOOK_val: // just use the every useful Am_Draw_Motif_Bo_valx
      Am_Draw_Motif_Box( r.left, r.top, r.width, r.height, selected,
                         color_rec, draw );
      inset = kMotBox;
      break;

    case Am_WINDOWS_LOOK_val:
    {
      Am_Draw_Windows_Box( r.left, r.top, r.width, r.height, selected,
                         color_rec, draw );
      inset = 2; // only inset for the box, not the internal key_selected stuff
      break;
    }

    case Am_MACINTOSH_LOOK_val:
    {
      Am_Style line_style = Am_Style( "black", 1 ); // black and 1 pixel thick
      Am_Style fill_style = !selected ? Am_White : Am_Black;
      draw->Draw_Roundtangle( line_style, fill_style, r.left, r.top, r.width,
                              r.height, 5, 5 );
      inset = kMacBox;
      break;
    }

    default:
      Am_Error ("Unknown Look parameter");
      break;
  } // switch( look )

  draw->Pop_Clip();
  return inset;
}


/******************************************************************************
 * Am_Draw_Check_Button_Box
 *   Draws a check button box for a particular look and feel
 */

void
Am_Draw_Check_Button_Box(
  am_rect r, // coords in terms of draw
  Am_Drawonable* draw,
  Am_Widget_Look look,
  bool selected, bool interim_selected,
  const Computed_Colors_Record& color_rec,
  bool mask = false )
{
  bool depressed = selected || interim_selected;
  draw->Push_Clip( r.left, r.top, r.width, r.height );

  switch( look.value ) {
    case Am_MOTIF_LOOK_val: // just use the every useful Am_Draw_Motif_Bo_valx
	  if (mask)
	    draw->Draw_Rectangle (Am_No_Style, Am_On_Bits, r.left, r.top,
		                      r.width, r.height);
	  else
        Am_Draw_Motif_Box( r.left, r.top, r.width, r.height, depressed,
                           color_rec, draw );
      break;
    case Am_WINDOWS_LOOK_val:
    {
      // draw the outside rectangle
      Am_Style upper_left, lower_right;
      upper_left  = color_rec.data->background_style;
      lower_right = color_rec.data->highlight_style;
	  if (mask)
	    draw->Draw_Rectangle (Am_On_Bits, Am_No_Style, r.left, r.top,
			                  r.width, r.height);
	  else
        Am_Draw_Rect_Border( r, upper_left, lower_right, draw );
      Inset_Rect( r, 1 );

      // draw the inside rectangle
      upper_left  = color_rec.data->shadow_style;
      lower_right = color_rec.data->foreground_style;
	  if (mask)
	    draw->Draw_Rectangle (Am_On_Bits, Am_No_Style, r.left, r.top,
			                  r.width, r.height);
	  else
        Am_Draw_Rect_Border( r, upper_left, lower_right, draw );
      Inset_Rect( r, 1 );

      // draw the fill
      Am_Style fill_style = interim_selected ? color_rec.data->foreground_style
                                             : Am_White;
      draw->Draw_Rectangle( Am_No_Style, fill_style, r.left, r.top, r.width,
                            r.height, mask ? Am_DRAW_MASK_COPY : Am_DRAW_COPY );

      const float ninth = (float)1/9;
      if( selected ) {
        Am_Style line_style( "black", 1 ); // black and 1 pixel
        float pix      = ninth * r.width,
              left_x   = 2*pix + r.left - 1 + 0.5f,
              center_x = 4*pix + r.left - 1 + 0.5f,
              right_x  = 8*pix + r.left - 1 + 0.5f,
              min_y    = 4*pix + r.top  - 1,
              max_y    = 6*pix + r.top  - 1,
              twopix   = 2*pix;
        for( float y = min_y; y <= max_y; y += 1 )
          draw->Draw_2_Lines( line_style, Am_No_Style,
           (int) left_x  ,
           (int) (y + 0.5),
           (int) center_x,
           (int) (y + twopix + 0.5),
           (int) right_x,
           (int) (y - twopix + 0.5),
		   mask ? Am_DRAW_MASK_COPY : Am_DRAW_COPY );
      }
      break;
    }

    case Am_MACINTOSH_LOOK_val:
    {
      Am_Style line_style( "black", interim_selected ? 2 : 1);
      Am_Style fill_style = Am_White;
      draw->Draw_Rectangle( line_style, fill_style, r.left, r.top, r.width,
                            r.height, mask ? Am_DRAW_MASK_COPY : Am_DRAW_COPY );
      if( selected ) { // draw the x mark
        line_style = Am_Style( "black", 1 );
        draw->Draw_Line( line_style, r.left, r.top, r.left + r.width,
                         r.top + r.height,
						 mask ? Am_DRAW_MASK_COPY : Am_DRAW_COPY );
        draw->Draw_Line( line_style, r.left, r.top + r.height - 1,
                         r.left + r.width - 1, r.top,
						 mask ? Am_DRAW_MASK_COPY : Am_DRAW_COPY );
      }
      break;
    }

    default:
      Am_Error ("Unknown Look parameter");
      break;
  } // switch( look )

  draw->Pop_Clip();
}

/******************************************************************************
 * Am_Draw_Radio_Button_Box
 *   Draws a radion button box for a particular look and feel
 */

void
Am_Draw_Radio_Button_Box(
  am_rect r, // coords in terms of draw
  Am_Drawonable* draw,
  Am_Widget_Look look,
  bool selected, bool interim_selected,
  const Computed_Colors_Record& color_rec,
  bool mask = false )
{
  bool depressed = selected || interim_selected;
  draw->Push_Clip( r.left, r.top, r.width, r.height );

  switch( look.value )
  {
    case Am_MOTIF_LOOK_val:
    {
      Am_Style top_fill    = depressed ? color_rec.data->shadow_style
                                       : color_rec.data->highlight_style;
      Am_Style bot_fill    = depressed ? color_rec.data->highlight_style
                                       : color_rec.data->shadow_style;
      Am_Style inside_fill = depressed ? color_rec.data->background_style
                                       : color_rec.data->foreground_style;

      int center_x = r.left + (r.width  + 1)/2 - 1;
      int center_y = r.top  + (r.height + 1)/2 - 1;
      int right    = r.left + r.width  - 1;
      int bottom   = r.top  + r.height - 1;

      // top edges
      draw->Draw_2_Lines( top_fill, Am_No_Style,
                          r.left, center_y,
                          center_x, r.top,
                          right, center_y,
						  mask ? Am_DRAW_MASK_COPY : Am_DRAW_COPY );
      draw->Draw_2_Lines( top_fill, inside_fill,
                          r.left+1, center_y,
                          center_x, r.top+1,
                          right-1, center_y,
						  mask ? Am_DRAW_MASK_COPY : Am_DRAW_COPY );

      // bottom edges
      draw->Draw_2_Lines( bot_fill, Am_No_Style,
                          r.left, center_y,
                          center_x, bottom,
                          right, center_y,
						  mask ? Am_DRAW_MASK_COPY : Am_DRAW_COPY );
      draw->Draw_2_Lines( bot_fill, inside_fill,
                          r.left+1, center_y,
                          center_x, bottom-1,
                          right-1, center_y,
						  mask ? Am_DRAW_MASK_COPY : Am_DRAW_COPY );
      break;
    }

    case Am_WINDOWS_LOOK_val:
    {
      // draw outer arcs
      draw->Draw_Arc( color_rec.data->background_style, Am_No_Style,
                      r.left, r.top, r.width, r.height, 45, 180,
					  mask ? Am_DRAW_MASK_COPY : Am_DRAW_COPY );
      draw->Draw_Arc( color_rec.data->highlight_style, Am_No_Style,
                      r.left, r.top, r.width, r.height, 225, 180,
					  mask ? Am_DRAW_MASK_COPY : Am_DRAW_COPY );
      Inset_Rect( r, 1 );

      // draw inner arcs - foreground_style if interim_selected, Am_White if
      // not
      Am_Style fill_style = interim_selected ?
                            color_rec.data->foreground_style: Am_White;
      draw->Draw_Arc( color_rec.data->shadow_style, fill_style,
                      r.left, r.top, r.width, r.height, 45, 180,
		      mask ? Am_DRAW_MASK_COPY : Am_DRAW_COPY );
      draw->Draw_Arc( color_rec.data->foreground_style, fill_style,
                      r.left, r.top, r.width, r.height, 225, 180,
		      mask ? Am_DRAW_MASK_COPY : Am_DRAW_COPY );

      // draw selected mark
      if( selected ) {
        Inset_Rect( r, 3 ); // 1 pixel for inner circle and 2 pixels of white
        draw->Draw_Arc( Am_No_Style, mask ? Am_On_Bits : Am_Black,
                        r.left, r.top, r.width, r.height );
      }
      break;
    }

    case Am_MACINTOSH_LOOK_val:
    {
      // draw the circle - 2 pixels if interim_selected, 1 if not
      Am_Style line_style( "black", interim_selected ? 2 : 1 ); // black
      draw->Draw_Arc( line_style, Am_White, r.left, r.top, r.width,
                      r.height, 0, 360, mask ? Am_DRAW_MASK_COPY : Am_DRAW_COPY );

      // draw selected mark
      if( selected ) {
        Inset_Rect( r, 3 );
        draw->Draw_Arc( Am_No_Style, mask ? Am_On_Bits : Am_Black,
                        r.left, r.top, r.width, r.height );
      }
      break;
    }

    default:
      Am_Error ("Unknown Look parameter");
      break;
  } // switch( type )

  draw->Pop_Clip();
}
