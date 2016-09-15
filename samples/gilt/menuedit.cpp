#include <amulet.h>
#include "outline.h"
#include "externs.h"
#include <string.h>

using namespace std;

#define COMMAND_CNT 19

const char *n2cr[] = {"Am_Command",
                      "Am_Selection_Widget_Select_All_Command",
                      "Am_Graphics_Clear_Command",
                      "Am_Graphics_Clear_All_Command",
                      "Am_Graphics_Copy_Command",
                      "Am_Graphics_Cut_Command",
                      "Am_Graphics_Paste_Command",
                      "Am_Graphics_To_Botom_Command",
                      "Am_Graphics_To_Top_Command",
                      "Am_Graphics_Duplicate_Command",
                      "Am_Graphics_Group_Command",
                      "Am_Graphics_Ungroup_Command",
                      "Am_Undo_Command",
                      "Am_Redo_Command",
                      "Am_Quit_No_Ask_Command",
                      "Am_Open_Command",
                      "Am_Save_Command",
                      "Am_Save_As_Command",
                      "Am_Menu_Line_Command"};

const char *n2cl[] = {"NO LABEL", "Select All", "Clear",    "Clear All",
                      "Copy",     "Cut",        "Paste",    "To Bottom",
                      "To Top",   "Duplicate",  "Group",    "Ungroup",
                      "Undo",     "Redo",       "Quit",     "Open",
                      "Save",     "Save As",    "Menu Line"};

Am_Object n2c[COMMAND_CNT];

Am_Slot_Key COMMAND_KEY = Am_Register_Slot_Name("COMMAND_KEY");
Am_Slot_Key Lw_GROUP = Am_Register_Slot_Name("Lw_GROUP");
Am_Slot_Key OUTLINE = Am_Register_Slot_Name("OUTLINE");
Am_Slot_Key NUMBER_OF_NODES = Am_Register_Slot_Name("NUMBER_OF_NODES");
Am_Slot_Key CHOICE_INTER = Am_Register_Slot_Name("CHOICE_INTER");
Am_Slot_Key TEXT1 = Am_Register_Slot_Name("TEXT1");
Am_Slot_Key TEXT2 = Am_Register_Slot_Name("TEXT2");
Am_Slot_Key COMMAND_LIST = Am_Register_Slot_Name("COMMAND_LIST");
Am_Slot_Key NEW_DELETE_PANNEL = Am_Register_Slot_Name("NEW_DELETE_PANNEL");
Am_Slot_Key ACCELERATOR_INPUT = Am_Register_Slot_Name("ACCELERATOR_INPUT");
Am_Slot_Key SAVE_MENU_BAR = Am_Register_Slot_Name("SAVE_MENU_BAR");
Am_Slot_Key FAKE_DELETE = Am_Register_Slot_Name("FAKE_DELETE");

Am_Object Menu_Edit_Window, menu_proto, first_command;
Am_Object am_command_proto, am_selection_widget_all_proto, clear_proto,
    clear_all_proto, copy_proto, cut_proto, paste_proto, bottom_proto,
    top_proto, duplicate_proto, group_proto, ungroup_proto, undo_proto,
    redo_proto, quit_proto, open_proto, save_proto, save_as_proto,
    menu_line_proto, node_proto;

Am_Style Motif_Blue_Stipple, Am_Blue_2;

Am_Image_Array arrow_left(Am_Merge_Pathname("lib/images/arrow_left.xbm"));
Am_Image_Array arrow_right(Am_Merge_Pathname("lib/images/arrow_right.xbm"));
Am_Image_Array arrow_up(Am_Merge_Pathname("lib/images/arrow_up.xbm"));
Am_Image_Array arrow_down(Am_Merge_Pathname("lib/images/arrow_down.xbm"));

void
Copy_and_Create_List(Am_Value_List source_list, Am_Value_List &target_list)
{
  Am_Value_List children, new_children;
  Am_Object header, submenu, item;
  Am_Value v;

  for (source_list.Start(); !source_list.Last(); source_list.Next()) {
    new_children.Make_Empty();
    header = source_list.Get();
    children = header.Get(Lw_CHILDREN);
    for (children.Start(); !children.Last(); children.Next()) {
      submenu = ((Am_Object)children.Get()).Copy();
      v = submenu.Peek(Lw_VITEM);
      if (v.Valid()) {
        item = ((Am_Object)v).Copy();
        item.Set(Lw_PARENT, submenu);
        submenu.Set(Lw_VITEM, item);
      }
      v = submenu.Peek(Lw_BITMAP);
      if (v.Valid()) {
        item = ((Am_Object)v).Create();
        item.Set(Lw_PARENT, submenu);
        submenu.Set(Lw_BITMAP, item);
      }
      item = submenu.Get_Object(Lw_HITEM).Copy();
      item.Set(Lw_PARENT, submenu);
      submenu.Set(Lw_HITEM, item);
      new_children.Add(submenu);
    }
    header = header.Copy();
    header.Set(Lw_CHILDREN, new_children);
    target_list.Add(header);
  }
}

void
Restore_Menu_Tree(Am_Object group, Am_Value_List items)
{
  Am_Value v;
  Am_Object header;
  Am_Object submenu;
  Am_Value_List children;

  for (items.Start(); !items.Last(); items.Next()) {
    header = (Am_Object)items.Get();
    group.Add_Part(header);
    children = header.Get(Lw_CHILDREN);
    for (children.Start(); !children.Last(); children.Next()) {
      submenu = (Am_Object)children.Get();
      group.Add_Part(submenu);
      v = submenu.Peek(Lw_VITEM);
      if (v.Valid())
        group.Add_Part((Am_Object)v);
      v = submenu.Peek(Lw_BITMAP);
      if (v.Valid())
        group.Add_Part((Am_Object)v);
      group.Add_Part(submenu.Get_Object(Lw_HITEM));
    }
  }
}

Am_Object
create_menu(Am_Value_List l)
{
  Am_Value_List subtree;
  Am_Object node;
  int key;
  Am_Value_List items1, items2;
  Am_Object command1, command2;

  Am_Object menu = menu_proto.Create();

  for (l.Start(); !l.Last(); l.Next()) {
    items2.Make_Empty();
    node = l.Get();
    Am_String label = node.Get_Object(TEXT1).Get(Am_TEXT);
    key = (int)node.Get_Object(TEXT2).Get(COMMAND_KEY);
    command1 = (n2c[key]).Create();
    command1.Set(Am_LABEL, label);
    command1.Set(COMMAND_KEY, key);
    subtree = node.Get(Lw_CHILDREN);
    for (subtree.Start(); !subtree.Last(); subtree.Next()) {
      node = subtree.Get();
      Am_String label2 = node.Get_Object(TEXT1).Get(Am_TEXT);
      Am_String accelator = node.Get(Am_ACCELERATOR);
      key = (int)node.Get_Object(TEXT2).Get(COMMAND_KEY);
      command2 = (n2c[key]).Create();
      command2.Set(Am_LABEL, label2);
      command2.Set(COMMAND_KEY, key);

      if (accelator.Valid() && strcmp((char *)accelator, ""))
        command2.Set(Am_ACCELERATOR, (Am_String)accelator);
      items2.Add(command2);
    }
    command1.Set(Am_ITEMS, items2, Am_OK_IF_NOT_THERE);
    items1.Add(command1);
  }

  menu.Set(Am_ITEMS, items1);
  return menu;
}

int
destroy_nodes(Am_Value_List l)
{
  Am_Object obj;
  Am_Value_List children;
  Am_Value v;

  if (!l.Empty()) {
    l.Start();
    obj = (Am_Object)l.Get();
    cout << "Object being destroyed right now is :: " << obj << endl;
    v = obj.Peek(Lw_VITEM);
    if (v.Valid() && v != NULL)
      ((Am_Object)v).Destroy();
    v = obj.Peek(Lw_HITEM);
    if (v.Valid() && v != NULL)
      ((Am_Object)v).Destroy();
    v = obj.Peek(Lw_BITMAP);
    if (v.Valid() && v != NULL)
      ((Am_Object)v).Destroy();
    children = obj.Get(Lw_CHILDREN);
    if (!children.Empty())
      destroy_nodes(children);
    obj.Destroy();
    l.Delete();
    destroy_nodes(l);
  }
  return 0;
}

Am_Define_Method(Am_Object_Method, void, close_ok_menu_edit_window,
                 (Am_Object cmd))
{
  Am_Object window = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  Outline *ol = (Outline *)(Am_Ptr)window.Get(OUTLINE);
  Am_Value_List mt = ((Am_Object)ol->Get_Root()).Get(Lw_CHILDREN);
  Am_Object menu = create_menu(mt);

  Am_Value_List fake_items = window.Get(FAKE_DELETE);

  destroy_nodes(fake_items);

  menu.Set(Am_ACTIVE_2, run_tool);
  Am_Finish_Pop_Up_Waiting(window, (Am_Value)menu);
}

