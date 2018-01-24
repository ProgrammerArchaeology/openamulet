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

// This file contains the functions for handling the Move_Grow Interactor
//
// Designed and implemented by Brad Myers

#include <am_inc.h>

#include <amulet/am_io.h>
#include <amulet/impl/types_logging.h>
#include <amulet/impl/types_wrapper.h>
#include <amulet/impl/am_object.h>
#include <amulet/impl/slots_opal.h>
#include <amulet/impl/am_inter_location.h>
#include <amulet/impl/inter_debug.h>
#include <amulet/impl/opal_op.h>
#include <amulet/impl/opal_objects.h>
#include <amulet/initializer.h>

#include <math.h> // needed for sqrt for min-length of lines

//-------------------------------------------------------
// Functions to make Am_Inter_Location work as a Wrapper
//-------------------------------------------------------

class Am_Inter_Location_Data : public Am_Wrapper
{
  AM_WRAPPER_DATA_DECL(Am_Inter_Location)

public:
  // empty
  Am_Inter_Location_Data();
  Am_Inter_Location_Data(bool as_line, Am_Object ref_obj, int a, int b, int c,
                         int d, bool grow, bool direct);

  // to print out the contents
  void Print(std::ostream &out) const;

  // required by wrapper
  Am_Inter_Location_Data(Am_Inter_Location_Data *proto);

  // required by wrapper
  bool operator==(const Am_Inter_Location_Data &test_data) const
  {
    return ((ref_obj == test_data.ref_obj) && (as_line == test_data.as_line) &&
            (growing == test_data.growing) &&
            (data.line.x1 == test_data.data.line.x1) &&
            (data.line.y1 == test_data.data.line.y1) &&
            (data.line.x2 == test_data.data.line.x2) &&
            (data.line.y2 == test_data.data.line.y2));
  }

  // will be the window the coordinates are w.r.t.
  Am_Object ref_obj;
  // whether the location is two ends or width-height
  bool as_line;
  // whether the location has size also
  bool growing;

  // if location is a line, then line may not be distinguishable
  // when reversed.  (Directional for non-lines means that width
  // and height are not allowed to be negative [which is always true]).
  bool directional;

  Am_Location_Value_Control a_control;
  Am_Location_Value_Control b_control;
  Am_Location_Value_Control c_control;
  Am_Location_Value_Control d_control;

  union
  {
    struct
    {
      int left;
      int top;
      int width;
      int height;
    } rect;

    struct
    {
      int x1;
      int y1;
      int x2;
      int y2;
    } line;

  } data;
};

AM_WRAPPER_IMPL(Am_Inter_Location);

Am_Inter_Location Am_No_Location;

Am_Inter_Location::Am_Inter_Location() { data = nullptr; }

Am_Inter_Location::Am_Inter_Location(bool as_line, Am_Object ref_obj, int a,
                                     int b, int c, int d, bool grow,
                                     bool direct)
{
  data = new Am_Inter_Location_Data(as_line, ref_obj, a, b, c, d, grow, direct);
}

Am_Inter_Location::Am_Inter_Location(const Am_Object &object)
{
  Am_Object owner = object.Get_Owner();
  bool as_line = false;
  if (object.Is_Instance_Of(Am_Line))
    as_line = true;
  else {
    Am_Value value;
    value = object.Peek(Am_AS_LINE);
    as_line = value.Valid();
  }
  int a, b, c, d;
  bool directional;
  if (as_line) {
    a = object.Get(Am_X1);
    b = object.Get(Am_Y1);
    c = object.Get(Am_X2);
    d = object.Get(Am_Y2);
    directional = object.Get(Am_DIRECTIONAL);
  } else {
    a = object.Get(Am_LEFT);
    b = object.Get(Am_TOP);
    c = object.Get(Am_WIDTH);
    d = object.Get(Am_HEIGHT);
    directional = true;
  }
  data =
      new Am_Inter_Location_Data(as_line, owner, a, b, c, d, true, directional);
}

