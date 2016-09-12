/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file contains Am_Gesture_Classifier, which uses a maximum-likelihood
   estimator to identify a gesture represented by an Am_Feature_Vector. 
   
   Designed in C by Dean Rubine;  ported to C++ by Rob Miller.
*/

#include <am_inc.h>

#include AM_IO__H
#include GESTURE__H
#include <string.h>

#include <fstream>
#include <math.h>

#include "amulet/gest_impl.h"

static double MahalanobisDistance(register Vector v, register Vector u,
                                  register Matrix sigma);

// // // // // // // // // // // // // // // // // // // //
// Am_Gesture_Classifier
// // // // // // // // // // // // // // // // // // // //

AM_WRAPPER_IMPL(Am_Gesture_Classifier)

//////
// Am_Gesture_Classifier constructors
//////

Am_Gesture_Classifier::Am_Gesture_Classifier()
{
  data = new Am_Gesture_Classifier_Data();
}

Am_Gesture_Classifier::Am_Gesture_Classifier(const char *filename)
{
  data = (0L);

  std::ifstream in(filename, std::ios::in);
  if (!in) {
    std::cerr << "can't open gesture classifier file `" << filename << "'"
              << std::endl;
  } else {
    in >> *this;
  }
}

bool
Am_Gesture_Classifier::Trained()
{
  return (data && data->trained);
}

Am_String
Am_Gesture_Classifier::Classify(Am_Feature_Vector fv, double *nap, double *dist)
{
  if (data)
    return data->Classify(fv, nap, dist);
  else
    return Am_No_String;
}

// // // // // // // // // // // // // // // // // // // //
// I/O for Am_Gesture_Classifier and Am_Gesture_Trainer
// // // // // // // // // // // // // // // // // // // //

static const char *version_messages[] = {
#define NO_DOT_VERSION 0
    "Amulet Gesture Classifier 1.0",

#define CURRENT_VERSION 1
    "Amulet Gesture Classifier 1.1",

    (0L)};

std::ostream &
operator<<(std::ostream &s, Am_Gesture_Classifier &cl)
{
  Am_Gesture_Classifier_Data *cldata = cl.data;
  Am_Gesture_Classifier_Data::Gesture_Class *classes = cldata->classes;
  int n = cldata->nclasses;
  int i;

  s << version_messages[CURRENT_VERSION] << std::endl;
  s << n << " classes" << std::endl;
  for (i = 0; i < n; i++)
    s << classes[i].name << std::endl;

  if (cl.Trained()) {
    for (i = 0; i < n; i++) {
      OutputVectorCPP(s, classes[i].average);
      OutputVectorCPP(s, classes[i].w);
    }

    Vector v = NewVector(n);

    // output constants for each class
    for (i = 0; i < n; i++)
      v[i] = classes[i].cnst;
    OutputVectorCPP(s, v);

    // output is_dot fields for each class
    for (i = 0; i < n; ++i)
      v[i] = (double)classes[i].is_dot;
    OutputVectorCPP(s, v);

    FreeVector(v);

    OutputMatrixCPP(s, cldata->invavgcov);
  }

  return s;
}

std::istream &
operator>>(std::istream &s, Am_Gesture_Classifier &cl)
{
  int n;
  char buf[100];
  Am_Gesture_Classifier_Data *cldata = 0;
  Am_Gesture_Classifier_Data::Gesture_Class *classes;
  int ver;
  int i;
  char check;

  // First check gesture version

  s.get(buf, sizeof(buf), '\n');
  if (!s.get(check) || check != '\n')
    goto lFail;

  for (ver = 0;; ++ver)
    if (!version_messages[ver])
      goto lFail;
    else if (!strcmp(buf, version_messages[ver]))
      break;

  // Classifier and trainer formats both begin with a class list.  E.g.:
  //
  //   3 classes
  //   cut
  //   copy
  //   paste

  s >> n;
  s.get(buf, sizeof(buf), '\n');

  if (!s.get(check) || check != '\n' || strcmp(buf, " classes"))
    goto lFail;

  // construct a new classifer with n classes
  cldata = new Am_Gesture_Classifier_Data(n);
  classes = cldata->classes;

  //AM_TRACE(n << " classes " <<std::flush);
  for (i = 0; i < n; i++) {
    s.get(buf, sizeof(buf));
    if (!s.get(check) || check != '\n')
      // class name too long
      goto lFail;
    classes[i].name = buf;
    //AM_TRACE(buf << " " <<std::flush);
  }
  //AM_TRACE(endl);

  // Now look for weights, constants, covariance matrix.
  // Weight vector begins with 'V'.  If we don't find it, assume
  // we have an untrained classifier (i.e., just a list of classes).

  if (s >> check && check == 'V') {
    s.putback(check);

    for (i = 0; i < n; i++) {
      classes[i].average = InputVectorCPP(s);
      classes[i].w = InputVectorCPP(s);
    }

    Vector v = InputVectorCPP(s);
    for (i = 0; i < n; i++)
      classes[i].cnst = v[i];
    FreeVector(v);

    if (ver > NO_DOT_VERSION) {
      Vector v = InputVectorCPP(s);
      // read in is_dot fields for each class
      for (i = 0; i < n; ++i)
        classes[i].is_dot = (v[i] != 0.0);
      FreeVector(v);
    } else {
      // initialize all is_dot fields
      for (i = 0; i < n; ++i)
        classes[i].is_dot = false;
    }

    cldata->invavgcov = InputMatrixCPP(s);

    cldata->trained = true;
  } else if (s)
    s.putback(check);
  else
    s.clear(std::ios::goodbit);

  if (!s)
    goto lFail;

  if (cl.data)
    cl.data->Release();
  cl.data = cldata;
  return s;

lFail:
  std::cerr << "unrecognized gesture classifier format" << std::endl;
  s.clear(std::ios::badbit);
  return s;
}

