/* ****************************** -*-c++-*- *******************************/
/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <string.h>
#include <stdlib.h>

#include <am_inc.h>
#include AM_IO__H
#include <amulet/impl/types_logging.h>

#include <amulet/impl/types_string.h>
#include <amulet/impl/slots_opal.h>
#include <amulet/impl/am_drawonable.h>

#include <amulet/impl/am_object.h>
#include <amulet/impl/opal_op.h>
#include TEXT_FNS__H
#include <amulet/impl/inter_text.h>
#include GDEFS__H

//  Code recognizing routines
enum Am_Code_Flag
{
  Am_ONE_BYTE_CODE, // one-byte code flag for ASCII and so on
  Am_TWO_BYTE_CODE, // two-byte code flag for JIS, EUC and so on
  Am_SHIFT_JIS_CODE // multi-byte code flag for Shift-JIS
};

Am_Code_Flag
ml_which_code(Am_Object text)
{
  Am_String font_name;
  Am_Font_Family_Flag family;
  bool is_bold, is_italic, is_underline;
  Am_Font_Size_Flag size;

  Am_Font font = text.Get(Am_FONT);

  font.Get(font_name, family, is_bold, is_italic, is_underline, size);
  if ((int)family < (int)Am_FONT_JFIXED)
    return (Am_ONE_BYTE_CODE);
  else { // We use the Shift-JIS code on PC, and two-byte codes on X
#ifdef _WIN32
    return (Am_SHIFT_JIS_CODE);
#else
    return (Am_TWO_BYTE_CODE);
#endif
  }
}

int
ml_head_pos_str(const char *p)
{
  // returns the head position without escape sequence
  int i = 0;

  if (*p ==
      0x1B) { // Two-byte code such as 7bit-JIS code beginning with escape sequence
    p++;
    i++;
    if (*p++ != '$')
      return (0); // Illegal escape sequence
    i++;
    switch (*p) {
    case '(':
    case ')':
    case '*':
    case '+':
    case '-':
    case '.':
    case '/':
    case ',':
      i++;
      break;
    }
    i++; // System do not check the character set.
  }
  return (i);
}

int
ml_head_pos(Am_Object text)
{
  // returns the head position with out escape sequence
  if (ml_which_code(text) == Am_TWO_BYTE_CODE)
    return (ml_head_pos_str(Am_String(text.Get(Am_TEXT))));
  else
    return (0);
}

int
ml_strlen(const char *p)
{
  if (ml_head_pos_str(p) > 0) {
    int i;
    for (i = 1, p++; *p != 0x1B && *p != '\0'; p++, i++)
      ;
    return (i);
  } else
    return (strlen(p));
}

#define FBSJIS(x) (0x81 <= (x) && (x) <= 0x9F || 0xE0 <= (x) && (x) <= 0xFC)
bool
ml_two_byte_SHIFT_JIS(char *p, char *head)
{
  int x;

  while (head < p) {
    x = (*head) & 0xFF;
    if (FBSJIS(x))
      head += 2;
    else
      head++;
  }
  if (head == p) {
    x = (*head) & 0xFF;
    return (FBSJIS(x));
  }
  return (false);
}

void
Am_Set_Pending_Delete(Am_Object text, bool pending_delete)
{
  text.Set(Am_PENDING_DELETE, pending_delete);
  text.Set(Am_INVERT, pending_delete);
  if (pending_delete) //cursor always at beginning when pending-delete
    text.Set(Am_CURSOR_INDEX, 0);
}

void
Am_Move_Cursor_To_Start(Am_Object text)
{
  text.Set(Am_CURSOR_INDEX, ml_head_pos(text));
  Am_Set_Pending_Delete(text, false);
}

void
Am_Move_Cursor_To_End(Am_Object text)
{
  int str_len = ml_strlen(Am_String(text.Get(Am_TEXT)));
  text.Set(Am_CURSOR_INDEX, str_len);
  Am_Set_Pending_Delete(text, false);
}

void
Am_Move_Cursor_Right(Am_Object text, int n)
{
  int str_len = ml_strlen(Am_String(text.Get(Am_TEXT)));
  int new_index;
  Am_Value v;
  v = text.Peek(Am_PENDING_DELETE);
  if (v.Valid()) { //if pending delete, go to end of string
    new_index = str_len;
    Am_Set_Pending_Delete(text, false);
  } else {
    int cursor_index = text.Get(Am_CURSOR_INDEX);
    new_index = cursor_index + n;
  }
  if (new_index <= str_len)
    text.Set(Am_CURSOR_INDEX, new_index);
}

