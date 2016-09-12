#include "am_object.h"

#define Am_WINTODO_CREATE 0x00000001
#define Am_WINTODO_DESTROY 0x00000002
#define Am_WINTODO_EXPOSE 0x00000004
#define Am_WINTODO_REPARENT 0x00000008
#define Am_WINTODO_VISIBLE 0x00000010
#define Am_WINTODO_POSITION 0x00000020
#define Am_WINTODO_SIZE 0x00000040
#define Am_WINTODO_TITLE 0x00000080
#define Am_WINTODO_ICON_TITLE 0x00000100
#define Am_WINTODO_TITLE_BAR 0x00000200
#define Am_WINTODO_FILL_STYLE 0x00000400
#define Am_WINTODO_MIN_SIZE 0x00000800
#define Am_WINTODO_MAX_SIZE 0x00001000
#define Am_WINTODO_ICONIFY 0x00002000
#define Am_WINTODO_SAVE_UNDER 0x00004000
#define Am_WINTODO_CURSOR 0x00008000
#define Am_WINTODO_CLIP 0x00010000

class Am_Window_ToDo
{
public:
  Am_Object window;
  unsigned long flags;
  int left, top; // Invalid rectangle
  int width, height;
  Am_Window_ToDo *prev;
  Am_Window_ToDo *next;
  static Am_Window_ToDo *Narrow(Am_Ptr input)
  {
    return (Am_Window_ToDo *)input;
  }
  void Merge_Rectangle(int left, int top, int width, int height);
  void Add();
  void Remove();
};
