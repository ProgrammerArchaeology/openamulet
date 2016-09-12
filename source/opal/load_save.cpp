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

#include <am_inc.h>

#include <amulet/impl/types_logging.h>
#include <amulet/impl/types_string.h>
#include <amulet/impl/types_load_save.h>
#include <amulet/impl/am_style.h>
#include <amulet/impl/am_object.h>
#include <amulet/impl/am_point_list.h>
#include <amulet/impl/opal_objects.h>
#include <amulet/impl/opal_load_save.h>
#include <amulet/impl/slots.h>
#include <amulet/impl/object_misc.h>
#include <amulet/value_list.h>
#include <amulet/initializer.h>

#if 1
///////////////////////////////////
// Default Load and Save Methods //
///////////////////////////////////

// For loading and saving files.
Am_Load_Save_Context Am_Default_Load_Save_Context;

Am_Define_Method(Am_Load_Method, Am_Value, load_none,
                 (std::istream & is, Am_Load_Save_Context & /* context */))
{
  char ch(0);
  is.get(ch);        // skip eoln
  return Am_Value(); //null Am_Value
}

Am_Define_Method(Am_Save_Method, void, save_none,
                 (std::ostream & os, Am_Load_Save_Context &context,
                  const Am_Value & /* value */))
{
  context.Save_Type_Name(os, "Am_NONE");
  os << std::endl;
}

Am_Define_Method(Am_Load_Method, Am_Value, load_int,
                 (std::istream & is, Am_Load_Save_Context & /* context */))
{
  int value;
  is >> value;
  char ch(0);
  is.get(ch); // skip eoln
  return Am_Value(value);
}

Am_Define_Method(Am_Save_Method, void, save_int,
                 (std::ostream & os, Am_Load_Save_Context &context,
                  const Am_Value &value))
{
  context.Save_Type_Name(os, "Am_INT");
  os << (int)value << std::endl;
}

Am_Define_Method(Am_Load_Method, Am_Value, load_long,
                 (std::istream & is, Am_Load_Save_Context & /* context */))
{
  long value;
  is >> value;
  char ch;
  is.get(ch); // skip eoln
  return Am_Value(value);
}

Am_Define_Method(Am_Save_Method, void, save_long,
                 (std::ostream & os, Am_Load_Save_Context &context,
                  const Am_Value &value))
{
  context.Save_Type_Name(os, "Am_LONG");
  os << (long)value << std::endl;
}

Am_Define_Method(Am_Load_Method, Am_Value, load_bool,
                 (std::istream & is, Am_Load_Save_Context & /* context */))
{
  char value[10];
  is >> value;
  char ch;
  is.get(ch); // skip eoln
  return Am_Value((bool)!strcmp(value, "true"));
}

Am_Define_Method(Am_Save_Method, void, save_bool,
                 (std::ostream & os, Am_Load_Save_Context &context,
                  const Am_Value &value))
{
  context.Save_Type_Name(os, "Am_BOOL");
  if ((bool)value)
    os << "true" << std::endl;
  else
    os << "false" << std::endl;
}

Am_Define_Method(Am_Load_Method, Am_Value, load_float,
                 (std::istream & is, Am_Load_Save_Context & /* context */))
{
  float value;
  is >> value;
  char ch;
  is.get(ch); // skip eoln
  return Am_Value(value);
}

Am_Define_Method(Am_Save_Method, void, save_float,
                 (std::ostream & os, Am_Load_Save_Context &context,
                  const Am_Value &value))
{
  context.Save_Type_Name(os, "Am_FLOAT");
  os << (float)value << std::endl;
}

Am_Define_Method(Am_Load_Method, Am_Value, load_double,
                 (std::istream & is, Am_Load_Save_Context & /* context */))
{
  double value;
  is >> value;
  char ch;
  is.get(ch); // skip eoln
  return Am_Value(value);
}

Am_Define_Method(Am_Save_Method, void, save_double,
                 (std::ostream & os, Am_Load_Save_Context &context,
                  const Am_Value &value))
{
  context.Save_Type_Name(os, "Am_DOUBLE");
  os << (double)value << std::endl;
}

