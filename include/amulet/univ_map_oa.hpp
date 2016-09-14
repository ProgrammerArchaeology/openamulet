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

#ifndef UNIV_MAP_OA_HPP
#define UNIV_MAP_OA_HPP

#include <string.h>

#include <map>
#include <iostream>
#include <amulet/impl/types_logging.h> // Debug logging functions

// we provide a less function which can be used by the Map_Str2Int (const char*) specialization,
// so that keys are compared as strings and not as pointers
template <class T> struct lex_less : public std::binary_function<T, T, bool>
{
  bool operator()(const T &x, const T &y) const
  {
    return (std::lexicographical_compare(x, x + strlen(x), y, y + strlen(y)));
  }
};

// To avoid nameclashes, we place all OpenAmulet related datastructures
// within an own namespace
namespace OpenAmulet {

// This is mainly a wrapper class to std::map<...> which provides the
// same interface as the 'old' Am_Map_##Name class. This class avoids that
// we have to use macros to implement the needed maps
template <class Key, class Item, class Compare = std::less<Key>> class Map
{
public:
  // TYPEDEFS
  typedef std::map<Key, Item, Compare> MAP;
  typedef typename std::map<Key, Item, Compare>::const_iterator MAP_ITERATOR;

  // used by iterator class
  typedef Item ITEM;
  typedef Key KEY;

  // CREATORS
  Map() { ; }

  // MANIPULATORS
  //:Returns the element with the given key, if not found
  // a new entry/item is created in the map
  Item &operator[](const Key &aKey) { return (mMap[aKey]); }

  void Clear()
  {
    mMap.erase(mMap.begin(), mMap.end());
    return;
  }

  void DeleteKey(const Key &aKey)
  {
    typename MAP::size_type numberOfElementsDeleted = mMap.erase(aKey);
#ifdef DEBUG
    if (numberOfElementsDeleted > 1) {
      DERR(
          "Attention! " << numberOfElementsDeleted
                        << " elements deleted from map for one given key-value:"
                        << aKey << std::endl);
    }
#else
    (void)numberOfElementsDeleted;
#endif
    return;
  }

  // we don't use references as we have to ensure the parameter values
  // will exist as long as this map exists
  void SetAt(const Key aKey, Item aItem)
  {
    mMap[aKey] = aItem;
    return;
  }

  // we don't use references as we have to ensure the parameter values
  // will exist as long as this map exists
  void SetDefaultReturnValue(const Item aValue)
  {
    mDefaultReturn = aValue;
    return;
  }

  void Resize(int aSize);

  // ACCESSORS
  long Count() const { return (mMap.size()); }

  Map<Key, Item, Compare> *Copy()
  {
    Map<Key, Item, Compare> *newMap = new Map<Key, Item, Compare>(*this);
    return (newMap);
  }

#ifdef DEBUG
  // To use this function, the Iey and Item objects must have an operator << const
  // definition which will be used to dump the information to std::cout
  void Dump() const
  {
    for (MAP_ITERATOR it = mMap.begin(); it != mMap.end(); ++it) {
      std::cout << "Key: " << (*it).first << " Value: " << (*it).second
                << std::endl;
    }

    return;
  }
#endif

  Item GetAt(const Key &aKey) const
  {
#ifdef DEBUG
    // This is for debugging propose only. If you want to look at the
    // map change the dumpMap value to true within your debugger
    bool dumpMap(false);
    if (dumpMap == true) {
      Dump();
    }
#endif

    // do we have an item with aKey?
    typename std::map<Key, Item, Compare>::const_iterator it = mMap.find(aKey);
    if (it != mMap.end()) {
      return ((*it).second);
    } else {
      // the key wasn't found so return the default object value
      return (mDefaultReturn);
    }
  }

  bool IsEmpty() const { return (mMap.empty()); }

  long Size() const { return (mMap.max_size()); }

  const MAP &getMap() const { return (mMap); }

  // DATA MEMBERS

protected:
  // CREATORS
  // MANIPULATORS
  // ACCESSORS
  // DATA MEMBERS

private:
  // CREATORS
  // MANIPULATORS
  // ACCESSORS
  // DATA MEMBERS
  MAP mMap;

  // the default return value
  Item mDefaultReturn;
};

// As Amulet and still OpenAmulet uses a non STL conforming interface for iterators, we
// provide this wrapper class. It's initialized with a reference to the map object
// the iterator should work with
template <class T> class Map_Iterator
{
public:
  // CREATORS
  // Default Ctor, use this if it's not possible to set the
  // associated map object directly, because of template code
  // use function Init(...) instead
  Map_Iterator() { ; }

  Map_Iterator(T &aMap) : mMap(aMap), mMapIterator(aMap.getMap().begin()) { ; }

  Map_Iterator(const T &aMap) : mMap(aMap), mMapIterator(aMap.getMap().begin())
  {
    ;
  }

  // MANIPULATORS
  void Init(const T &aMap)
  {
    mMap = aMap;
    mMapIterator = mMap.begin();
  }

  void Reset()
  {
    mMapIterator = mMap.getMap().begin();
    return;
  }

  // ACCESSORS
  typename T::ITEM operator()()
  {
    if (mMapIterator != mMap.getMap().end()) {
      // we first get the item and than advance the iterator
      mCurrentItem = mMapIterator;

      // ATTENTION: We change the iterator, so after call to this
      // function, the iterator will point to an other object
      ++mMapIterator;

      return ((*mCurrentItem).second);
    } else {
      mCurrentItem = mMap.getMap().end();
      return ((typename T::ITEM)0);
    }
  }

  typename T::KEY Key() const
  {
    if (mCurrentItem != mMap.getMap().end()) {
      return ((*mCurrentItem).first);
    } else {
      return ((typename T::KEY)0);
    }
  }

  // DATA MEMBERS

protected:
  // CREATORS
  // MANIPULATORS
  // ACCESSORS
  // DATA MEMBERS

private:
  // CREATORS
  // MANIPULATORS
  // ACCESSORS
  // DATA MEMBERS
  const T &mMap;
  typename T::MAP_ITERATOR mCurrentItem;
  typename T::MAP_ITERATOR mMapIterator;
};

// Here are the specializations used by OpenAmulet
// we provide the same names used by Amulet as typedefs so there
// won't be any differences in the usage, the only one is that
// the OpenAmulet namespace has to be specified
typedef Map<int, const char *> Map_Int2Str;
typedef Map<int, void *> Map_Int2Ptr;
typedef Map<void *, int> Map_Ptr2Int;
typedef Map<int, int> Map_Int2Int;
typedef Map<const char *, int, lex_less<const char *>> Map_Str2Int;

typedef Map_Iterator<Map_Int2Str> Map_Iterator_Int2Str;
typedef Map_Iterator<Map_Int2Ptr> Map_Iterator_Int2Ptr;
typedef Map_Iterator<Map_Ptr2Int> Map_Iterator_Ptr2Int;
typedef Map_Iterator<Map_Str2Int> Map_Iterator_Str2Int;
typedef Map_Iterator<Map_Int2Int> Map_Iterator_Int2Int;

// General Table class. Tables support look-up in both directions, from key to item and from
// item to key. For this a table contains two maps.
template <class Key, class Item, class KeyCompare = std::less<Key>,
          class ItemCompare = std::less<Item>>
class Table
{
  typedef OpenAmulet::Map<Key, Item, KeyCompare> Map_Key2Item;
  typedef OpenAmulet::Map_Iterator<Map_Key2Item> MapIterator_Key2Item;

