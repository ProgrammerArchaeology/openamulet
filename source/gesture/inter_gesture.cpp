/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file contains the functions for handling the Gesture Interactor
   
   Designed and implemented by Rob Miller
*/

#include <am_inc.h>

#include <amulet/univ_map.h>
#include <amulet/am_io.h>
#include <amulet/impl/types_string.h>
#include <amulet/inter_advanced.h>
#include <amulet/standard_slots.h>
#include <amulet/value_list.h>
#include <amulet/initializer.h>

#include <amulet/impl/opal_op.h>
#include <amulet/impl/opal_misc.h>

#include <amulet/registry.h>

#include <amulet/gesture.h>

static void
gesture_start(Am_Object inter)
{
  // clear point list
  inter.Set(Am_POINT_LIST, Am_Point_List());
}

static void
gesture_new_point(Am_Object inter, Am_Object command_obj, int x, int y,
                  Am_Object ref_obj, Am_Input_Char ic)
{
  // translate point and add to POINT_LIST
  Am_Point_List pl;
  pl = inter.Get(Am_POINT_LIST);
  Am_Object owner = inter.Get_Owner();
  if (ref_obj != owner) {
    Am_Translate_Coordinates(ref_obj, x, y, owner, x, y);
    ref_obj = owner;
  }
  pl.Add((float)x, (float)y);
  inter.Set(Am_POINT_LIST, pl);

  if (Am_Inter_Tracing(inter))
    std::cout << x << ' ' << y << '\n';

  // call interim do methods
  Am_Inter_Call_Both_Method(inter, command_obj, Am_INTERIM_DO_METHOD, x, y,
                            ref_obj, ic, Am_No_Object, Am_No_Location);
}

static void
gesture_stop(Am_Object inter)
{
  if (Am_Inter_Tracing(inter))
    std::cout << "END\n" << std::flush;
}

static void
store_classifier_results(Am_Object inter, Am_Value value,
                         Am_Feature_Vector feat, double nap, double dist)
{
  inter.Set(Am_VALUE, value);
  inter.Set(Am_START_X, (int)feat.StartX());
  inter.Set(Am_START_Y, (int)feat.StartY());
  inter.Set(Am_INITIAL_SIN, feat.InitialSin());
  inter.Set(Am_INITIAL_COS, feat.InitialCos());
  inter.Set(Am_DX2, (int)feat.Dx2());
  inter.Set(Am_DY2, (int)feat.Dy2());
  inter.Set(Am_MAGSQ2, feat.MagSq2());
  inter.Set(Am_END_X, (int)feat.EndX());
  inter.Set(Am_END_Y, (int)feat.EndY());
  inter.Set(Am_MIN_X, (int)feat.MinX());
  inter.Set(Am_MAX_X, (int)feat.MaxX());
  inter.Set(Am_MIN_Y, (int)feat.MinY());
  inter.Set(Am_MAX_Y, (int)feat.MaxY());
  inter.Set(Am_TOTAL_LENGTH, feat.PathR());
  inter.Set(Am_TOTAL_ANGLE, feat.PathTh());
  inter.Set(Am_ABS_ANGLE, feat.AbsTh());
  inter.Set(Am_SHARPNESS, feat.Sharpness());
  inter.Set(Am_NONAMBIGUITY_PROB, nap);
  inter.Set(Am_DIST_TO_MEAN, dist);
}

