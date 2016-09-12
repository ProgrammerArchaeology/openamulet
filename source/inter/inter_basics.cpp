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

// This file contains the basic functions for Interactors input handling
//
// Designed and implemented by Brad Myers
//
// bdk added supports multiple users 1/98

#include <am_inc.h>

#include AM_IO__H

#include <amulet/impl/inter_all.h>
#include <amulet/impl/am_inter_location.h>
#include PRIORITY_LIST__H
#include <amulet/impl/types_string.h>
#include <amulet/impl/opal_objects.h>
#include <amulet/impl/opal_default_demon.h>
#include <amulet/impl/opal_op.h>
#include FORMULA__H    // for creating formula constraints
#include VALUE_LIST__H // for Am_Value_List type
#include TEXT_FNS__H   // for Am_Edit_Translation_Table, etc.
#include INITIALIZER__H
#ifdef DEBUG
#include <amulet/impl/slots_registry.h>
#endif

//Global variable controlling the number of pixels that the mouse has
//to move before it is considered a DRAG event (like "LEFT_DRAG").
//Default value = 3 pixels.
//Am_Minimum_Move_For_Drag - 1 is the maximum the mouse can move
//between the down and the up and still be classified as a "CLICK",
//like "LEFT_CLICK".

int Am_Minimum_Move_For_Drag = 3;

////////////////////////////////////////////////////////////
// global method types
////////////////////////////////////////////////////////////

//in inter.h
AM_DEFINE_METHOD_TYPE_IMPL(Am_Where_Method)
AM_DEFINE_METHOD_TYPE_IMPL(Am_Event_Method)
AM_DEFINE_METHOD_TYPE_IMPL(Am_Custom_Gridding_Method)
AM_DEFINE_METHOD_TYPE_IMPL(Am_Create_New_Object_Method)
AM_DEFINE_METHOD_TYPE_IMPL(Am_Text_Edit_Method)
AM_DEFINE_METHOD_TYPE_IMPL(Am_Register_Command_Method)
AM_DEFINE_METHOD_TYPE_IMPL(Am_Mouse_Event_Method)
AM_DEFINE_METHOD_TYPE_IMPL(Am_Current_Location_Method)
AM_DEFINE_METHOD_TYPE_IMPL(Am_Text_Check_Legal_Method)

//in inter_advanced.h
AM_DEFINE_METHOD_TYPE_IMPL(Am_Inter_Internal_Method)

/////////////////////////////////////////////////////////////
// Enum types
/////////////////////////////////////////////////////////////

AM_DEFINE_ENUM_SUPPORT(Am_Choice_How_Set,
                       "Am_CHOICE_SET Am_CHOICE_CLEAR "
                       "Am_CHOICE_TOGGLE Am_CHOICE_LIST_TOGGLE")

AM_DEFINE_ENUM_SUPPORT(
    Am_Move_Grow_Where_Attach,
    "Am_ATTACH_WHERE_HIT Am_ATTACH_NW Am_ATTACH_N Am_ATTACH_NE "
    "Am_ATTACH_E Am_ATTACH_SE Am_ATTACH_S Am_ATTACH_SW Am_ATTACH_W "
    "Am_ATTACH_END_1 Am_ATTACH_END_2 Am_ATTACH_CENTER")

////////////////////////////////////////////////////////////
// Global variables for interactors
////////////////////////////////////////////////////////////

// list of interactors or special codes for tracing
Am_Value_List trace_list;
bool tracing_anything = false;

//global list for all inters with Am_MULTI_OWNERS = true
Am_Priority_List *all_wins_inter_list;

//global list of all modal windows currently in force
Am_Value_List Am_Modal_Windows;

//exported values

// The actual interactor objects
Am_Object Am_Interactor = 0; //base of the Interactor hierarchy

/*
Am_Object Am_Rotate_Interactor = 0;  // rotating
Am_Object Am_Animation_Interactor = 0;  // handling animations
*/

Am_Input_Char Am_Default_Start_Char;
Am_Input_Char Am_Default_Stop_Char;

////////////////////////////////////////////////////////////
// debug and tracing handling
////////////////////////////////////////////////////////////

// do we need to trace the next interactor to run?
bool am_trace_next_inter = false;

//prints current status
void
Am_Set_Inter_Trace()
{
  if (trace_list.Empty())

    std::cout << ">>Tracing NO interactors\n" << std::flush;
  else {
    std::cout << ">>Now Tracing Interactors ";
    trace_list.Start();
    if (trace_list.Get_Type() == Am_INT &&
        (int)trace_list.Get() == Am_INTER_TRACE_ALL) {
      std::cout << "EVERYTHING\n";
      return;
    }
    for (; !trace_list.Last(); trace_list.Next()) {
      Am_Value_Type typ = trace_list.Get_Type(); // type of the current element
      if (typ == Am_INT) {
        int vali = trace_list.Get();
        switch (vali) {
        case Am_INTER_TRACE_NONE:
          std::cout << "**Trace NONE (Error)\n";
          break;
        case Am_INTER_TRACE_SETTING:
          std::cout << "Setting Slots ";
          break;
        case Am_INTER_TRACE_EVENTS:
          std::cout << "Events ";
          break;
        case Am_INTER_TRACE_PRIORITIES:
          std::cout << "Priorities ";
          break;
        case Am_INTER_TRACE_NEXT:
          std::cout << "Next Interactor ";
          break;
        case Am_INTER_TRACE_SHORT:
          std::cout << "Short ";
          break;
        default:
          std::cout << "** Illegal trace code=" << vali << std::endl;
        }
      } else if (typ == Am_OBJECT) {
        Am_Object inter;
        inter = trace_list.Get();
        std::cout << inter << " ";
      } else {
        std::cout << "** entry in interactor trace list has wrong type = ";
        Am_Print_Type(std::cout, typ);
        std::cout << std::endl;
      }
    } //for loop
    std::cout << std::endl << std::flush;
  } // not empty
}

void
Am_Set_Inter_Trace(Am_Inter_Trace_Options trace_code)
{ //add trace of that
  if (trace_code == Am_INTER_TRACE_NONE) {
    trace_list.Make_Empty();
    tracing_anything = false;
    Am_Debug_Print_Input_Events = false; // in gem

  } else if (trace_code == Am_INTER_TRACE_NEXT) {
    //don't add anything to the trace list now, the next interactor
    //will be added
    am_trace_next_inter = true;
    std::cout << "\n>> Will trace the next Interactor to run\n\n" << std::flush;
    return; //don't do the Am_Set_Inter_Trace at the end,
            //since it is misleading
  } else if (trace_code == Am_INTER_TRACE_ALL ||
             trace_code == Am_INTER_TRACE_SHORT) {
    trace_list.Make_Empty();
    trace_list.Add((int)trace_code); // if ALL or SHORT, then that should be
                                     // all that is in the list
    tracing_anything = true;
    Am_Debug_Print_Input_Events = true; // in gem
  } else {
    trace_list.Add((int)trace_code);
    tracing_anything = true;
    Am_Debug_Print_Input_Events = true; // always trace events when
                                        // tracing other things
  }
  Am_Set_Inter_Trace(); // print resulting trace
}

void
Am_Set_Inter_Trace(Am_Object inter_to_trace)
{ //add trace of that inter
  trace_list.Add(inter_to_trace);
  tracing_anything = true;
  // when tracing an interactor, also trace events automatically
  trace_list.Start();
  if (!trace_list.Member((int)Am_INTER_TRACE_EVENTS)) {
    trace_list.Add((int)Am_INTER_TRACE_EVENTS);
    Am_Debug_Print_Input_Events = true;
  }
  // when tracing an interactor, also trace SETTING automatically
  trace_list.Start();
  if (!trace_list.Member((int)Am_INTER_TRACE_SETTING)) {
    trace_list.Add((int)Am_INTER_TRACE_SETTING);
  }
  Am_Set_Inter_Trace(); // print resulting trace
}

void
Am_Clear_Inter_Trace()
{ //set not trace, same as Am_Set_Inter_Trace(0)
  Am_Set_Inter_Trace(Am_INTER_TRACE_NONE);
}

//These are used internally to see if should print something
bool
Am_Inter_Tracing(Am_Inter_Trace_Options trace_code)
{
  if (tracing_anything) { // test this first for efficiency
    trace_list.Start();
    if (trace_list.Member((int)Am_INTER_TRACE_ALL))
      return true;
    else {
      trace_list.Start();
      if (trace_list.Member((int)trace_code))
        return true;
      else
        return false;
    }
  } else
    return false;
}

bool
Am_Inter_Tracing(Am_Object inter_to_trace)
{
  if (tracing_anything) { // test this first for efficiency
    trace_list.Start();
    if (trace_list.Member((int)Am_INTER_TRACE_ALL))
      return true;
    else {
      trace_list.Start();
      if (trace_list.Member(inter_to_trace))
        return true;
      else
        return false;
    }
  } else
    return false;
}

/* This is called by General-Go to check am_trace_next_inter.
*/
inline void
Check_And_Handle_Trace_Next_Inter(Am_Object inter)
{
  if (am_trace_next_inter) {
    am_trace_next_inter = false; //be sure to clear first in case set
    //by the tracing function
    std::cout << "\n>> ** Interactor " << inter
              << " started.  Starting tracing\n"
              << std::flush;
    Am_Set_Inter_Trace(inter);
  }
}

////////////////////////////////////////////////////////////
// Main top-level routine to accept events.
// Corresponds to process-events in garnet
////////////////////////////////////////////////////////////

/* ----------------------------------------------------------------
   Design for searching for which interactor to run:

When an input event occurs, all the interactors attached to any of the
objects from the leaf to the root (up the OWNER tree) are tried in
priority order, where the priority is just a number attached to each
interactor.  By default all interactors will be, priority 1.0 when
waiting and that number + Am_INTER_PRIORITY_DIFF (currently 100 --
defined in inter_advanced.h) when running.  Any positive or negative
number will work.  For interactors with the same number priority,
interactors attached closer to the leaf take priority and at the same
object, the priority is from front to back.  This is enforced by
looking at the slot PRIORITY of the graphical objects.  This slot is
maintained by Opal, with higher numbers going to objects that are more
towards the front.

In addition to a single OWNER graphical object (or group) that the
Interactor is attached to, interactors will have a START-WHEN (and
running when, etc).  The START-WHEN will be a function that takes the
interactor, owner and input event and determines whether to start or
not.  Built-in functions will include Am_Event_In and Am_Event_Part_Of
(corresponding to Garnet's :in and :element-of).  Most other tests
will be written by the programmer (rather than having a big built-in
set like currently).

        bam.garnet.cs.cmu.edu:0.0 (screen)
             |               \
        Multi-win-group1     win4
         /    |   \          / \
      win1  win2  win3  group2  group3
                                 |   |
                                o4   o5

Note that this means that the system cannot actually determine which
object an interactor wants to refer to.  For example, I1 might be
attached to group3 (above) and refer to Am_Event_Part_Of but I2 might
be attached directly to o5 and be Am_Event_In.  For an event in o5,
both might want to start.  If they had the same priority, then the one
directly on o5 would start.

After an interactor is found and the event given to it, the system
will continue to search for any interactors marked with the
Am_RUN_ALSO slot set to true that have equal priorities.  The event is
also given to this interactor.  The search starts in the same object
with the interactor that took the event (here, o5) and continues up
the owner tree.  More than one Am_RUN_ALSO interactor is allowed to
take the same event, but only one non-Am_RUN_ALSO interactor can.  If
the Am_RUN_ALSO interactor is found first, then the search continues
to see if there is a non-Am_RUN_ALSO interactor for the event with any
priority (the priority of the Am_RUN_ALSO interactor doesn't matter).
This means that more than one interactor is allowed to run, and the
various interactors do not need to know that another will also run (to
make balloon help easy).

Advanced feature: An interactor ACTION function may change the
Am_RUN_ALSO so this flag should be checked only AFTER the action
functions are called (by Inter_Check_Go).  This is useful for things
like the text_input_widget wanting to let a final mouse_down or RETURN
also invoke other interactors.

Another advanced feature: Am_RUN_ALSO can be set to an interactor or a list of
interacters as well as a boolean.  When the Am_RUN_ALSO slot is an interactor
then only that interactor is permitted to run along with the interactor that
has already started.  Likewise with a list, only interactors in the list are
permitted to run.  All other interactors are not allowed to run.

----------------------------------------------------------------
*/

// Function Prototypes
void clear_priority_list_for_window(Am_Object inter, Am_Object win);
void clear_priority_list_all_windows(Am_Object inter, Am_Object root_window);
void clear_priority_list_for_windows(Am_Object inter, Am_Value_List win_list);
void add_priority_list_for_window(Am_Object inter, Am_Object new_win,
                                  float main_priority, int second_priority,
                                  int third_priority, bool check_if_member,
                                  bool need_multi);
void add_priority_list_for_windows(Am_Object inter, Am_Value_List win_list,
                                   float main_priority, int second_priority,
                                   int third_priority);
void add_priority_list_all_windows(Am_Object inter, Am_Object root_window,
                                   float main_priority, int second_priority,
                                   int third_priority);
void adjust_inter_multi_owners_priority_list(Am_Object inter, Am_Object owner,
                                             Am_Value new_window_val);

////////////////////////////////////////////////

void
clear_priority_list_for_window(Am_Object inter, Am_Object win)
{
  if (win.Valid()) {
    Am_Priority_List *inter_list;
    Am_INTER_TRACE_PRINT(Am_INTER_TRACE_PRIORITIES,
                         "Removing inter " << inter << " from window " << win);
    inter_list =
        Am_Priority_List::Narrow(win.Get(Am_INTER_LIST, Am_NO_DEPENDENCY));
    if (inter_list) {
      inter_list->Delete(inter); // removes inter if there
    }
  }
}

