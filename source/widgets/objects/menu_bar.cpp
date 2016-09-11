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

Am_Object Am_Menu_Bar;
Am_Object Am_Menu_Bar_Sub_Window_Proto;


///////////////////////////////////////////////////////////////////////////
// Menu_bars
///////////////////////////////////////////////////////////////////////////

// DESIGN:
// The top-level Am_ITEMS slot should contain a list of command
// objects.  Unlike other menus and panels, the first level members of
// the value_list in the Am_ITEMS slot MUST be command objects. The
// Am_LABEL of each of these command objects will be the top-level
// names of the sub-menus.  Each of these command objects should
// contain a Am_ITEMS slot containing the sub-menu items, which can be
// strings, objects or commands like regular menus and panels.
//
// The top level menu_bar is a horizontal menu.  Each item is set with a
// Am_SUB_MENU slot containing the window containing a Am_SUB_MENU part
// containing a vertical menu.  The interactor in the top level menu_bar works
// over all the windows, and its interim_do deals with turning the visibility
// on and off.  The individual menu items know how to take the right part of
// the Am_ITEM list for their own use.

Am_Define_Formula(int, sub_menu_set_old_owner)
{
  Am_Object window, for_item, menu_bar;
  window = self.Get_Owner();
  int ret = 0;
  if( window.Valid() )
  {
    for_item = window.Get( Am_FOR_ITEM );
    if( for_item.Valid() )
    {
      menu_bar = for_item.Get_Owner();
      if( menu_bar.Valid() )
      {
        Am_Value_List parts;
        parts = self.Get( Am_GRAPHICAL_PARTS );
        ret = set_parts_list_commands_old_owner( parts, menu_bar );
      }
    }
  }
  return ret;
}


Am_Define_Formula( int, popup_sub_win_width )
{
  return self.Get_Object( Am_SUB_MENU ).Get( Am_WIDTH );
}

Am_Define_Formula( int, popup_sub_win_height )
{
  return self.Get_Object( Am_SUB_MENU ).Get( Am_HEIGHT );
}

Am_Define_Object_Formula( popup_sub_win_undo_handler )
{
  Am_Object for_item, main_window, undo_handler;
  for_item = self.Get( Am_FOR_ITEM );
  if( for_item.Valid() )
  {
    Am_Value v;
    v = for_item.Peek(Am_WINDOW);
    if( v.Valid() )
    {
      main_window = v;
      if( main_window.Valid() )
      {
        v = main_window.Peek(Am_UNDO_HANDLER);
        if( v.Valid() )
          undo_handler = v;
      }
    }
  }
  return undo_handler;
}

extern Am_Object Am_Menu_Bar_Sub_Window_Proto; //defined below

void create_sub_menu_window(Am_Object menu_item)
{
	Am_Object new_window = Am_Menu_Bar_Sub_Window_Proto.Create();
	new_window.Set(Am_FOR_ITEM, menu_item);

	Am_Screen.Add_Part(new_window);
	menu_item.Set(Am_SUB_MENU, new_window, Am_OK_IF_NOT_THERE);

	return;
}

// Put into the Am_ITEM_TO_COMMAND slot of each top-level item of menu_bar.
// Gets the ITEM value set by the Map. If value is a command object, add it as
// a part.  Also creates the sub-menus
Am_Define_Formula(Am_Value, menu_bar_copy_item_to_command)
{
	Am_Value value;
	value = self.Peek(Am_ITEM);
	//if (!value.Exists()) return 0;
	if( value.type == Am_OBJECT && Am_Object (value).Valid() && Am_Object (value).Is_Instance_Of(Am_Command) )
	{
		//then fine
	}
	else
	{
		Am_ERRORO("In a menu_bar, the top-level items must be\ncommand objects, but for " << self << " item # "	<< (int)self.Get(Am_RANK) + 1 << " is " << value, self, Am_ITEM);
	}

	copy_item_to_command_proc(self, value);

	//now create the sub-menu if necessary
	if(self.Get_Slot_Type (Am_SUB_MENU) != Am_OBJECT)
	{
		create_sub_menu_window(self);
	}

	return(value);
}

