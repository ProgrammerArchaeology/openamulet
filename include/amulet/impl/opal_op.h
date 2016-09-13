// Opal operations

class Am_Object;
class Am_Drawonable;
class Am_Font;

//Check whether point is inside all the owners of object, up to the
//window.  Also validates that all of the owners are visible.
//  If not, returns false.  Use this to make sure that not pressing
//  outside of an owner since the other operations below do NOT check
//  this.
_OA_DL_IMPORT extern bool Am_Point_In_All_Owners(const Am_Object &in_obj, int x,
                                                 int y,
                                                 const Am_Object &ref_obj);

// Check whether the point is inside the object.  Ignores
//   covering (i.e., just checks whether point is inside the
//   object even if the object is covered.  If inside, returns the
//   object, otherwise returns (0L) (0)
//  The coordinate system of x and y is defined w.r.t. ref_obj
_OA_DL_IMPORT extern Am_Object Am_Point_In_Obj(const Am_Object &in_obj, int x,
                                               int y, const Am_Object &ref_obj);

// Find the front-most immediate child object at the specified
//  location.  If none, then if want_self then if inside in_obj,
//  returns in_obj.   If NOT want_self or NOT inside in_obj, returns
//  Am_No_Object. The coordinate system of x and y is defined
//  w.r.t. ref_obj.   If want_groups is true, the finds the
//   leaf-most element even if it is a group.  If want_groups is
//   false, then will not return a group (if x,y is not over a
//   "primitive" object, returns Am_No_Object)
_OA_DL_IMPORT extern Am_Object Am_Point_In_Part(const Am_Object &in_obj, int x,
                                                int y, const Am_Object &ref_obj,
                                                bool want_self = false,
                                                bool want_groups = true);

// Find the leaf-most object at the specified location.  If x,y is inside
//   in_obj but not over a leaf, then if want_self returns in_obj,
//   otherwise returns Am_No_Object.  If want_groups is true, the finds the
//   leaf-most element even if it is a group.  If want_groups is
//   false, then will not return a group (if x,y is not over a
//   "primitive" object, returns Am_No_Object)
// The coordinate system of x and y is defined w.r.t. ref_obj
_OA_DL_IMPORT extern Am_Object Am_Point_In_Leaf(const Am_Object &in_obj, int x,
                                                int y, const Am_Object &ref_obj,
                                                bool want_self = true,
                                                bool want_groups = true);

// Converts a point in one object's coordinate system, to that of another
// object.  If the objects are not comparable (like being on different screens
// or not being on a screen at all) then the function will return false.
// Otherwise, it will return true and dest_x and dest_y will contain the
// converted coordinates.  Note that the coordinates are for the
// INSIDE of dest_obj.  This means that if "obj" was at src_x, src_y
// in src_obj and you remove it from src_obj and add it to dest_obj at
// dest_x, dest_y then it will be at the same physical screen position.
_OA_DL_IMPORT extern bool Am_Translate_Coordinates(const Am_Object &src_obj,
                                                   int src_x, int src_y,
                                                   const Am_Object &dest_obj,
                                                   int &dest_x, int &dest_y);

_OA_DL_IMPORT extern bool Am_Beep_Happened;

// update all windows- used by event loop
_OA_DL_IMPORT extern void Am_Update_All();

// update a single window
_OA_DL_IMPORT extern void Am_Update(Am_Object window);

_OA_DL_IMPORT extern bool Am_Is_Group_Or_Map(Am_Object &obj);
_OA_DL_IMPORT extern void Am_Initialize();
_OA_DL_IMPORT extern void Am_Cleanup();
_OA_DL_IMPORT extern void Am_Beep(Am_Object window = Am_No_Object);
_OA_DL_IMPORT extern void Am_Move_Object(Am_Object object, Am_Object ref_object,
                                         bool above = true);
_OA_DL_IMPORT extern void Am_To_Top(Am_Object object);
_OA_DL_IMPORT extern void Am_To_Bottom(Am_Object object);
_OA_DL_IMPORT extern bool Am_Do_Events(bool wait = false);
_OA_DL_IMPORT extern void Am_Wait_For_Event();
_OA_DL_IMPORT extern void Am_Main_Event_Loop();
_OA_DL_IMPORT extern void Am_Exit_Main_Event_Loop();
_OA_DL_IMPORT extern Am_Object Am_Create_Screen(const char *display_name);

_OA_DL_IMPORT extern bool am_is_group_and_not_pretending(Am_Object in_obj);

_OA_DL_IMPORT extern void
am_translate_coord_to_me(Am_Object in_obj, Am_Object ref_obj, int &x, int &y);
_OA_DL_IMPORT extern Am_Drawonable *GV_a_drawonable(Am_Object obj);
_OA_DL_IMPORT extern Am_Drawonable *Get_a_drawonable(Am_Object obj);
// Finds the screen that this object is attached to.  (Mostly used in
// conjunction with Am_Get_String_Extents.
_OA_DL_IMPORT extern Am_Object Am_GV_Screen(const Am_Object &self);

// Find the size of a text string given a screen and a font.  (Font size
// depends on the screen.)  The length parameter is the number of characters
// in the string.  Use -1 to mean the entire string.
_OA_DL_IMPORT extern void Am_Get_String_Extents(const Am_Object &screen,
                                                const Am_Font &font,
                                                const char *string, int length,
                                                int &width, int &height);

// Find the width of a text string given a screen and a font.  (Font size
// depends on the screen.)  The length parameter is the number of characters
// in the string.  Use -1 to mean the entire string.
_OA_DL_IMPORT extern int Am_Get_String_Width(const Am_Object &screen,
                                             const Am_Font &font,
                                             const char *string, int length);

// Find the size properties of a font.  (Font size depends on the screen.)
_OA_DL_IMPORT extern void Am_Get_Font_Properties(const Am_Object &screen,
                                                 const Am_Font &font,
                                                 int &max_char_width,
                                                 int &min_char_width,
                                                 int &ascent, int &descent);