Am_Define_Method(Am_Object_Method, void, close_cancel_menu_edit_window,
                 (Am_Object cmd))
{
  Am_Object window = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  Am_Value_List fake_items = window.Get(FAKE_DELETE);

  destroy_nodes(fake_items);

  Am_Finish_Pop_Up_Waiting(window, (Am_Value)Am_No_Object);
}

Am_Define_Method(Am_Object_Method, void, change_command_key, (Am_Object cmd))
{
  Am_Object owner = cmd.Get_Owner().Get_Owner().Get_Owner();
  int key = cmd.Get_Owner().Get_Object(Am_VALUE).Get(Am_RANK);

  owner.Set(COMMAND_KEY, key);
}

Am_Define_Method(Am_Object_Method, void, add_new_submenu, (Am_Object cmd))
{
  Am_Object win = cmd.Get_Object(Am_SAVED_OLD_OWNER);
  int n;
  if (win.Is_Instance_Of(Am_One_Shot_Interactor))
    win =
        win.Get_Owner().Get_Owner().Get_Owner().Get_Sibling(NEW_DELETE_PANNEL);
  else if (win.Is_Instance_Of(Am_Menu_Bar))
    win = win.Get_Sibling(NEW_DELETE_PANNEL);
  win.Set(NUMBER_OF_NODES, n = ((int)win.Get(NUMBER_OF_NODES)) + 1);
  win = win.Get_Owner();

  Am_Object l_group = win.Get_Object(Lw_GROUP).Get_Object(Lw_GROUP);
  Am_Object ck = win.Get_Object(COMMAND_KEY);
  int key = ck.Get(COMMAND_KEY);
  Outline *ol = (Outline *)(Am_Ptr)win.Get(OUTLINE);
  Am_Object curr_obj = l_group.Get_Object(CHOICE_INTER).Get_Object(Am_VALUE);
  win = ol->Get_Root(); //win is now set to Root object
  int prev_n;
  Am_Value v;

  Am_Input_Char ic;
  char s[5];

  Am_Object node = node_proto.Create()
                       .Get_Object(TEXT1)
                       .Set(Am_TEXT, n2cl[key])
                       .Get_Owner()
                       .Get_Object(TEXT2)
                       .Set(Am_TEXT, n2cr[key])
                       .Set(COMMAND_KEY, key)
                       .Get_Owner();

  if (((Am_Object)n2c[key]).Get(Am_ACCELERATOR) == NULL) {
    node.Set(Am_ACCELERATOR, n2c[key].Get(Am_ACCELERATOR));
  } else {
    ic = (Am_Input_Char)n2c[key].Get(Am_ACCELERATOR);
    ic.As_Short_String(s);
    node.Set(Am_ACCELERATOR, (Am_String)s);
  }

  if (curr_obj != NULL) {
    prev_n = curr_obj.Get(Lw_NODEID);

    if ((curr_obj == win) || (curr_obj.Get_Object(Lw_PARENT) == win)) {
      l_group.Add_Part(ol->AddNode(n, prev_n, node));
      ck.Set(COMMAND_KEY, 0);
      ck.Set(Am_Y_OFFSET, 0);

      Am_Object ci = ck.Get_Object(COMMAND_LIST).Get_Object(CHOICE_INTER);
      ci.Get_Object(Am_VALUE).Set(Am_SELECTED, false);
      ci.Set(Am_VALUE, first_command);
      first_command.Set(Am_SELECTED, true);

      cmd.Set(Am_OBJECT_MODIFIED, node);
      cmd.Set(Am_VALUE, true);
      cmd.Set(Am_OLD_VALUE, false);
    } else
      Am_Show_Alert_Dialog((Am_String) "Submenu can not be added");
  } else {
    Am_Value_List l = win.Get(Lw_CHILDREN);
    l.End();
    Am_Object l_node = (Am_Object)l.Get();
    prev_n = l_node.Get(Lw_NODEID);
    l_group.Add_Part(ol->AddNode(n, prev_n, node));

    ck.Set(COMMAND_KEY, 0);
    ck.Set(Am_Y_OFFSET, 0);

    Am_Object ci = ck.Get_Object(COMMAND_LIST).Get_Object(CHOICE_INTER);
    ci.Get_Object(Am_VALUE).Set(Am_SELECTED, false);
    ci.Set(Am_VALUE, first_command);
    first_command.Set(Am_SELECTED, true);

    cmd.Set(Am_OBJECT_MODIFIED, node);
    cmd.Set(Am_VALUE, true);
    cmd.Set(Am_OLD_VALUE, false);
  }
}

Am_Define_Method(Am_Object_Method, void, undo_add_submenu, (Am_Object cmd))
{
  Am_Object node = cmd.Get_Object(Am_OBJECT_MODIFIED);
  Am_Object parent = node.Get_Object(Lw_PARENT);
  Am_Value_List children = parent.Get(Lw_CHILDREN);
  bool exists = cmd.Get(Am_VALUE);
  Am_Object win = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  Outline *ol = (Outline *)(Am_Ptr)win.Get(OUTLINE);
  Am_Value_List fake_items = win.Get(FAKE_DELETE);
  Am_Object lw_group = win.Get_Object(Lw_GROUP).Get_Object(Lw_GROUP);

  Am_Value v;

  if (exists) {
    children.End();
    children.Delete(); // New node is always added at the end
    parent.Set(Lw_CHILDREN, children);
    ol->RemoveBm(parent);
    parent.Note_Changed(Lw_CHILDREN);
    lw_group.Remove_Part(node);
    v = node.Peek(Lw_HITEM);
    if (v.Valid() && v != NULL)
      lw_group.Remove_Part((Am_Object)v);
    v = node.Peek(Lw_BITMAP);
    if (v.Valid() && v != NULL)
      lw_group.Remove_Part((Am_Object)v);

    fake_items.Add(node);
    win.Set(FAKE_DELETE, fake_items);
    cmd.Set(Am_OBJECT_MODIFIED, node);
    cmd.Set(Am_VALUE, false);
    cmd.Set(Am_OLD_VALUE, true);
  } else {
    fake_items.Start();
    fake_items.Member(node);
    fake_items.Delete();

    children.Add(node);
    parent.Set(Lw_CHILDREN, children);
    ol->AddBmVi(parent);
    lw_group.Add_Part(node);
    v = node.Peek(Lw_HITEM);
    if (v.Valid() && v != NULL)
      lw_group.Add_Part((Am_Object)v);
    v = node.Peek(Lw_VITEM);
    if (v.Valid() && v != NULL)
      lw_group.Add_Part((Am_Object)v);
    v = node.Peek(Lw_BITMAP);
    if (v.Valid() && v != NULL)
      lw_group.Add_Part((Am_Object)v);
    parent.Note_Changed(Lw_CHILDREN);

    win.Set(FAKE_DELETE, fake_items);
    cmd.Set(Am_OBJECT_MODIFIED, node);
    cmd.Set(Am_VALUE, true);
    cmd.Set(Am_OLD_VALUE, false);
  }
}

Am_Define_Method(Am_Object_Method, void, delete_node, (Am_Object cmd))
{
  Am_Object win = cmd.Get_Object(Am_SAVED_OLD_OWNER);
  win = win.Get_Owner();
  Am_Object l_group = win.Get_Object(Lw_GROUP).Get_Object(Lw_GROUP);
  Outline *ol = (Outline *)(Am_Ptr)win.Get(OUTLINE);
  Am_Object curr_obj = l_group.Get_Object(CHOICE_INTER).Get_Object(Am_VALUE);
  Am_Value_List children;

  Am_Value v;
  Am_Object obj;

  if (!curr_obj.Valid())
    Am_Show_Alert_Dialog("No node is currently selected");
  else {
    if (ol->Get_Root() == curr_obj)
      Am_Show_Alert_Dialog("Menubar can not be deleted");
    else {
      Am_Object parent = curr_obj.Get_Object(Lw_PARENT);
      Am_Value_List children = parent.Get(Lw_CHILDREN);
      Am_Value_List fake_delete = win.Get(FAKE_DELETE);
      int nth = 0;
      Am_Object ci = l_group.Get_Object(CHOICE_INTER);

      for (children.Start();
           (!children.Last() && (Am_Object)children.Get() != curr_obj);
           children.Next())
        nth++;
      //Always have one node selected.
      ci.Set(Am_VALUE, (nth > 0) ? (Am_Object)children.Get_Nth(nth - 1)
                                 : curr_obj.Get_Object(Lw_PARENT));

      children.Delete();
      parent.Set(Lw_CHILDREN, children);
      ol->RemoveBm(parent);
      parent.Note_Changed(Lw_CHILDREN);
      l_group.Remove_Part(curr_obj);

      v = curr_obj.Peek(Lw_HITEM);
      if (v.Valid() && v != NULL)
        l_group.Remove_Part((Am_Object)v);
      v = curr_obj.Peek(Lw_VITEM);
      if (v.Valid() && v != NULL)
        l_group.Remove_Part((Am_Object)v);
      v = curr_obj.Peek(Lw_BITMAP);
      if (v.Valid() && v != NULL)
        l_group.Remove_Part((Am_Object)v);

      children = curr_obj.Get(Lw_CHILDREN);
      for (children.Start(); !children.Last(); children.Next()) {
        obj = (Am_Object)children.Get();
        v = obj.Peek(Lw_HITEM);
        if (v.Valid())
          l_group.Remove_Part((Am_Object)v);
        v = obj.Peek(Lw_VITEM);
        if (v.Valid())
          l_group.Remove_Part((Am_Object)v);
        l_group.Remove_Part(obj);
      }

      win.Set(FAKE_DELETE, fake_delete.Add(curr_obj));

      cmd.Set(Am_VALUE, -1);
      cmd.Set(Am_OLD_VALUE, nth);
      cout << "JUst deleted " << nth << "th item\n";
      cmd.Set(Am_OBJECT_MODIFIED, curr_obj);
    }
  }
}

