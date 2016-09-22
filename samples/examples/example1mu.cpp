/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/


/* Example1: Simple editor for creating objects.  Demonstrates:
   1) How to organize objects in groups
   2) How to attach interactors and widgets to groups so they only operate on
      the correct objects 
   3) How to enable and disable interactors based on a global palette
   4) Use of the menubar, standard, built-in command objects,
      and the standard selection handles widget.
   5) How to use the standard load/save mechanism

Important points:
   * Collect different types of objects into their own groups.  In
     particular, the interactors are attached to the created_objs group
	 so they don't operate on the feedback objects, etc.
   * Use of standard command objects.
   * Create named slots of windows for the parts we need access to
     so we don't need to use global variables to access the objects.
	 Alternatively, could use a global variable for things like tool_panel
	 instead of self.Get_Object(Am_WINDOW).Get_Object(TOOL_PANEL).
*/   

#include <amulet.h>

#include <am_inc.h>                    /* supports multiple users */
#include <amulet/inter_advanced.h>     /* supports multiple users */
#include <amulet/widgets_advanced.h>   /* supports multiple users for HANDLE_SIZE */
#include <amulet/opal.h>               /* supports multiple users for Am_Polygon */

Am_Object window;

#include <amulet/pebbles_amulet.h>

//file that holds the bitmap arrow picture
#define ARROW_BITMAP "lib/images/arrow.gif"

using namespace std;

//supports multiple users
// this function builds a list of handle shapes that can be drawn for each
// user selection widget
Am_Object Am_Diamond;

void Build_Handles(void)
{
  Am_Diamond = Am_Polygon.Create("Am_Diamond");
  Am_Point_List p_list;
  int hsd2 = HANDLE_SIZE_D2;  //debug
  int hs   = HANDLE_SIZE;     //debug
  p_list.Add(0, HANDLE_SIZE_D2)             //left
        .Add(HANDLE_SIZE_D2, 0)             //top
        .Add(HANDLE_SIZE, HANDLE_SIZE_D2)   //right
        .Add(HANDLE_SIZE_D2, HANDLE_SIZE);  //bottom
         
  Am_Diamond.Set(Am_POINT_LIST,p_list);
}

//Make the prototypes put into the tool panel be globals so
//can see what mode we are in using constraints.  An alternative
//would be to have the items in the button panel be a list of
//command objects, with these as the Am_LABEL of the commands,
//and then pick constants as the Am_ID of each command, and
//test the button panel's value with be the appropriate ID.
//The advantage of using the ID is that it will work if there
//are multiple windows.

Am_Object arrow_bm, rect_proto;

//slot name of slot to put the tool_panel object in so easy to find
Am_Slot_Key TOOL_PANEL = Am_Register_Slot_Name ("TOOL_PANEL");

//Created_objs is the group that will contain all the objects
//to be created.  Define a slot in the window so can find it.
//Alternatively, the created_objs object could just be a global variable. 
Am_Slot_Key CREATED_OBJS = Am_Register_Slot_Name ("CREATED_OBJS");

Am_Define_Formula (bool, is_rect_mode_selected) {
  //go to my window, get the tool_panel object out of it, and get
  //the value of the tool_panel.
  Am_Object mode = self.Get_Object(Am_WINDOW)
      .Get_Object(TOOL_PANEL).Get(Am_VALUE);
  if (mode == rect_proto) return true;
  else return false;
}

Am_Define_Formula (bool, is_arrow_selected) {
  Am_Object mode = self.Get_Object(Am_WINDOW)
      .Get_Object(TOOL_PANEL).Get(Am_VALUE);
  if (mode == arrow_bm) return true;
  else return false;
}


//Formulas for the size of the scroll region.  The scroller's owner
//is the window.  Get its height, subtract my top, and subtract 10 to
//leave a 10 pixel border on the bottom. 
Am_Define_Formula(int, scroll_height) {
  return (int)self.Get_Owner().Get(Am_HEIGHT) - 10 - (int)self.Get(Am_TOP);
}
Am_Define_Formula(int, scroll_width) {
  return (int)self.Get_Owner().Get(Am_WIDTH) - 10 - (int)self.Get(Am_LEFT);
}