Am_Define_Method(Am_Load_Method, Am_Value, load_char,
                 (std::istream & is, Am_Load_Save_Context & /* context */))
{
  char value;
  is >> value;
  char ch;
  is.get(ch); // skip eoln
  return Am_Value(value);
}

Am_Define_Method(Am_Save_Method, void, save_char,
                 (std::ostream & os, Am_Load_Save_Context &context,
                  const Am_Value &value))
{
  context.Save_Type_Name(os, "Am_CHAR");
  os << (char)value << std::endl;
}

Am_Define_Method(Am_Load_Method, Am_Value, load_string,
                 (std::istream & is, Am_Load_Save_Context & /* context */))
{
  char ch;
  int length;
  is >> length;
  is.get(ch); // skip eoln
  char *value = new char[length + 1];
  int i;
  for (i = 0; i < length; ++i) {
    is.get(value[i]);
  }
  value[length] = '\0';
  is.get(ch); // skip eoln
  Am_String string(value);
  delete[] value;
  return Am_Value(string);
}

Am_Define_Method(Am_Save_Method, void, save_string,
                 (std::ostream & os, Am_Load_Save_Context &context,
                  const Am_Value &value))
{
  context.Save_Type_Name(os, "Am_STRING");
  Am_String string = value;
  const char *string_val = string;
  int length = strlen(string_val);
  os << length << std::endl;
  int i;
  for (i = 0; i < length; ++i) {
    os << string_val[i];
  }
  os << std::endl;
}

Am_Define_Method(Am_Load_Method, Am_Value, load_list,
                 (std::istream & is, Am_Load_Save_Context &context))
{
  Am_Value_List list = Am_Value_List::Empty_List();
  context.Recursive_Load_Ahead(list);
  int length;
  is >> length;
  char ch;
  is.get(ch); // skip eoln
  int i;
  Am_Value v;
  for (i = 0; i < length; ++i) {
    v = context.Load(is);
    //if any errors, return immediately
    if (v == Am_No_Value)
      return Am_No_Value;
    list.Add(v, Am_TAIL, false);
  }
  return Am_Value(list);
}

Am_Define_Method(Am_Save_Method, void, save_list,
                 (std::ostream & os, Am_Load_Save_Context &context,
                  const Am_Value &value))
{
  context.Save_Type_Name(os, "Am_VALUE_LIST");
  Am_Value_List list = value;
  os << list.Length() << std::endl;
  for (list.Start(); !list.Last(); list.Next())
    context.Save(os, list.Get());
}

Am_Define_Method(Am_Load_Method, Am_Value, load_point_list,
                 (std::istream & is, Am_Load_Save_Context & /*context*/))
{
  int length, i;
  is >> length;
  Am_Point_List list;
  float x, y;
  for (i = 0; i < length; ++i) {
    is >> x;
    is >> y;
    list.Add(x, y, Am_TAIL, false);
  }
  char ch;
  is.get(ch); // skip eoln
  return Am_Value(list);
}

Am_Define_Method(Am_Save_Method, void, save_point_list,
                 (std::ostream & os, Am_Load_Save_Context &context,
                  const Am_Value &value))
{
  context.Save_Type_Name(os, "Am_POINT_LIST");
  Am_Point_List list = value;
  os << list.Length() << std::endl;
  float x, y;
  for (list.Start(); !list.Last(); list.Next()) {
    list.Get(x, y);
    os << x << " " << y << std::endl;
  }
}

Am_Define_Method(Am_Save_Method, void, save_object,
                 (std::ostream & os, Am_Load_Save_Context &context,
                  const Am_Value &value))
{
  Am_Object object = value;
  Am_Save_Object_Method method = object.Get(Am_SAVE_OBJECT_METHOD);
  if (!method.Valid())
    Am_ERRORO(
        "Object "
            << object
            << " does not have a save method in its Am_SAVE_OBJECT_METHOD slot",
        object, Am_SAVE_OBJECT_METHOD);
  method.Call(os, context, object);
}

