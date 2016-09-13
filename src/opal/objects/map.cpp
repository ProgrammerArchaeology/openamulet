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

#include <amulet/initializer.h>
#include <amulet/value_list.h>
#include <amulet/formula.h>
#include <amulet/impl/opal_impl.h>
#include <amulet/impl/opal_misc.h>
#include <amulet/impl/method_item.h>
#include <amulet/impl/opal_constraints.h>

#include <iostream>

extern Am_Object *Am_Map_ptr;

Am_Object Am_Map;

Am_Define_Value_List_Formula(map_make_components)
{
  self.Make_Unique(Am_GRAPHICAL_PARTS);

  Am_Value value = self.Peek(Am_GRAPHICAL_PARTS, Am_NO_DEPENDENCY);
  Am_Value_List components;

  if (value.Exists()) {
    components = value;
  }

  int i;
  Am_Object item_instance;
  Am_Object new_item;
  Am_Value stored_value = 0;
  bool was_empty = components.Empty();
  Am_Item_Method item_method = self.Get(Am_ITEM_METHOD);
  Am_Object item_prototype =
      self.Get(Am_ITEM_PROTOTYPE, Am_RETURN_ZERO_ON_ERROR);

  value = self.Get(Am_ITEMS);
  components.Start();

  if (value.type == Am_INT) {
    int number = value;

    for (i = 0; i < number; ++i) {
      if (components.Last()) {
        if (item_prototype.Valid())
          item_instance = item_prototype.Create();
        else
          item_instance = Am_No_Object;

        if (item_instance.Valid())
          self.Add_Part(item_instance, false);

        new_item = item_method.Call(i, stored_value, item_instance);

        if (new_item != item_instance) {
          item_instance.Destroy();

          if (new_item.Valid())
            self.Add_Part(new_item, false);
        }

        if (new_item.Valid())
          components.Add(new_item, Am_TAIL, false);
      } else {
        item_instance = components.Get();
        if (item_instance.Get_Owner(Am_NO_DEPENDENCY) != self) {
          if (item_prototype.Valid())
            item_instance = item_prototype.Create();
          else
            item_instance = Am_No_Object;
        }

        new_item = item_method.Call(i, stored_value, item_instance);

        if (new_item != item_instance)
          item_instance.Destroy();

        if (new_item.Valid()) {
          components.Set(new_item, false);

          if (!new_item.Get_Owner(Am_NO_DEPENDENCY))
            self.Add_Part(new_item, false);
        } else
          components.Delete(false);

        components.Next();
      }
    }
  } else if (value.type == Am_VALUE_LIST) {
    Am_Value_List item_list;
    item_list = value;

    if (!item_list.Empty()) {
      i = 0;
      for (item_list.Start(); !item_list.Last(); item_list.Next()) {
        stored_value = item_list.Get();
        if (components.Last()) {
          if (item_prototype.Valid())
            item_instance = item_prototype.Create();
          else
            item_instance = Am_No_Object;

          if (item_instance.Valid())
            self.Add_Part(item_instance, false);

          new_item = item_method.Call(i, stored_value, item_instance);

          if (new_item != item_instance) {
            item_instance.Destroy();

            if (new_item.Valid())
              self.Add_Part(new_item, false);
          }

          if (new_item.Valid())
            components.Add(new_item, Am_TAIL, false);
        } else {
          item_instance = components.Get();

          if (item_instance.Get_Owner(Am_NO_DEPENDENCY) != self) {
            if (item_prototype.Valid())
              item_instance = item_prototype.Create();
            else
              item_instance = Am_No_Object;
          }

          new_item = item_method.Call(i, stored_value, item_instance);

          if (new_item != item_instance)
            item_instance.Destroy();

          if (new_item.Valid()) {
            components.Set(new_item, false);

            if (!new_item.Get_Owner(Am_NO_DEPENDENCY))
              self.Add_Part(new_item, false);
          } else
            components.Delete(false);

          components.Next();
        }
        ++i;
      }
    }
  } else if (value.Valid()) { // any type of 0 is valid: no items.
    std::cerr << "** Value of Am_ITEMS slot in object, " << self
              << ", is not an integer or Am_Value_List." << std::endl;
    Am_Error();
  }

  while (!components.Last()) {
    item_instance = components.Get();

    if (item_instance.Get_Owner(Am_NO_DEPENDENCY) == self)
      item_instance.Destroy();

    components.Delete(false);
    components.Next();
  }

  if (!was_empty && !components.Empty())
    self.Note_Changed(Am_GRAPHICAL_PARTS);

  if (components.Empty())
    return Am_Value_List();
  else
    return components;
}

Am_Define_Method(Am_Item_Method, Am_Object, Am_Standard_Item_Method,
                 (int rank, Am_Value &value, Am_Object item_instance))
{
  if (item_instance.Valid()) {
    item_instance.Set(Am_RANK, rank, Am_OK_IF_NOT_THERE);
    item_instance.Set(Am_ITEM, value, Am_OK_IF_NOT_THERE);
  }
  return item_instance;
}

static void
init()
{
  Am_Map = Am_Aggregate.Create(DSTR("Am_Map"))
               .Set(Am_WIDTH, Am_Width_Of_Parts)
               .Set(Am_HEIGHT, Am_Height_Of_Parts)
               .Set(Am_GRAPHICAL_PARTS, map_make_components)
               .Add(Am_ITEMS, 0)
               .Add(Am_ITEM_METHOD, Am_Standard_Item_Method)
               .Add(Am_ITEM_PROTOTYPE, 0);
  Am_Object_Advanced temp = (Am_Object_Advanced &)Am_Map;
  temp.Get_Slot(Am_GRAPHICAL_PARTS)
      .Set_Demon_Bits(Am_MOVING_REDRAW | Am_EAGER_DEMON);

  //
  // Let the rest of the world know that this application is using Am_Maps.
  //
  Am_Map_ptr = &Am_Map;
}

static void
cleanup()
{
  Am_Map.Destroy();
}

static Am_Initializer *initializer =
    new Am_Initializer(DSTR("Am_Map"), init, 2.1f, 112, cleanup);
