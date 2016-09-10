#include <am_inc.h>

#include GEM__H
#define OPTION1      1
bool selected = false;
bool done = false;

Am_Style* Red = Am_Style::Create (1.0, 0.0, 0.0);
Am_Style* Green = Am_Style::Create (0.0, 1.0, 0.0);
Am_Style* Stipple = Am_Style::Halftone_Stipple (50); 

void redraw (Am_Drawonable* drawon)
{
  int width, height;
  drawon->Get_Size (width, height);
  drawon->Clear_Area (0, 0, width, height);
  if (selected) {
    drawon->Draw_Rectangle ((0L), Red, width / 4, height / 4,
                            width / 2, height / 2);
    drawon->Draw_Rectangle ((0L), Stipple, 0, 0,
                            width / 4, height / 4);
  }
  else {
    drawon->Draw_Rectangle ((0L), Green, width / 4, height / 4,
                            width / 2, height / 2);
    drawon->Draw_Rectangle ((0L), Stipple, 0, 0,
                            width / 4, height / 4);
  }
  drawon->Flush_Output ();
}

class Handlers : public Am_Input_Event_Handlers {
public:
  void Iconify_Notify (Am_Drawonable*, bool) {}
  void Frame_Resize_Notify (Am_Drawonable*, int, int, int, int) {}
  void Destroy_Notify (Am_Drawonable*) {}
  void Configure_Notify (Am_Drawonable*, int, int, int, int) {}
  void Exposure_Notify (Am_Drawonable *draw, int, int, int, int)
  {
    redraw (draw);
  }
  void Input_Event_Notify (Am_Drawonable *draw, Am_Input_Event *ev)
  {
    fprintf(stderr,"testMS: Input_Event_Notify-[clickct=%d]---->",
	    ev->input_char.click_count);
    if (ev->input_char.click_count) {
      if ((ev->input_char.button_down == Am_BUTTON_DOWN) &&
          (ev->input_char.code == Am_LEFT_MOUSE)) {
        selected = !selected;
	fprintf(stderr,"BTN_DN-->");
      }
        redraw (draw);
    }
    else {
      char ch = (char)ev->input_char.code;
      if (ch == 'q')
        done = true;
    }
    fprintf(stderr,"selected = 0x%x\n",selected);
  }
} My_Handlers;

main (int argc, char** argv)
{
  Am_Drawonable* root = Am_Drawonable::Get_Root_Drawonable();
  Am_Drawonable* drawon = root->Create (100, 100, 200, 200);
  drawon->Set_Input_Dispatch_Functions (&My_Handlers);
  
  if (argc == 2) {
    Am_Drawonable* root2 =
        Am_Drawonable::Get_Root_Drawonable (argv[1]);
    Am_Drawonable* drawon2 = root2->Create (500, 100, 200, 200);
    drawon2->Set_Input_Dispatch_Functions (&My_Handlers);
  }

#ifdef OPTION1
  while (!done)
    Am_Drawonable::Process_Event ();
#else
  while (!done) {
    Am_Drawonable::Process_Immediate_Event ();
    printf ("\nPress RETURN: returned from Process_Immediate_Event");
    getchar ();
  }
#endif
}
