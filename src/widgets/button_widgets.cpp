/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file contains the button-like widgets

   Designed and implemented by Brad Myers
*/

#include <am_inc.h>
#include <amulet/impl/opal_obj2.h>
#include <amulet/impl/opal_constraints.h>
#include <amulet/impl/inter_undo.h>

class Am_Input_Event;

#ifdef DEBUG
#include <amulet/registry.h> //used for Am_Get_Type_Name to print a warning
#endif

//generate a string from the label if the command if not a string
//or object
#include <amulet/am_strstream.h>

Am_Object Am_Menu_In_Scrolling_Menu;

/******************************************************************************
 * Buttons
 *   Drawing Am_PUSH_BUTTON, Am_CHECK_BUTTON and Am_RADIO_BUTTON
 *   -----------------------------------------------------------
 * The basic strategy is to divide the drawing into two distinct stages.
 * The first stage involves drawing the _box_. Here is what the box is:
 *  o For push buttons  => the button outline and background.
 *  o For check buttons => the check box
 *  o For radio buttons => either the diamond or circle.
 * The second stage involves drawing the title. A title can either be a string
 * or an object. If both are supplied, the string is drawn.
 *
 * All drawing passes through Am_Draw_Button_Widget.
 *   Am_Draw_Button_Widget first does drawing for key_selected.
 *   Then it calls Am_Draw_*_Button_Box to draw the appropriate box.
 *     The coordinates are sent in the form of a struct rect. These coords
 *     define the rectangle for the box to be drawn. This allows the drawing
 *     routines to clip drawing to that region. Additionally the drawing
 *     routines are not concerned with if the check box is on the left or right.
 *     Am_Draw_Button_Widget determines the appropriate coords.
 *   Once the box is drawn, then either the string or object is drawn, inside
 *   its own clipped rectangle. Since the drawing for the string or the object
 *   std::ends to be almost look and feel independent, all the drawing is done
 *   directly from Am_Draw_Button_Widget.
 */

/******************************************************************************
 * Button drawing helper stuff.
 *   o struct am_rect - makes passes coordinates easier. Also allows for
 *       Inset_Rect
 *   o Inset_Rect, since much of the drawing involves concentric rectangles
 *       which grow smaller, Inset_Rect simplifies maintaining rectangle coords.
 *   o Am_Draw_Rect_Border - used mainly for the Windows drawing routines.
 *       For drawing 3D like rectangles. Upper_left is the line style used to
 *       draw the top and left sides. Lower_right for the bottom and right.
 */

/******************************************************************************
 * PUSH BUTTONS
 *****************************************************************************/

/******************************************************************************
 * CHECK BUTTONS
 *****************************************************************************/

/******************************************************************************
 * RADIO BUTTONS
 *******************************************************************************/

/******************************************************************************
 * Single button Undo stuff
 *****************************************************************************/

//no repeat on new for button widgets
void
Am_Widget_General_Undo_Redo(Am_Object command_obj, bool undo, bool selective)
{
  Am_Value new_value, old_value;
  Am_Object inter, widget;
  // this command was in the inter in the widget.  The SAVED_OLD_OWNER
  // will be the inter.  Want to actually set the widget.  The
  // old_value in the command object is for the widget, not for the
  // interactor.  The interactor's value is reset by impl_command.
  inter = command_obj.Get(Am_SAVED_OLD_OWNER);
  if (inter.Valid())
    widget = inter.Get_Owner();

  if (selective) { // then get current value from the interactor
    if (widget.Valid())
      new_value = widget.Peek(Am_VALUE);
  } else // get current value from the command_obj
    new_value = command_obj.Peek(Am_VALUE);

  if (undo)
    old_value = command_obj.Peek(Am_OLD_VALUE);
  else // repeat
    old_value = command_obj.Peek(Am_VALUE);

  command_obj.Set(Am_OLD_VALUE, new_value);
  command_obj.Set(Am_VALUE, old_value);
  //also set widget
  if (widget.Valid()) {
#ifdef DEBUG
    if (Am_Inter_Tracing(Am_INTER_TRACE_SETTING)) {
      std::cout << "++ ";
      if (selective)
        std::cout << "selective ";
      if (undo)
        std::cout << "undo ";
      else
        std::cout << "repeat ";
      std::cout << "setting the Am_VALUE of " << widget << " to " << old_value
                << std::endl
                << std::flush;
    }
#endif
    widget.Set(Am_OLD_VALUE, new_value, Am_OK_IF_NOT_THERE);
    widget.Set(Am_VALUE, old_value);
  }
}

