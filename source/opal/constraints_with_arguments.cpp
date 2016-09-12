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
#include <amulet/impl/am_object_advanced.h>
#include <amulet/impl/am_constraint_context.h>
#include <amulet/impl/slots.h>
#include FORMULA__H
#include <amulet/impl/opal_constraints.h>

#if 1
/***************************************************
 ** Standard exported constraints, with arguments **
 ***************************************************/

/*
 *  Constraints with arguments are really formula generators
 */

class Key_Store_Data : public Am_Wrapper
{
  AM_WRAPPER_DATA_DECL(Key_Store)
public:
  Key_Store_Data(Am_Slot_Key in_key) : key(in_key) {}
  Key_Store_Data(Key_Store_Data *proto) : key(proto->key) {}
  bool operator==(Key_Store_Data &test) { return key == test.key; }
  bool operator==(Key_Store_Data &test) const { return key == test.key; }

  Am_Slot_Key key;
};

AM_WRAPPER_DATA_IMPL(Key_Store, (this))

static Am_Value
same_as_procedure(Am_Object &self)
{
  Am_Value value;
  Key_Store_Data *store =
      (Key_Store_Data *)Am_Object_Advanced::Get_Context()->Get_Data();
  value = self.Get(store->key);
  store->Release();
  return value;
}

Am_Formula
Am_Same_As(Am_Slot_Key key)
{
  Am_Formula formula(same_as_procedure, DSTR("Am_Same_As"));
  formula.Set_Data(new Key_Store_Data(key));
  return formula;
}

static Am_Value
from_owner_procedure(Am_Object &self)
{
  Am_Value value;
  Am_Object owner = self.Get_Owner();
  if (owner.Valid()) {
    Key_Store_Data *store =
        (Key_Store_Data *)Am_Object_Advanced::Get_Context()->Get_Data();
    value = owner.Get(store->key);
    store->Release();
  } else
    value.Set_Value_Type(Am_ZERO); //not there, return a ZERO type
  return value;
}

Am_Formula
Am_From_Owner(Am_Slot_Key key)
{
  Am_Formula formula(from_owner_procedure, DSTR("Am_From_Owner"));
  formula.Set_Data(new Key_Store_Data(key));
  return formula;
}

class Part_Key_Store_Data : public Am_Wrapper
{
  AM_WRAPPER_DATA_DECL(Part_Key_Store)
public:
  Part_Key_Store_Data(Am_Slot_Key in_part, Am_Slot_Key in_key)
      : part(in_part), key(in_key)
  {
  }
  Part_Key_Store_Data(Part_Key_Store_Data *proto)
      : part(proto->part), key(proto->key)
  {
  }
  bool operator==(Part_Key_Store_Data &test)
  {
    return key == test.key && part == test.part;
  }
  bool operator==(Part_Key_Store_Data &test) const
  {
    return key == test.key && part == test.part;
  }

  Am_Slot_Key part;
  Am_Slot_Key key;
};

AM_WRAPPER_DATA_IMPL(Part_Key_Store, (this))

static Am_Value
from_part_procedure(Am_Object &self)
{
  Am_Value value;
  Part_Key_Store_Data *store =
      (Part_Key_Store_Data *)Am_Object_Advanced::Get_Context()->Get_Data();
  value = self.Get_Object(store->part).Get(store->key);
  store->Release();
  return value;
}

Am_Formula
Am_From_Part(Am_Slot_Key part, Am_Slot_Key key)
{
  Am_Formula formula(from_part_procedure, DSTR("Am_From_Part"));
  formula.Set_Data(new Part_Key_Store_Data(part, key));
  return formula;
}

static Am_Value
from_sibling_procedure(Am_Object &self)
{
  Am_Value value;
  Part_Key_Store_Data *store =
      (Part_Key_Store_Data *)Am_Object_Advanced::Get_Context()->Get_Data();
  value = self.Get_Owner().Get_Object(store->part).Get(store->key);
  store->Release();
  return value;
}

