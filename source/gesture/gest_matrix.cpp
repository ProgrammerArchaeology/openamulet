/***********************************************************************
   (C) Copyright, 1990 by Dean Rubine, Carnegie Mellon University
    Permission to use this code for noncommercial purposes is hereby granted.
    Permission to copy and distribute this code is hereby granted provided
    this copyright notice is retained.  All other rights reserved.
 **********************************************************************/

/*
 
 Simple matrix operations
 Why I am writing this stuff over is beyond me

*/

#undef PIQ_DEBUG

#include <am_inc.h>

#include AM_IO__H
#include TYPES__H  // for Am_Wrapper, Am_Error

#include <math.h>

#ifdef SHORT_NAMES
#include "amulet/gest_mat.h"
#else
#include "amulet/gest_matrix.h"
#endif

typedef	union array_header *Array;

#define EPSILON		(1.0e-10)	/* zero range */

/*
 Allocation functions
*/

Vector
NewVector(int r)
{
	const int size_in_bytes = r * sizeof(double);
	const int size_in_headers = (size_in_bytes + sizeof(array_header) - 1)
	                            / sizeof(array_header);
	
	register array_header *a = new array_header[1 + size_in_headers];
	register Vector v;

	a->s.ndims = 1;
	a->s.nrows = r;
	a->s.ncols = 1;
	v = (Vector) (a + 1);

#define CHECK
#ifdef CHECK
	if(HEADER(v) != (union array_header *) a ||
	   NDIMS(v) != 1 || NROWS(v) != r || NCOLS(v) != 1)
	       std::cout << "NewVector error: v=" << v << " H: " << HEADER(v) << ',' << a << " D:" << (int)NDIMS(v) << ',' << 1 << " R:" << (int)NROWS(v) << ',' << r << " C:" << (int)NCOLS(v) << ',' << 1 <<std::endl;
//	    	printf("NewVector error: v=%x H: %x,%x  D:%d,%d  R:%d,%d  C:%d,%d\n", v,  HEADER(v), a,  NDIMS(v), 1,  NROWS(v), r, NCOLS(v), 1);
#endif

	return v;
}

Matrix
NewMatrix(int r, int c)
{
	const int size_in_bytes = r * sizeof(double*);
	const int size_in_headers = (size_in_bytes + sizeof(array_header) - 1)
	                            / sizeof(array_header);
	
	register array_header *a = new array_header[1 + size_in_headers];
	register int i;
	register Matrix m;

	a->s.ndims = 2;
	a->s.nrows = r;
	a->s.ncols = c;
	
	m = (Matrix) (a + 1);
	for(i = 0; i < r; i++)
		m[i] = new double[c];
	return m;
}

void
FreeVector(Vector v)
{
	delete [] HEADER(v);
}

void
FreeMatrix(Matrix m)
{
	register int i;

	for(i = 0; i < NROWS(m); i++)
		delete [] m[i];
	delete [] HEADER(m);
}

Vector
VectorCopy(register Vector v)
{
	register Vector r = NewVector(NROWS(v));
	register int i;

	for(i = 0; i < NROWS(v); i++)
		r[i] = v[i];
	return r;
}

Matrix
MatrixCopy(register Matrix m)
{
	register Matrix r = NewMatrix(NROWS(m), NCOLS(m));
	register int i, j;

	for(i = 0; i < NROWS(m); i++)
		for(j = 0; j < NROWS(m); j++)
			r[i][j] = m[i][j];
	return r;
}

#if 0  // omit (variable args don't compile on HP CC)

extern "C" {
#include <varargs.h>

void
PrintVector(register Vector v, char *s, ...)
{
	register int i;
  va_list ap;
  va_start(ap);
	vprintf(s,ap);
	for(i = 0; i < NROWS(v); i++) printf(" %8.4f", v[i]);
	printf("\n");
}

void
PrintMatrix(register Matrix m, char *s, ...)
{
	register int i, j;
  va_list ap;
  va_start(ap);
	printf(s,ap);
	for(i = 0; i < NROWS(m);  i++) {
		for(j = 0; j < NCOLS(m); j++)
			printf(" %8.4f", m[i][j]);
		printf("\n");
	}
}

}
#endif // omitted code

/* Null vector and matrixes */

void
ZeroVector(Vector v)
{
	register int i;
	for(i = 0; i < NROWS(v); i++) v[i] = 0.0;
}

