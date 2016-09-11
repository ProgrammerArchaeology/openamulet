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

// This file contains the functions for handling the Choice Interactor
//
// Designed and implemented by Brad Myers


#include <am_inc.h>

#include AM_IO__H

//#include INTER_ADVANCED__H
#include <amulet/impl/inter_all.h>
#include <amulet/impl/am_inter_location.h>
#include FORMULA__H
#include IDEFS__H
#include VALUE_LIST__H
#include REGISTRY__H
#include INITIALIZER__H

//////////////////////////////////////////////////////////////////
/// Utilities
//////////////////////////////////////////////////////////////////

void Choice_Handle_New_Old(Am_Object command_obj, Am_Object new_object,
			   Am_Object inter, int x, int y,
			   Am_Object ref_obj, Am_Input_Char ic) {
  if ((bool)inter.Get(Am_FIRST_ONE_ONLY)) {
    Am_Object start_object;
    start_object = inter.Get(Am_START_OBJECT);
    if (new_object != start_object) new_object = Am_No_Object;
  }
  Am_Object prev_object;
  prev_object = inter.Get(Am_INTERIM_VALUE);
  if (prev_object != new_object) {
    inter.Set(Am_OLD_INTERIM_VALUE, prev_object);
    inter.Set(Am_INTERIM_VALUE, new_object);
    Am_Inter_Call_Both_Method(inter, command_obj, Am_INTERIM_DO_METHOD, x, y,
			      ref_obj, ic, new_object, Am_No_Location);
  }
}

Am_Object choice_set_impl_command(Am_Object inter,
				  Am_Object /* object_modified */,
				  Am_Inter_Location /* data */) {
  Am_Object impl_command;
  impl_command = inter.Get_Object(Am_IMPLEMENTATION_COMMAND);
  if(impl_command.Valid()) {

    //set current value into the Am_VALUE slot
    Am_Value value;
    value=inter.Peek(Am_VALUE);
    impl_command.Set(Am_VALUE, value.Exists() ? value : Am_No_Value,
		     Am_OK_IF_NOT_THERE);

    impl_command.Set(Am_OBJECT_MODIFIED, value);

    //set original data into the OLD_VALUE slot
    value=inter.Peek(Am_OLD_VALUE);
    impl_command.Set(Am_OLD_VALUE, value.Exists() ? value : Am_No_Value,
		     Am_OK_IF_NOT_THERE);
  }
  return impl_command;
}

//////////////////////////////////////////////////////////////////
/// Methods
//////////////////////////////////////////////////////////////////

Am_Define_Method(Am_Inter_Internal_Method, void, Am_Choice_Start_Method,
		 (Am_Object& inter, Am_Object& object, Am_Object& event_window,
		  Am_Input_Event *ev)) {
  Am_INTER_TRACE_PRINT(inter, "Choice starting over " << object);

  //do this before prototype's method, so won't even change state
  if (!Am_Check_Inter_Abort_Inactive_Object(object, Am_SELECT_INACTIVE, inter))
    return;

  // first, call the prototype's method
  Am_Inter_Internal_Method inter_method;
  inter_method = Am_Interactor.Get(Am_INTER_START_METHOD);
  inter_method.Call(inter, object, event_window, ev);

  Am_Object command_obj;
  int x = ev->x;
  int y = ev->y;
  Am_Input_Char ic = ev->input_char;
  command_obj = inter.Get(Am_COMMAND);

  Am_Inter_Call_Both_Method(inter, command_obj, Am_START_DO_METHOD, x, y,
			    event_window, ic, object, Am_No_Location);

  if ((bool)inter.Get(Am_CONTINUOUS))
    //calls interim_do_method
    Choice_Handle_New_Old(command_obj, object, inter, x, y, event_window, ic);
  else { // not continuous, just call stop
    inter.Set(Am_INTERIM_VALUE, object);
    Am_Call_Final_Do_And_Register(inter, command_obj, x, y,
				  event_window, ic, object,
				  Am_No_Location, choice_set_impl_command);
  }
}


