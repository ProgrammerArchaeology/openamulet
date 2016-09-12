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

// This file contains the functions for handling the Am_New_Points_Interactor
//
// Designed and implemented by Brad Myers

#include <am_inc.h>

#include <amulet/am_io.h>

#include <amulet/inter_advanced.h>
#include <amulet/standard_slots.h>
#include <amulet/object_advanced.h> // for Am_Slot_Advanced for demons
#include <math.h>           // needed for sqrt for min-length of lines
//math.h is in /usr/local/lib/gcc-lib/hppa1.1-hp-hpux/2.6.0/include/

#include <amulet/opal.h> // needed for translate_coordinates
#include <amulet/registry.h>
#include <amulet/initializer.h>

//////////////////////////////////////////////////////////////////
/// Helper functions
//////////////////////////////////////////////////////////////////

inline int
IABS(int a)
{
  return (a < 0) ? -a : a;
}

void
calc_rect_two_points(Am_Object &inter, bool first_point, int x, int y,
                     bool &abort, int &out_x, int &out_y, int &out_width,
                     int &out_height)
{
  int min_width = inter.Get(Am_MINIMUM_WIDTH);
  int min_height = inter.Get(Am_MINIMUM_HEIGHT);
  bool flip = inter.Get(Am_FLIP_IF_CHANGE_SIDES);
  bool want_abort = inter.Get(Am_ABORT_IF_TOO_SMALL);

  abort = false;

  if (first_point) {
    out_x = x;
    out_y = y;
    // Must reset first_x and first_y because gridding may change x and y.
    inter.Set(Am_FIRST_X, x);
    inter.Set(Am_FIRST_Y, y);
    out_width = min_width; //min_width = 0 means no minimum
    out_height = min_height;
    if (want_abort && (min_width || min_height)) {
      Am_INTER_TRACE_PRINT(inter, "-- rectangle too small");
      abort = true;
    }
  } else { //second point
    int first_x = inter.Get(Am_FIRST_X);
    int first_y = inter.Get(Am_FIRST_Y);
    if (flip) { // then make sure width and height are positive
      out_width = IABS(x - first_x);
      out_height = IABS(y - first_y);
    } else { // use actual width and height
      out_width = (x - first_x);
      out_height = (y - first_y);
    }
    if (want_abort) {
      if (out_width < min_width || out_height < min_height) {
        Am_INTER_TRACE_PRINT(inter, "-- rectangle too small");
        abort = true;
      }
    } else { // don't want_abort
      if (out_width < min_width)
        out_width = min_width;
      if (out_height < min_height)
        out_height = min_height;
    }
    // now calculate out_x and out_y
    if (flip) { // then might have to change first and second
      if (x < first_x)
        out_x = first_x - out_width;
      else
        out_x = first_x;
      if (y < first_y)
        out_y = first_y - out_height;
      else
        out_y = first_y;
    } else {
      out_x = first_x;
      out_y = first_y;
    }
  }
}

void
calc_line_two_points(Am_Object &inter, bool first_point, int x, int y,
                     bool &abort, int &out_x1, int &out_y1, int &out_x2,
                     int &out_y2)
{
  int min_length = inter.Get(Am_MINIMUM_LENGTH);
  bool want_abort = inter.Get(Am_ABORT_IF_TOO_SMALL);

  abort = false;

  if (first_point) {
    out_x1 = x;
    out_y1 = y;
    out_x2 = x + min_length; //by default, line is horizontal
    out_y2 = y;
    if (want_abort && min_length) {
      Am_INTER_TRACE_PRINT(inter, "-- line too small");
      abort = true;
    }
  } else { // second point
    Am_Inter_Location last_points = inter.Get(Am_INTERIM_VALUE);
    int first_x, first_y, c, d;
    last_points.Get_Points(first_x, first_y, c, d);
    out_x1 = first_x;
    out_y1 = first_y;
    out_x2 = x; // these may be overwritten below if required by min_length
    out_y2 = y;
    if (min_length) { // time for expensive math
      double x_dist = x - first_x;
      double y_dist = y - first_y;
      double denom = sqrt((x_dist * x_dist) + (y_dist * y_dist));
      if (denom < (double)min_length) {
        if (want_abort) {
          Am_INTER_TRACE_PRINT(inter, "-- line too small");
          abort = true;
        }
        if (denom == 0.0) { // don't divide by zero
          out_x2 = first_x + min_length;
          out_y2 = first_y;
        } else { // not zero, create a min_length line in correct direction
          out_x2 = (int)(first_x + ((x_dist * min_length) / denom));
          out_y2 = (int)(first_y + ((y_dist * min_length) / denom));
        }
      }
      // else using x,y as set above is fine
    }
  }
}

