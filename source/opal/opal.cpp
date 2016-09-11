/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <am_inc.h>

#ifdef DEBUG
	#include DEBUGGER__H
#endif

// only include the initializer call to inspector if we use a debug-version of OA
// otherwise the code isn't linked in, resulting in a smaller executable
// The inspector code itself is	contained in file 'inspector.cpp'. Don't
// move this code to an other place!! If so the inspector code won't be
// linked in
#ifdef INCLUDE_INSPECTOR
	#include INITIALIZER__H
	#ifdef _MSC_VER
		#pragma message("Adding Inspector Code")
	#endif
	static Am_Initializer* inspector_init = new Am_Initializer("Inspector", Am_Initialize_Inspector, 8.0f, 116, Am_Cleanup_Inspector);
#endif

#include AM_IO__H
#include MISC__H // Am_Get_Amulet_Pathname

#include <amulet/impl/types_logging.h>
#include <amulet/impl/types_method.h>
#include <amulet/impl/slots.h>
#include <amulet/impl/am_object.h>
#include <amulet/impl/am_part_iterator.h>
#include <amulet/impl/opal_default_demon.h>
#include <amulet/impl/opal_load_save.h>
#include <amulet/impl/am_state_store.h>
#include <amulet/impl/am_alignment.h>
#include <amulet/impl/opal_objects.h>
#include <amulet/impl/opal_op.h>
#include <amulet/impl/opal_misc.h>
#include <amulet/impl/opal_methods.h>
#include <amulet/impl/method_item.h>
#include VALUE_LIST__H

#include FORMULA__H
//#include OPAL__H
#include REGISTRY__H
#include INITIALIZER__H
#include "amulet/opal_inlines.h"

//
// The Am_Is_Group_Or_Map() function needs to check whether an object is an
// instance of Am_Map. If it references Am_Map directly then that will pull
// in all of the Am_Map code, even if the application isn't using any
// Am_Maps. So instead we use the pointer below as indirection: if it is
// null then the application is not using any Am_Maps so
// Am_Is_Group_Or_Map() doesn't need to do the check.
//
// This pointer is initialized by Am_Map's Initializer in map.cpp
//
Am_Object *Am_Map_ptr = 0;

// exported types
Am_Define_Method_Type_Impl (Am_Draw_Method);
Am_Define_Method_Type_Impl (Am_Invalid_Method);
Am_Define_Method_Type_Impl (Am_Point_In_Method);
Am_Define_Method_Type_Impl (Am_Point_In_Or_Self_Method);
Am_Define_Method_Type_Impl (Am_Translate_Coordinates_Method);
Am_Define_Method_Type_Impl (Am_Item_Method);
Am_Define_Method_Type_Impl (Am_Save_Object_Method);

Am_Define_Enum_Support (Am_Alignment, "Am_CENTER_ALIGN Am_TOP_ALIGN "
			"Am_BOTTOM_ALIGN Am_LEFT_ALIGN Am_RIGHT_ALIGN");

// main global demon queue

Am_Demon_Queue Main_Demon_Queue;

// These are utility functions that return either the drawonable of the
// window that the object is displayed in, or the drawonable of the
// root window (e.g., if the object is not yet in a window, or the window
// has not been mapped yet).
Am_Drawonable* Get_a_drawonable (Am_Object obj) {
  Am_Drawonable *drawonable;
  Am_Object window, owner;

  // Please don't try to clean this up (i.e. by turning it into a single
  // complicated if statement).  Solaris gcc 2.7.0 produces buggy code
  // if you do. -- rcm
  if (obj.Valid()) {
    window = obj.Get (Am_WINDOW, Am_NO_DEPENDENCY);
    if (window.Valid()) {
      owner = obj.Get_Owner (Am_NO_DEPENDENCY);
      if (owner.Valid()) {
	drawonable = Am_Drawonable::Narrow (window.Get (Am_DRAWONABLE, Am_NO_DEPENDENCY));
	if (drawonable)
	  return drawonable;
      }
    }
  }
  // ELSE...
    // If the object is not valid, or
    // if the object is not in a window yet,
    // or the window's drawonable has not yet been created,
    // then use the drawonable for the root screen.
  drawonable = Am_Drawonable::Narrow (Am_Screen.Get (Am_DRAWONABLE, Am_NO_DEPENDENCY));
  return drawonable;
}

