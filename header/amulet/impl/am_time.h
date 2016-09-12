// Time structure for timing events, etc.
class Am_Time_Data;

class _OA_DL_CLASSIMPORT Am_Time 
{
  AM_WRAPPER_DECL (Am_Time)
public:
  Am_Time();  // defaults to 0 delta time
  Am_Time (unsigned long milliseconds); // starting number of msec
  static Am_Time Now();
  bool Is_Future () const;
  bool Is_Past () const;
  bool operator> (const Am_Time& other) const;
  bool operator< (const Am_Time& other) const;
  bool operator>= (const Am_Time& other) const;
  bool operator<= (const Am_Time& other) const;

  unsigned long Milliseconds() const;
  
  Am_Time operator- (unsigned long milliseconds) const;
  Am_Time operator+ (unsigned long milliseconds) const;
  Am_Time operator- (const Am_Time& other) const;
  Am_Time operator+ (const Am_Time& other) const;
  void operator+= (unsigned long milliseconds);
  void operator-= (unsigned long milliseconds);
  void operator+= (const Am_Time& other);
  void operator-= (const Am_Time& other);

  bool Zero() const;  // is the delta time 0?  (uninitialized time test)
};

_OA_DL_IMPORT std::ostream& operator<< (std::ostream& os, const Am_Time& time);

_OA_DL_IMPORT extern Am_Time Am_No_Time;

//returns the current time and date as a string, like
//  "Fri Jan 17 16:03:55 EST 1997\n".
_OA_DL_IMPORT extern Am_String Am_Get_Time_And_Date(); 