void
Am_Move_Cursor_Right(Am_Object text)
{
  switch (ml_which_code(text)) {
  case Am_ONE_BYTE_CODE:
    Am_Move_Cursor_Right(text, 1);
    break;
  case Am_TWO_BYTE_CODE:
    Am_Move_Cursor_Right(text, 2);
    break;
  case Am_SHIFT_JIS_CODE: {
    int pos = text.Get(Am_CURSOR_INDEX);
    Am_String p = text.Get(Am_TEXT);

    if (ml_two_byte_SHIFT_JIS(&(((char *)p)[pos]), (char *)p))
      Am_Move_Cursor_Right(text, 2);
    else
      Am_Move_Cursor_Right(text, 1);
    break;
  }
  }
}

void
Am_Move_Cursor_Word_Right(Am_Object text)
{
  if (ml_which_code(text) == Am_TWO_BYTE_CODE)
    ;
  // Space does not exist in two-byte code string.
  else { // Am_ONE_BYTE_CODE or Am_SHIFT_JIS_CODE
    int cursor_index = text.Get(Am_CURSOR_INDEX);
    int orig_cursor_index = cursor_index;
    Am_String old_str = text.Get(Am_TEXT);
    int str_len = strlen((const char *)old_str);
    if (0 <= cursor_index && cursor_index < str_len) {
      int start_wrd = cursor_index;
      // if there are spaces after the cursor, advance past them
      while (cursor_index < str_len &&
             ((const char *)old_str)[cursor_index] == ' ') {
        cursor_index++;
      }
      // advance up to but not past the next space
      while (cursor_index < str_len &&
             ((const char *)old_str)[cursor_index] != ' ') {
        cursor_index++;
      }
      Am_Move_Cursor_Right(text, cursor_index - orig_cursor_index);
    }
  }
}

void
Am_Move_Cursor_Left(Am_Object text, int n)
{
  int cursor_index = text.Get(Am_CURSOR_INDEX);
  int new_index = cursor_index - n;
  if (new_index >= ml_head_pos(text))
    text.Set(Am_CURSOR_INDEX, new_index);
  Am_Set_Pending_Delete(text, false);
}

void
Am_Move_Cursor_Left(Am_Object text)
{
  switch (ml_which_code(text)) {
  case Am_ONE_BYTE_CODE:
    Am_Move_Cursor_Left(text, 1);
    break;
  case Am_TWO_BYTE_CODE:
    Am_Move_Cursor_Left(text, 2);
    break;
  case Am_SHIFT_JIS_CODE: {
    int pos = (int)text.Get(Am_CURSOR_INDEX) - 2;
    Am_String p = text.Get(Am_TEXT);

    if (0 <= pos && ml_two_byte_SHIFT_JIS(&(((char *)p)[pos]), (char *)p))
      Am_Move_Cursor_Left(text, 2);
    else
      Am_Move_Cursor_Left(text, 1);
    break;
  }
  }
}

void
Am_Move_Cursor_Word_Left(Am_Object text)
{
  if (ml_which_code(text) == Am_TWO_BYTE_CODE)
    ;
  // Space does not exist in two-byte code string.
  else { // Am_ONE_BYTE_CODE or Am_SHIFT_JIS_CODE
    int cursor_index = text.Get(Am_CURSOR_INDEX);
    Am_String old_str = text.Get(Am_TEXT);
    int str_len = strlen(old_str);
    if (cursor_index <= str_len) {
      int start_wrd = cursor_index;
      // if there are spaces before the cursor, move past them
      while (start_wrd > 0 && ((const char *)old_str)[start_wrd - 1] == ' ') {
        start_wrd--;
      }
      // move back to but not past the next space
      while (start_wrd > 0 && ((const char *)old_str)[start_wrd - 1] != ' ') {
        start_wrd--;
      }
      Am_Move_Cursor_Left(text, cursor_index - start_wrd);
    }
  }
}

void
Am_Move_Cursor_To(Am_Object text, int n)
{
  Am_String p = text.Get(Am_TEXT);
  int str_len = ml_strlen((char *)p);
  int head_pos = ml_head_pos_str((char *)p);

  if (n >= head_pos && n <= str_len)
    text.Set(Am_CURSOR_INDEX, n);
  Am_Set_Pending_Delete(text, false);
}

//void Am_Set_Text (Am_Object text, char *str) {
//  text.Set(Am_TEXT, str);
//}

// Roma-ji kana conversion routine for Japanese input
#ifndef _WIN32

#include "amulet/romkantab.h"