Am_Drawonable* GV_a_drawonable (Am_Object obj) {
  Am_Drawonable *drawonable;
  Am_Object window, owner;

  // Please don't try to clean this up (i.e. by turning it into a single
  // complicated if statement).  Solaris gcc 2.7.0 produces buggy code
  // if you do. -- rcm
  window = obj.Get (Am_WINDOW, Am_OK_IF_NOT_THERE);
  if (window.Valid()) {
    owner = obj.Get_Owner ();
    if (owner.Valid()) {
      drawonable = Am_Drawonable::Narrow (window.Get (Am_DRAWONABLE));
      if (drawonable)
	return drawonable;
    }
  }

  // ELSE...
    // If the object is not valid, or
    // if the object is not in a window yet,
    // or the window's drawonable has not yet been created,
    // then use the drawonable for the root screen.
  drawonable = Am_Drawonable::Narrow (Am_Screen.Get (Am_DRAWONABLE));
  return drawonable;
}

// Beeps at window's drawonable.  If window is not valid, beeps at default
// screen.
// Defaults: window = Am_No_Object

bool Am_Beep_Happened = false;

void Am_Beep (Am_Object window)
{
  Am_Drawonable *d = Get_a_drawonable (window);
  d->Beep();
  d->Flush_Output();
  Am_Beep_Happened = true; //used in scripts
}

//
// Check whether an object is a group or map.
//
bool
Am_Is_Group_Or_Map(Am_Object &obj)
{
	if (obj.Is_Instance_Of(Am_Group)) return true;

	//
	// If Am_Map_ptr is not set, then there are no Am_Maps in this
	// application, so we don't have to check to see if `obj' is an Am_Map.
	//
	if (Am_Map_ptr && obj.Is_Instance_Of(*Am_Map_ptr)) return true;

	return false;
}


static int Am_Initialized_Flag=0;

void Am_Initialize ()
{
        if (Am_Initialized_Flag) return;

	Am_Initializer::Do_Initialize();
	Main_Demon_Queue.Invoke();

        Am_Initialized_Flag=1;
}

void init_styles();
void init_aggregates();
void set_slot_names();

static void init() {

  init_styles();

  set_slot_names ();

  Am_Initialize_Aux ();  // Initialize Am_Window and Am_Graphical_Object

  init_aggregates(); // Initialize Am_Aggregate and Am_Group
}

static Am_Initializer* initializer =
	new Am_Initializer(DSTR("Opal"), init, 2.0f);

// used by Am_Cleanup() function
class  Am_Symbol_Table;
extern Am_Symbol_Table 		*Am_Slot_Name_Key_Table;
extern Am_Symbol_Table 		*char_map;
extern Am_Symbol_Table 		*click_count_map;
extern Am_Object 			Am_Menu_Bar_Sub_Window_Proto;

void Am_Cleanup ()
{
	if (!Am_Initialized_Flag)
    	return;

	Main_Demon_Queue.Shutdown();
	Am_State_Store::Shutdown();

	#ifdef DEBUG
		Am_Global_Slot_Trace_Proc = (0L);
	#endif

	Am_Group.Destroy ();
	Am_Aggregate.Destroy ();
	Am_Window.Destroy ();
	Am_Screen.Destroy ();

	// Destroy prototypes
	//	item_prototype.Destroy();
	Am_Menu_Bar_Sub_Window_Proto.Destroy(); // see button_widgets.cpp ~line 3712

	// NDY: Make sure these are released -- in order -- once defined.
	// 	Am_Polygon.Destroy ();
	Am_Graphical_Object.Destroy ();

	// ---------------------------------------------------------------------------
	// we cleanup stuff allocated once in the complete OpenAmulet environment, too (still buggy!)
	// ---------------------------------------------------------------------------

	// Am_Slot_Key_Table is allocated in file standard_slot.cpp function verify_slot_name_key_table()
	//	delete(Am_Slot_Name_Key_Table);

	// these both maps are defined in gem_keytrans.cpp
	//	delete(char_map);
	//	delete(click_count_map);

	// call any registered cleanup procedure
	Am_Initializer::Do_Cleanup();

	// finally destroy our root
	Am_Root_Object.Destroy();

    Am_Initialized_Flag=0;
}