Am_Define_Method(Am_Object_Method, void, Am_Widget_Inter_Command_Undo,
                 (Am_Object command_obj))
{
  Am_Widget_General_Undo_Redo(command_obj, true, false);
}
Am_Define_Method(Am_Object_Method, void, Am_Widget_Inter_Command_Selective_Undo,
                 (Am_Object command_obj))
{
  Am_Widget_General_Undo_Redo(command_obj, true, true);
}
Am_Define_Method(Am_Object_Method, void,
                 Am_Widget_Inter_Command_Selective_Repeat,
                 (Am_Object command_obj))
{
  Am_Widget_General_Undo_Redo(command_obj, false, true);
}

Am_Define_Font_Formula(Am_Font_From_Owner)
{
  return self.Get_Owner().Get(Am_FONT);
}

Am_Define_Formula(int, Am_Left_From_Owner)
{
  return self.Get_Owner().Get(Am_LEFT);
}

Am_Define_Formula(int, Am_Top_From_Owner)
{
  return self.Get_Owner().Get(Am_TOP);
}

Am_Define_Formula(int, Am_Width_From_Owner)
{
  return self.Get_Owner().Get(Am_WIDTH);
}

Am_Define_Formula(int, Am_Height_From_Owner)
{
  return self.Get_Owner().Get(Am_HEIGHT);
}

/******************************************************************************
 * Am_Get_Real_String_Or_Obj
 *   Formula to get the real object or string to use in the widget. It can be
 *   directly in the Am_COMMAND slot or there can be a command object there,
 *   and then the real value is in the command's Am_LABEL slot.
 */

