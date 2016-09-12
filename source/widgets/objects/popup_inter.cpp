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
#include <am_inc.h>

#include <amulet/impl/opal_obj2.h>

Am_Object Am_Pop_Up_Menu_Interactor;

Am_Object Am_Pop_Up_Menu_Sub_Window_Proto; // defined below

////////////////////////////////////////////////////////////////////////
// PopUp Menus
////////////////////////////////////////////////////////////////////////

int
create_popup_sub_menu_proc(Am_Object &self)
{
  Am_Value v;
  v = self.Peek(Am_SUB_MENU, Am_NO_DEPENDENCY);
  if (v.Valid())
    Am_Error("create_popup_sub_menu called but already has a menu");
  Am_Object new_window =
      Am_Pop_Up_Menu_Sub_Window_Proto.Create().Set(Am_FOR_ITEM, self);
  Am_Screen.Add_Part(new_window);
  self.Set(Am_SUB_MENU, new_window, Am_OK_IF_NOT_THERE);
  return -1;
}

Am_Formula create_popup_sub_menu(create_popup_sub_menu_proc,
                                 DSTR("create_popup_sub_menu"));

Am_Define_Method(Am_Mouse_Event_Method, void, popup_inter_start_do,
                 (Am_Object inter, int x, int y, Am_Object ref_obj,
                  Am_Input_Char /* ic */))
{
  Am_Object sub_menu_window = inter.Get(Am_SUB_MENU);
  if (sub_menu_window.Valid()) {
    Am_Translate_Coordinates(ref_obj, x, y, Am_Screen, x, y);
    sub_menu_window.Set(Am_LEFT, x);
    sub_menu_window.Set(Am_TOP, y);
    Am_Object main_win = inter.Get(Am_WINDOW);
    set_popup_win_visible(sub_menu_window, true, main_win);
    Am_Object sub_inter =
        sub_menu_window.Get_Object(Am_SUB_MENU).Get_Object(Am_INTERACTOR);
    Am_Start_Interactor(sub_inter);
  }
  Am_Abort_Interactor(inter); //so the do method isn't called on start
}

/******************************************************************************
 *   A custom destroy demon for option button to destroy the menu popup window.
 */

void
destroy_popup_interactor(Am_Object object)
{
  Am_Object sub_menu = object.Get(Am_SUB_MENU);
  sub_menu.Destroy();

  //now run the regular button destroy demon, if any
  Am_Object_Demon *proto_demon = ((Am_Object_Advanced &)Am_One_Shot_Interactor)
                                     .Get_Demons()
                                     .Get_Object_Demon(Am_DESTROY_OBJ);
  if (proto_demon)
    proto_demon(object);
}

static void
init()
{
  Am_Object inter;            // interactor in the widget
  Am_Object_Advanced obj_adv; // to get at advanced features like
                              // local-only and demons.
  ///////////////////////////////////////////////////////////////////////////
  // Am_Pop_Up_Menu_Interactor: Pops up a menu
  ///////////////////////////////////////////////////////////////////////////

  Am_Pop_Up_Menu_Sub_Window_Proto =
      Am_Option_Button_Sub_Window_Proto.Create(DSTR("Pop_Up_Menu_Sub_Window"))
          .Set(Am_LEFT, 0) //set by interactor
          .Set(Am_TOP, 0);

  Am_Pop_Up_Menu_Interactor =
      Am_One_Shot_Interactor.Create(DSTR("Am_Pop_Up_Menu_Interactor"))
          .Add(Am_ITEMS, 0)
          .Add(Am_FOR_ITEM, create_popup_sub_menu)
          .Add(Am_SUB_MENU, 0)
          .Add(Am_FILL_STYLE, Am_Default_Color)
          .Add(Am_WIDGET_LOOK, Am_Default_Widget_Look)
          .Set_Inherit_Rule(Am_SUB_MENU, Am_LOCAL)
          .Set(Am_VALUE, option_button_value.Multi_Constraint())
          .Set(Am_DO_METHOD, popup_inter_start_do)
          .Add(Am_REAL_STRING_OR_OBJ, get_real_string_from_inter_val);

  obj_adv = (Am_Object_Advanced &)Am_Pop_Up_Menu_Interactor;
  Am_Demon_Set demons = obj_adv.Get_Demons().Copy();
  demons.Set_Object_Demon(Am_DESTROY_OBJ, destroy_popup_interactor);
  obj_adv.Set_Demons(demons);
}

static void
cleanup()
{
  Am_Pop_Up_Menu_Interactor.Destroy();
}

static Am_Initializer *initializer = new Am_Initializer(
    DSTR("Am_Pop_Up_Menu_Interactor"), init, 5.2091f, 117, cleanup);