// Remove Inter from global list.  This is
// used when we used to have an interactor with Am_MULTI_OWNERS = true
void
clear_all_wins_list_for_inter(Am_Object inter)
{
  Am_INTER_TRACE_PRINT(Am_INTER_TRACE_PRIORITIES,
                       "Removing inter "
                           << inter
                           << " from global list for Am_MULTI_OWNERS=true");
  all_wins_inter_list->Delete(inter); // removes inter if there
}

void
clear_priority_list_for_objects(Am_Object inter, Am_Value_List obj_list,
                                Am_Object skip_win)
{
  Am_Object obj, win;
  for (obj_list.Start(); !obj_list.Last(); obj_list.Next()) {
    obj = obj_list.Get();
    if (obj.Valid()) {
      //obj might not be valid if destroying the window
      win = obj.Get(
          Am_WINDOW,
          Am_NO_DEPENDENCY); //obj might not be a window, and even if so,
                             //windows' Am_WINDOW slot is itself
      if (win != skip_win)
        clear_priority_list_for_window(inter, win);
    }
  }
}

// if check_if_member is false, then assumes inter is not on new_window's
// list, and adds it, otherwise checks first.
// need_multi should be true when the window is part of a multi-window
// interactor
void
add_priority_list_for_window(Am_Object inter, Am_Object new_win,
                             float main_priority, int second_priority,
                             int third_priority, bool check_if_member,
                             bool need_multi)
{
  Am_Value value;
  value = new_win.Peek(Am_INTER_LIST, Am_NO_DEPENDENCY);
  Am_Priority_List *inter_list;
  if (value.Valid()) {
    inter_list = Am_Priority_List::Narrow(value);
    if (check_if_member && inter_list->Member(inter))
      inter_list->Delete(); // remove old version and add in new one
  } else {                  // need to create new list
    inter_list = Am_Priority_List::Create();
    //use the Pointer_Wrapper
    new_win.Set(Am_INTER_LIST, (Am_Am_Priority_List)inter_list,
                Am_OK_IF_NOT_THERE);
    //std::cout << " ~~~Allocated inter_list " << inter_list <<std::endl;
  }
  Am_INTER_TRACE_PRINT(
      Am_INTER_TRACE_PRIORITIES,
      "Adding inter " << inter << " to window " << new_win << " at priority ("
                      << main_priority << "," << second_priority << ","
                      << third_priority << ") multi= " << need_multi);
  inter_list->Add(inter, main_priority, second_priority, third_priority);

  // Now do multi-window.  This MUST be set up (for X) BEFORE the
  // window gets the the down-press input event that will start an
  // interactor.  Currently, only turn on multi; never turn it off.
  // To turn off, need to make sure NONE of the interactors on the
  // window care about multi-windows.
  if (need_multi) {
    Am_Drawonable *draw =
        Am_Drawonable::Narrow(new_win.Get(Am_DRAWONABLE, Am_NO_DEPENDENCY));
    if (draw)
      draw->Set_Multi_Window(need_multi);
    else //set slot so will be done when drawonable is finally created
      new_win.Set(Am_INIT_WANT_MULTI_WINDOW, true, Am_OK_IF_NOT_THERE);
  }
}

void
add_priority_list_for_objects(Am_Object inter, Am_Value_List obj_list,
                              Am_Object skip_win, float main_priority,
                              int second_priority, int third_priority)
{
  Am_Object obj, win;
  for (obj_list.Start(); !obj_list.Last(); obj_list.Next()) {
    obj = obj_list.Get();
    if (!obj.Valid()) {
      // This can happen when a window is being destroyed so don't raise error
      //Am_ERRORO("Invalid object " << obj << " added to MULTI_OWNERS list of "
      //      << inter, inter, Am_MULTI_OWNERS);
      return;
    }
    win =
        obj.Get(Am_WINDOW,
                Am_NO_DEPENDENCY); //obj might not be a window, and even if so,
                                   //windows' Am_WINDOW slot is itself
    if (win != skip_win)
      add_priority_list_for_window(inter, win, main_priority, second_priority,
                                   third_priority, true, true);
  }
}

// Go through all windows and add inter to their priority list.  This is
// used when we used to have an interactor with Am_MULTI_OWNERS = true
// *** Assume that do NOT need multi-window set on for all windows
// (i.e., won't try to move an object from one window to another using
// an interactor with Am_MULTI_OWNERS = true.)
void
add_inter_to_all_wins_list(Am_Object inter, float main_priority,
                           int second_priority, int third_priority)
{
  Am_INTER_TRACE_PRINT(
      Am_INTER_TRACE_PRIORITIES,
      "Adding inter "
          << inter << " to global list for Am_MULTI_OWNERS=true at priority ("
          << main_priority << "," << second_priority << "," << third_priority
          << ")");
  all_wins_inter_list->Add(inter, main_priority, second_priority,
                           third_priority);
}

void
set_want_move_one_win(Am_Object window, bool want_move)
{
  Am_Value v;
  int cnt = 0;
  int new_cnt = 0;
  v = window.Peek(Am_WINDOW_WANT_MOVE_CNT);
  if (v.Valid())
    cnt = v;
  if (want_move)
    new_cnt = cnt + 1;
  else if (cnt > 0)
    new_cnt = cnt - 1;

  window.Set(Am_WINDOW_WANT_MOVE_CNT, new_cnt, Am_OK_IF_NOT_THERE);
  //call if turning moving on, or if turning off and the count is 1 or less
  if (want_move || cnt <= 1) {
    Am_Drawonable *draw = Am_Drawonable::Narrow(window.Get(Am_DRAWONABLE));
    if (draw)
      draw->Set_Want_Move(want_move);
  }
}

//used when interactor needs to be added or removed from an extra
//window's list, for example when the feedback object is a window
//itself, call this on the window.  Also sets the window's move events
//to be want_move.
void
Am_Add_Remove_Inter_To_Extra_Window(Am_Object inter, Am_Object window, bool add,
                                    bool want_move)
{
  Am_Object owner = inter.Get_Owner();
  if (owner.Valid()) {
    float main_priority = inter.Get(Am_PRIORITY);
    int second_priority = owner.Get(Am_OWNER_DEPTH);
    int third_priority = owner.Get(Am_RANK);
    if (add) {
      add_priority_list_for_window(inter, window, main_priority,
                                   second_priority, third_priority, true, true);
    } else
      clear_priority_list_for_window(inter, window);
  }
  set_want_move_one_win(window, want_move);
}

//function to adjust the window_list.  Depends on the value of the
//Am_MULTI_OWNERS slot and window
void
adjust_inter_multi_owners_priority_list(Am_Object inter, Am_Object owner,
                                        Am_Value new_window_val)
{
  // second_priority is the Am_OWNER_DEPTH of the graphical object
  // (owner) of the interactor, computed by Opal and third is the Am_RANK
  if (owner.Valid()) {
    Am_Object main_win = owner.Get(Am_WINDOW, Am_NO_DEPENDENCY);
    if (new_window_val.Valid()) {
      // first, make the main window be multi-windows
      if (main_win.Valid()) {
        Am_Drawonable *draw = Am_Drawonable::Narrow(
            main_win.Get(Am_DRAWONABLE, Am_NO_DEPENDENCY));
        if (draw)
          draw->Set_Multi_Window(true);
        else //set slot so will be done when drawonable is finally created
          main_win.Set(Am_INIT_WANT_MULTI_WINDOW, true, Am_OK_IF_NOT_THERE);
      }
    }
    float main_priority = inter.Get(Am_PRIORITY);
    int second_priority = owner.Get(Am_OWNER_DEPTH);
    int third_priority = owner.Get(Am_RANK);
    // just do it the easy (expensive) way, clear out all old values, then set
    // all new values.
    Am_Value old_window_val;
    old_window_val = inter.Peek(Am_LAST_WINDOWS, Am_NO_DEPENDENCY);
    // clear out all old values
    switch (old_window_val.type) {
    case Am_INT:
      if ((int)old_window_val != 0)
        clear_all_wins_list_for_inter(inter); //non-zero is true
      break;
    case Am_BOOL:
      if ((bool)old_window_val != false)
        clear_all_wins_list_for_inter(inter); //non-zero is true
      break;
    case Am_WRAPPER_TYPE:
    case Am_VALUE_LIST: { //must be a list of windows
      Am_Value_List old_list;
      old_list = old_window_val;
      clear_priority_list_for_objects(inter, old_list, main_win);
      break;
    }
    default:
      if (!old_window_val.Exists())
        // no previous value, fine
        break;
      Am_Error("Illegal Am_LAST_WINDOWS in Interactor", inter, Am_LAST_WINDOWS);
    } // end switch on old value

    // now set based on new value
    switch (new_window_val.type) {
    case Am_INT:
      if ((int)new_window_val != 0) { //non-zero is true
        add_inter_to_all_wins_list(inter, main_priority, second_priority,
                                   third_priority);
      }
      break;
    case Am_BOOL:
      if ((bool)new_window_val != false) {
        add_inter_to_all_wins_list(inter, main_priority, second_priority,
                                   third_priority);
      }
      break;
    case Am_WRAPPER_TYPE:
    case Am_VALUE_LIST: { //must be a list of windows
      Am_Value_List new_list;
      new_list = new_window_val;
      add_priority_list_for_objects(inter, new_list, main_win, main_priority,
                                    second_priority, third_priority);
      break;
    }
    default:
      std::cerr << "** Amulet Error: Am_MULTI_OWNERS slot of " << inter
                << " should contain true, false, 0, 1, or\n"
                << "   a Am_Value_List of objects, but it contains a value"
                << " of type ";
      Am_Print_Type(std::cerr, new_window_val.type);
      std::cerr << std::endl << std::flush;
      Am_Error(inter, Am_MULTI_OWNERS);
    } // end switch on new value
    inter.Set(Am_LAST_WINDOWS, new_window_val, Am_OK_IF_NOT_THERE);
  }
}

// function used by the two formulas below.  Adjusts priority list for main
// window
void
adjust_inter_to_priority_list(Am_Object &inter, Am_Object &owner,
                              Am_Object &owner_window,
                              bool setting_main_priority, bool multiowner,
                              float main_priority, int second_priority = -1,
                              int third_priority = -1)
{
  //if window slot not set yet, set it
  if (!Am_Object(inter.Get(Am_WINDOW, Am_NO_DEPENDENCY)).Valid())
    inter.Set(Am_WINDOW, owner_window);
  Am_Priority_List *inter_list;
  Am_Object on_window;
  Am_Value value;
  value = inter.Peek(Am_LAST_WINDOW, Am_NO_DEPENDENCY);
  if (value.type == Am_OBJECT)
    on_window = value;

  //std::cout << "......For inter " << inter << " Multiowner " << multiowner
  //      << " old win " << on_window << " new win " << owner_window
  //      <<std::endl <<std::flush;
  if (!multiowner && on_window.Valid() && on_window == owner_window) {
    // then just change priority in place
    inter_list = Am_Priority_List::Narrow(
        on_window.Get(Am_INTER_LIST, Am_NO_DEPENDENCY));
    if (setting_main_priority) {
      Am_INTER_TRACE_PRINT(Am_INTER_TRACE_PRIORITIES,
                           "Changing main priority of "
                               << inter << " to be " << main_priority << " in "
                               << on_window);
      inter_list->Change_Main_Priority(inter, main_priority);
    } else {
      Am_INTER_TRACE_PRINT(Am_INTER_TRACE_PRIORITIES,
                           "Changing second/third priority of "
                               << inter << " to be (" << second_priority << ","
                               << third_priority << ") in " << on_window);
      inter_list->Change_Second_Third_Priority(inter, second_priority,
                                               third_priority);
    }
  } else { // don't have an old window or new window is different
    if (on_window.Valid()) {
      // then changing windows, remove from old window
      Am_INTER_TRACE_PRINT(Am_INTER_TRACE_PRIORITIES,
                           "Removing inter " << inter << " from window "
                                             << on_window);
      inter_list = Am_Priority_List::Narrow(
          on_window.Get(Am_INTER_LIST, Am_NO_DEPENDENCY));
      //std::cout << " ~~~Deleting from inter_list " << inter_list <<std::endl;
      if (inter_list)
        inter_list->Delete(inter); // removes inter if there
    }
    if (!multiowner && owner_window.Valid()) { // then add to new list
      Am_Value_Type typ;
      // second_priority is the Am_OWNER_DEPTH of the graphical object
      // (owner) of the interactor, computed by Opal and third is the Am_RANK
      if (setting_main_priority) {
        // then second and third parameters are bogus, have to Get them
        typ = owner.Get_Slot_Type(Am_OWNER_DEPTH);
        if (typ == Am_INT)
          second_priority = owner.Get(Am_OWNER_DEPTH, Am_NO_DEPENDENCY);
        typ = owner.Get_Slot_Type(Am_RANK);
        if (typ == Am_INT)
          third_priority = owner.Get(Am_RANK, Am_NO_DEPENDENCY);
      } else // need to get main_priority
        main_priority = inter.Get(Am_PRIORITY, Am_NO_DEPENDENCY);
      if (second_priority != -1 && third_priority != -1) {
        add_priority_list_for_window(inter, owner_window, main_priority,
                                     second_priority, third_priority, false,
                                     false);
        inter.Set(Am_LAST_WINDOW, owner_window, Am_OK_IF_NOT_THERE);
      } else
        inter.Set(Am_LAST_WINDOW, 0, Am_OK_IF_NOT_THERE);
    } else
      inter.Set(Am_LAST_WINDOW, 0, Am_OK_IF_NOT_THERE);
  }
}