// Main routine, that picks the correct procedure above to calc the points.
// Sets the number fields of data.  RefObj in data should be already
// set, but it is ignored by these routines because not relevant
void
new_points_calc_all(Am_Object &inter, Am_Object &ref_obj, bool first_point,
                    int x, int y, bool &abort, Am_Inter_Location &data)
{
  bool as_line = inter.Get(Am_AS_LINE);
  int a, b, c, d;
  if (first_point)
    inter.Set(Am_INITIAL_REF_OBJECT, ref_obj);
  else {
    Am_Object init_ref_obj = inter.Get(Am_INITIAL_REF_OBJECT);
    if (init_ref_obj != ref_obj) {
      Am_Translate_Coordinates(ref_obj, x, y, init_ref_obj, x, y);
      ref_obj = init_ref_obj;
    }
  }

  if (as_line)
    calc_line_two_points(inter, first_point, x, y, abort, a, b, c, d);
  else
    calc_rect_two_points(inter, first_point, x, y, abort, a, b, c, d);
  data.Set_Location(as_line, ref_obj, a, b, c, d, false);
  if (first_point)
    data.Set_Growing(false);
}

void
initialize_interim_value_and_feedback(Am_Object inter)
{
  Am_Inter_Location inter_data;
  if (Am_Type_Class(inter.Get_Slot_Type(Am_INTERIM_VALUE)) == Am_WRAPPER) {
    inter.Make_Unique(Am_INTERIM_VALUE);
  } else {
    //set up with some temporary values
    inter_data.Set_Location(false, Am_No_Object, 0, 0, 0, 0);
    inter.Set(Am_INTERIM_VALUE, inter_data);
  }

  Am_Object feedback = inter.Get(Am_FEEDBACK_OBJECT);
  Am_Object new_owner;
  if (feedback.Valid()) {
#ifdef DEBUG
    //test for a frequent bug:
    if (!feedback.Get_Owner().Valid() || !feedback.Get(Am_WINDOW).Valid()) {
      std::cout << "** AMULET WARNING: feedback object " << feedback
                << " in interactor " << inter
                << " does not seem to be in a group or window,"
                << " so it won't be visible\n"
                << std::flush;
    }
#endif
    bool feedback_is_window = feedback.Is_Instance_Of(Am_Window);
    new_owner = Am_Check_And_Fix_Feedback_Group(feedback, inter);
    if (feedback_is_window)
      Am_Add_Remove_Inter_To_Extra_Window(inter, feedback, true, true);
  }
  if (new_owner.Valid())
    inter.Set(Am_SAVED_OLD_OWNER, new_owner, Am_OK_IF_NOT_THERE);
  else
    inter.Set(Am_SAVED_OLD_OWNER, inter.Get_Owner(), Am_OK_IF_NOT_THERE);
}

Am_Inter_Location
new_points_interim_val(Am_Object command_obj, bool first_point, Am_Object inter,
                       int x, int y, Am_Object ref_obj, Am_Input_Char ic)
{
  // interim_value initialized in Am_New_Points_Start_Do
  Am_Inter_Location data(inter.Get(Am_INTERIM_VALUE));
  //don't need Make_Unique here because will have been done by
  // initialize_interim_value_and_feedback

  //first, translate the coordinates to be w.r.t. inter's owner
  Am_Object owner = inter.Get(Am_SAVED_OLD_OWNER);
  if (owner.Valid()) {
    Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING, "Translating coordinates from "
                                                     << ref_obj << " to "
                                                     << owner);
    Am_Translate_Coordinates(ref_obj, x, y, owner, x, y);
    ref_obj = owner;
  }

  // set x,y based on gridding
  Am_Get_Filtered_Input(inter, ref_obj, x, y, x, y);
  bool abort;
  new_points_calc_all(inter, ref_obj, first_point, x, y, abort, data);
  inter.Set(Am_TOO_SMALL, abort);
  inter.Note_Changed(Am_INTERIM_VALUE);

  Am_Inter_Call_Both_Method(inter, command_obj, Am_INTERIM_DO_METHOD, x, y,
                            ref_obj, ic, Am_No_Object, data);
  return data;
}