// formula for the items list of a sub-menu: get the list from the Am_FOR_ITEM
// of my window.
Am_Define_Formula (Am_Value, sub_menu_items) {
  Am_Value value;
  value = 0;
  Am_Object window, for_item;
  window = self.Get(Am_WINDOW);
  if (window.Valid()) {
    for_item = window.Get(Am_FOR_ITEM);
    if (for_item.Valid()) {
      Am_Object cmd;
      cmd = for_item.Get(Am_ITEM);
      // all top-level items must be commands
      if (cmd.Valid()) {
    // then set the return value of this formula with the contents of the
    // Am_ITEMS slot of the command.
    value = cmd.Peek(Am_ITEMS);
    if (!value.Valid() )
          value = 0;
      }
    }
  }
  return value;
}

//set the sub-window visibility, and also deal with interim selected of the
//associated main item
void set_sub_window_vis(Am_Object &sub_window, bool vis, Am_Object &main_win) {
  set_popup_win_visible(sub_window, vis, main_win);
  Am_Object for_item = sub_window.Get(Am_FOR_ITEM);
  if (for_item.Valid()) for_item.Set(Am_INTERIM_SELECTED, vis);
}

// For the where test of the interactor in the menu bar: Work over all windows
// and items, and pop-up sub-windows when necessary.
Am_Define_Method(Am_Where_Method, Am_Object, in_menu_bar_item,
         (Am_Object inter, Am_Object /* object */,
          Am_Object event_window, int x, int y)) {
  Am_Object menu_bar = inter.Get_Owner ();
  Am_Object result = Am_No_Object;
  Am_Object menu_bar_window, old_window, new_window;
  Am_Value v;
  // get the old sub_window which used to be visible
  v=inter.Peek(Am_SUB_MENU);
  if (v.Valid()) old_window = v;
  menu_bar_window = menu_bar.Get(Am_WINDOW);
  if (menu_bar_window == event_window) {
    // in the top level window,
    result = Am_Point_In_Part (menu_bar, x, y, event_window);
    // now deal with visibility of sub-windows
    if (result.Valid()) {
      v=result.Peek(Am_SUB_MENU);
      if (v.Valid())
        new_window = v;
    }
    if (new_window != old_window && new_window.Valid()) {
      if (old_window.Valid())
	set_sub_window_vis(old_window, false, menu_bar_window);
      if (new_window.Valid())
	set_sub_window_vis(new_window, true, menu_bar_window);
      inter.Set(Am_SUB_MENU, new_window, Am_OK_IF_NOT_THERE);
    }
    return result;  //don't test if top-level menu items are active!!
  }
  else { //must be in a sub-menu
    Am_Object sub_menu = event_window.Get_Object(Am_SUB_MENU);
    if (sub_menu.Valid())
      result = Am_Point_In_Part (sub_menu, x, y, event_window);
    // test if active
    if (result.Valid() ) {
      Am_Value value;
      value=result.Peek(Am_ACTIVE);
      // if slot exists and is zero then return 0.
      // If slot does NOT exist, return result.
      if (value.Exists() && !value.Valid())
        return Am_No_Object;
      else
        return result;
    }
  }
  return Am_No_Object;
}

//menu_bar has a list of parts which are Am_Item_In_Menu and each one should
//have a Am_SUB_MENU slot which contains the appropriate sub-menu
Am_Define_Value_List_Formula(menu_bar_window_list) {
  Am_Object menu_bar = self.Get_Owner();
  Am_Object menu_bar_main_win = menu_bar.Get(Am_WINDOW);
  Am_Value_List window_list;
  Am_Value_List components;
  Am_Object comp;
  Am_Value v;
  components = menu_bar.Get(Am_GRAPHICAL_PARTS);
  if (menu_bar_main_win.Valid())
    window_list.Add(menu_bar_main_win); // main window must be there also
  for (components.Start(); !components.Last(); components.Next()) {
    comp = components.Get( );
    v = comp.Peek(Am_SUB_MENU);
    if (v.Valid()) {
      window_list.Add(v);
    }
  }
  return window_list;
}

