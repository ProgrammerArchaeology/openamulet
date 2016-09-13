/**************************************************************************
 *         The Amulet User Interface Development Environment              *
 **************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 **************************************************************************/

/* This file contains the implementation of the multi-font, multi-line text
   object.  The design is complicated by the need for all operations to be
   easily undone.

   Design notes:
     A text object has associated marks and regions.  Each mark
     and region can only be in one text object at a time, and the
     mark knows which text object it is associated with.  Cursors
     are a subclass of mark and have an associated style with which
     the next characters will be drawn.
*/

#include <ctype.h>
#include <string.h>

#include <am_inc.h>

#include <amulet/rich_text.h>
#include <amulet/opal_advanced.h>
#include <amulet/standard_slots.h>
#include <amulet/initializer.h>

// ???? why is something like this not defined already ????
#define max(x, y) ((x) > (y) ? (x) : (y))
#define min(x, y) ((x) > (y) ? (y) : (x))

// prints out status messages to aid in testing/debugging
#define DEBUG_HELP 0

/******************************************************************************
 *  Initialize Rich Text Widget
 *****************************************************************************/

/******************************************************************************
 *  Global Objects
 */

Am_Object Am_Text_Viewer;

Am_Rich_Text Am_No_Rich_Text;
Am_Text_Mark Am_No_Text_Mark;
Am_Text_Cursor Am_No_Text_Cursor;
Am_Text_Viewing_Context Am_No_Text_Viewing_Context;

/******************************************************************************
 *  draw_Rich_text
 */

Am_Define_Method(Am_Draw_Method, void, draw_rich_text,
                 (Am_Object self, Am_Drawonable *drawonable, int x_offset,
                  int y_offset))
{
  int left = self.Get(Am_LEFT);
  int top = self.Get(Am_TOP);
  int width = self.Get(Am_WIDTH);
  int height = self.Get(Am_HEIGHT);

  Am_Value value;
  self.Make_Unique(Am_TEXT_VIEWER);
  value = self.Peek(Am_TEXT_VIEWER);

  if (!Am_Text_Viewing_Context::Test(value))
    Am_ERRORO(
        "The slot Am_TEXT_VIEWER does not contain an Am_Text_Viewing_Context.",
        self, Am_TEXT_VIEWER);

  Am_Text_Viewing_Context vc = value;
  if (vc.Valid()) {
    drawonable->Push_Clip(x_offset + left, y_offset + top, width, height);
    vc.Draw(drawonable, x_offset + left, y_offset + top);
    drawonable->Pop_Clip();
  } else
    Am_ERRORO("Tried to draw an invalid Am_Text_Viewing_Context.", self,
              Am_TEXT_VIEWER);
}

/******************************************************************************
 *  rich_text_demon
 */

void
rich_text_demon(Am_Slot inSlot)
{
#if DEBUG_HELP
  std::cout << "Entered Rich_text_demon with inSlot:\n  ";
  inSlot.Text_Inspect();
#endif

  Am_Object_Advanced self = inSlot.Get_Owner();
  Am_Slot_Key slot_key = inSlot.Get_Key();
  Am_Value value;

  // Get the object in Am_TEXT_VIEWER and see if it is a valid
  // Am_Text_Viewing_Context. The object will be modified so make sure that
  // it is unique.
  self.Make_Unique(Am_TEXT_VIEWER);
  value = self.Peek(Am_TEXT_VIEWER);
  if (!Am_Text_Viewing_Context::Test(value))
    Am_ERRORO(
        "The slot Am_TEXT_VIEWER does not contain an Am_Text_Viewing_Context.",
        self, Am_TEXT_VIEWER);

  Am_Text_Viewing_Context vc = value;
  if (!vc.Valid())
    return;

  // At this point we have established that we have a valid viewing context

  if (slot_key == Am_WIDTH) {
    vc.Set_Width((long)self.Get(Am_WIDTH));
    return;
  }

  if (slot_key == Am_TEXT) {
    value = self.Peek(Am_TEXT);
    if (Am_Rich_Text::Test(value)) {
      Am_Rich_Text rich_text = value;
      if (!rich_text.Valid())
        return;
      vc.Set_Text(rich_text);
    }
  }
}

/******************************************************************************
 *  Am_Rich_Text_Initialize
 */

void
Am_Rich_Text_Initialize()
{
#ifdef DEBUG
  Am_Register_Slot_Key(Am_TEXT_VIEWER, "~TEXT_VIEWER~");
#endif

  Am_Text_Viewer = Am_Graphical_Object.Create(DSTR("Rich_Text_Viewer"));

  // Installing the demon procedure Rich_text_demon for Am_TEXT and Am_WIDTH

  // Get the advanced object of Am_Text_Viewer
  Am_Object_Advanced obj_adv_viewer = (Am_Object_Advanced &)Am_Text_Viewer;

  // Make a copy of the inherited demon set.
  Am_Demon_Set demons(obj_adv_viewer.Get_Demons().Copy());

  // Add the Rich_text_demon to the demon set using Rich_text_demon_bit
  demons.Set_Slot_Demon(rich_text_demon_bit, rich_text_demon,
                        Am_DEMON_ON_CHANGE | Am_DEMON_PER_SLOT);

  // Put the modified demon set back into the Am_Text_Viewer
  obj_adv_viewer.Set_Demons(demons);

  // Add the Rich_text_demon_bit to the demon bits of the
  // Am_WIDTH and Am_TEXT slots
  Am_Slot slot;
  unsigned short prev_bits;

  slot = obj_adv_viewer.Get_Slot(Am_WIDTH);
  prev_bits = slot.Get_Demon_Bits();
  slot.Set_Demon_Bits(rich_text_demon_bit | prev_bits);

  slot = obj_adv_viewer.Get_Slot(Am_TEXT);
  prev_bits = slot.Get_Demon_Bits();
  slot.Set_Demon_Bits(rich_text_demon_bit | prev_bits);

  // Set the demon mask
  unsigned short mask = obj_adv_viewer.Get_Demon_Mask();
  mask |= rich_text_demon_bit;
  obj_adv_viewer.Set_Demon_Mask(mask);

  // Set the initial values now that the demon procedures are installed
  Am_Text_Viewer.Set(Am_TOP, 0)
      .Set(Am_LEFT, 0)
      .Set(Am_WIDTH, 200)
      .Set(Am_HEIGHT, 200)
      .Set(Am_DRAW_METHOD, draw_rich_text)
      .Add(Am_TEXT, Am_No_Rich_Text) // should be a Am_Rich_Text object
      .Add(Am_TEXT_VIEWER, Am_Text_Viewing_Context());
}

static Am_Initializer *rich_text_init =
    new Am_Initializer(DSTR("Rich_Text"), Am_Rich_Text_Initialize, 6.0);

/******************************************************************************
 *  Am_Rich_Text
 *****************************************************************************/

