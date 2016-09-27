#include <am_inc.h>
#include <amulet/am_io.h>
#include <amulet/impl/types_logging.h>
#include <amulet/impl/types_string.h>
#include <amulet/stdvalue.h>

#include <string.h>

/////////////////////////////////////////////////////////////////
// The Am_String procedures
/////////////////////////////////////////////////////////////////

AM_WRAPPER_IMPL(Am_String)

Am_String Am_No_String;

Am_String::Am_String(const char *string, bool copy)
{
  if (string)
    data = new Am_String_Data(string, copy);
  else
    data = nullptr;
}

Am_String &
Am_String::operator=(const char *string)
{
  if (data)
    data->Release();
  if (string)
    data = new Am_String_Data(string);
  else
    data = nullptr;
  return *this;
}

Am_String::operator const char *() const
{
  if (data)
    return *data;
  else
    return nullptr;
}

Am_String::operator char *()
{
  if (data) {
    data = (Am_String_Data *)data->Make_Unique();
    return *data;
  } else
    return nullptr;
}

bool
Am_String::operator==(const Am_String &test_string) const
{
  if (data && test_string.data) {
    const char *string = *data;
    return (string == (const char *)test_string) ||
           !strcmp(string, (const char *)test_string);
  } else
    return data == test_string.data;
}

bool
Am_String::operator==(const char *test_string) const
{
  if (test_string) {
    if (data) {
      const char *string = *data;
      return (string == test_string) || !strcmp(string, test_string);
    } else
      return test_string == nullptr;
  } else
    return data == nullptr;
}

std::ostream &
operator<<(std::ostream &os, const Am_String &string)
{
  if (string.Valid())
    os << (const char *)string;
  return os;
}