Am_Formula
Am_From_Sibling(Am_Slot_Key part, Am_Slot_Key key)
{
  Am_Formula formula(from_sibling_procedure, DSTR("Am_From_Sibling"));
  formula.Set_Data(new Part_Key_Store_Data(part, key));
  return formula;
}

class Object_Key_Store_Data : public Am_Wrapper
{
  AM_WRAPPER_DATA_DECL(Object_Key_Store)
public:
  Object_Key_Store_Data(Am_Object in_object, Am_Slot_Key in_key)
      : object(in_object), key(in_key)
  {
  }
  Object_Key_Store_Data(Object_Key_Store_Data *proto)
      : object(proto->object), key(proto->key)
  {
  }
  bool operator==(Object_Key_Store_Data &test)
  {
    return key == test.key && object == test.object;
  }
  bool operator==(Object_Key_Store_Data &test) const
  {
    return key == test.key && object == test.object;
  }

  Am_Object object;
  Am_Slot_Key key;
};

AM_WRAPPER_DATA_IMPL(Object_Key_Store, (this))

static Am_Value
from_object_procedure(Am_Object & /*self*/)
{
  Am_Value value;
  Object_Key_Store_Data *store =
      (Object_Key_Store_Data *)Am_Object_Advanced::Get_Context()->Get_Data();
  value = (store->object).Get(store->key);
  store->Release();
  return value;
}

Am_Formula
Am_From_Object(Am_Object object, Am_Slot_Key key)
{
  Am_Formula formula(from_object_procedure, DSTR("Am_From_Object"));
  formula.Set_Data(new Object_Key_Store_Data(object, key));
  return formula;
}

class Key_Offset_Store_Data : public Am_Wrapper
{
  AM_WRAPPER_DATA_DECL(Key_Offset_Store)
public:
  Key_Offset_Store_Data(Am_Slot_Key in_key, int in_offset, float in_multiplier)
      : key(in_key), offset(in_offset), multiplier(in_multiplier)
  {
  }
  Key_Offset_Store_Data(Key_Offset_Store_Data *proto)
      : key(proto->key), offset(proto->offset), multiplier(proto->multiplier)
  {
  }
  bool operator==(Key_Offset_Store_Data &test)
  {
    return key == test.key && offset == test.offset &&
           multiplier == test.multiplier;
  }

  bool operator==(Key_Offset_Store_Data &test) const
  {
    return key == test.key && offset == test.offset &&
           multiplier == test.multiplier;
  }

  Am_Slot_Key key;
  int offset;
  float multiplier;
};

AM_WRAPPER_DATA_IMPL(Key_Offset_Store, (this))

inline void
modify_value(Am_Value &value, int offset, float multiplier)
{
  switch (value.type) {
  case Am_INT:
  case Am_LONG:
    value.value.long_value =
        (long)(value.value.long_value * multiplier) + offset;
    break;
  case Am_FLOAT:
    value.value.float_value = (value.value.float_value * multiplier) + offset;
    break;
  case Am_DOUBLE: {
    double d_value = value;
    d_value = (d_value * multiplier) + offset;
    value = d_value;
    break;
  }
  default:
    break;
  }
}

static Am_Value
offset_same_as_procedure(Am_Object &self)
{
  Am_Value value;
  Key_Offset_Store_Data *store =
      (Key_Offset_Store_Data *)Am_Object_Advanced::Get_Context()->Get_Data();
  value = self.Get(store->key);
  store->Release();
  modify_value(value, store->offset, store->multiplier);
  return value;
}

Am_Formula
Am_Same_As(Am_Slot_Key key, int offset, float multiplier)
{
  Am_Formula formula(offset_same_as_procedure, DSTR("Am_Same_As"));
  formula.Set_Data(new Key_Offset_Store_Data(key, offset, multiplier));
  return formula;
}

