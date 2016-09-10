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


Am_Object Am_Button_Panel;
Am_Object Am_Button_In_Panel;


// For the where test of interactor: Find a component of owner which is active.
Am_Define_Method(Am_Where_Method, Am_Object, Am_In_Active_Widget_Part,
         (Am_Object /*inter*/, Am_Object object, Am_Object event_window,
          int x, int y))
{
  Am_Object result = Am_No_Object;
  if (Am_Point_In_All_Owners(object, x, y, event_window))
    result = Am_Point_In_Part (object, x, y, event_window);
  if (result.Valid() ) {
    Am_Value value;
    value=result.Peek(Am_ACTIVE);
    // if slot exists and is zero then return 0 (if slot doesn't
    // exist, return result)
    if (value.Exists() && !value.Valid())
      return Am_No_Object;
    else
      return result;
  }
  else return Am_No_Object;
}


// goes in the interactor for a button panel
Am_Define_Formula(Am_Value, Am_How_Set_From_Owner )
{
  return self.Get_Owner().Get( Am_HOW_SET );
}

//arbitrarily use first part as the start object
Am_Define_Method(Am_Explicit_Widget_Run_Method, void,
         widget_first_member_start_method,
         (Am_Object widget, Am_Value initial_value)) {
  Am_Object inter = widget.Get_Object(Am_INTERACTOR);
  if (initial_value.Valid()) widget.Set(Am_VALUE, initial_value);
  Am_Value_List parts = widget.Get(Am_GRAPHICAL_PARTS);
  parts.Start();
  Am_Object first_obj = parts.Get();
  Am_Start_Interactor(inter, first_obj);
}

Am_Define_Method(Am_Object_Method, void,
                 button_panel_abort_method, (Am_Object widget)) {
  Am_Object inter = widget.Get_Object(Am_INTERACTOR);
  Am_Abort_Interactor(inter);
  //now restore the widget's correct value
  Am_Value v;
  v=widget.Peek(Am_OLD_VALUE);
  // std::cout << "... Button panel " << widget << " aborting; set value to " << v
  //  <<std::endl <<std::flush;
  widget.Set(Am_VALUE, v);
}

///////////////////////////////////////////////////////////////////////////
// Constraints for button panels
///////////////////////////////////////////////////////////////////////////

void get_inter_value_from_panel_value(const Am_Value& panel_value,
                      const Am_Object& panel, Am_Value& value) {
  Am_Value_List parts, panel_value_list, inter_value_list;
  Am_Value v, label_or_id;
  Am_Object inter, item;
  bool is_list = false;
  bool found_it = false;
  int panel_value_found_count = 0;
  v=panel.Peek(Am_GRAPHICAL_PARTS);
  if (!v.Valid()) return;
  parts = v;
  value = (0L); //initialize return value
  if (panel_value.Valid() && Am_Value_List::Test(panel_value)) { // is a list
    panel_value_list = panel_value;
    is_list = true;
  }
  for (parts.Start(); !parts.Last(); parts.Next()) {
    item = parts.Get();
    label_or_id = item.Peek(Am_LABEL_OR_ID);
    if (is_list) {
      panel_value_list.Start();
      if (panel_value_list.Member(label_or_id)) { // then this one should be on
	item.Set(Am_SELECTED, true);
	inter_value_list.Add(item);
	panel_value_found_count++;
      }
      else  // this one should be off
	item.Set(Am_SELECTED, false);
    }
    else { // panel value not a list, should be Am_LABEL_OR_ID
      if (panel_value == label_or_id) { // then this one should be on
	item.Set(Am_SELECTED, true);
	value = item;
	found_it = true;
      }
      else  // this one should be off
	item.Set(Am_SELECTED, false);
    }
  } // end for parts
  if( is_list ) {
    value = inter_value_list;
#if 0  //this testing doesn't work, move to a Set_Type_Check
    int unused = panel_value_list.Length() - panel_value_found_count;
    if (unused != 0) {
     std::cerr << "** Amulet WARNING: Value List " << panel_value
	   << " which was set into widget "
	   << panel << " seems to contain " << unused
	   << " item(s) that do not match one the values in its "
	   "Am_ITEMS list.\n" <<std::flush;
    }
#endif
  }
  else
  {
#if 0 // this testing doesn't work, move to a Set_Type_Check **
    if( !found_it ) {
      // unfortunately, can't raise an error because panels are often
      // initialized with illegal values temporarily
      if (panel_value.Valid())) {
     std::cerr << "** Amulet WARNING: Value " << panel_value
	   << " (type=" << Am_Get_Type_Name(panel_value.type)
	   << ") which was set into widget " << panel
	  << " seems to not be one the values in its Am_ITEMS list\n" <<std::flush;
      }
      return;  // don't remove this line!
      // gcc 2.7.0 on HP requires it (otherwise compiler bug causes a crash)
      // -- rcm
    }
#endif
  return;
  }
}