bool
isJapanese(Am_Object text)
{
  Am_String font_name;
  Am_Font_Family_Flag family;
  bool is_bold, is_italic, is_underline;
  Am_Font_Size_Flag size;
  Am_Font font = text.Get(Am_FONT);

  font.Get(font_name, family, is_bold, is_italic, is_underline, size);
  return (family == Am_FONT_JFIXED || family == Am_FONT_JPROPORTIONAL);
}

#define iseuc(p) (((*(p)) & 0x80) != 0)

#define isboin(c)                                                              \
  ((c) == 'a' || (c) == 'e' || (c) == 'i' || (c) == 'o' || (c) == 'u')
#define isshiin(c) ('b' <= (c) && (c) <= 'z' && !isboin(c))

char *
to_correct_code(const char *p, bool eucf)
{
  static char buf[16]; // Probably at most 4 bytes are used.
  char *q = buf;

  while (*p != '\0') {
    *q++ = eucf ? (*p | 0x80) : (*p & 0x7F);
    p++;
  }
  *q = '\0';
  return (buf);
}

int
jcomp(const char *p, const char *q)
{
  return (((p[0] & 0x7F) << 8) + (p[1] & 0x7F) - ((q[0] & 0x7F) << 8) -
          (q[1] & 0x7F));
}

int
isalpha_2(const char *p)
{
  int i = jcomp(p, onetotwo['a']);
  if (0 <= i && i <= 25)
    return (i);
  return (-1);
}

void
Am_Insert_JChar_At_Cursor(Am_Object text, char c, bool advance_cursor)
{
  int cursor_index = text.Get(Am_CURSOR_INDEX);
  int head_pos = ml_head_pos(text);
  if (cursor_index >= head_pos) {
    if ((bool)text.Get(Am_PENDING_DELETE)) //delete all text first
      Am_Delete_Entire_String(text);
    Am_String old_str = text.Get(Am_TEXT);
    bool eucf = iseuc((char *)old_str);
    if (cursor_index >= head_pos + 2) {
      cursor_index -= 2;
      if (c == 'n' &&
          jcomp(&((char *)old_str)[cursor_index], onetotwo['n']) ==
              0) { // case 'nn'
        Am_Delete_Char_Before_Cursor(text);
        Am_Insert_String_At_Cursor(text, to_correct_code(n_Japanese, eucf),
                                   advance_cursor);
      } else if (!isboin(c) && c != 'h' && c != 'y' &&
                 jcomp(&((char *)old_str)[cursor_index], onetotwo['n']) ==
                     0) { // case 'nt' etc.
        Am_Delete_Char_Before_Cursor(text);
        Am_Insert_String_At_Cursor(text, to_correct_code(n_Japanese, eucf),
                                   true);
        Am_Insert_String_At_Cursor(text, to_correct_code(onetotwo[c], eucf),
                                   advance_cursor);
      } else if (isshiin(c) &&
                 jcomp(&((char *)old_str)[cursor_index], onetotwo[c]) ==
                     0) { // case 'tt' etc.
        Am_Move_Cursor_Left(text);
        Am_Insert_String_At_Cursor(text, to_correct_code(small_tsu, eucf),
                                   true);
        if (advance_cursor)
          Am_Move_Cursor_Right(text);
      } else { // not 'nn', 'tt' etc.
        int boin, shiin;
        switch (c) {
        case 'a':
          boin = 0;
          break;
        case 'i':
          boin = 1;
          break;
        case 'u':
          boin = 2;
          break;
        case 'e':
          boin = 3;
          break;
        case 'o':
          boin = 4;
          break;
        default: // case only Shiin
          Am_Insert_String_At_Cursor(text, to_correct_code(onetotwo[c], eucf),
                                     advance_cursor);
          return;
        }
        if (jcomp(&((char *)old_str)[cursor_index], onetotwo['y']) == 0) {
          if (cursor_index >= head_pos + 2 &&
              (shiin = isalpha_2(&((char *)old_str)[cursor_index - 2])) >
                  0) { //case 'kya' etc.
            Am_Delete_Char_Before_Cursor(text);
            Am_Delete_Char_Before_Cursor(text);
            Am_Insert_String_At_Cursor(
                text, to_correct_code(rktab[shiin][boin + 5], eucf),
                advance_cursor);
          } else { // two alphabets in 'y' case
            Am_Delete_Char_Before_Cursor(text);
            Am_Insert_String_At_Cursor(
                text, to_correct_code(rktab['y' - 'a'][boin], eucf),
                advance_cursor);
          }
        } else if (jcomp(&((char *)old_str)[cursor_index], onetotwo['h']) ==
                   0) { // case 'kha' etc.
          if (cursor_index >= head_pos + 2 &&
              (shiin = isalpha_2(&((char *)old_str)[cursor_index - 2])) > 0) {
            Am_Delete_Char_Before_Cursor(text);
            Am_Delete_Char_Before_Cursor(text);
            Am_Insert_String_At_Cursor(
                text, to_correct_code(rktab[shiin][boin + 5], eucf),
                advance_cursor);
          } else { // two alphabets in 'h' case
            Am_Delete_Char_Before_Cursor(text);
            Am_Insert_String_At_Cursor(
                text, to_correct_code(rktab['h' - 'a'][boin], eucf),
                advance_cursor);
          }
        } else { // without 'h', 'y'
          if ((shiin = isalpha_2(&((char *)old_str)[cursor_index])) >
              0) { // case 'ka' etc.
            Am_Delete_Char_Before_Cursor(text);
            Am_Insert_String_At_Cursor(
                text, to_correct_code(rktab[shiin][boin], eucf),
                advance_cursor);
          } else { // case Boin only, such as 'a' etc.
            Am_Insert_String_At_Cursor(
                text, to_correct_code(rktab[0][boin], eucf), advance_cursor);
          }
        }
      }
    } else { // insert to the head of input field
      int boin;
      switch (c) {
      case 'a':
        boin = 0;
        break;
      case 'i':
        boin = 1;
        break;
      case 'u':
        boin = 2;
        break;
      case 'e':
        boin = 3;
        break;
      case 'o':
        boin = 4;
        break;
      default: // case only Shiin
        Am_Insert_String_At_Cursor(text, to_correct_code(onetotwo[c], eucf),
                                   advance_cursor);
        return;
      }
      Am_Insert_String_At_Cursor(text, to_correct_code(rktab[0][boin], eucf),
                                 advance_cursor);
    }
  }
}

