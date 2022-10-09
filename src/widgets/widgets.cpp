/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file contains the standard widgets

   Designed and implemented by Brad Myers
*/

#include <am_inc.h>

#include <amulet/am_io.h>
#include <amulet/impl/opal_obj2.h>
#include <amulet/impl/style2motif.h>
#include <amulet/impl/widget_op.h>
#include <amulet/impl/slots_registry.h>

#ifdef OA_VERSION
// declared as extern in widgets_advanced.h
//	Map_Style2MotifRec 			Am_Map_Style2MotifRec;
//	Map_Iterator_Style2MotifRec	Am_MapIterator_Style2MotifRec;
#endif

////////////////////////////////////////////////////////////////////////
// Enum types
////////////////////////////////////////////////////////////////////////
#if 1
void
Am_Widget_Look::Print(std::ostream &out) const
{
  Am_Enum_Print_Helper(out, Am_Widget_Look_ID, (long)value);
}
void
Am_Widget_Look::Println() const
{
  Print(std::cout);
  std::cout << std::endl << std::flush;
}

#endif

//won't print as Am_NATIVE_LOOK, but can type this in as a value to be read
class Am_Widget_Look_Enum_Support : public Am_Enum_Support
{
public:
  Am_Widget_Look_Enum_Support(const char *value_string, Am_Value_Type typ)
      : Am_Enum_Support(value_string, typ)
  {
    type = typ;
  }
  Am_Value From_String(const char *string) const override
  {
    if (!strcmp(string, "Am_NATIVE_LOOK"))
      return Am_Value(Am_NATIVE_LOOK_val, type);
    else
      return Am_Enum_Support::From_String(string);
  }
};

Am_ID_Tag widget_look_id = Am_Get_Unique_ID_Tag("Am_Widget_Look", Am_ENUM_TYPE);

Am_Widget_Look_Enum_Support
    Am_Widget_Look_Support("Am_MOTIF_LOOK "
                           "Am_WINDOWS_LOOK Am_MACINTOSH_LOOK",
                           widget_look_id);

std::ostream &
operator<<(std::ostream &os, const Am_Widget_Look &item)
{
  item.Print(os);
  return os;
}

Am_ID_Tag Am_Widget_Look::Am_Widget_Look_ID = widget_look_id;

// register_support is below in initialize.

// Can't use the macro because need the ID above to create the
// Am_Widget_Look_Support object, but the macro wants to allocate a
// new ID.
// AM_DEFINE_ENUM_TYPE_IMPL(Am_Widget_Look, &Am_Widget_Look_Support);

void
Am_Widget_Look::TypeError(Am_Value const &in_value)
{
  DERR("** Tried to set a "
       "Am_Widget_Look"
       " enum with a non "
       "Am_Widget_Look"
       " wrapper: "
       << in_value << std::endl);
  Am_Error();
}

////////////////////////////////////////////////////////////////////////
//  Default Look
////////////////////////////////////////////////////////////////////////

Am_Define_No_Self_Formula(Am_Value, Am_Default_Widget_Look)
{
  return Am_Screen.Get(Am_WIDGET_LOOK);
}

////////////////////////////////////////////////////////////////////////
//  Color Utility Functions
////////////////////////////////////////////////////////////////////////

//  Values for changing the brightness of the foreground-color for platform
//  looks

#ifdef _WIN32
// These are values to catch Windows look
#define Y_SHADOW_VALUE 0.66666f
#define Y_BACKGROUND_VALUE 0.85f
#define Y_HIGHLIGHT2_VALUE 1.16666f
#define Y_HIGHLIGHT_VALUE 1.45f
#define MIN_ACCEPTABLE_BRIGHTNESS 0.3 // I guess
#define FULL_SAT_ADJUST 0.5
#else
// These are MOTIF values, from Amulet
#define Y_SHADOW_VALUE 0.24f
#define Y_BACKGROUND_VALUE 0.85f
#define Y_HIGHLIGHT2_VALUE 1.1f
#define Y_HIGHLIGHT_VALUE 1.45f
#define MIN_ACCEPTABLE_BRIGHTNESS 0.4
#define FULL_SAT_ADJUST 0.5
#endif

Am_Define_Style_Formula(Am_Default_Motif_Fill_Style)
{
  Am_Object win;
  win = self.Get(Am_WINDOW);
  if (win.Valid()) {
    bool color = win.Get(Am_IS_COLOR);
    if (color)
      return Am_Default_Color;
    else
      return Am_White;
  } else
    return Am_Default_Color;
}

inline float
fmax(float f1, float f2)
{
  if (f1 > f2)
    return f1;
  else
    return f2;
}
inline float
fmin(float f1, float f2)
{
  if (f1 > f2)
    return f2;
  else
    return f1;
}

// Compute the shades of the Motif colors in the widgets.
// Given a color and a brightness adjustment, find a color which is
// "brighter" or "dimmer" based on the adjustment factor.
//        | Y |   | .3   .59  .11 | | R |
//        | I | = | .6  -.28 -.32 | | G |       ( Y = Brightness )
//        | G |   | .21 -.52  .31 | | B |
// Given the RGB values, multiply by the above matrix to get YIG values.
// Multiply Y by the adjustment to get the new brightness, then multiply
// the new YIG matrix by the inverse of the original 3x3 matrix.
//
// V3.0: If the color is too dark or too light, adjust the other
// colors so can use fully saturated or fully dark colors.

