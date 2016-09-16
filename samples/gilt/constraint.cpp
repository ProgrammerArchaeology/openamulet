/************************************************************************************
 *   This file contains an implementation of Constraint_Widget which is
 *   implemented using Am_Selection_Widget
 ***********************************************************************************/
#include <amulet.h>
#include "constraint.h"
#include "externs.h"

#include <amulet/widgets_advanced.h>
#include <amulet/inter_advanced.h> //needed for Am_Inter_Tracing
#include <amulet/gem.h>
#include <amulet/registry.h>
#include <amulet/opal_advanced.h>
#include <amulet/object.h>
#include <amulet/object_advanced.h>
#include <amulet/formula.h>
#include <amulet/formula_advanced.h>
#include <amulet/value_list.h>
#include <amulet/debugger.h>
#include <amulet/standard_slots.h>

using namespace std;

extern Am_Formula compute_drag_start_when;
extern Am_Object line_proto;
extern Am_Object Display_Text_Proto;
extern Am_Slot_Key MAIN_TEXT_GROUP;
extern Am_Slot_Key REF_COUNT;

#define C_HANDLE_SIZE 12
#define HANDLE_SPACING 19
#define C_HANDLE_SIZE_D2 6
#define BLACK_HANDLE_SIZE 9
#define NOT_SELECTED 0
#define PRIMARY 1
#define SECONDARY 2
#define BOTH_STATE 3
#define CONSTRAINT_VARIABLES_CNT 52

//int X1_COUNT, X2_COUNT, Y1_COUNT, Y2_COUNT, WIDTH_COUNT;
//int HEIGHT_COUNT, LEFT_COUNT, TOP_COUNT;

Am_Slot_Key PRIMARY_OBJ = Am_Register_Slot_Name("PRIMARY_OBJ");
Am_Slot_Key PRIMARY_HANDLE = Am_Register_Slot_Name("PRIMARY_HANDLE");
Am_Slot_Key SECONDARY_OBJ = Am_Register_Slot_Name("SECONDARY_OBJ");
Am_Slot_Key SECONDARY_HANDLE = Am_Register_Slot_Name("SECONDARY_HANDLE");
Am_Slot_Key CONSTRAINT_SETUP_INTERACTOR =
    Am_Register_Slot_Name("CONSTRAINT_SETUP_INTERACTOR");
Am_Slot_Key TEXT_HANDLE = Am_Register_Slot_Name("TEXT_HANDLE");
Am_Slot_Key CENTER_X = Am_Register_Slot_Name("CENTER_X");
Am_Slot_Key LEFT_MOST_HANDLE = Am_Register_Slot_Name("LEFT_MOST_HANDLE");
Am_Slot_Key TOP_HANDLE = Am_Register_Slot_Name("TOP_HANDLE");
Am_Slot_Key LEFT_HANDLE = Am_Register_Slot_Name("LEFT_HANDLE");
Am_Slot_Key WIDTH_HANDLE = Am_Register_Slot_Name("WIDTH_HANDLE");
Am_Slot_Key HEIGHT_HANDLE = Am_Register_Slot_Name("HEIGHT_HANDLE");
Am_Slot_Key Y1_HANDLE = Am_Register_Slot_Name("Y1_HANDLE");
Am_Slot_Key Y2_HANDLE = Am_Register_Slot_Name("Y2_HANDLE");
Am_Slot_Key X1_HANDLE = Am_Register_Slot_Name("X1_HANDLE");
Am_Slot_Key X2_HANDLE = Am_Register_Slot_Name("X2_HANDLE");
Am_Slot_Key CX_HANDLE = Am_Register_Slot_Name("CX_HANDLE");
Am_Slot_Key CY_HANDLE = Am_Register_Slot_Name("CY_HANDLE");
Am_Slot_Key RIGHT_HANDLE = Am_Register_Slot_Name("RIGHT_HANDLE");
Am_Slot_Key BOTTOM_HANDLE = Am_Register_Slot_Name("BOTTOM_HANDLE");
Am_Slot_Key MAIN_LINE = Am_Register_Slot_Name("MAIN_LINE");
Am_Slot_Key STATE = Am_Register_Slot_Name("STATE");
Am_Slot_Key OFFSET_INPUT = Am_Register_Slot_Name("OFFSET_INPUT");
Am_Slot_Key PERCENT_INPUT = Am_Register_Slot_Name("PERCENT_INPUT");
Am_Slot_Key CONSTRAINT_VAR = Am_Register_Slot_Name("CONSTRAINT_VAR");
Am_Slot_Key HANDLE_DEPENDS = Am_Register_Slot_Name("HANDLE_DEPENDS");

extern Am_Object Offset_Percentage_Dialog;
Am_Object Constraint_Widget;
Am_Object One_Constraint_Handle;
Am_Object handle_proto;

AM_DEFINE_ENUM_SUPPORT(Constraint_How_Set,
                       "CONSTRAINT_SETUP ABORT_SETUP MULTIPLE_SETUP");
AM_DEFINE_ENUM_SUPPORT(Which_Constraint_Handle,
                       "NO_HANDLE CH_TOP CH_LEFT CH_BOTTOM CH_RIGHT "
                       "CH_HEIGHT CH_WIDTH CH_CENTER_X CH_CENTER_Y "
                       "CH_X1 CH_X2 CH_Y1 CH_Y2 CH_SPECIAL ");

AM_DEFINE_METHOD_TYPE_IMPL(Add_Handle_Method);

//////////////////// Variable stuff //////////////

class Gilt_Constraint_Variable_Data : public Am_Wrapper
{
  AM_WRAPPER_DATA_DECL(Gilt_Constraint_Variable)
public:
  Gilt_Constraint_Variable_Data();
  Gilt_Constraint_Variable_Data(Gilt_Constraint_Variable_Data *proto);
  //never test
  bool operator==(Gilt_Constraint_Variable_Data &test) { return true; }
  char Get_Next_Var(Am_Object obj);
  void Release_Var(char var)
  {
    int diff;
    if ((short)var > (short)'Z')
      diff = 26 + (short)var - (short)'a';
    else
      diff = (short)var - (short)'A';
    cout << "Releasing " << var << " at index " << diff << endl;
    variables[diff].busy = false;
    variables[diff].object = NULL;
  }

protected:
  int index;
  constraint_variable variables[CONSTRAINT_VARIABLES_CNT];
};

AM_WRAPPER_DATA_IMPL(Gilt_Constraint_Variable, (this))

Gilt_Constraint_Variable_Data::Gilt_Constraint_Variable_Data()
{
  int i = 0;
  char var = 'A';
  index = 0;

  for (; i < CONSTRAINT_VARIABLES_CNT; i++) {
    variables[i].object = NULL;
    variables[i].var = var;
    variables[i].busy = false;
    var++;
    if (var > 90)
      var = 97;
  }
}
Gilt_Constraint_Variable_Data::Gilt_Constraint_Variable_Data(
    Gilt_Constraint_Variable_Data *proto)
{
  int i = 0;
  int count = CONSTRAINT_VARIABLES_CNT;
  while (i < count) {
    variables[i].object = proto->variables[i].object;
    variables[i].var = proto->variables[i].var;
    variables[i].busy = proto->variables[i].busy;
    i++;
  }
}

char
Gilt_Constraint_Variable_Data::Get_Next_Var(Am_Object obj)
{
  int count = CONSTRAINT_VARIABLES_CNT;
  int current = index;
  constraint_variable var = variables[index];

  if (var.busy) {
    for (index++, index %= count, var = variables[index];
         var.busy && index != current;
         index++, index %= count, var = variables[index])
      ;

    if (index == current && var.busy) {
      Am_Error("Can not allocate variables");
      return 0;
    }
    variables[index].object = &obj;
    variables[index].busy = true;
    return var.var;
  } else {
    variables[index].object = &obj;
    variables[index].busy = true;
    index++;
    return var.var;
  }
}

//////////////////// New formula stuff //////////

class Gilt_Inferred_Object_Offset_Store_Data : public Am_Wrapper
{
  AM_WRAPPER_DATA_DECL(Gilt_Inferred_Object_Offset_Store)
public:
  Gilt_Inferred_Object_Offset_Store_Data(
      Am_Object in_object, Am_Slot_Key in_obj_key, Am_Slot_Key in_obj_ref_key,
      int in_obj_offset, float in_obj_multiplier, Am_Slot_Key in_self_key,
      float in_self_multiplier)
  {
    object = in_object;
    obj_key = in_obj_key;
    obj_ref_key = in_obj_ref_key;
    obj_offset = in_obj_offset;
    obj_multiplier = in_obj_multiplier;
    self_key = in_self_key;
    self_multiplier = in_self_multiplier;
  }
  Gilt_Inferred_Object_Offset_Store_Data(
      Gilt_Inferred_Object_Offset_Store_Data *proto)
  {
    object = proto->object;
    obj_key = proto->obj_key;
    obj_ref_key = proto->obj_ref_key;
    obj_offset = proto->obj_offset;
    obj_multiplier = proto->obj_multiplier;
    self_key = proto->self_key;
    self_multiplier = proto->self_multiplier;
  }
  bool operator==(Gilt_Inferred_Object_Offset_Store_Data &test)
  {
    return (object == test.object && obj_key == test.obj_key &&
            obj_ref_key == test.obj_ref_key && obj_offset == test.obj_offset &&
            obj_multiplier == test.obj_multiplier &&
            self_key == test.self_key &&
            self_multiplier == test.self_multiplier);
  }

  Am_Object object;
  Am_Slot_Key obj_key;
  Am_Slot_Key obj_ref_key;
  int obj_offset;
  float obj_multiplier;
  Am_Slot_Key self_key;
  float self_multiplier;
};

AM_WRAPPER_DATA_IMPL(Gilt_Inferred_Object_Offset_Store, (this))

