
/*****************************************************************
 * Class Am_Connection
 *****************************************************************
 */

//Constants

#define Am_SOCKET_PORT 5800
#define MAX_CONNECT 100
#define MAX_BIND_RETRY 10
#define MAX_NET_SLOTS 100
#define MAXHOSTNAME   32   /* maximum host name length we tolerate */
#define BACKLOG       1
#define BUFFER_SIZE   512

// Includes

#include <amulet.h>
#include CONNECTION__H
#include INITIALIZER__H
#include OBJECT_ADVANCED__H
#include TYPES__H
#include STANDARD_SLOTS__H
#include VALUE_LIST__H


//New slot key for object identification
Am_Slot_Key Am_NET_BLESSING= Am_Register_Slot_Name ("BLESSING");

//Define Net Types that aren't equal to predefined Types
////////////////////////////////////////////////////////
//Lazy conserves bandwidth?
//const Am_Value_Type   Am_NET_REQUEST=(Am_PROC+1);
//const Am_Value_Type   Am_NET_REPLY=Am_PROC+2;
//Eager minimizes lag time in Get()
//const Am_Value_Type   Am_NET_UPDATE=Am_PROC+3;
//const Am_Value_Type   Am_NET_NEW_SLOT=Am_PROC+4;
const Am_Value_Type   Am_NET_OBJECT_OK=Am_PROC+4;

#ifndef _WIN32

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#endif

////////////////////////////////////////////////////////////////////////////
// Supporting Types
#include "amulet/b_order.h"
#include "def_net_methods.cpp"

class connection_list_class
  {
  public:
    Am_Connection *connection_ptr;
    connection_list_class * next;
  };


typedef class incomplete_list_node
{
public:
  Am_Value_List list;
  unsigned long int length;
  Am_Receive_Method prev_receiver;
  incomplete_list_node *next;
} Incomplete_List_Type, *Incomplete_List_Ptr;

typedef class incomplete_obj_node
{
public:
  Am_Object obj;
  bool bogus;
  Am_Value_List slot_list;
  Am_Receive_Method prev_receiver;
  class incomplete_obj_node *next;
} Incomplete_Object_Type, *Incomplete_Object_Ptr;

//
// Static Variable File-Scope Declaration
//

Connection_List_Ptr Am_Connection::connection_list=(0L);
long Am_Connection::num_sockets=0;
int Am_Connection::max_socket=0;
int Am_Connection::m_parent_socket=0;
unsigned long Am_Connection::id_count=0;
fd_set *Am_Connection::socket_flags;


/////////////////////////
//Marshalling

AM_DEFINE_METHOD_TYPE_IMPL (Am_Unmarshall_Method);
AM_DEFINE_METHOD_TYPE_IMPL (Am_Marshall_Method);

Am_Unmarshall_Method Am_No_Unmarshall_Method;
Am_Marshall_Method Am_No_Marshall_Method;

/////////////////////////////////////////////////////////////////////////
//Method maps

#ifndef OA_VERSION
	AM_IMPL_MAP(Unmarshall_Methods, Am_Value_Type, Am_NONE, Am_Unmarshall_Method, Am_No_Unmarshall_Method)
	AM_IMPL_MAP(Marshall_Methods, Am_Value_Type, Am_NONE, Am_Marshall_Method, Am_No_Marshall_Method)
	AM_IMPL_MAP(Types, Am_Value, Am_NONE, Am_Value_Type, Am_NONE)
	AM_IMPL_MAP(Net_IDs, Am_Value_Type, Am_NONE, Am_Value, Am_NONE)
#endif

Am_Map_Unmarshall_Methods Am_Connection::Unmarshall_Methods;
Am_Map_Marshall_Methods Am_Connection::Marshall_Methods;
Am_Map_Net_IDs Am_Connection::Net_IDs;
Am_Map_Types Am_Connection::Types;

/////////////////////////////////////////////////////////////////////////
//Object Prototype Maps