static Am_Value
offset_from_owner_procedure(Am_Object &self)
{
  Am_Value value;
  Am_Object owner = self.Get_Owner();
  if (owner.Valid()) {
    Key_Offset_Store_Data *store =
        (Key_Offset_Store_Data *)Am_Object_Advanced::Get_Context()->Get_Data();
    value = owner.Get(store->key);
    store->Release();
    modify_value(value, store->offset, store->multiplier);
  } else
    value.Set_Value_Type(Am_ZERO); //not there, return a ZERO type
  return value;
}

Am_Formula
Am_From_Owner(Am_Slot_Key key, int offset, float multiplier)
{
  Am_Formula formula(offset_from_owner_procedure, DSTR("Am_From_Owner"));
  formula.Set_Data(new Key_Offset_Store_Data(key, offset, multiplier));
  return formula;
}

class Part_Key_Offset_Store_Data : public Am_Wrapper
{
  AM_WRAPPER_DATA_DECL(Part_Key_Offset_Store)
public:
  Part_Key_Offset_Store_Data(Am_Slot_Key in_part, Am_Slot_Key in_key,
                             int in_offset, float in_multiplier)
      : part(in_part), key(in_key), offset(in_offset), multiplier(in_multiplier)
  {
  }
  Part_Key_Offset_Store_Data(Part_Key_Offset_Store_Data *proto)
      : part(proto->part), key(proto->key), offset(proto->offset),
        multiplier(proto->multiplier)
  {
  }
  bool operator==(Part_Key_Offset_Store_Data &test)
  {
    return key == test.key && part == test.part && offset == test.offset &&
           multiplier == test.multiplier;
  }
  bool operator==(Part_Key_Offset_Store_Data &test) const
  {
    return key == test.key && part == test.part && offset == test.offset &&
           multiplier == test.multiplier;
  }

  Am_Slot_Key part;
  Am_Slot_Key key;
  int offset;
  float multiplier;
};

AM_WRAPPER_DATA_IMPL(Part_Key_Offset_Store, (this))

static Am_Value
offset_from_part_procedure(Am_Object &self)
{
  Am_Value value;
  Part_Key_Offset_Store_Data *store =
      (Part_Key_Offset_Store_Data *)Am_Object_Advanced::Get_Context()
          ->Get_Data();
  value = self.Get_Object(store->part).Get(store->key);
  store->Release();
  modify_value(value, store->offset, store->multiplier);
  return value;
}

Am_Formula
Am_From_Part(Am_Slot_Key part, Am_Slot_Key key, int offset, float multiplier)
{
  Am_Formula formula(offset_from_part_procedure, DSTR("Am_From_Part"));
  formula.Set_Data(
      new Part_Key_Offset_Store_Data(part, key, offset, multiplier));
  return formula;
}

static Am_Value
offset_from_sibling_procedure(Am_Object &self)
{
  Am_Value value;
  Part_Key_Offset_Store_Data *store =
      (Part_Key_Offset_Store_Data *)Am_Object_Advanced::Get_Context()
          ->Get_Data();
  value = self.Get_Owner().Get_Object(store->part).Get(store->key);
  store->Release();
  modify_value(value, store->offset, store->multiplier);
  return value;
}

Am_Formula
Am_From_Sibling(Am_Slot_Key sibling, Am_Slot_Key key, int offset,
                float multiplier)
{
  Am_Formula formula(offset_from_sibling_procedure, DSTR("Am_From_Sibling"));
  formula.Set_Data(
      new Part_Key_Offset_Store_Data(sibling, key, offset, multiplier));
  return formula;
}

static Am_Value
bottom_of_sibling_procedure(Am_Object &self)
{
  Am_Value value;
  Part_Key_Offset_Store_Data *store =
      (Part_Key_Offset_Store_Data *)Am_Object_Advanced::Get_Context()
          ->Get_Data();
  Am_Object object = self.Get_Owner().Get_Object(store->part);
  value = (int)object.Get(Am_TOP) + (int)object.Get(Am_HEIGHT);
  store->Release();
  modify_value(value, store->offset, store->multiplier);
  return value;
}

