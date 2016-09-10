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
#include <amulet/impl/am_demon_set.h>
#include <amulet/impl/opal_impl.h>
#include <amulet/impl/gem_misc.h>
#include <amulet/impl/am_state_store.h>
#include <amulet/impl/opal_load_save.h>
#include FORMULA__H


Am_Object Am_Resize_Parts_Group;

//demon procedure
static void
resize_group_parts (Am_Slot first_invalidated)
{
  Am_Object self;
  self = first_invalidated.Get_Owner ();
 std::cout << "resize of " << self <<std::endl <<std::flush;
  int old_width, old_height, new_width, new_height;
  float width_ratio, height_ratio;
  old_width = self.Get(Am_OLD_WIDTH);
  old_height = self.Get(Am_OLD_HEIGHT);
  new_width = self.Get(Am_WIDTH);
  new_height = self.Get(Am_HEIGHT);
  if (old_width && old_height) { //otherwise, first time changed
    if (new_width == 0) new_width = 1;
    if (new_height == 0) new_height = 1;
    width_ratio = (float)new_width / (float)old_width;
    height_ratio = (float)new_height / (float)old_height;
    Am_Value_List parts;
    parts = self.Get(Am_GRAPHICAL_PARTS);
    Am_Object part;
    for (parts.Start(); !parts.Last(); parts.Next()) {
      part = parts.Get();
      int left = (int)part.Get(Am_LEFT);
      int top = (int)part.Get(Am_TOP);
      int height = (int)part.Get(Am_HEIGHT);
      int width = (int)part.Get(Am_WIDTH);
      part.Set(Am_LEFT, (int) (left * width_ratio));
      part.Set(Am_WIDTH, (int) (width * width_ratio));
      part.Set(Am_TOP, (int) (top * height_ratio));
      part.Set(Am_HEIGHT, (int) (height * height_ratio));
    }
  }
  self.Set(Am_OLD_HEIGHT, new_height);
  self.Set(Am_OLD_WIDTH, new_width);
}

static void init()
{
  Am_Resize_Parts_Group = Am_Group.Create(DSTR("Am_Resize_Parts_Group"))
    .Add(Am_OLD_WIDTH, 0)
    .Add(Am_OLD_HEIGHT, 0)
    .Add(Am_CREATED_GROUP, true)
    .Add (Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Group)
    .Add (Am_SLOTS_TO_SAVE, Am_Value_List()
	  .Add(Am_LEFT).Add(Am_TOP).Add(Am_WIDTH).Add(Am_HEIGHT)
	  .Add(Am_CREATED_GROUP))
    ;

  Am_Object_Advanced temp = (Am_Object_Advanced&)Am_Resize_Parts_Group;

  Am_Demon_Set demons = temp.Get_Demons ().Copy ();
  demons.Set_Slot_Demon (Am_GROUP_RESIZE_PARTS, resize_group_parts,
			 Am_DEMON_PER_OBJECT | Am_DEMON_ON_CHANGE);
  temp.Set_Demons (demons);
  Am_Slot slot = temp.Get_Slot (Am_WIDTH);
  unsigned short prev_demon_bits = slot.Get_Demon_Bits();
  slot.Set_Demon_Bits(prev_demon_bits | Am_GROUP_RESIZE_PARTS);
  slot = temp.Get_Slot (Am_HEIGHT);
  prev_demon_bits = slot.Get_Demon_Bits();
  slot.Set_Demon_Bits(prev_demon_bits | Am_GROUP_RESIZE_PARTS);

  // Default Load and Save Methods
  Am_Default_Load_Save_Context.Register_Prototype("Am_RESIZE_PARTS_GROUP",
						  Am_Resize_Parts_Group);
	
}


static Am_Initializer *initializer = new Am_Initializer(DSTR("Am_Resize_Parts_Group"), init, 2.1f, 11);
