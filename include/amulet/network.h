#ifndef _AM_NETWORK_H
#define _AM_NETWORK_H

#define NET_BIT 0x0040

#include <am_inc.h>
#include <amulet/connection.h>

class Am_Network_Group
{
public:
  Am_Network_Group();
  ~Am_Network_Group();
  void Add(Am_String machine_name);
  void Link(Am_Object net_obj, Am_String name);

  // obscure
  void Broadcast(Am_Object obj);
  void Curse(Am_Object obj); // Set demons upon it
  //Current model doesn't allow the leader to quit. Would need to track
  //personal order and second best.
  void Set_Lowest(long new_low);
  long Lowest(void);
  bool Am_Leader(void);
  //  Kick out (connection) // how is the broadcast list updated?

private:
  //object association list for link
  Am_Value_List Net_Objects;
  // A list of connections to machines added to this network.
  Am_Value_List Member_Connections;
  long lowest;
  bool leader;
};

extern Am_Network_Group Am_Network;

/*
With merge, there will only be one net object. The connection
will register one as sent and one as received, but Merge will use only one
    if merged obj has already been received, destroy data and replace with incoming. Register the new object as that instance?

Add will broadcast all objects in our list to the newcomer
*/

#endif
