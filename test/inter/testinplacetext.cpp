/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <am_inc.h>
#include INTER_ADVANCED__H
#include TEXT_FNS__H

#include WIDGETS_ADVANCED__H
#include OPAL_ADVANCED__H
#include GEM__H

#include <amulet.h>

/////// Interactor Stuff //////////

Am_Slot_Key Am_INPLACE_TEXT_INFO =
            Am_Register_Slot_Name ("~INPLACE_TEXT_INFO~");

Am_Slot_Key Am_INPLACE_TEXT_NAV =
            Am_Register_Slot_Name ("~INPLACE_TEXT_NAV~");

Am_Slot_Key Am_AFFECTING_SLOTS =
            Am_Register_Slot_Name ("~AFFECTING_SLOTS~");

Am_Slot_Key Am_RESET_INPLACE_FEEDBACK =
            Am_Register_Slot_Name ("~RESET_INPLACE_FEEDBACK~");

Am_Slot_Key Am_RESET_INPLACE_TEXT =
            Am_Register_Slot_Name ("~RESET_INPLACE_TEXT~");

Am_Slot_Key Am_TEXT_X_OFFSET =
            Am_Register_Slot_Name ("~TEXT_X_OFFSET~");

Am_Slot_Key Am_TEXT_Y_OFFSET =
            Am_Register_Slot_Name ("~TEXT_Y_OFFSET~");

class Am_Text_Navigate;

class Am_Text_Info {
 public:
  virtual void Release () = 0;

  virtual void Get_Info (int& offset_x, int& offset_y, Am_String& string,
			 Am_Style& style, Am_Font& font,
			 Am_Value_List& changing_slots) = 0;

  virtual void Set_Text (const Am_String& string) = 0;

  virtual Am_Text_Navigate* Get_Navigator () = 0;

  static Am_Text_Info* Narrow (void* value)
  {
    return (Am_Text_Info*)value;
  }
};

enum Am_Inplace_Text_Direction {
  Am_INPLACE_UP, Am_INPLACE_DOWN,
  Am_INPLACE_LEFT, Am_INPLACE_RIGHT,
  Am_INPLACE_PREV, Am_INPLACE_NEXT,
  Am_INPLACE_DONE };

class Am_Text_Navigate {
 public:
  virtual bool Move (Am_Inplace_Text_Direction direction) = 0;

#ifdef AMULET2_INSTRUMENT
#undef Add
#endif
  virtual void Add (const Am_String& text, Am_Insert_Position position) = 0;
#ifdef AMULET2_INSTRUMENT
  Am_Instrumentation (Am_Text_Navigate)
#define Add Am_Instrumented(Add)
#endif
  virtual bool Delete (Am_Inplace_Text_Direction where_to_go) = 0;

  static Am_Text_Navigate* Narrow (void* value)
  {
    return (Am_Text_Navigate*)value;
  }
};

AM_DEFINE_METHOD_TYPE (Am_Inplace_Info_Method, Am_Text_Info*,
		       (const Am_Object& self, const Am_Object& ref_object,
			int x, int y))

AM_DEFINE_METHOD_TYPE_IMPL (Am_Inplace_Info_Method);

Am_Object Am_Inplace_Text_Edit_Interactor;
Am_Object Am_Inplace_Text_Feedback;

static Am_Object test_inplace_text (Am_Object& inter,
				    const Am_Object& object,
				    const Am_Object& event_window,
				    int x, int y)
{
  Am_Value info_value;
  info_value=object.Peek(Am_INPLACE_TEXT_INFO);
  if (info_value.Valid ()) {
    Am_Inplace_Info_Method info_test (info_value);
    Am_Text_Info* text_info = info_test.Call (object, event_window, x, y);
    if (text_info) {
      inter.Set (Am_INPLACE_TEXT_INFO, (Am_Ptr)text_info);
      Am_Text_Navigate* text_nav = text_info->Get_Navigator ();
      if (text_nav)
        inter.Set (Am_INPLACE_TEXT_NAV, (Am_Ptr)text_nav);
      return object;
    }
  }
  return Am_No_Object;
}

Am_Define_Method (Am_Where_Method, Am_Object, Am_Inter_In_Inplace_Text,
		  // point in text object?
		  (Am_Object inter, Am_Object object, Am_Object event_window,
		   int x, int y))
{
  Am_INTER_TRACE_PRINT (inter, " --testing in inplace text " << object);
  if (Am_Point_In_All_Owners (object, x, y, event_window) &&
      Am_Point_In_Obj (object, x, y, event_window).Valid ())
    return test_inplace_text (inter, object, event_window, x, y);
  return Am_No_Object;
}