void
standard_save_internal(std::ostream &os, Am_Load_Save_Context &context,
                       const Am_Object &object, const char *save_type_name)
{
  // Tests to see if prototype is registered
  const char *name = context.Is_Registered_Prototype(object);
  Am_Object proto;
  if (name)
    proto = object;
  else {
    proto = object.Get_Prototype();
    name = context.Is_Registered_Prototype(proto);
    if (!name)
      Am_ERROR("Neither object "
               << object << " or its prototype " << proto
               << " have been registered for save for Am_Standard_Save_Object");
  }
  Am_Value_List slots = proto.Get(Am_SLOTS_TO_SAVE);
  context.Save_Type_Name(os, save_type_name);
  context.Save(os, Am_Value(proto));
  for (slots.Start(); !slots.Last(); slots.Next()) {
    Am_Slot_Key slot = static_cast<Am_Slot_Key>(static_cast<int>(slots.Get()));
    context.Save(os, object.Get(slot));
  }
}

Am_Define_Method(Am_Save_Object_Method, void, Am_Standard_Save_Object,
                 (std::ostream & os, Am_Load_Save_Context &context,
                  const Am_Object &object))
{
  standard_save_internal(os, context, object, "Am_STANDARD_SAVE");
}

Am_Define_Method(Am_Save_Object_Method, void, Am_Standard_Save_Group,
                 (std::ostream & os, Am_Load_Save_Context &context,
                  const Am_Object &object))
{
  standard_save_internal(os, context, object, "Am_STANDARD_SAVE_GROUP");
  Am_Value_List parts = object.Get(Am_GRAPHICAL_PARTS);
  os << parts.Length() << std::endl;
  Am_Object part;
  for (parts.Start(); !parts.Last(); parts.Next()) {
    Am_Value v = parts.Get();
    context.Save(os, v);
  }
}

Am_Object Am_Load_Object_Swap;

Am_Define_Method(Am_Load_Method, Am_Value, Am_Standard_Load_Object,
                 (std::istream & is, Am_Load_Save_Context &context))
{
  Am_Object loaded_obj = context.Load(is);
  if (!loaded_obj.Valid())
    return Am_No_Value;
  Am_Object proto;
  bool call_proto = false;
  if (loaded_obj.Is_Instance_Of(Am_Load_Object_Swap)) {
    proto = loaded_obj.Get(Am_ITEM_PROTOTYPE);
    if (!proto.Valid())
      return Am_No_Value;
    call_proto = true;
  } else
    proto = loaded_obj;
  Am_Object new_obj = proto.Create();
  context.Recursive_Load_Ahead(new_obj);
  Am_Value_List slots = loaded_obj.Get(Am_SLOTS_TO_SAVE);
  for (slots.Start(); !slots.Last(); slots.Next()) {
    Am_Slot_Key slot = static_cast<Am_Slot_Key>(static_cast<int>(slots.Get()));
    new_obj.Set(slot, context.Load(is), Am_OK_IF_NOT_THERE);
  }
  if (call_proto) {
    Am_Object_Method proto_method =
        loaded_obj.Get(Am_ITEM_METHOD, Am_RETURN_ZERO_ON_ERROR);
    // TODO: Check the following test. It seems proto_method is always Valid...
    if (proto_method.Valid())
      proto_method.Call(new_obj);
  }
  return Am_Value(new_obj);
}

Am_Define_Method(Am_Load_Method, Am_Value, Am_Standard_Load_Group,
                 (std::istream & is, Am_Load_Save_Context &context))
{
  Am_Value new_obj_val = Am_Standard_Load_Object_proc(is, context);
  if (new_obj_val.Valid()) {
    Am_Object new_obj = new_obj_val;
    int num_parts;
    is >> num_parts;
    for (int i = 0; i < num_parts; i++) {
      Am_Object part = context.Load(is);
      new_obj.Add_Part(part);
    }
  }
  return new_obj_val;
}

