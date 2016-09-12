/*
  Implemented by Alan Ferrency, September 1995.
  Amulet's Object system provides a hook to a user (in this case, developer)
  provided routine that will be called every time any slot anywhere is set.
  This file provides slot set tracing capabilities, using this facility.
  Unfortunately, its hooks into the Inspector are very hardwired at this
  point.
  The tracing system keeps a list of object/slot/value combinations that
  are being traced.  Every time a slot is set, it searches through this list
  to see if the slot being set matches some combination that's being
  traced.  If it matches, that slot set is "traced."  Tracing a slot set
  consists of writing a message to cout, and changing the value in the
  inspector window, if it's open.  A slot can also have "store old values"
  mode on.  In this case, tracing does not report to cout, but the value is
  added to a list in the tracer, and a routine is called to update the
  inspector display.
 */

#include <am_inc.h>

#include <amulet/object.h>
#include <amulet/debugger.h>
#include <amulet/value_list.h>
#include <amulet/object_advanced.h>
#include <amulet/standard_slots.h>
#include <amulet/am_strstream.h>
#include <amulet/misc.h>

class Am_Slot_Tracer
{
  // these are the conditions we're looking for, shouldn't be
  // set anywhere except in the constructor.
  Am_Object object;
  Am_Slot_Key key;
  Am_Value value;
  bool object_tracer; //if true, then tracing object creation and destroying
  bool method_tracer; //if true, then tracing method calls

  bool store_old_values;
  Am_Trace_Status trace_status;
  Am_Value_List old_values;
  Am_Value current_value;

  // an inspector string object whose Am_TEXT slot we should set when the
  // value is set.
  Am_Object string_object;

public:
  Am_Slot_Tracer(Am_Object new_obj, Am_Slot_Key new_key, Am_Value new_val,
                 bool store_old, Am_Value first_value = (0L),
                 bool object_tracing = false, bool method_tracing = false);
  ~Am_Slot_Tracer();

  // these test to see if one tracer contains another.  We will use them
  // instead of "==" because it's not a reflexive operator.
  // For example, {(0L), Am_LEFT, Am_None} > {my_window, Am_LEFT, Am_None}
  // but they're not equal, and certainly  <  is false here.
  // Basically a > b implies that the object, key, and value (if any)
  // specified in b are encompassed by those specified in a.  A null object,
  // key, or value encompasses any object, key, or value.
  bool operator>(Am_Slot_Tracer &test_value);
  bool operator<(Am_Slot_Tracer &test_value) { return test_value > *this; };

  // operator== is used only to remove slot tracers from the list.
  // exact match only, including store_old part.
  bool operator==(Am_Slot_Tracer &test_value);

  // these are called when an event happens
  void Trace(Am_Value v, Am_Set_Reason reason);
  void Trace_Object(const Am_Object &old_object, const Am_Object &new_object,
                    Am_Set_Reason reason);
  void Trace_Method(const Am_Object &object, Am_Slot_Key key,
                    Am_Value method_value);

  // turn tracing on and off for one tracer object
  void Start_Tracing() { trace_status |= Am_TRACING; };
  void Stop_Tracing() { trace_status &= (~Am_TRACING); };

  void Start_Breaking() { trace_status |= Am_BREAKING; };
  void Stop_Breaking() { trace_status &= (~Am_BREAKING); };

  void Set_Value_Object(Am_Object obj) { string_object = obj; };
  Am_Value_List Get_Old_Values() { return old_values; };
  Am_Trace_Status Trace_Status() { return trace_status; };
};

Am_Value_List am_all_tracers;

bool
Am_Slot_Tracer::operator==(Am_Slot_Tracer &test_value)
{
  return (object == test_value.object) && (key == test_value.key) &&
         (value == test_value.value) &&
         store_old_values == test_value.store_old_values &&
         object_tracer == test_value.object_tracer &&
         method_tracer == test_value.method_tracer;
}