Am_Formula
Am_Bottom_Of_Sibling(Am_Slot_Key sibling, int offset, float multiplier)
{
  Am_Formula formula(bottom_of_sibling_procedure, DSTR("Am_Bottom_Of_Sibling"));
  formula.Set_Data(
      new Part_Key_Offset_Store_Data(sibling, 0, offset, multiplier));
  return formula;
}

static Am_Value
right_of_sibling_procedure(Am_Object &self)
{
  Am_Value value;
  Part_Key_Offset_Store_Data *store =
      (Part_Key_Offset_Store_Data *)Am_Object_Advanced::Get_Context()
          ->Get_Data();
  Am_Object object = self.Get_Owner().Get_Object(store->part);
  value = (int)object.Get(Am_LEFT) + (int)object.Get(Am_WIDTH);
  store->Release();
  modify_value(value, store->offset, store->multiplier);
  return value;
}

Am_Formula
Am_Right_Of_Sibling(Am_Slot_Key sibling, int offset, float multiplier)
{
  Am_Formula formula(right_of_sibling_procedure, DSTR("Am_Right_Of_Sibling"));
  formula.Set_Data(
      new Part_Key_Offset_Store_Data(sibling, 0, offset, multiplier));
  return formula;
}

static Am_Value
rest_of_height_above_procedure(Am_Object &self)
{
  Am_Value value;
  Part_Key_Offset_Store_Data *store =
      (Part_Key_Offset_Store_Data *)Am_Object_Advanced::Get_Context()
          ->Get_Data();
  Am_Object object = self.Get_Owner().Get_Object(store->part);
  value = (int)object.Get(Am_TOP) - (int)self.Get(Am_TOP) - store->offset;
  store->Release();
  return value;
}

//this object extends from its top to offset above the sibling
Am_Formula
Am_Rest_Of_Height_Above(Am_Slot_Key sibling, int offset)
{
  Am_Formula formula(rest_of_height_above_procedure,
                     DSTR("Am_Rest_Of_Height_Above"));
  formula.Set_Data(new Part_Key_Offset_Store_Data(sibling, 0, offset, 0.0));
  return formula;
}

class Object_Key_Offset_Store_Data : public Am_Wrapper
{
  AM_WRAPPER_DATA_DECL(Object_Key_Offset_Store)
public:
  Object_Key_Offset_Store_Data(Am_Object in_object, Am_Slot_Key in_key,
                               int in_offset, float in_multiplier)
      : object(in_object), key(in_key), offset(in_offset),
        multiplier(in_multiplier)
  {
  }
  Object_Key_Offset_Store_Data(Object_Key_Offset_Store_Data *proto)
      : object(proto->object), key(proto->key), offset(proto->offset),
        multiplier(proto->multiplier)
  {
  }
  bool operator==(Object_Key_Offset_Store_Data &test)
  {
    return key == test.key && object == test.object && offset == test.offset &&
           multiplier == test.multiplier;
  }
  bool operator==(Object_Key_Offset_Store_Data &test) const
  {
    return key == test.key && object == test.object && offset == test.offset &&
           multiplier == test.multiplier;
  }

  Am_Object object;
  Am_Slot_Key key;
  int offset;
  float multiplier;
};

AM_WRAPPER_DATA_IMPL(Object_Key_Offset_Store, (this))

static Am_Value
offset_from_object_procedure(Am_Object & /*self*/)
{
  Am_Value value;
  Object_Key_Offset_Store_Data *store =
      (Object_Key_Offset_Store_Data *)Am_Object_Advanced::Get_Context()
          ->Get_Data();
  value = (store->object).Get(store->key);
  store->Release();
  modify_value(value, store->offset, store->multiplier);
  return value;
}

