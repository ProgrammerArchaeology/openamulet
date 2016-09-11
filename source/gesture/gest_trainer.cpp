/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file contains Am_Gesture_Trainer, which uses a maximum-likelihood
   estimator to identify a gesture represented by an Am_Feature_Vector. 
   
   Designed in C by Dean Rubine;  ported to C++ by Rob Miller.
*/

#include <am_inc.h>

#include AM_IO__H
#include GESTURE__H

#include <fstream>
#include <math.h>

#ifdef SHORT_NAMES
#include "amulet/gest_imp.h"
#else
#include "amulet/gest_impl.h"
#endif				   

#define	EPS	(1.0e-6)	/* for singular matrix check */

static bool fix_classifier (Matrix avgcov, Matrix invavgcov);

// // // // // // // // // // // // // // // // // // // //
// Am_Gesture_Trainer
// // // // // // // // // // // // // // // // // // // //

Am_WRAPPER_IMPL (Am_Gesture_Trainer)

//////
// Am_Gesture_Trainer constructors
//////

Am_Gesture_Trainer::Am_Gesture_Trainer ()
{
  data = new Am_Gesture_Trainer_Data ();
}

Am_Gesture_Trainer::Am_Gesture_Trainer (const char *filename)
{
  data = (0L);
  
  std::ifstream in(filename, std::ios::in);
  if (!in) {
   std::cerr << "can't open gesture classifier file `" << filename << "'" <<std::endl;
  }
  else {
    in >> *this;
  }
}

bool Am_Gesture_Trainer::Add_Class (Am_String classname, bool unique)
{
  if (!data)
    data = new Am_Gesture_Trainer_Data();
  else if (unique)
    data = (Am_Gesture_Trainer_Data*)data->Make_Unique ();

  if (data->Find_Class (classname))
    return false; // class already exists

  data->Add_Class (classname);

  data->cached_classifier = 0;
  return true;
}

bool Am_Gesture_Trainer::Delete_Class (Am_String classname, bool unique)
{
  if (!data)
    data = new Am_Gesture_Trainer_Data();
  else if (unique)
    data = (Am_Gesture_Trainer_Data*)data->Make_Unique ();

  Am_Gesture_Trainer_Data::Gesture_Class *c;

  if ((c = data->Find_Class (classname)) == (0L))
    return false;

  data->Delete_Class (c);
  data->cached_classifier = 0;
  return true;
}

bool Am_Gesture_Trainer::Rename_Class (Am_String old_name, Am_String new_name,
				       bool unique)
{
  if (!data)
    data = new Am_Gesture_Trainer_Data();
  else if (unique)
    data = (Am_Gesture_Trainer_Data*)data->Make_Unique ();

  if (data->Find_Class(new_name) != (0L))
    return false;

  Am_Gesture_Trainer_Data::Gesture_Class *c;
  if ((c = data->Find_Class (old_name)) == (0L))
    return false;

  c->name = new_name;
  data->cached_classifier = 0;
  return true;
}

Am_Value_List Am_Gesture_Trainer::Get_Class_Names ()
{
  Am_Gesture_Trainer_Data::Gesture_Class *c;
  Am_Value_List l;

  if (!data)
    return l;

  for (c = data->head; c != (0L); c = c->next)
    l.Add (c->name);

  return l;
}

bool Am_Gesture_Trainer::Add_Example (Am_String classname, 
					 Am_Feature_Vector fv,
					 bool unique)
{
  if (!data)
    data = new Am_Gesture_Trainer_Data();
  else if (unique)
    data = (Am_Gesture_Trainer_Data*)data->Make_Unique ();

  Am_Gesture_Trainer_Data::Gesture_Class *c = data->Find_Class (classname);
  if (!c)
    return false;

  c->examples.Add(fv);
  
  data->cached_classifier = 0;
  return true;
}