static const char *
string_reason(Am_Set_Reason reason)
{
#ifdef DEBUG
  switch (reason) {
  case Am_TRACE_NOT_SPECIFIED:
    return "no reason specified.";
  case Am_TRACE_SLOT_SET:
    return "the slot was set.";
  case Am_TRACE_CONSTRAINT_FETCH:
    return "a constraint was evaluated.";
  case Am_TRACE_SLOT_DESTROY:
    return "the slot was destroyed.";
  case Am_TRACE_INHERITANCE_PROPAGATION:
    return "the slot inherited a value.";
  default:
    return "invalid reason.";
  }
#else
  return (0L);
#endif
}

/////
// trace utility routines
/////

static void
destroy_all_tracers()
{
  for (am_all_tracers.Start(); !am_all_tracers.Empty(); am_all_tracers.Delete())
    delete (Am_Slot_Tracer *)(Am_Ptr)am_all_tracers.Get();
}

// am I tracing this particular slot and object?  Only for exact matches.
Am_Slot_Tracer *
Am_Tracing(Am_Object obj, Am_Slot_Key key, bool storing)
{
  Am_Slot_Tracer slot_looking_for(obj, key, Am_Value(), storing);
  Am_Slot_Tracer *this_slot;
  for (am_all_tracers.Start(); !am_all_tracers.Last(); am_all_tracers.Next()) {
    this_slot = (Am_Slot_Tracer *)(Am_Ptr)am_all_tracers.Get();
    if (*this_slot == slot_looking_for)
      return this_slot;
  }
  return (0L);
}

/////
// exported tracing routines
/////

void
Am_Notify_On_Slot_Set(Am_Object object, Am_Slot_Key key, Am_Value value)
{
  if (!object.Valid() && !key && !value.Valid())
    // can't simply Make_Empty on the list because it won't know what to
    // do with all the Am_Ptr's; we need to call delete.
    destroy_all_tracers();
  else {
    Am_Slot_Tracer *temp = new Am_Slot_Tracer(object, key, value, false);
    // NDY: check to see if this test is a super- or sub-set of one of
    // the existing traces.  If it is a superset, replace old trace; if it is
    // a subset, don't add it to the list.

    am_all_tracers.Add((Am_Ptr)temp);
  }
}

void
Am_Notify_Or_Break_On_Object_Action(Am_Object old_object, Am_Set_Reason reason,
                                    bool do_break)
{
  Am_Slot_Tracer *temp = new Am_Slot_Tracer(old_object, reason, Am_No_Value,
                                            false, (0L), true, false);
  am_all_tracers.Add((Am_Ptr)temp);
  if (do_break)
    temp->Start_Breaking();
}
void
Am_Notify_Or_Break_On_Method(Am_Object &object, Am_Slot_Key slot_key,
                             bool do_break)
{
  Am_Slot_Tracer *temp = new Am_Slot_Tracer(object, slot_key, Am_No_Value,
                                            false, (0L), false, true);
  am_all_tracers.Add((Am_Ptr)temp);
  if (do_break)
    temp->Start_Breaking();
}

void
Am_Break_On_Slot_Set(Am_Object object, Am_Slot_Key key, Am_Value value)
{
  if (!object.Valid() && !key && !value.Valid())
    // can't simply Make_Empty on the list because it won't know what to
    // do with all the Am_Ptr's; we need to call delete.
    destroy_all_tracers();
  else {
    Am_Slot_Tracer *temp = new Am_Slot_Tracer(object, key, value, false);
    // NDY: check to see if this test is a super- or sub-set of one of
    // the existing traces.  If it is a superset, replace old trace; if it is
    // a subset, don't add it to the list.

    am_all_tracers.Add((Am_Ptr)temp);
    temp->Start_Breaking();
  }
}