static void
init()
{
  //
  // Default Load and Save Methods
  Am_Default_Load_Save_Context.Register_Prototype("Am_RESIZE_PARTS_GROUP",
                                                  Am_Resize_Parts_Group);
  Am_Default_Load_Save_Context.Register_Loader("Am_NONE", load_none);
  Am_Default_Load_Save_Context.Register_Saver(Am_NONE, save_none);
  Am_Default_Load_Save_Context.Register_Loader("Am_INT", load_int);
  Am_Default_Load_Save_Context.Register_Saver(Am_INT, save_int);
  Am_Default_Load_Save_Context.Register_Loader("Am_LONG", load_long);
  Am_Default_Load_Save_Context.Register_Saver(Am_LONG, save_long);
  Am_Default_Load_Save_Context.Register_Loader("Am_BOOL", load_bool);
  Am_Default_Load_Save_Context.Register_Saver(Am_BOOL, save_bool);
  Am_Default_Load_Save_Context.Register_Loader("Am_FLOAT", load_float);
  Am_Default_Load_Save_Context.Register_Saver(Am_FLOAT, save_float);
  Am_Default_Load_Save_Context.Register_Loader("Am_DOUBLE", load_double);
  Am_Default_Load_Save_Context.Register_Saver(Am_DOUBLE, save_double);
  Am_Default_Load_Save_Context.Register_Loader("Am_CHAR", load_char);
  Am_Default_Load_Save_Context.Register_Saver(Am_CHAR, save_char);
  Am_Default_Load_Save_Context.Register_Loader("Am_STRING", load_string);
  Am_Default_Load_Save_Context.Register_Saver(Am_STRING, save_string);
  Am_Default_Load_Save_Context.Register_Loader("Am_VALUE_LIST", load_list);
  Am_Default_Load_Save_Context.Register_Saver(Am_Value_List::Type_ID(),
                                              save_list);
  Am_Default_Load_Save_Context.Register_Saver(Am_OBJECT, save_object);
  Am_Default_Load_Save_Context.Register_Loader("Am_STANDARD_SAVE",
                                               Am_Standard_Load_Object);
  Am_Default_Load_Save_Context.Register_Loader("Am_STANDARD_SAVE_GROUP",
                                               Am_Standard_Load_Group);
  Am_Default_Load_Save_Context.Register_Loader("Am_POINT_LIST",
                                               load_point_list);
  Am_Default_Load_Save_Context.Register_Saver(Am_Point_List::Type_ID(),
                                              save_point_list);

  Am_Load_Object_Swap = Am_Root_Object.Create(DSTR("Am_Load_Object_Swap"))
                            .Add(Am_ITEM_PROTOTYPE, Am_No_Object)
                            .Add(Am_SLOTS_TO_SAVE, Am_No_Value_List)
                            .Add(Am_ITEM_METHOD, Am_Object_Method());

  Am_Default_Load_Save_Context.Register_Prototype("Am_Red", Am_Red);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Green", Am_Green);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Blue", Am_Blue);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Yellow", Am_Yellow);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Purple", Am_Purple);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Cyan", Am_Cyan);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Orange", Am_Orange);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Black", Am_Black);
  Am_Default_Load_Save_Context.Register_Prototype("Am_White", Am_White);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Amulet_Purple",
                                                  Am_Amulet_Purple);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Motif_Gray",
                                                  Am_Motif_Gray);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Motif_Light_Gray",
                                                  Am_Motif_Light_Gray);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Motif_Blue",
                                                  Am_Motif_Blue);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Motif_Light_Blue",
                                                  Am_Motif_Light_Blue);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Motif_Green",
                                                  Am_Motif_Green);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Motif_Light_Green",
                                                  Am_Motif_Light_Green);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Motif_Orange",
                                                  Am_Motif_Orange);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Motif_Light_Orange",
                                                  Am_Motif_Light_Orange);
  // these all conflict with Am_Black
  //Am_Default_Load_Save_Context.Register_Prototype ("Am_Thin_Line",
  //						   Am_Thin_Line);
  //Am_Default_Load_Save_Context.Register_Prototype ("Am_Line_0", Am_Line_0);
  //Am_Default_Load_Save_Context.Register_Prototype ("Am_Line_1", Am_Line_1);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Line_2", Am_Line_2);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Line_4", Am_Line_4);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Line_8", Am_Line_8);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Dotted_Line",
                                                  Am_Dotted_Line);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Dashed_Line",
                                                  Am_Dashed_Line);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Gray_Stipple",
                                                  Am_Gray_Stipple);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Opaque_Gray_Stipple",
                                                  Am_Opaque_Gray_Stipple);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Light_Gray_Stipple",
                                                  Am_Light_Gray_Stipple);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Dark_Gray_Stipple",
                                                  Am_Dark_Gray_Stipple);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Diamond_Stipple",
                                                  Am_Diamond_Stipple);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Opaque_Diamond_Stipple",
                                                  Am_Opaque_Diamond_Stipple);
}

