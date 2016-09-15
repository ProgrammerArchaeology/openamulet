/* 
  designed and implemented  by S. Nebel Linkworks Ltd. Wellinton, NZ 
  Implements a MS Filemanager like tree.
  
  This code is public domain. 

  Use it on own risk.

*/

#include <stdio.h>
#include <iostream.h>
#include <amulet.h>
#include "outline.h"
#include <stdlib.h>

Am_Slot_Key Lw_CHILDREN = Am_Register_Slot_Name("Lw_CHILDREN");
Am_Slot_Key Lw_HIDDEN = Am_Register_Slot_Name("Lw_HIDDEN");
Am_Slot_Key Lw_HSPACING = Am_Register_Slot_Name("Lw_HSPACING");
Am_Slot_Key Lw_NODEID = Am_Register_Slot_Name("Lw_NODEID");
Am_Slot_Key Lw_PARENT = Am_Register_Slot_Name("Lw_PARENT");
Am_Slot_Key Lw_VSPACING = Am_Register_Slot_Name("Lw_VSPACING");
Am_Slot_Key Lw_HITEM = Am_Register_Slot_Name("Lw_HITEM");
Am_Slot_Key Lw_VITEM = Am_Register_Slot_Name("Lw_VITEM");
Am_Slot_Key Lw_BITMAP = Am_Register_Slot_Name("Lw_BITMAP");
Am_Slot_Key Lw_LAYOUT = Am_Register_Slot_Name("Lw_LAYOUT");

Am_Image_Array plus_image(Am_Merge_Pathname("lib/images/plus.gif"));
Am_Image_Array minus_image(Am_Merge_Pathname("lib/images/minus.gif"));

/* uncomment EXPAND_ONE_LEVEL_ONLY to expand after collapsing only one level.
   The default behaviour is to remember what was expanded 
*/

// #define EXPAND_ONE_LEVEL_ONLY

Am_Style Motif_Blue_Dashed = Am_No_Style;

Am_Define_Method(Am_Object_Method, void, do_hide, (Am_Object cmd))
{
  Am_Object bm = cmd.Get_Owner().Get_Object(Am_INTERIM_VALUE);
  if (bm.Valid()) {
    Am_Object am = bm.Get(Lw_PARENT);
    if (((Am_Value_List)am.Get(Lw_CHILDREN)).Length()) {
      am.Set(Lw_HIDDEN, !(bool)am.Get(Lw_HIDDEN));
    } else {
      am.Set(Lw_HIDDEN, false);
    }
  }
}

Am_Define_Formula(int, hitem_coords)
{
  Am_Object p = self.Get_Object(Lw_PARENT);
  self.Set(Am_X2, (int)p.Get(Am_LEFT) - 2)
      .Set(Am_Y1, (int)p.Get(Am_TOP) + (int)p.Get(Am_HEIGHT) / 2)
      .Set(Am_Y2, (int)p.Get(Am_TOP) + (int)p.Get(Am_HEIGHT) / 2);
  return ((int)p.Get(Am_LEFT) - 15 - (int)p.Get(Lw_HSPACING));
}

Am_Define_Formula(int, vitem_coords)
{
  Am_Object p = self.Get_Object(Lw_PARENT);
  //p = p.Get(Lw_PARENT);
  Am_Value_List l = p.Get(Lw_CHILDREN);
  if (l.Length()) {
    l.End();
    Am_Object last = l.Get();

    self.Set(Am_Y1, (int)p.Get(Am_TOP) + (int)p.Get(Am_HEIGHT) / 2)
        .Set(Am_Y2, (int)last.Get(Am_TOP) + (int)last.Get(Am_HEIGHT) / 2)
        .Set(Am_X2, (int)p.Get(Am_LEFT) - 15);
  }
  return ((int)p.Get(Am_LEFT) - 15);
}

Am_Define_Formula(int, bm_coords)
{
  Am_Object p = self.Get_Object(Lw_PARENT);

  if (!p.Valid()) {
    // cout << "bm  not valid" << p << endl;
    return (0);
  }
  //p = p.Get(Lw_PARENT);
  // cout << "bm parent:" << p  << "bm " << self << "left " << (int)p.Get(Am_LEFT) -24 << endl;
  Am_Value_List l = p.Get(Lw_CHILDREN);
  if (l.Length()) {
    l.End();
    Am_Object last = l.Get();

    self.Set(Am_TOP, (int)p.Get(Am_TOP) + (int)p.Get(Am_HEIGHT) / 2 - 5);
    if ((bool)p.Get(Lw_HIDDEN))
      self.Set(Am_IMAGE, plus_image);
    else
      self.Set(Am_IMAGE, minus_image);
  }
  //return(24);
  return ((int)p.Get(Am_LEFT) - 20);
}