static void
gesture_classify(Am_Object inter)
{
  // fetch slots from interactor: example, classifier, rejection thresholds
  Am_Point_List pl;
  pl = inter.Get(Am_POINT_LIST);
  Am_Value classifier;
  double min_nonambig_prob, max_dist_to_mean;

  classifier = inter.Peek(Am_CLASSIFIER);
  min_nonambig_prob = inter.Get(Am_MIN_NONAMBIGUITY_PROB);
  max_dist_to_mean = inter.Get(Am_MAX_DIST_TO_MEAN);

  // calculate a feature vector from the points
  Am_Feature_Vector feat(pl);

  // run the classifier
  Am_String classname;
  double nap, dist;

  if (!classifier.Valid()) {
    classname = Am_No_String;
    nap = 1.0;
    dist = 0.0;
  } else
    classname = ((Am_Gesture_Classifier)classifier).Classify(feat, &nap, &dist);

  // check for rejection and compute resulting value
  Am_Value value;

  if (!classname.Valid() || nap < min_nonambig_prob ||
      (max_dist_to_mean > 0 && dist > max_dist_to_mean))
    value = Am_No_Value;
  else
    value = classname;

  // store value and selected features into interactor
  store_classifier_results(inter, value, feat, nap, dist);
}

//////////////////////////////////////////////////////////////////
/// Choosing command object to run based on recognized gesture
//////////////////////////////////////////////////////////////////

static Am_Object gesture_set_impl_command(Am_Object,
                                          Am_Object /* object_modified */,
                                          Am_Inter_Location /* data */)
{
  // gesture interactor lacks an implementation command, so nothing to do!
  return (0L);
}

static Am_Object
choose_command_obj(Am_Object inter)
{
  Am_Value_List items;
  items = inter.Get(Am_ITEMS);
  Am_String gesture_name;
  gesture_name = inter.Get(Am_VALUE);
  Am_Object matching_cmd;

  // search for a command in Am_ITEMS whose label is gesture_name.
  if (items.Valid() && gesture_name.Valid()) {
    for (items.Start(); !items.Last(); items.Next()) {
      Am_Object item_cmd;
      item_cmd = items.Get();

      Am_String item_name;
      item_name = item_cmd.Get(Am_LABEL);

      if (gesture_name == item_name && (bool)item_cmd.Get(Am_ACTIVE)) {
        matching_cmd = item_cmd;
        break;
      }
    }
  }

  if (matching_cmd.Valid())
    return matching_cmd;
  else
    return (Am_Object)inter.Get(Am_COMMAND);
}

//////////////////////////////////////////////////////////////////
/// Maintain SAVED_OLD_OWNER slot on command objects (it points
///  back to the gesture interactor, which for commands on the
///  Am_ITEMS isn't the same as the owner)
//////////////////////////////////////////////////////////////////

static int
set_items_list_old_owner(Am_Value_List items, Am_Object widget)
{
  Am_Value item_value;
  Am_Object item;
  int ret = 0; //not used, just for debugging
  for (items.Start(); !items.Last(); items.Next()) {
    item_value = items.Get();
    if (item_value.type == Am_OBJECT) {
      item = (Am_Object)item_value;
      if (item.Is_Instance_Of(Am_Command)) {
        item.Set(Am_SAVED_OLD_OWNER, widget, Am_OK_IF_NOT_THERE);
        ret = 2;
      }
    }
  }
  return ret;
}

Am_Define_Formula(int, Am_Gesture_Set_Old_Owner_To_Me)
{
  int ret = 0;
  Am_Object cmd;
  Am_Value_List items;
  cmd = self.Get(Am_COMMAND);
  if (cmd.Valid()) {
    cmd.Set(Am_SAVED_OLD_OWNER, self, Am_OK_IF_NOT_THERE);
    ret = 1;
  }
  items = self.Get(Am_ITEMS);
  ret = set_items_list_old_owner(items, self);

  //ret not used, different values just for debugging
  return ret;
}

//////////////////////////////////////////////////////////////////
/// Methods
//////////////////////////////////////////////////////////////////

Am_Define_Method(Am_Inter_Internal_Method, void, Am_Gesture_Start_Method,
                 (Am_Object & inter, Am_Object &object, Am_Object &event_window,
                  Am_Input_Event *ev))
{
  if (Am_Inter_Tracing(inter))
    std::cout << "Gesture starting over " << object << std::endl;
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

  gesture_start(inter);
  gesture_new_point(inter, command_obj, x, y, event_window, ic);
}