AM_WRAPPER_IMPL(Am_Rich_Text)

/******************************************************************************
 *  Am_Rich_Text
 */

Am_Rich_Text::Am_Rich_Text() { data = (0L); }

Am_Rich_Text::Am_Rich_Text(const char *inString)
{
  data = new Am_Rich_Text_Data(inString);
}

/******************************************************************************
 *  Draw_Line
 */

inline void
Am_Rich_Text::Draw_Line(Am_Drawonable *inDrawonable, long inLeft, long inTop,
                        Am_Text_Index inStartIndex, Am_Text_Length inLineLength,
                        unsigned short inAscent)
{
  data->Draw_Line(inDrawonable, inLeft, inTop, inStartIndex, inLineLength,
                  inAscent);
}

/******************************************************************************
 *  Get_Fragment_At
 *    Returns the fragment contains the character specified by inIndex.
 *    On output the second parameter returns the relative position of the
 *    character within the fragment. The second version of this function
 *    does not return the relative value.
 */

inline Am_Text_Fragment *
Am_Rich_Text::Get_Fragment_At(const Am_Text_Index inIndex,
                              Am_Text_Length &outRelIndex) const
{
  return data->Get_Fragment_At(inIndex, outRelIndex);
}

inline Am_Text_Fragment *
Am_Rich_Text::Get_Fragment_At(const Am_Text_Index inIndex) const
{
  return data->Get_Fragment_At(inIndex);
}

/******************************************************************************
 *  New_Cursor_Ref
 */

/*
Am_Cursor_Ref
Am_Rich_Text::New_Cursor(
  Am_Text_Viewing_Context )
{
// NDY

  return -1;
}
*/

/******************************************************************************
 *  Cursor_Move
 */

/*
inline void
Am_Rich_Text::Cursor_Move(
  Am_Cursor&     inCursor,
  Am_Cursor_Move inMove )
{
// NDY

}
*/

/******************************************************************************
 *  Am_Rich_Text_Data
 *****************************************************************************/

AM_WRAPPER_DATA_IMPL(Am_Rich_Text, (this))

/******************************************************************************
 *  Am_Rich_Text_Data
 *    First create a single empty fragment
 */

Am_Rich_Text_Data::Am_Rich_Text_Data()
{
  mHeadFragment = (0L);
  mHeadStyle = (0L);
}

Am_Rich_Text_Data::Am_Rich_Text_Data(const char *inString,
                                     Am_Font inFont, // default: Am_Default_Font
                                     Am_Style inTextStyle, // default: Am_Black
                                     Am_Style inBGStyle)   // default: Am_White
{
  //  int len = strlen( inString );

  mHeadFragment = new Am_Text_Fragment(this, inString);

  mHeadStyle =
      new Am_Text_Style_Run(strlen(inString), inFont, inTextStyle, inBGStyle);

  /*
  mHeadFragment = new Am_Text_Fragment( this, inString );
  mHeadFragment->Break_At( 200 );
  mHeadFragment->Break_At( 150 );
  mHeadFragment->Break_At( 100 );
  mHeadFragment->Break_At(  50 );
  mHeadStyle    = new Am_Text_Style_Run( 2, Am_Font( Am_FONT_SERIF, true,
                                                       false, false,
                                                       Am_FONT_VERY_LARGE ),
                                           Am_Blue, Am_White );

  Am_Text_Style_Run* last = mHeadStyle;
  last->Set_Next(
    new Am_Text_Style_Run( 19, Am_Default_Font, Am_Black, Am_White ) );
  last = last->Next();
  last->Set_Next(
    new Am_Text_Style_Run( 14, Am_Default_Font, Am_White, Am_Black ) );
  last = last->Next();
  last->Set_Next(
    new Am_Text_Style_Run( 292, Am_FONT_SERIF, Am_Red, Am_White ) );
  last = last->Next();
*/
}

Am_Rich_Text_Data::Am_Rich_Text_Data(Am_Rich_Text_Data * /* inProto */)
{
  // should copy all the data over to this object
  // fragment and styles -- need to change their owner if they have one
  Am_Error("** Am_Rich_Text_Data constructor. Tried to make a new object from "
           "a constructor. Most like you tried to Make_Unique on Am_Rich_Text. "
           "This function is not yet supported.");
}

/******************************************************************************
 *  find_next_break
 *    Assumes are characters in inStr are '\n' or 0x20 <= c <= 0x7E.
 */

enum EBreakTypes
{
  Space,
  EOLN,
  EndOfText
};
struct SBreakInfo
{
  EBreakTypes why;
  unsigned long len;
  Am_Text_Fragment *frag;
};

struct SBreakInfo Find_Next_Break(
    Am_Text_Index inRelIndex,      // relative index into inStartFrag
    Am_Text_Fragment *inStartFrag) // on input, the starting fragment
{
  SBreakInfo ret;
  ret.len = 0;
  ret.frag = inStartFrag;

  Am_Text_Length charsRemInFrag = ret.frag->Length() - inRelIndex;
  char *str = ret.frag->String(inRelIndex);

  // find the first graphical character
  while (true) {
    // have we reached the end of the fragment?
    if (charsRemInFrag <= 0) {
      ret.frag = ret.frag->Next();

      if (ret.frag == (0L)) {
        ret.why = EndOfText;
        return ret;
      } else {
        charsRemInFrag = ret.frag->Length();
        str = ret.frag->String(0);
      }
    }

    if (isgraph(*str))
      break;

    ret.len++;
    str++;
    charsRemInFrag--;
  }

  // find first non-graphical character or '\n'
  while (true) {
    // have we reached the end of the fragment?
    if (charsRemInFrag <= 0) {
      ret.frag = ret.frag->Next();

      if (ret.frag == (0L)) {
        ret.why = EndOfText;
        return ret;
      } else {
        charsRemInFrag = ret.frag->Length();
        str = ret.frag->String(0);
      }
    }

    if (!isgraph(*str)) {
      if ((*str) == '\n')
        ret.why = EOLN;
      else
        ret.why = Space;

      break;
    }
    ret.len++;
    str++;
    charsRemInFrag--;
  }

  return ret;
}

/******************************************************************************
 *  Calc_Line_Dim
 *
 *    Calculates the dimensions for a line of text.
 *
 *    Input:  - drawonable
 *            - start index
 *            - target width (in pixels)
 *
 *    Output: - char on line
 *            - ascent
 *            - descent
 *
 *    Returns true if reached the end of text, false otherwise
 */

