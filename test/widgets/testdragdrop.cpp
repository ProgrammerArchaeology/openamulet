/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <amulet.h>

#include <amulet/opal_advanced.h>

Am_Define_Method(Am_Where_Method, Am_Object, Rect_In_Part,
                 (Am_Object inter, Am_Object /* object */,
                  Am_Object event_window, int x, int y))
{
  Am_Value_List owners = inter.Get(Am_MULTI_OWNERS);
  for (owners.Start(); !owners.Last(); owners.Next()) {
    Am_Object owner = owners.Get();
    if (owner.Is_Part_Of(event_window)) {
      Am_Object obj = Am_Point_In_Part(owner, x, y, event_window);
      if (obj.Is_Instance_Of(Am_Rectangle))
        return obj;
    }
  }
  return Am_No_Object;
}

Am_Style selected_line(0.0, 1.0, 0.0, 5);

Am_Define_Style_Formula(select_target)
{
  if ((bool)self.Get_Object(Am_DROP_TARGET).Get(Am_INTERIM_SELECTED))
    return selected_line;
  else
    return Am_Black;
}

Am_Define_Method(Am_Drop_Target_Do_Method, void, drop_do,
                 (Am_Object & command, const Am_Value &value))
{
  Am_Object self = command.Get_Owner();
  command.Set(Am_OBJECT_MODIFIED, self);
  command.Set(Am_OLD_VALUE, self.Get(Am_FILL_STYLE));
  if (Am_Value_List::Test(value)) {
    Am_Value_List obj_list(value);
    if (obj_list.Length() == 1) {
      obj_list.Start();
      self.Set(Am_FILL_STYLE, Am_Object(obj_list.Get()).Get(Am_FILL_STYLE));
    } else
      self.Set(Am_FILL_STYLE, Am_Motif_Gray);
  } else
    self.Set(Am_FILL_STYLE, Am_Purple);
  command.Set(Am_VALUE, self.Get(Am_FILL_STYLE));
}

Am_Define_Method(Am_Object_Method, void, drop_undo, (Am_Object command))
{
  Am_Style old_fill = command.Get(Am_OLD_VALUE);
  Am_Style new_fill = command.Get(Am_VALUE);
  Am_Object self = command.Get(Am_OBJECT_MODIFIED);
  self.Set(Am_FILL_STYLE, old_fill);
  command.Set(Am_OLD_VALUE, new_fill);
  command.Set(Am_VALUE, old_fill);
}

Am_Define_Method(Am_Drop_Target_Interim_Do_Method, bool, no_reds,
                 (Am_Object & /* command_obj */, const Am_Value &value))
{
  Am_Value_List obj_list(value);
  if (obj_list.Length() == 1) {
    obj_list.Start();
    Am_Object obj = obj_list.Get();
    if (Am_Style(obj.Get(Am_FILL_STYLE)) == Am_Red)
      return false;
  }
  return true;
}

