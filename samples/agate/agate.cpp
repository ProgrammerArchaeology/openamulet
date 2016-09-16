/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <amulet.h>
#include <amulet/debugger.h>

#include <fstream>  // for ifstream, ofstream
#include <math.h>   // for sqrt()

using namespace std;

#if defined(_WINDOWS) || defined(_MACINTOSH)
#define AMULET_BITMAP "data/images/amside.gif"
#else									   
#define AMULET_BITMAP "data/images/amuletside.xbm"
#endif				 

#define NEW_CLASS_NAME  ""

Am_Object polyline;

// Global display objects

// Root window objects
Am_Object window;
Am_Object window_layout;
Am_Object menu_bar;
Am_Object my_selection;

// Class pane
// Am_Object class_name_editor;
Am_Object class_pane;
Am_Object class_scroller;
Am_Object class_panel;

// Example pane
Am_Object current_class_name;
Am_Object example_pane;
Am_Object example_scroller;
Am_Object example_panel;

// Gesture pane
Am_Object gesture_pane;
Am_Object gesture_frame;
Am_Object gesture_inter;
Am_Object gesture_feedback;
Am_Object gesture_shown;

// Mode
Am_Object mode;
bool dirty;  //  true iff current classifier needs to be saved

Am_String training_mode("Train");
Am_String recognizing_mode("Recognize");

// Global trainer object

Am_Gesture_Trainer trainer;

Am_String current_filename = "";
bool save_with_examples = true;

////////////////////////////////////////////////////////////////
////  Example abstraction 

#define ARROWHEAD_LENGTH  5

int EXAMPLE_FEATURES;
int EXAMPLE_STROKE;
int EXAMPLE_ARROW;
int EXAMPLE_SET_SCALING;

Am_Object Example;

Am_Point_List compute_arrowhead (Am_Point_List pl)
{
  int x1, y1, x2, y2;

  // get the first two distinct points out of pl into (x1,y1) and (x2,y2)
  pl.Start();  
  if (pl.Last()) 
    return 0;
  pl.Get(x1, y1);

  do {
    pl.Next ();
    if (pl.Last()) {
      x2 = x1 + 1; y2 = y1;
    }
    else pl.Get(x2, y2);

  } while (x1 == x2 && y1 == y2);

  // compute a 60 degree angle with vertex at x1,y1, pointing toward x2,y2
  int dx = x2 - x1;
  int dy = y2 - y1;
  float d = sqrt((double)(dx*dx + dy*dy));
  float scale = ARROWHEAD_LENGTH / d;

  const float a = 0.8660254037844;  // cos(30 deg)
  const float b = 0.5;              // sin(30 deg)
  
  float dxa = (- a*dx - b*dy) * scale;
  float dya = (  b*dx - a*dy) * scale;
  float dxb = (- a*dx + b*dy) * scale;
  float dyb = (- b*dx - a*dy) * scale;

  return Am_Point_List()
    .Add (x1 + (int)dxa, y1 + (int)dya)
    .Add (x1, y1)
    .Add (x1 + (int)dxb, y1 + (int)dyb)
    ;
}

Am_Define_Formula(int, set_scaling)
{
  int view_width = gesture_frame.Get (Am_WIDTH);
  int view_height = gesture_frame.Get (Am_HEIGHT);

  Am_Feature_Vector feat = self.Get (EXAMPLE_FEATURES);
  Am_Point_List pl, arrow_pl;

  if (feat.Valid())
    pl = feat.Points ();

  arrow_pl = compute_arrowhead(pl);

  if (pl.Valid()) {
    float xscale = (float)(int)self.Get (Am_WIDTH) / view_width;
    float yscale = (float)(int)self.Get (Am_HEIGHT) / view_height;

    pl.Scale (xscale, yscale);
    arrow_pl.Scale (xscale, yscale);
  }

  self.Get_Object (EXAMPLE_STROKE) .Set (Am_POINT_LIST, pl);
  self.Get_Object (EXAMPLE_ARROW) .Set (Am_POINT_LIST, compute_arrowhead(pl));
  return 1;
}

void Example_Init ()
{
  EXAMPLE_FEATURES = Am_Register_Slot_Name ("EXAMPLE_FEATURES");
  EXAMPLE_STROKE = Am_Register_Slot_Name ("EXAMPLE_STROKE");
  EXAMPLE_ARROW = Am_Register_Slot_Name ("EXAMPLE_ARROW");
  EXAMPLE_SET_SCALING = Am_Register_Slot_Name ("EXAMPLE_SET_SCALING");

  Example = Am_Group.Create ("Example")
    .Set (Am_WIDTH, 50)
    .Set (Am_HEIGHT, 50)

    // model: the example feature vector
    .Add (EXAMPLE_FEATURES, 0)

    // view: a polyline tracing out the gesture, with a little red 
    //       directional arrow at its starting point
    .Add_Part (EXAMPLE_STROKE, 
	       polyline.Create ("Example_stroke"))
    .Add_Part (EXAMPLE_ARROW, 
	       polyline.Create ("Example_arrow")
	         .Set (Am_LINE_STYLE, Am_Red))

    .Add (EXAMPLE_SET_SCALING, set_scaling)
    ;
}

Am_Object Example_Create (Am_Feature_Vector feat)
{
  Am_Object e = Example.Create()
    .Set (EXAMPLE_FEATURES, feat);
  return e;
}

Am_Value_List Example_Create_List (Am_Value_List feats)
{
  Am_Value_List examples;

  for (feats.Start(); !feats.Last(); feats.Next())
    examples.Add (Example_Create (feats.Get()));
  return examples;
}