bool Am_Point_In_All_Owners(const Am_Object& in_obj, int x, int y,
			    const Am_Object& ref_obj)
{
  Am_Object owner, result;
  if (in_obj.Is_Instance_Of(Am_Screen)) return true;
  if (!in_obj.Is_Part_Of (ref_obj)) return false;
     // in_obj not actually part of system
  if (in_obj == ref_obj) return true;
  owner = in_obj;
  //check all owners all the way up to window
  do {
    owner = owner.Get_Owner();
    //if (!(bool)owner.Get(Am_VISIBLE)) return false;
    // checking visible should be handled in Am_Point_In_Obj
    result = Am_Point_In_Obj(owner, x, y, ref_obj);
    if (!result.Valid()) return false;
  }
  while (owner != ref_obj);
  return true;  // owner reached top of hierarchy (ref_obj)
}

Am_Object Am_Point_In_Obj (const Am_Object& in_obj, int x, int y,
			   const Am_Object& ref_obj)
{
  Am_Point_In_Method method;
  method = in_obj.Get(Am_POINT_IN_OBJ_METHOD);
  return method.Call (in_obj, x, y, ref_obj);
}

Am_Object Am_Point_In_Part (const Am_Object& in_obj, int x, int y,
			    const Am_Object& ref_obj, bool want_self,
			    bool want_groups)
{
  Am_Point_In_Or_Self_Method method;
  method = in_obj.Get(Am_POINT_IN_PART_METHOD);
  return method.Call (in_obj, x, y, ref_obj, want_self, want_groups);
}

Am_Object Am_Point_In_Leaf (const Am_Object& in_obj, int x, int y,
			    const Am_Object& ref_obj,
			    bool want_self, bool want_groups)
{
  Am_Point_In_Or_Self_Method method;
  method = in_obj.Get(Am_POINT_IN_LEAF_METHOD);
  return method.Call (in_obj, x, y, ref_obj, want_self, want_groups);
}

static Am_Drawonable* get_safe_drawonable(const Am_Object& screen) {
  Am_Object used_screen;
  if (screen.Valid ()) used_screen = screen;
  else used_screen = Am_Screen;

  Am_Drawonable* drawonable =
        Am_Drawonable::Narrow (used_screen.Get (Am_DRAWONABLE, Am_NO_DEPENDENCY));
  if (!drawonable)
    drawonable = Am_Drawonable::Narrow (Am_Screen.Get (Am_DRAWONABLE, Am_NO_DEPENDENCY));
  return drawonable;
}

void Am_Get_String_Extents (const Am_Object& screen, const Am_Font& font,
			    const char* string, int length,
			    int& width, int& height)
{
  Am_Drawonable* drawonable = get_safe_drawonable(screen);
  int ascent, descent, a, b;
  drawonable->Get_String_Extents (font, string,
				  (length == -1) ? strlen (string) : length,
				  width, ascent, descent, a, b);
  height = ascent + descent;
}

int Am_Get_String_Width (const Am_Object& screen, const Am_Font& font,
			 const char* string, int length)
{
  Am_Drawonable* drawonable = get_safe_drawonable(screen);
  return drawonable->Get_String_Width (font, string,
				  (length == -1) ? strlen (string) : length);
}

void Am_Get_Font_Properties (const Am_Object& screen, const Am_Font& font,
			     int& max_char_width, int& min_char_width,
			     int& max_char_ascent, int& max_char_descent)
{
  Am_Drawonable* drawonable = get_safe_drawonable(screen);
  if (drawonable)
    drawonable->Get_Font_Properties (font, max_char_width, min_char_width,
				     max_char_ascent, max_char_descent);
}