#endif

// Defaults:
//   advance_cursor = true
void
Am_Insert_Char_At_Cursor(Am_Object text, char c, bool advance_cursor)
{

#ifndef _WIN32
  if (isJapanese(text)) {
    Am_Insert_JChar_At_Cursor(text, c, advance_cursor);
    return;
  }
#endif

  int cursor_index = text.Get(Am_CURSOR_INDEX);
  if (cursor_index >= 0) {
    if ((bool)text.Get(Am_PENDING_DELETE)) //delete all text first
      Am_Delete_Entire_String(text);
    Am_String old_str = text.Get(Am_TEXT);
    int str_len = strlen(old_str);
    // +2 for the new character and the terminating \0
    char *new_str = new char[str_len + 2];
    memcpy(new_str, (const char *)old_str, sizeof(char) * cursor_index);
    new_str[cursor_index] = c;
    memcpy((void *)&new_str[cursor_index + 1],
           &((const char *)old_str)[cursor_index],
           // Copy the terminating \0 also
           (str_len - cursor_index + 1) * sizeof(char));
    text.Set(Am_TEXT, Am_String(new_str, false));
    if (advance_cursor == true)
      Am_Move_Cursor_Right(text, 1);
  }
}

// Defaults:
//   advance_cursor = true
void
Am_Insert_String_At_Cursor(Am_Object text, const char *add_str,
                           bool advance_cursor)
{
  if (!add_str)
    return; // don't crash if (0L) string.
  int cursor_index = text.Get(Am_CURSOR_INDEX);
  if (cursor_index >= 0) {
    if ((bool)text.Get(Am_PENDING_DELETE)) //delete all text first
      Am_Delete_Entire_String(text);
    Am_String old_str;
    old_str = text.Get(Am_TEXT);
    int old_str_len = strlen(old_str);
    int add_str_len = ml_strlen(add_str);
    int add_head_pos = ml_head_pos_str(add_str);
    // +1 for the terminating \0
    int new_str_len = old_str_len + add_str_len - add_head_pos + 1;
    char *new_str = new char[new_str_len];
    memcpy(new_str, (const char *)old_str, sizeof(char) * cursor_index);
    memcpy((void *)&new_str[cursor_index],
           &((const char *)add_str)[add_head_pos],
           sizeof(char) * (add_str_len - add_head_pos));
    memcpy((void *)&new_str[cursor_index + add_str_len - add_head_pos],
           &((const char *)old_str)[cursor_index],
           // Copy the terminating \0 also
           (old_str_len - cursor_index + 1) * sizeof(char));
    text.Set(Am_TEXT, Am_String(new_str, false));
    if (advance_cursor == true)
      Am_Move_Cursor_Right(text, add_str_len - add_head_pos);
  }
}