void Show_Dialog_Centered (Am_Object dlg, Am_Object win)
{
  // center save dialog within the current win
  int win_left = win.Get (Am_LEFT);
  int win_width = win.Get (Am_WIDTH);
  int win_top = win.Get (Am_TOP);
  int win_height = win.Get (Am_HEIGHT);
  int dlg_width = dlg.Get (Am_WIDTH);
  int dlg_height = dlg.Get (Am_HEIGHT);
  dlg.Set (Am_LEFT, win_left + (win_width - dlg_width)/2);
  dlg.Set (Am_TOP, win_top + (win_height - dlg_height)/2);

  // show it
  dlg.Set (Am_VISIBLE, true);
  Am_To_Top(dlg);
}

////////////////////////////////////////////////////////////////
////  Class abstraction 

int CLASS_NAME;
int CLASS_EXAMPLES;
int CLASS_REPRESENTATIVE;
int CLASS_SET_REPRESENTATIVE;
int CLASS_LABEL;

Am_Object Class;

// constraint procedures

Am_Define_String_Formula(same_as_class_name) {
  return self.Get_Owner().Get(CLASS_NAME);
}

Am_Define_Formula(int, set_representative) {
  Am_String class_name = self.Get (CLASS_NAME);
//   Am_Value_List l = trainer.Get_Examples (class_name);
//   Am_Object example;

//   l.Start ();
//   if (!l.Last())
//     example = Example_Create (l.Get());

  Am_Object example;
  Am_Value_List l = self.Get (CLASS_EXAMPLES);
  l.Start ();
  if (!l.Last ())
    example = Am_Object(l.Get()).Copy();
  
  self.Remove_Part (CLASS_REPRESENTATIVE);
  self.Add_Part (CLASS_REPRESENTATIVE, example);
//  cout << "Class " << self << " represented by " << example << endl;
  return 1;
}

void Class_Init ()
{
  CLASS_NAME = Am_Register_Slot_Name ("CLASS_NAME");
  CLASS_EXAMPLES = Am_Register_Slot_Name ("CLASS_EXAMPLES");
  CLASS_REPRESENTATIVE = Am_Register_Slot_Name ("CLASS_REPRESENTATIVE");
  CLASS_SET_REPRESENTATIVE = Am_Register_Slot_Name ("CLASS_SET_REPRESENTATIVE");
  CLASS_LABEL = Am_Register_Slot_Name ("CLASS_LABEL");

  Class = Am_Group.Create ("Class")
    // model: the class name and list of examples (in small scale)
    .Add (CLASS_NAME, "")

    // view: a representative example with a label underneath it
    .Add_Part (CLASS_REPRESENTATIVE, 
 	       Example.Create ("Class_representative"))
    .Add (CLASS_SET_REPRESENTATIVE, set_representative)
    .Add_Part (CLASS_LABEL, Am_Text.Create ("Class_label")
	                     .Set (Am_TEXT, same_as_class_name))
    
    .Set (Am_WIDTH, 50)
    .Set (Am_HEIGHT, 50)
    .Set (Am_LAYOUT, Am_Vertical_Layout)
    .Set (Am_H_ALIGN, Am_LEFT_ALIGN)
    ;
}

Am_Object Class_Create (Am_String name)
{
  Am_Value_List l = Example_Create_List (trainer.Get_Examples (name));
  Am_Object cls = Class.Create () 
    .Set (CLASS_NAME, name)
    .Add (CLASS_EXAMPLES, l)
    ;

  return cls;
}

Am_Value_List Class_Create_List (Am_Value_List names)
{
  Am_Value_List classes;

  for (names.Start(); !names.Last(); names.Next())
    classes.Add (Class_Create (names.Get()));
  return classes;
}

////////////////////////////////////////////////////////////////
////  Error dialog boxes

Am_Object error_dialog;

void Popup_Error (Am_Value_List strings)
{
  if (!error_dialog.Valid()) {
    error_dialog = Am_Alert_Dialog.Create ("error_dialog")
      .Set (Am_VISIBLE, false);
    Am_Screen.Add_Part (error_dialog);
  }

  // add new message to error dialog
  error_dialog.Set (Am_ITEMS, strings);

  // show it
  Show_Dialog_Centered (error_dialog, window);
  Am_Show_Dialog_And_Wait (error_dialog);
}

////////////////////////////////////////////////////////////////
////  File-load dialog

Am_Object load_dialog;

// returns true if filename has examples in it
void Load_Trainer (char *filename, bool &save_with_examples)
{
  ifstream in(filename);
  if (!in) {
    Popup_Error (Am_Value_List()
		 .Add (Am_String(filename))
		 .Add (Am_String("can't be opened.")));
    return;
  }
   
  if (!(in >> trainer)) {
    Popup_Error (Am_Value_List()
		 .Add (Am_String(filename))
		 .Add (Am_String("is not a gesture classifier.")));
    return;
  }

  Am_Value_List class_names = trainer.Get_Class_Names ();
  Am_Value_List class_list = Class_Create_List (class_names);
  class_panel.Set (Am_ITEMS, class_list);
  class_panel.Set (Am_VALUE, 0);

  save_with_examples = false;
  for (class_list.Start(); !class_list.Last(); class_list.Next())
    if (trainer.Get_Examples (Am_Object(class_list.Get()) .Get (CLASS_NAME))){
      save_with_examples = true;
      break;
    }

  mode.Set (Am_VALUE, training_mode);
  dirty = false;
}

Am_Define_Method (Am_Object_Method, void, do_show_load_dialog, (Am_Object /*cmd*/))
{
  if (!load_dialog.Valid()) {
    load_dialog = Am_Text_Input_Dialog.Create ("load_dialog")
      .Set (Am_ITEMS, Am_Value_List ().Add ("Filename:"))
      .Set (Am_WIDTH, 200)
      .Set (Am_HEIGHT, 89)
      .Set (Am_TITLE, "Open Classifier")
      .Set (Am_ICON_TITLE, "Open Classifier")
      .Set(Am_FILL_STYLE, Am_Amulet_Purple)
      .Set (Am_VISIBLE, false);
    Am_Screen.Add_Part(load_dialog);
  }

  // load the current filename
  // load_dialog.Set (Am_VALUE, current_filename);

  // show it
  Show_Dialog_Centered (load_dialog, window);
  Am_Value fn;
  fn = Am_Show_Dialog_And_Wait (load_dialog);

  if (fn.Valid()) {
    current_filename = fn;
    Load_Trainer(current_filename, save_with_examples);
  }
}