bool Am_Gesture_Trainer::Delete_Example (Am_String classname, 
					    Am_Feature_Vector fv,
					    bool unique)
{
  if (!data)
    data = new Am_Gesture_Trainer_Data();
  else if (unique)
    data = (Am_Gesture_Trainer_Data*)data->Make_Unique ();

  Am_Gesture_Trainer_Data::Gesture_Class *c = data->Find_Class (classname);
  if (!c)
    return false;

  Am_Value_List &l = c->examples;

  l.Start ();
  if (!l.Member(fv))
    return false;

  do
    l.Delete ();
  while (l.Member(fv));

  data->cached_classifier = 0;
  return true;
}

Am_Value_List Am_Gesture_Trainer::Get_Examples (Am_String classname)
{
  if (data) {
    Am_Gesture_Trainer_Data::Gesture_Class *c = data->Find_Class (classname);
    if (c)
      return c->examples;
  }

  // else...
  return Am_No_Value;
}

Am_Gesture_Classifier Am_Gesture_Trainer::Train ()
{
  if (!data)
    // empty classifier
    return Am_Gesture_Classifier();

  return data->Train ();
}

std::ostream&  operator<< (std::ostream& s, Am_Gesture_Trainer& tr)
{
  Am_Gesture_Trainer_Data *trdata = tr.data;

  // Write out the trainer's classifier 
  Am_Gesture_Classifier c = tr.Train ();
  s << c;

  // Write out examples

  Am_Gesture_Trainer_Data::Gesture_Class *cls;
  for (cls = trdata->head; cls!=(0L); cls = cls->next) {
    Am_Value_List l = cls->examples;
    for (l.Start(); !l.Last(); l.Next()) {
      Am_Feature_Vector fv = l.Get();
      Am_Point_List pl = fv.Points ();
      int x, y;

      s << "x" << cls->name <<std::endl;
      for (pl.Start(); !pl.Last(); pl.Next()) {
	pl.Get(x,y);
	s << ' ' << x << ' ' << y;
      }
      s <<std::endl;
    }
  }
  
  return s;
}

std::istream&  operator>> (std::istream& s, Am_Gesture_Trainer& tr)
{
  // First read in a classifier (or an untrained classifier containing only
  // class names)
  Am_Gesture_Classifier cl;

  if (!(s >> cl))
    return s;

  Am_Gesture_Trainer_Data *trdata;
  Am_Gesture_Trainer_Data::Gesture_Class *cls = 0;
  char check;
  char buf[100];
  int x, y;
    
  // construct a new trainer by copying the classes in cl
  trdata = new Am_Gesture_Trainer_Data ((Am_Gesture_Classifier_Data*)
					(Am_Wrapper*)cl);

  if (cl.Trained ())
    trdata->cached_classifier = cl;
  else
    trdata->cached_classifier = 0;
  
  // Now look for examples.  Examples have the check character 'x'.
  
  if ((s >> check) && check == 'x') {
    //Am_TRACE("Reading training examples\n");
    
    do {
      Am_Feature_Vector fv;
      
      // get the class name of this example
      s.get (buf, sizeof(buf));
      if (!s ||(cls = trdata->Find_Class (buf)) == (0L))
	goto LFail;
      
      // accumulate the points from this example
      while (s >> x >> y)
	fv.Add_Point (x, y);
      s.clear(std::ios::goodbit);

      cls->examples.Add(fv);
      
    } while ((s >> check) && check == 'x');
  }

  if (s) 
    s.putback(check);

  // return the new trainer
  if (tr.data)
    tr.data->Release();
  tr.data = trdata;

  s.clear (std::ios::goodbit);
  return s;

 LFail:
  static const char *bad_format = "unrecognized gesture classifier format";
 std::cerr << bad_format <<std::endl;
  s.clear(std::ios::badbit);
  return s;
}

// // // // // // // // // // // // // // // // // // // //
// Am_Gesture_Trainer_Data
// // // // // // // // // // // // // // // // // // // //

Am_WRAPPER_DATA_IMPL (Am_Gesture_Trainer, (this))

Am_Gesture_Trainer_Data::Am_Gesture_Trainer_Data ()
{
  head = tail = (0L);
  nclasses = 0;
  cached_classifier = 0;
}

