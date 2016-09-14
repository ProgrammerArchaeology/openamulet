/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <amulet.h>

#include <amulet/standard_slots.h>
#include <amulet/value_list.h>
#include <amulet/inter_advanced.h> // for Am_Selective_New_Allowed_Return_False

#include <amulet/widgets.h>
#include <amulet/debugger.h>
#include <amulet/misc.h>
#include <amulet/registry.h>
#include <amulet/gesture.h>
#include <amulet/scripting.h>
#include <amulet/anim.h>

#define CLASSIFIER_FILENAME "select.cl"

Am_Slot_Key R0 = Am_Register_Slot_Name("R0");
Am_Slot_Key NEW_OBJECT_PROTOTYPE =
    Am_Register_Slot_Name("NEW_OBJECT_PROTOTYPE");

Am_Object window;
Am_Object create_inter;
Am_Object create_poly_inter;
Am_Object freehand_inter;
Am_Object rfeedback;
Am_Object lfeedback;
Am_Object undo_handler;
Am_Object my_selection;
Am_Object last_created_obj;
Am_Object created_objs;
Am_Object scroller;
Am_Object my_undo_dialog;
Am_Object menu_bar;
Am_Object arrow_bm;
Am_Object circle_bm;
Am_Object rect_bm;
Am_Object line_bm;
Am_Object poly_bm;
Am_Object text_bm;
Am_Object freehand_bm;
Am_Object amulet_icon;
Am_Object tool_panel;
Am_Object color_panel;
Am_Object cut_command;
Am_Style special_style = Am_No_Style;
Am_Object grid_command;
Am_Object animations_command;
Am_Object animator_proto, color_anim_proto;

Am_Define_No_Self_Formula(int, grid_if_should)
{
  Am_Value grid_on;
  grid_on = grid_command.Peek(Am_VALUE);
  if (grid_on.Valid())
    return 20;
  else
    return 0;
}

void
Set_LTWH_Anims_On(Am_Object &obj, Am_Slot_Key color_slot, bool wh)
{
  obj.Get(Am_LEFT); //evaluate these before putting in the animation
  obj.Get(Am_TOP);  //evaluate these before putting in the animation
  if (wh) {
    obj.Get(Am_WIDTH);  //evaluate these before putting in the animation
    obj.Get(Am_HEIGHT); //evaluate these before putting in the animation
  }
  Am_Object animlt = animator_proto.Create();
  obj.Set(Am_LEFT, Am_Animate_With(animlt))
      .Set(Am_TOP, Am_Animate_With(animlt));
  if (wh) {
    obj.Set(Am_WIDTH, Am_Animate_With(animlt))
        .Set(Am_HEIGHT, Am_Animate_With(animlt));
  }
  if (color_slot) {
    Am_Object animc = color_anim_proto.Create("line_color_anim");
    obj.Set(color_slot, Am_Animate_With(animc));
  }
}

void
Set_XY_Anims_On(Am_Object &obj)
{
  obj.Get(Am_X1); //evaluate these before putting in the animation
  obj.Get(Am_Y1); //evaluate these before putting in the animation
  obj.Get(Am_X2); //evaluate these before putting in the animation
  obj.Get(Am_Y2); //evaluate these before putting in the animation
  Am_Object animlt = animator_proto.Create();
  Am_Object animc = color_anim_proto.Create("line_color_anim");
  obj.Set(Am_X1, Am_Animate_With(animlt))
      .Set(Am_Y1, Am_Animate_With(animlt))
      .Set(Am_X2, Am_Animate_With(animlt))
      .Set(Am_Y2, Am_Animate_With(animlt))
      .Set(Am_LINE_STYLE, Am_Animate_With(animc));
}

Am_Object my_arc_proto, my_rectangle_proto, my_line_proto, my_polygon_proto,
    freehand_line_proto, my_text_proto;

Am_Object
create_new_obj_internal(Am_Object prototype, Am_Inter_Location data)
{
  Am_Object ref_obj;
  int a, b, c, d;
  bool create_line;
  data.Get_Location(create_line, ref_obj, a, b, c, d);
  if (ref_obj != created_objs) {
    Am_Translate_Coordinates(ref_obj, a, b, created_objs, a, b);
    if (create_line)
      Am_Translate_Coordinates(ref_obj, c, d, created_objs, c, d);
  }

  Am_Object color_rect;
  color_rect = color_panel.Get(Am_VALUE);
  Am_Style color = color_rect.Get(Am_FILL_STYLE);

  Am_Object new_obj;
  if (create_line) {
    new_obj = prototype.Create()
                  .Set(Am_LINE_STYLE, color, Am_NO_ANIMATION)
                  .Set(Am_X1, a, Am_NO_ANIMATION)
                  .Set(Am_Y1, b, Am_NO_ANIMATION)
                  .Set(Am_X2, c, Am_NO_ANIMATION)
                  .Set(Am_Y2, d, Am_NO_ANIMATION);
    Set_XY_Anims_On(new_obj);
  } else if (prototype.Valid()) {
    new_obj = prototype.Create()
                  .Set(Am_LEFT, a, Am_NO_ANIMATION)
                  .Set(Am_TOP, b, Am_NO_ANIMATION);
    if (data.Get_Growing())
      new_obj.Set(Am_WIDTH, c, Am_NO_ANIMATION)
          .Set(Am_HEIGHT, d, Am_NO_ANIMATION);
    if (prototype == my_text_proto)
      new_obj.Set(Am_LINE_STYLE, color, Am_NO_ANIMATION);
    else
      new_obj.Set(Am_FILL_STYLE, color, Am_NO_ANIMATION);
  } else
    new_obj = Am_No_Object;

  if (new_obj.Valid()) {
    std::cout << "\n--++-- Created new object " << new_obj << " at (" << a
              << "," << b << "," << c << "," << d << ")\n"
              << std::flush;
    created_objs.Add_Part(new_obj);
    last_created_obj = new_obj;
  }
  return new_obj;
}

// Am_Create_New_Object_Proc for new object command
Am_Define_Method(Am_Create_New_Object_Method, Am_Object, create_new_object,
                 (Am_Object /* inter */, Am_Inter_Location data,
                  Am_Object old_object))
{
  Am_Object new_obj;

  if (old_object.Valid()) {
    new_obj = old_object.Copy();
    std::cout << "\n--++-- Created new object " << new_obj << " as a copy of "
              << old_object << std::endl
              << std::flush;
    if (new_obj.Valid()) {
      created_objs.Add_Part(new_obj);
      last_created_obj = new_obj;
    }
  } else {
    Am_Object proto;
    Am_Object v;
    v = tool_panel.Get(Am_VALUE);
    if (v == circle_bm)
      proto = my_arc_proto;
    else if (v == rect_bm)
      proto = my_rectangle_proto;
    else if (v == line_bm)
      proto = my_line_proto;
    else if (v == text_bm)
      proto = my_text_proto;
    else
      std::cout << "**UNKNOWN type of create: " << v << std::flush;

    new_obj = create_new_obj_internal(proto, data);
  }

  std::cout << "-- create returning " << new_obj << std::endl << std::flush;
  return new_obj;
}

Am_Define_Object_Formula(compute_feedback_obj)
{
  Am_Object which_feedback;
  if ((bool)self.Get(Am_AS_LINE)) {
    std::cout << "&-&&- Recompute formula; using LINE feedback\n";
    which_feedback = lfeedback;
  } else {
    std::cout << "&-&&- Recompute formula; using RECT feedback\n";
    which_feedback = rfeedback;
  }
  return which_feedback;
}

Am_Define_No_Self_Formula(bool, line_tool)
{
  Am_Value v;
  v = tool_panel.Peek(Am_VALUE);
  if (v.type == Am_OBJECT && v == line_bm)
    return true;
  else
    return false;
}

Am_Define_No_Self_Formula(bool, selection_tool)
{
  Am_Value v;
  v = tool_panel.Peek(Am_VALUE);
  if (v.type == Am_OBJECT && v == arrow_bm)
    return true;
  else
    return false;
}

Am_Define_No_Self_Formula(bool, somethings_selected)
{
  if (my_selection.Valid()) {
    Am_Value v;
    v = my_selection.Get(Am_VALUE);
    return !v.Valid();
  } else
    return false;
}

