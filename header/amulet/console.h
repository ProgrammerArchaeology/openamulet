#include <stdiostr.h>
#include <iostream>

class SetupConsole
{
public:
  SetupConsole();
  ~SetupConsole();
  void setFile(const char* fileName);
private:
  std::iostream *myStreamIn;
  std::iostream *myStreamOut;
  std::iostream *myStreamErr;
};