Am_Define_Method (Am_Where_Method, Am_Object, Am_Inter_In_Inplace_Text_Part,
		  // point in text part?
		  (Am_Object inter, Am_Object object, Am_Object event_window,
		   int x, int y))
{
  Am_INTER_TRACE_PRINT (inter, " --testing inplace text part of " << object);
  if (Am_Point_In_All_Owners (object, x, y, event_window)) {
    Am_Object obj = Am_Point_In_Part (object, x, y, event_window);
    if (obj.Valid ())
      return test_inplace_text (inter, obj, event_window, x, y);
  }
  return Am_No_Object;
}

inline bool group_like (Am_Object object)
{
  return (object.Is_Instance_Of(Am_Group) || object.Is_Instance_Of(Am_Map)
	  || object.Is_Instance_Of(Am_Window)
	  || object.Is_Instance_Of(Am_Screen));
}

// if attached to a group, then point in part?  else point in object?
Am_Define_Method (Am_Where_Method, Am_Object,
		  Am_Inter_In_Inplace_Text_Object_Or_Part,
		  (Am_Object inter, Am_Object object, Am_Object event_window,
		   int x, int y))
{
  if (group_like (object)) {
    Am_INTER_TRACE_PRINT (inter, " --attached to group, so testing "
			  "inplace text part of " << object);
    if (Am_Point_In_All_Owners (object, x, y, event_window)) {
      Am_Object obj = Am_Point_In_Part (object, x, y, event_window);
      if (obj.Valid ())
        return test_inplace_text (inter, obj, event_window, x, y);
    }
  }
  else {
    Am_INTER_TRACE_PRINT (inter, " --attached to non-group, so testing "
			  "in inplace text "
			  << object);
    if (Am_Point_In_All_Owners (object, x, y, event_window) &&
	Am_Point_In_Obj (object, x, y, event_window).Valid ())
      return test_inplace_text (inter, object, event_window, x, y);
  }
  return Am_No_Object;
}

Am_Define_Method (Am_Where_Method, Am_Object, Am_Inter_In_Inplace_Text_Leaf,
		  // point in text part?
		  (Am_Object inter, Am_Object object, Am_Object event_window,
		   int x, int y))
{
  Am_INTER_TRACE_PRINT (inter, " --testing inplace text leaf of " << object);
  if (Am_Point_In_All_Owners (object, x, y, event_window)) {
    Am_Object obj = Am_Point_In_Leaf (object, x, y, event_window);
    if (obj.Valid ())
      return test_inplace_text (inter, obj, event_window, x, y);
  }
  return Am_No_Object;
}

Am_Define_Method (Am_Object_Method, void, Am_Inplace_Text_Do,
		  (Am_Object command))
{
  Am_Object inter = command.Get_Owner ();
  Am_Object feedback = inter.Get (Am_FEEDBACK_OBJECT);
  if (!feedback.Valid ())
    return;
  Am_Object operates_on = inter.Get (Am_START_OBJECT);
  feedback.Set (Am_VISIBLE, true);
  feedback.Set (Am_OPERATES_ON, operates_on);
  feedback.Set (Am_INPLACE_TEXT_INFO, inter.Get (Am_INPLACE_TEXT_INFO));
  feedback.Set (Am_INPLACE_TEXT_NAV, inter.Get (Am_INPLACE_TEXT_NAV));
  feedback.Get (Am_RESET_INPLACE_FEEDBACK); // To update feedback constraints
  Am_Object text_inter = feedback.Get_Object (Am_INTERACTOR);
  Am_Object event_window = inter.Get (Am_WINDOW);
  int x = inter.Get (Am_FIRST_X);
  int y = inter.Get (Am_FIRST_Y);
  Am_Start_Interactor (text_inter, feedback, Am_Default_Start_Char,
		       event_window, x, y, false);
}

Am_Define_Formula (int, inplace_feedback_left)
{
  Am_Object operates_on = self.Get_Object(Am_OPERATES_ON);
  if (operates_on.Valid ()) {
    int x, y;
    Am_Translate_Coordinates (operates_on, 0, 0, self.Get_Owner (), x, y);
    return x + (int)self.Get (Am_TEXT_X_OFFSET);
  }
  return 0;
}

Am_Define_Formula (int, inplace_feedback_top)
{
  Am_Object operates_on = self.Get_Object (Am_OPERATES_ON);
  if (operates_on.Valid ()) {
    int x, y;
    Am_Translate_Coordinates (operates_on, 0, 0, self.Get_Owner (), x, y);
    return y + (int)self.Get (Am_TEXT_Y_OFFSET);
  }
  return 0;
}

