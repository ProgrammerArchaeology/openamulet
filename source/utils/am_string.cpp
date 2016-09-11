#include <am_inc.h>
#include AM_IO__H
#include <amulet/impl/types_logging.h>
#include <amulet/impl/types_string.h>
#include STDVALUE__H

#include<string.h>

/////////////////////////////////////////////////////////////////
// The Am_String procedures
/////////////////////////////////////////////////////////////////

Am_WRAPPER_IMPL (Am_String)

Am_String Am_No_String;

Am_String::Am_String (const char* string, bool copy)
{
  if (string)
    data = new Am_String_Data (string, copy);
  else data = (0L);
}

Am_String& Am_String::operator= (const char* string)
{
  if (data)
    data->Release ();
  if (string)
    data = new Am_String_Data (string);
  else data = (0L);
  return *this;
}

Am_String::operator const char* () const
{
  if (data)
    return *data;
  else
    return (0L);
}

// GCC 2.95 produces this kind of warning if this cast is not defined:
// warning: choosing `Am_String::operator char *()' over `Am_String::operator const char *() const'
// warning:   for conversion from `Am_String' to `const char *'
// warning:   because conversion sequence for the argument is better
Am_String::operator const char* ()
{
  if (data) {
    data = (Am_String_Data*)data->Make_Unique ();
    return *data;
  }
  else
    return (0L);
}

Am_String::operator char* ()
{
  if (data) {
    data = (Am_String_Data*)data->Make_Unique ();
    return *data;
  }
  else
    return (0L);
}

const char* Am_String::constant () const
{
  if (data)
    return *data;
  else
    return (0L);
}

bool Am_String::operator== (const Am_String& test_string) const
{
  if (data && test_string.data) {
    const char* string = *data;
    return (string == (const char*)test_string) ||
           !strcmp (string, (const char*)test_string);
  }
  else
    return data == test_string.data;
}

bool Am_String::operator== (const char* test_string) const
{
  if (test_string) {
    if (data) {
      const char* string = *data;
      return (string == test_string) ||
        !strcmp (string, test_string);
    }
    else
      return test_string == (0L);
  } else
      return data == (0L);
}

std::ostream& operator<< (std::ostream& os, const Am_String& string)
{
  if (string.Valid ())
    os << (const char*) string;
  return os;
}