Am_Define_Formula(Am_Value, Am_Get_Real_String_Or_Obj)
{
  Am_Value value, v;
  Am_Object obj, command;
  value = self.Peek(Am_COMMAND);
  if (!value.Valid()) {
    // The Am_COMMAND slot is empty, just return (0L)
    value = Am_No_Object;
    return value;
  } else if (value.type == Am_OBJECT) {
    obj = value;
    if (obj.Is_Instance_Of(Am_Command)) {
      command = obj;
      obj = Am_No_Object;
      if (command.Is_Instance_Of(Am_Menu_Line_Command))
        value = true; // this takes care of the menu line case
      else {          // get the value out of the command object
        value = command.Peek(Am_LABEL);
        if (value.type == Am_STRING)
          ; // have a string in value
        else if (value.type == Am_OBJECT)
          obj = value; // have a new object
        else {         //generate a string from the value by printing it
          char line[100];
          OSTRSTREAM_CONSTR(oss, line, 100, std::ios::out);
          oss << value << std::ends;
          OSTRSTREAM_COPY(oss, line, 100);
          value = line; //now use the string as the value
        }
      }
    } else { //not a command, must be a regular object
      if (!obj.Is_Instance_Of(Am_Graphical_Object))
        Am_ERRORO("Object " << obj << " being added as part of widget " << self
                            << " should be a Command or Graphical Object",
                  self, Am_COMMAND);
      value = obj;
    }
  } //else not an object
  else if (value.type == Am_STRING)
    ;    // value is already set correctly
  else { //generate a string from the value by printing it
    char line[100];
    OSTRSTREAM_CONSTR(oss, line, 100, std::ios::out);
    oss << value << std::ends;
    OSTRSTREAM_COPY(oss, line, 100);
    value = line; //now use the string as the value
  }

  // now deal with the components list

  v = self.Peek(Am_ATTACHED_OBJECT, Am_NO_DEPENDENCY);
  Am_Object old_part;
  if (v.Valid()) {
    old_part = v;
#ifdef DEBUG
    if (!old_part.Is_Part_Of(self)) {
      Am_ERRORO("old part " << old_part
                            << " in Am_ATTACHED_OBJECT not part of me " << self,
                self, Am_ATTACHED_OBJECT);
    }
#endif
  }

  if (obj != old_part) { //otherwise, if ==, then all OK

    //since a button is an aggregate and not a group, have to set the
    //parts list explicitly
    Am_Value_List graphical_parts_list;

    if (old_part.Valid()) {
      self.Remove_Part(old_part);
      //SPACE LEAK, sometimes should delete old_part
    }
    if (obj.Valid()) {
      Am_Object obj_owner = obj.Get_Owner(Am_NO_DEPENDENCY);
      //make sure obj not in another group (or button) already
      if (obj_owner.Valid() && (obj_owner != self)) {
        // then obj is already in another object, use an instance of it
        obj = obj.Create();
        value = obj; // the return value from this formula is the new object
        obj_owner = Am_No_Value;
      }
      if (!obj_owner.Valid()) { //might be valid if owner is self
        self.Add_Part(Am_NO_INHERIT, obj);
      }
      graphical_parts_list.Add(obj);
      obj.Set(Am_RANK, 0); //needed by embedded interactors, but might
      //not be set since I am not a Am_Group
      self.Set(Am_ATTACHED_OBJECT, obj, Am_OK_IF_NOT_THERE)
          .Set_Inherit_Rule(Am_ATTACHED_OBJECT, Am_LOCAL);
    } else //just remove old object
      self.Set(Am_ATTACHED_OBJECT, Am_No_Object, Am_OK_IF_NOT_THERE);

    self.Set(Am_GRAPHICAL_PARTS, graphical_parts_list, Am_OK_IF_NOT_THERE)
        .Set_Inherit_Rule(Am_GRAPHICAL_PARTS, Am_LOCAL);
  }

  return value;
}

////////////////////////////////////////////////////////////////////////
// Am_Scrolling_Menu
////////////////////////////////////////////////////////////////////////

//ref object better be valid
static void
calc_ranks(int &startSet, int &stopSet, int &startErase, int &stopErase,
           Am_Object &old_object, Am_Object &new_object, Am_Object &ref_object)
{
  startSet = -1;
  stopSet = -1;
  startErase = -1;
  stopErase = -1;
  int ref_rank = ref_object.Get(Am_RANK);
  if (old_object.Valid()) {
    if (!new_object.Valid())
      return;
    int old_rank = old_object.Get(Am_RANK);
    int new_rank = new_object.Get(Am_RANK);
    if (ref_rank <= old_rank && old_rank < new_rank) {
      // ref < new < old
      startSet = old_rank + 1;
      stopSet = new_rank;
    } else if (ref_rank <= new_rank && new_rank < old_rank) {
      // ref < new < old
      startErase = new_rank + 1;
      stopErase = old_rank;
    } else if (new_rank < ref_rank && ref_rank < old_rank) {
      // new < ref < old
      startSet = new_rank;
      stopSet = ref_rank - 1;
      startErase = ref_rank + 1;
      stopErase = old_rank;
    } else if (old_rank < ref_rank && ref_rank < new_rank) {
      // old < ref < new
      startSet = ref_rank + 1;
      stopSet = new_rank;
      startErase = old_rank;
      stopErase = ref_rank - 1;
    } else if (old_rank < new_rank && new_rank <= ref_rank) {
      // old < new < ref
      startErase = old_rank;
      stopErase = new_rank - 1;
    } else if (new_rank < old_rank && old_rank <= ref_rank) {
      // new < old < ref
      startSet = new_rank;
      stopSet = old_rank - 1;
    }
  } else { //no old object, go from ref to new inclusive
    if (!new_object.Valid()) {
      startSet = ref_rank;
      stopSet = ref_rank;
    } else {
      int new_rank = new_object.Get(Am_RANK);
      if (ref_rank < new_rank) {
        startSet = ref_rank;
        stopSet = new_rank;
      } else {
        startSet = new_rank;
        stopSet = ref_rank;
      }
    }
  }
  // std::cout << "Calc rank returning startSet " << startSet
  //     << " stopSet " << stopSet << " startErase " << startErase
  //     << " stopErase " << stopErase <<std::endl <<std::flush;
}