bool
Am_Rich_Text_Data::Calc_Line_Dim(Am_Drawonable *inDrawonable,
                                 Am_Text_Index inStartIndex,
                                 unsigned long inTargetWidth,
                                 Am_Text_Length &outCharsOnLine,
                                 unsigned short &outAscent,
                                 unsigned short &outDescent)
{
  unsigned long cumChars = 0, cumWidth = 0;
  Am_Text_Index relSRIndex = 0, chars_rem_in_sr = 0;
  SBreakInfo break_info;
  SStyleRunInfo sr_info;

  outCharsOnLine = 0;
  outAscent = 0;
  outDescent = 0;

  sr_info.end_frag = Get_Fragment_At(inStartIndex, sr_info.end_frag_offset);
  Am_Text_Style_Run *curSR = Get_Style_Run_At(inStartIndex, relSRIndex);

  chars_rem_in_sr = curSR->Length() - relSRIndex + 1; // incl cur char too

  // COMMENT .....
  while (true) {
    break_info = Find_Next_Break(sr_info.end_frag_offset, sr_info.end_frag);

    // this loop calculates how many pixels are needed to print until
    // the next break. There may be more than one style run until that break

    Am_Text_Length break_chars_used = 0, max_chars = 0;
    unsigned long break_width_used = 0;
    int break_ascent = 0, break_descent = 0;

    // Continue stepping through the style runs, until having processed
    // break_info.len characters or used up more pixels than were remaining
    while (break_chars_used < break_info.len) {
      max_chars = min(break_info.len - break_chars_used, chars_rem_in_sr);

      sr_info = Calc_SR_Dim(inDrawonable, curSR, sr_info.end_frag,
                            sr_info.end_frag_offset, max_chars);

      // Update data about this break
      break_width_used += sr_info.width_used;
      break_chars_used += max_chars;
      chars_rem_in_sr -= max_chars;

      break_ascent = max(sr_info.ascent, break_ascent);
      break_descent = max(sr_info.descent, break_descent);

      if (break_info.why == EndOfText)
        break;

      // skip to next style run, since there should be more test remaining
      if (chars_rem_in_sr == 0) {
        curSR = curSR->Next();
        if (curSR == (0L)) {
          Am_Error("Ran out of Style_Run info.");
          // this should never happen... eventually throw an exception
        }
        chars_rem_in_sr = curSR->Length();
      }
    }

    if (cumWidth + break_width_used <= inTargetWidth) {
      outCharsOnLine += break_chars_used;

      outAscent = max(break_ascent, outAscent);
      outDescent = max(break_descent, outDescent);
      cumChars += break_chars_used;
      cumWidth += break_width_used;
    } else
      break;

    if (break_info.why == EndOfText)
      break;
  }

  return (break_info.why == EndOfText) ? true : false;
}

/******************************************************************************
 *  Calc_SR_Dim
 *    Calculates the dimensions of a style run.
 *    The character range starts at inStartIndex and runs for inMaxChars.
 *    These values better be within the given style run.
 *    Given a style run and a start index (should be in the given style run).
 *    inCharsRemInSR
 *    Determines how many characters can be drawn with inWidthWanted pixels.
 *
 *    THIS CODE IS IN FLUX; THE ABOVE COMMENTS ARE PROBABLY WRONG!!!
 */

SStyleRunInfo
Am_Rich_Text_Data::Calc_SR_Dim(Am_Drawonable *inDrawonable,
                               Am_Text_Style_Run *inStyleRun,
                               Am_Text_Fragment *inStartFrag,
                               Am_Text_Offset inFragRelIndex,
                               Am_Text_Index inMaxChars)
{
  SStyleRunInfo ret;
  ret.width_used = 0;
  ret.end_frag = inStartFrag;
  ret.end_frag_offset = inFragRelIndex;

  Am_Font font = inStyleRun->Get_Font();

  int ign;
  inDrawonable->Get_Font_Properties(font, ign, ign, ret.ascent, ret.descent);

  char *string = ret.end_frag->String(inFragRelIndex);

  int charsRem = (int)inMaxChars,
      charsRemInFrag = (int)(ret.end_frag->Length() - inFragRelIndex);

  //
  while (true) {
    int charsToMeasure = min(charsRem, charsRemInFrag);

    ret.width_used +=
        inDrawonable->Get_String_Width(font, string, charsToMeasure);

    charsRem -= charsToMeasure;
    ret.end_frag_offset += charsToMeasure;

    if (charsRem > 0) {
      ret.end_frag = ret.end_frag->Next();
      ret.end_frag_offset = 0;

      if (ret.end_frag == (0L))
        Am_Error("Rich Text Engine: Unexpected null fragment in Calc_SR_Dim");

      string = ret.end_frag->String(0);
      charsRemInFrag = (int)ret.end_frag->Length();
    } else
      break;
  }

  return ret;

  /*
  // Searches for the fragment which contains the end of the style run (1)
  //  or whose width would be longer than the wanted width (2)
  // Assumes that string and strLen are correct at top of loop
  while( ret.end_frag != (0L) )
  {
    // See if the style run extends past the ret.end_fragment
    if( (ret.chars_used + charsRemInFrag) > charsRem ) // (1)
      break;
    // Only continues if the style run goes into the next fragment.
    // Therefore ret.end_frag->Next() can not equal (0L). If it did the style
    // run would have been too long resulting in an error (3)

    strWidth = inDrawonable->Get_String_Width( inStyleRun->Get_Font(),
                                               string, charsRem );
    // see if the frags width in pixels is longer that the desired width
//    if( strWidth >= widthRemaining ) // (2)
//      break;

    // skip to next fragment
    ret.width_used += strWidth;
    ret.chars_used += charsRemInFrag;
//    widthRemaining -= strWidth;
    charsRem       -= charsRemInFrag;

    ret.end_frag = ret.end_frag->Next();
//    ret.end_frag_rel_index = 0;
    if( ret.end_frag != (0L) )
    {
      charsRemInFrag = ret.end_frag->Length();
      string = ret.end_frag->String( 0 );
    }
    else
    {
      // This should never happen.
    }
  }

  // outCharsUsed and widthRemaining only go up to the previous fragment
  // At this point frag != (0L). It is an error for this to happen.
  if( ret.end_frag == (0L) ) // (3)
    Am_Error("** In Am_Rich_Text_Data::Calc_Style_Run_Dim: frag == (0L).");

  // At this point we know that we will either
  //   * run out of pixels (width filled)
  //       or
  //   * run out of chars in this style run (end of style)

  Am_Text_Length charCount = 0;
  unsigned long tmpWidth = 0;
  strWidth = 0;
  while( true )
  {
    if( charCount >= charsRem )
    {
      ret.chars_used += charCount; // should be (+= charCount-1)
      ret.width_used += strWidth;
      return ret;
    }

    // we have at least one more char left
    tmpWidth = inDrawonable->Get_String_Width( inStyleRun->Get_Font(),
                                               string, charCount + 1 );
//    if( tmpWidth < widthRemaining )
//    {
      charCount++;
      strWidth = tmpWidth;
//    }
//    else
//    {
//      ret.end_frag_rel_index = charCount;
//      break;
//    }
  }
//  widthRemaining -= strWidth;

  ret.chars_used += charCount;
  ret.width_used = inMaxWidth - widthRemaining;

  return ret;
*/
}

