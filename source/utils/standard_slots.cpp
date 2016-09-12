/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <stdlib.h>

#include <am_inc.h>

#include <amulet/am_io.h>

#include <amulet/impl/slots_registry.h>
#include <amulet/standard_slots.h>
#include <amulet/symbol_table.h>

//#include <amulet/types.h>

#define SLOT_NAME_START Am_MAXIMUM_USER_SLOT_KEY + 1
// == 30000.

// exported by symbol_table.h
Am_Symbol_Table *Am_Slot_Name_Key_Table = 0;

inline void
verify_slot_name_key_table()
{
  // do we allready have a symbol table?
  if (!Am_Slot_Name_Key_Table) {
    // if not we allocate one
    Am_Slot_Name_Key_Table = Am_Symbol_Table::Create(500);
  }

  return;
}

void
Am_Register_Slot_Key(Am_Slot_Key key, const char *string)
{
  verify_slot_name_key_table();
  int prev_key = Am_Slot_Name_Key_Table->Get_Value(string);
  if (prev_key != -1) {
    std::cerr << "Slot Name, " << string << ", has been overwritten."
              << std::endl;
    Am_Error();
  }
  const char *prev_name = Am_Slot_Name_Key_Table->Get_Key(key);
  if (prev_name) {
    std::cerr << "Slot Key, ";
    Am_Print_Key(std::cerr, key);
    std::cerr << ", has already been used." << std::endl;
    Am_Error();
  }
  Am_Slot_Name_Key_Table->Add_Item(key, string);
}

Am_Slot_Key
Am_Register_Slot_Name(const char *string)
{
  // current_name stores the next slot key to allocate
  static Am_Slot_Key current_name = SLOT_NAME_START;

  // do we allready have a symbol-table?
  verify_slot_name_key_table();

  int prev_key = Am_Slot_Name_Key_Table->Get_Value(string);
  if (prev_key == -1) {
    Am_Slot_Name_Key_Table->Add_Item(current_name, string);

    // do we have an overflow?
    if (current_name + 1 < SLOT_NAME_START) {
      Am_Error("** Am_Register_Slot_Name: too many slot names registered!\n");
    }

    return (current_name++);
  } else {
    return ((Am_Slot_Key)prev_key);
  }
}

// Returns 0 if slot name not found
Am_Slot_Key
Am_From_Slot_Name(const char *string)
{
  verify_slot_name_key_table();
  int prev_key = Am_Slot_Name_Key_Table->Get_Value(string);
  if (prev_key == -1)
    return 0;
  else
    return (Am_Slot_Key)prev_key;
}

const char *
Am_Get_Slot_Name(Am_Slot_Key key)
{
  verify_slot_name_key_table();
  return Am_Slot_Name_Key_Table->Get_Key(key);
}

bool
Am_Slot_Name_Exists(const char *string)
{
  verify_slot_name_key_table();
  return (Am_Slot_Name_Key_Table->Get_Value(string) != -1);
}
