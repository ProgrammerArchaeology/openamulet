/***************************************************************/
#include <amulet.h> 
#include <am_inc.h>
#include TYPES__H

Am_DECL_MAP(objstr, Am_Object, Am_String);
Am_DECL_MAP(objpstr, Am_Object*, Am_String);
Am_DECL_MAP(objchr, Am_Object, char *);
Am_DECL_MAP(objpchr, Am_Object*, char*);

Am_IMPL_MAP(objstr, Am_Object,  Am_No_Object, Am_String,  Am_No_String);
Am_IMPL_MAP(objpstr, Am_Object*,  &Am_No_Object, Am_String,  Am_No_String);
Am_IMPL_MAP(objchr, Am_Object, Am_No_Object, char *,  (0L));
Am_IMPL_MAP(objpchr, Am_Object*,  &Am_No_Object,  char *,  (0L));

  Am_Map_objstr os;
  Am_Map_objpstr ops;
  Am_Map_objchr oc;
  Am_Map_objpchr opc;
  Am_Object test;

void setstr(void)
{
  Am_String foo="Foo";
  char bar[5]="bar";


  os.SetAt(test,foo);
  ops.SetAt(&test,foo);
  oc.SetAt(test,bar);
  opc.SetAt(&test,bar);
}

main ()
{


  Am_Initialize();
  test=Am_Text.Create("test");
  setstr();

  Am_String fout;
  char *bout;
  
  fout=os.GetAt(test);
  cout << fout<< endl;
  fout=ops.GetAt(&test);
  cout << fout<< endl;
  bout=oc.GetAt(test);
  cout << bout<< endl;
  bout=opc.GetAt(&test);
  cout << bout << endl;
  Am_Cleanup();

}

  