Am_Define_Method(Am_Object_Method, void, undo_delete_node, (Am_Object cmd))
{
  Am_Object win = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  Am_Object l_group = win.Get_Object(Lw_GROUP).Get_Object(Lw_GROUP);
  Outline *ol = (Outline *)(Am_Ptr)win.Get(OUTLINE);
  Am_Object node = cmd.Get_Object(Am_OBJECT_MODIFIED);
  Am_Object parent = node.Get_Object(Lw_PARENT);
  Am_Value_List children = parent.Get(Lw_CHILDREN);
  Am_Value_List fake_delete = win.Get(FAKE_DELETE);
  int nth = cmd.Get(Am_OLD_VALUE);
  Am_Value v;
  Am_Object obj;

  if (nth > -1) {
    cout << "Trying to undelete " << nth << "th item\n";
    if (nth == 0)
      children.Add(node, Am_HEAD);
    else {
      children.Start();
      while (nth > 1) {
        children.Next();
        nth--;
      }
      children.Insert(node, Am_AFTER);
    }
    parent.Set(Lw_CHILDREN, children);
    ol->AddBmVi(parent);
    parent.Note_Changed(Lw_CHILDREN);
    l_group.Add_Part(node);

    v = node.Peek(Lw_HITEM);
    if (v.Valid() && v != NULL)
      l_group.Add_Part((Am_Object)v);
    v = node.Peek(Lw_VITEM);
    if (v.Valid() && v != NULL)
      l_group.Add_Part((Am_Object)v);
    v = node.Peek(Lw_BITMAP);
    if (v.Valid() && v != NULL)
      l_group.Add_Part((Am_Object)v);

    children = node.Get(Lw_CHILDREN);
    for (children.Start(); !children.Last(); children.Next()) {
      obj = (Am_Object)children.Get();
      v = obj.Peek(Lw_HITEM);
      if (v.Valid())
        l_group.Add_Part((Am_Object)v);
      v = obj.Peek(Lw_VITEM);
      if (v.Valid())
        l_group.Add_Part((Am_Object)v);
      l_group.Add_Part(obj);
    }

    fake_delete.Start();
    fake_delete.Member(node);
    fake_delete.Delete();
    win.Set(FAKE_DELETE, fake_delete);

    cmd.Set(Am_OBJECT_MODIFIED, node);
    cmd.Set(Am_VALUE, nth);
    cmd.Set(Am_OLD_VALUE, -1);
  } else {
    nth = (int)cmd.Get(Am_VALUE);
    children.Get_Nth(nth);
    children.Delete();

    parent.Set(Lw_CHILDREN, children);
    ol->RemoveBm(parent);
    parent.Note_Changed(Lw_CHILDREN);
    l_group.Remove_Part(node);

    v = node.Peek(Lw_HITEM);
    if (v.Valid() && v != NULL)
      l_group.Add_Part((Am_Object)v);
    v = node.Peek(Lw_VITEM);
    if (v.Valid() && v != NULL)
      l_group.Add_Part((Am_Object)v);
    v = node.Peek(Lw_BITMAP);
    if (v.Valid() && v != NULL)
      l_group.Add_Part((Am_Object)v);

    children = node.Get(Lw_CHILDREN);
    for (children.Start(); !children.Last(); children.Next()) {
      obj = (Am_Object)children.Get();
      v = obj.Peek(Lw_HITEM);
      if (v.Valid())
        l_group.Remove_Part((Am_Object)v);
      v = obj.Peek(Lw_VITEM);
      if (v.Valid())
        l_group.Remove_Part((Am_Object)v);
      l_group.Remove_Part(obj);
    }

    win.Set(FAKE_DELETE, fake_delete.Add(node));

    cmd.Set(Am_VALUE, -1);
    cmd.Set(Am_OLD_VALUE, (int)cmd.Get(Am_VALUE));
    cmd.Set(Am_OBJECT_MODIFIED, node);
  }
  //always have one node selected
  Am_Object ci = l_group.Get_Object(CHOICE_INTER);
  ci.Set(Am_VALUE, node);
}

Am_Define_Method(Am_Object_Method, void, make_it_upper_level, (Am_Object cmd))
{
  Am_Object window = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  Outline *ol = (Outline *)(Am_Ptr)window.Get(OUTLINE);
  Am_Object obj = window.Get_Object(Lw_GROUP)
                      .Get_Object(Lw_GROUP)
                      .Get_Object(CHOICE_INTER)
                      .Get_Object(Am_VALUE);
  Am_Object pr = obj.Get_Object(Lw_PARENT);
  Am_Object root = ol->Get_Root();

  if (obj == Am_No_Object)
    Am_Show_Alert_Dialog("No node is currently selected");
  else if (root == obj)
    Am_Show_Alert_Dialog("Already at the top level");
  else if (pr == root)
    Am_Show_Alert_Dialog("There can be only one menubar in a window");
  else {
    Am_Value_List l = pr.Get(Lw_CHILDREN);
    Am_Value_List l2 = root.Get(Lw_CHILDREN);
    Am_Object ch;

    cmd.Set(Am_OLD_VALUE, Am_Value_List().Add(l).Add(l2).Add(Am_Value_List()));
    cmd.Set(Am_OBJECT_MODIFIED, Am_Value_List().Add(pr).Add(root).Add(obj));

    l.Start();
    l.Member(obj);
    l.Delete();
    l.Next();

    l2.Start();
    l2.Member(pr);

    Am_Value_List l3;
    while (!l.Last()) {
      ch = l.Get();
      ch.Set(Lw_PARENT, obj);
      l3.Add(ch);
      l.Delete();
      l.Next();
    }
    pr.Set(Lw_CHILDREN, l);
    ol->AddBmVi(pr);
    obj.Set(Lw_CHILDREN, l3);
    obj.Set(Lw_PARENT, root);
    ol->AddBmVi(obj);
    l2.Insert(obj, Am_AFTER);
    root.Set(Lw_CHILDREN, l2);
    cmd.Set(Am_VALUE, Am_Value_List().Add(l).Add(l2).Add(l3));

    obj.Note_Changed(Lw_CHILDREN);
    pr.Note_Changed(Lw_CHILDREN);
    root.Note_Changed(Lw_CHILDREN);
  }
}

