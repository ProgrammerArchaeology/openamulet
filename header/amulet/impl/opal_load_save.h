#include "types_load_save.h"
#include "types_method.h"

Am_Define_Method_Type (Am_Save_Object_Method, void,
		       (std::ostream& os, Am_Load_Save_Context& context,
			const Am_Object& object))

_OA_DL_IMPORT extern Am_Load_Save_Context Am_Default_Load_Save_Context;

//Am_Standard_Save_Object can be put in the Am_SAVE_OBJECT_METHOD of an object.
//It expects the object or its immediate prototype to have been
//registered with context.Register_Prototype.  The object or its
//prototype must have a list of slot keys in the Am_SLOTS_TO_SAVE
//slot.  These slots are saved.  The load method for
//Am_Standard_Save_Object is automatically registered with every context.
_OA_DL_IMPORT extern Am_Save_Object_Method Am_Standard_Save_Object;

//Am_Standard_Load_Object is automatically registered for loading the
//results of Am_Standard_Save_Object in the
//Am_Default_Load_Save_Context, but you might need to register it if
//you create a DIFFERENT context from the default.
_OA_DL_IMPORT extern Am_Load_Method Am_Standard_Load_Object;

//A special object for transforming one kind of saved object into another.
//Register an instance of this object as the prototype of an object saved
//with standard save.  Set the slot Am_ITEM_PROTOTYPE with the real prototype
//and the slot Am_SLOTS_TO_SAVE with the slots to load.  Set the slot
//Am_ITEM_METHOD with an Am_Object_Method that will be called after object is
//loaded (optional).
_OA_DL_IMPORT extern Am_Object Am_Load_Object_Swap;

//use Am_Standard_Save_Group instead of Am_Standard_Save_Object for
//groups (so the parts will be saved).  For example, you should
//register Am_Resize_Parts_Group if you have grouping cmds in the edit menu.
_OA_DL_IMPORT extern Am_Save_Object_Method Am_Standard_Save_Group;
_OA_DL_IMPORT extern Am_Load_Method Am_Standard_Load_Group;