// Am_Create_New_Object_Method for new object command.
// old_object is passed in when selective repeat is enabled.  Since
// we are not using selective repeat, don't need to use old_object
Am_Define_Method(Am_Create_New_Object_Method, Am_Object, create_new_object,
		 (Am_Object inter, Am_Inter_Location data,
		  Am_Object /* old_object */)) {
  
  //if necessary, could get the particular value of the tool panel
  //by getting the window of the inter, and the tool_panel of the
  //window. Here, since only creating one kind of thing, don't need
  //to know the tool panel value.

  Am_Object ref_obj;
  int left, top, width, height;
  //if could create both lines and rectangles, then would have to 
  //check if the data is a line type vs. a rect type, but here we
  //know it will always be a rect type.
  bool create_line;
  data.Get_Location(create_line, ref_obj, left, top, width, height);

  //Get the group to put the created object into
  Am_Object created_objs = inter.Get_Object(Am_WINDOW).Get(CREATED_OBJS);

  //All coordinates must be with respect to some group.  Make sure that
  //the coordinates for the new object are with respect to the correct
  //group.
  if (ref_obj != created_objs) 
    Am_Translate_Coordinates(ref_obj, left, top, created_objs, left, top);

  Am_Object new_obj = rect_proto.Create()
    .Set(Am_LEFT, left)
    .Set(Am_TOP, top)
    .Set(Am_WIDTH, width)
    .Set(Am_HEIGHT, height);
  created_objs.Add_Part(new_obj);
  return new_obj;
}

// This method should take the supplied contents list and add it to
// the window, after removing what is already there
Am_Define_Method (Am_Handle_Loaded_Items_Method, void, use_file_contents,
		  (Am_Object command, Am_Value_List &contents)) {
  //Get the group to put the created object into.  Every command will
  //have a Am_SAVED_OLD_OWNER which is set with the widget or
  //interactor the command is attached to.  Get the CREATED_OBJS group
  //out of that widget's window.
  Am_Object created_objs = command.Get_Object(Am_SAVED_OLD_OWNER)
    .Get_Object(Am_WINDOW).Get(CREATED_OBJS);
  Am_Value_List current = created_objs.Get (Am_GRAPHICAL_PARTS);
  Am_Object item;
  //first delete all of the current contents of the window
  for (current.Start (); !current.Last (); current.Next ()) {
    item = current.Get ();
    item.Destroy ();
  }
  //now add the new objects
  for (contents.Start (); !contents.Last (); contents.Next ()) {
    item = contents.Get ();
    created_objs.Add_Part (item);
  }
}

//This method should return the list of objects to save
Am_Define_Method (Am_Items_To_Save_Method, Am_Value_List, contents_for_save,
		  (Am_Object command)) {
  Am_Object created_objs = command.Get_Object(Am_SAVED_OLD_OWNER)
    .Get_Object(Am_WINDOW).Get(CREATED_OBJS);
  Am_Value_List obs_to_save = created_objs.Get (Am_GRAPHICAL_PARTS);
  return obs_to_save;
}

// this function supports multiple users
// this sets up the selection widgets for a single user
// it adds them to the scroller and into my_selection_list
// user_id_int is an int to make it easier to make lables

static int user_id_int=0;  // this is a counter, one of each user

