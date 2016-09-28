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

#include <amulet.h>
#include <amulet/opal_advanced.h>
#include <amulet/inter_advanced.h> //for am_inter_trace_print
#include <amulet/web.h>
#include <amulet/initializer.h>

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923
#endif

//lint -e641 disable using enum as int warning

// Starting, stopping, aborting interpolators
void
Am_Start_Animator(Am_Object interp, const Am_Value &value1,
                  const Am_Value &value2)
{
  if (static_cast<bool>(interp.Get(Am_RUNNING)) == true) {
    return;
  }

  interp.Set(Am_RUNNING, true);

  if (value1 != Am_No_Value) {
    interp.Set(Am_VALUE_1, value1);
    interp.Set(Am_VALUE, value1);
    Am_Set_Animated_Slots(interp); // reset the slots to value1
  }

  if (value2 != Am_No_Value) {
    interp.Set(Am_VALUE_2, value2);
  }

  Am_Object_Method method = interp.Get(Am_START_DO_METHOD);
  if (method.Valid()) {
    method.Call(interp);
  }

  return;
}

void
Am_Interrupt_Animator(const Am_Object &interp, const Am_Value &new_value)
{
  if (static_cast<bool>(interp.Get(Am_RUNNING)) == false) {
    return;
  }

  Am_Anim_Update_Method method = interp.Get(Am_INTERRUPT_METHOD);
  if (method.Valid()) {
    method.Call(interp, new_value);
  }

  return;
}

void
Am_Abort_Animator(Am_Object interp)
{
  if (static_cast<bool>(interp.Get(Am_RUNNING)) == false) {
    return;
  }

  interp.Set(Am_RUNNING, false);

  Am_Object_Method method = interp.Get(Am_ABORT_DO_METHOD);
  if (method.Valid()) {
    method.Call(interp);
  }

  return;
}

void
Am_Stop_Animator(Am_Object interp)
{
  if (!static_cast<bool>(interp.Get(Am_RUNNING))) {
    return;
  }

  interp.Set(Am_RUNNING, false);

  Am_Object_Method method = interp.Get(Am_DO_METHOD);
  if (method.Valid()) {
    method.Call(interp);
  }

  return;
}

////////////////////////////////////////////////////////////
// The standard interpolator
////////////////////////////////////////////////////////////

AM_DEFINE_METHOD_TYPE_IMPL(Am_Timing_Function);
AM_DEFINE_METHOD_TYPE_IMPL(Am_Path_Function);

//
// The standard command methods START_DO, ANIMATION, ABORT_DO, and DO
// have the following semantics in interpolator objects:
//
// - START_DO is called every time a Set occurs.  Before it is called,
//   VALUE_1 is set equal to VALUE, and VALUE_2 is set to the value that
//   was Set.  This method should start a timer.
//
// - ANIMATION is called on every timer tick with the elapsed time since
//   the animation started.
//   The method should compute a new VALUE, and call the Am_INTERIM_DO method
//   when the final value has been reached.
//
// - ABORT_DO is called if the animation is aborted (perhaps by another Set
//   if the animation is interruptible, or perhaps by an explicit
//   Am_Abort_Animator).  This method should stop the timer and
//   assert the current value.
//
// - DO is called when the animation reaches its final value.  This method
//   should stop the timer.
//

///
/// Standard methods in prototypical interpolator
///

Am_Define_Method(Am_Object_Method, void, Am_Animator_Start_Do,
                 (Am_Object interp))
{
  // set up the animation timer
  Am_Time delay = interp.Get(Am_REPEAT_DELAY);
  Am_INTER_TRACE_PRINT(interp, "Animator Start of " << interp
                                                    << " delay = " << delay);
  Am_Register_Timer(delay, interp, Am_ANIMATION_METHOD, false);

  // invoke the animation method for time == 0
  Am_Timer_Method method = interp.Get(Am_ANIMATION_METHOD);
  method.Call(interp, Am_Time(static_cast<unsigned long>(0)));

  Am_Set_Animated_Slots(interp);

  return;
}

Am_Define_Method(Am_Anim_Update_Method, void, Am_Animator_Interrupt,
                 (Am_Object interp, const Am_Value &new_value))
{
  if ((bool)interp.Get(Am_INTERRUPTIBLE)) {
    const Am_Value& curr_value = interp.Get(Am_VALUE);
    Am_INTER_TRACE_PRINT(interp, "Animator Interrupt of "
                                     << interp << "; restarting from value "
                                     << curr_value << " towards new value "
                                     << new_value);
    Am_Abort_Animator(interp);

    if (curr_value != new_value)
      Am_Start_Animator(interp, curr_value, new_value);
  } else {
    Am_INTER_TRACE_PRINT(
        interp, "Animator Interrupt of "
                    << interp << "(running uninterrupted); got new value "
                    << new_value);

    Am_Value_List values;
    values = interp.Get(Am_VALUES);
    int curr_phase = interp.Get(Am_CURRENT_PHASE);

    if (curr_phase < 0 || curr_phase >= ((int)values.Length()) - 1) {
      // then curr_phase and values are not currently in use -- initialize them
      values = Am_Value_List()
                   .Add(interp.Get(Am_VALUE_1))
                   .Add(interp.Get(Am_VALUE_2));
      curr_phase = 0;
      interp.Set(Am_CURRENT_PHASE, curr_phase);
    }

    values.Add(new_value);
    interp.Set(Am_VALUES, values);

    Am_Set_Animated_Slots(interp);
  }

  return;
}

Am_Define_Method(Am_Object_Method, void, Am_Animator_Interim_Do,
                 (Am_Object interp))
{
  Am_INTER_TRACE_PRINT(interp, "Animator Interim Do of " << interp);

  Am_Value_List values = interp.Get(Am_VALUES);
  int curr_phase = interp.Get(Am_CURRENT_PHASE);

  if (curr_phase < ((int)values.Length()) - 1) {
    ++curr_phase;
    interp.Set(Am_CURRENT_PHASE, curr_phase);
  }

  if (curr_phase < ((int)values.Length()) - 1) {
    Am_Stop_Timer(interp, Am_ANIMATION_METHOD);

    values.Move_Nth(curr_phase);
    interp.Set(Am_VALUE_1, values.Get());
    values.Next();
    interp.Set(Am_VALUE_2, values.Get());

    Am_Object_Method method = interp.Get(Am_START_DO_METHOD);
    if (method.Valid())
      method.Call(interp);
  } else {
    Am_Stop_Animator(interp);
  }

  Am_Set_Animated_Slots(interp);
}

Am_Define_Method(Am_Timer_Method, void, Am_Animator_Animation,
                 (Am_Object interp, const Am_Time &elapsed_time))
{
  interp.Set(Am_ELAPSED_TIME, elapsed_time);

  //
  // INTERPOLATING
  //

  // tau is the current position along the parameterized path,
  // measured as a fraction of the distance along total path.
  // (The path may be abstract;  for instance, if the path is just a list
  // of values, tau = 0.5 would mean halfway down the list.)
  //
  // The timing function maps elapsed_time (true time) into tau (path time).
  Am_Timing_Function timing = interp.Get(Am_TIMING_FUNCTION);
  float tau = timing.Call(interp, elapsed_time);

  // The path function maps tau into an actual position on the path.
  Am_Value value1, value2;
  value1 = interp.Peek(Am_VALUE_1);
  value2 = interp.Peek(Am_VALUE_2);
  Am_Path_Function path = interp.Get(Am_PATH_FUNCTION);
  Am_Value curr_value = path.Call(interp, value1, value2, tau);
  interp.Set(Am_VALUE, curr_value);

  Am_Time duration = interp.Get(Am_CURRENT_DURATION);
  Am_INTER_TRACE_PRINT(interp, "Animator Animation of "
                                   << interp << ", time=" << elapsed_time
                                   << ", duration=" << duration
                                   << ", val=" << interp.Get(Am_VALUE));

  if (tau >= 1.0 && !elapsed_time.Zero()) {
    // call INTERIM_DO to get next segment of path (if any)
    interp.Set(Am_VALUE, value2);

    Am_Object_Method method = interp.Get(Am_INTERIM_DO_METHOD);
    if (method.Valid())
      method.Call(interp);
  }

  Am_Set_Animated_Slots(interp);
}

static void
call_all_commands(Am_Value value, Am_Slot_Key method_slot, Am_Object &inter,
                  bool skip_first)
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
      Am_Object_Method do_method = value;
      Am_INTER_TRACE_PRINT(
          inter, "%%Animator " << inter << " executing " << method_slot
                               << " of " << command_obj << "=" << do_method);
      do_method.Call(command_obj);
      if ((bool)command_obj.Get(Am_COMMAND_IS_ABORTING))
        return; //break out of loop
    }
    value = command_obj.Peek(Am_IMPLEMENTATION_PARENT);
  }
}

Am_Define_Method(Am_Object_Method, void, Am_Animator_Abort_Do,
                 (Am_Object interp))
{
  Am_INTER_TRACE_PRINT(interp, "Animator Abort of " << interp);

  // leave current value as-is, but stop the timer
  Am_Stop_Timer(interp, Am_ANIMATION_METHOD);

  Am_Set_Animated_Slots(interp);

  Am_Value value;
  value = interp.Peek(Am_COMMAND);
  call_all_commands(value, Am_ABORT_DO_METHOD, interp, false);
}