//Calculate inter value from panel's value.  Used when
//the programmer sets the panel's value.  Also sets the SELECTED slot
Am_Define_Formula (Am_Value, inter_value_from_panel_value) {
  Am_Value value;
  Am_Object panel = self.Get_Owner();
  if (!panel.Valid()) {
    return value;
  }
  Am_Value panel_value;
  panel_value = panel.Peek(Am_VALUE);
  // std::cout << "** computing inter " << self << " value from panel value " <<
  //  panel_value <<std::endl <<std::flush;
  get_inter_value_from_panel_value(panel_value, panel, value);
  return value;
}

/******************************************************************************
 * set_parts_list_commands_old_owner
 */

int
set_parts_list_commands_old_owner(

  Am_Value_List &parts,
  Am_Object &widget )
{
  Am_Value item_value;
  Am_Object part, item;
  int ret = 0; //not used, just for debugging
  for( parts.Start(); !parts.Last(); parts.Next() )
  {
    part = parts.Get();
    item_value = part.Peek(Am_COMMAND);
    if( item_value.type == Am_OBJECT )
    {
      item = (Am_Object)item_value;
      if( item.Is_Instance_Of( Am_Command ) )
      {
        item.Set(Am_SAVED_OLD_OWNER, widget, Am_OK_IF_NOT_THERE);
        ret++;
      }
    }
  }
  return ret;
}


/******************************************************************************
 * Am_Panel_Set_Old_Owner_To_Me
 */

Am_Define_Formula( int, Am_Panel_Set_Old_Owner_To_Me )
{
  int ret = 0;
  Am_Object cmd;
  Am_Value_List parts;
  cmd = self.Get_Object(Am_COMMAND);
  if( cmd.Valid() )
  {
    cmd.Set( Am_SAVED_OLD_OWNER, self, Am_OK_IF_NOT_THERE );
    ret = 1;
  }
  parts = self.Get( Am_GRAPHICAL_PARTS );
  ret = set_parts_list_commands_old_owner( parts, self );

  //ret not used, different values just for debugging
  return ret;
}


// Max Item Height simply calculates maximum width of all parts, not maximum
// extents of the objects.
// potential circular constraint: dependancy on Am_WIDTH instead of
// Am_REAL_WIDTH in some cases.

Am_Define_Formula( int, max_item_width )
{
  // based on Am_map_width in opal.cc
  // finds width of widest visible item

  Am_Value_List components;
  components = self.Get( Am_GRAPHICAL_PARTS );
  int width = 0;
  Am_Object item;
  for( components.Start(); !components.Last(); components.Next() )
  {
    item = components.Get();
    if( (bool)item.Get( Am_VISIBLE ) )
    {
      int item_width;
      if( item.Get_Slot_Type( Am_REAL_WIDTH ) == Am_INT )
        item_width = item.Get( Am_REAL_WIDTH );
      else
        item_width = item.Get( Am_WIDTH );
      if( item_width > width )
        width = item_width;
    }
  }
  return width;
}

Am_Define_Formula( int, max_item_height )
{
  // based on Am_map_width in opal.cc
  // finds width of widest visible item

  Am_Value_List components;
  components = self.Get( Am_GRAPHICAL_PARTS );
  int height = 0;
  for( components.Start(); !components.Last(); components.Next() )
  {
    Am_Object item;
    item = components.Get();
    if( (bool)item.Get( Am_VISIBLE ) )
    {
      int item_height;
      if( item.Get_Slot_Type( Am_REAL_HEIGHT ) == Am_INT )
        item_height = item.Get( Am_REAL_HEIGHT );
      else
        item_height = item.Get( Am_HEIGHT );
      if( item_height > height )
        height = item_height;
    }
  }
  return height;
}