static Am_Value
get_data_and_adjust_procedure(Am_Object &self)
{
  Am_Value obj_value;
  Am_Value obj_ref_value;

  Gilt_Inferred_Object_Offset_Store_Data *store =
      (Gilt_Inferred_Object_Offset_Store_Data *)
          Am_Object_Advanced::Get_Context()
              ->Get_Data();

  obj_ref_value = (store->object).Get(store->obj_ref_key);
  if (store->obj_key)
    obj_value = (store->object).Get(store->obj_key);
  store->Release(); //if multiplier is not grayed out

  if ((obj_ref_value.type != Am_INT && obj_ref_value.type != Am_LONG) ||
      (obj_value.Valid() && obj_value.type != Am_INT &&
       obj_value.type != Am_LONG))
    Am_Error("This slot must contain int or long type");

  long int value = (long)(obj_value.Valid()) ? obj_value.value.long_value : 0;
  float mult = store->obj_multiplier;

  value +=
      (long)(mult * (float)obj_ref_value.value.long_value) + store->obj_offset;
  mult = store->self_multiplier;
  value += (long)(store->self_key)
               ? (long int)((float)self.Get(store->self_key) * mult)
               : 0;
  obj_ref_value.value.long_value = value;
  return obj_ref_value;
}

Am_Formula
Get_Data_And_Adjust(Am_Object object, Am_Slot_Key obj_key,
                    Am_Slot_Key obj_ref_key, int obj_offset,
                    float obj_multiplier, Am_Slot_Key key = Am_NO_SLOT,
                    float multiplier = -0.5)
{
  Am_Formula formula(get_data_and_adjust_procedure, "Get_Data_And_Adjust");
  formula.Set_Data(new Gilt_Inferred_Object_Offset_Store_Data(
      object, obj_key, obj_ref_key, obj_offset, obj_multiplier, key,
      multiplier));
  return formula;
}

/////////////// Conflict and loop checking stuff //////////////////////

// This function returns list of constraint in the slot
Am_Constraint_Iterator
find_constraint_in(Am_Object &obj, Am_Slot_Key key)
{
  Am_Object_Advanced obj_adv = (Am_Object_Advanced &)obj;
  Am_Slot slot_adv = obj_adv.Get_Slot_Locale(key).Get_Slot(key);
  Am_Constraint_Iterator const_iter(slot_adv);

  return const_iter;
}

// This function returns list of object and slots depends on this constraint
Am_Value_List
get_depends_list(Am_Formula_Advanced *formula)
{
  Am_Depends_Iterator dep_iter = formula;
  Am_Object dep_obj;
  Am_Slot dep_slot;
  Am_Slot_Key dep_key;
  Am_Value_List dep_list;

  for (dep_iter.Start(); !dep_iter.Last(); dep_iter.Next()) {
    dep_slot = dep_iter.Get();
    dep_key = dep_slot.Get_Key();
    dep_obj = (Am_Object)dep_slot.Get_Owner();

    dep_list.Add(Am_Value_List().Add(dep_obj).Add(dep_key));
  }
  return dep_list;
}

//forward declaration
Am_Slot_Key get_slot_key(Which_Constraint_Handle ch);

Constraint_How_Set
check_dependency(Am_Object &obj, Which_Constraint_Handle handle,
                 Am_Object window)
{
  Am_Slot_Key key;
  if (handle == CH_CENTER_X || handle == CH_RIGHT)
    key = Am_LEFT;
  else if (handle == CH_CENTER_Y || handle == CH_BOTTOM)
    key = Am_TOP;
  else
    key = get_slot_key(handle);
  Am_Constraint_Iterator const_iter = find_constraint_in(obj, key);
  Am_Value to_do = CONSTRAINT_SETUP;
  const_iter.Start();
  if (!const_iter.Last()) {
    Am_Value_List dep_list;
    Am_Constraint *constr = const_iter.Get();

    if (Am_Formula_Advanced::Test(constr)) {
      Am_Formula_Advanced *form = Am_Formula_Advanced::Narrow(constr);
      dep_list = get_depends_list(form);
    }
    Conflict_Window.Set(WORK_OBJECT, obj);
    Conflict_Window.Set(WORK_HANDLE, handle);
    Am_Object text_group = Conflict_Window.Get_Object(MAIN_TEXT_GROUP);
    Am_Object_Method method = text_group.Get(Am_DO_METHOD);
    Conflict_Window.Set(CONSTRAINT_INFO, dep_list);
    method.Call(text_group);
    Conflict_Window.Set(Am_LEFT, (int)window.Get(Am_LEFT) + 10)
        .Set(Am_TOP, (int)window.Get(Am_TOP) + 10);
    Am_Pop_Up_Window_And_Wait(Conflict_Window, to_do);
  }
  return (Constraint_How_Set)to_do;
}

////////////// Handles  ////////////////////

bool
slot_has_dependency(Am_Object handle, Am_Object for_obj)
{
  Which_Constraint_Handle wh = (Which_Constraint_Handle)handle.Get(Am_ID);
  Am_Value_List info = (Am_Value_List)for_obj.Get(CONSTRAINT_INFO);
  Am_Value_List tmp;
  for (info.Start(); !info.Last(); info.Next()) {
    tmp = (Am_Value_List)info.Get();
    tmp.Start();
    if ((Which_Constraint_Handle)tmp.Get() == wh)
      return true;
  }
  return false;
}

//forward declaration
int get_count_index(Which_Constraint_Handle wh);

int
get_ref_count(Am_Object obj, Which_Constraint_Handle wh)
{
  int index = get_count_index(wh);
  Am_Value_List ref_count = obj.Get(REF_COUNT);
  return (int)ref_count.Get_Nth(index);
}

Am_Object find_other_handle(Am_Value_List depends, Am_Object handle_group);

Am_Define_Value_List_Formula(get_handle_depends_list)
{
  Am_Object for_obj = self.Get_Owner().Get_Object(Am_ITEM);
  bool visible = (bool)self.Get(Am_VISIBLE);

  if (for_obj.Valid() && visible) {
    Am_Value_List c_info = (Am_Value_List)for_obj.Get(CONSTRAINT_INFO);
    Which_Constraint_Handle wh = (Which_Constraint_Handle)self.Get(Am_ID);
    bool found = false;
    Am_Value_List tmp;
    Am_Value_List constraint_info = Am_No_Value_List;
    Am_Value_List depends_list = Am_No_Value_List;

    for (c_info.Start(); !c_info.Last() && !found; c_info.Next()) {
      tmp = (Am_Value_List)c_info.Get();
      if ((Which_Constraint_Handle)tmp.Get_Nth(0) == wh)
        found = true;
    }

    if (found) {
      int offset = (int)tmp.Get_Nth(5);
      float percent = (float)tmp.Get_Nth(6);
      float mult = 1.0;
      if (wh == CH_CENTER_X || wh == CH_CENTER_Y)
        mult = 0.5;

      if (mult != percent || offset != 0)
        constraint_info = Am_Value_List().Add(offset).Add(percent);
      depends_list = Am_Value_List().Add(tmp.Get_Nth(1)).Add(tmp.Get_Nth(2));
    }

    self.Set(CONSTRAINT_INFO, constraint_info);
    return depends_list;
  } else {
    self.Set(CONSTRAINT_INFO, Am_No_Value);
    return Am_No_Value_List;
  }
}

Am_Define_Formula(Am_Value, get_handle_variable)
{
  Am_Object for_obj = self.Get_Owner().Get_Object(Am_ITEM);
  //When items are deselected, cv sometimes becomes NULL because owner
  //is lost
  if (for_obj.Valid() && (bool)self.Get(Am_VISIBLE)) {
    Am_Value_List const_info = self.Get(CONSTRAINT_INFO);
    Am_Value_List dep_list = self.Get(HANDLE_DEPENDS);
    bool char_needed = false, use_other_char = false;
    Which_Constraint_Handle wh = (Which_Constraint_Handle)self.Get(Am_ID);
    Am_Value my_char = self.Get(CONSTRAINT_VAR, Am_NO_DEPENDENCY);
    Gilt_Constraint_Variable_Data *cv =
        (Gilt_Constraint_Variable_Data *)(Am_Ptr)self.Get_Owner()
            .Get_Owner()
            .Get_Owner()
            .Get(CONSTRAINT_VAR);
    if (cv != NULL) {
      // my_char is true when this handle should have a variable
      // use_other_char is true when this handle should use variable of
      //another handle
      Am_Object other_handle;

      if (Am_Value(dep_list).Valid()) {
        other_handle = find_other_handle(dep_list, self.Get_Owner());

        if (Am_Value(const_info).Valid())
          char_needed = true;
        else if (other_handle.Valid()) {
          Am_Value other_char = other_handle.Get(CONSTRAINT_VAR);
          if (my_char.Valid() && my_char != other_char)
            cv->Release_Var((char)my_char);
          my_char = other_char;
          use_other_char = true;
        }
        //since other handle has a variable no need to allocate new variable
      }

      if (!char_needed) {
        Am_Value_List ref_list = for_obj.Get(REF_COUNT);
        if ((bool)ref_list.Get_Nth(get_count_index(wh)))
          char_needed = true;
      }

      if (char_needed) {
        if (!my_char.Valid())
          my_char = cv->Get_Next_Var(self);
        else {
          Am_Value other_char = (other_handle.Valid())
                                    ? other_handle.Get(CONSTRAINT_VAR)
                                    : Am_No_Value;
          if (other_char.Valid() && my_char.Valid() && !use_other_char &&
              other_char == my_char)
            my_char = cv->Get_Next_Var(self);
        }
      } else if (!char_needed && !use_other_char && my_char.Valid()) {
        cv->Release_Var((char)my_char);
        my_char = Am_No_Value;
      }
      return my_char;
    } else
      return Am_No_Value;
  } else
    return Am_No_Value;
}