static Am_Initializer *initializer =
    new Am_Initializer(DSTR("Load_Save"), init, 2.01f);
#else
///////////////////////////////////
// Default Load and Save Methods //
///////////////////////////////////

// For loading and saving files.
Am_Load_Save_Context Am_Default_Load_Save_Context;

Am_Define_Method(Am_Load_Method, Am_Value, load_none,
                 (std::istream & is, Am_Load_Save_Context & /* context */))
{
  char ch(0);
  is.get(ch);        // skip eoln
  return Am_Value(); //null Am_Value
}

Am_Define_Method(Am_Save_Method, void, save_none,
                 (std::ostream & os, Am_Load_Save_Context &context,
                  const Am_Value & /* value */))
{
  context.Save_Type_Name(os, "Am_NONE");
  os << std::endl;
}

Am_Define_Method(Am_Load_Method, Am_Value, load_int,
                 (std::istream & is, Am_Load_Save_Context & /* context */))
{
  int value;
  is >> value;
  char ch(0);
  is.get(ch); // skip eoln
  return Am_Value(value);
}

Am_Define_Method(Am_Save_Method, void, save_int,
                 (std::ostream & os, Am_Load_Save_Context &context,
                  const Am_Value &value))
{
  context.Save_Type_Name(os, "Am_INT");
  os << (int)value << std::endl;
}

Am_Define_Method(Am_Load_Method, Am_Value, load_long,
                 (std::istream & is, Am_Load_Save_Context & /* context */))
{
  long value;
  is >> value;
  char ch;
  is.get(ch); // skip eoln
  return Am_Value(value);
}

Am_Define_Method(Am_Save_Method, void, save_long,
                 (std::ostream & os, Am_Load_Save_Context &context,
                  const Am_Value &value))
{
  context.Save_Type_Name(os, "Am_LONG");
  os << (long)value << std::endl;
}

Am_Define_Method(Am_Load_Method, Am_Value, load_bool,
                 (std::istream & is, Am_Load_Save_Context & /* context */))
{
  char value[10];
  is >> value;
  char ch;
  is.get(ch); // skip eoln
  return Am_Value((bool)!strcmp(value, "true"));
}

Am_Define_Method(Am_Save_Method, void, save_bool,
                 (std::ostream & os, Am_Load_Save_Context &context,
                  const Am_Value &value))
{
  context.Save_Type_Name(os, "Am_BOOL");
  if ((bool)value)
    os << "true" << std::endl;
  else
    os << "false" << std::endl;
}

Am_Define_Method(Am_Load_Method, Am_Value, load_float,
                 (std::istream & is, Am_Load_Save_Context & /* context */))
{
  float value;
  is >> value;
  char ch;
  is.get(ch); // skip eoln
  return Am_Value(value);
}

Am_Define_Method(Am_Save_Method, void, save_float,
                 (std::ostream & os, Am_Load_Save_Context &context,
                  const Am_Value &value))
{
  context.Save_Type_Name(os, "Am_FLOAT");
  os << (float)value << std::endl;
}

Am_Define_Method(Am_Load_Method, Am_Value, load_double,
                 (std::istream & is, Am_Load_Save_Context & /* context */))
{
  double value;
  is >> value;
  char ch;
  is.get(ch); // skip eoln
  return Am_Value(value);
}

Am_Define_Method(Am_Save_Method, void, save_double,
                 (std::ostream & os, Am_Load_Save_Context &context,
                  const Am_Value &value))
{
  context.Save_Type_Name(os, "Am_DOUBLE");
  os << (double)value << std::endl;
}

Am_Define_Method(Am_Load_Method, Am_Value, load_char,
                 (std::istream & is, Am_Load_Save_Context & /* context */))
{
  char value;
  is >> value;
  char ch;
  is.get(ch); // skip eoln
  return Am_Value(value);
}

