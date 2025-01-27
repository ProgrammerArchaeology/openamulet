/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file contains the functions for handling the animation Interactor

   Designed and implemented by Brad Myers

   Note: the Animation Interactor is NOT undoable, so this is a lot
   simpler than the other Interactors.
*/

#include <am_inc.h>
#include <math.h> // for copysign() in constant_velocity_do

#include <amulet/am_io.h>

#include <amulet/inter_advanced.h>
#include <amulet/standard_slots.h>
#include <amulet/value_list.h>
#include <amulet/opal_advanced.h>

#include <amulet/types.h>
#include <amulet/opal.h>
#include <amulet/text_fns.h>
#include <amulet/registry.h>
#include <amulet/anim.h>
#include <amulet/initializer.h>

AM_DEFINE_ENUM_SUPPORT(
    Am_Animation_End_Action,
    "Am_ANIMATION_STOP Am_ANIMATION_WRAP Am_ANIMATION_BOUNCE");

//////////////////////////////////////////////////////////////////
/// Main method routines
//////////////////////////////////////////////////////////////////

Am_Define_Method(Am_Inter_Internal_Method, void, Am_Animation_Start_Method,
                 (Am_Object & inter, Am_Object &object, Am_Object &event_window,
                  Am_Input_Event *ev))
{
  Am_INTER_TRACE_PRINT(inter, "Animation starting over " << object);

  // first, call the prototype's method
  Am_Inter_Internal_Method inter_method;
  inter_method = Am_Interactor.Get(Am_INTER_START_METHOD);
  inter_method.Call(inter, object, event_window, ev);

  Am_Object command_obj;
  command_obj = inter.Get(Am_COMMAND);
  Am_Inter_Call_Both_Method(inter, command_obj, Am_START_DO_METHOD, ev->x,
                            ev->y, event_window, ev->input_char, object,
                            Am_No_Location);
}

Am_Define_Method(Am_Inter_Internal_Method, void, Am_Animation_Outside_Method,
                 (Am_Object & inter, Am_Object & /* object */,
                  Am_Object &event_window, Am_Input_Event *ev))
{
  Am_INTER_TRACE_PRINT(inter, "Animation Outside");

  // ignore object parameter, used saved object
  Am_Object object;
  object = inter.Get(Am_START_OBJECT);

  // first, call the prototype's method
  Am_Inter_Internal_Method inter_method;
  inter_method = Am_Interactor.Get(Am_INTER_OUTSIDE_METHOD);
  inter_method.Call(inter, object, event_window, ev);

  // Now, call the abort Command method
  Am_Object command_obj;
  command_obj = inter.Get(Am_COMMAND);
  Am_Inter_Call_Both_Method(inter, command_obj, Am_ABORT_DO_METHOD, ev->x,
                            ev->y, event_window, ev->input_char, object,
                            Am_No_Location);
}

Am_Define_Method(Am_Inter_Internal_Method, void,
                 Am_Animation_Back_Inside_Method,
                 (Am_Object & inter, Am_Object & /* object */,
                  Am_Object &event_window, Am_Input_Event *ev))
{
  int x = ev->x;
  int y = ev->y;
  // ignore object parameter, used saved object
  Am_Object object, command_obj;
  object = inter.Get(Am_START_OBJECT);
  Am_INTER_TRACE_PRINT(inter, "Animation back inside over " << object);
  // first, call the prototype's method
  Am_Inter_Internal_Method inter_method;
  inter_method = Am_Interactor.Get(Am_INTER_BACK_INSIDE_METHOD);
  inter_method.Call(inter, object, event_window, ev);

  command_obj = inter.Get(Am_COMMAND);
  Am_Inter_Call_Both_Method(inter, command_obj, Am_START_DO_METHOD, x, y,
                            event_window, ev->input_char, object,
                            Am_No_Location);
}

Am_Define_Method(Am_Inter_Internal_Method, void, Am_Animation_Abort_Method,
                 (Am_Object & inter, Am_Object & /* object */,
                  Am_Object &event_window, Am_Input_Event *ev))
{
  Am_INTER_TRACE_PRINT(inter, "Animation Aborting");

  Am_Object command_obj, object;
  object = inter.Get(Am_START_OBJECT);
  // first, call the prototype's method
  Am_Inter_Internal_Method inter_method;
  inter_method = Am_Interactor.Get(Am_INTER_ABORT_METHOD);
  inter_method.Call(inter, object, event_window, ev);

  command_obj = inter.Get(Am_COMMAND);
  Am_Inter_Call_Both_Method(inter, command_obj, Am_ABORT_DO_METHOD, ev->x,
                            ev->y, event_window, ev->input_char, object,
                            Am_No_Location);
}

