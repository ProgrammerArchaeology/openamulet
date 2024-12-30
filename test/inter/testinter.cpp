/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <am_inc.h>

#include <amulet/am_io.h>

#include <amulet/standard_slots.h>
#include <amulet/inter_advanced.h>
#include <amulet/value_list.h>
#include <amulet/inter.h>
#include <amulet/opal.h>
#include <amulet/opal_advanced.h>
#include <amulet/formula.h>
#include <amulet/text_fns.h>
#include <amulet/debugger.h>
#include <amulet/registry.h>

using namespace std;

Am_Slot_Key PART1 = Am_Register_Slot_Name("PART1");
Am_Slot_Key PART2 = Am_Register_Slot_Name("PART2");
Am_Slot_Key PART3 = Am_Register_Slot_Name("PART3");
Am_Slot_Key PART4 = Am_Register_Slot_Name("PART4");
Am_Slot_Key EEK = Am_Register_Slot_Name("EEK");

Am_Object r1;
Am_Object rect_proto;
Am_Object line_proto;
Am_Object select_inter;
Am_Object move_inter;
Am_Object create_inter;
Am_Object window;
Am_Object window2 = nullptr;
Am_Object text_window;
Am_Object proto_rect = nullptr;
Am_Object rfeedback;
Am_Object lfeedback;
Am_Object single_undo_handler;
Am_Object multi_undo_handler;
Am_Object text0, text1, text2, textnum;
Am_Object text_inter0, text_inter1;
Am_Object feedback_window_for_move;
Am_Object sub_window;
Am_Object r2_inactive_commands, text2_inactive_commands;
Am_Object password_text;

Am_Object win3; // for multi-window
Am_Object pop_up_window;

Am_Style thick_line(0.0f, 0.0f, 0.0f, 7);
Am_Style even_line(0.0f, 0.0f, 0.0f, 6);
Am_Style thin_line(0.0f, 0.0f, 0.0f, 1);

Am_Font short_font(Am_FONT_SERIF);
Am_Font long_font(Am_FONT_SERIF, false, true, false, Am_FONT_VERY_LARGE);
Am_Font testinter_bold_font(Am_FONT_FIXED, true, false, true,
                            Am_FONT_VERY_LARGE);

Am_Define_Method(Am_Object_Method, void, my_do, (Am_Object cmd))
{
  Am_Object inter = cmd.Get_Owner();
  Am_Value value;
  value = cmd.Peek(Am_VALUE);
  cout << "\n+-+-+- Command " << cmd << " for inter " << inter
       << " value = " << value << " type ";
  Am_Print_Type(cout, value.type);
  cout << endl << flush;
}

Am_Define_Method(Am_Object_Method, void, return_10, (Am_Object self))
{
  Am_Object window;
  window = self.Get_Object(Am_SAVED_OLD_OWNER).Get(Am_WINDOW);
  cout << "Calling Finish_Pop_Up_Waiting on " << window << endl << flush;
  Am_Finish_Pop_Up_Waiting(window, 10);
}
Am_Define_Method(Am_Object_Method, void, return_null, (Am_Object self))
{
  Am_Object window;
  window = self.Get_Object(Am_SAVED_OLD_OWNER).Get(Am_WINDOW);
  cout << "Calling Finish_Pop_Up_Waiting on " << window << endl << flush;
  Am_Finish_Pop_Up_Waiting(window, (0L));
}

int window_count = 1;
Am_Define_Method(Am_Object_Method, void, popupnewone, (Am_Object /*self*/))
{
  Am_Object new_pop = pop_up_window.Create()
                          .Set(Am_LEFT, 100 + window_count * 10)
                          .Set(Am_TOP, 100 + window_count * 10)
                          .Set(Am_VISIBLE, false);
  if (window_count % 4 == 1)
    new_pop.Set(Am_FILL_STYLE, Am_Yellow);
  else if (window_count % 4 == 2)
    new_pop.Set(Am_FILL_STYLE, Am_Blue);
  else if (window_count % 4 == 3)
    new_pop.Set(Am_FILL_STYLE, Am_Green);
  window_count++;
  cout << "Popping up window " << new_pop << endl << flush;
  Am_Value v;
  Am_Pop_Up_Window_And_Wait(new_pop, v, false);
  cout << "Popup " << new_pop << " returned " << v << endl << flush;
}
Am_Define_Method(Am_Object_Method, void, popupmodal, (Am_Object /*self*/))
{
  Am_Object new_pop = pop_up_window.Create()
                          .Set(Am_LEFT, 100 + window_count * 10)
                          .Set(Am_TOP, 100 + window_count * 10)
                          .Set(Am_VISIBLE, false)
                          .Set(Am_FILL_STYLE, Am_Orange)
                          .Set(Am_TITLE, "MODAL Window");
  new_pop.Remove_Part(PART2);
  window_count++;
  cout << "Popping up MODAL window " << new_pop << endl << flush;
  Am_Value v;
  Am_Pop_Up_Window_And_Wait(new_pop, v, true);
  cout << "Modal Popup " << new_pop << " returned " << v << endl << flush;
}

Am_Define_Font_Formula(testinter_pick_font)
{
  if ((bool)self.Get(Am_INTERIM_SELECTED))
    return long_font;
  else if ((bool)self.Get(Am_SELECTED))
    return testinter_bold_font;
  else
    return short_font;
}

Am_Define_Style_Formula(rect_fill)
{
  if ((bool)self.Get(Am_SELECTED))
    return Am_White;
  else
    return self.Get(Am_VALUE);
}

Am_Define_Style_Formula(rect_line)
{
  if ((bool)self.Get(Am_INTERIM_SELECTED))
    return thick_line;
  else
    return thin_line;
}

Am_Define_Style_Formula(line_line)
{
  if ((bool)self.Get(Am_INTERIM_SELECTED))
    return thick_line;
  else if ((bool)self.Get(Am_SELECTED))
    return Am_White;
  else
    return self.Get(Am_VALUE);
}

Am_Define_Formula(int, mover)
{
  if ((bool)self.Get(Am_INTERIM_SELECTED))
    return 110;
  else
    return 100;
}

Am_Define_Formula(bool, as_line_if_shift)
{
  Am_Input_Char start_char = self.Get(Am_START_CHAR);
  if (start_char == Am_Input_Char("SHIFT_RIGHT_DOWN"))
    return true;
  else
    return false;
}

Am_Slot_Key USING_FEEDBACK = Am_Register_Slot_Name("USING_FEEDBACK");
Am_Slot_Key USING_WINDOW = Am_Register_Slot_Name("USING_WINDOW");

Am_Define_Object_Formula(compute_feedback_obj)
{
  Am_Object which_feedback;
  if ((bool)self.Get(USING_FEEDBACK)) {
    if ((bool)self.Get(USING_WINDOW)) {
      cout << "&-&&- Recompute formula; using WINDOW as feedback\n" << flush;
      return which_feedback = feedback_window_for_move;
    } else {
      if ((bool)self.Get(Am_AS_LINE)) {
        cout << "&-&&- Recompute formula; using LINE feedback\n" << flush;
        which_feedback = lfeedback;
      } else {
        cout << "&-&&- Recompute formula; using RECT feedback\n" << flush;
        which_feedback = rfeedback;
      }
    }
  } else {
    cout << "&-&&- Recompute formula; using NO feedback\n";
    which_feedback = Am_No_Object;
  }
  return which_feedback;
}