void
list_set_and_clear(Am_Object &inter, Am_Object &old_object,
                   Am_Object &new_object, Am_Object &ref_object)
{
  Am_Value_List all_objects = inter.Get_Owner().Get(Am_GRAPHICAL_PARTS);
  Am_Object current;

  int startSet, stopSet, startErase, stopErase;
  calc_ranks(startSet, stopSet, startErase, stopErase, old_object, new_object,
             ref_object);
  if (startErase != -1) {
    int i;
    for (all_objects.Start(), i = 0; i < startErase; all_objects.Next(), i++)
      ;
    for (; i <= stopErase; all_objects.Next(), i++) {
      current = all_objects.Get();
      Am_REPORT_SET_SEL_VALUE(true, inter, current, false);
      current.Set(Am_SELECTED, false, Am_OK_IF_NOT_THERE);
    }
  }
  if (startSet != -1) {
    int i;
    for (all_objects.Start(), i = 0; i < startSet; all_objects.Next(), i++)
      ;
    for (; i <= stopSet; all_objects.Next(), i++) {
      current = all_objects.Get();
      if (current.Get(Am_ACTIVE).Valid()) {
        Am_REPORT_SET_SEL_VALUE(true, inter, current, true);
        current.Set(Am_SELECTED, true, Am_OK_IF_NOT_THERE);
      }
    }
  }
}

void
clear_all_objects_selected(Am_Object &inter)
{
  Am_Value_List all_objects = inter.Get_Owner().Get(Am_GRAPHICAL_PARTS);
  Am_Object current;
  for (all_objects.Start(); !all_objects.Last(); all_objects.Next()) {
    current = all_objects.Get();
    Am_REPORT_SET_SEL_VALUE(true, inter, current, false);
    current.Set(Am_SELECTED, false, Am_OK_IF_NOT_THERE);
  }
}

// called when start continuous
Am_Define_Method(Am_Object_Method, void, Am_Choice_List_Start_Do,
                 (Am_Object inter))
{
  inter.Set(Am_OLD_INTERIM_VALUE, 0);
  inter.Set(Am_INTERIM_VALUE, 0);
  inter.Set(Am_LAST_USED_OLD_INTERIM_VALUE, 0, Am_OK_IF_NOT_THERE);
  Am_Choice_How_Set how_set = inter.Get(Am_HOW_SET);
  if (how_set == Am_CHOICE_LIST_TOGGLE) {
    Am_Input_Char ic = inter.Get(Am_START_CHAR);
    bool extend_from_orig = ic.shift;
    bool separate_extend = ic.control;
    Am_Value old_ref = inter.Peek(Am_INITIAL_REF_OBJECT);
    inter.Set(Am_LAST_INITIAL_REF_OBJECT,
              old_ref.Exists() ? old_ref : Am_No_Value, Am_OK_IF_NOT_THERE);
    if (!extend_from_orig)
      inter.Set(Am_INITIAL_REF_OBJECT, Am_No_Object, Am_OK_IF_NOT_THERE);
    if (!separate_extend)
      clear_all_objects_selected(inter);
  } else {
    clear_all_objects_selected(inter);
  }
}