Am_Formula
Am_From_Object(Am_Object object, Am_Slot_Key key, int offset, float multiplier)
{
  Am_Formula formula(offset_from_object_procedure, DSTR("Am_From_Object"));
  formula.Set_Data(
      new Object_Key_Offset_Store_Data(object, key, offset, multiplier));
  return formula;
}
#else
/***************************************************
 ** Standard exported constraints, with arguments **
 ***************************************************/

/*
 *  Constraints with arguments are really formula generators
 */

class Key_Store_Data : public Am_Wrapper
{
  AM_WRAPPER_DATA_DECL(Key_Store)
public:
  Key_Store_Data(Am_Slot_Key in_key) { key = in_key; }
  Key_Store_Data(Key_Store_Data *proto) { key = proto->key; }
  bool operator==(Key_Store_Data &test) { return key == test.key; }

  bool operator==(Key_Store_Data &test) const { return key == test.key; }

  Am_Slot_Key key;
};

AM_WRAPPER_DATA_IMPL(Key_Store, (this))

static Am_Value
same_as_procedure(Am_Object &self)
{
  Am_Value value;
  Key_Store_Data *store =
      (Key_Store_Data *)Am_Object_Advanced::Get_Context()->Get_Data();
  value = self.Get(store->key);
  store->Release();
  return value;
}

Am_Formula
Am_Same_As(Am_Slot_Key key)
{
  Am_Formula formula(same_as_procedure, DSTR("Am_Same_As"));
  formula.Set_Data(new Key_Store_Data(key));
  return formula;
}

static Am_Value
from_owner_procedure(Am_Object &self)
{
  Am_Value value;
  Am_Object owner = self.Get_Owner();
  if (owner.Valid()) {
    Key_Store_Data *store =
        (Key_Store_Data *)Am_Object_Advanced::Get_Context()->Get_Data();
    value = owner.Get(store->key);
    store->Release();
  } else
    value.Set_Value_Type(Am_ZERO); //not there, return a ZERO type
  return value;
}

Am_Formula
Am_From_Owner(Am_Slot_Key key)
{
  Am_Formula formula(from_owner_procedure, DSTR("Am_From_Owner"));
  formula.Set_Data(new Key_Store_Data(key));
  return formula;
}

class Part_Key_Store_Data : public Am_Wrapper
{
  AM_WRAPPER_DATA_DECL(Part_Key_Store)
public:
  Part_Key_Store_Data(Am_Slot_Key in_part, Am_Slot_Key in_key)
  {
    part = in_part;
    key = in_key;
  }
  Part_Key_Store_Data(Part_Key_Store_Data *proto)
  {
    part = proto->part;
    key = proto->key;
  }
  bool operator==(Part_Key_Store_Data &test)
  {
    return key == test.key && part == test.part;
  }

  bool operator==(Part_Key_Store_Data &test) const
  {
    return key == test.key && part == test.part;
  }

  Am_Slot_Key part;
  Am_Slot_Key key;
};

AM_WRAPPER_DATA_IMPL(Part_Key_Store, (this))

static Am_Value
from_part_procedure(Am_Object &self)
{
  Am_Value value;
  Part_Key_Store_Data *store =
      (Part_Key_Store_Data *)Am_Object_Advanced::Get_Context()->Get_Data();
  value = self.Get_Object(store->part).Get(store->key);
  store->Release();
  return value;
}

Am_Formula
Am_From_Part(Am_Slot_Key part, Am_Slot_Key key)
{
  Am_Formula formula(from_part_procedure, DSTR("Am_From_Part"));
  formula.Set_Data(new Part_Key_Store_Data(part, key));
  return formula;
}

static Am_Value
from_sibling_procedure(Am_Object &self)
{
  Am_Value value;
  Part_Key_Store_Data *store =
      (Part_Key_Store_Data *)Am_Object_Advanced::Get_Context()->Get_Data();
  value = self.Get_Owner().Get_Object(store->part).Get(store->key);
  store->Release();
  return value;
}

Am_Formula
Am_From_Sibling(Am_Slot_Key part, Am_Slot_Key key)
{
  Am_Formula formula(from_sibling_procedure, DSTR("Am_From_Sibling"));
  formula.Set_Data(new Part_Key_Store_Data(part, key));
  return formula;
}