void
do_print_name(Am_Object cmd)
{
  Am_Object inter = cmd.Get_Owner();
  cout << "\n+-+-+- Inter " << inter << " run -+-+-+-+\n" << flush;
}

Am_Define_Method(Am_Object_Method, void, print_my_name, (Am_Object cmd))
{
  do_print_name(cmd);
}

Am_Define_Method(Am_Create_New_Object_Method, Am_Object, create_new_object,
                 (Am_Object /* inter */, Am_Inter_Location data,
                  Am_Object /* old_object */))
{
  Am_Object ref_obj;
  int a, b, c, d;
  bool create_line;
  data.Get_Location(create_line, ref_obj, a, b, c, d);

  //don't need translate coordinates because feedback object's owner
  //and real object's owner are both at the top level, so ref_obj will
  //be the right window

  static int new_object_cnt = 1;

  Am_Style color;
  const char *colorstr = nullptr;
  switch (new_object_cnt) {
  case 1:
    color = Am_Black;
    colorstr = "Black";
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
    color = Am_Yellow;
    colorstr = "Yellow";
    break;
  case 5:
    color = Am_Purple;
    colorstr = "Purple";
    break;
  case 6:
    color = Am_Cyan;
    colorstr = "Cyan";
    break;
  case 7:
    color = Am_Orange;
    colorstr = "Orange";
    break;
  case 8:
    color = Am_Red;
    colorstr = "Red";
    break;
  case 9:
    color = Am_White;
    colorstr = "White";
    break;
  case 10:
    color = Am_Motif_Orange;
    colorstr = "Motif_Orange";
    break;
  case 11:
    color = Am_Motif_Light_Orange;
    colorstr = "Motif_Light_Orange";
    break;
  case 12:
    color = Am_Motif_Gray;
    colorstr = "Motif_Gray";
    break;
  case 13:
    color = Am_Motif_Light_Gray;
    colorstr = "Motif_Light_Gray";
    break;
  case 14:
    color = Am_Motif_Blue;
    colorstr = "Motif_Blue";
    break;
  case 15:
    color = Am_Motif_Light_Blue;
    colorstr = "Motif_Light_Blue";
    break;
  case 16:
    color = Am_Motif_Green;
    colorstr = "Motif_Green";
    break;
  case 17:
    color = Am_Motif_Light_Green;
    colorstr = "Motif_Light_Green";
    break;
  }
  new_object_cnt = (new_object_cnt % 17) + 1;
  Am_Object new_obj;
  if (create_line)
    new_obj = line_proto.Create()
                  .Set(Am_VALUE, color)
                  .Set(Am_X1, a)
                  .Set(Am_Y1, b)
                  .Set(Am_X2, c)
                  .Set(Am_Y2, d);
  else
    new_obj = rect_proto.Create()
                  .Set(Am_VALUE, color)
                  .Set(Am_LEFT, a)
                  .Set(Am_TOP, b)
                  .Set(Am_WIDTH, c)
                  .Set(Am_HEIGHT, d);

  cout << "\n--++-- Created new object " << new_obj << " in window " << ref_obj
       << " at (" << a << "," << b << "," << c << "," << d
       << ") color=" << colorstr << " cnt =" << new_object_cnt << endl
       << flush;
  ref_obj.Add_Part(new_obj);
  return new_obj;
}

Am_Slot_Key INTER_SLOT = Am_Register_Slot_Name("INTER_SLOT");
Am_Slot_Key COPY_CNT = Am_Register_Slot_Name("COPY_CNT");
Am_Slot_Key PROTO_RECT_SLOT = Am_Register_Slot_Name("PROTO_RECT");

void
test_make_instance1()
{
  window2 =
      Am_Window.Create("win2")
          .Set(Am_FILL_STYLE, Am_Blue)
          .Set(Am_LEFT, 550)
#ifndef _MACINTOSH
          .Set(Am_TOP, 20)
#else
          .Set(Am_TOP, 50)
#endif
          .Set(Am_WIDTH, 200)
          .Set(Am_HEIGHT, 200)
          .Set(Am_DOUBLE_BUFFER, true)
          .Add_Part(
              PROTO_RECT_SLOT,
              proto_rect =
                  rect_proto.Create("proto_rect")
                      .Set(COPY_CNT, 0)
                      .Set(Am_LEFT, 5)
                      .Set(Am_TOP, 5)
                      .Set(Am_WIDTH, 20)
                      .Set(Am_HEIGHT, 20)
                      .Add_Part(INTER_SLOT,
                                Am_Choice_Interactor.Create("click_on_proto")
                                    .Set(Am_HOW_SET, Am_CHOICE_TOGGLE)));
  Am_Screen.Add_Part(window2);
  cout << "Made first window, now type 'I' to make the instance\n" << flush;
}

void
test_make_instance2()
{
  if (!window2.Valid())
    test_make_instance1();
  int cnt = 1 + (int)proto_rect.Get(COPY_CNT);
  proto_rect.Set(COPY_CNT, cnt);
  cout << "Creating rectangle copy of " << proto_rect << " " << proto_rect
       << endl;
  Am_Object copy_rect = proto_rect.Create().Set(Am_LEFT, cnt * 30);
  cout << "New rectangle = " << copy_rect << " " << copy_rect << endl;
  Am_Object copy_inter;
  copy_inter = copy_rect.Get(INTER_SLOT);
  cout << "It's interactor is " << copy_inter << " " << copy_inter;
  cout << "Adding it to the window\n" << flush;
  window2.Add_Part(copy_rect);
}

void
Undo_Redo_Selective(Am_Slot_Key allowed_slot, Am_Slot_Key method_slot,
                    const char *prompt_str)
{
  Am_Object undo_handler, last_command;
  undo_handler = window.Get(Am_UNDO_HANDLER);
  bool use_new = (method_slot == Am_SELECTIVE_REPEAT_ON_NEW_METHOD);
  if (undo_handler != multi_undo_handler)
    cout << "** Selective only allowed with multi\n";
  else {
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
      Am_Value_Type typ = select_inter.Get_Slot_Type(Am_VALUE);
      if (typ == Am_OBJECT)
        current_selection = select_inter.Peek(Am_VALUE);
      else {
        cout << "For Do Again on New, Selection not single object, type = "
             << typ << endl
             << flush;
        return;
      }
    } else
      allowed_method = undo_handler.Get(allowed_slot);

    bool allowed;
    int cnt;
    for (l.Start(), cnt = 0; !l.Last(); l.Next(), cnt++) {
      cmd = l.Get();
      s = cmd.Get(Am_LABEL);
      obj_modified = cmd.Peek(Am_OBJECT_MODIFIED);
      cout << cnt << " " << s << " on " << obj_modified << " (cmd = " << cmd
           << ") ";
      if (use_new)
        allowed = new_allowed_method.Call(cmd, current_selection, Am_No_Value);
      else
        allowed = allowed_method.Call(cmd);
      if (allowed)
        cout << "OK\n";
      else
        cout << "NOT OK\n";
    }
    cout << "--Type index of command to " << prompt_str
         << " (or -1 to exit): " << flush;
    int which;
    cin >> which;
    if (which < 0)
      return;
    for (l.Start(), cnt = 0; cnt < which; l.Next(), cnt++)
      ;
    cmd = l.Get();
    if (use_new) {
      Am_Handler_Selective_Repeat_New_Method method;
      method = undo_handler.Get(method_slot);
      cout << prompt_str << " on cmd " << cmd << " method = " << method
           << " new obj = " << current_selection << endl
           << flush;
      new_cmd = method.Call(undo_handler, cmd, current_selection, Am_No_Value);
      cout << "  new_cmd = " << new_cmd << endl << flush;
    } else {
      Am_Handler_Selective_Undo_Method method;
      method = undo_handler.Get(method_slot);
      cout << prompt_str << " on cmd " << cmd << " method = " << method << endl
           << flush;
      new_cmd = method.Call(undo_handler, cmd);
      cout << "  new_cmd = " << new_cmd << endl << flush;
    }
  }
}