//current object should be already set into Am_INTERIM_VALUE and old
//value in Am_LAST_USED_OLD_INTERIM_VALUE
//modified from Am_Choice_Interim_Do in inter_choice.cc
Am_Define_Method(Am_Object_Method, void, Am_Choice_List_Interim_Do,
                 (Am_Object inter))
{
  Am_Choice_How_Set how_set = inter.Get(Am_HOW_SET);
  if (how_set == Am_CHOICE_LIST_TOGGLE) {
    Am_Object new_object = inter.Get(Am_INTERIM_VALUE);
    if (!new_object.Valid())
      return; //don't do anything if outside

    Am_Object ref_object = inter.Get(Am_INITIAL_REF_OBJECT);
    Am_Object old_object = inter.Get(Am_LAST_USED_OLD_INTERIM_VALUE);

    if (!ref_object.Valid()) {
      ref_object = new_object;
      inter.Set(Am_INITIAL_REF_OBJECT, ref_object, Am_OK_IF_NOT_THERE);
    }
    list_set_and_clear(inter, old_object, new_object, ref_object);
    inter.Set(Am_LAST_USED_OLD_INTERIM_VALUE, new_object);
  } else {
    //use the standard method
    Am_Object_Method method = Am_Choice_Interactor.Get(Am_INTERIM_DO_METHOD);
    method.Call(inter);
  }
}

void
set_selected_from_list(Am_Object &inter, Am_Value_List &all_objects,
                       Am_Value_List &new_value)
{
  Am_Object current;
  bool old_sel;
  for (all_objects.Start(); !all_objects.Last(); all_objects.Next()) {
    current = all_objects.Get();
    old_sel = current.Get(Am_SELECTED, Am_RETURN_ZERO_ON_ERROR);
    new_value.Start();
    if (new_value.Member(current)) {
      if (!old_sel) {
        Am_REPORT_SET_SEL_VALUE(true, inter, current, true);
        current.Set(Am_SELECTED, true, Am_OK_IF_NOT_THERE);
      }
    } else {
      if (old_sel) {
        Am_REPORT_SET_SEL_VALUE(true, inter, current, false);
        current.Set(Am_SELECTED, false, Am_OK_IF_NOT_THERE);
      }
    }
  }
}

Am_Value_List
list_final_value(Am_Object &inter, Am_Object &new_object, Am_Object &ref_object,
                 Am_Value_List &old_value, bool separate_extend)
{
  Am_Object widget = inter.Get_Owner();
  Am_Value_List all_objects = widget.Get(Am_GRAPHICAL_PARTS);
  Am_Value_List new_value;
  Am_Object current;

  int startSet, stopSet, startErase, stopErase;
  calc_ranks(startSet, stopSet, startErase, stopErase, Am_No_Object, new_object,
             ref_object);
  all_objects.Start();
  if (startSet != -1) {
    int i;
    for (i = 0; i < startSet; all_objects.Next(), i++) {
      if (separate_extend) { //otherwise, just ignore these
        current = all_objects.Get();
        old_value.Start();
        if (old_value.Member(current)) {
          if (current.Get(Am_ACTIVE).Valid())
            new_value.Add(current);
        }
      }
    }
    for (; i <= stopSet; all_objects.Next(), i++) {
      current = all_objects.Get();
      if (current.Get(Am_ACTIVE).Valid())
        new_value.Add(current);
    }
  }
  if (separate_extend) { //process the rest of the list
    //start from current position in the list
    for (; !all_objects.Last(); all_objects.Next()) {
      current = all_objects.Get();
      old_value.Start();
      if (old_value.Member(current))
        if (current.Get(Am_ACTIVE).Valid())
          new_value.Add(current);
    }
  }
  //now new_value has the final value
  set_selected_from_list(inter, all_objects, new_value);
  return new_value;
}

Am_Define_Method(Am_Object_Method, void, Am_Choice_List_Abort_Do,
                 (Am_Object inter))
{
  Am_Value old_value = inter.Peek(Am_VALUE);
  Am_Choice_How_Set how_set = inter.Get(Am_HOW_SET);
  if (how_set != Am_CHOICE_LIST_TOGGLE) {
    //use the standard method to remove the interim selected
    Am_Object_Method method = Am_Choice_Interactor.Get(Am_ABORT_DO_METHOD);
    method.Call(inter);
  }
  Am_Value_List old_value_list;
  if (Am_Value_List::Test(old_value)) {
    old_value_list = old_value;
    inter.Set(Am_INITIAL_REF_OBJECT, inter.Peek(Am_LAST_INITIAL_REF_OBJECT));
  } else if (old_value.type == Am_OBJECT) {
    Am_Object current = old_value;
    Am_REPORT_SET_SEL_VALUE(true, inter, current, true);
    current.Set(Am_SELECTED, true, Am_OK_IF_NOT_THERE);
    old_value_list.Add(old_value);
  }
  Am_Value_List all_objects = inter.Get_Owner().Get(Am_GRAPHICAL_PARTS);
  set_selected_from_list(inter, all_objects, old_value_list);
}

