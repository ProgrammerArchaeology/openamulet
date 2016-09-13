#ifndef TYPES_BASIC_H
#define TYPES_BASIC_H

// class std::ostream;

// Is used for Am_Null_Method_Error_Function as this function takes any
// number of parameters
typedef void Am_Any_Procedure(...);

// Am_ID_Tag is used to name classes derived from Am_Wrapper.
typedef unsigned short Am_ID_Tag;

// A helper function that generates Am_ID_Tag's for classes typically derived
// from Am_Wrapper.  Providing a base tag will OR that base onto the returned
// id.  Providing a base with an already defined subnumber will return that
// number.
_OA_DL_IMPORT extern Am_ID_Tag Am_Get_Unique_ID_Tag(const char *type_name,
                                                    Am_ID_Tag base = 0);
_OA_DL_IMPORT extern Am_ID_Tag Am_Set_ID_Class(const char *type_name,
                                               Am_ID_Tag base);

// These helper functions strip apart the various portions of an ID tag.
// The first returns the base number of the id.
_OA_DL_IMPORT inline Am_ID_Tag
Am_Type_Base(Am_ID_Tag in_id)
{
  return in_id & 0x0FFF;
}

// The second returns the class portion of the id.
_OA_DL_IMPORT inline Am_ID_Tag
Am_Type_Class(Am_ID_Tag in_id)
{
  return in_id & 0x7000;
}

// The third returns true iff the type is a reference-counted pointer.
_OA_DL_IMPORT inline bool
Am_Type_Is_Ref_Counted(Am_ID_Tag in_id)
{
  return (in_id & 0x8000) != 0;
}

#endif //TYPES_BASIC_H