Am_Define_Formula(int, get_state)
{
  Am_Object for_obj = self.Get_Owner().Get_Object(Am_ITEM);
  int state = NOT_SELECTED;

  if (for_obj.Valid()) {
    int ref_count =
        get_ref_count(for_obj, (Which_Constraint_Handle)self.Get(Am_ID));
    bool refed = (ref_count) ? true : false;
    bool dep = slot_has_dependency(self, for_obj);

    if (dep && refed)
      state = BOTH_STATE;
    else if (dep)
      state = PRIMARY;
    else if (refed)
      state = SECONDARY;
  }
  return state;
}

Am_Slot_Key
get_slot_key(Which_Constraint_Handle ch)
{
  Am_Slot_Key key;

  switch (ch.value) {
  case CH_LEFT_val:
    key = Am_LEFT;
    break;
  case CH_TOP_val:
    key = Am_TOP;
    break;
  case CH_WIDTH_val:
    key = Am_WIDTH;
    break;
  case CH_HEIGHT_val:
    key = Am_HEIGHT;
    break;
  case CH_X1_val:
    key = Am_X1;
    break;
  case CH_X2_val:
    key = Am_X2;
    break;
  case CH_Y1_val:
    key = Am_Y1;
    break;
  case CH_Y2_val:
    key = Am_Y2;
    break;
  default:
    key = Am_No_Slot_Key;
    break;
  }

  return key;
}

Am_Slot_Key
get_handle_slot(Which_Constraint_Handle ch)
{
  Am_Slot_Key key;

  switch (ch.value) {
  case CH_LEFT_val:
    key = LEFT_HANDLE;
    break;
  case CH_TOP_val:
    key = TOP_HANDLE;
    break;
  case CH_WIDTH_val:
    key = WIDTH_HANDLE;
    break;
  case CH_HEIGHT_val:
    key = HEIGHT_HANDLE;
    break;
  case CH_X1_val:
    key = X1_HANDLE;
    break;
  case CH_X2_val:
    key = X2_HANDLE;
    break;
  case CH_Y1_val:
    key = Y1_HANDLE;
    break;
  case CH_Y2_val:
    key = Y2_HANDLE;
    break;
  case CH_CENTER_X_val:
    key = CX_HANDLE;
    break;
  case CH_CENTER_Y_val:
    key = CY_HANDLE;
    break;
  case CH_BOTTOM_val:
    key = BOTTOM_HANDLE;
    break;
  case CH_RIGHT_val:
    key = RIGHT_HANDLE;
    break;
  default:
    key = Am_No_Slot_Key;
    break;
  }

  return key;
}

Am_Object
find_other_handle(Am_Value_List depends, Am_Object handle_group)
{
  Am_Object other_handle = Am_No_Object;

  if (((Am_Value)depends).Valid()) { //finding the right handle
    depends.Start();
    Am_Object obj = (Am_Object)depends.Get();
    Am_Value_List items =
        (Am_Value_List)handle_group.Get_Owner().Get(Am_GRAPHICAL_PARTS);
    Am_Object temp;

    for (items.Start(); !items.Last(); items.Next()) {
      temp = (Am_Object)items.Get();
      if (temp.Get_Object(Am_ITEM) == obj) {
        depends.Next();
        Which_Constraint_Handle wh = (Which_Constraint_Handle)depends.Get();
        Am_Slot_Key slot = get_handle_slot(wh);
        other_handle = temp.Get_Object(slot);
        break;
      }
    }
  }
  return other_handle;
}

Am_Define_String_Formula(compute_handle_text)
{
  Am_Object owner = self.Get_Owner();
  Am_Object handle_group = owner.Get_Owner();
  Am_Object for_obj = handle_group.Get_Object(Am_ITEM);
  char buffer[40];
  buffer[0] = '\0';
  bool visible = (bool)owner.Get(Am_VISIBLE);

  if (for_obj.Valid() && visible) {
    Am_Value my_char = (Am_Value)owner.Get(CONSTRAINT_VAR);
    Am_Value_List depends = (Am_Value_List)owner.Get(HANDLE_DEPENDS);
    Am_Object other_handle = Am_No_Object;
    Which_Constraint_Handle my_wh = (Which_Constraint_Handle)owner.Get(Am_ID);
    float percent = 1.0;
    if (my_wh == CH_CENTER_X || my_wh == CH_CENTER_Y)
      percent = 0.5;
    Am_Slot_Key key = get_slot_key(my_wh);
    Am_Value my_value;
    if (key != Am_No_Slot_Key)
      my_value = for_obj.Peek(key);

    other_handle = find_other_handle(depends, handle_group);

    if (other_handle.Valid()) {
      Am_Value_List info = owner.Get(CONSTRAINT_INFO);
      Am_Value other_char = other_handle.Get(CONSTRAINT_VAR);

      if (Am_Value(info).Valid() && other_char.Valid() &&
          my_char.Valid()) { //if this is valid, it should have constraint_var
        info.Start();
        int offset = (int)info.Get();
        info.Next();
        float mult = (float)info.Get();

        if (offset != 0 && mult != percent)
          sprintf(buffer, "%c=%c*%.2f+%d", (char)my_char, (char)other_char,
                  mult, offset);
        else if (offset != 0)
          sprintf(buffer, "%c=%c+%d", (char)my_char, (char)other_char, offset);
        else
          sprintf(buffer, "%c=%c*%.2f", (char)my_char, (char)other_char, mult);
      } else if (my_char.Valid())
        sprintf(buffer, "%c", (char)my_char);
    } else {
      if (my_char.Valid()) {
        if (my_value.Valid())
          sprintf(buffer, "%c=%d", (char)my_char, (int)my_value);
        else
          sprintf(buffer, "%c", (char)my_char);
      } else if (my_value.Valid())
        sprintf(buffer, "%d", (int)my_value);
    }
  }
  if (buffer[0] == '\0')
    return NULL;
  else
    return (Am_String)buffer;
}

Am_Define_Style_Formula(get_state_color)
{
  int state = self.Get(STATE);
  if (state == NOT_SELECTED)
    return Am_Black;
  else if (state == PRIMARY)
    return Am_Red;
  else if (state == SECONDARY)
    return Am_Blue;
  else
    return Am_Style::Am_Style(1.0, 0, 1.0);
}

Am_Define_Formula(bool, rect_visible)
{
  Am_Value v = self.Get_Sibling(Am_ITEM).Peek(Am_AS_LINE);

  if (v.Valid())
    return false;
  else
    return true;
}

Am_Define_Formula(bool, line_visible)
{
  Am_Value v = self.Get_Sibling(Am_ITEM).Peek(Am_AS_LINE);

  if (v.Valid())
    return true;
  else
    return false;
}

Am_Define_Formula(int, compute_left)
{
  int width = self.Get(Am_WIDTH);
  int center = self.Get(CENTER_X);

  return center - width / 2;
}

Am_Define_Formula(int, compute_handle_width)
{
  Am_Object text = self.Get_Sibling(TEXT_HANDLE);

  if ((bool)text.Get(Am_VISIBLE)) {
    self.Set(Am_HEIGHT, (int)text.Get(Am_HEIGHT) + 2);
    return (int)text.Get(Am_WIDTH) + 2;
  } else {
    self.Set(Am_HEIGHT, BLACK_HANDLE_SIZE);
    return BLACK_HANDLE_SIZE;
  }
}

Am_Define_Formula(bool, is_text_valid)
{
  if ((const char *)(Am_String)self.Get(Am_TEXT))
    return true;
  else
    return false;
}

Am_Define_Formula(int, compute_center_x_rect)
{
  Which_Constraint_Handle wh = self.Get(Am_ID);
  int center_x = 0;
  Am_Object for_obj = self.Get_Sibling(Am_ITEM);

  if (for_obj.Valid()) {
    Am_Value v = for_obj.Peek(Am_AS_LINE);
    if (!v.Valid()) {
      int width = for_obj.Get(Am_WIDTH);
      Am_Object handle = self.Get_Sibling(LEFT_HANDLE);

      if (handle.Valid()) {
        int left_off = (int)(handle.Get(Am_WIDTH)) / 2;

        switch (wh.value) {
        case CH_LEFT_val:
          center_x = (int)self.Get(Am_WIDTH) / 2;
          break;
        case CH_CENTER_Y_val:
          center_x = left_off + width / 2 - C_HANDLE_SIZE;
          break;
        case CH_WIDTH_val:
        case CH_TOP_val:
        case CH_CENTER_X_val:
        case CH_BOTTOM_val:
          center_x = left_off + width / 2;
          break;
        case CH_RIGHT_val:
          center_x = left_off + width;
          break;
        case CH_HEIGHT_val:
          center_x = left_off + width + HANDLE_SPACING;
          break;
        default:
          break;
        }
      }
    }
  }
  return center_x;
}

Am_Define_Formula(int, compute_top_rect)
{
  Which_Constraint_Handle wh = self.Get(Am_ID);
  int top = 0;
  Am_Object for_obj = self.Get_Sibling(Am_ITEM);
  if (for_obj.Valid()) {
    Am_Value v = for_obj.Peek(Am_AS_LINE);
    if (!v.Valid()) {
      int height = for_obj.Get(Am_HEIGHT);
      int y_off = HANDLE_SPACING - C_HANDLE_SIZE_D2;

      switch (wh.value) {
      case CH_RIGHT_val:
      case CH_HEIGHT_val:
      case CH_LEFT_val:
        top = y_off + height / 2;
        break;
      case CH_WIDTH_val:
        break;
      case CH_TOP_val:
        top = y_off;
        break;
      case CH_BOTTOM_val:
        top = y_off + height;
        break;
      case CH_CENTER_X_val:
        top = y_off + height / 2 - C_HANDLE_SIZE;
        break;
      case CH_CENTER_Y_val:
        top = y_off + height / 2;
        break;
      default:
        break;
      }
    }
  }
  return top;
}

