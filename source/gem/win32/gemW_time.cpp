//
// ------------------------------------------------------------------------
// 		The OpenAmulet User Interface Development Environment
// ------------------------------------------------------------------------
// This code is based on the Amulet project of Carnegie Mellon University,
// and has been placed in the public domain.  If you are using this code
// or any part of OpenAmulet, please contact amulet@cs.cmu.edu to be put
// on the mailing list or have a look at:
// http://www.openip.org
//

#include <windows.h>
#include <mmsystem.h>
#include <am_inc.h>
#include <time.h>
#include <amulet/am_io.h>
#include <amulet/impl/types_logging.h>
#include <amulet/impl/types_string.h>
#include <amulet/impl/types_wrapper.h>

#include <amulet/impl/am_time.h>

#include "amulet/gemW_time.h"

AM_WRAPPER_IMPL(Am_Time)

Am_Time Am_No_Time;

Am_Time::Am_Time() { data = new Am_Time_Data; }

Am_Time::Am_Time(unsigned long milliseconds)
{
  data = new Am_Time_Data;
  data->time = milliseconds;
}

Am_Time
Am_Time::Now()
{
  Am_Time t((unsigned long)timeGetTime());
  return t;
}

bool
Am_Time::operator>(const Am_Time &other) const
{
  return data->time > other.data->time;
}

bool
Am_Time::operator<(const Am_Time &other) const
{
  return data->time < other.data->time;
}

bool
Am_Time::operator>=(const Am_Time &other) const
{
  return data->time >= other.data->time;
}

bool
Am_Time::operator<=(const Am_Time &other) const
{
  return data->time <= other.data->time;
}

Am_Time
Am_Time::operator+(unsigned long milliseconds) const
{
  Am_Time rt;

  rt.data->time = data->time + milliseconds;
  return rt;
}

Am_Time
Am_Time::operator-(unsigned long milliseconds) const
{
  Am_Time rt;

  rt.data->time = data->time - milliseconds;
  return rt;
}

Am_Time
Am_Time::operator+(const Am_Time &other) const
{
  Am_Time rt;

  rt.data->time = data->time + other.data->time;
  return rt;
}

Am_Time
Am_Time::operator-(const Am_Time &other) const
{
  Am_Time rt;

  rt.data->time = data->time - other.data->time;
  return rt;
}

void
Am_Time::operator+=(const Am_Time &other)
{
  data->time += other.data->time;
}

void
Am_Time::operator-=(const Am_Time &other)
{
  data->time -= other.data->time;
}

void
Am_Time::operator+=(unsigned long milliseconds)
{
  data->time += milliseconds;
}

void
Am_Time::operator-=(unsigned long milliseconds)
{
  data->time -= milliseconds;
}

unsigned long Am_Time::Milliseconds() const // return # of milliseconds
{
  return data->time;
}

bool
Am_Time::Zero() const
{
  return (data->time == 0);
}

bool
Am_Time::Is_Future() const
{
  return (*this > Now());
}

bool
Am_Time::Is_Past() const
{
  return (*this < Now());
}

AM_WRAPPER_DATA_IMPL(Am_Time, (this))

void
Am_Time_Data::Print(std::ostream &os) const
{
  os << time << " milliseconds";
}

std::ostream &
operator<<(std::ostream &os, const Am_Time &time)
{
  time.Print(os);
  return os;
}

//returns the current time and date as a string, like
//  "Fri Jan 17 16:03:55 EST 1997\n".
Am_String
Am_Get_Time_And_Date()
{
  Am_String str;
  time_t time_ptr;
  (void)time(&time_ptr);

  char *s = ctime(&time_ptr);
  str = s;
  return str;
}