bool
make_old_bright_enough(const Am_Style &old, float &y, float &i, float &q)
{
  float red, green, blue;
  old.Get_Values(red, green, blue);
  //if one of the "pure" colors, then adjust highlight color so will show up
  bool saturated = ((red == 1.0f || blue == 1.0f || green == 1.0f) &&
                    (red == 0.0f || blue == 0.0f || green == 0.0f));
  //  std::cout << "   rgb=" << red << "," << green << "," << blue <<std::endl <<std::flush;
  y = (red * 0.3f) + (green * 0.59f) + (blue * 0.11f);
  i = (red * 0.6f) + (green * -0.28f) + (blue * -0.32f);
  q = (red * 0.21f) + (green * -0.52f) + (blue * 0.31f);
  if (y < MIN_ACCEPTABLE_BRIGHTNESS) {
    //    std::cout << "   old Y " << y << " too dark\n" <<std::flush;
    y = static_cast<float>(MIN_ACCEPTABLE_BRIGHTNESS);
  }
  return saturated;
}

Am_Style
make_style_from_yiq(float y, float i, float q, bool adjust)
{
  float red = fmax(0.0f, fmin(1.0f, y + (i * 0.95f) + (q * 0.62f)));
  float green = fmax(0.0f, fmin(1.0f, y + (i * -0.28f) + (q * -0.64f)));
  float blue = fmax(0.0f, fmin(1.0f, y + (i * -1.10f) + (q * 1.70f)));
  if (adjust) {
    if (red != 1.0f)
      red = fmin(1.0f, red + static_cast<float>(FULL_SAT_ADJUST));
    else if (green != 1.0f)
      green = fmin(1.0f, green + static_cast<float>(FULL_SAT_ADJUST));
    else if (blue != 1.0f)
      blue = fmin(1.0f, blue + static_cast<float>(FULL_SAT_ADJUST));
    if (red == 1.0f && blue == 1.0f && green == 1.0f) { //white
      red = 0.9f;
      green = 0.9f;
      blue = 0.9f;
    }
  }
  //  std::cout<<"  new rgb=" << red << "," << green << "," << blue <<std::endl <<std::flush;
  return Am_Style(red, green, blue, 0);
}

//adjustment5 must be the biggest value
void
make_converted_styles(const Am_Style &old, float adjustment1, Am_Style &style1,
                      float adjustment2, Am_Style &style2, float adjustment3,
                      Am_Style &style3, float adjustment4, Am_Style &style4,
                      float adjustment5, Am_Style &style5)
{
  // Am_Value v = (Am_Wrapper*)old;   std::cout << "For style " << v;
  float y, i, q, y1, y2, y3, y4, y5;
  bool adjust = make_old_bright_enough(old, y, i, q);
  y1 = y * adjustment1;
  y2 = y * adjustment2;
  y3 = y * adjustment3;
  y4 = y * adjustment4;
  y5 = y * adjustment5;
  //  std::cout << "   yiq=" << y << "," << i << "," << q << " new y`s: "
  //    << y1 << " " << y2 << " " << y3 << " " << y4 << " " << y5 <std::endl <<std::flush;
  style1 = make_style_from_yiq(y1, i, q, false);
  style2 = make_style_from_yiq(y2, i, q, false);
  style3 = make_style_from_yiq(y3, i, q, false);
  style4 = make_style_from_yiq(y4, i, q, false);
  style5 = make_style_from_yiq(y5, i, q, adjust);
}

inline float
fmax(float f1, float f2, float f3)
{
  if (f1 > f2) {
    if (f3 > f1)
      return f3;
    else
      return f1;
  } else {
    if (f3 > f2)
      return f3;
    else
      return f2;
  }
}
inline float
fmin(float f1, float f2, float f3)
{
  if (f1 > f2) {
    if (f3 < f2)
      return f3;
    else
      return f2;
  } else {
    if (f3 < f1)
      return f3;
    else
      return f1;
  }
}

// LUM = Average of the Minimum and Maximum values in the set (R, G, B).
// Is LUM > 0.5 ?
bool
is_light(Am_Style &color)
{
  float red, green, blue;
  color.Get_Values(red, green, blue);
  if (((fmin(red, green, blue) + fmax(red, green, blue)) / 2) > 0.7)
    return true;
  else
    return false;
}

AM_WRAPPER_DATA_IMPL(Computed_Colors_Record, (this))

std::ostream &
operator<<(std::ostream &os, Computed_Colors_Record_Data &rec)
{
  rec.Print(os);
  return os;
}
void
print_color(std::ostream &os, const char *s, const Am_Style &style)
{
  float red, green, blue;
  style.Get_Values(red, green, blue);
  os << s << "=(" << red << "," << green << "," << blue << ")";
}
void
Computed_Colors_Record_Data::Print(std::ostream &os) const
{
  os << "[" << (void *)this << "]";
  print_color(os, " fore", foreground_style);
  print_color(os, " back", background_style);
  print_color(os, " shad", shadow_style);
  print_color(os, " high", highlight_style);
  print_color(os, " high2", highlight2_style);
  os << " light=" << light;
}

//constructor
Computed_Colors_Record_Data::Computed_Colors_Record_Data(
    const Am_Style &foreground)
{
  key = foreground;
  if (foreground == Am_No_Style) {
    foreground_style = Am_No_Style;
    background_style = Am_No_Style;
    shadow_style = Am_No_Style;
    highlight_style = Am_No_Style;
    highlight2_style = Am_No_Style;
    light = true;
  } else {
    // need to convert foreground to make sure it has a line-thickness of 2
    make_converted_styles(foreground, Y_SHADOW_VALUE, shadow_style,
                          Y_BACKGROUND_VALUE, background_style, 1.0f,
                          foreground_style, Y_HIGHLIGHT2_VALUE,
                          highlight2_style, Y_HIGHLIGHT_VALUE, highlight_style);
    light = is_light(foreground_style);
  }
}

