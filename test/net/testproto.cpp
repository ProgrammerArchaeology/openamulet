#include <amulet.h>
#include <am_inc.h>
#include OBJECT_ADVANCED__H
#include <iostream>

using namespace std;

int
main(void)
{
  Am_Initialize();

  Am_Object proto_rect = Am_Rectangle.Create("proto_rect")
                             .Set(Am_TOP, 10)
                             .Set(Am_LEFT, 10)
                             .Set(Am_WIDTH, 100)
                             .Set(Am_HEIGHT, 100);

  Am_Connection *mc = Am_Connection::Open();
  mc->Register_Prototype("rect", proto_rect);
  cout << mc->Is_Registered_Prototype(&proto_rect) << endl;

  ////////////////////////
  Am_Cleanup();

  return 0;
}