Am_Object
new_points_set_impl_command(Am_Object inter, Am_Object /* object_modified */,
                            Am_Inter_Location data)
{
  Am_Object impl_command, main_command, new_object, start_object;
  impl_command = inter.Get_Object(Am_IMPLEMENTATION_COMMAND);
  if (impl_command.Valid()) {
    new_object = inter.Get(Am_VALUE);
    start_object = new_object.Copy();
    impl_command.Set(Am_OBJECT_MODIFIED, new_object);
    impl_command.Set(Am_VALUE, new_object);
    impl_command.Set(Am_START_OBJECT, start_object);
    impl_command.Set(Am_AS_LINE, (bool)inter.Get(Am_AS_LINE));
    impl_command.Set(Am_INTERIM_VALUE, data);
    impl_command.Set(Am_HAS_BEEN_UNDONE, false);
    impl_command.Set(Am_HOW_MANY_POINTS, inter.Get(Am_HOW_MANY_POINTS),
                     Am_OK_IF_NOT_THERE);
  }
  main_command = inter.Peek(Am_COMMAND);
  if (main_command.Valid())
    main_command.Set(Am_START_OBJECT, start_object, Am_OK_IF_NOT_THERE);
  return impl_command;
}

//////////////////////////////////////////////////////////////////
/// Main method routines
//////////////////////////////////////////////////////////////////

Am_Define_Method(Am_Inter_Internal_Method, void, Am_New_Points_Start_Method,
                 (Am_Object & inter, Am_Object &object, Am_Object &event_window,
                  Am_Input_Event *ev))
{
  int x = ev->x;
  int y = ev->y;
  Am_Input_Char ic = ev->input_char;

  Am_INTER_TRACE_PRINT(inter, "New_Points starting over " << object);

  // first, call the prototype's method
  Am_Inter_Internal_Method inter_method;
  inter_method = Am_Interactor.Get(Am_INTER_START_METHOD);
  inter_method.Call(inter, object, event_window, ev);

  int num_points = inter.Get(Am_HOW_MANY_POINTS);
  if (num_points < 1 || num_points > 2) {
    std::cerr << "** Amulet Error: Number of points for " << inter << " is "
              << num_points << " but only 1 or 2 are implemented\n";
    Am_Error();
  }

  Am_Object command_obj;
  command_obj = inter.Get(Am_COMMAND);

  initialize_interim_value_and_feedback(inter);
  //call start method
  Am_Inter_Call_Both_Method(inter, command_obj, Am_START_DO_METHOD, x, y,
                            event_window, ic, Am_No_Object, Am_No_Location);

  Am_Inter_Location data =
      new_points_interim_val(command_obj, true, inter, x, y, event_window, ic);

  if (!(bool)inter.Get(Am_CONTINUOUS)) { // not continuous, call stop
    Am_Call_Final_Do_And_Register(inter, command_obj, x, y, event_window, ic,
                                  Am_No_Object, data,
                                  new_points_set_impl_command);
  }
}

Am_Define_Method(Am_Inter_Internal_Method, void, Am_New_Points_Abort_Method,
                 (Am_Object & inter, Am_Object &object, Am_Object &event_window,
                  Am_Input_Event *ev))
{
  Am_INTER_TRACE_PRINT(inter, "New_Points Aborting");

  // first, call the prototype's method
  Am_Inter_Internal_Method inter_method;
  inter_method = Am_Interactor.Get(Am_INTER_ABORT_METHOD);
  inter_method.Call(inter, object, event_window, ev);

  // Now, call the correct Command method
  Am_Object command_obj;
  command_obj = inter.Get(Am_COMMAND);
  Am_Inter_Call_Both_Method(inter, command_obj, Am_ABORT_DO_METHOD, ev->x,
                            ev->y, event_window, ev->input_char, Am_No_Object,
                            Am_No_Location);
}