//modified from Am_Choice_Do in inter_choice.cc
Am_Define_Method(Am_Object_Method, void, Am_Choice_List_Do, (Am_Object inter))
{
  Am_Choice_How_Set how_set = inter.Get(Am_HOW_SET);
  Am_Value old_value = inter.Get(Am_VALUE, Am_RETURN_ZERO_ON_ERROR);
  Am_Value_List old_value_list;
  if (Am_Value_List::Test(old_value))
    old_value_list = old_value;
  else if (old_value.type == Am_OBJECT)
    old_value_list.Add(old_value);
  Am_Object new_object = inter.Get(Am_INTERIM_VALUE);
  if (how_set == Am_CHOICE_LIST_TOGGLE) {
    Am_Object ref_object = inter.Get(Am_INITIAL_REF_OBJECT);
    Am_Input_Char ic = inter.Get(Am_START_CHAR);
    bool separate_extend = ic.control;
    if (!ref_object.Valid())
      ref_object = new_object;
    else if (!new_object.Valid())
      new_object = inter.Get(Am_LAST_USED_OLD_INTERIM_VALUE);
    // clears interim value and sets new value
    Am_Value_List new_value = list_final_value(inter, new_object, ref_object,
                                               old_value_list, separate_extend);
    inter.Set(Am_VALUE, new_value, Am_OK_IF_NOT_THERE);
    inter.Set(Am_OBJECT_MODIFIED, ref_object, Am_OK_IF_NOT_THERE);
  } else {
    Am_REPORT_SET_SEL_VALUE(false, inter, new_object, false);
    new_object.Set(Am_INTERIM_SELECTED, false, Am_OK_IF_NOT_THERE);
    old_value_list.Start();
    if ((how_set == Am_CHOICE_TOGGLE && old_value_list.Member(new_object)) ||
        how_set == Am_CHOICE_CLEAR)
      inter.Set(Am_VALUE, Am_No_Object); // empty
    else {
      if (new_object.Valid()) {
        Am_REPORT_SET_SEL_VALUE(true, inter, new_object, true);
        new_object.Set(Am_SELECTED, true, Am_OK_IF_NOT_THERE);
      }
      inter.Set(Am_VALUE, new_object, Am_OK_IF_NOT_THERE);
    }
    inter.Set(Am_OBJECT_MODIFIED, new_object, Am_OK_IF_NOT_THERE);
  }
  inter.Set(Am_OLD_VALUE, old_value, Am_OK_IF_NOT_THERE);
  Am_Copy_Values_To_Command(inter);
}

Am_Define_Formula(int, scroll_menu_v_scroll_left)
{
  Am_Object group = self.Get_Owner();
  Am_Widget_Look look = self.Get(Am_WIDGET_LOOK);
  int scroll_border = (look == Am_MOTIF_LOOK) ? -1 : -3;
  if ((bool)group.Get(Am_V_SCROLL_BAR_ON_LEFT))
    return -scroll_border;
  else // on right
    return (int)group.Get(Am_WIDTH) - (int)self.Get(Am_WIDTH) + scroll_border;
}
Am_Define_Formula(int, scroll_menu_h_scroll_top)
{
  Am_Object group = self.Get_Owner();
  Am_Widget_Look look = self.Get(Am_WIDGET_LOOK);
  int scroll_border = (look == Am_MOTIF_LOOK) ? -1 : -3;
  if ((bool)group.Get(Am_H_SCROLL_BAR_ON_TOP))
    return -scroll_border;
  else // on bottom
    return (int)group.Get(Am_HEIGHT) - (int)self.Get(Am_HEIGHT) + scroll_border;
}

