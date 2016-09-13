//
// ------------------------------------------------------------------------
// 		The OpenAmulet User Interface Development Environment
// ------------------------------------------------------------------------
// This code is based on the Amulet project of Carnegie Mellon University,
// and has been placed in the public domain.  If you are using this code
// or any part of OpenAmulet, please contact amulet@cs.cmu.edu to be put
// on the mailing list or have a look at:
// http://www.openip.org
//

// This file contains the implementation of the multi-font, multi-line text
// object.  The design is complicated by the need for all operations to be
// easily undone.
//
// Design notes:
// A text object has associated marks and regions.  Each mark
// and region can only be in one text object at a time, and the
// mark knows which text object it is associated with.  Cursors
// are a subclass of mark and have an associated style with which
// the next characters will be drawn.

#ifndef Rich_TEXT_DEFS_H
#define Rich_TEXT_DEFS_H

#include <am_inc.h>

#include <amulet/typedefs.hpp>
#include <amulet/object.h>
#include <amulet/gdefs.h> // for Am_Style
#include <amulet/gem.h>
#include <amulet/opal.h> // for Am_Line_2

/******************************************************************************
 *  Am_Property_Change_Control
 *    true means set the property to be true, toggle means toggle true/false
 *    throughout region, toggle-first means look at value at mark or beginning
 *    of region and toggle based on that value
 */

enum Am_Property_Change_Control
{
  Am_FONT_PROP_NO_CHANGE,
  Am_FONT_PROP_TRUE,
  Am_FONT_PROP_FALSE,
  Am_FONT_PROP_TOGGLE,
  Am_FONT_PROP_TOGGLE_FIRST
};

/******************************************************************************
 *  Am_Size_Change_Control
 *    can pass an Am_Font_Size_Flag (which is Am_FONT_SMALL = 0,
 *    Am_FONT_MEDIUM = 1, Am_FONT_LARGE = 2, Am_FONT_VERY_LARGE = 3, or an
 *    actual font size as an int (bigger than 3), like 12 for 12 points, or one
 *    of the following special values:
 */

/**** THESE SHOULD BE ADDED TO Am_Font_Size_Flag ****/

enum Am_Size_Change_Control
{
  Am_FONT_SIZE_NO_CHANGE = -1,
  Am_FONT_SIZE_LARGER = -2,
  Am_FONT_SIZE_SMALLER = -3
};

/******************************************************************************
 *  Am_Face_Change_Control
 *    can pass a Am_Font_Family_Flag (which is Am_FONT_FIXED, Am_FONT_SERIF,
 *    Am_FONT_SANS_SERIF ) or the following special value:
 */

/**** THESE SHOULD BE ADDED TO Am_Font_Family_Flag ****/

enum Am_Face_Change_Control
{
  Am_FACE_NO_CHANGE = -1
};

/******************************************************************************
 *  Am_Cursor_Move
 */

enum Am_Cursor_Move
{
  Am_Cursor_Move_Left,
  Am_Cursor_Move_Right,
  Am_Cursor_Move_Up,
  Am_Cursor_Move_Down,
  Am_Cursor_Move_End_Line,
  Am_Cursor_Move_Begin_Line,
  Am_Cursor_Move_Next_Word,
  Am_Cursor_Move_Prev_Word
};

//Am_Cursor_Ref Am_No_Cursor = -1;

/******************************************************************************
 *  Rich_text_demon_bit
 */

const unsigned short rich_text_demon_bit = 0x0002;

/******************************************************************************
 *  Forward Declarations
 */

// forward declarations
class Am_Rich_Text;
class Am_Rich_Text_Data;
class Am_Text_Mark;
class Am_Text_Mark_Data;
class Am_Text_Cursor;
class Am_Text_Cursor_Data;
class Am_Text_Viewing_Context;
class Am_Text_Viewing_Context_Data;
class Am_Text_Fragment;
class Am_Text_Style_Run;

/******************************************************************************
 *  Exported Objects and Functions
 */

_OA_DL_IMPORT extern Am_Object Am_Text_Viewer;