void
ZeroMatrix(Matrix m)
{
	register int i, j;
	for(i = 0; i < NROWS(m); i++)
		for(j = 0; j < NCOLS(m); j++)
			m[i][j] = 0.0;
}

void
FillMatrix(Matrix m, double fill)
{
	register int i, j;
	for(i = 0; i < NROWS(m); i++)
		for(j = 0; j < NCOLS(m); j++)
			m[i][j] = fill;
}

double
InnerProduct(register Vector v1, register Vector v2)
{
	double result = 0;
	register int n = NROWS(v1);
	if(n != NROWS(v2))
		Am_Error("bad InnerProduct");//, n, NROWS(v2));

	while(--n >= 0)
		result += *v1++ * *v2++;
	return result;
}

void
MatrixMultiply(register Matrix m1, register Matrix m2, register Matrix prod)
{
	register int i, j, k;
	double sum;

	if(NCOLS(m1) != NROWS(m2))
		Am_Error("MatrixMultiply");//: Can't multiply %dx%d and %dx%d matrices",
			//NROWS(m1), NCOLS(m1), NROWS(m2), NCOLS(m2));
	if(NROWS(prod) != NROWS(m1) || NCOLS(prod) != NCOLS(m2))
		Am_Error("MatrixMultiply");//: %dx%d times %dx%d does not give %dx%d product",
			//NROWS(m1), NCOLS(m1), NROWS(m2), NCOLS(m2),
			//NROWS(prod), NCOLS(prod));

	for(i = 0; i < NROWS(m1); i++)
		for(j = 0; j < NCOLS(m2); j++) {
			sum = 0;
			for(k = 0; k < NCOLS(m1); k++)
				sum += m1[i][k] * m2[k][j];
			prod[i][j] = sum;
		}
}

/*
Compute result = v'm where
	v is a column vector (r x 1)
	m is a matrix (r x c)
	result is a column vector (c x 1)
*/

void
VectorTimesMatrix(Vector v, Matrix m, Vector prod)
{
	register int i, j;

	if(NROWS(v) != NROWS(m))
		Am_Error("VectorTimesMatrix");//: Can't multiply %d vector by %dx%d",
			//NROWS(v), NROWS(m), NCOLS(m));
	if(NROWS(prod) != NCOLS(m))
		Am_Error("VectorTimesMatrix");//: %d vector times %dx%d mat does not fit in %d product" ,
			//NROWS(v), NROWS(m), NCOLS(m), NROWS(prod));

	for(j = 0; j < NCOLS(m); j++) {
		prod[j] = 0;
		for(i = 0; i < NROWS(m); i++)
			prod[j] += v[i] * m[i][j];
	}
}	

void
ScalarTimesVector(double s, register Vector v, register Vector product)
{
	register int n = NROWS(v);

	if(NROWS(v) != NROWS(product))
		Am_Error("ScalarTimesVector");//: result wrong size (%d!=%d)",
			//NROWS(v), NROWS(product));

	while(--n >= 0)
		*product++ = s * *v++;
}

void
ScalarTimesMatrix(double s, register Matrix m, register Matrix product)
{
	register int i, j;

	if(NROWS(m) != NROWS(product)  || 
	   NCOLS(m) != NCOLS(product))
		Am_Error("ScalarTimesMatrix");//: result wrong size (%d!=%d)or(%d!=%d)",
			//NROWS(m), NROWS(product),
			//NCOLS(m), NCOLS(product));

	for(i = 0; i < NROWS(m); i++)
		for(j = 0; j < NCOLS(m); j++)
			product[i][j] = s * m[i][j];
}

/*
 Compute v'mv
 */

double
QuadraticForm(register Vector v, register Matrix m)
{
	register int i, j, n;
	double result = 0;

	n = NROWS(v);

	if(n != NROWS(m) || n != NCOLS(m))
		
		Am_Error("QuadraticForm");//: bad matrix size (%dx%d not %dx%d)",
			//NROWS(m), NCOLS(m), n, n);

	for(i = 0; i < n; i++)
		for(j = 0; j < n; j++) {

#ifdef PIQ_DEBUG
			printf("%g*%g*%g [%g] %s ",
			m[i][j],v[i],v[j],
			m[i][j] * v[i] * v[j],
			i==n-1&&j==n-1? "=" : "+");
#endif

			result += m[i][j] * v[i] * v[j];
		}
	return result;
}