Computed_Colors_Record_Data::Computed_Colors_Record_Data(
    Computed_Colors_Record_Data *prev)
{
  key = prev->key;
  foreground_style = prev->foreground_style;
  background_style = prev->background_style;
  shadow_style = prev->shadow_style;
  highlight_style = prev->highlight_style;
  highlight2_style = prev->highlight2_style;
  light = prev->light;
}

//////// define the hash table from a style to a Computed_Colors_Record //////

#ifndef OA_VERSION
AM_IMPL_MAP(Style2MotifRec, Am_Wrapper *, (0L), Computed_Colors_Record_Data *,
            (0L))
#endif

////////////////////////////////////////////////////////////////////////
// Global color map
////////////////////////////////////////////////////////////////////////

Am_Map_Style2MotifRec computed_colors_map;
Computed_Colors_Record black_and_white_rec;
Computed_Colors_Record_Data black_and_white_rec_data;
Am_Style Am_Motif_Inactive_Stipple;
Am_Style Am_Motif_White_Inactive_Stipple;
Am_Style Am_Key_Border_Line; //black, thickness=2

Computed_Colors_Record::Computed_Colors_Record(const Am_Style &foreground_color)
{
  Am_Wrapper *foreground_wrapper = foreground_color;
  Computed_Colors_Record_Data *rec =
      computed_colors_map.GetAt(foreground_wrapper);
  if (!rec) {
    rec = new Computed_Colors_Record_Data(foreground_color);
    if (foreground_wrapper)
      foreground_wrapper->Note_Reference();
    computed_colors_map[foreground_wrapper] = rec;
  } else
    rec->Note_Reference();
  if (foreground_wrapper)
    foreground_wrapper->Release();
  data = rec;
}

Computed_Colors_Record_Data::~Computed_Colors_Record_Data()
{
  Am_Wrapper *key_val = key;
  computed_colors_map.DeleteKey(key_val);
  if (key_val)
    key_val->Release();
}

Am_Define_Formula(Am_Wrapper *, Am_Get_Computed_Colors_Record_Form)
{
  Am_Style foreground_color;
  Am_Value value;
  bool is_color = true;
  value = self.Peek(Am_FILL_STYLE);
  if (Am_Type_Class(value.type) == Am_WRAPPER)
    foreground_color = value;
  Am_Object window;
  window = self.Get(Am_WINDOW);
  if (window.Valid()) {
    Am_Drawonable *draw = Am_Drawonable::Narrow(window.Get(Am_DRAWONABLE));
    is_color = !draw || draw->Is_Color();
  }
  if (is_color)
    return Computed_Colors_Record(foreground_color);
  else
    return black_and_white_rec;
}

///////////////////////////////////////////////////////////////////////////
// Accelerators
///////////////////////////////////////////////////////////////////////////

//// Design: Uses destructive modification on the accelerator list so
//// the list can be shared among multiple windows, which is set up using
//// the function Am_Share_Accelerators.  Also, the list must start
//// off as non-empty so it never goes to empty, because (like in
//// Lisp) an empty list cannot be shared.

//matches on the accelerator character
static Am_Object
accel_match(Am_Value_List accel_list, Am_Input_Char target_ic, Am_Object window)
{
  Am_Input_Char accel_char;
  Am_Object cmd, ret;
  bool changed = false;
  Am_Value value;
  for (accel_list.Start(); !accel_list.Last(); accel_list.Next()) {
    cmd = accel_list.Get();
    if (cmd.Valid()) {
      value = cmd.Peek(Am_ACCELERATOR, Am_NO_DEPENDENCY);
      if (value.Valid()) {
        if (Am_Input_Char::Test(value)) {
          accel_char = value;
          if (accel_char == target_ic) {
            ret = cmd;
            break; //leave loop
          }
        }
      }
    } else { //here, remove the cmd since it isn't valid anymore.
      //false so destructive modification
      accel_list.Delete(false);
      changed = true;
    }
  }
  if (changed && window.Valid())
    window.Note_Changed(Am_ACCELERATOR_LIST);
  return ret;
}

static void
check_for_conflicts(Am_Object window, Am_Value_List accel_list,
                    Am_Object command)
{
  Am_Input_Char target_ic;
  const Am_Value& accel = command.Get(Am_ACCELERATOR, Am_NO_DEPENDENCY);
  if (Am_Input_Char::Test(accel)) {
    target_ic = accel;
    Am_Object old_cmd = accel_match(accel_list, target_ic, Am_No_Object);
    if (old_cmd.Valid()) {
      std::cerr << "** Amulet Error, In window " << window
                << " accelerator character " << target_ic << " for new command "
                << command;
      Am_Object widget;
      widget = command.Get(Am_SAVED_OLD_OWNER, Am_RETURN_ZERO_ON_ERROR);
      if (widget.Valid())
        std::cerr << " in widget " << widget;
      std::cerr << " conflicts with accelerator on existing command "
                << old_cmd;
      widget = old_cmd.Get(Am_SAVED_OLD_OWNER, Am_RETURN_ZERO_ON_ERROR);
      if (widget.Valid())
        std::cerr << " in widget " << widget;
      std::cerr << std::endl << std::flush;
      Am_Error(window, Am_ACCELERATOR_LIST);
    }
  }
}