_OA_DL_IMPORT extern Am_Rich_Text Am_No_Rich_Text;
_OA_DL_IMPORT extern Am_Text_Mark Am_No_Text_Mark;
_OA_DL_IMPORT extern Am_Text_Cursor Am_No_Text_Cursor;
_OA_DL_IMPORT extern Am_Text_Viewing_Context Am_No_Text_Viewing_Context;

void Am_Rich_Text_Initialize();

/******************************************************************************
 * Am_Rich_Text
 */

class _OA_DL_CLASSIMPORT Am_Rich_Text
{
  AM_WRAPPER_DECL(Am_Rich_Text)
public:
  Am_Rich_Text();
  Am_Rich_Text(const char *inString);

  inline void Draw_Line(Am_Drawonable *inDrawonable, long inLeft, long inTop,
                        Am_Text_Index inStartIndex, Am_Text_Length inLineLength,
                        unsigned short inMaxAscent);

  inline Am_Text_Fragment *Get_Fragment_At(const Am_Text_Index inIndex,
                                           Am_Text_Length &outRelIndex) const;
  inline Am_Text_Fragment *Get_Fragment_At(const Am_Text_Index inIndex) const;

  inline void Move_Cursor(Am_Cursor &inCursor, Am_Cursor_Move inMove) const;
};

/******************************************************************************
 *  Am_Rich_Text_Data
 */

typedef struct SStyleRunInfo
{
  unsigned long width_used;
  int ascent;
  int descent;
  Am_Text_Fragment *end_frag;
  Am_Text_Length end_frag_offset;
} SStyleRunInfo;

class Am_Rich_Text_Data : public Am_Wrapper
{
  AM_WRAPPER_DATA_DECL(Am_Rich_Text)

public:
  typedef struct SStyleRunInfo SStyleRunInfo;

  Am_Rich_Text_Data();
  Am_Rich_Text_Data(const char *inString, Am_Font inFont = Am_Default_Font,
                    Am_Style inTextStyle = Am_Black,
                    Am_Style inBGStyle = Am_White);

  Am_Rich_Text_Data(Am_Rich_Text_Data *inProto); // needed for the wrapper

  bool operator==(const Am_Rich_Text_Data &inText);
  bool operator==(const Am_Rich_Text_Data &inText) const;

  void Draw_Line(Am_Drawonable *inDrawonable, long inLeft, long inTop,
                 Am_Text_Index inStartIndex, Am_Text_Length inLineLength,
                 unsigned short inMaxAscent);

  bool Calc_Line_Dim(Am_Drawonable *inDrawonable, Am_Text_Index inStartIndex,
                     unsigned long inTargetWidth,
                     Am_Text_Length &outCharsOnLine, unsigned short &outAscent,
                     unsigned short &outDescent);

  // this should probably be private...
  SStyleRunInfo Calc_SR_Dim(Am_Drawonable *inDrawonable,
                            Am_Text_Style_Run *inStyleRun,
                            Am_Text_Fragment *inStartFrag,
                            Am_Text_Offset inFragRelIndex,
                            Am_Text_Index inMaxChars);

  Am_Text_Fragment *Get_Fragment_At(const Am_Text_Index inIndex,
                                    Am_Text_Length &outRelIndex) const;

  Am_Text_Fragment *Get_Fragment_At(const Am_Text_Index inIndex) const;

  Am_Text_Style_Run *Get_Style_Run_At(const Am_Text_Index inIndex,
                                      Am_Text_Length &outRemChars) const;

  Am_Text_Style_Run *Get_Style_Run_At(const Am_Text_Index inIndex) const;

private:
  Am_Text_Fragment *mHeadFragment;
  Am_Text_Style_Run *mHeadStyle;
  Am_Cursor *mHeadCursor; /* treat as an array of cursors */