void
Do_Pop_Up_Window(bool modal)
{
  Am_Value ret;
  cout << "Popping up window " << pop_up_window << " modal " << modal << endl
       << flush;
  Am_Object o = pop_up_window.Create();
  if (modal)
    o.Remove_Part(PART2);
  Am_Pop_Up_Window_And_Wait(o, ret, modal);
  cout << "Window " << o << " returned value = " << ret << endl << flush;
}

Am_Text_Abort_Or_Stop_Code cur_code = Am_TEXT_ABORT_AND_RESTORE;

Am_Define_Method(Am_Text_Check_Legal_Method, Am_Text_Abort_Or_Stop_Code,
                 check_num_method, (Am_Object & text, Am_Object & /*inter*/))
{
  Am_String str = text.Get(Am_TEXT);
  char *s = str;
  int len = strlen(s);
  char *ptr;
  long i = strtol(s, &ptr, 10);
  cout << "** Checking string = `" << s << "' len = " << len
       << " int value = " << i << endl
       << flush;
  if (ptr < s + len) {
    Am_Beep(window);
    cout << "** ptr " << (void *)ptr << " less than s+len " << (void *)(s + len)
         << " returning " << (int)cur_code << flush;
    return cur_code;
  } else
    return Am_TEXT_OK;
}