static void
check_create_inter_and_list(Am_Object &window, Am_Object &inter,
                            Am_Value_List &accel_list)
{
  Am_Value value;
  value = window.Peek(Am_ACCELERATOR_INTER, Am_NO_DEPENDENCY);
  if (!value.Valid()) {
    Am_INTER_TRACE_PRINT(window, "Creating accel inter for " << window);
    inter = Am_Accelerator_Inter.Create();
    window.Add_Part(Am_ACCELERATOR_INTER, inter);
  } else
    inter = value;
  value = window.Peek(Am_ACCELERATOR_LIST, Am_NO_DEPENDENCY);
  if (value.Valid() && Am_Value_List::Test(value))
    accel_list = value;
  else {
    accel_list.Add(Am_Command.Create(DSTR("dummy"))); //so will be a real list
    window.Add(Am_ACCELERATOR_LIST, accel_list)
        .Set_Inherit_Rule(Am_ACCELERATOR_LIST, Am_LOCAL);
  }
}

Am_Object
Am_Check_Accelerator_Char_For_Window(Am_Input_Char accel, Am_Object window)
{
  if (window.Valid()) {
    Am_Value value;
    value = window.Peek(Am_ACCELERATOR_LIST);
    if (Am_Value_List::Test(value)) {
      Am_Value_List accel_list = value;
      accel_list.Start();
      return accel_match(accel_list, accel, Am_No_Object);
    }
  }
  return Am_No_Object;
}

// widget has a command in its Am_COMMAND slot with an event in the
// Am_ACCELERATOR slot of the command.  Add the widget to the widget's
// window's global list of accelerators.
void
Am_Add_Accelerator_Command_To_Window(Am_Object command, Am_Object window)
{
  Am_Object inter;
  Am_Value_List accel_list;
  if (window.Valid()) {
    check_create_inter_and_list(window, inter, accel_list);
    accel_list.Start();
    if (!accel_list.Member(command)) {
      check_for_conflicts(window, accel_list, command);
      //destructive modification, because list might be shared by
      //multiple windows
      accel_list.Add(command, Am_TAIL, false);
      Am_INTER_TRACE_PRINT(inter, "Adding " << command << " to accel list for "
                                            << window);
      window.Note_Changed(Am_ACCELERATOR_LIST);
    }
  }
}

//OK if not there
void
Am_Remove_Accelerator_Command_From_Window(Am_Object command, Am_Object window)
{
  Am_Value_List accel_list;
  Am_Value value;
  if (window.Valid()) {
    value = window.Peek(Am_ACCELERATOR_LIST, Am_NO_DEPENDENCY);
    if (value.Valid() && Am_Value_List::Test(value)) {
      accel_list = value;
      accel_list.Start();
      if (accel_list.Member(command)) {
        //false so destructive modification, because list may be shared
        accel_list.Delete(false);
        window.Note_Changed(Am_ACCELERATOR_LIST);
        Am_INTER_TRACE_PRINT(
            window, "Removing " << command << " from accel list of " << window);
      }
    }
  }
}

Am_Define_Method(Am_Event_Method, bool, accelerator_start_where,
                 (Am_Object inter, Am_Object event_window, Am_Input_Char ic))
{
  Am_INTER_TRACE_PRINT(inter, "testing " << ic << " as accelerator for "
                                         << event_window);
  Am_Value_List accel_list;
  Am_Value value;
  value = event_window.Peek(Am_ACCELERATOR_LIST);
  if (value.Valid() && Am_Value_List::Test(value)) {
    accel_list = value;
    Am_Object cmd = accel_match(accel_list, ic, event_window);
    if (cmd.Valid()) {
      Am_INTER_TRACE_PRINT(inter, "Found accelerator " << cmd);
      if ((bool)cmd.Get(Am_ACTIVE)) {
        inter.Set(Am_ACCELERATOR, cmd, Am_OK_IF_NOT_THERE);
        return true;
      } else {
        Am_INTER_TRACE_PRINT(inter, "-> but not active");
      }
    }
  }
  //if get here, then fail
  return false;
}

Am_Define_Method(Am_Object_Method, void, do_accelerator_action, (Am_Object cmd))
{
  Am_Object inter, accel_cmd;
  inter = cmd.Get_Owner();
  if (inter.Valid())
    accel_cmd = inter.Get(Am_ACCELERATOR);
  cmd.Set(Am_IMPLEMENTATION_PARENT, accel_cmd);
  Am_INTER_TRACE_PRINT(cmd, "Executing accelerator for " << accel_cmd);
}

void
Am_Share_Accelerators(Am_Object &source_window, Am_Object &new_window)
{
  Am_Object inter;
  Am_Value_List accel_list;
  check_create_inter_and_list(source_window, inter, accel_list);
  new_window.Add_Part(Am_ACCELERATOR_INTER, Am_Accelerator_Inter.Create());
  new_window.Set(Am_ACCELERATOR_LIST, accel_list, Am_OK_IF_NOT_THERE)
      .Set_Inherit_Rule(Am_ACCELERATOR_LIST, Am_LOCAL);
  Am_INTER_TRACE_PRINT(new_window, "Sharing accel inter " << inter << " with "
                                                          << new_window);
}

////////////////////////////////////////////////////////////////////////
//  Starting, Stopping and Aborting Widgets
////////////////////////////////////////////////////////////////////////

AM_DEFINE_METHOD_TYPE_IMPL(Am_Explicit_Widget_Run_Method)

