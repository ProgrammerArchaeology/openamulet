/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#ifndef DYNARRAY_H
#define DYNARRAY_H

class DynArray
{
public:
  DynArray(unsigned elem_size);

  char *Get(unsigned i);
  void Set(unsigned i, const char *value);

  void Insert(unsigned i, const char *value);
  void Delete(unsigned i);
  void Add(char *value);

  DynArray *Copy();
  void Destroy();

  char *data;
  unsigned length;

private:
  // if we don't specify a default ctor make it private
  DynArray();

  unsigned elem_size;
  unsigned data_size;
};

class Dyn_Link
{
public:
  Dyn_Link *next;
};

class Dyn_Memory_Manager
{
public:
  Dyn_Memory_Manager(unsigned short size, const char *aPoolName = "");
  ~Dyn_Memory_Manager();

  void *New();
  void Delete(void *ptr);

private:
  //:Number if bytes requested by the user
  unsigned short data_size;
  //:Number of bytes which are allocated at once from the systems
  unsigned long block_size;
  //:Number of 'data_size' allocations which can be handled with the memory
  // allocated by the last block_size
  unsigned long block_length;
  //:Next block to allocate
  unsigned long block_position;

  //:List of allocations
  Dyn_Link *block;
  //:List of free allocations
  Dyn_Link *free_list;

#ifdef DEBUG
  unsigned int blk_count;
#endif
};

#endif // DYNARRAY_H
