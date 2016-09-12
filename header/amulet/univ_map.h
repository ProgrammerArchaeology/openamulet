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

// This file declares maps, tables and iterators for several data types
// as well as template macros for custom maps, tables and iterators.
// *************************************************************************
// Each map is provided by a class Am_Map_<Name> and is
// a hash table of (<KeyType> key , <ValueType> value) pairs, where
// key is unic for the map.
//
// Member functions:
//
// Am_Map_<Name> (int size = 53) - constructs a map.
//
// void SetAt (<KeyType> key, <ValueType> key2) - adds new pair.
//   The existing pair with key is deleted.
//
// <ValueType>& operator [] (<KeyType> key) - returns a value for this key.
//   Adds a new pair with this key and null value if there is no such key.
//   NOTE: To avoid this effect when quering the map use function GetAt.
//
// <ValueType> GetAt (<KeyType> key) const - returns a value for this key.
//   Returns null value if the key is not found.
//
// void DeleteKey (<KeyType> key) - deletes a pair with this key.
//
// bool IsEmpty () const - returns true if the map is empty, otherwise false.
//
// int Count () const - returns number of pairs in the map.
//
// int Size () const - returns size of the map.
//
// void Resize (int size) - changes the size of the map.
//   Preserves all the existing pairs.
//
// void Clear () - deletes all pairs from the table
//
// Example:
//
//	Am_Map_Int2Str map;
//	map[123] = "abc";
//	map[347] = "cde";
//	map[-678] = "abc";
//std::cout << map[123] << map[347] << map[-678];
//
//	Am_Map_Str2Int map1;
//	map1["rst"] = 3456;
//	map1["qwerty"] = -345;
//std::cout << map1["qwerty"] << map1["rst"];
//	if (!map1["xyz"])std::cout << "No such element";
//
// *************************************************************************
// For each map class Am_Map_<Name> an iterator is provided by a class
// Am_MapIterator_<Name>.
//
// Member functions:
//
// Am_MapIterator_<Name> (Am_Map_<Name>* map) - constructs an iterator
//  for this map
//
// <ItemType> operator () () - return the next item
//
// <KeyType> Key () const - returns the key for the current item
//
// void Reset () - resets the iterator
//
// void Init (const Am_Map_<Name>* map) - initializes the iterator to work
//   with another map (otherwise no need to call this function).
//
// Example:
//
//	Am_Map_Int2Str map;
//	map[123] = "abc";
//	map[347] = "cde";
//	map[-678] = "abc";
//
//	Am_MapIterator_Int2Str next(&map);
//	for (char* s = next(); s; s = next())
//	 std::cout << s << next.Key();  //prints out all pairs
//
// *************************************************************************
// Each table is provided by a class Am_Map_<Name> and is
// a double hash table of (<KeyType1> key1 , <KeyType2> key2) pairs, where
// both key1 & key2 are unic for the table.
//
// Member functions:
//
// Am_Table_<Name> (int size = 89) - constructs a table.
//
// void SetAt (<KeyType1> key1, <KeyType2> key2) - adds new pair.
//   The same as
// void SetAt (<KeyType2> key1, <KeyType1> key2) - adds new pair.
//   The existing pairs with key1 or key2 are deleted.
//
// <KeyType2> operator [] (<KeyType1> key) const -
//   returns a value for this key.
//   Returns (0L) if there is no such key.
//
// <KeyType1> operator [] (<KeyType2> key) const -
//   returns a char* value for this int key.
//   Returns (0L) if there is no such key.
//
// void DeleteKey (<KeyType1> key) - deletes a pair with this key.
// void DeleteKey (<KeyType2> key) - deletes a pair with this key.
//
// bool IsEmpty () const - returns true if the table is empty, otherwise false.
//
// int Count () const - returns number of pairs in the table.
//
// int Size () const - returns size of the table.
//
// void Resize (int size) - changes the size of the table.
//   Preservs all the existing pairs.
//
// void Clear () - deletes all pairs from the table
//
// Example.
//
//	Am_Table_Int2Str tbl;
//	tbl.SetAt(123, "abc");
//	tbl.SetAt("cde", 678);
//	tbl.SetAt("abc", 123); //replaces the pair with "abc"
//std::cout << tbl[-34] << tbl["abc"] << tbl["cde"];
//	if (!tbl[123])std::cout << "(123,abc) has been replaced";
//
// You can use an iterator for any of the both key types. For example:
//
//	Am_Table_Int2Str tbl;
//
//	//...add pairs
//
//	//iterate on int key
//	Am_MapIterator_Int2Str next(&tbl);
//	for (char* s = next(); s; s = next())std::cout << s;
//
//	//iterate on char* key
//	Am_MapIterator_Str2Int next1(&tbl);
//	for (int k = next1(); k; k = next1())std::cout << k;
//
// *************************************************************************
//   Creating your own maps and tables.
//
// File univ_map.h defines the following maps (with iterators) and tables:
//	Am_Map_Int2Ptr	-- map int -> void*
//	Am_Map_Int2Str	-- map int -> char*
//	Am_Map_Ptr2Int	-- map void* -> int
//	Am_Map_Str2Int	-- map char* -> int
//	Am_Table_Int2Str -- table int <-> char*
//
// Using template macros provided by univ_map.h you can define maps and
// tables for other types. To do this, you don't need to write much code.
//
// Example. Defining map int -> double.
//
// **file my_map_int2dbl.h
//	#include <amulet/univ_map.h>
//	AM_DECL_MAP(Int2Dbl, int, double) //Name suffix, key type, value type
//
// **file my_map_int2dbl.cc
//	#include "my_map_int2dbl.h"
//	AM_IMPL_MAP(Int2Dbl, int, 0, double, 0.0)
//		//Name suffix, key type, null key, value type, null value
//
// That was it. You get the new map as class Am_Map_Int2Dbl and the matching
// iterator as class Am_MapIterator_Int2Dbl.
//
// If you use a type other then int, [const] void* and [const] char* as a key
// type, you have to define two functions - HashValue and KeyComp for the new
// key type.
//
// Example. Defining map double -> int.
//
// **file my_map_dbl2int.h
//	#include "univ_map.h"
//	int HashValue (double key, int size);
//	int KeyComp (double key1, double key2);
//	AM_DECL_MAP(Dbl2Int, double, int)
//
// **file my_map_dbl2int.cc
//	#include "my_map_dbl2int.h"
//
//	int HashValue (double key, int size)
//	//returns hash value for key. 0 <= hash value < size.
//	{
//	  //... //see file test_map.cc
//	}
//	int KeyComp (double key1, double key2)
//	// returned value < 0 if key1 < key2, =0 if key1 = key2, >0 otherwise.
//	{
//	  return (key1 > key2)? 1 : (key1 == key2)? 0 : -1;
//	}
//
//	AM_IMPL_MAP(Dbl2Int, double, 0.0, int, 0)
//
// Having these two maps, you can define the corresponding table.
//
// Example. Defining table int <-> double.
//
// **file my_tbl_int2dbl.h
//	#include "my_map_int2dbl.h"
//	#include "my_map_dbl2int.h"
//	AM_DECL_TABLE(Int2Dbl, Dbl2Int, int, double)
//
// No .cc file is needed. You get class Am_Table_Int2Dbl.
//
// See file test_map.cc for the working examples.

#ifndef UNIV_MAP_H
#define UNIV_MAP_H

#include <string.h>

#include <stdlib.h>

#include UNIV_MAP__TPL

// Hash value fuctions
inline int
HashValue(int key, int size)
{
  return (abs(key % size));
}

inline int
HashValue(const void *key, int size)
{
  return (abs(static_cast<int>(reinterpret_cast<uintptr_t>(key) % size)));
}

int HashValue(const char *key, int size);

// Key comparison functions
inline int
KeyComp(int key1, int key2)
{
  return (key1 - key2);
}

inline int
lsign(long l)
{
  return (l > 0L) ? 1 : (l < 0L) ? -1 : 0;
}

inline int
KeyComp(const void *key1, const void *key2)
{
  return (lsign((uintptr_t)key1 - (uintptr_t)key2));
}

inline int
KeyComp(const char *key1, const char *key2)
{
  return (strcmp(key1, key2));
}

#endif