Am_Value_List Am_Connection::Net_Prototypes;
Am_Value_List Am_Connection::Net_Instances;


//
// Net_ID_Str creator function
//

char * Net_ID_Str(const char * new_protoname, int new_instance)
{
  int plen,ilen;
  char *string;
  plen=strlen(new_protoname);
  ilen=10; // should be log(new_instance)
  string=new char[ilen+plen];
  sprintf(string,"%s %d",new_protoname,new_instance);
  return string;
  /*
   * If strsteam works
   *
   ostrstream buffer;
   buffer << protoname <<" "<< instance;
   return buffer.str();
   */
}

long int Am_Connection::Extract_Instance_Num(const char * id_str)
{
  long int inst_num;
  char proto_name[30];
  sscanf(id_str,"%s %ld",proto_name,&inst_num);
  return inst_num;
}
/*
char * Am_Connection::Extract_Proto_Name(char * id_str)
{
  char *proto_name=new char[30];
  sscanf(id_str,"%s %dl",proto_name,inst_num);
  return inst_num;
}

Am_Object Am_Connection::Extract_Prototype(char * id_str)
{
  char *proto_name=new char[30];
  sscanf(id_str,"%s %dl",proto_name,inst_num);
  return Get_Net_Prototype(proto_name);
}
*/


////////////////////////////////////////////////////////////////////////////

class Am_Connection_Data : public Am_Wrapper
{
  AM_WRAPPER_DATA_DECL (Am_Connection)

    friend class Am_Connection;
public:
  Am_Connection_Data(void);
  Am_Connection_Data(Am_Connection_Data* prototype);
  int operator== (Am_Connection_Data &Test_Data);
private:
  int m_socket;           // connected socket Add depends on public m_socket
  bool connected;         // Set by Open, Close, possibly UnmarshallX or
                          //Marshall (x)
  bool waiting;           // Am I waiting to accept an incomming conn?
  Am_Value_List Value_Queue; // Stores Amulet formatted incomming data
  Am_Receive_Method my_receiver;//Acts on new items in Value_Queue immediately
  Incomplete_List_Ptr List_Queue; //Stores info on lists while they are being
                                  //read.
  Incomplete_Object_Ptr Object_Queue;//Stores info on lists while they are
                                     // being read.
};



AM_WRAPPER_IMPL (Am_Connection)

Am_Connection::Am_Connection(void)
{
  data = (0L);
}

static void init()
{
  Am_Connection::Reset();
}

static void cleanup()
{
  Am_Connection::Delete_All_Connections();
}

static Am_Initializer* initializer =
	new Am_Initializer(DSTR("Connection"), init , 3.0, 1, cleanup);


AM_DEFINE_METHOD_TYPE_IMPL(Am_Receive_Method);

void Am_Connection::Reset(void)
{
    connection_list=(0L);
    num_sockets=0;
    max_socket=0;
    m_parent_socket=0;
    id_count=0;
    socket_flags=new fd_set;
    FD_ZERO(socket_flags);
    Register_Default_Methods();
}



/*
 * Caveat:
 * These 3 Open methods assume that we don't need each side to specify the
 * other. The first waiting connection will get the first caller.
 * A better way might be to have the Handle sockets check the address of
 * the caller.
 */

/*
 * Open(char []): Creates a connection, based on a string which contains
 * a human readable or dotted decimal internet address. Calls the other
 * Open methods to complete the task.
 */
Am_Connection *Am_Connection::Open(char addr[])
{
  /*
   * Variable initialization
   */
  struct hostent *hp;         /* result of host name lookup */
  struct sockaddr_in *sa;     /* Internet socket addr. structure */
  Am_Connection * p_connection;
  sa= new (struct sockaddr_in);

  /* Look up the specified hostname */
  if ((hp = gethostbyname(addr))== (0L))
    {
      p_connection=Open();
    }
  else
    {
      /* Put host's address and addresss type into socket structure */
      memmove ( (char *)&(sa->sin_addr),(char*)hp->h_addr, hp->h_length);
      sa->sin_family = PF_INET;
      /* Put the port into the socket structure */
      sa->sin_port=Am_SOCKET_PORT;
      p_connection=Open((struct sockaddr*)sa);
     }
  return p_connection;
}

