#include <am_inc.h>

#include OPAL__H
#include OPAL_ADVANCED__H

#include STANDARD_SLOTS__H
#include INTER__H
#include VALUE_LIST__H

using namespace std;

Am_Slot_Key STAGGER = Am_Register_Slot_Name("STAGGER");

Am_Object window;
Am_Object map1;
Am_Object layout1;
Am_Object layout_proto1;
Am_Object layout2;
Am_Object layout_proto2;

void
print_help()
{
  cout << "s: toggle stagger" << endl;
  cout << "r: toggle max rank" << endl;
  cout << "1: toggle max rank of 1" << endl;
  cout << "z: toggle max size" << endl;
  cout << "w: toggle fixed width" << endl;
  cout << "h: toggle fixed height" << endl;
  cout << "a: switch h alignment" << endl;
  cout << "b: switch v alignment" << endl;
  cout << "i: toggle indent" << endl;
  cout << "S: switch between 4 and 8 item mode" << endl;
  cout << "T: toggle the creation of an instance" << endl;
  cout << "q: quit" << endl;
}

Am_Define_Method(Am_Object_Method, void, change_setting, (Am_Object self))
{
  Am_Object inter = self.Get_Owner();
  Am_Input_Char c = inter.Get(Am_START_CHAR);
  if (c == "q") {
    Am_Exit_Main_Event_Loop();
  } else if (c == "S") {
    static bool switcher = false;
    if (switcher) {
      switcher = false;
      map1.Set(Am_ITEMS, 4);
    } else {
      switcher = true;
      map1.Set(Am_ITEMS, 8);
    }
  } else if (c == "T") {
    static Am_Object instance = (0L);
    if (instance.Valid()) {
      instance.Destroy();
    } else {
      instance = map1.Create("map2").Set(Am_TOP, 50);
      window.Add_Part(instance);
    }
  } else if (c == "s") {
    bool stagger = layout_proto1.Get(STAGGER);
    layout_proto1.Set(STAGGER, !stagger);
    stagger = layout_proto2.Get(STAGGER);
    layout_proto2.Set(STAGGER, !stagger);
  } else if (c == "r") {
    int max_rank;
    if (layout1.Get_Slot_Type(Am_MAX_RANK) == Am_INT)
      max_rank = layout1.Get(Am_MAX_RANK);
    else
      max_rank = 0;
    layout1.Set(Am_MAX_RANK, max_rank ? 0 : 3);
    if (layout2.Get_Slot_Type(Am_MAX_RANK) == Am_INT)
      max_rank = layout2.Get(Am_MAX_RANK);
    else
      max_rank = 0;
    layout2.Set(Am_MAX_RANK, max_rank ? 0 : 3);
  } else if (c == "1") {
    int max_rank;
    if (layout1.Get_Slot_Type(Am_MAX_RANK) == Am_INT)
      max_rank = layout1.Get(Am_MAX_RANK);
    else
      max_rank = 0;
    layout1.Set(Am_MAX_RANK, max_rank ? 0 : 1);
    if (layout2.Get_Slot_Type(Am_MAX_RANK) == Am_INT)
      max_rank = layout2.Get(Am_MAX_RANK);
    else
      max_rank = 0;
    layout2.Set(Am_MAX_RANK, max_rank ? 0 : 1);
  } else if (c == "z") {
    int max_size;
    if (layout1.Get_Slot_Type(Am_MAX_SIZE) == Am_INT)
      max_size = layout1.Get(Am_MAX_SIZE);
    else
      max_size = 0;
    layout1.Set(Am_MAX_SIZE, max_size ? 0 : 70);
    if (layout2.Get_Slot_Type(Am_MAX_SIZE) == Am_INT)
      max_size = layout2.Get(Am_MAX_SIZE);
    else
      max_size = 0;
    layout2.Set(Am_MAX_SIZE, max_size ? 0 : 94);
  } else if (c == "w") {
    int fixed_width = layout1.Get(Am_FIXED_WIDTH);
    layout1.Set(Am_FIXED_WIDTH,
                fixed_width ? Am_NOT_FIXED_SIZE : Am_MAX_FIXED_SIZE);
    fixed_width = layout2.Get(Am_FIXED_WIDTH);
    layout2.Set(Am_FIXED_WIDTH,
                fixed_width ? Am_NOT_FIXED_SIZE : Am_MAX_FIXED_SIZE);
  } else if (c == "h") {
    int fixed_height = layout1.Get(Am_FIXED_HEIGHT);
    layout1.Set(Am_FIXED_HEIGHT,
                fixed_height ? Am_NOT_FIXED_SIZE : Am_MAX_FIXED_SIZE);
    fixed_height = layout2.Get(Am_FIXED_HEIGHT);
    layout2.Set(Am_FIXED_HEIGHT,
                fixed_height ? Am_NOT_FIXED_SIZE : Am_MAX_FIXED_SIZE);
  } else if (c == "a") {
    Am_Alignment h_align = layout1.Get(Am_H_ALIGN);
    switch (h_align.value) {
    case Am_LEFT_ALIGN_val:
      layout1.Set(Am_H_ALIGN, Am_RIGHT_ALIGN);
      break;
    case Am_RIGHT_ALIGN_val:
      layout1.Set(Am_H_ALIGN, Am_CENTER_ALIGN);
      break;
    default:
      layout1.Set(Am_H_ALIGN, Am_LEFT_ALIGN);
      break;
    }
    h_align = layout2.Get(Am_H_ALIGN);
    switch (h_align.value) {
    case Am_LEFT_ALIGN_val:
      layout2.Set(Am_H_ALIGN, Am_RIGHT_ALIGN);
      break;
    case Am_RIGHT_ALIGN_val:
      layout2.Set(Am_H_ALIGN, Am_CENTER_ALIGN);
      break;
    default:
      layout2.Set(Am_H_ALIGN, Am_LEFT_ALIGN);
      break;
    }
  } else if (c == "b") {
    Am_Alignment v_align = layout1.Get(Am_V_ALIGN);
    switch (v_align.value) {
    case Am_TOP_ALIGN_val:
      layout1.Set(Am_V_ALIGN, Am_BOTTOM_ALIGN);
      break;
    case Am_BOTTOM_ALIGN_val:
      layout1.Set(Am_V_ALIGN, Am_CENTER_ALIGN);
      break;
    default:
      layout1.Set(Am_V_ALIGN, Am_TOP_ALIGN);
      break;
    }
    v_align = layout2.Get(Am_V_ALIGN);
    switch (v_align.value) {
    case Am_TOP_ALIGN_val:
      layout2.Set(Am_V_ALIGN, Am_BOTTOM_ALIGN);
      break;
    case Am_BOTTOM_ALIGN_val:
      layout2.Set(Am_V_ALIGN, Am_CENTER_ALIGN);
      break;
    default:
      layout2.Set(Am_V_ALIGN, Am_TOP_ALIGN);
      break;
    }
  } else if (c == "i") {
    int indent = layout1.Get(Am_INDENT);
    layout1.Set(Am_INDENT, indent ? 0 : 10);
    indent = layout2.Get(Am_INDENT);
    layout2.Set(Am_INDENT, indent ? 0 : 10);
  } else
    print_help();
}

