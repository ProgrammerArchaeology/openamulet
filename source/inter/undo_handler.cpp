#include <am_inc.h>
#include INITIALIZER__H
#include <amulet/impl/types_logging.h>
#include <amulet/impl/am_object.h>
#include <amulet/impl/inter_commands.h>
#include <amulet/impl/opal_objects.h>
#include <amulet/impl/slots.h>

Am_Object Am_Undo_Handler; // general, prototype undo handler obj

static void init()
{
  Am_Undo_Handler = Am_Root_Object.Create (DSTR("Am_Undo_Handler"))
    .Add (Am_REGISTER_COMMAND, (0L))
    .Add (Am_PERFORM_UNDO, (0L))
    .Add (Am_PERFORM_REDO, (0L))
    .Add (Am_UNDO_ALLOWED, (0L))
    .Add (Am_REDO_ALLOWED, (0L))
    .Add (Am_COMMAND, 0)

    .Add (Am_SELECTIVE_UNDO_METHOD, (0L))
    .Add (Am_SELECTIVE_REPEAT_SAME_METHOD, (0L))
    .Add (Am_SELECTIVE_REPEAT_ON_NEW_METHOD, (0L))

    .Add (Am_SELECTIVE_UNDO_ALLOWED, (0L))
    .Add (Am_SELECTIVE_REPEAT_SAME_ALLOWED, (0L))
    .Add (Am_SELECTIVE_REPEAT_NEW_ALLOWED, (0L))
    ;
}

static Am_Initializer* initializer = new Am_Initializer(DSTR("Am_Undo_Handler"), init, 3.13f);
