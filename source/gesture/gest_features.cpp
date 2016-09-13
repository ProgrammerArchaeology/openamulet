/* ************************************************************************
 *         The Amulet User Interface Development Environment              *
 * ************************************************************************
 * This code was written as part of the Amulet project at                 *
 * Carnegie Mellon University, and has been placed in the public          *
 * domain.  If you are using this code or any part of Amulet,             *
 * please contact amulet@cs.cmu.edu to be put on the mailing list.        *
 * ************************************************************************/

/* This file contains Am_Feature_Vector, which computes the interesting
   features of a gesture trace (represented by Am_Point_List).

   Designed in C by Dean Rubine;  ported to C++ by Rob Miller.
*/

#include <am_inc.h>

#include <amulet/am_io.h>
#include <amulet/gesture.h>

#include <math.h>
#include "amulet/gest_impl.h"

/* runtime settable parameters */
double dist_sq_threshold = DIST_SQ_THRESHOLD;
double se_th_rolloff = SE_TH_ROLLOFF;

#define EPS (1.0e-4)

// A hack as there are some problems with this function sometimes
// we use _hypot even we shouldn't but hypot is	a defined function
// in math.h on unix
#undef _hypot
inline double
_hypot(double x, double y)
{
  return sqrt(x * x + y * y);
}

// // // // // // // // // // // // // // // // // // // //
// Am_Feature_Vector
// // // // // // // // // // // // // // // // // // // //

AM_WRAPPER_IMPL(Am_Feature_Vector)

Am_Feature_Vector::Am_Feature_Vector() { data = new Am_Feature_Vector_Data(); }

Am_Feature_Vector::Am_Feature_Vector(Am_Point_List &pl)
{
  data = new Am_Feature_Vector_Data();
  int x, y;

  for (pl.Start(); !pl.Last(); pl.Next()) {
    pl.Get(x, y);
    data->Add_Point(x, y);
  }
  data->points = pl;
}

void
Am_Feature_Vector::Add_Point(int x, int y)
{
  data->Add_Point(x, y);
  data->points.Add((float)x, (float)y);
}

Am_Point_List
Am_Feature_Vector::Points()
{
  return data->points;
}

bool
Am_Feature_Vector::IsDot()
{
  return data->npoints == 1;
}

double
Am_Feature_Vector::StartX()
{
  return data->startx;
}

double
Am_Feature_Vector::StartY()
{
  return data->starty;
}

double
Am_Feature_Vector::InitialSin()
{
  return data->initial_sin;
}

double
Am_Feature_Vector::InitialCos()
{
  return data->initial_cos;
}

double
Am_Feature_Vector::Dx2()
{
  return data->dx2;
}

double
Am_Feature_Vector::Dy2()
{
  return data->dy2;
}

double
Am_Feature_Vector::MagSq2()
{
  return data->magsq2;
}

double
Am_Feature_Vector::EndX()
{
  return data->endx;
}

double
Am_Feature_Vector::EndY()
{
  return data->endy;
}

double
Am_Feature_Vector::MinX()
{
  return data->minx;
}

double
Am_Feature_Vector::MaxX()
{
  return data->maxx;
}

double
Am_Feature_Vector::MinY()
{
  return data->miny;
}

double
Am_Feature_Vector::MaxY()
{
  return data->maxy;
}

double
Am_Feature_Vector::PathR()
{
  return data->path_r;
}

double
Am_Feature_Vector::PathTh()
{
  return data->path_th;
}

double
Am_Feature_Vector::AbsTh()
{
  return data->abs_th;
}

double
Am_Feature_Vector::Sharpness()
{
  return data->sharpness;
}

double *
Am_Feature_Vector::Vector()
{
  if (!data->calculated)
    data->Calc();
  return data->y;
}

// // // // // // // // // // // // // // // // // // // //
// Am_Feature_Vector_Data
// // // // // // // // // // // // // // // // // // // //

AM_WRAPPER_DATA_IMPL(Am_Feature_Vector, (this))

Am_Feature_Vector_Data::Am_Feature_Vector_Data()
{
  int i;

  y = NewVector(NFEATURES);

  npoints = 0;
  initial_sin = initial_cos = 0.0;
  path_r = 0;
  path_th = 0;
  abs_th = 0;
  sharpness = 0;
  for (i = 0; i < NFEATURES; i++)
    y[i] = 0.0;
  calculated = false;
}

Am_Feature_Vector_Data::Am_Feature_Vector_Data(Am_Feature_Vector_Data *d)
{
  int i;

  y = NewVector(NFEATURES);

  npoints = d->npoints;
  startx = d->startx;
  starty = d->starty;
  initial_sin = d->initial_sin;
  initial_cos = d->initial_cos;
  npoints = d->npoints;
  dx2 = d->dx2;
  dy2 = d->dy2;
  magsq2 = d->magsq2;
  endx = d->endx;
  endy = d->endy;
  minx = d->minx;
  miny = d->miny;
  maxx = d->maxx;
  maxy = d->maxy;
  path_r = d->path_r;
  path_th = d->path_th;
  abs_th = d->abs_th;
  sharpness = d->sharpness;

  for (i = 0; i < NFEATURES; i++)
    y[i] = d->y[i];
  calculated = d->calculated;
}

