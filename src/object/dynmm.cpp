/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <am_inc.h>
#include <amulet/dynarray.h>

#ifdef DEBUG
#include <iostream>
#endif

Dyn_Memory_Manager::Dyn_Memory_Manager(unsigned short size,
                                       const char *aPoolName)
    : data_size(size), free_list(static_cast<Dyn_Link *>(nullptr)),
#ifdef DEBUG
      blk_count(1),
#endif
      block_size(1), // the minimum size will at least be one byte
      block_length(1), block_position(0)
{
  // the block size we need to handle allocations of the requested 'size'
  // is at least the 'size' itself and the overhead for the links
  unsigned short min_block_size = size + sizeof(Dyn_Link);

  // multiply the block_size by 2 until we can handle min_block_size requests
  while (block_size < min_block_size) {
    block_size <<= 1;
  }

  // allocate the first block
  block = (Dyn_Link *)new char[block_size];
  block->next = nullptr;
}

Dyn_Memory_Manager::~Dyn_Memory_Manager()
{
#ifdef DEBUG
  unsigned int count = 0;
#endif
  Dyn_Link *next;

  // free the allocated blocks
  Dyn_Link *curr = block;
  while (curr != static_cast<Dyn_Link *>(nullptr)) {
#ifdef DEBUG
    ++count;
#endif
    // get the next block
    next = curr->next;

    // delete the actual one; this was a char array
    delete[] curr;

    // ok next
    curr = next;
  }

#ifdef DEBUG
  if (count != blk_count)
    std::cout << "DEBUG:Memory leak in <Dyn_Memory_Manager " << std::hex << this
              << ">, alloc'ed:" << blk_count << ", freed:" << count
              << std::endl;
#endif

  // for safety
  block = static_cast<Dyn_Link *>(nullptr);
  free_list = static_cast<Dyn_Link *>(nullptr);
}

void *
Dyn_Memory_Manager::New()
{
  void *new_ptr;

  // Do we have some blocks available in the free-list
  if (free_list != static_cast<Dyn_Link *>(nullptr)) {
    // assign it and return
    new_ptr = free_list;
    free_list = free_list->next;

    // we return here to avoid a jmp
    return (new_ptr);
  } else {
    // do we have to get new memory?
    if (block_position == block_length) {
#ifdef DEBUG
      ++blk_count;
#endif
      // reset the position
      block_position = 0;

      // we now allocate twice as much to better satisfy later requests
      block_size <<= 1;

      // how many blocks of 'data_size' does this give us?
      block_length = (block_size - sizeof(Dyn_Link)) / data_size;

      // next chunk of memory
      Dyn_Link *new_block = reinterpret_cast<Dyn_Link *>(new char[block_size]);

      // put at the beginning of the list, because we calculate the block position
      // from the beginning of the list
      new_block->next = block;

      // and safe the pointer
      block = new_block;
    }

    // x86 alloc
    // get the next free block and return it
    new_ptr = reinterpret_cast<void *>(
        ((uintptr_t)block) + block_position * data_size + sizeof(Dyn_Link));

    // update our counter
    block_position++;
  }

  return (new_ptr);
}

void
Dyn_Memory_Manager::Delete(void *ptr)
{
  // save the free block in the free list
  Dyn_Link *old_ptr = static_cast<Dyn_Link *>(ptr);

  old_ptr->next = free_list;
  free_list = old_ptr;
}