void
Am_Inter_Location::Set_Location(bool as_line, Am_Object ref_obj, int a, int b,
                                int c, int d, bool make_unique)
{
  if (data) {
    if (make_unique)
      data = (Am_Inter_Location_Data *)data->Make_Unique();
    data->as_line = as_line;
    data->growing = true;
    data->directional = true;
    data->ref_obj = ref_obj;
    data->data.line.x1 = a;
    data->data.line.y1 = b;
    data->data.line.x2 = c;
    data->data.line.y2 = d;
  } else
    data = new Am_Inter_Location_Data(as_line, ref_obj, a, b, c, d, true, true);
}

Am_Inter_Location
Am_Inter_Location::Copy() const
{
  Am_Inter_Location ret;
  if (data)
    ret.data = new Am_Inter_Location_Data(data);
  return ret;
}

void
Am_Inter_Location::Set_Points(int a, int b, int c, int d, bool make_unique)
{
  if (data) {
    if (make_unique)
      data = (Am_Inter_Location_Data *)data->Make_Unique();
  } else
    data = new Am_Inter_Location_Data();
  data->data.line.x1 = a;
  data->data.line.y1 = b;
  data->data.line.x2 = c;
  data->data.line.y2 = d;
}

void
Am_Inter_Location::Set_Ref_Obj(const Am_Object &ref_obj, bool make_unique)
{
  if (data) {
    if (make_unique)
      data = (Am_Inter_Location_Data *)data->Make_Unique();
  } else
    data = new Am_Inter_Location_Data();
  data->ref_obj = ref_obj;
}

void
Am_Inter_Location::Set_Growing(bool growing, bool make_unique)
{
  if (data) {
    if (make_unique)
      data = (Am_Inter_Location_Data *)data->Make_Unique();
  } else
    data = new Am_Inter_Location_Data();
  data->growing = growing;
}

bool
Am_Inter_Location::Get_Growing() const
{
  if (!data)
    Am_Error("Am_Inter_Location not initialized");
  return data->growing;
}

void
Am_Inter_Location::Set_Directional(bool directional, bool make_unique)
{
  if (data) {
    if (make_unique)
      data = (Am_Inter_Location_Data *)data->Make_Unique();
  } else
    data = new Am_Inter_Location_Data();
  data->directional = directional;
}

bool
Am_Inter_Location::Get_Directional() const
{
  if (!data)
    Am_Error("Am_Inter_Location not initialized");
  return data->directional;
}

void
Am_Inter_Location::Set_Control(Am_Location_Value_Control a_control,
                               Am_Location_Value_Control b_control,
                               Am_Location_Value_Control c_control,
                               Am_Location_Value_Control d_control,
                               bool make_unique)
{
  if (data) {
    if (make_unique)
      data = (Am_Inter_Location_Data *)data->Make_Unique();
  } else
    data = new Am_Inter_Location_Data();
  data->a_control = a_control;
  data->b_control = b_control;
  data->c_control = c_control;
  data->d_control = d_control;
}
void
Am_Inter_Location::Set_Control(Am_Location_Value_Control all_control,
                               bool make_unique)
{
  if (data) {
    if (make_unique)
      data = (Am_Inter_Location_Data *)data->Make_Unique();
  } else
    data = new Am_Inter_Location_Data();
  data->a_control = all_control;
  data->b_control = all_control;
  data->c_control = all_control;
  data->d_control = all_control;
}
void
Am_Inter_Location::Set_A_Control(Am_Location_Value_Control control,
                                 bool make_unique)
{
  if (data) {
    if (make_unique)
      data = (Am_Inter_Location_Data *)data->Make_Unique();
  } else
    data = new Am_Inter_Location_Data();
  data->a_control = control;
}
void
Am_Inter_Location::Set_B_Control(Am_Location_Value_Control control,
                                 bool make_unique)
{
  if (data) {
    if (make_unique)
      data = (Am_Inter_Location_Data *)data->Make_Unique();
  } else
    data = new Am_Inter_Location_Data();
  data->b_control = control;
}
void
Am_Inter_Location::Set_C_Control(Am_Location_Value_Control control,
                                 bool make_unique)
{
  if (data) {
    if (make_unique)
      data = (Am_Inter_Location_Data *)data->Make_Unique();
  } else
    data = new Am_Inter_Location_Data();
  data->c_control = control;
}
void
Am_Inter_Location::Set_D_Control(Am_Location_Value_Control control,
                                 bool make_unique)
{
  if (data) {
    if (make_unique)
      data = (Am_Inter_Location_Data *)data->Make_Unique();
  } else
    data = new Am_Inter_Location_Data();
  data->d_control = control;
}
void
Am_Inter_Location::Get_Control(Am_Location_Value_Control &a_control,
                               Am_Location_Value_Control &b_control,
                               Am_Location_Value_Control &c_control,
                               Am_Location_Value_Control &d_control) const
{
  if (!data)
    Am_Error("Am_Inter_Location not initialized");
  a_control = data->a_control;
  b_control = data->b_control;
  c_control = data->c_control;
  d_control = data->d_control;
}