Am_Define_Method(Am_Save_Method, void, save_char,
                 (std::ostream & os, Am_Load_Save_Context &context,
                  const Am_Value &value))
{
  context.Save_Type_Name(os, "Am_CHAR");
  os << (char)value << std::endl;
}

Am_Define_Method(Am_Load_Method, Am_Value, load_string,
                 (std::istream & is, Am_Load_Save_Context & /* context */))
{
  char ch;
  int length;
  is >> length;
  is.get(ch); // skip eoln
  char *value = new char[length + 1];
  int i;
  for (i = 0; i < length; ++i) {
    is.get(value[i]);
  }
  value[length] = '\0';
  is.get(ch); // skip eoln
  Am_String string(value);
  delete value;
  return Am_Value(string);
}

Am_Define_Method(Am_Save_Method, void, save_string,
                 (std::ostream & os, Am_Load_Save_Context &context,
                  const Am_Value &value))
{
  context.Save_Type_Name(os, "Am_STRING");
  Am_String string = value;
  const char *string_val = string;
  int length = strlen(string_val);
  os << length << std::endl;
  int i;
  for (i = 0; i < length; ++i) {
    os << string_val[i];
  }
  os << std::endl;
}

Am_Define_Method(Am_Load_Method, Am_Value, load_list,
                 (std::istream & is, Am_Load_Save_Context &context))
{
  Am_Value_List list = Am_Value_List::Empty_List();
  context.Recursive_Load_Ahead(list);
  int length;
  is >> length;
  char ch;
  is.get(ch); // skip eoln
  int i;
  Am_Value v;
  for (i = 0; i < length; ++i) {
    v = context.Load(is);
    //if any errors, return immediately
    if (v == Am_No_Value)
      return Am_No_Value;
    list.Add(v, Am_TAIL, false);
  }
  return Am_Value(list);
}

Am_Define_Method(Am_Save_Method, void, save_list,
                 (std::ostream & os, Am_Load_Save_Context &context,
                  const Am_Value &value))
{
  context.Save_Type_Name(os, "Am_VALUE_LIST");
  Am_Value_List list = value;
  os << list.Length() << std::endl;
  for (list.Start(); !list.Last(); list.Next())
    context.Save(os, list.Get());
}

Am_Define_Method(Am_Load_Method, Am_Value, load_point_list,
                 (std::istream & is, Am_Load_Save_Context & /*context*/))
{
  int length, i;
  is >> length;
  Am_Point_List list;
  float x, y;
  for (i = 0; i < length; ++i) {
    is >> x;
    is >> y;
    list.Add(x, y, Am_TAIL, false);
  }
  char ch;
  is.get(ch); // skip eoln
  return Am_Value(list);
}

Am_Define_Method(Am_Save_Method, void, save_point_list,
                 (std::ostream & os, Am_Load_Save_Context &context,
                  const Am_Value &value))
{
  context.Save_Type_Name(os, "Am_POINT_LIST");
  Am_Point_List list = value;
  os << list.Length() << std::endl;
  float x, y;
  for (list.Start(); !list.Last(); list.Next()) {
    list.Get(x, y);
    os << x << " " << y << std::endl;
  }
}

Am_Define_Method(Am_Save_Method, void, save_object,
                 (std::ostream & os, Am_Load_Save_Context &context,
                  const Am_Value &value))
{
  Am_Object object = value;
  Am_Save_Object_Method method = object.Get(Am_SAVE_OBJECT_METHOD);
  if (!method.Valid())
    Am_ERRORO(
        "Object "
            << object
            << " does not have a save method in its Am_SAVE_OBJECT_METHOD slot",
        object, Am_SAVE_OBJECT_METHOD);
  method.Call(os, context, object);
}

void
standard_save_internal(std::ostream &os, Am_Load_Save_Context &context,
                       const Am_Object &object, const char *save_type_name)
{
  const char *name;
  //test to see if prototype is registered
  name = context.Is_Registered_Prototype(object);
  Am_Object proto;
  if (name)
    proto = object;
  else {
    proto = object.Get_Prototype();
    name = context.Is_Registered_Prototype(proto);
    if (!name)
      Am_ERROR("Neither object "
               << object << " or its prototype " << proto
               << " have been registered for save for Am_Standard_Save_Object");
  }
  Am_Value_List slots = proto.Get(Am_SLOTS_TO_SAVE);
  context.Save_Type_Name(os, save_type_name);
  context.Save(os, Am_Value(proto));
  Am_Slot_Key slot;
  for (slots.Start(); !slots.Last(); slots.Next()) {
    slot = (Am_Slot_Key)(int)slots.Get();
    context.Save(os, object.Get(slot));
  }
}

