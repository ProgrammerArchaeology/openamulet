#ifndef TYPEDEFS_HPP
#define TYPEDEFS_HPP

// Forward class declarations, these are needed here to get the typedefs
// declared without any other includes
class Am_Object;
class Am_Slot;
class Am_Value;
class Am_Wrapper;
class Am_Method_Wrapper;

// Am_Ptr, don't move to other place within this file!
#if defined(_MSC_VER)
typedef unsigned char *Am_Ptr;
#else
typedef void *Am_Ptr;
#endif

typedef unsigned short
    Am_Slot_Key; // Am_Slot_Key is used to name the slots of objects
typedef unsigned long Am_Slot_Flags;
typedef unsigned long
    Am_Text_Index; // An index within a text object. Could be the location of a character, a mark, an object, a fragment's range, et cetera.
typedef long
    Am_Text_Offset; // Used to specify an index relative to some Am_Text_Index
typedef unsigned long Am_Text_Length;
typedef long Am_Cursor_Ref;
typedef unsigned short Am_Demon_Protocol;
typedef void *Am_Constraint_Tag;
typedef unsigned Am_Set_Reason;
typedef unsigned short Am_Formula_Mode;
typedef unsigned short
    Am_ID_Tag; // Am_ID_Tag is used to name classes derived from Am_Wrapper
typedef Am_ID_Tag
    Am_Value_Type;         // Am_Value_Type is an enumeration of all the distinct types that can be stored in a Am_Value object
typedef void *Am_Position; // Position type for lists

typedef void
Am_Generic_Procedure(); // A procedure type to use as a placeholder, don't move to other place within this file!
typedef void Am_Any_Procedure(
    ...); // Is used for Am_Null_Method_Error_Function as this function takes any number of parameters
typedef void
Am_Object_Demon(Am_Object object); // Object Demon - connected to objects
typedef void Am_Slot_Demon(
    Am_Slot first_invalidated); // Slot Demon 	- connected to slots
typedef const char *Am_Type_Check(
    const Am_Value
        &value); // Type Check 	- connected to slots, used for testing types
typedef void Am_Text_Edit_Operation(
    Am_Object
        text); // functions of this type go into the Am_Edit_Translation_Table hash table
typedef void Am_Part_Demon(
    Am_Object owner, Am_Object old_object,
    Am_Object
        new_object); // Part Demon 	- connected to objects, but used when parts are added/removed
typedef void Am_Slot_Set_Trace_Proc(const Am_Slot &slot, Am_Set_Reason reason);
typedef void Am_Object_Create_Trace_Proc(const Am_Object &old_object,
                                         const Am_Object &new_object,
                                         Am_Set_Reason reason);
typedef void Am_Method_Get_Trace_Proc(const Am_Object &of_object,
                                      Am_Slot_Key slot_key,
                                      Am_Value method_value);

typedef Am_Wrapper *Am_FProc_Wrapper(Am_Object &context);
typedef Am_Ptr Am_FProc_Void(Am_Object &context);
typedef int Am_FProc_Int(Am_Object &context);
typedef long Am_FProc_Long(Am_Object &context);
typedef bool Am_FProc_Bool(Am_Object &context);
typedef float Am_FProc_Float(Am_Object &context);
typedef double Am_FProc_Double(Am_Object &context);
typedef char Am_FProc_Char(Am_Object &context);
typedef char *Am_FProc_String(Am_Object &context);
typedef const char *Am_FProc_Const_String(Am_Object &context);
typedef Am_Generic_Procedure *Am_FProc_Proc(Am_Object &context);
typedef Am_Method_Wrapper *Am_FProc_Method(Am_Object &context);
typedef Am_Value Am_FProc_Value(Am_Object &context);
typedef const Am_Value Am_FProc_Const_Value(Am_Object &context);

// some global macros, should be moved to a separate file RMM
#define DECLARE_SLOT(x) Am_Slot_Key x = Am_Register_Slot_Name(#x)
#define EXTERN_SLOT(x) extern Am_Slot_Key x
#define EXTERN_FORMULA(x) extern Am_Formula x
#define EXTERN_METHOD(Method) extern Am_Object_Method Method

#endif