//By putting the next three formulas into every interactor, it should
//automatically maintain its proper place in the window's inter_list whenever
//the priority of the interactor or the rank or depth of the owner object
//changes.  The first formula is for the interactor's priority and the second
//for the owner's two slots.  The third is for the Am_MULTI_OWNERS slot,
//to take care of multiple-window interactors

Am_Define_Formula(float, maintain_inter_to_window_list_priority)
{
  float my_priority = self.Get(Am_PRIORITY);
  Am_Object owner =
      self.Get_Owner(Am_NO_DEPENDENCY); // no dependency here on owner, since
                                        // have a dependency in other formula
  Am_Value multi_owners_value;
  multi_owners_value = self.Peek(Am_MULTI_OWNERS);
  bool multiowner = false;
  if (owner.Valid()) {
    if (multi_owners_value.Valid() && (multi_owners_value.type == Am_BOOL ||
                                       multi_owners_value.type == Am_INT))
      //then don't add to a local list because will be added to the global list
      multiowner = true;
    Am_Object owner_window =
        owner.Get(Am_WINDOW, Am_NO_DEPENDENCY | Am_RETURN_ZERO_ON_ERROR);
    adjust_inter_to_priority_list(self, owner, owner_window, true, multiowner,
                                  my_priority);
  }
  return my_priority; //just random, not used by anything
}

Am_Define_Formula(int, maintain_inter_to_window_list_rank)
{
  //dependencies on Owner and Owner's window
  Am_Object owner = self.Get_Owner();
  if (owner.Valid()) {
    Am_Value v = owner.Peek(Am_WINDOW);
    if (v.Valid()) {
      Am_Object owner_window = v;
      // second_priority is the Am_OWNER_DEPTH of the graphical object
      // (owner) of the interactor, computed by Opal and third is the Am_RANK
      bool multiowner = false;
      v = self.Peek(Am_MULTI_OWNERS);
      if (v.Valid() && (v.type == Am_BOOL || v.type == Am_INT))
        //then don't add to local list because will be added to the global list
        multiowner = true;
      int second_priority = owner.Get(Am_OWNER_DEPTH);
      int third_priority = owner.Get(Am_RANK);
      adjust_inter_to_priority_list(self, owner, owner_window, false,
                                    multiowner, 0.0f, second_priority,
                                    third_priority);
      return second_priority + third_priority; // not used by anything
    }
  }
  return 0;
}

Am_Define_Formula(int, maintain_inter_for_multi_owners)
{
  Am_Value multi_owners;
  multi_owners = self.Peek(Am_ALL_WINDOWS);
  if (!multi_owners.Valid())
    multi_owners = self.Peek(Am_MULTI_OWNERS);
  Am_Object owner = self.Get_Owner(); //may need to recalc this if owner
                                      //becomes valid
  adjust_inter_multi_owners_priority_list(self, owner, multi_owners);
  return 1; //return value ignored
}

////////////////////////////////////////////////////////////
// deferring events
////////////////////////////////////////////////////////////

class Deferred_Event_Holder
{
public:
  Am_Object inter;
  Am_Object event_window;
  Am_Object obj;
  Am_Input_Char want_ic;
  Am_Input_Char got_ic;
  int x, y;
  Deferred_Event_Holder *next;
};

Deferred_Event_Holder *Am_List_Of_Deferred_Events = (0L);

static void
set_inter_deferred(Am_Object &inter, Am_Input_Event *ev,
                   Am_Object &event_window, Am_Object &obj,
                   Am_Input_Char deferring)
{
  Am_Input_Char want_ic = deferring;
  Deferred_Event_Holder *holder = new Deferred_Event_Holder;
  holder->inter = inter;
  holder->obj = obj;
  holder->want_ic = want_ic;
  holder->got_ic = ev->input_char;
  holder->x = ev->x;
  holder->y = ev->y;
  holder->event_window = event_window;
  holder->next = Am_List_Of_Deferred_Events;
  Am_List_Of_Deferred_Events = holder;
  Am_INTER_TRACE_PRINT(inter, "Adding " << inter << " to deferred list");
  //turn on mouse moved
  set_want_move_one_win(event_window, true);
}

inline bool
big_enough_move(int i1, int i2)
{
  return (abs(i1 - i2)) >= Am_Minimum_Move_For_Drag;
}

static void
check_deferred_start_or_abort(Deferred_Event_Holder *holder, Am_Input_Event *ev,
                              Am_Object &event_window, bool &start, bool &abort)
{
  Am_INTER_TRACE_PRINT_NOENDL(holder->inter, "Checking deferred start of "
                                                 << holder->inter);
  start = false;
  abort = false;

#ifdef DEBUG
  int was_inprogress =
      holder->event_window.Get(Am_OBJECT_IN_PROGRESS, Am_RETURN_ZERO_ON_ERROR);
  if (was_inprogress & 2) {
    std::cerr << "** Processing events for window " << holder->event_window
              << " but crashed last time, so aborting deferred.\n"
              << std::flush;
    abort = true;
    return;
  }
#endif

  if (ev->input_char.button_down == Am_BUTTON_DOWN ||
      ev->input_char.click_count == Am_NOT_MOUSE) {
    abort = true;
  } else if (holder->event_window != event_window ||
             big_enough_move(holder->x, ev->x) ||
             big_enough_move(holder->y, ev->y)) {
    if (holder->want_ic.button_down == Am_BUTTON_DRAG)
      start = true;
    else if (holder->want_ic.button_down == Am_BUTTON_CLICK)
      abort = true;
  } else if (ev->input_char.button_down == Am_BUTTON_UP) {
    if (holder->want_ic.button_down == Am_BUTTON_DRAG)
      abort = true;
    else if (holder->want_ic.button_down == Am_BUTTON_CLICK)
      start = true;
  }
  //else leave both false

  Am_INTER_TRACE_PRINT(holder->inter, ". Returning abort " << abort << " start "
                                                           << start);
}

static void
remove_holder_item(Deferred_Event_Holder *prev, Deferred_Event_Holder *holder)
{
  //want move is a counter, so decrement every time
  set_want_move_one_win(holder->event_window, false);
  if (prev)
    prev->next = holder->next;
  else
    Am_List_Of_Deferred_Events = holder->next;
  delete holder;
}

static void
start_and_remove_deferred_inter(Deferred_Event_Holder *prev,
                                Deferred_Event_Holder *holder)
{
  Am_Object inter = holder->inter;
  Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SHORT, "Deferred Starting " << inter);
  Check_And_Handle_Trace_Next_Inter(inter);
  Am_Object event_window = holder->event_window;
  Am_Object obj = holder->obj;
  Am_Input_Event ev;
  ev.x = holder->x;
  ev.y = holder->y;
  ev.input_char = holder->got_ic;
  ev.draw = Get_a_drawonable(event_window);
  ev.time_stamp = 0;

  //remove the deferred holder first, in case the pending event pushes
  //a recursive main-event-loop.  Don't call remove_holder_item since
  //don't want to turn off move events until later.

  if (prev)
    prev->next = holder->next;
  else
    Am_List_Of_Deferred_Events = holder->next;
  delete holder;

#ifdef DEBUG
  event_window.Set(Am_OBJECT_IN_PROGRESS, 2, Am_OK_IF_NOT_THERE);
#endif

  inter.Set(Am_WINDOW, event_window);
  inter.Set(Am_START_OBJECT, obj);
  inter.Set(Am_CURRENT_OBJECT, obj);
  inter.Set(Am_START_CHAR, ev.input_char);
  inter.Set(Am_FIRST_X, ev.x);
  inter.Set(Am_FIRST_Y, ev.y);
  inter.Set(Am_INTERIM_X, ev.x);
  inter.Set(Am_INTERIM_Y, ev.y);
  Am_Inter_Internal_Method method = inter.Get(Am_INTER_START_METHOD);
  method.Call(inter, obj, event_window, &ev);

  if (event_window.Valid()) {
    //finish remove_holder_item
    set_want_move_one_win(event_window, false);
#ifdef DEBUG
    event_window.Set(Am_OBJECT_IN_PROGRESS, 0, Am_OK_IF_NOT_THERE);
#endif
  }
}

//check and see if any deferred interactor wants this event
static bool
check_deferred_events(Am_Input_Event *ev, Am_Object &event_window)
{
  if (!Am_List_Of_Deferred_Events)
    return false; //quick exit for normal case
  Deferred_Event_Holder *prev = (0L);
  Deferred_Event_Holder *next = Am_List_Of_Deferred_Events;
  Deferred_Event_Holder *holder;
  bool start, abort;
  bool found_event = false;
  Am_Object inter;
  do {
    holder = next;
    next = holder->next;
    check_deferred_start_or_abort(holder, ev, event_window, start, abort);
    if (start) {
      start_and_remove_deferred_inter(prev, holder);
      found_event = true;
    } else if (abort) {
      remove_holder_item(prev, holder);
      found_event = true;
    }
  } while (next);
  return found_event;
}

//check if should defer this event.
//inter just used for debugging
static bool
check_deferring_event(Am_Object &inter, Am_Input_Char target_char,
                      Am_Input_Char input_char)
{
  if (target_char.button_down == Am_BUTTON_CLICK ||
      target_char.button_down == Am_BUTTON_DRAG) {
    Am_Input_Char ic2 = target_char;
    ic2.button_down = Am_BUTTON_DOWN;
    if (ic2 == input_char) {
      Am_INTER_TRACE_PRINT_NOENDL(inter, "(click/drag match)");
      return true;
    }
  }
  return false;
}

/////////////////////////////////////////////////////////////////////////////
// Main top-level input handler.  Figures out which interactors to call for
// the event.  This is called from the Input_Event_Notify method for the
// default event handler class defined in Opal.
/////////////////////////////////////////////////////////////////////////////

static bool
int_check_event(Am_Object &inter, Am_Object &event_window,
                Am_Input_Char input_char, Am_Slot_Key when_slot,
                Am_Value &slot_value, Am_Input_Char &deferred)
{
  bool result = false;
  Am_Input_Char target_char;
  if (!slot_value.Valid()) {
    // luckily 0 and (0L) are illegal Am_Input_Char so can use as false
    result = false;
  } else if ((slot_value.type == Am_INT) || (slot_value.type == Am_LONG)) {
    long val = slot_value;
    // first test if event is true or false (for always or never)
    result = (val != 0);
  } else if (slot_value.type == Am_Input_Char::Type_ID()) {
    target_char = slot_value;
    Am_INTER_TRACE_PRINT_NOENDL(inter, target_char);
    // now do comparison
    result = (target_char == input_char);
    if (!result) {
      result = check_deferring_event(inter, target_char, input_char);
      if (result)
        deferred = target_char;
    }
  } else if (slot_value.type == Am_BOOL)
    result = slot_value;
  else if (slot_value.type == Am_STRING) {
    // convert string into right type
    Am_String sval;
    sval = slot_value;
    target_char = Am_Input_Char((char *)sval);
    Am_INTER_TRACE_PRINT_NOENDL(inter, (const char *)sval
                                           << " (str--converted to Input_Char)="
                                           << target_char);
    if (when_slot) { //otherwise is part of a list, so leave it alone
      // store it back into the slot so more efficient next time
      // (because no parsing will be needed next time)
      //don't delete the constraint in the slot, if any
      inter.Set(when_slot, target_char, Am_KEEP_FORMULAS);
    }
    // now do comparison
    result = (target_char == input_char);
    if (!result) {
      result = check_deferring_event(inter, target_char, input_char);
      if (result)
        deferred = target_char;
    }
  } else if (Am_Event_Method::Test(slot_value)) {
    Am_Event_Method method(slot_value);
    result = method.Call(inter, event_window, input_char);
  } else {
    Am_ERRORO(
        "Type of value in when slot is "
            << slot_value
            << " but should be a bool, string, Am_Input_Char or Am_Value_List",
        inter, when_slot);
  }

// here have result set
#ifdef DEBUG
  if (Am_Inter_Tracing(inter)) {
    if (result)
      std::cout << " * SUCCESS\n" << std::flush;
    else
      std::cout << " * FAILED\n" << std::flush;
  }
#endif
  return result;
}

// Tests the input_char from ev against the event designators stored in the
// interactor.  Stores the successful event char into the when_slot of the
// interactor.  Sets deferred to false normally, but to true if event
// wanted is DRAG or CLICK
static bool
check_event(Am_Object &inter, Am_Object &event_window, Am_Input_Char input_char,
            Am_Slot_Key when_slot, Am_Input_Char &deferred)
{
  Am_Value slot_value;
  slot_value = inter.Peek(when_slot);
  deferred = Am_No_Input_Char; //normal case

#ifdef DEBUG
  if (Am_Inter_Tracing(inter)) {
    std::cout << "Checking ";
    switch (when_slot) {
    case Am_START_WHEN:
      std::cout << "start";
      break;
    case Am_ABORT_WHEN:
      std::cout << "abort";
      break;
    case Am_STOP_WHEN:
      std::cout << "stop";
      break;
    }
    std::cout << " event = ";
  }
#endif
  if (Am_Value_List::Test(slot_value)) {
    Am_Value_List choices = slot_value;
    Am_Value list_value;
    Am_INTER_TRACE_PRINT(inter, " LIST");
    for (choices.Start(); !choices.Last(); choices.Next()) {
      list_value = choices.Get();
      Am_INTER_TRACE_PRINT_NOENDL(inter, "  Checking ");
      if (int_check_event(inter, event_window, input_char, 0, list_value,
                          deferred))
        return true;
    }
    return false; //nothing in the list matches
  } else
    return int_check_event(inter, event_window, input_char, when_slot,
                           slot_value, deferred);
}