void
Am_Start_Widget(Am_Object widget, Am_Value initial_value)
{
  Am_Value v;
  v = widget.Peek(Am_WIDGET_START_METHOD);
  if (v.Valid()) {
    Am_Explicit_Widget_Run_Method method = v;
    method.Call(widget, initial_value);
  } else
    Am_ERROR("Widget " << widget << " has no start method");
}

//Explicitly abort an interactor.  If not running, does nothing.  This
//makes sure the Widget's commands are not entered into the command
//history, even if this is called from the widget command's DO method.

void
Am_Abort_Widget(Am_Object widget_or_inter_or_command)
{
  Am_Object widget, inter, command;
  if (widget_or_inter_or_command.Is_Instance_Of(Am_Command)) {
    command = widget_or_inter_or_command;
    widget_or_inter_or_command = command.Get(Am_SAVED_OLD_OWNER);
  }
  if (widget_or_inter_or_command.Valid()) {
    if (widget_or_inter_or_command.Is_Instance_Of(Am_Interactor)) {
      inter = widget_or_inter_or_command;
      Am_Abort_Interactor(inter);
    } else { //hope it is a widget with a method
      widget = widget_or_inter_or_command;
      Am_Value v;
      v = widget.Peek(Am_WIDGET_ABORT_METHOD);
      if (v.Valid()) {
        Am_Object_Method method = v;
        method.Call(widget);
      }
    }
  }
  //make sure the command isn't entered into the history
  if (command.Valid())
    command.Set(Am_COMMAND_IS_ABORTING, true, Am_OK_IF_NOT_THERE);
}

//Explicitly stop a widget.  If final_value is supplied, then this is
//the value used as the value of
//the widget.  If final_value is not supplied, the widget uses its current
//value.  If the widget is running, commands associated with the
//widget will be invoked just as if the widget had terminated normally.
void
Am_Stop_Widget(Am_Object widget, Am_Value final_value)
{
  Am_Value v;
  v = widget.Peek(Am_WIDGET_STOP_METHOD);
  if (v.Valid()) {
    Am_Explicit_Widget_Run_Method method = v;
    method.Call(widget, final_value);
  }
}

Am_Define_Method(Am_Explicit_Widget_Run_Method, void,
                 Am_Standard_Widget_Start_Method,
                 (Am_Object widget, Am_Value initial_value))
{
  if (initial_value.Valid())
    widget.Set(Am_VALUE, initial_value);
  Am_Object inter = widget.Get_Object(Am_INTERACTOR);
  Am_Start_Interactor(inter);
}

Am_Define_Method(Am_Explicit_Widget_Run_Method, void,
                 Am_Standard_Widget_Stop_Method,
                 (Am_Object widget, Am_Value final_value))
{
  if (final_value.Valid())
    widget.Set(Am_VALUE, final_value);
  Am_Object inter = widget.Get_Object(Am_INTERACTOR);
  Am_Stop_Interactor(inter);
}

Am_Define_Method(Am_Object_Method, void, Am_Standard_Widget_Abort_Method,
                 (Am_Object widget))
{
  Am_Object inter = widget.Get_Object(Am_INTERACTOR);
  Am_Abort_Interactor(inter);
}

void
Am_Set_Default_Look(Am_Widget_Look inLook)
{
  Am_Screen.Set(Am_WIDGET_LOOK, inLook, Am_OK_IF_NOT_THERE);
}

///////////////////////////////////////////////////////////////////////////
// Create all the widgets
///////////////////////////////////////////////////////////////////////////