/*
 * Open(sockaddr*): Creates a connection, based an address passed in
 * a sockaddr structure pointer. Calls Open(void) if connect fails.
 */
Am_Connection *Am_Connection::Open(struct sockaddr *sa)
{
  /*
   * Variable initialization
   */
  Am_Connection * p_connection;
  p_connection= new Am_Connection;
  p_connection->data= new Am_Connection_Data;

  Add_Connection(p_connection);

  /* Allocate an open socket */
  int temp_socket;

  if ((temp_socket = socket(PF_INET,SOCK_STREAM, 0)) < 0)
    {
      perror ("socket");
      p_connection->Set_Valid_Flag(false);
    }
  else
    {
      p_connection->Set_Socket(temp_socket);
      /* Attempt to Connect to the remote server */
      if ((connect (p_connection->Get_Socket(), sa,
		   sizeof (struct sockaddr))) >= 0)
	{
	  p_connection->Set_Connect_Flag(true);
	}
      else /* Go into passive mode */
	{
	  //perror("connect");
	  Delete_Connection(p_connection);
	  p_connection=Open();
	}
    }
  return p_connection;
}


/*
 * Open(void): Creates a new, unconnected Am_Connection and establishes
 * the parent socket as a listener for new connections.
 */
Am_Connection *Am_Connection::Open(void)
{
  /*
   * Variable initialization
   */
  struct sockaddr_in in_sa;      /* Internet socket addr. structure */
  Am_Connection * p_connection;
  p_connection= new Am_Connection;
  p_connection->data= new Am_Connection_Data;

  Add_Connection(p_connection);


  p_connection->Set_Waiting_Flag(true);
  if (m_parent_socket == 0)
  {
      /* Put host's address and addresss type into socket structure */
      in_sa.sin_addr.s_addr=INADDR_ANY;
      in_sa.sin_family = PF_INET;
      /* Put the port into the socket structure */
      in_sa.sin_port=Am_SOCKET_PORT;

      if ((m_parent_socket =
	   socket(PF_INET,SOCK_STREAM, 0)) < 0)
	{
	  perror ("socket");
	  p_connection->Set_Valid_Flag(false);
	  // InvalidateAll();
	}
      else // m_parent_socket valid
	{
	  max_socket=m_parent_socket;
	  int flag=1;
	  if (setsockopt(m_parent_socket, SOL_SOCKET,
			 SO_REUSEADDR, &flag, sizeof(int)))
	    {
	      perror("setsockopt (SO_REUSE_ADDR)");
	      p_connection->Set_Valid_Flag(false);
	    }
	  int count =0;
	  // bind my socket to the specified port
	  int error_code=0;
	  if((error_code=bind(m_parent_socket,(struct sockaddr*)&in_sa,
			      sizeof (in_sa))) < 0)
	    {
	      perror ("fallback bind");
	      count++;
	      //p_connection->Set_Valid_Flag(false);
	      // Invalidate_All();
	    }// need to add check for failure 7/2/97
	  //Set the maximum number of connections we
	  //will fall behind

	  if ((listen (m_parent_socket, BACKLOG)) < 0)
	    {
	     std::cerr << "Listen failed\n";
	      //p_connection->Set_Valid_Flag(false);
	      //Invalidate_All();
	    }
	} //m_parent_socket valid
    }// If parent socket == 0
  else // Parent Socket initialized
    close(p_connection->Get_Socket());
  return p_connection;
}


