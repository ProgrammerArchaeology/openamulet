#ifndef OBJECT_DEP_ITER_H
#define OBJECT_DEP_ITER_H

#include "am_slot.h"

class Am_Dependency_Iterator_Data;

class _OA_DL_CLASSIMPORT Am_Dependency_Iterator
{
	public:
	Am_Dependency_Iterator();
	Am_Dependency_Iterator(const Am_Slot& slot);

		Am_Dependency_Iterator& operator= (const Am_Slot& slot);

	unsigned short Length() const;
	void Start();
	void Next();
	bool Last() const;
	Am_Constraint* Get() const;
	Am_Constraint_Tag Get_Tag() const;

		private:
	Am_Slot context;
	Am_Dependency_Iterator_Data* current;
};

#endif // OBJECT_DEP_ITER_H
