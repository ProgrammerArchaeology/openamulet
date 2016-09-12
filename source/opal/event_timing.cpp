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

// Timer objects.
// Event handling: Am_Do_Events, Am_Main_Event_Loop, Am_Exit_Main_Event_Loop,
//		   Am_Wait_For_Event

#include <am_inc.h>
#include AM_IO__H
#include <amulet/impl/types_logging.h>

#include <amulet/impl/am_object.h>
#include <amulet/impl/am_drawonable.h>
#include <amulet/impl/gem_misc.h>
#include <amulet/impl/method_timer.h>
#include <amulet/impl/opal_op.h>
#include <amulet/impl/am_time.h>
#include <amulet/impl/timers.h>
#include VALUE_LIST__H

//
// Timing stuff
//

AM_DEFINE_METHOD_TYPE_IMPL(Am_Timer_Method);

Am_Timer_State Am_Global_Timer_State = Am_TIMERS_RUNNING;

// am_clock(): returns current time-of-day unless all timers are suspended,
// in which case it returns the moment of suspension
static inline Am_Time
am_clock()
{
  static Am_Time clock;

  if (Am_Global_Timer_State != Am_TIMERS_SUSPENDED)
    clock = Am_Time::Now();

  return clock;
}

// This stores the information required to take care of the animation
// interactors' timing events.  gdefs.h defines Am_Time.

class Am_Interactor_Time_Event
{
public:
  Am_Interactor_Time_Event(Am_Time new_delta, Am_Object obj, Am_Slot_Key key,
                           bool once)
  {
    delta = new_delta;
    object = obj;
    method_slot = key;
    only_once = once;
    start_time = am_clock();
    next_timeout = start_time + delta;
  }
  Am_Object object;        // object to call timeout method on
  Am_Slot_Key method_slot; // slot key containing timeout method
  bool only_once;          // if true, only time out once
  Am_Time delta;           // time between events
  Am_Time next_timeout;    // next time it should time out
  Am_Time start_time; // time when this event was added to timing event list.
};

static Am_Value_List am_timing_events;

static void
am_add_timing_event(Am_Interactor_Time_Event *new_event)
{
  Am_Interactor_Time_Event *e;
  Am_Time new_time = new_event->next_timeout;
  for (am_timing_events.Start(); !am_timing_events.Last();
       am_timing_events.Next()) {
    e = (Am_Interactor_Time_Event *)(Am_Ptr)(am_timing_events.Get());
    if (e->next_timeout > new_time) {
      am_timing_events.Insert((Am_Ptr)new_event, Am_BEFORE);
      return;
    }
  } // end for: if we get here, add to end of list.
  am_timing_events.Add((Am_Ptr)new_event, Am_TAIL);
}

void
Am_Register_Timer(Am_Time wait_time, Am_Object obj, Am_Slot_Key method_slot,
                  bool once)
{
  Am_Interactor_Time_Event *new_event =
      new Am_Interactor_Time_Event(wait_time, obj, method_slot, once);
  am_add_timing_event(new_event);
}

// deletes the first timer event which matches obj and slot.
void
Am_Stop_Timer(Am_Object obj, Am_Slot_Key slot)
{
  Am_Interactor_Time_Event *e;
  for (am_timing_events.Start(); !am_timing_events.Last();
       am_timing_events.Next()) {
    e = (Am_Interactor_Time_Event *)(Am_Ptr)am_timing_events.Get();
    if (e->object == obj && e->method_slot == slot) {
      am_timing_events.Delete();
      delete e;
      return;
    }
  }
  //Am_ERROR("Am_Stop_Timer on " << obj << " slot " << slot
  //     << " but hasn't been registered")
}

void
Am_Reset_Timer_Start(Am_Object obj, Am_Slot_Key slot)
{
  Am_Interactor_Time_Event *e;
  for (am_timing_events.Start(); !am_timing_events.Last();
       am_timing_events.Next()) {
    e = (Am_Interactor_Time_Event *)(Am_Ptr)am_timing_events.Get();
    if (e->object == obj && e->method_slot == slot) {
      e->start_time = am_clock();
      return;
    }
  }
  Am_ERROR("Am_Reset_Timer_Start on " << obj << " slot " << slot
                                      << " but hasn't been registered")
}