Am_Define_Method(Am_Object_Method, void, Am_Animator_Do, (Am_Object interp))
{
  Am_Value value_2 = interp.Get(Am_VALUE_2);
  Am_INTER_TRACE_PRINT(
      interp, "Animator Do of " << interp << " setting value to " << value_2);
  // assert final value and stop the timer
  interp.Set(Am_VALUE, value_2);
  Am_Stop_Timer(interp, Am_ANIMATION_METHOD);

  Am_Set_Animated_Slots(interp);

  Am_Value value;
  value = interp.Peek(Am_COMMAND);
  call_all_commands(value, Am_DO_METHOD, interp, false);
}

///
/// Standard timing formulas
///

Am_Define_Formula(float, compute_path_length)
{
  Am_Value value1, value2;
  value1 = self.Get(Am_VALUE_1, Am_OK_IF_NOT_THERE);
  value2 = self.Get(Am_VALUE_2, Am_OK_IF_NOT_THERE);
  Am_Path_Function path = self.Get(Am_PATH_FUNCTION);

  if (!value1.Exists() || !value2.Exists() || !path.Valid())
    return 0;
  else
    return path.Call(self, value1, value2, Am_COMPUTE_DISTANCE);
}

bool
timing_web_create(const Am_Slot &slot)
{
  return slot.Get_Key() == Am_CURRENT_VELOCITY;
}

void
timing_web_init(const Am_Slot &slot, Am_Web_Init &init)
{
  Am_Object_Advanced obj = slot.Get_Owner();
  init.Note_Input(obj, Am_VELOCITY);
  init.Note_Input(obj, Am_DURATION);
  init.Note_Input(obj, Am_PATH_LENGTH);
  init.Note_Input(obj, Am_TIMING_MODE);
  init.Note_Output(obj, Am_VELOCITY);
  init.Note_Output(obj, Am_DURATION);
  init.Note_Output(obj, Am_CURRENT_DURATION);
  init.Note_Output(obj, Am_TIMING_MODE);
}

void
timing_web_validate(Am_Web_Events &events)
{
  events.Start();
  Am_Slot slot = events.Get();
  Am_Object self = slot.Get_Owner();
  float fixed_velocity = self.Get(Am_VELOCITY);
  Am_Time fixed_duration = self.Get(Am_DURATION);
  float velocity = fixed_velocity;
  Am_Time duration = fixed_duration;
  float length = self.Get(Am_PATH_LENGTH);
  Am_Slot_Key timing_mode = (int)self.Get(Am_TIMING_MODE);

  events.Start();
  while (!events.Last()) {
    slot = events.Get();
    switch (slot.Get_Key()) {
    case Am_VELOCITY:
      // std::cout << self << " VELOCITY <- " << velocity <<std::endl;
      if (fixed_velocity > 0) {
        duration = Am_Time((unsigned long)((1000 * length) / fixed_velocity));
        fixed_duration = Am_Time((unsigned long)0);
        timing_mode = Am_VELOCITY;
      }
      break;

    case Am_DURATION:
      // std::cout << self << " DURATION <- " << duration <<std::endl;
      if (fixed_duration.Valid() && fixed_duration.Milliseconds() > 0) {
        velocity = (1000 * length) / (float)fixed_duration.Milliseconds();
        fixed_velocity = 0;
        timing_mode = Am_DURATION;
      }
      break;

    case Am_PATH_LENGTH:
    case Am_TIMING_MODE:
      // std::cout << self << " LENGTH <- " << length <<std::endl;
      if (timing_mode == Am_VELOCITY) {
        // std::cout << "\tMODE = velocity" <<std::endl;
        duration = Am_Time((unsigned long)(1000 * length / fixed_velocity));
      } else {
        // std::cout << "\tMODE = duration" <<std::endl;
        velocity = (1000 * length) / (float)fixed_duration.Milliseconds();
      }
      break;
    }
    events.Next();
  }
  self.Set(Am_VELOCITY, fixed_velocity);
  self.Set(Am_DURATION, fixed_duration);
  self.Set(Am_CURRENT_VELOCITY, velocity);
  self.Set(Am_CURRENT_DURATION, duration);
  self.Set(Am_TIMING_MODE, (int)timing_mode);
}

Am_Define_Formula(Am_Wrapper *, compute_repeat_delay)
{
  float velocity = self.Get(Am_CURRENT_VELOCITY);
  Am_Time min_repeat_delay = self.Get(Am_MIN_REPEAT_DELAY);

  Am_Time delay_per_pixel = Am_Time((long)(velocity ? 1000 / velocity : 0));

  return (delay_per_pixel > min_repeat_delay) ? delay_per_pixel
                                              : min_repeat_delay;
}

////////////////////////////////////////////////////////////
// Timing functions
////////////////////////////////////////////////////////////

//
// Am_Linear_Timing
//

Am_Define_Method(Am_Timing_Function, float, Am_Linear_Timing,
                 (Am_Object interp, Am_Time t))
{
  Am_Time total_time = interp.Get(Am_CURRENT_DURATION);

  unsigned long t_ms = t.Milliseconds();
  unsigned long total_ms = total_time.Milliseconds();

  if (t_ms >= total_ms)
    return 1.0;
  else
    return (float)((double)t_ms / (double)total_ms);
}

//
// Am_Slow_In_Slow_Out
//

Am_Define_Method(Am_Timing_Function, float, Am_Delayed_Timing,
                 (Am_Object interp, Am_Time t))
{
  Am_Time delay = interp.Get(Am_INITIAL_DELAY);

  if (t < delay)
    return 0.0;
  else
    return Am_Linear_Timing_proc(interp, t - delay);
}

//
// Am_Slow_In_Slow_Out
//

Am_Define_Method(Am_Timing_Function, float, Am_Slow_In_Slow_Out,
                 (Am_Object interp, Am_Time t))
{
  float fraction = Am_Linear_Timing_proc(interp, t);

  float D_0 = interp.Get(Am_SHARPNESS_1);
  float D_1 = interp.Get(Am_SHARPNESS_2);

  float tau = (float)((atan((D_0 + D_1) * fraction - D_0) + atan(D_0)) /
                      (atan(D_0) + atan(D_1)));

  return tau;
}

////////////////////////////////////////////////////////////
// Path functions
////////////////////////////////////////////////////////////

//
// Am_Linear_Path: N-dimensional float or integer interpolation
//

static Am_Value
interpolate_1D(const Am_Value &value1, const Am_Value &value2, float tau)
{
  float return_val;

  if (tau == Am_COMPUTE_DISTANCE) {
    // return magnitude of distance between value1 and value2
    return_val = (float)fabs((float)value1 - (float)value2);
  } else {
    float v1 = value1;
    float v2 = value2;

    // return interpolated value
    return_val = v1 + (v2 - v1) * tau;
  }

  if ((value1.type == Am_INT || value1.type == Am_LONG) &&
      (value2.type == Am_INT || value2.type == Am_LONG))
    return (int)return_val;
  else
    return return_val;
}

static Am_Value
interpolate_ND(const Am_Value &value1, const Am_Value &value2, float tau)
{
  Am_Value_List list1 = value1;
  Am_Value_List list2 = value2;

  if (tau == Am_COMPUTE_DISTANCE) {
    // return Euclidean distance between value1 and value2
    float D = 0.0;
    for (list1.Start(), list2.Start(); !list1.Last() && !list2.Last();
         list1.Next(), list2.Next()) {
      float dx = (float)interpolate_1D(list1.Get(), list2.Get(), tau);
      D += dx * dx;
    }

    return sqrt(D);
  } else {
    // return interpolated value
    Am_Value_List result;

    for (list1.Start(), list2.Start(); !list1.Last() && !list2.Last();
         list1.Next(), list2.Next()) {
      result.Add(interpolate_1D(list1.Get(), list2.Get(), tau));
    }

    return Am_Value(result);
  }
}

Am_Define_Method(Am_Path_Function, Am_Value, Am_Linear_Path,
                 (Am_Object /* interp */, Am_Value value1, Am_Value value2,
                  float tau))
{
  if (Am_Value_List::Test(value1))
    return interpolate_ND(value1, value2, tau);
  else
    return interpolate_1D(value1, value2, tau);
}

//
// Am_Style_Path: fill styles and line styles
//