////////////////////////////////////////////////////////////////
////  File-save and save-as dialog

Am_Object save_dialog, save_dialog_filename, save_dialog_format;
Am_String classifier_and_examples ("Classifier and examples");
Am_String classifier_only ("Classifier only");

extern Am_Object_Method do_show_save_dialog;  // forward declaration

void Save_Trainer (char *filename, bool save_with_examples)
{
  ofstream out(filename);

  if (save_with_examples) 
    out << trainer;
  else {
    Am_Gesture_Classifier cl = trainer.Train();
    out << cl;
  }

  if (!out)
    Popup_Error (Am_Value_List()
		 .Add (Am_String(filename))
		 .Add (Am_String("can't be written.")));
}
  

Am_Define_Method(Am_Object_Method, void, do_save_file, (Am_Object cmd))
{
  if (save_dialog.Valid()) {
    save_dialog.Set (Am_VISIBLE, false);
    current_filename = save_dialog_filename.Get (Am_VALUE);
    save_with_examples = (Am_String(save_dialog_format.Get (Am_VALUE))
			  == classifier_and_examples);
  }

  if (current_filename == "") {
    do_show_save_dialog.Call (cmd);
    return;
  }
  
  Save_Trainer(current_filename, save_with_examples);
  dirty = false;
}

Am_Define_Method(Am_Object_Method, void, do_hide_dialog, (Am_Object cmd))
{
  Am_Object obj; // the cancel button
  obj = cmd.Get_Owner();
  Am_Object window;
  window = obj.Get(Am_WINDOW);
  window.Set(Am_VISIBLE, false);
}

Am_Define_Method (Am_Object_Method, void, do_show_save_dialog, (Am_Object /*cmd*/))
{
  if (!save_dialog.Valid()) {
    Am_Object save_button, cancel_button, text_label;
    Am_Object c;
    save_dialog_filename = Am_Text_Input_Widget.Create("save_dialog_filename")
      .Set(Am_LEFT, 3)
      .Set(Am_TOP, 25)
      .Set(Am_WIDTH, 190)
      ;
    c = save_dialog_filename.Get(Am_COMMAND);
    c.Set(Am_LABEL, "")
      //.Set(Am_DO_METHOD, do_save_file)
      ;
    text_label = Am_Text.Create("save_dialog_label")
      .Set(Am_LEFT, 7)
      .Set(Am_TOP, 5)
      .Set(Am_TEXT, "Filename:")
      ;
    save_dialog_format = Am_Radio_Button_Panel. Create ("save_dialog_format")
      .Set (Am_ITEMS, Am_Value_List() 
	              .Add(classifier_and_examples)
	              .Add(classifier_only))
      .Set (Am_LAYOUT, Am_Vertical_Layout)
      .Set (Am_LEFT, 8)
      .Set (Am_TOP, 45)
      ;
    save_button = Am_Button.Create("save_dialog_save_button")
      .Set(Am_LEFT, 5)
      .Set(Am_TOP, 100)
      .Set(Am_WIDTH, 92)
      ;
    c = save_button.Get(Am_COMMAND);
    c.Set(Am_LABEL, "Save")
      .Set(Am_DO_METHOD, do_save_file)
      ;
    cancel_button = Am_Button.Create("save_dialog_cancel_button")
      .Set(Am_LEFT, 102)
      .Set(Am_TOP, 100)
      .Set(Am_WIDTH, 92)
      ;
    c = cancel_button.Get(Am_COMMAND);
    c.Set(Am_LABEL, "Cancel")
      .Set(Am_DO_METHOD, do_hide_dialog)
      ;
    save_dialog = Am_Window.Create("save_dialog")
      .Set(Am_LEFT, 100)
      .Set(Am_TOP, 100)
      .Set(Am_WIDTH, 200)
      .Set(Am_HEIGHT, 140)
      .Set (Am_TITLE, "Save Classifier")
      .Set (Am_ICON_TITLE, "Save Classifier")
      .Set(Am_FILL_STYLE, Am_Amulet_Purple)
      .Set(Am_VISIBLE, false)
      .Add_Part(text_label)
      .Add_Part(save_dialog_filename)
      .Add_Part(save_dialog_format)
      .Add_Part(save_button)
      .Add_Part(cancel_button)
      ;
    Am_Screen.Add_Part(save_dialog);
  }

  // save the current filename
  save_dialog_filename.Set (Am_VALUE, current_filename);
  save_dialog_format.Set (Am_VALUE, 
			  save_with_examples 
			  ? classifier_and_examples
			  : classifier_only);

  Show_Dialog_Centered (save_dialog, window);
}

////////////////////////////////////////////////////////////////
////  Confirming discard of unsaved classifier

Am_Object confirm_dialog;
Am_Slot_Key IF_CONFIRMED = Am_Register_Slot_Name ("IF_CONFIRMED");

Am_Define_Object_Formula (the_cmd_from_dialog)
{ 
  // the window is the widget
  return self.Get_Owner().Get_Object(Am_WINDOW).Get(IF_CONFIRMED);
}

Am_Define_Method(Am_Object_Method, void, do_hide_and_save, (Am_Object cmd))
{
  Am_Object obj; // the cancel button
  obj = cmd.Get_Owner();
  Am_Object window;
  window = obj.Get(Am_WINDOW);
  window.Set(Am_VISIBLE, false);

  do_save_file.Call (cmd);

  if (dirty)
    cmd.Set (Am_IMPLEMENTATION_PARENT, 0);
}

