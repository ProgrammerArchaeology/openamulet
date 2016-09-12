// Timing event stuff

// registers a periodic timer event.  Each time the event times out, opal
// will call the method stored in slot method_slot of object obj.
// if once, the event is only called once.  Otherwise it's called repeatedly
// until the timer event is removed with Am_Stop_Timer.
void Am_Register_Timer(Am_Time wait_time, Am_Object obj,
                       Am_Slot_Key method_slot, bool once);
// All timer events in Opal's list with a matching object and slot are
// removed from the timer list.
void Am_Stop_Timer(Am_Object obj, Am_Slot_Key slot);

//resets the start time associated with this obj and slot to be NOW.
void Am_Reset_Timer_Start(Am_Object obj, Am_Slot_Key slot);

extern enum Am_Timer_State {
  Am_TIMERS_RUNNING,
  Am_TIMERS_SUSPENDED,
  Am_TIMERS_SINGLE_STEP
} Am_Global_Timer_State;

void Am_Set_Timer_State(Am_Timer_State new_state);