  /*
  //returns the region used as the main selection
  Am_Text_Region Get_Selection_Region();

  //--------- Styles and Fonts ---------

  //default style or font for new text added after this mark (default = main
  //cursor)
  void Set_Style(Am_Style &new_style,
                 Am_Text_Mark_Data& mark = Am_No_Rich_Text_Mark,
                 Am_Object command_obj = Am_No_Object);
  void Set_Font(Am_Style &new_font,
                Am_Text_Mark_Data& mark = Am_No_Rich_Text_Mark,
                Am_Object command_obj = Am_No_Object);

  //can set, toggle or leave alone the various properties.
  void Set_Font(Am_Property_Change_Control italic,
                Am_Property_Change_Control bold = Am_Font_Prop_No_Change,
                Am_Property_Change_Control underline = Am_Font_Prop_No_Change,
                Am_Size_Change_Control size = Am_Font_Size_No_Change,
                Am_Face_Change_Control face = Am_Font_Face_No_Change,
                Am_Text_Mark_Data& mark = Am_No_Rich_Text_Mark,
                Am_Object command_obj = Am_No_Object);

  Am_Style Get_Style(Am_Text_Mark_Data& mark = Am_No_Rich_Text_Mark);
  Am_Font Get_Font(Am_Text_Mark_Data& mark = Am_No_Rich_Text_Mark);

  //return the style or font at the beginning of the region (region might span
  //multiple styles)
  Am_Style Get_Style(Am_Rich_Text_Region &region = Am_No_Rich_Text_Region);
  Am_Font Get_Font(Am_Rich_Text_Region &region = Am_No_Rich_Text_Region);

  void Change_Region_Style(Am_Style &new_style,
                           Am_Rich_Text_Region &region = Am_No_Rich_Text_Region,
                           Am_Object command_obj = Am_No_Object);
  void Change_Region_Font(Am_Style &new_font,
                          Am_Rich_Text_Region &region = Am_No_Rich_Text_Region,
                          Am_Object command_obj = Am_No_Object);

  //can set, toggle or leave alone the various properties
  void Change_Region_Font(Am_Property_Change_Control italic,
                Am_Property_Change_Control bold = Am_Font_Prop_No_Change,
                Am_Property_Change_Control underline = Am_Font_Prop_No_Change,
                Am_Size_Change_Control size = Am_Font_Size_No_Change,
                Am_Face_Change_Control face = Am_Font_Face_No_Change,
                Am_Rich_Text_Region &region = Am_No_Rich_Text_Region,
                Am_Object command_obj = Am_No_Object);

  //set the styles for all the paragraphs that intersect the region
  void Set_Paragraph_Styles(Am_Text_Paragraph_Fill fill,
                            Am_Text_Paragraph_Justification just,
                            Am_Rich_Text_Region &region = Am_No_Rich_Text_Region,
                            Am_Object command_obj = Am_No_Object);

  //get the paragraph style of the first paragraph that intersects region
  void Get_Paragraph_Styles(Am_Text_Paragraph_Fill &fill,
                            Am_Text_Paragraph_Justification &just,
                            Am_Rich_Text_Region &region = Am_No_Rich_Text_Region,
                            );

  //--------- Getting and searching ---------

  //returns character at the left of the mark.  Returns zero if mark is at
  //beginning of text (so no character at left).  Returns 255 if there is an
  //object to the left of the mark.
  char Get_Char(Am_Text_Mark_Data& which_mark = Am_No_Rich_Text_Mark);

  //returns character or object at left of mark.  Returns Am_No_Value if mark
  //is at beginning of text (so no character at left)
  Am_Value Get(Am_Text_Mark_Data& which_mark = Am_No_Rich_Text_Mark);

  //converts contents of region to a regular C string.  Objects are not
  //included and formatting information is lost.
  Am_String Get_String(Am_Rich_Text_Region &region = Am_No_Rich_Text_Region);

  //searches for the first occurrence of value v, which
  //will usually be a string, character or embedded object, ignoring fonts
  // (and if ignore_case then also ignores capital and lower case),
  //either forwards or backwards, starting from start_mark if from_mark is
  //true, or starting from the beginning or end of the from_mark is false,
  // and moves moving_mark to the found location.
  //Returns true if found.  If not found, returns false, and moving_mark is
  //not changed.  moving_mark and start_mark can be omitted to mean use the
  //default cursor.
  bool Search_For(Am_Value v, bool ignore_case = true,
                  bool forward = true, bool from_mark = true,
                  Am_Text_Mark_Data& start_mark = Am_No_Rich_Text_Mark,
                  Am_Text_Mark_Data& moving_mark = Am_No_Rich_Text_Mark,
                  Am_Object command_obj = Am_No_Object);
  //this version searches for a mark or region of a particular type.  If
  //region, leaves the moving_mark at the beginning if searching forwards, or
  //end if searching backwards.  If searching for a region and already inside
  //that region, will find it.
  bool Search_For(Am_ID_Tag mark_or_region_type,
                  bool forward = true, bool from_mark = true,
                  Am_Text_Mark_Data& start_mark = Am_No_Rich_Text_Mark,
                  Am_Text_Mark_Data& moving_mark = Am_No_Rich_Text_Mark,
                  Am_Object command_obj = Am_No_Object);
  //this version only searches in the region
  bool Search_For(Am_Value v, bool ignore_case = true,
                  bool forward = true,
                  Am_Rich_Text_Region &in_region = Am_No_Rich_Text_Region
                  Am_Text_Mark_Data& moving_mark = Am_No_Rich_Text_Mark,
                  Am_Object command_obj = Am_No_Object);
  //search for mark or region in another region
  bool Search_For(Am_ID_Tag mark_or_region_type,
                  bool forward = true,
                  Am_Rich_Text_Region &in_region = Am_No_Rich_Text_Region
                  Am_Text_Mark_Data& moving_mark = Am_No_Rich_Text_Mark,
                  Am_Object command_obj = Am_No_Object);
*/
};