////////////////
bool Am_Connection::Register_Type(Am_Value_Type type, const Am_Marshall_Method& marshall, const Am_Unmarshall_Method& unmarshall, unsigned long id)
{
	bool was_empty=true;

	// Am_Network will generate an ID
	if(id==0)
	{
		id=++id_count;
	}
	else	// The user provided an ID
	{
		// The id provided is already registered!
		if(Net_IDs.GetAt(id) != (0L))
		{
		std::cerr << "Declared duplicate type! Overwriting...";
			was_empty=false;
		}

		id_count=(id>id_count?id:id_count);
	}

	Types.SetAt(type,id);
	Marshall_Methods.SetAt(id,marshall);
	Unmarshall_Methods.SetAt(id,unmarshall);

	return(was_empty);
}


void Am_Connection::Register_Default_Methods(void)
{

  Register_Type(Am_CHAR,Marshall_char,Unmarshall_char);
  //Is using int for short wise?
  Register_Type(Am_INT,Marshall_int16,Unmarshall_int16);
  Register_Type(Am_LONG, Marshall_int32, Unmarshall_int32);
  Register_Type(Am_BOOL, Marshall_bool, Unmarshall_bool);
  Register_Type(Am_FLOAT,Marshall_float,Unmarshall_float);
  Register_Type(Am_DOUBLE,Marshall_double, Unmarshall_double);
  Register_Type(Am_STRING,Marshall_string, Unmarshall_string);
  Register_Type(Am_VALUE_LIST,Marshall_list, Unmarshall_list);
  Register_Type(Am_OBJECT,Marshall_object, Unmarshall_object);
  // Register_Type(Am_NET_OBJECT_OK, Marshall_object_body, UnMarshall_object_body);
}

void Am_Connection::Delete_All_Connections (void)
{
  if (connection_list!=(0L))
    {
    }
  Connection_List_Ptr place_holder;
  while (connection_list!=(0L))
    {
      connection_list->connection_ptr->Close();
      place_holder=connection_list;
      connection_list=connection_list->next;
      delete place_holder;
    }
}


void Am_Connection::Handle_Sockets(fd_set *readfds)
{
  if (!num_sockets)
    return;
  else
    {

      Connection_List_Ptr local_list;
      local_list=connection_list;
   ////
   //// Check Connections for incomming data
   ////
      while (local_list!=(0L))
	{
	  if ((local_list->connection_ptr->Connected()) &&
	      (FD_ISSET((local_list->connection_ptr->Get_Socket()),readfds)))
	    {
	      local_list->connection_ptr->Handle_Input();
	    }
	  local_list=local_list->next;
	}
   ////
   //// Check Parent socket for new connections
   ////
/* This assumes that we don't need each side to specify the other.
 * The first waiting connection will get the first caller.
 * A better way might be to have the Handle sockets check the address of
 * the caller.
 */
      if ((FD_ISSET(m_parent_socket, readfds)) && (m_parent_socket!=0))
	{
	  local_list=connection_list;
	  while ((local_list !=(0L)) &&
		 (!(local_list->connection_ptr->Waiting())) )
	    {
	      local_list=local_list->next;
	    }
	  if (local_list == (0L))
	   std::cerr  << "C:hs: remote connection received without receiving"
		  << "connection waiting\n" <<std::flush;
	  else
	    {
#ifdef _MIPS_SIM_ABI64
              int i;
#else
              socklen_t i;
#endif
	      int temp_socket;
	      struct sockaddr_in sa;
	      i = sizeof (sa);
	      if ((temp_socket=
		accept(m_parent_socket, (struct sockaddr*)&sa, &i)) < 0)
		{
		  perror ("accept");
		}
	      else
		{
		  local_list->connection_ptr->Set_Socket(temp_socket);
		  local_list->connection_ptr->Set_Waiting_Flag(false);
		  local_list->connection_ptr->Set_Connect_Flag(true);
		}

	    }// Found new connection
	}// someone trying to connect to parent
    }// There is at least 1 connection
}


