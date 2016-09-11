// Don't use namespaces
#ifdef NO_NAMESPACES
	#define OpenAmulet
#endif

#ifndef OA_VERSION
	#include UNIV_MAP__H
#else
	#include <amulet/univ_map_oa.hpp>
#endif

#ifndef OA_VERSION
	// define the hash table from a style to a Motif_Colors_Record
	inline int HashValue(Am_Wrapper* key, int size)
	{
		return HashValue((const void*)key, size);
	}

	inline int KeyComp(Am_Wrapper* key1, Am_Wrapper* key2)
	{
		return KeyComp((const void*)key1, (const void*)key2);
	}

	Am_DECL_MAP(Style2MotifRec, Am_Wrapper*, Computed_Colors_Record_Data*)
#else
	typedef OpenAmulet::Map<Am_Wrapper*, Computed_Colors_Record_Data*> 	Am_Map_Style2MotifRec;
	typedef OpenAmulet::Map_Iterator<Am_Map_Style2MotifRec> 				Am_Map_Iterator_Style2MotifRec;

	// defined in widgets.cpp
//	extern Map_Style2MotifRec 			Am_Map_Style2MotifRec;
//	extern Map_Iterator_Style2MotifRec 	Am_MapIterator_Style2MotifRec;
#endif
