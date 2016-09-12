/* ****************************** -*-c++-*- *******************************/
/* ************************************************************************ 
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file contains exported interface to the gesture-recognizer.
*/

#ifndef GESTURE_H
#define GESTURE_H

#include <am_inc.h>

#include AM_IO__H
#include GDEFS__H      // to get Am_Point_List
#include VALUE_LIST__H // to get Am_Value_List
#include OBJECT__H     // to get Am_Object

///////////////////////////////////////////////////////
/// Am_Feature_Vector: represents a gesture example
///

class Am_Feature_Vector_Data;

class _OA_DL_CLASSIMPORT Am_Feature_Vector
{
  AM_WRAPPER_DECL(Am_Feature_Vector)
public:
  Am_Feature_Vector(); // no points
  Am_Feature_Vector(Am_Point_List &pl);

  void Add_Point(int x, int y);

  Am_Point_List Points();
  bool
  IsDot(); // only one unique point (in a radius of DIST_SQ_THRESHOLD pixels, typically 3)
  double StartX();
  double StartY();
  double InitialSin();
  double InitialCos();
  double Dx2();
  double Dy2();
  double MagSq2();
  double EndX();
  double EndY();
  double MinX();
  double MaxX();
  double MinY();
  double MaxY();
  double PathR();
  double PathTh();
  double AbsTh();
  double Sharpness();
  double *Vector(); /* returns feature vector for classifier (as array
			    of doubles) */
};

///////////////////////////////////////////////////////
/// Am_Gesture_Classifier: recognizes gestures
///

class Am_Gesture_Classifier_Data;

class _OA_DL_CLASSIMPORT Am_Gesture_Classifier
{
  friend std::ostream &operator<<(std::ostream &s, Am_Gesture_Classifier &gc);
  friend std::istream &operator>>(std::istream &s, Am_Gesture_Classifier &gc);

  AM_WRAPPER_DECL(Am_Gesture_Classifier)
public:
  Am_Gesture_Classifier();                     // empty classifier
  Am_Gesture_Classifier(const char *filename); // read from file

  bool Trained();
  Am_String Classify(Am_Feature_Vector fv, double *nap = 0, double *dist = 0);
};

_OA_DL_MEMBERIMPORT std::ostream &operator<<(std::ostream &s,
                                             Am_Gesture_Classifier &gc);
_OA_DL_MEMBERIMPORT std::istream &operator>>(std::istream &s,
                                             Am_Gesture_Classifier &gc);

////////////////////////////////////////////////////////////////////////
/// Am_Gesture_Trainer:  generates a classifier from training examples
///

class Am_Gesture_Trainer_Data;

class _OA_DL_CLASSIMPORT Am_Gesture_Trainer
{
  friend std::ostream &operator<<(std::ostream &s, Am_Gesture_Trainer &gc);
  friend std::istream &operator>>(std::istream &s, Am_Gesture_Trainer &gc);

  AM_WRAPPER_DECL(Am_Gesture_Trainer)
public:
  Am_Gesture_Trainer();                     // empty Trainer
  Am_Gesture_Trainer(const char *filename); // read from file

  bool Add_Class(Am_String classname, bool unique = true);
  bool Delete_Class(Am_String classname, bool unique = true);
  bool Rename_Class(Am_String old_name, Am_String new_name, bool unique = true);
  Am_Value_List Get_Class_Names();

  bool Add_Example(Am_String classname, Am_Feature_Vector fv,
                   bool unique = true);
  bool Delete_Example(Am_String classname, Am_Feature_Vector fv,
                      bool unique = true);
  Am_Value_List Get_Examples(Am_String classname);

  Am_Gesture_Classifier Train();
};

std::ostream &operator<<(std::ostream &s, Am_Gesture_Trainer &gc);
std::istream &operator>>(std::istream &s, Am_Gesture_Trainer &gc);

/// Global initialization routine

void Am_Gesture_Initialize(void);

//The following are commands that might be put into the items slot of
//a gesture interactor

//just prints out ** UNRECOGNIZED to cout and beeps
_OA_DL_IMPORT extern Am_Object Am_Gesture_Unrecognized_Command;

//command object to create new objects in an undoable fashion.
//You should add a Am_Create_New_Object_Method into the
//Am_CREATE_NEW_OBJECT_METHOD slot of this command.  Also set Am_AS_LINE.
// The Am_VALUE of the command is the gesture string.  The
// inter_location will be the bounding box of the gesture if not
// as-line, and the first and last points of the gesture if as-line.
// The "inter" parameter to the Am_CREATE_NEW_OBJECT_METHOD will
// actually be the Am_Gesture_Create_Command instance instead of the
// gesture interactor.  (Gesture inter can be gotten from its
// Am_SAVED_OLD_OWNER slot.)  old_object is always Am_No_Object.
_OA_DL_IMPORT extern Am_Object Am_Gesture_Create_Command;

//Selects the objects in the bounding box of the gesture, and then
//calls the Do method of the command in the Am_SUB_COMMAND slot
// (like an instance of Am_Cut_Command).
// Set the selection widget into the Am_SELECTION_WIDGET slot of the
// command, or of the gesture interactor.  Gets the group object that
// should look for parts of from the selection widget
_OA_DL_IMPORT extern Am_Object Am_Gesture_Select_And_Do_Command;
#endif