Am_Define_Method(Am_Inter_Internal_Method, void, Am_Choice_Abort_Method,
		 (Am_Object& inter, Am_Object& object, Am_Object& event_window,
		  Am_Input_Event *ev)) {
  Am_INTER_TRACE_PRINT(inter, "Choice Aborting");

  // first, call the prototype's method
  Am_Inter_Internal_Method inter_method;
  inter_method = Am_Interactor.Get(Am_INTER_ABORT_METHOD);
  inter_method.Call(inter, object, event_window, ev);

  // Now, call the correct Command method
  Am_Object command_obj;
  command_obj = inter.Get(Am_COMMAND);
  Am_Inter_Call_Both_Method(inter, command_obj, Am_ABORT_DO_METHOD,
		    ev->x, ev->y, event_window, ev->input_char, object, Am_No_Location);
}


Am_Define_Method(Am_Inter_Internal_Method, void, Am_Choice_Outside_Method,
		 (Am_Object& inter, Am_Object& object, Am_Object& event_window,
		  Am_Input_Event *ev)) {
  Am_INTER_TRACE_PRINT(inter, "Choice Outside");

  // first, call the prototype's method
  Am_Inter_Internal_Method inter_method;
  inter_method = Am_Interactor.Get(Am_INTER_OUTSIDE_METHOD);
  inter_method.Call(inter, object, event_window, ev);

  Am_Object command_obj;
  command_obj = inter.Get(Am_COMMAND);
  Choice_Handle_New_Old(command_obj, object, inter, ev->x, ev->y, event_window,
			ev->input_char);
}

// default Outside_Stop method is fine

Am_Define_Method(Am_Inter_Internal_Method, void, Am_Choice_Back_Inside_Method,
		 (Am_Object& inter, Am_Object& object,
		  Am_Object& event_window, Am_Input_Event *ev)) {
  Am_INTER_TRACE_PRINT(inter, "Choice back inside over " << object);
  // first, call the prototype's method
  Am_Inter_Internal_Method inter_method;
  inter_method = Am_Interactor.Get(Am_INTER_BACK_INSIDE_METHOD);
  inter_method.Call(inter, object, event_window, ev);

  Am_Object command_obj;
  command_obj = inter.Get(Am_COMMAND);
  Choice_Handle_New_Old(command_obj, object, inter, ev->x, ev->y, event_window,
			ev->input_char);
}

Am_Define_Method(Am_Inter_Internal_Method, void, Am_Choice_Running_Method,
		 (Am_Object& inter, Am_Object& object, Am_Object& event_window,
		  Am_Input_Event *ev)) {

  Am_INTER_TRACE_PRINT(inter, "Choice running over " << object);
  // first, call the prototype's method
  Am_Inter_Internal_Method inter_method;
  inter_method = Am_Interactor.Get(Am_INTER_RUNNING_METHOD);
  inter_method.Call(inter, object, event_window, ev);

  Am_Object command_obj;
  command_obj = inter.Get(Am_COMMAND);
  Choice_Handle_New_Old(command_obj, object, inter, ev->x, ev->y, event_window,
			ev->input_char);
}

Am_Define_Method(Am_Inter_Internal_Method, void, Am_Choice_Stop_Method,
		 (Am_Object& inter, Am_Object& object, Am_Object& event_window,
		  Am_Input_Event *ev)) {
  Am_INTER_TRACE_PRINT(inter, "Choice stopping over " << object);

  // First, call the prototype's method.  This is first so the
  // interactor is sure not to be running if the command's do
  // method calls pop-up or something and thus doesn't exit right away.
  Am_Inter_Internal_Method inter_method;
  inter_method = Am_Interactor.Get(Am_INTER_STOP_METHOD);
  inter_method.Call(inter, object, event_window, ev);

  Am_Object command_obj;
  command_obj = inter.Get(Am_COMMAND);
  Choice_Handle_New_Old(command_obj, object, inter, ev->x, ev->y,
			event_window, ev->input_char);
  Am_Call_Final_Do_And_Register(inter, command_obj, ev->x, ev->y,
				event_window, ev->input_char, object,
				Am_No_Location, choice_set_impl_command);

}

//////////////////////////////////////////////////////////////////////////////
// Default methods to make the Choice Interactor work
//////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG
void am_report_set_sel_value(bool selected_slot, Am_Object inter_or_cmd,
			  Am_Object obj, bool value) {
 std::cout << "++ Object " << inter_or_cmd << " setting ";
  if (selected_slot)std::cout << "Am_SELECTED";
  else std::cout << "Am_INTERIM_SELECTED";
 std::cout << " of " << obj << " to ";
  if (value)std::cout << "true\n" <<std::flush;
  else std::cout << "false\n" <<std::flush;
}