/******************************************************************************
 *  Draw_Line
 */

void
Am_Rich_Text_Data::Draw_Line(Am_Drawonable *inDrawonable, long inLeft,
                             long inTop, Am_Text_Index inStartIndex,
                             Am_Text_Length inLineLength,
                             unsigned short inMaxAscent)
{
  Am_Text_Length remCharsSR = 0, remCharsFrag = 0, relFragIndex = 0,
                 relSRIndex = 0, charsToDraw = 0, charsLeft = inLineLength;
  Am_Text_Fragment *frag = Get_Fragment_At(inStartIndex, relFragIndex);
  Am_Text_Style_Run *sr = Get_Style_Run_At(inStartIndex, relSRIndex);

  remCharsFrag = frag->Length() - relFragIndex;
  remCharsSR = sr->Length() - relSRIndex + 1; // sr is 1-based
  char *string = frag->String() + relFragIndex;

  int srAscent, ignore;
  inDrawonable->Get_Font_Properties(sr->Get_Font(), ignore, ignore, srAscent,
                                    ignore);

  unsigned long top = inTop + inMaxAscent - srAscent;
  unsigned long left = inLeft;

  while (charsLeft > 0) {
    charsToDraw = (remCharsFrag > remCharsSR) ? remCharsSR : remCharsFrag;
    if (charsToDraw > charsLeft)
      charsToDraw = charsLeft;

    inDrawonable->Draw_Text(sr->Get_TextStyle(), string, (int)charsToDraw,
                            sr->Get_Font(), (int)left, (int)top, Am_DRAW_COPY,
                            sr->Get_BGStyle(), false);
    left += inDrawonable->Get_String_Width(sr->Get_Font(), string,
                                           (int)charsToDraw);
    remCharsFrag -= charsToDraw;
    remCharsSR -= charsToDraw;
    charsLeft -= charsToDraw;
    string += charsToDraw;

    if (charsLeft <= 0)
      return;

    if (remCharsFrag <= 0) {
      frag = frag->Next();
      // should check for (0L)
      remCharsFrag = frag->Length();
      string = frag->String();
      relFragIndex = 0;
    }

    if (remCharsSR <= 0) {
      sr = sr->Next();
      // should check for (0L)
      remCharsSR = sr->Length();
      inDrawonable->Get_Font_Properties(sr->Get_Font(), ignore, ignore,
                                        srAscent, ignore);
      top = inTop + inMaxAscent - srAscent;
    }
  }
}

/******************************************************************************
 *  Get_Fragment_At
 *    Given an absolute index returns the fragment which contains that
 *    character. Additionally the first version of this function returns the
 *    relative position of the given character within the fragment. That value
 *    is 0-based.
 */

Am_Text_Fragment *
Am_Rich_Text_Data::Get_Fragment_At(const Am_Text_Index inIndex,
                                   Am_Text_Length &outRelIndex) const
{
  Am_Text_Index endOfFrag = 0;
  Am_Text_Fragment *frag = mHeadFragment;

  while (frag != (0L)) {
    endOfFrag += frag->Length();
    if (inIndex <= endOfFrag) {
      // find the relative index
      outRelIndex = frag->Length() - endOfFrag + inIndex - 1;
      break;
    }
    frag = frag->Next();
  }
  return frag;
}

Am_Text_Fragment *
Am_Rich_Text_Data::Get_Fragment_At(const Am_Text_Index inIndex) const
{
  Am_Text_Index endOfFrag = 0;
  Am_Text_Fragment *frag = mHeadFragment;

  while (frag != (0L)) {
    endOfFrag += frag->Length();
    if (inIndex <= endOfFrag)
      break;
    frag = frag->Next();
  }
  return frag;
}

/******************************************************************************
 *  Get_Style_Run_At
 *    Returns the Style Run which contains the character at index, inIndex.
 *    Note that inIndex is 1-based. outRemChars are the number of character
 *    after the inIndex char that are left in the style run.
 */

Am_Text_Style_Run *
Am_Rich_Text_Data::Get_Style_Run_At(const Am_Text_Index inIndex,
                                    Am_Text_Length &outRemChars) const
{
  Am_Text_Index lastCharInRun = 0;
  Am_Text_Style_Run *sr = mHeadStyle;
  outRemChars = 0; // just in case

  while (sr != (0L)) {
    lastCharInRun += sr->Length();
    if (inIndex <= lastCharInRun) {
      // find the relative index
      outRemChars = sr->Length() - lastCharInRun + inIndex;
      break;
    }
    sr = sr->Next();
  }
  return sr;
}

Am_Text_Style_Run *
Am_Rich_Text_Data::Get_Style_Run_At(const Am_Text_Index inIndex) const
{
  Am_Text_Index endOfRun = 0;
  Am_Text_Style_Run *sr = mHeadStyle;

  while (sr != (0L)) {
    endOfRun += sr->Length();
    if (inIndex <= endOfRun)
      break;
    sr = sr->Next();
  }
  return sr;
}

/******************************************************************************
 *  operator==
 */

bool
Am_Rich_Text_Data::operator==(const Am_Rich_Text_Data & /* inText */)
{
  return false;
}

bool
Am_Rich_Text_Data::operator==(const Am_Rich_Text_Data & /* inText */) const
{
  return false;
}

/******************************************************************************
 *  operator!=
 */

//bool
//Am_Rich_Text_Data::operator!=(
//  const Am_Rich_Text_Data& inText ) const
//{
//  return (*this != inText);
//}

/******************************************************************************
 *  Am_Text_Mark
 *****************************************************************************/

AM_WRAPPER_IMPL(Am_Text_Mark)

/******************************************************************************
 *  Am_Text_Mark
 */

Am_Text_Mark::Am_Text_Mark() { data = (0L); }

Am_Text_Mark::Am_Text_Mark(
    const Am_Rich_Text *inText,
    Am_Text_Index inWhere, // by default create a beginning
    Am_Value inMarkData, bool inStickyLeft, bool inDeleteable, bool inVisible)
{
  data = new Am_Text_Mark_Data(inText, inWhere, inMarkData, inStickyLeft,
                               inDeleteable, inVisible);
}

/*
Am_Text_Mark::Am_Text_Mark(
  const Am_Text_Mark inRefMark,
  Am_Value inMarkData = Am_No_Value,
  bool inStickyLeft = true,
  bool inDeleteable = true,
  bool inVisible    = false )
{
  ;
}
*/

/******************************************************************************
 *  Get_Index && Get_Absolute_Index
 */

inline Am_Text_Index
Am_Text_Mark::Get_Index() const
{
  return data->Get_Index();
}

inline Am_Text_Index
Am_Text_Mark::Get_Absolute_Index() const
{
  return data->Get_Absolute_Index();
}