Am_Define_Method(Am_Path_Function, Am_Value, Am_Style_Path,
                 (Am_Object interp, Am_Value value1, Am_Value value2,
                  float tau))
{
  float r1, g1, b1;
  float r2, g2, b2;
  short t1, t2;
  Am_Line_Cap_Style_Flag cap;
  Am_Join_Style_Flag join;
  Am_Line_Solid_Flag line_flag;
  const char *dash_l;
  int dash_l_length;
  Am_Fill_Solid_Flag fill_flag;
  Am_Fill_Poly_Flag poly;
  Am_Image_Array stipple;

  Am_Style(value1).Get_Values(r1, g1, b1);
  Am_Style(value1).Get_Line_Thickness_Values(t1, cap);
  Am_Style(value2).Get_Values(r2, g2, b2, t2, cap, join, line_flag, dash_l,
                              dash_l_length, fill_flag, poly, stipple);
  short dt = t2 - t1;

  float fr, fg, fb;

  if ((bool)interp.Get(Am_RGB_COLOR, Am_NO_DEPENDENCY)) {
    float dr = r2 - r1;
    float dg = g2 - g1;
    float db = b2 - b1;

    if (tau == Am_COMPUTE_DISTANCE)
      return sqrt(dr * dr + dg * dg + db * db + dt * dt) * 100.0;

    fr = dr * tau + r1;
    fg = dg * tau + g1;
    fb = db * tau + b1;
  } else {
    float h1, s1, v1;
    float h2, s2, v2;
    Am_RGB_To_HSV(r1, g1, b1, h1, s1, v1);
    Am_RGB_To_HSV(r2, g2, b2, h2, s2, v2);
    if (s1 == 0.0)
      h1 = h2;
    if (s2 == 0.0)
      h2 = h1;
    float dh = h2 - h1;
    if (dh > 180.0)
      dh -= 360.0;
    if (dh < -180.0)
      dh += 360.0;
    float ds = s2 - s1;
    float dv = v2 - v1;

    if (tau == Am_COMPUTE_DISTANCE)
      return sqrt(dh * dh + ds * ds + dv * dv + dt * dt) * 100.0;

    float fh = dh * tau + h1;
    if (fh < 0.0)
      fh += 360.0;
    if (fh >= 360.0)
      fh -= 360.0;
    float fs = ds * tau + s1;
    float fv = dv * tau + v1;
    Am_HSV_To_RGB(fh, fs, fv, fr, fg, fb);
  }

  short ft = (short)(dt * tau + t1);
  Am_Style s = Am_Style(fr, fg, fb, ft, cap, join, line_flag, dash_l,
                        dash_l_length, fill_flag, poly, stipple);
  return (Am_Wrapper *)s;
}

//
// Am_Step_Path: stays at VALUE_1 for entire path, then finishes at VALUE_2
//

Am_Define_Method(Am_Path_Function, Am_Value, Am_Step_Path,
                 (Am_Object /* interp */, Am_Value value_1, Am_Value value_2,
                  float tau))
{
  if (tau == Am_COMPUTE_DISTANCE)
    return value_1 != value_2 ? 1 : 0;
  else if (tau < 1.0)
    return value_1;
  else
    return value_2;
}

//
// Am_Through_List_Path: interpolates along the list found in Am_LIST_PATH,
//   which is usually computed by a constraint.
//

Am_Define_Method(Am_Path_Function, Am_Value, Am_Through_List_Path,
                 (Am_Object interp, Am_Value /* value_1 */, Am_Value value_2,
                  float tau))
{
  Am_Value_List list = interp.Get(Am_CURRENT_PATH);
  int len = list.Length();

  if (tau == Am_COMPUTE_DISTANCE) {
    return len;
  } else if (len == 0)
    return value_2;
  else {
    float f = len * tau;    // f in [0,len]
    int n = (int)(f + 0.5); // round f to nearest integer

#if 0
		static int last_n = -1;
	std::cout << n << " " << interp.Get(Am_ELAPSED_TIME);
		if(!(last_n == len ? (n == 0) : (n == last_n+1)))
		std::cout << " (not successor!)";
	std::cout <<std::endl;
		last_n = n;
#endif

    if (n >= len)
      n = len - 1;

    return list.Get_Nth(n);
  }
}

//
// Am_Oscillating_Path
//
// Computes a sinusoidal oscillation of frequency Am_FREQUENCY whose envelope
// is a decaying exponential with time constant Am_TIME_CONSTANT.
//
// boy, is this overkill.
//

// Am_Define_Method (Am_Path_Function, Am_Value, Am_Oscillating_Path,
// 		  (Am_Object interp, float tau))
// {
//   //
//   Am_Value value1, value2;
//   value1=interp.Peek(Am_VALUE_1);
//   value2=interp.Peek(Am_VALUE_2);

//   float freq = interp.Get (Am_FREQUENCY);
//   float time_constant = interp.Get (Am_TIME_CONSTANT);

//   float x1 = value1;
//   float x2 = value2;

//   float return_val =
//     x2 + (x1-x2) * exp (- tau / time_constant) * cos (freq * tau);

//   if (value1.type == Am_INT && value2.type == Am_INT)
//     return (int)return_val;
//   else
//     return return_val;
// }

////////////////////////////////////////////////////////////
/// Am_Stepping_Animator
////////////////////////////////////////////////////////////

static Am_Value
step_1D(const Am_Value &curr_value, const Am_Value &step_size,
        const Am_Value &value2, bool &keep_running)
{
  float v = curr_value;
  float step = step_size;

  if (value2.Exists()) {
    float v2 = value2;

    if (v < v2) {
      v += step;
      if (v > v2)
        v = v2;
      else
        keep_running = true;
    } else if (v > v2) {
      v -= step;
      if (v < v2)
        v = v2;
      else
        keep_running = true;
    }
  } else {
    v += step;
    keep_running = true;
  }

  if (curr_value.type == Am_INT || curr_value.type == Am_LONG)
    return (int)v;
  else
    return v;
}

static Am_Value
step_ND(const Am_Value &curr_value, const Am_Value &step_size,
        const Am_Value &value2, bool &keep_running)
{
  Am_Value_List values = curr_value;
  Am_Value_List steps = step_size;

  if (value2.Exists()) {
    Am_Value_List value2s = value2;
    Am_Value_List result;

    for (values.Start(), steps.Start(), value2s.Start();
         !values.Last() && !steps.Last() && !value2s.Last();
         values.Next(), steps.Next(), value2s.Next()) {
      result.Add(
          step_1D(values.Get(), steps.Get(), value2s.Get(), keep_running));
    }

    return Am_Value(result);
  } else {
    Am_Value_List result;

    for (values.Start(), steps.Start(); !values.Last() && !steps.Last();
         values.Next(), steps.Next()) {
      result.Add(step_1D(values.Get(), steps.Get(), Am_No_Value, keep_running));
    }

    return Am_Value(result);
  }
}

Am_Define_Method(Am_Timer_Method, void, Am_Stepping_Animator_Animation,
                 (Am_Object interp, const Am_Time &elapsed_time))
{
  interp.Set(Am_ELAPSED_TIME, elapsed_time);
  if (elapsed_time.Milliseconds() == 0)
    return;

  Am_Value curr_value = interp.Get(Am_VALUE);
  Am_Value step_size = interp.Get(Am_SMALL_INCREMENT);
  Am_Value value2 = interp.Get(Am_VALUE_2);

  bool keep_running = false;
  if (Am_Value_List::Test(curr_value))
    curr_value = step_ND(curr_value, step_size, value2, keep_running);
  else
    curr_value = step_1D(curr_value, step_size, value2, keep_running);

  Am_INTER_TRACE_PRINT(interp, "Stepper Animation " << interp << " new value "
                                                    << curr_value);

  interp.Set(Am_VALUE, curr_value);

  if (!keep_running) {
    Am_Object_Method method = interp.Get(Am_INTERIM_DO_METHOD);
    if (method.Valid())
      method.Call(interp);
  }

  Am_Set_Animated_Slots(interp);
}

////////////////////////////////////////////////////////////
// Am_Blink_Animator
////////////////////////////////////////////////////////////

Am_Define_Method(Am_Timer_Method, void, Am_Blink_Animator_Animation,
                 (Am_Object interp, const Am_Time &elapsed_time))
{
  interp.Set(Am_ELAPSED_TIME, elapsed_time);

  Am_Value curr_value = interp.Get(Am_VALUE);
  Am_Value value1 = interp.Get(Am_VALUE_1);
  Am_Value value2 = interp.Get(Am_VALUE_2);

  if (curr_value == value1)
    interp.Set(Am_VALUE, value2);
  else
    interp.Set(Am_VALUE, value1);

  Am_Set_Animated_Slots(interp);
}

////////////////////////////////////////////////////////////
// Am_Through_List_Animator
////////////////////////////////////////////////////////////

inline int
list_position(Am_Value_List &list, const Am_Value &v)
{
  int i;

  for (i = 0, list.Start(); !list.Last(); ++i, list.Next())
    if (v == list.Get())
      return i;
  return -1;
}

Am_Define_Value_List_Formula(compute_list_path)
{
  Am_Value value1;
  value1 = self.Peek(Am_VALUE_1);
  Am_Value value2;
  value2 = self.Peek(Am_VALUE_2);
  Am_Value_List list = self.Get(Am_LIST_OF_VALUES);
  Am_Value_List new_list;

  if (!list.Valid())
    return list;

  // find value1 and value2 in the list.
  // if value1 isn't present, assume first element;
  // if value2 isn't present, assume last.
  int value1_pos = list_position(list, value1);
  if (value1_pos == -1)
    value1_pos = 0;

  int value2_pos = list_position(list, value2);
  if (value2_pos == -1)
    value2_pos = ((int)list.Length()) - 1;

  // starting at value1...
  list.Start();
  list.Move_Nth(value1_pos);
  self.Set(Am_VALUE_1, list.Get());
  int i = value1_pos;

  // either add following elements (if value1 precedes value2)
  while (i < value2_pos) {
    new_list.Add(list.Get());
    list.Next();
    ++i;
  }

  // OR, add preceding elements (if value1 follows value2)
  while (i > value2_pos) {
    new_list.Add(list.Get());
    list.Prev();
    --i;
  }

  // then add value2 itself
  new_list.Add(list.Get());
  self.Set(Am_VALUE_2, list.Get());

  return new_list;
}

