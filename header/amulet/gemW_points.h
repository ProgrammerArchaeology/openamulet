/* ****************************** -*-c++-*- *******************************/
/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

										  
#if !defined(GWPTLIST_H)		         
#define GWPTLIST_H   			   

#include <windows.h>

class Am_Point_Array_Data : public Am_Wrapper {
friend class Am_Point_Array;
friend class Am_WinDrawonable; // for direct access to POINT representation

  Am_WRAPPER_DATA_DECL(Am_Point_Array)
public:
  Am_Point_Array_Data (Am_Point_Array_Data *proto);
  Am_Point_Array_Data (Am_Point_List pl, int offset_x, int offset_y);
  Am_Point_Array_Data (int *coords, int num_coords);
  Am_Point_Array_Data (int size);
  ~Am_Point_Array_Data ();

  bool operator== (Am_Point_Array_Data&)
  { return false; }

  bool operator== (Am_Point_Array_Data&) const
  { return false; }

protected:
  POINT *ar;
  int size;
};
									   

#endif

