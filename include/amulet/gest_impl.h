/* ************************ -*-c++-*- ************************************* 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file contains the implementation classes for the gesture module.
   
   Designed and implemented by Rob Miller
*/

#ifndef GEST_IMPL_H
#define GEST_IMPL_H

#include <am_inc.h>

#include <amulet/am_io.h>
#include "amulet/impl/types_string.h" // for Am_String
#include <amulet/gesture.h>

#include "gest_matrix.h"

/* ---------- compile time settable parameters ----------  */
/* some of these can also be set at runtime, see fv.c */

#undef USE_TIME /* Define to enable the duration and maximum
				   velocity features.  When not defined, 0
				   may be passed as the time to FvAddPoint */

#define DIST_SQ_THRESHOLD                                                      \
  (3 * 3) /* points within sqrt(DIST_SQ_THRESHOLD)
				   will be ignored to eliminate mouse jitter */

#define SE_TH_ROLLOFF                                                          \
  (4 * 4) /* The SE_THETA features (cos and sin of angle
				   between first and last point) will be
				   be scaled down if the distance between the
				   points is less than sqrt(SE_TH_ROLLOFF) */

/* indices into the Vector returned by FvCalc */

#define PF_INIT_COS 0 /* initial angle (cos) */
#define PF_INIT_SIN 1 /* initial angle (sin) */
#define PF_BB_LEN 2   /* length of bounding box diagonal */
#define PF_BB_TH 3    /* angle of bounding box diagonal */
#define PF_SE_LEN 4   /* length between start and end points */
#define PF_SE_COS 5   /* cos of angle between start and end points */
#define PF_SE_SIN 6   /* sin of angle between start and end points */
#define PF_LEN 7      /* arc length of path */
#define PF_TH 8       /* total angle traversed */
#define PF_ATH 9      /* sum of abs vals of angles traversed */
#define PF_SQTH 10    /* sum of squares of angles traversed */

#ifndef USE_TIME
#define NFEATURES 11
#else
#define PF_DUR 11  /* duration of path */
#define PF_MAXV 12 /* maximum speed */
#define NFEATURES 13
#endif

// // // // // // // // // // // // // // // // // // // //
// Am_Feature_Vector_Data
// // // // // // // // // // // // // // // // // // // //

class _OA_DL_CLASSIMPORT Am_Feature_Vector_Data : public Am_Wrapper
{
  friend class Am_Feature_Vector;

  AM_WRAPPER_DATA_DECL(Am_Feature_Vector)
public:
  Am_Feature_Vector_Data(Am_Feature_Vector_Data *);
  Am_Feature_Vector_Data();

  ~Am_Feature_Vector_Data();

  bool operator==(Am_Feature_Vector_Data &) { return false; }

  bool operator==(Am_Feature_Vector_Data &) const { return false; }

  void Add_Point(int x, int y);
  void Calc();

protected:
  /* the example's points */
  Am_Point_List points;

  /* the following are used in calculating the features */

  double startx, starty; /* starting point */

  /* these are set after a few points and left */
  double initial_sin, initial_cos; /* initial angle to x axis */

  /* these are updated incrementatally upon every point */

  int npoints; /* number of points in path */

  double dx2, dy2; /* differences: endx-prevx
					   endy-prevy */
  double magsq2;   /* dx2*dx2 + dy2*dy2 */

  double endx, endy; /* last point added */

  double minx, maxx, miny, maxy; /* bounding box */

  double path_r, path_th; /* total length and rotation
					    (in rads) */
  double abs_th;          /* sum of absolute values of
					   path angles */
  double sharpness;       /* sum of squares of path angles */

  Vector y;        /* Actual feature vector */
  bool calculated; /* true if y is up-to-date wrt other feature
				   fields */
};

// // // // // // // // // // // // // // // // // // // //
// Am_Gesture_Classifier_Data
// // // // // // // // // // // // // // // // // // // //

class _OA_DL_CLASSIMPORT Am_Gesture_Classifier_Data : public Am_Wrapper
{
  friend class Am_Gesture_Classifier;
  friend class Am_Gesture_Trainer_Data;
  friend std::ostream &operator<<(std::ostream &s, Am_Gesture_Classifier &gc);
  friend std::istream &operator>>(std::istream &s, Am_Gesture_Classifier &gc);

  AM_WRAPPER_DATA_DECL(Am_Gesture_Classifier)
public:
  void Print(std::ostream &os) const; //print my contents on the stream

  Am_Gesture_Classifier_Data(Am_Gesture_Classifier_Data *);
  Am_Gesture_Classifier_Data();
  Am_Gesture_Classifier_Data(int nclasses);

  ~Am_Gesture_Classifier_Data();

  bool operator==(Am_Gesture_Classifier_Data &) { return false; }
  bool operator==(Am_Gesture_Classifier_Data &) const { return false; }

  Am_String Classify(Am_Feature_Vector fv, double *nap = 0, double *dist = 0);

protected:
  struct Gesture_Class
  {
    Am_String name;
    bool is_dot;    // true iff all examples had exactly one point
    Vector average; // mean feature vector of all examples
    Vector w;       // coefficient weights
    double cnst;    //  and constant term
    double disc;    // estimated likelihood for this class
                    // (temporary storage for Classify())
    Gesture_Class()
    {
      average = (0L);
      w = NULL;
    }
    ~Gesture_Class()
    {
      if (average) {
        FreeVector(average);
        average = (0L);
      }
      if (w) {
        FreeVector(w);
        w = (0L);
      }
    }
  };

  int nclasses;
  Gesture_Class *classes;

  bool trained;     // has classifier been trained?
  Matrix invavgcov; /* inverse covariance matrix */
};

// // // // // // // // // // // // // // // // // // // //
// Am_Gesture_Trainer_Data
// // // // // // // // // // // // // // // // // // // //

class _OA_DL_CLASSIMPORT Am_Gesture_Trainer_Data : public Am_Wrapper
{
  friend class Am_Gesture_Trainer;
  friend std::ostream &operator<<(std::ostream &s, Am_Gesture_Trainer &gc);
  friend std::istream &operator>>(std::istream &s, Am_Gesture_Trainer &gc);

  AM_WRAPPER_DATA_DECL(Am_Gesture_Trainer)
public:
  Am_Gesture_Trainer_Data(Am_Gesture_Trainer_Data *);
  Am_Gesture_Trainer_Data();
  Am_Gesture_Trainer_Data(Am_Gesture_Classifier_Data *cl);

  ~Am_Gesture_Trainer_Data();

  bool operator==(Am_Gesture_Trainer_Data &) { return false; }
  bool operator==(Am_Gesture_Trainer_Data &) const { return false; }

  struct Gesture_Class
  {
    Am_String name;
    Am_Value_List examples;
    Gesture_Class *next, *prev;

    Gesture_Class() {}
    Gesture_Class(Gesture_Class *d)
    {
      name = d->name;
      examples = d->examples;
    }
  };

  void Add_Class(Am_String classname);
  void Delete_Class(Gesture_Class *c);
  Gesture_Class *Find_Class(Am_String classname);

  Am_Gesture_Classifier Make_Classifier();
  Am_Gesture_Classifier Train();

protected:
  Gesture_Class *head, *tail;
  int nclasses;
  Am_Gesture_Classifier cached_classifier; // result returned by last Train()
};

#endif // GEST_IMPL_H
