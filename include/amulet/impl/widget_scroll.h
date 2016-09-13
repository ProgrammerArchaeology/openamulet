//scroll bars

enum Am_Scroll_Arrow_Direction
{
  Am_SCROLL_ARROW_UP,
  Am_SCROLL_ARROW_DOWN,
  Am_SCROLL_ARROW_LEFT,
  Am_SCROLL_ARROW_RIGHT
};

//used to make the individual scroll-indicator drags not queued for undo
#define Am_MARKER_FOR_SCROLL_INC -2

extern void Am_Scroll_Widgets_Initialize();