// This function runs the start where test for detecting whether an interactor
// may begin.  It handles all the different cases that can be stored in the
// Am_START_WHERE_TEST slot.
static Am_Object
check_start_location(Am_Object &inter, Am_Object &event_window, int x, int y)
{
#ifdef DEBUG
  if (Am_Inter_Tracing(inter))
    std::cout << "Checking start where.. ";
#endif

  Am_Object result;
  Am_Value where_value;
  Am_Object owner = inter.Get_Owner();

  // WHERE is the owner the interactor is attached to
  where_value = inter.Peek(Am_START_WHERE_TEST);
  //first check for true or false
  if ((where_value.type == Am_INT) || (where_value.type == Am_BOOL)) {
    if (where_value.Valid())
      result = owner;
    else
      result = Am_No_Object;
  } else if (Am_Where_Method::Test(where_value)) {
    // check that owner of inter is visible
    if (Am_Object_And_Owners_Valid_And_Visible(owner)) {
      // is inside parent
      Am_Where_Method where_method(where_value);
      result = where_method.Call(inter, owner, event_window, x, y);
    } else
      Am_INTER_TRACE_PRINT(inter, " Object or owner(s) not visible");
  } else
    Am_ERRORO("Am_START_WHERE_TEST of "
                  << inter << " should be boolean or Am_Where_Method, but is "
                  << where_value,
              inter, Am_START_WHERE_TEST);

#ifdef DEBUG
  if (Am_Inter_Tracing(inter)) {
    if (result.Valid())
      std::cout << " ~~SUCCESS=" << result << std::endl << std::flush;
    else
      std::cout << " ~~Failed\n";
  }
#endif
  return result;
} // end check_start_location

// This function runs the running where test for detecting whether an
// interactor may continue.  First, it checks the running where object which
// is usually a boolean like true.  If the object is actually an object, this
// will pass it to the Am_RUNNING_WHERE_TEST method.
static Am_Object
check_running_location(Am_Object &inter, Am_Object &event_window, int x, int y)
{
#ifdef DEBUG
  if (Am_Inter_Tracing(inter))
    std::cout << "Checking running where.. ";
#endif

  Am_Object result;
  Am_Value where_value;

  where_value = inter.Peek(Am_RUNNING_WHERE_TEST);
  if ((where_value.type == Am_INT) || (where_value.type == Am_BOOL)) {
    if (where_value.Valid())
      result = event_window;
    else
      result = Am_No_Object;
  } else if (Am_Where_Method::Test(where_value)) {
    Am_Object owner = inter.Get_Owner();
    if (Am_Object_And_Owners_Valid_And_Visible(owner)) {
      // is inside parent
      Am_Where_Method where_method(where_value);
      result = where_method.Call(inter, owner, event_window, x, y);
    } else
      Am_INTER_TRACE_PRINT(inter, " Object or owner(s) not visible");
  } else { // bad type in slot
    std::cerr << "** Amulet Error: Contents of Am_RUNNING_WHERE_TEST slot"
              << " of inter " << inter
              << " is not Boolean or Am_Where_Method, type = ";
    Am_Print_Type(std::cerr, where_value.type);
    std::cerr << " Aborting\n";
    result = Am_No_Object;
    Am_Error(inter, Am_RUNNING_WHERE_TEST);
  }

#ifdef DEBUG
  if (Am_Inter_Tracing(inter)) {
    if (result.Valid())
      std::cout << " ~~SUCCESS=" << result << std::endl << std::flush;
    else
      std::cout << " ~~Failed\n";
  }
#endif
  return result;
} // end check_running_location

/* This is the main action procedure that makes the most interactors go.
   Corresponds to General-Go in Lisp Garnet.
   Called by Interactor_Input_Event_Notify.
   This procedure implements the state machine.  It
   is called by the main dispatcher when an event happens.
   The Event ev is the value returned by the window manager.
   This procedure call the Do-xxx procedures in the
   interactor, which are specialized for the particular type of interactor.
   The do-xxx procedures in turn call the xxx-action procedures.  These
   -action procedures my be supplied by outside applications.

   The complexity in the state machine implementation is that the same
   event may cause two things, e.g., both going outside and stop.  We cannot
   count of getting different events for this.
*/

// Main function called on an interactor in order to run it.  Handles
// interactor state transitions.  Calls the event and location tests and
// calls the interactor methods.
static bool
Inter_Check_Go(Am_Object &inter, Am_Input_Event *ev, Am_Object &event_window)
{

  Am_Inter_Internal_Method method;

  Am_Inter_State state = Am_Get_Inter_State(inter);
  if (state == Am_INTER_ABORTING)
    state = Am_INTER_WAITING;

  bool active = inter.Get(Am_ACTIVE);
  Am_Input_Char deferring = Am_No_Input_Char;

  Am_Object obj;

#ifdef DEBUG
  if (Am_Inter_Tracing(inter)) {
    std::cout << "\nEnter GO for " << inter << ", state=" << (int)state;
    float current_priority = inter.Get(Am_PRIORITY);
    std::cout << " priority = " << current_priority << std::endl << std::flush;
  }
#endif

  //must be active
  if (active == false) {
    if (state != Am_INTER_WAITING) {
      // have to abort running interactor
      Am_INTER_TRACE_PRINT(inter, "** Implicit become inactive");
      if ((bool)inter.Get(Am_INTER_BEEP_ON_ABORT))
        Am_Beep();
      method = inter.Get(Am_INTER_ABORT_METHOD);
      method.Call(inter, Am_No_Object, event_window, ev);
    }
    Am_INTER_TRACE_PRINT(inter, "Returning because Am_ACTIVE is NIL");
    return false;
  } // end active == false

  // Finished preliminary tests, now get to work

  switch (state) {
  case Am_INTER_ANIMATING:
    return false; // ignore animators

  case Am_INTER_WAITING:
    if (check_event(inter, event_window, ev->input_char, Am_START_WHEN,
                    deferring)) {
      obj = check_start_location(inter, event_window, ev->x, ev->y);
      if (obj.Valid()) {
        if (deferring.Valid()) {
          set_inter_deferred(inter, ev, event_window, obj, deferring);
          return false;
        } else {
          Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SHORT, "Starting " << inter);
          Check_And_Handle_Trace_Next_Inter(inter);
          inter.Set(Am_WINDOW, event_window);
          inter.Set(Am_START_OBJECT, obj);
          inter.Set(Am_CURRENT_OBJECT, obj);
          inter.Set(Am_START_CHAR, ev->input_char);
          inter.Set(Am_FIRST_X, ev->x);
          inter.Set(Am_FIRST_Y, ev->y);
          inter.Set(Am_INTERIM_X, ev->x);
          inter.Set(Am_INTERIM_Y, ev->y);
          method = inter.Get(Am_INTER_START_METHOD);
          method.Call(inter, obj, event_window, ev);
        }
      } else
        return false;
    } else
      return false;
    break;

  case Am_INTER_RUNNING:
  case Am_INTER_OUTSIDE:
    inter.Set(Am_WINDOW, event_window);
    inter.Set(Am_INTERIM_X, ev->x);
    inter.Set(Am_INTERIM_Y, ev->y);
    Check_And_Handle_Trace_Next_Inter(inter);
    // check for explicit abort
    if (check_event(inter, event_window, ev->input_char, Am_ABORT_WHEN,
                    deferring)) {
      Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SHORT, "Aborting " << inter);
      if ((bool)inter.Get(Am_INTER_BEEP_ON_ABORT))
        Am_Beep();
      method = inter.Get(Am_INTER_ABORT_METHOD);
      method.Call(inter, Am_No_Object, event_window, ev);
    } else { // not aborting
      obj = check_running_location(inter, event_window, ev->x, ev->y);
      bool stop = check_event(inter, event_window, ev->input_char, Am_STOP_WHEN,
                              deferring);

      if (!obj.Valid()) {
        // then is outside
        if (state == Am_INTER_RUNNING) {
          // just going outside
          Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SHORT, "Going outside for "
                                                         << inter);
          method = inter.Get(Am_INTER_OUTSIDE_METHOD);
          method.Call(inter, Am_No_Object, event_window, ev);
        } // end just going outside
        if (stop) {
          method = inter.Get(Am_INTER_OUTSIDE_STOP_METHOD);
          method.Call(inter, Am_No_Object, event_window, ev);
        }
      }      // end obj==0
      else { // obj is valid, so still inside
        inter.Set(Am_CURRENT_OBJECT, obj);
        if (state == Am_INTER_OUTSIDE) {
          // just going back inside
          Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SHORT, "Going back inside for "
                                                         << inter);
          method = inter.Get(Am_INTER_BACK_INSIDE_METHOD);
          method.Call(inter, obj, event_window, ev);
        } // end just going back inside

        if (stop) {
          Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SHORT, "Stopping " << inter);
          method = inter.Get(Am_INTER_STOP_METHOD);
          method.Call(inter, obj, event_window, ev);
        } else { //not stop, just run some more
          Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SHORT, "Running " << inter);
          method = inter.Get(Am_INTER_RUNNING_METHOD);
          method.Call(inter, obj, event_window, ev);
        }
      }    //end else obj is valid
    }      // end else not aborting
    break; //end case (Am_INTER_RUNNING)

  default:
    std::cerr << "** Amulet Error: Illegal state in Interactor " << inter
              << std::endl;
    Am_Error(inter, Am_CURRENT_STATE);
    break;
  } //end switch
  return true;
}

// returns true if there are modal windows and event_window isn't one.  Causes
// Am_Beep to occur if mouse is clicked on inactive window.
static bool
check_modal_windows(const Am_Object &event_window, Am_Input_Event *ev)
{
  if (Am_Modal_Windows.Empty())
    return false; //no modal windows
  Am_Object head;
  Am_Modal_Windows.Start();
  head = Am_Modal_Windows.Get();
  if (head == event_window)
    return false; // modal, but event_window
                  // is it
  //check if event_window is a sub_window of head
  Am_Object owner = event_window.Get_Owner();
  while (owner.Valid() && !owner.Is_Instance_Of(Am_Screen)) {
    if (owner == head)
      return false;
    owner = owner.Get_Owner();
  }
  //if get here, then will return true
  Am_INTER_TRACE_PRINT(Am_INTER_TRACE_PRIORITIES,
                       "----Input for window "
                           << event_window << " ignored\n"
                           << "----  because the modal window " << head
                           << " is blocking input");
  //first see if should beep; only beep if down event or keyboard
  if (ev->input_char.button_down == Am_BUTTON_DOWN ||
      ev->input_char.click_count == Am_NOT_MOUSE)
    Am_Beep();
  return true;
}

// begin supports multiple users
/*

Am_USER_ID and Am_INTERNAL_USER_ID are initalized as follows in
Am_Initialize_Interactors in below inter_basics.cc:

Am_USER_ID  = Am_ONE_AT_A_TIME
Am_INTERNAL_USER_ID = Am_ONE_AT_A_TIME  [note, this gets set by Amulet
             at runtime as each user runs the interactor and once
             a user has released an interactor it resets
             Am_INTERNAL_USER_ID to Am_ONE_AT_A_TIME]

Am_USER_ID is set by the developer and can be any of the following:

   Am_ONE_AT_A_TIME  which means only one user can use the interactor
                     at a time
   Am_EVERYONE_AT_SAME_TIME which means multiple users can use the
                            interactor at a time
   or a user identification of type Am_Value which means that only that
        specific user can use the interactor

Am_USER_ZERO is a special designation defined in gem.h for the main
console user

The developer must set and release Am_USER_ID as needed.

Am_ONE_AT_A_TIME and Am_EVERYONE_AT_SAME_TIME are declared in
inter_advanced.h

inter_is_go_for_user_and_event is called from Interactor_Input_Event_Notify
to determine if a particular interactor would process a particular event.
inter_is_go_for_user_and_event returns true if the interactor and event are
to be executed.

To make this decision, inter_is_go_for_user_and_event uses logic explained
in the following example where # means any user id can be entered.  3 means
a specific user we are testing.

        inter.Get        inter.Get
rule    (Am_USER_ID)     (Am_INTERNAL_USER_ID)  ev->user_id   return_value
----   ----------------- ---------------------  -----------   ---------------
#1      3                           #              3          true else false

#2      Am_EVERYONE_AT_SAME_TIME    #              #          true

#3      Am_ONE_AT_A_TIME            3              3          true else false

In other words,
Rule #1 if the Am_USER_ID is a specific user, and if the event's user is the
same, return true else return false.

Rule #2 if  Am_USER_ID == Am_EVERYONE_AT_SAME_TIME always return true

Rule #3 (and this is the default value for Am_USER_ID) if the internal ID and
the event's user ID are the same return true, else return false.

Other notes:
Am_User_Id is initialized in inter_basics.cc (in Am_Initialize_Interactors() )

The developer sets ev->user_id by setting global. Each event then has the user
attached to it.  Set the global Am_User_Id by:
Am_User_Id.Set(Am_USER_ID, user_value);

The current user can then be gotten by using:
Am_Value user_value = Am_User_Id.Get(Am_USER_ID);

*/
static bool
inter_is_go_for_user_and_event(Am_Object &inter, Am_Input_Event *ev)
{
  Am_Screen.Set(Am_LAST_USER_ID, ev->user_id); //bookkeeping

  Am_Value inter_user_id = inter.Get(Am_USER_ID, Am_OK_IF_NOT_THERE);

  //rule #3
  if (inter_user_id == Am_ONE_AT_A_TIME) {
    Am_Value internal_user_id =
        inter.Get(Am_INTERNAL_USER_ID, Am_OK_IF_NOT_THERE);
    if (!internal_user_id.Valid() || internal_user_id == ev->user_id ||
        internal_user_id == Am_ONE_AT_A_TIME)
      return true;
    else {
      Am_INTER_TRACE_PRINT(inter, "User IDs don't match for one-at-a-time "
                                      << inter << " internal_user_id "
                                      << internal_user_id << " event "
                                      << ev->user_id);

      return false;
    }
  } else

      // rule #2
      if (inter_user_id == Am_EVERYONE_AT_SAME_TIME)
    return true;
  else

      // rule #1
      if (inter_user_id == ev->user_id)
    return true;
  else {
    Am_INTER_TRACE_PRINT(inter, "User IDs don't match for "
                                    << inter << " user_id " << inter_user_id
                                    << " event " << ev->user_id);
    return false;
  }
}
// end supports multiple users

