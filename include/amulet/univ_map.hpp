/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

// Template macros for Amulet maps, iterators and tables

#include <stdlib.h>

#define AM_DECL_MAP(Name, KeyType, ItemType)                                   \
  struct Am_Assoc_##Name                                                       \
  {                                                                            \
    KeyType m_key;                                                             \
    ItemType m_item;                                                           \
    Am_Assoc_##Name *m_next;                                                   \
    /**/                                                                       \
    Am_Assoc_##Name(KeyType key, ItemType item)                                \
        : m_key(key), m_item(item), m_next(nullptr){};                            \
  };                                                                           \
  /**/                                                                         \
  /**/                                                                         \
  class Am_Map_##Name                                                          \
  {                                                                            \
    friend class Am_MapIterator_##Name;                                        \
                                                                               \
  public:                                                                      \
    Am_Map_##Name(int size = 53);                                              \
    virtual ~Am_Map_##Name();                                                  \
    void SetAt(KeyType key, ItemType item);                                    \
    ItemType GetAt(KeyType key) const;                                         \
    void DeleteKey(KeyType key);                                               \
    ItemType &operator[](KeyType key);                                         \
    /*always creates new (nullptr) item if the key isn't yet in map;*/              \
    /*doesn't call FreeItem if the item is being changed*/                     \
    bool IsEmpty() const { return m_count == 0; };                             \
    int Count() const { return m_count; };                                     \
    int Size() const { return m_size; };                                       \
    void Resize(int size);                                                     \
    Am_Map_##Name *Copy();                                                     \
    void Clear();                                                              \
    virtual void FreeItem(ItemType) {}                                         \
    /* empty, override to use */                                               \
  protected:                                                                   \
    Am_Assoc_##Name *FindAssoc(KeyType key) const;                             \
    void AddAssoc(KeyType key, Am_Assoc_##Name *assoc);                        \
    Am_Assoc_##Name *ExtractAssoc(KeyType key);                                \
                                                                               \
  protected:                                                                   \
    int m_size;                                                                \
    int m_count;                                                               \
    Am_Assoc_##Name **m_table;                                                 \
  };                                                                           \
  /**/                                                                         \
  /**/                                                                         \
  class Am_MapIterator_##Name                                                  \
  {                                                                            \
  public:                                                                      \
    Am_MapIterator_##Name(const Am_Map_##Name *map = nullptr) { Init(map); };     \
    ItemType operator()();                                                     \
    KeyType Key() const { return m_key; };                                     \
    void Init(const Am_Map_##Name *map);                                       \
    void Reset();                                                              \
                                                                               \
  protected:                                                                   \
    void NextTblEntry();                                                       \
                                                                               \
  protected:                                                                   \
    const Am_Map_##Name *m_map;                                                \
    int m_tbl_pos;                                                             \
    Am_Assoc_##Name *m_ass_pos;                                                \
    KeyType m_key;                                                             \
  };

#define Cast_Return_Type(x) (x) // GCC needs non-redundant cast

#define AM_IMPL_MAP(Name, KeyType, NullKey, ItemType, NullItem)                \
  /**/                                                                         \
  Am_Map_##Name::Am_Map_##Name(int size) : m_count(0)                          \
  {                                                                            \
    m_table = (Am_Assoc_##Name **)malloc(size * sizeof(Am_Assoc_##Name));      \
    memset(m_table, 0, size * sizeof(Am_Assoc_##Name));                        \
    m_size = size;                                                             \
  }                                                                            \
  /**/                                                                         \
  Am_Map_##Name::~Am_Map_##Name()                                              \
  {                                                                            \
    Clear();                                                                   \
    free(m_table);                                                             \
  }                                                                            \
  /**/                                                                         \
  void Am_Map_##Name::SetAt(KeyType key, ItemType item)                        \
  {                                                                            \
    Am_Assoc_##Name *assoc;                                                    \
    assoc = FindAssoc(key);                                                    \
    if (assoc) {                                                               \
      FreeItem(assoc->m_item);                                                 \
      assoc->m_key = key;                                                      \
      assoc->m_item = item;                                                    \
    } else                                                                     \
      AddAssoc(key, new Am_Assoc_##Name(key, item));                           \
  }                                                                            \
  /**/                                                                         \
  ItemType Am_Map_##Name::GetAt(KeyType key) const                             \
  {                                                                            \
    Am_Assoc_##Name *assoc;                                                    \
    return (assoc = FindAssoc(key)) ? assoc->m_item                            \
                                    : Cast_Return_Type(ItemType) NullItem;     \
  }                                                                            \
  /**/                                                                         \
  void Am_Map_##Name::DeleteKey(KeyType key)                                   \
  {                                                                            \
    Am_Assoc_##Name *assoc;                                                    \
    assoc = ExtractAssoc(key);                                                 \
    if (assoc) {                                                               \
      FreeItem(assoc->m_item);                                                 \
      delete assoc;                                                            \
    };                                                                         \
  }                                                                            \
  /**/                                                                         \
  ItemType &Am_Map_##Name::operator[](KeyType key)                             \
  {                                                                            \
    Am_Assoc_##Name *assoc;                                                    \
    if (!(assoc = FindAssoc(key))) {                                           \
      assoc = new Am_Assoc_##Name(key, NullItem);                              \
      AddAssoc(key, assoc);                                                    \
    }                                                                          \
    return assoc->m_item;                                                      \
  }                                                                            \
  /**/                                                                         \
  void Am_Map_##Name::Resize(int size)                                         \
  {                                                                            \
    Am_Assoc_##Name **old_table = m_table;                                     \
    /**/                                                                       \
    m_table = (Am_Assoc_##Name **)malloc(size * sizeof(Am_Assoc_##Name));      \
    memset(m_table, 0, size * sizeof(Am_Assoc_##Name));                        \
    /**/                                                                       \
    int old_size = m_size;                                                     \
    m_size = size;                                                             \
    /**/                                                                       \
    Am_Assoc_##Name *assoc;                                                    \
    Am_Assoc_##Name *next;                                                     \
    for (int i = 0; i < old_size; i++)                                         \
      for (assoc = old_table[i]; assoc; assoc = next) {                        \
        next = assoc->m_next;                                                  \
        assoc->m_next = nullptr;                                                  \
        AddAssoc(assoc->m_key, assoc);                                         \
      }                                                                        \
    /**/                                                                       \
    free(old_table);                                                           \
  }                                                                            \
  /**/                                                                         \
  Am_Map_##Name *Am_Map_##Name::Copy()                                         \
  {                                                                            \
    Am_Map_##Name *m = new Am_Map_##Name(m_size);                              \
    /**/                                                                       \
    Am_Assoc_##Name *assoc;                                                    \
    for (int i = 0; i < m_size; i++)                                           \
      for (assoc = m_table[i]; assoc; assoc = assoc->m_next)                   \
        m->SetAt(assoc->m_key, assoc->m_item);                                 \
    return m;                                                                  \
  }                                                                            \
  /**/                                                                         \
  void Am_Map_##Name::Clear()                                                  \
  {                                                                            \
    Am_Assoc_##Name *assoc;                                                    \
    Am_Assoc_##Name *next;                                                     \
    for (int i = 0; i < m_size; i++) {                                         \
      for (assoc = m_table[i]; assoc; assoc = next) {                          \
        next = assoc->m_next;                                                  \
        FreeItem(assoc->m_item);                                               \
        delete assoc;                                                          \
      }                                                                        \
      m_table[i] = nullptr;                                                       \
    }                                                                          \
  }                                                                            \
  /**/                                                                         \
  Am_Assoc_##Name *Am_Map_##Name::FindAssoc(KeyType key) const                 \
  {                                                                            \
    int hash = HashValue(key, m_size);                                         \
    Am_Assoc_##Name *assoc = m_table[hash];                                    \
    for (; assoc && KeyComp(assoc->m_key, key); assoc = assoc->m_next)         \
      ;                                                                        \
    return assoc;                                                              \
  }                                                                            \
  /**/                                                                         \
  void Am_Map_##Name::AddAssoc(KeyType key, Am_Assoc_##Name *assoc)            \
  {                                                                            \
    int hv = HashValue(key, m_size);                                           \
    assoc->m_next = m_table[hv];                                               \
    m_table[hv] = assoc;                                                       \
    m_count++;                                                                 \
  }                                                                            \
  /**/                                                                         \
  Am_Assoc_##Name *Am_Map_##Name::ExtractAssoc(KeyType key)                    \
  {                                                                            \
    int hv = HashValue(key, m_size);                                           \
    Am_Assoc_##Name *assoc = m_table[hv];                                      \
    Am_Assoc_##Name *prev = 0;                                                 \
    /**/                                                                       \
    for (prev = nullptr; assoc && KeyComp(assoc->m_key, key);                     \
         assoc = assoc->m_next)                                                \
      prev = assoc;                                                            \
    /**/                                                                       \
    if (assoc) {                                                               \
      if (prev)                                                                \
        prev->m_next = assoc->m_next;                                          \
      else                                                                     \
        m_table[hv] = assoc->m_next;                                           \
      m_count--;                                                               \
    }                                                                          \
    /**/                                                                       \
    return assoc;                                                              \
  }                                                                            \
  /**/                                                                         \
  /**/                                                                         \
  void Am_MapIterator_##Name::Init(const Am_Map_##Name *map)                   \
  {                                                                            \
    m_map = map;                                                               \
    Reset();                                                                   \
  }                                                                            \
  /**/                                                                         \
  void Am_MapIterator_##Name::Reset()                                          \
  {                                                                            \
    m_key = NullKey;                                                           \
    m_ass_pos = nullptr;                                                          \
    m_tbl_pos = -1;                                                            \
    if (m_map)                                                                 \
      NextTblEntry();                                                          \
  }                                                                            \
  /**/                                                                         \
  ItemType Am_MapIterator_##Name::operator()()                                 \
  {                                                                            \
    if (m_ass_pos) {                                                           \
      ItemType item = m_ass_pos->m_item;                                       \
      m_key = m_ass_pos->m_key;                                                \
      m_ass_pos = m_ass_pos->m_next;                                           \
      return item;                                                             \
    } else {                                                                   \
      NextTblEntry();                                                          \
      return (m_ass_pos) ? operator()() : Cast_Return_Type(ItemType) NullItem; \
    }                                                                          \
  }                                                                            \
  /**/                                                                         \
  void Am_MapIterator_##Name::NextTblEntry()                                   \
  {                                                                            \
    for (int i = m_tbl_pos + 1; i < m_map->Size(); i++)                        \
      if (m_map->m_table[i]) {                                                 \
        m_tbl_pos = i;                                                         \
        m_ass_pos = m_map->m_table[i];                                         \
        return;                                                                \
      }                                                                        \
    m_ass_pos = nullptr;                                                          \
    m_key = NullKey;                                                           \
  }

#define AM_DECL_TABLE(Name1, Name2, KeyType1, KeyType2)                        \
  class Am_Table_##Name1; /* needed for Solaris gcc for some reason */         \
  class Am_Table_##Name1 : public Am_Map_##Name1, public Am_Map_##Name2        \
  {                                                                            \
  public:                                                                      \
    Am_Table_##Name1(int size = 89)                                            \
        : Am_Map_##Name1(size), Am_Map_##Name2(size){};                        \
    void SetAt(KeyType1 key1, KeyType2 key2);                                  \
    void SetAt(KeyType2 key1, KeyType1 key2);                                  \
    KeyType2 GetAt(KeyType1 key1);                                             \
    KeyType1 GetAt(KeyType2 key1);                                             \
    KeyType1 operator[](KeyType2 key) const;                                   \
    KeyType2 operator[](KeyType1 key) const;                                   \
    void DeleteKey(KeyType1 key);                                              \
    void DeleteKey(KeyType2 key);                                              \
    bool IsEmpty() const;                                                      \
    int Count() const;                                                         \
    int Size() const;                                                          \
    void Resize(int size);                                                     \
    void Clear();                                                              \
  };                                                                           \
  /**/                                                                         \
  /**/                                                                         \
  inline void Am_Table_##Name1::SetAt(KeyType1 key1, KeyType2 key2)            \
  {                                                                            \
    Am_Map_##Name1::SetAt(key1, key2);                                         \
    Am_Map_##Name2::SetAt(key2, key1);                                         \
  }                                                                            \
  /**/                                                                         \
  inline void Am_Table_##Name1::SetAt(KeyType2 key1, KeyType1 key2)            \
  {                                                                            \
    SetAt(key2, key1);                                                         \
  }                                                                            \
  inline KeyType2 Am_Table_##Name1::GetAt(KeyType1 key1)                       \
  {                                                                            \
    return Am_Map_##Name1::GetAt(key1);                                        \
  }                                                                            \
  inline KeyType1 Am_Table_##Name1::GetAt(KeyType2 key1)                       \
  {                                                                            \
    return Am_Map_##Name2::GetAt(key1);                                        \
  }                                                                            \
  /**/                                                                         \
  inline KeyType1 Am_Table_##Name1::operator[](KeyType2 key) const             \
  {                                                                            \
    return Am_Map_##Name2::GetAt(key);                                         \
  }                                                                            \
  /**/                                                                         \
  inline KeyType2 Am_Table_##Name1::operator[](KeyType1 key) const             \
  {                                                                            \
    return Am_Map_##Name1::GetAt(key);                                         \
  }                                                                            \
  /**/                                                                         \
  inline void Am_Table_##Name1::DeleteKey(KeyType1 key)                        \
  {                                                                            \
    KeyType2 key2 = Am_Map_##Name1::GetAt(key);                                \
    Am_Map_##Name1::DeleteKey(key);                                            \
    if (key == Am_Map_##Name2::GetAt(key2))                                    \
      Am_Map_##Name2::DeleteKey(key2);                                         \
  }                                                                            \
  /**/                                                                         \
  inline void Am_Table_##Name1::DeleteKey(KeyType2 key)                        \
  {                                                                            \
    KeyType1 key2 = Am_Map_##Name2::GetAt(key);                                \
    Am_Map_##Name2::DeleteKey(key);                                            \
    if (key == Am_Map_##Name1::GetAt(key2))                                    \
      Am_Map_##Name1::DeleteKey(key2);                                         \
  }                                                                            \
  /**/                                                                         \
  inline bool Am_Table_##Name1::IsEmpty() const                                \
  {                                                                            \
    return Am_Map_##Name1::IsEmpty();                                          \
  }                                                                            \
  /**/                                                                         \
  inline int Am_Table_##Name1::Count() const                                   \
  {                                                                            \
    return Am_Map_##Name1::Count();                                            \
  }                                                                            \
  /**/                                                                         \
  inline int Am_Table_##Name1::Size() const { return Am_Map_##Name1::Size(); } \
  /**/                                                                         \
  inline void Am_Table_##Name1::Resize(int size)                               \
  {                                                                            \
    Am_Map_##Name1::Resize(size);                                              \
    Am_Map_##Name2::Resize(size);                                              \
  }                                                                            \
  /**/                                                                         \
  inline void Am_Table_##Name1::Clear()                                        \
  {                                                                            \
    Am_Map_##Name1::Clear();                                                   \
    Am_Map_##Name2::Clear();                                                   \
  }