Am_Define_Formula(int, compute_center_x_line)
{
  Am_Object for_obj = self.Get_Sibling(Am_ITEM);
  int center_x = 0;

  if (for_obj.Valid()) {
    Am_Value v = for_obj.Peek(Am_AS_LINE);
    if (v.Valid()) {
      int x1 = for_obj.Get(Am_X1);
      int x2 = for_obj.Get(Am_X2);
      bool rsl = (x1 > x2) ? true : false;
      int left_off;
      int width;

      if (rsl) {
        left_off = (int)self.Get_Sibling(X2_HANDLE).Get(Am_WIDTH) / 2;
        width = x1 - x2;
      } else {
        left_off = (int)self.Get_Sibling(X1_HANDLE).Get(Am_WIDTH) / 2;
        width = x2 - x1;
      }

      Which_Constraint_Handle wh = self.Get(Am_ID);

      switch (wh.value) {
      case CH_Y1_val:
        center_x =
            (rsl)
                ? left_off + width - C_HANDLE_SIZE - (int)self.Get(Am_WIDTH) / 2
                : left_off + C_HANDLE_SIZE + (int)self.Get(Am_WIDTH) / 2;
        break;
      case CH_Y2_val:
        center_x =
            (!rsl)
                ? left_off + width - C_HANDLE_SIZE - (int)self.Get(Am_WIDTH) / 2
                : left_off + C_HANDLE_SIZE + (int)self.Get(Am_WIDTH) / 2;
        break;
      case CH_X1_val:
        center_x = (rsl) ? left_off + width : left_off;
        break;
      case CH_X2_val:
        center_x = (!rsl) ? left_off + width : left_off;
        break;
      default:
        break;
      }
    }
  }
  return center_x;
}

Am_Define_Formula(int, compute_top_line)
{
  Am_Object for_obj = self.Get_Sibling(Am_ITEM);
  int top = 0;

  if (for_obj.Valid()) {
    Am_Value v = for_obj.Peek(Am_AS_LINE);
    if (v.Valid()) {
      int y1 = for_obj.Get(Am_Y1);
      int y2 = for_obj.Get(Am_Y2);

      bool upside_down = (y1 > y2) ? true : false;
      int height = (upside_down) ? y1 - y2 : y2 - y1;

      Which_Constraint_Handle wh = self.Get(Am_ID);

      switch (wh.value) {
      case CH_X1_val:
        top = (upside_down) ? height - C_HANDLE_SIZE
                            : C_HANDLE_SIZE_D2 + C_HANDLE_SIZE;
        break;
      case CH_X2_val:
        top = (!upside_down) ? height - C_HANDLE_SIZE
                             : C_HANDLE_SIZE + C_HANDLE_SIZE_D2;
        break;
      case CH_Y1_val:
        top = (!upside_down) ? 0 : height;
        break;
      case CH_Y2_val:
        top = (upside_down) ? 0 : height;
        break;
      default:
        break;
      }
    }
  }
  return top;
}

Am_Define_Formula(int, compute_vhline_height)
{
  int id = self.Get(Am_ID);
  int height = 0;
  Am_Object for_obj = self.Get_Sibling(Am_ITEM);
  Am_Value v = for_obj.Peek(Am_AS_LINE);

  switch (id) {
  case 1:
  case 0:
    height = (v.Valid()) ? C_HANDLE_SIZE : HANDLE_SPACING;
    break;
  case 3:
  case 2:
    height = 1;
    break;
  }

  return height;
}

Am_Define_Formula(int, compute_vhline_width)
{
  int id = self.Get(Am_ID);
  int height = 0;
  Am_Object for_obj = self.Get_Sibling(Am_ITEM);
  Am_Value v = for_obj.Peek(Am_AS_LINE);

  switch (id) {
  case 2:
  case 3:
    height = (v.Valid()) ? C_HANDLE_SIZE : HANDLE_SPACING;
    break;
  case 0:
  case 1:
    height = 1;
    break;
  }

  return height;
}

Am_Define_Formula(int, compute_vhline_top)
{
  int id = self.Get(Am_ID);
  Am_Object for_obj = self.Get_Sibling(Am_ITEM);
  Am_Value v = for_obj.Peek(Am_AS_LINE);
  int top = 0;

  if (!v.Valid()) {
    int height = for_obj.Get(Am_HEIGHT);
    switch (id) {
    case 0:
    case 1:
      top = 0;
      break;
    case 2:
      top = HANDLE_SPACING - 1;
      break;
    case 3:
      top = height + HANDLE_SPACING - 1;
      break;
    }
  } else {
    int y1 = for_obj.Get(Am_Y1);
    int y2 = for_obj.Get(Am_Y2);
    int height = for_obj.Get(Am_HEIGHT);
    bool upside_down = (y1 > y2) ? true : false;
    switch (id) {
    case 0:
      top = (upside_down) ? height - C_HANDLE_SIZE_D2 : C_HANDLE_SIZE_D2;
      break;
    case 1:
      top = (!upside_down) ? height - C_HANDLE_SIZE_D2 : C_HANDLE_SIZE_D2;
      break;
    case 2:
      top = (upside_down) ? height + C_HANDLE_SIZE_D2 : C_HANDLE_SIZE_D2;
      break;
    case 3:
      top = (!upside_down) ? height + C_HANDLE_SIZE_D2 : C_HANDLE_SIZE_D2;
      break;
    }
  }

  return top;
}

Am_Define_Formula(int, compute_vhline_left)
{
  int id = self.Get(Am_ID);
  Am_Object for_obj = self.Get_Sibling(Am_ITEM);
  Am_Value v = for_obj.Peek(Am_AS_LINE);
  int left = 0;

  if (!v.Valid()) {
    int width = for_obj.Get(Am_WIDTH);
    int left_off = (int)self.Get_Sibling(LEFT_HANDLE).Get(Am_WIDTH) / 2;
    switch (id) {
    case 0:
      left = left_off;
      break;
    case 1:
    case 2:
    case 3:
      left = left_off + width;
      break;
    }
  } else {
    int x1 = for_obj.Get(Am_X1);
    int x2 = for_obj.Get(Am_X2);
    int width = for_obj.Get(Am_WIDTH);
    bool rsl = (x1 > x2) ? true : false;
    int left_off = (rsl) ? (int)self.Get_Sibling(X2_HANDLE).Get(Am_WIDTH) / 2
                         : (int)self.Get_Sibling(X1_HANDLE).Get(Am_WIDTH) / 2;
    switch (id) {
    case 0:
      left = (rsl) ? left_off + width : left_off;
      break;
    case 1:
      left = (!rsl) ? left_off + width : left_off;
      break;
    case 2:
      left = (rsl) ? left_off + width - C_HANDLE_SIZE : left_off;
      break;
    case 3:
      left = (!rsl) ? left_off + width - C_HANDLE_SIZE : left_off;
      break;
    }
  }

  return left;
}

Am_Object
draw_arrow_lines(bool vertical)
{
  Am_Object al = Am_Group.Create()
                     .Set(Am_WIDTH, (vertical) ? 5 : 10)
                     .Set(Am_HEIGHT, (vertical) ? 10 : 5);
  Am_Object main_line = Am_Line.Create()
                            .Set(Am_TOP, (vertical) ? 0 : 3)
                            .Set(Am_LEFT, (vertical) ? 3 : 0);
  if (vertical) {
    main_line.Set(Am_WIDTH, 1).Set(Am_HEIGHT, Am_From_Owner(Am_HEIGHT));
  } else {
    main_line.Set(Am_WIDTH, Am_From_Owner(Am_WIDTH)).Set(Am_HEIGHT, 1);
  }

  al.Set(Am_VISIBLE, rect_visible)
      .Add_Part(MAIN_LINE, main_line)
      .Add_Part(Am_Line.Create()
                    .Set(Am_X1, Am_From_Sibling(MAIN_LINE, Am_X1))
                    .Set(Am_Y1, Am_From_Sibling(MAIN_LINE, Am_Y1))
                    .Set(Am_X2, (vertical) ? Am_Same_As(Am_X1, -2)
                                           : Am_Same_As(Am_X1, 4))
                    .Set(Am_Y2, (vertical) ? Am_Same_As(Am_Y1, 4)
                                           : Am_Same_As(Am_Y1, 2)))
      .Add_Part(Am_Line.Create()
                    .Set(Am_X1, Am_From_Sibling(MAIN_LINE, Am_X1))
                    .Set(Am_Y1, Am_From_Sibling(MAIN_LINE, Am_Y1))
                    .Set(Am_X2, (vertical) ? Am_Same_As(Am_X1, 2)
                                           : Am_Same_As(Am_X1, 4))
                    .Set(Am_Y2, (vertical) ? Am_Same_As(Am_Y1, 4)
                                           : Am_Same_As(Am_Y1, -2)))
      .Add_Part(Am_Line.Create()
                    .Set(Am_X1, Am_From_Sibling(MAIN_LINE, Am_X2))
                    .Set(Am_Y1, Am_From_Sibling(MAIN_LINE, Am_Y2))
                    .Set(Am_X2, (vertical) ? Am_Same_As(Am_X1, -2)
                                           : Am_Same_As(Am_X1, -4))
                    .Set(Am_Y2, (vertical) ? Am_Same_As(Am_Y1, -4)
                                           : Am_Same_As(Am_Y1, 2)))
      .Add_Part(Am_Line.Create()
                    .Set(Am_X1, Am_From_Sibling(MAIN_LINE, Am_X2))
                    .Set(Am_Y1, Am_From_Sibling(MAIN_LINE, Am_Y2))
                    .Set(Am_X2, (vertical) ? Am_Same_As(Am_X1, 2)
                                           : Am_Same_As(Am_X1, -4))
                    .Set(Am_Y2, (vertical) ? Am_Same_As(Am_Y1, -4)
                                           : Am_Same_As(Am_Y1, -2)));
  return al;
}

//fix to have redraw method rather than to have formula in slots.