int Am_Connection::Merge_Mask(fd_set *fds)
{
  Connection_List_Ptr place_holder;
  FD_SET(m_parent_socket,fds);
  if (num_sockets!=0)
    {
      place_holder=connection_list;
      while (place_holder != (0L))
        {
;
//                     fds) <<std::flush;
          if (place_holder->connection_ptr->Connected())
            {
              FD_SET(place_holder->connection_ptr->Get_Socket(), fds);
              if (place_holder->connection_ptr->Get_Socket() > max_socket)
                max_socket=place_holder->connection_ptr->Get_Socket();
            }
          //else
          place_holder=place_holder->next;
        }
    }
  return max_socket+1;
}

bool Am_Connection::Sockets_Have_Input(fd_set *fds)
{
	unsigned int index;
	for(index=0; index < NFDBITS; index++)
	{
		if(FD_ISSET(index, socket_flags) && FD_ISSET(index, fds))
		{
			return(true);
		}
	}

	return(false);
}



/////////////////////////////////////////////////////////////////////////
// Dynamic (Instance) Methods
//////////////////////////////


void Am_Connection::Handle_Input(void)
{
  unsigned long net_type,type;
  int i;
  if (!this->Valid())
    Am_Error ("Invalid Connection called to handle input!");
  if (!data->connected)
    Am_Error("C:HandIn:Error! Not connected!");
  else
    {
      Am_Value in_value;
      i=recv(data->m_socket, &net_type,sizeof(net_type),0);
      type=ntohl(net_type);
      Am_Unmarshall_Method handler=Am_Connection::Unmarshall_Methods.GetAt(type);
      if (handler == (0L))
	{
	 std::cerr << "C:hi:Unknown type received!\n" <<std::flush;
	  in_value=Am_No_Value;
	  Set_Connect_Flag(false);
	}
      else //handler is not (0L)
	{
	  in_value=handler.Call(data->m_socket, this);
	  data->Value_Queue.Add(in_value);
	  if ((data->my_receiver != (0L)) )
	    data->my_receiver.Call(this);
	}
    }//Connected
}

bool Am_Connection::Register_Receiver(Am_Receive_Method &receiver)
{
  data->my_receiver=receiver;
  return true;
}



//Receive Simply takes the next value from the queue. If there is no value,
// it returns Am_No_Value.

Am_Value Am_Connection::Receive(void)
{
  Am_Value first_value=data->Value_Queue.Pop();
  return first_value;
}


void Am_Connection::Send(Am_Value value)
{
  while ((data != (0L))&&(!(data->connected)))
    {
      Am_Connection::Wait_For_Connect();
    }
  Am_Marshall_Method Marshaller;
  unsigned long type, net_type;

  type=Am_Connection::Types.GetAt(value.type);
  if (type==Am_NONE)
    Am_Error("Send:Tried to send unknown type!");
  else
    {
      net_type=htonl(type);
      send(data->m_socket, &net_type,sizeof(net_type),0);
      Marshaller=(Am_Connection::Marshall_Methods.GetAt(type));
      Marshaller.Call(data->m_socket, value, this);
    }
}

//
// Blocks until connection arrives. Use with caution.
//
void Am_Connection::Wait_For_Connect(void)
{
  struct timeval select_timeout;
  select_timeout.tv_sec=1;
  select_timeout.tv_usec=0;

  fd_set *read_fd, read_fd_data;
  int nfds, status;
  read_fd=&read_fd_data;
  FD_ZERO(read_fd);
  FD_SET(m_parent_socket,read_fd);

  nfds=m_parent_socket+1;

  status = select (nfds, read_fd, (0L), NULL, &select_timeout);

  if (FD_ISSET(m_parent_socket,read_fd))
    Am_Connection::Handle_Sockets(read_fd);
  delete read_fd;
}



void Am_Connection::Close(void)
{
  close (data->m_socket);
}

int Am_Connection::Get_Socket(void)
{
  return data->m_socket;
}

bool Am_Connection::Connected(void)
{
  return data->connected;
}