//
// /******************************************************************************
//  *  Get_Prev && Set_Prev
//  */
//
// inline Am_Text_Mark*
// Am_Text_Mark::Get_Prev()
// {
//   return mPrev;
// }
//
// inline void
// Am_Text_Mark::Set_Prev(
//   Am_Text_Mark* inPrev )
// {
//   mPrev = inPrev;
// }
//

/******************************************************************************
 *  Get_Next && Set_Next
 */

inline Am_Text_Mark *
Am_Text_Mark::Get_Next()
{
  return mNext;
}

inline void
Am_Text_Mark::Set_Next(Am_Text_Mark *inNext)
{
  mNext = inNext;
}

/******************************************************************************
 *  Am_Text_Mark_Data
 *****************************************************************************/

AM_WRAPPER_DATA_IMPL(Am_Text_Mark, (this))

/******************************************************************************
 *  Am_Text_Mark_Data
 */

Am_Text_Mark_Data::Am_Text_Mark_Data(
    const Am_Rich_Text *inText,
    Am_Text_Index inWhere, // default: 0 => create at beginning
    Am_Value inMarkData,   // default: Am_No_Value
    bool inStickyLeft,     // default: true
    bool inDeleteable,     // default: true
    bool inVisible)        // default: false
{
  mFragment = inText->Get_Fragment_At(inWhere);
  mIndex = inWhere; // temp until above works
  Set_Data(inMarkData);
  Set_Sticky_Left(inStickyLeft);
  Set_Deleteable(inDeleteable);
  Set_Visible(inVisible);
  //  mFragment->Add_Mark( this );
}

Am_Text_Mark_Data::Am_Text_Mark_Data(Am_Text_Mark_Data *inProto)
{
  //  Rich_ragment = inText->Get_Fragment_At( inWhere );
  //  Rich_ragment->Add_Mark( this );
  mIndex = inProto->mIndex;
  Set_Data(inProto->Get_Data());
  Set_Sticky_Left(inProto->Is_Sticky_Left());
  Set_Deleteable(inProto->Is_Deleteable());
  Set_Visible(inProto->Is_Visible());
}

/******************************************************************************
 *  Am_Rich_TextMark
 *    create new mark at same place as old mark
 */
/*
Am_Text_Mark_Data::Am_Text_Mark_Data(
  const Am_Text_Mark_Data* inRefMark,
  Am_Value inMarkData,      // default: 0
  bool inStickyLeft,        // default: true
  bool inDeleteable,        // default: true
  bool inVisible )          // default: false
{
  Rich_ragment = inRefMark->Get_Fragment();
  Rich_ragment->Add_Mark( this );

  Set_Data( inMarkData );
  Set_Sticky_Left( inStickyLeft );
  Set_Deleteable( inDeleteable );
  Set_Visible( inVisible );

}
*/

/******************************************************************************
 *  Destroy
 */

/*    ??? Should this be a destructor ???
void
Am_Text_Mark_Data::Destroy()
{
}
*/

/******************************************************************************
 *  Get_Fragment
 *    returns the fragment to which this mark belongs
 */

Am_Text_Fragment *
Am_Text_Mark_Data::Get_Fragment() const
{
  return mFragment;
}

/******************************************************************************
 *  Get_Text_Object
 *    returns the text object to which this mark belongs
 */

Am_Rich_Text_Data *
Am_Text_Mark_Data::Get_Text_Object() const
{
  return mFragment->Text_Object();
}

/******************************************************************************
 *  Set_Sticky_Left && Is_Sticky_Left && Is_Sticky_Right
 *    A mark (M) should be considered as existing between two character,
 *    call the one on the left, L, and the one on the right R => LMR
 *    When one or more characters, an object are inserted between L and R,
 *    or when the am_fragment breaks between L and R, the sticky bit
 *    determines which character the mark still stays associated with.
 *    If the mark is sticky left and I is insert the result would be
 *    LMIR, but if the mark was sticky right the result would be LIMR
 */

inline void
Am_Text_Mark_Data::Set_Sticky_Left(bool inStickyBit)
{
  mFlags |= (inStickyBit) ? kStickyLeft : kStickyRight;
}

inline bool
Am_Text_Mark_Data::Is_Sticky_Left() const
{
  return (mFlags & kStickyBitFlag) ? true : false;
}

inline bool
Am_Text_Mark_Data::Is_Sticky_Right() const
{
  return (mFlags & kStickyBitFlag) ? false : true;
}

/******************************************************************************
 *  Set_Deleteable && Is_Deleteable
 *    If the mark is in an area (a region) which is deleted, the deleteable
 *    bit controls what happens to the mark.
 *    If the mark is deleteable then is goes away (becomes not Valid -
 *    destructor is called).
 *    If the mark is not deleteable, then the mark is placed between the
 *    characters which were previously the first characters left and right
 *    of the deleted region, (ie LDDDDMDDDR => LMR, duh).  However if the
 *    deletion causes a am_fragment break between L and R, then the sticky bit
 *    determines which am_fragment the mark becomes part of.  (obvious the left
 *    am_fragment if sticky left and the right am_fragment if sticky right)
 */

inline void
Am_Text_Mark_Data::Set_Deleteable(bool inDeleteable)
{
  mFlags |= (inDeleteable) ? kDeleteableFlag : kFalseFlag;
}

inline bool
Am_Text_Mark_Data::Is_Deleteable() const
{
  return (mFlags & kDeleteableFlag) ? true : false;
}

/******************************************************************************
 *  Set_Visible && Is_Visible
 *    If true the bit is displayed, if false then are not
 *    NOT YET DETERMINED: how visibility affect an am_fragment's width
 */

inline void
Am_Text_Mark_Data::Set_Visible(bool inVisible)
{
  mFlags |= (inVisible) ? kVisibleFlag : kFalseFlag;
}

inline bool
Am_Text_Mark_Data::Is_Visible() const
{
  return (mFlags & kVisibleFlag) ? true : false;
}

/******************************************************************************
 *  Set_Synced_With_Frag && Is_Synced_With_Frag
 *    This bit is for cursors to determine if they are synchronized with the
 *    fragment they are in. The synchronization deals with if the font, text
 *    style and background style of both the cursor and the fragment are the
 *    same. This allows the cursor to move into different fragments without
 *    breaking the fragment. Fragments only need to be broken if text is added.
 *    Deletions should not require synchronization.
 */

inline void
Am_Text_Mark_Data::Set_Synced_With_Frag(bool inSyncedWithFrag)
{
  mFlags |= (inSyncedWithFrag) ? kSyncedWithFragFlag : kFalseFlag;
}

inline bool
Am_Text_Mark_Data::Is_Synced_With_Frag() const
{
  return (mFlags & kSyncedWithFragFlag) ? true : false;
}

/******************************************************************************
 *  Set_Data && Get_Data
 *    Marks contain an Am_Value for storing information about the mark.
 *    The mark does not use this data for itself.
 */