Am_Define_Formula(bool, hline_visible)
{
  Am_Object p = self.Get_Object(Lw_PARENT);
  return ((bool)p.Get(Am_VISIBLE));
}

Am_Define_Formula(bool, bm_visible)
{
  Am_Object p = self.Get(Lw_PARENT);
  Am_Value_List l = p.Get(Lw_CHILDREN);
  return ((bool)p.Get(Am_VISIBLE) && !l.Empty());
}
Am_Define_Formula(bool, vline_visible)
{
  Am_Object p = self.Get_Object(Lw_PARENT);
  Am_Value_List l = p.Get(Lw_CHILDREN);
  //  cout << p.Get(Lw_NODEID) << "   " << l.Length() << endl;
  return ((bool)p.Get(Am_VISIBLE) && !(bool)p.Get(Lw_HIDDEN) && !l.Empty());
}

Outline::Outline(Am_Object group) : Am_Outline(), _group(group) {}

void
Outline::DestroyNode(Am_Object am)
{
  Am_Value item;

  item = am.Peek(Lw_HITEM);
  if (item.Valid())
    ((Am_Object)item).Destroy();

  item = am.Peek(Lw_VITEM);
  if (item.Valid())
    ((Am_Object)item).Destroy();

  item = am.Peek(Lw_BITMAP);
  if (item.Valid())
    ((Am_Object)item).Destroy();
  Am_Outline::DestroyNode(am);
}

Am_Object
Outline::Get_Root()
{
  return Am_Outline::GetRoot();
}

bool
Outline::RemoveNode(long id)
{
  Am_Object am = FindNode(id);
  if (am.Valid()) {
    Am_Object parent = am.Get(Lw_PARENT);
    bool ret = Am_Outline::RemoveNode(id);

    if (parent.Valid() && ret) {
      parent.Note_Changed(Lw_CHILDREN);
    }
    return (ret);
  }
  return (false);
}

Outline::~Outline() {}

Am_Object
Outline::Root(long id, Am_Object item)
{
  item.Set(Am_LEFT, (int)item.Get(Am_LEFT) + 20);
  item = Am_Outline::Root(id, item);

  if (Motif_Blue_Dashed == Am_No_Style) {
    float r, g, b;
    Am_Motif_Blue.Get_Values(r, g, b);
    static char dasher[2] = {2, 2};
    Motif_Blue_Dashed = Am_Style(r, g, b, 1, Am_CAP_BUTT, Am_JOIN_MITER,
                                 Am_LINE_ON_OFF_DASH, dasher, 2);
  }

  Am_Object hl = Am_Line.Create()
                     .Set(Am_LINE_STYLE, Motif_Blue_Dashed)
                     .Add(Lw_PARENT, item)
                     .Set(Am_VISIBLE, hline_visible)
                     .Set(Am_X1, hitem_coords);

  item.Add(Lw_HITEM, hl).Set(Am_LEFT, (int)item.Get(Am_LEFT) + 11);
  _group.Add_Part(hl);

  item.Add(Lw_VITEM, Am_No_Object);

  return (item);
}