bool Am_Connection::Valid(void)
{
  return (data?true:false);
}



void Am_Connection::Set_Socket(int the_socket)
{
  if (the_socket >= 0)
    data->m_socket=the_socket;
  FD_SET(the_socket,socket_flags);
}

void Am_Connection::Set_Connect_Flag(bool state)
{
  if (data)
    data->connected=state;
}

void Am_Connection::Set_Valid_Flag(bool state)
{
  if (!state)
    data=(0L);
}

bool Am_Connection::Waiting()
{
  return data->waiting;
}

void Am_Connection::Set_Waiting_Flag(bool state)
{
  if (data)
    data->waiting=state;
  else
    data->waiting=false;
}



/////////////////////////////////////////////////////////////////
// Am_Connection_Data Definitions

AM_WRAPPER_DATA_IMPL (Am_Connection, (this))

Am_Connection_Data::Am_Connection_Data(void)
{
  m_socket=1;                   // Stdout on unix, shouldn't be used anyway
  connected=false;              // Connection flag: Am I Connected?
  waiting=false;                // Am I waiting to accept an incomming conn?
  Value_Queue.Make_Empty();     // Processed incoming data (None yet)
  List_Queue=(0L);              // Lists being read in
  Object_Queue=(0L);            // Objects being read in
}

Am_Connection_Data::Am_Connection_Data(Am_Connection_Data* prototype)
{
  m_socket=prototype->m_socket;
  connected=prototype->connected;
  waiting=false;
  Value_Queue=prototype->Value_Queue;
  List_Queue=(0L);              // Lists being read in
  Object_Queue=(0L);            // Objects being read in
}

int Am_Connection_Data::operator== (Am_Connection_Data& Test_Data)
{
  return (Test_Data.m_socket == m_socket);
}

// DANGER!! We comment this, as the d'tor is declared implicit
// by a macro (yet) and newer compiler won't compile this
// Am_Connection_Data::~Am_Connection_Data(void)
// {
//  close (m_socket);
//  Value_Queue.Make_Empty();
// }

void Am_Connection::Add_Connection(Am_Connection *new_connection)
{
  Connection_List_Ptr new_node, place_holder;
  new_node = new (Connection_List_Type);
  new_node->connection_ptr=new_connection;

  // Add new node to head of list
  place_holder=new_node;
  place_holder->next=connection_list;
  connection_list=place_holder;
  num_sockets++;

}

void Am_Connection::Delete_Connection(int sock)
{
  Connection_List_Ptr place_holder, old_place_holder;
  old_place_holder=place_holder=connection_list;
  if (place_holder == (0L))
    {
    }
  else
    {
      if ((place_holder->connection_ptr->Get_Socket() == sock))
	{
	  connection_list=place_holder->next;
	  delete (place_holder);
	}
      else
	{
	  while ( (place_holder != (0L)) &&
		  (place_holder->connection_ptr->Get_Socket() != sock))
	    {
	      old_place_holder=place_holder;
	      place_holder=place_holder->next;
	    }
	  if (place_holder !=(0L))
	    {
	      old_place_holder->next=place_holder->next;
	      delete (place_holder);
	    }
	  else
	    {
	    }
	}
    }
}


void Am_Connection::Delete_Connection(Am_Connection *doomed_connection_ptr)
{
  Connection_List_Ptr place_holder, old_place_holder;
  old_place_holder=place_holder=connection_list;
  if (place_holder == (0L))
    {
    }
  else
    {
      if  (place_holder->connection_ptr == doomed_connection_ptr)
	{
	  connection_list=place_holder->next;
	  delete (place_holder);
	}
      else
	{
	  while ( (place_holder != (0L)) &&
		  (place_holder->connection_ptr != doomed_connection_ptr))
	    {
	      old_place_holder=place_holder;
	      place_holder=place_holder->next;
	    }
	  if (place_holder !=(0L))
	    {
	      old_place_holder->next=place_holder->next;
	      delete (place_holder->connection_ptr);
	      delete (place_holder);
	      num_sockets--;
	    }
	  else
	    {
	    }
	}
    }
}