// ** Main entry point for Interactors **
// First routine called by the opal event handler.  Handles modal window
// interaction and priority list management.  Calls Inter_Check_Go on
// individual interactors.
static void
Interactor_Input_Event_Notify(Am_Object event_window, Am_Input_Event *ev)
{
  //first check if modal window
  if (check_modal_windows(event_window, ev))
    return;

  //now check if a deferred interactor wants this event
  if (check_deferred_events(ev, event_window)) {
    return;
  }

#ifdef DEBUG
  int was_inprogress =
      event_window.Get(Am_OBJECT_IN_PROGRESS, Am_RETURN_ZERO_ON_ERROR);
  if (was_inprogress & 2) {
    std::cerr << "** Processing events for window " << event_window
              << " but crashed last time, so skipping it.\n"
              << std::flush;
    return; //skip this object
  } else
    event_window.Set(Am_OBJECT_IN_PROGRESS, 2, Am_OK_IF_NOT_THERE);
#endif

  Am_Value inter_list_value = event_window.Peek(Am_INTER_LIST);
  Am_Priority_List *inter_list;
  bool wants_event;

  if (inter_list_value.Valid()) {
    Am_INTER_TRACE_PRINT(Am_INTER_TRACE_PRIORITIES,
                         "----Interactors for window " << event_window);
    inter_list = Am_Priority_List::Narrow(inter_list_value);
  } else {
    Am_INTER_TRACE_PRINT(Am_INTER_TRACE_PRIORITIES,
                         "----NO Interactors for window " << event_window);
    // no interactor list, still have to check global ones so don't exit
    inter_list = (0L);
  }

  // process all the interactors in inter_list in priority order

  Am_Object inter;
  Am_Value run_also_value;
  Am_Object run_also_object;
  Am_Value_List run_also_list;
  bool doing_RUN_ALSO_only = false; //true if found an inter w/run_also
  bool check_RUN_ALSO_list_or_object = false;
  bool check_RUN_ALSO_object = false;
  bool check_RUN_ALSO_list = false;
  float run_also_priority = 0.0f;
  float this_priority;

  if (inter_list) {
    inter_list->Note_Reference();
    inter_list->Start();
  }
  all_wins_inter_list->Start();

  //std::cout <<"\nGlobal list: " << *all_wins_inter_list << endl
  //  << "List of " << event_window << " = " << *inter_list <<std::endl <<std::flush;

  while (all_wins_inter_list->Two_List_Get_Next(inter_list, inter,
                                                this_priority)) {
    //go through inter_list and all_wins_inter_list in parallel,
    //always getting the next priority inter from each

    // std::cout << "-<.> processing " << inter << " doing_RUN_ALSO_only "
    //      << doing_RUN_ALSO_only << " check_RUN_ALSO_list_or_object "
    //      << check_RUN_ALSO_list_or_object << " run_also_object= "
    //	  << run_also_object << " run_also_list " << run_also_list
    //	  <<std::endl <<std::flush;

    // Sometimes the window gets destroyed and all these interactors are
    // destroyed as well.  Must check to make sure interactors are okay.
    if (!inter.Valid())
      continue;

    if (doing_RUN_ALSO_only &&
        !check_RUN_ALSO_list_or_object) { // then found a regular inter, stop when priority is lower than
      // run_also_priority
      if (this_priority < run_also_priority)
        break; // then done.
    }          // end if (doing_RUN_ALSO_only)

    // std::cout << "__check " << inter << " doing run also=" << doing_RUN_ALSO_only
    //   << " inter runalso=" << (bool)inter.Get(Am_RUN_ALSO) <<std::endl <<std::flush;

    if (check_RUN_ALSO_list)
      run_also_list.Start();

    if ((!doing_RUN_ALSO_only ||
         (check_RUN_ALSO_list_or_object &&
              // if the run_also object is set then inter must be that object
              (check_RUN_ALSO_object && inter == run_also_object) ||
          // if the run_also list is set then inter must be member of list
          (check_RUN_ALSO_list && run_also_list.Member(inter))) ||
         //inter itself is a run_also
         (!check_RUN_ALSO_list_or_object && inter.Get(Am_RUN_ALSO).Valid())) &&
        // supports multiple users
        inter_is_go_for_user_and_event(inter, ev)) {
      //--> The actual running of the Interactor is here <--//
      wants_event = Inter_Check_Go(inter, ev, event_window);

      if (wants_event) {
        // Found one to run.  If it is not a RUN_ALSO_only, then only
        // run the RUN_ALSO interactors of the same priority level.
        // If it is a RUN_ALSO, then just keep going.
        // Be sure to fetch Am_RUN_ALSO again after Inter_Check_Go in
        // case changed by the running.
        if (inter.Valid()) {
          //inter might be destroyed while running
          run_also_value = inter.Peek(Am_RUN_ALSO);
          if (!run_also_value.Valid()) {
            //when Am_RUN_ALSO is not valid, then only run other
            // RUN_ALSO interactors.
            doing_RUN_ALSO_only = true;
            run_also_priority = this_priority;
          } else { //valid, see if object or list or ==true
            if (Am_Value_List::Test(run_also_value)) {
              run_also_list = run_also_value;
              doing_RUN_ALSO_only = true;
              run_also_priority = this_priority;
              check_RUN_ALSO_list = true;
              check_RUN_ALSO_list_or_object = true;
            } else if (run_also_value.type == Am_OBJECT) {
              run_also_object = run_also_value;
              doing_RUN_ALSO_only = true;
              run_also_priority = this_priority;
              check_RUN_ALSO_object = true;
              check_RUN_ALSO_list_or_object = true;
            }
            //else must be ==true, so continue to run other
            //interactors (leave doing_RUN_ALSO_only as false)
          }
        } else {
          // inter destroyed while running, but still only run other
          // run_also interactors.
          doing_RUN_ALSO_only = true;
          run_also_priority = this_priority;
        }
      }
    }
  }
  if (inter_list)
    inter_list->Release();
#ifdef DEBUG
  if (event_window.Valid()) //may not be valid while destroying window
    event_window.Set(Am_OBJECT_IN_PROGRESS, 0, Am_OK_IF_NOT_THERE);
#endif
}

//checks that object & all owner's are visible (all the way to the Am_Screen).
bool
Am_Object_And_Owners_Valid_And_Visible(Am_Object obj)
{
  while (true) {
    if (!obj.Valid())
      return false;
    if (!(bool)obj.Get(Am_VISIBLE, Am_NO_DEPENDENCY))
      return false;
    if (obj == Am_Screen || obj.Get_Prototype() == Am_Screen)
      return true;
    obj = obj.Get_Owner(Am_NO_DEPENDENCY);
  }
}

void
do_update()
{
  Main_Demon_Queue.Invoke();
  Am_Update_All();
}

//Initialized the state if not there yet
Am_Inter_State
Am_Get_Inter_State(Am_Object inter)
{
  Am_Value value;
  Am_Inter_State state;
  //state slot usually not there because is local-only
  value = inter.Peek(Am_CURRENT_STATE);
  if (value.type == Am_INT)
    state = (Am_Inter_State)(int)value;
  else {
    state = Am_INTER_WAITING; // if slot not there yet, then must be waiting
    inter.Set(Am_CURRENT_STATE, (int)Am_INTER_WAITING, Am_OK_IF_NOT_THERE);
  }
  if (state == Am_INTER_ABORTING) {
    //state is aborting only while the abort is taking place
    state = Am_INTER_WAITING;
    inter.Set(Am_CURRENT_STATE, (int)Am_INTER_WAITING, Am_OK_IF_NOT_THERE);
  }
  return state;
}

//Explicitly abort an interactor.  If not running, does nothing.
void
Am_Abort_Interactor(Am_Object inter, bool update_now)
{
  Am_Inter_State state = Am_Get_Inter_State(inter);

  if (state == Am_INTER_WAITING || state == Am_INTER_ABORTING) {
    Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SHORT,
                         "Explicitly Aborting " << inter << " but not running");
    inter.Set(Am_CURRENT_STATE, (int)Am_INTER_ABORTING, Am_OK_IF_NOT_THERE);
    return;
  }
  // else is running, abort the interactor

  Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SHORT, "Explicitly Aborting " << inter);
  Am_Input_Event ev;
  ev.input_char = inter.Get(Am_ABORT_WHEN);
  ev.x = 0;
  ev.y = 0;
  ev.draw = (0L);
  ev.time_stamp = 0;
  Am_Inter_Internal_Method method;
  method = inter.Get(Am_INTER_ABORT_METHOD);
  method.Call(inter, Am_No_Object, Am_No_Object, &ev);
  if (update_now)
    do_update();
}

//Explicitly stop an interactor.  If not running, raises an error.
//If not supply stop_obj, uses last one from inter.
// If not supply stop_window, uses stop_obj's and sets stop_x and stop_y
//to stop_obj's origin.  If supply stop_window, must also supply stop_x _y
void
Am_Stop_Interactor(Am_Object inter, Am_Object stop_obj, Am_Input_Char stop_char,
                   Am_Object stop_window, int stop_x, int stop_y,
                   bool update_now)
{

  Am_Inter_State state = Am_Get_Inter_State(inter);

  if (state == Am_INTER_WAITING) {
    Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SHORT, "Explicitly stopping Interactor "
                                                   << inter
                                                   << " but it is not running");
    return;
  }

  Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SHORT, "Explicitly Stopping " << inter);
  if (!stop_obj.Valid())
    stop_obj = inter.Get(Am_CURRENT_OBJECT);
  else
    inter.Set(Am_CURRENT_OBJECT, stop_obj);
  if (stop_obj.Valid()) {
    if (!stop_window.Valid()) {
      stop_window = stop_obj.Get(Am_WINDOW);
      if (stop_window.Valid() &&
          Am_Translate_Coordinates(stop_obj, 0, 0, stop_window, stop_x, stop_y))
        //then have pos of start_obj in start_window
        ;
      else {
        stop_x = stop_obj.Get(Am_LEFT);
        stop_y = stop_obj.Get(Am_TOP);
      }
    }
  }
  //now stop the interactor
  inter.Set(Am_WINDOW, stop_window);
  Am_Input_Event ev;
  ev.input_char = stop_char;
  ev.x = stop_x;
  ev.y = stop_y;
  ev.draw = (0L);
  ev.time_stamp = 0;
  Am_Inter_Internal_Method method;
  method = inter.Get(Am_INTER_STOP_METHOD);
  method.Call(inter, stop_obj, stop_window, &ev);
  if (update_now)
    do_update();
}

//Explicitly start an interactor.  If already running, does nothing.
//If not supply start_obj, uses inter's owner.
// If not supply start_window, uses start_obj's and sets start_x and start_y
//to start_obj's origin.  If supply start_window, must also supply start_x _y
void
Am_Start_Interactor(Am_Object inter, Am_Object start_obj,
                    Am_Input_Char start_char, Am_Object start_window,
                    int start_x, int start_y, bool update_now)
{

  Am_Inter_State state = Am_Get_Inter_State(inter);
  if (state != Am_INTER_WAITING)
    return;

  Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SHORT, "Explicitly Starting " << inter);
  Check_And_Handle_Trace_Next_Inter(inter);
  if (!start_obj.Valid())
    start_obj = inter.Get_Owner();
  if (!start_window.Valid() && start_obj.Valid()) {
    start_window = start_obj.Get(Am_WINDOW);
    if (start_window.Valid() &&
        Am_Translate_Coordinates(start_obj, 0, 0, start_window, start_x,
                                 start_y))
      //then have pos of start_obj in start_window
      ;
    else {
      start_x = start_obj.Get(Am_LEFT);
      start_y = start_obj.Get(Am_TOP);
    }
  }
  //now start the interactor
  inter.Set(Am_WINDOW, start_window);
  inter.Set(Am_START_OBJECT, start_obj);
  inter.Set(Am_CURRENT_OBJECT, start_obj);
  inter.Set(Am_START_CHAR, start_char);
  inter.Set(Am_FIRST_X, start_x);
  inter.Set(Am_FIRST_Y, start_y);
  Am_Input_Event ev;
  ev.input_char = start_char;
  ev.x = start_x;
  ev.y = start_y;
  ev.draw = (0L);
  ev.time_stamp = 0;
  Am_Inter_Internal_Method method;
  method = inter.Get(Am_INTER_START_METHOD);
  method.Call(inter, start_obj, start_window, &ev);
  if (update_now)
    do_update();
}

////////////////////////////////////////////////////////////
// Priority Level Manipulations
////////////////////////////////////////////////////////////

static void
change_priority_level(Am_Object &inter, bool from_running, bool to_running)
{
  float current_priority = inter.Get(Am_PRIORITY);
  if (from_running)
    current_priority = current_priority - Am_INTER_PRIORITY_DIFF;
  if (to_running)
    current_priority = current_priority + Am_INTER_PRIORITY_DIFF;
  inter.Set(Am_PRIORITY, current_priority);
  // Setting the priority should invoke the formula above:
  // maintain_inter_to_window_list_priority, which will adjust the interactor
  // in the window's inter_list.
}

static void
set_want_move_all_wins(const Am_Object &root_window, bool want_move)
{
  Am_Instance_Iterator instance_iter(root_window);
  Am_Object win;
  for (instance_iter.Start(); !instance_iter.Last(); instance_iter.Next()) {
    win = instance_iter.Get();
    if ((bool)win.Get(Am_VISIBLE) && win.Get_Owner().Valid())
      //just visible windows
      set_want_move_one_win(win, want_move);
    //recurse on instance
    set_want_move_all_wins(win, want_move);
  }
}