Am_Gesture_Trainer_Data::Am_Gesture_Trainer_Data (Am_Gesture_Trainer_Data *d)
{
  if (d->head == (0L)) 
    head = tail = (0L);
  else {
    Gesture_Class *from, *to, *last;

    head = new Gesture_Class (d->head);
    head->prev = (0L);
    for (last = head, from = d->head->next; 
	   from != (0L); 
	      last = to, from = from->next) {
      to = new Gesture_Class (from);
      last->next = to;
      to->prev = last;
    }
    last->next = (0L);
    tail = last;
  }

  nclasses = d->nclasses;
  cached_classifier = d->cached_classifier;
}

Am_Gesture_Trainer_Data::Am_Gesture_Trainer_Data (Am_Gesture_Classifier_Data *cldata)
{
  int i;

  if (cldata == (0L) || cldata->nclasses == 0) {
    head = tail = (0L);
    nclasses = 0;
  }
  else {
    Am_Gesture_Classifier_Data::Gesture_Class * classes = cldata->classes;
    Gesture_Class *curr, *last;

    nclasses = cldata->nclasses;
    
    head = new Gesture_Class ();
    head->name = classes[0].name;
    head->prev = (0L);
    for (last=head, i=1; i<nclasses; last=curr, ++i) {
      curr = new Gesture_Class ();
      curr->name = classes[i].name;
      curr->prev = last;
      last->next = curr;
    }
    last->next = (0L);
    tail = last;
  }

  cached_classifier = cldata;
}

Am_Gesture_Trainer_Data::~Am_Gesture_Trainer_Data ()
{
  Gesture_Class *c, *next;

  for (c = head; c != (0L); c = next) {
    next = c->next;
    delete c;
  }
  head = tail = (0L);
}

void  Am_Gesture_Trainer_Data::Add_Class(Am_String classname)
{
  Gesture_Class *c = new Gesture_Class;
  c->name = classname;

  c->prev = tail;
  c->next = (0L);
  if (!tail)
    head = tail = c;
  else tail->next = c;
  tail = c;

  ++nclasses;
}

void Am_Gesture_Trainer_Data::Delete_Class(Gesture_Class *c)
{
  if (c->next)
    c->next->prev = c->prev;
  else tail = c->prev;
  
  if (c->prev)
    c->prev->next = c->next;
  else head = c->next;

  --nclasses;
}

Am_Gesture_Trainer_Data::Gesture_Class * Am_Gesture_Trainer_Data::Find_Class(Am_String classname)
{
  Gesture_Class *c;

  for (c=head; c!=(0L); c=c->next)
    if (c->name == classname) 
      return c;

  return (0L);
}