void
Am_Inter_Location::Get_Control(Am_Location_Value_Control &a_control,
                               Am_Location_Value_Control &b_control) const
{
  if (!data)
    Am_Error("Am_Inter_Location not initialized");
  a_control = data->a_control;
  b_control = data->b_control;
}

void
Am_Inter_Location::Set_As_Line(bool as_line, bool make_unique)
{
  if (data) {
    if (make_unique)
      data = (Am_Inter_Location_Data *)data->Make_Unique();
  } else
    data = new Am_Inter_Location_Data();
  data->as_line = as_line;
}

void
Am_Inter_Location::Copy_From(Am_Inter_Location &other_data, bool make_unique)
{
  if (data) {
    if (make_unique)
      data = (Am_Inter_Location_Data *)data->Make_Unique();
  } else
    data = new Am_Inter_Location_Data();
  other_data.Get_Location(data->as_line, data->ref_obj, data->data.line.x1,
                          data->data.line.y1, data->data.line.x2,
                          data->data.line.y2);
}

void
Am_Inter_Location::Swap_With(Am_Inter_Location &other_obj, bool make_unique)
{
  bool tmp_as_line;
  Am_Object tmp_ref_obj;
  int a, b, c, d;
  Get_Location(tmp_as_line, tmp_ref_obj, a, b, c, d);
  Copy_From(other_obj, make_unique);
  other_obj.Set_Location(tmp_as_line, tmp_ref_obj, a, b, c, d, make_unique);
}

void
Am_Inter_Location::Set_Location(bool as_line, Am_Object ref_obj, int a, int b,
                                bool make_unique)
{
  if (data) {
    if (make_unique)
      data = (Am_Inter_Location_Data *)data->Make_Unique();
    data->as_line = as_line;
    data->ref_obj = ref_obj;
    data->data.rect.left = a;
    data->data.rect.top = b;
    data->growing = false;
  } else
    Am_Error(
        "Set Location with only 2 values, but no existing width and height");
}

void
Am_Inter_Location::Get_Location(bool &as_line, Am_Object &ref_obj, int &a,
                                int &b, int &c, int &d) const
{
  if (!data)
    Am_Error("Am_Inter_Location not initialized");
  as_line = data->as_line;
  ref_obj = data->ref_obj;
  a = data->data.line.x1;
  b = data->data.line.y1;
  c = data->data.line.x2;
  d = data->data.line.y2;
}

void
Am_Inter_Location::Get_Points(int &a, int &b, int &c, int &d) const
{
  if (!data)
    Am_Error("Am_Inter_Location not initialized");
  a = data->data.line.x1;
  b = data->data.line.y1;
  c = data->data.line.x2;
  d = data->data.line.y2;
}