Am_Define_Method(Am_Inter_Internal_Method, void, Am_Gesture_Abort_Method,
                 (Am_Object & inter, Am_Object &object, Am_Object &event_window,
                  Am_Input_Event *ev))
{
  if (Am_Inter_Tracing(inter))
    std::cout << "Gesture Aborting\n";

  // first, call the prototype's method
  Am_Inter_Internal_Method inter_method;
  inter_method = Am_Interactor.Get(Am_INTER_ABORT_METHOD);
  inter_method.Call(inter, object, event_window, ev);

  // Now, call the correct Command method
  Am_Object command_obj;
  command_obj = inter.Get(Am_COMMAND);
  Am_Inter_Call_Both_Method(inter, command_obj, Am_ABORT_DO_METHOD, ev->x,
                            ev->y, event_window, ev->input_char, object,
                            Am_No_Location);
}

Am_Define_Method(Am_Inter_Internal_Method, void, Am_Gesture_Outside_Method,
                 (Am_Object & inter, Am_Object &object, Am_Object &event_window,
                  Am_Input_Event *ev))
{
  if (Am_Inter_Tracing(inter))
    std::cout << "Gesture Outside\n";

  // first, call the prototype's method
  Am_Inter_Internal_Method inter_method;
  inter_method = Am_Interactor.Get(Am_INTER_OUTSIDE_METHOD);
  inter_method.Call(inter, object, event_window, ev);

  gesture_stop(inter);

  // abort when outside
  Am_Object command_obj;
  command_obj = inter.Get(Am_COMMAND);
  Am_Inter_Call_Both_Method(inter, command_obj, Am_ABORT_DO_METHOD, ev->x,
                            ev->y, event_window, ev->input_char, Am_No_Object,
                            Am_No_Location);
}

Am_Define_Method(Am_Inter_Internal_Method, void, Am_Gesture_Back_Inside_Method,
                 (Am_Object & inter, Am_Object &object, Am_Object &event_window,
                  Am_Input_Event *ev))
{
  if (Am_Inter_Tracing(inter))
    std::cout << "Gesture back inside over " << object << std::endl;
  // first, call the prototype's method
  Am_Inter_Internal_Method inter_method;
  inter_method = Am_Interactor.Get(Am_INTER_BACK_INSIDE_METHOD);
  inter_method.Call(inter, object, event_window, ev);

  Am_Object command_obj;
  command_obj = inter.Get(Am_COMMAND);
  // now, call interim_do_method
  gesture_new_point(inter, command_obj, ev->x, ev->y, event_window,
                    ev->input_char);
}

Am_Define_Method(Am_Inter_Internal_Method, void, Am_Gesture_Running_Method,
                 (Am_Object & inter, Am_Object &object, Am_Object &event_window,
                  Am_Input_Event *ev))
{

  if (Am_Inter_Tracing(inter))
    std::cout << "Gesture running over " << object << std::endl;
  // first, call the prototype's method
  Am_Inter_Internal_Method inter_method;
  inter_method = Am_Interactor.Get(Am_INTER_RUNNING_METHOD);
  inter_method.Call(inter, object, event_window, ev);

  // now, call interim_do_method
  Am_Object command_obj;
  command_obj = inter.Get(Am_COMMAND);
  gesture_new_point(inter, command_obj, ev->x, ev->y, event_window,
                    ev->input_char);
}

