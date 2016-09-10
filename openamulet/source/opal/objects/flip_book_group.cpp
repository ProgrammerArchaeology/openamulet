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

#include INITIALIZER__H
#include VALUE_LIST__H
#include <amulet/impl/opal_impl.h>
#include <amulet/impl/opal_misc.h>
#include <amulet/impl/am_demon_set.h>
#include <amulet/impl/am_part_iterator.h>
#include <amulet/impl/gem_misc.h>
#include <amulet/impl/am_state_store.h>
#include FORMULA__H


Am_Object Am_Flip_Book_Group;

static void
flip_book_create (Am_Object self)
{
  Am_Object_Demon* create_demon = ((Am_Object_Advanced&)Am_Aggregate).
      Get_Demons ().Get_Object_Demon (Am_CREATE_OBJ);
  create_demon (self);

  Am_Value_List components;
  components = self.Get (Am_FLIP_BOOK_PARTS);
  Am_Value_List new_components;
  Am_Part_Iterator parts = self;
  int parts_length = parts.Length ();
  // if the object has no inherited graphical parts (all the parts are
  // non-graphical) or no parts, then the new object will still have no
  // graphical parts, so exit quickly.
  if (components.Empty () || (parts_length == 0)) {
    self.Set(Am_FLIP_BOOK_PARTS, Am_No_Value_List);
    return;
  }
  Am_Object* part_map = new Am_Object [parts_length*2];
  Am_Object part;
  int i;
  for (i = 0, parts.Start(); !parts.Last (); ++i, parts.Next ()) {
    part = parts.Get ();
    part_map[i+parts_length] = part;
    part_map[i] = part.Get_Prototype ();
  }
  Am_Object current_component;
  for (components.Start(); !components.Last (); components.Next ()) {
    current_component = components.Get ();
    for (i = 0; i < parts_length; ++i)
      if (current_component == part_map[i]) {
	new_components.Add (part_map[i+parts_length]);
	break;
      }
  }
  delete [] part_map;
  self.Set (Am_FLIP_BOOK_PARTS, new_components);
}

static void
flip_book_copy (Am_Object self)
{
  Am_Object_Demon* copy_demon = ((Am_Object_Advanced&)Am_Aggregate).
      Get_Demons ().Get_Object_Demon (Am_COPY_OBJ);
  copy_demon (self);

  Am_Value_List components;
  components = self.Get (Am_FLIP_BOOK_PARTS);
  Am_Value_List new_components;
  Am_Part_Iterator parts = self;
  int parts_length = parts.Length ();
  if (components.Empty () || (parts_length == 0)) {
    self.Set(Am_FLIP_BOOK_PARTS, Am_No_Value_List);
    return;
  }
  Am_Object* part_map = new Am_Object [parts_length*2];
  Am_Object part;
  int i;
  for (i = 0, parts.Start(); !parts.Last (); ++i, parts.Next ()) {
    part = parts.Get ();
    part_map[i+parts_length] = part;
    part_map[i] = part.Get (Am_SOURCE_OF_COPY);
  }
  Am_Object current_component;
  for (components.Start(); !components.Last (); components.Next ()) {
    current_component = components.Get ();
    for (i = 0; i < parts_length; ++i)
      if (current_component == part_map[i]) {
	new_components.Add (part_map[i+parts_length]);
	break;
      }
  }
  delete [] part_map;
  self.Set (Am_FLIP_BOOK_PARTS, new_components);
}