Am_Object
Am_Inter_Location::Get_Ref_Obj() const
{
  if (!data)
    Am_Error("Am_Inter_Location not initialized");
  return data->ref_obj;
}

void
Am_Inter_Location::Get_As_Line(bool &as_line) const
{
  if (!data)
    Am_Error("Am_Inter_Location not initialized");
  as_line = data->as_line;
}

//trans_coord from ref_obj to dest_obj, returns true if translation is OK
bool
Am_Inter_Location::Translate_To(Am_Object dest_obj)
{
  data = (Am_Inter_Location_Data *)data->Make_Unique();
  bool ok = true;
  if (!data)
    Am_Error("Am_Inter_Location not initialized");
  if (data->ref_obj != dest_obj) {
    ok = Am_Translate_Coordinates(data->ref_obj, data->data.line.x1,
                                  data->data.line.y1, dest_obj,
                                  data->data.line.x1, data->data.line.y1);
    if (data->as_line)
      Am_Translate_Coordinates(data->ref_obj, data->data.line.x2,
                               data->data.line.y2, dest_obj, data->data.line.x2,
                               data->data.line.y2);
    data->ref_obj = dest_obj;
  }
  return ok;
}

bool
Am_Inter_Location::operator==(const Am_Inter_Location &test) const
{
  return data == test.data || (data && test.data && *data == *test.data);
}

bool
Am_Inter_Location::operator!=(const Am_Inter_Location &test) const
{
  return data != test.data && (!data || !test.data || !(*data == *test.data));
}

inline void
isort(int a, int b, int &out_a, int &out_b)
{
  if (a < b) {
    out_a = a;
    out_b = b;
  } else {
    out_a = b;
    out_b = a;
  }
}

// A >= B is A contains B.  A must be non-line, B can be line or not line
bool
Am_Inter_Location::operator>=(const Am_Inter_Location &test) const
{
  int test_left, test_top, test_right, test_bottom;
  int my_left, my_top, my_right, my_bottom;

  if (!data)
    Am_Error("Am_Inter_Location not initialized");
  if (!test.data)
    Am_Error("Other Am_Inter_Location not initialized");
  if (data->as_line)
    Am_Error("Can't test contain inside an Am_Inter_Location that is a Line");

  my_left = data->data.rect.left;
  my_top = data->data.rect.top;
  my_right = my_left + data->data.rect.width + 1;
  my_bottom = my_top + data->data.rect.height + 1;

  if (test.data->as_line) {
    test_left = test.data->data.line.x1;
    test_top = test.data->data.line.y1;
    test_right = test.data->data.line.x2;
    test_bottom = test.data->data.line.y2;
    isort(test_left, test_right, test_left, test_right);
    isort(test_top, test_bottom, test_top, test_bottom);
  } else {
    test_left = test.data->data.rect.left;
    test_top = test.data->data.rect.top;
    test_right = test_left + test.data->data.rect.width + 1;
    test_bottom = test_top + test.data->data.rect.height + 1;
  }

  if (data->ref_obj != test.data->ref_obj) {
    if (!Am_Translate_Coordinates(test.data->ref_obj, test_left, test_top,
                                  data->ref_obj, test_left, test_top))
      return false; //not in the same window
    Am_Translate_Coordinates(test.data->ref_obj, test_right, test_bottom,
                             data->ref_obj, test_right, test_bottom);
  }

  //now ready to do the comparisons
  if (test_left < my_left || test_left > my_right || test_right > my_right)
    return false;
  if (test_top < my_top || test_top > my_bottom || test_bottom > my_bottom)
    return false;
  return true;
}