//normally, outside stop aborts, but for this widget, it should just stop
Am_Define_Method(Am_Inter_Internal_Method, void,
                 Am_Choice_List_Outside_Stop_Method,
                 (Am_Object & inter, Am_Object &object,
                  Am_Object & /* event_window */, Am_Input_Event * /* ev */))
{
  Am_INTER_TRACE_PRINT(inter, "Choice List outside STOP");
  Am_Stop_Interactor(inter, object);
}

Am_Define_Formula(int, scroll_sub_menu_set_old_owner)
{
  Am_Object scroll_menu_widget = self.Get_Owner();
  int ret = 0;
  if (scroll_menu_widget.Valid()) {
    Am_Value_List parts = self.Get(Am_GRAPHICAL_PARTS);
    ret = set_parts_list_commands_old_owner(parts, scroll_menu_widget);
  }
  return ret;
}

//turn off selection if click in the background
Am_Define_Method(Am_Object_Method, void, scroll_menu_click_background,
                 (Am_Object inter))
{
  Am_Object scroll_menu_widget = inter.Get_Owner();
  scroll_menu_widget.Set(Am_VALUE, Am_No_Value);
}

/******************************************************************************
 * Initializing
 *****************************************************************************/

// exported objects

Am_Object Am_Menu_Line_Command = 0;

// internal objects

Am_Object Am_Pop_Up_Menu_From_Widget_Proto; //internal
Am_Object Am_Scrolling_Menu;

