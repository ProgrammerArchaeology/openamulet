#ifndef DYNMM_SMARTHEAP_CPP
#define DYNMM_SMARTHEAP_CPP

#ifdef USE_SMARTHEAP
	#include <heapagnt.h>
#endif

Dyn_Memory_Manager::Dyn_Memory_Manager(size_t aObjectSize, const char *aPoolName)
			{
				mMemoryPool = MemPoolInitFS(aObjectSize,10,MEM_POOL_DEFAULT);
				dbgMemPoolSetName(mMemoryPool, aPoolName);
// 				if(mMemoryPool == MEM_OUT_OF_MEMORY) 
// 				{                                    
// 					throw bad_allocation;
// 				}                                    
			}

			//:Releases the allocated memory-pool with one call
Dyn_Memory_Manager::~Dyn_Memory_Manager()
			{
				MemPoolFree(mMemoryPool);
			}

void Dyn_Memory_Manager::Delete(void *ptr)
			{
				MemFreeFS(ptr);
				return;
			}

void *Dyn_Memory_Manager::New()
			{
				return(MemAllocFS(mMemoryPool));
			}

#endif