Am_Define_Method(Am_Object_Method, void, make_it_lower_level, (Am_Object cmd))
{
  Am_Object window = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  Outline *ol = (Outline *)(Am_Ptr)window.Get(OUTLINE);
  Am_Object obj = window.Get_Object(Lw_GROUP)
                      .Get_Object(Lw_GROUP)
                      .Get_Object(CHOICE_INTER)
                      .Get_Object(Am_VALUE);
  Am_Object pr = obj.Get_Object(Lw_PARENT);
  Am_Object root = ol->Get_Root();
  Am_Value_List old_value, new_value, obj_modified;

  if (obj == Am_No_Object)
    Am_Show_Alert_Dialog("No node is currently selected");
  else if (root == obj)
    Am_Show_Alert_Dialog("Top level can't be converted to submenu");
  else if (pr != root)
    Am_Show_Alert_Dialog("Already at the lowest level");
  else {
    Am_Value_List l = pr.Get(Lw_CHILDREN), l2;
    Am_Object new_p;
    Am_Value_List new_ch;
    Am_Object ch;

    l.Start();

    if ((Am_Object)l.Get() == obj)
      Am_Show_Alert_Dialog(
          "Can't be made submenu because new parent can't be found");
    else {
      old_value.Add(l);
      l.Next();
      l.Member(obj);
      l.Delete();
      new_ch = obj.Get(Lw_CHILDREN);

      new_p = l.Get();
      obj_modified.Add(pr).Add(new_p).Add(obj);
      obj.Set(Lw_CHILDREN, Am_Value_List());
      obj.Set(Lw_PARENT, new_p);
      for (new_ch.Start(); !new_ch.Last(); new_ch.Next()) {
        ch = (Am_Object)new_ch.Get();
        ch.Set(Lw_PARENT, new_p);
      }
      l2 = new_p.Get(Lw_CHILDREN);
      old_value.Add(l2).Add(new_ch);

      obj.Note_Changed(Lw_CHILDREN);
      l2.Add(obj);
      new_p.Set(Lw_CHILDREN, l2.Append(new_ch));
      ol->AddBmVi(new_p);
      new_p.Note_Changed(Lw_CHILDREN);
      pr.Set(Lw_CHILDREN, l);
      pr.Note_Changed(Lw_CHILDREN);
      new_value.Add(l).Add(new_p.Get(Lw_CHILDREN)).Add(Am_Value_List());

      cmd.Set(Am_VALUE, new_value);
      cmd.Set(Am_OLD_VALUE, old_value);
      cmd.Set(Am_OBJECT_MODIFIED, obj_modified);
    }
  }
}

Am_Define_Method(Am_Object_Method, void, go_up_menu, (Am_Object cmd))
{
  Am_Object window = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  Outline *ol = (Outline *)(Am_Ptr)window.Get(OUTLINE);
  Am_Object obj = window.Get_Object(Lw_GROUP)
                      .Get_Object(Lw_GROUP)
                      .Get_Object(CHOICE_INTER)
                      .Get_Object(Am_VALUE);
  Am_Object pr = obj.Get_Object(Lw_PARENT);
  Am_Object root = ol->Get_Root();
  Am_Value_List old_value, new_value, obj_modified;

  Am_Value_List l1;

  if (obj != root) {
    l1 = pr.Get(Lw_CHILDREN);
    old_value.Add(l1);
    obj_modified.Add(pr);
    l1.Start();
    if (l1.Get() == obj) {
      if (pr != root) {
        Am_Value_List l2;

        l2 = pr.Get_Object(Lw_PARENT).Get(Lw_CHILDREN);
        l2.Start();

        if ((Am_Object)l2.Get() == pr)
          Am_Show_Alert_Dialog("At the top of the list");
        else {
          Am_Object tmp;

          l1.Delete();
          pr.Set(Lw_CHILDREN, l1);
          new_value.Add(l1);
          l2.Start();
          l2.Member(pr);
          l2.Prev();
          tmp = (Am_Object)l2.Get();
          obj_modified.Add(tmp).Add(obj);
          l2 = tmp.Get(Lw_CHILDREN);
          old_value.Add(l2).Add(Am_Value_List());
          obj.Set(Lw_PARENT, tmp);
          l2.Add(obj);
          tmp.Set(Lw_CHILDREN, l2);
          new_value.Add(l2).Add(Am_Value_List());
          ol->AddBmVi(tmp);
          tmp.Note_Changed(Lw_CHILDREN);
          pr.Note_Changed(Lw_CHILDREN);
          root.Note_Changed(Lw_CHILDREN);
        }
      } else
        Am_Show_Alert_Dialog("At the top of the list");
    } else {
      l1.Member(obj);
      l1.Delete();
      l1.Insert(obj, Am_BEFORE);
      pr.Set(Lw_CHILDREN, l1);
      new_value.Add(l1);
      pr.Note_Changed(Lw_CHILDREN);
      root.Note_Changed(Lw_CHILDREN);
    }
    cmd.Set(Am_VALUE, new_value);
    cmd.Set(Am_OLD_VALUE, old_value);
    cmd.Set(Am_OBJECT_MODIFIED, obj_modified);
  } else
    Am_Show_Alert_Dialog("Menubar can not be moved");
}

Am_Define_Method(Am_Object_Method, void, go_down_menu, (Am_Object cmd))
{
  Am_Object window = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Owner();
  Outline *ol = (Outline *)(Am_Ptr)window.Get(OUTLINE);
  Am_Object obj = window.Get_Object(Lw_GROUP)
                      .Get_Object(Lw_GROUP)
                      .Get_Object(CHOICE_INTER)
                      .Get_Object(Am_VALUE);
  Am_Object pr = obj.Get_Object(Lw_PARENT);
  Am_Object root = ol->Get_Root();
  Am_Value_List old_value, new_value, obj_modified;
  Am_Value_List l1;

  if (obj != root) {
    l1 = pr.Get(Lw_CHILDREN);
    old_value.Add(l1);
    obj_modified.Add(pr);
    l1.End();
    if (l1.Get() == obj) {
      if (pr != root) {
        Am_Value_List l2;

        l2 = pr.Get_Object(Lw_PARENT).Get(Lw_CHILDREN);
        l2.End();
        if ((Am_Object)l2.Get() == pr)
          Am_Show_Alert_Dialog("At the bottom of the list");
        else {
          Am_Object tmp;

          l1.Delete();
          pr.Set(Lw_CHILDREN, l1);
          new_value.Add(l1);
          l2.Start();
          l2.Member(pr);
          l2.Next();
          tmp = (Am_Object)l2.Get();

          l2 = tmp.Get(Lw_CHILDREN);
          old_value.Add(l2).Add(Am_Value_List());
          obj_modified.Add(tmp).Add(obj);
          obj.Set(Lw_PARENT, tmp);
          l2.Add(obj, Am_HEAD);
          tmp.Set(Lw_CHILDREN, l2);
          new_value.Add(l2).Add(Am_Value_List());
          ol->AddBmVi(tmp);

          tmp.Note_Changed(Lw_CHILDREN);
          pr.Note_Changed(Lw_CHILDREN);
          root.Note_Changed(Lw_CHILDREN);
        }
      } else
        Am_Show_Alert_Dialog("At the bottom of the list");
    } else {
      l1.Start();
      l1.Member(obj);
      l1.Delete();
      l1.Next();
      l1.Insert(obj, Am_AFTER);
      pr.Set(Lw_CHILDREN, l1);
      new_value.Add(l1);
      pr.Note_Changed(Lw_CHILDREN);
      root.Note_Changed(Lw_CHILDREN);
    }
    cmd.Set(Am_VALUE, new_value);
    cmd.Set(Am_OLD_VALUE, old_value);
    cmd.Set(Am_OBJECT_MODIFIED, obj_modified);
  } else
    Am_Show_Alert_Dialog("Menubar can not be moved");
}

Am_Define_Method(Am_Object_Method, void, undo_move_node, (Am_Object cmd))
{
  Am_Value_List old_value = cmd.Get(Am_OLD_VALUE);
  Am_Value_List new_value = cmd.Get(Am_VALUE);
  Am_Value_List obj_list = cmd.Get(Am_OBJECT_MODIFIED);
  Am_Object obj, first_obj;
  Am_Value_List objects, values;
  Outline *ol = (Outline *)(Am_Ptr)cmd.Get_Object(Am_SAVED_OLD_OWNER)
                    .Get_Owner()
                    .Get(OUTLINE);

  first_obj = (Am_Object)obj_list.Get_Nth(0);
  cmd.Set(Am_VALUE, old_value);
  cmd.Set(Am_OLD_VALUE, new_value);

  for (obj_list.Start(), old_value.Start(), new_value.Start(); !obj_list.Last();
       obj_list.Next(), old_value.Next(), new_value.Next()) {

    obj = (Am_Object)obj_list.Get();
    obj.Set(Lw_CHILDREN, old_value.Get());
  }

  if (obj_list.Length() > 1) {
    obj.Set(Lw_PARENT, first_obj);
    ol->RemoveBm((Am_Object)obj_list.Get_Nth(1));
  }
}

Am_Define_Method(Am_Text_Edit_Method, void, gilt_single_char_method,
                 (Am_Object text, Am_Input_Char ic, Am_Object inter))
{
  if (ic.click_count == Am_NOT_MOUSE) {
    char s[Am_LONGEST_CHAR_STRING];
    ic.As_Short_String(s);
    text.Set(Am_TEXT, s);
    text.Set(Am_CURSOR_INDEX, 0);
    Am_Stop_Interactor(inter);
  }
}