Am_Define_Method (Am_Object_Method, void, do_confirm, (Am_Object cmd))
{
  Am_Object goahead = cmd.Get (IF_CONFIRMED);

  if (!dirty) {
    Am_Object_Method (goahead.Get (Am_DO_METHOD)).Call (goahead);
    return;
  }

  if (!confirm_dialog.Valid()) {
    confirm_dialog = Am_Window.Create("confirm_dialog")
      .Set(Am_LEFT, 100)
      .Set(Am_TOP, 100)
      .Set(Am_WIDTH, Am_Width_Of_Parts)
      .Set(Am_HEIGHT, Am_Height_Of_Parts)
      .Set(Am_TITLE, "Save?")
      .Set(Am_ICON_TITLE, "Save?")
      .Set(Am_FILL_STYLE, Am_Amulet_Purple)
      .Set(Am_VISIBLE, false)
      .Add(IF_CONFIRMED, goahead)
      .Add_Part (Am_DIALOG_GROUP, Am_Widget_Group.Create ()
		 .Set (Am_LEFT, Am_Center_X_Is_Center_Of_Owner)
		 .Set (Am_TOP, Am_Center_Y_Is_Center_Of_Owner)
		 .Set (Am_WIDTH, Am_Width_Of_Parts)
		 .Set (Am_HEIGHT, Am_Height_Of_Parts)
		 .Set (Am_LAYOUT, Am_Vertical_Layout)
		 .Set (Am_V_SPACING, 10)
		 .Set (Am_TOP_OFFSET, 5)
		 .Set (Am_BOTTOM_OFFSET, 5)
		 .Add (Am_FILL_STYLE, Am_From_Owner (Am_FILL_STYLE))
		 .Add_Part (Am_Text.Create()
			    .Set (Am_TEXT, "The classifier has changed."))
		 .Add_Part (Am_Text.Create()
			    .Set (Am_TEXT, "Do you want to save it?"))
		 .Add_Part (Am_Button_Panel.Create()
			    .Set (Am_FIXED_WIDTH, true)
			    .Set (Am_LAYOUT, Am_Horizontal_Layout)
			    .Set (Am_H_SPACING, 10)
			    .Set (Am_LEFT_OFFSET, 5)
			    .Set (Am_RIGHT_OFFSET, 5)
			    .Set (Am_FILL_STYLE, Am_From_Owner(Am_FILL_STYLE))
			    .Set (Am_ITEMS, Am_Value_List()
				  .Add (Am_Command.Create ()
					.Set (Am_LABEL, "Save")
					.Set (Am_DO_METHOD, do_hide_and_save)
					.Set (Am_IMPLEMENTATION_PARENT, the_cmd_from_dialog))
				  .Add (Am_Command.Create ()
					.Set (Am_LABEL, "Don't Save")
					.Set (Am_DO_METHOD, do_hide_dialog)
					.Set (Am_IMPLEMENTATION_PARENT, the_cmd_from_dialog))
				  .Add (Am_Command.Create ()
					.Set (Am_LABEL, "Cancel")
					.Set (Am_DO_METHOD, do_hide_dialog))
				  )
			    )
		 )
      ;
    
    Am_Screen.Add_Part(confirm_dialog);
  }

  confirm_dialog.Set (IF_CONFIRMED, goahead);
  Show_Dialog_Centered (confirm_dialog, window);
}

////////////////////////////////////////////////////////////////
////  Class name editing

// Am_Define_Method(Am_Where_Method, Am_Object, over_name_of_selected_class,
// 		 (Am_Object /*inter*/, Am_Object object, 
// 		  Am_Object event_window,
// 		  Am_Input_Char /*ic*/, int x, int y))
// {
//   if (Am_Point_In_All_Owners(object, x, y, event_window)) {
//     Am_Object obj = Am_Point_In_Leaf (object, x, y, event_window);

//     if (obj.Valid() && obj.Is_Instance_Of(Am_Text) &&
// 	obj.Get_Owner() == class_panel.Get (Am_VALUE))
//       return obj;
//   }
//   return Am_No_Object;
  
// }

Am_Define_Method(Am_Object_Method, void, do_change_class_name, (Am_Object cmd))
{
  Am_String old_name = cmd.Get (Am_OLD_VALUE);
  Am_String new_name = cmd.Get (Am_VALUE);

//  cout << "Renaming class " << old_name << " to " << new_name << endl;

  if (old_name == NEW_CLASS_NAME && new_name == NEW_CLASS_NAME) {
    // user was entering name for a new class, but didn't change it.
    // Delete the new class
    extern Am_Object_Method do_delete_class;
    Am_Beep ();
    do_delete_class.Call (cmd);
    return;
  }

  if (old_name == new_name)
    return;

  // change name in trainer

  if (new_name == ""                                 // can't erase name
      || !trainer.Rename_Class (old_name, new_name)  // or duplicate another
      ) {
    Popup_Error (Am_Value_List()
		 .Add (Am_String("A class named"))
		 .Add (new_name)
		 .Add (Am_String("already exists.")));

    // undo edit to current_class_name widget
    current_class_name. Set (Am_VALUE, old_name);
    current_class_name. Get_Object (Am_COMMAND) .Set (Am_VALUE, old_name);
    return;
  }

  
  // now update Class object

  Am_Object cls = class_panel.Get (Am_VALUE);
  cls.Set (CLASS_NAME, new_name);

  // the classifier has changed, so set dirty flag
  dirty = true;
}

