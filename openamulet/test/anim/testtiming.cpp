/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <iostream>
#include <amulet.h>
#include ANIM__H
#include OPAL_ADVANCED__H
#include OPAL__H

Am_Time start;

Am_Define_Method (Am_Object_Method, void, print_elapsed_time,
		  (Am_Object interp))
{
  Am_Object_Method(interp.Get_Prototype().Get(Am_DO_METHOD)).Call (interp);

  int n = interp.Get (Am_VALUE);
  Am_Time stop = Am_Time::Now();

  long ms = (stop - start).Milliseconds();

  std::cout 
    << "Animation constraint executed " << n << " steps in " << ms << std::endl
      << "for average cost of " << double(ms)/n << " ms per iteration." << std::endl << std::flush;

  Am_Exit_Main_Event_Loop ();
}  

long event_count;

Am_Define_Method (Am_Object_Method, void, print_event_count,
		  (Am_Object interp))
{
  Am_Object_Method(interp.Get_Prototype().Get(Am_DO_METHOD)).Call (interp);

  int n = event_count;
  Am_Time stop = Am_Time::Now();

  long ms = (stop - start).Milliseconds();

  std::cout 
    << "Animation constraint executed " << n << " steps in " << ms << std::endl
      << "for average cost of " << double(ms)/n << " ms per iteration." << std::endl << std::flush;

  Am_Exit_Main_Event_Loop ();
}  

#if 0
void main (int argc, char *argv[])
{
  int n = argc > 1 ? atoi(argv[1]) : 1000;
  bool use_linear = (argc > 2);

  Am_Initialize ();

  Am_Object stepper = Am_Stepping_Animator.Create ("stepper")
	      .Set (Am_REPEAT_DELAY, Am_Time())
	      .Set (Am_SMALL_INCREMENT, 1)
	      .Set (Am_DO_METHOD, print_elapsed_time);
  Am_Object linear = Am_Smooth_Animator.Create ("linear")
	      .Set (Am_MIN_REPEAT_DELAY, Am_Time())
	      .Set (Am_VELOCITY, Am_Velocity_From_Time)
	      .Set (Am_TIME_FOR_ANIMATION, Am_Time (10000))
	      .Set (Am_DO_METHOD, print_event_count)
	      ;
  
  std::cout << "Starting test animation..." << std::endl << std::flush;
  Am_Object obj = Am_Root_Object.Create ("test object")
    .Set (Am_VALUE, 0)
    .Animate (Am_VALUE, use_linear ? linear : stepper)
    .Set (Am_VALUE, n)
    ;

  start = Am_Time::Now();
  Am_Main_Event_Loop ();

  std::cout << "Done." << std::endl << std::flush;
  Am_Cleanup ();
}
#endif

int count = 0;

Am_Object goober;   

void main (int argc, char *argv[])
{
	std::cout << "Test" << std::endl;

  int n = argc > 1 ? atoi(argv[1]) : 1000;
  int size = argc > 2 ? atoi(argv[2]) : 50;
  //  bool use_linear = (argc > 2);

  Am_Initialize ();

  Am_Object stepper = Am_Stepping_Animator.Create ("stepper")
	      .Set (Am_REPEAT_DELAY, Am_Time())
	      .Set (Am_SMALL_INCREMENT, 1)
	      .Set (Am_DO_METHOD, print_elapsed_time);
  Am_Object linear = Am_Animator.Create ("linear")
	      .Set (Am_MIN_REPEAT_DELAY, Am_Time())
	      .Set (Am_DURATION, Am_Time (10000))
	      .Set (Am_DO_METHOD, print_event_count)
	      ;
  
  std::cout << "Starting test animation..." << std::endl << std::flush;
  Am_Screen
    .Add_Part (Am_Window.Create()
	       .Set (Am_WIDTH, 100)
	       .Set (Am_HEIGHT, 100)
	       .Add_Part (goober = Am_Rectangle.Create()
			  .Set (Am_WIDTH, size)
			  .Set (Am_HEIGHT, size)
			  .Set (Am_LEFT, 0))
	       )
    ;
    
  start = Am_Time::Now();

  for (int i=0; i<n; ++i) {
    goober.Set (Am_LEFT, size/2 - (int)goober.Get (Am_LEFT));      
    Main_Demon_Queue.Invoke ();
    Am_Update_All (); 
  }

  Am_Time stop = Am_Time::Now();

  long ms = (stop - start).Milliseconds();

  std::cout 
    << "Redraw executed " << n << " steps in " << ms << std::endl
      << "for average cost of " << double(ms)/n << " ms per iteration." << std::endl << std::flush;

  std::cout << "Done." << std::endl << std::flush;
  Am_Cleanup ();
}
