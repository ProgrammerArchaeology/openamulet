#include <amulet.h>
#include "externs.h"

Am_Slot_Key IMAGE_OBJ = Am_Register_Slot_Name("IMAGE_OBJ");
Am_Slot_Key CHOICE_PANEL = Am_Register_Slot_Name("CHOICE_PANEL");
Am_Slot_Key OK_CANCEL = Am_Register_Slot_Name("OK_CANCEL");

Am_Object Image_Label_Dialog;
Am_Object Image_File_Input;
int Height;
int Left;
int Width;

#define SPACING 1
#define EXPAND_BY 80

Am_Object graphics_group_proto;

Am_Define_Formula(int, get_owner_size_if_valid)
{
  Am_Value v = self.Get_Owner().Get(Am_HEIGHT);
  if (v.Valid()) {
    self.Set(Am_HEIGHT, (int)v);
    v = self.Get_Owner().Get(Am_WIDTH);
    if (v.Valid())
      return (int)v;
    else
      return 0;
  } else
    return 0;
}

Am_Define_Formula(int, place_to_right)
{
  Am_Value v = self.Get(Am_WIDTH);
  if (v.Valid()) {
    int o_width = (int)self.Get_Owner().Get(Am_WIDTH);
    return o_width - (int)v;
  } else
    return 0;
}

Am_Define_Formula(int, compute_top_coord)
{
  int top = (int)self.Get_Sibling(LABEL_OBJ).Get(Am_TOP);
  int height = (int)self.Get_Sibling(LABEL_OBJ).Get(Am_HEIGHT);

  return top + height + SPACING;
}

Am_Define_Formula(int, compute_height)
{
  Am_Value v = self.Get(Am_TOP);

  if (v.Valid()) {
    Am_Object ok_cancel = self.Get_Sibling(OK_CANCEL);
    int top = (int)ok_cancel.Get(Am_TOP);

    return top - (int)v - SPACING;
  } else
    return 0;
}

Am_Define_Formula(int, get_owner_width_if_ready)
{
  Am_Object owner = self.Get_Owner();
  Am_Value v = owner.Get(Am_WIDTH);

  if (v.Valid())
    return (int)v - 3;
  else
    return 0;
}

Am_Define_Formula(int, place_to_bottom)
{
  Am_Object window = self.Get_Owner();
  int w_height = window.Get(Am_HEIGHT);
  int my_height = self.Get(Am_HEIGHT);

  return w_height - my_height;
}

Am_Define_Method(Am_Object_Method, void, return_label, (Am_Object /*cmd*/))
{
  Am_Finish_Pop_Up_Waiting(Image_Label_Dialog, "yes");
}

Am_Define_Method(Am_Object_Method, void, return_null, (Am_Object /*cmd*/))
{
  Am_Finish_Pop_Up_Waiting(Image_Label_Dialog, Am_No_Value);
}

Am_Define_Method(Am_Object_Method, void, disable_text_enable_image,
                 (Am_Object cmd))
{
  Am_Object image =
      cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner().Get_Sibling(IMAGE_OBJ);

  if (!(bool)image.Get(Am_VISIBLE)) {
    Am_Object text = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Sibling(LABEL_OBJ);
    Am_Object window = image.Get_Owner();
    Am_Object ok_cancel = image.Get_Sibling(OK_CANCEL);
    int h = (int)window.Get(Am_HEIGHT) + EXPAND_BY;
    int w = (int)window.Get(Am_WIDTH);
    int tmp;
    Am_Value_List l;

    text.Set(Am_ACTIVE, false);
    image.Set(Am_VISIBLE, true);
    image = image.Get_Object(IMAGE_OBJ);
    l = image.Get(Am_GRAPHICAL_PARTS);
    if (!l.Empty()) {
      l.Start();
      image = (Am_Object)l.Get();
      if (image.Is_Instance_Of(Am_Bitmap)) {
        h = (int)image.Get(Am_HEIGHT) + Height + 10;
        w = (int)image.Get(Am_WIDTH) + Width + 10;
        if (h < (tmp = (Height + EXPAND_BY)))
          h = tmp;
        if (w < (tmp = (Width + Left)))
          w = tmp;
      }
    }

    window.Set(Am_HEIGHT, h);
    window.Set(Am_WIDTH, w);
    ok_cancel.Set(Am_TOP, place_to_bottom);
  }
}