Am_Define_Formula (int, inplace_text_resetter)
{
  Am_Text_Info* text_info =
            Am_Text_Info::Narrow ((Am_Ptr)self.Get (Am_INPLACE_TEXT_INFO));

  if (!text_info)
    return 0;

  Am_String string = self.Get (Am_TEXT);
  text_info->Set_Text (string);
  return 0;
}

Am_Define_Formula (int, inplace_feedback_resetter)
{
  Am_Text_Info* text_info =
            Am_Text_Info::Narrow ((Am_Ptr)self.Get (Am_INPLACE_TEXT_INFO));

  if (!text_info)
    return 0;
  int offset_x, offset_y;
  Am_String string;
  Am_Style style;
  Am_Font font;
  Am_Value_List changing_slots;
  text_info->Get_Info (offset_x, offset_y, string, style, font,
		       changing_slots);
  self
    .Set (Am_TEXT_X_OFFSET, offset_x)
    .Set (Am_TEXT_Y_OFFSET, offset_y)
    .Set (Am_TEXT, string)
    //.Set (Am_LINE_STYLE, style)
    .Set (Am_LINE_STYLE, Am_Yellow)
    .Set (Am_FONT, font)
    .Set (Am_AFFECTING_SLOTS, changing_slots)
  ;

  Am_Object object;
  Am_Slot_Key key;
  for (changing_slots.Start (); !changing_slots.Last ();
       changing_slots.Next ()) {
    object = changing_slots.Get ();
    changing_slots.Next ();
    key = (int)changing_slots.Get ();
    object.Get (key);
  }
  return 0;
}

Am_Define_Method (Am_Object_Method, void, Am_Inplace_Editor_Abort_Do,
		  (Am_Object inter))
{
  Am_Object_Method method = Am_Text_Edit_Interactor.Get (Am_ABORT_DO_METHOD);
  method.Call (inter); // Call prototype method.

  Am_Object feedback = inter.Get_Owner ();
  feedback.Set (Am_VISIBLE, false);
  Am_Text_Info* text_info =
            Am_Text_Info::Narrow ((Am_Ptr)feedback.Get (Am_INPLACE_TEXT_INFO));
  text_info->Release ();
  feedback.Set (Am_INPLACE_TEXT_INFO, false);
  feedback.Set (Am_INPLACE_TEXT_NAV, false);
  inter.Set (Am_INPLACE_TEXT_INFO, false);
  inter.Set (Am_INPLACE_TEXT_NAV, false);
}

Am_Define_Method (Am_Object_Method, void, Am_Inplace_Editor_Do,
		  (Am_Object inter))
{
  Am_Object_Method method = Am_Text_Edit_Interactor.Get (Am_DO_METHOD);
  method.Call (inter); // Call prototype method.

  Am_Object feedback = inter.Get_Owner ();
  feedback.Set (Am_VISIBLE, false);
  Am_Text_Info* text_info =
            Am_Text_Info::Narrow ((Am_Ptr)feedback.Get (Am_INPLACE_TEXT_INFO));
  text_info->Release ();
}

static void Am_Inplace_Move_Cursor_Right (Am_Object text)
{
  int str_len = strlen (Am_String (text.Get (Am_TEXT)));
  int index = text.Get (Am_CURSOR_INDEX);
  if (index < str_len)
    Am_Move_Cursor_Right (text, 1);
  else {
    Am_Text_Navigate* text_nav =
          Am_Text_Navigate::Narrow ((Am_Ptr)text.Get (Am_INPLACE_TEXT_NAV));
    if (text_nav) {
      Am_String value = text.Get (Am_TEXT);
      bool success;
      if (value == "")
        success = text_nav->Delete (Am_INPLACE_RIGHT);
      else
        success = text_nav->Move (Am_INPLACE_RIGHT);
      text.Note_Changed (Am_INPLACE_TEXT_INFO);
      if (success)
        Am_Move_Cursor_To_Start (text);
    }
  }
}

static void Am_Inplace_Move_Cursor_Left (Am_Object text)
{
  //int str_len = strlen (Am_String (text.Get (Am_TEXT)));
  int index = text.Get (Am_CURSOR_INDEX);
  if (index > 0)
    Am_Move_Cursor_Left (text, 1);
  else {
    Am_Text_Navigate* text_nav =
          Am_Text_Navigate::Narrow ((Am_Ptr)text.Get (Am_INPLACE_TEXT_NAV));
    if (text_nav) {
      Am_String value = text.Get (Am_TEXT);
      bool success;
      if (value == "")
        success = text_nav->Delete (Am_INPLACE_LEFT);
      else
        success = text_nav->Move (Am_INPLACE_LEFT);
      text.Note_Changed (Am_INPLACE_TEXT_INFO);
      if (success)
        Am_Move_Cursor_To_End (text);
    }
  }
}

