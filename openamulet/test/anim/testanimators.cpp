/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

// for Am_INTER_PRIORITY_DIFF
#include <am_inc.h>
#include INTER_ADVANCED__H
#include OPAL_ADVANCED__H

#include <amulet.h>

Am_Object Mover;
Am_Object Animator;
Am_Object hopper;
Am_Object wiggler;
Am_Object fixed_time;
Am_Object window;
Am_Object fade_group;
Am_Object palette;
Am_Object line_palette;
Am_Object polygon;
Am_Object anim7, change_pic_anim, wiggler_blink, wiggler_vis_blink;

/////////////////////////////////////////////////////////////////////////
// For the bouncing button
/////////////////////////////////////////////////////////////////////////

Am_Object win2, animated_button, animated_button_color, animated_button_bounce;
Am_Value_List color_list;

Am_Define_Method(Am_Object_Method, void, next_button_color,
		 (Am_Object /* command_obj */)) {
  color_list.Next();
  if (color_list.Last()) color_list.Start();
  Am_Style new_color = color_list.Get();
  std::cout << "Setting color of button\n" << std::flush;
  animated_button.Set(Am_FILL_STYLE, new_color);
}

Am_Define_Method(Am_Object_Method, void, start_button_anims,
		 (Am_Object command_obj)) {
  //first, call the standard do
  Am_Object_Method method = Am_Cycle_Value_Command.Get(Am_DO_METHOD);
  method.Call(command_obj);
  //now see if need to start an animation
  int val = command_obj.Get(Am_VALUE);
  if (val == 0) { //just hit stop
    std::cout << "Turning OFF animations of " << animated_button_color << " and "
	 << animated_button_bounce << std::endl << std::flush;
    Am_Abort_Animator(animated_button_color);
    Am_Abort_Interactor(animated_button_bounce);
  }
  else if (val == 1) { //just hit color
    std::cout << "Turning on animations of " << animated_button_color
	 << std::endl << std::flush;
    next_button_color_proc(command_obj);
  }
  else if (val == 2) { //just hit position
    Am_Abort_Animator(animated_button_color);
    std::cout << "Turning on animations of " << animated_button_bounce
	 << std::endl << std::flush;
    animated_button_bounce.Set(Am_ACTIVE, true);
    Am_Start_Interactor(animated_button_bounce);
  }
}

void Create_Bouncing_Button() {
  if (win2.Valid()) Am_To_Top(win2);
  else {
    win2 = Am_Window.Create("Bouncing Button")
      .Set(Am_FILL_STYLE, Am_Motif_Light_Gray)
      .Set(Am_LEFT, 500)
      .Set(Am_TOP, 20)
      .Set(Am_WIDTH, 300)
      .Set(Am_HEIGHT, 300);
    Am_Screen.Add_Part(win2);
    color_list = Am_Value_List()
	    .Add(Am_Motif_Light_Gray)
	    .Add(Am_Yellow)
	    .Add(Am_Motif_Blue)
	    .Add(Am_Motif_Orange)
	    .Add(Am_Motif_Green)
	    ;
    color_list.Start();
    animated_button = Am_Button.Create("animated_button")
      .Set (Am_LEFT, 20)
      .Set (Am_TOP, 20)
      .Set (Am_FILL_STYLE, Am_Motif_Light_Gray)
      .Set (Am_FILL_STYLE,
	    Am_Animate_With (animated_button_color =
			     Am_Style_Animator.Create ("anim_but_color")
			     .Set (Am_DURATION, Am_Time (2000))
			     .Get_Object(Am_COMMAND)
			         .Set(Am_DO_METHOD, next_button_color)
			         .Get_Owner()
			     ))
      .Set_Part(Am_COMMAND, Am_Cycle_Value_Command.Create("cycle_value")
		.Set(Am_DO_METHOD, start_button_anims)
		.Set(Am_LABEL_LIST, Am_Value_List()
		     .Add("Animate My Color   ")
		     .Add("Animate My Position")
		     .Add("Stop               ")
		     ))
      ;
    animated_button_bounce = Am_Animation_Interactor.Create("bounce_button")
      .Set (Am_START_WHEN, true)
      .Set (Am_ACTIVE, false)
      .Set (Am_PRIORITY, -101)
      .Set_Part(Am_COMMAND, Am_Animation_Object_In_Owner_Command.Create()
		.Set(Am_X_OFFSET, 3)
		.Set(Am_Y_OFFSET, 3)
		.Set(Am_ANIMATE_END_ACTION, Am_ANIMATION_BOUNCE));
    animated_button.Add_Part(animated_button_bounce);
    win2.Add_Part(animated_button);
  }
}

/////////////////////////////////////////////////////////////////////////
// For the polygon
/////////////////////////////////////////////////////////////////////////

int star_int_list [12] = {0, 50, 125, 125, 75, 0, 25, 125, 150, 50, 0, 50};
int tria [8] = {25, 50, 50, 100, 75, 50, 25, 50};
Am_Point_List star_point_list (star_int_list, 12);
Am_Point_List triangle_point_list (tria, 8);

