#ifndef AM_VALUE_HPP
#define AM_VALUE_HPP

#include <iostream>
#include "amulet/impl/types_basic.h"

// class std::ostream;

class Am_Registered_Type; 	//forward reference
class Am_Wrapper; 			//forward reference
class Am_Method_Wrapper; 	//forward reference
class Am_String; 			//forward reference (see below)

// Am_Value_Type is an enumeration of all the distinct types that can be
// stored in a Am_Value object.
typedef Am_ID_Tag Am_Value_Type;

// Am_Ptr
#if defined(_MSC_VER)
	typedef unsigned char	*Am_Ptr;
#else
	typedef void			*Am_Ptr;
#endif

// A procedure type to use as a placeholder
typedef void Am_Generic_Procedure ();

// Wrapper flag
const Am_Value_Type Am_REF_COUNTED 			= 0x8000;

// Basic supertypes. Can be 3 bits (1000 to 7000)
const Am_Value_Type Am_SIMPLE_TYPE      	= 0x0000;
const Am_Value_Type Am_ERROR_VALUE_TYPE 	= 0x1000;
const Am_Value_Type Am_WRAPPER 				= 0x2000;
const Am_Value_Type Am_METHOD  				= 0x3000;
const Am_Value_Type Am_ENUM    				= 0x4000;

// Basic types.
const Am_Value_Type Am_NONE    				= 0;
const Am_Value_Type Am_ZERO    				= 1;
const Am_Value_Type Am_INT     				= 2;
const Am_Value_Type Am_LONG    				= 3;
const Am_Value_Type Am_BOOL    				= 4;
const Am_Value_Type Am_FLOAT   				= 5;
const Am_Value_Type Am_DOUBLE  				= 6 | Am_REF_COUNTED;
const Am_Value_Type Am_CHAR    				= 7;
const Am_Value_Type Am_STRING  				= 8 | Am_REF_COUNTED;
const Am_Value_Type Am_VOIDPTR 				= 9;
const Am_Value_Type Am_PROC    				= 10;
const Am_Value_Type Am_WRAPPER_TYPE 		= Am_WRAPPER | Am_REF_COUNTED;
const Am_Value_Type Am_METHOD_TYPE  		= Am_METHOD;
const Am_Value_Type Am_ENUM_TYPE    		= Am_ENUM;
const Am_Value_Type Am_FORMULA_INVALID     	= Am_ERROR_VALUE_TYPE | 1; //4097
const Am_Value_Type Am_MISSING_SLOT  	   	= Am_ERROR_VALUE_TYPE | 2; //4098
const Am_Value_Type Am_GET_ON_NULL_OBJECT  	= Am_ERROR_VALUE_TYPE | 3; //4099

const Am_Value_Type Am_TYPE_TYPE     		= Am_ENUM | 20 ; //type of type names
const Am_Value_Type Am_SLOT_KEY_TYPE 		= Am_ENUM | 21 ; //type of slot keys

// This class is a union of all the distinct types that can be stored as a
// single entity.  It is used as object slots and value list items.  Certain
// Get and Set functions use the 'type' datamember to make it possible to set/retrieve
// the value without having to know its type explicitly.
// As the union has an Am_Wrapper member too, it's possible to cover user-defined
// types as an Am_Value and therefore store user-defined types into a slot.

class _OA_DL_CLASSIMPORT Am_Value
{
	public:
		Am_Value_Type type;

		// Storage for the value
		union
		{
			Am_Wrapper				*wrapper_value;
			Am_Ptr                	voidptr_value;
			long                  	long_value;
			bool                  	bool_value;
			float                 	float_value;
			char                  	char_value;  	// FIX: should we get rid of this?  could put in long
			Am_Method_Wrapper		*method_value;
			Am_Generic_Procedure	*proc_value; 	//like void* for procedures
		} value;
                
	// Casting operators to mitigate some syntax
	operator Am_Wrapper*() 				const;
	operator Am_Ptr() 					const;
	operator int() 						const;
	operator long() 					const;
	operator float() 					const;
	operator double() 					const;
	operator char() 					const;
	operator Am_Generic_Procedure*() 	const;
	operator Am_Method_Wrapper*() 		const;
	#ifndef NEED_BOOL
	  operator bool() 					const;
	#endif

  bool operator== (Am_Wrapper* test_value) const;
  bool operator== (Am_Ptr test_value) const;
  bool operator== (int test_value) const;
  bool operator== (long test_value) const;
#ifndef NEED_BOOL
  bool operator== (bool test_value) const;
#endif
  bool operator== (float test_value) const;
  bool operator== (double test_value) const;
  bool operator== (char test_value) const;
  bool operator== (const char* test_value) const;
  bool operator== (const Am_String& test_value) const;
  bool operator== (Am_Generic_Procedure* test_value) const;
  bool operator== (Am_Method_Wrapper* test_value) const;
  bool operator== (const Am_Value& test_value) const;

