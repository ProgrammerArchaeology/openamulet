#include <amulet.h>
#include <am_inc.h>
#include OBJECT_ADVANCED__H
#include <iostream>

#define MY_BIT 0x0040

using namespace std;

Am_Slot_Key MY_SLOT = Am_Register_Slot_Name ("My Slot");

Am_Object Foo;
Am_Object_Demon* proto_create;


void my_create_demon (Am_Object self)
{
if (proto_create != (0L))
    {
      char c;
      cout << proto_create << "Id the fn ptr\n" << flush;
      cin >> c; 
      proto_create (self);  // Call prototype create demon.
    }
  // Do my own code.
  cout << self << " was created!!\n";
}

void my_slot_demon (Am_Slot slot)

{

  Am_Object_Advanced self = slot.Get_Owner ();
  int top;
  top=self.Get(Am_TOP);
  self.Set (MY_SLOT, top);

  cerr << "demon activated at " << top << endl << flush;
  cerr << "Network contains "<< Foo.Peek(MY_SLOT) << endl << flush;
}

int main (void)
{
Am_Initialize();

Foo=Am_Rectangle.Create("foo");

Am_Object my_win=Am_Window.Create("my_win")
  .Set(Am_WIDTH,400)
  .Set(Am_HEIGHT,400);


//Am_Object_Advanced my_adv_obj;
Am_Object proto_rect=Am_Rectangle.Create()
  .Set(Am_TOP,10)
  .Set(Am_LEFT,10)
  .Set(Am_WIDTH,100)
  .Set(Am_HEIGHT,100)
  .Add (MY_SLOT, 10);


Am_Object_Advanced my_adv_obj=(Am_Object_Advanced&)proto_rect;


// Here we will modify the demon set of my_adv_obj by first making
// a copy of the old set and modifying it. The new demon set is then
// placed back into the object.
Am_Demon_Set my_demons (my_adv_obj.Get_Demons().Copy());

// Here I set the creation demon. Note that the creation demon calls the 
// prototype creation demon too.

  Am_Object proto_obj=my_adv_obj.Get_Prototype ();
  Am_Object_Advanced proto_obj_adv =  (Am_Object_Advanced &)proto_obj;
  proto_create = proto_obj_adv. \
    Get_Demons ().Get_Object_Demon (Am_CREATE_OBJ);
  my_demons.Set_Object_Demon(Am_CREATE_OBJ,my_create_demon);

// Here we install a slot demon that uses bit 5. The slot demon's semantics
// are to activate when the slot changes value and only once per object.
// Make sure that the demon set is local to the object (see above section). 

my_demons.Set_Slot_Demon (MY_BIT, my_slot_demon,
                           Am_DEMON_ON_CHANGE | Am_DEMON_PER_OBJECT);
my_adv_obj.Set_Demons(my_demons);

// Here we set a new bit to a slot. To make sure we do not turn off
// previously set bits, we first get the old bits and bitwise-or the new one.
//
Am_Slot slot = my_adv_obj.Get_Slot (Am_TOP);
unsigned short prev_bits = slot.Get_Demon_Bits ();
slot.Set_Demon_Bits (MY_BIT | prev_bits);
if (prev_bits & MY_BIT)
  Am_Error ("duplicate demon bit in slot\n");

// Make the new slot demon bit default on.
unsigned short default_bits = my_adv_obj.Get_Default_Demon_Bits ();
if (default_bits & MY_BIT)
  Am_Error ("duplicate default demon bit\n");
default_bits |= MY_BIT;
my_adv_obj.Set_Default_Demon_Bits (default_bits);

// Setting the demon mask
unsigned short mask = my_adv_obj.Get_Demon_Mask ();
mask |= MY_BIT;  // add the new demon bit.
my_adv_obj.Set_Demon_Mask (mask);

Am_Object my_rect=my_adv_obj.Create("my_rect");

Am_Screen.Add_Part(my_win);

Am_Object rect_2=my_rect.Create("rect2")
  .Set(Am_FILL_STYLE, Am_Red)
  .Set(Am_LEFT,30);

Am_String my_str="Foo is an object!";
Foo.Add(MY_SLOT,my_str);


////////////////////////
my_win.Add_Part(my_rect);
my_win.Add_Part(rect_2);

Am_Object my_inter = Am_Move_Grow_Interactor.Create ("my_inter");

my_win.Add_Part(my_inter);

////////////////////////
Am_Main_Event_Loop();
Am_Cleanup();

return 0;
}