Am_Define_Formula(int, compute_handle_lt)
{
  Am_Object for_obj = self.Get(Am_ITEM);

  int left = 0, top = 0;
  Am_Object handle;

  if (for_obj.Valid()) {
    if (for_obj.Is_Instance_Of(line_proto)) {
      int x1 = for_obj.Get(Am_X1);
      int x2 = for_obj.Get(Am_X2);

      left = (x1 > x2)
                 ? (int)for_obj.Get(Am_LEFT) -
                       (int)self.Get_Object(X2_HANDLE).Get(Am_WIDTH) / 2
                 : (int)for_obj.Get(Am_LEFT) -
                       (int)self.Get_Object(X1_HANDLE).Get(Am_WIDTH) / 2;
      top = (int)for_obj.Get(Am_TOP) - C_HANDLE_SIZE_D2;
    } else {
      top = (int)for_obj.Get(Am_TOP) - HANDLE_SPACING + 2;
      left = (int)for_obj.Get(Am_LEFT) -
             (int)self.Get_Object(LEFT_HANDLE).Get(Am_WIDTH) / 2;
    }
  }
  self.Set(Am_TOP, top);
  return left;
}

///////////////////
void
release_variable(Am_Value_List list, Am_Object for_obj,
                 Gilt_Constraint_Variable_Data *cv)
{
  Am_Object tmp;
  Am_Value_List const_info;
  Am_Value_List tmp_list;
  Which_Constraint_Handle wh;
  Am_Slot_Key key;
  Am_Value my_char;

  for (list.Start(); !list.Last(); list.Next()) {
    tmp = (Am_Object)list.Get();
    if (tmp.Get_Object(Am_ITEM) == for_obj) {
      const_info = for_obj.Get(CONSTRAINT_INFO);

      for (const_info.Start(); !const_info.Last(); const_info.Next()) {
        tmp_list = const_info.Get();
        wh = (Which_Constraint_Handle)tmp_list.Get_Nth(0);
        key = get_handle_slot(wh);
        tmp = tmp.Get_Object(key);
        my_char = tmp.Get(CONSTRAINT_VAR);
        if (my_char.Valid())
          cv->Release_Var((char)my_char);
      }
      break;
    }
  }
}

//minor detail is different from modified_toggle_object_in_list
//this releases variables any constraint_handle has when object is
//deleted from the list
void
constraint_toggle_object_in_list(const Am_Object &widget, Am_Object new_object,
                                 Am_Value_List &list, Am_Value_List &g_list,
                                 Gilt_Constraint_Variable_Data *cv)
{
  list.Start();
  if (list.Member(new_object)) {
    Am_INTER_TRACE_PRINT(widget, "Selection handle removing "
                                     << new_object << " from " << widget);
    release_variable(g_list, new_object, cv);
    list.Delete();
  } else { // not a member, add it
    Am_INTER_TRACE_PRINT(widget, "Selection handle adding "
                                     << new_object << " to " << widget);
    // Not allowed to have objects selected that are parts of
    // new_object, owners of new_object, or have owners different from
    // new_object.  This can happen when you have select into groups.
    for (list.Start(); !list.Last(); list.Next()) {
      Am_Object item = list.Get();
      if (item.Is_Part_Of(new_object) || new_object.Is_Part_Of(item) ||
          (!item.Is_Instance_Of(main_group_rect_proto) &&
           item.Get_Owner() != new_object.Get_Owner()))
        release_variable(g_list, new_object, cv);
      list.Delete();
    }
    list.Add(new_object);
  }
}

void
release_all_variables(Am_Value_List g_list, Am_Value_List list,
                      Gilt_Constraint_Variable_Data *cv)
{
  Am_Object tmp;

  for (list.Start(); !list.Last(); list.Next()) {
    tmp = (Am_Object)list.Get();
    release_variable(g_list, tmp, cv);
  }
}

//different from modified_sel_obj by releasing all the variables any
//constraint handle has
Am_Define_Method(Am_Mouse_Event_Method, void, constraint_sel_object,
                 (Am_Object inter, int mouse_x, int mouse_y, Am_Object ref_obj,
                  Am_Input_Char ic))
{
  Am_Object new_object;
  bool window_border = false;
  if (inter.Valid()) {
    Am_Object widget = inter.Get_Owner();
    modified_clear_multi_selections(widget);
    new_object = inter.Get(Am_START_OBJECT);
    Am_Value_List list = widget.Get(Am_VALUE);
    Am_Value_List g_list =
        widget.Get_Object(Am_FEEDBACK_OBJECT).Get(Am_GRAPHICAL_PARTS);
    Gilt_Constraint_Variable_Data *cv =
        (Gilt_Constraint_Variable_Data *)(Am_Ptr)widget.Get(CONSTRAINT_VAR);
    Am_Object tmp;

    if (new_object.Valid()) { //valid object
      Am_Value new_value;
      bool toggle_in_set = ic.shift;
      Am_Object main_rect = widget.Get_Sibling(MAIN_GROUP_RECT);

      if (new_object == widget) { // then clicked in the background
        int width = (int)main_rect.Get(Am_WIDTH);
        int height = (int)main_rect.Get(Am_HEIGHT);
        int top = (int)main_rect.Get_Owner().Get(Am_TOP);

        if (mouse_x < 20 || (mouse_x > (width - 20)) ||
            (mouse_y < (top + 20)) || (mouse_y > (top + height - 20)))
          window_border = true;
        if (toggle_in_set) { //  don't do anything
          if (window_border) {
            new_object = main_rect;
            list.Start();
            if (list.Member(main_rect)) {
              release_variable(g_list, main_rect, cv);
              list.Delete();
            } else
              list.Add(new_object);
            new_value = list;
          } else {
            Am_Abort_Interactor(inter);
            return;
          }
        } else { // select nothing
          Am_INTER_TRACE_PRINT(widget, "Selection handle setting empty for "
                                           << widget);
          if (window_border) {
            release_all_variables(g_list, list, cv);
            list.Make_Empty();
            list.Add(main_rect);
            new_object = main_rect;
          } else {
            release_all_variables(g_list, list, cv);
            list.Make_Empty();
            new_object = NULL;
          }
          new_value = list;
        }
      } else { // over a specific object
        Am_Value value;
        Am_Object diff_obj;
        Am_Where_Method method;
        bool abort_ok = true;
        value = widget.Peek(Am_SELECT_CLOSEST_POINT_STYLE);
        if (value.Valid()) {
          method = widget.Get(Am_SELECT_CLOSEST_POINT_METHOD);
          diff_obj = method.Call(widget, new_object, ref_obj, mouse_x, mouse_y);
          if (diff_obj.Valid())
            abort_ok = false;
        }
        list.Start();
        if (toggle_in_set)
          constraint_toggle_object_in_list(widget, new_object, list, g_list,
                                           cv);
        else { //if object is selected, do nothing, otherwise,
          // make new_object be the only selection
          if (list.Member(new_object)) {
            if (abort_ok) {
              Am_Abort_Interactor(inter); //make sure not queued for undo
              return;
            }
          } else {
            Am_INTER_TRACE_PRINT(widget, "Selection handle setting "
                                             << widget << " to contain only "
                                             << new_object);
            release_all_variables(g_list, list, cv);
            list.Make_Empty();
            list.Add(new_object);
          }
        }
        new_value = list;
      }
      Am_Value old_value;
      old_value = widget.Peek(Am_VALUE);
      widget.Set(Am_VALUE, new_value);
      Am_Object command = inter.Get_Object(Am_COMMAND);
      modified_set_commands_for_sel(command, inter, widget, new_object,
                                    new_value, old_value);
    }
  }
}

///// For setting up constraints ////////
Am_Define_Method(Am_Where_Method, Am_Object, compute_constraint_start_where,
                 (Am_Object /*inter*/, Am_Object object, Am_Object event_window,
                  int x, int y))
{
  if (!Am_Point_In_All_Owners(object, x, y, event_window))
    return Am_No_Object;

  Am_Object constraint_handle_group = Am_Point_In_Part(
      object.Get_Object(Am_FEEDBACK_OBJECT), x, y, event_window);
  if (constraint_handle_group.Valid()) {
    Am_Object handle =
        Am_Point_In_Part(constraint_handle_group, x, y, event_window);
    if (handle.Is_Instance_Of(handle_proto)) {
      object.Set(PRIMARY_OBJ, constraint_handle_group.Get_Object(Am_ITEM));
      object.Set(PRIMARY_HANDLE, handle);
      return handle;
    } else
      return Am_No_Object;
  }

  return Am_No_Object;
}

Am_Define_Method(Am_Mouse_Event_Method, void, set_primary_object,
                 (Am_Object inter, int mouse_x, int mouse_y, Am_Object ref_obj,
                  Am_Input_Char /*ic*/))
{

  Am_Object widget = inter.Get_Owner();

  Am_Object other_inter = widget.Get_Object(Am_INTERACTOR);
  Am_Abort_Interactor(other_inter);
  other_inter = widget.Get_Object(Am_SELECT_OUTSIDE_INTERACTOR);
  Am_Abort_Interactor(other_inter);
  other_inter = widget.Get_Object(Am_BACKGROUND_INTERACTOR);
  Am_Abort_Interactor(other_inter);

  //need to clear secondary slots to preserve color of handles
  //previously selected

  widget.Set(SECONDARY_HANDLE, NULL);
  widget.Set(SECONDARY_OBJ, NULL);

  Am_Object handle = widget.Get(PRIMARY_HANDLE);
  if (handle.Valid())
    handle.Set(STATE, PRIMARY);
  Am_Object feedback_obj = inter.Get(Am_FEEDBACK_OBJECT);

  Am_Translate_Coordinates(ref_obj, mouse_x, mouse_y,
                           widget.Get_Object(Am_OPERATES_ON), mouse_x, mouse_y);
  feedback_obj.Set(Am_VISIBLE, true).Set(Am_X1, mouse_x).Set(Am_Y1, mouse_y);
}

