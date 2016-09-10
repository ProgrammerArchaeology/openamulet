#ifndef INTER_DEBUG_H
#define INTER_DEBUG_H

// debugging functions

enum Am_Inter_Trace_Options { Am_INTER_TRACE_NONE, Am_INTER_TRACE_ALL,
			      Am_INTER_TRACE_EVENTS, Am_INTER_TRACE_SETTING,
			      Am_INTER_TRACE_PRIORITIES, Am_INTER_TRACE_NEXT,
			      Am_INTER_TRACE_SHORT };

_OA_DL_IMPORT void Am_Set_Inter_Trace(); //prints current status
_OA_DL_IMPORT void Am_Set_Inter_Trace(Am_Inter_Trace_Options trace_code); //add trace of that
_OA_DL_IMPORT void Am_Set_Inter_Trace(Am_Object inter_to_trace);  //add trace of that inter
_OA_DL_IMPORT void Am_Clear_Inter_Trace(); //set not trace, same as Am_Set_Inter_Trace(0)

//Debugging: These are used internally to see if should print something
extern bool Am_Inter_Tracing (Am_Inter_Trace_Options trace_code);
extern bool Am_Inter_Tracing (Am_Object inter_to_trace);

#ifdef DEBUG

#define Am_INTER_TRACE_PRINT(condition, printout) \
 if (Am_Inter_Tracing(condition))        \
   std::cout << printout << std::endl << std::flush

#define Am_INTER_TRACE_PRINT_NOENDL(condition, printout) \
 if (Am_Inter_Tracing(condition))        \
   std::cout << printout

extern void Am_Report_Set_Vis(Am_Object inter, Am_Object obj, bool value);
#define Am_REPORT_VIS_CONDITION(condition, inter, obj, value) \
 if (Am_Inter_Tracing(condition))       		   \
     Am_Report_Set_Vis(inter, obj, value);

extern void am_report_set_sel_value(bool selected_slot, Am_Object inter_or_cmd,
				    Am_Object obj, bool value);
#define Am_REPORT_SET_SEL_VALUE(selected_slot, inter_or_cmd, obj, value) \
     if (Am_Inter_Tracing(Am_INTER_TRACE_SETTING))			 \
       am_report_set_sel_value(selected_slot, inter_or_cmd, obj, value)

#else

#define Am_INTER_TRACE_PRINT(condition, printout)
    /* if not debugging, define it to be nothing */

#define Am_INTER_TRACE_PRINT_NOENDL(condition, printout)
    /* if not debugging, define it to be nothing */

#define Am_REPORT_VIS_CONDITION(condition, inter, obj, value)
    /* if not debugging, define it to be nothing */

#define Am_REPORT_SET_SEL_VALUE(selected_slot, inter_or_cmd, obj, value)
    /* if not debugging, define it to be nothing */

#endif

#endif // INTER_DEBUG_H