/******************************************************************************
 *  Am_Text_Mark
 */

class _OA_DL_CLASSIMPORT Am_Text_Mark
{
  AM_WRAPPER_DECL(Am_Text_Mark)
public:
  Am_Text_Mark();
  Am_Text_Mark(const Am_Rich_Text *inText,
               Am_Text_Index inWhere = 0, // by default create a beginning
               Am_Value inMarkData = Am_No_Value, bool inStickyLeft = true,
               bool inDeleteable = true, bool inVisible = false);
  /* The code for this method is commented out in rich_text.cc
  Am_Text_Mark( const Am_Text_Mark& inRefMark,
                Am_Value inMarkData = Am_No_Value,
                bool inStickyLeft   = true,
                bool inDeleteable   = true,
                bool inVisible      = false );
  */
  inline Am_Text_Index Get_Index() const;
  inline Am_Text_Index Get_Absolute_Index() const;

  inline Am_Text_Mark *Get_Next();
  inline void Set_Next(Am_Text_Mark *inNext);

private:
  Am_Text_Mark *mNext;
};

/******************************************************************************
 *  Am_Text_Mark_Data
 */

class Am_Text_Mark_Data : public Am_Wrapper
{
  AM_WRAPPER_DATA_DECL(Am_Text_Mark)
  friend class Am_Text_Fragment;

public:
  Am_Text_Mark_Data(Am_Text_Mark_Data *inProto);
  Am_Text_Mark_Data(const Am_Rich_Text *inText,
                    Am_Text_Index inWhere = 0, // default: at beginning
                    Am_Value inMarkData = Am_No_Value, bool inStickyLeft = true,
                    bool inDeleteable = true, bool inVisible = false);
  /*
  // create new mark at same place as old mark
  Am_Text_Mark_Data( const Am_Text_Mark_Data* inRefMark,
                     Am_Value inMarkData = Am_No_Value,
                     bool inStickyLeft   = true,
                     bool inDeleteable   = true,
                     bool inVisible      = false );
*/
  //  void Destroy();

  // ********** Rich_lags **********
  enum
  {
    kFalseFlag = 0x00000000,
    kStickyBitFlag = 0x00000001,
    kDeleteableFlag = 0x00000002,
    kVisibleFlag = 0x00000004,
    kSyncedWithFragFlag = 0x00000008,

    kStickyLeft = kStickyBitFlag,
    kStickyRight = kFalseFlag
  };

  // -- Sticky Bit --
  inline void Set_Sticky_Left(bool inStickyBit);
  inline bool Is_Sticky_Left() const;
  inline bool Is_Sticky_Right() const;

  // -- Deleteable Bit --
  inline void Set_Deleteable(bool inDeleteable);
  inline bool Is_Deleteable() const;

  // -- Visible Bit --
  inline void Set_Visible(bool inVisible);
  inline bool Is_Visible() const;

  // -- SyncedWithFrag --
  inline void Set_Synced_With_Frag(bool inSyncedWithFrag);
  inline bool Is_Synced_With_Frag() const;