Am_Define_Method(Am_Object_Method, void, disable_image_enable_text,
                 (Am_Object cmd))
{
  Am_Object text = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Sibling(LABEL_OBJ);

  if (!(bool)text.Get(Am_ACTIVE)) {
    Am_Object image =
        cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner().Get_Sibling(IMAGE_OBJ);
    int top;
    Am_Object ok_cancel = image.Get_Sibling(OK_CANCEL);

    text.Set(Am_ACTIVE, true);
    image.Set(Am_VISIBLE, false);
    ok_cancel.Set(Am_TOP, Am_From_Sibling(IMAGE_OBJ, Am_TOP, +3));
    top = (int)ok_cancel.Get(Am_TOP) + (int)ok_cancel.Get(Am_HEIGHT);
    image.Get_Owner().Set(Am_HEIGHT, top);
  }
}

Am_Define_Method(Am_Object_Method, void, load_image_file, (Am_Object cmd))
{
  Am_Object window =
      cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner().Get_Owner().Get_Owner();
  Am_Object image = window.Get_Object(IMAGE_OBJ).Get_Object(IMAGE_OBJ);

  Am_String file_name = (Am_String)window.Get(FILE_NAME);
  Image_File_Input.Get_Object(LABEL_OBJ).Set(Am_VALUE, (char *)file_name);
  Am_Value v;
  int h, w, tmp;

  Am_Pop_Up_Window_And_Wait(Image_File_Input, v, true);

  if (v.Valid()) {
    Am_Value_List l = image.Get(Am_GRAPHICAL_PARTS);

    l.Make_Empty();
    image.Set(Am_GRAPHICAL_PARTS, l);
    window.Set(FILE_NAME, (Am_String)v);
    image.Add_Part(bitmap_proto.Create()
                       .Set(Am_IMAGE, Am_Image_Array((char *)(Am_String)v))
                       .Set(Am_LEFT, 10)
                       .Set(FILE_NAME, (char *)(Am_String)v)
                       .Set(Am_TOP, 10));
    h = (int)image.Get(Am_HEIGHT) + Height + 10;
    w = (int)image.Get(Am_WIDTH) + Width + 10;
    if (h < (tmp = (Height + EXPAND_BY)))
      h = tmp;
    if (w < (tmp = (Width + Left)))
      w = tmp;
    window.Set(Am_WIDTH, w);
    window.Set(Am_HEIGHT, h);
  }
}

