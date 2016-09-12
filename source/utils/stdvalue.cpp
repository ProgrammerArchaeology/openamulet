/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <string.h>

#include <am_inc.h>

#include STDVALUE__H
#include TYPES__H

AM_WRAPPER_DATA_IMPL_ID (Am_Double, (value), Am_DOUBLE)

bool Am_Double_Data::operator== (const Am_Double_Data& test_value) const
{
  return (this == &test_value) || ((double)test_value == value);
}

Am_Double_Data::operator double () const
{
  return value;
}

Am_Double_Data::Am_Double_Data (double in_value)
{
  value = in_value;
}

void Am_Double_Data::Print (std::ostream& out) const {
  out << value;
}
  
AM_WRAPPER_DATA_IMPL_ID (Am_String, (value), Am_STRING)

bool Am_String_Data::operator== (const Am_String_Data& test_value) const
{
  return (this == &test_value) || !strcmp ((const char*)test_value, value);
}

bool Am_String_Data::operator== (const char* test_value) const
{
  return (value == test_value) || !strcmp (test_value, value);
}

Am_String_Data::operator const char* () const
{
  return value;
}

Am_String_Data::operator char* () const
{
  return (char*)value;
}

Am_String_Data::Am_String_Data (const char* in_value, bool copy)
{
  if (copy) {
    char* temp;
	int size = strlen (in_value) + 1;
    temp = new char [size];
    strcpy (temp, in_value);
    value = temp;
  }
  else
    value = in_value;
}

Am_String_Data::~Am_String_Data ()
{
  if (value) delete[] (char*)value;
}

void Am_String_Data::Print (std::ostream& out) const
{
  out << value;
}
const char * Am_String_Data::To_String() const
{
  return value;
}

Am_Value Am_String_Data::From_String (const char * string) const
{
  Am_Value v(string);
  return v;
}