Am_Define_Formula(int, compute_list_path_length)
{
  Am_Value_List list = self.Get(Am_CURRENT_PATH);
  return list.Valid() ? list.Length() : 0;
}

////////////////////////////////////////////////////////////
// Am_Exaggerated_Animator
////////////////////////////////////////////////////////////

enum
{
  Am_EXAGGERATOR_IDLE = -2,
  Am_EXAGGERATOR_WINDUP = -1,
  Am_EXAGGERATOR_INTERPOLATING = 0,
  Am_EXAGGERATOR_WIGGLING = 1 // and any integer higher
};

enum Direction
{
  AWAYFROM,
  TOWARDS
};

Am_Value
add_offset(Am_Value v, Am_Value delta, Direction d, Am_Value ref,
           float attenuation = 1.0)
{
  switch (v.type) {
  case Am_INT:
  case Am_LONG: {
    float fv = v;
    float fd = ((float)delta) * attenuation;
    float fref = ref;

    return (int)(((int)(d == AWAYFROM) ^ (int)(fv < fref)) ? fv + fd : fv - fd);
  }

  case Am_FLOAT: {
    float fv = v;
    float fd = ((float)delta) * attenuation;
    float fref = ref;

    return ((int)(d == AWAYFROM) ^ (int)(fv < fref)) ? fv + fd : fv - fd;
  }
  case Am_WRAPPER:
    if (Am_Value_List::Test(v)) {
      Am_Value_List lv = v;
      Am_Value_List ldelta = delta;
      Am_Value_List lref = ref;
      Am_Value_List result;

      for (lv.Start(), ldelta.Start(), lref.Start(); !lv.Last();
           lv.Next(), ldelta.Next(), lref.Next())
        result.Add(
            add_offset(lv.Get(), ldelta.Get(), d, lref.Get(), attenuation));
      return Am_Value(result);
    }
  // else fall through
  default:
    Am_ERROR("Exaggerated_Animator can't handle type of " << v);
    return Am_No_Value;
  }
}

static void
next_exaggerated_phase(Am_Object interp)
{
  int curr_state = interp.Get(Am_EXAGGERATOR_STATE);
  Am_Object subanimator = interp.Get_Object(Am_SUB_ANIMATOR);
  Am_Value curr_value = subanimator.Get(Am_VALUE);
  Am_Value value1 = interp.Get(Am_VALUE_1);
  Am_Value value2 = interp.Get(Am_VALUE_2);
  if (!curr_value.Valid())
    curr_value = value1;

  Am_Time duration;
  Am_Value target;
  int next_state;

  switch (curr_state) {
  case Am_EXAGGERATOR_IDLE:
    //
    // enter WINDUP phase
    //
    duration = interp.Get(Am_WINDUP_DELAY);
    target = add_offset(value1, interp.Get(Am_WINDUP_AMOUNT), AWAYFROM, value2);
    next_state = Am_EXAGGERATOR_WINDUP;
    break;

  case Am_EXAGGERATOR_WINDUP:
    //
    // enter INTERPOLATING phase
    //
    duration = interp.Get(Am_CURRENT_DURATION);
    target = value2;
    next_state = Am_EXAGGERATOR_INTERPOLATING;
    break;

  case Am_EXAGGERATOR_INTERPOLATING:
  default: {
    //
    // enter WIGGLING phase
    //
    int wiggle_count = interp.Get(Am_WIGGLES);
    int which_wiggle = curr_state - Am_EXAGGERATOR_INTERPOLATING;

    // also check whether there are any values waiting to be visited
    // skip the wiggling if there are (we'll just wiggle on the last one)
    Am_Value_List values = interp.Get(Am_VALUES);
    int curr_phase = interp.Get(Am_CURRENT_PHASE);
    bool values_waiting = (curr_phase + 1 < ((int)values.Length()) - 1);

    if (!values_waiting && which_wiggle < wiggle_count) {
      duration = interp.Get(Am_WIGGLE_DELAY);

      float attenuation = 1 - ((float)which_wiggle) / wiggle_count;

      target = add_offset(value2, interp.Get(Am_WIGGLE_AMOUNT),
                          (which_wiggle & 1) ? TOWARDS : AWAYFROM, value1,
                          attenuation);
      next_state = curr_state + 1;
    } else {
      // call INTERIM_DO to get next segment of path (if any)
      // pretend we've just done a windup at value2, so that the next segment of the path won't include a windup
      interp.Set(Am_VALUE, value2);
      interp.Set(Am_EXAGGERATOR_STATE, Am_EXAGGERATOR_WINDUP);

      Am_Object_Method method = interp.Get(Am_INTERIM_DO_METHOD);
      if (method.Valid())
        method.Call(interp);
      return;
    }
  }
  }

  interp.Set(Am_EXAGGERATOR_STATE, next_state);

  // set up sub animator to do the actual animation
  subanimator.Set(Am_DURATION, duration);
  Am_Start_Animator(subanimator, curr_value, target);
}

Am_Define_Method(Am_Object_Method, void, Am_Exaggerated_Animator_Start_Do,
                 (Am_Object interp))
{
  Am_INTER_TRACE_PRINT(interp, "Exaggerated Start of " << interp);

  next_exaggerated_phase(interp);
}

Am_Define_Method(Am_Object_Method, void, Am_Exaggerated_Animator_Abort_Do,
                 (Am_Object interp))
{
  Am_INTER_TRACE_PRINT(interp, "Exaggerated Abort of " << interp);
  int state = interp.Get(Am_EXAGGERATOR_STATE);

  Am_Abort_Animator(interp.Get(Am_SUB_ANIMATOR));

  if (state == Am_EXAGGERATOR_WINDUP) {
    // in wind-up phase -- set back to original value
    interp.Set(Am_VALUE, interp.Get(Am_VALUE_1));
  } else if (state >= Am_EXAGGERATOR_WIGGLING) {
    // in wiggle phase -- set to final value
    interp.Set(Am_VALUE, interp.Get(Am_VALUE_2));
  }

  interp.Set(Am_EXAGGERATOR_STATE, Am_EXAGGERATOR_IDLE);
}

Am_Define_Method(Am_Object_Method, void, Am_Exaggerated_Animator_Interim_Do,
                 (Am_Object interp))
{
  Am_INTER_TRACE_PRINT(interp, "Exaggerated Interim Do of " << interp);

  Am_Value_List values = interp.Get(Am_VALUES);
  int curr_phase = interp.Get(Am_CURRENT_PHASE);

  ++curr_phase;
  interp.Set(Am_CURRENT_PHASE, curr_phase);

  if (curr_phase < ((int)values.Length()) - 1) {
    values.Move_Nth(curr_phase);
    interp.Set(Am_VALUE_1, values.Get());
    values.Next();
    interp.Set(Am_VALUE_2, values.Get());

    next_exaggerated_phase(interp);
  } else {
    Am_Stop_Animator(interp);
  }

  Am_Set_Animated_Slots(interp);
}

Am_Define_Method(Am_Object_Method, void, Am_Exaggerated_Animator_Do,
                 (Am_Object interp))
{
  Am_INTER_TRACE_PRINT(interp, "Exaggerated Do of " << interp);
  interp.Set(Am_EXAGGERATOR_STATE, Am_EXAGGERATOR_IDLE);
  interp.Set(Am_VALUE, interp.Get(Am_VALUE_2));
}

// methods on subanimator
// (should really be a command object)
Am_Define_Method(Am_Object_Method, void, Am_Exaggerated_Sub_Do,
                 (Am_Object interp))
{
  Am_Animator_Do.Call(interp);
  next_exaggerated_phase(interp.Get_Owner());
}

////////////////////////////////////////////////////////////
// Am_Visible_Animator
////////////////////////////////////////////////////////////

//
//   This interpolator causes an object to move/grow to a fixed
//   left/top/width/height whenever it goes invisible.  For instance,
//   the object might streak off the screen, or it might shrink to
//   a point.  The opposite animation is performed when the object
//   becomes visible again.
//
//   Also, if attached to the VISIBLE slot of a fade group, this
//   interpolator can cause the fade group to fade in and out when
//   its visibility changes.
//

inline int
imin(int i1, int i2)
{
  if (i1 < i2)
    return i1;
  else
    return i2;
}
inline int
imax(int i1, int i2)
{
  if (i1 > i2)
    return i1;
  else
    return i2;
}

