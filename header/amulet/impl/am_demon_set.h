#ifndef OBJECT_DEMON_H
#define OBJECT_DEMON_H

#include "types_demon.h"

class Am_Object;
class Am_Slot;
class Am_Value;


typedef unsigned short Am_Demon_Protocol;
#define Am_DEMON_PER_SLOT      0x0000
#define Am_DEMON_PER_OBJECT    0x0001
#define Am_DEMON_ON_INVALIDATE 0x0002
#define Am_DEMON_ON_CHANGE     0x0004

enum Am_Object_Demon_Type
{ Am_CREATE_OBJ, Am_COPY_OBJ, Am_DESTROY_OBJ 
};
enum Am_Part_Demon_Type
{ Am_ADD_PART, Am_CHANGE_OWNER 
};

class Am_Demon_Set_Data;

class _OA_DL_CLASSIMPORT Am_Demon_Set
{
	public:
	_OA_DL_MEMBERIMPORT Am_Object_Demon* Get_Object_Demon(Am_Object_Demon_Type type) const;
	void Set_Object_Demon(Am_Object_Demon_Type type,
		Am_Object_Demon* demon);

	_OA_DL_MEMBERIMPORT Am_Slot_Demon* Get_Slot_Demon(unsigned short which_bit) const;
	void Set_Slot_Demon(unsigned short which_bit, Am_Slot_Demon* method,
		Am_Demon_Protocol protocol);

	_OA_DL_MEMBERIMPORT Am_Part_Demon* Get_Part_Demon(Am_Part_Demon_Type type) const;
	void Set_Part_Demon(Am_Part_Demon_Type type, Am_Part_Demon* demon);

	_OA_DL_MEMBERIMPORT Am_Type_Check* Get_Type_Check(unsigned short type) const;
	void Set_Type_Check(unsigned short type, Am_Type_Check* demon);

	_OA_DL_MEMBERIMPORT Am_Demon_Set Copy() const;
	Am_Demon_Set& operator= (const Am_Demon_Set& proto);

	~Am_Demon_Set();

	Am_Demon_Set();
	Am_Demon_Set(const Am_Demon_Set& proto);
	Am_Demon_Set(Am_Demon_Set_Data* in_data)
	{ data = in_data; 
	}

	_OA_DL_MEMBERIMPORT operator Am_Demon_Set_Data* () const
	{ return data; 
	}

	private:
	Am_Demon_Set_Data* data;
};

#endif /* OBJECT_DEMON_H */