Am_Feature_Vector_Data::~Am_Feature_Vector_Data()
{
  FreeVector(y);
  y = (0L);
}

void
Am_Feature_Vector_Data::Add_Point(int x, int y)
{
  double dx1, dy1, magsq1;
  double th, absth, d;

  if (++npoints == 1) {
    startx = endx = minx = maxx = x;
    starty = endy = miny = maxy = y;
    endx = x;
    endy = y;
    return;
  }

  dx1 = x - endx;
  dy1 = y - endy;
  magsq1 = dx1 * dx1 + dy1 * dy1;

  if (magsq1 <= dist_sq_threshold) {
    npoints--;
    return; /* ignore this point */
  }

  if (x < minx)
    minx = x;
  if (x > maxx)
    maxx = x;
  if (y < miny)
    miny = y;
  if (y > maxy)
    maxy = y;

  d = sqrt(magsq1);
  path_r += d;

  /* calculate initial theta */
  /*if(npoints == 2 || npoints == 3) {*/
  /*if(npoints == 3) {	debug*/
  /*if(path_r >= kludge['r'] && path_r <= kludge['R']) {*/
  /*if(npoints <= 3 || path_r <= kludge['R']) {*/
  /*if(npoints == 3 ||
	  (npoints > 3 && path_r <= kludge['r'])) {*/
  if (npoints == 3) {
    double magsq, dx, dy, recip;
    dx = x - startx;
    dy = y - starty;
    magsq = dx * dx + dy * dy;
    if (magsq > dist_sq_threshold) {
      /* find angle w.r.t. positive x axis e.g. (1,0) */
      recip = 1 / sqrt(magsq);
      initial_cos = dx * recip;
      initial_sin = dy * recip;
    }
  }

  if (npoints >= 3) {
#if 0 // debugging
		Z(T) {
			dot = dx1 * dx2 + dy1 * dy2;
			cos2 = dot*dot / (magsq1 * magsq2);
			th = EvalFuncTab(acos2, dot < 0 ? -cos2 : cos2);
			abs_th += th;
			sharpness += th*th;
			if(dx1 * dy2 < dx2 * dy1) /* sin th < 0 ? */
				th = -th;
			path_th += th;
		}
		else
#endif
    {
      th = absth = atan2(dx1 * dy2 - dx2 * dy1, dx1 * dx2 + dy1 * dy2);
      if (absth < 0)
        absth = -absth;
      path_th += th;
      abs_th += absth;
      sharpness += th * th;
    }

#ifdef PF_MAXV
    if (endtime > lasttime && (v = d / (endtime - lasttime)) > maxv)
      maxv = v;
#endif
  }

  endx = x;
  endy = y;
  dx2 = dx1;
  dy2 = dy1;
  magsq2 = magsq1;
}

void
Am_Feature_Vector_Data::Calc()
{
  double bblen, selen, factor;

  if (npoints < 1) {
    calculated = true;
    return;
  }

  y[PF_INIT_COS] = initial_cos;
  y[PF_INIT_SIN] = initial_sin;

  bblen = _hypot(maxx - minx, maxy - miny);

  y[PF_BB_LEN] = bblen;

  if (bblen * bblen > dist_sq_threshold)
    y[PF_BB_TH] = atan2(maxy - miny, maxx - minx);

  selen = _hypot(endx - startx, endy - starty);
  y[PF_SE_LEN] = selen;

#if 0 // debugging
	Z(R) factor = selen * selen / 10;
	else
#endif
  factor = selen * selen / se_th_rolloff;
  if (factor > 1.0)
    factor = 1.0;
  factor = selen > EPS ? factor / selen : 0.0;
  y[PF_SE_COS] = (endx - startx) * factor;
  y[PF_SE_SIN] = (endy - starty) * factor;

  y[PF_LEN] = path_r;
  y[PF_TH] = path_th;
  y[PF_ATH] = abs_th;
  y[PF_SQTH] = sharpness;

#ifdef PF_DUR
  y[PF_DUR] = (endtime - starttime) * .01;

#if 0  // debugging
	Z(D) {
		int i;
		for(i = NROWS(y); i > 0; i--) y[i] = y[i-1];
		y[0] = (endtime - starttime)*.01;
	}
#endif // debugging
#endif // PF_DUR

#ifdef PF_MAXV
  y[PF_MAXV] = maxv * 10000;
#endif

  calculated = true;
}