Am_Define_Method(Am_Object_Method, void, change_accelerator, (Am_Object cmd))
{
  Am_Object ci = cmd.Get_Object(Am_SAVED_OLD_OWNER)
                     .Get_Owner()
                     .Get_Object(Lw_GROUP)
                     .Get_Object(Lw_GROUP)
                     .Get_Object(CHOICE_INTER);
  Am_Object node = ci.Get_Object(Am_VALUE);

  cmd.Set(Am_OLD_VALUE, node.Get(Am_ACCELERATOR));
  node.Set(Am_ACCELERATOR, cmd.Get_Object(Am_SAVED_OLD_OWNER).Get(Am_VALUE));
  cmd.Set(Am_OBJECT_MODIFIED, node);
  cmd.Set(Am_VALUE, node.Get(Am_ACCELERATOR));
}

Am_Define_Method(Am_Object_Method, void, undo_change_accelerator,
                 (Am_Object cmd))
{
  Am_Object node = cmd.Get(Am_OBJECT_MODIFIED);
  Am_String accelerator = cmd.Get(Am_OLD_VALUE);

  node.Set(Am_ACCELERATOR, accelerator);
  cmd.Set(Am_OBJECT_MODIFIED, node);
  cmd.Set(Am_OLD_VALUE, (Am_String)cmd.Get(Am_VALUE));
  cmd.Set(Am_VALUE, accelerator);
}

Am_Define_Method(Am_Object_Method, void, display_accelerator, (Am_Object cmd))
{
  Am_Object input = cmd.Get_Object(Am_SAVED_OLD_OWNER)
                        .Get_Owner()
                        .Get_Owner()
                        .Get_Owner()
                        .Get_Object(ACCELERATOR_INPUT);
  Am_Object item = cmd.Get_Object(Am_SAVED_OLD_OWNER).Get_Object(Am_VALUE);

  input.Set(Am_VALUE, (Am_String)item.Get(Am_ACCELERATOR));
}

/************************** FORMULA *************************************/

Am_Define_Style_Formula(compute_line_style)
{
  bool b = self.Get_Owner().Get(Am_SELECTED);

  if (b)
    return Am_Blue_2;
  else
    return Am_Line_0;
}

Am_Define_Formula(bool, set_invert)
{
  if ((bool)self.Get(Am_SELECTED))
    return true;
  else
    return false;
}

Am_Define_Formula(int, get_width)
{
  Am_Object obj = self.Get_Object(Am_CURRENT_OBJECT);
  return obj.Get(Am_WIDTH);
}

Am_Define_Formula(int, get_height)
{
  Am_Object obj = self.Get_Object(Am_CURRENT_OBJECT);
  return obj.Get(Am_HEIGHT);
}

Am_Define_String_Formula(get_text)
{
  int i = self.Get(Am_RANK);
  if (i < 0)
    return (Am_String) "";
  else if (i == 0) {
    first_command = self;
    self.Set(Am_SELECTED, true);
  }
  return (Am_String)n2cr[i];
}

Am_Define_Formula(int, compute_right_width)
{
  Am_Object win = self.Get_Owner();
  Am_Value panel = win.Peek(NEW_DELETE_PANNEL);
  if (panel.Valid()) {
    int width = (int)win.Get(Am_WIDTH) - (int)((Am_Object)panel).Get(Am_WIDTH);
    return width - 15;
  } else
    return 300;
}

Am_Define_Formula(int, compute_left_coord)
{
  Am_Value v = self.Peek(Am_WIDTH);
  if (v.Valid())
    return (int)self.Get_Owner().Get(Am_WIDTH) - (int)v - 5;
  else
    return 150;
}