static void Am_Inplace_Move_Cursor_Up (Am_Object text)
{
  Am_Text_Navigate* text_nav =
        Am_Text_Navigate::Narrow ((Am_Ptr)text.Get (Am_INPLACE_TEXT_NAV));
  if (text_nav) {
    Am_String value = text.Get (Am_TEXT);
    bool success;
    if (value == "")
      success = text_nav->Delete (Am_INPLACE_UP);
    else
      success = text_nav->Move (Am_INPLACE_UP);
    text.Note_Changed (Am_INPLACE_TEXT_INFO);
    if (success)
      Am_Move_Cursor_To_Start (text);
  }
}

static void Am_Inplace_Move_Cursor_Down (Am_Object text)
{
  Am_Text_Navigate* text_nav =
        Am_Text_Navigate::Narrow ((Am_Ptr)text.Get (Am_INPLACE_TEXT_NAV));
  if (text_nav) {
    Am_String value = text.Get (Am_TEXT);
    bool success;
    if (value == "")
      success = text_nav->Delete (Am_INPLACE_DOWN);
    else
      success = text_nav->Move (Am_INPLACE_DOWN);
    text.Note_Changed (Am_INPLACE_TEXT_INFO);
    if (success)
      Am_Move_Cursor_To_Start (text);
  }
}

static void Am_Inplace_Move_Next (Am_Object text)
{
  Am_Text_Navigate* text_nav =
        Am_Text_Navigate::Narrow ((Am_Ptr)text.Get (Am_INPLACE_TEXT_NAV));
  if (text_nav) {
    Am_String value = text.Get (Am_TEXT);
    bool success;
    if (value == "")
      success = text_nav->Delete (Am_INPLACE_NEXT);
    else
      success = text_nav->Move (Am_INPLACE_NEXT);
    text.Note_Changed (Am_INPLACE_TEXT_INFO);
    if (success)
      Am_Move_Cursor_To_Start (text);
  }
}

static void Am_Inplace_Move_Prev (Am_Object text)
{
  Am_Text_Navigate* text_nav =
        Am_Text_Navigate::Narrow ((Am_Ptr)text.Get (Am_INPLACE_TEXT_NAV));
  if (text_nav) {
    Am_String value = text.Get (Am_TEXT);
    bool success;
    if (value == "")
      success = text_nav->Delete (Am_INPLACE_PREV);
    else
      success = text_nav->Move (Am_INPLACE_PREV);
    text.Note_Changed (Am_INPLACE_TEXT_INFO);
    if (success)
      Am_Move_Cursor_To_End (text);
  }
}

static void Am_Inplace_Add_Item (Am_Object text)
{
  Am_Text_Navigate* text_nav =
        Am_Text_Navigate::Narrow ((Am_Ptr)text.Get (Am_INPLACE_TEXT_NAV));
  if (text_nav) {
    Am_String str ("");
    int index = text.Get (Am_CURSOR_INDEX);
    Am_Insert_Position position;
    if (index == 0)
      position = Am_BEFORE;
    else
      position = Am_AFTER;

    text_nav->Add (str, position);
  }
}