/* Matrix inversion using full pivoting.
 * The standard Gauss-Jordan method is used.
 * The return value is the determinant.
 * The input matrix may be the same as the result matrix
 *
 *	det = InvertMatrix(inputmatrix, resultmatrix);
 *
 * HISTORY
 * 26-Feb-82  David Smith (drs) at Carnegie-Mellon University
 *	Written.
 * Sun Mar 20 19:36:16 EST 1988 - stolen by dandb, and converted to this form
 *
 */

int	DebugInvertMatrix = 0;

#define PERMBUFSIZE 200	/* Max mat size */

#define abs(x) ((x)>=0 ? (x) : -(x))

double
InvertMatrix(Matrix ym, Matrix rm)
{
	register int i, j, k;
	double det, biga, recip_biga, hold;
	int l[PERMBUFSIZE], m[PERMBUFSIZE];
	register int n;

	if(NROWS(ym) != NCOLS(ym))
		Am_Error("InvertMatrix: not square");

	n = NROWS(ym);

	if(n != NROWS(rm) || n != NCOLS(rm))
		Am_Error("InvertMatrix: result wrong size");

	/* Copy ym to rm */
	
	if(ym != rm)
		for(i = 0; i < n; i++)
			for(j = 0; j < n; j++)
				rm[i][j] = ym[i][j];

/*	if(DebugInvertMatrix) PrintMatrix(rm, "Inverting (det=%g)\n", det);*/

    /* Allocate permutation vectors for l and m, with the same origin
       as the matrix. */

	if (n >= PERMBUFSIZE)
		Am_Error("InvertMatrix: PERMBUFSIZE");

	det = 1.0;
	for (k = 0; k < n;  k++) {
		l[k] = k;  m[k] = k;
		biga = rm[k][k];

		/* Find the biggest element in the submatrix */
		for (i = k;  i < n;  i++)
			for (j = k; j < n; j++)
				if (abs(rm[i][j]) > abs(biga)) {
					biga = rm[i][j];
					l[k] = i;
					m[k] = j;
				}

/*		if(DebugInvertMatrix) 
			if(biga == 0.0)
				PrintMatrix((Matrix)m, "found zero biga = %g\n", biga);*/

		/* Interchange rows */
		i = l[k];
		if (i > k)
			for (j = 0; j < n; j++) {
				hold = -rm[k][j];
				rm[k][j] = rm[i][j];
				rm[i][j] = hold;
			}

		/* Interchange columns */
		j = m[k];
		if (j > k)
			for (i = 0; i < n; i++) {
				hold = -rm[i][k];
				rm[i][k] = rm[i][j];
				rm[i][j] = hold;
			}

		/* Divide column by minus pivot
		    (value of pivot element is contained in biga). */
		if (biga == 0.0) {
			return 0.0;
		}

		if(DebugInvertMatrix) printf("biga = %g\n", biga);
		recip_biga = 1/biga;
		for (i = 0; i < n; i++)
			if (i != k)
				rm[i][k] *= -recip_biga;

		/* Reduce matrix */
		for (i = 0; i < n; i++)
			if (i != k) {
				hold = rm[i][k];
				for (j = 0; j < n; j++)
					if (j != k)
						rm[i][j] += hold * rm[k][j];
			}

		/* Divide row by pivot */
		for (j = 0; j < n; j++)
			if (j != k)
				rm[k][j] *= recip_biga;

		det *= biga;	/* Product of pivots */
		if(DebugInvertMatrix) printf("det = %g\n", det);
		rm[k][k] = recip_biga;

	}	/* K loop */

	/* Final row & column interchanges */
	for (k = n - 1; k >= 0; k--) {
		i = l[k];
		if (i > k)
			for (j = 0; j < n; j++) {
				hold = rm[j][k];
				rm[j][k] = -rm[j][i];
				rm[j][i] = hold;
			}
		j = m[k];
		if (j > k)
			for (i = 0; i < n; i++) {
				hold = rm[k][i];
				rm[k][i] = -rm[j][i];
				rm[j][i] = hold;
			}
	}

	if(DebugInvertMatrix) printf("returning, det = %g\n", det);

	return det;
}

void
OutputVector(FILE *f, register Vector v)
{
	register int i;
	fprintf(f, " V %d   ", NROWS(v));
	for(i = 0; i < NROWS(v); i++)
		fprintf(f, " %g", v[i]);
	fprintf(f, "\n");
}