// Standard slots of interactors and command objects
void
widgets_set_slot_names()
{
#ifdef DEBUG
  Am_Register_Slot_Key(Am_FINAL_FEEDBACK_WANTED, "FINAL_FEEDBACK_WANTED");
  Am_Register_Slot_Key(Am_KEY_SELECTED, "KEY_SELECTED");
  Am_Register_Slot_Key(Am_ACTIVE_2, "ACTIVE_2");
  Am_Register_Slot_Key(Am_ITEM_OFFSET, "ITEM_OFFSET");
  Am_Register_Slot_Key(Am_TEXT_OFFSET, "TEXT_OFFSET");
  Am_Register_Slot_Key(Am_WIDGET_LOOK, "WIDGET_LOOK");
  Am_Register_Slot_Key(Am_DEFAULT, "DEFAULT");
  Am_Register_Slot_Key(Am_INTERACTOR, "~INTERACTOR~");
  Am_Register_Slot_Key(Am_STYLE_RECORD, "~STYLE_RECORD~");
  Am_Register_Slot_Key(Am_REAL_STRING_OR_OBJ, "~REAL_STRING_OR_OBJ~");
  Am_Register_Slot_Key(Am_ATTACHED_OBJECT, "~ATTACHED OBJECT~");
  Am_Register_Slot_Key(Am_ATTACHED_COMMAND, "~ATTACHED COMMAND~");
  Am_Register_Slot_Key(Am_LABEL_OR_ID, "~LABEL_OR_ID~");
  Am_Register_Slot_Key(Am_ITEM_TO_COMMAND, "~ITEM_TO_COMMAND~");
  Am_Register_Slot_Key(Am_BOX_ON_LEFT, "BOX_ON_LEFT");
  Am_Register_Slot_Key(Am_BOX_HEIGHT, "BOX_HEIGHT");
  Am_Register_Slot_Key(Am_BOX_WIDTH, "BOX_WIDTH");

  Am_Register_Slot_Key(Am_VALUE_1, "VALUE_1");
  Am_Register_Slot_Key(Am_VALUE_2, "VALUE_2");
  Am_Register_Slot_Key(Am_SMALL_INCREMENT, "SMALL_INCREMENT");
  Am_Register_Slot_Key(Am_LARGE_INCREMENT, "LARGE_INCREMENT");
  Am_Register_Slot_Key(Am_PERCENT_VISIBLE, "PERCENT_VISIBLE");
  Am_Register_Slot_Key(Am_SCROLL_ARROW_DIRECTION, "ARROW_DIRECTION");
  Am_Register_Slot_Key(Am_SCROLL_AREA_MIN, "SCROLL_AREA_MIN");
  Am_Register_Slot_Key(Am_SCROLL_AREA_MAX, "SCROLL_AREA_MAX");
  Am_Register_Slot_Key(Am_SCROLL_AREA_SIZE, "SCROLL_AREA_SIZE");
  Am_Register_Slot_Key(Am_SCROLL_ARROW1, "~SCROLL_ARROW1~");
  Am_Register_Slot_Key(Am_SCROLL_ARROW2, "~SCROLL_ARROW2~");
  Am_Register_Slot_Key(Am_SCROLL_INDICATOR, "~SCROLL_INDICATOR~");
  Am_Register_Slot_Key(Am_ARROW_INTERACTOR, "~ARROW_INTERACTOR~");
  Am_Register_Slot_Key(Am_BACKGROUND_INTERACTOR, "~BACKGROUND_INTERACTOR~");
  Am_Register_Slot_Key(Am_FIRST_TICK, "~FIRST_TICK~");

  Am_Register_Slot_Key(Am_REAL_WIDTH, "~REAL_WIDTH~");
  Am_Register_Slot_Key(Am_REAL_HEIGHT, "~REAL_HEIGHT~");
  Am_Register_Slot_Key(Am_SUB_MENU, "~SUB_MENU~");
  Am_Register_Slot_Key(Am_FOR_ITEM, "~FOR_ITEM~");
  Am_Register_Slot_Key(Am_COMPUTE_INTER_VALUE, "~COMPUTE_INTER_VALUE~");
  Am_Register_Slot_Key(Am_WEB_CONSTRAINT, "~WEB_CONSTRAINT~");
  Am_Register_Slot_Key(Am_TEXT_WIDGET_BOX_LEFT, "~TEXT_WIDGET_BOX_LEFT~");
  Am_Register_Slot_Key(Am_TEXT_WIDGET_TEXT_OBJ, "~TEXT_WIDGET_TEXT_OBJ~");

  Am_Register_Slot_Key(Am_SELECTIONS_HANDLES_USE_LT_RB,
                       "~SELECTIONS_HANDLES_USE_LT_RB~");
  Am_Register_Slot_Key(Am_MOVE_INTERACTOR, "~MOVE_INTERACTOR~");
  Am_Register_Slot_Key(Am_GROW_INTERACTOR, "~GROW_INTERACTOR~");
  Am_Register_Slot_Key(Am_SELECT_OUTSIDE_INTERACTOR,
                       "~SELECT_OUTSIDE_INTERACTOR~");
  Am_Register_Slot_Key(Am_MOVE_SEL_FOR_BACK_INTERACTOR,
                       "~Am_MOVE_SEL_FOR_BACK_INTERACTOR~");
  Am_Register_Slot_Key(Am_LINE_FEEDBACK_OBJECT, "~LINE_FEEDBACK_OBJECT~");
  Am_Register_Slot_Key(Am_RECT_FEEDBACK_OBJECT, "~RECT_FEEDBACK_OBJECT~");
  Am_Register_Slot_Key(Am_FAKE_GROUP, "~FAKE_GROUP~");
  Am_Register_Slot_Key(Am_ACCELERATOR_STRING, "~ACCELERATOR_STRING~");
  Am_Register_Slot_Key(Am_ACCELERATOR_INTER, "~ACCELERATOR_INTER~");
  Am_Register_Slot_Key(Am_ACCELERATOR_LIST, "~ACCELERATOR_LIST~");

  Am_Register_Slot_Key(Am_H_SCROLL_BAR, "H_SCROLL_BAR");
  Am_Register_Slot_Key(Am_V_SCROLL_BAR, "V_SCROLL_BAR");
  Am_Register_Slot_Key(Am_H_SCROLL_BAR_ON_TOP, "H_SCROLL_BAR_ON_TOP");
  Am_Register_Slot_Key(Am_V_SCROLL_BAR_ON_LEFT, "V_SCROLL_BAR_ON_LEFT");
  Am_Register_Slot_Key(Am_H_SMALL_INCREMENT, "H_SMALL_INCREMENT");
  Am_Register_Slot_Key(Am_H_LARGE_INCREMENT, "H_LARGE_INCREMENT");
  Am_Register_Slot_Key(Am_V_SMALL_INCREMENT, "V_SMALL_INCREMENT");
  Am_Register_Slot_Key(Am_V_LARGE_INCREMENT, "V_LARGE_INCREMENT");
  Am_Register_Slot_Key(Am_INNER_WIDTH, "INNER_WIDTH");
  Am_Register_Slot_Key(Am_INNER_HEIGHT, "INNER_HEIGHT");
  Am_Register_Slot_Key(Am_CLIP_LEFT, "~CLIP_LEFT~");
  Am_Register_Slot_Key(Am_CLIP_TOP, "~CLIP_TOP~");
  Am_Register_Slot_Key(Am_CLIP_WIDTH, "~CLIP_WIDTH~");
  Am_Register_Slot_Key(Am_CLIP_HEIGHT, "~CLIP_HEIGHT~");
  Am_Register_Slot_Key(Am_H_SCROLLER, "~H_SCROLLER~");
  Am_Register_Slot_Key(Am_V_SCROLLER, "~V_SCROLLER~");
  Am_Register_Slot_Key(Am_LEAVE_ROOM_FOR_FRINGE, "LEAVE_ROOM_FOR_FRINGE");
  Am_Register_Slot_Key(Am_MENU_BORDER, "~MENU_BORDER");
  Am_Register_Slot_Key(Am_MENU_SELECTED_COLOR, "MENU_SELECTED_COLOR");
  Am_Register_Slot_Key(Am_MENU_LINE_HEIGHT, "~MENU_LINE_HEIGHT");
  Am_Register_Slot_Key(Am_MENU_ITEM_TOP_OFFSET, "~MENU_ITEM_TOP_OFFSET");
  Am_Register_Slot_Key(Am_MENU_ITEM_BOT_OFFSET, "~MENU_ITEM_BOT_OFFSET");
  Am_Register_Slot_Key(Am_MENU_ITEM_LEFT_OFFSET, "~MENU_ITEM_LEFT_OFFSET");
  Am_Register_Slot_Key(Am_MENU_ITEM_ACCEL_OFFSET, "~MENU_ITEM_ACCEL_OFFSET");
  Am_Register_Slot_Key(Am_MENU_ITEM_RIGHT_OFFSET, "~MENU_ITEM_RIGHT_OFFSET");

  Am_Register_Slot_Key(Am_INNER_FILL_STYLE, "INNER_FILL_STYLE");
  Am_Register_Slot_Key(Am_LABEL_FONT, "LABEL_FONT");
  Am_Register_Slot_Key(Am_USE_ERROR_DIALOG, "USE_ERROR_DIALOG");
  Am_Register_Slot_Key(Am_OPERATES_ON, "OPERATES_ON");
  Am_Register_Slot_Key(Am_REGION_WHERE_TEST, "REGION_WHERE_TEST");
  Am_Register_Slot_Key(Am_MOVE_GROW_COMMAND, "Am_MOVE_GROW_COMMAND");
  Am_Register_Slot_Key(Am_SELECTION_MOVE_CHARS, "Am_SELECTION_MOVE_CHARS");
  Am_Register_Slot_Key(Am_SELECTION_WIDGET, "SELECTION_WIDGET");
  Am_Register_Slot_Key(Am_OBJECT_MODIFIED_PLACES, "~OBJECT_MODIFIED_PLACES~");
  Am_Register_Slot_Key(Am_GET_WIDGET_VALUE_METHOD, "GET_WIDGET_VALUE_METHOD");
  Am_Register_Slot_Key(Am_GET_OBJECT_VALUE_METHOD, "GET_OBJECT_VALUE_METHOD");
  Am_Register_Slot_Key(Am_SET_OBJECT_VALUE_METHOD, "SET_OBJECT_VALUE_METHOD");
  Am_Register_Slot_Key(Am_HANDLE_OPEN_SAVE_METHOD, "HANDLE_OPEN_SAVE_METHOD");
  Am_Register_Slot_Key(Am_SLOT_FOR_VALUE, "SLOT_FOR_VALUE");
  Am_Register_Slot_Key(Am_CLIPBOARD, "CLIPBOARD");
  Am_Register_Slot_Key(Am_OLD_CLIPBOARD_OBJECTS, "~OLD_CLIPBOARD_OBJECTS~");
  Am_Register_Slot_Key(Am_DROP_TARGET, "DROP_TARGET");
  Am_Register_Slot_Key(Am_DROP_TARGET_TEST, "DROP_TARGET_TEST");
  Am_Register_Slot_Key(Am_MULTI_SELECTIONS, "MULTI_SELECTIONS");
  Am_Register_Slot_Key(Am_DEFAULT_LOAD_SAVE_FILENAME,
                       "DEFAULT_LOAD_SAVE_FILENAME");
  Am_Register_Slot_Key(Am_CREATED_GROUP, "~CREATED_GROUP~");
  Am_Register_Slot_Key(Am_WIDGET_START_METHOD, "~WIDGET_START_METHOD~");
  Am_Register_Slot_Key(Am_WIDGET_ABORT_METHOD, "~WIDGET_ABORT_METHOD~");
  Am_Register_Slot_Key(Am_WIDGET_STOP_METHOD, "~WIDGET_STOP_METHOD~");

  Am_Register_Slot_Key(Am_TEXT_WIDGET, "~TEXT_WIDGET~");
  Am_Register_Slot_Key(Am_DIALOG_GROUP, "~DIALOG_GROUP~");
  Am_Register_Slot_Key(Am_ICON_IN_ABOUT_DB, "~ICON_IN_ABOUT_DB~");
  Am_Register_Slot_Key(Am_LAST_USED_OLD_INTERIM_VALUE,
                       "~Am_LAST_USED_OLD_INTERIM_VALUE~");
  Am_Register_Slot_Key(Am_LAST_INITIAL_REF_OBJECT,
                       "~Am_LAST_INITIAL_REF_OBJECT~");

  Am_Register_Slot_Key(Am_SELECT_CLOSEST_POINT_STYLE,
                       "SELECT_CLOSEST_POINT_STYLE");
  Am_Register_Slot_Key(Am_SELECT_CLOSEST_POINT_OBJ, "SELECT_CLOSEST_POINT_OBJ");
  Am_Register_Slot_Key(Am_SELECT_CLOSEST_POINT_WHERE,
                       "SELECT_CLOSEST_POINT_WHERE");
  Am_Register_Slot_Key(Am_SELECT_CLOSEST_POINT_METHOD,
                       "SELECT_CLOSEST_POINT_METHOD");

  Am_Register_Slot_Key(Am_VALID_INPUT, "VALID_INPUT");
  Am_Register_Slot_Key(Am_LIST_OF_TEXT_WIDGETS, "LIST_OF_TEXT_WIDGETS");
  Am_Register_Slot_Key(Am_LABEL_LIST, "LABEL_LIST");
  Am_Register_Slot_Key(Am_CHECKED_ITEM, "CHECKED_ITEM");

#endif
}