void report_set_value(Am_Object inter_or_cmd, Am_Slot_Key slot,
		      Am_Object obj, Am_Value value) {
  const char* name = Am_Get_Slot_Name(slot);
 std::cout << "++ Object " << inter_or_cmd << " setting ";
  if (name)std::cout << name;
  else std::cout << slot;
 std::cout << " of " << obj << " to " << value <<std::endl <<std::flush;
}

#else
#define Am_REPORT_SET_SEL_VALUE(selected_slot, inter_or_cmd, obj, value)
    /* if not debugging, define it to be nothing */
#endif

// before calling this, the new value, which should be a single
// object or 0, must be set into the Am_INTERIM_VALUE slot.
void Am_Choice_Set_Value (Am_Object inter, bool set_selected) {
  Am_Object new_value = inter.Get (Am_INTERIM_VALUE);
  Am_Choice_How_Set how_set = inter.Get (Am_HOW_SET);

  // now set real value

  // first check if slot contains correct type: single object or list
  Am_Value old_value;
  Am_Object old_value_obj;
  Am_Value_List old_value_list;
  old_value=inter.Peek(Am_VALUE);
  Am_Value_Type old_value_type = old_value.type;
  // if not valid, old_value_obj and old_value_list already are not valid
  if (old_value.Valid ()) {
    if (old_value_type == Am_OBJECT)
      old_value_obj = old_value;
    else if (Am_Value_List::Test (old_value))
      old_value_list = old_value;
    else {
     std::cerr << "** Amulet Error: Bad type ";
      Am_Print_Type(std::cerr, old_value_type);
     std::cerr << " in VALUE of inter " << inter << " = " << old_value
           <<std::endl <<std::flush;
      Am_Error ();
    }
  }

  //save current value as old in case needed for undo
  if (old_value_type == Am_OBJECT)
    inter.Set (Am_OLD_VALUE, old_value_obj, Am_OK_IF_NOT_THERE);
  else
    inter.Set (Am_OLD_VALUE, old_value_list, Am_OK_IF_NOT_THERE);

  if (how_set == Am_CHOICE_LIST_TOGGLE) {
    if (old_value_type == Am_OBJECT) {
      // used to be a single value, might become a list
      if (old_value_obj == new_value) {
        if (set_selected && old_value_obj.Valid ()) {
          Am_REPORT_SET_SEL_VALUE (true, inter, old_value_obj, false);
          old_value_obj.Set (Am_SELECTED, false, Am_OK_IF_NOT_THERE); // and leave new list empty
        }
      }
      else {  // old value is a list
        // put them both in the new list
        if (old_value_obj.Valid ())
          old_value_list.Add (old_value_obj);
        if (new_value.Valid ()) {
          old_value_list.Add (new_value);
          if (set_selected) {
	    Am_REPORT_SET_SEL_VALUE (true, inter, new_value, true);
	    new_value.Set (Am_SELECTED, true, Am_OK_IF_NOT_THERE);
	  }
        }
      }
      //put new list into slot.
      inter.Set (Am_VALUE, old_value_list);
    }
    else {
      // originally a list, will stay a list or get cleared
      old_value_list.Start (); // initialize list for Member or for loop
      if (!new_value.Valid ())
	{} /*  Don't do anything *********
	       // { // clear out list
	       // // clear all the SELECTED slots of the individual objects
	       // for ( ; !old_value_list.Last (); old_value_list.Next ()) {
	       // old_value_obj = old_value_list.Get ();
	       // Am_REPORT_SET_SEL_VALUE (true, inter, old_value_obj, false);
	       // old_value_obj.Set (Am_SELECTED, false);
	       // }
	       // // forget about the list; just make it an empty object.
	       // inter.Set (Am_VALUE, new_value);
	       // } // end of if (!new_value)
      */
      // old_value_list exists, new_value exists
      else if (old_value_list.Member (new_value)) {
	if (set_selected) {
	  Am_REPORT_SET_SEL_VALUE (true, inter, new_value, false);
	  new_value.Set (Am_SELECTED, false, Am_OK_IF_NOT_THERE);
	}
        old_value_list.Delete ();
        if (old_value_list.Empty ())
          // no objects left in list, so don't store empty list
          inter.Set(Am_VALUE, Am_No_Value_List);
        else
          inter.Set (Am_VALUE, old_value_list);
      }
      else { // add it at end
        old_value_list.Add (new_value);
	if (set_selected) {
	  Am_REPORT_SET_SEL_VALUE (true, inter, new_value, true);
	  new_value.Set (Am_SELECTED, true, Am_OK_IF_NOT_THERE);
	}
        inter.Set (Am_VALUE, old_value_list);
      }
    }
  } // end of how_set == List toggle
  else { // single value
    if (old_value_type == Am_OBJECT) {
      // single new value, single old value
      if (how_set == Am_CHOICE_SET) {
        if (set_selected && old_value_obj.Valid ()) {
	  Am_REPORT_SET_SEL_VALUE (true, inter, old_value_obj, false);
	  old_value_obj.Set (Am_SELECTED, false, Am_OK_IF_NOT_THERE);
        }
        if (set_selected && new_value.Valid ()) {
          Am_REPORT_SET_SEL_VALUE (true, inter, new_value, true);
          new_value.Set (Am_SELECTED, true, Am_OK_IF_NOT_THERE);
        }
        inter.Set (Am_VALUE, new_value); // put new value into slot
      }
      else if (how_set == Am_CHOICE_CLEAR) {
        if (set_selected && old_value_obj.Valid ()) {
          Am_REPORT_SET_SEL_VALUE (true, inter, old_value_obj, false);
          old_value_obj.Set (Am_SELECTED, false, Am_OK_IF_NOT_THERE);
        }
        inter.Set (Am_VALUE, Am_No_Object); // empty
      }
      else if (how_set == Am_CHOICE_TOGGLE) {
        if (old_value_obj == new_value) {
          if (set_selected && old_value_obj.Valid ()) {
            Am_REPORT_SET_SEL_VALUE (true, inter, old_value_obj, false);
            old_value_obj.Set (Am_SELECTED, false, Am_OK_IF_NOT_THERE);
          }
          inter.Set (Am_VALUE, Am_No_Object); // empty
        }
        else { // set with new value
          if (set_selected && old_value_obj.Valid ()) {
            Am_REPORT_SET_SEL_VALUE (true, inter, old_value_obj, false);
            old_value_obj.Set (Am_SELECTED, false, Am_OK_IF_NOT_THERE);
          }
          if (set_selected && new_value.Valid ()) {
            Am_REPORT_SET_SEL_VALUE (true, inter, new_value, true);
            new_value.Set (Am_SELECTED, true, Am_OK_IF_NOT_THERE);
          }
          inter.Set (Am_VALUE, new_value); // put new value into slot
        }
      }
      else {
       std::cerr << "** Amulet Error: Illegal how_set " << how_set <<std::endl;
        Am_Error();
      }
    }
    else {
      // used to be multiple values, now is just one
      old_value_list.Start (); // set pointer back to the beginning
      // see if new object should NOT be in the set
      if (new_value.Valid ()) {
        if ((how_set == Am_CHOICE_CLEAR) ||
            (how_set == Am_CHOICE_LIST_TOGGLE &&
             old_value_list.Member (new_value)))
          new_value = Am_No_Object;
        else {
	  if (set_selected) {
	    // item must be selected if not previously in list or being cleared
	    Am_REPORT_SET_SEL_VALUE(true, inter, new_value, true);
	    new_value.Set(Am_SELECTED, true, Am_OK_IF_NOT_THERE);
	  }
        }
      }
      for (old_value_list.Start (); !old_value_list.Last ();
           old_value_list.Next ()) {
        old_value_obj = old_value_list.Get ();
        if (set_selected && old_value_obj != new_value) {
          Am_REPORT_SET_SEL_VALUE(true, inter, old_value_obj, false);
	  //clear all the other values
          old_value_obj.Set(Am_SELECTED, false, Am_OK_IF_NOT_THERE);
        }
      }
      //put new value into slot. Will automatically deallocate old_value_list
      inter.Set(Am_VALUE, new_value);
    } // else single old value
  } // else single new value
} //Am_Choice_Set_Value