Am_Define_Method(Am_Inter_Internal_Method, void, Am_Animation_Stop_Method,
                 (Am_Object & inter, Am_Object &object, Am_Object &event_window,
                  Am_Input_Event *ev))
{
  Am_INTER_TRACE_PRINT(inter, "Animation stopping " << object);

  Am_Object command_obj, text;
  command_obj = inter.Get(Am_COMMAND);
  text = inter.Get(Am_START_OBJECT);
  // now call stop
  Am_Call_Final_Do_And_Register(inter, command_obj, ev->x, ev->y, event_window,
                                ev->input_char, text, Am_No_Location,
                                nullptr); //no impl_setter since no undo

  // LAST, call the prototype's method
  Am_Inter_Internal_Method inter_method;
  inter_method = Am_Interactor.Get(Am_INTER_STOP_METHOD);
  inter_method.Call(inter, object, event_window, ev);
}

//////////////////////////////////////////////////////////
// Animation interactor's command object's method functions. //
//////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////

Am_Define_Object_Formula(Am_Get_Inter_Object)
{
  //self is the command object
  return self.Get_Object(Am_SAVED_OLD_OWNER).Get(Am_START_OBJECT);
}

void
call_all_commands(Am_Value value, Am_Slot_Key method_slot, Am_Object &inter,
                  const Am_Time &elapsed_time, bool skip_first)
{
  Am_Object command_obj;
  if (skip_first) {
    if (value.Valid() && value.type == Am_OBJECT) {
      command_obj = value;
      value = command_obj.Peek(Am_IMPLEMENTATION_PARENT);
    }
  }
  while (value.Valid() && value.type == Am_OBJECT) {
    command_obj = value;
    if (!command_obj.Is_Instance_Of(Am_Command))
      break;
    value = command_obj.Peek(method_slot);
    if (Am_Object_Method::Test(value)) {
      command_obj.Set(Am_COMMAND_IS_ABORTING, false, Am_OK_IF_NOT_THERE);
      command_obj.Set(Am_SAVED_OLD_OWNER, inter, Am_OK_IF_NOT_THERE);
      if (elapsed_time.Valid())
        command_obj.Set(Am_ELAPSED_TIME, elapsed_time);
      Am_Object_Method do_method = value;
      Am_INTER_TRACE_PRINT(
          inter, "%%Animator " << inter << " executing " << method_slot
                               << " of " << command_obj << "=" << do_method
                               << " at elapsedtime " << elapsed_time);
      do_method.Call(command_obj);
      if ((bool)command_obj.Get(Am_COMMAND_IS_ABORTING))
        return; //break out of loop
    }
    value = command_obj.Peek(Am_IMPLEMENTATION_PARENT);
  }
}

//executes the command obj's do method, and all implementation parents
Am_Define_Method(Am_Timer_Method, void, animation_timer_method,
                 (Am_Object inter, const Am_Time &elapsed_time))
{
  Am_Value value;
  value = inter.Peek(Am_COMMAND);
  call_all_commands(value, Am_INTERIM_DO_METHOD, inter, elapsed_time, false);
}

Am_Define_Method(Am_Object_Method, void, Am_Animation_Start_Do,
                 (Am_Object inter))
{
  Am_Time delay = inter.Get(Am_REPEAT_DELAY);
  bool once = !(bool)inter.Get(Am_CONTINUOUS);
  Am_Register_Timer(delay, inter, Am_ANIMATION_METHOD, once);
}

//Am_Time Am_No_Time;  // Now part of Gem.

Am_Define_Method(Am_Object_Method, void, Am_Animation_Abort_Do,
                 (Am_Object inter))
{
  Am_Stop_Timer(inter, Am_ANIMATION_METHOD);
  Am_Value value;
  value = inter.Peek(Am_COMMAND);
  call_all_commands(value, Am_ABORT_DO_METHOD, inter, Am_No_Time, true);
}
Am_Define_Method(Am_Object_Method, void, Am_Animation_Stop_Do,
                 (Am_Object inter))
{
  Am_Stop_Timer(inter, Am_ANIMATION_METHOD);
  Am_Value value;
  value = inter.Peek(Am_COMMAND);
  call_all_commands(value, Am_DO_METHOD, inter, Am_No_Time, true);
}

//////////////////////////////////////////////////////////////////////////////
// Special types of Animator Command Objects
//////////////////////////////////////////////////////////////////////////////

///////////////  Am_Incrementer_Animation_Command;  ///////////////////
// Gets the object out of the Am_OPERATES_ON slot, and then gets the
// slot or list of slots out of the Am_SLOT_TO_ANIMATE slot, and then
// increments each of those slots by Am_SMALL_INCREMENT (if
// Am_SLOT_TO_ANIMATE is a list, Am_SMALL_INCREMENT must be a list of
// the same length).  Never stops by itself.

#ifdef DEBUG
void
inc_int_value(Am_Object &obj, Am_Slot_Key slot, int inc, Am_Object &command_obj)
{
#else
void
inc_int_value(Am_Object &obj, Am_Slot_Key slot, int inc,
              Am_Object & /*command_obj*/)
{
#endif
  int cur_val = obj.Get(slot);
  Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING,
                       "++ Animation command "
                           << command_obj << " incrementing slot " << slot
                           << " of " << obj << " by " << inc << " to be (int)"
                           << cur_val + inc);
  obj.Set(slot, cur_val + inc);
}