Am_Define_Method(Am_Object_Method, void, use_clipboard, (Am_Object cmd))
{
  Am_Object image =
      cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner().Get_Sibling(IMAGE_OBJ);
  Am_Value_List clip_b = (Am_Value_List)Am_Global_Clipboard.Get(Am_VALUE);
  Am_Value_List label = (Am_Value_List)image.Get(Am_GRAPHICAL_PARTS);
  Am_Object obj, group;

  if (!clip_b.Empty()) {
    int h, w, tmp;

    label.Make_Empty();
    image.Set(Am_GRAPHICAL_PARTS, label);
    if (clip_b.Length() > 1) {
      int small_x = 9999, small_y = 9999;
      for (clip_b.Start(); !clip_b.Last(); clip_b.Next()) {
        obj = (Am_Object)clip_b.Get();
        if ((int)obj.Get(Am_LEFT) < small_x)
          small_x = (int)obj.Get(Am_LEFT);
        if ((int)obj.Get(Am_TOP) < small_y)
          small_y = (int)obj.Get(Am_TOP);
      }

      group = graphics_group_proto.Create();

      for (clip_b.Start(); !clip_b.Last(); clip_b.Next()) {
        obj = ((Am_Object)clip_b.Get()).Copy();
        tmp = (int)obj.Get(Am_LEFT);
        obj.Set(Am_LEFT, (tmp - small_x));
        tmp = (int)obj.Get(Am_TOP);
        obj.Set(Am_TOP, (tmp - small_y));
        group.Add_Part(obj);
      }
    } else {
      clip_b.Start();
      group = (Am_Object)clip_b.Get();
    }

    group.Set(Am_LEFT, 10)
        .Set(Am_TOP, 10)
        .Set(Lw_NAME, "")
        .Set(TYPE_NAME, "Am_Group");
    image.Add_Part(group);
    h = (int)image.Get(Am_HEIGHT) + Height + 10;
    w = (int)image.Get(Am_WIDTH) + Width + 10;
    if (h < (tmp = (Height + EXPAND_BY)))
      h = tmp;
    if (w < (tmp = (Width + Left)))
      w = tmp;
    Image_Label_Dialog.Set(Am_WIDTH, w);
    Image_Label_Dialog.Set(Am_HEIGHT, h);
  } else
    Am_Show_Alert_Dialog("Clipboard is empty");
}

Am_Define_Method(Am_Object_Method, void, paste_to_clipboard, (Am_Object cmd))
{
  Am_Object image =
      cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner().Get_Sibling(IMAGE_OBJ);
  Am_Value_List clip_b = (Am_Value_List)Am_Global_Clipboard.Get(Am_VALUE);
  Am_Value_List label = (Am_Value_List)image.Get(Am_GRAPHICAL_PARTS);
  label.Start();

  clip_b.Add((Am_Object)label.Get());
  Am_Global_Clipboard.Set(Am_VALUE, clip_b);
}

Am_Define_Method(Am_Object_Method, void, return_file_name, (Am_Object cmd))
{
  Am_Object file_name =
      cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Sibling(LABEL_OBJ);
  Am_Object window = file_name.Get_Owner();
  Am_String name = file_name.Get(Am_VALUE);
  FILE *fptr;
  char s[100];

  window.Set(Am_VISIBLE, false);

  if ((fptr = fopen((char *)name, "r")) == NULL) {
    sprintf(s, "File %s does not exists", (char *)name);
    Am_Show_Alert_Dialog((Am_String)s);
    window.Set(Am_VISIBLE, true);
  } else
    Am_Finish_Pop_Up_Waiting(window, file_name.Get(Am_VALUE));
}

Am_Define_Method(Am_Object_Method, void, do_not_return_file_name,
                 (Am_Object cmd))
{
  Am_Object window = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();

  Am_Finish_Pop_Up_Waiting(window, Am_No_Value);
}