Am_Define_Method(Am_Inter_Internal_Method, void, Am_Gesture_Stop_Method,
                 (Am_Object & inter, Am_Object &object, Am_Object &event_window,
                  Am_Input_Event *ev))
{
  Am_Object inter_command_obj;
  inter_command_obj = inter.Get(Am_COMMAND);

  if (Am_Inter_Tracing(inter))
    std::cout << "Gesture stopping over " << object << std::endl;

  // get last point
  gesture_new_point(inter, inter_command_obj, ev->x, ev->y, event_window,
                    ev->input_char);
  gesture_stop(inter);

  // classify the gesture and store its name in Am_VALUE
  gesture_classify(inter);

  // choose the appropriate command object to invoke (either from Am_ITEMS
  // list or from Am_COMMAND slot) and store the gesture name in its Am_VALUE
  Am_Object chosen_command_obj;
  Am_Value gesture_name;
  chosen_command_obj = choose_command_obj(inter);
  gesture_name = inter.Peek(Am_VALUE);
  chosen_command_obj.Set(Am_VALUE, gesture_name);

  // invoke the Do methods and register for undo
  Am_Call_Final_Do_And_Register(inter, chosen_command_obj, ev->x, ev->y,
                                event_window, ev->input_char, object,
                                Am_No_Location, gesture_set_impl_command);

  // LAST, call our prototype's method
  Am_Inter_Internal_Method inter_method;
  inter_method = Am_Interactor.Get(Am_INTER_STOP_METHOD);
  inter_method.Call(inter, object, event_window, ev);
}

//////////////////////////////////////////////////////////////////////////////
// Default methods to make the Gesture Interactor work
//////////////////////////////////////////////////////////////////////////////

Am_Define_Method(Am_Object_Method, void, Am_Gesture_Start_Do, (Am_Object inter))
{
  Am_Object feedback;
  feedback = inter.Get(Am_FEEDBACK_OBJECT);
  if (feedback.Valid()) {
    feedback.Set(Am_POINT_LIST, (Am_Point_List)inter.Get(Am_POINT_LIST));
    //move feedback object to new group if necessary
    Am_Check_And_Fix_Feedback_Group(feedback, inter);
    feedback.Set(Am_VISIBLE, true);
    Am_To_Top(feedback);
#ifdef DEBUG
    if (Am_Inter_Tracing(Am_INTER_TRACE_SETTING))
      Am_Report_Set_Vis(inter, feedback, true);
#endif
  }
}

Am_Define_Method(Am_Object_Method, void, Am_Gesture_Interim_Do,
                 (Am_Object inter))
{
  Am_Object feedback;
  feedback = inter.Get(Am_FEEDBACK_OBJECT);
  if (feedback.Valid()) {
    feedback.Set(Am_POINT_LIST, (Am_Point_List)inter.Get(Am_POINT_LIST));
  }
}

Am_Define_Method(Am_Object_Method, void, Am_Gesture_Abort_Do, (Am_Object inter))
{
  Am_Object feedback;
  feedback = inter.Get(Am_FEEDBACK_OBJECT);
  if (feedback.Valid()) {
    feedback.Set(Am_VISIBLE, false);
#ifdef DEBUG
    if (Am_Inter_Tracing(Am_INTER_TRACE_SETTING))
      Am_Report_Set_Vis(inter, feedback, false);
#endif
  }
}

Am_Define_Method(Am_Object_Method, void, Am_Gesture_Do, (Am_Object inter))
{
  // hide feedback
  Am_Object feedback;
  feedback = inter.Get(Am_FEEDBACK_OBJECT);
  if (feedback.Valid()) {
    feedback.Set(Am_VISIBLE, false);
#ifdef DEBUG
    if (Am_Inter_Tracing(Am_INTER_TRACE_SETTING))
      Am_Report_Set_Vis(inter, feedback, false);
#endif
  }
}

//////////////////////////////////////////////////////////////////////////////
// Default methods to make the Gesture Interactor work
//////////////////////////////////////////////////////////////////////////////

