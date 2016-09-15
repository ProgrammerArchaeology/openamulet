/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code has been placed in the public   			          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* Originally written as AmEdit by S.Nebel (Linkworks Ltd Wellington,NZ) 1997.
   Modified and updated by Brad A. Myers of the Amulet project at CMU.
*/

#include <amulet.h>

#include <stdio.h>
#include <iostream.h>
#include <stdlib.h>

#include "panel.h"
#include "externs.h"

Am_Slot_Key MY_RANK = Am_Register_Slot_Name ("MY_RANK");
Am_Slot_Key SELECTED_ITEM = Am_Register_Slot_Name ("SELECTED_ITEM");

static Am_Object selection_rect;
static Am_Object items_group; 
static Am_Object var_name; 
static Am_Object ok_button; 
static Am_Object cancel_button; 
static Am_Object item_list; 
static Am_Object vert_box; 
static Am_Object fixed_width; 
static Am_Object delete_button; 
static Am_Object update_button; 
static Am_Object new_button; 
static Am_Object item_name; 
static Am_Object win = Am_No_Object;
static Am_Object selected_item = Am_No_Object;
static Am_Object box_on_left_cmd;
static Am_Object vert_cmd;
static Am_Object h_spacing;
static Am_Object v_spacing;
static Am_Object max_rank;
static Am_Object choose_item;

void change_selection(int rank, Am_String new_one, Am_bool do_set = true)
{
  int i ;
  if (do_set) {
    Am_Value_List li = (Am_Value_List) items_group.Get(Am_ITEMS);
    for (i=0 , li.Start(); i < rank ; i++,li.Next()) ;
    li.Set(new_one, false);
  }
  items_group.Note_Changed(Am_ITEMS);
  item_name.Set(Am_VALUE, new_one);
   win.Set(MY_RANK, rank);
}

Am_Define_No_Self_Formula(Am_Wrapper*, set_selected_item) {  
   int rank = win.Get(MY_RANK);
   if (selected_item.Valid()){
	selected_item.Set(Am_LINE_STYLE, Am_Black);
      }
   if (rank < 0) {
     item_name.Set(Am_VALUE, "");
     delete_button.Set(Am_ACTIVE, false);
     update_button.Set(Am_ACTIVE, false);
     selection_rect .Set(Am_VISIBLE, false);
     selected_item = Am_No_Object;
   }
   else {

      
      Am_Value_List l = (Am_Value_List) items_group.Get(Am_GRAPHICAL_PARTS);
      int i ;
      if (l.Length() <= rank) {
	l.End();
      }
      else{
	for ( i=0, l.Start() ; i < rank ; i++,l.Next()) ;
      }   
      selected_item = (Am_Object)l.Get();
      
      selected_item.Set(Am_LINE_STYLE, Am_White);
      selection_rect.Set(Am_TOP,selected_item.Get(Am_TOP))
	.Set(Am_LEFT, selected_item.Get(Am_LEFT))
	  .Set(Am_FILL_STYLE, Am_Black)
	    .Set(Am_WIDTH, 400)
	      .Set(Am_HEIGHT, selected_item.Get(Am_HEIGHT))
		.Set(Am_VISIBLE, true);
      item_name.Set(Am_VALUE, (Am_String)selected_item.Get(Am_TEXT));
      delete_button.Set(Am_ACTIVE, true);
      update_button.Set(Am_ACTIVE, true);
    }

 return(selected_item);  
 }


Am_Define_Method(Am_Object_Method, void, selection_done, (Am_Object cmd))

{ 
  Am_Object new_one = cmd.Get_Owner().Get(Am_INTERIM_VALUE);
  if (new_one != selected_item) 
    change_selection((int)new_one.Get(Am_RANK),(Am_String)new_one.Get(Am_TEXT));
  else
   win.Set(MY_RANK, -1);
   
}


void paneldialog::Cancel()
{
  if (win != Am_No_Object)
    Am_Finish_Pop_Up_Waiting(win,Am_Value(false));
}

void Set_Items_Group_Items(Am_Value_List &il) {
  Am_Value_List new_l;
  Am_Value item;
  Am_Object o;
  for (il.Start (); !il.Last (); il.Next ()) {
    item = il.Get();
    if (item.type == Am_OBJECT) {
      o = item;
      new_l.Add(o.Get(Am_LABEL));
    }
    else new_l.Add(item);
  }
  items_group.Set(Am_ITEMS, new_l);
}

