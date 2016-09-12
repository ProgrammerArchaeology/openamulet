/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <am_inc.h>
#include TYPES__H
#include GEM__H

#include "amulet/gemX_time.h"
#include <sys/time.h>

AM_WRAPPER_IMPL(Am_Time);

Am_Time Am_No_Time;

Am_Time::Am_Time() { data = new Am_Time_Data; }

Am_Time::Am_Time(unsigned long milliseconds)
{
  data = new Am_Time_Data;
  data->time.tv_sec = milliseconds / 1000;
  data->time.tv_usec = (milliseconds % 1000) * 1000;
}

Am_Time
Am_Time::Now()
{
  Am_Time t;
  gettimeofday(&(t.data->time), (0L));
  return t;
}

// A generic comparison for two timevals.  Should be defined in <sys/time.h>
#ifndef timercmp
#define timercmp(tvp, uvp, cmp)                                                \
  (((tvp)->tv_sec == (uvp)->tv_sec && (tvp)->tv_usec cmp(uvp)->tv_usec) ||     \
   (tvp)->tv_sec cmp(uvp)->tv_sec)
#endif
// otherwise assume it does the Right Thing.

bool
Am_Time::operator>(const Am_Time &other) const
{
  return timercmp(&(data->time), &(other.data->time), >);
}

bool
Am_Time::operator<(const Am_Time &other) const
{
  return timercmp(&(data->time), &(other.data->time), <);
}

bool
Am_Time::operator>=(const Am_Time &other) const
{
  return timercmp(&(data->time), &(other.data->time), >=);
}

bool
Am_Time::operator<=(const Am_Time &other) const
{
  return timercmp(&(data->time), &(other.data->time), <=);
}

Am_Time
Am_Time::operator+(unsigned long milliseconds) const
{
  Am_Time rt;
  long microseconds = milliseconds * 1000;
  long total_usec = data->time.tv_usec + microseconds;
  rt.data->time.tv_sec = data->time.tv_sec + total_usec / 1000000;
  rt.data->time.tv_usec = total_usec % 1000000;
  return rt;
}

Am_Time
Am_Time::operator-(unsigned long milliseconds) const
{
  Am_Time rt;
  long microseconds = milliseconds * 1000;
  long total_usec = data->time.tv_usec - microseconds;
  long borrowed = 0;
  while (total_usec < 0) {
    total_usec += 1000000;
    borrowed++;
  }
  rt.data->time.tv_sec = data->time.tv_sec - borrowed + total_usec / 1000000;
  rt.data->time.tv_usec = total_usec % 1000000;
  return rt;
}

Am_Time
Am_Time::operator+(const Am_Time &other) const
{
  Am_Time rt;
  long total_usec = data->time.tv_usec + other.data->time.tv_usec;
  rt.data->time.tv_sec =
      data->time.tv_sec + other.data->time.tv_sec + total_usec / 1000000;
  rt.data->time.tv_usec = total_usec % 1000000;
  return rt;
}

Am_Time
Am_Time::operator-(const Am_Time &other) const
{
  Am_Time rt;
  long total_usec = 1000000 + data->time.tv_usec - other.data->time.tv_usec;
  rt.data->time.tv_sec = (data->time.tv_sec) - (other.data->time.tv_sec) - 1 +
                         (total_usec / 1000000); // borrow
  rt.data->time.tv_usec = (total_usec % 1000000);
  return rt;
}

void
Am_Time::operator+=(const Am_Time &other)
{
  long total_usec = data->time.tv_usec + other.data->time.tv_usec;
  data->time.tv_sec =
      data->time.tv_sec + other.data->time.tv_sec + total_usec / 1000000;
  data->time.tv_usec = total_usec % 1000000;
}

void
Am_Time::operator-=(const Am_Time &other)
{
  long total_usec = 1000000 + data->time.tv_usec - other.data->time.tv_usec;
  data->time.tv_sec = data->time.tv_sec - other.data->time.tv_sec - 1 // borrow
                      + total_usec / 1000000;
  data->time.tv_usec = total_usec % 1000000;
}

void
Am_Time::operator+=(unsigned long msec)
{
  long total_usec = data->time.tv_usec + msec * 1000;
  data->time.tv_sec = data->time.tv_sec + total_usec / 1000000;
  data->time.tv_usec = total_usec % 1000000;
}

void
Am_Time::operator-=(unsigned long msec)
{
  long total_usec = data->time.tv_usec - msec * 1000;
  while (total_usec < 0) {
    total_usec += 1000000;
    data->time.tv_sec--;
  }
  data->time.tv_sec = data->time.tv_sec + total_usec / 1000000;
  data->time.tv_usec = total_usec % 1000000;
}

unsigned long Am_Time::Milliseconds() const // return # of milliseconds
{
  return (unsigned long)data->time.tv_sec * 1000 + data->time.tv_usec / 1000;
}

bool
Am_Time::Zero() const
{
  return (data->time.tv_sec == 0) && (data->time.tv_usec == 0);
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

AM_WRAPPER_DATA_IMPL(Am_Time, (this));

void
Am_Time_Data::Print(std::ostream &os) const
{
  //  os << time.tv_sec <<"s:"<< time.tv_usec << "us";
  os << time.tv_sec * 1000 + time.tv_usec / 1000 << " milliseconds";
}

std::ostream &
operator<<(std::ostream &os, const Am_Time &time)
{
  time.Print(os);
  return os;
}

Am_String
Am_Get_Time_And_Date()
{
  Am_String str;
  time_t time_ptr;
  time(&time_ptr);

  char *s = ctime(&time_ptr);
  str = s;
  return str;
}