static void
gesture_set_slot_names()
{
#ifdef DEBUG
  Am_Register_Slot_Key(Am_CLASSIFIER, "CLASSIFIER");
  Am_Register_Slot_Key(Am_MIN_NONAMBIGUITY_PROB, "MIN_NONAMBIGUITY_PROB");
  Am_Register_Slot_Key(Am_MAX_DIST_TO_MEAN, "MAX_DIST_TO_MEAN");

  Am_Register_Slot_Key(Am_START_X, "START_X");
  Am_Register_Slot_Key(Am_START_Y, "START_Y");
  Am_Register_Slot_Key(Am_INITIAL_SIN, "INITIAL_SIN");
  Am_Register_Slot_Key(Am_INITIAL_COS, "INITIAL_COS");
  Am_Register_Slot_Key(Am_DX2, "DX2");
  Am_Register_Slot_Key(Am_DY2, "DY2");
  Am_Register_Slot_Key(Am_MAGSQ2, "MAGSQ2");
  Am_Register_Slot_Key(Am_END_X, "END_X");
  Am_Register_Slot_Key(Am_END_Y, "END_Y");
  Am_Register_Slot_Key(Am_MIN_X, "MIN_X");
  Am_Register_Slot_Key(Am_MAX_X, "MAX_X");
  Am_Register_Slot_Key(Am_MIN_Y, "MIN_Y");
  Am_Register_Slot_Key(Am_MAX_Y, "MAX_Y");
  Am_Register_Slot_Key(Am_TOTAL_LENGTH, "TOTAL_LENGTH");
  Am_Register_Slot_Key(Am_TOTAL_ANGLE, "TOTAL_ANGLE");
  Am_Register_Slot_Key(Am_ABS_ANGLE, "ABS_ANGLE");
  Am_Register_Slot_Key(Am_SHARPNESS, "SHARPNESS");
  Am_Register_Slot_Key(Am_NONAMBIGUITY_PROB, "NONAMBIGUITY_PROB");
  Am_Register_Slot_Key(Am_DIST_TO_MEAN, "DIST_TO_MEAN");
#endif
}

Am_Define_Method(Am_Object_Method, void, gesture_unrecognized_do,
                 (Am_Object cmd))
{
  Am_Object inter, window;
  inter = cmd.Get(Am_SAVED_OLD_OWNER);
  window = inter.Get(Am_WINDOW);
  std::cout << "** Unrecognized Gesture\n" << std::flush;
  Am_Beep(window);
}

Am_Define_Method(Am_Object_Method, void, gesture_create_command_do_method,
                 (Am_Object cmd))
{
  bool as_line = cmd.Get(Am_AS_LINE);
  Am_Object inter = cmd.Get(Am_SAVED_OLD_OWNER);
  Am_Object owner = inter.Get_Owner();
  Am_Inter_Location data;
  if (as_line) {
    int x1 = inter.Get(Am_START_X);
    int y1 = inter.Get(Am_START_Y);
    int x2 = inter.Get(Am_END_X);
    int y2 = inter.Get(Am_END_Y);
    data.Set_Location(true, owner, x1, y1, x2, y2);
  } else {
    int left = inter.Get(Am_MIN_X);
    int top = inter.Get(Am_MIN_Y);
    int width = (int)inter.Get(Am_MAX_X) - left;
    int height = (int)inter.Get(Am_MAX_Y) - top;
    data.Set_Location(false, owner, left, top, width, height);
  }
  Am_Object new_object;
  Am_Create_New_Object_Method new_obj_method;
  new_obj_method = cmd.Get(Am_CREATE_NEW_OBJECT_METHOD);
  if (new_obj_method.Valid())
    new_object = new_obj_method.Call(cmd, data, Am_No_Object);

  // Set appropriate slots in implementation parent (to be called next).
  // This parent command is actually the New_Points_Interactor's
  // implementation command.
  Am_Object create_cmd = cmd.Get(Am_SUB_COMMAND);
  create_cmd.Set(Am_SAVED_OLD_OWNER, inter, Am_OK_IF_NOT_THERE)
      .Set(Am_OBJECT_MODIFIED, new_object)
      .Set(Am_START_OBJECT, new_object.Copy())
      .Set(Am_VALUE, new_object)
      .Set(Am_AS_LINE, as_line)
      .Set(Am_INTERIM_VALUE, data)
      .Set(Am_HAS_BEEN_UNDONE, false);
  //usually, the cmd itself isn't undone, just the sub command, but
  //just in case, set some slots that might be useful
  cmd.Set(Am_SAVED_OLD_OWNER, inter, Am_OK_IF_NOT_THERE)
      .Set(Am_OBJECT_MODIFIED, new_object)
      .Set(Am_VALUE, new_object);
  Am_Object impl_parent = cmd.Get(Am_IMPLEMENTATION_PARENT);
  if (!impl_parent.Valid() || !impl_parent.Is_Instance_Of(create_cmd)) {
    //if there is an impl parent that isn't the sub-command, then make
    //that command be the impl parent of the sub-command.
    create_cmd.Set(Am_IMPLEMENTATION_PARENT, impl_parent);
    cmd.Set(Am_IMPLEMENTATION_PARENT, create_cmd);
  }
  impl_parent = create_cmd.Get(Am_IMPLEMENTATION_PARENT);
  if (impl_parent.Valid()) { // then provide it with some slots it can use
    impl_parent.Set(Am_OBJECT_MODIFIED, new_object);
  }
}