#ifdef DEBUG
void
inc_float_value(Am_Object &obj, Am_Slot_Key slot, float inc,
                Am_Object &command_obj)
{
#else
void
inc_float_value(Am_Object &obj, Am_Slot_Key slot, float inc,
                Am_Object & /* command_obj */)
{
#endif
  float cur_val = obj.Get(slot);
  Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING,
                       "++ Animation command "
                           << command_obj << " incrementing slot " << slot
                           << " of " << obj << " by " << inc << " to be (float)"
                           << cur_val + inc);
  obj.Set(slot, cur_val + inc);
}

void
inc_value_slot(Am_Slot_Key slot, Am_Value inc_value, Am_Object &obj,
               Am_Object &command_obj)
{
  if (inc_value.type == Am_INT)
    inc_int_value(obj, slot, (int)inc_value, command_obj);
  else if (inc_value.type == Am_FLOAT)
    inc_float_value(obj, slot, (float)inc_value, command_obj);
  else
    Am_ERROR("Am_SMALL_INCREMENT should be of type int or float, but it is "
             << inc_value << " in " << command_obj);
}

void
inc_value_list(Am_Value_List slot_list, Am_Value_List inc_list, Am_Object &obj,
               Am_Object &command_obj)
{
  if (slot_list.Length() != inc_list.Length())
    Am_ERROR("length of list of slots not equal to length of list of "
             " increment values in "
             << command_obj);

  Am_Slot_Key slot;
  Am_Value inc_value;
  for (slot_list.Start(), inc_list.Start(); !slot_list.Last();
       slot_list.Next(), inc_list.Next()) {
    slot = (int)slot_list.Get();
    inc_value = inc_list.Get();
    inc_value_slot(slot, inc_value, obj, command_obj);
  }
}

