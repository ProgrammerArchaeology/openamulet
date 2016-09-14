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

#ifndef AMULET_H
#include <amulet.h>
#endif

#ifndef LISTBOX_ABC_HPP
#include "amulet/listbox_widget_abc.hpp"
#endif

//-------
// Slots
//-------
DECLARE_SLOT(Am_SELECTED_COLOR);
DECLARE_SLOT(Am_MULTIPLE_SELECTION);
DECLARE_SLOT(Am_LISTITEM);

/// Pointer to the C++ object where the selectMethod() will be called
DECLARE_SLOT(Am_THIS);

//---------------
// Amulet Methods
//---------------
Am_Define_Method(Am_Object_Method, void, listbox_method, (Am_Object self))
{
  // Get the selected listitem object
  // Owner-chain: interactor-command->interactor->rectangle->listitem-group
  Am_Object listitem = self.Get_Owner().Get_Owner().Get_Owner();

  /// Do we have to handle multiple selects?
  if (static_cast<bool>(listitem.Get_Owner().Get(Am_MULTIPLE_SELECTION)) ==
      true) {
    /// just toggel the select status
    if (static_cast<bool>(listitem.Get(Am_SELECTED)) == true) {
      listitem.Set(Am_SELECTED, false);
    } else {
      listitem.Set(Am_SELECTED, true);
    }
  } else {
    /// Get list of all listitems from listbox object
    Am_Value_List list = listitem.Get_Owner().Get(Am_GRAPHICAL_PARTS);
    Am_Object item;

    /// find the previous selected listitem
    bool test = true;
    for (list.Start(); (list.Last() == false) && (test == true); list.Next()) {
      item = list.Get();
      if (static_cast<bool>(item.Get(Am_SELECTED)) == true) {
        /// and deselect it
        item.Set(Am_SELECTED, false);
        test = false;
      }
    }

    /// set the new listitem as selected
    listitem.Set(Am_SELECTED, true);
  }

  /// and last but not least call the selectMethod through the ABC pointer!
  reinterpret_cast<Am_Listbox_ABC *>(
      static_cast<Am_Ptr>(listitem.Get_Owner().Get(Am_THIS)))
      ->selectMethod();

  /// change the flag of the listbox object so that all constraints/formulas are re-evaluated
  if (static_cast<bool>(listitem.Get_Owner().Get(Am_SELECTED)) == true) {
    listitem.Get_Owner().Set(Am_SELECTED, false);
  } else {
    listitem.Get_Owner().Set(Am_SELECTED, true);
  }

  return;
}

//-----------------
// Amulet Formulars
//-----------------
Am_Define_Formula(Am_Value, selected_owner_color)
{
  // owner-chain: rectangle->listitem-group
  Am_Object listitem = self.Get_Owner();

  // Which color should we return
  if (static_cast<bool>(listitem.Get(Am_SELECTED)) == true) {
    // get the line-style information from the listbox item
    return (static_cast<Am_Value>(listitem.Get_Owner().Get(Am_SELECTED_COLOR)));
  } else {
    // get the line-style information from the listbox item
    return (static_cast<Am_Value>(listitem.Get_Owner().Get(Am_FILL_STYLE)));
  }
}

Am_Define_Formula(int, listbox_width)
{
  // Get the object owning the listbox
  Am_Object owner = self.Get_Owner();

  // return a width, which is a bit smaller than the parent width
  // TODO: Add a WIDTH_SUBTRACTOR slot to the object
  return (static_cast<int>(owner.Get(Am_WIDTH)) -
          10 /* GUIConstant::listboxWidthSubtractor */);
}

Am_Define_Formula(bool, listbox_hscrollbar)
{
  // do we need a horizontal scrollbar?
  if (static_cast<int>(self.Get(Am_INNER_WIDTH)) >
      static_cast<int>(self.Get(Am_WIDTH))) {
    // yep, just enable it
    return (true);
  } else {
    // no, disable it
    return (false);
  }
}

Am_Define_Formula(bool, listbox_vscrollbar)
{
  // do we need a vertical scrollbar?
  if (static_cast<int>(self.Get(Am_INNER_HEIGHT)) >
      static_cast<int>(self.Get(Am_HEIGHT))) {
    // yep, just enable it
    return (true);
  } else {
    // no, disable it
    return (false);
  }
}