// A custom destroy demon for menu bar to destroy the menu popup windows.
void Am_Destroy_Menu_Bar(Am_Object object)
{
	Am_Value_List 	parts;
	Am_Object 		part;
	Am_Object		sub_menu;

	// get all the parts of the given object
	parts = object.Get(Am_GRAPHICAL_PARTS);

	// iterate through the returned list
	for(parts.Start(); !parts.Last(); parts.Next())
	{
		// get the current part
		part 		= parts.Get();

		// is this a valid part?
		if(part.Valid() == true)
		{
			// get the submenu object and destroy it
			sub_menu 	= part.Get(Am_SUB_MENU);
			sub_menu.Destroy();
		}
	}

	Am_Object_Demon* proto_demon = ((Am_Object_Advanced&)Am_Menu).Get_Demons().Get_Object_Demon(Am_DESTROY_OBJ);
	if(proto_demon)
	{
		proto_demon(object);
	}

	return;
}

//current object should be already set into Am_INTERIM_VALUE and old
//value in Am_OLD_INTERIM_VALUE.  This will replace the choice_inter's interim
//do method.
Am_Define_Method(Am_Object_Method, void, menu_bar_inter_interim_do,
         (Am_Object inter)) {
  Am_Object old_object, new_object, menu_bar_win, old_object_win,
    new_object_win, main_item;
  menu_bar_win = inter.Get_Owner().Get(Am_WINDOW);
  old_object = inter.Get(Am_OLD_INTERIM_VALUE);
  new_object = inter.Get(Am_INTERIM_VALUE);
  if (new_object.Valid() )
    new_object.Set(Am_INTERIM_SELECTED, true);
  if (old_object.Valid() ) {
    old_object_win = old_object.Get(Am_WINDOW);
    //never clear the Am_INTERIM_SELECTED of top-level items
    if (old_object_win != menu_bar_win)
      old_object.Set(Am_INTERIM_SELECTED, false);
  }
}

// returns main-item, which might be obj if this is a main-item
Am_Object clear_interim_sel(Am_Object obj, Am_Object menu_bar_win) {
  Am_Object obj_win, main_item;
  if (obj.Valid() ) {
    obj.Set(Am_INTERIM_SELECTED, false);
    obj_win = obj.Get(Am_WINDOW);
    if (obj_win.Valid() &&
    obj_win != menu_bar_win) { // then clear old main menu selection
      main_item = obj_win.Get(Am_FOR_ITEM);
      if (main_item.Valid())
    main_item.Set(Am_INTERIM_SELECTED, false);
    }
    else main_item = obj;
  }
  return main_item;
}

Am_Define_Method(Am_Object_Method, void, menu_bar_inter_abort,
         (Am_Object inter)) {
  Am_Object obj, menu_bar_win, sub_window;
  menu_bar_win = inter.Get_Owner().Get(Am_WINDOW);
  obj = inter.Get(Am_OLD_INTERIM_VALUE);
  clear_interim_sel(obj, menu_bar_win);
  obj = inter.Get(Am_INTERIM_VALUE);
  clear_interim_sel(obj, menu_bar_win);
  // now make sub_window go away
  sub_window = inter.Get(Am_SUB_MENU);
  if (sub_window.Valid()) set_sub_window_vis(sub_window, false, menu_bar_win);
  inter.Set(Am_SUB_MENU, Am_No_Object, Am_OK_IF_NOT_THERE);
}