Am_Define_Method(Am_Object_Method, void, gesture_select_and_do, (Am_Object cmd))
{
  Am_Object selection_widget;
  Am_Value v;
  v = cmd.Peek(Am_SELECTION_WIDGET);
  if (v.Valid())
    selection_widget = v;
  else {
    Am_Object inter = cmd.Get(Am_SAVED_OLD_OWNER);
    v = inter.Peek(Am_SELECTION_WIDGET);
    if (!v.Valid())
      Am_Error("gesture_select_and_do command must have a widget in the "
               "Am_SELECTION_WIDGET slot",
               cmd, Am_SELECTION_WIDGET);
    selection_widget = v;
  }
  Am_Object group = selection_widget.Get(Am_OPERATES_ON);

  Am_Object inter;
  inter = cmd.Get(Am_SAVED_OLD_OWNER);
  int g_minx = inter.Get(Am_MIN_X);
  int g_miny = inter.Get(Am_MIN_Y);
  int g_maxx = inter.Get(Am_MAX_X);
  int g_maxy = inter.Get(Am_MAX_Y);

  // find all the objects whose bounding boxes intersect the gesture's bounding
  // box.
  Am_Value_List objs;
  objs = group.Get(Am_GRAPHICAL_PARTS);
  Am_Value_List selected_objs;

  int o_minx, o_miny, o_maxx, o_maxy;
  for (objs.Start(); !objs.Last(); objs.Next()) {
    Am_Object obj;
    obj = objs.Get();
    o_minx = obj.Get(Am_LEFT);
    o_miny = obj.Get(Am_TOP);
    o_maxx = o_minx + (int)obj.Get(Am_WIDTH);
    o_maxy = o_miny + (int)obj.Get(Am_HEIGHT);

    if (g_maxx > o_minx && o_maxx > g_minx && g_maxy > o_miny &&
        o_maxy > g_miny)
      selected_objs.Add(obj);
  }

  if (selected_objs.Empty()) {
    // nothing to do!  don't execute command, don't register for undo
    Am_Abort_Interactor(inter);
  } else { //set selection.  Will execute the command automatically
           //since it is already the Am_IMPLEMENTATION_PARENT of this command
    selection_widget.Set(Am_VALUE, selected_objs);
  }
}

//global variables
Am_Object Am_Gesture_Interactor; // choosing one from a set

Am_Object Am_Gesture_Create_Command;
Am_Object Am_Gesture_Select_And_Do_Command;
Am_Object Am_Gesture_Unrecognized_Command;