//// Am_Delete_Char_At_Index was used by Am_Delete_Char_{Before,After}_Cursor
//// now it isn't.  Definition removed from text_fns.h
//
//void Am_Delete_Char_At_Index (char *str, int index) {
//  int str_len = strlen (str);
//  memmove((void *)&str[index], &str[index+1],
//	  // Move the terminating \0 also
//	  (str_len - index) * sizeof(char));
//}

// Am_Delete_Substring deletes chars [from,to] of old_str, and returns a newly
// allocated string.
// Assumes old_str is /0 terminated properly, and does not check to and from
// for out of bounds conditions

char *
Am_Delete_Substring(char *old_str, int from, int to)
{
  int str_len = strlen(old_str);
  // +1 for the terminating /0
  char *new_str = new char[str_len - to + from + 1];
  memcpy((void *)new_str, old_str, from * sizeof(char));
  memcpy((void *)&new_str[from], &old_str[to],
         // +1 for the terminating /0
         (str_len - to + 1) * sizeof(char));
  return new_str;
}

void
Am_Delete_Char_Before_Cursor(Am_Object text)
{
  if ((bool)text.Get(Am_PENDING_DELETE)) //delete all text first
    Am_Delete_Entire_String(text);
  else {
    int cursor_index = text.Get(Am_CURSOR_INDEX);
    Am_String old_str = text.Get(Am_TEXT);
    char *new_str;

    switch (ml_which_code(text)) {
    case Am_ONE_BYTE_CODE:
      if (cursor_index > 0) {
        new_str = Am_Delete_Substring(old_str, cursor_index - 1, cursor_index);
        text.Set(Am_TEXT, Am_String(new_str, false));
        Am_Move_Cursor_Left(text);
      }
      break;
    case Am_TWO_BYTE_CODE:
      if (cursor_index > ml_head_pos_str(old_str) + 1) {
        new_str = Am_Delete_Substring(old_str, cursor_index - 2, cursor_index);
        text.Set(Am_TEXT, Am_String(new_str, false));
        Am_Move_Cursor_Left(text);
      }
      break;
    case Am_SHIFT_JIS_CODE:
      if (cursor_index >= 2 &&
          ml_two_byte_SHIFT_JIS(&(((char *)old_str)[cursor_index - 2]),
                                (char *)old_str)) {
        new_str = Am_Delete_Substring(old_str, cursor_index - 2, cursor_index);
        Am_Move_Cursor_Left(text);
        text.Set(Am_TEXT, Am_String(new_str, false));
      } else if (cursor_index > 0) {
        new_str = Am_Delete_Substring(old_str, cursor_index - 1, cursor_index);
        Am_Move_Cursor_Left(text);
        text.Set(Am_TEXT, Am_String(new_str, false));
      }
      break;
    }
  }
}

void
Am_Delete_Char_After_Cursor(Am_Object text)
{
  if ((bool)text.Get(Am_PENDING_DELETE)) //delete all text first
    Am_Delete_Entire_String(text);
  else {
    int cursor_index = text.Get(Am_CURSOR_INDEX);
    Am_String old_str = text.Get(Am_TEXT);
    int str_len = ml_strlen((char *)old_str);
    char *new_str;

    switch (ml_which_code(text)) {
    case Am_ONE_BYTE_CODE:
      if (0 <= cursor_index && cursor_index < str_len) {
        new_str = Am_Delete_Substring(old_str, cursor_index, cursor_index + 1);
        text.Set(Am_TEXT, Am_String(new_str, false));
      }
      break;
    case Am_TWO_BYTE_CODE:
      if (0 <= cursor_index && cursor_index < str_len - 1) {
        new_str = Am_Delete_Substring(old_str, cursor_index, cursor_index + 2);
        text.Set(Am_TEXT, Am_String(new_str, false));
      }
      break;
    case Am_SHIFT_JIS_CODE:
      if (0 <= cursor_index && cursor_index < str_len - 1 &&
          ml_two_byte_SHIFT_JIS(&(((char *)old_str)[cursor_index]),
                                (char *)old_str)) {
        new_str = Am_Delete_Substring(old_str, cursor_index, cursor_index + 2);
        text.Set(Am_TEXT, Am_String(new_str, false));
      } else if (0 <= cursor_index && cursor_index < str_len) {
        new_str = Am_Delete_Substring(old_str, cursor_index, cursor_index + 1);
        text.Set(Am_TEXT, Am_String(new_str, false));
      }
      break;
    }
  }
}

