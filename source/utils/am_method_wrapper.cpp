#include <am_inc.h>
#include <amulet/am_io.h>
#include <amulet/impl/types_logging.h>
#include <amulet/impl/types_method.h>
#ifdef DEBUG
#include REGISTRY__H
#endif

//the default looks up the name, and if found, returns it.  Otherwise,
//returns Am_No_Object.  Ignores the item it is called on.
#ifdef DEBUG
Am_Value
Am_Method_Wrapper::From_String(const char *string) const
{
  Am_Method_Wrapper *item = (Am_Method_Wrapper *)Am_Get_Named_Item(string);
  if (item) {
    Am_Value v(item);
    return v;
  }
  return Am_No_Value;
}
#else
Am_Value
Am_Method_Wrapper::From_String(const char * /* string */) const
{
  return Am_No_Value;
}
#endif

void
Am_Method_Wrapper::Print(std::ostream &os) const
{
#ifdef DEBUG
  const char *name = Am_Get_Name_Of_Item(this);
  if (name) {
    os << name;
    return;
  }
#endif
  Am_Standard_Print(os, (unsigned long)this->Call, ID());
}

#ifdef DEBUG
Am_Method_Wrapper::Am_Method_Wrapper(Am_ID_Tag *id_ptr, Am_Generic_Procedure *p,
                                     const char *name)
#else
Am_Method_Wrapper::Am_Method_Wrapper(Am_ID_Tag *id_ptr, Am_Generic_Procedure *p,
                                     const char * /* name */)
#endif
{
  ID_Ptr = id_ptr;
  Call = p;
#ifdef DEBUG
  Am_Register_Name(this, name);
#endif
}
