//
// ------------------------------------------------------------------------
// 		The OpenAmulet User Interface Development Environment
// ------------------------------------------------------------------------
// This code is based on the Amulet project of Carnegie Mellon University,
// and has been placed in the public domain.  If you are using this code
// or any part of OpenAmulet, please contact amulecs.cmu.edu to be put
// on the mailing list or have a look at:
// http://www.openip.org
//

// This file describes a simple symbol_table class that maps strings
// to ints and vice-versa.
//
// Designed and implemented by Brad Myers

#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H
#include <cassert>

#ifdef MEMORY
#include <amulet/dynarray.h>
#endif

#include <am_inc.h>
#include <amulet/am_value.hpp>

// if we use OpenAmulet, we use the much fast map implementation
#ifdef OA_VERSION
#include <amulet/univ_map_oa.hpp>
class Am_Symbol_Table
#else
#include UNIV_MAP__H
AM_DECL_MAP(CStr2Int, const char *, int)
class Am_Symbol_Table : public Am_Map_CStr2Int
#endif
{
public:
// OpenAmulet doesn't use inheritance here
#ifdef OA_VERSION
  Am_Symbol_Table(int initial_size = 89)
      : Am_MapIterator_CStr2Int(Am_Map_CStr2Int),
        Am_MapIterator_CInt2Str(Am_Map_CInt2Str)
  {
    Am_Map_CStr2Int.SetDefaultReturnValue(-1);
    Am_Map_CInt2Str.SetDefaultReturnValue(0);
  }
#else
  Am_Symbol_Table(int initial_size = 89) : Am_Map_CStr2Int(initial_size) { ; }
#endif

  static Am_Symbol_Table *Create(int initial_size = 89)
  {
    // uses our own memory pool if enabled
    return (new Am_Symbol_Table(initial_size));
  };

  void Add_Item(int value, const char *key)
  {
#ifdef OA_VERSION
    Am_Map_CStr2Int.SetAt(key, value);
    Am_Map_CInt2Str.SetAt(value, key);
#else
    SetAt(key, value);
#endif
  };

  void Add_Item(const char *key, int value)
  {
#ifdef OA_VERSION
    Am_Map_CStr2Int.SetAt(key, value);
    Am_Map_CInt2Str.SetAt(value, key);
#else
    SetAt(key, value);
#endif
  };

  // adds the (key value) pair to the symbol table.  If key is already
  // there, then the old value associated with key is replaced with
  // the new value.  More than one key can map to the same value.
  // Key is case significant.
  // For the non-OA Version you can also use operator[]:    my_sym_tbl[key] = value;
  int Get_Value(const char *key)
  {
#ifdef OA_VERSION
    return (Am_Map_CStr2Int.GetAt(key));
#else
    return GetAt(key);
#endif
  };

#ifdef OA_VERSION
  // returns the value associated with the key.  If the key is not
  // found, then -1 is returned
  int operator[](const char *key) { return (Am_Map_CStr2Int.GetAt(key)); };
#endif

  const char *Get_Key(int value) const { return operator[](value); };

  // returns the key associated with that value.  If more than one
  // key maps to the same value, then the LAST ADDED key is returned.
  // If not found, returns 0 (null pointer)
  const char *operator[](int value) const
  {
#ifdef OA_VERSION
    return (Am_Map_CInt2Str.GetAt(value));
#else
    Am_SymTable_Iterator next(this);
    for (int i = next(); next.Key() && (i != value); i = next()) {
      ;
    }
    return (next.Key());
#endif
  }

  // removes the entry in the symbol table associated with that value
  void Remove_Item(char *key)
  {
#ifdef OA_VERSION
    // first find the key-value
    int keyValue = Am_Map_CStr2Int.GetAt(key);

    // than remove both keys from the two maps
    Am_Map_CStr2Int.DeleteKey(key);
    Am_Map_CInt2Str.DeleteKey(keyValue);
#else
    DeleteKey(key);
#endif
  };

  int Current_Size() const
  {
#ifdef OA_VERSION
    // the two maps should contain the same number of elements
    // if not something must be wrong
    assert(Am_Map_CStr2Int.Count() == Am_Map_CInt2Str.Count());
    return (Am_Map_CStr2Int.Count());
#else
    return Count();
#endif
  }

  int Max_Size() const
  {
#ifdef OA_VERSION
    return (Am_Map_CStr2Int.Size());
#else
    return Size();
#endif
  }

#ifdef OA_VERSION
  operator OpenAmulet::Map_Str2Int &() { return (Am_Map_CStr2Int); }

  operator OpenAmulet::Map_Int2Str &() { return (Am_Map_CInt2Str); }

private:
  // This maps key-strings to key-values
  OpenAmulet::Map_Str2Int Am_Map_CStr2Int;
  OpenAmulet::Map_Iterator_Str2Int Am_MapIterator_CStr2Int;

  // This maps key-values to key-strings
  OpenAmulet::Map_Int2Str Am_Map_CInt2Str;
  OpenAmulet::Map_Iterator_Int2Str Am_MapIterator_CInt2Str;

#endif
};

#ifdef OA_VERSION
typedef OpenAmulet::Map_Iterator_Str2Int Am_SymTable_Iterator_Str2Int;
typedef OpenAmulet::Map_Iterator_Int2Str Am_SymTable_Iterator_Int2Str;
#endif

//the main symbol table for slots
extern Am_Symbol_Table *Am_Slot_Name_Key_Table;

#endif