static void add_selection_widgets_for_user(Am_Object& scroller, 
           Am_Value_List& my_selection_list, const Am_Object& created_objs,
           const Am_Value user_id_value )
{
  Am_Style fill_style;  char sw_name[20] = "selection_widget_"; char sw_id[2] = {0};
  Am_Object poly = 0;

  if (user_id_int > 8) 
    Am_Error("probably too many users");

  switch (user_id_int) {
    case 0: 
            fill_style = Am_Black;              sw_id[0] = '0'; 
            poly = Am_Diamond;                  
            break;
    case 1: fill_style = Am_Green;              sw_id[0] = '1'; break;
    case 2: fill_style = Am_Blue;               sw_id[0] = '2'; break;
    case 3: fill_style = Am_Purple;             sw_id[0] = '3'; break;
    case 4: fill_style = Am_Cyan;               sw_id[0] = '4'; break;
    case 5: fill_style = Am_Orange;             sw_id[0] = '5'; break;
    case 6: fill_style = Am_Amulet_Purple;      sw_id[0] = '6'; break;
    case 7: fill_style = Am_Motif_Light_Green;  sw_id[0] = '7'; break;
    case 8: fill_style = Am_Motif_Light_Orange; sw_id[0] = '8'; break;

    default:  fill_style = Am_Motif_Gray;       sw_id[0] = 'X'; break;
  }

  strcat(sw_name, sw_id);
  //add an instance of the standard selection widget
  //It needs the specific group of objects to select in, and 
  //that's the created_objs group.  Put a constraint into its
  //Am_ACTIVE slot that makes it operational only when the
  //arrow is selected in the tool panel
  
  Am_Object my_selection = Am_Selection_Widget.Create(sw_name)
    .Set(Am_OPERATES_ON, created_objs)
    .Set(Am_ACTIVE, is_arrow_selected)
    .Set(Am_USER_ID, user_id_value)
    .Set(Am_FILL_STYLE, fill_style)
    ;
  if (poly.Valid())
    my_selection.Add(Am_SPECIAL_HANDLE, poly);
  
  scroller.Add_Part(my_selection); 
  my_selection_list.Add(my_selection);
  
  user_id_int++;
}

#define MULTIUSER_TEST 1

// begin supports multiple user test
#ifdef MULTIUSER_TEST
Am_Define_Method( Am_Object_Method, void, set_user_do, (Am_Object cmd) ) {
  Am_Value value;
  value=cmd.Peek(Am_VALUE);
  //TEMP
  cout << "\n+-+-+- Command " << cmd  << " value = "
       << value << " type ";
  Am_Print_Type(cout, value.type);
  cout << " Saved_Old_Owner=" << cmd.Peek(Am_SAVED_OLD_OWNER) << endl << flush;

  if (value == "Am_USER_ZERO")
    Am_User_Id.Set(Am_USER_ID, Am_USER_ZERO);
  else if (value == "3")
    Am_User_Id.Set(Am_USER_ID, 3);
  
  else if (value == "Port:3000")
    Am_User_Id.Set(Am_USER_ID, "Port:3000");
  
  else if (value == "10")
    Am_User_Id.Set(Am_USER_ID, "10");
  
  else if (value == "8000L")
    Am_User_Id.Set(Am_USER_ID, 8000L);
  
  /* else we've clicked on the menu itself and nothing needs changing */
}
#endif
// send supports multiple user test



Am_Define_Method( Am_Object_Method, void, change_setting, (Am_Object cmd) )
{
  Am_Object inter = cmd.Get_Owner();
  Am_Input_Char c = inter.Get( Am_START_CHAR );

  cout << "---- got " << c << endl;
  switch( c.As_Char() )
  {
  case 'i': {
    static bool tracing = false;
    if (tracing) Am_Set_Inter_Trace(Am_INTER_TRACE_NONE);
    else Am_Set_Inter_Trace(Am_INTER_TRACE_ALL);
    tracing = !tracing;
    break;
  }
  case 'p': {
    int port_number;
	cout << "Port Number to open: " << flush;
	cin >> port_number;
	// *** should get a name too
	bool success = Am_Pebbles_Open_Port(port_number, NULL);
	cout << "Open port " << port_number << " success = " << success
		<< endl << flush;
	break;
	}
  default:
    cout << "** Illegal, want:\n"
		 << "   p = add pebbles port\n"
		 << "   i = toggle inter tracing\n"
         << endl << flush;
    break;
  } // end switch
}