// am_Handle_Timing_Events calls the methods for any timing events that have
// expired, and returns the deadline of the next event in the
// queue.  If a long period of time has passed since the last time we
// handled timing events, the same event might have timed out many times in
// the mean time.  We make sure to call the method once for every time it
// timed out, to make sure the method is always called the correct number
// of times in a given time period.
//
// Returns the deadline of the next timing event.
// If no timing events are active, returns time Zero.
//
static Am_Time
am_Handle_Timing_Events()
{
  bool ticked = false;
  Am_Time return_time; // next deadline
  if (Am_Global_Timer_State != Am_TIMERS_SUSPENDED &&
      !am_timing_events.Empty()) {
    Am_Timer_Method method;
    Am_Time next_timeout_time;
    Am_Time now = am_clock();
    Am_Interactor_Time_Event *next_event;
    Am_Time elapsed_time;

    Am_Object method_object;
    Am_Slot_Key method_slot;
    Am_Time inter_start_time;
    while (true) { // stop when the next event is not past
      am_timing_events.Start();
      if (am_timing_events.Empty()) {
        //deleted the last event
        return return_time; // will be uninitialized
      }
      next_event = (Am_Interactor_Time_Event *)(Am_Ptr)am_timing_events.Get();
      if (next_event->next_timeout < now) {
        // Deal with moving the timing event in its queue _before_ we
        // call the method.  The method might call Am_Stop_Timer,
        // removing this event from the queue, which deletes it.
        next_event->next_timeout += next_event->delta;
        am_timing_events.Delete();
        method_object = next_event->object;
        method_slot = next_event->method_slot;
        inter_start_time = next_event->start_time;
        if (next_event->only_once)
          delete next_event;
        else
          am_add_timing_event(next_event);
        if (method_object.Valid()) {
          //  Call the method for this object
          method = method_object.Get(method_slot);
          if (method.Valid()) {
            elapsed_time = now - inter_start_time;
            method.Call(method_object, elapsed_time);
          }
        }
        ticked = true;
      } else
        break;
    }

    if (ticked && Am_Global_Timer_State == Am_TIMERS_SINGLE_STEP)
      Am_Set_Timer_State(Am_TIMERS_SUSPENDED);

    return_time = next_event->next_timeout;
  }
  return return_time;
}

#if 0
// no longer necessary, now that Process_Event() takes a deadline rather
// than a timeout duration
static void am_Adjust_Timeout (Am_Time& timeout)
{
  if (!timeout.Zero()) {
    //time may have passed since the previous call to am_clock()
    timeout = timeout - am_clock();
    // Hack:
    // if we were _almost_ going to time out at the next_timeout.Is_Past()
    // test above, but failed, your timeout could be negative here.
    // Make it 1ms instead: 0ms will cause Unix gem to treat it as a
    // single screen, no timeout case, which is also bad.
    // Bug: there's no difference between invalid time and 0 time
    // Another solution would be to simply not crash on select errors
    // in Unix.  Ideally we want to have the Right Answer, but we can't
    // do all of this work atomically so we're doomed to occasional failure.
    if (timeout < Am_Time()) {
      timeout = Am_Time(1);
     std::cerr << "am_Adjust_Timeout: negative timeout.\n";
    }
  }
}
#endif

void
Am_Set_Timer_State(Am_Timer_State new_state)
{
  Am_Time before = am_clock();
  Am_Global_Timer_State = new_state;
  Am_Time after = am_clock();

  if (before != after) {
    // a discontinuity in the clock (caused by suspending and then resuming);
    // translate every timestamp on the queue to the new timeline
    Am_Time diff = after - before;

    Am_Interactor_Time_Event *next_event;
    Am_Value_List events = am_timing_events;
    for (events.Start(); !events.Last(); events.Next()) {
      next_event = (Am_Interactor_Time_Event *)(Am_Ptr)events.Get();
      next_event->next_timeout = next_event->next_timeout + diff;
      next_event->start_time = next_event->start_time + diff;
    }
  }
}

bool
Am_Do_Events(bool wait)
{
  am_Handle_Timing_Events();
  Am_Update_All();
  if (wait) {
    Am_Time deadline = Am_Time::Now() + 100UL; // 100 milliseconds from now
    Am_Drawonable::Process_Event(deadline);
  } else
    Am_Drawonable::Process_Immediate_Event();
  Am_Update_All();
  return Am_Main_Loop_Go;
}

void
Am_Main_Event_Loop()
{
  //// TODO: make exit when no windows are owned. (visible?)
  Am_Time deadline;
  while (Am_Main_Loop_Go) {
    deadline = am_Handle_Timing_Events();
    Am_Update_All();
    Am_Drawonable::Process_Event(deadline);
  }
}

void
Am_Exit_Main_Event_Loop()
{
  Am_Main_Loop_Go = false;
}

void
Am_Wait_For_Event()
{
  Am_Drawonable::Wait_For_Event();
}
