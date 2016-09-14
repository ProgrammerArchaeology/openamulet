/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <amulet.h>

using namespace std;

Am_Slot_Key PARENT = Am_Register_Slot_Name("PARENT");
Am_Slot_Key CHILDREN = Am_Register_Slot_Name("CHILDREN");
Am_Slot_Key PREV_CHILD = Am_Register_Slot_Name("PREV_CHILD");
Am_Slot_Key RUNG = Am_Register_Slot_Name("RUNG");
Am_Slot_Key OFFSET = Am_Register_Slot_Name("OFFSET");
Am_Slot_Key NODES = Am_Register_Slot_Name("NODES");
Am_Slot_Key LINES = Am_Register_Slot_Name("LINES");
Am_Slot_Key START_NODE = Am_Register_Slot_Name("START_NODE");
Am_Slot_Key END_NODE = Am_Register_Slot_Name("END_NODE");
Am_Slot_Key ANIMATOR = Am_Register_Slot_Name("ANIMATOR");
Am_Slot_Key COLORS = Am_Register_Slot_Name("COLORS");

int tree[10] = {9, 7, 9, 4, 2, 7, 0, 6, 2, 9};

Am_Define_Formula(int, find_rung)
{
  int parent_rank = self.Get(PARENT);
  int rank = self.Get(Am_RANK);
  if (rank == parent_rank)
    return 0;
  Am_Value_List nodes = self.Get_Owner().Get(Am_GRAPHICAL_PARTS);
  if (nodes.Valid()) {
    Am_Object parent = nodes.Get_Nth(parent_rank);
    int parent_rung = parent.Get(RUNG);
    return parent_rung + 1;
  }
  return 0;
}

Am_Define_Formula(int, find_children)
{
  int rank = self.Get(Am_RANK);
  int children = 0;
  Am_Value_List nodes = self.Get_Owner().Get(Am_GRAPHICAL_PARTS);
  for (nodes.Start(); !nodes.Last(); nodes.Next()) {
    Am_Object item = nodes.Get();
    if ((int)item.Get(PARENT) == rank && item != self)
      children += (int)item.Get(CHILDREN);
  }
  if (children)
    return children;
  else
    return 1;
}

Am_Define_Object_Formula(find_prev_child)
{
  int rank = self.Get(Am_RANK);
  int parent_rank = self.Get(PARENT);
  if (rank == parent_rank)
    return Am_No_Object;
  Am_Value_List nodes = self.Get_Owner().Get(Am_GRAPHICAL_PARTS);
  if (nodes.Valid()) {
    nodes.Move_Nth(rank);
    nodes.Next();
    while (!nodes.Last()) {
      Am_Object item = nodes.Get();
      if ((int)item.Get(PARENT) == parent_rank &&
          (int)item.Get(Am_RANK) != parent_rank)
        return item;
      nodes.Next();
    }
  }
  return Am_No_Object;
}

Am_Define_Formula(int, calc_offset)
{
  Am_Object prev_child = self.Get(PREV_CHILD);
  if (prev_child.Valid()) {
    int prev_offset = prev_child.Get(OFFSET);
    int prev_children = prev_child.Get(CHILDREN);
    return prev_offset + prev_children;
  } else {
    int rank = self.Get(Am_RANK);
    int parent_rank = self.Get(PARENT);
    if (rank == parent_rank)
      return 0;
    Am_Value_List nodes = self.Get_Owner().Get(Am_GRAPHICAL_PARTS);
    if (nodes.Valid()) {
      Am_Object parent = nodes.Get_Nth(parent_rank);
      return parent.Get(OFFSET);
    } else
      return 0;
  }
}

Am_Define_Formula(int, node_top)
{
  int rung = self.Get(RUNG);
  return 30 + 100 * rung;
}

Am_Define_Formula(int, node_left)
{
  int offset = self.Get(OFFSET);
  int children = self.Get(CHILDREN);
  return 60 * offset + 30 * children;
}

