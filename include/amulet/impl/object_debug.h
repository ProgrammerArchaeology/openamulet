#ifdef DEBUG

#define Am_TRACE_NOT_SPECIFIED 0x00
#define Am_TRACE_SLOT_SET 0x01
#define Am_TRACE_SLOT_CHANGED 0x02
#define Am_TRACE_CONSTRAINT_FETCH 0x03
#define Am_TRACE_SLOT_DESTROY 0x04
#define Am_TRACE_OBJECT_CREATED 0x05
#define Am_TRACE_OBJECT_COPIED 0x06
#define Am_TRACE_OBJECT_DESTROYED 0x07
#define Am_TRACE_METHOD_GET 0x08
#define Am_TRACE_INHERITANCE_PROPAGATION 0x80

typedef void Am_Slot_Set_Trace_Proc(const Am_Slot &slot, Am_Set_Reason reason);
typedef void Am_Object_Create_Trace_Proc(const Am_Object &old_object,
                                         const Am_Object &new_object,
                                         Am_Set_Reason reason);
typedef void Am_Method_Get_Trace_Proc(const Am_Object &of_object,
                                      Am_Slot_Key slot_key,
                                      Am_Value method_value);

_OA_DL_IMPORT extern Am_Slot_Set_Trace_Proc *Am_Global_Slot_Trace_Proc;
_OA_DL_IMPORT extern Am_Object_Create_Trace_Proc *Am_Global_Object_Trace_Proc;
_OA_DL_IMPORT extern Am_Method_Get_Trace_Proc *Am_Global_Method_Get_Proc;

#endif