void
Am_Clear_Slot_Notify(Am_Object object, Am_Slot_Key key, Am_Value value)
{
  Am_Slot_Tracer *this_slot;
  Am_Slot_Tracer slot_looking_for(object, key, value, false);

  for (am_all_tracers.Start(); !am_all_tracers.Last(); am_all_tracers.Next()) {
    this_slot = (Am_Slot_Tracer *)(Am_Ptr)am_all_tracers.Get();
    if (*this_slot == slot_looking_for) {
      delete this_slot;
      am_all_tracers.Delete();
    }
  }
}

void
Am_Start_Slot_Value_Tracing(Am_Object object, Am_Slot_Key key,
                            Am_Object /*old_values_object*/)
{
  if (!object.Valid() || !key)
    std::cerr << "** Am_Start_Slot_Value_Tracing:\n"
                 "   You must specify both an object and a slot key to trace "
                 "values.\n";
  else {
    Am_Value v;
    v = object.Peek(key);
    Am_Slot_Tracer *temp = new Am_Slot_Tracer(object, key, Am_Value(), true, v);
    Am_Slot_Tracer *this_slot;
    for (am_all_tracers.Start(); !am_all_tracers.Last();
         am_all_tracers.Next()) {
      this_slot = (Am_Slot_Tracer *)(Am_Ptr)am_all_tracers.Get();
      if (*this_slot == *temp) { // if the slot tracer is already there
        this_slot->Start_Tracing();
        delete temp;
        return;
      }
    }
    am_all_tracers.Add((Am_Ptr)temp);
  }
}

void
Am_Stop_Slot_Value_Tracing(Am_Object object, Am_Slot_Key key)
{
  Am_Slot_Tracer *this_slot;
  Am_Slot_Tracer slot_looking_for(object, key, Am_Value(), true);

  for (am_all_tracers.Start(); !am_all_tracers.Last(); am_all_tracers.Next()) {
    this_slot = (Am_Slot_Tracer *)(Am_Ptr)am_all_tracers.Get();
    if (*this_slot == slot_looking_for)
      this_slot->Stop_Tracing(); // don't delete, so we can still get to
                                 // saved values
  }
}

Am_Trace_Status
Am_Update_Tracer_String_Object(Am_Object obj, Am_Slot_Key key,
                               Am_Object string_obj)
{
  Am_Slot_Tracer *the_tracer = Am_Tracing(obj, key, false);
  if (the_tracer) {
    the_tracer->Set_Value_Object(string_obj);
    return the_tracer->Trace_Status();
  } else
    return Am_NOT_TRACING;
}

Am_Value_List
Am_Get_Tracer_Old_Values(Am_Object obj, Am_Slot_Key key, Am_Object str_obj)
{
  Am_Slot_Tracer *the_tracer = Am_Tracing(obj, key, true);
  if (the_tracer) {
    the_tracer->Set_Value_Object(str_obj);
    return the_tracer->Get_Old_Values();
  }
  return Am_No_Value_List;
}

Am_Trace_Status
Am_Get_Tracer_Status(Am_Object obj, Am_Slot_Key key)
{
  Am_Slot_Tracer *the_tracer = Am_Tracing(obj, key, true);
  if (the_tracer) {
    return the_tracer->Trace_Status();
  }
  return Am_NOT_TRACING;
}

void
Am_Invalidate_All_Tracer_String_Objects(Am_Object the_obj)
{
  Am_Slot_Tracer *this_slot;
  Am_Slot_Tracer test_slot(the_obj, 0, (0L), false);
  // only invalidate those matching the_obj.
  for (am_all_tracers.Start(); !am_all_tracers.Last(); am_all_tracers.Next()) {
    this_slot = (Am_Slot_Tracer *)(Am_Ptr)am_all_tracers.Get();
    if (*this_slot > test_slot)
      this_slot->Set_Value_Object(Am_No_Object);
  }
}