Am_Object
Image_Label_Dialog_Initialize()
{
  Am_Object command;
  Am_Object image = Am_Group.Create();

  graphics_group_proto =
      Am_Group.Create("graphics_group_proto")
          .Add(Lw_NAME, "")
          .Add(TYPE_NAME, "Am_GROUP")
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Group)
          .Add(Am_SLOTS_TO_SAVE,
               Am_Value_List().Add(Lw_NAME).Add(TYPE_NAME).Add(Am_WIDTH).Add(
                   Am_HEIGHT))
          .Set(Am_WIDTH, Am_Width_Of_Parts)
          .Set(Am_HEIGHT, Am_Height_Of_Parts);

  Am_Default_Load_Save_Context.Register_Prototype("GRAPHICS_GROUP_PROTO",
                                                  graphics_group_proto);

  command = Am_Command.Create()
                .Set(Am_LABEL, "Text Label")
                .Set(Am_DO_METHOD, disable_image_enable_text);

  Image_Label_Dialog =
      Am_Window.Create()
          .Set(Am_TOP, 100)
          .Set(Am_LEFT, 200)
          .Set(Am_WIDTH, 250)
          .Set(Am_FILL_STYLE, Am_White)
          .Add(FILE_NAME, "")
          .Add_Part(NAME_OBJ,
                    Am_Group.Create()
                        .Set(Am_TOP, 0)
                        .Set(Am_LEFT, 0)
                        .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH))
                        .Set(Am_HEIGHT, Am_Height_Of_Parts)
                        .Add_Part(Am_Border_Rectangle.Create()
                                      .Set(Am_LEFT, 0)
                                      .Set(Am_TOP, 0)
                                      .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH))
                                      .Set(Am_HEIGHT, 40))
                        .Add_Part(NAME_OBJ, Am_Text_Input_Widget.Create()
                                                .Set(Am_TOP, 10)
                                                .Set(Am_LEFT, 30)
                                                .Get_Object(Am_COMMAND)
                                                .Set(Am_LABEL, "Item Name")
                                                .Get_Owner()))
          .Add_Part(
              LABEL_OBJ,
              Am_Group.Create("label_group")
                  .Set(Am_TOP, Am_From_Sibling(NAME_OBJ, Am_HEIGHT, SPACING))
                  .Set(Am_LEFT, 0)
                  .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH))
                  .Set(Am_HEIGHT, Am_Height_Of_Parts)
                  .Add_Part(Am_Border_Rectangle.Create().Set(
                      Am_WIDTH, get_owner_size_if_valid))
                  .Add_Part(CHOICE_PANEL,
                            Am_Radio_Button_Panel.Create()
                                .Set(Am_TOP, SPACING)
                                .Set(Am_LEFT, 20)
                                .Set(Am_LAYOUT, Am_Vertical_Layout)
                                .Set(Am_V_SPACING, 23)
                                .Set(Am_ITEMS,
                                     Am_Value_List().Add(command).Add(
                                         Am_Command.Create()
                                             .Set(Am_LABEL, "Image Label")
                                             .Set(Am_DO_METHOD,
                                                  disable_text_enable_image))))
                  .Add_Part(LABEL_OBJ, Am_Text_Input_Widget.Create()
                                           .Set(Am_TOP, 23)
                                           .Set(Am_LEFT, 30)
                                           .Get_Object(Am_COMMAND)
                                           .Set(Am_LABEL, "Item Label")
                                           .Get_Owner()))
          .Add_Part(
              IMAGE_OBJ,
              Am_Group.Create()
                  .Set(Am_VISIBLE, false)
                  .Set(Am_TOP, compute_top_coord)
                  .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH))
                  .Set(Am_HEIGHT, compute_height)
                  .Add_Part(IMAGE_OBJ, Am_Group.Create()
                                           .Set(Am_TOP, 0)
                                           .Set(Am_LEFT, 0)
                                           .Set(Am_WIDTH, Am_Width_Of_Parts)
                                           .Set(Am_HEIGHT, Am_Height_Of_Parts))
                  .Add_Part(
                      CHOICE_PANEL,
                      Am_Group.Create()
                          .Set(Am_TOP, 0)
                          .Set(Am_WIDTH, Am_Width_Of_Parts)
                          .Set(Am_HEIGHT, Am_From_Owner(Am_HEIGHT))
                          .Add_Part(
                              Am_Border_Rectangle.Create()
                                  .Set(Am_HEIGHT, Am_From_Owner(Am_HEIGHT))
                                  .Set(Am_TOP, 0)
                                  .Set(Am_LEFT, 0)
                                  .Set(Am_WIDTH, Am_From_Sibling(CHOICE_PANEL,
                                                                 Am_WIDTH, 6)))
                          .Add_Part(
                              CHOICE_PANEL,
                              Am_Button_Panel.Create()
                                  .Set(Am_WIDTH, Am_Width_Of_Parts)
                                  .Set(Am_HEIGHT, Am_From_Owner(Am_HEIGHT))
                                  .Set(Am_LEFT, 3)
                                  .Set(Am_TOP, 3)
                                  .Set(Am_V_SPACING, 3)
                                  .Set(Am_ITEMS,
                                       Am_Value_List()
                                           .Add(Am_Command.Create()
                                                    .Set(Am_LABEL, "From File")
                                                    .Set(Am_DO_METHOD,
                                                         load_image_file))
                                           .Add(Am_Command.Create()
                                                    .Set(Am_LABEL,
                                                         "From Clipboard")
                                                    .Set(Am_DO_METHOD,
                                                         use_clipboard))
                                           .Add(Am_Command.Create()
                                                    .Set(Am_LABEL,
                                                         "To Clipboard")
                                                    .Set(Am_DO_METHOD,
                                                         paste_to_clipboard))))
                          .Set(Am_LEFT, place_to_right)))
          .Add_Part(
              OK_CANCEL,
              Am_Group.Create()
                  .Set(Am_TOP, Am_From_Sibling(IMAGE_OBJ, Am_TOP, SPACING))
                  .Set(Am_LEFT, 0)
                  .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH))
                  .Set(Am_HEIGHT, 40)
                  .Add_Part(Am_Border_Rectangle.Create()
                                .Set(Am_TOP, 0)
                                .Set(Am_LEFT, 0)
                                .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH))
                                .Set(Am_HEIGHT, Am_From_Owner(Am_HEIGHT)))
                  .Add_Part(
                      Am_Button_Panel.Create()
                          .Set(Am_LEFT, Am_From_Owner(Am_WIDTH, -60, 0.5))
                          .Set(Am_TOP, 5)
                          .Set(Am_LAYOUT, Am_Horizontal_Layout)
                          .Set(Am_ITEMS,
                               Am_Value_List()
                                   .Add(Am_Command.Create()
                                            .Set(Am_LABEL, "OK")
                                            .Set(Am_DO_METHOD, return_label))
                                   .Add(Am_Command.Create()
                                            .Set(Am_LABEL, "Cancel")
                                            .Set(Am_DO_METHOD, return_null)))))
          .Set(Am_HEIGHT, Am_Height_Of_Parts);

  Image_File_Input =
      Am_Window.Create()
          .Set(Am_TOP, Am_From_Object(Image_Label_Dialog, Am_TOP, 100))
          .Set(Am_LEFT, Am_From_Object(Image_Label_Dialog, Am_LEFT, 100))
          .Set(Am_WIDTH, Am_Width_Of_Parts)
          .Set(Am_HEIGHT, Am_Height_Of_Parts)
          .Add_Part(LABEL_OBJ, Am_Text_Input_Widget.Create()
                                   .Set(Am_TOP, 3)
                                   .Set(Am_LEFT, 3)
                                   .Set(Am_WIDTH, 200)
                                   .Get_Object(Am_COMMAND)
                                   .Set(Am_LABEL, "File Name")
                                   .Get_Owner())
          .Add_Part(Am_Button_Panel.Create()
                        .Set(Am_TOP, 23)
                        .Set(Am_LEFT, 40)
                        .Set(Am_LAYOUT, Am_Horizontal_Layout)
                        .Set(Am_H_SPACING, 3)
                        .Set(Am_ITEMS,
                             Am_Value_List()
                                 .Add(Am_Command.Create()
                                          .Set(Am_LABEL, "Load File")
                                          .Set(Am_DO_METHOD, return_file_name))
                                 .Add(Am_Command.Create()
                                          .Set(Am_LABEL, "Cancel")
                                          .Set(Am_DO_METHOD,
                                               do_not_return_file_name))));

  Height = (int)Image_Label_Dialog.Get(Am_HEIGHT);
  Left = (int)Image_Label_Dialog.Get_Object(IMAGE_OBJ)
             .Get_Object(CHOICE_PANEL)
             .Get(Am_LEFT);
  Width = (int)Image_Label_Dialog.Get_Object(IMAGE_OBJ)
              .Get_Object(CHOICE_PANEL)
              .Get(Am_WIDTH);
  return Image_Label_Dialog;
}