// // // // // // // // // // // // // // // // // // // //
// Am_Gesture_Classifier_Data
// // // // // // // // // // // // // // // // // // // //

AM_WRAPPER_DATA_IMPL(Am_Gesture_Classifier, (this))

Am_Gesture_Classifier_Data::Am_Gesture_Classifier_Data()
{
  nclasses = 0;
  classes = (0L);
  trained = false;
  invavgcov = (0L);
}

void
Am_Gesture_Classifier_Data::Print(std::ostream &os) const
{
  os << "Gestures[" << nclasses << "](";
  int i;
  for (i = 0; i < nclasses; i++) {
    os << classes[i].name;
    if (i < nclasses - 1)
      os << " ";
  }
  os << ")";
}

Am_Gesture_Classifier_Data::Am_Gesture_Classifier_Data(int nc)
{
  nclasses = nc;
  classes = new Gesture_Class[nc];
  trained = false;
  invavgcov = (0L);
}

Am_Gesture_Classifier_Data::Am_Gesture_Classifier_Data(
    Am_Gesture_Classifier_Data *d)
{
  register int i;
  register Gesture_Class *classTo, *classFrom;

  nclasses = d->nclasses;
  classes = new Gesture_Class[nclasses];

  for (i = 0; i < nclasses; i++) {
    classTo = &classes[i];
    classFrom = &d->classes[i];

    classTo->name = classFrom->name;
    classTo->average =
        classFrom->average ? VectorCopy(classFrom->average) : (Vector)(0L);
    classTo->w = classFrom->w ? VectorCopy(classFrom->w) : (Vector)(0L);
    classTo->cnst = classFrom->cnst;
    classTo->disc = classFrom->disc;
  }

  trained = d->trained;
  invavgcov = d->invavgcov ? MatrixCopy(d->invavgcov) : (Matrix)(0L);
}

Am_Gesture_Classifier_Data::~Am_Gesture_Classifier_Data()
{
  delete[] classes;

  if (invavgcov) {
    FreeMatrix(invavgcov);
    invavgcov = (0L);
  }
}

Am_String
Am_Gesture_Classifier_Data::Classify(Am_Feature_Vector feat, double *ap,
                                     double *dp)
{
  register int i, maxclass;
  double denom;
  double d;

  if (nclasses == 0 || !trained)
    return Am_No_String;

  if (feat.IsDot()) {
    // gesture is just a dot
    for (i = 0; i < nclasses; i++)
      if (classes[i].is_dot) {
        // found a "dot" class
        if (ap)
          *ap = 1.0;
        if (dp)
          *dp = 0.0;
        return classes[i].name;
      }

    // none of the classes are "dot"
    return Am_No_String;
  }

  // compute likelihood for each class
  Vector fv = feat.Vector();
  for (i = 0; i < nclasses; i++)
    classes[i].disc = InnerProduct(classes[i].w, fv) + classes[i].cnst;

  // find non-dot class with maximum likelihood
  for (maxclass = 0; classes[maxclass].is_dot; ++maxclass)
    if (maxclass == nclasses)
      return Am_No_String;
  for (i = 1; i < nclasses; i++)
    if (classes[i].disc > classes[maxclass].disc && !classes[i].is_dot)
      maxclass = i;

  // calculate accuracy measurements (if requested)

  if (ap) { /* calculate probability of non-ambiguity */
    for (denom = 0, i = 0; i < nclasses; i++)
      /* quick check to avoid computing negligible term */
      if ((d = classes[i].disc - classes[maxclass].disc) > -7.0)
        denom += exp(d);
    *ap = 1.0 / denom;
  }

  if (dp) /* calculate distance to mean of chosen class */
    *dp = MahalanobisDistance(fv, classes[maxclass].average, invavgcov);

  return classes[maxclass].name;
}

/*
 Compute (v-u)' sigma (v-u)
 */

static double
MahalanobisDistance(register Vector v, register Vector u, register Matrix sigma)
{
  register int i;
  static Vector space;
  double result;

  if (space == (0L) || NROWS(space) != NROWS(v)) {
    if (space)
      FreeVector(space);
    space = NewVector(NROWS(v));
  }
  for (i = 0; i < NROWS(v); i++)
    space[i] = v[i] - u[i];
  result = QuadraticForm(space, sigma);
  return result;
}