bool
Am_Inter_Location::operator&&(const Am_Inter_Location &test) const
{
  int test_left, test_top, test_right, test_bottom;
  test.Get_Points(test_left, test_top, test_right, test_bottom);
  bool test_as_line;
  test.Get_As_Line(test_as_line);
  if (test_as_line) {
    isort(test_left, test_right, test_left, test_right);
    isort(test_top, test_bottom, test_top, test_bottom);
  } else {
    test_right += test_left;
    test_bottom += test_top;
  }
  int left, top, right, bottom;
  left = data->data.line.x1;
  top = data->data.line.y1;
  right = data->data.line.x2;
  bottom = data->data.line.y2;
  if (data->as_line) {
    isort(left, right, left, right);
    isort(top, bottom, top, bottom);
  } else {
    right += left;
    bottom += top;
  }
  return (left < test_right) && (right >= test_left) && (top < test_bottom) &&
         (bottom >= test_top);
}

void
Am_Inter_Location::Install(Am_Object &object) const
{
  if (!data)
    Am_Error("Am_Inter_Location not initialized");
  Install(object, data->growing);
}

void
Am_Inter_Location::Install(Am_Object &object, bool growing) const
{
  if (!data)
    Am_Error("Am_Inter_Location not initialized");
  Am_Object owner = object.Get_Owner();
  int a = data->data.line.x1;
  int b = data->data.line.y1;
  int c = data->data.line.x2;
  int d = data->data.line.y2;

  if (data->ref_obj.Valid() && owner != data->ref_obj) {
    Am_INTER_TRACE_PRINT(Am_INTER_TRACE_SETTING,
                         "Translating set coordinates from "
                             << data->ref_obj << " to " << owner);
    Am_Translate_Coordinates(data->ref_obj, a, b, owner, a, b);
    if (data->as_line)
      Am_Translate_Coordinates(data->ref_obj, c, d, owner, c, d);
  }
  Am_INTER_TRACE_PRINT_NOENDL(Am_INTER_TRACE_SETTING,
                              "++Object setting obj position=" << object);
  if (data->as_line) {
    if (data->a_control == Am_LOCATION_RELATIVE)
      a = (int)object.Get(Am_X1) + data->data.line.x1;
    if (data->b_control == Am_LOCATION_RELATIVE)
      b = (int)object.Get(Am_Y1) + data->data.line.y1;
    if (data->c_control == Am_LOCATION_RELATIVE)
      c = (int)object.Get(Am_X2) + data->data.line.x2;
    if (data->d_control == Am_LOCATION_RELATIVE)
      d = (int)object.Get(Am_Y2) + data->data.line.y2;

    if (data->a_control != Am_LOCATION_NOT_USED) {
      Am_INTER_TRACE_PRINT_NOENDL(Am_INTER_TRACE_SETTING, " X1=" << a);
      object.Set(Am_X1, a); // , Am_NO_ANIMATION
    }
    if (data->b_control != Am_LOCATION_NOT_USED) {
      Am_INTER_TRACE_PRINT_NOENDL(Am_INTER_TRACE_SETTING, " Y1=" << b);
      object.Set(Am_Y1, b); // , Am_NO_ANIMATION
    }
    if (data->c_control != Am_LOCATION_NOT_USED) {
      Am_INTER_TRACE_PRINT_NOENDL(Am_INTER_TRACE_SETTING, " X2=" << c);
      object.Set(Am_X2, c); // , Am_NO_ANIMATION
    }
    if (data->d_control != Am_LOCATION_NOT_USED) {
      Am_INTER_TRACE_PRINT_NOENDL(Am_INTER_TRACE_SETTING, " Y2=" << d);
      object.Set(Am_Y2, d); // , Am_NO_ANIMATION
    }
  } else {
    if (data->a_control == Am_LOCATION_RELATIVE)
      a = (int)object.Get(Am_LEFT) + data->data.rect.left;
    if (data->b_control == Am_LOCATION_RELATIVE)
      b = (int)object.Get(Am_TOP) + data->data.rect.top;

    if (data->a_control != Am_LOCATION_NOT_USED) {
      Am_INTER_TRACE_PRINT_NOENDL(Am_INTER_TRACE_SETTING, " left=" << a);
      object.Set(Am_LEFT, a); // , Am_NO_ANIMATION
    }
    if (data->b_control != Am_LOCATION_NOT_USED) {
      Am_INTER_TRACE_PRINT_NOENDL(Am_INTER_TRACE_SETTING, " top=" << b);
      object.Set(Am_TOP, b); // , Am_NO_ANIMATION
    }
    if (growing) {
      if (data->c_control == Am_LOCATION_RELATIVE)
        c = (int)object.Get(Am_WIDTH) + data->data.rect.width;
      if (data->d_control == Am_LOCATION_RELATIVE)
        d = (int)object.Get(Am_HEIGHT) + data->data.rect.height;

      if (data->c_control != Am_LOCATION_NOT_USED) {
        Am_INTER_TRACE_PRINT_NOENDL(Am_INTER_TRACE_SETTING, " width=" << c);
        object.Set(Am_WIDTH, c); // , Am_NO_ANIMATION
      }
      if (data->d_control != Am_LOCATION_NOT_USED) {
        Am_INTER_TRACE_PRINT_NOENDL(Am_INTER_TRACE_SETTING, " height=" << d);
        object.Set(Am_HEIGHT, d); // , Am_NO_ANIMATION
      }
    }
  }
  Am_INTER_TRACE_PRINT_NOENDL(Am_INTER_TRACE_SETTING, std::endl << std::flush);
}

