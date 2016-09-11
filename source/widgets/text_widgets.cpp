/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file contains the text input widgets

   Designed and implemented by Brad Myers
*/

#include <am_inc.h>

#include <amulet/impl/opal_obj2.h>
#include <amulet/impl/widget_op.h>
#include <amulet/impl/inter_text.h>
#include <amulet/impl/inter_undo.h>
#include <amulet/impl/opal_constraints.h>


#include TEXT_FNS__H        // for Am_Set_Pending_Delete
#include INITIALIZER__H

//generate a string from the value for the field if not a string
#include STR_STREAM__H
/******************************************************************************
 * Text Input Box
 *   Uses the label of the command as the label of the box, the value of
 *   the command as the initial (default) value
 *****************************************************************************/

/******************************************************************************
 * draw_motif_text_input
 */

static void draw_text_input(
  int left, int top, int width, int height,
  const char* string, Am_Object obj,
  Am_Object string_obj,
  bool /*active*/, bool key_selected,
  Am_Font label_font,
  Am_Widget_Look look,
  const Computed_Colors_Record& color_rec,
  Am_Drawonable* draw,
  bool mask = false )
{
  int box_left = 0; //where the text input area starts
  int box_width = width;

  // draw box showing that the keyboard is over this item
  if( look == Am_MOTIF_LOOK )
  {
    if( key_selected )
      draw->Draw_Rectangle( Am_Key_Border_Line, Am_No_Style, left, top,
                            width, height, mask ? Am_DRAW_MASK_COPY : Am_DRAW_COPY );

    left   += 2;
    top    += 2;
    width  -= 4;
    height -= 4;
  }

  // first, draw the label as a string or object:
  if( string )
  {
    int str_width, ascent, descent, a, b, str_top;
    draw->Get_String_Extents( label_font, string, strlen( string ), str_width,
                              ascent, descent, a, b );
    // text goes at left, centered vertically
    str_top = top + (height - ascent - descent) / 2;
    // set a clip region in case string bigger than the button
    draw->Push_Clip( left, top, width, height);
    draw->Draw_Text( Am_Black, string, strlen( string ), label_font,
                     left, str_top, mask ? Am_DRAW_MASK_COPY : Am_DRAW_COPY );
    draw->Pop_Clip();
    str_width += 2; // want to extra pixels on the right before the box
    box_left  = left + str_width;
    box_width = width - str_width;
  }
  else if( obj.Valid() )
  {
    // since a part of the button, will be offset from
    // buttons' left and top automatically.
    int obj_left = obj.Get( Am_LEFT );
    int obj_top = obj.Get( Am_TOP );
    int obj_width = obj.Get( Am_WIDTH );
    int obj_height = obj.Get( Am_HEIGHT );
    int x_offset = left - obj_left;
    int y_offset = top - obj_top + (height - obj_height) / 2;
    // call the object's draw method to draw the component
    Am_Draw( obj, draw, x_offset, y_offset );
    obj_width += 2; // want to extra pixels on the right before the box
    box_left  = left + obj_width;
    box_width = width - obj_width;
  }

  // draw the external box, leave 2 pixel border for the key-selected box
  // selected and interim selected look the same in Motif
  am_rect r( box_left, top, box_width, height );
  switch( look.value )
  {
    case Am_MOTIF_LOOK_val:
      if (mask)
        draw->Draw_Rectangle (Am_No_Style, Am_On_Bits, box_left, top, box_width,
                              height);
      else
        Am_Draw_Motif_Box( box_left, top, box_width, height, true, color_rec, draw );
      break;

    case Am_WINDOWS_LOOK_val:
    {
      // draw the outside rectangle
      Am_Style upper_left  = color_rec.data->background_style;
      Am_Style lower_right = color_rec.data->highlight_style;
      if (mask)
        draw->Draw_Rectangle (Am_On_Bits, Am_No_Style, box_left, top, box_width,
                              height);
      else
        Am_Draw_Rect_Border( r, upper_left, lower_right, draw );
      Inset_Rect( r, 1 );

      // draw the inside rectangle
      upper_left  = color_rec.data->shadow_style;
      lower_right = color_rec.data->foreground_style;
      if (mask)
        draw->Draw_Rectangle (Am_On_Bits, Am_No_Style, box_left, top, box_width,
                              height);
      else
        Am_Draw_Rect_Border( r, upper_left, lower_right, draw );
      Inset_Rect( r, 1 );

      // draw the fill
      draw->Draw_Rectangle( Am_No_Style, Am_White, r.left, r.top, r.width,
                            r.height, mask ? Am_DRAW_MASK_COPY : Am_DRAW_COPY );
      break;
    }

    case Am_MACINTOSH_LOOK_val:
      draw->Draw_Rectangle( Am_Black, Am_White, r.left, r.top, r.width,
                            r.height, mask ? Am_DRAW_MASK_COPY : Am_DRAW_COPY );
      break;

    default:
      Am_Error ("Unknown Look parameter");
      break;
  }

  // call the object's draw method to draw the component
  if( look == Am_MOTIF_LOOK )
  {
    left   -= 2;  // We have to reset these values b/c otherwise
    top    -= 2;  // string_obj is draw in the wrong place. The drawing
    width  += 4;  // routines require that the string object's coordinates
    height += 4;  // be relative to the input widget, so that the interactors
  }               // know where the object is.

  if (mask) {
    Am_Draw_Method method = string_obj.Get (Am_MASK_METHOD);
    if (method.Valid ())
      method.Call (string_obj, draw, left, top);
  }
  else
    Am_Draw( string_obj, draw, left, top );
}

