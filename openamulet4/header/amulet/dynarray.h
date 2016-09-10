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

#ifdef USE_SMARTHEAP
	#ifndef _SMARTHEAP_HPP
		#include <smrtheap.hpp>
	#endif
#endif

class DynArray
{
	public:
		DynArray(unsigned elem_size);

		char* Get(unsigned i);
		void Set(unsigned i, char* value);

		void Insert(unsigned i, char* value);
		void Delete(unsigned i);
		void Add(char* value);

		DynArray* Copy();
		void Destroy();

		char* data;
		unsigned length;

	private:
		// if we don't specify a default ctor make it private
		DynArray();

		unsigned elem_size;
		unsigned data_size;

#ifdef USE_SMARTHEAP
		MEM_POOL data_memory;
		static MEM_POOL dynarray_objects;
		static MEM_POOL dynarray_4byte_memory;
#endif
};

#ifdef USE_SMARTHEAP
	class Dyn_Memory_Manager
	{
		public:
		// CREATORS
			Dyn_Memory_Manager(size_t aObjectSize, char *aPoolName);

			//:Releases the allocated memory-pool with one call
			~Dyn_Memory_Manager();

		// MANIPULATORS
			void  Delete(void *ptr);
			void* New();
		// ACCESSORS
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
			MEM_POOL	mMemoryPool;
	};
#else
	// This here is the normal OpenAmulet memory-allocator
	// if you have SmartHeap use the compile time define USE_SMARTHEAP
	class Dyn_Link
	{
		public:
			Dyn_Link* next;
	};

	class Dyn_Memory_Manager
	{
		public:
			Dyn_Memory_Manager (unsigned short size, char *aPoolName = "");
			~Dyn_Memory_Manager ();

			void* New();
			void  Delete (void *ptr);

		private:
			//:Number if bytes requested by the user
			unsigned short 	data_size;
			//:Number of bytes which are allocated at once from the systems
			unsigned long 	block_size;
			//:Number of 'data_size' allocations which can be handled with the memory
			// allocated by the last block_size
			unsigned long 	block_length;
			//:Next block to allocate
			unsigned long 	block_position;

			//:List of allocations
			Dyn_Link  		*block;
			//:List of free allocations
			Dyn_Link  		*free_list;

#ifdef DEBUG
	        unsigned int blk_count;
#endif
	};
#endif // USE_SMARTHEAP

#endif // DYNARRAY_H
