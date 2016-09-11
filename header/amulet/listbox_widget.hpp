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

#ifndef LISTBOX_HPP
#define LISTBOX_HPP

#include <string>
#include <functional>

#ifndef AMULET_H
	#include <amulet.h>
#endif

// IsA relationship
#ifndef AM_LISTBOX_ABC_HPP
	#include <amulet/listbox_widget_abc.hpp>
#endif

//:This is a listbox widget.
// The listbox can be filled with different objects. The selectMethod()
// has to determine what kind of object was selected and how to map the
// selected object to an object (maybe a pointer) useable by the owner of the listbox.
// Because the selectable objects in the listbox are mostly 'visual representations'
// of internal objects this mapping is needed.
//
// The 'Am_SELECTED' slot of listbox is toggeld by each selection. So this slot can
// be used to trigger an action via a constraint on other objects.
//
// The first template parameter specifies the type of the 'owner' of the listbox.
// The owner is the object on which the selected function will be called.
template<class OWNER> class Am_Listbox : public Am_Listbox_ABC
{
	public:
		Am_Listbox();
		Am_Listbox(OWNER aOwner);

		virtual ~Am_Listbox();

		virtual void deleteAllItems();
		virtual void deleteFirstItem();
		virtual void deleteLastItem();

		Am_Object& getResult();

		virtual int		getNumberOfItems()
		{
			return( static_cast<Am_Value_List>(listbox.Get(Am_GRAPHICAL_PARTS)).Length() );
		};

		virtual void insertItemBack(std::string const &item);
		virtual void insertItemFront(std::string const &item);

		virtual void selectMethod();

		// Type cast operator!! Returns the Listbox Am_Object
		operator Am_Object&()
		{
			return(listbox);
		}
	protected:
	private:
		//:Called from the ctors of the class
		void	initialize();

		//:Stores a pointer to the owner object. This owner object, if specified,
		// is used to call the select method
		OWNER		*owner;

		//:This data member holds the result of a selection.
		Am_Object	result;

		//:Stores the position of the selected object in the Am_Value_List
		int			position;

		// Internal listbox objects
		Am_Object	listbox;
		Am_Object	listitem;
		Am_Object	listbox_interactor;
		Am_Object	listbox_interactor_command;
};

//------
// ctors
//------
template<class OWNER> Am_Listbox<OWNER>::Am_Listbox(void) : owner(0)
{
	// init the new object
	initialize();
}

template<class OWNER> Am_Listbox<OWNER>::Am_Listbox(OWNER aOwner) : owner(aOwner)
{
	// init the new object
	initialize();
}

//------
// dtor
//------
template<class OWNER> Am_Listbox<OWNER>::~Am_Listbox()
{
	listbox.Destroy();
	listitem.Destroy();
	listbox_interactor.Destroy();
	listbox_interactor_command.Destroy();
}

//----------------
// External Stuff
//----------------
EXTERN_SLOT(Am_THIS);
EXTERN_SLOT(Am_SELECTED_COLOR);
EXTERN_SLOT(Am_MULTIPLE_SELECTION);
EXTERN_SLOT(Am_LISTITEM);

EXTERN_METHOD(listbox_method);
EXTERN_FORMULA(selected_owner_color);
EXTERN_FORMULA(listbox_width);
EXTERN_FORMULA(listbox_hscrollbar);
EXTERN_FORMULA(listbox_vscrollbar);

//----------------
// deleteAllItems
//----------------
template<class OWNER> void Am_Listbox<OWNER>::deleteAllItems()
{
	// Get all the items
	Am_Value_List itemList = static_cast<Am_Value_List>(listbox.Get(Am_GRAPHICAL_PARTS));

	// 1st see if the list is valid
	if(itemList.Valid() == false)
	{
		// dbg("Am_Listbox::deleteAllItems",9,"Just returned.");
		// do nothing an return
		return;
	}

	// clear the list
	itemList.Make_Empty();

	// and write it back!
	listbox.Set(Am_GRAPHICAL_PARTS, itemList);

	return;
}

template<class OWNER> void Am_Listbox<OWNER>::deleteFirstItem()
{
	// Get all the items
	Am_Value_List itemList = static_cast<Am_Value_List>(listbox.Get(Am_GRAPHICAL_PARTS));

	// 1st see if the list is valid
	if(itemList.Valid() == false)
	{
		// dbg("Am_Listbox::deleteAllItems",9,"Just returned.");
		// do nothing an return
		return;
	}

	// clear the first element
	itemList.Start();
	itemList.Delete();

	// and write it back!
	listbox.Set(Am_GRAPHICAL_PARTS, itemList);

	return;
}

template<class OWNER> void Am_Listbox<OWNER>::deleteLastItem()
{
	// Get all the items
	Am_Value_List itemList = static_cast<Am_Value_List>(listbox.Get(Am_GRAPHICAL_PARTS));

	// 1st see if the list is valid
	if(itemList.Valid() == false)
	{
		// dbg("Am_Listbox::deleteAllItems",9,"Just returned.");
		// do nothing an return
		return;
	}

	// clear the last element
	itemList.End();
	itemList.Delete();

	// and write it back!
	listbox.Set(Am_GRAPHICAL_PARTS, itemList);

	return;
}

//-----------
// getResult
//-----------
template<class OWNER> Am_Object& Am_Listbox<OWNER>::getResult()
{
	// Use the list of all listitems from listbox object, and retrieve the object at 'position'
	Am_Value tmpValue = static_cast<Am_Value_List>(listbox.Get(Am_GRAPHICAL_PARTS)).Get_Nth(position);
	result = static_cast<Am_Object>(tmpValue).Get(Am_LISTITEM);

	return(result);
}