Am_Define_Method(Am_Object_Method, void, change_setting, (Am_Object cmd))
{
  Am_Object inter = cmd.Get_Owner();
  Am_Input_Char c = inter.Get(Am_START_CHAR);
  cout << "---- got " << c << endl;
  switch (c.As_Char()) {
  case 'q':
    Am_Exit_Main_Event_Loop();
    break;
  case 't':
    select_inter.Set(Am_HOW_SET, Am_CHOICE_TOGGLE);
    cout << " setting TOGGLE = " << Am_CHOICE_TOGGLE << endl << flush;
    break;
  case 's':
    select_inter.Set(Am_HOW_SET, Am_CHOICE_SET);
    cout << " setting SET = " << Am_CHOICE_SET << endl << flush;
    break;
  case 'l':
    select_inter.Set(Am_HOW_SET, Am_CHOICE_LIST_TOGGLE);
    cout << " setting LIST_TOGGLE = " << Am_CHOICE_LIST_TOGGLE << endl << flush;
    break;
  case 'c':
    select_inter.Set(Am_HOW_SET, Am_CHOICE_CLEAR);
    cout << " setting CLEAR = " << Am_CHOICE_CLEAR << endl << flush;
    break;
  case 'f':
    select_inter.Set(Am_FIRST_ONE_ONLY, true);
    cout << " setting first one only to TRUE\n" << flush;
    break;
  case 'F':
    select_inter.Set(Am_FIRST_ONE_ONLY, false);
    cout << " setting first one only to FALSE\n" << flush;
    break;
  case 'e': {
    static bool set_selected = false;
    select_inter.Set(Am_SET_SELECTED, set_selected);
    cout << " setting Am_SET_SELECTED to " << set_selected << endl << flush;
    set_selected = !set_selected;
    break;
  }
  case 'd':
    cout << " debugging all\n" << flush;
    Am_Set_Inter_Trace(Am_INTER_TRACE_ALL);
    break;
  case 'h':
    cout << " debugging SHORT\n" << flush;
    Am_Set_Inter_Trace(Am_INTER_TRACE_SHORT);
    break;
  case 'n':
    cout << " debugging none\n" << flush;
    Am_Set_Inter_Trace(Am_INTER_TRACE_NONE);
    break;
  case 'S':
    cout << " debugging setting only\n" << flush;
    Am_Set_Inter_Trace(Am_INTER_TRACE_NONE);
    Am_Set_Inter_Trace(Am_INTER_TRACE_SETTING);
    break;

  case 'g': {
    cout << " Type new grid amount for X: ";
    int grid;
    cin >> grid;
    move_inter.Set(Am_GRID_X, grid);
    create_inter.Set(Am_GRID_X, grid);
    cout << " Type new grid amount for Y: ";
    cin >> grid;
    move_inter.Set(Am_GRID_Y, grid);
    create_inter.Set(Am_GRID_Y, grid);
    cout << " Type new grid X origin : ";
    cin >> grid;
    move_inter.Set(Am_GRID_ORIGIN_X, grid);
    create_inter.Set(Am_GRID_ORIGIN_X, grid);
    cout << " Type new grid Y origin : ";
    cin >> grid;
    move_inter.Set(Am_GRID_ORIGIN_Y, grid);
    create_inter.Set(Am_GRID_ORIGIN_Y, grid);
    break;
  }
  case 'M': {
    cout << " Type new minimum width: ";
    int i;
    cin >> i;
    move_inter.Set(Am_MINIMUM_WIDTH, i);
    create_inter.Set(Am_MINIMUM_WIDTH, i);
    cout << " Type new minimum height: ";
    cin >> i;
    move_inter.Set(Am_MINIMUM_HEIGHT, i);
    create_inter.Set(Am_MINIMUM_HEIGHT, i);
    cout << " Type new minimum length: ";
    cin >> i;
    move_inter.Set(Am_MINIMUM_LENGTH, i);
    create_inter.Set(Am_MINIMUM_LENGTH, i);
    break;
  }
  case 'm': {
    bool growing = move_inter.Get(Am_GROWING);
    move_inter.Set(Am_GROWING, !growing);
    cout << " Changing growing to be " << !growing << endl << flush;
    break;
  }
  case 'W': {
    bool using_window = move_inter.Get(USING_WINDOW);
    move_inter.Set(USING_WINDOW, !using_window);
    create_inter.Set(USING_WINDOW, !using_window);
    cout << " Changing using window to be " << !using_window << endl << flush;
    break;
  }
  case 'o': {
    bool cur_feedback = move_inter.Get(USING_FEEDBACK);
    move_inter.Set(USING_FEEDBACK, !cur_feedback);
    cout << "Changing so feedback = " << !cur_feedback << endl << flush;
    break;
  }
  case 'w': {
    cout << " Where attach options are:\n"
            "Am_ATTACH_WHERE_HIT Am_ATTACH_NW Am_ATTACH_N Am_ATTACH_NE\n"
            "Am_ATTACH_E Am_ATTACH_SE Am_ATTACH_S Am_ATTACH_SW Am_ATTACH_W\n"
            "Am_ATTACH_END_1 Am_ATTACH_END_2 Am_ATTACH_CENTER\n"
            " Type new attach point as a string: ";
    char attach_str[100];
    cin >> attach_str;
    Am_Type_Support *ws = Am_Find_Support(Am_Move_Grow_Where_Attach::Type_ID());
    if (ws) {
      Am_Value new_val = ws->From_String(attach_str);
      cout << "For string `" << attach_str << "' val return is " << new_val
           << endl
           << flush;
      if (new_val.Exists())
        move_inter.Set(Am_WHERE_ATTACH, new_val);
      else
        cout << "** Bad value, not set\n" << flush;
    } else
      cout << "** Enum Support for Am_Move_Grow_Where_Attach gone\n";
    break;
  }

  case 'T': {
    Am_Object command_obj;
    command_obj = select_inter.Get(Am_COMMAND);
    Am_Value_Type typ = command_obj.Get_Slot_Type(Am_VALUE);
    Am_Object current_selection;
    if (typ == Am_OBJECT) {
      current_selection = command_obj.Get(Am_VALUE);
      cout << "Moving " << current_selection << " to the top\n" << flush;
      Am_To_Top(current_selection);
    } else
      cout << "Selection is not a single object, type = " << typ << endl
           << flush;
    break;
  }
  case 'B': {
    Am_Object command_obj;
    command_obj = select_inter.Get(Am_COMMAND);
    Am_Value_Type typ = command_obj.Get_Slot_Type(Am_VALUE);
    Am_Object current_selection;
    if (typ == Am_OBJECT) {
      current_selection = command_obj.Get(Am_VALUE);
      cout << "Moving " << current_selection << " to the bottom\n" << flush;
      Am_To_Bottom(current_selection);
    } else
      cout << "Selection is not a single object, type = " << typ << endl
           << flush;
    break;
  }
  case 'D': {
    Am_Value_Type typ = select_inter.Get_Slot_Type(Am_VALUE);
    Am_Object current_selection;
    if (typ == Am_OBJECT) {
      current_selection = select_inter.Get(Am_VALUE);
      cout << "Deleting " << current_selection << " (not undoable)\n" << flush;
      current_selection.Destroy();
    } else
      cout << "Selection is not a single object, type = " << typ << endl
           << flush;
    break;
  }
  case 'U': {
    Am_Object cur_handler;
    cur_handler = window.Get(Am_UNDO_HANDLER);
    if (cur_handler == multi_undo_handler) {
      cout << "Now will have single undo\n" << flush;
      window.Set(Am_UNDO_HANDLER, single_undo_handler);
      win3.Set(Am_UNDO_HANDLER, single_undo_handler);
    } else {
      cout << "Now will have multiple undo\n" << flush;
      window.Set(Am_UNDO_HANDLER, multi_undo_handler);
      win3.Set(Am_UNDO_HANDLER, multi_undo_handler);
    }
    break;
  }
  case 'a': {
    //this should remove the default constraint which is there
    int width = text2.Get(Am_WIDTH);
    cout << "setting width of " << text2 << " to be " << width << endl << flush;
    text2.Set(Am_WIDTH, width);
    break;
  }
  case 'K': {
    switch (cur_code) {
    case Am_TEXT_ABORT_AND_RESTORE:
      cout << "Code for when text edit not an int now= Am_TEXT_KEEP_RUNNING\n"
           << flush;
      cur_code = Am_TEXT_KEEP_RUNNING;
      break;
    case Am_TEXT_KEEP_RUNNING:
      cout << "Code for when text edit not an int now= Am_TEXT_STOP_ANYWAY\n"
           << flush;
      cur_code = Am_TEXT_STOP_ANYWAY;
      break;
    default:
      cout << "Code for when text edit not an int now= "
              "Am_TEXT_ABORT_AND_RESTORE\n"
           << flush;
      cur_code = Am_TEXT_ABORT_AND_RESTORE;
      break;
    }
    break;
  }

  case 'u': {
    Am_Object undo_handler, last_command;
    cout << " checking undo...";
    undo_handler = window.Get(Am_UNDO_HANDLER);
    cout << " undo handler = " << undo_handler << endl << flush;
    last_command = undo_handler.Get(Am_UNDO_ALLOWED);
    if (last_command.Valid()) {
      Am_String s;
      s = last_command.Get(Am_LABEL);
      cout << " undoing cmd " << last_command << " = " << (const char *)s
           << endl
           << flush;
      Am_Object_Method undoit;
      undoit = undo_handler.Get(Am_PERFORM_UNDO);
      undoit.Call(undo_handler);
    } else
      cout << " nothing to undo\n" << flush;
    break;
  }
  case 'r': {
    Am_Object undo_handler, last_command;
    cout << " checking redo ...";
    undo_handler = window.Get(Am_UNDO_HANDLER);
    cout << " undo handler = " << undo_handler << endl << flush;
    last_command = undo_handler.Get(Am_REDO_ALLOWED);
    if (last_command.Valid()) {
      Am_String s;
      s = last_command.Get(Am_LABEL);
      cout << " re-doing cmd " << last_command << " = " << (const char *)s
           << endl
           << flush;
      Am_Object_Method redoit;
      redoit = undo_handler.Get(Am_PERFORM_REDO);
      redoit.Call(undo_handler);
    } else
      cout << " nothing to redo\n" << flush;
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
  case 'N': {
    Undo_Redo_Selective(Am_SELECTIVE_REPEAT_NEW_ALLOWED,
                        Am_SELECTIVE_REPEAT_ON_NEW_METHOD,
                        "Selective Repeat on New");
    break;
  }
  case 'A': {
    cout << "Explicitly aborting all inters\n" << flush;
    Am_Abort_Interactor(select_inter);
    Am_Abort_Interactor(move_inter);
    Am_Abort_Interactor(create_inter);
    Am_Abort_Interactor(text_inter0);
    Am_Abort_Interactor(text_inter1);
    break;
  }
  case 'G': {
    cout << "Making moving inter go (explicit start) over " << r1 << endl
         << flush;
    Am_Start_Interactor(move_inter, r1);
    break;
  }
  case 'E': {
    Am_Object running_inter;
    if (Am_Get_Inter_State(select_inter) > Am_INTER_WAITING)
      running_inter = select_inter;
    else if (Am_Get_Inter_State(move_inter) > Am_INTER_WAITING)
      running_inter = move_inter;
    else if (Am_Get_Inter_State(create_inter) > Am_INTER_WAITING)
      running_inter = create_inter;
    else if (Am_Get_Inter_State(text_inter0) > Am_INTER_WAITING)
      running_inter = text_inter0;
    else if (Am_Get_Inter_State(text_inter1) > Am_INTER_WAITING)
      running_inter = text_inter1;
    if (running_inter.Valid()) {
      cout << "Explicitly stopping inter " << running_inter << endl << flush;
      Am_Stop_Interactor(running_inter);
    } else
      cout << "Couldn't find any inters running to stop\n";
    break;
  }

  case 'i':
    test_make_instance1();
    break;
  case 'I':
    test_make_instance2();
    break;

  case 'p':
    Do_Pop_Up_Window(false);
    break;
  case 'P':
    Do_Pop_Up_Window(true);
    break;

  //case 'b': { //temporary -- this crashes on purpose
  // r1.Add_Part(Am_Arc.Create("new circle"));
  // break;
  // }
  case 'z': {
    static int inactive_which = 1;
    switch (inactive_which) {
    case 1: {
      r2_inactive_commands.Set(Am_MOVE_INACTIVE, true);
      cout << "Disabling MOVE of r2\n" << flush;
      break;
    }
    case 2: {
      r2_inactive_commands.Set(Am_GROW_INACTIVE, true);
      cout << "Disabling GROW of r2\n" << flush;
      break;
    }
    case 3: {
      r2_inactive_commands.Set(Am_SELECT_INACTIVE, true);
      cout << "Disabling SELECT of r2\n" << flush;
      break;
    }
    case 4: {
      text2_inactive_commands.Set(Am_TEXT_EDIT_INACTIVE, true);
      cout << "Disabling Am_TEXT_EDIT_INACTIVE of text2 \n" << flush;
      break;
    }
    }
    inactive_which = (inactive_which % 4) + 1;
    break;
  }
  case 'Z': {
    static int active_which = 1;
    switch (active_which) {
    case 1: {
      r2_inactive_commands.Set(Am_MOVE_INACTIVE, false);
      cout << "ENabling MOVE of r2\n" << flush;
      break;
    }
    case 2: {
      r2_inactive_commands.Set(Am_GROW_INACTIVE, false);
      cout << "ENabling GROW of r2\n" << flush;
      break;
    }
    case 3: {
      r2_inactive_commands.Set(Am_SELECT_INACTIVE, false);
      cout << "ENabling SELECT of r2\n" << flush;
      break;
    }
    case 4: {
      text2_inactive_commands.Set(Am_TEXT_EDIT_INACTIVE, false);
      cout << "ENabling Am_TEXT_EDIT_INACTIVE of text2\n" << flush;
      break;
    }
    }
    active_which = (active_which % 4) + 1;
    break;
  }
  case 'k': {
#ifdef DEBUG
    static bool newkeys = false;
    newkeys = !newkeys;
    if (newkeys) {
      Am_Set_Inspector_Keys("1", "2", "3");
      cout << "Set inspector keys to 1, 2, and 3\n" << flush;
    } else {
      Am_Set_Inspector_Keys("F1", "F2", "F3");
      cout << "Set inspector keys to F1, F2, and F3\n" << flush;
    }
#endif
    break;
  }

  case 'X': {
    move_inter.Set(Am_MULTI_OWNERS, false);
    cout << "Made inter " << move_inter << " not multi-owner\n" << flush;
    break;
  }
  case 'C': {
    cout << " Type new how many points (1 or 2): ";
    int i;
    cin >> i;
    create_inter.Set(Am_HOW_MANY_POINTS, i);
    cout << " Flip if change sides? (0 =no, 1=yes): ";
    cin >> i;
    create_inter.Set(Am_FLIP_IF_CHANGE_SIDES, i);
    cout << " Abort if too small? (0 =no, 1=yes): ";
    cin >> i;
    create_inter.Set(Am_ABORT_IF_TOO_SMALL, i);
    break;
  }
  case 'H': {
    static char password_char = '*';
    switch (password_char) {
    case '*':
      password_char = '.';
      break;
    case '.':
      password_char = 'W';
      break;
    case 'W':
      password_char = '@';
      break;
    case '@':
      password_char = '`';
      break;
    case '`':
      password_char = '*';
      break;
    }
    password_text.Set(Am_START_CHAR, password_char);
    break;
  }
  default:
    cout << "** Illegal, want:\n"
         << " q = quit.\n"
         << " For choice: t=toggle, s=set, l=list-toggle, c=clear\n"
         << "    f=first_one_only, F= not first_one_only, e=toggle set sel\n"
         << " Debugging:\n"
         << "    d=full-debug, h=short-debug, n=no-debug, S=setting only\n"
         << "    k=change inspector keys\n"
         << " Undoing:\n"
         << "    u=undo, r=redo, U=switch single or multi undo\n"
         << "    x=selective undo, R=selective repeat same, N=repeat new\n"
         << " Move/Grow\n"
         << "    g=grid, w=where_attach, m=toggle move vs. grow\n"
         << "    o=toggle use feedback on move/grow, M=min size\n"
         << "    W=use window as feedback\n"
         << "    X= make move/grow not multi-owner\n"
         << " Text\n"
         << "    a=set size of string object in red window\n"
         << "    K=change abort mode for editing integer\n"
         << " Create\n"
         << "    g=grid, M=min size, C=set other parameters for create\n"
         << " Explicit\n"
         << "    G = start (go) interactor\n"
         << "    A = abort interactor\n"
         << "    E = stop (end) interactor\n"
         << " Other:\n"
         << "    p = pop up window\n"
         << "    P = pop up modal window\n"
         << "    D=delete selected obj\n"
         << "    T=selected obj to top, B= to bottom\n"
         << "    i then I = test make instance\n"
         << "    z = toggle inactive object\n"
         << "    Z = toggle Active object\n"
         << "    H = change password character to '.'\n"
        // << "    b = crashes by adding a part illegally.\n"
        ;
    cout << "Left_down on object to select it\n"
         << "Any keyboard key to change settings, ? for help\n"
         << "shift_middle_down on r1 or r2, or HELP key to test priorities\n"
         << "middle_down to move or grow a top-level object\n"
         << "right_down to create a new rect\n"
         << "shift_right_down to create a new line\n\n"
         << "shift_left_down to edit text in red window\n"
         << "left_down in white window to edit text\n"
         << endl
         << flush;
    break;
  } // end switch
}
//free: jvy JLOQVY
//used: abcdefghiklmnopqrstuwxz ABCDEFGHIKMNPURSTWXZ

void
Insert_Stuff(Am_Object text)
{
  Am_Insert_String_At_Cursor(text, " How Now Brown Cow. ", true);
}

Am_Define_Method(Am_Where_Method, Am_Object, obj_in_any_win,
                 (Am_Object /* inter */, Am_Object /* object */,
                  Am_Object event_window, int x, int y))
{
  return Am_Point_In_Part(event_window, x, y, event_window);
}

//it has to be in the window, so no need to test; just return the window.
Am_Define_Method(Am_Where_Method, Am_Object, in_any_win,
                 (Am_Object /* inter */, Am_Object /* object */,
                  Am_Object event_window, int /* x */, int /* y */))
{
  return event_window;
}

int
main()
{
  cout << "General initialize\n";
  Am_Initialize();

  Am_Register_Name(thick_line, "thick line");
  Am_Register_Name(thin_line, "thin line");
  Am_Register_Name(even_line, "even line");

  cout << "Turning on debugging\n";
  Am_Set_Inter_Trace(Am_INTER_TRACE_ALL);

  cout << "Creating objects\n";

  single_undo_handler = Am_Single_Undo_Object.Create("My_Single_Undo");
  multi_undo_handler = Am_Multiple_Undo_Object.Create("My_Multi_Undo");

  window =
      Am_Window.Create("window")
          .Set(Am_TITLE, "Test Interactors")
          .Set(Am_FILL_STYLE, Am_Red)
          .Set(Am_LEFT, 20)
#ifndef _MACINTOSH
          .Set(Am_TOP, 20)
#else
          .Set(Am_TOP, 50)
#endif
          .Set(Am_WIDTH, 500)
          .Set(Am_HEIGHT, 310)
          .Add(EEK, Am_Value_List()
                        .Add("1s")
                        .Add('2')
                        .Add(" ")
                        .Add("4s")
                        .Add(' ')
                        .Add('6'))
          .Set(Am_UNDO_HANDLER, multi_undo_handler)
          .Set(Am_DOUBLE_BUFFER, true)
          .Set(Am_DESTROY_WINDOW_METHOD, Am_Window_Destroy_And_Exit_Method);

  window.Add_Part(
      Am_Choice_Interactor.Create("Events_As_List")
          .Set(Am_START_WHERE_TEST, Am_Inter_In)
          .Set(Am_START_WHEN, Am_Value_List().Add("R11").Add("^Y"))
          .Set(Am_ABORT_WHEN, Am_Value_List().Add("^C").Add("^g").Add(
                                  "CONTROL_META_RIGHT_DOWN"))
          .Set(
              Am_STOP_WHEN,
              Am_Value_List().Add("A").Add("B").Add("Y").Add("z").Add("RETURN"))
          .Set(Am_PRIORITY, 2)
          .Set_Part(Am_COMMAND, Am_Command.Create("list choices")
                                    .Set(Am_DO_METHOD, print_my_name)));
  Am_Object i1, i2, i3, i4;
  pop_up_window =
      Am_Window.Create("pop_up_window")
          .Set(Am_LEFT, 100)
          .Set(Am_TOP, 100)
          .Set(Am_VISIBLE, false)
          .Set(Am_WIDTH, 175)
          .Set(Am_HEIGHT, 80)
          .Set(Am_FILL_STYLE, Am_Amulet_Purple)
          .Set(Am_TITLE, "Pop Up")
          .Add_Part(PART1,
                    Am_Text.Create()
                        .Set(Am_LEFT, 10)
                        .Set(Am_TOP, 10)
                        .Set(Am_TEXT, "OK: Return 10")
                        .Add_Part(EEK, i1 = Am_One_Shot_Interactor.Create()))
          .Add_Part(PART2,
                    Am_Text.Create()
                        .Set(Am_LEFT, 10)
                        .Set(Am_TOP, 25)
                        .Set(Am_TEXT, "PopUp New One")
                        .Add_Part(EEK, i2 = Am_One_Shot_Interactor.Create()))
          .Add_Part(PART3,
                    Am_Text.Create()
                        .Set(Am_LEFT, 10)
                        .Set(Am_TOP, 40)
                        .Set(Am_TEXT, "PopUp New Modal One")
                        .Add_Part(EEK, i3 = Am_One_Shot_Interactor.Create()))
          .Add_Part(PART4,
                    Am_Text.Create()
                        .Set(Am_LEFT, 10)
                        .Set(Am_TOP, 55)
                        .Set(Am_TEXT, "Cancel: Return (0L)")
                        .Add_Part(EEK, i4 = Am_One_Shot_Interactor.Create()));
  i1.Get_Object(Am_COMMAND).Set(Am_DO_METHOD, return_10);
  i2.Get_Object(Am_COMMAND).Set(Am_DO_METHOD, popupnewone);
  i3.Get_Object(Am_COMMAND).Set(Am_DO_METHOD, popupmodal);
  i4.Get_Object(Am_COMMAND).Set(Am_DO_METHOD, return_null);

  r1 = Am_Rectangle.Create("r1")
           .Add(Am_SELECTED, false)
           .Add(Am_INTERIM_SELECTED, false)
           .Set(Am_FILL_STYLE, rect_fill)
           .Set(Am_LINE_STYLE, thick_line) //rect_line)
           .Add(Am_VALUE, Am_Green)
           .Set(Am_LEFT, 10)
           .Set(Am_TOP, 10)
           .Set(Am_WIDTH, 50)
           .Set(Am_HEIGHT, 50);

  rect_proto = Am_Roundtangle.Create("rect_proto")
                   .Add(Am_SELECTED, false)
                   .Add(Am_INTERIM_SELECTED, false)
                   .Set(Am_FILL_STYLE, rect_fill)
                   .Set(Am_LINE_STYLE, rect_line)
                   .Add(Am_VALUE, Am_Green);
  line_proto = Am_Line.Create("line_proto")
                   .Add(Am_SELECTED, false)
                   .Add(Am_INTERIM_SELECTED, false)
                   .Set(Am_LINE_STYLE, line_line)
                   .Add(Am_VALUE, Am_Green);

  Am_Object r2 = rect_proto.Create("r2")
                     .Set(Am_VALUE, Am_Blue)
                     .Set(Am_LEFT, 30)
                     .Set(Am_TOP, 30)
                     .Set(Am_WIDTH, 50)
                     .Set(Am_HEIGHT, 50);
  r2.Add_Part(Am_INACTIVE_COMMANDS,
              r2_inactive_commands = Am_Command.Create("r2_inactive_commands"));
  Am_Object r3 = rect_proto.Create("r3")
                     .Set(Am_VALUE, Am_Yellow)
                     .Set(Am_LEFT, 100)
                     .Set(Am_TOP, 10)
                     .Set(Am_WIDTH, 50)
                     .Set(Am_HEIGHT, 50);

  Am_Object r4, r5;

  Am_Object group1 =
      Am_Group.Create("group1")
          .Add(Am_SELECTED, false)
          .Add(Am_INTERIM_SELECTED, false)
          .Set(Am_LEFT, mover)
          .Set(Am_TOP, mover)
          .Set(Am_WIDTH, 100)
          .Set(Am_HEIGHT, 100)
          .Add_Part(r4 = rect_proto.Create("g1_r4")
                             .Set(Am_LEFT, 0)
                             .Set(Am_TOP, 0)
                             .Set(Am_WIDTH, 30)
                             .Set(Am_HEIGHT, 30)
                             .Set(Am_VALUE, Am_Purple))
          .Add_Part(r5 = rect_proto.Create("g1_r5")
                             .Set(Am_LEFT, 40)
                             .Set(Am_TOP, 10)
                             .Set(Am_WIDTH, 30)
                             .Set(Am_HEIGHT, 30)
                             .Set(Am_VALUE, Am_Cyan))
          .Add_Part(Am_Choice_Interactor.Create("choose_in_group")
                        .Set(Am_START_WHEN, "CONTROL_LEFT_DOWN"))

      ; // end group1

  rfeedback = Am_Rectangle.Create("rfeedback")
                  .Set(Am_FILL_STYLE, 0)
                  .Set(Am_LINE_STYLE, Am_Dotted_Line)
                  .Set(Am_VISIBLE, 0)
                  .Set(Am_LEFT, 100)
                  .Set(Am_TOP, 10)
                  .Set(Am_WIDTH, 50)
                  .Set(Am_HEIGHT, 50);
  lfeedback = Am_Line.Create("lfeedback")
                  .Set(Am_LINE_STYLE, // thick dotted line
                       Am_Style(0.0f, 0.0f, 0.0f, 5, Am_CAP_BUTT, Am_JOIN_MITER,
                                Am_LINE_ON_OFF_DASH))
                  .Set(Am_VISIBLE, 0);

  feedback_window_for_move =
      Am_Window.Create("Feedback_window")
          .Set(Am_TITLE, "feedback window")
          .Set(Am_FILL_STYLE, Am_Motif_Light_Blue)
          .Set(Am_WIDTH, Am_Width_Of_Parts)
          .Set(Am_HEIGHT, Am_Height_Of_Parts)
          .Set(Am_OMIT_TITLE_BAR, true)
          .Set(Am_VISIBLE, false)
          .Add_Part(Am_Text.Create().Set(Am_TEXT, "Feedback Window"));
  Am_Screen.Add_Part(feedback_window_for_move);

  win3 =
      Am_Window.Create("win3")
          .Set(Am_TITLE, "Extra Window")
          .Set(Am_FILL_STYLE, Am_Motif_Light_Green)
          .Set(Am_LEFT, 20)
          .Set(Am_TOP, 350)
          .Set(Am_WIDTH, 500)
          .Set(Am_HEIGHT, 100)
          .Set(Am_UNDO_HANDLER, multi_undo_handler)
          .Add_Part(rect_proto.Create()
                        .Set(Am_VALUE, Am_Yellow)
                        .Set(Am_LEFT, 10)
                        .Set(Am_TOP, 10)
                        .Set(Am_WIDTH, 50)
                        .Set(Am_HEIGHT, 50))
          .Add_Part(sub_window = Am_Window.Create("Sub_Window")
                                     .Set(Am_TITLE, "Sub Window")
                                     .Set(Am_LEFT, 300)
                                     .Set(Am_TOP, 20)
                                     .Set(Am_WIDTH, 175)
                                     .Set(Am_HEIGHT, 75)
                                     .Set(Am_FILL_STYLE, Am_White)
                                     .Set(Am_UNDO_HANDLER, multi_undo_handler));

  text_inter0 = Am_Text_Edit_Interactor.Create("text_editor");

  text_window = Am_Window.Create("text_window")
                    .Set(Am_LEFT, 540)
                    .Set(Am_TOP, 20)
                    .Set(Am_WIDTH, 200)
                    .Set(Am_HEIGHT, 120)
                    .Set(Am_TITLE, "Test Text Interactor")
                    .Add_Part(text_inter0)
                    .Add_Part(Am_Choice_Interactor.Create("choose_text")
                                  .Set(Am_START_WHEN, "RIGHT_DOWN"));

  text0 = Am_Text.Create("text0")
              .Add(Am_INTERIM_SELECTED, 0)
              .Add(Am_SELECTED, 0)
              .Set(Am_FONT, testinter_pick_font)
              .Set(Am_LEFT, 10)
              .Set(Am_TOP, 20)
              .Set(Am_TEXT, "Edit me");

  text1 = Am_Text.Create("text1")
              .Add(Am_INTERIM_SELECTED, 0)
              .Add(Am_SELECTED, 0)
              .Set(Am_FONT, testinter_pick_font)
              .Set(Am_LEFT, 10)
              .Set(Am_TOP, 40)
              .Set(Am_TEXT, "Edit me, too");
  textnum =
      Am_Text.Create("textnum")
          .Add(Am_INTERIM_SELECTED, 0)
          .Add(Am_SELECTED, 0)
          .Set(Am_FONT, testinter_pick_font)
          .Set(Am_LEFT, 10)
          .Set(Am_TOP, 70)
          .Set(Am_TEXT, "3456")
          .Add_Part(Am_Text_Edit_Interactor.Create("num_text_editor")
                        .Set(Am_TEXT_CHECK_LEGAL_METHOD, check_num_method));

  password_text = Am_Hidden_Text.Create()
                      .Add(Am_INTERIM_SELECTED, 0)
                      .Add(Am_SELECTED, 0)
                      .Set(Am_FONT, testinter_pick_font)
                      .Set(Am_LEFT, 10)
                      .Set(Am_TOP, 90)
                      .Set(Am_TEXT, "password");

  Am_Object r6 = Am_Rectangle.Create("r6")
                     .Set(Am_LEFT, 110)
                     .Set(Am_TOP, 80)
                     .Set(Am_WIDTH, 20)
                     .Set(Am_HEIGHT, 20)
                     .Set(Am_FILL_STYLE, Am_Red);
  text_window.Add_Part(r6);

  //click & drag tests
  Am_Object click_only_inter = Am_One_Shot_Interactor.Create("click_only")
                                   .Set(Am_START_WHEN, "MIDDLE_CLICK");
  click_only_inter.Get_Object(Am_COMMAND).Set(Am_DO_METHOD, my_do);
  r6.Add_Part(click_only_inter);

  //click & drag tests
  Am_Object drag_only_inter = Am_Move_Grow_Interactor.Create("drag_only")
                                  .Set(Am_START_WHEN, "MIDDLE_DRAG");
  drag_only_inter.Get_Object(Am_COMMAND).Set(Am_DO_METHOD, my_do);
  r6.Add_Part(drag_only_inter);

  text_window.Add_Part(text0);
  text_window.Add_Part(text1);
  text_window.Add_Part(textnum);
  text_window.Add_Part(password_text);

  text2 = Am_Text.Create("text2")
              .Set(Am_LEFT, 50)
              .Set(Am_TOP, 50)
              .Set(Am_TEXT, "This space for rent.");
  text2.Add_Part(Am_INACTIVE_COMMANDS,
                 text2_inactive_commands =
                     Am_Command.Create("text2_inactive_commands"));

  text_inter1 = text_inter0.Create("text in big window")
                    .Set(Am_START_WHEN, "SHIFT_LEFT_DOWN")
                    .Set(Am_RUNNING_WHERE_TEST, Am_Inter_In_Object_Or_Part);

  Am_Screen.Add_Part(window).Add_Part(win3).Add_Part(text_window);
  window.Add_Part(r1)
      .Add_Part(r2)
      .Add_Part(r3)
      .Add_Part(group1)
      .Add_Part(rfeedback)
      .Add_Part(lfeedback)
      .Add_Part(text2);

  cout << "Left_down on object to select it\n"
       << "Any keyboard key to change settings, ? for help\n"
       << "shift_middle_down on r1 or r2, or HELP key to test priorities\n"
       << "middle_down to move or grow a top-level object\n"
       << "right_down to create a new rect\n"
       << "shift_right_down to create a new line\n\n"
       << "shift_left_down to edit text in red window\n"
       << "left_down in white window to edit text\n"
       << endl
       << flush;

  Am_Edit_Translation_Table table;

  table = text_inter1.Get(Am_EDIT_TRANSLATION_TABLE);
  table.Add(Am_Input_Char('a'), &Insert_Stuff);
  text_inter1.Set(Am_EDIT_TRANSLATION_TABLE, table);
  text2.Add_Part(text_inter1);

  select_inter =
      Am_Choice_Interactor.Create("choose_rect")
          .Set(Am_START_WHERE_TEST, obj_in_any_win)
          .Set(Am_MULTI_OWNERS,
               Am_Value_List().Add(window).Add(win3).Add(sub_window));
  window.Add_Part(select_inter);

  Am_Object how_set_inter = Am_One_Shot_Interactor.Create("change_settings")
                                .Set(Am_START_WHEN, "ANY_KEYBOARD")
      //  .Set(Am_PRIORITY, 200) //higher than running, so can abort inters
      ;
  Am_Object cmd;
  cmd = how_set_inter.Get(Am_COMMAND);
  cmd.Set(Am_DO_METHOD, change_setting);
  cmd.Set(Am_IMPLEMENTATION_PARENT, true); //not undo-able
  cmd.Set_Name("change_settings_command");

  window.Add_Part(how_set_inter);

  // add two specific interactors to test priorities
  Am_Object only_r1 = Am_Choice_Interactor
                          .Create("only_r1")
                          //#if defined(_WINDOWS)
                          //   .Set(Am_START_WHEN, "SHIFT_META_LEFT_DOWN")
                          //#else
                          .Set(Am_START_WHEN, "SHIFT_MIDDLE_DOWN")
      //#endif
      ;
  cmd = only_r1.Get(Am_COMMAND);
  cmd.Set(Am_DO_METHOD, print_my_name);
  r1.Add_Part(only_r1);

  Am_Object only_r2 = Am_Choice_Interactor
                          .Create("only_r2")
                          //#if defined(_WINDOWS)
                          //    .Set(Am_START_WHEN, "SHIFT_META_LEFT_DOWN")
                          //#else
                          .Set(Am_START_WHEN, "SHIFT_MIDDLE_DOWN")
      //#endif
      ;
  cmd = only_r2.Get(Am_COMMAND);
  cmd.Set(Am_DO_METHOD, print_my_name);
  r2.Add_Part(only_r2);

  //Run also tests
  Am_Object run_also = Am_One_Shot_Interactor.Create("run_also_HELP")
                           .Set(Am_MULTI_OWNERS, true)
                           .Set(Am_START_WHEN, "HELP")
                           .Set(Am_RUN_ALSO, 1)
                           .Set(Am_PRIORITY, 200.0);
  ;
  cmd = run_also.Get(Am_COMMAND);
  cmd.Set(Am_DO_METHOD, print_my_name);
  window.Add_Part(run_also);
  //
  Am_Object run_also2 = Am_Move_Grow_Interactor.Create("run_also_C_MIDDLE")
                            .Set(Am_START_WHEN, "CONTROL_MIDDLE_DOWN")
                            .Set(Am_RUN_ALSO, 1)
                            .Set(Am_PRIORITY, 200.0);
  ;
  cmd = run_also2.Get(Am_COMMAND);
  cmd.Set(Am_DO_METHOD, print_my_name);
  window.Add_Part(run_also2);
  //
  Am_Object run_other = Am_One_Shot_Interactor.Create("run_also_F7")
                            .Set(Am_START_WHEN, "F7")
                            .Set(Am_PRIORITY, 150.0);
  ;
  cmd = run_other.Get(Am_COMMAND);
  cmd.Set(Am_DO_METHOD, print_my_name);
  window.Add_Part(run_other);
  Am_Object run_other2 = Am_One_Shot_Interactor.Create("run_also2_F7")
                             .Set(Am_START_WHEN, "F7")
                             .Set(Am_PRIORITY, 100.0);
  ;
  cmd = run_other2.Get(Am_COMMAND);
  cmd.Set(Am_DO_METHOD, print_my_name);
  window.Add_Part(run_other2);
  run_other.Set(Am_RUN_ALSO, run_other2);

  move_inter = Am_Move_Grow_Interactor.Create("move_object")
                   .Set(Am_START_WHERE_TEST, obj_in_any_win)
                   //#if defined(_WINDOWS)
                   //    .Set(Am_START_WHEN, "META_LEFT_DOWN")
                   //#else
                   .Set(Am_START_WHEN, "MIDDLE_DOWN")
                   //#endif
                   // next two to test various feedback options
                   .Set(Am_FEEDBACK_OBJECT, compute_feedback_obj)
                   .Add(USING_FEEDBACK, true)
                   .Add(USING_WINDOW, false)
                   .Set(Am_MULTI_OWNERS,
                        Am_Value_List().Add(window).Add(win3).Add(sub_window));
  window.Add_Part(move_inter);

  create_inter = Am_New_Points_Interactor.Create("create_objects")
                     .Set(Am_START_WHEN, "ANY_RIGHT_DOWN")
                     .Set(Am_AS_LINE, as_line_if_shift)
                     .Set(Am_FEEDBACK_OBJECT, compute_feedback_obj)
                     .Set(Am_START_WHERE_TEST, in_any_win)
                     .Set(Am_MULTI_OWNERS,
                          Am_Value_List().Add(window).Add(win3).Add(sub_window))
                     .Add(USING_FEEDBACK, true)
                     .Add(USING_WINDOW, false)
                     .Set(Am_CREATE_NEW_OBJECT_METHOD, create_new_object);
  ;
  window.Add_Part(create_inter);

  cout << flush << "Window depth=" << (int)window.Get(Am_OWNER_DEPTH)
       << " rank=" << (int)window.Get(Am_RANK) << endl;
  cout << "r1 depth=" << (int)r1.Get(Am_OWNER_DEPTH)
       << " rank=" << (int)r1.Get(Am_RANK) << endl;
  cout << "r2 depth=" << (int)r2.Get(Am_OWNER_DEPTH)
       << " rank=" << (int)r2.Get(Am_RANK) << endl;
  cout << "r3 depth=" << (int)r3.Get(Am_OWNER_DEPTH)
       << " rank=" << (int)r3.Get(Am_RANK) << endl;
  cout << "group1 depth=" << (int)group1.Get(Am_OWNER_DEPTH)
       << " rank=" << (int)group1.Get(Am_RANK) << endl;
  cout << "r4 depth=" << (int)r4.Get(Am_OWNER_DEPTH)
       << " rank=" << (int)r4.Get(Am_RANK) << endl;
  cout << "r5 depth=" << (int)r5.Get(Am_OWNER_DEPTH)
       << " rank=" << (int)r5.Get(Am_RANK) << endl
       << flush;

  Am_Main_Event_Loop();
  Am_Cleanup();

  return 0;
}
