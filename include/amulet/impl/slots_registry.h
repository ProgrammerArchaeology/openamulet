#include "types_slot.h"

// Registered Slot Range (30000-65535)

// Register a specific key for the slot name.  Allows explicit assignment
// of name/key pairs.
_OA_DL_IMPORT extern void Am_Register_Slot_Key(Am_Slot_Key key,
                                               const char *string);

// Generates a new slot key given a string for a slot name.  Returns the same
// key for a given name.
_OA_DL_IMPORT extern Am_Slot_Key Am_Register_Slot_Name(const char *string);

// Returns the name of a slot key.  Returns (nullptr) if the key is not defined.
_OA_DL_IMPORT extern const char *Am_Get_Slot_Name(Am_Slot_Key key);

// Returns 0 if slot name not found
_OA_DL_IMPORT extern Am_Slot_Key Am_From_Slot_Name(const char *string);

// Returns true if the slot name has been assigned a key.
// Returns false, otherwise
_OA_DL_IMPORT extern bool Am_Slot_Name_Exists(const char *string);