Am_Define_Method(Am_Object_Method, void, Am_Slot_Incrementer_Do,
                 (Am_Object command_obj))
{
  Am_Object obj = command_obj.Get(Am_OPERATES_ON);
  Am_INTER_TRACE_PRINT(command_obj, "Animation command " << command_obj
                                                         << " operating on "
                                                         << obj);
  if (obj.Valid()) {
    Am_Value slot_value, inc_value;
    slot_value = command_obj.Peek(Am_SLOT_TO_ANIMATE);
    inc_value = command_obj.Peek(Am_SMALL_INCREMENT);
    if (Am_Value_List::Test(slot_value))
      inc_value_list(slot_value, inc_value, obj, command_obj);
    else {
      if (Am_Value_List::Test(inc_value))
        Am_ERROR("Am_SMALL_INCREMENT can only be a list when "
                 "Am_SLOT_TO_ANIMATE is a list; for "
                 << command_obj);
      inc_value_slot((int)slot_value, inc_value, obj, command_obj);
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

///////////////  Am_Animation_Object_In_Owner_Command;  ///////////////////
//Gets the object out of the Am_OPERATES_ON slot.  Each time interval,
//increments the LEFT of that object by Am_X_OFFSET and the TOP by
//Am_Y_OFFSET.  If reach the boundaries of the owner of the object,
//then either stops, wraps or bounces, depending on Am_ANIMATE_END_ACTION.
//If wrap or bounce, then never stops by itself.

Am_Define_Method(Am_Object_Method, void, Am_Inc_Object_In_Owner_Do,
                 (Am_Object command_obj))
{
  Am_Object obj = command_obj.Get(Am_OPERATES_ON);
  Am_INTER_TRACE_PRINT(command_obj, "Animation command " << command_obj
                                                         << " operating on "
                                                         << obj);
  if (obj.Valid()) {
    Am_Object owner = obj.Get_Owner();
    if (owner.Valid()) {
      Am_Animation_End_Action end_act = command_obj.Get(Am_ANIMATE_END_ACTION);
      int x_inc = command_obj.Get(Am_X_OFFSET);
      int y_inc = command_obj.Get(Am_Y_OFFSET);
      int x = (int)obj.Get(Am_LEFT) + x_inc;
      int y = (int)obj.Get(Am_TOP) + y_inc;
      int width = obj.Get(Am_WIDTH);
      int height = obj.Get(Am_HEIGHT);
      int right = x + width;
      int bottom = y + height;
      int owner_width = owner.Get(Am_WIDTH);
      int owner_height = owner.Get(Am_HEIGHT);
      switch (end_act.value) {
      case Am_ANIMATION_STOP_val:
        if (x < 0 || right > owner_width || y < 0 || bottom > owner_height) {
          Am_Object inter = command_obj.Get(Am_SAVED_OLD_OWNER);
          Am_Stop_Interactor(inter);
          return;
        }
        break;
      case Am_ANIMATION_WRAP_val:
        if (x < -width)
          x = owner_width;
        else if (x > owner_width)
          x = -width;
        if (y < -height)
          y = owner_height;
        else if (y > owner_height)
          y = -height;
        break;
      case Am_ANIMATION_BOUNCE_val:
        if (x < 0) {
          if (x_inc < 0)
            command_obj.Set(Am_X_OFFSET, -x_inc);
          x += x_inc;
        } else if (right > owner_width) {
          if (x_inc > 0)
            command_obj.Set(Am_X_OFFSET, -x_inc);
          x += x_inc;
        }
        if (y < 0) {
          if (y_inc < 0)
            command_obj.Set(Am_Y_OFFSET, -y_inc);
          y += y_inc;
        } else if (bottom > owner_height) {
          if (y_inc > 0)
            command_obj.Set(Am_Y_OFFSET, -y_inc);
          y += y_inc;
        }
        break;
      default:
        Am_ERROR("Bad Am_ANIMATE_END_ACTION " << (int)end_act.value << " in "
                                              << command_obj)
      }
      Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING,
                           "++ Animation command "
                               << command_obj << " setting LEFT, TOP of " << obj
                               << " to be " << x << ", " << y);
      if (x_inc)
        obj.Set(Am_LEFT, x);
      if (y_inc)
        obj.Set(Am_TOP, y);
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

/////// Am_Animator_Animation_Command;
//Gets the object out of the Am_OPERATES_ON slot, and then gets the
// slot or list of slots out of the Am_SLOT_TO_ANIMATE slot.  Gets the
// start value or values out of the Am_VALUE_1 slot and the final
// value or values out of the Am_VALUE_2 slot.  (If one of these is a
// list, they all must be lists of the same length).  Animates the
// slot through the values to take Am_TIME_FOR_ANIMATION time.  When
// finished, uses Am_ANIMATE_END_ACTION to decide whether to stop,
// wrap (just restart) or bounce (swap Value1 and Value2 and then restart)

void
interp_value_slot(Am_Slot_Key slot, Am_Value value1, Am_Value value2,
                  Am_Time elapsed_time, Am_Time time_for_animation,
                  Am_Object &obj, Am_Object &command_obj)
{
  float return_val = Am_Clip_And_Map((float)elapsed_time.Milliseconds(), 0.0,
                                     (float)time_for_animation.Milliseconds(),
                                     (float)value1, (float)value2);
  Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING,
                       "++ Animation command "
                           << command_obj << " interpolating time "
                           << elapsed_time << " out of " << time_for_animation
                           << " to get " << return_val << " for slot " << slot
                           << " of " << obj);
  if (value1.type == Am_INT && value2.type == Am_INT)
    obj.Set(slot, (int)return_val);
  else
    obj.Set(slot, return_val);
}

void
interp_value_list(Am_Value_List slot_list, Am_Value_List value1_list,
                  Am_Value_List value2_list, Am_Time elapsed_time,
                  Am_Time time_for_animation, Am_Object &obj,
                  Am_Object &command_obj)
{
  if (slot_list.Length() != value1_list.Length() ||
      slot_list.Length() != value2_list.Length())
    Am_ERROR("length of list of slots not equal to length of list in "
             " Am_VALUE_1 or Am_VALUE_2 slots of "
             << command_obj);

  Am_Slot_Key slot;
  Am_Value value1, value2;
  for (slot_list.Start(), value1_list.Start(), value2_list.Start();
       !slot_list.Last();
       slot_list.Next(), value1_list.Next(), value2_list.Next()) {
    slot = (int)slot_list.Get();
    value1 = value1_list.Get();
    value2 = value2_list.Get();
    interp_value_slot(slot, value1, value2, elapsed_time, time_for_animation,
                      obj, command_obj);
  }
}

// af1x 4-15-96  I changed the direct set of Am_VALUE1 and Am_VALUE2
// to something which tracks the Am_ANIMATE_FORWARD slot.  That way, people
// can have constraints in Am_VALUE1 and Am_VALUE2, or can change these
// slots in bounce mode without ill effects.

Am_Define_Method(Am_Object_Method, void, Am_Slot_Animator_Do,
                 (Am_Object command_obj))
{
  Am_Object obj = command_obj.Get(Am_OPERATES_ON);
  Am_INTER_TRACE_PRINT(command_obj, "Animation command " << command_obj
                                                         << " operating on "
                                                         << obj);
  if (obj.Valid()) {
    Am_Value slot_value, value1, value2, temp;
    slot_value = command_obj.Peek(Am_SLOT_TO_ANIMATE);
    value1 = command_obj.Peek(Am_VALUE_1);
    value2 = command_obj.Peek(Am_VALUE_2);
    bool forward = command_obj.Get(Am_ANIMATE_FORWARD);
    if (!forward) {
      temp = value1;
      value1 = value2;
      value2 = temp;
    }
    Am_Time elapsed_time = command_obj.Get(Am_ELAPSED_TIME);
    Am_Time time_for_animation = command_obj.Get(Am_TIME_FOR_ANIMATION);

    if (Am_Value_List::Test(slot_value)) {
      if (!Am_Value_List::Test(value1))
        Am_ERROR("In " << command_obj << " Am_SLOT_TO_ANIMATE is a list "
                       << slot_value << " but Am_VALUE_1 is not:" << value1);
      if (!Am_Value_List::Test(value2))
        Am_ERROR("In " << command_obj << " Am_SLOT_TO_ANIMATE is a list "
                       << slot_value << " but Am_VALUE_2 is not:" << value2);
      interp_value_list(slot_value, value1, value2, elapsed_time,
                        time_for_animation, obj, command_obj);
    } else
      interp_value_slot((int)slot_value, value1, value2, elapsed_time,
                        time_for_animation, obj, command_obj);
    if (elapsed_time > time_for_animation) {
      Am_Animation_End_Action end_act = command_obj.Get(Am_ANIMATE_END_ACTION);
      Am_Object inter = command_obj.Get(Am_SAVED_OLD_OWNER);
      switch (end_act.value) {
      case Am_ANIMATION_STOP_val:
        Am_Stop_Interactor(inter);
        return;
      case Am_ANIMATION_WRAP_val:
        Am_Reset_Timer_Start(inter, Am_ANIMATION_METHOD);
        break;
      case Am_ANIMATION_BOUNCE_val:
        Am_Reset_Timer_Start(inter, Am_ANIMATION_METHOD);
        //	command_obj.Set(Am_VALUE_1, value2);
        //	command_obj.Set(Am_VALUE_2, value1);
        command_obj.Set(Am_ANIMATE_FORWARD, !forward);
        break;
      default:
        Am_ERROR("Bad Am_ANIMATE_END_ACTION " << (int)end_act.value << " in "
                                              << command_obj)
      }
    }
  }
}

Am_Define_Method(Am_Object_Method, void, Am_Constant_Velocity_Do,
                 (Am_Object command_obj))
{
  Am_Object obj = command_obj.Get(Am_OPERATES_ON);
  Am_INTER_TRACE_PRINT(command_obj, "Constant Velocity command "
                                        << command_obj << " operating on "
                                        << obj);
  if (obj.Valid()) {
    Am_Value current_value, value1, value2, velocity;
    Am_Slot_Key slot_key = (int)command_obj.Get(Am_SLOT_TO_ANIMATE);
    value1 = command_obj.Peek(Am_VALUE_1);
    value2 = command_obj.Peek(Am_VALUE_2);
    velocity = command_obj.Peek(Am_VELOCITY);
    current_value = command_obj.Peek(Am_VALUE);
    if (current_value.type != Am_FLOAT)
      current_value = obj.Peek(slot_key);
    bool forward = command_obj.Get(Am_ANIMATE_FORWARD);
    bool end = false; // set to true if we're at the end of animation

    float delta_time =
        (float)(((Am_Time)(command_obj.Get_Owner().Get(Am_REPEAT_DELAY)))
                    .Milliseconds());
    float delta = static_cast<float>((delta_time * (float)velocity) / 1000.0);
    float new_value;
    float fvalue1 = (float)value1;
    float fvalue2 = (float)value2;
    delta = fvalue2 > fvalue1 ? static_cast<float>(fabs(delta))
                              : static_cast<float>(-fabs(delta));
    // copysign (delta, (fvalue2 - fvalue1));
    if (forward) {
      new_value = (float)current_value + delta;
      if ((delta > 0.0 && new_value > fvalue2) ||
          (delta < 0.0 && new_value < fvalue2)) {
        new_value = fvalue2;
        end = true;
      }
    } else {
      new_value = (float)current_value - delta;
      if ((delta > 0.0 && new_value < fvalue1) ||
          (delta < 0.0 && new_value > fvalue1)) {
        new_value = fvalue1;
        end = true;
      }
    }
    if (end) {
      Am_Animation_End_Action end_act =
          (Am_Animation_End_Action)command_obj.Get(Am_ANIMATE_END_ACTION);
      Am_Object inter = command_obj.Get(Am_SAVED_OLD_OWNER);
      switch (end_act.value) {
      case Am_ANIMATION_STOP_val:
        Am_Stop_Interactor(inter);
        return;
      case Am_ANIMATION_WRAP_val:
        if (forward)
          new_value = value1;
        else
          new_value = value2;
        break;
      case Am_ANIMATION_BOUNCE_val:
        command_obj.Set(Am_ANIMATE_FORWARD, !forward);
        break;
      default:
        Am_ERROR("Bad Am_ANIMATE_END_ACTION " << (int)end_act.value << " in "
                                              << command_obj)
      }
      Am_Object_Method end_method = command_obj.Get(Am_ANIMATE_END_METHOD);
      if (end_method.Valid())
        end_method.Call(command_obj);
    }
    if (value1.type == Am_INT && value2.type == Am_INT)
      obj.Set(slot_key, (int)new_value);
    else
      obj.Set(slot_key, new_value);
    command_obj.Set(Am_VALUE, new_value); // always a float: handle slow rates
  }
}

//////////////////////////////////////////////////////////////////////////////

/////// Am_Animation_Through_List_Command;
//Gets the object out of the Am_OPERATES_ON slot, and then gets the
// slot (no lists supported) out of the Am_SLOT_TO_ANIMATE slot.  Gets the
// list of values for the slot out of the
// Am_VALUES_FOR_SLOT_TO_ANIMATE slot and the (optional) indices into
// that list from the Am_INDICES_FOR_VALUES_FOR_SLOT_TO_ANIMATE slot.
// Goes through the list of values, and when get to end, either stops,
// bounces (reverses direction) or wraps depending on
// Am_ANIMATE_END_ACTION.  The slot Am_CURRENT_DIRECTION holds either
// +1 or -1 to tell the current direction, and the slot
// Am_CURRENT_STATE is set with the current index being used.

Am_Define_Method(Am_Object_Method, void, Am_Slot_List_Setter_Do,
                 (Am_Object command_obj))
{
  Am_Object obj = command_obj.Get(Am_OPERATES_ON);
  Am_INTER_TRACE_PRINT(command_obj, "Animation command " << command_obj
                                                         << " operating on "
                                                         << obj);
  if (obj.Valid()) {
    Am_Value slot_value, value1, value2, inc_value, value_to_use;
    Am_Slot_Key slot = (int)command_obj.Get(Am_SLOT_TO_ANIMATE);
    value1 = command_obj.Peek(Am_VALUES_FOR_SLOT_TO_ANIMATE);
    if (Am_Value_List::Test(value1)) {
      Am_Value_List values = value1;
      value2 = command_obj.Peek(Am_INDICES_FOR_VALUES_FOR_SLOT_TO_ANIMATE);
      Am_Value_List indices;
      bool indirect = false;
      int len;
      if (Am_Value_List::Test(value2)) {
        indices = value2;
        len = indices.Length();
        indirect = true;
      } else
        len = values.Length();
      int current_index = command_obj.Get(Am_CURRENT_STATE);
      int current_direction = command_obj.Get(Am_CURRENT_DIRECTION);
      Am_Animation_End_Action end_act = command_obj.Get(Am_ANIMATE_END_ACTION);
      current_index += current_direction;
      switch (end_act.value) {
      case Am_ANIMATION_STOP_val:
        if (current_index < 0 || current_index >= len) {
          Am_Object inter = command_obj.Get(Am_SAVED_OLD_OWNER);
          Am_Stop_Interactor(inter);
          return;
        }
        break;
      case Am_ANIMATION_WRAP_val:
        if (current_index < 0)
          current_index = len - 1;
        else if (current_index >= len)
          current_index = 0;
        break;
      case Am_ANIMATION_BOUNCE_val:
        if (current_index < 0) {
          current_index = 1;
          command_obj.Set(Am_CURRENT_DIRECTION, 1);
        } else if (current_index >= len) {
          current_index = len - 2;
          command_obj.Set(Am_CURRENT_DIRECTION, -1);
        }
        break;
      default:
        Am_ERROR("Bad Am_ANIMATE_END_ACTION " << (int)end_act.value << " in "
                                              << command_obj)
      }
      command_obj.Set(Am_CURRENT_STATE, current_index);
      if (indirect) {
        value_to_use = indices.Get_Nth(current_index);
        current_index = value_to_use;
      }
      value_to_use = values.Get_Nth(current_index);

      Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING,
                           "++ Animation list command "
                               << command_obj << " index " << current_index
                               << " setting slot " << slot << " of " << obj
                               << " to be " << value_to_use);
      obj.Set(slot, value_to_use);
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

/////// Am_Animation_Blink_Command;
//Gets the object out of the Am_OPERATES_ON slot, and then gets the
// slot (no lists supported) out of the Am_SLOT_TO_ANIMATE slot.  Gets
// the values out of the Am_VALUE_1 and Am_VALUE_2 slots, and if the
// object has value1 in its slot, then sets it to value2, else sets it
// to value1. Never stops by itself.

Am_Define_Method(Am_Object_Method, void, Am_Slot_Blink_Do,
                 (Am_Object command_obj))
{
  Am_Object obj = command_obj.Get(Am_OPERATES_ON);
  Am_INTER_TRACE_PRINT(command_obj, "Animation command " << command_obj
                                                         << " operating on "
                                                         << obj);
  if (obj.Valid()) {
    Am_Value value1, value2, cur_value, new_value;
    Am_Slot_Key slot = (int)command_obj.Get(Am_SLOT_TO_ANIMATE);
    value1 = command_obj.Peek(Am_VALUE_1);
    value2 = command_obj.Peek(Am_VALUE_2);
    cur_value = obj.Peek(slot);
    if (cur_value == value1)
      new_value = value2;
    else
      new_value = value1;
    Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING,
                         "++ Animation blink command "
                             << command_obj << " setting slot " << slot
                             << " of " << obj << " to be " << new_value);
    obj.Set(slot, new_value);
  }
}

/* ******************************************************
1) Call DO_METHOD every Am_REPEAT_DELAY seconds
* just use any Command object

2) Call DO_METHOD once after Am_REPEAT_DELAY seconds;
* set Am_CONTINUOUS of inter to false.

3) Increment slot S by INC after N sec have passed
4) Increment slot S by INC every N sec
* use the Am_Incrementer_Animation_Command and set Am_CONTINUOUS
* appropriately

5) Change slot S from value_1 to value_2 (which are numbers) in N sec as
smoothly as possible
* use the Am_Animator_Animation_Command

6) Given list L of values, animate slot S by setting it with the next value in
L every N msec.  L might be a list of images for a pixmap.
  Variations:
	6a) May want control over whether go back and forth or wrap:
		1 2 3 4 3 2 1 2 3 4... vs. 1 2 3 4 1 2 3 4
	6b) Use a separate list of indices into L and iterate through the
		indices, in case you want to use the entries of L out of order
		or reuse entries multiple times.
*  Am_Slot_List_Setter_Do
*****************************************
*/

/*
//////////////////////////////////////////////////////////////////
/// Colliding objects
//////////////////////////////////////////////////////////////////

void Am_Initialize_Collide_Interactor () {
 #ifdef DEBUG
 #endif

  Am_Collide_Interactor = Am_Interactor.Create(DSTR("Am_Collide_Interactor"))
     .Set (Am_INTER_START_METHOD, Am_Collide_Start_Method)
     .Set (Am_INTER_ABORT_METHOD, Am_Collide_Abort_Method)
     .Set (Am_INTER_OUTSIDE_METHOD, Am_Collide_Outside_Method)
     // default outside stop method is fine
     .Set (Am_INTER_BACK_INSIDE_METHOD, Am_Collide_Back_Inside_Method)
     .Set (Am_INTER_STOP_METHOD, Am_Collide_Stop_Method)

     .Set (Am_START_DO_METHOD, Am_Collide_Start_Do)
     .Set (Am_ABORT_DO_METHOD, Am_Collide_Abort_Do)
     .Set (Am_DO_METHOD, Am_Collide_Stop_Do)

     .Set (Am_CONTINUOUS, false)

     //not a regular Interactor, make sure never gets on Window's lists
     .Set (Am_DEPENDS_ON_PRIORITY, (0L))
     .Set (Am_DEPENDS_ON_RANK_DEPTH, maintain_collide_inter_on_owner)
     .Set (Am_DEPENDS_ON_MULTI_OWNERS, (0L))
     ;
}

*/
////////////////////////////////////////////////////////////
//global variables

Am_Object Am_Animation_Interactor;
Am_Object Am_Animation_Command;
Am_Object Am_Incrementer_Animation_Command;
Am_Object Am_Animation_Object_In_Owner_Command;
Am_Object Am_Animator_Animation_Command;
Am_Object Am_Constant_Velocity_Command;
Am_Object Am_Animation_Through_List_Command;
Am_Object Am_Animation_Blink_Command;

static void
init()
{
#ifdef DEBUG
  Am_Register_Slot_Key(Am_ANIMATE_FORWARD, "~Am_ANIMATE_FORWARD~");
  Am_Register_Slot_Key(Am_ANIMATION_METHOD, "ANIMATION_METHOD");
  Am_Register_Slot_Key(Am_REPEAT_DELAY, "REPEAT_DELAY");
  Am_Register_Slot_Key(Am_ELAPSED_TIME, "ELAPSED_TIME");
  Am_Register_Slot_Key(Am_SLOT_TO_ANIMATE, "SLOT_TO_ANIMATE");
  Am_Register_Slot_Key(Am_TIME_FOR_ANIMATION, "TIME_FOR_ANIMATION");
  Am_Register_Slot_Key(Am_VALUES_FOR_SLOT_TO_ANIMATE,
                       "VALUES_FOR_SLOT_TO_ANIMATE");
  Am_Register_Slot_Key(Am_INDICES_FOR_VALUES_FOR_SLOT_TO_ANIMATE,
                       "INDICES_FOR_VALUES_FOR_SLOT_TO_ANIMATE");
  Am_Register_Slot_Key(Am_CURRENT_DIRECTION, "CURRENT_DIRECTION");
  Am_Register_Slot_Key(Am_ANIMATE_END_ACTION, "ANIMATE_END_ACTION");
  Am_Register_Slot_Key(Am_ANIMATE_END_METHOD, "ANIMATE_END_METHOD");
#endif

  Am_Animation_Command =
      Am_Command.Create(DSTR("Animation_Command"))
          .Set(Am_INTERIM_DO_METHOD, (0L))
          .Set(Am_DO_METHOD, (0L))
          .Set(Am_UNDO_METHOD, (0L))
          .Set(Am_REDO_METHOD, (0L))
          .Set(Am_SELECTIVE_UNDO_METHOD, (0L))
          .Set(Am_SELECTIVE_REPEAT_SAME_METHOD, (0L))
          .Set(Am_SELECTIVE_REPEAT_ON_NEW_METHOD, (0L))
          .Set(Am_SELECTIVE_UNDO_ALLOWED, Am_Selective_Allowed_Return_False)
          .Set(Am_SELECTIVE_REPEAT_SAME_ALLOWED,
               Am_Selective_Allowed_Return_False)
          .Set(Am_SELECTIVE_REPEAT_NEW_ALLOWED,
               Am_Selective_New_Allowed_Return_False)
          .Set(Am_LABEL, "Animation command")
          .Add(Am_ELAPSED_TIME, 0);

  Am_Incrementer_Animation_Command =
      Am_Animation_Command.Create(DSTR("Incrementer_Animation_Command"))
          .Set(Am_INTERIM_DO_METHOD, Am_Slot_Incrementer_Do)
          .Add(Am_OPERATES_ON, Am_Get_Inter_Object)
          .Add(Am_SLOT_TO_ANIMATE, Am_LEFT)
          .Add(Am_SMALL_INCREMENT, 1);

  Am_Animation_Object_In_Owner_Command =
      Am_Animation_Command.Create(DSTR("Animation_Object_In_Owner_Command"))
          .Set(Am_INTERIM_DO_METHOD, Am_Inc_Object_In_Owner_Do)
          .Add(Am_OPERATES_ON, Am_Get_Inter_Object)
          .Add(Am_X_OFFSET, 1)
          .Add(Am_Y_OFFSET, 1)
          .Add(Am_ANIMATE_END_ACTION, Am_ANIMATION_BOUNCE);

  Am_Animator_Animation_Command =
      Am_Animation_Command.Create(DSTR("Animator_Animation_Command"))
          .Set(Am_INTERIM_DO_METHOD, Am_Slot_Animator_Do)
          .Add(Am_OPERATES_ON, Am_Get_Inter_Object)
          .Add(Am_SLOT_TO_ANIMATE, Am_LEFT)
          .Add(Am_VALUE_1, 0)
          .Add(Am_VALUE_2, 100)
          .Add(Am_ANIMATE_FORWARD, true)
          .Add(Am_ANIMATE_END_ACTION, Am_ANIMATION_STOP)
          .Add(Am_TIME_FOR_ANIMATION, Am_Time(3000)) //milliseconds
      ;

  Am_Constant_Velocity_Command =
      Am_Animator_Animation_Command.Create(DSTR("Constant_Velocity_Command"))
          .Set(Am_INTERIM_DO_METHOD, Am_Constant_Velocity_Do)
          .Add(Am_VELOCITY, 1000); // units per second
  Am_Constant_Velocity_Command.Remove_Slot(Am_TIME_FOR_ANIMATION);

  Am_Animation_Through_List_Command =
      Am_Animation_Command.Create(DSTR("Animation_Through_List_Command"))
          .Set(Am_INTERIM_DO_METHOD, Am_Slot_List_Setter_Do)
          .Add(Am_OPERATES_ON, Am_Get_Inter_Object)
          .Add(Am_SLOT_TO_ANIMATE, Am_IMAGE)
          .Add(Am_VALUES_FOR_SLOT_TO_ANIMATE,
               (0L)) //value_list of values for slot
          .Add(Am_INDICES_FOR_VALUES_FOR_SLOT_TO_ANIMATE, (0L))
          .Add(Am_ANIMATE_END_ACTION, Am_ANIMATION_BOUNCE)
          .Set(Am_SLOT_TO_ANIMATE, Am_IMAGE)
          .Add(Am_CURRENT_DIRECTION, 1) // +1 or -1
          .Add(Am_CURRENT_STATE, 0)     //initial value
      ;

  Am_Animation_Blink_Command =
      Am_Animation_Command.Create(DSTR("Animation_Blink_Command"))
          .Set(Am_INTERIM_DO_METHOD, Am_Slot_Blink_Do)
          .Add(Am_OPERATES_ON, Am_Get_Inter_Object)
          .Add(Am_SLOT_TO_ANIMATE, Am_VISIBLE)
          .Add(Am_VALUE_1, true)
          .Add(Am_VALUE_2, false)
          .Add(Am_TIME_FOR_ANIMATION, Am_Time(3000)) //milliseconds
      ;

  Am_Animation_Interactor =
      Am_Interactor.Create(DSTR("Am_Animation_Interactor"))
          .Set(Am_INTER_START_METHOD, Am_Animation_Start_Method)
          .Set(Am_INTER_ABORT_METHOD, Am_Animation_Abort_Method)
          .Set(Am_INTER_OUTSIDE_METHOD, Am_Animation_Outside_Method)
          // default outside stop method is fine
          .Set(Am_INTER_BACK_INSIDE_METHOD, Am_Animation_Back_Inside_Method)
          .Set(Am_INTER_STOP_METHOD, Am_Animation_Stop_Method)

          .Set(Am_START_DO_METHOD, Am_Animation_Start_Do)
          .Set(Am_ABORT_DO_METHOD, Am_Animation_Abort_Do)
          .Set(Am_DO_METHOD, Am_Animation_Stop_Do)

          .Add(Am_ANIMATION_METHOD, animation_timer_method)

          .Add(Am_REPEAT_DELAY, Am_Time(100)) //milliseconds = 0.1 sec
          .Set(Am_CONTINUOUS, true)
          .Set(Am_STOP_WHEN, false) //never stops
          .Set_Part(Am_COMMAND, Am_Animation_Command);
}

static Am_Initializer *initializer =
    new Am_Initializer(DSTR("Inter_Anim"), init, 4.1f);

/*  ***
** Make interactors start by themselves when Set_part using this formula

// use this one to also start the interactor automatically if its
// start_when is true.

Am_Define_Formula (int, maintain_inter_to_window_list_rank)

Get animation interactors NOT interested in move events!!

*/