Am_Define_Method(Am_Object_Method, void, menu_bar_inter_do,
         (Am_Object inter)) {
  Am_Object new_object, parent_command, item_command, main_item_command,
    main_item, menu_bar, sub_window, menu_bar_win;
  Am_Value value;
  new_object = inter.Get(Am_INTERIM_VALUE);
  menu_bar = inter.Get_Owner();
  menu_bar_win = menu_bar.Get(Am_WINDOW);

  // make sub_window go away
  sub_window = inter.Get(Am_SUB_MENU);
  if (sub_window.Valid()) set_sub_window_vis(sub_window, false, menu_bar_win);
  inter.Set(Am_SUB_MENU, Am_No_Object, Am_OK_IF_NOT_THERE);

  // if click on top-level item, it might not be valid, so check first
  if (new_object.Valid()) {
    value=new_object.Peek(Am_ACTIVE);
    // if slot exists and is zero then return 0.
    // If slot does NOT exist, return result.
    if (value.Exists() && !value.Valid()) {
      Am_Abort_Interactor(inter);
      return;
    }
    // else continue
  }

  // Now the standard choice interactor stuff.
  //   clear interim selection of sub-item and main item.  Returns
  //   main-item, which might be new_object if it is a main-item
  main_item = clear_interim_sel(new_object, menu_bar_win);

  // sets the interactor's command value and SELECTED of the individual widgets
  Am_Choice_Set_Value(inter, true);
  inter.Set(Am_OBJECT_MODIFIED, new_object);

  // now find and set the right parent command object
  // This code is like Am_Inter_For_Panel_Do
  if (main_item.Valid()) {
    value=main_item.Peek(Am_COMMAND);
    if (value.type == Am_OBJECT) {
      main_item_command = value;
      if (!main_item_command.Is_Instance_Of(Am_Command))
	main_item_command = 0;  //wrong type object
    }
  }

  if (new_object.Valid() ) {
    value=new_object.Peek(Am_COMMAND);
    if (value.type == Am_OBJECT) {
      item_command = value;
      if (item_command.Is_Instance_Of(Am_Command))
	parent_command = item_command;
      else item_command = 0; // wrong type object
    }
  }

  if (!item_command.Valid() ) { //not in the item, use main item
    parent_command = main_item_command;
  }

  // set the value field of the parent_command
  set_command_from_button(parent_command, new_object);

  // install the found command into my command object's parent
  value=inter.Peek(Am_COMMAND);
  if (value.type == Am_OBJECT) {
    Am_Object command_obj = value;
    if (command_obj.Is_Instance_Of(Am_Command)) {
      command_obj.Set(Am_IMPLEMENTATION_PARENT, parent_command);
    }
  }
}

//this goes in the window for the sub-menu
Am_Define_Style_Formula (popup_fill_from_for_item) {
  Am_Value v;
  v = self.Get_Object(Am_FOR_ITEM).Peek(Am_FILL_STYLE);
  if (v.Valid()) return v;
  return Am_Motif_Light_Blue;
}

// the next three go into the sub-menu itself.  FOR_ITEM is in my window
Am_Define_Formula (Am_Value, sub_menu_look_from_win_for_item) {
  Am_Value v;
  v = self.Get_Object(Am_WINDOW).Get_Object(Am_FOR_ITEM).Peek(Am_WIDGET_LOOK);
  if (v.Valid()) return v;
  else return Am_MOTIF_LOOK;
}

/******************************************************************************
 * sub_menu_font_from_win_for_item
 * Button in a menu_bar is active if owner and main-menu is, and if specific
 * command object is
 */