static void
init()
{

  Am_Object inter; // interactor in the widget
  Am_Object command_obj;
  Am_Object_Advanced obj_adv; // to get at advanced features like
                              // local-only and demons.
  Am_Demon_Set demons;

  //////////// Command Objects ////////////////////////////

  Am_Menu_Line_Command =
      Am_Command.Create(DSTR("Menu_Line_Command"))
          .Set(Am_LABEL, "Menu_Line_Command")
          .Set(Am_ACTIVE, false) // menu lines aren't active menu members.
          .Set(Am_VALUE, (0L));

  // Am_Button was here

  // Am_Check_Button was here

  // Am_Radio_Button was here

  // Am_Menu_Item was here

  // Am_Button_Panel was here

  // Am_Radio_Button_Panel was here

  // Am_Checkbox_Panel was here

  // Am_Menu was here

  // Am_Menu_Bar was here

  // Am_Option_Button was here

  // Am_Pop_Up_Menu_Interactor was here

  // Am_Scrolling_Menu was here

  ///////////////////////////////////////////////////////////////////////////
  // Scrolling_Menu  (Called ListBox on PC)
  ///////////////////////////////////////////////////////////////////////////

  // Based on menu, but use white background

  Am_Menu_In_Scrolling_Menu =
      Am_Menu.Create(DSTR("Am_Menu_In_Scrolling_Menu"))
          .Set(Am_HOW_SET, Am_From_Owner(Am_HOW_SET))
          .Set(Am_FIXED_WIDTH, true)
          .Set(Am_VALUE, Am_From_Owner(Am_VALUE).Multi_Constraint())
          .Set(Am_FIXED_HEIGHT, false)
          .Set(Am_FINAL_FEEDBACK_WANTED, true)
          .Set(Am_ITEMS, Am_From_Owner(Am_ITEMS))
          .Set(Am_WIDTH, Am_Width_Of_Parts)
          .Set(Am_HEIGHT, Am_Height_Of_Parts)
          .Set(Am_LAYOUT, Am_Vertical_Layout)
          .Set(Am_H_ALIGN, Am_LEFT_ALIGN)
          .Set(Am_ACTIVE, Am_From_Owner(Am_ACTIVE))
          .Set(Am_ACTIVE_2, Am_From_Owner(Am_ACTIVE_2))
          .Set(Am_SET_COMMAND_OLD_OWNER, scroll_sub_menu_set_old_owner)
          //always use Mac Look since want white background
          .Set(Am_WIDGET_LOOK, Am_MACINTOSH_LOOK);
  Am_Scrolling_Menu =
      Am_Scrolling_Group.Create(DSTR("Am_Scrolling_Menu"))
          .Add(Am_VALUE, Am_From_Part(Am_SUB_MENU, Am_VALUE).Multi_Constraint())
          .Add(Am_HOW_SET, Am_CHOICE_LIST_TOGGLE)
          .Set(Am_FILL_STYLE, Am_Default_Color)
          .Set(Am_INNER_FILL_STYLE, Am_White)
          .Set(Am_H_SCROLL_BAR, false)
          .Set(Am_V_SCROLL_BAR, true)
          .Add(Am_ITEMS, (0L))
          .Set(Am_INNER_WIDTH, Am_Width_Of_Parts)
          .Set(Am_INNER_HEIGHT, Am_Height_Of_Parts)
          .Set(Am_ACTIVE, Am_Active_From_Command)
          .Set(Am_ACTIVE_2, true)
          .Add_Part(Am_SUB_MENU, Am_Menu_In_Scrolling_Menu)
          .Set_Part(Am_COMMAND,
                    Am_Command.Create(DSTR("Command_In_Scrolling_Menu"))
                        .Set(Am_LABEL, "Scrolling_Menu_Command"))
          .Add_Part(Am_BACKGROUND_INTERACTOR,
                    Am_One_Shot_Interactor
                        .Create(DSTR("Scroll_menu_click_background"))
                        .Set(Am_PRIORITY, -1.0) //lower than the others
                        .Set(Am_DO_METHOD, scroll_menu_click_background));
  Am_Menu_In_Scrolling_Menu.Remove_Part(Am_COMMAND);
  Am_Menu_In_Scrolling_Menu.Add(Am_COMMAND, Am_From_Owner(Am_COMMAND));
  Am_Scrolling_Menu.Get_Object(Am_V_SCROLLER)
      .Set(Am_LEFT, scroll_menu_v_scroll_left);
  Am_Scrolling_Menu.Get_Object(Am_H_SCROLLER)
      .Set(Am_TOP, scroll_menu_h_scroll_top);
  Am_Scrolling_Menu.Get_Object(Am_V_SCROLLER)
      .Get_Object(Am_COMMAND)
      .Set(Am_IMPLEMENTATION_PARENT, Am_NOT_USUALLY_UNDONE);
  Am_Scrolling_Menu.Get_Object(Am_H_SCROLLER)
      .Get_Object(Am_COMMAND)
      .Set(Am_IMPLEMENTATION_PARENT, Am_NOT_USUALLY_UNDONE);
  Am_Scrolling_Menu.Get_Object(Am_SUB_MENU)
      .Get_Object(Am_ITEM_PROTOTYPE)
      .Set_Name(DSTR("Scrolling_Menu_Item"))
      .Set(Am_MENU_ITEM_LEFT_OFFSET, 1) //always use value of 1 as left offset
      .Set(Am_CHECKED_ITEM, false)
      .Set(Am_LEAVE_ROOM_FOR_FRINGE, false);
  Am_Scrolling_Menu.Get_Object(Am_SUB_MENU)
      .Get_Object(Am_INTERACTOR)
      .Set(Am_START_DO_METHOD, Am_Choice_List_Start_Do)
      .Set(Am_INTERIM_DO_METHOD, Am_Choice_List_Interim_Do)
      .Set(Am_ABORT_DO_METHOD, Am_Choice_List_Abort_Do)
      .Set(Am_DO_METHOD, Am_Choice_List_Do)
      .Set(Am_INTER_OUTSIDE_STOP_METHOD, Am_Choice_List_Outside_Stop_Method);
}

//scrolling_menu in buttons uses scrollbars, so bigger than scroll_widgets
static Am_Initializer *initializer =
    new Am_Initializer(DSTR("Button_Widgets"), init, 5.21f);
