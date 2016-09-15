/* 
  designed and implemented  by S. Nebel Linkworks Ltd. Wellinton, NZ 
  Implements a MS Filemanager like tree.
  
  This code is public domain. 

  Use it on own risk.

*/

#ifndef __OUTLINE_H
#define __OUTLINE_H
#include <amulet.h>


class Am_Outline
{
 public:
  Am_Outline();
  virtual void DestroyNode(Am_Object am) {am.Destroy();}
  virtual ~Am_Outline();
  virtual Am_Object  Root (long id, Am_Object item);  
  virtual Am_Object  AddNode (long id, long addto, Am_Object item); 
  virtual bool RemoveNode(long id);
  virtual bool RemoveNode(Am_Object  am);
  virtual bool ShowSubtree(long id, bool show);
  Am_Object FindNode(long id); 
  Am_Object GetRoot() {return (_rootObject);}
 protected:
  Am_Object FindNodeInternal(Am_Object  am, long id);
  void RemoveSubtree(Am_Object am);
  Am_Object _rootObject;
};



class Outline: public Am_Outline
{
 public:
  Outline(Am_Object  group) ;
  Outline() : Am_Outline(), _group(Am_No_Object) {};
  virtual ~Outline();
  virtual Am_Object  Root (long id, Am_Object item);  
  virtual Am_Object  Get_Root();
  virtual Am_Object  AddNode (long id, long addto, Am_Object item); 
  virtual void SetGroup(Am_Object group) { _group = group;}
  virtual void AddBmVi(Am_Object object);
  virtual void RemoveBm(Am_Object object);
  virtual void DestroyNode(Am_Object am);    
  virtual bool RemoveNode(long id);
  virtual bool  RemoveNode(Am_Object am) { return(Am_Outline::RemoveNode(am));}
 protected:
  Am_Object  _group;
};
#endif