void Am_Standard_Slot_Trace_Proc(const Am_Slot &slot, Am_Set_Reason reason);
void Am_Standard_Object_Trace_Proc(const Am_Object &old_object,
                                   const Am_Object &new_object,
                                   Am_Set_Reason reason);
void Am_Standard_Method_Trace_Proc(const Am_Object &of_object,
                                   Am_Slot_Key slot_key, Am_Value method_value);

void
Am_Initialize_Tracer()
{
#ifdef DEBUG
  Am_Global_Slot_Trace_Proc = Am_Standard_Slot_Trace_Proc;
  Am_Global_Object_Trace_Proc = Am_Standard_Object_Trace_Proc;
  Am_Global_Method_Get_Proc = Am_Standard_Method_Trace_Proc;
#endif
}

//////
// The stuff below is (potentially) called every time a slot is set.

void
Am_Standard_Slot_Trace_Proc(const Am_Slot &slot, Am_Set_Reason reason)
{
  if (!am_all_tracers.Empty()) {
    Am_Slot_Tracer *this_slot;
    Am_Value v;
    v = slot.Get();
    Am_Object_Advanced oa = slot.Get_Owner();
    Am_Object owner = oa;
    Am_Slot_Tracer slot_looking_for(owner, slot.Get_Key(), v, false);
    Am_Value_List trace_list = am_all_tracers;

    for (trace_list.Start(); !trace_list.Last(); trace_list.Next()) {
      this_slot = (Am_Slot_Tracer *)(Am_Ptr)trace_list.Get();
      if (*this_slot > slot_looking_for) {
        this_slot->Trace(v, reason);
      }
    }
  }
  //This should be last so that if the slot causes a crash, but have a
  //break on the slot, the break will be evaluated first before the
  //inspector runs, which might trigger the crash.
  Am_Refresh_Inspector_If_Object_Changed(slot, reason);
}

void
Am_Standard_Object_Trace_Proc(const Am_Object &old_object,
                              const Am_Object &new_object, Am_Set_Reason reason)
{
  if (!am_all_tracers.Empty()) {
    Am_Slot_Tracer *this_slot;
    Am_Slot_Tracer slot_looking_for(old_object, reason, Am_No_Value, false);
    Am_Value_List trace_list = am_all_tracers;

    for (trace_list.Start(); !trace_list.Last(); trace_list.Next()) {
      this_slot = (Am_Slot_Tracer *)(Am_Ptr)trace_list.Get();
      if (*this_slot > slot_looking_for) {
        this_slot->Trace_Object(old_object, new_object, reason);
      }
    }
  }
}

void
Am_Standard_Method_Trace_Proc(const Am_Object &of_object, Am_Slot_Key slot_key,
                              Am_Value method_value)
{
  if (!am_all_tracers.Empty()) {
    Am_Slot_Tracer *this_slot;
    Am_Slot_Tracer slot_looking_for(of_object, slot_key, Am_No_Value, false,
                                    (0L), false, true);
    Am_Value_List trace_list = am_all_tracers;

    for (trace_list.Start(); !trace_list.Last(); trace_list.Next()) {
      this_slot = (Am_Slot_Tracer *)(Am_Ptr)trace_list.Get();
      if (*this_slot > slot_looking_for) {
        this_slot->Trace_Method(of_object, slot_key, method_value);
      }
    }
  }
}

Am_Slot_Tracer::Am_Slot_Tracer(Am_Object new_obj, Am_Slot_Key new_key,
                               Am_Value new_val, bool store_old,
                               Am_Value first_value, bool object_tracing,
                               bool method_tracing)
{
  object = new_obj;
  key = new_key;
  value = new_val;
  store_old_values = store_old;
  object_tracer = object_tracing;
  method_tracer = method_tracing;
  trace_status = Am_TRACING + (store_old ? Am_STORING_OLD : 0);
  current_value = first_value;
  // I want to just current_value=object.Peek() here, but that crashes
  // on some slots: Am_Slot_Tracer::Am_Slot_Tracer is called in the standard
  // slot trace procedure for the slot it's looking for; that is called
  // on slot create, which could cause problems if the slot has
  // no value yet?
}

