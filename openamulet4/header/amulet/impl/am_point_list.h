#ifndef GDEFS_POINT_LIST_H
#define GDEFS_POINT_LIST_H

#include "types_misc.h"
#include "types_wrapper.h"

class Am_Point_Item;
class Am_Point_List_Data;

class _OA_DL_CLASSIMPORT Am_Point_List {
  Am_WRAPPER_DECL (Am_Point_List)
public:
  Am_Point_List ();
//  Am_Point_List (Am_Point_List&);
  Am_Point_List (int *ar, int size);
  Am_Point_List (float *ar, int size);

  bool operator== (const Am_Point_List& test_list) const;
  bool operator!= (const Am_Point_List& test_list) const;

  // Returns the number of points in the list.
  unsigned short Length () const;

  // Returns whether list is empty or not.
  _OA_DL_MEMBERIMPORT bool Empty () const;

  void Start (); // Make first point be current.
  void End ();   // Make last point be current.

  void Prev ();  // Make previous point be current.
  void Next ();  // Make next point be current.

  // Returns TRUE when current point passes the first point.
  _OA_DL_MEMBERIMPORT bool First () const;

  // Returns TRUE when current point passes the last point.
  _OA_DL_MEMBERIMPORT bool Last () const;

  // Retrieve the current point.  Error if no point is
  // current.  (Can be returned in either float or int representation.)
  _OA_DL_MEMBERIMPORT void Get (int &x, int &y) const;
  _OA_DL_MEMBERIMPORT void Get (float &x, float &y) const;

  // Add puts the new point at the head or tail of the list.
  Am_Point_List& Add (float x, float y, Am_Add_Position position = Am_TAIL,
                      bool unique = true);

  // Insert puts the new point before or after the current position
  // in the list.  The current position is set by using the Start, End, Next,
  // and Prev methods.
  void Insert (float x, float y, Am_Insert_Position position,
               bool unique = true);

  // Change the current point.  Error if no point is current.
  void Set (float x, float y, bool unique = true);

  // Delete the current point.  Error if no point is current.  The current
  // position is shifted to the point previous to the deleted.
  void Delete (bool unique = true);

  // Delete the entire list.  All points are deleted. The current position
  void Make_Empty ();  // becomes undefined.

  //adds points in other_list to my end. Returns me (this) (so can be cascaded)
  Am_Point_List& Append (Am_Point_List other_list, bool unique = true);

  // returns bounding box of all points
  _OA_DL_MEMBERIMPORT void Get_Extents (int& min_x, int& min_y, int& max_x, int& max_y) const;
  
  // translates all points to a new origin.
  void Translate (int offset_x, int offset_y,
		  bool unique = true);

  // scales all points.  Scaling is relative to (origin_x, origin_y).
  void Scale (float scale_x, float scale_y,
	      int origin_x = 0, int origin_y = 0,
	      bool unique = true);

 private:
  Am_Point_Item* item;
};

_OA_DL_IMPORT std::ostream& operator<< (std::ostream& os, Am_Point_List& list);

#endif //GDEFS_POINT_LIST_H