static void
flip_book_add_part (Am_Object owner, Am_Object old_object, Am_Object new_object)
{
  Am_Value_List components;
  if (old_object.Valid () && old_object.Is_Instance_Of (Am_Graphical_Object)) {
    owner.Make_Unique (Am_FLIP_BOOK_PARTS);
    components = owner.Get (Am_FLIP_BOOK_PARTS);
    components.Start ();
    if (components.Member (old_object)) {
      components.Delete (false);
      owner.Note_Changed (Am_FLIP_BOOK_PARTS);
    }
  }
  if (new_object.Valid () && new_object.Is_Instance_Of (Am_Graphical_Object)) {
    owner.Make_Unique (Am_FLIP_BOOK_PARTS);
    components = owner.Get (Am_FLIP_BOOK_PARTS);
    bool was_empty = components.Empty ();
    int value = (int)owner.Get (Am_VALUE, Am_RETURN_ZERO_ON_ERROR) %
                (components.Length () + 1);
    if (was_empty)
      owner.Set (Am_FLIP_BOOK_PARTS, Am_Value_List ().Add (new_object));
    else {
      components.Move_Nth (value);
      components.Insert (new_object, Am_BEFORE, false);
      owner.Note_Changed (Am_FLIP_BOOK_PARTS);
    }
  }
}

Am_Define_Value_List_Formula (flip_book_parts)
{
  Am_Value_List parts_list = self.Get (Am_FLIP_BOOK_PARTS);
  int i;
  parts_list.Start ();
  bool was_unique = self.Is_Unique (Am_GRAPHICAL_PARTS);
  Am_Value_List component = self.Get (Am_GRAPHICAL_PARTS, Am_NO_DEPENDENCY);
  if (parts_list.Last ()) {
    component.Start ();
    if (!component.Last ()) {
      component.Delete (!was_unique);
      if (was_unique)
        self.Note_Changed (Am_GRAPHICAL_PARTS);
    }
  } else {
    int position = (int)self.Get (Am_VALUE, Am_RETURN_ZERO_ON_ERROR) %
      parts_list.Length ();
    for (i = 0; i < position; ++i, parts_list.Next ()) ;
    Am_Object current = parts_list.Get ();
    component.Start ();
    if (component.Last ()) {
      was_unique = was_unique && component.Valid ();
      component.Add (current, Am_TAIL, !was_unique);
      current.Set (Am_RANK, 0);
      if (was_unique)
        self.Note_Changed (Am_GRAPHICAL_PARTS);
    } else if (component.Get () != current) {
      component.Set (current, !was_unique);
      current.Set (Am_RANK, 0);
      if (was_unique)
        self.Note_Changed (Am_GRAPHICAL_PARTS);
    }
  }
  return component;
}

#ifdef USE_SMARTHEAP
	#include <smrtheap.hpp>
#endif

static void init()
{
	#ifdef USE_SMARTHEAP
		unsigned oldCheckPoint = dbgMemSetCheckpoint(12);
	#endif

  Am_Flip_Book_Group = Am_Aggregate.Create (DSTR("Am_Flip_Book_Group"))
    .Set (Am_GRAPHICAL_PARTS, Am_No_Value_List)
    .Add (Am_FLIP_BOOK_PARTS, Am_No_Value_List)
    .Add (Am_VALUE, 0)
    .Set (Am_GRAPHICAL_PARTS, flip_book_parts);

  Am_Object_Advanced temp = (Am_Object_Advanced&)Am_Flip_Book_Group;

  Am_Demon_Set demons = temp.Get_Demons ().Copy ();
  demons.Set_Object_Demon (Am_CREATE_OBJ, flip_book_create);
  demons.Set_Object_Demon (Am_COPY_OBJ, flip_book_copy);
  demons.Set_Part_Demon (Am_ADD_PART, flip_book_add_part);

  temp.Get_Slot (Am_GRAPHICAL_PARTS).Set_Demon_Bits (Am_STATIONARY_REDRAW |
						     Am_EAGER_DEMON);
  temp.Set_Demons (demons);

  	#ifdef USE_SMARTHEAP
		dbgMemSetCheckpoint(oldCheckPoint);
	#endif
}

static Am_Initializer *initializer =
	new Am_Initializer(DSTR("Am_Flip_Book_Group"), init, 2.1f);