int main (int argc, char *argv[])
{
  Am_Initialize(); //All Amulet code starts with this
  
  
  Build_Handles();

  //Create the main window, setting its size.
  window = Am_Window.Create("window")
    .Set(Am_TOP, 50)
    .Set(Am_LEFT, 50)
    .Set(Am_WIDTH, 500)
    .Set(Am_HEIGHT, 400)
    .Set(Am_TITLE, "Amulet Example 1")
    //to support undo, only need to add an undo handler to the window.
    .Set(Am_UNDO_HANDLER, Am_Multiple_Undo_Object.Create("my_multi_undo"))
    // Set the background color so the widgets will look better
    .Set(Am_FILL_STYLE, Am_Amulet_Purple);
  
  //add the window to the screen
  Am_Screen.Add_Part(window);

  //Add a scrolling window to put the graphics in
  Am_Object scroller = Am_Scrolling_Group.Create("scroller")
    .Set (Am_LEFT, 55)
    .Set (Am_TOP, 40)
    //The width and height are of the outer scroll area in the window.
    //The constraints make it be the window's width minus my left and
    //top so the scroller will grow and shrink with the window.
    .Set (Am_WIDTH, scroll_width)
    .Set (Am_HEIGHT, scroll_height)
    //The Inner width and height is the area that the graphics can
    //be in.  If bigger than the width and height, then scroll bars are
    //enabled. 
    .Set (Am_INNER_WIDTH, 1000)  
    .Set (Am_INNER_HEIGHT, 1000)
    //The INNER_FILL_STYLE is the color of the background of the scrolling
    //area.  If not set, then uses the Am_FILL_STYLE color which defaults
    //to Am_Amulet_Purple, which will be the color of the widgets
    .Set (Am_INNER_FILL_STYLE, Am_White)
    ;

  //put the scroller into the window
  window.Add_Part(scroller);

  //Create a group to put the created objects in
  Am_Object created_objs = Am_Group.Create("created_objs")
    //make the bounding box of this group be the same as the inside
    //area of the scrolling group.  Often, the width and height
    //will be a constraint like Am_From_Owner, if the size of the
    //owner is not fixed.  Here, the size of the graphics area is
    //the Am_INNER_WIDTH and Am_INNER_HEIGHT of the scroller. 
    .Set (Am_LEFT, 0)
    .Set (Am_TOP, 0)
    .Set (Am_WIDTH, 1000)  
    .Set (Am_HEIGHT, 1000)
    ;

  //add the created_objs group to the scroller
  scroller.Add_Part(created_objs);
  
  //make the created_objs group be easily accessible by setting it 
  //as a slot of the window.  Don't use add_part since created_objs is
  //already a part of the scroller.
  window.Add(CREATED_OBJS, created_objs);

  // Now create the items to put into the tool panel.
  // First, load a bitmap of the arrow for selection mode
  const char *pathname = Am_Merge_Pathname(ARROW_BITMAP);
  Am_Image_Array arrow = Am_Image_Array(pathname);
  delete [] pathname; //Merge allocates a string, delete it
  if (!arrow.Valid()) Am_Error ("Arrow bitmap image not found");
  arrow_bm = Am_Bitmap.Create("Arrow bitmap")
    .Set (Am_IMAGE, arrow)
    .Set (Am_LINE_STYLE, Am_Black)
    ;

  rect_proto = Am_Rectangle.Create("Rect_Proto")
    .Set (Am_LEFT, 0)
    .Set (Am_TOP, 0)
    .Set (Am_WIDTH, 12)
    .Set (Am_HEIGHT, 8)
    .Set (Am_LINE_STYLE, Am_Black)
    .Set (Am_FILL_STYLE, Am_Red)
    //allow these objects to be saved to disk and read back in using
    //the standard method
    .Add (Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
    //the standard method needs the list of slots that are different
    //in each object, here, just the bounding box 
    .Add (Am_SLOTS_TO_SAVE, Am_Value_List()
	  .Add(Am_LEFT).Add(Am_TOP).Add(Am_WIDTH).Add(Am_HEIGHT))
    ;						   

  //tell the default loader what to call a rect_proto in the file
  Am_Default_Load_Save_Context.Register_Prototype ("RECT_PROTO", rect_proto);

  Am_Object tool_panel = Am_Button_Panel.Create ("tool panel")
    .Set (Am_LEFT, 10) //Position of the toolpanel in the window
    .Set (Am_TOP, 40)
    //make all the items the same height even if different sizes
    .Set (Am_FIXED_HEIGHT, true)
    //normally a button panel doesn't show which item is selected, so
	//turn that on so can see what the mode is
    .Set (Am_FINAL_FEEDBACK_WANTED, true)
    //put the items closer together
    .Set (Am_V_SPACING, -4)
    .Set (Am_ITEMS, Am_Value_List()
	  .Add(arrow_bm)
	  .Add(rect_proto)
	  )
    //start the panel off with the arrow selected
    .Set(Am_VALUE, arrow_bm)
    ;

  //Comment on Am_IMPLEMENTATION_PARENT: Every command is
  //by default queued for undo.  By setting the Am_IMPLEMENTATION_PARENT
  //of the command to true, that makes the command NOT be queued
  //for undo.
  tool_panel.Get_Object(Am_COMMAND)
    .Set (Am_IMPLEMENTATION_PARENT, true) //don't queue for UNDO
    ;
  
  //Be sure the part for the tool panel is labeled so can find it easily.
  window.Add_Part(TOOL_PANEL, tool_panel);

 
  //Create a feedback rectangle	for when creating new objects
  Am_Object rect_feedback = Am_Rectangle.Create ("rect_feedback")
    .Set (Am_FILL_STYLE, 0L) //not filled
    .Set (Am_LINE_STYLE, Am_Dotted_Line)
    .Set (Am_VISIBLE, false) 
    ;
  //don't forget to add the feedback as a part.  Note that not
  //added to the created_objs group since don't want the selection
  //widget to select it.
  scroller.Add_Part(rect_feedback);
  
  Am_Object create_inter = Am_New_Points_Interactor.Create("create_inter")
    .Set(Am_FEEDBACK_OBJECT, rect_feedback)
    //method to create the new rectangles
    .Set(Am_CREATE_NEW_OBJECT_METHOD, create_new_object)
    //this interactor is active when rectangle is selected in the tool panel
    .Set(Am_ACTIVE, is_rect_mode_selected)
    ;
  
  //add interactor as a part of the created_objs group
  created_objs.Add_Part (create_inter);

  // open_command is created as part of the menu_bar, but will need it
  // below, so store it in a local variable.
  Am_Object open_command;

  // begin supports multiple users
  
  // create a list of selection widgets.  The work is done in 
  // add_selection_widgets_for_user
  Am_Value_List selection_widget_list;      

  // set up user 0 for our cursor
  //add_selection_widgets_for_user (scroller, 
  //     selection_widget_list, created_objs, Am_USER_ZERO);
       
  // get list of external user ID's
  Am_Value_List user_id_list;
  user_id_list
    .Add(Am_USER_ZERO)  // otherwise it thinks the console is the last user in this list
    .Add(1)
    .Add("Port:3000")
    .Add("10")
    .Add(8000L); 
       
  // now add the other users' selection widgets
  for (user_id_list.Start(); !user_id_list.Last(); user_id_list.Next() )
     add_selection_widgets_for_user (scroller, 
           selection_widget_list, created_objs, user_id_list.Get());
  //end supports multiple users
  
  //Add an instance of the menu bar widget to the window
  window.Add_Part(Am_Menu_Bar.Create("menu_bar")
      //tell the various commands where the selection widget is
      .Set(Am_SELECTION_WIDGET, selection_widget_list)
	  //All of the other default slots of the menubar are fine, so
	  // just need to set the items list
	  .Set(Am_ITEMS, Am_Value_List ()
	       //menubars take a list of the top-level commands
	       .Add (Am_Command.Create("File_Command")
		     .Set(Am_LABEL, "File")
		     //make sure that if release over the
		     // File menu item itself, that
		     // nothing is put onto the undo history list
		     .Set(Am_IMPLEMENTATION_PARENT, true) //not undoable
		     .Set(Am_ITEMS, Am_Value_List ()
			  //list of items in the File menu.
			  // Use instances of the built-in open and save cmds.
			  // Will use the open_command below, so save
			  // it in a local variable.
			  .Add (open_command = Am_Open_Command.Create()
				//open needs a method to find the objs to save
				.Set(Am_HANDLE_OPEN_SAVE_METHOD,
				     use_file_contents))
			  .Add (Am_Save_As_Command.Create()
				//save needs a method to take the objs and add
				//to the window
				.Set(Am_HANDLE_OPEN_SAVE_METHOD,
				     contents_for_save))
			  .Add (Am_Save_Command.Create()
				.Set(Am_HANDLE_OPEN_SAVE_METHOD,
				     contents_for_save))
			  //Use an instance of the built-in quit command.
			  // No_Ask because it doesn't check whether save or
			  // not if window contents are modified.
			  .Add (Am_Quit_No_Ask_Command.Create())
			  ) //end items
		     ) //end file command
	       .Add (Am_Command.Create("Edit_Command")
		     .Set(Am_LABEL, "Edit")
			 //make sure that if release over the edit itself, that
			 //nothing is put onto the undo history list
		     .Set(Am_IMPLEMENTATION_PARENT, true) //not undoable
		     .Set(Am_ITEMS, Am_Value_List ()
		           //list of items in the edit menu.
			   //Use instances of the undo and redo commands 
			  .Add (Am_Undo_Command.Create())
			  .Add (Am_Redo_Command.Create())
			  .Add (Am_Menu_Line_Command.Create())
			  //Cut, Copy, paste. commands
			  //These commands will find the selection widget in
			  //the menu_bar since none is provided here.
			  .Add (Am_Graphics_Cut_Command.Create())
			  .Add (Am_Graphics_Copy_Command.Create())
			  .Add (Am_Graphics_Paste_Command.Create())
			  )	//end items
		     ) //end edit command
// begin supports multiple user test
#ifdef MULTIUSER_TEST
	       .Add (Am_Command.Create("User_Command")
		     .Set(Am_LABEL, "User")
		     .Set(Am_IMPLEMENTATION_PARENT, true) //not undoable
		     .Set(Am_DO_METHOD, set_user_do)
		     .Set(Am_ITEMS, Am_Value_List ()
                .Add (Am_Command.Create("Am_USER_ZERO_Command")
                   .Set(Am_LABEL, "Am_USER_ZERO")
                   .Set(Am_ACCELERATOR, "CONTROL_0")
                   //.Add(Am_CHECKED_ITEM, true)
                   .Set(Am_DO_METHOD, set_user_do))
                 .Add (Am_Command.Create("Am_USER_3_Command")
                    .Set(Am_LABEL, "3")
                    .Set(Am_ACCELERATOR, "CONTROL_1")
                    //.Add(Am_CHECKED_ITEM, false)
                    .Set(Am_DO_METHOD, set_user_do))
                 .Add (Am_Command.Create("Am_USER_Port:3000_Command")
                    .Set(Am_LABEL, "Port:3000")
                    .Set(Am_ACCELERATOR, "CONTROL_2")
                    //.Add(Am_CHECKED_ITEM, false)
                    .Set(Am_DO_METHOD, set_user_do))
                  .Add (Am_Command.Create("Am_USER_10_Command")
                     .Set(Am_LABEL, 10)
                     .Set(Am_ACCELERATOR, "CONTROL_3")
                     //.Add(Am_CHECKED_ITEM, false)
                     .Set(Am_DO_METHOD, set_user_do))
                  .Add (Am_Command.Create("Am_USER_8000L_Command")
                     .Set(Am_LABEL, "8000L")
                     .Set(Am_ACCELERATOR, "CONTROL_4")
                     //.Add(Am_CHECKED_ITEM, false)
                     .Set(Am_DO_METHOD, set_user_do))
              )//end items
             )//end user command           
#endif
// end supports multiple user test
		     
	       ) //end add_part
	  )
  ;

  // this allows (under Unix and DOS Prompt) the file to be opened
  // when the program is started up, by doing typing like the
  // following on the command line:
  //    example1 filename.sav
  if (argc > 1) {
    Am_String s = (char *)argv[1];
    Am_Standard_Open_From_Filename(open_command, s);
  }
  

  Am_Object change_settings_inter =
    Am_One_Shot_Interactor.Create("change_settings")
    .Set(Am_START_WHEN, "ANY_KEYBOARD")
    .Set(Am_PRIORITY, 0.9)  // so it runs after the menu accelerators
    ;
  change_settings_inter.Get_Object(Am_COMMAND)
    .Set(Am_DO_METHOD, change_setting)
    .Set(Am_IMPLEMENTATION_PARENT, true) //not undo-able
    .Set_Name("change_settings_command")
    ;
  window.Add_Part (change_settings_inter);

  Am_Pebbles_Init(window);

  Am_Main_Event_Loop(); //Now process input events

  Am_Pebbles_Quit_Port();

  Am_Cleanup ();  //All Amulet code ends with this
  return 0;
}