  typedef OpenAmulet::Map<Item, Key, ItemCompare> Map_Item2Key;
  typedef OpenAmulet::Map_Iterator<Map_Item2Key> MapIterator_Item2Key;

public:
  // OpenAmulet doesn't use inheritance here
  Table()
      : Am_MapIterator_Key2Item(Am_Map_Key2Item),
        Am_MapIterator_Item2Key(Am_Map_Item2Key)
  {
    ;
  }

  // we don't use references as we have to ensure the parameter values
  // will exist as long as this map exists
  Table(Key aNullKey, Item aNullItem)
      : Am_MapIterator_Key2Item(Am_Map_Key2Item),
        Am_MapIterator_Item2Key(Am_Map_Item2Key)
  {
    Am_Map_Key2Item.SetDefaultReturnValue(aNullItem);
    Am_Map_Item2Key.SetDefaultReturnValue(aNullKey);
  }

  // we can use references here because we use the Map class
  // which ensures that the key,item will be copied. This will avoid
  // that to much temporary objects are created.
  void SetAt(const Item &aValue, const Key &aKey)
  {
    Am_Map_Key2Item.SetAt(aKey, aValue);
    Am_Map_Item2Key.SetAt(aValue, aKey);
  }

  // we can use references here because we use the Map class
  // which ensures that the key,item will be copied. This will avoid
  // that to much temporary objects are created.
  void SetAt(const Key &aKey, const Item &aValue)
  {
    Am_Map_Key2Item.SetAt(aKey, aValue);
    Am_Map_Item2Key.SetAt(aValue, aKey);
  }