Am_Define_No_Self_Formula(bool, rubber_bandable_tool_is_selected)
{
  //Am_Object v;
  // v = tool_panel.GV(Am_VALUE);
  Am_Object v(tool_panel.Get(Am_VALUE));
  return (v == circle_bm || v == rect_bm || v == line_bm);
}

Am_Define_No_Self_Formula(bool, polygon_tool_is_selected)
{
  return tool_panel.Get(Am_VALUE) == poly_bm;
}

Am_Define_No_Self_Formula(bool, freehand_tool_is_selected)
{
  return tool_panel.Get(Am_VALUE) == freehand_bm;
}
Am_Define_No_Self_Formula(bool, text_is_selected)
{
  return tool_panel.Get(Am_VALUE) == text_bm;
}

Am_Object partial_poly; // polygon under construction

Am_Style Thick_Black_Line(0.0f, 0.0f, 0.0f, 10);

Am_Define_Method(Am_Mouse_Event_Method, void, polygon_start,
                 (Am_Object inter, int x, int y, Am_Object event_window,
                  Am_Input_Char ic))
{
  if (event_window != created_objs)
    Am_Translate_Coordinates(event_window, x, y, created_objs, x, y);

  // start the polygon
  partial_poly =
      my_polygon_proto.Create()
          .Set(Am_POINT_LIST, Am_Point_List())
          .Set(Am_FILL_STYLE, Am_No_Style)
          .Set(Am_LINE_STYLE, ic.shift ? Thick_Black_Line : Am_Black);
  ;

  created_objs.Add_Part(partial_poly);

  // enable vertex-selection interactor
  //   poly_vertex_inter.Set (Am_ACTIVE, true);

  // show feedback
  Am_Object feedback(inter.Get(Am_FEEDBACK_OBJECT));
  if (feedback.Valid()) {
    feedback.Set(Am_VISIBLE, true);
    Am_To_Top(feedback);
    // feedback position will be set by polygon_interim_do
  }
}

Am_Define_Method(Am_Mouse_Event_Method, void, polygon_interim_do,
                 (Am_Object inter, int x, int y, Am_Object event_window,
                  Am_Input_Char ic))
{
  if (event_window != created_objs)
    Am_Translate_Coordinates(event_window, x, y, created_objs, x, y);

  // move endpoint of feedback line
  Am_Object feedback;
  feedback = inter.Get(Am_FEEDBACK_OBJECT);
  if (feedback.Valid()) {
    bool feedback_vis = feedback.Get(Am_VISIBLE);
    if (feedback_vis == false) {
      feedback.Set(Am_VISIBLE, true);
    }
    feedback.Set(Am_X2, x);
    feedback.Set(Am_Y2, y);
  }

  static Am_Input_Char vertex_event("any_left_down");
  if (ic == vertex_event) {
    // user has left-clicked!
    if (!Am_Point_In_All_Owners(partial_poly, x, y, created_objs)) {
      // clicked outside of drawing window -- throw away this point and
      // stop
      Am_Stop_Interactor(inter, Am_No_Object, Am_Input_Char(), Am_No_Object, 0,
                         0);
    } else {
      Am_Point_List pl = partial_poly.Get(Am_POINT_LIST);
      if (pl.Empty()) {
        // the click that started it all -- first point of the polygon
        partial_poly.Set(Am_POINT_LIST, pl.Add((float)x, (float)y));
        feedback.Set(Am_X1, x);
        feedback.Set(Am_Y1, y);
      } else {
        int first_x;
        int first_y;
        pl.Start();
        pl.Get(first_x, first_y);

        int delta_x = x - first_x;
        int delta_y = y - first_y;

        if (delta_x < 5 && delta_x > -5 && delta_y < 5 && delta_y > -5) {
          // clicked on (er, near) the initial point again -- close
          // the polygon and stop
          pl.Add((float)first_x, (float)first_y, Am_TAIL, false);
          Am_Stop_Interactor(inter, Am_No_Object, Am_Input_Char(), Am_No_Object,
                             0, 0);
        } else {
          // add new point to polygon, reset feedback origin to it, and
          // keep running
          pl.Add((float)x, (float)y, Am_TAIL, false);
          feedback.Set(Am_X1, x);
          feedback.Set(Am_Y1, y);
        }
        partial_poly.Note_Changed(Am_POINT_LIST);
      }
    }
  }
}

Am_Define_Method(Am_Mouse_Event_Method, void, polygon_abort,
                 (Am_Object inter, int x, int y, Am_Object event_window,
                  Am_Input_Char /* ic */))
{
  if (event_window != created_objs)
    Am_Translate_Coordinates(event_window, x, y, created_objs, x, y);

  // hide feedback
  Am_Object feedback;
  feedback = inter.Get(Am_FEEDBACK_OBJECT);
  if (feedback.Valid())
    feedback.Set(Am_VISIBLE, false);

  // destroy polygon under construction
  partial_poly.Destroy();
}

Am_Define_Method(Am_Mouse_Event_Method, void, polygon_do,
                 (Am_Object inter, int /*x*/, int /*y*/,
                  Am_Object /*event_window*/, Am_Input_Char /* ic */))
{
  // hide feedback
  Am_Object feedback;
  feedback = inter.Get(Am_FEEDBACK_OBJECT);
  if (feedback.Valid())
    feedback.Set(Am_VISIBLE, false);

  Am_Object new_object = partial_poly;
  Am_Style color = color_panel.Get_Object(Am_VALUE).Get(Am_FILL_STYLE);
  new_object.Set(Am_FILL_STYLE, color);
  // new_object has already been added to created_objs

  Set_LTWH_Anims_On(new_object, Am_FILL_STYLE, true);

  // take care of undo/redo
  inter.Set(Am_VALUE, new_object);
  inter.Set(Am_OBJECT_MODIFIED, new_object);
  extern void Am_Copy_Values_To_Command(Am_Object from_object);
  Am_Copy_Values_To_Command(inter);
}

void
Undo_Redo_Selective(Am_Slot_Key allowed_slot, Am_Slot_Key method_slot,
                    const char *prompt_str)
{
  Am_Object last_command;
  bool use_new = (method_slot == Am_SELECTIVE_REPEAT_ON_NEW_METHOD);
  Am_Value_List l;
  l = undo_handler.Get(Am_COMMAND);
  Am_String s;
  Am_Object cmd, new_cmd;
  Am_Selective_Allowed_Method allowed_method;
  Am_Selective_New_Allowed_Method new_allowed_method;
  Am_Value current_selection, obj_modified;
  if (use_new) {
    new_allowed_method = undo_handler.Get(allowed_slot);
    //get the selection
    current_selection = my_selection.Peek(Am_VALUE);
  } else
    allowed_method = undo_handler.Get(allowed_slot);

  bool allowed;
  int cnt;
  for (l.Start(), cnt = 0; !l.Last(); l.Next(), cnt++) {
    cmd = l.Get();
    s = cmd.Get(Am_LABEL);
    obj_modified = cmd.Peek(Am_OBJECT_MODIFIED);
    std::cout << cnt << " " << s << " on " << obj_modified << " (cmd = " << cmd
              << ") ";
    if (use_new)
      allowed = new_allowed_method.Call(cmd, current_selection, Am_No_Value);
    else
      allowed = allowed_method.Call(cmd);
    if (allowed)
      std::cout << "OK\n";
    else
      std::cout << "NOT OK\n";
  }
  std::cout << "--Type index of command to " << prompt_str
            << " (or -1 to exit): " << std::flush;
  int which;
  std::cin >> which;
  if (which < 0)
    return;
  for (l.Start(), cnt = 0; cnt < which; l.Next(), cnt++)
    ;
  cmd = l.Get();
  if (use_new) {
    Am_Handler_Selective_Repeat_New_Method method;
    method = undo_handler.Get(method_slot);
    std::cout << prompt_str << " on cmd " << cmd << " method = " << method
              << " new obj = " << current_selection << std::endl
              << std::flush;
    new_cmd = method.Call(undo_handler, cmd, current_selection, Am_No_Value);
    std::cout << "  new_cmd = " << new_cmd << std::endl << std::flush;
  } else {
    Am_Handler_Selective_Undo_Method method;
    method = undo_handler.Get(method_slot);
    std::cout << prompt_str << " on cmd " << cmd << " method = " << method
              << std::endl
              << std::flush;
    new_cmd = method.Call(undo_handler, cmd);
    std::cout << "  new_cmd = " << new_cmd << std::endl << std::flush;
  }
}