Am_Object option_button;
Am_Object rectvisibleanimation;

#define TESTUSEPIXMAPS

#ifdef TESTUSEPIXMAPS

Am_Object horse,girl,dog,bug; 
Am_Object doganimation;
Am_Object dogvisibleanimation;
Am_Object coloranimation;
Am_Object optionvisibleanimation;

static Am_Object get_icon(char* initial_name, char* obj_name) {
  char* pathname;
  pathname = Am_Merge_Pathname(initial_name);
  Am_Image_Array icon;
  icon = Am_Image_Array(pathname);
  delete [] pathname;
  return Am_Bitmap.Create(obj_name)
    .Set (Am_IMAGE, icon);
}

Am_Define_Formula(int, same_as_horse_bottom) {
  return (int)horse.Get(Am_TOP)+(int)horse.Get(Am_HEIGHT)-
    (int)self.Get(Am_HEIGHT);
}
Am_Define_Formula(int, behind_horse) {
  return (int)horse.Get(Am_LEFT) - (int)self.Get(Am_WIDTH) - 3;
}
Am_Define_No_Self_Formula(int, left_on_horse) {
  return (int)horse.Get(Am_LEFT) + 5;
}
Am_Define_No_Self_Formula(int, top_on_horse) {
  return (int)horse.Get(Am_TOP) - 18;
}
Am_Define_No_Self_Formula(int, left_on_dog) {
  return (int)dog.Get(Am_LEFT) + 5;
}
Am_Define_No_Self_Formula(int, top_on_dog) {
  return (int)dog.Get(Am_TOP) - 5;
}

static void init_pixmaps () {
  horse = get_icon("lib/images/horse.gif", "horse");
  girl = get_icon("lib/images/girl.gif", "girl");
  dog = get_icon("lib/images/dog.gif", "dog");
  bug = get_icon("lib/images/bugsmall.gif", "bug");

  horse.Set(Am_LEFT, 500)
    .Set(Am_TOP, 50)
    .Add_Part (Am_Move_Grow_Interactor.Create ("Move_Horse"));

  doganimation = Am_Animator.Create ("dog_position")
    .Set (Am_ACTIVE, false)
    .Set (Am_TIMING_FUNCTION, Am_Slow_In_Slow_Out)
    .Add (Am_SHARPNESS_1, 3)
    .Add (Am_SHARPNESS_2, 3)
    ;
  dog.Set(Am_TOP, same_as_horse_bottom)
    .Set(Am_LEFT, behind_horse)
    .Set (Am_LEFT, Am_Animate_With (doganimation))
    .Set (Am_TOP, Am_Animate_With (doganimation))
    .Set (Am_VISIBLE,
	  Am_Animate_With (
	     dogvisibleanimation = 
	     Am_Visible_Animator.Create("dog_visible")
			   .Set (Am_LEFT, -1000)
			   .Set (Am_TOP, -1000)))
    ;
  girl.Set(Am_LEFT, left_on_horse)
    .Set(Am_TOP, top_on_horse)
    .Set(Am_VISIBLE, false);
  bug.Set(Am_LEFT, left_on_dog)
    .Set(Am_TOP, top_on_dog)
    .Set(Am_VISIBLE, false);
  
  fade_group.Add_Part(horse)
    .Add_Part(girl)
    .Add_Part(dog)
    .Add_Part(bug);
}
#endif

