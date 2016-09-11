/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <amulet.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

using namespace std;

#define MAXHOSTNAME 32

Am_Object rectangle;

char remote_host_data[MAXHOSTNAME], *remote_host;
Am_Connection *mc=(0L);


//
// This method keeps a live connection.
// Also need to test opening new connection each time, opening multiple
// connections and handling a break from one side.
//


Am_Define_Method (Am_Object_Method, void, net_obj_method, (Am_Object cmdobj))
{
  cmdobj=cmdobj;
 
  //cout << "t1\n";
  Am_Value val=(Am_Value)rectangle;
  mc->Send(val);
  //cout << "t2\n" << flush;
}


Am_Define_Method (Am_Receive_Method, void, test_receive, 
		  (Am_Connection *my_connection_ptr))
{
  Am_Value val=my_connection_ptr->Receive();
  cout << "testnobj:Recieved "<< val <<endl << flush;

  Am_Object sent_obj=(Am_Object)val;
  rectangle.Set(Am_TOP,sent_obj.Get(Am_TOP));
  rectangle.Set(Am_LEFT,sent_obj.Get(Am_LEFT));

}

int main (int argc, char *argv[])
{
  Am_Initialize ();

  cout << argc << endl;
  if (argc !=2)
    {
      remote_host=remote_host_data;
      sprintf(remote_host,"basalt.amulet.cs.cmu.edu");
    }
  else
    remote_host=argv[1];
  
  mc=Am_Connection::Open(remote_host);
  mc->Register_Receiver(test_receive);

  Am_Object my_win = Am_Window.Create ("my_win")
    .Set (Am_LEFT, 20)
    .Set (Am_TOP, 50)
    .Set (Am_HEIGHT, 200)
    .Set (Am_WIDTH, 200);
 
  Am_Screen.Add_Part (my_win);


  Am_Object proto_rect=Am_Rectangle.Create("proto rect")
    .Set(Am_HEIGHT,20).Set(Am_WIDTH,20)
    .Add (Am_SLOTS_TO_SAVE, Am_Value_List()
          .Add(Am_LEFT).Add(Am_TOP));

  Am_Connection::Register_Prototype("net_rect",proto_rect);
  cout << Am_Connection::Get_Net_Proto_Name(proto_rect) << "#\n";
  cout << Am_Connection::Get_Net_Prototype("net_rect") << "#\n";


  rectangle=proto_rect.Create("rect_inst")
    .Set(Am_TOP,10).Set(Am_LEFT,10);


  Am_Object mg_inter_inst=Am_Move_Grow_Interactor.Create("mover");
    //    .Set(Am_DO_METHOD,edit_update_method);

  my_win.Add_Part(mg_inter_inst);
  my_win.Add_Part(rectangle);





  Am_Object send_cmd = Am_Command.Create("send cmd")
	  .Set (Am_LABEL,"Send Object!")
	  .Set (Am_DO_METHOD, net_obj_method);

  Am_Object send_button = Am_Button.Create("send_button")
	.Set (Am_LEFT, 20)
	.Set (Am_TOP, 50)
	.Set (Am_COMMAND, send_cmd);
  
  Am_Object quit_button = Am_Button.Create("quit_button")
	.Set (Am_LEFT, 20)
	.Set (Am_TOP, 140)
	.Set (Am_COMMAND, Am_Quit_No_Ask_Command);
  my_win.Add_Part(send_button);
  my_win.Add_Part(quit_button);

 
  /* ************************************************************ */
  /* End user code                                                */
  /* ************************************************************ */
  
  Am_Main_Event_Loop ();
  Am_Cleanup ();

  return 0;
}