Am_Define_Method(Am_Object_Method, void, Am_Visible_Animator_Start_Do,
                 (Am_Object interp))
{
  Am_INTER_TRACE_PRINT(interp, "Visible Start of " << interp);
  Am_Object owner = interp.Get(Am_OPERATES_ON);

  int left = owner.Get(Am_LEFT);
  int top = owner.Get(Am_TOP);
  int width = owner.Get(Am_WIDTH);
  int height = owner.Get(Am_HEIGHT);

  int invisible_width = width;
  int invisible_height = height;

  Am_Value_List slots;
  Am_Value_List visible_location;
  Am_Value_List invisible_location;

  Am_Value v;

  v = interp.Get(Am_WIDTH);
  if (v.type == Am_INT) {
    slots.Add(Am_WIDTH);
    invisible_location.Add(v);
    invisible_width = v;
    visible_location.Add(width);
  }

  v = interp.Get(Am_HEIGHT);
  if (v.type == Am_INT) {
    slots.Add(Am_HEIGHT);
    invisible_location.Add(v);
    invisible_height = v;
    visible_location.Add(height);
  }

  v = interp.Get(Am_LEFT);
  if (v.type == Am_INT) {
    // clip target left so that it's only as far off the screen as necessary
    //  to make object invisible: i.e., we want -width <= left <= screen.width
    // **FIX:  handle multiple screens
    v = imax(-invisible_width, imin((int)Am_Screen.Get(Am_WIDTH), v));
    slots.Add(Am_LEFT);
    invisible_location.Add(v);
    visible_location.Add(left);
  }
  v = interp.Get(Am_TOP);
  if (v.type == Am_INT) {
    // clip target top so that it's only as far off the screen as necessary
    //  to make object invisible: i.e., we want -height <= top <= screen.height
    // **FIX:  handle multiple screens
    v = imax(-invisible_height, imin(Am_Screen.Get(Am_HEIGHT), v));
    slots.Add(Am_TOP);
    invisible_location.Add(v);
    visible_location.Add(top);
  }

  v = interp.Get(Am_USE_FADING);
  if (v.Valid()) {
    slots.Add(Am_VALUE);
    invisible_location.Add(100);
    visible_location.Add(0);
  }

  // save visible location so we can restore it below
  // FIX: sub_animator should be an animation constraint that
  // watches for external changes to the visible location while the
  // animation is underway.  Presently, external changes are just lost.
  interp.Set(Am_VISIBLE_LOCATION, visible_location);
  interp.Set(Am_INVISIBLE_LOCATION, invisible_location);

  // set up sub animator to do the actual animation
  Am_Object sub_animator = interp.Get(Am_SUB_ANIMATOR);
  sub_animator.Set(Am_SLOT_TO_ANIMATE, slots);
  sub_animator.Set(Am_OPERATES_ON, owner);

  Am_Value final_value = interp.Get(Am_VALUE_2);
  if ((bool)final_value) { //currently invisible.  make it visible
    Am_Start_Animator(sub_animator, (Am_Wrapper *)invisible_location,
                      (Am_Wrapper *)visible_location);

    interp.Set(Am_VALUE, true); //make visible throughout animation

    Am_INTER_TRACE_PRINT(
        interp, "  Animating to visible from "
                    << (Am_Value)(Am_Wrapper *)invisible_location << " to "
                    << (Am_Value)(Am_Wrapper *)visible_location);
  } else { //making invisible
    Am_Start_Animator(sub_animator, (Am_Wrapper *)visible_location,
                      (Am_Wrapper *)invisible_location);

    interp.Set(Am_VALUE, true); //make visible throughout animation

    Am_INTER_TRACE_PRINT(
        interp, "  Animating invisible from "
                    << (Am_Value)(Am_Wrapper *)visible_location << " to "
                    << (Am_Value)(Am_Wrapper *)invisible_location);
  }

  // don't register a timer here!
  // the sub-animator will do all the animating, and then call us
  // back when it's done

  Am_Set_Animated_Slots(interp);
}

Am_Define_Method(Am_Anim_Update_Method, void, Am_Visible_Animator_Interrupt,
                 (Am_Object interp, const Am_Value &new_target))
{
  Am_INTER_TRACE_PRINT(interp, "Visible Interrupt of " << interp);

  if ((bool)interp.Get(Am_INTERRUPTIBLE)) {
    Am_Value old_target = interp.Get(Am_VALUE_2);

    if (old_target == new_target) {
      // interrupted animation to target with another set to the same target;
      // just abort
      Am_Abort_Animator(interp);
    } else {
      // need to reverse course and head back in the other direction,
      // but without sudden jumps in sub-animator
      Am_INTER_TRACE_PRINT(
          interp, "Visible Interrupt changing direction, now animating to "
                      << new_target);

      // abort sub animator first (which leaves it at current position)
      Am_Object sub_animator = interp.Get(Am_SUB_ANIMATOR);
      Am_Abort_Animator(sub_animator);

      // sub-animator should start from its current position (Am_VALUE)
      // and head for either visible or invisible location,
      // depending on new_target
      Am_Start_Animator(sub_animator, sub_animator.Get(Am_VALUE),
                        interp.Get((bool)new_target ? Am_VISIBLE_LOCATION
                                                    : Am_INVISIBLE_LOCATION));

      interp.Set(Am_VALUE_1, old_target);
      interp.Set(Am_VALUE_2, new_target);
      interp.Set(Am_VALUE, true); //make visible throughout animation

      Am_Set_Animated_Slots(interp);
    }
  } else {
    // not interruptible -- pass back to prototype's method

    Am_Anim_Update_Method(Am_Animator.Get(Am_INTERRUPT_METHOD))
        .Call(interp, new_target);
  }
}

Am_Define_Method(Am_Object_Method, void, Am_Visible_Animator_Do,
                 (Am_Object interp))
{
  Am_INTER_TRACE_PRINT(interp, "Visible Stop of " << interp);

  Am_Object sub_animator = interp.Get(Am_SUB_ANIMATOR);

  // restore object's original position
  sub_animator.Set(Am_VALUE, interp.Get(Am_VISIBLE_LOCATION));
  Am_Set_Animated_Slots(sub_animator);

  // and assert the final visibility value
  interp.Set(Am_VALUE, interp.Get(Am_VALUE_2));
  Am_Set_Animated_Slots(interp);
}

Am_Define_Method(Am_Object_Method, void, Am_Visible_Animator_Abort_Do,
                 (Am_Object interp))
{
  Am_INTER_TRACE_PRINT(interp, "Visible Abort of " << interp);

  Am_Object sub_animator = interp.Get(Am_SUB_ANIMATOR);
  Am_Abort_Animator(sub_animator);
  Am_Visible_Animator_Do.Call(interp);
}

// method on subanimator
// should really be a command object
Am_Define_Method(Am_Object_Method, void, Am_Visible_Sub_Do, (Am_Object interp))
{
  Am_Animator_Do.Call(interp);
  Am_Stop_Animator(interp.Get_Owner());
}

////////////////////////////////////////////////////////////
// Visible by Flying Apart
////////////////////////////////////////////////////////////

//flatten out the list of parts
void
add_obj_and_parts(Am_Object &orig, Am_Value_List &new_parts, int &maxobjw,
                  int &maxobjh)
{
  //don't add parts that use the regular aggregate draw method, since
  //it draws all subparts anyway.  This includes regular Maps and Groups
  Am_Draw_Method method = orig.Get(Am_DRAW_METHOD);
  Am_Draw_Method agg_method = Am_Aggregate.Get(Am_DRAW_METHOD);
  Am_Draw_Method fade_method = Am_Fade_Group.Get(Am_DRAW_METHOD);
  Am_Object new_obj;
  if (method != agg_method && method != fade_method) {
    new_obj = orig.Copy_Value_Only();
    new_parts.Add(new_obj);
    int wh = orig.Get(Am_WIDTH);
    if (wh > maxobjw)
      maxobjw = wh;
    wh = orig.Get(Am_HEIGHT);
    if (wh > maxobjh)
      maxobjh = wh;
  }
  Am_Object sub_obj;
  if (Am_Is_Group_Or_Map(orig)) {
    //flatten out the parts
    //can't use graphical_parts list because it is computed by a
    //constraint which has been removed
    if (new_obj.Valid())
      new_obj.Set(Am_GRAPHICAL_PARTS, Am_Value_List()); //make it empty in copy
    Am_Value_List parts = orig.Get(Am_GRAPHICAL_PARTS);
    for (parts.Start(); !parts.Last(); parts.Next()) {
      sub_obj = parts.Get();
      if (sub_obj.Is_Instance_Of(Am_Graphical_Object))
        add_obj_and_parts(sub_obj, new_parts, maxobjw, maxobjh);
    }
  }
}

Am_Define_Method(Am_Object_Method, void, fly_apart_anim_done, (Am_Object cmd))
{
  Am_Object interp = cmd.Get(Am_SAVED_OLD_OBJECT_OWNER);
  int i = (int)interp.Get(Am_WIGGLES) - 1;
  interp.Set(Am_WIGGLES, i);
  //std::cout << " Anim done, cnt = " << i <<std::endl <<std::flush;
  if (i == 0)
    Am_Stop_Animator(interp);
}