// This handles the keyboard input to the window.
Am_Define_Method (Am_Object_Method, void, key_press, (Am_Object self))
{
  Am_Object inter = self.Get_Owner();
  Am_Input_Char startc = inter.Get (Am_START_CHAR);
  int c = startc.As_Char();

  if (c == 'q')
    Am_Exit_Main_Event_Loop ();
  else if (c == 's') {
    if ((int)Mover.Get (Am_LEFT) == 500) {
      Mover.Set (Am_LEFT, 175);
      Mover.Set (Am_TOP, 125);
    }
    else {
      Mover.Set (Am_LEFT, 500);
      Mover.Set (Am_TOP, 500);
    }
  }
  else if (c == 'a')
    Animator.Set (Am_VISIBLE, !(bool)Animator.Get(Am_VISIBLE));
  else if (c == 'f')
    fixed_time.Set (Am_VISIBLE, !(bool)fixed_time.Get(Am_VISIBLE));
  else if (c == 'h')
    hopper.Set (Am_VISIBLE, !(bool)hopper.Get(Am_VISIBLE));
  else if (c == 'w')
    wiggler.Set (Am_VISIBLE, !(bool)wiggler.Get(Am_VISIBLE));
  else if (c == 'e') {
    static bool walking = true;
    if (walking) {
      Am_Abort_Interactor(anim7);
      Am_Abort_Animator (change_pic_anim);
    }
    else {
      Am_Start_Interactor(anim7);
      Am_Start_Animator (change_pic_anim);
    }
    walking = !walking;
  }
  else if (c == 'c')
    coloranimation.Set (Am_RGB_COLOR, !(bool)coloranimation.Get(Am_RGB_COLOR));
  else if (c == 'N') Create_Bouncing_Button();
  else if (c == 'B') {
    static bool blinking = false;
    if (blinking) {
      Am_Abort_Animator (wiggler_blink);
    }
    else {
      Am_Start_Animator (wiggler_blink);
    }
    blinking = !blinking;
  }
  else if (c == 'A') {
    static bool visblinking = false;
    if (visblinking) {
      Am_Abort_Animator (wiggler_vis_blink);
      wiggler.Set(Am_VISIBLE, true); //might stop with it invisible
    }
    else {
      Am_Start_Animator (wiggler_vis_blink);
    }
    visblinking = !visblinking;
  }
  else if (c == 'r')
    Mover.Set (Am_VISIBLE, !(bool)Mover.Get(Am_VISIBLE));
  else if (c == 'i') {
    static bool debugging = false;
    if (!debugging) Am_Set_Inter_Trace(Am_INTER_TRACE_ALL);
    else Am_Set_Inter_Trace(Am_INTER_TRACE_NONE);
    debugging = !debugging;
  }
  else if (c == 'm') {
    static int visibleanimationcnt = 1;
    bool new_visible = !(bool)Mover.Get(Am_VISIBLE);
    switch (visibleanimationcnt) {
    case 1:
    case 2:
      rectvisibleanimation.Set(Am_WIDTH, 1);
      rectvisibleanimation.Set(Am_HEIGHT, 1);
      std::cout << " setting visible to " << new_visible
	   << " in X and Y" << std::endl << std::flush;
      break;
    case 3:
    case 4:
      rectvisibleanimation.Set(Am_WIDTH, 1);
      rectvisibleanimation.Set(Am_HEIGHT, Am_No_Value);
      std::cout << " setting visible to " << new_visible
	   << " shrink in X only" << std::endl << std::flush;
      break;
    case 5:
    case 6:
      rectvisibleanimation.Set(Am_WIDTH, Am_No_Value);
      rectvisibleanimation.Set(Am_HEIGHT, 1);
      std::cout << " setting visible to " << new_visible
	   << " shrink in Y only" << std::endl << std::flush;
      break;
    }
    visibleanimationcnt = (visibleanimationcnt % 6) + 1;
    Mover.Set (Am_VISIBLE, new_visible);
  }
#ifdef TESTUSEPIXMAPS
  else if (c == 'g')
    girl.Set (Am_VISIBLE, !(bool)girl.Get(Am_VISIBLE));
  else if (c == 'b')
    bug.Set (Am_VISIBLE, !(bool)bug.Get(Am_VISIBLE));
  else if (c == 'v')
    fade_group.Set (Am_VISIBLE, !(bool)fade_group.Get(Am_VISIBLE));
  else if (c == 'V') {
    static int dogvisibleanimationcnt = 1;
    bool new_visible = !(bool)dog.Get(Am_VISIBLE);
    switch (dogvisibleanimationcnt) {
    case 1:
    case 2: dogvisibleanimation.Set(Am_LEFT, -1000);
      dogvisibleanimation.Set(Am_TOP, -1000);
      std::cout << "setting visible to " << new_visible << " from -1000,-1000 "
	   << std::endl << std::flush;
      break;
    case 3:
    case 4:dogvisibleanimation.Set(Am_LEFT, Am_No_Value);
      dogvisibleanimation.Set(Am_TOP, 100);
      std::cout << "setting visible to " << new_visible << " from Y=100 only "
	   << std::endl << std::flush;
      break;
    case 5:
    case 6:dogvisibleanimation.Set(Am_LEFT, 500);
      dogvisibleanimation.Set(Am_TOP, Am_No_Value);
      std::cout << "setting visible to " << new_visible << " from x=500 only "
	   << std::endl << std::flush;
      break;
    }
    dogvisibleanimationcnt = (dogvisibleanimationcnt % 6) + 1;
    dog.Set (Am_VISIBLE, new_visible);
  }
  else if (c == 'd')
    doganimation.Set (Am_ACTIVE, !(bool)doganimation.Get(Am_ACTIVE));
  else if (c == 'o')
    optionvisibleanimation.Set (Am_ACTIVE,
				!(bool)optionvisibleanimation.Get(Am_ACTIVE));
#endif
  else if (c == 'p') {
    static int polygonanimationcnt = 1;
    static Am_Point_List copy_point_list;
    Am_Point_List pl = polygon.Get(Am_POINT_LIST);
    
    float x1, y1;
    switch (polygonanimationcnt) {
    case 1: {
      std::cout << "<><>Adding new point to polygon old = " << pl ;
      pl.Start();
      pl.Next();
      pl.Next();
      pl.Get(x1, y1);
      pl.Insert(x1-25, y1-50, Am_AFTER);
      break;
    }
    case 2: {
      std::cout << "<><>Removing the new point from polygon old = " << pl ;
      pl.Start();
      pl.Next();
      pl.Next();
      pl.Next();
      pl.Delete();
      break;
    }
    case 3: {
      std::cout << "<><>Point list to nothing; old = " << pl ;
      pl = Am_Point_List(); // empty list
      break;
    }
    case 4: {
      std::cout << "<><>Point list from nothing; old = " << pl ;
      pl = triangle_point_list;
      break;
    }
    case 5: {
      std::cout << "<><>Changing radically to be a star; old = " << pl;
      pl = star_point_list;
      break;
    }
    case 6: {
      std::cout << "<><>Back to triangle; old = " << pl;
      pl = triangle_point_list;
      break;
    }
    }
    polygonanimationcnt = (polygonanimationcnt % 6) + 1;
    std::cout << " new = " << pl << std::endl << std::flush;
    polygon.Set(Am_POINT_LIST, pl);
  }
  else std::cout
    << "Commands:" << std::endl
    << "a to toggle 2D animator" << std::endl
    << "f to toggle fixed-time animator" << std::endl
    << "h to toggle hopping animator" << std::endl
    << "w to toggle wiggling animator" << std::endl
    << "s to set red rectangle's left & top" << std::endl
    << "c to toggle RGB and HSV color paths" << std::endl
    << "r to hide red rectangle\n"
    << "i = toggle interactor (animator) tracing\n"
    << "m = grow visible animate rectange\n"
    << "o = toggle animation on 1st option menu\n"
    << "p = test polygon animation\n"
    << "e = toggle eye walking" << std::endl
#ifdef TESTUSEPIXMAPS
    << "For icons:\n"
    << "   g = toggle girl\n"
    << "   d = toggle dog animation\n"
    << "   b = toggle bug\n"
    << "   v = toggle visible on group (fade out)\n"
    << "   V = toggle visible on dog (fly away)\n"
#endif
    << " A = blink wiggler visible\n"
    << " B = blink wiggler blue and green\n"
    << " N = create new window with animated button\n"
    << "q to quit" << std::endl 
    << std::flush ; 
}
// used: abcdefghimoprsvw BV