static void
set_want_move_for_objs_wins(Am_Value_List &obj_list, const Am_Object &skip_win,
                            bool want_move)
{
  Am_Object obj, win;
  for (obj_list.Start(); !obj_list.Last(); obj_list.Next()) {
    obj = obj_list.Get();
    win = obj.Get(Am_WINDOW);
    if (win != skip_win)
      set_want_move_one_win(win, want_move);
  }
}

// if want_move, then want move events.  If interactor is
// multi-window, then do for all windows it is attached to
static void
set_want_move(const Am_Object &inter, bool want_move)
{
  //first do the main window
  Am_Object owner, main_win;
  owner = inter.Get_Owner();
  if (owner.Valid()) {
    main_win = owner.Get(Am_WINDOW);
    if (main_win.Valid())
      set_want_move_one_win(main_win, want_move);

    // next, do any other windows
    Am_Value value;
    value = inter.Peek(Am_ALL_WINDOWS);
    if (!value.Valid())
      value = inter.Peek(Am_MULTI_OWNERS);
    switch (value.type) {
    case Am_INT:
      if ((int)value != 0)
        //Am_window is root of instance tree
        set_want_move_all_wins(Am_Window, want_move);
      break;
    case Am_BOOL:
      if ((bool)value != false)
        set_want_move_all_wins(Am_Window, want_move);
      break;
    case Am_VALUE_LIST: { //a list of objects
      Am_Value_List new_list;
      new_list = value;
      if (main_win.Valid())
        set_want_move_for_objs_wins(new_list, main_win, want_move);
      break;
    }
    } // end switch on new value
  }
}

void
Am_Copy_Values_To_Command(Am_Object from_object)
{
  if (from_object.Valid()) {
    Am_Object command = from_object.Get_Object(Am_COMMAND);
    Am_Value value;
    value = from_object.Peek(Am_VALUE);
    command.Set(Am_VALUE, value.Exists() ? value : Am_No_Value,
                Am_OK_IF_NOT_THERE);
    value = from_object.Peek(Am_OBJECT_MODIFIED);
    command.Set(Am_OBJECT_MODIFIED, value.Exists() ? value : Am_No_Value,
                Am_OK_IF_NOT_THERE);
    value = from_object.Peek(Am_OLD_VALUE);
    command.Set(Am_OLD_VALUE, value.Exists() ? value : Am_No_Value,
                Am_OK_IF_NOT_THERE);
  }
}

void
Am_Inter_Call_Method(Am_Object &inter_or_cmd, Am_Slot_Key method_slot, int x,
                     int y, Am_Object &ref_obj, Am_Input_Char &ic,
                     Am_Object &object_modified, Am_Inter_Location &points)
{
  //be careful that have object and it has the slot and the slot has a method
  if (inter_or_cmd.Valid()) {
    Am_Value method_value;
    method_value = inter_or_cmd.Peek(method_slot);
    if (method_value.Valid()) {
      Am_Method_Wrapper *wrapper;
      wrapper = method_value;
      Am_INTER_TRACE_PRINT_NOENDL(
          inter_or_cmd, "%%Executing " << Am_Get_Slot_Name(method_slot)
                                       << " of " << inter_or_cmd << "=");
      Am_ID_Tag id = wrapper->ID();
      if (id == Am_Object_Method::Am_Object_Method_ID) {
        Am_Object_Method method;
        method = wrapper;
        Am_INTER_TRACE_PRINT(inter_or_cmd, method);
        if (method.Valid())
          method.Call(inter_or_cmd);
      } else if (id == Am_Mouse_Event_Method::Am_Mouse_Event_Method_ID) {
        Am_Mouse_Event_Method method;
        method = wrapper;
        Am_INTER_TRACE_PRINT(inter_or_cmd, method);
        if (method.Valid())
          method.Call(inter_or_cmd, x, y, ref_obj, ic);
      } else if (id ==
                 Am_Current_Location_Method::Am_Current_Location_Method_ID) {
        if (!points.Valid())
          Am_Error("Method of type Current_Location_Method but no data points",
                   inter_or_cmd, method_slot);
        Am_Current_Location_Method method;
        method = wrapper;
        Am_INTER_TRACE_PRINT(inter_or_cmd, method);
        if (method.Valid())
          method.Call(inter_or_cmd, object_modified, points);
      } else
        Am_Error("Wrong type method: can only accept methods of types "
                 "Am_Object_Method, Am_Mouse_Event_Method, or "
                 "Am_Current_Location_Method.",
                 inter_or_cmd, method_slot);
    }
  }
}

//returns true if should continue processing
bool
Am_Inter_Call_Both_Method(Am_Object &inter, Am_Object &command_obj,
                          Am_Slot_Key method_slot, int x, int y,
                          Am_Object &ref_obj, Am_Input_Char &ic,
                          Am_Object &object_modified, Am_Inter_Location &points)
{
  Am_Inter_Call_Method(inter, method_slot, x, y, ref_obj, ic, object_modified,
                       points);
  Am_Value state;
  //method might destroy interactor, so be cautious
  if (!inter.Valid())
    return false;
  state = inter.Peek(Am_CURRENT_STATE);
  //see if that method destroyed the interactor or aborted it when not
  //doing the abort method
  if (!state.Exists() ||
      ((int)state == Am_INTER_ABORTING && method_slot != Am_ABORT_DO_METHOD))
    return false;
  // otherwise, continue

  Am_Inter_Call_Method(command_obj, method_slot, x, y, ref_obj, ic,
                       object_modified, points);
  //method might destroy interactor, so be cautious
  if (!inter.Valid())
    return false;

  state = inter.Peek(Am_CURRENT_STATE);
  if (!state.Exists() || (int)state == Am_INTER_ABORTING)
    return false;
  else
    return true;
}

//also calls the parent's command's do
void
Am_Register_For_Undo(Am_Object inter, Am_Object command_obj,
                     Am_Object object_modified, Am_Inter_Location data,
                     Am_Impl_Command_Setter *impl_command_setter)
{
  Am_Object undo_handler = Am_Inter_Find_Undo_Handler(inter);
  Am_Register_Command_Method method;
  if (undo_handler.Valid()) {
    method = undo_handler.Get(Am_REGISTER_COMMAND);
    if (method.Valid() && impl_command_setter) {
      Am_Object impl_command =
          impl_command_setter(inter, object_modified, data);
      if (impl_command.Valid()) {
        if (command_obj.Valid())
          impl_command.Set(Am_IMPLEMENTATION_PARENT, command_obj);
        impl_command.Set(Am_SAVED_OLD_OWNER, inter, Am_OK_IF_NOT_THERE);
        method.Call(undo_handler, impl_command);
      }
      if (command_obj.Valid())
        method.Call(undo_handler, command_obj);
    }
  }
  if (command_obj.Valid()) {
    Am_Process_All_Parent_Commands(command_obj, undo_handler, method);
  }
}

void
Am_Call_Final_Do_And_Register(Am_Object inter, Am_Object command_obj, int x,
                              int y, Am_Object ref_obj, Am_Input_Char ic,
                              Am_Object object_modified, Am_Inter_Location data,
                              Am_Impl_Command_Setter *impl_command_setter)
{
  if (Am_Inter_Call_Both_Method(inter, command_obj, Am_DO_METHOD, x, y, ref_obj,
                                ic, object_modified, data))
    Am_Register_For_Undo(inter, command_obj, object_modified, data,
                         impl_command_setter);
}

// used in many interactors and widgets in their
// Am_SET_COMMAND_OLD_OWNER to set the Am_SAVED_OLD_OWNER of their
// command to be the interactor or widget
Am_Define_Formula(int, Am_Set_Old_Owner_To_Me)
{
  Am_Value value;
  value = self.Peek(Am_COMMAND);
  if (value.Valid() && value.type == Am_OBJECT) {
    Am_Object cmd;
    cmd = value;
    cmd.Set(Am_SAVED_OLD_OWNER, self, Am_OK_IF_NOT_THERE);
    return 1;
  }
  return 0;
}

// General Procedures for commands and undo are defined in command_basics.cc
// Specific procedures are in the various inter_*.cc files

////////////////////////////////////////////////////////////

