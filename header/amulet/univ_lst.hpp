/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

// Template macros for Amulet lists

#define Am_DECL_LIST(Name, ItemType, NullItem)                       \
struct Am_Elem_##Name {                                              \
  ItemType m_item;                                                   \
  Am_Elem_##Name* m_next;                                            \
/**/                                                                 \
  Am_Elem_##Name (ItemType item) :                                   \
    m_item(item), m_next((0L)) { };                                  \
};                                                                   \
/**/                                                                 \
/**/                                                                 \
class Am_List_##Name {                                               \
  public:                                                            \
    Am_List_##Name () :                                              \
      m_count(0), m_head((0L)) { };                                  \
    virtual ~Am_List_##Name ();                                      \
    void InsertAt (Am_Position pos, ItemType item);                  \
    void Add (ItemType item);                                        \
    Am_Position HeadPos () const;                                    \
    Am_Position NextPos (Am_Position pos) const;                     \
    Am_Position PrevPos (Am_Position pos) const;                     \
    Am_Position TailPos () const;                                    \
    Am_Position FindPos (ItemType item) const;                       \
    ItemType GetAt (Am_Position pos) const;                          \
    void DeleteAt (Am_Position pos);                                 \
    ItemType& operator [] (Am_Position pos);                         \
      /*always adds new (0L) item if pos == NULL*/                   \
      /*doesn't call FreeItem if the item is being changed*/         \
    bool IsEmpty () const { return m_count == 0; };                  \
    int Count () const { return m_count; };                          \
    void Clear ();                                                   \
    virtual void FreeItem (ItemType /*item*/) { };                   \
      /*gets called after the item is removed from list*/            \
      /*redefine if you for ex. need to call item's destructor*/     \
  protected:                                                         \
    int m_count;                                                     \
    Am_Elem_##Name* m_head;                                          \
};                                                                   \
/**/                                                                 \
/**/                                                                 \
inline Am_Position Am_List_##Name::HeadPos () const                  \
{ return (Am_Position)m_head; }                                      \
/**/                                                                 \
inline Am_Position Am_List_##Name::NextPos (Am_Position pos) const   \
{ return pos? (Am_Position)((Am_Elem_##Name*)pos) -> m_next : (0L); }\
/**/                                                                 \
inline ItemType Am_List_##Name::GetAt (Am_Position pos) const        \
{ return pos? ((Am_Elem_##Name*)pos) -> m_item : NullItem; }


#define Am_IMPL_LIST(Name, ItemType, NullItem)                       \
Am_List_##Name::~Am_List_##Name ()                                   \
{                                                                    \
  Clear();                                                           \
}                                                                    \
/**/                                                                 \
Am_Position Am_List_##Name::PrevPos (Am_Position pos) const          \
{                                                                    \
  if (!pos) return (0L);                                             \
  Am_Position prev;                                                  \
  for (prev = HeadPos(); prev; prev = NextPos(prev))                 \
    if (((Am_Elem_##Name*)prev) -> m_next == (Am_Elem_##Name*)pos)   \
      break;                                                         \
  return prev;                                                       \
}                                                                    \
/**/                                                                 \
Am_Position Am_List_##Name::TailPos () const                         \
{                                                                    \
  Am_Position prev;                                                  \
  for (prev = HeadPos(); prev; prev = NextPos(prev))                 \
    if (!((Am_Elem_##Name*)prev) -> m_next)                          \
      break;                                                         \
  return prev;                                                       \
}                                                                    \
/**/                                                                 \
Am_Position Am_List_##Name::FindPos (ItemType item) const            \
{                                                                    \
  Am_Position pos;                                                   \
  for (pos = HeadPos(); pos; pos = NextPos(pos))                     \
    if (((Am_Elem_##Name*)pos) -> m_item == item)                    \
      break;                                                         \
  return pos;                                                        \
}                                                                    \
/**/                                                                 \
void Am_List_##Name::InsertAt (Am_Position pos, ItemType item)       \
{                                                                    \
  if (!pos) {                                                        \
    Add(item);                                                       \
    return;                                                          \
  }                                                                  \
  Am_Elem_##Name* elem = new Am_Elem_##Name(item);                   \
  elem -> m_next = (Am_Elem_##Name*)pos;                             \
  Am_Elem_##Name* prev = (Am_Elem_##Name*)PrevPos(pos);              \
  if (prev) prev -> m_next = elem;                                   \
  else m_head = elem;                                                \
  m_count++;                                                         \
}                                                                    \
/**/                                                                 \
void Am_List_##Name::Add (ItemType item)                             \
{                                                                    \
  Am_Elem_##Name* elem = new Am_Elem_##Name(item);                   \
  Am_Elem_##Name* tail = (Am_Elem_##Name*)TailPos();                 \
  if (tail) tail -> m_next = elem;                                   \
  else m_head = elem;                                                \
  m_count++;                                                         \
}                                                                    \
/**/                                                                 \
void Am_List_##Name::DeleteAt (Am_Position pos)                      \
{                                                                    \
  if (!pos)                                                          \
    return;                                                          \
  Am_Elem_##Name* prev = (Am_Elem_##Name*)PrevPos(pos);              \
  if (prev) prev -> m_next = ((Am_Elem_##Name*)pos) -> m_next;       \
  else m_head = ((Am_Elem_##Name*)pos) -> m_next;                    \
  FreeItem(((Am_Elem_##Name*)pos) -> m_item);                        \
  delete (Am_Elem_##Name*)pos;                                       \
  m_count--;                                                         \
}                                                                    \
/**/                                                                 \
ItemType& Am_List_##Name::operator [] (Am_Position pos)              \
{                                                                    \
  if (pos)                                                           \
    return ((Am_Elem_##Name*)pos) -> m_item;                         \
  else {                                                             \
    Am_Elem_##Name* elem = new Am_Elem_##Name(NullItem);             \
    Am_Elem_##Name* tail = (Am_Elem_##Name*)TailPos();               \
    if (tail) tail -> m_next = elem;                                 \
    else m_head = elem;                                              \
    m_count++;                                                       \
    return elem -> m_item;                                           \
  }                                                                  \
}                                                                    \
/**/                                                                 \
void Am_List_##Name::Clear ()                                        \
{                                                                    \
  Am_Position next;                                                  \
  for (Am_Position pos = HeadPos(); pos; pos = next) {               \
    next = ((Am_Elem_##Name*)pos) -> m_next;                         \
    FreeItem(((Am_Elem_##Name*)pos) -> m_item);                      \
    delete (Am_Elem_##Name*)pos;                                     \
  }                                                                  \
  m_count = 0;                                                       \
  m_head = (0L);                                                     \
}