//breaks up orig object into its pieces recursively until gets to the
//leaf pieces and makes copies of these.  Returns new list.
//if vis, then fly from offscreen to current location, if !vis, flies
//to offscreen
Am_Value_List
break_up_objects(Am_Object &interp, Am_Object &orig, bool vis)
{
  Am_Value_List new_parts;
  int maxobjw = 0;
  int maxobjh = 0;
  //get flat list of all parts
  add_obj_and_parts(orig, new_parts, maxobjw, maxobjh);
  Am_INTER_TRACE_PRINT(interp, "  New parts = " << new_parts);
  Am_Object win = orig.Get(Am_WINDOW);
  Am_Object part, from_part, animator;
  int num_parts = new_parts.Length();
  int trans_left, trans_top, win_w, win_h, win_center_x, win_center_y, maxwh, x,
      y, left, top;
  win_w = win.Get(Am_WIDTH);
  win_h = win.Get(Am_HEIGHT);
  win_center_x = win_w / 2;
  win_center_y = win_h / 2;
  maxwh = imax(win_w + maxobjw, win_h + maxobjh) / 2 + 1;
  double rads_per_obj = (M_PI * 2) / num_parts;
  double rads = 0.0;
  for (new_parts.Start(); !new_parts.Last();
       new_parts.Next(), rads += rads_per_obj) {
    part = new_parts.Get();
    part.Remove_From_Owner(); //make sure not a part now
    from_part = part.Get(Am_SOURCE_OF_COPY);
    Am_Translate_Coordinates(from_part, 0, 0, win, trans_left, trans_top);
    x = (int)(maxwh * sin(rads));
    y = (int)(maxwh * cos(rads));
    left = win_center_x + x;
    top = win_center_y - y;
    animator = Am_Animator.Create();
    animator.Get_Object(Am_COMMAND)
        .Set(Am_SAVED_OLD_OBJECT_OWNER, interp, Am_OK_IF_NOT_THERE)
        .Set(Am_DO_METHOD, fly_apart_anim_done)
        .Set(Am_ABORT_DO_METHOD, fly_apart_anim_done);
    //set start location
    if (vis) { // then starts off edge
      part.Set(Am_LEFT, left);
      part.Set(Am_TOP, top);
    } else { //starts at object
      part.Set(Am_LEFT, trans_left);
      part.Set(Am_TOP, trans_top);
    }
    win.Add_Part(part);

    part.Set(Am_LEFT, Am_Animate_With(animator));
    part.Set(Am_TOP, Am_Animate_With(animator));
    if (vis) { //then goes to object
      part.Set(Am_LEFT, trans_left);
      part.Set(Am_TOP, trans_top);
    } else { // goes off edge
      part.Set(Am_LEFT, left);
      part.Set(Am_TOP, top);
    }
  }
  return new_parts;
}

void
delete_all_parts(Am_Object &interp)
{
  Am_Value_List new_objs = interp.Get(Am_GRAPHICAL_PARTS);
  //std::cout << "DONE ALL, destroying " << new_objs <<std::endl <<std::flush;
  Am_Object obj;
  for (new_objs.Start(); !new_objs.Last(); new_objs.Next()) {
    obj = new_objs.Get();
    // obj.Destroy(); BUG
    obj.Remove_From_Owner();
  }
}

Am_Define_Method(Am_Object_Method, void, Am_Visible_By_Fly_Apart_Start_Do,
                 (Am_Object interp))
{
  Am_INTER_TRACE_PRINT(interp, "Animator Start of " << interp);

  Am_Value value2;
  value2 = interp.Peek(Am_VALUE_2);
  bool vis = value2.Valid(); //going visible?

  Am_Object owner = interp.Get(Am_OPERATES_ON);
  Am_Value_List new_objs = break_up_objects(interp, owner, vis);
  interp.Set(Am_GRAPHICAL_PARTS, new_objs, Am_OK_IF_NOT_THERE);
  //countdown of number of anims
  interp.Set(Am_WIGGLES, new_objs.Length(), Am_OK_IF_NOT_THERE);

  //whether getting visible or not, be invisible during animation
  interp.Set(Am_VALUE, false);

  // don't register a timer here!
  // the sub-animators will do all the animating, and then call us
  // back when their done

  Am_Set_Animated_Slots(interp);
}

Am_Define_Method(Am_Object_Method, void, Am_Fly_Apart_Animator_Do,
                 (Am_Object interp))
{
  Am_INTER_TRACE_PRINT(interp, "Fly Apart Stop of " << interp);

  // and assert the final visibility value
  interp.Set(Am_VALUE, interp.Get(Am_VALUE_2));
  Am_Set_Animated_Slots(interp);
  delete_all_parts(interp);

  Am_Value value;
  value = interp.Peek(Am_COMMAND);
  call_all_commands(value, Am_DO_METHOD, interp, false);
}

Am_Define_Method(Am_Object_Method, void, Am_Fly_Apart_Animator_Abort_Do,
                 (Am_Object interp))
{
  Am_INTER_TRACE_PRINT(interp, "Fly Apart Abort of " << interp);
  Am_Set_Animated_Slots(interp);
  delete_all_parts(interp);

  Am_Value value;
  value = interp.Peek(Am_COMMAND);
  call_all_commands(value, Am_ABORT_DO_METHOD, interp, false);
}

////////////////////////////////////////////////////////////
// Point Lists
////////////////////////////////////////////////////////////

enum point_list_differences
{
  Am_PL_SAME = 0,
  Am_PL_FROM_NOTHING,
  Am_PL_TO_NOTHING,
  Am_PL_ADD_POINT,
  Am_PL_REMOVE_POINT,
  Am_PL_VERY_DIFFERENT
};

//newp is 1 bigger than old, if all the other points are the same
//value, return the index in newp of the newp point, otherwise return -1
int
find_point_diff(Am_Point_List &old, Am_Point_List &newp)
{
  int diff = -1;
  int index = 0;
  float x1, y1, x2, y2;
  for (old.Start(), newp.Start(); !old.Last();
       old.Next(), newp.Next(), index++) {
    old.Get(x1, y1);
    newp.Get(x2, y2);
    if (x1 != x2 || y1 != y2) {
      if (diff == -1) { //maybe this is the point that is different
        diff = index;
        // std::cout << "different at index " << index <<std::endl <<std::flush;
        newp.Next();
      } else
        return -1; // already found a different one to be different
    }
    //otherwise they are equal, just loop
  }
  if (diff == -1) { //maybe it is the last point of newp
    if (newp.Last())
      Am_ERROR("newp at end after loop " << newp << " old " << old);
    //    std::cout << "different at END, index " << index <<std::endl <<std::flush;
    diff = index;
  }
  return diff;
}

point_list_differences
find_differences(Am_Point_List &old, Am_Point_List &newp, int &index_of_diff)
{
  //  std::cout << "checking differences of " << old << " and " << newp <<std::endl <<std::flush;
  if (old == newp)
    return Am_PL_SAME;
  int length_old = old.Length();
  int length_newp = newp.Length();
  if (length_old == 0)
    return Am_PL_FROM_NOTHING;
  if (length_newp == 0)
    return Am_PL_TO_NOTHING;
  if (length_newp == length_old + 1) {
    //might be new point, see if can find the index of the point that is new
    index_of_diff = find_point_diff(old, newp);
    if (index_of_diff >= 0)
      return Am_PL_ADD_POINT;
  } else if (length_newp == length_old - 1) {
    //might be delete point, see if can find the index of the point that is new
    index_of_diff = find_point_diff(newp, old);
    if (index_of_diff >= 0)
      return Am_PL_REMOVE_POINT;
  }
  return Am_PL_VERY_DIFFERENT;
}

//create a new point list the same length as ref with all the points
//at the center of ref
Am_Point_List
create_list_at_center(Am_Point_List &ref)
{
  int min_x, min_y, max_x, max_y;
  ref.Get_Extents(min_x, min_y, max_x, max_y);
  float centerX = static_cast<float>(min_x + ((max_x - min_x) / 2.0));
  float centerY = static_cast<float>(min_y + ((max_y - min_y) / 2.0));
  Am_Point_List newl;
  int new_length = ref.Length();
  for (int i = 0; i < new_length; i++) {
    newl.Add(centerX, centerY);
  }
  //  std::cout << "Created new list length " << new_length << " of (" << centerX
  //        << "," << centerY << " = " << newl <<std::endl <<std::flush;

  return newl;
}

Am_Point_List
add_point_to(Am_Point_List &old, int changed_point)
{
  Am_Point_List newl = old;
  newl.Start();
  float x1, y1, x2, y2;
  if (changed_point == 0) {
    newl.Get(x1, y1);
    newl.Insert(x1, y1, Am_BEFORE);
    //    std::cout << "Changed point is first, adding (" << x1 << "," << y1
    // 	 << ") at beginning = " << newl <<std::endl <<std::flush;
  } else {
    for (int i = 0; i < changed_point - 1; i++) {
      newl.Next();
    }
    newl.Get(x1, y1);
    if (newl.Last()) {
      newl.Insert(x1, y1, Am_AFTER);
      //      std::cout << "Changed point is last, adding (" << x1 << "," << y1
      // 	 << ") at end = " << newl <<std::endl <<std::flush;
    } else {
      newl.Next();
      newl.Get(x2, y2);
      newl.Insert((x1 + x2) / 2, (y1 + y2) / 2, Am_BEFORE);
      //      std::cout << "Changed point is at " << changed_point << ", adding new point ("
      // 	   << (x1+x2)/2 << "," << (y1+y2)/2 << ") from 1=("
      // 	   << x1 << "," << y1 << ") 2=("
      // 	   << x2 << "," << y2 << ") = " << newl <<std::endl <<std::flush;
    }
  }
  return newl;
}

Am_Point_List
add_points_at_start_to(Am_Point_List &old, int num_needed)
{
  Am_Point_List newl = old;
  newl.Start();
  float x1, y1;
  newl.Get(x1, y1);
  for (int i = 0; i < num_needed; i++) {
    newl.Add(x1, y1, Am_HEAD);
  }
  //  std::cout << "Adding " << num_needed << " points (" << x1 << "," << y1
  //        << ") at beginning to get " << newl <<std::endl <<std::flush;
  return newl;
}

