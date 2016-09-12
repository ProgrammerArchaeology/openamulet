/*
 * dl_import.h
 * Win32 DLL definitions for OpenAmulet
 * 
 * by Paul.Sokolovsky@technologist.com
 *
 * We have three macros:
 *      _OA_DL_IMPORT to mark usual objects,
 * 	_OA_DL_CLASSIMPORT to mark classes, and
 *	_OA_DL_MEMBERIMPORT to mark memers of classes
 * All these must be empty strings for non-Win32 builds.
 * _OA_DL_MEMBERIMPORT is not normally needed, but required to workaround
 * gcc 2.95 bug.
 * 
 * Currently, MSVC and GCC are supported.
 */

#if defined(DLL)
#ifdef OA_SCRIPT_BUILD
#define _OA_SCRIPT_DL_IMPORT __declspec(dllexport)
#else
#define _OA_SCRIPT_DL_IMPORT __declspec(dllimport)
#endif //OA_SCRIPT_BUILD

#ifdef OA_BUILD
#ifdef _MSC_VER
#pragma message("OA DLL Library building mode!")
#define _OA_DL_IMPORT __declspec(dllexport)
#define _OA_DL_CLASSIMPORT __declspec(dllexport)
#define _OA_DL_MEMBERIMPORT
#else
#define _OA_DL_IMPORT __declspec(dllexport)
#define _OA_DL_CLASSIMPORT __declspec(dllexport)
#define _OA_DL_MEMBERIMPORT __declspec(dllexport)
#endif
#else
#ifdef _MSC_VER
#pragma message("Using OA DLL Library!")
#define _OA_DL_IMPORT __declspec(dllimport)
#define _OA_DL_CLASSIMPORT __declspec(dllimport)
#define _OA_DL_MEMBERIMPORT
#else
#define _OA_DL_IMPORT __declspec(dllimport)
#define _OA_DL_CLASSIMPORT __declspec(dllimport)
#define _OA_DL_MEMBERIMPORT
#endif
#endif //OA_BUILD
#else
#define _OA_DL_IMPORT
#define _OA_DL_CLASSIMPORT
#define _OA_DL_MEMBERIMPORT
#define _OA_SCRIPT_DL_IMPORT
#endif