Am_Object
Outline::AddNode(long id, long addto, Am_Object item)
{

  item = Am_Outline::AddNode(id, addto, item);
  Am_Object hl = Am_Line.Create()
                     .Add(Lw_PARENT, item)
                     .Set(Am_LINE_STYLE, Motif_Blue_Dashed)
                     .Set(Am_VISIBLE, hline_visible)
                     .Set(Am_X1, hitem_coords);

  item.Add(Lw_HITEM, hl).Add(Lw_VITEM, Am_No_Object);
  _group.Add_Part(hl);
  Am_Object p = item.Get(Lw_PARENT);
  if (p.Get(Lw_VITEM) == Am_No_Object) {

    Am_Object vl, bm;
    _group.Add_Part(vl = Am_Line.Create()
                             .Set(Am_LINE_STYLE, Motif_Blue_Dashed)
                             .Add(Lw_PARENT, p)
                             .Set(Am_VISIBLE, vline_visible)
                             .Set(Am_X1, vitem_coords));
    p.Set(Lw_VITEM, vl);

    _group.Add_Part(bm = Am_Bitmap.Create()
                             .Add(Lw_PARENT, p)
                             .Set(Am_VISIBLE, bm_visible)
                             .Set(Am_LEFT, 20)
                             .Add(Am_HIT_THRESHOLD, 7)
                             .Set(Am_LEFT, bm_coords)
                             .Add_Part(Am_One_Shot_Interactor.Create()
                                           .Get_Object(Am_COMMAND)
                                           .Set(Am_DO_METHOD, do_hide)
                                           .Get_Owner()));

    p.Add(Lw_BITMAP, bm, Am_OK_IF_THERE);
  }
  return (item);
}

void
Outline::AddBmVi(Am_Object p)
{

  if (p.Get(Lw_VITEM) == Am_No_Object) {

    Am_Object vl, bm;
    _group.Add_Part(vl = Am_Line.Create()
                             .Set(Am_LINE_STYLE, Motif_Blue_Dashed)
                             .Add(Lw_PARENT, p)
                             .Set(Am_VISIBLE, vline_visible)
                             .Set(Am_X1, vitem_coords));
    p.Set(Lw_VITEM, vl);

    _group.Add_Part(bm = Am_Bitmap.Create()
                             .Add(Lw_PARENT, p)
                             .Set(Am_VISIBLE, bm_visible)
                             .Set(Am_LEFT, 20)
                             .Add(Am_HIT_THRESHOLD, 7)
                             .Set(Am_LEFT, bm_coords)
                             .Add_Part(Am_One_Shot_Interactor.Create()
                                           .Get_Object(Am_COMMAND)
                                           .Set(Am_DO_METHOD, do_hide)
                                           .Get_Owner()));

    p.Add(Lw_BITMAP, bm);
  }
}

void
Outline::RemoveBm(Am_Object p)
{

  if (p.Get(Lw_VITEM) != Am_No_Object) {
    if (((Am_Value_List)p.Get(Lw_CHILDREN)).Empty()) {

      _group.Remove_Part((Am_Object)p.Get(Lw_VITEM));
      p.Set(Lw_VITEM, Am_No_Object);

      _group.Remove_Part((Am_Object)p.Get(Lw_BITMAP));
      p.Set(Lw_BITMAP, Am_No_Object);
    }
  }
}

Am_Outline::Am_Outline() : _rootObject(Am_No_Object) {}

void
Hide_Subtree(Am_Object am)
{

  //  cout << "Hide_Subtree :" << am << am.Get(Am_TEXT) << endl;
  if (!(bool)am.Get(Am_VISIBLE))
    return;
  Am_Value_List kids = am.Get(Lw_CHILDREN);
  am.Set(Am_VISIBLE, false);

  Am_Object child;

  for (kids.Start(); !kids.Last(); kids.Next()) {
    child = kids.Get();
    Hide_Subtree(child);
  }
}

void
Layout_Subtree(Am_Object am, int &curtop, int curleft)
{
  Am_Value_List kids = am.Get(Lw_CHILDREN);
  // cout << "Layout_Subtree :" << am  << endl;
  am.Set(Am_TOP, curtop, Am_OK_IF_NOT_THERE).Set(Am_LEFT, curleft);
  curtop = curtop + (int)am.Get(Lw_VSPACING) + (int)am.Get(Am_HEIGHT);
  Am_Object child;
  for (kids.Start(); !kids.Last(); kids.Next()) {
    child = kids.Get();
    if ((bool)am.Get(Lw_HIDDEN)) {

#ifdef EXPAND_ONE_LEVEL_ONLY
      child.Set(Lw_HIDDEN, true);
#endif
      Hide_Subtree(child);
    } else {
      child.Set(Am_VISIBLE, true, Am_OK_IF_NOT_THERE);
      Layout_Subtree(child, curtop, curleft + (int)am.Get(Lw_HSPACING));
    }
  }
}