static void
init()
{

  gesture_set_slot_names();

  Am_Gesture_Interactor =
      Am_Interactor.Create(DSTR("Am_Gesture_Interactor"))
          .Set(Am_START_WHERE_TEST, Am_Inter_In)
          .Set(Am_INTER_START_METHOD, Am_Gesture_Start_Method)
          .Set(Am_INTER_ABORT_METHOD, Am_Gesture_Abort_Method)
          .Set(Am_INTER_OUTSIDE_METHOD, Am_Gesture_Outside_Method)
          .Set(Am_INTER_BACK_INSIDE_METHOD, Am_Gesture_Back_Inside_Method)
          .Set(Am_INTER_RUNNING_METHOD, Am_Gesture_Running_Method)
          .Set(Am_INTER_STOP_METHOD, Am_Gesture_Stop_Method)

          .Set(Am_START_DO_METHOD, Am_Gesture_Start_Do)
          .Set(Am_INTERIM_DO_METHOD, Am_Gesture_Interim_Do)
          .Set(Am_ABORT_DO_METHOD, Am_Gesture_Abort_Do)
          .Set(Am_DO_METHOD, Am_Gesture_Do)
          .Set(Am_SET_COMMAND_OLD_OWNER, Am_Gesture_Set_Old_Owner_To_Me)

          .Add(Am_CLASSIFIER, 0)
          .Add(Am_MIN_NONAMBIGUITY_PROB, 0)
          .Add(Am_MAX_DIST_TO_MEAN, 0)
          .Add(Am_ITEMS, 0)

          .Add(Am_FEEDBACK_OBJECT, 0)

          .Add(Am_POINT_LIST, Am_Point_List())

          .Add(Am_START_X, 0)
          .Add(Am_START_Y, 0)
          .Add(Am_INITIAL_SIN, 0)
          .Add(Am_INITIAL_COS, 0)
          .Add(Am_DX2, 0)
          .Add(Am_DY2, 0)
          .Add(Am_MAGSQ2, 0)
          .Add(Am_END_X, 0)
          .Add(Am_END_Y, 0)
          .Add(Am_MIN_X, 0)
          .Add(Am_MAX_X, 0)
          .Add(Am_MIN_Y, 0)
          .Add(Am_MAX_Y, 0)
          .Add(Am_TOTAL_LENGTH, 0)
          .Add(Am_TOTAL_ANGLE, 0)
          .Add(Am_ABS_ANGLE, 0)
          .Add(Am_SHARPNESS, 0)
          .Add(Am_NONAMBIGUITY_PROB, 0)
          .Add(Am_DIST_TO_MEAN, 0);

  Am_Gesture_Interactor.Get_Object(Am_COMMAND)
      .Set(Am_LABEL, "Gesture")
      .Set_Name(DSTR("Am_Command_in_Gesture"));

  Am_Object new_points_impl_cmd =
      Am_New_Points_Interactor.Get_Object(Am_IMPLEMENTATION_COMMAND).Create();
  //  new_points_impl_cmd.Set(Am_CREATE_NEW_OBJECT_METHOD,
  //			  Am_From_Owner(Am_CREATE_NEW_OBJECT_METHOD));
  Am_Gesture_Create_Command =
      Am_Command.Create(DSTR("Gesture_Create_Command"))
          .Set(Am_DO_METHOD, gesture_create_command_do_method)
          .Add(Am_AS_LINE, false)
          .Set(Am_VALUE, (0L)) //set with the name of the gesture
          .Add(Am_CREATE_NEW_OBJECT_METHOD, (0L)) //must be supplied
          .Set(Am_IMPLEMENTATION_PARENT, (0L))    //may be supplied
          .Add_Part(Am_SUB_COMMAND, new_points_impl_cmd);

  Am_Gesture_Select_And_Do_Command =
      Am_Command.Create(DSTR("Gesture_Select_And_Do_Command"))
          .Set(Am_DO_METHOD, gesture_select_and_do)
          .Add(Am_SELECTION_WIDGET, (0L)) //must be set here or in inter
          .Add(Am_SUB_COMMAND, (0L)) // used internally as a copy of impl parent
          .Set(Am_IMPLEMENTATION_PARENT, (0L)) //command to execute; REQUIRED
      ;
  Am_Gesture_Unrecognized_Command =
      Am_Command.Create(DSTR("Gesture_Unrecognized_Command"))
          .Set(Am_DO_METHOD, gesture_unrecognized_do)
          .Set(Am_IMPLEMENTATION_PARENT, true); // inhibit undo
}

static Am_Initializer *initializer =
    new Am_Initializer(DSTR("Gesture"), init, 7.0);
