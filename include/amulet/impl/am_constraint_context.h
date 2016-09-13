#ifndef OBJECT_CONSTR_CONTEXT_H
#define OBJECT_CONSTR_CONTEXT_H

class _OA_DL_CLASSIMPORT Am_Constraint_Context
{
public:
  virtual Am_ID_Tag ID() = 0;

  virtual const Am_Value &Get(const Am_Object_Advanced &object, Am_Slot_Key key,
                              Am_Slot_Flags flags) = 0;

  virtual void Set(const Am_Object_Advanced &object, Am_Slot_Key key,
                   const Am_Value &new_value, Am_Slot_Flags flags) = 0;

  virtual void Note_Changed(const Am_Object_Advanced &object,
                            Am_Slot_Key key) = 0;
  virtual void Note_Unchanged(const Am_Object_Advanced &object,
                              Am_Slot_Key key) = 0;

  virtual const Am_Value &Raise_Get_Exception(const Am_Value &value,
                                              const Am_Object_Advanced &object,
                                              Am_Slot_Key key,
                                              Am_Slot_Flags flags,
                                              const char *msg) = 0;

  virtual Am_Wrapper *Get_Data() = 0;
  virtual void Set_Data(Am_Wrapper *data) = 0;
};

#define Am_PUSH_CC(cc)                                                         \
  {                                                                            \
    Am_Constraint_Context *old_cc = Am_Object_Advanced::Swap_Context(cc);

#define Am_POP_CC()                                                            \
  Am_Object_Advanced::Swap_Context(old_cc);                                    \
  }

// An empty constraint context.  Gets and sets directly to the object.
_OA_DL_IMPORT extern Am_Constraint_Context *Am_Empty_Constraint_Context;

#define Am_PUSH_EMPTY_CC() Am_PUSH_CC(Am_Empty_Constraint_Context)
#define Am_POP_EMPTY_CC() Am_POP_CC()

#endif // OBJECT_CONSTR_CONTEXT_H
