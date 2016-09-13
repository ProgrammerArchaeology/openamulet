/***********************************************************************
   (C) Copyright, 1990 by Dean Rubine, Carnegie Mellon University
    Permission to use this code for noncommercial purposes is hereby granted.
    Permission to copy and distribute this code is hereby granted provided
    this copyright notice is retained.  All other rights reserved.
 **********************************************************************/

#ifndef GEST_MATRIX_H
#define GEST_MATRIX_H

#include <iostream>

/*
 
 Simple matrix operations
 Why I am writing this stuff over is beyond me

*/

/*

This package provides the Matrix and Vector data types

The difference between this matrix package and others is that:
	Vectors may be accessed as 1d arrays
	Matrices may still be accessed like two dimensional arrays
This is accomplished by putting a structure containing the bounds
of the matrix before the pointer to the (array of) doubles (in the
case of a Vector) or before the pointer to an (array of) pointers
to doubles (in the case of a Matrix).

Vectors and matrices are collectively called "arrays" herein.
*/

#define HEADER(a) (((union array_header *)a) - 1)

#define NDIMS(a) (HEADER(a)->s.ndims)
#define NROWS(a) (HEADER(a)->s.nrows)
#define NCOLS(a) (HEADER(a)->s.ncols)
#define ISVECTOR(a) (NDIMS(a) == 1)
#define ISMATRIX(a) (NDIMS(a) == 2)

union array_header
{
  struct
  {
    unsigned char ndims; /* 1 = vector, 2 = matrix */
    unsigned char nrows;
    unsigned char ncols;
  } s;
  double d;
  double *p;
};

typedef double **Matrix;
typedef double *Vector;

Vector NewVector(int r);
Matrix NewMatrix(int r, int c);
void FreeVector(Vector v);
void FreeMatrix(Matrix m);
extern "C" {
void PrintVector(Vector v, const char *fmt, ...);
void PrintMatrix(Matrix m, const char *fmt, ...);
}
double InnerProduct(Vector v1, Vector v2);
void MatrixMultiply(Matrix m1, Matrix m2, Matrix prod);
void VectorTimesMatrix(Vector v, Matrix m, Vector prod);
void ScalarTimesVector(double s, Vector v, Vector prod);
double QuadraticForm(Vector v, Matrix m);
double InvertMatrix(Matrix input, Matrix result);
Vector VectorCopy(Vector v);
Matrix MatrixCopy(Matrix m);
Vector InputVector(FILE *f);
Matrix InputMatrix(FILE *f);
void OutputVector(FILE *f, Vector v);
void OutputVectorCPP(std::ostream &s, Vector m);
void OutputMatrix(FILE *f, Matrix m);
void OutputMatrixCPP(std::ostream &s, Matrix m);

double InvertSingularMatrix(Matrix input, Matrix result);

Matrix SliceMatrix(Matrix m, unsigned rowmask, unsigned colmask);
Matrix DeSliceMatrix(Matrix m, double fill, unsigned rowmask, unsigned colmask,
                     Matrix r);

void ZeroVector(Vector v);
void ZeroMatrix(Matrix m);
void FillMatrix(Matrix m, double fill);

Vector InputVectorCPP(std::istream &f);
Matrix InputMatrixCPP(std::istream &f);

#endif // GEST_MATRIX_H
