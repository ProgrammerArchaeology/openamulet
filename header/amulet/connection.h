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

#ifndef _AM_CONNECTION_H
#define _AM_CONNECTION_H

#ifndef _WIN32

#include <sys/types.h>
#include <am_inc.h>
#include TYPES__H
#include VALUE_LIST__H
#include OBJECT__H
#include STANDARD_SLOTS__H

#ifdef OA_VERSION
	#include <amulet/univ_map_oa.hpp>
#else
	#include UNIV_MAP__H
#endif

#include SOCKET_STUBS__H

extern Am_Slot_Key Am_NET_BLESSING;

//Advance declarations
class Am_Connection_Data;
class Am_Connection;

// Method used to Unmarshall a value from a socket.  Use the Am_Connection
// parameter to Unmarshall nested items from the same socket.
Am_Define_Method_Type (Am_Unmarshall_Method, Am_Value, (int the_socket, Am_Connection *my_connection))

// Method used to Marshall a value into a socket.  Use the Am_Connection
// parameter to Marshall nested items to the same socket.
Am_Define_Method_Type(Am_Marshall_Method, void,(int the_socket, const Am_Value& value, Am_Connection *my_connection))

extern Am_Unmarshall_Method Am_No_Unmarshall_Method;
extern Am_Marshall_Method	Am_No_Marshall_Method;

#ifdef OA_VERSION
	typedef OpenAmulet::Map<Am_Value_Type,Am_Marshall_Method> 	Am_Map_Marshall_Methods;
	typedef OpenAmulet::Map_Iterator<Am_Map_Marshall_Methods> 	Am_MapIterator_Marshall_Methods;

	typedef OpenAmulet::Map<Am_Value_Type,Am_Unmarshall_Method>	Am_Map_Unmarshall_Methods;
	typedef OpenAmulet::Map_Iterator<Am_Map_Unmarshall_Methods>	Am_MapIterator_Unmarshall_Methods;

	typedef OpenAmulet::Map<Am_Value,Am_Value_Type> 			Am_Map_Types;
	typedef OpenAmulet::Map_Iterator<Am_Map_Types> 				Am_MapIterator_Types;

	typedef OpenAmulet::Map<Am_Value_Type,Am_Value> 			Am_Map_Net_IDs;
	typedef OpenAmulet::Map_Iterator<Am_Map_Net_IDs> 			Am_MapIterator_Net_IDs;
#else
	// Note the unsigned long type id is stored in machine byte order
	// *NOT* network byte order.
	Am_DECL_MAP (Marshall_Methods, Am_Value_Type, Am_Marshall_Method)
	Am_DECL_MAP (Unmarshall_Methods, Am_Value_Type, Am_Unmarshall_Method)

	// Types should always be the inverse of Net_IDs.
	Am_DECL_MAP (Types, Am_Value, Am_Value_Type)
	Am_DECL_MAP (Net_IDs, Am_Value_Type, Am_Value)
#endif


////////////////////////////////////////////////////
// There may be one Receiver method per connection. It should Pop() items from
// the Value Queue and act upon them. It will be cxalled immediately after the
// Unmarshall methods are done.

Am_Define_Method_Type (Am_Receive_Method, void,
                       (Am_Connection *my_connection))


class connection_list_class;
typedef class connection_list_class
  Connection_List_Type, * Connection_List_Ptr;

//////////////////////////////////////////////////////////////////////////////
// Am_Connection declaration
/////////////////////////////


class Am_Connection
{
  Am_WRAPPER_DECL (Am_Connection)
public:

  Am_Connection(void);
  // The destructor is declared by Am_WRAPPER_DECL (??)

  ///////////////////////////////////////////////////////
  // Static constructors
  // These methods are used to create a Connection object and initiate the
  // connection. The connection need not have have been established before the
  // connection object is returned. Check if the connection is connected with
  // Am_Connection::Connected();

  static void Reset(void);

  static Am_Connection *Open(char addr[]);
  static Am_Connection *Open(struct sockaddr *sa);
  static Am_Connection *Open(void );

  // These methods register the types for sending across an Am_Connection.
  // Each type needs a method to flatten the type into network bytes, and a
  // method to restore those bytes to the proper object.
  // An unmarshalling method must return an Am_Value for insertion into the
  // Connection's Value_Queue.
  //   Currently these methods are also responsible for sending & recieving.
  // This is because recieving some types requires knowledge of the data in
  // order to know the length.

  static bool Register_Type (Am_Value_Type type,
		      const Am_Marshall_Method& marshall,
		      const Am_Unmarshall_Method& unmarshall,
		      unsigned long id = 0 );
  // Is the network being used?
  static bool Active(void)
  { return num_sockets != 0; }