  // ---------- mData ----------
  inline void Set_Data(Am_Value inData);
  inline Am_Value Get_Data() const;

  // ---------- mValue ----------
  inline void Set_Draw_Style(Am_Style inStyle = Am_Line_2);
  inline Am_Style Get_Draw_Style() const;

  // ---------- mIndex ----------
  inline Am_Text_Index Set_Index(Am_Text_Index inIndex);
  //  inline Am_Text_Index Set_Absolute_Index( Am_Text_Index inIndex );
  inline Am_Text_Index Get_Index() const;
  inline Am_Text_Index Get_Absolute_Index() const;

  inline Am_Text_Fragment *Get_Fragment() const;     // the fragment to which
                                                     // this mark belongs
  inline Am_Rich_Text_Data *Get_Text_Object() const; // the text object to which
                                                     // this mark belongs

  /*
  //--------- Moving Marks ---------

  //move the Mark forward or backwards by amt characters (amt can be
  //negative for backwards)
  //if command_obj is supplied, then it is filled with information needed to
  //undo this operation.
  //returns amt if could move that number of characters, and if got to end or
  //beginning, then returns amount actually moved.  Returns zero if didn't
  //move at all.
  long Move_By(long amt = 1,Am_Object command_obj = Am_No_Object);
  //returns true if moved anywhere and false if already at end
  bool Move_To_End(Am_Object command_obj = Am_No_Object);
  bool Move_To_Beginning(Am_Object command_obj = Am_No_Object);
  //if already at end of line, returns false
  bool Move_To_End_Of_Line(Am_Object command_obj = Am_No_Object);
  bool Move_To_Beginning_Of_Line(Am_Object command_obj = Am_No_Object);

  long Move_By_Words(long amt_words = 1,
                          Am_Object command_obj = Am_No_Object);

  //returns false if already at ref_mark
  bool Move_To(Am_Text_Mark_Data& ref_mark,
                    Am_Object command_obj = Am_No_Object);

*/
  //--------- Mark in region? ---------

  //ask whether the mark is in a region.
  //default region = main selection.
  //Error if region and mark are not w.r.t. same Rich_Text object
  //  bool Mark_In_Region(Am_Rich_Text_Region &which_region = Am_No_Rich_Text_Region);

  bool operator==(const Am_Text_Mark_Data &inOtherMark);
  bool operator==(const Am_Text_Mark_Data &inOtherMark) const;
  //  bool operator!= ( const Am_Text_Mark_Data& inOtherMark ) const;
  //  bool operator<= ( const Am_Text_Mark_Data& inOtherMark ) const;

private:
  Am_Text_Index mIndex;        // the marks index in its Am_Text_Fragment
  unsigned long mFlags;        // a bunch of flags (sticky, deleteable, visible)
  Am_Style mStyle;             // the style of the mark when drawn
  Am_Value mData;              // an arbitrary Am_Value which the mark holds
  Am_Text_Fragment *mFragment; // the Am_Text_Fragment which contains this
                               // mark can get the Am_Rich_Text object by
                               // calling Get_Text_Object on the fragment
};

/******************************************************************************
 *  Am_Text_Cursor
 */

class _OA_DL_CLASSIMPORT Am_Text_Cursor
{
  AM_WRAPPER_DECL(Am_Text_Cursor)
public:
  Am_Text_Cursor();
  Am_Text_Cursor(const Am_Rich_Text *inText, Am_Value inMarkData = Am_No_Value,
                 Am_Text_Index inWhere = 0, // default: at beginning
                 bool inStickyLeft = true, bool inDeleteable = true,
                 bool inVisible = true);
};

/******************************************************************************
 *  Am_Text_Cursor_Data
 */

class Am_Text_Cursor_Data : public Am_Text_Mark_Data
{ /* subclass of Am_WRAPPER */
  AM_WRAPPER_DATA_DECL(Am_Text_Cursor)
public:
  Am_Text_Cursor_Data(Am_Text_Cursor_Data *inProto);
  Am_Text_Cursor_Data(const Am_Rich_Text *inText,
                      Am_Text_Index inWhere = 0, // default: at beginning
                      Am_Value inMarkData = Am_No_Value,
                      bool inStickyLeft = true, bool inDeleteable = true,
                      bool inVisible = true);

