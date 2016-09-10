#include "amulet/impl/am_demon_queue.h"
#include "amulet/impl/method_point_in_or_self.h"

// Declarations and functions needed to coordinate opal.cc and windows.cc
extern Am_Demon_Queue Main_Demon_Queue;
extern void am_generic_renew_components(Am_Object object);
extern void am_generic_renew_copied_comp(Am_Object object);
extern void am_generic_add_part(Am_Object owner, Am_Object old_object,
			      Am_Object new_object);
extern Am_Point_In_Or_Self_Method am_group_point_in_part;
extern Am_Point_In_Or_Self_Method am_group_point_in_leaf;