inline void
Am_Text_Mark_Data::Set_Data(Am_Value inData)
{
  mData = inData;
}

inline Am_Value
Am_Text_Mark_Data::Get_Data() const
{
  return mData;
}

/******************************************************************************
 *  Set_Draw_Style && Get_Draw_Style
 *    Controls the color, thickness, etc.  of the mark when it is visible.
 */

inline void
Am_Text_Mark_Data::Set_Draw_Style(Am_Style inStyle)
{
  mStyle = inStyle;
}

inline Am_Style
Am_Text_Mark_Data::Get_Draw_Style() const
{
  return mStyle;
}

/******************************************************************************
 *  Set_Index
 */

inline Am_Text_Index
Am_Text_Mark_Data::Set_Index(Am_Text_Index inIndex)
{
  return mIndex = inIndex;
}

// NDY : What should this do?
// inline Am_Text_Index
// Am_Text_Mark_Data::Set_Absolute_Index(
//   Am_Text_Index inIndex )
// {
//   return 0;//Rich_ragment->Get_Beg_Index() + mIndex = inIndex;
// }

/******************************************************************************
 *  Get_Index && Get_Absolute_Index
 */

inline Am_Text_Index
Am_Text_Mark_Data::Get_Index() const
{
  return mIndex;
}

inline Am_Text_Index
Am_Text_Mark_Data::Get_Absolute_Index() const
{
  return /* Rich_ragment->Get_Start_Index() */ +mIndex;
}

//  Am_Rich_Text Get_Reference_Text(); // text the mark is with respect to

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

  //--------- Mark in region? ---------

  //ask whether the mark is in a region.
  //default region = main selection.
  //Error if region and mark are not w.r.t. same Rich_Text object
  bool Mark_In_Region(Am_Rich_Text_Region &which_region = Am_No_Rich_Text_Region);
*/

/******************************************************************************
 *  operator== && operator!= && operator<=
 */

bool
Am_Text_Mark_Data::operator==(const Am_Text_Mark_Data & /* inOtherMark */)
{
  return false;
}

bool
Am_Text_Mark_Data::operator==(const Am_Text_Mark_Data & /* inOtherMark */) const
{
  return false;
}

//bool
//Am_Text_Mark_Data::operator!=(
//  const Am_Text_Mark_Data& inOtherMark ) const
//{
//  return (*this != inOtherMark);
//}
/*
bool
Am_Text_Mark_Data::operator<=(
  const Am_Text_Mark_Data& inOtherMark ) const
{
  return false;
}
*/

/******************************************************************************
 *  Am_Text_Cursor
 *****************************************************************************/

AM_WRAPPER_IMPL(Am_Text_Cursor)

/******************************************************************************
 *  Am_Text_Cursor
 */

Am_Text_Cursor::Am_Text_Cursor() { data = (0L); }

Am_Text_Cursor::Am_Text_Cursor(
    const Am_Rich_Text *inText, Am_Value inMarkData,
    Am_Text_Index inWhere, // by default create at beginning
    bool inStickyLeft, bool inDeleteable, bool inVisible)
{
  data = new Am_Text_Cursor_Data(inText, inWhere, inMarkData, inStickyLeft,
                                 inDeleteable, inVisible);
}

/******************************************************************************
 *  Am_Text_Cursor_Data
 *****************************************************************************/

AM_WRAPPER_DATA_IMPL(Am_Text_Cursor, (this))

/******************************************************************************
 *  Am_Text_Cursor_Data
 */

Am_Text_Cursor_Data::Am_Text_Cursor_Data(
    const Am_Rich_Text *inText,
    Am_Text_Index inWhere, // default: 0
    Am_Value inMarkData,   // default: Am_No_Value
    bool inStickyLeft,     // default: true
    bool inDeleteable,     // default: true
    bool inVisible)        // default: true
    : Am_Text_Mark_Data(inText, inWhere, inMarkData, inStickyLeft, inDeleteable,
                        inVisible)
{
  Set_Synced_With_Frag(false);
}

Am_Text_Cursor_Data::Am_Text_Cursor_Data(Am_Text_Cursor_Data *inProto)
    : Am_Text_Mark_Data(inProto)
{
}

/******************************************************************************
 *  Add_Char
 *    Adds the char specifies by inChar to the text object at the current
 *    cursor position.
 */

void
Am_Text_Cursor_Data::Add_Char(const char /* inChar */)
{
}

/******************************************************************************
 *  Add_String
 *    Adds the string specifies by inString to the text object at the current
 *    cursor position.
 */

void
Am_Text_Cursor_Data::Add_String(const char * /* inString */)
{
}

/******************************************************************************
 *  Add_Object
 */

void Am_Text_Cursor_Data::Add_Object(Am_Object /* inObject */)
{
  Am_Error("Adding an object to a Am_Rich_Text is not yet implemented.\nIn "
           "Am_Text_Cursor::Add_Object()");
}

/******************************************************************************
 *  Add_Newline
 */

void
Am_Text_Cursor_Data::Add_Newline()
{
}

/******************************************************************************
 *  Change_Font
 */

void Am_Text_Cursor_Data::Change_Font(Am_Font /* inFont */) {}

/******************************************************************************
 *  Change_Text_Style
 */

void Am_Text_Cursor_Data::Change_Text_Style(Am_Style /* inTextStyle */) {}

/******************************************************************************
 *  Change_BG_Style
 */

void Am_Text_Cursor_Data::Change_BG_Style(Am_Style /* inBGStyle */) {}

/******************************************************************************
 *  operator== && operator!= && operator<=
 */

bool
Am_Text_Cursor_Data::operator==(const Am_Text_Cursor_Data &inOtherCursor)
{
  return (*this == inOtherCursor);
}

bool
Am_Text_Cursor_Data::operator==(const Am_Text_Cursor_Data &inOtherCursor) const
{
  return (*this == inOtherCursor);
}

//bool
//Am_Text_Cursor_Data::operator!=(
//  const Am_Text_Cursor_Data& inOtherCursor ) const
//{
//  return (*this != inOtherCursor);
//}
/*
bool
Am_Text_Cursor_Data::operator<=(
  const Am_Text_Cursor_Data& inOtherCursor ) const
{
  return false;
}
*/

/******************************************************************************
 *  Am_Text_Viewing_Context
 *****************************************************************************/

AM_WRAPPER_IMPL(Am_Text_Viewing_Context)

/******************************************************************************
 *  Am_Text_Viewing_Context
 */

Am_Text_Viewing_Context::Am_Text_Viewing_Context()
{
  data = new Am_Text_Viewing_Context_Data();
}

/******************************************************************************
 *  Set_Text
 */

/**** NDY: should Make_Unique ****/

void
Am_Text_Viewing_Context::Set_Text(const Am_Rich_Text &inText)
{
  data->Set_Text(inText);
}