Am_Define_Method(Am_Object_Method, void, do_abort_change_class_name, (Am_Object inter))
{
  // call prototype's method
  Am_Object proto = inter.Get_Prototype();
  Am_Object_Method(proto.Get (Am_ABORT_DO_METHOD)).Call (inter);

  Am_String new_name = inter.Get (Am_OLD_VALUE);
  if (new_name == NEW_CLASS_NAME) {
    // user was entering name for a new class, but aborted.
    // Delete the new class
    extern Am_Object_Method do_delete_class;
    Am_Beep ();
    do_delete_class.Call (inter);
  }
}

////////////////////////////////////////////////////////////////
////  Gesturing commands

Am_Define_Method(Am_Object_Method, void, do_start_gesture, (Am_Object /*cmd*/))
{
  example_panel.Set (Am_VALUE, 0);

  if (Am_String(mode.Get (Am_VALUE)) != training_mode)
    class_panel.Set (Am_VALUE, 0);
}

void scroll_into_view (Am_Object cls)
{
  int cls_left = cls.Get (Am_LEFT);
  int cls_top = cls.Get (Am_TOP);
  Am_Translate_Coordinates (cls.Get_Owner(), cls_left, cls_top, 
			    class_scroller.Get_Owner(), cls_left, cls_top);

  int cls_height = cls.Get (Am_HEIGHT);
  int cls_bottom = cls_top + cls_height;

  int scroller_top = class_scroller.Get (Am_TOP);
  int scroller_height = class_scroller.Get (Am_HEIGHT);
  int scroller_bottom = scroller_top + scroller_height;

  int scroller_y_offset = class_scroller.Get (Am_Y_OFFSET);

  if (cls_top < scroller_top)
    class_scroller.Set (Am_Y_OFFSET, scroller_y_offset - (scroller_top - cls_top));
  else if (cls_bottom > scroller_bottom)
    class_scroller.Set (Am_Y_OFFSET, scroller_y_offset + (cls_bottom - scroller_bottom));
}

Am_Define_Method(Am_Object_Method, void, done_gesture, (Am_Object cmd)) 
{
  Am_Object inter = cmd.Get_Owner();
  Am_Point_List pl = inter.Get (Am_POINT_LIST);
  Am_Feature_Vector feat(pl);

  if (Am_String(mode.Get (Am_VALUE)) == training_mode) {
    Am_Object cls = class_panel.Get (Am_VALUE);
    if (cls.Valid()) {
      Am_String class_name = cls.Get (CLASS_NAME);
      Am_Value_List examples = cls.Get (CLASS_EXAMPLES);
      bool new_list = examples.Empty();
      
      trainer.Add_Example (class_name, feat);
      
      Am_Object example = Example_Create (feat);
      examples.Add (example, Am_TAIL, false);
      if (new_list)
	cls.Set (CLASS_EXAMPLES, examples);
      else
	cls.Note_Changed (CLASS_EXAMPLES);
      
      // select it?
      // example_panel.Set (Am_VALUE, example);

      // the classifier has changed, so set dirty flag
      dirty = true;
    }
  }

  else {
    // search for recognized class and select it
    Am_String recognized = cmd.Get (Am_VALUE);

    if (!recognized.Valid()) {
      // unrecognized!
      Am_Beep();
      class_panel.Set (Am_VALUE, 0);
    }
    else {
      // search for recognized class name
      Am_Value_List l = class_panel.Get (Am_ITEMS);
      for (l.Start(); !l.Last(); l.Next()) {
	Am_Object cls = l.Get();
	if (Am_String(cls.Get(CLASS_NAME)) == recognized) {
	  class_panel.Set (Am_VALUE, cls);
	  scroll_into_view (cls.Get_Owner());
	}
      }
    }
    example_panel.Set (Am_VALUE, 0);
  }
}

////////////////////////////////////////////////////////////////
////  Adding/deleting classes and examples

Am_Define_Method(Am_Object_Method, void, do_add_class, (Am_Object /*cmd*/)) 
{
  // create a class named NEW_CLASS_NAME (its name must be immediately changed by
  // the user)
  if (!trainer.Add_Class (NEW_CLASS_NAME)) {
    // new class already under construction -- select it
    Am_Value_List l = class_panel.Get (Am_ITEMS);
    l.End();
    Am_Object cls = l.Get();
    class_panel.Set (Am_VALUE, cls);
  }
  else {
    Am_Object cls = Class_Create (NEW_CLASS_NAME);
    
    // add the class to the class_panel
    Am_Value_List l = class_panel.Get (Am_ITEMS);
    bool new_list = l.Empty();
    l.Add (cls, Am_TAIL, false);
    if (new_list)
      class_panel.Set (Am_ITEMS, l);
    else
      class_panel.Note_Changed (Am_ITEMS);
    
    // select it
    class_panel.Set (Am_VALUE, cls);

    // the classifier has changed, so set dirty flag
    dirty = true;
  }

  mode.Set (Am_VALUE, training_mode);

  // start editing the class name
  Am_Start_Interactor (current_class_name.Get (Am_INTERACTOR),
		       current_class_name.Get (Am_TEXT_WIDGET_TEXT_OBJ));
}

Am_Define_Method(Am_Object_Method, void, do_delete_class, (Am_Object /*cmd*/)) 
{
  Am_Object cls = class_panel.Get (Am_VALUE);
  Am_String class_name = cls.Get (CLASS_NAME);

  trainer.Delete_Class (class_name);

  Am_Value_List l = class_panel.Get (Am_ITEMS);
  for (l.Start(); !l.Last(); l.Next())
    if (Am_Object(l.Get()) == cls)
      l.Delete(false);
  class_panel.Note_Changed (Am_ITEMS);
  
  class_panel.Set (Am_VALUE, 0);

  mode.Set (Am_VALUE, training_mode);

  // the classifier has changed, so set dirty flag
  dirty = true;
}