Am_Define_Method(Am_Inter_Internal_Method, void, Am_New_Points_Outside_Method,
                 (Am_Object & inter, Am_Object &object, Am_Object &event_window,
                  Am_Input_Event *ev))
{

  Am_INTER_TRACE_PRINT(inter, "New_Points Outside");

  // first, call the prototype's method
  Am_Inter_Internal_Method inter_method;
  inter_method = Am_Interactor.Get(Am_INTER_OUTSIDE_METHOD);
  inter_method.Call(inter, object, event_window, ev);

  Am_Object command_obj;
  command_obj = inter.Get(Am_COMMAND);
  Am_Inter_Call_Both_Method(inter, command_obj, Am_ABORT_DO_METHOD, ev->x,
                            ev->y, event_window, ev->input_char, Am_No_Object,
                            Am_No_Location);
}

Am_Define_Method(Am_Inter_Internal_Method, void,
                 Am_New_Points_Back_Inside_Method,
                 (Am_Object & inter, Am_Object &object, Am_Object &event_window,
                  Am_Input_Event *ev))
{
  int x = ev->x;
  int y = ev->y;
  Am_Input_Char ic = ev->input_char;

  Am_INTER_TRACE_PRINT(inter, "New_Points back inside over " << object);
  // first, call the prototype's method
  Am_Inter_Internal_Method inter_method;
  inter_method = Am_Interactor.Get(Am_INTER_BACK_INSIDE_METHOD);
  inter_method.Call(inter, object, event_window, ev);

  Am_Object command_obj;
  command_obj = inter.Get(Am_COMMAND);

  //call start method
  Am_Inter_Call_Both_Method(inter, command_obj, Am_START_DO_METHOD, x, y,
                            event_window, ic, Am_No_Object, Am_No_Location);

  bool first_point = (int)inter.Get(Am_HOW_MANY_POINTS) == 1;
  Am_Inter_Location data = new_points_interim_val(
      command_obj, first_point, inter, x, y, event_window, ic);
}

Am_Define_Method(Am_Inter_Internal_Method, void, Am_New_Points_Running_Method,
                 (Am_Object & inter, Am_Object &object, Am_Object &event_window,
                  Am_Input_Event *ev))
{
  int x = ev->x;
  int y = ev->y;
  Am_INTER_TRACE_PRINT(inter, "New_Points running over " << object);
  // first, call the prototype's method
  Am_Inter_Internal_Method inter_method;
  inter_method = Am_Interactor.Get(Am_INTER_RUNNING_METHOD);
  inter_method.Call(inter, object, event_window, ev);

  Am_Object command_obj;
  command_obj = inter.Get(Am_COMMAND);
  bool first_point = (int)inter.Get(Am_HOW_MANY_POINTS) == 1;
  Am_Inter_Location data = new_points_interim_val(
      command_obj, first_point, inter, x, y, event_window, ev->input_char);
}

Am_Define_Method(Am_Inter_Internal_Method, void, Am_New_Points_Stop_Method,
                 (Am_Object & inter, Am_Object &object, Am_Object &event_window,
                  Am_Input_Event *ev))
{
  int x = ev->x;
  int y = ev->y;
  Am_Input_Char ic = ev->input_char;
  Am_INTER_TRACE_PRINT(inter, "New_Points stopping over " << object);

  Am_Object command_obj;
  command_obj = inter.Get(Am_COMMAND);
  bool first_point = (int)inter.Get(Am_HOW_MANY_POINTS) == 1;
  //call interim_do on last point
  Am_Inter_Location data = new_points_interim_val(
      command_obj, first_point, inter, x, y, event_window, ic);

  if ((bool)inter.Get(Am_TOO_SMALL)) {
    // if TOO_SMALL is set, then both ABORT_IF_TOO_SMALL is true and
    // the new object actually is too small; hence we should abort
    Am_Inter_Internal_Method method;
    Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SHORT, "Aborting " << inter);
    method = inter.Get(Am_INTER_ABORT_METHOD);
    method.Call(inter, object, event_window, ev);
    return;
  }

  // now call stop
  Am_Call_Final_Do_And_Register(inter, command_obj, x, y, event_window, ic,
                                Am_No_Object, data,
                                new_points_set_impl_command);

  // LAST, call the prototype's method
  Am_Inter_Internal_Method inter_method;
  inter_method = Am_Interactor.Get(Am_INTER_STOP_METHOD);
  inter_method.Call(inter, object, event_window, ev);
}

