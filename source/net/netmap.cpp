#include <amulet/netmap.h>

/////////////////////////////////////////////////////////////////////////
//Method maps for Am_Network.

Am_Define_Method_Type_Impl (Am_Unmarshall_Method);
Am_Define_Method_Type_Impl (Am_Marshall_Method);

Am_Unmarshall_Method Am_No_Unmarshall_Method;
Am_Marshall_Method Am_No_Marshall_Method;

AM_IMPL_MAP (Unmarshall_Methods, unsigned long, Am_NONE, Am_Unmarshall_Method, Am_No_Unmarshall_Method)
AM_IMPL_MAP (Marshall_Methods, unsigned long, Am_NONE, Am_Marshall_Method, Am_No_Marshall_Method)
AM_IMPL_MAP (Types, Am_Value, Am_NONE, unsigned long, Am_NONE)
AM_IMPL_MAP (Net_IDs, unsigned long, Am_NONE, Am_Value, Am_NONE)
AM_IMPL_MAP (Net_Prototypes, int, 0, char, 'x')












