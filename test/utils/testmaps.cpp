/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

// Sample file. Using amulet maps, tables and iterators.
// See file univ_map.h for details.

#include <am_inc.h>

#include <amulet/am_io.h>
#include <amulet/univ_map.h>

using namespace std;

void test_custom_maps();
void test_symbol_table();

int
main()
{
  cout << "\n**Example 1a\n";

  Am_Map_Int2Str map;
  map[123] = "abc";
  map[347] = "cde";
  map[-678] = "abc";
  cout << map[123] << ' ' << map[347] << ' ' << map[-678];

  cout << "\n**Example 1b\n";

  Am_Map_Str2Int map1;
  map1["rst"] = 3456;
  map1["qwerty"] = -345;
  cout << map1["qwerty"] << ' ' << map1["rst"];
  if (!map1["xyz"])
    cout << "\nxyz: No such key";

  cout << "\n**Example 2\n";

  Am_MapIterator_Int2Str next(&map);
  char *s;
  for (s = next(); s; s = next())
    cout << next.Key() << "->" << s << endl; //prints out all pairs

  cout << "\n**Example 3a\n";

  Am_Table_Int2Str tbl;
  tbl.SetAt(123, "abc");
  tbl.SetAt("cde", 678);
  tbl.SetAt("abc", -34); //replaces the pair with "abc"
  cout << tbl[-34] << ' ' << tbl["abc"] << ' ' << tbl["cde"];
  if (!tbl[123])
    cout << "\n(123,abc) has been replaced";

  cout << "\n**Example 3b\n";

  //iterate on int key
  Am_MapIterator_Int2Str next3b(&tbl);
  for (s = next3b(); s; s = next3b())
    cout << next3b.Key() << "<->" << s << endl;

  cout << "\n**Example 3c\n";

  //iterate on char* key
  Am_MapIterator_Str2Int next3c(&tbl);
  for (int k = next3c(); k; k = next3c())
    cout << next3c.Key() << "<->" << k << endl;

  test_custom_maps();
  test_symbol_table();

  return 0;
}

#include <amulet/symbol_table.h>
void
test_symbol_table()
{
  cout << "\n**Example 5\n";

  Am_Symbol_Table sym_tbl;

  sym_tbl["Amulet"] = 123;
  sym_tbl["Garnet"] = 456;
  sym_tbl["Gem"] = 678;
  sym_tbl["Opal"] = 678;

  cout << sym_tbl["Amulet"] << ' ' << sym_tbl[123] << ' ' << sym_tbl["Gem"]
       << ' ' << sym_tbl[678] << endl;

  if (sym_tbl["object system"] == -1)
    cout << "We have neither \"object system\"";
  if (sym_tbl[999] == (0L))
    cout << " nor 999" << endl;

  Am_SymTable_Iterator sym_next(&sym_tbl);
  for (int v = sym_next(); sym_next.Key(); v = sym_next())
    cout << sym_next.Key() << "_->" << v << endl;

  cout << "Note: \"object system\" added by operator []!\n";
}

int HashValue(double key, int size);   // |
int KeyComp(double key1, double key2); // |
                                       //  > this can be in a .h file
AM_DECL_MAP(Dbl2Int, double, int)      // |
AM_DECL_MAP(Int2Dbl, int, double)      // |

AM_IMPL_MAP(Int2Dbl, int, 0, double, 0.0)

#include <limits.h> //for INT_MAX

int
HashValue(double key, int size)
//returns hash value for key. 0 <= hash value < size.
{
  if (key < 0.0)
    key = -key;
  else if (key == 0.0)
    return 0;

  if (key > 1.0)
    while (key >= (double)INT_MAX)
      key /= (double)INT_MAX;
  else
    while (key < 1.0)
      key *= (double)INT_MAX;
  return (int)key % size;
}

int
KeyComp(double key1, double key2)
// returned value < 0 if key1 < key2, =0 if key1 = key2, >0 otherwise.
{
  return (key1 > key2) ? 1 : (key1 == key2) ? 0 : -1;
}

AM_IMPL_MAP(Dbl2Int, double, 0.0, int, 0)

AM_DECL_TABLE(Int2Dbl, Dbl2Int, int, double) // this can be in a .h file

void
test_custom_maps()
{
  cout << "\n**Example 4a\n";

  Am_Map_Dbl2Int map;
  map[-3.897e-2] = 345;
  map[3.456e10] = 578;
  map[-3.456e10] = -890;

  int i;
  Am_MapIterator_Dbl2Int next(&map);
  for (i = next(); i; i = next())
    cout << next.Key() << "->" << i << endl;

  cout << "\n**Example 4b\n";

  Am_Table_Int2Dbl tbl;

  next.Reset(); //reset iterator
  for (i = next(); i; i = next())
    tbl.SetAt(next.Key(), i);

  next.Init(&tbl); //reuse iterator 'next' for 'tbl'
  for (i = next(); i; i = next())
    cout << next.Key() << "<->" << i << endl;
}