static void init_proto ()
{
  Am_Inplace_Text_Edit_Interactor = Am_One_Shot_Interactor
    .Create ("Inplace Interactor")
    .Set (Am_START_WHERE_TEST, Am_Inter_In_Inplace_Text_Object_Or_Part)
    .Add (Am_FEEDBACK_OBJECT, Am_No_Object)
    .Add (Am_INPLACE_TEXT_INFO, false)
    .Add (Am_INPLACE_TEXT_NAV, false)
    .Get_Object (Am_COMMAND)
      .Set_Name ("Inplace Text Select Command")
      .Set (Am_DO_METHOD, Am_Inplace_Text_Do)
      .Set (Am_IMPLEMENTATION_PARENT, true)  // Do not queue for undo
      .Get_Owner ()
  ;

  Am_Edit_Translation_Table table =
       Am_Edit_Translation_Table::Default_Table ();

  table.Add ("RIGHT_ARROW",  Am_Inplace_Move_Cursor_Right);
  table.Add ("LEFT_ARROW",   Am_Inplace_Move_Cursor_Left);
  table.Add ("UP_ARROW",     Am_Inplace_Move_Cursor_Up);
  table.Add ("DOWN_ARROW",   Am_Inplace_Move_Cursor_Down);
  table.Add ("CONTROL_f",    Am_Inplace_Move_Cursor_Right);
  table.Add ("CONTROL_b",    Am_Inplace_Move_Cursor_Left);
  table.Add ("CONTROL_p",    Am_Inplace_Move_Cursor_Up);
  table.Add ("CONTROL_n",    Am_Inplace_Move_Cursor_Down);
  table.Add ("TAB",          Am_Inplace_Move_Next);
  table.Add ("SHIFT_TAB",    Am_Inplace_Move_Prev);
  table.Add ("SHIFT_RETURN", Am_Inplace_Add_Item);

  Am_Inplace_Text_Feedback = Am_Text.Create ("Inplace Feedback Object")
    .Set (Am_VISIBLE, false)
    .Set (Am_TEXT, "DUMMY_STRING")
    .Add (Am_OPERATES_ON, Am_No_Object)
    .Add (Am_TEXT_X_OFFSET, 0)
    .Add (Am_TEXT_Y_OFFSET, 0)
    .Set (Am_LEFT, inplace_feedback_left)
    .Set (Am_TOP, inplace_feedback_top)
    .Add (Am_RESET_INPLACE_TEXT, inplace_text_resetter)
    .Add (Am_RESET_INPLACE_FEEDBACK, inplace_feedback_resetter)
    .Add (Am_AFFECTING_SLOTS, Am_No_Value_List)
    .Add (Am_INPLACE_TEXT_INFO, false)
    .Add (Am_INPLACE_TEXT_NAV, false)
    .Add_Part (Am_INTERACTOR, Am_Text_Edit_Interactor.Create ()
      .Set (Am_EDIT_TRANSLATION_TABLE, table)
      .Set (Am_ABORT_DO_METHOD, Am_Inplace_Editor_Abort_Do)
      .Set (Am_DO_METHOD, Am_Inplace_Editor_Do)
    )
  ;
}

/////// Widget Stuff //////////

// Finds the position of the text in a single Motif Button
static void Am_Motif_Button_Text_Info (Am_Object self,
    int width, int height, const char* string, Am_Font font,
    Am_Button_Type but_type, int /*box_height*/, int box_width,
    bool box_on_left, int align, int offset_left,
    int& text_left, int& text_top, int& text_width, int& text_height)
{
  //  int box_left, box_top;
  int real_left, real_top, real_width, real_height;

  switch (but_type) {
  case Am_PUSH_BUTTON:
    real_left = 4;
    real_width = width - 8;
    real_top = 4;
    real_height = height - 8;
    break;
  case Am_CHECK_BUTTON:
    // on left if box_on_left; on right otherwise.  Center vertically.
    // leave 2 pixel border for the key-selected box
    // selected and interim selected look the same in Motif  ??
    real_left = box_on_left ? box_width + 3 : 2;
    real_width = width - box_width - 4;
    real_top = 0;
    real_height = height;
    break;
  case Am_RADIO_BUTTON:
    // on left if box_on_left; on right otherwise.  Center vertically.
    // leave 2 pixel border for the key-selected box
    // selected and interim selected look the same in Motif  ??
    real_left = box_on_left ? box_width + 3 : 2;
    real_width = width - box_width - 4;
    real_top = 0;
    real_height = height;
    break;
  default:
    Am_Error ("Am_Draw_Motif_Button: incorrect button type.\n");
  }

  int ascent, descent, a, b;

  Am_Drawonable* draw = Get_a_drawonable (self);
  draw->Get_String_Extents (font, string, strlen (string), text_width,
			    ascent, descent, a, b);
  text_height = ascent + descent;

  // center the text
  switch (align) {
  case Am_LEFT_ALIGN_val:
    text_left = real_left + offset_left;
    break;
  case Am_RIGHT_ALIGN_val:
    text_left = real_left + (real_width - text_width) - 2;
    break;
  case Am_CENTER_ALIGN_val:
  default:
    text_left = real_left + (real_width - text_width) / 2;
    break;
  }
  text_top = real_top + (real_height - text_height) / 2;
}

static bool inside_button_text (const Am_Object& self,
				const Am_Object& ref_object, int ref_x,
				int ref_y)
{
  // First, find out where the string lives inside the button.
  int width = self.Get (Am_WIDTH);
  int height = self.Get (Am_HEIGHT);

  Am_Font font (self.Get (Am_FONT));
  int align = self.Get (Am_H_ALIGN);

  Am_Widget_Look look = (Am_Widget_Look)(int)self.Get (Am_WIDGET_LOOK);

  // now find the contents to draw in the button
  Am_String string;
  Am_Value value;
  // string slot contains a formula which gets the real object based on the
  // value of the COMMAND slot
  value=self.Peek(Am_REAL_STRING_OR_OBJ);
  if (value.type == Am_STRING)
    string = value;
  else if (value.type == Am_OBJECT)
    return (0L);
  else
    Am_Error ("String slot of widget should have string or object value");

  int text_left, text_top, text_width, text_height;

  if (look == Am_MOTIF_LOOK)
    Am_Motif_Button_Text_Info (self, width, height, string, font,
			       Am_PUSH_BUTTON, 0, 0, false, align, 0,
			       text_left, text_top, text_width, text_height);
  // Second, test the mouse position to see if it's inside text box.
  int x, y;

  Am_Translate_Coordinates (ref_object, ref_x, ref_y, self, x, y);

  return (x >= text_left) && (x <= text_left + text_width) &&
    (y >= text_top) && (y <= text_top + text_height);
}