Am_Gesture_Classifier Am_Gesture_Trainer_Data::Train ()
{
  int i, j;
  int ne, denom;
  double oneoverdenom;
  Matrix avgcov;
  double det;
  Gesture_Class * c;
  Am_Gesture_Classifier_Data::Gesture_Class * d, *dotclass;
  int nexamples;
  double nfv[NFEATURES];

  if (cached_classifier.Valid())
    return cached_classifier;

  if (nclasses == 0)
    // return an empty classifier
    return Am_Gesture_Classifier();

  // make a new classifier with the class names in it
  Am_Gesture_Classifier_Data *cldata = 
    new Am_Gesture_Classifier_Data(nclasses);
  Am_Gesture_Classifier cl = cldata;

  for (c=head, d=cldata->classes; c!=(0L); c=c->next, ++d)
    d->name = c->name;

  
  /* Given covariance matrices for each class (* number of examples - 1)
     compute the average (common) covariance matrix */
  
  avgcov = NewMatrix(NFEATURES, NFEATURES);
  ZeroMatrix(avgcov);
  ne = 0;

  dotclass = (0L);

  for (c=head, d=cldata->classes; c!=(0L); c=c->next, ++d) {
    Am_Value_List &l = c->examples;

    nexamples = l.Length();
    ne += nexamples;

    ////// compute average feature vector

    d->average = NewVector(NFEATURES);
    ZeroVector (d->average);

    for (l.Start(); !l.Last(); l.Next()) {
      Am_Feature_Vector fv = l.Get();
      Vector y = fv.Vector();
      for(i = 0; i < NFEATURES; i++)
	d->average[i] += y[i];
    }
      
    if (nexamples == 0) {
      d->is_dot = false;
    }
    else {
      for(i = 0; i < NFEATURES; i++)
	d->average[i] /= nexamples;

      /////// update average covariance matrix and check for dot class
      
      d->is_dot = true;

      for (l.Start(); !l.Last(); l.Next()) {
	Am_Feature_Vector fv = l.Get();
	Vector y = fv.Vector();
	
	if (!fv.IsDot())
	  d->is_dot = false;

	for(i = 0; i < NFEATURES; i++)
	  nfv[i] = y[i] - d->average[i];
	
	/* only upper triangular part computed */
	for(i = 0; i < NFEATURES; i++)
	  for(j = i; j < NFEATURES; j++)
	    avgcov[i][j] += nfv[i] * nfv[j];
      }
      
      if (d->is_dot) {
	if (dotclass == (0L)) {
	  dotclass = d;
	  // std::cout << d->name << " is dot-like" <<std::endl;
	}
	else {
	 std::cerr << "Cannot discriminate the dot-like gestures " 
	       << d->name << " and " << dotclass->name <<std::endl;
	  goto LFail;
	}
      }
    }
  }
  
  denom = ne - nclasses;
  if(denom <= 0) {
    //printf("no examples, denom=%d\n", denom);
    goto LFail;
  }
  
  oneoverdenom = 1.0 / denom;
  for(i = 0; i < NFEATURES; i++)
    for(j = i; j < NFEATURES; j++)
      avgcov[j][i] = avgcov[i][j] *= oneoverdenom;
  
  // PrintMatrix(avgcov, "Average Covariance Matrix\n");

  /* invert the avg covariance matrix */
  
  cldata->invavgcov = NewMatrix(NFEATURES, NFEATURES);
  det = InvertMatrix(avgcov, cldata->invavgcov);
  if(fabs(det) <= EPS
     && !fix_classifier (avgcov, cldata->invavgcov))
    goto LFail;
  
  /* now compute discrimination function (w and cnst) for each class */
  
  for (c=head, d=cldata->classes; c!=(0L); c=c->next, ++d) {
    d->w = NewVector(NFEATURES);
    VectorTimesMatrix(d->average, cldata->invavgcov, d->w);
    d->cnst = -0.5 * InnerProduct(d->w, d->average);
    /* could add log(priorprob class c) to d->cnst */
  }
  
  FreeMatrix(avgcov);

  cldata->trained = true;
  cached_classifier = cl;
  return cl;

 LFail:
  FreeMatrix (avgcov);
  return cl;
}

static bool
fix_classifier (Matrix avgcov, Matrix invavgcov)
{
  int i;
  double det;
  unsigned mask;
  Matrix m, r;
  
  /* just add the features one by one, discarding any that cause
     the matrix to be non-invertible */
  
  mask = 0;
  for(i = 0; i < NFEATURES; i++) {
    mask |= (1<<i);
    m = SliceMatrix(avgcov, mask, mask);
    r = NewMatrix(NROWS(m), NCOLS(m));
    det = InvertMatrix(m, r);
    if(fabs(det) <= EPS)
      mask &= ~(1<<i);
    FreeMatrix(m);
    FreeMatrix(r);
  }
  
  m = SliceMatrix(avgcov, mask, mask);
  r = NewMatrix(NROWS(m), NCOLS(m));
  det = InvertMatrix(m, r);
  if(fabs(det) <= EPS)
    return false;
  DeSliceMatrix(r, 0.0, mask, mask, invavgcov);
  
  FreeMatrix(m);
  FreeMatrix(r);
  return true;
}