void Am_Connection::Delete_Connection(Am_Connection &doomed_connection_ref)
{
  Delete_Connection( &doomed_connection_ref);
}

///////////////////////////////////////////////////////////////////
// Lists
////////////////////////////

//
// List Receiver Methods
//

Am_Define_Method (Am_Receive_Method, void, receive_list_length,
		  (Am_Connection *my_connection_ptr))
{
  //  Am_Value val=my_connection_ptr->Receive(); // Clear null value;
  Am_Value val=my_connection_ptr->Receive();
  long unsigned int length=(int)val;
  my_connection_ptr->Read_List(length); // receiver="receive_list_items"
}

Am_Define_Method (Am_Receive_Method, void, receive_list_items,
		  (Am_Connection *my_connection_ptr))
{
  Am_Value val=my_connection_ptr->Receive();
  my_connection_ptr->Add_To_Current_List(val);
  my_connection_ptr->Decrement_Check_List();
}

//
// List processing methods
//

void Am_Connection::Read_Length_and_List()
{
  Incomplete_List_Ptr new_partial_list_ptr=new Incomplete_List_Type;
  //  new_partial_list_ptr->list=Am_Value_List::Empty_List();
  new_partial_list_ptr->prev_receiver=data->my_receiver;
  new_partial_list_ptr->next=data->List_Queue;
  data->List_Queue=new_partial_list_ptr;
  data->my_receiver=receive_list_length;
}


void Am_Connection::Read_List(unsigned long int length)
{
  data->my_receiver=receive_list_items;
  data->List_Queue->length=length;
}


void Am_Connection::Add_To_Current_List(Am_Value val)
{
  data->List_Queue->list.Add(val);
}

void Am_Connection::Decrement_Check_List()
{
  if ((--(data->List_Queue->length)) == 0)
    {
      data->Value_Queue.Push(data->List_Queue->list);
      data->my_receiver=data->List_Queue->prev_receiver;
      Incomplete_List_Ptr temp=data->List_Queue;
      data->List_Queue=data->List_Queue->next;
      delete(temp);
      data->my_receiver.Call(this); // Now that we have the list, let the
                                    // user fn take it.
    }
}

//
// Stores the local objects from which incoming network objects are created.
//
bool Am_Connection::Register_Prototype (Am_String name, Am_Object & obj)
{
  Am_String old_name;
  Am_Object old_obj;
  if((old_obj=Net_Prototypes.Assoc_2((Am_Value)name)) != Am_Value(0))
    {
      return false;
    }
  else // name not taken
    if ((old_name=Net_Prototypes.Assoc_2(obj))!= Am_No_String)
      {
	return false;
      }
    else // neither name not object previously registered.
      {
	Am_Assoc proto_reg((Am_Value)name,obj);
	Net_Prototypes.Add(proto_reg);
      }
  return true;
}


const char * Am_Connection::Get_Net_Proto_Name(Am_Object &the_obj)
{
  Am_String name;
  Am_Object proto_obj=the_obj.Get_Prototype();
  name=Net_Prototypes.Assoc_1(proto_obj);
  return name;
}


Am_Object Am_Connection::Get_Net_Prototype(const char * proto_name)
{
  Am_Object obj;
  obj=Net_Prototypes.Assoc_2(proto_name);
  return obj;
}

const char * Am_Connection::Get_Net_Object_ID(Am_Object &the_obj)
{
  return (Am_String)Net_Instances.Assoc_1(the_obj);
}

Am_Object  Am_Connection::Get_Net_Instance(const char *proto_name,
					   int instance_num)
{
  Am_Object obj;
  obj=Net_Instances.Assoc_2(Net_ID_Str(proto_name, instance_num));
  return obj;
}

