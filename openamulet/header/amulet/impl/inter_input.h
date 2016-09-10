#ifndef INTER_INPUT_H
#define INTER_INPUT_H

class Am_Input_Event;

// Main top-level input handler.  Figures out which interactors to call for
// the event.  This is called from the Input_Event_Notify method for the
// default event handler class defined in Opal.
extern void (*Interactor_Input_Func)(Am_Object event_window, Am_Input_Event *ev);


// for the Am_CURRENT_STATE slot of interactors
enum Am_Inter_State { Am_INTER_WAITING, Am_INTER_RUNNING, Am_INTER_OUTSIDE,
		       Am_INTER_ABORTING, Am_INTER_ANIMATING };

// for the Am_CURRENT_STATE slot of interactors
enum Am_Modal_Waiting { Am_INTER_NOT_WAITING, Am_INTER_WAITING_MODAL,
			 Am_INTER_WAITING_NOT_MODAL };

#endif // INTER_INPUT_H
