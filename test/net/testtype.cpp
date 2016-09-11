#include <amulet.h>
#include <iostream>

using namespace std;

Am_Define_Method (Am_Object_Method, void, meth, (Am_Object self))
{
  self.Set (Am_TOP,0);
}

int main (void)
{
  Am_Initialize();
  Am_Value val;
  Am_Value_List list;
  list.Add(Am_String("foo"))
    .Add(Am_String("Bar"))
    .Add(5);
  val=list;

  cout << (int)val.type << ", " << val << endl;

  Am_Object obj=Am_Root_Object.Create("root obj");
  val=obj;
  
  cout << (int)val.type << ", " << val << endl;

  Am_Object obj2=Am_Command.Create("cmd obj");
  val=obj2;

  cout << (int)val.type << ", " << val << endl;

  obj=Am_Window.Create("win obj");
  val=obj;

  cout << (int)val.type << ", " << val << endl;

  obj=Am_Rectangle.Create("rect obj");
  val=obj;

  cout << (int)val.type << ", " << val << endl;

  val=meth;

  cout << (int)val.type << ", " << val << endl;


}