void
OutputVectorCPP(std::ostream &s, register Vector v)
{
  register int i;
  s << "V " << (int)NROWS(v);
  for(i = 0; i < NROWS(v); i++)
    s << ' ' << v[i];
  s <<std::endl;
  
}

Vector
InputVector(FILE *f)
{
	register Vector v;
	register int i;
	char check;
	int nrows;

	if(fscanf(f, "%1s %d", &check, &nrows) != 2)
		Am_Error("InputVector fscanf 1");
	if(check != 'V')
		Am_Error("InputVector check");
	v = NewVector(nrows);
	for(i = 0; i < nrows; i++)
		if(fscanf(f, "%lf", &v[i]) != 1)
			Am_Error("InputVector fscanf 2");
	return v;
}

Vector
InputVectorCPP(std::istream &f)
{
	register Vector v;
	register int i;
	char check;
	int nrows;

	f >> check >> nrows;
	if(check != 'V') {
	  f.clear(std::ios::badbit);
	  return (0L);
	}
	v = NewVector(nrows);
	for(i = 0; i < nrows; i++)
	  f >> v[i];
	return v;
}

static int bitcount (int max, unsigned mask)
{
  int c = 0;
  int i;

  for (i = 0; i<max; ++i)
    if (mask & (1<<i))
      ++c;
  return c;
}

Matrix
SliceMatrix(Matrix m, unsigned rowmask, unsigned colmask)
{
	register int i, ri, j, rj;

	Matrix r;
	
	r = NewMatrix(bitcount(NROWS(m), rowmask),
		      bitcount(NCOLS(m), colmask));
	for(i = ri = 0; i < NROWS(m); i++)
		if(rowmask & (1<<i)) {
			for(j = rj = 0; j < NCOLS(m); j++)
				if(colmask & (1<<j))
					r[ri][rj++] = m[i][j];
			ri++;
		}

	return r;
}

Matrix
DeSliceMatrix(Matrix m, double fill, unsigned rowmask, unsigned colmask,
	      Matrix r)
{
	register int i, ri, j, rj;

	FillMatrix(r, fill);

	for(i = ri = 0; i < NROWS(r); i++) {
		if(rowmask & (1<<i)) {
			for(j = rj = 0; j < NCOLS(r); j++)
				if(colmask & (1<<j))
					r[i][j] = m[ri][rj++];
			ri++;
		}
	}

	return r;
}

void
OutputMatrix(FILE *f, register Matrix m)
{
	register int i, j;
	fprintf(f, " M %d %d\n", NROWS(m), NCOLS(m));
	for(i = 0; i < NROWS(m);  i++) {
		for(j = 0; j < NCOLS(m); j++)
			fprintf(f, " %g", m[i][j]);
		fprintf(f, "\n");
	}
}

Matrix
InputMatrix(FILE *f)
{
	register Matrix m;
	register int i, j;
	char check;
	int nrows, ncols;

	if(fscanf(f, "%1s %d %d", &check, &nrows, &ncols) != 3)
		Am_Error("InputMatrix fscanf 1");
	if(check != 'M')
		Am_Error("InputMatrix check");
	m = NewMatrix(nrows, ncols);
	for(i = 0; i < nrows; i++)
		for(j = 0; j < ncols; j++)
			if(fscanf(f, "%lf", &m[i][j]) != 1)
				Am_Error("InputMatrix fscanf 2");
	return m;
}

void
OutputMatrixCPP(std::ostream &s, register Matrix m)
{
  register int i, j;
  s << "M " << (int)NROWS(m) << ' ' << (int)NCOLS(m) <<std::endl;
  for(i = 0; i < NROWS(m);  i++) {
    for(j = 0; j < NCOLS(m); j++)
      s << ' ' << m[i][j];
    s <<std::endl;
  }
}

Matrix
InputMatrixCPP(std::istream &f)
{
	register Matrix m;
	register int i, j;
	char check;
	int nrows, ncols;

	f >> check >> nrows >> ncols;
	if(check != 'M') {
	  f.clear(std::ios::badbit);
	  return (0L);
	}
	m = NewMatrix(nrows, ncols);
	for(i = 0; i < nrows; i++)
		for(j = 0; j < ncols; j++)
		  f >> m[i][j];
	return m;
}

