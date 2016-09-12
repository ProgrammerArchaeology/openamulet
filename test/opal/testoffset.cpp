#include <amulet.h>

Am_Slot_Key KEY0 = Am_Register_Slot_Name("KEY0");
Am_Slot_Key KEY1 = Am_Register_Slot_Name("KEY1");
Am_Slot_Key KEY2 = Am_Register_Slot_Name("KEY2");

int
main()
{
  Am_Initialize();

  Am_Object sample = Am_Rectangle.Create().Set(Am_LEFT, 150).Set(Am_TOP, 200);

  Am_Screen.Add_Part(
      Am_Window.Create()
          .Set(Am_WIDTH, 300)
          .Set(Am_HEIGHT, 400)
          .Add_Part(KEY0, Am_Rectangle.Create()
                              .Set(Am_FILL_STYLE, Am_Red)
                              .Set(Am_LEFT, 10)
                              .Set(Am_TOP, 10)
                              .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH, -20))
                              .Set(Am_HEIGHT, Am_From_Owner(Am_HEIGHT, -20)))
          .Add_Part(Am_Rectangle.Create()
                        .Set(Am_FILL_STYLE, Am_Blue)
                        .Set(Am_LEFT, 15)
                        .Set(Am_TOP, 15)
                        .Set(Am_WIDTH, Am_From_Sibling(KEY0, Am_WIDTH, 0, 0.5))
                        .Set(Am_HEIGHT, Am_Same_As(Am_WIDTH, -20)))
          .Add_Part(Am_Group.Create()
                        .Set(Am_TOP, Am_From_Owner(Am_HEIGHT, 0, 0.5))
                        .Set(Am_LEFT, 20)
                        .Set(Am_WIDTH, Am_From_Part(KEY0, Am_WIDTH, 20))
                        .Set(Am_HEIGHT, Am_From_Part(KEY0, Am_HEIGHT, 20))
                        .Add_Part(Am_Rectangle.Create()
                                      .Set(Am_FILL_STYLE, Am_Green)
                                      .Set(Am_WIDTH, 50)
                                      .Set(Am_HEIGHT, 100))
                        .Add_Part(KEY0, Am_Rectangle.Create()
                                            .Set(Am_FILL_STYLE, Am_Orange)
                                            .Set(Am_LEFT, 10)
                                            .Set(Am_TOP, 10)
                                            .Set(Am_WIDTH, 30)
                                            .Set(Am_HEIGHT, 80)))
          .Add_Part(Am_Rectangle.Create()
                        .Set(Am_FILL_STYLE, Am_Blue)
                        .Set(Am_LEFT, Am_From_Object(sample, Am_LEFT))
                        .Set(Am_TOP, Am_From_Object(sample, Am_TOP, 15))
                        .Set(Am_WIDTH, Am_From_Sibling(KEY0, Am_WIDTH, 0, 0.5))
                        .Set(Am_HEIGHT, Am_Same_As(Am_WIDTH, -20))));

  /*  
    extern Am_Formula Am_Same_As (Am_Slot_Key key);
    extern Am_Formula Am_From_Owner (Am_Slot_Key key);
    extern Am_Formula Am_From_Part (Am_Slot_Key part, Am_Slot_Key key);
    extern Am_Formula Am_From_Sibling (Am_Slot_Key sibling, Am_Slot_Key key);
    extern Am_Formula Am_From_Object (Am_Object object, Am_Slot_Key key);

    extern Am_Formula Am_Same_As (Am_Slot_Key key, int offset,
                                  float multiplier = 1.0);
    extern Am_Formula Am_From_Owner (Am_Slot_Key key, int offset,
                                     float multiplier = 1.0);
    extern Am_Formula Am_From_Part (Am_Slot_Key part, Am_Slot_Key key,
			            int offset, float multiplier = 1.0);
    extern Am_Formula Am_From_Sibling (Am_Slot_Key sibling, Am_Slot_Key key,
				       int offset, float multiplier = 1.0);
    extern Am_Formula Am_From_Object (Am_Object object, Am_Slot_Key key,
				      int offset, float multiplier = 1.0);
  */

  Am_Main_Event_Loop();

  Am_Cleanup();
}
