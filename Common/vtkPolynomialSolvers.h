/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================
  Copyright 2007 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
  license for use of this work by or on behalf of the
  U.S. Government. Redistribution and use in source and binary forms, with
  or without modification, are permitted provided that this Notice and any
  statement of authorship are reproduced on all copies.

  Contact: pppebay@ca.sandia.gov,dcthomp@sandia.gov

=========================================================================*/
// .NAME vtkPolynomialSolvers - polynomial solvers
// .SECTION Description
// vtkPolynomialSolvers provides solvers for univariate polynomial 
// equations and for multivariate polynomial systems.

#ifndef __vtkPolynomialSolvers_h
#define __vtkPolynomialSolvers_h

#include "vtkObject.h"

#ifndef DBL_EPSILON
#  define VTK_DBL_EPSILON    2.2204460492503131e-16
#else  // DBL_EPSILON
#  define VTK_DBL_EPSILON    DBL_EPSILON
#endif  // DBL_EPSILON

class VTK_COMMON_EXPORT vtkPolynomialSolvers : public vtkObject
{
public:
  static vtkPolynomialSolvers *New();
  vtkTypeRevisionMacro(vtkPolynomialSolvers,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Counts the number of REAL roots of the \a d -th degree polynomial 
  //   P[0] X^d + ... + P[d-1] X + P[d] 
  // in ]\a[0], \a[1]] using Sturm's theorem ( polynomial coefficients are 
  // REAL ) and returns the count.
  // Returns -1 if anything went wrong (such as: polynomial does not have
  // degree d, the interval provided by the other is absurd, etc.).
  static int SturmRootCount( double* P, int d, double* a );

  // Description:
  // Finds all REAL roots (within tolerance \tol) of the \a d -th degree polynomial 
  //   P[0] X^d + ... + P[d-1] X + P[d] 
  // in ]\a[0] ; \a[1]] using Sturm's theorem ( polynomial 
  // coefficients are REAL ) and returns the count \nr. All roots are bracketed
  // in the \nr first ]\lowerBnds[i] ; \lowerBnds[i] + tol] intervals.
  // Returns -1 if anything went wrong (such as: polynomial does not have
  // degree d, the interval provided by the other is absurd, etc.).
  // Warning: it is the user's responsibility to make sure the \lowerBnds 
  // Note that \nr is smaller or equal to the actual number of roots in 
  // ]\a[0] ; \a[1]] since roots within \tol are lumped in the same bracket.
  // array is large enough to contain the maximal number of expected lower bounds.
  static int SturmBissectionSolve( double* P, int d, double* a, double *lowerBnds, double tol );

protected:
  vtkPolynomialSolvers() {};
  ~vtkPolynomialSolvers() {};
  
  static long Seed;
private:
  vtkPolynomialSolvers(const vtkPolynomialSolvers&);  // Not implemented.
  void operator=(const vtkPolynomialSolvers&);  // Not implemented.
};

#endif