////////////// Button Widget ///////////////

class Am_Button_Text_Info : public Am_Text_Info {
 public:
  void Release ()
  {
    delete this;
  }
  void Get_Info (int& offset_x, int& offset_y, Am_String& string,
		 Am_Style& style, Am_Font& font, Am_Value_List& changing_slots)
  {
    // now find the contents to draw in the button
    Am_Value value;
    // string slot contains a formula which gets the real object based on the
    // value of the COMMAND slot
    value=self.Peek(Am_REAL_STRING_OR_OBJ);
    if (value.type == Am_STRING)
      string = value;
    else if (value.type == Am_OBJECT)
      return;
    else
      Am_Error ("String slot of widget should have string or object value");

    bool active = self.Get (Am_ACTIVE);

    font = self.Get (Am_FONT);

    // find out where the string lives inside the button.
    int width = self.Get (Am_WIDTH);
    int height = self.Get (Am_HEIGHT);
    int align = self.Get (Am_H_ALIGN);
    Am_Widget_Look look = (Am_Widget_Look)(int)self.Get (Am_WIDGET_LOOK);

    int text_width, text_height;

    if (look == Am_MOTIF_LOOK) {
      Am_Motif_Button_Text_Info (self, width, height, string, font,
				 Am_PUSH_BUTTON, 0, 0, false, align, 0,
				 offset_x, offset_y, text_width, text_height);
      style = (active ? Am_Black : Am_Motif_Inactive_Stipple);
    }

    changing_slots = slot_list;
  }
  void Set_Text (const Am_String& string)
  {
    Am_Object command = self.Get (Am_COMMAND);
    command.Set (Am_LABEL, string);
  }
  Am_Text_Navigate* Get_Navigator ()
  {
    return (0L);
  }

  Am_Button_Text_Info (const Am_Object& in_button)
  {
    self = in_button;
    slot_list = Am_Value_List ()
      .Add (self).Add (Am_FONT)
      .Add (self).Add (Am_ACTIVE)
      .Add (self).Add (Am_WIDTH)
      .Add (self).Add (Am_HEIGHT)
      .Add (self).Add (Am_H_ALIGN)
    ;
  }

  Am_Object self;
  Am_Value_List slot_list;
};

Am_Define_Method (Am_Inplace_Info_Method, Am_Text_Info*, button_inplace_text,
		  (const Am_Object& self, const Am_Object& ref_object,
		   int ref_x, int ref_y))
{
  if (inside_button_text (self, ref_object, ref_x, ref_y))
    return new Am_Button_Text_Info (self);
  else
    return (0L);
}

///////// Button Panel Widget ///////////////