class Object_Key_Store_Data : public Am_Wrapper
{
  AM_WRAPPER_DATA_DECL(Object_Key_Store)
public:
  Object_Key_Store_Data(Am_Object in_object, Am_Slot_Key in_key)
  {
    object = in_object;
    key = in_key;
  }
  Object_Key_Store_Data(Object_Key_Store_Data *proto)
  {
    object = proto->object;
    key = proto->key;
  }
  bool operator==(Object_Key_Store_Data &test)
  {
    return key == test.key && object == test.object;
  }

  bool operator==(Object_Key_Store_Data &test) const
  {
    return key == test.key && object == test.object;
  }

  Am_Object object;
  Am_Slot_Key key;
};

AM_WRAPPER_DATA_IMPL(Object_Key_Store, (this))

static Am_Value
from_object_procedure(Am_Object & /*self*/)
{
  Am_Value value;
  Object_Key_Store_Data *store =
      (Object_Key_Store_Data *)Am_Object_Advanced::Get_Context()->Get_Data();
  value = (store->object).Get(store->key);
  store->Release();
  return value;
}

Am_Formula
Am_From_Object(Am_Object object, Am_Slot_Key key)
{
  Am_Formula formula(from_object_procedure, DSTR("Am_From_Object"));
  formula.Set_Data(new Object_Key_Store_Data(object, key));
  return formula;
}

class Key_Offset_Store_Data : public Am_Wrapper
{
  AM_WRAPPER_DATA_DECL(Key_Offset_Store)
public:
  Key_Offset_Store_Data(Am_Slot_Key in_key, int in_offset, float in_multiplier)
  {
    key = in_key;
    offset = in_offset;
    multiplier = in_multiplier;
  }
  Key_Offset_Store_Data(Key_Offset_Store_Data *proto)
  {
    key = proto->key;
    offset = proto->offset;
    multiplier = proto->multiplier;
  }
  bool operator==(Key_Offset_Store_Data &test)
  {
    return key == test.key && offset == test.offset &&
           multiplier == test.multiplier;
  }

  bool operator==(Key_Offset_Store_Data &test) const
  {
    return key == test.key && offset == test.offset &&
           multiplier == test.multiplier;
  }

  Am_Slot_Key key;
  int offset;
  float multiplier;
};

AM_WRAPPER_DATA_IMPL(Key_Offset_Store, (this))

inline void
modify_value(Am_Value &value, int offset, float multiplier)
{
  switch (value.type) {
  case Am_INT:
  case Am_LONG:
    value.value.long_value =
        (long)(value.value.long_value * multiplier) + offset;
    break;
  case Am_FLOAT:
    value.value.float_value = (value.value.float_value * multiplier) + offset;
    break;
  case Am_DOUBLE: {
    double d_value = value;
    d_value = (d_value * multiplier) + offset;
    value = d_value;
    break;
  }
  default:
    break;
  }
}

static Am_Value
offset_same_as_procedure(Am_Object &self)
{
  Am_Value value;
  Key_Offset_Store_Data *store =
      (Key_Offset_Store_Data *)Am_Object_Advanced::Get_Context()->Get_Data();
  value = self.Get(store->key);
  store->Release();
  modify_value(value, store->offset, store->multiplier);
  return value;
}

Am_Formula
Am_Same_As(Am_Slot_Key key, int offset, float multiplier)
{
  Am_Formula formula(offset_same_as_procedure, DSTR("Am_Same_As"));
  formula.Set_Data(new Key_Offset_Store_Data(key, offset, multiplier));
  return formula;
}

static Am_Value
offset_from_owner_procedure(Am_Object &self)
{
  Am_Value value;
  Am_Object owner = self.Get_Owner();
  if (owner.Valid()) {
    Key_Offset_Store_Data *store =
        (Key_Offset_Store_Data *)Am_Object_Advanced::Get_Context()->Get_Data();
    value = owner.Get(store->key);
    store->Release();
    modify_value(value, store->offset, store->multiplier);
  } else
    value.Set_Value_Type(Am_ZERO); //not there, return a ZERO type
  return value;
}