/******************************************************************************
 * text_input_draw
 */

Am_Define_Method( Am_Draw_Method, void, text_input_draw,
                ( Am_Object self, Am_Drawonable* drawonable,
                  int x_offset, int y_offset ) )
{
  int left = (int)self.Get (Am_LEFT) + x_offset;
  int top = (int)self.Get (Am_TOP) + y_offset;
  int width = self.Get (Am_WIDTH);
  int height = self.Get (Am_HEIGHT);
  bool active = self.Get (Am_ACTIVE);
  bool key_selected = self.Get (Am_KEY_SELECTED);
  Am_Font label_font;
  label_font = self.Get (Am_LABEL_FONT);
  Computed_Colors_Record rec = self.Get (Am_STYLE_RECORD);
  Am_Widget_Look look = self.Get (Am_WIDGET_LOOK);

  // now find the contents to draw in the button
  Am_String string;
  Am_Object obj;
  Am_Value value;
  // string slot contains a formula which gets the real object based on the
  // value of the COMMAND slot
  value=self.Peek(Am_REAL_STRING_OR_OBJ);
  if (value.type == Am_STRING)
    string = value;
  else if (value.type == Am_OBJECT)
    obj = value;
  else Am_Error("String slot of widget should have string or object value");

  Am_Object string_obj = self.Get_Object(Am_TEXT_WIDGET_TEXT_OBJ);

  // finally ready to draw it
  draw_text_input( left, top, width, height, string,
                   obj, string_obj, active, key_selected,
                   label_font, look, rec, drawonable );
}

Am_Define_Method( Am_Draw_Method, void, text_input_mask,
                ( Am_Object self, Am_Drawonable* drawonable,
                  int x_offset, int y_offset ) )
{
  int left = (int)self.Get (Am_LEFT) + x_offset;
  int top = (int)self.Get (Am_TOP) + y_offset;
  int width = self.Get (Am_WIDTH);
  int height = self.Get (Am_HEIGHT);
  bool active = self.Get (Am_ACTIVE);
  bool key_selected = self.Get (Am_KEY_SELECTED);
  Am_Font label_font;
  label_font = self.Get (Am_LABEL_FONT);
  Computed_Colors_Record rec = self.Get (Am_STYLE_RECORD);
  Am_Widget_Look look = self.Get (Am_WIDGET_LOOK);

  // now find the contents to draw in the button
  Am_String string;
  Am_Object obj;
  Am_Value value;
  // string slot contains a formula which gets the real object based on the
  // value of the COMMAND slot
  value=self.Peek(Am_REAL_STRING_OR_OBJ);
  if (value.type == Am_STRING)
    string = value;
  else if (value.type == Am_OBJECT)
    obj = value;
  else Am_Error("String slot of widget should have string or object value");

  Am_Object string_obj = self.Get_Object(Am_TEXT_WIDGET_TEXT_OBJ);

  // finally ready to draw it
  draw_text_input( left, top, width, height, string,
                   obj, string_obj, active, key_selected,
                   label_font, look, rec, drawonable, true );
}

/******************************************************************************
 * get_text_input_height
 *   in the height slot of the text input widget
 */