void
Am_Delete_Word_Before_Cursor(Am_Object text)
{
  if ((bool)text.Get(Am_PENDING_DELETE)) //delete all text first
    Am_Delete_Entire_String(text);
  else if (ml_which_code(text) == Am_TWO_BYTE_CODE)
    ;
  // Space does not exist in two-byte code string.
  else { // Am_ONE_BYTE_CODE or Am_SHIFT_JIS_CODE
    int cursor_index = text.Get(Am_CURSOR_INDEX);
    Am_String old_str = text.Get(Am_TEXT);
    int str_len = strlen(old_str);
    if (cursor_index <= str_len) {
      char *new_str;
      int start_wrd = cursor_index;
      // if there are spaces before the cursor, delete them with the word
      while (start_wrd > 0 && ((const char *)old_str)[start_wrd - 1] == ' ') {
        start_wrd--;
      }
      // delete back to but not including the next space
      while (start_wrd > 0 && ((const char *)old_str)[start_wrd - 1] != ' ') {
        start_wrd--;
      }
      // create a new string, since it changed in size.
      // +1 for the terminating /0
      new_str = Am_Delete_Substring(old_str, start_wrd, cursor_index);
      text.Set(Am_TEXT, Am_String(new_str, false));
      Am_Move_Cursor_Left(text, cursor_index - start_wrd);
    }
  }
}

void
Am_Delete_Word_After_Cursor(Am_Object text)
{
  if ((bool)text.Get(Am_PENDING_DELETE)) //delete all text first
    Am_Delete_Entire_String(text);
  else {
    if (ml_which_code(text) == Am_TWO_BYTE_CODE)
      ;
    // Space does not exist in two-byte code string.
    else { // Am_ONE_BYTE_CODE or Am_SHIFT_JIS_CODE
      int cursor_index = text.Get(Am_CURSOR_INDEX);
      Am_String old_str = text.Get(Am_TEXT);
      int str_len = strlen((const char *)old_str);
      char *new_str;
      if (0 <= cursor_index && cursor_index < str_len) {
        int start_wrd = cursor_index;
        // if there are spaces after the cursor, delete them with the word
        while (cursor_index < str_len &&
               ((const char *)old_str)[cursor_index] == ' ') {
          cursor_index++;
        }
        // delete up to but not including the next space
        while (cursor_index < str_len &&
               ((const char *)old_str)[cursor_index] != ' ') {
          cursor_index++;
        }
        // create a new string, since it changed in size.
        // +1 for the terminating /0
        new_str = Am_Delete_Substring(old_str, start_wrd, cursor_index);
        text.Set(Am_TEXT, Am_String(new_str, false));
      }
    }
  }
}

void
Am_Kill_From_Cursor(Am_Object text)
{
  if ((bool)text.Get(Am_PENDING_DELETE)) //delete all text first
    Am_Delete_Entire_String(text);
  else {
    int cursor_index = text.Get(Am_CURSOR_INDEX);
    Am_String old_str = text.Get(Am_TEXT);
    int str_len = strlen(old_str);
    if ((cursor_index >= 0) && (cursor_index <= str_len)) {
      char *new_str = Am_Delete_Substring(old_str, cursor_index, str_len);
      text.Set(Am_TEXT, Am_String(new_str, false));
      // cursor is now already at end of string
    }
  }
}

void
Am_Delete_Entire_String(Am_Object text)
{
  Am_String old_str = text.Get(Am_TEXT);
  text.Set(Am_TEXT, "");
  Am_Move_Cursor_To_Start(text); //will clear PENDING_DELETE
}

// modelled after garnet's get-index, without a special case for fixed-width
// fonts.

