#include <am_inc.h>

#include INITIALIZER__H
#include <amulet/impl/gem_flags.h>
#include <amulet/impl/am_object.h>
#include <amulet/impl/object_misc.h>
#include <amulet/impl/inter_user_id.h>
#include <amulet/impl/slots.h>
#include <amulet/impl/opal_objects.h>

Am_Object Am_User_Id;


static void init()
{
	Am_User_Id = Am_Root_Object.Create(DSTR("Am_User_Id"))
		.Add (Am_USER_ID, Am_USER_ZERO);              // supports multiple users
}

static Am_Initializer* initializer = new Am_Initializer(DSTR("Am_User_Id"), init, 3.2f, 0);