Am_Define_Formula( int, get_text_input_height )
{
  int height = 8;
  Am_Object window;
  Am_Font label_font, font;
  window = self.Get( Am_WINDOW );
  if( !window.Valid() )
    return 10;
  Am_Drawonable* draw = GV_a_drawonable( window );
  if( !draw )
    return 10;

  Am_Widget_Look look = self.Get( Am_WIDGET_LOOK );
  int key_sel_border = (look == Am_MOTIF_LOOK) ? 4 : 0;

  //first, check the label's height
  Am_String string;
  Am_Object obj = 0;
  Am_Value value;

  label_font = self.Get( Am_FONT );

  // contains a formula which gets the real object based on the
  // value of the COMMAND slot
  value = self.Peek(Am_REAL_STRING_OR_OBJ);
  if( value.type == Am_STRING )
    string = value;
  else if( value.type == Am_OBJECT )
    obj = value;

  if( (const char*)string )
  {
    font = self.Get( Am_FONT );
    int str_width, ascent, descent, a, b, height;
    draw->Get_String_Extents( font, string, strlen( string ), str_width,
                              ascent, descent, a, b );
    height = ascent + descent + key_sel_border;
  }
  else if( obj.Valid() )
    height = (int)obj.Get( Am_HEIGHT ) + key_sel_border;

  // now see if text input area is bigger
  Am_Object text = self.Get_Object( Am_TEXT_WIDGET_TEXT_OBJ );
  if( text.Valid() )
  {                                          // + 6 for border
    int text_height = (int)text.Get( Am_HEIGHT ) + 6 + key_sel_border;
    if( text_height > height )
      height = text_height;
  }
  return height;
}

/******************************************************************************
 * text_input_in
 *   Am_Where_Function:  For the where test of interactor.
 *   Return the Am_Text text_value part if click anywhere in the box region
 */

Am_Define_Method( Am_Where_Method, Am_Object, text_input_in,
                ( Am_Object /* inter */,  Am_Object object,
                  Am_Object event_window, int x, int y ) )
{
  //object is the text input widget
  if( Am_Point_In_All_Owners( object, x, y, event_window ) &&
      ( Am_Point_In_Obj( object, x, y, event_window ).Valid() ) )
  {
    //make sure is inside the box area
    int box_left = object.Get( Am_TEXT_WIDGET_BOX_LEFT );
    int outx, outy;
    // get the coordinates with respect to the widget
    Am_Translate_Coordinates( event_window, x, y, object, outx, outy );
    if( outx > box_left )
    {
      Am_Object text = object.Get_Object( Am_TEXT_WIDGET_TEXT_OBJ );
      return text;
    }
  }
  return Am_No_Object;
}

/******************************************************************************
 * text_input_command_inter_do
 */

Am_Define_Method( Am_Object_Method, void, text_input_command_inter_do,
                  (Am_Object inter_command) )
{
  // put a copy of the finally edited value into Am_VALUE slot of
  // command obj of the widget.  Widget value itself is set by the constraint
  Am_Object text = inter_command.Get( Am_OBJECT_MODIFIED );
  Am_Object widget = text.Get_Owner();
  //widget's value is set by a constraint or by a Am_Text_Check_Legal_Method
  Am_Value final_value = widget.Get(Am_VALUE);
  Am_Object widget_command = widget.Get_Object(Am_COMMAND);
  if( widget_command.Valid() )
  {
    if (!widget_command.Is_Instance_Of(Am_Command))
      Am_ERRORO("Am_COMMAND slot of widget " << widget
		<< " must contain an Am_Command object, but it contains"
		<< widget_command, widget, Am_COMMAND);
    // set up for undo
    Am_Value old_value = widget_command.Get(Am_VALUE);
    widget_command.Set(Am_OLD_VALUE, old_value);
    //set the new value
    widget_command.Set(Am_VALUE, final_value);
  }
  else
   std::cout << "**No widget for " << inter_command << " in " << widget <<std::endl;
}

/******************************************************************************
 * get_text_part_value
 */

static Am_String val ("");

Am_Define_String_Formula( get_text_part_value )
{
  Am_Object part = self.Get_Object( Am_TEXT_WIDGET_TEXT_OBJ );
  if (part.Valid ())
    return part.Get( Am_TEXT );
  else
    return val;
}

/******************************************************************************
 * get_text_widget_value
 *   put into the text slot of the text object that is edited.  This
 *   sets up a circular constraint so setting the Value of the widget
 *   will affect the current string.
 */

Am_Define_String_Formula( get_text_widget_value ) {
  static Am_String val ("");

  Am_Object owner = self.Get_Owner();
  if (owner.Valid ()) {
    Am_Value v = owner.Get( Am_VALUE );
    if (v.type != Am_STRING) { //generate string from the value by printing it
      char line[100];
      OSTRSTREAM_CONSTR (oss,line, 100, std::ios::out);
      oss << v <<std::ends;
      OSTRSTREAM_COPY(oss,line,100);
      v = line; //now use the string as the value
    }
    return v;
  }
  else
    return val;
}

/******************************************************************************
 * text_widget_box_left
 *   self is the Am_Text_Input_Widget
 */