int
Am_Get_Cursor_Index(Am_Object text, int x, int y)
{
  Am_String str = text.Get(Am_TEXT);
  Am_Object window = text.Get(Am_WINDOW);
  Am_Object owner = text.Get(Am_OWNER);
  bool visible = text.Get(Am_VISIBLE);
  if (window && owner && visible) {
    Am_Drawonable *drawonable =
        Am_Drawonable::Narrow(window.Get(Am_DRAWONABLE));
    if (!drawonable)
      return 0;
    Am_Font font(text.Get(Am_FONT));
    int x_offset = text.Get(Am_X_OFFSET);

    int str_len = ml_strlen((char *)str);
    // Must use translate_coordinates in case text is in a group.
    int target;
    Am_Translate_Coordinates(window, x, y, text, target, y);
    target += x_offset; //take into account string might be scrolled

    int n, low, high, next;
    Am_Code_Flag codef = ml_which_code(text);
    for (n = ml_head_pos_str(str); n < str_len; n = next) {
      switch (codef) {
      case Am_TWO_BYTE_CODE:
        next = n + 2;
        break;
      case Am_SHIFT_JIS_CODE:
        if (ml_two_byte_SHIFT_JIS(&(((char *)str)[n]), (char *)str))
          next = n + 2;
        else
          next = n + 1;
        break;
      case Am_ONE_BYTE_CODE:
      default:
        next = n + 1;
        break;
      }
      low = drawonable->Get_String_Width(font, str, n);
      high = drawonable->Get_String_Width(font, str, next);
      if ((low <= target) && (target <= high))
        return ((target - low) > (high - target)) ? next : n;
    }
    // get here when pressed to the right of the string, so return end
    return str_len;
  } else
    return 0;
}

void
Am_Set_Cut_Buffer(Am_Object win, char *str)
{
  if (win) {
    Am_Drawonable *drawonable = Am_Drawonable::Narrow(win.Get(Am_DRAWONABLE));
    if (drawonable)
      drawonable->Set_Cut_Buffer(str);
  }
}

char *
Am_Get_Cut_Buffer(Am_Object win)
{
  if (win) {
    Am_Drawonable *drawonable = Am_Drawonable::Narrow(win.Get(Am_DRAWONABLE));
    return drawonable->Get_Cut_Buffer();
  } else
    return 0;
}

void
Am_Insert_Cut_Buffer_At_Cursor(Am_Object text)
{
  if ((bool)text.Get(Am_PENDING_DELETE)) //delete all text first
    Am_Delete_Entire_String(text);
  Am_Object win;
  if ((win = text.Get(Am_WINDOW)))
    Am_Insert_String_At_Cursor(text, Am_Get_Cut_Buffer(win));
}

void
Am_Set_Cut_Buffer_From_Text(Am_Object text)
{
  Am_Object win;
  if ((win = text.Get(Am_WINDOW)))
    Am_Set_Cut_Buffer(win, Am_String(text.Get(Am_TEXT)));
}

//////////////////
// Am_Edit_Translation_Table
//////////////////

////////////////////
// Class Am_Edit_Translation_Table
////////////////////

#include UNIV_MAP__H

// custom hash table functions

int
HashValue(Am_Input_Char key, int size)
{
  // Look only at the code, button_down, shift, control, meta,
  // click_count fields.  any_modifier is ignored.
  return ((((((unsigned long)key.code << 1 + (unsigned long)key.shift)
             << 1 + (unsigned long)key.control)
            << 1 + (unsigned long)key.meta)
           << 3 + (unsigned long)key.button_down)
          << 4 + (unsigned long)key.click_count) %
         size;
}

int
KeyComp(Am_Input_Char key1, Am_Input_Char key2)
{
  // For now, just return 0 if equal, !=0 if not equal.
  return key1 != key2;
}

AM_DECL_MAP(in_char2text_op, Am_Input_Char, Am_Text_Edit_Operation *)

AM_IMPL_MAP(in_char2text_op, Am_Input_Char, Am_Input_Char(-1),
            Am_Text_Edit_Operation *, 0)

class Am_Edit_Translation_Table_Data : public Am_Wrapper
{
  // declares Copy, Release, operator==, ID, Narrow, Am_Edit_Translation_ID, id
  AM_WRAPPER_DATA_DECL(Am_Edit_Translation_Table)
public:
  Am_Edit_Translation_Table_Data() { table = new Am_Map_in_char2text_op; }
  Am_Edit_Translation_Table_Data(Am_Edit_Translation_Table_Data *proto)
  {
    table = proto->table->Copy();
  }
  ~Am_Edit_Translation_Table_Data()
  {
    if (table)
      delete table;
  }
  void Add(Am_Input_Char ic, Am_Text_Edit_Operation *func);
  void Remove(Am_Input_Char ic);
  Am_Text_Edit_Operation *Translate(Am_Input_Char ic) const;
  bool operator==(Am_Edit_Translation_Table_Data &test_value) const
  {
    return this == &test_value;
  }

protected:
  Am_Map_in_char2text_op *table;
};

AM_WRAPPER_IMPL(Am_Edit_Translation_Table)

Am_Edit_Translation_Table::Am_Edit_Translation_Table()
{
  data = new Am_Edit_Translation_Table_Data;
}