// Standard slots of interactors and command objects
void
inter_set_slot_names()
{
#ifdef DEBUG
  Am_Register_Slot_Key(Am_START_WHEN, "START_WHEN");
  Am_Register_Slot_Key(Am_START_WHERE_TEST, "START_WHERE_TEST");
  Am_Register_Slot_Key(Am_RUNNING_WHERE_TEST, "RUNNING_WHERE_TEST");
  Am_Register_Slot_Key(Am_ABORT_WHEN, "ABORT_WHEN");
  Am_Register_Slot_Key(Am_STOP_WHEN, "STOP_WHEN");
  Am_Register_Slot_Key(Am_INTER_START_METHOD, "~INTER_START_METHOD~");
  Am_Register_Slot_Key(Am_INTER_RUNNING_METHOD, "~INTER_RUNNING_METHOD~");
  Am_Register_Slot_Key(Am_INTER_OUTSIDE_METHOD, "~INTER_OUTSIDE_METHOD~");
  Am_Register_Slot_Key(Am_INTER_BACK_INSIDE_METHOD,
                       "~INTER_BACK_INSIDE_METHOD~");
  Am_Register_Slot_Key(Am_INTER_STOP_METHOD, "~INTER_STOP_METHOD~");
  Am_Register_Slot_Key(Am_INTER_OUTSIDE_STOP_METHOD,
                       "~INTER_OUTSIDE_STOP_METHOD~");
  Am_Register_Slot_Key(Am_INTER_ABORT_METHOD, "~INTER_ABORT_METHOD~");
  Am_Register_Slot_Key(Am_COMMAND, "COMMAND");
  Am_Register_Slot_Key(Am_IMPLEMENTATION_COMMAND, "IMPLEMENTATION_COMMAND");
  Am_Register_Slot_Key(Am_START_DO_METHOD, "START_DO_METHOD");
  Am_Register_Slot_Key(Am_INTERIM_DO_METHOD, "INTERIM_DO_METHOD");
  Am_Register_Slot_Key(Am_ABORT_DO_METHOD, "ABORT_DO_METHOD");
  Am_Register_Slot_Key(Am_DO_METHOD, "DO_METHOD");
  Am_Register_Slot_Key(Am_CONTINUOUS, "CONTINUOUS");
  Am_Register_Slot_Key(Am_ACTIVE, "ACTIVE");
  Am_Register_Slot_Key(Am_PRIORITY, "PRIORITY");
  Am_Register_Slot_Key(Am_GRID_X, "GRID_X");
  Am_Register_Slot_Key(Am_GRID_Y, "GRID_Y");
  Am_Register_Slot_Key(Am_GRID_ORIGIN_X, "GRID_ORIGIN_X");
  Am_Register_Slot_Key(Am_GRID_ORIGIN_Y, "GRID_ORIGIN_Y");
  Am_Register_Slot_Key(Am_GRID_METHOD, "GRID_METHOD");
  Am_Register_Slot_Key(Am_INTER_BEEP_ON_ABORT, "INTER_BEEP_ON_ABORT");

  Am_Register_Slot_Key(Am_HOW_SET, "HOW_SET");
  Am_Register_Slot_Key(Am_FIRST_ONE_ONLY, "FIRST_ONE_ONLY");
  Am_Register_Slot_Key(Am_WHERE_ATTACH, "WHERE_ATTACH");
  Am_Register_Slot_Key(Am_MINIMUM_WIDTH, "MINIMUM_WIDTH");
  Am_Register_Slot_Key(Am_MINIMUM_HEIGHT, "MINIMUM_HEIGHT");
  Am_Register_Slot_Key(Am_MINIMUM_LENGTH, "MINIMUM_LENGTH");
  Am_Register_Slot_Key(Am_GROWING, "GROWING");
  Am_Register_Slot_Key(Am_AS_LINE, "AS_LINE");
  Am_Register_Slot_Key(Am_FEEDBACK_OBJECT, "FEEDBACK_OBJECT");
  Am_Register_Slot_Key(Am_SET_SELECTED, "SET_SELECTED");

  Am_Register_Slot_Key(Am_CREATE_NEW_OBJECT_METHOD, "CREATE_NEW_OBJECT_METHOD");
  Am_Register_Slot_Key(Am_HOW_MANY_POINTS, "HOW_MANY_POINTS");
  Am_Register_Slot_Key(Am_FLIP_IF_CHANGE_SIDES, "FLIP_IF_CHANGE_SIDES");
  Am_Register_Slot_Key(Am_ABORT_IF_TOO_SMALL, "ABORT_IF_TOO_SMALL");
  Am_Register_Slot_Key(Am_EDIT_TRANSLATION_TABLE, "EDIT_TRANSLATION_TABLE");
  Am_Register_Slot_Key(Am_TEXT_EDIT_METHOD, "TEXT_EDIT_METHOD");
  Am_Register_Slot_Key(Am_PENDING_DELETE, "PENDING_DELETE");
  Am_Register_Slot_Key(Am_WANT_PENDING_DELETE, "WANT_PENDING_DELETE");
  Am_Register_Slot_Key(Am_TEXT_CHECK_LEGAL_METHOD, "TEXT_CHECK_LEGAL_METHOD");

  Am_Register_Slot_Key(Am_CURRENT_STATE, "~CURRENT_STATE~");
  Am_Register_Slot_Key(Am_MULTI_OWNERS, "MULTI_OWNERS");
  Am_Register_Slot_Key(Am_MULTI_FEEDBACK_OWNERS, "MULTI_FEEDBACK_OWNERS");
  Am_Register_Slot_Key(Am_ALL_WINDOWS, "ALL_WINDOWS");
  Am_Register_Slot_Key(Am_CURRENT_OBJECT, "CURRENT_OBJECT");
  Am_Register_Slot_Key(Am_START_OBJECT, "START_OBJECT");
  Am_Register_Slot_Key(Am_START_CHAR, "START_CHAR");
  Am_Register_Slot_Key(Am_RUN_ALSO, "RUN_ALSO");
  Am_Register_Slot_Key(Am_INTER_LIST, "~INTER_LIST~");
  Am_Register_Slot_Key(Am_WHERE_HIT_WHERE_ATTACH, "~WHERE_HIT_WHERE_ATTACH~");
  Am_Register_Slot_Key(Am_FIRST_X, "FIRST_X");
  Am_Register_Slot_Key(Am_FIRST_Y, "FIRST_Y");
  Am_Register_Slot_Key(Am_INTERIM_X, "INTERIM_X");
  Am_Register_Slot_Key(Am_INTERIM_Y, "INTERIM_Y");

  Am_Register_Slot_Key(Am_DEPENDS_ON_PRIORITY, "~DEPENDS_ON_PRIORITY~");
  Am_Register_Slot_Key(Am_DEPENDS_ON_RANK_DEPTH, "~DEPENDS_ON_RANK_DEPTH~");
  Am_Register_Slot_Key(Am_DEPENDS_ON_MULTI_OWNERS, "~DEPENDS_ON_MULTI_OWNERS~");
  Am_Register_Slot_Key(Am_LAST_WINDOW, "~LAST_WINDOW~");
  Am_Register_Slot_Key(Am_LAST_WINDOWS, "~LAST_WINDOWS~");
  Am_Register_Slot_Key(Am_WINDOW_WANT_MOVE_CNT, "~WINDOW_WANT_MOVE_CNT~");
  Am_Register_Slot_Key(Am_REAL_RUN_ALSO, "~REAL_RUN_ALSO~");
  Am_Register_Slot_Key(Am_SET_COMMAND_OLD_OWNER, "~SET_COMMAND_OLD_OWNER~");
  Am_Register_Slot_Key(Am_INITIAL_REF_OBJECT, "~INITIAL_REF_OBJECT~");

  Am_Register_Slot_Key(Am_UNDO_METHOD, "UNDO_METHOD");
  Am_Register_Slot_Key(Am_REDO_METHOD, "REDO_METHOD");
  Am_Register_Slot_Key(Am_SELECTIVE_UNDO_METHOD, "SELECTIVE_UNDO_METHOD");
  Am_Register_Slot_Key(Am_SELECTIVE_REPEAT_SAME_METHOD,
                       "SELECTIVE_REPEAT_SAME_METHOD");
  Am_Register_Slot_Key(Am_SELECTIVE_REPEAT_ON_NEW_METHOD,
                       "SELECTIVE_REPEAT_ON_NEW_METHOD");
  Am_Register_Slot_Key(Am_SELECTIVE_UNDO_ALLOWED, "SELECTIVE_UNDO_ALLOWED");
  Am_Register_Slot_Key(Am_SELECTIVE_REPEAT_SAME_ALLOWED,
                       "SELECTIVE_REPEAT_SAME_ALLOWED");
  Am_Register_Slot_Key(Am_SELECTIVE_REPEAT_NEW_ALLOWED,
                       "SELECTIVE_REPEAT_NEW_ALLOWED");

  Am_Register_Slot_Key(Am_SHORT_HELP, "SHORT_HELP");
  Am_Register_Slot_Key(Am_LONG_HELP, "LONG_HELP");
  Am_Register_Slot_Key(Am_LABEL, "LABEL");
  Am_Register_Slot_Key(Am_SHORT_LABEL, "SHORT_LABEL");
  Am_Register_Slot_Key(Am_ACCELERATOR, "ACCELERATOR");
  Am_Register_Slot_Key(Am_ID, "ID");

  Am_Register_Slot_Key(Am_IMPLEMENTATION_PARENT, "IMPLEMENTATION_PARENT");
  Am_Register_Slot_Key(Am_IMPLEMENTATION_CHILD, "~IMPLEMENTATION_CHILD~");
  Am_Register_Slot_Key(Am_COMPOSITE_PARENT, "COMPOSITE_PARENT");

  Am_Register_Slot_Key(Am_CHECK_INACTIVE_COMMANDS, "CHECK_INACTIVE_COMMANDS");
  Am_Register_Slot_Key(Am_INACTIVE_COMMANDS, "INACTIVE_COMMANDS");
  Am_Register_Slot_Key(Am_SLOT_FOR_THIS_COMMAND_INACTIVE,
                       "SLOT_FOR_THIS_COMMAND_INACTIVE");
  Am_Register_Slot_Key(Am_MOVE_INACTIVE, "MOVE_INACTIVE");
  Am_Register_Slot_Key(Am_GROW_INACTIVE, "GROW_INACTIVE");
  Am_Register_Slot_Key(Am_SELECT_INACTIVE, "SELECT_INACTIVE");
  Am_Register_Slot_Key(Am_TEXT_EDIT_INACTIVE, "TEXT_EDIT_INACTIVE");
  Am_Register_Slot_Key(Am_SET_PROPERTY_INACTIVE, "SET_PROPERTY_INACTIVE");
  Am_Register_Slot_Key(Am_CLEAR_INACTIVE, "CLEAR_INACTIVE");
  Am_Register_Slot_Key(Am_COPY_INACTIVE, "COPY_INACTIVE");
  Am_Register_Slot_Key(Am_CUT_INACTIVE, "CUT_INACTIVE");
  Am_Register_Slot_Key(Am_DUPLICATE_INACTIVE, "DUPLICATE_INACTIVE");
  Am_Register_Slot_Key(Am_TO_TOP_INACTIVE, "TO_TOP_INACTIVE");
  Am_Register_Slot_Key(Am_TO_BOTTOM_INACTIVE, "TO_BOTTOM_INACTIVE");
  Am_Register_Slot_Key(Am_GROUP_INACTIVE, "GROUP_INACTIVE");
  Am_Register_Slot_Key(Am_UNGROUP_INACTIVE, "UNGROUP_INACTIVE");

  Am_Register_Slot_Key(Am_COMPOSITE_CHILDREN, "~COMPOSITE_CHILDREN~");
  Am_Register_Slot_Key(Am_COMMAND_IS_ABORTING, "~COMMAND_IS_ABORTING~");

  Am_Register_Slot_Key(Am_VALUE, "VALUE");
  Am_Register_Slot_Key(Am_INTERIM_VALUE, "INTERIM_VALUE");
  Am_Register_Slot_Key(Am_OLD_INTERIM_VALUE, "OLD_INTERIM_VALUE");
  Am_Register_Slot_Key(Am_OLD_VALUE, "OLD_VALUE");
  Am_Register_Slot_Key(Am_OBJECT_MODIFIED, "OBJECT_MODIFIED");
  Am_Register_Slot_Key(Am_TOO_SMALL, "TOO_SMALL");
  Am_Register_Slot_Key(Am_HAS_BEEN_UNDONE, "HAS_BEEN_UNDONE");
  Am_Register_Slot_Key(Am_SAVED_OLD_OWNER, "SAVED_OLD_OWNER");
  Am_Register_Slot_Key(Am_SAVED_OLD_OBJECT_OWNER, "SAVED_OLD_OBJECT_OWNER");

  Am_Register_Slot_Key(Am_SELECTED, "SELECTED");
  Am_Register_Slot_Key(Am_INTERIM_SELECTED, "INTERIM_SELECTED");

  Am_Register_Slot_Key(Am_UNDO_HANDLER, "UNDO_HANDLER");
  Am_Register_Slot_Key(Am_REGISTER_COMMAND, "REGISTER_COMMAND");
  Am_Register_Slot_Key(Am_PERFORM_UNDO, "PERFORM_UNDO");
  Am_Register_Slot_Key(Am_PERFORM_REDO, "PERFORM_REDO");
  Am_Register_Slot_Key(Am_UNDO_ALLOWED, "UNDO_ALLOWED");
  Am_Register_Slot_Key(Am_REDO_ALLOWED, "REDO_ALLOWED");
  Am_Register_Slot_Key(Am_LAST_UNDONE_COMMAND, "LAST_UNDONE_COMMAND");

  Am_Register_Slot_Key(Am_USER_ID, "USER_ID"); // supports multiple users
  Am_Register_Slot_Key(Am_INTERNAL_USER_ID,
                       "~INTERNAL_USER_ID"); // supports multiple users
  Am_Register_Slot_Key(Am_LAST_USER_ID,
                       "LAST_USER_ID"); // supports multiple users
#endif
}

////////////////////////////////////////////////////////////
// Default actions for all interactors
////////////////////////////////////////////////////////////

Am_Define_Method(Am_Inter_Internal_Method, void, Am_Inter_Start_Method,
                 (Am_Object & inter, Am_Object &object,
                  Am_Object & /* event_window */, Am_Input_Event *ev))
{
  Am_INTER_TRACE_PRINT(inter, "Interactor starting over " << object);

  // now set up states.  It is up to the sub-classes to call the
  // command or object function appropriately

  if ((bool)inter.Get(Am_CONTINUOUS)) {
    if ((int)inter.Get(Am_CURRENT_STATE) != (int)Am_INTER_RUNNING) {
      inter.Set(Am_CURRENT_STATE, (int)Am_INTER_RUNNING, Am_OK_IF_NOT_THERE);
      inter.Set(Am_INTERNAL_USER_ID, ev->user_id,
                Am_OK_IF_NOT_THERE); // supports multiple users
      change_priority_level(inter, false, true);
      set_want_move(inter, true);
    }
    //otherwise, it is already running
  } else { // not continuous
    inter.Set(Am_CURRENT_STATE, (int)Am_INTER_WAITING, Am_OK_IF_NOT_THERE);
  }
}

Am_Define_Method(Am_Inter_Internal_Method, void, Am_Inter_Abort_Method,
                 (Am_Object & inter, Am_Object & /* object */,
                  Am_Object & /* event_window */, Am_Input_Event * /* ev */))
{
  Am_INTER_TRACE_PRINT(inter, "Interactor aborting");

  Am_Inter_State state = Am_Get_Inter_State(inter);
  if (state == Am_INTER_WAITING || state == Am_INTER_ABORTING)
    // already aborted, don't do anything
    ;
  else {
    // set the interactor state
    inter.Set(Am_CURRENT_STATE, (int)Am_INTER_ABORTING, Am_OK_IF_NOT_THERE);
    change_priority_level(inter, true, false);
    set_want_move(inter, false);
  }

  // real work done by specialization methods

  // abort sets user id back to zero -- supports multiple users
  inter.Set(Am_INTERNAL_USER_ID, Am_ONE_AT_A_TIME,
            Am_OK_IF_NOT_THERE); // supports multiple users
}

Am_Define_Method(Am_Inter_Internal_Method, void, Am_Inter_Outside_Method,
                 (Am_Object & inter, Am_Object & /* object */,
                  Am_Object & /* event_window */, Am_Input_Event * /* ev */))
{
  Am_INTER_TRACE_PRINT(inter, "Interactor outside");
  inter.Set(Am_CURRENT_STATE, (int)Am_INTER_OUTSIDE, Am_OK_IF_NOT_THERE);
  // real work done by specialization methods
}

Am_Define_Method(Am_Inter_Internal_Method, void, Am_Inter_Outside_Stop_Method,
                 (Am_Object & inter, Am_Object &object, Am_Object &event_window,
                  Am_Input_Event *ev))
{
  Am_INTER_TRACE_PRINT(inter, "Interactor outside STOP");
  Am_Inter_Internal_Method method;
  method = inter.Get(Am_INTER_ABORT_METHOD);
  method.Call(inter, object, event_window, ev);
}

Am_Define_Method(Am_Inter_Internal_Method, void, Am_Inter_Back_Inside_Method,
                 (Am_Object & inter, Am_Object &object,
                  Am_Object & /* event_window */, Am_Input_Event * /* ev */))
{
  Am_INTER_TRACE_PRINT(inter, "Interactor back inside over " << object);
  inter.Set(Am_CURRENT_STATE, (int)Am_INTER_RUNNING, Am_OK_IF_NOT_THERE);
  // real work done by specialization methods
}

Am_Define_Method(Am_Inter_Internal_Method, void, Am_Inter_Running_Method,
                 (Am_Object & inter, Am_Object &object,
                  Am_Object & /* event_window */, Am_Input_Event * /* ev */))
{
  Am_INTER_TRACE_PRINT(inter, "Interactor running over " << object);
  // real work done by specialization methods
}

Am_Define_Method(Am_Inter_Internal_Method, void, Am_Inter_Stop_Method,
                 (Am_Object & inter, Am_Object &object,
                  Am_Object & /* event_window */, Am_Input_Event * /* ev */))
{
  //inter may not be valid if part of window that is being destroyed by me
  if (inter.Valid()) {
    Am_Inter_State state = Am_Get_Inter_State(inter);
    if (state == Am_INTER_RUNNING || state == Am_INTER_OUTSIDE) {
      //otherwise, already aborted
      Am_INTER_TRACE_PRINT(inter, "Interactor stop over " << object);
      inter.Set(Am_CURRENT_STATE, (int)Am_INTER_WAITING, Am_OK_IF_NOT_THERE);
      change_priority_level(inter, true, false);
      set_want_move(inter, false);
      // real work done by specialization methods

      // clear multiple support settings
      inter.Set(Am_INTERNAL_USER_ID, Am_ONE_AT_A_TIME,
                Am_OK_IF_NOT_THERE); // supports multiple users
    }
  }
}