// called when start continuous
Am_Define_Method(Am_Object_Method, void, Am_Choice_Start_Do,
		 (Am_Object inter)) {
  inter.Set(Am_OLD_INTERIM_VALUE, 0);
  inter.Set(Am_INTERIM_VALUE, 0);
}

//current object should be already set into Am_INTERIM_VALUE and old
//value in Am_OLD_INTERIM_VALUE
Am_Define_Method(Am_Object_Method, void, Am_Choice_Interim_Do,
		 (Am_Object inter)) {
  Am_Object old_object, new_object;
  old_object = inter.Get(Am_OLD_INTERIM_VALUE);
  new_object = inter.Get(Am_INTERIM_VALUE);
  bool set_selected = inter.Get(Am_SET_SELECTED);
  if (set_selected) {
    if (old_object.Valid ()) {
      Am_REPORT_SET_SEL_VALUE(false, inter, old_object, false);
      old_object.Set(Am_INTERIM_SELECTED, false, Am_OK_IF_NOT_THERE);
    }
    if (new_object.Valid ()) {
      Am_REPORT_SET_SEL_VALUE(false, inter, new_object, true);
      new_object .Set(Am_INTERIM_SELECTED, true, Am_OK_IF_NOT_THERE);
    }
  }
}

Am_Define_Method(Am_Object_Method, void, Am_Choice_Abort_Do,
		 (Am_Object inter)) {
  Am_Object obj;
  obj = inter.Get(Am_OLD_INTERIM_VALUE);
  bool set_selected = inter.Get(Am_SET_SELECTED);
  if (set_selected) {
    if (obj.Valid ()) {
      Am_REPORT_SET_SEL_VALUE(false, inter, obj, false);
      obj.Set(Am_INTERIM_SELECTED, false, Am_OK_IF_NOT_THERE);
    }
    obj = inter.Get(Am_INTERIM_VALUE);
    if (obj.Valid ()) {
      Am_REPORT_SET_SEL_VALUE(false, inter, obj, false);
      obj .Set(Am_INTERIM_SELECTED, false, Am_OK_IF_NOT_THERE);
    }
  }
}