//////////////////////////////////////////////////////////////////////////////
// Default object to make the New_Points Interactor work
//////////////////////////////////////////////////////////////////////////////

Am_Define_Method(Am_Object_Method, void, Am_New_Points_Start_Do,
                 (Am_Object inter))
{
  Am_Object feedback;
  feedback = inter.Get(Am_FEEDBACK_OBJECT);
  //feedback initialized in initialize_interim_value_and_feedback so
  //don't need to do it here
  if (feedback.Valid()) {
    Am_REPORT_VIS_CONDITION(Am_INTER_TRACE_SETTING, inter, feedback, true);
    feedback.Set(Am_VISIBLE, true);
    Am_To_Top(feedback);
    // position of feedback will be set by interim_do, called next
  }
}

//Modifies the Am_FEEDBACK_OBJECT
Am_Define_Method(Am_Current_Location_Method, void, Am_New_Points_Interim_Do,
                 (Am_Object inter, Am_Object /* object_modified */,
                  Am_Inter_Location data))
{
  Am_Object feedback;
  feedback = inter.Get(Am_FEEDBACK_OBJECT);
  if (feedback.Valid()) {
    bool feedback_vis = feedback.Get(Am_VISIBLE);
    bool too_small = inter.Get(Am_TOO_SMALL);
    if (too_small) {
      if (feedback_vis) {
        Am_REPORT_VIS_CONDITION(Am_INTER_TRACE_SETTING, inter, feedback, false);
        feedback.Set(Am_VISIBLE, false);
      }
    } else {
      if (feedback_vis == false) {
        Am_REPORT_VIS_CONDITION(Am_INTER_TRACE_SETTING, inter, feedback, true);
        feedback.Set(Am_VISIBLE, true);
      }
      // Am_Modify_Object_Pos is defined in inter_move_grow.cc
      Am_Modify_Object_Pos(feedback, data, true);
      inter.Set(Am_INTERIM_VALUE, data);
    }
  } else {
    Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING,
                         "++Create object inter "
                             << inter
                             << " running, but no feedback object to set");
  }
}

Am_Define_Method(Am_Object_Method, void, Am_New_Points_Abort_Do,
                 (Am_Object inter))
{
  Am_Object feedback;
  feedback = inter.Get(Am_FEEDBACK_OBJECT);
  if (feedback.Valid()) {
    Am_REPORT_VIS_CONDITION(Am_INTER_TRACE_SETTING, inter, feedback, false);
    if (feedback.Is_Instance_Of(Am_Window))
      Am_Add_Remove_Inter_To_Extra_Window(inter, feedback, false, false);
    feedback.Set(Am_VISIBLE, false);
  }
}

Am_Object
am_create_new_object(Am_Object &inter, Am_Object &cmd, Am_Inter_Location &data,
                     Am_Object &old_object, bool override_position)
{
  Am_Object new_object;
  Am_Create_New_Object_Method new_obj_method;
  if (cmd.Valid())
    new_obj_method =
        cmd.Get(Am_CREATE_NEW_OBJECT_METHOD, Am_RETURN_ZERO_ON_ERROR);
  if (!new_obj_method.Valid()) {
    new_obj_method =
        inter.Get(Am_CREATE_NEW_OBJECT_METHOD, Am_RETURN_ZERO_ON_ERROR);
    if (!new_obj_method.Valid())
      return Am_No_Object;
    // decided it was not really an error to have no create method
    //Am_ERRORO("Neither cmd " << cmd << " nor inter " << inter
    // << " contain a Am_CREATE_NEW_OBJECT_METHOD",
    // inter, Am_CREATE_NEW_OBJECT_METHOD);
  }
  new_object = new_obj_method.Call(inter, data, old_object);
  if (override_position && new_object.Valid())
    data.Install(new_object);
  return new_object;
}