//layout_key 1 = horiz, 0 = vert
void paneldialog::SetValues(Am_String &name , Am_Value_List &il,
			    Am_Value &layout_key, Am_Value &box_on_left,
			    Am_Value &fw, Am_Value &hspace, Am_Value &vspace,
			    Am_Value &maxrnk)
{
  Am_Value_List l;
  if (layout_key.Exists()) {
    if ((int)layout_key == 0) l.Add(1); //verticla
    vert_cmd.Set(Am_ACTIVE, true);
  }
  else vert_cmd.Set(Am_ACTIVE, false);

  if (box_on_left.Exists()) {
    if ((bool)box_on_left) l.Add(2);
    box_on_left_cmd.Set(Am_ACTIVE, true);
  }
  else box_on_left_cmd.Set(Am_ACTIVE, false);

  Am_Value_List l2;
  if (fw.Exists()) {
    if ((bool)fw)  l2.Add(1);
    fixed_width.Set(Am_ACTIVE, true);
  }
  else fixed_width.Set(Am_ACTIVE, false);

  vert_box.Set(Am_VALUE, l);
  fixed_width.Set(Am_VALUE, l2);
  items_group.Note_Changed(Am_ITEMS);
  var_name.Set(Am_VALUE,name);
 
  Set_Items_Group_Items(il);

  if (hspace.Exists()) {
    h_spacing.Set(Am_ACTIVE, true);
    h_spacing.Set(Am_VALUE, (int)hspace);
  }
  else {
    h_spacing.Set(Am_ACTIVE, false);
    h_spacing.Set(Am_VALUE, 0);
  }
  if (vspace.Exists()) {
    v_spacing.Set(Am_ACTIVE, true);
    v_spacing.Set(Am_VALUE, (int)vspace);
  }
  else {
    v_spacing.Set(Am_ACTIVE, false);
    v_spacing.Set(Am_VALUE, 0);
  }
  if (maxrnk.Exists()) {
    max_rank.Set(Am_ACTIVE, true);
    max_rank.Set(Am_VALUE, (int)maxrnk);
  }
  else {
    max_rank.Set(Am_ACTIVE, false);
    max_rank.Set(Am_VALUE, 0);
  }
}

//layout_key 1 = horiz, 0 = vert
void paneldialog::GetValues(Am_String &name, Am_Value_List &il,
			    Am_Value &layout_key, Am_Value &box_on_left,
			    Am_Value &fw, Am_Value &hspace, Am_Value &vspace,
			    Am_Value &maxrnk) {
 Am_Value_List l = vert_box.Get(Am_VALUE);
 l.Start(); bool vl = l.Member(1);
 l.Start(); bool bol = l.Member(2);
 l = fixed_width.Get(Am_VALUE);
 l.Start(); bool fwb = l.Member(1);
 int hval = h_spacing.Get(Am_VALUE);
 int vval = v_spacing.Get(Am_VALUE);
 int rnk  = max_rank.Get(Am_VALUE);
 

 name = var_name.Get(Am_VALUE);
 il = items_group.Get(Am_ITEMS);
 if (layout_key.Exists()) layout_key = (int)(!vl);
 if (box_on_left.Exists()) box_on_left = bol;
 if (fw.Exists()) fw = fwb;
 if (hspace.Exists()) hspace = hval;
 if (vspace.Exists()) vspace = vval;
 if (maxrnk.Exists()) maxrnk = rnk;
}

void Set_Change_Change_Items(bool can_change_items) {
  delete_button.Set(Am_ACTIVE, can_change_items);
  new_button.Set(Am_ACTIVE, can_change_items);
  update_button.Set(Am_ACTIVE, can_change_items);
  item_name.Set(Am_ACTIVE, can_change_items);
  choose_item.Set(Am_ACTIVE, can_change_items);
}

Am_bool paneldialog::Execute (Am_String &name , Am_Value_List &il,
                              Am_Value &layout_key, Am_Value &box_on_left,
                              Am_Value &fw, Am_Value &hspace, Am_Value &vspace,
			      Am_Value &maxrnk, bool can_change_items) {
  Init();

  delete_button.Set(Am_ACTIVE, false);
  update_button.Set(Am_ACTIVE, false);
  selection_rect.Set(Am_VISIBLE,false);
  item_name.Set(Am_VALUE,""); 
  selected_item = Am_No_Object;

  SetValues(name, il, layout_key, box_on_left, fw, hspace, vspace, maxrnk);
  Set_Change_Change_Items(can_change_items);

  Am_Value val;
  Am_Pop_Up_Window_And_Wait(win, val, true);
  
  if (selected_item.Valid())
    selected_item.Set(Am_LINE_STYLE, Am_Black);
  if (val.Valid()) {
    GetValues(name, il, layout_key, box_on_left, fw, hspace, vspace, maxrnk);
    return(true);
  }
  else return(false);
}