  void Add_Char(const char inChar);
  void Add_String(const char *inString);
  void Add_Object(Am_Object inObject);
  void Add_Newline();

  void Change_Font(Am_Font inFont);
  void Change_Text_Style(Am_Style inTextStyle);
  void Change_BG_Style(Am_Style inBGStyle);

  bool operator==(const Am_Text_Cursor_Data &inOtherCursor);
  bool operator==(const Am_Text_Cursor_Data &inOtherCursor) const;
  //  bool operator!=( const Am_Text_Cursor_Data& inOtherCursor ) const;

  /*
  // create new cursor at same place as old cursor
  Am_Text_Cursor( const Am_Text_Cursor& inRefCursor,
                    Am_Value inMarkData = Am_No_Value,
                    bool inStickyLeft = true,
                    bool inDeleteable = true );

  //--------- Adding to the string ---------

  //if no font and/or style, uses default
  void Add_Char(char c, Am_Font font = Am_No_Font,
                Am_Style style = Am_No_Style,
                Am_Object command_obj = Am_No_Object);
  void Add_String(const char* s, Am_Font font = Am_No_Font,
                  Am_Style style = Am_No_Style,
                  Am_Object command_obj = Am_No_Object);
  void Add_Object(Am_Object o,
                  short v_alignment,
                  Am_Object command_obj = Am_No_Object);
  void Add_Newline(Am_Object command_obj = Am_No_Object);

  //--------- Deleting from the string ---------

  //amt is number of chars, -1 is backwards, +1 is forwards
  void Delete_Chars(long amt = -1,
                    Am_Object command_obj = Am_No_Object);

  void Delete_Words(long amt_words = -1,
                    Am_Object command_obj = Am_No_Object);

  void Delete_To_End_Of_Line(Am_Object command_obj = Am_No_Object);
*/

private:
  Am_Font mFont;
  Am_Style mStyle;
};

/*

class Am_Rich_Text_Region {
  AM_WRAPPER_DECL (Am_Rich_Text_Region)
public:
  Am_Rich_Text_Region ();
    //create a new region based on another region.
  Am_Rich_Text_Region (Am_Rich_Text_Region const & ref_region,
                    Am_Value region_data = Am_No_Value);
  //create a new region based on two marks.  Default is a zero-length region
  //at the main cursor
  Am_Rich_Text_Region (Am_Text_Mark_Data const& end1_mark = Am_No_Rich_Text_Mark,
                         Am_Text_Mark_Data const & end2_mark = Am_No_Rich_Text_Mark,
                    Am_Value region_data = Am_No_Value);

  //destroys the region data structure, but does not affect the contents of
  //the region in the Rich_Text object: see Delete_Contents for that
  void Destroy();

  bool operator== (const Am_Rich_Text_Region& other_region) const;
  bool operator!= (const Am_Rich_Text_Region& other_region) const;
  bool Inside (const Am_Text_Mark_Data& mark) const;

  void Move_End1_To(Am_Text_Mark_Data& ref_mark = Am_No_Rich_Text_Mark);
  void Move_End2_To(Am_Text_Mark_Data& ref_mark = Am_No_Rich_Text_Mark);

  Am_Text_Mark_Data Create_Mark_At_End1(Am_Value mark_data = Am_No_Value);
  Am_Text_Mark_Data Create_Mark_At_End2(Am_Value mark_data = Am_No_Value);

  //these copy the data from the region to the new mark
  Am_Text_Mark_Data Create_Mark_At_End1();
  Am_Text_Mark_Data Create_Mark_At_End2();

  //If not deleteable, then becomes a zero size region at the beginning of the
  //deleted area.
  //If deleteable, then goes away (becomes not Valid) when area deleted
  void Set_Deleteable(bool deleteable);
  bool Get_Deleteable();

  void Set_Data(Am_Value data);
  Am_Value Get_Data();

  //--------- Controlling display of regions ---------

  void Set_Visible(bool visible);
  bool Get_Visible();

  //normally regions are made visible by inverting the text colors, but if the
  //following routine is used, then will use the specified colors for the
  //region.  Note: this doesn't change the color of the actual text in the
  //region, just how the region is made visible as the region moves around
  // If text is in multiple regions, resulting color is controlled by the
  // region closest to the character.
  void Set_Region_Style(Am_Style foreground_color = Am_White,
                        Am_Style background_color = Am_Black);
  void Get_Region_Style(Am_Style &foreground_color,
                        Am_Style &background_color);

  //--------- Region Contents ---------

  //if region is deleteable, then it goes away also.
  void Delete_Contents(Am_Object command_obj = Am_No_Object);

  //Useful for replacing the entire contents of a region with a new string.
  //Could alternatively delete the region contents, set a mark at region_end1,
  //then add the new contents.
  void Replace_Contents(const char* s,
                        Am_Font font = Am_No_Font,
                        Am_Style style = Am_No_Style,
                        Am_Object command_obj = Am_No_Object);

  //creates and returns a new Rich_Text object containing a copy of
  //the region's contents
  Am_Rich_Text Copy_Region_Contents();

  //contents as string, up to length max_length
  void Copy_Region_Contents(char *s, long max_length);

}
*/