Am_Define_Formula( int, text_widget_box_left )
{
  Am_String string;
  Am_Object obj, window;
  Am_Value value;

  // string slot contains a formula which gets the real object based on the
  // value of the COMMAND slot
  window = self.Get( Am_WINDOW );
  value = self.Peek(Am_REAL_STRING_OR_OBJ);
  if( value.type == Am_STRING )
    string = value;
  else if( value.type == Am_OBJECT )
    obj = value;
  Am_Drawonable* draw = (0L);

  if( window.Valid() )
    draw = GV_a_drawonable( window );
  if( !draw )
    return 10;

  Am_Widget_Look look = self.Get( Am_WIDGET_LOOK );
  int key_sel_pixs = (look == Am_MOTIF_LOOK ) ? 2 : 0;
  int width;

  if( string.Valid() )
  {
    Am_Font label_font;
    label_font = self.Get( Am_LABEL_FONT, Am_NO_DEPENDENCY );
    int ascent, descent, a, b;
    draw->Get_String_Extents( label_font, string, strlen( string ), width,
                              ascent, descent, a, b );
  }
  else if( obj.Valid() )
    width = obj.Get( Am_WIDTH );
  else
    width = 0;
  // text goes at left, centered vertically
  return key_sel_pixs + width + 2;
}

/******************************************************************************
 * text_widget_value_left
 */

Am_Define_Formula( int, text_widget_value_left )
{
  return (int)(self.Get_Owner().Get( Am_TEXT_WIDGET_BOX_LEFT ) ) + 3;
}

/******************************************************************************
 * text_widget_value_width
 */

Am_Define_Formula( int, text_widget_value_width )
{
  Am_Object owner = self.Get_Owner();
  if( owner.Valid() )
  {
    int box_left = owner.Get( Am_TEXT_WIDGET_BOX_LEFT );
    int owner_width = owner.Get( Am_WIDTH );
    Am_Widget_Look look = owner.Get( Am_WIDGET_LOOK );
    if( look == Am_MOTIF_LOOK )
      owner_width -= 2;
    return owner_width - box_left - 6; // 3 on left and 3 on right
  }
  else
    return 100;
}

/******************************************************************************
 * text_widget_value_top
 */

Am_Define_Formula( int, text_widget_value_top )
{
  int ret = 0;
  Am_Object owner = self.Get_Owner();
  if( owner.Valid() )
  {
    int owner_height = owner.Get( Am_HEIGHT );
    int my_height = self.Get( Am_HEIGHT );
    ret = ( (owner_height - my_height) / 2 );
  }
  return ret;
}

/******************************************************************************
 * text_widget_value_style
 */

Am_Define_Style_Formula( text_widget_value_style )
{
  Am_Object owner = self.Get_Owner();
  if( owner.Valid() ) {
    bool black = true;
    Am_Widget_Look look = owner.Get (Am_WIDGET_LOOK, Am_NO_DEPENDENCY);
    if (look == Am_MOTIF_LOOK) {
      Computed_Colors_Record rec = owner.Get (Am_STYLE_RECORD, Am_NO_DEPENDENCY);
      if (!rec.data->light) black = false;
      }
    if( (bool)owner.Get( Am_ACTIVE) ) {
      if (black) return Am_Black;
      else return Am_White;
      }
    else {
      if (black) return Am_Motif_Inactive_Stipple;
      else return Am_Motif_White_Inactive_Stipple;
    }
  }
  else
    return Am_Black;
}

/******************************************************************************
 * text_widget_start_method
 *   need special method because must pass the text_obj to start_interactor
 */

Am_Define_Method( Am_Explicit_Widget_Run_Method, void, text_widget_start_method,
                ( Am_Object widget, Am_Value initial_value ) )
{
  if( initial_value.Valid() )
    widget.Set( Am_VALUE, initial_value );
  Am_Object inter = widget.Get_Object( Am_INTERACTOR );
  Am_Object text_obj = widget.Get_Object( Am_TEXT_WIDGET_TEXT_OBJ );
  Am_Start_Interactor( inter, text_obj );
  //need to set pending delete after starting the interactor, since
  //starting the interactor moves the cursor which turns pending
  //delete off.
  if( (bool)widget.Get( Am_WANT_PENDING_DELETE ) )
    Am_Set_Pending_Delete( text_obj, true );  //start off with pending delete
}

///////////////////////////////////////////////////////////////////////////
//  Am_Number_Input_Widget
///////////////////////////////////////////////////////////////////////////