Am_Define_Method(Am_Current_Location_Method, void, Am_New_Points_Do,
                 (Am_Object inter, Am_Object /* object_modified */,
                  Am_Inter_Location data))
{
  bool beep_on_abort = inter.Get(Am_INTER_BEEP_ON_ABORT);
  if (data.Is_Zero_Size() && (int)inter.Get(Am_HOW_MANY_POINTS) > 1 &&
      beep_on_abort) {
    Am_INTER_TRACE_PRINT(inter, "Aborting " << inter << " because zero size");
    Am_Beep();
    Am_Abort_Interactor(inter);
    return;
  }
  Am_Object feedback, new_object;
  feedback = inter.Get(Am_FEEDBACK_OBJECT);
  if (feedback.Valid()) {
    Am_REPORT_VIS_CONDITION(Am_INTER_TRACE_SETTING, inter, feedback, false);
    feedback.Set(Am_VISIBLE, false);
    if (feedback.Is_Instance_Of(Am_Window)) {
      Am_Add_Remove_Inter_To_Extra_Window(inter, feedback, false, false);
      //data's ref-obj will always be INITIAL_REF_OBJECT which will never be
      //the feedback window, so don't have to check here.
    }
  }
  new_object =
      am_create_new_object(inter, Am_No_Object, data, Am_No_Object, false);
  inter.Set(Am_VALUE, new_object);
  inter.Set(Am_OBJECT_MODIFIED, new_object);
  Am_Copy_Values_To_Command(inter);
}

//-----------------------------------
// Undo and Redo methods for Command
//-----------------------------------

// selective has no impact on undo operation.
// reload_data doesn't make sense for create, so ignored
// declared extern in inter_advanced.h
void
am_new_points_general_undo_redo(Am_Object command_obj, bool undo,
                                bool selective, bool reload_data,
                                Am_Object new_obj_proto,
                                const Am_Value &new_loc)
{
  Am_Object inter, new_object, owner;
  inter = command_obj.Get(Am_SAVED_OLD_OWNER);
  new_object = command_obj.Get(Am_VALUE);

#ifdef DEBUG
  if (inter.Valid() && Am_Inter_Tracing(inter)) {
    if (selective)
      std::cout << "Selective ";
    if (undo)
      std::cout << "Undo";
    else
      std::cout << "Repeat";
    std::cout << " command " << command_obj << " on obj " << new_object
              << std::endl
              << std::flush;
  }
#endif

  //assume created object is always on top, so OK to undo it to the top
  bool currently_undone = command_obj.Get(Am_HAS_BEEN_UNDONE);
  if (currently_undone) {
    if (new_object.Valid()) {
      //then put it back
      owner = command_obj.Get(Am_SAVED_OLD_OBJECT_OWNER);
      if (owner.Valid()) {
#ifdef DEBUG
        if (inter.Valid() && Am_Inter_Tracing(inter))
          std::cout << "++Adding " << new_object << " back to owner " << owner
                    << std::endl
                    << std::flush;
#endif
        owner.Add_Part(new_object);
      }
    }
  } else {
    if (undo) {
      if (new_object.Valid()) {
        // remove it
        owner = new_object.Get_Owner();
        command_obj.Set(Am_SAVED_OLD_OBJECT_OWNER, owner, Am_OK_IF_NOT_THERE);
#ifdef DEBUG
        if (inter.Valid() && Am_Inter_Tracing(inter))
          std::cout << "++Removing " << new_object << " from owner " << owner
                    << std::endl
                    << std::flush;
#endif
        owner.Remove_Part(new_object);
      }
    } else { // selective repeat, create a new object
      if (inter.Valid()) {
        Am_Object orig_object, impl_parent;
        if (reload_data && new_obj_proto.Valid()) {
          orig_object = new_obj_proto;
          command_obj.Set(Am_START_OBJECT, orig_object, Am_OK_IF_NOT_THERE);
        } else
          orig_object = command_obj.Get(Am_START_OBJECT);
        if (orig_object.Valid()) {
          // command_obj.Get(Am_INTERIM_VALUE);
          Am_Inter_Location data(orig_object);
          bool override = false;
          if (reload_data && new_loc.Valid()) {
            data = new_loc;
            override = true;
          }
          //if only one point, then size doesn't matter
          if ((int)command_obj.Get(Am_HOW_MANY_POINTS) >= 2) {
            bool beep_on_abort = inter.Get(Am_INTER_BEEP_ON_ABORT);
            if (beep_on_abort && data.Is_Zero_Size()) {
              Am_INTER_TRACE_PRINT(inter, "Aborting " << command_obj
                                                      << " because zero size");
              //not queued for undo
              command_obj.Set(Am_COMMAND_IS_ABORTING, true, Am_OK_IF_NOT_THERE);
              Am_Beep();
              return;
            }
          }
          new_object = am_create_new_object(inter, command_obj, data,
                                            orig_object, override);
          Am_INTER_TRACE_PRINT(inter, "++Created new object " << new_object
                                                              << " at location "
                                                              << data);
          command_obj.Set(Am_VALUE, new_object);
          command_obj.Set(Am_OBJECT_MODIFIED, new_object);
          impl_parent = command_obj.Get(Am_IMPLEMENTATION_PARENT);
          if (impl_parent.Valid()) {
            impl_parent.Set(Am_VALUE, new_object);
            impl_parent.Set(Am_OBJECT_MODIFIED, new_object);
          }
          currently_undone = true; //so set below will make it false
        }
      }
    }
  }
  command_obj.Set(Am_HAS_BEEN_UNDONE, !currently_undone);
}

