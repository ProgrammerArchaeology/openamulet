#ifndef TYPES_SLOT_TYPES_H
#define TYPES_SLOT_TYPES_H

// Am_Slot_Key is used to name the slots of objects.
typedef unsigned short Am_Slot_Key;

typedef unsigned long Am_Slot_Flags;

enum Am_Inherit_Rule
{
  Am_LOCAL,
  Am_INHERIT,
  Am_COPY,
  Am_STATIC
};

#endif //TYPES_SLOT_TYPES_H