void Am_Connection::Set_Net_Instance(Am_Object &the_obj,
				       const char * proto_name, int instance_num)
{
  Am_Assoc instance_reg(Net_ID_Str(proto_name, instance_num),the_obj);
  Net_Instances.Add(instance_reg);
}


int Am_Connection::Num_Instances(void)
{
  return Net_Instances.Length();
}

Am_Define_Method (Am_Receive_Method, void, receive_obj,
		  (Am_Connection *my_connection_ptr))
{
  Am_Value val=my_connection_ptr->Receive();
  if (Am_No_Object==(Am_Object)val)
    {
      my_connection_ptr->Receive_Object_Slots();
    }
  else
    {
     std::cerr << "I expected Unmarshall Object's bogus return value and got ";
     std::cerr << val << " instead! \n";
      Am_Error("Mangled incoming object!");
    }
}

Am_Define_Method (Am_Receive_Method, void, receive_obj_slots,
		  (Am_Connection *my_connection_ptr))
{
  Am_Value val=my_connection_ptr->Receive();
  my_connection_ptr->Add_To_Current_Object(val);
  my_connection_ptr->Check_Object();
}

void null_slot_demon (Am_Slot slot)
{
  Am_Object_Advanced self = slot.Get_Owner ();
}


void Am_Connection::Receive_Object(Am_Object &the_obj)
{
  //
  //This implements a "latest received" policy.
  //It enforces that there will only be one copy of an object being
  // built at a time.
  //
  Incomplete_Object_Ptr new_partial_obj_ptr, prev_ptr;
  new_partial_obj_ptr=data->Object_Queue;
  while((new_partial_obj_ptr != (0L))&&(new_partial_obj_ptr->obj != the_obj))
    {
      prev_ptr=new_partial_obj_ptr;
      new_partial_obj_ptr=new_partial_obj_ptr->next;
    }
  if (new_partial_obj_ptr==(0L))
    new_partial_obj_ptr=new Incomplete_Object_Type;
  else
    //Move this object to the head of the list and restart
    //building it from scratch.
    prev_ptr->next=new_partial_obj_ptr->next;

  //Risk: If objects come in fast and the demons
  // do not fire soon after an object is received...
  // Might need to maintain multiple object states to ensure integrety
  // During simultaneos Send and Receives.
  new_partial_obj_ptr->obj=the_obj;
  // Declaw demon during receive
  Am_Value_List slot_list=the_obj.Get(Am_SLOTS_TO_SAVE);
  Am_Assoc blessing((Am_Ptr)this,(slot_list.Length()));
  new_partial_obj_ptr->obj.Set(Am_NET_BLESSING,blessing);

  new_partial_obj_ptr->bogus=true;
  new_partial_obj_ptr->slot_list=the_obj.Get(Am_SLOTS_TO_SAVE);

  new_partial_obj_ptr->prev_receiver=data->my_receiver;
  data->my_receiver=receive_obj;

  new_partial_obj_ptr->next=data->Object_Queue;
  data->Object_Queue=new_partial_obj_ptr;

}

void Am_Connection::Receive_Object_Slots(void)
{
  data->my_receiver=receive_obj_slots;
}

void Am_Connection::Add_To_Current_Object(Am_Value val)
{
  // converts short to char overflow @ 256
  Am_Slot_Key curr_slot=(int)data->Object_Queue->slot_list.Pop();
  data->Object_Queue->obj.Add(curr_slot,val,Am_OK_IF_THERE);
}

void Am_Connection::Check_Object()
{
  if (data->Object_Queue->slot_list.Length() == 0)
    {
      data->Value_Queue.Push(data->Object_Queue->obj);
      data->my_receiver=data->Object_Queue->prev_receiver;

      Incomplete_Object_Ptr temp=data->Object_Queue;
      data->Object_Queue=data->Object_Queue->next;
      delete(temp);
      data->my_receiver.Call(this); // Now that we have the object, let the
                                    // user fn take it.
    }
}

