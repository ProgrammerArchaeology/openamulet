#include <am_inc.h>
#include <amulet/impl/opal_obj2.h>

/******************************************************************************
 * Button Panels
 *****************************************************************************/


//Button in a panel is active if owner is and if specific command object is
Am_Define_Formula(bool, active_from_command_panel) {
  Am_Object owner = self.Get_Owner();
  if (owner.Valid() && !(bool)owner.Get(Am_ACTIVE)) return false;
  //now check my command object, if any
  Am_Value v;
  v = self.Peek(Am_COMMAND);
  if (v.Valid() && v.type == Am_OBJECT) {
    Am_Object cmd = v;
    if (cmd.Is_Instance_Of(Am_Command)) return cmd.Get(Am_ACTIVE);
  }
  return true;
}

//copy look from owner
Am_Define_Formula(Am_Value, look_from_owner )
{
  return self.Get_Owner().Get( Am_WIDGET_LOOK );
}
Am_Define_Formula( bool, final_feedback_from_owner )
{
  return self.Get_Owner().Get( Am_FINAL_FEEDBACK_WANTED );
}

Am_Define_Formula( bool, box_on_left_from_owner )
{
  return self.Get_Owner().Get( Am_BOX_ON_LEFT );
}

Am_Define_Formula( int, box_width_from_owner )
{
  return self.Get_Owner().Get( Am_BOX_WIDTH );
}

Am_Define_Formula( int, box_height_from_owner )
{
  return self.Get_Owner().Get( Am_BOX_HEIGHT );
}

// Panel item (width, height) calculate the (width, height) of an
// item in a button panel or menu by checking its owner's fixed_width slot.
// 0 or false means, use real width.
// 1 or true means use max. width (from owner).
// Otherwise, set it to that int value
Am_Define_Formula(int, panel_item_width) {
  Am_Object owner = self.Get_Owner();
  if (owner.Valid()) {
    Am_Value fw;
    fw = owner.Peek(Am_FIXED_WIDTH);
    if (fw.type == Am_BOOL) {
      if ((bool)fw) return owner.Get(Am_MAX_WIDTH);
      else return self.Get(Am_REAL_WIDTH);
    }
    else // not Am_BOOL
      if (fw.type == Am_INT) {
	int n = fw;
	if (n == 0) return self.Get(Am_REAL_WIDTH);
	if (n == 1) return owner.Get(Am_MAX_WIDTH);
	return self.Get(Am_FIXED_WIDTH);
      }
    //    else if (!fw.Exists())
    //  return 0;
      else { // neither int nor bool: error.
	Am_Error("Am_Panel_Item_Width: wrong type for Am_FIXED_WIDTH slot.",
		 self, Am_FIXED_WIDTH);
	return 0;
      }
  }
  else return self.Get(Am_REAL_WIDTH);
}

Am_Define_Formula( int, panel_item_height )
{
  Am_Object owner = self.Get_Owner();
  if( owner.Valid() )
  {
    Am_Value fh;
    fh = owner.Peek(Am_FIXED_HEIGHT);
    if( fh.type == Am_BOOL )
      if( (bool)fh )
        return owner.Get( Am_MAX_HEIGHT );
      else
      {
        int height = self.Get( Am_REAL_HEIGHT );
        return height;
      }
    else // not Am_BOOL
      if( fh.type == Am_INT )
      {
        int n = fh;
        if( n == 0 )
          return self.Get( Am_REAL_HEIGHT );
        if( n == 1 )
          return owner.Get( Am_MAX_HEIGHT );
        return self.Get( Am_FIXED_HEIGHT );
      }
    //    else if (!fh.Exists())
    // return 0;
      else // neither int nor bool: error.
      {
        Am_Error("Am_Panel_Item_Height: wrong type for Am_FIXED_HEIGHT slot",
		 self, Am_FIXED_HEIGHT);
        return 0;
      }
  }
  else return self.Get(Am_REAL_HEIGHT);
}