//given original lists old and new which differ as diff, create new
//lists with the same number of points in each
void
make_new_old_and_new_lists(Am_Point_List &old, Am_Point_List &newp,
                           point_list_differences diff, int changed_point,
                           Am_Point_List &new_old, Am_Point_List &new_newp)
{
  switch (diff) {
  case Am_PL_SAME:
    // Am_Error("Lists are the same");
    new_old = old;
    new_newp = newp;
    break;
  case Am_PL_FROM_NOTHING:
    new_newp = newp;
    new_old = create_list_at_center(newp);
    break;
  case Am_PL_TO_NOTHING:
    new_old = old;
    new_newp = create_list_at_center(old);
    break;
  case Am_PL_ADD_POINT:
    new_old = add_point_to(old, changed_point);
    new_newp = newp;
    break;
  case Am_PL_REMOVE_POINT:
    new_newp = add_point_to(newp, changed_point);
    new_old = old;
    break;
  case Am_PL_VERY_DIFFERENT: {
    int len_old = old.Length();
    int len_newp = newp.Length();
    int num_needed = len_old - len_newp;
    if (num_needed < 0) {
      new_newp = newp;
      new_old = add_points_at_start_to(old, -num_needed);
    } else if (num_needed > 0) {
      new_old = old;
      new_newp = add_points_at_start_to(newp, num_needed);
    } else {
      new_newp = newp;
      new_old = old;
    }
    break;
  }
  } //switch
}

Am_Define_Method(Am_Path_Function, Am_Value, Am_Point_List_Path,
                 (Am_Object interp, Am_Value /* value1 */,
                  Am_Value /* value2 */, float tau))
{
  // ignore value1 and value2 since will be the original values not
  // using the new point lists, so get them directly
  Am_Point_List p1 = interp.Get(Am_POINT_VALUE_1, Am_NO_DEPENDENCY);
  Am_Point_List p2 = interp.Get(Am_POINT_VALUE_2, Am_NO_DEPENDENCY);

  float x1, y1, x2, y2;
  if (tau == Am_COMPUTE_DISTANCE) {
    float dx, dy, this_dist;
    float dist = 0.0;
    // return magnitude of distance between value1 and value2
    for (p1.Start(), p2.Start(); !p1.Last(); p1.Next(), p2.Next()) {
      p1.Get(x1, y1);
      p2.Get(x2, y2);
      dx = x1 - x2;
      dy = y1 - y2;
      this_dist = dx * dx + dy * dy;
      if (this_dist > dist)
        dist = this_dist;
    }
    dist = static_cast<float>(sqrt(dist));
    //    std::cout << "Computing distance " << dist << " from " << p1 << " to " << p2
    // 	 <<std::endl <<std::flush;
    return dist;
  } else {
    Am_Point_List return_val;
    float new_x, new_y;
    for (p1.Start(), p2.Start(); !p1.Last(); p1.Next(), p2.Next()) {
      p1.Get(x1, y1);
      p2.Get(x2, y2);
      new_x = x1 * (1 - tau) + x2 * tau;
      new_y = y1 * (1 - tau) + y2 * tau;
      return_val.Add(new_x, new_y);
    }
    Am_Value v;
    v = return_val;
    return v;
  }
}

Am_Define_Point_List_Formula(compute_point_lists)
{
  //see what kind of animation to do
  int changed_point = 0;
  Am_Point_List old, newp, new_old, new_newp;
  Am_Value old_value, new_value;
  old_value = self.Peek(Am_VALUE_1);
  new_value = self.Peek(Am_VALUE_2);
  if (old_value.Valid()) { //invalid is OK (use empty list)
    if (Am_Point_List::Test(old_value))
      old = old_value;
    else
      Am_ERROR("Old value not a list, = " << old_value);
  }
  if (new_value.Valid()) { //invalid is OK (use empty list)
    if (Am_Point_List::Test(new_value))
      newp = new_value;
    else
      Am_ERROR("New value not a list, = " << new_value);
  }
  point_list_differences diff = find_differences(old, newp, changed_point);

  make_new_old_and_new_lists(old, newp, diff, changed_point, new_old, new_newp);

  //  std::cout << "--Modified val1 = " << new_old << " val2 = " << new_newp
  //        <<std::endl <<std::flush;

  self.Set(Am_POINT_VALUE_2, new_newp);
  return new_old;
}

////////////////////////////////////////////////////////////
// Standard termination commands
////////////////////////////////////////////////////////////

// for Am_Animation_Wrap_Command
Am_Define_Method(Am_Object_Method, void, wrap_do, (Am_Object cmd))
{
  Am_Object interp = cmd.Get(Am_SAVED_OLD_OWNER);
  Am_Value_List values = interp.Get(Am_VALUES);
  Am_Value new_v1;
  Am_Value new_v2;

  if (values.Valid()) {
    // multi-point path -- return to phase 0
    interp.Set(Am_CURRENT_PHASE, 0);
    values.Start();
    new_v1 = values.Get();
    values.Next();
    new_v2 = values.Get();
  } else {
    // just a two-point path
    new_v1 = interp.Get(Am_VALUE_1);
    new_v2 = interp.Get(Am_VALUE_2);
  }

  Am_Start_Animator(interp, new_v1, new_v2);
}

static Am_Value_List
reverse(Am_Value_List lst)
{
  Am_Value_List rev;
  for (lst.Start(); !lst.Last(); lst.Next())
    rev.Add(lst.Get());
  return rev;
}

Am_Define_Method(Am_Object_Method, void, bounce_do, (Am_Object cmd))
{
  Am_Object interp = cmd.Get(Am_SAVED_OLD_OWNER);
  Am_Value_List values = interp.Get(Am_VALUES);
  Am_Value new_v1;
  Am_Value new_v2;

  if (values.Valid()) {
    // multi-point path -- return to phase 0, but reverse the list
    interp.Set(Am_CURRENT_PHASE, 0);

    values = reverse(values);
    interp.Set(Am_VALUES, values);

    values.Start();
    new_v1 = values.Get();
    values.Next();
    new_v2 = values.Get();
  } else {
    // just a two-point path: reverse the points
    new_v2 = interp.Get(Am_VALUE_1);
    new_v1 = interp.Get(Am_VALUE_2);
  }

  Am_Start_Animator(interp, new_v1, new_v2);
}

Am_Define_Method(Am_Object_Method, void, discard_intermediate_values,
                 (Am_Object cmd))
{
  Am_Object interp = cmd.Get_Owner();
  interp.Set(Am_VALUES, Am_No_Value_List);
}

////////////////////////////////////////////////////////////
// Global variables & functions
////////////////////////////////////////////////////////////

Am_Object Am_Animator;
Am_Object Am_Stepping_Animator;
Am_Object Am_Style_Animator;
Am_Object Am_Exaggerated_Animator;
Am_Object Am_Visible_Animator;
Am_Object Am_Blink_Animator;
Am_Object Am_Through_List_Animator;
Am_Object Am_Object_In_Owner_Animator;
Am_Object Am_Point_List_Animator;
Am_Object Am_Fly_Apart_Animator;

Am_Object Am_Animation_Wrap_Command;
Am_Object Am_Animation_Bounce_Command;