//------------
// initialize
//------------
template<class OWNER> void Am_Listbox<OWNER>::initialize()
{
	// listbox interactor command
	listbox_interactor_command = Am_Command.Create()
                 					.Set(Am_DO_METHOD, listbox_method);

	// listbox interactor; default start on "LEFT_DOWN"
	listbox_interactor = Am_One_Shot_Interactor.Create("listbox_interactor")
							.Set_Part(Am_COMMAND, listbox_interactor_command);

	// make a prototype of the listbox_container
	listbox = Am_Scrolling_Group.Create("listbox")
		.Add(Am_MULTIPLE_SELECTION, false)
		.Add(Am_SELECTED_COLOR, Am_Default_Color)
		.Add(Am_THIS, reinterpret_cast<Am_Ptr>(this))

		// can be used in constraints to trigger an update because
		// this Slot is toggeled on each selection, see 'listbox_method' above
		.Add(Am_SELECTED, false)

	    .Set(Am_FILL_STYLE, Am_Motif_Gray)
		.Set(Am_LAYOUT, Am_Vertical_Layout)
		.Set(Am_V_SPACING, 0)
		.Set(Am_INNER_WIDTH, Am_Width_Of_Parts)
		.Set(Am_INNER_HEIGHT, Am_Height_Of_Parts)
		.Set(Am_VISIBLE, Am_From_Owner(Am_VISIBLE)) // inherits the visible flag from the owner
		.Set(Am_WIDTH, listbox_width)
		.Set(Am_H_SCROLL_BAR, listbox_hscrollbar)
		.Set(Am_V_SCROLL_BAR, listbox_vscrollbar)
		.Set(Am_H_ALIGN, Am_LEFT_ALIGN)
		;

	// make a prototype of a listbox item
	listitem = Am_Group.Create("listitem_group")
				.Set(Am_WIDTH, Am_Width_Of_Parts)
				.Set(Am_HEIGHT, Am_Height_Of_Parts)
				.Add(Am_SELECTED, false)
				.Add_Part(Am_Rectangle.Create("listitem_rectangle")
							.Set(Am_WIDTH, Am_From_Sibling(Am_LISTITEM, Am_WIDTH))
							.Set(Am_HEIGHT,Am_From_Owner(Am_HEIGHT))
							.Set(Am_LINE_STYLE, selected_owner_color)							.Set(Am_FILL_STYLE, selected_owner_color)
							.Add_Part(listbox_interactor)
						 )
				.Add_Part(Am_LISTITEM, Am_Text.Create("listitem_text")
											.Set(Am_TEXT, "")
						 )
						 ;
}

//: inserts an item at the beginning into the listbox
template<class OWNER> void Am_Listbox<OWNER>::insertItemFront(std::string const &item)
{
	// create the listitem to be added
	Am_Object tmpItem = listitem.Create();

	// set the text
	tmpItem.Get_Object(Am_LISTITEM).Set(Am_TEXT, item.c_str());

	// add it to the listbox
	listbox.Add_Part(tmpItem);

	// get all the items for reordering
	Am_Value_List itemList = static_cast<Am_Value_List>(listbox.Get(Am_GRAPHICAL_PARTS));

	// make the last item the current one
	itemList.End();

	// and delete it
	// we have the original reference still available in form of the object
	// tmpItem
	itemList.Delete();

	// new re-add it at the head of the list
	itemList.Add(tmpItem, Am_HEAD);

	// and write it back the listbox group
	listbox.Set(Am_GRAPHICAL_PARTS, itemList);

	return;
}

//: inserts an item at the end into the listbox
template<class OWNER> void Am_Listbox<OWNER>::insertItemBack(std::string const &item)
{
	// create the listitem to be added
	Am_Object tmpItem = listitem.Create();

	// set the text
	tmpItem.Get_Object(Am_LISTITEM).Set(Am_TEXT, item.c_str());

	// add the text to the end of the listbox
	listbox.Add_Part(tmpItem);

	return;
}

//: We retrieve the values of the selected item(s) so they can be queried
template<class OWNER> void Am_Listbox<OWNER>::selectMethod()
{
	if( static_cast<bool>(listbox.Get(Am_MULTIPLE_SELECTION)) == true)
	{
		Am_Show_Alert_Dialog("Listbox multiple select-function not implemented yet!");
		return;
	}
	else
	{
		// Get list of all listitems from listbox object
		Am_Value_List 	list = listbox.Get(Am_GRAPHICAL_PARTS);

		// find the selected listitem
		position 	= 0;
		for(list.Start(); list.Last() == false; list.Next())
		{
			// check to see if we have found the selected item
			if(static_cast<bool>(static_cast<Am_Object>(list.Get()).Get(Am_SELECTED)) == true)
			{
				// that's it, so break out of the loop
				break;
			}

			// next item
			++position;
		}
	}

	// and call the listbox_select function of the owner
	if(owner != 0)
	{
		owner->listbox_select();
	}

	return;
}

//:Function object which is used in the 'for_each' algorithm to fill the listbox
template<class T> class fillListbox : public std::unary_function <T , void>
{
	public:
		// Ctor assigns the listbox-item to be used
		fillListbox(Am_Listbox_ABC *aListbox) : usedListbox(aListbox)
		{
			// and removes all actual elements from the listbox
			usedListbox->deleteAllItems();
		}

		// this function is called through the for_each STL aglortihm
 		void	operator()(T item)
 		{
			// get the reference to the string
			std::string	const &tmpString = item.first;

			// we use the first item of the pair
 			usedListbox->insertItemBack(tmpString);

 			return;
 		}

	private:
		fillListbox()
		{
			;
		}

		// pointer to the listbox to use
		Am_Listbox_ABC	*usedListbox;
};
#endif