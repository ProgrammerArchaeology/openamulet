#include "types_wrapper.h"

// A simple string class Used to store retrieve strings from Am_Values and
// similar places.  Uses wrapper reference counting for automatic deallocation.
class Am_String_Data;

class _OA_DL_CLASSIMPORT Am_String {
  AM_WRAPPER_DECL (Am_String)
 public:
  Am_String ()
  { data = (0L); }
  Am_String (const char* string, bool copy = true);

  Am_String& operator= (const char* string);

  operator const char* () const;
  operator char* ();

  bool operator== (const Am_String& test_string) const;
  bool operator!= (const Am_String& test_string) const
     { return !operator==(test_string); }
  bool operator== (const char* test_string) const;
  bool operator!= (const char* test_string) const
     { return !operator==(test_string); }
#if defined(_WIN32)
  bool operator== (char* test_string) const
     { return operator== ((const char*)test_string); }
  bool operator!= (char* test_string) const
     { return !operator==(test_string); }
#endif
};

// The (0L) string.
_OA_DL_IMPORT extern Am_String Am_No_String;

_OA_DL_IMPORT extern std::ostream& operator<< (std::ostream& os, const Am_String& string);
