// Default Sender Methods
///////////////////////////////////////////////////////////////////////

#include <iostream>
#include <am_inc.h>
#include <amulet/connection.h>
#include <sys/socket.h>
//
// char
//
Am_Define_Method(Am_Marshall_Method, void, Marshall_char,
                 (int the_socket, const Am_Value &in_value,
                  Am_Connection *my_connection_ptr))
{
  (void)my_connection_ptr; //avoid warning,  used by Lists & Obj
  char in_char = (char)in_value;
  send(the_socket, &in_char, 1, 0);
}

Am_Define_Method(Am_Unmarshall_Method, Am_Value, Unmarshall_char,
                 (int the_socket, Am_Connection *my_connection_ptr))
{
  (void)my_connection_ptr; //avoid warning,  used by Lists & Obj
  Am_Value out_value;
  char in_char;
  recv(the_socket, &in_char, 1, 0);
  out_value = in_char;
  return out_value;
}

//
// bool
//
Am_Define_Method(Am_Marshall_Method, void, Marshall_bool,
                 (int the_socket, const Am_Value &in_value,
                  Am_Connection *my_connection_ptr))
{
  (void)my_connection_ptr; //avoid warning,  used by Lists & Obj
  char in_bool = (in_value ? 1 : 0);
  send(the_socket, &in_bool, 1, 0);
}

Am_Define_Method(Am_Unmarshall_Method, Am_Value, Unmarshall_bool,
                 (int the_socket, Am_Connection *my_connection_ptr))
{
  (void)my_connection_ptr; //avoid warning,  used by Lists & Obj
  Am_Value out_value;
  char in_bool;
  recv(the_socket, &in_bool, 1, 0);
  out_value = (bool)(in_bool == 1 ? true : false);
  return out_value;
}

//
// short (16 bit) int
//
Am_Define_Method(Am_Marshall_Method, void, Marshall_int16,
                 (int the_socket, const Am_Value &in_value,
                  Am_Connection *my_connection_ptr))
{
  (void)my_connection_ptr; //avoid warning,  used by Lists & Obj
  int in_int = (int)in_value;
  short in_int16 = (short)in_int;
  short net_int16;
  net_int16 = htons(in_int16);
  send(the_socket, &net_int16, sizeof(net_int16), 0);
}

Am_Define_Method(Am_Unmarshall_Method, Am_Value, Unmarshall_int16,
                 (int the_socket, Am_Connection *my_connection_ptr))
{
  (void)my_connection_ptr; //avoid warning,  used by Lists & Obj
  Am_Value out_value;
  short int out_int16, net_int16;
  recv(the_socket, &net_int16, sizeof(net_int16), 0);
  out_int16 = ntohs(net_int16);
  out_value = out_int16;
  return out_value;
}

//
// long (32 bit) int
//
Am_Define_Method(Am_Marshall_Method, void, Marshall_int32,
                 (int the_socket, const Am_Value &in_value,
                  Am_Connection *my_connection_ptr))
{
  (void)my_connection_ptr; //avoid warning,  used by Lists & Obj
  long int in_int32 = (long int)in_value;
  long int net_int32;
  net_int32 = htonl(in_int32);
  send(the_socket, &net_int32, sizeof(net_int32), 0);
}

Am_Define_Method(Am_Unmarshall_Method, Am_Value, Unmarshall_int32,
                 (int the_socket, Am_Connection *my_connection_ptr))
{
  (void)my_connection_ptr; //avoid warning,  used by Lists & Obj
  Am_Value out_value;
  long int out_int32, net_int32;
  recv(the_socket, &net_int32, sizeof(net_int32), 0);
  out_int32 = ntohl(net_int32);
  out_value = out_int32;
  return out_value;
}

//
// float (16bit IEEE)
//
Am_Define_Method(Am_Marshall_Method, void, Marshall_float,
                 (int the_socket, const Am_Value &in_value,
                  Am_Connection *my_connection_ptr))
{
  (void)my_connection_ptr; //avoid warning,  used by Lists & Obj
  float in_float = (float)in_value;
  float net_float;
  net_float = htonf(in_float);
  send(the_socket, &net_float, sizeof(net_float), 0);
}

Am_Define_Method(Am_Unmarshall_Method, Am_Value, Unmarshall_float,
                 (int the_socket, Am_Connection *my_connection_ptr))
{
  (void)my_connection_ptr; //avoid warning,  used by Lists & Obj
  Am_Value out_value;
  float out_float, net_float;
  recv(the_socket, &net_float, sizeof(net_float), 0);
  out_float = ntohf(net_float);
  out_value = out_float;
  return out_value;
}

//
// double (32bit IEEE)
//
Am_Define_Method(Am_Marshall_Method, void, Marshall_double,
                 (int the_socket, const Am_Value &in_value,
                  Am_Connection *my_connection_ptr))
{
  (void)my_connection_ptr; //avoid warning,  used by Lists & Obj
  double in_double = (double)in_value;
  double net_double;
  net_double = htond(in_double);
  send(the_socket, &net_double, sizeof(net_double), 0);
}

Am_Define_Method(Am_Unmarshall_Method, Am_Value, Unmarshall_double,
                 (int the_socket, Am_Connection *my_connection_ptr))
{
  (void)my_connection_ptr; //avoid warning,  used by Lists & Obj
  Am_Value out_value;
  double out_double, net_double;
  recv(the_socket, &net_double, sizeof(net_double), 0);
  out_double = ntohd(net_double);
  out_value = out_double;
  return out_value;
}