Am_Define_Formula( bool, menu_bar_sub_item_active )
{
  Am_Object owner = self.Get_Owner();
  if( owner.Valid() && !(bool)owner.Get(Am_ACTIVE) )
    return false;
  // now check the main-item I am attached to, if any
  Am_Object win;
  win = self.Get(Am_WINDOW);
  if (win.Valid())
  {
    Am_Object main_item;
    main_item = win.Get(Am_FOR_ITEM);
    if( main_item.Valid() && !(bool)main_item.Get(Am_ACTIVE) )
      return false;
  }
  //now check my command object, if any
//   Am_Object cmd = self.GV_Object(Am_COMMAND);
//   if( cmd.Valid() && cmd.Is_Instance_Of(Am_Command) )
//     return cmd.GV(Am_ACTIVE);
//   else
//     return true;
  Am_Value cmd_value = self.Peek(Am_COMMAND);
  if (cmd_value.type == Am_OBJECT) {
    Am_Object cmd = cmd_value;
    if( cmd.Is_Instance_Of(Am_Command) )
      return cmd.Get(Am_ACTIVE);
  }
  return true;
}

Am_Define_Formula( bool, menu_bar_sub_item_active_2 )
{
  Am_Object owner = self.Get_Owner();
  if( owner.Valid() && !(bool)owner.Get(Am_ACTIVE_2) )
    return false;
  // now check the main-item I am attached to, if any
  Am_Object win = self.Get(Am_WINDOW);
  if (win.Valid())
  {
    Am_Object main_item = win.Get(Am_FOR_ITEM);
    if( main_item.Valid() && !(bool)main_item.Get(Am_ACTIVE_2) )
      return false;
  }
  return true;
}


Am_Define_Formula( int, menu_bar_width )
{
  return self.Get_Owner().Get( Am_WIDTH );
}

/******************************************************************************
 * menu_bar_height
 *   Based on height_of_parts_procedure in opal.cc.
 */

Am_Define_Formula( int, menu_bar_height )
{
  int max_y = 0, comp_bottom = 0;
  int border = self.Get( Am_MENU_BORDER );
  Am_Widget_Look look   = self.Get( Am_WIDGET_LOOK );
  Am_Value_List components;
  Am_Object comp;
  components = self.Get( Am_GRAPHICAL_PARTS );
  for( components.Start(); !components.Last(); components.Next() )
  {
    comp = components.Get();
    // compute how much of the component extends below the origin
    comp_bottom = ((int)comp.Get( Am_TOP ) + (int)comp.Get( Am_HEIGHT ));
    if( comp_bottom > max_y )
      max_y = comp_bottom;
  }

  int inset_border = 0;
  if( look == Am_MOTIF_LOOK )
          inset_border = 3;

  return max_y + border + inset_border;
}

/******************************************************************************
 * menu_bar_h_spacing
 */

Am_Define_Formula( int, menu_bar_h_spacing )
{
  Am_Widget_Look look = self.Get( Am_WIDGET_LOOK );
  return ( look == Am_MACINTOSH_LOOK ) ? -4 : 0;
}

/******************************************************************************
 * menu_bar_x_offset
 */

Am_Define_Formula( int, menu_bar_x_offset )
{
  Am_Widget_Look look = self.Get( Am_WIDGET_LOOK );
  int border = self.Get( Am_MENU_BORDER );
  if( look == Am_MOTIF_LOOK )
    return border + 3;
  else
    return border;
}

/******************************************************************************
 * menu_bar_y_offset
 */

Am_Define_Formula( int, menu_bar_y_offset )
{
  Am_Widget_Look look = self.Get( Am_WIDGET_LOOK );
  int border = self.Get( Am_MENU_BORDER );
  if( look == Am_MOTIF_LOOK )
    return border + 3;
  else
    return border;
}

/******************************************************************************
 * menu_bar_sub_win_top
 */