Am_Define_Object_Formula(find_start_node)
{
  int parent = self.Get(Am_ITEM);
  int rank = self.Get(Am_RANK);
  if (parent == rank)
    return Am_No_Object;
  Am_Value_List nodes =
      self.Get_Owner().Get_Sibling(NODES).Get(Am_GRAPHICAL_PARTS);
  if (nodes.Valid())
    return nodes.Get_Nth(parent);
  else
    return Am_No_Object;
}

Am_Define_Object_Formula(find_end_node)
{
  int rank = self.Get(Am_RANK);
  Am_Value_List nodes =
      self.Get_Owner().Get_Sibling(NODES).Get(Am_GRAPHICAL_PARTS);
  if (nodes.Valid())
    return nodes.Get_Nth(rank);
  else
    return Am_No_Object;
}

Am_Define_Formula(int, line_x1)
{
  Am_Object node = self.Get(START_NODE);
  if (!node.Valid())
    node = self.Get(END_NODE);
  return (int)node.Get(Am_LEFT) + (int)node.Get(Am_WIDTH) / 2;
}

Am_Define_Formula(int, line_x2)
{
  Am_Object node = self.Get(END_NODE);
  return (int)node.Get(Am_LEFT) + (int)node.Get(Am_WIDTH) / 2;
}

Am_Define_Formula(int, line_y1)
{
  Am_Object node = self.Get(START_NODE);
  if (node.Valid())
    return (int)node.Get(Am_TOP) + (int)node.Get(Am_HEIGHT);
  else
    return 0;
}

Am_Define_Formula(int, line_y2)
{
  Am_Object node = self.Get(END_NODE);
  return node.Get(Am_TOP);
}

Am_Define_Method(Am_Object_Method, void, node_click, (Am_Object command))
{
  Am_Object inter = command.Get_Owner();
  Am_Object node = inter.Get(Am_START_OBJECT);
  int rank = node.Get(Am_RANK);
  int prev_rank = rank;
  int next_rank = tree[rank];
  while (next_rank != rank) {
    tree[rank] = prev_rank;
    prev_rank = rank;
    rank = next_rank;
    next_rank = tree[rank];
  }
  tree[rank] = prev_rank;

  int i;
  Am_Value_List parents;
  for (i = 0; i < 10; ++i)
    parents.Add(tree[i]);
  Am_Object nodes = inter.Get_Owner();
  Am_Object lines = nodes.Get_Sibling(LINES);
  nodes.Set(Am_ITEMS, parents);
  lines.Set(Am_ITEMS, parents);
}

Am_Define_Method(Am_Object_Method, void, key_press, (Am_Object command))
{
  Am_Object inter = command.Get_Owner();
  Am_Input_Char c = inter.Get(Am_START_CHAR);
  if (c == "q")
    Am_Exit_Main_Event_Loop();
  else if (c == "a") {
    Am_Object animator = inter.Get_Owner().Get(ANIMATOR);
    bool active = animator.Get(Am_ACTIVE);
    animator.Set(Am_ACTIVE, !active);
  } else
    cout << "Keyboard Commands:" << endl
         << "  a - toggle animation" << endl
         << "  q - quit" << endl;
}

Am_Define_String_Formula(value_to_string)
{
  int value = self.Get(Am_VALUE);
  static char buffer[100];
  sprintf(buffer, "%d", value);
  Am_String string(buffer);
  return string;
}

Am_Define_Style_Formula(node_color)
{
  Am_Value_List colors = self.Get(COLORS);
  if (colors.Valid()) {
    int rank = self.Get_Owner().Get(Am_RANK);
    if (0 <= rank && rank < 10)
      return colors.Get_Nth(rank);
  }
  return Am_No_Style;
}

