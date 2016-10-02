/*
 * Network_Class implementation. 
 *
 */

#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <am_inc.h>
#include <amulet/network.h>
#include <amulet/connection.h>
#include <amulet/object_advanced.h>
#include <amulet/types.h>

//
// Global function prototypes
//

void net_create_demon(Am_Object self);
void net_slot_demon(Am_Slot slot);

Am_Define_Method(Am_Receive_Method, void, net_receive,
                 (Am_Connection * my_connection_ptr))
{
  Am_Value val = my_connection_ptr->Receive();
  switch (val.type) {
  case Am_LONG: // Note, this will only work
                // for Am_Network, no other groups.
    {
      long other_guy = (long int)val;
      if (other_guy < Am_Network.Lowest())
        Am_Network.Set_Lowest(other_guy); // Also re-evaluates Am_Leader
    }
    break;
  case Am_OBJECT:
    //Am_Object obj=val;
    //obj.Set(Am_NET_RECEIVING,(Am_Ptr)(0L));
    break;
  }
}

//
// Slot key defn
//
Am_Slot_Key Am_CREATE_DEMON = Am_Register_Slot_Name("CREATE_DEMON");

//
// Am_Network_Group Constructors
//
Am_Network_Group::Am_Network_Group()
{
  lowest = time(nullptr);
  leader = true;
}

Am_Network_Group::~Am_Network_Group() {}

//
// Public Methods
//

void
Am_Network_Group::Add(Am_String machine_name)
{
  Am_Connection *new_connection = Am_Connection::Open(machine_name);
  Member_Connections.Add((Am_Ptr)new_connection);
  new_connection->Register_Receiver(net_receive);
  new_connection->Send(lowest);
  Am_Object temp_obj;
  Am_Assoc assoc_pair;
  for (Net_Objects.Start(); !(Net_Objects.Last()); Net_Objects.Next()) {
    assoc_pair = Net_Objects.Get();
    temp_obj = assoc_pair.Value_2();
    new_connection->Send((Am_Value)temp_obj);
  }
}

void
Am_Network_Group::Link(Am_Object net_obj, Am_String name)
{
  Am_String real_proto_name;
  if (Net_Objects.Assoc_2((Am_Value)name) == Am_No_Value) {
    Am_Assoc obj_reg((Am_Value)name, net_obj);
    Net_Objects.Add(obj_reg);
    Am_Object proto_obj = net_obj.Get_Prototype();
    real_proto_name = Am_Connection::Get_Net_Proto_Name(net_obj);
    if (real_proto_name == Am_No_String) {
      Am_Connection::Register_Prototype(name, proto_obj);
      real_proto_name = name;
    }
  } // If not in Net_Objects
  else {
    Net_Objects.Delete();
    Am_Assoc obj_reg((Am_Value)name, net_obj);
    Net_Objects.Add(obj_reg);
    //Potential error if new object has different prototype need override
    // at Connection level.
    real_proto_name = name;
  }
  Net_Objects.Assoc_1(net_obj);
  int n = Net_Objects.Nth();
  Am_Connection::Set_Net_Instance(net_obj, real_proto_name, n);
  Curse(net_obj); // Set demons upon it.
  net_obj.Add(Am_NET_BLESSING, (Am_Ptr)nullptr, Am_OK_IF_THERE);
}

//
// Semi-Private methods
//

void
Am_Network_Group::Broadcast(Am_Object obj)
{
  Am_Ptr mistyped_ptr;
  Am_Connection *conn_ptr;
  uintptr_t conn_id, bless_id;
  for (Member_Connections.Start(); !(Member_Connections.Last());
       Member_Connections.Next()) {
    mistyped_ptr = Member_Connections.Get();
    conn_ptr = (Am_Connection *)mistyped_ptr;
    Am_Value b_val = obj.Peek(Am_NET_BLESSING);
    if (b_val != Am_No_Value) {
      Am_Assoc b_assoc = b_val;
      Am_Ptr b_ptr = b_assoc.Value_1();
      bless_id = (uintptr_t)b_ptr;
      conn_id = (uintptr_t)conn_ptr;
      if (bless_id != conn_id) {
        conn_ptr->Send((Am_Value)obj);
      } else {
      }
    } else {
      conn_ptr->Send((Am_Value)obj);
    }
  }
}