// Interim_Do should have already been called if new_object different
// from old object, so don't have to interim erase old object.
// Before calling this, the new value, which should be a single
// object or 0, must be set into the Am_INTERIM_VALUE slot.
Am_Define_Method(Am_Object_Method, void, Am_Choice_Do,
		 (Am_Object inter)) {
  Am_Object new_object;
  new_object = inter.Get(Am_INTERIM_VALUE);
  bool set_selected = inter.Get(Am_SET_SELECTED);
  if (set_selected) {
    if(new_object.Valid ()) {
      Am_REPORT_SET_SEL_VALUE(false, inter, new_object, false);
      new_object.Set(Am_INTERIM_SELECTED, false, Am_OK_IF_NOT_THERE);
    }
  }
  Am_Choice_Set_Value(inter, set_selected);
  inter.Set(Am_OBJECT_MODIFIED, new_object);
  Am_Copy_Values_To_Command(inter);
}

void choice_general_undo_redo(Am_Object command_obj, bool undo,
			      bool selective, bool reload_data,
			      Am_Value sel_value) {
  Am_Object inter;
  inter = command_obj.Get(Am_SAVED_OLD_OWNER);
  bool set_selected = inter.Get(Am_SET_SELECTED);

  #ifdef DEBUG
  if (inter.Valid () && Am_Inter_Tracing(inter)) {
    if (selective)std::cout << "Selective ";
    if (undo) std::cout << "Undo"; else std::cout << "Repeat";
   std::cout << " of command " << command_obj <<std::endl <<std::flush;
  }
  #endif
  // Go through current value and unset any SELECTED slots and then set any
  // in the old value

  // first clear SELECTED for the current values
  Am_Value new_value, old_value;
  Am_Object new_value_obj, old_value_obj;
  Am_Value_List new_value_list, old_value_list;

  if (selective) { // then get current value from the interactor
    if (inter.Valid()) new_value=inter.Peek(Am_VALUE);
  }
  else // get current value from the command_obj
    new_value=command_obj.Peek(Am_VALUE);

  if (undo) old_value=command_obj.Peek(Am_OLD_VALUE);
  else { // repeat
    //set to new obj or current value of command
    if (reload_data) old_value = sel_value;
    else old_value=command_obj.Peek(Am_VALUE);
  }

  Am_Value_Type typ = new_value.type;

  if (!new_value.Valid()) ; //don't do anything if not Valid
  else if (typ == Am_OBJECT) {
    new_value_obj = new_value;
    // have to check since might be a null object
    if(set_selected && new_value_obj.Valid ()) {
      Am_REPORT_SET_SEL_VALUE(true, inter, new_value_obj, false);
      new_value_obj.Set(Am_SELECTED, false, Am_OK_IF_NOT_THERE);
    }
  }
  else if (Am_Value_List::Test(new_value.value.wrapper_value)) {
    new_value_list = new_value;
    for (new_value_list.Start (); !new_value_list.Last ();
	 new_value_list.Next ()) {
      new_value_obj = new_value_list.Get ();
      if (set_selected) {
	Am_REPORT_SET_SEL_VALUE(true, inter, new_value_obj, false);
	//clear all current values
	new_value_obj.Set(Am_SELECTED, false, Am_OK_IF_NOT_THERE);
      }
    }
  }
  else {
   std::cerr << "** Amulet Error: Bad type ";
    Am_Print_Type(std::cerr, typ);
   std::cerr << " in VALUE of command " << command_obj
	 <<std::endl;
    Am_Error();
  }

  // now set SELECTED for the old_value_objs
  typ = old_value.type;

  if (set_selected) {
    if (!old_value.Valid())
      old_value_obj = Am_No_Object; //all invalids map to zero
    else if (typ == Am_OBJECT) {
      old_value_obj = old_value;
      if(old_value_obj.Valid ()) {
	//have to check valid since might be a null object
	Am_REPORT_SET_SEL_VALUE(true, inter, old_value_obj, true);
	old_value_obj.Set(Am_SELECTED, true, Am_OK_IF_NOT_THERE);
      }
    }
//// BUG: Need to move add_ref to setter to avoid this annoyance
    else if (Am_Value_List::Test(old_value.value.wrapper_value)) { //then is a list
      old_value_list = old_value;
      for (old_value_list.Start (); !old_value_list.Last ();
	   old_value_list.Next ()) {
	old_value_obj = old_value_list.Get ();
	Am_REPORT_SET_SEL_VALUE(true, inter, old_value_obj, true);
	old_value_obj.Set(Am_SELECTED, true, Am_OK_IF_NOT_THERE);  //set all old values
      }
    }
    else {
     std::cerr << "** Amulet Error: Bad type ";
      Am_Print_Type(std::cerr, typ);
     std::cerr << " in OLD_VALUE of command " << command_obj
	   <<std::endl;
      Am_Error();
    }
  }

  // swap current and old values, in case undo or undo-the-undo again.

  #ifdef DEBUG
  if (Am_Inter_Tracing(Am_INTER_TRACE_SETTING)) {
    report_set_value(inter, Am_OLD_VALUE, command_obj, new_value);
    report_set_value(inter, Am_VALUE, command_obj, old_value);
    report_set_value(inter, Am_VALUE, inter, old_value);
  }
  #endif
  command_obj.Set(Am_OLD_VALUE, new_value);
  command_obj.Set(Am_VALUE, old_value);
  command_obj.Set(Am_OBJECT_MODIFIED, old_value);
  // make current interactor have the correct current value
  if (inter.Valid())
    inter.Set(Am_VALUE, old_value);
}