Am_Define_Method(Am_Mouse_Event_Method, void, look_for_secondary_object,
                 (Am_Object inter, int mouse_x, int mouse_y, Am_Object ref_obj,
                  Am_Input_Char /*ic*/))
{
  Am_Object feedback_obj = inter.Get(Am_FEEDBACK_OBJECT);
  Am_Object widget = inter.Get_Owner();

  Am_Object handle_group = Am_Point_In_Part(
      widget.Get_Object(Am_FEEDBACK_OBJECT), mouse_x, mouse_y, ref_obj);
  if (handle_group.Valid()) {
    Am_Object handle =
        Am_Point_In_Part(handle_group, mouse_x, mouse_y, ref_obj);
    if (handle.Is_Instance_Of(handle_proto) &&
        handle != widget.Get_Object(PRIMARY_HANDLE)) {
      Am_Object prev = widget.Get_Object(SECONDARY_HANDLE);
      if (prev.Valid())
        prev.Set(STATE, get_state);
      widget.Set(SECONDARY_HANDLE, handle);
      handle.Set(STATE, SECONDARY);
    } else {
      Am_Object second_h = widget.Get_Object(SECONDARY_HANDLE);
      if (second_h.Valid()) {
        second_h.Set(STATE, NOT_SELECTED);
        widget.Set(SECONDARY_HANDLE, Am_No_Object);
      }
    }
  } else {
    Am_Object second_h = widget.Get_Object(SECONDARY_HANDLE);
    if (second_h.Valid()) {
      second_h.Set(STATE, NOT_SELECTED);
      widget.Set(SECONDARY_HANDLE, Am_No_Object);
    }
  }

  Am_Translate_Coordinates(ref_obj, mouse_x, mouse_y,
                           widget.Get_Object(Am_OPERATES_ON), mouse_x, mouse_y);
  feedback_obj.Set(Am_X2, mouse_x).Set(Am_Y2, mouse_y);
}

Am_Value_List
setup_regular_constraint(Am_Object &command, Am_Object po,
                         Which_Constraint_Handle &ph, Am_Object so,
                         Which_Constraint_Handle &sh, int offset,
                         float percentage)
{
  Am_Slot_Key p_obj_slot;
  Am_Slot_Key s_obj_slot;
  Am_Slot_Key s_obj_ref_slot;
  Am_Slot_Key self_slot;
  float self_percent = 0;
  //Check loop or conflict

  switch (sh.value) {
  case CH_RIGHT_val:
    s_obj_slot = Am_LEFT;
    s_obj_ref_slot = Am_WIDTH;
    break;
  case CH_BOTTOM_val:
    s_obj_slot = Am_TOP, s_obj_ref_slot = Am_HEIGHT;
    break;
  case CH_CENTER_X_val:
    s_obj_slot = Am_LEFT;
    s_obj_ref_slot = Am_WIDTH;
    break;
  case CH_CENTER_Y_val:
    s_obj_slot = Am_TOP, s_obj_ref_slot = Am_HEIGHT;
    break;
  default:
    s_obj_slot = Am_NO_SLOT, s_obj_ref_slot = get_slot_key(sh);
    break;
  }

  switch (ph.value) {
  case CH_CENTER_X_val:
    p_obj_slot = Am_LEFT;
    self_slot = Am_WIDTH;
    self_percent = -0.5;
    break;
  case CH_CENTER_Y_val:
    p_obj_slot = Am_TOP;
    self_slot = Am_HEIGHT;
    self_percent = -0.5;
    break;
  case CH_RIGHT_val:
    p_obj_slot = Am_LEFT;
    self_slot = Am_WIDTH;
    self_percent = -1.0;
    break;
  case CH_BOTTOM_val:
    p_obj_slot = Am_TOP;
    self_slot = Am_HEIGHT;
    self_percent = -1.0;
    break;
  default:
    p_obj_slot = get_slot_key(ph);
    self_slot = Am_NO_SLOT;
    ;
    break;
  }
  command.Set(Am_OLD_VALUE,
              Am_Value_List().Add(p_obj_slot).Add(po.Get(p_obj_slot)));
  Am_Formula formula =
      Get_Data_And_Adjust(so, s_obj_slot, s_obj_ref_slot, offset, percentage,
                          self_slot, self_percent);
  po.Set(p_obj_slot, formula);
  command.Set(Am_VALUE, formula);
  command.Set(Am_OBJECT_MODIFIED, po);

  return Am_Value_List()
      .Add(ph)
      .Add(so)
      .Add(sh)
      .Add(s_obj_slot)
      .Add(s_obj_ref_slot)
      .Add(offset)
      .Add(percentage)
      .Add(self_slot)
      .Add(self_percent);
}

int
get_count_index(Which_Constraint_Handle wh)
{
  int index = 0;

  switch (wh.value) {
  case CH_TOP_val:
    index = 0;
    break;
  case CH_LEFT_val:
    index = 1;
    break;
  case CH_BOTTOM_val:
    index = 2;
    break;
  case CH_RIGHT_val:
    index = 3;
    break;
  case CH_HEIGHT_val:
    index = 4;
    break;
  case CH_WIDTH_val:
    index = 5;
    break;
  case CH_CENTER_X_val:
    index = 6;
    break;
  case CH_CENTER_Y_val:
    index = 7;
    break;
  case CH_X1_val:
    index = 0;
    break;
  case CH_Y1_val:
    index = 1;
    break;
  case CH_X2_val:
    index = 2;
    break;
  case CH_Y2_val:
    index = 3;
    break;
  default:
    break;
  }

  return index;
}

void
decrease_ref_count(Am_Object obj, Which_Constraint_Handle wh)
{
  int index = get_count_index(wh);
  Am_Value_List ref_count = (Am_Value_List)obj.Get(REF_COUNT);
  int count = (int)ref_count.Get_Nth(index);
  ref_count.Move_Nth(index);
  ref_count.Set(count - 1);
  obj.Set(REF_COUNT, ref_count);
}

void
increase_ref_count(Am_Object obj, Which_Constraint_Handle wh)
{
  int index = get_count_index(wh);
  Am_Value_List ref_count = (Am_Value_List)obj.Get(REF_COUNT);
  int count = (int)ref_count.Get_Nth(index);
  ref_count.Move_Nth(index);
  ref_count.Set(count + 1);
  obj.Set(REF_COUNT, ref_count);
}

Am_Value_List
depends_list_be_decreased(Am_Object primary_handle, Am_Object handle_group)
{
  Which_Constraint_Handle wh =
      (Which_Constraint_Handle)primary_handle.Get(Am_ID);
  Am_Value_List depends_list = primary_handle.Get(HANDLE_DEPENDS);
  Am_Value_List tmp_list;
  Am_Object tmp_obj;

  if (wh == CH_CENTER_X || wh == CH_LEFT || wh == CH_RIGHT) {
    tmp_obj = handle_group.Get_Object(CX_HANDLE);
    tmp_list = tmp_obj.Get(HANDLE_DEPENDS);
    if (tmp_obj != primary_handle && ((Am_Value_List)tmp_list).Valid())
      depends_list = tmp_list;
    tmp_obj = handle_group.Get_Object(LEFT_HANDLE);
    tmp_list = tmp_obj.Get(HANDLE_DEPENDS);
    if (tmp_obj != primary_handle && ((Am_Value_List)tmp_list).Valid())
      depends_list = tmp_list;
    tmp_obj = handle_group.Get_Object(RIGHT_HANDLE);
    tmp_list = tmp_obj.Get(HANDLE_DEPENDS);
    if (tmp_obj != primary_handle && ((Am_Value_List)tmp_list).Valid())
      depends_list = tmp_list;
  } else if (wh == CH_CENTER_Y || wh == CH_TOP || wh == CH_BOTTOM) {
    tmp_obj = handle_group.Get_Object(CY_HANDLE);
    tmp_list = tmp_obj.Get(HANDLE_DEPENDS);
    if (tmp_obj != primary_handle && ((Am_Value_List)tmp_list).Valid())
      depends_list = tmp_list;
    tmp_obj = handle_group.Get_Object(TOP_HANDLE);
    tmp_list = tmp_obj.Get(HANDLE_DEPENDS);
    if (tmp_obj != primary_handle && ((Am_Value_List)tmp_list).Valid())
      depends_list = tmp_list;
    tmp_obj = handle_group.Get_Object(BOTTOM_HANDLE);
    tmp_list = tmp_obj.Get(HANDLE_DEPENDS);
    if (tmp_obj != primary_handle && ((Am_Value_List)tmp_list).Valid())
      depends_list = tmp_list;
  }
  return depends_list;
}

void
remove_redundancy(Am_Value_List &one_const_info, Am_Value_List &const_info)
{
  Which_Constraint_Handle wh =
      (Which_Constraint_Handle)one_const_info.Get_Nth(0);
  Am_Value_List tmp_list;
  Which_Constraint_Handle tmp_wh;
  bool replaced = false;

  for (const_info.Start(); !const_info.Last() && !replaced; const_info.Next()) {
    tmp_list = const_info.Get();
    tmp_wh = (Which_Constraint_Handle)tmp_list.Get_Nth(0);

    if (tmp_wh == wh ||
        ((tmp_wh == CH_RIGHT || tmp_wh == CH_LEFT || tmp_wh == CH_CENTER_X) &&
         (wh == CH_RIGHT || wh == CH_LEFT || wh == CH_CENTER_X)) ||
        ((tmp_wh == CH_TOP || tmp_wh == CH_CENTER_Y || tmp_wh == CH_BOTTOM) &&
         (wh == CH_TOP || wh == CH_CENTER_Y || wh == CH_BOTTOM))) {
      const_info.Set(one_const_info);
      replaced = true;
    }
  }

  if (!replaced)
    const_info.Add(one_const_info);
}