Am_Edit_Translation_Table
Am_Edit_Translation_Table::Table()
{
  Am_Edit_Translation_Table table;
  return table;
}

Am_Edit_Translation_Table
Am_Edit_Translation_Table::Default_Table()
{
  Am_Edit_Translation_Table table;
  table.Add("RIGHT_ARROW", Am_Move_Cursor_Right);
  table.Add("LEFT_ARROW", Am_Move_Cursor_Left);
  table.Add("CONTROL_f", Am_Move_Cursor_Right);
  table.Add("CONTROL_b", Am_Move_Cursor_Left);
  table.Add("CONTROL_a", Am_Move_Cursor_To_Start);
  table.Add("HOME", Am_Move_Cursor_To_Start);
  table.Add("CONTROL_e", Am_Move_Cursor_To_End);
  table.Add("END", Am_Move_Cursor_To_End);
  table.Add("CONTROL_h", Am_Delete_Char_Before_Cursor);
  table.Add("BACKSPACE", Am_Delete_Char_Before_Cursor);
  table.Add("CONTROL_d", Am_Delete_Char_After_Cursor);
  table.Add("CONTROL_k", Am_Kill_From_Cursor);
  table.Add("CONTROL_u", Am_Delete_Entire_String);
  table.Add("CONTROL_w", Am_Delete_Word_Before_Cursor);
  table.Add("CONTROL_BACKSPACE", Am_Delete_Word_Before_Cursor);
  table.Add("CONTROL_c", Am_Set_Cut_Buffer_From_Text);
  table.Add("MIDDLE_DOWN", Am_Insert_Cut_Buffer_At_Cursor);
#ifdef WIN_KEYBINDINGS
  table.Add("DELETE", Am_Delete_Char_After_Cursor);
  //  table.Add ("CONTROL_DELETE",   Am_Delete_Word_After_Cursor);
  table.Add("CONTROL_v", Am_Insert_Cut_Buffer_At_Cursor);
  table.Add("CONTROL_y", Am_Delete_Entire_String);
#else
  table.Add("DELETE", Am_Delete_Char_Before_Cursor);
  table.Add("CONTROL_DELETE", Am_Delete_Word_After_Cursor);
  table.Add("CONTROL_y", Am_Insert_Cut_Buffer_At_Cursor);
#endif
  return table;
}

Am_Text_Edit_Operation *
Am_Edit_Translation_Table::Translate(Am_Input_Char ic) const
{
  return data->Translate(ic);
}

void
Am_Edit_Translation_Table::Add(Am_Input_Char ic, Am_Text_Edit_Operation *func)
{
  data = (Am_Edit_Translation_Table_Data *)data->Make_Unique();
  data->Add(ic, func);
}

void
Am_Edit_Translation_Table::Remove(Am_Input_Char ic)
{
  data = (Am_Edit_Translation_Table_Data *)data->Make_Unique();
  data->Remove(ic);
}

AM_WRAPPER_DATA_IMPL(Am_Edit_Translation_Table, (this))

void
Am_Edit_Translation_Table_Data::Add(Am_Input_Char ic,
                                    Am_Text_Edit_Operation *func)
{
  table->SetAt(ic, func);
}

void
Am_Edit_Translation_Table_Data::Remove(Am_Input_Char ic)
{
  table->DeleteKey(ic);
}

Am_Text_Edit_Operation *
Am_Edit_Translation_Table_Data::Translate(Am_Input_Char ic) const
{
  return table->GetAt(ic);
}

// Default edit translation table, text edit function
Am_Define_Method(Am_Text_Edit_Method, void, Am_Default_Text_Edit_Method,
                 (Am_Object text, Am_Input_Char ic, Am_Object inter))
{
  Am_Edit_Translation_Table table;
  table = inter.Get(Am_EDIT_TRANSLATION_TABLE);
  Am_Text_Edit_Operation *op = table.Translate(ic);
  if (op)
    op(text);
  else {
    unsigned char c = ic.As_Char();
    if ((' ' <= c && c <= '~') || // printing characters & JIS code
        (c >= 0x80 && c <= 0xFF)) // Mac fonts & EUC code have 80->FF,
      // X font have a1->FF, 1st byte of SJIS code have 81->9F & A0->FC
      // 2nd byte of SJIS code have 40->7E & 80->FC
      Am_Insert_Char_At_Cursor(text, c);
    else if (c) // if it's 0, it was some other event, so don't beep.
    // otherwise, it beeps too much.
    {
      Am_Object win;
      win = text.Get(Am_WINDOW);
      Am_Beep(win);
    }
  }
}
