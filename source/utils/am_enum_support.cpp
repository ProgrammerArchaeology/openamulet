#include <am_inc.h>
#include <amulet/am_io.h>
#include <amulet/impl/types_logging.h>
#include <amulet/impl/types_enum.h>

#include <string.h>

const char *
Am_Enum_To_String_Helper(Am_ID_Tag type, long value)
{
  Am_Type_Support *support = Am_Find_Support(type);
  if (support) {
    Am_Value v(value, type);
    return support->To_String(v);
  } else
    return (0L);
}

void
Am_Enum_Print_Helper(std::ostream &out, Am_ID_Tag type, long value)
{
  Am_Type_Support *support = Am_Find_Support(type);
  if (support) {
    Am_Value v(value, type);
    support->Print(out, v);
  } else
    Am_Standard_Print(out, (unsigned long)value, type);
}

Am_Enum_Support::Am_Enum_Support(const char *string, Am_Value_Type in_type)
{
  type = in_type;
  int len = strlen(string);
  char *hold = new char[len + 1];
  strcpy(hold, string);
  int i;
  number = 1;
  for (i = 0; i < len; ++i) {
    if (string[i] == ' ')
      ++number;
  }
  char **array = new char *[number];
  array[0] = hold;
  int j = 1;
  for (i = 0; i < len; ++i) {
    if (hold[i] == ' ') {
      hold[i] = '\0';
      array[j] = &hold[i + 1];
      ++j;
    }
  }
  value_string = hold;
  item = array;
  Am_Register_Support(type, this);
}

Am_Enum_Support::~Am_Enum_Support()
{
  delete[](char *) value_string;
  delete[] item;
}

void
Am_Enum_Support::Print(std::ostream &os, const Am_Value &value) const
{
  long index = value.value.long_value;
  if (index < 0 || index >= number) {
    os << "(";
    Am_Print_Type(os, value.type);
    os << ")" << index << " (ILLEGAL VALUE)";
  } else
    os << item[index];
}

const char *
Am_Enum_Support::To_String(const Am_Value &value) const
{
  long index = value.value.long_value;
  if (index < 0 || index >= number)
    return (0L);
  else
    return item[index];
}

Am_Value
Am_Enum_Support::From_String(const char *string) const
{
  int i;
  for (i = 0; i < number; ++i) {
    if (!strcmp(string, item[i]))
      return Am_Value(i, type);
  }
  return Am_No_Value;
}

Am_Value
Am_Enum_Support::Fetch(int item)
{
  if (item < number)
    return Am_Value(item, type);
  else
    return Am_No_Value;
}