Am_Define_No_Self_Formula (Am_Value, fetch_color)
{
  return palette.Get_Object (Am_VALUE).Get (Am_FILL_STYLE);
}

Am_Define_No_Self_Formula (Am_Value, fetch_line)
{
  return line_palette.Get_Object (Am_VALUE).Get (Am_LINE_STYLE);
}

Am_Object make_color_rect (Am_Style& color)
{
  return Am_Rectangle.Create ()
    .Set (Am_WIDTH, 30)
    .Set (Am_HEIGHT, 20)
    .Set (Am_LINE_STYLE, Am_Black)
    .Set (Am_FILL_STYLE, color)
  ;
}

Am_Object menu_fade_group;
Am_Object menu;

//goes in option_button
Am_Define_Method(Am_Object_Method, void, pop_up_sub_menu, (Am_Object cmd)) {
  Am_Object submenu = cmd.Get_Object(Am_SUB_MENU);
  submenu.Set(Am_VISIBLE, !(bool)submenu.Get(Am_VISIBLE));
}

//goes in submenu
Am_Define_Method(Am_Object_Method, void, pop_down_sub_menu, (Am_Object cmd)) {
  Am_Value value;
  Am_Object forbutton = cmd.Get(Am_FOR_ITEM);
  value=cmd.Peek(Am_VALUE);
  forbutton.Get_Object(Am_COMMAND).Set(Am_LABEL, value);
  cmd.Get_Owner().Get_Owner().Set(Am_VISIBLE, false);
}