Am_Slot_Tracer::~Am_Slot_Tracer() {}

// Tests to see if the values of (object, key, value) in this encompass
// the values in test_value.  Technically it's >= not >.
bool
Am_Slot_Tracer::operator>(Am_Slot_Tracer &test_value)
{
  // if object is null (unspecified), any object in the test value
  // will match.  If not, they must be exacly equal.  similar for other fields.
  return (object_tracer == test_value.object_tracer) &&
         (method_tracer == test_value.method_tracer) &&
         (((!(Am_Wrapper *)
                object) || // only match everything if it's a (0L) ptr
           (test_value.object.Valid() && (test_value.object == object))) &&
          (!key || (test_value.key && (test_value.key == key)))
          // value.Exists() is false when value is not an error code
          && (!value.Exists() ||
              (test_value.value.Exists() && (test_value.value == value))));
}

void
Am_Slot_Tracer::Trace(Am_Value v, Am_Set_Reason reason)
{
  if (!(trace_status & Am_TRACING))
    return;
  if (store_old_values) {
    if (!old_values.Valid())
      old_values = Am_Value_List();
    old_values.Start();
    Am_Value old_value;
    if (!old_values.Empty()) {
      old_value = old_values.Get();
      //     std::cerr << "%% Storing old value of slot: " << old_value <<std::endl;
      if (!(old_value == current_value) && !(v == current_value))
        // only add if slot changed value.
        old_values.Add(current_value, Am_HEAD);
    } else
      old_values.Add(current_value, Am_HEAD);
    // This is in inspector.cc
    // it is a _very_ heavy operation.
    Am_Add_Old_Values(old_values, string_object, (0L));
  } else {
    std::cerr << "%% Slot " << Am_Get_Slot_Name(key) << " in object " << object
              << " has been set to " << v
              << "\n   Reason: " << string_reason(reason) << std::endl;
  }
  if (string_object.Valid()) {
    char line[250];
    OSTRSTREAM_CONSTR(oss, line, 250, std::ios::out);
    reset_ostrstream(oss);
    oss << v << std::ends;
    OSTRSTREAM_COPY(oss, line, 250);
    string_object.Set(Am_TEXT, line);
  }
  current_value = v;
  if (trace_status & Am_BREAKING)
    Am_Break_Into_Debugger();
}

void
Am_Slot_Tracer::Trace_Object(const Am_Object &old_object,
                             const Am_Object &new_object, Am_Set_Reason reason)
{
  if (!(trace_status & Am_TRACING))
    return;
  if (reason == Am_TRACE_OBJECT_DESTROYED)
    std::cerr << "%% Object " << old_object << " is about to be destroyed\n"
              << std::flush;
  else {
    std::cerr << "%% Creating a new object " << new_object
              << " from old object " << old_object;
    if (reason == Am_TRACE_OBJECT_COPIED)
      std::cerr << " by Copy\n" << std::flush;
    else if (reason == Am_TRACE_OBJECT_CREATED)
      std::cerr << " by regular Create\n" << std::flush;
    else
      std::cerr << " by illegal reason " << reason << std::endl << std::flush;
  }
  current_value = Am_No_Value;
  if (trace_status & Am_BREAKING)
    Am_Break_Into_Debugger();
}

void
Am_Slot_Tracer::Trace_Method(const Am_Object &object, Am_Slot_Key key,
                             Am_Value method_value)
{
  if (!(trace_status & Am_TRACING))
    return;
  std::cerr << "%% Method in slot " << Am_Get_Slot_Name(key) << " in object "
            << object << " = " << method_value << " was accessed\n."
            << std::flush;
  current_value = method_value;
  if (trace_status & Am_BREAKING)
    Am_Break_Into_Debugger();
}