Am_Formula
Am_From_Owner(Am_Slot_Key key, int offset, float multiplier)
{
  Am_Formula formula(offset_from_owner_procedure, DSTR("Am_From_Owner"));
  formula.Set_Data(new Key_Offset_Store_Data(key, offset, multiplier));
  return formula;
}

class Part_Key_Offset_Store_Data : public Am_Wrapper
{
  AM_WRAPPER_DATA_DECL(Part_Key_Offset_Store)
public:
  Part_Key_Offset_Store_Data(Am_Slot_Key in_part, Am_Slot_Key in_key,
                             int in_offset, float in_multiplier)
  {
    part = in_part;
    key = in_key;
    offset = in_offset;
    multiplier = in_multiplier;
  }
  Part_Key_Offset_Store_Data(Part_Key_Offset_Store_Data *proto)
  {
    part = proto->part;
    key = proto->key;
    offset = proto->offset;
    multiplier = proto->multiplier;
  }
  bool operator==(Part_Key_Offset_Store_Data &test)
  {
    return key == test.key && part == test.part && offset == test.offset &&
           multiplier == test.multiplier;
  }

  bool operator==(Part_Key_Offset_Store_Data &test) const
  {
    return key == test.key && part == test.part && offset == test.offset &&
           multiplier == test.multiplier;
  }

  Am_Slot_Key part;
  Am_Slot_Key key;
  int offset;
  float multiplier;
};

AM_WRAPPER_DATA_IMPL(Part_Key_Offset_Store, (this))

static Am_Value
offset_from_part_procedure(Am_Object &self)
{
  Am_Value value;
  Part_Key_Offset_Store_Data *store =
      (Part_Key_Offset_Store_Data *)Am_Object_Advanced::Get_Context()
          ->Get_Data();
  value = self.Get_Object(store->part).Get(store->key);
  store->Release();
  modify_value(value, store->offset, store->multiplier);
  return value;
}

Am_Formula
Am_From_Part(Am_Slot_Key part, Am_Slot_Key key, int offset, float multiplier)
{
  Am_Formula formula(offset_from_part_procedure, DSTR("Am_From_Part"));
  formula.Set_Data(
      new Part_Key_Offset_Store_Data(part, key, offset, multiplier));
  return formula;
}

static Am_Value
offset_from_sibling_procedure(Am_Object &self)
{
  Am_Value value;
  Part_Key_Offset_Store_Data *store =
      (Part_Key_Offset_Store_Data *)Am_Object_Advanced::Get_Context()
          ->Get_Data();
  value = self.Get_Owner().Get_Object(store->part).Get(store->key);
  store->Release();
  modify_value(value, store->offset, store->multiplier);
  return value;
}

Am_Formula
Am_From_Sibling(Am_Slot_Key sibling, Am_Slot_Key key, int offset,
                float multiplier)
{
  Am_Formula formula(offset_from_sibling_procedure, DSTR("Am_From_Sibling"));
  formula.Set_Data(
      new Part_Key_Offset_Store_Data(sibling, key, offset, multiplier));
  return formula;
}

static Am_Value
bottom_of_sibling_procedure(Am_Object &self)
{
  Am_Value value;
  Part_Key_Offset_Store_Data *store =
      (Part_Key_Offset_Store_Data *)Am_Object_Advanced::Get_Context()
          ->Get_Data();
  Am_Object object = self.Get_Owner().Get_Object(store->part);
  value = (int)object.Get(Am_TOP) + (int)object.Get(Am_HEIGHT);
  store->Release();
  modify_value(value, store->offset, store->multiplier);
  return value;
}