// std::ostream& operator<< ( std::ostream& os, Am_Text_Mark_Data& mark );
// std::ostream& operator<< ( std::ostream& os, Am_Rich_Text_Region& region );

/******************************************************************************
 *  Am_Rich_Text_Viewing_Context
 */

class _OA_DL_CLASSIMPORT Am_Text_Viewing_Context
{
  AM_WRAPPER_DECL(Am_Text_Viewing_Context)
public:
  Am_Text_Viewing_Context();
  void Set_Text(const Am_Rich_Text &inText);
  void Set_Width(const unsigned long inWidth);
  // probably not a good idea, since drawing can modify the object
  // should we make the user get the object... probably not, since
  // most of this code should be internal. Make sure that Make_Unique
  // is called.
  void Draw(Am_Drawonable *inDrawonable, long inLeft, long inTop);
};

/******************************************************************************
 *  Am_Text_Viewing_Context_Data
 */

const unsigned long kLinesInBlock = 16;

class Am_Text_Viewing_Context_Data : public Am_Wrapper
{
  AM_WRAPPER_DATA_DECL(Am_Text_Viewing_Context)
public:
  Am_Text_Viewing_Context_Data();
  Am_Text_Viewing_Context_Data(Am_Text_Viewing_Context_Data *inProto);
  Am_Text_Viewing_Context_Data(Am_Text_Viewing_Context_Data &inProto);
  ~Am_Text_Viewing_Context_Data();

  // destructor to release mText
  void Init(Am_Rich_Text inText, unsigned long inWidth);

  void Layout(Am_Drawonable *inDrawonable, Am_Text_Index inFirstInvChar = 0,
              Am_Text_Index inLastInvChar = 0xFFFFFFFF);
  void Draw(Am_Drawonable *inDrawonable, long inLeft, long inTop);

  inline void Set_Text(const Am_Rich_Text &inText);
  inline void Set_Width(unsigned long inHeight);
  void Invalidate_Text_Range(Am_Text_Index inStart, Am_Text_Length inLength);

  bool operator==(const Am_Text_Viewing_Context_Data &inOtherVC);
  bool operator==(const Am_Text_Viewing_Context_Data &inOtherVC) const;

private:
  struct SLIBlock
  {
    // line info declaration
    typedef struct SLineInfo
    {
      Am_Text_Index fFirst;
      Am_Text_Index fLength;
      unsigned long fHeight;
    } SLineInfo;

    // initializer
    // 5/7/97 eab: moved constructor definition to rich_text.cc
    // to support CC compiler (does not allow for loops in inline code)
    SLIBlock();

    // fields
    SLineInfo li[kLinesInBlock];
    SLIBlock *fNext;
  };

  bool mLIValid;      // has the width changed or has the text changed
  Am_Rich_Text mText; // should there be a destructor to release this
  Am_Cursor_Ref mCursorRef;
  unsigned long mWidth;
  unsigned long mNumLines;
  SLIBlock *mFirstLIBlock;
};

/******************************************************************************
 *  Am_Text_Fragment
 */