//
// String
//
Am_Define_Method(Am_Marshall_Method, void, Marshall_string,
                 (int the_socket, const Am_Value &in_value,
                  Am_Connection *my_connection_ptr))
{
  (void)my_connection_ptr; //avoid warning,  used by Lists & Obj
  Am_String in_am_str = in_value;
  long size = strlen((const char *)in_am_str) + 1;
  long net_size = htonl(size);
  const char *in_str = in_am_str;
  send(the_socket, &net_size, sizeof(net_size), 0);
  send(the_socket, in_str, size, 0);
}

Am_Define_Method(Am_Unmarshall_Method, Am_Value, Unmarshall_string,
                 (int the_socket, Am_Connection *my_connection_ptr))
{
  (void)my_connection_ptr; //avoid warning,  used by Lists & Obj
  Am_Value out_value;
  char *out_str;
  long net_size, size;
  recv(the_socket, &net_size, sizeof(net_size), 0);
  size = ntohl(net_size);
  out_str = new char[size];
  recv(the_socket, out_str, size, 0);

  Am_String out_am_str = out_str;
  out_value = out_am_str;
  return out_value;
}

//
// Am_Value_List
//
Am_Define_Method(Am_Marshall_Method, void, Marshall_list,
                 (int the_socket, const Am_Value &in_value,
                  Am_Connection *my_connection_ptr))
{
  (void)the_socket; // avoid warning
  Am_Value_List in_list = in_value;
  my_connection_ptr->Send(in_list.Length());
  for (in_list.Start(); !in_list.Last(); in_list.Next())
    my_connection_ptr->Send(in_list.Get());
}

Am_Define_Method(Am_Unmarshall_Method, Am_Value, Unmarshall_list,
                 (int the_socket, Am_Connection *my_connection_ptr))
{
  long net_type;
  (void)the_socket;                          // avoid warning
  my_connection_ptr->Read_Length_and_List(); // receiver="receive_list_length"
  recv(the_socket, &net_type, sizeof(net_type), 0);
  Am_Value out_value = Unmarshall_int16.Call(the_socket, my_connection_ptr);
  return out_value;
}

//
// Am_Object
//

Am_Define_Method(Am_Marshall_Method, void, Marshall_object,
                 (int the_socket, const Am_Value &in_value,
                  Am_Connection *my_connection_ptr))
{
  (void)the_socket; // avoid warning
  Am_Object in_obj = in_value;
  Am_String proto_name = Am_Connection::Get_Net_Proto_Name(in_obj);
  //
  my_connection_ptr->Send(proto_name);

  //
  const char *in_obj_id = Am_Connection::Get_Net_Object_ID(in_obj);
  //
  long instance_num;
  if (in_obj_id == nullptr) // No ID assigned yet
  {
    //
    instance_num = Am_Connection::Num_Instances() + 1;
    Am_Connection::Set_Net_Instance(in_obj, proto_name, instance_num);
  } else {
    instance_num = my_connection_ptr->Extract_Instance_Num(in_obj_id);
  }
  my_connection_ptr->Send(instance_num);
  //  recv(the_socket, &net_type,sizeof(net_type),0);
  //  bool ok=Unmarshall_bool.Call(the_socket,my_connection_ptr);
  if (true) {
    Am_Value_List slot_list = in_obj.Get(Am_SLOTS_TO_SAVE);
    Am_Slot_Key current_slot;
    for (
        slot_list.Start(); !(slot_list.Last());
        slot_list
            .Next()) { /*Note: slot keys are shorts, value is char 256 will overflow!*/
      current_slot = (int)slot_list.Get();
      my_connection_ptr->Send(in_obj.Get(current_slot));
    }
  } else // Not ok
    std::cerr << " Error when sending object: " << in_obj << "!\n";
}

Am_Define_Method(Am_Unmarshall_Method, Am_Value, Unmarshall_object,
                 (int the_socket, Am_Connection *my_connection_ptr))
{
  long int net_type;
  recv(the_socket, &net_type, sizeof(net_type), 0);
  Am_String proto_name = Unmarshall_string.Call(the_socket, my_connection_ptr);
  Am_Object proto_obj = Am_Connection::Get_Net_Prototype(proto_name);
  if (proto_obj == Am_No_Object) {
    //my_connection_ptr->Send (false);
    return (Am_Value)Am_No_Object;
  } else //A prototype exists
  {
    recv(the_socket, &net_type, sizeof(net_type), 0);
    long inst_num = Unmarshall_int32.Call(the_socket, my_connection_ptr);
    Am_Object instance_obj =
        my_connection_ptr->Get_Net_Instance(proto_name, inst_num);
    if (instance_obj != Am_No_Object) {
      // Request Slots
      //my_connection_ptr->Send (true);
      my_connection_ptr->Receive_Object(instance_obj);
      return (Am_Value)Am_No_Object;
    } else // new instance
    {
      instance_obj = proto_obj.Create(DSTR("Remote_Obj"));
      // Need to check that new instance number is current+1.
      //if (inst_num == (Am_Connection::Num_Instances()+1))
      //{
      // Request Slots
      //my_connection_ptr->Send (true);
      Am_Connection::Set_Net_Instance(instance_obj, proto_name, inst_num);
      my_connection_ptr->Receive_Object(instance_obj);
      return (Am_Value)Am_No_Object;
      //}
      //else
      //{
      // Reject this object
      // std::cerr << "Object instance received out of order!\n";
      //my_connection_ptr->Send (false);
      //return (Am_Value)Am_No_Object;
      //}// wrong inst num
    } // new instance
  }   // else Prototype exists
  return (Am_Value)Am_No_Object;
} // Unmarshall Object