Am_Define_Method(Am_Object_Method, void, update_button_pressed_cmd,
		 (Am_Object /*cmd*/)) {
  if (selected_item.Valid()) 
    change_selection((int)selected_item.Get(Am_RANK),
		       (Am_String) item_name.Get(Am_VALUE));
}

Am_Define_Method(Am_Object_Method, void, delete_button_pressed_cmd,
		 (Am_Object /*cmd*/))
{
  if (selected_item.Valid()) {
    int i;
    selected_item.Set(Am_LINE_STYLE, Am_Black);
    int rank = selected_item.Get(Am_RANK);
    Am_Value_List l = (Am_Value_List) items_group.Get(Am_ITEMS);
    for (i =0, l.Start(); i < rank ; i++, l.Next()) ;
    l.Delete(false);
    items_group.Note_Changed(Am_ITEMS);
    win.Set(MY_RANK, -1);
    selected_item = Am_No_Object;
  }
}

Am_Define_Method(Am_Object_Method, void, new_button_pressed_cmd,
		 (Am_Object /* cmd */))
{
  int rank =-1;
  int i;
  Am_String text("New Item");
  Am_Value_List l = (Am_Value_List)items_group.Get(Am_GRAPHICAL_PARTS);
  Am_Value_List li = (Am_Value_List)items_group.Get(Am_ITEMS);
  if (selected_item.Valid()) {
    l.Start();

    if (l.Member(selected_item)) {
    Am_Object am = l.Get();
      rank = am.Get(Am_RANK);
      for (li.Start(), i=0; i < rank; i++,li.Next()) ;
      li.Insert(text, Am_BEFORE,false);
    }
    else {

      li.Add(text, Am_TAIL, false);
      rank = li.Length();
    }
  }
  else {
    li.Add(text,Am_TAIL,false);
    rank = li.Length() ; 
 }
  change_selection(rank,text, false);
}

Am_Define_Method(Am_Object_Method, void, do_update, (Am_Object cmd)){
  Am_String new_string = item_name.Get(Am_VALUE);
  if (!selected_item.Valid()) //add new item with this name
    new_button_pressed_cmd_proc(cmd); //should leave it selected
  cmd.Get(Am_VALUE); //run the demons
  if (selected_item.Valid()) {
    change_selection((int)selected_item.Get(Am_RANK), new_string);
    //clear selected item, so another new item will go afterwards
   win.Set(MY_RANK, -1);
  }
}
     


