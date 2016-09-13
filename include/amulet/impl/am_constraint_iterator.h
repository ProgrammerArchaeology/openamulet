#ifndef OBJECT_CONSTR_ITER_H
#define OBJECT_CONSTR_ITER_H

class Am_Slot;

class Am_Constraint_Iterator_Data;

class _OA_DL_CLASSIMPORT Am_Constraint_Iterator
{
public:
  Am_Constraint_Iterator();
  Am_Constraint_Iterator(const Am_Slot &slot);

  Am_Constraint_Iterator &operator=(const Am_Slot &slot);

  unsigned short Length() const;
  void Start();
  void Next();
  bool Last() const;
  Am_Constraint *Get() const;
  Am_Constraint_Tag Get_Tag() const;

private:
  Am_Slot context;
  Am_Constraint_Iterator_Data *current;
};

#endif // OBJECT_CONSTR_ITER_H