Am_Formula
Am_Bottom_Of_Sibling(Am_Slot_Key sibling, int offset, float multiplier)
{
  Am_Formula formula(bottom_of_sibling_procedure, DSTR("Am_Bottom_Of_Sibling"));
  formula.Set_Data(
      new Part_Key_Offset_Store_Data(sibling, 0, offset, multiplier));
  return formula;
}

static Am_Value
right_of_sibling_procedure(Am_Object &self)
{
  Am_Value value;
  Part_Key_Offset_Store_Data *store =
      (Part_Key_Offset_Store_Data *)Am_Object_Advanced::Get_Context()
          ->Get_Data();
  Am_Object object = self.Get_Owner().Get_Object(store->part);
  value = (int)object.Get(Am_LEFT) + (int)object.Get(Am_WIDTH);
  store->Release();
  modify_value(value, store->offset, store->multiplier);
  return value;
}

Am_Formula
Am_Right_Of_Sibling(Am_Slot_Key sibling, int offset, float multiplier)
{
  Am_Formula formula(right_of_sibling_procedure, DSTR("Am_Right_Of_Sibling"));
  formula.Set_Data(
      new Part_Key_Offset_Store_Data(sibling, 0, offset, multiplier));
  return formula;
}

static Am_Value
rest_of_height_above_procedure(Am_Object &self)
{
  Am_Value value;
  Part_Key_Offset_Store_Data *store =
      (Part_Key_Offset_Store_Data *)Am_Object_Advanced::Get_Context()
          ->Get_Data();
  Am_Object object = self.Get_Owner().Get_Object(store->part);
  value = (int)object.Get(Am_TOP) - (int)self.Get(Am_TOP) - store->offset;
  store->Release();
  return value;
}

//this object extends from its top to offset above the sibling
Am_Formula
Am_Rest_Of_Height_Above(Am_Slot_Key sibling, int offset)
{
  Am_Formula formula(rest_of_height_above_procedure,
                     DSTR("Am_Rest_Of_Height_Above"));
  formula.Set_Data(new Part_Key_Offset_Store_Data(sibling, 0, offset, 0.0));
  return formula;
}

class Object_Key_Offset_Store_Data : public Am_Wrapper
{
  AM_WRAPPER_DATA_DECL(Object_Key_Offset_Store)
public:
  Object_Key_Offset_Store_Data(Am_Object in_object, Am_Slot_Key in_key,
                               int in_offset, float in_multiplier)
  {
    object = in_object;
    key = in_key;
    offset = in_offset;
    multiplier = in_multiplier;
  }
  Object_Key_Offset_Store_Data(Object_Key_Offset_Store_Data *proto)
  {
    object = proto->object;
    key = proto->key;
    offset = proto->offset;
    multiplier = proto->multiplier;
  }
  bool operator==(Object_Key_Offset_Store_Data &test)
  {
    return key == test.key && object == test.object && offset == test.offset &&
           multiplier == test.multiplier;
  }

  bool operator==(Object_Key_Offset_Store_Data &test) const
  {
    return key == test.key && object == test.object && offset == test.offset &&
           multiplier == test.multiplier;
  }

  Am_Object object;
  Am_Slot_Key key;
  int offset;
  float multiplier;
};

AM_WRAPPER_DATA_IMPL(Object_Key_Offset_Store, (this))

static Am_Value
offset_from_object_procedure(Am_Object & /*self*/)
{
  Am_Value value;
  Object_Key_Offset_Store_Data *store =
      (Object_Key_Offset_Store_Data *)Am_Object_Advanced::Get_Context()
          ->Get_Data();
  value = (store->object).Get(store->key);
  store->Release();
  modify_value(value, store->offset, store->multiplier);
  return value;
}

Am_Formula
Am_From_Object(Am_Object object, Am_Slot_Key key, int offset, float multiplier)
{
  Am_Formula formula(offset_from_object_procedure, DSTR("Am_From_Object"));
  formula.Set_Data(
      new Object_Key_Offset_Store_Data(object, key, offset, multiplier));
  return formula;
}
#endif