const int kFragStrSize = 1024;
const int kMinFragBreak = kFragStrSize / 2;

class _OA_DL_CLASSIMPORT Am_Text_Fragment
{
public:
  Am_Text_Fragment(Am_Rich_Text_Data *inTextObject, const char *inString);
  Am_Text_Fragment(Am_Rich_Text_Data *inTextObject, const char *inString,
                   Am_Text_Length inStrLen);

  inline Am_Rich_Text_Data *Text_Object() const;
  inline Am_Text_Index Start_Index() const;
  inline Am_Text_Length Length() const;
  inline char *String(Am_Text_Offset inOffset = 0) const;
  inline Am_Text_Index Change_Start_Index(Am_Text_Index inDelta);

  void Add_Mark(Am_Text_Mark *inMarkToAdd);
  void Break_At(Am_Text_Index inRelIndex);

  // Linked List management code
  inline Am_Text_Fragment *Next();
  inline Am_Text_Fragment *Prev();

private:
  Am_Value_Type mType;
  Am_Rich_Text_Data *mTextObject; // the text object who owns this fragment
  Am_Text_Index mStartIndex;
  Am_Text_Index mStrLen; // does include '\0' as usual
  Am_Text_Mark *mFirstMark;
  Am_Text_Fragment *mPrev;
  Am_Text_Fragment *mNext;
  char mString[kFragStrSize];
};

/******************************************************************************
 *  Am_Text_Style_Run
 */

class _OA_DL_CLASSIMPORT Am_Text_Style_Run
{
public:
  Am_Text_Style_Run(unsigned long inCharsInRun, Am_Font inFont,
                    Am_Style inTextStyle, Am_Style inBGStyle);

  inline unsigned long Length() const;
  inline unsigned long Change_Length(Am_Text_Offset inDelta);
  inline Am_Text_Style_Run *Next() const;
  inline void Set_Next(Am_Text_Style_Run *inNext);

  inline Am_Font Get_Font() const;
  inline Am_Style Get_TextStyle() const;
  inline Am_Style Get_BGStyle() const;

private:
  unsigned long mCharsInRun;
  Am_Font mFont;
  Am_Style mTextStyle;
  Am_Style mBGStyle;
  Am_Text_Style_Run *mNext;
};

/******************************************************************************
 *  Am_Text_Style_Run_Block
 */

//class Am_Text_Style_Run_Block
//{
//public:
//  Am_Text_Style_Run_Block();
//
//private:
//  unsigned long             mAlloc;
//  Am_Text_Style_Run         mArr[sizeof(unsigned long)*8]; // should be 32
//  Am_Text_Style_Run_Block*  mNext;
//};

/*

//--------- Iterators ---------

class _OA_DL_CLASSIMPORT Am_Text_Mark_Data_Iterator {
  Am_Text_Mark_Data_Iterator ();

  //iterate through all marks starting from around_mark and moving either
  //forward or backwards.  Does not move around_mark
  Am_Text_Mark_Data_Iterator (Am_Text_Mark_Data from_mark, bool forward = true);
  void Start ();                  // Begin list at the start.
  void Next ();                   // Move to next element in list.
  bool Last ();                   // Is this the last element?
  Am_Text_Mark_Data Get ();          // Get the current element.
private:
  Am_Text_Mark_Data current;
  Am_Text_Mark_Data reference;
  bool forw;
}

class _OA_DL_CLASSIMPORT Am_Rich_Text_Region_Iterator {
  Am_Rich_Text_Region_Iterator ();

  //If outwards is false, then iterates through all regions starting from
  //around_mark and moving either forwards or backwards.  First lists all
  //regions that mark is already inside of, and then other regions found in
  //the text after that.  If outwards is true, then ONLY lists regions that
  //mark is inside of.
  Am_Rich_Text_Region_Iterator (Am_Text_Mark_Data around_mark,
                             bool forward = true, bool outwards = false);
  void Start ();                  // Begin list at the start.
  void Next ();                   // Move to next element in list.
  bool Last ();                   // Is this the last element?
  Am_Rich_Text_Region Get ();              // Get the current element.
private:
  Am_Rich_Text_Region current;
  Am_Rich_Text_Region reference;
  bool forw;
  bool outw;
}

*/
#endif