  bool operator!= (Am_Wrapper* test_value) const;
  bool operator!= (Am_Ptr test_value) const;
  bool operator!= (int test_value) const;
  bool operator!= (long test_value) const;
#ifndef NEED_BOOL
  bool operator!= (bool test_value) const;
#endif
  bool operator!= (float test_value) const;
  bool operator!= (double test_value) const;
  bool operator!= (char test_value) const;
  bool operator!= (const char* test_value) const;
  bool operator!= (const Am_String& test_value) const;
  bool operator!= (Am_Generic_Procedure* test_value) const;
  bool operator!= (Am_Method_Wrapper* test_value) const;
  bool operator!= (const Am_Value& test_value) const;

	// Returns true if value represents an actual slot value; false if value is an error code (like Am_MISSING_SLOT)
	bool Safe() const;
	// Returns true if value contains a piece of typed data; false if value is an error code or an empty value (Am_NONE)
	bool Exists() const;
	// Returns true if value's binary representation is nonzero; false if not Exists() or is zero
	bool Valid() const;

	// Clears contents and sets to Am_NONE
	void Set_Empty();

	// Clears contents and makes value be of the specified type
	void Set_Value_Type(Am_Value_Type new_type);

	// Creation operations provided to aid initialization.
	Am_Value()
	{
		type 				= Am_NONE;
		value.long_value 	= 0;
	}
	Am_Value(Am_Wrapper *initial);
	Am_Value(Am_Ptr initial)
	{
		type 				= Am_VOIDPTR;
		value.voidptr_value = initial;
	}
	Am_Value(int initial)
	{
		type 				= Am_INT;
		value.long_value 	= initial;
	}
	Am_Value(long initial)
	{
		type 				= Am_LONG;
		value.long_value 	= initial;
	}
	#if !defined(NEED_BOOL)
		Am_Value(bool initial)
		{
			type 				= Am_BOOL;
			value.bool_value 	= initial;
		}
	#endif
	Am_Value(float initial)
	{
		type = Am_FLOAT;
		value.float_value = initial;
	}
	Am_Value(double initial);
	Am_Value(char initial)
	{
		type 				= Am_CHAR;
		value.long_value 	= 0;  		// a temporary expediency: clear value first!!!
		value.char_value 	= initial;
	}
	Am_Value(const char* initial);
	Am_Value(const Am_String& initial);
	Am_Value(Am_Generic_Procedure* initial)
	{
		type = Am_PROC;
		value.proc_value = initial;
	}
	Am_Value(Am_Method_Wrapper* initial);
	Am_Value(long in_value, Am_Value_Type in_type)
	{
		type = in_type;
		value.long_value = in_value;
	}
	Am_Value(const Am_Value& initial);

	~Am_Value();

	// Assignment operators have been provided to ease some syntax juggling.
	Am_Value& operator= (Am_Wrapper* in_value);
	Am_Value& operator= (Am_Ptr in_value);
	Am_Value& operator= (int in_value);
	Am_Value& operator= (long in_value);
#ifndef NEED_BOOL
	Am_Value& operator= (bool in_value);
#endif
	Am_Value& operator= (float in_value);
	Am_Value& operator= (double in_value);
	Am_Value& operator= (char in_value);
	Am_Value& operator= (const char* in_value);
	Am_Value& operator= (const Am_String& in_value);
	Am_Value& operator= (Am_Generic_Procedure* in_value);
	Am_Value& operator= (Am_Method_Wrapper* in_value);
	Am_Value& operator= (const Am_Value& in_value);

	void Print(std::ostream& out) const;
	void Println() const;

	//will use strstream and Print(out) if the value doesn't have a string name.
	const char * To_String() const;

	// if the string is in the global name registry, then the type is
	// not needed.  If the name is not there, then looks for a Am_Type_Support
	// for the ID.  Will return Am_No_Value if can't be found or can't be parsed.
	static Am_Value From_String(const char * string, Am_ID_Tag type = 0);
};

_OA_DL_IMPORT extern const 		Am_Value Am_No_Value;
_OA_DL_IMPORT extern			Am_Value Am_No_Value_Non_Const;
_OA_DL_IMPORT extern 			Am_Value Am_Zero_Value;

_OA_DL_IMPORT extern std::ostream& operator<< (std::ostream& os, const Am_Value& value);

//not very efficient, mostly for use inside the debugger.  Returns a
//string for any value.
_OA_DL_IMPORT extern const char* Am_To_String(const Am_Value &value);

_OA_DL_IMPORT extern void Am_Print_Type (std::ostream& os, Am_Value_Type type);

#endif // AM_VALUE_HPP