Am_Define_Method(Am_Save_Object_Method, void, Am_Standard_Save_Object,
                 (std::ostream & os, Am_Load_Save_Context &context,
                  const Am_Object &object))
{
  standard_save_internal(os, context, object, "Am_STANDARD_SAVE");
}

Am_Define_Method(Am_Save_Object_Method, void, Am_Standard_Save_Group,
                 (std::ostream & os, Am_Load_Save_Context &context,
                  const Am_Object &object))
{
  standard_save_internal(os, context, object, "Am_STANDARD_SAVE_GROUP");
  Am_Value_List parts = object.Get(Am_GRAPHICAL_PARTS);
  os << parts.Length() << std::endl;
  Am_Object part;
  for (parts.Start(); !parts.Last(); parts.Next()) {
    Am_Value v = parts.Get();
    context.Save(os, v);
  }
}

Am_Object Am_Load_Object_Swap;

Am_Define_Method(Am_Load_Method, Am_Value, Am_Standard_Load_Object,
                 (std::istream & is, Am_Load_Save_Context &context))
{
  Am_Object loaded_obj = context.Load(is);
  if (!loaded_obj.Valid())
    return Am_No_Value;
  Am_Object proto;
  Am_Object_Method proto_method;
  if (loaded_obj.Is_Instance_Of(Am_Load_Object_Swap)) {
    proto = loaded_obj.Get(Am_ITEM_PROTOTYPE);
    proto_method = loaded_obj.Get(Am_ITEM_METHOD, Am_RETURN_ZERO_ON_ERROR);
    if (!proto.Valid())
      return Am_No_Value;
  } else
    proto = loaded_obj;
  Am_Object new_obj = proto.Create();
  context.Recursive_Load_Ahead(new_obj);
  Am_Value_List slots = loaded_obj.Get(Am_SLOTS_TO_SAVE);
  Am_Slot_Key slot;
  for (slots.Start(); !slots.Last(); slots.Next()) {
    slot = (Am_Slot_Key)(int)slots.Get();
    new_obj.Set(slot, context.Load(is), Am_OK_IF_NOT_THERE);
  }
  if (proto_method.Valid())
    proto_method.Call(new_obj);
  return Am_Value(new_obj);
}

Am_Define_Method(Am_Load_Method, Am_Value, Am_Standard_Load_Group,
                 (std::istream & is, Am_Load_Save_Context &context))
{
  Am_Value new_obj_val = Am_Standard_Load_Object_proc(is, context);
  if (new_obj_val.Valid()) {
    Am_Object new_obj = new_obj_val;
    int num_parts;
    is >> num_parts;
    Am_Object part;
    int i;
    for (i = 0; i < num_parts; i++) {
      part = context.Load(is);
      new_obj.Add_Part(part);
    }
  }
  return new_obj_val;
}