Am_Define_Method(Am_Object_Method, void, do_delete_example, (Am_Object /*cmd*/)) 
{
  Am_Object cls = class_panel.Get (Am_VALUE);
  Am_String class_name = cls.Get (CLASS_NAME);

  Am_Object example = example_panel.Get (Am_VALUE);
  Am_Feature_Vector feat = example.Get (EXAMPLE_FEATURES);

  // delete from trainer
  trainer.Delete_Example (class_name, feat);

  // and from agate's class rep
  Am_Value_List l = cls.Get (CLASS_EXAMPLES);
  for (l.Start(); !l.Last(); l.Next())
    if (Am_Object(l.Get()).Get (EXAMPLE_FEATURES) == feat)
      l.Delete(false);
  cls.Note_Changed (CLASS_EXAMPLES);

  mode.Set (Am_VALUE, training_mode);

  // the classifier has changed, so set dirty flag
  dirty = true;
}

////////////////////////////////////////////////////////////////
////  New classifier

Am_Define_Method(Am_Object_Method, void, do_new, (Am_Object /*cmd*/))
{
  trainer = Am_Gesture_Trainer ();

  current_filename = "";
  save_with_examples = true;

  class_panel.Set (Am_ITEMS, 0);
  class_panel.Set (Am_VALUE, 0);

  mode.Set (Am_VALUE, training_mode);

  dirty = false;
}

////////////////////////////////////////////////////////////////
//// Switching between training and recognizing

Am_Define_Method(Am_Object_Method, void, do_change_mode, (Am_Object cmd)) 
{
  Am_String new_mode = cmd.Get (Am_VALUE);

  if (new_mode == training_mode) {
    gesture_inter.Set (Am_CLASSIFIER, 0);
  }
  else {
    Am_Gesture_Classifier cl = trainer.Train();

    if (!cl.Trained()) {
      Popup_Error (Am_Value_List()
		   .Add (Am_String("This classifier can't recognize anything."))
		   .Add (Am_String("Please provide more examples.")));
      // set the mode back
      mode.Set(Am_VALUE, training_mode);
    }
    else
      gesture_inter.Set (Am_CLASSIFIER, cl);
  }
}

////////////////////////////////////////////////////////////////
////  Constraints

Am_Define_Value_List_Formula (examples_of_selected_class)
{
  // fetch current class's examples
  Am_Value_List l;
  Am_Object cls = class_panel.Get (Am_VALUE);

  if (cls.Valid())
    l = cls.Get (CLASS_EXAMPLES);

  // if currently-selected example is no longer in the list, unselect it
  Am_Object curr_example = self.Get (Am_VALUE);
  l.Start();
  if (!curr_example.Valid()
      || !l.Member (curr_example))
    self.Set (Am_VALUE, 0);

  // incidentally set the name into current_class_name widget
  Am_String class_name;
  if (cls.Valid())
    class_name = cls.Get (CLASS_NAME);
  else
    class_name = "";
  current_class_name. Set (Am_VALUE, class_name);
  current_class_name. Get_Object (Am_COMMAND) .Set (Am_VALUE, class_name);
    
  return l;
}

// Am_Define_No_Self_Formula (Am_Wrapper*,name_of_selected_class)
// {
//   Am_Value_List l;
//   Am_Object cls = class_panel.Get (Am_VALUE);

//   if (cls.Valid())
//     return Am_String(cls.Get (CLASS_NAME));
//   else
//     return Am_String("");
// }

Am_Define_Formula(int, show_current_example)
{
  Am_Object example = example_panel.Get (Am_VALUE);

  if (gesture_shown.Valid())
    self.Remove_Part (gesture_shown);

  if (example.Valid()) {
    gesture_shown = example.Copy ()
      .Set (Am_WIDTH, gesture_frame.Get (Am_WIDTH))
      .Set (Am_HEIGHT, gesture_frame.Get (Am_HEIGHT))
      ;
    self.Add_Part (gesture_shown);
  }
  else gesture_shown = 0;

  return 1;
}

Am_Define_No_Self_Formula(bool, if_class_selected)
{
  return class_panel.Get_Object (Am_VALUE).Valid();
}

Am_Define_No_Self_Formula(bool, if_example_selected)
{
  return example_panel.Get_Object (Am_VALUE).Valid();
}

Am_Define_Formula(int, width_of_window)
{
  Am_Object win = self.Get (Am_WINDOW);
  return win.Valid() ? (int)win.Get (Am_WIDTH) : 10;
}

Am_Define_Formula(int, width_of_owner)
{
  Am_Object own = self.Get_Owner ();
  return own.Valid() ? (int)own.Get (Am_WIDTH) : 10;
}

Am_Define_No_Self_Formula(int, compute_gesture_frame_height)
{
  return (int)window.Get (Am_HEIGHT) - 408;
}

Am_Define_No_Self_Formula(int, same_as_gesture_frame_width)
{
  return gesture_frame.Valid() ? (int)gesture_frame.Get (Am_WIDTH) : 10;
}

Am_Define_No_Self_Formula(int, same_as_gesture_frame_height)
{
  return gesture_frame.Valid() ? (int)gesture_frame.Get (Am_HEIGHT) : 10;
}

////////////////////////////////////////////////////////////////
////  main