Am_Define_Method(Am_Mouse_Event_Method, void, setup_constraint_and_clear_slots,
                 (Am_Object inter, int mouse_x, int mouse_y, Am_Object ref_obj,
                  Am_Input_Char /*ic*/))
{
  Am_Object feedback_obj = inter.Get(Am_FEEDBACK_OBJECT);
  feedback_obj.Set(Am_VISIBLE, false);

  Am_Object widget = inter.Get_Owner();

  Am_Object handle_group = Am_Point_In_Part(
      widget.Get_Object(Am_FEEDBACK_OBJECT), mouse_x, mouse_y, ref_obj);
  Am_Object prev = widget.Get_Object(SECONDARY_HANDLE);
  bool constraint_set = false;
  Am_Object secondary_handle;

  if (prev.Valid())
    prev.Set(STATE, get_state);

  if (handle_group.Valid()) {
    Am_Object handle =
        Am_Point_In_Part(handle_group, mouse_x, mouse_y, ref_obj);
    Am_Object primary_handle = widget.Get_Object(PRIMARY_HANDLE);

    if (handle.Valid() && handle.Is_Instance_Of(handle_proto) &&
        handle != primary_handle) {
      Am_Object primary_obj = widget.Get_Object(PRIMARY_OBJ);
      Am_Object secondary_obj = handle_group.Get_Object(Am_ITEM);
      secondary_handle = handle;
      Which_Constraint_Handle ph =
          (Which_Constraint_Handle)primary_handle.Get(Am_ID);
      Which_Constraint_Handle sh =
          (Which_Constraint_Handle)secondary_handle.Get(Am_ID);

      Am_Value return_value;
      Am_Object calling_window = widget.Get_Object(Am_WINDOW);
      int percentage = 100;
      secondary_handle.Set(STATE, PRIMARY);

      widget.Set(SECONDARY_OBJ, secondary_obj);
      widget.Set(SECONDARY_HANDLE, secondary_handle);

      if (sh == CH_CENTER_X || sh == CH_CENTER_Y)
        percentage = 50;

      Offset_Percentage_Dialog.Get_Object(OFFSET_INPUT)
          .Set(Am_VALUE, 0)
          .Get_Owner()
          .Get_Object(PERCENT_INPUT)
          .Set(Am_VALUE, percentage)
          .Get_Owner()
          .Set(Am_TOP, (int)calling_window.Get(Am_TOP) + 20)
          .Set(Am_LEFT, (int)calling_window.Get(Am_LEFT) + 20);

      Am_Pop_Up_Window_And_Wait(Offset_Percentage_Dialog, return_value);

      if (return_value.Valid()) {
        Am_Value_List constraint_info = (Am_Value_List)return_value;
        constraint_info.Start();
        int offset = (int)constraint_info.Get();
        constraint_info.Next();
        float mult = (float)constraint_info.Get() / 100;
        Constraint_How_Set whattodo =
            check_dependency(primary_obj, ph, calling_window);
        if (whattodo == CONSTRAINT_SETUP) {
          Am_Value_List depends =
              depends_list_be_decreased(primary_handle, handle_group);
          bool increase = true;

          if (((Am_Value)depends).Valid()) {
            Am_Object tmp_obj = (Am_Object)depends.Get_Nth(0);
            Which_Constraint_Handle tmp_wh =
                (Which_Constraint_Handle)depends.Get_Nth(1);
            if (tmp_obj != secondary_obj || tmp_wh != sh)
              decrease_ref_count(tmp_obj, tmp_wh);
            else
              increase = false;
          }
          constraint_set = true;
          Am_Object cmd = inter.Get_Object(Am_COMMAND);
          constraint_info = setup_regular_constraint(
              cmd, primary_obj, ph, secondary_obj, sh, offset, mult);

          if (increase)
            increase_ref_count(secondary_obj, sh);
          Am_Value_List l = (Am_Value_List)primary_obj.Get(CONSTRAINT_INFO);
          remove_redundancy(constraint_info, l);
          primary_obj.Set(CONSTRAINT_INFO, l);
        } else if (whattodo == MULTIPLE_SETUP)
          Am_Show_Alert_Dialog(
              "Don't know what to do yet, aborting constraint setup");
      }
    }
  }
  secondary_handle = widget.Get_Object(SECONDARY_HANDLE);
  if (secondary_handle.Valid())
    secondary_handle.Set(STATE, get_state);
  widget.Get_Object(PRIMARY_HANDLE).Set(STATE, get_state);

  if (!constraint_set) {
    widget.Set(PRIMARY_HANDLE, NULL)
        .Set(PRIMARY_OBJ, NULL)
        .Set(SECONDARY_OBJ, NULL)
        .Set(SECONDARY_HANDLE, NULL);
    Am_Abort_Interactor(inter);
  }
}

Am_Define_Method(Am_Object_Method, void, restore_old_value, (Am_Object cmd))
{
  Am_Value_List old_value = cmd.Get(Am_OLD_VALUE);
  Am_Value new_value = cmd.Get(Am_VALUE);
  Am_Object mo = cmd.Get_Object(Am_OBJECT_MODIFIED);
  old_value.Start();
  Am_Slot_Key key = (Am_Slot_Key)(int)old_value.Get();
  old_value.Next();

  mo.Set(key, old_value.Get());
  cmd.Set(Am_VALUE, old_value.Get()); //formula is not restored
  cmd.Set(Am_OLD_VALUE, Am_Value_List().Add(key).Add(new_value));
  cmd.Set(Am_OBJECT_MODIFIED, mo);
}

////////// Dialog window stuff ///////

Am_Define_Method(Am_Object_Method, void, create_custom_constraint,
                 (Am_Object /*cmd*/))
{
  Am_Show_Alert_Dialog("Custom constrainr not implemented yet");
}

Am_Define_Method(Am_Object_Method, void, return_constraint_info,
                 (Am_Object cmd))
{
  if ((int)cmd.Get(Am_ID) == 1) {
    int offset =
        Offset_Percentage_Dialog.Get_Object(OFFSET_INPUT).Get(Am_VALUE);
    int percent =
        Offset_Percentage_Dialog.Get_Object(PERCENT_INPUT).Get(Am_VALUE);
    Am_Value_List vlist = Am_Value_List().Add(offset).Add(percent);
    Am_Finish_Pop_Up_Waiting(Offset_Percentage_Dialog, (Am_Value)vlist);

  } else
    Am_Finish_Pop_Up_Waiting(Offset_Percentage_Dialog, Am_No_Value);
}