//used by Am_Copy_Item_To_Command and Am_Menu_Bar_Copy_Item_To_Command
//  Converts the Am_ITEM slot set by the map into a command part or value
void copy_item_to_command_proc(Am_Object& panel_item,
				      const Am_Value& value ) {
  if( value.type == Am_OBJECT && Am_Object( value ).Valid() &&
      Am_Object( value ).Is_Instance_Of( Am_Command ) )
  {
    // then is a command
    Am_Object command = value;
    Am_Object owner = command.Get_Owner(Am_NO_DEPENDENCY);
    if( owner == panel_item )
    {
      if( panel_item.Get_Object( Am_COMMAND, Am_NO_DEPENDENCY ) == command )
        return; // already fine
      else
        panel_item.Remove_Part( command ); // part of me in the wrong slot??
    }
    else if( owner.Valid() ) {
      Am_Object owners_cmd = owner.Get(Am_ITEM, Am_NO_DEPENDENCY |
				                Am_RETURN_ZERO_ON_ERROR);
      if (owners_cmd != command) {
	//this can happen when the map re-shuffles the items due to a
	//new one being added or removed.  The map would remove command
	//from owner later.
	// std::cout << "Owner " << owner << " changing item from " << command
	//     << " to " << owners_cmd <<std::endl <<std::flush;
	command.Remove_From_Owner();
      }
      else // make new instance, leave command where it is
	command = command.Create();
    }
    if (panel_item.Is_Part_Slot(Am_COMMAND)) {
      // prevent deleting the prior command
      panel_item.Remove_Part( Am_COMMAND );
    }
    //Set_Part removes old value if any
    panel_item.Set_Part(Am_COMMAND, command, Am_OK_IF_NOT_THERE);
    panel_item.Set(Am_ATTACHED_COMMAND, command, Am_OK_IF_NOT_THERE);
  }
  else // not a command object, add to COMMAND slot as a value
  {
    if (panel_item.Is_Part_Slot(Am_COMMAND )) {
      // prevent deleting the prior command
      panel_item.Remove_Part( Am_COMMAND );
    }
    panel_item.Set(Am_COMMAND, value, Am_OK_IF_NOT_THERE);
  }
}

// Get the ITEM value set by the Map.
// If value is a command object, add it as a part otherwise just set
// the Command slot with it
Am_Define_Formula (Am_Value, Am_Copy_Item_To_Command)
{
  Am_Value value;
  value = self.Peek(Am_ITEM);
  // std::cout << "regular copy_item_to_command for " << self << " value = " << value
  //     <<std::endl <<std::flush;
  copy_item_to_command_proc( self, value );
  return value;
}


bool inter_value_is_or_contains(Am_Object inter, Am_Object new_object)
{
  Am_Value inter_value;
  inter_value=inter.Peek(Am_VALUE);
  if (Am_Value_List::Test(inter_value)) { // is a list
    Am_Value_List inter_list;
    inter_list = inter_value;
    inter_list.Start();
    if (inter_list.Member(new_object)) return true;
    else return false;
  }
  else { //not list
    Am_Object val;
    val = inter_value;
    if (val == new_object) return true;
    else return false;
  }
}

//assign to the panel value based on the interactor's value.
void set_panel_value_from_inter_value(Am_Object inter)
{
  Am_Object panel = inter.Get_Owner();
  if (!(inter.Valid())) return;
  Am_Value inter_value, label_or_id, value;
  inter_value=inter.Peek(Am_VALUE);
  //std::cout << "** setting from inter " << inter  << " value " << inter_value
  //      <<std::endl <<std::flush;
  if (Am_Value_List::Test(inter_value)) // is a list
  {
    Am_Value_List inter_list, panel_list;
    inter_list = inter_value;
    Am_Object item;
    for (inter_list.Start(); !inter_list.Last(); inter_list.Next())
    {
      item = inter_list.Get();
      label_or_id=item.Peek(Am_LABEL_OR_ID);
      panel_list.Add(label_or_id);
    }
    value = panel_list;
  } //end if value is a list
  else //value is a single value, should be an object or null
  {
    if (inter_value.Valid())
    {
      Am_Object value_obj;
      value_obj = inter_value;
      value=value_obj.Peek(Am_LABEL_OR_ID);
    }
    else
      value = 0;
  }
  Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING,
            "++ Panel DO method setting the Am_VALUE of " << panel
            << " to " << value);
  panel.Set(Am_VALUE, value);
}