class Am_Button_Panel_Text_Info : public Am_Text_Info {
 public:
  class Am_Button_Panel_Text_Navigate : public Am_Text_Navigate {
   public:
    bool Move (Am_Inplace_Text_Direction direction)
    {
      Am_Value_List parts = info->self.Get (Am_GRAPHICAL_PARTS);
      parts.Start ();
      if (!parts.Member (info->current_button))
        return false;
      switch (direction) {
      case Am_INPLACE_PREV:
        parts.Prev ();
        if (parts.First ())
          parts.End ();
        break;
      case Am_INPLACE_LEFT:
      case Am_INPLACE_UP:
        parts.Prev ();
        if (parts.First ())
          return false;
        break;
      case Am_INPLACE_NEXT:
        parts.Next ();
        if (parts.Last ())
          parts.Start ();
        break;
      case Am_INPLACE_RIGHT:
      case Am_INPLACE_DOWN:
        parts.Next ();
        if (parts.Last ())
          return false;
        break;
      }
      info->current_button = parts.Get ();
      return true;
    }
#ifdef AMULET2_INSTRUMENT
#undef Add
#endif
    void Add (const Am_String& text, Am_Insert_Position position)
#ifdef AMULET2_INSTRUMENT
#define Add Am_Instrumented(Add)
#endif
    {
      Am_Object new_item = Am_Command.Create ("button panel item")
        .Set (Am_LABEL, text)
      ;
      Am_Value_List items = info->self.Get (Am_ITEMS);
      Am_Object current_item = info->current_button.Get (Am_COMMAND);
      items.Start ();
      if (!items.Member (current_item))
        return;
      items.Insert (new_item, position);
      info->self.Set (Am_ITEMS, items);
    }
    bool Delete (Am_Inplace_Text_Direction direction)
    {
      Am_Object next_button;
      Am_Value_List parts = info->self.Get (Am_GRAPHICAL_PARTS);
      parts.Start ();
      if (parts.Length () <= 1)
        return false;
      if (!parts.Member (info->current_button))
        return false;
      switch (direction) {
      case Am_INPLACE_PREV:
        parts.Prev ();
        if (parts.First ())
          parts.End ();
        break;
      case Am_INPLACE_LEFT:
      case Am_INPLACE_UP:
        parts.Prev ();
        if (parts.First ())
          return false;
        break;
      case Am_INPLACE_NEXT:
        parts.Next ();
        if (parts.Last ())
          parts.Start ();
        break;
      case Am_INPLACE_RIGHT:
      case Am_INPLACE_DOWN:
        parts.Next ();
        if (parts.Last ())
          return false;
        break;
      default:
        break;
      }
      next_button = parts.Get ();
      Am_Value_List items = info->self.Get (Am_ITEMS);
      Am_Object current_item = info->current_button.Get (Am_COMMAND);
      items.Start ();
      if (!items.Member (current_item))
        return false;
      items.Delete ();
      info->self.Set (Am_ITEMS, items);
      return true;
    }

    Am_Button_Panel_Text_Navigate (Am_Button_Panel_Text_Info* in_info)
    {
      info = in_info;
    }

    Am_Button_Panel_Text_Info* info;
  } navigator;

  void Release ()
  {
    delete this;
  }
  void Get_Info (int& offset_x, int& offset_y, Am_String& string,
		 Am_Style& style, Am_Font& font, Am_Value_List& changing_slots)
  {
    // now find the contents to draw in the button
    Am_Value value;
    // string slot contains a formula which gets the real object based on the
    // value of the COMMAND slot
    value=current_button.Peek(Am_REAL_STRING_OR_OBJ);
    if (value.type == Am_STRING)
      string = value;
    else if (value.type == Am_OBJECT)
      return;
    else
      Am_Error ("String slot of widget should have string or object value");

    bool active = current_button.Get (Am_ACTIVE);

    font = current_button.Get (Am_FONT);

    // find out where the string lives inside the button.
    int width = current_button.Get (Am_WIDTH);
    int height = current_button.Get (Am_HEIGHT);
    int align = current_button.Get (Am_H_ALIGN);
    Am_Widget_Look look =
          (Am_Widget_Look)(int)current_button.Get (Am_WIDGET_LOOK);

    int text_width, text_height;

    if (look == Am_MOTIF_LOOK) {
      Am_Motif_Button_Text_Info (current_button, width, height, string, font,
				 Am_PUSH_BUTTON, 0, 0, false, align, 0,
				 offset_x, offset_y, text_width, text_height);
      style = (active ? Am_Black : Am_Motif_Inactive_Stipple);
    }
    offset_x += (int)current_button.Get (Am_LEFT);
    offset_y += (int)current_button.Get (Am_TOP);

    changing_slots = slot_list;
  }
  void Set_Text (const Am_String& string)
  {
    Am_Object command = current_button.Get (Am_COMMAND);
    command.Set (Am_LABEL, string);
  }
  Am_Text_Navigate* Get_Navigator ()
  {
    return &navigator;
  }

  Am_Button_Panel_Text_Info (const Am_Object& panel,
			     const Am_Object& button)
   : navigator (this)
  {
    self = panel;
    current_button = button;
    slot_list = Am_Value_List ()
      .Add (self).Add (Am_FONT)
      .Add (self).Add (Am_ACTIVE)
      .Add (self).Add (Am_WIDTH)
      .Add (self).Add (Am_HEIGHT)
      .Add (self).Add (Am_LAYOUT)
      //.Add (self).Add (Am_H_ALIGN)
      //.Add (self).Add (Am_V_ALIGN)
      //.Add (self).Add (Am_H_SPACING)
      //.Add (self).Add (Am_V_SPACING)
    ;
  }

  Am_Object self;
  Am_Object current_button;
  Am_Value_List slot_list;
};