static void
init()
{
  //
  // Default Load and Save Methods
  Am_Default_Load_Save_Context.Register_Prototype("Am_RESIZE_PARTS_GROUP",
                                                  Am_Resize_Parts_Group);
  Am_Default_Load_Save_Context.Register_Loader("Am_NONE", load_none);
  Am_Default_Load_Save_Context.Register_Saver(Am_NONE, save_none);
  Am_Default_Load_Save_Context.Register_Loader("Am_INT", load_int);
  Am_Default_Load_Save_Context.Register_Saver(Am_INT, save_int);
  Am_Default_Load_Save_Context.Register_Loader("Am_LONG", load_long);
  Am_Default_Load_Save_Context.Register_Saver(Am_LONG, save_long);
  Am_Default_Load_Save_Context.Register_Loader("Am_BOOL", load_bool);
  Am_Default_Load_Save_Context.Register_Saver(Am_BOOL, save_bool);
  Am_Default_Load_Save_Context.Register_Loader("Am_FLOAT", load_float);
  Am_Default_Load_Save_Context.Register_Saver(Am_FLOAT, save_float);
  Am_Default_Load_Save_Context.Register_Loader("Am_DOUBLE", load_double);
  Am_Default_Load_Save_Context.Register_Saver(Am_DOUBLE, save_double);
  Am_Default_Load_Save_Context.Register_Loader("Am_CHAR", load_char);
  Am_Default_Load_Save_Context.Register_Saver(Am_CHAR, save_char);
  Am_Default_Load_Save_Context.Register_Loader("Am_STRING", load_string);
  Am_Default_Load_Save_Context.Register_Saver(Am_STRING, save_string);
  Am_Default_Load_Save_Context.Register_Loader("Am_VALUE_LIST", load_list);
  Am_Default_Load_Save_Context.Register_Saver(Am_Value_List::Type_ID(),
                                              save_list);
  Am_Default_Load_Save_Context.Register_Saver(Am_OBJECT, save_object);
  Am_Default_Load_Save_Context.Register_Loader("Am_STANDARD_SAVE",
                                               Am_Standard_Load_Object);
  Am_Default_Load_Save_Context.Register_Loader("Am_STANDARD_SAVE_GROUP",
                                               Am_Standard_Load_Group);
  Am_Default_Load_Save_Context.Register_Loader("Am_POINT_LIST",
                                               load_point_list);
  Am_Default_Load_Save_Context.Register_Saver(Am_Point_List::Type_ID(),
                                              save_point_list);

  Am_Load_Object_Swap = Am_Root_Object.Create(DSTR("Am_Load_Object_Swap"))
                            .Add(Am_ITEM_PROTOTYPE, Am_No_Object)
                            .Add(Am_SLOTS_TO_SAVE, Am_No_Value_List)
                            .Add(Am_ITEM_METHOD, Am_Object_Method());

  Am_Default_Load_Save_Context.Register_Prototype("Am_Red", Am_Red);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Green", Am_Green);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Blue", Am_Blue);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Yellow", Am_Yellow);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Purple", Am_Purple);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Cyan", Am_Cyan);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Orange", Am_Orange);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Black", Am_Black);
  Am_Default_Load_Save_Context.Register_Prototype("Am_White", Am_White);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Amulet_Purple",
                                                  Am_Amulet_Purple);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Motif_Gray",
                                                  Am_Motif_Gray);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Motif_Light_Gray",
                                                  Am_Motif_Light_Gray);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Motif_Blue",
                                                  Am_Motif_Blue);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Motif_Light_Blue",
                                                  Am_Motif_Light_Blue);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Motif_Green",
                                                  Am_Motif_Green);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Motif_Light_Green",
                                                  Am_Motif_Light_Green);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Motif_Orange",
                                                  Am_Motif_Orange);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Motif_Light_Orange",
                                                  Am_Motif_Light_Orange);
  // these all conflict with Am_Black
  //Am_Default_Load_Save_Context.Register_Prototype ("Am_Thin_Line",
  //						   Am_Thin_Line);
  //Am_Default_Load_Save_Context.Register_Prototype ("Am_Line_0", Am_Line_0);
  //Am_Default_Load_Save_Context.Register_Prototype ("Am_Line_1", Am_Line_1);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Line_2", Am_Line_2);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Line_4", Am_Line_4);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Line_8", Am_Line_8);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Dotted_Line",
                                                  Am_Dotted_Line);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Dashed_Line",
                                                  Am_Dashed_Line);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Gray_Stipple",
                                                  Am_Gray_Stipple);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Opaque_Gray_Stipple",
                                                  Am_Opaque_Gray_Stipple);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Light_Gray_Stipple",
                                                  Am_Light_Gray_Stipple);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Dark_Gray_Stipple",
                                                  Am_Dark_Gray_Stipple);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Diamond_Stipple",
                                                  Am_Diamond_Stipple);
  Am_Default_Load_Save_Context.Register_Prototype("Am_Opaque_Diamond_Stipple",
                                                  Am_Opaque_Diamond_Stipple);
}

static Am_Initializer *initializer =
    new Am_Initializer(DSTR("Load_Save"), init, 2.01f);
#endif