Am_Define_Formula(int, map_left)
{
  int rank = self.Get(Am_RANK);
  int width = self.Get(Am_WIDTH);
  return (width + 5) * rank;
}

Am_Define_Formula(int, map_width)
{
  if ((bool)self.Get(STAGGER)) {
    int rank = self.Get(Am_RANK);
    if (rank % 2)
      return 25;
    else
      return 15;
  } else
    return 20;
}

Am_Define_Formula(int, map_height)
{
  if ((bool)self.Get(STAGGER)) {
    int rank = self.Get(Am_RANK);
    if (rank % 2)
      return 35;
    else
      return 25;
  } else
    return 30;
}

Am_Define_Style_Formula(map_fill)
{
  int rank = self.Get(Am_RANK);
  switch (rank) {
  case 0:
    return Am_Red;
  case 1:
    return Am_Blue;
  case 2:
    return Am_Green;
  case 3:
    return Am_Orange;
  case 4:
    return Am_Purple;
  case 5:
    return Am_Cyan;
  case 6:
    return Am_Yellow;
  default:
    return Am_White;
  }
}

Am_Define_Style_Formula(list_map_fill) { return self.Get(Am_ITEM); }

int
main()
{
  Am_Initialize();

  Am_Screen.Add_Part(
      window =
          Am_Window.Create("window")
              .Set(Am_TITLE, "Test Map")
              .Set(Am_FILL_STYLE, Am_Motif_Gray)
              .Set(Am_LEFT, 50)
              .Set(Am_TOP, 200)
              .Set(Am_WIDTH, 515)
              .Set(Am_HEIGHT, 430)
              .Add_Part(map1 = Am_Map.Create("count map")
                                   .Set(Am_LEFT, 10)
                                   .Set(Am_TOP, 10)
                                   .Set(Am_ITEMS, 4)
                                   .Set(Am_ITEM_PROTOTYPE,
                                        Am_Rectangle.Create("map item")
                                            .Set(Am_LINE_STYLE, Am_Black)
                                            .Set(Am_FILL_STYLE, map_fill)
                                            .Set(Am_WIDTH, 20)
                                            .Set(Am_HEIGHT, 30)
                                            .Set(Am_LEFT, map_left)
                                            .Set(Am_TOP, 0)))
              .Add_Part(Am_Map.Create("list map")
                            .Set(Am_LEFT, 10)
                            .Set(Am_TOP, 90)
                            .Set(Am_ITEMS, Am_Value_List()
                                               .Add(Am_Motif_Blue)
                                               .Add(Am_Motif_Green)
                                               .Add(Am_Motif_Orange)
                                               .Add(Am_Motif_Light_Blue))
                            .Set(Am_ITEM_PROTOTYPE,
                                 Am_Rectangle.Create("map item")
                                     .Set(Am_LINE_STYLE, Am_Black)
                                     .Set(Am_FILL_STYLE, list_map_fill)
                                     .Add(Am_ITEM, Am_Black)
                                     .Set(Am_WIDTH, 20)
                                     .Set(Am_HEIGHT, 30)
                                     .Set(Am_LEFT, map_left)
                                     .Set(Am_TOP, 0)))
              .Add_Part(layout1 = Am_Map.Create("horizontal layout")
                                      .Set(Am_LEFT, 10)
                                      .Set(Am_TOP, 130)
                                      .Set(Am_ITEMS, 8)
                                      .Set(Am_LAYOUT, Am_Horizontal_Layout)
                                      .Set(Am_H_SPACING, 5)
                                      .Set(Am_V_SPACING, 2)
                                      .Set(Am_ITEM_PROTOTYPE,
                                           layout_proto1 =
                                               Am_Rectangle.Create("map item")
                                                   .Set(Am_LINE_STYLE, Am_Black)
                                                   .Set(Am_FILL_STYLE, map_fill)
                                                   .Add(STAGGER, false)
                                                   .Set(Am_WIDTH, map_width)
                                                   .Set(Am_HEIGHT, map_height)))
              .Add_Part(layout2 = Am_Map.Create("horizontal layout")
                                      .Set(Am_LEFT, 270)
                                      .Set(Am_TOP, 10)
                                      .Set(Am_ITEMS, 8)
                                      .Set(Am_LAYOUT, Am_Vertical_Layout)
                                      .Set(Am_H_SPACING, 5)
                                      .Set(Am_V_SPACING, 2)
                                      .Set(Am_ITEM_PROTOTYPE,
                                           layout_proto2 =
                                               Am_Rectangle.Create("map item")
                                                   .Set(Am_LINE_STYLE, Am_Black)
                                                   .Set(Am_FILL_STYLE, map_fill)
                                                   .Add(STAGGER, false)
                                                   .Set(Am_WIDTH, map_width)
                                                   .Set(Am_HEIGHT, map_height)))
              .Add_Part(Am_One_Shot_Interactor.Create("keyboard press")
                            .Set(Am_START_WHEN, "ANY_KEYBOARD")
                            .Get_Object(Am_COMMAND)
                            .Set(Am_DO_METHOD, change_setting)
                            .Get_Owner()));

  print_help();

  Am_Main_Event_Loop();
  Am_Cleanup();

  return 0;
}