void paneldialog::Init()
{
  if (!win) {
    vert_cmd = Am_Command.Create("Vert Command")
      .Set(Am_LABEL,"Vertical Layout")
      .Set(Am_ID,1)
      .Set(Am_ACTIVE, true)
      ;
    box_on_left_cmd = Am_Command.Create("Box on Left Cmd")
      .Set(Am_LABEL,"Box on Left")
      .Set(Am_ID, 2)
      .Set(Am_ACTIVE, true)
      ;

    win = Am_Window.Create()
      .Set(Am_DESTROY_WINDOW_METHOD, Am_Default_Pop_Up_Window_Destroy_Method)
      .Add(MY_RANK,-1)
#ifndef _MACINTOSH
      .Set(Am_LEFT,1)
      .Set(Am_TOP,1)
#else
      .Set(Am_LEFT,20)
      .Set(Am_TOP,50)
#endif
      .Set(Am_WIDTH,332)
      .Set(Am_HEIGHT,454)
      .Set(Am_FILL_STYLE, Am_Motif_Light_Gray)
      .Add_Part(var_name = Am_Text_Input_Widget.Create( "var_name" )
		.Set(Am_LEFT, 6)
		.Set(Am_TOP, 14)
		.Set(Am_WIDTH, 314)
		.Set(Am_FONT,fontarray[0][1][0])
		.Set(Am_FILL_STYLE, Am_White)
		.Get_Object(Am_COMMAND).Set(Am_LABEL,"Name for item:")
		.Get_Owner()
		)
      .Add_Part(item_list = Am_Scrolling_Group.Create( "item_list" )
		.Set(Am_LEFT,8)
		.Set(Am_TOP,45)
		.Set(Am_WIDTH,311)
		.Set(Am_HEIGHT,157)
		.Set(Am_FILL_STYLE, Am_Motif_Gray)
		.Set(Am_LINE_STYLE,Am_Black)
		.Set(Am_INNER_WIDTH,400)
		.Set(Am_INNER_HEIGHT,400)
		.Set(Am_H_SCROLL_BAR,0)
		.Set(Am_V_SCROLL_BAR,1)
		.Set(Am_H_SCROLL_BAR_ON_TOP,0)
		.Set(Am_V_SCROLL_BAR_ON_LEFT,0)
		)
      .Add_Part(vert_box = Am_Checkbox_Panel.Create( "vert_box" )
		.Set(Am_LEFT,8)
		.Set(Am_TOP,288)
		.Set(Am_WIDTH,247)
		.Set(Am_HEIGHT,25)
		.Set(Am_FONT,fontarray[0][1][0])
		.Set(Am_FILL_STYLE, Am_Motif_Gray)
		.Set(Am_LAYOUT,Am_Horizontal_Layout)
		.Set(Am_BOX_ON_LEFT,1)
		.Set(Am_ITEMS, Am_Value_List().Add(vert_cmd).Add(box_on_left_cmd))
		)
      .Add_Part(fixed_width = Am_Checkbox_Panel.Create( "fixed_width" )
		.Set(Am_LEFT,8)
		.Set(Am_TOP,307)
		.Set(Am_WIDTH,106)
		.Set(Am_HEIGHT,25)
		.Set(Am_FONT, fontarray[0][1][0])
		.Set(Am_FILL_STYLE, Am_Motif_Gray)
		.Set(Am_LAYOUT,Am_Vertical_Layout)
		.Set(Am_BOX_ON_LEFT,1)
		.Set(Am_ITEMS, Am_Value_List()
		     .Add(Am_Command.Create()
			  .Set(Am_LABEL,"Fixed Width")
			  .Set(Am_ID,1)
			  )
		     )
		)
      .Add_Part(delete_button = Am_Button.Create( "delete_button" )
		.Set(Am_LEFT, 15)
		.Set(Am_TOP,241)
		.Set(Am_FONT,fontarray[0][1][0])
		.Set(Am_FILL_STYLE, Am_Motif_Gray)
		.Get_Object(Am_COMMAND).Set(Am_LABEL,"Delete Item")
		.Get_Owner()
		)
      .Add_Part(new_button = Am_Button.Create( "new_button" )
		.Set(Am_LEFT, Am_Center_X_Is_Center_Of_Owner)
		.Set(Am_TOP,241)
		.Set(Am_FONT,fontarray[0][1][0])
		.Set(Am_FILL_STYLE, Am_Motif_Gray)
		.Get_Object(Am_COMMAND).Set(Am_LABEL,"New Item")
		.Get_Owner()
		)
      .Add_Part(update_button = Am_Button.Create( "update_button" )
		.Set(Am_LEFT, Am_Right_Is_Right_Of_Owner)
		.Set(Am_RIGHT_OFFSET, 15) //offset from right
		.Set(Am_TOP,241)
		.Set(Am_FONT,fontarray[0][1][0])
		.Set(Am_FILL_STYLE, Am_Motif_Gray)
		.Get_Object(Am_COMMAND).Set(Am_LABEL,"Update Item")
		.Get_Owner()
		)
      .Add_Part(item_name = Am_Text_Input_Widget.Create( "item_name" )
		//.Set(Lw_FORMAT,"")
		.Set(Am_LEFT,8)
		.Set(Am_TOP,211)
		.Set(Am_WIDTH,306)
		.Set(Am_FONT,fontarray[0][1][0])
		.Set(Am_FILL_STYLE, Am_White)
		.Get_Object(Am_COMMAND).Set(Am_LABEL,"")
		.Set(Am_DO_METHOD, do_update)
		.Get_Owner()
		)
      .Add_Part(h_spacing = Am_Number_Input_Widget.Create("H_SPACING")
		.Set(Am_LEFT, 30)
		.Set(Am_TOP, 331)
		.Set(Am_WIDTH, 289)
		.Set(Am_FILL_STYLE, Am_Motif_Light_Gray)
		.Get_Object(Am_COMMAND)
		.Set(Am_LABEL, "Horizontal Spacing between items")
		.Get_Owner()
		.Set(Am_VALUE_1, Am_No_Value)
		.Set(Am_VALUE_2, Am_No_Value)
		)
      .Add_Part(v_spacing = Am_Number_Input_Widget.Create("V_SPACING")
		.Set(Am_LEFT, 29)
		.Set(Am_TOP, 362)
		.Set(Am_WIDTH, 290)
		.Set(Am_FILL_STYLE, Am_Motif_Light_Gray)
		.Get_Object(Am_COMMAND)
		.Set(Am_LABEL, "  Vertical Spacing between items")
		.Get_Owner()
		.Set(Am_VALUE_1, Am_No_Value)
		.Set(Am_VALUE_2, Am_No_Value)
		)
      .Add_Part(max_rank = Am_Number_Input_Widget.Create("MAX_RANK")
		.Set(Am_LEFT, 0)
		.Set(Am_TOP, 393)
		.Set(Am_WIDTH, 319)
		.Set(Am_FILL_STYLE, Am_Motif_Light_Gray)
		.Get_Object(Am_COMMAND)
		.Set(Am_LABEL, "Maximum Number Items in Row or Column")
		.Get_Owner()
		.Set(Am_VALUE_1, Am_No_Value)
		.Set(Am_VALUE_2, Am_No_Value)
		)
      .Add_Part(ok_button = Am_Button.Create( "ok_button" )
		.Set(Am_LEFT,63)
		.Set(Am_TOP,420)
		.Set(Am_WIDTH,93)
		.Set(Am_HEIGHT,31)
		.Set(Am_FONT,fontarray[0][1][0])
		.Set(Am_FILL_STYLE, Am_Motif_Gray)
		.Get_Object(Am_COMMAND).Set(Am_LABEL,"Ok")
		.Get_Owner()
		)
      .Add_Part(cancel_button = Am_Button.Create( "cancel_button" )
		.Set(Am_LEFT,169)
		.Set(Am_TOP,420)
		.Set(Am_WIDTH,93)
		.Set(Am_HEIGHT,31)
		.Set(Am_FONT,fontarray[0][1][0])
		.Set(Am_FILL_STYLE, Am_Motif_Gray)
		.Get_Object(Am_COMMAND).Set(Am_LABEL,"Cancel")
		.Get_Owner()
		)
      ;

    win.Set(Am_TITLE, "Panel Settings")
      .Set(Am_ICON_TITLE, Am_Same_As(Am_TITLE))
      .Set(Am_DESTROY_WINDOW_METHOD, Am_Default_Pop_Up_Window_Destroy_Method) 
      .Set(Am_USE_MIN_WIDTH, true)
      .Set(Am_USE_MAX_WIDTH, true)
      .Set(Am_USE_MIN_HEIGHT, true)
      .Set(Am_USE_MAX_HEIGHT, true)
      .Set(Am_MIN_WIDTH, Am_Same_As(Am_WIDTH))
      .Set(Am_MAX_WIDTH, Am_Same_As(Am_WIDTH))
      .Set(Am_MIN_HEIGHT, Am_Same_As(Am_HEIGHT))
      .Set(Am_MAX_HEIGHT, Am_Same_As(Am_HEIGHT))
      .Add_Part(Am_Tab_To_Next_Widget_Interactor.Create());     

    item_list.Set(Am_INNER_HEIGHT, Am_Height_Of_Parts);
    item_list.Add_Part(selection_rect = Am_Rectangle.Create()
		       .Set(Am_VISIBLE,false)
		       .Set(Am_FILL_STYLE,Am_Black));

    item_list.Add_Part(items_group = Am_Map.Create("items_group")
		       .Set(Am_LEFT, 0)
		       .Set(Am_TOP, 0)
                       .Set(Am_WIDTH, 400)
                       .Set(Am_HEIGHT,Am_Height_Of_Parts)
		       .Set(Am_LAYOUT, Am_Vertical_Layout)
                       .Set(Am_H_ALIGN,Am_LEFT_ALIGN)
		       .Set(Am_ITEMS,Am_Value_List())
		       .Set(Am_ITEM_PROTOTYPE, Am_Text.Create()
			    .Add(Am_ITEM, "")
                            .Set(Am_WIDTH, 400)
			    .Set(Am_TEXT, Am_Same_As(Am_ITEM))));
    
    items_group.Add_Part(choose_item = Am_Choice_Interactor.Create()
			 .Set(Am_HOW_SET,Am_CHOICE_TOGGLE)
			 .Get_Object(Am_COMMAND)
			 .Set(Am_DO_METHOD, selection_done)
			 .Get_Owner());

    delete_button.Set(Am_ACTIVE, false).Get_Object(Am_COMMAND).Set(Am_DO_METHOD, delete_button_pressed_cmd); 
    update_button.Set(Am_ACTIVE, false).Get_Object(Am_COMMAND).Set(Am_DO_METHOD, update_button_pressed_cmd); 

    new_button.Get_Object(Am_COMMAND).Set(Am_DO_METHOD, new_button_pressed_cmd);                             
    ok_button.Get_Object(Am_COMMAND).Set(Am_DO_METHOD, ok_button_pressed_cmd); 
    cancel_button.Get_Object(Am_COMMAND).Set(Am_DO_METHOD, cancel_button_pressed_cmd); 

    win.Add(SELECTED_ITEM, set_selected_item);
    
    
  }
}

