Am_Define_Method(Am_Object_Method, void, Am_New_Points_Command_Undo,
                 (Am_Object command_obj))
{
  am_new_points_general_undo_redo(command_obj, true, false, false, Am_No_Object,
                                  Am_No_Value);
}

//Am_New_Points_Command_Redo same as Am_New_Points_Command_Undo

//command obj is a copy of the original, so can modify it
Am_Define_Method(Am_Object_Method, void, Am_New_Points_Command_Selective_Undo,
                 (Am_Object command_obj))
{
  am_new_points_general_undo_redo(command_obj, true, true, false, Am_No_Object,
                                  Am_No_Value);
}

//command obj is a copy of the original, so can modify it
Am_Define_Method(Am_Object_Method, void, Am_New_Points_Command_Repeat_Same,
                 (Am_Object command_obj))
{
  am_new_points_general_undo_redo(command_obj, false, true, false, Am_No_Object,
                                  Am_No_Value);
}

Am_Define_Method(Am_Selective_Repeat_New_Method, void,
                 Am_New_Points_Command_Repeat_New,
                 (Am_Object command_obj, Am_Value new_selection,
                  Am_Value new_value))
{
  Am_Object new_object = new_selection;
  am_new_points_general_undo_redo(command_obj, false, true, true, new_object,
                                  new_value);
}

Am_Define_Method(Am_Selective_New_Allowed_Method, bool,
                 ok_if_value_is_one_object,
                 (Am_Object /* command_obj */, Am_Value new_selection,
                  Am_Value new_value))
{
  return ((new_selection.Valid() && new_selection.type == Am_OBJECT) ||
          Am_Inter_Location::Test(new_value));
}

////////////////////////////////////////////////////////////////////////
// demon for when command object destroyed; destroy the saved object
// also if this is for a command which has been undone (so the object
// is no longer visible)
void
Am_New_Point_Command_Destroy_Demon(Am_Object command_obj)
{
  if ((bool)command_obj.Get(Am_HAS_BEEN_UNDONE)) {
    Am_Object new_object;
    new_object = command_obj.Get(Am_VALUE);
    Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING,
                         "++Destroying Command "
                             << command_obj
                             << " so destroying object it created = "
                             << new_object);
    if (new_object.Valid())
      new_object.Destroy();
  }
}

////////////////////////////////////////////////////////////
//global variables

Am_Object Am_New_Points_Internal_Command;
Am_Object Am_New_Points_Interactor;