Am_Define_No_Self_Formula(int, width_of_menu) {
  return menu.Get(Am_WIDTH);
}
Am_Define_Formula(int, button_left_m2) {
  return (int)self.Get_Object(Am_FOR_ITEM).Get(Am_LEFT, Am_NO_DEPENDENCY); //+1;
}
Am_Define_Formula(int, button_top_p_28) {
  Am_Object obj = self.Get_Object(Am_FOR_ITEM);
  return (int)obj.Get(Am_TOP, Am_NO_DEPENDENCY) + (int)obj.Get(Am_HEIGHT, Am_NO_DEPENDENCY);
//return (int)self.GV_Object(Am_FOR_ITEM).Get(Am_TOP) + 30;
}
void create_hacked_option_button() {
  menu = Am_Menu.Create("my_option_menu")
    .Set(Am_LEFT, 0)
    .Set(Am_TOP, 0)
    .Set(Am_ITEMS, Am_Value_List()
          .Add ("Option Menu First item")
          .Add ("Another menu item")
	  .Add (Am_Menu_Line_Command.Create("my menu line"))
          .Add ("Not active")
          .Add ("Item 4")
	  .Add ("Item 5")
	  .Add ("Last item")
          )
    ;
  option_button = Am_Button.Create("pretent_option_button")
    .Set(Am_LEAVE_ROOM_FOR_FRINGE, false)
    .Set(Am_LEFT, 10)
    .Set(Am_TOP, 250)
    .Set(Am_WIDTH, width_of_menu)
    .Set(Am_LEAVE_ROOM_FOR_FRINGE, false);
  option_button.Get_Object(Am_INTERACTOR)
    .Set(Am_CONTINUOUS, false);
  menu_fade_group = Am_Fade_Group.Create ("menu_Fade_Group")
    .Add(Am_FOR_ITEM, option_button)
    .Set(Am_LEFT, button_left_m2)
    .Set(Am_TOP, button_top_p_28)
    .Set(Am_VISIBLE, false)
    .Set (Am_WIDTH, width_of_menu)
    .Set (Am_HEIGHT, Am_Height_Of_Parts)
    .Set (Am_VALUE, 0)
    ;
  option_button.Get_Object(Am_COMMAND)
    .Set(Am_LABEL, "Push me")
    .Set(Am_SUB_MENU, menu_fade_group, Am_OK_IF_NOT_THERE)
    .Set(Am_DO_METHOD, pop_up_sub_menu);
  menu.Get_Object(Am_COMMAND)
    .Add(Am_FOR_ITEM, option_button)
    .Set(Am_DO_METHOD, pop_down_sub_menu);
  menu_fade_group.Add_Part(Am_SUB_MENU, menu);
  fade_group.Add_Part(option_button)
    .Add_Part(menu_fade_group);

  Am_Object option2 = option_button.Copy()
    .Set(Am_LEFT, 220);
  Am_Object menu_fade_group2 = menu_fade_group.Copy()
    .Set(Am_FOR_ITEM,  option2);
  menu_fade_group2.Get_Object(Am_SUB_MENU).Get_Object(Am_COMMAND)
    .Set(Am_FOR_ITEM, option2);
  option2.Get_Object(Am_COMMAND)
    .Set(Am_LABEL, "Push me also")
    .Set(Am_SUB_MENU, menu_fade_group2, Am_OK_IF_NOT_THERE)
    .Set(Am_DO_METHOD, pop_up_sub_menu);
  fade_group.Add_Part(option2)
    .Add_Part(menu_fade_group2);

  Am_Object option3 = option_button.Copy()
    .Set(Am_LEFT, 220)
    .Set(Am_TOP, 75);
  Am_Object menu_fade_group3 = menu_fade_group.Copy()
    .Set(Am_FOR_ITEM,  option3);
  menu_fade_group3.Get_Object(Am_SUB_MENU).Get_Object(Am_COMMAND)
    .Set(Am_FOR_ITEM, option3);
  option3.Get_Object(Am_COMMAND)
    .Set(Am_LABEL, "Push me too")
    .Set(Am_SUB_MENU, menu_fade_group3, Am_OK_IF_NOT_THERE)
    .Set(Am_DO_METHOD, pop_up_sub_menu);
  fade_group.Add_Part(option3)
    .Add_Part(menu_fade_group3);
  
  Am_Object option4 = option_button.Copy()
    .Set(Am_LEFT, 430);
  Am_Object menu_fade_group4 = menu_fade_group.Copy()
    .Set(Am_FOR_ITEM,  option4);
  menu_fade_group4.Get_Object(Am_SUB_MENU).Get_Object(Am_COMMAND)
    .Set(Am_FOR_ITEM, option4);
  option4.Get_Object(Am_COMMAND)
    .Set(Am_LABEL, "Another Push Me")
    .Set(Am_SUB_MENU, menu_fade_group4, Am_OK_IF_NOT_THERE)
    .Set(Am_DO_METHOD, pop_up_sub_menu);
  fade_group.Add_Part(option4)
    .Add_Part(menu_fade_group4);

  menu_fade_group.Set(Am_VISIBLE, 
		      Am_Animate_With (
			  optionvisibleanimation = 
			  Am_Visible_Animator.Create("fade_menu")
				       .Set (Am_USE_FADING, true)));
  
  menu_fade_group2.Set(Am_VISIBLE,
		       Am_Animate_With 
		       (Am_Visible_Animator.Create("fly_sub_menu")
			   .Set(Am_LEFT, Am_No_Value)
			   .Set(Am_TOP, -1000)));
  menu_fade_group3.Set(Am_VISIBLE,
		       Am_Animate_With
	       (Am_Fly_Apart_Animator.Create("fly_apart_sub_menu")));
  menu_fade_group4.Set(Am_VISIBLE,
		       Am_Animate_With
		       (Am_Visible_Animator.Create("grow_sub_menu")
			   .Set(Am_LEFT, Am_No_Value)
			   .Set(Am_TOP, Am_No_Value)
			   .Set(Am_WIDTH, Am_No_Value)
			   .Set(Am_HEIGHT, 1)));
}

char* pixmapfilename[] = {"lib/images/eye1.gif",
				  "lib/images/eye2.gif",
				  "lib/images/eye3.gif",
				  "lib/images/eye4.gif",
				  "lib/images/eye5.gif",
				  "lib/images/eye6.gif",
				  "lib/images/eye13.gif",
				  "lib/images/eye14.gif",
				  "lib/images/eye15.gif",
				  "lib/images/eye16.gif",
				  "lib/images/eye17.gif",
				  "lib/images/eye18.gif"};

