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
#include <amulet/impl/opal_constraints.h>

Am_Object Am_Radio_Button_Panel;
Am_Object Am_Radio_Button_In_Panel = nullptr;

static void
init()
{
  Am_Object inter;            // interactor in the widget
  Am_Object_Advanced obj_adv; // to get at advanced features like
                              // local-only and demons.

  ///////////////////////////////////////////////////////////////////////////
  // Radio button panel
  ///////////////////////////////////////////////////////////////////////////

  Am_Radio_Button_Panel = Am_Button_Panel.Create(DSTR("Radio button Panel"));
  Am_Radio_Button_Panel.Add(Am_BOX_ON_LEFT, true)
      .Add(Am_BOX_HEIGHT, radio_button_diameter)
      .Add(Am_BOX_WIDTH, radio_button_diameter)
      .Set(Am_ITEM_OFFSET, 3)
      .Set(Am_H_ALIGN, Am_Align_From_Box_On_Left)
      .Set(Am_V_SPACING, 0)
      .Set(Am_FIXED_WIDTH, false) // fixed width makes the labels centered
      .Set(Am_FINAL_FEEDBACK_WANTED, true)
      .Set_Part(
          Am_ITEM_PROTOTYPE,
          Am_Radio_Button_In_Panel =
              Am_Radio_Button.Create(DSTR("Radio_Button_In_Panel_Proto"))
                  .Add(Am_REAL_WIDTH, checkbox_width)
                  .Add(Am_REAL_HEIGHT, checkbox_height)
                  .Set(Am_WIDTH, panel_item_width)
                  .Set(Am_HEIGHT, panel_item_height)
                  .Set(Am_BOX_ON_LEFT, box_on_left_from_owner)
                  .Set(Am_BOX_WIDTH, box_width_from_owner)
                  .Set(Am_BOX_HEIGHT, box_height_from_owner)
                  .Set(Am_ACTIVE, active_from_command_panel)
                  .Set(Am_SELECTED, false)
                  .Set(Am_WIDGET_LOOK, look_from_owner)
                  .Set(Am_FONT, Am_Font_From_Owner)
                  .Set(Am_FILL_STYLE, Am_From_Owner(Am_FILL_STYLE))
                  .Set(Am_ITEM_OFFSET, Am_From_Owner(Am_ITEM_OFFSET))
                  .Set(Am_SET_COMMAND_OLD_OWNER, (0L))
                  .Set(Am_FINAL_FEEDBACK_WANTED, final_feedback_from_owner));

  inter = Am_Radio_Button_Panel.Get(Am_INTERACTOR);
  inter.Set(Am_FIRST_ONE_ONLY, true);

  // don't want the individual interactor from the button
  Am_Radio_Button_In_Panel.Remove_Part(Am_INTERACTOR);

  // when in a panel, the box's command object is gotten from the item slot
  // which is set automatically by the Map
  Am_Object command_obj = Am_Radio_Button_In_Panel.Get_Object(Am_COMMAND);
  Am_Radio_Button_In_Panel.Add(Am_ITEM, command_obj); // default value
  Am_Radio_Button_In_Panel.Add(Am_ITEM_TO_COMMAND, Am_Copy_Item_To_Command);
}

static void
cleanup()
{
  Am_Radio_Button_Panel.Destroy();
}

static Am_Initializer *initializer = new Am_Initializer(
    DSTR("Am_Radio_Button_Panel"), init, 5.205f, 119, cleanup);