int main (int argc, char *argv[])
{
  Am_Initialize ();

  polyline = Am_Polygon.Create ("polyline") .Set (Am_FILL_STYLE, 0);

  // initialize the model objects
  Example_Init();
  Class_Init();

  // Am_Object undo_handler = Am_Multiple_Undo_Object.Create ("multi_undo");

  window = Am_Window.Create ("window")
    .Set (Am_LEFT, 50)
    .Set (Am_TOP,  50)
    .Set (Am_WIDTH, 500)
    .Set (Am_HEIGHT, 700)
    .Set (Am_TITLE, "Agate Gesture Trainer")
    .Set (Am_ICON_TITLE, "Agate Gesture Trainer")
    .Set (Am_FILL_STYLE, Am_Motif_Gray)
    //.Set (Am_UNDO_HANDLER, undo_handler)
    ;
  
  Am_Object amulet_icon = Am_Bitmap.Create("Amulet_Icon")
    .Set (Am_IMAGE,   Am_Image_Array(Am_Merge_Pathname(AMULET_BITMAP)))
    .Set (Am_LINE_STYLE, Am_Black)
    ;									   
  menu_bar = Am_Menu_Bar.Create("menu_bar")
      .Set(Am_ITEMS, Am_Value_List ()
	 .Add (Am_Command.Create("Amulet_Command")
	       .Set(Am_IMPLEMENTATION_PARENT, true) //not undoable
	       .Set(Am_LABEL, amulet_icon)
	       .Set(Am_ITEMS, Am_Value_List ()
                     .Add ("About Agate...")
		    )
	       .Set(Am_ACTIVE, false)
	       )
	 .Add (Am_Command.Create("File_Command")
	       .Set(Am_IMPLEMENTATION_PARENT, true) //not undoable
	       .Set(Am_LABEL, "File")
	       .Set(Am_ITEMS, Am_Value_List ()
                     .Add (Am_Command.Create("New")
			   .Set(Am_LABEL, "New")
			   .Set(Am_ACTIVE, true)
			   .Set(Am_DO_METHOD, do_confirm)
			   .Add (IF_CONFIRMED,
				 Am_Command.Create ("new")
				 .Set (Am_DO_METHOD, do_new))
			   .Set (Am_ACCELERATOR, "CONTROL_n"))
                     .Add (Am_Command.Create("Open...")
			   .Set(Am_LABEL, "Open...")
			   .Set(Am_ACTIVE, true)
			   .Set(Am_DO_METHOD, do_confirm)
			   .Add (IF_CONFIRMED,
				 Am_Command.Create ("load")
				 .Set (Am_DO_METHOD, do_show_load_dialog))
			   .Set (Am_ACCELERATOR, "CONTROL_o"))
                     .Add (Am_Command.Create("Save As...")
			   .Set(Am_LABEL, "Save As...")
			   .Set(Am_ACTIVE, true)
			   .Set(Am_DO_METHOD, do_show_save_dialog))
                     .Add (Am_Command.Create("Save_Command")
                           .Set(Am_LABEL, "Save")
			   .Set(Am_ACTIVE, true)
			   .Set(Am_DO_METHOD, do_save_file)
			   .Set(Am_ACCELERATOR, "CONTROL_s"))
                     .Add (Am_Command.Create ("Quit")
			   .Set (Am_LABEL, "Quit")
			   .Set (Am_ACTIVE, true)
			   .Set (Am_DO_METHOD, do_confirm)
			   .Add (IF_CONFIRMED, Am_Quit_No_Ask_Command)
			   .Set (Am_ACCELERATOR, "CONTROL_q"))
		     )
	       )
// 	   .Add (Am_Command.Create("Edit_Command")
// 		 .Set(Am_LABEL, "Edit")
// 		 .Set(Am_IMPLEMENTATION_PARENT, true) //not undoable
// 		 .Set(Am_ITEMS, Am_Value_List ()
// 		      .Add (Am_Undo_Command.Create())
// 		      .Add (Am_Undo_The_Undo_Command.Create())
// 		      )
// 		 )
	   )
  ;

  // Window has three panes:
  //  
  //      Classes
  //    -------------
  //      Examples
  //    -------------
  //      Gesture

  // Construct the class pane
//   class_name_editor = 
//     Am_Text_Edit_Interactor.Create ("class_name_editor")
//     .Set (Am_PRIORITY, 50)
//     .Set (Am_START_WHERE_TEST, over_name_of_selected_class)
//     ;
//   class_name_editor.Get_Object (Am_COMMAND)
//     .Set (Am_DO_METHOD, do_change_class_name)
//     ;
  class_panel = Am_Button_Panel.Create ("class_panel")
    .Set (Am_FIXED_WIDTH, true)
    .Set (Am_FIXED_HEIGHT, true)
    .Set (Am_LAYOUT, Am_Horizontal_Layout)
    .Set (Am_MAX_SIZE, width_of_owner)
    .Set (Am_FINAL_FEEDBACK_WANTED, true)
    .Set (Am_FILL_STYLE, Am_White)
    .Set (Am_HOW_SET, Am_CHOICE_SET)
    ;
  class_scroller = Am_Scrolling_Group.Create ("class_scroller")
    .Set (Am_INNER_WIDTH, Am_Width_Of_Parts)
    .Set (Am_INNER_HEIGHT, Am_Height_Of_Parts)
    .Set (Am_H_SCROLL_BAR, false)
    .Set (Am_V_SCROLL_BAR, true)
    .Set (Am_WIDTH, width_of_window)
    .Set (Am_HEIGHT, 65)
    .Set (Am_FILL_STYLE, Am_Motif_Gray)
    .Add_Part (class_panel);
    ;
  Am_Object class_control = Am_Button_Panel.Create ("class_control")
    .Set (Am_ITEMS, 
	  Am_Value_List()
	  .Add (Am_Command.Create ("add_class")
		.Set (Am_LABEL, "Add class")
		.Set (Am_DO_METHOD, do_add_class))
	  .Add (Am_Command.Create ("delete_class")
		.Set (Am_LABEL, "Delete class")
		.Set (Am_ACTIVE, if_class_selected)
		.Set (Am_DO_METHOD, do_delete_class)))
    .Set (Am_LAYOUT, Am_Horizontal_Layout)
    .Set (Am_H_SPACING, 10)
    ;
  class_pane = Am_Group.Create ("class_pane")
    .Set (Am_LAYOUT, Am_Vertical_Layout)
    .Set (Am_TOP_OFFSET, 5)
    .Set (Am_V_SPACING, 5)
    .Set (Am_WIDTH, Am_Width_Of_Parts)
    .Set (Am_HEIGHT, Am_Height_Of_Parts)
    .Add_Part (class_scroller)
    .Add_Part (class_control)
//     .Add_Part (class_name_editor)
    ;

  // Construct the example pane
  current_class_name = Am_Text_Input_Widget.Create ("current_class_name")
    .Set (Am_WIDTH, 200)
    .Set (Am_ACTIVE, if_class_selected)
    ;
  current_class_name.Get_Object (Am_INTERACTOR)
    .Set (Am_ABORT_DO_METHOD, do_abort_change_class_name)
    ;
  current_class_name.Get_Object (Am_COMMAND)
    .Set (Am_LABEL, "Class: ")
    .Set (Am_DO_METHOD, do_change_class_name)
    ;
  example_panel = Am_Button_Panel.Create ("example_panel")
    .Set (Am_ITEMS, examples_of_selected_class)
    .Set (Am_FIXED_WIDTH, true)
    .Set (Am_FIXED_HEIGHT, true)
    .Set (Am_MAX_SIZE, width_of_owner) 
    .Set (Am_LAYOUT, Am_Horizontal_Layout)
    .Set (Am_FINAL_FEEDBACK_WANTED, true)
    .Set (Am_FILL_STYLE, Am_White)
    .Set (Am_HOW_SET, Am_CHOICE_SET)
    ;
  example_scroller = Am_Scrolling_Group.Create ("example_scroller")
    .Set (Am_INNER_WIDTH, Am_Width_Of_Parts)
    .Set (Am_INNER_HEIGHT, Am_Height_Of_Parts)
    .Set (Am_H_SCROLL_BAR, false)
    .Set (Am_V_SCROLL_BAR, true)
    .Set (Am_WIDTH, width_of_window)
    .Set (Am_HEIGHT, 130)
    .Set (Am_FILL_STYLE, Am_Motif_Gray)
    .Add_Part (example_panel)
    ;
  Am_Object example_control = Am_Button_Panel.Create ("example_control")
    .Set (Am_ITEMS, 
	  Am_Value_List()
	  .Add (Am_Command.Create ("delete_example")
		.Set (Am_LABEL, "Delete example")
		.Set (Am_ACTIVE, if_example_selected)
		.Set (Am_DO_METHOD, do_delete_example)))
    .Set (Am_LAYOUT, Am_Horizontal_Layout)
    ;
  example_pane = Am_Group.Create ("example_pane")
    .Set (Am_LAYOUT, Am_Vertical_Layout)
    .Set (Am_TOP_OFFSET, 5)
    .Set (Am_V_SPACING, 5)
    .Set (Am_WIDTH, Am_Width_Of_Parts)
    .Set (Am_HEIGHT, Am_Height_Of_Parts)
    .Add_Part (current_class_name)
    .Add_Part (example_scroller)
    .Add_Part (example_control)
    ;

  
  // Construct the mode

  mode = Am_Radio_Button_Panel. Create ("mode")
    .Set (Am_ITEMS, Am_Value_List() 
	            .Add(training_mode)
	            .Add(recognizing_mode))
    .Set (Am_LAYOUT, Am_Horizontal_Layout)
    .Set (Am_H_SPACING, 30)
    .Set (Am_VALUE, training_mode)
    ;
  mode.Get_Object (Am_COMMAND)
    .Set (Am_DO_METHOD, do_change_mode)
    ;

  // Construct the gesture pane

  // black frame around inner edge of gesture pane
  // (otherwise it would be invisible)
  gesture_frame = Am_Rectangle.Create ("gesture_frame")
    .Set (Am_FILL_STYLE, Am_White)
    .Set (Am_WIDTH, width_of_window)
    .Set (Am_HEIGHT, compute_gesture_frame_height)
    ;
  // feedback for gesture interactor
  Am_Object gesture_feedback = Am_Polygon.Create ("gesture_feedback")
    .Set (Am_FILL_STYLE, NULL)
    ;

  gesture_inter = Am_Gesture_Interactor.Create ("gesture_inter")
    .Set (Am_FEEDBACK_OBJECT, gesture_feedback)
    ;
  gesture_inter.Get_Object (Am_COMMAND)
    .Set (Am_START_DO_METHOD, do_start_gesture)
    .Set (Am_DO_METHOD, done_gesture)
    ;

  int GESTURE_SHOW_EXAMPLE = Am_Register_Slot_Name ("GESTURE_SHOW_EXAMPLE");
  gesture_pane = Am_Group.Create ("gesture_pane")
    .Add (GESTURE_SHOW_EXAMPLE, show_current_example)
    .Set (Am_WIDTH, same_as_gesture_frame_width)
    .Set (Am_HEIGHT, same_as_gesture_frame_height)
    .Add_Part (gesture_frame)
    .Add_Part (gesture_inter)
    .Add_Part (gesture_feedback)
    ;

  // Lay out the panes vertically in the window

  window_layout = Am_Group.Create ("window_layout")
    .Set (Am_LAYOUT, Am_Vertical_Layout)
    .Set (Am_TOP, 28)
    .Set (Am_WIDTH, Am_Width_Of_Parts)
    .Set (Am_HEIGHT, Am_Height_Of_Parts)
    .Set (Am_V_SPACING, 20)
    .Add_Part (class_pane)
    .Add_Part (example_pane)
    .Add_Part (mode)
    .Add_Part (gesture_pane)
    ;

  window
    .Add_Part (menu_bar)
    .Add_Part (window_layout)
    ; 
  Am_Screen.Add_Part (window);

  if (argc > 1) {
    current_filename = argv[1];
    Load_Trainer (current_filename, save_with_examples);
  }

  // enter the event loop
  Am_Main_Event_Loop ();
  Am_Cleanup ();

  return 0;
}