bool Am_Test_Image_File (const Am_Object& screen, const Am_Image_Array& image)
{
  if (!screen.Valid ())
    return false;
  Am_Drawonable* drawonable =
        Am_Drawonable::Narrow (screen.Get (Am_DRAWONABLE));
  return drawonable->Test_Image (image);
}

Am_Object Am_GV_Screen (const Am_Object& self)
{
  Am_Value v;
  Am_Object screen;
  v = self.Peek(Am_WINDOW);
  if (v.Valid()) {
    screen = v;
    for ( ;
	 screen.Valid () && !screen.Is_Instance_Of (Am_Screen);
	 screen = screen.Get_Owner()) ;
  }
  if (!screen.Valid()) return Am_Screen;
  else return screen;
}

void Am_Draw (Am_Object object, Am_Drawonable* drawonable,
          int x_offset, int y_offset)
{
  Am_Draw_Method draw = object.Get (Am_DRAW_METHOD);
  draw.Call (object, drawonable, x_offset, y_offset);
}

// Called when an object is copied or created.
// components is the list of inherited graphical parts,
// parts = local parts (all parts the object has)
// We need to rename the parts because they're now instances.
// Also sets rank of parts.

void am_generic_renew_components (Am_Object object)
{
  Am_Value_List components;
  components = object.Get (Am_GRAPHICAL_PARTS);
  Am_Value_List new_components;
  Am_Part_Iterator parts = object;
  int parts_length = parts.Length ();
  // if the object has no inherited graphical parts (all the parts are
  // non-graphical) or no parts, then the new object will still have no
  // graphical parts, so exit quickly.
  if (components.Empty () || (parts_length == 0)) {
    object.Set(Am_GRAPHICAL_PARTS, Am_Value_List());
    return;
  }
  Am_Object* part_map = new Am_Object [parts_length*2];
  Am_Object part;
  int i;
  for (i = 0, parts.Start(); !parts.Last (); ++i, parts.Next ()) {
    part = parts.Get ();
    part_map[i+parts_length] = part;
    part_map[i] = part.Get_Prototype ();
  }
  Am_Object current_component;
  int current_rank = 0;
  for (components.Start(); !components.Last (); components.Next ()) {
    current_component = components.Get ();
    if (current_component.Get_Key () == Am_NO_INHERIT)
      continue;
    for (i = 0; i < parts_length; ++i)
      if (current_component == part_map[i]) {
        part_map[i+parts_length].Set (Am_RANK, current_rank);
        new_components.Add (part_map[i+parts_length]);
        ++current_rank;
        break;
      }
  }
  delete [] part_map;
  object.Set (Am_GRAPHICAL_PARTS, new_components);
}

void am_generic_renew_copied_comp (Am_Object object)
{
  Am_Value_List components;
  components = object.Get (Am_GRAPHICAL_PARTS);
  Am_Value_List new_components;
  Am_Part_Iterator parts = object;
  int parts_length = parts.Length ();
  if (components.Empty () || (parts_length == 0)) {
    object.Set(Am_GRAPHICAL_PARTS, Am_Value_List());
    return;
  }
  Am_Object* part_map = new Am_Object [parts_length*2];
  Am_Object part;
  int i;
  for (i = 0, parts.Start(); !parts.Last (); ++i, parts.Next ()) {
    part = parts.Get ();
    part_map[i+parts_length] = part;
    part_map[i] = part.Get (Am_SOURCE_OF_COPY);
  }
  Am_Object current_component;
  int current_rank = 0;
  for (components.Start(); !components.Last (); components.Next ()) {
    current_component = components.Get ();
    if (current_component.Get_Key () == Am_NO_INHERIT)
      continue;
    for (i = 0; i < parts_length; ++i)
      if (current_component == part_map[i]) {
        part_map[i+parts_length].Set (Am_RANK, current_rank);
        new_components.Add (part_map[i+parts_length]);
        ++current_rank;
        break;
      }
  }
  delete [] part_map;
  object.Set (Am_GRAPHICAL_PARTS, new_components);
}