static void
init()
{
#ifdef DEBUG
  Am_Register_Slot_Key(Am_ANIM_UPDATE_METHOD, "ANIM_UPDATE_METHOD");
  Am_Register_Slot_Key(Am_INTERRUPT_METHOD, "INTERRUPT_METHOD");
  Am_Register_Slot_Key(Am_ANIM_CONSTRAINT, "ANIM_CONSTRAINT");
  Am_Register_Slot_Key(Am_RUNNING, "~RUNNING~");
  Am_Register_Slot_Key(Am_PATH_LENGTH, "PATH_LENGTH");
  Am_Register_Slot_Key(Am_VALUES, "VALUES");
  Am_Register_Slot_Key(Am_CURRENT_PHASE, "CURRENT_PHASE");
  Am_Register_Slot_Key(Am_INTERRUPTIBLE, "INTERRUPTIBLE");

  Am_Register_Slot_Key(Am_SUB_ANIMATOR, "SUB_ANIMATOR");
  Am_Register_Slot_Key(Am_VELOCITY, "VELOCITY");
  Am_Register_Slot_Key(Am_CURRENT_VELOCITY, "CURRENT_VELOCITY");
  Am_Register_Slot_Key(Am_DURATION, "DURATION");
  Am_Register_Slot_Key(Am_CURRENT_DURATION, "CURRENT_DURATION");
  Am_Register_Slot_Key(Am_TIMING_MODE, "~TIMING_MODE~");
  Am_Register_Slot_Key(Am_MIN_REPEAT_DELAY, "MIN_REPEAT_DELAY");
  Am_Register_Slot_Key(Am_TIMING_FUNCTION, "TIMING_FUNCTION");
  Am_Register_Slot_Key(Am_PATH_FUNCTION, "PATH_FUNCTION");
  Am_Register_Slot_Key(Am_INITIAL_DELAY, "INITIAL_DELAY");
  Am_Register_Slot_Key(Am_SHARPNESS_1, "SHARPNESS_1");
  Am_Register_Slot_Key(Am_SHARPNESS_2, "SHARPNESS_2");
  Am_Register_Slot_Key(Am_RGB_COLOR, "RGB_COLOR");
  Am_Register_Slot_Key(Am_WINDUP_DELAY, "WINDUP_DELAY");
  Am_Register_Slot_Key(Am_WINDUP_AMOUNT, "WINDUP_AMOUNT");
  Am_Register_Slot_Key(Am_WIGGLE_DELAY, "WIGGLE_DELAY");
  Am_Register_Slot_Key(Am_WIGGLE_AMOUNT, "WIGGLE_AMOUNT");
  Am_Register_Slot_Key(Am_WIGGLES, "WIGGLES");
  Am_Register_Slot_Key(Am_EXAGGERATOR_STATE, "~EXAGGERATOR_STATE~");
  Am_Register_Slot_Key(Am_VISIBLE_LOCATION, "VISIBLE_LOCATION");
  Am_Register_Slot_Key(Am_INVISIBLE_LOCATION, "INVISIBLE_LOCATION");
  Am_Register_Slot_Key(Am_POINT_VALUE_1, "~POINT_VALUE_1~");
  Am_Register_Slot_Key(Am_POINT_VALUE_2, "~POINT_VALUE_2~");
  Am_Register_Slot_Key(Am_CURRENT_PATH, "~CURRENT_PATH~");
  Am_Register_Slot_Key(Am_LIST_OF_VALUES, "LIST_OF_VALUES");
#endif

  Am_Web timing_web(timing_web_create, timing_web_init, timing_web_validate);

  Am_Animator =
      Am_Root_Object
          .Create(DSTR("Am_Animator"))
          // internal methods
          .Add(Am_ANIM_CONSTRAINT, 0)
          .Add(Am_SLOT_TO_ANIMATE, 0)
          .Add(Am_OPERATES_ON, 0)

          // standard methods
          .Add(Am_START_DO_METHOD, Am_Animator_Start_Do)
          .Add(Am_INTERRUPT_METHOD, Am_Animator_Interrupt)
          .Add(Am_ABORT_DO_METHOD, Am_Animator_Abort_Do)
          .Add(Am_ANIMATION_METHOD, Am_Animator_Animation)
          .Add(Am_INTERIM_DO_METHOD, Am_Animator_Interim_Do)
          .Add(Am_DO_METHOD, Am_Animator_Do)

          .Add(Am_RUNNING, false)
          .Add(Am_ACTIVE, true)
          .Add(Am_ELAPSED_TIME, Am_Time((unsigned long)0))
          .Add(Am_VALUE, Am_No_Value)
          .Add(Am_VALUE_1, Am_No_Value)
          .Add(Am_VALUE_2, Am_No_Value)
          .Add(Am_VALUES, Am_No_Value_List)
          .Add(Am_CURRENT_PHASE, 0)
          .Add(Am_INTERRUPTIBLE, true)

          // standard parameters
          .Add(Am_PATH_FUNCTION, Am_Linear_Path)
          .Add(Am_PATH_LENGTH, compute_path_length)

          .Add(Am_TIMING_MODE, Am_VELOCITY)
          .Add(Am_VELOCITY, 200) // fixed velocity of 200 values per sec
          .Add(Am_DURATION, Am_Time((unsigned long)0)) // derived duration
          .Add(Am_CURRENT_VELOCITY, timing_web)
          .Add(Am_CURRENT_DURATION, 0)

          .Add(Am_REPEAT_DELAY, compute_repeat_delay)
          .Add(Am_MIN_REPEAT_DELAY, Am_Time(25)) // at least 25 ms per frame
          // (so no more than 40 frames per sec)

          .Add(Am_TIMING_FUNCTION, Am_Linear_Timing)

          .Add_Part(Am_COMMAND, Am_Command.Create(DSTR("Am_Animator_Command")));

  Am_Stepping_Animator =
      Am_Animator.Create(DSTR("Am_Stepping_Animator"))
          .Set(Am_ANIMATION_METHOD, Am_Stepping_Animator_Animation)
          .Add(Am_SMALL_INCREMENT, 5);

  Am_Style_Animator = Am_Animator.Create(DSTR("Am_Style_Animator"))
                          .Set(Am_PATH_FUNCTION, Am_Style_Path)
                          .Add(Am_RGB_COLOR, true);

  Am_Exaggerated_Animator =
      Am_Animator.Create(DSTR("Am_Exaggerated_Animator"))
          .Set(Am_START_DO_METHOD, Am_Exaggerated_Animator_Start_Do)
          //.Set (Am_INTERRUPT_METHOD, Am_Exaggerated_Animator_Interrupt)
          .Set(Am_ABORT_DO_METHOD, Am_Exaggerated_Animator_Abort_Do)
          .Set(Am_INTERIM_DO_METHOD, Am_Exaggerated_Animator_Interim_Do)
          .Set(Am_DO_METHOD, Am_Exaggerated_Animator_Do)
          .Add(Am_WINDUP_DELAY, Am_Time(200))
          .Add(Am_WINDUP_AMOUNT, 5)
          .Add(Am_WIGGLE_DELAY, Am_Time(100))
          .Add(Am_WIGGLE_AMOUNT, 2)
          .Add(Am_WIGGLES, 4)
          .Add(Am_EXAGGERATOR_STATE, Am_EXAGGERATOR_IDLE)
          .Add_Part(
              Am_SUB_ANIMATOR,
              Am_Animator.Create(DSTR("Am_Exaggerated_Sub_Animator"))
                  .Set(Am_DO_METHOD, Am_Exaggerated_Sub_Do)
                  .Set(Am_SLOT_TO_ANIMATE, Am_From_Owner(Am_SLOT_TO_ANIMATE))
                  .Set(Am_OPERATES_ON, Am_From_Owner(Am_OPERATES_ON)))
          .Set(Am_VALUE,
               Am_From_Part(Am_SUB_ANIMATOR, Am_VALUE).Multi_Constraint());

  Am_Visible_Animator =
      Am_Animator.Create(DSTR("Am_Visible_Animator"))
          .Set(Am_START_DO_METHOD, Am_Visible_Animator_Start_Do)
          .Set(Am_INTERRUPT_METHOD, Am_Visible_Animator_Interrupt)
          .Set(Am_ABORT_DO_METHOD, Am_Visible_Animator_Abort_Do)
          .Set(Am_DO_METHOD, Am_Visible_Animator_Do)
          .Set(Am_PATH_FUNCTION, Am_Step_Path)
          .Add_Part(Am_SUB_ANIMATOR,
                    Am_Animator.Create(DSTR("Am_Visible_Sub_Animator"))
                        .Set(Am_DO_METHOD, Am_Visible_Sub_Do)
                        .Set(Am_DURATION, Am_Time(500)))
          .Add(Am_LEFT, Am_No_Value)
          .Add(Am_TOP, Am_No_Value)
          .Add(Am_WIDTH, Am_No_Value)
          .Add(Am_HEIGHT, Am_No_Value)
          .Add(Am_USE_FADING, false)
          .Add(Am_VISIBLE_LOCATION, 0)
          .Add(Am_INVISIBLE_LOCATION, 0);

  Am_Fly_Apart_Animator =
      Am_Animator.Create(DSTR("Am_Fly_Apart_Animator"))
          .Set(Am_START_DO_METHOD, Am_Visible_By_Fly_Apart_Start_Do)
          .Set(Am_ABORT_DO_METHOD, Am_Fly_Apart_Animator_Abort_Do)
          .Set(Am_DO_METHOD, Am_Fly_Apart_Animator_Do)
          .Set(Am_PATH_FUNCTION, Am_Step_Path);

  Am_Blink_Animator = Am_Animator.Create(DSTR("Am_Blink_Animator"))
                          .Set(Am_PATH_FUNCTION, Am_Step_Path)
                          .Set(Am_ANIMATION_METHOD, Am_Blink_Animator_Animation)
                          .Set(Am_DURATION, Am_Time(500))
                          .Set(Am_VALUE_1, true)
                          .Set(Am_VALUE_2, false);

  Am_Through_List_Animator =
      Am_Animator.Create(DSTR("Am_Through_List_Animator"))
          .Set(Am_PATH_FUNCTION, Am_Through_List_Path)
          .Add(Am_LIST_OF_VALUES, 0)
          .Add(Am_CURRENT_PATH, compute_list_path)
          .Set(Am_PATH_LENGTH, compute_list_path_length)
          .Set(Am_VELOCITY, 10) // i.e., 10 transitions per second
      ;

  Am_Object_In_Owner_Animator =
      Am_Animator.Create(DSTR("Am_Object_In_Owner_Animator"));

  Am_Point_List_Animator = Am_Animator.Create(DSTR("Am_Point_List_Animator"))
                               .Add(Am_POINT_VALUE_1, compute_point_lists)
                               .Add(Am_POINT_VALUE_2, 0)
#if 0
		.Set(Am_START_DO_METHOD, Am_Point_List_Animator_Start_Do)
#endif
                               .Set(Am_PATH_FUNCTION, Am_Point_List_Path);

  Am_Animation_Wrap_Command =
      Am_Command.Create(DSTR("Am_Animation_Wrap_Command"))
          .Set(Am_DO_METHOD, wrap_do)
          .Set(Am_ABORT_DO_METHOD, discard_intermediate_values);

  Am_Animation_Bounce_Command =
      Am_Command.Create(DSTR("Am_Animation_Wrap_Command"))
          .Set(Am_DO_METHOD, bounce_do)
          .Set(Am_ABORT_DO_METHOD, discard_intermediate_values);
}

static void
cleanup()
{
  Am_Fly_Apart_Animator.Destroy();
  Am_Animator.Destroy();
}

static Am_Initializer *initializer =
    new Am_Initializer(DSTR("Anim_Constraint"), init, 4.2f, 101, cleanup);

//lint +e641 re-enable enum as int warning