Am_Object
Menuedit_Window_Initialize()
{

  /******************************************************************
     This window is a dialog box for menu edit window
     ****************************************************************/
  float r, g, b;
  Am_Motif_Blue.Get_Values(r, g, b);
  Motif_Blue_Stipple = Am_Style::Am_Style(
      r, g, b, 1, Am_CAP_BUTT, Am_JOIN_MITER, Am_LINE_SOLID,
      Am_DEFAULT_DASH_LIST, Am_DEFAULT_DASH_LIST_LENGTH, Am_FILL_STIPPLED,
      Am_FILL_POLY_EVEN_ODD, (Am_Image_Array(30)));

  Am_Blue_2 =
      Am_Style::Am_Style("blue", 2, Am_CAP_BUTT, Am_JOIN_MITER, Am_LINE_SOLID,
                         Am_DEFAULT_DASH_LIST, Am_DEFAULT_DASH_LIST_LENGTH,
                         Am_FILL_SOLID, Am_FILL_POLY_EVEN_ODD, Am_No_Image);

  n2c[0] = am_command_proto =
      Am_Command.Create()
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
          .Add(TYPE_NAME, "Am_Command")
          .Add(SAVE_MENU_BAR, true)
          .Add(COMMAND_KEY, 0)
          .Set(Am_ACTIVE, false)
          .Add(Am_ACTIVE_2, false)
          .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                     .Add(Am_LABEL)
                                     .Add(COMMAND_KEY)
                                     .Add(Am_ITEMS)
                                     .Add(Am_ACCELERATOR)
                                     .Add(Am_ACTIVE)
                                     .Add(Am_ACTIVE_2));
  n2c[1] = am_selection_widget_all_proto =
      Am_Selection_Widget_Select_All_Command.Create()
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
          .Add(TYPE_NAME, "Am_Selection_Widget_All_Command")
          .Add(SAVE_MENU_BAR, true)
          .Add(COMMAND_KEY, 1)
          .Set(Am_ACTIVE, false)
          .Add(Am_ACTIVE_2, false)
          .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                     .Add(Am_LABEL)
                                     .Add(COMMAND_KEY)
                                     .Add(Am_ITEMS)
                                     .Add(Am_ACCELERATOR)
                                     .Add(Am_ACTIVE)
                                     .Add(Am_ACTIVE_2));
  n2c[2] = clear_proto =
      Am_Graphics_Clear_Command.Create()
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
          .Add(TYPE_NAME, "Am_Graphics_Clear_Command")
          .Add(SAVE_MENU_BAR, true)
          .Add(COMMAND_KEY, 2)
          .Set(Am_ACTIVE, false)
          .Add(Am_ACTIVE_2, false)
          .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                     .Add(Am_LABEL)
                                     .Add(COMMAND_KEY)
                                     .Add(Am_ITEMS)
                                     .Add(Am_ACCELERATOR)
                                     .Add(Am_ACTIVE)
                                     .Add(Am_ACTIVE_2));
  n2c[3] = clear_all_proto =
      Am_Graphics_Clear_All_Command.Create()
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
          .Add(TYPE_NAME, "Am_Graphics_Clear_All_Command")
          .Add(SAVE_MENU_BAR, true)
          .Add(COMMAND_KEY, 3)
          .Set(Am_ACTIVE, false)
          .Add(Am_ACTIVE_2, false)
          .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                     .Add(Am_LABEL)
                                     .Add(COMMAND_KEY)
                                     .Add(Am_ITEMS)
                                     .Add(Am_ACCELERATOR)
                                     .Add(Am_ACTIVE)
                                     .Add(Am_ACTIVE_2));
  n2c[4] = copy_proto = Am_Graphics_Copy_Command.Create()
                            .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
                            .Add(TYPE_NAME, "Am_Graphics_Copy_Command")
                            .Add(SAVE_MENU_BAR, true)
                            .Add(COMMAND_KEY, 4)
                            .Set(Am_ACTIVE, false)
                            .Add(Am_ACTIVE_2, false)
                            .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                                       .Add(Am_LABEL)
                                                       .Add(COMMAND_KEY)
                                                       .Add(Am_ITEMS)
                                                       .Add(Am_ACCELERATOR)
                                                       .Add(Am_ACTIVE)
                                                       .Add(Am_ACTIVE_2));
  n2c[5] = cut_proto = Am_Graphics_Cut_Command.Create()
                           .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
                           .Add(TYPE_NAME, "Am_Graphics_Cut_Command")
                           .Add(SAVE_MENU_BAR, true)
                           .Add(COMMAND_KEY, 5)
                           .Set(Am_ACTIVE, false)
                           .Add(Am_ACTIVE_2, false)
                           .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                                      .Add(Am_LABEL)
                                                      .Add(COMMAND_KEY)
                                                      .Add(Am_ITEMS)
                                                      .Add(Am_ACCELERATOR)
                                                      .Add(Am_ACTIVE)
                                                      .Add(Am_ACTIVE_2));
  n2c[6] = paste_proto =
      Am_Graphics_Paste_Command.Create()
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
          .Add(TYPE_NAME, "Am_Graphics_Paste_Command")
          .Add(SAVE_MENU_BAR, true)
          .Add(COMMAND_KEY, 6)
          .Set(Am_ACTIVE, false)
          .Add(Am_ACTIVE_2, false)
          .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                     .Add(Am_LABEL)
                                     .Add(COMMAND_KEY)
                                     .Add(Am_ITEMS)
                                     .Add(Am_ACCELERATOR)
                                     .Add(Am_ACTIVE)
                                     .Add(Am_ACTIVE_2));
  n2c[7] = bottom_proto =
      Am_Graphics_To_Bottom_Command.Create()
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
          .Add(TYPE_NAME, "Am_Graphics_To_Bottom_Command")
          .Add(SAVE_MENU_BAR, true)
          .Add(COMMAND_KEY, 7)
          .Set(Am_ACTIVE, false)
          .Add(Am_ACTIVE_2, false)
          .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                     .Add(Am_LABEL)
                                     .Add(COMMAND_KEY)
                                     .Add(Am_ITEMS)
                                     .Add(Am_ACCELERATOR)
                                     .Add(Am_ACTIVE)
                                     .Add(Am_ACTIVE_2));
  n2c[8] = top_proto = Am_Graphics_To_Top_Command.Create()
                           .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
                           .Add(TYPE_NAME, "Am_Graphics_To_Top_Command")
                           .Add(SAVE_MENU_BAR, true)
                           .Add(COMMAND_KEY, 8)
                           .Set(Am_ACTIVE, false)
                           .Add(Am_ACTIVE_2, false)
                           .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                                      .Add(Am_LABEL)
                                                      .Add(COMMAND_KEY)
                                                      .Add(Am_ITEMS)
                                                      .Add(Am_ACCELERATOR)
                                                      .Add(Am_ACTIVE)
                                                      .Add(Am_ACTIVE_2));
  n2c[9] = duplicate_proto =
      Am_Graphics_Duplicate_Command.Create()
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
          .Add(TYPE_NAME, "Am_Graphics_Duplicate_Command")
          .Add(SAVE_MENU_BAR, true)
          .Add(COMMAND_KEY, 9)
          .Set(Am_ACTIVE, false)
          .Add(Am_ACTIVE_2, false)
          .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                     .Add(Am_LABEL)
                                     .Add(COMMAND_KEY)
                                     .Add(Am_ITEMS)
                                     .Add(Am_ACCELERATOR)
                                     .Add(Am_ACTIVE)
                                     .Add(Am_ACTIVE_2));
  n2c[10] = group_proto =
      Am_Graphics_Group_Command.Create()
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
          .Add(TYPE_NAME, "Am_Graphics_Group")
          .Add(SAVE_MENU_BAR, true)
          .Add(COMMAND_KEY, 10)
          .Set(Am_ACTIVE, false)
          .Add(Am_ACTIVE_2, false)
          .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                     .Add(Am_LABEL)
                                     .Add(COMMAND_KEY)
                                     .Add(Am_ITEMS)
                                     .Add(Am_ACCELERATOR)
                                     .Add(Am_ACTIVE)
                                     .Add(Am_ACTIVE_2));
  n2c[11] = ungroup_proto =
      Am_Graphics_Ungroup_Command.Create()
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
          .Add(TYPE_NAME, "Am_Graphics_Ungroup_Command")
          .Add(SAVE_MENU_BAR, true)
          .Add(COMMAND_KEY, 11)
          .Set(Am_ACTIVE, false)
          .Add(Am_ACTIVE_2, false)
          .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                     .Add(Am_LABEL)
                                     .Add(COMMAND_KEY)
                                     .Add(Am_ITEMS)
                                     .Add(Am_ACCELERATOR)
                                     .Add(Am_ACTIVE)
                                     .Add(Am_ACTIVE_2));
  n2c[12] = undo_proto =
      Am_Undo_Command.Create()
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
          .Add(TYPE_NAME, "Am_Undo_Command")
          .Add(SAVE_MENU_BAR, true)
          .Add(COMMAND_KEY, 12)
          .Set(Am_ACTIVE, false)
          .Add(Am_ACTIVE_2, false)
          .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                     .Add(Am_LABEL)
                                     .Add(COMMAND_KEY)
                                     .Add(Am_ITEMS)
                                     .Add(Am_ACCELERATOR)
                                     .Add(Am_ACTIVE)
                                     .Add(Am_ACTIVE_2));
  n2c[13] = redo_proto =
      Am_Redo_Command.Create()
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
          .Add(TYPE_NAME, "Am_Redo_Command")
          .Add(SAVE_MENU_BAR, true)
          .Add(COMMAND_KEY, 13)
          .Set(Am_ACTIVE, false)
          .Add(Am_ACTIVE_2, false)
          .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                     .Add(Am_LABEL)
                                     .Add(COMMAND_KEY)
                                     .Add(Am_ITEMS)
                                     .Add(Am_ACCELERATOR)
                                     .Add(Am_ACTIVE)
                                     .Add(Am_ACTIVE_2));
  n2c[14] = quit_proto =
      Am_Quit_No_Ask_Command.Create()
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
          .Add(TYPE_NAME, "Am_Quit_No_Ask_Command")
          .Add(SAVE_MENU_BAR, true)
          .Add(COMMAND_KEY, 14)
          .Add(Am_ACTIVE_2, false)
          .Set(Am_ACTIVE, false)
          .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                     .Add(Am_LABEL)
                                     .Add(COMMAND_KEY)
                                     .Add(Am_ITEMS)
                                     .Add(Am_ACCELERATOR)
                                     .Add(Am_ACTIVE_2)
                                     .Add(Am_ACTIVE));
  n2c[15] = open_proto =
      Am_Open_Command.Create()
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
          .Add(TYPE_NAME, "Am_Open_Command")
          .Add(SAVE_MENU_BAR, true)
          .Add(COMMAND_KEY, 15)
          .Set(Am_ACTIVE, false)
          .Add(Am_ACTIVE_2, false)
          .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                     .Add(Am_LABEL)
                                     .Add(COMMAND_KEY)
                                     .Add(Am_ITEMS)
                                     .Add(Am_ACCELERATOR)
                                     .Add(Am_ACTIVE)
                                     .Add(Am_ACTIVE_2));
  n2c[16] = save_proto =
      Am_Save_Command.Create()
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
          .Add(TYPE_NAME, "Am_Save_Command")
          .Add(SAVE_MENU_BAR, true)
          .Add(COMMAND_KEY, 16)
          .Set(Am_ACTIVE, false)
          .Add(Am_ACTIVE_2, false)
          .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                     .Add(Am_LABEL)
                                     .Add(COMMAND_KEY)
                                     .Add(Am_ITEMS)
                                     .Add(Am_ACCELERATOR)
                                     .Add(Am_ACTIVE)
                                     .Add(Am_ACTIVE_2));
  n2c[17] = save_as_proto =
      Am_Save_As_Command.Create()
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
          .Add(TYPE_NAME, "Am_Save_As_Command")
          .Add(SAVE_MENU_BAR, true)
          .Add(COMMAND_KEY, 17)
          .Set(Am_ACTIVE, false)
          .Add(Am_ACTIVE_2, false)
          .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                     .Add(Am_LABEL)
                                     .Add(COMMAND_KEY)
                                     .Add(Am_ITEMS)
                                     .Add(Am_ACCELERATOR)
                                     .Add(Am_ACTIVE)
                                     .Add(Am_ACTIVE_2));
  n2c[18] = menu_line_proto =
      Am_Menu_Line_Command.Create()
          .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
          .Add(TYPE_NAME, "Am_Meu_Line_Command")
          .Add(SAVE_MENU_BAR, true)
          .Add(COMMAND_KEY, 18)
          .Set(Am_ACTIVE, false)
          .Add(Am_ACTIVE_2, false)
          .Add(Am_SLOTS_TO_SAVE, Am_Value_List()
                                     .Add(Am_LABEL)
                                     .Add(COMMAND_KEY)
                                     .Add(Am_ITEMS)
                                     .Add(Am_ACCELERATOR)
                                     .Add(Am_ACTIVE)
                                     .Add(Am_ACTIVE_2));

  menu_proto = Am_Menu_Bar.Create()
                   .Set(Am_ITEMS, NULL)
                   .Set(Am_FILL_STYLE, n2s[5])
                   .Add(TYPE_NAME, "Am_Menu_Bar")
                   .Add(FILL_STYLE_KEY, 5)
                   .Add(SAVE_MENU_BAR, true)
                   .Add(Am_SAVE_OBJECT_METHOD, Am_Standard_Save_Object)
                   .Add(Am_SLOTS_TO_SAVE,
                        Am_Value_List().Add(Am_ITEMS).Add(FILL_STYLE_KEY));

  Am_Default_Load_Save_Context.Register_Prototype("MENU_BAR", menu_proto);
  Am_Default_Load_Save_Context.Register_Prototype("COMMAND", am_command_proto);
  Am_Default_Load_Save_Context.Register_Prototype(
      "SELECT_ALL_COMMAND", am_selection_widget_all_proto);
  Am_Default_Load_Save_Context.Register_Prototype("CLEAR_COMMAND", clear_proto);
  Am_Default_Load_Save_Context.Register_Prototype("CLEAR_ALL_COMMAND",
                                                  clear_all_proto);
  Am_Default_Load_Save_Context.Register_Prototype("COPY_COMMAND", copy_proto);
  Am_Default_Load_Save_Context.Register_Prototype("CUT_COMMAND", cut_proto);
  Am_Default_Load_Save_Context.Register_Prototype("PASTE_COMMAND", paste_proto);
  Am_Default_Load_Save_Context.Register_Prototype("BOTTOM_COMMAND",
                                                  bottom_proto);
  Am_Default_Load_Save_Context.Register_Prototype("TOP_COMMAND", top_proto);
  Am_Default_Load_Save_Context.Register_Prototype("DUPLICATE_COMMAND",
                                                  duplicate_proto);
  Am_Default_Load_Save_Context.Register_Prototype("GROUP_COMMAND", group_proto);
  Am_Default_Load_Save_Context.Register_Prototype("UNGROUP_COMMAND",
                                                  ungroup_proto);
  Am_Default_Load_Save_Context.Register_Prototype("UNDO_COMMAND", undo_proto);
  Am_Default_Load_Save_Context.Register_Prototype("REDO_COMMAND", redo_proto);
  Am_Default_Load_Save_Context.Register_Prototype("QUIT_COMMAND", quit_proto);
  Am_Default_Load_Save_Context.Register_Prototype("OPEN_COMMAND", open_proto);
  Am_Default_Load_Save_Context.Register_Prototype("SAVE_AS_COMMAND",
                                                  save_as_proto);
  Am_Default_Load_Save_Context.Register_Prototype("SAVE_COMMAND", save_proto);
  Am_Default_Load_Save_Context.Register_Prototype("MENU_LINE_COMMAND",
                                                  menu_line_proto);

  node_proto =
      Am_Group.Create()
          .Set(Am_WIDTH, Am_Width_Of_Parts)
          .Set(Am_HEIGHT, Am_Height_Of_Parts)
          .Add(Lw_CHILDREN, Am_Value_List())
          .Add(Am_SELECTED, false)
          .Add(Am_ACCELERATOR, "")
          .Add_Part(TEXT1, Am_Text.Create()
                               .Set(Am_LEFT, 4)
                               .Set(Am_CURSOR_INDEX, 0)
                               .Set(Am_TEXT, "")
                               .Add_Part(Am_Text_Edit_Interactor.Create().Set(
                                   Am_WANT_PENDING_DELETE, true)))
          .Add_Part(Am_Text.Create()
                        .Set(Am_LEFT, Am_From_Sibling(TEXT1, Am_WIDTH, +13))
                        .Set(Am_TEXT, Am_From_Owner(Am_ACCELERATOR)))
          .Add_Part(TEXT2, Am_Text.Create()
                               .Set(Am_TOP, 13)
                               .Add(COMMAND_KEY, 0)
                               .Set(Am_TEXT, ""))
          .Add_Part(Am_Rectangle.Create()
                        .Set(Am_FILL_STYLE, Am_No_Style)
                        .Set(Am_LINE_STYLE, compute_line_style)
                        .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH))
                        .Set(Am_HEIGHT, Am_From_Owner(Am_HEIGHT)));

  Am_Object text = Am_Text.Create().Set(Am_FILL_STYLE, Am_Motif_Light_Gray);

  Menu_Edit_Window =
      Am_Window.Create()
          .Set(Am_WIDTH, 355)
          .Set(Am_HEIGHT, 445)
          .Set(Am_FILL_STYLE, Am_Motif_Light_Gray)
          .Add(FAKE_DELETE, Am_Value_List())
          .Set(Am_UNDO_HANDLER, Am_Multiple_Undo_Object.Create())
          .Add_Part(Am_Menu_Bar.Create().Set(
              Am_ITEMS,
              Am_Value_List()
                  .Add(Am_Command.Create()
                           .Set(Am_LABEL, "Node")
                           .Set(Am_ITEMS,
                                Am_Value_List()
                                    .Add(Am_Command.Create()
                                             .Set(Am_LABEL, "New")
                                             .Set(Am_ACCELERATOR, "CONTROL_n")
                                             .Set(Am_DO_METHOD, add_new_submenu)
                                             .Set(Am_UNDO_METHOD,
                                                  undo_add_submenu)
                                             .Set(Am_REDO_METHOD,
                                                  undo_add_submenu))
                                    .Add(Am_Command.Create()
                                             .Set(Am_LABEL, "Delete")
                                             .Set(Am_ACCELERATOR, "CONTROL_d")
                                             .Set(Am_DO_METHOD, delete_node)
                                             .Set(Am_UNDO_METHOD,
                                                  undo_delete_node)
                                             .Set(Am_REDO_METHOD,
                                                  undo_delete_node))))
                  .Add(
                      Am_Command.Create()
                          .Set(Am_LABEL, "Edit")
                          .Set(
                              Am_ITEMS,
                              Am_Value_List()
                                  .Add(Am_Undo_Command.Create())
                                  .Add(Am_Redo_Command.Create())
                                  .Add(Am_Menu_Line_Command.Create())
                                  .Add(Am_Command.Create()
                                           .Set(Am_LABEL, "Go up")
                                           .Set(Am_ACCELERATOR, "CONTROL_u")
                                           .Set(Am_DO_METHOD, go_up_menu)
                                           .Set(Am_UNDO_METHOD, undo_move_node)
                                           .Set(Am_REDO_METHOD, undo_move_node))
                                  .Add(Am_Command.Create()
                                           .Set(Am_LABEL, "Go down")
                                           .Set(Am_ACCELERATOR, "CONTROL_b")
                                           .Set(Am_DO_METHOD, go_down_menu)
                                           .Set(Am_UNDO_METHOD, undo_move_node)
                                           .Set(Am_REDO_METHOD, undo_move_node))
                                  .Add(Am_Command.Create()
                                           .Set(Am_LABEL, "Make parent")
                                           .Set(Am_ACCELERATOR, "CONTROL_p")
                                           .Set(Am_DO_METHOD,
                                                make_it_upper_level)
                                           .Set(Am_UNDO_METHOD, undo_move_node)
                                           .Set(Am_REDO_METHOD, undo_move_node))
                                  .Add(Am_Command.Create()
                                           .Set(Am_LABEL, "Make child")
                                           .Set(Am_ACCELERATOR, "CONTROL_c")
                                           .Set(Am_DO_METHOD,
                                                make_it_lower_level)
                                           .Set(Am_UNDO_METHOD, undo_move_node)
                                           .Set(Am_REDO_METHOD,
                                                undo_move_node))))))
          .Add_Part(
              COMMAND_KEY,
              Am_Scrolling_Group.Create()
                  .Set(Am_TOP, 35)
                  .Set(Am_LEFT, 5)
                  .Set(Am_HEIGHT, 68)
                  .Set(Am_WIDTH, compute_right_width)
                  .Set(Am_INNER_WIDTH, Am_Width_Of_Parts)
                  .Set(Am_INNER_HEIGHT, Am_Height_Of_Parts)
                  .Set(Am_V_SMALL_INCREMENT, 12)
                  .Set(Am_INNER_FILL_STYLE, Am_White)
                  .Set(Am_FILL_STYLE, Am_Motif_Gray)
                  .Set(Am_H_SCROLL_BAR, false)
                  .Add(COMMAND_KEY, 0)
                  .Add_Part(
                      COMMAND_LIST,
                      Am_Map.Create()
                          .Add(Am_WIDTH, Am_From_Owner(Am_WIDTH),
                               Am_OK_IF_THERE)
                          .Set(Am_TOP, 0)
                          .Set(Am_LEFT, 0)
                          .Set(Am_ITEMS, COMMAND_CNT)
                          .Set(Am_LAYOUT, Am_Vertical_Layout)
                          .Set(
                              Am_ITEM_PROTOTYPE,
                              Am_Text.Create()
                                  .Add(Am_SELECTED, false)
                                  .Set(Am_INVERT, set_invert)
                                  .Set(Am_FILL_STYLE, Am_White)
                                  .Set(Am_TEXT, get_text)
                                  .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH))
                                  .Add_Part(
                                      Am_One_Shot_Interactor
                                          .Create()
                                          .Set(Am_START_WHEN,
                                               "DOUBLE_LEFT_CLICK")
                                          .Get_Object(Am_COMMAND)
                                          .Set(Am_DO_METHOD, add_new_submenu)
                                          .Set(Am_UNDO_METHOD, undo_add_submenu)
                                          .Set(Am_REDO_METHOD, undo_add_submenu)
                                          .Get_Owner()))
                          .Add_Part(CHOICE_INTER,
                                    Am_Choice_Interactor.Create()
                                        .Set(Am_HOW_SET, Am_CHOICE_SET)
                                        .Get_Object(Am_COMMAND)
                                        .Set(Am_DO_METHOD, change_command_key)
                                        .Get_Owner())))
          .Add_Part(
              NEW_DELETE_PANNEL,
              Am_Button_Panel.Create()
                  .Add(NUMBER_OF_NODES, 1)
                  .Set(Am_HEIGHT, Am_Height_Of_Parts)
                  .Set(Am_WIDTH, Am_Width_Of_Parts)
                  .Set(Am_LEFT, Am_From_Sibling(COMMAND_KEY, Am_WIDTH, +10))
                  .Set(Am_TOP, 35)
                  .Set(Am_FILL_STYLE, Am_Motif_Light_Gray)
                  .Set(Am_H_ALIGN, Am_CENTER_ALIGN)
                  .Set(Am_LAYOUT, Am_Vertical_Layout)
                  .Set(Am_ITEMS,
                       Am_Value_List()
                           .Add(Am_Command.Create()
                                    .Set(Am_LABEL, "Create")
                                    .Set(Am_DO_METHOD, add_new_submenu)
                                    .Set(Am_UNDO_METHOD, undo_add_submenu)
                                    .Set(Am_REDO_METHOD, undo_add_submenu))
                           .Add(Am_Command.Create()
                                    .Set(Am_LABEL, "Delete")
                                    .Set(Am_DO_METHOD, delete_node)
                                    .Set(Am_UNDO_METHOD, undo_delete_node)
                                    .Set(Am_REDO_METHOD, undo_delete_node))))
          .Add_Part(Am_Line.Create()
                        .Set(Am_X1, 0)
                        .Set(Am_X2, Am_From_Owner(Am_WIDTH))
                        .Set(Am_Y1, 107)
                        .Set(Am_Y2, 107)
                        .Set(Am_LINE_STYLE, n2l[11][3]))
          .Add_Part(Am_Line.Create()
                        .Set(Am_X1, 0)
                        .Set(Am_X2, Am_From_Owner(Am_WIDTH))
                        .Set(Am_Y1, 109)
                        .Set(Am_Y2, 109)
                        .Set(Am_LINE_STYLE, n2l[9][3]))
          .Add_Part(
              Am_Button_Panel.Create()
                  .Set(Am_HEIGHT, Am_Height_Of_Parts)
                  .Set(Am_WIDTH, Am_Width_Of_Parts)
                  .Set(Am_LEFT, 10)
                  .Set(Am_TOP, 114)
                  .Set(Am_FILL_STYLE, Am_Motif_Light_Gray)
                  .Set(Am_H_ALIGN, Am_CENTER_ALIGN)
                  .Set(Am_LAYOUT, Am_Horizontal_Layout)
                  .Set(Am_ITEMS,
                       Am_Value_List()
                           .Add(Am_Command.Create()
                                    .Set(Am_LABEL, Am_Bitmap.Create().Set(
                                                       Am_IMAGE, arrow_left))
                                    .Set(Am_DO_METHOD, make_it_upper_level)
                                    .Set(Am_UNDO_METHOD, undo_move_node)
                                    .Set(Am_REDO_METHOD, undo_move_node))
                           .Add(Am_Command.Create()
                                    .Set(Am_LABEL, Am_Bitmap.Create().Set(
                                                       Am_IMAGE, arrow_right))
                                    .Set(Am_DO_METHOD, make_it_lower_level)
                                    .Set(Am_UNDO_METHOD, undo_move_node)
                                    .Set(Am_REDO_METHOD, undo_move_node))
                           .Add(Am_Command.Create()
                                    .Set(Am_LABEL, Am_Bitmap.Create().Set(
                                                       Am_IMAGE, arrow_up))
                                    .Set(Am_DO_METHOD, go_up_menu)
                                    .Set(Am_UNDO_METHOD, undo_move_node)
                                    .Set(Am_REDO_METHOD, undo_move_node))
                           .Add(Am_Command.Create()
                                    .Set(Am_LABEL, Am_Bitmap.Create().Set(
                                                       Am_IMAGE, arrow_down))
                                    .Set(Am_DO_METHOD, go_down_menu)
                                    .Set(Am_UNDO_METHOD, undo_move_node)
                                    .Set(Am_REDO_METHOD, undo_move_node))))
          .Add_Part(ACCELERATOR_INPUT,
                    Am_Text_Input_Widget.Create()
                        .Set(Am_LEFT, compute_left_coord)
                        .Set(Am_TOP, 115)
                        .Set(Am_WIDTH, 150)
                        .Set(Am_FILL_STYLE, Am_White)
                        .Get_Object(Am_INTERACTOR)
                        .Set(Am_STOP_WHEN, NULL)
                        .Set(Am_TEXT_EDIT_METHOD, gilt_single_char_method)
                        .Get_Owner()
                        .Get_Object(Am_COMMAND)
                        .Set(Am_LABEL, "Accelator")
                        .Set(Am_DO_METHOD, change_accelerator)
                        .Set(Am_UNDO_METHOD, undo_change_accelerator)
                        .Get_Owner())
          .Add_Part(
              Lw_GROUP,
              Am_Scrolling_Group.Create()
                  .Set(Am_LEFT, 5)
                  .Set(Am_TOP, 145)
                  .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH, -10))
                  .Set(Am_HEIGHT, 265)
                  .Set(Am_INNER_WIDTH, Am_Width_Of_Parts)
                  .Set(Am_INNER_HEIGHT, Am_Height_Of_Parts)
                  .Set(Am_FILL_STYLE, Am_White)
                  .Set(Am_INNER_WIDTH, Am_Width_Of_Parts)
                  .Set(Am_INNER_HEIGHT, Am_Height_Of_Parts)
                  .Add_Part(Lw_GROUP,
                            Am_Group.Create()
                                .Set(Am_LEFT, 0)
                                .Set(Am_TOP, 10)
                                .Set(Am_WIDTH, Am_Width_Of_Parts)
                                .Set(Am_HEIGHT, Am_Height_Of_Parts)
                                .Add_Part(CHOICE_INTER,
                                          Am_Choice_Interactor.Create()
                                              .Set(Am_HOW_SET, Am_CHOICE_SET)
                                              .Get_Object(Am_COMMAND)
                                              .Set(Am_DO_METHOD,
                                                   display_accelerator)
                                              .Get_Owner())))
          .Add_Part(
              Am_Button_Panel.Create()
                  .Set(Am_HEIGHT, Am_Height_Of_Parts)
                  .Set(Am_LEFT, compute_left_coord)
                  .Set(Am_TOP, 414)
                  .Set(Am_LAYOUT, Am_Horizontal_Layout)
                  .Set(Am_FILL_STYLE, Am_Motif_Light_Gray)
                  .Set(Am_H_ALIGN, Am_CENTER_ALIGN)
                  .Set(Am_ITEMS,
                       Am_Value_List()
                           .Add(Am_Command.Create()
                                    .Set(Am_LABEL, "    OK    ")
                                    .Set(Am_DO_METHOD,
                                         close_ok_menu_edit_window))
                           .Add(Am_Command.Create()
                                    .Set(Am_LABEL, "  Cancel  ")
                                    .Set(Am_DO_METHOD,
                                         close_cancel_menu_edit_window))));

  Am_Object lgroup = Menu_Edit_Window.Get_Object(Lw_GROUP).Get_Object(Lw_GROUP);
  Outline *ol = new Outline(lgroup);

  Menu_Edit_Window.Add(OUTLINE, (Am_Ptr)ol);

  lgroup.Add_Part(
      ol->Root(1, Am_Group.Create()
                      .Set(Am_WIDTH, Am_Width_Of_Parts)
                      .Set(Am_HEIGHT, Am_Height_Of_Parts)
                      .Add(Am_SELECTED, true)
                      .Add(Am_ACCELERATOR, "")
                      .Add_Part(Am_Text.Create()
                                    .Set(Am_TEXT, "Menu Bar")
                                    .Set(Am_FILL_STYLE, Am_No_Style))
                      .Add_Part(Am_Rectangle.Create()
                                    .Set(Am_WIDTH, Am_From_Owner(Am_WIDTH))
                                    .Set(Am_FILL_STYLE, Am_No_Style)
                                    .Set(Am_HEIGHT, Am_From_Owner(Am_HEIGHT))
                                    .Set(Am_LINE_STYLE, compute_line_style))));

  /******************* END OF MENU EDIT WINDOW ****************************/

  return Menu_Edit_Window;
}