int
main(void)
{
  Am_Initialize();

  int i;
  Am_Value_List parents;
  for (i = 0; i < 10; ++i)
    parents.Add(tree[i]);

  Am_Font big_font(Am_FONT_SANS_SERIF, true, false, false, Am_FONT_LARGE);

  Am_Object animator = Am_Animator.Create("animator")
                           .Set(Am_DURATION, Am_Time(1000))
                           .Set(Am_ACTIVE, false);

  Am_Screen.Add_Part(
      Am_Window.Create("window")
          .Set(Am_LEFT, 50)
          .Set(Am_TOP, 50)
          .Set(Am_WIDTH, 300)
          .Set(Am_HEIGHT, 700)
          .Set(Am_FILL_STYLE, Am_Motif_Gray)
          .Add(ANIMATOR, animator)
          .Add_Part(LINES, Am_Map.Create()
                               .Set(Am_ITEMS, parents)
                               .Set(Am_ITEM_PROTOTYPE,
                                    Am_Arrow_Line.Create("line")
                                        .Set(Am_LINE_STYLE, Am_Line_2)
                                        .Set(Am_HEAD_LENGTH, 10)
                                        .Set(Am_HEAD_WIDTH, 5)
                                        .Add(START_NODE, find_start_node)
                                        .Add(END_NODE, find_end_node)
                                        .Set(Am_X1, line_x1)
                                        .Set(Am_X2, line_x2)
                                        .Set(Am_Y1, line_y1)
                                        .Set(Am_Y2, line_y2)))
          .Add_Part(
              NODES,
              Am_Map.Create()
                  .Set(Am_ITEMS, parents)
                  .Set(Am_ITEM_PROTOTYPE,
                       Am_Group.Create("node")
                           .Add(PARENT, Am_Same_As(Am_ITEM))
                           .Add(CHILDREN, find_children)
                           .Add(PREV_CHILD, find_prev_child)
                           .Add(RUNG, find_rung)
                           .Add(OFFSET, calc_offset)
                           .Set(Am_LEFT, node_left)
                           .Set(Am_TOP, node_top)
                           .Set(Am_LEFT, Am_Animate_With(animator))
                           .Set(Am_TOP, Am_Animate_With(animator))
                           .Set(Am_WIDTH, 50)
                           .Set(Am_HEIGHT, 50)
                           .Add_Part(Am_Arc.Create("arc")
                                         .Add(COLORS, Am_Value_List()
                                                          .Add(Am_Black)
                                                          .Add(Am_White)
                                                          .Add(Am_Red)
                                                          .Add(Am_Blue)
                                                          .Add(Am_Purple)
                                                          .Add(Am_Green)
                                                          .Add(Am_Cyan)
                                                          .Add(Am_Yellow)
                                                          .Add(Am_Amulet_Purple)
                                                          .Add(Am_Orange))
                                         .Set(Am_FILL_STYLE, node_color)
                                         .Set(Am_WIDTH, 50)
                                         .Set(Am_HEIGHT, 50))
                           .Add_Part(
                               Am_Arc.Create("arc")
                                   .Set(Am_FILL_STYLE, Am_White)
                                   .Set(Am_LEFT, Am_Center_X_Is_Center_Of_Owner)
                                   .Set(Am_TOP, Am_Center_Y_Is_Center_Of_Owner)
                                   .Set(Am_WIDTH, 30)
                                   .Set(Am_HEIGHT, 30))
                           .Add_Part(
                               Am_Text.Create("text")
                                   .Set(Am_FONT, big_font)
                                   .Set(Am_LEFT, Am_Center_X_Is_Center_Of_Owner)
                                   .Set(Am_TOP, Am_Center_Y_Is_Center_Of_Owner)
                                   .Add(Am_VALUE, Am_From_Owner(Am_RANK))
                                   .Set(Am_TEXT, value_to_string)))
                  .Add_Part(Am_One_Shot_Interactor.Create("click")
                                .Set(Am_START_WHERE_TEST, Am_Inter_In_Part)
                                .Get_Object(Am_COMMAND)
                                .Set(Am_DO_METHOD, node_click)
                                .Get_Owner()))
          .Add_Part(Am_One_Shot_Interactor.Create("key")
                        .Set(Am_START_WHEN, "ANY_KEYBOARD")
                        .Get_Object(Am_COMMAND)
                        .Set(Am_DO_METHOD, key_press)
                        .Get_Owner()));

  cout << "Keyboard Commands:" << endl
       << "  a - toggle animation" << endl
       << "  q - quit" << endl;

  Am_Main_Event_Loop();
  Am_Cleanup();

  return 0;
}