Am_Define_Method (Am_Inplace_Info_Method, Am_Text_Info*,
		  button_panel_inplace_text,
		  (const Am_Object& self, const Am_Object& ref_object,
		   int ref_x, int ref_y))
{
  Am_Value_List button_list = self.Get (Am_GRAPHICAL_PARTS);
  Am_Object button;
  for (button_list.Start (); !button_list.Last (); button_list.Next ()) {
    button = button_list.Get ();
    if (inside_button_text (button, ref_object, ref_x, ref_y))
      return new Am_Button_Panel_Text_Info (self, button);
  }
  return (0L);
}

///////// Menu Bar Widget ///////////////

Am_Define_Method (Am_Inplace_Info_Method, Am_Text_Info*,
		  menu_bar_inplace_text,
		  (const Am_Object& /*self*/, const Am_Object& /*ref_object*/,
		   int /*ref_x*/, int /*ref_y*/))
{
  return (0L);
}

static void mod_widgets ()
{
  Am_Button
    .Add (Am_INPLACE_TEXT_INFO, button_inplace_text)
  ;
  Am_Button_Panel
    .Add (Am_INPLACE_TEXT_INFO, button_panel_inplace_text)
  ;
  Am_Menu_Bar
    .Set (Am_INPLACE_TEXT_INFO, menu_bar_inplace_text)
  ;
}

///////// Test Program Stuff /////////

int main ()
{
  Am_Initialize ();

  mod_widgets ();
  init_proto ();

  Am_Object feedback_text = Am_Inplace_Text_Feedback.Create ();
  Am_Object undo_handler = Am_Multiple_Undo_Object.Create();

  Am_Screen
    .Add_Part (Am_Window.Create ()
      .Set (Am_TOP, 50)
      .Set (Am_WIDTH, 400)
      .Set (Am_HEIGHT, 300)
      .Set (Am_UNDO_HANDLER, undo_handler)
      .Add_Part (Am_Group.Create ()
        .Set (Am_WIDTH, Am_From_Owner (Am_WIDTH))
        .Set (Am_HEIGHT, Am_From_Owner (Am_HEIGHT))
        .Add_Part (Am_Menu_Bar.Create ()
          .Set (Am_ACTIVE_2, false)
          .Set (Am_ITEMS, Am_Value_List ()
            .Add (Am_Command.Create ()
              .Set (Am_LABEL, "Menu")
              .Set (Am_ITEMS, Am_Value_List ()
                .Add (Am_Command.Create ()
                  .Set (Am_LABEL, "Item 1")
                )
                .Add (Am_Command.Create ()
                  .Set (Am_LABEL, "Item 2")
                )
                .Add (Am_Command.Create ()
                  .Set (Am_LABEL, "Item 3")
                )
              )
            )
            .Add (Am_Command.Create ()
              .Set (Am_LABEL, "Other")
              .Set (Am_ITEMS, Am_Value_List ()
                .Add (Am_Command.Create ()
                  .Set (Am_LABEL, "Item 1")
                )
                .Add (Am_Command.Create ()
                  .Set (Am_LABEL, "Item 2")
                )
              )
            )
          )
        )
        .Add_Part (Am_Button.Create ()
          .Set (Am_LEFT, 50)
          .Set (Am_TOP, 40)
          .Set (Am_ACTIVE_2, false)
        )
        .Add_Part (Am_Button.Create ()
          .Set (Am_LEFT, 150)
          .Set (Am_TOP, 40)
          .Set (Am_ACTIVE_2, false)
        )
        .Add_Part (Am_Button_Panel.Create ()
          .Set (Am_LEFT, 50)
          .Set (Am_TOP, 150)
          .Set (Am_ACTIVE_2, false)
          .Set (Am_ITEMS, Am_Value_List ()
            .Add (Am_Command.Create ()
              .Set (Am_LABEL, "Item 1")
            )
            .Add (Am_Command.Create ()
              .Set (Am_LABEL, "Item 2")
            )
            .Add (Am_Command.Create ()
              .Set (Am_LABEL, "Item 3")
            )
          )
        )
        .Add_Part (Am_Inplace_Text_Edit_Interactor.Create ()
	  .Set (Am_FEEDBACK_OBJECT, feedback_text)
        )
      )
      .Add_Part (feedback_text)
      .Set (Am_RIGHT_OFFSET, 10)
      .Set (Am_BOTTOM_OFFSET, 10)
      .Add_Part (Am_Button_Panel.Create()
        .Set (Am_LEFT, Am_Right_Is_Right_Of_Owner)
        .Set (Am_TOP, Am_Bottom_Is_Bottom_Of_Owner)
        .Set (Am_ITEMS, Am_Value_List ()
	  .Add (Am_Undo_Command.Create())
          .Add (Am_Redo_Command.Create())
        )
      )
    )
  ;

  Am_Main_Event_Loop ();
  Am_Cleanup ();
}