Am_Define_Method(Am_Object_Method, void, change_setting, (Am_Object cmd))
{
  Am_Object inter = cmd.Get_Owner();
  Am_Input_Char c = inter.Get(Am_START_CHAR);

  std::cout << "---- got " << c << std::endl;
  switch (c.As_Char()) {
  case 'n': { //no value
    my_selection.Set(Am_VALUE, Am_Value_List());
    Am_Value v;
    v = my_selection.Peek(Am_VALUE);
    std::cout << "setting Am_VALUE of " << my_selection << " to " << v
              << std::endl
              << std::flush;
    break;
  }
  case 'l': { // set to one rectangle object
    if (last_created_obj.Valid()) {
      my_selection.Set(Am_VALUE, Am_Value_List().Add(last_created_obj));
      Am_Value v;
      v = my_selection.Peek(Am_VALUE);
      std::cout << "setting Am_VALUE of " << my_selection << " to " << v
                << " so only " << last_created_obj << std::endl
                << std::flush;
    }
    break;
  }
  case 'e': { // everything selected
    Am_Value_List l;
    l = created_objs.Get(Am_GRAPHICAL_PARTS);
    my_selection.Set(Am_VALUE, l);
    Am_Value v;
    v = my_selection.Peek(Am_VALUE);
    std::cout << "setting Am_VALUE of " << my_selection << " to " << v
              << std::endl;
    break;
  }
  case 'i': {
    static bool tracing = false;
    if (tracing)
      Am_Set_Inter_Trace(Am_INTER_TRACE_NONE);
    else
      Am_Set_Inter_Trace(Am_INTER_TRACE_ALL);
    tracing = !tracing;
    break;
  }
  case 'a': { // toggle active
    bool active = !(bool)my_selection.Get(Am_ACTIVE);
    std::cout << "setting Am_ACTIVE to " << active << std::endl << std::flush;
    my_selection.Set(Am_ACTIVE, active);
    break;
  }
  case 's': { // toggle queuing of selections
    Am_Object cmd = my_selection.Get_Object(Am_COMMAND);
    int queued = cmd.Get(Am_IMPLEMENTATION_PARENT);
    if (queued == Am_NOT_USUALLY_UNDONE) {
      std::cout << "Changing " << cmd << " so will be queued\n" << std::flush;
      cmd.Set(Am_IMPLEMENTATION_PARENT, 0);
    } else {
      std::cout << cmd << " parent now " << queued
                << " changing so will be NOT be queued\n"
                << std::flush;
      cmd.Set(Am_IMPLEMENTATION_PARENT, Am_NOT_USUALLY_UNDONE);
    }
    break;
  }
  case 'm': {
    static bool using_left = true;
    if (using_left) {
      using_left = false;
      std::cout << "switching to middle button\n" << std::flush;
      my_selection.Set(Am_START_WHEN, "ANY_MIDDLE_DOWN");
    } else {
      using_left = true;
      std::cout << "switching to left button\n" << std::flush;
      my_selection.Set(Am_START_WHEN, "ANY_LEFT_DOWN");
    }
    break;
  }

  case 'S': {
    static int line_styles_cnt = 1;
    switch (line_styles_cnt) {
    case 1:
      scroller.Set(Am_LINE_STYLE, Am_Red);
      break;
    case 2:
      scroller.Set(Am_LINE_STYLE, Am_Line_8);
      break;
    case 3:
      scroller.Set(Am_LINE_STYLE, Am_No_Style);
      break;
    case 4:
      scroller.Set(Am_LINE_STYLE, Am_Black);
      break;
    }
    line_styles_cnt = (line_styles_cnt % 4) + 1;
    std::cout << "Changing line style of scroller" << std::endl << std::flush;
    break;
  }

  case 'c': {
    static int handles_color_cnt = 1;
    Am_Style color;
    const char *colorstr = 0;
    switch (handles_color_cnt) {
    case 1:
      color = Am_Red;
      colorstr = "Red";
      break;
    case 2:
      color = Am_Green;
      colorstr = "Green";
      break;
    case 3:
      color = Am_Blue;
      colorstr = "Blue";
      break;
    case 4:
      color = Am_Black;
      colorstr = "Black";
      break;
    }
    handles_color_cnt = (handles_color_cnt % 4) + 1;
    std::cout << "Changing handles color to " << colorstr << std::endl
              << std::flush;
    my_selection.Set(Am_FILL_STYLE, color);
    break;
  }

  case 'u': {
    Am_Object last_command;
    std::cout << " checking undo...";
    std::cout << " undo handler = " << undo_handler << std::endl << std::flush;
    last_command = undo_handler.Get(Am_UNDO_ALLOWED);
    if (last_command.Valid()) {
      Am_String s;
      s = last_command.Get(Am_LABEL);
      std::cout << " undoing cmd " << last_command << " = " << s << std::endl
                << std::flush;
      Am_Object_Method undoit;
      undoit = undo_handler.Get(Am_PERFORM_UNDO);
      undoit.Call(undo_handler);
    } else
      std::cout << " nothing to undo\n" << std::flush;
    break;
  }
  case 'r': {
    Am_Object last_command;
    std::cout << " checking redo (undo the undo) ...";
    std::cout << " undo handler = " << undo_handler << std::endl << std::flush;
    last_command = undo_handler.Get(Am_REDO_ALLOWED);
    if (last_command.Valid()) {
      Am_String s;
      s = last_command.Get(Am_LABEL);
      std::cout << " re-doing cmd " << last_command << " = " << s << std::endl
                << std::flush;
      Am_Object_Method redoit;
      redoit = undo_handler.Get(Am_PERFORM_REDO);
      redoit.Call(undo_handler);
    } else
      std::cout << " nothing to redo\n" << std::flush;
    break;
  }
  case 'x': {
    Undo_Redo_Selective(Am_SELECTIVE_UNDO_ALLOWED, Am_SELECTIVE_UNDO_METHOD,
                        "Selective Undo");
    break;
  }
  case 'R': {
    Undo_Redo_Selective(Am_SELECTIVE_REPEAT_SAME_ALLOWED,
                        Am_SELECTIVE_REPEAT_SAME_METHOD, "Selective Repeat");
    break;
  }
  case 'L': {
    Am_Widget_Look look = Am_Widget_Group.Get(Am_WIDGET_LOOK);

    switch (look.value) {
    case Am_MOTIF_LOOK_val:
      look = Am_WINDOWS_LOOK;
      std::cout << "Setting Windows Look\n";
      break;

    case Am_WINDOWS_LOOK_val:
      look = Am_MACINTOSH_LOOK;
      std::cout << "Setting Macintosh Look\n";
      break;

    case Am_MACINTOSH_LOOK_val:
      look = Am_MOTIF_LOOK;
      std::cout << "Setting Motif Look\n";
      break;

    default:
      Am_Error("Unknown Look parameter");
      break;
    }

    Am_Set_Default_Look(look);
    break;
  }
  case 'N': {
    Undo_Redo_Selective(Am_SELECTIVE_REPEAT_NEW_ALLOWED,
                        Am_SELECTIVE_REPEAT_ON_NEW_METHOD,
                        "Selective Repeat on New");
    break;
  }
  case 'd': {
    my_undo_dialog.Set(Am_VISIBLE, true);
    Am_To_Top(my_undo_dialog);
    std::cout << "Displayed undo dialog box\n" << std::flush;
    break;
  }
  case 'J': {
    static int forward_back_cnt = 1;
    switch (forward_back_cnt) {
    case 1:
      std::cout << "Setting forward/backwards null (disabled)\n" << std::flush;
      my_selection.Set(Am_SELECTION_MOVE_CHARS, (0L));
      break;
    case 2:
      std::cout << "Setting forward/backwards to f,b,h,e\n" << std::flush;
      my_selection.Set(Am_SELECTION_MOVE_CHARS, Am_Value_List()
                                                    .Add("ANY_f") //forward
                                                    .Add("ANY_b") //backward
                                                    .Add("ANY_h") //home
                                                    .Add("ANY_e") //end
                       );
      break;
    case 3:
      std::cout << "Setting forward/backwards to default\n" << std::flush;
      my_selection.Set(Am_SELECTION_MOVE_CHARS,
                       Am_Selection_Widget.Get(Am_SELECTION_MOVE_CHARS));
      break;
    }
    forward_back_cnt = (forward_back_cnt % 3) + 1;
    break;
  }
  /*
  case 'f': {
    Am_Object inter;
    Am_Value feedback;
    inter = my_selection.Get_Object(Am_MOVE_INTERACTOR);
    feedback=inter.Peek(Am_FEEDBACK_OBJECT);
    if (feedback.Valid()) {
      std::cout << "Turning off feedback\n" << std::flush;
      inter.Set(Am_FEEDBACK_OBJECT, (0L));
    }
    else {
      std::cout << "Turning on feedback\n" << std::flush;
      inter.Set(Am_FEEDBACK_OBJECT, Am_Compute_MG_Feedback_Object);
    }
    break;
  }
  */
  case 'q':
    Am_Exit_Main_Event_Loop();
    break;
  case 'p': {
    if (special_style == Am_No_Style) {
      special_style = Am_Red;
      std::cout << "Setting special style to be red\n" << std::flush;
    } else {
      special_style = Am_No_Style;
      std::cout << "Setting special style to be (0L)\n" << std::flush;
    }
    my_selection.Set(Am_SELECT_CLOSEST_POINT_STYLE, special_style);
    break;
  }
  case 'z':
  case 'Z': {
    static int inactive_which = 1;
    static int active_which = 1;
    bool inactive = (c.As_Char() == 'z');
    int cnt = (inactive ? inactive_which : active_which);
    Am_Value_List vl = my_selection.Get(Am_VALUE);
    if (vl.Empty())
      std::cout << "List empty\n" << std::flush;
    else {
      Am_Slot_Key slot = 0;
      const char *name = 0;
      vl.Start();
      Am_Object obj = vl.Get();
      Am_Value v;
      Am_Object cmd;
      v = obj.Peek(Am_INACTIVE_COMMANDS);
      if (v.Valid())
        cmd = v;
      else
        obj.Add_Part(Am_INACTIVE_COMMANDS, cmd = Am_Command.Create());

      switch (cnt) {
      case 1:
        slot = Am_MOVE_INACTIVE;
        name = "MOVE";
        break;
      case 2:
        slot = Am_GROW_INACTIVE;
        name = "GROW";
        break;
      case 3:
        slot = Am_SELECT_INACTIVE;
        name = "SELECT";
        break;
      case 4:
        slot = Am_CLEAR_INACTIVE;
        name = "CLEAR";
        break;
      case 5:
        slot = Am_COPY_INACTIVE;
        name = "COPY";
        break;
      case 6:
        slot = Am_CUT_INACTIVE;
        name = "CUT";
        break;
      case 7:
        slot = Am_DUPLICATE_INACTIVE;
        name = "DUPLICATE";
        break;
      case 8:
        slot = Am_TO_TOP_INACTIVE;
        name = "TO_TOP";
        break;
      case 9:
        slot = Am_TO_BOTTOM_INACTIVE;
        name = "TO_BOTTOM";
        break;
      case 10:
        slot = Am_GROUP_INACTIVE;
        name = "GROUP";
        break;
      case 11:
        slot = Am_UNGROUP_INACTIVE;
        name = "UNGROUP";
        break;
      }
      std::cout << "For object " << obj << " setting cmd " << cmd << " slot "
                << name << "=" << slot << " to be " << inactive << std::endl
                << std::flush;
      cmd.Set(slot, inactive);
      if (inactive)
        inactive_which = (inactive_which % 11) + 1;
      else
        active_which = (active_which % 11) + 1;
    }
    break;
  }
  default:
    std::cout
        << "** Illegal, want:\n"
        << "   a = toggle active\n"
        /*         << "   A = toggle animations for moving\n" **dont work */
        //       << "   b =\n"
        //       << "   B =\n"
        << "   c = change selection handle color\n"
        //       << "   C =\n"
        << "   d = display undo dialog box\n"
        //       << "   D =\n"
        << "   e = everything selected\n"
        //       << "   E =\n"
        << "   f = toggle whether move uses feedback object or not\n"
        //       << "   F =\n"
        //       << "   g =\n"
        //       << "   G =\n"
        //       << "   h =\n"
        //       << "   H =\n"
        << "   i = inter debug\n"
        //       << "   I =\n"
        //       << "   j =\n"
        << "   J = toggle whether selection forward/backward is enabled\n"
        //       << "   k =\n"
        //       << "   K =\n"
        << "   l = last object selected\n"
        << "   L = toggle default widget look\n"
        << "   m = switch to middle mouse button, and back\n"
        //       << "   M =\n"
        << "   n = no selection\n"
        << "   N = repeat new\n"
        //       << "   o =\n"
        //       << "   O =\n"
        << "   p = toggle show closest point in red\n"
        //       << "   P =\n"
        << "   q = quit\n"
        //       << "   Q =\n"
        << "   r = redo\n"
        << "   R = selective repeat same\n"
        << "   s = toggle queuing of selections for undo\n"
        << "   S = line style of scrolling group\n"
        //       << "   t =\n"
        //       << "   T =\n"
        << "   u = undo\n"
        //       << "   U =\n"
        //       << "   v =\n"
        //       << "   V =\n"
        //       << "   w =\n"
        //       << "   W =\n"
        << "   x = selective undo\n"
        //       << "   X =\n"
        //       << "   y =\n"
        //       << "   Y =\n"
        << "   z = toggle inactive object\n"
        << "   Z = toggle Active object\n"
        << std::endl;
    break;
  } // end switch
}