  const Item GetAt(Key &aKey) const { return (Am_Map_Key2Item.GetAt(aKey)); }

  // reverse look up
  const Key GetAt(Item &aValue) const
  {
    return (Am_Map_Item2Key.GetAt(aValue));
  }

  // returns the value associated with the key.  If the key is not
  // found, then aNullItem is returned
  const Item operator[](Key &aKey) const
  {
    return (Am_Map_Key2Item.GetAt(aKey));
  }

  const Key operator[](Item &aValue) const
  {
    return (Am_Map_Item2Key.GetAt(aValue));
  }

  // removes the entry in the symbol table associated with that value
  void DeleteKey(Key &aKey)
  {
    // first find the key-value
    Item item = Am_Map_Key2Item.GetAt(aKey);

    // than remove the key
    Am_Map_Key2Item.DeleteKey(aKey);

    // it can happen that one or more keys are having
    // the same item, so only delete the item too if
    // this is really the key associated in the opposite
    // direction too.
    if (aKey == Am_Map_Item2Key.GetAt(item)) {
      Am_Map_Item2Key.DeleteKey(item);
    }
  };

  void DeleteKey(Item &aItem)
  {
    // first find the item-key
    Key key = Am_Map_Item2Key.GetAt(aItem);

    // than remove both keys from the two maps
    Am_Map_Item2Key.DeleteKey(aItem);

    // it can happen that one or more keys are having
    // the same item, so only delete the item too if
    // this is really the key associated in the opposite
    // direction too.
    if (aItem == Am_Map_Key2Item.GetAt(key)) {
      Am_Map_Key2Item.DeleteKey(key);
    }
  };

  const std::pair<long, long> Current_Size() const
  {
    return (std::pair<long, long>(Am_Map_Key2Item.Count(),
                                  Am_Map_Item2Key.Count()));
  }

  int Max_Size() const { return (Am_Map_Key2Item.Size()); }

  // ATTENTION: Keep this order between Am_Map_* and Am_MapIterator_* because
  // the iterators are initialized in the initializer-list of the ctors and
  // need an already constructed map object, the code will fail if the map
  // isn't properly constructed at the time the iterator ctors are invoked
private:
  // This maps key-strings to key-values
  Map_Key2Item Am_Map_Key2Item;

  // This maps key-values to key-strings
  Map_Item2Key Am_Map_Item2Key;

public:
  MapIterator_Key2Item Am_MapIterator_Key2Item;
  MapIterator_Item2Key Am_MapIterator_Item2Key;
};

} // namespace OpenAmulet
#endif