///////////////////////////////////////////////////////////////////////////
// Pop Up Windows
///////////////////////////////////////////////////////////////////////////
// These are in interactors instead of in opal because usually want
// them to be modal and that is a property of interactors

void
Am_Remove_Modal_Window(Am_Object &window)
{
  Am_Modal_Windows.Start();
  if (Am_Modal_Windows.Member(window))
    Am_Modal_Windows.Delete();
}
void
Am_Push_Modal_Window(Am_Object &window)
{
  Am_Modal_Windows.Add(window, Am_HEAD);
}

void
Am_Pop_Up_Window_And_Wait(Am_Object window, Am_Value &return_value, bool modal)
{
  Am_Value v;
  v = window.Peek(Am_WAITING_FOR_COMPLETION);
  if (v.Valid()) {
    std::cerr << "** Amulet Error: Window " << window
              << " already popped up but called Am_Pop_Up_Window_And_Wait on "
                 "it again\n";
    Am_Error(window, Am_WAITING_FOR_COMPLETION);
  }

#ifdef DEBUG
  if (Am_Inter_Tracing(Am_INTER_TRACE_SHORT)) {
    std::cout << "<>popup on " << window << " Modal " << modal << std::endl
              << std::flush;
  }
#endif

  window.Set(Am_VISIBLE, true);
  if (!window.Get_Owner().Valid()) // common bug to forget to add window
                                   // to screen, so just do it
    Am_Screen.Add_Part(window);
  else
    Am_To_Top(window);
  window.Set(Am_COMPLETION_VALUE, (0L), Am_OK_IF_NOT_THERE);
  if (modal) {
    window.Set(Am_WAITING_FOR_COMPLETION, Am_INTER_WAITING_MODAL,
               Am_OK_IF_NOT_THERE);
    Am_Push_Modal_Window(window);
  } else
    window.Set(Am_WAITING_FOR_COMPLETION, Am_INTER_WAITING_NOT_MODAL,
               Am_OK_IF_NOT_THERE);
  bool continue_looping = true;
  while (continue_looping) {
    continue_looping = Am_Do_Events(true);
    //window might be destroyed while looping
    if (continue_looping && window.Valid())
      continue_looping = window.Get(Am_WAITING_FOR_COMPLETION).Valid();
  }
  if (window.Valid())
    return_value = window.Peek(Am_COMPLETION_VALUE);
  else
    return_value = Am_No_Value;
}

void
Am_Finish_Pop_Up_Waiting(Am_Object window, Am_Value return_value)
{
#ifdef DEBUG
  Am_Value v;
  v = window.Peek(Am_WAITING_FOR_COMPLETION);
  if (!v.Valid()) {
    std::cerr << "**AMULET Warning: Called Am_Finish_Pop_Up_Waiting on "
              << window
              << " without prior Am_Pop_Up_Window_And_Wait; continuing.\n"
              << std::flush;
  }
#endif
  Am_INTER_TRACE_PRINT(window, "<>Am_Finish_Pop_Up_Waiting on " << window);

  Am_Remove_Modal_Window(window);
  window.Set(Am_WAITING_FOR_COMPLETION, Am_INTER_NOT_WAITING,
             Am_OK_IF_NOT_THERE);
  window.Set(Am_COMPLETION_VALUE, return_value, Am_OK_IF_NOT_THERE);
  window.Set(Am_VISIBLE, false);
}

///////////////////////////////////////////////////////////////////////////
// Where methods
///////////////////////////////////////////////////////////////////////////

// Exported functions for the _Where functions. These are of type
// Am_Where_Function
Am_Define_Method(Am_Where_Method, Am_Object, Am_Inter_In,
                 // point in object?
                 (Am_Object inter, Am_Object object, Am_Object event_window,
                  int x, int y))
{
  Am_INTER_TRACE_PRINT(inter, " --testing in " << object);
  if (Am_Point_In_All_Owners(object, x, y, event_window))
    return Am_Point_In_Obj(object, x, y, event_window);
  else
    return Am_No_Object;
}

Am_Define_Method(Am_Where_Method, Am_Object, Am_Inter_In_Part,
                 // point in object?
                 (Am_Object inter, Am_Object object, Am_Object event_window,
                  int x, int y))
{
  Am_INTER_TRACE_PRINT(inter, " --testing part of " << object);
  if (Am_Point_In_All_Owners(object, x, y, event_window))
    return Am_Point_In_Part(object, x, y, event_window);
  else
    return Am_No_Object;
}

inline bool
group_like(Am_Object object)
{
  return (Am_Is_Group_Or_Map(object) || object.Is_Instance_Of(Am_Window) ||
          object.Is_Instance_Of(Am_Screen));
}

//if attached to a group, then point in part?  else point in object?
Am_Define_Method(Am_Where_Method, Am_Object, Am_Inter_In_Object_Or_Part,
                 (Am_Object inter, Am_Object object, Am_Object event_window,
                  int x, int y))
{
  if (group_like(object)) {
    Am_INTER_TRACE_PRINT(inter, " --attached to group, so testing part of "
                                    << object);
    if (Am_Point_In_All_Owners(object, x, y, event_window))
      return Am_Point_In_Part(object, x, y, event_window);
  } else {
    Am_INTER_TRACE_PRINT(inter, " --attached to non-group, so testing in "
                                    << object);
    if (Am_Point_In_All_Owners(object, x, y, event_window))
      return Am_Point_In_Obj(object, x, y, event_window);
  }
  return Am_No_Object;
}

Am_Define_Method(Am_Where_Method, Am_Object, Am_Inter_In_Leaf,
                 // point in a leaf part?
                 (Am_Object inter, Am_Object object, Am_Object event_window,
                  int x, int y))
{
  Am_INTER_TRACE_PRINT(inter, " --testing leaf of " << object);
  if (Am_Point_In_All_Owners(object, x, y, event_window))
    return Am_Point_In_Leaf(object, x, y, event_window);
  else
    return Am_No_Object;
}

Am_Define_Method(Am_Where_Method, Am_Object, Am_Inter_In_Text,
                 // point in text object?
                 (Am_Object inter, Am_Object object, Am_Object event_window,
                  int x, int y))
{
  Am_INTER_TRACE_PRINT(inter, " --testing in text " << object);
  if (Am_Point_In_All_Owners(object, x, y, event_window) &&
      object.Is_Instance_Of(Am_Text))
    return Am_Point_In_Obj(object, x, y, event_window);
  else
    return Am_No_Object;
}

Am_Define_Method(Am_Where_Method, Am_Object, Am_Inter_In_Text_Part,
                 // point in text part?
                 (Am_Object inter, Am_Object object, Am_Object event_window,
                  int x, int y))
{
  Am_INTER_TRACE_PRINT(inter, " --testing text part of " << object);
  if (Am_Point_In_All_Owners(object, x, y, event_window)) {
    Am_Object obj = Am_Point_In_Part(object, x, y, event_window);
    if (obj.Valid() && obj.Is_Instance_Of(Am_Text))
      return obj;
  }
  return Am_No_Object;
}

//if attached to a group, then point in part?  else point in object?
Am_Define_Method(Am_Where_Method, Am_Object, Am_Inter_In_Text_Object_Or_Part,
                 (Am_Object inter, Am_Object object, Am_Object event_window,
                  int x, int y))
{
  if (group_like(object)) {
    Am_INTER_TRACE_PRINT(inter, " --attached to group, so testing text part of "
                                    << object);
    if (Am_Point_In_All_Owners(object, x, y, event_window)) {
      Am_Object obj = Am_Point_In_Part(object, x, y, event_window);
      if (obj.Valid() && obj.Is_Instance_Of(Am_Text))
        return obj;
    }
  } else {
    Am_INTER_TRACE_PRINT(inter, " --attached to non-group, so testing in text "
                                    << object);
    if (Am_Point_In_All_Owners(object, x, y, event_window) &&
        object.Is_Instance_Of(Am_Text))
      return Am_Point_In_Obj(object, x, y, event_window);
  }
  return Am_No_Object;
}

Am_Define_Method(Am_Where_Method, Am_Object, Am_Inter_In_Text_Leaf,
                 // point in text part?
                 (Am_Object inter, Am_Object object, Am_Object event_window,
                  int x, int y))
{
  Am_INTER_TRACE_PRINT(inter, " --testing text leaf of " << object);
  if (Am_Point_In_All_Owners(object, x, y, event_window)) {
    Am_Object obj = Am_Point_In_Leaf(object, x, y, event_window);
    if (obj.Valid() && obj.Is_Instance_Of(Am_Text))
      return obj;
  }
  return Am_No_Object;
}

///////////////////////////////////////////////////////////////////////////
// Initialization
///////////////////////////////////////////////////////////////////////////

// call this first to initialize all the interactor objects.

static void
init()
{
  Interactor_Input_Func = Interactor_Input_Event_Notify;

  inter_set_slot_names(); // set the slot names, for debugging

  all_wins_inter_list = Am_Priority_List::Create(); //global list

  Am_Default_Start_Char = Am_Input_Char(Am_LEFT_MOUSE, false, false, false,
                                        Am_BUTTON_DOWN, Am_ANY_CLICK, false);
  Am_Default_Stop_Char = Am_Input_Char("ANY_MOUSE_UP");

  Am_Interactor =
      Am_Root_Object
          .Create(DSTR("Am_Interactor"))
          //default start event is left down with no modifiers but any
          //number of clicks (no current way to specify this with a string)
          .Add(Am_START_WHEN, Am_Default_Start_Char)
          .Add(Am_START_WHERE_TEST, Am_Inter_In_Object_Or_Part)
          .Add(Am_INTER_START_METHOD, Am_Inter_Start_Method)
          .Add(Am_ABORT_WHEN, Am_Input_Char("CONTROL_g"))
          .Add(Am_RUNNING_WHERE_TEST, true)
          .Add(Am_INTER_RUNNING_METHOD, Am_Inter_Running_Method)
          .Add(Am_INTER_ABORT_METHOD, Am_Inter_Abort_Method)
          .Add(Am_STOP_WHEN, Am_Default_Stop_Char)
          .Add(Am_INTER_STOP_METHOD, Am_Inter_Stop_Method)
          .Add(Am_INTER_OUTSIDE_METHOD, Am_Inter_Outside_Method)
          .Add(Am_INTER_OUTSIDE_STOP_METHOD, Am_Inter_Outside_Stop_Method)
          .Add(Am_INTER_BACK_INSIDE_METHOD, Am_Inter_Back_Inside_Method)

          .Add(Am_START_DO_METHOD, (0L))
          .Add(Am_INTERIM_DO_METHOD, (0L))
          .Add(Am_ABORT_DO_METHOD, (0L))
          .Add(Am_DO_METHOD, (0L))

          .Add(Am_CONTINUOUS, true)
          .Add(Am_RUN_ALSO, false)
          .Add(Am_ACTIVE, true)
          .Add(Am_PRIORITY, 1.0) // starts off waiting
          .Add(Am_CURRENT_STATE, (int)Am_INTER_WAITING)
          .Add(Am_OWNER_DEPTH, -1)
          .Add(Am_LAST_WINDOW, 0)
          .Add(Am_MULTI_OWNERS, 0)
          .Add(Am_MULTI_FEEDBACK_OWNERS, 0)
          .Add(Am_ALL_WINDOWS, false)
          .Add(Am_LAST_WINDOWS, 0)
          .Add(Am_INTER_LIST, 0)
          .Add(Am_DEPENDS_ON_PRIORITY, maintain_inter_to_window_list_priority)
          .Add(Am_DEPENDS_ON_RANK_DEPTH, maintain_inter_to_window_list_rank)
          .Add(Am_DEPENDS_ON_MULTI_OWNERS, maintain_inter_for_multi_owners)
          .Add_Part(Am_COMMAND,
                    Am_Command.Create(DSTR("Am_Command_in_Interactor")))
          .Add(Am_START_OBJECT,
               0)                // set while running but might be dependencies
          .Add(Am_START_CHAR, 0) //   so create so will exist
          .Add(Am_FIRST_X, 0)
          .Add(Am_FIRST_Y, 0)
          .Add(Am_INTERIM_X, 0)
          .Add(Am_INTERIM_Y, 0)
          .Add(Am_WINDOW, 0)
          .Add(Am_CURRENT_OBJECT, 0)
          .Add(Am_VALUE, 0)
          .Add(Am_OLD_VALUE, 0)
          .Add(Am_INTERIM_VALUE, 0)
          .Add(Am_OLD_INTERIM_VALUE, 0)
          .Add(Am_OBJECT_MODIFIED, 0)
          .Add(Am_SET_COMMAND_OLD_OWNER, Am_Set_Old_Owner_To_Me)
          .Add(Am_INTER_BEEP_ON_ABORT, true)
          .Add(Am_CHECK_INACTIVE_COMMANDS, true)
          .Add(Am_USER_ID, Am_ONE_AT_A_TIME)          // supports multiple users
          .Add(Am_INTERNAL_USER_ID, Am_ONE_AT_A_TIME) // supports multiple users
      ;

  Am_Interactor.Set_Inherit_Rule(Am_LAST_WINDOW, Am_LOCAL)
      .Set_Inherit_Rule(Am_LAST_WINDOWS, Am_LOCAL)
      .Set_Inherit_Rule(Am_CURRENT_STATE, Am_LOCAL)
      .Set_Inherit_Rule(Am_OLD_VALUE, Am_LOCAL)
      .Set_Inherit_Rule(Am_INTERNAL_USER_ID,
                        Am_LOCAL) // supports multiple users
      ;
}

static void
cleanup()
{
  Am_Interactor.Destroy();
  Am_Command.Destroy();
  Am_Undo_Handler.Destroy();
}

Am_Initializer *inter_basics_init =
    new Am_Initializer(DSTR("Inter_Basics"), init, 3.2f, 110, cleanup);
