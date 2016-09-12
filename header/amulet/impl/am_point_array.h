#ifndef GDEFS_POINT_ARRAY_H
#define GDEFS_POINT_ARRAY_H

class Am_Point_List;

class Am_Point_Array_Data;

class _OA_DL_CLASSIMPORT Am_Point_Array {
  AM_WRAPPER_DECL(Am_Point_Array)
public:
  Am_Point_Array (Am_Point_List pl);
  Am_Point_Array (Am_Point_List pl, int offset_x, int offset_y);
  Am_Point_Array (int *ar, int num_coords);
  Am_Point_Array (int num_points);

  int Length ();
  void Get (int index, int &x, int &y);
  void Set (int index, int x, int y);
  void Translate (int offset_x, int offset_y);
};

#endif //GDEFS_POINT_ARRAY_H