int
main(void)
{
  Am_Initialize();

  Am_Object group1 =
      Am_Group.Create()
          .Set(Am_WIDTH, 300)
          .Set(Am_HEIGHT, 400)
          .Add_Part(
              Am_Rectangle.Create()
                  .Set(Am_WIDTH, 50)
                  .Set(Am_HEIGHT, 50)
                  .Set(Am_FILL_STYLE, Am_Red)
                  .Set(Am_LINE_STYLE, select_target)
                  .Add_Part(Am_DROP_TARGET, Am_Drop_Target_Command.Create()))
          .Add_Part(Am_Rectangle.Create()
                        .Set(Am_LEFT, 100)
                        .Set(Am_WIDTH, 50)
                        .Set(Am_HEIGHT, 50)
                        .Set(Am_FILL_STYLE, Am_Blue))
          .Add_Part(Am_Line.Create()
                        .Set(Am_LEFT, 100)
                        .Set(Am_TOP, 100)
                        .Set(Am_WIDTH, 50)
                        .Set(Am_HEIGHT, 50)
                        .Add(Am_FILL_STYLE, Am_Blue));

  Am_Object select1 = Am_Selection_Widget.Create().Set(Am_OPERATES_ON, group1);

  Am_Object undo_handler = Am_Multiple_Undo_Object.Create();

  Am_Object window1 =
      Am_Window.Create()
          .Set(Am_WIDTH, Am_Width_Of_Parts)
          .Set(Am_HEIGHT, Am_Height_Of_Parts)
          .Set(Am_UNDO_HANDLER, undo_handler)
          .Set(Am_DESTROY_WINDOW_METHOD, Am_Window_Destroy_And_Exit_Method)
          .Add(Am_LAYOUT, Am_Vertical_Layout)
          .Add_Part(Am_Menu_Bar.Create().Set(
              Am_ITEMS,
              Am_Value_List()
                  .Add(Am_Command.Create("Menu")
                           .Set(Am_IMPLEMENTATION_PARENT, true) //not undoable
                           .Set(Am_LABEL, "Menu1")
                           .Set(Am_ITEMS,
                                Am_Value_List().Add("Item1").Add("Item2")))
                  .Add(Am_Command.Create("menu2")
                           .Set(Am_IMPLEMENTATION_PARENT, true) //not undoable
                           .Set(Am_LABEL, "Menu2")
                           .Set(Am_ITEMS,
                                Am_Value_List().Add("New").Add("Middle").Add(
                                    "Old")))))
          .Add_Part(Am_Group.Create()
                        .Set(Am_WIDTH, Am_Width_Of_Parts)
                        .Set(Am_HEIGHT, Am_Height_Of_Parts)
                        .Add_Part(group1)
                        .Add_Part(select1));

  Am_Object group2 = Am_Group.Create().Set(Am_WIDTH, 1000).Set(Am_HEIGHT, 1000);

  Am_Object select2 = Am_Selection_Widget.Create()
                          .Set(Am_OPERATES_ON, group2)
                          .Set(Am_DROP_TARGET_TEST, Am_In_Target);

  Am_Object scroll1 = Am_Scrolling_Group.Create()
                          .Set(Am_LEFT, 20)
                          .Set(Am_TOP, 20)
                          .Set(Am_WIDTH, Am_Fill_To_Right)
                          .Set(Am_HEIGHT, Am_Fill_To_Bottom)
                          .Set(Am_INNER_WIDTH, 1000)
                          .Set(Am_INNER_HEIGHT, 1000)
                          .Set(Am_FILL_STYLE, Am_Motif_Gray)
                          .Add_Part(group2)
                          .Add_Part(select2);

  Am_Object group3 =
      Am_Group.Create()
          .Set(Am_WIDTH, 1000)
          .Set(Am_HEIGHT, 1000)
          .Add_Part(Am_Group.Create()
                        .Set(Am_WIDTH, 200)
                        .Set(Am_HEIGHT, 300)
                        .Set(Am_RIGHT_OFFSET, 20)
                        .Set(Am_BOTTOM_OFFSET, 20)
                        .Add_Part(Am_Rectangle.Create()
                                      .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH))
                                      .Set(Am_HEIGHT, Am_From_Owner(Am_HEIGHT))
                                      .Set(Am_FILL_STYLE, Am_Motif_Gray))
                        .Add_Part(scroll1));

  Am_Object select3 = Am_Selection_Widget.Create().Set(Am_OPERATES_ON, group3);

  Am_Object scroll2 = Am_Scrolling_Group.Create()
                          .Set(Am_WIDTH, 300)
                          .Set(Am_HEIGHT, 400)
                          .Set(Am_INNER_WIDTH, 1000)
                          .Set(Am_INNER_HEIGHT, 1000)
                          .Set(Am_FILL_STYLE, Am_Motif_Gray)
                          .Set(Am_INNER_FILL_STYLE, Am_Motif_Light_Gray)
                          .Add_Part(group3)
                          .Add_Part(select3);

  Am_Object target =
      Am_Rectangle.Create()
          .Set(Am_LEFT, 400)
          .Set(Am_TOP, 130)
          .Set(Am_WIDTH, 100)
          .Set(Am_HEIGHT, 40)
          .Set(Am_FILL_STYLE, Am_White)
          .Set(Am_LINE_STYLE, select_target)
          .Add_Part(Am_DROP_TARGET, Am_Drop_Target_Command.Create()
                                        .Set(Am_DO_METHOD, drop_do)
                                        .Set(Am_UNDO_METHOD, drop_undo)
                                        .Set(Am_REDO_METHOD, drop_undo));

  Am_Object window2 =
      Am_Window.Create()
          .Set(Am_LEFT, 310)
          .Set(Am_WIDTH, 600)
          .Set(Am_HEIGHT, Am_Height_Of_Parts)
          .Set(Am_UNDO_HANDLER, undo_handler)
          .Set(Am_DESTROY_WINDOW_METHOD, Am_Window_Destroy_And_Exit_Method)
          .Add(Am_LAYOUT, Am_Vertical_Layout)
          .Set(Am_H_ALIGN, Am_LEFT_ALIGN)
          .Add_Part(Am_Menu_Bar.Create().Set(
              Am_ITEMS,
              Am_Value_List()
                  .Add(Am_Command.Create("Menu")
                           .Set(Am_IMPLEMENTATION_PARENT, true) //not undoable
                           .Set(Am_LABEL, "Menu1")
                           .Set(Am_ITEMS,
                                Am_Value_List().Add("Item1").Add("Item2")))
                  .Add(Am_Command.Create("menu2")
                           .Set(Am_IMPLEMENTATION_PARENT, true) //not undoable
                           .Set(Am_LABEL, "Menu2")
                           .Set(Am_ITEMS,
                                Am_Value_List().Add("New").Add("Middle").Add(
                                    "Old")))))
          .Add_Part(Am_Group.Create()
                        .Set(Am_WIDTH, Am_Width_Of_Parts)
                        .Set(Am_HEIGHT, Am_Height_Of_Parts)
                        .Add_Part(scroll2)
                        .Add_Part(target));

  Am_Value_List select_list =
      Am_Value_List().Add(select1).Add(select2).Add(select3);

  select1.Set(Am_MULTI_SELECTIONS, select_list);

  select2.Set(Am_MULTI_SELECTIONS, select_list);

  select3.Set(Am_MULTI_SELECTIONS, select_list);

  Am_Screen.Add_Part(window1).Add_Part(window2).Add_Part(
      Am_Window.Create()
          .Set(Am_TOP, 460)
          .Set(Am_WIDTH, 300)
          .Set(Am_HEIGHT, 200)
          .Set(Am_UNDO_HANDLER, undo_handler)
          .Set(Am_DESTROY_WINDOW_METHOD, Am_Window_Destroy_And_Exit_Method)
          .Add_Part(target.Copy()
                        .Set(Am_LEFT, 100)
                        .Set(Am_TOP, 80)
                        .Get_Object(Am_DROP_TARGET)
                        .Add(Am_DROP_TARGET_TEST, no_reds)
                        .Get_Owner())
          .Add_Part(
              Am_Button_Panel.Create()
                  .Set(Am_LEFT, 20)
                  .Set(Am_TOP, 10)
                  .Set(Am_ITEMS, Am_Value_List()
                                     .Add(Am_Undo_Command.Create())
                                     .Add(Am_Redo_Command.Create())
                                     .Add(Am_Quit_No_Ask_Command.Create()))));

  Am_Main_Event_Loop();
  Am_Cleanup();

  return 0;
}