Am_Value_List eye_walking_right, eye_walking_left;
Am_Object eye_icon, icon, anim8;

Am_Slot_Key WALKER = Am_Register_Slot_Name ("WALKER");

Am_Define_Value_List_Formula(which_way) {
  int x_offset = self.Get_Object(Am_OPERATES_ON)
    .Get_Object (WALKER).Get_Object(Am_COMMAND).Get(Am_X_OFFSET);
  if (x_offset > 0) return eye_walking_right;
  else return eye_walking_left;
}

Am_Define_Formula(int, plus_or_minus) {
  Am_Object inter = self.Get_Owner();
  //  Am_Input_Char ic = Am_Input_Char::Narrow(inter.GV(Am_START_CHAR));
  Am_Input_Char ic = inter.Get(Am_START_CHAR);
  if (ic.code == Am_LEFT_MOUSE) return -8;
  return 8;
}

Am_Define_Method(Am_Object_Method, void, my_do, (Am_Object cmd)) {
  std::cout << "\n+-+-+- Do method for Command " << cmd  << std::endl << std::flush;
}

Am_Image_Array read_pixmap (char* initialname) {
  char* pathname;
  Am_Image_Array icon;

  pathname = Am_Merge_Pathname(initialname);
  std::cout << "Reading image " << pathname << std::endl << std::flush;
  icon = Am_Image_Array(pathname);
  delete [] pathname;
  /*
    window2 = Am_Window.Create()
    .Set (Am_LEFT, 500 )
    .Set (Am_TOP, 50 + 30 * i)
    .Set (Am_WIDTH, Am_Width_Of_Parts)
    .Set (Am_HEIGHT, Am_Height_Of_Parts)
    .Add_Part(Am_Bitmap.Create()
    .Set (Am_IMAGE, icon))
    ;
    Am_Screen.Add_Part(window2);
  */

  return icon;
}

static void init_eye () {
  int i;
  for (i = 0; i < 6; i++)
    eye_walking_right.Add (read_pixmap(pixmapfilename[i]));

  for (; i < 12; i++)
    eye_walking_left.Add (read_pixmap(pixmapfilename[i]));

  std::cout << "creating eye icon \n" << std::flush;
  Am_Object cmd, cmd2;
  eye_icon = Am_Bitmap.Create("Eye_Icon")
    .Set (Am_LEFT, 5)
    .Set (Am_TOP, 220) // Am_Bottom_Is_Bottom_Of_Owner)
    .Add_Part(WALKER, anim7=Am_Animation_Interactor.Create("anim7")
		    // what an annoying hack this is -- setting priority
		    // intensely negative so that the running animation
		    // interactor doesn't block all other interactors
		    .Set (Am_PRIORITY, -Am_INTER_PRIORITY_DIFF)
		    .Set (Am_START_WHEN, true)
		    //change every 30 times a second
		    //.Set(Am_REPEAT_DELAY, Am_Time(33))
		    .Set_Part(Am_COMMAND, cmd =
		       Am_Animation_Object_In_Owner_Command.Create("move_eye")
		   .Set(Am_X_OFFSET, 4)
		   .Set(Am_Y_OFFSET, 0)
		   .Set(Am_ANIMATE_END_ACTION, Am_ANIMATION_BOUNCE)))
    .Set (Am_IMAGE, 0) //iniialize with some value
    .Set (Am_IMAGE, Am_Animate_With (
	  change_pic_anim = Am_Through_List_Animator.Create("change_eye_pict")
	  .Set(Am_LIST_OF_VALUES, which_way)
	  .Set_Part (Am_COMMAND, Am_Animation_Wrap_Command.Create ())))
    ;
  std::cout << eye_icon << " being added to " << window << std::endl << std::flush;
  window.Add_Part(eye_icon);
  std::cout << "done" << window << std::endl << std::flush;

  // start the animations
  Am_Start_Interactor(anim7);
  Am_Start_Animator (change_pic_anim);

#if 0
  eye_icon.Add_Part(anim8=Am_Animation_Interactor.Create("anim8")
		    .Set (Am_START_WHEN, "ANY_MOUSE_DOWN")
		    .Set (Am_STOP_WHEN, "ANY_MOUSE_UP")
		    .Set (Am_START_WHERE_TEST, true)
		    .Set_Part(Am_COMMAND, cmd2 =
	      Am_Animation_Object_In_Owner_Command.Create("move_eye_and_stop")
		   .Set(Am_X_OFFSET, plus_or_minus)
		   .Set(Am_Y_OFFSET, 0)
		   .Set(Am_DO_METHOD, my_do)
		   .Set(Am_ANIMATE_END_ACTION, Am_ANIMATION_STOP)));
  cmd2.Set(Am_IMPLEMENTATION_PARENT,
	  Am_Animation_Through_List_Command.Create("change_eye_pict2")
	  .Set(Am_SLOT_TO_ANIMATE, Am_IMAGE)
	  .Set(Am_VALUES_FOR_SLOT_TO_ANIMATE, pixmap_images)
	  .Set(Am_ANIMATE_END_ACTION, Am_ANIMATION_WRAP) //start back at beg
	   .Set(Am_DO_METHOD, my_do)
	  .Set(Am_IMPLEMENTATION_CHILD, cmd2) 
	  .Set(Am_INDICES_FOR_VALUES_FOR_SLOT_TO_ANIMATE, which_way)
	  );
#endif
}