Am_Define_Method(Am_Object_Method, void, my_do, (Am_Object cmd))
{
  Am_Value value;
  value = cmd.Peek(Am_VALUE);
  std::cout << "\n+-+-+- Command " << cmd << " value = " << value << std::endl
            << std::flush;
}
Am_Define_Method(Am_Object_Method, void, my_do_sel, (Am_Object cmd))
{
  Am_Value value;
  value = cmd.Peek(Am_VALUE);
  std::cout << "\n+-+-+- Command " << cmd << " value = " << value << std::endl
            << std::flush;
  if (special_style != Am_No_Style) {
    Am_Object which_obj = my_selection.Get(Am_SELECT_CLOSEST_POINT_OBJ);
    Am_Move_Grow_Where_Attach attach =
        my_selection.Get(Am_SELECT_CLOSEST_POINT_WHERE);
    std::cout << "  Attach on " << which_obj << " at " << attach << std::endl
              << std::flush;
  }
}

////// Freehand drawing

Am_Define_Method(Am_Object_Method, void, freehand_create, (Am_Object cmd))
{
  Am_Object inter;
  inter = cmd.Get(Am_SAVED_OLD_OWNER);
  int left = inter.Get(Am_MIN_X);
  int top = inter.Get(Am_MIN_Y);
  int width = (int)inter.Get(Am_MAX_X) - left;
  int height = (int)inter.Get(Am_MAX_Y) - top;
  Am_Inter_Location data(false, inter.Get_Owner(), left, top, width, height);

  Am_Style color = color_panel.Get_Object(Am_VALUE).Get(Am_FILL_STYLE);

  Am_Object new_object = freehand_line_proto.Create()
                             .Set(Am_POINT_LIST, inter.Get(Am_POINT_LIST))
                             .Set(Am_LINE_STYLE, color);
  created_objs.Add_Part(new_object);

  Set_LTWH_Anims_On(new_object, Am_LINE_STYLE, true);

  std::cout << "\n--++-- Created new freehand " << new_object << " at (" << left
            << "," << top << ")\n"
            << std::flush;

  // Set appropriate slots in implementation parent (to be called next).
  // This parent command is actually the New_Points_Interactor's
  // implementation command.
  Am_Object parent_cmd;
  parent_cmd = cmd.Get(Am_IMPLEMENTATION_PARENT);
  parent_cmd.Set(Am_IMPLEMENTATION_PARENT, 0)
      .Set(Am_SAVED_OLD_OWNER, create_inter, Am_OK_IF_NOT_THERE)
      .Set(Am_OBJECT_MODIFIED, new_object)
      .Set(Am_START_OBJECT, new_object.Copy())
      .Set(Am_VALUE, new_object)
      .Set(Am_AS_LINE, true)
      .Set(Am_INTERIM_VALUE, data)
      .Set(Am_HAS_BEEN_UNDONE, false);
}