// exported objects

Am_Object Am_Widget_Aggregate;
Am_Object Am_Widget_Group;
Am_Object Am_Widget_Map;

Am_Object Am_Accelerator_Inter;
Am_Input_Char Am_Default_Widget_Start_Char;

// internal objects

#ifdef DEBUG
/////////////////////////////////////////////////////////////////////////////
// Easy way to change the look and feel in any application
/////////////////////////////////////////////////////////////////////////////
Am_Define_Method(Am_Object_Method, void, am_cycle_widget_look,
                 (Am_Object /* cmd */))
{
  Am_Widget_Look look = Am_Widget_Group.Get(Am_WIDGET_LOOK);
  switch (look.value) {
  case Am_MOTIF_LOOK_val:
    look = Am_WINDOWS_LOOK;
    break;
  case Am_WINDOWS_LOOK_val:
    look = Am_MACINTOSH_LOOK;
    break;
  case Am_MACINTOSH_LOOK_val:
    look = Am_MOTIF_LOOK;
    break;
  default:
    Am_Error("Unknown Look parameter");
    break;
  }
  Am_Set_Default_Look(look);
}

// we make this a global object so we can get rid of it later
Am_Object change_look_inter;

#endif

void
Am_Widgets_Initialize()
{
  Am_Register_Support(widget_look_id, &Am_Widget_Look_Support);

  Am_Object inter; // interactor in the widget
  Am_Object command_obj;

  widgets_set_slot_names();

  Am_Motif_Inactive_Stipple = Am_Gray_Stipple;
  Am_Motif_White_Inactive_Stipple =
      Am_Gray_Stipple.Clone_With_New_Color(Am_White);

  Am_Key_Border_Line = Am_Style(0.0f, 0.0f, 0.0f, 2); // black, thickness=2

  // for black and white drawonables:
  black_and_white_rec.data = &black_and_white_rec_data;
  black_and_white_rec_data.foreground_style = Am_White;
  black_and_white_rec_data.background_style = Am_White;
  black_and_white_rec_data.shadow_style = Am_Black;
  black_and_white_rec_data.highlight_style = Am_Opaque_Gray_Stipple;
  black_and_white_rec_data.light = true;

  //any click, any modifier
  Am_Default_Widget_Start_Char = Am_Input_Char("ANY_LEFT_DOWN");

  // global look and feel;
  Am_Set_Default_Look(Am_NATIVE_LOOK);

  //////////// rectangle ///////////// was here

  // main prototypes from which all widgets are derived
  Am_Widget_Aggregate = Am_Aggregate.Create(DSTR("Widget_Aggregate"))
                            .Add(Am_WIDGET_LOOK, Am_Default_Widget_Look)
                            .Add(Am_PRETEND_TO_BE_LEAF, true);
  Am_Widget_Group = Am_Group.Create(DSTR("Widget_Group"))
                        .Add(Am_WIDGET_LOOK, Am_Default_Widget_Look);
  Am_Widget_Map = Am_Map.Create(DSTR("Widget_Map"))
                      .Add(Am_WIDGET_LOOK, Am_Default_Widget_Look);

  Am_Accelerator_Inter =
      Am_One_Shot_Interactor.Create(DSTR("Accelerator"))
          .Set(Am_START_WHERE_TEST, true)
          //start_when test does the complex event check, so any place is fine
          .Set(Am_START_WHEN, accelerator_start_where)
          .Set(Am_SET_SELECTED, false);
  Am_Accelerator_Inter.Get_Object(Am_COMMAND)
      .Set(Am_DO_METHOD, do_accelerator_action);

#ifdef DEBUG
  change_look_inter =
      Am_One_Shot_Interactor.Create(DSTR("cycle_widget_look_inter"))
          .Set(Am_MULTI_OWNERS, true) //will operate in all windows
          .Set(Am_START_WHEN, Am_Input_Char("CONTROL_SHIFT_META_L"));
  change_look_inter.Get_Object(Am_COMMAND)
      .Set(Am_DO_METHOD, am_cycle_widget_look)
      .Set(Am_IMPLEMENTATION_PARENT, true); //not undoable
  Am_Screen.Add_Part(change_look_inter);
#endif

} // end widgets initialize

void
Am_Cleanup_Widgets()
{
#ifdef DEBUG
  change_look_inter.Destroy();
#endif
}

void
Am_Pop_Up_Error_Window(const char *error_string)
{
  Am_Beep();
  Am_Show_Alert_Dialog(error_string);
}

static Am_Initializer *widgets_basics_init =
    new Am_Initializer(DSTR("Widgets_Basics"), Am_Widgets_Initialize, 5.0f, 124,
                       Am_Cleanup_Widgets);