bool icheck_max_min(Am_Value &result, long mini, long maxi, bool want_db,
		    Am_Object &widget) {
  if (result.type != Am_LONG) {
    if (want_db) {
      Am_POP_UP_ERROR_WINDOW("Value must be an integer (no '.' or 'e').");
    }
    else Am_Beep(widget.Get(Am_WINDOW));
    return false;
  }
  long result_int = result;
  if (result_int < mini || result_int > maxi) {
    if (want_db) {
      Am_POP_UP_ERROR_WINDOW("Value must be between " << mini << " and "
			     << maxi);
    }
    else Am_Beep(widget.Get(Am_WINDOW));
    return false;
  }
  return true;
}
bool fcheck_max_min(Am_Value &result, float minf, float maxf, bool want_db,
		    Am_Object &widget) {
  float result_float = result;
  if (result_float < minf || result_float > maxf) {
    if (want_db) {
      Am_POP_UP_ERROR_WINDOW("Value must be between " << minf << " and "
			     << maxf);
    }
    else Am_Beep(widget.Get(Am_WINDOW));
    return false;
  }
  return true;
}
bool check_max_min(Am_Value result, Am_Object &widget, bool want_db) {
  Am_Value v1 = widget.Get(Am_VALUE_1);
  Am_Value v2 = widget.Get(Am_VALUE_2);
  if (!(v1.type == v2.type && (v1.type == Am_FLOAT || v1.type == Am_LONG ||
			       v1.type == Am_DOUBLE || v1.type == Am_INT ||
			       v1.type == Am_NONE)))
    Am_ERRORO("Am_VALUE_1 and Am_VALUE_2 of " << widget
	      << " must be int, long, float, double, or Am_No_Value, "
	      " and the same type, but they are "
	      << v1 << " and " << v2, widget, Am_VALUE_1);
  if (v1.type == Am_LONG || v1.type == Am_INT)
    return icheck_max_min(result, v1, v2, want_db, widget);
  else if (v1.type == Am_FLOAT || v1.type == Am_DOUBLE)
    return fcheck_max_min(result, v1, v2, want_db, widget);
  return true;
}

//Returns true if parse is OK, and value is put into output_value;
bool Am_String_To_Long_Or_Float(Am_String string, Am_Value& output_value) {
  char* s = string;
  char* hasfloatchar = strchr(s, '.');
  if (!hasfloatchar)
    hasfloatchar = strchr(s, 'E');
  if (!hasfloatchar)
    hasfloatchar = strchr(s, 'e');
  int len = strlen(s);
  char *ptr;
  if (hasfloatchar) {
    double d = strtod(s, &ptr);
    output_value = (float)d;
  }
  else {
    long i = strtol(s, &ptr, 10);
    output_value = i;
  }
  if (ptr < s+len) return false;
  else return true;
}

Am_Define_Method(Am_Text_Check_Legal_Method, Am_Text_Abort_Or_Stop_Code,
                  Am_Number_Input_Filter_Method,
                  ( Am_Object &text, Am_Object& inter) ) {
  Am_String str = text.Get(Am_TEXT);
  Am_Object widget = inter.Get_Owner();
  Am_Value result;

  bool ok = Am_String_To_Long_Or_Float(str, result);
  bool want_db = (bool)widget.Get(Am_USE_ERROR_DIALOG);
  if (!ok) {
    if (want_db)
      Am_POP_UP_ERROR_WINDOW("Value must be a number.")
    else Am_Beep(widget.Get(Am_WINDOW));
    return Am_TEXT_ABORT_AND_RESTORE;
  }
  else {
    if (check_max_min(result, widget, want_db)) {
      widget.Set(Am_VALUE, result);
      char* s = str;
      int len = strlen(s);
      if (len == 0) {
	//needed since otherwise, constraint doesn't necessarily run and the
	//field can end up with an empty string if previous result was also 0
	widget.Note_Changed(Am_VALUE);
      }
      return Am_TEXT_OK;
    }
    else return Am_TEXT_ABORT_AND_RESTORE;
  }
}

///////////////////////////////////////////////////////////////////////////
// Moving to next widget
///////////////////////////////////////////////////////////////////////////

/******************************************************************************
 * Tabbing from widget to widget
 *   need special method because must pass the text_obj to start_interactor
 *****************************************************************************/

/******************************************************************************
 * generate_list_of_text_widgets
 *   default formula for the Am_LIST_OF_TEXT_WIDGETS slot of the interactor
 */

Am_Define_Value_List_Formula( generate_list_of_text_widgets )
{
  Am_Object group = self.Get_Owner();
  Am_Value_List all_text_widgets;
  if( group.Valid() )
  {
    Am_Value v;
    v = group.Peek(Am_GRAPHICAL_PARTS);
    if( v.Valid() )
    {
      Am_Value_List all_parts = v;
      Am_Object part;
      for( all_parts.Start(); !all_parts.Last(); all_parts.Next())
      {
        part = all_parts.Get();
        if( part.Is_Instance_Of( Am_Text_Input_Widget ) )
          all_text_widgets.Add( part );
      }
    }
  }
  return all_text_widgets;
}

/******************************************************************************
 * get_next_from
 */