////// Gesture stuff

// the Do Method for an unrecognized gesture
Am_Define_Method(Am_Object_Method, void, gesture_unrecognized, (Am_Object cmd))
{
  Am_Object inter;
  inter = cmd.Get(Am_SAVED_OLD_OWNER);
  std::cout << "** UNRECOGNIZED" << std::endl;
  std::cout << "(nap = " << (double)inter.Get(Am_NONAMBIGUITY_PROB)
            << ", dist=" << (double)inter.Get(Am_DIST_TO_MEAN) << ")"
            << std::endl;
}

// the create method for a CIRCLE, RECTANGLE, or LINE gesture
Am_Define_Method(Am_Create_New_Object_Method, Am_Object, gesture_creator,
                 (Am_Object cmd, Am_Inter_Location data, Am_Object old_object))
{
  Am_Object new_object;
  if (old_object.Valid()) {
    new_object = old_object.Copy();
    std::cout << "\n--++-- Gesture created new object " << new_object
              << " as a copy of " << old_object << std::endl
              << std::flush;
    created_objs.Add_Part(new_object);
  } else {
    Am_Object prototype = cmd.Get(NEW_OBJECT_PROTOTYPE);
    new_object = create_new_obj_internal(prototype, data);
  }
  return new_object;
}

Am_Define_Formula(int, scroll_height)
{
  return (int)self.Get_Owner().Get(Am_HEIGHT) - 10 - (int)self.Get(Am_TOP);
}
Am_Define_Formula(int, scroll_width)
{
  return (int)self.Get_Owner().Get(Am_WIDTH) - 10 - (int)self.Get(Am_LEFT);
}

#if defined(_WINDOWS) || defined(_MACINTOSH)

#define AMULET_BITMAP "images/amside.gif"
#define ARROW_BITMAP "images/arrow.gif"
#else
#define AMULET_BITMAP "images/amuletside.xbm"
#define ARROW_BITMAP "images/arrow.xbm"
#endif

Am_Image_Array icon, arrow;

void
load_bitmaps()
{
  const char *pathname = Am_Merge_Pathname(AMULET_BITMAP);
  icon = Am_Image_Array(pathname);
  delete[] pathname;
  if (!icon.Valid())
    Am_Error("Amulet Icon bitmap image not found");
  amulet_icon = Am_Bitmap.Create("Amulet_Icon")
                    .Set(Am_IMAGE, icon)
                    .Set(Am_LINE_STYLE, Am_Black);
  circle_bm = Am_Arc.Create("Rect not bitmap")
                  .Set(Am_LEFT, 0)
                  .Set(Am_TOP, 0)
                  .Set(Am_WIDTH, 16)
                  .Set(Am_HEIGHT, 16)
                  .Set(Am_LINE_STYLE, Am_Black)
                  .Set(Am_FILL_STYLE, Am_Blue);

  pathname = Am_Merge_Pathname(ARROW_BITMAP);
  arrow = Am_Image_Array(pathname);
  delete[] pathname;
  if (!arrow.Valid())
    Am_Error("Arrow bitmap image not found");
  arrow_bm = Am_Bitmap.Create("Arrow bitmap")
                 .Set(Am_IMAGE, arrow)
                 .Set(Am_LINE_STYLE, Am_Black);

  line_bm = Am_Line.Create("Line, not bitmap")
                .Set(Am_LEFT, 0)
                .Set(Am_TOP, 0)
                .Set(Am_WIDTH, 16)
                .Set(Am_HEIGHT, 8);

  rect_bm = Am_Rectangle.Create("Rect not bitmap")
                .Set(Am_LEFT, 0)
                .Set(Am_TOP, 0)
                .Set(Am_WIDTH, 12)
                .Set(Am_HEIGHT, 8)
                .Set(Am_LINE_STYLE, Am_Black)
                .Set(Am_FILL_STYLE, Am_Red);

  poly_bm = Am_Polygon.Create("Polygon not bitmap")
                .Set(Am_POINT_LIST, Am_Point_List()
                                        // an irregular closed polygon
                                        .Add(10.0f, 0.0f)
                                        .Add(0.0f, 30.0f)
                                        .Add(30.0f, 40.0f)
                                        .Add(30.0f, 20.0f)
                                        .Add(15.0f, 20.0f)
                                        .Add(10.0f, 0.0f))
                .Set(Am_WIDTH, 16)
                .Set(Am_HEIGHT, 16)
                .Set(Am_LEFT, 0)
                .Set(Am_TOP, 0)
                .Set(Am_LINE_STYLE, Am_Black)
                .Set(Am_FILL_STYLE, Am_Green);

  // (I sketched a curly-que freehand, then copied the points below.)
  static int ar[] = {133, 127, 122, 159, 121, 195, 126, 211, 140, 229,
                     156, 239, 190, 243, 208, 220, 200, 182, 196, 179,
                     194, 179, 168, 186, 144, 201, 135, 217, 132, 235,
                     134, 255, 143, 267, 195, 293, 219, 305, 249, 306};
  freehand_bm =
      Am_Polygon.Create("Freehand not bitmap")
          .Set(Am_POINT_LIST, Am_Point_List(ar, sizeof(ar) / sizeof(int)))
          .Set(Am_WIDTH, 16)
          .Set(Am_HEIGHT, 16)
          .Set(Am_LEFT, 0)
          .Set(Am_TOP, 0)
          .Set(Am_FILL_STYLE, Am_No_Style)
          .Set(Am_LINE_STYLE, Am_Black);
  text_bm = Am_Text.Create("text not bitmap")
                .Set(Am_TEXT, "T")
                .Set(Am_FONT,
                     Am_Font(Am_FONT_SERIF, true, false, false, Am_FONT_LARGE))
                .Set(Am_LINE_STYLE, Am_Black);
}

Am_Define_Method(Am_Object_Method, void, make_no_selection, (Am_Object /*cmd*/))
{
  my_selection.Set(Am_VALUE, (0L));
}

Am_Define_Method(Am_Get_Object_Property_Value_Method, void, get_obj_color,
                 (Am_Object /* command_obj */, Am_Object obj,
                  Am_Value &old_value))
{
  if (obj.Is_Instance_Of(Am_Line) || obj.Is_Instance_Of(Am_Text))
    old_value = obj.Peek(Am_LINE_STYLE);
  else
    old_value = obj.Peek(Am_FILL_STYLE);
}
Am_Define_Method(Am_Set_Object_Property_Value_Method, void, set_obj_color,
                 (Am_Object /* command_obj */, Am_Object obj,
                  Am_Value new_value))
{
  if (obj.Is_Instance_Of(Am_Line) || obj.Is_Instance_Of(freehand_line_proto) ||
      obj.Is_Instance_Of(Am_Text))
    obj.Set(Am_LINE_STYLE, new_value);
  else
    obj.Set(Am_FILL_STYLE, new_value);
}

/*
Am_Define_Method(Am_Object_Method, void, align_vertical_command,
                 (Am_Object cmd)) {
  Am_Object selection_widget, group, obj, place;
  Am_Value_List selected_objs, selected_objs_places;
  selection_widget = cmd.Get(Am_SELECTION_WIDGET);
  // Am_Get_Selection_In_Display_Order(selection_widget, selected_objs, group);
  // don't want them in display order--want them in selected order
  selected_objs = selection_widget.Get(Am_VALUE);
  cmd.Set(Am_OBJECT_MODIFIED, selected_objs);
  cmd.Set(Am_SAVED_OLD_OBJECT_OWNER, group, Am_OK_IF_NOT_THERE);
  cmd.Set(Am_HAS_BEEN_UNDONE, false);

  selected_objs.Start();
  obj = selected_objs.Get();
  int left = obj.Get(Am_LEFT);
  selected_objs.Next();
  for( ; !selected_objs.Last(); selected_objs.Next()) {
    obj = selected_objs.Get();
    // ** save old values for undo
    obj.Set(Am_LEFT, left);
  }
}
*/