static void
init()
{
  Am_Object inter; // interactor in the widget
  Am_Object_Advanced obj_adv; // to get at advanced features like
                              // local-only and demons.

  //////////// Button Panel /////////////
  // Design: Basically, use the regular button as an Item_Prototype and the
  // standard mapping stuff to copy it as needed.
  // Setting the ITEM slot of a Am_Button_In_Panel will cause the
  // value to be copied into the COMMAND slot either as a value or a part so
  // the regular button functions will work with it.  Other values are also
  // copied down.

  Am_Button_Panel = Am_Widget_Map.Create(DSTR("Button Panel"))
    .Add (Am_VALUE, (0L))
    .Add (Am_OLD_VALUE, 0)
    .Add (Am_ITEM_OFFSET, 0) // how far to indent the string or obj
    // active here is whether whole widget is active.  Use command part if any
    .Set (Am_FIXED_WIDTH, true)
    .Set (Am_FIXED_HEIGHT, false)
    .Add (Am_KEY_SELECTED, false)
    .Add (Am_MAX_WIDTH, max_item_width)
    .Add (Am_MAX_HEIGHT, max_item_height)
    .Set (Am_WIDTH, Am_Width_Of_Parts)
    .Set (Am_HEIGHT,Am_Height_Of_Parts)
    .Add (Am_ACTIVE, Am_Active_From_Command)
    .Add (Am_ACTIVE_2, true) // used by interactive tools
    .Add (Am_FONT, Am_Default_Font)
    .Add (Am_LEAVE_ROOM_FOR_FRINGE, false) // normally no fringe in button panels
    .Add (Am_FILL_STYLE, Am_Default_Color)
    .Add (Am_FINAL_FEEDBACK_WANTED, false)
    .Add (Am_HOW_SET, Am_CHOICE_SET) //toggle is also a good choice
    .Set (Am_LAYOUT, Am_Vertical_Layout) // or horiz
    .Set (Am_H_ALIGN, Am_LEFT_ALIGN)
    .Set (Am_ITEMS, 0)
    .Add (Am_SET_COMMAND_OLD_OWNER, Am_Panel_Set_Old_Owner_To_Me)
    .Add (Am_WIDGET_START_METHOD, widget_first_member_start_method)
    .Add (Am_WIDGET_ABORT_METHOD, button_panel_abort_method)
    .Add (Am_WIDGET_STOP_METHOD, Am_Standard_Widget_Stop_Method)

    // Plus all the slots of a Map: Am_H_ALIGN, Am_V_ALIGN, Am_FIXED_WIDTH,
    //     Am_FIXED_HEIGHT
    .Add_Part (Am_INTERACTOR,
           inter = Am_Choice_Interactor_Repeat_Same.Create(DSTR("inter_in_button_panel"))
           .Set (Am_HOW_SET, Am_How_Set_From_Owner)
           .Set (Am_START_WHEN, Am_Default_Widget_Start_Char)
           .Set (Am_START_WHERE_TEST, Am_In_Active_Widget_Part)
           .Set (Am_ACTIVE, Am_Active_And_Active2)
           .Set (Am_VALUE, inter_value_from_panel_value.Multi_Constraint() )
           )
    .Set_Part (Am_ITEM_PROTOTYPE, Am_Button_In_Panel =
        Am_Button.Create (DSTR("Button_In_Panel_Proto"))
           .Add (Am_REAL_WIDTH, button_width)
           .Add (Am_REAL_HEIGHT, button_height)
           .Set (Am_WIDTH, panel_item_width)
           .Set (Am_HEIGHT, panel_item_height)
           .Set (Am_ACTIVE, active_from_command_panel)
           .Set (Am_SELECTED, false)
           .Set (Am_LEAVE_ROOM_FOR_FRINGE, Am_From_Owner( Am_LEAVE_ROOM_FOR_FRINGE ) )
           .Set (Am_WIDGET_LOOK, look_from_owner)
           .Set (Am_FONT, Am_Font_From_Owner)
           .Set (Am_ITEM_OFFSET, Am_From_Owner (Am_ITEM_OFFSET))
           .Set (Am_FILL_STYLE, Am_From_Owner (Am_FILL_STYLE))
           .Set (Am_FINAL_FEEDBACK_WANTED, final_feedback_from_owner)
           .Set (Am_SET_COMMAND_OLD_OWNER, (0L))
           )
    .Add_Part (Am_COMMAND, Am_Command.Create(DSTR("Command_In_Button_Panel"))
           .Set (Am_LABEL, "Panel Button"))
    ;

  //this do method is in addition to the impl_command's of the interactor
  inter.Get_Object(Am_COMMAND)
    .Set(Am_DO_METHOD, Am_Inter_For_Panel_Do)
    .Set(Am_UNDO_METHOD, Am_Widget_Inter_Command_Undo)
    .Set(Am_REDO_METHOD, Am_Widget_Inter_Command_Undo)
    .Set(Am_SELECTIVE_UNDO_METHOD, Am_Widget_Inter_Command_Selective_Undo)
    .Set(Am_SELECTIVE_REPEAT_SAME_METHOD,
     Am_Widget_Inter_Command_Selective_Repeat)
    .Set(Am_SELECTIVE_REPEAT_ON_NEW_METHOD, (0L))
    .Set_Name(DSTR("Command_In_Button_Inter"))
    ;

  obj_adv = (Am_Object_Advanced&)Am_Button_In_Panel;

  obj_adv = (Am_Object_Advanced&)Am_Button_Panel;

  // don't want the individual interactor from the button
  Am_Button_In_Panel.Remove_Part(Am_INTERACTOR);

  // when in a panel, the button's command object is gotten from the item slot
  // which is set automatically by the Map
  Am_Object command_obj = Am_Button_In_Panel.Get_Object(Am_COMMAND);
  Am_Button_In_Panel.Add(Am_ITEM, command_obj); // default value
  Am_Button_In_Panel.Add(Am_ITEM_TO_COMMAND, Am_Copy_Item_To_Command);
}

static void
cleanup()
{
	Am_Button_Panel.Destroy ();
}


static Am_Initializer *initializer = new Am_Initializer(DSTR("Am_Button_Panel"), init, 5.204f, 106, cleanup);