Am_Define_Formula( int, menu_bar_sub_win_top )
{
  Am_Object for_item = self.Get(Am_FOR_ITEM);
  Am_Object menu_bar = for_item.Get_Owner();
  Am_Widget_Look look   = menu_bar.Get( Am_WIDGET_LOOK );
  int height = menu_bar.Get( Am_HEIGHT );

  int overlap = 0;
  switch( look.value )
  {
    case Am_MOTIF_LOOK_val:
      overlap = 5;
      break;

    case Am_WINDOWS_LOOK_val:
      overlap = 3;
      break;

    case Am_MACINTOSH_LOOK_val:
      overlap = 1;
      break;

    default:
      Am_Error ("Unknown Look parameter");
      break;
  }

  if( for_item.Valid() && menu_bar.Valid() )
  {
    int x = 0; //initializations so no compiler warnings for translate_coords
    int y = 0;
    // get the coordinates of the bottom of the menu item w.r.t. the screen
    Am_Translate_Coordinates( menu_bar, 0, height - overlap, Am_Screen, x, y);
    return y;
  }
  else
    return 0;
}

/******************************************************************************
 * menu_bar_sub_win_left
 */

Am_Define_Formula( int, menu_bar_sub_win_left )
{
  Am_Object for_item = self.Get(Am_FOR_ITEM);
  if( for_item.Valid() )
  {
    int x = 0; //initializations so no compiler warnings for translate_coords
    int y = 0;
    // get the coordinates of the left of the menu item w.r.t. the screen
    Am_Translate_Coordinates(for_item, 0, 0, Am_Screen, x, y);
    return x;
  }
  else
    return 0;
}

/******************************************************************************
 * sub_menu_font_from_win_for_item
 */

Am_Define_Font_Formula(sub_menu_font_from_win_for_item)
{
  Am_Value v;
  v = self.Get_Object(Am_WINDOW).Get_Object(Am_FOR_ITEM).Peek(Am_FONT);
  if( v.Valid() )
    return v;
  else
    return Am_Default_Font;
}



