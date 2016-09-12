/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

class Am_Time_Data : public Am_Wrapper
{
  friend class Am_Time;
  AM_WRAPPER_DATA_DECL(Am_Time)
public:
  void Print (std::ostream& out) const;
  Am_Time_Data() { time = 0; }
  Am_Time_Data(Am_Time_Data* other) 
    { time = other->time; }
  unsigned long time;
  bool operator== (Am_Time_Data& other)
    { return time == other.time; }
  bool operator== (Am_Time_Data& other) const
    { return time == other.time; }
};