void
Am_Network_Group::Curse(Am_Object obj)
{
  Am_Object_Advanced adv_obj = (Am_Object_Advanced &)obj;

  // Here we will modify the net_demon set of adv_obj by first making
  // a copy of the old set and modifying it. The new net_demon set is then
  // placed back into the object.
  Am_Demon_Set net_demons(adv_obj.Get_Demons().Copy());

  // Here I set the creation demon. Note that the creation demon calls the
  // prototype creation demon too.

  Am_Object proto_obj = adv_obj.Get_Prototype();
  Am_Object_Advanced proto_obj_adv = (Am_Object_Advanced &)proto_obj;
  Am_Object_Demon *proto_create =
      proto_obj_adv.Get_Demons().Get_Object_Demon(Am_CREATE_OBJ);
  // Am_Assoc creator(obj,proto_create);
  // Mapping from objects to their parent create demons
  net_demons.Set_Object_Demon(Am_CREATE_OBJ, net_create_demon);

  // Here we install a slot demon that uses bit 5. The slot demon's semantics
  // are to activate when the slot changes value and only once per object.
  // Make sure that the demon set is local to the object (see above section).

  net_demons.Set_Slot_Demon(NET_BIT, net_slot_demon,
                            Am_DEMON_ON_CHANGE | Am_DEMON_PER_OBJECT);
  adv_obj.Set_Demons(net_demons);

  // Here we set a new bit to a slot. To make sure we do not turn off
  // previously set bits, we first get the old bits and bitwise-or the new one.
  //
  Am_Value_List slot_list = adv_obj.Get(Am_SLOTS_TO_SAVE);
  for (slot_list.Start(); !(slot_list.Last()); slot_list.Next()) {
    Am_Slot_Key key = (int)slot_list.Get();
    Am_Slot slot = adv_obj.Get_Slot(key);
    unsigned short prev_bits = slot.Get_Demon_Bits();
    slot.Set_Demon_Bits(NET_BIT | prev_bits);
    if (prev_bits & NET_BIT)
      Am_Error("duplicate demon bit in slot\n");
  }
  // Make the new slot demon bit default on.
  unsigned short default_bits = adv_obj.Get_Default_Demon_Bits();
  if (default_bits & NET_BIT)
    Am_Error("duplicate default demon bit\n");
  default_bits |= NET_BIT;
  adv_obj.Set_Default_Demon_Bits(default_bits);

  // Setting the demon mask
  unsigned short mask = adv_obj.Get_Demon_Mask();
  mask |= NET_BIT; // add the net demon bit.
  adv_obj.Set_Demon_Mask(mask);
}

//
// Global functions
//

void
net_create_demon(Am_Object self)
{
  Am_Object_Demon *proto_create =
      (Am_Object_Demon *)(Am_Ptr)self.Peek(Am_CREATE_DEMON);
  if (proto_create != nullptr) {
    char c;
    std::cin >> c;
    proto_create(self); // Call prototype create demon.
  }
  // Do my own code.
}

void
net_slot_demon(Am_Slot slot)
{
  Am_Object_Advanced owner = slot.Get_Owner();

  if (slot.Get_Key() != Am_NET_BLESSING) {
    Am_Network.Broadcast(owner);

    // This design needs improvement. The behavior of Am_PER_OBJECT was
    // unexpected.
    Am_Value val = owner.Get(Am_NET_BLESSING);
    if (val != Am_No_Value) {
      Am_Assoc blessing = val;
      int slots_remaining = blessing.Value_2();
      Am_Ptr conn_ptr = blessing.Value_1();
      slots_remaining--;
      if (slots_remaining > 0) {
        Am_Assoc new_blessing(conn_ptr, slots_remaining);
        owner.Set(Am_NET_BLESSING, new_blessing);
      } else {
        owner.Set(Am_NET_BLESSING, Am_No_Value);
      }
    }
  }
}

void
Am_Network_Group::Set_Lowest(long new_low)
{
  leader = false;
  lowest = new_low;
}
long
Am_Network_Group::Lowest(void)
{
  return lowest;
}
bool
Am_Network_Group::Am_Leader(void)
{
  return leader;
}

/////////////////////////////////////////////////////////////
//

Am_Network_Group Am_Network;