Am_Object
get_next_from(
  Am_Value_List &all_text_widgets,
  bool forward )
{
  if( forward )
  {
    all_text_widgets.Next();
    if( all_text_widgets.Last() )
      all_text_widgets.Start(); //wrap around
  }
  else
  {
    all_text_widgets.Prev();
    if( all_text_widgets.First() )
      all_text_widgets.End(); //wrap around
  }
  return all_text_widgets.Get();
}

/******************************************************************************
 * find_next_active_widget
 */

Am_Object
find_next_active_widget(
  Am_Object &next_widget,
  Am_Value_List &all_text_widgets )
{
  if( (bool)next_widget.Get( Am_ACTIVE ) )
    return next_widget;
  else
  {
    Am_Object obj;
    all_text_widgets.Start();
    if( !all_text_widgets.Member( next_widget ) )
      Am_Error("Next widget not in list"); //shouldn't happen
    while( true )
    {
      all_text_widgets.Next();
      if( all_text_widgets.Last() )
        all_text_widgets.Start();
      obj = all_text_widgets.Get();
      if( obj == next_widget ) // went all the way around
         return Am_No_Object;
      else if( (bool)obj.Get( Am_ACTIVE ) )
         return obj;
      //else continue looping
    }
  }
}

/******************************************************************************
 * check_value_legal
 */

bool
check_value_legal(
  Am_Object &text,
  Am_Object &inter )
{
  Am_Text_Abort_Or_Stop_Code code = am_check_text_legal( inter, text );
  if( code == Am_TEXT_OK || code == Am_TEXT_STOP_ANYWAY )
    return true;
  else if( code == Am_TEXT_ABORT_AND_RESTORE )
    Am_Abort_Interactor( inter );

  return false;
}

/******************************************************************************
 * tab_to_next_widget
 */

Am_Define_Method( Am_Object_Method, void, tab_to_next_widget, (Am_Object cmd) )
{
  Am_Object inter = cmd.Get_Owner();
  if( inter.Valid() )
  {
    Am_Value_List all_text_widgets = inter.Get( Am_LIST_OF_TEXT_WIDGETS );
    Am_Object last_widget, cur_widget, next_widget;
    bool forward;
    bool running = false;
    Am_Value v;
    if( all_text_widgets.Empty() )
      return; //nothing to do
    v=cmd.Peek(Am_LAST_TEXT_WIDGET);
    if( v.Valid() )
      last_widget = v;
    // see if going forward or backwards
    inter = cmd.Get_Owner();
    Am_Input_Char ic = inter.Get( Am_START_CHAR );
    //go forward if not shifted
    forward = !ic.shift;
    // search to see which part is active, if any
    for( all_text_widgets.Start(); !all_text_widgets.Last();
         all_text_widgets.Next() )
    {
      cur_widget = all_text_widgets.Get();
      inter = cur_widget.Get_Object( Am_INTERACTOR );
      v=inter.Peek(Am_CURRENT_STATE);
      if( v.Valid() && (int)v == 1 )
      { //then this is it
        running = true;
        next_widget = get_next_from( all_text_widgets, forward );
        break;
      }
    }
    if( !next_widget.Valid() )
    {
      //nothing running, see if have current
      if( last_widget.Valid() )
      {
        all_text_widgets.Start();
        if( all_text_widgets.Member( last_widget ) )
        {
          next_widget = get_next_from( all_text_widgets, forward );
        }
      }
      if( !next_widget.Valid() ) // just use first in the list
      {
        all_text_widgets.Start();
        next_widget = all_text_widgets.Get();
      }
    }
    if( running )
    {
      //check to see whether current value is legal and if not, don't go on
      Am_Object cur_inter = cur_widget.Get_Object( Am_INTERACTOR );
      Am_Object cur_text = cur_widget.Get_Object( Am_TEXT_WIDGET_TEXT_OBJ );
      if( !check_value_legal( cur_text, cur_inter ) )
        return;
      //check if old value same as current, in which case, abort so
      //command not queued for undo
      Am_String old_cur_value = cur_inter.Get( Am_OLD_VALUE );
      Am_String cur_cur_value = cur_text.Get( Am_TEXT );
      if( old_cur_value == cur_cur_value )
        Am_Abort_Interactor( inter );
      else
        Am_Stop_Widget( cur_widget );

      cur_widget.Set( Am_KEY_SELECTED, false );
    }
    if( last_widget.Valid() )
      last_widget.Set( Am_KEY_SELECTED, false );

    if( next_widget.Valid() )
    {
      next_widget = find_next_active_widget( next_widget, all_text_widgets );
      if( next_widget.Valid() )
      {
        cmd.Set( Am_LAST_TEXT_WIDGET, next_widget, Am_OK_IF_NOT_THERE );
        next_widget.Set( Am_KEY_SELECTED, true );
        Am_Start_Widget( next_widget );
      }
    }
  }
}

  ///////////////////////////////////////////////////////////////////////////
  // Initialization
  ///////////////////////////////////////////////////////////////////////////