/******************************************************************************
 *  Set_Width
 */

/**** NDY: should Make_Unique ****/

void
Am_Text_Viewing_Context::Set_Width(const unsigned long inWidth)
{
  data->Set_Width(inWidth);
}

/******************************************************************************
 *  Draw
 */

void
Am_Text_Viewing_Context::Draw(Am_Drawonable *inDrawonable, long inLeft,
                              long inTop)
{
  data->Draw(inDrawonable, inLeft, inTop);
}

/******************************************************************************
 *  Am_Text_Viewing_Context_Data
 *****************************************************************************/

AM_WRAPPER_DATA_IMPL(Am_Text_Viewing_Context, (this))

/******************************************************************************
 *  Am_Text_Viewing_Context_Data
 */

Am_Text_Viewing_Context_Data::SLIBlock::SLIBlock()
{
  for (unsigned long line = 0; line < kLinesInBlock; line++) {
    li[line].fFirst = 0;
    li[line].fLength = 0;
    li[line].fHeight = 0;
  }
  fNext = (0L);
}

Am_Text_Viewing_Context_Data::Am_Text_Viewing_Context_Data()
{
  Init(Am_No_Rich_Text, 0); //Am_Rich_Text( preamble ), 0 );
}

Am_Text_Viewing_Context_Data::Am_Text_Viewing_Context_Data(
    Am_Text_Viewing_Context_Data *inProto)
{
  mWidth = inProto->mWidth;
  mText = inProto->mText;

  mNumLines = 0;
  mFirstLIBlock = new SLIBlock();
  mLIValid = false; // forces a call to Layout()
}

Am_Text_Viewing_Context_Data::Am_Text_Viewing_Context_Data(
    Am_Text_Viewing_Context_Data &inProto)
{
  mWidth = inProto.mWidth;
  mText = inProto.mText;

  mNumLines = 0;
  mFirstLIBlock = new SLIBlock();
  mLIValid = false; // forces a call to Layout()
}

Am_Text_Viewing_Context_Data::~Am_Text_Viewing_Context_Data()
{
  delete mFirstLIBlock;
}

/******************************************************************************
 *  Init
 */

void
Am_Text_Viewing_Context_Data::Init(Am_Rich_Text inText, unsigned long inWidth)
{
  mLIValid = false;
  mText = inText;
  mCursorRef = -1; // (inText == Am_No_Rich_Text)
  //  ? Am_No_Cursor : Am_No_Cursor; // inText->New_Cursor( this );
  mWidth = inWidth;
  mNumLines = 0;
  mFirstLIBlock = new SLIBlock;
}

/******************************************************************************
 *  Layout
 *    Computes the indexes of the first character of each line.
 */

void Am_Text_Viewing_Context_Data::Layout(
    Am_Drawonable *inDrawonable,
    Am_Text_Index /* inFirstInvChar */, // ignored for now - uses 0
    Am_Text_Index /* inLastInvChar */)  // ignored for now - uses end of text
{
#if DEBUG_HELP
  std::cout << "Am_Text_Viewing_Context_Data::Layout" << std::endl;
#endif

  bool endOfText = false;
  unsigned long charsOnLine = 0;
  unsigned short ascent = 0, descent = 0;
  Am_Text_Index textIndex = 1;

  if (mText == Am_No_Rich_Text) // || mWidth <= 0 )
    return;

  Am_Rich_Text_Data *textData = Am_Rich_Text_Data::Narrow(mText);

  SLIBlock *liBlock = mFirstLIBlock;

  unsigned long line = 0;
  while (true) {
    endOfText = textData->Calc_Line_Dim(inDrawonable, textIndex, mWidth,
                                        charsOnLine, ascent, descent);
#if DEBUG_HELP
    std::cout << "  Calculated line: " << line << std::endl;
#endif

    liBlock->li[line % kLinesInBlock].fFirst = textIndex;
    liBlock->li[line % kLinesInBlock].fLength = charsOnLine;
    liBlock->li[line % kLinesInBlock].fHeight =
        (ascent << 16) + (descent + ascent);
    textIndex += charsOnLine;

    line++;
    if (endOfText)
      break;

    if (line % kLinesInBlock == 0) {
      if (liBlock->fNext == (0L))
        liBlock->fNext = new SLIBlock;
      liBlock = liBlock->fNext;
    }
  }

  mNumLines = line;
  textData->Release();
  mLIValid = true;

#if DEBUG_HELP
  std::cout << "mNumLines = " << mNumLines << std::endl;
#endif
}

/******************************************************************************
 *  Draw
 */

void
Am_Text_Viewing_Context_Data::Draw(Am_Drawonable *inDrawonable, long inLeft,
                                   long inTop)
{
  if (!mLIValid) {
#if DEBUG_HELP
    std::cout << "Am_Text_Viewing_Context_Data::Draw calling Layout"
              << std::endl;
#endif
    Layout(inDrawonable);
  }

  // if Layout could not validate the object (mWidth <= 0) do not draw
  if (!mLIValid) {
#if DEBUG_HELP
    std::cout << "Am_Text_Viewing_Context_Data::Draw: call to Layout did not "
                 "validate the line info."
              << std::endl;
#endif
    return;
  }

#if DEBUG_HELP
  std::cout << "Am_Text_Viewing_Context_Data::Draw drawing the viewing context"
            << std::endl;
#endif

  long left = inLeft, top = inTop;

  SLIBlock *liBlock = mFirstLIBlock;

  // assume liBlock is valid
  unsigned long line = 0;
  while (true) {
    //   std::cout << "Drawing line: " << line << "  Top: " << top <<std::endl;
    mText.Draw_Line(
        inDrawonable, left, top, liBlock->li[line % kLinesInBlock].fFirst,
        liBlock->li[line % kLinesInBlock].fLength,
        (unsigned short)(liBlock->li[line % kLinesInBlock].fHeight >> 16));

    top += (liBlock->li[line % kLinesInBlock].fHeight & 0x0000FFFF);

    line++;
    if (line == mNumLines)
      break;

    if (line % kLinesInBlock == 0)
      liBlock = liBlock->fNext;
  }
}

/******************************************************************************
 *  Set_Text
 */

inline void
Am_Text_Viewing_Context_Data::Set_Text(const Am_Rich_Text &inText)
{
  mText = inText; // this calls Release on the old text object
  mLIValid = false;
}

/******************************************************************************
 *  Get_Width
 */

//inline unsigned long
//Am_Text_Viewing_Context_Data::Get_Width() const
//{
//  return mWidth;
//}

/******************************************************************************
 *  Set_Width
 */

inline void
Am_Text_Viewing_Context_Data::Set_Width(unsigned long inWidth)
{
  mWidth = inWidth;
  mLIValid = false;
}

/******************************************************************************
 *  Invalidate_Text_Range
 */