bool
Am_Inter_Location::Is_Zero_Size() const
{
  if (!data)
    Am_Error("Am_Inter_Location not initialized");
  if (data->as_line) {
    if (data->data.line.x1 == data->data.line.x2 &&
        data->data.line.y1 == data->data.line.y2)
      return true;
  } else {
    if (data->data.rect.width <= 0 || data->data.rect.height <= 0)
      return true;
  }
  return false;
}

std::ostream &
operator<<(std::ostream &os, Am_Inter_Location &loc)
{
  loc.Print(os);
  return os;
}

AM_WRAPPER_DATA_IMPL(Am_Inter_Location, (this))

void
Am_Inter_Location_Data::Print(std::ostream &os) const
{
  os << "(";
  if (as_line) {
    os << data.line.x1 << "," << data.line.y1 << ").(" << data.line.x2 << ","
       << data.line.y2 << ")";
  } else if (growing)
    os << data.rect.left << "," << data.rect.top << "," << data.rect.width
       << "," << data.rect.height << ")";
  else
    os << data.rect.left << "," << data.rect.top << ")";
  os << " w.r.t. " << ref_obj;
}

Am_Inter_Location_Data::Am_Inter_Location_Data()
{
  data.line.x1 = 0;
  data.line.y1 = 0;
  data.line.x2 = 0;
  data.line.y2 = 0;
  as_line = false;
  growing = true;
  a_control = Am_LOCATION_OK;
  b_control = Am_LOCATION_OK;
  c_control = Am_LOCATION_OK;
  d_control = Am_LOCATION_OK;
  directional = true;
  ref_obj = Am_No_Object;
}

Am_Inter_Location_Data::Am_Inter_Location_Data(bool line, Am_Object ref, int a,
                                               int b, int c, int d, bool grow,
                                               bool direct)
{
  as_line = line;
  directional = direct;
  ref_obj = ref;
  growing = grow;
  data.line.x1 = a;
  data.line.y1 = b;
  data.line.x2 = c;
  data.line.y2 = d;
  a_control = Am_LOCATION_OK;
  b_control = Am_LOCATION_OK;
  c_control = Am_LOCATION_OK;
  d_control = Am_LOCATION_OK;
}

Am_Inter_Location_Data::Am_Inter_Location_Data(Am_Inter_Location_Data *proto)
{
  ref_obj = proto->ref_obj;
  as_line = proto->as_line;
  growing = proto->growing;
  a_control = proto->a_control;
  b_control = proto->b_control;
  c_control = proto->c_control;
  d_control = proto->d_control;
  data = proto->data;
}
