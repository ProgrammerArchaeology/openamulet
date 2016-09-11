#ifndef INTER_TEXT_H
#define INTER_TEXT_H

#include "types_method.h"

//Methods of this type go into the Am_TEXT_EDIT_METHOD slot for
//text_edit_interactors
Am_Define_Method_Type(Am_Text_Edit_Method, void,
		      (Am_Object text, Am_Input_Char ic, Am_Object inter))

enum Am_Text_Abort_Or_Stop_Code { Am_TEXT_OK, Am_TEXT_ABORT_AND_RESTORE,
				  Am_TEXT_KEEP_RUNNING, Am_TEXT_STOP_ANYWAY};

//Methods of this type go into the Am_TEXT_CHECK_LEGAL_METHOD slot for
//text_edit_interactors
Am_Define_Method_Type(Am_Text_Check_Legal_Method, Am_Text_Abort_Or_Stop_Code,
		      (Am_Object &text, Am_Object &inter))

//for a text_interactor, calls the Am_TEXT_CHECK_LEGAL_METHOD
extern Am_Text_Abort_Or_Stop_Code am_check_text_legal(Am_Object &inter,
						      Am_Object &text);

#endif // INTER_TEXT_H
