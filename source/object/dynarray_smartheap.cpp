#include <string.h>
#include <stdlib.h>

#define DYNARRAY__H <amulet/dynarray.h>
#include DYNARRAY__H

#ifdef NEED_MEMMOVE
void memmove (char* dest, const char* src, int length)
{
	char *data = MemAllocPtr(data_memory,length,0);

	memcpy(data, src, length);
	memcpy(dest, data, length);

	MemFreePtr(data);
}
#endif

// Set the memory pool for the copies of dynarray_objects to 0
MEM_POOL DynArray::dynarray_objects = 0;
MEM_POOL DynArray::dynarray_4byte_memory = 0;

DynArray::DynArray(unsigned in_elem_size) :
	elem_size(in_elem_size),
	data_size(1),
	length(0)
{
	// do we allready have our 4byte pool?
	if(dynarray_4byte_memory == 0)
	{
		dynarray_4byte_memory = MemPoolInitFS(4,200,MEM_POOL_DEFAULT);
		dbgMemPoolSetName(dynarray_4byte_memory,"DynArray 4 ByteObjects");
	}

	// is this a 4 byte object?
	if(elem_size == 4)
	{
		// if so use the existing pool
		data_memory = dynarray_4byte_memory;
	}
	else
	{
		// else create a pool
		data_memory = MemPoolInitFS(elem_size, 10, 0);
		dbgMemPoolSetName(data_memory,"DynArray Objects Pool");
	}

	data = static_cast<char*>(MemAllocFS(data_memory));
}

char* DynArray::Get(unsigned i)
{
	if(i < length)
	{
	    return(&data[i*elem_size]);
	}
	else
	{
	  return(static_cast<char*>(0));
	}
}

void DynArray::Set(unsigned i, char *value)
{
	if(i < length)
	{
		memcpy(&data[i*elem_size], value, elem_size);
	}
}

void DynArray::Insert(unsigned i, char *value)
{
	if(i <= length)
	{

		if(length == data_size)
		{
			data_size *= 2;
			data = static_cast<char*>(MemReAllocPtr(data, data_size * elem_size, MEM_RESIZEABLE));
		}
		else
		{
			memmove(&data[(i+1)*elem_size], &data[i*elem_size],(length-i)*elem_size);
		}

		// copy the value to the array
		memcpy(&data[i*elem_size], value, elem_size);
		length++;
	}
}

void DynArray::Delete(unsigned i)
{
	if (i < length)
	{
		length--;
		if(length < (unsigned)(data_size / 3))
		{
			data_size /= 2;
			char* new_data;

			new_data = static_cast<char*>(MemAllocPtr(data_memory, data_size * elem_size, 0));

			memcpy(new_data, data, i*elem_size);
			memcpy(&new_data[i*elem_size], &data[(i+1)*elem_size], (length-i)*elem_size);

			MemFreePtr(data);

			data = new_data;
		}
		else
		{
			memmove(&data[i*elem_size], &data[(i+1)*elem_size],(length-i)*elem_size);
		}
	}
}

void DynArray::Add (char* value)
{
	if(length == data_size)
	{
		data_size *= 2;

		data = static_cast<char*>(MemReAllocPtr(data, data_size * elem_size, MEM_RESIZEABLE));
	}

	// copy the data to the array
	memcpy(&data[length*elem_size], value, elem_size);
	length++;
}

DynArray* DynArray::Copy()
{
	if(dynarray_objects == 0)
	{
		dynarray_objects = MemPoolInitFS(sizeof(DynArray),10,MEM_POOL_DEFAULT);
		dbgMemPoolSetName(data_memory,"DynArray Copy-Objecs");
	}

	DynArray	*new_array = DEBUG_NEW1 (dynarray_objects) DynArray(elem_size);

	new_array->length 		= length;
	new_array->data_size 	= data_size;

	new_array->data = static_cast<char*>(MemReAllocPtr(new_array->data, data_size*elem_size, MEM_RESIZEABLE));
	memcpy(new_array->data, data, length*elem_size);

	return(new_array);
}

void DynArray::Destroy()
{
	// only if we have an own pool we can free it
	if(elem_size != 4)
	{
		MemPoolFree(data_memory);
	}
	else
	{
		// otherwise only free the memory
		MemFreePtr(data);
	}

	data = static_cast<char*>(0);
}
