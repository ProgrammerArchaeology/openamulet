#include <am_inc.h>
#include INITIALIZER__H
#include <amulet/impl/types_logging.h>
#include <amulet/impl/am_object.h>
#include <amulet/impl/inter_commands.h>

Am_Object Am_Defines_A_Collection_Command;
Am_Object Am_Select_Command;
Am_Object Am_Create_Command;
Am_Object Am_Object_Create_Command;
Am_Object Am_From_Other_Create_Command;

static void
init()
{
  Am_Defines_A_Collection_Command =
      Am_Command.Create(DSTR("Am_Defines_A_Collection_Command"));
  Am_Select_Command =
      Am_Defines_A_Collection_Command.Create(DSTR("Am_Select_Command"));
  Am_Create_Command =
      Am_Defines_A_Collection_Command.Create(DSTR("Am_Create_Command"));
  Am_Object_Create_Command =
      Am_Create_Command.Create(DSTR("Am_Object_Create_Command"));
  Am_From_Other_Create_Command =
      Am_Create_Command.Create(DSTR("Am_From_Other_Create_Command"));
}

static Am_Initializer *initializer =
    new Am_Initializer(DSTR("Other_Commands"), init, 3.12f);