// exported objects

Am_Object Am_Text_Input_Widget;
Am_Object Am_Password_Input_Widget;
Am_Object Am_Number_Input_Widget;

Am_Object Am_Tab_To_Next_Widget_Command;
Am_Object Am_Tab_To_Next_Widget_Interactor;

static void init()
{
  Am_Object inter; // interactor in the widget
  Am_Object command_obj;
  Am_Object_Advanced obj_adv; // to get at advanced features like
             // local-only and demons.

  //////////// Command Object /////////////

  ///////////////////////////////////////////////////////////////////////////
  // Text Input
  ///////////////////////////////////////////////////////////////////////////

  Am_Font bold_font(Am_FONT_FIXED, true);

  Am_Text_Input_Widget = Am_Widget_Aggregate.Create(DSTR("Text_Input_Widget"))
    .Add (Am_VALUE, "") //empty string is initial value
    .Set (Am_VALUE, get_text_part_value.Multi_Constraint()) //in case UNDO
    .Add (Am_ACTIVE, Am_Active_From_Command)
    .Add (Am_ACTIVE_2, true) // used by interactive tools
    .Add (Am_KEY_SELECTED, false)
    .Add (Am_FONT, Am_Default_Font)
    .Add (Am_TEXT_CHECK_LEGAL_METHOD, (0L)) // no checking by default
    .Add (Am_LABEL_FONT, bold_font)
    .Add (Am_FILL_STYLE, Am_Default_Color)
    .Add (Am_STYLE_RECORD, Am_Get_Computed_Colors_Record_Form)
    .Set (Am_DRAW_METHOD, text_input_draw)
    .Set (Am_MASK_METHOD, text_input_mask)
    .Set (Am_WIDTH, 150)
    .Set (Am_HEIGHT, get_text_input_height)
    .Add (Am_WANT_PENDING_DELETE, true)
    .Add (Am_TEXT_WIDGET_BOX_LEFT, text_widget_box_left)
    .Add (Am_SET_COMMAND_OLD_OWNER, Am_Set_Old_Owner_To_Me)
    .Add (Am_WIDGET_START_METHOD, text_widget_start_method)
    .Add (Am_WIDGET_ABORT_METHOD, Am_Standard_Widget_Abort_Method)
    .Add (Am_WIDGET_STOP_METHOD, Am_Standard_Widget_Stop_Method)
    .Add_Part (Am_COMMAND, Am_Command.Create(DSTR("Text_Input_Command"))
         .Set (Am_LABEL, "Text_Input")
         .Set (Am_VALUE, ""))
    .Add_Part (Am_TEXT_WIDGET_TEXT_OBJ,
         Am_Text.Create(DSTR("Value Text in Text_Widget"))
         .Set(Am_LEFT, text_widget_value_left)
         .Set(Am_TOP, text_widget_value_top)
         .Set(Am_WIDTH, text_widget_value_width)
         .Set(Am_FONT, Am_Font_From_Owner)
         .Set(Am_LINE_STYLE, text_widget_value_style)
         .Set(Am_TEXT, get_text_widget_value.Multi_Constraint())
         )
    .Add_Part (Am_INTERACTOR,
     inter = Am_Text_Edit_Interactor.Create(DSTR("inter_in_text_input"))
          .Set (Am_START_WHEN, Am_Default_Widget_Start_Char)
	       .Set (Am_START_WHERE_TEST, text_input_in)
	       .Set (Am_WANT_PENDING_DELETE,
		     Am_From_Owner(Am_WANT_PENDING_DELETE))
	       .Set (Am_ACTIVE, Am_Active_And_Active2)
	       .Set (Am_TEXT_CHECK_LEGAL_METHOD,
		     Am_From_Owner(Am_TEXT_CHECK_LEGAL_METHOD))
    )
    .Add (Am_REAL_STRING_OR_OBJ, Am_Get_Real_String_Or_Obj)
    ;
  inter.Get_Object(Am_COMMAND)
    .Set(Am_IMPLEMENTATION_PARENT, Am_Get_Owners_Command)
    .Set(Am_DO_METHOD, text_input_command_inter_do)
    .Set_Name(DSTR("Command_In_Text_Input_Widget"))
    ;

  obj_adv = (Am_Object_Advanced&)Am_Text_Input_Widget;

  obj_adv.Get_Slot (Am_REAL_STRING_OR_OBJ)
    .Set_Demon_Bits (Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  obj_adv.Get_Slot (Am_KEY_SELECTED)
    .Set_Demon_Bits (Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  obj_adv.Get_Slot (Am_ACTIVE)
    .Set_Demon_Bits (Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  obj_adv.Get_Slot (Am_WIDGET_LOOK)
    .Set_Demon_Bits (Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  obj_adv.Get_Slot (Am_FONT)
    .Set_Demon_Bits (Am_STATIONARY_REDRAW | Am_EAGER_DEMON);
  obj_adv.Get_Slot (Am_FILL_STYLE)
    .Set_Demon_Bits (Am_STATIONARY_REDRAW | Am_EAGER_DEMON);

  // all the next slots should not be inherited
  obj_adv.Get_Slot(Am_KEY_SELECTED).Set_Inherit_Rule(Am_COPY);
  obj_adv.Get_Slot(Am_ACTIVE).Set_Inherit_Rule(Am_COPY);
  obj_adv.Get_Slot(Am_ACTIVE_2).Set_Inherit_Rule(Am_COPY);

  Am_Text_Input_Widget.Set (Am_VALUE, ""); //empty string is initial value

  ///////////////////////////////////////////////////////////////////////////
  //  Am_Password_Input_Widget
  ///////////////////////////////////////////////////////////////////////////


  Am_Password_Input_Widget =
     Am_Text_Input_Widget.Create(DSTR("Am_Password_Input_Widget"));
  Am_Password_Input_Widget.Remove_Part (Am_TEXT_WIDGET_TEXT_OBJ);
  Am_Password_Input_Widget.Add_Part (Am_TEXT_WIDGET_TEXT_OBJ,
       Am_Hidden_Text.Create(DSTR("Value Text in Text_Widget"))
         .Set(Am_LEFT, text_widget_value_left)
         .Set(Am_TOP, text_widget_value_top)
         .Set(Am_WIDTH, text_widget_value_width)
          // font must be fixed width or doesn't work
         .Set(Am_FONT, Am_Font_From_Owner)
         .Set(Am_LINE_STYLE, text_widget_value_style)
         .Set(Am_TEXT, get_text_widget_value.Multi_Constraint())
     );
  Am_Password_Input_Widget.Get_Object (Am_COMMAND)
    .Set_Name(DSTR("Password_Input_Command"))
    .Set (Am_LABEL, "Password");

  ///////////////////////////////////////////////////////////////////////////
  //  Am_Number_Input_Widget
  ///////////////////////////////////////////////////////////////////////////

  Am_Number_Input_Widget =
     Am_Text_Input_Widget.Create(DSTR("Am_Number_Input_Widget"))
     .Add (Am_VALUE_1, Am_No_Value)
     .Add (Am_VALUE_2, Am_No_Value)
     .Add (Am_USE_ERROR_DIALOG, true)
     .Set (Am_TEXT_CHECK_LEGAL_METHOD, Am_Number_Input_Filter_Method)
     ;
  Am_Number_Input_Widget.Get_Object (Am_COMMAND)
     .Set_Name(DSTR("Number_Input_Command"))
     .Set (Am_LABEL, "Number_Input");

  //get rid of the constraint in the Am_VALUE slot because value is
  //set by the filter method
  Am_Number_Input_Widget.Remove_Slot(Am_VALUE);
  Am_Number_Input_Widget.Set(Am_VALUE, 0);

  ///////////////////////////////////////////////////////////////////////////
  // Moving to next widget
  ///////////////////////////////////////////////////////////////////////////

  Am_Tab_To_Next_Widget_Command = Am_Command.Create(DSTR("Tab_To_Next_Widget_Cmd"))
    .Set(Am_LABEL, "To Next Widget")
    .Set(Am_ACTIVE, true)
    .Set(Am_ACCELERATOR, (0L))
    .Set(Am_DO_METHOD, tab_to_next_widget)
    .Set(Am_IMPLEMENTATION_PARENT, Am_NOT_USUALLY_UNDONE) //not undo-able
    ;
  Am_Tab_To_Next_Widget_Interactor =
    Am_One_Shot_Interactor.Create(DSTR("Tab_To_Next_Widget_Interactor"))
    .Set(Am_START_WHEN, Am_Input_Char("ANY_TAB"))
    .Set(Am_PRIORITY, Am_INTER_PRIORITY_DIFF+5 ) //must be higher than running
    // fill next slot with a list of the tab widgets, or a formula to
    // compute it
    .Add(Am_LIST_OF_TEXT_WIDGETS, generate_list_of_text_widgets)
    .Set_Part(Am_COMMAND, Am_Tab_To_Next_Widget_Command.Create())
    ;

}

static void cleanup()
{
	
}

static Am_Initializer* initializer = new Am_Initializer(DSTR("Text_Widgets"), init, 5.3f, 123, cleanup);