/////////////////////// save load stuff /////////////////////////

// This method should take the supplied contents list and add it to
// the window, after removing what is already there
Am_Define_Method(Am_Handle_Loaded_Items_Method, void, use_file_contents,
                 (Am_Object /* command */, Am_Value_List &contents))
{
  Am_Value_List current = created_objs.Get(Am_GRAPHICAL_PARTS);
  Am_Object item;
  //first delete all of the current contents of the window
  for (current.Start(); !current.Last(); current.Next()) {
    item = current.Get();
    item.Destroy();
  }
  //now add the new objects
  for (contents.Start(); !contents.Last(); contents.Next()) {
    item = contents.Get();
    created_objs.Add_Part(item);
  }
}

//This method should return the list of objects to save
Am_Define_Method(Am_Items_To_Save_Method, Am_Value_List, contents_for_save,
                 (Am_Object /* command */))
{
  Am_Value_List obs_to_save = created_objs.Get(Am_GRAPHICAL_PARTS);
  return obs_to_save;
}

Am_Define_No_Self_Formula(bool, should_do_animation)
{
  bool anim_on = animations_command.Get(Am_VALUE);
  //only if this is a top-level object
  if (anim_on)
    return true; // && self.Get_Owner() == created_objs)
  else
    return false;
}

#include <fstream> //for load and save debug
int
main(int argc, char *argv[])
{

  Am_Object cmd;
  Am_Object open_command;

  Am_Initialize();

  load_bitmaps();

  undo_handler = Am_Multiple_Undo_Object.Create("my_multi_undo");

  animations_command = Am_Cycle_Value_Command.Create("animations_enabled")
                           .Set(Am_LABEL_LIST, Am_Value_List()
                                                   .Add("Turn Animations On")
                                                   .Add("Turn Animations Off"));

  animator_proto =
      Am_Animator.Create("animator_proto").Set(Am_ACTIVE, should_do_animation);
  color_anim_proto = Am_Style_Animator.Create("fill_style_anim_proto")
                         .Set(Am_ACTIVE, should_do_animation);
  my_arc_proto = Am_Arc.Create("Sel_Circle_Proto")
                     .Set(Am_LEFT, 0)
                     .Set(Am_TOP, 0)
                     .Set(Am_FILL_STYLE, Am_Black)
                     .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
                     .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                                .Add(Am_LEFT)
                                                .Add(Am_TOP)
                                                .Add(Am_WIDTH)
                                                .Add(Am_HEIGHT)
                                                .Add(Am_FILL_STYLE));
  Set_LTWH_Anims_On(my_arc_proto, Am_FILL_STYLE, true);
  my_rectangle_proto = Am_Rectangle.Create("Sel_Rect_Proto")
                           .Set(Am_LEFT, 0)
                           .Set(Am_TOP, 0)
                           .Set(Am_FILL_STYLE, Am_Black)
                           .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
                           .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                                      .Add(Am_LEFT)
                                                      .Add(Am_TOP)
                                                      .Add(Am_WIDTH)
                                                      .Add(Am_HEIGHT)
                                                      .Add(Am_FILL_STYLE));
  Set_LTWH_Anims_On(my_rectangle_proto, Am_FILL_STYLE, true);
  my_line_proto =
      Am_Line.Create("Sel_Line_Proto")
          .Set(Am_LEFT, 0)
          .Set(Am_TOP, 0)
          .Set(Am_LINE_STYLE, Am_Black)
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
          .Add(Am_SLOTS_TO_SAVE,
               Am_Value_List().Add(Am_X1).Add(Am_Y1).Add(Am_X2).Add(Am_Y2).Add(
                   Am_LINE_STYLE));
  my_polygon_proto =
      Am_Polygon.Create("Sel_Polygon_Proto")
          .Set(Am_LEFT, 0)
          .Set(Am_TOP, 0)
          .Set(Am_FILL_STYLE, Am_Black)
          .Set(Am_FILL_STYLE, Am_No_Style)
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
          .Add(Am_SLOTS_TO_SAVE,
               Am_Value_List().Add(Am_POINT_LIST).Add(Am_FILL_STYLE));
  freehand_line_proto =
      Am_Polygon.Create("Sel_Freehand_Proto")
          .Set(Am_LEFT, 0)
          .Set(Am_TOP, 0)
          .Set(Am_LINE_STYLE, Am_Black)
          .Set(Am_FILL_STYLE, Am_No_Style)
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
          .Add(Am_SLOTS_TO_SAVE,
               Am_Value_List().Add(Am_POINT_LIST).Add(Am_LINE_STYLE));
  my_text_proto =
      Am_Text.Create("Text_Proto")
          .Set(Am_LEFT, 0)
          .Set(Am_TOP, 0)
          .Set(Am_LINE_STYLE, Am_Black)
          .Set(Am_TEXT, "")
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
          .Add(Am_SLOTS_TO_SAVE,
               Am_Value_List().Add(Am_LEFT).Add(Am_TOP).Add(Am_TEXT).Add(
                   Am_LINE_STYLE));
  Set_LTWH_Anims_On(my_text_proto, Am_LINE_STYLE, false);

  Set_LTWH_Anims_On(Am_Resize_Parts_Group, 0, false); //wh doesn't work

  //tell the default loader what to call a rect_proto in the file
  Am_Default_Load_Save_Context.Register_Prototype("ARC", my_arc_proto);
  Am_Default_Load_Save_Context.Register_Prototype("RECT", my_rectangle_proto);
  Am_Default_Load_Save_Context.Register_Prototype("LIN", my_line_proto);
  Am_Default_Load_Save_Context.Register_Prototype("POLY", my_polygon_proto);
  Am_Default_Load_Save_Context.Register_Prototype("FREE", freehand_line_proto);
  Am_Default_Load_Save_Context.Register_Prototype("TEXT", my_text_proto);

  // ** TEMP
  Am_TEMP_Register_Prototype("ARC", my_arc_proto);
  Am_TEMP_Register_Prototype("RECT", my_rectangle_proto);
  Am_TEMP_Register_Prototype("LIN", my_line_proto);
  Am_TEMP_Register_Prototype("POLY", my_polygon_proto);
  Am_TEMP_Register_Prototype("FREE", freehand_line_proto);
  Am_TEMP_Register_Prototype("TEXT", my_text_proto);
  Am_TEMP_Register_Prototype("GROUP", Am_Resize_Parts_Group);

  window =
      Am_Window.Create("window")
          .Set(Am_LEFT, 20)
          .Set(Am_TOP, 45)
          .Set(Am_WIDTH, 500)
          .Set(Am_TITLE, "Amulet Test Selection Widget")
          .Set(Am_ICON_TITLE, "Test Selection Widget")
          .Set(Am_HEIGHT, 530)
          .Set(Am_UNDO_HANDLER, undo_handler)
          .Set(Am_FILL_STYLE, Am_Default_Color)
          //always exit when destroy the main window
          .Set(Am_DESTROY_WINDOW_METHOD, Am_Window_Destroy_And_Exit_Method);

  grid_command =
      Am_Cycle_Value_Command.Create("grid")
          .Set(Am_SHORT_LABEL, "Grid On/Off")
          .Set(Am_LABEL_LIST,
               Am_Value_List().Add("Turn Grid On").Add("Turn Grid Off"));

  /*
  Am_Object Am_Graphics_Align_Vertical_Command = Am_Command.Create("align_vertical")
    .Add(Am_SELECTION_WIDGET, (0L))
    .Set(Am_LABEL, "Align Left")
    .Set(Am_ACTIVE, Am_Active_If_Selection)
    .Set(Am_DO_METHOD, align_vertical_command)
    .Set(Am_ACCELERATOR, Am_Input_Char("CONTROL_|"))
    .Set(Am_IMPLEMENTATION_PARENT, true) //not yet undoable
    ;
*/
  scroller = Am_Scrolling_Group.Create("scroller")
                 .Set(Am_LEFT, 55)
                 .Set(Am_TOP, 40)
                 .Set(Am_INNER_WIDTH, 1000)
                 .Set(Am_INNER_HEIGHT, 1000)
                 .Set(Am_INNER_FILL_STYLE, Am_White)
                 .Set(Am_WIDTH, scroll_width)
                 .Set(Am_HEIGHT, scroll_height);
  created_objs = Am_Group.Create("created_objs")
                     .Set(Am_LEFT, 0)
                     .Set(Am_TOP, 0)
                     .Set(Am_WIDTH, 1000)
                     .Set(Am_HEIGHT, 1000);

  window.Add_Part(scroller);
  scroller.Add_Part(created_objs);

  tool_panel = Am_Button_Panel.Create("tool panel")
                   .Set(Am_LEFT, 10)
                   .Set(Am_TOP, 40)
                   .Set(Am_FIXED_HEIGHT, true)
                   .Set(Am_FINAL_FEEDBACK_WANTED, true)
                   .Set(Am_ITEMS, Am_Value_List()
                                      .Add(arrow_bm)
                                      .Add(line_bm)
                                      .Add(rect_bm)
                                      .Add(circle_bm)
                                      .Add(poly_bm)
                                      .Add(freehand_bm)
                                      .Add(text_bm))
                   .Set(Am_VALUE, arrow_bm);
  tool_panel.Get_Object(Am_COMMAND)
      .Set(Am_DO_METHOD, make_no_selection)
      .Set(Am_IMPLEMENTATION_PARENT, true) //don't queue for UNDO
      ;

  window.Add_Part(tool_panel);

  Am_Object color_proto = Am_Rectangle.Create("color proto")
                              .Set(Am_WIDTH, 16)
                              .Set(Am_HEIGHT, 16)
                              .Set(Am_LINE_STYLE, Am_No_Style)
                              .Set(Am_FILL_STYLE, Am_Black);

  rfeedback = Am_Rectangle.Create("rfeedback")
                  .Set(Am_FILL_STYLE, 0)
                  .Set(Am_LINE_STYLE, Am_Dotted_Line)
                  .Set(Am_VISIBLE, 0);
  lfeedback = Am_Line.Create("lfeedback")
                  .Set(Am_LINE_STYLE, // thick dotted line
                       Am_Style(0.0f, 0.0f, 0.0f, 6, Am_CAP_BUTT, Am_JOIN_MITER,
                                Am_LINE_ON_OFF_DASH))
                  .Set(Am_VISIBLE, 0);
  scroller.Add_Part(lfeedback).Add_Part(rfeedback);

  create_inter = Am_New_Points_Interactor
                     .Create("create_objects")
                     //    .Set(Am_START_WHEN, "ANY_RIGHT_DOWN")
                     .Set(Am_AS_LINE, line_tool)
                     .Set(Am_FEEDBACK_OBJECT, compute_feedback_obj)
                     .Set(Am_CREATE_NEW_OBJECT_METHOD, create_new_object)
                     .Set(Am_ACTIVE, rubber_bandable_tool_is_selected)
                     .Set(Am_GRID_X, grid_if_should)
                     .Set(Am_GRID_Y, grid_if_should);
  created_objs.Add_Part(create_inter);

  create_poly_inter = Am_New_Points_Interactor.Create("create_polygons")
                          .Set(Am_START_WHEN, "ANY_SINGLE_LEFT_DOWN")
                          .Set(Am_STOP_WHEN, "ANY_DOUBLE_LEFT_DOWN")
                          .Set(Am_AS_LINE, true)
                          .Set(Am_FEEDBACK_OBJECT, lfeedback)
                          .Set(Am_START_DO_METHOD, polygon_start)
                          .Set(Am_INTERIM_DO_METHOD, polygon_interim_do)
                          .Set(Am_DO_METHOD, polygon_do)
                          .Set(Am_ABORT_DO_METHOD, polygon_abort)
                          .Set(Am_CREATE_NEW_OBJECT_METHOD, create_new_object)
                          .Set(Am_GRID_X, grid_if_should)
                          .Set(Am_GRID_Y, grid_if_should)
                          .Set(Am_ACTIVE, polygon_tool_is_selected);
  created_objs.Add_Part(create_poly_inter);

  Am_Object change_settings_inter =
      Am_One_Shot_Interactor.Create("change_settings")
          .Set(Am_START_WHEN, "ANY_KEYBOARD")
          .Set(Am_PRIORITY, 0.9) // so it runs after the menu accelerators
      ;
  change_settings_inter.Get_Object(Am_COMMAND)
      .Set(Am_DO_METHOD, change_setting)
      .Set(Am_IMPLEMENTATION_PARENT, true) //not undo-able
      .Set_Name("change_settings_command");
  window.Add_Part(change_settings_inter);

  my_selection = Am_Selection_Widget.Create("my_selection")
                     .Set(Am_OPERATES_ON, created_objs)
                     .Set(Am_ACTIVE, selection_tool)
                     .Set(Am_GRID_X, grid_if_should)
                     .Set(Am_GRID_Y, grid_if_should);
  my_selection.Get_Object(Am_COMMAND).Set(Am_DO_METHOD, my_do_sel);
  my_selection.Get_Object(Am_MOVE_GROW_COMMAND).Set(Am_DO_METHOD, my_do);

  scroller.Add_Part(my_selection);

  Am_Object black_color_item;
  color_panel =
      Am_Button_Panel.Create("color_panel")
          .Set(Am_LEFT, 10)
          .Set(Am_TOP, 300)
          .Set(Am_FINAL_FEEDBACK_WANTED, true)
          .Set(Am_ITEMS,
               Am_Value_List()
                   .Add(black_color_item =
                            color_proto.Create().Set(Am_FILL_STYLE, Am_Black))
                   .Add(color_proto.Create().Set(Am_FILL_STYLE, Am_White))
                   .Add(color_proto.Create().Set(Am_FILL_STYLE, Am_Red))
                   .Add(color_proto.Create().Set(Am_FILL_STYLE, Am_Orange))
                   .Add(color_proto.Create().Set(Am_FILL_STYLE, Am_Yellow))
                   .Add(color_proto.Create().Set(Am_FILL_STYLE, Am_Green))
                   .Add(color_proto.Create().Set(Am_FILL_STYLE, Am_Blue)))
          .Set_Part(Am_COMMAND,
                    Am_Graphics_Set_Property_Command.Create("Change_Color")
                        .Set(Am_LABEL, "Change color")
                        .Set(Am_GET_OBJECT_VALUE_METHOD, get_obj_color)
                        .Set(Am_SET_OBJECT_VALUE_METHOD, set_obj_color)
                        .Set(Am_SLOT_FOR_VALUE, (int)Am_FILL_STYLE)
                        .Set(Am_SELECTION_WIDGET, my_selection));
  color_panel.Set(Am_VALUE, black_color_item);

  window.Add_Part(color_panel);

  Am_Scripting_Register_Palette(undo_handler, tool_panel,
                                Am_Am_Slot_Key(Am_PROTOTYPE));
  Am_Scripting_Register_Palette(undo_handler, color_panel, Am_Value(Am_Red));

  my_undo_dialog = Am_Undo_Dialog_Box.Create("My_Undo_Dialog")
                       .Set(Am_LEFT, 550)
                       .Set(Am_TOP, 200)
                       .Set(Am_UNDO_HANDLER_TO_DISPLAY, undo_handler)
                       .Set(Am_SELECTION_WIDGET, my_selection)
                       .Set(Am_SCROLLING_GROUP_SLOT, scroller)
                       .Set(Am_VISIBLE, false);
  Am_Screen.Add_Part(my_undo_dialog);
  Am_Object search_dialog = Am_Create_Search_Dialog_For(undo_handler)
                                .Set(Am_SELECTION_WIDGET, my_selection);
  Am_Value_List l =
      Am_Value_List()
          .Add(Am_Command.Create("Amulet_Command")
                   .Set(Am_IMPLEMENTATION_PARENT, true) //not undoable
                   .Set(Am_DO_METHOD, my_do)
                   .Set(Am_LABEL, amulet_icon)
                   .Set(Am_ITEMS,
                        Am_Value_List().Add(Am_About_Amulet_Command.Create())))
          .Add(Am_Command.Create("File_Command")
                   .Set(Am_IMPLEMENTATION_PARENT, true) //not undoable
                   .Set(Am_LABEL, "File")
                   .Set(Am_DO_METHOD, my_do)
                   .Set(Am_ITEMS,
                        Am_Value_List()
                            .Add(open_command = Am_Open_Command.Create().Set(
                                     Am_HANDLE_OPEN_SAVE_METHOD,
                                     use_file_contents))
                            .Add(Am_Save_As_Command.Create().Set(
                                Am_HANDLE_OPEN_SAVE_METHOD, contents_for_save))
                            .Add(Am_Save_Command.Create().Set(
                                Am_HANDLE_OPEN_SAVE_METHOD, contents_for_save))
                            .Add(Am_Quit_No_Ask_Command.Create())))
          .Add(
              Am_Command.Create("Edit_Command")
                  .Set(Am_LABEL, "Edit")
                  .Set(Am_DO_METHOD, my_do)
                  .Set(Am_IMPLEMENTATION_PARENT, true) //not undoable
                  .Set(Am_ITEMS,
                       Am_Value_List()
                           .Add(Am_Undo_Command.Create())
                           .Add(Am_Redo_Command.Create())
                           .Add(Am_Show_Undo_Dialog_Box_Command.Create().Add(
                               Am_UNDO_DIALOG_BOX_SLOT, my_undo_dialog))
                           .Add(Am_Menu_Line_Command.Create())
                           .Add(cut_command = Am_Graphics_Cut_Command.Create())
                           .Add(Am_Graphics_Copy_Command.Create())
                           .Add(Am_Graphics_Paste_Command.Create())
                           .Add(Am_Graphics_Clear_Command.Create())
                           .Add(Am_Graphics_Clear_All_Command.Create())
                           .Add(Am_Menu_Line_Command.Create())
                           .Add(Am_Graphics_Duplicate_Command.Create())
                           .Add(Am_Selection_Widget_Select_All_Command.Create())
                           .Add(Am_Menu_Line_Command.Create())
                           .Add(Am_Show_Search_Command.Create().Set(
                               Am_SEARCH_DIALOG, search_dialog))));
  //Visual C++ can't handle long statements
  l.Add(
      Am_Command.Create("Arrange_Command")
          .Set(Am_LABEL, "Arrange")
          .Set(Am_DO_METHOD, my_do)
          .Set(Am_IMPLEMENTATION_PARENT, true) //not undoable
          .Set(
              Am_ITEMS, Am_Value_List()
                            .Add(Am_Graphics_To_Top_Command.Create())
                            .Add(Am_Graphics_To_Bottom_Command.Create())
                            .Add(Am_Menu_Line_Command.Create())
                            .Add(Am_Graphics_Group_Command.Create())
                            .Add(Am_Graphics_Ungroup_Command.Create())
                            .Add(Am_Menu_Line_Command.Create())
                            .Add(grid_command)
                            .Add(Am_Menu_Line_Command.Create())
                            .Add(animations_command)
              /*                      .Add (Am_Graphics_Align_Vertical_Command.Create())
*/
              ));
  menu_bar = Am_Menu_Bar.Create("menu_bar")
                 .Set(Am_SELECTION_WIDGET, my_selection)
                 .Set(Am_ITEMS, l);
  window.Add_Part(menu_bar);

  Am_Object gesture_feedback =
      Am_Polygon.Create("gesture feedback").Set(Am_FILL_STYLE, Am_No_Style);
  scroller.Add_Part(gesture_feedback);

  Am_Object new_points_impl_cmd =
      Am_New_Points_Interactor.Get_Object(Am_IMPLEMENTATION_COMMAND);
  Am_Object freehand_inter = Am_Gesture_Interactor.Create("gesture")
                                 .Set(Am_START_WHEN, "any_left_down")
                                 .Set(Am_FEEDBACK_OBJECT, gesture_feedback)
                                 .Set(Am_ACTIVE, freehand_tool_is_selected);
  freehand_inter.Get_Object(Am_COMMAND)
      .Set(Am_DO_METHOD, freehand_create)
      .Set(Am_IMPLEMENTATION_PARENT, new_points_impl_cmd);
  created_objs.Add_Part(freehand_inter);

  Am_Object text_create_inter =
      Am_Text_Create_Interactor.Create("my_text_create")
          .Set(Am_ACTIVE, text_is_selected)
          .Set(Am_START_WHERE_TEST, Am_Inter_Not_In_Text_Leaf)
          .Add(NEW_OBJECT_PROTOTYPE, my_text_proto)
          .Set(Am_CREATE_NEW_OBJECT_METHOD, create_new_object)
          .Set(Am_GRID_X, grid_if_should)
          .Set(Am_GRID_Y, grid_if_should);
  Am_Object text_edit_inter = Am_Text_Edit_Interactor.Create("my_text_edit")
                                  .Set(Am_ACTIVE, text_is_selected);
  created_objs.Add_Part(text_create_inter).Add_Part(text_edit_inter);

  //  char	pathname[] = "f:\\openamulet\\data\\select.cl";
  const char *pathname = Am_Merge_Pathname(
      CLASSIFIER_FILENAME); // This should be portable no ? -- ortalo
  // at least... try to do so...
  Am_Gesture_Classifier gc(pathname);
  //  delete [] pathname;
  if (!gc.Valid())
    Am_Error("gesture classifier not found");
  Am_Object gesture_reader = Am_Gesture_Interactor.Create("gesture");
  gesture_reader.Set(Am_START_WHEN, "right_down")
      .Set(Am_FEEDBACK_OBJECT, gesture_feedback)
      .Set(Am_CLASSIFIER, gc)
      //.Set (Am_MIN_NONAMBIGUITY_PROB, 0.95)
      .Set(Am_MAX_DIST_TO_MEAN, 200)
      .Set(Am_ITEMS,
           Am_Value_List()
               .Add(Am_Gesture_Create_Command.Create()
                        .Set(Am_LABEL, ":LINE")
                        .Add(NEW_OBJECT_PROTOTYPE, my_line_proto)
                        .Set(Am_AS_LINE, true)
                        .Set(Am_CREATE_NEW_OBJECT_METHOD, gesture_creator)
                        .Set(Am_IMPLEMENTATION_PARENT,
                             Am_Command.Create("Parent of line")
                                 .Set(Am_DO_METHOD, my_do)))
               .Add(Am_Gesture_Create_Command.Create("circle_gesture")
                        .Set(Am_LABEL, ":CIRCLE")
                        .Add(NEW_OBJECT_PROTOTYPE, my_arc_proto)
                        .Set(Am_AS_LINE, false)
                        .Set(Am_CREATE_NEW_OBJECT_METHOD, gesture_creator))
               .Add(Am_Gesture_Create_Command.Create("rect_gesture")
                        .Set(Am_LABEL, ":RECTANGLE")
                        .Add(NEW_OBJECT_PROTOTYPE, my_rectangle_proto)
                        .Set(Am_AS_LINE, false)
                        .Set(Am_CREATE_NEW_OBJECT_METHOD, gesture_creator))
               .Add(Am_Gesture_Select_And_Do_Command.Create("gesture_cut")
                        .Set(Am_LABEL, ":CUT")
                        .Set(Am_IMPLEMENTATION_PARENT, cut_command)
                        .Set(Am_SELECTION_WIDGET, my_selection))
               .Add(Am_Undo_Command.Create().Set(Am_LABEL, ":DOT")));
  gesture_reader.Get_Object(Am_COMMAND)
      .Set(Am_DO_METHOD, gesture_unrecognized)
      .Set(Am_IMPLEMENTATION_PARENT, true) // inhibit undo
      ;

  created_objs.Add_Part(gesture_reader);

  Am_Screen.Add_Part(window);

  if (argc > 1) {
    Am_String s = argv[1];
    Am_Standard_Open_From_Filename(open_command, s);
  }

  std::cout << "Ready\n" << std::flush;
  Am_Main_Event_Loop();

  Am_Cleanup();

  return 0;
}