int main ()
{
  Am_Initialize ();
  
  Am_Slot_Key MOVER = Am_Register_Slot_Name ("MOVER");
  //  Am_Slot_Key COLOR = Am_Register_Slot_Name ("COLOR");

//   Am_Object feedback = Am_Rectangle.Create ("Feedback")
//     .Set (Am_FILL_STYLE, Am_No_Style)
//     .Set (Am_LINE_STYLE, Am_Dotted_Line)
//     .Set (Am_WIDTH, 50)
//     .Set (Am_HEIGHT, 50)
//     .Set (Am_VISIBLE, false)
//   ;

  Am_Value_List color_rects = Am_Value_List ()
    .Add (make_color_rect (Am_Red))
    .Add (make_color_rect (Am_Blue))
    .Add (make_color_rect (Am_Green))
    .Add (make_color_rect (Am_Yellow))
    .Add (make_color_rect (Am_Purple))
    .Add (make_color_rect (Am_Cyan))
    .Add (make_color_rect (Am_Black))
    .Add (make_color_rect (Am_White))
  ;

  Am_Style thickred (1.0, 0.0, 0.0, 7);
  Am_Style thickblack (0.0, 0.0, 0.0, 7);
  Am_Value_List line_rects = Am_Value_List ()
    .Add (Am_Rectangle.Create ()
      .Set (Am_FILL_STYLE, Am_White)
      .Set (Am_LINE_STYLE, Am_Black)
    )
    .Add (Am_Rectangle.Create ()
      .Set (Am_FILL_STYLE, Am_White)
      .Set (Am_LINE_STYLE, Am_Red)
    )
    .Add (Am_Rectangle.Create ()
      .Set (Am_FILL_STYLE, Am_White)
      .Set (Am_LINE_STYLE, thickred)
    )
    .Add (Am_Rectangle.Create ()
      .Set (Am_FILL_STYLE, Am_White)
      .Set (Am_LINE_STYLE, thickblack)
    )
  ;
  color_rects.Start ();
  line_rects.Start ();
  palette = Am_Button_Panel.Create ("color palette")
      .Set (Am_LEFT, 10)
      .Set (Am_TOP, Am_Bottom_Is_Bottom_Of_Owner)
      .Set (Am_LAYOUT, Am_Horizontal_Layout)
      .Set (Am_FINAL_FEEDBACK_WANTED, true)
      .Set (Am_ITEMS, color_rects)
      .Set (Am_VALUE, color_rects.Get ())
      ;
  line_palette = Am_Button_Panel.Create ("line palette")
      .Set (Am_LEFT, Am_Right_Is_Right_Of_Owner)
      .Set (Am_TOP, Am_Bottom_Is_Bottom_Of_Owner)
      .Set (Am_LAYOUT, Am_Horizontal_Layout)
      .Set (Am_FINAL_FEEDBACK_WANTED, true)
      .Set (Am_ITEMS, line_rects)
      .Set (Am_VALUE, line_rects.Get ())
      ;
  
  Am_Screen
    .Add_Part (window = Am_Window.Create ("window")
      .Set (Am_RIGHT_OFFSET, 10)
      .Set (Am_BOTTOM_OFFSET, 10)
      .Set (Am_LEFT, 0)
      .Set (Am_TOP, 40)
      .Set (Am_WIDTH, 640)
      .Set (Am_HEIGHT, 440)
      .Add_Part (MOVER, Mover = Am_Rectangle.Create ("Grab Me")
        .Set (Am_LEFT, 120)
        .Set (Am_TOP, 120)
        .Set (Am_WIDTH, 50)
        .Set (Am_HEIGHT, 50)
        .Set (Am_FILL_STYLE, fetch_color.Multi_Constraint())
       .Set (Am_FILL_STYLE,
	      Am_Animate_With (
	      coloranimation =
          Am_Style_Animator.Create ("fill_style_interp")
	  .Set (Am_DURATION, Am_Time (2000)))
       )
        .Set (Am_LINE_STYLE, fetch_line.Multi_Constraint())
       .Set (Am_LINE_STYLE,
	      Am_Animate_With (
          Am_Style_Animator.Create ("line_style_interp")
	  .Set (Am_DURATION, Am_Time (3000)))
       )
        .Set (Am_LINE_STYLE, Am_Black)
	.Set(Am_VISIBLE,
		 Am_Animate_With (
	    rectvisibleanimation = 
	    Am_Visible_Animator.Create("grow_square")
				  .Set (Am_LEFT, 0)
				  .Set (Am_TOP, 0)
				  .Set (Am_WIDTH, 1)
				  .Set (Am_HEIGHT, 1))
	     )
        .Add_Part (Am_Move_Grow_Interactor.Create ("Mover")
          //.Set (Am_FEEDBACK_OBJECT, feedback)
		   )
		 )
      //.Add_Part (feedback)
      .Add_Part (Am_One_Shot_Interactor.Create("keyboard press")
        .Set (Am_START_WHEN, "ANY_KEYBOARD")
        .Get_Object (Am_COMMAND)
          .Set (Am_DO_METHOD, key_press)
          .Get_Owner ()
      )
     )
  ;

  hopper = Am_Arc.Create ("Hopper")
    .Set (Am_LEFT, Am_From_Sibling (MOVER, Am_LEFT))
    .Set (Am_LEFT, 
	      Am_Animate_With (
	      Am_Stepping_Animator.Create ("Hopper_Interp")
	      .Set (Am_REPEAT_DELAY, Am_Time(1000))
	      .Set (Am_SMALL_INCREMENT, 20)))
    .Set (Am_TOP, 20)
    .Set (Am_FILL_STYLE, Am_Green)
    ;
  window.Add_Part (hopper);

  fixed_time = Am_Roundtangle.Create ("Fixed Time")
    .Set(Am_WIDTH, 20)
    .Set(Am_HEIGHT, 20)
    .Set (Am_TOP, Am_From_Sibling (MOVER, Am_TOP))
    .Set (Am_TOP, 
	  Am_Animate_With (
	      Am_Animator.Create ("Fixed_Time_Interp")
	      .Set (Am_DURATION, Am_Time(4000))))
    .Set (Am_LEFT, 10)
    .Set (Am_FILL_STYLE, Am_Orange)
    ;
  window.Add_Part(fixed_time);

  wiggler = Am_Arc.Create ("Wiggler")
    .Set(Am_WIDTH, 20)
    .Set(Am_HEIGHT, 20)
    .Set (Am_TOP, Am_From_Sibling (MOVER, Am_TOP))
    .Set (Am_TOP, 
	  Am_Animate_With (
	      Am_Exaggerated_Animator.Create ("Wiggler_Interp")
	      .Set(Am_WIGGLE_AMOUNT, 4)))
    .Set (Am_LEFT, 60)
    .Set (Am_FILL_STYLE, Am_Blue)
    .Set (Am_FILL_STYLE, Am_Animate_With (wiggler_blink =
					  Am_Blink_Animator.Create()
					  .Set(Am_VALUE_1, Am_Blue)
					  .Set(Am_VALUE_2, Am_Green)
					  .Set(Am_ACTIVE, false)))
    .Set (Am_VISIBLE, Am_Animate_With (wiggler_vis_blink =
					  Am_Blink_Animator.Create()
					  .Set(Am_ACTIVE, false)))
    ;
  window.Add_Part(wiggler);

  Am_Object interpolator = 
    Am_Animator.Create ("Interp_2D")
//       .Set (Am_TIMING_FUNCTION, Am_Slow_In_Slow_Out)
//       .Set (Am_SHARPNESS_1, 3)
//       .Set (Am_SHARPNESS_2, 3)
      .Set (Am_INTERRUPTIBLE, false)
      ;
  Animator = Am_Rectangle.Create ("Animator")
    .Set (Am_LEFT, Am_From_Sibling (MOVER, Am_LEFT))
    .Set (Am_TOP, Am_From_Sibling (MOVER, Am_TOP))
    .Set (Am_LEFT, Am_Animate_With (interpolator))
    .Set (Am_TOP, Am_Animate_With (interpolator))

    .Set (Am_WIDTH, 50)
    .Set (Am_HEIGHT, 50)
    ;
  window.Add_Part(Animator);

  Am_To_Top (Mover);

  fade_group = Am_Fade_Group.Create ("Fade_Group")
    .Set (Am_WIDTH, Am_From_Owner(Am_WIDTH))
    .Set (Am_HEIGHT, Am_From_Owner(Am_HEIGHT))
    .Set (Am_VALUE, 0)
    .Set (Am_VISIBLE, Am_Animate_With (Am_Visible_Animator.Create("fade")
				       .Set (Am_USE_FADING, true)))
    ;
  triangle_point_list.Translate(475, 300);
  star_point_list.Translate(475, 300);
  polygon = Am_Polygon.Create ("Polygon")
    .Set (Am_FILL_STYLE, Am_Red)
    .Set (Am_POINT_LIST, triangle_point_list);
  fade_group
    .Add_Part (palette)
    .Add_Part (line_palette)
    .Add_Part (polygon)
  ;
  polygon.Set(Am_POINT_LIST,
  	      Am_Animate_With (Am_Point_List_Animator.Create()));
  window.Add_Part(fade_group);

#ifdef TESTUSEPIXMAPS
  init_pixmaps();
#endif

  create_hacked_option_button();

  init_eye ();

  Am_Main_Event_Loop ();
  Am_Cleanup ();
  return 0;
}