/**** NDY: might not need to invalidate the entire range ****/

void Am_Text_Viewing_Context_Data::Invalidate_Text_Range(
    Am_Text_Index /* inStart */, Am_Text_Length /* inLength */)
{
  mLIValid = false;
}

/******************************************************************************
 *  operator==
 */

bool
Am_Text_Viewing_Context_Data::
operator==(const Am_Text_Viewing_Context_Data & /* inOtherVC */)
{
  return false;
}

bool
Am_Text_Viewing_Context_Data::
operator==(const Am_Text_Viewing_Context_Data & /* inOtherVC */) const
{
  return false;
}

/******************************************************************************
 *  Am_Text_Fragment
 *****************************************************************************/

/******************************************************************************
 *  Am_Text_Fragment
 */

Am_Text_Fragment::Am_Text_Fragment(Am_Rich_Text_Data *inTextObject,
                                   const char *inString)
{
  mType = Am_STRING;
  mTextObject = inTextObject;
  mStartIndex = 0;
  mFirstMark = (0L);
  mPrev = mNext = (0L);

  Am_Text_Length stringLen = strlen(inString);

  if (stringLen > kFragStrSize) {
    mStrLen = kFragStrSize;
    memmove(mString, inString, (int)mStrLen);
    mNext = new Am_Text_Fragment(inTextObject, mString + kFragStrSize);
  } else {
    mStrLen = stringLen;
    memmove(mString, inString, (int)mStrLen);
  }
}

Am_Text_Fragment::Am_Text_Fragment(Am_Rich_Text_Data *inTextObject,
                                   const char *inString,
                                   Am_Text_Length inStrLen)
{
  mType = Am_STRING;
  mTextObject = inTextObject;
  mStrLen = inStrLen;
  mStartIndex = 0;
  mFirstMark = (0L);
  mPrev = mNext = (0L);

  if (inStrLen > kFragStrSize) {
    mStrLen = kFragStrSize;
    memmove(mString, inString, (int)mStrLen);
    mNext = new Am_Text_Fragment(inTextObject, mString + kFragStrSize,
                                 inStrLen - kFragStrSize);
  } else {
    mStrLen = inStrLen;
    memmove(mString, inString, (int)mStrLen);
  }
}

/******************************************************************************
 *  Text_Object
 */

Am_Rich_Text_Data *
Am_Text_Fragment::Text_Object() const
{
  return mTextObject;
}

/******************************************************************************
 *  Get_Start_Index
 */

//inline Am_Text_Index
//Am_Text_Fragment::Get_Start_Index() const
//{
//  return mStartIndex;
//}

/******************************************************************************
 *  Change_Start_Index
 */

//inline Am_Text_Index
//Am_Text_Fragment::Change_Start_Index(
//  Am_Text_Index inDelta )
//{
//  return mStartIndex += inDelta;
//}

/******************************************************************************
 *  Length
 */

inline Am_Text_Length
Am_Text_Fragment::Length() const
{
  return mStrLen;
}

/******************************************************************************
 *  String
 */

inline char *
Am_Text_Fragment::String(Am_Text_Offset inOffset) const
{
  if ((unsigned)inOffset < mStrLen) // signed vs unsigned comparison
                                    // make sure that inOffset > 0
    return (char *)mString + inOffset;
  else
    return (0L); // probably should raise some error, throw an exception...
}

/******************************************************************************
 *  Add_Mark
 */

void
Am_Text_Fragment::Add_Mark(Am_Text_Mark * /* inMarkToAdd */)
{
  return;

  /*
  Am_Text_Index searchIndex = inMarkToAdd->Get_Index();

  // check for legal range
  // if( searchIndex > mStrLen )
  //   error

  if( mFirstMark == (0L) )
  {
    // list empty; insert at beginning
    mFirstMark = inMarkToAdd;
    return;
  }

  if( mFirstMark->Get_Index() > searchIndex )
  {
    // insert at beginning of list
    inMarkToAdd->Set_Next( mFirstMark );
    mFirstMark = inMarkToAdd;
    return;
  }
*/

  // by now we are inserting after the first item
  // NDY: finish the insert
  //   Am_Rich_Text_Index* mark = mHead;
  //   while( mark != (0L) )
  //   {
  //     if( searchIndex > mark->Get_Index()
  //
  //   }
}

/******************************************************************************
 *  Break_At
 */

void
Am_Text_Fragment::Break_At(Am_Text_Index inRelIndex) // first char in next frag
{
  if (inRelIndex > mStrLen)
    return;

  Am_Text_Fragment *new_frag = new Am_Text_Fragment(
      mTextObject, (char *)mString + inRelIndex - 1, mStrLen - inRelIndex + 1);
  mStrLen = inRelIndex - 1;

  new_frag->mNext = this->mNext;
  if (this->mNext != (0L))
    this->mNext->mPrev = new_frag;

  this->mNext = new_frag;
  new_frag->mPrev = this;
}

/******************************************************************************
 *  Next_Frag & Prev_Frag
 */

inline Am_Text_Fragment *
Am_Text_Fragment::Next()
{
  return mNext;
}

inline Am_Text_Fragment *
Am_Text_Fragment::Prev()
{
  return mPrev;
}

/******************************************************************************
 *  Am_Text_Style_Run
 *****************************************************************************/

/******************************************************************************
 *  Am_Text_Style_Run
 */

Am_Text_Style_Run::Am_Text_Style_Run(unsigned long inCharsInRun, Am_Font inFont,
                                     Am_Style inTextStyle, Am_Style inBGStyle)
{
  mCharsInRun = inCharsInRun;
  mFont = inFont;
  mTextStyle = inTextStyle;
  mBGStyle = inBGStyle;
  mNext = (0L);
}

/******************************************************************************
 *  Length
 */

inline unsigned long
Am_Text_Style_Run::Length() const
{
  return mCharsInRun;
}

/******************************************************************************
 *  Change_Length
 */

inline unsigned long
Am_Text_Style_Run::Change_Length(Am_Text_Offset inDelta)
{
  return mCharsInRun += inDelta;
}

/******************************************************************************
 *  Next
 */

inline Am_Text_Style_Run *
Am_Text_Style_Run::Next() const
{
  return mNext;
}

/******************************************************************************
 *  Set_Next
 */

inline void
Am_Text_Style_Run::Set_Next(Am_Text_Style_Run *inNext)
{
  mNext = inNext;
}

/******************************************************************************
 *  Get_Font
 */

inline Am_Font
Am_Text_Style_Run::Get_Font() const
{
  return mFont;
}

/******************************************************************************
 *  Get_TextStyle
 */

inline Am_Style
Am_Text_Style_Run::Get_TextStyle() const
{
  return mTextStyle;
}

/******************************************************************************
 *  Get_BGStyle
 */

inline Am_Style
Am_Text_Style_Run::Get_BGStyle() const
{
  return mBGStyle;
}