Am_Define_Formula(int, Horizontal_Tree_Layout)
{
  int curtop = self.Get(Am_TOP);
  //  int curheight = self.Get(Am_HEIGHT);
  //  curtop += curheight;
  int curleft = self.Get(Am_LEFT);

  //  cout << "Horizontal_Tree_Layout" << self << self.Get(Am_TEXT) << endl;
  Layout_Subtree(self, curtop, curleft);
  return (3);
}

Am_Outline::~Am_Outline()
{

  if (_rootObject.Valid()) {
    RemoveNode((int)_rootObject.Get(Lw_NODEID));
  }
}

Am_Object
Am_Outline::Root(long id, Am_Object item)
{
  if (_rootObject)
    RemoveNode((int)_rootObject.Get(Lw_NODEID));
  item.Add(Lw_HIDDEN, false);
  item.Add(Lw_NODEID, id);
  item.Add(Lw_PARENT, Am_No_Object);
  Am_Value_List l1;
  item.Add(Lw_HSPACING, 20);
  item.Add(Lw_VSPACING, 3);

  //    cout <<" root:" <<id << "rootobject :" << item <<"item :" << item <<endl;
  item.Add(Lw_CHILDREN, l1);
  item.Add(Lw_LAYOUT, Horizontal_Tree_Layout);

  _rootObject = item;

  return (_rootObject);
}

Am_Object
Am_Outline::AddNode(long id, long addto, Am_Object item)
{
  Am_Object am = FindNode(addto);
  if (am) {
    // cout << id << "addto : "<< addto << " parent : " << am << "item :" << item << endl;
    item.Set(Am_VISIBLE, true);
    item.Add(Lw_HSPACING, 20);
    item.Add(Lw_VSPACING, 3);
    item.Add(Lw_PARENT, am);
    item.Add(Lw_HIDDEN, false);
    item.Add(Lw_NODEID, id);
    Am_Value_List l1;
    item.Set(Lw_CHILDREN, l1, Am_OK_IF_NOT_THERE);

    Am_Value_List l = am.Get(Lw_CHILDREN);
    l.Add(item, Am_TAIL);
    am.Set(Lw_CHILDREN, l);
    _rootObject.Note_Changed(Lw_CHILDREN);
    return (item);
  }
  return (Am_No_Object);
}

bool
Am_Outline::RemoveNode(Am_Object am)
{
  if (am.Valid())
    return (RemoveNode((long)(int)am.Get(Lw_NODEID)));
  return (false);
}

bool
Am_Outline::RemoveNode(long id)
{
  Am_Object am = FindNode(id);
  if (am.Valid()) {
    Am_Object parent = am.Get(Lw_PARENT);
    if (parent.Valid()) {
      Am_Value_List l = parent.Get(Lw_CHILDREN);
      l.Start();
      if (l.Member(am))
        l.Delete(false);
    }
    RemoveSubtree(am);
    _rootObject.Note_Changed(Lw_CHILDREN);
    return (true);
  }
  return (false);
}

bool
Am_Outline::ShowSubtree(long id, bool show)
{
  Am_Object am = FindNode(id);
  if (am) {
    am.Set(Lw_HIDDEN, !show);
    _rootObject.Note_Changed(Lw_CHILDREN);
    return (true);
  }
  return (false);
}

void
Am_Outline::RemoveSubtree(Am_Object am)
{
  if (am) {
    Am_Value_List l = am.Get(Lw_CHILDREN);
    for (l.End(); !l.First(); l.Prev()) {
      Am_Object child = l.Get();
      RemoveSubtree(child);
    } /* for */
    l.Make_Empty();
    if (am != _rootObject)
      DestroyNode(am);
  }
}

Am_Object
Am_Outline::FindNode(long id)
{
  if ((int)_rootObject.Get(Lw_NODEID) == id)
    return (_rootObject);
  else {
    return (FindNodeInternal(_rootObject, id));
  }
}

Am_Object
Am_Outline::FindNodeInternal(Am_Object am, long id)
{
  Am_Value_List l = am.Get(Lw_CHILDREN);
  for (l.End(); !l.First(); l.Prev()) {
    Am_Object child = l.Get();
    if ((int)(child.Get(Lw_NODEID)) == id)
      return (child);
    else {
      Am_Object found = FindNodeInternal(child, id);
      if (found)
        return (found);
    }
  } /* for */
  /* no luck situation from here on */

  return (Am_No_Object);
}