//////////////////////////////////////////////////////////////////////////////
// Methods for the Internal Command object
//////////////////////////////////////////////////////////////////////////////

Am_Define_Method(Am_Object_Method, void, Am_Choice_Command_Undo,
		 (Am_Object command_obj)) {
  choice_general_undo_redo(command_obj, true, false, false, (0L));
}

//command obj is a copy of the original, so can modify it
Am_Define_Method(Am_Object_Method, void, Am_Choice_Command_Selective_Undo,
		 (Am_Object command_obj)) {
  choice_general_undo_redo(command_obj, true, true, false, (0L));
}

//command obj is a copy of the original, so can modify it
Am_Define_Method(Am_Object_Method, void,
		 Am_Choice_Command_Repeat_Same, (Am_Object command_obj)){
  choice_general_undo_redo(command_obj, false, true, false, (0L));
}

//new value is supplied is ignored
Am_Define_Method(Am_Selective_Repeat_New_Method, void,
		 Am_Choice_Command_Repeat_New,
		 (Am_Object command_obj, Am_Value new_selection,
		  Am_Value /* new_val */)){
  choice_general_undo_redo(command_obj, false, true, true, new_selection);
}

//this one repeats on the same object even though says new.  This is
//used by button widgets.
Am_Define_Method(Am_Selective_Repeat_New_Method, void,
		 Am_Choice_Command_Repeat_New_As_Same,
		 (Am_Object command_obj, Am_Value /* new_selection */,
		  Am_Value /* new_val */)){
  choice_general_undo_redo(command_obj, false, true, false, (0L));
}

