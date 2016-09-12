////////////////////////////////////////////////////////////////////////
// Am_Inter_Location used in move_grow and new_points interactors
// and commands to store old value, etc.
////////////////////////////////////////////////////////////////////////

enum Am_Location_Value_Control { Am_LOCATION_OK, Am_LOCATION_RELATIVE,
				 Am_LOCATION_NOT_USED };

class Am_Inter_Location_Data;
  
class _OA_DL_CLASSIMPORT Am_Inter_Location {
  AM_WRAPPER_DECL (Am_Inter_Location)
public:
  Am_Inter_Location (); // empty
  Am_Inter_Location (bool as_line, Am_Object ref_obj,
		     int a, int b, int c, int d, bool growing = true,
		     bool directional = true); 
  Am_Inter_Location (const Am_Object& object);
  void Set_Location (bool as_line, Am_Object ref_obj,
		     int a, int b, int c, int d, bool make_unique = true);
  //two-value form sets growning to false
  void Set_Location (bool as_line, Am_Object ref_obj,
		     int a, int b, bool make_unique = true);
  void Get_Location (bool &as_line, Am_Object &ref_obj,
		     int &a, int &b, int &c, int &d) const;
  void Set_Growing (bool growing, bool make_unique = true);
  bool Get_Growing () const;
  void Set_Directional (bool directional, bool make_unique = true);
  bool Get_Directional () const;
  void Set_As_Line (bool as_line, bool make_unique = true);
  void Get_As_Line (bool &as_line) const;
  void Set_Control (Am_Location_Value_Control a_control,
		    Am_Location_Value_Control b_control,
		    Am_Location_Value_Control c_control,
		    Am_Location_Value_Control d_control,
		    bool make_unique = true);
  void Set_Control (Am_Location_Value_Control all_control,
		    bool make_unique = true);
  void Set_A_Control (Am_Location_Value_Control control,
		      bool make_unique = true);
  void Set_B_Control (Am_Location_Value_Control control,
		      bool make_unique = true);
  void Set_C_Control (Am_Location_Value_Control control,
		      bool make_unique = true);
  void Set_D_Control (Am_Location_Value_Control control,
		      bool make_unique = true);
  void Get_Control (Am_Location_Value_Control &a_control,
		    Am_Location_Value_Control &b_control,
		    Am_Location_Value_Control &c_control,
		    Am_Location_Value_Control &d_control) const;
  void Get_Control (Am_Location_Value_Control &a_control,
		    Am_Location_Value_Control &b_control) const;
  void Get_Points (int &a, int &b, int &c, int &d) const;
  Am_Object Get_Ref_Obj () const;
  void Set_Points (int a, int b, int c, int d, bool make_unique = true);
  void Set_Ref_Obj (const Am_Object& ref_obj, bool make_unique = true);
  void Copy_From (Am_Inter_Location& other_obj, bool make_unique = true);
  void Swap_With (Am_Inter_Location& other_obj, bool make_unique = true);
  Am_Inter_Location Copy() const;
  bool Translate_To(Am_Object dest_obj); //trans_coord from ref_obj to dest_obj
  bool operator== (const Am_Inter_Location& test);
  bool operator== (const Am_Inter_Location& test) const;
  bool operator!= (const Am_Inter_Location& test);
  bool operator>= (const Am_Inter_Location& test); // A >= B is A contains B
  bool operator&& (const Am_Inter_Location& test); // test overlapping

  //checks growing against the internal growing
  void Install (Am_Object& object, bool growing) const;
  //uses the internal growing
  void Install (Am_Object& object) const;
  bool Is_Zero_Size() const; 
};

_OA_DL_IMPORT extern Am_Inter_Location Am_No_Location;

_OA_DL_IMPORT std::ostream& operator<< (std::ostream& os, Am_Inter_Location& loc);