void
Constraint_Widget_Initialize()
{
  Am_Object one_handle;

  Conflict_Window_Initialize();

  handle_proto =
      Am_Group.Create()
          .Add(CENTER_X, 0)
          .Set(Am_LEFT, compute_left)
          .Set(Am_WIDTH, Am_Width_Of_Parts)
          .Set(Am_HEIGHT, Am_Height_Of_Parts)
          .Add(CONSTRAINT_INFO, Am_No_Value)
          .Add(HANDLE_DEPENDS, get_handle_depends_list)
          .Add(CONSTRAINT_VAR, get_handle_variable)
          .Add(STATE, get_state)
          .Add(Am_ID, NULL)
          .Add(Am_FILL_STYLE, get_state_color)
          .Add_Part(Am_Rectangle.Create()
                        .Set(Am_LEFT, 0)
                        .Set(Am_TOP, 0)
                        .Set(Am_WIDTH, compute_handle_width)
                        .Set(Am_FILL_STYLE, Am_From_Owner(Am_FILL_STYLE))
                        .Set(Am_LINE_STYLE, Am_White))
          .Add_Part(TEXT_HANDLE,
                    Am_Text.Create()
                        .Set(Am_TEXT, compute_handle_text)
                        .Set(Am_VISIBLE, is_text_valid)
                        .Set(Am_LEFT, 1)
                        .Set(Am_TOP, 1)
                        .Set(Am_FONT, Am_Font(Am_FONT_SERIF, false, false,
                                              false, Am_FONT_SMALL))
                        .Set(Am_FILL_STYLE, Am_From_Owner(Am_FILL_STYLE))
                        .Set(Am_LINE_STYLE, Am_White));

  One_Constraint_Handle = Am_Group.Create()
                              .Add(Am_ITEM, NULL)
                              .Add(LEFT_MOST_HANDLE, NULL)
                              .Set(Am_LEFT, compute_handle_lt)
                              .Set(Am_WIDTH, Am_Width_Of_Parts)
                              .Set(Am_HEIGHT, Am_Height_Of_Parts);

  one_handle = handle_proto.Create()
                   .Set(Am_ID, CH_LEFT)
                   .Set(Am_VISIBLE, rect_visible)
                   .Set(CENTER_X, compute_center_x_rect)
                   .Set(Am_TOP, compute_top_rect);

  Am_Object one_line;

  One_Constraint_Handle.Add_Part(LEFT_HANDLE, one_handle)
      .Add_Part(Am_Line.Create()
                    .Set(Am_WIDTH, 1)
                    .Set(Am_VISIBLE, rect_visible)
                    .Set(Am_LINE_STYLE, Am_Dotted_Line)
                    .Set(Am_HEIGHT, Am_From_Sibling(Am_ITEM, Am_HEIGHT))
                    .Set(Am_LEFT, Am_From_Sibling(TOP_HANDLE, CENTER_X))
                    .Set(Am_TOP, HANDLE_SPACING))
      .Add_Part(
          Am_Line.Create()
              .Set(Am_HEIGHT, 1)
              .Set(Am_VISIBLE, rect_visible)
              .Set(Am_LINE_STYLE, Am_Dotted_Line)
              .Set(Am_WIDTH, Am_From_Sibling(Am_ITEM, Am_WIDTH))
              .Set(Am_LEFT, Am_From_Sibling(LEFT_HANDLE, Am_WIDTH, 0, 0.5))
              .Set(Am_TOP,
                   Am_From_Sibling(LEFT_HANDLE, Am_TOP, C_HANDLE_SIZE_D2 - 1)))
      .Add_Part(BOTTOM_HANDLE, one_handle.Copy().Set(Am_ID, CH_BOTTOM))
      .Add_Part(RIGHT_HANDLE, one_handle.Copy().Set(Am_ID, CH_RIGHT))
      .Add_Part(CX_HANDLE, one_handle.Copy().Set(Am_ID, CH_CENTER_X))
      .Add_Part(CY_HANDLE, one_handle.Copy().Set(Am_ID, CH_CENTER_Y))
      .Add_Part(HEIGHT_HANDLE, one_handle.Copy().Set(Am_ID, CH_HEIGHT))
      .Add_Part(TOP_HANDLE, one_handle.Copy().Set(Am_ID, CH_TOP))
      .Add_Part(WIDTH_HANDLE, one_handle.Copy().Set(Am_ID, CH_WIDTH));

  one_line = draw_arrow_lines(true);
  One_Constraint_Handle.Add_Part(
      one_line.Set(Am_HEIGHT, Am_From_Sibling(Am_ITEM, Am_HEIGHT))
          .Set(Am_TOP, HANDLE_SPACING)
          .Set(Am_LEFT, Am_From_Sibling(HEIGHT_HANDLE, CENTER_X, -3)));

  one_line = draw_arrow_lines(false);
  One_Constraint_Handle.Add_Part(
      one_line.Set(Am_WIDTH, Am_From_Sibling(Am_ITEM, Am_WIDTH))
          .Set(Am_TOP, C_HANDLE_SIZE_D2 - 3)
          .Set(Am_LEFT, Am_From_Sibling(LEFT_HANDLE, CENTER_X)));

  one_line = Am_Line.Create()
                 .Set(Am_LINE_STYLE, Am_Dotted_Line)
                 .Set(Am_WIDTH, compute_vhline_width)
                 .Set(Am_HEIGHT, compute_vhline_height)
                 .Set(Am_TOP, compute_vhline_top)
                 .Set(Am_LEFT, compute_vhline_left);

  //vertical border lines
  One_Constraint_Handle
      .Add_Part(one_line.Add(Am_ID, 0))         //vertical left
      .Add_Part(one_line.Copy().Set(Am_ID, 1))  //vertical right
      .Add_Part(one_line.Copy().Set(Am_ID, 2))  //horizontal top
      .Add_Part(one_line.Copy().Set(Am_ID, 3)); //horizontal bottom

  Am_To_Top(one_handle);
  Am_To_Top(One_Constraint_Handle.Get_Object(WIDTH_HANDLE));
  Am_To_Top(One_Constraint_Handle.Get_Object(HEIGHT_HANDLE));

  one_handle = handle_proto.Create()
                   .Set(Am_ID, CH_X1)
                   .Set(Am_VISIBLE, line_visible)
                   .Set(CENTER_X, compute_center_x_line)
                   .Set(Am_TOP, compute_top_line);

  One_Constraint_Handle.Add_Part(X1_HANDLE, one_handle)
      .Add_Part(X2_HANDLE, one_handle.Copy().Set(Am_ID, CH_X2))
      .Add_Part(Y1_HANDLE, one_handle.Copy().Set(Am_ID, CH_Y1))
      .Add_Part(Y2_HANDLE, one_handle.Copy().Set(Am_ID, CH_Y2));

  Constraint_Widget = Modified_Selection_Widget.Create("Constraint_Widget")
                          .Remove_Part(Am_MOVE_GROW_COMMAND)
                          .Remove_Part(Am_MOVE_SEL_FOR_BACK_INTERACTOR)
                          .Remove_Part(Am_MOVE_INTERACTOR)
                          .Remove_Part(Am_GROW_INTERACTOR)
                          .Get_Object(Am_INTERACTOR)
                          .Set(Am_DO_METHOD, constraint_sel_object)
                          .Get_Owner()
                          .Add(PRIMARY_OBJ, NULL)
                          .Add(PRIMARY_HANDLE, NULL)
                          .Add(SECONDARY_HANDLE, NULL)
                          .Add(SECONDARY_OBJ, NULL);

  Am_Object lfeedback = Constraint_Widget.Get_Object(Am_LINE_FEEDBACK_OBJECT);
  lfeedback
      .Set(Am_LINE_STYLE, Am_Thin_Line) //change this to arrow line
      .Set(Am_OPERATES_ON, Am_From_Owner(Am_OPERATES_ON));

  Constraint_Widget.Set(Am_LINE_FEEDBACK_OBJECT, lfeedback)
      .Add(CONSTRAINT_VAR, (Am_Ptr)(Gilt_Constraint_Variable_Data
                                        *)(new Gilt_Constraint_Variable_Data()))
      .Add_Part(CONSTRAINT_SETUP_INTERACTOR,
                Am_New_Points_Interactor.Create("constraint setup interactor")
                    .Set(Am_AS_LINE, true)
                    .Set(Am_START_WHERE_TEST, compute_constraint_start_where)
                    .Set(Am_PRIORITY, 5.0)
                    .Set(Am_FEEDBACK_OBJECT, lfeedback)
                    .Set(Am_START_WHEN, compute_drag_start_when)
                    .Set(Am_STOP_WHEN, "LEFT_UP")
                    .Set(Am_ACTIVE, Am_From_Owner(Am_ACTIVE))
                    .Set(Am_START_DO_METHOD, set_primary_object)
                    .Set(Am_INTERIM_DO_METHOD, look_for_secondary_object)
                    .Set(Am_DO_METHOD, setup_constraint_and_clear_slots)
                    .Set(Am_IMPLEMENTATION_COMMAND, 0, Am_OK_IF_NOT_THERE)
                    .Get_Object(Am_COMMAND)
                    .Set(Am_UNDO_METHOD, restore_old_value)
                    .Set(Am_REDO_METHOD, restore_old_value)
                    .Get_Owner())
      .Get_Object(Am_INTERACTOR)
      .Set(Am_PRIORITY, 3.5) //higher than its prototype
      //On PC if same priority as 3.0 it crashes
      //and it screws up value in SLOT Am_RUN_ALSO
      .Set(Am_RUN_ALSO, false)
      .Get_Owner()
      .Get_Object(Am_FEEDBACK_OBJECT)
      .Set_Part(Am_ITEM_PROTOTYPE,
                One_Constraint_Handle.Create("constraint handle proto"))
      .Get_Owner();

  Offset_Percentage_Dialog = Am_Window.Create("Offset_Percentage_Dialog")
                                 .Set(Am_DESTROY_WINDOW_METHOD,
                                      Am_Default_Pop_Up_Window_Destroy_Method)
                                 .Set(Am_FILL_STYLE, Am_Motif_Gray)
                                 .Set(Am_TITLE, "Constraint Setup")
                                 .Set(Am_ICON_TITLE, "Constraint Setup")
                                 .Set(Am_WIDTH, Am_Width_Of_Parts)
                                 .Set(Am_HEIGHT, Am_Height_Of_Parts);
  Offset_Percentage_Dialog
      .Add_Part(Am_Border_Rectangle.Create()
                    .Set(Am_LEFT, 17)
                    .Set(Am_TOP, 13)
                    .Set(Am_WIDTH, 168)
                    .Set(Am_HEIGHT, 94)
                    .Set(Am_SELECTED, 0)
                    .Set(Am_FILL_STYLE, Am_Motif_Gray))
      .Add_Part(OFFSET_INPUT, Am_Number_Input_Widget.Create()
                                  .Set(Am_LEFT, 33)
                                  .Set(Am_TOP, 32)
                                  .Set(Am_WIDTH, 137)
                                  .Set(Am_HEIGHT, 25)
                                  .Set(Am_FILL_STYLE, Am_Amulet_Purple)
                                  .Get_Object(Am_COMMAND)
                                  .Set(Am_LABEL, "Offset : ")
                                  .Get_Owner()
                                  .Set(Am_VALUE_1, Am_No_Value)
                                  .Set(Am_VALUE_2, Am_No_Value))
      .Add_Part(PERCENT_INPUT, Am_Number_Input_Widget.Create()
                                   .Set(Am_LEFT, 33)
                                   .Set(Am_TOP, 68)
                                   .Set(Am_WIDTH, 136)
                                   .Set(Am_HEIGHT, 25)
                                   .Set(Am_FILL_STYLE, Am_Amulet_Purple)
                                   .Get_Object(Am_COMMAND)
                                   .Set(Am_LABEL, "Percentage : ")
                                   .Get_Owner()
                                   .Set(Am_VALUE_1, Am_No_Value)
                                   .Set(Am_VALUE_2, Am_No_Value))
      .Add_Part(Am_Button.Create()
                    .Set(Am_LEFT, 14)
                    .Set(Am_TOP, 117)
                    .Set(Am_FILL_STYLE, Am_Motif_Gray)
                    .Get_Object(Am_COMMAND)
                    .Set(Am_LABEL, "    Custom    ")
                    .Set(Am_DO_METHOD, create_custom_constraint)
                    .Get_Owner())
      .Add_Part(Am_Button_Panel.Create()
                    .Set(Am_LEFT, 14)
                    .Set(Am_TOP, 150)
                    .Set(Am_FILL_STYLE, Am_Motif_Gray)
                    .Set(Am_ITEMS,
                         Am_Value_List()
                             .Add(Am_Command.Create()
                                      .Set(Am_LABEL, "   Apply  ")
                                      .Set(Am_DO_METHOD, return_constraint_info)
                                      .Set(Am_ID, 1))
                             .Add(Am_Command.Create()
                                      .Set(Am_LABEL, "  Cancel  ")
                                      .Set(Am_DO_METHOD, return_constraint_info)
                                      .Set(Am_ID, 2)))
                    .Set(Am_LAYOUT, Am_Horizontal_Layout)
                    .Set(Am_H_SPACING, 20)
                    .Set(Am_V_SPACING, 0)
                    .Set(Am_MAX_RANK, 0))
      .Add_Part(Am_Tab_To_Next_Widget_Interactor.Create());
}
