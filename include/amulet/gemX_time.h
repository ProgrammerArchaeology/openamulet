/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

#include <sys/time.h>

class Am_Time_Data : public Am_Wrapper
{
  friend class Am_Time;
  AM_WRAPPER_DATA_DECL(Am_Time)
public:
  void Print(std::ostream &out) const;
  Am_Time_Data()
  {
    time.tv_usec = 0;
    time.tv_sec = 0;
  }
  Am_Time_Data(Am_Time_Data *other)
  {
    time.tv_usec = other->time.tv_usec;
    time.tv_sec = other->time.tv_sec;
  }
  timeval time;
  bool operator==(Am_Time_Data &other)
  {
    return time.tv_usec == other.time.tv_usec &&
           time.tv_sec == other.time.tv_sec;
  }
  bool operator==(Am_Time_Data &other) const
  {
    return time.tv_usec == other.time.tv_usec &&
           time.tv_sec == other.time.tv_sec;
  }
};
