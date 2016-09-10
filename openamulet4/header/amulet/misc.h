// misc.h
// Misc. machine dependant utility routines.  

_OA_DL_IMPORT void Am_Break_Into_Debugger ();
_OA_DL_IMPORT void Am_Wait(int milliseconds);

_OA_DL_IMPORT char *Am_Get_Amulet_Pathname ();
_OA_DL_IMPORT char *Am_Merge_Pathname(char *name);

#ifdef _MACINTOSH
void Am_Init_Pathname();
#endif