////////////////////////////////////////////////////////////

// Used in to make the running_where_test be the same as
// the start_where_test.  Use value since different types returned
Am_Define_Formula (Am_Value, same_as_start_where_test) {
  return self.Peek(Am_START_WHERE_TEST);
}

//global variables
Am_Object Am_Choice_Internal_Command;

Am_Object Am_Choice_Interactor;  // choosing one from a set
Am_Object Am_One_Shot_Interactor;  // does something at input event
Am_Object Am_Choice_Interactor_Repeat_Same;

static void init() {

  Am_Choice_Internal_Command = Am_Command.Create(DSTR("Am_Choice_Command"))
    .Set (Am_LABEL, "Choice")
    .Set (Am_UNDO_METHOD, Am_Choice_Command_Undo)
    .Set (Am_REDO_METHOD, Am_Choice_Command_Undo)//undo works for both
    .Set (Am_SELECTIVE_UNDO_METHOD, Am_Choice_Command_Selective_Undo)
    .Set (Am_SELECTIVE_REPEAT_SAME_METHOD, Am_Choice_Command_Repeat_Same)
    .Set (Am_SELECTIVE_REPEAT_ON_NEW_METHOD, Am_Choice_Command_Repeat_New)
    ;

    Am_Choice_Interactor = Am_Interactor.Create(DSTR("Am_Choice_Interactor"))
     .Set (Am_INTER_START_METHOD, Am_Choice_Start_Method)
     .Set (Am_INTER_ABORT_METHOD, Am_Choice_Abort_Method)
     .Set (Am_INTER_OUTSIDE_METHOD, Am_Choice_Outside_Method)
     .Set (Am_INTER_BACK_INSIDE_METHOD, Am_Choice_Back_Inside_Method)
     .Set (Am_INTER_RUNNING_METHOD, Am_Choice_Running_Method)
     .Set (Am_INTER_STOP_METHOD, Am_Choice_Stop_Method)

     .Set (Am_START_DO_METHOD, Am_Choice_Start_Do)
     .Set (Am_INTERIM_DO_METHOD, Am_Choice_Interim_Do)
     .Set (Am_ABORT_DO_METHOD, Am_Choice_Abort_Do)
     .Set (Am_DO_METHOD, Am_Choice_Do)

     .Add (Am_FIRST_ONE_ONLY, false) // whether menu or button like
     .Set (Am_RUNNING_WHERE_TEST, same_as_start_where_test)
     .Add (Am_HOW_SET, Am_CHOICE_TOGGLE)
     .Add (Am_SET_SELECTED, true)
     .Add_Part(Am_IMPLEMENTATION_COMMAND,
	       Am_Choice_Internal_Command.Create(DSTR("Choice_Internal_Command")))
     ;
  Am_Choice_Interactor.Set_Part(Am_COMMAND,
		     Am_Select_Command.Create(DSTR("Am_Command_in_Choice"))
				.Set (Am_LABEL, "Choice")
				);

  Am_One_Shot_Interactor=Am_Choice_Interactor.Create(DSTR("Am_One_Shot_Interactor"))
    .Set (Am_START_WHERE_TEST, Am_Inter_In)
    .Set (Am_CONTINUOUS, false);
  //change all the names
    Am_One_Shot_Interactor.Get_Object(Am_COMMAND)
  .Set (Am_LABEL, "One_Shot")
  .Set_Name (DSTR("Am_Command_in_One_Shot"))
  ;

  Am_One_Shot_Interactor.Get_Object(Am_IMPLEMENTATION_COMMAND)
    .Set (Am_LABEL, "One_Shot")
    .Set_Name (DSTR("One_Shot_Internal_Command"))
    ;
  Am_Choice_Interactor_Repeat_Same =
    Am_Choice_Interactor.Create(DSTR("Choice_Interactor_Repeat_Same"));
  Am_Choice_Interactor_Repeat_Same.Get_Object(Am_IMPLEMENTATION_COMMAND)
    .Set(Am_SELECTIVE_REPEAT_ON_NEW_METHOD,
	 Am_Choice_Command_Repeat_New_As_Same);
}

static Am_Initializer* initializer = new Am_Initializer(DSTR("Inter_Choice"), init, 3.3f);