static void init()
{
  Am_Object inter; // interactor in the widget
  Am_Object_Advanced obj_adv; // to get at advanced features like
                              // local-only and demons.

  ///////////////////////////////////////////////////////////////////////////
  // Menu_Bars
  ///////////////////////////////////////////////////////////////////////////

  // Based on menus.

  // internal: prototype for the sub-menu windows
  Am_Object proto, sub_menu;

  Am_Pop_Up_Menu_From_Widget_Proto =
    Am_Window.Create(DSTR("Pop_Up_Menu_From_Widget"))
      .Set(Am_OMIT_TITLE_BAR, true)
      .Set(Am_SAVE_UNDER, true)
      .Add(Am_FOR_ITEM, Am_No_Object) // for_item is menu button widget
      .Set(Am_LEFT, 0) // usually overridden with a formula
      .Set(Am_TOP, 0)  // usually overridden with a formula
      .Set(Am_WIDTH, popup_sub_win_width.Multi_Constraint())
      .Set(Am_HEIGHT, popup_sub_win_height.Multi_Constraint())
      .Set(Am_VISIBLE, false)
      .Set(Am_FILL_STYLE, popup_fill_from_for_item)
      .Set(Am_UNDO_HANDLER, popup_sub_win_undo_handler)
      // should specify an undo handler
      .Add_Part(Am_SUB_MENU, sub_menu = Am_Menu.Create(DSTR("Sub_Menu"))
		.Set(Am_ITEMS, sub_menu_items)
		.Set(Am_FILL_STYLE, Am_From_Owner (Am_FILL_STYLE))
		.Set(Am_WIDGET_LOOK, sub_menu_look_from_win_for_item)
		.Set(Am_FONT, sub_menu_font_from_win_for_item)
		.Set(Am_SET_COMMAND_OLD_OWNER, sub_menu_set_old_owner)
		);

  //remove the interactor from the sub-menu
  sub_menu.Remove_Part( Am_INTERACTOR );

  proto = sub_menu.Get_Object( Am_ITEM_PROTOTYPE );
  proto.Set_Name(DSTR("popup_sub_item"))
      .Set( Am_ACTIVE_2, menu_bar_sub_item_active_2 )
      .Set( Am_ACTIVE, menu_bar_sub_item_active );

  Am_Menu_Bar_Sub_Window_Proto =
    Am_Pop_Up_Menu_From_Widget_Proto.Create(DSTR("Sub_Menu_Window"))
	  //under windows, these slots are set and the constraints
	  //go away, so make sure they are Multi_Constraint
      .Set( Am_LEFT, menu_bar_sub_win_left.Multi_Constraint() )
      .Set( Am_TOP, menu_bar_sub_win_top.Multi_Constraint() )
      ;

	// create a copy/instance of Am_Menu
  Am_Menu_Bar = Am_Menu.Create(DSTR("Menu_Bar"))
      //default width = width of container (usually a window)
      .Set( Am_WIDTH, menu_bar_width )
      .Set( Am_HEIGHT, menu_bar_height )
      .Set( Am_HOW_SET, Am_CHOICE_SET )
      .Set( Am_FIXED_WIDTH, false )
      .Set( Am_FIXED_HEIGHT, true )
      .Set( Am_LEFT_OFFSET, menu_bar_x_offset )
      .Set( Am_TOP_OFFSET, menu_bar_y_offset )
      .Set( Am_LAYOUT, Am_Horizontal_Layout )
      .Set( Am_H_SPACING, menu_bar_h_spacing )
      .Set( Am_ITEMS, 0 )
      .Add( Am_SELECTION_WIDGET, Am_No_Object ) //can be set here or in cmds
      ;

  obj_adv = (Am_Object_Advanced&)Am_Menu_Bar;
  obj_adv.Get_Slot(Am_FILL_STYLE)
      .Set_Demon_Bits( Am_STATIONARY_REDRAW | Am_EAGER_DEMON );
  Am_Demon_Set demons = obj_adv.Get_Demons().Copy();
  demons.Set_Object_Demon( Am_DESTROY_OBJ, Am_Destroy_Menu_Bar );
  obj_adv.Set_Demons( demons );

  proto = Am_Menu_Bar.Get_Object( Am_ITEM_PROTOTYPE );
  proto.Remove_Slot( Am_ITEM_TO_COMMAND ); // make sure the inherited
                                           // constraint is destroyed
  proto.Set( Am_ITEM_TO_COMMAND, menu_bar_copy_item_to_command );
  proto.Add( Am_SUB_MENU, Am_No_Object );  // fix a bug that if create slot
                                           // later, doesn't re-evaluate formulas

  //make Am_SUB_MENU slot be local
  obj_adv = ( Am_Object_Advanced& )proto;
  obj_adv.Get_Slot( Am_SUB_MENU ).Set_Inherit_Rule( Am_LOCAL );

  inter = Am_Menu_Bar.Get_Object( Am_INTERACTOR )
      .Set( Am_START_WHERE_TEST, in_menu_bar_item )
      .Set( Am_MULTI_OWNERS, menu_bar_window_list )
      .Set( Am_INTERIM_DO_METHOD, menu_bar_inter_interim_do )
      .Set( Am_ABORT_DO_METHOD, menu_bar_inter_abort )
      .Set( Am_DO_METHOD, menu_bar_inter_do )
      .Set_Name(DSTR("inter_in_menu_bar"))
      ;

  inter.Get_Object(Am_COMMAND)
      .Set( Am_DO_METHOD, (0L) ) //get rid of Am_Inter_For_Panel_Do
      .Set( Am_UNDO_METHOD, (0L) )
      .Set( Am_REDO_METHOD, (0L) )
      .Set( Am_SELECTIVE_UNDO_METHOD, (0L) )
      .Set( Am_SELECTIVE_REPEAT_SAME_METHOD, (0L) )
      .Set( Am_SELECTIVE_REPEAT_ON_NEW_METHOD, (0L) )
      ;
}

static void cleanup()
{
	Am_Menu_Bar.Destroy ();
}


static Am_Initializer *initializer = new Am_Initializer(DSTR("Am_Menu_Bar"), init, 5.208f, 114, cleanup);