  // Block Pending incomming connection. Use with Caution:
  // No other events can take place until this returns.
  static void Wait_For_Connect(void);

  // Was there any activity on the sockets?
  static bool Sockets_Have_Input(fd_set *fds);

  // Merge mask adds the sockets in the Connection list to the supplied file
  // descriptor bit-mask without disturbing bits related to other sockets.

  static int Merge_Mask(fd_set *fds);

  //Handle_Sockets calls the Unmarshall method for the apropriate types on each
  //socket with a 1 in the readfds bitmask.

  static void Handle_Sockets(fd_set *readfds);

  // These are for marshalling objects. Comming soon!
  //
  // This method is used to record items that are referred to by the objects
  // being Received or Sent, but the items themselves are permanent parts of
  // the application hence they shouldn't (or possibly can't) be Sent as
  // well.



  //returns the name if the value is registered as a prototype.  If
  //not registered, returns (0L)

  //  Am_String Is_Registered_Prototype (Am_Object obj);

  //
  //Object Marshalling Methods
  //
  static const char * Get_Net_Proto_Name(Am_Object &the_obj);
  static Am_Object  Get_Net_Prototype(const char * proto_name);
  static const char * Get_Net_Object_ID(Am_Object &the_obj);
  static Am_Object  Get_Net_Instance(const char *proto_name,
				     int instance_num);

  static bool Register_Prototype (Am_String name, Am_Object & obj);
  static void Set_Net_Instance(Am_Object &the_obj_ptr,
			        const char * proto_name,
			       int instance_num);
  static int Num_Instances(void);


 //////////////////////////////////////////////////////////////////////////////
 // Instance Methods
 ///////////////////////

  void Send(Am_Value value);     // Will call the apropriate method
  Am_Value Receive(void);  // Retireves Value from Queue Syncronously

  // List marshalling methods
  //
  void Read_Length_and_List(void); // Gets length; sets up Read_List;
  void Read_List(unsigned long int length);
  void Add_To_Current_List(Am_Value val); //Adds val to end of current list;

  void Decrement_Check_List(); // Decrements the count of items in the current
                               // list and pushes the list onto the Value
                               // Queue if it is finished.

  // Object methods

  long int Extract_Instance_Num(const char * id_str);
  //char * Extract_Proto_Name(char * id_str);
  //Am_Object Extract_Prototype(char * id_str);

  void Receive_Object(Am_Object &the_obj);
  void Receive_Object_Slots();
  void Add_To_Current_Object(Am_Value val);
  void Check_Object();

  // Input loop mehtods

  void Handle_Input(void); // Automatically invokes marshall methods, which
			   // must queue an  Am_Value on the Value_Queue,
                           // and calls the Receive method if there is one.

  bool Register_Receiver(Am_Receive_Method &receiver); // Async Receive

  void Close(void);

// Data access methods
  int Get_Socket(void);
  bool Connected(void);
  bool Valid(void);
  bool Waiting();
  void Set_Socket(int the_socket);
  void Set_Connect_Flag(bool state);
  void Set_Valid_Flag(bool state);
  void Set_Waiting_Flag(bool state);

  ////////////////////////////////////////////////////////////////////////////
  // private:
  ////////////////////////////////////////////////////////////////////////////
  // Keep track of connections for dispaching and cleanup:

  static void Add_Connection(Am_Connection *new_connection);
  static void Delete_Connection(int sock);
  static void Delete_Connection(Am_Connection *doomed_connection_ptr);
  static void Delete_Connection(Am_Connection &doomed_connection_ref);
  static void Delete_All_Connections(void);
  static Connection_List_Ptr connection_list;

  //Marshalling
  static Am_Map_Unmarshall_Methods Unmarshall_Methods;
  static Am_Map_Marshall_Methods Marshall_Methods;
  static Am_Map_Net_IDs Net_IDs;
  static Am_Map_Types Types;
  static void Register_Default_Methods(void);


  //Object Handling
  static Am_Value_List Net_Prototypes;
  static Am_Value_List Net_Instances;


  //
  // Data Members
  //
  static unsigned long id_count;   // For generating unique IDs to do
                                   //   type ID over the network.
  static long num_sockets;         // How many sockets are there?
  static int max_socket;           // We tell select this is the highest
                                   //   socket number to listen to?
  static int m_parent_socket;      // The socket used to receive incomming
                                   //   connections.
  static fd_set *socket_flags;       // Used for rapid check for input.
};


#endif // _WIN32
#endif // CONNECTION__H