static void
init()
{
  Am_Object_Advanced obj_adv; // to get at advanced features like demons.
  Am_Demon_Set demons;        // for specialized demons for some objects

  // now create the objects
  Am_New_Points_Internal_Command =
      Am_Object_Create_Command.Create(DSTR("Am_New_Points_Internal_Command"))
          .Set(Am_LABEL, "Create")
          .Add(Am_AS_LINE, false)
          .Add(Am_INTERIM_VALUE, 0)
          .Add(Am_START_OBJECT, (0L))
          .Set(Am_UNDO_METHOD, Am_New_Points_Command_Undo)
          .Set(Am_REDO_METHOD, Am_New_Points_Command_Undo) //works for both
          .Set(Am_SELECTIVE_UNDO_METHOD, Am_New_Points_Command_Selective_Undo)
          .Set(Am_SELECTIVE_REPEAT_SAME_METHOD,
               Am_New_Points_Command_Repeat_Same)
          .Set(Am_SELECTIVE_REPEAT_ON_NEW_METHOD,
               Am_New_Points_Command_Repeat_New)
          .Set(Am_SELECTIVE_REPEAT_NEW_ALLOWED, ok_if_value_is_one_object)
          .Set(Am_SELECTIVE_REPEAT_SAME_ALLOWED,
               Am_Selective_Allowed_Return_True);

  // when the new_point_command object is destroyed, want to destroy
  // the newly object it is saving, so have a special demon
  obj_adv = (Am_Object_Advanced &)Am_New_Points_Internal_Command;
  demons = obj_adv.Get_Demons().Copy();
  demons.Set_Object_Demon(Am_DESTROY_OBJ, Am_New_Point_Command_Destroy_Demon);
  obj_adv.Set_Demons(demons);

  Am_New_Points_Interactor =
      Am_Interactor.Create(DSTR("Am_New_Points_Interactor"))
          .Set(Am_START_WHERE_TEST, Am_Inter_In)
          .Set(Am_INTER_START_METHOD, Am_New_Points_Start_Method)
          .Set(Am_INTER_ABORT_METHOD, Am_New_Points_Abort_Method)
          .Set(Am_INTER_OUTSIDE_METHOD, Am_New_Points_Outside_Method)
          .Set(Am_INTER_BACK_INSIDE_METHOD, Am_New_Points_Back_Inside_Method)
          .Set(Am_INTER_RUNNING_METHOD, Am_New_Points_Running_Method)
          .Set(Am_INTER_STOP_METHOD, Am_New_Points_Stop_Method)

          .Set(Am_START_DO_METHOD, Am_New_Points_Start_Do)
          .Set(Am_INTERIM_DO_METHOD, Am_New_Points_Interim_Do)
          .Set(Am_ABORT_DO_METHOD, Am_New_Points_Abort_Do)
          .Set(Am_DO_METHOD, Am_New_Points_Do)

          .Add(Am_GRID_X, 0)
          .Add(Am_GRID_Y, 0)
          .Add(Am_GRID_ORIGIN_X, 0)
          .Add(Am_GRID_ORIGIN_Y, 0)
          .Add(Am_GRID_METHOD, 0)
          .Add(Am_MINIMUM_WIDTH, 0)
          .Add(Am_MINIMUM_HEIGHT, 0)
          .Add(Am_MINIMUM_LENGTH, 0)
          .Add(Am_AS_LINE, 0)
          .Add(Am_FEEDBACK_OBJECT, 0)
          .Add(Am_HOW_MANY_POINTS, 2)
          .Add(Am_FLIP_IF_CHANGE_SIDES, true)
          .Add(Am_ABORT_IF_TOO_SMALL, false)
          .Add(Am_TOO_SMALL, false)
          .Add(Am_CREATE_NEW_OBJECT_METHOD, (0L))
          .Add(Am_SAVED_OLD_OWNER, (0L))
          .Add(Am_INITIAL_REF_OBJECT, (0L))
          .Add_Part(Am_IMPLEMENTATION_COMMAND,
                    Am_New_Points_Internal_Command.Create(
                        DSTR("New_Points_Internal_Command")))
          .Set_Part(
              Am_COMMAND,
              Am_Object_Create_Command.Create(DSTR("Am_Command_in_New_Points"))
                  .Set(Am_LABEL, "Create")
                  .Set(Am_SELECTIVE_REPEAT_SAME_ALLOWED, (0L))
                  .Set(Am_SELECTIVE_REPEAT_NEW_ALLOWED, (0L)));
}

static Am_Initializer *initializer =
    new Am_Initializer(DSTR("Inter_New_Points"), init, 3.3f);
